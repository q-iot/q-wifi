#include "SysDefines.h"
#include "Q_List.h"

//构建一个可以被上层调用的，线程安全的队列机制





#define B2U_24(p,Offset) ((((u8 *)(p))[(Offset)])<<24)
#define B2U_16(p,Offset) ((((u8 *)(p))[(Offset+1)])<<16)
#define B2U_8(p,Offset) ((((u8 *)(p))[(Offset+2)])<<8)
#define B2U_0(p,Offset) ((((u8 *)(p))[(Offset+3)]))
#define Bytes2U32(p,Offset) (B2U_24(p,Offset)+B2U_16(p,Offset)+B2U_8(p,Offset)+B2U_0(p,Offset))



#define NewLock(pList) do{pList->MutexLock=OS_SemCreateMutex();}while(0);
#define Lock(pList) OS_SemTake(pList->MutexLock,OS_MAX_DELAY)
#define Unlock(pList) OS_SemGive(pList->MutexLock)

//应用层调用的打印函数
void QList_Debug(QLIST_INFO *pList)
{
	QLIST_ITEM_H *p=NULL;

	if((pList==NULL) || (pList->MutexLock==NULL)) return;

	Lock(pList);
	Debug("ItemSize:%u, ItemNum:%u, Max:%u\n\r",pList->ItemSize,pList->ItemCount,pList->MaxNum);

	p=pList->pHeader;
	while(p)
	{	
		//信息输出
		Debug("[%u]%u(0x%x)\n\r",p->ID,*(u32 *)(p->Data),*(u32 *)(p->Data));

		if(p->pNext==NULL && p!=pList->pLast) Debug("List tail error!\n\r");

		p=p->pNext;
	}

	Unlock(pList);
}

//新建一个list队列
//确定单元大小
//确定存储上限
//返回头指针
bool QList_New(QLIST_INFO *pList,u16 ItemSize,u16 MaxNum,pFreeFunc pFreeCb)
{
	if( (pList==NULL) 
		||(pList->MutexLock!=NULL) 
		||(ItemSize==0) 
		||(MaxNum==0||MaxNum>100) )
	{
		Debug("[QLI]new error!\n\r");
		return FALSE;
	}
	
	NewLock(pList);
	pList->pHeader=NULL;
	pList->pLast=NULL;
	pList->pFreeCb=pFreeCb;
	pList->ItemCount=0;
	pList->ItemSize=ItemSize;
	pList->MaxNum=MaxNum;

	return TRUE;
}

//销毁队列
bool QList_Destory(QLIST_INFO *pList)
{
	return TRUE;
}

//加入item到队列
//返回队列单元数
//失败返回0
u16 QList_AddItem(QLIST_INFO *pList,void *pItem)
{
	QLIST_ITEM_H *pNew=NULL;
	QLIST_ITEM_H *p=NULL;
	u16 Count=0;
	
	if( (pList==NULL) 
		||(pList->MutexLock==NULL)
		||(pItem==NULL) )
	{
		Debug("[QLI]addi error!\n\r");
		return 0;
	}
	
	Lock(pList);
	if(pList->ItemCount >= pList->MaxNum)
	{
		Unlock(pList);
		Debug("[QLI]addi max error!\n\r");
		return 0;
	}

	pNew=Q_Malloc(pList->ItemSize);
	MemCpy(pNew,pItem,pList->ItemSize);	
	pNew->pNext=NULL;

	if(pList->pHeader==NULL)
	{
		pList->pHeader=pNew;
		pList->pLast=pNew;
	}
	else
	{
		p=pList->pLast;
		p->pNext=pNew;
		pList->pLast=pNew;
	}

	pList->ItemCount++;
	Count=pList->ItemCount;
	
	Unlock(pList);
	return Count;
}

//删除一个对象
//返回删除的个数
//DelNum=0表示一直删
u16 QList_DeleteByFilter(QLIST_INFO *pList,pFilterFunc pFilter,void *pCompare,u16 DelNum)
{
	QLIST_ITEM_H *p=NULL;
	QLIST_ITEM_H *pPre=NULL;
	u16 Count=0;
	
	if( (pList==NULL)
		||(pList->MutexLock==NULL)
		||(pFilter==NULL) )
	{
		Debug("[QLI]Delf error!\n\r");
		return 0;
	}

	Lock(pList);
	if(pList->pHeader==NULL)
	{
		Unlock(pList);
		return 0;
	}

	p=pList->pHeader;
	while(p)
	{	
		if(pFilter(p,pCompare)==TRUE)
		{
			if(pPre!=NULL) pPre->pNext=p->pNext;
			if(p==pList->pHeader) pList->pHeader=p->pNext;
			if(p==pList->pLast) pList->pLast=pPre;

			if(pList->pFreeCb!=NULL) pList->pFreeCb(p);
			Q_Free(p);
			pList->ItemCount--;
			Count++;
			if(DelNum && Count>=DelNum)
			{
				break;
			}
			else 
			{
				p=pPre?pPre->pNext:pList->pHeader;
				continue;
			}
		}

		pPre=p;
		p=p->pNext;
	}

	Unlock(pList);
	return Count;
}


//根据条件获取一个单元，采用复制的方式
//读取到了返回true
//pFilter获取属性的过滤器
//pCompare条件值，传入到pFilter函数
//pRetItem拷贝返回的item
bool QList_ReadItemByFilter(QLIST_INFO *pList,pFilterFunc pFilter,void *pCompare,void *pRetItem)
{
	QLIST_ITEM_H *p=NULL;

	if( (pList==NULL)
		||(pList->MutexLock==NULL)
		||(pFilter==NULL)
		||(pCompare==NULL)
		||(pRetItem==NULL) )
	{
		Debug("[QLI]Readif error!\n\r");
		return FALSE;
	}

	Lock(pList);
	if(pList->pHeader==NULL)
	{
		Unlock(pList);
		return FALSE;
	}

	p=pList->pHeader;
	while(p)
	{
		if(pFilter(p,pCompare)==TRUE)
		{
			MemCpy(pRetItem,p,pList->ItemSize);
			Unlock(pList);
			return TRUE;
		}
	
		p=p->pNext;
	}

	Unlock(pList);
	return FALSE;
}

//根据条件获取一个单元，采用复制的方式
//读取到了返回true
//pOpt获取属性的条件
//pVal属性值
//pRetItem拷贝返回的item
bool QList_ReadItemByOpt(QLIST_INFO *pList,QLIST_ATTRIB_OPT *pOpt,void *pCompVal,void *pRetItem)
{
	QLIST_ITEM_H *p=NULL;

	if( (pList==NULL)
		||(pList->MutexLock==NULL)
		||(pOpt==NULL)
		||(pCompVal==NULL)
		||(pRetItem==NULL) )
	{
		Debug("[QLI]Readi error!\n\r");
		return FALSE;
	}

	Lock(pList);
	if(pList->pHeader==NULL)
	{
		Unlock(pList);
		return FALSE;
	}

	p=pList->pHeader;
	while(p)
	{
		if(pOpt->Bytes==1)
		{
			u8 Val=*(u8 *)pCompVal;
			u8 ItemAttrib;
			MemCpy(&ItemAttrib,p+pOpt->Offset,pOpt->Bytes);
			if((ItemAttrib&pOpt->Mask) == (Val&pOpt->Mask))
			{
				MemCpy(pRetItem,p,pList->ItemSize);
				Unlock(pList);
				return TRUE;
			}
		}
		else if(pOpt->Bytes==2)
		{
			u16 Val=*(u16 *)pCompVal;
			u16 ItemAttrib;
			MemCpy(&ItemAttrib,p+pOpt->Offset,pOpt->Bytes);
			if((ItemAttrib&pOpt->Mask) == (Val&pOpt->Mask))
			{
				MemCpy(pRetItem,p,pList->ItemSize);
				Unlock(pList);
				return TRUE;
			}
		}
		else if(pOpt->Bytes==4)
		{
			u32 Val=*(u32 *)pCompVal;
			u32 ItemAttrib;
			MemCpy(&ItemAttrib,p+pOpt->Offset,pOpt->Bytes);
			if((ItemAttrib&pOpt->Mask) == (Val&pOpt->Mask))
			{
				MemCpy(pRetItem,p,pList->ItemSize);
				Unlock(pList);
				return TRUE;
			}
		}
		else
		{
			Unlock(pList);
			Debug("[QLI]Readi error2!\n\r");
			return FALSE;
		}
	
		p=p->pNext;
	}

	Unlock(pList);
	return FALSE;
}

//通过id获取item
//成功返回true
bool QList_ReadItemByID(QLIST_INFO *pList,u32 ID,void *pRetItem)
{
	QLIST_ATTRIB_OPT Opt={0,4,0xffffffff};
	return QList_ReadItemByOpt(pList,&Opt,&ID,pRetItem);
}

//返回第一个item
//成功返回true
bool QList_GetFirst(QLIST_INFO *pList,void *pRetItem)
{
	if( (pList==NULL)
		||(pList->MutexLock==NULL)
		||(pRetItem==NULL) )
	{
		Debug("[QLI]Readf error!\n\r");
		return FALSE;
	}

	Lock(pList);
	if(pList->pHeader==NULL)
	{
		MemSet(pRetItem,0,pList->ItemSize);
		Unlock(pList);
		return FALSE;
	}

	MemCpy(pRetItem,pList->pHeader,pList->ItemSize);
	Unlock(pList);
	return TRUE;
}

//返回最后一个item
//成功返回true
bool QList_GetLast(QLIST_INFO *pList,void *pRetItem)
{
	if( (pList==NULL)
		||(pList->MutexLock==NULL)
		||(pRetItem==NULL) )
	{
		Debug("[QLI]Readf error!\n\r");
		return FALSE;
	}

	Lock(pList);
	if(pList->pLast==NULL)
	{
		MemSet(pRetItem,0,pList->ItemSize);
		Unlock(pList);
		return FALSE;
	}

	MemCpy(pRetItem,pList->pLast,pList->ItemSize);
	Unlock(pList);
	return TRUE;
}

//根据id搜寻下一个item
//成功返回true
bool QList_GetNext(QLIST_INFO *pList,u32 NowID,void *pRetItem)
{
	QLIST_ITEM_H *p=NULL;

	if( (pList==NULL)
		||(pList->MutexLock==NULL)
		||(NowID==0)
		||(pRetItem==NULL) )
	{
		Debug("[QLI]Readn error!\n\r");
		return FALSE;
	}

	Lock(pList);
	if(pList->pHeader==NULL)
	{
		Unlock(pList);
		return FALSE;
	}

	p=pList->pHeader;
	while(p)
	{
		if(p->ID==NowID)
		{
			if(p->pNext==NULL)
			{
				Unlock(pList);
				return FALSE;
			}
			else
			{
				MemCpy(pRetItem,p->pNext,pList->ItemSize);
				Unlock(pList);
				return TRUE;
			}
		}

		p=p->pNext;
	}

	Unlock(pList);
	return FALSE;
}

//根据id获取一个单元的指定数据
bool QList_ReadAttrib(QLIST_INFO *pList,u32 ID,QLIST_ATTRIB_OPT *pOpt,void *pAttribRet)
{
	QLIST_ITEM_H *p=NULL;

	if( (pList==NULL)
		||(pList->MutexLock==NULL)
		||(ID==0) 		
		||(pOpt==NULL)
		||(pAttribRet==NULL) )
	{
		Debug("[QLI]Reada error!\n\r");
		return FALSE;
	}

	Lock(pList);
	if(pList->pHeader==NULL)
	{
		Unlock(pList);
		return FALSE;
	}

	p=pList->pHeader;
	while(p)
	{
		if(p->ID == ID)
		{
			if(pOpt->Bytes==1)
			{
				MemCpy(pAttribRet,p+pOpt->Offset,pOpt->Bytes);
				*(u8 *)pAttribRet=*(u8 *)pAttribRet & pOpt->Mask;
				Unlock(pList);
				return TRUE;
			}
			else if(pOpt->Bytes==2)
			{
				MemCpy(pAttribRet,p+pOpt->Offset,pOpt->Bytes);
				*(u16 *)pAttribRet=*(u16 *)pAttribRet & pOpt->Mask;
				Unlock(pList);
				return TRUE;
			}
			else if(pOpt->Bytes==4)
			{
				MemCpy(pAttribRet,p+pOpt->Offset,pOpt->Bytes);
				*(u32 *)pAttribRet=*(u32 *)pAttribRet & pOpt->Mask;
				Unlock(pList);
				return TRUE;
			}
			else
			{
				Unlock(pList);
				Debug("[QLI]Reada error2!\n\r");
				return FALSE;
			}
		}
		
		p=p->pNext;
	}

	Unlock(pList);
	return FALSE;
}

//更新一个单元的指定数据
bool QList_UpdateAttrib(QLIST_INFO *pList,u32 ID,QLIST_ATTRIB_OPT *pOpt,void *pNewVal)
{
	QLIST_ITEM_H *p=NULL;

	if( (pList==NULL)
		||(pList->MutexLock==NULL)
		||(ID==0) 		
		||(pOpt==NULL)
		||(pNewVal==NULL) )
	{
		Debug("[QLI]Readu error!\n\r");
		return FALSE;
	}

	Lock(pList);
	if(pList->pHeader==NULL)
	{
		Unlock(pList);
		return FALSE;
	}

	p=pList->pHeader;
	while(p)
	{
		if(p->ID == ID)
		{
			if(pOpt->Bytes==1)
			{
				u8 New=(*(u8 *)pNewVal) & pOpt->Mask;
				u8 Old;
				
				MemCpy(&Old,p+pOpt->Offset,pOpt->Bytes);
				New=New|(Old&~pOpt->Mask);
				MemCpy(p+pOpt->Offset,&New,pOpt->Bytes);
				
				Unlock(pList);
				return TRUE;
			}
			else if(pOpt->Bytes==2)
			{
				u16 New=(*(u16 *)pNewVal) & pOpt->Mask;
				u16 Old;
				
				MemCpy(&Old,p+pOpt->Offset,pOpt->Bytes);
				New=New|(Old&~pOpt->Mask);
				MemCpy(p+pOpt->Offset,&New,pOpt->Bytes);
				
				Unlock(pList);
				return TRUE;
			}
			else if(pOpt->Bytes==4)
			{
				u32 New=(*(u32 *)pNewVal) & pOpt->Mask;
				u32 Old;
				
				MemCpy(&Old,p+pOpt->Offset,pOpt->Bytes);
				New=New|(Old&~pOpt->Mask);
				MemCpy(p+pOpt->Offset,&New,pOpt->Bytes);
				
				Unlock(pList);
				return TRUE;
			}
			else
			{
				Unlock(pList);
				Debug("[QLI]Readu error2!\n\r");
				return FALSE;
			}
		}
		
		p=p->pNext;
	}

	Unlock(pList);
	return FALSE;
}

//检查是否触及存储上限
bool QList_IsFull(QLIST_INFO *pList)
{	
	if( (pList==NULL)
 		||(pList->MutexLock==NULL) )
	{
		Debug("[QLI]full error!\n\r");
		return FALSE;
	}

	Lock(pList);
	if(pList->ItemCount==pList->MaxNum)
	{
		Unlock(pList);
		return TRUE;
	}
	else if(pList->ItemCount>pList->MaxNum)
	{
		Unlock(pList);
		Debug("[QLI]full error2!\n\r");
		return FALSE;
	}

	Unlock(pList);
	return FALSE;
}

//自定义的测试函数
void QList_Test(QLIST_INFO *pList)
{

}




