/*!
*****************************************************************************
** \file        goolink/audioRecorder.c
**
** \brief       goolink application
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2019-2020 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <time.h>
#include <sys/prctl.h>

#include "cfg_system.h"
#include "netcam_api.h"
#include "media_fifo.h"
#include "cfg_user.h"
#include "cfg_video.h"
#include "cfg_ptz.h"
#include "ntp_client.h"
//#include "crc32.h"
#include "parser_cfg.h"
#include "flash_ctl.h"
#include "work_queue.h"
#include "mmc_api.h"
#include "avi_search_api.h"
#include "avi_utility.h"
#include "sdk_isp.h"
#include "sdk_audio.h"
#include "sdk_api.h"

#include "audioPlayer.h"

#include "goolink_apiv2.h"
#include "audioRecorder.h"


//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************

#define LT_RED          "\033[0;32;31m"
#define LT_GREEN        "\033[0;32;32m"
#define LT_BLUE         "\033[0;32;34m"
#define LT_YELLOW       "\033[1;33m"
#define LT_WHITE        "\033[1;37m"

#define GOOLINK_AUDIO_DEBUG
#ifdef GOOLINK_AUDIO_DEBUG
#define lt_printk(S,...)      printf("\n\n%slt:[%s-%d]"S"\033[0;0m\n",LT_GREEN,__func__,__LINE__, ##__VA_ARGS__)
#define lt_error(S,...)       printf("\n\n%slt:[%s-%d]"S"\033[0;0m\n",LT_RED,__func__,__LINE__, ##__VA_ARGS__)
#define lt_warning(S,...)     printf("\n\n%slt:[%s-%d]"S"\033[0;0m\n",LT_YELLOW,__func__,__LINE__, ##__VA_ARGS__)
#else
#define lt_printk(S,...)
#define lt_error(S,...)       printf("\n\n%slt:[%s-%d]"S"\033[0;0m\n",LT_RED,__func__,__LINE__, ##__VA_ARGS__)
#define lt_warning(S,...)
#endif

#define LTAUDIO_FRAME_SAMPLE 640


//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************

typedef struct _cmsAaioInstStr
{
    pthread_t 				stPid;
	int32_t					aiothread_stop;
	int32_t					UsrInst;
	int32_t					(*rFun)(void *_writer, const void *_data, unsigned long _sampleCout);
	int32_t					CapEn;
	int32_t					sample_bits;
	MEDIABUF_HANDLE ltVoiceMediabufHandle;
}AioInst_t, *pAioInst_t;


typedef short   RecvAudioType;


//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************

static pthread_mutex_t ltAudioMutex;

//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************

static void *voiceReadProc(void *piparam);
static inline int lt_audio_enter_lock(void);

static inline int lt_audio_enter_unlock(void);

static inline int lt_audio_enter_destrory(void);


//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************


static inline int lt_audio_enter_lock(void)
{
	pthread_mutex_lock(&ltAudioMutex);
}

static inline int lt_audio_enter_unlock(void)
{
	pthread_mutex_unlock(&ltAudioMutex);
}

static inline int lt_audio_enter_destrory(void)
{
	return pthread_mutex_destroy(&ltAudioMutex);
}

#if 0
int gk_audio_input_enable(bool flag)
{
    if(flag)
    {
		/* enable AI*/
		if (gadi_audio_ai_enable()){
			lt_error("Enable ai device failed \n");
			return -1;
		}

#ifdef AUDIO_ECHO_CANCELLATION_SUPPORT
		if(gadi_audio_ai_aec_enable()) {
			lt_error("Enable ai aec failed \n");
			return -1;
		}
#endif

    }
    else
    {
		/* disable AI*/
		if (gadi_audio_ai_disable()){
			lt_error("disable ai device failed \n");
			return -1;
		}

#ifdef AUDIO_ECHO_CANCELLATION_SUPPORT
		if(gadi_audio_ai_aec_disable()) {
			lt_error("disable ai aec failed \n");
			return -1;
		}
#endif


    }

    return 0;
}

#endif
int LT_voice_restore_init_status(int rateMode)
{
	int ret = 0;


	return ret;
}

int LT_voiceInitRecorder(int _sampleRateInHz, int _channel, int _audioFormat,
														int _bufferSize, void **_precorder)
{
	int ret = 0;
	pAioInst_t tpInst = NULL;
	pthread_mutex_init(&ltAudioMutex,NULL);
	
	tpInst = (pAioInst_t)malloc(sizeof(AioInst_t));
	if (tpInst == NULL){
		lt_error("malloc tpInst failed! \n");
		return -1;
	}
	memset(tpInst,0,sizeof(AioInst_t));
	tpInst->sample_bits = _audioFormat;

	if (tpInst->stPid == 0) {
		tpInst->aiothread_stop = 1;
		if (pthread_create(&tpInst->stPid, NULL, voiceReadProc, (void *)tpInst))
		{
			lt_error("create voiceReadProc fail!\n");
			return -1;
		}
		
	}

	*_precorder = (int)tpInst;

	return ret;
}

int LT_voiceStartRecord(void *_recorder, void *_writer, r_pwrite _pwrite)
{

	pAioInst_t tpInst = (pAioInst_t)_recorder;
	int s32ret = 0;

	if (tpInst == NULL){
		lt_error("tpInst is NULL\n");
		return -1;
	}
	lt_audio_enter_lock();
	tpInst->UsrInst = _writer;
	tpInst->rFun    = _pwrite;
	tpInst->CapEn   = 1;
	lt_audio_enter_unlock();

	return 0;

}

int LT_voiceStopRecord(void *_recorder)
{
	pAioInst_t tpInst = (pAioInst_t)_recorder;

	if (tpInst == NULL)
		return -1;
	
	lt_audio_enter_lock();
	tpInst->CapEn = 0;
	lt_audio_enter_unlock();

	return 0;

}


int LT_voiceReleaseRecorder(void *_recorder)
{
	pAioInst_t tpInst = (pAioInst_t)_recorder;
	int s32ret;
	lt_error("***************** LT_voiceReleaseRecorder***************\n");

	if (tpInst == NULL)
		return -1;
	
	lt_audio_enter_lock();

	if (tpInst->stPid != 0)
	{
		tpInst->aiothread_stop = 1;

		pthread_join(tpInst->stPid,0);
	}

	if (tpInst->ltVoiceMediabufHandle != NULL)
		mediabuf_del_reader(tpInst->ltVoiceMediabufHandle);
	
	if (tpInst != NULL)
	{
		free (tpInst);
		tpInst= NULL;
	}
	lt_audio_enter_unlock();


	lt_audio_enter_destrory();
	return 0;

}



int LT_voiceCreatePlayer(int _sampleRateInHz, int _channel,
									int _audioFormat, int _bufferSize, void **_pplay)
{

}

int LT_voiceStartPlay(void *_player)
{

}

int LT_voiceStopPlay(void *_player)
{

}

int LT_voiceWritePlayer(void *_player, char *_audioData, int _dataLen)
{

}

int LT_voiceFlushPlayer(void *_player)
{

}

int LT_voiceReleasePlayer(void *_player)
{

}







////////////////////////////////////////////////////////////////////
///////////////////   locate funcation       //////////////////////
//////////////////////////////////////////////////////////////////
static void *voiceReadProc(void *piparam)
{
	int ret           = -1;
	char *dataBuf     = NULL;
	int data_size     = 0;
	int readlen       = 0;
	pAioInst_t tpInst = (pAioInst_t)piparam;
	int outPutLen     = 0;
	int    i          = 0;
	char ltAoFramebuf[LTAUDIO_FRAME_SAMPLE];	
	int ltAoFrameLen  = LTAUDIO_FRAME_SAMPLE;	
	
	lt_printk("voiceReadProc running... 0x%x\n",tpInst);

	GK_NET_FRAME_HEADER header     = {0};
	MEDIABUF_HANDLE mediabufHandle = NULL;

    pthread_detach(pthread_self());
    sdk_sys_thread_set_name("voiceReadProc");
	
	lt_audio_enter_lock();

	tpInst->ltVoiceMediabufHandle = mediabuf_add_reader(GK_NET_STREAM_TYPE_STREAM0);
	if(tpInst->ltVoiceMediabufHandle == NULL)
	{
		lt_error("mediabuf_add_reader failed\n");
		lt_audio_enter_unlock();
		return NULL;
	}
	tpInst->aiothread_stop = 0;
	lt_audio_enter_unlock();	

	
	while(tpInst->aiothread_stop == 0)
	{
		while(netcam_get_update_status() < 0)
    	{
            lt_error("wait at updating\n");
    		sleep(3);
    	}
		
		data_size = 0;
		dataBuf   = NULL;
		
		lt_audio_enter_lock();
		if (tpInst->ltVoiceMediabufHandle){
			readlen = mediabuf_read_frame(tpInst->ltVoiceMediabufHandle, (void **)&dataBuf, &data_size, &header);
		}
		lt_audio_enter_unlock();
		
		if (readlen <= 0)
    	{
			lt_error("mediabuf_read_frame failed\n");
			usleep(300*1000);
			continue;
    	}
		else if(readlen > 0)
		{
			if (header.frame_type != GK_NET_FRAME_TYPE_A)
				continue;

			if (header.frame_type == GK_NET_FRAME_TYPE_A)
			{
				lt_audio_enter_lock();
				if (tpInst->rFun && tpInst->CapEn)
				{	
					outPutLen = (data_size > ltAoFrameLen) ? ltAoFrameLen : data_size;
					memset(ltAoFramebuf,0,ltAoFrameLen);
					memcpy(ltAoFramebuf,dataBuf,outPutLen);
					ret = tpInst->rFun(tpInst->UsrInst, ltAoFramebuf, ltAoFrameLen/(tpInst->sample_bits/8));
					if (ret < 0)
					{
						lt_error("tpInst->rFun send frame failed! ret = %d\n",ret);
						lt_audio_enter_unlock();
						return NULL;
					}
						
					lt_audio_enter_unlock();
				}else
					lt_audio_enter_unlock();
					
			}

		}

		usleep(1);// ÈÃ³öCPU×ÊÔ´

	}
	
	return NULL;
}


