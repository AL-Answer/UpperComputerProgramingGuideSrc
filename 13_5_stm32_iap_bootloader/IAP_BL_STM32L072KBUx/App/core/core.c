#include "core.h"

#include "../protocol/command.h"

//Э�鷴�����ݵĻ���
#define TX_BUF_SIZE 20
static uint8_t TxByte[TX_BUF_SIZE];

//�汾��
#define VERSION_ID 0x01

//flashһҳ�Ļ���1024
extern uint8_t FlashOnePageCache[STM32_FLASH_ONE_PAGE_BYTE];//shaare array

static uint8_t FlashWriteToCacheIndex = 0;

static void ShakeHands();
static void DownloadStart();
static void DownloadStop();
static void DownloadData(uint8_t *buf, uint16_t len);
static void DownloadCheckSum(uint8_t *buf, uint16_t len);

//��Щ���Դ�ӡ������ʱ��ɾ���������Ժ���ֲ������оƬ���������
//uint32_t global_temp_checksum = 0;
//uint32_t global_temp_checksum2 = 0;

#define BIN_FILE_PIECE_SIZE	128

void APP_ProcessMetaPkg(uint8_t *buf, uint16_t len) {

	if(buf[0] == SetValue) {
		switch (buf[1]) {
		case COMMAND_SHAKE_HANDS : ShakeHands(); break;
		case COMMAND_DOWNLOAD_START : DownloadStart(); break;
		case COMMAND_DOWNLOAD_STOP : DownloadStop(); break;
		case COMMAND_DOWNLOAD_DATA : DownloadData(buf, len); break;
		case COMMAND_DOWNLOAD_CHECKSUM : DownloadCheckSum(buf, len); break;
		default: break;
		}
	}
}

static void ResponseHelper(uint8_t command, uint8_t* param_buf, uint16_t param_buf_len) {
	ResponsePkg resPkg;
	resPkg.res_buffer = TxByte;
	resPkg.res_buffer_len = TX_BUF_SIZE;
	resPkg.command_type = SetValue;
	resPkg.command = command;
	resPkg.param_buf = param_buf;
	resPkg.param_len = param_buf_len;
	int len = APP_GetProtocolResponsePKG(&resPkg);
	if( len > 0) {
		BSP_Uart1SendData(TxByte, len);
	}
}

static void ShakeHands() {
	uint8_t VersionBuf[1];
	VersionBuf[0] = VERSION_ID;
	ResponseHelper(COMMAND_SHAKE_HANDS, VersionBuf, 1);
}

static void DownloadStart() {
	//��Щ���Դ�ӡ������ʱ��ɾ���������Ժ���ֲ������оƬ���������
	//global_temp_checksum = 0;
	//global_temp_checksum2 = 0;
	//flash_checksum = 0;

	FlashWriteToCacheIndex = 0;


	Main_ClearStartupFlag(false);
	memset(FlashOnePageCache, 0, STM32_FLASH_ONE_PAGE_BYTE);
	APP_EraseAppUsingPages();
	ResponseHelper(COMMAND_DOWNLOAD_START, 0, 0);
}

static void DownloadStop() {
	ResponseHelper(COMMAND_DOWNLOAD_STOP, 0, 0);
	APP_SetAppFlag();
	APP_RunApp();
}

//�����ع̼���У��ֵ��0x00, 0x00, 0x00, 0x00��4byte,��λ��ǰ ��
static void DownloadCheckSum(uint8_t *buf, uint16_t len) {
	//be note that : index 00 01 are command_type and command.
	//your data is start from index 02

	uint32_t correct_check_sum = 0;
	uint32_t temp = 0;

	temp = buf[2]; correct_check_sum |= temp << 24;
	temp = buf[3]; correct_check_sum |= temp << 16;
	temp = buf[4]; correct_check_sum |= temp << 8;
	temp = buf[5]; correct_check_sum |= temp << 0;
	//should be 0x0024f485

	uint16_t need_been_checked_page_count = 0;
	uint32_t total_bin_file_pieces_count = 0;
	temp = buf[6]; total_bin_file_pieces_count |= temp << 24;
	temp = buf[7]; total_bin_file_pieces_count |= temp << 16;
	temp = buf[8]; total_bin_file_pieces_count |= temp << 8;
	temp = buf[9]; total_bin_file_pieces_count |= temp << 0;


	need_been_checked_page_count = total_bin_file_pieces_count / 8;
	if(total_bin_file_pieces_count % 8 != 0) {
		need_been_checked_page_count ++;
	}

	uint32_t calced_checksum = APP_CheckAppSum(need_been_checked_page_count);

	//��Щ���Դ�ӡ������ʱ��ɾ���������Ժ���ֲ������оƬ���������
	//printf("correct_check_sum  = %x", (correct_check_sum) & 0xffffffff);
	//printf("global_temp_checksum = %x", (global_temp_checksum) & 0xffffffff);
	//printf("global_temp_checksum2 = %x", (global_temp_checksum2) & 0xffffffff);
	//printf("flash_checksum = %x", (flash_checksum) & 0xffffffff);
	//printf("calced_checksum = %x", (calced_checksum) & 0xffffffff);

	if(calced_checksum == correct_check_sum) {
		uint8_t param_buf[1];
		param_buf[0] = 0x01;//success flag
		ResponseHelper(COMMAND_DOWNLOAD_CHECKSUM, param_buf, 1);
	} else {
		uint8_t param_buf[1];
		param_buf[0] = 0x00;//failed flag
		ResponseHelper(COMMAND_DOWNLOAD_CHECKSUM, param_buf, 1);
	}
}

static void DownloadData(uint8_t *buf, uint16_t len) {
	//be note that : index 00 01 are command_type and command.
	//your data is start from index 02

	uint16_t data_piece_index = 0;

	data_piece_index = buf[2];
	data_piece_index <<= 8;
	data_piece_index |= buf[3];

	bool is_last_piece = false;
	if(buf[4] == 0x01) {
		is_last_piece = true;
	}

	//todo����data_piece_index ����ţ�ת��Ϊ 0~7 �ĵ�FlashWriteToCacheIndex���� ���÷�Χ��
	//�������滻����FlashWriteToCacheIndex

	//���ݷ�Ƭ��ţ�2byte,��λ��ǰ �� + �Ƿ�Ϊ���һƬ��1byte����Ҫ���ص����ݣ�Լ��Ϊ�̶���С128byte�� 128*8 = 1024��
	uint16_t index = FlashWriteToCacheIndex * BIN_FILE_PIECE_SIZE;
	uint8_t* hex_buf = &buf[5];

	for (int a = 0; a < BIN_FILE_PIECE_SIZE; a ++) {
		FlashOnePageCache[index + a] = hex_buf[a];
		//��Щ���Դ�ӡ������ʱ��ɾ���������Ժ���ֲ������оƬ���������
		//global_temp_checksum += hex_buf[a];
	}
	FlashWriteToCacheIndex ++;

	//���յ�һ��ҳ��ʱ�򣬰�һҳ��������д��flash�����������
	if(FlashWriteToCacheIndex >= 8 || is_last_piece) {
		FlashWriteToCacheIndex = 0;

		//write flash_data into chip_flash
		APP_ProgramOnePageIntoFlash(FlashOnePageCache, STM32_FLASH_ONE_PAGE_BYTE);

		//��Щ���Դ�ӡ������ʱ��ɾ���������Ժ���ֲ������оƬ���������
		//for (int v = 0; v < STM32_FLASH_ONE_PAGE_BYTE; v ++) {
		//	global_temp_checksum2 += FlashOnePageCache[v];
		//}

		memset(FlashOnePageCache, 0, STM32_FLASH_ONE_PAGE_BYTE);
	}

	ResponseHelper(COMMAND_DOWNLOAD_DATA, 0, 0);
}






