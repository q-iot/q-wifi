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

本文件定义了tlv的解码和编码方法
*/
//---------------------------------------------------------------------------------------------//
#include "SysDefines.h"
#include "TLV.h"

void TLV_Debug(u8 *pTlv,u16 Len)
{
	u8 *pBuf=Q_Zalloc(512);
	TLV_DATA *pItem=(void *)pBuf;
	u16 Idx=1;
	s16 TlvLen=0;
	u16 Sum=0;
	
	while(Sum<Len)
	{
		MemSet(pBuf,0,64);
		TlvLen=TLV_Decode(pTlv,Len,Idx++,pItem);
		Sum+=(2+TlvLen);

		if(TlvLen<0) break;
		else if(TlvLen>0) Debug("[T%2u]%s(L%u)\n\r",pItem->Type,pItem->Str,pItem->Len);
		else Debug("[T%2u](L%u)\n\r",pItem->Type,TlvLen);
	}

	Q_Free(pBuf);
}

//会将值加入到pOut的末尾
//返回整体长度
//错误返回0
u16 TLV_Build(u8 *pOut,u16 BufLen,u8 Type,u8 *ValueStr)
{
	u16 Len,Sum=0;
	u8 i;
	
	for(i=0;i<0xff;i++)//不允许添加太多
	{
		if(pOut[0]==0)//空闲位置
		{
			//开始添加
			Len=strlen((void *)ValueStr)+1;
			if(Len>0xfe) return 0; 
			pOut[0]=Type;
			pOut[1]=Len;
			pOut+=2;
			MemCpy(pOut,ValueStr,Len);
			pOut[Len-1]=0;//结束符
			pOut[Len]=0;//给下一个tlv的type赋值，防止下次越界
			return Sum+Len+2;
		}
		else//有值
		{
			Len=pOut[1];
			pOut=&pOut[Len+2];
			Sum+=(Len+2);
			if((Sum+3+strlen((void *)ValueStr))>=BufLen) return 0;//越界检查
		}
	}

	return 0;
}

//解码tlv
//idx从1开始
//解码内容被拷贝到pItem中
//错误返回-1，正确返回长度
s16 TLV_Decode(u8 *pIn,u16 BufLen,u16 Idx,TLV_DATA *pItem)
{
	u16 Len,Sum=0;
	u8 i;

	for(i=1;i<=0xff;i++)//无需解码太多
	{
		if(pIn[0]==0)//空闲位置
		{
			return -1;
		}
		else//有值
		{
			if(i==Idx)//取值
			{
				pItem->Type=pIn[0];
				pItem->Len=pIn[1];
				MemCpy(pItem->Str,&pIn[2],pItem->Len);
				return pItem->Len;
			}
			else//跳过
			{
				Len=pIn[1];
				pIn=&pIn[Len+2];
				Sum+=(Len+2);
				if((Sum+2)>=BufLen) return -1;//越界检查
			}
		}
	}

	return -1;
}


