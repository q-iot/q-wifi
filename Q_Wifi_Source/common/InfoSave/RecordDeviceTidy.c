#include "SysDefines.h"


//启动的时候调用一次，会删掉冗余dev key记录
void DevRecordTidy(void)
{
	DEVICE_RECORD *pDev=(void *)Q_Zalloc(sizeof(DEVICE_RECORD));
	KEYS_SET_DEV *pKeysSet=&(pDev->Record.KeysSet);
	INFO_IDX Idx=1;
	u16 Res=1;
	u16 InfoNum=0;
	u16 i;
	bool BurnFlag=FALSE;
	
	InfoNum=GetTypeInfoTotal(IFT_DEV);
	
	for(Idx=1;Idx<=InfoNum;Idx++)
	{
		MemSet(pDev,0,sizeof(DEVICE_RECORD));
		Res=ReadInfoByIdx(IFT_DEV,Idx,pDev);
		if(Res && pDev->ID)
		{
			switch(pDev->ProdID)
			{
				case PID_IR_KEYS:
				case PID_RF_KEYS:
					for(i=0;i<KEYS_SET_MAX_KEY_NUM;i++)
					{						
						if(pKeysSet->KeyList[i]==0xff && pKeysSet->SigList[i]==0xffff){continue;}
						else if(pKeysSet->KeyList[i] && pKeysSet->SigList[i])
						{
							INFO_TYPE Type=(pDev->ProdID==PID_IR_KEYS?IFT_IR_DATA:IFT_RF_DATA);
							u16 GID=(pDev->ProdID==PID_IR_KEYS?GOT_IR_SIGNAL:GOT_RF);							
							
							if(ReadInfoByID(Type,HL16_U32(GID,pKeysSet->SigList[i]),NULL)==0)
							{
								pKeysSet->KeyList[i]=0;
								pKeysSet->SigList[i]=0;
								BurnFlag=TRUE;
							}
						}
						else	if((pKeysSet->KeyList[i] && pKeysSet->SigList[i]==0) || (pKeysSet->KeyList[i]==0 && pKeysSet->SigList[i]))
						{
							pKeysSet->KeyList[i]=0;
							pKeysSet->SigList[i]=0;
							BurnFlag=TRUE;
						}
					}
					break;
			}

			if(BurnFlag) CoverInfo(IFT_DEV,pDev);			
		}
	}

	Q_Free(pDev);
}




