/*!
*****************************************************************************
** FileName     : sdk_enc.h
**
** Description  : config for alarm in, alarm out.
**
** Author       : hyb <heyongbin@gokemicro.com>
** Create       : 2015-7-29
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef GK_ENC_H_
#define GK_ENC_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sdk_def.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <time.h>
#include <poll.h>

/* audio or video encoding type */
typedef enum  {
	// video
	GK_ENC_DATA_H264 = (0x1),
	GK_ENC_DATA_JPEG,
	GK_ENC_DATA_H265 = (0x3),

	// audio
	GK_ENC_DATA_PCM = (0X11),
	GK_ENC_DATA_G711A,
	GK_ENC_DATA_G711U,
	GK_ENC_DATA_G726
}EM_GK_ENC_DATA_TYPE ;



typedef struct  {

	EM_GK_ENC_DATA_TYPE type;       // data type
	unsigned int time_us;               // the timestamp of system clock, unit: us
	void *data;                     // data pointer
	uint32_t data_sz;                 // data size
	int no;
	union {
		//
		struct {
			bool keyframe;          // TRUE / FALSE
			uint32_t fps;           // frame rate
			uint32_t width;         // width
			uint32_t height;        // height
		}video;
		// kSdkEncBufDataPcm, kSdkEncBufDataG711a, kSdkEncBufDataG711u
		struct {
			uint32_t sample_rate;   // sample rate
			uint32_t sample_width;  // sample width
		}audio;
	};
}ST_GK_ENC_BUF_ATTR, *PS_GK_ENC_BUF_ATTR;


//////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////
/*H264 profile type*/
typedef enum
{
    GK_ENC_H264_PROFILE_AUTO = 0,
    GK_ENC_H264_PROFILE_BASELINE,
    GK_ENC_H264_PROFILE_MAIN,
    GK_ENC_H264_PROFILE_HIGH
}ST_GK_ENC_H264_PROFILE_ATTR, *PS_GK_ENC_H264_PROFILE_ATTR;

/*H264 bit rate control mode*/
typedef enum
{
 GK_ENC_H264_RC_MODE_CBR = 0,
 GK_ENC_H264_RC_MODE_VBR ,
 GK_ENC_H264_RC_MODE_CBR_QUALITY,
 GK_ENC_H264_RC_MODE_VBR_QUALITY
}ST_GK_ENC_H264_RC_ATTR, *PS_GK_ENC_H264_RC_ATTR;

typedef struct  {
    SDK_U32 width;
    SDK_U32 height;
    SDK_U32 fps;
    SDK_U32 pi;
    SDK_U8  mirrorPattern;
    SDK_U8  bayerPattern;
}ST_GK_VI_ATTR, *PST_GK_VI_ATTR;

typedef struct  {
    /*specify vout channel.*/
    SDK_U8 voutChannel;
    /*video output resolution mode,*/
    SDK_U8 resoluMode;
    /*video output device type.*/
    SDK_U8 deviceType;
}ST_GK_VOUT_ATTR, *PST_GK_VOUT_ATTR;

/* video encoding property */
typedef struct  {
	char name[32];  /*stream name*/
    int  enctype;
	int  width;     /*stream width*/
    int  height;    /*stream height*/
	int  fps;       /*encoding frame rate*/
    int  gop;       /*the interval of key I frame*/
	int  profile;   /* encoding level*/
	int  rc_mode;   /*0: CBR; 1: VBR; 2: CBR keep quality; 3: VBR keep quality: GADI_VENC_BrcModeEnumT*/
	int  bps;       /* bitrate value, unit:kbps*/
	int  quality;   /* quality mode,0-3, 3 is best,0 is lowest*/
    int  re_rec_mode;
//	int  stream_switch; /* 0 send audio and video, 1 : only send video , 2: not send audio and video */
}ST_GK_ENC_STREAM_H264_ATTR, *PS_GK_ENC_STREAM_H264_ATTR;


typedef struct
{
    /*stream index.*/
    unsigned int             streamId;
    /*rate control factor. value:1~51, qpMinOnI <= qpMinOnP.*/
    unsigned char            qpMinOnI;
    /*rate control factor.qpMinOnI <= qpMaxOnI*/
    unsigned char            qpMaxOnI;
    /*rate control factor. value:1~51, qpMinOnI <= qpMinOnP.*/
    unsigned char            qpMinOnP;
    /*rate control factor.qpMinOnP <=  qpMaxOnP*/
    unsigned char            qpMaxOnP;
    /*rate control factor: I frame qp weight, range: 1~10,*/
    unsigned char            qpIWeight;
    /*rate control factor. P frame qp weight, range: 1~5.*/
    unsigned char            qpPWeight;
    /*picture quality consistency, range: 0~2. 2:will be best.*/
    unsigned char            adaptQp;
} ST_VENC_H264QP_CONF; 




/*audio encoding property */
typedef struct  {
	int ain; // ignore when init
	int vin_ref; // ignore when init
	int sample_rate, sample_width;
	size_t packet_size;
    EM_GK_ENC_DATA_TYPE enctype;
}ST_GK_ENC_STREAM_AUDIO_ATTR, *PS_GK_ENC_STREAM_AUDIO_ATTR;

/*jpeg snapshot quality */
typedef enum  {
	GK_ENC_SNAPSHOT_QUALITY_HIGHEST,
	GK_ENC_SNAPSHOT_QUALITY_HIGH,
	GK_ENC_SNAPSHOT_QUALITY_MEDIUM,
	GK_ENC_SNAPSHOT_QUALITY_LOW,
	GK_ENC_SNAPSHOT_QUALITY_LOWEST,
}EM_GK_ENC_SNAPSHOT_QUALITY;

/*check sensor resolution mode */
typedef enum
{
	SENSOR_1920_1080P = 0,
	SENSOR_1280_960P = 1,
	SENSOR_1280_720P = 2,
}EM_ENC_MAX_RES_MODE;

/*when snapshot jpeg frame success, sdk_enc_snapshot can call this function*/
typedef void (*GK_ENC_SNAPSHOT_CALLBACK)(char *data, int size);

/*the sdk receive one audio or video frame, sdk will call this function.*/
typedef void (*GK_ENC_BUG_CALLBACK)(int stream, PS_GK_ENC_BUF_ATTR frameBuf);


/* auto use max resolution */
#define GK_ENC_SNAPSHOT_SIZE_MAX (-1)
/* auto use min resolution */
#define GK_ENC_SNAPSHOT_SIZE_MIN (-2)

/*!
*******************************************************************************
** \brief Encoding module init
**
** \param[in] cb            the frame data of callback
**
** \return
** - #0         success
** - #other     failed
*******************************************************************************
*/
int sdk_enc_init();
int sdk_vi_set_params(ST_GK_VI_ATTR *vi);
int sdk_vi_get_params(ST_GK_VI_ATTR *vi);
int sdk_vout_set_params(ST_GK_VOUT_ATTR *vout);

int sdk_set_get_video_stream_cb(GK_ENC_BUG_CALLBACK cb);
int sdk_set_get_auido_stream_cb(GK_ENC_BUG_CALLBACK cb);

/*!
*******************************************************************************
** \brief Encoding module exit
**
** \return
** - #0         success
** - #other     failed
*******************************************************************************
*/
int sdk_enc_destroy();


/*!
*******************************************************************************
** \brief Create H264 stream, set parameter and frame's cb
**
** \param[in] vin           vin's id
** \param[in] stream        stream's id in vin
** \param[in] h264_attr     stream's attr

**
** \return
** - #0         success
** - #other     failed
**
*******************************************************************************
*/

int sdk_enc_create_stream_h264(int vin, int stream, PS_GK_ENC_STREAM_H264_ATTR h264_attr, ST_VENC_H264QP_CONF *qp_attr);

/*!
*******************************************************************************
** \brief Close H264 stream
**
** \param[in] vin           vin's id
** \param[in] stream        stream's id in vin
**
** \return
** - #0         success
** - #other     failed
**
*******************************************************************************
*/
int sdk_enc_release_stream_h264(int vin, int stream);

/*!
*******************************************************************************
** \brief control H264 stream encoding
**
** \param[in] vin           vin's id
** \param[in] stream        stream's id in vin
** \param[in] flag          0,disable,1 enable
**
** \return
** - #0         success
** - #other     failed
**
*******************************************************************************
*/
int sdk_enc_enable_stream_h264(int vin, int stream, int flag);


/*!
*******************************************************************************
** \brief set H264 stream encoding property
**
** \param[in] vin           vin's id
** \param[in] stream        stream's id in vin
** \param[in] h264_attr     encoding property
**
** \return
** - #0         success
** - #other     failed
**
*******************************************************************************
*/
int sdk_enc_set_stream_h264(int vin, int stream, PS_GK_ENC_STREAM_H264_ATTR h264_attr);

/*!
*******************************************************************************
** \brief get H264 stream encoding property
**
** \param[in] vin           vin's id
** \param[in] stream        stream's id in vin
** \param[out] h264_attr     encoding property
**
** \return
** - #0         success
** - #other     failed
**
*******************************************************************************
*/
int sdk_enc_get_stream_h264(int vin, int stream, PS_GK_ENC_STREAM_H264_ATTR h264_attr);

/*!
*******************************************************************************
** \brief Request soc produce one key frame
**
** \param[in] vin           vin's id
** \param[in] stream        stream's id in vin
**
** \return
** - #0         success
** - #other     failed
**
*******************************************************************************
*/
int sdk_enc_request_stream_h264_keyframe(int vin, int stream);


/*!
*******************************************************************************
** \brief Start encoding, receive audio and video frame from driver
**
** \return
** - #0         success
** - #other     failed
**
*******************************************************************************
*/
int sdk_video_enc_start(void);

/*!
*******************************************************************************
** \brief Stop encoding, don't receive audio and video frame from driver
**
** \return
** - #0         success
** - #other     failed
**
*******************************************************************************
*/
int sdk_video_enc_stop(void);


/*!
*******************************************************************************
** \brief Start encoding, receive audio frame from driver
**
** \return
** - #0         success
** - #other     failed
**
*******************************************************************************
*/
int sdk_audio_enc_start(void);

/*!
*******************************************************************************
** \brief Stop encoding, don't receive audio frame from driver
**
** \return
** - #0         success
** - #other     failed
**
*******************************************************************************
*/
int sdk_audio_enc_stop(void);

/*!
*******************************************************************************
** \brief snapshot one jpeg, width and height can set min or max, api will auto
**        adjust the picture's width and height. we usually use enoding stream's size for a capture.
**        if you don't use stream's size, the jpeg will one part of stream.
**
** \param[in] vin           video input channel id
** \param[in] quality       jpeg's quality
** \param[in] width         jpeg's width
** \param[in] height        jpeg's height
** \param[in] cb            call this function when snapshot success

** \return
** - #0         success
** - #other     failed
**
*******************************************************************************
*/
int sdk_enc_snapshot(int vin, EM_GK_ENC_SNAPSHOT_QUALITY quality, ssize_t width, ssize_t height, GK_ENC_SNAPSHOT_CALLBACK cb);


/*!
*******************************************************************************
** \brief get system encoding max resolution

** \return
** - # EM_ENC_MAX_RES_MODE
**
*******************************************************************************
*/
EM_ENC_MAX_RES_MODE sdk_enc_max_resolution_mode();


/*!
*******************************************************************************
** \brief get stream's frame rate

** \return
** - # frame rate
**
*******************************************************************************
*/
int sdk_enc_get_stream_fps(int vin,int stream);
int sdk_venc_get_stream_resolution(int stream_id, int *width, int *height);


/*!
*******************************************************************************
** \brief force I frame for stream id

** \return
** - # frame rate
**
*******************************************************************************
*/

int sdk_video_enc_force_i_frame(int stream_id);

void sdk_set_h26x_smart(int enable, int gopInterval);

typedef void (*audio_proc_callback)(unsigned char *frame_addr, unsigned int frame_len);
void sdk_set_audio_proc_callback(audio_proc_callback callback);

#ifdef __cplusplus
};
#endif
#endif //GK_ENC_H_

