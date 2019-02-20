#ifndef QSYS_Q_HEAP_H
#define QSYS_Q_HEAP_H

#define USE_Q_HEAP 0 //为了与系统heap5并存

#if USE_Q_HEAP //4 支持与系统heap5并存
#define Q_HEAP_DEBUG 2//置1打开打印，置2会打开记录Malloc申请者的功能，但会增加额外内存开销

void QHeapDebug(void);
void QHeapMemSetName(void *pMem,const char *pName);
u32 QHeapGetIdleSize(void);
bool IsHeapRam(void *p);

#if Q_HEAP_DEBUG > 1
void *_Q_Malloc(u16 Size,const char *pFuncCaller,u32 Line);
void *_Q_MallocAsyn(u16 Size,const char *pFuncCaller,u32 Line);
void _Q_Free(void *Ptr,const char *pFuncCaller,u32 Line);
#else
void *_Q_Malloc(u16 Size);
void *_Q_MallocAsyn(u16 Size);
void _Q_Free(void *Ptr);
#endif

#if Q_HEAP_DEBUG > 1
#define Q_Malloc(n) _Q_Malloc(n,__func__,__LINE__)
#define Q_MallocAsyn(n) _Q_MallocAsyn(n,__func__,__LINE__)
#define Q_Zalloc(n) _Q_Malloc(n,__func__,__LINE__)
#define Q_ZallocAsyn(n) _Q_MallocAsyn(n,__func__,__LINE__)
#define Q_Calloc(n,s) _Q_Malloc((n)*(s),__func__,__LINE__)
#define Q_Free(p) _Q_Free(p,__func__,__LINE__)
#else
#define Q_Malloc _Q_Malloc
#define Q_MallocAsyn _Q_MallocAsyn
#define Q_Zalloc _Q_Malloc
#define Q_ZallocAsyn _Q_MallocAsyn
#define Q_Calloc(n,s) _Q_Malloc((n)*(s))
#define Q_Free _Q_Free
#endif





#else //4 for #if USE_Q_HEAP 
void QHeapMemSetName(void *pMem,const char *pName);

#define QHeapDebug pvShowMalloc
#define QHeapGetIdleSize system_get_free_heap_size
#define Q_Malloc os_malloc
#define Q_MallocAsyn os_malloc
#define Q_Zalloc os_zalloc
#define Q_ZallocAsyn os_zalloc
#define Q_Free os_free






#endif//4 end #if USE_Q_HEAP 



#endif

