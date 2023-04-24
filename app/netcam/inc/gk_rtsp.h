/*!
*****************************************************************************
** \file      $gkprjinc/gk_rtsp.h
**
** \version	$id: inc/gk_rtsp.h 15-08-04  8鏈?:08:1438664577
**
** \brief
**
** \attention   this code is provided as is. goke microelectronics
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#ifndef GK_RTSP_H_
#define GK_RTSP_H_
#include "rtspServLib.h"

#define MAX_VIDEO_ENC_CHN   3

typedef struct video_param{
    int max_ch;//max video channle number
    int enc_type[MAX_VIDEO_ENC_CHN];
    int buffer_id[MAX_VIDEO_ENC_CHN];//video stream buffer id default 0-2
    int fps[MAX_VIDEO_ENC_CHN];
    int width[MAX_VIDEO_ENC_CHN];
    int height[MAX_VIDEO_ENC_CHN];
    char rtsp_route[MAX_VIDEO_ENC_CHN][512];
}VIDEO_PARAM_S;

typedef struct audio_patam{
    int enable;// 0 disable; 1 input; 2- input&output
    int enc_type;//audio enc type
    int samplerate;//audio samplerate
    int samplewidth;//audio samplewidth
    int channle_num;
}AUDIO_PATAM_S;

typedef struct rtsp_param{
    VIDEO_PARAM_S video;
    AUDIO_PATAM_S audio;
    int authenticate;
}RTSP_PARAM_S;



int rtsp_getParam(RTSP_PARAM_S *param);
int rtsp_setParam(RTSP_PARAM_S *param);

/**********************************************************************
函数描述：开启 rtsp server
入口参数：无
返回值：  0: 成功
          非0: 失败
**********************************************************************/
int rtsp_start(void);
/**********************************************************************
函数描述：关闭 rtsp server
入口参数：无
返回值：  无
**********************************************************************/
void rtsp_stop(void);

#endif


