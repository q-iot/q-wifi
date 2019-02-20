#include "SysDefines.h"

extern const VARIABLE_RECORD gInfo_VarRcdDef[];

//����Ƿ����е�ϵͳ�����������ݿ���
//ForceCreatAll == TRUE ʱ����������ϵͳ����
void CheckAllSysVarHave(bool ForceCreatAll)
{
	VARIABLE_RECORD *pRcd=Q_Zalloc(sizeof(VARIABLE_RECORD));
	u16 i;
	bool NeedReboot=FALSE;
	
	for(i=0;;i++)
	{
		if(gInfo_VarRcdDef[i].ProdTag.Num==0) break;
		if(FindVarId_ByTag2(gInfo_VarRcdDef[i].ProdTag.Char,gInfo_VarRcdDef[i].VarTag.Char)==0 || ForceCreatAll)
		{
			NeedReboot=TRUE;

			//�������ڵ�ϵͳ�����½������ݿ���
			Debug("New[%u][%c%c%c%c.%c%c%c%c]\n\r",i,DispVarTag(gInfo_VarRcdDef[i]));
			MemCpy(pRcd,&gInfo_VarRcdDef[i],sizeof(VARIABLE_RECORD));
			pRcd->ID=HL16_U32(GOT_VARIABLE,ObjIdBase());
			UpdateObjIdBase();
			if(SaveInfo(IFT_VARIABLE,pRcd)>0)//�洢�ɹ�
			{
				Debug("Save New SysVar %u\n\r",pRcd->ID);
			}		
		}
	}
	Q_Free(pRcd);
	
	//����������
	if(NeedReboot)
	{
		RebootBoard();
	}
}

//����������ϵͳ����
void SysVarPeriodicitySet(bool SecEvt)
{
	static u16 TimeVid=0;
	static u16 WeekVid=0;
	static u16 SecdVid=0;
	static u16 MintVid=0;

	if(SecEvt) //���¼�
	{
		RTC_TIME Time;

		if(SecdVid==0) SecdVid=FindVarId_ByTag2("TIME","SECD");
		if(TimeVid==0) TimeVid=FindVarId_ByTag2("NOWS","TIME");

		RtcCnt2Time(RtcGetCnt(),&Time);
		if(GetVarValue(SecdVid,NULL)) IncVar(SecdVid,VRT_SYS,0);
		SetVarVal(TimeVid,Time.hour*100+Time.min,VRT_SYS,0);
	}
	else //����¼�
	{
		static u16 OldMin=0;
		TVAR32 Week,Wbit=0;
		RTC_TIME Time;

		if(WeekVid==0) WeekVid=FindVarId_ByTag2("DATE","WEEK");
		if(MintVid==0) MintVid=FindVarId_ByTag2("TIME","MINT");		

		RtcCnt2Time(RtcGetCnt(),&Time);
		Week=Time.week?Time.week:7;//��0-6�ĳ�1-7
		SetBit(Wbit,Week-1);
		
		SetVarVal(WeekVid,Week,VRT_SYS,0);

		if(Time.min != OldMin && GetVarValue(MintVid,NULL))
		{
			IncVar(MintVid,VRT_SYS,0);
			OldMin=Time.min;
		}		
	}
}

//���ַ����л�ȡ��ֵ�����õ�ϵͳ������
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


