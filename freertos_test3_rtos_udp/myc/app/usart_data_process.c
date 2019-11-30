/****************************************
**file name :
**function  :
****************************************/

#include "usart_data_process.h"

/*************************************************************/
data_pack_t usart1_data = {0xff,0x00,STEP_START,2};
data_pack_t usart2_data = {0xff,0x00,STEP_START,2};
data_pack_t usart3_data = {0xff,0x00,STEP_START,2};
/*************************************************************/


/***************************
  @name: l_USART2_RevData_Process()
  @func: 
  @return:
***************************/
void g_USART1_RevData_Process(
    uint8_t data,                /* usart recv data*/
    data_pack_t* recv_data       /* point to recv object*/
)
{  
    static uint8_t data_len = 0;
    switch(recv_data->state){
        case STEP_START:
            if(data == recv_data->startbyte){
                recv_data->state = STEP_LEN;
            }
            break;
       case STEP_LEN:
             recv_data->len = data;
             recv_data->state = STEP_RECV;
            break;
        case STEP_RECV:
            recv_data->data[data_len++] = data;                
            if(data_len == recv_data->len){
                recv_data->state = STEP_END;
                data_len = 0;
            }
            break;
            
        case STEP_CRC: 
            recv_data->data[data_len] = data; 
            data_len = 0;
            recv_data->state = STEP_END;        
            break;          
        case STEP_END:

            break;
        
        default:
            break;
    }
}

/***************************
  @name: l_USART2_RevData_Process()
  @func: 
  @return:
***************************/
void g_USART2_RevData_Process(
    uint8_t data,                /* usart recv data*/
    data_pack_t* recv_data       /* point to recv object*/
)
{  
    static uint8_t data_len = 0;
    switch(recv_data->state){
        case STEP_START:
            if(data == recv_data->startbyte){
                if(recv_data->len > 2){
                    recv_data->state = STEP_RECV;
                }else{
                    recv_data->state = STEP_CRC;
                }
                recv_data->data[data_len++] = data;
            }
            break;
        
        case STEP_RECV:
            recv_data->data[data_len++] = data;                
            if(data_len == recv_data->len-1)
                recv_data->state = STEP_CRC;
            break;
            
        case STEP_CRC: 
            recv_data->data[data_len] = data; 
            data_len = 0;
            recv_data->state = STEP_END;        
            break;          
        case STEP_END:

            break;
        
        default:
            break;
    }
}


/***************************
  @name: l_USART2_RevData_Process()
  @func: 
  @return:
***************************/
void g_USART3_RevData_Process(
    uint8_t data,                /* usart recv data*/
    data_pack_t* recv_data       /* point to recv object*/
)
{  
    static uint8_t data_len = 0;
    switch(recv_data->state){
        case STEP_START:
            if(data == recv_data->startbyte){
                if(recv_data->len > 2){
                    recv_data->state = STEP_RECV;
                }else{
                    recv_data->state = STEP_CRC;
                }
                recv_data->data[data_len++] = data;
            }
            break;
        
        case STEP_RECV:
            recv_data->data[data_len++] = data;                
            if(data_len == recv_data->len-1)
                recv_data->state = STEP_CRC;
            break;
            
        case STEP_CRC: 
            recv_data->data[data_len] = data; 
            data_len = 0;
            recv_data->state = STEP_END;        
            break;          
        case STEP_END:

            break;
        
        default:
            break;
    }
}



/***************************
  @name: l_analysis_data()
  @func: 
  @return:
***************************/
static void l_analysis_data(
    data_pack_t usart_data  /* data object*/
)
{
    g_usartx_send_data(USART1, usart_data.data, usart_data.len);
}


/***************************
  @name: g_handle_usart1_data()
  @func: 
  @return:
***************************/
void g_handle_usart1_data()   
{
    if(USART1_RECV_DATA){
        l_analysis_data(usart1_data);
        usart1_data.state = STEP_START;
    }
}

/***************************
  @name: g_handle_usart2_data()
  @func: 
  @return:
***************************/
void g_handle_usart2_data()
{
    if(USART2_RECV_DATA){
        l_analysis_data(usart2_data);
        usart2_data.state = STEP_START;
    }
}

/***************************
  @name: g_handle_usart3_data()
  @func: 
  @return:
***************************/
void g_handle_usart3_data()
{
    if(USART3_RECV_DATA){
        l_analysis_data(usart3_data);
        usart3_data.state = STEP_START;
    }
}
