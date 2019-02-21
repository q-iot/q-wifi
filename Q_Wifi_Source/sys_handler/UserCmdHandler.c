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

本文件定义了与第三方系统的#命令的交互
*/
//---------------------------------------------------------------------------------------------//
#include "SysDefines.h"

bool UserCmdHandler(char **pCmd,const char *StrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"#var")==0)
	{
		u16 Vid=0;
		u8 *pTags=pCmd[1];

		if(strlen(pTags)!=VAR_TAG_LEN*2)
		{
			UDebug("#rvar 1 %s\r",pTags);
			return FALSE;
		}

		Vid=FindVarId_ByTag(pTags);
		if(Vid==0)
		{
			UDebug("#rvar 1 %s\r",pTags);
			return FALSE;
		}

		if(IsNullStr(pCmd[2])) //read var
		{	
			TVAR32 Val=0;
			VAR_STATE State=GetVarState(Vid,&Val);
			VAR_DISP_FAT Fat=GetVarDispFat(Vid,NULL);
			char *p=Q_Zalloc(32);
			
			if(State==VST_VALID) 
			{
				switch(Fat)
				{
					case VDF_U16:
					case VDF_BIN:
					case VDF_HEX:
					case VDF_U32:
						sprintf(p,"#rvar 0 %s %u\r",pTags,Val);
						break;
					case VDF_FLOAT:
						sprintf(p,"#rvar 0 %s %f\r",pTags,Val);
						break;
					default:
						sprintf(p,"#rvar 0 %s %d\r",pTags,Val);
				}				
			}
			else
			{
				sprintf(p,"#rvar 1 %s x\r",pTags);
			}
			
			UDebug(p);
			Q_Free(p);
			
			return TRUE;
		}
		else //set var
		{
			TVAR32 Val=Str2Sint(pCmd[2]);
			bool Res=SetVarVal(Vid,Val,VRT_WDEV,0);//用户串口丢过来的，类似于设备自己改变

			if(Res) UDebug("#rvar 0 %s\r",pTags);
			else UDebug("#rvar 1 %s\r",pTags);
			
			return TRUE;
		}			
	}
	else if(strcmp((void *)pCmd[0],"#str")==0) //发送界面提示字符串
	{
		u32 StrID=Str2Uint(pCmd[1]);
		
		if(IsNullStr(pCmd[1])||IsNullStr(pCmd[2]))
		{
			UDebug("#rstr 1\r");
			return FALSE;
		}
		
		//触发内部情景
		TrigIn_SysMsg(&StrCopy[strlen(pCmd[1])+6]);

		//上报服务器及通知app
		JsonConnSendString(StrID,&StrCopy[strlen(pCmd[1])+6]);
		StrChangeInform(0,StrID,&StrCopy[strlen(pCmd[1])+6]);		

		UDebug("#rstr 0\r");
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"#msg")==0) //发送系统消息
	{
		u32 FlagNum=0;
		u8 MsgFlag=0;

		if(IsNullStr(pCmd[1])||IsNullStr(pCmd[2])) 
		{
			UDebug("#rmsg 1\r");
			return FALSE;
		}
		
		FlagNum=Str2Uint(pCmd[1]);

		//触发内部情景
		TrigIn_SysMsg(&StrCopy[strlen(pCmd[1])+6]);

		//发送到家庭板卡中
		if(ReadBit(FlagNum,SMF_SYS)) SetBit(MsgFlag,SMF_SYS);	
		if(ReadBit(FlagNum,SMF_GSM)) SetBit(MsgFlag,SMF_GSM);	
		if(ReadBit(FlagNum,SMF_PUSH)) SetBit(MsgFlag,SMF_PUSH);
		
		JsonConnSendMsg(&StrCopy[strlen(pCmd[1])+6],"0",MsgFlag);
		SrvConnSendMsg(&StrCopy[strlen(pCmd[1])+6],"0",MsgFlag);

		UDebug("#rmsg 0\r");
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"#sta")==0) 
	{
		struct ip_info ip_config;	
		
		wifi_get_ip_info(STATION_IF, &ip_config);

		if(ip_config.ip.addr == 0)
		{
			UDebug("#rsta 0 rdy\r");
		}
		else
		{
			UDebug("#rsta 0 con\r");
		}
		
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"#rst")==0)
	{
		UDebug("#rrst 0\r");
		RebootBoard();	
		return TRUE;
	}
	
	return FALSE;
}

