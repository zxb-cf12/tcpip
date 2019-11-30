/****************************************
**file name :
**function  :
****************************************/

#ifndef __USART_H
#define __USART_H

#include "gpio.h"
#include "delay.h"
#include "stm32f10x_gpio.h"
#include "stm32f10x_usart.h"



/**************************************************/
enum 
{
    RECIEVE_TIMEOUT = 0,
    RECIEVE_SUCCESS = 1,
};

enum
{
    STEP_START = 0,
    STEP_LEN,
    STEP_RECV,
    STEP_CRC,
    STEP_END,
};
/**************************************************/

/**************************************************/
typedef struct USART_INFO
{
    USART_TypeDef* usartx;
    uint32_t rcc_periph_usart;
    gpio_t* usartx_tx;
    gpio_t* usartx_rx;
    uint32_t baud;
    NVIC_InitTypeDef* usartx_nvic;
}usart_info_t;

typedef struct USART_DATA
{
    uint8_t startbyte;        /* packet start byte*/
    uint8_t endbyte;          /* packet end byte*/
    uint8_t state;            /* packet recv state*/
    uint8_t len;              /* data bytes need recv*/
    uint8_t data[48];         /* recv data*/ 
}data_pack_t;
/**************************************************/

/**************************************************/
extern data_pack_t usart1_data;
extern data_pack_t usart2_data;
extern data_pack_t usart3_data;



/**************************************************/

void g_usart_config(void);
void g_usartx_send_data(USART_TypeDef* usartx, uint8_t* data, uint8_t data_len);
void g_USART1_RevData_Process(uint8_t data, data_pack_t* recv_data);
void g_USART2_RevData_Process(uint8_t data, data_pack_t* recv_data);
void g_USART3_RevData_Process(uint8_t data, data_pack_t* recv_data);


/**************************************************/
#endif
/**************************************************/

