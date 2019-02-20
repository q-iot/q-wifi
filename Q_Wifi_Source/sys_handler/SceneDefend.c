#include "SysDefines.h"

void ScnDefendDebug(void)
{
	ID_T *pList=Q_Zalloc(sizeof(ID_T)*SCENE_MAX_NUM);
	u16 ScnNum;
	u16 i;

	ScnNum=QDB_GetValue(SDN_QCK,QIN_SCN_FLAG,0,pList);
	for(i=0;i<ScnNum;i++)
	{
		Debug("[%u]\n\r",pList[i]);
	}

	Q_Free(pList);
}

//���ؼ�����龰����
u16 GetScnDefendList(ID_T *pListOut)
{
	ID_T *pList;
	u16 ScnNum;
	u16 i;

	ScnNum=QDB_GetValue(SDN_QCK,QIN_SCN_FLAG,0,pListOut);

	return ScnNum;
}

//�����龰
void SetScnDefendToList(ID_T *pIDs,u16 Num)
{
	ID_T *pList;
	u16 ScnNum;
	u16 i,n;

	if(pIDs==NULL || Num > SCENE_MAX_NUM) return;	

	for(i=0;i<Num;i++)//ѭ������
	{
		if(pIDs[i]==0) continue;
		TrigEnable_ByScn(pIDs[i]);//������Ӧ����Դ
		QDB_SetValue(SDN_QCK,QIN_SCN_SET,pIDs[i],NULL,0);
	}
}

//�����龰
void ClrScnDefendToList(ID_T *pIDs,u16 Num)
{
	ID_T *pList;
	u16 ScnNum;
	u16 i,n;

	if(pIDs==NULL || Num > SCENE_MAX_NUM) return;

	for(i=0;i<Num;i++)//ѭ������
	{
		if(pIDs[i]==0) continue;
		TrigDisable_ByScn(pIDs[i]);//ʧЧ��Ӧ����Դ
		QDB_SetValue(SDN_QCK,QIN_SCN_CLR,pIDs[i],NULL,0);
	}
}

//�龰ģʽ���ڼ���̬
bool ScnIsActive(ID_T ScnID)
{	
	return QDB_GetValue(SDN_QCK,QIN_SCN_FLAG,ScnID,NULL)?TRUE:FALSE;
}

//ֻ������Ч���龰ģʽ
bool ScnDefendListTidy(void)
{
	u16 ScnNum=QDB_GetValue(SDN_QCK,QIN_SCN_FLAG,0,NULL);
	ID_T *pScnID=Q_ZallocAsyn(ScnNum*sizeof(ID_T));
	u16 i;
	
	QDB_GetValue(SDN_QCK,QIN_SCN_FLAG,0,pScnID);

	for(i=0;i<ScnNum;i++)
	{
		if(ReadInfoByID(IFT_SCENE,pScnID[i],NULL))
		{
			pScnID[i]=0;
		}
	}

	ClrScnDefendToList(pScnID,ScnNum);	
}


