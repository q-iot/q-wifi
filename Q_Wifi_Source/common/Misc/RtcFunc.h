#ifndef RTC_FUNC_H
#define RTC_FUNC_H

//��������UNIXʱ���Ǵ�1970��0ʱ����ʼ������ʱ���Ǵ�1980��8ʱ����ʼ
#define RTC_UNIX_OFFSET (3652*86400) 
#define RTC_ZONE8_OFFSET (8*3600)
#define UnixStamp2Rtc(x) ((x)-RTC_UNIX_OFFSET+RTC_ZONE8_OFFSET)
#define Rtc2UnixStamp(x) ((x)+RTC_UNIX_OFFSET-RTC_ZONE8_OFFSET)

u32 RtcTime2Cnt(RTC_TIME *pTime);
void RtcCnt2Time(u32 TimeCnt,RTC_TIME *pTime);
void RtcTimeGetWeek(RTC_TIME *pTime);





#endif

