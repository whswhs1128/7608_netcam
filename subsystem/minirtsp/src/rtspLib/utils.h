/*!
*****************************************************************************
** \file      $gkprjutils.h
**
** \version	$id: utils.h 15-08-04  8月:08:1438655303 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/

#ifndef __RTSPUTIL_H__
#define __RTSPUTIL_H__

#include <stdlib.h>
#include "rtspServ.h"

#define msleep(x) usleep((x) * 1000)

#define RTSP_ERR(fmt, args...) do {                                      \
        printf("\033[0;31m[RTSP ERR] [%s:%d] "fmt"\033[0;0m\n", \
            __FILE__, __LINE__,  ##args);                                \
    } while(0)

#define RTSP_INFO(fmt, args...) do {                   \
        printf("[RTSP INFO] [%s:%d] "fmt"\n", \
            __FILE__, __LINE__,  ##args);              \
    } while(0)

#define RTSP_WARNING(fmt, args...) do {                                      \
        printf("\033[0;33m[RTSP WARNING] [%s:%d] "fmt"\033[0;0m\n", \
            __FILE__, __LINE__,  ##args);                                \
    } while(0)

#define max(x, y) ((x) > (y) ? (x) : (y))
#define min(x, y) ((x) < (y) ? (x) : (y))

#define NELEMS(a) (sizeof(a) / sizeof((a)[0]))

ssize_t rtsp_read(int fd, void *buf, size_t count);
ssize_t rtsp_write(int fd, const void *buf, size_t count);

ssize_t rtsp_readn(int fd, void *buf, size_t count);
ssize_t rtsp_writen(int fd, const void *buf, size_t count);

long long rtsp_random64(void);
char *rtsp_state_message(unsigned int code);
int rtsp_send_reply(rtsp_session_t *sessp, char *buf, int size);
int rtsp_reply_error(rtsp_session_t *sessp, int code);
char *rtsp_encodeBase64(char *buf, int buf_len,
                        const unsigned char *src, int src_len);

int socket_connect(int fd, in_addr_t s_addr, uint16_t port);

int parse_time(int64_t *timeval, const char *timestr, int duration);

#endif
