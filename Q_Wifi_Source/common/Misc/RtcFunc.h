#ifndef RTC_FUNC_H
#define RTC_FUNC_H

//服务器的UNIX时间是从1970年0时区开始，本机时间是从1980年8时区起始
#define RTC_UNIX_OFFSET (3652*86400) 
#define RTC_ZONE8_OFFSET (8*3600)
#define UnixStamp2Rtc(x) ((x)-RTC_UNIX_OFFSET+RTC_ZONE8_OFFSET)
#define Rtc2UnixStamp(x) ((x)+RTC_UNIX_OFFSET-RTC_ZONE8_OFFSET)

u32 RtcTime2Cnt(RTC_TIME *pTime);
void RtcCnt2Time(u32 TimeCnt,RTC_TIME *pTime);
void RtcTimeGetWeek(RTC_TIME *pTime);





#endif

