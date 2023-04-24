/*!
*****************************************************************************
** \file      $gkprjgk_rtsp.c
**
** \version	$id: gk_rtsp.c 15-08-03
**
** \brief
**
** \attention   this code is provided as is. goke microelectronics
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <pthread.h>
#include "sdk_debug.h"
#include "media_fifo.h"
#include "sdk_enc.h"
#include "gk_rtsp.h"
#include "rtspServLib.h"
#include "rtp.h"
#include "g711.h"
#include "netcam_api.h"

static RTSP_PARAM_S glbRtspParam;
static pthread_t rtsp_th;
static int rtsp_run = 0;

static int set_rtsp_start_param(RTSP_PARAM_S *rtsp_param)
{
    int i;

    memset(rtsp_param, 0, sizeof(RTSP_PARAM_S));
    /////////video
    rtsp_param->video.max_ch = netcam_video_get_channel_number();
    if(rtsp_param->video.max_ch > 4)//max venc channle is 4
        rtsp_param->video.max_ch = 4;
    for(i = 0; i < rtsp_param->video.max_ch; i ++)
    {
        if(1 == runVideoCfg.vencStream[i].enctype)/*0: none, 1: H.264, 2: MJPEG, 3: H.265*/
        {
            rtsp_param->video.enc_type[i] = RTP_CODEC_ID_H264;
            //LOG_INFO("rtsp stream is h264\n");
        }
        else if(3 == runVideoCfg.vencStream[i].enctype)
        {
            rtsp_param->video.enc_type[i] = RTP_CODEC_ID_H265;
            //LOG_INFO("rtsp stream is h265\n");
        }
        else
            rtsp_param->video.enc_type[i] = RTP_CODEC_ID_NONE;
        rtsp_param->video.buffer_id[i]    = i;
        rtsp_param->video.fps[i]          = runVideoCfg.vencStream[i].h264Conf.fps;
        rtsp_param->video.width[i]        = runVideoCfg.vencStream[i].h264Conf.width;
        rtsp_param->video.height[i]       = runVideoCfg.vencStream[i].h264Conf.height;
        sprintf(rtsp_param->video.rtsp_route[i], "/stream%d", i);
    }
    /////////audio
    //LOG_INFO("runAudioCfg.mode:%d\n", runAudioCfg.mode);
    if(runAudioCfg.mode)
        rtsp_param->audio.enable = 1;
    //switch(runAudioCfg.type)// 0 a-law; 1 u-law; 2-PCM
    switch(0)// 0 a-law; 1 u-law; 2-PCM
    {
        case 0:
            rtsp_param->audio.enc_type = RTP_CODEC_ID_PCM_ALAW;
            break;
        case 1:
            rtsp_param->audio.enc_type = RTP_CODEC_ID_PCM_MULAW;
            break;
        case 2:
           rtsp_param->audio.enc_type = RTP_CODEC_ID_PCM;
            break;
        default:
            //rtsp_param->audio.enc_type = CODEC_ID_NONE;
            rtsp_param->audio.enc_type = RTP_CODEC_ID_PCM_ALAW;
            LOG_ERR("rtsp_param audio not support!\n");
            break;
    }
    rtsp_param->audio.samplerate = runAudioCfg.sampleRate;
    rtsp_param->audio.samplewidth = runAudioCfg.sampleBitWidth;
    rtsp_param->audio.channle_num = runAudioCfg.chans;
    rtsp_param->authenticate= 0;

    return 0;
}

static int gk_rtsp_describe(char *path, char *query, MediaDesc *descs, int nmemb)
{
    int i, count = 0,chn = -1;
	MediaDesc *desc = NULL;
    char route[128];
    RTSP_PARAM_S rtsp_param;
    set_rtsp_start_param(&rtsp_param);

    for(i = 0; i < MAX_VIDEO_ENC_CHN; i ++)
    {
        memset(route, 0, sizeof(route));
        sprintf(route, "%s", rtsp_param.video.rtsp_route[i]);
        if(strlen(path) < strlen(route))
            continue;
        if(!(strncmp(path, route, strlen(route))))
        {
            chn = i;
            break;
        }
    }
    if(chn < 0)
        return 0;
    desc = &descs[MEDIA_TYPE_VIDEO];
    memset(desc, 0, sizeof(MediaDesc));
    desc->media = MEDIA_TYPE_VIDEO;
    desc->codec = rtsp_param.video.enc_type[chn];//CODEC_ID_H264;
    desc->video_width = rtsp_param.video.width[chn];
    desc->video_height = rtsp_param.video.height[chn];
    desc->video_fps = rtsp_param.video.fps[chn];
    desc->clockrate = 90000;
    desc->record = 0;
    desc->start_ms = 10000;
    desc->end_ms = 1000000000;
    desc->multicast.enable = 1;
    strncpy(desc->multicast.ipaddr, "239.100.101.102", sizeof(desc->multicast.ipaddr) - 1);
    desc->multicast.port = 6423;
    desc->multicast.ttl = 32;
    count ++;

    if(rtsp_param.audio.enable)
    {
        desc = &descs[MEDIA_TYPE_AUDIO];
        memset(desc, 0, sizeof(MediaDesc));
        desc->media = MEDIA_TYPE_AUDIO;
        desc->codec = rtsp_param.audio.enc_type;//CODEC_ID_PCM_ALAW;
        desc->audio_samplerate = rtsp_param.audio.samplerate;
        desc->audio_samplewidth = rtsp_param.audio.samplewidth;
        desc->audio_channle_num = rtsp_param.audio.channle_num;
        //desc->clockrate = 80000;
        desc->clockrate = rtsp_param.audio.samplerate;
        desc->record = 0;
        desc->start_ms = 10000;
        desc->end_ms = 1000000000;
        desc->multicast.enable = 1;
        strncpy(desc->multicast.ipaddr, "239.100.101.102", sizeof(desc->multicast.ipaddr) - 1);
        desc->multicast.port = 6423;
        desc->multicast.ttl = 32;
        count ++;
    }

    return count;
}

static void *gk_rtsp_open(char *path, char *query)
{
    int i, id;
    char route[128];
	LOG_INFO(">>>> %s: ##### path = %s, query:%s\n", __func__, path, query);

    for(i = 0; i < VENC_MAX_STREAM_NUM; i ++)
    {
        memset(route, 0, sizeof(route));
        sprintf(route, "%s",glbRtspParam.video.rtsp_route[i]);
        if(strlen(path) < strlen(route))
            continue;
        if(!(strncmp(path, route, strlen(route))))
        {
            id = i;
            break;
        }
    }
    if(VENC_MAX_STREAM_NUM == i)
    {
        LOG_ERR("rtsp path error:%s", path);
        return 0;
    }
    MEDIABUF_HANDLE handle = mediabuf_add_reader(id);
    if(handle)
        mediabuf_set_newest_frame(handle);
    LOG_INFO("rtsp add reader %d, handle = 0x%08x,\n", id, (int)handle);
	return (void *)handle;
}

static int gk_rtsp_close(void *handle)
{
	LOG_INFO("%s: #####\n", __func__);
    //zfifo_close((ZFIFO_DESC *)handle);
    //MEDIABUF_close_byhandle(handle);
    mediabuf_del_reader(handle);
    return 0;
}

static int convert_to_rtspheader(frame_head_t *dst, GK_NET_FRAME_HEADER *src)
{
    dst->device_type = src->device_type;
    dst->frame_size = src->frame_size;
    dst->frame_no = src->frame_no;
    dst->video_reso = src->video_reso;
    switch (src->frame_type) {
        case GK_NET_FRAME_TYPE_I:
            dst->frame_type = GOKE_I_FRAME;
            break;
        case GK_NET_FRAME_TYPE_P:
            dst->frame_type = GOKE_P_FRAME;
            break;
        case GK_NET_FRAME_TYPE_A:
            dst->frame_type = GOKE_A_FRAME;
            break;
        default:
            break;
    }
    dst->payload = src->media_codec_type;
    dst->frame_rate = src->frame_rate;
    dst->video_standard = src->video_standard;
    dst->sec = src->sec;
    dst->usec = src->usec;
    dst->pts = src->pts;
    return 0;
}

static int gk_rtsp_read_p(void *handle, void **buf, size_t *size, int backward,void *rtsp_header)
{
    int  n;

    GK_NET_FRAME_HEADER header = {0};
    frame_head_t *new_rtsp_head = (frame_head_t *)rtsp_header;

    n = mediabuf_read_frame((MEDIABUF_HANDLE)handle, buf, (int*)size, &header);
    if (n <= 0)
        return n;

    convert_to_rtspheader(new_rtsp_head, &header);

    #if 0
    char tmp[10] = {0};
    if(header.frame_type == GK_NET_FRAME_TYPE_I)
        strcpy(tmp, "I : ");
    else if(header.frame_type == GK_NET_FRAME_TYPE_P)
        strcpy(tmp, "P : ");
    else if(header.frame_type == GK_NET_FRAME_TYPE_A)
        strcpy(tmp, "A : ");
    LOG_INFO("%s index = %d size = %d, pts = %lld \n", tmp, header.frame_no, n, header.pts);
    #endif

    return (*size);
}


static int gk_rtsp_read(void *handle, void *buf, size_t size, int backward)
{

    if ((buf == NULL) || (handle == NULL))
    {
        LOG_ERR("param error. %s: #####\n", __func__);
        return -1;
    }
    int data_size, n;
    char *pbuf = buf;
    frame_head_t *rtsp_header = (frame_head_t *)pbuf;
    void *data = (void *)(pbuf + sizeof(frame_head_t));
    GK_NET_FRAME_HEADER header = {0};
    data_size = size - sizeof(frame_head_t);

    n = mediabuf_read_frame((MEDIABUF_HANDLE)handle, &data, &data_size, &header);
    if (n <= 0)
        return n;

    convert_to_rtspheader(rtsp_header, &header);

    return (sizeof(frame_head_t) + data_size);
}

static int gk_rtsp_getusers(int *flag, users_s *usersp)
{
	if(flag == NULL || usersp == NULL)
	{
		LOG_ERR("bad paramters.\n");
		return -1;
	}
	*flag = glbRtspParam.authenticate;
	if(glbRtspParam.authenticate)
	{
		int i = 0;
		for(i = 0; i < GK_MAX_USER_NUM; i ++) 
		{
			if(runUserCfg.user[i].enable == 1) 
			{
				if(usersp->sizeUser >= RTSP_USER_INFO_NUMBER_MAX)
				{
					LOG_WARNING("numbers of users is overflow.");
				}
				strncpy(usersp->stUser[usersp->sizeUser].name, runUserCfg.user[i].userName, 63);
                if (strlen(runUserCfg.user[i].password) > 0)
				    strncpy(usersp->stUser[usersp->sizeUser].pw, runUserCfg.user[i].password, 63);
                else
                    memset(usersp->stUser[usersp->sizeUser].pw, 0, sizeof(usersp->stUser[usersp->sizeUser].pw));
				usersp->sizeUser++;
			}
		}
	}
	
	return 0;
}

void printf_rtsp_param(RTSP_PARAM_S *param)
{
    int i;

    LOG_INFO("video max ch:%d\n", param->video.max_ch);
    for(i = 0; i < MAX_VIDEO_ENC_CHN; i ++)
    {
        LOG_INFO("video enc_type[%d]:%d\n", i, param->video.enc_type[i]);
        LOG_INFO("video buffer_id:%d\n", param->video.buffer_id[i]);
        LOG_INFO("rtsp_route[%d]:%s\n", i, param->video.rtsp_route[i]);
    }
    LOG_INFO("audio enable:%d\n", param->audio.enable);
    LOG_INFO("audio enc_type:%d\n", param->audio.enc_type);
    LOG_INFO("audio samplerate:%d\n", param->audio.samplerate);
    LOG_INFO("audio samplewidth:%d\n", param->audio.samplewidth);
    LOG_INFO("authenticate:%d\n", param->authenticate);
}

int rtsp_setParam(RTSP_PARAM_S *param)
{
    if(!param)
    {
        LOG_ERR("rtsp setParam error");
        return -1;
    }
    memcpy(&glbRtspParam, param, sizeof(RTSP_PARAM_S));
    //printf_rtsp_param(param);

    return 0;
}

int rtsp_getParam(RTSP_PARAM_S *param)
{
    if(NULL == param)
    {
        return -1;
    }
    memcpy(param, &glbRtspParam, sizeof(RTSP_PARAM_S));

    return 0;
}
static void rtsp_defaultParam(RTSP_PARAM_S *rtsp)
{
    int i;

    memset(rtsp, 0, sizeof(RTSP_PARAM_S));
    rtsp->video.max_ch = MAX_VIDEO_ENC_CHN;
    for(i = 0; i < MAX_VIDEO_ENC_CHN; i ++)
    {
        rtsp->video.enc_type[i] = RTP_CODEC_ID_H264;
        rtsp->video.buffer_id[i] = i;
        rtsp->video.fps[i] = 25;
        sprintf(rtsp->video.rtsp_route[i], "/stream%d", i);
    }
    rtsp->audio.enc_type = RTP_CODEC_ID_PCM_ALAW;
    //rtsp->audio.enc_type = MEDIA_CODEC_RAW_PCM;
    rtsp->audio.samplerate = 8000;
    rtsp->audio.samplewidth = 16;
    rtsp->authenticate= 0;
}

//int main(int argc, char *argv[])
static void *rtsp_thread(void*arg)
{
    sdk_sys_thread_set_name("rtsp_thread");
    int i;
	RtspOps stream_ops;

    LOG_INFO(">>> %s", __func__);
	signal(SIGINT, SIG_DFL);
	signal(SIGPIPE, SIG_IGN);
	st_rtsp_initLib();
	//st_rtsp_setupRtcp(0, 0);

	memset(&stream_ops, 0, sizeof(RtspOps));

	stream_ops.open     = gk_rtsp_open;
	stream_ops.close    = gk_rtsp_close;
	stream_ops.read     = gk_rtsp_read;
    stream_ops.read_p   = gk_rtsp_read_p;
	stream_ops.describe = gk_rtsp_describe;
    stream_ops.users_info = gk_rtsp_getusers;

    for(i = 0; i < MAX_VIDEO_ENC_CHN; i ++)
    {
        st_rtsp_addRoute(glbRtspParam.video.rtsp_route[i], &stream_ops);
    }

	st_rtsp_startService(NULL, DFL_RTSP_PORT, DFL_HTTP_PORT); // 554, 8081

#if 0
	sleep(3);

	st_rtsp_startMulticastStreaming("/stream/av0_0");

	sleep(100);

	st_rtsp_stopMulticastStreaming("/stream/av0_0");
#endif

	while (rtsp_run)
    {
		sleep(1);
    }

	st_rtsp_stopService();
	st_rtsp_deinitLib();

	return (void *)0;
}

/**********************************************************************
???????~{v#:?~}??? rtsp server
???~{Z2~}???????
????~{V5~}??  0: ?~{I9~}?
          ??0: ~{J'~}??
**********************************************************************/
int rtsp_start(void)
{
    RTSP_PARAM_S rtsp_param;

    int ret = set_rtsp_start_param(&rtsp_param);
    if (ret == 0)
        rtsp_setParam(&rtsp_param);
    else
        rtsp_defaultParam(&glbRtspParam);

    rtsp_run = 1;
    if(!pthread_create(&rtsp_th, NULL, rtsp_thread, NULL ))
        return 0;
    return -1;
}

/**********************************************************************
???????~{v#:9X1~}? rtsp server
???~{Z2~}???????
????~{V5~}??  ??
**********************************************************************/
void rtsp_stop(void)
{
    rtsp_run = 0;
}

