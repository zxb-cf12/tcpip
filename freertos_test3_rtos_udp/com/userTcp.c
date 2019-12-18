#include "userTcp.h"
#include "stm32f10x.h"                  // Device header
#include "main.h"
#include "lwip/tcp.h"
#include "xlink_app.h"

/**************************************************************************
** ����	@brief : TCP�͑��ͨ������,�Լ���������
** ����	@param :
** ���	@retval:
***************************************************************************/
void vTask_TcpAsClient_Thread(void *parm)
{
	err_t err;
	ip_addr_t addr;
	TcpThreadParm_t *tcpParm = (TcpThreadParm_t*)parm;
	uint32_t begin = sys_now();
    tcpParm->tcpconn = netconn_new(NETCONN_TCP);//建立一个tcp连接

    while(1)
    {
        if(ERR_OK == netconn_gethostbyname("cm2.xlink.cn", &addr)) //根据主机名获取IP
        {
            os_printf("xlink_tcp: ip=%s\n", ipaddr_ntoa(&addr));
            begin = sys_now();
            break;
        }
        else if(sys_now()-begin<60*1000) //60s 不断获取
        {
            vTaskDelay(100/portTICK_RATE_MS);
        }
        else //超过60s通过gprs连接
        {
            os_printf("get hostname timeout so run by gprs\n");
            gbPcb.accessType = 1;
            GPRSACCESS();
            vTaskDelay(60*1000/portTICK_RATE_MS);								//1���Ӻ��ٻ�ȡ����
            begin = sys_now();
        }
    }
    //当网络通信正常时，才会运行到这
    while(1)
    {
        if(ERR_OK == netconn_connect(tcpParm->tcpconn,&addr,23778))
        {
            os_printf("connect server ok\r\n");
            gbPcb.accessType = 0;
            XlinkSystemSetWifiStatus(0);
            ETHACCESS();

            netconn_set_sendtimeout(tcpParm->tcpconn,tcpParm->sendTimeout);
            netconn_set_recvtimeout(tcpParm->tcpconn,tcpParm->recvTimeout);	// ���ý��ճ�ʱʱ��
            gbPcb.connectServerStatus = 1;
            user_config.setServerStatus(1, 0);
            XlinkInitData();
            XlinkSystemSetWifiStatus(1);

            uint32_t reconn_recvTimeoutCnt=0;
            while(1)
            {
                if(0 == gbPcb.connectServerStatus)
                {
                    os_printf("connectServerStatus=0\r\n");
                    break;
                }
                struct netbuf *netBuf;
                err = netconn_recv(tcpParm->tcpconn, &netBuf);

                if(ERR_OK == err)
                {
                    reconn_recvTimeoutCnt = 0;
                    if(tcpParm->recvCb)
                    {
                        DataBufType_Typedef dataBufType;
                        dataBufType.datLen = netBuf->p->tot_len;
                        dataBufType.dat = pvPortMalloc(dataBufType.datLen);
                        dataBufType.needFree = 1;
                        if(dataBufType.dat)
                        {
                            netbuf_copy(netBuf,dataBufType.dat,dataBufType.datLen);
                            tcpParm->recvCb(tcpParm->tcpconn,dataBufType.dat,dataBufType.datLen);
                            vPortFree(dataBufType.dat);
                        }
                    }
                    netbuf_delete(netBuf);
                }
                else if(ERR_TIMEOUT == err)
                {
                    reconn_recvTimeoutCnt++;
                    if(reconn_recvTimeoutCnt>2)
                    {
                        os_printf("recv timeout\r\n");
                        break;
                    }
                }
                else
                {
                    os_printf("err = %d\r\n",err);
                    break;
                }
            }
            os_printf("exit.\r\n");
            netconn_disconnect(tcpParm->tcpconn);
            gbPcb.connectServerStatus = 0;
            user_config.setServerStatus(0, 0);
            XlinkSystemSetWifiStatus(0);
            begin = sys_now();
        }
        else if(sys_now()-begin < 60*1000)
        {
            vTaskDelay(100/portTICK_RATE_MS);
        }
        else
        {
            gbPcb.accessType = 1;
			GPRSACCESS();
            vTaskDelay(60*1000/portTICK_RATE_MS);
			begin = sys_now();
        }
    }
}
/**************************************************************************
** ����	@brief : tcp���ͺ���
** ����	@param :
** ���	@retval:
***************************************************************************/
void TcpAsSorC_Send(struct netconn *tcpconn,void *datBuf,uint32_t len)
{
	if(tcpconn == NULL)
		return;
	netconn_write(tcpconn,datBuf,len, NETCONN_COPY);
}
