#include "SysDefines.h"

typedef struct{
	u32 ID;
	u32 SubID;
}STR_RCD_HEADER;

//�����Ч���ַ���
static void FilterVaildID(STR_RCD_HEADER *pIDs,u16 IdNum,ID_T ID)
{
	u16 i;

	if(ID)
	for(i=0;i<IdNum;i++)
	{
		if(pIDs[i].ID==ID) 
		{
			if(NeedDebug(DFT_SYS)) Debug("+[%u:%u]",pIDs[i].ID,pIDs[i].ID);
			pIDs[i].ID=0;
		}
	}
}

//�����Ч���ַ���
static void FilterVaildSubID(STR_RCD_HEADER *pIDs,u16 IdNum,ID_T SubID)
{
	u16 i;

	if(SubID)
	for(i=0;i<IdNum;i++)
	{
		if(pIDs[i].SubID==SubID) 
		{
			if(NeedDebug(DFT_SYS)) Debug("+[%u:%u]",pIDs[i].ID,pIDs[i].ID);
			pIDs[i].ID=0;
			pIDs[i].SubID=0;
		}
	}
}

//������ʱ�����һ�Σ���ɾ�������ַ�����¼
void StrRecordTidy(void)
{
	u32 *pRcd=Q_Malloc(1024);
	STR_RECORD *pStrRcd=(void *)pRcd;
	SCENE_RECORD *pScn=(void *)pRcd;
	TRIGGER_RECORD *pTrig=(void *)pRcd;
	DEVICE_RECORD *pDev=(void *)pRcd;
	VARIABLE_RECORD *pVar=(void *)pRcd;
	
	INFO_IDX Idx=1;
	u16 Res=0;
	u16 InfoNum=0;
	STR_RCD_HEADER *pObjIDs=NULL;
	INFO_TYPE Type;

	InfoNum=GetTypeInfoTotal(IFT_STR);
	pObjIDs=Q_Malloc(InfoNum*sizeof(STR_RCD_HEADER));
	if(NeedDebug(DFT_SYS)) Debug("Str Info Num = %u\n\r",InfoNum);
	
	for(Idx=1;Idx<=InfoNum;Idx++)
	{
		pStrRcd->ID=0;
		Res=ReadInfoByIdx(IFT_STR,Idx,pStrRcd);
		if(Res && pStrRcd->ID) 
		{
			switch(pStrRcd->Type)//��¼Ҫɾ�������ݣ��Ƚ��������ݶ���¼����
			{
				case SRT_STR:
				case SRT_DATA:
				case SRT_DEV_STR:
				case SRT_VAR_UP_CONF:
				default:
					pObjIDs[Idx-1].ID=pStrRcd->ID;
					pObjIDs[Idx-1].SubID=pStrRcd->SubID;			
			}

			if(NeedDebug(DFT_SYS)) Debug("=[%u]",pStrRcd->ID);
		}
	}
	if(NeedDebug(DFT_SYS)) Debug("\n\r");
		
	//������Ч��scn�ַ���
	Res=1;Idx=1;
	while(Res)
	{
		pScn->ID=0;
		Res=ReadInfoByIdx(IFT_SCENE,Idx++,pScn);
		if(Res && pScn->ID)
		{
			u16 i;
			for(i=0;i<SCENE_MAX_ITEM_NUM;i++)
			{
				if(pScn->Items[i].Act==SIA_NULL) break;
				if(pScn->Items[i].Act==SIA_SYS_MSG)
				{
					FilterVaildID(pObjIDs,InfoNum,pScn->Items[i].Exc.Str.StrID);
					FilterVaildID(pObjIDs,InfoNum,pScn->Items[i].Exc.Str.UserID);
				}
			}			
		}
	}

	//������Ч��trig�ַ���
	Res=1;Idx=1;
	while(Res)
	{
		pTrig->ID=0;
		Res=ReadInfoByIdx(IFT_TRIGGER,Idx++,pTrig);
		if(Res && pTrig->ID && pTrig->TrigType==TT_SYS_MSG)
		{
			FilterVaildID(pObjIDs,InfoNum,pTrig->Oppo.SysMsg.StrID);
		}
	}	

	//������Ч���豸�ַ���
	Res=1;Idx=1;
	while(Res)
	{
		pDev->ID=0;
		Res=ReadInfoByIdx(IFT_DEV,Idx++,pDev);
		if(Res && pDev->ID && ProdIsWav(pDev->ProdID))
		{
			FilterVaildSubID(pObjIDs,InfoNum,pDev->ID);
		}
	}	

	//������Ч�ı����ϱ���Ϣ
	Res=1;Idx=1;
	while(Res)
	{
		pVar->ID=0;
		Res=ReadInfoByIdx(IFT_VARIABLE,Idx++,pVar);
		if(Res && pVar->ID)
		{
			FilterVaildSubID(pObjIDs,InfoNum,pVar->ID);
		}
	}
	
	//�������
	if(NeedDebug(DFT_SYS)) Debug("\n\r");

	for(Idx=0;Idx<InfoNum;Idx++)
	{
		if(pObjIDs[Idx].ID)//Ҳ��˳���obj id=0����Ϣɾ����
		{
			if(NeedDebug(DFT_SYS)) Debug("-[%u]",pObjIDs[Idx].ID);
			DeleteInfo(IFT_STR,pObjIDs[Idx].ID);
		}	
	}	
	if(NeedDebug(DFT_SYS)) Debug("\n\rStr Tidy Finish!\n\r");

	Q_Free(pObjIDs);
	Q_Free(pRcd);
}

//ͨ��id��ȡstr info ����
u8 GetStrRecordData(ID_T Id,u8 *pOut)
{
	u16 Res,Bytes=0;
		
	if(HBit16(Id)==GOT_NAME)
	{
		STR_RECORD *pStrRcd=Q_Malloc(sizeof(STR_RECORD));
		
		Res=ReadInfoByID(IFT_STR,Id,pStrRcd);
		if(Res && pStrRcd->ID)
		{
			Bytes=pStrRcd->DataLen;
			if(pOut) MemCpy(pOut,pStrRcd->Data,Bytes);
		}
		else
		{
			if(pOut) pOut[0]=0;
		}

		Q_Free(pStrRcd);
	}
	
	return Bytes;
}

//�����ַ�����¼��hashֵ
u32 CalcStrRecordHash(ID_T Id)
{
	u32 Hash=0;
	u16 Len=0;
	u16 Res;
	
	if(HBit16(Id)==GOT_NAME)
	{
		STR_RECORD *pStrRcd=Q_Malloc(sizeof(STR_RECORD));
		
		Res=ReadInfoByID(IFT_STR,Id,pStrRcd);
		if(Res && pStrRcd->ID)
		{
			Len=strlen(pStrRcd->Data);
			if(Len==0 || Len >= STR_RECORD_BYTES) Hash=0;
			else Hash=MakeHash33(pStrRcd->Data,Len);
		}

		Q_Free(pStrRcd);
	}
	
	return Hash;
}

//�����ַ���hashֵ
//��CalcBytes=0ʱ�����ַ�������
u32 CalcStrHash(const u8 *pStr,u16 CalcBytes)
{
	if(CalcBytes==0) CalcBytes=strlen(pStr);
	if(CalcBytes==0 || CalcBytes >= STR_RECORD_BYTES) return 0;
	
	return MakeHash33((void *)pStr,CalcBytes);
}

