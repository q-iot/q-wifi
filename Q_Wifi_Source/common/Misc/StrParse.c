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

本文件定义了字符串解析到参数的方法
*/
//---------------------------------------------------------------------------------------------//
#include "SysDefines.h"
#include "StrParse.h"

//处理类似&typ=1&exp=1539865800&cnt=5的参数字符串
//name解析到pParam
//值解析到pVal
//返回解析的参数个数
u8 StrParamParse(char *pParamStr,char **pName,char **pVal)
{
	u16 StrLen=strlen(pParamStr);
	u16 Num=0,i,n,m;
	
	if(pParamStr==NULL || pParamStr[0]==0 || StrLen==0) return 0;

	//Debug("Str[%u]:%s\n\r",StrLen,pParamStr);

	//分离参数
	pName[Num++]=pParamStr;
	for(i=0;i<StrLen;i++)
	{
		if(pParamStr[i]=='&' && pParamStr[i+1])
		{
			pParamStr[i]=0;
			if(Num>=STR_PARAM_MAX_NUM) break;
			pName[Num++]=&pParamStr[i+1];
		}
	}

	//分离参数值
	for(i=0;i<Num;i++)
	{
		char *pChr=strchr(pName[i],'=');
		if(pChr!=NULL)
		{
			pChr[0]=0;
			pChr++;
			pVal[i]=pChr;
		}
	}

	//参数值转码
	for(i=0;i<Num;i++)
	{
		char *pValTmp=pVal[i];
		char AsciiChr=0;
		u16 ValLen=strlen(pValTmp);
		
		n=0;m=0;
		while(pValTmp[n])
		{
			if(pValTmp[n]=='%')
			{
				char NumStr[6];
				NumStr[0]='0';
				NumStr[1]='x';
				NumStr[2]=pValTmp[n+1];
				NumStr[3]=pValTmp[n+2];
				NumStr[4]=0;

				AsciiChr=Str2Uint(NumStr)&0xff;
				if(AsciiChr<0x80)//ascii
				{
					pValTmp[m]=AsciiChr;
					n+=2;	
				}
				else //utf-8 code
				{
					pValTmp[m]=0xA8;
					pValTmp[m+1]=0x81;
					m+=1;
					n+=8;
				}								
			}
			else
			{
				if(pValTmp[n]=='+') pValTmp[m]=' ';//加号转空格
				else pValTmp[m]=pValTmp[n];
			}
			
			n++;
			m++;

			if(n >= ValLen) break;
		}

		pValTmp[m]=0;
	}

	return Num;
}

//通用的串口输入处理，处理以空格间隔的字符串命令
//pStr 字符串
//pRet 返回的指针数组，[0]指向cmd，[1]指向第一个参数，以此类推
//pBuf 外部自行申请的buf，函数调用完成后，需自行free
//CmdToLower 命令名转小写
//返回处理的参数个数，包括命令
u8 StrCmdParse(const char *pCmdStr,char **pRet,char *pBuf,bool CmdToLower)
{
	u16 StrLen=strlen(pCmdStr);
	u16 Num=0,i;
	
	if(StrLen==0 || pCmdStr==NULL || pRet==NULL || pBuf==NULL)//控制字符
	{
		return 0;
	}

	strcpy(pBuf,pCmdStr);//拷贝到buf中，用于参数分解

	if(CmdToLower)
	{
		for(i=0;i<StrLen;i++)//命令字符串全部转小写
		{
			if(pBuf[i]>='A' && pBuf[i]<='Z')
			{
				pBuf[i]=pBuf[i]+32;
			}
			else if(pBuf[i]==' ')
			{
				break;
			}
		}
	}
	
	pRet[Num++]=pBuf;
	for(i=0;pBuf[i];i++)//取参数
	{
		if(pBuf[i]==' ')
		{
			pBuf[i]=0;
			if(pBuf[i+1]&&pBuf[i+1]!=' ')
			{
				if(Num>=STR_CMD_MAX_PARAM_NUM) break;
				pRet[Num++]=&pBuf[i+1];
			}
		}
	}
	
	return Num;
}


