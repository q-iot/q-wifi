#include "SysDefines.h"
#include "TrigManger.h"
#include "SceneExcute.h"

typedef enum{
	VTGS_NULL=0,//尚未触发
	VTGS_VAILD,//已经有效，等待检测期满
	VTGS_WAITTING,//检测期满，但是状态变量不对，等待状态变量变成对
	VTGS_FINISH,//完成了触发
	
}VAR_TRIG_STAUS;

typedef struct{
	ID_T TrigID;//释放时，此值需归零，防止额外触发
	ID_T SceneID;
	TRIGGER_TYPE TrigType;//触发源类型
	bool Active:1;//设防	
	
	VAR_TRIG_STAUS VarTrigStaus:3;//变量触发状态
	u32 VarTrigExpires;//在此时间之前，变量必须符合条件
	
	union{
		KEY_TRIGGER KeyVal;
		RF_TRIGGER Rf;
		SYS_MSG_TRIGGER SysMsg;
		VAR_TRIGGER VarTrigger;
	}Oppo;//时机	

	VAR_TRIGGER *pVarState;//附加条件

	void *pNext;
}TRIG_LIST_ITEM;
static TRIG_LIST_ITEM *gpTrigList=NULL;
static TRIG_LIST_ITEM *gpVarTrigNearest=NULL;
static u32 gVarTrigNearest_ID=0;

static OS_SEM_T gTrigListMutex=NULL;
#define TrigListMutexNull() (gTrigListMutex==NULL?TRUE:FALSE)
#define TrigListMutexLock() OS_SemTake(gTrigListMutex,OS_MAX_DELAY)
#define TrigListMutexUnlock() OS_SemGive(gTrigListMutex)

static void TrigAdd(TRIGGER_RECORD *pRcd);

void TrigListDebug(void)
{
	TRIG_LIST_ITEM *p=NULL;
	u16 i;

	if(TrigListMutexNull()) return;
	TrigListMutexLock();
	p=gpTrigList;
	while(p)
	{
		Debug("[%u]",p->TrigID);
		DebugCol("%s",gNameTriggerType[p->TrigType]);
		switch(p->TrigType)
		{
			case TT_IN_RF:Debug("Rf%x@%uuS",p->Oppo.Rf.Code,p->Oppo.Rf.BasePeriod);break;
			case TT_APP_KEY:Debug("Dev%u Key%u",p->Oppo.KeyVal.DevID,p->Oppo.KeyVal.Key);break;
			case TT_SYS_MSG:Debug("Str%u Hash%u",p->Oppo.SysMsg.StrID,CalcStrRecordHash(p->Oppo.SysMsg.StrID));break;
			case TT_VAR_SET:
				Debug("Var%u,%s,&%x,AB:%d-%d,",p->Oppo.VarTrigger.Vid,p->Oppo.VarTrigger.Method==VCM_IN?"IN":"OUT",p->Oppo.VarTrigger.Mask,p->Oppo.VarTrigger.CalcA,p->Oppo.VarTrigger.CalcB);
				Debug("Time:%u. State%u,Expires%u,Now%u",p->Oppo.VarTrigger.TimeSec,p->VarTrigStaus,p->VarTrigExpires,RtcGetCnt());
				break;
			default:Debug("Unknow(%u)",p->TrigType);
		}
		
		Debug(" -> Scn:%u%s\n\r",p->SceneID,p->Active?" *":"");	

		if(p->pVarState)
		{
			VAR_TRIGGER *pV=p->pVarState;
			Debug("  |CheckVar:%u, Method:%u, AB:%d-%d & %x\n\r",pV->Vid,pV->Method,pV->CalcA,pV->CalcB,pV->Mask);
		}

		
		
		p=p->pNext;
	}
	TrigListMutexUnlock();	
}

#if 1
//检查变量状态
static bool JudgeVarState(VAR_TRIGGER *pVarState)
{
	VAR_TRIGGER *pCheck=NULL;
	bool Ret=FALSE;
	
	if(pVarState==NULL) return FALSE;

#if 0
	if(TrigID)//读取trig的var state
	{
		TRIGGER_RECORD *pRcd=Q_Zalloc(sizeof(TRIGGER_RECORD));
		u16 Bytes;

		pVarState=pCheck=Q_Zalloc(sizeof(VAR_TRIGGER));
		
		Bytes=ReadInfoByID(IFT_TRIGGER,TrigID,pRcd);
		if(Bytes && pRcd->ID && pRcd->VarState.Vid)
		{
			MemCpy(pVarState,&pRcd->VarState,sizeof(VAR_TRIGGER));
		}
		
		Q_Free(pRcd);
	}
#endif

	if(pVarState)
	{
		TVAR32 Val=0;
		VAR_STATE VarState;
		VAR_DISP_FAT Fat;

		if(pVarState->Vid==0) 
		{
			Ret=TRUE;
			goto End;
		}

		VarState=GetVarState(pVarState->Vid,&Val);
		Fat=GetVarDispFat(pVarState->Vid,NULL);
		
		if(VarState!=VST_VALID)//变量不可取
		{
			Ret=FALSE;
			goto End;
		}
		
		switch(pVarState->Method)//判断变量范围
		{
			case VCM_CHANGED:
			case VCM_IN:			
				if(Fat==VDF_S16)
				{				
					if(Val>=pVarState->CalcA && Val<=pVarState->CalcB) Ret=TRUE;
					else Ret=FALSE;
				}
				else
				{
					u16 A=pVarState->CalcA;
					u16 B=pVarState->CalcB;
					u16 Valu=Val;

					if(Valu>=A && Valu<=B) Ret=TRUE;
					else Ret=FALSE;				
				}			
				break;
			case VCM_OUT:
				if(Fat==VDF_S16)
				{				
					if(Val>=pVarState->CalcA && Val<=pVarState->CalcB) Ret=FALSE;
					else Ret=TRUE;
				}
				else
				{
					u16 A=pVarState->CalcA;
					u16 B=pVarState->CalcB;
					u16 Valu=Val;

					if(Valu>=A && Valu<=B) Ret=FALSE;
					else Ret=TRUE;				
				}			
				break;
		}		
	}

End:
	if(pCheck) Q_Free(pCheck);
	return Ret;
}

//更新计时列表
static void UpdateVarTrigTimer(TRIG_LIST_ITEM *pItem)
{
	u32 TrigID=pItem->TrigID;
	u32 TimSec=pItem->Oppo.VarTrigger.TimeSec;
	bool Change=FALSE;

	//找出最近的
	if(gpVarTrigNearest==NULL)
	{
		gpVarTrigNearest=pItem;
		gVarTrigNearest_ID=TrigID;
		Change=TRUE;
	}
	else
	{
		if(gpVarTrigNearest->TrigID==gVarTrigNearest_ID)
		{
			if((RtcGetCnt()+TimSec) < gpVarTrigNearest->VarTrigExpires)
			{
				gpVarTrigNearest=(void *)pItem;
				gVarTrigNearest_ID=TrigID;
				Change=TRUE;			
			}
		}
		else
		{
			Debug("trig tim error %u %u\n\r",gpVarTrigNearest->TrigID,gVarTrigNearest_ID);
			while(1);
		}
	}

	if(Change) //最近的改变了
	{

	}	
}

//检查变量状态，并执行情景
static void CheckAndExcuteScene(TRIG_LIST_ITEM *pItem)
{
	if(pItem->pVarState==NULL || JudgeVarState(pItem->pVarState)==TRUE)//检查变量状态
	{
		CallSceneExcuteOne(pItem->SceneID,0);
		if(pItem->Oppo.VarTrigger.Method==VCM_CHANGED) pItem->VarTrigStaus=VTGS_NULL;//为了下次变化就触发，还原标志
		else pItem->VarTrigStaus=VTGS_FINISH;
		pItem->VarTrigExpires=0;
	}
}

//注 gpTrigList不能为null
static TRIG_LIST_ITEM *TrigListLast_(void)
{
	TRIG_LIST_ITEM *pLast=gpTrigList;
	while(pLast->pNext) pLast=pLast->pNext;
	return pLast;
}

//增加一个设备到列表中
static void TrigNewOne(TRIG_LIST_ITEM *pNew)
{
	pNew->pNext=NULL;

	if(TrigListMutexNull()) return;
	TrigListMutexLock();
	if(gpTrigList==NULL)
	{
		gpTrigList=(void *)pNew;
	}
	else
	{
		TrigListLast_()->pNext=pNew;
	}
	TrigListMutexUnlock();	
}

//变量触发的注册函数
static void VarTrigStart(TRIG_LIST_ITEM *p)
{
	if(p->TrigType==TT_VAR_SET && p->Active)//变量触发，要绑定变量
	{
		NotifyAdd_Var2Trig(p->Oppo.VarTrigger.Vid,(u32)p,p->TrigID);//绑定时机变量
		if(p->pVarState) NotifyAdd_Var2TrigSta(p->pVarState->Vid,(u32)p,p->TrigID);//绑定状态变量
		
		if(JudgeVarState(&(p->Oppo.VarTrigger))==TRUE)//检查当前是否已经符合触发条件
		{
			if(p->Oppo.VarTrigger.TimeSec)//有时间参数
			{
				p->VarTrigStaus=VTGS_VAILD;
				p->VarTrigExpires=RtcGetCnt()+p->Oppo.VarTrigger.TimeSec;
				UpdateVarTrigTimer(p);
			}
			else//无时间参数，直接执行
			{
				p->VarTrigStaus=VTGS_WAITTING;
				p->VarTrigExpires=0;
				CheckAndExcuteScene(p);
			}
		}
		else//不符合条件
		{
			p->VarTrigStaus=VTGS_NULL;
			p->VarTrigExpires=0;
		}
	}			
}

//变量触发的注销函数
static void VarTrigEnd(TRIG_LIST_ITEM *p)
{
	if(p->TrigType==TT_VAR_SET)
	{
		NotifyDel_Var2Trig(p->Oppo.VarTrigger.Vid,(u32)p);
		if(p->pVarState) NotifyDel_Var2TrigSta(p->pVarState->Vid,(u32)p);
		
		p->VarTrigStaus=VTGS_NULL;
		p->VarTrigExpires=0;
	}
}
#endif

//初始化触发源
//可重复调用
void TrigListInit(void)
{
	if(gTrigListMutex==NULL) gTrigListMutex=OS_SemCreateMutex();
	
	//清空旧数据
	TrigListMutexLock();
	gpVarTrigNearest=NULL;
	gVarTrigNearest_ID=0;
	while(gpTrigList)
	{
		TRIG_LIST_ITEM *pTrigNext=gpTrigList->pNext;
		VarTrigEnd(gpTrigList);
		if(gpTrigList->pVarState) Q_Free(gpTrigList->pVarState);
		MemSet((void *)gpTrigList,0,sizeof(TRIG_LIST_ITEM));//p->TrigID=0;//防止额外触发
		Q_Free((void *)gpTrigList);
		gpTrigList=pTrigNext;
	}
	TrigListMutexUnlock();

  	ScnDefendListTidy();//删除冗余的设防状态记录

	//从数据库读取信息，并添加和使能触发源
	{
		TRIGGER_RECORD *pRcd=Q_Zalloc(sizeof(TRIGGER_RECORD));
		u16 i,InfoNum=GetTypeInfoTotal(IFT_TRIGGER);
		u16 Res;
		
		for(i=1;i<=InfoNum;i++)
		{
			pRcd->ID=0;
			Res=ReadInfoByIdx(IFT_TRIGGER,i,pRcd);
			if(Res && pRcd->ID) 
			{
				TrigAdd(pRcd);
			}
		}

		Q_Free(pRcd);
	}
}

//激活触发源
void TrigEnable_ByScn(ID_T SceneID)
{
	TRIG_LIST_ITEM *p=NULL;

	if(TrigListMutexNull()) return;
	TrigListMutexLock();
	p=gpTrigList;
	while(p)
	{
		if(p->SceneID==SceneID)
		{
			p->Active=TRUE;
			VarTrigStart(p);
		}
		
		p=p->pNext;
	}
	TrigListMutexUnlock();	
}

//失效触发源
void TrigDisable_ByScn(ID_T SceneID)
{
	TRIG_LIST_ITEM *p=NULL;

	if(TrigListMutexNull()) return;
	TrigListMutexLock();
	p=gpTrigList;
	while(p)
	{
		if(p->SceneID==SceneID)
		{
			p->Active=FALSE;
			VarTrigEnd(p);
		}
		
		p=p->pNext;
	}
	TrigListMutexUnlock();	
}

//随时删除触发源
static void TrigsDelete_ByScn(ID_T ScnID)
{
	TRIG_LIST_ITEM *p=NULL;
	TRIG_LIST_ITEM *pPre=NULL;

	ClrScnDefendToList(&ScnID,1);

	if(TrigListMutexNull()) return;
	TrigListMutexLock();
	p=gpTrigList;
	while(p)
	{
		if(p->SceneID==ScnID)
		{
			if(pPre!=NULL) pPre->pNext=p->pNext;
			if(p==gpTrigList) gpTrigList=p->pNext;
			
			Debug("-Trig:%u\n\r",p->TrigID);
			VarTrigEnd(p);
			if(p->pVarState) Q_Free(p->pVarState);
			MemSet(p,0,sizeof(TRIG_LIST_ITEM));//p->TrigID=0;//防止额外触发
			Q_Free(p);

			p=pPre?pPre->pNext:gpTrigList;
			continue;
		}

		pPre=p;
		p=p->pNext;
	}
	TrigListMutexUnlock();
}




//------------------------------------------触发接口-------------------------------------------------
//设备键值触发
void TrigIn_AppKey(u32 DevId,u32 Key)
{
	TRIG_LIST_ITEM *p=NULL;

	if(DevId==0 || Key==0) return;

	if(TrigListMutexNull()) return;
	TrigListMutexLock();
	p=gpTrigList;
	while(p)
	{
		if(p->Active && p->TrigType==TT_APP_KEY && p->Oppo.KeyVal.DevID==DevId && p->Oppo.KeyVal.Key==Key)
		{
			CheckAndExcuteScene(p);
		}
		
		p=p->pNext;
	}
	TrigListMutexUnlock();		
}

//射频触发
void TrigIn_Rf(u32 Code,u16 BasePeriod)
{
	TRIG_LIST_ITEM *p=NULL;

	if(Code==0 || BasePeriod==0) return;

	if(TrigListMutexNull()) return;
	TrigListMutexLock();
	p=gpTrigList;
	while(p)
	{
		if(p->Active && p->TrigType==TT_IN_RF && p->Oppo.Rf.Code==Code && FuzzyEqual(p->Oppo.Rf.BasePeriod,BasePeriod,10))
		{
			CheckAndExcuteScene(p);
		}
		
		p=p->pNext;
	}
	TrigListMutexUnlock();		
}

//pMsg中不再包含系统分隔符冒号
//匹配以.终止
void TrigIn_SysMsg(const char *pMsg)
{
	u32 Hash=0;	
	char *p=NULL;

	if(pMsg==NULL) return;
	if(TrigListMutexNull()) return;
	
	Debug("SysMsgIn:%s\n\r",pMsg);

	Fetch2VarFormString(pMsg);//从系统消息中提取变量值

	p=strchr(pMsg,'.');//匹配结束符
	if(p) Hash=CalcStrHash(pMsg,p-pMsg);
	else Hash=CalcStrHash(pMsg,0);
	
	if(Hash)
	{
		TRIG_LIST_ITEM *p=NULL;

		TrigListMutexLock();
		p=gpTrigList;
		while(p)
		{
			if(p->Active && p->TrigType==TT_SYS_MSG && p->Oppo.SysMsg.Hash==Hash)
			{
				CheckAndExcuteScene(p);
			}
			
			p=p->pNext;
		}
		TrigListMutexUnlock();	
	}	
}

//定时调用的，用来确定变量状态符合到期的
// 1秒调用一次
void TrigIn_VarPoll(void)
{
	if(gpVarTrigNearest!=NULL && gpVarTrigNearest->TrigID==gVarTrigNearest_ID)
	{
		u32 Now=RtcGetCnt();
		if(Now >= gpVarTrigNearest->VarTrigExpires)//到期了,检查所有的
		{
			TRIG_LIST_ITEM *p=NULL;
			u32 NearestRtc=0xffffffff;

			TrigListMutexLock();
			p=gpTrigList;
			while(p)
			{
				if(p->Active && p->TrigType==TT_VAR_SET && p->VarTrigStaus==VTGS_VAILD && p->VarTrigExpires)//只检查变量触发有效的
				{
					if(Now >= p->VarTrigExpires)//到期了的
					{
						p->VarTrigStaus=VTGS_WAITTING;
						p->VarTrigExpires=0;						
						CheckAndExcuteScene(p);
					}
					else//尚未到期的
					{
						if(p->VarTrigExpires<=NearestRtc)
						{
							NearestRtc=p->VarTrigExpires;
							gpVarTrigNearest=p;
							gVarTrigNearest_ID=p->TrigID;
						}
					}
				}
				
				p=p->pNext;
			}
			TrigListMutexUnlock();	
		}
	}
}

//变量改变了
void TrigIn_Var(u16 TrigID,void *p,u16 Vid)
{
	TRIG_LIST_ITEM *pItem=p;
	bool NowTrigJudge=FALSE;
	u32 TimeSec=0; 
	VAR_TRIGGER *pVarTrigger;

	if(TrigListMutexNull()) return;
	TrigListMutexLock();
	
	if(TrigID==0 || pItem==NULL) goto End;	
	if(LBit16(pItem->TrigID)!=TrigID) goto End;
	if(pItem->Active == FALSE) goto End;
	if(pItem->TrigType!=TT_VAR_SET) goto End;
	if(pItem->Oppo.VarTrigger.Vid!=Vid) goto End;	
	
	pVarTrigger=&(pItem->Oppo.VarTrigger);
	TimeSec=pVarTrigger->TimeSec;
	NowTrigJudge=JudgeVarState(pVarTrigger);//检查当前变量是否符合触发条件
	switch(pItem->VarTrigStaus)
	{
		case VTGS_NULL:
			if(NowTrigJudge)//条件符合
			{
				if(TimeSec==0) //不需要时长，直接触发
				{
					pItem->VarTrigStaus=VTGS_WAITTING;
					pItem->VarTrigExpires=0;
					CheckAndExcuteScene(pItem);
				}
				else//需要时长，先设定好期限
				{
					pItem->VarTrigStaus=VTGS_VAILD;
					pItem->VarTrigExpires=RtcGetCnt()+TimeSec;
					UpdateVarTrigTimer(pItem);
				}
			}
			break;
		case VTGS_VAILD:
			if(NowTrigJudge)//条件符合
			{
				if(RtcGetCnt() >= pItem->VarTrigExpires)//时长已经满足
				{
					pItem->VarTrigStaus=VTGS_WAITTING;
					pItem->VarTrigExpires=0;
					CheckAndExcuteScene(pItem);
				}
			}
			else//条件符合失效
			{
				pItem->VarTrigStaus=VTGS_NULL;
				pItem->VarTrigExpires=0;
			}
			break;
		case VTGS_WAITTING:
			if(NowTrigJudge==FALSE) //条件符合失效，重新计算
			{
				pItem->VarTrigStaus=VTGS_NULL;
				pItem->VarTrigExpires=0;
			}
			break;
		case VTGS_FINISH:
			if(NowTrigJudge==FALSE) //条件符合失效，重新计算
			{
				pItem->VarTrigStaus=VTGS_NULL;
				pItem->VarTrigExpires=0;
			}
			break;
	}

End:
	TrigListMutexUnlock();
}

//触发源状态检查发生变动
void TrigStateVarChange(u16 TrigID,void *p,u16 Vid)
{	
	TRIG_LIST_ITEM *pItem=p;

	if(TrigListMutexNull()) return;
	TrigListMutexLock();
	
	if(TrigID==0 || pItem==NULL) goto End;	
	if(LBit16(pItem->TrigID)!=TrigID) goto End;
	if(pItem->Active == FALSE) goto End;
	if(pItem->TrigType!=TT_VAR_SET) goto End;

	if(pItem->VarTrigStaus==VTGS_WAITTING)//只处理准备好的触发源
	{
		CheckAndExcuteScene(pItem);
	}	

End:
	TrigListMutexUnlock();
}

#if 1
//删除一个触发源
static void TrigDelete(ID_T TrigID)
{
	TRIG_LIST_ITEM *p=NULL;
	TRIG_LIST_ITEM *pPre=NULL;

	if(TrigID==0) return;

	if(TrigListMutexNull()) return;
	TrigListMutexLock();
	p=gpTrigList;
	while(p)
	{
		if(p->TrigID==TrigID)
		{
			if(pPre!=NULL) pPre->pNext=p->pNext;
			if(p==gpTrigList) gpTrigList=p->pNext;
			
			Debug("-Trig:%u\n\r",p->TrigID);
			VarTrigEnd(p);
			if(p->pVarState) Q_Free(p->pVarState);
			MemSet(p,0,sizeof(TRIG_LIST_ITEM));//p->TrigID=0;//防止额外触发
			Q_Free(p);
			break;
		}
		pPre=p;
		p=p->pNext;
	}
	TrigListMutexUnlock();
}

//随时增加一个触发源
static void TrigAdd(TRIGGER_RECORD *pRcd)
{
	if(pRcd && pRcd->TrigType)
	{
		TRIG_LIST_ITEM *pItem=Q_ZallocAsyn(sizeof(TRIG_LIST_ITEM));
		u16 Res;

		TrigDelete(pRcd->ID);//删除旧的

		pItem->TrigID=pRcd->ID;
		pItem->SceneID=pRcd->SceneID;
		pItem->Active=ScnIsActive(pRcd->SceneID);
		pItem->TrigType=pRcd->TrigType;
		MemCpy(&pItem->Oppo,&pRcd->Oppo,sizeof(pRcd->Oppo));

		if(pRcd->VarState.Vid)
		{
			pItem->pVarState=Q_ZallocAsyn(sizeof(VAR_TRIGGER));
			MemCpy(pItem->pVarState,&pRcd->VarState,sizeof(VAR_TRIGGER));
		}
		else
		{
			pItem->pVarState=NULL;
		}
		
		TrigNewOne(pItem);
		VarTrigStart(pItem);
	}
}

//处理系统事件
void TrigManEventInput(PUBLIC_EVENT Event,u32 Param,void *p)
{
	switch(Event)
	{
		case PET_APP_NEW_TRIG:
			if(p!=NULL)
			{
				TrigAdd(p);
			}
			break;
		case PET_APP_MODIFY_SCENE:
		case PET_APP_DEL_SCENE:
			{
				TrigsDelete_ByScn(Param);
			}
			break;
		case PET_APP_DEL_TRIG:
			{
				TrigDelete(Param);
			}
			break;
	}
}
#endif

