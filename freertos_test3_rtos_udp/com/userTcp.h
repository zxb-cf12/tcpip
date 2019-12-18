#ifndef __USER_TCP_H__
#define __USER_TCP_H__

#include "stm32f10x.h"                  // Device header

typedef struct
{
	TaskHandle_t taskHandle;	// ������
	struct netconn *tcpconn;
	uint16_t localPort;			// ���ض˿�
	void (*recvCb)(struct netconn *tcpconn,void *datBuf,uint32_t len);
	int sendTimeout;
	int recvTimeout;	// ��ʱʱ������λms,=0ʱ��ʾһֱ�����ȴ���Ч����
}TcpThreadParm_t;
void vTask_TcpAsClient_Thread(void *parm);
void TcpAsSorC_Send(struct netconn *tcpconn,void *datBuf,uint32_t len);
#endif
