#include "SysDefines.h"

typedef struct{
	u16 ID;
	u16 MainDevID;
}VAR_RCD_HEADER;

static void FilterVaildID(VAR_RCD_HEADER *pIDs,u16 IdNum,u16 MainDevID)
{
	u16 i;

	if(MainDevID)
	for(i=0;i<IdNum;i++)
	{
		if(pIDs[i].MainDevID==MainDevID) 
		{
			if(NeedDebug(DFT_SYS)) Debug("+[%u]",HL16_U32(GOT_VARIABLE,pIDs[i].ID));
			pIDs[i].ID=0;
			pIDs[i].MainDevID=0;
		}
	}
}

//启动的时候调用一次，会删掉冗余var记录
void VarRecordTidy(void)
{
	u32 *pRcd=Q_Malloc(1024);
	VARIABLE_RECORD *pVarRcd=(void *)pRcd;
	DEVICE_RECORD *pDev=(void *)pRcd;
	INFO_IDX Idx=1;
	u16 Res=1;
	u16 InfoNum=0;
	VAR_RCD_HEADER *pObjIDs=NULL;
	
	InfoNum=GetTypeInfoTotal(IFT_VARIABLE);
	pObjIDs=Q_Malloc(InfoNum*sizeof(VAR_RCD_HEADER));
	if(NeedDebug(DFT_SYS)) Debug("Var Info Num = %u\n\r",InfoNum);
	
	for(Idx=1;Idx<=InfoNum;Idx++)
	{
		pVarRcd->ID=0;
		Res=ReadInfoByIdx(IFT_VARIABLE,Idx,pVarRcd);
		if(Res && pVarRcd->ID && pVarRcd->VarType==VAR_T_DEV)//只整理设备变量
		{
			pObjIDs[Idx-1].ID=pVarRcd->ID;
			pObjIDs[Idx-1].MainDevID=pVarRcd->MainDevID;
			if(NeedDebug(DFT_SYS)) Debug("=[%u]",pVarRcd->ID);
		}
	}
	if(NeedDebug(DFT_SYS)) Debug("\n\r");
		
	Res=1;Idx=1;
	while(Res)
	{
		pDev->ID=0;
		Res=ReadInfoByIdx(IFT_DEV,Idx++,pDev);
		if(Res && ProdIsWav(pDev->ProdID))
		{
			FilterVaildID(pObjIDs,InfoNum,LBit16(pDev->ID));			
		}
	}
	if(NeedDebug(DFT_SYS)) Debug("\n\r");

#if 1 //把一些错误信息也加进去删除
	for(Idx=1;Idx<=InfoNum;Idx++)
	{
		pVarRcd->ID=0;
		Res=ReadInfoByIdx(IFT_VARIABLE,Idx,pVarRcd);
		if(Res && pVarRcd->VarType==VAR_T_NULL)//只整理设备变量
		{
			pObjIDs[Idx-1].ID=pVarRcd->ID;
		}
	}
#endif	

	for(Idx=0;Idx<InfoNum;Idx++)
	{
		if(pObjIDs[Idx].ID)
		{
			if(NeedDebug(DFT_SYS)) Debug("-[%u]",HL16_U32(GOT_VARIABLE,pObjIDs[Idx].ID));
			DeleteInfo(IFT_VARIABLE,HL16_U32(GOT_VARIABLE,pObjIDs[Idx].ID));
		}	
	}	
	if(NeedDebug(DFT_SYS)) Debug("\n\rVar Tidy Finish!\n\r");

	Q_Free(pObjIDs);
	Q_Free(pRcd);
}

//变量默认以s16存储
//显示方式以fat传入，
//DotNum表示需要将多少位放到小数点后，当fat=二进制时，DotNum用来决定一共显示多少位
//字符串以pOut传出，pOut请自行申请空间，不少于20 bytes
void VarStrOut(TVAR Var,VAR_DISP_FAT Fat,u8 DotNum,u8 *pOut)
{
	u8 Len=0,Char=0,i=0;
	u8 Buf[20]={0,0};
	u8 *pStr=Buf;

	if(pOut==NULL) return;
	
	MemSet(pOut,'0',20);
	
	switch(Fat)
	{
		case VDF_U16:
			if(DotNum>5){pOut[0]=0;return;}
			sprintf(pStr,"%u",(u16)(Var&0xffff));
			goto Handler;
		case VDF_HEX:
			if(DotNum>4){pOut[0]=0;return;}
			sprintf(pStr,"%X",(u16)(Var&0xffff));
			goto Handler;
		case VDF_S16:
			if(DotNum>5){pOut[0]=0;return;}
			sprintf(pStr,"%d",(s16)(Var&0xffff));
			goto Handler;
		case VDF_BIN:
			if(DotNum>16){pOut[0]=0;return;}
			for(Len=DotNum;Len;Len--,i++)
			{
				if(ReadBit(Var,Len-1)) pOut[i]='1';
				else pOut[i]='0';
			}		
			pOut[i]=0;
			break;
	}

	while(0)
	{
Handler:	
		if(DotNum)
		{
			Len=strlen(pStr);

			if(pStr[0]=='-')
			{
				Char=1;
				pStr++;
				pOut[0]='-';
				pOut++;
				Len--;
			}
			
			if(DotNum>=Len)
			{
				pOut[0]='0';
				pOut[1]='.';
				MemCpy(&pOut[2+DotNum-Len],&pStr[0],Len);
				pOut[DotNum+2]=0;
			}
			else
			{
				MemCpy(&pOut[0],&pStr[0],Len-DotNum);
				pOut[Len-DotNum]='.';
				MemCpy(&pOut[Len-DotNum+1],&pStr[Len-DotNum],DotNum);
				pOut[Len+1]=0;				
			}	

			if(Char) pOut--;
		}
		else
		{	
			Len=strlen(pStr);
			MemCpy(pOut,pStr,Len);
			pOut[Len]=0;
		}
	}
}

