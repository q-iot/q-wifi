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
	netconn_set_recvtimeout(SrvConn,SRV_CONN_RECV_TIMOUT_S*1000);//设置超时掉线
    Error=netconn_connect(SrvConn,pServerIp,ServerPort);            //连接主机 cnn，目标机IP，端口号  
    if(Error==ERR_OK)  //连接成功
	{  
	    if(NeedDebug(DFT_SRV)) Debug("Server connect ok!\n\r");
	    SysVars()->SrvConnStaus=SCS_ONLINE;
	   	SysVars()->SrvConnPoint=SrvConn;//记录服务器句柄
	   	SrvConnSendBeat(TRUE);//发送登录包

	   	//开始正常运作
		{	
			while ((Error = netconn_recv(SrvConn, &NetBuf)) == ERR_OK) //阻塞此处，开始接收信息
			{
				if(NetBuf->p->tot_len)
				{
					u16 MallocLen=NetBuf->p->tot_len+TailLen;
					u8 *pData=Q_Malloc(MallocLen>PKT_MAX_DATA_LEN?MallocLen:PKT_MAX_DATA_LEN);//多申请一点，防止溢出
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
						
					if(NeedDebug(DFT_SRV)) Debug("\n\rSRecv[%u]\n\r",RecvLen);
					//DisplayBuf(pData,RecvLen,16);

RecvHandle:				
					//分隔主包和多余数据
					pHeader=(void *)pData;				
					if(RecvLen < 4 || (pHeader[0]==0) || (pHeader[0]!=(pHeader[1]^NET_PKTLEN_CHK_CODE)) )//包头长度区域错误
					{
						Debug("SPktHeader error!\n\r");
						goto RecvFinish;					
					}

					if(RecvLen>pHeader[0])//收多了
					{
						TailLen=RecvLen-pHeader[0];//多收的长度
						RecvLen=pHeader[0];//方便后面使用
						pTail=Q_Malloc(TailLen);
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
						HandlerRes=ServerDataInHandler(SrvConn,(void *)pData,RecvLen);//命令处理
						switch(HandlerRes)
						{
							case PHR_FALSE:
							case PHR_OK://同步回复
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
							case PHR_ASYNC://异步回复
								break;
						}
					}
					
RecvFinish:			    
					Q_Free(pData);
					RecvLen=0;

					if(TailLen && pTail!=NULL && (*(u16 *)pTail)<=TailLen) //多余的数据，可以形成一个包
					{
						pData=Q_Malloc(TailLen>PKT_MAX_DATA_LEN?TailLen:PKT_MAX_DATA_LEN);//多申请一点，防止溢出
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
			if(NeedDebug(DFT_SRV)) Debug("SRV_CONN_%x \"%s\"\n\r",SrvConn,lwip_strerr(Error));

			if(pTail!=NULL) Q_Free(pTail);
			PublicEvent(PET_TCP_SRV_LOST,0,NULL);//删除客户端
		}
	}
}

//连接服务器的线程
//启动了一个tcp client，连接超时，则退出，销毁自己。
static void SrvConnTask(void *arg)  
{  
    NET_CONN_T *SrvConn;  
   	NET_ERR_T Error;	
    IP_ADDR ServerIp;           //目标机IP  
    u16 ServerPort=0;       //目标机端口号  

	SysVars()->SrvConnStaus=SCS_OFFLINE;
	SysVars()->SrvConnPoint=NULL;

	//OS_TaskDelayMs(3000); 	  

	//通过DNS找寻服务器ip
  	{
		char *pURL=Q_Malloc(NORMAL_STR_BYTES);
		char *pPort=NULL;
		
		QDB_GetStr(SDN_SYS,SIN_ServerURL,pURL);//读取服务器地址
		if(NeedDebug(DFT_SRV)) Debug("Find Server:%s\n\r",pURL);
		
		//废弃了读取独立端口号的功能，从url里面分析端口号
		pPort=strchr(pURL,':');//查找端口号
		if(pPort){*pPort++=0;ServerPort=Str2Uint(pPort);}
		if(ServerPort==0) ServerPort=8500;
		
		Error=netconn_gethostbyname((void *)pURL,(void *)&ServerIp);
		switch(Error)
		{
			case ERR_OK://无需寻找
				MemCpy(SysVars()->ServerIp,&ServerIp,sizeof(SysVars()->ServerIp));
				if(NeedDebug(DFT_SRV)) Debug("Server Ip:%u.%u.%u.%u\n\r",DipIpAddr(ServerIp.addr));
				SysVars()->SrvConnStaus=SCS_FOUND_SRV;
				break;			

			case ERR_VAL://dns服务器返回有误
			case ERR_ARG://dns client错误，域名有误
			case ERR_MEM://内存错误
			default:
				Debug("Tcp dns Error!%d\n\r",Error);
				goto Restart;
		}
		Q_Free(pURL);
	}
  	
    SrvConn=netconn_new(NETCONN_TCP);      /* 创建TCP连接  */  
    netconn_bind(SrvConn,IP_ADDR_ANY,10000+Rand(0x1ff));           /* 绑定本地地址和监听的端口号 */   

	SrvConnTask_WaitRecv(SrvConn,&ServerIp,ServerPort);//正常循环收发包
	
	//删除资源
	if(NeedDebug(DFT_SRV)) Debug("SrvNeedDel %x %x\n\r",SrvConn,SrvConn->pcb.tcp);
	netconn_close(SrvConn);
	netconn_delete(SrvConn);

	SysVars()->SrvConnStaus=SCS_OFFLINE;
	SysVars()->SrvConnPoint=NULL;

Restart:
	OS_TaskDelayMs(SRV_FAILD_RETRY_MS); //延时等待重试	
	if(SysVars()->SrvSecretKey) sys_thread_new("SrvConn   ",SrvConnTask,NULL,512,TASK_TCP_CLIENT_PRIO);//重新建立线程，如果由于登陆回包不正确，则SysVars()->SrvSecretKey会被板卡设置为0，则停止连接
	OS_TaskDelete(NULL);//删除线程自己    
}  

//初始化所有tcp连接客户端
void TcpClient_Init(void)
{
	static bool OnlyFlag=TRUE;

	if(OnlyFlag)
	{
		sys_thread_new("SrvConn   ",SrvConnTask,NULL,512,TASK_TCP_CLIENT_PRIO);
		OnlyFlag=FALSE;
	}	
}

//删除链接
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


