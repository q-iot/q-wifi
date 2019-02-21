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

本文件是基于Database.c定义的机制的实际存储内容
*/
//---------------------------------------------------------------------------------------------//
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

