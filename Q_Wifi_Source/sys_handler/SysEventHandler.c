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

	gQueueSysEvent=OS_QueueCreate(32,sizeof(SYS_EVENT_ITEM));//��һ������ָ��������ȣ��ڶ�������ָ����Ա��С

	while(1)
	{
		//MemSet(&gEventItemRecv,0,sizeof(gEventItemRecv));
		gEventItemRecv.Event=0;
		OS_QueueReceive(gQueueSysEvent,&gEventItemRecv,OS_MAX_DELAY);
		//Debug("Evt:%u(%u ,0x%x) @%u\n\r",gEventItemRecv.Event,gEventItemRecv.Arg1,(u32)gEventItemRecv.pArg2,OS_GetNowMs());

		switch(gEventItemRecv.Event)
		{
			case SEN_SYS_CMD://������ָ��
				//Debug("CmdIn\n\r");
				SysCmdHandler(gEventItemRecv.Arg1,gEventItemRecv.pArg2,NULL);
				break;
			case SEN_SEC:// �����¼�
				{
					//u32 Now=OS_GetNowMs();
					//if(gEventItemRecv.Arg1!=Now) Debug("Rtc %u %u\n\r",gEventItemRecv.Arg1,Now);
					TrigIn_VarPoll();
					SysVarPeriodicitySet(TRUE);//����ϵͳ����
				}
				break;
			case SEN_32_SEC://�����¼�
				{
					//Debug("32 Sec!@%u %u\n\r",OS_GetNowMs(),RtcGetCnt());
					AppClientOnlinePoll();//����client
					SysVarPeriodicitySet(FALSE);//����ϵͳ����
				}
				break;
				
			case SEN_SRV_BEAT:
				if(SysVars()->SrvConnPoint)
				{
					SrvConnSendBeat(FALSE);
					SysEventMsSend(SRV_CONN_BEAT_S*1000,SEN_SRV_BEAT,0,NULL,MFM_COVER);//��һ��Beat��ʱ
				}
				break;
				
			case SEN_OVER_CLIENT://�ߵ�app����
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

				    PublicEvent(PET_APP_LOST,gEventItemRecv.Arg1,NULL);//ɾ������
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
			case SEN_SET_AP://esp8266ר��
				{
					struct softap_config *ap_config=gEventItemRecv.pArg2; // initialization
					//Debug("set ap %s:%s\n\r",ap_config->ssid,ap_config->password);
					wifi_softap_set_config(ap_config); // Set ESP8266 soft-AP ap_config
					Q_Free(ap_config);
				}	
				break;

			case SEN_QCK_SAVE://esp8266ר��
				QDB_BurnToSpiFlash(SDN_QCK);
				break;

			case SEN_OLED_DISP://esp8266ר��
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

//�����¼�����sys event�̸߳����¼���update����app
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

//����esp sdk�еĶ�ʱ��ʵ�ֵ���ʱ�¼�����
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

//��ʱִ���¼�case
//Mode=0������AddMsFunc��ֱ������
//Mode=1������AddMsFuncCover�����������Ǿɵģ��ɵ�Ҳ���ж����ȫ��ɾ����ֻ�������Ĳ���Ч��
//Mode=2������AddMsFuncNearest�����������ҳ�ʱ������ģ��ɵ�Ҳ���ж����ͬ������һ��Աȣ�ȡʱ�������Ϊ��Ч������
void SysEventMsSend(u32 Ms,u32 Event,u32 Arg1,void *pArg2,MS_FUNC_MODE Mode)
{
	SYS_EVENT_ITEM *pItem=NULL;
	u32 i,n;
	
	if(gQueueSysEvent==NULL) return;

	for(i=0;i<SYS_EVT_MS_RCD_NUM;i++)//�ҿ�λ
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
					if(pRcd!=NULL && pRcd->Event==Event)//ɾ�����оɵ�
					{
						if(n==i) continue;
						if(pRcd->pTimer==NULL) Debug("SysEvtMsRcd Error!\n\r");
						OS_TimerDeinit(pRcd->pTimer);
						Q_Free(pRcd->pTimer);
						Q_Free(pRcd);
						gpSysEvtMsRcd[n]=NULL;
					}
				}//����break��ֱ�������µ�
			case MFM_ALWAYS:
Always:	
				pItem->pTimer=Q_Malloc(sizeof(OS_TIMER_T));
				OS_TimerSetCallback(pItem->pTimer,(OS_TIMER_FUNC_T *)SysEventMsSend_Cb,(void *)i);
				OS_TimerInit(pItem->pTimer,Ms,FALSE);
				pItem->ExpireTime=OS_GetNowMs()+Ms;//��¼����ʱ��
				break;
			case MFM_NEAREST:
				{
					u32 Now=OS_GetNowMs();
					u32 Expire=Now+Ms;

					for(n=0;n<SYS_EVT_MS_RCD_NUM;n++)
					{
						SYS_EVENT_ITEM *pRcd=gpSysEvtMsRcd[n];
						if(pRcd!=NULL && pRcd->Event==Event)//ɾ�����оɵ�
						{
							if(n==i) continue;
							if(pRcd->pTimer==NULL) Debug("SysEvtMsRcd Error2!\n\r");
							if(pRcd->ExpireTime<=Now) continue;//�����ɵ��Ѿ����ڵģ����������ڴ��������������
							if(pRcd->ExpireTime<Expire) Expire=pRcd->ExpireTime;//��¼���絽�ڵ�ʱ��
							OS_TimerDeinit(pRcd->pTimer);
							Q_Free(pRcd->pTimer);
							Q_Free(pRcd);
							gpSysEvtMsRcd[n]=NULL;
						}
					}

					Ms=Expire-Now;//Expireһ������Ms�������ʣʱ��
					goto Always;//����break��ֱ�������µ�
				}
			default:
				Debug("SysEvtMs enum error!\n\r");
				while(1);
		}	
	}
}


//ɾ�����¼����ж�ʱ
void SysEventMsDelete(u32 Event)
{
	u32 n;
	
	for(n=0;n<SYS_EVT_MS_RCD_NUM;n++)
	{
		SYS_EVENT_ITEM *pRcd=gpSysEvtMsRcd[n];
		if(pRcd!=NULL && pRcd->Event==Event)//ɾ������
		{
			if(pRcd->pTimer==NULL) Debug("SysEvtMsRcd Error3!\n\r");
			OS_TimerDeinit(pRcd->pTimer);
			Q_Free(pRcd->pTimer);
			Q_Free(pRcd);
			gpSysEvtMsRcd[n]=NULL;
		}
	}
}


