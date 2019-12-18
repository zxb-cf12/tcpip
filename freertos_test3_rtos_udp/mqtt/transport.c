/*******************************************************************************
 * Copyright (c) 2014 IBM Corp.
 *
 * All rights reserved. This program and the accompanying materials
 * are made available under the terms of the Eclipse Public License v1.0
 * and Eclipse Distribution License v1.0 which accompany this distribution.
 *
 * The Eclipse Public License is available at
 *    http://www.eclipse.org/legal/epl-v10.html
 * and the Eclipse Distribution License is available at
 *   http://www.eclipse.org/org/documents/edl-v10.php.
 *
 * Contributors:
 *    Ian Craggs - initial API and implementation and/or initial documentation
 *    Sergio R. Caprile - "commonalization" from prior samples and/or documentation extension
 *******************************************************************************/

//#include <sys/types.h>
#include "transport.h"
#include "lwip/opt.h" 
#include "lwip/arch.h" 
#include "lwip/api.h" 
#include "lwip/inet.h"  
#include "string.h"
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>


/**
This simple low-level implementation assumes a single connection for a single thread. Thus, a static
variable is used for that connection.
On other scenarios, the user must solve this by taking into account that the current implementation of
MQTTPacket_read() has a function pointer for a function call to get the data to a buffer, but no provisions
to know the caller or other indicator (the socket id): int (*getfn)(unsigned char*, int)
*/

int transport_sendPacketBuffer(struct netconn * tcp_client, unsigned char* buf, int buflen)
{
    // 
    int err;
    
    err = netconn_write(tcp_client ,buf,buflen,NETCONN_COPY); //发送tcp_server_sentbuf中的数据
    
    return err;
}


int transport_getdata(unsigned char* buf, int count)
{

    
}

int transport_getdatanb(void *sck, unsigned char* buf, int count)
{

}

/**
return >=0 for a socket descriptor, <0 for an error code
@todo Basically moved from the sample without changes, should accomodate same usage for 'sock' for clarity,
removing indirections
*/
int transport_open(char* addr, int port)
{

}

int transport_close(int sock)
{

}
