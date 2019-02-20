#include "SysDefines.h"

static u8 gAuthNum[16]={0};//登陆授权序列号
static char http_200_html[]  = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\nConnection: close\r\n\r\n";

bool http_get_user_val_wifi(const char *pVarStr,char *pOutStr)
{
	if(strcmp(pVarStr,"wifi_ssid")==0)
	{
		struct station_config *pConfig=Q_Zalloc(sizeof(struct station_config));
		wifi_station_get_config_default(pConfig);
		sprintf(pOutStr,"%s",pConfig->ssid);
		Q_Free(pConfig);
		return TRUE;
	}
	else if(strcmp(pVarStr,"wifi_ssid_v")==0)
	{
		struct station_config *pConfig=Q_Zalloc(ESP_AP_RECORD_NUM*sizeof(struct station_config));
		u16 ap_id=wifi_station_get_current_ap_id();
		u16 i;
		
		wifi_station_get_ap_info(pConfig);
		for(i=0;i<ESP_AP_RECORD_NUM;i++)
		{
			if(i==ap_id)
			{
				sprintf(pOutStr,"%s",pConfig[i].ssid);
				break;
			}
		}		
		Q_Free(pConfig);
		return TRUE;
	}
	else if(strcmp(pVarStr,"wifi_rssi")==0)
	{	
		sprintf(pOutStr,"%d",wifi_station_get_rssi());
		return TRUE;
	}
	else if(strcmp(pVarStr,"wifi_status")==0)
	{	
		sprintf(pOutStr,"%s",gNameStationStatus[wifi_station_get_connect_status()]);
		return TRUE;
	}
	else if(strcmp(pVarStr,"wifi_ip")==0)
	{
		struct ip_info ip_config;	
		wifi_get_ip_info(STATION_IF, &ip_config);
		sprintf(pOutStr,IPSTR,IP2STR(&(ip_config.ip.addr)));
		return TRUE;
	}
	else if(strcmp(pVarStr,"wifi_mask")==0)
	{
		struct ip_info ip_config;	
		wifi_get_ip_info(STATION_IF, &ip_config);
		sprintf(pOutStr,IPSTR,IP2STR(&(ip_config.netmask.addr)));
		return TRUE;
	}
	else if(strcmp(pVarStr,"wifi_gw")==0)
	{
		struct ip_info ip_config;	
		wifi_get_ip_info(STATION_IF, &ip_config);
		sprintf(pOutStr,IPSTR,IP2STR(&(ip_config.gw.addr)));
		return TRUE;
	}
	else if(strcmp(pVarStr,"wifi_dns1")==0)
	{
		IP_ADDR Dns1=dns_getserver(0);
		sprintf(pOutStr,"%u.%u.%u.%u",DipIpAddr(Dns1.addr));
		return TRUE;
	}
	else if(strcmp(pVarStr,"wifi_dns2")==0)
	{
		IP_ADDR Dns2=dns_getserver(1);
		sprintf(pOutStr,"%u.%u.%u.%u",DipIpAddr(Dns2.addr));
		return TRUE;
	}
	
	return FALSE;
}

bool http_get_user_val_ap(const char *pVarStr,char *pOutStr)
{
	if(strcmp(pVarStr,"ap_ssid")==0)
	{
		struct softap_config *pConfig = (struct softap_config *)Q_Zalloc(sizeof(struct softap_config)); // initialization
		wifi_softap_get_config(pConfig);
		sprintf(pOutStr,"%s",pConfig->ssid);
		//Debug("Ap:%s:%s, %s\n\r",pConfig->ssid,pConfig->password,gNameAuthMode[pConfig->authmode]);
		Q_Free(pConfig);
		return TRUE;
	}
	else if(strcmp(pVarStr,"ap_pw")==0)
	{
		struct softap_config *pConfig = (struct softap_config *)Q_Zalloc(sizeof(struct softap_config)); // initialization
		wifi_softap_get_config(pConfig);
		sprintf(pOutStr,"%s",pConfig->password);
		//Debug("Ap:%s:%s, %s\n\r",pConfig->ssid,pConfig->password,gNameAuthMode[pConfig->authmode]);
		Q_Free(pConfig);
		return TRUE;
	}
	else if(strcmp(pVarStr,"ap_dev_num")==0)
	{
		sprintf(pOutStr,"%u",wifi_softap_get_station_num());
		return TRUE;
	}
	else if(strcmp(pVarStr,"ap_ip")==0)
	{
		struct ip_info ip_config;	
		wifi_get_ip_info(SOFTAP_IF,&ip_config);
		sprintf(pOutStr,IPSTR,IP2STR(&(ip_config.ip.addr)));
		return TRUE;
	}
	
	return FALSE;
}

bool http_get_user_val_gpio(const char *pVarStr,char *pOutStr)
{
	if(strcmp(pVarStr,"gpio_conf")==0)//io5 -io11的配置序列化
	{
		WE_IO_CONFIG IoConf;
		u16 Pin,n=0;

		for(Pin=0;Pin<=16;Pin++)
		{
			if(Pin==2) Pin=4;//跳过2 3
			if(Pin==6) Pin=12;//跳过6 7 8 9 10 11		
			
			QDB_GetValue(SDN_HWC,HWC_IO_CONF,Pin,&IoConf);
			sprintf(&pOutStr[n],"c%u=",Pin);
			n+=((Pin>=10)?4:3);
			pOutStr[n++]='0'+(IoConf.IoMode);
			pOutStr[n++]=IoConf.Pullup?'1':'0';
			pOutStr[n++]='&';
		}
		
		if(n) pOutStr[n-1]=0;
		
		return TRUE;
	}
	
	return FALSE;
}

bool http_get_user_val_vars(const char *pVarStr,char *pOutStr)
{
	if(strcmp(pVarStr,"vars_conf")==0)//io5 -io11的配置序列化
	{
		WE_IO_CONFIG IoConf;
		u16 Pin,n=0;

		for(Pin=0;Pin<=16;Pin++)
		{
			if(Pin==2) Pin=4;//跳过2 3
			if(Pin==6) Pin=12;//跳过6 7 8 9 10 11		
			
			QDB_GetValue(SDN_HWC,HWC_IO_CONF,Pin,&IoConf);
			sprintf(&pOutStr[n],"c%u=",Pin);
			n+=((Pin>=10)?4:3);
			pOutStr[n++]='0'+(IoConf.IoMode);
			pOutStr[n++]='0'+(IoConf.VarIoMth);
			if(IoConf.Bit<10) pOutStr[n++]='0'+IoConf.Bit;
			else pOutStr[n++]='a'+IoConf.Bit-10;
			pOutStr[n++]='&';
		}

		if(n) pOutStr[n-1]=0;
		
		return TRUE;
	}
	else if(strcmp(pVarStr,"var_ptag")==0)
	{
		WE_IO_CONFIG IoConf;
		u16 Pin,n=0;

		for(Pin=0;Pin<=16;Pin++)
		{
			u8 Buf[VAR_TAG_LEN+2];
			
			if(Pin==2) Pin=4;//跳过2 3
			if(Pin==6) Pin=12;//跳过6 7 8 9 10 11		
			
			QDB_GetValue(SDN_HWC,HWC_IO_CONF,Pin,&IoConf);
			if(IoConf.VarID && GetVarProdTag(IoConf.VarID,Buf,NULL))
			{
				Buf[4]=0;
				sprintf(&pOutStr[n],"p%u=%s",Pin,Buf);
			}
			else
			{
				sprintf(&pOutStr[n],"p%u=NULL",Pin);
			}
			n+=strlen(&pOutStr[n]);
			pOutStr[n++]='&';
		}

		if(n) pOutStr[n-1]=0;
		
		return TRUE;
	}
	else if(strcmp(pVarStr,"var_vtag")==0)
	{
		WE_IO_CONFIG IoConf;
		u16 Pin,n=0;

		for(Pin=0;Pin<=16;Pin++)
		{
			u8 Buf[VAR_TAG_LEN+2];
			
			if(Pin==2) Pin=4;//跳过2 3
			if(Pin==6) Pin=12;//跳过6 7 8 9 10 11		
			
			QDB_GetValue(SDN_HWC,HWC_IO_CONF,Pin,&IoConf);
			if(IoConf.VarID && GetVarProdTag(IoConf.VarID,NULL,Buf))
			{
				Buf[4]=0;
				sprintf(&pOutStr[n],"v%u=%s",Pin,Buf);
			}
			else
			{
				sprintf(&pOutStr[n],"v%u=NULL",Pin);
			}
			n+=strlen(&pOutStr[n]);
			pOutStr[n++]='&';
		}

		if(n) pOutStr[n-1]=0;
		
		return TRUE;
	}	
	return FALSE;
}

bool http_get_user_val_misc(const char *pVarStr,char *pOutStr)
{
	if(strcmp(pVarStr,"misc_debug_en")==0)
	{
		sprintf(pOutStr,"%u",0);
		return TRUE;
	}
	else if(strcmp(pVarStr,"misc_pwm_period")==0)
	{
		sprintf(pOutStr,"%u",QDB_GetNum(SDN_HWC,HWC_PWM_PERIOD)/100);
		return TRUE;
	}
	else if(strcmp(pVarStr,"misc_pwm_duty")==0)
	{
		sprintf(pOutStr,"%u",QDB_GetNum(SDN_HWC,HWC_PWM_DUTY));
		return TRUE;
	}
	else if(strcmp(pVarStr,"misc_adc_en")==0)
	{
		WE_AIN_CONFIG AinConfig;
		QDB_GetValue(SDN_HWC,HWC_ADC_CONF,0,&AinConfig);
		sprintf(pOutStr,"%u",AinConfig.AdcMode);
		return TRUE;
	}
	else if(strcmp(pVarStr,"misc_adc_tole")==0)
	{
		WE_AIN_CONFIG AinConfig;
		QDB_GetValue(SDN_HWC,HWC_ADC_CONF,0,&AinConfig);
		sprintf(pOutStr,"%u",AinConfig.Tolerance);	
		return TRUE;
	}
	else if(strcmp(pVarStr,"misc_adc_factor")==0)
	{
		WE_AIN_CONFIG AinConfig;
		QDB_GetValue(SDN_HWC,HWC_ADC_CONF,0,&AinConfig);
		sprintf(pOutStr,"%u",AinConfig.Factor);		
		return TRUE;
	}
	else if(strcmp(pVarStr,"misc_adc_offset")==0)
	{
		WE_AIN_CONFIG AinConfig;
		QDB_GetValue(SDN_HWC,HWC_ADC_CONF,0,&AinConfig);
		sprintf(pOutStr,"%d",AinConfig.Offset);		
		return TRUE;
	}
	else if(strcmp(pVarStr,"misc_adc_p")==0)
	{
		WE_AIN_CONFIG AinConfig;
		u8 Buf[VAR_TAG_LEN];
		
		QDB_GetValue(SDN_HWC,HWC_ADC_CONF,0,&AinConfig);		
		if(AinConfig.VarID && GetVarProdTag(AinConfig.VarID,Buf,NULL))
		{
			Buf[4]=0;
			sprintf(pOutStr,"%s",Buf);		
		}
		else
		{
			sprintf(pOutStr,"NULL");
		}	
		return TRUE;
	}
	else if(strcmp(pVarStr,"misc_adc_v")==0)
	{
		WE_AIN_CONFIG AinConfig;
		u8 Buf[VAR_TAG_LEN];
		
		QDB_GetValue(SDN_HWC,HWC_ADC_CONF,0,&AinConfig);		
		if(AinConfig.VarID && GetVarProdTag(AinConfig.VarID,NULL,Buf))
		{
			Buf[4]=0;
			sprintf(pOutStr,"%s",Buf);		
		}
		else
		{
			sprintf(pOutStr,"NULL");
		}	
		return TRUE;
	}
	else if(strcmp(pVarStr,"misc_adc_period")==0)
	{
		WE_AIN_CONFIG AinConfig;
		QDB_GetValue(SDN_HWC,HWC_ADC_CONF,0,&AinConfig);
		sprintf(pOutStr,"%u",AinConfig.PeriodIdx);		
		return TRUE;
	}
	
	return FALSE;
}

//获取内部参数值
bool http_get_user_val(const char *pVarStr,char *pOutStr)
{
	u16 Code=HL8_U16(pVarStr[0],pVarStr[1]);
	
	//Debug("get val %s\r\n",pVarStr);
	switch(Code)
	{
		case HL8_U16('w','i'): return http_get_user_val_wifi(pVarStr,pOutStr);
		case HL8_U16('a','p'): return http_get_user_val_ap(pVarStr,pOutStr);
		case HL8_U16('g','p'): return http_get_user_val_gpio(pVarStr,pOutStr);
		case HL8_U16('v','a'): return http_get_user_val_vars(pVarStr,pOutStr);
		case HL8_U16('m','i'): return http_get_user_val_misc(pVarStr,pOutStr);
		
		default:
		{
			if(strcmp(pVarStr,"auth_num")==0)
			{
				sprintf(pOutStr,"%s",gAuthNum);
				return TRUE;
			}
			else if(strcmp(pVarStr,"reg_sn")==0)//板卡注册密钥
			{
				u32 DutRegSn=QDB_GetNum(SDN_SYS,SIN_DutRegSn);
				if(DutRegSn) sprintf(pOutStr,"%u",DutRegSn);
				else pOutStr[0]=0;
				return TRUE;
			}
			else if(strcmp(pVarStr,"reg_sn_dis")==0)//板卡注册密钥显示开关
			{
				if(QDB_GetNum(SDN_SYS,SIN_DutRegSn))
				{
					sprintf(pOutStr,"disabled");
				}
				else
				{
					sprintf(pOutStr,"xxx");
				}
				return TRUE;
			}
			


			
		}
	}
	
	return FALSE;
}

//用户自定义的文件路径的获取
bool http_get_user_handler(NET_CONN_T *pConn,const char *pUrl)
{
	if(strcmp(pUrl,"/ret?res=ok")==0)
	{
		netconn_write(pConn,http_200_html,sizeof(http_200_html)-1,NETCONN_NOCOPY);	
		netconn_write(pConn,"ok",2,NETCONN_NOCOPY);//直接输出
		return TRUE;
	}

	return FALSE;
}

//处理用户提交的表单
const char *http_post_user_handler(const char *pUrl,u16 ParamNum,const char **pParam,const char **pVal)
{
	const char *pRetUrl="";
	u16 i;

	if(pUrl==NULL || pParam==NULL || pVal==NULL) return;
	
	//全局
	for(i=0;i<ParamNum;i++)
	{
		if(pParam[i]==NULL || pVal[i]==NULL) continue;
		
		Debug("[%u]%s:%s\n\r",i,pParam[i],pVal[i]);//参数打印调试

		if(strcmp(pParam[i],"test")==0)
		{
	
		}
	}

	//分页面
	if(strcmp(pUrl,"/login.set")==0)//4login
	{
		const char *pPw=NULL;
		const char *pAuthNum=NULL;
		
		//参数分析
		for(i=0;i<ParamNum;i++)
		{		
			if(pParam[i]==NULL || pVal[i]==NULL) continue;
			if(strcmp(pParam[i],"pw")==0){ pPw=pVal[i];}
			else if(strcmp(pParam[i],"auth_num")==0){ pAuthNum=pVal[i];}
			else if(strcmp(pParam[i],"reg_sn")==0)//设定板卡注册密钥
			{
				u32 Key=Str2Uint(pVal[i]);
				if(Key==GetDutRegSn())
				{
					if(Key!=QDB_GetNum(SDN_SYS,SIN_DutRegSn))
					{
						QDB_SetNum(SDN_SYS,SIN_DutRegSn,Key);
						QDB_BurnToSpiFlash(SDN_SYS);
					}
					SysVars()->DutRegSnIsOk=TRUE;
				}
				else
				{
					return "/?tip=37";
				}
			}
		}			

		//逻辑处理
		if(pPw!=NULL && pAuthNum!=NULL)
		{
			u32 Hash=MakeHash33((void *)pPw,strlen(pPw));
			if(Hash==QDB_GetNum(SDN_SYS,SIN_UserPwHash))//密码对比
			{
				strncpy(gAuthNum,pAuthNum,sizeof(gAuthNum));
				gAuthNum[sizeof(gAuthNum)-1]=0;
				return "/wifi";
			}
		}

		return "/?tip=31";
	}
	else if(strcmp(pUrl,"/wifi.set")==0)//4wifi
	{
		const char *pSsid=NULL;
		const char *pPw=NULL;

		//参数分析
		for(i=0;i<ParamNum;i++)
		{		
			if(pParam[i]==NULL || pVal[i]==NULL) continue;
			if(strcmp(pParam[i],"wifi_ssid")==0) pSsid=pVal[i];
			else if(strcmp(pParam[i],"wifi_pw")==0) pPw=pVal[i];
		}

		//逻辑处理
		if(pSsid!=NULL && pPw!=NULL)
		{
			struct station_config *pStaConfig=Q_Zalloc(sizeof(struct station_config));
			sprintf(pStaConfig->ssid,pSsid);
			sprintf(pStaConfig->password,pPw);	
			wifi_station_set_config(pStaConfig);
			Q_Free(pStaConfig);
			return "/wifi?tip=32";
		}	

		return "/wifi?tip=2";
	}
	else if(strcmp(pUrl,"/ap.set")==0)//4ap
	{
		const char *pSsid=NULL;
		const char *pPw=NULL;

		//参数分析
		for(i=0;i<ParamNum;i++)
		{		
			if(pParam[i]==NULL || pVal[i]==NULL) continue;
			if(strcmp(pParam[i],"ap_ssid")==0) pSsid=pVal[i];
			else if(strcmp(pParam[i],"ap_pw")==0) pPw=pVal[i];
		}

		//逻辑处理
		if(pSsid!=NULL && pPw!=NULL)
		{
			struct softap_config *ap_config = Q_ZallocAsyn(sizeof(struct softap_config)); // initialization
			wifi_softap_get_config(ap_config); // Get soft-AP ap_config first.
			sprintf(ap_config->ssid, pSsid);
			sprintf(ap_config->password, pPw);
			ap_config->authmode = AUTH_WPA_WPA2_PSK;
			ap_config->ssid_len = 0; // or its actual SSID length
			ap_config->max_connection = 4;
			SysEventMsSend(3*1000,SEN_SET_AP,0,ap_config,MFM_ALWAYS);
			return "/msg?tip=32&btn=11&url=%2F";//连接已经断开
		}	

		return "/ap?tip=2";
	}
	else if(strcmp(pUrl,"/gpio.set")==0)//4gpio
	{
		bool NeedBurn=FALSE;
		
		//参数分析
		for(i=0;i<ParamNum;i++)
		{		
			if(pParam[i]==NULL || pVal[i]==NULL) continue;
			if(pParam[i][0]=='c')//"cxx_x=xxx"
			{
				WE_IO_CONFIG IoConf;
				const char *pOptStr=NULL;
				u32 Pin=0;
				u32 InputIdx=0;
				u32 Val=0;
				
				pOptStr=pParam[i];
				Pin=Str2Uint(&pOptStr[1]);
				if(Pin>=2 && Pin<=3) continue;
				if(Pin>=6 && Pin<=11) continue;
				if(Pin>16) continue;
				
				InputIdx=Str2Uint(&pOptStr[4]);
				Val=Str2Uint(pVal[i]);
				
				QDB_GetValue(SDN_HWC,HWC_IO_CONF,Pin,&IoConf);
				if(InputIdx==1) IoConf.IoMode=Val;
				else if(InputIdx==2) IoConf.Pullup=Val?TRUE:FALSE;
				
				QDB_SetValue(SDN_HWC,HWC_IO_CONF,Pin,&IoConf,sizeof(IoConf));
				NeedBurn=TRUE;
			}			
		}

		if(NeedBurn) QDB_BurnToSpiFlash(SDN_HWC);
		return "/msg?tip=33&btn=12&url=%2Fgpio";
	}
	else if(strcmp(pUrl,"/vars.set")==0)//4vars
	{
		bool NeedBurn=FALSE;
		
		//参数分析
		for(i=0;i<ParamNum;i++)
		{		
			if(pParam[i]==NULL || pVal[i]==NULL) continue;
			if(pParam[i][0]=='c')//"cxx_x=xxx"
			{
				WE_IO_CONFIG IoConf;
				const char *pOptStr=NULL;
				u32 Pin=0;
				u32 InputIdx=0;
				u32 Val=0;
				
				pOptStr=pParam[i];//cxx_x=xxx，单个的选项序列
				Pin=Str2Uint(&pOptStr[1]);//从字符串中获取pin值
				if(Pin>=2 && Pin<=3) continue; //跳过2 3
				if(Pin>=6 && Pin<=11) continue; //跳过6 7 8 9 10 11
				if(Pin>16) continue;
				
				InputIdx=Str2Uint(&pOptStr[4]);//input序号
				Val=Str2Uint(pVal[i]);
				
				QDB_GetValue(SDN_HWC,HWC_IO_CONF,Pin,&IoConf);
				if(InputIdx==2) IoConf.VarIoMth=Val;
				else if(InputIdx==3) IoConf.Bit=Val;
				
				QDB_SetValue(SDN_HWC,HWC_IO_CONF,Pin,&IoConf,sizeof(IoConf));
				NeedBurn=TRUE;
			}		
			else if(pParam[i][0]=='t')//"tx=xxxxxxxx"
			{
				WE_IO_CONFIG IoConf;
				const char *pOptStr=NULL;
				u32 Pin=0;
				const char *pTagStr=pVal[i];

				pOptStr=pParam[i];//tx_x=xxxxxxxx，单个的选项序列
				Pin=Str2Uint(&pOptStr[1]);//从字符串中获取pin值
				if(Pin>=2 && Pin<=3) continue; //跳过2 3
				if(Pin>=6 && Pin<=11) continue; //跳过6 7 8 9 10 11
				if(Pin>16) continue;

				if(strlen(pTagStr)==VAR_TAG_LEN*2)
				{
					u16 Vid=FindVarId_ByTag(pTagStr);

					if(Vid)
					{
						QDB_GetValue(SDN_HWC,HWC_IO_CONF,Pin,&IoConf);
						IoConf.VarID=Vid;
						QDB_SetValue(SDN_HWC,HWC_IO_CONF,Pin,&IoConf,sizeof(IoConf));
						NeedBurn=TRUE;
					}
				}
			}
		}

		if(NeedBurn) QDB_BurnToSpiFlash(SDN_HWC);
		return "/msg?tip=34&btn=12&url=%2Fvars";
	}
	else if(strcmp(pUrl,"/misc.set")==0)//4misc
	{
		bool NeedBurn=FALSE;
		
		//参数分析
		for(i=0;i<ParamNum;i++)
		{		
			if(pParam[i]==NULL || pVal[i]==NULL) continue;
			if(strcmp(pParam[i],"pwm_period")==0)
			{
				u32 PwmPeriod=Str2Uint(pVal[i]);
				QDB_SetNum(SDN_HWC,HWC_PWM_PERIOD,PwmPeriod*100);
				NeedBurn=TRUE;				
			}
			else if(strcmp(pParam[i],"pwm_duty")==0)
			{
				u32 PwmDuty=Str2Uint(pVal[i]);
				QDB_SetNum(SDN_HWC,HWC_PWM_DUTY,PwmDuty);
				NeedBurn=TRUE;				
			}
			else if(strcmp(pParam[i],"adc_en")==0)
			{
				WE_AIN_CONFIG AinConfig;				
				QDB_GetValue(SDN_HWC,HWC_ADC_CONF,0,&AinConfig);
				AinConfig.AdcMode=Str2Uint(pVal[i]);
				QDB_SetValue(SDN_HWC,HWC_ADC_CONF,0,&AinConfig,sizeof(WE_AIN_CONFIG));

				NeedBurn=TRUE;	
			}
			else if(strcmp(pParam[i],"adc_tole")==0)
			{
				WE_AIN_CONFIG AinConfig;				
				QDB_GetValue(SDN_HWC,HWC_ADC_CONF,0,&AinConfig);
				AinConfig.Tolerance=Str2Uint(pVal[i]);
				QDB_SetValue(SDN_HWC,HWC_ADC_CONF,0,&AinConfig,sizeof(WE_AIN_CONFIG));

				NeedBurn=TRUE;	
			}
			else if(strcmp(pParam[i],"adc_factor")==0)
			{
				WE_AIN_CONFIG AinConfig;				
				QDB_GetValue(SDN_HWC,HWC_ADC_CONF,0,&AinConfig);
				AinConfig.Factor=Str2Uint(pVal[i]);
				QDB_SetValue(SDN_HWC,HWC_ADC_CONF,0,&AinConfig,sizeof(WE_AIN_CONFIG));

				NeedBurn=TRUE;	
			}
			else if(strcmp(pParam[i],"adc_offset")==0)
			{
				WE_AIN_CONFIG AinConfig;				
				QDB_GetValue(SDN_HWC,HWC_ADC_CONF,0,&AinConfig);
				AinConfig.Offset=Str2Sint(pVal[i]);
				QDB_SetValue(SDN_HWC,HWC_ADC_CONF,0,&AinConfig,sizeof(WE_AIN_CONFIG));

				NeedBurn=TRUE;	
			}
			else if(strcmp(pParam[i],"adc_var_tags")==0)
			{
				WE_AIN_CONFIG AinConfig;				
				const char *pTags=pVal[i];
				
				QDB_GetValue(SDN_HWC,HWC_ADC_CONF,0,&AinConfig);

				if(strlen(pTags)==VAR_TAG_LEN*2)
				{
					AinConfig.VarID=FindVarId_ByTag(pTags);
					QDB_SetValue(SDN_HWC,HWC_ADC_CONF,0,&AinConfig,sizeof(WE_AIN_CONFIG));
				
					NeedBurn=TRUE;	
				}
			}
			else if(strcmp(pParam[i],"adc_period")==0)
			{
				WE_AIN_CONFIG AinConfig;				
				QDB_GetValue(SDN_HWC,HWC_ADC_CONF,0,&AinConfig);
				AinConfig.PeriodIdx=Str2Uint(pVal[i]);
				QDB_SetValue(SDN_HWC,HWC_ADC_CONF,0,&AinConfig,sizeof(WE_AIN_CONFIG));

				NeedBurn=TRUE;	
			}			
		}

		if(NeedBurn) QDB_BurnToSpiFlash(SDN_HWC);
		return "/msg?tip=35&btn=12&url=%2Fmisc";
	}
	else if(strcmp(pUrl,"/reboot.set")==0)//4reset
	{
		Debug("Web Need Dut Reboot!\n\r");
		SysEventMsSend(3000,SEN_CB_FUNC,0,RebootBoard,MFM_COVER);
		return "";
	}
	else if(strcmp(pUrl,"/open.set")==0)//4open
	{
		for(i=0;i<ParamNum;i++)
		{		
			if(pParam[i]==NULL || pVal[i]==NULL) continue;
			if(strcmp(pParam[i],"op")==0)
			{
				u32 OpVal=Str2Uint(pVal[i]);
				if(OpVal==1)
				{
					Debug("OpenLock\n\r");
					//SendLockCmd(0x24);//开锁
				}
			}
		}		
		
		return "/ret?res=ok";
	}




	
	return pRetUrl;
}

