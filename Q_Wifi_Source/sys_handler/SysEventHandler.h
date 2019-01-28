#ifndef SYS_EVENT_HANDLER
#define SYS_EVENT_HANDLER


typedef enum{
	MFM_NULL=0,
	MFM_ALWAYS,//����ģʽ��ֻҪ���ã���һֱ�����
	MFM_COVER,//�����ظ������¼���ֻ�ᷢ��һ�Σ��ڶ��ε���ֻ�޸�ʱ��
	MFM_NEAREST,//�����ظ������¼����޸ĵ�ʱ��һ���ǵ������õ����ʱ��
}MS_FUNC_MODE;


typedef enum{
	SEN_NULL=0,
	SEN_SYS_CMD,//�����д�������
	SEN_SEC,//һ���¼�
	SEN_32_SEC,//������¼�
	//SEN_IR_CAPTURE,//ir����ɹ�
	//SEN_IR_TIMOUT,
	//SEN_RF_CAPTURE,//rf����ɹ�	
	//SEN_RF_TIMOUT,
	//SEN_WRECV_DISTRIB_DEATH,//wnet recv �ַ����ڴ���
	SEN_SRV_BEAT,//������������������
	SEN_OVER_CLIENT,//�ߵ�����
	SEN_INFORM_TASK,//����inform task
	//SEN_CLEAN_LCD,//���lcd��ʾ
	//SEN_SIMU_STD_RF,//����ģ����Ƶ�źţ������Զ��������Ƶ��SimulateRecvStdRf
	SEN_CB_FUNC,//ִ�б�׼�ص�����
	SEN_SET_AP,//�첽�����µĽ���㣬esp8266����
	SEN_QCK_SAVE,//�첽��ʱ����������ݿ⣬esp8266����
	SEN_OLED_DISP,//oled��ʾˢ�£�esp8266����
	SEN_PIN_CHANGE,//pin�ı䣬esp8266����
	SEN_USER_EVT1,//�û��¼���esp8266����
	SEN_USER_EVT2,//�û��¼���esp8266����
	SEN_USER_EVT3,//�û��¼���esp8266����
	SEN_USER_EVT4,//�û��¼���esp8266����
	
	SEN_MAX	
}SYS_EVENT_NAME;

void SysEventTask(void *pvParameters);
void SysEventSend(SYS_EVENT_NAME Event,u32 Arg1,void *pArg2,bool *pYeild);
void SysEventMsSend(u32 Ms,u32 Event,u32 Arg1,void *pArg2,MS_FUNC_MODE Mode);
void SysEventMsDelete(u32 Event);

#endif




