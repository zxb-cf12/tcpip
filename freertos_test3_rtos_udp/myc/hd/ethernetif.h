#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include "lwip/err.h"
#include "lwip/netif.h"
#include "stm32f10x.h"
#include "FreeRTOS.h"


extern uint8_t MACaddr[6];
extern xSemaphoreHandle xSemaphore_input;
err_t ethernetif_init(struct netif *netif);
struct netif *ethernetif_register(void);
int ethernetif_poll(void);
#endif 
