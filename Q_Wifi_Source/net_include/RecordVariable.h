#ifndef VARIABLE_H
#define VARIABLE_H

//--------------------------------�����洢--------------------------------------
enum{
	VAR_T_NULL=0,
	VAR_T_SRV,//ͨ�������������õı�����tag��������ͬ������Ψһ
	VAR_T_SYS,//ͨ��ϵͳ�����õı�����tag��������ͬ������Ψһ��appֻ��
	VAR_T_DEV,//ͨ���豸�����õı�����tag�п�����ͬ��app�ɶ���д
	VAR_T_USER,//�û��豸�Լ����õı�����tag��������ͬ������Ψһ��app�ɶ���д

	VAR_T_MAX
};
typedef u8 VAR_TYPE;

enum{
	VDF_U16=0,
	VDF_S16,
	VDF_HEX,
	VDF_BIN,
	VDF_U32,
	VDF_S32,
	VDF_FLOAT,

	VDF_MAX
};
typedef u8 VAR_DISP_FAT;

#define VAR_TAG_LEN 4

typedef union{
	u8 Char[VAR_TAG_LEN];
	u32 Num;
}CHAR_NUM_UNI;

typedef s16 TVAR;//����ֵ������
typedef s32 TVAR32;

typedef struct{
	u32 ID;
	CHAR_NUM_UNI ProdTag;//�Բ�ƷΪ���ֵ�Ψһ��ǩ���ɿ�����ˣ�QSRV��QDEV��Q��ͷ��Ϊ������ǩ�������ִ�Сд
	CHAR_NUM_UNI VarTag;//�Ա���Ϊ���ֵı�ǩ���ɳ��������ƶ������ظ��������ִ�Сд
	VAR_TYPE VarType:3;//��������
	bool ReadOnly:1;//�Ƿ�����������app���޸ı���
	u8 a:4;
	VAR_DISP_FAT DispFat:3;//��ʾ��ʽ
	u8 DotNum:5;//��ʾС������λ��
	TVAR InitVal;//��ʼֵ
	u16 MainDevID;//�����������豸id
	u8 VarIdx;//�������豸�����������1��ʼ
	u8 b;
}VARIABLE_RECORD;

#define DispVarTag(x) x.ProdTag.Char[0],x.ProdTag.Char[1],x.ProdTag.Char[2],x.ProdTag.Char[3],x.VarTag.Char[0],x.VarTag.Char[1],x.VarTag.Char[2],x.VarTag.Char[3]
#define DispVarTagp(x) x->ProdTag.Char[0],x->ProdTag.Char[1],x->ProdTag.Char[2],x->ProdTag.Char[3],x->VarTag.Char[0],x->VarTag.Char[1],x->VarTag.Char[2],x->VarTag.Char[3]

//------------------------------��������----------------------------------------
enum{
	VOT_NULL=0,
	VOT_SET,
	VOT_GET,

	VOT_MAX
};
typedef u8 VAR_OPT;//��������

enum{
	VST_NULL=0,
	VST_VALID,	//������
	VST_NOT_FOUND,
	
	VST_MAX
};
typedef u8 VAR_STATE;//����״̬

enum{
	VIT_SELF=0,
	VIT_RELATE,

	VIT_MAX
};
typedef u8 VAR_IDX_T;

typedef struct{
	VAR_OPT VarOpt:2;//����ѡ��
	VAR_STATE VarState:2;//����״̬
	u8 a:4;
	VAR_IDX_T VarIdxType:2;//��������
	u8 VarIdx:6;//������ţ�1��ʼ
	
	TVAR VarValue;//����ֵ
}VAR_OBJ;




//-------------------------------��������----------------------------------------
enum{
	VCM_IN=0,//��CalcA��CalcB�ķ�Χ��
	VCM_OUT,//����CalcA��CalcB�ķ�Χ��
	VCM_CHANGED,//�ڷ�Χ�ڣ��仯�ʹ���
};
typedef u8 VAR_RANGE_JUDGE; //��Χ�ж�����

enum{
	CMT_NULL=0,
	CMT_SET,//ֱ�ӵ���
	CMT_INC,//���
	CMT_DEC,//���
	CMT_AND,//��λ &
	CMT_OR,//��λ |
	CMT_REV,//��λȡ��
	CMT_MAX,
	
	CMT_VAR_CALC_BIT=0x10,
	CMT_SET_VAR=0x11,//��������ֵ
	CMT_INC_VAR,
	CMT_DEC_VAR,
	CMT_AND_VAR,
	CMT_OR_VAR,
	CMT_REV_VAR,
};
typedef u8 VAR_CALC_METHOD;//���㷽��



#endif

