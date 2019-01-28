#ifndef STR_SAVE_H
#define STR_SAVE_H


enum{
	SRT_NULL=0,
	SRT_STR,//字符串，用作情景消息内容，触发源消息内容等等
	SRT_DATA,//数据
	SRT_DEV_STR,//设备归属字符串，SubID表示所属设备的id，用于设备自定义字符串
	SRT_VAR_UP_CONF,//变量上报配置，SubID表示对应变量id，Num表示上报类型(0,变化，1-19周期)，用于记录需上报的变量
};
typedef u8 STR_REC_TYPE;

typedef struct{
	u32 ID;
	u32 SubID;//绑定的主对象ID
	STR_REC_TYPE Type;
	u8 Num;
	u16 DataLen;
	u8 Data[STR_RECORD_BYTES];
}STR_RECORD;

#endif

