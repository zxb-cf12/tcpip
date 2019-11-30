/****************************************
**file name :
**function  :
****************************************/

#ifndef __LED_FLASH
#define __LED_FLASH

#include "delay.h"
#include "led.h"

void g_led_single_flash(gpio_t led,u32 freq);
void g_led_double_flash(gpio_t led,u32 freq);



void g_set_led(gpio_t led,u32 freq);

void g_reset_led(gpio_t led,u32 freq);

#endif

