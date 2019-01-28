#ifndef INFO_SAVE_H
#define INFO_SAVE_H

typedef s32 INFO_ADDR;//���Ե�ַ
typedef s32 INFO_SITE;//��Ե�ַ����0��ʼ
typedef u32 INFO_ID;//�û�����ָ����id������ֵ
typedef u32 INFO_IDX;//��Ч��Ϣ��������1��ʼ

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
	IFT_STR=0,//�ַ���
	IFT_VARIABLE,//����
	IFT_RF_DATA,
	IFT_IR_DATA,
	IFT_DEV,//�豸
	IFT_TRIGGER,//����Դ
	IFT_SCENE,//�龰

	IFT_MAX,
	
	//ʵ��ʹ��ֻ��ȡ0-13�����ȡ14,15����info save�����лᷢ������map��ͻ����

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

