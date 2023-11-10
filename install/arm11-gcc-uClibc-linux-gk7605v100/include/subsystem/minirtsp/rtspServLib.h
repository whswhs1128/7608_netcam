/*!
*****************************************************************************
** \file      $gkprjinc/rtspServLib.h
**
** \version	$id: inc/rtspServLib.h 15-08-08  8月:08:1439014830
**
** \brief
**
** \attention   this code is provided as is. goke microelectronics
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#ifndef __RTSPLIB_H__
#define __RTSPLIB_H__

#include <stdint.h>

#define DFL_RTSP_PORT       554	/* Default RTSP port. */
#define DFL_HTTP_PORT       8080	/* Default HTTP port. */

#define RTSP_USER_INFO_NUMBER_MAX     10

typedef enum RTP_AVCodecID {
    /* video codecs */
    RTP_CODEC_ID_MJPEG      = 5,
    RTP_CODEC_ID_H264       = 0,
    RTP_CODEC_ID_HEVC       = 6,
    RTP_CODEC_ID_H265       = RTP_CODEC_ID_HEVC,

    /* various PCM "codecs" */
    RTP_CODEC_ID_PCM        = 3,
    RTP_CODEC_ID_PCM_MULAW  = 2,
    RTP_CODEC_ID_PCM_ALAW   = 1,

    /* various ADPCM codecs */
    RTP_CODEC_ID_ADPCM_G726 = 4,
    RTP_CODEC_ID_NONE,
}RTP_AV_CODEC_ID;
/* Media types. */
typedef enum AV_MEDIA_TYPE {
    MEDIA_TYPE_VIDEO    = 0,
    MEDIA_TYPE_AUDIO    = 1,
    MEDIA_TYPE_METADATA = 2,
    MEDIA_TYPE_MAX,
}AV_MEDIA_TYPE;

typedef struct
{
    char name[64];
    char pw[64];
}user_s;

typedef struct
{
    int sizeUser;
    user_s stUser[RTSP_USER_INFO_NUMBER_MAX];
}users_s;

typedef struct MediaDesc {
	int media;	/* 0 - video, 1 - audio, 2 - metadata */
	int codec;	/* 0 - H.264, 1 - G.711ALaw, 2 - G.711uLaw, 6 - H265 */
    int video_width;
    int video_height;
    int video_fps;
    int audio_samplerate;
    int audio_samplewidth;
    int audio_channle_num;
	int clockrate;
	int record;	/* 0 - Real-Time, 1 - Record */
	int64_t start_ms;
	int64_t end_ms;
	struct {
		int  enable;
		char ipaddr[32];
		int  port;
		int  ttl;
	} multicast;
} MediaDesc;

typedef struct RtspOps {
	int     (*describe)(char *path, char *query, MediaDesc *descs, int nmemb);

	void *  (*open)(char *path, char *query);
	int     (*close)(void *handle);

	int     (*read)(void *handle, void *buf, size_t size, int backward);
    int     (*read_p)(void *handle, void **buf, size_t *size, int backward,void *rtsp_header);
	int64_t (*seek)(void *handle, int64_t off_ms, int whence);
	int     (*skip)(void *handle, int numframes, int frametype, int backward);	/* frametype: 0 - any, 1 - intra */
	int     (*users_info)(int *flag, users_s *usersp);
} RtspOps;

/**
 * @strm_rpospp:    two level pointer to stream read position.
 */
int st_rtsp_initLib(void);
int st_rtsp_deinitLib(void);

int st_rtsp_addRoute(const char *route, RtspOps *ops);

int st_rtsp_startService(const char *ip, int rtspport, int httpport);
int st_rtsp_stopService(void);

int st_rtsp_setupRtcp(int enable, int interval_s);
int st_rtsp_getVersion(char *version, char *buildDate);

int st_rtsp_startMulticastStreaming(char *uri);
int st_rtsp_stopMulticastStreaming(char *uri);

#endif
