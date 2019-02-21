//--------------------------Q-IOT Platform------------------------------------------------------//
/*
Q-Wifi是酷享物联平台的固件实现，基于ESP8266开发，构建了一套与酷享APP、酷享云平台相互配合的物联网
底层实现机制，代码基于ESP8266_RTOS_SDK开发，开源，欢迎更多程序员大牛加入！

Q-Wifi构架了一套设备、变量、情景、动作、用户终端的管理机制，内建了若干个TCP连接与APP及云平台互动，
其中json上报连接，采用通用的http json格式，可被开发者导向至自己的物联网平台，进行数据汇集和控制。

Q-Wifi内建了一套动态的web网页服务器，用来进行硬件配置，此web网页服务器封装完整，可被开发者移植到
其他项目。

Q-Wifi内建了一套基于串口的指令解析系统，用来进行串口调试，指令下发，此解析系统封装完整，包含一系
列解析函数，可被开发者移植至其他项目。

Q-Wifi内部带user标识的文件，均为支持开发者自主修改的客制化文件，特别是user_hook.c文件，系统内所
有关键点都会有hook函数在此文件中，供开发者二次开发。

Q-Wifi代码拥有众多模块化的机制或方法，可以被复用及移植，减少物联网系统的开发难度。
所有基于酷享物联平台进行的开发或案例、产品，均可联系酷享团队，免费放置于酷物联视频（q-iot.cn）进行
传播或有偿售卖，相应所有扣除税费及维护费用后，均全额提供给贡献者，以此鼓励国内开源事业。

By Karlno 酷享科技

本文件定义了一个rtc转换库
*/
//---------------------------------------------------------------------------------------------//
#include "SysDefines.h"

#define RTC_START_YEAR 1980  //可从1904年后的任何一个闰年开始,如果改变此值，星期初值要改变。只能跨越135年
#define RTC_WEEK_INIT_OFFSET 2//周天取值0，周六取值6以此类推
#define DAY_SECONDS 86400	//一天的总秒数

//闰年的逐月秒数计算
const int Leap_Month_Seconds[13]={
	0,
	DAY_SECONDS*31,
	DAY_SECONDS*(31+29),
	DAY_SECONDS*(31+29+31),
	DAY_SECONDS*(31+29+31+30),
	DAY_SECONDS*(31+29+31+30+31),
	DAY_SECONDS*(31+29+31+30+31+30),
	DAY_SECONDS*(31+29+31+30+31+30+31),
	DAY_SECONDS*(31+29+31+30+31+30+31+31),
	DAY_SECONDS*(31+29+31+30+31+30+31+31+30),
	DAY_SECONDS*(31+29+31+30+31+30+31+31+30+31),
	DAY_SECONDS*(31+29+31+30+31+30+31+31+30+31+30),
	DAY_SECONDS*(31+29+31+30+31+30+31+31+30+31+30+31),
};

//非闰年的逐月秒数计算
const int Month_Seconds[13]={
	0,
	DAY_SECONDS*31,
	DAY_SECONDS*(31+28),
	DAY_SECONDS*(31+28+31),
	DAY_SECONDS*(31+28+31+30),
	DAY_SECONDS*(31+28+31+30+31),
	DAY_SECONDS*(31+28+31+30+31+30),
	DAY_SECONDS*(31+28+31+30+31+30+31),
	DAY_SECONDS*(31+28+31+30+31+30+31+31),
	DAY_SECONDS*(31+28+31+30+31+30+31+31+30),
	DAY_SECONDS*(31+28+31+30+31+30+31+31+30+31),
	DAY_SECONDS*(31+28+31+30+31+30+31+31+30+31+30),
	DAY_SECONDS*(31+28+31+30+31+30+31+31+30+31+30+31),
};

//时间转计数
//格式不对返回0
u32 RtcTime2Cnt(RTC_TIME *pTime)
{
	u32 TimeCnt,Tmp;
	u8 LeapFlag=0;

	TimeCnt=pTime->year-RTC_START_YEAR;
	if((TimeCnt>135)||(pTime->year<RTC_START_YEAR)) return 0; //年份检测
	
	if(TimeCnt) Tmp=(TimeCnt-1)/4+1;//判断闰年个数
	else Tmp=0;
	LeapFlag=(TimeCnt%4)?0:1;
	TimeCnt=(TimeCnt*365+Tmp)*DAY_SECONDS;//年换算成的秒数

	if((pTime->mon<1)||(pTime->mon>12)) return 0;  //月份检查
	
	if(LeapFlag)
	{
		if(pTime->day>((Leap_Month_Seconds[pTime->mon]-Leap_Month_Seconds[pTime->mon-1])/DAY_SECONDS)) return 0; //日检查
		Tmp=Leap_Month_Seconds[pTime->mon-1];
	}
	else
	{
		if(pTime->day>((Month_Seconds[pTime->mon]-Month_Seconds[pTime->mon-1])/DAY_SECONDS)) return 0;//日检查
		Tmp=Month_Seconds[pTime->mon-1];
	}

	if(pTime->hour>23) return 0; //小时检查
	if(pTime->min>59) return 0;	 //分钟检查
	if(pTime->sec>59) return 0;	 //秒检查

	TimeCnt+=(Tmp+(pTime->day-1)*DAY_SECONDS);

	TimeCnt+=(pTime->hour*3600 + pTime->min*60 + pTime->sec);

	return TimeCnt;
}

//计数转时间
void RtcCnt2Time(u32 TimeCnt,RTC_TIME *pTime)
{ 
  u32 Tmp,i;

  //计算周期年，考虑到闰年的存在，以4年一个周期
  Tmp=TimeCnt%(DAY_SECONDS*366+DAY_SECONDS*365*3);
  if(Tmp<DAY_SECONDS*366) pTime->year=RTC_START_YEAR+TimeCnt/(DAY_SECONDS*366+DAY_SECONDS*365*3)*4+0;
  else if(Tmp<DAY_SECONDS*366+DAY_SECONDS*365*1) pTime->year=RTC_START_YEAR+TimeCnt/(DAY_SECONDS*366+DAY_SECONDS*365*3)*4+1;
  else if(Tmp<DAY_SECONDS*366+DAY_SECONDS*365*2) pTime->year=RTC_START_YEAR+TimeCnt/(DAY_SECONDS*366+DAY_SECONDS*365*3)*4+2;
  else pTime->year=RTC_START_YEAR+TimeCnt/(DAY_SECONDS*366+DAY_SECONDS*365*3)*4+3;

  if(Tmp<DAY_SECONDS*366) //闰年
  {
		for(i=1;i<13;i++)
	    {
			if(Tmp<Leap_Month_Seconds[i])
			{
				pTime->mon=i;
				Tmp-=Leap_Month_Seconds[i-1];//用数组查询代替复杂的计算
				break;
		  	}
	   }
  }
  else if(Tmp<DAY_SECONDS*366+DAY_SECONDS*365*1)
  {
		Tmp-=DAY_SECONDS*366;

	  	for(i=1;i<13;i++)
		{
			if(Tmp<Month_Seconds[i])
			{
				pTime->mon=i;
				Tmp-=Month_Seconds[i-1];
				break;
			}
		}		
  }
  else if(Tmp<DAY_SECONDS*366+DAY_SECONDS*365*2)
  {
  		Tmp-=DAY_SECONDS*366+DAY_SECONDS*365*1;

	  	for(i=1;i<13;i++)
		{
			if(Tmp<Month_Seconds[i])
			{
				pTime->mon=i;
				Tmp-=Month_Seconds[i-1];
				break;
			}
		}	  		
  }
  else
  {
		Tmp-=DAY_SECONDS*366+DAY_SECONDS*365*2;
		
	  	for(i=1;i<13;i++)
		{
			if(Tmp<Month_Seconds[i])
			{
				pTime->mon=i;
				Tmp-=Month_Seconds[i-1];
				break;
			}
		}			
  }

  pTime->week=(TimeCnt/DAY_SECONDS+RTC_WEEK_INIT_OFFSET)%7;
  //注意获取的值范围为0-6，这样是为了方便数组查阅，从而翻译成中文显示

  pTime->day=Tmp/DAY_SECONDS+1;
  Tmp=Tmp%DAY_SECONDS;

  pTime->hour = Tmp/3600;
  Tmp=Tmp%3600;

  pTime->min = Tmp/60;

  pTime->sec = Tmp%60;
}

//根据时间得星期
void RtcTimeGetWeek(RTC_TIME *pTime)
{
	u32 TimeCnt=RtcTime2Cnt(pTime);

	if(TimeCnt)
	{
		pTime->week=(TimeCnt/DAY_SECONDS+RTC_WEEK_INIT_OFFSET)%7;
	}	
}


