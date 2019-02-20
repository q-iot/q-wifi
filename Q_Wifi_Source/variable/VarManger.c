#include "SysDefines.h"
//#include "UserComManger.h"
#include "InfoOperateHooks.h"

typedef struct{
	u16 Vid;
	
	VAR_TYPE Type:3;//��������
	bool ReadOnly:1;//appֻ��
	VAR_STATE VarState:2;//����״̬
	
	u8 SelfIdx;//������ͬһ�豸�Ĳ�ͬ�����Ķ�Ӧ������1��ʼ
	
	VAR_DISP_FAT DispFat:3;//��ʾ��ʽ
	u8 DotNum:5;//��ʾС������λ��

	TVAR32 OldVal;
	TVAR32 VarVal;//��ǰֵ

	CHAR_NUM_UNI ProdTag;
	CHAR_NUM_UNI VarTag;
	u32 MainDevID;//���豸id
	u32 MainDevWAddr;//���豸��������ַ
	
	void *pNext;
}VAR_LIST_ITEM;//�����б�
static VAR_LIST_ITEM *gpVarList=NULL;

typedef struct{
	u16 Vid;//���붨������λ
	VAR_STATE VarState;
	VAR_OPTOR SetorType;
	TVAR32 Value;
	u32 SetorID;
}VAR_CHANGE_INFO;//�����ı���Ϣ

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
				Debug("Dev[%u]%u\n\r",p->MainDevID,p->SelfIdx);//esp8266û��waddr
				break;
			default:
				Debug("Error Type %u\n\r",p->Type);
		}
		p=p->pNext;
	}
}

//��ǩ�Ĵ�д
void TagToH(u8 *pTag)
{
	u8 i;
	
	for(i=0;i<VAR_TAG_LEN;i++)//�����ַ���ȫ��ת��д
	{
		if(pTag[i]>='a' && pTag[i]<='z')
			pTag[i]=pTag[i]-32;
	}
}

//��ǩ������
u32 TagNum(const u8 *pTag)
{
	u8 i;
	u32 Num;
	u8 *Buf=(void *)&Num;
	
	for(i=0;i<VAR_TAG_LEN;i++)
	{
		if(pTag[i]>='a' && pTag[i]<='z') Buf[i]=pTag[i]-32;//�����ַ���ȫ��ת��д
		else Buf[i]=pTag[i];
	}

	return Num;
}

#if 1
//ע gpVarList����Ϊnull
static VAR_LIST_ITEM *VarListLast_(void)
{
	VAR_LIST_ITEM *pLast=gpVarList;

	while(pLast->pNext) pLast=pLast->pNext;

	return pLast;
}

//���ݱ���id��list���ҵ�����
//ע�⽨���ٽ���
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

//����һ���豸���б���
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

//�����ݿ��ȡ�豸��waddr
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

//����record���ݣ���������item,���浽list��
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
		pVar->VarState=VST_VALID;//esp8266����ָ�����������ñ�־��������ʶ�豸�Ƿ����ߣ�esp��Զ���ߣ���������
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
			pVar->MainDevWAddr=0;//esp8266����Ҫ��GetVarMainDevWAddrFromInfo(pVarRcd->MainDevID);
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

	if(pInfoRcd==NULL) Q_Free(pVarRcd);//��pInfoRcdΪ��ʱ��pVarRcd���Լ�����ģ���Ҫ�ͷ�
}

#if 0 //esp8266����
//���������������豸�Ĺ�ϵ
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
			
			for(i=0;i<RELATE_VAR_MAX;i++) //���ı���
			{
				if(pWav->RelateVar[i]) Debug("+Dev[%u]->Relate[%u]\n\r",pDev->ID,pWav->RelateVar[i]);
				NotfiyAdd_Var2WDev(pWav->RelateVar[i],pWav->WNetAddr,i);
			}
		}
	}

	Q_Free(pDev);
}
#endif

//��ʼ�����б���
//���ظ�����
void VarListInit(void)
{
	//��վ�����
	OS_EnterCritical();
	while(gpVarList)//�����
	{
		VAR_LIST_ITEM *pVarNext=gpVarList->pNext;
		NotifyDel_Var2All(gpVarList->Vid);
		Q_Free((void *)gpVarList);
		gpVarList=pVarNext;
	}
	OS_ExitCritical();

	//�����ݿ��ȡ��Ϣ
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
				VarCreate_ByRecord(pVarRcd->ID,NULL);//���뵽list��
			}
		}

		Q_Free(pVarRcd);
		Q_Free(pVar);
	}
	
	//���ϵͳ�����в����ڵģ����½����ݿ�
	//CheckAllSysVarHave(FALSE); 

	//���������������豸�Ĺ�ϵ
	//BuildDevVarRelation();//esp8266����Ҫ

}

//ɾ���豸�������������
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

			NotifyDel_Var2All(p->Vid);//ɾ�������豸����������Ĺ�ע
			if(NeedDebug(DFT_VAR)) Debug("-pVar:0x%x(%u)\n\r",p->Vid,p->Vid);
			if(DelFromRecord) DeleteInfo(IFT_VARIABLE,HL16_U32(GOT_VARIABLE,p->Vid));//��recordɾ��
			Q_Free(p);	
			if(p==gpVarList) gpVarList=NULL;

			//����ѭ��
			p=pPre?pPre->pNext:gpVarList;
			continue;
		}

		pPre=p;
		p=p->pNext;
	}
	OS_ExitCritical();
}

//�����ܳ�ͻ��ProdTagת��Ϊ����ͻ��tag����
u32 ConvertOnlyTags(u32 TagNum)
{
	VAR_LIST_ITEM *p=NULL;
	u8 *pTag=(u8 *)&TagNum;
	
	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(TagNum == p->ProdTag.Num)//�ҵ�һ������
		{
			if(pTag[3]<'1' || pTag[3]>'9')//is letter
			{
				pTag[3]='1';
			}
			else 
			{
				pTag[3]++;
			}
			p=gpVarList;//��Ϊ�ı���tag������Ҫ��ͷ��ʼ�ȶ��ظ�
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
//����vid��ȡ�������豸id����������ַ
//�������жϱ�����Ӧ���豸�Ƿ����
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

//����vid��ȡ��������������ַ����������
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

#if 0 //esp8266����
//�����������������ȡvid
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

//���ݹ�������������ȡvid
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

//ͨ��tag�����ر���id
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

//ͨ��tag�����ر���id
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

//ͨ���豸info id�ͱ������������ر���id
u16 FindVarId_ByDev(u32 DevID,u8 Idx,VAR_IDX_T VarIdxType)
{
	u16 Vid=0;
	
	if(DevID==0 || Idx==0) return 0;

	if(VarIdxType==VIT_RELATE) //��������
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
	else if(VarIdxType==VIT_SELF) //�������
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
//�����޸���Ϻ�Ĺ��Ӻ���
static void VarSetFinish_Hook(VAR_CHANGE_INFO *pInfo,u16 InfoNum)
{
	for(;InfoNum;InfoNum--,pInfo++)
	{
		if(NeedDebug(DFT_VAR)){if(pInfo->Vid) Debug("VarSet:[%u]=%d\n\r",pInfo->Vid,pInfo->Value);}
		if(pInfo->Vid) VarChangeInform(HL16_U32(GOT_VARIABLE,pInfo->Vid),pInfo->Value,pInfo->VarState,pInfo->SetorType,pInfo->SetorID);
		if(pInfo->VarState==VST_VALID) UpdateIoStateByVar(pInfo->Vid,pInfo->Value);//ͬ��gpio
	}
}

//ʹ����ϵͳ������Ч������߳����������
void VaildAllSysVars(void)
{
	VAR_CHANGE_INFO *pInfoList=NULL;
	VAR_LIST_ITEM *p=NULL;
	u16 VarNum=0;

	//�������
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

	//��ʽ����
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

//�豸���ߺ󣬽���Ӧ�ı�������Ϊ��Ч
//ֻ���������Լ�������
void SetVarInvaild_ByDev(u32 WDevAddr)
{
	VAR_CHANGE_INFO *pInfoList=NULL;
	VAR_LIST_ITEM *p=NULL;
	u16 VarNum=0;
	
	if(WDevAddr==0) return;

	//�������
	OS_EnterCritical();
	p=gpVarList;
	while(p)
	{
		if(p->Type==VAR_T_DEV && p->VarState==VST_VALID && p->MainDevWAddr==WDevAddr)//�ҵ�������
		{
			VarNum++;
		}
		p=p->pNext;
	}
	
	//��ʽ����
	if(VarNum)
	{
		pInfoList=Q_Zalloc(VarNum*sizeof(VAR_CHANGE_INFO));
		p=gpVarList;
		while(p)
		{
			if(p->Type==VAR_T_DEV && p->VarState==VST_VALID && p->MainDevWAddr==WDevAddr)//�ҵ�������
			{			
				p->VarState=VST_NULL;
				
				pInfoList->Vid=p->Vid;
				pInfoList->VarState=p->VarState;
				pInfoList->Value=p->VarVal;
				pInfoList->SetorType=VRT_WDEV;//��װ���豸�����ģ������ͻ᲻��֪ͨ�豸�Լ�
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

//���ñ�����Ч���ڲ�ʹ��
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
		if(p->Vid==Vid && p->VarState==VST_NULL)//�ҵ�������
		{			
			p->VarState=VST_VALID;
			Res=VRES_SUCESS;

			ChgInfo.Vid=p->Vid;
			ChgInfo.VarState=p->VarState;
			ChgInfo.Value=p->VarVal;
			ChgInfo.SetorType=VRT_WDEV;//��װ���豸�����ģ������ͻ᲻��֪ͨ�豸�Լ�
			ChgInfo.SetorID=p->MainDevWAddr;
			break;
		}

		p=p->pNext;
	}
	OS_ExitCritical();

	if(ChgInfo.Vid) VarSetFinish_Hook(&ChgInfo,1);

	return Res;
}

//�ڲ�ʹ�ã�����ture��ʾ�޸ĳɹ�
//SetorID:���ӵ�app client id�����豸��wnet addr
static VAR_CHG_RES SetVarVal_(VAR_LIST_ITEM *p,TVAR32 Val,VAR_OPTOR SetorType,u32 SetorID)
{
	VAR_CHG_RES Ret=VRES_FAILD;

	if(Val==p->VarVal && p->VarState==VST_VALID) Ret=VRES_NOCHANGE;

	switch(SetorType)
	{
		case VRT_SYS://�����Լ��ı��
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
		case VRT_WDEV://dev�ı��
			if(p->Type==VAR_T_DEV?p->MainDevWAddr==SetorID:TRUE) //wdevֻ�ܸ�ϵͳ�������û������������Լ����豸����
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
		case VRT_APP://app�ı�ı���
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

//���ñ���ֵ
//SetorID:���ӵ�app client id�����豸��wnet addr
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
		if(p->Vid==Vid)//�ҵ�������
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

//��λ�����豸����
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
		if(p->Vid==Vid)//�ҵ�������
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

//��λ��ת�豸����
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
		if(p->Vid==Vid)//�ҵ�������
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

//����������
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
		if(p->Vid==Vid)//�ҵ�������
		{			
			if(p->VarVal!=0xffff) Val=p->VarVal+1;//���ӵ�0xffff
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

//�����Լ���
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
		if(p->Vid==Vid)//�ҵ�������
		{			
			if(p->VarVal) Val=p->VarVal-1;//������0
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

//��ȡ����tag�����û�ҵ�������false
//ע�⣬���ص�tag����������
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

//��ȡ����tag�����û�ҵ�������false
//ע�⣬���ص�tag�������������Կռ�Ҫ���һ���ֽ�
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

//��ȡ����ֵ��״̬
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

//��ȡ����ֵ��״̬
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

#if 0 //esp8266����
//���豸����һ����ʧЧʱ������false�����򷵻�true
//DevID��WAddr������һ��ֵ����
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
//��list��ɾ��һ������
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

//����wav�豸���������Ӷ�Ӧ�ı�������������
static void UpdateVars_NewDev(u32 DevID)
{
	DEVICE_RECORD *pDev=Q_Zalloc(sizeof(DEVICE_RECORD));
	u16 Res=0;
	
	Res=ReadInfoByID(IFT_DEV,DevID,pDev);
	if(Res && ProdIsWav(pDev->ProdID))
	{
		//�����������
		{
			VARIABLE_RECORD *pVarRcd=Q_Zalloc(sizeof(VARIABLE_RECORD));
			INFO_IDX Idx=1;
			u16 vRes=1;

			while(vRes)
			{
				vRes=ReadInfoByIdx(IFT_VARIABLE,Idx++,pVarRcd);
				if(vRes && pVarRcd->ID && pVarRcd->MainDevID == LBit16(pDev->ID))//�����������
				{
					VarCreate_ByRecord(pVarRcd->ID,pVarRcd);//���뵽list��
				}
			}

			Q_Free(pVarRcd);
		}

#if 0	//���Ĺ������� //esp8266����Ҫ
		{
			WAVER_DEV *pWav=&pDev->Record.Waver;
			u16 i=0;

			for(i=0;i<RELATE_VAR_MAX;i++)//���ı���
			{
				NotfiyAdd_Var2WDev(pWav->RelateVar[i],pWav->WNetAddr,i);							
			}
		}
#endif
	}

	Q_Free(pDev);
}

//ɾ��wav�豸���б���������������ϵ
//NeedDelInfoΪtrueʱ��ɾ������record info
static void UpdateVars_DeleteDev(u32 DevID)
{
	DEVICE_RECORD *pDev=NULL;

	if(DevID==0) return;

	pDev=Q_Zalloc(sizeof(DEVICE_RECORD));
	
	if(ReadInfoByID(IFT_DEV,DevID,pDev))
	{
		if(ProdIsWav(pDev->ProdID))
		{
			VarsDelete_ByMainDev(pDev->ID,TRUE);//ɾ���������		
		}
	}

	Q_Free(pDev);
}

//����ϵͳ�¼�
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







