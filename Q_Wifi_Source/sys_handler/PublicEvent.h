#ifndef SYS_EVENT_H
#define SYS_EVENT_H


typedef enum{
	PET_NULL=0,

	PET_NET_OK,//网络连接
	PET_NET_LOST,//网络失联
	
	PET_TCP_CLIENT_LOST,//同某个tcp连接中断了
	PET_TCP_SRV_LOST,//同服务器连接中断了
	PET_TCP_JSON_LOST,//json链接断了

	
	PET_APP_LOGIN,//app登陆成功
	PET_APP_LOST,//app主动或者被动离开

	PET_APP_NEW_DEV,//app新增了一个设备
	PET_APP_NEW_TRIG,//app新增了一个触发源
	PET_APP_MODIFY_DEV,
	PET_APP_MODIFY_SCENE,
	PET_APP_DEL_DEV,//app删除了一个设备
	PET_APP_DEL_SCENE,
	PET_APP_DEL_VAR,
	PET_APP_DEL_TRIG,

	PET_SRV_RTC_CHANGE,//服务器更新了本机时间	
	PET_DEV_UNBIND,//设备解绑

	
	
}PUBLIC_EVENT;


void PublicEvent(PUBLIC_EVENT Event,u32 Param,void *p);


#endif

