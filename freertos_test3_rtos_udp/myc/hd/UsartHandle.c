/*************************************************************************
**		(C) COPYRIGHT 2011~  吴健超(DGUT)
**		    源码未经检录,使用需谨慎,修改请注明原作者
** 文件名 @file	   :
** 创建人 @author  : 吴健超
** 版本   @version : V 1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   :
** 硬件   @hardware：
** 其他   @other   ：
***************************************************************************/
#include "stm32f10x.h"                  // Device header
#include "UsartHandle.h"
#include "stdlib.h"
#include "stdio.h"


static DMA_Channel_TypeDef* DMAy_Channelx[]=
{
	DMA1_Channel1,DMA1_Channel2,DMA1_Channel3,DMA1_Channel4,DMA1_Channel5,DMA1_Channel6,DMA1_Channel7,
	DMA2_Channel1,DMA2_Channel2,DMA2_Channel3,DMA2_Channel4,DMA2_Channel5
};
static IRQn_Type IRQn_Typex[] = 
{
	DMA1_Channel1_IRQn,DMA1_Channel2_IRQn,DMA1_Channel3_IRQn,DMA1_Channel4_IRQn,DMA1_Channel5_IRQn,DMA1_Channel6_IRQn,DMA1_Channel7_IRQn,
	DMA2_Channel1_IRQn,DMA2_Channel2_IRQn,DMA2_Channel3_IRQn
};

/**************************************************************************
** 功能	@brief :
** 输入	@param :         
** 输出	@retval:        
***************************************************************************/
typedef struct
{
	uint32_t Periph_GPIOx;
	USART_TypeDef* USARTx;
	GPIO_TypeDef* GPIOx;
	uint16_t GPIO_Pin;
	uint8_t DmaId:4;
	uint8_t useDma:1;
}UartConfigAttr_t;	// 不可修改
UartConfigAttr_t UartTxCfgAttr[] = 
{
	{
		.Periph_GPIOx = RCC_APB2Periph_GPIOA,
		.USARTx      = USART1,
		.GPIOx		 = GPIOA,
		.GPIO_Pin 	= GPIO_Pin_9,
		.DmaId	 	= 3,
	},
	{
		.Periph_GPIOx = RCC_APB2Periph_GPIOA,
		.USARTx      = USART2,
		.GPIOx		 = GPIOA,
		.GPIO_Pin    = GPIO_Pin_2,
		.DmaId  	 = 6,
	},
	{
		.Periph_GPIOx = RCC_APB2Periph_GPIOB,
		.USARTx      = USART3,
		.GPIOx		 = GPIOB,
		.GPIO_Pin    = GPIO_Pin_10,
		.DmaId  	 = 1,
	},
	{
		.Periph_GPIOx = RCC_APB2Periph_GPIOC,
		.USARTx      = UART4,
		.GPIOx		 = GPIOC,
		.GPIO_Pin    = GPIO_Pin_10,
		.DmaId	 	 = 7+4,
	},
	{
		.Periph_GPIOx = RCC_APB2Periph_GPIOC,
		.USARTx      = UART5,
		.GPIOx		 = GPIOC,
		.GPIO_Pin    = GPIO_Pin_12,
	}
};
/**************************************************************************
** 功能	@brief : 串口发送初始化
** 输入	@param :         
** 输出	@retval:        
***************************************************************************/
void UartSendInit(uint8_t uartId,uint32_t baudRate,uint8_t useDma)
{
	if(uartId==0 || baudRate == 0)
	{
		return;
	}
	uartId-=1;
	UartTxCfgAttr[uartId].useDma = useDma;
	
	GPIO_InitTypeDef GPIO_InitStruct;
	USART_InitTypeDef USART_InitStruct;
	
	RCC_APB2PeriphClockCmd(UartTxCfgAttr[uartId].Periph_GPIOx,ENABLE);
	
	GPIO_InitStruct.GPIO_Speed 	= GPIO_Speed_50MHz;	
	GPIO_InitStruct.GPIO_Mode 	= GPIO_Mode_AF_PP;
	GPIO_InitStruct.GPIO_Pin	= UartTxCfgAttr[uartId].GPIO_Pin;
	GPIO_Init(UartTxCfgAttr[uartId].GPIOx,&GPIO_InitStruct);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);
	if(uartId==0)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	}
	else
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2<<(uartId-1),ENABLE);
	}
	
	USART_InitStruct.USART_BaudRate 		= baudRate;
	USART_InitStruct.USART_Mode 			= USART_Mode_Tx|(UartTxCfgAttr[uartId].USARTx->CR1&USART_Mode_Rx);
	USART_InitStruct.USART_Parity 			= USART_Parity_No;
	USART_InitStruct.USART_StopBits 		= USART_StopBits_1;
	USART_InitStruct.USART_WordLength 		= USART_WordLength_8b;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init      (UartTxCfgAttr[uartId].USARTx, &USART_InitStruct);	
	USART_Cmd       (UartTxCfgAttr[uartId].USARTx, ENABLE);
	USART_ClearFlag (UartTxCfgAttr[uartId].USARTx, USART_IT_TC);
	
	if(useDma)
	{
		if((uint32_t)DMAy_Channelx[UartTxCfgAttr[uartId].DmaId] < DMA2_BASE)
		{
			RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
		}
		else
		{
			RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2,ENABLE);
		}
		DMA_InitTypeDef DMA_InitStruct;
		DMA_InitStruct.DMA_MemoryBaseAddr 		= NULL;
		DMA_InitStruct.DMA_BufferSize			= 0;
		DMA_InitStruct.DMA_DIR 					= DMA_DIR_PeripheralDST;
		DMA_InitStruct.DMA_M2M 					= DMA_M2M_Disable;
		DMA_InitStruct.DMA_Mode 				= DMA_Mode_Normal;
		DMA_InitStruct.DMA_PeripheralBaseAddr 	= (uint32_t)&UartTxCfgAttr[uartId].USARTx->DR;
		DMA_InitStruct.DMA_MemoryDataSize 		= DMA_MemoryDataSize_Byte;
		DMA_InitStruct.DMA_PeripheralDataSize	= DMA_PeripheralDataSize_Byte;
		DMA_InitStruct.DMA_MemoryInc 			= DMA_MemoryInc_Enable;
		DMA_InitStruct.DMA_PeripheralInc 		= DMA_PeripheralInc_Disable;
		DMA_InitStruct.DMA_Priority 			= DMA_Priority_High;
		DMA_Init(DMAy_Channelx[UartTxCfgAttr[uartId].DmaId],&DMA_InitStruct);
		
		DMA_Cmd(DMAy_Channelx[UartTxCfgAttr[uartId].DmaId],DISABLE);
		USART_DMACmd(UartTxCfgAttr[uartId].USARTx,USART_DMAReq_Tx,ENABLE);
	}
}
/**************************************************************************
** 功能	@brief : 处理串口发送
** 输入	@param :         
** 输出	@retval:        
***************************************************************************/
void Uart_Send(uint8_t uartId,void *buf,uint32_t len)
{
	if(uartId == 0 || NULL == buf || len == 0)
	{
		return;
	}
	uartId-=1;
	
	if(UartTxCfgAttr[uartId].useDma)
	{
		while(DMA_GetCurrDataCounter(DMAy_Channelx[UartTxCfgAttr[uartId].DmaId]) !=0);
		if(UartTxCfgAttr[uartId].DmaId<7)
		{
			DMA_ClearFlag(0x0F<<(UartTxCfgAttr[uartId].DmaId<<2));
		}
		else
		{
			DMA_ClearFlag(0x10000000|0x0F<<((UartTxCfgAttr[uartId].DmaId-7)<<2));
		}
		DMAy_Channelx[UartTxCfgAttr[uartId].DmaId]->CMAR = (uint32_t)buf;
		DMAy_Channelx[UartTxCfgAttr[uartId].DmaId]->CNDTR = len;
		DMA_Cmd(DMAy_Channelx[UartTxCfgAttr[uartId].DmaId],ENABLE);
		while(DMA_GetCurrDataCounter(DMAy_Channelx[UartTxCfgAttr[uartId].DmaId]) !=0);
		DMA_Cmd(DMAy_Channelx[UartTxCfgAttr[uartId].DmaId],DISABLE);
	}
	else
	{
		uint32_t i;
		vTaskSuspendAll();
		for(i=0;i < len;i++)
		{
			USART_SendData(UartTxCfgAttr[uartId].USARTx,((uint8_t*)buf)[i]);
			while(USART_GetFlagStatus(UartTxCfgAttr[uartId].USARTx,USART_FLAG_TC)==RESET);
		}
		xTaskResumeAll();
	}
}
/**************************************************************************
** 功能	@brief :
** 输入	@param :         
** 输出	@retval:        
***************************************************************************/


UartConfigAttr_t UartRxCfgAttr[] = 
{
	{
		.Periph_GPIOx 	= RCC_APB2Periph_GPIOA,
		.USARTx      	= USART1,
		.GPIOx   		= GPIOA,
		.GPIO_Pin 		= GPIO_Pin_10,
		.DmaId	 		= 4,
	},
	{
		.Periph_GPIOx 	= RCC_APB2Periph_GPIOA,
		.USARTx      	= USART2,
		.GPIOx   		= GPIOA,
		.GPIO_Pin	 	= GPIO_Pin_3,
		.DmaId	 		= 5,
	},
	{
		.Periph_GPIOx 	= RCC_APB2Periph_GPIOB,
		.USARTx      	= USART3,
		.GPIOx   		= GPIOB,
		.GPIO_Pin 		= GPIO_Pin_11,
		.DmaId		 	= 2,
	},
	{
		.Periph_GPIOx 	= RCC_APB2Periph_GPIOC,
		.USARTx      	= UART4,
		.GPIOx   		= GPIOC,
		.GPIO_Pin 		= GPIO_Pin_11,
		.DmaId	 		= 7+3,
	},
	{
		.Periph_GPIOx 	= RCC_APB2Periph_GPIOD,
		.USARTx      	= UART5,
		.GPIOx   		= GPIOD,
		.GPIO_Pin 		= GPIO_Pin_2,
	}
};
UartRxData_t UartRxData[sizeof(UartRxCfgAttr)/sizeof(UartRxCfgAttr[0])];
void vTask_UsartRx_Thread(void *parm);

/**************************************************************************
** 功能	@brief :
** 输入	@param :         
** 输出	@retval:        
***************************************************************************/
void UartRecvInit(uint8_t uartId,uint32_t baudRate,uint8_t useDma,uint32_t RxPriority,
	uint32_t bufMaxLen,void (*RecvCB)(DataBufType_Typedef *dataBufType))
{
	if(uartId==0 || baudRate == 0)
	{
		return;
	}
	uartId-=1;
	UartRxCfgAttr[uartId].useDma 	= useDma;
	UartRxData[uartId].rxbuf  		= NULL;
	UartRxData[uartId].bufMaxLen 	= bufMaxLen;
	UartRxData[uartId].bufIndex 	= 0;
	UartRxData[uartId].RecvCB 		= RecvCB;
	//UartRxData[uartId].xSem_Rx 		= xSemaphoreCreateBinary();
	
	RCC_APB2PeriphClockCmd(UartRxCfgAttr[uartId].Periph_GPIOx,ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStruct;	
	GPIO_InitStruct.GPIO_Speed 	= GPIO_Speed_50MHz;	
	GPIO_InitStruct.GPIO_Mode	= GPIO_Mode_IN_FLOATING;
	GPIO_InitStruct.GPIO_Pin    = UartRxCfgAttr[uartId].GPIO_Pin;
	GPIO_Init(UartRxCfgAttr[uartId].GPIOx,&GPIO_InitStruct);
	if(uartId==0)
	{
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
	}
	else
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2<<(uartId-1),ENABLE);
	}
	
	USART_InitTypeDef USART_InitStruct;
	USART_InitStruct.USART_BaudRate 		= baudRate;
	USART_InitStruct.USART_Mode 			= USART_Mode_Rx|(UartRxCfgAttr[uartId].USARTx->CR1&USART_Mode_Tx);
	USART_InitStruct.USART_Parity 			= USART_Parity_No;
	USART_InitStruct.USART_StopBits 		= USART_StopBits_1;
	USART_InitStruct.USART_WordLength 		= USART_WordLength_8b;
	USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_Init      (UartRxCfgAttr[uartId].USARTx, &USART_InitStruct);	
	USART_Cmd       (UartRxCfgAttr[uartId].USARTx, ENABLE);
	USART_ClearFlag (UartRxCfgAttr[uartId].USARTx, USART_IT_IDLE|USART_IT_RXNE);
	if(uartId<3)
	{
		NVIC_SetPriority((IRQn_Type)(USART1_IRQn+uartId), RxPriority);
		NVIC_EnableIRQ  ((IRQn_Type)(USART1_IRQn+uartId));
	}
	else
	{
		NVIC_SetPriority((IRQn_Type)(UART4_IRQn+uartId-3), RxPriority);
		NVIC_EnableIRQ  ((IRQn_Type)(UART4_IRQn+uartId-3));
	}
	if(useDma)
	{
		DMA_InitTypeDef DMA_InitStruct;
		if(UartRxCfgAttr[uartId].DmaId <7 )
		{
			RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1,ENABLE);
		}
		else
		{
			RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA2,ENABLE);
		}
		DMA_InitStruct.DMA_MemoryBaseAddr 		= (uint32_t)UartRxData[uartId].rxbuf;
		DMA_InitStruct.DMA_BufferSize			= UartRxData[uartId].bufMaxLen;
		DMA_InitStruct.DMA_DIR 					= DMA_DIR_PeripheralSRC;
		DMA_InitStruct.DMA_M2M 					= DMA_M2M_Disable;
		DMA_InitStruct.DMA_Mode 				= DMA_Mode_Normal;
		DMA_InitStruct.DMA_PeripheralBaseAddr 	= (uint32_t)&UartRxCfgAttr[uartId].USARTx->DR;
		DMA_InitStruct.DMA_MemoryDataSize 		= DMA_MemoryDataSize_Byte;
		DMA_InitStruct.DMA_PeripheralDataSize	= DMA_PeripheralDataSize_Byte;
		DMA_InitStruct.DMA_MemoryInc 			= DMA_MemoryInc_Enable;
		DMA_InitStruct.DMA_PeripheralInc 		= DMA_PeripheralInc_Disable;
		DMA_InitStruct.DMA_Priority 			= DMA_Priority_High;
		DMA_Init(DMAy_Channelx[UartRxCfgAttr[uartId].DmaId],&DMA_InitStruct);
		
		USART_ITConfig(UartRxCfgAttr[uartId].USARTx, USART_IT_IDLE, DISABLE);				
		DMA_ITConfig(DMAy_Channelx[UartRxCfgAttr[uartId].DmaId],DMA_IT_TC|DMA_IT_TE,ENABLE);
		NVIC_SetPriority(IRQn_Typex[UartRxCfgAttr[uartId].DmaId],RxPriority);
		NVIC_EnableIRQ(IRQn_Typex[UartRxCfgAttr[uartId].DmaId]);
		
		DMA_Cmd(DMAy_Channelx[UartRxCfgAttr[uartId].DmaId],DISABLE);		
		USART_DMACmd(UartRxCfgAttr[uartId].USARTx,USART_DMAReq_Rx,ENABLE);
	}
	else
	{
		USART_ITConfig(UartRxCfgAttr[uartId].USARTx, USART_IT_RXNE|USART_IT_IDLE, DISABLE);
	}
	uint8_t *parmToTask = pvPortMalloc(1);
	*parmToTask = uartId;
	xTaskCreate(vTask_UsartRx_Thread,"vTask_UsartRx_Thread",300,parmToTask,1,NULL);
}

/**************************************************************************
** 功能	@brief : 
** 输入	@param :         
** 输出	@retval:        
***************************************************************************/
static void USART_IRQHandler(uint8_t uartId)
{
	uartId -= 1;
	if(USART_GetITStatus(UartRxCfgAttr[uartId].USARTx,USART_IT_IDLE))
	{
		__IO uint32_t i;	// 防止被编译器优化
		portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
		if(UartRxCfgAttr[uartId].useDma)
		{
			USART_ITConfig(UartRxCfgAttr[uartId].USARTx, USART_IT_IDLE, DISABLE);
			DMA_Cmd(DMAy_Channelx[UartRxCfgAttr[uartId].DmaId],DISABLE);
			USART_ClearITPendingBit(UartRxCfgAttr[uartId].USARTx, USART_IT_IDLE);
		}
		else
		{
			USART_ITConfig(UartRxCfgAttr[uartId].USARTx, USART_IT_IDLE|USART_IT_RXNE, DISABLE);
			USART_ClearITPendingBit(UartRxCfgAttr[uartId].USARTx, USART_IT_IDLE|USART_IT_RXNE);
		}
		i = UartRxCfgAttr[uartId].USARTx->SR;
		i = UartRxCfgAttr[uartId].USARTx->DR;
		
		//xSemaphoreGiveFromISR(UartRxData[uartId].xSem_Rx,&xHigherPriorityTaskWoken);
		if( xHigherPriorityTaskWoken != pdFALSE )
		{
			portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
		}
	}
	else if(USART_GetITStatus(UartRxCfgAttr[uartId].USARTx,USART_IT_RXNE))
	{
		UartRxData[uartId].rxbuf[UartRxData[uartId].bufIndex]= UartRxCfgAttr[uartId].USARTx->DR;
		UartRxData[uartId].bufIndex++;
		USART_ClearITPendingBit(UartRxCfgAttr[uartId].USARTx, USART_IT_RXNE);
		if(UartRxData[uartId].bufIndex>=UartRxData[uartId].bufMaxLen)
		{
			portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
			USART_ITConfig(UartRxCfgAttr[uartId].USARTx, USART_IT_IDLE|USART_IT_RXNE, DISABLE);
			
			xSemaphoreGiveFromISR(UartRxData[uartId].xSem_Rx,&xHigherPriorityTaskWoken);
			
            if( xHigherPriorityTaskWoken != pdFALSE )
			{
				portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
			}
		}
	}
}
/**************************************************************************
** 功能	@brief : 处理串口接收帧结束通知系统
** 输入	@param :         
** 输出	@retval:        
***************************************************************************/
void USART1_IRQHandler(void)
{
	USART_IRQHandler(1);
}
void USART2_IRQHandler(void)
{
	USART_IRQHandler(2);
}
void USART3_IRQHandler(void)
{
	USART_IRQHandler(3);
}
void UART4_IRQHandler(void)
{
	USART_IRQHandler(4);
}
void UART5_IRQHandler(void)
{
	USART_IRQHandler(5);
}
/**************************************************************************
** 功能	@brief :
** 输入	@param :         
** 输出	@retval:        
***************************************************************************/
static void DMA_Channel_IRQHandler(uint8_t uartId,uint8_t DMA_id)
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	uartId-= 1;
	DMA_id -= 1;
	if(DMA_id<7)
	{
		if(DMA_GetITStatus(0x02<<(DMA_id<<2)))
		{
			USART_ITConfig(UartRxCfgAttr[uartId].USARTx, USART_IT_IDLE, DISABLE);
			DMA_Cmd(DMAy_Channelx[UartRxCfgAttr[uartId].DmaId],DISABLE);
			xSemaphoreGiveFromISR(UartRxData[uartId].xSem_Rx,&xHigherPriorityTaskWoken);
		}
		DMA_ClearITPendingBit(0x0F<<(DMA_id<<2));
	}
	else
	{
		DMA_id-=7;
		if(DMA_GetITStatus(0x10000000|0x02<<(DMA_id<<2)))
		{
			USART_ITConfig(UartRxCfgAttr[uartId].USARTx, USART_IT_IDLE, DISABLE);
			DMA_Cmd(DMAy_Channelx[UartRxCfgAttr[uartId].DmaId],DISABLE);
			xSemaphoreGiveFromISR(UartRxData[uartId].xSem_Rx,&xHigherPriorityTaskWoken);
		}
		DMA_ClearITPendingBit(0x10000000|0x0F<<(DMA_id<<2));
	}
	
	if( xHigherPriorityTaskWoken != pdFALSE )
	{
		portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
	}
}
/**************************************************************************
** 功能	@brief : 串口DMA接收缓冲区满时通知系统
** 输入	@param :         
** 输出	@retval:        
***************************************************************************/
void DMA1_Channel5_IRQHandler(void)
{
	DMA_Channel_IRQHandler(1,5);
}
void DMA1_Channel6_IRQHandler(void)
{
	DMA_Channel_IRQHandler(2,6);
}
void DMA1_Channel3_IRQHandler(void)
{
	DMA_Channel_IRQHandler(3,3);
}
void DMA2_Channel3_IRQHandler(void)
{
	DMA_Channel_IRQHandler(4,10);
}
/**************************************************************************
** 功能	@brief : 串口接收结束处理缓冲区任务
** 输入	@param :          
** 输出	@retval:        
***************************************************************************/
static char Max485_2_buf[620] = {0};

void vTask_UsartRx_Thread(void *parm)
{
	uint8_t uartId = *(uint8_t*)parm;
    xSemaphoreGive(UartRxData[uartId].xSem_Rx);
	while(1)
	{
		if(xSemaphoreTake(UartRxData[uartId].xSem_Rx,portMAX_DELAY))
		{
			if(uartId == 3)		// 4 - 1
			{
				DataBufType_Typedef dataBufType;
				dataBufType.dat = UartRxData[uartId].rxbuf;
				dataBufType.datLen = UartRxData[uartId].bufIndex;
				dataBufType.needFree = 0;

				UartRxData[uartId].rxbuf = Max485_2_buf;

				UartRxData[uartId].bufIndex=0;
				USART_ITConfig(UartRxCfgAttr[uartId].USARTx, USART_IT_RXNE, ENABLE);
				USART_ITConfig(UartRxCfgAttr[uartId].USARTx, USART_IT_IDLE, ENABLE);

				if(dataBufType.datLen && UartRxData[uartId].RecvCB)
				{
					UartRxData[uartId].RecvCB(&dataBufType);
				}
			}
			else
			{
				char *newBuf;
				DataBufType_Typedef dataBufType;
				
				dataBufType.dat = UartRxData[uartId].rxbuf;
				if(UartRxCfgAttr[uartId].useDma)
				{
					dataBufType.datLen = UartRxData[uartId].bufMaxLen-DMAy_Channelx[UartRxCfgAttr[uartId].DmaId]->CNDTR;
				}
				else
				{
					dataBufType.datLen = UartRxData[uartId].bufIndex;
				}
				dataBufType.needFree = 1;
				newBuf = pvPortMalloc(UartRxData[uartId].bufMaxLen);
				
				if(newBuf)
				{
					UartRxData[uartId].rxbuf = newBuf;
					if(UartRxCfgAttr[uartId].useDma)
					{
						DMAy_Channelx[UartRxCfgAttr[uartId].DmaId]->CMAR = (uint32_t)UartRxData[uartId].rxbuf;
						DMAy_Channelx[UartRxCfgAttr[uartId].DmaId]->CNDTR = UartRxData[uartId].bufMaxLen;
						DMA_Cmd(DMAy_Channelx[UartRxCfgAttr[uartId].DmaId],ENABLE);
						USART_ITConfig(UartRxCfgAttr[uartId].USARTx, USART_IT_IDLE, ENABLE);
					}
					else
					{
						UartRxData[uartId].bufIndex=0;
						USART_ITConfig(UartRxCfgAttr[uartId].USARTx, USART_IT_RXNE, ENABLE);
						USART_ITConfig(UartRxCfgAttr[uartId].USARTx, USART_IT_IDLE, ENABLE);
					}
				}
				else	// malloc new buf failure,so have to remalloc
				{
					vTaskDelay(10);
					xSemaphoreGive(UartRxData[uartId].xSem_Rx);
				}
				// deal usart receive dat
				if(dataBufType.datLen && UartRxData[uartId].RecvCB)
				{
					UartRxData[uartId].RecvCB(&dataBufType);
				}
			}
		}
	}
}

#if DB_PRINTF_ENABLE	==1
#pragma import(__use_no_semihosting)             
//标准库需要的支持函数                 
struct __FILE 
{ 
	int handle; 
}; 

FILE __stdout;       
//定义_sys_exit()以避免使用半主机模式    
void _sys_exit(int x) 
{ 
	x = x; 
} 
//重定义fputc函数 
int fputc(int ch, FILE *f)
{ 	
	while((UART5->SR&0X40)==0);//循环发送,直到发送完毕   
	UART5->DR = (uint8_t) ch;      
	return ch;
}

void dump_array(void *buf,uint16_t len)
{
	for (uint16_t i = 0; i < len; i++)
	{
		if (i % 20 == 0)
		{
			printf("\r\n");
		}
		printf("%02X ", ((uint8_t *)buf)[i]);
	}
	printf("\r\n");
}

#endif


void rs485_gpio_init()
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB,ENABLE);	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_3;
	GPIO_Init(GPIOB,&GPIO_InitStruct);	
	GPIO_ResetBits(GPIOB,GPIO_Pin_3);
}

void g_all_usart_config()
{
    rs485_gpio_init();
    UartSendInit(5,9600,0);
}
