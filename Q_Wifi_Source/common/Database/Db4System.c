#include "SysDefines.h"
//#include "SecretRun.h"

const SYS_DB_STRUCT gDefSysDb={
0,0,0,//无需用户干预的版本及标志信息

//用户数据
QWIFI_SOFT_VER,//u16 SIN_SoftVer;如果需要强制清数据，增加此值
0,//u16 SIN_HwVer;
{0,0,0,0},//u8 SIN_IpAddr[4];
{0,0,0,0},//u8 SIN_SubMask[4];
{0,0,0,0},//u8 SIN_Gataway[4];
5800,//u16 SIN_Port;
{8,8,8,8},//u8 DnsIp[4];
{8,8,4,4},//u8 BackupDnsIp[4];
{'s','r','v','.','q','-','i','o','t','.','c','n',0},	//u8 ServerURL[32];
//{'1','9','2','.','1','6','8','.','2','0','.','1','1','0',0},
//{'q','-','s','h','a','r','e','.','3','3','2','2','.','o','r','g',0},
{'u','p','.','q','-','i','o','t','.','c','n','/','b','i','g',0},//u8 JsonServerURL[32];
0,//u16 RfTranID;
0,//u32 UserPwHash;
1,//u8 NeedSrvTrans;
100,//u8 RssiThred;rssi阀值
0,//u32 DutRegSn;//板卡注册序列号
0,//u32 CityCode;//城市编码
FALSE,//u8 DataUpEnable;//是否上报json数据
FALSE,//u8 OpenBeep
0xff,//	u8 UcomEnBits;//串口初始化标识，每bit代表一个UCOM_ID
{0,0,0,0},//u16 LcdDispVar[F_LCD_VAR_DISP_NUM];
{{0},{0},{0},{0}},//u8 LcdDispVarName[F_LCD_VAR_DISP_NUM][SHORT_STR_BYTES];
100,//u16 WNetChannel;//wnet频道，不能大于等于50，否则用默认433.5
25,//u16 ZigbeeChannal;//zigbee频道
0x1001,//u16 ZigbeeGroup;//zigbee组
};	
	

SYS_DB_STRUCT gSysDb={0,0,0};//系统数据库
static SYS_VAR_STRUCT gSysVars={//系统变量库
{0,0,0,0},//server ip
{0,0,0,0},//json server ip
0,//secret key
SCS_OFFLINE,//srv connect status
SCS_OFFLINE,//JsonConnStaus
NULL,//SrvConn
NULL,//JsonConn

0xff,//厂家ID
FALSE,//is backup srv
0,//LastVaildBeatTime
FALSE,//OpenDhcp
FALSE,//DhcpFinish
0,//u8 SiRssi;//si4432信号指示
0,//u8 StopDataUp;//json上报的最小时间周期
0,//	u8 VarMinPeriod;//变量上报的最小计算周期

FALSE,//bool RtcRstFlag;//是否重启了后备电池
FALSE,//bool DutRegSnIsOk;//注册授权码是否正确
FALSE,//bool SupportLCD:1;//支持lcd显示
FALSE,//bool SwapUartOut:1;//是否切换uart0到io13 io15
FALSE,//bool FinishRtcSync:1;//是否完成了服务器的对时

0,//系统启动绝对时间
FALSE,//用si发送射频
0,//WDevSyncCnt
};

#define Frame() //Debug("                                                                              |\r");
void Sys_DbDebug(u8 Flag)
{
	u16 i;
	
	Debug("  ----------------------------------------------------------------\n\r");

	if(Flag==0)
	{
		Frame();Debug("  |SnHash:%u\n\r",GetHwID());
#if ADMIN_DEBUG
		Frame();Debug("  |SoftVer:%u.%u(*)\n\r",__gBinSoftVer,RELEASE_DAY);
#else
		Frame();Debug("  |SoftVer:%u.%u\n\r",__gBinSoftVer,RELEASE_DAY);
#endif
		Frame();Debug("  |HwVer:%u %s\n\r",gSysDb.HwVer,SysVars()->SupportLCD?"OLED":"");
		if(gSysDb.IpAddr[0]*gSysDb.IpAddr[1]*gSysDb.IpAddr[2]*gSysDb.IpAddr[3])//非自动获取
		{
			Frame();Debug("  |Ip:%u.%u.%u.%u\n\r",gSysDb.IpAddr[0],gSysDb.IpAddr[1],gSysDb.IpAddr[2],gSysDb.IpAddr[3]);
			Frame();Debug("  |Mask:%u.%u.%u.%u\n\r",gSysDb.SubMask[0],gSysDb.SubMask[1],gSysDb.SubMask[2],gSysDb.SubMask[3]);
			Frame();Debug("  |Gataway:%u.%u.%u.%u\n\r",gSysDb.Gataway[0],gSysDb.Gataway[1],gSysDb.Gataway[2],gSysDb.Gataway[3]);	
			Frame();Debug("  |Dns:%u.%u.%u.%u\n\r",gSysDb.DnsIp[0],gSysDb.DnsIp[1],gSysDb.DnsIp[2],gSysDb.DnsIp[3]);
			Frame();Debug("  |Dns2:%u.%u.%u.%u\n\r",gSysDb.BackupDnsIp[0],gSysDb.BackupDnsIp[1],gSysDb.BackupDnsIp[2],gSysDb.BackupDnsIp[3]);		
		}
		else
		{
			u8 Ip[5][4];
			Lwip_GetNetInfo((void *)Ip);
			Frame();Debug("  |Ip:%u.%u.%u.%u(DHCP)\n\r",	Ip[0][0],Ip[0][1],Ip[0][2],Ip[0][3]);
			Frame();Debug("  |Mask:%u.%u.%u.%u\n\r",			Ip[1][0],Ip[1][1],Ip[1][2],Ip[1][3]);
			Frame();Debug("  |Gataway:%u.%u.%u.%u\n\r",		Ip[2][0],Ip[2][1],Ip[2][2],Ip[2][3]);		
			Frame();Debug("  |Dns:%u.%u.%u.%u\n\r",			Ip[3][0],Ip[3][1],Ip[3][2],Ip[3][3]);
			Frame();Debug("  |Dns2:%u.%u.%u.%u\n\r",			Ip[4][0],Ip[4][1],Ip[4][2],Ip[4][3]);
		}
		Frame();Debug("  |DutPort:%u\n\r",gSysDb.Port);
		Frame();Debug("  |Srv:%s(default port 8500)\n\r",gSysDb.ServerURL);
		Frame();Debug("  |JsonSrv:%s %s V%u DB%u = %u\n\r",gSysDb.JsonServerURL,	SysVars()->StopDataUp?"OFF":"UP",SysVars()->VarMinPeriodIdx,gSysDb.DataUpPeriodIdx,GetVarsJsonUpPeriodIdx());
		Frame();Debug("  |IdBase:%u\n\r",ObjIdBase());
		Frame();Debug("  |ConfigFlag:%u\n\r",GetSysConfigFlag());
		//Frame();Debug("  |RfTrans:%u\n\r",gSysDb.RfTransID);
		Frame();Debug("  |Def Pw:%u(Now Hash:0x%x)\n\r",GenerateDefPw(),gSysDb.UserPwHash);
		Frame();Debug("  |DutRegSn:%u(%s)\n\r",gSysDb.DutRegSn,SysVars()->DutRegSnIsOk?"OK":"ERR");
		//Frame();Debug("  |NeedSrvTrans:%u\n\r",gSysDb.NeedSrvTrans);
		//Frame();Debug("  |RssiThreshold:%u\n\r",gSysDb.RssiThred);
		Frame();Debug("  |City:%u\n\r",gSysDb.CityCode);
	}
	
	if(Flag==1)
	{
		for(i=0;i<F_LCD_VAR_DISP_NUM;i++)
		{
			Frame();Debug("  |LCD VAR [%u]:%s\n\r",gSysDb.LcdDispVar[i],gSysDb.LcdDispVarName[i]);
		}
	}
	
	Debug("  ------------------------------------------------------------------------\n\r");
}

//系统初始化时调用
//运行于系统读出数据库后，用户程序之前
//用于设置一些无法保存到默认配置的默认配置
void Sys_DbInit(void)
{
	//生成服务器密钥
	gSysVars.SrvSecretKey=Rand(0xffff);
}

//运行于系统即将要烧默认数据库到flash之前
void Sys_Default(void)
{
	u8 PwHash[16]="";
	
	sprintf((void *)PwHash,"%u",GenerateDefPw());//得到密码
	gSysDb.UserPwHash=MakeHash33(PwHash,strlen((void *)PwHash));//生成默认密码hash 276363151
	
    //RAM_INFO->ObjIdBase=Rand(0x7fff);
}

u32 Sys_GetValue(u16 Item,u32 IntParam,void *Val)
{
	switch(Item)
	{
		case SIN_SN:
			return GetHwID();
		case SIN_SoftVer:
			return gSysDb.SoftVer;
		case SIN_HwVer:
			return gSysDb.HwVer;
		case SIN_IpAddr:
			MemCpy(Val,gSysDb.IpAddr,4);
			return 4;
		case SIN_SubMask:
			MemCpy(Val,gSysDb.SubMask,4);
			return 4;
		case SIN_Gataway:
			MemCpy(Val,gSysDb.Gataway,4);
			return 4;
		case SIN_Port:
			return gSysDb.Port;
		case SIN_DnsIp:
			MemCpy(Val,gSysDb.DnsIp,4);
			return 4;			
		case SIN_BackupDnsIp:
			MemCpy(Val,gSysDb.BackupDnsIp,4);
			return 4;			
		case SIN_ServerURL:
			strcpy(Val,(void *)gSysDb.ServerURL);
			return strlen((void *)gSysDb.ServerURL)+1;
		case SIN_JsonServerURL:
			strcpy(Val,(void *)gSysDb.JsonServerURL);
			return strlen((void *)gSysDb.JsonServerURL)+1;
		case SIN_WnetAddr:
			return WNetMyAddr();
		case SIN_SysIdBase:
			return ObjIdBase();
		case SIN_SysConfigFlag:
			return GetSysConfigFlag();
		case SIN_UserPwHash:
			return gSysDb.UserPwHash;
		case SIN_NeedSrvTrans:
			return gSysDb.NeedSrvTrans;
		case SIN_RssiThred:
			return gSysDb.RssiThred;
		case SIN_RfTrans:
			return gSysDb.RfTransID;
		case SIN_UcomEnBits:
			return gSysDb.UcomEnBits;
		case SIN_LcdVars:
			if(IntParam<F_LCD_VAR_DISP_NUM)	return gSysDb.LcdDispVar[IntParam];
			else return 0;
		case SIN_LcdVarsName:
			if(IntParam<F_LCD_VAR_DISP_NUM)
			{
				strcpy(Val,(void *)gSysDb.LcdDispVarName[IntParam]);
				return strlen((void *)gSysDb.LcdDispVarName[IntParam])+1;
			}
			else return 0;
		case SIN_OpenBeep:
			return gSysDb.OpenBeep;
		case SIN_WNetChannel:
			return gSysDb.WNetChannel;
		case SIN_ZigbeeChannel:
			return gSysDb.ZigbeeChannal;
		case SIN_ZigbeeGroup:
			return gSysDb.ZigbeeGroup;
		case SIN_DutRegSn:
			return gSysDb.DutRegSn;
		case SIN_DataUpPeriod:
			return gSysDb.DataUpPeriodIdx;
	}

	return 0;
}

bool Sys_SetValue(u16 Item,u32 IntParam,void *pParam,u16 ByteLen)
{
	switch(Item)
	{
		case SIN_SoftVer:
			gSysDb.SoftVer=IntParam;
			break;
		case SIN_IpAddr:
			if(pParam!=NULL) MemCpy(gSysDb.IpAddr,pParam,4);
			break;
		case SIN_SubMask:
			if(pParam!=NULL) MemCpy(gSysDb.SubMask,pParam,4);
			break;
		case SIN_Gataway:
			if(pParam!=NULL) MemCpy(gSysDb.Gataway,pParam,4);
			break;
		case SIN_Port:
			gSysDb.Port=IntParam;
			break;
		case SIN_DnsIp:
			if(pParam!=NULL) MemCpy(gSysDb.DnsIp,pParam,4);
			break;		
		case SIN_BackupDnsIp:
			if(pParam!=NULL) MemCpy(gSysDb.BackupDnsIp,pParam,4);
			break;	
		case SIN_ServerURL:
			if(ByteLen < sizeof(gSysDb.ServerURL) && pParam!=NULL);
				MemCpy((void *)gSysDb.ServerURL,pParam,ByteLen+1);
			break;
		case SIN_JsonServerURL:
			if(ByteLen < sizeof(gSysDb.JsonServerURL) && pParam!=NULL);
				MemCpy((void *)gSysDb.JsonServerURL,pParam,ByteLen+1);
			break;
		case SIN_SysIdBase:
			break;
		case SIN_SysConfigFlag:
			break;
		case SIN_UserPwHash:
			gSysDb.UserPwHash=IntParam;
			break;
		case SIN_NeedSrvTrans:
			gSysDb.NeedSrvTrans=IntParam;
			break;
		case SIN_RssiThred:
			gSysDb.RssiThred=IntParam;
			break;
		case SIN_RfTrans:
			gSysDb.RfTransID=IntParam;
			break;
		case SIN_UcomEnBits:
			gSysDb.UcomEnBits=IntParam&0xff;
			break;
		case SIN_LcdVars:
			if(IntParam<F_LCD_VAR_DISP_NUM)	
				if(pParam!=NULL) 
					gSysDb.LcdDispVar[IntParam]=*(u16 *)pParam;
			break;
		case SIN_LcdVarsName:
			if(IntParam<F_LCD_VAR_DISP_NUM)
			{
				if(pParam!=NULL && ByteLen && ByteLen < sizeof(gSysDb.LcdDispVarName[0])) 
				{
					MemCpy((void *)gSysDb.LcdDispVarName[IntParam],pParam,ByteLen+1);
				}
			}
			break;
		case SIN_OpenBeep:
			gSysDb.OpenBeep=IntParam;
			break;
		case SIN_WNetChannel:
			gSysDb.WNetChannel=IntParam;
			break;
		case SIN_ZigbeeChannel:
			gSysDb.ZigbeeChannal=IntParam;
			break;
		case SIN_ZigbeeGroup:
			gSysDb.ZigbeeGroup=IntParam;
			break;
		case SIN_DutRegSn:
			gSysDb.DutRegSn=IntParam;
			break;
		case SIN_DataUpPeriod:
			gSysDb.DataUpPeriodIdx=IntParam;
			break;
			
		default:
			return FALSE;
	}

	return TRUE;
}

SYS_VAR_STRUCT *SysVars(void)
{
	return &gSysVars;
}

