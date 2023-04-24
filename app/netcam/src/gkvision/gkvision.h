/*!
*****************************************************************************
** FileName     : gkvision.h
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

#ifndef _GKVISION_H_
#define _GKVISION_H_
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

//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************


//*****************************************************************************
//*****************************************************************************
//** Enumerated types
//*****************************************************************************
//*****************************************************************************
typedef enum
{
    GK_VISION_REGION_HUMAN_DETECTION,
	GK_VISION_REGION_MOTION_TRACING,
	GK_VISION_REGION_REGION_INTRUSION
}GK_VISION_REGION_TypeEnumT;

//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************
typedef struct {
    int leftRightKp;    //x轴，左右电机的Kp系数
    int upDownKp;       //y轴，上下电机的Kp系数
    int speed;
} GK_VISION_PTZ_CfgT;

typedef struct {
    /* ptz config for motion tracing */
    GK_VISION_PTZ_CfgT ptzCfg;
    int debugEnable;
    int osdEnable;
    int imgUpload;
} GK_VISION_HandleT;

//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************

int gk_vision_init(GK_VISION_HandleT *params);
int gk_vision_exit(void);

int gk_vision_set_human_detection_state(int enable);
int gk_vision_set_motion_tracing(GK_ALARM_MT_CFG *config);
int gk_vision_fresh_rect(void);
int gk_vision_set_sensitive(int sensitive);

#ifdef __cplusplus
}
#endif
#endif
