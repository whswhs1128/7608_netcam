/*!
*****************************************************************************
** FileName     : cfg_video.c
**
** Description  : video config api.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-8-1
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include "cfg_video.h"
#include "cfg_channel.h"
#include "cfg_common.h"
#include "utility_api.h"
#include "sdk_def.h"
extern int copy_file(char *src_name, char *des_name);


#define RESOLUTION_720P//when cjson confg can't be used, max resolution is 720P.

GK_NET_VIDEO_CFG runVideoCfg;
/*
1080P

960P
720P
*/
/*!
*******************************************************************************
** \brief (brief description)
**
** (addition description is here)
**
** \param[in]  parameter_name   (parameter description)
** \param[out] parameter_name   (parameter description)
**
**
** \return
** - #RETURN_VALUE_A (return value description)
** - #RETURN_VALUE_B (return value description)
**
** \sa
** - see_also_symbol
*******************************************************************************
*/
#if USE_DEFAULT_PARAM
GK_CFG_MAP videoMap[MAX_VENC_STREAM_NUM][MAX_VENC_ITEM_NUM] = {
    {
        {"id",                 &(runVideoCfg.vencStream[0].id),                      GK_CFG_DATA_TYPE_S32, "0",       "rw", 0, 3, "0-3"},
        {"enable",             &(runVideoCfg.vencStream[0].enable),                  GK_CFG_DATA_TYPE_S32, "1",       "rw", 0, 1, NULL},
        {"avStream",		   &(runVideoCfg.vencStream[0].avStream),				 GK_CFG_DATA_TYPE_S32, "0", 	  "rw", 0, 2, "0: send all 1:just send video 2: not send"},
        {"format_encodeType",  &(runVideoCfg.vencStream[0].streamFormat.encodeType), GK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 2, "0: none, 1: H.264, 2: MJPEG"},
        #ifdef RESOLUTION_1080P
        {"format_width",       &(runVideoCfg.vencStream[0].streamFormat.width),      GK_CFG_DATA_TYPE_U16, "1920",    "rw", 0, 1920, NULL},
        {"format_height",      &(runVideoCfg.vencStream[0].streamFormat.height),     GK_CFG_DATA_TYPE_U16, "1080",    "rw", 0, 1080, NULL},
        {"resolution",         &(runVideoCfg.vencStream[0].streamFormat.resolution), GK_CFG_DATA_TYPE_STRING, "{\"opt\" : [ \"1920x1080\", \"1280x960\", \"1280x720\", \"1024x768\", \"1024x576\", \"960x576\", \"960x480\", \"720x576\", \"720x480\" ]}", "ro", 0, MAX_STR_LEN_128, NULL},
        #elif defined RESOLUTION_960P
        {"format_width",       &(runVideoCfg.vencStream[0].streamFormat.width),      GK_CFG_DATA_TYPE_U16, "1280",    "rw", 0, 1280, NULL},
        {"format_height",      &(runVideoCfg.vencStream[0].streamFormat.height),     GK_CFG_DATA_TYPE_U16, "960",    "rw", 0,  960, NULL},
        {"resolution",         &(runVideoCfg.vencStream[0].streamFormat.resolution), GK_CFG_DATA_TYPE_STRING, "{\"opt\" : [ \"1280x960\", \"1024x576\", \"960x576\", \"960x480\", \"720x576\", \"720x480\",  \"640x480\" ]}", "ro", 0, MAX_STR_LEN_128, NULL},
        #elif defined RESOLUTION_720P
        {"format_width",       &(runVideoCfg.vencStream[0].streamFormat.width),      GK_CFG_DATA_TYPE_U16, "1280",    "rw", 0, 1280, NULL},
        {"format_height",      &(runVideoCfg.vencStream[0].streamFormat.height),     GK_CFG_DATA_TYPE_U16, "720",    "rw", 0,  720, NULL},
        {"resolution",         &(runVideoCfg.vencStream[0].streamFormat.resolution), GK_CFG_DATA_TYPE_STRING, "{\"opt\" : [ \"1280x720\", \"1024x576\", \"960x576\", \"960x480\", \"720x576\", \"720x480\",  \"640x480\" ]}", "ro", 0, MAX_STR_LEN_128, NULL},
        #endif

        {"format_fps",         &(runVideoCfg.vencStream[0].streamFormat.fps),        GK_CFG_DATA_TYPE_U32, "25",      "rw", 1, 25,   NULL},
        {"format_keepAspRat",  &(runVideoCfg.vencStream[0].streamFormat.keepAspRat), GK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 1,  "1: means keep aspect ration, 0: means do not keep."},
        {"format_streamName",  &(runVideoCfg.vencStream[0].streamFormat.streamName), GK_CFG_DATA_TYPE_STRING,  "IPC", "rw", 0, MAX_STR_LEN_128,  NULL},
        {"h264_gopM",          &(runVideoCfg.vencStream[0].h264Conf.gopM),           GK_CFG_DATA_TYPE_U8,  "1",       "rw", 1, 120,  NULL},
        {"h264_gopN",          &(runVideoCfg.vencStream[0].h264Conf.gopN),           GK_CFG_DATA_TYPE_U8,  "25",      "rw", 1, 50,  NULL},
        {"h264_idrInterval",   &(runVideoCfg.vencStream[0].h264Conf.idrInterval),    GK_CFG_DATA_TYPE_U8,  "1",       "rw", 1, 30,  NULL},
        {"h264_gopModel",      &(runVideoCfg.vencStream[0].h264Conf.gopModel),       GK_CFG_DATA_TYPE_U8,  "0",       "rw", 0, 5,  NULL},
        {"h264_profile",       &(runVideoCfg.vencStream[0].h264Conf.profile),        GK_CFG_DATA_TYPE_U8,  "0",       "rw", 0, 1,  "0: main, 1: baseline"},
        {"h264_brcMode",       &(runVideoCfg.vencStream[0].h264Conf.brcMode),        GK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 3,  "0: CBR; 1: VBR; 2: CBR keep quality; 3: VBR keep quality"},
        #ifdef GK7101
        {"h264_cbrAvgBps",     &(runVideoCfg.vencStream[0].h264Conf.cbrAvgBps),      GK_CFG_DATA_TYPE_U32, "2000", "rw", 0, 8096,  NULL},
        {"h264_vbrMinbps",     &(runVideoCfg.vencStream[0].h264Conf.vbrMinbps),      GK_CFG_DATA_TYPE_U32, "1000", "rw", 0, 100,  NULL},
        {"h264_vbrMaxbps",     &(runVideoCfg.vencStream[0].h264Conf.vbrMaxbps),      GK_CFG_DATA_TYPE_U32, "2000", "rw", 0, 8096,  NULL},
        #else
        {"h264_cbrAvgBps",     &(runVideoCfg.vencStream[0].h264Conf.cbrAvgBps),      GK_CFG_DATA_TYPE_U32, "2000", "rw", 0, 4096,  NULL},
        {"h264_vbrMinbps",     &(runVideoCfg.vencStream[0].h264Conf.vbrMinbps),      GK_CFG_DATA_TYPE_U32, "1000", "rw", 0, 50,  NULL},
        {"h264_vbrMaxbps",     &(runVideoCfg.vencStream[0].h264Conf.vbrMaxbps),      GK_CFG_DATA_TYPE_U32, "2000", "rw", 0, 4096,  NULL},

        #endif
        {"h264_quality",       &(runVideoCfg.vencStream[0].h264Conf.quality),        GK_CFG_DATA_TYPE_U8,  "3",       "rw", 0, 3,  "0: poor, 3: best"},
        {"h264_qcon",          &(runVideoCfg.vencStream[0].h264Conf.qcon),           GK_CFG_DATA_TYPE_U8,  "3",       "rw", 0, 3,  "0: poor, 3: best"},

        {"h264_qpMinOnI",      &(runVideoCfg.vencStream[0].h264QpConf.qpMinOnI),     GK_CFG_DATA_TYPE_U8,  "255",       "rw", 0, 100,  NULL},
        {"h264_qpMaxOnI",      &(runVideoCfg.vencStream[0].h264QpConf.qpMaxOnI),     GK_CFG_DATA_TYPE_U8,  "35",       "rw", 0, 100,  NULL},
        {"h264_qpMinOnP",      &(runVideoCfg.vencStream[0].h264QpConf.qpMinOnP),     GK_CFG_DATA_TYPE_U8,  "255",       "rw", 0, 100,  NULL},
        {"h264_qpMaxOnP",      &(runVideoCfg.vencStream[0].h264QpConf.qpMaxOnP),     GK_CFG_DATA_TYPE_U8,  "40",       "rw", 0, 100,  NULL},
        {"h264_qpIWeight",     &(runVideoCfg.vencStream[0].h264QpConf.qpIWeight),    GK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 100,  NULL},
        {"h264_qpPWeight",     &(runVideoCfg.vencStream[0].h264QpConf.qpPWeight),    GK_CFG_DATA_TYPE_U8,  "5",       "rw", 0, 100,  NULL},
        {"h264_adaptQp",       &(runVideoCfg.vencStream[0].h264QpConf.adaptQp),      GK_CFG_DATA_TYPE_U8,  "2",       "rw", 0, 100,  NULL},

        {"mjpeg_chromaFormat", &(runVideoCfg.vencStream[0].mjpegConf.chromaFormat),  GK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 1,  "0: YUV 422, 1: YUV 420."},
        {"mjpeg_quality",      &(runVideoCfg.vencStream[0].mjpegConf.quality),       GK_CFG_DATA_TYPE_U8,  "50",      "rw", 1, 100,  "1 ~ 100, 100 is best quality"},
        {NULL,},

    },
    {
        {"id",                 &(runVideoCfg.vencStream[1].id),                      GK_CFG_DATA_TYPE_S32, "1",       "rw", 0, 3, "0-3"},
        {"enable",             &(runVideoCfg.vencStream[1].enable),                  GK_CFG_DATA_TYPE_S32, "1",       "rw", 0, 1, NULL},
		{"avStream",		   &(runVideoCfg.vencStream[1].avStream),				 GK_CFG_DATA_TYPE_S32, "0", 	  "rw", 0, 2, "0: send all 1:just send video 2: not send"},
        {"format_encodeType",  &(runVideoCfg.vencStream[1].streamFormat.encodeType), GK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 2, "0: none, 1: H.264, 2: MJPEG"},
#ifdef RESOLUTION_1080P
        {"format_width",       &(runVideoCfg.vencStream[1].streamFormat.width),      GK_CFG_DATA_TYPE_U16, "640",     "rw", 0, 640, NULL},
        {"format_height",      &(runVideoCfg.vencStream[1].streamFormat.height),     GK_CFG_DATA_TYPE_U16, "480",     "rw", 0, 480, NULL},
        {"resolution",         &(runVideoCfg.vencStream[1].streamFormat.resolution), GK_CFG_DATA_TYPE_STRING, "{\"opt\" : [ \"640x480\", \"640x360\", \"352x288\", \"352x240\" ]}", "ro", 0, MAX_STR_LEN_128, NULL},
#elif defined RESOLUTION_960P
        {"format_width",       &(runVideoCfg.vencStream[1].streamFormat.width),      GK_CFG_DATA_TYPE_U16, "640",     "rw", 0, 640, NULL},
        {"format_height",      &(runVideoCfg.vencStream[1].streamFormat.height),     GK_CFG_DATA_TYPE_U16, "360",     "rw", 0, 360, NULL},
        {"resolution",         &(runVideoCfg.vencStream[1].streamFormat.resolution), GK_CFG_DATA_TYPE_STRING, "{\"opt\" : [  \"640x360\", \"352x288\", \"352x240\" ]}", "ro", 0, MAX_STR_LEN_128, NULL},
#elif defined RESOLUTION_720P
		{"format_width",	   &(runVideoCfg.vencStream[1].streamFormat.width), 	 GK_CFG_DATA_TYPE_U16, "640",	  "rw", 0, 640, NULL},
		{"format_height",	   &(runVideoCfg.vencStream[1].streamFormat.height),	 GK_CFG_DATA_TYPE_U16, "360",	  "rw", 0, 360, NULL},
        {"resolution",         &(runVideoCfg.vencStream[1].streamFormat.resolution), GK_CFG_DATA_TYPE_STRING, "{\"opt\" : [ \"640x360\", \"352x288\", \"352x240\" ]}", "ro", 0, MAX_STR_LEN_128, NULL},

#endif
        {"format_fps",         &(runVideoCfg.vencStream[1].streamFormat.fps),        GK_CFG_DATA_TYPE_U32, "25",      "rw", 1, 30,  NULL},
        {"format_keepAspRat",  &(runVideoCfg.vencStream[1].streamFormat.keepAspRat), GK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 1,  "1: means keep aspect ration, 0: means do not keep."},
        {"format_streamName",  &(runVideoCfg.vencStream[1].streamFormat.streamName), GK_CFG_DATA_TYPE_STRING,  "IPC", "rw", 0, MAX_STR_LEN_128,  NULL},

        {"h264_gopM",          &(runVideoCfg.vencStream[1].h264Conf.gopM),           GK_CFG_DATA_TYPE_U8,  "1",       "rw", 1, 120,  NULL},
        {"h264_gopN",          &(runVideoCfg.vencStream[1].h264Conf.gopN),           GK_CFG_DATA_TYPE_U8,  "25",      "rw", 1, 50,  NULL},
        {"h264_idrInterval",   &(runVideoCfg.vencStream[1].h264Conf.idrInterval),    GK_CFG_DATA_TYPE_U8,  "1",       "rw", 1, 30,  NULL},
        {"h264_gopModel",      &(runVideoCfg.vencStream[1].h264Conf.gopModel),       GK_CFG_DATA_TYPE_U8,  "0",       "rw", 0, 5,  NULL},
        {"h264_profile",       &(runVideoCfg.vencStream[1].h264Conf.profile),        GK_CFG_DATA_TYPE_U8,  "0",       "rw", 0, 1,  "0: main, 1: baseline"},
        {"h264_brcMode",       &(runVideoCfg.vencStream[1].h264Conf.brcMode),        GK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 3,  "0: CBR; 1: VBR; 2: CBR keep quality; 3: VBR keep quality"},
        {"h264_cbrAvgBps",     &(runVideoCfg.vencStream[1].h264Conf.cbrAvgBps),      GK_CFG_DATA_TYPE_U32, "500",    "rw", 0, 3000,  NULL},
        {"h264_vbrMinbps",     &(runVideoCfg.vencStream[1].h264Conf.vbrMinbps),      GK_CFG_DATA_TYPE_U32, "300",    "rw", 0,  200,  NULL},
        {"h264_vbrMaxbps",     &(runVideoCfg.vencStream[1].h264Conf.vbrMaxbps),      GK_CFG_DATA_TYPE_U32, "500",    "rw", 0, 3000,  NULL},
        {"h264_quality",       &(runVideoCfg.vencStream[1].h264Conf.quality),        GK_CFG_DATA_TYPE_U8,  "3",       "rw", 0, 3,  "0: poor, 3: best"},
        {"h264_qcon",          &(runVideoCfg.vencStream[1].h264Conf.qcon),           GK_CFG_DATA_TYPE_U8,  "3",       "rw", 0, 3,  "0: poor, 3: best"},

        {"h264_qpMinOnI",      &(runVideoCfg.vencStream[1].h264QpConf.qpMinOnI),     GK_CFG_DATA_TYPE_U8,  "255",       "rw", 0, 100,  NULL},
        {"h264_qpMaxOnI",      &(runVideoCfg.vencStream[1].h264QpConf.qpMaxOnI),     GK_CFG_DATA_TYPE_U8,  "35",       "rw", 0, 100,  NULL},
        {"h264_qpMinOnP",      &(runVideoCfg.vencStream[1].h264QpConf.qpMinOnP),     GK_CFG_DATA_TYPE_U8,  "255",       "rw", 0, 100,  NULL},
        {"h264_qpMaxOnP",      &(runVideoCfg.vencStream[1].h264QpConf.qpMaxOnP),     GK_CFG_DATA_TYPE_U8,  "40",       "rw", 0, 100,  NULL},
        {"h264_qpIWeight",     &(runVideoCfg.vencStream[1].h264QpConf.qpIWeight),    GK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 100,  NULL},
        {"h264_qpPWeight",     &(runVideoCfg.vencStream[1].h264QpConf.qpPWeight),    GK_CFG_DATA_TYPE_U8,  "5",       "rw", 0, 100,  NULL},
        {"h264_adaptQp",       &(runVideoCfg.vencStream[1].h264QpConf.adaptQp),      GK_CFG_DATA_TYPE_U8,  "2",       "rw", 0, 100,  NULL},

        {"mjpeg_chromaFormat", &(runVideoCfg.vencStream[1].mjpegConf.chromaFormat),  GK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 1,  "0: YUV 422, 1: YUV 420."},
        {"mjpeg_quality",      &(runVideoCfg.vencStream[1].mjpegConf.quality),       GK_CFG_DATA_TYPE_U8,  "50",      "rw", 1, 100,  "1 ~ 100, 100 is best quality"},
        {NULL,},
    },
    {
        {"id",                 &(runVideoCfg.vencStream[2].id),                      GK_CFG_DATA_TYPE_S32, "2",       "rw", 0, 3, "0-3"},
        {"enable",             &(runVideoCfg.vencStream[2].enable),                  GK_CFG_DATA_TYPE_S32, "1",       "rw", 0, 1, NULL},
		{"avStream",		   &(runVideoCfg.vencStream[2].avStream),				 GK_CFG_DATA_TYPE_S32, "0", 	  "rw", 0, 2, "0: send all 1:just send video 2: not send"},
        {"format_encodeType",  &(runVideoCfg.vencStream[2].streamFormat.encodeType), GK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 2, "0: none, 1: H.264, 2: MJPEG"},
#ifdef RESOLUTION_1080P
        {"format_width",       &(runVideoCfg.vencStream[2].streamFormat.width),      GK_CFG_DATA_TYPE_U16, "320",       "rw", 0, 320, NULL},
        {"format_height",      &(runVideoCfg.vencStream[2].streamFormat.height),     GK_CFG_DATA_TYPE_U16, "240",       "rw", 0, 240, NULL},
        {"resolution",         &(runVideoCfg.vencStream[2].streamFormat.resolution), GK_CFG_DATA_TYPE_STRING, "{\"opt\" : [ \"320x240\", \"320x180\" ]}", "ro", 0, MAX_STR_LEN_128, NULL},
#elif defined RESOLUTION_960P
        {"format_width",       &(runVideoCfg.vencStream[2].streamFormat.width),      GK_CFG_DATA_TYPE_U16, "320",       "rw", 0, 320, NULL},
        {"format_height",      &(runVideoCfg.vencStream[2].streamFormat.height),     GK_CFG_DATA_TYPE_U16, "220",       "rw", 0, 220, NULL},
        {"resolution",         &(runVideoCfg.vencStream[2].streamFormat.resolution), GK_CFG_DATA_TYPE_STRING, "{\"opt\" : [ \"320x220\", \"320x180\" ]}", "ro", 0, MAX_STR_LEN_128, NULL},
#elif defined RESOLUTION_720P
        {"format_width",       &(runVideoCfg.vencStream[2].streamFormat.width),      GK_CFG_DATA_TYPE_U16, "320",       "rw", 0, 320, NULL},
        {"format_height",      &(runVideoCfg.vencStream[2].streamFormat.height),     GK_CFG_DATA_TYPE_U16, "240",       "rw", 0, 240, NULL},
        {"resolution",         &(runVideoCfg.vencStream[2].streamFormat.resolution), GK_CFG_DATA_TYPE_STRING, "{\"opt\" : [ \"320x240\", \"320x180\" ]}", "ro", 0, MAX_STR_LEN_128, NULL},
#endif
        {"format_fps",         &(runVideoCfg.vencStream[2].streamFormat.fps),        GK_CFG_DATA_TYPE_U32, "25",       "rw", 1, 30,   NULL},
        {"format_keepAspRat",  &(runVideoCfg.vencStream[2].streamFormat.keepAspRat), GK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 1,  "1: means keep aspect ration, 0: means do not keep."},
        {"format_streamName",  &(runVideoCfg.vencStream[2].streamFormat.streamName), GK_CFG_DATA_TYPE_STRING,  "CH-2", "rw", 0, MAX_STR_LEN_128,  NULL},

        {"h264_gopM",          &(runVideoCfg.vencStream[2].h264Conf.gopM),           GK_CFG_DATA_TYPE_U8,  "1",       "rw", 1, 120,  NULL},
        {"h264_gopN",          &(runVideoCfg.vencStream[2].h264Conf.gopN),           GK_CFG_DATA_TYPE_U8,  "25",      "rw", 1, 50,  NULL},
        {"h264_idrInterval",   &(runVideoCfg.vencStream[2].h264Conf.idrInterval),    GK_CFG_DATA_TYPE_U8,  "1",       "rw", 1, 30,  NULL},
        {"h264_gopModel",      &(runVideoCfg.vencStream[2].h264Conf.gopModel),       GK_CFG_DATA_TYPE_U8,  "0",       "rw", 0, 5,  NULL},
        {"h264_profile",       &(runVideoCfg.vencStream[2].h264Conf.profile),        GK_CFG_DATA_TYPE_U8,  "0",       "rw", 0, 1,  "0: main, 1: baseline"},
        {"h264_brcMode",       &(runVideoCfg.vencStream[2].h264Conf.brcMode),        GK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 3,  "0: CBR; 1: VBR; 2: CBR keep quality; 3: VBR keep quality"},
        {"h264_cbrAvgBps",     &(runVideoCfg.vencStream[2].h264Conf.cbrAvgBps),      GK_CFG_DATA_TYPE_U32, "128",    "rw", 0, 400,  NULL},
        {"h264_vbrMinbps",     &(runVideoCfg.vencStream[2].h264Conf.vbrMinbps),      GK_CFG_DATA_TYPE_U32, "88",     "rw", 0, 10,  NULL},
        {"h264_vbrMaxbps",     &(runVideoCfg.vencStream[2].h264Conf.vbrMaxbps),      GK_CFG_DATA_TYPE_U32, "128",    "rw", 0, 400,  NULL},
        {"h264_quality",       &(runVideoCfg.vencStream[2].h264Conf.quality),        GK_CFG_DATA_TYPE_U8,  "3",       "rw", 0, 3,  "0: poor, 3: best"},
        {"h264_qcon",          &(runVideoCfg.vencStream[2].h264Conf.qcon),           GK_CFG_DATA_TYPE_U8,  "3",       "rw", 0, 3,  "0: poor, 3: best"},

        {"h264_qpMinOnI",      &(runVideoCfg.vencStream[2].h264QpConf.qpMinOnI),     GK_CFG_DATA_TYPE_U8,  "255",       "rw", 0, 100,  NULL},
        {"h264_qpMaxOnI",      &(runVideoCfg.vencStream[2].h264QpConf.qpMaxOnI),     GK_CFG_DATA_TYPE_U8,  "35",       "rw", 0, 100,  NULL},
        {"h264_qpMinOnP",      &(runVideoCfg.vencStream[2].h264QpConf.qpMinOnP),     GK_CFG_DATA_TYPE_U8,  "255",       "rw", 0, 100,  NULL},
        {"h264_qpMaxOnP",      &(runVideoCfg.vencStream[2].h264QpConf.qpMaxOnP),     GK_CFG_DATA_TYPE_U8,  "40",       "rw", 0, 100,  NULL},
        {"h264_qpIWeight",     &(runVideoCfg.vencStream[2].h264QpConf.qpIWeight),    GK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 100,  NULL},
        {"h264_qpPWeight",     &(runVideoCfg.vencStream[2].h264QpConf.qpPWeight),    GK_CFG_DATA_TYPE_U8,  "5",       "rw", 0, 100,  NULL},
        {"h264_adaptQp",       &(runVideoCfg.vencStream[2].h264QpConf.adaptQp),      GK_CFG_DATA_TYPE_U8,  "2",       "rw", 0, 100,  NULL},

        {"mjpeg_chromaFormat", &(runVideoCfg.vencStream[2].mjpegConf.chromaFormat),  GK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 1,  "0: YUV 422, 1: YUV 420."},
        {"mjpeg_quality",      &(runVideoCfg.vencStream[2].mjpegConf.quality),       GK_CFG_DATA_TYPE_U8,  "50",      "rw", 1, 100,  "1 ~ 100, 100 is best quality"},
        {NULL,},
    },
    {
        {"id",                 &(runVideoCfg.vencStream[3].id),                      GK_CFG_DATA_TYPE_S32, "3",       "rw", 0, 3, "0-3"},
        {"enable",             &(runVideoCfg.vencStream[3].enable),                  GK_CFG_DATA_TYPE_S32, "0",       "rw", 0, 1, NULL},
		{"avStream",		   &(runVideoCfg.vencStream[3].avStream),				 GK_CFG_DATA_TYPE_S32, "0", 	  "rw", 0, 2, "0: send all 1:just send video 2: not send"},
        {"format_encodeType",  &(runVideoCfg.vencStream[3].streamFormat.encodeType), GK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 2, "0: none, 1: H.264, 2: MJPEG"},
        {"format_width",       &(runVideoCfg.vencStream[3].streamFormat.width),      GK_CFG_DATA_TYPE_U16, "0",       "rw", 0, 160, NULL},
        #ifdef RESOLUTION_1080P
        {"resolution",         &(runVideoCfg.vencStream[3].streamFormat.resolution), GK_CFG_DATA_TYPE_STRING, "{\"opt\" : [ \"1920x1080\", \"1280x960\", \"1280x720\", \"1024x768\", \"1024x576\", \"960x576\", \"960x480\", \"720x576\", \"720x480\" ]}", "ro", 0, MAX_STR_LEN_128, NULL},
        #elif defined RESOLUTION_960P
        {"resolution",         &(runVideoCfg.vencStream[3].streamFormat.resolution), GK_CFG_DATA_TYPE_STRING, "{\"opt\" : [ \"1280x960\", \"1024x576\", \"960x576\", \"960x480\", \"720x576\", \"720x480\",  \"640x480\" ]}", "ro", 0, MAX_STR_LEN_128, NULL},
        #elif defined RESOLUTION_720P
        {"resolution",         &(runVideoCfg.vencStream[3].streamFormat.resolution), GK_CFG_DATA_TYPE_STRING, "{\"opt\" : [ \"1280x720\", \"1024x576\", \"960x576\", \"960x480\", \"720x576\", \"720x480\",  \"640x480\" ]}", "ro", 0, MAX_STR_LEN_128, NULL},
        #endif
        {"format_height",      &(runVideoCfg.vencStream[3].streamFormat.height),     GK_CFG_DATA_TYPE_U16, "0",       "rw", 0, 90, NULL},
        {"format_fps",         &(runVideoCfg.vencStream[3].streamFormat.fps),        GK_CFG_DATA_TYPE_U32, "0",       "rw", 1, 30,   NULL},
        {"format_keepAspRat",  &(runVideoCfg.vencStream[3].streamFormat.keepAspRat), GK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 1,  "1: means keep aspect ration, 0: means do not keep."},
        {"format_streamName",  &(runVideoCfg.vencStream[3].streamFormat.streamName), GK_CFG_DATA_TYPE_STRING,  "CH-3", "rw", 0, MAX_STR_LEN_128,  NULL},

        {"h264_gopM",          &(runVideoCfg.vencStream[3].h264Conf.gopM),           GK_CFG_DATA_TYPE_U8,  "1",       "rw", 1, 120,  NULL},
        {"h264_gopN",          &(runVideoCfg.vencStream[3].h264Conf.gopN),           GK_CFG_DATA_TYPE_U8,  "30",      "rw", 1, 50,  NULL},
        {"h264_idrInterval",   &(runVideoCfg.vencStream[3].h264Conf.idrInterval),    GK_CFG_DATA_TYPE_U8,  "1",       "rw", 1, 30,  NULL},
        {"h264_gopModel",      &(runVideoCfg.vencStream[3].h264Conf.gopModel),       GK_CFG_DATA_TYPE_U8,  "0",       "rw", 0, 5,  NULL},
        {"h264_profile",       &(runVideoCfg.vencStream[3].h264Conf.profile),        GK_CFG_DATA_TYPE_U8,  "0",       "rw", 0, 1,  "0: main, 1: baseline"},
        {"h264_brcMode",       &(runVideoCfg.vencStream[3].h264Conf.brcMode),        GK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 3,  "0: CBR; 1: VBR; 2: CBR keep quality; 3: VBR keep quality"},
        {"h264_cbrAvgBps",     &(runVideoCfg.vencStream[3].h264Conf.cbrAvgBps),      GK_CFG_DATA_TYPE_U32, "100",     "rw", 0, 100,  NULL},
        {"h264_vbrMinbps",     &(runVideoCfg.vencStream[3].h264Conf.vbrMinbps),      GK_CFG_DATA_TYPE_U32, "5",       "rw", 0, 5,  NULL},
        {"h264_vbrMaxbps",     &(runVideoCfg.vencStream[3].h264Conf.vbrMaxbps),      GK_CFG_DATA_TYPE_U32, "100",     "rw", 0, 100,  NULL},
        {"h264_quality",       &(runVideoCfg.vencStream[3].h264Conf.quality),        GK_CFG_DATA_TYPE_U8,  "3",       "rw", 0, 3,  "0: poor, 3: best"},
        {"h264_qcon",          &(runVideoCfg.vencStream[3].h264Conf.qcon),           GK_CFG_DATA_TYPE_U8,  "3",       "rw", 0, 3,  "0: poor, 3: best"},

        {"h264_qpMinOnI",      &(runVideoCfg.vencStream[3].h264QpConf.qpMinOnI),     GK_CFG_DATA_TYPE_U8,  "255",       "rw", 0, 100,  NULL},
        {"h264_qpMaxOnI",      &(runVideoCfg.vencStream[3].h264QpConf.qpMaxOnI),     GK_CFG_DATA_TYPE_U8,  "35",       "rw", 0, 100,  NULL},
        {"h264_qpMinOnP",      &(runVideoCfg.vencStream[3].h264QpConf.qpMinOnP),     GK_CFG_DATA_TYPE_U8,  "255",       "rw", 0, 100,  NULL},
        {"h264_qpMaxOnP",      &(runVideoCfg.vencStream[3].h264QpConf.qpMaxOnP),     GK_CFG_DATA_TYPE_U8,  "40",       "rw", 0, 100,  NULL},
        {"h264_qpIWeight",     &(runVideoCfg.vencStream[3].h264QpConf.qpIWeight),    GK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 100,  NULL},
        {"h264_qpPWeight",     &(runVideoCfg.vencStream[3].h264QpConf.qpPWeight),    GK_CFG_DATA_TYPE_U8,  "5",       "rw", 0, 100,  NULL},
        {"h264_adaptQp",       &(runVideoCfg.vencStream[3].h264QpConf.adaptQp),      GK_CFG_DATA_TYPE_U8,  "2",       "rw", 0, 100,  NULL},

        {"mjpeg_chromaFormat", &(runVideoCfg.vencStream[3].mjpegConf.chromaFormat),  GK_CFG_DATA_TYPE_U8,  "1",       "rw", 0, 1,  "0: YUV 422, 1: YUV 420."},
        {"mjpeg_quality",      &(runVideoCfg.vencStream[3].mjpegConf.quality),       GK_CFG_DATA_TYPE_U8,  "50",      "rw", 1, 100,  "1 ~ 100, 100 is best quality"},
        {NULL,},
    },
};


#else
static GK_CFG_MAP viMap[MAX_SENSOR_NUM][MAX_SENSOR_ITEM_NUM] = {
    {
        {"width",  &(runVideoCfg.vi[0].width),          },
        {"height", &(runVideoCfg.vi[0].height),         },
        {"fps",    &(runVideoCfg.vi[0].fps),            },
        {"pi",     &(runVideoCfg.vi[0].pi),             }, 
        {"mirror", &(runVideoCfg.vi[0].mirrorPattern),  },
        {"bayer",  &(runVideoCfg.vi[0].bayerPattern),   },
        {NULL,},
    },
    {
        {"width",  &(runVideoCfg.vi[1].width),          },
        {"height", &(runVideoCfg.vi[1].height),         },
        {"fps",    &(runVideoCfg.vi[1].fps),            },
        {"pi",     &(runVideoCfg.vi[1].pi),             }, 
        {"mirror", &(runVideoCfg.vi[1].mirrorPattern),  },
        {"bayer",  &(runVideoCfg.vi[1].bayerPattern),   },
        {NULL,},
    }
};

static GK_CFG_MAP voutMap[MAX_VOUT_NUM][MAX_VOUT_ITEM_NUM] = {
    {
        {"channel", &(runVideoCfg.vout[0].voutChannel), },
        {"mode",    &(runVideoCfg.vout[0].resoluMode),  },
        {"dev",     &(runVideoCfg.vout[0].deviceType),  },
        {NULL,},
    },
    {
        {"channel", &(runVideoCfg.vout[1].voutChannel), },
        {"mode",    &(runVideoCfg.vout[1].resoluMode),  },
        {"dev",     &(runVideoCfg.vout[1].deviceType),  },
        {NULL,},
    }
};

GK_CFG_MAP videoMap[MAX_VENC_STREAM_NUM][MAX_VENC_ITEM_NUM] = {
    {
        {"id",         &(runVideoCfg.vencStream[0].id),             },
        {"enable",     &(runVideoCfg.vencStream[0].enable),         },
        {"avStream",   &(runVideoCfg.vencStream[0].avStream),       },
        {"enctype",    &(runVideoCfg.vencStream[0].enctype),        }, 
        {"resolution", &(runVideoCfg.vencStream[0].resolution),     },
         
        {"name",       &(runVideoCfg.vencStream[0].h264Conf.name),    },
        {"width",      &(runVideoCfg.vencStream[0].h264Conf.width),   },
        {"height",     &(runVideoCfg.vencStream[0].h264Conf.height),  },
        {"fps",        &(runVideoCfg.vencStream[0].h264Conf.fps),     },
        {"gop",        &(runVideoCfg.vencStream[0].h264Conf.gop),     },
        {"profile",    &(runVideoCfg.vencStream[0].h264Conf.profile), },
        {"rc_mode",    &(runVideoCfg.vencStream[0].h264Conf.rc_mode), },
        {"bps",        &(runVideoCfg.vencStream[0].h264Conf.bps),     },
        {"quality",    &(runVideoCfg.vencStream[0].h264Conf.quality), },
        {"re_rec_mode",    &(runVideoCfg.vencStream[0].h264Conf.re_rec_mode), },

        {"qpMinOnI",   &(runVideoCfg.vencStream[0].h264QpConf.qpMinOnI),  },
        {"qpMaxOnI",   &(runVideoCfg.vencStream[0].h264QpConf.qpMaxOnI),  },
        {"qpMinOnP",   &(runVideoCfg.vencStream[0].h264QpConf.qpMinOnP),  },
        {"qpMaxOnP",   &(runVideoCfg.vencStream[0].h264QpConf.qpMaxOnP),  },
        {"qpIWeight",  &(runVideoCfg.vencStream[0].h264QpConf.qpIWeight), },
        {"qpPWeight",  &(runVideoCfg.vencStream[0].h264QpConf.qpPWeight), },
        {"adaptQp",    &(runVideoCfg.vencStream[0].h264QpConf.adaptQp),   },

        {NULL,},
    },
    {
        {"id",         &(runVideoCfg.vencStream[1].id),             },
        {"enable",     &(runVideoCfg.vencStream[1].enable),         },
        {"avStream",   &(runVideoCfg.vencStream[1].avStream),       },
        {"enctype",    &(runVideoCfg.vencStream[1].enctype),        }, 
        {"resolution", &(runVideoCfg.vencStream[1].resolution),     },
         
        {"name",       &(runVideoCfg.vencStream[1].h264Conf.name),    },
        {"width",      &(runVideoCfg.vencStream[1].h264Conf.width),   },
        {"height",     &(runVideoCfg.vencStream[1].h264Conf.height),  },
        {"fps",        &(runVideoCfg.vencStream[1].h264Conf.fps),     },
        {"gop",        &(runVideoCfg.vencStream[1].h264Conf.gop),     },
        {"profile",    &(runVideoCfg.vencStream[1].h264Conf.profile), },
        {"rc_mode",    &(runVideoCfg.vencStream[1].h264Conf.rc_mode), },
        {"bps",        &(runVideoCfg.vencStream[1].h264Conf.bps),     },
        {"quality",    &(runVideoCfg.vencStream[1].h264Conf.quality), },
        {"re_rec_mode",    &(runVideoCfg.vencStream[1].h264Conf.re_rec_mode), },

        {"qpMinOnI",   &(runVideoCfg.vencStream[1].h264QpConf.qpMinOnI),  },
        {"qpMaxOnI",   &(runVideoCfg.vencStream[1].h264QpConf.qpMaxOnI),  },
        {"qpMinOnP",   &(runVideoCfg.vencStream[1].h264QpConf.qpMinOnP),  },
        {"qpMaxOnP",   &(runVideoCfg.vencStream[1].h264QpConf.qpMaxOnP),  },
        {"qpIWeight",  &(runVideoCfg.vencStream[1].h264QpConf.qpIWeight), },
        {"qpPWeight",  &(runVideoCfg.vencStream[1].h264QpConf.qpPWeight), },
        {"adaptQp",    &(runVideoCfg.vencStream[1].h264QpConf.adaptQp),   },


        {NULL,},
    },
    {
        {"id",         &(runVideoCfg.vencStream[2].id),             },
        {"enable",     &(runVideoCfg.vencStream[2].enable),         },
        {"avStream",   &(runVideoCfg.vencStream[2].avStream),       },
        {"enctype",    &(runVideoCfg.vencStream[2].enctype),        }, 
        {"resolution", &(runVideoCfg.vencStream[2].resolution),     },
         
        {"name",       &(runVideoCfg.vencStream[2].h264Conf.name),    },
        {"width",      &(runVideoCfg.vencStream[2].h264Conf.width),   },
        {"height",     &(runVideoCfg.vencStream[2].h264Conf.height),  },
        {"fps",        &(runVideoCfg.vencStream[2].h264Conf.fps),     },
        {"gop",        &(runVideoCfg.vencStream[2].h264Conf.gop),     },
        {"profile",    &(runVideoCfg.vencStream[2].h264Conf.profile), },
        {"rc_mode",    &(runVideoCfg.vencStream[2].h264Conf.rc_mode), },
        {"bps",        &(runVideoCfg.vencStream[2].h264Conf.bps),     },
        {"quality",    &(runVideoCfg.vencStream[2].h264Conf.quality), },
        {"re_rec_mode",    &(runVideoCfg.vencStream[2].h264Conf.re_rec_mode), },

        {"qpMinOnI",   &(runVideoCfg.vencStream[2].h264QpConf.qpMinOnI),  },
        {"qpMaxOnI",   &(runVideoCfg.vencStream[2].h264QpConf.qpMaxOnI),  },
        {"qpMinOnP",   &(runVideoCfg.vencStream[2].h264QpConf.qpMinOnP),  },
        {"qpMaxOnP",   &(runVideoCfg.vencStream[2].h264QpConf.qpMaxOnP),  },
        {"qpIWeight",  &(runVideoCfg.vencStream[2].h264QpConf.qpIWeight), },
        {"qpPWeight",  &(runVideoCfg.vencStream[2].h264QpConf.qpPWeight), },
        {"adaptQp",    &(runVideoCfg.vencStream[2].h264QpConf.adaptQp),   },

        {NULL,},
    },
    {
        {"id",         &(runVideoCfg.vencStream[3].id),             },
        {"enable",     &(runVideoCfg.vencStream[3].enable),         },
        {"avStream",   &(runVideoCfg.vencStream[3].avStream),       },
        {"enctype",    &(runVideoCfg.vencStream[3].enctype),        }, 
        {"resolution", &(runVideoCfg.vencStream[3].resolution),     },
         
        {"name",       &(runVideoCfg.vencStream[3].h264Conf.name),    },
        {"width",      &(runVideoCfg.vencStream[3].h264Conf.width),   },
        {"height",     &(runVideoCfg.vencStream[3].h264Conf.height),  },
        {"fps",        &(runVideoCfg.vencStream[3].h264Conf.fps),     },
        {"gop",        &(runVideoCfg.vencStream[3].h264Conf.gop),     },
        {"profile",    &(runVideoCfg.vencStream[3].h264Conf.profile), },
        {"rc_mode",    &(runVideoCfg.vencStream[3].h264Conf.rc_mode), },
        {"bps",        &(runVideoCfg.vencStream[3].h264Conf.bps),     },
        {"quality",    &(runVideoCfg.vencStream[3].h264Conf.quality), },
        {"re_rec_mode",    &(runVideoCfg.vencStream[3].h264Conf.re_rec_mode), },

        {"qpMinOnI",   &(runVideoCfg.vencStream[3].h264QpConf.qpMinOnI),  },
        {"qpMaxOnI",   &(runVideoCfg.vencStream[3].h264QpConf.qpMaxOnI),  },
        {"qpMinOnP",   &(runVideoCfg.vencStream[3].h264QpConf.qpMinOnP),  },
        {"qpMaxOnP",   &(runVideoCfg.vencStream[3].h264QpConf.qpMaxOnP),  },
        {"qpIWeight",  &(runVideoCfg.vencStream[3].h264QpConf.qpIWeight), },
        {"qpPWeight",  &(runVideoCfg.vencStream[3].h264QpConf.qpPWeight), },
        {"adaptQp",    &(runVideoCfg.vencStream[3].h264QpConf.adaptQp),   },

        {NULL,},
    }
};
#endif

void VideoCfgPrint()
{
    int i;
    PRINT_INFO("********** sensors *********\n");
    CfgPrintMap((GK_CFG_MAP*)viMap);
    PRINT_INFO("********** sensors *********\n\n");
    
    PRINT_INFO("********** display *********\n");
    CfgPrintMap((GK_CFG_MAP*)voutMap);
    PRINT_INFO("********** display *********\n\n");
    
    PRINT_INFO("********** Video *********\n");
    for(i = 0; i < MAX_VENC_STREAM_NUM; i ++) {
        printf("stream%d:\n", i);
        CfgPrintMap(videoMap[i]);
        printf("\n");
    }
    PRINT_INFO("********** Video *********\n\n");
}

int VideoCfgSave()
{
    cJSON *root, *array, *item;
    char *out;
    int i;
    root = cJSON_CreateObject();//创建项目
    array = cJSON_CreateArray();
    for (i = 0; i < MAX_SENSOR_NUM&&viMap[i][0].defaultValue; i ++) {
        item = CfgDataToCjsonByMap(viMap[i]);
        cJSON_AddItemToArray(array, item);
    }
    cJSON_AddItemToObject(root, "sensors", array);
    
    array = cJSON_CreateArray();
    for (i = 0; i < MAX_VOUT_NUM&&voutMap[i][0].defaultValue; i ++) {
        item = CfgDataToCjsonByMap(voutMap[i]);
        cJSON_AddItemToArray(array, item);
    }
    cJSON_AddItemToObject(root, "display", array);
    
    array = cJSON_CreateArray();
    for (i = 0; i < MAX_VENC_STREAM_NUM; i ++) {
        item = CfgDataToCjsonByMap(videoMap[i]);
        cJSON_AddItemToArray(array, item);
    }
    cJSON_AddItemToObject(root, "venc stream", array);

    out = cJSON_Print(root);

    int ret = CfgWriteToFile(VIDEO_CFG_FILE, out);
    if (ret != 0) {
        PRINT_ERR("CfgWriteToFile %s error.", VIDEO_CFG_FILE);
        return -1;
    }

    free(out);
    cJSON_Delete(root);

    return 0;
}

int VideoCfgLoadDefValue()
{
    int i;
    for (i = 0; i < MAX_VENC_STREAM_NUM; i ++) {
        CfgLoadDefValue(videoMap[i]);
    }

    return 0;
}

int VideoCfgLoad()
{
    int load_def_config = 0;
    char *data = NULL;
    int index, arraySize;
    cJSON *array = NULL,*arrayItem = NULL;
start_load:
    data = NULL;
    data = CfgReadFromFile(VIDEO_CFG_FILE);
    if (data == NULL) {
        //从配置文件读取失败，则使用默认参数
        PRINT_INFO("load %s error, so to load default cfg param.\n", VIDEO_CFG_FILE);
        goto err;
    }

    cJSON *json = NULL;
    json = cJSON_Parse(data);
    if (!json){
        //从配置文件解析cjson失败，则使用默认参数
        PRINT_INFO("Error before: [%s]\n", cJSON_GetErrorPtr());
        free(data);
        goto err;
    }
    array = cJSON_GetObjectItem(json, "sensors");
    if(!array){
        PRINT_ERR("get sensors error\n");
        goto err1;
    }

	arraySize = cJSON_GetArraySize(array);
	//PRINT_INFO("sensors arraySize=%d\n", arraySize);

    for(index = 0; index < arraySize; index++) {
        arrayItem = cJSON_GetArrayItem(array, index);
        if(!arrayItem){
            PRINT_ERR("cJSON_GetArrayItem error\n");
            goto err1;
        }
        CfgCjsonToDataByMap(viMap[index], arrayItem);
    }
    
    array = cJSON_GetObjectItem(json, "display");
    if(!array){
        PRINT_ERR("get sensors error\n");
        goto err1;
    }

	arraySize = cJSON_GetArraySize(array);
	//PRINT_INFO("display arraySize=%d\n", arraySize);

    for(index = 0; index < arraySize; index++) {
        arrayItem = cJSON_GetArrayItem(array, index);
        if(!arrayItem){
            PRINT_ERR("cJSON_GetArrayItem error\n");
            goto err1;
        }
        CfgCjsonToDataByMap(voutMap[index], arrayItem);
    }

    array = cJSON_GetObjectItem(json, "venc stream");
    if(!array){
        PRINT_ERR("get venc stream error\n");
        goto err1;
    }

	arraySize = cJSON_GetArraySize(array);
	//PRINT_INFO("venc stream arraySize=%d\n", arraySize);

    for(index = 0; index < arraySize; index++) {
        arrayItem = cJSON_GetArrayItem(array, index);
        if(!arrayItem){
            PRINT_ERR("cJSON_GetArrayItem error\n");
            goto err1;
        }
        CfgCjsonToDataByMap(videoMap[index], arrayItem);
    }

    cJSON_Delete(json);
    free(data);
    //if(runVideoCfg.vencStream[0].h264Conf.bps > 10000)//maxbps
    if(videoMap[0][12].max > 10000)//maxbps
    {
        PRINT_ERR("video bps setting error,reset it\n");
        goto err1;
    }

    #ifdef MODULE_SUPPORT_MOJING_V4
    if (runVideoCfg.vencStream[0].enctype != runVideoCfg.vencStream[1].enctype)
    {
        //runVideoCfg.vencStream[1].enctype = runVideoCfg.vencStream[0].enctype;
    }
    #endif
    
    return 0;

err1:
    cJSON_Delete(json);
    free(data);
err:
    if(!load_def_config)
    {
        load_def_config = 1;
        char src_name[64], des_name[64];
        memset(src_name, 0, sizeof(src_name));
        snprintf(src_name,sizeof(src_name), "%s%s", DEFCFG_DIR, VIDEO_CFG_FILE);
        memset(des_name, 0, sizeof(des_name));
        snprintf(des_name,sizeof(des_name), "%s%s", CFG_DIR, VIDEO_CFG_FILE);
        if(!copy_file(src_name, des_name))
            goto start_load;
    }
    VideoCfgLoadDefValue();
    VideoCfgSave();
    return 0;

}

char* videoCfgLoadStreamJson( int streamId)
{
    cJSON *opt, *item,*streamJson;
    char *out = NULL;

    item = CfgDataToCjsonByMap(videoMap[streamId]);
    if(item == NULL)
    {
        return NULL;
    }
    opt =cJSON_Parse(((char*)runVideoCfg.vencStream[streamId].resolution));
    if(opt == NULL)
    {
        cJSON_Delete(item);
        return NULL;
    }
    streamJson = cJSON_GetObjectItem(opt,"opt");
    if(streamJson == NULL)
    {
        cJSON_Delete(item);
		return NULL;
    }

    cJSON_AddItemToObject(item, "resloution_opt", streamJson);

    out = cJSON_Print(item);
    cJSON_Delete(item);
    //PRINT_INFO("test out:\n%s\n",out);
    return out;
}
