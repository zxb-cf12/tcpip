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



#define UDP_DEMO_PORT			8089	//����udp���ӵĶ˿� 

void udp_demo_recv(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port);
const u8 *tcp_demo_sendbuf="udp_test\r\n";


u8 flag_1 =0;
void task_udp_test()
{
    
    err_t err;
	struct udp_pcb *udppcb;  	//����һ��TCP���������ƿ�
	struct ip_addr rmtipaddr;  	//Զ��ip��ַ
 	
	u8 *tbuf;
 	u8 key;
	u8 res=0;		
	u8 t=0; 
    u8 i = 0;
    u8 test[128] = {0};
    struct pbuf *ptr;
	//u8 ptr[] = "udp test"; //�����ڴ�
    ptr = (void*)test;
    
    
    udppcb=udp_new();
    if(udppcb)//�����ɹ�
    { 

        IP4_ADDR(&rmtipaddr, 192, 168, 1, 11);   //Զ��ip
        err=udp_connect(udppcb,&rmtipaddr,UDP_DEMO_PORT);//UDP�ͻ������ӵ�ָ��IP��ַ�Ͷ˿ںŵķ�����
        if(err==ERR_OK)
        {
            err=udp_bind(udppcb,IP_ADDR_ANY,UDP_DEMO_PORT);//�󶨱���IP��ַ��˿ں�
            if(err==ERR_OK)	//�����
            {
                udp_recv(udppcb,udp_demo_recv,NULL);//ע����ջص����� 

            }
        }		
    }
    
    while(1)
    {
        if(flag_1 == 1) //���յ�����
        {
            flag_1 = 0;
              
            ptr=pbuf_alloc(PBUF_TRANSPORT,strlen((char*)tcp_demo_sendbuf),PBUF_POOL); //�����ڴ�
            pbuf_take(ptr,(char*)tcp_demo_sendbuf, strlen((char*)tcp_demo_sendbuf)); //��tcp_demo_sendbuf�е����ݴ����pbuf�ṹ��
            udp_send(udppcb,ptr);	//udp�������� 
            pbuf_free(ptr);//�ͷ��ڴ�
            //udp_disconnect(udppcb); 
            //udp_remove(udppcb);			//�Ͽ�UDP���� 
        }        
        vTaskDelay(2000/portTICK_RATE_MS);
    }
 
}
#define UDP_DEMO_RX_BUFSIZE		2000	//����udp���������ݳ��� 
u8 udp_demo_recvbuf[UDP_DEMO_RX_BUFSIZE];	//UDP�������ݻ����� 



#define TCP_CLIENT_RX_BUFSIZE		2000	//����udp���������ݳ��� 
u8 tcp_demo_recvbuf[TCP_CLIENT_RX_BUFSIZE];	//UDP�������ݻ����� 

void udp_demo_recv(void *arg,struct udp_pcb *upcb,struct pbuf *p,struct ip_addr *addr,u16_t port)
{

	u32 data_len = 0;
	struct pbuf *q;
	if(p!=NULL)	//���յ���Ϊ�յ�����ʱ
	{
		memset(udp_demo_recvbuf,0,UDP_DEMO_RX_BUFSIZE);  //���ݽ��ջ���������
		for(q=p;q!=NULL;q=q->next)  //����������pbuf����
		{
			//�ж�Ҫ������UDP_DEMO_RX_BUFSIZE�е������Ƿ����UDP_DEMO_RX_BUFSIZE��ʣ��ռ䣬�������
			//�Ļ���ֻ����UDP_DEMO_RX_BUFSIZE��ʣ�೤�ȵ����ݣ�����Ļ��Ϳ������е�����
			if(q->len > (UDP_DEMO_RX_BUFSIZE-data_len)) memcpy(udp_demo_recvbuf+data_len,q->payload,(UDP_DEMO_RX_BUFSIZE-data_len));//��������
			else memcpy(udp_demo_recvbuf+data_len,q->payload,q->len);
			data_len += q->len;  	
			if(data_len > UDP_DEMO_RX_BUFSIZE) break; //����TCP�ͻ��˽�������,����	
		}
		pbuf_free(p);//�ͷ��ڴ�
        flag_1 = 1;           
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
struct tcp_client_struct
{
	u8 state;               //��ǰ����״
	struct tcp_pcb *pcb;    //ָ��ǰ��pcb
	struct pbuf *p;         //ָ�����/�����pbuf
};  

//tcp����������״̬
enum tcp_client_states
{
	ES_TCPCLIENT_NONE = 0,		//û������
	ES_TCPCLIENT_CONNECTED,		//���ӵ��������� 
	ES_TCPCLIENT_CLOSING,		//�ر�����
}; 







//lwIP tcp_recv()�����Ļص�����
err_t tcp_client_recv(void *arg,struct tcp_pcb *tpcb,struct pbuf *p,err_t err)
{ 
	u32 data_len = 0;
	struct pbuf *q;
	struct tcp_client_struct *es;
	err_t ret_err; 
	LWIP_ASSERT("arg != NULL",arg != NULL);
	es=(struct tcp_client_struct *)arg; 
    
    if(es->state==ES_TCPCLIENT_CONNECTED)	//����������״̬ʱ
	{
		if(p!=NULL)//����������״̬���ҽ��յ������ݲ�Ϊ��ʱ
		{
			memset(tcp_demo_recvbuf,0,TCP_CLIENT_RX_BUFSIZE);  //���ݽ��ջ���������
			for(q=p;q!=NULL;q=q->next)  //����������pbuf����
			{
				//�ж�Ҫ������TCP_CLIENT_RX_BUFSIZE�е������Ƿ����TCP_CLIENT_RX_BUFSIZE��ʣ��ռ䣬�������
				//�Ļ���ֻ����TCP_CLIENT_RX_BUFSIZE��ʣ�೤�ȵ����ݣ�����Ļ��Ϳ������е�����
				if(q->len > (TCP_CLIENT_RX_BUFSIZE-data_len)) memcpy(tcp_demo_recvbuf+data_len,q->payload,(TCP_CLIENT_RX_BUFSIZE-data_len));//��������
				else memcpy(tcp_demo_recvbuf+data_len,q->payload,q->len);
				data_len += q->len;  	
				if(data_len > TCP_CLIENT_RX_BUFSIZE) break; //����TCP�ͻ��˽�������,����	
			}
			flag_1  =1;

 			tcp_recved(tpcb,p->tot_len);//���ڻ�ȡ��������,֪ͨLWIP���Ի�ȡ��������
			pbuf_free(p);  	//�ͷ��ڴ�
			ret_err=ERR_OK;
		}
	}
	return ret_err;
}

void tcp_client_error(void *arg,err_t err)
{  
	//�������ǲ����κδ���
} 

//�˺���������������
void tcp_client_senddata(struct tcp_pcb *tpcb, struct tcp_client_struct * es)
{
	struct pbuf *ptr; 
 	err_t wr_err=ERR_OK;
	while((wr_err==ERR_OK)&&es->p&&(es->p->len<=tcp_sndbuf(tpcb)))
	{
		ptr=es->p;
		wr_err=tcp_write(tpcb,ptr->payload,ptr->len,1); //��Ҫ���͵����ݼ��뵽���ͻ��������
		if(wr_err==ERR_OK)
		{  
			es->p=ptr->next;			//ָ����һ��pbuf
			if(es->p)pbuf_ref(es->p);	//pbuf��ref��һ
			pbuf_free(ptr);				//�ͷ�ptr 
		}else if(wr_err==ERR_MEM)es->p=ptr;
		tcp_output(tpcb);		//�����ͻ�������е������������ͳ�ȥ
	} 	
} 

//lwIP tcp_sent�Ļص�����(����Զ���������յ�ACK�źź�������)
err_t tcp_client_sent(void *arg, struct tcp_pcb *tpcb, u16_t len)
{
	struct tcp_client_struct *es;
	LWIP_UNUSED_ARG(len);
	es=(struct tcp_client_struct*)arg;
	if(es->p)tcp_client_senddata(tpcb,es);//��������
	return ERR_OK;
}




//lwIP tcp_poll�Ļص�����
err_t tcp_client_poll(void *arg, struct tcp_pcb *tpcb)
{
	err_t ret_err;
	struct tcp_client_struct *es; 
	es=(struct tcp_client_struct*)arg;
	if(es!=NULL)  //���Ӵ��ڿ��п��Է�������
	{
		if(flag_1 == 1)	//�ж��Ƿ�������Ҫ���� 
		{
            flag_1 = 0;
            
			es->p=pbuf_alloc(PBUF_TRANSPORT, strlen((char*)tcp_demo_sendbuf),PBUF_POOL);	//�����ڴ� 
            
			pbuf_take(es->p,(char*)tcp_demo_sendbuf,strlen((char*)tcp_demo_sendbuf));	//��tcp_client_sentbuf[]�е����ݿ�����es->p_tx��
			tcp_client_senddata(tpcb,es);//��tcp_client_sentbuf[]���渴�Ƹ�pbuf�����ݷ��ͳ�ȥ
			
			if(es->p)pbuf_free(es->p);	//�ͷ��ڴ�
		}
        else if(es->state==ES_TCPCLIENT_CLOSING)
		{ 
 			//tcp_client_connection_close(tpcb,es);//�ر�TCP����
		} 
		ret_err=ERR_OK;
	}
    else
	{ 
		tcp_abort(tpcb);//��ֹ����,ɾ��pcb���ƿ�
		ret_err=ERR_ABRT;
	}
	return ret_err;
}
//lwIP TCP���ӽ�������ûص�����
err_t tcp_client_connected(void *arg, struct tcp_pcb *tpcb, err_t err)
{
	struct tcp_client_struct *es=NULL;  
	if(err==ERR_OK)   
	{
		es=(struct tcp_client_struct*)mem_malloc(sizeof(struct tcp_client_struct));  //�����ڴ�
		if(es) //�ڴ�����ɹ�
		{
 			es->state=ES_TCPCLIENT_CONNECTED;//״̬Ϊ���ӳɹ�
			es->pcb=tpcb;  
			es->p=NULL; 
			tcp_arg(tpcb,es);        			//ʹ��es����tpcb��callback_arg
			tcp_recv(tpcb,tcp_client_recv);  	//��ʼ��LwIP��tcp_recv�ص�����   
			tcp_err(tpcb,tcp_client_error); 	//��ʼ��tcp_err()�ص�����
			tcp_sent(tpcb,tcp_client_sent);		//��ʼ��LwIP��tcp_sent�ص�����
			tcp_poll(tpcb,tcp_client_poll,1); 	//��ʼ��LwIP��tcp_poll�ص����� 
 			err=ERR_OK;
		}
	}
	return err;
}



void task_tcp_test()
{
	struct ip_addr rmtipaddr;  	//Զ��ip��ַ
    struct tcp_pcb *tcppcb;  	//����һ��TCP����������

	tcppcb=tcp_new();	//����һ���µ�pcb
	if(tcppcb)			//�����ɹ�
	{
		IP4_ADDR(&rmtipaddr, 192, 168, 1, 11);   //Զ��ip
        //while()
        {
            tcp_connect(tcppcb,&rmtipaddr,UDP_DEMO_PORT,tcp_client_connected);  //���ӵ�Ŀ�ĵ�ַ��ָ���˿���,�����ӳɹ���ص�tcp_client_connected()����
            vTaskDelay(1000/portTICK_RATE_MS);
        }
 	}
    
    while(1)
    {
        if(flag_1 == 1) //���յ�����
        {
            flag_1 = 0;
            
        }        
        vTaskDelay(2000/portTICK_RATE_MS);
    }
}




int main()
{
    g_app_hd_init();
    
    xTaskCreate(task_led_on,"vTask_XlinkLoop_Thread",500,NULL,2,NULL);
    xTaskCreate(task_led_off,"vTask_XlinThread",500,NULL,1,NULL);
    xTaskCreate(task_tcp_test,"vTask_XlinThread",1000,NULL,3,NULL);
    vTaskStartScheduler();
    while(1)
    {
        
    }
}
