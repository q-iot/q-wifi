#ifndef APP_CLIENT_LIST_H
#define APP_CLIENT_LIST_H


#define MAX_APP_CLIENT_NUM 4 //允许4个客户端登录


enum{
	ACT_NULL=0,
	ACT_PHONE,//手机客户端
	ACT_PC,//电脑登录
	ACT_DEV,//其他主节点
	ACT_SRV,//服务器连接

	ACT_MAX
};
typedef u8 APP_CLIENT_TYPE;

enum{
	ACS_NULL=0,
	ACS_ONLINE,//在线
	ACS_ONLY,//独占，其他所有连接将被设置为ACS_NULL并踢下线

	ACS_MAX
};
typedef u8 APP_CLIENT_STATUS;

typedef struct{
	u32 AppClientID;//不同客户端appid不同，具备数据唯一性
	const void *pClient;//客户端句柄，有可能是app client，也有可能是srv
	u32 LastActiveRtc;//最后活动时间
	u16 SecretKey;//客户端密钥
	APP_CLIENT_TYPE Type;//客户端类型
	APP_CLIENT_STATUS Status;//当前状态
	u16 AppVer;//app版本号
	
	void *pNext;
}APP_CLIENT_ITEM;



void AppClientDebug(void);
void AppClientInit(void);
bool AppClientIsFull(void);
u16 AppClientRegister(u32 AppClientID,const void *pClient);
u32 AppClientGetNextAppID(u32 AppClientID);
void AppClientActive(u32 AppClientID);
void AppClientSetType(u32 AppClientID,APP_CLIENT_TYPE Type);
void AppClientSetAppVer(u32 AppClientID,u16 AppVer);
u16 AppClientGetAppVer(u32 AppClientID);
u32 AppClientGetAppID(const void *pClient);
const void *AppClientGetClientPoint(u32 AppClientID);
u16 AppClientGetSecretKey(u32 AppClientID);
u32 AppClientChkSum(u32 AppClientID,u8 *pData,u32 Len);

bool AppClientSetOnly(u32 AppClientID);
void AppClientCancleOnly(void);
const void *AppClientGetOnlyConn(void);

void AppClientOnlinePoll(void);
void AppClientEventInput(PUBLIC_EVENT Event,u32 Param,void *p);







#endif

