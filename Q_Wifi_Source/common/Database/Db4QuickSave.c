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

���ļ��ǻ���Database.c����Ļ��Ƶ�ʵ�ʴ洢����
*/
//---------------------------------------------------------------------------------------------//
#include "SysDefines.h"

const QUICK_SAVE_STRUCT gDefQuickSave={
0,0,0,//�����û���Ԥ�İ汾����־��Ϣ

//�û�����
0,
0,
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},


};	
	
QUICK_SAVE_STRUCT gQuickSave={0,0,0};//

//ϵͳ��ʼ��ʱ����
//������ϵͳ�������ݿ���û�����֮ǰ
//��������һЩ�޷����浽Ĭ�����õ�Ĭ������
void QSS_DbInit(void)
{
	if(gQuickSave.ObjIdBase<100) gQuickSave.ObjIdBase=100;
}

//������ϵͳ����Ҫ��Ĭ�����ݿ⵽flash֮ǰ
void QSS_Default(void)
{
	u16 i;
	
	gQuickSave.ObjIdBase=Rand(0x7fff);
	if(gQuickSave.ObjIdBase<100) gQuickSave.ObjIdBase+=100;

	for(i=0;i<IFT_SAVE_NUM;i++)
	{
		gQuickSave.InfoFlag[i]=Rand(0xffffffff);
	}	
}

u32 QSS_GetValue(u16 Item,u32 IntParam,void *pVal)
{
	switch(Item)
	{
		case QIN_OBJ_ID_BASE:
			return gQuickSave.ObjIdBase;
		case QIN_DEBUG_FLAG:
			if(IntParam<DFT_MAX) return ReadBit(gQuickSave.DebugFlag,IntParam);
			else return gQuickSave.DebugFlag;
		case QIN_INFO_FLAG:
			return gQuickSave.InfoFlag[IntParam];
		case QIN_SCN_FLAG:
			if(IntParam)//��ѯ��Ӧ���龰�Ƿ����б���
			{
				u16 n;
				for(n=0;n<gQuickSave.ScnNum;n++)
				{
					if(IntParam==gQuickSave.ScnDefend[n])//�ҵ���
					{
						return 1;
					}
				}
				return 0;
			}
			else
			{
				if(pVal!=NULL)//�������������ȥ
				{
					MemCpy(pVal,gQuickSave.ScnDefend,gQuickSave.ScnNum*sizeof(u32));
				}			

				return gQuickSave.ScnNum;
			}
	}

	return 0;
}

bool QSS_SetValue(u16 Item,u32 IntParam,void *pParam,u16 ByteLen)
{
	switch(Item)
	{
		case QIN_OBJ_ID_BASE:
			if(++gQuickSave.ObjIdBase>0x7fff) gQuickSave.ObjIdBase=100;//����Ϊ0������s16�ĸ���
			break;
		case QIN_DEBUG_FLAG:
			if(pParam!=NULL)
			{
				if(IntParam<DFT_MAX) SetBit(gQuickSave.DebugFlag,IntParam);
				else gQuickSave.DebugFlag=0xffffffff;
			}
			else
			{
				if(IntParam<DFT_MAX) ClrBit(gQuickSave.DebugFlag,IntParam);
				else gQuickSave.DebugFlag=0;
			}
			break;
		case QIN_INFO_FLAG:
			gQuickSave.InfoFlag[IntParam]+=1;
			break;
		case QIN_SCN_SET:
			if(IntParam)
			{
				u16 n,ScnNum=gQuickSave.ScnNum;
				u32 *pList=gQuickSave.ScnDefend;
				
				for(n=0;n<ScnNum;n++)
				{
					if(IntParam==pList[n])//�Ѿ�����
					{
						break;
					}
				}

				if(n==ScnNum)//��δ����
				{
					if(ScnNum < SCENE_MAX_NUM)//δ��
					{
						pList[ScnNum]=IntParam;
						gQuickSave.ScnNum++;
						ScnNum++;
					}
				}
			}
			break;
		case QIN_SCN_CLR:
			if(IntParam)
			{
				u16 n,ScnNum=gQuickSave.ScnNum;
				u32 *pList=gQuickSave.ScnDefend;

				for(n=0;n<ScnNum;n++)
				{
					if(IntParam==pList[n])//�ҵ���
					{
						if(n==(ScnNum-1)) //�ҵ��ľ������һ��
						{
							pList[n]=0;//ɾ��
						}
						else //�ҵ������м��
						{
							pList[n]=pList[ScnNum-1];
							pList[ScnNum-1]=0;
						}
						gQuickSave.ScnNum--;
						ScnNum--;
						
						break;
					}
				}
			}
			break;
			
		default:
			return FALSE;
	}

	SysEventMsSend(3*1000,SEN_QCK_SAVE,0,NULL,MFM_COVER);
	
	return TRUE;
}

