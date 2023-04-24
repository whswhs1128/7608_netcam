#ifndef __LIB_HIK_SESSION_H__
#define __LIB_HIK_SESSION_H__


#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>

#include <sys/types.h>

#include <netinet/tcp.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <semaphore.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <linux/unistd.h>
#include <errno.h>
#include <sys/syscall.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <net/route.h>
#include <sys/time.h>


//extern int errno;
//extern int g_xmai_handle;

int lib_hik_start();
int lib_hik_stop();

#endif

