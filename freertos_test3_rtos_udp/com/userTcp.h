#ifndef __USER_TCP_H__
#define __USER_TCP_H__

#include "stm32f10x.h"                  // Device header

typedef struct
{
	TaskHandle_t taskHandle;	// 任务句柄
	struct netconn *tcpconn;
	uint16_t localPort;			// 本地端口
	void (*recvCb)(struct netconn *tcpconn,void *datBuf,uint32_t len);
	int sendTimeout;
	int recvTimeout;	// 超时时长，单位ms,=0时表示一直阻塞等待有效数据
}TcpThreadParm_t;
void vTask_TcpAsClient_Thread(void *parm);
void TcpAsSorC_Send(struct netconn *tcpconn,void *datBuf,uint32_t len);
#endif
