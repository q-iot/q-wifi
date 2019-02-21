//--------------------------Q-IOT Platform------------------------------------------------------//
/*
Q-Wifi是酷享物联平台的固件实现，基于ESP8266开发，构建了一套与酷享APP、酷享云平台相互配合的物联网
底层实现机制，代码基于ESP8266_RTOS_SDK开发，开源，欢迎更多程序员大牛加入！

Q-Wifi构架了一套设备、变量、情景、动作、用户终端的管理机制，内建了若干个TCP连接与APP及云平台互动，
其中json上报连接，采用通用的http json格式，可被开发者导向至自己的物联网平台，进行数据汇集和控制。

Q-Wifi内建了一套动态的web网页服务器，用来进行硬件配置，此web网页服务器封装完整，可被开发者移植到
其他项目。

Q-Wifi内建了一套基于串口的指令解析系统，用来进行串口调试，指令下发，此解析系统封装完整，包含一系
列解析函数，可被开发者移植至其他项目。

Q-Wifi内部带user标识的文件，均为支持开发者自主修改的客制化文件，特别是user_hook.c文件，系统内所
有关键点都会有hook函数在此文件中，供开发者二次开发。

Q-Wifi代码拥有众多模块化的机制或方法，可以被复用及移植，减少物联网系统的开发难度。
所有基于酷享物联平台进行的开发或案例、产品，均可联系酷享团队，免费放置于酷物联视频（q-iot.cn）进行
传播或有偿售卖，相应所有扣除税费及维护费用后，均全额提供给贡献者，以此鼓励国内开源事业。

By Karlno 酷享科技

本文件是基于Database.c定义的机制的实际存储内容
*/
//---------------------------------------------------------------------------------------------//
#include "SysDefines.h"

const HW_CONF_STRUCT gDefHwConf={
0,0,0,//无需用户干预的版本及标志信息

//用户数据
{{0,FALSE,0,0,0},{0,FALSE,0,0,0},{0,FALSE,0,0,0},{0,FALSE,0,0,0},
{0,FALSE,0,0,0},{0,FALSE,0,0,0},{0,FALSE,0,0,0},{0,FALSE,0,0,0},
{0,FALSE,0,0,0},{0,FALSE,0,0,0},{0,FALSE,0,0,0},{0,FALSE,0,0,0},
{0,FALSE,0,0,0},{0,FALSE,0,0,0},{0,FALSE,0,0,0},{0,FALSE,0,0,0},{0,FALSE,0,0,0}},//WE_IO_CONFIG IoConfig[16];
{0,5,100,0,0,0},//WE_AIN_CONFIG AdcConfig;
1000,//u32 PwmPeriod; in us
500,//u32 PwmDuty;
};	
	
HW_CONF_STRUCT gHwConf={0,0,0};//存放缓存

#define Frame() //Debug("                                                                              |\r");
void HWC_Debug(void)
{
	WE_IO_CONFIG *pIoConf=gHwConf.IoConfig;
	u8 Pin=0;
	
	Debug("  ------------------------------------------------------------------------\n\r");
	Frame();Debug("  |SnHash:%u\n\r",GetHwID());

	for(Pin=0;Pin<=16;Pin++)
	{
		if(Pin==2) Pin=4;//跳过2 3
		if(Pin==6) Pin=12;//跳过6 7 8 9 10 11		
		Debug("  |Pin[%u] %s %s [Match%u Var%u:%u]\n\r",Pin,gNameIoMode[pIoConf[Pin].IoMode],pIoConf[Pin].Pullup?"PullUp":"------",pIoConf[Pin].VarIoMth,pIoConf[Pin].VarID,pIoConf[Pin].Bit);
	}	

	Frame();Debug("  |ADC %s\n\r",gHwConf.AdcConfig.AdcMode?"ON":"OFF");
	Frame();Debug("  |ADC / %u%% + (%d)\n\r",gHwConf.AdcConfig.Factor,gHwConf.AdcConfig.Offset);
	Frame();Debug("  |ADC Tonlerance %u%%\n\r",gHwConf.AdcConfig.Tolerance);
	Frame();Debug("  |ADC VarID %u\n\r",gHwConf.AdcConfig.VarID);
	Frame();Debug("  |ADC Update Period %u\n\r",gHwConf.AdcConfig.PeriodIdx);

	Frame();Debug("  |PWM Period:%uuS, Duty:%u\n\r",gHwConf.PwmPeriod,gHwConf.PwmDuty);
	
	Debug("  ------------------------------------------------------------------------\n\r");
}


//系统初始化时调用
//运行于系统读出数据库后，用户程序之前
//用于设置一些无法保存到默认配置的默认配置
void HWC_DbInit(void)
{

}

//运行于系统即将要烧默认数据库到flash之前
void HWC_Default(void)
{
	UserHwDefConfigHook(&gHwConf);
}

u32 HWC_GetValue(u16 Item,u32 IntParam,void *Val)
{
	switch(Item)
	{
		case HWC_IO_CONF:
			if(IntParam<HW_CONF_IO_TOTAL && Val!=NULL)
			{
				UserHwConfigInitHook(IntParam,&gHwConf.IoConfig[IntParam]);
				MemCpy(Val,&gHwConf.IoConfig[IntParam],sizeof(WE_IO_CONFIG));
			}
			return 0;
		case HWC_ADC_CONF:
			if(Val!=NULL)
			{
				MemCpy(Val,&gHwConf.AdcConfig,sizeof(WE_AIN_CONFIG));
			}
			return 0;
		case HWC_PWM_PERIOD:
			return gHwConf.PwmPeriod;
		case HWC_PWM_DUTY:
			return gHwConf.PwmDuty;
	}

	return 0;
}

bool HWC_SetValue(u16 Item,u32 IntParam,void *pParam,u16 ByteLen)
{
	switch(Item)
	{
		case HWC_IO_CONF:
			if(IntParam<HW_CONF_IO_TOTAL && pParam!=NULL)
			{
				MemCpy(&gHwConf.IoConfig[IntParam],pParam,sizeof(WE_IO_CONFIG));//复制内容
			}
			break;
		case HWC_ADC_CONF:
			if(pParam!=NULL)
			{
				MemCpy(&gHwConf.AdcConfig,pParam,sizeof(WE_AIN_CONFIG));//复制内容
			}
			break;
		case HWC_PWM_PERIOD:
			gHwConf.PwmPeriod=IntParam;
			break;
		case HWC_PWM_DUTY:
			gHwConf.PwmDuty=IntParam;
			break;
				
		default:
			return FALSE;
	}
	
	return TRUE;
}

