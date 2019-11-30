/*************************************************************************
**		(C) COPYRIGHT 2011~  吴健超(DGUT)
**		    源码未经检录,使用需谨慎,修改请注明原作者
** 文件名 @file	   : udp通信接口
** 创建人 @author  : 吴健超
** 版本   @version : V 1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   :
** 硬件   @hardware：
** 其他   @other   ：
***************************************************************************/
#include "userUdp.h"


///**************************************************************************
//** 功能	@brief : UDP客戶端通信任务,以及接收数据
//** 输入	@param : parm：具体查看UdpThreadParm_t结构体       
//** 输出	@retval:        
//***************************************************************************/
//void vTask_Udp_Thread(void *parm)
//{
//	UdpThreadParm_t *udpParm = (UdpThreadParm_t *)parm;
//	err_t err;
//	udpParm->udpconn = netconn_new(NETCONN_UDP);
//	if(NULL == udpParm->udpconn)
//	{
//		vTaskDelete(NULL);
//	}

//	if(udpParm->localPort !=0)	// 非零说明创建UDP服务器
//	{
//		err = netconn_bind(udpParm->udpconn, IP_ADDR_ANY, udpParm->localPort);
//	}
//	else
//	{
//		err = netconn_connect(udpParm->udpconn,&udpParm->serverIp,udpParm->serverPort);
//	}
//	if(ERR_OK != err)
//	{
//		netconn_delete(udpParm->udpconn);
//		vTaskDelete(NULL);
//	}
//	netconn_set_recvtimeout(udpParm->udpconn,udpParm->timeout);	// 设置接收超时时间
//	while(1)
//	{
//		struct netbuf *netBuf;
//		 err = netconn_recv(udpParm->udpconn,&netBuf);
//		if(err == ERR_OK)
//		{
//			if((udpParm->localPort != 0) && 
//				(netBuf->addr.addr==0 										||
//				ip4_addr1(&netBuf->addr) != ip4_addr1(&gbPcb.netif.ip_addr) ||
//				ip4_addr2(&netBuf->addr) != ip4_addr2(&gbPcb.netif.ip_addr) ||
//				ip4_addr3(&netBuf->addr) != ip4_addr3(&gbPcb.netif.ip_addr))		// 在udp作为服务端时判断数据是否来自同一个局域网
//			)
//			{
//				os_printf("data is not from lan,data ip [%s],local ip [%s]\n",ipaddr_ntoa(&netBuf->addr),ipaddr_ntoa(&gbPcb.netif.ip_addr));
//			}
//			else if(udpParm->localPort == 0 && 
//				(netBuf->addr.addr != udpParm->serverIp.addr || 
//				netBuf->port != udpParm->serverPort))	// 在udp作为客户端时判断数据是否来自服务器
//			{
//				os_printf("data is not from server,data ip [%s],server ip [%s]\n",ipaddr_ntoa(&netBuf->addr),ipaddr_ntoa(&udpParm->serverIp));
//			}				
//			else if(udpParm->recvCb)
//			{
//				DataBufType_Typedef dataBufType;
//				dataBufType.datLen = netBuf->p->tot_len;			
//				dataBufType.dat = pvPortMalloc(dataBufType.datLen);
//				if(dataBufType.dat )
//				{
//					netbuf_copy(netBuf,dataBufType.dat,dataBufType.datLen);
//					udpParm->recvCb(udpParm->udpconn,netbuf_fromaddr(netBuf),netbuf_fromport(netBuf),dataBufType.dat,dataBufType.datLen);
//					vPortFree(dataBufType.dat);
//				}
//			}
//			netbuf_delete(netBuf);
//		}	
//	}
//}
///**************************************************************************
//** 功能	@brief : udp发送函数
//** 输入	@param : udpconn：udp通信句柄，ip目标设备ip地址，datBuf：待发送的数据指针，len：待发送数据长度        
//** 输出	@retval:        
//***************************************************************************/
//void UdpAsSorC_Send(struct netconn *udpconn,struct ip_addr *ip,uint16_t port,void *datBuf,uint32_t len)
//{
//	struct netbuf *netBuf;
//	if(udpconn == NULL)
//		return;
//	netBuf = netbuf_new();
//	if(netBuf)
//	{
//		if(netbuf_alloc(netBuf,len))
//		{
//			netbuf_take(netBuf,datBuf,len);
//			netconn_sendto(udpconn,netBuf,ip,port);
//		}		
//		netbuf_delete(netBuf);
//	}
//}

