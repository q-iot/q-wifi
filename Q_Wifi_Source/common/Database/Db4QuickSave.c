#include "SysDefines.h"

const QUICK_SAVE_STRUCT gDefQuickSave={
0,0,0,//无需用户干预的版本及标志信息

//用户数据
0,
0,
{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},


};	
	
QUICK_SAVE_STRUCT gQuickSave={0,0,0};//

//系统初始化时调用
//运行于系统读出数据库后，用户程序之前
//用于设置一些无法保存到默认配置的默认配置
void QSS_DbInit(void)
{
	if(gQuickSave.ObjIdBase<100) gQuickSave.ObjIdBase=100;
}

//运行于系统即将要烧默认数据库到flash之前
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
			if(IntParam)//查询对应的情景是否在列表中
			{
				u16 n;
				for(n=0;n<gQuickSave.ScnNum;n++)
				{
					if(IntParam==gQuickSave.ScnDefend[n])//找到了
					{
						return 1;
					}
				}
				return 0;
			}
			else
			{
				if(pVal!=NULL)//拷贝整个数组出去
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
			if(++gQuickSave.ObjIdBase>0x7fff) gQuickSave.ObjIdBase=100;//不能为0，跳过s16的负数
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
					if(IntParam==pList[n])//已经设置
					{
						break;
					}
				}

				if(n==ScnNum)//尚未设置
				{
					if(ScnNum < SCENE_MAX_NUM)//未满
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
					if(IntParam==pList[n])//找到了
					{
						if(n==(ScnNum-1)) //找到的就是最后一个
						{
							pList[n]=0;//删除
						}
						else //找到的是中间的
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

