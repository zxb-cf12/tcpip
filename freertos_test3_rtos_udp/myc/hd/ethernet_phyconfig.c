#include "ethernet_phyconfig.h"
#include "stm32f4x7_eth.h"
#include "lwip/tcpip.h"
#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "ethernetif.h"
typedef enum
{
	DHCP_STA_IDEL	= 0,
	DHCP_STA_START,
	DHCP_STA_WAIT_ADDRESS,
	DHCP_STA_ADDRESS_ASSIGNED,
	DHCP_STA_TIMEOUT,
	DHCP_STA_LINK_DOWN,
}DhcpStatus_typedef;

struct netif netif_net;	
/**************************************************************************
** 功能	@brief : 设置MCO频率
** 输入	@param :        
** 输出	@retval:        
***************************************************************************/
static void MCO_FreqSet(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	/* Set PLL3 clock output to 50MHz (25MHz /5 *10 =50MHz) */
	RCC_PLL3Config(RCC_PLL3Mul_10);
	/* Enable PLL3 */
	RCC_PLL3Cmd(ENABLE);
	/* Wait till PLL3 is ready */
	while (RCC_GetFlagStatus(RCC_FLAG_PLL3RDY) == RESET)
	{}
	/* Get PLL3 clock on PA8 pin (MCO) */
	RCC_MCOConfig(RCC_MCO_PLL3CLK);
}
/**************************************************************************
** 功能  @brief : 配置PHY中断模式
** 输入  @param :         
** 输出  @retval:       
***************************************************************************/
static uint32_t Eth_Link_PHYITConfig(uint16_t PHYAddress)
{
  uint16_t tmpreg = 0;
  tmpreg = ETH_ReadPHYRegister(PHYAddress, PHY_MICR);

  /* Enable output interrupt events to signal via the INT pin */
  tmpreg |= (uint16_t)(PHY_MICR_INT_EN);
  if(!(ETH_WritePHYRegister(PHYAddress, PHY_MICR, tmpreg)))
  {
    /* Return ERROR in case of write timeout */
    return ETH_ERROR;
  }

  /* Read MISR register */
  tmpreg = ETH_ReadPHYRegister(PHYAddress, PHY_MISR);

  /* Enable Interrupt on change of link status */
  tmpreg |= (uint16_t)PHY_MISR_LINK_INT_EN;
  if(!(ETH_WritePHYRegister(PHYAddress, PHY_MISR, tmpreg)))
  {
    /* Return ERROR in case of write timeout */
    return ETH_ERROR;
  }
  /* Return SUCCESS */
  return ETH_SUCCESS;   
}
/**************************************************************************
** 功能	@brief : 初始化STM的以太网GPIO外设
** 输入	@param :        
** 输出	@retval:        
***************************************************************************/
static void Periph_EthGpioInit(void)
{
	GPIO_InitTypeDef GPIO_InitStruct;
	/*ETH_MDIO -------------------------> PA2
	  ETH_MDC --------------------------> PC1
	  ETH_TX_EN ------------------------> PB11
	  ETH_TXD0 -------------------------> PB12
	  ETH_TXD1 -------------------------> PB13	  
	  ETH_REF_CLK-----------------------> PA1
	  ETH_CRS_DV------------------------> PA7
	  ETH_RXD0--------------------------> PC4
	  ETH_RXD1--------------------------> PC5
	  ETH_MCO---------------------------> PA8
	  ETH_INT---------------------------> PC6
	*/
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC |RCC_APB2Periph_AFIO, ENABLE);

	GPIO_ETH_MediaInterfaceConfig(GPIO_ETH_MediaInterface_RMII);
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_7;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOB, &GPIO_InitStruct);
	
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_8;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_Init(GPIOA, &GPIO_InitStruct);
}
/**************************************************************************
** 功能	@brief : 初始化STM的以太网MAC外设
** 输入	@param :        
** 输出	@retval:        
***************************************************************************/
static void Periph_EthMacDmaInit(uint16_t PHYAddress)
{
	ETH_InitTypeDef ETH_InitStruct;
	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_ETH_MAC | RCC_AHBPeriph_ETH_MAC_Tx |
						RCC_AHBPeriph_ETH_MAC_Rx, ENABLE);
	ETH_DeInit();
	ETH_SoftwareReset();
	while (ETH_GetSoftwareResetStatus() == SET);
	ETH_StructInit(&ETH_InitStruct);
	/*------------------------   MAC   -----------------------------------*/
	ETH_InitStruct.ETH_AutoNegotiation = ETH_AutoNegotiation_Disable;
	ETH_InitStruct.ETH_LoopbackMode = ETH_LoopbackMode_Disable;
	ETH_InitStruct.ETH_RetryTransmission = ETH_RetryTransmission_Disable;
	ETH_InitStruct.ETH_AutomaticPadCRCStrip = ETH_AutomaticPadCRCStrip_Disable;
	ETH_InitStruct.ETH_ReceiveAll = ETH_ReceiveAll_Disable;
	ETH_InitStruct.ETH_BroadcastFramesReception = ETH_BroadcastFramesReception_Enable;
	ETH_InitStruct.ETH_PromiscuousMode = ETH_PromiscuousMode_Disable;
	ETH_InitStruct.ETH_MulticastFramesFilter = ETH_MulticastFramesFilter_Perfect;
	ETH_InitStruct.ETH_UnicastFramesFilter = ETH_UnicastFramesFilter_Perfect;
#ifdef CHECKSUM_BY_HARDWARE
	ETH_InitStruct.ETH_ChecksumOffload = ETH_ChecksumOffload_Enable;
#endif
	  /*------------------------   DMA   -----------------------------------*/    
	ETH_InitStruct.ETH_DropTCPIPChecksumErrorFrame = ETH_DropTCPIPChecksumErrorFrame_Enable; 
	ETH_InitStruct.ETH_ReceiveStoreForward = ETH_ReceiveStoreForward_Enable;         
	ETH_InitStruct.ETH_TransmitStoreForward = ETH_TransmitStoreForward_Enable;     
	ETH_InitStruct.ETH_ForwardErrorFrames = ETH_ForwardErrorFrames_Disable;       
	ETH_InitStruct.ETH_ForwardUndersizedGoodFrames = ETH_ForwardUndersizedGoodFrames_Disable;   
	ETH_InitStruct.ETH_SecondFrameOperate = ETH_SecondFrameOperate_Enable;                                                          
	ETH_InitStruct.ETH_AddressAlignedBeats = ETH_AddressAlignedBeats_Enable;      
	ETH_InitStruct.ETH_FixedBurst = ETH_FixedBurst_Enable;                
	ETH_InitStruct.ETH_RxDMABurstLength = ETH_RxDMABurstLength_32Beat;          
	ETH_InitStruct.ETH_TxDMABurstLength = ETH_TxDMABurstLength_32Beat;                                                                 
	ETH_InitStruct.ETH_DMAArbitration = ETH_DMAArbitration_RoundRobin_RxTx_2_1;

	ETH_Init(&ETH_InitStruct, PHYAddress);
	ETH_DMAITConfig(ETH_DMA_IT_NIS | ETH_DMA_IT_R, ENABLE);
	NVIC_SetPriority(ETH_IRQn,5);
	NVIC_EnableIRQ(ETH_IRQn);
}
extern uint8_t flag;

void ETH_IRQHandler(void) //中断处理函数，给与信号量
{
	portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
    
	if(ETH_GetDMAITStatus(ETH_DMA_IT_R)!= RESET)
	{
		xSemaphoreGiveFromISR(xSemaphore_input, &xHigherPriorityTaskWoken );
 
	}
	ETH_DMAClearITPendingBit(ETH_DMA_IT_R);
	ETH_DMAClearITPendingBit(ETH_DMA_IT_NIS);
    if( xHigherPriorityTaskWoken != pdFALSE )
	{
		portEND_SWITCHING_ISR( xHigherPriorityTaskWoken );
	}

}

/**************************************************************************
** 功能	@brief : 初始化STM的以太网外设
** 输入	@param :        
** 输出	@retval:        
***************************************************************************/
static void Periph_EthernetInit(void)
{
	Periph_EthGpioInit();
	MCO_FreqSet();
	Periph_EthMacDmaInit(1);
	Eth_Link_PHYITConfig(1);
}
/**************************************************************************
** 功能  @brief : link回调函数，在link改变时调用
** 输入  @param :         
** 输出  @retval:       
***************************************************************************/
static void ETH_link_callback(struct netif *netif)
{
	__IO uint32_t timeout = 0;
	uint16_t RegValue;
	struct ip_addr ipaddr;
	struct ip_addr netmask;
	struct ip_addr gw;
	
	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr = 0;
	netif_set_addr(netif, &ipaddr , &netmask, &gw);
	
	if(netif_is_link_up(netif))
	{
		//os_printf("the netif is link up\n");
		ETH_WritePHYRegister(1, PHY_BCR, PHY_AutoNegotiation);
		timeout = 0;
		do
		{
			timeout++;
		} while (!(ETH_ReadPHYRegister(1, PHY_BSR) & PHY_AutoNego_Complete) && (timeout < (uint32_t)PHY_READ_TO));  
		timeout = 0;
		RegValue = ETH_ReadPHYRegister(1, PHY_SR);
		if(RegValue & PHY_DUPLEX_STATUS)
		{
			ETH->MACCR|=ETH_Mode_FullDuplex;
		}
		else
		{
			ETH->MACCR&=~ETH_Mode_FullDuplex;          
		}
		if(RegValue & PHY_SPEED_STATUS)
		{
			ETH->MACCR&=~ETH_Speed_100M;
		}
		else
		{
			ETH->MACCR|=ETH_Speed_100M;		  
		}
		ETH_Start();
		netif_set_up(netif);
	}
	else
	{
		//os_printf("the netif is link down\n");
		netif_set_down(netif);
	}
}
extern uint8_t MACaddr[6];
/**************************************************************************
** 功能  @brief : 初始化LWIP
** 输入  @param :         
** 输出  @retval:       
***************************************************************************/
void LwIP_EthernetInit()
{
    struct ip_addr ipaddr;
    struct ip_addr netmask;
    struct ip_addr gw;

    Periph_EthernetInit();
    tcpip_init( NULL, NULL );
//	mem_init();  /* Initializes the dynamic memory heap defined by MEM_SIZE.*/ 
//    memp_init(); /* Initializes the memory pools defined by MEMP_NUM_x.*/
//    
    //uint8_t macaddress[6]={0,0,0,0,0,1};
    uint8_t macaddress[6]={0xcc,0xbb,0xaa,0x99,0x88,0x2};

    IP4_ADDR(&ipaddr, 192, 168, 1, 7);
    IP4_ADDR(&netmask, 255, 255, 255, 0);
    IP4_ADDR(&gw, 192, 168, 1, 1);
    MACaddr[0] = macaddress[0];
    MACaddr[1] = macaddress[1];
    MACaddr[2] = macaddress[2];
    MACaddr[3] = macaddress[3];
    MACaddr[4] = macaddress[4];
    MACaddr[5] = macaddress[5];
    ETH_MACAddressConfig(ETH_MAC_Address0, macaddress);   //set mac
    
  /* - netif_add(struct netif *netif, struct ip_addr *ipaddr,
            struct ip_addr *netmask, struct ip_addr *gw,
            void *state, err_t (* init)(struct netif *netif),
            err_t (* input)(struct pbuf *p, struct netif *netif))
    
   Adds your network interface to the netif_list. Allocate a struct
  netif and pass a pointer to this structure as the first argument.
  Give pointers to cleared ip_addr structures when using DHCP,
  or fill them with sane numbers otherwise. The state pointer may be NULL.

  The init function pointer must point to a initialization function for
  your ethernet netif interface. The following code illustrates it's use.*/
  //netif_add(&netif_net, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &ethernet_input);
  netif_add(&netif_net, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &tcpip_input);

  /*  Registers the default network interface.*/
  netif_set_default(&netif_net);



  /*  When the netif is fully configured this function must be called.*/
  netif_set_up(&netif_net);
}
/**************************************************************************
** 功能  @brief : 网线连接状态检测
** 输入  @param :         
** 输出  @retval:       
***************************************************************************/
//void vTask_Link_Thread(void *parm)
//{
//	DhcpStatus_typedef dhcpstatus = DHCP_STA_IDEL;
//	struct netif *netif = (struct netif *)parm;
//	/**启动时先检测一次网线连接状态***/
//	if((ETH_ReadPHYRegister(PHY_ADDRESS, PHY_SR) & 1))
//	{
//		netif_set_link_up(netif);
//		dhcpstatus = DHCP_STA_START;
//	}
//	else
//	{
//		netif_set_link_down(netif);
//		dhcpstatus = DHCP_STA_LINK_DOWN;
//	}
//	
//	while(1)
//	{
//		if(((ETH_ReadPHYRegister(PHY_ADDRESS, PHY_MISR)) & PHY_LINK_STATUS) != 0)
//		{
//			if((ETH_ReadPHYRegister(PHY_ADDRESS, PHY_SR) & 1))
//			{
//				netif_set_link_up(netif);
//				dhcpstatus = DHCP_STA_START;
//			}
//			else
//			{
//				netif_set_link_down(netif);
//				dhcpstatus = DHCP_STA_LINK_DOWN;
//			}
//		}
//		switch (dhcpstatus)
//		{
//			case DHCP_STA_START:
//			{
//				dhcpstatus = DHCP_STA_WAIT_ADDRESS;
//				dhcp_stop(netif);
//				dhcp_start(netif);			
//				os_printf("start to dhcp...\n");
//			}
//			break;
//			case DHCP_STA_WAIT_ADDRESS:
//			{
//				if (netif->ip_addr.addr!=0) 
//				{
//					dhcpstatus = DHCP_STA_ADDRESS_ASSIGNED;
//					//dhcp_stop(netif);
//					os_printf("ip_addr is %s\n",ipaddr_ntoa(&netif->ip_addr));
//					os_printf("netmask is %s\n",ipaddr_ntoa(&netif->netmask));
//					os_printf("gw is %s\n",ipaddr_ntoa(&netif->gw));
//				}
//				else if(netif->dhcp->tries > LWIP_MAX_DHCP_TRIES)
//				{
//					/* DHCP timeout */
//					dhcpstatus = DHCP_STA_TIMEOUT;
//					dhcp_stop(netif);
//					os_printf("dhcp is timeout\n");
//					dhcpstatus = DHCP_STA_START;	// 重新启动DHCP			
//				}
//			}
//			break;
//			case DHCP_STA_LINK_DOWN:
//			{
//				dhcp_stop(netif);
//				dhcpstatus = DHCP_STA_IDEL;
//			}
//			break;
//			default: break;
//		}
//		vTaskDelay(250/portTICK_RATE_MS);
//	}
//}
