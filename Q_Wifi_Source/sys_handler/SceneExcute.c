#include "SysDefines.h"
#include "SceneExcute.h"

//执行按键动作
static SCENE_EXC_RES SceneKeyExc(EXC_KEY_ITEM *pKey)
{
	DEVICE_RECORD *pDev=Q_Zalloc(sizeof(DEVICE_RECORD));
	SCENE_EXC_RES ExcRes=SER_NULL;

	Debug("Dev %u, Key %u\n\r",pKey->DevID,pKey->Key);
	
	UpdateIoStateByBtn(pKey->DevID,pKey->Key);
	UserAppButtonHook(pKey->DevID,pKey->Key);	
	TrigIn_AppKey(pKey->DevID,pKey->Key);

	Q_Free(pDev);

	return ExcRes==SER_NULL?SER_OK:ExcRes;
}

//执行变量动作
static SCENE_EXC_RES SceneVarExc(EXC_VAR_ITEM *pVar)
{
	u32 Mask=pVar->CalcMask;
	TVAR32 CalcVal=pVar->CalcVal;
	TVAR32 New=0;
	TVAR32 Val=0;

	if(GetVarState(pVar->Vid,&Val)!=VST_VALID)
	{
		Debug("[%u]VAR NOT VALID\n\r",pVar->Vid);
		return SER_FAILD;
	}
		
	if(pVar->CalcMethod & CMT_VAR_CALC_BIT) //变量与变量进行运算
	{
		if(GetVarState(CalcVal,&CalcVal)!=VST_VALID)
		{
			Debug("[%u]CALC_VAR NOT VALID\n\r",CalcVal);
			return SER_FAILD;
		}
	}	

	if(Mask==0xffff) Mask=0xffffffff;//兼容大变量
	
	switch(pVar->CalcMethod) //运算只影响mask为1的bit
	{
		case CMT_SET:
		case CMT_SET_VAR:
			New=(Val & ~Mask) | (CalcVal  & Mask);
			break;
		case CMT_INC:
		case CMT_INC_VAR:
			New=(Val & ~Mask) | (((Val & Mask) + (CalcVal  & Mask)) & Mask);
			break;
		case CMT_DEC:
		case CMT_DEC_VAR:
			New=(Val & ~Mask) | (((Val & Mask) - (CalcVal  & Mask)) & Mask);
			break;
		case CMT_AND:
			New=Val & ~Mask;
			break;
		case CMT_AND_VAR:
			New=Val & ~CalcVal;
			break;
		case CMT_OR:
			New=Val | Mask;
			break;
		case CMT_OR_VAR:
			New=Val | CalcVal;
			break;
		case CMT_REV:
			New=Val ^ Mask;
			break;
		case CMT_REV_VAR:
			New=Val ^ CalcVal;
			break;
			
		default:
			return SER_FAILD;
	}

	Debug("[%u]=%d\n\r",pVar->Vid,New);
	SetVarVal(pVar->Vid,New,VRT_APP,0);
	
	return SER_OK;
}

//执行字符串动作
static SCENE_EXC_RES SceneMsgExc(EXC_STR_ITEM *pMsg)
{
	SCENE_EXC_RES ExcRes=SER_NULL;
	
	if(pMsg->StrID)
	{
		u8 *pUser=NULL;
		u8 *pStr=Q_Zalloc(STR_RECORD_BYTES);
		u8 StrLen=GetStrRecordData(pMsg->StrID,pStr);
		u8 UserNameLen=0;
		u8 MsgFlag=0;

		if(pMsg->UserID)//单个用户
		{
			pUser=Q_Zalloc(STR_RECORD_BYTES);
			UserNameLen=GetStrRecordData(pMsg->UserID,pUser);		
		}
		else//所有用户
		{
			pUser=Q_Zalloc(4);
			pUser[0]='0';
			pUser[1]=0;
			UserNameLen=1;
		}
	
		if(StrLen && UserNameLen)//有内容
		{
			if(pMsg->Sys) SetBit(MsgFlag,SMF_SYS);
			if(pMsg->DevStr) SetBit(MsgFlag,SMF_DEV_STR);
			if(pMsg->GsmMsg) SetBit(MsgFlag,SMF_GSM);
			if(pMsg->PushMsg) SetBit(MsgFlag,SMF_PUSH);
			
			Debug("%s:%s\n\r",pUser,pStr);
			JsonConnSendMsg(pStr,pUser,MsgFlag);//发送给json服务器
			SrvConnSendMsg(pStr,pUser,MsgFlag);//发送给设备服务器
			ExcRes=SER_OK;
			
			if(pMsg->DevStr) //发送给设备
				ExcRes=SendSysMsgToDev(0,pStr)?SER_OK:SER_FAILD;
		}
		else
		{
			ExcRes=SER_FAILD;
		}
		
		Q_Free(pUser);			
		Q_Free(pStr);		
	}

	return ExcRes;
}

//执行情景的任务
static void SceneExcuteTask(void *pvParameters)
{
	u16 i;
	SCENE_EXCUTE_INFO *pInfo=pvParameters;
	SCENE_ITEM *pItem=pInfo->Items;
	bool FaildFlag=FALSE;

	Debug("---------------- Scene Excute ----------------\n\r");
	
	for(i=0;i<pInfo->ItemNum;i++,pItem++)
	{
		Debug(" #%u-[%2u]%s:",OS_GetNowMs(),i+1,gNameSceneItemAct[pItem->Act]);
		switch(pItem->Act)
		{
			case SIA_KEY:
				if(SER_OK != SceneKeyExc(&pItem->Exc.Key)) FaildFlag=TRUE;
				break;
			case SIA_VAR_SET:
				if(SER_OK != SceneVarExc(&pItem->Exc.Var)) FaildFlag=TRUE;
				break;
			case SIA_SYS_MSG:
				if(SER_OK != SceneMsgExc(&pItem->Exc.Str)) FaildFlag=TRUE;
				break;
			case SIA_SCENE_EXC:
				{
					SCENE_RECORD *pScn=Q_Zalloc(sizeof(SCENE_RECORD));

					if(ReadInfoByID(IFT_SCENE,pItem->Exc.SceneID,pScn))
					{
						Debug("%u\n\r",pItem->Exc.SceneID);
						CallSceneExcute(0,pScn->Items,0);//永远返回正确
					}
					else //未找到
					{
						Debug("Not Found %u\n\r",pItem->Exc.SceneID);
						FaildFlag=TRUE;
					}					
					Q_Free(pScn);
				}
				break;
		}
		
		if(pItem->DelaySec) OS_TaskDelayMs(pItem->DelaySec*1000);
	}

	if(FaildFlag) *(pInfo->pWaitFlag)=SER_FAILD;
	else *(pInfo->pWaitFlag)=SER_OK;

	Debug("---------------- Scene Finish ----------------\n\r");
	
	Q_Free(pInfo);
	OS_TaskDelete(NULL);//删除线程自己    
}

//执行情景的动作列表
//当timeout=0时，非阻塞
SCENE_EXC_RES CallSceneExcute(u8 ItemNum,SCENE_ITEM *pItems,u32 TimeoutMs)
{
	SCENE_EXCUTE_INFO *pInfo=NULL;
	SCENE_EXC_RES ExcRes=SER_NULL;
	
	if(pItems==NULL) return SER_FAILD;

	if(ItemNum==0) //计算个数
	{
		while(pItems[ItemNum].Act!=SIA_NULL)
		{
			ItemNum++;		
			if(ItemNum >= SCENE_MAX_ITEM_NUM) break;
		}
	}

	pInfo=Q_ZallocAsyn(sizeof(SCENE_EXCUTE_INFO)+(ItemNum-1)*sizeof(SCENE_ITEM));

	pInfo->ItemNum=ItemNum;
	pInfo->pWaitFlag=&ExcRes;
	MemCpy(pInfo->Items,pItems,ItemNum*sizeof(SCENE_ITEM));
	
	OS_TaskCreate(SceneExcuteTask,"ScnExe Task",TASK_STK_SIZE_MIN,pInfo,TASK_SCN_EXE_PRIO,NULL);

	if(TimeoutMs==0) return SER_OK;
	
	while(TimeoutMs)
	{
		OS_TaskDelayMs(50);
		if(ExcRes!=SER_NULL) break;
		if(TimeoutMs<=50) break;
		TimeoutMs-=50;		
	}

	return ExcRes;
}

//执行单个情景模式
SCENE_EXC_RES CallSceneExcuteOne(ID_T ScnID,u32 TimeoutMs)
{
	SCENE_EXC_RES Res;
	SCENE_ITEM *pItem=Q_Zalloc(sizeof(SCENE_ITEM));
	
	pItem->Act=SIA_SCENE_EXC;
	pItem->DelaySec=0;
	pItem->Exc.SceneID=ScnID;

	Res=CallSceneExcute(1,pItem,TimeoutMs);//永远返回正确
	Q_Free(pItem);

	return Res;
}































