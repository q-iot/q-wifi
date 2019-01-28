#ifndef TRIGGER_H
#define TRIGGER_H

#include "RecordVariable.h"

enum{
	TT_NULL=0,

	TT_IN_RF,
	XXX_TT_IN_CRF_OVER,//������Ƶ�źŽ���
	XXX_TT_IN_ALARM,
	TT_APP_KEY,//��ֵ����
	TT_VAR_SET,	//��������
	TT_SYS_MSG,// ϵͳ��Ϣ

	TT_MAX
};
typedef u8 TRIGGER_TYPE;

typedef struct{
	u32 DevID;//�豸id
	u32 Key;//��ֵ
}KEY_TRIGGER;//��ֵ����

typedef struct{
	u32 Code;//����
	u16 BasePeriod;//ʱ�����Ǳ�׼����ʱΪ0
	u16 OverDelayMin;//��ʱ�������������0��ʾ��������Ƶ
	u32 TransWAddr;//ת������ַ����ת����Ϊȫffffffffʱ����ʾ����ȫ�ֽ���
}RF_TRIGGER;//��׼��Ƶ����

typedef struct{
	u32 Hash;
	u32 StrID;
}SYS_MSG_TRIGGER;

typedef struct{
	u16 Vid;
	TVAR Mask;
	TVAR CalcA;
	TVAR CalcB;
	u32 Method:4; //VAR_RANGE_JUDGE
	u32 TimeSec:28;//��������������
}VAR_TRIGGER;

typedef struct{
	u32 ID;
	u8 Name[SHORT_STR_BYTES];
	TRIGGER_TYPE TrigType;//����Դ����
	u8 a;
	u16 b;
	union{
		KEY_TRIGGER KeyVal;
		RF_TRIGGER Rf;
		SYS_MSG_TRIGGER SysMsg;
		VAR_TRIGGER VarTrigger;
	}Oppo;//ʱ��	

	VAR_TRIGGER VarState;//״̬����
	
	u32 SceneID;//Ҫִ�е��龰
}TRIGGER_RECORD;


#endif

