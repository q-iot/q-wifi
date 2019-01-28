#ifndef STR_SAVE_H
#define STR_SAVE_H


enum{
	SRT_NULL=0,
	SRT_STR,//�ַ����������龰��Ϣ���ݣ�����Դ��Ϣ���ݵȵ�
	SRT_DATA,//����
	SRT_DEV_STR,//�豸�����ַ�����SubID��ʾ�����豸��id�������豸�Զ����ַ���
	SRT_VAR_UP_CONF,//�����ϱ����ã�SubID��ʾ��Ӧ����id��Num��ʾ�ϱ�����(0,�仯��1-19����)�����ڼ�¼���ϱ��ı���
};
typedef u8 STR_REC_TYPE;

typedef struct{
	u32 ID;
	u32 SubID;//�󶨵�������ID
	STR_REC_TYPE Type;
	u8 Num;
	u16 DataLen;
	u8 Data[STR_RECORD_BYTES];
}STR_RECORD;

#endif

