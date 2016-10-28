#ifndef __UDPSEND_H
#define __UDPSEND_H
#include <stdint.h>

intptr_t usCreateIPv4Socket();
int usSendToIPv4(intptr_t sk,char* buf,size_t sz,int IP,int port);
void usCloseSocket(intptr_t sk);
#endif
