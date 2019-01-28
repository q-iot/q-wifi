#ifndef NET_TYPE_H
#define NET_TYPE_H

#include "LimitMarco.h"
#include "PublicEvent.h"

typedef u32 ID_T;

//---------------------------------------------------------------------------
//ע:���м�¼��ͷ�ֽڱ��뱣֤�洢ȫ�ֶ�������
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
	

	GOT_MAX//���ܳ���255�������������������
}GLOBA_OBJ_TYPE;


//---------------------------------------------------------------------------


//--------------------------------����------------------------------------
enum{
	PT_NULL=0,//����
	PT_SYS_PARAM,//ϵͳ�����趨
	PT_BULK_OPERATE,//info��������
	PT_SIG_OPERATE,//�ź��趨,����¼�룬�ر�¼�룬����¼�룬�洢¼��
	PT_INFO_OPERATE,//info��������
	PT_WDEV_CTRL,//jumper����Ĺ�ͨ
	PT_KEY_CTRL,//��������
	PT_FLASH_OPERATE,//ר�����spi flash���ж�д����
	PT_LOGIN,//��¼��
	PT_STATUS,//״̬��
	PT_CMD,//������
	PT_VARIABLE,//������
	PT_UPDATE,//�ϱ���
	PT_WAVER_CONFIG,//waver���ð�
	PT_APP_BEAT,//app������������
	PT_APP_ACT,//app������
	PT_APP_ATTENTION,//app��ע��
	PT_LOCAL_MAX,
	
	PT_SRV_BEAT=0x20,//������������������
	PT_SRV_LOGIN,//��¼�����������ص�½������
	PT_SRV_QUERY,//��ѯ��
	PT_SRV_MSG,//��Ϣ�����������������˫����Ϣ����
	PT_SRV_UNBIND,//���������ͥ�İ�
	PT_SRV_CMD,//�����������
	
	PT_APP_EXPEND=0x7f,//��չ����app�������������ͨ
	
	PT_MAX,
};
typedef u8 GLOBA_PKT_TYPE;

enum{
	GPR_MAIN=0,
	
	GPR_SUCESS,
	GPR_FAILD,
	GPR_PARAM_ERROR,//����Ĳ�������
	GPR_RES_NULL,//��Ҫ��ȡ����ԴΪ��
	GPR_RES_UNENOUGH,//��Դ����
	GPR_NUM_OVERFLOW,//��������
	GPR_ROUTING_ERROR,//�������
	GPR_HARDWARD_ERROR,//Ӳ������
	GPR_AUTH_ERROR,//Ȩ�޴���
	GPR_TIMEOUT,//��ʱ
	GPR_BUSY,//æ
	GPR_OFFLINE,//������
	
	GPR_MAX
};
typedef u8 GLOBA_PKT_RESULT;

typedef struct {
	u16 PktLen;//���������ȣ�������������ֽڵ�У����
	u16 _PktLen;//���������ȱ�����
	u16 PktSn;//�����к�
	GLOBA_PKT_TYPE Type;//�˰�������
	GLOBA_PKT_RESULT Res;//0��ʾ����������0��ʾ�ǻذ�������ʾ����Ļظ���
	u32 DutID;//fly��id
	u32 AppID;//app���ӵ�id
	
	u8 Data[2];//������	
	//������������ֽڶ�����������У��
}GLOBA_PKT_HEADER;

#define GLOBA_PKT_HD_LEN (sizeof(GLOBA_PKT_HEADER)-4)

//---------------------------------------------------------------------------


//----------------------------------��½��-----------------------------------------
#include "AppClientManger.h"

typedef struct{
	u16 PktLen;//���������ȣ�������������ֽڵ�У����
	u16 _PktLen;//���������ȱ�����
	u16 PktSn;//�����к�
	GLOBA_PKT_TYPE Type;//�˰�������
	GLOBA_PKT_RESULT Res;//0��ʾ����������0��ʾ�ǻذ�������ʾ����Ļظ���
	u32 DutID;//fly��id
	u32 AppID;//app���ӵ�id

	APP_CLIENT_TYPE ClientType;//��¼���ͣ�����ʱ���ؿͻ����������ͻ��˼���PktSnʱ������Ӵ���������4λ��
	u8 VendorID;//���̺���
	u16 FwVer;//�̼��汾��app����ʱ�����Լ��İ汾�ţ�dut�ذ�ʱҲ�����Լ��İ汾��
	u32 SnHash;//���к���MakeHash���ֵ
	u32 PwHash;//������MakeHash���ֵ
	u16 SecretKey;	//���ص���Կ
	u16 Role;//��ɫ	
	u32 SysConfigFlag;//ϵͳ�������ñ�ʶ
	u32 InfoConfigFlag[IFT_SAVE_NUM];//��Ϣ���ñ�ʶ
	u8 Data[2];//��Ű�����
}LOGIN_PKT;

//------------------------------------ϵͳ������-----------------------------
//#include "DbForSys.h"

enum{
	SPT_GET=0,
	SPT_SET,
	SPT_BURN,
	SPT_DEF,//�������ָ�Ĭ��
	SPT_DEFEND_GET,//��ȡ������龰�б�
	SPT_DEFEND_SET,//������Ҫ������龰
	SPT_DEFEND_CLR,//������Ҫ���ε��龰


	
	SPT_MAX
};
typedef u8 SYS_PARAM_TYPE;

typedef struct{
	u16 PktLen;//���������ȣ�������������ֽڵ�У����
	u16 _PktLen;//���������ȱ�����
	u16 PktSn;//�����к�
	GLOBA_PKT_TYPE Type;//�˰�������
	GLOBA_PKT_RESULT Res;//0��ʾ����������0��ʾ�ǻذ�������ʾ����Ļظ���
	u32 DutID;//fly��id
	u32 AppID;//app���ӵ�id

	u32 SysConfigFlag;//���ñ�ʶ
	SYS_PARAM_TYPE Act;
	u8 ItemNum;//data��tlv�ĸ��������龰id����
	u16 a;
	u8 Data[2];//ʵ�壬����4�ֽڶ���
}SYS_PARAM_PKT;
//---------------------------------------------------------------------------


//----------------------------------������������-----------------------------
#include "InfoSave.h"

enum{
	BOPT_ARG=0,//��ȡ��Ϣ��Ԫ��Ϣ
	BOPT_GET,//��ȡ��Ϣ
	
	BOPT_MAX
};
typedef u8 BULK_OPERATE_PKT_TYPE;

typedef struct{
	u16 Size;//��Ԫ��С
	u16 VaildNum;//��Ч��Ԫ��
	u16 FreeNum;//���пռ�
	u16 OnceNum;//һ�οɴ����С
}BULK_OPERATE_PKT_INFO;

typedef struct{
	u16 PktLen;//���������ȣ�������������ֽڵ�У����
	u16 _PktLen;//���������ȱ�����
	u16 PktSn;//�����к�
	GLOBA_PKT_TYPE Type;//�˰�������
	GLOBA_PKT_RESULT Res;//0��ʾ����������0��ʾ�ǻذ�������ʾ����Ļظ���
	u32 DutID;//fly��id
	u32 AppID;//app���ӵ�id
	
	BULK_OPERATE_PKT_TYPE Act;//����
	INFO_TYPE Name;//ָ����������
	u16 Idx;
	u16 Num;
	u16 a;
	u8 Data[2];//ʵ�壬����4�ֽڶ���
}BULK_OPERATE_PKT;
//---------------------------------------------------------------------------

//------------------------------------��Ϣ�������ð�-----------------------------
#include "RecordVariable.h"
enum{
	IOPT_READ=0,//��ȡ��Ϣ
	IOPT_READ_BY_IDX,
	IOPT_DELETE,
	IOPT_NEW,
	
	IOPT_MAX
};
typedef u8 INFO_OPERATE_PKT_TYPE;

typedef struct{
	u16 PktLen;//���������ȣ�������������ֽڵ�У����
	u16 _PktLen;//���������ȱ�����
	u16 PktSn;//�����к�
	GLOBA_PKT_TYPE Type;//�˰�������
	GLOBA_PKT_RESULT Res;//0��ʾ����������0��ʾ�ǻذ�������ʾ����Ļظ���
	u32 DutID;//fly��id
	u32 AppID;//app���ӵ�id
	
	u32 InfoConfigFlag;//���ñ�ʶ
	INFO_OPERATE_PKT_TYPE Act;//����
	INFO_TYPE Name;//ָ����������
	u16 Num;
	u8 Data[2];//ʵ�壬����4�ֽڶ���
}INFO_OPERATE_PKT;

typedef struct{
	GLOBA_PKT_RESULT Res;//������Ϣ�����������GPR_SUCESS��ʾ�ɹ�
	u8 a;
	u16 b;
	ID_T InfoID;//������������Ķ���id
	CHAR_NUM_UNI ProdTag;//����Ǳ���������tag���ܻᱻ�����ı䣬����Ҫ����ʵ�ʵ�tag
	u32 c;
}INFO_PKT_RESP;

//---------------------------------------------------------------------------

#if 0//esp �޴˹���
//------------------------------------�źŲ������ð�-----------------------------
#include "Ir_Rf_Record.h"

typedef enum{
	SOT_NULL=0,
	SOT_IR,
	SOT_RF,
	SOT_IR_TRANS,
	SOT_RF_TRANS,//rfת��

	SOT_MAX
}SIGNAL_OPT_TYPE;

typedef enum{
	SOPT_STUDY=0,//ѧϰ�ź�
	SOPT_SEND,
	SOPT_CAPTURE,//�����ź�������Դ
	
	SOPT_MAX
}SIG_OPERATE_PKT_TYPE;

typedef struct{
	u16 PktLen;//���������ȣ�������������ֽڵ�У����
	u16 _PktLen;//���������ȱ�����
	u16 PktSn;//�����к�
	GLOBA_PKT_TYPE Type;//�˰�������
	GLOBA_PKT_RESULT Res;//0��ʾ����������0��ʾ�ǻذ�������ʾ����Ļظ���
	u32 DutID;//fly��id
	u32 AppID;//app���ӵ�id
	
	SIG_OPERATE_PKT_TYPE Act;//����
	SIGNAL_OPT_TYPE SignalType;//����
	u16 Num;//������ż�ֵ
	u32 WDevAddr;//ת���豸
	u8 Data[2];
}SIG_OPERATE_PKT;
//---------------------------------------------------------------------------
#endif

//--------------------------------�����豸������-----------------------------------
#include "RecordDevice.h"

enum{
	WCPT_NULL=0,
	WCPT_DEV_SEARCH,
	WCPT_DEV_ADD,//ͨ���������豸
	WCPT_DEV_DELETE,//ͨ����ɾ���豸
	WCPT_TEST,//�豸��������

	WCPT_MAX
};
typedef u8 WDEV_CTRL_PKT_TYPE;

typedef struct{
	u16 PktLen;//���������ȣ�������������ֽڵ�У����
	u16 _PktLen;//���������ȱ�����
	u16 PktSn;//�����к�
	GLOBA_PKT_TYPE Type;//�˰�������
	GLOBA_PKT_RESULT Res;//0��ʾ����������0��ʾ�ǻذ�������ʾ����Ļظ���
	u32 DutID;//fly��id
	u32 AppID;//app���ӵ�id
	
	WDEV_CTRL_PKT_TYPE Act;//����
	u8 Num;
	WAVER_DUT_TYPE DutType;
	u32 WDevAddr;
	u32 Footer;//ת����ַ
	u8 Data[2];//UPKT_WDEV_LIST
}WDEV_CTRL_PKT;

//---------------------------------��ֵ���ư�------------------------------------------
typedef struct{
	u16 PktLen;//���������ȣ�������������ֽڵ�У����
	u16 _PktLen;//���������ȱ�����
	u16 PktSn;//�����к�
	GLOBA_PKT_TYPE Type;//�˰�������
	GLOBA_PKT_RESULT Res;//0��ʾ����������0��ʾ�ǻذ�������ʾ����Ļظ���
	u32 DutID;//fly��id
	u32 AppID;//app���ӵ�id
	
	u8 Num;	 
	u8 a;
	u16 b;
	u8 Data[2];//ʵ�壬��SCENE_ITEMΪ��λ���ж����Ĵ��
}KEY_CTRL_PKT;

#if 0//esp �޴˹���
//---------------------------------------------------------------------------
typedef enum{
	FOPT_READ=0,//������ȡ
	FOPT_WRITE,//�����洢
	FOPT_ERASE,//������

	FOPT_MAX
}FLASH_OPERATE_PKT_TYPE;

typedef enum{
	FBN_OEM=0,
	FBN_UNICODE,
	FBN_IAP,

	FBN_MAX
}FLASH_BLOCK_NAME;

typedef struct{
	u16 PktLen;//���������ȣ�������������ֽڵ�У����
	u16 _PktLen;//���������ȱ�����
	u16 PktSn;//�����к�
	GLOBA_PKT_TYPE Type;//�˰�������
	GLOBA_PKT_RESULT Res;//0��ʾ����������0��ʾ�ǻذ�������ʾ����Ļظ���
	u32 DutID;//fly��id
	u32 AppID;//app���ӵ�id
	
	FLASH_OPERATE_PKT_TYPE Act;//����
	FLASH_BLOCK_NAME Name;
	u16 Page;//��0��ʼ
	u16 Num;
	u16 a;
	u8 Data[2];//ʵ��
}FLASH_OPERATE_PKT;
#endif

//------------------------------------״̬��-----------------------------
enum{
	SCT_NULL=0,
	SCT_EDIT_START,//����༭̬
	SCT_EDIT_END,//�˳��༭̬
	SCT_UPGRADE,//��ʼ����
	
	SCT_MAX,
};
typedef u8 STATUS_CMD_TYPE;

typedef struct{
	u16 PktLen;//���������ȣ�������������ֽڵ�У����
	u16 _PktLen;//���������ȱ�����
	u16 PktSn;//�����к�
	GLOBA_PKT_TYPE Type;//�˰�������
	GLOBA_PKT_RESULT Res;//0��ʾ����������0��ʾ�ǻذ�������ʾ����Ļظ���
	u32 DutID;//fly��id
	u32 AppID;//app���ӵ�id
	
	STATUS_CMD_TYPE Act;
	u8 Num;
	u16 a;
	u8 Data[2];
}STATUS_CMD_PKT;


//------------------------------------������-----------------------------
#include "RecordVariable.h"
enum{
	VPA_NULL=0,
	VPA_QUERY,//��ѯ
	VPA_BIND,//��
	VPA_RELASE,//���
	VPA_SET,//����ֵ

	VPA_MAX
};
typedef u8 VARI_GET_ACT;

enum{
	VGM_ID=0,//ͨ��id��ȡ
	VGM_TAG,//ͨ��tagֱ�ӻ�ȡ
};
typedef u8 VARI_GET_METHOD;

typedef struct{
	CHAR_NUM_UNI ProdTag;//�Բ�ƷΪ���ֵ�Ψһ��ǩ���ɿ�����ˣ�QSRV��QDEV��Q��ͷ��Ϊ������ǩ�������ִ�Сд
	CHAR_NUM_UNI VarTag;//�Ա���Ϊ���ֵı�ǩ���ɳ��������ƶ������ظ��������ִ�Сд
}VARI_GET_BY_TAG;

typedef struct{
	u32 DevID;//�豸id
	VAR_IDX_T VarIdxType:2;//��������
	u8 Idx:6;//��������,��1��ʼ
}VARI_GET_BY_DEV;

typedef struct{
	union{
		VARI_GET_BY_TAG Tag;//ͨ��tag����������������tag��ͬ�ı�����ϵͳ�������û����������ַ���
		VARI_GET_BY_DEV Dev;//ͨ��dev����������������dev��Ӧ������������߹�������.�豸���������ַ���
	}Info;

	u32 VarID;//����id������ֵ������0��ʾδ�ɹ�
	VARI_GET_ACT Act:4;//����
	VARI_GET_METHOD Method:4;//ƥ������ķ���	
	VAR_STATE VarState;//���صı���״̬
	TVAR VarVal;//���û򷵻صı���ֵ
}VARI_PKT_ITEM;

enum{
	VPC_NULL=0,
	VPC_RELEASE_ALL,//�������
};
typedef u8 VARI_PKT_CMD;

typedef struct{
	u16 PktLen;//���������ȣ�������������ֽڵ�У����
	u16 _PktLen;//���������ȱ�����
	u16 PktSn;//�����к�
	GLOBA_PKT_TYPE Type;//�˰�������
	GLOBA_PKT_RESULT Res;//0��ʾ����������0��ʾ�ǻذ�������ʾ����Ļظ���
	u32 DutID;//fly��id
	u32 AppID;//app���ӵ�id
	
	VARI_PKT_CMD Cmd;//����ָ��
	u8 Num;//VARI_PKT_ITEM����
	u16 b;
	u8 Data[2];//�������ű���ֵ�б���λVARI_PKT_ITEM
}VARIABLE_PKT;

//------------------------------------app��ͼ��-----------------------------
enum{
	AAT_NULL=0,
	AAT_INTO_DEV,//�����豸
	AAT_LEAVE_DEV,//�뿪�豸

};
typedef u8 APP_ACT_TYPE;

typedef struct{
	u16 PktLen;//���������ȣ�������������ֽڵ�У����
	u16 _PktLen;//���������ȱ�����
	u16 PktSn;//�����к�
	GLOBA_PKT_TYPE Type;//�˰�������
	GLOBA_PKT_RESULT Res;//0��ʾ����������0��ʾ�ǻذ�������ʾ����Ļظ���
	u32 DutID;//fly��id
	u32 AppID;//app���ӵ�id
	
	APP_ACT_TYPE ActType;//��ͼ����
	u8 Num;//item����
	u16 b;
	u8 Data[2];//���item
}APP_ACT_PKT;

//------------------------------------app��ע��-----------------------------
enum{
	AATA_NULL=0,
	AATA_ATTENT,//��ʼ����
	AATA_CANCEL,//ȡ������
};
typedef u8 APP_ATTENTION_ACT;

enum{
	AATT_NULL=0,
	AATT_ALL,//ȡ��app���ж���
	AATT_DEV,//���Ļ�ȡ���豸����
	AATT_VAR,//���Ļ�ȡ����������

};
typedef u8 APP_ATTENTION_TYPE;

typedef struct{
	APP_ATTENTION_ACT Act;
	APP_ATTENTION_TYPE ObjType;
	u16 b;
	u32 ObjID;
	TVAR32 Param;//�洢����ֵ
}APP_ATTENTION_LIST;

typedef struct{
	u16 PktLen;//���������ȣ�������������ֽڵ�У����
	u16 _PktLen;//���������ȱ�����
	u16 PktSn;//�����к�
	GLOBA_PKT_TYPE Type;//�˰�������
	GLOBA_PKT_RESULT Res;//0��ʾ����������0��ʾ�ǻذ�������ʾ����Ļظ���
	u32 DutID;//fly��id
	u32 AppID;//app���ӵ�id
	
	u8 Num;//item����
	u8 a;
	u16 DataLen;
	u8 Data[2];//���APP_ATTENTION_LIST
}APP_ATTENTION_PKT;

//------------------------------------�ϱ���-----------------------------
enum{
	UPA_NULL=0,
	UPA_IR_STUDY,//irѧϰ���󣬴��ݸ�app
	UPA_RF_STUDY,//rfѧϰ���󣬴��ݸ�app
	UPA_WDEV_SEARCH,//wdev���������󣬴��ݸ�app
	UPA_OVER_CLIENT,//�ߵ��ͻ���
	UPA_VAR,//�����ϱ�
	UPA_STR,//�ַ����ϱ�
	UPA_BEAT,//������
	UPA_RF_CAPTURE,//rf����
	UPA_DEV_STATE,//�豸״̬
	
	UPA_SRV=100,
	
	UPA_MAX
};
typedef u8 UPDATE_PKT_ACT;

typedef struct{
	u32 DevAddr;
	u32 HostAddr;
}UPKT_WDEV_LIST;//�ϱ���wnet dev�б�

#if 0//esp �޴˹���
typedef struct{
	u32 DevInfoFlag;//���µ�device��info flagֵ
	u32 DevID;//device info��id
	u32 SigID;//��Ӧ���ź�id
	u8 Key;//��ֵ
	GLOBA_PKT_RESULT Res;	
}UPKT_SIG_STUDY;//�ϱ����ź�ѧϰ���

typedef struct{
	u32 Code;
	u16 BasePeriod;
	u16 a;
}UPKT_SIG_CAPTURE;//�������Ƶ
#endif

typedef struct{
	u32 VarID;//����id������ֵ������0��ʾδ�ɹ�
	u8 a;
	VAR_STATE VarState;//���صı���״̬
	TVAR VarVal;//���صı���ֵ	
}UPKT_VAR;//�ϱ��ı�������ֵ

typedef struct{
	u32 DevID;//�豸�ţ�0Ϊϵͳ�ַ���
	u32 StrID;
	u8 StrBuf[2];
}UPKT_STR;

typedef struct{
	u32 DevID;//�豸��
	bool IsOnline;	//����Ϊ1��������Ϊ0
	u8 a;
	u16 b;
}UPKT_DEV_STATE;

typedef struct{
	u16 PktLen;//���������ȣ�������������ֽڵ�У����
	u16 _PktLen;//���������ȱ�����
	u16 PktSn;//�����к�
	GLOBA_PKT_TYPE Type;//�˰�������
	GLOBA_PKT_RESULT Res;//0��ʾ����������0��ʾ�ǻذ�������ʾ����Ļظ���
	u32 DutID;//fly��id
	u32 AppID;//app���ӵ�id
	
	UPDATE_PKT_ACT Act;
	u8 Num;
	u16 DataLen;
	u8 Data[2];	//UPKT_WDEV_LIST��UPKT_SIG_STUDY��UPKT_SIG_CAPTURE��UPKT_VAR��UPKT_STR
}UPDATE_PKT;

#if 0//esp �޴˹���
//------------------------------------��¼�������-----------------------------
typedef enum{
	ROPT_NULL=0,
	ROPT_NEW,
	ROPT_DELETE,
	
	ROPT_MAX
}RECORD_OPERATE_PKT_TYPE;

typedef struct{
	u16 PktLen;//���������ȣ�������������ֽڵ�У����
	u16 _PktLen;//���������ȱ�����
	u16 PktSn;//�����к�
	GLOBA_PKT_TYPE Type;//�˰�������
	GLOBA_PKT_RESULT Res;//0��ʾ����������0��ʾ�ǻذ�������ʾ����Ļظ���
	u32 DutID;//fly��id
	u32 AppID;//app���ӵ�id
	
	RECORD_OPERATE_PKT_TYPE Act;//����
	INFO_TYPE Name;//ָ����������
	u16 Num;
	u8 Data[2];//ʵ��
}RECORD_OPERATE_PKT;
#endif

#if 0//esp �޴˹���
//------------------------------------����ʱ�����-----------------------------
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
	u32 RandID;//���ID
	u32 CountDownSec;//����ʱ��ʱ��
	u32 RemainSec;//ʣ������
	SCENE_ITEM Item;//����
}COUNT_DOWN_PKT_ITEM;

typedef struct{
	u16 PktLen;//���������ȣ�������������ֽڵ�У����
	u16 _PktLen;//���������ȱ�����
	u16 PktSn;//�����к�
	GLOBA_PKT_TYPE Type;//�˰�������
	GLOBA_PKT_RESULT Res;//0��ʾ����������0��ʾ�ǻذ�������ʾ����Ļظ���
	u32 DutID;//fly��id
	u32 AppID;//app���ӵ�id
	
	COUNT_DOWN_PKT_TYPE Act;//����
	u8 Num;//�������
   	u8 Data[2];//ʵ�壬ʵ�ʴ�ŵ���COUNT_DOWN_PKT_ITEM����
}COUNT_DOWN_PKT;
#endif
#endif

#if 0//esp �޴˹���
//-----------------------------------waver������--------------------------------
typedef enum{
	WCT_NULL=0,
	WCT_TEST,
	WCT_CONFIG,

	
}WAVER_CONFIG_TYPE;

typedef struct{
	u16 PktLen;//���������ȣ�������������ֽڵ�У����
	u16 _PktLen;//���������ȱ�����580319
	u16 PktSn;//�����к�
	GLOBA_PKT_TYPE Type;//�˰�������
	GLOBA_PKT_RESULT Res;//0��ʾ����������0��ʾ�ǻذ�������ʾ����Ļظ���
	u32 DutID;//fly��id
	u32 AppID;//app���ӵ�id
	
	WAVER_CONFIG_TYPE Act;
	u8 Num;
	u32 WDevAddr;
	u8 Data[2];//WAVER_PARAM_TAB Tab;
}WAVER_CONFIG_PKT;
#endif

//---------------------------------app������------------------------------------
typedef struct{
	u16 PktLen;//���������ȣ�������������ֽڵ�У����
	u16 _PktLen;//���������ȱ�����580319
	u16 PktSn;//�����к�
	GLOBA_PKT_TYPE Type;//�˰�������
	GLOBA_PKT_RESULT Res;//0��ʾ����������0��ʾ�ǻذ�������ʾ����Ļظ���
	u32 DutID;//fly��id
	u32 AppID;//app���ӵ�id

	u32 Flag;//
	u8 Data[2];//
}APP_BEAT_PKT;










//------------------------------------------------------------------------------

#define MaxPktDataLen(x) (PKT_MAX_DATA_LEN-sizeof(x))

#define CLIENT_T_LOCAL 			0x00000000//ֱ������
#define CLIENT_T_SRV_APP 		0x80000000	//������ת��app��ѯ
#define CLIENT_T_MASK 			0x80000000
#define GetClientFlag(pClient) (CLIENT_T_MASK&(u32)(pClient))
#define CleanClientFlag(pClient) ((~CLIENT_T_MASK)&(u32)(pClient))

enum{
	PHR_NO_REPLY=0,//���ظ�
	PHR_FALSE,//�ظ�����
	PHR_OK,//�ظ��ɹ�
	PHR_ASYNC,//�첽�ظ�
};
typedef u8 PKT_HANDLER_RES;

enum{
	DST_ONLINE=0,
	DST_ONLY, //�༭̬
};
typedef u8 DUT_STATUS;

typedef struct{
	u16 PktLen;//���������ȣ�������������ֽڵ�У����
	u16 _PktLen;//���������ȱ�����
	u16 PktSn;//�����к�
	GLOBA_PKT_TYPE Type;//�˰�������
	GLOBA_PKT_RESULT Res;//0��ʾ����������0��ʾ�ǻذ�������ʾ����Ļظ���
	u32 DutID;//fly��id
	u32 AppID;//app���ӵ�id

	u16 SecretKey;	//���ص���Կ
	DUT_STATUS DutStatus;//��ǰ�����״̬
	u8 DutType;//0:future, 1:qwifi
	u32 PwHash; //���������ϣֵ

	u32 SysConfigFlag;//ϵͳ�������ñ�ʶ
	u32 InfoConfigFlag[IFT_SAVE_NUM];//��Ϣ���ñ�ʶ

	u32 RtcCnt;

	u8 Data[2];//��Ű�����
}SRV_LOGIN_PKT;

enum{
	SMF_SYS=0,//ϵͳ��Ϣ���������轫��Ϣ�㲥����������
	SMF_GSM,//���ţ��������轫��Ϣgsm��ָ���û�
	SMF_DEV_STR,//�豸�ַ�������dut���͸�app����΢����Ϣ�޸�
	SMF_PUSH,//������Ϣ���������轫��Ϣ�����ͷ���ָ���û�
};
typedef u8 SRV_MSG_FLAG;

typedef struct{
	u16 PktLen;//���������ȣ�������������ֽڵ�У����
	u16 _PktLen;//���������ȱ�����
	u16 PktSn;//�����к�
	GLOBA_PKT_TYPE Type;//�˰�������
	GLOBA_PKT_RESULT Res;//0��ʾ����������0��ʾ�ǻذ�������ʾ����Ļظ���
	u32 DutID;//fly��id
	u32 AppID;//app���ӵ�id

	u32 Param[4];//Ԥ��������
	u8 MsgFlag;//�������룬ÿλ�����壬��SRV_MSG_FLAGָ��
	u8 a;
	
	u16 DataLen;//�ַ������ȣ�����������
	u8 Data[2];
	//�ַ�����Ÿ�ʽ[UserName]:[MsgString]������
	//karlno:���±�����Ϣ  ->  ����Ϣ����karlno
	//0:���±�����Ϣ   ->  ����Ϣ�������м�ͥ�û�
	
}SRV_MSG_PKT;
//�ַ����������а�
//����:dutִ���龰��dut�����ַ�������������Ҫ�������ת������
//����:������ת���ַ�������ƽ̨�ӿڹ������������忨����


#endif




