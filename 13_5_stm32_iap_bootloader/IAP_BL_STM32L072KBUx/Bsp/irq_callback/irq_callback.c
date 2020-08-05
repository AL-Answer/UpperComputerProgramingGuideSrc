#include "irq_callback.h"

static bool isTimerCallbackHappend = false;

void BSP_EnableIrq() {
	__set_PRIMASK(0);//���ж�
}

void BSP_DisableIrq() {
	__set_PRIMASK(1);//�ر��ж�
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
	APP_UartProcess(RxBuf[0]);
	BSP_UartRecvReset();//����ʹ�ܴ��ڽ����ж�
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
	if (htim == (&htim7)) {
		//ע�����жϷ�ʽ������ʱ����ʱ�򣬻�ֱ�Ӵ���һ�α��ж�
		if (isTimerCallbackHappend == false) {
			isTimerCallbackHappend = true;
			return;
		} else {
			BSP_500msIRQ();
		}
	}
}
