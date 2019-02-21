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

本文件定义了一个基于spi flash的数据存储机制
*/
//---------------------------------------------------------------------------------------------//
#include "SysDefines.h"
#include "SpiFlashApi.h"

#define DB_Debug Debug

typedef void (*DbInit_CB)(void);
typedef u32 (*GetValue_CB)(u16 Item,u32 IntParam,void *Val);
typedef bool (*SetValue_CB)(u16 Item,u32 IntParam,void *pParam,u16 ByteLen);
typedef void (*BurnDefault_CB)(void);

#define DBF_REMOVED 0x55550000//废弃
#define DBF_USED 0x55555555//使用中
#define DBF_IDLE 0x5555ffff//空闲

typedef struct{	
	u32 Flag;//读取标志，无需用户干预
	u32 Ver;//版本，无需用户干预
	u32 ChkSum;//校验和，无需用户干预
	
	u8 Data[4];//用户数据区域
}DB_STRUCT;	//数据库

typedef struct{
	const u8 *pName;
	u16 StartSector;//起始扇区号
	u16 SectorNum;//占用扇区个数
	u16 OccupySize;//每个单元占用的字节数，必须是扇区的整分尺寸
	const DB_STRUCT *pDefaultData;//默认值存储空间
	DB_STRUCT *pData;//当前值存储空间
	u32 DataBytes;//存储结构体大小，含ver check等前体部分
	DbInit_CB DbInitFunc;//数据库初始化的回调
	GetValue_CB GetValueFunc;
	SetValue_CB SetValueFunc;
	BurnDefault_CB BurnDefaultFunc;//恢复默认函数回调
}SUB_DB_STRUCT;

static u32 gNowDbAddr[SDN_MAX];//当前db有效位置

#define GetStartAddr(n) (gSubDbs[n].StartSector*FLASH_SECTOR_BYTES)
//DB版本:工程的版本、flash布局、数据库默认内容发生改变时，此值会变。
#define GetDbVer(n) (__gBinSoftVer+gSubDbs[n].StartSector+gSubDbs[n].SectorNum+gSubDbs[n].OccupySize  \
									+gSubDbs[n].DataBytes+MakeHash33((void *)gSubDbs[n].pDefaultData,gSubDbs[n].DataBytes))


#if 1//用户自定义区域
extern const SYS_DB_STRUCT gDefSysDb;
extern SYS_DB_STRUCT gSysDb;
extern const QUICK_SAVE_STRUCT gDefQuickSave;
extern QUICK_SAVE_STRUCT gQuickSave;
extern const HW_CONF_STRUCT gDefHwConf;
extern HW_CONF_STRUCT gHwConf;

const SUB_DB_STRUCT gSubDbs[SDN_MAX]={//存储位置定义
//StartSector SectorNum UnitPageNum pDefaultData pData DataBytes GetValue SetValue BurnDefault
{"SysDB",			FM_DATABASE_BASE_SECTOR,		FM_DATABASE_SEC_NUM,	1024,	(void *)&gDefSysDb,(void *)&gSysDb,sizeof(SYS_DB_STRUCT),Sys_DbInit,Sys_GetValue,Sys_SetValue,Sys_Default},
{"QuickSave",	FM_QUICKSAVE_BASE_SECTOR,	FM_QUICKSAVE_SEC_NUM,	2048,		(void *)&gDefQuickSave,(void *)&gQuickSave,sizeof(QUICK_SAVE_STRUCT),QSS_DbInit,QSS_GetValue,QSS_SetValue,QSS_Default},
{"HwConf",		FM_HWCONF_BASE_SECTOR,		FM_HWCONF_SEC_NUM,		256,		(void *)&gDefHwConf,(void *)&gHwConf,sizeof(HW_CONF_STRUCT),HWC_DbInit,HWC_GetValue,HWC_SetValue,HWC_Default},
};
#endif



#if 0 //for debug
static void DB_BufDisp(void)
{
	u8 buf[FLASH_PAGE_SIZE];
	int i,j;

	DB_Debug("Spi Flash:\n\r");
	for(j=0;j<16;j++)
	{
		Q_SpiFlashSync(FlashRead,GetStartAddr(Name)+j*256,sizeof(buf),buf);
		DB_Debug("page %d context:",j);
		for(i=0;i<FLASH_PAGE_SIZE;i++)
		{
			if(buf[i]!=0xff)
			{
				DB_Debug("0x%02x ",buf[i]);
			}
		}
		DB_Debug("\n\r");
	}
}
#endif

void QDB_Debug(void)
{	
	SUB_DB_NAME Name;
	
	//检查数据
	for(Name=(SUB_DB_NAME)0;Name<SDN_MAX;Name++)
	{
		if((gSubDbs[Name].DataBytes) > gSubDbs[Name].OccupySize)
		{
			Debug("!!!DB[%d] init error!\n\r***Flash size[%d] for database[%d] is too small!\n\r",Name,gSubDbs[Name].OccupySize,gSubDbs[Name].DataBytes);
			while(1);
		}

		Debug("DB[%d] At:%u.%u\n\r",Name,gNowDbAddr[Name]/FLASH_SECTOR_BYTES,(gNowDbAddr[Name]%FLASH_SECTOR_BYTES)/FLASH_PAGE_SIZE);
		Debug("DB[%d] Burn Start Sector:%u\n\r",Name,gSubDbs[Name].StartSector);
		Debug("DB[%d] Version:%x-%x\n\r",Name,gSubDbs[Name].pData->Ver,GetDbVer(Name));
		Debug("DB[%d] ChkSum:%x\n\r",Name,gSubDbs[Name].pData->ChkSum);
		Debug("DB[%d] Size:%u Byte < OccupySize (=%u Byte)\n\r\n\r",Name,gSubDbs[Name].DataBytes,gSubDbs[Name].OccupySize);
	}
}


static u32 QDB_NowUnitSec(SUB_DB_NAME Name)//获取当前扇区
{
	if(gNowDbAddr[Name]==0) gNowDbAddr[Name]=gSubDbs[Name].StartSector*FLASH_SECTOR_BYTES;

	return gNowDbAddr[Name]/FLASH_SECTOR_BYTES;
}

static u32 QDB_GetNextUnitSec(SUB_DB_NAME Name)//获取下一个扇区
{
	u32 NowSec;
	
	if(gNowDbAddr[Name]==0) gNowDbAddr[Name]=gSubDbs[Name].StartSector*FLASH_SECTOR_BYTES;

	NowSec=gNowDbAddr[Name]/FLASH_SECTOR_BYTES;

	if(NowSec == (gSubDbs[Name].StartSector+gSubDbs[Name].SectorNum-1)) return gSubDbs[Name].StartSector;
	else return NowSec+1;	
}

//写默认值到flash和系统
void QDB_BurnDefaultToSpiFlash(SUB_DB_NAME Name)
{
	DB_Debug("Now burn DB[%u] DEF to spi flash!\n\r",Name);

	OS_EnterCritical();
	
	SpiFlsEraseSector(QDB_NowUnitSec(Name),1);//擦除1个存放扇区
	gNowDbAddr[Name]=QDB_NowUnitSec(Name)*FLASH_SECTOR_BYTES;

	//将默认值写到系统和flash
	MemCpy((void *)gSubDbs[Name].pData,(void *)gSubDbs[Name].pDefaultData,gSubDbs[Name].DataBytes);//将默认数据拷贝到系统
	if(gSubDbs[Name].BurnDefaultFunc != NULL) gSubDbs[Name].BurnDefaultFunc();
	gSubDbs[Name].pData->Flag=DBF_USED;
	gSubDbs[Name].pData->Ver=GetDbVer(Name);
	gSubDbs[Name].pData->ChkSum=0;//先赋0值，避免影响校验和
	gSubDbs[Name].pData->ChkSum=MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes);
	DB_Debug("Database chksum is %x %x-%x\n\r",gSubDbs[Name].pData->ChkSum,gSubDbs[Name].pData,gSubDbs[Name].DataBytes);
	SpiFlsWriteData(gNowDbAddr[Name],gSubDbs[Name].DataBytes,(void *)gSubDbs[Name].pData);//将默认数据拷贝到flash db扇区

	OS_ExitCritical();
}

//将缓存烧写到flash
void QDB_BurnToSpiFlash(SUB_DB_NAME Name)
{
	u32 OldChkSum;
	u32 DbFlag=DBF_REMOVED;
	u32 OldAddr=gNowDbAddr[Name];	

	OS_EnterCritical();

	OldChkSum=gSubDbs[Name].pData->ChkSum;
	gSubDbs[Name].pData->ChkSum=0;//先赋0值，避免影响校验和
	gSubDbs[Name].pData->ChkSum=MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes);//记录旧校验和
	if(gSubDbs[Name].pData->ChkSum==OldChkSum)//校验和一样，不予处理		
	{
		OS_ExitCritical();
		return;
	}
	
	if(gNowDbAddr[Name] == ((QDB_NowUnitSec(Name)+1)*FLASH_SECTOR_BYTES-gSubDbs[Name].OccupySize))//本扇区最后一页，需要写到下个扇区
	{
		SpiFlsEraseSector(QDB_GetNextUnitSec(Name),1);//擦除下个扇区
		gNowDbAddr[Name]=QDB_GetNextUnitSec(Name)*FLASH_SECTOR_BYTES;		
	}
	else //非本扇区最后一页，直接写入
	{
		gNowDbAddr[Name]+=(gSubDbs[Name].OccupySize);
	}
	
	gSubDbs[Name].pData->Flag=DBF_USED;
	gSubDbs[Name].pData->Ver=GetDbVer(Name);
	gSubDbs[Name].pData->ChkSum=0;//先赋0值，避免影响校验和
	gSubDbs[Name].pData->ChkSum=MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes);		
	//DB_Debug("Database Chk:%x,P:%x,Len:%u\n\r",gSubDbs[Name].pData->ChkSum,gSubDbs[Name].pData,gSubDbs[Name].DataBytes);
	SpiFlsWriteData(gNowDbAddr[Name],gSubDbs[Name].DataBytes,(void *)gSubDbs[Name].pData);//将数据拷贝到flash db扇区
	//Debug("Burn At: %u.%u\n\r",gNowDbAddr[Name]/FLASH_SECTOR_BYTES,(gNowDbAddr[Name]%FLASH_SECTOR_BYTES)/FLASH_PAGE_SIZE);

	SpiFlsWriteData(OldAddr,sizeof(u32),(void *)&DbFlag);//擦除标志
	//Debug("Set Remove Flag At: %u.%u\n\r",OldAddr/FLASH_SECTOR_BYTES,(OldAddr%FLASH_SECTOR_BYTES)/FLASH_PAGE_SIZE);
		
	OS_ExitCritical();
}

//从存储体读出数据库到内存，通过指针返回当前存储页
static void QDB_ReadFromSpiFlash(SUB_DB_NAME Name)
{
	u32 Idx;
	u32 DbFlag;
	u32 ChkSum;

	OS_EnterCritical();
	
	for(Idx=0;Idx<(gSubDbs[Name].SectorNum*FLASH_SECTOR_BYTES/gSubDbs[Name].OccupySize);Idx++) //遍历存储区
	{
		gNowDbAddr[Name]=GetStartAddr(Name)+Idx*gSubDbs[Name].OccupySize;		
		SpiFlsReadData(gNowDbAddr[Name],sizeof(u32),(void *)&DbFlag);//读标志位
		
		if(DbFlag == DBF_USED)//读到正确数据
		{
			Debug("Read DB[%u] Idx:%u, At:%u.%u\n\r",Name,Idx,gNowDbAddr[Name]/FLASH_SECTOR_BYTES,(gNowDbAddr[Name]%FLASH_SECTOR_BYTES)/FLASH_PAGE_SIZE);
			SpiFlsReadData(gNowDbAddr[Name],gSubDbs[Name].DataBytes,(void *)gSubDbs[Name].pData);

			if(gSubDbs[Name].pData->Ver!=GetDbVer(Name))//检查版本是否正确
			{
				DB_Debug("Database[%u] version 0x%x is not right(!=0x%x)!\n\rBurn All DB DEF to flash\n\r",Name,gSubDbs[Name].pData->Ver,GetDbVer(Name));
				for(Name=(SUB_DB_NAME)0;Name<SDN_MAX;Name++) QDB_BurnDefaultToSpiFlash(Name);
				OS_ExitCritical();
				return;
			}

			ChkSum=gSubDbs[Name].pData->ChkSum;
			gSubDbs[Name].pData->ChkSum=0;//先赋0值，避免影响校验和
			if(ChkSum!= MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes))//检查校验和是否正确
			{
				DB_Debug("Database[%u] chksum is not right!(%x != %x) %x-%x\n\rsys halt!\n\r",Name,ChkSum, MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes),gSubDbs[Name].pData->Data,gSubDbs[Name].DataBytes);
				QDB_BurnDefaultToSpiFlash(Name);//校验和不对说明数据库被毁坏，需重建
			}
			else
			{
				gSubDbs[Name].pData->ChkSum=ChkSum;//还原校验和
			}

			OS_ExitCritical();
			return;
		}
	}

	OS_ExitCritical();
	
	DB_Debug("Not read database for DB[%d]!\n\rBurn default database!\n\r",Name);
	QDB_BurnDefaultToSpiFlash(Name);//没读到数据库，直接重建
}

//初始化函数 作用如下
// 1.从flash读取数据库内容
// 2.如果没有读到数据库，则将默认值数据库烧写到flash
// 3.如果有数据库，则读取数据库到缓存
void QDB_Init(void)
{	
	SUB_DB_NAME Name;
	
	Debug("Database Init\n\r");

	//检查数据
	for(Name=(SUB_DB_NAME)0;Name<SDN_MAX;Name++)
	{
		gNowDbAddr[Name]=0;
		
		if((gSubDbs[Name].DataBytes) > gSubDbs[Name].OccupySize)
		{
			Debug("!!!DB[%d] init error!\n\r***Flash size[%d] for database[%d] is too small!\n\r",Name,gSubDbs[Name].OccupySize,gSubDbs[Name].DataBytes);
			while(1);
		}
		
		QDB_ReadFromSpiFlash(Name);
		if(gSubDbs[Name].DbInitFunc!=NULL) gSubDbs[Name].DbInitFunc();
		
		Debug("DB[%d] Burn Start Sector:%u\n\r",Name,gSubDbs[Name].StartSector);
		Debug("DB[%d] Version:%x-%x\n\r",Name,gSubDbs[Name].pData->Ver,GetDbVer(Name));
		Debug("DB[%d] ChkSum:%x\n\r",Name,gSubDbs[Name].pData->ChkSum);
		Debug("DB[%d] Size:%u Byte < OccupySize (=%u Byte)\n\r\n\r",Name,gSubDbs[Name].DataBytes,gSubDbs[Name].OccupySize);
	}
}

//从数据库缓存读取值，只能使用pDB_Setting获取
u32 QDB_GetValue(SUB_DB_NAME Name,u16 Item,u32 IntParam,void *Val)
{
	if(gSubDbs[Name].GetValueFunc != NULL)	
		return gSubDbs[Name].GetValueFunc(Item,IntParam,Val);

	return 0;
}

//写值到数据库缓存，只能使用pDB_Setting存储
bool QDB_SetValue(SUB_DB_NAME Name,u16 Item,u32 IntParam,void *pParam,u16 ByteLen)
{
	if(gSubDbs[Name].SetValueFunc != NULL)
		return gSubDbs[Name].SetValueFunc(Item,IntParam,pParam,ByteLen);	

	return FALSE;
}

//获取当前缓存里的数据校验和，当任意数据改变时，此值会变
u32 QDB_GetNowChkSum(SUB_DB_NAME Name)
{
	u32 ChkSum=gSubDbs[Name].pData->ChkSum;
	u32 Ret;
	
	gSubDbs[Name].pData->ChkSum=0;//先赋0值，避免影响校验和
	Ret=MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes);		
	gSubDbs[Name].pData->ChkSum=ChkSum;
	
	return Ret;
}

