#ifndef SCENE_H
#define SCENE_H

#include "RecordVariable.h"

enum{	
	SIA_NULL=0,
	SIA_KEY,//发送键值，相当于app上按键被按下
	SIA_VAR_SET,//设定变量为指定值
	SIA_SYS_MSG,//系统信息
	SIA_DEFEND_ON,
	SIA_DEFEND_OFF,
	SIA_SCENE_EXC,//执行某个情景
	
	SIA_MAX
};
typedef u8 SCENE_ITEM_ACT;

typedef struct{
	u32 DevID;//输出设备
	u32 Key;
}EXC_KEY_ITEM;//键值输出

typedef struct{ 
	u16 Vid;
	VAR_CALC_METHOD CalcMethod;
	u8 a;
	TVAR CalcMask;
	TVAR CalcVal;	//立即数或者变量id
}EXC_VAR_ITEM;

typedef struct{
	u32 StrID;//字符串内容
	u32 UserID;//用户id，当需要发送给家庭所有成员时，此值设置为0
	bool Sys:1;//系统字符串，发给所有板卡
	bool GsmMsg:1;//短信
	bool DevStr:1;//设备字符串，被推送到app设备界面，由微信修改来
	bool PushMsg:1;//推送
	bool xxxxIsVid:1;//置1时，Param传递的是vid。否则传递实际数值，暂时废弃
	bool xxxxDevMsg:1;//置1时，表示信息发给设备，UserID存储设备id，暂时废弃
	u8 a:2;
	u8 b;
	TVAR xxxxParam; //需要传递的变量id或值，暂时废弃
}EXC_STR_ITEM;

typedef struct{
	SCENE_ITEM_ACT Act;//动作 
	u8 a;
	u16 DelaySec;//执行完毕后的延时
	union{
		EXC_KEY_ITEM Key;//发出键值给设备
		EXC_VAR_ITEM Var;//设定变量值
		EXC_STR_ITEM Str;//家庭信息或系统信息
		ID_T SceneID;//执行指定的情景
	}Exc;
}SCENE_ITEM;

typedef struct{
	u32 ID;
	u8 Name[SHORT_STR_BYTES];
	SCENE_ITEM Items[SCENE_MAX_ITEM_NUM];
}SCENE_RECORD;

bool SceneItemOk(SCENE_ITEM *pItem);

#endif

