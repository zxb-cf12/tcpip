/****************************************
**file name :
**function  :
****************************************/

#include "app_config.h"

#include "ethernet_phyconfig.h"



/***************************
  @name: g_app_hd_init()
  @func: config a gpio as led
  @return:
***************************/
void g_app_hd_init()
{
    
    g_sys_led_config();
    g_all_usart_config();
    
    LwIP_EthernetInit();
    
    
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
}














