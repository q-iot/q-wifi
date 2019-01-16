#include "esp_common.h"
#include "EspDevelop.h"
#include "uart.h"
#include "oled.h" 
#include "DebugOutput.h"

//ssd1306的驱动

#define OLED_RESET_H() //GPIO_OUTPUT_SET(GPIO_ID_PIN(14),1)
#define OLED_RESET_L() //GPIO_OUTPUT_SET(GPIO_ID_PIN(14),0)
#define OLED_SDA(val) GPIO_OUTPUT_SET(GPIO_ID_PIN(12),val)
#define OLED_SCL(val) GPIO_OUTPUT_SET(GPIO_ID_PIN(14),val)
//#define OLED_SCL(val) gpio16_output_set(val)

#define PAGE_DISP 4
#define WIDTH_DISP 128
#define HEIGHT_DISP 32

#define DISP_BUF_BYTES (PAGE_DISP*WIDTH_DISP)
static volatile u8 gpDisp[PAGE_DISP][WIDTH_DISP]={{0}};//显示缓存

static u8 Oled_DataH(u8 Data)
{
	u8 Out=0;
	if(ReadBit(Data,4)) SetBit(Out,1);
	if(ReadBit(Data,5)) SetBit(Out,3);
	if(ReadBit(Data,6)) SetBit(Out,5);
	if(ReadBit(Data,7)) SetBit(Out,7);
	return Out;
}

static u8 Oled_DataL(u8 Data)
{
	u8 Out=0;
	if(ReadBit(Data,0)) SetBit(Out,1);
	if(ReadBit(Data,1)) SetBit(Out,3);
	if(ReadBit(Data,2)) SetBit(Out,5);
	if(ReadBit(Data,3)) SetBit(Out,7);
	return Out;
}

//i2c开始信号
static void Oled_Start() 
{ 
	OLED_SCL(1); 
	OLED_SDA(1); 
	OLED_SDA(0); 
	OLED_SCL(0); 
} 

//i2c结束信号
static void Oled_Stop() 
{ 
	OLED_SCL(0); 
	OLED_SDA(0); 
	OLED_SDA(1); 
	OLED_SCL(1); 
} 

//传8位指令或数据到OLED显示模块
static void Oled_Transfer(u8 Byte) 
{
	u8 n; 
	
	for(n=0;n<8;n++) 
	{ 
		OLED_SCL(0);      
		OLED_SDA(Byte>>7);  
		Byte<<=1;
		OLED_SCL(1); 
    } 
    OLED_SCL(0); 
    os_delay_us(3);  //   ACK时间
    OLED_SCL(1); 
}

//写指令到OLED显示模块
static void Oled_WriteCmd(u8 Cmd)   
{
	Oled_Start(); 
	Oled_Transfer(0x78); 
	Oled_Transfer(0x00); 
	Oled_Transfer(Cmd); 
	Oled_Stop(); 
}

//写数据到OLED显示模块
static void Oled_WriteData(u8 Data)
{
	Oled_Start(); 
	Oled_Transfer(0x78); 
	Oled_Transfer(0x40); 
	Oled_Transfer(Data); 
	Oled_Stop(); 
} 

//设定地址
static void Oled_Address(u8 Page,u8 Column)
{
	Oled_WriteCmd(0xb0+Page);   			    //设置页地址。每页是8行。一个画面的64行被分成8个页。
	Oled_WriteCmd(((Column>>4)&0x0f)+0x10);	//设置列地址的高4位
	Oled_WriteCmd(Column&0x0f);				//设置列地址的低4位
}

//i2c初始化
void Oled_Init(void)
{
	u32 i=0;

	OLED_RESET_H();
	for(i=100;i;i--) os_delay_us(1000);
	OLED_RESET_L();
	for(i=100;i;i--) os_delay_us(1000);
	OLED_RESET_H();	
	for(i=100;i;i--) os_delay_us(1000);
	
	Oled_WriteCmd(0xAE); //display off
	Oled_WriteCmd(0x20);	//Set Memory Addressing Mode	
	Oled_WriteCmd(0x10);	//00,Horizontal Addressing Mode;01,Vertical Addressing Mode;10,Page Addressing Mode (RESET);11,Invalid
	Oled_WriteCmd(0xb0);	//Set Page Start Address for Page Addressing Mode,0-7
	Oled_WriteCmd(0xc8);	//Set COM Output Scan Direction
	Oled_WriteCmd(0x00); //---set low column address
	Oled_WriteCmd(0x10); //---set high column address
	Oled_WriteCmd(0x40); //--set start line address
	Oled_WriteCmd(0x81); //--set contrast control register
	Oled_WriteCmd(0xff); //亮度调节 0x00~0xff
	Oled_WriteCmd(0xa1); //--set segment re-map 0 to 127
	Oled_WriteCmd(0xa6); //--set normal display
	Oled_WriteCmd(0xa8); //--set multiplex ratio(1 to 64)
	Oled_WriteCmd(0x3F); //
	Oled_WriteCmd(0xa4); //0xa4,Output follows RAM content;0xa5,Output ignores RAM content
	Oled_WriteCmd(0xd3); //-set display offset
	Oled_WriteCmd(0x00); //-not offset
	Oled_WriteCmd(0xd5); //--set display clock divide ratio/oscillator frequency
	Oled_WriteCmd(0xf0); //--set divide ratio
	Oled_WriteCmd(0xd9); //--set pre-charge period
	Oled_WriteCmd(0x22); //
	Oled_WriteCmd(0xda); //--set com pins hardware configuration
	Oled_WriteCmd(0x12);
	Oled_WriteCmd(0xdb); //--set vcomh
	Oled_WriteCmd(0x20); //0x20,0.77xVcc
	Oled_WriteCmd(0x8d); //--set DC-DC enable
	Oled_WriteCmd(0x14); //
	Oled_WriteCmd(0xaf); //--turn on oled panel.

	//Debug("Oled Init Finish\n\r");

	MemSet(gpDisp,0,sizeof(gpDisp));
	Oled_Refresh();
}

//刷新具体的块
void Oled_Brush(u16 X,u16 Y,u16 W,u16 H)
{
	u16 i,j;
	
	//计算占了几个page，占了几个col，从哪一page，哪一col开始刷新屏幕
	{
		u16 StartPage=Y>>3;
		u16 StartCol=X;
		u16 PageTotal=((Y+H)>>3) - (Y>>3) +1;//本算式不能合并为H/8 +1，否则失效

		for(j=0;j<PageTotal;j++)
		{
			if((StartPage+j) < PAGE_DISP) //防止越界
			{
				Oled_Address((StartPage+j)*2,StartCol);
				for(i=0;i<W;i++)
				{	
					if((StartCol+i) < WIDTH_DISP)  //防止越界
						Oled_WriteData(Oled_DataL(gpDisp[StartPage+j][StartCol+i])); 
				}


				Oled_Address((StartPage+j)*2+1,StartCol);
				for(i=0;i<W;i++)
				{	
					if((StartCol+i) < WIDTH_DISP)  //防止越界
						Oled_WriteData(Oled_DataH(gpDisp[StartPage+j][StartCol+i])); 
				}				
			}
		}
	}
}

//pData的大小必须等于[H][(W-1)/8+1]
void Oled_DrawRegion(u16 X,u16 Y,u16 W,u16 H,const u8 *pData)
{
	u16 i,j;
	u16 RowBytes=((W-1)>>3)+1;

	for(i=0;i<W;i++)//更新缓存
	{
		for(j=0;j<H;j++)
		{
			if(((Y+j)>>3)<PAGE_DISP && (X+i)<WIDTH_DISP) //防止越界
			{
				if(ReadBit(pData[j*RowBytes+(i>>3)],7-(i&0x07)))	
				{
					SetBit(gpDisp[(Y+j)>>3][X+i],(Y+j)&0x07);
				}
				else
				{
					ClrBit(gpDisp[(Y+j)>>3][X+i],(Y+j)&0x07);
				}
			}
		}
	}

	Oled_Brush(X,Y,W,H);
}

//往区域填充单一数据
void Oled_Fill(u16 X,u16 Y,u16 W,u16 H,u8 Data)
{
	u16 i,j;
	u16 RowBytes=((W-1)>>3)+1;

	for(i=0;i<W;i++)//更新缓存
	{
		for(j=0;j<H;j++)
		{
			if(((Y+j)>>3)<PAGE_DISP && (X+i)<WIDTH_DISP) //防止越界
			{
				if(ReadBit(Data,7-(i&0x07)))	
				{
					SetBit(gpDisp[(Y+j)>>3][X+i],(Y+j)&0x07);
				}
				else
				{
					ClrBit(gpDisp[(Y+j)>>3][X+i],(Y+j)&0x07);
				}
			}
		}
	}

	Oled_Brush(X,Y,W,H);
}

//刷新整屏
void Oled_Refresh(void)
{
	u32 i,j;
	
	for(j=0;j<PAGE_DISP;j++)
	{
		Oled_Address(j*2,0);
		for(i=0;i<WIDTH_DISP;i++)
		{	
			Oled_WriteData(Oled_DataL(gpDisp[j][i])); 
		}

		Oled_Address(j*2+1,0);
		for(i=0;i<WIDTH_DISP;i++)
		{	
			Oled_WriteData(Oled_DataH(gpDisp[j][i])); 
		}
	}
}

