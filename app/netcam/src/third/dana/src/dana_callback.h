#ifndef DANA_CALLBACK_INCLUDE_H
#define DANA_CALLBACK_INCLUDE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

//#include "debug.h"
#include "danavideo.h"
#include "danavideo_cmd.h"
#include "danavideo_cloud.h"
#include "dana_access_export.h"

#include "media_fifo.h"


#define MAX_RETRY_TIMES 5
#define DANACLOUD_MAX_BUFFER_SIZE 1*1024*1024
#define	DANACLOUD_MAX_PACKET_SIZE 5*1024*1024

#ifdef GK7101
#define DANALIB_MAX_BUFFER_SIZE  1536*1024        //1.5M
#else
#define DANALIB_MAX_BUFFER_SIZE  700*1024        //700KB
#endif
#define DANALIB_STARTUP_TIMEOUT 30
#define DANA_SENDING_TIMEOUT 1000*1000


#define DANACLOUD_CHAN_NO         1


typedef struct __dana_access
{
	uint32_t chan_video;
	uint32_t chan_audio;
    pdana_video_conn_t *danavideoconn;

    volatile bool run_video_media;
    pthread_t thread_video_media;

    volatile bool run_audio_media; 
	MEDIABUF_HANDLE handle;
	volatile bool run_talkback;   
	volatile bool exit_talback;
	volatile bool playback;
	pthread_t thread_talkback;
	pthread_t thread_playback;
}DANA_ACCESS;

typedef struct __media_frame
{
	int type;
	int encode_type;
	int iskey;
	int size;
	unsigned int timestamp;
	char* buffer;
}MEDIA_FRAME;

typedef struct __danacloud_realtime_upload_core
{
    volatile bool run_realtime_upload;
    pthread_t     thread_realtime_upload;
	MEDIABUF_HANDLE handle;
}danacloud_realtime_upload_core_t;


extern dana_video_callback_funs_t danavideocallbackfuns;
extern void danacloud_mode_changed(const danavideo_cloud_mode_t cloud_mode);

#endif //DANA_CALLBACK_INCLUDE_H

