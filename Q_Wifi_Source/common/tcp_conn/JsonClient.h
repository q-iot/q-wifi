#ifndef JSON_CLIENT_H
#define JSON_CLIENT_H

typedef enum{
	JEN_NULL=0,
	JEN_UP,
	
}JSON_EVENT_NAME;

void JsonClient_Init(void);
void DeleteJsonConn(void);
void UpJsonDataToSrv(const u8 *pPath,u8 *pJsonStr,pStdFunc JsonSentCb);

#define cJSON_Free Q_Free

#endif
