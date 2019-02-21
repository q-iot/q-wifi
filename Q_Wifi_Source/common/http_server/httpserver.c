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

本文件定义了一个支持动态变量的web服务器
*/
//---------------------------------------------------------------------------------------------//
#include "SysDefines.h"
#include "StrParse.h"
#include "httpserver.h"
#include "httpserver_user.h"

static char http_200_html[]  = "HTTP/1.1 200 OK\r\nContent-type: text/html\r\nConnection: close\r\n\r\n";
static char http_200_css[] = "HTTP/1.1 200 OK\r\nContent-type: text/css\r\nConnection: close\r\n\r\n";
static char http_200_png[] = "HTTP/1.1 200 OK\r\nContent-type: image/png\r\nConnection: close\r\n\r\n";
static char http_200_js[] = "HTTP/1.1 200 OK\r\nContent-type: application/x-javascript\r\nConnection: close\r\n\r\n";

static char http_404_html[] = "HTTP/1.1 404 Not Found\r\nContent-type: text/html\r\nConnection: close\r\n\r\n";
static char http_404_css[] = "HTTP/1.1 404 Not Found\r\nContent-type: text/css\r\nConnection: close\r\n\r\n";
static char http_404_png[] = "HTTP/1.1 404 Not Found\r\nContent-type: image/png\r\nConnection: close\r\n\r\n";
static char http_404_js[] = "HTTP/1.1 404 Not Found\r\nContent-type: application/x-javascript\r\nConnection: close\r\n\r\n";

static char http_location[] = "HTTP/1.1 301 Moved Permanently\r\nLocation: %s\r\nConnection: close\r\n\r\n";

//4文件资源定义
enum{
	WRT_NULL=0,
	WRT_HTML,//网页
	WRT_CSS,//css
	WRT_PNG,//png图片
	WRT_JS,//javascript	
};
typedef u8 WEB_RES_TYPE;

typedef struct{
	const char *pFileName;
	WEB_RES_TYPE Type;
	bool DirectOutput;//不理会变量标识符直接输出
	u32 FileSize;
	const char *pRes;
	u32 Sector;//base FM_WEB_FILES_BASE_SECTOR
	u16 SecNum;//占用扇区数目，一个扇区4096字节
}WEB_RESOURCE;//4文件资源表
static const WEB_RESOURCE gWebResources[]=
{
{"/logo.png",	WRT_PNG,	TRUE,	0,NULL,0x00,4},//0x300000
{"/zepto.js",	WRT_JS,		TRUE,	0,NULL,0x04,22},//0x304000
{"/user.js",		WRT_JS,		FALSE,	0,NULL,0x1A,4},//0x31A000
{"/style.css",	WRT_CSS,	TRUE,	0,NULL,0x1E,4},//0x31E000
{"/",				WRT_HTML,	FALSE,	0,NULL,0x22,2},//0x322000
{"/wifi",			WRT_HTML,	FALSE,	0,NULL,0x24,2},//0x324000
{"/ap",				WRT_HTML,	FALSE,	0,NULL,0x26,2},//0x326000
{"/gpio",			WRT_HTML,	FALSE,	0,NULL,0x28,2},//0x324000
{"/msg",			WRT_HTML,	FALSE,	0,NULL,0x2a,2},//0x32A000
{"/vars",			WRT_HTML,	FALSE,	0,NULL,0x2c,2},//0x32C000
{"/misc",			WRT_HTML,	FALSE,	0,NULL,0x2e,2},//0x32E000

{"",WRT_NULL,TRUE,0,NULL,0,0}//必须以此成员结尾
};

//4网页文件动态变量定义
typedef enum{
	WPT_NULL=0,
	WPT_NUM,
	WPT_STR,
	WPT_OTHERS

	
}WEB_PARAM_TYPE;

typedef struct{
	const char *pName;
	WEB_PARAM_TYPE Type;
	SUB_DB_NAME DbName;
	u16 DbItem;
	u32 DbIntParam;
}WEB_PARAM;//4参数获取列表
static const WEB_PARAM gWebParams[]=
{
{"soft_ver",WPT_NUM,SDN_SYS,SIN_SoftVer,0},
{"hw_ver",WPT_NUM,SDN_SYS,SIN_HwVer,0},
{"dut_id",WPT_NUM,SDN_SYS,SIN_SN,0},

{"",WPT_NULL,0,0,0}//必须以此成员结尾
};

//自动将存储区文件的末尾添上0结束符
#define TXT_END_STR "<!--END-->" //结束字符串
#define TXT_END_STR_LEN 10 //结束字符串的长度
#define VAR_KEY_MAX_LEN 32 //变量替换关键字的最长长度，包含开始和结束符${}

#if 0
static void _html_file_zero_end(void)
{
	const WEB_RESOURCE *pItem=gWebResources;
	char *pData=Q_Zalloc(1024);	
	u8 Zero[]={0,0,0,0};

	while(pItem->pFileName[0])//从文件列表轮询
	{
		Debug("\n\rCheck:%s\n\r",pItem->pFileName);
		if(pItem->Type==WRT_HTML)
		{
			if(pItem->pRes==NULL && pItem->SecNum && (pItem->Sector+pItem->SecNum)<=FM_WEB_FILES_SEC_NUM)
			{
				u32 Addr=(FM_WEB_FILES_BASE_SECTOR+pItem->Sector)*FLASH_SECTOR_BYTES;
				u32 EndAddr=(FM_WEB_FILES_BASE_SECTOR+pItem->Sector+pItem->SecNum)*FLASH_SECTOR_BYTES;
				
				for(;Addr<=EndAddr-1024;Addr+=512)//拷贝字节数必须可以被扇区大小整除
				{
					char *pEnd=NULL;

					SpiFlsReadData(Addr,1024,(void *)pData);
					Debug("%u ",Addr);
					
					pEnd=strstr(pData,TXT_END_STR);
					if(pEnd!=NULL)
					{					
						if(pEnd[strlen(TXT_END_STR)]!=0) 
						{
							//SpiFlsWriteData((Addr+(pEnd-pData)+strlen(TXT_END_STR)),4,(void *)Zero);				
							Debug("Set %u\n\r",Addr+(pEnd-pData)+strlen(TXT_END_STR));
						}
						break;
					}
				}
			}			
		}
		else if(pItem->Type==WRT_CSS || pItem->Type==WRT_JS)
		{
			if(pItem->pRes==NULL && pItem->SecNum && (pItem->Sector+pItem->SecNum)<=FM_WEB_FILES_SEC_NUM)
			{
				u32 Addr=(FM_WEB_FILES_BASE_SECTOR+pItem->Sector)*FLASH_SECTOR_BYTES;
				u32 EndAddr=(FM_WEB_FILES_BASE_SECTOR+pItem->Sector+pItem->SecNum)*FLASH_SECTOR_BYTES;
				
				for(;Addr<=EndAddr-1024;Addr+=512)//拷贝字节数必须可以被扇区大小整除
				{
					char *pEnd=NULL;

					SpiFlsReadData(Addr,1024,(void *)pData);Debug("%u ",Addr);
					
					pEnd=strstr(pData,CSS_ZERO_END_STR);
					if(pEnd!=NULL)
					{					
						if(pEnd[strlen(CSS_ZERO_END_STR)]!=0)
						{
							//SpiFlsWriteData((Addr+(pEnd-pData)+strlen(CSS_ZERO_END_STR)),4,(void *)Zero);				
							Debug("Set %u\n\r",Addr+(pEnd-pData)+strlen(CSS_ZERO_END_STR));
						}
						break;
					}
				}
			}			
		}


		pItem++;
	}				

	Q_Free(pData);
}
#endif

//将单个变量变成字符串并输出
static bool _get_val_output(const char *pVarStr,char *pOutStr)
{
	const WEB_PARAM *pParam=gWebParams;
	bool InParamList=FALSE;

	if(pVarStr==NULL || pVarStr[0]==0 || pOutStr==NULL)
	{
		return FALSE;
	}

	pOutStr[0]=0;
	while(InParamList==FALSE && pParam->pName[0])//从变量列表轮询
	{
		if(strcmp(pParam->pName,pVarStr)==0)
		{
			InParamList=TRUE;
			if(pParam->Type==WPT_STR) //获取字符串
			{
				QDB_GetValue(pParam->DbName,pParam->DbItem,pParam->DbIntParam,pOutStr);
				return TRUE;				
			}
			else if(pParam->Type==WPT_NUM) //获取数字
			{
				u32 Num=QDB_GetValue(pParam->DbName,pParam->DbItem,pParam->DbIntParam,NULL);
				sprintf(pOutStr,"%u",Num);
				return TRUE;
			}			
		}

		pParam++;//找下一个变量
	}

	//变量列表未找到
	if(InParamList==FALSE)
	{
		return http_get_user_val(pVarStr,pOutStr);
	}	

	return FALSE;
}

//将一段网页字符串中的变量全部转换并输出到outbuf中
//输入的字符串不应该大于一个tcp包长
#define VAR_OUTPUT_BUF_LEN 200
static u16 _get_vals_handler(char *pOutBuf,const char *pWebStr)
{
	const char *pOutTmp=pWebStr;
	const char *pTmp=NULL;
	const char *pTmp2=NULL;
	u32 ReadIdx=0,OutLen=0;
	
	if(pOutBuf!=NULL)
	{	//Debug("web71\n\r");
FindKeyword:	
		pTmp=strstr(&pOutTmp[ReadIdx],"${");
		if(pTmp!=NULL) pTmp2=strchr(pTmp,'}');

		//Debug("FindFrom:%s\n\r*****************\n\r",&pOutTmp[ReadIdx]);

		if(pTmp==NULL)//未找到任何标识，直接输出
		{
			//Debug("1[%u]%s\n\r###############\n\r",strlen(pOutTmp),pOutTmp);
			//netconn_write(pConn,pOutTmp,strlen(pOutTmp),NETCONN_NOCOPY);//直接输出
			sprintf(&pOutBuf[OutLen],"%s",pOutTmp);
			OutLen+=strlen(pOutTmp);
		}
		else //找到了头标识
		{
			if(pTmp2==NULL || (pTmp2-pTmp)>VAR_KEY_MAX_LEN) //未找到尾标识，或标识符太长
			{
				ReadIdx=pTmp-pOutTmp+2;//读取指针往后移
				goto FindKeyword;
			}
			else //找到了尾标识
			{
				char *pValKey=Q_Zalloc(VAR_KEY_MAX_LEN+2);//变量关键字
				char *pValStr=Q_Zalloc(VAR_OUTPUT_BUF_LEN);//变量内容输出

				//Debug(" 2[%u]",pTmp-pOutTmp);
				//DisplayStrN(pOutTmp,pTmp-pOutTmp);
				//Debug("\n\r###############\n\r");
				//netconn_write(pConn,pOutTmp,pTmp-pOutTmp,NETCONN_NOCOPY);//输出参数前的内容
				sprintf(&pOutBuf[OutLen],"%s",pOutTmp);
				OutLen+=(pTmp-pOutTmp);
			
				//开始替换参数为实际值并输出
				MemCpy(pValKey,&pTmp[2],pTmp2-pTmp-2);
				pValKey[pTmp2-pTmp-2]=0;
				
				if(_get_val_output(pValKey,pValStr)==TRUE) //从列表找到了变量
				{
					pValStr[VAR_OUTPUT_BUF_LEN-1]=0;
					//Debug("3[%u]%s\n\r###############\n\r",strlen(pValStr),pValStr);
					//Debug(" 3[%u]",strlen(pValStr));
					//netconn_write(pConn,pValStr,strlen(pValStr),NETCONN_NOCOPY);//输出参数前的内容
					sprintf(&pOutBuf[OutLen],"%s",pValStr);
					OutLen+=strlen(pValStr);
				}
				else //未从列表找到了变量
				{
					//Debug(" 4[%u]",strlen(pValKey));
					//netconn_write(pConn,pValKey,strlen(pValKey),NETCONN_NOCOPY);//输出参数前的内容
					//netconn_write(pConn,"=null",5,NETCONN_NOCOPY);//输出参数前的内容
					sprintf(&pOutBuf[OutLen],"%s=null",pValKey);
					OutLen+=(strlen(pValKey)+5);
				}

				Q_Free(pValKey);
				Q_Free(pValStr);
				
				pOutTmp=&pTmp2[1];//定位pOut到后面的内容
				ReadIdx=0;
				goto FindKeyword;	
			}
		}
	}

	return OutLen;
}

//找到需要替换的变量标签并处理
static u16 _find_vals_tag(char *pWeb,u16 Len,char *Tail,u16 TailLen,char *pEndStr)
{
	u32 i;
	
	if(pWeb[Len-1]=='$')//特殊情况
	{
		Tail[0]='$';
		TailLen=1;
		Len-=1;
		pWeb[Len]=0;
	}
	else if(pWeb[Len-2]=='$' && pWeb[Len-1]=='{')//特殊情况
	{
		Tail[0]='$';Tail[1]='{';
		TailLen=2;
		Len-=2;
		pWeb[Len]=0;
	}
	else
	{
		//从后往前找第一次出现的字符$
		pEndStr=pWeb;
#if 1 //自定义查找函数				
		for(i=Len-1;i>=Len-VAR_KEY_MAX_LEN;i--)
		{
			if(pWeb[i]=='$') 
			{
				pEndStr=&pWeb[i];
				break;
			}
		}
#else
		pEndStr=strrchr(&pWeb[Len-VAR_KEY_MAX_LEN],'$');//找最后出现的一次
#endif						
		//Debug("strrchr2[%d] %x %x\n\r",pEndStr-pWeb,pEndStr,pWeb);
		if(pEndStr!=NULL && pEndStr[1]=='{')//找到了变量开始字符
		{
			if(strchr(&pEndStr[2],'}')==NULL)
			{
				strcpy(Tail,pEndStr);
				TailLen=strlen(Tail);							
				Len=(u32)pEndStr-(u32)pWeb;
				pWeb[Len]=0;								
			}
		}
	}

	return TailLen;
}

//输出包含变量的网页文本
#define WEB_OUTPUT_BUF_LEN 1500
static bool http_get_web(NET_CONN_T *pConn,const char *pUrl,const WEB_RESOURCE *pItem)
{
	if(pItem->pRes!=NULL)//从数组读取
	{
		if(pItem->Type==WRT_HTML) netconn_write(pConn,http_200_html,sizeof(http_200_html)-1,NETCONN_NOCOPY);//输出参数前的内容
		else if(pItem->Type==WRT_CSS) netconn_write(pConn,http_200_css,sizeof(http_200_css)-1,NETCONN_NOCOPY);//输出参数前的内容
		else if(pItem->Type==WRT_JS) netconn_write(pConn,http_200_js,sizeof(http_200_js)-1,NETCONN_NOCOPY);//输出参数前的内容

		if(pItem->DirectOutput) 
		{
			netconn_write(pConn,pItem->pRes,strlen(pItem->pRes),NETCONN_NOCOPY);//直接输出
		}
		else 
		{
			char *pOutBuf=Q_Zalloc(WEB_OUTPUT_BUF_LEN);
			u16 OutLen=_get_vals_handler(pOutBuf,pItem->pRes);
			netconn_write(pConn,pOutBuf,strlen(pOutBuf),NETCONN_NOCOPY);
			Q_Free(pOutBuf);
		}
		return TRUE;
	}
	else if(pItem->SecNum && (pItem->Sector+pItem->SecNum)<=FM_WEB_FILES_SEC_NUM)//从flash读取
	{
		char *pStrBuf=Q_Zalloc(1050);//多申请几个字节放结束符
		char *pEndStr=NULL;
		u8 Tail[VAR_KEY_MAX_LEN+2];//多申请几个字节放结束符
		u32 TailLen=0,TailLenAlign4=0;
		u32 Addr,EndAddr;

		if(pItem->Type==WRT_HTML) netconn_write(pConn,http_200_html,sizeof(http_200_html)-1,NETCONN_NOCOPY);//输出参数前的内容
		else if(pItem->Type==WRT_CSS) netconn_write(pConn,http_200_css,sizeof(http_200_css)-1,NETCONN_NOCOPY);//输出参数前的内容
		else if(pItem->Type==WRT_JS) netconn_write(pConn,http_200_js,sizeof(http_200_js)-1,NETCONN_NOCOPY);//输出参数前的内容
		
		Addr=(FM_WEB_FILES_BASE_SECTOR+pItem->Sector)*FLASH_SECTOR_BYTES;
		EndAddr=(FM_WEB_FILES_BASE_SECTOR+pItem->Sector+pItem->SecNum)*FLASH_SECTOR_BYTES;
		for(;Addr<EndAddr;Addr+=1024)//拷贝字节数必须可以被扇区大小整除
		{
			u32 i=0;
			u32 Len=0;
			bool NeedEnd=FALSE;
			char *pWeb=NULL;

			//将上次的内容和本次的内容一起复制到输出缓存区
			if(TailLen)
			{
				TailLenAlign4=AlignTo4(TailLen);
				pWeb=&pStrBuf[TailLenAlign4-TailLen];
				MemCpy(pWeb,Tail,TailLen);				
				Len=1024+TailLen;				
			}
			else
			{
				TailLenAlign4=0;
				pWeb=pStrBuf;
				Len=1024;
			}

			//Debug("[%x]Read Len %u %u %u %x %x %x\r\n",Addr,TailLen,TailLenAlign4,Len,pStrBuf,pWeb,(void *)&pStrBuf[TailLenAlign4]);
			SpiFlsReadData(Addr,1024,(void *)&pStrBuf[TailLenAlign4]);//4此处TailLen需要四字节对齐，否则会crash
			pWeb[Len]=0;

FindTxtEnd:
			//找找拷贝出来的有没有结束串
			pEndStr=strstr(pWeb,TXT_END_STR);
			if(pEndStr!=NULL) //找到结束串了
			{
				Len=(u32)pEndStr-(u32)pWeb;
				pWeb[Len]=0;
				NeedEnd=TRUE;
				goto Output;
			}
			else //没有找到结束串，会不会是结束串被截断了
			{
				char *pEndBuf=Q_Zalloc(64);
				SpiFlsReadData(Addr-16,16+TXT_END_STR_LEN,(void *)pEndBuf);//往前读一点，避免切断结束符
				pEndBuf[16+TXT_END_STR_LEN]=0;

				pEndStr=strstr(pEndBuf,TXT_END_STR);//寻找文档结束符
				if(pEndStr!=NULL) //找到了，表示确实被截断了 //如果没找到，没有被截断，表明文件还未结束
				{
					Len=Len-16+(pEndStr-pEndBuf);
					pWeb[Len]=0;	
					NeedEnd=TRUE;
					Q_Free(pEndBuf);
					goto Output;
				}		
				Q_Free(pEndBuf);
			}

Output: //开始替换输出
			//Debug("Output[%u]\r\n",Len);
			Tail[0]=0;TailLen=0;
			if(pItem->DirectOutput) //直接输出不用替换变量
			{
				//if(strcmp(pUrl,"/jquery.js")){Debug("WRITE:");DisplayStrN(pStrBuf,Len);}
				//Debug("Out1[%u]\n\r",Len);
				netconn_write(pConn,pWeb,Len,NETCONN_NOCOPY);//直接输出
				//if(strcmp(pUrl,"/jquery.js"))Debug("END\n\r");
			}
			else //需要替换变量
			{
				char *pOutBuf=NULL;
				
				if(NeedEnd==FALSE) TailLen=_find_vals_tag(pWeb,Len,Tail,TailLen,pEndStr);//非最后一次发送网页内容

				//将关键字换成内容输出
				//Debug("getVals[%u]%u\r\n%s\r\n************************\r\n",Len,strlen(pWeb),pWeb);
				//_get_vals_handler(pConn,pWeb);	
				
				pOutBuf=Q_Zalloc(WEB_OUTPUT_BUF_LEN);
				Len=_get_vals_handler(pOutBuf,pWeb);
				//Debug("Out2[%u]\n\r",Len);
				netconn_write(pConn,pOutBuf,strlen(pOutBuf),NETCONN_NOCOPY);
				Q_Free(pOutBuf);
			}
			
			if(NeedEnd)break;			
		}

		Q_Free(pStrBuf);
		return TRUE;		
	}		

	return FALSE;
}

//输出图片文件
static bool http_get_file(NET_CONN_T *pConn,const char *pUrl,const WEB_RESOURCE *pItem)
{
	if(pItem->pRes!=NULL)//从数组读取
	{
		//not support
	}
	else if(pItem->SecNum && (pItem->Sector+pItem->SecNum)<=FM_WEB_FILES_SEC_NUM)//从flash读取
	{
		char *pData=Q_Zalloc(1024);
		u32 Addr,EndAddr;

		if(pItem->Type==WRT_PNG) netconn_write(pConn,http_200_png,sizeof(http_200_png)-1,NETCONN_NOCOPY);
		else netconn_write(pConn,http_200_html,sizeof(http_200_html)-1,NETCONN_NOCOPY);
		
		Addr=(FM_WEB_FILES_BASE_SECTOR+pItem->Sector)*FLASH_SECTOR_BYTES;
		EndAddr=(FM_WEB_FILES_BASE_SECTOR+pItem->Sector+pItem->SecNum)*FLASH_SECTOR_BYTES;
		for(;Addr<EndAddr;Addr+=1024)//拷贝字节数必须可以被扇区大小整除
		{
			SpiFlsReadData(Addr,1024,(void *)pData);
			netconn_write(pConn,pData,1024,NETCONN_NOCOPY);//直接输出
		}

		Q_Free(pData);
		return TRUE;
	}

	return FALSE;
}

//处理get请求
static void http_get_handler(NET_CONN_T *pConn,const char *pUrl)
{
	const WEB_RESOURCE *pItem=gWebResources;
	bool InFileList=FALSE;
	bool HasOutput=FALSE;
	const char *pEnd=NULL;
	u16 UrlLen=0;
	
	if(pConn==NULL || pUrl==NULL || pUrl[0]==0) return;

	pEnd=strchr(pUrl,'?');
	if(pEnd!=NULL)
	{
		UrlLen=pEnd-pUrl;
	}
	else
	{
		UrlLen=strlen(pUrl);
	}

	while(InFileList==FALSE && pItem->pFileName[0])//从文件列表轮询
	{
		if(strlen(pItem->pFileName)==UrlLen && strncmp(pItem->pFileName,pUrl,UrlLen)==0)//从文件列表找到了文件
		{
			InFileList=TRUE;
			switch(pItem->Type)
			{
				case WRT_HTML:
				case WRT_CSS:
				case WRT_JS:
					{
						Debug("get web[%s]:\n\r",pUrl);
						HasOutput=http_get_web(pConn,pUrl,pItem);
					}
					break;
				case WRT_PNG:
					{
						Debug("get file[%s]:\n\r",pUrl);
						HasOutput=http_get_file(pConn,pUrl,pItem);
					}
					break;
			}	
		}

		pItem++;//找下一个文件
	}

	if(InFileList==FALSE)//如果没找到预定义的，就丢给用户回调自己处理
	{
		HasOutput=http_get_user_handler(pConn,pUrl);
		if(HasOutput) InFileList=TRUE;
	}

	if(InFileList==FALSE || HasOutput==FALSE)//not in web file list
	{
		if(strstr(pUrl,".html")!=NULL) netconn_write(pConn,http_404_html,sizeof(http_404_html)-1,NETCONN_NOCOPY);
		else if(strstr(pUrl,".css")!=NULL) netconn_write(pConn,http_404_css,sizeof(http_404_css)-1,NETCONN_NOCOPY);
		else if(strstr(pUrl,".png")!=NULL) netconn_write(pConn,http_404_png,sizeof(http_404_png)-1,NETCONN_NOCOPY);
		else if(strstr(pUrl,".js")!=NULL) netconn_write(pConn,http_404_js,sizeof(http_404_js)-1,NETCONN_NOCOPY);
		else netconn_write(pConn,http_404_html,sizeof(http_404_html)-1,NETCONN_NOCOPY);		
	}
}

//处理post请求
static const char *http_post_handler(NET_CONN_T *pConn,const char *pUrl,char *pParamStr,u32 Len)
{
	const char *pRetUrl="";
	
	if(pConn==NULL || pUrl==NULL || pUrl[0]==0 || pParamStr==NULL || pParamStr[0]==0 || Len==0) return pRetUrl;
	Debug("post %s[%u]:%s\n\r",pUrl,Len,pParamStr);

	if(1)
	{
		u16 Num=0,i,n,m;
		char **pParam=Q_Zalloc(STR_PARAM_MAX_NUM*sizeof(char *));//支持最多32个参数
		char **pVal=Q_Zalloc(STR_PARAM_MAX_NUM*sizeof(char *));

		Num=StrParamParse(pParamStr,pParam,pVal);	
		pRetUrl=http_post_user_handler(pUrl,Num,(const char **)pParam,(const char **)pVal);//custom处理函数

		Q_Free(pParam);
		Q_Free(pVal);
	}

	return pRetUrl;
}

//处理前端请求
static void http_server_handler(NET_CONN_T *pConn)
{
	NET_BUF_T *NetBuf;
	NET_ERR_T Err;
	const char *pJumpUrl="";
	char *pData=Q_Zalloc(2000);
	void *p=NULL;
	u16 RecvLen=0,Len=0;

	Err = netconn_recv(pConn, &NetBuf);

	if (Err == ERR_OK) 
	{
		do //读取数据
		{
		     netbuf_data(NetBuf,&p, &Len);//获取指针和长度
		     MemCpy(&pData[RecvLen],p,Len);//直接拷贝内容
		     RecvLen+=Len;
		}while(netbuf_next(NetBuf) >= 0);
		netbuf_delete(NetBuf);
		
		if(RecvLen>=5 && strncmp(pData,"GET ",4)==0)
		{
			char *pUrl=&pData[4];
			char *pEnd=NULL;

			pEnd=strchr(pUrl,' ');
			if(pEnd==NULL) goto HandlerEnd;
			*pEnd=0;

			http_get_handler(pConn,pUrl);
		}
		else if(RecvLen>=6 && strncmp(pData,"POST ",5)==0)
		{
			char *pUrl=&pData[5];
			char *pEnd=NULL;
			char *pParams=NULL;
			u32 ContentLen=0;
			
			pEnd=strchr(pUrl,' ');	
			if(pEnd==NULL) goto HandlerEnd;
			*pEnd++=0;
			
			pParams=strstr(pEnd,"Content-Length");
			if(pParams==NULL) goto GetHandler;
			pParams=strchr(pParams,':');
			if(pParams==NULL) goto GetHandler;
			pParams++;
			ContentLen=Str2Uint(pParams);//剥离len
			if(ContentLen==0) goto GetHandler;

			pParams=strstr(pParams,"\r\n\r\n");
			if(pParams==NULL) goto GetHandler;

			pParams[4+ContentLen]=0;
			pJumpUrl=http_post_handler(pConn,pUrl,&pParams[4],ContentLen);
			
GetHandler:
			if(pJumpUrl!=NULL && pJumpUrl[0]!=0) //重定向
			{
				char *pLocal=Q_Zalloc(1024);
				sprintf(pLocal,http_location,pJumpUrl);
				netconn_write(pConn,pLocal,strlen(pLocal),NETCONN_NOCOPY);
				Q_Free(pLocal);				
			}
			else 
			{
				http_get_handler(pConn,pUrl);
			}
		}
	}

HandlerEnd:
	if(pData!=NULL) Q_Free(pData);
}

static void http_server_thread(void *arg)
{
  	NET_CONN_T *Conn, *NewConn;
  	NET_ERR_T Err;
  
  	LWIP_UNUSED_ARG(arg);  
	Conn = netconn_new(NETCONN_TCP);
	LWIP_ERROR("http_server: invalid Conn", (Conn != NULL), return;);
	netconn_bind(Conn,IP_ADDR_ANY,80);
	netconn_listen(Conn);

	do{
		Err = netconn_accept(Conn, &NewConn);
		if (Err == ERR_OK) 
		{
			http_server_handler(NewConn);
			netconn_close(NewConn);
			netconn_delete(NewConn);
		}
	}while(Err == ERR_OK);

	LWIP_DEBUGF(LWIP_DBG_ON,("http_server_thread: netconn_accept received error %d, shutting down",Err));
	netconn_close(Conn);
	netconn_delete(Conn);
}

void http_server_init(void)
{
	sys_thread_new("http_server",http_server_thread,NULL,512,4);
}

