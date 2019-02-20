#include "SysDefines.h"

typedef struct{
	u16 ID;
	u16 ScnID;
}TRIG_RCD_HEADER;

static void FilterVaildID(TRIG_RCD_HEADER *pIDs,u16 IdNum,u16 ScnID)
{
	u16 i;

	if(ScnID)
	for(i=0;i<IdNum;i++)
	{
		if(pIDs[i].ScnID==ScnID) 
		{
			if(NeedDebug(DFT_SYS)) Debug("+[%u]",HL16_U32(GOT_TRIGGER,pIDs[i].ID));
			pIDs[i].ID=0;
		}
	}
}

//启动的时候调用一次，会删掉冗余var记录
void TrigRecordTidy(void)
{
	u32 *pRcd=Q_Zalloc(1024);
	TRIGGER_RECORD *pTrigRcd=(void *)pRcd;
	SCENE_RECORD *pScn=(void *)pRcd;
	INFO_IDX Idx=1;
	u16 Res=1;
	u16 InfoNum=0;
	TRIG_RCD_HEADER *pObjIDs=NULL;
	
	InfoNum=GetTypeInfoTotal(IFT_TRIGGER);
	pObjIDs=Q_Zalloc(InfoNum*sizeof(TRIG_RCD_HEADER));
	if(NeedDebug(DFT_SYS)) Debug("Trig Info Num = %u\n\r",InfoNum);
	
	for(Idx=1;Idx<=InfoNum;Idx++)
	{
		pTrigRcd->ID=0;
		Res=ReadInfoByIdx(IFT_TRIGGER,Idx,pTrigRcd);
		if(Res && pTrigRcd->ID)
		{
			pObjIDs[Idx-1].ID=pTrigRcd->ID;
			pObjIDs[Idx-1].ScnID=pTrigRcd->SceneID;
			if(NeedDebug(DFT_SYS)) Debug("=[%u]",pTrigRcd->ID);
		}
	}
	if(NeedDebug(DFT_SYS)) Debug("\n\r");
		
	Res=1;Idx=1;
	while(Res)
	{
		pScn->ID=0;
		Res=ReadInfoByIdx(IFT_SCENE,Idx++,pScn);
		if(Res && pScn->ID)
		{
			FilterVaildID(pObjIDs,InfoNum,pScn->ID);			
		}
	}
	if(NeedDebug(DFT_SYS)) Debug("\n\r");

	for(Idx=0;Idx<InfoNum;Idx++)
	{
		if(pObjIDs[Idx].ID)
		{
			if(NeedDebug(DFT_SYS)) Debug("-[%u]",HL16_U32(GOT_TRIGGER,pObjIDs[Idx].ID));
			DeleteInfo(IFT_TRIGGER,HL16_U32(GOT_TRIGGER,pObjIDs[Idx].ID));
		}	
	}	
	if(NeedDebug(DFT_SYS)) Debug("\n\rTrig Tidy Finish!\n\r");

	Q_Free(pObjIDs);
	Q_Free(pRcd);
}

