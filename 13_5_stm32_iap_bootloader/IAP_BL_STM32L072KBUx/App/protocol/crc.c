#include "crc.h"

/**
 * @brief	�Ѱ������������ġ����ݲ������ŵ�һ�������С�
 * 			��������ΪGetCRC�Ĳ���������GetCRC���ɻ�ȡһ��16bit��CRCУ���롣
 * @param	data	ԭʼ��������
 * @param	len		���鳤��
 * @retval	crcֵ 16bit
 */
void GetCRC(uint8_t *data, int16_t len, uint16_t *crc) {
    uint16_t i,j,carry_flag, temp;
    *crc = 0xffff;
    for (i = 0; i < len; i++) {
        *crc = *crc ^ data[i];
        for (j = 0; j < 8; j ++) {
            temp = *crc;
            carry_flag = temp & 0x0001;
            *crc = *crc >> 1;
            if (carry_flag == 1) {
                 *crc = *crc ^ 0xa001;
            }
         }
    }
}

/**
 * @brief	�յ����ݷ���󣬰ѷ���İ�ͷ�Ͱ�βȥ������ʣ�µĲ��ַŵ�һ�������У�
 * 			�Ѵ�������ΪCheckCRC�Ĳ�����������CheckCRC��
 * 			����ͨ���˺����ķ���ֵ���ж�CRC���Ƿ�ƥ�䡣
 * @param	data	ԭʼ��������
 * @param	len		���鳤��
 * @retval	У���Ƿ���ȷ
 */
bool CheckCRC(uint8_t *data, int16_t len) {
    if (len < 2) return false;
    uint16_t crcLow = data[len - 1];
    uint16_t crcHigh = data[len - 2];
    uint16_t crc_received = (uint16_t)(crcHigh << 8 | crcLow);
    uint16_t crc_new;
    GetCRC(data, len - 2, &crc_new);
    if (crc_new == crc_received) {
        return true;
    }
    return false;
}
