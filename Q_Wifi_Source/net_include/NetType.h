#ifndef NET_TYPE_H
#define NET_TYPE_H

#include "LimitMarco.h"
#include "PublicEvent.h"

typedef u32 ID_T;

//---------------------------------------------------------------------------
//注:所有记录体头字节必须保证存储全局对象类型
typedef enum{
	GOT_NULL=0,
	GOT_FLOOR=1,//
	GOT_ROOM,
	GOT_IN_DEVICE,
	GOT_DEVICE,
	GOT_JUMPER,
	GOT_IR_SIGNAL,
	GOT_JUMPER_IR,
	GOT_RF,
	GOT_S433M_SIGNAL,
	GOT_KEYS_SET=10,
	GOT_SCENE,
	GOT_TRIGGER,
	GOT_ALARM,
	GOT_MSG_IN,
	GOT_MSG_OUT,
	GOT_MEMBER,
	GOT_NAME,
	GOT_VARIABLE,
	GOT_WAVER,
	GOT_PC_CTRL=20,
	GOT_TOUCH,
	GOT_REPORT,
	GOT_WEIBO,
	GOT_CURTAIN,
	GOT_COM_OUT,
	

	GOT_MAX//不能超过255，否则变量传递有问题
}GLOBA_OBJ_TYPE;


//---------------------------------------------------------------------------


//--------------------------------主包------------------------------------
enum{
	PT_NULL=0,//不用
	PT_SYS_PARAM,//系统参数设定
	PT_BULK_OPERATE,//info批量操作
	PT_SIG_OPERATE,//信号设定,开启录入，关闭录入，发射录入，存储录入
	PT_INFO_OPERATE,//info单条处理
	PT_WDEV_CTRL,//jumper外设的沟通
	PT_KEY_CTRL,//按键控制
	PT_FLASH_OPERATE,//专门针对spi flash进行读写操作
	PT_LOGIN,//登录包
	PT_STATUS,//状态包
	PT_CMD,//命令行
	PT_VARIABLE,//变量包
	PT_UPDATE,//上报包
	PT_WAVER_CONFIG,//waver配置包
	PT_APP_BEAT,//app发来的心跳包
	PT_APP_ACT,//app动作包
	PT_APP_ATTENTION,//app关注包
	PT_LOCAL_MAX,
	
	PT_SRV_BEAT=0x20,//发给服务器的心跳包
	PT_SRV_LOGIN,//登录包，用以网关登陆服务器
	PT_SRV_QUERY,//问询包
	PT_SRV_MSG,//信息包，网关与服务器的双向消息传递
	PT_SRV_UNBIND,//解除主机家庭的绑定
	PT_SRV_CMD,//服务器命令包
	
	PT_APP_EXPEND=0x7f,//扩展包，app用来与服务器沟通
	
	PT_MAX,
};
typedef u8 GLOBA_PKT_TYPE;

enum{
	GPR_MAIN=0,
	
	GPR_SUCESS,
	GPR_FAILD,
	GPR_PARAM_ERROR,//传入的参数有误
	GPR_RES_NULL,//需要获取的资源为空
	GPR_RES_UNENOUGH,//资源不够
	GPR_NUM_OVERFLOW,//参数超限
	GPR_ROUTING_ERROR,//程序错误
	GPR_HARDWARD_ERROR,//硬件错误
	GPR_AUTH_ERROR,//权限错误
	GPR_TIMEOUT,//超时
	GPR_BUSY,//忙
	GPR_OFFLINE,//不在线
	
	GPR_MAX
};
typedef u8 GLOBA_PKT_RESULT;

typedef struct {
	u16 PktLen;//数据区长度，包含最后两个字节的校验码
	u16 _PktLen;//数据区长度保密码
	u16 PktSn;//包序列号
	GLOBA_PKT_TYPE Type;//此包的类型
	GLOBA_PKT_RESULT Res;//0表示是主包，非0表示是回包，并表示具体的回复码
	u32 DutID;//fly的id
	u32 AppID;//app连接的id
	
	u8 Data[2];//数据区	
	//整包最后两个字节对整个包进行校验
}GLOBA_PKT_HEADER;

#define GLOBA_PKT_HD_LEN (sizeof(GLOBA_PKT_HEADER)-4)

//---------------------------------------------------------------------------


//----------------------------------登陆包-----------------------------------------
#include "AppClientManger.h"

typedef struct{
	u16 PktLen;//数据区长度，包含最后两个字节的校验码
	u16 _PktLen;//数据区长度保密码
	u16 PktSn;//包序列号
	GLOBA_PKT_TYPE Type;//此包的类型
	GLOBA_PKT_RESULT Res;//0表示是主包，非0表示是回包，并表示具体的回复码
	u32 DutID;//fly的id
	u32 AppID;//app连接的id

	APP_CLIENT_TYPE ClientType;//登录类型，返回时带回客户端索引，客户端计算PktSn时必须叠加此索引到高4位。
	u8 VendorID;//厂商号码
	u16 FwVer;//固件版本，app发包时填入自己的版本号，dut回包时也填入自己的版本号
	u32 SnHash;//序列号做MakeHash后的值
	u32 PwHash;//密码做MakeHash后的值
	u16 SecretKey;	//返回的密钥
	u16 Role;//角色	
	u32 SysConfigFlag;//系统参数配置标识
	u32 InfoConfigFlag[IFT_SAVE_NUM];//信息配置标识
	u8 Data[2];//存放包检验
}LOGIN_PKT;

//------------------------------------系统参数包-----------------------------
//#include "DbForSys.h"

enum{
	SPT_GET=0,
	SPT_SET,
	SPT_BURN,
	SPT_DEF,//仅参数恢复默认
	SPT_DEFEND_GET,//获取激活的情景列表
	SPT_DEFEND_SET,//设置需要激活的情景
	SPT_DEFEND_CLR,//设置需要屏蔽的情景


	
	SPT_MAX
};
typedef u8 SYS_PARAM_TYPE;

typedef struct{
	u16 PktLen;//数据区长度，包含最后两个字节的校验码
	u16 _PktLen;//数据区长度保密码
	u16 PktSn;//包序列号
	GLOBA_PKT_TYPE Type;//此包的类型
	GLOBA_PKT_RESULT Res;//0表示是主包，非0表示是回包，并表示具体的回复码
	u32 DutID;//fly的id
	u32 AppID;//app连接的id

	u32 SysConfigFlag;//配置标识
	SYS_PARAM_TYPE Act;
	u8 ItemNum;//data中tlv的个数，或情景id个数
	u16 a;
	u8 Data[2];//实体，必须4字节对齐
}SYS_PARAM_PKT;
//---------------------------------------------------------------------------


//----------------------------------批量操作命令-----------------------------
#include "InfoSave.h"

enum{
	BOPT_ARG=0,//获取信息单元信息
	BOPT_GET,//获取信息
	
	BOPT_MAX
};
typedef u8 BULK_OPERATE_PKT_TYPE;

typedef struct{
	u16 Size;//单元大小
	u16 VaildNum;//有效单元数
	u16 FreeNum;//空闲空间
	u16 OnceNum;//一次可传输大小
}BULK_OPERATE_PKT_INFO;

typedef struct{
	u16 PktLen;//数据区长度，包含最后两个字节的校验码
	u16 _PktLen;//数据区长度保密码
	u16 PktSn;//包序列号
	GLOBA_PKT_TYPE Type;//此包的类型
	GLOBA_PKT_RESULT Res;//0表示是主包，非0表示是回包，并表示具体的回复码
	u32 DutID;//fly的id
	u32 AppID;//app连接的id
	
	BULK_OPERATE_PKT_TYPE Act;//动作
	INFO_TYPE Name;//指定操作对象
	u16 Idx;
	u16 Num;
	u16 a;
	u8 Data[2];//实体，必须4字节对齐
}BULK_OPERATE_PKT;
//---------------------------------------------------------------------------

//------------------------------------信息操作配置包-----------------------------
#include "RecordVariable.h"
enum{
	IOPT_READ=0,//获取信息
	IOPT_READ_BY_IDX,
	IOPT_DELETE,
	IOPT_NEW,
	
	IOPT_MAX
};
typedef u8 INFO_OPERATE_PKT_TYPE;

typedef struct{
	u16 PktLen;//数据区长度，包含最后两个字节的校验码
	u16 _PktLen;//数据区长度保密码
	u16 PktSn;//包序列号
	GLOBA_PKT_TYPE Type;//此包的类型
	GLOBA_PKT_RESULT Res;//0表示是主包，非0表示是回包，并表示具体的回复码
	u32 DutID;//fly的id
	u32 AppID;//app连接的id
	
	u32 InfoConfigFlag;//配置标识
	INFO_OPERATE_PKT_TYPE Act;//动作
	INFO_TYPE Name;//指定操作对象
	u16 Num;
	u8 Data[2];//实体，必须4字节对齐
}INFO_OPERATE_PKT;

typedef struct{
	GLOBA_PKT_RESULT Res;//单条信息的新增结果，GPR_SUCESS表示成功
	u8 a;
	u16 b;
	ID_T InfoID;//返回主机分配的对象id
	CHAR_NUM_UNI ProdTag;//如果是变量，由于tag可能会被主机改变，所以要返回实际的tag
	u32 c;
}INFO_PKT_RESP;

//---------------------------------------------------------------------------

#if 0//esp 无此功能
//------------------------------------信号操作配置包-----------------------------
#include "Ir_Rf_Record.h"

typedef enum{
	SOT_NULL=0,
	SOT_IR,
	SOT_RF,
	SOT_IR_TRANS,
	SOT_RF_TRANS,//rf转发

	SOT_MAX
}SIGNAL_OPT_TYPE;

typedef enum{
	SOPT_STUDY=0,//学习信号
	SOPT_SEND,
	SOPT_CAPTURE,//捕获信号做触发源
	
	SOPT_MAX
}SIG_OPERATE_PKT_TYPE;

typedef struct{
	u16 PktLen;//数据区长度，包含最后两个字节的校验码
	u16 _PktLen;//数据区长度保密码
	u16 PktSn;//包序列号
	GLOBA_PKT_TYPE Type;//此包的类型
	GLOBA_PKT_RESULT Res;//0表示是主包，非0表示是回包，并表示具体的回复码
	u32 DutID;//fly的id
	u32 AppID;//app连接的id
	
	SIG_OPERATE_PKT_TYPE Act;//动作
	SIGNAL_OPT_TYPE SignalType;//类型
	u16 Num;//用来存放键值
	u32 WDevAddr;//转发设备
	u8 Data[2];
}SIG_OPERATE_PKT;
//---------------------------------------------------------------------------
#endif

//--------------------------------万纳设备操作包-----------------------------------
#include "RecordDevice.h"

enum{
	WCPT_NULL=0,
	WCPT_DEV_SEARCH,
	WCPT_DEV_ADD,//通用型增加设备
	WCPT_DEV_DELETE,//通用型删除设备
	WCPT_TEST,//设备测试命令

	WCPT_MAX
};
typedef u8 WDEV_CTRL_PKT_TYPE;

typedef struct{
	u16 PktLen;//数据区长度，包含最后两个字节的校验码
	u16 _PktLen;//数据区长度保密码
	u16 PktSn;//包序列号
	GLOBA_PKT_TYPE Type;//此包的类型
	GLOBA_PKT_RESULT Res;//0表示是主包，非0表示是回包，并表示具体的回复码
	u32 DutID;//fly的id
	u32 AppID;//app连接的id
	
	WDEV_CTRL_PKT_TYPE Act;//动作
	u8 Num;
	WAVER_DUT_TYPE DutType;
	u32 WDevAddr;
	u32 Footer;//转发地址
	u8 Data[2];//UPKT_WDEV_LIST
}WDEV_CTRL_PKT;

//---------------------------------键值控制包------------------------------------------
typedef struct{
	u16 PktLen;//数据区长度，包含最后两个字节的校验码
	u16 _PktLen;//数据区长度保密码
	u16 PktSn;//包序列号
	GLOBA_PKT_TYPE Type;//此包的类型
	GLOBA_PKT_RESULT Res;//0表示是主包，非0表示是回包，并表示具体的回复码
	u32 DutID;//fly的id
	u32 AppID;//app连接的id
	
	u8 Num;	 
	u8 a;
	u16 b;
	u8 Data[2];//实体，以SCENE_ITEM为单位进行动作的存放
}KEY_CTRL_PKT;

#if 0//esp 无此功能
//---------------------------------------------------------------------------
typedef enum{
	FOPT_READ=0,//批量获取
	FOPT_WRITE,//批量存储
	FOPT_ERASE,//擦扇区

	FOPT_MAX
}FLASH_OPERATE_PKT_TYPE;

typedef enum{
	FBN_OEM=0,
	FBN_UNICODE,
	FBN_IAP,

	FBN_MAX
}FLASH_BLOCK_NAME;

typedef struct{
	u16 PktLen;//数据区长度，包含最后两个字节的校验码
	u16 _PktLen;//数据区长度保密码
	u16 PktSn;//包序列号
	GLOBA_PKT_TYPE Type;//此包的类型
	GLOBA_PKT_RESULT Res;//0表示是主包，非0表示是回包，并表示具体的回复码
	u32 DutID;//fly的id
	u32 AppID;//app连接的id
	
	FLASH_OPERATE_PKT_TYPE Act;//动作
	FLASH_BLOCK_NAME Name;
	u16 Page;//从0开始
	u16 Num;
	u16 a;
	u8 Data[2];//实体
}FLASH_OPERATE_PKT;
#endif

//------------------------------------状态包-----------------------------
enum{
	SCT_NULL=0,
	SCT_EDIT_START,//进入编辑态
	SCT_EDIT_END,//退出编辑态
	SCT_UPGRADE,//开始升级
	
	SCT_MAX,
};
typedef u8 STATUS_CMD_TYPE;

typedef struct{
	u16 PktLen;//数据区长度，包含最后两个字节的校验码
	u16 _PktLen;//数据区长度保密码
	u16 PktSn;//包序列号
	GLOBA_PKT_TYPE Type;//此包的类型
	GLOBA_PKT_RESULT Res;//0表示是主包，非0表示是回包，并表示具体的回复码
	u32 DutID;//fly的id
	u32 AppID;//app连接的id
	
	STATUS_CMD_TYPE Act;
	u8 Num;
	u16 a;
	u8 Data[2];
}STATUS_CMD_PKT;


//------------------------------------变量包-----------------------------
#include "RecordVariable.h"
enum{
	VPA_NULL=0,
	VPA_QUERY,//查询
	VPA_BIND,//绑定
	VPA_RELASE,//解绑
	VPA_SET,//设置值

	VPA_MAX
};
typedef u8 VARI_GET_ACT;

enum{
	VGM_ID=0,//通过id获取
	VGM_TAG,//通过tag直接获取
};
typedef u8 VARI_GET_METHOD;

typedef struct{
	CHAR_NUM_UNI ProdTag;//以产品为区分的唯一标签，由酷享审核，QSRV、QDEV等Q打头的为保留标签，不区分大小写
	CHAR_NUM_UNI VarTag;//以变量为区分的标签，由厂家自行制定，可重复，不区分大小写
}VARI_GET_BY_TAG;

typedef struct{
	u32 DevID;//设备id
	VAR_IDX_T VarIdxType:2;//变量类型
	u8 Idx:6;//变量索引,从1开始
}VARI_GET_BY_DEV;

typedef struct{
	union{
		VARI_GET_BY_TAG Tag;//通过tag方法，到主机里找tag相同的变量，系统变量，用户变量用这种方法
		VARI_GET_BY_DEV Dev;//通过dev方法，到主机里找dev对应的自身变量或者关联变量.设备变量用这种方法
	}Info;

	u32 VarID;//变量id，返回值，返回0表示未成功
	VARI_GET_ACT Act:4;//操作
	VARI_GET_METHOD Method:4;//匹配变量的方法	
	VAR_STATE VarState;//返回的变量状态
	TVAR VarVal;//设置或返回的变量值
}VARI_PKT_ITEM;

enum{
	VPC_NULL=0,
	VPC_RELEASE_ALL,//解绑所有
};
typedef u8 VARI_PKT_CMD;

typedef struct{
	u16 PktLen;//数据区长度，包含最后两个字节的校验码
	u16 _PktLen;//数据区长度保密码
	u16 PktSn;//包序列号
	GLOBA_PKT_TYPE Type;//此包的类型
	GLOBA_PKT_RESULT Res;//0表示是主包，非0表示是回包，并表示具体的回复码
	u32 DutID;//fly的id
	u32 AppID;//app连接的id
	
	VARI_PKT_CMD Cmd;//特殊指令
	u8 Num;//VARI_PKT_ITEM个数
	u16 b;
	u8 Data[2];//按次序存放变量值列表，单位VARI_PKT_ITEM
}VARIABLE_PKT;

//------------------------------------app意图包-----------------------------
enum{
	AAT_NULL=0,
	AAT_INTO_DEV,//进入设备
	AAT_LEAVE_DEV,//离开设备

};
typedef u8 APP_ACT_TYPE;

typedef struct{
	u16 PktLen;//数据区长度，包含最后两个字节的校验码
	u16 _PktLen;//数据区长度保密码
	u16 PktSn;//包序列号
	GLOBA_PKT_TYPE Type;//此包的类型
	GLOBA_PKT_RESULT Res;//0表示是主包，非0表示是回包，并表示具体的回复码
	u32 DutID;//fly的id
	u32 AppID;//app连接的id
	
	APP_ACT_TYPE ActType;//意图类型
	u8 Num;//item个数
	u16 b;
	u8 Data[2];//存放item
}APP_ACT_PKT;

//------------------------------------app关注包-----------------------------
enum{
	AATA_NULL=0,
	AATA_ATTENT,//开始订阅
	AATA_CANCEL,//取消订阅
};
typedef u8 APP_ATTENTION_ACT;

enum{
	AATT_NULL=0,
	AATT_ALL,//取消app所有订阅
	AATT_DEV,//关心或取消设备订阅
	AATT_VAR,//关心或取消变量订阅

};
typedef u8 APP_ATTENTION_TYPE;

typedef struct{
	APP_ATTENTION_ACT Act;
	APP_ATTENTION_TYPE ObjType;
	u16 b;
	u32 ObjID;
	TVAR32 Param;//存储变量值
}APP_ATTENTION_LIST;

typedef struct{
	u16 PktLen;//数据区长度，包含最后两个字节的校验码
	u16 _PktLen;//数据区长度保密码
	u16 PktSn;//包序列号
	GLOBA_PKT_TYPE Type;//此包的类型
	GLOBA_PKT_RESULT Res;//0表示是主包，非0表示是回包，并表示具体的回复码
	u32 DutID;//fly的id
	u32 AppID;//app连接的id
	
	u8 Num;//item个数
	u8 a;
	u16 DataLen;
	u8 Data[2];//存放APP_ATTENTION_LIST
}APP_ATTENTION_PKT;

//------------------------------------上报包-----------------------------
enum{
	UPA_NULL=0,
	UPA_IR_STUDY,//ir学习到后，传递给app
	UPA_RF_STUDY,//rf学习到后，传递给app
	UPA_WDEV_SEARCH,//wdev回搜索包后，传递给app
	UPA_OVER_CLIENT,//踢掉客户端
	UPA_VAR,//变量上报
	UPA_STR,//字符串上报
	UPA_BEAT,//心跳包
	UPA_RF_CAPTURE,//rf捕获
	UPA_DEV_STATE,//设备状态
	
	UPA_SRV=100,
	
	UPA_MAX
};
typedef u8 UPDATE_PKT_ACT;

typedef struct{
	u32 DevAddr;
	u32 HostAddr;
}UPKT_WDEV_LIST;//上报的wnet dev列表

#if 0//esp 无此功能
typedef struct{
	u32 DevInfoFlag;//最新的device的info flag值
	u32 DevID;//device info的id
	u32 SigID;//对应的信号id
	u8 Key;//键值
	GLOBA_PKT_RESULT Res;	
}UPKT_SIG_STUDY;//上报的信号学习结果

typedef struct{
	u32 Code;
	u16 BasePeriod;
	u16 a;
}UPKT_SIG_CAPTURE;//捕获的射频
#endif

typedef struct{
	u32 VarID;//变量id，返回值，返回0表示未成功
	u8 a;
	VAR_STATE VarState;//返回的变量状态
	TVAR VarVal;//返回的变量值	
}UPKT_VAR;//上报的变量最新值

typedef struct{
	u32 DevID;//设备号，0为系统字符串
	u32 StrID;
	u8 StrBuf[2];
}UPKT_STR;

typedef struct{
	u32 DevID;//设备号
	bool IsOnline;	//在线为1，不在线为0
	u8 a;
	u16 b;
}UPKT_DEV_STATE;

typedef struct{
	u16 PktLen;//数据区长度，包含最后两个字节的校验码
	u16 _PktLen;//数据区长度保密码
	u16 PktSn;//包序列号
	GLOBA_PKT_TYPE Type;//此包的类型
	GLOBA_PKT_RESULT Res;//0表示是主包，非0表示是回包，并表示具体的回复码
	u32 DutID;//fly的id
	u32 AppID;//app连接的id
	
	UPDATE_PKT_ACT Act;
	u8 Num;
	u16 DataLen;
	u8 Data[2];	//UPKT_WDEV_LIST、UPKT_SIG_STUDY、UPKT_SIG_CAPTURE、UPKT_VAR、UPKT_STR
}UPDATE_PKT;

#if 0//esp 无此功能
//------------------------------------记录体操作包-----------------------------
typedef enum{
	ROPT_NULL=0,
	ROPT_NEW,
	ROPT_DELETE,
	
	ROPT_MAX
}RECORD_OPERATE_PKT_TYPE;

typedef struct{
	u16 PktLen;//数据区长度，包含最后两个字节的校验码
	u16 _PktLen;//数据区长度保密码
	u16 PktSn;//包序列号
	GLOBA_PKT_TYPE Type;//此包的类型
	GLOBA_PKT_RESULT Res;//0表示是主包，非0表示是回包，并表示具体的回复码
	u32 DutID;//fly的id
	u32 AppID;//app连接的id
	
	RECORD_OPERATE_PKT_TYPE Act;//动作
	INFO_TYPE Name;//指定操作对象
	u16 Num;
	u8 Data[2];//实体
}RECORD_OPERATE_PKT;
#endif

#if 0//esp 无此功能
//------------------------------------倒计时任务包-----------------------------
#if 0
#include "SceneRecord.h"

typedef enum{
	CDPT_NULL=0,
	CDPT_NEW,
	CDPT_DELETE,
	CDPT_READ,
	CDPT_GET_NUM,
	
	CDPT_MAX,
}COUNT_DOWN_PKT_TYPE;

typedef struct{
	u32 RandID;//随机ID
	u32 CountDownSec;//倒计时总时间
	u32 RemainSec;//剩余秒数
	SCENE_ITEM Item;//任务
}COUNT_DOWN_PKT_ITEM;

typedef struct{
	u16 PktLen;//数据区长度，包含最后两个字节的校验码
	u16 _PktLen;//数据区长度保密码
	u16 PktSn;//包序列号
	GLOBA_PKT_TYPE Type;//此包的类型
	GLOBA_PKT_RESULT Res;//0表示是主包，非0表示是回包，并表示具体的回复码
	u32 DutID;//fly的id
	u32 AppID;//app连接的id
	
	COUNT_DOWN_PKT_TYPE Act;//动作
	u8 Num;//任务个数
   	u8 Data[2];//实体，实际存放的是COUNT_DOWN_PKT_ITEM数组
}COUNT_DOWN_PKT;
#endif
#endif

#if 0//esp 无此功能
//-----------------------------------waver量产包--------------------------------
typedef enum{
	WCT_NULL=0,
	WCT_TEST,
	WCT_CONFIG,

	
}WAVER_CONFIG_TYPE;

typedef struct{
	u16 PktLen;//数据区长度，包含最后两个字节的校验码
	u16 _PktLen;//数据区长度保密码580319
	u16 PktSn;//包序列号
	GLOBA_PKT_TYPE Type;//此包的类型
	GLOBA_PKT_RESULT Res;//0表示是主包，非0表示是回包，并表示具体的回复码
	u32 DutID;//fly的id
	u32 AppID;//app连接的id
	
	WAVER_CONFIG_TYPE Act;
	u8 Num;
	u32 WDevAddr;
	u8 Data[2];//WAVER_PARAM_TAB Tab;
}WAVER_CONFIG_PKT;
#endif

//---------------------------------app心跳包------------------------------------
typedef struct{
	u16 PktLen;//数据区长度，包含最后两个字节的校验码
	u16 _PktLen;//数据区长度保密码580319
	u16 PktSn;//包序列号
	GLOBA_PKT_TYPE Type;//此包的类型
	GLOBA_PKT_RESULT Res;//0表示是主包，非0表示是回包，并表示具体的回复码
	u32 DutID;//fly的id
	u32 AppID;//app连接的id

	u32 Flag;//
	u8 Data[2];//
}APP_BEAT_PKT;










//------------------------------------------------------------------------------

#define MaxPktDataLen(x) (PKT_MAX_DATA_LEN-sizeof(x))

#define CLIENT_T_LOCAL 			0x00000000//直接连接
#define CLIENT_T_SRV_APP 		0x80000000	//服务器转发app问询
#define CLIENT_T_MASK 			0x80000000
#define GetClientFlag(pClient) (CLIENT_T_MASK&(u32)(pClient))
#define CleanClientFlag(pClient) ((~CLIENT_T_MASK)&(u32)(pClient))

enum{
	PHR_NO_REPLY=0,//不回复
	PHR_FALSE,//回复错误
	PHR_OK,//回复成功
	PHR_ASYNC,//异步回复
};
typedef u8 PKT_HANDLER_RES;

enum{
	DST_ONLINE=0,
	DST_ONLY, //编辑态
};
typedef u8 DUT_STATUS;

typedef struct{
	u16 PktLen;//数据区长度，包含最后两个字节的校验码
	u16 _PktLen;//数据区长度保密码
	u16 PktSn;//包序列号
	GLOBA_PKT_TYPE Type;//此包的类型
	GLOBA_PKT_RESULT Res;//0表示是主包，非0表示是回包，并表示具体的回复码
	u32 DutID;//fly的id
	u32 AppID;//app连接的id

	u16 SecretKey;	//返回的密钥
	DUT_STATUS DutStatus;//当前主板的状态
	u8 DutType;//0:future, 1:qwifi
	u32 PwHash; //返回密码哈希值

	u32 SysConfigFlag;//系统参数配置标识
	u32 InfoConfigFlag[IFT_SAVE_NUM];//信息配置标识

	u32 RtcCnt;

	u8 Data[2];//存放包检验
}SRV_LOGIN_PKT;

enum{
	SMF_SYS=0,//系统信息，服务器需将信息广播给所有主机
	SMF_GSM,//短信，服务器需将信息gsm给指定用户
	SMF_DEV_STR,//设备字符串，被dut推送给app，由微信消息修改
	SMF_PUSH,//推送信息，服务器需将信息由推送发给指定用户
};
typedef u8 SRV_MSG_FLAG;

typedef struct{
	u16 PktLen;//数据区长度，包含最后两个字节的校验码
	u16 _PktLen;//数据区长度保密码
	u16 PktSn;//包序列号
	GLOBA_PKT_TYPE Type;//此包的类型
	GLOBA_PKT_RESULT Res;//0表示是主包，非0表示是回包，并表示具体的回复码
	u32 DutID;//fly的id
	u32 AppID;//app连接的id

	u32 Param[4];//预留的数据
	u8 MsgFlag;//发送掩码，每位的意义，由SRV_MSG_FLAG指定
	u8 a;
	
	u16 DataLen;//字符串长度，包含结束符
	u8 Data[2];
	//字符串存放格式[UserName]:[MsgString]，例如
	//karlno:高温报警信息  ->  将信息发给karlno
	//0:高温报警信息   ->  将信息发给所有家庭用户
	
}SRV_MSG_PKT;
//字符串上行下行包
//上行:dut执行情景，dut发送字符串给服务器，要求服务器转发处理
//下行:服务器转发字符串，从平台接口过来，从其他板卡过来


#endif




