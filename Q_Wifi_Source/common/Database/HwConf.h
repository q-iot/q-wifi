#ifndef Q_HW_CONFIG_H
#define Q_HW_CONFIG_H

enum{
	WIM_NULL=0,
		
	WIM_KEY,//按键模式，会有防干扰处理，只响应下降沿，发生响应时，可以匹配、反向匹配、取反、计数变量。变量响应时间为延时200ms，适合接按键。
	WIM_IPT,//输入

	WIM_OUT,//推挽输出，根据绑定的变量来匹配输出，当变量某bit发生变化时，可以匹配、反向匹配、取反。
	WIM_OUT_OD,//开漏输出

	WIM_AIN,//模拟量输入，采样周期同上报周期相同，但上报周期到期时，如变量未变，则不上报
	WIM_PWM,//pwm输出
	WIM_UART,//用户通讯口
	WIM_OLED,//oled的i2c输出
	
	WIM_MAX
};
typedef u8 WE_IO_MODE;

enum{
	AIM_NULL=0,
	AIM_MATCH,//匹配。输出时，变量对应bit为1，输出1，bit为0，输出0。输入时，io为1，bit改为1，io为0，bit改为0。
	AIM_REVERSE,//反向匹配。输出时，bit为1，输出0，bit为0，输出1。输入时，io为1，bit改为0，io为0，bit改为1。
	AIM_NEGATION,//取反。输入时，io有中断，则bit取反
	AIM_COUNT,//计数。输入时，io有中断，则变量加1

	AIM_MAX,
};
typedef u8 VAR_IO_MATCH;

typedef struct{
	WE_IO_MODE IoMode:4;//io模式
	bool Pullup:1;//是否上拉电阻
	VAR_IO_MATCH VarIoMth:3;//io与变量的关系
	u8 Bit;//变量位，0-15
	u16 VarID;//变量ID，0表示不映射
}WE_IO_CONFIG;

typedef struct{
	u8 AdcMode;//使用adc时，此值置1
	u8 Tolerance;//容差，1-99%，采样值运算的结果超过容差，变量才改变
	u16 Factor;//因子，采样值除以此值，再加上偏差，才存储到变量，此值为实际值的100倍，不允许为0
	s16 Offset;//偏差
	u16 VarID;//绑定的变量索引
	u16 PeriodIdx;//采样周期
}WE_AIN_CONFIG;

enum{
	WPW_NULL=0,//不使用
	WPW_FIXED,//固定式，变量的bit控制开关
	WPW_PERIOD,//脉宽可调式，变量决定Cnt值
	WPW_FACTOR,//占空比可调式，变量决定PluseCnt值
};
typedef u8 WE_PWM_TYPE;

typedef struct{
	WE_PWM_TYPE PwmMode;
	u8 a;
	u16 Cnt;//决定脉宽
	u16 PluseCnt;//决定占空比
	u16 uS_Base;
}WE_PWM_CONFIG;




#endif
