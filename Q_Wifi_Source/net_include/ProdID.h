#ifndef PROD_ID_H
#define PROD_ID_H

typedef enum{
	PID_SYS_START=1000,
	PID_IR_KEYS,//按键集合
	PID_RF_KEYS,
	PID_COM,
	
	PID_SYS_END=1999,
	
	PID_JUMPER=2001,	

}PROD_ID_DEVICE;//系统自用prod id

#define ProdIsWav(x) ((x) && ((x)<PID_SYS_START||(x)>PID_SYS_END)) //不在自用范围内















#endif


