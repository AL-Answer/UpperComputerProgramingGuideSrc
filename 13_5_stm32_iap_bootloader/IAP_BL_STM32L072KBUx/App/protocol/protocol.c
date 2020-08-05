#include "protocol.h"

#include "crc.h"
#include "command.h"
#include "../core/core.h"

#define RECV_BUF_SIZE 150

static bool IsStartRecvCommand = false;

static uint8_t UartRxCache[RECV_BUF_SIZE];
static uint16_t UartRxCacheIndex = 0;

static uint8_t UartRxCache1[RECV_BUF_SIZE];
static uint16_t UartRxCacheValidDataLen1 = 0;
static bool IsUartRxCacheHasData1 = false;

static uint8_t UartRxCache2[RECV_BUF_SIZE];
static uint16_t UartRxCacheValidDataLen2 = 0;
static bool IsUartRxCacheHasData2 = false;



static void ProcessCommand(uint8_t *buffer, uint16_t len);



//������һ�����ݵĴ�����
//˭�����ң����ܳ�ʱ�жϺ����������жϽ��պ���,��⵽��β
static void ProcessAFinishedPackage() {

	//copy UartRxCache into AppRxCache
	if(IsUartRxCacheHasData1 == false) {
		memcpy(UartRxCache1, UartRxCache, UartRxCacheIndex);
		UartRxCacheValidDataLen1 = UartRxCacheIndex;
		IsUartRxCacheHasData1 = true;

	} else if (IsUartRxCacheHasData2 == false) {
		memcpy(UartRxCache2, UartRxCache, UartRxCacheIndex);
		UartRxCacheValidDataLen2 = UartRxCacheIndex;
		IsUartRxCacheHasData2 = true;
	}

	//clear UartRxCache
	UartRxCacheIndex = 0;
}

/**
 * @brief	����������whileѭ�������˺���(ע��:��Ҫ��֤�����ٶȴ������ݽ����ٶ�)
 */
void APP_ProcessProtocolReceivedPkgAtMain(void) {

	uint8_t *buffer = 0;
	int buffer_valid_data_len = 0;

	if(IsUartRxCacheHasData1) {
		buffer = UartRxCache1;
		buffer_valid_data_len = UartRxCacheValidDataLen1;
		ProcessCommand(buffer, buffer_valid_data_len);
		IsUartRxCacheHasData1 = false;

	} else if (IsUartRxCacheHasData2) {
		buffer = UartRxCache2;
		buffer_valid_data_len = UartRxCacheValidDataLen2;
		ProcessCommand(buffer, buffer_valid_data_len);
		IsUartRxCacheHasData2 = false;
	}
}

void APP_UartProcess(uint8_t uart_data) {
	//˫����:���˼·����
	//����2���������� ����ѭ��ʹ��
	//���ڴ����жϽ��գ��鿴���е����ݰ����Ǹ��ǳ��ڿ���״̬�ģ��ҵ��������������洮�����յ������ݣ�����һ�������޸ı�־��ʾ������һ�����ȴ�����
	//������ѭ���������ж��ĸ����ݻ������н�����ɵ����ݰ����ͽ����ĸ���Ȼ�����������棬�������ٴ��ڴ��ڴ��������ݰ�

	if(IsStartRecvCommand == true) {
		UartRxCache[UartRxCacheIndex++] = uart_data;

		uint16_t TempCacheLen = UartRxCacheIndex;
		if(TempCacheLen > 4 &&
				UartRxCache[TempCacheLen - 4] == TAIL[0] &&
				UartRxCache[TempCacheLen - 3] == TAIL[1] &&
				UartRxCache[TempCacheLen - 2] == TAIL[2] &&
				UartRxCache[TempCacheLen - 1] == TAIL[3] &&
				UartRxCache[sizeof (HEAD)] + sizeof (HEAD) + sizeof (TAIL)  == TempCacheLen //check pkg len

		) { //��⵽һ���������
			ProcessAFinishedPackage();
			UartRxCacheIndex = 0;
			IsStartRecvCommand = false;
		}

	} else if(uart_data == HEAD[0] && IsStartRecvCommand == false) { //only receiving command when start with a head
		IsStartRecvCommand = true;
		UartRxCache[UartRxCacheIndex++] = uart_data;
	}
}

static void ProcessCommand(uint8_t *buffer, uint16_t len) {

	int skip_head_and_pkg_len = 2;
	int skip_tail = 4;
	uint8_t* pkg = 0;

	pkg = &buffer[skip_head_and_pkg_len]; //remove head and pkg-len
	int pkg_len = len - skip_head_and_pkg_len - skip_tail;//skip index of tail byte

	if (CheckCRC(pkg, pkg_len) == true) {
		APP_ProcessMetaPkg(pkg, pkg_len-2);//-2 : remove crc byte
	}
}




