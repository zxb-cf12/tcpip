/****************************************
**file name :
**function  :
****************************************/

#include "led_flash.h"
#include "stm32f10x_it.h"



/***************************
  @name: s_gpio_turn
  @func: gpiox pinx flip
  @return: NULL
***************************/
void s_gpio_turn(gpio_t* led)
{
    led->gpiox->ODR ^= led->gpio_pin;
}





void g_led_single_flash(gpio_t led,u32 freq)
{
    if(get_tick()%freq == 0)
    {
        s_gpio_turn(&led);
    }
}

void g_set_led(gpio_t led,u32 freq)
{   
    GPIO_SetBits(led.gpiox, led.gpio_pin); 
}

void g_reset_led(gpio_t led,u32 freq)
{
    GPIO_ResetBits(led.gpiox, led.gpio_pin); 
}







