#ifndef HW_CONF_H
#define HW_CONF_H

#include "LimitMarco.h"
#include "HwConf.h"

#define HW_CONF_IO_TOTAL 17

typedef enum {
	HWC_NULL=0,
	HWC_IO_CONF,	
	HWC_ADC_CONF,
	HWC_PWM_PERIOD,
	HWC_PWM_DUTY,
	
	HWC_MAX
}HWC_ITEM_NAME;

typedef struct{	
	u32 Flag;//读取标志，无需用户干预
	u32 Ver;//版本，无需用户干预
	u32 ChkSum;//校验和，无需用户干预
	
	//用户数据区域
	WE_IO_CONFIG IoConfig[HW_CONF_IO_TOTAL];
	WE_AIN_CONFIG AdcConfig;
	u32 PwmPeriod;//默认pwm周期，单位us，取值范围1000 - 4 000 000
	u32 PwmDuty; //默认pwm占空比，千分制，取值范围0-1000
	
	u32 Tail;//兼容spi fls write 4bytes align
}HW_CONF_STRUCT;	//数据库

void HWC_Debug(void);
void HWC_DbInit(void);
u32 HWC_GetValue(u16 Item,u32 IntParam,void *Val);
bool HWC_SetValue(u16 Item,u32 IntParam,void *pParam,u16 ByteLen);
void HWC_Default(void);

#endif

