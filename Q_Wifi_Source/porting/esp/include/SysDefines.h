#ifndef SYS_DEFINES_H
#define SYS_DEFINES_H

#define ADMIN_DEBUG 1
#define QWIFI_SOFT_VER 300
#define RELEASE_DAY 190122

#define SRV_FAILD_RETRY_MS (10*1000)//����������ʧ�ܵ�����ʱ��
#define SRV_CONN_BEAT_S 30//�ͻ���������ʱ�䣬��λ��
#define SRV_CONN_SEND_TIMOUT_S 5
#define SRV_CONN_RECV_TIMOUT_S (3*SRV_CONN_BEAT_S+5)//�ͻ��˵���ʱ�䣬��λ��

#define APP_CONN_BEAT_S 120
#define APP_CONN_SEND_TIMOUT_S 5
#define APP_CONN_RECV_TIMOUT_S (2*APP_CONN_BEAT_S+5)

#define MAX_VAR(a,b)  ((a)>(b)?(a):(b))
#define MIN_VAR(a,b)  ((a)<(b)?(a):(b))

#define HL8_U16(H,L) ((((H)&0xff)<<8)|((L)&0xff))
#define HL16_U32(H,L) ((((H)&0xffff)<<16)|((L)&0xffff))

#define Frame() //Debug("                                                                              |\r");

//---------------------------ͷ�ļ�----------------------------------------
#include "esp_common.h"
#include "oled.h"
#include "EspDevelop.h"
#include "PrintFunc.h"
#include "cJSON.h"
#include "SpiFlashApi.h"
#include "DebugOutput.h"
#include "Os_Wrap.h"
#include "LimitMarco.h"
#include "NameDebug.h"
#include "Q_Heap.h"

#include "lwip/err.h"
#include "lwip/opt.h"
#include "lwip/arch.h"
#include "lwip/api.h"
#include "lwip/dns.h"

#include "FuncType.h"
#include "PublicFunc.h"
#include "InfoSave.h"
#include "Database.h"
#include "Db4System.h"
#include "Db4QuickSave.h"
#include "Db4HwConf.h"

#include "ProdID.h"
#include "NetType.h"
#include "RecordVariable.h"
#include "RecordDevice.h"
#include "RecordString.h"
#include "RecordScene.h"
#include "RecordTrigger.h"

#include "SystemMsgDisplay.h"
#include "VarManger.h"
#include "SysEventHandler.h"
#include "InformEventTask.h"
#include "NetDataSend.h"
#include "NotifyManger.h"
#include "VarIoHandler.h"
#include "user_hook.h"






//-----------------------------�̶߳���------------------------------------
//��ջ��С���壬���Ϊu32������ʵ�ʷ����ֽ���������Ŀ��4��
#define TASK_STK_SIZE_MIN  176
#define TASK_STK_SIZE_NORMAL 256
#define TASK_STK_SIZE_MAX  512

//�������ȼ�����
//����Խ�ͣ����ȼ�Խ�ͣ���Сֵ0�����ֵconfigMAX_PRIORITIES - 1
//ESP SDK�ײ��Ѿ���������ȼ�:
//watchdog task 14
//pp task 13
//timer task 12
//tcp ip task 10
//freerots timer 2
//idle task 0
#define TASK_PRIO_MIN   1
#define TASK_PRIO_MAX  9
#define TASK_MAIN_PRIO   8
#define TASK_INFORM_EVENT_PRIO 6
#define TASK_TCP_APP_PRIO 5
#define TASK_TCP_CLIENT_PRIO 4
#define TASK_SYS_EVENT_PRIO 3
#define TASK_SCN_EXE_PRIO 5
#define TASK_VAR_JSON_UP_PRIO 6

//---------------------------�������紦���һЩ�궨��----------------------
#define NET_PKTLEN_CHK_CODE 0xa5a5



#define DUT_TYPE_FUTURE 0
#define DUT_TYPE_QWIFI 1



#endif

