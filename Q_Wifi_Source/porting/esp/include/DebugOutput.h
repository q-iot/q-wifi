#ifndef DEBUG_OUTPUT_H
#define DEBUG_OUTPUT_H

//普通打印和cmd输出都放串口1
//用户串口放到串口0
#define GenPrint my_print1 //调试模式0，正常模式1
#define UserPrint my_print0 //正常模式0
#define CmdPrint my_print1 //调试模式0，正常模式1

//#define debug(format,...) printf("File: "__FILE__", Line: %05d: "format"/n", __LINE__, ##__VA_ARGS__)  
#define Debug(format,...) GenPrint(format,##__VA_ARGS__)  
#define DebugCol(format,...) {GenPrint("\033[7m");GenPrint(format,##__VA_ARGS__);GenPrint("\033[0m");}
#define CDebug(format,...) CmdPrint(format,##__VA_ARGS__)	//字符串命令的输出
#define CDebugCol(format,...) {CmdPrint("\033[7m");CmdPrint(format,##__VA_ARGS__);CmdPrint("\033[0m");}
#define UDebug(format,...) UserPrint(format,##__VA_ARGS__)  //用户指令的返回



#endif

