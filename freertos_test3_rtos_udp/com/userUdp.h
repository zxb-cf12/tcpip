#ifndef __USER_UDP_H__
#define __USER_UDP_H__

#include "stm32f10x.h"                  // Device header

typedef struct
{
	TaskHandle_t taskHandle;	// ������
	struct netconn *udpconn;	//
	uint16_t localPort;			// ���ض˿�
	uint16_t serverPort;		// Զ�̶˿�
	struct ip_addr serverIp;	// Զ��IP
	void (*recvCb)(struct netconn *udpconn,struct ip_addr *ip,uint16_t port,void *datBuf,uint32_t len);	// �ɹ����յ����ݻص�����ָ��
	int timeout;	// ��ʱʱ������λms,=0ʱ��ʾһֱ�����ȴ���Ч����
}UdpThreadParm_t;	// ���ض˿ڷ�0˵������UDP��������0����udp�ͻ��ˣ���ʱserverPort������Ч

void vTask_Udp_Thread(void *parm);	// ����udp����
void UdpAsSorC_Send(struct netconn *udpconn,struct ip_addr *ip,uint16_t port,void *datBuf,uint32_t len);	// udp���ͽӿ�

#endif
