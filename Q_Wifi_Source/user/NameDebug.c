#include "SysDefines.h"

#if 1
u8 * const gNameGlobaPktTypes[PT_LOCAL_MAX]=
{
	"NULL",
	"SYS PARAM",
	"BULK",//��������
	"SIG OPT",//�ź��趨,����¼�룬�ر�¼�룬����¼�룬�洢¼��
	"INFO",//�źż��趨���źż��룬�޸ģ�ɾ������գ��źż���
	"WDEV CTRL",//jumper����
	"KEY CTRL",//��������
	"FLASH OPERATE",//flash����
	"LOGIN",//login
	"STATUS",//״̬��
	"CMD",
	"VAR",
	"UPDATE",
	"WAVER",
	"APP_BEAT",
	"APP_ACT",
	"APP_ATTENTION",
};

u8 * const gNameGlobaPktRes[GPR_MAX]={
	"MainPkt",
	
	"Sucess",
	"Faild",
	"ParamError",//����Ĳ�������
	"ResNull",//��Ҫ��ȡ����ԴΪ��
	"ResUnenough",//��Դ����
	"NumOverflow",//��������
	"RoutingError",//�������
	"HardwareError",//Ӳ������

	"AuthError",//Ȩ�޴���
	"Timeout",//��ʱ
	"Bust",//æ	
};

u8 * const gNameSysParamType[SPT_MAX]={
	"Get",
	"Set",
	"Burn",
	"Def",
	"DefendGet",
	"DefendSet",
	"DefendClr",
};

u8 * const gNameBulkPktType[BOPT_MAX]={
	"Arg",
	"Get",//����idx������ȡ
};

u8 * const gNameInfoPktType[IOPT_MAX]={
	"Read",
	"ReadIdx",
	"Delete",
	"New",
};

u8 * const gNameWnetCtrlType[WCPT_MAX]={
	"Null",
	"DevSearch",
	"DevAdd",
	"DevDelete",
	"DevTest",
};

u8 * const gNameInfoName[IFT_MAX]={
	"String  ",
	"Variable",
	"Rf      ",
	"IR      ",
	"Device  ",
	"Trigger ",
	"Scene   ",
};

u8 * const gNameSceneItemAct[SIA_MAX]={
	"Null",
	"Key",
	"VarSet",
	"SysMsg",
	"Def On",
	"Def Off",
	"Scn Exc",

};

u8 * const gNameStatusPktType[SCT_MAX]={
	"NULL",
	"EditStart",//��ʼ��ȡͬ��
	"EditEnd",//������ȡͬ��
	"Upgrade",
};

u8 * const gNameVariablePktType[VPA_MAX]={
	"NULL",
	"QUERY",
	"BIND",
	"RELEASE",
	"SET"
};


u8 * const gNameSrvConnStatus[SCS_MAX]={
	"OFF LINE",
	"FOUND SRV",
	"ON LINE",
	"HAS LOGIN",	
};

u8 * const gNameVarType[VAR_T_MAX]={
	"Null",
	"SRV",//ͨ�������������õı���
	"SYS",//ͨ��ϵͳ�����õı���
	"DEV",//ͨ���豸�����õı���
	"USR",//�û��豸�Լ����õı���
};

u8 * const gNameVarFat[VDF_MAX]={
	"U",
	"S",
	"H",
	"B",
	"U32",
	"S32",
	"FL",
};

u8 * const gNameVarOpt[VOT_MAX]={
	"X",
	"S",
	"G"
};

u8 * const gNameVarState[VST_MAX]={
	"NUL",
	"VAD",
	"UFD",
};

u8 * const gNameVarIdxType[VIT_MAX]={
	"SELF",
	"REL",
};

u8 * const gNameCalcMethod[CMT_MAX]={
	"#",
	"=",
	"+",
	"-",
	"&",
	"|",
	"~",
};

u8 * const gNameTriggerType[TT_MAX]={
	"NULL",
	"RF  ",
	"CRF ",
	"ALRM",
	"KEY ",
	"VAR ",
	"MSG ",
	
	
	
};

u8 *const gNameAppClientType[ACT_MAX]={
	"Null",
	"Phone",
	"PC",
	"Dev"
};

u8 *const gNameAppClientStatus[ACS_MAX]={
	"Null",
	"Online",
	"Only",
};

#if 1 //for esp wifi
const u8 * gNameStationStatus[6]={
	"IDLE",
	"CONNECTING",
	"WRONG PW",
	"NO AP FOUND",
	"CONNECT FAIL",
	"GOT IP",
};

const u8 * gNameWifiMode[4]={
    "NULL",      /**< null mode */
    "STATION",       /**< WiFi station mode */
    "AP",        /**< WiFi soft-AP mode */
    "BOTH"    /**< WiFi station + soft-AP mode */   
};

const u8 * gNameAuthMode[5]={
    "OPEN",      /**< authenticate mode : open */
    "WEP",           /**< authenticate mode : WEP */
    "WPA_PSK",       /**< authenticate mode : WPA_PSK */
    "WPA2_PSK",      /**< authenticate mode : WPA2_PSK */
    "WPA_WPA2"  /**< authenticate mode : WPA_WPA2_PSK */
};

const u8 * gNameWifiEvent[9]={
    "[S]Scan Done",        /**< ESP8266 station finish scanning AP */
    "[S]Connected",            /**< ESP8266 station connected to AP */
    "[S]Disconnected",         /**< ESP8266 station disconnected to AP */
    "[S]AuthMode change",      /**< the auth mode of AP connected by ESP8266 station changed */
    "[S]Got ip",               /**< ESP8266 station got IP from connected AP */
    "[S]DHCP timeout",         /**< ESP8266 station dhcp client got IP timeout */
    "[A]Sta connected",      /**< a station connected to ESP8266 soft-AP */
    "[A]Sta disconnected",   /**< a station disconnected to ESP8266 soft-AP */
    "[A]Probe req receive",    /**< Receive probe request packet in soft-AP interface */
};

const u8 *gNameIoMode[WIM_MAX]={
	"NULL",		
	"KEY",//����ģʽ�����з����Ŵ���ֻ��Ӧ�½��أ�������Ӧʱ������ƥ�䡢����ƥ�䡢ȡ��������������������Ӧʱ��Ϊ��ʱ200ms���ʺϽӰ�����
	"IPT",//����
	"OUT",//������������ݰ󶨵ı�����ƥ�������������ĳbit�����仯ʱ������ƥ�䡢����ƥ�䡢ȡ����
	"OUT_OD",//��©���
	"AIN",//ģ�������룬��������ͬ�ϱ�������ͬ�����ϱ����ڵ���ʱ�������δ�䣬���ϱ�
	"PWM",//pwm���
	"UART",//�û�ͨѶ��
	"OLED",//oled

};
#endif

const u8 *GlobaPktTypes(u8 Type)
{
	if(Type<PT_LOCAL_MAX) return gNameGlobaPktTypes[Type];

	switch(Type)
	{
		case PT_SRV_BEAT: return "S_BEAT";
		case PT_SRV_LOGIN: return "S_LOGIN";
		case PT_SRV_QUERY: return "S_QUERY";
		case PT_SRV_MSG: return "S_MSG";
		case PT_SRV_UNBIND: return "S_UNBIND";
		case PT_SRV_CMD: return "S_CMD";
	}

	return "Unknow";
}

#if 0
const u8 *CalcTypeToStr(COMP_METHOD Method)
{
	switch(Method)
	{
		case CLM_EQU: return "==";
		case CLM_NOT_EQU: return "!=";
		case CLM_GREATER: return ">";
		case CLM_GREATER_EQU: return ">=";
		case CLM_LESS: return "<";
		case CLM_LESS_EQU: return "<=";
		case CLM_FUZZY_EQU: return "��";
	}

	return "UnKnow";
}

const u8 *TrigTypeToStr(u8 Type)
{
	switch(Type)
	{
		case TT_IN_RF: return "Rf";
		case XXX_TT_IN_CRF_OVER: return "Crf";
		case XXX_TT_IN_ALARM: return "Alarm";
		case TT_VAR_SET: return "Var";
	}
	
	return "UnKnow";
}
#endif

const u8 *VarStateToStr(u8 State)
{
	switch(State)
	{
		case VST_NULL: return "Null";
		case VST_VALID: return "Vaild";
	}

	return "Unknow";
}


#endif
