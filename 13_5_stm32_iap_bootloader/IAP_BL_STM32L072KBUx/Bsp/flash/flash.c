#include "flash.h"


#ifdef TEST
uint8_t flash_check_buf[STM32_FLASH_ONE_PAGE_BYTE];
#endif

//��Щ���Դ�ӡ������ʱ��ɾ���������Ժ���ֲ������оƬ���������
//uint32_t flash_checksum = 0;

/*
//��1���ֽ�
static uint8_t FLASH_ReadByte(uint32_t Addr) {
	return *(uint8_t *)Addr;
}

//��2���ֽ�
static uint16_t FLASH_ReadHalfWord(uint32_t Addr) {
	return *(uint16_t *)Addr;
}*/

static uint32_t FLASH_ReadWord(uint32_t Addr) {
	return *(uint32_t *)Addr;
}

/**
 * @brief	��flash��ָ��ҳ����һҳ������
 * @param	PageIndex	ҳ����ʼ��ַ
 * @param	pBuff	ҳ�Ļ�������
 * @retval	��
 */
void BSP_FLASH_ReadPage(uint8_t PageIndex, uint8_t *pBuff) {
	uint16_t i;
	uint32_t Buff;
	uint32_t Addr;

	//�Ƿ񳬳���Χ
	if(PageIndex > STM32_FLASH_PAGE_SIZE)
		return;
	//�ȼ���ҳ�׵�ַ
	Addr = PageIndex * STM32_FLASH_ONE_PAGE_BYTE + STM32_FLASH_BASE;

	for(i = 0; i < STM32_FLASH_ONE_PAGE_BYTE; i += 4) {
		Buff = FLASH_ReadWord(Addr);

		//ע�⣬����д��оƬ��flash���棬�ǵ��ֽ���ǰ����������޷�����
		pBuff[i+0] = (Buff >> 0) & 0xff;
		pBuff[i+1] = (Buff >> 8) & 0xff;
		pBuff[i+2] = (Buff >> 16) & 0xff;
		pBuff[i+3] = (Buff >> 24) & 0xff;

		Addr += 4;
	}
}

/**
 * @brief	��flash��ָ��ҳд��һҳ������
 * @param	PageIndex	ҳ����ʼ��ַ
 * @param	pBuff	ҳ�Ļ�������
 * @retval	��
 */
void BSP_FLASH_WritePage(uint8_t PageIndex, uint8_t *pBuff) {
	uint16_t i = 0;
	uint32_t Buff = 0;
	uint32_t Addr = 0;
	uint32_t temp = 0;

	if(PageIndex > STM32_FLASH_PAGE_SIZE-1)
		return;
	HAL_FLASH_Unlock();

	Addr = PageIndex * STM32_FLASH_ONE_PAGE_BYTE + STM32_FLASH_BASE;

	for(i = 0; i < STM32_FLASH_ONE_PAGE_BYTE ; i += 4)
	{
		//ע�⣬����д��оƬ��flash���棬�ǵ��ֽ���ǰ����������޷�����
		Buff = 0x00000000U;//Ah hell, you must do not forget to clear this parameter
		temp = pBuff[i+0] & 0xff; Buff |= (temp << 0); //flash_checksum += temp;
		temp = pBuff[i+1] & 0xff; Buff |= (temp << 8); //flash_checksum += temp;
		temp = pBuff[i+2] & 0xff; Buff |= (temp << 16);  //flash_checksum += temp;
		temp = pBuff[i+3] & 0xff; Buff |= (temp << 24);  //flash_checksum += temp;

		HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Addr, Buff);//32bit
		Addr += 4;
	}
	HAL_FLASH_Lock();
}

/**
 * @brief	���flash���������Ƿ�����
 * @param	StartPageIndex	ɾ��ҳ����ʼ��ַ
 * @param	PageCount	ɾ��ҳ�ĸ���
 * @retval	��
 */
void BSP_Flash_ErasePage(uint8_t StartPageIndex, uint32_t PageCount) {
	HAL_FLASH_Unlock();
	uint32_t addr = STM32_FLASH_BASE + StartPageIndex * STM32_FLASH_ONE_PAGE_BYTE;

	FLASH_EraseInitTypeDef EraseParam;
	EraseParam.TypeErase = FLASH_TYPEERASE_PAGES;
	EraseParam.NbPages = PageCount*8;//�����1ҳΪ128byte������оƬ��ҳȷʵ��1024byte������������8
	EraseParam.PageAddress = addr;
	uint32_t PageError = 0;

	HAL_FLASHEx_Erase(&EraseParam, &PageError);

	if (PageError == 0xFFFFFFFFU) {
		//printf("erase pages success \n");
	} else {
		//printf("erase pages result %d\n", PageError);
	}

	HAL_FLASH_Lock();
}

void BSP_Flash_ClearFlashFlag() {

	__HAL_FLASH_CLEAR_FLAG(	FLASH_FLAG_EOP 		|
							FLASH_FLAG_WRPERR 	|
							FLASH_FLAG_PGAERR

							//FLASH_FLAG_OPERR  |
							//FLASH_FLAG_PGPERR |
							//FLASH_FLAG_PGSERR
							);
}


/**
 * @brief	���flash���������Ƿ�����
 */
#ifdef TEST
void BSP_Flash_Check(uint8_t PageIndex) {
	uint16_t i = 0;

	for(i = 0;i < STM32_FLASH_ONE_PAGE_BYTE;i++)
	{
		flash_check_buf[i] = i;
	}

	BSP_Flash_ErasePage(PageIndex, 1);
	BSP_FLASH_WritePage(PageIndex, flash_check_buf);
	memset(flash_check_buf, 0, STM32_FLASH_ONE_PAGE_BYTE);
	BSP_FLASH_ReadPage(PageIndex, flash_check_buf);

	for(i = 0; i < STM32_FLASH_ONE_PAGE_BYTE; i++)
	{
		printf("%x ", flash_check_buf[i]);
	}
	printf("\r\n");
}
#endif

