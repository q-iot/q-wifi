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

本文件是系统所有关键点的hook函数
*/
//---------------------------------------------------------------------------------------------//
#include "SysDefines.h"
#include "Q_Gui.h"

static u16 gVidSleepSec=0;
static u16 gVidRandMask=0;
static u16 gVidRandBit=0;
static u16 gVarRandNum=0;

#if 1//用户自定义的非标函数
//sleep变量被改变
void VarQsysSleepChange_Cb(int n,void *p)
{
	TVAR32 SleepSec=GetVarValue(gVidSleepSec,NULL);

	if(SleepSec && SleepSec<=1800)
	{
		system_deep_sleep(SleepSec*1000*1000);
	}	
}

//随机数变量被改变
void VarRandMaskChange_Cb(int n,void *p)
{
	TVAR32 MaskNum=GetVarValue(gVidRandMask,NULL);

	if(MaskNum)
	{
		u16 RandNum=Rand(0xffff)%MaskNum;
		
		if(MaskNum && MaskNum<=16)
		{
			u16 Bit=(1<<RandNum);
			SetVarVal(gVidRandBit,Bit,VRT_SYS,0);
		}
		else
		{
			SetVarVal(gVidRandBit,RandNum,VRT_SYS,0);
		}

		SetVarVal(gVarRandNum,RandNum,VRT_SYS,0);
		SetVarVal(gVidRandMask,0,VRT_SYS,0);
	}
}
#endif

//用户自定义硬件管脚的默认配置
//恢复出厂后，将按此配置
void UserHwDefConfigHook(HW_CONF_STRUCT *pHwConf)
{
	pHwConf->IoConfig[0].IoMode=WIM_KEY;//io0在硬件上已经连接到了key
	pHwConf->IoConfig[0].Pullup=TRUE;
	
	//pHwConf->IoConfig[13].IoMode=WIM_UART;//默认swap uart0到io13 io15
	//pHwConf->IoConfig[15].IoMode=WIM_UART;
	pHwConf->IoConfig[15].Pullup=FALSE;//硬件上限制了，io15必须下拉
}

//userhook强制设置每次上电初始化系统时的hw config
void UserHwConfigInitHook(u8 Pin,WE_IO_CONFIG *pIoConf)
{
	switch(Pin)
	{
		case 0://io0在硬件上已经连接到了key
			pIoConf->IoMode=WIM_KEY;
			pIoConf->Pullup=TRUE;
			break;		
	}
}

//硬件初始化
void UserHwInitHook(void)
{
	//UART_SetBaudrate(UART0,BIT_RATE_115200);//修改串口波特率

	if(SysVars()->SupportLCD)
	{
		GUI_REGION Region={0,0,LCD_WIDTH,LCD_HIGHT,0x00};
		
		Region.h=16;
		Gui_DrawFont(ASC14B_FONT,"    Welcome!",&Region);
		Region.y=16;
		Gui_DrawFont(GBK16_FONT,"    Q-IOT.CN",&Region);
		Debug("OLED Support!\n\r");
	}
	
	UDebug("####\r");
}

//初始化
void UserAppInitHook(void)
{
	UDebug("#rdy\r");

	gVidSleepSec=FindVarId_ByTag2("QSYS","SLEP");
	gVidRandMask=FindVarId_ByTag2("RAND","MASK");
	gVidRandBit=FindVarId_ByTag2("RAND","NBIT");
	gVarRandNum=FindVarId_ByTag2("RAND","NUMB");

	if(gVidSleepSec) NotifyAdd_Var2CbFunc(gVidSleepSec,VarQsysSleepChange_Cb,0);	
	if(gVidRandMask) NotifyAdd_Var2CbFunc(gVidRandMask,VarRandMaskChange_Cb,0);	
}

//已经连接上网
void UserConnHook(void)
{
	UDebug("#con\r");
}

void UserDisconnHook(void)
{
	UDebug("#dis\r");
}

//key io改变
void UserKeyStateHook(u8 Pin,u16 PressMs)
{
	Debug("Key[%u] %uMs\n\r",Pin,PressMs);

	if(Pin==0 && SysVars()->SupportLCD)
	{
		SysEventSend(SEN_OLED_DISP,0,NULL,NULL);//切换显示按键
	}

}

//io改变
void UserIoStateHook(u8 Pin,u8 PinVal)
{
	if(PinVal) Debug("P%u|\n\r",Pin);
	else Debug("P%u_\n\r",Pin);
}

//收包
void UserEvent1Hook(s32 S32Param,void *pParam)
{

}

void UserEvent2Hook(s32 S32Param,void *pParam)
{

}

void UserEvent3Hook(s32 S32Param,void *pParam)
{

}

void UserEvent4Hook(s32 S32Param,void *pParam)
{

}

//当app或主机系统里对本机变量进行操作时，变量值改变，会触发此函数
void UserVarChangeHook(u16 VarID,TVAR32 VarValue)
{
	char Tags[VAR_TAG_LEN*2+2]={0};
	
	if(GetVarProdTag2(VarID,Tags)==TRUE)
	{
		switch(GetVarDispFat(VarID,NULL))
		{
			case VDF_U16:
			case VDF_BIN:
			case VDF_HEX:
			case VDF_U32:
				UDebug("#var %s %u\r",Tags,VarValue);
				break;
			case VDF_FLOAT:
				UDebug("#var %s %f\r",Tags,VarValue);
				break;
			default:
				UDebug("#var %s %d\r",Tags,VarValue);
		}
		
		
	}
}

//当用户在手机app按下按键时，键值会传递过来，并触发此函数.ButtonID必须小于100
void UserAppButtonHook(u32 DevID,u16 ButtonID)
{
	UDebug("#key %u %u\r",DevID,ButtonID);
}

//当板卡收到系统消息时
void UserSysMsgHook(const u8 *pMsg)
{
	UDebug("#msg %s\r",pMsg);
}






#define CMD_BUF_LEN 128
static u8 CmdBuf[CMD_BUF_LEN+2];
static u16 CmdLen=0;
void UserUart0ByteInHook(u8 Byte)          
{
	bool NeedYield=FALSE;
	
	if(Byte=='\r')//回车
	{
		CmdBuf[CmdLen]=0;
		SysEventSend(SEN_SYS_CMD,CmdLen,(void *)CmdBuf,&NeedYield);
		CmdLen=0;
	}
	else if(Byte=='\b')//回删
	{
		if(CmdLen>0)
		{
			CmdBuf[--CmdLen]=0;
			CDebug("\b \b");//回退
		}
	}
	else
	{
		if(CmdLen<CMD_BUF_LEN) CmdBuf[CmdLen++]=Byte;
		CDebug("%c",Byte);//回显
	}	

	if(NeedYield)
	{
		OS_TaskYield();
	}
}

void UserUart0IdleHook(void)
{

}


