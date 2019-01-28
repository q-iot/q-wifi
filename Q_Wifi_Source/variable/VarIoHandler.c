#include "SysDefines.h"
#include "GpioHwHandler.h"

//搜寻所有输出io和变量的匹配关系
void UpdateIoStateByVar(u16 Vid,TVAR32 Val)
{
	WE_IO_CONFIG IoConf;
	u8 Pin=0;

	if(Vid==0) return;

    //pin0 - pin16
	for(Pin=0;Pin<=16;Pin++)
	{
		if(Pin==2) Pin=4;//跳过2 3
		if(Pin==6) Pin=12;//跳过6 7 8 9 10 11
		
		QDB_GetValue(SDN_HWC,HWC_IO_CONF,Pin,&IoConf);

		if(IoConf.VarIoMth && IoConf.VarID==Vid)
		{
			if(IoConf.IoMode==WIM_OUT || IoConf.IoMode==WIM_OUT_OD) //普通输出
			{
				u8 OutState=ReadBit(Val,IoConf.Bit);
				
				switch(IoConf.VarIoMth)
				{
					case AIM_REVERSE://反向匹配
						OutState=OutState?0:1;
					case AIM_MATCH://匹配
						if(Pin==16) gpio16_output_set(OutState);
						else GPIO_OUTPUT_SET(GPIO_ID_PIN(Pin),OutState);
						break;
				}
			}
			else if(IoConf.IoMode==WIM_PWM)//pwm输出
			{
				switch(IoConf.VarIoMth)
				{
					case AIM_MATCH://改变占空比
						{
							u32 Duty=Val*1023/1000;//Duty范围0-1023，0表示占空比为0，1023表示占空比为100%
							//Debug("PWM[%u] Duty:%u, Channel %u\n\r",Pin,Duty>1023?1023:Duty,gHwPwmConfig.ChanMap[Pin]);
							pwm_set_duty(Duty>1023?1023:Duty,gHwPwmConfig.ChanMap[Pin]);//channel from 0，
							pwm_start();	
						}
						break;
					case AIM_REVERSE://改变周期
						if(Val)
						{
							u32 Period=Val*100;							
							//Debug("PWM[%u] Period:%uuS %umS\r\n",Pin,Period,Period/1000);
							pwm_set_duty(gHwPwmConfig.Duty,gHwPwmConfig.ChanMap[Pin]);
							pwm_set_period(Period>4000000?4000000:Period);
						}
						else
						{
							pwm_set_duty(0,gHwPwmConfig.ChanMap[Pin]);
						}
						pwm_start();
						break;
				}
			}
		}		
	}
}

//根据用户app输入的按钮键值来改变io状态
//BtnKey = Pin * 1000 + Arg
void UpdateIoStateByBtn(u32 DevID,u16 BtnKey)
{
	WE_IO_CONFIG IoConf;
	u8 Pin=BtnKey/1000;
	u16 Arg=BtnKey%1000;

	if(Pin==0) return;
	if(Pin==20) Pin=0;//用pin20替代pin0
	if(Pin>16) return;
	
	Debug("BtnKey Pin%u Arg%u\n\r",Pin,Arg);
	
	QDB_GetValue(SDN_HWC,HWC_IO_CONF,Pin,&IoConf);
	if(IoConf.IoMode!=WIM_OUT && IoConf.IoMode!=WIM_OUT_OD) return;
	
	if(Arg==900)//置低
	{
		if(Pin==16) gpio16_output_set(0);
		else GPIO_OUTPUT_SET(GPIO_ID_PIN(Pin),0);
	}
	else if(Arg==901)//置高
	{
		if(Pin==16) gpio16_output_set(1);
		else GPIO_OUTPUT_SET(GPIO_ID_PIN(Pin),1);
	}		
	else if(Arg==902)//取反，esp硬件不支持从输出口读高低
	{
		u8 Val=0;

		if(Pin==16) Val=gpio16_input_get();
		else Val=GPIO_INPUT_GET(GPIO_ID_PIN(Pin));

		if(Pin==16) gpio16_output_set(Val?0:1);
		else GPIO_OUTPUT_SET(GPIO_ID_PIN(Pin),Val?0:1);		
	}
	else if(Arg && Arg<=100)//低脉冲
	{
		if(Pin==16) gpio16_output_set(0);
		else GPIO_OUTPUT_SET(GPIO_ID_PIN(Pin),0);

		if(Pin==0) Pin=20;
		SysEventMsSend(Arg*100,SEN_PIN_CHANGE,Pin*1000+901,NULL,MFM_ALWAYS);//定时
	}
	else if(Arg<=200)//高脉冲
	{
		if(Pin==16) gpio16_output_set(1);
		else GPIO_OUTPUT_SET(GPIO_ID_PIN(Pin),1);

		if(Pin==0) Pin=20;
		SysEventMsSend((Arg-100)*100,SEN_PIN_CHANGE,Pin*1000+900,NULL,MFM_ALWAYS);//定时
	}

}

//按键状态改变时处理
void ChangeVarByKeyState(u8 Pin,u16 PressMs)
{
	WE_IO_CONFIG IoConf;
	
	//Debug("Key:%d %d\n\r",Pin,PressMs);
	QDB_GetValue(SDN_HWC,HWC_IO_CONF,Pin,&IoConf);

	//看看是否要改变变量
	if(IoConf.VarIoMth && IoConf.VarID)
	{		
		switch(IoConf.VarIoMth)
		{
			case AIM_MATCH://置高io
				SetVarBit(IoConf.VarID,IoConf.Bit,1,VRT_WDEV,0);
				break;
			case AIM_REVERSE://置低io
				SetVarBit(IoConf.VarID,IoConf.Bit,0,VRT_WDEV,0);
				break;
			case AIM_NEGATION://置反
				RevVarBit(IoConf.VarID,IoConf.Bit,VRT_WDEV,0);
				break;
			case AIM_COUNT://自增
				IncVar(IoConf.VarID,VRT_WDEV,0);
				break;
		}
	}
}

//有作为输入的io发生输入时
void ChangeVarByIoState(u8 Pin,u8 PinVal)
{
	WE_IO_CONFIG IoConf;
	
	//Debug("PIO:%d %d\n\r",Pin,PinVal);
	QDB_GetValue(SDN_HWC,HWC_IO_CONF,Pin,&IoConf);
	
	//看看是否要改变变量
	if(IoConf.VarIoMth && IoConf.VarID)
	{
		switch(IoConf.VarIoMth)
		{
			case AIM_MATCH://同向设置
				SetVarBit(IoConf.VarID,IoConf.Bit,PinVal,VRT_WDEV,0);
				break;
			case AIM_REVERSE://反向设置
				SetVarBit(IoConf.VarID,IoConf.Bit,PinVal?0:1,VRT_WDEV,0);
				break;
			case AIM_NEGATION://取反
				RevVarBit(IoConf.VarID,IoConf.Bit,VRT_WDEV,0);
				break;
			case AIM_COUNT://自增
				IncVar(IoConf.VarID,VRT_WDEV,0);
				break;
		}
	}
}

//根据变量值，更新所有io状态
void UpdateAllIoStateByVar(void)
{
	WE_IO_CONFIG IoConf;
	u8 Pin=0;
	
    //pin0 - pin16
	for(Pin=0;Pin<=16;Pin++)
	{
		if(Pin==2) Pin=4;//跳过2 3
		if(Pin==6) Pin=12;//跳过6 7 8 9 10 11
		
		QDB_GetValue(SDN_HWC,HWC_IO_CONF,Pin,&IoConf);

		if(IoConf.IoMode==WIM_OUT || IoConf.IoMode==WIM_OUT_OD || IoConf.IoMode==WIM_PWM)//输出io的设置
		{
			if(IoConf.VarIoMth && IoConf.VarID)
			{
				VAR_STATE VarState;
				TVAR32 VarVal=GetVarValue(IoConf.VarID,&VarState);
				if(VarState==VST_VALID) UpdateIoStateByVar(IoConf.VarID,VarVal);
			}
		}		
	}
}

//根据io初值和模式，设置变量初值
void InitVarByIoState(void)
{
	WE_IO_CONFIG IoConf;
	u8 Pin=0,PinVal=0;

    //pin0 - pin15
	for(Pin=0;Pin<=15;Pin++)//io16只能输出，所以不用了
	{
		if(Pin==2) Pin=4;//跳过2 3
		if(Pin==6) Pin=12;//跳过6 7 8 9 10 11
		
		QDB_GetValue(SDN_HWC,HWC_IO_CONF,Pin,&IoConf);

		if(IoConf.IoMode==WIM_IPT)//不处理按键模式
		{
			if(IoConf.VarIoMth && IoConf.VarID)
			{
				switch(IoConf.VarIoMth)
				{
					case AIM_MATCH://同向设置
						PinVal=GPIO_INPUT_GET(GPIO_ID_PIN(Pin));
						SetVarBit(IoConf.VarID,IoConf.Bit,PinVal,VRT_WDEV,0);
						break;
					case AIM_REVERSE://反向设置
						PinVal=GPIO_INPUT_GET(GPIO_ID_PIN(Pin));
						SetVarBit(IoConf.VarID,IoConf.Bit,PinVal?0:1,VRT_WDEV,0);
						break;
				}
			}
		}		
	}
}


