#ifndef TRIGGER_MANGER_H
#define TRIGGER_MANGER_H

void TrigListDebug(void);
void TrigListInit(void);
void TrigEnable_ByScn(ID_T SceneID);
void TrigDisable_ByScn(ID_T SceneID);

//------------------------------------------´¥·¢½Ó¿Ú-------------------------------------------------
void TrigIn_Rf(u32 Code,u16 BasePeriod);
void TrigIn_SysMsg(const char *pMsg);
void TrigIn_VarPoll(void);
void TrigIn_Var(u16 TrigID,void *pItem,u16 Vid);
void TrigStateVarChange(u16 TrigID,void *pItem,u16 Vid);




#endif
