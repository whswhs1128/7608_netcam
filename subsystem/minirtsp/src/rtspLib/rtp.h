/*!
*****************************************************************************
** \file      $gkprjrtp.h
**
** \version	$id: rtp.h 15-08-04  8月:08:1438655275
**
** \brief
**
** \attention   this code is provided as is. goke microelectronics
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/

#ifndef __RTPSERV_H__
#define __RTPSERV_H__

//#ifdef GK7101
//#define MAX_FRAME_SIZE (1024 * 1024)
//#else
#define MAX_FRAME_SIZE (500 * 1024)
//#endif
enum {
    RTP_PT_VIDEO_H264 = 96,
    RTP_PT_VIDEO_H265 = 96,
    RTP_PT_AUDIO_PCMU = 0,
    RTP_PT_AUDIO_PCMA = 8,
};

/* NALU payload types in RTP packets */
enum {
    NALU_PT_FU_A = 28,
    NALU_PT_FU_B = 29,
};

/* NALU types */
enum {
    NALU_TYPE_SPS = 7,          /* sequence parameter set */
    NALU_TYPE_PPS = 8,          /* picture parameter set */
};

/* Frame types. */
enum {
    GOKE_I_FRAME = 1,
    GOKE_P_FRAME = 2,
    GOKE_A_FRAME = 3,
};

/* Frame head of Goke. */
typedef struct frame_head_s {
    unsigned int device_type;
    unsigned int payload;
    unsigned int frame_size;
    unsigned int frame_no;
    unsigned char video_reso;   /* Video resolution. */
    unsigned char frame_type;
    unsigned char frame_rate;
    unsigned char video_standard;
    unsigned int sec;
    unsigned int usec;
    int64_t pts;
} frame_head_t;

typedef struct rtp_payload_type{
    int pt;
    const char codec_name[6];
    AV_MEDIA_TYPE media_type;
    RTP_AV_CODEC_ID codec_id;
    int clock_rate;
    int audio_channels;
} rtp_payload_type;

void *rtsp_stream_thread(void *arg);
void *rtsp_replay_thread(void *arg);
int rtp_get_codec_info(int codec_id, rtp_payload_type* par);
int rtp_get_payload_type(int codec_id);
const char *rtp_codec_name(int codec_id);
int rtp_codec_id(const char *codec_name, int media_type);


#endif /* __RTPSERV_H__ */
