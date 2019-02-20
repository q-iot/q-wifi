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

