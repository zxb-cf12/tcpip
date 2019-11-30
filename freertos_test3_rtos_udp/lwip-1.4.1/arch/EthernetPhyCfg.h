#ifndef __ETHERNET_PHY_CFG__
#define __ETHERNET_PHY_CFG__
#include "stm32f10x.h"                  // Device header
#include "ethernetif.h"

#define LWIP_MAX_DHCP_TRIES		4   // DHCP服务器最大重试次数
#define ETHARP_HWADDR_LEN		6	// 以太网MAC地址长度

#define PHY_ADDRESS 			1   // PHY地址


void LwIP_EthernetInit();

#endif
