#ifndef VAR_IO_HANDLER_H
#define VAR_IO_HANDLER_H

//搜寻所有输出io和变量的匹配关系
void UpdateIoStateByVar(u16 Vid,TVAR32 Val);

//根据用户app输入的按钮键值来改变io状态
void UpdateIoStateByBtn(u32 DevID,u16 BtnKey);

//按键状态改变时处理
void ChangeVarByKeyState(u8 Pin,u16 PressMs);

//有作为输入的io发生输入时
void ChangeVarByIoState(u8 Pin,u8 PinVal);

//根据变量值，更新所有io状态
void UpdateAllIoStateByVar(void);

//根据io初值和模式，设置变量初值
void InitVarByIoState(void);





#endif

