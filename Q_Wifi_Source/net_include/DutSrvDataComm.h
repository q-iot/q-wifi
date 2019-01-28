#ifndef DUT_SRV_DATA_COMMT_H
#define DUT_SRV_DATA_COMMT_H

void SrvConnSendBeat(bool IsBeat);
void SrvConnSendMsg(const char *pMsg,const char *pUser,u8 MsgFlag);
bool SrvConnSendMainPkt(GLOBA_PKT_HEADER *pPkt);
PKT_HANDLER_RES ServerDataInHandler(const void *pSrv,u8 *pData,u16 DataLen);




#endif

