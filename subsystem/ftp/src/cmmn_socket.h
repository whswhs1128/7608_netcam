/*=====================================================================================

FILE NAME   : cmmn_socket.h
MODULE NAME : cmmon


GENERAL DESCRIPTION

this file is the cmmn_socket header all include write here

Nbpt Software
Copyright (c) 2008 by Nbpt Software. All Rights Reserved.

=======================================================================================
Revision History

Modification                  Tracking
Date           Author         Number            Description of changes
----------     ------------   ------------     ---------------------------------
2009-04-17     Jerry Chen     CRxxxxxxx         Create


=====================================================================================*/


#ifndef __TS_CMMN_SOCKET_H__
#define __TS_CMMN_SOCKET_H__

#define	PLUGIN_DEF

#ifndef __unix__
#define __unix__
#endif
//#define	VMS


/************************************************************************/
/* TSTester configure                                                        */
/************************************************************************/



/*******************************************************************************
* Include File Section
*******************************************************************************/
#if defined(__unix__) || defined(__VMS)
#include <unistd.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

#if defined(__unix__)
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#elif defined(VMS)
#include <types.h>
#include <socket.h>
#include <in.h>
#include <netdb.h>
#include <inet.h>
#elif defined(WIN32)
#include <windows.h>
#include <winsock.h>
#pragma comment (lib,"ws2_32.lib")
#endif

//#include "we_def.h"

#ifdef LWIP
#include "TSDialupAgent/TSDialupAgent.h"
#ifndef socklen_t
#define socklen_t WE_INT
#endif
#endif


#define ICMP_ECHO           8
#define ICMP_ECHOREPLY      0
#define ICMP_MIN            8         // minimum 8 byte icmp packet (just header)



#if defined(LWIP)

#define TSgethostbyname(a)      DaGetDnsIpAddress(a)
#define TSinet_addr(a)          DaInet_Addr(a)
#define TShtons(a)              DaHtons(a)
#define TShtonl(a)              DaHtonl(a)

#define TSinet_nota(a)          DaInet_Ntoa(a)
#define TSinet_aton(a,b)        DaInet_Aton(a,b)
#define TSntohs(a)              DaNtohs(a)
#define TSntohl(a)              DaNtohl(a)

#define TSaccept(a,b,c)         DaAccept(a,b,c)
#define TSbind(a,b,c)           DaBind(a,b,c)
#define TSshutdown(a,b)         DaShutDown(a,b)
#define TSclosesocket(s)        DaClose(s)
#define TSconnect(a,b,c)        DaConnect(a,b,c)
#define TSgetsockname(a,b,c)    DaGetSockName(a,b,c)
#define TSgetpeername(a,b,c)    DaGetPeerName(a,b,c)
#define TSsetsockopt(a,b,c,d,e) DaSetSockOpt(a,b,c,d,e)
#define TSgetsockopt(a,b,c,d,e) DaGetSockOpt(a,b,c,d,e)
#define TSlisten(a,b)           DaListen(a,b)
#define TSrecv(a,b,c,d)         DaRecv(a,b,c,d)
#define TSrecvfrom(a,b,c,d,e,f) DaRecvFrom(a,b,c,d,e,f)
#define TSsend(a,b,c,d)         DaSend(a,b,c,d)
#define TSsendto(a,b,c,d,e,f)   DaSendTo(a,b,c,d,e,f)
#define TSsocket(a,b,c)         DaSocket(a,b,c)
#define TSselect(a,b,c,d,e)     DaSelect(a,b,c,d,e)
#define TSioctlsocket(a,b,c)    DaIoctlSocket(a,b,c)
#define TSread(a,b,c)           DaRead(a,b,c)
#define TSwrite(a,b,c)          DaWrite(a,b,c)
#define TSclose(s)              DaClose(s)
#define TSSleep(ms)             DaSleep(ms)

#elif defined(WIN32)

#define TSgethostbyname(a)      sdk_net_gethostbyname(a)
#define TSinet_addr(a)          inet_addr(a)
#define TShtons(a)              htons(a)
#define TShtonl(a)              htonl(a)

#define TSinet_nota(a)          inet_nota(a)
#define TSinet_aton(a,b)        inet_aton(a,b)
#define TSntohs(a)              ntohs(a)
#define TSntohl(a)              ntohl(a)

#define TSaccept(a,b,c)         accept(a,b,c)
#define TSbind(a,b,c)           bind(a,b,c)
#define TSshutdown(a,b)         shutdown(a,b)
#define TSclosesocket(s)        closesocket(s)
#define TSconnect(a,b,c)        connect(a,b,c)
#define TSgetsockname(a,b,c)    getsockname(a,b,c)
#define TSgetpeername(a,b,c)    getpeername(a,b,c)
#define TSsetsockopt(a,b,c,d,e) setsockopt(a,b,c,d,e)
#define TSgetsockopt(a,b,c,d,e) getsockopt(a,b,c,d,e)
#define TSlisten(a,b)           listen(a,b)
#define TSrecv(a,b,c,d)         recv(a,b,c,d)
#define TSrecvfrom(a,b,c,d,e,f) recvfrom(a,b,c,d,e,f)
#define TSsend(a,b,c,d)         send(a,b,c,d)
#define TSsendto(a,b,c,d,e,f)   sendto(a,b,c,d,e,f)
#define TSsocket(a,b,c)         socket(a,b,c)
#define TSselect(a,b,c,d,e)     select(a,b,c,d,e)
#define TSioctlsocket(a,b,c)    ioctlsocket(a,b,c)
#define TSread(a,b,c)           read(a,b,c)
#define TSwrite(a,b,c)          write(a,b,c)
#define TSclose(s)              close(s)
#define TSSleep(ms)             Sleep(ms)

#elif defined(__unix__)
//#define TSgethostbyname(a)      getnameinfo(a)
//#define TSgethostbyname(a)      sdk_net_gethostbyname(a)
#define TSinet_addr(a)          inet_addr(a)
#define TShtons(a)              htons(a)
#define TShtonl(a)              htonl(a)

#define TSinet_nota(a)          inet_nota(a)
#define TSinet_aton(a,b)        inet_aton(a,b)
#define TSntohs(a)              ntohs(a)
#define TSntohl(a)              ntohl(a)

#define TSaccept(a,b,c)         accept(a,b,c)
#define TSbind(a,b,c)           bind(a,b,c)
#define TSshutdown(a,b)         shutdown(a,b)
#define TSclosesocket(s)        closesocket(s)
#define TSconnect(a,b,c)        connect(a,b,c)
#define TSgetsockname(a,b,c)    getsockname(a,b,c)
#define TSgetpeername(a,b,c)    getpeername(a,b,c)
#define TSsetsockopt(a,b,c,d,e) setsockopt(a,b,c,d,e)
#define TSgetsockopt(a,b,c,d,e) getsockopt(a,b,c,d,e)
#define TSlisten(a,b)           listen(a,b)
#define TSrecv(a,b,c,d)         recv(a,b,c,d)
#define TSrecvfrom(a,b,c,d,e,f) recvfrom(a,b,c,d,e,f)
#define TSsend(a,b,c,d)         send(a,b,c,d)
#define TSsendto(a,b,c,d,e,f)   sendto(a,b,c,d,e,f)
#define TSsocket(a,b,c)         socket(a,b,c)
#define TSselect(a,b,c,d,e)     select(a,b,c,d,e)
#define TSioctlsocket(a,b,c)    ioctlsocket(a,b,c)
#define TSread(a,b,c)           read(a,b,c)
#define TSwrite(a,b,c)          write(a,b,c)
#define TSclose(s)              close(s)
#define TSSleep(ms)             Sleep(ms)

#endif


#ifdef WIN32
#define SOCKETCLOSE(p) {
if((p)){
TSclosesocket((p));(p)=NULL;
}
}
#else
#define SOCKET int
#define WSAGetLastError() errno
#define GetLastError() errno
#define SOCKETCLOSE(p) { if((p)) { TSclose((p));(p)=NULL;} }
#endif



#endif //end __TS_CMMN_SOCKET_H__
