#ifndef ESP_DEVELOP_H
#define ESP_DEVELOP_H

#define ESP_AP_RECORD_NUM 5 //esp��¼ap��Ŀ

#define __gBinSoftVer QWIFI_SOFT_VER

//#define NULL (void *)0
#define Bit(bit)		(1<<(bit))
#define SetBit(reg,bit) (reg|=(1<<(bit)))
#define ClrBit(reg,bit) (reg&=~(1<<(bit)))
#define RevBit(reg,bit) (reg^=(1<<(bit)))
#define ReadBit(reg,bit) ((reg>>(bit))&1)

//��u8��������ʾ�ڶ����ı�ʶ
#define SetArrayBit(a,bit) SetBit(a[(bit)>>3],7-((bit)&0x7))
#define ClrArrayBit(a,bit) ClrBit(a[(bit)>>3],7-((bit)&0x7))
#define ReadArrayBit(a,bit) ReadBit(a[(bit)>>3],7-((bit)&0x7))

#define HBit8(v) (((v)>>8)&0xff)
#define LBit8(v) ((v)&0xff)
#define HBit16(v) (((v)>>16)&0xffff)
#define LBit16(v) ((v)&0xffff)

#define ClientAddr(Addr)  ((Addr)&0xffff) //���һ���ӻ���ַ�ĵ�16λ
#define GroupAddr(Addr) (((Addr)>>16)&0xffff) //���һ���ӻ���ַ�ĸ�16λ

#define EspHwAdcRead() system_adc_read()
#define IWDG_ReloadCounter() system_soft_wdt_feed()

#define UpdateObjIdBase() do{QDB_SetNum(SDN_QCK,QIN_OBJ_ID_BASE,0);}while(0);//����Ϊ0������s16�ĸ���
#define ObjIdBase() (QDB_GetNum(SDN_QCK,QIN_OBJ_ID_BASE))

#define UpdateInfoFlag(x)	 do{QDB_SetValue(SDN_QCK,QIN_INFO_FLAG,x,NULL,0);}while(0);
#define GetInfoFlag(x) (QDB_GetValue(SDN_QCK,QIN_INFO_FLAG,x,NULL))
#define GetSysConfigFlag() (QDB_GetNowChkSum(SDN_SYS))

typedef struct {
	u16 year;// 2000~2099
	u8 mon;// 1~12
	u8 day;// 0~31
	u8 week;//ȡֵ0-6��0����������
	u8 hour; 
	u8 min;
	u8 sec;
}RTC_TIME;

void SimulateRtc_Init(void);
u32 RtcGetCnt(void);
void RtcSetCnt(u32 t);

void RebootBoard(void);
void ConfigChange(bool SysChange,bool InfoChange);

u32 GenerateDefPw(void);
u8 *GetMacAddress(void);
u32 GetHwID(void);
u32 GetDutRegSn(void);

#endif

