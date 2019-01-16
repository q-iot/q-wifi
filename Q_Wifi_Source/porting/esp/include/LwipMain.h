#ifndef __LWIP_MAIN_H
#define __LWIP_MAIN_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "lwip/ip_addr.h"
#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/dns.h"
#include "lwip/api.h"
#include "lwip/api_msg.h"
#include "lwip/tcpip.h"

//#include "ethernetif.h"

void LwIP_CommInit(void);
void LwIP_ChangeIP(u8 ip1,u8 ip2,u8 ip3,u8 ip4);
void Lwip_GetNetInfo(u8 *pIp);
u32 LwIP_GetNetIP(void);






#endif













