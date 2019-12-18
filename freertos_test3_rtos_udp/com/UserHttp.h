#ifndef __USER_HTTP_H__
#define __USER_HTTP_H__
#include "stm32f10x.h"                  // Device header

#define HTTP_RESQUEST_DATA	\
	"%s /%.*s HTTP/1.1\r\n"\
	"Host: %s\r\n"\
	"\r\n"
err_t HttpSend(char *url,char *parmData,uint32_t *writeSuccLen);

#endif
