#ifndef __USER_UDP_H__
#define __USER_UDP_H__

#include "stm32f10x.h"                  // Device header

typedef struct
{
	TaskHandle_t taskHandle;	// 任务句柄
	struct netconn *udpconn;	//
	uint16_t localPort;			// 本地端口
	uint16_t serverPort;		// 远程端口
	struct ip_addr serverIp;	// 远程IP
	void (*recvCb)(struct netconn *udpconn,struct ip_addr *ip,uint16_t port,void *datBuf,uint32_t len);	// 成功接收到数据回调函数指针
	int timeout;	// 超时时长，单位ms,=0时表示一直阻塞等待有效数据
}UdpThreadParm_t;	// 本地端口非0说明创建UDP服务器，0创建udp客户端，此时serverPort必须有效

void vTask_Udp_Thread(void *parm);	// 创建udp任务
void UdpAsSorC_Send(struct netconn *udpconn,struct ip_addr *ip,uint16_t port,void *datBuf,uint32_t len);	// udp发送接口

#endif
