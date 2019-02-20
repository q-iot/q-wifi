#include "SysDefines.h"
//#include "UserComManger.h"
#include "InfoOperateHooks.h"

typedef struct{
	u16 Vid;
	
	VAR_TYPE Type:3;//变量类型
	bool ReadOnly:1;//app只读
	VAR_STATE VarState:2;//变量状态
	
	u8 SelfIdx;//归属于同一设备的不同变量的对应索引，1起始
	
	VAR_DISP_FAT DispFat:3;//显示格式
	u8 DotNum:5;//显示小数点后的位数

	TVAR32 OldVal;
	TVAR32 VarVal;//当前值

	CHAR_NUM_UNI ProdTag;
	CHAR_NUM_UNI VarTag;
	u32 MainDevID;//主设备id
	u32 MainDevWAddr;//主设备万纳网地址
	
	void *pNext;
}VAR_LIST_ITEM;//变量列表
static VAR_LIST_ITEM *gpVarList=NULL;

typedef struct{
	u16 Vid;//必须定义在首位
	VAR_STATE VarState;
	VAR_OPTOR SetorType;
	TVAR32 Value;
	u32 SetorID;
}VAR_CHANGE_INFO;//变量改变信息

void VarListDebug(void)
{
	VAR_LIST_ITEM *p=gpVarList;
	
	while(p)
	{
		Debug("[%s]",gNameVarType[p->Type]);
		Debug("[%c%c%c%c.%c%c%c%c]",DispVarTagp(p));
		Debug("%s ",gNameVarState[p->VarState]);
		
		switch(p->Type)
		{
			case VAR_T_SRV:
				Debug("\n\r");
			case VAR_T_SYS:
			case VAR_T_USER:
				if(p->DispFat==VDF_S16) Debug("%u=%d(0x%x)\n\r",p->Vid,p->VarVal,p->VarVal);
				else Debug("%u=%u(0x%x)\n\r",p->Vid,(u16)p->VarVal,(u16)p->VarVal);
				break;
			case VAR_T_DEV:	
				if(p->DispFat==VDF_S16) Debug("%u=%d(0x%x), ",p->Vid,p->VarVal,p->VarVal);
				else Debug("%u=%u(0x%x), ",p->Vid,p->VarVal,p->VarVal);
				Debug("Dev[%u]%u\n\r",p->MainDevID,p->SelfIdx);//esp8266没有waddr
				break;
			default:
				Debug("Error Type %u\n\r",p->Type);
		}
		p=p->pNext;
	}
}

//标签改大写
void TagToH(u8 *pTag)
{
	u8 i;
	
	for(i=0;i<VAR_TAG_LEN;i++)//命令字符串全部转大写
	{
		if(pTag[i]>='a' && pTag[i]<='z')
			pTag[i]=pTag[i]-32;
	}
}

//标签算数字
u32 TagNum(const u8 *pTag)
{
	u8 i;
	u32 Num;
	u8 *Buf=(void *)&Num;
	
	for(i=0;i<VAR_TAG_LEN;i++)
	{
		if(pTag[i]>='a' && pTag[i]<='z') Buf[i]=pTag[i]-32;//命令字符串全部转大写
		else Buf[i]=pTag[i];
	}

	return Num;
}

#if 1
//注 gpVarList不能为null
static VAR_LIST_ITEM *VarListLast_(void)
{
	VAR_LIST_ITEM *pLast=gpVarList;

	while(pLast->pNext) pLast=pLast->pNext;

	return pLast;
}

//根据变量id在list中找到本体
//注意建立临界区
static VAR_LIST_ITEM *VarListFindOne_(u16 Vid)
{
	VAR_LIST_ITEM *p=gpVarList;

	if(Vid)
	{
		while(p)
		{
			if(p->Vid==Vid) return p;

			p=p->pNext;
		}
	}

	return NULL;
}

//增加一个设备到列表中
void VarListNewOne(VAR_LIST_ITEM *pNew)
{
	OS_EnterCritical();
	if(gpVarList==NULL)
	{
		gpVarList=Q_ZallocAsyn(sizeof(VAR_LIST_ITEM));
		MemCpy((void *)gpVarList,pNew,sizeof(VAR_LIST_ITEM));
		gpVarList->pNext=NULL;
	}
	else
	{
		VAR_LIST_ITEM *p=Q_ZallocAsyn(sizeof(VAR_LIST_ITEM));
		MemCpy(p,pNew,sizeof(VAR_LIST_ITEM));
		p->pNext=NULL;
		
		VarListLast_()->pNext=p;
	}
	OS_ExitCritical();	
}
#endif

//从数据库读取设备的waddr
static u32 GetVarMainDevWAddrFromInfo(u16 MainDevID)
{
	DEVICE_RECORD *pDev=Q_Zalloc(sizeof(DEVICE_RECORD));
	INFO_IDX Idx=1;
	u16 Res=1;
	u32 DevInfoID=HL16_U32(GOT_DEVICE,MainDevID);
	u32 WAddr=0;

	if(MainDevID==0) 
	{
		Q_Free(pDev);
		return 0;
	}
	
	while(Res)
	{
		Res=ReadInfoByIdx(IFT_DEV,Idx++,pDev);
		if(Res && ProdIsWav(pDev->ProdID) && pDev->ID==DevInfoID)
		{
			WAddr=pDev->Record.Waver.WNetAddr;
			break;
		}
	}

	Q_Free(pDev);

	return WAddr;
}

//根据record内容，建立变量item,并存到list中
static void VarCreate_ByRecord(u32 VarID,void *pInfoRcd)
{
	VARIABLE_RECORD *pVarRcd=NULL;
	INFO_IDX Idx=1;
	u16 Res=1;

	if(pInfoRcd==NULL)
	{
		pVarRcd=Q_Zalloc(sizeof(VARIABLE_RECORD));
		Res=ReadInfoByID(IFT_VARIABLE,VarID,pVarRcd);
	}
	else
	{
		pVarRcd=pInfoRcd;
	}
	
	if(Res && pVarRcd->ID)
	{
		VAR_LIST_ITEM *pVar=Q_Zalloc(sizeof(VAR_LIST_ITEM));
		pVar->Vid=pVarRcd->ID;
		pVar->Type=pVarRcd->VarType;
		pVar->ReadOnly=pVarRcd->ReadOnly;
		pVar->VarState=VST_VALID;//esp8266特殊指定，变量可用标志，用来标识设备是否在线，esp永远在线，所以无用
		pVar->SelfIdx=0;

		pVar->DispFat=pVarRcd->DispFat;
		pVar->DotNum=pVarRcd->DotNum;
		
		pVar->OldVal=0;
		pVar->VarVal=pVarRcd->InitVal;
		
		pVar->ProdTag.Num=pVarRcd->ProdTag.Num;
		pVar->VarTag.Num=pVarRcd->VarTag.Num;
		TagToH(pVar->ProdTag.Char);
		TagToH(pVar->VarTag.Char);

		if(pVarRcd->VarType==VAR_T_DEV)
		{
			pVar->MainDevID=HL16_U32(GOT_DEVICE,pVarRcd->MainDevID);
			pVar->SelfIdx=pVarRcd->VarIdx;
			pVar->MainDevWAddr=0;//esp8266不需要，GetVarMainDevWAddrFromInfo(pVarRcd->MainDevID);
		}
		else
		{
			pVar->MainDevID=0;
			pVar->SelfIdx=0;
			pVar->MainDevWAddr=0;
		}

		VarListNewOne(pVar);
		if(NeedDebug(DFT_VAR)) Debug("+pVar:0x%x(%u)\n\r",pVar->Vid,pVar->Vid);
		
		Q_Free(pVar);
	}

	if(pInfoRcd==NULL) Q_Free(pVarRcd);//当pInfoRcd为空时，pVarRcd是自己申请的，需要释放
}

#if 0 //esp8266无需
//建立关联变量和设备的关系
static void BuildDevVarRelation(void)
{
	DEVICE_RECORD *pDev=Q_Zalloc(sizeof(DEVICE_RECORD));
	INFO_IDX Idx=1;
	u16 Res=1;
	
	while(Res)
	{
		Res=ReadInfoByIdx(IFT_DEV,Idx++,pDev);
		if(Res && ProdIsWav(pDev->ProdID))
		{
			WAVER_DEV *pWav=&pDev->Record.Waver;
			u16 i=0;
			
			for(i=0;i<RELATE_VAR_MAX;i++) //关心变量
			{
				if(pWav->RelateVar[i]) Debug("+Dev[%u]->Relate[%u]\n\r",pDev->ID,pWav->RelateVar[i]);
				NotfiyAdd_Var2WDev(pWav->RelateVar[i],pWav->WNetAddr,i);
			}
		}
	}

	Q_Free(pDev);
}
#endif

//初始化所有变量
//可重复调用
void VarListInit(void)
{
	//清空旧数据
	OS_EnterCritical();
	while(gpVarList)//清变量
	{
		VAR_LIST_ITEM *pVarNext=gpVarList->pNext;
		NotifyDel_Var2All(gpVarList->Vid);
		Q_Free((void *)gpVarList);
		gpVarList=pVarNext;
	}
	OS_ExitCritical();

	//从数据库读取信息
	{
		VARIABLE_RECORD *pVarRcd=Q_Zalloc(sizeof(VARIABLE_RECORD));
		VAR_LIST_ITEM *pVar=Q_Zalloc(sizeof(VAR_LIST_ITEM));
		INFO_IDX Idx=1;
		u16 Res=1;
		
		while(Res)
		{
			Res=ReadInfoByIdx(IFT_VARIABLE,Idx++,pVarRcd);
			if(Res && pVarRcd->ID)
			{
				//Debug("Has[%c%c%c%c.%c%c%c%c]\n\r",DispVarTagp(pVarRcd));
				VarCreate_ByRecord(pVarRcd->ID,NULL);//插入到list中
			}
		}

		Q_Free(pVarRcd);
		Q_Free(pVar);
	}
	
	//如果系统变量有不存在的，则新建数据库
	//CheckAllSysVarHave(FALSE); 

	//建立关联变量和设备的关系
	//BuildDevVarRelation();//esp8266不需要

}

//删除设备下所有自身变量
static void VarsDelete_ByMainDev(u32 MainDevID,bool DelFromRecord)
{
	VAR_LIST_ITEM *p=NULL;
	VAR_LIST_ITEM *pPre=NULL;
	
	if(LBit16(MainDevID)==0) return;

	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(p->MainDevID == MainDevID)
		{
			if(pPre!=NULL) pPre->pNext=p->pNext;
			if(p==gpVarList) gpVarList=p->pNext;

			NotifyDel_Var2All(p->Vid);//删除其他设备对自身变量的关注
			if(NeedDebug(DFT_VAR)) Debug("-pVar:0x%x(%u)\n\r",p->Vid,p->Vid);
			if(DelFromRecord) DeleteInfo(IFT_VARIABLE,HL16_U32(GOT_VARIABLE,p->Vid));//从record删除
			Q_Free(p);	
			if(p==gpVarList) gpVarList=NULL;

			//继续循环
			p=pPre?pPre->pNext:gpVarList;
			continue;
		}

		pPre=p;
		p=p->pNext;
	}
	OS_ExitCritical();
}

//将可能冲突的ProdTag转换为不冲突的tag返回
u32 ConvertOnlyTags(u32 TagNum)
{
	VAR_LIST_ITEM *p=NULL;
	u8 *pTag=(u8 *)&TagNum;
	
	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(TagNum == p->ProdTag.Num)//找到一样的了
		{
			if(pTag[3]<'1' || pTag[3]>'9')//is letter
			{
				pTag[3]='1';
			}
			else 
			{
				pTag[3]++;
			}
			p=gpVarList;//因为改变了tag，所以要从头开始比对重复
			continue;
		}		

		p=p->pNext;
	}
	OS_ExitCritical();

	return TagNum;
}

VAR_TYPE GetVarType(u16 Vid)
{
	VAR_LIST_ITEM *p=NULL;
	VAR_TYPE Type=VAR_T_NULL;
	
	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(p->Vid==Vid)
		{
			Type=p->Type;
			break;
		}

		p=p->pNext;
	}
	OS_ExitCritical();

	return Type;
}

VAR_DISP_FAT GetVarDispFat(u16 Vid,u8 *pDotNum)
{
	VAR_LIST_ITEM *p=NULL;
	VAR_DISP_FAT DispFat=VDF_U16;
	u8 DotNum=0;
	
	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(p->Vid==Vid)
		{
			DispFat=p->DispFat;
			DotNum=p->DotNum;
			break;
		}

		p=p->pNext;
	}
	OS_ExitCritical();

	if(pDotNum!=NULL) *pDotNum=DotNum;

	return DispFat;
}

#if 1
//根据vid获取变量的设备id和万纳网地址
//可用来判断变量对应的设备是否存在
u32 GetVarMainDevID(u16 Vid,u32 *pWAddr)
{
	VAR_LIST_ITEM *p=NULL;
	u32 MainDevID=0;
	
	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(p->Vid==Vid)
		{
			MainDevID=p->MainDevID;
			if(pWAddr) *pWAddr=p->MainDevWAddr;
			break;
		}

		p=p->pNext;
	}
	OS_ExitCritical();

	return MainDevID;
}

//根据vid获取变量的万纳网地址及自身索引
u32 GetVarMainDevWAddr(u16 Vid,u8 *pSelfIdx)
{
	VAR_LIST_ITEM *p=NULL;
	u32 MainDevWAddr=0;
	
	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(p->Vid==Vid)
		{
			MainDevWAddr=p->MainDevWAddr;
			if(pSelfIdx) *pSelfIdx=p->SelfIdx;
			break;
		}

		p=p->pNext;
	}
	OS_ExitCritical();

	return MainDevWAddr;
}

#if 0 //esp8266无需
//根据自身变量索引获取vid
u16 GetVarVid_ByWAddr(u32 MainWAddr,u8 SelfIdx)
{
	VAR_LIST_ITEM *p=NULL;
	u16 Vid=0;
	
	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(p->MainDevWAddr==MainWAddr && p->SelfIdx==SelfIdx)
		{
			Vid=p->Vid;
			break;
		}

		p=p->pNext;
	}
	OS_ExitCritical();

	return Vid;
}

//根据关联变量索引获取vid
u16 GetRelVarVid_ByWAddr(u32 MainWAddr,u8 RelIdx)
{
	u32 DevID=WDevGetInfoID(MainWAddr);
	u16 Vid=0;

	if(DevID)
	{
		DEVICE_RECORD *pDev=Q_Zalloc(sizeof(DEVICE_RECORD));
		u16 Res=ReadInfoByID(IFT_DEV,DevID,pDev);
		if(Res && pDev->ID && ProdIsWav(pDev->ProdID))
		{				
			Vid=pDev->Record.Waver.RelateVar[RelIdx-1];
		}		 
		Q_Free(pDev);
	}

	return Vid;
}
#endif
#endif

//通过tag，返回变量id
u16 FindVarId_ByTag(const u8 *pTagStr)
{
	u32 ProdTag=TagNum(pTagStr);
	u32 VarTag=TagNum(&pTagStr[VAR_TAG_LEN]);
	VAR_LIST_ITEM *p=NULL;
	u16 Vid=0;
	
	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(p->ProdTag.Num==ProdTag && p->VarTag.Num==VarTag)
		{
			Vid=p->Vid;
			break;
		}

		p=p->pNext;
	}
	OS_ExitCritical();
	
	return Vid;
}

//通过tag，返回变量id
u16 FindVarId_ByTag2(const u8 *pProdTagStr,const u8 *pVarTagStr)
{
	u32 ProdTag=TagNum(pProdTagStr);
	u32 VarTag=TagNum(pVarTagStr);
	VAR_LIST_ITEM *p=NULL;
	u16 Vid=0;
	
	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(p->ProdTag.Num==ProdTag && p->VarTag.Num==VarTag)
		{
			Vid=p->Vid;
			break;
		}

		p=p->pNext;
	}
	OS_ExitCritical();
	
	return Vid;
}

//通过设备info id和变量索引，返回变量id
u16 FindVarId_ByDev(u32 DevID,u8 Idx,VAR_IDX_T VarIdxType)
{
	u16 Vid=0;
	
	if(DevID==0 || Idx==0) return 0;

	if(VarIdxType==VIT_RELATE) //关联变量
	{
		if(Idx>=1 && Idx<=RELATE_VAR_MAX)
		{
			DEVICE_RECORD *pDev=Q_Zalloc(sizeof(DEVICE_RECORD));
			u16 Res=ReadInfoByID(IFT_DEV,DevID,pDev);
			if(Res && pDev->ID && ProdIsWav(pDev->ProdID))
			{				
				Vid=pDev->Record.Waver.RelateVar[Idx-1];
			}		
			Q_Free(pDev);
			
			return Vid;	
		}
	}
	else if(VarIdxType==VIT_SELF) //自身变量
	{
		if(Idx>=1 && Idx<=SELF_VAR_TOTAL)
		{
			VAR_LIST_ITEM *p=NULL;
			
			OS_EnterCritical();
			p=gpVarList;
			while(p)
			{
				if(p->Type==VAR_T_DEV && p->MainDevID==DevID && p->SelfIdx==Idx)
				{
					Vid=p->Vid;
					break;
				}

				p=p->pNext;
			}
			OS_ExitCritical();

			return Vid;
		}
	}

	return 0;
}

#if 1
//变量修改完毕后的钩子函数
static void VarSetFinish_Hook(VAR_CHANGE_INFO *pInfo,u16 InfoNum)
{
	for(;InfoNum;InfoNum--,pInfo++)
	{
		if(NeedDebug(DFT_VAR)){if(pInfo->Vid) Debug("VarSet:[%u]=%d\n\r",pInfo->Vid,pInfo->Value);}
		if(pInfo->Vid) VarChangeInform(HL16_U32(GOT_VARIABLE,pInfo->Vid),pInfo->Value,pInfo->VarState,pInfo->SetorType,pInfo->SetorID);
		if(pInfo->VarState==VST_VALID) UpdateIoStateByVar(pInfo->Vid,pInfo->Value);//同步gpio
	}
}

//使所有系统变量有效，相关线程启动后调用
void VaildAllSysVars(void)
{
	VAR_CHANGE_INFO *pInfoList=NULL;
	VAR_LIST_ITEM *p=NULL;
	u16 VarNum=0;

	//计算个数
	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(p->Type==VAR_T_SYS || p->Type==VAR_T_USER) 
		{
			VarNum++;
		}
		p=p->pNext;
	}	

	//正式处理
	if(VarNum)
	{
		pInfoList=Q_Zalloc(VarNum*sizeof(VAR_CHANGE_INFO));
		p=gpVarList;
		while(p)
		{
			if(p->Type==VAR_T_SYS || p->Type==VAR_T_USER) 
			{
				p->VarState=VST_VALID;

				pInfoList->Vid=p->Vid;
				pInfoList->VarState=p->VarState;
				pInfoList->Value=p->VarVal;
				pInfoList->SetorType=VRT_SYS;
				pInfoList->SetorID=0;
				pInfoList++;
			}

			p=p->pNext;
		}	
	}	
	OS_ExitCritical();

	if(VarNum && pInfoList) VarSetFinish_Hook(pInfoList,VarNum);
	if(pInfoList!=NULL) Q_Free(pInfoList);
}

//设备掉线后，将对应的变量设置为无效
//只能由主机自己来设置
void SetVarInvaild_ByDev(u32 WDevAddr)
{
	VAR_CHANGE_INFO *pInfoList=NULL;
	VAR_LIST_ITEM *p=NULL;
	u16 VarNum=0;
	
	if(WDevAddr==0) return;

	//计算个数
	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(p->Type==VAR_T_DEV && p->VarState==VST_VALID && p->MainDevWAddr==WDevAddr)//找到对象了
		{
			VarNum++;
		}
		p=p->pNext;
	}
	
	//正式处理
	if(VarNum)
	{
		pInfoList=Q_Zalloc(VarNum*sizeof(VAR_CHANGE_INFO));
		p=gpVarList;
		while(p)
		{
			if(p->Type==VAR_T_DEV && p->VarState==VST_VALID && p->MainDevWAddr==WDevAddr)//找到对象了
			{			
				p->VarState=VST_NULL;
				
				pInfoList->Vid=p->Vid;
				pInfoList->VarState=p->VarState;
				pInfoList->Value=p->VarVal;
				pInfoList->SetorType=VRT_WDEV;//假装是设备发出的，这样就会不再通知设备自己
				pInfoList->SetorID=WDevAddr;
				pInfoList++;
			}

			p=p->pNext;
		}
	}
	OS_ExitCritical();

	if(VarNum && pInfoList) VarSetFinish_Hook(pInfoList,VarNum);
	Q_Free(pInfoList);
}

//设置变量有效，内部使用
VAR_CHG_RES SetVarVaild(u16 Vid)
{
	VAR_CHANGE_INFO ChgInfo={0};
	VAR_LIST_ITEM *p=NULL;
	VAR_CHG_RES Res=VRES_FAILD;
	
	if(Vid==0) return Res;
	
	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(p->Vid==Vid && p->VarState==VST_NULL)//找到对象了
		{			
			p->VarState=VST_VALID;
			Res=VRES_SUCESS;

			ChgInfo.Vid=p->Vid;
			ChgInfo.VarState=p->VarState;
			ChgInfo.Value=p->VarVal;
			ChgInfo.SetorType=VRT_WDEV;//假装是设备发出的，这样就会不再通知设备自己
			ChgInfo.SetorID=p->MainDevWAddr;
			break;
		}

		p=p->pNext;
	}
	OS_ExitCritical();

	if(ChgInfo.Vid) VarSetFinish_Hook(&ChgInfo,1);

	return Res;
}

//内部使用，返回ture表示修改成功
//SetorID:链接的app client id或者设备的wnet addr
static VAR_CHG_RES SetVarVal_(VAR_LIST_ITEM *p,TVAR32 Val,VAR_OPTOR SetorType,u32 SetorID)
{
	VAR_CHG_RES Ret=VRES_FAILD;

	if(Val==p->VarVal && p->VarState==VST_VALID) Ret=VRES_NOCHANGE;

	switch(SetorType)
	{
		case VRT_SYS://网关自己改变的
			if(Val!=p->VarVal)
			{
				if(p->VarState==VST_NULL)
				{
					p->VarState=VST_VALID;
					p->OldVal=p->VarVal;
					p->VarVal=Val;
					Ret=VRES_SUCESS;
				}

				if(Val!=p->VarVal)
				{
					p->OldVal=p->VarVal;
					p->VarVal=Val;
					Ret=VRES_SUCESS;
				}
			}
			break;
		case VRT_WDEV://dev改变的
			if(p->Type==VAR_T_DEV?p->MainDevWAddr==SetorID:TRUE) //wdev只能改系统变量，用户变量，或者自己的设备变量
			{
				if(p->VarState==VST_NULL)
				{
					p->VarState=VST_VALID;
					p->OldVal=p->VarVal;
					p->VarVal=Val;
					Ret=VRES_SUCESS;
				}

				if(Val!=p->VarVal)
				{
					p->OldVal=p->VarVal;
					p->VarVal=Val;
					Ret=VRES_SUCESS;
				}
			}
			break;
		case VRT_APP://app改变的变量
			if(p->VarState==VST_VALID && p->ReadOnly==FALSE)
			{
				if(Val!=p->VarVal)
				{
					p->OldVal=p->VarVal;
					p->VarVal=Val;
					Ret=VRES_SUCESS;
				}
			}
			break;
	}

	return Ret;
}

//设置变量值
//SetorID:链接的app client id或者设备的wnet addr
VAR_CHG_RES SetVarVal(u16 Vid,TVAR32 Val,VAR_OPTOR SetorType,u32 SetorID)
{
	VAR_CHANGE_INFO ChgInfo={0};
	VAR_LIST_ITEM *p=NULL;
	VAR_CHG_RES Ret=VRES_FAILD;

	if(Vid==0) return Ret;
	
	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(p->Vid==Vid)//找到对象了
		{			
			Ret=SetVarVal_(p,Val,SetorType,SetorID);

			if(Ret==VRES_SUCESS)
			{
				ChgInfo.Vid=p->Vid;
				ChgInfo.VarState=p->VarState;
				ChgInfo.Value=p->VarVal;
				ChgInfo.SetorType=SetorType;
				ChgInfo.SetorID=SetorID;		
			}
			break;
		}

		p=p->pNext;
	}
	OS_ExitCritical();

	if(ChgInfo.Vid) VarSetFinish_Hook(&ChgInfo,1);

	return Ret;
}

//按位设置设备变量
VAR_CHG_RES SetVarBit(u16 Vid,u8 Bit,u8 BitVal,VAR_OPTOR SetorType,u32 SetorID)
{
	VAR_CHANGE_INFO ChgInfo={0};
	VAR_LIST_ITEM *p=NULL;
	VAR_CHG_RES Ret=VRES_FAILD;
	TVAR32 Val=0;
	
	if(Vid==0 || Bit>15) return Ret;
	
	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(p->Vid==Vid)//找到对象了
		{			
			Val=p->VarVal;
			if(BitVal) SetBit(Val,Bit);
			else ClrBit(Val,Bit);			
			Ret=SetVarVal_(p,Val,SetorType,SetorID);

			if(Ret==VRES_SUCESS)
			{
				ChgInfo.Vid=p->Vid;
				ChgInfo.VarState=p->VarState;
				ChgInfo.Value=p->VarVal;
				ChgInfo.SetorType=SetorType;
				ChgInfo.SetorID=SetorID;		
			}
			break;
		}

		p=p->pNext;
	}
	OS_ExitCritical();

	if(ChgInfo.Vid) VarSetFinish_Hook(&ChgInfo,1);

	return Ret;
}

//按位翻转设备变量
VAR_CHG_RES RevVarBit(u16 Vid,u8 Bit,VAR_OPTOR SetorType,u32 SetorID)
{
	VAR_CHANGE_INFO ChgInfo={0};
	VAR_LIST_ITEM *p=NULL;
	VAR_CHG_RES Ret=VRES_FAILD;
	TVAR32 Val=0;
	
	if(Vid==0 || Bit>15) return Ret;
	
	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(p->Vid==Vid)//找到对象了
		{			
			Val=p->VarVal;
			RevBit(Val,Bit);		
			Ret=SetVarVal_(p,Val,SetorType,SetorID);

			if(Ret==VRES_SUCESS)
			{
				ChgInfo.Vid=p->Vid;
				ChgInfo.VarState=p->VarState;
				ChgInfo.Value=p->VarVal;
				ChgInfo.SetorType=SetorType;
				ChgInfo.SetorID=SetorID;		
			}
			break;
		}

		p=p->pNext;
	}
	OS_ExitCritical();

	if(ChgInfo.Vid) VarSetFinish_Hook(&ChgInfo,1);

	return Ret;
}

//变量自增加
VAR_CHG_RES IncVar(u16 Vid,VAR_OPTOR SetorType,u32 SetorID)
{
	VAR_CHANGE_INFO ChgInfo={0};
	VAR_LIST_ITEM *p=NULL;
	VAR_CHG_RES Ret=VRES_FAILD;
	TVAR32 Val=0;
	
	if(Vid==0) return Ret;
	
	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(p->Vid==Vid)//找到对象了
		{			
			if(p->VarVal!=0xffff) Val=p->VarVal+1;//最多加到0xffff
			Ret=SetVarVal_(p,Val,SetorType,SetorID);

			if(Ret==VRES_SUCESS)
			{
				ChgInfo.Vid=p->Vid;
				ChgInfo.VarState=p->VarState;
				ChgInfo.Value=p->VarVal;
				ChgInfo.SetorType=SetorType;
				ChgInfo.SetorID=SetorID;		
			}
			break;
		}

		p=p->pNext;
	}
	OS_ExitCritical();

	if(ChgInfo.Vid) VarSetFinish_Hook(&ChgInfo,1);

	return Ret;
}

//变量自减少
VAR_CHG_RES DecVar(u16 Vid,VAR_OPTOR SetorType,u32 SetorID)
{
	VAR_CHANGE_INFO ChgInfo={0};
	VAR_LIST_ITEM *p=NULL;
	VAR_CHG_RES Ret=VRES_FAILD;
	TVAR32 Val=0;
	
	if(Vid==0) return Ret;
	
	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(p->Vid==Vid)//找到对象了
		{			
			if(p->VarVal) Val=p->VarVal-1;//最多减到0
			Ret=SetVarVal_(p,Val,SetorType,SetorID);

			if(Ret==VRES_SUCESS)
			{
				ChgInfo.Vid=p->Vid;
				ChgInfo.VarState=p->VarState;
				ChgInfo.Value=p->VarVal;
				ChgInfo.SetorType=SetorType;
				ChgInfo.SetorID=SetorID;		
			}
			break;
		}

		p=p->pNext;
	}
	OS_ExitCritical();

	if(ChgInfo.Vid) VarSetFinish_Hook(&ChgInfo,1);

	return Ret;
}

//获取变量tag，如果没找到，返回false
//注意，返回的tag不带结束符
bool GetVarProdTag(u16 Vid,u8 *pProdTag,u8 *pVarTag)
{
	VAR_LIST_ITEM *p=NULL;
	bool Has=FALSE;
	
	if(pProdTag) MemSet(pProdTag,0,VAR_TAG_LEN);
	if(pVarTag) MemSet(pVarTag,0,VAR_TAG_LEN);
		
	if(Vid==0) return FALSE;
	
	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(p->Vid==Vid)
		{
			if(pProdTag) MemCpy(pProdTag,&p->ProdTag,VAR_TAG_LEN);
			if(pVarTag) MemCpy(pVarTag,&p->VarTag,VAR_TAG_LEN);
			Has=TRUE;
			break;
		}

		p=p->pNext;
	}
	OS_ExitCritical();

	return Has;
}

//获取变量tag，如果没找到，返回false
//注意，返回的tag带结束符，所以空间要多加一个字节
bool GetVarProdTag2(u16 Vid,u8 *pTags)
{
	VAR_LIST_ITEM *p=NULL;
	bool Has=FALSE;
	
	if(pTags) MemSet(pTags,0,VAR_TAG_LEN*2+1);
		
	if(Vid==0) return FALSE;
	
	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(p->Vid==Vid)
		{
			if(pTags) 
			{
				MemCpy(pTags,&p->ProdTag,VAR_TAG_LEN);
				MemCpy(&pTags[VAR_TAG_LEN],&p->VarTag,VAR_TAG_LEN);
				pTags[VAR_TAG_LEN*2]='\0';
			}
			Has=TRUE;
			break;
		}

		p=p->pNext;
	}
	OS_ExitCritical();

	return Has;
}

//获取变量值及状态
VAR_STATE GetVarState(u16 Vid,TVAR32 *pVal)
{
	VAR_LIST_ITEM *p=NULL;
	VAR_STATE State=VST_NOT_FOUND;

	if(pVal) *pVal=0;
	if(Vid==0) return State;
	
	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(p->Vid==Vid)
		{
			if(pVal) *pVal=p->VarVal;
			State=p->VarState;
			break;
		}

		p=p->pNext;
	}
	OS_ExitCritical();

	return State;
}

//获取变量值及状态
TVAR32 GetVarValue(u16 Vid,VAR_STATE *pState)
{
	VAR_LIST_ITEM *p=NULL;
	TVAR32 Val=0;

	if(Vid==0)
	{
		if(pState) *pState=VST_NOT_FOUND;
		return Val;
	}
	
	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(p->Vid==Vid)
		{
			if(pState) *pState=p->VarState;
			Val=p->VarVal;
			break;
		}

		p=p->pNext;
	}
	OS_ExitCritical();

	return Val;
}
#endif

#if 0 //esp8266无需
//当设备的任一变量失效时，返回false，否则返回true
//DevID或WAddr任意填一个值即可
bool CheckDevVarAllVaild(u32 DevID,u32 WAddr)
{
	VAR_LIST_ITEM *p=NULL;
	bool Ret=TRUE;

	if(DevID)
	{
		OS_EnterCritical();
		p=gpVarList;
		while(p)
		{
			if(p->MainDevID==DevID && p->VarState!=VST_VALID)
			{				
				Ret=FALSE;
				break;
			}

			p=p->pNext;
		}
		OS_ExitCritical();
		return Ret;
	}
	else if(WAddr)
	{
		OS_EnterCritical();
		p=gpVarList;
		while(p)
		{
			if(p->MainDevWAddr==WAddr && p->VarState!=VST_VALID)
			{				
				Ret=FALSE;
				break;
			}

			p=p->pNext;
		}
		OS_ExitCritical();
		return Ret;
	}

	return TRUE;
}
#endif

#if 1
//从list中删除一个变量
static void VarDelete(u32 Vid)
{
	VAR_LIST_ITEM *p=NULL;
	VAR_LIST_ITEM *pPre=NULL;

	Vid=LBit16(Vid);
	
	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(p->Vid==Vid)
		{
			if(pPre!=NULL) pPre->pNext=p->pNext;
			if(p==gpVarList) gpVarList=p->pNext;

			if(NeedDebug(DFT_VAR)) Debug("-pVar:0x%x(%u)\n\r",p->Vid,p->Vid);
			Q_Free(p);
			break;
		}

		pPre=p;
		p=p->pNext;
	}
	OS_ExitCritical();
}

//增加wav设备后，用来增加对应的变量，关联变量
static void UpdateVars_NewDev(u32 DevID)
{
	DEVICE_RECORD *pDev=Q_Zalloc(sizeof(DEVICE_RECORD));
	u16 Res=0;
	
	Res=ReadInfoByID(IFT_DEV,DevID,pDev);
	if(Res && ProdIsWav(pDev->ProdID))
	{
		//建立自身变量
		{
			VARIABLE_RECORD *pVarRcd=Q_Zalloc(sizeof(VARIABLE_RECORD));
			INFO_IDX Idx=1;
			u16 vRes=1;

			while(vRes)
			{
				vRes=ReadInfoByIdx(IFT_VARIABLE,Idx++,pVarRcd);
				if(vRes && pVarRcd->ID && pVarRcd->MainDevID == LBit16(pDev->ID))//查找自身变量
				{
					VarCreate_ByRecord(pVarRcd->ID,pVarRcd);//插入到list中
				}
			}

			Q_Free(pVarRcd);
		}

#if 0	//关心关联变量 //esp8266不需要
		{
			WAVER_DEV *pWav=&pDev->Record.Waver;
			u16 i=0;

			for(i=0;i<RELATE_VAR_MAX;i++)//关心变量
			{
				NotfiyAdd_Var2WDev(pWav->RelateVar[i],pWav->WNetAddr,i);							
			}
		}
#endif
	}

	Q_Free(pDev);
}

//删除wav设备所有变量，关联变量关系
//NeedDelInfo为true时，删除变量record info
static void UpdateVars_DeleteDev(u32 DevID)
{
	DEVICE_RECORD *pDev=NULL;

	if(DevID==0) return;

	pDev=Q_Zalloc(sizeof(DEVICE_RECORD));
	
	if(ReadInfoByID(IFT_DEV,DevID,pDev))
	{
		if(ProdIsWav(pDev->ProdID))
		{
			VarsDelete_ByMainDev(pDev->ID,TRUE);//删除自身变量		
		}
	}

	Q_Free(pDev);
}

//处理系统事件
void VarManEventInput(PUBLIC_EVENT Event,u32 Param,void *p)
{
	switch(Event)
	{
		case PET_APP_NEW_DEV:
			UpdateVars_NewDev(Param);
			break;
		case PET_APP_MODIFY_DEV:
			break;
		case PET_APP_DEL_DEV:	
			UpdateVars_DeleteDev(Param);
			break;
		case PET_APP_DEL_VAR:
			VarDelete(Param);
			break;
	}
}
#endif







