#include "SysDefines.h"
#include "TcpClient.h"
#include "AppClientManger.h"
#include "DutSrvDataComm.h"
#include "LwipMain.h"

static void SrvConnTask_WaitRecv(NET_CONN_T *SrvConn,IP_ADDR *pServerIp,u16 ServerPort)
{
	NET_BUF_T *NetBuf;
	u8 *pTail=NULL;
	u16 *pHeader=NULL;
	void *p;
	u16 Len,TailLen=0;
	NET_ERR_T Error;	
	   	
	netconn_set_sendtimeout(SrvConn,SRV_CONN_SEND_TIMOUT_S*1000);
	netconn_set_recvtimeout(SrvConn,SRV_CONN_RECV_TIMOUT_S*1000);//���ó�ʱ����
    Error=netconn_connect(SrvConn,pServerIp,ServerPort);            //�������� cnn��Ŀ���IP���˿ں�  
    if(Error==ERR_OK)  //���ӳɹ�
	{  
	    if(NeedDebug(DFT_SRV)) Debug("Server connect ok!\n\r");
	    SysVars()->SrvConnStaus=SCS_ONLINE;
	   	SysVars()->SrvConnPoint=SrvConn;//��¼���������
	   	SrvConnSendBeat(TRUE);//���͵�¼��

	   	//��ʼ��������
		{	
			while ((Error = netconn_recv(SrvConn, &NetBuf)) == ERR_OK) //�����˴�����ʼ������Ϣ
			{
				if(NetBuf->p->tot_len)
				{
					u16 MallocLen=NetBuf->p->tot_len+TailLen;
					u8 *pData=Q_Malloc(MallocLen>PKT_MAX_DATA_LEN?MallocLen:PKT_MAX_DATA_LEN);//������һ�㣬��ֹ���
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
						
					if(NeedDebug(DFT_SRV)) Debug("\n\rSRecv[%u]\n\r",RecvLen);
					//DisplayBuf(pData,RecvLen,16);

RecvHandle:				
					//�ָ������Ͷ�������
					pHeader=(void *)pData;				
					if(RecvLen < 4 || (pHeader[0]==0) || (pHeader[0]!=(pHeader[1]^NET_PKTLEN_CHK_CODE)) )//��ͷ�����������
					{
						Debug("SPktHeader error!\n\r");
						goto RecvFinish;					
					}

					if(RecvLen>pHeader[0])//�ն���
					{
						TailLen=RecvLen-pHeader[0];//���յĳ���
						RecvLen=pHeader[0];//�������ʹ��
						pTail=Q_Malloc(TailLen);
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
						HandlerRes=ServerDataInHandler(SrvConn,(void *)pData,RecvLen);//�����
						switch(HandlerRes)
						{
							case PHR_FALSE:
							case PHR_OK://ͬ���ظ�
							    Error = netconn_write(SrvConn,pData,pHeader[0],NETCONN_COPY);
							    if (Error != ERR_OK) 
							    {
							    	Debug("SRV_CONN_%x write \"%s\"\n\r",SrvConn,lwip_strerr(Error));
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
						pData=Q_Malloc(TailLen>PKT_MAX_DATA_LEN?TailLen:PKT_MAX_DATA_LEN);//������һ�㣬��ֹ���
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
			if(NeedDebug(DFT_SRV)) Debug("SRV_CONN_%x \"%s\"\n\r",SrvConn,lwip_strerr(Error));

			if(pTail!=NULL) Q_Free(pTail);
			PublicEvent(PET_TCP_SRV_LOST,0,NULL);//ɾ���ͻ���
		}
	}
}

//���ӷ��������߳�
//������һ��tcp client�����ӳ�ʱ�����˳��������Լ���
static void SrvConnTask(void *arg)  
{  
    NET_CONN_T *SrvConn;  
   	NET_ERR_T Error;	
    IP_ADDR ServerIp;           //Ŀ���IP  
    u16 ServerPort=0;       //Ŀ����˿ں�  

	SysVars()->SrvConnStaus=SCS_OFFLINE;
	SysVars()->SrvConnPoint=NULL;

	//OS_TaskDelayMs(3000); 	  

	//ͨ��DNS��Ѱ������ip
  	{
		char *pURL=Q_Malloc(NORMAL_STR_BYTES);
		char *pPort=NULL;
		
		QDB_GetStr(SDN_SYS,SIN_ServerURL,pURL);//��ȡ��������ַ
		if(NeedDebug(DFT_SRV)) Debug("Find Server:%s\n\r",pURL);
		
		//�����˶�ȡ�����˿ںŵĹ��ܣ���url��������˿ں�
		pPort=strchr(pURL,':');//���Ҷ˿ں�
		if(pPort){*pPort++=0;ServerPort=Str2Uint(pPort);}
		if(ServerPort==0) ServerPort=8500;
		
		Error=netconn_gethostbyname((void *)pURL,(void *)&ServerIp);
		switch(Error)
		{
			case ERR_OK://����Ѱ��
				MemCpy(SysVars()->ServerIp,&ServerIp,sizeof(SysVars()->ServerIp));
				if(NeedDebug(DFT_SRV)) Debug("Server Ip:%u.%u.%u.%u\n\r",DipIpAddr(ServerIp.addr));
				SysVars()->SrvConnStaus=SCS_FOUND_SRV;
				break;			

			case ERR_VAL://dns��������������
			case ERR_ARG://dns client������������
			case ERR_MEM://�ڴ����
			default:
				Debug("Tcp dns Error!%d\n\r",Error);
				goto Restart;
		}
		Q_Free(pURL);
	}
  	
    SrvConn=netconn_new(NETCONN_TCP);      /* ����TCP����  */  
    netconn_bind(SrvConn,IP_ADDR_ANY,10000+Rand(0x1ff));           /* �󶨱��ص�ַ�ͼ����Ķ˿ں� */   

	SrvConnTask_WaitRecv(SrvConn,&ServerIp,ServerPort);//����ѭ���շ���
	
	//ɾ����Դ
	if(NeedDebug(DFT_SRV)) Debug("SrvNeedDel %x %x\n\r",SrvConn,SrvConn->pcb.tcp);
	netconn_close(SrvConn);
	netconn_delete(SrvConn);

	SysVars()->SrvConnStaus=SCS_OFFLINE;
	SysVars()->SrvConnPoint=NULL;

Restart:
	OS_TaskDelayMs(SRV_FAILD_RETRY_MS); //��ʱ�ȴ�����	
	if(SysVars()->SrvSecretKey) sys_thread_new("SrvConn   ",SrvConnTask,NULL,512,TASK_TCP_CLIENT_PRIO);//���½����̣߳�������ڵ�½�ذ�����ȷ����SysVars()->SrvSecretKey�ᱻ�忨����Ϊ0����ֹͣ����
	OS_TaskDelete(NULL);//ɾ���߳��Լ�    
}  

//��ʼ������tcp���ӿͻ���
void TcpClient_Init(void)
{
	static bool OnlyFlag=TRUE;

	if(OnlyFlag)
	{
		sys_thread_new("SrvConn   ",SrvConnTask,NULL,512,TASK_TCP_CLIENT_PRIO);
		OnlyFlag=FALSE;
	}	
}

//ɾ������
void DeleteTcpSrvConn(void)
{
	if(SysVars()->SrvConnPoint)
	{	
		if(NeedDebug(DFT_SRV)) Debug("Del Conn %x\n\r",SysVars()->SrvConnPoint);
		netconn_close(SysVars()->SrvConnPoint);
		netconn_delete(SysVars()->SrvConnPoint);
		SysVars()->SrvConnStaus=SCS_OFFLINE;
		SysVars()->SrvConnPoint=NULL;
	}
}


