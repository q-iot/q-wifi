#ifndef SYS_EVENT_HANDLER
#define SYS_EVENT_HANDLER


typedef enum{
	MFM_NULL=0,
	MFM_ALWAYS,//正常模式，只要调用，就一直往里加
	MFM_COVER,//不会重复发送事件，只会发送一次，第二次调用只修改时间
	MFM_NEAREST,//不会重复发送事件，修改的时间一定是调用设置的最近时间
}MS_FUNC_MODE;


typedef enum{
	SEN_NULL=0,
	SEN_SYS_CMD,//命令行串口命令
	SEN_SEC,//一秒事件
	SEN_32_SEC,//半分钟事件
	//SEN_IR_CAPTURE,//ir捕获成功
	//SEN_IR_TIMOUT,
	//SEN_RF_CAPTURE,//rf捕获成功	
	//SEN_RF_TIMOUT,
	//SEN_WRECV_DISTRIB_DEATH,//wnet recv 分发到期处理
	SEN_SRV_BEAT,//发送心跳包给服务器
	SEN_OVER_CLIENT,//踢掉链接
	SEN_INFORM_TASK,//唤醒inform task
	//SEN_CLEAN_LCD,//清除lcd显示
	//SEN_SIMU_STD_RF,//发送模拟射频信号，用于自定义编码射频，SimulateRecvStdRf
	SEN_CB_FUNC,//执行标准回调函数
	SEN_SET_AP,//异步设置新的接入点，esp8266特有
	SEN_QCK_SAVE,//异步延时保存快速数据库，esp8266特有
	SEN_OLED_DISP,//oled显示刷新，esp8266特有
	SEN_PIN_CHANGE,//pin改变，esp8266特有
	SEN_USER_EVT1,//用户事件，esp8266特有
	SEN_USER_EVT2,//用户事件，esp8266特有
	SEN_USER_EVT3,//用户事件，esp8266特有
	SEN_USER_EVT4,//用户事件，esp8266特有
	
	SEN_MAX	
}SYS_EVENT_NAME;

void SysEventTask(void *pvParameters);
void SysEventSend(SYS_EVENT_NAME Event,u32 Arg1,void *pArg2,bool *pYeild);
void SysEventMsSend(u32 Ms,u32 Event,u32 Arg1,void *pArg2,MS_FUNC_MODE Mode);
void SysEventMsDelete(u32 Event);

#endif




