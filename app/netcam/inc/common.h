/*!
*****************************************************************************
** FileName     : common.h
**
** Description  : common api of all module.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-7-31
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_COMMON_H__
#define _GK_COMMON_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <libgen.h>
#include <net/route.h>

#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <ifaddrs.h>
#include <pthread.h>
#include <errno.h>
#include <signal.h>
#include <time.h>

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
#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
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
#include "sdk_def.h"



//提示信息可根据需要是否打印

#define GK_COMMON_DEBUG 1
#define GK_NORMAL_DEBUG 1
#define APPSERVER_ERROR 1
#if GK_COMMON_DEBUG
#define PRINT_INFO(fmt, args...)  \
    do{\
        printf("[APP INFO] [ %s, Line: %d ]  " fmt "\n", __FILE__, __LINE__,  ##args); \
    }while(0)

#elif GK_NORMAL_DEBUG
#define PRINT_INFO(fmt, args...)        printf("[I-%s]"fmt, __FILE__, ##args)

#else
#define PRINT_INFO(fmt, args...)

#endif

#if APPSERVER_ERROR
#define PRINT_ERR(fmt, args...)  \
    do{\
        printf("\033[0;31m[APP ERR] [ %s, Line:%d ]  " fmt "\033[0;0m\n", __FILE__, __LINE__,  ##args); \
    }while(0)

#elif GK_NORMAL_DEBUG
#define PRINT_ERR(fmt, args...)     printf("\033[0;31m[E-%s]"fmt"\033[0;0m\n", __FILE__, ##args)

#else
#define PRINT_ERR(fmt, args...) {}

#endif

#ifdef __cplusplus
}
#endif
#endif
