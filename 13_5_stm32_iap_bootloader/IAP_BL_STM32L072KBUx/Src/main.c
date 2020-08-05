/* USER CODE BEGIN Header */
//�������д��룬�ᱣ�����ļ��ڵ����д��벻�� cubemx�������ɵ�ʱ�򸲸ǵ���#include "includes.h"

#include "includes.h"

static bool startup_wait_flag = true;
static bool is_startup_wait_timeout = false;

static void TimeUp() {
	Main_ClearStartupFlag(true);
}

//��ʱ����ʱ�жϺ󣬻���ô˺����� ��true
//�յ����ع̼�����Ҳ����ô˺�������false
void Main_ClearStartupFlag(bool isTimeout) {
	startup_wait_flag = false;
	is_startup_wait_timeout = isTimeout;
}

static void Main_ProcessWhileLoop() {
	//��9600�����ʵ��ٶ���,����1byte��Ҫ����1ms.һ����С������Э������8byte.������8ms����һ��Э����
	//��Ҫ��֤����������ݵ��ٶȴ��ڽ����ٶ�,�Ӷ���֤�������ݵ�˫���湻��,�����ۻ�����.��������delay �Ƽ�5ms
	HAL_Delay(5);//5ms����һ���������ݰ�
	APP_ProcessProtocolReceivedPkgAtMain();
}

int main(void)
{
	HAL_Init();
	BSP_Init();

	//��ʱ������ʼ�ȴ������ع̼���ָ��
	BSP_SetTimeUpCallback(TimeUp);
	BSP_TimeCounterStart(5000); //wait 5s
	while(startup_wait_flag) {
		Main_ProcessWhileLoop();//�ڵȴ���ʱ�Ĺ����У�����Ƿ��յ�����ָ��
	}

	//�������Ϊ�ȴ���ʱ�����п���app����ת��appִ��
	if(is_startup_wait_timeout == true && APP_IsHadAppFlag()) {
		//printf("start app\n");
		APP_RunApp();
	}
	// APP_EraseAppUsingPages();//����ʱ��ʹ��

	while (1)
	{
		Main_ProcessWhileLoop();
	}
}

//ʹ��CubeMx���ɴ���󣬰�����Ĵ��붼ɾ�������ɡ�
/* USER CODE END Header */
