#include "SysDefines.h"
#include "StrParse.h"

//��������&typ=1&exp=1539865800&cnt=5�Ĳ����ַ���
//name������pParam
//ֵ������pVal
//���ؽ����Ĳ�������
u8 StrParamParse(char *pParamStr,char **pName,char **pVal)
{
	u16 StrLen=strlen(pParamStr);
	u16 Num=0,i,n,m;
	
	if(pParamStr==NULL || pParamStr[0]==0 || StrLen==0) return 0;

	//Debug("Str[%u]:%s\n\r",StrLen,pParamStr);

	//�������
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

	//�������ֵ
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

	//����ֵת��
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
				if(pValTmp[n]=='+') pValTmp[m]=' ';//�Ӻ�ת�ո�
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

//ͨ�õĴ������봦�������Կո������ַ�������
//pStr �ַ���
//pRet ���ص�ָ�����飬[0]ָ��cmd��[1]ָ���һ���������Դ�����
//pBuf �ⲿ���������buf������������ɺ�������free
//CmdToLower ������תСд
//���ش���Ĳ�����������������
u8 StrCmdParse(const char *pCmdStr,char **pRet,char *pBuf,bool CmdToLower)
{
	u16 StrLen=strlen(pCmdStr);
	u16 Num=0,i;
	
	if(StrLen==0 || pCmdStr==NULL || pRet==NULL || pBuf==NULL)//�����ַ�
	{
		return 0;
	}

	strcpy(pBuf,pCmdStr);//������buf�У����ڲ����ֽ�

	if(CmdToLower)
	{
		for(i=0;i<StrLen;i++)//�����ַ���ȫ��תСд
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
	for(i=0;pBuf[i];i++)//ȡ����
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


