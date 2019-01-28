#include "SysDefines.h"
#include "Q_Gui.h"

#define Gui_Debug Debug
#define LCD_Lock()
#define LCD_UnLock()
#define LCD_BlukWriteDataStart()
#define LCD_Fill Oled_Fill
#define LCD_DrawRegion Oled_DrawRegion

//���ڻ�ȡspi flash������ֿ�
static void __inline Gui_ReadFontLib(u32 Base,u32 local,u8 *buf,u32 len)
{
	SpiFlsReadData(Base+local,AlignTo4(len),(void *)buf);
}

void Gui_Init(void)
{
	Gui_Debug("Gui initialize OK!\n\r");
}

void Gui_DeInit(void)
{
	return;
}

void Gui_Fill(const GUI_REGION *pRegion,bool Dot) 
{
	LCD_Fill(pRegion->x,pRegion->y,pRegion->w,pRegion->h,Dot?0xff:0);
}

void Gui_Draw(const GUI_REGION *pRegion,const u8 *pData)
{
	LCD_DrawRegion(pRegion->x,pRegion->y,pRegion->w,pRegion->h,pData);
}

//�Ӵ洢���ж�ȡ�ֿ���ʾ����
//����ʵ�ʶ�ȡ��byte
//ע��region.space��region.row_space���ڿ����־�,�ұ�������ֵ!!!
static u16 Gui_DrawFont_GBK16(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//�����ַ�ռ��λ�ã������ر�ʾ
	u32 FontLocal;	//�ֿ�ƫ����
	u8 CharBuf[GUI_GBK16_FONT_BUF_SIZE];
	u8 w,h,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	COLOR_TYPE Color;

	//Color=pRegion->Color;

	LCD_Lock();
	
	while((Offset=*pStr++)!=0) //��ʼ��ȡ�ַ���
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//��ʾ����
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//Խ�׽���
			if((RowLen+GUI_GBK16_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			FontLocal=((Offset-GUI_GBK_FONT_OFFSET_H)*GUI_GBK_FONT_PAGENUM+
				((*pStr)-GUI_GBK_FONT_OFFSET_L))<<5;	//��ȡƫ����,<<5= *GUI_GBK16_FONT_BUF_SIZE
			if((*pStr++) > GUI_GBK_FONT_OFFSET_P) FontLocal-=GUI_GBK16_FONT_BUF_SIZE; //��ȥ�ֿ���0x7f������
			
			//�Ƶ��ֿ��Ӧλ�ö�ȡһ���ֵĵ�������,32���ֽ�
			Gui_ReadFontLib(GUI_GBK16_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK16_FONT_BUF_SIZE);
			LCD_DrawRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK16_CHAR_WIDTH,GUI_GBK16_FONT_HEIGHT,CharBuf);
			
			ColLen+=(GUI_GBK16_CHAR_WIDTH+Space);
			//Խ�߽���
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK16_CHAR_WIDTH;
			else w=GUI_GBK16_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//�������Ԥ����
			{				
				ColLen=0;
				RowLen+=(GUI_GBK16_FONT_HEIGHT+RowSpace);
			}

			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//��ʾ����
		{
			//Debug("C:%c ",*(pStr-1));
			//Խ�׽���
			if((RowLen+GUI_GBK16_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//����������
			{
				case 0x0A:	//���лس������
				{
					ColLen=0;
					RowLen+=(GUI_GBK16_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //��ͨ�ַ������
				{
					FontLocal=GUI_GBK16_FONT_END+Offset*GUI_GBK16_FONT_BUF_SIZE;//��ȡ�ֿ���ASCii��ƫ����
					
					//�Ƶ��ֿ��Ӧλ�ö�ȡһ���ֵĵ�������,32���ֽ�
					Gui_ReadFontLib(GUI_GBK16_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK16_FONT_BUF_SIZE);
					for(h=2;h<GUI_GBK16_FONT_BUF_SIZE;h+=2) CharBuf[h>>1]=CharBuf[h];//������һ��û�õ�ȥ��
					LCD_DrawRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK16_ASCII_WIDTH,GUI_GBK16_FONT_HEIGHT,CharBuf);
					
					ColLen+=(GUI_GBK16_ASCII_WIDTH+Space);
					//Խ�߽���
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK16_CHAR_WIDTH;
					else w=GUI_GBK16_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//�������Ԥ����
					{				
						ColLen=0;
						RowLen+=(GUI_GBK16_FONT_HEIGHT+RowSpace);
					}
					//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
				}
			}
		}
	}

	LCD_UnLock();
	return (u16)(pStr-Str-1);
}

//����ʵ��������Ҫ���ַ���byte��
//����ֵʵ�ʾ���Gui_DrawFont_GBK16�ķ���ֵ��ֻ��������ʾʵ������
static u16 Gui_DrawFont_GBK16_Num(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//�����ַ�ռ��λ�ã������ر�ʾ
	u8 w,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	
	while((Offset=*pStr++)!=0) //��ʼ��ȡ�ַ���
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//��ʾ����
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//Խ�׽���
			if((RowLen+GUI_GBK16_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			pStr++;
			
			ColLen+=(GUI_GBK16_CHAR_WIDTH+Space);
			//Խ�߽���
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK16_CHAR_WIDTH;
			else w=GUI_GBK16_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//�������Ԥ����
			{				
				ColLen=0;
				RowLen+=(GUI_GBK16_FONT_HEIGHT+RowSpace);
			}
			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//��ʾ����
		{
			//Debug("C:%c ",*(pStr-1));
			//Խ�׽���
			if((RowLen+GUI_GBK16_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//����������
			{
				case 0x0A:	//���лس������
				{
					ColLen=0;
					RowLen+=(GUI_GBK16_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //��ͨ�ַ������
				{					
					ColLen+=(GUI_GBK16_ASCII_WIDTH+Space);
					//Խ�߽���
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK16_CHAR_WIDTH;
					else w=GUI_GBK16_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//�������Ԥ����
					{				
						ColLen=0;
						RowLen+=(GUI_GBK16_FONT_HEIGHT+RowSpace);
					}
					//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
				}
			}
		}
	}

	return (u16)(pStr-Str-1);
}

//�Ӵ洢���ж�ȡ�ֿ���ʾ����
//����ʵ�ʶ�ȡ��byte
//ע��region.space��region.row_space���ڿ����־�,�ұ�������ֵ!!!
static u16 Gui_DrawFont_GBK12(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//�����ַ�ռ��λ�ã������ر�ʾ
	u32 FontLocal;	//�ֿ�ƫ����
	u8 CharBuf[GUI_GBK12_FONT_BUF_SIZE];
	u8 w,h,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	COLOR_TYPE Color;

	//Color=pRegion->Color;

	LCD_Lock();
	
	while((Offset=*pStr++)!=0) //��ʼ��ȡ�ַ���
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//��ʾ����
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//Խ�׽���
			if((RowLen+GUI_GBK12_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			FontLocal=((Offset-GUI_GBK_FONT_OFFSET_H)*GUI_GBK_FONT_PAGENUM+
				((*pStr)-GUI_GBK_FONT_OFFSET_L))*GUI_GBK12_FONT_BUF_SIZE;	//�ӵ�һ���ֽڵõ�ƫ����
			if((*pStr++) > GUI_GBK_FONT_OFFSET_P) FontLocal-=GUI_GBK12_FONT_BUF_SIZE; //��ȥ�ֿ���0x7f������
			
			//�Ƶ��ֿ��Ӧλ�ö�ȡһ���ֵĵ�������,24���ֽ�
			Gui_ReadFontLib(GUI_GBK12_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK12_FONT_BUF_SIZE);
			LCD_DrawRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK12_CHAR_WIDTH,GUI_GBK12_FONT_HEIGHT,CharBuf);
			
			ColLen+=(GUI_GBK12_CHAR_WIDTH+Space);
			//Խ�߽���
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK12_CHAR_WIDTH;
			else w=GUI_GBK12_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//�������Ԥ����
			{				
				ColLen=0;
				RowLen+=(GUI_GBK12_FONT_HEIGHT+RowSpace);
			}

			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//��ʾ����
		{
			//Debug("C:%c ",*(pStr-1));
			//Խ�׽���
			if((RowLen+GUI_GBK12_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//����������
			{
				case 0x0A:	//���лس������
				{
					ColLen=0;
					RowLen+=(GUI_GBK12_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //��ͨ�ַ������
				{
					FontLocal=GUI_GBK12_FONT_END+Offset*GUI_GBK12_FONT_BUF_SIZE;//��ȡ�ֿ���ASCii��ƫ����
					
					//�Ƶ��ֿ��Ӧλ�ö�ȡһ���ֵĵ�������,24���ֽ�
					Gui_ReadFontLib(GUI_GBK12_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK12_FONT_BUF_SIZE);
					for(h=2;h<GUI_GBK12_FONT_BUF_SIZE;h+=2) CharBuf[h>>1]=CharBuf[h];//������һ��û�õ�ȥ��
					LCD_DrawRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK12_ASCII_WIDTH,GUI_GBK12_FONT_HEIGHT,CharBuf);
					
					ColLen+=(GUI_GBK12_ASCII_WIDTH+Space);
					//Խ�߽���
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK12_CHAR_WIDTH;
					else w=GUI_GBK12_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//�������Ԥ����
					{				
						ColLen=0;
						RowLen+=(GUI_GBK12_FONT_HEIGHT+RowSpace);
					}
					//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
				}
			}
		}
	}

	LCD_UnLock();
	return (u16)(pStr-Str-1);
}

//����ʵ��������Ҫ���ַ���byte��
//����ֵʵ�ʾ���Gui_DrawFont_GBK12�ķ���ֵ��ֻ��������ʾʵ������
static u16 Gui_DrawFont_GBK12_Num(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//�����ַ�ռ��λ�ã������ر�ʾ
	u8 w,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	
	while((Offset=*pStr++)!=0) //��ʼ��ȡ�ַ���
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//��ʾ����
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//Խ�׽���
			if((RowLen+GUI_GBK12_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			pStr++;
			
			ColLen+=(GUI_GBK12_CHAR_WIDTH+Space);
			//Խ�߽���
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK12_CHAR_WIDTH;
			else w=GUI_GBK12_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//�������Ԥ����
			{				
				ColLen=0;
				RowLen+=(GUI_GBK12_FONT_HEIGHT+RowSpace);
			}
			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//��ʾ����
		{
			//Debug("C:%c ",*(pStr-1));
			//Խ�׽���
			if((RowLen+GUI_GBK12_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//����������
			{
				case 0x0A:	//���лس������
				{
					ColLen=0;
					RowLen+=(GUI_GBK12_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //��ͨ�ַ������
				{					
					ColLen+=(GUI_GBK12_ASCII_WIDTH+Space);
					//Խ�߽���
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK12_CHAR_WIDTH;
					else w=GUI_GBK12_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//�������Ԥ����
					{				
						ColLen=0;
						RowLen+=(GUI_GBK12_FONT_HEIGHT+RowSpace);
					}
					//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
				}
			}
		}
	}

	return (u16)(pStr-Str-1);
}

//�Ӵ洢���ж�ȡ�ֿ���ʾ����
//����ʵ�ʶ�ȡ��byte
//ע��region.space��region.row_space���ڿ����־�,�ұ�������ֵ!!!
static u16 Gui_DrawFont_ASC14B(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//�����ַ�ռ��λ�ã������ر�ʾ
	u32 FontLocal;	//�ֿ�ƫ����
	u8 CharBuf[GUI_ASC14B_FONT_BUF_SIZE];
	u8 w,h,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	COLOR_TYPE Color;

	//Color=pRegion->Color;

	LCD_Lock();
	while((Offset=*pStr++)!=0) //��ʼ��ȡ�ַ���
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//��ʾ����
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//Խ�׽���
			if((RowLen+GUI_ASC14B_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}

#if 0 //���ֿⲻ֧�ֺ���			
			FontLocal=((Offset-GUI_GBK_FONT_OFFSET_H)*GUI_GBK_FONT_PAGENUM+
				((*pStr)-GUI_GBK_FONT_OFFSET_L))*GUI_ASC14B_FONT_BUF_SIZE;	//�ӵ�һ���ֽڵõ�ƫ����
			if((*pStr++) > GUI_GBK_FONT_OFFSET_P) FontLocal-=GUI_ASC14B_FONT_BUF_SIZE; //��ȥ�ֿ���0x7f������
			
			//�Ƶ��ֿ��Ӧλ�ö�ȡһ���ֵĵ�������,24���ֽ�
			Gui_ReadFontLib(GUI_ASC14B_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_ASC14B_FONT_BUF_SIZE);
#else
			pStr++;
#endif

#if 1		//���ڲ�֧�ֺ��֣���ʾ����
			MemSet(CharBuf,0xff,GUI_ASC14B_FONT_BUF_SIZE);
 			LCD_DrawRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_ASC14B_CHAR_WIDTH,GUI_ASC14B_FONT_HEIGHT,CharBuf);
 #endif

			ColLen+=(GUI_ASC14B_CHAR_WIDTH+Space);
			//Խ�߽���
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_ASC14B_CHAR_WIDTH;
			else w=GUI_ASC14B_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//�������Ԥ����
			{				
				ColLen=0;
				RowLen+=(GUI_ASC14B_FONT_HEIGHT+RowSpace);
			}

			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//��ʾ����
		{
			//Debug("C:%c ",*(pStr-1));
			//Խ�׽���
			if((RowLen+GUI_ASC14B_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//����������
			{
				case 0x0A:	//���лس������
				{
					ColLen=0;
					RowLen+=(GUI_ASC14B_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //��ͨ�ַ������
				{
					FontLocal=Offset*GUI_ASC14B_FONT_BUF_SIZE;//��ȡ�ֿ���ASCii��ƫ����
					
					//�Ƶ��ֿ��Ӧλ�ö�for(i=100;i;i--) os_delay_us(1000);ȡһ���ֵĵ�������,28���ֽ�
					Gui_ReadFontLib(GUI_ASC14B_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_ASC14B_FONT_BUF_SIZE);
					for(h=2;h<GUI_ASC14B_FONT_BUF_SIZE;h+=2) CharBuf[h>>1]=CharBuf[h];//������һ��û�õ�ȥ��
					LCD_DrawRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_ASC14B_ASCII_WIDTH,GUI_ASC14B_FONT_HEIGHT,CharBuf);
					
					ColLen+=(GUI_ASC14B_ASCII_WIDTH+Space);
					//Խ�߽���
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_ASC14B_CHAR_WIDTH;
					else w=GUI_ASC14B_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//�������Ԥ����
					{				
						ColLen=0;
						RowLen+=(GUI_ASC14B_FONT_HEIGHT+RowSpace);
					}
					//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
				}
			}
		}
	}

	LCD_UnLock();
	return (u16)(pStr-Str-1);
}

//����ʵ��������Ҫ���ַ���byte��
//����ֵʵ�ʾ���Gui_DrawFont_GBK12�ķ���ֵ��ֻ��������ʾʵ������
static u16 Gui_DrawFont_ASC14B_Num(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//�����ַ�ռ��λ�ã������ر�ʾ
	u8 w,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	
	while((Offset=*pStr++)!=0) //��ʼ��ȡ�ַ���
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//��ʾ����
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//Խ�׽���
			if((RowLen+GUI_ASC14B_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			pStr++;
			
			ColLen+=(GUI_ASC14B_CHAR_WIDTH+Space);
			//Խ�߽���
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_ASC14B_CHAR_WIDTH;
			else w=GUI_ASC14B_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//�������Ԥ����
			{				
				ColLen=0;
				RowLen+=(GUI_ASC14B_FONT_HEIGHT+RowSpace);
			}
			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//��ʾ����
		{
			//Debug("C:%c ",*(pStr-1));
			//Խ�׽���
			if((RowLen+GUI_ASC14B_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//����������
			{
				case 0x0A:	//���лس������
				{
					ColLen=0;
					RowLen+=(GUI_ASC14B_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //��ͨ�ַ������
				{					
					ColLen+=(GUI_ASC14B_ASCII_WIDTH+Space);
					//Խ�߽���
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_ASC14B_CHAR_WIDTH;
					else w=GUI_ASC14B_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//�������Ԥ����
					{				
						ColLen=0;
						RowLen+=(GUI_ASC14B_FONT_HEIGHT+RowSpace);
					}
					//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
				}
			}
		}
	}

	return (u16)(pStr-Str-1);
}

//�Ӵ洢���ж�ȡ�ֿ���ʾ����
//����ʵ�ʶ�ȡ��byte
//ע��region.space��region.row_space���ڿ����־�,�ұ�������ֵ!!!
static u16 Gui_DrawFont_GBK21(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//�����ַ�ռ��λ�ã������ر�ʾ
	u32 FontLocal;	//�ֿ�ƫ����
	u8 CharBuf[GUI_GBK21_FONT_BUF_SIZE];
	u8 w,h,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	COLOR_TYPE Color;

	//Color=pRegion->Color;

	LCD_Lock();
	
	while((Offset=*pStr++)!=0) //��ʼ��ȡ�ַ���
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//��ʾ����
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//Խ�׽���
			if((RowLen+GUI_GBK21_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			FontLocal=((Offset-GUI_GBK_FONT_OFFSET_H)*GUI_GBK_FONT_PAGENUM+
				((*pStr)-GUI_GBK_FONT_OFFSET_L))*GUI_GBK21_FONT_BUF_SIZE;	//��ȡƫ����,<<x= *GUI_GBK21_FONT_BUF_SIZE
			if((*pStr++) > GUI_GBK_FONT_OFFSET_P) FontLocal-=GUI_GBK21_FONT_BUF_SIZE; //��ȥ�ֿ���0x7f������
			
			//�Ƶ��ֿ��Ӧλ�ö�ȡһ���ֵĵ�������,63���ֽ�
			Gui_ReadFontLib(GUI_GBK21_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK21_FONT_BUF_SIZE);
			LCD_DrawRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK21_CHAR_WIDTH,GUI_GBK21_FONT_HEIGHT,CharBuf);
			
			ColLen+=(GUI_GBK21_CHAR_WIDTH+Space);
			//Խ�߽���
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK21_CHAR_WIDTH;
			else w=GUI_GBK21_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//�������Ԥ����
			{				
				ColLen=0;
				RowLen+=(GUI_GBK21_FONT_HEIGHT+RowSpace);
			}

			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//��ʾ����
		{
			//Debug("C:%c ",*(pStr-1));
			//Խ�׽���
			if((RowLen+GUI_GBK21_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//����������
			{
				case 0x0A:	//���лس������
				{
					ColLen=0;
					RowLen+=(GUI_GBK21_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //��ͨ�ַ������
				{
					FontLocal=GUI_GBK21_FONT_END+Offset*GUI_GBK21_FONT_BUF_SIZE;//��ȡ�ֿ���ASCii��ƫ����
					
					//�Ƶ��ֿ��Ӧλ�ö�ȡһ���ֵĵ�������,63���ֽ�
					Gui_ReadFontLib(GUI_GBK21_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK21_FONT_BUF_SIZE);
					for(h=3,w=2;h<GUI_GBK21_FONT_BUF_SIZE;h+=3,w+=2)//������һ��û�õ�ȥ��
					{						
						CharBuf[w]=CharBuf[h];
						CharBuf[w+1]=CharBuf[h+1];
					}					
					LCD_DrawRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK21_ASCII_WIDTH,GUI_GBK21_FONT_HEIGHT,CharBuf);
					
					ColLen+=(GUI_GBK21_ASCII_WIDTH+Space);
					//Խ�߽���
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK21_CHAR_WIDTH;
					else w=GUI_GBK21_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//�������Ԥ����
					{				
						ColLen=0;
						RowLen+=(GUI_GBK21_FONT_HEIGHT+RowSpace);
					}
					//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
				}
			}
		}
	}

	LCD_UnLock();
	return (u16)(pStr-Str-1);
}

//����ʵ��������Ҫ���ַ���byte��
//����ֵʵ�ʾ���Gui_DrawFont_GBK21�ķ���ֵ��ֻ��������ʾʵ������
static u16 Gui_DrawFont_GBK21_Num(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//�����ַ�ռ��λ�ã������ر�ʾ
	u8 w,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	
	while((Offset=*pStr++)!=0) //��ʼ��ȡ�ַ���
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//��ʾ����
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//Խ�׽���
			if((RowLen+GUI_GBK21_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			pStr++;
			
			ColLen+=(GUI_GBK21_CHAR_WIDTH+Space);
			//Խ�߽���
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK21_CHAR_WIDTH;
			else w=GUI_GBK21_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//�������Ԥ����
			{				
				ColLen=0;
				RowLen+=(GUI_GBK21_FONT_HEIGHT+RowSpace);
			}
			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//��ʾ����
		{
			//Debug("C:%c ",*(pStr-1));
			//Խ�׽���
			if((RowLen+GUI_GBK21_FONT_HEIGHT) > pRegion->h )//�������Ԥ��߶�����ʾ����
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//����������
			{
				case 0x0A:	//���лس������
				{
					ColLen=0;
					RowLen+=(GUI_GBK21_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //��ͨ�ַ������
				{					
					ColLen+=(GUI_GBK21_ASCII_WIDTH+Space);
					//Խ�߽���
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK21_CHAR_WIDTH;
					else w=GUI_GBK21_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//�������Ԥ����
					{				
						ColLen=0;
						RowLen+=(GUI_GBK21_FONT_HEIGHT+RowSpace);
					}
					//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
				}
			}
		}
	}

	return (u16)(pStr-Str-1);
}

//ͨ����ʾ�ַ��ĺ���
u16 Gui_DrawFont(FONT_ACT FontLib,const u8 *Str,const GUI_REGION *pRegion)
{
	switch(FontLib)
	{
		case GBK12_FONT:
			return Gui_DrawFont_GBK12(Str,pRegion);
		case GBK12_NUM:
			return Gui_DrawFont_GBK12_Num(Str,pRegion);
		case GBK16_FONT:
			return Gui_DrawFont_GBK16(Str,pRegion);
		case GBK16_NUM:
			return Gui_DrawFont_GBK16_Num(Str,pRegion);
		case ASC14B_FONT:
			return Gui_DrawFont_ASC14B(Str,pRegion);
		case ASC14B_NUM:
			return Gui_DrawFont_ASC14B_Num(Str,pRegion);
		case GBK21_FONT:
			return Gui_DrawFont_GBK21(Str,pRegion);
		case GBK21_NUM:
			return Gui_DrawFont_GBK21_Num(Str,pRegion);

	}
  
  return 0;
}



