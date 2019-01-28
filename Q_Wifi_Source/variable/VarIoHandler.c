#include "SysDefines.h"
#include "GpioHwHandler.h"

//��Ѱ�������io�ͱ�����ƥ���ϵ
void UpdateIoStateByVar(u16 Vid,TVAR32 Val)
{
	WE_IO_CONFIG IoConf;
	u8 Pin=0;

	if(Vid==0) return;

    //pin0 - pin16
	for(Pin=0;Pin<=16;Pin++)
	{
		if(Pin==2) Pin=4;//����2 3
		if(Pin==6) Pin=12;//����6 7 8 9 10 11
		
		QDB_GetValue(SDN_HWC,HWC_IO_CONF,Pin,&IoConf);

		if(IoConf.VarIoMth && IoConf.VarID==Vid)
		{
			if(IoConf.IoMode==WIM_OUT || IoConf.IoMode==WIM_OUT_OD) //��ͨ���
			{
				u8 OutState=ReadBit(Val,IoConf.Bit);
				
				switch(IoConf.VarIoMth)
				{
					case AIM_REVERSE://����ƥ��
						OutState=OutState?0:1;
					case AIM_MATCH://ƥ��
						if(Pin==16) gpio16_output_set(OutState);
						else GPIO_OUTPUT_SET(GPIO_ID_PIN(Pin),OutState);
						break;
				}
			}
			else if(IoConf.IoMode==WIM_PWM)//pwm���
			{
				switch(IoConf.VarIoMth)
				{
					case AIM_MATCH://�ı�ռ�ձ�
						{
							u32 Duty=Val*1023/1000;//Duty��Χ0-1023��0��ʾռ�ձ�Ϊ0��1023��ʾռ�ձ�Ϊ100%
							//Debug("PWM[%u] Duty:%u, Channel %u\n\r",Pin,Duty>1023?1023:Duty,gHwPwmConfig.ChanMap[Pin]);
							pwm_set_duty(Duty>1023?1023:Duty,gHwPwmConfig.ChanMap[Pin]);//channel from 0��
							pwm_start();	
						}
						break;
					case AIM_REVERSE://�ı�����
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

//�����û�app����İ�ť��ֵ���ı�io״̬
//BtnKey = Pin * 1000 + Arg
void UpdateIoStateByBtn(u32 DevID,u16 BtnKey)
{
	WE_IO_CONFIG IoConf;
	u8 Pin=BtnKey/1000;
	u16 Arg=BtnKey%1000;

	if(Pin==0) return;
	if(Pin==20) Pin=0;//��pin20���pin0
	if(Pin>16) return;
	
	Debug("BtnKey Pin%u Arg%u\n\r",Pin,Arg);
	
	QDB_GetValue(SDN_HWC,HWC_IO_CONF,Pin,&IoConf);
	if(IoConf.IoMode!=WIM_OUT && IoConf.IoMode!=WIM_OUT_OD) return;
	
	if(Arg==900)//�õ�
	{
		if(Pin==16) gpio16_output_set(0);
		else GPIO_OUTPUT_SET(GPIO_ID_PIN(Pin),0);
	}
	else if(Arg==901)//�ø�
	{
		if(Pin==16) gpio16_output_set(1);
		else GPIO_OUTPUT_SET(GPIO_ID_PIN(Pin),1);
	}		
	else if(Arg==902)//ȡ����espӲ����֧�ִ�����ڶ��ߵ�
	{
		u8 Val=0;

		if(Pin==16) Val=gpio16_input_get();
		else Val=GPIO_INPUT_GET(GPIO_ID_PIN(Pin));

		if(Pin==16) gpio16_output_set(Val?0:1);
		else GPIO_OUTPUT_SET(GPIO_ID_PIN(Pin),Val?0:1);		
	}
	else if(Arg && Arg<=100)//������
	{
		if(Pin==16) gpio16_output_set(0);
		else GPIO_OUTPUT_SET(GPIO_ID_PIN(Pin),0);

		if(Pin==0) Pin=20;
		SysEventMsSend(Arg*100,SEN_PIN_CHANGE,Pin*1000+901,NULL,MFM_ALWAYS);//��ʱ
	}
	else if(Arg<=200)//������
	{
		if(Pin==16) gpio16_output_set(1);
		else GPIO_OUTPUT_SET(GPIO_ID_PIN(Pin),1);

		if(Pin==0) Pin=20;
		SysEventMsSend((Arg-100)*100,SEN_PIN_CHANGE,Pin*1000+900,NULL,MFM_ALWAYS);//��ʱ
	}

}

//����״̬�ı�ʱ����
void ChangeVarByKeyState(u8 Pin,u16 PressMs)
{
	WE_IO_CONFIG IoConf;
	
	//Debug("Key:%d %d\n\r",Pin,PressMs);
	QDB_GetValue(SDN_HWC,HWC_IO_CONF,Pin,&IoConf);

	//�����Ƿ�Ҫ�ı����
	if(IoConf.VarIoMth && IoConf.VarID)
	{		
		switch(IoConf.VarIoMth)
		{
			case AIM_MATCH://�ø�io
				SetVarBit(IoConf.VarID,IoConf.Bit,1,VRT_WDEV,0);
				break;
			case AIM_REVERSE://�õ�io
				SetVarBit(IoConf.VarID,IoConf.Bit,0,VRT_WDEV,0);
				break;
			case AIM_NEGATION://�÷�
				RevVarBit(IoConf.VarID,IoConf.Bit,VRT_WDEV,0);
				break;
			case AIM_COUNT://����
				IncVar(IoConf.VarID,VRT_WDEV,0);
				break;
		}
	}
}

//����Ϊ�����io��������ʱ
void ChangeVarByIoState(u8 Pin,u8 PinVal)
{
	WE_IO_CONFIG IoConf;
	
	//Debug("PIO:%d %d\n\r",Pin,PinVal);
	QDB_GetValue(SDN_HWC,HWC_IO_CONF,Pin,&IoConf);
	
	//�����Ƿ�Ҫ�ı����
	if(IoConf.VarIoMth && IoConf.VarID)
	{
		switch(IoConf.VarIoMth)
		{
			case AIM_MATCH://ͬ������
				SetVarBit(IoConf.VarID,IoConf.Bit,PinVal,VRT_WDEV,0);
				break;
			case AIM_REVERSE://��������
				SetVarBit(IoConf.VarID,IoConf.Bit,PinVal?0:1,VRT_WDEV,0);
				break;
			case AIM_NEGATION://ȡ��
				RevVarBit(IoConf.VarID,IoConf.Bit,VRT_WDEV,0);
				break;
			case AIM_COUNT://����
				IncVar(IoConf.VarID,VRT_WDEV,0);
				break;
		}
	}
}

//���ݱ���ֵ����������io״̬
void UpdateAllIoStateByVar(void)
{
	WE_IO_CONFIG IoConf;
	u8 Pin=0;
	
    //pin0 - pin16
	for(Pin=0;Pin<=16;Pin++)
	{
		if(Pin==2) Pin=4;//����2 3
		if(Pin==6) Pin=12;//����6 7 8 9 10 11
		
		QDB_GetValue(SDN_HWC,HWC_IO_CONF,Pin,&IoConf);

		if(IoConf.IoMode==WIM_OUT || IoConf.IoMode==WIM_OUT_OD || IoConf.IoMode==WIM_PWM)//���io������
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

//����io��ֵ��ģʽ�����ñ�����ֵ
void InitVarByIoState(void)
{
	WE_IO_CONFIG IoConf;
	u8 Pin=0,PinVal=0;

    //pin0 - pin15
	for(Pin=0;Pin<=15;Pin++)//io16ֻ����������Բ�����
	{
		if(Pin==2) Pin=4;//����2 3
		if(Pin==6) Pin=12;//����6 7 8 9 10 11
		
		QDB_GetValue(SDN_HWC,HWC_IO_CONF,Pin,&IoConf);

		if(IoConf.IoMode==WIM_IPT)//��������ģʽ
		{
			if(IoConf.VarIoMth && IoConf.VarID)
			{
				switch(IoConf.VarIoMth)
				{
					case AIM_MATCH://ͬ������
						PinVal=GPIO_INPUT_GET(GPIO_ID_PIN(Pin));
						SetVarBit(IoConf.VarID,IoConf.Bit,PinVal,VRT_WDEV,0);
						break;
					case AIM_REVERSE://��������
						PinVal=GPIO_INPUT_GET(GPIO_ID_PIN(Pin));
						SetVarBit(IoConf.VarID,IoConf.Bit,PinVal?0:1,VRT_WDEV,0);
						break;
				}
			}
		}		
	}
}


