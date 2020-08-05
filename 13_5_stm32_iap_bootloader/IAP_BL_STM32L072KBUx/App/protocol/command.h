#ifndef APP_COMMAND_H_
#define APP_COMMAND_H_

#include "includes.h"

const static uint8_t HEAD[1] = {0xFF};
const static uint8_t TAIL[4] = {0xE3, 0xD1, 0xAC, 0xB5};

typedef enum CommandType_ {
	SetValue	 	= 0x01,        //��λ������λ�������ݵ���������
	GetValue 		= 0x02,        //��λ������λ����ȡ���ݵ���������
	ReportValue 	= 0x03         //��λ����������λ���ϱ����ݵ���������

} CommandType;

typedef struct ResponsePkg_ {
	uint8_t* 	res_buffer;			//���ɵķ������ݰ���������
	uint16_t 	res_buffer_len;		//���ɵķ������ݰ��������鳤��
	uint8_t 	command_type;		//��������
	uint8_t 	command;			//��������
	uint8_t* 	param_buf;			//���������������
	uint16_t 	param_len;			//��������������鳤��
} ResponsePkg;


#define COMMAND_SHAKE_HANDS			0X01
#define COMMAND_DOWNLOAD_START		0X02
#define COMMAND_DOWNLOAD_STOP		0X03
#define COMMAND_DOWNLOAD_DATA		0X04
#define COMMAND_DOWNLOAD_CHECKSUM	0X05

extern int APP_GetProtocolResponsePKG(ResponsePkg *resPkg);

#endif //APP_COMMAND_H_
