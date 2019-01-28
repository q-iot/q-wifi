#ifndef VARIABLE_H
#define VARIABLE_H

//--------------------------------变量存储--------------------------------------
enum{
	VAR_T_NULL=0,
	VAR_T_SRV,//通过服务器来设置的变量，tag不允许相同，必须唯一
	VAR_T_SYS,//通过系统来设置的变量，tag不允许相同，必须唯一。app只读
	VAR_T_DEV,//通过设备来设置的变量，tag有可能相同。app可读可写
	VAR_T_USER,//用户设备自己设置的变量，tag不允许相同，必须唯一。app可读可写

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

typedef s16 TVAR;//变量值的类型
typedef s32 TVAR32;

typedef struct{
	u32 ID;
	CHAR_NUM_UNI ProdTag;//以产品为区分的唯一标签，由酷享审核，QSRV、QDEV等Q打头的为保留标签，不区分大小写
	CHAR_NUM_UNI VarTag;//以变量为区分的标签，由厂家自行制定，可重复，不区分大小写
	VAR_TYPE VarType:3;//变量类型
	bool ReadOnly:1;//是否允许主机、app来修改变量
	u8 a:4;
	VAR_DISP_FAT DispFat:3;//显示格式
	u8 DotNum:5;//显示小数点后的位数
	TVAR InitVal;//初始值
	u16 MainDevID;//变量归属的设备id
	u8 VarIdx;//变量在设备里面的索引，1起始
	u8 b;
}VARIABLE_RECORD;

#define DispVarTag(x) x.ProdTag.Char[0],x.ProdTag.Char[1],x.ProdTag.Char[2],x.ProdTag.Char[3],x.VarTag.Char[0],x.VarTag.Char[1],x.VarTag.Char[2],x.VarTag.Char[3]
#define DispVarTagp(x) x->ProdTag.Char[0],x->ProdTag.Char[1],x->ProdTag.Char[2],x->ProdTag.Char[3],x->VarTag.Char[0],x->VarTag.Char[1],x->VarTag.Char[2],x->VarTag.Char[3]

//------------------------------变量传递----------------------------------------
enum{
	VOT_NULL=0,
	VOT_SET,
	VOT_GET,

	VOT_MAX
};
typedef u8 VAR_OPT;//变量操作

enum{
	VST_NULL=0,
	VST_VALID,	//可用了
	VST_NOT_FOUND,
	
	VST_MAX
};
typedef u8 VAR_STATE;//变量状态

enum{
	VIT_SELF=0,
	VIT_RELATE,

	VIT_MAX
};
typedef u8 VAR_IDX_T;

typedef struct{
	VAR_OPT VarOpt:2;//操作选项
	VAR_STATE VarState:2;//变量状态
	u8 a:4;
	VAR_IDX_T VarIdxType:2;//变量类型
	u8 VarIdx:6;//变量序号，1开始
	
	TVAR VarValue;//变量值
}VAR_OBJ;




//-------------------------------变量计算----------------------------------------
enum{
	VCM_IN=0,//在CalcA和CalcB的范围内
	VCM_OUT,//不在CalcA和CalcB的范围内
	VCM_CHANGED,//在范围内，变化就触发
};
typedef u8 VAR_RANGE_JUDGE; //范围判定方法

enum{
	CMT_NULL=0,
	CMT_SET,//直接等于
	CMT_INC,//相加
	CMT_DEC,//相减
	CMT_AND,//按位 &
	CMT_OR,//按位 |
	CMT_REV,//按位取反
	CMT_MAX,
	
	CMT_VAR_CALC_BIT=0x10,
	CMT_SET_VAR=0x11,//拷贝变量值
	CMT_INC_VAR,
	CMT_DEC_VAR,
	CMT_AND_VAR,
	CMT_OR_VAR,
	CMT_REV_VAR,
};
typedef u8 VAR_CALC_METHOD;//计算方法



#endif

