#ifndef _Q_LIST_H_
#define _Q_LIST_H_

typedef bool (*pFilterFunc)(void *,void *);
typedef bool (*pFreeFunc)(void *);

typedef struct{
	u32 ID;
	void *pNext;//内部使用，外部不能使用
	u8 Data[4];
}QLIST_ITEM_H;//所有单元头都必须遵从此结构，ItemSize包含了此部分

typedef struct{
	OS_SEM_T MutexLock;
	QLIST_ITEM_H *pHeader;//队列头
	QLIST_ITEM_H *pLast;//队列尾
	pFreeFunc pFreeCb;//资源释放函数
	u16 ItemCount;//队列当下数目
	u16 ItemSize;//单元尺寸
	u16 MaxNum;//最大容纳单元数
}QLIST_INFO;

typedef struct{
	u8 Offset;//在item中的偏移值
	u8 Bytes;//属性变量长度
	u32 Mask;//掩码
}QLIST_ATTRIB_OPT;


void QList_Debug(QLIST_INFO *pList);
bool QList_New(QLIST_INFO *pList,u16 ItemSize,u16 MaxNum,pFreeFunc pFreeCb);
bool QList_Destory(QLIST_INFO *pList);
u16 QList_AddItem(QLIST_INFO *pList,void *pItem);
u16 QList_DeleteByFilter(QLIST_INFO *pList,pFilterFunc pFilter,void *pCompare,u16 DelNum);
bool QList_ReadItemByFilter(QLIST_INFO *pList,pFilterFunc pFilter,void *pCompare,void *pRetItem);


#endif

