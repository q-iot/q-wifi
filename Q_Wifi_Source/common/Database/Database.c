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

���ļ�������һ������spi flash�����ݴ洢����
*/
//---------------------------------------------------------------------------------------------//
#include "SysDefines.h"
#include "SpiFlashApi.h"

#define DB_Debug Debug

typedef void (*DbInit_CB)(void);
typedef u32 (*GetValue_CB)(u16 Item,u32 IntParam,void *Val);
typedef bool (*SetValue_CB)(u16 Item,u32 IntParam,void *pParam,u16 ByteLen);
typedef void (*BurnDefault_CB)(void);

#define DBF_REMOVED 0x55550000//����
#define DBF_USED 0x55555555//ʹ����
#define DBF_IDLE 0x5555ffff//����

typedef struct{	
	u32 Flag;//��ȡ��־�������û���Ԥ
	u32 Ver;//�汾�������û���Ԥ
	u32 ChkSum;//У��ͣ������û���Ԥ
	
	u8 Data[4];//�û���������
}DB_STRUCT;	//���ݿ�

typedef struct{
	const u8 *pName;
	u16 StartSector;//��ʼ������
	u16 SectorNum;//ռ����������
	u16 OccupySize;//ÿ����Ԫռ�õ��ֽ��������������������ֳߴ�
	const DB_STRUCT *pDefaultData;//Ĭ��ֵ�洢�ռ�
	DB_STRUCT *pData;//��ǰֵ�洢�ռ�
	u32 DataBytes;//�洢�ṹ���С����ver check��ǰ�岿��
	DbInit_CB DbInitFunc;//���ݿ��ʼ���Ļص�
	GetValue_CB GetValueFunc;
	SetValue_CB SetValueFunc;
	BurnDefault_CB BurnDefaultFunc;//�ָ�Ĭ�Ϻ����ص�
}SUB_DB_STRUCT;

static u32 gNowDbAddr[SDN_MAX];//��ǰdb��Чλ��

#define GetStartAddr(n) (gSubDbs[n].StartSector*FLASH_SECTOR_BYTES)
//DB�汾:���̵İ汾��flash���֡����ݿ�Ĭ�����ݷ����ı�ʱ����ֵ��䡣
#define GetDbVer(n) (__gBinSoftVer+gSubDbs[n].StartSector+gSubDbs[n].SectorNum+gSubDbs[n].OccupySize  \
									+gSubDbs[n].DataBytes+MakeHash33((void *)gSubDbs[n].pDefaultData,gSubDbs[n].DataBytes))


#if 1//�û��Զ�������
extern const SYS_DB_STRUCT gDefSysDb;
extern SYS_DB_STRUCT gSysDb;
extern const QUICK_SAVE_STRUCT gDefQuickSave;
extern QUICK_SAVE_STRUCT gQuickSave;
extern const HW_CONF_STRUCT gDefHwConf;
extern HW_CONF_STRUCT gHwConf;

const SUB_DB_STRUCT gSubDbs[SDN_MAX]={//�洢λ�ö���
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
	
	//�������
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


static u32 QDB_NowUnitSec(SUB_DB_NAME Name)//��ȡ��ǰ����
{
	if(gNowDbAddr[Name]==0) gNowDbAddr[Name]=gSubDbs[Name].StartSector*FLASH_SECTOR_BYTES;

	return gNowDbAddr[Name]/FLASH_SECTOR_BYTES;
}

static u32 QDB_GetNextUnitSec(SUB_DB_NAME Name)//��ȡ��һ������
{
	u32 NowSec;
	
	if(gNowDbAddr[Name]==0) gNowDbAddr[Name]=gSubDbs[Name].StartSector*FLASH_SECTOR_BYTES;

	NowSec=gNowDbAddr[Name]/FLASH_SECTOR_BYTES;

	if(NowSec == (gSubDbs[Name].StartSector+gSubDbs[Name].SectorNum-1)) return gSubDbs[Name].StartSector;
	else return NowSec+1;	
}

//дĬ��ֵ��flash��ϵͳ
void QDB_BurnDefaultToSpiFlash(SUB_DB_NAME Name)
{
	DB_Debug("Now burn DB[%u] DEF to spi flash!\n\r",Name);

	OS_EnterCritical();
	
	SpiFlsEraseSector(QDB_NowUnitSec(Name),1);//����1���������
	gNowDbAddr[Name]=QDB_NowUnitSec(Name)*FLASH_SECTOR_BYTES;

	//��Ĭ��ֵд��ϵͳ��flash
	MemCpy((void *)gSubDbs[Name].pData,(void *)gSubDbs[Name].pDefaultData,gSubDbs[Name].DataBytes);//��Ĭ�����ݿ�����ϵͳ
	if(gSubDbs[Name].BurnDefaultFunc != NULL) gSubDbs[Name].BurnDefaultFunc();
	gSubDbs[Name].pData->Flag=DBF_USED;
	gSubDbs[Name].pData->Ver=GetDbVer(Name);
	gSubDbs[Name].pData->ChkSum=0;//�ȸ�0ֵ������Ӱ��У���
	gSubDbs[Name].pData->ChkSum=MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes);
	DB_Debug("Database chksum is %x %x-%x\n\r",gSubDbs[Name].pData->ChkSum,gSubDbs[Name].pData,gSubDbs[Name].DataBytes);
	SpiFlsWriteData(gNowDbAddr[Name],gSubDbs[Name].DataBytes,(void *)gSubDbs[Name].pData);//��Ĭ�����ݿ�����flash db����

	OS_ExitCritical();
}

//��������д��flash
void QDB_BurnToSpiFlash(SUB_DB_NAME Name)
{
	u32 OldChkSum;
	u32 DbFlag=DBF_REMOVED;
	u32 OldAddr=gNowDbAddr[Name];	

	OS_EnterCritical();

	OldChkSum=gSubDbs[Name].pData->ChkSum;
	gSubDbs[Name].pData->ChkSum=0;//�ȸ�0ֵ������Ӱ��У���
	gSubDbs[Name].pData->ChkSum=MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes);//��¼��У���
	if(gSubDbs[Name].pData->ChkSum==OldChkSum)//У���һ�������账��		
	{
		OS_ExitCritical();
		return;
	}
	
	if(gNowDbAddr[Name] == ((QDB_NowUnitSec(Name)+1)*FLASH_SECTOR_BYTES-gSubDbs[Name].OccupySize))//���������һҳ����Ҫд���¸�����
	{
		SpiFlsEraseSector(QDB_GetNextUnitSec(Name),1);//�����¸�����
		gNowDbAddr[Name]=QDB_GetNextUnitSec(Name)*FLASH_SECTOR_BYTES;		
	}
	else //�Ǳ��������һҳ��ֱ��д��
	{
		gNowDbAddr[Name]+=(gSubDbs[Name].OccupySize);
	}
	
	gSubDbs[Name].pData->Flag=DBF_USED;
	gSubDbs[Name].pData->Ver=GetDbVer(Name);
	gSubDbs[Name].pData->ChkSum=0;//�ȸ�0ֵ������Ӱ��У���
	gSubDbs[Name].pData->ChkSum=MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes);		
	//DB_Debug("Database Chk:%x,P:%x,Len:%u\n\r",gSubDbs[Name].pData->ChkSum,gSubDbs[Name].pData,gSubDbs[Name].DataBytes);
	SpiFlsWriteData(gNowDbAddr[Name],gSubDbs[Name].DataBytes,(void *)gSubDbs[Name].pData);//�����ݿ�����flash db����
	//Debug("Burn At: %u.%u\n\r",gNowDbAddr[Name]/FLASH_SECTOR_BYTES,(gNowDbAddr[Name]%FLASH_SECTOR_BYTES)/FLASH_PAGE_SIZE);

	SpiFlsWriteData(OldAddr,sizeof(u32),(void *)&DbFlag);//������־
	//Debug("Set Remove Flag At: %u.%u\n\r",OldAddr/FLASH_SECTOR_BYTES,(OldAddr%FLASH_SECTOR_BYTES)/FLASH_PAGE_SIZE);
		
	OS_ExitCritical();
}

//�Ӵ洢��������ݿ⵽�ڴ棬ͨ��ָ�뷵�ص�ǰ�洢ҳ
static void QDB_ReadFromSpiFlash(SUB_DB_NAME Name)
{
	u32 Idx;
	u32 DbFlag;
	u32 ChkSum;

	OS_EnterCritical();
	
	for(Idx=0;Idx<(gSubDbs[Name].SectorNum*FLASH_SECTOR_BYTES/gSubDbs[Name].OccupySize);Idx++) //�����洢��
	{
		gNowDbAddr[Name]=GetStartAddr(Name)+Idx*gSubDbs[Name].OccupySize;		
		SpiFlsReadData(gNowDbAddr[Name],sizeof(u32),(void *)&DbFlag);//����־λ
		
		if(DbFlag == DBF_USED)//������ȷ����
		{
			Debug("Read DB[%u] Idx:%u, At:%u.%u\n\r",Name,Idx,gNowDbAddr[Name]/FLASH_SECTOR_BYTES,(gNowDbAddr[Name]%FLASH_SECTOR_BYTES)/FLASH_PAGE_SIZE);
			SpiFlsReadData(gNowDbAddr[Name],gSubDbs[Name].DataBytes,(void *)gSubDbs[Name].pData);

			if(gSubDbs[Name].pData->Ver!=GetDbVer(Name))//���汾�Ƿ���ȷ
			{
				DB_Debug("Database[%u] version 0x%x is not right(!=0x%x)!\n\rBurn All DB DEF to flash\n\r",Name,gSubDbs[Name].pData->Ver,GetDbVer(Name));
				for(Name=(SUB_DB_NAME)0;Name<SDN_MAX;Name++) QDB_BurnDefaultToSpiFlash(Name);
				OS_ExitCritical();
				return;
			}

			ChkSum=gSubDbs[Name].pData->ChkSum;
			gSubDbs[Name].pData->ChkSum=0;//�ȸ�0ֵ������Ӱ��У���
			if(ChkSum!= MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes))//���У����Ƿ���ȷ
			{
				DB_Debug("Database[%u] chksum is not right!(%x != %x) %x-%x\n\rsys halt!\n\r",Name,ChkSum, MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes),gSubDbs[Name].pData->Data,gSubDbs[Name].DataBytes);
				QDB_BurnDefaultToSpiFlash(Name);//У��Ͳ���˵�����ݿⱻ�ٻ������ؽ�
			}
			else
			{
				gSubDbs[Name].pData->ChkSum=ChkSum;//��ԭУ���
			}

			OS_ExitCritical();
			return;
		}
	}

	OS_ExitCritical();
	
	DB_Debug("Not read database for DB[%d]!\n\rBurn default database!\n\r",Name);
	QDB_BurnDefaultToSpiFlash(Name);//û�������ݿ⣬ֱ���ؽ�
}

//��ʼ������ ��������
// 1.��flash��ȡ���ݿ�����
// 2.���û�ж������ݿ⣬��Ĭ��ֵ���ݿ���д��flash
// 3.��������ݿ⣬���ȡ���ݿ⵽����
void QDB_Init(void)
{	
	SUB_DB_NAME Name;
	
	Debug("Database Init\n\r");

	//�������
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

//�����ݿ⻺���ȡֵ��ֻ��ʹ��pDB_Setting��ȡ
u32 QDB_GetValue(SUB_DB_NAME Name,u16 Item,u32 IntParam,void *Val)
{
	if(gSubDbs[Name].GetValueFunc != NULL)	
		return gSubDbs[Name].GetValueFunc(Item,IntParam,Val);

	return 0;
}

//дֵ�����ݿ⻺�棬ֻ��ʹ��pDB_Setting�洢
bool QDB_SetValue(SUB_DB_NAME Name,u16 Item,u32 IntParam,void *pParam,u16 ByteLen)
{
	if(gSubDbs[Name].SetValueFunc != NULL)
		return gSubDbs[Name].SetValueFunc(Item,IntParam,pParam,ByteLen);	

	return FALSE;
}

//��ȡ��ǰ�����������У��ͣ����������ݸı�ʱ����ֵ���
u32 QDB_GetNowChkSum(SUB_DB_NAME Name)
{
	u32 ChkSum=gSubDbs[Name].pData->ChkSum;
	u32 Ret;
	
	gSubDbs[Name].pData->ChkSum=0;//�ȸ�0ֵ������Ӱ��У���
	Ret=MakeHash33((void *)gSubDbs[Name].pData,gSubDbs[Name].DataBytes);		
	gSubDbs[Name].pData->ChkSum=ChkSum;
	
	return Ret;
}

