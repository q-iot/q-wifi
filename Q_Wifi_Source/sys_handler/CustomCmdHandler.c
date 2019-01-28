#include "SysDefines.h"

bool CustomCmdHandler(char **pCmd,char *StrCopyBuf,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"#var")==0)
	{
		u16 Vid=0;
		u8 *pTags=pCmd[1];

		if(strlen(pTags)!=VAR_TAG_LEN*2)
		{
			UDebug("#rvar 1\r");
			return FALSE;
		}

		Vid=FindVarId_ByTag(pTags);
		if(Vid==0)
		{
			UDebug("#rvar 1\r");
			return FALSE;
		}

		if(IsNullStr(pCmd[2])) //read
		{	
			TVAR32 Val=0;
			VAR_STATE State=GetVarState(Vid,&Val);

			if(State==VST_VALID) sprintf(StrCopyBuf,"#rvar %s %d\r",pTags,Val);
			else sprintf(StrCopyBuf,"#rvar %s x\r",pTags);
			
			UDebug(StrCopyBuf);
			return TRUE;
		}
		else //set
		{
			TVAR32 Val=Str2Sint(pCmd[2]);
			bool Res=SetVarVal(Vid,Val,VRT_WDEV,0);//用户串口丢过来的，类似于设备自己改变

			if(Res) UDebug("#rvar 0\r");
			else UDebug("#rvar 1\r");
			return TRUE;
		}			
	}
	else if(strcmp((void *)pCmd[0],"#str")==0) 
	{
		u32 StrID=Str2Uint(pCmd[1]);
		
		if(IsNullStr(pCmd[1])||IsNullStr(pCmd[2]))
		{
			UDebug("#rstr 1\r");
			return FALSE;
		}
		
		//触发内部情景
		TrigIn_SysMsg(&StrCopyBuf[strlen(pCmd[1])+6]);

		//上报服务器及通知app
		JsonConnSendString(StrID,&StrCopyBuf[strlen(pCmd[1])+6]);
		StrChangeInform(0,StrID,&StrCopyBuf[strlen(pCmd[1])+6]);		

		UDebug("#rstr 0\r");
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"#msg")==0) //无返回
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
		TrigIn_SysMsg(&StrCopyBuf[strlen(pCmd[1])+6]);

		//发送到家庭板卡中
		if(ReadBit(FlagNum,SMF_SYS)) SetBit(MsgFlag,SMF_SYS);	
		if(ReadBit(FlagNum,SMF_GSM)) SetBit(MsgFlag,SMF_GSM);	
		if(ReadBit(FlagNum,SMF_PUSH)) SetBit(MsgFlag,SMF_PUSH);
		
		JsonConnSendMsg(&StrCopyBuf[strlen(pCmd[1])+6],"0",MsgFlag);
		SrvConnSendMsg(&StrCopyBuf[strlen(pCmd[1])+6],"0",MsgFlag);

		UDebug("#rmsg 0\r");
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"#sta")==0) 
	{
		struct ip_info ip_config;	
		
		wifi_get_ip_info(STATION_IF, &ip_config);

		if(ip_config.ip.addr == 0)
		{
			UDebug("#rsta rdy\r");
		}
		else
		{
			UDebug("#rsta con\r");
		}
		
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"#rst")==0)//无返回
	{
		UDebug("#rrst 0\r");
		RebootBoard();	
		return TRUE;
	}
	
	return FALSE;
}

