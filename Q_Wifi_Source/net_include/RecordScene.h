#ifndef SCENE_H
#define SCENE_H

#include "RecordVariable.h"

enum{	
	SIA_NULL=0,
	SIA_KEY,//���ͼ�ֵ���൱��app�ϰ���������
	SIA_VAR_SET,//�趨����Ϊָ��ֵ
	SIA_SYS_MSG,//ϵͳ��Ϣ
	SIA_DEFEND_ON,
	SIA_DEFEND_OFF,
	SIA_SCENE_EXC,//ִ��ĳ���龰
	
	SIA_MAX
};
typedef u8 SCENE_ITEM_ACT;

typedef struct{
	u32 DevID;//����豸
	u32 Key;
}EXC_KEY_ITEM;//��ֵ���

typedef struct{ 
	u16 Vid;
	VAR_CALC_METHOD CalcMethod;
	u8 a;
	TVAR CalcMask;
	TVAR CalcVal;	//���������߱���id
}EXC_VAR_ITEM;

typedef struct{
	u32 StrID;//�ַ�������
	u32 UserID;//�û�id������Ҫ���͸���ͥ���г�Աʱ����ֵ����Ϊ0
	bool Sys:1;//ϵͳ�ַ������������а忨
	bool GsmMsg:1;//����
	bool DevStr:1;//�豸�ַ����������͵�app�豸���棬��΢���޸���
	bool PushMsg:1;//����
	bool xxxxIsVid:1;//��1ʱ��Param���ݵ���vid�����򴫵�ʵ����ֵ����ʱ����
	bool xxxxDevMsg:1;//��1ʱ����ʾ��Ϣ�����豸��UserID�洢�豸id����ʱ����
	u8 a:2;
	u8 b;
	TVAR xxxxParam; //��Ҫ���ݵı���id��ֵ����ʱ����
}EXC_STR_ITEM;

typedef struct{
	SCENE_ITEM_ACT Act;//���� 
	u8 a;
	u16 DelaySec;//ִ����Ϻ����ʱ
	union{
		EXC_KEY_ITEM Key;//������ֵ���豸
		EXC_VAR_ITEM Var;//�趨����ֵ
		EXC_STR_ITEM Str;//��ͥ��Ϣ��ϵͳ��Ϣ
		ID_T SceneID;//ִ��ָ�����龰
	}Exc;
}SCENE_ITEM;

typedef struct{
	u32 ID;
	u8 Name[SHORT_STR_BYTES];
	SCENE_ITEM Items[SCENE_MAX_ITEM_NUM];
}SCENE_RECORD;

bool SceneItemOk(SCENE_ITEM *pItem);

#endif

