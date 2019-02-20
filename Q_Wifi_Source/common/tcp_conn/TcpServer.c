#include "SysDefines.h"
#include "TcpServer.h"
#include "AppClientDataIn.h"
#include "AppClientManger.h"

static u8 gTcpAppClientCnt=0;//���Ӽ���

#if 0
typedef struct{
	void *pConn;
	OS_TASK_T pTaskHandler;
}TCP_CLIENT_TASK;

static volatile TCP_CLIENT_TASK gTcpClientTasks[MAX_APP_CLIENT_NUM];

void RecordTcpClientTask(void *pConn,OS_TASK_T pTaskHandler)
{
	u16 i;

	if(pConn == NULL || pTaskHandler == NULL) return;

	OS_EnterCritical();
	for(i=0;i<MAX_APP_CLIENT_NUM;i++)
	{
		if(gTcpClientTasks[i].pConn==pConn)
		{
			gTcpClientTasks[i].pTaskHandler=pTaskHandler;
			OS_ExitCritical();
			return;
		}
	}

	for(i=0;i<MAX_APP_CLIENT_NUM;i++)
	{
		if(gTcpClientTasks[i].pConn==0)
		{
			gTcpClientTasks[i].pConn=pConn;
			gTcpClientTasks[i].pTaskHandler=pTaskHandler;
			OS_ExitCritical();
			return;
		}
	}	
}

OS_TASK_T FindTcpClientTask(void *pConn)
{
	u16 i;
	OS_TASK_T Ret=NULL;

	if(pConn == NULL) return NULL;

	OS_EnterCritical();
	for(i=0;i<MAX_APP_CLIENT_NUM;i++)
	{
		if(gTcpClientTasks[i].pConn==pConn)
		{
			Ret=gTcpClientTasks[i].pTaskHandler;
			OS_ExitCritical();
			return Ret;
		}
	}

	return NULL;
}

void DeleteTcpClientTask(void *pConn)
{
	u16 i;

	if(pConn == NULL) return;

	OS_EnterCritical();
	for(i=0;i<MAX_APP_CLIENT_NUM;i++)
	{
		if(gTcpClientTasks[i].pConn==pConn)
		{
			gTcpClientTasks[i].pConn=NULL;
			gTcpClientTasks[i].pTaskHandler=NULL;
			OS_ExitCritical();
			return;
		}
	}
}

#endif


//ÿ�����������Ŀͻ��ˣ���ӵ�ж������߳�
static void NewClient_Thread(NET_CONN_T *NewConn)
{
	NET_BUF_T *NetBuf;
	u8 *pTail=NULL;
	u16 *pHeader=NULL;
	void *p;
	u16 Len,TailLen=0;
	NET_ERR_T Error;	
	
	if(AppClientIsFull()==FALSE)//����¿ͻ���
	{
		netconn_set_sendtimeout(NewConn,APP_CONN_SEND_TIMOUT_S*1000);
		netconn_set_recvtimeout(NewConn,APP_CONN_RECV_TIMOUT_S*1000);//���ó�ʱ����
		while ((Error = netconn_recv(NewConn, &NetBuf)) == ERR_OK) //�����˴�����ʼ������Ϣ
		{
			if(NetBuf==NULL || NetBuf->p==NULL){Debug("NetBuf NULL\n\r");break;}
			
			if(NetBuf->p->tot_len)
			{
				u16 MallocLen=NetBuf->p->tot_len+TailLen;
				u8 *pData=Q_Zalloc(MallocLen>PKT_MAX_DATA_LEN?MallocLen:PKT_MAX_DATA_LEN);//������һ�㣬��ֹ���
				u32 RecvLen=TailLen;

				if(TailLen && pTail!=NULL)
				{
					MemCpy(pData,pTail,TailLen);//�����ϴζ���
					Q_Free(pTail);
					TailLen=0;
					pTail=NULL;					
				}
				
				do //��ȡ����
				{
				     netbuf_data(NetBuf, &p, &Len);//��ȡָ��ͳ���
				     MemCpy(&pData[RecvLen],p,Len);//ֱ�ӿ�������
				     RecvLen+=Len;
				}while(netbuf_next(NetBuf) >= 0);
				netbuf_delete(NetBuf);
					
				if(NeedDebug(DFT_APP)) Debug("\n\rRecv[%u]\n\r",RecvLen);
				//DisplayBuf(pData,RecvLen,16);

RecvHandle:				
				//�ָ������Ͷ�������
				pHeader=(void *)pData;				
				if(	RecvLen < 4 || (pHeader[0]==0) || (pHeader[0]!=(pHeader[1]^NET_PKTLEN_CHK_CODE)) )//��ͷ�����������
				{
					Debug("PktHeader error!\n\r");
					goto RecvFinish;					
				}

				if(RecvLen>pHeader[0])//�ն���
				{
					TailLen=RecvLen-pHeader[0];//���յĳ���
					RecvLen=pHeader[0];//�������ʹ��
					pTail=Q_Zalloc(TailLen);
					MemCpy(pTail,&pData[pHeader[0]],TailLen);//��������Ĳ���
					MemSet(&pData[pHeader[0]],0,TailLen);
				}
				else if(RecvLen<pHeader[0])//�����ˣ��´�����
				{
					pTail=pData;
					pData=NULL;
					TailLen=RecvLen;
					goto RecvFinish;
				}

				//�ָ���ϣ����԰���ʹ��pData���������
				{
					PKT_HANDLER_RES HandlerRes;
					HandlerRes=AppClientDataInHandler(NewConn,(void *)pData,RecvLen);//�����
					switch(HandlerRes)
					{
						case PHR_FALSE:
						case PHR_OK://ͬ���ظ�
						    Error = netconn_write(NewConn,pData,pHeader[0],NETCONN_COPY);
						    if (Error != ERR_OK) 
						    {
						    	Debug("APU_%x write \"%s\"\n\r",NewConn,lwip_strerr(Error));
						    	Q_Free(pData);
						    	goto ErrorHandler;
						    }
							
							//Debug("SSSS,Res:%s,T%d\n\r",gNameGlobaPktRes[pData[7]],OS_GetNowMs());
							//UserSoftPktDisplay(pData);
							//DisplayBuf((void *)pData,pHeader[0],16);
							break;
						case PHR_ASYNC://�첽�ظ�
							break;
					}
				}
				
RecvFinish:			    
				Q_Free(pData);
				RecvLen=0;

				if(TailLen && pTail!=NULL && (*(u16 *)pTail)<=TailLen) //��������ݣ������γ�һ����
				{
					pData=Q_Zalloc(TailLen>PKT_MAX_DATA_LEN?TailLen:PKT_MAX_DATA_LEN);//������һ�㣬��ֹ���
					RecvLen=TailLen;
					
					MemCpy(pData,pTail,TailLen);//�����ϴζ���
					Q_Free(pTail);
					TailLen=0;
					pTail=NULL;					
					
					goto RecvHandle;
				}
			}
		}

ErrorHandler:		
		Debug("APU_%x \"%s\"\n\r",NewConn,lwip_strerr(Error));

		if(pTail!=NULL) Q_Free(pTail);
		PublicEvent(PET_TCP_CLIENT_LOST,AppClientGetAppID(NewConn),NewConn);//ɾ���ͻ���
	}
	
	//ɾ����Դ
	netconn_close(NewConn);
	netconn_delete(NewConn);
	if(gTcpAppClientCnt) gTcpAppClientCnt--;

	OS_TaskDelete(NULL);//ɾ���߳��Լ�
}

//������������������app�ͻ��ˣ������߳�
static void AppUserClientsHandler(void *arg)
{
	NET_CONN_T *Conn, *NewConn;
	NET_ERR_T Error;

	Conn = netconn_new(NETCONN_TCP);
	netconn_bind(Conn, IP_ADDR_ANY, QDB_GetNum(SDN_SYS,SIN_Port));
	netconn_listen(Conn);
	if(NeedDebug(DFT_APP)) Debug("Listen Port:%u\n\r",QDB_GetNum(SDN_SYS,SIN_Port));

	while(1) 
	{
		Error = netconn_accept(Conn,&NewConn);//ÿ������һ����������һ�����߳�

		if(gTcpAppClientCnt<USER_CLIENT_MAX_NUM)//����δ��
		{
			if(NeedDebug(DFT_APP)) Debug("Accepted New Client %x %x\n\r", NewConn,NewConn->pcb.tcp);
			if (Error == ERR_OK) 
			{
				u8 ThreadName[24];
				sprintf((void *)ThreadName,"AUC_%x",NewConn);
				sys_thread_new((void *)ThreadName,(void *)NewClient_Thread,NewConn,400,DEFAULT_THREAD_PRIO);
				gTcpAppClientCnt++;
			}
		}
		else//��������
		{
			Debug("AUC Conn Full!\n\r");
			netconn_close(NewConn);
			netconn_delete(NewConn);		

			//������������
		}		
	}

	netconn_delete(Conn);
}

//��ʼ������tcp������
void TcpServer_Init(void)
{
	static bool OnlyFlag=TRUE;

	if(OnlyFlag)
	{
		sys_thread_new("AppCltMonitor",AppUserClientsHandler,NULL,400,TASK_TCP_APP_PRIO);
		OnlyFlag=FALSE;
	}
}

//ɾ���ͻ�������
//ɾ��lwip��Դ���̳߳�ʱ������ɾ�����������ٽ����ڵ��ã���������
void DeleteTcpClientConn(void *pDelete)
{
	if(pDelete!=NULL && GetClientFlag(pDelete)==CLIENT_T_LOCAL)
	{	
		Debug("Del Conn %x\n\r",pDelete);
		netconn_disconnect(pDelete);
	}
}

//��ȡ��ǰ������
u8 GetTcpClientsNum(void)
{
	return gTcpAppClientCnt;
}
























