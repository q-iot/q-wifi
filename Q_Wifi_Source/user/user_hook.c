#include "SysDefines.h"
#include "Q_Gui.h"

static u16 gVidSleepSec=0;
static u16 gVidRandMask=0;
static u16 gVidRandBit=0;
static u16 gVarRandNum=0;

#if 1//�û��Զ���ķǱ꺯��
//sleep�������ı�
void VarQsysSleepChange_Cb(int n,void *p)
{
	TVAR32 SleepSec=GetVarValue(gVidSleepSec,NULL);

	if(SleepSec && SleepSec<=1800)
	{
		system_deep_sleep(SleepSec*1000*1000);
	}	
}

//������������ı�
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

//�û��Զ���Ӳ���ܽŵ�Ĭ������
//�ָ������󣬽���������
void UserHwDefConfigHook(HW_CONF_STRUCT *pHwConf)
{
	pHwConf->IoConfig[0].IoMode=WIM_KEY;//io0��Ӳ�����Ѿ����ӵ���key
	pHwConf->IoConfig[0].Pullup=TRUE;
	
	//pHwConf->IoConfig[13].IoMode=WIM_UART;//Ĭ��swap uart0��io13 io15
	//pHwConf->IoConfig[15].IoMode=WIM_UART;
	pHwConf->IoConfig[15].Pullup=FALSE;//Ӳ���������ˣ�io15��������
}

//userhookǿ������ÿ���ϵ��ʼ��ϵͳʱ��hw config
void UserHwConfigInitHook(u8 Pin,WE_IO_CONFIG *pIoConf)
{
	switch(Pin)
	{
		case 0://io0��Ӳ�����Ѿ����ӵ���key
			pIoConf->IoMode=WIM_KEY;
			pIoConf->Pullup=TRUE;
			break;		
	}
}

//Ӳ����ʼ��
void UserHwInitHook(void)
{
	//UART_SetBaudrate(UART0,BIT_RATE_115200);//�޸Ĵ��ڲ�����

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

//��ʼ��
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

//�Ѿ���������
void UserConnHook(void)
{
	UDebug("#con\r");
}

void UserDisconnHook(void)
{
	UDebug("#dis\r");
}

//key io�ı�
void UserKeyStateHook(u8 Pin,u16 PressMs)
{
	Debug("Key[%u] %uMs\n\r",Pin,PressMs);

	if(Pin==0 && SysVars()->SupportLCD)
	{
		SysEventSend(SEN_OLED_DISP,0,NULL,NULL);//�л���ʾ����
	}

}

//io�ı�
void UserIoStateHook(u8 Pin,u8 PinVal)
{
	if(PinVal) Debug("P%u|\n\r",Pin);
	else Debug("P%u_\n\r",Pin);
}

//�հ�
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

//��app������ϵͳ��Ա����������в���ʱ������ֵ�ı䣬�ᴥ���˺���
void UserVarChangeHook(u16 VarID,TVAR32 VarValue)
{
	char Tags[VAR_TAG_LEN*2+2]={0};
	
	if(GetVarProdTag2(VarID,Tags)==TRUE)
	{
		UDebug("#var %s %d\r",Tags,VarValue);
	}
}

//���û����ֻ�app���°���ʱ����ֵ�ᴫ�ݹ������������˺���.ButtonID����С��100
void UserAppButtonHook(u32 DevID,u16 ButtonID)
{
	UDebug("#key %u %u\r",DevID,ButtonID);
}

//���忨�յ�ϵͳ��Ϣʱ
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
	
	if(Byte=='\r')//�س�
	{
		CmdBuf[CmdLen]=0;
		SysEventSend(SEN_SYS_CMD,CmdLen,(void *)CmdBuf,&NeedYield);
		CmdLen=0;
	}
	else if(Byte=='\b')//��ɾ
	{
		if(CmdLen>0)
		{
			CmdBuf[--CmdLen]=0;
			CDebug("\b \b");//����
		}
	}
	else
	{
		if(CmdLen<CMD_BUF_LEN) CmdBuf[CmdLen++]=Byte;
		CDebug("%c",Byte);//����
	}	

	if(NeedYield)
	{
		OS_TaskYield();
	}
}

void UserUart0IdleHook(void)
{

}


