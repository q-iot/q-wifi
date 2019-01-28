#ifndef JSON_DATA_IN_OUT_H
#define JSON_DATA_IN_OUT_H

void JsonConnSendBeat(void);
void JsonConnSendString(u32 StrID,const char *pStr);
void JsonConnSendMsg(const char *pMsg,const char *pUser,u8 MsgFlag);
cJSON *JsonConnAddItem(cJSON *pRoot,u32 VarID,u32 PeriodMs);
void JsonConnSendVars(cJSON *pRoot);


#endif
