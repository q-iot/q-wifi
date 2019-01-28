#ifndef QUICK_SAVE_H
#define QUICK_SAVE_H

#include "LimitMarco.h"

typedef enum {
	QIN_NULL=0,
	QIN_OBJ_ID_BASE,
	QIN_DEBUG_FLAG,
	QIN_INFO_FLAG,
	QIN_SCN_FLAG,
	QIN_SCN_SET,
	QIN_SCN_CLR,
	
	QIN_MAX
}QSS_ITEM_NAME;

typedef struct{	
	u32 Flag;//��ȡ��־�������û���Ԥ
	u32 Ver;//�汾�������û���Ԥ
	u32 ChkSum;//У��ͣ������û���Ԥ
	
	//�û���������
	u16 ObjIdBase;
	u32 DebugFlag;
	u32 InfoFlag[IFT_SAVE_NUM];

	//�龰ģʽ���
	u32 ScnDefend[SCENE_MAX_NUM];
	u16 ScnNum;	

	u32 Tail;//����spi fls write 4bytes align
}QUICK_SAVE_STRUCT;	//���ݿ�


void QSS_DbInit(void);
u32 QSS_GetValue(u16 Item,u32 IntParam,void *pVal);
bool QSS_SetValue(u16 Item,u32 IntParam,void *pParam,u16 ByteLen);
void QSS_Default(void);


#endif

