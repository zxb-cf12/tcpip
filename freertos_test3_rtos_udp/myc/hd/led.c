/****************************************
**file name :
**function  :
****************************************/

#include "led.h"


gpio_t sys_led = {
    SYS_GPIOX,            
    SYS_GPIO_PIN,   
    SYS_GPIO_CLC
};


/***************************
  @name: led_config()
  @func: config a gpio as led
  @return:
***************************/
static void l_led_config(
    gpio_t* led    // point to a led struct 
)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    
    RCC_APB2PeriphClockCmd(led->rcc_clock, ENABLE);

    
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Pin = led->gpio_pin;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;

    GPIO_Init(led->gpiox, &GPIO_InitStruct);
    
    led->gpiox->BRR = led->gpio_pin;
    led->gpiox->BSRR = 0;
}


/***************************
  @name: sys_led_config()
  @func: config sys run led
  @return:
***************************/
void g_sys_led_config()
{
    l_led_config(&sys_led);
}





