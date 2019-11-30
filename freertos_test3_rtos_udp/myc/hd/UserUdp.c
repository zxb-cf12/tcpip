/*************************************************************************
**		(C) COPYRIGHT 2011~  �⽡��(DGUT)
**		    Դ��δ����¼,ʹ�������,�޸���ע��ԭ����
** �ļ��� @file	   : udpͨ�Žӿ�
** ������ @author  : �⽡��
** �汾   @version : V 1.0.0 ԭʼ�汾
** ����   @date    :
** ����   @brief   :
** Ӳ��   @hardware��
** ����   @other   ��
***************************************************************************/
#include "userUdp.h"


///**************************************************************************
//** ����	@brief : UDP�͑���ͨ������,�Լ���������
//** ����	@param : parm������鿴UdpThreadParm_t�ṹ��       
//** ���	@retval:        
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

//	if(udpParm->localPort !=0)	// ����˵������UDP������
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
//	netconn_set_recvtimeout(udpParm->udpconn,udpParm->timeout);	// ���ý��ճ�ʱʱ��
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
//				ip4_addr3(&netBuf->addr) != ip4_addr3(&gbPcb.netif.ip_addr))		// ��udp��Ϊ�����ʱ�ж������Ƿ�����ͬһ��������
//			)
//			{
//				os_printf("data is not from lan,data ip [%s],local ip [%s]\n",ipaddr_ntoa(&netBuf->addr),ipaddr_ntoa(&gbPcb.netif.ip_addr));
//			}
//			else if(udpParm->localPort == 0 && 
//				(netBuf->addr.addr != udpParm->serverIp.addr || 
//				netBuf->port != udpParm->serverPort))	// ��udp��Ϊ�ͻ���ʱ�ж������Ƿ����Է�����
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
//** ����	@brief : udp���ͺ���
//** ����	@param : udpconn��udpͨ�ž����ipĿ���豸ip��ַ��datBuf�������͵�����ָ�룬len�����������ݳ���        
//** ���	@retval:        
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

