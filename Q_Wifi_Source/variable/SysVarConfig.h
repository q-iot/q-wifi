#ifndef SYS_VAR_CONFIG_H
#define SYS_VAR_CONFIG_H

void SetSysVarHave(u32 ProdTag,u32 VarTag);
void CheckAllSysVarHave(bool ForceCreatAll);
void SysVarPeriodicitySet(bool SecEvt);
void Fetch2VarFormString(const char *pMsg);


#endif

