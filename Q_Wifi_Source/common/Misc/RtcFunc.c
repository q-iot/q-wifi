//--------------------------Q-IOT Platform------------------------------------------------------//
/*
Q-Wifi�ǿ�������ƽ̨�Ĺ̼�ʵ�֣�����ESP8266������������һ�������APP��������ƽ̨�໥��ϵ�������
�ײ�ʵ�ֻ��ƣ��������ESP8266_RTOS_SDK��������Դ����ӭ�������Ա��ţ���룡

Q-Wifi������һ���豸���������龰���������û��ն˵Ĺ�����ƣ��ڽ������ɸ�TCP������APP����ƽ̨������
����json�ϱ����ӣ�����ͨ�õ�http json��ʽ���ɱ������ߵ������Լ���������ƽ̨���������ݻ㼯�Ϳ��ơ�

Q-Wifi�ڽ���һ�׶�̬��web��ҳ����������������Ӳ�����ã���web��ҳ��������װ�������ɱ���������ֲ��
������Ŀ��

Q-Wifi�ڽ���һ�׻��ڴ��ڵ�ָ�����ϵͳ���������д��ڵ��ԣ�ָ���·����˽���ϵͳ��װ����������һϵ
�н����������ɱ���������ֲ��������Ŀ��

Q-Wifi�ڲ���user��ʶ���ļ�����Ϊ֧�ֿ����������޸ĵĿ��ƻ��ļ����ر���user_hook.c�ļ���ϵͳ����
�йؼ��㶼����hook�����ڴ��ļ��У��������߶��ο�����

Q-Wifi����ӵ���ڶ�ģ�黯�Ļ��ƻ򷽷������Ա����ü���ֲ������������ϵͳ�Ŀ����Ѷȡ�
���л��ڿ�������ƽ̨���еĿ�����������Ʒ��������ϵ�����Ŷӣ���ѷ����ڿ�������Ƶ��q-iot.cn������
�������г���������Ӧ���п۳�˰�Ѽ�ά�����ú󣬾�ȫ���ṩ�������ߣ��Դ˹������ڿ�Դ��ҵ��

By Karlno ����Ƽ�

���ļ�������һ��rtcת����
*/
//---------------------------------------------------------------------------------------------//
#include "SysDefines.h"

#define RTC_START_YEAR 1980  //�ɴ�1904�����κ�һ�����꿪ʼ,����ı��ֵ�����ڳ�ֵҪ�ı䡣ֻ�ܿ�Խ135��
#define RTC_WEEK_INIT_OFFSET 2//����ȡֵ0������ȡֵ6�Դ�����
#define DAY_SECONDS 86400	//һ���������

//�����������������
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

//�������������������
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

//ʱ��ת����
//��ʽ���Է���0
u32 RtcTime2Cnt(RTC_TIME *pTime)
{
	u32 TimeCnt,Tmp;
	u8 LeapFlag=0;

	TimeCnt=pTime->year-RTC_START_YEAR;
	if((TimeCnt>135)||(pTime->year<RTC_START_YEAR)) return 0; //��ݼ��
	
	if(TimeCnt) Tmp=(TimeCnt-1)/4+1;//�ж��������
	else Tmp=0;
	LeapFlag=(TimeCnt%4)?0:1;
	TimeCnt=(TimeCnt*365+Tmp)*DAY_SECONDS;//�껻��ɵ�����

	if((pTime->mon<1)||(pTime->mon>12)) return 0;  //�·ݼ��
	
	if(LeapFlag)
	{
		if(pTime->day>((Leap_Month_Seconds[pTime->mon]-Leap_Month_Seconds[pTime->mon-1])/DAY_SECONDS)) return 0; //�ռ��
		Tmp=Leap_Month_Seconds[pTime->mon-1];
	}
	else
	{
		if(pTime->day>((Month_Seconds[pTime->mon]-Month_Seconds[pTime->mon-1])/DAY_SECONDS)) return 0;//�ռ��
		Tmp=Month_Seconds[pTime->mon-1];
	}

	if(pTime->hour>23) return 0; //Сʱ���
	if(pTime->min>59) return 0;	 //���Ӽ��
	if(pTime->sec>59) return 0;	 //����

	TimeCnt+=(Tmp+(pTime->day-1)*DAY_SECONDS);

	TimeCnt+=(pTime->hour*3600 + pTime->min*60 + pTime->sec);

	return TimeCnt;
}

//����תʱ��
void RtcCnt2Time(u32 TimeCnt,RTC_TIME *pTime)
{ 
  u32 Tmp,i;

  //���������꣬���ǵ�����Ĵ��ڣ���4��һ������
  Tmp=TimeCnt%(DAY_SECONDS*366+DAY_SECONDS*365*3);
  if(Tmp<DAY_SECONDS*366) pTime->year=RTC_START_YEAR+TimeCnt/(DAY_SECONDS*366+DAY_SECONDS*365*3)*4+0;
  else if(Tmp<DAY_SECONDS*366+DAY_SECONDS*365*1) pTime->year=RTC_START_YEAR+TimeCnt/(DAY_SECONDS*366+DAY_SECONDS*365*3)*4+1;
  else if(Tmp<DAY_SECONDS*366+DAY_SECONDS*365*2) pTime->year=RTC_START_YEAR+TimeCnt/(DAY_SECONDS*366+DAY_SECONDS*365*3)*4+2;
  else pTime->year=RTC_START_YEAR+TimeCnt/(DAY_SECONDS*366+DAY_SECONDS*365*3)*4+3;

  if(Tmp<DAY_SECONDS*366) //����
  {
		for(i=1;i<13;i++)
	    {
			if(Tmp<Leap_Month_Seconds[i])
			{
				pTime->mon=i;
				Tmp-=Leap_Month_Seconds[i-1];//�������ѯ���渴�ӵļ���
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
  //ע���ȡ��ֵ��ΧΪ0-6��������Ϊ�˷���������ģ��Ӷ������������ʾ

  pTime->day=Tmp/DAY_SECONDS+1;
  Tmp=Tmp%DAY_SECONDS;

  pTime->hour = Tmp/3600;
  Tmp=Tmp%3600;

  pTime->min = Tmp/60;

  pTime->sec = Tmp%60;
}

//����ʱ�������
void RtcTimeGetWeek(RTC_TIME *pTime)
{
	u32 TimeCnt=RtcTime2Cnt(pTime);

	if(TimeCnt)
	{
		pTime->week=(TimeCnt/DAY_SECONDS+RTC_WEEK_INIT_OFFSET)%7;
	}	
}


