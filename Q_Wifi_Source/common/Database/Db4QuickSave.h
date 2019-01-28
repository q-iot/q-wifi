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
	u32 Flag;//读取标志，无需用户干预
	u32 Ver;//版本，无需用户干预
	u32 ChkSum;//校验和，无需用户干预
	
	//用户数据区域
	u16 ObjIdBase;
	u32 DebugFlag;
	u32 InfoFlag[IFT_SAVE_NUM];

	//情景模式相关
	u32 ScnDefend[SCENE_MAX_NUM];
	u16 ScnNum;	

	u32 Tail;//兼容spi fls write 4bytes align
}QUICK_SAVE_STRUCT;	//数据库


void QSS_DbInit(void);
u32 QSS_GetValue(u16 Item,u32 IntParam,void *pVal);
bool QSS_SetValue(u16 Item,u32 IntParam,void *pParam,u16 ByteLen);
void QSS_Default(void);


#endif

