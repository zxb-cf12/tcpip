/****************************************
**file name :
**function  :
****************************************/

#include "usart.h"

/*******************************************************************************/
gpio_t usart1_tx = {GPIOA, GPIO_Pin_9, RCC_APB2Periph_GPIOA};
gpio_t usart1_rx = {GPIOA, GPIO_Pin_10, RCC_APB2Periph_GPIOA};
gpio_t usart2_tx = {GPIOA, GPIO_Pin_2, RCC_APB2Periph_GPIOA};
gpio_t usart2_rx = {GPIOA, GPIO_Pin_3, RCC_APB2Periph_GPIOA};
gpio_t usart3_tx = {GPIOB, GPIO_Pin_10, RCC_APB2Periph_GPIOB};
gpio_t usart3_rx = {GPIOB, GPIO_Pin_11, RCC_APB2Periph_GPIOB};

NVIC_InitTypeDef usart1_nvic = {USART1_IRQn, 0X03, 0X01, ENABLE};
NVIC_InitTypeDef usart2_nvic = {USART2_IRQn, 0X03, 0X02, ENABLE};
NVIC_InitTypeDef usart3_nvic = {USART3_IRQn, 0X03, 0X03, ENABLE}; 



/*******************************************************************************/

const usart_info_t USART1_INFO = {
    USART1, 
    RCC_APB2Periph_USART1,
    &usart1_tx,
    &usart1_rx,
    19200,  
    &usart1_nvic
};

const usart_info_t USART2_INFO = {
    USART2,
    RCC_APB1Periph_USART2,
    &usart2_tx,
    &usart2_rx,
    9600,
    &usart2_nvic
};

const usart_info_t USART3_INFO = {
    USART3,
    RCC_APB1Periph_USART3,
    &usart3_tx,
    &usart3_rx,
    19200,
    &usart3_nvic
};
/*******************************************************************************/


/***************************
  @name: l_usartx_gpio_config()
  @func: 
  @return:
***************************/
static void l_usartx_gpio_config(
    const usart_info_t* uartx   /*Point to usartx information*/
)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(uartx->usartx_tx->rcc_clock | RCC_APB2Periph_AFIO, ENABLE);//RCC_APB2Periph_AFIO
   
    GPIO_InitStructure.GPIO_Pin = uartx->usartx_tx->gpio_pin;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_Init(uartx->usartx_tx->gpiox, &GPIO_InitStructure);
    
    GPIO_InitStructure.GPIO_Pin = uartx->usartx_rx->gpio_pin; 
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
    GPIO_Init(uartx->usartx_rx->gpiox, &GPIO_InitStructure);
}
/***************************
  @name: l_usartx_func_config()
  @func: 
  @return:
***************************/
static void l_usartx_func_config(
    const usart_info_t* uartx   /*Point to usartx information*/
)
{
    USART_InitTypeDef USART_InitStruct;
   
    if(USART1 == uartx->usartx)
        RCC_APB2PeriphClockCmd(uartx->rcc_periph_usart, ENABLE);
    else
        RCC_APB1PeriphClockCmd(uartx->rcc_periph_usart, ENABLE);    

    USART_InitStruct.USART_BaudRate = uartx->baud;
    USART_InitStruct.USART_WordLength = USART_WordLength_8b;
    USART_InitStruct.USART_StopBits = USART_StopBits_1;
    USART_InitStruct.USART_Parity = USART_Parity_No;
    USART_InitStruct.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;
    USART_InitStruct.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_Init(uartx->usartx, &USART_InitStruct);   
    USART_Cmd(uartx->usartx, ENABLE);
   
    USART_ITConfig(uartx->usartx, USART_IT_RXNE, uartx->usartx_nvic->NVIC_IRQChannelCmd); 
    USART_ClearFlag(uartx->usartx, USART_FLAG_TC);
}
/***************************
  @name: l_usartx_nvic_config()
  @func: 
  @return:
***************************/
static void l_usartx_nvic_config(
     const usart_info_t* uartx   /*Point to usartx information*/
)
{
    NVIC_InitTypeDef  NVIC_InitStructure;
    
    NVIC_InitStructure.NVIC_IRQChannel = uartx->usartx_nvic->NVIC_IRQChannel;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = uartx->usartx_nvic->NVIC_IRQChannelPreemptionPriority;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = uartx->usartx_nvic->NVIC_IRQChannelSubPriority;
    NVIC_InitStructure.NVIC_IRQChannelCmd = uartx->usartx_nvic->NVIC_IRQChannelCmd;
    NVIC_Init(&NVIC_InitStructure);

}

/***************************
  @name: l_usartx_config()
  @func: 
  @return:
***************************/
static void l_usartx_config(
    const usart_info_t* uartx  /* point to usartx object*/
)
{
    l_usartx_gpio_config(uartx);
    l_usartx_func_config(uartx);
    l_usartx_nvic_config(uartx);
}

/***************************
  @name: g_usartx_send_cmd()
  @func: 
  @return:
***************************/
void g_usartx_send_data(
    USART_TypeDef* usartx,  /*usart object*/
    uint8_t* data,           /*send data*/
    uint8_t data_len         /*send data len*/
)
{  
    uint8_t i = 0;
    for(i = 0;i<data_len;i++){ 
        while(USART_GetFlagStatus(usartx, USART_FLAG_TC) != SET);        
        USART_SendData(usartx, data[i]);       
    }
}

/***************************
  @name: g_usart_config()
  @func: config usart123
  @return:
***************************/
void g_usart_config()
{
    l_usartx_config(&USART1_INFO);
    l_usartx_config(&USART2_INFO); 
    l_usartx_config(&USART3_INFO);
}

/***************************
  @name: USART2_IRQHandler()
  @func: 
  @return:
***************************/
void USART1_IRQHandler()
{
    uint8_t data = 0;
    if(USART_GetITStatus(USART1, USART_IT_RXNE) != RESET){
        data = USART_ReceiveData(USART1);
        g_USART1_RevData_Process(data, &usart1_data);
    }   
}


/***************************
  @name: USART2_IRQHandler()
  @func: 
  @return:
***************************/
void USART2_IRQHandler()
{
    uint8_t data = 0;
    if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET){
        data = USART_ReceiveData(USART2);
        g_USART2_RevData_Process(data, &usart2_data);
    }   
}

/***************************
  @name: USART2_IRQHandler()
  @func: 
  @return:
***************************/
void USART3_IRQHandler()
{
    uint8_t data = 0;
    if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET){
        data = USART_ReceiveData(USART3);
        g_USART3_RevData_Process(data, &usart3_data);
    }   
}
