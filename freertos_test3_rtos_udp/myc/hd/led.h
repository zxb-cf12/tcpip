/****************************************
**file name :
**function  :
****************************************/

#ifndef __LED
#define __LED

#include "stm32f10x_gpio.h"
#include "gpio.h"

#define SYS_GPIOX        GPIOC      
#define SYS_GPIO_PIN     GPIO_Pin_3
#define SYS_GPIO_CLC     RCC_APB2Periph_GPIOC




extern gpio_t sys_led;

void g_sys_led_config(void);


#endif


