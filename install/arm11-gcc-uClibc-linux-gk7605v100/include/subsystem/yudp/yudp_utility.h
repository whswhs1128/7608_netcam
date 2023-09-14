
/*
 * yudp_utility api define.
 *
 * Author : Heyong
 * Date   : 2015.11.18
 * Copyright (c) 2015 Harric He(Heyong)
 *
 */
#ifndef _YUDP_UTILITY_H__
#define _YUDP_UTILITY_H__


#define Y_LOG_OPEN 1


#ifdef _WIN32 /* WIN32 */

#include <string>/* for erron EINTR & EAGAIN */

#include "GKLoger.h"

#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

#include <winsock2.h>

#define Y_ERROR LOG_ERROR
#define Y_INFO LOG_INFO

#define Y_SLEEP_MS(ms) Sleep((ms))

typedef DWORD y_time_t;
typedef int pthread_t;
typedef int socklen_t;


#else /* LINUX */

#include <stdio.h>
#include <stdlib.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>
#include <signal.h>
#include <sys/syscall.h>

#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>

#include <pthread.h>

#define Y_ERROR(FM, ...) if(1){printf((FM), ##__VA_ARGS__);}
#define Y_INFO(FM, ...) if(Y_LOG_OPEN) {printf((FM), ##__VA_ARGS__);}

#define Y_SLEEP_MS(ms) usleep((ms)*1000)

typedef int64_t y_time_t;


#endif/* endif _WIN32 */


#define Y_SFREE(PTR) if((PTR)){free((PTR));(PTR)=NULL;}//safe free.
#define Y_SMALLOC(PTR,SIZE,TYPE) (PTR)=(TYPE *)malloc(SIZE);if((PTR) == NULL){Y_ERROR("Y_SMALLOC error %d",(SIZE));abort();}//safe malloc.

void y_socketclose(int sock);

#ifdef _WIN32
#else
#endif

#ifdef _WIN32 /* WIN32 */
#else /* LINUX */
#endif/* endif _WIN32 */



#endif /* _YUDP_UTILITY_H__ */
