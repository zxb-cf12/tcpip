/*************************************************************************
**		(C) COPYRIGHT 2011~  �⽡��(DGUT)
**		    Դ��δ����¼,ʹ�������,�޸���ע��ԭ����
** �ļ��� @file	   :
** ������ @author  : �⽡��
** �汾   @version : V 1.0.0 ԭʼ�汾
** ����   @date    :
** ����   @brief   :
** Ӳ��   @hardware��
** ����   @other   ��
***************************************************************************/
#include "UserHttp.h"
#include "stm32f10x.h"                  // Device header
#include "main.h"

#define HTTP_HEAD_SUCCESS	"HTTP/1.1 200 OK"
#define HTTP_HEAD_DATALEN	"Content-Length: "

	
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
	struct ip_addr ip;	// ����IP��ip.addr = 0�Ǳ�ʾ����ʧ��
	uint16_t port;		// �˿�
	HttpType_Typedef type;
	char *pathAddr;		// pathָ��url�ĵ�ַ
	uint16_t pathLen;	// path����
}urlAnalyzeStruct;

/**************************************************************************
** ����	@brief :url ����
** ����	@param :         
** ���	@retval: urlAS.ip.addr=0�ǽ���ʧ��       
***************************************************************************/
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
	
	//******�ж�HTTP�������� POST/GET
	if(strncasecmp(url,"GET ",4) == 0)
	{
		urlAS.type = HTTP_TYPE_GET;
		tempUrl += 4;
	}
	else if(strncasecmp(url,"POST ",5) == 0)	// Ĭ��POST����
	{
		urlAS.type = HTTP_TYPE_POST;
		tempUrl += 5;
	}
	else	// ����POST/GET����ʧ��
	{
		return urlAS;
	}
	if(*tempUrl)
	{	// ����http://�͹���
		char *tp = strstr(tempUrl,"://");
		if(NULL != tp)
		{
			tempUrl = tp+3;
		}
	}
	//******��ȡ��������
	if(*tempUrl)
	{
		char *hostName = pvPortMalloc(url+urlLen - tempUrl);
		if(hostName)
		{
			sscanf(tempUrl,"%[^:/\r\n]:%hd",hostName,&urlAS.port);	//��ȡhostName�Ͷ˿�
			urlAS.pathAddr = strstr(tempUrl,"/");	// ���ҵ�һ��'/'��Ϊpath��ʼ			
			if(urlAS.pathAddr)						// �������path
			{
				urlAS.pathAddr += 1;				// ���˵�·���ĵ�һ��'/'
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
/**************************************************************************
** ����	@brief : HTTP��Ӧͷ����
** ����	@param :         
** ���	@retval: ����ʵ�����ݳ��ȣ�0Ϊʧ��       
***************************************************************************/
uint32_t HttpResponeHeadAnalyze(char *httpRspData)
{
	uint32_t len = 0;
	char *tp;
	// �жϷ���ͷ�Ƿ���HTTP/1.1 200 OK
	if(strncasecmp(httpRspData,HTTP_HEAD_SUCCESS,sizeof(HTTP_HEAD_SUCCESS)-1)!=0)
	{
		return 0;
	}
	// ��ȡ���ݳ���
	tp = strstr(httpRspData,HTTP_HEAD_DATALEN);
	if(tp == NULL)
	{
		return 0;
	}
	len = atoi(tp+sizeof(HTTP_HEAD_DATALEN)-1);
	return len;
}
/**************************************************************************
** ����	@brief :
** ����	@param :
** ���	@retval:        
***************************************************************************/
err_t HttpSend(char *url,char *parmData,uint32_t *writeSuccLen)
{
	err_t err = ERR_MEM;
	urlAnalyzeStruct urlAS;		// ����url�������
	struct netconn *tcpconn;
	struct netbuf *netBuf;
	uint32_t datLen = 0;
	
	(*writeSuccLen) = 0;
	tcpconn = netconn_new(NETCONN_TCP);
	if(tcpconn == NULL)
	{
		return ERR_VAL;
	}
	
	/*����url*/
	urlAS= urlAnalyze(url,strlen(url));
	if(urlAS.ip.addr == 0)	// ����urlʧ��
	{
		netconn_delete(tcpconn);
		return ERR_RTE;
	}
	
	netconn_set_recvtimeout(tcpconn,5000);	// ���ý��ճ�ʱʱ��
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
	datLen = 0;	// �Ӵ˴���ʼ��datLen��ʾ�����������ݰ��Ĵ�С��(*writeSuccLen)��ʾ�Ѿ����յ������ݴ�С
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
		
		if(datLen == 0)	// ��һƬ��
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
			// ���յ����ļ����浽Flash��
			STM32_WriteDisk(OTASTORAGE_ADDR,tp,(*writeSuccLen));
			vPortFree(tempBuf);
		}
		else
		{
			tempBufLen = (tempBufLen+(*writeSuccLen) < datLen)?tempBufLen:(datLen - (*writeSuccLen));
			STM32_WriteDisk(OTASTORAGE_ADDR+(*writeSuccLen),tempBuf,tempBufLen);
			(*writeSuccLen) += tempBufLen;
			os_printf("(*writeSuccLen)=%d/",*writeSuccLen);
			vPortFree(tempBuf);
			if((*writeSuccLen) >= datLen)	// �������
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
