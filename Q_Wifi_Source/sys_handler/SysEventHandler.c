#include "SysDefines.h"
#include "SysEventHandler.h"
#include "VarDisplay.h"

typedef struct{
	SYS_EVENT_NAME Event;

	u32 Arg1;
	void *pArg2;

	OS_TIMER_T *pTimer;
	u32 ExpireTime;
}SYS_EVENT_ITEM;
static OS_QUEUE_T gQueueSysEvent=NULL;

extern bool SysCmdHandler(u16 Len,char *pStr,char *pOutStream);

void SysEventTask(void *pvParameters)
{
	static SYS_EVENT_ITEM gEventItemRecv;

	gQueueSysEvent=OS_QueueCreate(32,sizeof(SYS_EVENT_ITEM));//第一个参数指定队列深度，第二个参数指定成员大小

	while(1)
	{
		//MemSet(&gEventItemRecv,0,sizeof(gEventItemRecv));
		gEventItemRecv.Event=0;
		OS_QueueReceive(gQueueSysEvent,&gEventItemRecv,OS_MAX_DELAY);
		//Debug("Evt:%u(%u ,0x%x) @%u\n\r",gEventItemRecv.Event,gEventItemRecv.Arg1,(u32)gEventItemRecv.pArg2,OS_GetNowMs());

		switch(gEventItemRecv.Event)
		{
			case SEN_SYS_CMD://命令行指令
				//Debug("CmdIn\n\r");
				SysCmdHandler(gEventItemRecv.Arg1,gEventItemRecv.pArg2,NULL);
				break;
			case SEN_SEC:// 单秒事件
				{
					//u32 Now=OS_GetNowMs();
					//if(gEventItemRecv.Arg1!=Now) Debug("Rtc %u %u\n\r",gEventItemRecv.Arg1,Now);
					TrigIn_VarPoll();
					SysVarPeriodicitySet(TRUE);//设置系统变量
				}
				break;
			case SEN_32_SEC://慢速事件
				{
					//Debug("32 Sec!@%u %u\n\r",OS_GetNowMs(),RtcGetCnt());
					AppClientOnlinePoll();//管理client
					SysVarPeriodicitySet(FALSE);//设置系统变量
				}
				break;
				
			case SEN_SRV_BEAT:
				if(SysVars()->SrvConnPoint)
				{
					SrvConnSendBeat(FALSE);
					SysEventMsSend(SRV_CONN_BEAT_S*1000,SEN_SRV_BEAT,0,NULL,MFM_COVER);//下一次Beat定时
				}
				break;
				
			case SEN_OVER_CLIENT://踢掉app链接
				{
					if(GetClientFlag(gEventItemRecv.pArg2) == CLIENT_T_LOCAL)
					{
						PKT_HANDLER_RES HandlerRes;
						UPDATE_PKT *pUpdPkt=Q_Malloc(64);

						//Debug("SEN_OVER_CLIENT AppID:%u, pClient:0x%x\n\r\n\r",gEventItemRecv.Arg1,gEventItemRecv.pArg2);
						pUpdPkt->PktLen=sizeof(UPDATE_PKT);
						pUpdPkt->PktSn=Rand(0xffff);
						pUpdPkt->Type=PT_UPDATE;
						pUpdPkt->Res=GPR_MAIN;
						pUpdPkt->Act=UPA_OVER_CLIENT;
						pUpdPkt->Num=0;
						pUpdPkt->DataLen=0;				

						HandlerRes=BuildUpdatePkt(gEventItemRecv.Arg1,0,pUpdPkt);
						if(HandlerRes==PHR_OK)
						{
							SendToAppConn(gEventItemRecv.pArg2,pUpdPkt,pUpdPkt->PktLen);
					    }
					    Q_Free(pUpdPkt);
				    }

				    PublicEvent(PET_APP_LOST,gEventItemRecv.Arg1,NULL);//删除连接
			    }
				break;
			case SEN_INFORM_TASK:
				InformEventTaskWalk();
				break;
			case SEN_CB_FUNC:
				{
					pStdFunc pCallback=gEventItemRecv.pArg2;
					pCallback(gEventItemRecv.Arg1,NULL);
				}
				break;
			case SEN_SET_AP://esp8266专有
				{
					struct softap_config *ap_config=gEventItemRecv.pArg2; // initialization
					//Debug("set ap %s:%s\n\r",ap_config->ssid,ap_config->password);
					wifi_softap_set_config(ap_config); // Set ESP8266 soft-AP ap_config
					Q_Free(ap_config);
				}	
				break;

			case SEN_QCK_SAVE://esp8266专有
				QDB_BurnToSpiFlash(SDN_QCK);
				break;

			case SEN_OLED_DISP://esp8266专有
				OledDisplayLoop();
				break;
			case SEN_PIN_CHANGE:
				UpdateIoStateByBtn(0,gEventItemRecv.Arg1);
				break;
			case SEN_USER_EVT1:
				UserEvent1Hook(gEventItemRecv.Arg1,gEventItemRecv.pArg2);
				break;
			case SEN_USER_EVT2:
				UserEvent2Hook(gEventItemRecv.Arg1,gEventItemRecv.pArg2);
				break;
			case SEN_USER_EVT3:
				UserEvent3Hook(gEventItemRecv.Arg1,gEventItemRecv.pArg2);
				break;
			case SEN_USER_EVT4:
				UserEvent4Hook(gEventItemRecv.Arg1,gEventItemRecv.pArg2);
				break;
				
			default:
				Debug("Arg:%u, pArg:0x%x\n\r",gEventItemRecv.Arg1,gEventItemRecv.pArg2);
		}
	}
}

//发送事件，让sys event线程根据事件发update包给app
void SysEventSend(SYS_EVENT_NAME Event,u32 Arg1,void *pArg2,bool *pYeild)
{
	SYS_EVENT_ITEM gEventItem;
	OS_BASE_T NeedYield;

	if(gQueueSysEvent==NULL) return;
	
	gEventItem.Event=Event;
	gEventItem.Arg1=Arg1;
	gEventItem.pArg2=pArg2;
	gEventItem.pTimer=NULL;

	if(pYeild==NULL)
	{
		OS_QueueSend(gQueueSysEvent,&gEventItem,0);
	}
	else 
	{
		OS_QueueSend_ISR(gQueueSysEvent,&gEventItem,&NeedYield);
		*pYeild=NeedYield?TRUE:FALSE;
	}	
}

#define SYS_EVT_MS_RCD_NUM 16
static SYS_EVENT_ITEM *gpSysEvtMsRcd[SYS_EVT_MS_RCD_NUM]={
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL,
NULL,NULL,NULL,NULL,NULL,NULL,NULL,NULL};

//利用esp sdk中的定时器实现的延时事件发送
static void SysEventMsSend_Cb(u32 Index)
{
	SYS_EVENT_ITEM *pItem=gpSysEvtMsRcd[Index];
	
	if(gQueueSysEvent!=NULL && pItem!=NULL && pItem->pTimer!=NULL)
	{		
		//Debug("Evt Cb:%u, Now %u\n\r",pItem->Event,RtcGetCnt());
		OS_QueueSend(gQueueSysEvent,pItem,0);
		Q_Free(pItem->pTimer);
		Q_Free(pItem);
		gpSysEvtMsRcd[Index]=NULL;
	}
}

//定时执行事件case
//Mode=0，调用AddMsFunc，直接新增
//Mode=1，调用AddMsFuncCover，新增并覆盖旧的，旧的也许有多个，全部删除，只有新增的才有效。
//Mode=2，调用AddMsFuncNearest，新增，并找出时间最近的，旧的也许有多个，同新增的一起对比，取时间最近的为有效新增。
void SysEventMsSend(u32 Ms,u32 Event,u32 Arg1,void *pArg2,MS_FUNC_MODE Mode)
{
	SYS_EVENT_ITEM *pItem=NULL;
	u32 i,n;
	
	if(gQueueSysEvent==NULL) return;

	for(i=0;i<SYS_EVT_MS_RCD_NUM;i++)//找空位
	{
		if(gpSysEvtMsRcd[i]==NULL)
		{
			break;
		}
	}
	if(i==SYS_EVT_MS_RCD_NUM)
	{
		Debug("SYS_EVT_MS_RCD_NUM too small!\n\r");
		while(1);
	}
	
	gpSysEvtMsRcd[i]=pItem=Q_Malloc(sizeof(SYS_EVENT_ITEM));
	pItem->Event=Event;
	pItem->Arg1=Arg1;
	pItem->pArg2=pArg2;

	if(Ms==0)
	{
		pItem->pTimer=NULL;
		OS_QueueSend(gQueueSysEvent,pItem,0);
		Q_Free(pItem);
		gpSysEvtMsRcd[i]=NULL;
	}
	else
	{
		switch(Mode)
		{
			case MFM_COVER:
				for(n=0;n<SYS_EVT_MS_RCD_NUM;n++)
				{
					SYS_EVENT_ITEM *pRcd=gpSysEvtMsRcd[n];
					if(pRcd!=NULL && pRcd->Event==Event)//删除所有旧的
					{
						if(n==i) continue;
						if(pRcd->pTimer==NULL) Debug("SysEvtMsRcd Error!\n\r");
						OS_TimerDeinit(pRcd->pTimer);
						Q_Free(pRcd->pTimer);
						Q_Free(pRcd);
						gpSysEvtMsRcd[n]=NULL;
					}
				}//无需break，直接新增新的
			case MFM_ALWAYS:
Always:	
				pItem->pTimer=Q_Malloc(sizeof(OS_TIMER_T));
				OS_TimerSetCallback(pItem->pTimer,(OS_TIMER_FUNC_T *)SysEventMsSend_Cb,(void *)i);
				OS_TimerInit(pItem->pTimer,Ms,FALSE);
				pItem->ExpireTime=OS_GetNowMs()+Ms;//记录到期时间
				break;
			case MFM_NEAREST:
				{
					u32 Now=OS_GetNowMs();
					u32 Expire=Now+Ms;

					for(n=0;n<SYS_EVT_MS_RCD_NUM;n++)
					{
						SYS_EVENT_ITEM *pRcd=gpSysEvtMsRcd[n];
						if(pRcd!=NULL && pRcd->Event==Event)//删除所有旧的
						{
							if(n==i) continue;
							if(pRcd->pTimer==NULL) Debug("SysEvtMsRcd Error2!\n\r");
							if(pRcd->ExpireTime<=Now) continue;//误差造成的已经到期的，当作不存在处理，这种情况极少
							if(pRcd->ExpireTime<Expire) Expire=pRcd->ExpireTime;//记录最早到期的时间
							OS_TimerDeinit(pRcd->pTimer);
							Q_Free(pRcd->pTimer);
							Q_Free(pRcd);
							gpSysEvtMsRcd[n]=NULL;
						}
					}

					Ms=Expire-Now;//Expire一定大于Ms，算出所剩时间
					goto Always;//无需break，直接新增新的
				}
			default:
				Debug("SysEvtMs enum error!\n\r");
				while(1);
		}	
	}
}


//删除此事件所有定时
void SysEventMsDelete(u32 Event)
{
	u32 n;
	
	for(n=0;n<SYS_EVT_MS_RCD_NUM;n++)
	{
		SYS_EVENT_ITEM *pRcd=gpSysEvtMsRcd[n];
		if(pRcd!=NULL && pRcd->Event==Event)//删除所有
		{
			if(pRcd->pTimer==NULL) Debug("SysEvtMsRcd Error3!\n\r");
			OS_TimerDeinit(pRcd->pTimer);
			Q_Free(pRcd->pTimer);
			Q_Free(pRcd);
			gpSysEvtMsRcd[n]=NULL;
		}
	}
}


