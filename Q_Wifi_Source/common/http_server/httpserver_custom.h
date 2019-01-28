#ifndef HTTP_SERVER_USER_H
#define HTTP_SERVER_USER_H

bool http_get_user_val(const char *pVarStr,char *pOutStr);
bool http_get_user_handler(NET_CONN_T *pConn,const char *pUrl);
const u8 *http_post_user_handler(const char *pUrl,u16 ParamNum,const char **pParam,const char **pVal);



#endif

