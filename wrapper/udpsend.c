#include <stdlib.h>
#include <stdint.h>
#ifdef _WIN32
	#include <winsock2.h>      // Needed for all Winsock stuff
#else
	#include <stdlib.h>
	#include <unistd.h>
	#include <errno.h>
	#include <sys/types.h>    // Needed for sockets stuff
	#include <netinet/in.h>   // Needed for sockets stuff
	#include <sys/socket.h>   // Needed for sockets stuff
	#include <arpa/inet.h>    // Needed for sockets stuff
	#include <fcntl.h>        // Needed for sockets stuff
	#include <netdb.h>        // Needed for sockets stuff
	typedef int SOCKET;
	#define closesocket close
#endif
#define EXPORT

static int g_udp_send_inited=0;

EXPORT intptr_t usCreateIPv4Socket(){
	SOCKET sk;
	#ifdef _WIN32
		if(!g_udp_send_inited){
			WORD wVersionRequested = MAKEWORD(1,1);       // Stuff for WSA functions
			WSADATA wsaData;                              // Stuff for WSA functions
			memset(&wsaData,0,sizeof(wsaData));
			WSAStartup(wVersionRequested, &wsaData);
			g_udp_send_inited=1;
		}
	#endif
	sk=socket(AF_INET,SOCK_DGRAM,0);
	if(sk>=0){
		#ifdef _WIN32
			unsigned long mode = 1;
			ioctlsocket(sk, FIONBIO, &mode);
		#else
			int flags = fcntl(sk, F_GETFL, 0);
			if(flags>=0){
				flags |= O_NONBLOCK;
				fcntl(sk, F_SETFL, flags);
			}
		#endif
	}
	return (intptr_t)sk;
}

EXPORT int usSendToIPv4(intptr_t sk,char* buf,size_t sz,int IP,int port){
	struct sockaddr_in   server_addr;
	int ret=0;
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);
	server_addr.sin_addr.s_addr = (unsigned long)IP;
	#ifndef MSG_NOSIGNAL
		#define MSG_NOSIGNAL 0
	#endif
	ret=sendto((SOCKET)sk, buf, sz, MSG_NOSIGNAL, (struct sockaddr *)&server_addr, sizeof(server_addr));
	if(ret<0){
		#ifdef _WIN32
			int err=WSAGetLastError();
			if(err==WSAENOTSOCK){
				return 0;
			}
		#else
			errno_t err=errno;
			if(err==ENOTSOCK||err==EPIPE){
				return 0;
			}
		#endif
	}
	return 1;
}

EXPORT void usCloseSocket(intptr_t sk){
	closesocket((SOCKET)sk);
}
