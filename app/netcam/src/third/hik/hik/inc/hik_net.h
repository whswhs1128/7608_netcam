#ifndef _HIK_NET_H_
#define _HIK_NET_H_

#include "hik_netfun.h"

typedef struct
{
	int 				ClientSocket;
	struct sockaddr_in	ClientAddr;
} HIK_CLIENT_INFO;


int HikStart();

void *hik_Session_Thread(void *param);
int lib_hik_init(void *param);
int lib_hik_uninit();


#endif


