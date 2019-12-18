/*************************************************************************
**		(C) COPYRIGHT 2011~  吴健超(DGUT)
**		    源码未经检录,使用需谨慎,修改请注明原作者
** 文件名 @file	   :
** 创建人 @author  : 吴健超
** 版本   @version : V 1.0.0 原始版本
** 日期   @date    :
** 功能   @brief   :
** 硬件   @hardware：
** 其他   @other   ：
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
	struct ip_addr ip;	// 解析IP，ip.addr = 0是表示解析失败
	uint16_t port;		// 端口
	HttpType_Typedef type;
	char *pathAddr;		// path指向url的地址
	uint16_t pathLen;	// path长度
}urlAnalyzeStruct;

/**************************************************************************
** 功能	@brief :url 解析
** 输入	@param :         
** 输出	@retval: urlAS.ip.addr=0是解析失败       
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
	
	//******判断HTTP请求类型 POST/GET
	if(strncasecmp(url,"GET ",4) == 0)
	{
		urlAS.type = HTTP_TYPE_GET;
		tempUrl += 4;
	}
	else if(strncasecmp(url,"POST ",5) == 0)	// 默认POST类型
	{
		urlAS.type = HTTP_TYPE_POST;
		tempUrl += 5;
	}
	else	// 解析POST/GET类型失败
	{
		return urlAS;
	}
	if(*tempUrl)
	{	// 存在http://就过滤
		char *tp = strstr(tempUrl,"://");
		if(NULL != tp)
		{
			tempUrl = tp+3;
		}
	}
	//******获取链接域名
	if(*tempUrl)
	{
		char *hostName = pvPortMalloc(url+urlLen - tempUrl);
		if(hostName)
		{
			sscanf(tempUrl,"%[^:/\r\n]:%hd",hostName,&urlAS.port);	//获取hostName和端口
			urlAS.pathAddr = strstr(tempUrl,"/");	// 查找第一个'/'即为path开始			
			if(urlAS.pathAddr)						// 如果存在path
			{
				urlAS.pathAddr += 1;				// 过滤掉路径的第一个'/'
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
** 功能	@brief : HTTP响应头解析
** 输入	@param :         
** 输出	@retval: 返回实体内容长度，0为失败       
***************************************************************************/
uint32_t HttpResponeHeadAnalyze(char *httpRspData)
{
	uint32_t len = 0;
	char *tp;
	// 判断返回头是否是HTTP/1.1 200 OK
	if(strncasecmp(httpRspData,HTTP_HEAD_SUCCESS,sizeof(HTTP_HEAD_SUCCESS)-1)!=0)
	{
		return 0;
	}
	// 获取内容长度
	tp = strstr(httpRspData,HTTP_HEAD_DATALEN);
	if(tp == NULL)
	{
		return 0;
	}
	len = atoi(tp+sizeof(HTTP_HEAD_DATALEN)-1);
	return len;
}
/**************************************************************************
** 功能	@brief :
** 输入	@param :
** 输出	@retval:        
***************************************************************************/
err_t HttpSend(char *url,char *parmData,uint32_t *writeSuccLen)
{
	err_t err = ERR_MEM;
	urlAnalyzeStruct urlAS;		// 保存url解析结果
	struct netconn *tcpconn;
	struct netbuf *netBuf;
	uint32_t datLen = 0;
	
	(*writeSuccLen) = 0;
	tcpconn = netconn_new(NETCONN_TCP);
	if(tcpconn == NULL)
	{
		return ERR_VAL;
	}
	
	/*解析url*/
	urlAS= urlAnalyze(url,strlen(url));
	if(urlAS.ip.addr == 0)	// 解析url失败
	{
		netconn_delete(tcpconn);
		return ERR_RTE;
	}
	
	netconn_set_recvtimeout(tcpconn,5000);	// 设置接收超时时间
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
	datLen = 0;	// 从此处开始，datLen表示整个返回数据包的大小，(*writeSuccLen)表示已经接收到的数据大小
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
		
		if(datLen == 0)	// 第一片包
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
			// 接收到的文件保存到Flash中
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
			if((*writeSuccLen) >= datLen)	// 接收完毕
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
