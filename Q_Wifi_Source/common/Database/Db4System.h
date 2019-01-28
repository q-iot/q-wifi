#ifndef DB_FOR_SYS_H
#define DB_FOR_SYS_H

#include "LimitMarco.h"

typedef enum {//4��Ϊǣ�浽app��qwifi��future����ͳһ������ֵֻ�ܼӵ���β
	SIN_NULL=0,
	SIN_SN,//����Ӳ��ID,ֻ��
	SIN_SoftVer,//����汾��ֻ��
	SIN_HwVer,//Ӳ���汾��ֻ��
	SIN_IpAddr,//����ip
	SIN_SubMask,//����
	SIN_Gataway,//����
	SIN_Port,//�˿�
	SIN_DnsIp,//dns
	SIN_BackupDnsIp,//����dns
	SIN_ServerURL=10,//������
	SIN_JsonServerURL,//���÷�����
	SIN_ServerPort_xxx,//Զ�̶˿ڣ���������·���У����Է���
	SIN_WnetAddr,//wnet ��ַ
	SIN_SysIdBase,//ϵͳid����
	SIN_SysConfigFlag,//���ñ�־����pc�ã�������0
	SIN_UserName,//�û���
	SIN_UserPwHash,//����hashֵ
	SIN_NeedSrvTrans,//�Ƿ���Ҫ��ת
	SIN_RssiThred,//�źŷ�ֵ
	SIN_RfTrans=20,//ת������ַ
	SIN_GsmSupport,//�Ƿ�֧��GSM
	SIN_CityCode,//����id
	SIN_InfoConfigFlag,//info���ñ�ʶ������16����
	SIN_UcomEnBits,//�û��ӿڿ������
	SIN_LcdVars,//��������
	SIN_LcdVarsName,//������������
	SIN_OpenBeep,//����������
	SIN_WNetChannel,//������Ƶ��
	SIN_ZigbeeChannel,//zigbeeƵ��
	SIN_ZigbeeGroup=30,//zigbee����ţ����ַ
	SIN_DutRegSn,//�忨ע�����к�
	SIN_DataUpPeriod,//�ϱ����ڵ�����
	
	SIN_MAX
}SYS_ITEM_NAME;

typedef struct{	
	u32 Flag;//��ȡ��־�������û���Ԥ
	u32 Ver;//�汾�������û���Ԥ
	u32 ChkSum;//У��ͣ������û���Ԥ
	
	//�û���������
	u16 SoftVer;
	u16 HwVer;
	u8 IpAddr[4];
	u8 SubMask[4];
	u8 Gataway[4];
	u16 Port;
	u8 DnsIp[4];
	u8 BackupDnsIp[4];
	u8 ServerURL[NORMAL_STR_BYTES];
	u8 JsonServerURL[LONG_STR_BYTES];
	u16 RfTransID;
	u32 UserPwHash;
	u8 NeedSrvTrans;
	u8 RssiThred;
	u32 DutRegSn;//�忨ע�����к�
	u32 CityCode;//���б���

	u8 DataUpPeriodIdx;//�����ϱ����ڣ����������
	u8 OpenBeep;
	u8 UcomEnBits;//���ڳ�ʼ����ʶ��ÿbit����һ��UCOM_ID
	
	u16 LcdDispVar[F_LCD_VAR_DISP_NUM];
	u8 LcdDispVarName[F_LCD_VAR_DISP_NUM][SHORT_STR_BYTES];
	
	u16 WNetChannel;//wnetƵ��
	u16 ZigbeeChannal;//zigbeeƵ��
	u16 ZigbeeGroup;//zigbee��
	
	u32 Tail;//����spi fls write 4bytes align
}SYS_DB_STRUCT;	//���ݿ�

typedef enum{
	SCS_OFFLINE=0,//��ʼ
	SCS_FOUND_SRV,//�����������
	SCS_ONLINE,//tcp����
	SCS_HAS_LOGIN,//��¼�ɹ�

	SCS_MAX
}SRV_CONN_STATUS;//���ӷ�������״̬

typedef struct{
	u8 ServerIp[4];//dns������ķ�����ip
	u8 JsonSrvIp[4];//dns�������json������ip
	u16 SrvSecretKey;//��Կ
	SRV_CONN_STATUS SrvConnStaus;//����������״̬
	SRV_CONN_STATUS JsonConnStaus;//�����ݷ���������״̬
	void *SrvConnPoint;//���������ӣ�δ���������ǵ�ָ��
	void *JsonConnPoint;//�����ݷ��������ӣ�δ���������ǵ�ָ��
	
	u16 VendorID;//����ID
	bool IsBackupSrv;//��ǰ�����Ƿ��Ǳ��÷�����
	u32 LastVaildBeatTime;//��Чbeatʱ��
	bool OpenDhcp;//�Ƿ���dhcp
	bool DhcpFinish;//�Ƿ������dhcp��ȡ
	u8 SiRssi;//si4432�ź�ָʾ
	u8 StopDataUp;//�Ƿ�ֹͣ�ϱ����ɻظ��ı��Ŀ���
	u8 VarMinPeriodIdx;//�����ϱ�����С���ڣ������б����ϱ�����õ�
	
	bool RtcRstFlag:1;//�Ƿ������˺󱸵��
	bool DutRegSnIsOk:1;//ע����Ȩ���Ƿ���ȷ
	bool SupportLCD:1;//֧��lcd��ʾ
	bool SwapUartOut:1;//�Ƿ��л�uart0��io13 io15
	bool FinishRtcSync:1;//�Ƿ�����˷������Ķ�ʱ
	
	u32 SysStartRtc;//ϵͳ��������ʱ��
	bool RfSendBySi;//��si4432������ͨ��Ƶ
	u32 WDevSyncCnt;//������ͬ��������
}SYS_VAR_STRUCT;//ϵͳ����

void Sys_DbDebug(u8 Flag);
void Sys_DbInit(void);
u32 Sys_GetValue(u16 Item,u32 IntParam,void *Val);
bool Sys_SetValue(u16 Item,u32 IntParam,void *pParam,u16 ByteLen);
void Sys_Default(void);

SYS_VAR_STRUCT *SysVars(void);

#endif

