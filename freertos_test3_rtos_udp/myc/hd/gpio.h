/****************************************
**file name :
**function  :
****************************************/

#ifndef __GPIO_H
#define __GPIO_H

#include "stm32f10x_gpio.h"

typedef struct
{
    GPIO_TypeDef * gpiox;
    uint16_t gpio_pin;
    uint32_t rcc_clock;
}gpio_t;


#endif


