#ifndef QSYS_DATABASE_H
#define QSYS_DATABASE_H

typedef enum{
	SDN_SYS=0,//id,ip��ַ,ϵͳ����
	SDN_QCK,//�����ȡ��һЩ���ݣ�ģ������洢��
	SDN_HWC,//Ӳ�����ñ�
	
	SDN_MAX
}SUB_DB_NAME;

void QDB_Debug(void);
void QDB_Init(void);
void QDB_BurnDefaultToSpiFlash(SUB_DB_NAME Name);
void QDB_BurnToSpiFlash(SUB_DB_NAME Name);
u32 QDB_GetValue(SUB_DB_NAME Name,u16 Item,u32 IntParam,void *Val);
bool QDB_SetValue(SUB_DB_NAME Name,u16 Item,u32 IntParam,void *pParam,u16 ByteLen);
u32 QDB_GetNowChkSum(SUB_DB_NAME Name);

#define QDB_GetNum(Name,Item) QDB_GetValue(Name,Item,0,NULL)//��ȡ�����������ϵͳ����
#define QDB_GetIp(Name,Item,pIpAddrBuf) QDB_GetValue(Name,Item,0,pIpAddrBuf)//��ȡ����ip���ϵͳ����
#define QDB_GetStr(Name,Item,pStr) QDB_GetValue(Name,Item,0,pStr)//��ȡ�����ַ������ϵͳ�������ַ�����������㹻�������ȴӷ���ֵ����

#define QDB_SetNum(Name,Item,Num) QDB_SetValue(Name,Item,Num,NULL,0)//���������������ϵͳ����
#define QDB_SetIp(Name,Item,pIpAddrBuf) QDB_SetValue(Name,Item,0,pIpAddrBuf,4)//��������ip���ϵͳ����
#define QDB_SetStr(Name,Item,pStr) QDB_SetValue(Name,Item,0,pStr,strlen(pStr))//���������ַ�����ϵͳ����

#endif

