#ifndef SYS_EVENT_H
#define SYS_EVENT_H


typedef enum{
	PET_NULL=0,

	PET_NET_OK,//��������
	PET_NET_LOST,//����ʧ��
	
	PET_TCP_CLIENT_LOST,//ͬĳ��tcp�����ж���
	PET_TCP_SRV_LOST,//ͬ�����������ж���
	PET_TCP_JSON_LOST,//json���Ӷ���

	
	PET_APP_LOGIN,//app��½�ɹ�
	PET_APP_LOST,//app�������߱����뿪

	PET_APP_NEW_DEV,//app������һ���豸
	PET_APP_NEW_TRIG,//app������һ������Դ
	PET_APP_MODIFY_DEV,
	PET_APP_MODIFY_SCENE,
	PET_APP_DEL_DEV,//appɾ����һ���豸
	PET_APP_DEL_SCENE,
	PET_APP_DEL_VAR,
	PET_APP_DEL_TRIG,

	PET_SRV_RTC_CHANGE,//�����������˱���ʱ��	
	PET_DEV_UNBIND,//�豸���

	
	
}PUBLIC_EVENT;


void PublicEvent(PUBLIC_EVENT Event,u32 Param,void *p);


#endif

