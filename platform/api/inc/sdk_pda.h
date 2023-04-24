/*!
*****************************************************************************
** \file      GK-sdk_pda.h
**
** \version	$id: sdk_pda.h 19-05-16 17:47:31
**
** \brief
**
** \attention   this code is provided as is. goke microelectronics
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2019-2020 by goke microelectronics co.,ltd
*****************************************************************************
*/
#ifndef __SDK_PDA_H__
#define __SDK_PDA_H__

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "sdk_cfg.h"

#if 1
#define CHECK_CONTENT(cond, ret, msg, ...)  \
    if (cond) { \
        LOG_INFO(msg, ##__VA_ARGS__); \
            return ret; \
    }
#else
#define CHECK_CONTENT(cond, ret, msg, ...)  \
    if (cond) { \
        printf("[%s L%d] " msg, __FUNCTION__, __LINE__, ##__VA_ARGS__); \
            return ret; \
    }
#endif

#define SDK_PDA_RECT_NUM    20

typedef struct sdk_pda_attr{
    unsigned int width;
    unsigned height;
}SDK_PDA_ATTR_T;

typedef struct sdk_pda_data{
    void *pAddr;
    unsigned int totalNumByte;
    int stride;
    unsigned int MbWidth;
    unsigned int MbHeight;
}SDK_PDA_DATA_T;


/*format of caputre frame*/
typedef enum
{
    SDK_FORMAT_YUV_SEMIPLANAR_420,/*Y is planer,U V is interleaved*/
    SDK_FORMAT_YUV_SEMIPLANAR_422,/*Y is planer,U V is interleaved*/
}SDK_PDA_Frame_Format;

typedef struct sdk_pm_yuv_attr{
    SDK_PDA_Frame_Format frameFormat; /*format of caputre frame*/
    unsigned int    width;                 /*width of caputre frame*/
    unsigned int    height;                /*height of caputre frame*/
    unsigned char    *yAddr;                /*Y address of caputre frame*/
    unsigned char    *uvAddr;               /*UV address of caputre frame, the UV data is interleaved*/
    unsigned int    stride;                /*stride of caputre frame*/

}SDK_PDA_YUV_ATTR_T;
//xqq add
typedef struct sdk_pda_point
{
    int s32X;
    int s32Y;
} SDK_PDA_POINT_S;

typedef struct sdk_pda_rect
{
    SDK_PDA_POINT_S astPoint[4];
} SDK_PDA_RECT_S;

typedef struct sdk_pda_rect_array
{
    unsigned short u16Num;
    SDK_PDA_RECT_S astRect[SDK_PDA_RECT_NUM];
} SDK_PDA_RECT_ARRAY_S;
//xqq add end

typedef void (*GK_PDA_MD_CALLBACK)(int vin, int target_num);

/*!
******************************************
** Description   @
** Param[in]     @ NULL
** Param[out]    @ NULL
**
** Return        @ 0:success, -1:fail
******************************************
*/
extern int sdk_pda_init(void);
/*!
******************************************
** Description   @
** Param[in]     @ NULL
** Param[out]    @ NULL
**
** Return        @ 0:success, -1:fail
******************************************
*/
extern int sdk_pda_exit(void);

/*!
******************************************
** Description   @
** Param[in]     @ vin: input sensor id
** Param[out]    @ NULL
**
** Return        @ 0:success, -1:fail
******************************************
*/
extern int sdk_pda_create(int vin);

/*!
******************************************
** Description   @
** Param[in]     @ vin: input sensor id
** Param[out]    @ NULL
**
** Return        @ 0:success, -1:fail
******************************************
*/
extern int sdk_pda_destroy(int vin);

/*!
******************************************
** Description   @
** Param[in]     @ vin: input sensor id
                   attr: pda attributes strcut addr
** Param[out]    @ NULL
**
** Return        @ 0:success, -1:fail
******************************************
*/
extern int sdk_pda_set_attr(int vin, SDK_PDA_ATTR_T *attr);

/*!
******************************************
** Description   @
** Param[in]     @ NULL
** Param[out]    @ attr: pda attributes strcut addr
**
** Return        @ 0:success, -1:fail
******************************************
*/
extern int sdk_pda_get_attr(int vin, SDK_PDA_ATTR_T *attr);

/*!
******************************************
** Description   @
** Param[in]     @ vin: input sensor id
                   falg 0:disalbe 1:enable
** Param[out]    @ NULL
**
** Return        @ 0:success, -1:fail
******************************************
*/
extern int sdk_pda_enable(int vin, bool falg);

/*!
******************************************
** Description   @
** Param[in]     @ vin: input sensor id
                   size:data buffer size
** Param[out]    @ data:pda data
                   size:pda data size
**
** Return        @ 0:success, -1:fail
******************************************
*/
extern int sdk_pda_get_data(int vin, SDK_PDA_DATA_T *pdaData);
extern int sdk_pda_get_yuv(int vin, int bufferType, SDK_PDA_YUV_ATTR_T *yuvAttr);
extern void* sdk_pda_get_pdaHandle(int vin);
extern int sdk_pda_get_yuv_last(SDK_PDA_YUV_ATTR_T *yuvAttr);
#endif
