#include "SysDefines.h"

extern const VARIABLE_RECORD gInfo_VarRcdDef[];
static u8 gSysVarHaveFlag[4]={0,0,0,0};//数组大小根据系统变量实际个数确定，每个bit存储一个变量的状态

//设置已经存在的系统变量标志位
void SetSysVarHave(u32 ProdTag,u32 VarTag)
{
	u16 i;
	
	for(i=0;;i++)
	{
		if(gInfo_VarRcdDef[i].ProdTag.Num==0) break;
		if(TagNum(gInfo_VarRcdDef[i].ProdTag.Char)==ProdTag && TagNum(gInfo_VarRcdDef[i].VarTag.Char)==VarTag)
		{
			SetArrayBit(gSysVarHaveFlag,i);
			return;
		}
	}	
}

//检查是否所有的系统变量都在数据库中
//ForceCreatAll == TRUE 时，创建所有系统变量
void CheckAllSysVarHave(bool ForceCreatAll)
{
	VARIABLE_RECORD *pRcd=Q_Malloc(sizeof(VARIABLE_RECORD));
	u16 i;
	bool NeedReboot=FALSE;
	
	for(i=0;;i++)
	{
		if(gInfo_VarRcdDef[i].ProdTag.Num==0) break;
		if(ReadArrayBit(gSysVarHaveFlag,i)==0 || ForceCreatAll)
		{
			NeedReboot=TRUE;

			//将不存在的系统变量新建到数据库中
			Debug("New[%u][%c%c%c%c.%c%c%c%c]\n\r",i,DispVarTag(gInfo_VarRcdDef[i]));
			MemCpy(pRcd,&gInfo_VarRcdDef[i],sizeof(VARIABLE_RECORD));
			pRcd->ID=HL16_U32(GOT_VARIABLE,ObjIdBase());
			UpdateObjIdBase();
			if(SaveInfo(IFT_VARIABLE,pRcd)>0)//存储成功
			{
				Debug("Save New SysVar %u\n\r",pRcd->ID);
			}		
		}
	}
	Q_Free(pRcd);
	
	//并重启主机
	if(NeedReboot)
	{
		RebootBoard();
	}
}

//周期性设置系统变量
void SysVarPeriodicitySet(bool SecEvt)
{
	static u16 TimeVid=0;
	static u16 WeekVid=0;
	static u16 SecdVid=0;
	static u16 MintVid=0;

	if(SecEvt) //秒事件
	{
		RTC_TIME Time;

		if(SecdVid==0) SecdVid=FindVarId_ByTag2("TIME","SECD");
		if(TimeVid==0) TimeVid=FindVarId_ByTag2("NOWS","TIME");

		RtcCnt2Time(RtcGetCnt(),&Time);
		if(GetVarValue(SecdVid,NULL)) IncVar(SecdVid,VRT_SYS,0);
		SetVarVal(TimeVid,Time.hour*100+Time.min,VRT_SYS,0);
	}
	else //半分事件
	{
		static u16 OldMin=0;
		TVAR32 Week,Wbit=0;
		RTC_TIME Time;

		if(WeekVid==0) WeekVid=FindVarId_ByTag2("DATE","WEEK");
		if(MintVid==0) MintVid=FindVarId_ByTag2("TIME","MINT");		

		RtcCnt2Time(RtcGetCnt(),&Time);
		Week=Time.week?Time.week:7;//由0-6改成1-7
		SetBit(Wbit,Week-1);
		
		SetVarVal(WeekVid,Week,VRT_SYS,0);

		if(Time.min != OldMin && GetVarValue(MintVid,NULL))
		{
			IncVar(MintVid,VRT_SYS,0);
			OldMin=Time.min;
		}		
	}
}

//从字符串中获取数值并设置到系统变量中
void Fetch2VarFormString(const char *pMsg)
{
	u16 Var1=FindVarId_ByTag2("SMSG","VAR1");
	u16 Var2=FindVarId_ByTag2("SMSG","VAR2");
	TVAR32 Value1,Value2;
	const char *pNext=NULL;
	
	pNext=FindNumFromStr(pMsg,&Value1);
	if(pNext)
	{
		SetVarVal(Var1,Value1,VRT_SYS,0);

		if(*pNext)
		{
			pNext=FindNumFromStr(pNext,&Value2);
			if(pNext)
			{
				SetVarVal(Var2,Value2,VRT_SYS,0);
			}
		}
		else
		{
			SetVarVal(Var2,0,VRT_SYS,0);
		}
	}	
	else
	{
		SetVarVal(Var1,0,VRT_SYS,0);
		SetVarVal(Var2,0,VRT_SYS,0);
	}
}


