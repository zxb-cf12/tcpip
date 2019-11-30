#ifndef __USART_HANDLE_H__
#define __USART_HANDLE_H__

#include "stm32f10x.h"
#include "FreeRTOS.h"
#include "timers.h"
#include "queue.h"
#include "stdio.h"
typedef struct
{
	uint8_t needFree;	// 0:使用ROM 用完无需 free，1:使用RAM 用完需要free
	uint32_t datLen;
	char *dat;
}DataBufType_Typedef;	// 函数之间数据传递，在数据最后不再使用后，如果needFree=1，需要free掉dat的内存

typedef void * SemaphoreHandle_t;


#define DB_PRINTF_ENABLE	1


#ifndef os_printf
#if DB_PRINTF_ENABLE	== 1
	#define os_printf(format,...)										\
		vTaskSuspendAll();												\
		printf("%s[%d]:" format,__FILE__,__LINE__, ##__VA_ARGS__);		\
		xTaskResumeAll()
	
		extern void dump_array(void *buf,uint16_t len);	
	#define os_dumpArray(head,str,len)								\
		do{															\
            GPIO_SetBits(GPIOB,GPIO_Pin_3);                       \
			vTaskSuspendAll();										\
			dump_array(str,len);									\
			xTaskResumeAll();										\
            GPIO_ResetBits(GPIOB,GPIO_Pin_3);                       \
		}while(0)
#else
	#define os_printf(format,...)
	#define os_dumpArray(head,str,len)
#endif
#endif



#ifndef use_rtos
#define use_rtos
#endif

#ifdef use_rtos

#define semBINARY_SEMAPHORE_QUEUE_LENGTH	( ( uint8_t ) 1U )
#define semSEMAPHORE_QUEUE_ITEM_LENGTH		( ( uint8_t ) 0U )
#define semGIVE_BLOCK_TIME					( ( TickType_t ) 0U )

#define xSemaphoreTake( xSemaphore, xBlockTime )		                xQueueReceive( ( QueueHandle_t ) ( xSemaphore ), NULL, ( xBlockTime ))
#define xSemaphoreGive( xSemaphore )		                            xQueueGenericSend( ( QueueHandle_t ) ( xSemaphore ), NULL, semGIVE_BLOCK_TIME, queueSEND_TO_BACK )
#define xSemaphoreGiveFromISR( xSemaphore, pxHigherPriorityTaskWoken )	xQueueGiveFromISR( ( QueueHandle_t ) ( xSemaphore ), ( pxHigherPriorityTaskWoken ) )
#endif

typedef struct
{
	char *rxbuf;
	uint32_t bufIndex;
	uint32_t bufMaxLen;
	xSemaphoreHandle xSem_Rx;
	void (*RecvCB)(DataBufType_Typedef *dataBufType);
}UartRxData_t;

//printf("%s[%d]:%s:%d\n",__FILE__,__LINE__,head,len);	\

void g_all_usart_config(void);
void UartSendInit(uint8_t uartId,uint32_t baudRate,uint8_t useDma);	
void Uart_Send(uint8_t usartId,void *buf,uint32_t len);
void UartRecvInit(uint8_t uartId,uint32_t baudRate,uint8_t useDma,uint32_t RxPriority,
uint32_t bufMaxLen,void (*RecvCB)(DataBufType_Typedef *dataBufType));

#endif
