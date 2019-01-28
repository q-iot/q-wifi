#ifndef NOTIRY_MANGER_H
#define NOTIRY_MANGER_H


typedef enum{
	NRT_NULL=0,
	NRT_VAR_TO_APP,
	NRT_DEV_TO_APP,
	NRT_VAR_TO_WDEV,
	NRT_VAR_TO_TRIG,
	NRT_VAR_TO_TRIG_STA,
	NRT_VAR_TO_CALLBACK,

	NRT_MAX
}NOTIFY_RECV_TYPE;//关心者类型

typedef struct{
	NOTIFY_RECV_TYPE RecvType;

	u32 Param;//release var idx,trig id
	
	u32 AttentionID;//var id,dev id 
	u32 RecvID;//dev id,app id,trig pointer
	
	void *pNext;
}NOTIFY_LIST_ITEM;//关心列表


void NotifyList_Debug(void);
void NotifyList_New(u32 AttentionID,NOTIFY_RECV_TYPE RecvType,u32 RecvID,u32 Param);
void NotifyList_Delete(u32 AttentionID,NOTIFY_RECV_TYPE RecvType,u32 RecvID);
void NotifyList_CleanAll(void);
NOTIFY_LIST_ITEM *NotifyList_Read(u32 AttentionID);
void NotifyList_ReadDestory(NOTIFY_LIST_ITEM *pList); 

#define NotifyAdd_Var2App(VID,ID) NotifyList_New(HL16_U32(GOT_VARIABLE,VID),NRT_VAR_TO_APP,ID,0)
#define NotifyAdd_Dev2App(DID,ID) NotifyList_New(HL16_U32(GOT_DEVICE,DID),NRT_DEV_TO_APP,ID,0)
#define NotfiyAdd_Var2WDev(VID,ID,RelIdx) NotifyList_New(HL16_U32(GOT_VARIABLE,VID),NRT_VAR_TO_WDEV,ID,RelIdx)
#define NotifyAdd_Var2Trig(VID,ID,TrigID) NotifyList_New(HL16_U32(GOT_VARIABLE,VID),NRT_VAR_TO_TRIG,ID,TrigID)
#define NotifyAdd_Var2TrigSta(VID,ID,TrigID) NotifyList_New(HL16_U32(GOT_VARIABLE,VID),NRT_VAR_TO_TRIG_STA,ID,TrigID)
#define NotifyAdd_Var2CbFunc(VID,pFunc,Param) NotifyList_New(HL16_U32(GOT_VARIABLE,VID),NRT_VAR_TO_CALLBACK,(u32)pFunc,Param)

#define NotifyDel_Var2App(VID,ID) NotifyList_Delete(VID?HL16_U32(GOT_VARIABLE,VID):0,NRT_VAR_TO_APP,ID)
#define NotifyDel_Dev2App(DID,ID) NotifyList_Delete(DID?HL16_U32(GOT_DEVICE,DID):0,NRT_DEV_TO_APP,ID)
#define NotifyDel_Var2WDev(VID,ID) NotifyList_Delete(VID?HL16_U32(GOT_VARIABLE,VID):0,NRT_VAR_TO_WDEV,ID)
#define NotifyDel_Var2Trig(VID,ID) NotifyList_Delete(VID?HL16_U32(GOT_VARIABLE,VID):0,NRT_VAR_TO_TRIG,ID)
#define NotifyDel_Var2TrigSta(VID,ID) NotifyList_Delete(VID?HL16_U32(GOT_VARIABLE,VID):0,NRT_VAR_TO_TRIG_STA,ID)
#define NotifyDel_Var2CbFunc(VID,pFunc) NotifyList_Delete(VID?HL16_U32(GOT_VARIABLE,VID):0,NRT_VAR_TO_CALLBACK,(u32)pFunc)
#define NotifyDel_Var2All(VID) do{NotifyDel_Var2App(VID,0);NotifyDel_Var2WDev(VID,0);NotifyDel_Var2Trig(VID,0);NotifyDel_Var2TrigSta(VID,0);}while(0)



#endif

