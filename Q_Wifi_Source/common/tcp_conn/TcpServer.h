#ifndef TCP_SERVER_H
#define TCP_SERVER_H

void TcpServer_Init(void);
void DeleteTcpClientConn(void *pDelete);
u8 GetTcpClientsNum(void);

#endif
