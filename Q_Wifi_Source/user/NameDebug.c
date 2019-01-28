#include "SysDefines.h"

#if 1
u8 * const gNameGlobaPktTypes[PT_LOCAL_MAX]=
{
	"NULL",
	"SYS PARAM",
	"BULK",//批量操作
	"SIG OPT",//信号设定,开启录入，关闭录入，发射录入，存储录入
	"INFO",//信号集设定，信号加入，修改，删除，清空，信号集绑定
	"WDEV CTRL",//jumper控制
	"KEY CTRL",//按键控制
	"FLASH OPERATE",//flash操作
	"LOGIN",//login
	"STATUS",//状态包
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
	"ParamError",//传入的参数有误
	"ResNull",//需要获取的资源为空
	"ResUnenough",//资源不够
	"NumOverflow",//参数超限
	"RoutingError",//程序错误
	"HardwareError",//硬件错误

	"AuthError",//权限错误
	"Timeout",//超时
	"Bust",//忙	
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
	"Get",//根据idx批量获取
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
	"EditStart",//开始读取同步
	"EditEnd",//结束读取同步
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
	"SRV",//通过服务器来设置的变量
	"SYS",//通过系统来设置的变量
	"DEV",//通过设备来设置的变量
	"USR",//用户设备自己设置的变量
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
	"KEY",//按键模式，会有防干扰处理，只响应下降沿，发生响应时，可以匹配、反向匹配、取反、计数变量。变量响应时间为延时200ms，适合接按键。
	"IPT",//输入
	"OUT",//推挽输出，根据绑定的变量来匹配输出，当变量某bit发生变化时，可以匹配、反向匹配、取反。
	"OUT_OD",//开漏输出
	"AIN",//模拟量输入，采样周期同上报周期相同，但上报周期到期时，如变量未变，则不上报
	"PWM",//pwm输出
	"UART",//用户通讯口
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
		case CLM_FUZZY_EQU: return "≈";
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
