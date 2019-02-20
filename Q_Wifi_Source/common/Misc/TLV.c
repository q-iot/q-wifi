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

//�Ὣֵ���뵽pOut��ĩβ
//�������峤��
//���󷵻�0
u16 TLV_Build(u8 *pOut,u16 BufLen,u8 Type,u8 *ValueStr)
{
	u16 Len,Sum=0;
	u8 i;
	
	for(i=0;i<0xff;i++)//���������̫��
	{
		if(pOut[0]==0)//����λ��
		{
			//��ʼ���
			Len=strlen((void *)ValueStr)+1;
			if(Len>0xfe) return 0; 
			pOut[0]=Type;
			pOut[1]=Len;
			pOut+=2;
			MemCpy(pOut,ValueStr,Len);
			pOut[Len-1]=0;//������
			pOut[Len]=0;//����һ��tlv��type��ֵ����ֹ�´�Խ��
			return Sum+Len+2;
		}
		else//��ֵ
		{
			Len=pOut[1];
			pOut=&pOut[Len+2];
			Sum+=(Len+2);
			if((Sum+3+strlen((void *)ValueStr))>=BufLen) return 0;//Խ����
		}
	}

	return 0;
}

//����tlv
//idx��1��ʼ
//�������ݱ�������pItem��
//���󷵻�-1����ȷ���س���
s16 TLV_Decode(u8 *pIn,u16 BufLen,u16 Idx,TLV_DATA *pItem)
{
	u16 Len,Sum=0;
	u8 i;

	for(i=1;i<=0xff;i++)//�������̫��
	{
		if(pIn[0]==0)//����λ��
		{
			return -1;
		}
		else//��ֵ
		{
			if(i==Idx)//ȡֵ
			{
				pItem->Type=pIn[0];
				pItem->Len=pIn[1];
				MemCpy(pItem->Str,&pIn[2],pItem->Len);
				return pItem->Len;
			}
			else//����
			{
				Len=pIn[1];
				pIn=&pIn[Len+2];
				Sum+=(Len+2);
				if((Sum+2)>=BufLen) return -1;//Խ����
			}
		}
	}

	return -1;
}


