//--------------------------Q-IOT Platform------------------------------------------------------//
/*
Q-Wifi是酷享物联平台的固件实现，基于ESP8266开发，构建了一套与酷享APP、酷享云平台相互配合的物联网
底层实现机制，代码基于ESP8266_RTOS_SDK开发，开源，欢迎更多程序员大牛加入！

Q-Wifi构架了一套设备、变量、情景、动作、用户终端的管理机制，内建了若干个TCP连接与APP及云平台互动，
其中json上报连接，采用通用的http json格式，可被开发者导向至自己的物联网平台，进行数据汇集和控制。

Q-Wifi内建了一套动态的web网页服务器，用来进行硬件配置，此web网页服务器封装完整，可被开发者移植到
其他项目。

Q-Wifi内建了一套基于串口的指令解析系统，用来进行串口调试，指令下发，此解析系统封装完整，包含一系
列解析函数，可被开发者移植至其他项目。

Q-Wifi内部带user标识的文件，均为支持开发者自主修改的客制化文件，特别是user_hook.c文件，系统内所
有关键点都会有hook函数在此文件中，供开发者二次开发。

Q-Wifi代码拥有众多模块化的机制或方法，可以被复用及移植，减少物联网系统的开发难度。
所有基于酷享物联平台进行的开发或案例、产品，均可联系酷享团队，免费放置于酷物联视频（q-iot.cn）进行
传播或有偿售卖，相应所有扣除税费及维护费用后，均全额提供给贡献者，以此鼓励国内开源事业。

By Karlno 酷享科技

本文件定义了一个tcp server用于通信
*/
//---------------------------------------------------------------------------------------------//
#include "SysDefines.h"
#include "TcpServer.h"
#include "AppClientDataIn.h"
#include "AppClientManger.h"

static u8 gTcpAppClientCnt=0;//连接计数

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


//每个连接上来的客户端，都拥有独立的线程
static void NewClient_Thread(NET_CONN_T *NewConn)
{
	NET_BUF_T *NetBuf;
	u8 *pTail=NULL;
	u16 *pHeader=NULL;
	void *p;
	u16 Len,TailLen=0;
	NET_ERR_T Error;	
	
	if(AppClientIsFull()==FALSE)//添加新客户端
	{
		netconn_set_sendtimeout(NewConn,APP_CONN_SEND_TIMOUT_S*1000);
		netconn_set_recvtimeout(NewConn,APP_CONN_RECV_TIMOUT_S*1000);//设置超时掉线
		while ((Error = netconn_recv(NewConn, &NetBuf)) == ERR_OK) //阻塞此处，开始接收信息
		{
			if(NetBuf==NULL || NetBuf->p==NULL){Debug("NetBuf NULL\n\r");break;}
			
			if(NetBuf->p->tot_len)
			{
				u16 MallocLen=NetBuf->p->tot_len+TailLen;
				u8 *pData=Q_Zalloc(MallocLen>PKT_MAX_DATA_LEN?MallocLen:PKT_MAX_DATA_LEN);//多申请一点，防止溢出
				u32 RecvLen=TailLen;

				if(TailLen && pTail!=NULL)
				{
					MemCpy(pData,pTail,TailLen);//拷贝上次多余
					Q_Free(pTail);
					TailLen=0;
					pTail=NULL;					
				}
				
				do //读取数据
				{
				     netbuf_data(NetBuf, &p, &Len);//获取指针和长度
				     MemCpy(&pData[RecvLen],p,Len);//直接拷贝内容
				     RecvLen+=Len;
				}while(netbuf_next(NetBuf) >= 0);
				netbuf_delete(NetBuf);
					
				if(NeedDebug(DFT_APP)) Debug("\n\rRecv[%u]\n\r",RecvLen);
				//DisplayBuf(pData,RecvLen,16);

RecvHandle:				
				//分隔主包和多余数据
				pHeader=(void *)pData;				
				if(	RecvLen < 4 || (pHeader[0]==0) || (pHeader[0]!=(pHeader[1]^NET_PKTLEN_CHK_CODE)) )//包头长度区域错误
				{
					Debug("PktHeader error!\n\r");
					goto RecvFinish;					
				}

				if(RecvLen>pHeader[0])//收多了
				{
					TailLen=RecvLen-pHeader[0];//多收的长度
					RecvLen=pHeader[0];//方便后面使用
					pTail=Q_Zalloc(TailLen);
					MemCpy(pTail,&pData[pHeader[0]],TailLen);//拷贝多余的部分
					MemSet(&pData[pHeader[0]],0,TailLen);
				}
				else if(RecvLen<pHeader[0])//收少了，下次再收
				{
					pTail=pData;
					pData=NULL;
					TailLen=RecvLen;
					goto RecvFinish;
				}

				//分割完毕，可以安心使用pData里的数据了
				{
					PKT_HANDLER_RES HandlerRes;
					HandlerRes=AppClientDataInHandler(NewConn,(void *)pData,RecvLen);//命令处理
					switch(HandlerRes)
					{
						case PHR_FALSE:
						case PHR_OK://同步回复
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
						case PHR_ASYNC://异步回复
							break;
					}
				}
				
RecvFinish:			    
				Q_Free(pData);
				RecvLen=0;

				if(TailLen && pTail!=NULL && (*(u16 *)pTail)<=TailLen) //多余的数据，可以形成一个包
				{
					pData=Q_Zalloc(TailLen>PKT_MAX_DATA_LEN?TailLen:PKT_MAX_DATA_LEN);//多申请一点，防止溢出
					RecvLen=TailLen;
					
					MemCpy(pData,pTail,TailLen);//拷贝上次多余
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
		PublicEvent(PET_TCP_CLIENT_LOST,AppClientGetAppID(NewConn),NewConn);//删除客户端
	}
	
	//删除资源
	netconn_close(NewConn);
	netconn_delete(NewConn);
	if(gTcpAppClientCnt) gTcpAppClientCnt--;

	OS_TaskDelete(NULL);//删除线程自己
}

//处理所有连接上来的app客户端，分配线程
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
		Error = netconn_accept(Conn,&NewConn);//每连接上一个，就生成一个新线程

		if(gTcpAppClientCnt<USER_CLIENT_MAX_NUM)//连接未满
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
		else//连接已满
		{
			Debug("AUC Conn Full!\n\r");
			netconn_close(NewConn);
			netconn_delete(NewConn);		

			//可做蜂鸣报警
		}		
	}

	netconn_delete(Conn);
}

//初始化所有tcp服务器
void TcpServer_Init(void)
{
	static bool OnlyFlag=TRUE;

	if(OnlyFlag)
	{
		sys_thread_new("AppCltMonitor",AppUserClientsHandler,NULL,400,TASK_TCP_APP_PRIO);
		OnlyFlag=FALSE;
	}
}

//删除客户端连接
//删除lwip资源，线程超时后自行删除，不能在临界区内调用，否则死锁
void DeleteTcpClientConn(void *pDelete)
{
	if(pDelete!=NULL && GetClientFlag(pDelete)==CLIENT_T_LOCAL)
	{	
		Debug("Del Conn %x\n\r",pDelete);
		netconn_disconnect(pDelete);
	}
}

//获取当前连接数
u8 GetTcpClientsNum(void)
{
	return gTcpAppClientCnt;
}
























