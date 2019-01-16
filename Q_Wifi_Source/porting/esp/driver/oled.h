#ifndef I2C_OLED_H
#define I2C_OLED_H

void Oled_Init(void);
void Oled_Brush(u16 X,u16 Y,u16 W,u16 H);
void Oled_DrawRegion(u16 X,u16 Y,u16 W,u16 H,const u8 *pData);
void Oled_Fill(u16 X,u16 Y,u16 W,u16 H,u8 Data);
void Oled_Refresh(void);

#endif

