#ifndef INFO_SAVE_H
#define INFO_SAVE_H

typedef s32 INFO_ADDR;//绝对地址
typedef s32 INFO_SITE;//相对地址，从0开始
typedef u32 INFO_ID;//用户程序指定的id，非零值
typedef u32 INFO_IDX;//有效信息索引，从1开始

#define INFO_FAILD (-1)
#define INFO_NOT_FOUND (-2)
#define INFO_PARAM_ERROR (-3)
#define INFO_RES_ERROR (-4)
#define INFO_SPACE_FULL (-5)

enum{
	IBN_64B=0,
	IBN_128B,
	IBN_256B,
	IBN_512B,

	IBN_MAX	
};
typedef u8 INFO_BLOCK; 

enum{
	IFT_STR=0,//字符串
	IFT_VARIABLE,//变量
	IFT_RF_DATA,
	IFT_IR_DATA,
	IFT_DEV,//设备
	IFT_TRIGGER,//触发源
	IFT_SCENE,//情景

	IFT_MAX,
	
	//实际使用只能取0-13，如果取14,15，在info save管理中会发生类型map冲突错误

	IFT_NOVALID=0x0e,
	IFT_IDLE=0x0f,	

	IFT_SAVE_NUM=16
};
typedef u8 INFO_TYPE;

void DebugInfoSave(INFO_BLOCK Block);
void InfoBuildBlock(INFO_BLOCK Block,bool ForceClean);
void InfoSaveInit(bool ForceClean);
INFO_ADDR SaveInfo(INFO_TYPE Type,void *pData);
INFO_ADDR DeleteInfo(INFO_TYPE Type,INFO_ID AppID);
INFO_ADDR CoverInfo(INFO_TYPE Type,void *pData);
u16 ReadInfoByID(INFO_TYPE Type,INFO_ID AppID,void *pData);
u16 ReadInfoByIdx(INFO_TYPE Type,INFO_IDX Idx,void *pData);
u16 GetTypeInfoTotal(INFO_TYPE Type);
u16 GetTypeItemSize(INFO_TYPE Type);
u16 GetBlockFreeUnit(INFO_BLOCK Block);
u16 GetTypeFreeUnit(INFO_TYPE Type);
u16 GetTypeBlockSize(INFO_TYPE Type);

#endif

