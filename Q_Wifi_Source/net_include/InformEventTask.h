#ifndef INFORM_EVENT_TASK
#define INFORM_EVENT_TASK

bool VarChangeInform(u32 VarID,TVAR32 Val,VAR_STATE State,VAR_OPTOR SetorType,u32 SetorID);
bool StrChangeInform(u32 DevID,u32 StrID,const char *pStr);
bool DevChangeInform(u32 DevID,bool IsOnline);

bool SendSysMsgToDev(u32 DevID,void *pStr);

void InformEventTaskWalk(void);
void InformEventTask(void *pvParameters);

#endif

