#ifndef TRIGGER_H
#define TRIGGER_H

#include "RecordVariable.h"

enum{
	TT_NULL=0,

	TT_IN_RF,
	XXX_TT_IN_CRF_OVER,//连续射频信号结束
	XXX_TT_IN_ALARM,
	TT_APP_KEY,//键值触发
	TT_VAR_SET,	//变量触发
	TT_SYS_MSG,// 系统消息

	TT_MAX
};
typedef u8 TRIGGER_TYPE;

typedef struct{
	u32 DevID;//设备id
	u32 Key;//键值
}KEY_TRIGGER;//键值触发

typedef struct{
	u32 Code;//编码
	u16 BasePeriod;//时基，非标准编码时为0
	u16 OverDelayMin;//延时多久算作结束，0表示非连续射频
	u32 TransWAddr;//转发器地址，当转发器为全ffffffff时，表示允许全局接收
}RF_TRIGGER;//标准射频输入

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
	u32 TimeSec:28;//满足条件的秒数
}VAR_TRIGGER;

typedef struct{
	u32 ID;
	u8 Name[SHORT_STR_BYTES];
	TRIGGER_TYPE TrigType;//触发源类型
	u8 a;
	u16 b;
	union{
		KEY_TRIGGER KeyVal;
		RF_TRIGGER Rf;
		SYS_MSG_TRIGGER SysMsg;
		VAR_TRIGGER VarTrigger;
	}Oppo;//时机	

	VAR_TRIGGER VarState;//状态符合
	
	u32 SceneID;//要执行的情景
}TRIGGER_RECORD;


#endif

