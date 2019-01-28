#include "SysDefines.h"
#include "Os_Wrap.h"
#include "Q_Heap.h"

//����:����һ���򵥵Ķѹ������
//����Ҫ˼���ǽ��ڴ����Ϊ���ɸ���С��Ԫ�������ڴ�ʱ���������������С��Ԫ���ڴ�
//��Ϊ���ڴ����Ϊ��С��Ԫ�ˣ�����������¼�ڴ�������ı���ֻ��Ҫ16λ����
//�����С��Ԫ��4byte����ô�˴�����Թ���ĶѴ�СΪ0xffff��4��
//��ʵ���ϣ�16bit���棬���1λ������ʾ�ڴ���Ƿ�ʹ�ã�����ʵ�ʴ�С��0x7fff��4����
//��ԼΪ130k�Ŀռ䡣�����Ҫ�������ռ䣬ֻ��Ҫ�����С��Ԫ��Byte��Ŀ���ɡ�
//�˶ѹ�����ƵĶ��⿪����2����
// 1.��Ҫһ����¼��������¼�ڴ����Ϣ(��������HEAP_RECORD)��ÿһ���ڴ��Ӧһ����¼���Ա
//   �����¼��Ĵ�СMAX_RECORD_NUM���Ը���ʵ������޸�
// 2.������Ŀռ䲻����С��Ԫ��������+2ʱ�����ж�����ڴ����ģ����2���ֽ����ڴ�ŷ����ǡ�
//   ���統��С��Ԫ��4byte����������СΪ7����ʵ�ʷ�����ڴ���12��Ϊ4����������
//�ڶѷ���ʱ��Ϊ��ʡ����ʱ�䣬�˻��Ʋ���������ռ�����
//�öѹ������û�н��������:�ڴ���Ƭ
//Ĭ�Ϸ���ĵ�ַ8�ֽڶ���

//�˶ѹ�������ڲ���ϵͳ��Ҳ���������㱼������
//�������㱼�������޸����¼���
// 1.�޸ĶѴ�С���������ʵ���������ֻ��Ҫ�޸�Q_HEAP_SIZE_BYTE����������
// 2.�޸ļ�¼���С��ÿ����һ���ڴ棬��ķ�һ����¼�飬�ɸ���ʵ������޸�MAX_RECORD_NUM�꼴��
// 3.�����������ٽ����ĺ궨��ɿյľ�����:Q_HEAP_CRIT_SAVE_DATA_DEFINE��Q_HEAP_ENTER_CRIT��Q_HEAP_EXIT_CRIT
// 4.�����Ҫ����Ŀռ�ܴ���ô����Ҫ�޸���С��Ԫ����Ӧ����غ�Q_HEAP_MINI_UNIT��Q_HEAP_UNIT_OFFSET

#define Q_HEAP_TRACK_DEBUG Q_HEAP_DEBUG

#ifndef NULL
#define NULL ((void *)0)
#endif

#if Q_HEAP_TRACK_DEBUG > 1
#define Need_Debug 1
#else
#define Need_Debug 0
#endif

#if Need_Debug
#define QH_Debug Debug
#else
#define QH_Debug(x,y...)
#endif

//Ϊ��֤���߳��¶ѷ����ԭ�Ӳ����������������ϵͳ�ٽ���
//����˻��Ʋ������ڶ��̣߳�����Զ���������������Ϊ�հ�
#define Q_HEAP_CRIT_SAVE_DATA_DEFINE 	//OS_CPU_SR cpu_sr
#define Q_HEAP_ENTER_CRIT OS_EnterCritical()
#define Q_HEAP_EXIT_CRIT OS_ExitCritical()

//������֤�ڴ��Ƿ񱻳�ٵı�ʶ
//ռ��2���ֽڣ������ڴ�ǰ�󱻳�ٶ����ܲ���ĸ���Ϊ65535��֮1
#define Q_HEAP_UNIT_REWRITE_ID 0x5808 //������ÿ���������ڴ��ĩβ�ĳ�ٱ�ʶ
#define Q_HEAP_REWRITE_ID ((Q_HEAP_UNIT_REWRITE_ID<<16)|0x1234) //heap�����ڴ�����ǰ��ĳ�ٱ�ʶ����Q_HEAP_UNIT_REWRITE_ID�ŵ���16λ��Ϊ�˷�ֹ��һ�����鱻�ͷ���ɵļ������

//�������������ͬʱ�޸�
#define Q_HEAP_MINI_UNIT 8 //�ڴ浥Ԫ��С����λByte�����ǵ�32λ��������4�ֽڶ��룬��ֵ����Ϊ4����
#define Q_HEAP_UNIT_OFFSET 3 // 2��Q_HEAP_UNIT_OFFSET�η��������Q_HEAP_MINI_UNIT ,������λ���������˷�
#define Q_HEAP_UNIT_MASK (Q_HEAP_MINI_UNIT-1)//���ں��������������

//�ڴ������
#define QH_GetUnitIdx(x) (gQpRecords[x].UnitIdx)//xΪ��¼����ţ������ڴ��ĵ�һ����Ԫ����
#define QH_SetUnitIdx(x,Index) do{gQpRecords[x].UnitIdx=(Index);}while(0);
#define QH_SetUnitNum(x,n) do{gQpRecords[x].UnitNum=(n);}//��¼ռ���ڴ���Ŀ
#define QH_GetUnitNum(x) (gQpRecords[x].UnitNum)//xΪ��¼����ţ������ڴ��ռ�õ�Ԫ��
//#define UnitIdxEnd Q_HEAP_UNIT_MAX_NUM

#define QH_GetMemIdle(x) (gQpRecords[x].MemIdle)
#define QH_SetMemIdle(x,v) do{gQpRecords[x].MemIdle=(v);}while(0);
#define QH_GetUsed(x) (gQpRecords[x].Used)
#define QH_SetUsed(x,v) do{gQpRecords[x].Used=(v);}while(0);

//�����������ݼ�¼����Ż�ȡ��һ�����Ӧ��¼������
#define QH_GetNext(x) (gQpRecords[x].Next)
#define QH_SetNext(x,v) do{gQpRecords[x].Next=(v);}while(0);
#define QH_GetPrev(x) (gQpRecords[x].Prev)
#define QH_SetPrev(x,v) do{gQpRecords[x].Prev=(v);}while(0);

#define QH_CleanRecord(x) do{MemSet(&gQpRecords[x],0,sizeof(QH_RECORD));}while(0);

//For Track
#if Q_HEAP_TRACK_DEBUG
#define QH_SetCallerName(x,p) gQpRecords[x].pFuncName=(p)
#define QH_SetCallerLine(x,s) gQpRecords[x].Line=(s)
#define QH_GetCallerName(x) gQpRecords[x].pFuncName
#define QH_GetCallerLine(x) gQpRecords[x].Line
#else
#define QH_SetCallerName(x,p)
#define QH_SetCallerLine(x,s)
#define QH_GetCallerName(x)
#define QH_GetCallerLine(x)
#endif

//��Ԫ���ֽڵ�ת��
#define QH_Unit2Byte(U) ((U)<<Q_HEAP_UNIT_OFFSET) //ת����Ԫ�����ֽ���,��λ�ƴ���˷�
#define QH_Byte2Unit(B) ((B)?((((B)-1)>>Q_HEAP_UNIT_OFFSET)+1):0) //ת���ֽ�����ռ�õ�Ԫ������λ�ƴ������

typedef struct {
	u16 Used:1;//����¼����
	u16 MemIdle:1;//ָ����ڴ��Ϊ���п�
	u16 Prev:14;//��¼��һ���ڴ��ļ�¼�����
	u16 Next;//��¼��һ���ڴ��ļ�¼�����
	u16 UnitIdx;//��¼�ڴ����ʼ��Ԫλ��
	u16 UnitNum;//�����ڴ�ռ�ö��ٵ�Ԫ
#if Q_HEAP_TRACK_DEBUG
	const char *pFuncName;
	u16 Line;
#endif
}QH_RECORD;
#define QH_RECORD_MAX_NUM 300
#define QH_RECORD_NULL 0
#define QH_RECORD_START 1
#define QH_RECORD_END (QH_RECORD_MAX_NUM-1)
static QH_RECORD gQpRecords[QH_RECORD_MAX_NUM];//�����¼��

//����ѵ�ַ
#define QH_HEAP_MEM_NUM 2
static u32 *pQHeap[QH_HEAP_MEM_NUM];//������8�ֽڶ���
static u32 *pQHeapEnd[QH_HEAP_MEM_NUM];

//���ڵ���
void DebugHeap(void)
{
	u16 Index;//��ʾ�����ʼ��Ԫ
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;
	
	Debug("\n\r--------------Heap Record--0x%08x----R:0x%08x-0x%08x--------\n\r",(u32)pQHeap->Heap,(u32)gQpRecords,(u32)gQpRecords+QH_RECORD_MAX_NUM*sizeof(QH_RECORD));
	Q_HEAP_ENTER_CRIT;
	for(Index=QH_RECORD_START;;Index=QH_GetNext(Index))
	{
		if(QH_GetMemIdle(Index))
		{
			DebugCol(" %3d ",Index);
		}
		else
		{
			Debug("[%3d]",Index);
		}
		
		Debug(" Unit %4u-%4u, Addr %04x - %04x,%5uB, %3u-> Me->%3u",
			QH_GetUnitIdx(Index),QH_GetUnitIdx(QH_GetNext(Index)),
			LBit16((u32)pQHeap->Heap+QH_Unit2Byte(QH_GetUnitIdx(Index))),LBit16((u32)pQHeap->Heap+QH_Unit2Byte(QH_GetUnitIdx(QH_GetNext(Index)))),
			QH_Unit2Byte(QH_GetUnitNum(Index)),
			QH_GetPrev(Index),QH_GetNext(Index));
#if Q_HEAP_TRACK_DEBUG
		if(QH_GetMemIdle(Index)) Debug(", %s:%u",QH_GetCallerName(Index),QH_GetCallerLine(Index));
#endif
		Debug("\n\r");

		if(QH_GetNext(Index)==QH_RECORD_NULL) break;
	}
	Q_HEAP_EXIT_CRIT;
	Debug("--------------Heap Record End--0x%08x---------\n\r",(u32)pQHeap->Heap+Q_HEAP_SIZE_BYTE);
}

//���÷Ŷ�ջ��ٱ�־
void QS_SetAntRwFlag(void)
{

}

//����ջͷβ�ķ���ٱ�־
//����true��ʾ������δ���
bool QS_CheckRwFlag(void)
{

	return TRUE;
}

//�ѳ�ʼ��
void QS_HeapInit(void)
{
	u16 Index;
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;

	if( (QH_Unit2Byte(1)!=Q_HEAP_MINI_UNIT)||(QH_Byte2Unit(Q_HEAP_MINI_UNIT)!=1) )
	{
		Debug("Q_HEAP_MINI_UNIT & Q_HEAP_UNIT_OFFSET Marco Define Error!!!\n\r");
		while(1);
	}
		
	Q_HEAP_ENTER_CRIT;
	for(Index=0;Index<QH_RECORD_MAX_NUM;Index++)
	{
		QH_CleanRecord(Index);
	}

	//����ѵ�ַ
	{
		extern char _heap_start;//_heap_start - 0x40000000
		extern char _lit4_end;//_lit4_end - 0x4010C000 (iram)

		pQHeap[0]=AlignTo8(&_heap_start);
		*pQHeap[0]=Q_HEAP_REWRITE_ID;
		pQHeap[0]+=8;
		pQHeapEnd[0]=0x40000000;
		
		pQHeap[1]=AlignTo8(&_lit4_end);
		*pQHeap[1]=Q_HEAP_REWRITE_ID;
		pQHeap[1]+=8;
		pQHeapEnd[1]=0x4010C000;
	}

	//������¼
	for(Index=QH_RECORD_START;Index<(QH_RECORD_START+QH_HEAP_MEM_NUM);Index++)
	{
		QH_SetPrev(Index,QH_RECORD_NULL);
		if(Index==(QH_RECORD_START+QH_HEAP_MEM_NUM-1)) QH_SetNext(Index,QH_RECORD_NULL);
		else QH_SetNext(Index,Index+1);
		
		QH_SetUnitIdx(Index,QH_Byte2Unit(pQHeap[Index]-pQHeap[0]));
		QH_SetUnitNum(Index,QH_Byte2Unit(pQHeapEnd[Index]-pQHeap[Index]));
		
		QH_SetUsed(Index,TRUE);
		QH_SetMemIdle(Index,TRUE);
		
#if Q_HEAP_TRACK_DEBUG
		QH_SetCallerName(Index,"<Idle>");
		QH_SetCallerLine(Index,0);
#endif
	}

	//�������ڴ����ͷ���
	QS_SetAntRwFlag();
	
	//DebugHeap();
	Q_HEAP_EXIT_CRIT;
}

//����һ���洢���¼����¼����
//Index:�����뵽�˼�¼,
//Index�����Ǹ���¼���ڴ�ļ�¼��
static bool InsertRecord(u16 Index,u16 NeedUnit)
{	
	u16 n;

	//QH_Debug("  ##Insert UnitNum %d @ Idx %d\n\r",NeedUnit,Index);

	if(NeedUnit<QH_GetUnitNum(Index))//��Ҫ�Ŀռ�С�ڿ��пռ�
	{
		//���½���һ���ڴ��¼��
		for(n=QH_RECORD_START;n<QH_RECORD_END;n++)
		{
			if(QH_GetUsed(n)==FALSE)//�ҵ��յļ�¼��
			{
				QH_SetUsed(n,TRUE);
				QH_SetMemIdle(n,TRUE);			
				
				//�������ڴ�飬����һ�����п�
				QH_SetPrev(n,Index);//prev����
				QH_SetNext(n,QH_GetNext(Index));//nextת��
				QH_SetUnitIdx(n,QH_GetUnitIdx(Index)+NeedUnit);
				QH_SetUnitNum(n,QH_GetUnitNum(Index)-NeedUnit);

				//����ԭ�����е��ڴ��֮����ڴ��
				QH_SetPrev(QH_GetNext(Index),n);
				
				//����Ҫʹ�õ��ڴ��
				QH_SetUnitNum(Index,NeedUnit);
				QH_SetMemIdle(Index,FALSE);//���ʹ�ñ�־
				QH_SetNext(Index,n);//nextת��
				
#if Need_Debug
				//DebugHeap();
#endif
				return TRUE;
			}
		}
		
		if(n==QH_RECORD_END)
		{
			Debug("!!!Record Num is not enough!!!\n\r");
		}
		return FALSE;
	}
	else //�����Ҫ������ڴ��Ϳ����ڴ��պ����
	{
		QH_SetMemIdle(Index,FALSE);//��Ǳ�ʹ��
		return TRUE;
	}
}

//UnitIdxΪ��ʼ��Ԫ��������HEAP_MINI_UNITΪ��λ
//����0��ʾ���󣬷��򷵻�free���ڴ���С
#if Q_HEAP_TRACK_DEBUG
static u16 DeleteRecord(u16 UnitIdx,u8 *pFuncName,u32 Lines)
#else
static u16 DeleteRecord(u16 UnitIdx)
#endif
{
	u16 Index,Size=0;
	u16 i;

	//Debug("DeleteRecord Unit 0x%x\n\r",UnitIdx);

	for(Index=QH_RECORD_START;;Index=QH_GetNext(Index))
	{
		if(QH_GetUsed()==TRUE && QH_GetUnitIdx(Index)==UnitIdx)//�ҵ�ƥ������
		{
#if Q_HEAP_TRACK_DEBUG > 1
			Debug("##Free   Func:%s Line:%d (For %s %d)\n\r",pFuncName,Lines,QH_GetCallerName(Index),QH_GetCallerLine(Index));
#endif

#if Q_HEAP_TRACK_DEBUG
			QH_SetCallerName(Index,"<Idle>");
			QH_SetCallerLine(Index,0);
#endif

			if(QH_GetMemIdle(Index)==TRUE) break;//δʹ�õĿ飬���ܱ�free
			
			Size=QH_Unit2Byte(QH_GetUnitNum(Index));//��ȡ�ڴ���С
			
			QH_SetMemIdle(Index,TRUE);//���õ�ǰΪδʹ��
			
			//�ȿ�ǰ��һ���ǲ��ǿյ�
			i=QH_GetPrev(Index);
			if(QH_GetMemIdle(i)==TRUE)//ǰ��һ���ǿյģ����ҵ�ַ�������ģ��ϲ�ǰ��һ��
			{
				if(QH_GetUnitIdx(i)+QH_GetUnitNum(i) == QH_GetUnitIdx(Index))
				{
					QH_SetNext(i,QH_GetNext(Index));
					QH_SetPrev(QH_GetNext(Index),i);
					QH_CleanRecord(Index);		
					Index=i;
				}
			}

			//�ٿ�����һ���ǲ��ǿյ�
			i=QH_GetNext(Index);
			if(QH_GetMemIdle(i)==TRUE)//����һ���ǿյģ����ҵ�ַ�������ģ��ͺ���ĺϲ�
			{
				if(QH_GetUnitIdx(Index)+QH_GetUnitNum(Index) == QH_GetUnitIdx(i))
				{
					QH_SetNext(Index,QH_GetNext(i));
					QH_SetPrev(QH_GetNext(i),Index);
					QH_CleanRecord(i);
				}
			}

#if Need_Debug
			//DebugHeap();
#endif
			return Size;
		}
		
		if(QH_GetNext(Index)==QH_RECORD_NULL) break;//�������һ�����ˣ��˳���
	}

	return 0;	
}

#if Q_HEAP_TRACK_DEBUG
void *_Q_Malloc(u16 Size,u8 *pFuncName,u32 Lines)
#else
void *_Q_Malloc(u16 Size)
#endif
{
	u16 UnitNum,Index,AttachBytes;
	u16 *pMem=NULL;
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;

	if(QS_CheckRwFlag()==FALSE)
	{
		Debug("Heap Be Rewrite!\n\r");
		while(1);
	}
	
	if(Size==0) return NULL;

	UnitNum=QH_Byte2Unit(Size);//����Ҫ������Ԫ
	
	//���һ����Ԫ��������֤�ڴ�ͷβ�Ƿ񱻳�١�
	//���Size�����ͱ���С��Ԫ��������С2����С3���Ͳ��ö����һ����Ԫ�ˡ�
	AttachBytes=Size&Q_HEAP_UNIT_MASK;
	if(AttachBytes==0||(Q_HEAP_MINI_UNIT-AttachBytes)<2) UnitNum++;

#if Q_HEAP_MINI_UNIT<8 //�����ڴ�8�ֽڶ���
	//���ڵ�һ���ڴ��Ѿ�������Ϊ8�ֽڶ����ַ������Ϊ��8�ֽڶ��룬ÿ��ֳ�ȥ���ڴ��С��������8�ı���
	if((UnitNum<<Q_HEAP_UNIT_OFFSET)&0x07)//ɸѡ��δ8�ֽڶ����
	{
		UnitNum++;//�˴��и�ǰ�ᣬ���Ǳ����ڴ浥Ԫ�Ĵ�С��4
	}
#endif

	Q_HEAP_ENTER_CRIT;
	for(Index=QH_RECORD_START;;Index=QH_GetNext(Index))//��ѯ��¼��
	{
		if(QH_GetUsed(Index)==TRUE && QH_GetMemIdle(Index)==TRUE)//�ҵ�һ�����е��ڴ��
		{
			if(QH_GetUnitNum(Index)>=UnitNum)
			{
				if(InsertRecord(Index,UnitNum)==TRUE)
				{
					pMem=(void *)((u32)pQHeap[0]+QH_Unit2Byte(QH_GetUnitIdx(Index)));
					pMem[(QH_Unit2Byte(UnitNum)>>1)-1]=Q_HEAP_UNIT_REWRITE_ID;//�ӵ�β���ķ���ٱ�־
					QH_Debug("##Malloc Size %d, Addr 0x%x,Unit 0x%x\n\r",QH_Unit2Byte(UnitNum),(u32)pMem,QH_GetUnitIdx(Index));
#if Q_HEAP_TRACK_DEBUG
					QH_SetCallerName(Index,pFuncName);
					QH_SetCallerLine(Index,Lines);
#endif

#if Q_HEAP_TRACK_DEBUG > 1				
					Debug("##Malloc Func:%s Line:%d\n\n\r",pFuncName,Lines);
#endif
					Q_HEAP_EXIT_CRIT;	
					//Debug("  M[%x]%d\n\r\n\r",pMem,Size);

					MemSet(pMem,0,Size);	//��ʼ������				
					return (void *)pMem;
				}
				else 
					break;
			}
		}			
		
		if(QH_GetNext(Index)==QH_RECORD_NULL) 
			break;
	}
	Q_HEAP_EXIT_CRIT;
	
	DebugHeap();
	QS_MonitorFragment();
	Debug("!!!No Get Heap,Size:%d!!!\n\r",Size);
	return 0;
//	OS_SchedLock();
//	while(1);
}

#if Q_HEAP_TRACK_DEBUG
bool _Q_Free(void *Ptr,u8 *pFuncName,u32 Lines)
#else
bool _Q_Free(void *Ptr)
#endif
{	
	u16 *pMem=Ptr;
	u16 Size;
	Q_HEAP_CRIT_SAVE_DATA_DEFINE;

	if(Ptr==NULL) return TRUE;
	
	if(QS_CheckRwFlag()==FALSE)
	{
		Debug("Heap Be Rewrite!\n\r");
		while(1);
	}
	//Debug("F[%x]\n\r",(u32)Ptr);
	
	if(Ptr)
	{
		Q_HEAP_ENTER_CRIT;

#if Q_HEAP_TRACK_DEBUG
		Size=DeleteRecord(QH_Byte2Unit((u32)Ptr-(u32)pQHeap[0]),pFuncName,Lines);
#else
		Size=DeleteRecord(QH_Byte2Unit((u32)Ptr-(u32)pQHeap[0]));
#endif

		//Debug("  F[%x]%d\n\r",Ptr,Size);
		
		if(Size==0)
		{
			Debug("##Free Error!!!Can not find the match memory!!!0x%x\n\r",(u32)Ptr);
#if Q_HEAP_TRACK_DEBUG > 1
			Debug("##Free   Func:%s Line:%d\n\r",pFuncName,Lines);
#endif
			//while(1);
			Q_HEAP_EXIT_CRIT;
			return FALSE;
		}
		else 
		{
			QH_Debug("##Free   Size %d, Addr 0x%x,Unit 0x%x\n\n\r",Size,(u32)Ptr,QH_Byte2Unit((u32)Ptr-(u32)pQHeap[0]));
		}
		
		//����ڴ������
		if(pMem[(Size>>1)-1]!=Q_HEAP_UNIT_REWRITE_ID)
		{
			Debug("Memory[%x] end be rewrited!!!Flag %x, Size %d\n\r",Ptr,pMem[(Size>>1)-1],Size);
#if Q_HEAP_TRACK_DEBUG > 1
			Debug("##Free   Func:%s Line:%d\n\r",pFuncName,Lines);
#endif
			DebugHeap();
			while(1);
			Q_HEAP_EXIT_CRIT;
			return FALSE;
		}

		pMem--;
		if(pMem[0]!=Q_HEAP_UNIT_REWRITE_ID)
		{
			Debug("Memory header be rewrited!!![%x]=%x\n\r",(u32)pMem,pMem[0]);
#if Q_HEAP_TRACK_DEBUG > 1
			Debug("##FuncName:%s Lines:%d\n\r",pFuncName,Lines);
#endif
			DebugHeap();
			while(1);
			Q_HEAP_EXIT_CRIT;
			return FALSE;
		}
				
		Q_HEAP_EXIT_CRIT;
		return TRUE;
	}
	else
		return FALSE;
}

#if 0
#if Q_HEAP_TRACK_DEBUG
void *_Q_Calloc(u16 Size,u8 *pFuncName,u32 Lines)
#else
void *_Q_Calloc(u16 Size)
#endif
{
	void *p;
#if Q_HEAP_TRACK_DEBUG
	p = _Q_Malloc(Size,pFuncName,Lines);
#else
	p = _Q_Malloc(Size);
#endif
	if (p) 
	{
		MemSet(p, 0, Size);
	}
	return p;
}
#endif

//�����ڴ���Ƭ���
/*Ϊ�������ڴ���Ƭ�ж��٣�������һ��������fragindices����������
fragindices������Ϊ��
fragindices = (float)MaxSize/(float)TotalSize
����MaxSize��ʾ�������ɷ�������������ڴ��С
TotalSize��ʾ�������п����ڴ���ܺ�
�ɼ���ϵͳ����Ƭʱfragindices = 1
����ϵͳ���ڴ�ɷ���ʱfragindices = 0 
fragindicesԽС��ʾ��ƬԽ��*/
void QS_MonitorFragment(void)
{
	u16 Index,Size;
	u32 MaxSize=0,TotalSize=0;
	
	for(Index=QH_RECORD_START;;Index=QH_GetNext(Index))
	{ 
		if(QH_GetMemIdle(Index)==TRUE)
		{
			Size=QH_Unit2Byte(QH_GetUnitNum(Index));//��ȡ�ڴ���С
			TotalSize+=Size;
			if(Size>MaxSize)
				MaxSize=Size;
		}
		if(QH_GetNext(Index)==QH_RECORD_NULL) break;
	}

	Debug("********** Heap Monitor **********\n\r");
	Debug(" TotalFreeMem=%d Byte\n\r",TotalSize);
	Debug(" MaxFreeMem=%d Byte\n\r",MaxSize);
	Debug(" Fragindices=%u.%02uf\n\r",MaxSize/TotalSize,MaxSize*100/TotalSize%100);
	Debug("**********************************\n\r");
}

//���������ڴ���Ƭ
bool QS_HeapArrange(void)
{//unfinish
	return TRUE;
}

//���õ�ַ�Ƿ����ڶѿռ�
bool IsHeapRam(void *ptr)
{
	u16 i;

	for(i=0;i<QH_HEAP_MEM_NUM;i++)
	{
		if(((u32)ptr>=(u32)&pQHeap[i]) && ((u32)ptr<&pQHeapEnd[i])) return TRUE;
	}
	
	return FALSE;
}


