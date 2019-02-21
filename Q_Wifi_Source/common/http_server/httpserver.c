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

���ļ�������һ��֧�ֶ�̬������web������
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

//4�ļ���Դ����
enum{
	WRT_NULL=0,
	WRT_HTML,//��ҳ
	WRT_CSS,//css
	WRT_PNG,//pngͼƬ
	WRT_JS,//javascript	
};
typedef u8 WEB_RES_TYPE;

typedef struct{
	const char *pFileName;
	WEB_RES_TYPE Type;
	bool DirectOutput;//����������ʶ��ֱ�����
	u32 FileSize;
	const char *pRes;
	u32 Sector;//base FM_WEB_FILES_BASE_SECTOR
	u16 SecNum;//ռ��������Ŀ��һ������4096�ֽ�
}WEB_RESOURCE;//4�ļ���Դ��
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

{"",WRT_NULL,TRUE,0,NULL,0,0}//�����Դ˳�Ա��β
};

//4��ҳ�ļ���̬��������
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
}WEB_PARAM;//4������ȡ�б�
static const WEB_PARAM gWebParams[]=
{
{"soft_ver",WPT_NUM,SDN_SYS,SIN_SoftVer,0},
{"hw_ver",WPT_NUM,SDN_SYS,SIN_HwVer,0},
{"dut_id",WPT_NUM,SDN_SYS,SIN_SN,0},

{"",WPT_NULL,0,0,0}//�����Դ˳�Ա��β
};

//�Զ����洢���ļ���ĩβ����0������
#define TXT_END_STR "<!--END-->" //�����ַ���
#define TXT_END_STR_LEN 10 //�����ַ����ĳ���
#define VAR_KEY_MAX_LEN 32 //�����滻�ؼ��ֵ�����ȣ�������ʼ�ͽ�����${}

#if 0
static void _html_file_zero_end(void)
{
	const WEB_RESOURCE *pItem=gWebResources;
	char *pData=Q_Zalloc(1024);	
	u8 Zero[]={0,0,0,0};

	while(pItem->pFileName[0])//���ļ��б���ѯ
	{
		Debug("\n\rCheck:%s\n\r",pItem->pFileName);
		if(pItem->Type==WRT_HTML)
		{
			if(pItem->pRes==NULL && pItem->SecNum && (pItem->Sector+pItem->SecNum)<=FM_WEB_FILES_SEC_NUM)
			{
				u32 Addr=(FM_WEB_FILES_BASE_SECTOR+pItem->Sector)*FLASH_SECTOR_BYTES;
				u32 EndAddr=(FM_WEB_FILES_BASE_SECTOR+pItem->Sector+pItem->SecNum)*FLASH_SECTOR_BYTES;
				
				for(;Addr<=EndAddr-1024;Addr+=512)//�����ֽ���������Ա�������С����
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
				
				for(;Addr<=EndAddr-1024;Addr+=512)//�����ֽ���������Ա�������С����
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

//��������������ַ��������
static bool _get_val_output(const char *pVarStr,char *pOutStr)
{
	const WEB_PARAM *pParam=gWebParams;
	bool InParamList=FALSE;

	if(pVarStr==NULL || pVarStr[0]==0 || pOutStr==NULL)
	{
		return FALSE;
	}

	pOutStr[0]=0;
	while(InParamList==FALSE && pParam->pName[0])//�ӱ����б���ѯ
	{
		if(strcmp(pParam->pName,pVarStr)==0)
		{
			InParamList=TRUE;
			if(pParam->Type==WPT_STR) //��ȡ�ַ���
			{
				QDB_GetValue(pParam->DbName,pParam->DbItem,pParam->DbIntParam,pOutStr);
				return TRUE;				
			}
			else if(pParam->Type==WPT_NUM) //��ȡ����
			{
				u32 Num=QDB_GetValue(pParam->DbName,pParam->DbItem,pParam->DbIntParam,NULL);
				sprintf(pOutStr,"%u",Num);
				return TRUE;
			}			
		}

		pParam++;//����һ������
	}

	//�����б�δ�ҵ�
	if(InParamList==FALSE)
	{
		return http_get_user_val(pVarStr,pOutStr);
	}	

	return FALSE;
}

//��һ����ҳ�ַ����еı���ȫ��ת���������outbuf��
//������ַ�����Ӧ�ô���һ��tcp����
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

		if(pTmp==NULL)//δ�ҵ��κα�ʶ��ֱ�����
		{
			//Debug("1[%u]%s\n\r###############\n\r",strlen(pOutTmp),pOutTmp);
			//netconn_write(pConn,pOutTmp,strlen(pOutTmp),NETCONN_NOCOPY);//ֱ�����
			sprintf(&pOutBuf[OutLen],"%s",pOutTmp);
			OutLen+=strlen(pOutTmp);
		}
		else //�ҵ���ͷ��ʶ
		{
			if(pTmp2==NULL || (pTmp2-pTmp)>VAR_KEY_MAX_LEN) //δ�ҵ�β��ʶ�����ʶ��̫��
			{
				ReadIdx=pTmp-pOutTmp+2;//��ȡָ��������
				goto FindKeyword;
			}
			else //�ҵ���β��ʶ
			{
				char *pValKey=Q_Zalloc(VAR_KEY_MAX_LEN+2);//�����ؼ���
				char *pValStr=Q_Zalloc(VAR_OUTPUT_BUF_LEN);//�����������

				//Debug(" 2[%u]",pTmp-pOutTmp);
				//DisplayStrN(pOutTmp,pTmp-pOutTmp);
				//Debug("\n\r###############\n\r");
				//netconn_write(pConn,pOutTmp,pTmp-pOutTmp,NETCONN_NOCOPY);//�������ǰ������
				sprintf(&pOutBuf[OutLen],"%s",pOutTmp);
				OutLen+=(pTmp-pOutTmp);
			
				//��ʼ�滻����Ϊʵ��ֵ�����
				MemCpy(pValKey,&pTmp[2],pTmp2-pTmp-2);
				pValKey[pTmp2-pTmp-2]=0;
				
				if(_get_val_output(pValKey,pValStr)==TRUE) //���б��ҵ��˱���
				{
					pValStr[VAR_OUTPUT_BUF_LEN-1]=0;
					//Debug("3[%u]%s\n\r###############\n\r",strlen(pValStr),pValStr);
					//Debug(" 3[%u]",strlen(pValStr));
					//netconn_write(pConn,pValStr,strlen(pValStr),NETCONN_NOCOPY);//�������ǰ������
					sprintf(&pOutBuf[OutLen],"%s",pValStr);
					OutLen+=strlen(pValStr);
				}
				else //δ���б��ҵ��˱���
				{
					//Debug(" 4[%u]",strlen(pValKey));
					//netconn_write(pConn,pValKey,strlen(pValKey),NETCONN_NOCOPY);//�������ǰ������
					//netconn_write(pConn,"=null",5,NETCONN_NOCOPY);//�������ǰ������
					sprintf(&pOutBuf[OutLen],"%s=null",pValKey);
					OutLen+=(strlen(pValKey)+5);
				}

				Q_Free(pValKey);
				Q_Free(pValStr);
				
				pOutTmp=&pTmp2[1];//��λpOut�����������
				ReadIdx=0;
				goto FindKeyword;	
			}
		}
	}

	return OutLen;
}

//�ҵ���Ҫ�滻�ı�����ǩ������
static u16 _find_vals_tag(char *pWeb,u16 Len,char *Tail,u16 TailLen,char *pEndStr)
{
	u32 i;
	
	if(pWeb[Len-1]=='$')//�������
	{
		Tail[0]='$';
		TailLen=1;
		Len-=1;
		pWeb[Len]=0;
	}
	else if(pWeb[Len-2]=='$' && pWeb[Len-1]=='{')//�������
	{
		Tail[0]='$';Tail[1]='{';
		TailLen=2;
		Len-=2;
		pWeb[Len]=0;
	}
	else
	{
		//�Ӻ���ǰ�ҵ�һ�γ��ֵ��ַ�$
		pEndStr=pWeb;
#if 1 //�Զ�����Һ���				
		for(i=Len-1;i>=Len-VAR_KEY_MAX_LEN;i--)
		{
			if(pWeb[i]=='$') 
			{
				pEndStr=&pWeb[i];
				break;
			}
		}
#else
		pEndStr=strrchr(&pWeb[Len-VAR_KEY_MAX_LEN],'$');//�������ֵ�һ��
#endif						
		//Debug("strrchr2[%d] %x %x\n\r",pEndStr-pWeb,pEndStr,pWeb);
		if(pEndStr!=NULL && pEndStr[1]=='{')//�ҵ��˱�����ʼ�ַ�
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

//���������������ҳ�ı�
#define WEB_OUTPUT_BUF_LEN 1500
static bool http_get_web(NET_CONN_T *pConn,const char *pUrl,const WEB_RESOURCE *pItem)
{
	if(pItem->pRes!=NULL)//�������ȡ
	{
		if(pItem->Type==WRT_HTML) netconn_write(pConn,http_200_html,sizeof(http_200_html)-1,NETCONN_NOCOPY);//�������ǰ������
		else if(pItem->Type==WRT_CSS) netconn_write(pConn,http_200_css,sizeof(http_200_css)-1,NETCONN_NOCOPY);//�������ǰ������
		else if(pItem->Type==WRT_JS) netconn_write(pConn,http_200_js,sizeof(http_200_js)-1,NETCONN_NOCOPY);//�������ǰ������

		if(pItem->DirectOutput) 
		{
			netconn_write(pConn,pItem->pRes,strlen(pItem->pRes),NETCONN_NOCOPY);//ֱ�����
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
	else if(pItem->SecNum && (pItem->Sector+pItem->SecNum)<=FM_WEB_FILES_SEC_NUM)//��flash��ȡ
	{
		char *pStrBuf=Q_Zalloc(1050);//�����뼸���ֽڷŽ�����
		char *pEndStr=NULL;
		u8 Tail[VAR_KEY_MAX_LEN+2];//�����뼸���ֽڷŽ�����
		u32 TailLen=0,TailLenAlign4=0;
		u32 Addr,EndAddr;

		if(pItem->Type==WRT_HTML) netconn_write(pConn,http_200_html,sizeof(http_200_html)-1,NETCONN_NOCOPY);//�������ǰ������
		else if(pItem->Type==WRT_CSS) netconn_write(pConn,http_200_css,sizeof(http_200_css)-1,NETCONN_NOCOPY);//�������ǰ������
		else if(pItem->Type==WRT_JS) netconn_write(pConn,http_200_js,sizeof(http_200_js)-1,NETCONN_NOCOPY);//�������ǰ������
		
		Addr=(FM_WEB_FILES_BASE_SECTOR+pItem->Sector)*FLASH_SECTOR_BYTES;
		EndAddr=(FM_WEB_FILES_BASE_SECTOR+pItem->Sector+pItem->SecNum)*FLASH_SECTOR_BYTES;
		for(;Addr<EndAddr;Addr+=1024)//�����ֽ���������Ա�������С����
		{
			u32 i=0;
			u32 Len=0;
			bool NeedEnd=FALSE;
			char *pWeb=NULL;

			//���ϴε����ݺͱ��ε�����һ���Ƶ����������
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
			SpiFlsReadData(Addr,1024,(void *)&pStrBuf[TailLenAlign4]);//4�˴�TailLen��Ҫ���ֽڶ��룬�����crash
			pWeb[Len]=0;

FindTxtEnd:
			//���ҿ�����������û�н�����
			pEndStr=strstr(pWeb,TXT_END_STR);
			if(pEndStr!=NULL) //�ҵ���������
			{
				Len=(u32)pEndStr-(u32)pWeb;
				pWeb[Len]=0;
				NeedEnd=TRUE;
				goto Output;
			}
			else //û���ҵ����������᲻���ǽ��������ض���
			{
				char *pEndBuf=Q_Zalloc(64);
				SpiFlsReadData(Addr-16,16+TXT_END_STR_LEN,(void *)pEndBuf);//��ǰ��һ�㣬�����жϽ�����
				pEndBuf[16+TXT_END_STR_LEN]=0;

				pEndStr=strstr(pEndBuf,TXT_END_STR);//Ѱ���ĵ�������
				if(pEndStr!=NULL) //�ҵ��ˣ���ʾȷʵ���ض��� //���û�ҵ���û�б��ضϣ������ļ���δ����
				{
					Len=Len-16+(pEndStr-pEndBuf);
					pWeb[Len]=0;	
					NeedEnd=TRUE;
					Q_Free(pEndBuf);
					goto Output;
				}		
				Q_Free(pEndBuf);
			}

Output: //��ʼ�滻���
			//Debug("Output[%u]\r\n",Len);
			Tail[0]=0;TailLen=0;
			if(pItem->DirectOutput) //ֱ����������滻����
			{
				//if(strcmp(pUrl,"/jquery.js")){Debug("WRITE:");DisplayStrN(pStrBuf,Len);}
				//Debug("Out1[%u]\n\r",Len);
				netconn_write(pConn,pWeb,Len,NETCONN_NOCOPY);//ֱ�����
				//if(strcmp(pUrl,"/jquery.js"))Debug("END\n\r");
			}
			else //��Ҫ�滻����
			{
				char *pOutBuf=NULL;
				
				if(NeedEnd==FALSE) TailLen=_find_vals_tag(pWeb,Len,Tail,TailLen,pEndStr);//�����һ�η�����ҳ����

				//���ؼ��ֻ����������
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

//���ͼƬ�ļ�
static bool http_get_file(NET_CONN_T *pConn,const char *pUrl,const WEB_RESOURCE *pItem)
{
	if(pItem->pRes!=NULL)//�������ȡ
	{
		//not support
	}
	else if(pItem->SecNum && (pItem->Sector+pItem->SecNum)<=FM_WEB_FILES_SEC_NUM)//��flash��ȡ
	{
		char *pData=Q_Zalloc(1024);
		u32 Addr,EndAddr;

		if(pItem->Type==WRT_PNG) netconn_write(pConn,http_200_png,sizeof(http_200_png)-1,NETCONN_NOCOPY);
		else netconn_write(pConn,http_200_html,sizeof(http_200_html)-1,NETCONN_NOCOPY);
		
		Addr=(FM_WEB_FILES_BASE_SECTOR+pItem->Sector)*FLASH_SECTOR_BYTES;
		EndAddr=(FM_WEB_FILES_BASE_SECTOR+pItem->Sector+pItem->SecNum)*FLASH_SECTOR_BYTES;
		for(;Addr<EndAddr;Addr+=1024)//�����ֽ���������Ա�������С����
		{
			SpiFlsReadData(Addr,1024,(void *)pData);
			netconn_write(pConn,pData,1024,NETCONN_NOCOPY);//ֱ�����
		}

		Q_Free(pData);
		return TRUE;
	}

	return FALSE;
}

//����get����
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

	while(InFileList==FALSE && pItem->pFileName[0])//���ļ��б���ѯ
	{
		if(strlen(pItem->pFileName)==UrlLen && strncmp(pItem->pFileName,pUrl,UrlLen)==0)//���ļ��б��ҵ����ļ�
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

		pItem++;//����һ���ļ�
	}

	if(InFileList==FALSE)//���û�ҵ�Ԥ����ģ��Ͷ����û��ص��Լ�����
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

//����post����
static const char *http_post_handler(NET_CONN_T *pConn,const char *pUrl,char *pParamStr,u32 Len)
{
	const char *pRetUrl="";
	
	if(pConn==NULL || pUrl==NULL || pUrl[0]==0 || pParamStr==NULL || pParamStr[0]==0 || Len==0) return pRetUrl;
	Debug("post %s[%u]:%s\n\r",pUrl,Len,pParamStr);

	if(1)
	{
		u16 Num=0,i,n,m;
		char **pParam=Q_Zalloc(STR_PARAM_MAX_NUM*sizeof(char *));//֧�����32������
		char **pVal=Q_Zalloc(STR_PARAM_MAX_NUM*sizeof(char *));

		Num=StrParamParse(pParamStr,pParam,pVal);	
		pRetUrl=http_post_user_handler(pUrl,Num,(const char **)pParam,(const char **)pVal);//custom������

		Q_Free(pParam);
		Q_Free(pVal);
	}

	return pRetUrl;
}

//����ǰ������
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
		do //��ȡ����
		{
		     netbuf_data(NetBuf,&p, &Len);//��ȡָ��ͳ���
		     MemCpy(&pData[RecvLen],p,Len);//ֱ�ӿ�������
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
			ContentLen=Str2Uint(pParams);//����len
			if(ContentLen==0) goto GetHandler;

			pParams=strstr(pParams,"\r\n\r\n");
			if(pParams==NULL) goto GetHandler;

			pParams[4+ContentLen]=0;
			pJumpUrl=http_post_handler(pConn,pUrl,&pParams[4],ContentLen);
			
GetHandler:
			if(pJumpUrl!=NULL && pJumpUrl[0]!=0) //�ض���
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

