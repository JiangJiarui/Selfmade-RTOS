#include	"./usart/bsp_usart.h"

 /**
  * @brief  配置嵌套向量中断控制器NVIC
  * @param  无
  * @retval 无
  */
//static void NVIC_Config(void)
//{
//	NVIC_InitTypeDef NVIC_InitStructure;
//	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
//	NVIC_InitStructure.NVIC_IRQChannel = DEBUG_USART_IRQ;	
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
//	
//	NVIC_Init(&NVIC_InitStructure);
//}

 /**
  * @brief  DEBUG_USART GPIO 配置,工作模式配置。115200 8-N-1 ，中断接收模式
  * @param  无
  * @retval 无
  */
void USART_Config(void)
{
	/* GPIO时钟 */
	RCC_AHB1PeriphClockCmd(DEBUG_USART_RX_GPIO_CLK|DEBUG_USART_TX_GPIO_CLK,ENABLE);
	/* USART时钟 */
  RCC_APB2PeriphClockCmd(DEBUG_USART_CLK, ENABLE);
	
/************* GPIO复用功能配置 与USART连接 *********************/
	GPIO_InitTypeDef GPIO_InitStructure;
	
	/* 配置TX 推挽悬浮 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART_TX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	
	GPIO_Init(DEBUG_USART_TX_GPIO_PORT, &GPIO_InitStructure);
	
	GPIO_PinAFConfig(DEBUG_USART_TX_GPIO_PORT, DEBUG_USART_TX_SOURCE, DEBUG_USART_TX_AF);
	
	/* 配置RX 推挽上拉 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_2MHz;
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;
	GPIO_InitStructure.GPIO_Pin = DEBUG_USART_RX_PIN;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF;
	
	GPIO_Init(DEBUG_USART_RX_GPIO_PORT, &GPIO_InitStructure);

	GPIO_PinAFConfig(DEBUG_USART_RX_GPIO_PORT, DEBUG_USART_RX_SOURCE, DEBUG_USART_RX_AF);
	
/********************** 配置USART ******************************/

	USART_InitTypeDef USART_InitStructure;
	
	USART_InitStructure.USART_BaudRate = DEBUG_USART_BAUDRATE;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	
	USART_Init(DEBUG_USART,&USART_InitStructure);

//	NVIC_Config();
//	
//	/* 使能串口接收中断 */
//	USART_ITConfig(DEBUG_USART,USART_IT_RXNE, ENABLE);
//	/* 串口使能 */
	USART_Cmd(DEBUG_USART, ENABLE);
}


/***************** 发送字符 **********************/
void Usart_SendByte(USART_TypeDef* pUSARTx, uint8_t ch)
{
	USART_SendData(pUSARTx,ch);

	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET)
		;	
}

/*****************  发送字符串 **********************/
void Usart_SendString(USART_TypeDef* pUSARTx, char* str)
{
	unsigned int k = 0;
	do 
  {
		Usart_SendByte( pUSARTx, *(str + k) );
		k++;
  } while(*(str + k)!='\0');
	
	  while(USART_GetFlagStatus(pUSARTx,USART_FLAG_TC)==RESET)
			;
}

/*****************  发送一个16位数 **********************/
void Usart_SendHalfWord( USART_TypeDef * pUSARTx, uint16_t ch)
{
	uint8_t temp_h, temp_l;
		/* 取出高八位 */
	temp_h = (ch&0XFF00)>>8;  
	/* 取出低八位 */
	temp_l = ch&0XFF;
	
	/* 发送高八位 */
	USART_SendData(pUSARTx,temp_h);	
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET)
		;
	
	/* 发送低八位 */
	USART_SendData(pUSARTx,temp_l);	
	while (USART_GetFlagStatus(pUSARTx, USART_FLAG_TXE) == RESET)
		;	
}


/* printf重定向 */
int fputc(int ch, FILE *f)
{
		/* 发送一个字节数据到串口 */
		USART_SendData(DEBUG_USART, (uint8_t) ch);
		
		/* 等待发送完毕 */
		while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_TXE) == RESET);		
	
		return (ch);
}

/* scanf重定向 */
int fgetc(FILE *f)
{
		/* 等待串口输入数据 */
		while (USART_GetFlagStatus(DEBUG_USART, USART_FLAG_RXNE) == RESET);

		return (int)USART_ReceiveData(DEBUG_USART);
}

/******************************** END ***********************************/















