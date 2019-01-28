#ifndef VAR_DISPLAY_H
#define VAR_DISPLAY_H

void VarDisplay_Cb(int n,void *p);//变量改变后的回调函数
void OledDisplayLoop(void);
void VarDisplay_Init(void);//初始化屏显变量的订阅关系


#endif


