#ifndef __ETHERNET_PHY_CFG__
#define __ETHERNET_PHY_CFG__
#include "stm32f10x.h"                  // Device header
#include "ethernetif.h"

#define LWIP_MAX_DHCP_TRIES		4   // DHCP������������Դ���
#define ETHARP_HWADDR_LEN		6	// ��̫��MAC��ַ����

#define PHY_ADDRESS 			1   // PHY��ַ


void LwIP_EthernetInit();

#endif
