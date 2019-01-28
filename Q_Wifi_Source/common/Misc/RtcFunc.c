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


