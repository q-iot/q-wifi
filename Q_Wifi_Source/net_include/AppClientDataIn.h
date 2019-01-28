#ifndef APP_CLIENT_DATA_IN_H
#define APP_CLIENT_DATA_IN_H

PKT_HANDLER_RES AppClientPktHandler(u32 AppClientID,GLOBA_PKT_HEADER *pPkt);
PKT_HANDLER_RES BuildClientPktReply(u16 SecretKey,GLOBA_PKT_HEADER *pPkt,PKT_HANDLER_RES HandlerRes);
PKT_HANDLER_RES BuildUpdatePkt(u32 AppClientID,u16 SecretKey,UPDATE_PKT *pPkt);
PKT_HANDLER_RES AppClientDataInHandler(const void *pClient,u8 *pData,u16 DataLen);


#endif

