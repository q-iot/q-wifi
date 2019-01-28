#include "SysDefines.h"

const VARIABLE_RECORD gInfo_VarRcdDef[]={//系统变量
{
	0,//ID;
	{{'Q','S','Y','S'}},//NUM_UNI ProdTag;//以产品为区分的唯一标签，由酷享审核，QSRV、QDEV等Q打头的为保留标签
	{{'S','L','E','P'}},//CHAR_NUM_UNI VarTag;//以变量为区分的标签，由厂家自行制定，可重复
	VAR_T_SYS,//VAR_TYPE VarType:3;//变量类型
	FALSE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_U16,//VAR_DISP_FAT DispFat:3;//显示格式
	0,//u8 DotNum:5;//显示小数点后的位数
	0,//TVAR InitVal;//初始值
	0,//u16 MainDevID;//变量归属的设备id
	0,//u8 VarIdx;//变量在设备里面的索引，1起始
	0//u8 b;
},

{
	0,//ID;
	{{'Q','S','Y','S'}},//NUM_UNI ProdTag;//以产品为区分的唯一标签，由酷享审核，QSRV、QDEV等Q打头的为保留标签
	{{'A','D','C','V'}},//CHAR_NUM_UNI VarTag;//以变量为区分的标签，由厂家自行制定，可重复
	VAR_T_SYS,//VAR_TYPE VarType:3;//变量类型
	TRUE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_U16,//VAR_DISP_FAT DispFat:3;//显示格式
	0,//u8 DotNum:5;//显示小数点后的位数
	0,//TVAR InitVal;//初始值
	0,//u16 MainDevID;//变量归属的设备id
	0,//u8 VarIdx;//变量在设备里面的索引，1起始
	0//u8 b;
},

{
	0,//ID;
	{{'N','O','W','S'}},//NUM_UNI ProdTag;//以产品为区分的唯一标签，由酷享审核，QSRV、QDEV等Q打头的为保留标签
	{{'T','I','M','E'}},//CHAR_NUM_UNI VarTag;//以变量为区分的标签，由厂家自行制定，可重复
	VAR_T_SYS,//VAR_TYPE VarType:3;//变量类型
	TRUE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_U16,//VAR_DISP_FAT DispFat:3;//显示格式
	2,//u8 DotNum:5;//显示小数点后的位数
	0,//TVAR InitVal;//初始值
	0,//u16 MainDevID;//变量归属的设备id
	0,//u8 VarIdx;//变量在设备里面的索引，1起始
	0//u8 b;
},

{
	0,//ID;
	{{'T','I','M','E'}},//NUM_UNI ProdTag;//以产品为区分的唯一标签，由酷享审核，QSRV、QDEV等Q打头的为保留标签
	{{'S','E','C','D'}},//CHAR_NUM_UNI VarTag;//以变量为区分的标签，由厂家自行制定，可重复
	VAR_T_SYS,//VAR_TYPE VarType:3;//变量类型
	FALSE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_U16,//VAR_DISP_FAT DispFat:3;//显示格式
	0,//u8 DotNum:5;//显示小数点后的位数
	0,//TVAR InitVal;//初始值
	0,//u16 MainDevID;//变量归属的设备id
	0,//u8 VarIdx;//变量在设备里面的索引，1起始
	0//u8 b;
},

{
	0,//ID;
	{{'T','I','M','E'}},//NUM_UNI ProdTag;//以产品为区分的唯一标签，由酷享审核，QSRV、QDEV等Q打头的为保留标签
	{{'M','I','N','T'}},//CHAR_NUM_UNI VarTag;//以变量为区分的标签，由厂家自行制定，可重复
	VAR_T_SYS,//VAR_TYPE VarType:3;//变量类型
	FALSE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_U16,//VAR_DISP_FAT DispFat:3;//显示格式
	0,//u8 DotNum:5;//显示小数点后的位数
	0,//TVAR InitVal;//初始值
	0,//u16 MainDevID;//变量归属的设备id
	0,//u8 VarIdx;//变量在设备里面的索引，1起始
	0//u8 b;
},

{
	0,//ID;
	{{'D','A','T','E'}},//NUM_UNI ProdTag;//以产品为区分的唯一标签，由酷享审核，QSRV、QDEV等Q打头的为保留标签
	{{'W','E','E','K'}},//CHAR_NUM_UNI VarTag;//以变量为区分的标签，由厂家自行制定，可重复
	VAR_T_SYS,//VAR_TYPE VarType:3;//变量类型
	TRUE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_U16,//VAR_DISP_FAT DispFat:3;//显示格式
	0,//u8 DotNum:5;//显示小数点后的位数
	0,//TVAR InitVal;//初始值
	0,//u16 MainDevID;//变量归属的设备id
	0,//u8 VarIdx;//变量在设备里面的索引，1起始
	0//u8 b;
},

{
	0,//ID;
	{{'S','M','S','G'}},//NUM_UNI ProdTag;//以产品为区分的唯一标签，由酷享审核，QSRV、QDEV等Q打头的为保留标签
	{{'V','A','R','1'}},//CHAR_NUM_UNI VarTag;//以变量为区分的标签，由厂家自行制定，可重复
	VAR_T_SYS,//VAR_TYPE VarType:3;//变量类型
	FALSE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_U16,//VAR_DISP_FAT DispFat:3;//显示格式
	0,//u8 DotNum:5;//显示小数点后的位数
	0,//TVAR InitVal;//初始值
	0,//u16 MainDevID;//变量归属的设备id
	0,//u8 VarIdx;//变量在设备里面的索引，1起始
	0//u8 b;
},

{
	0,//ID;
	{{'S','M','S','G'}},//NUM_UNI ProdTag;//以产品为区分的唯一标签，由酷享审核，QSRV、QDEV等Q打头的为保留标签
	{{'V','A','R','2'}},//CHAR_NUM_UNI VarTag;//以变量为区分的标签，由厂家自行制定，可重复
	VAR_T_SYS,//VAR_TYPE VarType:3;//变量类型
	FALSE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_U16,//VAR_DISP_FAT DispFat:3;//显示格式
	0,//u8 DotNum:5;//显示小数点后的位数
	0,//TVAR InitVal;//初始值
	0,//u16 MainDevID;//变量归属的设备id
	0,//u8 VarIdx;//变量在设备里面的索引，1起始
	0//u8 b;
},

{
	0,//ID;
	{{'U','S','E','R'}},//NUM_UNI ProdTag;//以产品为区分的唯一标签，由酷享审核，QSRV、QDEV等Q打头的为保留标签
	{{'V','A','R','1'}},//CHAR_NUM_UNI VarTag;//以变量为区分的标签，由厂家自行制定，可重复
	VAR_T_USER,//VAR_TYPE VarType:3;//变量类型
	FALSE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_S16,//VAR_DISP_FAT DispFat:3;//显示格式
	0,//u8 DotNum:5;//显示小数点后的位数
	0,//TVAR InitVal;//初始值
	0,//u16 MainDevID;//变量归属的设备id
	0,//u8 VarIdx;//变量在设备里面的索引，1起始
	0//u8 b;
},

{
	0,//ID;
	{{'U','S','E','R'}},//NUM_UNI ProdTag;//以产品为区分的唯一标签，由酷享审核，QSRV、QDEV等Q打头的为保留标签
	{{'V','A','R','2'}},//CHAR_NUM_UNI VarTag;//以变量为区分的标签，由厂家自行制定，可重复
	VAR_T_USER,//VAR_TYPE VarType:3;//变量类型
	FALSE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_U16,//VAR_DISP_FAT DispFat:3;//显示格式
	0,//u8 DotNum:5;//显示小数点后的位数
	0,//TVAR InitVal;//初始值
	0,//u16 MainDevID;//变量归属的设备id
	0,//u8 VarIdx;//变量在设备里面的索引，1起始
	0//u8 b;
},

{
	0,//ID;
	{{'U','S','E','R'}},//NUM_UNI ProdTag;//以产品为区分的唯一标签，由酷享审核，QSRV、QDEV等Q打头的为保留标签
	{{'V','A','R','3'}},//CHAR_NUM_UNI VarTag;//以变量为区分的标签，由厂家自行制定，可重复
	VAR_T_USER,//VAR_TYPE VarType:3;//变量类型
	FALSE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_HEX,//VAR_DISP_FAT DispFat:3;//显示格式
	0,//u8 DotNum:5;//显示小数点后的位数
	0,//TVAR InitVal;//初始值
	0,//u16 MainDevID;//变量归属的设备id
	0,//u8 VarIdx;//变量在设备里面的索引，1起始
	0//u8 b;
},

{
	0,//ID;
	{{'U','S','E','R'}},//NUM_UNI ProdTag;//以产品为区分的唯一标签，由酷享审核，QSRV、QDEV等Q打头的为保留标签
	{{'V','A','R','4'}},//CHAR_NUM_UNI VarTag;//以变量为区分的标签，由厂家自行制定，可重复
	VAR_T_USER,//VAR_TYPE VarType:3;//变量类型
	FALSE,//bool ReadOnly:1;
	0,//u8 a:4;
	VDF_BIN,//VAR_DISP_FAT DispFat:3;//显示格式
	8,//u8 DotNum:5;//显示小数点后的位数
	0,//TVAR InitVal;//初始值
	0,//u16 MainDevID;//变量归属的设备id
	0,//u8 VarIdx;//变量在设备里面的索引，1起始
	0//u8 b;
},












{0,{{0,0,0,0}}}//以此结束
};





