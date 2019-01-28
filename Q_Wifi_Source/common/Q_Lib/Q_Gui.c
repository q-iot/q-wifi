#include "SysDefines.h"
#include "Q_Gui.h"

#define Gui_Debug Debug
#define LCD_Lock()
#define LCD_UnLock()
#define LCD_BlukWriteDataStart()
#define LCD_Fill Oled_Fill
#define LCD_DrawRegion Oled_DrawRegion

//用于获取spi flash里面的字库
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

//从存储体中读取字库显示汉字
//返回实际读取的byte
//注意region.space和region.row_space用于控制字距,且必须设置值!!!
static u16 Gui_DrawFont_GBK16(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//计算字符占用位置，按像素表示
	u32 FontLocal;	//字库偏移量
	u8 CharBuf[GUI_GBK16_FONT_BUF_SIZE];
	u8 w,h,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	COLOR_TYPE Color;

	//Color=pRegion->Color;

	LCD_Lock();
	
	while((Offset=*pStr++)!=0) //开始读取字符串
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//显示汉字
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//越底界检查
			if((RowLen+GUI_GBK16_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			FontLocal=((Offset-GUI_GBK_FONT_OFFSET_H)*GUI_GBK_FONT_PAGENUM+
				((*pStr)-GUI_GBK_FONT_OFFSET_L))<<5;	//获取偏移量,<<5= *GUI_GBK16_FONT_BUF_SIZE
			if((*pStr++) > GUI_GBK_FONT_OFFSET_P) FontLocal-=GUI_GBK16_FONT_BUF_SIZE; //除去字库中0x7f这条线
			
			//移到字库对应位置读取一个字的点阵数据,32个字节
			Gui_ReadFontLib(GUI_GBK16_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK16_FONT_BUF_SIZE);
			LCD_DrawRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK16_CHAR_WIDTH,GUI_GBK16_FONT_HEIGHT,CharBuf);
			
			ColLen+=(GUI_GBK16_CHAR_WIDTH+Space);
			//越边界检查
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK16_CHAR_WIDTH;
			else w=GUI_GBK16_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//如果超过预设宽度
			{				
				ColLen=0;
				RowLen+=(GUI_GBK16_FONT_HEIGHT+RowSpace);
			}

			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//显示符号
		{
			//Debug("C:%c ",*(pStr-1));
			//越底界检查
			if((RowLen+GUI_GBK16_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//检测符号类型
			{
				case 0x0A:	//换行回车的情况
				{
					ColLen=0;
					RowLen+=(GUI_GBK16_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //普通字符的情况
				{
					FontLocal=GUI_GBK16_FONT_END+Offset*GUI_GBK16_FONT_BUF_SIZE;//获取字库中ASCii的偏移量
					
					//移到字库对应位置读取一个字的点阵数据,32个字节
					Gui_ReadFontLib(GUI_GBK16_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK16_FONT_BUF_SIZE);
					for(h=2;h<GUI_GBK16_FONT_BUF_SIZE;h+=2) CharBuf[h>>1]=CharBuf[h];//将另外一半没用的去掉
					LCD_DrawRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK16_ASCII_WIDTH,GUI_GBK16_FONT_HEIGHT,CharBuf);
					
					ColLen+=(GUI_GBK16_ASCII_WIDTH+Space);
					//越边界检查
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK16_CHAR_WIDTH;
					else w=GUI_GBK16_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//如果超过预设宽度
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

//返回实际区域需要的字符串byte数
//返回值实际就是Gui_DrawFont_GBK16的返回值，只不过不显示实际内容
static u16 Gui_DrawFont_GBK16_Num(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//计算字符占用位置，按像素表示
	u8 w,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	
	while((Offset=*pStr++)!=0) //开始读取字符串
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//显示汉字
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//越底界检查
			if((RowLen+GUI_GBK16_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			pStr++;
			
			ColLen+=(GUI_GBK16_CHAR_WIDTH+Space);
			//越边界检查
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK16_CHAR_WIDTH;
			else w=GUI_GBK16_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//如果超过预设宽度
			{				
				ColLen=0;
				RowLen+=(GUI_GBK16_FONT_HEIGHT+RowSpace);
			}
			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//显示符号
		{
			//Debug("C:%c ",*(pStr-1));
			//越底界检查
			if((RowLen+GUI_GBK16_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//检测符号类型
			{
				case 0x0A:	//换行回车的情况
				{
					ColLen=0;
					RowLen+=(GUI_GBK16_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //普通字符的情况
				{					
					ColLen+=(GUI_GBK16_ASCII_WIDTH+Space);
					//越边界检查
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK16_CHAR_WIDTH;
					else w=GUI_GBK16_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//如果超过预设宽度
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

//从存储体中读取字库显示汉字
//返回实际读取的byte
//注意region.space和region.row_space用于控制字距,且必须设置值!!!
static u16 Gui_DrawFont_GBK12(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//计算字符占用位置，按像素表示
	u32 FontLocal;	//字库偏移量
	u8 CharBuf[GUI_GBK12_FONT_BUF_SIZE];
	u8 w,h,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	COLOR_TYPE Color;

	//Color=pRegion->Color;

	LCD_Lock();
	
	while((Offset=*pStr++)!=0) //开始读取字符串
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//显示汉字
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//越底界检查
			if((RowLen+GUI_GBK12_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			FontLocal=((Offset-GUI_GBK_FONT_OFFSET_H)*GUI_GBK_FONT_PAGENUM+
				((*pStr)-GUI_GBK_FONT_OFFSET_L))*GUI_GBK12_FONT_BUF_SIZE;	//从第一个字节得到偏移量
			if((*pStr++) > GUI_GBK_FONT_OFFSET_P) FontLocal-=GUI_GBK12_FONT_BUF_SIZE; //除去字库中0x7f这条线
			
			//移到字库对应位置读取一个字的点阵数据,24个字节
			Gui_ReadFontLib(GUI_GBK12_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK12_FONT_BUF_SIZE);
			LCD_DrawRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK12_CHAR_WIDTH,GUI_GBK12_FONT_HEIGHT,CharBuf);
			
			ColLen+=(GUI_GBK12_CHAR_WIDTH+Space);
			//越边界检查
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK12_CHAR_WIDTH;
			else w=GUI_GBK12_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//如果超过预设宽度
			{				
				ColLen=0;
				RowLen+=(GUI_GBK12_FONT_HEIGHT+RowSpace);
			}

			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//显示符号
		{
			//Debug("C:%c ",*(pStr-1));
			//越底界检查
			if((RowLen+GUI_GBK12_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//检测符号类型
			{
				case 0x0A:	//换行回车的情况
				{
					ColLen=0;
					RowLen+=(GUI_GBK12_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //普通字符的情况
				{
					FontLocal=GUI_GBK12_FONT_END+Offset*GUI_GBK12_FONT_BUF_SIZE;//获取字库中ASCii的偏移量
					
					//移到字库对应位置读取一个字的点阵数据,24个字节
					Gui_ReadFontLib(GUI_GBK12_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK12_FONT_BUF_SIZE);
					for(h=2;h<GUI_GBK12_FONT_BUF_SIZE;h+=2) CharBuf[h>>1]=CharBuf[h];//将另外一半没用的去掉
					LCD_DrawRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK12_ASCII_WIDTH,GUI_GBK12_FONT_HEIGHT,CharBuf);
					
					ColLen+=(GUI_GBK12_ASCII_WIDTH+Space);
					//越边界检查
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK12_CHAR_WIDTH;
					else w=GUI_GBK12_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//如果超过预设宽度
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

//返回实际区域需要的字符串byte数
//返回值实际就是Gui_DrawFont_GBK12的返回值，只不过不显示实际内容
static u16 Gui_DrawFont_GBK12_Num(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//计算字符占用位置，按像素表示
	u8 w,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	
	while((Offset=*pStr++)!=0) //开始读取字符串
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//显示汉字
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//越底界检查
			if((RowLen+GUI_GBK12_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			pStr++;
			
			ColLen+=(GUI_GBK12_CHAR_WIDTH+Space);
			//越边界检查
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK12_CHAR_WIDTH;
			else w=GUI_GBK12_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//如果超过预设宽度
			{				
				ColLen=0;
				RowLen+=(GUI_GBK12_FONT_HEIGHT+RowSpace);
			}
			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//显示符号
		{
			//Debug("C:%c ",*(pStr-1));
			//越底界检查
			if((RowLen+GUI_GBK12_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//检测符号类型
			{
				case 0x0A:	//换行回车的情况
				{
					ColLen=0;
					RowLen+=(GUI_GBK12_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //普通字符的情况
				{					
					ColLen+=(GUI_GBK12_ASCII_WIDTH+Space);
					//越边界检查
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK12_CHAR_WIDTH;
					else w=GUI_GBK12_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//如果超过预设宽度
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

//从存储体中读取字库显示汉字
//返回实际读取的byte
//注意region.space和region.row_space用于控制字距,且必须设置值!!!
static u16 Gui_DrawFont_ASC14B(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//计算字符占用位置，按像素表示
	u32 FontLocal;	//字库偏移量
	u8 CharBuf[GUI_ASC14B_FONT_BUF_SIZE];
	u8 w,h,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	COLOR_TYPE Color;

	//Color=pRegion->Color;

	LCD_Lock();
	while((Offset=*pStr++)!=0) //开始读取字符串
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//显示汉字
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//越底界检查
			if((RowLen+GUI_ASC14B_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}

#if 0 //此字库不支持汉字			
			FontLocal=((Offset-GUI_GBK_FONT_OFFSET_H)*GUI_GBK_FONT_PAGENUM+
				((*pStr)-GUI_GBK_FONT_OFFSET_L))*GUI_ASC14B_FONT_BUF_SIZE;	//从第一个字节得到偏移量
			if((*pStr++) > GUI_GBK_FONT_OFFSET_P) FontLocal-=GUI_ASC14B_FONT_BUF_SIZE; //除去字库中0x7f这条线
			
			//移到字库对应位置读取一个字的点阵数据,24个字节
			Gui_ReadFontLib(GUI_ASC14B_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_ASC14B_FONT_BUF_SIZE);
#else
			pStr++;
#endif

#if 1		//由于不支持汉字，显示填充块
			MemSet(CharBuf,0xff,GUI_ASC14B_FONT_BUF_SIZE);
 			LCD_DrawRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_ASC14B_CHAR_WIDTH,GUI_ASC14B_FONT_HEIGHT,CharBuf);
 #endif

			ColLen+=(GUI_ASC14B_CHAR_WIDTH+Space);
			//越边界检查
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_ASC14B_CHAR_WIDTH;
			else w=GUI_ASC14B_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//如果超过预设宽度
			{				
				ColLen=0;
				RowLen+=(GUI_ASC14B_FONT_HEIGHT+RowSpace);
			}

			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//显示符号
		{
			//Debug("C:%c ",*(pStr-1));
			//越底界检查
			if((RowLen+GUI_ASC14B_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//检测符号类型
			{
				case 0x0A:	//换行回车的情况
				{
					ColLen=0;
					RowLen+=(GUI_ASC14B_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //普通字符的情况
				{
					FontLocal=Offset*GUI_ASC14B_FONT_BUF_SIZE;//获取字库中ASCii的偏移量
					
					//移到字库对应位置读for(i=100;i;i--) os_delay_us(1000);取一个字的点阵数据,28个字节
					Gui_ReadFontLib(GUI_ASC14B_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_ASC14B_FONT_BUF_SIZE);
					for(h=2;h<GUI_ASC14B_FONT_BUF_SIZE;h+=2) CharBuf[h>>1]=CharBuf[h];//将另外一半没用的去掉
					LCD_DrawRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_ASC14B_ASCII_WIDTH,GUI_ASC14B_FONT_HEIGHT,CharBuf);
					
					ColLen+=(GUI_ASC14B_ASCII_WIDTH+Space);
					//越边界检查
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_ASC14B_CHAR_WIDTH;
					else w=GUI_ASC14B_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//如果超过预设宽度
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

//返回实际区域需要的字符串byte数
//返回值实际就是Gui_DrawFont_GBK12的返回值，只不过不显示实际内容
static u16 Gui_DrawFont_ASC14B_Num(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//计算字符占用位置，按像素表示
	u8 w,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	
	while((Offset=*pStr++)!=0) //开始读取字符串
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//显示汉字
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//越底界检查
			if((RowLen+GUI_ASC14B_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			pStr++;
			
			ColLen+=(GUI_ASC14B_CHAR_WIDTH+Space);
			//越边界检查
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_ASC14B_CHAR_WIDTH;
			else w=GUI_ASC14B_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//如果超过预设宽度
			{				
				ColLen=0;
				RowLen+=(GUI_ASC14B_FONT_HEIGHT+RowSpace);
			}
			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//显示符号
		{
			//Debug("C:%c ",*(pStr-1));
			//越底界检查
			if((RowLen+GUI_ASC14B_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//检测符号类型
			{
				case 0x0A:	//换行回车的情况
				{
					ColLen=0;
					RowLen+=(GUI_ASC14B_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //普通字符的情况
				{					
					ColLen+=(GUI_ASC14B_ASCII_WIDTH+Space);
					//越边界检查
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_ASC14B_CHAR_WIDTH;
					else w=GUI_ASC14B_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//如果超过预设宽度
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

//从存储体中读取字库显示汉字
//返回实际读取的byte
//注意region.space和region.row_space用于控制字距,且必须设置值!!!
static u16 Gui_DrawFont_GBK21(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//计算字符占用位置，按像素表示
	u32 FontLocal;	//字库偏移量
	u8 CharBuf[GUI_GBK21_FONT_BUF_SIZE];
	u8 w,h,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	COLOR_TYPE Color;

	//Color=pRegion->Color;

	LCD_Lock();
	
	while((Offset=*pStr++)!=0) //开始读取字符串
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//显示汉字
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//越底界检查
			if((RowLen+GUI_GBK21_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			FontLocal=((Offset-GUI_GBK_FONT_OFFSET_H)*GUI_GBK_FONT_PAGENUM+
				((*pStr)-GUI_GBK_FONT_OFFSET_L))*GUI_GBK21_FONT_BUF_SIZE;	//获取偏移量,<<x= *GUI_GBK21_FONT_BUF_SIZE
			if((*pStr++) > GUI_GBK_FONT_OFFSET_P) FontLocal-=GUI_GBK21_FONT_BUF_SIZE; //除去字库中0x7f这条线
			
			//移到字库对应位置读取一个字的点阵数据,63个字节
			Gui_ReadFontLib(GUI_GBK21_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK21_FONT_BUF_SIZE);
			LCD_DrawRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK21_CHAR_WIDTH,GUI_GBK21_FONT_HEIGHT,CharBuf);
			
			ColLen+=(GUI_GBK21_CHAR_WIDTH+Space);
			//越边界检查
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK21_CHAR_WIDTH;
			else w=GUI_GBK21_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//如果超过预设宽度
			{				
				ColLen=0;
				RowLen+=(GUI_GBK21_FONT_HEIGHT+RowSpace);
			}

			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//显示符号
		{
			//Debug("C:%c ",*(pStr-1));
			//越底界检查
			if((RowLen+GUI_GBK21_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//检测符号类型
			{
				case 0x0A:	//换行回车的情况
				{
					ColLen=0;
					RowLen+=(GUI_GBK21_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //普通字符的情况
				{
					FontLocal=GUI_GBK21_FONT_END+Offset*GUI_GBK21_FONT_BUF_SIZE;//获取字库中ASCii的偏移量
					
					//移到字库对应位置读取一个字的点阵数据,63个字节
					Gui_ReadFontLib(GUI_GBK21_FONT_SPI_FLASH_BASE,FontLocal,CharBuf,GUI_GBK21_FONT_BUF_SIZE);
					for(h=3,w=2;h<GUI_GBK21_FONT_BUF_SIZE;h+=3,w+=2)//将另外一半没用的去掉
					{						
						CharBuf[w]=CharBuf[h];
						CharBuf[w+1]=CharBuf[h+1];
					}					
					LCD_DrawRegion(pRegion->x+ColLen,pRegion->y+RowLen,GUI_GBK21_ASCII_WIDTH,GUI_GBK21_FONT_HEIGHT,CharBuf);
					
					ColLen+=(GUI_GBK21_ASCII_WIDTH+Space);
					//越边界检查
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK21_CHAR_WIDTH;
					else w=GUI_GBK21_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//如果超过预设宽度
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

//返回实际区域需要的字符串byte数
//返回值实际就是Gui_DrawFont_GBK21的返回值，只不过不显示实际内容
static u16 Gui_DrawFont_GBK21_Num(const u8 *Str,const GUI_REGION *pRegion)
{
	u8 *pStr=(u8 *)Str;
	u16 ColLen=0,RowLen=0;//计算字符占用位置，按像素表示
	u8 w,Offset;
	u8 Space=(pRegion->Space>>4);
	u8 RowSpace=(pRegion->Space&0x0f);
	
	while((Offset=*pStr++)!=0) //开始读取字符串
	{
		if(Offset>=GUI_GBK_FONT_OFFSET_H)//显示汉字
		{
			//Debug("F:%c%c ",*(pStr-1),*pStr);
			//越底界检查
			if((RowLen+GUI_GBK21_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			pStr++;
			
			ColLen+=(GUI_GBK21_CHAR_WIDTH+Space);
			//越边界检查
			if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK21_CHAR_WIDTH;
			else w=GUI_GBK21_ASCII_WIDTH;
			if((ColLen+w) > pRegion->w)//如果超过预设宽度
			{				
				ColLen=0;
				RowLen+=(GUI_GBK21_FONT_HEIGHT+RowSpace);
			}
			//Debug("c:%d r:%d,w %d,h %d\n\r",ColLen,RowLen,pRegion->w,pRegion->h);
		}
		else//显示符号
		{
			//Debug("C:%c ",*(pStr-1));
			//越底界检查
			if((RowLen+GUI_GBK21_FONT_HEIGHT) > pRegion->h )//如果超过预设高度则显示结束
			{	
				//Gui_Debug("Disp Font End:RowLen=%d\n",RowLen);
				break;
			}
			
			switch(Offset)//检测符号类型
			{
				case 0x0A:	//换行回车的情况
				{
					ColLen=0;
					RowLen+=(GUI_GBK21_FONT_HEIGHT+RowSpace);
				}
				case 0x0D:break;
				default: //普通字符的情况
				{					
					ColLen+=(GUI_GBK21_ASCII_WIDTH+Space);
					//越边界检查
					if(*pStr>=GUI_GBK_FONT_OFFSET_H)	w=GUI_GBK21_CHAR_WIDTH;
					else w=GUI_GBK21_ASCII_WIDTH;
					if((ColLen+w) > pRegion->w)//如果超过预设宽度
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

//通用显示字符的函数
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



