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






int main()
{
    g_app_hd_init();
    
    
    

    xTaskCreate(task_led_on,"vTask_XlinkLoop_Thread",500,NULL,2,NULL);
    xTaskCreate(task_led_off,"vTask_XlinThread",500,NULL,1,NULL);
    xTaskCreate(task_udp_test,"vTask_XlinThread",1000,NULL,3,NULL);
    vTaskStartScheduler();
    while(1)
    {
        
    }
}
