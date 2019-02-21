//--------------------------Q-IOT Platform------------------------------------------------------//
/*
Q-Wifi�ǿ�������ƽ̨�Ĺ̼�ʵ�֣�����ESP8266������������һ�������APP��������ƽ̨�໥��ϵ�������
�ײ�ʵ�ֻ��ƣ��������ESP8266_RTOS_SDK��������Դ����ӭ�������Ա��ţ���룡

Q-Wifi������һ���豸���������龰���������û��ն˵Ĺ�����ƣ��ڽ������ɸ�TCP������APP����ƽ̨������
����json�ϱ����ӣ�����ͨ�õ�http json��ʽ���ɱ������ߵ������Լ���������ƽ̨���������ݻ㼯�Ϳ��ơ�

Q-Wifi�ڽ���һ�׶�̬��web��ҳ����������������Ӳ�����ã���web��ҳ��������װ�������ɱ���������ֲ��
������Ŀ��

Q-Wifi�ڽ���һ�׻��ڴ��ڵ�ָ�����ϵͳ���������д��ڵ��ԣ�ָ���·����˽���ϵͳ��װ����������һϵ
�н����������ɱ���������ֲ��������Ŀ��

Q-Wifi�ڲ���user��ʶ���ļ�����Ϊ֧�ֿ����������޸ĵĿ��ƻ��ļ����ر���user_hook.c�ļ���ϵͳ����
�йؼ��㶼����hook�����ڴ��ļ��У��������߶��ο�����

Q-Wifi����ӵ���ڶ�ģ�黯�Ļ��ƻ򷽷������Ա����ü���ֲ������������ϵͳ�Ŀ����Ѷȡ�
���л��ڿ�������ƽ̨���еĿ�����������Ʒ��������ϵ�����Ŷӣ���ѷ����ڿ�������Ƶ��q-iot.cn������
�������г���������Ӧ���п۳�˰�Ѽ�ά�����ú󣬾�ȫ���ṩ�������ߣ��Դ˹������ڿ�Դ��ҵ��

By Karlno ����Ƽ�

���ļ�������һ�����������ݿ�����ݴ洢����
*/
//---------------------------------------------------------------------------------------------//
#include "SysDefines.h"
#include "SpiFlashApi.h"

//info�洢�鲻Ӧ��ʵ��flash��0��ַ��ʼ��������Ϊ�˱��Ȿҳ�����е�FlashAddr����Ϊ0

//Addr:ָ��Ϣ�洢��ľ��Ե�ַ
//Site:ָ��Ϣ�洢���λ�ã���1��ʼ���洢�ĵ�1����Ϣ���ַΪ1���洢�ĵ�2����Ϣ��Ϊ2
//ID:ָ��Ϣ�ڲ���Ӧ�ó���ָ����id�������0
//Idx:ָ��Ϣ�洢����Ч��Ϣ��˳����������1��ʼ�������1,3����Ϣ����Ч����2����Ϣ��Ϊ�����飬��ô��������Ϣ��IdxΪ2��������3

enum{
	IBF_Removed=0xe0,//�˿����ã���ϢҲ�Ѿ���ɾ��
	IBF_Vaild=0xf0,//�˿���Ϣ��Ч
	IBF_Null=0xfe,//�˿�հ�δʹ��
};
typedef u8 INFO_BLOCK_FLAG;

typedef struct{
	INFO_BLOCK_FLAG Flag;///������ʾ����Ϣ����ڻ��Ǳ�ɾ��
	INFO_TYPE Type;//����
	u16 Resv;
}INFO_HEADER;

typedef struct{
	INFO_BLOCK_FLAG Flag;///������ʾ����Ϣ����ڻ��Ǳ�ɾ��
	INFO_TYPE Type;//����
	u16 Resv;
	INFO_ID InfoID;//���û����壬��info�ṹ�޹أ�����ŵ��û����ݵ���ǰ��
}INFO_HEADER_AND_ID;

typedef struct{
	INFO_TYPE Type:4;//0-14���ã�15������ʾ��
	u8 IdBits:4;//�洢id�����4λ���������ٶԱ�
}INFO_MAP;//����һ���ֽڣ���������ͣ�id��������

typedef struct{
	u32 StartSec;//��ʼ����������FM_INFOSAVE_BASE_SECTOR��ƫ��ֵ
	u32 SectorNum;//ռ��������
	u32 UnitSize;//��Ԫ�ֽ�������4��ͷ�ֽ�
	u32 UnitTotal;//��Ԫ����
	INFO_MAP *pMap;//ӳ���
}INFO_BLOCK_ATTRIB;//�洢����Ϣ

typedef struct{
	INFO_TYPE Type;
	INFO_BLOCK Block;
	u16 ItemBytes;//����С�ڵ���INFO_BLOCK_ATTRIB.UnitSize-4
}INFO_TYPE_ATTRIB;

//Ѱַ�������ÿ��item�����ʹ�Ŵ˴���0��ʾ��ɾ����0xff��ʾδ��
static INFO_MAP gpTypeMapB64[512];//�����С���ӦgBlockAttrib�޸�
static INFO_MAP gpTypeMapB128[256];//�����С���ӦgBlockAttrib�޸�
static INFO_MAP gpTypeMapB256[128];//�����С���ӦgBlockAttrib�޸�
static INFO_MAP gpTypeMapB512[64];//�����С���ӦgBlockAttrib�޸�

//�洢����ÿ���洢����������1024���ɹ��洢����
//ÿ������64kB����256ҳ
static const INFO_BLOCK_ATTRIB gBlockAttrib[IBN_MAX]={ //ÿ���洢����Ϣ
//	StartSec		SectorNum			UnitSize		UnitTotal
{0,		8,		FLASH_PAGE_SIZE/4	,	512,gpTypeMapB64},//IBN_64B
{8,		8,		FLASH_PAGE_SIZE/2	,	256,gpTypeMapB128},//IBN_128B
{16,		8,		FLASH_PAGE_SIZE,		128,gpTypeMapB256},//IBN_256B
{24,		8,		FLASH_PAGE_SIZE*2	,	64,gpTypeMapB512},//IBN_512B
};

#define GetBlockStartSec(b) (FM_INFOSAVE_BASE_SECTOR+gBlockAttrib[b].StartSec)
#define GetBlockStartAddr(b) ((FM_INFOSAVE_BASE_SECTOR+gBlockAttrib[b].StartSec)*FLASH_SECTOR_BYTES)

static const INFO_TYPE_ATTRIB gTypeAttrib[IFT_MAX]={ //ÿ�����͵���Ϣ
//Type		Block		ItemBytes
{IFT_STR,				IBN_128B,		sizeof(STR_RECORD)},
{IFT_VARIABLE,		IBN_64B,			sizeof(VARIABLE_RECORD)},
{IFT_RF_DATA,		IBN_256B,		252},//ռλ������ɾ
{IFT_IR_DATA,		IBN_256B,		252},//ռλ������ɾ
{IFT_DEV,				IBN_256B,		sizeof(DEVICE_RECORD)},
{IFT_TRIGGER,		IBN_64B,			sizeof(TRIGGER_RECORD)},
{IFT_SCENE,			IBN_512B,		sizeof(SCENE_RECORD)},
};


#if 1 //��������أ�����ŵ�map����֮��ʹ��
static void TypeMapInit(INFO_BLOCK Block)
{
	MemSet(gBlockAttrib[Block].pMap,0xff,gBlockAttrib[Block].UnitTotal*sizeof(INFO_MAP));
}

//���ݶ����Ĵ洢���ݣ�����map
static void BuildTypeMap(INFO_BLOCK Block)
{
	INFO_MAP *pMap=gBlockAttrib[Block].pMap;
	INFO_HEADER_AND_ID Header;
	u32 Unit;

	TypeMapInit(Block);

	for(Unit=0;Unit<gBlockAttrib[Block].UnitTotal;Unit++)//������ȡ��Ϣͷ
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

//�޸��������
static void SetItemMapFlag(INFO_BLOCK Block,INFO_SITE Site,INFO_TYPE Type,u32 InfoID)
{
	INFO_MAP *pMap=gBlockAttrib[Block].pMap;
	pMap[Site].Type=Type;
	pMap[Site].IdBits=InfoID;
}

//�õ���ʵ��ַ
static INFO_ADDR SiteToAddr(INFO_BLOCK Block,INFO_SITE Site)
{
	return GetBlockStartAddr(Block)+Site*gBlockAttrib[Block].UnitSize;
}

//����app id��Ѱ��Ŀ��Ҫ��ȡflash�����ԱȽϷ�ʱ��
static INFO_SITE FindItemByID(INFO_BLOCK Block,INFO_TYPE Type,INFO_ID InfoID)
{
	INFO_MAP *pMap=gBlockAttrib[Block].pMap;
	INFO_HEADER_AND_ID Header;
	u16 Site;

	for(Site=0;Site<gBlockAttrib[Block].UnitTotal;Site++)
	{
		if(pMap[Site].Type==Type && (pMap[Site].IdBits == (InfoID&0x0f)))//�ҵ�������
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

//������������λ��
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


#if 1 //�ڲ�����
//ͳ�Ƹ�������info����
static void StatsInfoItemNum(INFO_BLOCK Block,u16 *pIdleNum,u16 *pValidNum,u16 *pRemovedNum)
{
	INFO_HEADER Header;
	u32 Unit;

	for(Unit=0;Unit<gBlockAttrib[Block].UnitTotal;Unit++)//������ȡ��Ϣͷ
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

//���洢��
//����洢��δ����ʽ��������FALSE
//����洢���ѱ���ʽ��������TRUE
static bool CheckBlockFat(INFO_BLOCK Block)
{
	INFO_HEADER Header;
	u32 Unit;

	for(Unit=0;Unit<gBlockAttrib[Block].UnitTotal;Unit++)//������ȡ��Ϣͷ
	{
		SpiFlsReadData(GetBlockStartAddr(Block)+Unit*gBlockAttrib[Block].UnitSize,sizeof(Header),(void *)&Header);

		if(Header.Flag!=IBF_Removed && Header.Flag!=IBF_Vaild && Header.Flag!=IBF_Null)
		{
			return FALSE;
		}		
	}

	return TRUE;
}

//���洢���Ƿ�ȫΪ0xff
static bool CheckBlockIdle(INFO_BLOCK Block)
{
	u32 j,Addr,EndAddr=(GetBlockStartSec(Block)+gBlockAttrib[Block].SectorNum)*FLASH_SECTOR_BYTES;
	u32 *pBuf=Q_Zalloc(1024);

	for(Addr=GetBlockStartAddr(Block);Addr<EndAddr;Addr+=1024)//��ҳ��ȡ
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

//��������ʽ��ĳ��
static void FromatBlock(INFO_BLOCK Block)
{
	if(CheckBlockIdle(Block)!=TRUE)//��ȫ��
	{
		u32 Now;
		Debug("FromatBlock[%u] EraseSec:%u-%u,",Block,GetBlockStartSec(Block),gBlockAttrib[Block].SectorNum);
		Now=OS_GetNowMs();
		SpiFlsEraseSector(GetBlockStartSec(Block),gBlockAttrib[Block].SectorNum);//������blockȫ������
		Debug("Finish %dmS\n\r",OS_GetNowMs()-Now);
	}

	//��ʼ��ʽ��
	{
		INFO_HEADER InfoHeader={IBF_Null,0xff,0xffff};
		u32 Unit;

		for(Unit=0;Unit<gBlockAttrib[Block].UnitTotal;Unit++)//����������Ϣͷ
		{
			SpiFlsWriteData(GetBlockStartAddr(Block)+Unit*gBlockAttrib[Block].UnitSize,sizeof(INFO_HEADER),(void *)&InfoHeader);
		}
	}

	//���map
	TypeMapInit(Block);
}

//������϶�����´洢
//����flash����ֻ����4k���ص�
//�˹��̲��ܶϵ�
//��ɺ�Ὠ��map
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

	TypeMapInit(Block);//���map
			
	for(Addr=GetBlockStartAddr(Block);Addr<EndAddr;Addr+=FLASH_SECTOR_BYTES)
	{
		SpiFlsReadData(Addr,FLASH_SECTOR_BYTES,(void *)pSecBuf);//ÿ4kΪһ����λ��ȫ��������buf��
		SpiFlsEraseSector(Addr/FLASH_SECTOR_BYTES,1);//����4k����

		for(Unit=0,pBuf=(void *)pSecBuf;Unit<UintNum;Unit++,pBuf=(void *)(UnitSize+(u32)pBuf))//���unit��飬������Чunit��������
		{
			if(pBuf->Flag==IBF_Vaild)//��������Ч��
			{
				SpiFlsWriteData(DstAddr,UnitSize,(void *)pBuf);
				DstAddr+=UnitSize;
				pMap[MapIdx].Type=pBuf->Type;//���map
				pMap[MapIdx].IdBits=pBuf->InfoID;
				MapIdx++;				
			}		
		}		
	}

	//ʣ��Ŀռ�ȫ����ʽ��
	{
		INFO_HEADER InfoHeader={IBF_Null,0xff,0xffff};

		for(;DstAddr<EndAddr;DstAddr+=UnitSize)//����������Ϣͷ
		{
			SpiFlsWriteData(DstAddr,sizeof(INFO_HEADER),(void *)&InfoHeader);
		}
	}	

	Q_Free(pSecBuf);
}

//Force�Ƿ�ǿ������
//���ռ�÷�����̫�࣬����������洢��
//���������������true
//��ɺ󣬻Ὠ��map
static bool TidyBlock(INFO_BLOCK Block)
{
	u16 IdleNum=0,ValidNum=0,RemovedNum=0;
	u16 Total=gBlockAttrib[Block].UnitTotal;

	//ͳ�Ƹ���
	StatsInfoItemNum(Block,&IdleNum,&ValidNum,&RemovedNum);

	if((IdleNum+ValidNum+RemovedNum)!=Total)
	{
		Debug("Block Flag Num Is Error!\n\r");
	}

	//����Ƿ���Ҫ����
	if(IdleNum<(Total>>2) && RemovedNum>(Total>>3))//�հ׵�Ԫ��ĿС�������ķ�֮һ������ɾ����Ԫ����������˷�֮һ��������
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
		BuildTypeMap(Block);//����������������ڲ���
		return FALSE;
	}
}
#endif

//չʾ������Ϣ
void DebugInfoSave(INFO_BLOCK Block)
{
	u16 IdleNum=0,ValidNum=0,RemovedNum=0;
	INFO_MAP *pMap=NULL;
	u16 Unit;

	if(Block>=IBN_MAX) return;
	pMap=gBlockAttrib[Block].pMap;
	
	OS_EnterCritical();
	
	//չʾռ������ʣ�����ȵ�
	StatsInfoItemNum(Block,&IdleNum,&ValidNum,&RemovedNum);
	
	Debug("Block[%u] Idle:%4u, Vaild:%4u, Removed:%4u @ Sector %u:%u\n\r",Block,IdleNum,ValidNum,RemovedNum,GetBlockStartSec(Block),gBlockAttrib[Block].SectorNum);
	
	//չʾÿ������map
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

//�����洢�������Ƿ�Ҫ��ʽ������ȫ��flagȫ����Ϊ0xfe
//���ÿ���洢�鲻���Ͼ͸�ʽ��
//���洢�������������Ҫʱ����洢���������洢��typeӳ���
//ForceClean==TRUE�ָ���������
//ɾ�������ʱ���Զ�����
//�κ�ʱ�򶼿��Ե��ô˺������������洢����С
void InfoBuildBlock(INFO_BLOCK Block,bool ForceClean)
{
	if(Block>=IBN_MAX) return;

	OS_EnterCritical();
	
	if(ForceClean)//ǿ�ƻָ���������
	{
		FromatBlock(Block);//��������ʽ��
	}
	else //�Լ�
	{
		if(CheckBlockFat(Block)==TRUE)//��������£�Ӧ���Ǹ�ʽ��״̬(��Ч��)
		{
			TidyBlock(Block);//����洢��
		}
		else
		{
			FromatBlock(Block);//��������ʽ��
		}	
	}

	OS_ExitCritical();
}

//��ʼ��info���ݿ�
//ForceClean==TRUE�ָ���������
//ɾ�������ʱ���Զ�����
//�κ�ʱ�򶼿��Ե��ô˺��������������д洢����С
void InfoSaveInit(bool ForceClean)
{
	INFO_BLOCK Block;
	INFO_TYPE Type;
	
#if 1	//������
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

//ɾ��flash��ָ��λ�õ�info��Ϣ
//����ԭ���ľ��Դ洢λ��
//δ�ҵ�����0
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
	
	SetItemMapFlag(Block,Site,IFT_NOVALID,0);//�޸��������
	Addr=SiteToAddr(Block,Site);//�õ���ʵ��ַ

	Header.Flag=IBF_Removed;
	Header.Type=IFT_IDLE;
	Header.Resv=0xffff;
	SpiFlsWriteData(Addr,sizeof(Header),(void *)&Header);
	UpdateInfoFlag(Type);
	
	OS_ExitCritical();
	return Addr;
}


//�����info��Ϣ��flash�����ؾ��Դ洢λ��
//����INFO_RES_SPACE_FULL��ʾû�ռ�
INFO_ADDR SaveInfo(INFO_TYPE Type,void *pData)
{
	INFO_BLOCK Block;
	INFO_SITE Site;
	INFO_ADDR Addr;
	INFO_HEADER Header;
	INFO_ID InfoID;

	if(Type>=IFT_MAX) return INFO_PARAM_ERROR;
	
	if(pData==NULL) return INFO_PARAM_ERROR;
	else MemCpy(&InfoID,pData,sizeof(INFO_ID));//����id

	if(InfoID==0) return INFO_PARAM_ERROR;
	
	Block=gTypeAttrib[Type].Block;

	OS_EnterCritical();
	Site=FindItemByIdx(Block,IFT_IDLE,1);
	if(Site<0)
	{
		OS_ExitCritical();
		return INFO_SPACE_FULL;	//��Դ����
	}
	
	SetItemMapFlag(Block,Site,Type,InfoID);//�޸��������
	Addr=SiteToAddr(Block,Site);//�õ���ʵ��ַ

	Header.Flag=IBF_Vaild;
	Header.Type=Type;
	Header.Resv=0xffff;
	SpiFlsWriteData(Addr,sizeof(Header),(void *)&Header);
	if(pData!=NULL) SpiFlsWriteData(Addr+sizeof(Header),gTypeAttrib[Type].ItemBytes,pData);
	UpdateInfoFlag(Type);
	
	OS_ExitCritical();
	return Addr;
}

//������ͬInfoID��info��Ϣ��flash�����ؾ��Դ洢λ��
//�������ݶԱȺ�У��
//����flashֻ����1��0�����ԣ�������ݲ��ԣ����ܴ洢������ǳ���Ԥ��
INFO_ADDR CoverInfo(INFO_TYPE Type,void *pData)
{
	INFO_BLOCK Block;
	INFO_SITE Site;
	INFO_ADDR Addr;
	INFO_HEADER Header;
	INFO_ID InfoID;

	if(Type>=IFT_MAX) return INFO_PARAM_ERROR;
	
	if(pData==NULL) return INFO_PARAM_ERROR;
	else MemCpy(&InfoID,pData,sizeof(INFO_ID));//����id

	if(InfoID==0) return INFO_PARAM_ERROR;
		
	Block=gTypeAttrib[Type].Block;

	OS_EnterCritical();
	Site=FindItemByID(Block,Type,InfoID);
	if(Site<0) 
	{
		OS_ExitCritical();
		return INFO_PARAM_ERROR;
	}	
	Addr=SiteToAddr(Block,Site);//�õ���ʵ��ַ

	SpiFlsWriteData(Addr+sizeof(Header),gTypeAttrib[Type].ItemBytes,pData);
	UpdateInfoFlag(Type);

	OS_ExitCritical();
	return Addr;
}

//��ʵ�����ݵ�ͷ4�ֽ���ΪInfoID����ѯ�洢�飬ƥ��ʱ������Ϣ
//���ض�ȡ��С
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
	
	Addr=SiteToAddr(Block,Site);//�õ���ʵ��ַ

	if(pData!=NULL)
		SpiFlsReadData(Addr+sizeof(INFO_HEADER),gTypeAttrib[Type].ItemBytes,pData);

	OS_ExitCritical();
	return gTypeAttrib[Type].ItemBytes;
}

//��������˳���ȡinfo��Ϣ
//���ض�ȡ��С
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
	
	Addr=SiteToAddr(Block,Site);//�õ���ʵ��ַ

	if(pData!=NULL)
		SpiFlsReadData(Addr+sizeof(INFO_HEADER),gTypeAttrib[Type].ItemBytes,pData);

	OS_ExitCritical();
	return gTypeAttrib[Type].ItemBytes;
}

//��ȡ��Ϣ����
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

//��ȡ��Ϣ��С
u16 GetTypeItemSize(INFO_TYPE Type)
{
	if(Type>=IFT_MAX) return 0;
	
	return gTypeAttrib[Type].ItemBytes;
}

//��ȡ�հ׵�Ԫ��
u16 GetBlockFreeUnit(INFO_BLOCK Block)
{
	INFO_MAP *pMap;
	s32 i,Num=0;

	if(Block>=IBN_MAX) return 0;

	OS_EnterCritical();
	pMap=gBlockAttrib[Block].pMap;

	for(i=gBlockAttrib[Block].UnitTotal-1;i>=0;i--,Num++)//��ƨ����ǰ���հ׸񼴿�
	{
		if(pMap[i].Type!=IFT_IDLE) break;
	}	

	OS_ExitCritical();
	return Num;
}

//��ȡָ�����͵Ŀ�����Ŀ
u16 GetTypeFreeUnit(INFO_TYPE Type)
{
	return GetBlockFreeUnit(gTypeAttrib[Type].Block);
}

//��ȡָ�����͵Ŀ��С
u16 GetTypeBlockSize(INFO_TYPE Type)
{
	return 1<<(7+gTypeAttrib[Type].Block);
}

