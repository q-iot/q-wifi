#include "SysDefines.h"
#include "JsonClient.h"
#include "VarsJsonUpTask.h"
#include "AppClientManger.h"

#define JSON_UP_QUEUE_LEN 30

typedef struct{
	JSON_EVENT_NAME Event;

	u8 *pPath;
	u8 *pJsonStr;
	bool PathStrNeedFree:1;
	bool JsonStrNeedFree:1;

	pStdFunc pJsonReturn;

}JSON_UP_EVENT;

static OS_QUEUE_T gJsonUpQueue=NULL;//��������

static u32 gJsonQNum=0;

//POST������Ϻ���
bool SendPostRequest(const u8 *pUrl,const u8 *pJsonStr)
{
	u16 JsonStrLen=strlen(pJsonStr);
	u8 *pPostStr=Q_Malloc(JsonStrLen+256);
	u8 *pHost=Q_Malloc(32);
	u8 *p=NULL;
	u8 *pDir=NULL;
	
	QDB_GetStr(SDN_SYS,SIN_JsonServerURL,pHost);//��ȡ��������ַ
	p=strchr(pHost,':');//ȡhost
	pDir=strchr(pHost,'/');//ȡ·��
	
	if(p) {*p='\0';}
	if(pDir) {*pDir='\0';pDir++;}
	
	if(pDir) sprintf(pPostStr,"POST /%s%s HTTP/1.1\r\n",pDir,pUrl);
	else sprintf(pPostStr,"POST %s HTTP/1.1\r\n",pUrl);
	sprintf(&pPostStr[strlen(pPostStr)],"Host: %s\r\n",pHost);
	sprintf(&pPostStr[strlen(pPostStr)],"Connection: close\r\n");
	sprintf(&pPostStr[strlen(pPostStr)],"Content-Type: application/json; charset=utf-8\r\n");
	sprintf(&pPostStr[strlen(pPostStr)],"Content-Length: %u\r\n\r\n",JsonStrLen);
	sprintf(&pPostStr[strlen(pPostStr)],"%s",pJsonStr);

	//if(NeedDebug(DFT_SRV_JSON)) Debug("SendJson[%u]:%s[END]\r\n",JsonStrLen,pJsonStr);
	if(NeedDebug(DFT_SRV_JSON)) Debug(pPostStr);
	//if(NeedDebug(DFT_SRV_JSON)) DisplayBuf(pPostStr,128,16);
	//if(NeedDebug(DFT_SRV_JSON)) Debug("[%u]END\n\r",strlen(pPostStr));

	SendToJsonSrv(pPostStr,strlen(pPostStr));

	Q_Free(pPostStr);
	Q_Free(pHost);
}

//�ظ����Ľ���
//����״̬���json�ַ�����ַ
u16 HttpRespParse(const char *pData,char **pJsonStr,u16 *pJsonLen)
{
	char *p1=NULL;
	char *p2=NULL;
	u16 RespCode=0;

	if(pData==NULL) return 0;
	
	//����״̬��
	p1=strstr(pData,"HTTP");
	if(p1==NULL) p1=strstr(pData,"http");
	if(p1==NULL) return 400;
	p2=strchr(p1,' ');
	if(p2==NULL) return 400;

	while(*p2==' ') p2++;
	while(*p2)//ת��������
	{
		if(*p2<'0'||*p2>'9') break;//str����ȷ�ַ�
		else RespCode=RespCode*10+(*p2-'0');
		p2++;
	}

	if(RespCode==200)//״̬����ȷ
	{
		u16 JsonLen=0;
		p1=strstr(pData,"Content-Length:");//��ȡ���ݳ���
		if(p1==NULL) p1=strstr(pData,"content-length:");
		if(p1==NULL) return 400;

		p1=strchr(p1,':');
		p2=strstr(p1,"\r\n");
		if(p2==NULL) return 400;
		JsonLen=Str2Uint(&((char *)p1)[1]);//��ȡcontent����
		if(JsonLen==0) return 400;

		//�ҵ��洢�ַ����ĵط�
		p1=strstr(p2,"\r\n\r\n{");
		if(p1==NULL) return 400;
		p2=strchr(p1,'{');
		p1=strrchr(p2,'}');
		if(p1==NULL) return 400;
		//Debug("p2=%s,%u,%u-%u\n\r",p2,JsonLen,(u32)p2,(u32)p1);
		if(JsonLen!=((u32)p1-(u32)p2+1)) return 400;//�����ְ��ˣ����Գ��ȱ������		

		if(pJsonStr) *pJsonStr=p2;
		if(pJsonLen) *pJsonLen=JsonLen;
	}
	
	return RespCode;
}

//�������д�����
//����:���ն���-��������-���-����-��ȡ�ذ�����-�ر�����
static void JsonConnTask_WaitEvent(IP_ADDR *pServerIp,u16 ServerPort)
{
	JSON_UP_EVENT JsonEvtItem;
				
	while(1)
	{
		JsonEvtItem.Event=0;
		OS_QueueReceive(gJsonUpQueue,&JsonEvtItem,OS_MAX_DELAY);
		gJsonQNum--;
		
		switch(JsonEvtItem.Event)
		{
			case JEN_UP://�ϱ���Ϣ
				if(SysVars()->StopDataUp)
				{
					if(JsonEvtItem.PathStrNeedFree) Q_Free(JsonEvtItem.pPath);//�����ڴ�
					if(JsonEvtItem.JsonStrNeedFree) Q_Free(JsonEvtItem.pJsonStr);//�����ڴ�
				}
				else
				{
					NET_CONN_T *SrvConn;
					NET_BUF_T *NetBuf;
					NET_ERR_T Error;

					SrvConn=netconn_new(NETCONN_TCP);      //����TCP����   
					netconn_bind(SrvConn,IP_ADDR_ANY,20000+Rand(0x1ff));           //�󶨱��ص�ַ�ͼ����Ķ˿ں� 
					netconn_set_sendtimeout(SrvConn,3*1000);//���ͳ�ʱ�趨
					netconn_set_recvtimeout(SrvConn,3*1000);//���ó�ʱ����			
					Error=netconn_connect(SrvConn,pServerIp,ServerPort);            //�������� cnn��Ŀ���IP���˿ں�  
	    			if(Error==ERR_OK)  //���ӳɹ�
					{
						if(NeedDebug(DFT_SRV_JSON)) Debug("Json connect ok!%x\n\r",SrvConn);
						SysVars()->JsonConnStaus=SCS_ONLINE;
						SysVars()->JsonConnPoint=SrvConn;//��¼���������

						SendPostRequest(JsonEvtItem.pPath,JsonEvtItem.pJsonStr);//����json�ַ���
						if(JsonEvtItem.PathStrNeedFree) Q_Free(JsonEvtItem.pPath);//�����ڴ�
						if(JsonEvtItem.JsonStrNeedFree) Q_Free(JsonEvtItem.pJsonStr);//�����ڴ�

						//�ȴ��ظ�����
						while((Error = netconn_recv(SrvConn, &NetBuf)) == ERR_OK) //�����˴�����ʼ������Ϣ
						{
							if(NetBuf->p->tot_len)
							{
								char *pData=Q_Malloc(2000);//������һ�㣬��ֹ���
								char *p=NULL;
								u16 RecvLen=0;
								u16 Len,JsonLen;
								u16 RespCode;
								
								do //��ȡ����
								{
								     netbuf_data(NetBuf, (void *)&p, &Len);//��ȡָ��ͳ���
								     if(RecvLen+Len<2000)
								     {
									     MemCpy(&pData[RecvLen],p,Len);//ֱ�ӿ�������
									     RecvLen+=Len;
								     }
								}while(netbuf_next(NetBuf) >= 0);
								netbuf_delete(NetBuf);
								pData[RecvLen]='\0';

								if(NeedDebug(DFT_SRV_JSON)) Debug("\n\rJRecv[%u]\n\r",RecvLen);
								//Debug(pData);
								//DisplayBuf(pData,RecvLen,16);

								RespCode=HttpRespParse(pData,&p,&JsonLen);//���Ľ���

								if(NeedDebug(DFT_SRV_JSON)) Debug("Code[%u]\r\n",RespCode);
								if(RespCode==403 || RespCode==404 || RespCode==500 || RespCode==503)
								{
									if(NeedDebug(DFT_SRV_JSON)) Debug("Resp %u, Stop Post!\n\r",RespCode);
									SysVars()->StopDataUp=TRUE;//ֹͣ�ϱ�
								}
								if(JsonLen && NeedDebug(DFT_SRV_JSON)) DisplayStrN(p,JsonLen);
								JsonEvtItem.pJsonReturn(JsonLen,p);
								Q_Free(pData);
 							}
						}

						//���ӹرպ�ͻ��˳�while�������ʱҲ��
					}
					else //����δ�ɹ�
					{
						Debug("Json_%x conn err \"%s\"\n\r",SrvConn,lwip_strerr(Error));
						
						while(OS_QueueReceive(gJsonUpQueue,&JsonEvtItem,0))//���Ӵ��󣬽�������գ�����������ݹ������
						{
							gJsonQNum--;Debug("-JsonQ%u\n\r",gJsonQNum);
							if(JsonEvtItem.PathStrNeedFree) Q_Free(JsonEvtItem.pPath);//�����ڴ�
							if(JsonEvtItem.JsonStrNeedFree) Q_Free(JsonEvtItem.pJsonStr);//�����ڴ�
						}
					}

					if(NeedDebug(DFT_SRV_JSON)) Debug("JsonNeedDel %x %x\n\r",SrvConn,SrvConn->pcb.tcp);
					netconn_close(SrvConn);//��Ȼ�Է����ˣ����������ٹ�һ��
					netconn_delete(SrvConn);

					SysVars()->JsonConnStaus=SCS_OFFLINE;
					SysVars()->JsonConnPoint=NULL;
				}
				break;
		}  
	}
}

//��ʼ�����ӷ��������������м���ѭ��
static void JsonConnTask(void *arg)  
{  
   	NET_ERR_T Error;		
    IP_ADDR ServerIp;           //Ŀ���IP  
    u16 ServerPort=0;       //Ŀ����˿ں�  

	SysVars()->JsonConnStaus=SCS_OFFLINE;
	SysVars()->JsonConnPoint=NULL;

	if(SysVars()->FinishRtcSync==FALSE) OS_TaskDelayMs(1000); 	 //�ȴ�����������½��� 

	//ͨ��DNS��Ѱ������ip
  	{
		char *pURL=Q_Malloc(LONG_STR_BYTES);
		char *pPort=NULL;
		char *p=NULL;
		
		QDB_GetStr(SDN_SYS,SIN_JsonServerURL,pURL);//��ȡ��������ַ

		pPort=strchr(pURL,':');//���Ҷ˿ں�
		if(pPort){*pPort++=0;ServerPort=Str2Uint(pPort);}	
		if(ServerPort==0) ServerPort=80;

		p=strchr(pURL,'/');
		if(p) *p='\0';//ȥ��Ŀ¼
		if(NeedDebug(DFT_SRV_JSON)) Debug("Find Json Srv:%s Port:%u\n\r",pURL,ServerPort);
		Error=netconn_gethostbyname((void *)pURL,(void *)&ServerIp);
		switch(Error)
		{
			case ERR_OK://����Ѱ��
				MemCpy(SysVars()->JsonSrvIp,&ServerIp,sizeof(SysVars()->JsonSrvIp));
				if(NeedDebug(DFT_SRV_JSON)) Debug("Json Srv Ip:%u.%u.%u.%u\n\r",DipIpAddr(ServerIp.addr));
				SysVars()->JsonConnStaus=SCS_FOUND_SRV;
				break;			

			case ERR_VAL://dns��������������
			case ERR_ARG://dns client������������
			case ERR_MEM://�ڴ����
			default:
				Debug("json dns Error!%d\n\r",Error);
				goto Restart;
		}
		Q_Free(pURL);
	}

	JsonConnSendBeat();
    OS_TaskCreate(VarsJsonUp_Task, "JsonUp Task",512,NULL,TASK_VAR_JSON_UP_PRIO,NULL); 
	JsonConnTask_WaitEvent(&ServerIp,ServerPort);//�ȴ������̷߳�json����	
	
	SysVars()->JsonConnStaus=SCS_OFFLINE;
	SysVars()->JsonConnPoint=NULL;

Restart:
	OS_TaskDelayMs(SRV_FAILD_RETRY_MS); //��ʱ�ȴ�����	
	if(SysVars()->SrvSecretKey) sys_thread_new("JsonConn   ",JsonConnTask,NULL,512,TASK_TCP_CLIENT_PRIO);//���½����̣߳�������ڵ�½�ذ�����ȷ����SysVars()->SrvSecretKey�ᱻ�忨����Ϊ0����ֹͣ����
	OS_TaskDelete(NULL);//ɾ���߳��Լ�    
}  

//��ʼ������
void JsonClient_Init(void)
{
	static bool OnlyFlag=TRUE;

	if(OnlyFlag)
	{
		gJsonUpQueue=OS_QueueCreate(JSON_UP_QUEUE_LEN,sizeof(JSON_UP_EVENT));//��һ������ָ��������ȣ��ڶ�������ָ����Ա��С
		sys_thread_new("JsonConn   ",JsonConnTask,NULL,512,TASK_TCP_CLIENT_PRIO);
		OnlyFlag=FALSE;
	}	
}

//ɾ������
void DeleteJsonConn(void)
{
	if(SysVars()->JsonConnPoint)
	{	
		if(NeedDebug(DFT_SRV_JSON)) Debug("Del Json %x\n\r",SysVars()->JsonConnPoint);
		netconn_close(SysVars()->JsonConnPoint);
		netconn_delete(SysVars()->JsonConnPoint);
		SysVars()->JsonConnStaus=SCS_OFFLINE;
		SysVars()->JsonConnPoint=NULL;
	}
}

//��Ӧ�ó�����õģ��ϱ����ݸ�json�������ĺ������첽
void UpJsonDataToSrv(const u8 *pPath,u8 *pJsonStr,pStdFunc JsonSentCb)
{
	JSON_UP_EVENT JsonItem;

	if(gJsonUpQueue==NULL) return;

	JsonItem.Event=JEN_UP;
	JsonItem.pPath=Q_Malloc(strlen(pPath)+LONG_STR_BYTES+2);//�ȴ�json������ɺ��ͷ�
	strcpy(JsonItem.pPath,pPath);
	JsonItem.PathStrNeedFree=TRUE;
	JsonItem.pJsonStr=Q_Malloc(strlen(pJsonStr)+32);//�ȴ�json������ɺ��ͷ�
	strcpy(JsonItem.pJsonStr,pJsonStr);
	JsonItem.JsonStrNeedFree=TRUE;
	JsonItem.pJsonReturn=JsonSentCb;

	
	if(OS_QueueSend(gJsonUpQueue,&JsonItem,0)!= pdPASS)
	{
		Debug("QSend Failed\n\r");
	}
	else
	{
gJsonQNum++;
	}Debug("JsonQ %u\n\r",gJsonQNum);
}


