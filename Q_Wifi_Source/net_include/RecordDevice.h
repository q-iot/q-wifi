 #ifndef DEVICE_RECORD_H
#define DEVICE_RECORD_H

typedef struct{
	u32 TransAddr;//按键信号的转发器地址
	u8 KeyList[KEYS_SET_MAX_KEY_NUM];//存储信号对应的key值
	u16 SigList[KEYS_SET_MAX_KEY_NUM];//存储信号
}KEYS_SET_DEV;

enum{
	WDT_NULL=0,
	WDT_WDEV_IO,//万纳模块
	WDT_OPEN_GATEWAY,//透传zigbee
	WDT_JMP,//转发器
	WDT_COM,//串口透传
	WDT_WIFI,//wifi模块设备
	WDT_LOW_POWER,//低功耗设备
	WDT_BLUE_TOOTH,//蓝牙设备

};
typedef u8 WAVER_DUT_TYPE;

typedef struct{
	u32 WNetAddr;//万纳网地址，地址必须放结构首部
	u32 Footer;
	WAVER_DUT_TYPE DutType;
	u8 SelfVarNum;
	bool Online:1;//用来存储设备是否在线，存储到flash的时候无用，数据传递的时候主机会填充
	bool NeedPw:1;//需要密码才能进入
	u8 a:6;
	u8 b;
	u16 RelateVar[RELATE_VAR_MAX];//每个设备关注的属性表
	u16 AwakenCode;//低功耗唤醒码
	u16 AwakenLifeSecDef;//唤醒时，如果没带唤醒存活时间，则使用此时间
}WAVER_DEV;

enum{
	CSM_NULL=0,
	CSM_SUM,//累加
	CSM_CRC,
	CSM_HASH33,
};
typedef u8 CHK_SUM_METHOD;

typedef struct{
	u8 VendorID;//厂商id，0通用串口设备，1恒维
	bool AddKeyToCmd:1;//附加key号码到命令字符串
	bool ChkSumRev:1;//检验码反向填充
	u8 ChkSumLen:2;//校验码长度
	CHK_SUM_METHOD ChkSumMothed:4;//校验码方法
	u8 a;//空隙
	u8 PktHeaderLen:4;//附加包头长度
	u8 AckLen:4;//回复的长度
	u8 PktHeader[4];//附加包头
	u8 SuccessAck[4];//成功的回复，如果有附加包头，也会加入附加包头
	u8 FaildAck[4];//失败的回复
}COM_DEV;//第三方串口输出

typedef struct{
	u32 ID;
	u8 Name[SHORT_STR_BYTES];
	u32 FileID;//产品型号文件ID，云端唯一
	u32 ProdID;//厂设ID，高16位表示厂家，低16位表示设备类型
	u32 UiID;//UI ID
	union{
		WAVER_DEV Waver;//双向设备，地址必须放首部
		KEYS_SET_DEV KeysSet;
		COM_DEV Com;
	}Record;
}DEVICE_RECORD;

#endif

