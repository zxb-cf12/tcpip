#ifndef __etherent_phyconfig_H__
#define __etherent_phyconfig_H__


#include "lwip/err.h"
#include "lwip/netif.h"
#include "netif/etharp.h"

extern struct netif netif_net;	

//err_t ethernetif_init(void);
//struct netif *ethernetif_register(void);
//int ethernetif_poll(void);
void LwIP_EthernetInit(void);
#endif 
