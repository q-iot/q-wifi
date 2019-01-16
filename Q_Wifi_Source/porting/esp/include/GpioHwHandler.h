#ifndef GPIO_VARS_HANDLER_H
#define GPIO_VARS_HANDLER_H


typedef struct{
	u32 Period;//默认周期，单位us
	u32 Duty;//默认占空比，0-1000
	u8 ChanMap[6];//channel from 0
}HW_PWM_CONFIG;

extern HW_PWM_CONFIG gHwPwmConfig;

void GpioHwInit(void);


#endif

