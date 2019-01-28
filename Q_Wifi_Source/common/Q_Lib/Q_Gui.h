#ifndef QSYS_Q_GUI_H
#define QSYS_Q_GUI_H

#include "SpiFlashApi.h"

//Gui����
#define LCD_WIDTH	128//��Ļ��
#define LCD_HIGHT		32//��Ļ��

//�ֿ�ѡ��
typedef enum{
	GBK12_FONT=0,//д12x12����
	GBK12_NUM,//���������ڿ����ɵ�12x12����
	GBK16_FONT,//д��
	GBK16_NUM,//����
	ASC14B_FONT,//д��
	ASC14B_NUM,//����
	GBK21_FONT,//д��
	GBK21_NUM,//����
}FONT_ACT;

//GBK�ֿⶨ��
#define GUI_GBK_FONT_OFFSET_H		0x81
#define GUI_GBK_FONT_OFFSET_L		0x40
#define GUI_GBK_FONT_OFFSET_P		0x7F
#define GUI_GBK_FONT_PAGENUM    	0xBE //=0xFE-0x40+1-1

//GBK16x16�ֿ�
#define GUI_GBK16_FONT_HEIGHT	16			//����߶���16
#define GUI_GBK16_CHAR_WIDTH	16			//�����ַ���16
#define GUI_GBK16_ASCII_WIDTH	8			//ascii�ַ���8
#define GUI_GBK16_FONT_BUF_SIZE 	32 //һ��������Ҫ���ٸ��ֽ����洢��=GUI_GBK_FONT_SIZE/8xGUI_GBK_FONT_SIZE
#define GUI_GBK16_FONT_SPI_FLASH_BASE		(FM_FONT_G16_BASE_SECTOR*FLASH_SECTOR_BYTES)// DOWNLOAD_START_PAGE*DOWNLOAD_PAGE_SIZE�洢�ֿ����ʼҳ
#define GUI_GBK16_FONT_END				(23940*GUI_GBK16_FONT_BUF_SIZE) //�ֿ������ ��Ascii�����

//GBK12x12�ֿ�
#define GUI_GBK12_FONT_HEIGHT	12		//����߶���12
#define GUI_GBK12_CHAR_WIDTH	12			//�����ַ���12
#define GUI_GBK12_ASCII_WIDTH	6			//ascii�ַ���6
#define GUI_GBK12_FONT_BUF_SIZE 	24 //һ��������Ҫ���ٸ��ֽ����洢��=GUI_GBK_FONT_SIZE/8xGUI_GBK_FONT_SIZE
#define GUI_GBK12_FONT_SPI_FLASH_BASE		(FM_FONT_G12_BASE_SECTOR*FLASH_SECTOR_BYTES)// DOWNLOAD_START_PAGE*DOWNLOAD_PAGE_SIZE�洢�ֿ����ʼҳ
#define GUI_GBK12_FONT_END	(23940*GUI_GBK12_FONT_BUF_SIZE) //�ֿ������ ��Ascii�����

//ASCII 14 BOLD �ֿⶨ��
#define GUI_ASC14B_FONT_HEIGHT	14		//����߶���14
#define GUI_ASC14B_CHAR_WIDTH		15			//�����ַ���15��ʵ���ϴ��ֿⲻ֧�ֺ���
#define GUI_ASC14B_ASCII_WIDTH	8			//ascii�ַ���8
#define GUI_ASC14B_FONT_BUF_SIZE 	28 //һ������Ҫ���ٸ��ֽ����洢��=GUI_GBK_FONT_SIZE/8xGUI_GBK_FONT_SIZE
#define GUI_ASC14B_FONT_SPI_FLASH_BASE		(FM_FONT_A14B_BASE_SECTOR*FLASH_SECTOR_BYTES)// DOWNLOAD_START_PAGE*DOWNLOAD_PAGE_SIZE�洢�ֿ����ʼҳ

//GBK21x21�ֿ�
#define GUI_GBK21_FONT_HEIGHT	21		//����߶���12
#define GUI_GBK21_CHAR_WIDTH	21			//�����ַ���12
#define GUI_GBK21_ASCII_WIDTH	11			//ascii�ַ���6
#define GUI_GBK21_FONT_BUF_SIZE 	63 //һ��������Ҫ���ٸ��ֽ����洢��=GUI_GBK_FONT_SIZE/8xGUI_GBK_FONT_SIZE
#define GUI_GBK21_FONT_SPI_FLASH_BASE		(FM_FONT_G21_BASE_SECTOR*FLASH_SECTOR_BYTES)// DOWNLOAD_START_PAGE*DOWNLOAD_PAGE_SIZE�洢�ֿ����ʼҳ
#define GUI_GBK21_FONT_END	0x170380//16�ֽڶ���=(23940*GUI_GBK21_FONT_BUF_SIZE) //�ֿ������ ��Ascii�����

#if 0
//GB2312�ֿ�
#define GUI_FONT_GB2312_OFFSET_H			0xA1
#define GUI_FONT_GB2312_OFFSET_L			0xA1
#define GUI_FONT_GB2312_PAGENUM    		0x5E //=0xFE-0xA1+1
#define GUI_FONT_GB2312_SPACE					0		//���
#define GUI_FONT_GB2312_MARGIN				16		//�о�
#define GUI_FONT_GB2312_END						8836*128
#endif


//3	GUI ��������

typedef u16 COLOR_TYPE;

typedef struct { 
	u16 x; 	//����������Ͻ�x
	u16 y;	//����������Ͻ�y
	u16 w; 	//�����
	u16 h;	//�����
	u16 Space;//����ָ����࣬��д��ʱ��4-7λ��ʾ�ּ��,0-3λ��ʾ�м��
	//bool IsScreen2;//��ʾ��������
}GUI_REGION,*pGUI_REGION;

typedef enum{
	Gui_True=0,
	Gui_False,
	Gui_No_Such_File,//û�ж����ļ�
	Gui_Out_Of_Range//ͼƬ��ʾ������Χ
}GUI_RESULT;

//2		GUI API   
//��ʼ��GUI
void Gui_Init(void);

void Gui_Fill(const GUI_REGION *pRegion,bool Dot);

void Gui_Draw(const GUI_REGION *pRegion,const u8 *pData);

#if 0
//��ָ��������д��,��������ʾ
//��pRegion�����в�����Ҫ����,�ر���space����
//Str��ʾҪд���ַ���
//Color��ʾ������ɫ
//����Ϊ16x16
u16 Gui_DrawFont_GBK16(const u8 *Str,const GUI_REGION *pRegion);

//����ʵ��������Ҫ���ַ���byte��
//����ֵʵ�ʾ���Gui_DrawFont_GBK�ķ���ֵ��ֻ��������ʾʵ������
u16 Gui_DrawFont_GBK16_Num(const u8 *Str,const GUI_REGION *pRegion);

//��ָ��������д��,��������ʾ
//��pRegion�����в�����Ҫ����,�ر���space����
//Str��ʾҪд���ַ���
//Color��ʾ������ɫ
//����Ϊ12x12
u16 Gui_DrawFont_GBK12(const u8 *Str,const GUI_REGION *pRegion);

//����ʵ��������Ҫ���ַ���byte��
//����ֵʵ�ʾ���Gui_DrawFont_GBK�ķ���ֵ��ֻ��������ʾʵ������
u16 Gui_DrawFont_GBK12_Num(const u8 *Str,const GUI_REGION *pRegion);

//��ָ��������д��,��������ʾ
//��pRegion�����в�����Ҫ����,�ر���space����
//Str��ʾҪд���ַ���
//Color��ʾ������ɫ
//����Ϊ9x14,��֧������
u16 Gui_DrawFont_ASC14B(const u8 *Str,const GUI_REGION *pRegion);

//����ʵ��������Ҫ���ַ���byte��
//����ֵʵ�ʾ���Gui_DrawFont_GBK�ķ���ֵ��ֻ��������ʾʵ������
u16 Gui_DrawFont_ASC14B_Num(const u8 *Str,const GUI_REGION *pRegion);
#endif

//ͨ����ʾ�ַ��ĺ���
u16 Gui_DrawFont(FONT_ACT FontLib,const u8 *Str,const GUI_REGION *pRegion);




#endif

