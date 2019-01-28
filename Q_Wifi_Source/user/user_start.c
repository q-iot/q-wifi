#include "SysDefines.h"
#include "AdcHandler.h"
#include "VarDisplay.h"
#include "GpioHwHandler.h"

#undef CDebug
#define CDebug Debug

void DispWifiStation(void)
{
	u16 ap_id=wifi_station_get_current_ap_id();
	struct station_config *pConfig=Q_Malloc(ESP_AP_RECORD_NUM*sizeof(struct station_config));
	struct ip_info ip_config;	
	u16 i;
	
	wifi_station_get_ap_info(pConfig);

	CDebug("Status: %s[%d]\n\r",gNameStationStatus[wifi_station_get_connect_status()],wifi_station_get_rssi());

	wifi_get_ip_info(STATION_IF, &ip_config);
	CDebug("Ip:"IPSTR,IP2STR(&(ip_config.ip.addr)));
	CDebug(", mask:"IPSTR,IP2STR(&(ip_config.netmask.addr)));
	CDebug(", gw:"IPSTR"\n\r",IP2STR(&(ip_config.gw.addr)));

	{
		IP_ADDR Dns1=dns_getserver(0);
		IP_ADDR Dns2=dns_getserver(1);
		CDebug("DNS1:%u.%u.%u.%u\n\r",DipIpAddr(Dns1.addr));
		CDebug("DNS2:%u.%u.%u.%u\n\r",DipIpAddr(Dns2.addr));
	}
	
	CDebug("Ap record:\n\r");
	for(i=0;i<ESP_AP_RECORD_NUM;i++)
	{
		CDebug("[%d]%s:%s",i,pConfig[i].ssid,pConfig[i].password);
		if(pConfig[i].bssid_set) CDebug(MACSTR,pConfig[i].bssid);
		if(i==ap_id) CDebug(" *");
		CDebug("\n\r");
	}

	//��ǰ��
	wifi_station_get_config_default(pConfig);
	CDebug("[Current]%s:%s",pConfig[0].ssid,pConfig[0].password);
	if(pConfig[0].bssid_set) CDebug(MACSTR,pConfig[0].bssid);
	CDebug("\n\r");	
	
	Q_Free(pConfig);
}

void DispWifiAp(void)
{
	struct station_info * station = wifi_softap_get_station_info();
	struct softap_config *ap_config = (struct softap_config *)Q_Malloc(sizeof(struct softap_config)); // initialization
	struct ip_info ip_config;	
		
	wifi_softap_get_config(ap_config); // Get soft-AP ap_config first.
	CDebug("Ap:%s:%s, %s\n\r",ap_config->ssid,ap_config->password,gNameAuthMode[ap_config->authmode]);
	Q_Free(ap_config);

	wifi_get_ip_info(SOFTAP_IF, &ip_config);
	CDebug("Ip:"IPSTR,IP2STR(&(ip_config.ip.addr)));
	CDebug(", mask:"IPSTR,IP2STR(&(ip_config.netmask.addr)));
	CDebug(", gw:"IPSTR"\n\r",IP2STR(&(ip_config.gw.addr)));

	CDebug("Has %d station connecting:\n\r",wifi_softap_get_station_num());
	while(station)
	{
		CDebug("mac:"MACSTR", ip:"IPSTR"\n\r",MAC2STR(station->bssid), IP2STR(&station->ip));
		station = STAILQ_NEXT(station, next);
	}
	wifi_softap_free_station_info(); // Free it by calling functions
}

//����ʾ��
void Timer1_Cb(void *pTimer)
{
	Q_Free(pTimer);
}

//json������ص�ʾ��
void JsonSent_Cb(int Len,void *pData)
{
	//Debug("JsonSent[%d]:\r\n",Len);
	//if(Len>0) Debug(pData);
}

//�޸�apΪqiot
static void WifiConfig_ModifyApSsid(void)
{
	struct softap_config ApConfig;
	u8 SsidBuf[20];

	sprintf(SsidBuf,"Q-IOT-%u",GetHwID());
	
	wifi_softap_get_config_default(&ApConfig);

	if(strcmp(ApConfig.ssid,SsidBuf)==0 && ApConfig.ssid_len==strlen(SsidBuf))//ssidһ��
	{
		Debug("AP SSID is %s\n\r",SsidBuf);
	}
	else//ssid��һ��
	{
		Debug("Change AP SSID to %s\n\r",SsidBuf);
		strcpy(ApConfig.ssid,SsidBuf);
		ApConfig.ssid_len=strlen(ApConfig.ssid);
		wifi_softap_set_config(&ApConfig);
	}	
}

static void WifiConfig_Task(void *pvParameters)
{
	u16 i;
	struct ip_info ip_config;	

	WifiConfig_ModifyApSsid();//�޸�Ĭ�ϵ�ssid

	wifi_get_ip_info(SOFTAP_IF, &ip_config);//�ж�ap��û������
	while(ip_config.ip.addr == 0){
		OS_TaskDelaySec(1);
		wifi_get_ip_info(SOFTAP_IF, &ip_config);
	}
	Debug("Ap ok! ip:"IPSTR"\n\r",IP2STR(&(ip_config.ip.addr)));

	http_server_init();
	TcpServer_Init();

	UserAppInitHook();//send rdy

	wifi_get_ip_info(STATION_IF, &ip_config);//�ж�station��û������
	while(ip_config.ip.addr == 0){
		OS_TaskDelaySec(1);
		wifi_get_ip_info(STATION_IF, &ip_config);
	}
	Debug("station connect ok! ip:"IPSTR"\n\r",IP2STR(&(ip_config.ip.addr)));

	TcpClient_Init();//��ȡ��������ip�ˣ���ȥ��½Զ�̷�����
	JsonClient_Init();//���Ӵ����ݷ�����

	while(SysVars()->JsonConnStaus!=SCS_FOUND_SRV)//�ȴ����ӣ�����һ����½��Ϣ
	{
		OS_TaskDelaySec(1);
		
	}




	
	
	if(0){//��ʱ��ʾ��
		OS_TIMER_T *pTimer1=Q_Malloc(sizeof(OS_TIMER_T));
		OS_TimerDeinit(pTimer1);
		OS_TimerSetCallback(pTimer1,Timer1_Cb,pTimer1);
		OS_TimerInit(pTimer1,1000,FALSE);	
	}


	while(1){
		OS_TaskDelaySec(10);

		if(0)
		{
			char *pStr;
		    cJSON * root =  cJSON_CreateArray();
		    cJSON * one =  cJSON_CreateObject();
		    cJSON * two =  cJSON_CreateObject();
		    cJSON_AddItemToObject(one, "rc", cJSON_CreateNumber(0));//���ڵ������
		    cJSON_AddItemToObject(one, "operation", cJSON_CreateString("CALL"));
		    cJSON_AddItemToObject(one, "service", cJSON_CreateString("telephone"));
		    cJSON_AddItemToObject(one, "text", cJSON_CreateString("tel"));
			cJSON_AddItemToArray(root,one);

			cJSON_AddItemToObject(two, "rc", cJSON_CreateNumber(0));//���ڵ������
		    cJSON_AddItemToObject(two, "operation", cJSON_CreateString("CALL"));
			cJSON_AddItemToArray(root,two);

			pStr=cJSON_PrintUnformatted(root);
			UpJsonDataToSrv("/test",pStr,JsonSent_Cb);
			cJSON_Free(pStr);
		    cJSON_Delete(root);
		    //Debug("\n\r");
		}




		//os_delay_us(60000);
		//Debug(".");
	}


	
	vTaskDelete(NULL);
}

//wifi�¼��Ļص�����
void wifi_handle_event_cb(System_Event_t *evt)
{
    //Debug("EVT %s\n\r",gNameWifiEvent[evt->event_id]);
    
    switch (evt->event_id) 
    {
		case EVENT_STAMODE_SCAN_DONE:
			if(NeedDebug(DFT_SRV)) Debug("Scan done:%s\n\r",
			gNameStationStatus[evt->event_info.scan_done.status]);
			break;
		case EVENT_STAMODE_CONNECTED:
			PublicEvent(PET_NET_OK,0,NULL);
			UserConnHook();//send con	
			if(NeedDebug(DFT_SRV)) Debug("Connect to ssid %s, channel %d\n\r",
			evt->event_info.connected.ssid,
			evt->event_info.connected.channel);
			break;
		case EVENT_STAMODE_DISCONNECTED:
			PublicEvent(PET_NET_LOST,0,NULL);
			UserDisconnHook();//send con	
			if(NeedDebug(DFT_SRV)) Debug("Disconnect from ssid %s, reason %d\n\r",
			evt->event_info.disconnected.ssid,
			evt->event_info.disconnected.reason);

			if(evt->event_info.disconnected.reason==REASON_NO_AP_FOUND)//����apid
			{
				struct station_config *pConfig=Q_Malloc(ESP_AP_RECORD_NUM*sizeof(struct station_config));
				u16 ApId=wifi_station_get_current_ap_id();
				u16 NextId=((ApId+1)==ESP_AP_RECORD_NUM)?0:ApId+1;
				u16 i;
				
				wifi_station_get_ap_info(pConfig);
				for(i=0;i<ESP_AP_RECORD_NUM;i++)
				{
					if(i==NextId)
					{
						if(strlen(pConfig[i].ssid)==0) wifi_station_ap_change(0);
						else wifi_station_ap_change(NextId);
						break;
					}
				}		
				Q_Free(pConfig);				
			}
			break;
		case EVENT_STAMODE_AUTHMODE_CHANGE:
			if(NeedDebug(DFT_SRV)) Debug("Mode: %s -> %s\n\r",
			gNameAuthMode[evt->event_info.auth_change.old_mode],
			gNameAuthMode[evt->event_info.auth_change.new_mode]);
			break;
		case EVENT_STAMODE_GOT_IP:
			if(NeedDebug(DFT_SRV)) Debug("Ip:" IPSTR ",mask:" IPSTR ",gw:" IPSTR,
			IP2STR(&evt->event_info.got_ip.ip),
			IP2STR(&evt->event_info.got_ip.mask),
			IP2STR(&evt->event_info.got_ip.gw));
			if(NeedDebug(DFT_SRV)) Debug("\n\r");
			break;
		case EVENT_STAMODE_DHCP_TIMEOUT:
			if(NeedDebug(DFT_SRV)) Debug("Dhcp timeout\n\r");
			break;			
		case EVENT_SOFTAPMODE_STACONNECTED:
			if(NeedDebug(DFT_SRV)) Debug("Station: " MACSTR "join, AID = %d\n\r",
			MAC2STR(evt->event_info.sta_connected.mac),
			evt->event_info.sta_connected.aid);
			break;
		case EVENT_SOFTAPMODE_STADISCONNECTED:
			if(NeedDebug(DFT_SRV)) Debug("Station: " MACSTR "leave, AID = %d\n\r",
			MAC2STR(evt->event_info.sta_disconnected.mac),
			evt->event_info.sta_disconnected.aid);
			break;
		case EVENT_SOFTAPMODE_PROBEREQRECVED:
			//Debug("Probe req mac: "MACSTR"\n\r",MAC2STR(evt->event_info.ap_probereqrecved.mac));
			break;
		default:
			break;
 	}
} 

//ϵͳ������ʼ�����߳�
void StartSys_Task(void *pvParameters)
{ 
	SimulateRtc_Init();//ģ��rtc
    QDB_Init();//���ݿ��ʼ��
    InfoSaveInit(FALSE);//info���ݳ�ʼ��

    if(QDB_GetNum(SDN_SYS,SIN_DutRegSn)==GetDutRegSn())//��Ȩ��ȷ
    {
    	//Debug("Dut Reg Sn Is Ok!\r\n");
		SysVars()->DutRegSnIsOk=TRUE;
    }    

    //����Ҫʹ�����ݿ�����ݣ����Դ˿̲��ܳ�ʼ��lwip
	LwIP_CommInit();//��ʼ��lwip

	GpioHwInit();//gpio��ʼ������ȡ���Ƿ���Ҫ֧��uart������oled֧�ֵ���Ϣ
	if(SysVars()->SwapUartOut) system_uart_swap();//��������0����	
	if(SysVars()->SupportLCD) Oled_Init();

  	//SigRecordTidy();//ɾ���������źż�¼
  	VarRecordTidy();//ɾ���������豸������¼
  	DevRecordTidy();//ɾ����Ч����Ϣӳ��
  	TrigRecordTidy();//ɾ����Ч�Ĵ���Դ
  	StrRecordTidy();//ɾ�������������ַ���

    Debug("SoftVer %u\n\r",QDB_GetNum(SDN_SYS,SIN_SoftVer));
    //system_print_meminfo();

	AppClientInit();//��ʼ���û��б�
	VarListInit();//������ʼ��
	InitVarByIoState();//����io״̬������var��ֵ
	UpdateAllIoStateByVar();//���ݱ������������io
	SysVarPeriodicitySet(FALSE);//��һ������ϵͳ����
	
	AdcVar_Init();//��ʼ��adc�ͱ����Ĺ�ϵ
	UserHwInitHook();//�û��Զ���Ӳ����ʼ��������oled
	VarDisplay_Init();//��ʼ�����Ա����Ķ��ģ�����ŵ�oled��ʼ��֮��
	TrigListInit();//��ʼ������Դ
	Debug("System Init Finish!\n\r");

	wifi_set_event_handler_cb(wifi_handle_event_cb);
	OS_TaskCreate(InformEventTask,"InformEvent Task",512,NULL,TASK_INFORM_EVENT_PRIO,NULL);
	OS_TaskCreate(SysEventTask,"SysEvent Task",512,NULL,TASK_SYS_EVENT_PRIO,NULL);
    OS_TaskCreate(WifiConfig_Task, "WifiConfig Task",512,NULL,TASK_MAIN_PRIO,NULL); 
	
    vTaskDelete(NULL);
}

