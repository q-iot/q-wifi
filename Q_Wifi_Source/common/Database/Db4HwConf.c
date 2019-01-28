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

