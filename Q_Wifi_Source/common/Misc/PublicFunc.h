#ifndef QSYS_PUBLIC_FUNC_H
#define QSYS_PUBLIC_FUNC_H


//2									���ú���	(PublicFunc.c)						

#if 1
void MemSet(void *Dst,u8 C,u16 Byte);
void MemCpy(void *Dst,const void *Src,u16 Byte);
#else
#define MemSet memset
#define MemCpy memcpy
#endif

void DisplayStrN(const char *Buf,u16 Len);//��ӡָ�����ȵ��ַ���
void DisplayBuf(const u8 *Buf,u16 Len,u8 RawLen);//��ӡbuf����
void DisplayBufU16_Dec(const u16 *Buf,u16 Len,u8 RawLen);
void DisplayBufU16(const u16 *Buf,u16 Len,u8 RawLen);
void DisplayBufU32(const u32 *Buf,u16 Len,u8 RawLen);
bool CompareBuf(const u8 *Buf1,const u8 *Buf2,u16 Len);//�Ƚ������ڴ�����ֽ��Ƿ���ͬ������TRUE��ʾ��ͬ��Lenָ���Ƚϳ���
bool FuzzyEqual(u32 A,u32 B,u8 Tole);

u32 HexStr2Uint(const char *pStr);//������ǰ׺��ʮ�����������ַ���ת��Ϊ����       
u32 Str2Uint(const char *pStr);//�ַ���ת��Ϊ����
s32 Str2Sint(const char *pStr);//�з���ת��
int Float2Int(float f);//������ת��Ϊ���ͣ���������
u32 Float2Ieee(float f_num);//������תieee��׼
float Ieee2Float(u32 b_num);//ieee��׼ת������
char *Ip2Str(void *pIp);
bool Str2Ip(const char *pStr,u8 *Ip);
void StrChrRep(char *pStr,char Orgc,char Repc);
const char *FindNumFromStr(const char *pStr,s32 *pNumRet);
void Str2Lower(char *pStr);
bool IsNullStr(char *pStr);
bool NotNullStr(char *p);
u16 StrnCmp(const char *pStr1,const char *pStr2,u16 Bytes);
char *ChkStr(const char *pStr1, const char *pStr2 );
char *FindStr(char *pStr,char *pStrStart,char *pStrEnd);

u32 AlignTo4(u32 v);
bool IsAlign4(u32 v);
u32 AlignTo8(u32 v);
bool IsAlign8(u32 v);

u32 MakeHash33(const u8 *pData,u32 Len);
u32 CheckSum(const u8 *pData,u32 Len);
u16 CRC16(const u8 *pData,u16 Len);
u16 Rev16(u16 Data);
u32 Rev32(u32 Data);
u32 Rand(u32 Mask);



#endif

