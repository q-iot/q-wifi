#include "SysDefines.h"

const VARIABLE_RECORD gInfo_VarRcdDef[]={//ϵͳ����
{
	0,//ID;
	{{'Q','S','Y','S'}},//NUM_UNI ProdTag;//�Բ�ƷΪ���ֵ�Ψһ��ǩ���ɿ�����ˣ�QSRV��QDEV��Q��ͷ��Ϊ������ǩ
	{{'S','L','E','P'}},//CHAR_NUM_UNI VarTag;//�Ա���Ϊ���ֵı�ǩ���ɳ��������ƶ������ظ�
	VAR_T_SYS,//VAR_TYPE VarType:3;//��������
	FALSE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_U16,//VAR_DISP_FAT DispFat:3;//��ʾ��ʽ
	0,//u8 DotNum:5;//��ʾС������λ��
	0,//TVAR InitVal;//��ʼֵ
	0,//u16 MainDevID;//�����������豸id
	0,//u8 VarIdx;//�������豸�����������1��ʼ
	0//u8 b;
},

{
	0,//ID;
	{{'Q','S','Y','S'}},//NUM_UNI ProdTag;//�Բ�ƷΪ���ֵ�Ψһ��ǩ���ɿ�����ˣ�QSRV��QDEV��Q��ͷ��Ϊ������ǩ
	{{'A','D','C','V'}},//CHAR_NUM_UNI VarTag;//�Ա���Ϊ���ֵı�ǩ���ɳ��������ƶ������ظ�
	VAR_T_SYS,//VAR_TYPE VarType:3;//��������
	TRUE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_U16,//VAR_DISP_FAT DispFat:3;//��ʾ��ʽ
	0,//u8 DotNum:5;//��ʾС������λ��
	0,//TVAR InitVal;//��ʼֵ
	0,//u16 MainDevID;//�����������豸id
	0,//u8 VarIdx;//�������豸�����������1��ʼ
	0//u8 b;
},

{
	0,//ID;
	{{'N','O','W','S'}},//NUM_UNI ProdTag;//�Բ�ƷΪ���ֵ�Ψһ��ǩ���ɿ�����ˣ�QSRV��QDEV��Q��ͷ��Ϊ������ǩ
	{{'T','I','M','E'}},//CHAR_NUM_UNI VarTag;//�Ա���Ϊ���ֵı�ǩ���ɳ��������ƶ������ظ�
	VAR_T_SYS,//VAR_TYPE VarType:3;//��������
	TRUE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_U16,//VAR_DISP_FAT DispFat:3;//��ʾ��ʽ
	2,//u8 DotNum:5;//��ʾС������λ��
	0,//TVAR InitVal;//��ʼֵ
	0,//u16 MainDevID;//�����������豸id
	0,//u8 VarIdx;//�������豸�����������1��ʼ
	0//u8 b;
},

{
	0,//ID;
	{{'T','I','M','E'}},//NUM_UNI ProdTag;//�Բ�ƷΪ���ֵ�Ψһ��ǩ���ɿ�����ˣ�QSRV��QDEV��Q��ͷ��Ϊ������ǩ
	{{'S','E','C','D'}},//CHAR_NUM_UNI VarTag;//�Ա���Ϊ���ֵı�ǩ���ɳ��������ƶ������ظ�
	VAR_T_SYS,//VAR_TYPE VarType:3;//��������
	FALSE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_U16,//VAR_DISP_FAT DispFat:3;//��ʾ��ʽ
	0,//u8 DotNum:5;//��ʾС������λ��
	0,//TVAR InitVal;//��ʼֵ
	0,//u16 MainDevID;//�����������豸id
	0,//u8 VarIdx;//�������豸�����������1��ʼ
	0//u8 b;
},

{
	0,//ID;
	{{'T','I','M','E'}},//NUM_UNI ProdTag;//�Բ�ƷΪ���ֵ�Ψһ��ǩ���ɿ�����ˣ�QSRV��QDEV��Q��ͷ��Ϊ������ǩ
	{{'M','I','N','T'}},//CHAR_NUM_UNI VarTag;//�Ա���Ϊ���ֵı�ǩ���ɳ��������ƶ������ظ�
	VAR_T_SYS,//VAR_TYPE VarType:3;//��������
	FALSE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_U16,//VAR_DISP_FAT DispFat:3;//��ʾ��ʽ
	0,//u8 DotNum:5;//��ʾС������λ��
	0,//TVAR InitVal;//��ʼֵ
	0,//u16 MainDevID;//�����������豸id
	0,//u8 VarIdx;//�������豸�����������1��ʼ
	0//u8 b;
},

{
	0,//ID;
	{{'D','A','T','E'}},//NUM_UNI ProdTag;//�Բ�ƷΪ���ֵ�Ψһ��ǩ���ɿ�����ˣ�QSRV��QDEV��Q��ͷ��Ϊ������ǩ
	{{'W','E','E','K'}},//CHAR_NUM_UNI VarTag;//�Ա���Ϊ���ֵı�ǩ���ɳ��������ƶ������ظ�
	VAR_T_SYS,//VAR_TYPE VarType:3;//��������
	TRUE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_U16,//VAR_DISP_FAT DispFat:3;//��ʾ��ʽ
	0,//u8 DotNum:5;//��ʾС������λ��
	0,//TVAR InitVal;//��ʼֵ
	0,//u16 MainDevID;//�����������豸id
	0,//u8 VarIdx;//�������豸�����������1��ʼ
	0//u8 b;
},

{
	0,//ID;
	{{'S','M','S','G'}},//NUM_UNI ProdTag;//�Բ�ƷΪ���ֵ�Ψһ��ǩ���ɿ�����ˣ�QSRV��QDEV��Q��ͷ��Ϊ������ǩ
	{{'V','A','R','1'}},//CHAR_NUM_UNI VarTag;//�Ա���Ϊ���ֵı�ǩ���ɳ��������ƶ������ظ�
	VAR_T_SYS,//VAR_TYPE VarType:3;//��������
	FALSE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_U16,//VAR_DISP_FAT DispFat:3;//��ʾ��ʽ
	0,//u8 DotNum:5;//��ʾС������λ��
	0,//TVAR InitVal;//��ʼֵ
	0,//u16 MainDevID;//�����������豸id
	0,//u8 VarIdx;//�������豸�����������1��ʼ
	0//u8 b;
},

{
	0,//ID;
	{{'S','M','S','G'}},//NUM_UNI ProdTag;//�Բ�ƷΪ���ֵ�Ψһ��ǩ���ɿ�����ˣ�QSRV��QDEV��Q��ͷ��Ϊ������ǩ
	{{'V','A','R','2'}},//CHAR_NUM_UNI VarTag;//�Ա���Ϊ���ֵı�ǩ���ɳ��������ƶ������ظ�
	VAR_T_SYS,//VAR_TYPE VarType:3;//��������
	FALSE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_U16,//VAR_DISP_FAT DispFat:3;//��ʾ��ʽ
	0,//u8 DotNum:5;//��ʾС������λ��
	0,//TVAR InitVal;//��ʼֵ
	0,//u16 MainDevID;//�����������豸id
	0,//u8 VarIdx;//�������豸�����������1��ʼ
	0//u8 b;
},

{
	0,//ID;
	{{'U','S','E','R'}},//NUM_UNI ProdTag;//�Բ�ƷΪ���ֵ�Ψһ��ǩ���ɿ�����ˣ�QSRV��QDEV��Q��ͷ��Ϊ������ǩ
	{{'V','A','R','1'}},//CHAR_NUM_UNI VarTag;//�Ա���Ϊ���ֵı�ǩ���ɳ��������ƶ������ظ�
	VAR_T_USER,//VAR_TYPE VarType:3;//��������
	FALSE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_S16,//VAR_DISP_FAT DispFat:3;//��ʾ��ʽ
	0,//u8 DotNum:5;//��ʾС������λ��
	0,//TVAR InitVal;//��ʼֵ
	0,//u16 MainDevID;//�����������豸id
	0,//u8 VarIdx;//�������豸�����������1��ʼ
	0//u8 b;
},

{
	0,//ID;
	{{'U','S','E','R'}},//NUM_UNI ProdTag;//�Բ�ƷΪ���ֵ�Ψһ��ǩ���ɿ�����ˣ�QSRV��QDEV��Q��ͷ��Ϊ������ǩ
	{{'V','A','R','2'}},//CHAR_NUM_UNI VarTag;//�Ա���Ϊ���ֵı�ǩ���ɳ��������ƶ������ظ�
	VAR_T_USER,//VAR_TYPE VarType:3;//��������
	FALSE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_U16,//VAR_DISP_FAT DispFat:3;//��ʾ��ʽ
	0,//u8 DotNum:5;//��ʾС������λ��
	0,//TVAR InitVal;//��ʼֵ
	0,//u16 MainDevID;//�����������豸id
	0,//u8 VarIdx;//�������豸�����������1��ʼ
	0//u8 b;
},

{
	0,//ID;
	{{'U','S','E','R'}},//NUM_UNI ProdTag;//�Բ�ƷΪ���ֵ�Ψһ��ǩ���ɿ�����ˣ�QSRV��QDEV��Q��ͷ��Ϊ������ǩ
	{{'V','A','R','3'}},//CHAR_NUM_UNI VarTag;//�Ա���Ϊ���ֵı�ǩ���ɳ��������ƶ������ظ�
	VAR_T_USER,//VAR_TYPE VarType:3;//��������
	FALSE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_HEX,//VAR_DISP_FAT DispFat:3;//��ʾ��ʽ
	0,//u8 DotNum:5;//��ʾС������λ��
	0,//TVAR InitVal;//��ʼֵ
	0,//u16 MainDevID;//�����������豸id
	0,//u8 VarIdx;//�������豸�����������1��ʼ
	0//u8 b;
},

{
	0,//ID;
	{{'U','S','E','R'}},//NUM_UNI ProdTag;//�Բ�ƷΪ���ֵ�Ψһ��ǩ���ɿ�����ˣ�QSRV��QDEV��Q��ͷ��Ϊ������ǩ
	{{'V','A','R','4'}},//CHAR_NUM_UNI VarTag;//�Ա���Ϊ���ֵı�ǩ���ɳ��������ƶ������ظ�
	VAR_T_USER,//VAR_TYPE VarType:3;//��������
	FALSE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_BIN,//VAR_DISP_FAT DispFat:3;//��ʾ��ʽ
	8,//u8 DotNum:5;//��ʾС������λ��
	0,//TVAR InitVal;//��ʼֵ
	0,//u16 MainDevID;//�����������豸id
	0,//u8 VarIdx;//�������豸�����������1��ʼ
	0//u8 b;
},












{0,{{0,0,0,0}}}//�Դ˽���
};





