#ifndef VAR_IO_HANDLER_H
#define VAR_IO_HANDLER_H

//��Ѱ�������io�ͱ�����ƥ���ϵ
void UpdateIoStateByVar(u16 Vid,TVAR32 Val);

//�����û�app����İ�ť��ֵ���ı�io״̬
void UpdateIoStateByBtn(u32 DevID,u16 BtnKey);

//����״̬�ı�ʱ����
void ChangeVarByKeyState(u8 Pin,u16 PressMs);

//����Ϊ�����io��������ʱ
void ChangeVarByIoState(u8 Pin,u8 PinVal);

//���ݱ���ֵ����������io״̬
void UpdateAllIoStateByVar(void);

//����io��ֵ��ģʽ�����ñ�����ֵ
void InitVarByIoState(void);





#endif

