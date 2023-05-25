/*!
*****************************************************************************
** \file        gk7101_enc.c
**
** \version
**
** \brief       sdk encode control
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2014-2015 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include <sys/types.h>
#include "sdk_api.h"
#include "sdk_enc.h"
#include "sdk.h"
#include "sdk_debug.h"
#include "sdk_osd.h"

//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************
#define GK7101_VENC_H264_REF_MODE   (1)
#define GK7101_VENC_MAX_ZOOM_FACTOR (10)
#define GK7101_VENC_PIP_THRESHOLD   (320)

#define GK7101_VENC_JPEG_MIN_WIDTH  (64)
#define GK7101_VENC_JPEG_MIN_HEIGHT (64)
#define GK7101_VENC_MJPEG_FPS       (1)

static unsigned int video_pts[GADI_VENC_STREAM_NUM];
static unsigned int audio_pts;
static int audio_frame_index = 0;

//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************
extern GADI_SYS_HandleT ispHandle;

GADI_SYS_HandleT viHandle    = NULL;
GADI_SYS_HandleT voutHandle  = NULL;
GADI_SYS_HandleT vencHandle  = NULL;
GADI_SYS_HandleT vpssHandle  = NULL;

static int venc_status = 0;
//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************
static ST_GK_VI_ATTR   *viParams   = NULL;
static ST_GK_VOUT_ATTR *voutParams = NULL;
static goke_encode_stream_params streamsPar[GADI_VENC_STREAM_NUM];

//static bool streamReadFlag = false;
static bool streamReadVideoFlag = false;
static bool streamReadAudioFlag = false;

/*capture JPEG flag.*/
static bool capJpegFlag = false;
static int  jpegSize = 0;
static char *jpegAddr = NULL;
static pthread_mutex_t snapshotMutex;
static int jpegCapSem;

/*overlay.*/
static EM_ENC_MAX_RES_MODE gencMaxRes = SENSOR_1920_1080P;
static int enc_init = 0;

static GK_ENC_BUG_CALLBACK encStreamCb = NULL;
static GK_ENC_BUG_CALLBACK getAudioStreamCb = NULL;

static void *abuf = NULL;
static int a_num = 0;

static pthread_t stream_v_pid;
static pthread_t stream_a_pid;

/*if audio is outputing, we can discard audio input*/
//extern int audio_outputing;
static audio_proc_callback aac_encoding_process = NULL;

//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************
static int check_encode_streams_resolution( GADI_U32 vinWidth, GADI_U32 vinHeight);
static int get_channels_params(GADI_VPSS_ChannelsParamsT* chansParams);
//static int get_actual_framerate(int fps);
static int setup_all_encode_streams(void);
static int start_all_encode_streams(void);
static int stop_all_encode_streams(void);
static int set_video_resolution(int streamid, int width, int height, int *fps, unsigned char encodeType);
//static void* enc_loop(void* arg);
static void* enc_video_loop(void* arg);
static void* enc_audio_loop(void* arg);

static int read_video_stream(void);
static int read_audio_stream();

static void cal_video_pts(unsigned int*pts, unsigned int ch);
static void cal_audio_pts(unsigned int*pts, unsigned int pkg_len);

//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************
int sdk_vi_set_params(ST_GK_VI_ATTR *vi)
{
    int retVal = GADI_OK;
    GADI_VI_SettingParamsT viParamsT;
    if(NULL == vi)
        return -1;
    if(NULL == viParams)
        viParams = vi;
    else if(viParams != vi && memcmp(viParams, vi, sizeof(ST_GK_VI_ATTR)))
        memcpy(viParams, vi, sizeof(ST_GK_VI_ATTR));
    viParamsT.width = vi->width;
    viParamsT.height= vi->height;
    viParamsT.fps   = vi->fps;
    viParamsT.pi    = vi->pi;
    viParamsT.mirrorPattern= vi->mirrorPattern;
    viParamsT.bayerPattern = vi->bayerPattern;

    LOG_INFO("vi, %dx%d@%d, pi:%d, bayer:%d, mirror:%d\n", viParamsT.width,viParamsT.height,viParamsT.fps, viParamsT.pi, viParamsT.bayerPattern, viParamsT.mirrorPattern);
    /*video input module: set video input frame rate and resolution.*/
//    retVal = gadi_vi_set_params(viHandle, &viParamsT);	xqq
    if(retVal != GADI_OK)
    {
        LOG_ERR("gadi_vin_set_params error\n");
    }
    return retVal;
}

int sdk_vi_get_params(ST_GK_VI_ATTR *vi)
{
    int retVal = GADI_OK;
    if(vi)
    {
        GADI_VI_SettingParamsT viParamsT;
        //retVal = gadi_vi_get_params(viHandle, &viParamsT);	xqq
        if(retVal != GADI_OK)
        {
            LOG_ERR("gadi_vi_get_params error\n");
            if(viParams)
                memcpy(vi, viParams, sizeof(ST_GK_VI_ATTR));
        }
        else
        {
            vi->width  = viParamsT.width;
            vi->height = viParamsT.height;
            vi->fps    = viParamsT.fps;
            vi->pi     = viParamsT.pi;
            vi->mirrorPattern= viParamsT.mirrorPattern;
            vi->bayerPattern = viParamsT.bayerPattern;
        }
        LOG_INFO("vi, %dx%d@%d, pi:%d, bayer:%d, mirror:%d\n", vi->width,vi->height,vi->fps, vi->pi, vi->bayerPattern, vi->mirrorPattern);
    }
    return retVal;
}

int sdk_vout_set_params(ST_GK_VOUT_ATTR *vout)
{
    int retVal = GADI_OK;
    GADI_VOUT_SettingParamsT voutParamsT;
    if(NULL == voutParams)
        voutParams = vout;
    else if(voutParams != vout && memcmp(voutParams, vout, sizeof(ST_GK_VOUT_ATTR)))
        memcpy(voutParams, vout, sizeof(ST_GK_VOUT_ATTR));
    voutParamsT.deviceType = vout->deviceType;
    voutParamsT.voutChannel = vout->voutChannel;
    voutParamsT.resoluMode = vout->resoluMode;

    /*video out module: set video output resolution and output device.*/
//    retVal = gadi_vout_set_params(voutHandle, &voutParamsT);	xqq
    if(retVal != GADI_OK)
    {
        LOG_ERR("gadi_vout_set_params error\n");
        return retVal;
    }
    return retVal;
}

int sdk_enc_init()
{
    int retVal = GADI_OK;
    GADI_VI_OpenParamsT viOpenParams;
    GADI_VENC_DspMapInfoT info;

    /*check parameters*/
    if(enc_init != 0)
    {
        LOG_ERR("gk_enc_init has been called.");
        return 0;
    }

    /*reset stream parameters.*/
	//gadi_sys_memset(&streamsPar, 0, sizeof(goke_encode_stream_params)*GADI_VENC_STREAM_NUM);	xqq

    /*init the snapshot mutex*/
    pthread_mutex_init(&snapshotMutex, NULL);

    /*create capture jpeg semaphore.*/
//    jpegCapSem = gadi_sys_sem_create(0);	xqq

    /*call gadi init & ipen of vi, vout, venc modules.*/
//    retVal = gadi_vi_init();
//    retVal = gadi_vout_init();
//    retVal = gadi_vpss_init();
//    retVal = gadi_venc_init();	xqq 4

    viOpenParams.channelId = GADI_VI_A;
    viOpenParams.sensorHWPath = NULL;
//    viHandle = gadi_vi_open(&viOpenParams, &retVal);	xqq
    if(retVal != GADI_OK)
    {
        LOG_ERR("gadi_vin_open error\n");
        return retVal;
    }

    /*open vout module.*/
//    voutHandle = gadi_vout_open(&retVal);	xqq
    if(retVal != GADI_OK)
    {
        LOG_ERR("gadi_vout_open error\n");
        return retVal;
    }

//    vpssHandle = gadi_vpss_open(&retVal);	xqq
    if(retVal != GADI_OK){
      LOG_ERR("gadi_vpss_open error\n");
      return retVal;
    }

    /*open venc module.*/
//    vencHandle = gadi_venc_open(&retVal);	xqq
    if(retVal != GADI_OK)
    {
        LOG_ERR("gadi_venc_open error\n");
        return retVal;
    }

//    retVal = gadi_venc_map_bsb(vencHandle);	xqq
    if(retVal != GADI_OK)
    {
        LOG_ERR("gadi_venc_map_bsb error\n");
        return retVal;
    }

//    retVal = gadi_venc_map_dsp(vencHandle,&info);	xqq
    if(retVal != GADI_OK)
    {
        LOG_ERR("gadi_venc_map_dsp error\n");
        return retVal;
    }

    /*video input module: ensure exit preview status.*/
//    retVal = gadi_vi_enable(viHandle, 0);	xqq
    if(retVal != GADI_OK)
    {
        LOG_ERR("gadi_vi_enable error\n");
        return retVal;
    }
    enc_init = 1;
    return retVal;
}

int sdk_set_get_video_stream_cb(GK_ENC_BUG_CALLBACK cb)
{
    if (cb)
        encStreamCb = cb;
    return 0;
}

int sdk_set_get_auido_stream_cb(GK_ENC_BUG_CALLBACK cb)
{
    if (cb)
        getAudioStreamCb = cb;
    return 0;
}

int sdk_enc_destroy()
{
    int retVal = GADI_OK;
    int i      = 0;

    /*check parameters*/
    if(enc_init == 0)
    {
        LOG_ERR("gk_enc_init has not been called.");
        return -1;
    }

    retVal = sdk_video_enc_stop();
    LOG_INFO("gk_enc_video_stop %s", retVal == GADI_OK ? "success" : "fail");

    retVal = sdk_audio_enc_stop();
    LOG_INFO("gk_enc_video_stop %s", retVal == GADI_OK ? "success" : "fail");

    enc_init = 0;

    /*destroy the snapshot mutex*/
    pthread_mutex_destroy(&snapshotMutex);

    for (i = 0; i < GADI_VENC_STREAM_NUM; i++) {
        //retVal = gadi_venc_stop_stream(vencHandle, i);	xqq
        if(retVal != GADI_OK) {
            LOG_ERR("gadi_venc_close error\n");
            return retVal;
        }
    }
#if 0
    retVal = gadi_venc_close(vencHandle);
    if(retVal != GADI_OK)
    {
        LOG_ERR("gadi_venc_close error\n");
        return retVal;
    }

    retVal = gadi_vpss_close(vpssHandle);
    if(retVal != GADI_OK)
    {
        LOG_ERR("gadi_venc_close error\n");
        return retVal;
    }

    retVal = gadi_vout_close(voutHandle);
    if(retVal != GADI_OK)
    {
        LOG_ERR("gadi_vout_close error\n");
        return retVal;
    }

    retVal = gadi_vi_close(viHandle);
    if(retVal != GADI_OK)
    {
        LOG_ERR("gadi_vin_close error\n");
        return retVal;
    }

    gadi_pda_exit();
    retVal = gadi_osd_exit();
    retVal = gadi_vi_exit();
    retVal = gadi_vout_exit();
    retVal = gadi_vpss_exit();
    retVal = gadi_venc_exit();
#endif		//xqq
    return retVal;
}

//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************

// ����һ·���룬ֻ�ڳ�ʼ��ʱ����һ��
int sdk_enc_create_stream_h264(int vin, int stream, PS_GK_ENC_STREAM_H264_ATTR h264_attr, ST_VENC_H264QP_CONF *qp_attr)
{
    goke_encode_stream_params* streamPar;
    int bpsLimit;

    /*check input parameters.*/
    if((h264_attr == NULL) ||
       (stream > GADI_VENC_STREAM_FORTH) ||
       (strlen(h264_attr->name) == 0))
    {
        LOG_ERR("gk_enc_create_stream_h264: invalid input parameters.");
        return -1;
    }

    LOG_INFO("create stream ID:%d, name:%s\n", stream, h264_attr->name);
    LOG_INFO("resolution:%dX%d, fps=%d, gop=%d\n", h264_attr->width, h264_attr->height, h264_attr->fps, h264_attr->gop);
    LOG_INFO("rc mode:%d, bps=%d\n", h264_attr->rc_mode, h264_attr->bps);

    /*check stream has been create or not by name.*/
    streamPar = &(streamsPar[stream]);
    if(strlen((streamPar->h264Attr.name)) > 0)
    {
        LOG_ERR("gk_enc_create_stream_h264: stream(id=%d) has been set.", stream);
        return -1;
    }
    memcpy(&streamPar->h264Attr, h264_attr, sizeof(ST_GK_ENC_STREAM_H264_ATTR));	
    /*set default value.*/
    streamPar->dptz = 0;
    streamPar->streamFormat.streamId = stream;
    streamPar->streamFormat.encodeType = h264_attr->enctype;

    /*set input parameters.*/
    streamPar->streamFormat.width  = h264_attr->width;
    streamPar->streamFormat.height = h264_attr->height;
    streamPar->streamFormat.fps    = h264_attr->fps; //get_actual_framerate(h264_attr->fps);

    bpsLimit = sdk_venc_bps_limit(h264_attr->width,h264_attr->height, h264_attr->fps,h264_attr->bps);
    switch(h264_attr->rc_mode)
    {
        /*fix me: our hardware has not below bitrate control mode.*/
        case GK_ENC_H264_RC_MODE_CBR_QUALITY:
            streamPar->streamFormat.stAttrH264.cbrAvgBps = bpsLimit*1000;
            streamPar->streamFormat.stAttrH264.vbrMinbps = bpsLimit*500;
            streamPar->streamFormat.stAttrH264.vbrMaxbps = bpsLimit*1000;
            streamPar->streamFormat.stAttrH264.brcMode   = GADI_VENC_CBR_QUALITY_MODE;
            break;
        case GK_ENC_H264_RC_MODE_VBR_QUALITY:
            streamPar->streamFormat.stAttrH264.cbrAvgBps = bpsLimit*1000;
            streamPar->streamFormat.stAttrH264.vbrMinbps = bpsLimit*500;
            streamPar->streamFormat.stAttrH264.vbrMaxbps = bpsLimit*1000;
            streamPar->streamFormat.stAttrH264.brcMode   = GADI_VENC_VBR_QUALITY_MODE;
            break;

        case GK_ENC_H264_RC_MODE_VBR:
            streamPar->streamFormat.stAttrH264.cbrAvgBps = bpsLimit*1000;
            streamPar->streamFormat.stAttrH264.vbrMinbps = bpsLimit*500;
            streamPar->streamFormat.stAttrH264.vbrMaxbps = bpsLimit*1000;
            streamPar->streamFormat.stAttrH264.brcMode   = GADI_VENC_VBR_MODE;
            break;
        default:
        case GK_ENC_H264_RC_MODE_CBR:
            streamPar->streamFormat.stAttrH264.cbrAvgBps = bpsLimit*1000;
            streamPar->streamFormat.stAttrH264.vbrMinbps = bpsLimit*500;
            streamPar->streamFormat.stAttrH264.vbrMaxbps = bpsLimit*1000;
            streamPar->streamFormat.stAttrH264.brcMode   = GADI_VENC_CBR_MODE;
            break;
    }

    LOG_INFO("Actual brc mode:%d, cbrbps:%d, vbrMinbps:%d, vbrMaxbps:%d, fps:%d\n",
           streamPar->streamFormat.stAttrH264.brcMode, streamPar->streamFormat.stAttrH264.cbrAvgBps,
           streamPar->streamFormat.stAttrH264.vbrMinbps, streamPar->streamFormat.stAttrH264.vbrMaxbps,
           streamPar->streamFormat.fps);

    //streamPar->streamFormat.stAttrH264.keepAspRat = 0;
    streamPar->streamFormat.stAttrH264.gopM = 1;
    streamPar->streamFormat.stAttrH264.gopN = h264_attr->gop;
    streamPar->streamFormat.stAttrH264.gopModel = 0;
    streamPar->streamFormat.stAttrH264.idrInterval = 1;
    streamPar->streamFormat.stAttrH264.profile = 0;
    streamPar->streamFormat.stAttrH264.flip = 0;


	if(h264_attr->quality > 3)
	{
		h264_attr->quality = 3;
	}
	if( h264_attr->quality < 0)
	{
		h264_attr->quality = 0;
	}

    //h264 qp
    streamPar->h264QpConf.streamId = stream;
    streamPar->h264QpConf.qpMinOnI = qp_attr->qpMinOnI;
    streamPar->h264QpConf.qpMaxOnI = qp_attr->qpMaxOnI;
    streamPar->h264QpConf.qpMinOnP = qp_attr->qpMinOnP;
    streamPar->h264QpConf.qpMaxOnP = qp_attr->qpMaxOnP;
    streamPar->h264QpConf.qpIWeight = qp_attr->qpIWeight;
    streamPar->h264QpConf.qpPWeight = qp_attr->qpPWeight;
    streamPar->h264QpConf.adaptQp = qp_attr->adaptQp; 

    LOG_INFO("stream:%d, encodeType:%d, %dX%d\n", streamPar->streamFormat.streamId, streamPar->streamFormat.encodeType, 
    streamPar->streamFormat.width, streamPar->streamFormat.height);

    return 0;
}

// �ر�һ·���룬ֻ�ڳ�ʼ��ʱ����һ��
int sdk_enc_release_stream_h264(int vin, int stream)
{
    goke_encode_stream_params* streamPar;

    /*check input parameters.*/
    if(stream > GADI_VENC_STREAM_FORTH)
    {
        LOG_ERR("gk_enc_release_stream_h264: invalid input parameters.");
        return -1;
    }
#if 0
    if(gadi_vi_enable(viHandle, 0) < 0)
    {
        LOG_ERR("gadi_vi_enable error.\n");
        return -1;
    }
#endif		//xqq
    /*check stream has been create or not by name.*/
    streamPar = &(streamsPar[stream]);
    if(strlen(streamPar->h264Attr.name) == 0)
    {
        LOG_ERR("gk_enc_release_stream_h264: stream(id=%d) has not been set.",
                 stream);
        return -1;
    }

	//gadi_sys_memset(streamPar, 0, sizeof(goke_encode_stream_params));	xqq

    return 0;

}
// ʹ��ÿ·���룬ֻ�ڳ�ʼ��ʱ����һ��
int sdk_enc_enable_stream_h264(int vin, int stream, int flag)
{
    static unsigned char setupCnt = 0;//workaround
    static bool setupFlag = false;
    unsigned char  encodeType, cnt;
    int startStreamNum = 0;

    /*check input parameters.*/
    if(stream > GADI_VENC_STREAM_FORTH)
    {
        LOG_ERR("enc_enable_stream: invalid input parameters.");
        return -1;
    }

    streamsPar[stream].enFlag = flag;
#if 0
    if(flag)
    {
        LOG_INFO("start stream[%d], stream num[%d]\n",stream, gk_get_max_stream_nums());
        if((setupCnt + 1) < gk_get_max_stream_nums())
        {
            /*wrokaround:we need to wait for setup all video channels.*/
            setupCnt++;
        }
        else
        {
            /*wrokaround:set all streams.*/
            if(setupFlag == false)
            {

                if(setup_all_encode_streams())
                {
                    LOG_ERR("setup streams failed.\n");
                    return -1;
                }


                /*start isp after set video channel.*/
                gk_isp_init_start();
                setupFlag = true;
            }

            /*check which stream has been enabled.*/
            for (cnt = 0; cnt < GADI_VENC_STREAM_NUM; cnt++)
            {
                encodeType = streamsPar[cnt].streamFormat.encodeType;
                if((encodeType != 0) && (streamsPar[cnt].enFlag))
                {
                    LOG_INFO("stream%d to call gadi_venc_start_stream.\n", cnt);
//                    if(gadi_venc_start_stream(vencHandle, cnt) < 0)
//                    {
//                        LOG_ERR("gadi_venc_start_stream error, streams:%d\n",
//                                 stream);
//                        return -1;
//                    }		xqq

                    startStreamNum ++;
                }

                //һֱ����mjeg����һ·���룬ͬʱץ�ĵ�ʱ��ͣ���룬��ֹosd����
                #if 0
                if((encodeType == 2) )
                {
                    LOG_INFO("stream%d to call gadi_venc_start_stream mjpeg.\n", cnt);
                    if(gadi_venc_start_stream(vencHandle, cnt) < 0)
                    {
                        LOG_ERR("gadi_venc_start_stream mjpeg error, streams:%d\n",
                                 stream);
                        return -1;
                    }

                    startStreamNum ++;
                }
				#endif

            }
        }
    }
    else
    {
        LOG_INFO("stop stream[%d]\n",stream);
#if 0
        if(gadi_venc_stop_stream(vencHandle, stream) < 0)
        {
            LOG_ERR("gadi_venc_stop_stream error, streams:%d\n", stream);
            return -1;
        }
#endif		//xqq
    }
#endif  //xqq
    return 0;
}

// ��̬�޸�H264�������
int sdk_enc_set_stream_h264(int vin, int stream, PS_GK_ENC_STREAM_H264_ATTR h264_attr)
{
    goke_encode_stream_params* streamPar;
	GADI_VENC_BitRateRangeT stBitRate;
    GADI_VENC_FrameRateT stFrameRate;
    int chage_fps = 0;
    int bpsLimit;
    /*�ñ�־�����Ƿ���Ҫ������������һ��ֻ�����÷ֱ���ʱ����Ҫ�������룬�������ʣ�fps������Ҫ��������*/
    //bool resetH264Flag = false;

    /*check input parameters.*/
    if((h264_attr == NULL) || (stream > GADI_VENC_STREAM_FORTH))
    {
        LOG_ERR("gk_enc_create_stream_h264: invalid input parameters.");
        return -1;
    }

    /*check stream has been create or not by name.*/
    streamPar = &(streamsPar[stream]);
    if(strlen(streamPar->h264Attr.name) == 0)
    {
        LOG_ERR("gk_enc_set_stream_h264: stream(id=%d) has not been set.", stream);
        return -1;
    }

    LOG_INFO("set stream parameters, ID:%d, name:%s, enctype:%d\n", stream, h264_attr->name, h264_attr->enctype);
    LOG_INFO("resolution:%dX%d, fps=%d, gop=%d\n", h264_attr->width, h264_attr->height, h264_attr->fps, h264_attr->gop);
    LOG_INFO("rc mode:%d, bps=%d\n", h264_attr->rc_mode,h264_attr->bps);

    /*check which parameters has be changed.*/
    /*bit rate control.*/
    if((streamPar->h264Attr.bps !=  h264_attr->bps) ||
       (streamPar->h264Attr.rc_mode != h264_attr->rc_mode))
    {
        bpsLimit = sdk_venc_bps_limit(h264_attr->width,h264_attr->height,
                                      h264_attr->fps,h264_attr->bps);

        switch(h264_attr->rc_mode)
        {
                /*fix me: our hardware has not below bitrate control mode.*/
            case GK_ENC_H264_RC_MODE_CBR_QUALITY:
                stBitRate.cbrAvgBps = bpsLimit*1000;
                stBitRate.vbrMinbps = bpsLimit*500;
                stBitRate.vbrMaxbps = bpsLimit*1000;
                stBitRate.brcMode   = GADI_VENC_CBR_QUALITY_MODE;
                break;
            case GK_ENC_H264_RC_MODE_VBR_QUALITY:
                stBitRate.cbrAvgBps = bpsLimit*1000;
                stBitRate.vbrMinbps = bpsLimit*500;
                stBitRate.vbrMaxbps = bpsLimit*1000;
                stBitRate.brcMode   = GADI_VENC_VBR_QUALITY_MODE;
                break;

            case GK_ENC_H264_RC_MODE_VBR:

                stBitRate.cbrAvgBps = bpsLimit*1000;
                stBitRate.vbrMinbps = bpsLimit*500;
                stBitRate.vbrMaxbps = bpsLimit*1000;
                stBitRate.brcMode   = GADI_VENC_VBR_MODE;
                break;

            default:
            case GK_ENC_H264_RC_MODE_CBR:
                stBitRate.cbrAvgBps = bpsLimit*1000;
                stBitRate.vbrMinbps = bpsLimit*500;
                stBitRate.vbrMaxbps = bpsLimit*1000;
                stBitRate.brcMode   = GADI_VENC_CBR_MODE;
                break;
        }

        stBitRate.streamId  = stream;
#if 0
        if(gadi_venc_set_bitrate(vencHandle, &stBitRate) < 0)
        {
            LOG_ERR("set bitrate failed\n");
            return -1;
        }
#endif		//xqq
        streamPar->streamFormat.stAttrH264.brcMode   = stBitRate.brcMode;
        streamPar->streamFormat.stAttrH264.cbrAvgBps = stBitRate.cbrAvgBps;
        streamPar->streamFormat.stAttrH264.vbrMinbps = stBitRate.vbrMinbps;
        streamPar->streamFormat.stAttrH264.vbrMaxbps = stBitRate.vbrMaxbps;

        #if 0
        //Bruce ���⿪һ������
        /* ���ñ�����ʵ�����޸�QPֵ�������Сֵ
                    ��ʱ��I֡�����ر����Ϊ��ƽ���޸�I֡��P֡
                    ��Ȩ��*/
        GADI_VENC_H264QpConfigT qp_cfg = {0};
        qp_cfg.streamId = stream;
        if (gadi_venc_get_h264_qp_config(vencHandle, &qp_cfg) < 0)
        {
            LOG_ERR("get h264 QP failed\n");
            return -1;
        }
        LOG_INFO("before set qp stream%d: I[%d-%d], P[%d-%d], weight[%d-%d]",
            stream, qp_cfg.qpMinOnI, qp_cfg.qpMaxOnI, qp_cfg.qpMinOnP,
            qp_cfg.qpMaxOnP, qp_cfg.qpIWeight, qp_cfg.qpPWeight, qp_cfg.adaptQp);

        qp_cfg.qpMinOnI = streamPar->h264QpConf.qpMinOnI;
        qp_cfg.qpMaxOnI = streamPar->h264QpConf.qpMaxOnI;
        qp_cfg.qpMinOnP = streamPar->h264QpConf.qpMinOnP;
        qp_cfg.qpMaxOnP = streamPar->h264QpConf.qpMaxOnP;
        qp_cfg.qpIWeight = streamPar->h264QpConf.qpIWeight;
        qp_cfg.qpPWeight = streamPar->h264QpConf.qpPWeight;
        qp_cfg.qpPWeight = streamPar->h264QpConf.qpPWeight;
        qp_cfg.adaptQp = streamPar->h264QpConf.adaptQp;
        LOG_INFO("after set qp stream%d: I[%d-%d], P[%d-%d], weight[%d-%d]",
            stream, qp_cfg.qpMinOnI, qp_cfg.qpMaxOnI, qp_cfg.qpMinOnP,
            qp_cfg.qpMaxOnP, qp_cfg.qpIWeight, qp_cfg.qpPWeight, qp_cfg.adaptQp);

        if (gadi_venc_set_h264_qp_config(vencHandle, &qp_cfg) < 0)
        {
            LOG_ERR("set h264 QP failed\n");
            return -1;
        }
        #endif
    }


    /*frame rate.*/
    if(streamPar->h264Attr.fps !=  h264_attr->fps)
    {
        GADI_VI_FrameRateEnumT vi_fps;
#if 0
        if(gadi_vi_get_framerate(viHandle, &vi_fps) != GADI_OK)
        {
            LOG_ERR("resume normal mode frome slowshutter failed.\n");
            return -1;
        }
#endif		//xqq
        if(h264_attr->fps > vi_fps)
            chage_fps = 1;
        else
        {
            LOG_INFO("Actual frame rate=%d\n", stFrameRate.fps);
            stFrameRate.streamId = stream;
            stFrameRate.fps = h264_attr->fps; //get_actual_framerate(h264_attr->fps);
#if 0
            if(gadi_venc_set_framerate(vencHandle, &stFrameRate) < 0)
            {
                LOG_ERR("set framerate failed\n");
                return -1;
            }
#endif		//xqq
            streamPar->streamFormat.fps = stFrameRate.fps;
        }
    }

    /*h264 gop.*/
    if(streamPar->h264Attr.gop != h264_attr->gop)
    {
        GADI_VENC_GopConfigT gop_cfg;
        gop_cfg.streamId = stream;
        gop_cfg.gopN = h264_attr->gop;

        LOG_INFO("Actual GOP=%d\n", gop_cfg.gopN);
#if 0
        if (gadi_venc_set_gop(vencHandle, &gop_cfg) < 0)
        {
            LOG_ERR("set H264 GOP failed\n");
            return -1;
        }
        if (gadi_venc_get_gop(vencHandle, &gop_cfg) < 0)
        {
            LOG_ERR("get H264 GOP failed\n");
            return -1;
        }
#endif		//xqq
        streamPar->streamFormat.stAttrH264.gopN = gop_cfg.gopN;
    }

    /*video resolution.*/
    if((streamPar->h264Attr.width  != h264_attr->width) ||
       (streamPar->h264Attr.height != h264_attr->height) ||
       (streamPar->h264Attr.enctype!= h264_attr->enctype) ||
       chage_fps)
    {
        chage_fps = 0;
        if(set_video_resolution(stream, h264_attr->width, h264_attr->height, &h264_attr->fps, h264_attr->enctype) < 0)
        {
            LOG_ERR("set_video_resolution error, streamId=%d\n", stream);
            return (-1);
        }
        /*reset cover after restart stream in goke platform.*/
        gk_vin_reset_cover();
    }

    //gadi_sys_memcpy(&streamPar->h264Attr, h264_attr, sizeof(ST_GK_ENC_STREAM_H264_ATTR));	xqq

    LOG_INFO("Actual brc mode:%d, cbrbps:%d, vbrMinbps:%d, vbrMaxbps:%d\n",
           streamPar->streamFormat.stAttrH264.brcMode, streamPar->streamFormat.stAttrH264.cbrAvgBps,
           streamPar->streamFormat.stAttrH264.vbrMinbps, streamPar->streamFormat.stAttrH264.vbrMaxbps);
    return 0;
}


/*��ȡ������ǿ�Ʋ���*/
int sdk_enc_get_stream_h264(int vin, int stream, PS_GK_ENC_STREAM_H264_ATTR h264_attr)
{
    goke_encode_stream_params* streamPar;

    LOG_INFO("gk_enc_get_stream_h264: streamId;%d", stream);

    /*check input parameters.*/
    if((h264_attr == NULL) || (stream > GADI_VENC_STREAM_FORTH))
    {
        LOG_ERR("gk_enc_create_stream_h264: invalid input parameters.");
        return -1;
    }

    /*check stream has been create or not by name.*/
    streamPar = &(streamsPar[stream]);
    if(strlen(streamPar->h264Attr.name) == 0)
    {
        LOG_ERR("gk_enc_get_stream_h264: stream(id=%d) has not been set.",
                 stream);
        return -1;
    }   

    memcpy(h264_attr, &streamPar->h264Attr, sizeof(ST_GK_ENC_STREAM_H264_ATTR));

    return 0;
}

/*�ñ�����ǿ�Ʋ���һ��I֡*/
int sdk_enc_request_stream_h264_keyframe(int vin, int stream)
{
	GADI_ERR  ret;
	//ret = gadi_venc_force_idr(vencHandle, stream);	xqq
	if(ret != GADI_OK)
	{
		LOG_ERR("Force stream_%d I Frame error\n",stream);
		ret = -1;
	}
	else
	{
		LOG_ERR("Force stream_%d I Frame OK\n",stream);
	}
	return ret;

}

/*��ʼ����Ƶ���Ķ�ȡ�߳�*/
/* ��Ƶ����Ƶ���룬��Ϊ�������㷨����Ƶ����Ƶ��������2���߳�Ҫ�ֿ� */
int sdk_video_enc_start(void)
{
	int ret = 0;
    streamReadVideoFlag = true;

	ret = pthread_create(&stream_v_pid, NULL, enc_video_loop, NULL);
    if(ret < 0)
    {
        LOG_ERR("AV encode do loop create video thread failed!\n");
    }

	return 0;
}

/*ֹͣ����Ƶ���Ķ�ȡ�߳�*/
int sdk_video_enc_stop(void)
{
    streamReadVideoFlag = false;
    pthread_join(stream_v_pid, NULL);

    return 0;
}

int sdk_audio_enc_start(void)
{
    int ret = 0;
    streamReadAudioFlag = true;

	ret = pthread_create(&stream_a_pid, NULL, enc_audio_loop, NULL);
    if(ret < 0)
    {
        LOG_ERR("AV encode do loop create audio thread failed!\n");
    }
    return 0;
}

int sdk_audio_enc_stop(void)
{
    streamReadAudioFlag = false;
    pthread_join(stream_a_pid, NULL);

    return 0;
}



/* ��ͼ���ܣ����ݴ���Ŀ���������������ӽ���ͨ���ϻ�ȡ���ݣ��������߲�������Сһ�£����Ӵ��ڸø߿��ı���ͨ���Ͻ�ȡ����ͼ��*/
int sdk_enc_snapshot(int vin, EM_GK_ENC_SNAPSHOT_QUALITY quality,
                                    ssize_t width, ssize_t height,  GK_ENC_SNAPSHOT_CALLBACK cb)
{
    int retVal;
    #if 0
    signed int jpegQuality = 0;
    unsigned char strFirChanId, strSecChanId;
    unsigned short  strFirWidth, strFirHeight, strSecWidth, strSecHeight;
    unsigned int capWidth, capHeight;
    GADI_VENC_StreamFormatT *streamFormat = NULL;
    GADI_VENC_StrMjpegAttrT *mjpegConf = NULL;
    #endif

    /*check input parameters.*/
    if( cb == NULL)
    {
        LOG_INFO("snapshot: input invalid parameters.\n");
    }

    pthread_mutex_lock(&snapshotMutex);
#if 0
    if(quality == GK_ENC_SNAPSHOT_QUALITY_HIGHEST)
    {
        jpegQuality = 99;
    }
    else if(quality == GK_ENC_SNAPSHOT_QUALITY_HIGH)
    {
        jpegQuality = 90;
    }
    else if(quality == GK_ENC_SNAPSHOT_QUALITY_MEDIUM)
    {
        jpegQuality = 80;
    }
    else if(quality == GK_ENC_SNAPSHOT_QUALITY_LOW)
    {
        jpegQuality = 50;
    }
    else
    {
        jpegQuality = 20;
    }


    {
        if(width == GK_ENC_SNAPSHOT_SIZE_MAX)
        {
            capWidth = streamsPar[0].streamFormat.width;
        }
        else if(width == GK_ENC_SNAPSHOT_SIZE_MIN)
        {
            capWidth = GK7101_VENC_JPEG_MIN_WIDTH;
        }
        else
        {
            capWidth = width;
        }

        if(height == GK_ENC_SNAPSHOT_SIZE_MAX)
        {
            capHeight = streamsPar[0].streamFormat.height;
        }

        else if(height == GK_ENC_SNAPSHOT_SIZE_MIN)
        {
            capHeight = GK7101_VENC_JPEG_MIN_HEIGHT;
        }
        else
        {
            capHeight = height;
        }
    }

    capWidth  = SDK_ALIGNED_LITTLE_ENDIAN(capWidth, 4);
    capHeight = SDK_ALIGNED_LITTLE_ENDIAN(capHeight, 4);

    LOG_INFO("enc snapshot: width:%d, height:%d, quality:%d\n",
              width, height, quality);


    /*init channel 4 for JPEG capture.*/
    streamFormat = &streamsPar[GADI_VENC_STREAM_FORTH].streamFormat;
    streamFormat->streamId   = GADI_VENC_STREAM_FORTH;
    streamFormat->encodeType = GADI_VENC_TYPE_MJPEG;
	//degnbiao edit,
    //streamFormat->flip = 0;
    streamFormat->keepAspRat = 0;
    streamFormat->width      = capWidth;
    streamFormat->height     = capHeight;
    streamFormat->xOffset    = 0;
    streamFormat->yOffset    = 0;
    streamFormat->fps        = GK7101_VENC_MJPEG_FPS;/*set 3 fps default.*/
    /*select channel buffer.*/
    strFirChanId = streamsPar[GADI_VENC_STREAM_FIRST].streamFormat.channelId;
    strSecChanId = streamsPar[GADI_VENC_STREAM_SECOND].streamFormat.channelId;
    strFirWidth  = streamsPar[GADI_VENC_STREAM_FIRST].streamFormat.width;
    strFirHeight = streamsPar[GADI_VENC_STREAM_FIRST].streamFormat.height;
    strSecWidth  = streamsPar[GADI_VENC_STREAM_SECOND].streamFormat.width;
    strSecHeight = streamsPar[GADI_VENC_STREAM_SECOND].streamFormat.height;

    /*stream0 resolution > stream1 resolution: so stream0 chanId > stream1 chanId*/
    /*resolution0 > resolution1, and the capW <= stream1W && capH <= stream1H*/
    /*we will use the stream1'S source buffer as the JPEG source buffer.*/
    if((strSecChanId > strFirChanId) &&
       ((strSecWidth >= capWidth) && (strSecHeight >= capHeight)))
    {
        /*it means the resolution of stream2 is larger than stream1.*/
        streamFormat->channelId = strSecChanId;

        /*capture the center picture.*/
        if(strSecWidth > capWidth)
        {
            streamFormat->xOffset = (strSecWidth - capWidth) / 2;
        }
        if(strSecHeight > capHeight)
        {
            streamFormat->yOffset = (strSecHeight - capHeight) / 2;
        }
    }
    else
    {
        streamFormat->channelId = strFirChanId;
        if(capWidth > strFirWidth)
        {
            streamFormat->width = strFirWidth;
        }
        if(capHeight > strFirHeight)
        {
            streamFormat->height = strFirHeight;
        }
    }
    if (gadi_venc_set_stream_format(vencHandle, streamFormat) < 0)
    {
        LOG_ERR("snapshot set mjpeg stream format failed.\n");
        pthread_mutex_unlock(&snapshotMutex);
        return -1;
    }

    mjpegConf = &streamsPar[GADI_VENC_STREAM_FORTH].mjpegConf;
    mjpegConf->quality = jpegQuality;
    mjpegConf->chromaFormat = 1;
    mjpegConf->streamId = GADI_VENC_STREAM_FORTH;
    if(gadi_venc_set_mjpeg_config(vencHandle, mjpegConf) < 0)
    {
        LOG_ERR("snapshot set mjpeg config failed.\n");
        pthread_mutex_unlock(&snapshotMutex);
        return -1;
    }

    /*start jpeg encoder.*/
//    if(gadi_venc_start_stream(vencHandle, GADI_VENC_STREAM_FORTH) < 0)
//    {
//        LOG_ERR("snapshot start MJPEG stream.\n");
//        pthread_mutex_unlock(&snapshotMutex);
//        return -1;
//    }		xqq
#endif
    /*start to wait jpeg data.*/
    capJpegFlag = true;
//    retVal = gadi_sys_sem_wait_timeout(jpegCapSem, 3000);	xqq
    if(retVal == GADI_SYS_ERR_TIMEOUT)
    {
        LOG_ERR("snapshot wait JPEG data timeout.\n");
    }
#if 0
    /*capture end, stop jpeg encoder.*/
    if(gadi_venc_stop_stream(vencHandle, GADI_VENC_STREAM_FORTH) < 0)
    {
        LOG_ERR("snapshot stop MJPEG stream.\n");
        pthread_mutex_unlock(&snapshotMutex);
        return -1;
    }
#endif
    if((jpegSize != 0) &&  (jpegAddr != NULL))
    {
       cb(jpegAddr,jpegSize); // application handle data
    }

    jpegSize = 0;
    jpegAddr = NULL;

    pthread_mutex_unlock(&snapshotMutex);

    return 0;
}

EM_ENC_MAX_RES_MODE sdk_enc_max_resolution_mode(void)
{
    return gencMaxRes;
}

int sdk_enc_get_stream_fps(int vin,int stream)
{
    if(stream > GADI_VENC_STREAM_FORTH)
    {
        LOG_ERR("Stream id %d error,use default fps 25",stream);
        return 25;
    }

    int fps = streamsPar[stream].streamFormat.fps;
    if(fps < 0)
    {
        LOG_ERR("Stream id %d fps(%d) < 0,use 25fps ",stream,fps);
        fps = 25;
    }
    else if( fps > 30)
    {
        LOG_ERR("Stream id %d fps(%d) > 30,use 30 fps ",stream,fps);
        fps = 30;
    }

    return fps;
}

int gk_enc_set_mirror(uint8_t enable)
{
	GADI_ERR retVal = GADI_OK;
	unsigned int cnt;
    GADI_VENC_StrAttr *stream;
    GADI_U8 *pflip;

	if(sdk_osd_get_mirror() == enable)
		return 0;

	stop_all_encode_streams();
	//gadi_vi_enable(viHandle, 0);	xqq
	for (cnt = 0; cnt < GADI_VENC_STREAM_NUM; cnt++)
    {
        stream = &(streamsPar[cnt].streamFormat);
        //retVal = gadi_venc_get_stream_attr(vencHandle, stream);	xqq
        if(retVal != GADI_OK)
        {
	        LOG_ERR("get venc filp parameters failed \n");
			continue;
		}
		/* set flip params */
        if (stream->encodeType == GADI_VENC_TYPE_H264)
        {
            pflip = &(stream->stAttrH264.flip);
        }
        else if (stream->encodeType == GADI_VENC_TYPE_MJPEG)
        {
            pflip = &(stream->stAttrMjpeg.flip);
        }
        else if (stream->encodeType == GADI_VENC_TYPE_H265)
        {
            pflip = &(stream->stAttrH265.flip);
        }
        else
            continue;

        if(enable)
        {
			*pflip |= 0x01;
		} else {
			*pflip &= ~0x01;
		}
		//retVal = gadi_venc_get_stream_attr(vencHandle, stream);	xqq
		if(retVal != GADI_OK)
        {
	        LOG_ERR("get venc filp parameters failed \n");
			continue;
		}
	}
	//gadi_vi_enable(viHandle, 1);	xqq
	start_all_encode_streams();
	gk_osd_set_mirror(enable);
	gk_vin_reset_cover();
	return GADI_OK;
}

int gk_enc_set_flip(uint8_t enable)
{
	GADI_ERR retVal = GADI_OK;
	unsigned int cnt;
    GADI_VENC_StrAttr *stream;
    GADI_U8 *pflip;

	if(sdk_osd_get_flip() == enable)
		return 0;

	stop_all_encode_streams();
	//gadi_vi_enable(viHandle, 0);	xqq
	for (cnt = 0; cnt < GADI_VENC_STREAM_NUM; cnt++)
    {
        stream = &(streamsPar[cnt].streamFormat);
//        retVal = gadi_venc_get_stream_attr(vencHandle, stream);	xqq
        if(retVal != GADI_OK)
        {
	        LOG_ERR("get venc filp parameters failed \n");
			continue;
		}
		/* set flip params */
        if (stream->encodeType == GADI_VENC_TYPE_H264)
        {
            pflip = &(stream->stAttrH264.flip);
        }
        else if (stream->encodeType == GADI_VENC_TYPE_MJPEG)
        {
            pflip = &(stream->stAttrMjpeg.flip);
        }
        else if (stream->encodeType == GADI_VENC_TYPE_H265)
        {
            pflip = &(stream->stAttrH265.flip);
        }
        else
            continue;

        if(enable)
        {
			*pflip |= 0x02;
		} else {
			*pflip &= ~0x02;
		}
//		retVal = gadi_venc_get_stream_attr(vencHandle, stream);	xqq
		if(retVal != GADI_OK)
        {
	        LOG_ERR("get venc filp parameters failed \n");
			continue;
		}
	}
	//gadi_vi_enable(viHandle, 1);	xqq
	start_all_encode_streams();
	gk_osd_set_flip(enable);
	gk_vin_reset_cover();
	return GADI_OK;
}

static int check_encode_streams_resolution( GADI_U32 vinWidth, GADI_U32 vinHeight)
{
    int retVal = 0;
    unsigned char   cnt;
    GADI_VENC_StrAttr *format = NULL;

    for(cnt=0; cnt<GADI_VENC_STREAM_NUM; cnt++)
    {
        format = &(streamsPar[cnt].streamFormat);
        if((format->width > vinWidth)  ||
           (format->height> vinHeight) ||
           (format->width + format->xOffset > vinWidth) ||
           (format->height + format->yOffset > vinHeight))
        {
            LOG_INFO("encode stream[%d] resolution:%dX%d,x offset:%d, y offset:%d is bigger than video input:%dX%d\n",
                      cnt, format->width, format->height, format->xOffset, format->yOffset, vinWidth, vinHeight);

            format->xOffset = 0;
            format->yOffset = 0;
            format->width   = vinWidth;
            format->height  = vinHeight;
        }

        /*video min width.*/
        if(format->width < GK7101_VENC_PIP_THRESHOLD)
        {
            format->width  = GK7101_VENC_PIP_THRESHOLD;
        }

        /*MAX zoom factor.*/
        if(format->width * GK7101_VENC_MAX_ZOOM_FACTOR < vinWidth)
        {
            format->width = vinWidth / GK7101_VENC_MAX_ZOOM_FACTOR;
        }
        if(format->height * GK7101_VENC_MAX_ZOOM_FACTOR < vinHeight)
        {
            format->height = vinHeight / GK7101_VENC_MAX_ZOOM_FACTOR;
        }

        /*height & width must aligned to 4.*/
		format->width = SDK_ALIGNED_LITTLE_ENDIAN(format->width, 4);
		format->height = SDK_ALIGNED_LITTLE_ENDIAN(format->height, 4);
    }

    return retVal;
}

static GADI_ERR calculate_encode_size_order(goke_encode_stream_params* strEncAttr, GADI_U32 *order, GADI_U32 streamsNum)
{
    GADI_U32 tmp, resolution[GADI_VENC_STREAM_NUM];
    GADI_VENC_StrAttr *format;
    GADI_U32 i, j;

    for (i = 0; i < streamsNum; ++i)
    {
        format = &(strEncAttr[i].streamFormat);
        order[i] = i;
        if (format->encodeType == GADI_VENC_TYPE_OFF)
            resolution[i] = 0;
        else
            resolution[i] = (format->width << 16) + format->height;
    }
    for (i = 0; i < streamsNum; ++i)
    {
        for (j = i + 1; j < streamsNum; ++j)
        {
            if (resolution[i] < resolution[j])
            {
                tmp = resolution[j];
                resolution[j] = resolution[i];
                resolution[i] = tmp;
                tmp = order[j];
                order[j] = order[i];
                order[i] = tmp;
            }
        }
    }

    LOG_INFO("resolution order of streams : ");
    for (i = 0; i < streamsNum; ++i)
    {
        format = &(strEncAttr[i].streamFormat);
        LOG_INFO("%d<%d>, encodeType:%d,%dX%d@%d\n", order[i], format->streamId, format->encodeType, format->width, format->height, format->fps);
    }

    return GADI_OK;
}

static void get_vout_resolution(GADI_VOUT_ResolutionModeEnumT resoluMode,
            GADI_U32* width, GADI_U32* height)
{
    if (GADI_VOUT_RESOLUTION_480I == resoluMode)
    {
        *width = 720;
        *height = 480;
    }
    else if (GADI_VOUT_RESOLUTION_576I == resoluMode)
    {
        *width = 720;
        *height = 576;
    }
    else if (GADI_VOUT_RESOLUTION_720P_PAL == resoluMode)
    {
        *width = 1280;
        *height = 720;
    }
    else if (GADI_VOUT_RESOLUTION_720P30 == resoluMode)
    {
        *width = 1280;
        *height = 720;
    }
    else if (GADI_VOUT_RESOLUTION_WVGA == resoluMode)
    {
        *width = 800;
        *height = 480;
    }
    else if (GADI_VOUT_RESOLUTION_D1_NTSC == resoluMode)
    {
        *width = 720;
        *height = 480;
    }
    else if (GADI_VOUT_RESOLUTION_D1_PAL == resoluMode)
    {
        *width = 720;
        *height = 576;
    }
    else if (GADI_VOUT_RESOLUTION_720P60 == resoluMode)
    {
        *width = 1280;
        *height = 720;
    }
    else if (GADI_VOUT_RESOLUTION_1080I50 == resoluMode)
    {
        *width = 1920;
        *height = 1080;
    }
    else if (GADI_VOUT_RESOLUTION_1080I60 == resoluMode)
    {
        *width = 1920;
        *height = 1080;
    }
    else if (GADI_VOUT_RESOLUTION_HDTV == resoluMode)
    {
        *width = 1920;
        *height = 1080;
    }
    else if (GADI_VOUT_RESOLUTION_1080P30 == resoluMode)
    {
        *width = 1920;
        *height = 1080;
    }
    else if (GADI_VOUT_RESOLUTION_720P25 == resoluMode)
    {
        *width = 1280;
        *height = 720;
    }
    else
    {
        *width = 0;
        *height = 0;
    	LOG_ERR("get_vout_resolution error resolution\n");
    }

    return;
}

static int set_one_channel_params(GADI_VPSS_ChannelsParamsT* chansParams,
                                    GADI_VPSS_ChannelEnumT channel,
                                    GADI_U32 type,
                                    GADI_U32 width,
                                    GADI_U32 height)
{
    //LOG_INFO("channel:%d type:%d %dx%d\n", channel, type, width, height);
    if (GADI_VPSS_CHANNEL_MAIN_A == channel)
    {
        chansParams->chanMainAType       = GADI_VPSS_CHANNEL_TYPE_ENCODE;
        chansParams->chanMainAWidth      = width;
        chansParams->chanMainAHeight     = height;
        chansParams->chanMainAKeepAspRat = 0;
    }
    else if (GADI_VPSS_CHANNEL_SUB_A == channel)
    {
        if (height > 1280)
        {
            LOG_ERR("set_channel_params size error!\n");
            return -1;
        }
        chansParams->chanSubAType       = type;
        chansParams->chanSubAWidth      = width;
        chansParams->chanSubAHeight     = height;
        chansParams->chanSubASourceFrom = GADI_VPSS_CHANNEL_MAIN_A;
        chansParams->chanSubAKeepAspRat = 0;
    }
    else if (GADI_VPSS_CHANNEL_SUB_B == channel)
    {
        if (height > 1920)
        {
            LOG_ERR("set_channel_params size error!\n");
            return -1;
        }
        chansParams->chanSubBType       = type;
        chansParams->chanSubBWidth      = width;
        chansParams->chanSubBHeight     = height;
        chansParams->chanSubBSourceFrom = GADI_VPSS_CHANNEL_MAIN_A;
        chansParams->chanSubBKeepAspRat = 0;
    }
    else if (GADI_VPSS_CHANNEL_SUB_C == channel)
    {
        if (height > 1080)
        {
            LOG_ERR("set_channel_params size error!\n");
            return -1;
        }
        chansParams->chanSubCType       = type;
        chansParams->chanSubCWidth      = width;
        chansParams->chanSubCHeight     = height;
        chansParams->chanSubCSourceFrom = GADI_VPSS_CHANNEL_MAIN_A;
        chansParams->chanSubCKeepAspRat = 0;
    }

    return 0;
}

static GADI_ERR get_channels_params(GADI_VPSS_ChannelsParamsT* chansParams)
{
    GADI_ERR retVal = GADI_OK;
    GADI_U32 order[GADI_VENC_STREAM_NUM];
    GADI_U32 idx = 0;
    GADI_U32 orderidx = 0;
    GADI_U32 vpssChanIdx = 0;
    GADI_U32 voWidth = 0;
    GADI_U32 voHeight = 0;
    GADI_U16 chanWidth = 0;
    GADI_U16 chanHeight = 0;
    GADI_VOUT_SettingParamsT voParams;
    GADI_VPSS_ChannelEnumT vpssChannel = GADI_VPSS_CHANNEL_SUB_A;
    GADI_VENC_StrAttr *stream;

	voParams.voutChannel = GADI_VOUT_A;
//    retVal = gadi_vout_get_params(voutHandle, &voParams);	xqq
    if (GADI_OK != retVal)
  	{
    	LOG_ERR("get_channels_params vout get params error:%d!\n", retVal);
		return retVal;
	}

	if  ((GADI_VOUT_DEVICE_BT1120 == voParams.deviceType) ||
		(GADI_VOUT_DEVICE_RGB == voParams.deviceType) ||
		(GADI_VOUT_DEVICE_I80 == voParams.deviceType))
	{
        get_vout_resolution(voParams.resoluMode, &voWidth, &voHeight);
	}

    if(chansParams == NULL)
    {
        return GADI_ERR_BAD_PARAMETER;
    }

    calculate_encode_size_order(streamsPar, order, GADI_VENC_STREAM_NUM);
    memset(chansParams, 0, sizeof(GADI_VPSS_ChannelsParamsT));

    for (idx = 0; idx < GADI_VENC_STREAM_NUM; idx++)
    {
        orderidx = order[idx];
        stream = &(streamsPar[orderidx].streamFormat);
        chanWidth = stream->width + stream->xOffset;
        chanHeight = stream->height + stream->yOffset;
        if ((0 == stream->width) || (0 == stream->height))
        {
            stream->encodeType = GADI_VPSS_CHANNEL_TYPE_OFF;
        }

        if (GADI_VPSS_CHANNEL_TYPE_OFF == stream->encodeType)
        {
            continue;
        }

        if (chansParams->chanMainAHeight == chanHeight)
        {
            stream->vpssChanId = GADI_VPSS_CHANNEL_MAIN_A;
            continue;
        }
        else if (chansParams->chanSubAHeight == chanHeight)
        {
            stream->vpssChanId = GADI_VPSS_CHANNEL_SUB_A;
            continue;
        }
        else if (chansParams->chanSubBHeight == chanHeight)
        {
            stream->vpssChanId = GADI_VPSS_CHANNEL_SUB_B;
            continue;
        }
        else if (chansParams->chanSubCHeight == chanHeight)
        {
            stream->vpssChanId = GADI_VPSS_CHANNEL_SUB_C;
            continue;
        }

        if (0 == vpssChanIdx)
        {
            vpssChannel = GADI_VPSS_CHANNEL_MAIN_A;
        }
        #ifdef MODULE_SUPPORT_LARGE_SECOND_STREAM
        else if (1 == vpssChanIdx)
        {
            vpssChannel = GADI_VPSS_CHANNEL_SUB_C;//GADI_VPSS_CHANNEL_SUB_A
        }
        else if (2 == vpssChanIdx)
        {
            vpssChannel = GADI_VPSS_CHANNEL_SUB_A;//GADI_VPSS_CHANNEL_SUB_B
        }
        else if (3 == vpssChanIdx)
        {
            vpssChannel = GADI_VPSS_CHANNEL_SUB_B;
        }
        #else
        else if (1 == vpssChanIdx)
        {
            vpssChannel = GADI_VPSS_CHANNEL_SUB_A;//GADI_VPSS_CHANNEL_SUB_A
        }
        else if (2 == vpssChanIdx)
        {
            vpssChannel = GADI_VPSS_CHANNEL_SUB_B;//GADI_VPSS_CHANNEL_SUB_B
        }
        else if (3 == vpssChanIdx)
        {
            vpssChannel = GADI_VPSS_CHANNEL_SUB_C;
        }
        #endif
        else
        {
            continue;
        }
        retVal = set_one_channel_params(chansParams,
                                vpssChannel,
                                GADI_VPSS_CHANNEL_TYPE_ENCODE,
                                chanWidth,
                                chanHeight);
        if (0 == retVal)
        {
            stream->vpssChanId = vpssChannel;
            vpssChanIdx++;
        }
        else
            return retVal;
    }

    if ((vpssChanIdx < GADI_VENC_STREAM_NUM) && (0 != voHeight))
    {
        retVal = set_one_channel_params(chansParams,
                        GADI_VPSS_CHANNEL_SUB_C,
                        GADI_VPSS_CHANNEL_TYPE_VOUT,
                        voWidth,
                        voHeight);
    }
    return retVal;
}

void print_channels_params(GADI_VPSS_ChannelsParamsT *chansParams)
{
    LOG_INFO("chansParams:\n");
    printf("chanMainAType: %d\n", chansParams->chanMainAType);
    printf("chanMainAWidth: %d\n", chansParams->chanMainAWidth);
    printf("chanMainAHeight: %d\n", chansParams->chanMainAHeight);
    printf("chanMainBType: %d\n", chansParams->chanMainBType);
    printf("chanMainBWidth: %d\n", chansParams->chanMainBWidth);
    printf("chanMainBHeight: %d\n", chansParams->chanMainBHeight);
    printf("chanSubAType: %d\n", chansParams->chanSubAType);
    printf("chanSubAWidth: %d\n", chansParams->chanSubAWidth);
    printf("chanSubAHeight: %d\n", chansParams->chanSubAHeight);
    printf("chanSubBSourceFrom: %d\n", chansParams->chanSubASourceFrom);
    printf("chanSubBType: %d\n", chansParams->chanSubBType);
    printf("chanSubBWidth: %d\n", chansParams->chanSubBWidth);
    printf("chanSubBHeight: %d\n", chansParams->chanSubBHeight);
    printf("chanSubBSourceFrom: %d\n", chansParams->chanSubBSourceFrom);
    printf("chanSubCType: %d\n", chansParams->chanSubCType);
    printf("chanSubCWidth: %d\n", chansParams->chanSubCWidth);
    printf("chanSubCHeight: %d\n", chansParams->chanSubCHeight);
    printf("chanSubCSourceFrom: %d\n", chansParams->chanSubCSourceFrom);
}

#if 0
/* ���ݴ����֡�����¼����Ӳ��֧�ֵ�֡��*/
static int get_actual_framerate(int fps)
{
    int actualFps;
    /*gk7101 support below fps value: GADI_VENC_FrameRateEnumT*/

    if(fps > 30)
    {
        fps = 30;
    }

    switch(fps)
    {
        case 7:
            actualFps = GADI_VENC_FPS_7_5;
            break;
        case 8:
        case 9:
        case 11:
            actualFps = GADI_VENC_FPS_10;
            break;
        case 16:
        case 17:
            actualFps = GADI_VENC_FPS_15;
            break;
        case 18:
            actualFps = GADI_VENC_FPS_18;
            break;
        case 19:
        case 21:
        case 22:
            actualFps = GADI_VENC_FPS_20;
            break;
        case 23:
            actualFps = GADI_VENC_FPS_24;
            break;
        case 26:
        case 27:
            actualFps = GADI_VENC_FPS_25;
            break;
        case 28:
        case 29:
            actualFps = GADI_VENC_FPS_30;
            break;
        default:
            actualFps = fps;
            break;
    }

    return actualFps;
}
#endif

/*�������еı������*/
static int setup_all_encode_streams(void)
{
    int retVal;
    unsigned int vinWidth, vinHeight;
    unsigned char  encodeType, cnt;
    GADI_VPSS_ChannelsParamsT chansParams;
    GADI_VENC_StrAttr strAttr;
    GADI_VENC_StrAttr *stream = NULL;
    GADI_VPSS_OpModeEnumT opMode;
#ifdef MODULE_SUPPORT_SUB_STREAM_2_BUF
    GADI_VPSS_SubChanlBufNumT subBufNum;
#endif

    /*init channel 4 for JPEG capture.*/
    streamsPar[GADI_VENC_STREAM_FORTH].enFlag = 1;
    stream = &(streamsPar[GADI_VENC_STREAM_FORTH].streamFormat);
    stream->streamId   = GADI_VENC_STREAM_FORTH;
    stream->encodeType = GADI_VENC_TYPE_MJPEG;
    stream->vpssChanId = streamsPar[GADI_VENC_STREAM_FIRST].streamFormat.vpssChanId;
    stream->width      = streamsPar[GADI_VENC_STREAM_FIRST].streamFormat.width;
    stream->height     = streamsPar[GADI_VENC_STREAM_FIRST].streamFormat.height;
    stream->xOffset    = streamsPar[GADI_VENC_STREAM_FIRST].streamFormat.xOffset;
    stream->yOffset    = streamsPar[GADI_VENC_STREAM_FIRST].streamFormat.yOffset;
    stream->fps        = GK7101_VENC_MJPEG_FPS;
    stream->stAttrMjpeg.quality      = 50;
    stream->stAttrMjpeg.chromaFormat = 1;
    stream->stAttrMjpeg.flip= 0;

    /*check encode streams parameters.*/
//    retVal = gadi_vi_get_resolution(viHandle, &vinWidth, &vinHeight);	xqq
    if(retVal != GADI_OK)
    {
        LOG_ERR("gadi_vin_get_resolution error\n");
        return retVal;
    }
    retVal = check_encode_streams_resolution(vinWidth, vinHeight);
    if(retVal != GADI_OK)
    {
        LOG_ERR("check_encode_streams_resolution error\n");
        return retVal;
    }

    /*get channels paramers.*/
    retVal = get_channels_params(&chansParams);
    if(retVal != GADI_OK)
    {
        LOG_ERR("get_channels_params error\n");
        return retVal;
    }

    //LOG_INFO("print_channels_params\n");
    //print_channels_params(&chansParams);

    if ((vinWidth <= 1920) &&
        (chansParams.chanMainAWidth <= 1920))
    {
        opMode = GADI_VPSS_OP_MODE_REAL_TIME;
    }
    else
    {
        #if 1
        if(1==streamsPar[GADI_VENC_STREAM_FIRST].streamFormat.encodeType)
            opMode = GADI_VPSS_OP_MODE_NON_REAL_TIME0;//GADI_VPSS_OP_MODE_NON_REAL_TIME0
        else
            opMode = GADI_VPSS_OP_MODE_NON_REAL_TIME2;//GADI_VPSS_OP_MODE_NON_REAL_TIME2;
        #else
        opMode = GADI_VPSS_OP_MODE_NON_REAL_TIME3;
        #endif
    }
    #if 0
    opMode = GADI_VPSS_OP_MODE_NON_REAL_TIME0;
	#endif
    //retVal = gadi_vpss_set_op_mode(vpssHandle, opMode);	xqq
    if(retVal != GADI_OK)
    {
        LOG_ERR("gadi_vpss_set_op_mode failed!\n");
        return -1;
    }

    /*set channels parameters.*/
//    retVal = gadi_vpss_set_channels_params(vpssHandle, &chansParams);	xqq
    if(retVal != GADI_OK)
    {
        LOG_ERR("set channels parameters error.\n");
        return retVal;
    }

    
    //Ĭ��3��buffer������Ϊ2��
    #ifdef MODULE_SUPPORT_SUB_STREAM_2_BUF
    subBufNum.chanSuaBufNum = 2;
    subBufNum.chanSubBufNum = 2;
    subBufNum.chanSucBufNum = 2;
    gadi_vpss_set_sub_channel_buf_num(vpssHandle, &subBufNum);
    printf("gadi_vpss_set_sub_channel_buf_num:%d\n", subBufNum.chanSucBufNum);
    #endif

    
    /*set all encode stream channel.*/
    for (cnt = 0; cnt < GADI_VENC_STREAM_NUM; cnt++)
    {
        stream = &(streamsPar[cnt].streamFormat);
        encodeType = stream->encodeType;
        strAttr.streamId = cnt;

        if(encodeType != 0)
        {
#if 0
            if (gadi_venc_set_stream_attr(vencHandle, stream) < 0)
            {
                LOG_ERR("gadi_venc_set_stream_format error, steamId=%d\n",cnt);
                return -1;
            }
            if (gadi_venc_get_stream_attr(vencHandle, &strAttr) != GADI_OK)
            {
                LOG_ERR("gadi_venc_get_stream_attr fail\n");
                return -1;
            }
#endif		//xqq
            stream->fps = strAttr.fps;
            LOG_INFO("stream%d, vpssChanId:%d, encodeType:%d, %dx%d@%d\n",
                strAttr.streamId, strAttr.vpssChanId, strAttr.encodeType, strAttr.width, strAttr.height, strAttr.fps);
            if(encodeType == GADI_VENC_TYPE_H264)
            {
                GADI_VENC_BitRateRangeT stBitRate;

                stBitRate.streamId  = cnt;
                stBitRate.brcMode   = stream->stAttrH264.brcMode;
                stBitRate.cbrAvgBps = stream->stAttrH264.cbrAvgBps;
                stBitRate.vbrMaxbps = stream->stAttrH264.vbrMaxbps;
                stBitRate.vbrMinbps = stream->stAttrH264.vbrMinbps;
#if 0
                if(gadi_venc_set_bitrate(vencHandle, &stBitRate) < 0)
                {
                    LOG_ERR("set bitrate failed\n");
                    return -1;
                }
#endif		//xqq
                LOG_INFO("set qp, stream%d\n", cnt);
                #if 1
                GADI_VENC_H264QpConfigT qp_cfg = {0};
                qp_cfg.streamId = cnt;
#if 0
                if (gadi_venc_get_h264_qp_config(vencHandle, &qp_cfg) < 0)
                {
                    LOG_ERR("get h264 QP failed\n");
                    return -1;
                }
#endif 		//xqq
                LOG_INFO("before set qp stream%d: I[%d-%d], P[%d-%d], weight[%d-%d], adaptQp:%d\n",
                    cnt, qp_cfg.qpMinOnI, qp_cfg.qpMaxOnI, qp_cfg.qpMinOnP,
                    qp_cfg.qpMaxOnP, qp_cfg.qpIWeight, qp_cfg.qpPWeight, qp_cfg.adaptQp);
#if 0
                qp_cfg.qpMinOnI = streamsPar[cnt].h264QpConf.qpMinOnI;
                qp_cfg.qpMaxOnI = streamsPar[cnt].h264QpConf.qpMaxOnI;
                qp_cfg.qpMinOnP = streamsPar[cnt].h264QpConf.qpMinOnP;
                qp_cfg.qpMaxOnP = streamsPar[cnt].h264QpConf.qpMaxOnP;
                qp_cfg.qpIWeight = streamsPar[cnt].h264QpConf.qpIWeight;
                qp_cfg.qpPWeight = streamsPar[cnt].h264QpConf.qpPWeight;
                qp_cfg.adaptQp = streamsPar[cnt].h264QpConf.adaptQp;
                LOG_INFO("after set qp stream%d: I[%d-%d], P[%d-%d], weight[%d-%d], adaptQp:%d\n",
                    cnt, qp_cfg.qpMinOnI, qp_cfg.qpMaxOnI, qp_cfg.qpMinOnP,
                    qp_cfg.qpMaxOnP, qp_cfg.qpIWeight, qp_cfg.qpPWeight, qp_cfg.adaptQp);

                if (gadi_venc_set_h264_qp_config(vencHandle, &qp_cfg) < 0)
                {
                    LOG_ERR("set h264 QP failed\n");
                    return -1;
                }

                memset(&qp_cfg, 0, sizeof(GADI_VENC_H264QpConfigT));
                qp_cfg.streamId = cnt;
                if (gadi_venc_get_h264_qp_config(vencHandle, &qp_cfg) < 0)
                {
                    LOG_ERR("get h264 QP failed\n");
                    return -1;
                }
#endif
                LOG_INFO("after2 set qp stream%d: I[%d-%d], P[%d-%d], weight[%d-%d], adaptQp:%d\n", 
                    cnt, qp_cfg.qpMinOnI, qp_cfg.qpMaxOnI, qp_cfg.qpMinOnP, 
                    qp_cfg.qpMaxOnP, qp_cfg.qpIWeight, qp_cfg.qpPWeight, qp_cfg.adaptQp);

                #endif

                if (streamsPar[cnt].h264Attr.re_rec_mode == 1)
                {
                    LOG_INFO("re_rec is opened\n");
                    #if 0
                    //SDK2.0�������α��빦�ܣ��Ȼ�ȡ������
                    GADI_VENC_ReEncConfigT reencConfig;
                    reencConfig.streamId = cnt;
                    if(gadi_venc_get_h264_reenc(vencHandle,&reencConfig) < 0)
                    {
                        LOG_ERR("gadi_venc_get_h264_reenc error, steamId=%d\n",cnt);
                        return -1;
                    }

                    reencConfig.threshStC   = 3;
                    reencConfig.strengthStC = 2;
                    reencConfig.threshCtS   = 0;
                    reencConfig.strengthCtS = 0;
                    if(gadi_venc_set_h264_reenc(vencHandle,&reencConfig) < 0)
                    {
                        LOG_ERR("gadi_venc_set_h264_reenc error, steamId=%d\n",cnt);
                        return -1;
                    }

                    //bcr, I frame ��ܴ�
                    //sdk2.0�������ܣ�I֡��С����
                    GADI_VENC_IsizeCtlT isizeCtl;
                    isizeCtl.streamId = cnt;
                    if(gadi_venc_get_h264_iframe_size(vencHandle,&isizeCtl) < 0)
                    {
                        LOG_ERR("gadi_venc_get_h264_iframe_size error, steamId=%d\n",cnt);
                        return -1;
                    }

                    isizeCtl.IsizeCtlStrength = 3;
                    isizeCtl.IsizeCtlThresh = 4;
                    if(gadi_venc_set_h264_iframe_size(vencHandle,&isizeCtl) < 0)
                    {
                        LOG_ERR("gadi_venc_set_h264_iframe_size error, steamId=%d\n",cnt);
                        return -1;
                    }

                    //sdk2.0�������ܣ�bias����
                    GADI_VENC_BiasConfigT biasConfigPar;
                    biasConfigPar.streamId = cnt;
                    if(gadi_venc_get_h264_bias(vencHandle, &biasConfigPar) < 0)
                    {
                        LOG_ERR("gadi_venc_get_h264_bias error, steamId=%d\n",cnt);
                        return -1;
                    }
                    biasConfigPar.skipChance = 4;
                    if(gadi_venc_set_h264_bias(vencHandle, &biasConfigPar) < 0)
                    {
                        LOG_ERR("gadi_venc_get_h264_bias error, steamId=%d\n",cnt);
                        return -1;
                    }
                    #endif
                }
                else
                {
					LOG_INFO("re_rec is closed\n");
                    #if 0
					if (gadi_venc_set_buf_mode(vencHandle, GADI_VENC_BUF_MERGE_MODE_1) < 0) {
                    	LOG_ERR("gadi_venc_set_buf_mode failed, steamId=%d\n",cnt);
                   	 	return -1;
					}
                    #endif
				}
            }
        }
    }
    

    /*��ʼvi����*/
#if 0
    if(gadi_vi_enable(viHandle, 1) < 0)
    {
        LOG_ERR("gadi_vi_enable error.\n");
        return -1;
    }
#endif 	//xqq
    //LOG_INFO("print vencHandle:\n");
    //gadi_venc_print_params(vencHandle);
    return 0;
}

static int start_all_encode_streams(void)
{
    int retVal = GADI_OK;
    unsigned int  cnt;
    GADI_CHN_AttrT stChnAttr;
    int startStreamNum = 0;

    LOG_INFO("start_all_encode_streams.\n");

    /*start all streams encoding*/
    for (cnt = 0; cnt < GADI_VENC_STREAM_NUM; cnt++)
    {
        //gadi_sys_memset(&stChnAttr, 0, sizeof(stChnAttr));	xqq
//        if(gadi_venc_query(vencHandle,cnt,&stChnAttr) != GADI_OK)
//        {
//            continue;
//        }	xqq
        /*only start H264 stream.*/
        if(((stChnAttr.type == GADI_VENC_TYPE_H264) ||
            (stChnAttr.type == GADI_VENC_TYPE_MJPEG) ||
            (stChnAttr.type == GADI_VENC_TYPE_H265)) &&
           (stChnAttr.state != GADI_VENC_STREAM_STATE_ENCODING))
        {
            LOG_INFO("start stream[%d], type:%d\n", cnt, stChnAttr.type);
//            if(gadi_venc_start_stream(vencHandle, cnt) < 0)
//            {
//                LOG_ERR("gadi_venc_start_stream error, streams:%d\n", cnt);
//                return -1;
//            }	xqq
            startStreamNum++;
        }

    }
    // ���뿪�غ���Ҫ����3A������3A�޷���������
    //gadi_isp_reset_3a_static(ispHandle);	xqq
    return retVal;

}

static int stop_all_encode_streams(void)
{
    int retVal =  GADI_OK;
    unsigned int cnt;
    GADI_CHN_AttrT stChnAttr;

    LOG_INFO("stop_all_encode_streams.\n");

    /*stop all streams encoding*/
    for (cnt = 0; cnt < GADI_VENC_STREAM_NUM; cnt++)
    {
#if 0
        gadi_sys_memset(&stChnAttr, 0, sizeof(stChnAttr));
        if(gadi_venc_query(vencHandle, cnt, &stChnAttr) != GADI_OK)
        {
            continue;
        }
#endif	//xqq
        if(((stChnAttr.type == GADI_VENC_TYPE_H264) ||
            (stChnAttr.type == GADI_VENC_TYPE_MJPEG) ||
            (stChnAttr.type == GADI_VENC_TYPE_H265)) &&
           (stChnAttr.state ==  GADI_VENC_STREAM_STATE_ENCODING))
        {
            LOG_INFO("stop stream[%d], type:%d\n", cnt, stChnAttr.type);
#if 0
            if(gadi_venc_stop_stream(vencHandle, cnt) < 0)
            {
                LOG_ERR("gadi_venc_stop_stream error, streams:%d\n", cnt);
                return -1;
            }
#endif 		//xqq
        }
    }

    return retVal;
}

static int check_enc2auto_vi_params(int width, int height, int encodeType, int *fps)
{
    GADI_ISP_SensorModelEnumT modelPtr = GADI_ISP_SENSOR_UNKNOWN;
    ST_GK_VI_ATTR vi;
    sdk_vi_get_params(&vi);
//    if(gadi_isp_get_sensor_model(ispHandle, &modelPtr) == GADI_OK)	xqq
//    {
        LOG_INFO("gadi_isp_get_sensor_model:0x%x\n",modelPtr);
        switch(modelPtr)
        {
            case GADI_ISP_SENSOR_SC2232: // 1920x1080@20, 1280x960@20, 1280x720@25
            //case GADI_ISP_SENSOR_GC2053: // 1920x1080@20, 1280x960@20, 1280x720@25
                LOG_INFO("sc2232 GC2053\n");
                if(width>=1280 && height>720)
                {
                    vi.width  = 1920;
                    vi.height = 1080;
                    vi.fps    = 20;
                }
                else
                {
                    vi.width  = 1280;
                    vi.height = 720;
                    vi.fps    = 20;
                }
                break;
            case GADI_ISP_SENSOR_SC4236: // 2304x1296@20, 1920x1080@25, 1280x960@25, 1280x720@25
                LOG_INFO("sc4236\n");
                if(width==2304 && height==1296)
                {
                    vi.width  = width;
                    vi.height = height;
                    vi.fps    = 20;
                }
                else
                {
                    vi.width  = 1920;
                    vi.height = 1080;
                    vi.fps    = 25;
                }
                break;
            case GADI_ISP_SENSOR_OV4689: // 2688x1520@17, 1920x1080@25, 1280x960@25, 1280x720@25
                LOG_INFO("ov4689\n");
                if(width==2688 && height==1520)
                {
                    vi.width  = width;
                    vi.height = height;
                    vi.fps = (1==encodeType)?15:17;
                }
                else
                {
                    vi.width  = 1920;
                    vi.height = 1080;
                    vi.fps    = 25;
                }
                break;
            case GADI_ISP_SENSOR_GC4623: // 2688x1520@17, 1920x1080@25, 1280x960@25, 1280x720@25
                LOG_INFO("gc4623\n");
                if(width==2560 && height==1440)
                {
                    vi.width  = width;
                    vi.height = height;
                    vi.fps = (1==encodeType)?15:18;
                }
                else
                {
                    vi.width  = 1920;
                    vi.height = 1080;
                    vi.fps    = 25;
                }
                break;
            default:
                if(*fps > vi.fps)
                    vi.fps = *fps;
                else
                    return *fps;
                break;
        }
//    }

    sdk_vi_set_params(&vi);
    sdk_vi_get_params(&vi);
    sdk_isp_set_fps(vi.fps);    
    if(*fps > vi.fps)
        *fps = vi.fps;
    return *fps;
}

/* adjust video resolution*/
static int set_video_resolution(int streamid, int width, int height, int *fps, unsigned char encodeType)
{
	int retVal = GADI_OK;
    unsigned char cnt;
    unsigned int viWidth, viHeight;
    GADI_VPSS_ChannelsParamsT chansParams;
    GADI_VENC_StrAttr* stFormatPar;
    GADI_VPSS_OpModeEnumT opMode;

    retVal = stop_all_encode_streams();
    if(retVal != GADI_OK)
    {
        LOG_ERR("stop encode failed!\n");
        return -1;
    }

//    retVal = gadi_vi_enable(viHandle, 0);	xqq
    if(retVal != GADI_OK)
    {
        LOG_ERR("gadi_vi_enable error.\n");
        return -1;
    }

    /*change the stream resolution.*/
    stFormatPar = &(streamsPar[streamid].streamFormat);
    stFormatPar->width  = width;
    stFormatPar->height = height;
    stFormatPar->encodeType = encodeType;

    /*reset channel 4 for JPEG capture.*/
    stFormatPar = &streamsPar[GADI_VENC_STREAM_FORTH].streamFormat;
    stFormatPar->streamId   = GADI_VENC_STREAM_FORTH;
    stFormatPar->encodeType = GADI_VENC_TYPE_MJPEG;
    stFormatPar->vpssChanId = streamsPar[GADI_VENC_STREAM_FIRST].streamFormat.vpssChanId;
    stFormatPar->width      = streamsPar[GADI_VENC_STREAM_FIRST].streamFormat.width;
    stFormatPar->height     = streamsPar[GADI_VENC_STREAM_FIRST].streamFormat.height;
    stFormatPar->xOffset    = streamsPar[GADI_VENC_STREAM_FIRST].streamFormat.xOffset;
    stFormatPar->yOffset    = streamsPar[GADI_VENC_STREAM_FIRST].streamFormat.yOffset;
    stFormatPar->fps        = GK7101_VENC_MJPEG_FPS;
    stFormatPar->stAttrMjpeg.chromaFormat = 1;
    stFormatPar->stAttrMjpeg.flip   = 0;
    stFormatPar->stAttrMjpeg.quality      = 50;

    if(GADI_VENC_STREAM_FIRST==streamid)
    {
        check_enc2auto_vi_params(width, height, encodeType, fps);
    }
    else
    {
        GADI_VI_FrameRateEnumT vi_fps;
#if 0
        if(gadi_vi_get_framerate(viHandle, &vi_fps) != GADI_OK)
        {
            LOG_ERR("resume normal mode frome slowshutter failed.\n");
            return -1;
        }
#endif
        if(*fps > vi_fps)
            *fps = vi_fps;
    }
    streamsPar[streamid].streamFormat.fps = *fps;

    /*check encode streams parameters.*/
//    retVal = gadi_vi_get_resolution(viHandle, &viWidth, &viHeight);	xqq
    if(retVal != GADI_OK)
    {
        LOG_ERR("gadi_vin_get_resolution error\n");
        return -1;
    }
    retVal = check_encode_streams_resolution(viWidth, viHeight);
    if(retVal != GADI_OK)
    {
        LOG_ERR("check_encode_streams_resolution error\n");
        return -1;
    }

    //gadi_sys_memset(&chansParams, 0, sizeof(chansParams));	xqq
    retVal = get_channels_params(&chansParams);
    if(retVal != GADI_OK)
    {
        LOG_ERR("get_channels_params error\n");
        return -1;
    }

    //LOG_INFO("print_channels_params\n");
    //print_channels_params(&chansParams);
    if(GADI_VENC_STREAM_FIRST==streamid)
    {
        if ((viWidth <= 1920) &&
        (chansParams.chanMainAWidth <= 1920))
        {
            opMode = GADI_VPSS_OP_MODE_REAL_TIME;
        }
        else
        {
            #if 1
            if(1==encodeType)
                opMode = GADI_VPSS_OP_MODE_NON_REAL_TIME0;//GADI_VPSS_OP_MODE_NON_REAL_TIME3
            else
                opMode = GADI_VPSS_OP_MODE_NON_REAL_TIME2;
            #else
            opMode = GADI_VPSS_OP_MODE_NON_REAL_TIME3;
            #endif
        }
//        retVal = gadi_vpss_set_op_mode(vpssHandle, opMode);	xqq
        if(retVal != GADI_OK)
        {
            LOG_ERR("gadi_vpss_set_op_mode failed!\n");
            return -1;
        }
    }
    /*set channels parameters.*/
//    retVal = gadi_vpss_set_channels_params(vpssHandle, &chansParams);	xqq
    if(retVal != GADI_OK)
    {
        LOG_ERR("set channels parameters error.\n");
        return -1;
    }


    /*reset all encode stream fromat.*/
    for (cnt = 0; cnt < GADI_VENC_STREAM_NUM; cnt++)
    {
        stFormatPar = &(streamsPar[cnt].streamFormat);
        if(stFormatPar->encodeType != 0)
        {
            //retVal = gadi_venc_set_stream_attr(vencHandle, stFormatPar);	xqq
            if (retVal != GADI_OK)
            {
                LOG_ERR("gadi_venc_set_stream_format error, steamId=%d\n",streamid);
                return -1;
            }
        }
    }

//    retVal = gadi_vi_enable(viHandle, 1);	xqq
    if (retVal != GADI_OK)
    {
        LOG_ERR("gadi_vi_enable error.\n");
        return -1;
    }

    retVal = start_all_encode_streams();
    if (retVal != GADI_OK)
    {
        LOG_ERR("start encode failed!\n");
        return -1;
    }

	return 0;
}


static int read_video_stream(void)
{
    int i,ret;
    GADI_CHN_AttrT  chn_attr;
    GADI_VENC_StreamT stream;
    GADI_VENC_StrAttr *streamPar, strAttr;
    bool encodingState = false;
    bool is_keyframe = false;
    for(i = 0; i < GADI_VENC_STREAM_NUM; i ++)
    {
#if 0 
        if (gadi_venc_query(vencHandle, i, &chn_attr) < 0)
        {
            LOG_ERR("gadi_venc_query chn:%d", i);
            continue;
        }
#endif		//xqq
        if((chn_attr.state == GADI_VENC_STREAM_STATE_ENCODING) ||
           (chn_attr.state == GADI_VENC_STREAM_STATE_STOPPING))
        {
            encodingState = true;
            break;
        }
        else if(GADI_VENC_STREAM_STATE_DSP_TIMEOUT == chn_attr.state)
        {
            LOG_ERR("VENC dsp timeout, reset dsp\n");
            //gadi_venc_reset_dsp(vencHandle);	xqq
        }
    }

    if (encodingState)
    {
#if 0
        if (gadi_venc_get_stream(vencHandle, 0XFF, &stream)<0)//BLOCK
        {
            //LOG_ERR("gadi_venc_get_stream chn:%d failed.", stream.stream_id);
            return -1;
        }
#endif		//xqq
        /*stream end, stream size & stream addr is invalid.*/
        if(stream.stream_end == 1)
        {
            return -1;
        }

        if(stream.pic_type == GADI_VENC_JPEG_FRAME)
        {
			if( capJpegFlag == true)
			{
				jpegSize    = (int)stream.size;
            	jpegAddr    = (char *)stream.addr;
            	capJpegFlag = false;
            	//gadi_sys_sem_post(jpegCapSem);	xqq
			}
            return -1;
        }

        is_keyframe = (stream.pic_type == GADI_VENC_IDR_FRAME)?true:false;

        if((++venc_status) %1800 == 0)
        {
            //printf(".............video encode is ok ..........\n");
        }

	    #if 0 // ���1�����Ƿ��ж�֡
        static int video_fps= 0, video_count = 0;
        static time_t t1, t2;
        if(0 == stream.stream_id)
        {
            video_fps++;
            if (stream.pic_type == GADI_VENC_IDR_FRAME)
            {
                printf("$$$$$$$$$$$$$$$$$$$ IDR_FRAME count:%d\n", video_count);
                video_count = 1;
            }
            else
                video_count ++;
        }
        t2 = time(NULL);
        if(t2 < t1)
        {
            t1 = t2;
        }
        if(t2 - t1 >= 1)
        {
            printf("--------------venc--------video_fps:%d\n", video_fps);
            t1 = t2;
            video_fps = 0;
        }
        #endif

        ST_GK_ENC_BUF_ATTR header = {0};        
        streamPar = &strAttr;
        strAttr.streamId = stream.stream_id;
        //ret = gadi_venc_get_stream_attr(vencHandle, &strAttr);	xqq
        if(ret != GADI_OK)
        {
            LOG_ERR("gadi_venc_get_stream_attr error, ret:%d\n",ret);
            streamPar = &(streamsPar[stream.stream_id].streamFormat);
        }
        if(1 == streamPar->encodeType)
        {
            header.type = GK_ENC_DATA_H264;
        }
        else if(3 == streamPar->encodeType)
        {
            header.type = GK_ENC_DATA_H265;
        }

        //header.time_us = stream.PTS; //(stream.PTS / 90) * 1000; //ticker of 90KHz: (PTS/90000)*1000000 us
        cal_video_pts(&header.time_us, stream.stream_id);

        header.data_sz = stream.size;
        header.data = stream.addr;

        header.video.keyframe = is_keyframe;
        header.video.fps = streamPar->fps;
        header.video.width = streamPar->width ;
        header.video.height = streamPar->height;

        if(stream.size > 500*1024)
        {
            LOG_INFO("video_frame index:stream_id:%d size:%d, larger than 500KB\n",stream.stream_id,stream.size);
        }

        //LOG_INFO("type = %s id = %d, size = %d, pts = %lld \n",
        //        (stream.pic_type == GADI_VENC_IDR_FRAME ? "I" : "P"),
        //       buf_id, stream.size, header.pts);

        encStreamCb(stream.stream_id, &header);
    }
    else
    {
        LOG_ERR("************** not enc ***************\n");
        return -1;
    }

    return 0;
}


static int read_audio_stream()
{
    int ret;
    int i;
	GADI_AUDIO_AioFrameT ptrFrame = {0};
    ST_GK_ENC_BUF_ATTR header = {0};

#ifdef AUDIO_ECHO_CANCELLATION_SUPPORT
    GADI_AEC_AioFrameT aecFrame = {0};
    //ret = gadi_audio_ai_get_frame_aec(&ptrFrame, &aecFrame, GADI_TRUE);	xqq
    //LOG_INFO("a frame len:%d, aecframe len:%d \n", ptrFrame.len, aecFrame.len);
//#else		xqq
    //ret = gadi_audio_ai_get_frame(&ptrFrame, GADI_TRUE);	xqq
    //LOG_INFO("a frame len:%d\n", ptrFrame.len);
#endif
    if ( (ret != 0) || (ptrFrame.len == 0)) {
        LOG_WARNING("ret: %d ptrFrame.len: %d\n", ret, ptrFrame.len);
        return -1;
    }

	if(aac_encoding_process)
		aac_encoding_process(ptrFrame.virAddr, ptrFrame.len);

    #if 1
    /* ��Ϊ160�ֽ�һ����Ƶ����һ����������2��160�ֽڵ���Ƶ֡���һ���µ�320�ֽڵ���Ƶ֡ */
    if (abuf == NULL) {
        abuf = malloc(ptrFrame.len * 2);
    }
    if (a_num == 0) {
        memcpy(abuf, ptrFrame.virAddr, ptrFrame.len);
        a_num ++;
        return 0;
    }
    if (a_num == 1) {
        memcpy(abuf + ptrFrame.len, ptrFrame.virAddr, ptrFrame.len);
        a_num = 0;
    }
    header.data = abuf;
    header.data_sz = ptrFrame.len * 2;
    #else
    header.data = (void *)ptrFrame.virAddr;
    header.data_sz = ptrFrame.len;
    #endif


    #if 0
    unsigned int pts = 0;
    if (runAudioCfg.type == 2)
        cal_audio_pts(&pts, ptrFrame.len / 2);
    else if (runAudioCfg.type == 1)
        cal_audio_pts(&pts, ptrFrame.len * 2);
    else
        cal_audio_pts(&pts, ptrFrame.len);
    header.time_us = pts;
    #else
    cal_audio_pts(&(header.time_us), header.data_sz);
    #endif


    header.no = (++audio_frame_index);
    #if 0
    if (audio_frame_index % 800 == 0)
        LOG_INFO("************** mediabuf have audio frame, frame size: %d ***************\n", header.data_sz);
    #endif

    for (i = 0; i < gk_get_max_stream_nums(); i ++) {
        getAudioStreamCb(i, &header);
    }
	return 0;
}

#if 0
static void* enc_loop(void* arg)
{
    sdk_sys_thread_set_name("enc_loop");

    while (streamReadFlag) {
        /*audio stream.*/
        read_audio_stream();
        /*video stream. blocking read.*/
        read_video_stream();
    }

    return NULL;
}
#endif

static void* enc_video_loop(void* arg)
{
	sdk_sys_thread_set_name("enc_video_loop");

    while (streamReadVideoFlag) {
        /*video stream. blocking read.*/
        if(read_video_stream() != 0)
        {
			usleep(40000);
		}
    }

    return NULL;
}

static void* enc_audio_loop(void* arg)
{
	sdk_sys_thread_set_name("enc_audio_loop");

    while (streamReadAudioFlag) {
        /*video stream. blocking read.*/        
        if(read_audio_stream() != 0)
        {
			usleep(20000);
		}
    }

    /* ��Ϊ160�ֽ�һ����Ƶ����һ����������2��160�ֽڵ���Ƶ֡���һ���µ�320�ֽڵ���Ƶ֡ */
    if (abuf) {
        free(abuf);
        abuf = NULL;
    }

    return NULL;
}

// type =1 ����ϵͳ�����룬type=2 ����ϵͳ����ms
static unsigned long get_sys_runtime(int type)
{
    struct timespec times = {0, 0};
    unsigned long time;

    clock_gettime(CLOCK_MONOTONIC, &times);
    //LOG_INFO("CLOCK_MONOTONIC: %lu, %lu\n", times.tv_sec, times.tv_nsec);

    if (1 == type)
    {
        time = times.tv_sec;
    }
    else
    {
        time = times.tv_sec * 1000 + times.tv_nsec / 1000000;
    }

    //LOG_INFO("time = %ld\n", time);
    return time;
}

static void cal_video_pts(unsigned int*pts, unsigned int ch)
{
    #if 1
    static unsigned long tv1[GADI_VENC_STREAM_NUM] = {0, 0}, tv2[GADI_VENC_STREAM_NUM] = {0,0};
    unsigned int time_ms = 0;

    if(0 == tv1[ch])
        tv1[ch] = get_sys_runtime(2);
    tv2[ch] = get_sys_runtime(2);

    time_ms = tv2[ch] - tv1[ch];
    //LOG_INFO("video time_ms:%u\n", time_ms);

    video_pts[ch] += 90 * time_ms;
    *pts =video_pts[ch];
    tv1[ch] = tv2[ch];
    //LOG_INFO("video stream%d pts:%u\n", ch, *pts);
    #else
    GADI_VENC_FrameRateT frPar;
    frPar.streamId = ch;
    gadi_venc_get_framerate(vencHandle, &frPar);
    *pts = video_pts[ch];
    if(frPar.fps)
        video_pts[ch] += (90000 / frPar.fps);
    else
        video_pts[ch] += (90000 / streamsPar[ch].streamFormat.fps);
    #endif
    return;
}

static void cal_audio_pts(unsigned int*pts, unsigned int pkg_len)
{
    #if 1
    static unsigned long tv1 = 0, tv2 = 0;
    unsigned int time_ms = 0;

    if(0 == tv1)
        tv1 = get_sys_runtime(2);
    tv2 = get_sys_runtime(2);
    time_ms = tv2 - tv1;
    audio_pts += 8 * time_ms;
    *pts = audio_pts;
    tv1 = tv2;
    //LOG_INFO("audio pts:%u\n", *pts);
    #else
    static unsigned int add_pts = 0;
    if(!add_pts)
    {
        GADI_AUDIO_AioAttrT aiAttr;
        if(!gadi_audio_ai_get_attr(&aiAttr))
        {
            int chn_num = 1;
            switch(aiAttr.soundMode)
            {
                case GADI_AUDIO_SOUND_MODE_SINGLE:
                case GADI_AUDIO_SOUND_MODE_LEFT:
                case GADI_AUDIO_SOUND_MODE_RIGHT:
                case GADI_AUDIO_SOUND_MODE_MONO:
                    chn_num = 1;
                    break;
                case GADI_AUDIO_SOUND_MODE_STEREO:
                    chn_num = 2;
                    break;
                default:
                    chn_num = 1;
                    break;
            }
            add_pts = aiAttr.sampleRate/((aiAttr.sampleRate * (aiAttr.bitWidth/8) * chn_num)/pkg_len);
            //LOG_INFO("audio pts:%d, sam:%d, bit:%d, chn_num:%d, len:%d\n", add_pts, aiAttr.sampleRate ,aiAttr.bitWidth ,chn_num,pkg_len);
        }
    }
    *pts = audio_pts;
    audio_pts += add_pts;
    #endif
    return;
}

goke_encode_stream_params* gk_get_enc_stream_par(unsigned char strId)
{
    if(strId > GADI_VENC_STREAM_FORTH )
    {
        LOG_ERR("get encode stream parameters : input invalied stream id.\n");
        return NULL;
    }

    return &(streamsPar[strId]);

}

int gk_adjust_bps(int stream_id, int bps)
{
    GADI_VENC_BitRateRangeT stBitRate = {0};

    stBitRate.streamId  = stream_id;
    stBitRate.brcMode   = GADI_VENC_VBR_MODE;
    stBitRate.cbrAvgBps = bps * 1000;
    stBitRate.vbrMinbps = bps * 500;
    stBitRate.vbrMaxbps = bps * 1000;
#if 0
    int ret = gadi_venc_set_bitrate(vencHandle, &stBitRate);
    if (ret < 0) {
        LOG_ERR("set bitrate failed\n");
        return -1;
    }
#endif		//xqq
    return 0;
}

int gk_adjust_fps(int stream_id, int fps)
{
    GADI_VENC_FrameRateT stFrameRate = {0};
    stFrameRate.streamId = stream_id;
    stFrameRate.fps = fps;
#if 0
    int ret = gadi_venc_set_framerate(vencHandle, &stFrameRate);
    if (ret < 0) {
        LOG_ERR("set framerate failed\n");
        return -1;
    }
#endif		//xqq
    return 0;
}

int gk_venc_set_h264_qp_config(GADI_VENC_H264QpConfigT *pQpConfig)
{
    int ret;
   //ret  = gadi_venc_set_h264_qp_config(vencHandle, pQpConfig);	xqq

    if (ret < 0) {
        LOG_ERR("set stream%d h264 qp config failed\n", pQpConfig->streamId);
        return -1;
    }

    return 0;
}

int sdk_venc_get_stream_resolution(int stream_id, int *width, int *height)
{
	int retVal = GADI_OK;
	GADI_VENC_StrAttr formatPar;
	formatPar.streamId = stream_id;
	//retVal = gadi_venc_get_stream_attr(vencHandle, &formatPar);	xqq
	if(retVal == GADI_OK){
		*width = (int)formatPar.width;
		*height = (int)formatPar.height;
	}
	return retVal;
}


void sdk_set_h26x_smart(int enable, int gopInterval)
{
	GADI_VENC_SmartVideoT smart_video;

	if(enable && (gopInterval < 3))
		gopInterval = 3;
	smart_video.streamId = 0;//only main stream
	//gadi_venc_get_h26x_smart_video(vencHandle, &smart_video);	xqq
	smart_video.smartVideoEnable = enable;
	smart_video.gopInterval = gopInterval;
	smart_video.debugMode = 0;
	//gadi_venc_set_h26x_smart_video(vencHandle, &smart_video);
	LOG_ERR("xxxxxxxxxxxx smart_video smartVideoEnable:%d, gopInterval:%d\n", enable, gopInterval);

}
void sdk_set_audio_proc_callback(audio_proc_callback callback)
{
	aac_encoding_process = callback;
}
