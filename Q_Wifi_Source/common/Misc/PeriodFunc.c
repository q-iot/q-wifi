#include "SysDefines.h"

//adc循环和变量上报周期均采用的时间映射表
static const u32 gTimeMsMap[]={0,500,1000,5000,10000,60000,600000,1800000};

u32 PeriodIdx2Ms(u8 Idx)
{
	if(Idx >= (sizeof(gTimeMsMap)/sizeof(gTimeMsMap[0]))) return 0;
	return gTimeMsMap[Idx];
}


