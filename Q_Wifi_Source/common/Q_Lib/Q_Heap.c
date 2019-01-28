#include "SysDefines.h"
#include "Os_Wrap.h"
#include "Q_Heap.h"

//概述:这是一个简单的堆管理机制
//其主要思想是将内存分配为若干个最小单元，申请内存时，会分配整数个最小单元的内存
//因为将内存分配为最小单元了，所以用来记录内存块索引的变量只需要16位即可
//如果最小单元是4byte，那么此代码可以管理的堆大小为0xffff的4倍
//但实际上，16bit里面，最高1位用来表示内存块是否被使用，所以实际大小是0x7fff的4倍，
//大约为130k的空间。如果需要管理更大空间，只需要提高最小单元的Byte数目即可。
//此堆管理机制的额外开销有2部分
// 1.需要一个记录表用来记录内存块信息(定义类型HEAP_RECORD)，每一块内存对应一个记录表成员
//   这个记录表的大小MAX_RECORD_NUM可以根据实际情况修改
// 2.当申请的空间不是最小单元的整数倍+2时，会有多余的内存消耗，最后2个字节用于存放防冲标记。
//   例如当最小单元是4byte，如果申请大小为7，则实际分配的内存是12，为4的整数倍。
//在堆分配时，为节省分配时间，此机制不负责将申请空间清零
//该堆管理机制没有解决的问题:内存碎片
//默认分配的地址8字节对齐

//此堆管理可用于操作系统，也可以用于裸奔程序中
//如用于裸奔程序，需修改如下几步
// 1.修改堆大小，这个根据实际情况来，只需要修改Q_HEAP_SIZE_BYTE这个宏就行了
// 2.修改记录块大小，每分配一次内存，会耗费一个记录块，可根据实际情况修改MAX_RECORD_NUM宏即可
// 3.将三个关于临界区的宏定义成空的就行了:Q_HEAP_CRIT_SAVE_DATA_DEFINE、Q_HEAP_ENTER_CRIT、Q_HEAP_EXIT_CRIT
// 4.如果需要管理的空间很大，那么你需要修改最小单元来适应，相关宏Q_HEAP_MINI_UNIT、Q_HEAP_UNIT_OFFSET

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

//为保证多线程下堆分配的原子操作，必须借助操作系统临界区
//如果此机制不被用于多线程，则可以定义下面三个个宏为空白
#define Q_HEAP_CRIT_SAVE_DATA_DEFINE 	//OS_CPU_SR cpu_sr
#define Q_HEAP_ENTER_CRIT OS_EnterCritical()
#define Q_HEAP_EXIT_CRIT OS_ExitCritical()

//用来验证内存是否被冲毁的标识
//占用2个字节，所以内存前后被冲毁而不能察觉的概率为65535分之1
#define Q_HEAP_UNIT_REWRITE_ID 0x5808 //放置于每个被分配内存块末尾的冲毁标识
#define Q_HEAP_REWRITE_ID ((Q_HEAP_UNIT_REWRITE_ID<<16)|0x1234) //heap整个内存区域前后的冲毁标识，将Q_HEAP_UNIT_REWRITE_ID放到高16位是为了防止第一块分配块被释放造成的检查问题

//下面两个宏必须同时修改
#define Q_HEAP_MINI_UNIT 8 //内存单元大小，单位Byte，考虑到32位处理器的4字节对齐，此值必须为4倍数
#define Q_HEAP_UNIT_OFFSET 3 // 2的Q_HEAP_UNIT_OFFSET次方必须等于Q_HEAP_MINI_UNIT ,用于移位代替除法或乘法
#define Q_HEAP_UNIT_MASK (Q_HEAP_MINI_UNIT-1)//用于和运算代替余运算

//内存块索引
#define QH_GetUnitIdx(x) (gQpRecords[x].UnitIdx)//x为记录表序号，返回内存块的第一个单元索引
#define QH_SetUnitIdx(x,Index) do{gQpRecords[x].UnitIdx=(Index);}while(0);
#define QH_SetUnitNum(x,n) do{gQpRecords[x].UnitNum=(n);}//记录占用内存数目
#define QH_GetUnitNum(x) (gQpRecords[x].UnitNum)//x为记录表序号，返回内存块占用单元数
//#define UnitIdxEnd Q_HEAP_UNIT_MAX_NUM

#define QH_GetMemIdle(x) (gQpRecords[x].MemIdle)
#define QH_SetMemIdle(x,v) do{gQpRecords[x].MemIdle=(v);}while(0);
#define QH_GetUsed(x) (gQpRecords[x].Used)
#define QH_SetUsed(x,v) do{gQpRecords[x].Used=(v);}while(0);

//操作链表，根据记录表序号获取下一个块对应记录表的序号
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

//单元和字节的转换
#define QH_Unit2Byte(U) ((U)<<Q_HEAP_UNIT_OFFSET) //转换单元数到字节数,用位移代替乘法
#define QH_Byte2Unit(B) ((B)?((((B)-1)>>Q_HEAP_UNIT_OFFSET)+1):0) //转换字节数到占用单元数，用位移代替除法

typedef struct {
	u16 Used:1;//本记录有用
	u16 MemIdle:1;//指向的内存块为空闲块
	u16 Prev:14;//记录上一个内存块的记录表序号
	u16 Next;//记录下一个内存块的记录表序号
	u16 UnitIdx;//记录内存块起始单元位置
	u16 UnitNum;//本块内存占用多少单元
#if Q_HEAP_TRACK_DEBUG
	const char *pFuncName;
	u16 Line;
#endif
}QH_RECORD;
#define QH_RECORD_MAX_NUM 300
#define QH_RECORD_NULL 0
#define QH_RECORD_START 1
#define QH_RECORD_END (QH_RECORD_MAX_NUM-1)
static QH_RECORD gQpRecords[QH_RECORD_MAX_NUM];//分配记录表

//定义堆地址
#define QH_HEAP_MEM_NUM 2
static u32 *pQHeap[QH_HEAP_MEM_NUM];//起点必须8字节对齐
static u32 *pQHeapEnd[QH_HEAP_MEM_NUM];

//用于调试
void DebugHeap(void)
{
	u16 Index;//表示块的起始单元
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

//设置放堆栈冲毁标志
void QS_SetAntRwFlag(void)
{

}

//检查堆栈头尾的防冲毁标志
//返回true表示正常，未冲毁
bool QS_CheckRwFlag(void)
{

	return TRUE;
}

//堆初始化
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

	//定义堆地址
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

	//建立记录
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

	//建立冲内存检查的头标记
	QS_SetAntRwFlag();
	
	//DebugHeap();
	Q_HEAP_EXIT_CRIT;
}

//插入一个存储块记录到记录表中
//Index:将插入到此记录,
//Index必须是个记录空内存的记录块
static bool InsertRecord(u16 Index,u16 NeedUnit)
{	
	u16 n;

	//QH_Debug("  ##Insert UnitNum %d @ Idx %d\n\r",NeedUnit,Index);

	if(NeedUnit<QH_GetUnitNum(Index))//需要的空间小于空闲空间
	{
		//重新建立一个内存记录块
		for(n=QH_RECORD_START;n<QH_RECORD_END;n++)
		{
			if(QH_GetUsed(n)==FALSE)//找到空的记录体
			{
				QH_SetUsed(n,TRUE);
				QH_SetMemIdle(n,TRUE);			
				
				//设置新内存块，这是一个空闲块
				QH_SetPrev(n,Index);//prev设置
				QH_SetNext(n,QH_GetNext(Index));//next转移
				QH_SetUnitIdx(n,QH_GetUnitIdx(Index)+NeedUnit);
				QH_SetUnitNum(n,QH_GetUnitNum(Index)-NeedUnit);

				//处理原本空闲的内存块之后的内存块
				QH_SetPrev(QH_GetNext(Index),n);
				
				//处理要使用的内存块
				QH_SetUnitNum(Index,NeedUnit);
				QH_SetMemIdle(Index,FALSE);//标记使用标志
				QH_SetNext(Index,n);//next转移
				
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
	else //如果需要插入的内存块和空闲内存块刚好相等
	{
		QH_SetMemIdle(Index,FALSE);//标记被使用
		return TRUE;
	}
}

//UnitIdx为起始单元索引，以HEAP_MINI_UNIT为单位
//返回0表示错误，否则返回free的内存块大小
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
		if(QH_GetUsed()==TRUE && QH_GetUnitIdx(Index)==UnitIdx)//找到匹配项了
		{
#if Q_HEAP_TRACK_DEBUG > 1
			Debug("##Free   Func:%s Line:%d (For %s %d)\n\r",pFuncName,Lines,QH_GetCallerName(Index),QH_GetCallerLine(Index));
#endif

#if Q_HEAP_TRACK_DEBUG
			QH_SetCallerName(Index,"<Idle>");
			QH_SetCallerLine(Index,0);
#endif

			if(QH_GetMemIdle(Index)==TRUE) break;//未使用的块，不能被free
			
			Size=QH_Unit2Byte(QH_GetUnitNum(Index));//获取内存块大小
			
			QH_SetMemIdle(Index,TRUE);//设置当前为未使用
			
			//先看前面一个是不是空的
			i=QH_GetPrev(Index);
			if(QH_GetMemIdle(i)==TRUE)//前面一个是空的，并且地址是连续的，合并前面一个
			{
				if(QH_GetUnitIdx(i)+QH_GetUnitNum(i) == QH_GetUnitIdx(Index))
				{
					QH_SetNext(i,QH_GetNext(Index));
					QH_SetPrev(QH_GetNext(Index),i);
					QH_CleanRecord(Index);		
					Index=i;
				}
			}

			//再看后面一个是不是空的
			i=QH_GetNext(Index);
			if(QH_GetMemIdle(i)==TRUE)//后面一个是空的，并且地址是连续的，和后面的合并
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
		
		if(QH_GetNext(Index)==QH_RECORD_NULL) break;//到了最后一个块了，退出。
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

	UnitNum=QH_Byte2Unit(Size);//计算要几个单元
	
	//多加一个单元，用来验证内存头尾是否被冲毁。
	//如果Size本来就比最小单元的整数倍小2或者小3，就不用多余的一个单元了。
	AttachBytes=Size&Q_HEAP_UNIT_MASK;
	if(AttachBytes==0||(Q_HEAP_MINI_UNIT-AttachBytes)<2) UnitNum++;

#if Q_HEAP_MINI_UNIT<8 //分配内存8字节对齐
	//由于第一块内存已经被设置为8字节对齐地址，所以为了8字节对齐，每块分出去的内存大小都必须是8的倍数
	if((UnitNum<<Q_HEAP_UNIT_OFFSET)&0x07)//筛选出未8字节对齐的
	{
		UnitNum++;//此处有个前提，就是必须内存单元的大小是4
	}
#endif

	Q_HEAP_ENTER_CRIT;
	for(Index=QH_RECORD_START;;Index=QH_GetNext(Index))//轮询记录表
	{
		if(QH_GetUsed(Index)==TRUE && QH_GetMemIdle(Index)==TRUE)//找到一个空闲的内存块
		{
			if(QH_GetUnitNum(Index)>=UnitNum)
			{
				if(InsertRecord(Index,UnitNum)==TRUE)
				{
					pMem=(void *)((u32)pQHeap[0]+QH_Unit2Byte(QH_GetUnitIdx(Index)));
					pMem[(QH_Unit2Byte(UnitNum)>>1)-1]=Q_HEAP_UNIT_REWRITE_ID;//加到尾部的防冲毁标志
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

					MemSet(pMem,0,Size);	//初始化归零				
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
		
		//检查内存冲毁情况
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

//用于内存碎片监控
/*为了描述内存碎片有多少，引入了一个浮点数fragindices来描述它：
fragindices被定义为：
fragindices = (float)MaxSize/(float)TotalSize
其中MaxSize表示堆中最大可分配的连续空闲内存大小
TotalSize表示堆中所有空闲内存的总和
可见当系统无碎片时fragindices = 1
而当系统无内存可分配时fragindices = 0 
fragindices越小表示碎片越多*/
void QS_MonitorFragment(void)
{
	u16 Index,Size;
	u32 MaxSize=0,TotalSize=0;
	
	for(Index=QH_RECORD_START;;Index=QH_GetNext(Index))
	{ 
		if(QH_GetMemIdle(Index)==TRUE)
		{
			Size=QH_Unit2Byte(QH_GetUnitNum(Index));//获取内存块大小
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

//用于整理内存碎片
bool QS_HeapArrange(void)
{//unfinish
	return TRUE;
}

//检查该地址是否属于堆空间
bool IsHeapRam(void *ptr)
{
	u16 i;

	for(i=0;i<QH_HEAP_MEM_NUM;i++)
	{
		if(((u32)ptr>=(u32)&pQHeap[i]) && ((u32)ptr<&pQHeapEnd[i])) return TRUE;
	}
	
	return FALSE;
}


