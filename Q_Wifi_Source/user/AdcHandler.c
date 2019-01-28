#include "SysDefines.h"

static OS_TIMER_T gAdcVarTimer;

//周期性读取adc函数
static void AdcVar_TimerCB(void *pTimer)
{
	WE_AIN_CONFIG AinConfig;				
	QDB_GetValue(SDN_HWC,HWC_ADC_CONF,0,&AinConfig);

	if(AinConfig.AdcMode && PeriodIdx2Ms(AinConfig.PeriodIdx))
	{
		static u16 AdcVid=0;
		TVAR32 New=EspHwAdcRead();//最大值1024，对应1v
		
		//算出实际电压毫伏值
		New=(New*1000)>>10;

		//记录到默认系统变量
		if(AdcVid==0) AdcVid=FindVarId_ByTag2("QSYS","ADCV");
		SetVarVal(AdcVid,New,VRT_SYS,0);
		
		if(AinConfig.VarID)//绑定了变量才设置
		{
			TVAR32 Old=GetVarValue(AinConfig.VarID,NULL);
			
			//根据因子计算
			//Debug("Old[%u]=%u, New:%u -> ",AinConfig.VarIdx,Old,New);
			if(AinConfig.Factor!=100)
			{
				New*=100;
				New/=AinConfig.Factor;
			}
			New+=AinConfig.Offset;
			//Debug("%u\n\r",New);
			
			//更新到变量
			if(AinConfig.Tolerance==0)//改变就上报
			{
				if(New!=Old) SetVarVal(AinConfig.VarID,New,VRT_WDEV,0);
			}
			else if(New*100 < Old*(100-AinConfig.Tolerance) || New*100 > Old*(100+AinConfig.Tolerance))//改变超过容差才上报
			{
				SetVarVal(AinConfig.VarID,New,VRT_SYS,0);
			}	
		}
	}	
}

//初始化adc和变量的关系
void AdcVar_Init(void)
{
	WE_AIN_CONFIG AinConfig;				
	QDB_GetValue(SDN_HWC,HWC_ADC_CONF,0,&AinConfig);

	if(AinConfig.AdcMode && PeriodIdx2Ms(AinConfig.PeriodIdx))
	{
		AdcVar_TimerCB(&gAdcVarTimer);//第一遍，作为初始化
		
		//根据更新实现，设置系统定时器
		OS_TimerDeinit(&gAdcVarTimer);
		OS_TimerSetCallback(&gAdcVarTimer,AdcVar_TimerCB,&gAdcVarTimer);
		OS_TimerInit(&gAdcVarTimer,PeriodIdx2Ms(AinConfig.PeriodIdx),TRUE);	
	}


}

