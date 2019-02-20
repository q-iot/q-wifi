#include "esp_common.h"
#include "PublicFunc.h"
#include "Q_Heap.h"
#include "Os_Wrap.h"

#if USE_Q_HEAP //Ϊ����ϵͳheap5����

#ifndef NULL
#define NULL ((void *)0)
#endif

#ifdef Debug
#undef Debug
#endif
#define Debug printf

#if Q_HEAP_DEBUG == 1 || Q_HEAP_DEBUG == 3
#define QH_Debug Debug
#else
#define QH_Debug(x,y...)
#endif

#define Q_HEAP_PROG_CHECK 1//�Ƿ���������

//Ϊ��֤���߳��¶ѷ����ԭ�Ӳ����������������ϵͳ�ٽ���
//����˻��Ʋ������ڶ��̣߳�����Զ���������������Ϊ�հ�
#define Q_HEAP_CRIT_SAVE_DATA_DEFINE 	//OS_CPU_SR cpu_sr
#define Q_HEAP_ENTER_CRIT ETS_INTR_LOCK()
#define Q_HEAP_EXIT_CRIT ETS_INTR_UNLOCK()

//������֤�ڴ��Ƿ񱻳�ٵı�ʶ
//ռ��2���ֽڣ������ڴ�ǰ�󱻳�ٶ����ܲ���ĸ���Ϊ65535��֮1
#define Q_HEAP_RW_FLAG 0x5808 //������ÿ���������ڴ��ĩβ�ĳ�ٱ�ʶ
#define Q_HEAP_RW_FLAG32 ((Q_HEAP_RW_FLAG<<16)|0x1234) //heap�����ڴ�����ǰ��ĳ�ٱ�ʶ����Q_HEAP_UNIT_REWRITE_ID�ŵ���16λ��Ϊ�˷�ֹ��һ�����鱻�ͷ���ɵļ������

//��¼��
typedef struct{
	u16 RwFlag;//����ٱ�־
	u16 MemIsIdle:1;//�ڴ��Ƿ����
	u16 AsynFree:1;//��Ҫ�첽�ͷ�
	u16 Line:14;//���õ��к�
	u32 Bytes;//Data���������ֽ�����������ż��
	void *pPrev;//ǰһ��
	void *pNext;//��һ��
#if Q_HEAP_DEBUG > 1
	const char *pFunc;//���õĺ�����
#endif
	u16 Data[2];//�ڴ棬��������ֽڷŷ���ٱ�־
}QHEAP_ITEM;

#if Q_HEAP_DEBUG > 1
#define QH_ITEM_COMM_BYTES 20//Data֮ǰ�Ĵ�С
#define QH_ITEM_MIN_OCCUPY 36//һ��item��Ҫ����Сռ��bytes������ͷβ
#else
#define QH_ITEM_COMM_BYTES 16//Data֮ǰ�Ĵ�С
#define QH_ITEM_MIN_OCCUPY 32//һ��item��Ҫ����Сռ��bytes������ͷβ
#endif

//����ѵ�ַ
#define QH_HEAP_MEM_NUM 2
static u32 *pQHeap[QH_HEAP_MEM_NUM];//������8�ֽڶ���
static u32 *pQHeapEnd[QH_HEAP_MEM_NUM];
static QHEAP_ITEM *gpItemHdr=NULL;

//��ʵ��
extern char _heap_start; //_heap_start - 0x40000000
extern char _lit4_end; //_lit4_end - 0x4010C000 iram

//����
void QHeapDebug(void)
{
	QHEAP_ITEM *pItem=NULL;
	QHEAP_ITEM *p=NULL;
	u16 m;
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;

	Debug("-------------------------------------------------------------------------\n\r");
	Q_HEAP_ENTER_CRIT;	
	for(m=0;m<QH_HEAP_MEM_NUM;m++)
	{
		Debug("Heap[%u] 0x%x - 0x%x %u Byts\n\r",m,pQHeap[m],pQHeapEnd[m],(u32)pQHeapEnd[m]-(u32)pQHeap[m]);
	}

	Debug("\n\r");
	
	pItem=gpItemHdr;
	while(pItem)
	{
#if Q_HEAP_DEBUG > 1
		Debug("%8x<- %s Addr:%x - %05x p%x%c= %5uB ->%x %s:%u\n\r",pItem->pPrev,pItem->MemIsIdle?"[]":"XX",
			(u32)pItem,((u32)pItem->Data+pItem->Bytes)&0xfffff,(u32)pItem->Data,pItem->AsynFree?'*':' ',
			pItem->Bytes,pItem->pNext,pItem->pFunc,pItem->Line);
#else
		Debug("%8x<- %s Addr:%x - %05x p%x%c= %5uB ->%x\n\r",pItem->pPrev,pItem->MemIsIdle?"[]":"XX",
			(u32)pItem,((u32)pItem->Data+pItem->Bytes)&0xfffff,(u32)pItem->Data,pItem->AsynFree?'*':' ',
			pItem->Bytes,pItem->pNext);
#endif

#if Q_HEAP_PROG_CHECK //�������ָ��׼ȷ��
		if(p=pItem->pNext)
		{
			if(p->pPrev!=pItem) Debug("PrevPoint ERROR!\n\r");
			if(((u32)(pItem->Data)+pItem->Bytes)!=(u32)p) Debug("- ADDR GAP -\n\r");
		}
#endif		
		
		pItem=pItem->pNext;
	}	
	Q_HEAP_EXIT_CRIT;
	Debug("Total Idle Memory %u Byts\n\r",QHeapGetIdleSize());
	Debug("-------------------------------------------------------------------------\n\r\n\r");
}

//��ʼ��
static void QHeapInit(void)
{
	u16 m;
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;

	Q_HEAP_ENTER_CRIT;

	//����ѵ�ַ
	#if 1//��ƽ̨��ͬ
	{
		pQHeap[0]=(void *)AlignTo4((u32)&_heap_start);
		*pQHeap[0]++=Q_HEAP_RW_FLAG32;
		pQHeapEnd[0]=(void *)0x40000000;
		
		pQHeap[1]=(void *)AlignTo4((u32)&_lit4_end);
		*pQHeap[1]++=Q_HEAP_RW_FLAG32;
		pQHeapEnd[1]=(void *)0x4010C000;
	}
	#endif

	//������¼
	for(m=0;m<QH_HEAP_MEM_NUM;m++)
	{
		QHEAP_ITEM *pItem=(void *)pQHeap[m];

		pItem->RwFlag=Q_HEAP_RW_FLAG;
		pItem->MemIsIdle=TRUE;
		pItem->AsynFree=FALSE;
		pItem->Bytes=(u32)pQHeapEnd[m]-(u32)pQHeap[m]-QH_ITEM_COMM_BYTES;//����λ���㣬���ܱ�ʵ�ʿռ�С
		pItem->pPrev=(m?pQHeap[m-1]:NULL);
		pItem->pNext=((m==(QH_HEAP_MEM_NUM-1))?NULL:pQHeap[m+1]);
		//MemSet(pItem->Data,0,pItem->Bytes);//esp����memset�����ܷɣ���֪��Ϊʲô
#if Q_HEAP_DEBUG > 1
		pItem->pFunc="QHeapInit";
		pItem->Line=0;
#endif		
	}

	gpItemHdr=(void *)pQHeap[0];

	//QHeapDebug();

	Q_HEAP_EXIT_CRIT;
}

//����ٱ�־
static bool _CheckRwFlag(void)
{
	u16 i;

	for(i=0;i<QH_HEAP_MEM_NUM;i++)
	{
		u32 *p=pQHeap[i];
		
		if(*--p != Q_HEAP_RW_FLAG32) return FALSE;
	}

	return TRUE;
}

//����һ����
static void *_RequItem(QHEAP_ITEM *pItem,u32 Size)
{
	if((Size+QH_ITEM_MIN_OCCUPY) <= pItem->Bytes)//ʣ�µĿռ��㹻���½�һ��
	{
		QHEAP_ITEM *pNew=NULL;
		QHEAP_ITEM *pNext=NULL;

		//�½�һ�����п飬�ӵ�ԭ��item�ĺ���
		pNew=(void *)&pItem->Data[Size>>1];	
		pNew->RwFlag=Q_HEAP_RW_FLAG;
		pNew->MemIsIdle=TRUE;
		pNew->AsynFree=FALSE;
		pNew->Bytes=pItem->Bytes-Size-QH_ITEM_COMM_BYTES;
		pNew->pPrev=pItem;
		pNew->pNext=pItem->pNext;
#if Q_HEAP_DEBUG > 1	
		pNew->pFunc="Idle";
		pNew->Line=0;
#endif
		//����ɵĲ�����
		pNext=pItem->pNext;
		if(pNext) pNext->pPrev=pNew;
		
		pItem->MemIsIdle=FALSE;
		pItem->AsynFree=FALSE;
		pItem->Bytes=Size;
		pItem->pNext=pNew;
		pItem->Data[(pItem->Bytes>>1)-1]=Q_HEAP_RW_FLAG;
		
		return pItem->Data;
	}
	else//ʣ�µĿռ䲻�������������½��ˣ�ֱ������
	{
		pItem->MemIsIdle=FALSE;
		pItem->AsynFree=FALSE;
		pItem->Data[(pItem->Bytes>>1)-1]=Q_HEAP_RW_FLAG;
		return pItem->Data;
	}
}

//�ͷ�һ����
static void _ReleseItem(QHEAP_ITEM *pItem)
{
	QHEAP_ITEM *pNext;
	QHEAP_ITEM *pPrev;
	bool HasMerge=FALSE;

	pItem->MemIsIdle=TRUE;
	pItem->AsynFree=FALSE;
	
#if Q_HEAP_DEBUG > 1	
	pItem->pFunc="Idle";
	pItem->Line=0;
#endif

	//�鿴����һ�����ǲ���Ϊ����
	pNext=pItem->pNext;
	if(pNext && pNext->MemIsIdle==TRUE)
	{
		if(((u32)pItem->Data+pItem->Bytes) == (u32)pNext)//��ַ�������ģ����кϲ�
		{
			pItem->MemIsIdle=TRUE;
			pItem->AsynFree=FALSE;
			pItem->Bytes+=(QH_ITEM_COMM_BYTES+pNext->Bytes);
			pItem->pNext=pNext->pNext;

			pNext=pItem->pNext;
			if(pNext) pNext->pPrev=pItem;
		}
	}

	//�鿴ǰ��һ�����ǲ���Ϊ����
	pPrev=pItem->pPrev;
	if(pPrev && pPrev->MemIsIdle==TRUE)
	{
		if(((u32)pPrev->Data+pPrev->Bytes) == (u32)pItem)//��ַ�������ģ����кϲ�
		{
			pPrev->Bytes+=(QH_ITEM_COMM_BYTES+pItem->Bytes);
			pPrev->pNext=pItem->pNext;

			pItem=pPrev->pNext;
			if(pItem) pItem->pPrev=pPrev;
			
			pItem=pPrev;
		}
	}

	//MemSet(pItem->Data,0,pItem->Bytes);//��֤��һ�η����ȥ��Ҳ��0
}

#if Q_HEAP_DEBUG > 1
void *_Q_Malloc(u16 Size,const char *pFuncCaller,u32 Line)
#else
void *_Q_Malloc(u16 Size)
#endif
{
	static bool IsFirst=TRUE;
	QHEAP_ITEM *pItem=NULL;
	u16 *pMem=NULL;
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;

	if(IsFirst)
	{
		QHeapInit();
		IsFirst=FALSE;
	}

	if(Size==0) return NULL;

	Q_HEAP_ENTER_CRIT;	
	if(_CheckRwFlag()==FALSE)//���ǰ�����ٱ�־
	{
		Debug("Heap RwFlag Error!\n\r");
		while(1);
	}

	//���������ʵ�ʴ�С
	Size=AlignTo4(Size+2);//����β��У��

#if Q_HEAP_DEBUG >1 
	QH_Debug("# Malco %3uB From %s:%u   ",Size,pFuncCaller,Line);
#else
	QH_Debug("# Malco %3uB   ",Size);
#endif

	//�Һ��ʵĿ�
	pItem=gpItemHdr;
	while(pItem)
	{
		if(pItem->RwFlag!=Q_HEAP_RW_FLAG)
		{
			Debug("Heap Hd Rw!\n\r");
			while(1);
		}

		if(pItem->MemIsIdle==FALSE && pItem->Data[(pItem->Bytes>>1)-1]!=Q_HEAP_RW_FLAG)
		{
			Debug("Heap End Rw!\n\r");
			while(1);
		}

		if(pItem->MemIsIdle && pItem->Bytes>=Size)//�ҵ����п飬������ڵ��������С
		{
			pMem=_RequItem(pItem,Size);//����һ����
			MemSet(pItem->Data,0,pItem->Bytes-2);//β��Ԥ�������
			
#if Q_HEAP_DEBUG > 1
			pItem->pFunc=pFuncCaller;
			pItem->Line=Line;
#endif

#if Q_HEAP_PROG_CHECK //����ǲ��Ƕ�0������־
			{
				u16 i;

				if(pItem->Bytes < Size)
				{
					Debug("Malloc %uB size error\n\r",Size);
					while(1);
				}

				for(i=0;i<((pItem->Bytes>>1)-1);i++)
				{
					if(pMem[i]!=0) 
					{
						Debug("Malloc %uB not init 0\n\r",Size);
						while(1);
					}
				}

				if(pMem[(pItem->Bytes>>1)-1]!=Q_HEAP_RW_FLAG)//���β�������
				{
					Debug("Malloc %uB rwflag error\n\r",Size);
					QHeapDebug();
					while(1);
				}
			}
#endif

			Q_HEAP_EXIT_CRIT;
			QH_Debug("p%x\n\r",pMem);
			return pMem;
		}

		pItem=pItem->pNext;
	}	
	Q_HEAP_EXIT_CRIT;

	Debug("!!!No Get Heap,Size:%d!!!\n\r",Size);
	QHeapDebug();
	return NULL;
}

#if Q_HEAP_DEBUG > 1
void *_Q_MallocAsyn(u16 Size,const char *pFuncCaller,u32 Line)
#else
void *_Q_MallocAsyn(u16 Size)
#endif
{
	void *pMem=NULL;
	
#if Q_HEAP_DEBUG > 1
	pMem=_Q_Malloc(Size,pFuncCaller,Line);
#else
	pMem=_Q_Malloc(Size);
#endif

	if(pMem!=NULL)
	{
		QHEAP_ITEM *pItem=(void *)((u32)pMem-QH_ITEM_COMM_BYTES);
		pItem->AsynFree=TRUE;
	}

	return pMem;
}

#if Q_HEAP_DEBUG > 1
void _Q_Free(void *pMem,const char *pFuncName,u32 Line)
#else
void _Q_Free(void *pMem)
#endif
{
	QHEAP_ITEM *pItem=NULL;
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;

	if(pMem==NULL) return;
	if(IsHeapRam(pMem)==FALSE) return;
	
	Q_HEAP_ENTER_CRIT;
	if(_CheckRwFlag()==FALSE)//���ǰ�����ٱ�־
	{
		Debug("Heap Free RwFlag Error!\n\r");
		QHeapDebug();
		while(1);
	}

	pItem=(void *)((u32)pMem-QH_ITEM_COMM_BYTES);

#if Q_HEAP_DEBUG >1 
	QH_Debug("# Free  %3uB %s:%u For %s:%u   p%x\n\r",pItem->Bytes,pFuncName,Line,pItem->pFunc,pItem->Line,pMem);
#else
	QH_Debug("# Free  %3uB   p%x\n\r",pItem->Bytes,pMem);
#endif

	if(pItem->RwFlag!=Q_HEAP_RW_FLAG)//���ͷ�������
	{
		Debug("Heap Free %uB Hd Rw!\n\r",pItem->Bytes);
		QHeapDebug();
		while(1);
	}
	
	if(pItem->MemIsIdle!=FALSE)
	{
		Debug("Heap Free %uB Idle error\n\r",pItem->Bytes);
		QHeapDebug();
		while(1);
	}
	
	if(pItem->Data[(pItem->Bytes>>1)-1]!=Q_HEAP_RW_FLAG)//���β�������
	{
		Debug("Heap Free %uB End Rw!\n\r",pItem->Bytes);
		QHeapDebug();
		while(1);
	}

	_ReleseItem(pItem);

	Q_HEAP_EXIT_CRIT;
}

//Ϊ�ڴ�����õ�������
void QHeapMemSetName(void *pMem,const char *pName)
{
#if Q_HEAP_DEBUG > 1
	QHEAP_ITEM *pItem=NULL;
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;
	
	if(pMem==NULL) return;
	if(IsHeapRam(pMem)==FALSE) return;
	
	//��ǰ��itemͷ
	pMem=(void *)AlignTo4((u32)pMem);
	
	Q_HEAP_ENTER_CRIT;
	{
		u16 *p=pMem;

FindHd:
		while(*p!=Q_HEAP_RW_FLAG) p--;

		pItem=(void *)p;
		if(pItem->MemIsIdle!=FALSE){p--;goto FindHd;}
		if(pItem->Data[(pItem->Bytes>>1)-1]!=Q_HEAP_RW_FLAG){p--;goto FindHd;}

		pItem->pFunc=pName;
	}
	Q_HEAP_EXIT_CRIT;
#else 
	return;
#endif
}

//��ȡ���пռ�
u32 QHeapGetIdleSize(void)
{
	QHEAP_ITEM *pItem=NULL;
	u32 TotalBytes=0;
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;

	Q_HEAP_ENTER_CRIT;
	pItem=gpItemHdr;
	while(pItem)
	{
		if(pItem->MemIsIdle) TotalBytes+=pItem->Bytes;
		
		pItem=pItem->pNext;
	}	
	Q_HEAP_EXIT_CRIT;
	
	return TotalBytes;
}

//���õ�ַ�Ƿ����ڶѿռ�
bool IsHeapRam(void *pMem)
{
	u16 i;

	for(i=0;i<QH_HEAP_MEM_NUM;i++)
	{
		if(((u32)pMem>=(u32)pQHeap[i]) && ((u32)pMem<(u32)pQHeapEnd[i])) return TRUE;
	}
	
	return FALSE;
}






#else //for #if USE_Q_HEAP 
void QHeapMemSetName(void *pMem,const char *pName)
{
	return;
}

//��ʵ��
extern char _heap_start; //_heap_start - 0x40000000
extern char _lit4_end; //_lit4_end - 0x4010C000 iram
bool IsHeapRam(void *pMem)
{
	if(((u32)pMem>=(u32)&_heap_start) && ((u32)pMem<(u32)0x40000000)) return TRUE;
	if(((u32)pMem>=(u32)&_lit4_end) && ((u32)pMem<(u32)0x4010C000)) return TRUE;
	
	
	return FALSE;
}






#endif
