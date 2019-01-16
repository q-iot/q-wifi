#ifndef DEBUG_OUTPUT_H
#define DEBUG_OUTPUT_H

//��ͨ��ӡ��cmd������Ŵ���1
//�û����ڷŵ�����0
#define GenPrint my_print1 //����ģʽ0������ģʽ1
#define UserPrint my_print0 //����ģʽ0
#define CmdPrint my_print1 //����ģʽ0������ģʽ1

//#define debug(format,...) printf("File: "__FILE__", Line: %05d: "format"/n", __LINE__, ##__VA_ARGS__)  
#define Debug(format,...) GenPrint(format,##__VA_ARGS__)  
#define DebugCol(format,...) {GenPrint("\033[7m");GenPrint(format,##__VA_ARGS__);GenPrint("\033[0m");}
#define CDebug(format,...) CmdPrint(format,##__VA_ARGS__)	//�ַ�����������
#define CDebugCol(format,...) {CmdPrint("\033[7m");CmdPrint(format,##__VA_ARGS__);CmdPrint("\033[0m");}
#define UDebug(format,...) UserPrint(format,##__VA_ARGS__)  //�û�ָ��ķ���



#endif

