#ifndef GPIO_VARS_HANDLER_H
#define GPIO_VARS_HANDLER_H


typedef struct{
	u32 Period;//Ĭ�����ڣ���λus
	u32 Duty;//Ĭ��ռ�ձȣ�0-1000
	u8 ChanMap[6];//channel from 0
}HW_PWM_CONFIG;

extern HW_PWM_CONFIG gHwPwmConfig;

void GpioHwInit(void);


#endif

