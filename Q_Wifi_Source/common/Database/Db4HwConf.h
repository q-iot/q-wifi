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
	u32 Flag;//��ȡ��־�������û���Ԥ
	u32 Ver;//�汾�������û���Ԥ
	u32 ChkSum;//У��ͣ������û���Ԥ
	
	//�û���������
	WE_IO_CONFIG IoConfig[HW_CONF_IO_TOTAL];
	WE_AIN_CONFIG AdcConfig;
	u32 PwmPeriod;//Ĭ��pwm���ڣ���λus��ȡֵ��Χ1000 - 4 000 000
	u32 PwmDuty; //Ĭ��pwmռ�ձȣ�ǧ���ƣ�ȡֵ��Χ0-1000
	
	u32 Tail;//����spi fls write 4bytes align
}HW_CONF_STRUCT;	//���ݿ�

void HWC_Debug(void);
void HWC_DbInit(void);
u32 HWC_GetValue(u16 Item,u32 IntParam,void *Val);
bool HWC_SetValue(u16 Item,u32 IntParam,void *pParam,u16 ByteLen);
void HWC_Default(void);

#endif

