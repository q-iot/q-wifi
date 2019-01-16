#ifndef USER_HOOK_H
#define USER_HOOK_H

void UserHwDefConfigHook(HW_CONF_STRUCT *pHwConf);
void UserHwConfigInitHook(u8 Pin,WE_IO_CONFIG *pIoConf);
void UserHwInitHook(void);
void UserAppInitHook(void);
void UserConnHook(void);
void UserDisconnHook(void);
void UserKeyStateHook(u8 Pin,u16 PressMs);
void UserIoStateHook(u8 Pin,u8 PinVal);
void UserVarChangeHook(u16 VarID,TVAR32 VarValue);
void UserAppButtonHook(u32 DevID,u16 ButtonID);
void UserSysMsgHook(const u8 *pMsg);

void UserUart0ByteInHook(u8 Byte);
void UserUart0IdleHook(void);



#endif

