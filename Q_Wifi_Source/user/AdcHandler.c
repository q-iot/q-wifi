#include "SysDefines.h"

static OS_TIMER_T gAdcVarTimer;

//�����Զ�ȡadc����
static void AdcVar_TimerCB(void *pTimer)
{
	WE_AIN_CONFIG AinConfig;				
	QDB_GetValue(SDN_HWC,HWC_ADC_CONF,0,&AinConfig);

	if(AinConfig.AdcMode && PeriodIdx2Ms(AinConfig.PeriodIdx))
	{
		static u16 AdcVid=0;
		TVAR32 New=EspHwAdcRead();//���ֵ1024����Ӧ1v
		
		//���ʵ�ʵ�ѹ����ֵ
		New=(New*1000)>>10;

		//��¼��Ĭ��ϵͳ����
		if(AdcVid==0) AdcVid=FindVarId_ByTag2("QSYS","ADCV");
		SetVarVal(AdcVid,New,VRT_SYS,0);
		
		if(AinConfig.VarID)//���˱���������
		{
			TVAR32 Old=GetVarValue(AinConfig.VarID,NULL);
			
			//�������Ӽ���
			//Debug("Old[%u]=%u, New:%u -> ",AinConfig.VarIdx,Old,New);
			if(AinConfig.Factor!=100)
			{
				New*=100;
				New/=AinConfig.Factor;
			}
			New+=AinConfig.Offset;
			//Debug("%u\n\r",New);
			
			//���µ�����
			if(AinConfig.Tolerance==0)//�ı���ϱ�
			{
				if(New!=Old) SetVarVal(AinConfig.VarID,New,VRT_WDEV,0);
			}
			else if(New*100 < Old*(100-AinConfig.Tolerance) || New*100 > Old*(100+AinConfig.Tolerance))//�ı䳬���ݲ���ϱ�
			{
				SetVarVal(AinConfig.VarID,New,VRT_SYS,0);
			}	
		}
	}	
}

//��ʼ��adc�ͱ����Ĺ�ϵ
void AdcVar_Init(void)
{
	WE_AIN_CONFIG AinConfig;				
	QDB_GetValue(SDN_HWC,HWC_ADC_CONF,0,&AinConfig);

	if(AinConfig.AdcMode && PeriodIdx2Ms(AinConfig.PeriodIdx))
	{
		AdcVar_TimerCB(&gAdcVarTimer);//��һ�飬��Ϊ��ʼ��
		
		//���ݸ���ʵ�֣�����ϵͳ��ʱ��
		OS_TimerDeinit(&gAdcVarTimer);
		OS_TimerSetCallback(&gAdcVarTimer,AdcVar_TimerCB,&gAdcVarTimer);
		OS_TimerInit(&gAdcVarTimer,PeriodIdx2Ms(AinConfig.PeriodIdx),TRUE);	
	}


}

