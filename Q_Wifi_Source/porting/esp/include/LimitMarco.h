#ifndef LIMIT_MARCO_H
#define LIMIT_MARCO_H

#define PKT_MAX_DATA_LEN 1442 //TCP包数据段最大长度
#define LONG_STR_BYTES 64 //较长字符串长度
#define NORMAL_STR_BYTES 32 //一般字符串长度
#define SHORT_STR_BYTES 16 //短字符串长度
#define STR_RECORD_BYTES 100

#define KEYS_SET_MAX_KEY_NUM 32 //能存储的最大按键个数
#define SCENE_MAX_ITEM_NUM 30//每个情景模式支持的最大动作数
#define SCENE_MAX_NUM 256//允许的情景模式个数
#define PHONE_NUM_LEN 16//电话号码长度
#define MSG_TXT_LEN 160//短信长度
#define ALARM_MAX_NUM 32 //闹钟数
#define COUNT_DOWN_MAX_NUM 32//最多倒计时任务个数

#define WDEV_GROUP_MAN_NUM 32//可以管理的wdev组别数目

#define SELF_VAR_TOTAL 64//每个设备最多拥有64个变量
#define SELF_VAR_MAX 8//每个设备最多拥有8个可配置自身变量
#define RELATE_VAR_MAX 4//每个设备可以关注4个变量
#define WDEV_PARAM_NUM 4//万纳网设备参数个数
#define WDEV_DIR_COMM_NUM 8//设备直接通信的个数

#define WAVER_IO_NUM 8
#define WAVER_AIO_NUM 4
#define WAVER_PWM_NUM 2

#define USER_CLIENT_MAX_NUM 6//支持最大6个客户端

#define F_LCD_VAR_DISP_NUM 4//液晶屏显示4个变量

//json上报变量最大数
#define JSON_UP_VARS_MAX 16 //最多处理变量个数


#endif
