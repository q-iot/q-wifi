#ifndef Q_HW_CONFIG_H
#define Q_HW_CONFIG_H

enum{
	WIM_NULL=0,
		
	WIM_KEY,//����ģʽ�����з����Ŵ���ֻ��Ӧ�½��أ�������Ӧʱ������ƥ�䡢����ƥ�䡢ȡ��������������������Ӧʱ��Ϊ��ʱ200ms���ʺϽӰ�����
	WIM_IPT,//����

	WIM_OUT,//������������ݰ󶨵ı�����ƥ�������������ĳbit�����仯ʱ������ƥ�䡢����ƥ�䡢ȡ����
	WIM_OUT_OD,//��©���

	WIM_AIN,//ģ�������룬��������ͬ�ϱ�������ͬ�����ϱ����ڵ���ʱ�������δ�䣬���ϱ�
	WIM_PWM,//pwm���
	WIM_UART,//�û�ͨѶ��
	WIM_OLED,//oled��i2c���
	
	WIM_MAX
};
typedef u8 WE_IO_MODE;

enum{
	AIM_NULL=0,
	AIM_MATCH,//ƥ�䡣���ʱ��������ӦbitΪ1�����1��bitΪ0�����0������ʱ��ioΪ1��bit��Ϊ1��ioΪ0��bit��Ϊ0��
	AIM_REVERSE,//����ƥ�䡣���ʱ��bitΪ1�����0��bitΪ0�����1������ʱ��ioΪ1��bit��Ϊ0��ioΪ0��bit��Ϊ1��
	AIM_NEGATION,//ȡ��������ʱ��io���жϣ���bitȡ��
	AIM_COUNT,//����������ʱ��io���жϣ��������1

	AIM_MAX,
};
typedef u8 VAR_IO_MATCH;

typedef struct{
	WE_IO_MODE IoMode:4;//ioģʽ
	bool Pullup:1;//�Ƿ���������
	VAR_IO_MATCH VarIoMth:3;//io������Ĺ�ϵ
	u8 Bit;//����λ��0-15
	u16 VarID;//����ID��0��ʾ��ӳ��
}WE_IO_CONFIG;

typedef struct{
	u8 AdcMode;//ʹ��adcʱ����ֵ��1
	u8 Tolerance;//�ݲ1-99%������ֵ����Ľ�������ݲ�����Ÿı�
	u16 Factor;//���ӣ�����ֵ���Դ�ֵ���ټ���ƫ��Ŵ洢����������ֵΪʵ��ֵ��100����������Ϊ0
	s16 Offset;//ƫ��
	u16 VarID;//�󶨵ı�������
	u16 PeriodIdx;//��������
}WE_AIN_CONFIG;

enum{
	WPW_NULL=0,//��ʹ��
	WPW_FIXED,//�̶�ʽ��������bit���ƿ���
	WPW_PERIOD,//����ɵ�ʽ����������Cntֵ
	WPW_FACTOR,//ռ�ձȿɵ�ʽ����������PluseCntֵ
};
typedef u8 WE_PWM_TYPE;

typedef struct{
	WE_PWM_TYPE PwmMode;
	u8 a;
	u16 Cnt;//��������
	u16 PluseCnt;//����ռ�ձ�
	u16 uS_Base;
}WE_PWM_CONFIG;




#endif
