/*!
*****************************************************************************
** FileName     : sdk_vision.h
**
** Description  : common api of vision.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Date         : 2015-9-9, create.
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _SDK_VISION_H_
#define _SDK_VISION_H_
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
#include "gk_bd.h"	//xqq

//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************
#define VISION_REGION_MAX   4
#define VISION_CLASS_NAME_LEN 32 	//xqq
#define VISION_MAX_CLASS 2	//xqq
#define VENC_STREAM3_WIDTH   640	//xqq
#define VENC_STREAM3_HEIGHT  360	//xqq



//*****************************************************************************
//*****************************************************************************
//** Enumerated types
//*****************************************************************************
//*****************************************************************************

//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************
typedef struct {
    uint8_t   enable;
    uint32_t  x;
    uint32_t  y;
    uint32_t  width;
    uint32_t  height;
    uint8_t   sensitivity;
    uint8_t   regionTime;
    uint8_t   regionRatio;
} GK_VISION_ALARM_RECT;

typedef struct {
    uint8_t   enable;
    uint32_t  startX;
    uint32_t  startY;
    uint32_t  endX;
    uint32_t  endY;
    uint8_t   direction;
} GK_VISION_ALARM_LINE;

/* 人形检测*/
typedef struct {
    uint8_t                   enable;
    GK_VISION_ALARM_RECT      rect[4]; //支持4个区域
} GK_VISION_ALARM_HD_CFG, *LPGK_VISION_ALARM_HD_CFG;

/* 运动追踪*/
typedef struct {
    uint8_t             enable;
    GK_VISION_ALARM_RECT      rect[4]; //支持4个区域
    uint8_t             retPoint;// 通过设置预置点实现
    uint32_t            retTime; // 返回时间
} GK_VISION_ALARM_MT_CFG, *LPGK_VISION_ALARM_MT_CFG;

typedef struct {
    //开始时间
    int32_t startHour;
    int32_t startMin;
    //结束时间
    int32_t stopHour;
    int32_t stopMin;
} ALARM_SCHEDTIME;

/* 越界侦测*/
typedef struct {
    uint8_t            enable;
    GK_VISION_ALARM_LINE      line[4]; //支持4条越界线
    int32_t            schedule_mode;     //1(默认值)按布防时间scheduleTime, scheduleSlice   0全时段检测
    ALARM_SCHEDTIME    scheduleTime[7][4];  /*该通道的布防时间*/
    uint32_t           scheduleSlice[7][3]; /*该通道的布防时间片段*/
} GK_VISION_ALARM_TD_CFG, *LPGK_VISION_ALARM_TD_CFG;

/* 区域入侵*/
typedef struct {
    uint8_t             enable;
    GK_VISION_ALARM_RECT      rect[4]; //支持4个区域
    int32_t            schedule_mode;     //1(默认值)按布防时间scheduleTime, scheduleSlice   0全时段检测
    ALARM_SCHEDTIME    scheduleTime[7][4];  /*该通道的布防时间*/
    uint32_t            scheduleSlice[7][3]; /*该通道的布防时间片段*/
} GK_VISION_ALARM_RI_CFG, *LPGK_VISION_ALARM_RI_CFG;

typedef struct {
    uint8_t*            addr;
    uint32_t            size;
} GK_VISION_BUFFER_INFO;

typedef struct
{
    GV_MD_Result            mdResult;
    GK_VISION_BUFFER_INFO   buffInfo;
} GK_VISION_MD_RESULT;

typedef struct
{
    int x;
    int y;
    int width;
    int height;
}GK_VISION_RECT;

//xqq add
typedef struct{
        GK_VISION_RECT rect;
        float quality;
}GK_VISION_OBJ;

typedef struct{
    char class_name[VISION_CLASS_NAME_LEN];
    uint32_t rect_num;
    uint32_t rect_capcity;
    GK_VISION_OBJ *objs;
}GK_VISION_ONE_CLASS;

typedef struct{
    int32_t class_num;
    GK_VISION_ONE_CLASS obj_class[VISION_MAX_CLASS];
}GK_VISION_OBJ_ARRAY;
//xqq add end


/* Pointer to the vision detection result handler function */
typedef void (*GK_VISION_NOTIFY)(GK_VISION_MD_RESULT* result);

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

int sdk_vision_init(int threshold, int useGmd, GK_VISION_NOTIFY notifyFunc);
int sdk_vision_exit(void);

int sdk_vision_set_pause_state(int state);
int sdk_vision_get_pause_state(void);

int sdk_vision_set_vision_sensitivity(int sensitivity);
int sdk_vision_set_human_detection(GK_VISION_ALARM_HD_CFG *config);
int sdk_vision_set_transboundary_detection(GK_VISION_ALARM_TD_CFG *config);
int sdk_vision_set_region_intrusion(GK_VISION_ALARM_RI_CFG *config);
int sdk_vision_set_roi(int idx, GK_VISION_RECT rect, int minSize);

#ifdef __cplusplus
}
#endif
#endif //SDK_VISION_H_
