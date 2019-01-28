#include "SysDefines.h"
#include "VarsJsonUpTask.h"
#include "Q_List.h"

static bool __inline SysCmdHandler_A(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"ap")==0)
	{
		if(NotNullStr(pCmd[1]) && NotNullStr(pCmd[2]))
		{
			struct softap_config *ap_config = (struct softap_config *)Q_Malloc(sizeof(struct softap_config)); // initialization
			wifi_softap_get_config(ap_config); // Get soft-AP ap_config first.
			sprintf(ap_config->ssid, pCmd[1]);
			sprintf(ap_config->password, pCmd[2]);
			ap_config->authmode = AUTH_WPA_WPA2_PSK;
			ap_config->ssid_len = 0; // or its actual SSID length
			ap_config->max_connection = 4;
			wifi_softap_set_config(ap_config); // Set ESP8266 soft-AP ap_config
			Q_Free(ap_config);
			CDebug("Set ap ssid to %s:%s\n\r",pCmd[1],pCmd[2]);
		}
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"apid")==0)
	{
		if(IsNullStr(pCmd[1]))
		{
			CDebug("Current ap id:%u\n\r",wifi_station_get_current_ap_id());
		}
		else
		{
			u16 ApId=Str2Uint(pCmd[1]);

			wifi_station_ap_change(ApId);
			CDebug("Now change wifi station ap id to %u\n\r",ApId);
		}
		
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"adc")==0)
	{
		Debug("ADC=%umV\n\r",(EspHwAdcRead()*1000)>>10);
		return TRUE;
	}

	return FALSE;
}

static bool __inline SysCmdHandler_B(char **pCmd,const char *pStrCopy,char *pOutStream)
{

	return FALSE;
}

static bool __inline SysCmdHandler_C(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"change")==0)
	{
		if(strcmp((void *)pCmd[1],"id")==0) {UpdateObjIdBase();}
		else if(strcmp((void *)pCmd[1],"str")==0) {UpdateInfoFlag(IFT_STR);}
		else if(strcmp((void *)pCmd[1],"var")==0) {UpdateInfoFlag(IFT_VARIABLE);}
		else if(strcmp((void *)pCmd[1],"rf")==0) {UpdateInfoFlag(IFT_RF_DATA);}
		else if(strcmp((void *)pCmd[1],"ir")==0) {UpdateInfoFlag(IFT_IR_DATA);}
		else if(strcmp((void *)pCmd[1],"dev")==0) {UpdateInfoFlag(IFT_DEV);}
		else if(strcmp((void *)pCmd[1],"trig")==0) {UpdateInfoFlag(IFT_TRIGGER);}
		else if(strcmp((void *)pCmd[1],"scn")==0) {UpdateInfoFlag(IFT_SCENE);}
		else
		{
			u16 i;
			
			CDebug("Obj ID Base: %u (0x%x)\n\r",ObjIdBase(),ObjIdBase());

			for(i=0;i<IFT_MAX;i++)
			{
				CDebug("[%s] Flag %10u (0x%x)\n\r",gNameInfoName[i],GetInfoFlag(i),GetInfoFlag(i));
			}
		}

		return TRUE;
	}
	
	return FALSE;
}

static bool __inline SysCmdHandler_D(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"debug")==0)
	{
		if(IsNullStr(pCmd[1]))//help
		{
			Sys_DbDebug(0);
		}
		else
		{
			//INFO_TYPE Name;
			
			//if(NotNullStr(pCmd[2])) Idx=Str2Uint(pCmd[2]);
			//if(NotNullStr(pCmd[3])) Num=Str2Uint(pCmd[3]);
			
			if(strcmp((void *)pCmd[1],"lcd")==0){Sys_DbDebug(1);}
			else if(strcmp((void *)pCmd[1],"hw")==0){HWC_Debug();}
			else if(strcmp((void *)pCmd[1],"task")==0){OS_GetAllTaskInfo();}
			else if(strcmp((void *)pCmd[1],"client")==0) {AppClientDebug();}
			else if(strcmp((void *)pCmd[1],"var")==0) {VarListDebug();}
			else if(strcmp((void *)pCmd[1],"mem")==0){system_print_meminfo();}
			else if(strcmp((void *)pCmd[1],"sta")==0){DispWifiStation();}
			else if(strcmp((void *)pCmd[1],"ap")==0){DispWifiAp();}
			else if(strcmp((void *)pCmd[1],"db")==0){QDB_Debug();}
			else if(strcmp((void *)pCmd[1],"trig")==0) {TrigListDebug();}
			else if(strcmp((void *)pCmd[1],"defend")==0) {ScnDefendDebug();}			
			else if(strcmp((void *)pCmd[1],"notify")==0){NotifyList_Debug();}
			else if(strcmp((void *)pCmd[1],"pcb")==0){_Lwip_DebugPcb();}
			else if(strcmp((void *)pCmd[1],"inform")==0){InformListDebug();}
			else if(strcmp((void *)pCmd[1],"up")==0){VarsJsonUp_Debug();}
			
		}

		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"disp")==0)
	{
		if(IsNullStr(pCmd[2]))
		{
			if(strcmp((void *)pCmd[1],"all")==0) EnDebugBit(DFT_MAX);
			else if(strcmp((void *)pCmd[1],"sys")==0) EnDebugBit(DFT_SYS);
			else if(strcmp((void *)pCmd[1],"tcp")==0) EnDebugBit(DFT_TCP);
			else if(strcmp((void *)pCmd[1],"srv")==0) EnDebugBit(DFT_SRV);
			else if(strcmp((void *)pCmd[1],"json")==0) EnDebugBit(DFT_SRV_JSON);
			else if(strcmp((void *)pCmd[1],"app")==0) EnDebugBit(DFT_APP);
			else if(strcmp((void *)pCmd[1],"task")==0) EnDebugBit(DFT_TASK);
			else if(strcmp((void *)pCmd[1],"info")==0) EnDebugBit(DFT_INFO);
			else if(strcmp((void *)pCmd[1],"pkt")==0) EnDebugBit(DFT_PKT);
			else if(strcmp((void *)pCmd[1],"pkt_data")==0) EnDebugBit(DFT_PKT_DATA);
			else if(strcmp((void *)pCmd[1],"mod")==0) EnDebugBit(DFT_WIFI);
			else if(strcmp((void *)pCmd[1],"var")==0) EnDebugBit(DFT_VAR);
			else if(strcmp((void *)pCmd[1],"inform")==0) EnDebugBit(DFT_VAR_INF);
			else if(strcmp((void *)pCmd[1],"show")==0) CDebug("Disp Flag:0x%x\n\r",QDB_GetValue(SDN_QCK,QIN_DEBUG_FLAG,DFT_MAX,NULL));
		}
		else
		{
			if(strcmp((void *)pCmd[1],"all")==0) DisDebugBit(DFT_MAX);
			else if(strcmp((void *)pCmd[1],"sys")==0) DisDebugBit(DFT_SYS);
			else if(strcmp((void *)pCmd[1],"tcp")==0) DisDebugBit(DFT_TCP);
			else if(strcmp((void *)pCmd[1],"srv")==0) DisDebugBit(DFT_SRV);
			else if(strcmp((void *)pCmd[1],"json")==0) DisDebugBit(DFT_SRV_JSON);
			else if(strcmp((void *)pCmd[1],"app")==0) DisDebugBit(DFT_APP);
			else if(strcmp((void *)pCmd[1],"task")==0) DisDebugBit(DFT_TASK);
			else if(strcmp((void *)pCmd[1],"info")==0) DisDebugBit(DFT_INFO);
			else if(strcmp((void *)pCmd[1],"pkt")==0) DisDebugBit(DFT_PKT);
			else if(strcmp((void *)pCmd[1],"pkt_data")==0) DisDebugBit(DFT_PKT_DATA);
			else if(strcmp((void *)pCmd[1],"mod")==0) DisDebugBit(DFT_WIFI);
			else if(strcmp((void *)pCmd[1],"var")==0) DisDebugBit(DFT_VAR);
			else if(strcmp((void *)pCmd[1],"inform")==0) DisDebugBit(DFT_VAR_INF);
		}

		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"def")==0)
	{
		if(strcmp((void *)pCmd[1],"db")==0) QDB_BurnDefaultToSpiFlash(SDN_SYS);
		else if(strcmp((void *)pCmd[1],"hw")==0) QDB_BurnDefaultToSpiFlash(SDN_HWC);
		else if(strcmp((void *)pCmd[1],"qck")==0) QDB_BurnDefaultToSpiFlash(SDN_QCK);

		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"def_all")==0)
	{
		QDB_BurnDefaultToSpiFlash(SDN_SYS);
		QDB_BurnDefaultToSpiFlash(SDN_HWC);
		QDB_BurnDefaultToSpiFlash(SDN_QCK);
		InfoSaveInit(TRUE);//恢复出厂设置
		ConfigChange(TRUE,TRUE);
		RebootBoard();
		return TRUE;
	}


	return FALSE;
}

static bool __inline SysCmdHandler_E(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"eee")==0)
	{
		u32 a=Str2Uint(pCmd[1]);
		u32 b=Str2Uint(pCmd[2]);

		Debug("ieee2f %f = %x\n\r",110.80,Float2Ieee(110.80));
		Debug("ieee2f %f = %x\n\r",0.124,Float2Ieee(0.124));
		Debug("float2i %x = %f\n\r",0x3DFDF3B6,Ieee2Float(0x3DFDF3B6));
		Debug("float2i %x = %f\n\r",0x42DDCC80,Ieee2Float(0x42DDCC80));
		
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"erase")==0)
	{
		if(IsNullStr(pCmd[1]))//help
		{
			CDebug("----------------------------------------------\r\n");
			CDebug("erase sector num\r\n");
			CDebug("erase sector offset_sector num\r\n");
			CDebug("----------------------------------------------\r\n");
		}
		else if(NotNullStr(pCmd[1]))//erase sec num
		{
			u32 Sec=Str2Uint(pCmd[1]);
			u32 Num=Str2Uint(pCmd[2]);
			u32 Start=0;
			
			if(NotNullStr(pCmd[3]))//erase sec offset_start end
			{
				Start=Str2Uint(pCmd[2]);
				Num=Str2Uint(pCmd[3]);
			}
			
			if(Num && Num<=FLASH_SECTOR_BYTES)
			{
				SpiFlsEraseSector(Sec+Start,Num);
				CDebug("Erase Sec %u - %u Finish\r\n",Sec+Start,Sec+Start+Num-1);
			}
		}

		return TRUE;
	}	

	return FALSE;
}

static bool __inline SysCmdHandler_F(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"fls0")==0)
	{
		if(IsNullStr(pCmd[1]))//help
		{
			CDebug("----------------------------------------------\r\n");
			CDebug("fls0 sector num\r\n");
			CDebug("----------------------------------------------\r\n");
		}
		else if(NotNullStr(pCmd[1]))
		{
			u32 Sec=Str2Uint(pCmd[1]);
			u32 Num=Str2Uint(pCmd[2]);
			
			if(Num && Num<=FLASH_SECTOR_BYTES)
			{
				u8 Zero[]={0,0,0,0};
				SpiFlsWriteData(Sec*FLASH_SECTOR_BYTES+Num,4,(void *)Zero);		
			}
		}

		return TRUE;
	}	
	else if(strcmp((void *)pCmd[0],"fill")==0)
	{
		u8 Page=Str2Uint(pCmd[1]);
		u8 Col=Str2Uint(pCmd[2]);
		u8 Data=Str2Uint(pCmd[3]);
		u32 Num=Str2Uint(pCmd[4]);
	
		if(IsNullStr(pCmd[4])) Num=128;		
		//Oled_Fill(Page,Col,Data,Num);
		return TRUE;
	}

	return FALSE;
}

static bool __inline SysCmdHandler_G(char **pCmd,const char *pStrCopy,char *pOutStream)
{return FALSE;}

static bool __inline SysCmdHandler_H(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"heap")==0)	
	{
		system_show_malloc();
		return TRUE;
	}
	return FALSE;
}

#if 1
static bool __inline SysCmdHandler_InfoDebug(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	u8 *Buf=Q_Malloc(512);

	if(strcmp((void *)pCmd[0],"dev")==0)	
	{
		u32 *pID=(void *)Buf;
		INFO_TYPE Type=IFT_DEV;
		u32 Total=GetTypeInfoTotal(Type);
		u32 i,ItemLen=GetTypeItemSize(Type);

		for(i=1;i<=Total;i++)
		{
			if(ReadInfoByIdx(Type,i,Buf))
			{
				DEVICE_RECORD *pRcd=(void *)Buf;
			
				CDebug("[%u]%s File-Prod:%u-%u, UI:%u, ",pRcd->ID,pRcd->Name,pRcd->FileID,pRcd->ProdID,pRcd->UiID);
				switch(pRcd->ProdID)
				{
					case PID_IR_KEYS:
						{
							CDebugCol("IR");
							CDebug(" @%x.%u:\n\r",HBit16(pRcd->Record.KeysSet.TransAddr),LBit16(pRcd->Record.KeysSet.TransAddr));
							//DisplayBuf(pRcd->Record.KeysSet.KeyList,KEYS_SET_MAX_KEY_NUM,16);
							//DisplayBufU16(pRcd->Record.KeysSet.SigList,KEYS_SET_MAX_KEY_NUM,16);
						}
						break;
					case PID_RF_KEYS:
						{
							CDebugCol("RF");
							CDebug(" @%x.%u:\n\r",HBit16(pRcd->Record.KeysSet.TransAddr),LBit16(pRcd->Record.KeysSet.TransAddr));
							//DisplayBuf(pRcd->Record.KeysSet.KeyList,KEYS_SET_MAX_KEY_NUM,16);
							//DisplayBufU16(pRcd->Record.KeysSet.SigList,KEYS_SET_MAX_KEY_NUM,16);
						}
						break;
					case PID_COM:
						{
							CDebugCol("Com");
							CDebug("\n\r\n");
						}	
						break;
					default:
						if(ProdIsWav(pRcd->ProdID))
						{
							WAVER_DEV *pWav=&pRcd->Record.Waver;
							u8 n;

							if(pRcd->Record.Waver.DutType==WDT_WDEV_IO) {CDebugCol("Waver");}
							else if(pRcd->Record.Waver.DutType==WDT_OPEN_GATEWAY) {CDebugCol("Zigbee");}
							else if(pRcd->Record.Waver.DutType==WDT_JMP) {CDebugCol("Jumper");}
							else if(pRcd->Record.Waver.DutType==WDT_WIFI) {CDebugCol("Wifi");}
							else CDebugCol("Unknow[%u]",pRcd->Record.Waver.DutType);

							CDebug(" :%x.%u, %u Vars %s\n\r",HBit16(pRcd->Record.Waver.WNetAddr),LBit16(pRcd->Record.Waver.WNetAddr),pRcd->Record.Waver.SelfVarNum,pRcd->Record.Waver.NeedPw?"*":"");

							for(n=0;n<RELATE_VAR_MAX;n++)
							{
								if(pWav->RelateVar[n]) CDebug("[R%u]%u ",n+1,pWav->RelateVar[n]);
							}
							CDebug("\n\r\n\r");									
						}
						break;
				}
			}
			else
				CDebug("Read Type:%u,idx:%u = Null\n\r",Type,i,*pID);
		}
	}
	else if(strcmp((void *)pCmd[0],"str")==0)	
	{
		u32 *pID=(void *)Buf;
		INFO_TYPE Type=IFT_STR;
		u32 Total=GetTypeInfoTotal(Type);
		u32 i,ItemLen=GetTypeItemSize(Type);

		for(i=1;i<=Total;i++)
		{
			if(ReadInfoByIdx(Type,i,Buf))
			{
				STR_RECORD *pRcd=(void *)Buf;

				switch(pRcd->Type)
				{
					case SRT_STR:
						CDebug("[Str%u:SubId%u]%s\n\r",pRcd->ID,pRcd->SubID,pRcd->Data);
						break;
					case SRT_DATA:
						CDebug("[Str%u:SubId%u]Date:\n\r",pRcd->ID,pRcd->SubID);
						DisplayBuf(pRcd->Data,pRcd->DataLen,16);								
						break;
					case SRT_DEV_STR:
						CDebug("[Str%u:Dev%u:Num%u]%s\n\r",pRcd->ID,pRcd->SubID,pRcd->Num,pRcd->Data);							
						break;
					case SRT_VAR_UP_CONF:
						CDebug("[Str%u:Var%u:Num%u]%s\n\r",pRcd->ID,pRcd->SubID,pRcd->Num,pRcd->Data);							
						break;
				}
			}
			else
				CDebug("Read Type:%u,idx:%u = Null\n\r",Type,i,*pID);
		}
	}
	else if(strcmp((void *)pCmd[0],"var")==0)	
	{
		u32 *pID=(void *)Buf;
		INFO_TYPE Type=IFT_VARIABLE;
		u32 Total=GetTypeInfoTotal(Type);
		u32 i,ItemLen=GetTypeItemSize(Type);

		for(i=1;i<=Total;i++)
		{
			if(ReadInfoByIdx(Type,i,Buf))
			{
				VARIABLE_RECORD *pRcd=(void *)Buf;

				CDebug("[%s]",gNameVarType[pRcd->VarType]);
				CDebug("[%c%c%c%c.%c%c%c%c]",DispVarTagp(pRcd));
				CDebug("%u:%u %sInit:%u %s:%u",pRcd->ID,LBit16(pRcd->ID),pRcd->ReadOnly?"RO ":"   ",pRcd->InitVal,gNameVarFat[pRcd->DispFat],pRcd->DotNum);		
				if(pRcd->MainDevID) CDebug("    ->%u:%u\n\r",HL16_U32(GOT_DEVICE,pRcd->MainDevID),pRcd->VarIdx);
				else CDebug("\n\r");
			}
			else
				CDebug("Read Type:%u,idx:%u = Null\n\r",Type,i,*pID);
		}
	}
	else if(strcmp((void *)pCmd[0],"scn")==0)	
	{
		u32 *pID=(void *)Buf;
		INFO_TYPE Type=IFT_SCENE;
		u32 Total=GetTypeInfoTotal(Type);
		u32 i,ItemLen=GetTypeItemSize(Type);

		for(i=1;i<=Total;i++)
		{
			if(ReadInfoByIdx(Type,i,Buf))
			{
				SCENE_RECORD *pRcd=(void *)Buf;
				SCENE_ITEM *pItems=pRcd->Items;
				
				CDebug("[%u]%s%s\n\r",pRcd->ID,pRcd->Name,ScnIsActive(pRcd->ID)?" *":"");

				while(pItems->Act!=SIA_NULL)
				{
					CDebug("  |%s:",gNameSceneItemAct[pItems->Act]);

					switch(pItems->Act)
					{
						case SIA_KEY:
							CDebug("[%u]%u",pItems->Exc.Key.DevID,pItems->Exc.Key.Key);
							break;
						case SIA_VAR_SET:
							CDebug("[%u] & 0x%x %s %s%u",pItems->Exc.Var.Vid,pItems->Exc.Var.CalcMask,gNameCalcMethod[pItems->Exc.Var.CalcMethod&(~CMT_VAR_CALC_BIT)],(pItems->Exc.Var.CalcMethod&CMT_VAR_CALC_BIT)?"v":"",pItems->Exc.Var.CalcVal);
							break;
						case SIA_SYS_MSG:	
							{
								u8 *pStr=Q_Malloc(STR_RECORD_BYTES);
								u8 *pUser=Q_Malloc(STR_RECORD_BYTES);

								GetStrRecordData(pItems->Exc.Str.StrID,pStr);
								GetStrRecordData(pItems->Exc.Str.UserID,pUser);
								
								CDebug("[(%u)%s:(%u)%s]%s%s%s%s",pItems->Exc.Str.UserID,pUser,pItems->Exc.Str.StrID,pStr,pItems->Exc.Str.Sys?"[Sys]":"",pItems->Exc.Str.GsmMsg?"[Gsm]":"",pItems->Exc.Str.PushMsg?"[Push]":"",pItems->Exc.Str.DevStr?"[DevStr]":"");

								Q_Free(pStr);
								Q_Free(pUser);
							}
							break;
						default:
							CDebug("Unknow(%u)",pItems->Act);
							DisplayBuf((void *)pItems,sizeof(SCENE_ITEM),16);
					}
					CDebug(", Delay %uS\n\r",pItems->DelaySec);
					pItems++;
				}									
			}
			else
				CDebug("Read Type:%u,idx:%u = Null\n\r",Type,i,*pID);
		}
	}
	else if(strcmp((void *)pCmd[0],"trig")==0)	
	{
		u32 *pID=(void *)Buf;
		INFO_TYPE Type=IFT_TRIGGER;
		u32 Total=GetTypeInfoTotal(Type);
		u32 i,ItemLen=GetTypeItemSize(Type);

		for(i=1;i<=Total;i++)
		{
			if(ReadInfoByIdx(Type,i,Buf))
			{
				TRIGGER_RECORD *pRcd=(void *)Buf;

				CDebug("[%u]",pRcd->ID);
				CDebugCol("%s",gNameTriggerType[pRcd->TrigType]);
				CDebug("%s:",pRcd->Name);

				switch(pRcd->TrigType)
				{
					case TT_IN_RF:
						CDebug("%x@%uuS",pRcd->Oppo.Rf.Code,pRcd->Oppo.Rf.BasePeriod);
						break;
					case TT_APP_KEY:
						CDebug("Dev%u:Key%u",pRcd->Oppo.KeyVal.DevID,pRcd->Oppo.KeyVal.Key);
						break;
					case TT_SYS_MSG:
						CDebug("Str%u[Hash %u]",pRcd->Oppo.SysMsg.StrID,pRcd->Oppo.SysMsg.Hash);
						break;
					case TT_VAR_SET:
						CDebug("Var%u %s &:%x,",pRcd->Oppo.VarTrigger.Vid,pRcd->Oppo.VarTrigger.Method==VCM_IN?"IN":"OUT",pRcd->Oppo.VarTrigger.Mask);
						CDebug("AB:%d-%d,Time:%u",pRcd->Oppo.VarTrigger.CalcA,pRcd->Oppo.VarTrigger.CalcB,pRcd->Oppo.VarTrigger.TimeSec);
						break;
					default:
						CDebug("Unknow(%u)",pRcd->TrigType);
				}
				CDebug(" -> Scn%u%s\n\r",pRcd->SceneID,ScnIsActive(pRcd->SceneID)?"*":"");	

				if(pRcd->VarState.Vid)
				{
					CDebug("  |CheckVar:Var%u %s, AB:%d-%d & %x\n\r",pRcd->VarState.Vid,pRcd->VarState.Method==VCM_IN?"IN":"OUT",pRcd->VarState.CalcA,pRcd->VarState.CalcB,pRcd->VarState.Mask);
				}
			}
			else
				CDebug("Read Type:%u,idx:%u = Null\n\r",Type,i,*pID);
		}
	}
	
	Q_Free(Buf);

	return TRUE;
}

static bool __inline SysCmdHandler_Info(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"init")==0)
	{
		u32 Num=Str2Uint(pCmd[1]);
		CheckAllSysVarHave(Num?TRUE:FALSE);
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"new_scn")==0)
	{
		SCENE_RECORD *pScn=Q_Malloc(sizeof(SCENE_RECORD));
		SCENE_ITEM *pItems=pScn->Items;
		u32 DevID=Str2Uint(pCmd[1]);
		u16 Vid=Str2Uint(pCmd[2]);
		u32 StrID=Str2Uint(pCmd[3]);
		
		pScn->ID=HL16_U32(GOT_SCENE,ObjIdBase());
		UpdateObjIdBase();
		sprintf(pScn->Name,"情景%u",Rand(0xffff));

		pItems[0].Act=SIA_KEY;
		pItems[0].DelaySec=0;
		pItems[0].Exc.Key.DevID=DevID;
		pItems[0].Exc.Key.Key=1;

		pItems[1].Act=SIA_VAR_SET;
		pItems[1].DelaySec=0;
		pItems[1].Exc.Var.Vid=Vid&0xffff;
		pItems[1].Exc.Var.CalcMethod=CMT_INC;
		pItems[1].Exc.Var.CalcMask=0xffff;
		pItems[1].Exc.Var.CalcVal=1;

		pItems[2].Act=SIA_SYS_MSG;
		pItems[2].DelaySec=0;
		pItems[2].Exc.Str.StrID=StrID;
		pItems[2].Exc.Str.UserID=1;
		pItems[2].Exc.Str.PushMsg=1;

		SaveInfo(IFT_SCENE,pScn);

		Q_Free(pScn);
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"new_trig")==0)
	{
		TRIGGER_RECORD *pTrig=Q_Malloc(sizeof(TRIGGER_RECORD));
		u32 WDev=Str2Uint(pCmd[1]);
		u16 Oppo=Str2Uint(pCmd[2]);
		
		pTrig->ID=HL16_U32(GOT_TRIGGER,ObjIdBase());
		UpdateObjIdBase();
		sprintf(pTrig->Name,"触发源%u",Rand(0xffff));

		pTrig->TrigType=TT_IN_RF;

		SaveInfo(IFT_TRIGGER,pTrig);

		Q_Free(pTrig);
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"new_str")==0)
	{
		STR_RECORD *pStr=Q_Malloc(sizeof(STR_RECORD));
		
		pStr->ID=HL16_U32(GOT_NAME,ObjIdBase());
		UpdateObjIdBase();

		pStr->Type=SRT_STR;
		sprintf(pStr->Data,"%s",pCmd[1]);
		pStr->DataLen=strlen(pCmd[1])+1;

		SaveInfo(IFT_STR,pStr);

		Q_Free(pStr);
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"block")==0)	
	{
		u32 Block=Str2Uint(pCmd[1]);

		switch(Block)
		{
			case 128:DebugInfoSave(IBN_128B);break;
			case 256:DebugInfoSave(IBN_256B);break;
			case 512:DebugInfoSave(IBN_512B);break;				
		}
		
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"new")==0)	
	{
		INFO_TYPE Type=Str2Uint(pCmd[1]);
		u32 AppID=Str2Uint(pCmd[2]);
		u8 *Buf=Q_Malloc(512);

		MemCpy(Buf,&AppID,4);
		sprintf(&Buf[4],"%08x",AppID);

		CDebug("SaveInfo Type:%u , Appid:%u\n\r",Type,AppID);
		SaveInfo(Type,Buf);

		Q_Free(Buf);
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"del")==0)	
	{
		u32 AppID=Str2Uint(pCmd[2]);
		INFO_TYPE Type=0;

		if(strcmp((void *)pCmd[1],"str")==0){Type=IFT_STR;AppID=HL16_U32(GOT_NAME,AppID);}
		else if(strcmp((void *)pCmd[1],"var")==0){Type=IFT_VARIABLE;AppID=HL16_U32(GOT_VARIABLE,AppID);}
		else if(strcmp((void *)pCmd[1],"rf")==0){Type=IFT_RF_DATA;AppID=HL16_U32(GOT_RF,AppID);}
		else if(strcmp((void *)pCmd[1],"ir")==0){Type=IFT_IR_DATA;AppID=HL16_U32(GOT_IR_SIGNAL,AppID);}
		else if(strcmp((void *)pCmd[1],"dev")==0){Type=IFT_DEV;AppID=HL16_U32(GOT_DEVICE,AppID);}
		else if(strcmp((void *)pCmd[1],"trig")==0){Type=IFT_TRIGGER;AppID=HL16_U32(GOT_TRIGGER,AppID);}
		else if(strcmp((void *)pCmd[1],"scn")==0){Type=IFT_SCENE;AppID=HL16_U32(GOT_SCENE,AppID);}
		else return FALSE;
		
		if(DeleteInfo(Type,AppID))//type,id
		{
			CDebug("Del %u %u Ok!\n\r",Type,AppID);
		}
		else
		{
			CDebug("Del %u %u Faild!\n\r",Type,AppID);
		}
		
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"read")==0)	
	{
		u8 *Buf=Q_Malloc(512);
		u32 *pID=(void *)Buf;
		INFO_TYPE Type=Str2Uint(pCmd[1]);
		u32 Idx=Str2Uint(pCmd[2]);

		if(ReadInfoByIdx(Type,Idx,Buf))
			CDebug("Read Type:%u,Idx:%u = [%x]:%s\n\r",Type,Idx,*pID,&Buf[4]);
		else
			CDebug("Read Type:%u,Idx:%u = Null\n\r",Type,Idx,*pID);

		Q_Free(Buf);
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"read_bulk")==0)	
	{
		u8 *Buf=Q_Malloc(512);
		u32 *pID=(void *)Buf;
		INFO_TYPE Type=Str2Uint(pCmd[1]);
		u32 Total=GetTypeInfoTotal(Type);
		u32 i=1,ItemLen=GetTypeItemSize(Type);

		for(i=1;i<=Total;i++)
		{
			if(ReadInfoByIdx(Type,i,Buf))
			{
				CDebug("Read Type:%u,idx:%u = %u[%x]:%s\n\r",Type,i,*pID,*pID);
				DisplayBuf(Buf,ItemLen,16);
			}
			else
				CDebug("Read Type:%u,idx:%u = Null\n\r",Type,i,*pID);
		}
		
		Q_Free(Buf);
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"readid")==0)	
	{
		u8 *Buf=Q_Malloc(512);
		u32 *pID=(void *)Buf;
		
		ReadInfoByID(Str2Uint(pCmd[1]),Str2Uint(pCmd[2]),Buf);
		CDebug("Read[%x]:%s\n\r",*pID,&Buf[4]);

		Q_Free(Buf);
		return TRUE;
	}		
	else if(strcmp((void *)pCmd[0],"build")==0)	
	{
		INFO_BLOCK Block=Str2Uint(pCmd[1]);
		CDebug("Rebuild info block[%u]\n\r",Block);
		InfoBuildBlock(Block,FALSE);
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"clean")==0)	
	{
		InfoSaveInit(TRUE);//恢复出厂设置
		ConfigChange(FALSE,TRUE);
		RebootBoard();
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"num")==0)
	{
		u16 i;

		CDebug("[Name    ] size  vaild  free  block\n\r");
		for(i=0;i<IFT_MAX;i++)
		{
			CDebug("[%s] %3u   %4u   %4u   %3uB\n\r",gNameInfoName[i],GetTypeItemSize(i),GetTypeInfoTotal(i),GetTypeFreeUnit(i),GetTypeBlockSize(i));
		}
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"flag")==0)
	{
		u8 InfoType;

		CDebug(" IdBase:%u\n\r",ObjIdBase());
		CDebug(" Sys:%u\n\r",GetSysConfigFlag());
		for(InfoType=0;InfoType<IFT_MAX;InfoType++)
		{
			CDebug(" %s:%u\n\r",gNameInfoName[InfoType],GetInfoFlag(InfoType));
		}

		return TRUE;
	}
	else
	{
		return SysCmdHandler_InfoDebug(pCmd,pStrCopy,pOutStream);
	}

	return FALSE;
}
#endif
static bool __inline SysCmdHandler_I(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"info")==0)
	{
		return SysCmdHandler_Info(&pCmd[1],pStrCopy,pOutStream);
	}
	else if(strcmp((void *)pCmd[0],"ipset")==0)
	{
		if(IsNullStr(pCmd[1]))//help
		{
			CDebug("----------------------------------------------\r\n");
			CDebug("ipset xxx.xxx.xxx.xxx\r\n");
			CDebug("----------------------------------------------\r\n");
		}
		else if((NotNullStr(pCmd[1]))&&(NotNullStr(pCmd[2]))&&(NotNullStr(pCmd[3]))&&(NotNullStr(pCmd[4])))
		{
			u8 Ip[4]={0,0,0,0};
			Ip[0]=Str2Uint(pCmd[1]);
			Ip[1]=Str2Uint(pCmd[2]);
			Ip[2]=Str2Uint(pCmd[3]);
			Ip[3]=Str2Uint(pCmd[4]);

			if(Ip[3])
			{
				LwIP_ChangeIP(Ip[0],Ip[1],Ip[2],Ip[3]);
			}
		}
		else if(NotNullStr(pCmd[1]))
		{
			u8 Ip[4]={0,0,0,0};
			u8 *pIp1Str=pCmd[1];
			u8 *pIp2Str;
			u8 *pIp3Str;
			u8 *pIp4Str;

			pIp2Str=FindStr(pIp1Str,".",NULL);
			if(pIp2Str == NULL) return TRUE;
			
			pIp3Str=FindStr(pIp2Str,".",NULL);
			if(pIp2Str == NULL) return TRUE;

			pIp4Str=FindStr(pIp3Str,".",NULL);
			if(pIp2Str == NULL) return TRUE;
			
			Ip[0]=Str2Uint(pIp1Str);
			Ip[1]=Str2Uint(pIp2Str);
			Ip[2]=Str2Uint(pIp3Str);
			Ip[3]=Str2Uint(pIp4Str);

			if(Ip[3])
			{
				LwIP_ChangeIP(Ip[0],Ip[1],Ip[2],Ip[3]);
			}
		}

		return TRUE;
	}

	return FALSE;
}

static bool __inline SysCmdHandler_J(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"json")==0)
	{
		char *p1=strchr((void *)&pStrCopy[5],'.');
		char *p2=strchr((void *)&pStrCopy[5],' ');
		if(p1==NULL || p2!=NULL)
		{
			Debug("srv addr error!\n\r");
			return TRUE;
		}
		
		QDB_SetStr(SDN_SYS,SIN_JsonServerURL,(void *)&pStrCopy[5]);				
		QDB_BurnToSpiFlash(SDN_SYS);
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"jsonp")==0)
	{
		u32 Idx=Str2Uint(pCmd[1]);

		if(Idx)
		{
			QDB_SetNum(SDN_SYS,SIN_DataUpPeriod,Idx);
			Debug("Set json up period to %umS\n\r",PeriodIdx2Ms(Idx));
		}
		else
		{
			Idx=QDB_GetNum(SDN_SYS,SIN_DataUpPeriod);
			Debug("Json up period is %umS\n\r",PeriodIdx2Ms(Idx));
		}

		return TRUE;
	}

	return FALSE;
}

static bool __inline SysCmdHandler_K(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"key")==0)
	{
		u32 Pin=Str2Uint(pCmd[1]);
		u32 Arg=Str2Uint(pCmd[2]);
		
		Debug("Simulation app key %u\r\n",Pin*1000+Arg);
		SysEventSend(SEN_PIN_CHANGE,Pin*1000+Arg,NULL,NULL);
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"karlno")==0)
	{
		u32 Key=Str2Uint(pCmd[1]);
		
		if(Key==GetDutRegSn() || Key==580319)
		{
			if(GetDutRegSn()!=QDB_GetNum(SDN_SYS,SIN_DutRegSn))
			{
				QDB_SetNum(SDN_SYS,SIN_DutRegSn,GetDutRegSn());
				QDB_BurnToSpiFlash(SDN_SYS);
			}
			SysVars()->DutRegSnIsOk=TRUE;
		}
	}
	
	return FALSE;
}

static bool __inline SysCmdHandler_L(char **pCmd,const char *pStrCopy,char *pOutStream)
{return FALSE;}

static bool __inline SysCmdHandler_M(char **pCmd,const char *pStrCopy,char *pOutStream)
{return FALSE;}

static bool __inline SysCmdHandler_N(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"now")==0)
	{
		RTC_TIME Time;
		
		RtcCnt2Time(RtcGetCnt(),&Time);
		
		if(pOutStream!=NULL)
		{
			sprintf((void *)pOutStream,"StartSec:%u.%u\r\n",OS_GetNowMs()/1000,OS_GetNowMs()%1000);
			sprintf((void *)&pOutStream[strlen((void *)pOutStream)],"Now:%04u-%02u-%02u [%u] %02u:%02u:%02u\r\nRtcCnt:%u\r\n",Time.year,Time.mon,Time.day,Time.week,
				Time.hour,Time.min,Time.sec,RtcGetCnt());
		}
		else
		{
			CDebug("StartSec:%u.%u\r\n",OS_GetNowMs()/1000,OS_GetNowMs()%1000);
			CDebug("Now:%04u-%02u-%02u [%u] %02u:%02u:%02u\r\nRtcCnt:%u\r\n",Time.year,Time.mon,Time.day,Time.week,
					Time.hour,Time.min,Time.sec,RtcGetCnt());
		}
		
		return TRUE;
	}

	return FALSE;
}

static bool __inline SysCmdHandler_O(char **pCmd,const char *pStrCopy,char *pOutStream)
{return FALSE;}

static bool __inline SysCmdHandler_P(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"pw")==0)
	{
		if(IsNullStr(pCmd[1]))
		{
			QDB_SetNum(SDN_SYS,SIN_UserPwHash,MakeHash33("888888",6));
			QDB_BurnToSpiFlash(SDN_SYS);
		}
		else
		{
			Debug("Set New Password:%s\n\r",pCmd[1]);
			QDB_SetNum(SDN_SYS,SIN_UserPwHash,MakeHash33(pCmd[1],strlen((void *)pCmd[1])));
			QDB_BurnToSpiFlash(SDN_SYS);
		}
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"pw_def")==0)
	{
		sprintf((void *)pStrCopy,"%u",GenerateDefPw());//得到密码
		QDB_SetNum(SDN_SYS,SIN_UserPwHash,MakeHash33(pStrCopy,strlen((void *)pStrCopy)));
		QDB_BurnToSpiFlash(SDN_SYS);
		
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"print")==0)
	{
		static u8 flag=1;

		if(flag==0) UART_SetPrintPort(UART1);
		else UART_SetPrintPort(UART0);

		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"pin")==0)
	{
		u32 Pin=Str2Uint(pCmd[1]);
		u32 Val=Str2Uint(pCmd[2]);

		if(NotNullStr(pCmd[2]))
		{
			if(Pin<16)
			{
				GPIO_OUTPUT_SET(GPIO_ID_PIN(Pin),Val);
			}
			else if(Pin==16)
			{
				gpio16_output_set(Val);
			}
		}
		else
		{
			if(Pin<16)
			{
				Debug("Pin %u=%u\n\r",Pin,GPIO_INPUT_GET(GPIO_ID_PIN(Pin)));
			}
			else if(Pin==16)
			{
				Debug("Pin %u=%u\n\r",Pin,gpio16_input_get());
			}
		}
		
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"pwm")==0)
	{
		if(strcmp((void *)pCmd[1],"s")==0)
		{
			pwm_start();
		}
		else if(strcmp((void *)pCmd[1],"p")==0)
		{
			u32 Period=Str2Uint(pCmd[2]);
			pwm_set_period(Period);
		}
		else if(strcmp((void *)pCmd[1],"d")==0)
		{
			u32 duty=Str2Uint(pCmd[2]);
			u32 chan=Str2Uint(pCmd[3]);

			pwm_set_duty(duty,chan);
		}
		else
		{
			
		}


		return TRUE;
	}
	
	return FALSE;
}

bool MyList_FreeItemTest(QLIST_ITEM_H *pItem)
{
	Debug("Free 0x%x\n\r",pItem);
	return TRUE;
}

bool MyList_FilterTest(QLIST_ITEM_H *pInList,QLIST_ITEM_H *pCompare)
{
	if(pInList->ID==pCompare->ID)
	{
		return TRUE;
	}

	return FALSE;
}

bool MyList_FilterTest2(QLIST_ITEM_H *pInList,QLIST_ITEM_H *pCompare)
{
	if(pInList->ID%10 == pCompare->ID%10)
	{
		return TRUE;
	}

	return FALSE;
}

static QLIST_INFO gQList;
static bool __inline SysCmdHandler_Q(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"ql_debug")==0)
	{
		QList_Debug(&gQList);
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"ql_new")==0)
	{
		QList_New(&gQList,16,8,(void *)MyList_FreeItemTest);
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"ql_add")==0)
	{
		QLIST_ITEM_H Item={0,NULL,{0,0,0,0}};

		Item.ID=Str2Uint(pCmd[1]);
		if(NotNullStr(pCmd[2])) MemCpy(Item.Data,pCmd[2],4);
		
		QList_AddItem(&gQList,&Item);
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"ql_del")==0)
	{
		QLIST_ITEM_H Item;
		Item.ID=Str2Uint(pCmd[1]);
		
		QList_DeleteByFilter(&gQList,(void *)MyList_FilterTest,&Item,Str2Uint(pCmd[2]));
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"ql_del2")==0)
	{
		QLIST_ITEM_H Item;
		Item.ID=Str2Uint(pCmd[1]);
		
		QList_DeleteByFilter(&gQList,(void *)MyList_FilterTest2,&Item,Str2Uint(pCmd[2]));
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"ql_read")==0)
	{
		QLIST_ITEM_H Item,Ret;
		Item.ID=Str2Uint(pCmd[1]);
		
		QList_ReadItemByFilter(&gQList,(void *)MyList_FilterTest2,&Item,&Ret);

		Debug("Read[%u]%u(0x%x)\n\r",Ret.ID,*(u32 *)Ret.Data,*(u32 *)Ret.Data);
		return TRUE;
	}

	return FALSE;
}

static bool __inline SysCmdHandler_R(char **pCmd,const char *pStrCopy,char *pOutStream)
{	
	if(strcmp((void *)pCmd[0],"reset")==0 || strcmp((void *)pCmd[0],"reboot")==0)
	{
		RebootBoard();			
		return TRUE;
	}	
	else if(strcmp((void *)pCmd[0],"rtc")==0)
	{
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"read")==0)
	{
		if(IsNullStr(pCmd[1]))//help
		{
			CDebug("----------------------------------------------\r\n");
			CDebug("read sector num\r\n");
			CDebug("read sector offset_bytes num\r\n");
			CDebug("----------------------------------------------\r\n");
		}
		else if(NotNullStr(pCmd[1]))//read sec num
		{
			u32 Sec=Str2Uint(pCmd[1]);
			u32 Num=Str2Uint(pCmd[2]);
			u32 Start=0;
			
			if(NotNullStr(pCmd[3]))//read sec offset_start end
			{
				Start=Str2Uint(pCmd[2]);
				Num=Str2Uint(pCmd[3]);
			}
			
			if(Num && Num<=FLASH_SECTOR_BYTES)
			{
				u8 *pData=Q_Malloc(Num);
				SpiFlsReadData(Sec*FLASH_SECTOR_BYTES+Start,Num,(void *)pData);
				CDebug("Sec %u Data:\r\n",Sec);
				DisplayBuf(pData,Num,16);
				Q_Free(pData);
			}
		}

		return TRUE;
	}	
	else if(strcmp((void *)pCmd[0],"readstr")==0)
	{
		if(IsNullStr(pCmd[1]))//help
		{
			CDebug("----------------------------------------------\r\n");
			CDebug("readstr sector start end\r\n");
			CDebug("----------------------------------------------\r\n");
		}
		else if(NotNullStr(pCmd[1]))
		{
			u32 Sec=Str2Uint(pCmd[1]);
			u32 Num=Str2Uint(pCmd[2]);
			u32 Start=0;
			
			if(NotNullStr(pCmd[3]))
			{
				Start=Str2Uint(pCmd[2]);
				Num=Str2Uint(pCmd[3])-Start;
			}
			
			if(Num && Num<=FLASH_SECTOR_BYTES)
			{
				u8 *pData=Q_Malloc(Num);
				SpiFlsReadData(Sec*FLASH_SECTOR_BYTES+Start,Num,(void *)pData);
				CDebug("Sec %u str:\r\n",Sec);
				{
					int i;
					
					for(i=0;i<Num;i++)
					{
						if(pData[i]==0) break;
						CDebug("%c",pData[i]);
					}
					CDebug("\n\r");
				}
				
				Q_Free(pData);
			}
		}

		return TRUE;
	}	
	
	return FALSE;
}

static bool __inline SysCmdHandler_S(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"save")==0)
	{
		CDebug("Save System Parameter\r\n");
		
		QDB_BurnToSpiFlash(SDN_SYS);
		
		if(pOutStream!=NULL) sprintf((void *)pOutStream,"ok\r\n");
		
		//ConfigChanged();
		
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"ssid")==0)
	{
		if(NotNullStr(pCmd[1]) && NotNullStr(pCmd[2]))
		{
			struct station_config *pStaConfig=Q_Malloc(sizeof(struct station_config));

			StrChrRep(pCmd[1],'^',' ');//替换^为空格
			StrChrRep(pCmd[2],'^',' ');//替换^为空格
			
			sprintf(pStaConfig->ssid,pCmd[1]);
			sprintf(pStaConfig->password,pCmd[2]);	
			wifi_station_set_config(pStaConfig);

			DispWifiStation();
			Q_Free(pStaConfig);
			return TRUE;
		}	
	}	
	else if(strcmp((void *)pCmd[0],"sta_only")==0)
	{
		wifi_set_opmode(STATION_MODE);
		return TRUE;
	}	
	else if(strcmp((void *)pCmd[0],"sizeof")==0)
	{
		//打印各种调试中经常用到的变量
		CDebug("sizeof(GLOBA_PKT_HEADER)=%d\r\n",sizeof(GLOBA_PKT_HEADER));
		CDebug("sizeof(SYS_PARAM_PKT)=%d\r\n",sizeof(SYS_PARAM_PKT));
		CDebug("sizeof(QH_RECORD)=%d\r\n",sizeof(QH_RECORD));
		CDebug("\n\r");
		
		CDebug("sizeof(STR_RECORD)=%d\r\n",sizeof(STR_RECORD));		
		CDebug("sizeof(VARIABLE_RECORD)=%d\r\n",sizeof(VARIABLE_RECORD));		
		CDebug("sizeof(DEVICE_RECORD)=%d\r\n",sizeof(DEVICE_RECORD));
		CDebug("sizeof(TRIGGER_RECORD)=%d\r\n",sizeof(TRIGGER_RECORD));
		CDebug("sizeof(SCENE_RECORD)=%d\r\n",sizeof(SCENE_RECORD));
		CDebug("sizeof(SCENE_ITEM)=%d\r\n",sizeof(SCENE_ITEM));
		CDebug("sizeof(EXC_KEY_ITEM)=%d\r\n",sizeof(EXC_KEY_ITEM));
		CDebug("sizeof(EXC_VAR_ITEM)=%d\r\n",sizeof(EXC_VAR_ITEM));
		CDebug("sizeof(EXC_STR_ITEM)=%d\r\n",sizeof(EXC_STR_ITEM));
		CDebug("\n\r");
		
		//CDebug("sizeof(WTRANS_PACKET)=%d\r\n",sizeof(WTRANS_PACKET));
		CDebug("sizeof(VARIABLE_PKT)=%d\r\n",sizeof(VARIABLE_PKT));
		//CDebug("sizeof(KEY_CTRL_PKT)=%d\r\n",sizeof(KEY_CTRL_PKT));
		CDebug("sizeof(LOGIN_PKT)=%d\r\n",sizeof(LOGIN_PKT));
		CDebug("\n\r");
		
		CDebug("sizeof(SYS_DB_STRUCT)=%d\r\n",sizeof(SYS_DB_STRUCT));
		CDebug("sizeof(VAR_OBJ)=%d\r\n",sizeof(VAR_OBJ));

		CDebug("strlen(123r)=%u\n\r",strlen("123\r"));
		
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"srv")==0)
	{
		if(IsNullStr(pCmd[1]))//打印当前信息
		{
			u8 Ip[5][4];
			u8 URL[NORMAL_STR_BYTES];
			bool NeedFree=FALSE;
			
			QDB_GetStr(SDN_SYS,SIN_ServerURL,URL);
			
			if(pOutStream==NULL)
			{
				pOutStream=Q_Malloc(1024);
				NeedFree=TRUE;
			}
			
			pOutStream[0]=0;
			QDB_GetIp(SDN_SYS,SIN_IpAddr,(void *)Ip[0]);
			sprintf((void *)&pOutStream[strlen(pOutStream)],"Ip:%u.%u.%u.%u(SETTING)\n\r",	Ip[0][0],Ip[0][1],Ip[0][2],Ip[0][3]);
			Lwip_GetNetInfo((void *)Ip);
			sprintf((void *)&pOutStream[strlen(pOutStream)],"Ip:%u.%u.%u.%u(DHCP)\n\r",	Ip[0][0],Ip[0][1],Ip[0][2],Ip[0][3]);
			sprintf((void *)&pOutStream[strlen(pOutStream)],"Mask:%u.%u.%u.%u\n\r",			Ip[1][0],Ip[1][1],Ip[1][2],Ip[1][3]);
			sprintf((void *)&pOutStream[strlen(pOutStream)],"Gataway:%u.%u.%u.%u\n\r",		Ip[2][0],Ip[2][1],Ip[2][2],Ip[2][3]);		
			sprintf((void *)&pOutStream[strlen(pOutStream)],"Dns:%u.%u.%u.%u\n\r",			Ip[3][0],Ip[3][1],Ip[3][2],Ip[3][3]);
			sprintf((void *)&pOutStream[strlen(pOutStream)],"Dns2:%u.%u.%u.%u\n\r",			Ip[4][0],Ip[4][1],Ip[4][2],Ip[4][3]);
			sprintf((void *)&pOutStream[strlen(pOutStream)],"Srv:%s\n\r",URL);

			if(NeedFree)
			{
				CDebug(pOutStream);
				Q_Free(pOutStream);
			}
			
			return TRUE;
		}	
		else
		{
			if(strcmp((void *)pCmd[1],"del")==0)
			{
				DeleteTcpSrvConn();
				DeleteJsonConn();
				return TRUE;
			}
			else
			{				
				char *p1=strchr((void *)&pStrCopy[4],'.');
				char *p2=strchr((void *)&pStrCopy[4],' ');
				if(p1==NULL || p2!=NULL)
				{
					Debug("srv addr error!\n\r");
					return TRUE;
				}
				
				QDB_SetStr(SDN_SYS,SIN_ServerURL,(void *)&pStrCopy[4]);				
				QDB_BurnToSpiFlash(SDN_SYS);
				return TRUE;
			}
		}
	}
	else if(strcmp((void *)pCmd[0],"sleep")==0)
	{
		system_deep_sleep(Str2Uint(pCmd[1])*1000*1000);
		return TRUE;
	}
	else if(strcmp((void *)pCmd[0],"strnum")==0)
	{
		s32 Num;
		const char *p=FindNumFromStr(pCmd[1],&Num);

		if(p)	Debug("num:%d|%s\n\r",Num,p);
		else Debug("num:%d|null\n\r",Num);
		return TRUE;
	}
	
	return FALSE;
}

static bool __inline SysCmdHandler_T(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"test")==0)
	{
		//QDB_SetValue(SDN_SYS,SIN_LcdVarsName,0,pCmd[1],strlen(pCmd[1]));		
		u16 Idx=Str2Uint(pCmd[1]);
		spi_flash_erase_sector(Idx);
		return TRUE;
	}

	return FALSE;
}

static bool __inline SysCmdHandler_U(char **pCmd,const char *pStrCopy,char *pOutStream)
{}

static bool __inline SysCmdHandler_V(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"var")==0)
	{		
		if(NotNullStr(pCmd[1]))
		{
			if(NotNullStr(pCmd[2]))//set
			{
				u16 Vid=Str2Uint(pCmd[1]);
				TVAR32 Val=Str2Sint(pCmd[2]);

				if(SetVarVal(Vid,Val,VRT_SYS,0))
				{
					Debug("SetVar[%u]=%d(0x%x)!\n\r",Vid,Val,Val);
				}
				else
				{
					Debug("SetVar[%u] Faild!\n\r",Vid);
				}				
			}
			else //read
			{
				u16 Vid=Str2Uint(pCmd[1]);
				TVAR32 Val=0;
				VAR_STATE State=GetVarState(Vid,&Val);

				if(State==VST_VALID)
					Debug("Var[%u]=%d(0x%x)\n\r",Vid,Val,Val);
				else
					Debug("Var[%u]=%d(0x%x), NoVaild\n\r",Vid,Val,Val);
			}
		}
		return TRUE;
	}

	return FALSE;
}

static bool __inline SysCmdHandler_W(char **pCmd,const char *pStrCopy,char *pOutStream)
{
	if(strcmp((void *)pCmd[0],"wconn")==0)
	{
		if(IsNullStr(pCmd[1]) || Str2Uint(pCmd[1])==1)
		{
			wifi_station_connect();
			CDebug("connect wifi ap\n\r");
		}
		else
		{
			wifi_station_disconnect();
			CDebug("disconnect wifi ap\n\r");
		}

		return TRUE;
	}
	

	return FALSE;
}

static bool __inline SysCmdHandler_X(char **pCmd,const char *pStrCopy,char *pOutStream)
{return FALSE;}

static bool __inline SysCmdHandler_Y(char **pCmd,const char *pStrCopy,char *pOutStream)
{return FALSE;}

static bool __inline SysCmdHandler_Z(char **pCmd,const char *pStrCopy,char *pOutStream)
{return FALSE;}

bool CustomCmdHandler(char **pCmd,const char *pStrCopy,char *pOutStream);

typedef bool (*pCmdHandler)(char **,const char *,char *);
const pCmdHandler gpCmdHandlers[]={
SysCmdHandler_A,SysCmdHandler_B,SysCmdHandler_C,SysCmdHandler_D,SysCmdHandler_E,SysCmdHandler_F,
SysCmdHandler_G,SysCmdHandler_H,SysCmdHandler_I,SysCmdHandler_J,SysCmdHandler_K,SysCmdHandler_L,
SysCmdHandler_M,SysCmdHandler_N,SysCmdHandler_O,SysCmdHandler_P,SysCmdHandler_Q,SysCmdHandler_R,
SysCmdHandler_S,SysCmdHandler_T,SysCmdHandler_U,SysCmdHandler_V,SysCmdHandler_W,SysCmdHandler_X,
SysCmdHandler_Y,SysCmdHandler_Z
};

#define UART_CMD_MAX_PARAM_NUM 6//最长参数

//通用的串口输入处理
//Len 字符串个数
//pStr 字符串
//将处理如下命令:
bool SysCmdHandler(u16 Len,const char *pStr,char *pOutStream)
{
	char *pParam[UART_CMD_MAX_PARAM_NUM+1];
	char *pBuf=NULL;
	u16 i,n;
	bool Res=FALSE;
	char FirstByts=0;
	
	if(Len==0)//控制字符
	{
		if(((u16 *)pStr)[0]==0x445b)
		{

		}
		else if(((u16 *)pStr)[0]==0x435b)
		{

		}
		else if((((u16 *)pStr)[0]==0x5f00)||(((u16 *)pStr)[0]==0x0))
		{
			CDebug("\r\n");
		}
		else
		{
			CDebug("\r\n");
			//CDebug("CtrlCode:%x\r\n",((u16 *)pStr)[0]);
		}	
		return Res;
	}

	for(i=0;i<(UART_CMD_MAX_PARAM_NUM+1);i++) pParam[i]=NULL;

	pBuf=Q_Malloc(Len+2);
	StrCmdParse(pStr,pParam,pBuf,TRUE);//解析指令和参数
	CDebug("\r\n");
	
	FirstByts=pParam[0][0];
	if(FirstByts=='#') Res=CustomCmdHandler(pParam,pStr,pOutStream);
	else if(FirstByts>='a' && FirstByts<='z') Res=gpCmdHandlers[FirstByts-'a'](pParam,pStr,pOutStream);
	else Res=FALSE;
	
	if(Res==FALSE) 
	{
		if(FirstByts=='#') UDebug("#err\r");
		else CDebug("No Such Cmd[%u]:%s\r\n",Len,pBuf);
	}
	
	Q_Free(pBuf);
	
	return Res;
}

