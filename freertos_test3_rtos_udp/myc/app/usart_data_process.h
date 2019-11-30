/****************************************
**file name :
**function  :
****************************************/

#ifndef __USART_DATA_PROCESS
#define __USART_DATA_PROCESS

#include "usart.h"


#define USART1_RECV_DATA  (usart1_data.state == STEP_END)
#define USART2_RECV_DATA  (usart2_data.state == STEP_END)
#define USART3_RECV_DATA  (usart3_data.state == STEP_END)




void g_handle_usart1_data(void);
void g_handle_usart2_data(void);
void g_handle_usart3_data(void);

void g_USART1_RevData_Process(uint8_t data, data_pack_t* recv_data);
void g_USART2_RevData_Process(uint8_t data, data_pack_t* recv_data);
void g_USART3_RevData_Process(uint8_t data, data_pack_t* recv_data);


#endif

