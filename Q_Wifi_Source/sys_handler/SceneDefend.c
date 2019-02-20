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

//返回激活的情景个数
u16 GetScnDefendList(ID_T *pListOut)
{
	ID_T *pList;
	u16 ScnNum;
	u16 i;

	ScnNum=QDB_GetValue(SDN_QCK,QIN_SCN_FLAG,0,pListOut);

	return ScnNum;
}

//激活情景
void SetScnDefendToList(ID_T *pIDs,u16 Num)
{
	ID_T *pList;
	u16 ScnNum;
	u16 i,n;

	if(pIDs==NULL || Num > SCENE_MAX_NUM) return;	

	for(i=0;i<Num;i++)//循环设置
	{
		if(pIDs[i]==0) continue;
		TrigEnable_ByScn(pIDs[i]);//激活相应触发源
		QDB_SetValue(SDN_QCK,QIN_SCN_SET,pIDs[i],NULL,0);
	}
}

//屏蔽情景
void ClrScnDefendToList(ID_T *pIDs,u16 Num)
{
	ID_T *pList;
	u16 ScnNum;
	u16 i,n;

	if(pIDs==NULL || Num > SCENE_MAX_NUM) return;

	for(i=0;i<Num;i++)//循环设置
	{
		if(pIDs[i]==0) continue;
		TrigDisable_ByScn(pIDs[i]);//失效相应触发源
		QDB_SetValue(SDN_QCK,QIN_SCN_CLR,pIDs[i],NULL,0);
	}
}

//情景模式处在激活态
bool ScnIsActive(ID_T ScnID)
{	
	return QDB_GetValue(SDN_QCK,QIN_SCN_FLAG,ScnID,NULL)?TRUE:FALSE;
}

//只保留有效的情景模式
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


