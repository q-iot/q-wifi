#ifndef APP_CLIENT_LIST_H
#define APP_CLIENT_LIST_H


#define MAX_APP_CLIENT_NUM 4 //����4���ͻ��˵�¼


enum{
	ACT_NULL=0,
	ACT_PHONE,//�ֻ��ͻ���
	ACT_PC,//���Ե�¼
	ACT_DEV,//�������ڵ�
	ACT_SRV,//����������

	ACT_MAX
};
typedef u8 APP_CLIENT_TYPE;

enum{
	ACS_NULL=0,
	ACS_ONLINE,//����
	ACS_ONLY,//��ռ�������������ӽ�������ΪACS_NULL��������

	ACS_MAX
};
typedef u8 APP_CLIENT_STATUS;

typedef struct{
	u32 AppClientID;//��ͬ�ͻ���appid��ͬ���߱�����Ψһ��
	const void *pClient;//�ͻ��˾�����п�����app client��Ҳ�п�����srv
	u32 LastActiveRtc;//���ʱ��
	u16 SecretKey;//�ͻ�����Կ
	APP_CLIENT_TYPE Type;//�ͻ�������
	APP_CLIENT_STATUS Status;//��ǰ״̬
	u16 AppVer;//app�汾��
	
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

