#ifndef SCENE_EXCUTE_H
#define SCENE_EXCUTE_H



typedef enum{
	SER_NULL=0,
	SER_OK,
	SER_FAILD,
	SER_TIMEOUT
	
}SCENE_EXC_RES;//ִ�н��

typedef struct{
	u8 ItemNum;
	SCENE_EXC_RES *pWaitFlag;//ָ�򷵻ؽ���ı���
	SCENE_ITEM Items[1];
}SCENE_EXCUTE_INFO;

SCENE_EXC_RES CallSceneExcute(u8 ItemNum,SCENE_ITEM *pItems,u32 Timeout);
SCENE_EXC_RES CallSceneExcuteOne(ID_T ScnID,u32 TimeoutMs);

#endif

