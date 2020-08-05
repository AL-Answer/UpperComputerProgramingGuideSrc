#include "uart.h"

//TrueSTUDIOʹ��printfͨ�����ڴ�ӡ
//https://blog.csdn.net/u012308586/article/details/89514986

#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE  int __io_putchar(int ch)
PUTCHAR_PROTOTYPE
{
	BSP_Uart1SendData((uint8_t*)&ch, 1);
  return ch;
}
#endif


uint8_t RxBuf[1];
bool is_set_uart_irq_recv_failed = false;
bool is_first_time_set_up_uart_it = false;


void BSP_Uart1Init() {
	huart1.Instance = USART1;
	huart1.Init.BaudRate = 57600;
	huart1.Init.WordLength = UART_WORDLENGTH_8B;
	huart1.Init.StopBits = UART_STOPBITS_1;
	huart1.Init.Parity = UART_PARITY_NONE;
	huart1.Init.Mode = UART_MODE_TX_RX;
	huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	huart1.Init.OverSampling = UART_OVERSAMPLING_16;
	huart1.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
	huart1.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
	if (HAL_UART_Init(&huart1) != HAL_OK)
	{
		Error_Handler();
	}
}


/**
 * @brief	��485оƬ����Ϊ����ģʽ
 * @param	��
 * @retval	��
 */
static void SetIntoRevcMode() {
	HAL_GPIO_WritePin(GPIO_PORT_485_CTRL, GPIO_PIN_485_CTRL, GPIO_PIN_RESET);
}

/**
 * @brief	��485оƬ����Ϊ����ģʽ
 * @param	��
 * @retval	��
 */
static void SetIntoSendMode() {
	HAL_GPIO_WritePin(GPIO_PORT_485_CTRL, GPIO_PIN_485_CTRL, GPIO_PIN_SET);
}

/**
 * @brief	uart1��ʼ�������жϺ���
 * @param	��
 * @retval	��
 */
void BSP_UartRecvReset() {
	uint16_t i = 0;

	SetIntoRevcMode();

	//�ڽ��յ����ٵĴ���������,�ᵼ������ ���ڽ����ж�ʧ��(����ĸ��ٴ�������,��ָ���� �����ָ����9600�����ʵ��ٶ�)
	//��������,����Ƿ������ô����ж�ʧ��,������������

	while(HAL_UART_Receive_IT(&huart1, RxBuf, RX_BUF_LEN) != HAL_OK ) {
		i++;
		if( i > 10000 ) {
			huart1.RxState = HAL_UART_STATE_READY;
			__HAL_UNLOCK(&huart1);
			i = 0;
		}
	}
}

/**
 * @brief	uart1 ��������
 * @param	data ����ָ��
 * @param	len ���ݳ���
 * @retval	��
 */
void BSP_Uart1SendData(uint8_t *data, uint16_t len) {
	SetIntoSendMode();
	HAL_UART_Transmit(&huart1, data, len, HAL_MAX_DELAY);
	SetIntoRevcMode();
}

/**
 * @brief	��������жϴ���
 * @retval	��
 */
void BSP_ProcessUartRecvError(void) {
	//�ڽ��յ����ٵĴ���������,�ᵼ������ ���ڽ����ж�ʧ��(����ĸ��ٴ�������,��ָ���� �����ָ����9600�����ʵ��ٶ�)
	//��������,����Ƿ������ô����ж�ʧ��,������������
	if(is_set_uart_irq_recv_failed == true) {
		BSP_Uart1Init();
		is_set_uart_irq_recv_failed = false;
	}
}
