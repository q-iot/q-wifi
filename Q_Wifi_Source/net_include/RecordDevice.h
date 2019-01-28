 #ifndef DEVICE_RECORD_H
#define DEVICE_RECORD_H

typedef struct{
	u32 TransAddr;//�����źŵ�ת������ַ
	u8 KeyList[KEYS_SET_MAX_KEY_NUM];//�洢�źŶ�Ӧ��keyֵ
	u16 SigList[KEYS_SET_MAX_KEY_NUM];//�洢�ź�
}KEYS_SET_DEV;

enum{
	WDT_NULL=0,
	WDT_WDEV_IO,//����ģ��
	WDT_OPEN_GATEWAY,//͸��zigbee
	WDT_JMP,//ת����
	WDT_COM,//����͸��
	WDT_WIFI,//wifiģ���豸
	WDT_LOW_POWER,//�͹����豸
	WDT_BLUE_TOOTH,//�����豸

};
typedef u8 WAVER_DUT_TYPE;

typedef struct{
	u32 WNetAddr;//��������ַ����ַ����Žṹ�ײ�
	u32 Footer;
	WAVER_DUT_TYPE DutType;
	u8 SelfVarNum;
	bool Online:1;//�����洢�豸�Ƿ����ߣ��洢��flash��ʱ�����ã����ݴ��ݵ�ʱ�����������
	bool NeedPw:1;//��Ҫ������ܽ���
	u8 a:6;
	u8 b;
	u16 RelateVar[RELATE_VAR_MAX];//ÿ���豸��ע�����Ա�
	u16 AwakenCode;//�͹��Ļ�����
	u16 AwakenLifeSecDef;//����ʱ�����û�����Ѵ��ʱ�䣬��ʹ�ô�ʱ��
}WAVER_DEV;

enum{
	CSM_NULL=0,
	CSM_SUM,//�ۼ�
	CSM_CRC,
	CSM_HASH33,
};
typedef u8 CHK_SUM_METHOD;

typedef struct{
	u8 VendorID;//����id��0ͨ�ô����豸��1��ά
	bool AddKeyToCmd:1;//����key���뵽�����ַ���
	bool ChkSumRev:1;//�����뷴�����
	u8 ChkSumLen:2;//У���볤��
	CHK_SUM_METHOD ChkSumMothed:4;//У���뷽��
	u8 a;//��϶
	u8 PktHeaderLen:4;//���Ӱ�ͷ����
	u8 AckLen:4;//�ظ��ĳ���
	u8 PktHeader[4];//���Ӱ�ͷ
	u8 SuccessAck[4];//�ɹ��Ļظ�������и��Ӱ�ͷ��Ҳ����븽�Ӱ�ͷ
	u8 FaildAck[4];//ʧ�ܵĻظ�
}COM_DEV;//�������������

typedef struct{
	u32 ID;
	u8 Name[SHORT_STR_BYTES];
	u32 FileID;//��Ʒ�ͺ��ļ�ID���ƶ�Ψһ
	u32 ProdID;//����ID����16λ��ʾ���ң���16λ��ʾ�豸����
	u32 UiID;//UI ID
	union{
		WAVER_DEV Waver;//˫���豸����ַ������ײ�
		KEYS_SET_DEV KeysSet;
		COM_DEV Com;
	}Record;
}DEVICE_RECORD;

#endif

