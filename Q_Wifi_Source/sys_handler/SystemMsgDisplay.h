#ifndef SYSTEM_MSG_DEBUG_H
#define SYSTEM_MSG_DEBUG_H

typedef enum{
	DFT_SYS=0,
	DFT_TCP,
	DFT_SRV,
	DFT_SRV_JSON,
	DFT_APP,
	DFT_TASK,
	DFT_INFO,
	DFT_PKT,
	DFT_PKT_DATA,
	DFT_WIFI,
	DFT_VAR,
	DFT_VAR_INF,

	DFT_MAX
}DEBUG_FLAG_TYPE;

#define NeedDebug(x) (QDB_GetValue(SDN_QCK,QIN_DEBUG_FLAG,(x),NULL)?TRUE:FALSE)
#define EnDebugBit(x) QDB_SetValue(SDN_QCK,QIN_DEBUG_FLAG,(x),NOT_NULL,0)
#define DisDebugBit(x) QDB_SetValue(SDN_QCK,QIN_DEBUG_FLAG,(x),NULL,0)

#define SysRunningMsg(a,b)

#endif

