#ifndef QSYS_Q_GUI_H
#define QSYS_Q_GUI_H

#include "SpiFlashApi.h"

//Gui配置
#define LCD_WIDTH	128//屏幕宽
#define LCD_HIGHT		32//屏幕高

//字库选择
typedef enum{
	GBK12_FONT=0,//写12x12的字
	GBK12_NUM,//计算区域内可容纳的12x12字数
	GBK16_FONT,//写字
	GBK16_NUM,//计数
	ASC14B_FONT,//写字
	ASC14B_NUM,//计数
	GBK21_FONT,//写字
	GBK21_NUM,//计数
}FONT_ACT;

//GBK字库定义
#define GUI_GBK_FONT_OFFSET_H		0x81
#define GUI_GBK_FONT_OFFSET_L		0x40
#define GUI_GBK_FONT_OFFSET_P		0x7F
#define GUI_GBK_FONT_PAGENUM    	0xBE //=0xFE-0x40+1-1

//GBK16x16字库
#define GUI_GBK16_FONT_HEIGHT	16			//字体高度是16
#define GUI_GBK16_CHAR_WIDTH	16			//汉字字符宽16
#define GUI_GBK16_ASCII_WIDTH	8			//ascii字符宽8
#define GUI_GBK16_FONT_BUF_SIZE 	32 //一个汉字需要多少个字节来存储。=GUI_GBK_FONT_SIZE/8xGUI_GBK_FONT_SIZE
#define GUI_GBK16_FONT_SPI_FLASH_BASE		(FM_FONT_G16_BASE_SECTOR*FLASH_SECTOR_BYTES)// DOWNLOAD_START_PAGE*DOWNLOAD_PAGE_SIZE存储字库的起始页
#define GUI_GBK16_FONT_END				(23940*GUI_GBK16_FONT_BUF_SIZE) //字库结束点 即Ascii库起点

//GBK12x12字库
#define GUI_GBK12_FONT_HEIGHT	12		//字体高度是12
#define GUI_GBK12_CHAR_WIDTH	12			//汉字字符宽12
#define GUI_GBK12_ASCII_WIDTH	6			//ascii字符宽6
#define GUI_GBK12_FONT_BUF_SIZE 	24 //一个汉字需要多少个字节来存储。=GUI_GBK_FONT_SIZE/8xGUI_GBK_FONT_SIZE
#define GUI_GBK12_FONT_SPI_FLASH_BASE		(FM_FONT_G12_BASE_SECTOR*FLASH_SECTOR_BYTES)// DOWNLOAD_START_PAGE*DOWNLOAD_PAGE_SIZE存储字库的起始页
#define GUI_GBK12_FONT_END	(23940*GUI_GBK12_FONT_BUF_SIZE) //字库结束点 即Ascii库起点

//ASCII 14 BOLD 字库定义
#define GUI_ASC14B_FONT_HEIGHT	14		//字体高度是14
#define GUI_ASC14B_CHAR_WIDTH		15			//汉字字符宽15，实际上此字库不支持汉字
#define GUI_ASC14B_ASCII_WIDTH	8			//ascii字符宽8
#define GUI_ASC14B_FONT_BUF_SIZE 	28 //一个字需要多少个字节来存储。=GUI_GBK_FONT_SIZE/8xGUI_GBK_FONT_SIZE
#define GUI_ASC14B_FONT_SPI_FLASH_BASE		(FM_FONT_A14B_BASE_SECTOR*FLASH_SECTOR_BYTES)// DOWNLOAD_START_PAGE*DOWNLOAD_PAGE_SIZE存储字库的起始页

//GBK21x21字库
#define GUI_GBK21_FONT_HEIGHT	21		//字体高度是12
#define GUI_GBK21_CHAR_WIDTH	21			//汉字字符宽12
#define GUI_GBK21_ASCII_WIDTH	11			//ascii字符宽6
#define GUI_GBK21_FONT_BUF_SIZE 	63 //一个汉字需要多少个字节来存储。=GUI_GBK_FONT_SIZE/8xGUI_GBK_FONT_SIZE
#define GUI_GBK21_FONT_SPI_FLASH_BASE		(FM_FONT_G21_BASE_SECTOR*FLASH_SECTOR_BYTES)// DOWNLOAD_START_PAGE*DOWNLOAD_PAGE_SIZE存储字库的起始页
#define GUI_GBK21_FONT_END	0x170380//16字节对齐=(23940*GUI_GBK21_FONT_BUF_SIZE) //字库结束点 即Ascii库起点

#if 0
//GB2312字库
#define GUI_FONT_GB2312_OFFSET_H			0xA1
#define GUI_FONT_GB2312_OFFSET_L			0xA1
#define GUI_FONT_GB2312_PAGENUM    		0x5E //=0xFE-0xA1+1
#define GUI_FONT_GB2312_SPACE					0		//间距
#define GUI_FONT_GB2312_MARGIN				16		//行距
#define GUI_FONT_GB2312_END						8836*128
#endif


//3	GUI 定义类型

typedef u16 COLOR_TYPE;

typedef struct { 
	u16 x; 	//区域相对左上角x
	u16 y;	//区域相对左上角y
	u16 w; 	//区域宽
	u16 h;	//区域高
	u16 Space;//用来指定间距，在写字时，4-7位表示字间距,0-3位表示行间距
	//bool IsScreen2;//显示到备用屏
}GUI_REGION,*pGUI_REGION;

typedef enum{
	Gui_True=0,
	Gui_False,
	Gui_No_Such_File,//没有读到文件
	Gui_Out_Of_Range//图片显示超出范围
}GUI_RESULT;

//2		GUI API   
//初始化GUI
void Gui_Init(void);

void Gui_Fill(const GUI_REGION *pRegion,bool Dot);

void Gui_Draw(const GUI_REGION *pRegion,const u8 *pData);

#if 0
//在指定区域内写字,超出则不显示
//将pRegion的所有参数都要设置,特别是space参数
//Str表示要写的字符串
//Color表示字体颜色
//字体为16x16
u16 Gui_DrawFont_GBK16(const u8 *Str,const GUI_REGION *pRegion);

//返回实际区域需要的字符串byte数
//返回值实际就是Gui_DrawFont_GBK的返回值，只不过不显示实际内容
u16 Gui_DrawFont_GBK16_Num(const u8 *Str,const GUI_REGION *pRegion);

//在指定区域内写字,超出则不显示
//将pRegion的所有参数都要设置,特别是space参数
//Str表示要写的字符串
//Color表示字体颜色
//字体为12x12
u16 Gui_DrawFont_GBK12(const u8 *Str,const GUI_REGION *pRegion);

//返回实际区域需要的字符串byte数
//返回值实际就是Gui_DrawFont_GBK的返回值，只不过不显示实际内容
u16 Gui_DrawFont_GBK12_Num(const u8 *Str,const GUI_REGION *pRegion);

//在指定区域内写字,超出则不显示
//将pRegion的所有参数都要设置,特别是space参数
//Str表示要写的字符串
//Color表示字体颜色
//字体为9x14,不支持中文
u16 Gui_DrawFont_ASC14B(const u8 *Str,const GUI_REGION *pRegion);

//返回实际区域需要的字符串byte数
//返回值实际就是Gui_DrawFont_GBK的返回值，只不过不显示实际内容
u16 Gui_DrawFont_ASC14B_Num(const u8 *Str,const GUI_REGION *pRegion);
#endif

//通用显示字符的函数
u16 Gui_DrawFont(FONT_ACT FontLib,const u8 *Str,const GUI_REGION *pRegion);




#endif

