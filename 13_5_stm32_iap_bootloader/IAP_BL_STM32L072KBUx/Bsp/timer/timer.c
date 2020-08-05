#include "timer.h"

/*
 * ��ʱ����ԭʼƵ��Ϊ:72000000
 * Ԥ��Ƶ:720 �õ�Ƶ��Ϊ:100000
 * 100000 / 1000 = 100 ����Ϊ100��ʱ��, ��ʱ��1s����1000��,��1msһ��
 * 100 * 500 = 50000 ����Ϊ50000��ʱ��,��ʱ��500ms����һ��
 */

//STM32 Cubemx ���ö�ʱ����ʱ1mS
//https://www.cnblogs.com/xingboy/p/9897500.html

static uint32_t period_ms_;

static TimeUpCallback timeup_cb = NULL;



void BSP_TimeCounterInit() {
	TIM_MasterConfigTypeDef sMasterConfig = {0};

	htim7.Instance = TIM7;
	htim7.Init.Prescaler = 240-1;
	htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
	htim7.Init.Period = 50000-1;
	htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
	if (HAL_TIM_Base_Init(&htim7) != HAL_OK)
	{
		Error_Handler();
	}
	sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
	sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
	if (HAL_TIMEx_MasterConfigSynchronization(&htim7, &sMasterConfig) != HAL_OK)
	{
		Error_Handler();
	}
}


void BSP_SetTimeUpCallback(TimeUpCallback cb) {
	timeup_cb = cb;
}

void BSP_500msIRQ() {

	if(period_ms_ < 500) {
		period_ms_ = 0;
	} else {
		period_ms_ -= 500;
	}

	if(period_ms_ == 0) { //time up
		BSP_TimeCounterStop();

		if(timeup_cb != NULL) {
			timeup_cb();
		}
	}
}

/**
 * @brief	������ʱ��(ע�⣬ֻҪ��֤���˺�������д��Ƶ����ʵ��Ӳ��Ƶ��ƥ�䣬��������׼ȷ�� ����)
 * @param	��ʱ���ж�����,��λms
 * @retval	��
 */
void BSP_TimeCounterStart(uint32_t period_ms) {

	period_ms_ = period_ms;
	HAL_TIM_Base_Stop_IT(&htim7);

	//ע�����ö�ʱ�����ڵļĴ���λ16bit����ˣ���ʱ���������λ��65535��
	//ͨ������˵�����趨Ϊ50000�����õ��Ķ�ʱ��������Ϊ500ms�����ǽӽ����ڵ����ֵ�ˡ�
	//��ˣ����Ҫ��ʱ����500ms��Ӧ��ͨ����Σ��ۻ������մﵽĿ���ʱ��������һ���������������ڣ��ڶ�ʱ�������С�

	HAL_TIM_Base_Start_IT(&htim7);
}

void BSP_TimeCounterStop() {
	HAL_TIM_Base_Stop_IT(&htim7);
}
