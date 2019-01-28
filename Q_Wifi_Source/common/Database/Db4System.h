#ifndef DB_FOR_SYS_H
#define DB_FOR_SYS_H

#include "LimitMarco.h"

typedef enum {//4因为牵涉到app，qwifi和future必须统一，且新值只能加到队尾
	SIN_NULL=0,
	SIN_SN,//本机硬件ID,只读
	SIN_SoftVer,//软件版本，只读
	SIN_HwVer,//硬件版本，只读
	SIN_IpAddr,//本机ip
	SIN_SubMask,//掩码
	SIN_Gataway,//网关
	SIN_Port,//端口
	SIN_DnsIp,//dns
	SIN_BackupDnsIp,//备用dns
	SIN_ServerURL=10,//服务器
	SIN_JsonServerURL,//备用服务器
	SIN_ServerPort_xxx,//远程端口，被包含在路径中，所以废弃
	SIN_WnetAddr,//wnet 地址
	SIN_SysIdBase,//系统id基数
	SIN_SysConfigFlag,//配置标志，供pc用，仅返回0
	SIN_UserName,//用户名
	SIN_UserPwHash,//密码hash值
	SIN_NeedSrvTrans,//是否需要中转
	SIN_RssiThred,//信号阀值
	SIN_RfTrans=20,//转发器地址
	SIN_GsmSupport,//是否支持GSM
	SIN_CityCode,//城市id
	SIN_InfoConfigFlag,//info配置标识，返回16个数
	SIN_UcomEnBits,//用户接口开启与否
	SIN_LcdVars,//上屏变量
	SIN_LcdVarsName,//上屏变量名称
	SIN_OpenBeep,//开启蜂鸣器
	SIN_WNetChannel,//万纳网频段
	SIN_ZigbeeChannel,//zigbee频道
	SIN_ZigbeeGroup=30,//zigbee网络号，组地址
	SIN_DutRegSn,//板卡注册序列号
	SIN_DataUpPeriod,//上报周期的索引
	
	SIN_MAX
}SYS_ITEM_NAME;

typedef struct{	
	u32 Flag;//读取标志，无需用户干预
	u32 Ver;//版本，无需用户干预
	u32 ChkSum;//校验和，无需用户干预
	
	//用户数据区域
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
	u32 DutRegSn;//板卡注册序列号
	u32 CityCode;//城市编码

	u8 DataUpPeriodIdx;//数据上报周期，由命令控制
	u8 OpenBeep;
	u8 UcomEnBits;//串口初始化标识，每bit代表一个UCOM_ID
	
	u16 LcdDispVar[F_LCD_VAR_DISP_NUM];
	u8 LcdDispVarName[F_LCD_VAR_DISP_NUM][SHORT_STR_BYTES];
	
	u16 WNetChannel;//wnet频道
	u16 ZigbeeChannal;//zigbee频道
	u16 ZigbeeGroup;//zigbee组
	
	u32 Tail;//兼容spi fls write 4bytes align
}SYS_DB_STRUCT;	//数据库

typedef enum{
	SCS_OFFLINE=0,//初始
	SCS_FOUND_SRV,//域名解析完成
	SCS_ONLINE,//tcp连上
	SCS_HAS_LOGIN,//登录成功

	SCS_MAX
}SRV_CONN_STATUS;//连接服务器的状态

typedef struct{
	u8 ServerIp[4];//dns解析后的服务器ip
	u8 JsonSrvIp[4];//dns解析后的json服务器ip
	u16 SrvSecretKey;//密钥
	SRV_CONN_STATUS SrvConnStaus;//服务器连接状态
	SRV_CONN_STATUS JsonConnStaus;//大数据服务器连接状态
	void *SrvConnPoint;//服务器连接，未叠加掩码标记的指针
	void *JsonConnPoint;//大数据服务器连接，未叠加掩码标记的指针
	
	u16 VendorID;//厂家ID
	bool IsBackupSrv;//当前连接是否是备用服务器
	u32 LastVaildBeatTime;//有效beat时间
	bool OpenDhcp;//是否开了dhcp
	bool DhcpFinish;//是否完成了dhcp获取
	u8 SiRssi;//si4432信号指示
	u8 StopDataUp;//是否停止上报，由回复的报文控制
	u8 VarMinPeriodIdx;//变量上报的最小周期，由所有变量上报计算得到
	
	bool RtcRstFlag:1;//是否重启了后备电池
	bool DutRegSnIsOk:1;//注册授权码是否正确
	bool SupportLCD:1;//支持lcd显示
	bool SwapUartOut:1;//是否切换uart0到io13 io15
	bool FinishRtcSync:1;//是否完成了服务器的对时
	
	u32 SysStartRtc;//系统启动绝对时间
	bool RfSendBySi;//用si4432发送普通射频
	u32 WDevSyncCnt;//万纳网同步包计数
}SYS_VAR_STRUCT;//系统变量

void Sys_DbDebug(u8 Flag);
void Sys_DbInit(void);
u32 Sys_GetValue(u16 Item,u32 IntParam,void *Val);
bool Sys_SetValue(u16 Item,u32 IntParam,void *pParam,u16 ByteLen);
void Sys_Default(void);

SYS_VAR_STRUCT *SysVars(void);

#endif

