/*!
*****************************************************************************
** FileName     : gk_cms_common.h
**
** Description  : common api of cms.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Date         : 2015-9-9, create.
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_CMS_COMMON_H_
#define _GK_CMS_COMMON_H_
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
#include <net/if.h>
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
#include <sys/ioctl.h>
#include <dirent.h>
#include <linux/unistd.h>
#include <errno.h>
#include <sys/syscall.h>

#include "utility_api.h"

#define GK_CMS_BUFLEN 4096

#define GK_CMS_BC_RCV_PORT 0x9888
#define GK_CMS_BC_SEND_PORT 0x8888
#define GK_CMS_TCP_LISTEN_PORT 0x9123
#define	SERVER_PACK_FLAG 0x03404324
#define GK_CMS_MC_GROUP_IP "224.188.188.188"
#define GK_CMS_BC_IP "255.255.255.255"


#define GK_STREAM_DATA_SEND_BUF_SIZE (1*1024*1024)
#define GK_PACKET_SEND_LEN_ONCE (8*1024)
#define GK_MAX_LINK_NUM  20

#define GK_CMS_UDP_PORT  34568

#define GK_MSG_RECV_BUF_SIZE (16*1024)
#define GK_MSG_SEND_BUF_SIZE (16*1024)

typedef enum{
    UNINIT_SESSION = 0,
    CMD_SESSION,
    CONNECT_SESSION,
} SESSION_TYPE;

typedef struct tagGkSessionCtrl{
	//int flag;
	int fSessionInt;
	char fSessionId[16];
	int accept_sock;
	//GkLvSocket lv_sock;
	//int lv_sub_stream_sock;
	int fPeerSeqNum;
	int fSequenceNum;
	char fSendbuf[GK_MSG_SEND_BUF_SIZE];
	SESSION_TYPE fSessionType;
} GkSessionCtrl;

typedef struct tagGkTimetick
{
	int second            :6;  // 秒      1-60
	int minute            :6;  // 分      1-60
	int hour              :5;  // 时      1-24
	int day               :5;  // 日      1-31
	int month             :4;  // 月      1-12
	int year              :6;  // 年      2000为基准，0-63
} GkTimetick;



#ifdef __cplusplus
}
#endif
#endif
