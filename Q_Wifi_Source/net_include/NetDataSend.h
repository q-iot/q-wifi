#ifndef NET_DATA_SEND_H
#define NET_DATA_SEND_H

//----------------------------lwip struct typedef--------------------------
typedef err_t NET_ERR_T;
typedef ip_addr_t  IP_ADDR; 
typedef struct netbuf NET_BUF_T;
typedef struct netconn NET_CONN_T;

NET_ERR_T SendToSrv(void *pData,u16 DataLen);
NET_ERR_T SendToOnlyConn(void *pData,u16 DataLen);
NET_ERR_T SendToAppConn(const void *pConn,void *pData,u16 DataLen);
NET_ERR_T SendToJsonSrv(void *pData,u16 DataLen);


#endif
