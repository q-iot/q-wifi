#ifndef INFO_OPERATE_HOOK_H
#define INFO_OPERATE_HOOK_H

ID_T InfoIdAutoConfig(INFO_TYPE InfoType,u8 *pData,bool *pNeedDelete);
bool CheckInfoVaild(INFO_TYPE InfoType,u8 *pData);
void InfoReadHook(INFO_TYPE Name,ID_T InfoID,void *pInfo);
void InfoDeleteHook(INFO_TYPE Name,ID_T InfoID,bool IsModify);
bool InfoSaveBeforeHook(INFO_TYPE Name,ID_T InfoID,void *pInfo,INFO_PKT_RESP *pResp);
void InfoSaveAfterHook(INFO_TYPE Name,ID_T InfoID,void *pInfo);


#endif

