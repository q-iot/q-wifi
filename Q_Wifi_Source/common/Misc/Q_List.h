#ifndef _Q_LIST_H_
#define _Q_LIST_H_

typedef bool (*pFilterFunc)(void *,void *);
typedef bool (*pFreeFunc)(void *);

typedef struct{
	u32 ID;
	void *pNext;//�ڲ�ʹ�ã��ⲿ����ʹ��
	u8 Data[4];
}QLIST_ITEM_H;//���е�Ԫͷ��������Ӵ˽ṹ��ItemSize�����˴˲���

typedef struct{
	OS_SEM_T MutexLock;
	QLIST_ITEM_H *pHeader;//����ͷ
	QLIST_ITEM_H *pLast;//����β
	pFreeFunc pFreeCb;//��Դ�ͷź���
	u16 ItemCount;//���е�����Ŀ
	u16 ItemSize;//��Ԫ�ߴ�
	u16 MaxNum;//������ɵ�Ԫ��
}QLIST_INFO;

typedef struct{
	u8 Offset;//��item�е�ƫ��ֵ
	u8 Bytes;//���Ա�������
	u32 Mask;//����
}QLIST_ATTRIB_OPT;


void QList_Debug(QLIST_INFO *pList);
bool QList_New(QLIST_INFO *pList,u16 ItemSize,u16 MaxNum,pFreeFunc pFreeCb);
bool QList_Destory(QLIST_INFO *pList);
u16 QList_AddItem(QLIST_INFO *pList,void *pItem);
u16 QList_DeleteByFilter(QLIST_INFO *pList,pFilterFunc pFilter,void *pCompare,u16 DelNum);
bool QList_ReadItemByFilter(QLIST_INFO *pList,pFilterFunc pFilter,void *pCompare,void *pRetItem);


#endif

