#include "SysDefines.h"
#include "VarDisplay.h"
#include "Q_Gui.h"

#define OLED_DISPLAY_LAST_NUM 2
static u8 DispIndex=0;//记录显示序号

//变量改变后的回调函数
void VarDisplay_Cb(int n,void *p)//n传入屏显变量序号，从0开始
{
	GUI_REGION Region={0,0,LCD_WIDTH,LCD_HIGHT,0x00};
	u8 *pName=Q_Malloc(64);
	u8 *pLine2=Q_Malloc(64);
	u16 Vid;
	TVAR32 Value;
	VAR_STATE State;
	VAR_DISP_FAT DispFat;
	u8 DotNum;
	u8 i;
	u8 Total=0;

	if(p==NULL)
	{
		DispIndex=OLED_DISPLAY_LAST_NUM+1;
		for(i=0;i<F_LCD_VAR_DISP_NUM;i++)
		{
			if(QDB_GetValue(SDN_SYS,SIN_LcdVars,i,NULL))
			{
				Total++;
				if(i<=n) DispIndex++;
			}
		}
		if(DispIndex > OLED_DISPLAY_LAST_NUM+Total) DispIndex=0;
	}

	
	Vid=QDB_GetValue(SDN_SYS,SIN_LcdVars,n,NULL);
	Value=GetVarValue(Vid,&State);
	QDB_GetValue(SDN_SYS,SIN_LcdVarsName,n,pName);
	//Debug("[OLED]%s:%u\r\n",pName,Value);

	if(State==VST_VALID)
	{
		DispFat=GetVarDispFat(Vid,&DotNum);
		VarStrOut(Value,DispFat,DotNum,pLine2);//格式化输出
	}
	else
	{
		sprintf(pLine2,"---");
	}
	
	//display
	Gui_Fill(&Region,FALSE);	
	Region.h=16;
	Gui_DrawFont(GBK16_FONT,pName,&Region);
	Region.y=16;
	Gui_DrawFont(GBK16_FONT,pLine2,&Region);
	
	Q_Free(pName);
	Q_Free(pLine2);
}

void OledDisplayLoop(void)
{
	GUI_REGION Region={0,0,LCD_WIDTH,LCD_HIGHT,0x00};
	u8 *pLine1=Q_Malloc(64);
	u8 *pLine2=Q_Malloc(64);

Handle:
	switch(DispIndex)
	{
		case 0:		
			sprintf(pLine1,"AP:");
			sprintf(pLine2,"Q-IOT-%u",GetHwID());
			break;
		case 1:
			{
				struct station_config *pConfig=Q_Malloc(sizeof(struct station_config));
				wifi_station_get_config_default(pConfig);								
				sprintf(pLine1,"Link SSID:");
				sprintf(pLine2,"%s",pConfig->ssid);
				Q_Free(pConfig);
			}
			break;
		case OLED_DISPLAY_LAST_NUM:
			{
				struct ip_info ip_config;	
				wifi_get_ip_info(STATION_IF, &ip_config);
				sprintf(pLine1,"Client IP:");
				sprintf(pLine2,IPSTR,IP2STR(&(ip_config.ip.addr)));
			}
			break;
		default://显示变量
			{
				u8 i,n,Total=0;
				for(i=0;i<F_LCD_VAR_DISP_NUM;i++)
				{
					if(QDB_GetValue(SDN_SYS,SIN_LcdVars,i,NULL))
					{
						Total++;
					}
				}

				if(DispIndex-OLED_DISPLAY_LAST_NUM>Total)//越界了
				{
					DispIndex=0;
					goto Handle;
				}
				else
				{
					for(n=0,i=0;i<F_LCD_VAR_DISP_NUM;i++)
					{
						if(QDB_GetValue(SDN_SYS,SIN_LcdVars,i,NULL))
						{
							n++;
							if((DispIndex-OLED_DISPLAY_LAST_NUM)==n)
							{
								VarDisplay_Cb(i,(void *)1);	
								if(n==Total) DispIndex=0;
								else DispIndex++;
								return;
							}
						}
					}
				}				
			}
	}

	//display
	Gui_Fill(&Region,FALSE);
	Region.h=16;
	Gui_DrawFont(GBK16_FONT,pLine1,&Region);
	Region.y=16;
	Gui_DrawFont(GBK16_FONT,pLine2,&Region);

	Q_Free(pLine1);
	Q_Free(pLine2);
	if(DispIndex<=OLED_DISPLAY_LAST_NUM) DispIndex++;
}

//初始化屏显变量的订阅关系
void VarDisplay_Init(void)
{
	if(SysVars()->SupportLCD)
	{
		u16 Vid,n=0;

		for(n=0;n<F_LCD_VAR_DISP_NUM;n++)
		{
			Vid=QDB_GetValue(SDN_SYS,SIN_LcdVars,n,NULL);
			if(Vid) NotifyAdd_Var2CbFunc(Vid,VarDisplay_Cb,n);
		}	
	}
}



