/****************************************
**file name :
**function  :
****************************************/
#include "FreeRTOS.h"
#include "led_flash.h"
#include "app_config.h"
#include "FreeRTOSConfig.h"
#include "task.h"
#include "stm32f10x_tim.h"
#include "UsartHandle.h"

#include "ethernetif.h"
#include "lwip/udp.h"
#include "lwip/tcp.h"
#include "string.h"
#include "lwip/api.h"



void Timer4Init(void)
{
	TIM_TimeBaseInitTypeDef TimeBaseStruct;
	RCC_ClocksTypeDef RCC_Clocks;
	RCC_GetClocksFreq(&RCC_Clocks);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM4,ENABLE);
	TimeBaseStruct.TIM_ClockDivision = TIM_CKD_DIV1;
	TimeBaseStruct.TIM_CounterMode   = TIM_CounterMode_Up;
	TimeBaseStruct.TIM_Period		 = 100-1;
	TimeBaseStruct.TIM_Prescaler	 = RCC_Clocks.HCLK_Frequency/1000000-1;
	TimeBaseStruct.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM4,&TimeBaseStruct);
	TIM_ClearFlag(TIM4,TIM_FLAG_Update);
	TIM_ITConfig(TIM4,TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM4,ENABLE);
	NVIC_SetPriority(TIM4_IRQn,configMAX_PRIORITIES+2);
	NVIC_EnableIRQ(TIM4_IRQn);
}
char str1[] = "task 1 running";
char str2[] = "task 2 running";


uint8_t flag = 0;

void task_led_on()
{
    vTaskDelay(1000/portTICK_RATE_MS);
    while(1)
    {
//        if(flag == 1)
//        {
//         ethernetif_input(&netif_net);
//        }
        os_printf("%s\r\n", str2);
        g_set_led(sys_led,0);   
        vTaskDelay(2000/portTICK_RATE_MS);
    }
}









void task_led_off()
{
    
    while(1)
    {
        os_printf("%s\r\n", str1);
        g_reset_led(sys_led,200);
        vTaskDelay(2000/portTICK_RATE_MS);
    }
}



#define UDP_DEMO_PORT			8089	//定义udp连接的端口 

void udp_demo_recv(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port);
const u8 *tcp_demo_sendbuf="udp_test\r\n";


u8 flag_1 =0;
void task_udp_test()
{
    
    err_t err;
	struct udp_pcb *udppcb;  	//定义一个TCP服务器控制块
	struct ip_addr rmtipaddr;  	//远端ip地址
 	
	u8 *tbuf;
 	u8 key;
	u8 res=0;		
	u8 t=0; 
    u8 i = 0;
    u8 test[128] = {0};
    struct pbuf *ptr;
	//u8 ptr[] = "udp test"; //申请内存
    ptr = (void*)test;
    
    
    udppcb=udp_new();
    if(udppcb)//创建成功
    { 

        IP4_ADDR(&rmtipaddr, 192, 168, 1, 11);   //远端ip
        err=udp_connect(udppcb,&rmtipaddr,UDP_DEMO_PORT);//UDP客户端连接到指定IP地址和端口号的服务器
        if(err==ERR_OK)
        {
            err=udp_bind(udppcb,IP_ADDR_ANY,UDP_DEMO_PORT);//绑定本地IP地址与端口号
            if(err==ERR_OK)	//绑定完成
            {
                udp_recv(udppcb,udp_demo_recv,NULL);//注册接收回调函数 

            }
        }		
    }
    
    while(1)
    {
        if(flag_1 == 1) //接收到数据
        {
            flag_1 = 0;
              
            ptr=pbuf_alloc(PBUF_TRANSPORT,strlen((char*)tcp_demo_sendbuf),PBUF_POOL); //申请内存
            pbuf_take(ptr,(char*)tcp_demo_sendbuf, strlen((char*)tcp_demo_sendbuf)); //将tcp_demo_sendbuf中的数据打包进pbuf结构中
            udp_send(udppcb,ptr);	//udp发送数据 
            pbuf_free(ptr);//释放内存
            //udp_disconnect(udppcb); 
            //udp_remove(udppcb);			//断开UDP连接 
        }        
        vTaskDelay(2000/portTICK_RATE_MS);
    }
 
}
#define UDP_DEMO_RX_BUFSIZE		2000	//定义udp最大接收数据长度 
u8 udp_demo_recvbuf[UDP_DEMO_RX_BUFSIZE];	//UDP接收数据缓冲区 



#define TCP_CLIENT_RX_BUFSIZE		2000	//定义udp最大接收数据长度 
u8 tcp_demo_recvbuf[TCP_CLIENT_RX_BUFSIZE];	//UDP接收数据缓冲区 

void udp_demo_recv(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port)
{

	u32 data_len = 0;
	struct pbuf *q;
	if(p!=NULL)	//接收到不为空的数据时
	{
		memset(udp_demo_recvbuf,0,UDP_DEMO_RX_BUFSIZE);  //数据接收缓冲区清零
		for(q=p;q!=NULL;q=q->next)  //遍历完整个pbuf链表
		{
			//判断要拷贝到UDP_DEMO_RX_BUFSIZE中的数据是否大于UDP_DEMO_RX_BUFSIZE的剩余空间，如果大于
			//的话就只拷贝UDP_DEMO_RX_BUFSIZE中剩余长度的数据，否则的话就拷贝所有的数据
			if(q->len > (UDP_DEMO_RX_BUFSIZE-data_len)) memcpy(udp_demo_recvbuf+data_len,q->payload,(UDP_DEMO_RX_BUFSIZE-data_len));//拷贝数据
			else memcpy(udp_demo_recvbuf+data_len,q->payload,q->len);
			data_len += q->len;  	
			if(data_len > UDP_DEMO_RX_BUFSIZE) break; //超出TCP客户端接收数组,跳出	
		}
		pbuf_free(p);//释放内存
        flag_1 = 1;           
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct tcp_client_struct
{
	u8 state;               //当前连接状
	struct tcp_pcb *pcb;    //指向当前的pcb
	struct pbuf *p;         //指向接收/或传输的pbuf
};  

//tcp服务器连接状态
enum tcp_client_states
{
	ES_TCPCLIENT_NONE = 0,		//没有连接
	ES_TCPCLIENT_CONNECTED,		//连接到服务器了 
	ES_TCPCLIENT_CLOSING,		//关闭连接
}; 

typedef enum
{
	HTTP_TYPE_INVALID = 0,
	HTTP_TYPE_POST,
	HTTP_TYPE_GET
}HttpType_Typedef;

typedef struct
{
	char *hostName;
	uint8_t hostNameLen;
	struct ip_addr ip;	// ??IP,ip.addr = 0???????
	uint16_t port;		// ??
	HttpType_Typedef type;
	char *pathAddr;		// path??url???
	uint16_t pathLen;	// path??
}urlAnalyzeStruct;




//lwIP tcp_recv()函数的回调函数
err_t tcp_client_recv(void *arg,struct tcp_pcb *tpcb,struct pbuf *p,err_t err)
{ 
	u32 data_len = 0;
	struct pbuf *q;
	struct tcp_client_struct *es;
	err_t ret_err; 
	LWIP_ASSERT("arg != NULL",arg != NULL);
	es=(struct tcp_client_struct *)arg; 
    
    if(es->state==ES_TCPCLIENT_CONNECTED)	//当处于连接状态时
	{
		if(p!=NULL)//当处于连接状态并且接收到的数据不为空时
		{
			memset(tcp_demo_recvbuf,0,TCP_CLIENT_RX_BUFSIZE);  //数据接收缓冲区清零
			for(q=p;q!=NULL;q=q->next)  //遍历完整个pbuf链表
			{
				//判断要拷贝到TCP_CLIENT_RX_BUFSIZE中的数据是否大于TCP_CLIENT_RX_BUFSIZE的剩余空间，如果大于
				//的话就只拷贝TCP_CLIENT_RX_BUFSIZE中剩余长度的数据，否则的话就拷贝所有的数据
				if(q->len > (TCP_CLIENT_RX_BUFSIZE-data_len)) memcpy(tcp_demo_recvbuf+data_len,q->payload,(TCP_CLIENT_RX_BUFSIZE-data_len));//拷贝数据
				else memcpy(tcp_demo_recvbuf+data_len,q->payload,q->len);
				data_len += q->len;  	
				if(data_len > TCP_CLIENT_RX_BUFSIZE) break; //超出TCP客户端接收数组,跳出	
			}
			flag_1  =1;

 			tcp_recved(tpcb,p->tot_len);//用于获取接收数据,通知LWIP可以获取更多数据
			pbuf_free(p);  	//释放内存
			ret_err=ERR_OK;
		}
	}
	return ret_err;
}

void tcp_client_error(void *arg,err_t err)
{  
	//这里我们不做任何处理
} 

//此函数用来发送数据
void tcp_client_senddata(struct tcp_pcb *tpcb, struct tcp_client_struct * es)
{
	struct pbuf *ptr; 
 	err_t wr_err=ERR_OK;
	while((wr_err==ERR_OK)&&es->p&&(es->p->len<=tcp_sndbuf(tpcb)))
	{
		ptr=es->p;
		wr_err=tcp_write(tpcb,ptr->payload,ptr->len,1); //将要发送的数据加入到发送缓冲队列中
		if(wr_err==ERR_OK)
		{  
			es->p=ptr->next;			//指向下一个pbuf
			if(es->p)pbuf_ref(es->p);	//pbuf的ref加一
			pbuf_free(ptr);				//释放ptr 
		}else if(wr_err==ERR_MEM)es->p=ptr;
		tcp_output(tpcb);		//将发送缓冲队列中的数据立即发送出去
	} 	
} 

//lwIP tcp_sent的回调函数(当从远端主机接收到ACK信号后发送数据)
err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
	struct tcp_client_struct *es;
	LWIP_UNUSED_ARG(len);
	es=(struct tcp_client_struct*)arg;
	if(es->p)tcp_client_senddata(tpcb,es);//发送数据
	return ERR_OK;
}




//lwIP tcp_poll的回调函数
err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb)
{
	err_t ret_err;
	struct tcp_client_struct *es; 
	es=(struct tcp_client_struct*)arg;
	if(es!=NULL)  //连接处于空闲可以发送数据
	{
		if(flag_1 == 1)	//判断是否有数据要发送 
		{
            flag_1 = 0;
            
			es->p=pbuf_alloc(PBUF_TRANSPORT, strlen((char*)tcp_demo_sendbuf),PBUF_POOL);	//申请内存 
            
			pbuf_take(es->p,(char*)tcp_demo_sendbuf,strlen((char*)tcp_demo_sendbuf));	//将tcp_client_sentbuf[]中的数据拷贝到es->p_tx中
			tcp_client_senddata(tpcb,es);//将tcp_client_sentbuf[]里面复制给pbuf的数据发送出去
			
			if(es->p)pbuf_free(es->p);	//释放内存
		}
        else if(es->state==ES_TCPCLIENT_CLOSING)
		{ 
 			//tcp_client_connection_close(tpcb,es);//关闭TCP连接
		} 
		ret_err=ERR_OK;
	}
    else
	{ 
		tcp_abort(tpcb);//终止连接,删除pcb控制块
		ret_err=ERR_ABRT;
	}
	return ret_err;
}
//lwIP TCP连接建立后调用回调函数
err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	struct tcp_client_struct *es=NULL;  
	if(err==ERR_OK)   
	{
		es=(struct tcp_client_struct*)mem_malloc(sizeof(struct tcp_client_struct));  //申请内存
		if(es) //内存申请成功
		{
 			es->state=ES_TCPCLIENT_CONNECTED;//状态为连接成功
			es->pcb=tpcb;  
			es->p=NULL; 
			tcp_arg(tpcb,es);        			//使用es更新tpcb的callback_arg
			tcp_recv(tpcb,tcp_client_recv);  	//初始化LwIP的tcp_recv回调功能   
			tcp_err(tpcb,tcp_client_error); 	//初始化tcp_err()回调函数
			tcp_sent(tpcb,tcp_client_sent);		//初始化LwIP的tcp_sent回调功能
			tcp_poll(tpcb,tcp_client_poll,1); 	//初始化LwIP的tcp_poll回调功能 
 			err=ERR_OK;
		}
	}
	return err;
}



void task_tcp_test()
{
	struct ip_addr rmtipaddr;  	//远端ip地址
    struct tcp_pcb *tcppcb;  	//定义一个TCP服务器控制

	tcppcb=tcp_new();	//创建一个新的pcb
	if(tcppcb)			//创建成功
	{
		IP4_ADDR(&rmtipaddr, 192, 168, 1, 11);   //远端ip
        //while()
        {
            tcp_connect(tcppcb,&rmtipaddr,UDP_DEMO_PORT,tcp_client_connected);  //连接到目的地址的指定端口上,当连接成功后回调tcp_client_connected()函数
            vTaskDelay(1000/portTICK_RATE_MS);
        }
 	}
    
    while(1)
    {
        if(flag_1 == 1) //接收到数据
        {
            flag_1 = 0;
            
        }        
        vTaskDelay(2000/portTICK_RATE_MS);
    }
}
#define HTTP_HEAD_SUCCESS	"HTTP/1.1 200 OK"
#define HTTP_HEAD_DATALEN	"Content-Length: "

uint32_t HttpResponeHeadAnalyze(char *httpRspData)
{
	uint32_t len = 0;
	char *tp;
	// ????????HTTP/1.1 200 OK
	if(strncasecmp(httpRspData,HTTP_HEAD_SUCCESS,sizeof(HTTP_HEAD_SUCCESS)-1)!=0)
	{
		return 0;
	}
	// ??????
	tp = strstr(httpRspData,HTTP_HEAD_DATALEN);
	if(tp == NULL)
	{
		return 0;
	}
	len = atoi(tp+sizeof(HTTP_HEAD_DATALEN)-1);
	return len;
}

urlAnalyzeStruct urlAnalyze(char *url,uint16_t urlLen)
{
	char *tempUrl;
	urlAnalyzeStruct urlAS;
	memset(&urlAS,0,sizeof(urlAnalyzeStruct));
	urlAS.ip.addr = 0;
	urlAS.port = 80;
	if(url == NULL)
	{
		return urlAS;
	}
	tempUrl = url;
	
	//******??HTTP???? POST/GET
	if(strncasecmp(url,"GET ",4) == 0)
	{
		urlAS.type = HTTP_TYPE_GET;
		tempUrl += 4;
	}
	else if(strncasecmp(url,"POST ",5) == 0)	// ??POST??
	{
		urlAS.type = HTTP_TYPE_POST;
		tempUrl += 5;
	}
	else	// ??POST/GET????
	{
		return urlAS;
	}
	if(*tempUrl)
	{	// ??http://???
		char *tp = strstr(tempUrl,"://");
		if(NULL != tp)
		{
			tempUrl = tp+3;
		}
	}
	//******??????
	if(*tempUrl)
	{
		char *hostName = pvPortMalloc(url+urlLen - tempUrl);
		if(hostName)
		{
			sscanf(tempUrl,"%[^:/\r\n]:%hd",hostName,&urlAS.port);	//??hostName???
			urlAS.pathAddr = strstr(tempUrl,"/");	// ?????'/'??path??			
			if(urlAS.pathAddr)						// ????path
			{
				urlAS.pathAddr += 1;				// ?????????'/'
				urlAS.pathLen = url+urlLen - urlAS.pathAddr;
			}
			
			urlAS.hostName = hostName;
			urlAS.hostNameLen = strlen(hostName);
			os_printf("hostName=%s, hostNameLen=%d\r\n",hostName,urlAS.hostNameLen);
			if(ERR_OK!= netconn_gethostbyname(hostName,&urlAS.ip))
			{
				urlAS.ip.addr = 0;
			}
		}
	}
	return urlAS;
}
#define HTTP_RESQUEST_DATA	\
	"%s /%.*s HTTP/1.1\r\n"\
	"Host: %s\r\n"\
	"\r\n"
unsigned char urlstr[] = "http://122.112.251.34:8890/LBMCMeter.apk";

#define OTASTORAGE_ADDR				((uint32_t)0x19000)	

err_t HttpSend(char *url,char *parmData,uint32_t *writeSuccLen)
{
	err_t err = ERR_MEM;
	urlAnalyzeStruct urlAS;	
	struct netconn *tcpconn;
	struct netbuf *netBuf;
	uint32_t datLen = 0;
	
	(*writeSuccLen) = 0;
	tcpconn = netconn_new(NETCONN_TCP);
	if(tcpconn == NULL)
	{
		return ERR_VAL;
	}
	
	/*??url*/
	urlAS= urlAnalyze(url,strlen(url));
	if(urlAS.ip.addr == 0)	// ??url??
	{
		netconn_delete(tcpconn);
		return ERR_RTE;
	}
	
	netconn_set_recvtimeout(tcpconn,5000);	// ????????
	if(ERR_OK != netconn_connect(tcpconn,&urlAS.ip,urlAS.port))
	{
		netconn_delete(tcpconn);
		return err;
	}
	else
	{
		char *httpData = pvPortMalloc(urlAS.pathLen + 30 + sizeof(HTTP_RESQUEST_DATA) + strlen(parmData));
		if(httpData == NULL)
		{
			netconn_delete(tcpconn);
			return err;
		}						
		else
		{
			datLen = sprintf(httpData,HTTP_RESQUEST_DATA,
								(urlAS.type==HTTP_TYPE_GET?"GET":"POST"),
								urlAS.pathLen,urlAS.pathAddr,urlAS.hostName);
			if(parmData)
			{
				datLen += sprintf(httpData+datLen,"%s",parmData);
			}
			os_printf("%s",httpData);
			netconn_write(tcpconn,httpData,datLen, NETCONN_COPY);						
			vPortFree(httpData);
		}
	}
	datLen = 0;	// ?????,datLen????????????,(*writeSuccLen)????????????
	while( datLen==0 ||(*writeSuccLen) < datLen)
	{
		uint16_t tempBufLen;
		char *tempBuf;
		if(ERR_OK != netconn_recv(tcpconn, &netBuf))
		{
			err = ERR_TIMEOUT;
			break;
		}
		tempBufLen = netBuf->p->tot_len;
		tempBuf = pvPortMalloc(tempBufLen);
		if(tempBuf == NULL)
		{
			err = ERR_MEM;
			break;
		}
		netbuf_copy(netBuf,tempBuf,tempBufLen);
		netbuf_delete(netBuf);
		
		if(datLen == 0)	// ????
		{
			char *tp = strstr(tempBuf,"\r\n\r\n");
			datLen = HttpResponeHeadAnalyze(tempBuf);
			if(tp == NULL ||datLen == 0)
			{
				vPortFree(tempBuf);
				err = ERR_VAL;
				break;
			}
			tp+=sizeof("\r\n\r\n")-1;
			tempBufLen = tempBuf + tempBufLen - tp;
			(*writeSuccLen) = (tempBufLen < datLen)?tempBufLen:datLen;
			os_printf("*writeSuccLen = %d/",*writeSuccLen);
			// ?????????Flash?
			//STM32_WriteDisk(OTASTORAGE_ADDR,tp,(*writeSuccLen));
			vPortFree(tempBuf);
		}
		else
		{
			tempBufLen = (tempBufLen+(*writeSuccLen) < datLen)?tempBufLen:(datLen - (*writeSuccLen));
			//STM32_WriteDisk(OTASTORAGE_ADDR+(*writeSuccLen),tempBuf,tempBufLen);
			(*writeSuccLen) += tempBufLen;
			os_printf("(*writeSuccLen)=%d/",*writeSuccLen);
			vPortFree(tempBuf);
			if((*writeSuccLen) >= datLen)	// ????
			{
				os_printf("\r\nReceive finished: (*writeSuccLen)=%d\r\n",(*writeSuccLen));
				err = ERR_OK;
				break;
			}
		}
	}
	netconn_delete(tcpconn);
	return err;
}

//    struct ip_addr rmtipaddr;  	//远端ip地址
//    struct tcp_pcb *tcppcb;  	//定义一个TCP服务器控制
//    struct netconn *tcp_clientconn;	
//	static ip_addr_t server_ipaddr,loca_ipaddr;
//    static u16_t 		 server_port,loca_port;
//    server_port = 8087;
//    tcp_clientconn = netconn_new(NETCONN_TCP);  //创建一个TCP链接
//	if(tcppcb)			//创建成功
//	{
//		IP4_ADDR(&rmtipaddr, 192, 168, 1, 112);   //远端ip
//        {
//            //tcp_connect(tcppcb,&rmtipaddr,UDP_DEMO_PORT,tcp_client_connected);  //连接到目的地址的指定端口上,当连接成功后回调tcp_client_connected()函数
//            netconn_connect(tcp_clientconn,&rmtipaddr,server_port);//连接服务器
//            vTaskDelay(1000/portTICK_RATE_MS);
//        }
//        while(1)
//        {
//            if(netconn_recv(tcp_clientconn,&recvbuf) == ERR_OK)
//            {         
//                netconn_write(tcp_clientconn ,tcp_demo_sendbuf,strlen((char*)tcp_demo_sendbuf),NETCONN_COPY); //发送tcp_server_sentbuf中的数据
//            
//            } 
//        }
// 	}

void ota_test()
{

	char *url = pvPortMalloc(41 + 4);
	uint32_t writeSuccLen = 0;
	sprintf(url,"GET %s",urlstr);
    
    HttpSend(url,NULL,&writeSuccLen);



}



int main()
{
    g_app_hd_init();
    
    xTaskCreate(task_led_on,"vTask_XlinkLoop_Thread",500,NULL,2,NULL);
    xTaskCreate(task_led_off,"vTask_XlinThread",500,NULL,1,NULL);
    //xTaskCreate(task_tcp_test,"vTask_XlinThread",1000,NULL,3,NULL);
    xTaskCreate(ota_test,"vTask_XlinThread",1000,NULL,4,NULL);

    vTaskStartScheduler();
    while(1)
    {
        
    }
}
