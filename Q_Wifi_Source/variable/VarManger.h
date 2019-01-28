#ifndef VARIABLE_MANGER_H
#define VARIABLE_MANGER_H

enum{
	VRT_NULL=0,
	VRT_WDEV,
	VRT_APP,
	VRT_SYS,
	VRT_TRIG,
	
};
typedef u8 VAR_OPTOR;//变量操作者类型

typedef struct{
	VAR_OPTOR Type;
	u8 RelIdx;// rel var idx 1-4, or trig state flag
	u16 Param;//trig id
	u32 ID;//app id,wdev addr,trig item point
	void *pNext;
}VAR_REL_INFO;//关心信息

enum{
	VRES_FAILD=0,
	VRES_SUCESS,
	VRES_NOCHANGE
};
typedef u8 VAR_CHG_RES;//改变变量的结果

void TagToH(u8 *pTag);
u32 TagNum(const u8 *pTag);

void VarListInit(void);

u32 ConvertOnlyTags(u32 TagNum);
VAR_TYPE GetVarType(u16 Vid);
u32 GetVarMainDevID(u16 Vid,u32 *pWAddr);
u32 GetVarMainDevWAddr(u16 Vid,u8 *pSelfIdx);
u16 GetVarVid_ByWAddr(u32 MainWAddr,u8 SelfIdx);
u16 GetRelVarVid_ByWAddr(u32 MainWAddr,u8 RelIdx);
VAR_DISP_FAT GetVarDispFat(u16 Vid,u8 *pDotNum);

u16 FindVarId_ByTag(const u8 *pTagStr);
u16 FindVarId_ByTag2(const u8 *pProdTagStr,const u8 *pVarTagStr);
u16 FindVarId_ByDev(u32 DevID,u8 Idx,VAR_IDX_T VarIdxType);

void VaildAllSysVars(void);
void SetVarInvaild_ByDev(u32 WDevAddr);
VAR_CHG_RES SetVarVaild(u16 Vid);
VAR_CHG_RES SetVarVal(u16 Vid,TVAR32 Val,VAR_OPTOR SetorType,u32 SetorID);
VAR_CHG_RES SetVarBit(u16 Vid,u8 Bit,u8 BitVal,VAR_OPTOR SetorType,u32 SetorID);
VAR_CHG_RES RevVarBit(u16 Vid,u8 Bit,VAR_OPTOR SetorType,u32 SetorID);
VAR_CHG_RES IncVar(u16 Vid,VAR_OPTOR SetorType,u32 SetorID);
VAR_CHG_RES DecVar(u16 Vid,VAR_OPTOR SetorType,u32 SetorID);

bool GetVarProdTag(u16 Vid,u8 *pProdTag,u8 *pVarTag);
bool GetVarProdTag2(u16 Vid,u8 *pTags);
VAR_STATE GetVarState(u16 Vid,TVAR32 *pVal);
TVAR32 GetVarValue(u16 Vid,VAR_STATE *pState);

bool CheckDevVarAllVaild(u32 DevID,u32 WAddr);

void VarManEventInput(PUBLIC_EVENT Event,u32 Param,void *p);


#endif
