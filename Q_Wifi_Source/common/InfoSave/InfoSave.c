//--------------------------Q-IOT Platform------------------------------------------------------//
/*
Q-Wifi是酷享物联平台的固件实现，基于ESP8266开发，构建了一套与酷享APP、酷享云平台相互配合的物联网
底层实现机制，代码基于ESP8266_RTOS_SDK开发，开源，欢迎更多程序员大牛加入！

Q-Wifi构架了一套设备、变量、情景、动作、用户终端的管理机制，内建了若干个TCP连接与APP及云平台互动，
其中json上报连接，采用通用的http json格式，可被开发者导向至自己的物联网平台，进行数据汇集和控制。

Q-Wifi内建了一套动态的web网页服务器，用来进行硬件配置，此web网页服务器封装完整，可被开发者移植到
其他项目。

Q-Wifi内建了一套基于串口的指令解析系统，用来进行串口调试，指令下发，此解析系统封装完整，包含一系
列解析函数，可被开发者移植至其他项目。

Q-Wifi内部带user标识的文件，均为支持开发者自主修改的客制化文件，特别是user_hook.c文件，系统内所
有关键点都会有hook函数在此文件中，供开发者二次开发。

Q-Wifi代码拥有众多模块化的机制或方法，可以被复用及移植，减少物联网系统的开发难度。
所有基于酷享物联平台进行的开发或案例、产品，均可联系酷享团队，免费放置于酷物联视频（q-iot.cn）进行
传播或有偿售卖，相应所有扣除税费及维护费用后，均全额提供给贡献者，以此鼓励国内开源事业。

By Karlno 酷享科技

本文件定义了一个类似于数据库的数据存储机制
*/
//---------------------------------------------------------------------------------------------//
#include "SysDefines.h"
#include "SpiFlashApi.h"

//info存储块不应从实际flash的0地址开始，这样是为了避免本页程序中的FlashAddr可能为0

//Addr:指信息存储块的绝对地址
//Site:指信息存储相对位置，从1开始，存储的第1个信息块地址为1，存储的第2个信息块为2
//ID:指信息内部的应用程序指定的id，必须非0
//Idx:指信息存储中有效信息的顺序索引，从1开始，比如第1,3个信息块有效，第2个信息块为废弃块，那么第三个信息块Idx为2，而不是3

enum{
	IBF_Removed=0xe0,//此块无用，信息也已经被删除
	IBF_Vaild=0xf0,//此块信息有效
	IBF_Null=0xfe,//此块空白未使用
};
typedef u8 INFO_BLOCK_FLAG;

typedef struct{
	INFO_BLOCK_FLAG Flag;///用来表示此信息块存在还是被删除
	INFO_TYPE Type;//类型
	u16 Resv;
}INFO_HEADER;

typedef struct{
	INFO_BLOCK_FLAG Flag;///用来表示此信息块存在还是被删除
	INFO_TYPE Type;//类型
	u16 Resv;
	INFO_ID InfoID;//由用户定义，与info结构无关，必须放到用户数据的最前面
}INFO_HEADER_AND_ID;

typedef struct{
	INFO_TYPE Type:4;//0-14可用，15用来表示空
	u8 IdBits:4;//存储id的最后4位，用来快速对比
}INFO_MAP;//仅用一个字节，来存放类型，id快速索引

typedef struct{
	u32 StartSec;//起始扇区，对于FM_INFOSAVE_BASE_SECTOR的偏移值
	u32 SectorNum;//占用扇区数
	u32 UnitSize;//单元字节数，含4个头字节
	u32 UnitTotal;//单元总数
	INFO_MAP *pMap;//映射表
}INFO_BLOCK_ATTRIB;//存储块信息

typedef struct{
	INFO_TYPE Type;
	INFO_BLOCK Block;
	u16 ItemBytes;//必须小于等于INFO_BLOCK_ATTRIB.UnitSize-4
}INFO_TYPE_ATTRIB;

//寻址缓存表，将每个item的类型存放此处，0表示已删除，0xff表示未用
static INFO_MAP gpTypeMapB64[512];//数组大小请对应gBlockAttrib修改
static INFO_MAP gpTypeMapB128[256];//数组大小请对应gBlockAttrib修改
static INFO_MAP gpTypeMapB256[128];//数组大小请对应gBlockAttrib修改
static INFO_MAP gpTypeMapB512[64];//数组大小请对应gBlockAttrib修改

//存储区，每个存储区保有最少1024个可供存储区域
//每个扇区64kB，即256页
static const INFO_BLOCK_ATTRIB gBlockAttrib[IBN_MAX]={ //每个存储块信息
//	StartSec		SectorNum			UnitSize		UnitTotal
{0,		8,		FLASH_PAGE_SIZE/4	,	512,gpTypeMapB64},//IBN_64B
{8,		8,		FLASH_PAGE_SIZE/2	,	256,gpTypeMapB128},//IBN_128B
{16,		8,		FLASH_PAGE_SIZE,		128,gpTypeMapB256},//IBN_256B
{24,		8,		FLASH_PAGE_SIZE*2	,	64,gpTypeMapB512},//IBN_512B
};

#define GetBlockStartSec(b) (FM_INFOSAVE_BASE_SECTOR+gBlockAttrib[b].StartSec)
#define GetBlockStartAddr(b) ((FM_INFOSAVE_BASE_SECTOR+gBlockAttrib[b].StartSec)*FLASH_SECTOR_BYTES)

static const INFO_TYPE_ATTRIB gTypeAttrib[IFT_MAX]={ //每个类型的信息
//Type		Block		ItemBytes
{IFT_STR,				IBN_128B,		sizeof(STR_RECORD)},
{IFT_VARIABLE,		IBN_64B,			sizeof(VARIABLE_RECORD)},
{IFT_RF_DATA,		IBN_256B,		252},//占位，不可删
{IFT_IR_DATA,		IBN_256B,		252},//占位，不可删
{IFT_DEV,				IBN_256B,		sizeof(DEVICE_RECORD)},
{IFT_TRIGGER,		IBN_64B,			sizeof(TRIGGER_RECORD)},
{IFT_SCENE,			IBN_512B,		sizeof(SCENE_RECORD)},
};


#if 1 //索引表相关，必须放到map建立之后使用
static void TypeMapInit(INFO_BLOCK Block)
{
	MemSet(gBlockAttrib[Block].pMap,0xff,gBlockAttrib[Block].UnitTotal*sizeof(INFO_MAP));
}

//根据读出的存储内容，建立map
static void BuildTypeMap(INFO_BLOCK Block)
{
	INFO_MAP *pMap=gBlockAttrib[Block].pMap;
	INFO_HEADER_AND_ID Header;
	u32 Unit;

	TypeMapInit(Block);

	for(Unit=0;Unit<gBlockAttrib[Block].UnitTotal;Unit++)//逐条读取信息头
	{
		SpiFlsReadData(GetBlockStartAddr(Block)+Unit*gBlockAttrib[Block].UnitSize,sizeof(INFO_HEADER_AND_ID),(void *)&Header);

		if(Header.Flag==IBF_Null)
		{
			pMap[Unit].Type=IFT_IDLE;
			pMap[Unit].IdBits=0;
		}		
		else if(Header.Flag==IBF_Vaild)
		{
			pMap[Unit].Type=Header.Type;
			pMap[Unit].IdBits=Header.InfoID;
		}
		else if(Header.Flag==IBF_Removed)
		{
			pMap[Unit].Type=IFT_NOVALID;
			pMap[Unit].IdBits=0;
		}
		else
		{
			Debug("Format Block Is Error!%u\n\r",Block);
			while(1);
		}
	}
}

//修改索引标记
static void SetItemMapFlag(INFO_BLOCK Block,INFO_SITE Site,INFO_TYPE Type,u32 InfoID)
{
	INFO_MAP *pMap=gBlockAttrib[Block].pMap;
	pMap[Site].Type=Type;
	pMap[Site].IdBits=InfoID;
}

//得到真实地址
static INFO_ADDR SiteToAddr(INFO_BLOCK Block,INFO_SITE Site)
{
	return GetBlockStartAddr(Block)+Site*gBlockAttrib[Block].UnitSize;
}

//根据app id找寻项目，要读取flash，所以比较费时间
static INFO_SITE FindItemByID(INFO_BLOCK Block,INFO_TYPE Type,INFO_ID InfoID)
{
	INFO_MAP *pMap=gBlockAttrib[Block].pMap;
	INFO_HEADER_AND_ID Header;
	u16 Site;

	for(Site=0;Site<gBlockAttrib[Block].UnitTotal;Site++)
	{
		if(pMap[Site].Type==Type && (pMap[Site].IdBits == (InfoID&0x0f)))//找到类型了
		{
			u32 Addr=SiteToAddr(Block,Site);
			SpiFlsReadData(Addr,sizeof(Header),(void *)&Header);
			if(Header.InfoID==InfoID)
			{
				return Site;
			}
		}
	}

	return INFO_NOT_FOUND;
}

//根据索引查找位置
static INFO_SITE FindItemByIdx(INFO_BLOCK Block,INFO_TYPE Type,INFO_IDX Idx)
{
	INFO_MAP *pMap=gBlockAttrib[Block].pMap;
	u16 Site;
	u16 Cnt=0;
	
	for(Site=0;Site<gBlockAttrib[Block].UnitTotal;Site++)
	{
		if(pMap[Site].Type==Type)
		{
			if(++Cnt==Idx)
			{
				return Site;
			}
		}
	}

	return INFO_NOT_FOUND;
}

#endif


#if 1 //内部函数
//统计各种类型info个数
static void StatsInfoItemNum(INFO_BLOCK Block,u16 *pIdleNum,u16 *pValidNum,u16 *pRemovedNum)
{
	INFO_HEADER Header;
	u32 Unit;

	for(Unit=0;Unit<gBlockAttrib[Block].UnitTotal;Unit++)//逐条读取信息头
	{
		SpiFlsReadData(GetBlockStartAddr(Block)+Unit*gBlockAttrib[Block].UnitSize,sizeof(Header),(void *)&Header);

		if(Header.Flag==IBF_Null)
		{
			(*pIdleNum)++;
		}		
		else if(Header.Flag==IBF_Vaild)
		{
			(*pValidNum)++;
		}
		else if(Header.Flag==IBF_Removed)
		{
			(*pRemovedNum)++;
		}
		else
		{
			Debug("Format Block Is Error!%u\n\r",Block);
			while(1);
		}
	}

	//Debug("Block[%u] Idle:%4u, Vaild:%4u, Removed:%4u @ Sector %u:%u[%u/%u]\n\r",Block,*pIdleNum,*pValidNum,*pRemovedNum,MainAddr/FLASH_SECTOR_BYTES,gBlockAttrib[Block].SectorNum,(FM_INFOSAVE_BASE_SECTOR+GetStartSecOffset(Block,FALSE)),(FM_INFOSAVE_BASE_SECTOR+GetStartSecOffset(Block,TRUE)));
}

//检查存储块
//如果存储块未被格式化，返回FALSE
//如果存储块已被格式化，返回TRUE
static bool CheckBlockFat(INFO_BLOCK Block)
{
	INFO_HEADER Header;
	u32 Unit;

	for(Unit=0;Unit<gBlockAttrib[Block].UnitTotal;Unit++)//逐条读取信息头
	{
		SpiFlsReadData(GetBlockStartAddr(Block)+Unit*gBlockAttrib[Block].UnitSize,sizeof(Header),(void *)&Header);

		if(Header.Flag!=IBF_Removed && Header.Flag!=IBF_Vaild && Header.Flag!=IBF_Null)
		{
			return FALSE;
		}		
	}

	return TRUE;
}

//检查存储块是否全为0xff
static bool CheckBlockIdle(INFO_BLOCK Block)
{
	u32 j,Addr,EndAddr=(GetBlockStartSec(Block)+gBlockAttrib[Block].SectorNum)*FLASH_SECTOR_BYTES;
	u32 *pBuf=Q_Zalloc(1024);

	for(Addr=GetBlockStartAddr(Block);Addr<EndAddr;Addr+=1024)//逐页读取
	{
		SpiFlsReadData(Addr,1024,(void *)pBuf);

		for(j=0;j<(1024>>2);j++)
		{
			if(pBuf[j]!=0xffffffff)
			{
				Q_Free(pBuf);
				return FALSE;
			}		
		}
	}

	Q_Free(pBuf);
	return TRUE;
}

//擦除并格式化某区
static void FromatBlock(INFO_BLOCK Block)
{
	if(CheckBlockIdle(Block)!=TRUE)//非全空
	{
		u32 Now;
		Debug("FromatBlock[%u] EraseSec:%u-%u,",Block,GetBlockStartSec(Block),gBlockAttrib[Block].SectorNum);
		Now=OS_GetNowMs();
		SpiFlsEraseSector(GetBlockStartSec(Block),gBlockAttrib[Block].SectorNum);//擦除此block全部内容
		Debug("Finish %dmS\n\r",OS_GetNowMs()-Now);
	}

	//开始格式化
	{
		INFO_HEADER InfoHeader={IBF_Null,0xff,0xffff};
		u32 Unit;

		for(Unit=0;Unit<gBlockAttrib[Block].UnitTotal;Unit++)//逐条设置信息头
		{
			SpiFlsWriteData(GetBlockStartAddr(Block)+Unit*gBlockAttrib[Block].UnitSize,sizeof(INFO_HEADER),(void *)&InfoHeader);
		}
	}

	//标记map
	TypeMapInit(Block);
}

//挤掉空隙，重新存储
//利用flash可以只擦除4k的特点
//此过程不能断电
//完成后会建立map
static void FlushBlock(INFO_BLOCK Block)
{
	u32 EndAddr=(GetBlockStartSec(Block)+gBlockAttrib[Block].SectorNum)*FLASH_SECTOR_BYTES;
	u32 UnitSize=gBlockAttrib[Block].UnitSize;
	u32 UintNum=(FLASH_SECTOR_BYTES/gBlockAttrib[Block].UnitSize);
	u32 DstAddr=GetBlockStartAddr(Block);
	u8 *pSecBuf=Q_Zalloc(FLASH_SECTOR_BYTES);
	INFO_MAP *pMap=gBlockAttrib[Block].pMap;
	INFO_HEADER_AND_ID *pBuf;
	u32 MapIdx=0,Unit,Addr;

	TypeMapInit(Block);//清空map
			
	for(Addr=GetBlockStartAddr(Block);Addr<EndAddr;Addr+=FLASH_SECTOR_BYTES)
	{
		SpiFlsReadData(Addr,FLASH_SECTOR_BYTES,(void *)pSecBuf);//每4k为一个单位，全部拷贝到buf中
		SpiFlsEraseSector(Addr/FLASH_SECTOR_BYTES,1);//擦除4k扇区

		for(Unit=0,pBuf=(void *)pSecBuf;Unit<UintNum;Unit++,pBuf=(void *)(UnitSize+(u32)pBuf))//逐个unit检查，拷贝有效unit到扇区中
		{
			if(pBuf->Flag==IBF_Vaild)//仅拷贝有效块
			{
				SpiFlsWriteData(DstAddr,UnitSize,(void *)pBuf);
				DstAddr+=UnitSize;
				pMap[MapIdx].Type=pBuf->Type;//标记map
				pMap[MapIdx].IdBits=pBuf->InfoID;
				MapIdx++;				
			}		
		}		
	}

	//剩余的空间全部格式化
	{
		INFO_HEADER InfoHeader={IBF_Null,0xff,0xffff};

		for(;DstAddr<EndAddr;DstAddr+=UnitSize)//逐条设置信息头
		{
			SpiFlsWriteData(DstAddr,sizeof(INFO_HEADER),(void *)&InfoHeader);
		}
	}	

	Q_Free(pSecBuf);
}

//Force是否强制整理
//如果占用废弃块太多，就重新整理存储区
//如果发生整理，返回true
//完成后，会建立map
static bool TidyBlock(INFO_BLOCK Block)
{
	u16 IdleNum=0,ValidNum=0,RemovedNum=0;
	u16 Total=gBlockAttrib[Block].UnitTotal;

	//统计个数
	StatsInfoItemNum(Block,&IdleNum,&ValidNum,&RemovedNum);

	if((IdleNum+ValidNum+RemovedNum)!=Total)
	{
		Debug("Block Flag Num Is Error!\n\r");
	}

	//检查是否需要整理
	if(IdleNum<(Total>>2) && RemovedNum>(Total>>3))//空白单元数目小于总体四分之一，并且删除单元数大于总体八分之一，就整理
	{
		u32 Now;
		Debug("Need Tidy Block[%u], Idle:%4u, Valid:%4u, Remove:%4u ... ",Block,IdleNum,ValidNum,RemovedNum);
		Now=OS_GetNowMs();
		FlushBlock(Block);
		Debug("Finish by %umS\n\r",OS_GetNowMs()-Now);
		return TRUE;
	}
	else
	{
		Debug("No Need Tidy Block[%u], Idle:%4u, Valid:%4u, Remove:%4u\n\r",Block,IdleNum,ValidNum,RemovedNum);
		BuildTypeMap(Block);//建立索引表，方便后期操作
		return FALSE;
	}
}
#endif

//展示所有信息
void DebugInfoSave(INFO_BLOCK Block)
{
	u16 IdleNum=0,ValidNum=0,RemovedNum=0;
	INFO_MAP *pMap=NULL;
	u16 Unit;

	if(Block>=IBN_MAX) return;
	pMap=gBlockAttrib[Block].pMap;
	
	OS_EnterCritical();
	
	//展示占用数，剩余数等等
	StatsInfoItemNum(Block,&IdleNum,&ValidNum,&RemovedNum);
	
	Debug("Block[%u] Idle:%4u, Vaild:%4u, Removed:%4u @ Sector %u:%u\n\r",Block,IdleNum,ValidNum,RemovedNum,GetBlockStartSec(Block),gBlockAttrib[Block].SectorNum);
	
	//展示每个区域map
	for(Unit=0;Unit<gBlockAttrib[Block].UnitTotal;Unit++)
	{
		switch(pMap[Unit].Type)
		{
			case IFT_IDLE:
				Debug("_");
				break;
			case IFT_NOVALID:
				Debug("*");
				break;
			default:
				Debug("%x",pMap[Unit].Type);
		}

		if(Unit%64==(64-1)) Debug("\n\r");
	}

	OS_ExitCritical();
}

//遍历存储区，看是否要格式化，即全部flag全部变为0xfe
//检查每个存储块不符合就格式化
//检查存储区冗余情况，必要时整理存储区，建立存储区type映射表
//ForceClean==TRUE恢复出厂设置
//删除块过多时，自动整理
//任何时候都可以调用此函数用来缩减存储区大小
void InfoBuildBlock(INFO_BLOCK Block,bool ForceClean)
{
	if(Block>=IBN_MAX) return;

	OS_EnterCritical();
	
	if(ForceClean)//强制恢复出厂设置
	{
		FromatBlock(Block);//擦除并格式化
	}
	else //自检
	{
		if(CheckBlockFat(Block)==TRUE)//正常情况下，应该是格式化状态(有效区)
		{
			TidyBlock(Block);//整理存储区
		}
		else
		{
			FromatBlock(Block);//擦除并格式化
		}	
	}

	OS_ExitCritical();
}

//初始化info数据库
//ForceClean==TRUE恢复出厂设置
//删除块过多时，自动整理
//任何时候都可以调用此函数用来缩减所有存储区大小
void InfoSaveInit(bool ForceClean)
{
	INFO_BLOCK Block;
	INFO_TYPE Type;
	
#if 1	//检查参数
	for(Block=(INFO_BLOCK)0;Block<IBN_MAX;Block++)
	{
		if(gBlockAttrib[Block].UnitTotal > gBlockAttrib[Block].SectorNum*FLASH_SECTOR_BYTES/gBlockAttrib[Block].UnitSize)
		{
			Debug("INFO_BLOCK %u UnitTotal is too big!\n\r",Block);
			while(1);
		}

		if(Block!=(INFO_BLOCK)0)
		{
			if(GetBlockStartSec(Block)!=GetBlockStartSec(Block-1)+gBlockAttrib[Block-1].SectorNum)
			{
				Debug("INFO_BLOCK %u StartSec error!\n\r",Block);
				while(1);
			}
		}		
	}

	for(Type=(INFO_TYPE)0;Type<IFT_MAX;Type++)
	{
		if(gTypeAttrib[Type].Type!=Type)
		{
			Debug("INFO_TYPE %u Type is error!\n\r",Type);
			while(1);
		}
		
		if((gTypeAttrib[Type].ItemBytes+sizeof(INFO_HEADER))>gBlockAttrib[gTypeAttrib[Type].Block].UnitSize)
		{
			Debug("INFO_TYPE %u Param is error! %u\n\r",Type,gTypeAttrib[Type].ItemBytes+sizeof(INFO_HEADER));
			while(1);
		}

		if(gTypeAttrib[Type].ItemBytes % 4)
		{
			Debug("INFO_TYPE %u ItemBytes is error!\n\r",Type);
			while(1);
		}
	}		
#endif 

	for(Block=(INFO_BLOCK)0;Block<IBN_MAX;Block++)
	{
		InfoBuildBlock(Block,ForceClean);
	}

	for(Type=(INFO_TYPE)0;Type<IFT_MAX;Type++)
	{
		Debug("[%s]:%u\n\r",gNameInfoName[Type],GetTypeInfoTotal(Type));	
	}

	if(ForceClean)
	{
		for(Type=(INFO_TYPE)0;Type<IFT_MAX;Type++) UpdateInfoFlag(Type);
	}

	Debug("\n\r");
}

//删除flash里指定位置的info信息
//返回原来的绝对存储位置
//未找到返回0
INFO_ADDR DeleteInfo(INFO_TYPE Type,INFO_ID InfoID)
{
	INFO_BLOCK Block;
	INFO_SITE Site;
	INFO_ADDR Addr;
	INFO_HEADER Header;

	if(Type>=IFT_MAX) return INFO_PARAM_ERROR;
	if(InfoID==0) return INFO_PARAM_ERROR;

	Block=gTypeAttrib[Type].Block;

	OS_EnterCritical();
	Site=FindItemByID(Block,Type,InfoID);
	if(Site<0) 
	{
		OS_ExitCritical();
		return 0;
	}
	
	SetItemMapFlag(Block,Site,IFT_NOVALID,0);//修改索引标记
	Addr=SiteToAddr(Block,Site);//得到真实地址

	Header.Flag=IBF_Removed;
	Header.Type=IFT_IDLE;
	Header.Resv=0xffff;
	SpiFlsWriteData(Addr,sizeof(Header),(void *)&Header);
	UpdateInfoFlag(Type);
	
	OS_ExitCritical();
	return Addr;
}


//添加新info信息到flash，返回绝对存储位置
//返回INFO_RES_SPACE_FULL表示没空间
INFO_ADDR SaveInfo(INFO_TYPE Type,void *pData)
{
	INFO_BLOCK Block;
	INFO_SITE Site;
	INFO_ADDR Addr;
	INFO_HEADER Header;
	INFO_ID InfoID;

	if(Type>=IFT_MAX) return INFO_PARAM_ERROR;
	
	if(pData==NULL) return INFO_PARAM_ERROR;
	else MemCpy(&InfoID,pData,sizeof(INFO_ID));//拷贝id

	if(InfoID==0) return INFO_PARAM_ERROR;
	
	Block=gTypeAttrib[Type].Block;

	OS_EnterCritical();
	Site=FindItemByIdx(Block,IFT_IDLE,1);
	if(Site<0)
	{
		OS_ExitCritical();
		return INFO_SPACE_FULL;	//资源不足
	}
	
	SetItemMapFlag(Block,Site,Type,InfoID);//修改索引标记
	Addr=SiteToAddr(Block,Site);//得到真实地址

	Header.Flag=IBF_Vaild;
	Header.Type=Type;
	Header.Resv=0xffff;
	SpiFlsWriteData(Addr,sizeof(Header),(void *)&Header);
	if(pData!=NULL) SpiFlsWriteData(Addr+sizeof(Header),gTypeAttrib[Type].ItemBytes,pData);
	UpdateInfoFlag(Type);
	
	OS_ExitCritical();
	return Addr;
}

//覆盖相同InfoID的info信息到flash，返回绝对存储位置
//不做数据对比和校验
//由于flash只能由1变0的特性，如果数据不对，可能存储结果并非程序预期
INFO_ADDR CoverInfo(INFO_TYPE Type,void *pData)
{
	INFO_BLOCK Block;
	INFO_SITE Site;
	INFO_ADDR Addr;
	INFO_HEADER Header;
	INFO_ID InfoID;

	if(Type>=IFT_MAX) return INFO_PARAM_ERROR;
	
	if(pData==NULL) return INFO_PARAM_ERROR;
	else MemCpy(&InfoID,pData,sizeof(INFO_ID));//拷贝id

	if(InfoID==0) return INFO_PARAM_ERROR;
		
	Block=gTypeAttrib[Type].Block;

	OS_EnterCritical();
	Site=FindItemByID(Block,Type,InfoID);
	if(Site<0) 
	{
		OS_ExitCritical();
		return INFO_PARAM_ERROR;
	}	
	Addr=SiteToAddr(Block,Site);//得到真实地址

	SpiFlsWriteData(Addr+sizeof(Header),gTypeAttrib[Type].ItemBytes,pData);
	UpdateInfoFlag(Type);

	OS_ExitCritical();
	return Addr;
}

//将实际数据的头4字节作为InfoID，轮询存储块，匹配时返回信息
//返回读取大小
u16 ReadInfoByID(INFO_TYPE Type,INFO_ID InfoID,void *pData)
{
	INFO_BLOCK Block;
	INFO_SITE Site;
	INFO_ADDR Addr;

	if(Type>=IFT_MAX) return 0;
	if(InfoID==0) return 0;
	
	Block=gTypeAttrib[Type].Block;

	OS_EnterCritical();
	Site=FindItemByID(Block,Type,InfoID);
	if(Site<0)
	{
		OS_ExitCritical();
		return 0;
	}
	
	Addr=SiteToAddr(Block,Site);//得到真实地址

	if(pData!=NULL)
		SpiFlsReadData(Addr+sizeof(INFO_HEADER),gTypeAttrib[Type].ItemBytes,pData);

	OS_ExitCritical();
	return gTypeAttrib[Type].ItemBytes;
}

//根据索引顺序读取info信息
//返回读取大小
u16 ReadInfoByIdx(INFO_TYPE Type,INFO_IDX Idx,void *pData)
{
	INFO_BLOCK Block;
	INFO_SITE Site;
	INFO_ADDR Addr;

	if(Type>=IFT_MAX) return 0;

	Block=gTypeAttrib[Type].Block;

	if(Idx==0 || Idx>gBlockAttrib[Block].UnitTotal) return 0;

	OS_EnterCritical();
	Site=FindItemByIdx(Block,Type,Idx);
	if(Site<0)
	{
		OS_ExitCritical();
		return 0;
	}
	
	Addr=SiteToAddr(Block,Site);//得到真实地址

	if(pData!=NULL)
		SpiFlsReadData(Addr+sizeof(INFO_HEADER),gTypeAttrib[Type].ItemBytes,pData);

	OS_ExitCritical();
	return gTypeAttrib[Type].ItemBytes;
}

//获取信息总数
u16 GetTypeInfoTotal(INFO_TYPE Type)
{
	INFO_BLOCK Block=gTypeAttrib[Type].Block;
	INFO_MAP *pMap=NULL;
	s32 i,Num=0;

	if(Type>=IFT_MAX) return 0;
	pMap=gBlockAttrib[Block].pMap;
	
	OS_EnterCritical();
	
	for(i=0;i<gBlockAttrib[Block].UnitTotal;i++)
	{
		if(pMap[i].Type==IFT_IDLE) break;
		if(pMap[i].Type==Type) Num++;
	}	

	OS_ExitCritical();
	return Num;
}

//获取信息大小
u16 GetTypeItemSize(INFO_TYPE Type)
{
	if(Type>=IFT_MAX) return 0;
	
	return gTypeAttrib[Type].ItemBytes;
}

//获取空白单元数
u16 GetBlockFreeUnit(INFO_BLOCK Block)
{
	INFO_MAP *pMap;
	s32 i,Num=0;

	if(Block>=IBN_MAX) return 0;

	OS_EnterCritical();
	pMap=gBlockAttrib[Block].pMap;

	for(i=gBlockAttrib[Block].UnitTotal-1;i>=0;i--,Num++)//从屁股往前数空白格即可
	{
		if(pMap[i].Type!=IFT_IDLE) break;
	}	

	OS_ExitCritical();
	return Num;
}

//获取指定类型的空闲数目
u16 GetTypeFreeUnit(INFO_TYPE Type)
{
	return GetBlockFreeUnit(gTypeAttrib[Type].Block);
}

//获取指定类型的块大小
u16 GetTypeBlockSize(INFO_TYPE Type)
{
	return 1<<(7+gTypeAttrib[Type].Block);
}

