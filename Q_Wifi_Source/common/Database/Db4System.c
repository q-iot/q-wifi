#include "SysDefines.h"
//#include "SecretRun.h"

const SYS_DB_STRUCT gDefSysDb={
0,0,0,//�����û���Ԥ�İ汾����־��Ϣ

//�û�����
QWIFI_SOFT_VER,//u16 SIN_SoftVer;�����Ҫǿ�������ݣ����Ӵ�ֵ
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
100,//u8 RssiThred;rssi��ֵ
0,//u32 DutRegSn;//�忨ע�����к�
0,//u32 CityCode;//���б���
FALSE,//u8 DataUpEnable;//�Ƿ��ϱ�json����
FALSE,//u8 OpenBeep
0xff,//	u8 UcomEnBits;//���ڳ�ʼ����ʶ��ÿbit����һ��UCOM_ID
{0,0,0,0},//u16 LcdDispVar[F_LCD_VAR_DISP_NUM];
{{0},{0},{0},{0}},//u8 LcdDispVarName[F_LCD_VAR_DISP_NUM][SHORT_STR_BYTES];
100,//u16 WNetChannel;//wnetƵ�������ܴ��ڵ���50��������Ĭ��433.5
25,//u16 ZigbeeChannal;//zigbeeƵ��
0x1001,//u16 ZigbeeGroup;//zigbee��
};	
	

SYS_DB_STRUCT gSysDb={0,0,0};//ϵͳ���ݿ�
static SYS_VAR_STRUCT gSysVars={//ϵͳ������
{0,0,0,0},//server ip
{0,0,0,0},//json server ip
0,//secret key
SCS_OFFLINE,//srv connect status
SCS_OFFLINE,//JsonConnStaus
NULL,//SrvConn
NULL,//JsonConn

0xff,//����ID
FALSE,//is backup srv
0,//LastVaildBeatTime
FALSE,//OpenDhcp
FALSE,//DhcpFinish
0,//u8 SiRssi;//si4432�ź�ָʾ
0,//u8 StopDataUp;//json�ϱ�����Сʱ������
0,//	u8 VarMinPeriod;//�����ϱ�����С��������

FALSE,//bool RtcRstFlag;//�Ƿ������˺󱸵��
FALSE,//bool DutRegSnIsOk;//ע����Ȩ���Ƿ���ȷ
FALSE,//bool SupportLCD:1;//֧��lcd��ʾ
FALSE,//bool SwapUartOut:1;//�Ƿ��л�uart0��io13 io15
FALSE,//bool FinishRtcSync:1;//�Ƿ�����˷������Ķ�ʱ

0,//ϵͳ��������ʱ��
FALSE,//��si������Ƶ
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
		if(gSysDb.IpAddr[0]*gSysDb.IpAddr[1]*gSysDb.IpAddr[2]*gSysDb.IpAddr[3])//���Զ���ȡ
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

//ϵͳ��ʼ��ʱ����
//������ϵͳ�������ݿ���û�����֮ǰ
//��������һЩ�޷����浽Ĭ�����õ�Ĭ������
void Sys_DbInit(void)
{
	//���ɷ�������Կ
	gSysVars.SrvSecretKey=Rand(0xffff);
}

//������ϵͳ����Ҫ��Ĭ�����ݿ⵽flash֮ǰ
void Sys_Default(void)
{
	u8 PwHash[16]="";
	
	sprintf((void *)PwHash,"%u",GenerateDefPw());//�õ�����
	gSysDb.UserPwHash=MakeHash33(PwHash,strlen((void *)PwHash));//����Ĭ������hash 276363151
	
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

