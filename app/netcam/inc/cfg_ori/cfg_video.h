/*!
*****************************************************************************
** FileName     : cfg_video.h
**
** Description  : config for video in, video out, video enc.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-7-29
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_CFG_VIDEO_H__
#define _GK_CFG_VIDEO_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

#define MAX_SENSOR_NUM      2
#define MAX_SENSOR_ITEM_NUM 7
#define MAX_VOUT_NUM        2
#define MAX_VOUT_ITEM_NUM   4
#define MAX_VENC_STREAM_NUM 4
#define MAX_VENC_ITEM_NUM   25

#if 1
typedef struct {
    SDK_U32 width;
    SDK_U32 height;
    /*input video frame rate.*/
    SDK_U32 fps;
    SDK_U32 pi;
    /*enum GK_VI_MirrorPatternEnumT pattern*/
    SDK_U8  mirrorPattern;
    /*enum SDK_VI_BayerPatternEnumT bayer_pattern*/
    SDK_U8  bayerPattern;
} GK_NET_VI_CFG;

typedef struct {
    /*specify vout channel.*/
    SDK_U8 voutChannel;
    /*video output resolution mode,*/
    SDK_U8 resoluMode;
    /*video output device type.*/
    SDK_U8 deviceType;
} GK_NET_VOUT_CFG;
#endif

/* video encoding property */
typedef struct  {
	char name[32];  /*stream name*/
	int  width;     /*stream width*/
    int  height;    /*stream height*/
	int  fps;       /*encoding frame rate*/
    int  gop;       /*the interval of key I frame*/
	int  profile;   /* encoding level*/
	int  rc_mode;   /* rate mode,0 cbr, 1 vbr*/
	int  bps;       /* bitrate value, unit:kbps*/
	int  quality;   /* quality mode,0-3, 3 is best,0 is lowest*/
    int  re_rec_mode;
//	int  stream_switch; /* 0 send audio and video, 1 : only send video , 2: not send audio and video */
}GK_ENC_STREAM_H264_ATTR;

typedef struct
{
    /*stream index.*/
    SDK_U32            streamId;
    /*rate control factor. value:1~51, qpMinOnI <= qpMinOnP.*/
    SDK_U8             qpMinOnI;
    /*rate control factor.qpMinOnI <= qpMaxOnI*/
    SDK_U8             qpMaxOnI;
    /*rate control factor. value:1~51, qpMinOnI <= qpMinOnP.*/
    SDK_U8             qpMinOnP;
    /*rate control factor.qpMinOnP <=  qpMaxOnP*/
    SDK_U8             qpMaxOnP;
    /*rate control factor: I frame qp weight, range: 1~10,*/
    SDK_U8             qpIWeight;
    /*rate control factor. P frame qp weight, range: 1~5.*/
    SDK_U8             qpPWeight;
    /*picture quality consistency, range: 0~2. 2:will be best.*/
    SDK_U8             adaptQp;
}GK_ENC_H264QP_ATTR;


typedef struct {
    SDK_S32 id;         /*编码id，从0开始*/
    SDK_S32 enable;     /*1,频道使能，0，频道关闭*/
    SDK_S32 avStream;   /*音视频编码输出控制，0-音视频输出，1-视频输出*/
    SDK_S32 enctype;     /*0: none, 1: H.264, 2: MJPEG, 3: H.265*/
    SDK_U8  resolution[MAX_STR_LEN_128]; //用于存放当前分辨率和可选分辨率，格式(长*宽)，1920*1080
    GK_ENC_STREAM_H264_ATTR h264Conf;
    GK_ENC_H264QP_ATTR h264QpConf;
} GK_NET_VENC_STREAM;

typedef struct {
    GK_NET_VI_CFG vi[MAX_SENSOR_NUM];
    GK_NET_VOUT_CFG vout[MAX_VOUT_NUM];
    GK_NET_VENC_STREAM vencStream[MAX_VENC_STREAM_NUM];
} GK_NET_VIDEO_CFG;


extern int VideoCfgSave();
extern int VideoCfgLoad();
extern void VideoCfgPrint();
extern int VideoCfgLoadDefValue();
extern char* videoCfgLoadStreamJson( int streamId);

#define VIDEO_CFG_FILE "gk_video_cfg.cjson"

extern GK_NET_VIDEO_CFG runVideoCfg;
extern GK_CFG_MAP videoMap[MAX_VENC_STREAM_NUM][MAX_VENC_ITEM_NUM];

#ifdef __cplusplus
}
#endif
#endif

