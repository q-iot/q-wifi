#ifndef PROD_ID_H
#define PROD_ID_H

typedef enum{
	PID_SYS_START=1000,
	PID_IR_KEYS,//��������
	PID_RF_KEYS,
	PID_COM,
	
	PID_SYS_END=1999,
	
	PID_JUMPER=2001,	

}PROD_ID_DEVICE;//ϵͳ����prod id

#define ProdIsWav(x) ((x) && ((x)<PID_SYS_START||(x)>PID_SYS_END)) //�������÷�Χ��















#endif


