#include "debug.h"
#include "dana_callback.h"
#include "ntp_client.h"
#include "cJSON.h"
#include "cfg_common.h"
#include "cfg_network.h"
#include "cfg_image.h"
#include "cfg_video.h"
#include "netcam_api.h"
#include "mmc_api.h"
#include "work_queue.h"
#include "avi_common.h"
#include "avi_utility.h"
#include "eventalarm.h"

#include "avi_search_api.h"

unsigned long long start_time;
static int dana_is_step;
static int dana_is_stop;
static int dana_avi_flag_pause;
int week_re[7] = {0};
int week_Count = 0;
static int id = 0;
static int his_dana_playback = 0;
static int his_dana_playbacking = 0;
static FILE_NODE dana_node;
static DANA_ACCESS *acs;
static int stream_dana_id = 0;
static sem_t  dana_step_sem;
static AviPBHandle *pDANAPBHandle = NULL;
static pthread_mutex_t pb_dana_getframe_x = PTHREAD_MUTEX_INITIALIZER;

typedef struct tagJBNV_TIME{
	unsigned long dwYear;
	unsigned long dwMonth;
	unsigned long dwDay;
	unsigned long dwHour;
	unsigned long dwMinute;
	unsigned long dwSecond;
}JBNV_DANA_TIME;

typedef struct tagJBNV_FindFileResp{
	unsigned long	dwSize;
	unsigned long	nResult;		//0:success ;1:find file error ; 2:the number of file more than the memory size, and the result contains part of the data
	unsigned long	nCount;
	unsigned long	dwReserved[3];
}JBNV_DANA_FIND_FILE_RESP;

typedef struct tagJBNV_FILE_DATA_INFO{
	char		sFileName[256];			//文件名
	JBNV_DANA_TIME	BeginTime;				//
	JBNV_DANA_TIME	EndTime;
	unsigned long	nChannel;
	unsigned long	nFileSize;				//文件的大小
	unsigned long	nState;					//文件转储状态
}JBNV_DANA_FILE_DATA_INFO;

static void split( char **arr, char *str, const char *del)//字符分割函数的简单定义和实现
{
	 char *s =NULL;

	 s=strtok(str,del);
	 while(s != NULL)
	 {
	  *arr++ = s;
	  s = strtok(NULL,del);
	 }
}

void danavideo_sd_record(void)
{
	int rec_time_once = runRecordCfg.recordLen;
    printf("manu rec stream %d start. once_len:%d\n", runRecordCfg.stream_no, rec_time_once);
    manu_rec_start_alltime(runRecordCfg.stream_no, rec_time_once);
    return 0;

}

void danavideo_motion(void){
	uint32_t ch_no = 0;
	uint32_t alarm_level = 2;//DANA_VIDEO_PUSHMSG_ALARM_LEVEL_2;
	uint32_t msg_type = 1;//DANA_VIDEO_PUSHMSG_MSG_TYPE_MOTION_DETECT ; 移动侦测
	char     *msg_title = "danale motion";
	char     *msg_body  = "lib_danavideo_util_pushmsg";
	int64_t  cur_time = 0;

#if 1	// 不设置附件. 不设置录像
	uint32_t att_flag = 0;
	uint32_t record_flag = 0;
	if (lib_danavideo_util_pushmsg(ch_no, alarm_level, msg_type, msg_title, msg_body, cur_time, att_flag, NULL, NULL, record_flag, 0, 0, 0, NULL)) {
			printf("DANA_VIDEO_PUSHMSG_MSG_TYPE_MOTION_DETECT SUCCEED!\n");
	} else {
			printf("DANA_VIDEO_PUSHMSG_MSG_TYPE_MOTION_DETECT FAILED!\n");
	}
#endif
}

static int dana_record_history_stop()
{

	his_dana_playback = 0;

	while(his_dana_playbacking == 1)
	{
		usleep(100000); //wait shend data close
	}

	PRINT_INFO("record_history_stop succeed!\n");
	return 0;
}

static void *dana_pb_thread(void *param)
{
	int ret = 0;
	dana_is_stop = 0;
    dana_avi_flag_pause = 0;
    dana_is_step = 0;
	DANA_ACCESS *acs = (DANA_ACCESS *)param;
	PRINT_INFO("%d...%d\n",his_dana_playback,his_dana_playbacking);
	while(acs->playback)
	{
		/*while(dana_avi_flag_pause)
            sleep(2);
        if (dana_is_stop)
		{
            printf("to stop.\n");
			if(pDANAPBHandle)
		    {
		    	PRINT_INFO("stop rec handle for dana");
		   		dana_pb_stop(pDANAPBHandle);
		   		pDANAPBHandle = NULL;
		    }
		    if(pBuff)
		    {
			   free(pBuff);
			   PRINT_INFO("free pBuff!\n");
			   pBuff = NULL;
		    }
            break;
        }
        if (dana_is_step)
		{
            PRINT_INFO("to step in.\n");
            sem_wait(&dana_step_sem);
        }*/
	   if (his_dana_playback == 0)
	   {
		   if(his_dana_playbacking == 1)
		   {
			   his_dana_playbacking = 0;
			   if(pDANAPBHandle)
			   {
			   		dana_pb_stop(pDANAPBHandle);
					PRINT_INFO("stop rec handle for danale");
			   		pDANAPBHandle = NULL;
			   }

		   }
		   usleep(100000);

	   }
	   else
	   {
		   his_dana_playbacking = 1;
		   ret = dana_pb_send(pDANAPBHandle);

		   if(ret < 0)
		   {
			  PRINT_ERR("send playback video failed %d\n",ret);
			  his_dana_playback = 0;
		   }
	   }
		   //PRINT_INFO("-->%d\n", err);
		   //usleep(40*1000);
	}
    return NULL;

}


static AviPBHandle *dana_pb_bytime(u64t start, u64t stop){
	AviPBHandle *pPBHand = NULL;
	pPBHand = (AviPBHandle *)create_pb_handle();
	if (NULL == pPBHand)
	{
		PRINT_ERR("...\n");
		return NULL;
	}
	pPBHand->list = search_file_by_time(0,0, start, stop);
	if (NULL == pPBHand->list)
	{
		PRINT_ERR("list is null.\n");
		avi_pb_close(pPBHand);
		return NULL;

	}
	//print_pb_handle(pPBHand);
	return pPBHand;
}


int dana_pb_stop(AviPBHandle *pPBHandle)
{
	PRINT_INFO("dana playback stop\n");
    avi_pb_close(pPBHandle);
    return 0;
}

int dana_pb_send(AviPBHandle *DANAPBHandle)
{
	int ret;
	int chan_no ;
    int size = 0;
	uint32_t sample_rate;
	uint32_t track;
	sample_rate = runAudioCfg.sampleRate;
	track = 1;
    if (DANAPBHandle->no >= DANAPBHandle->index_count)
	{
        PRINT_INFO("playback %s end. no = %d\n", DANAPBHandle->file_path, DANAPBHandle->no);
        if (DANAPBHandle->list == NULL)
		{
            PRINT_ERR("playback end");
            return 0;
        }
        memset(&dana_node, 0, sizeof(FILE_NODE));
        ret = get_file_node(DANAPBHandle->list, &dana_node);
        if (ret != DMS_NET_FILE_SUCCESS)
		{
             PRINT_ERR("get_file_node failed, ret = %d\n", ret);
             return -1;
        }
        print_node(&dana_node);
        ret = avi_pb_open(dana_node.path, DANAPBHandle);
        if (ret < 0)
		{
            PRINT_ERR("avi_pb_open %s error.ret %d\n", dana_node.path,ret);
            return 0;
        }
    }
    pthread_mutex_lock(&pb_dana_getframe_x);
    ret = avi_pb_get_frame(DANAPBHandle, NULL, &size);
    pthread_mutex_unlock(&pb_dana_getframe_x);
	if (ret < 0)
	{
        PRINT_ERR("call avi_pb_get_frame failed.\n");
         return -3;
    }
    if (ret == GK_NET_NOMOREFILE)
	{
        PRINT_ERR("playback file end.\n");
         return 0;
    }
	MEDIA_FRAME m_playback;
	if (DANAPBHandle->node.frame_type == 0x11)// I
	{
		m_playback.iskey = 1;
    }
	else
	{
		m_playback.iskey = 0;
    }

	if (DANAPBHandle->node.frame_type == 0) // A
	{
        m_playback.type = audio_stream;
		m_playback.encode_type = G711A;
    }
	else{
		m_playback.type = video_stream;
		m_playback.encode_type = H264;
	}

	chan_no = m_playback.type == audio_stream ? acs->chan_audio : acs->chan_video;

	m_playback.size = size;
	m_playback.timestamp = DANAPBHandle->node.timetick;
	m_playback.buffer = DANAPBHandle->node.one_frame_buf;
	//PRINT_INFO("~~~~~~~~~~~~send video media~~~~~~~~~~~~~~~~~\n");
	if (!lib_danavideoconn_send(acs->danavideoconn, m_playback.type,
			m_playback.encode_type, chan_no, m_playback.iskey, m_playback.timestamp,
			(const char*)m_playback.buffer, m_playback.size, DANA_SENDING_TIMEOUT))
	{
		PRINT_ERR(" send video media failed %d\n",chan_no);
		//dana_record_history_stop();
		usleep(1000*1000);
		return -4;
	}
	if (!lib_danavideo_cmd_startaudio_response(acs->danavideoconn,id,
				0, "", NULL, &sample_rate, NULL, &track))
	{
		printf("[danavideoconn_command_handler] send start audio response failed\n");
		return ;
	}
    return 0;
}
int dana_setvideo(uint32_t video_quality)
{
	ST_GK_ENC_STREAM_H264_ATTR h264Attr;
	memset(&h264Attr,0,sizeof(ST_GK_ENC_STREAM_H264_ATTR));
	if (video_quality >= 1 && video_quality <= 25)
	{
		stream_dana_id = 2;
		netcam_video_get(0,stream_dana_id,&h264Attr);
		h264Attr.bps = video_quality * 100;
	}else if(video_quality >= 26 && video_quality <= 50)
	{
		stream_dana_id = 1;
		netcam_video_get(0,stream_dana_id,&h264Attr);
		h264Attr.bps = video_quality * 100;
	}else if(video_quality >= 51 && video_quality <= 60)
	{
		stream_dana_id = 0;
		netcam_video_get(0,stream_dana_id,&h264Attr);
		h264Attr.bps = video_quality * 100;
	}
	if (netcam_video_set(0,stream_dana_id,&h264Attr) == 0)
	{
		PRINT_INFO("stream_dana_id %d  h264Attr.bps %d\n",stream_dana_id,h264Attr.bps);
		netcam_video_cfg_save();
	}
	return 0;
}

void* th_talkback(void *arg) {
	DANA_ACCESS *acs = (DANA_ACCESS *)arg;
	uint32_t timeout_usec = 1000*1000; // 1000ms
	char dst_buf[1024] = {0};
	int i,len;
	dana_audio_packet_t *pmsg = NULL;
	while (acs->run_talkback)
	{
		len = 0;
		for(i=0;i<6;i++)
		{
			pmsg = lib_danavideoconn_readaudio(acs->danavideoconn, timeout_usec);
			//PRINT_INFO("pmsg->codec %d,%d\n",pmsg->codec,pmsg->len);
			if (pmsg)
			{
				memcpy(dst_buf+i*pmsg->len,pmsg->data,pmsg->len);
				len +=pmsg->len;
				lib_danavideo_audio_packet_destroy(pmsg);
			}
		}
		//PRINT_INFO("send talkback!len %d\n",len);
		netcam_audio_output(dst_buf,len,NETCAM_AUDIO_ENC_A_LAW, SDK_TRUE);
		memset(dst_buf,0,1024);
	}
	PRINT_INFO("th_talkback exit\n");
	acs->exit_talback = true;
	return NULL;
}

static int get_media(MEDIABUF_HANDLE *handle, MEDIA_FRAME* mfr)
{
	int readlen;
	GK_NET_FRAME_HEADER header = {0};

    mfr->buffer = NULL;
    mfr->size = 0;
	readlen = mediabuf_read_frame(handle[stream_dana_id], (void**)&mfr->buffer, &mfr->size, &header);
	if (readlen <= 0)
		return -1;

	if (header.frame_type == GK_NET_FRAME_TYPE_I)
		mfr->iskey = 1;
	else
		mfr->iskey = 0;

	if (header.frame_type == GK_NET_FRAME_TYPE_A)
	{
		mfr->type = audio_stream;
		mfr->encode_type = G711A;
		//mfr->encode_type = PCM;
	}
	else
	{
		mfr->type = video_stream;
		mfr->encode_type = H264;
	}
	mfr->timestamp = header.sec * 1000 + header.usec / 1000000;
	return 0;
}


static void* thread_media(void *arg)
{
	uint32_t chan_no;
	DANA_ACCESS *acs = (DANA_ACCESS *)arg;
	MEDIA_FRAME* mfr;
	MEDIABUF_HANDLE dana_handle[3] = {0};
    int i = 0;
	mfr = (MEDIA_FRAME*)malloc(sizeof(MEDIA_FRAME));
	if (!mfr)
	{
		PRINT_ERR("[thread_media]Fail to alloc space for media frame.\n");
		return 0;
	}
    for(i = 0; i < netcam_video_get_channel_number(); i++)
    {
	    dana_handle[i] = mediabuf_add_reader(i);
        if(dana_handle[i] != 0)
        mediabuf_set_newest_frame(dana_handle[i]);
    }


	while (acs->run_video_media)
	{
		mfr->size = 0;
		//memset(mfr->buffer, 0, MEDIA_FRAME_BUFFER_SIZE);
		if (get_media(dana_handle, mfr))
			continue;

		if (!acs->run_audio_media)           //audio no open
		{
			if (mfr->type == audio_stream)   //discard audio:丢弃音频
				continue;
			else
				chan_no = acs->chan_video;
		}
		else                                 //audio and video open
			chan_no = mfr->type == audio_stream ? acs->chan_audio : acs->chan_video;

		if (!lib_danavideoconn_send(acs->danavideoconn, mfr->type,
			mfr->encode_type, chan_no, mfr->iskey, mfr->timestamp,
			(const char*)mfr->buffer, mfr->size, DANA_SENDING_TIMEOUT))
		{
			//printf("[thread_media] send video media failed\n");
			//usleep(10*1000);

		}
	}
    for(i = 0; i < netcam_video_get_channel_number(); i++)
    {
        if(dana_handle[i] != 0)
            mediabuf_del_reader(i);
    }

	free(mfr);
	PRINT_INFO("[thread_media] video media thread exit\n");
	return 0;
}


static uint32_t danavideoconn_created(void *arg)
{
	DANA_ACCESS* acs;
	pdana_video_conn_t *danavideoconn = (pdana_video_conn_t *)arg;

	acs = (DANA_ACCESS*)malloc(sizeof(DANA_ACCESS));
	if (!acs)
	{
		printf("[danavideoconn_created] Fail to alloc space for dana access!\n");
		return -1;
	}

	memset(acs, 0, sizeof(DANA_ACCESS));
	acs->run_video_media = false;
	acs->run_audio_media = false;
	acs->run_talkback = false;
	acs->exit_talback = true;
	acs->playback = false;
	acs->danavideoconn = (pdana_video_conn_t *)arg;

	if (lib_danavideo_set_userdata(danavideoconn, acs))
	{
		free(acs);
		PRINT_ERR("[danavideoconn_created] Fail to set user data.\n");
		return -1;
	}

	//hread_t playback_id;
    /*pthread_attr_t attr_playback;
	pthread_attr_init(&attr_playback);
    	pthread_attr_setdetachstate(&attr_playback, PTHREAD_CREATE_DETACHED);//detached

	if (!acs->playback)
	{
		if(0 != pthread_create(&acs->thread_playback,&attr_playback,dana_pb_thread, NULL))
	    {
	        pthread_attr_destroy(&attr_playback);
	        PRINT_INFO("ulucu p2p playback start error\n");
	        return -1;
	    }
		acs->playback = true;
		pthread_attr_destroy(&attr_playback);
	}*/
	PRINT_INFO("[danavideoconn_created] connection succeeded\n");
	return 0;
}

static void danavideoconn_aborted(void *arg)
{
	DANA_ACCESS* acs;
	pdana_video_conn_t *danavideoconn = (pdana_video_conn_t *)arg;
	//dana_record_history_stop();
	PRINT_INFO("[danavideoconn_aborted] connection aborted\n");
	if (lib_danavideo_get_userdata(danavideoconn, (void **)&acs))
	{
		PRINT_ERR("[danavideoconn_aborted] Fail to get user data.\n");
		return;
	}

	if (!acs)
		return;
	his_dana_playback = 0;
	his_dana_playbacking = 0;
	// stop media thread
	acs->run_video_media = false;
	acs->run_audio_media = false;
	if (acs->thread_video_media)
		pthread_join(acs->thread_video_media, NULL);
	acs->thread_video_media = 0;

	//stop talkback thread
	acs->run_talkback = false;
	acs->exit_talback = true;
	if (acs->thread_talkback)
		pthread_join(acs->thread_talkback, NULL);
	acs->thread_talkback = 0;
	//stop playback thread
	acs->playback = false;
	if (acs->thread_playback)
		pthread_join(acs->thread_playback, NULL);
	acs->thread_playback = 0;
	free(acs);
	return;
}

static void danavideoconn_command_handler(void *arg, dana_video_cmd_t cmd,
	uint32_t trans_id, void *cmd_arg, uint32_t cmd_arg_len)
{
	uint32_t error_code = 0;
	char *code_msg = (char *)"";
	pdana_video_conn_t *danavideoconn;
	id = trans_id;
	danavideoconn = (pdana_video_conn_t *)arg;
	PRINT_INFO("[danavideoconn_command_handler] cmd[%d], trans_id[%d], cmd_arg_len[%d]\n",
		cmd, trans_id, cmd_arg_len);

	if (lib_danavideo_get_userdata(danavideoconn, (void**)&acs) != 0)
	{
		PRINT_ERR("[danavideoconn_command_handler] Fail to get user data\n");
		return;
}

	switch (cmd)
	{
	case DANAVIDEOCMD_STARTAUDIO:
		{
			uint32_t sample_rate;
			uint32_t track; // (1 mono; 2 stereo)1.单声道  2.立体声

			DANAVIDEOCMD_STARTAUDIO_ARG *startaudio_arg = (DANAVIDEOCMD_STARTAUDIO_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler] start audio. ch_no[%u]\n", startaudio_arg->ch_no);
			if (acs->run_audio_media)
			{
				PRINT_ERR("[danavideoconn_command_handler] audio media ch_no[%u] is already started.\n",
					startaudio_arg->ch_no);
				return ;
			}

			acs->chan_audio = startaudio_arg->ch_no;
			acs->run_audio_media = true;

			sample_rate = runAudioCfg.sampleRate;
			track = 1;

			if (!lib_danavideo_cmd_startaudio_response(danavideoconn, trans_id,
				error_code, code_msg, NULL, &sample_rate, NULL, &track))
			{
				PRINT_ERR("[danavideoconn_command_handler] send start audio response failed\n");
				return ;
			}
			PRINT_INFO("[danavideoconn_command_handler] send start audio response succeeded\n");
		}
		break;

	case DANAVIDEOCMD_STARTVIDEO://开启视频
		{
			uint32_t start_video_fps = 25;
			DANAVIDEOCMD_STARTVIDEO_ARG *startvideo_arg = (DANAVIDEOCMD_STARTVIDEO_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler] start video. ch_no[%u], client_type[%d], "
				"quality[%d], vstrm[%d]\n", startvideo_arg->ch_no, startvideo_arg->client_type,
				startvideo_arg->video_quality, startvideo_arg->vstrm);
			dana_setvideo(startvideo_arg->video_quality);
			if (acs->run_video_media)
			{
				PRINT_ERR("[danavideoconn_command_handler] video ch_no[%u] is already started.\n",
					startvideo_arg->ch_no);
				return;
			}
			acs->run_video_media = true;
			acs->chan_video= startvideo_arg->ch_no;

			if (pthread_create(&acs->thread_video_media, NULL, thread_media, acs))
			{
				acs->thread_video_media = 0;
				acs->run_video_media = false;

				PRINT_ERR("[danavideoconn_command_handler] fail to create video thread.\n");
				return ;
			}
			PRINT_INFO("[danavideoconn_command_handler] video thread is started!\n");


			if (!lib_danavideo_cmd_startvideo_response(danavideoconn, trans_id,
				error_code, code_msg, start_video_fps))
			{
				PRINT_ERR("[danavideoconn_command_handler] send video response failed\n");
				return;
			}
			PRINT_INFO("[danavideoconn_command_handler] send video response succeeded\n");
		}
		break;

	case DANAVIDEOCMD_STOPAUDIO://停止音频
		{
			DANAVIDEOCMD_STOPAUDIO_ARG *stopaudio_arg = (DANAVIDEOCMD_STOPAUDIO_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler] stop audio. ch_no[%u]\n", stopaudio_arg->ch_no);

			acs->run_audio_media = false;

			if (!lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id,
				error_code, code_msg))
			{
				PRINT_ERR("[danavideoconn_command_handler] send stop audio response failed\n");
				return;
			}
			PRINT_INFO("[danavideoconn_command_handler] send stop audio response succeeded\n");
		}
		break;

	case DANAVIDEOCMD_STOPVIDEO://停止视频
		{
			DANAVIDEOCMD_STOPVIDEO_ARG *stopvideo_arg = (DANAVIDEOCMD_STOPVIDEO_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler] stop video. ch_no[%u]\n", stopvideo_arg->ch_no);

			acs->run_audio_media = false;
			acs->run_video_media = false;
			if (acs->thread_video_media == 0)
				return;

			pthread_join(acs->thread_video_media, NULL);
			acs->thread_video_media = 0;
			mediabuf_del_reader(acs->handle);

			if (!lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id,
				error_code, code_msg))
			{
				PRINT_ERR("[danavideoconn_command_handler] send stop video response failed\n");
				return;
			}
			PRINT_INFO("[danavideoconn_command_handler] send stop video response succeeded\n");
		}
		break;

	case DANAVIDEOCMD_GETBASEINFO:
		{
			DANAVIDEOCMD_GETBASEINFO_ARG *getbaseinfo_arg = (DANAVIDEOCMD_GETBASEINFO_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler] get device information. ch_no[%d]\n",
				getbaseinfo_arg->ch_no);

			char *dana_id = lib_danavideo_deviceid();
			char *api_ver = (char *)runSystemCfg.deviceInfo.softwareVersion;
			char *sn      = (char *)runSystemCfg.deviceInfo.serialNumber;
			char *device_name = (char *)runSystemCfg.deviceInfo.deviceName;
			char *rom_ver = (char *)"di3_5";
			uint32_t device_type = 1;
			uint32_t ch_num = GK_NET_STREAM_TYPE_STREAM3 + 1;
			uint64_t size = grd_sd_get_all_size();
			uint64_t free = grd_sd_get_free_size();
			uint64_t sdc_size = size * 1024 * 1024;
			uint64_t sdc_free = free * 1024 * 1024;
			if (!lib_danavideo_cmd_getbaseinfo_response(danavideoconn, trans_id,
				error_code, code_msg, dana_id, api_ver, sn, device_name, rom_ver,
				device_type, ch_num, sdc_size, sdc_free))
			{
				PRINT_ERR("[danavideoconn_command_handler] send device info failed\n");
				return;
			}
			PRINT_INFO("[danavideoconn_command_handler] send device info succeeded\n");
		}

		break;

	case DANAVIDEOCMD_GETTIME://获取时间
		{
			int64_t now_time;
			char time_zone[64] = {0};
			int zone;
			DANAVIDEOCMD_GETTIME_ARG *gettime_arg =
				(DANAVIDEOCMD_GETTIME_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]get time. ch_no[%d]\n",
				gettime_arg->ch_no);

			now_time = time(NULL);
			zone = runSystemCfg.timezoneCfg.timezone;
			zone = zone < 0 ? 0-zone : zone;
			snprintf(time_zone, sizeof(time_zone), "GMT%c%02d:%02d",
				runSystemCfg.timezoneCfg.timezone < 0 ? '-' : '+',
				zone / 60, zone % 60);

			PRINT_INFO("[danavideoconn_command_handler]zone=%s\n", time_zone);
			//ntp_param ntp;
			//read_ntp_param(&ntp);
			char *ntp_server_1 = runSystemCfg.ntpCfg.serverDomain;
			char *ntp_server_2 = "NULL";
			if (!lib_danavideo_cmd_gettime_response(danavideoconn, trans_id,
				error_code, code_msg, now_time, time_zone,
				ntp_server_1, ntp_server_2))
			{
				PRINT_ERR("[danavideoconn_command_handler]get time fail\n");
				return;
			}

			PRINT_INFO("[danavideoconn_command_handler]get time succeeded\n");
		}
		break;

	case DANAVIDEOCMD_GETALARM://获取报警信息
		{
			DANAVIDEOCMD_GETALARM_ARG *getalarm_arg =
				(DANAVIDEOCMD_GETALARM_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]get alarm ch_no[%d]\n",
				getalarm_arg->ch_no);
			uint32_t opensound_detection = 0;
			uint32_t openi2o_detection = 0;
			uint32_t smoke_detection = 0;
			uint32_t shadow_detection = 0;
			uint32_t other_detection = 0;
			uint32_t motion_detection;//移动侦测级别0.关闭1.低2.中3.高
			if (runMdCfg.enable == 1){
				if (runMdCfg.sensitive >= 70){
					motion_detection = 1;
				}
				else if(runMdCfg.sensitive >= 40 && runMdCfg.sensitive < 70){
						motion_detection = 2;
				}else if(runMdCfg.sensitive >= 1 && runMdCfg.sensitive < 40){
						motion_detection = 3;
				}
			}
			else
			{
				motion_detection = 0;
			}

			if (!lib_danavideo_cmd_getalarm_response(danavideoconn, trans_id,
				error_code, code_msg, motion_detection,
				opensound_detection, openi2o_detection,
				smoke_detection, shadow_detection,
				other_detection))
			{
				PRINT_ERR("[danavideoconn_command_handler]get alarm fail\n");
				return;
			}
			PRINT_INFO("[danavideoconn_command_handler]get alarm succeeded\n");
		}
		break;

	case DANAVIDEOCMD_GETFLIP://获取翻转状态
		{
			DANAVIDEOCMD_GETFLIP_ARG *getflip_arg = (DANAVIDEOCMD_GETFLIP_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]get flip arg: ch_no[%d]\n",
				getflip_arg->ch_no);

			uint32_t flip_type;
			if (0 == runImageCfg.flipEnabled && 0 == runImageCfg.mirrorEnabled){
				flip_type = 0;
			}else if(0 == runImageCfg.flipEnabled && 1 == runImageCfg.mirrorEnabled){
				flip_type = 1;
			}else if(1 == runImageCfg.flipEnabled && 0 == runImageCfg.mirrorEnabled){
				flip_type = 2;
			}else{
				flip_type = 3;
			}
			if (!lib_danavideo_cmd_getflip_response(danavideoconn, trans_id,
				error_code, code_msg, flip_type))
			{
				PRINT_ERR("[danavideoconn_command_handler]get flip arg fail\n");
				return;
			}
			PRINT_INFO("[danavideoconn_command_handler]get flip arg succeeded\n");
		}
		break;

	case DANAVIDEOCMD_GETPOWERFREQ:
		{
			DANAVIDEOCMD_GETPOWERFREQ_ARG *getpowerfreq_arg =
				(DANAVIDEOCMD_GETPOWERFREQ_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]get power freq: ch_no[%d]\n",
				getpowerfreq_arg->ch_no);
			GK_NET_IMAGE_CFG imagAttr;
			netcam_image_get(&imagAttr);
			uint32_t freq_power ;
			if (50 == imagAttr.antiFlickerFreq){
				freq_power = 0;
			}else if (60 == imagAttr.antiFlickerFreq){
				freq_power = 1;
			}
			PRINT_INFO("[danavideoconn_command_handler]get power freq_power:%d\n",freq_power);
			//uint32_t freq = imagAttr.antiFlickerFreq;//.DANAVIDEO_POWERFREQ_50HZ;
			if (!lib_danavideo_cmd_getpowerfreq_response(danavideoconn, trans_id,
				error_code, code_msg, freq_power))
			{
				PRINT_ERR("[danavideoconn_command_handler]get power freq fail\n");
			}
			PRINT_INFO("[danavideoconn_command_handler]get power freq succeeded\n");
		}
		break;

	case DANAVIDEOCMD_DEVDEF://设备
		{
			DANAVIDEOCMD_DEVDEF_ARG *devdef_arg = (DANAVIDEOCMD_DEVDEF_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]device define: ch_no[%d]\n",
				devdef_arg->ch_no);

			netcam_image_reset_default();
			//CREATE_WORK(SetImageDefault, EVENT_TIMER_WORK, (WORK_CALLBACK)ImageCfgSave);
			//INIT_WORK(SetImageDefault, 20, NULL);
			//SCHEDULE_DEFAULT_WORK(SetImageDefault);
			CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0, 0, 0, 1, 0), (WORK_CALLBACK)ImageCfgSave);

			ST_GK_ENC_STREAM_H264_ATTR h264Attr;
			netcam_video_get(0, GK_NET_STREAM_TYPE_STREAM0, &h264Attr);
			h264Attr.bps = runVideoCfg.vencStream[GK_NET_STREAM_TYPE_STREAM0].h264Conf.bps;//maxbps
			h264Attr.fps = 25;
			netcam_video_set(0, GK_NET_STREAM_TYPE_STREAM0, &h264Attr);

			//CREATE_WORK(SetVideoDefault, EVENT_TIMER_WORK, (WORK_CALLBACK)VideoCfgSave);
			//INIT_WORK(SetVideoDefault, 20, NULL);
			//SCHEDULE_DEFAULT_WORK(SetVideoDefault);
			CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0, 0, 0, 1, 0), (WORK_CALLBACK)VideoCfgSave);

			if (!lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id,
				error_code, code_msg))
			{
				PRINT_ERR("[danavideoconn_command_handler]device define fail\n");
			}

			PRINT_INFO("[danavideoconn_command_handler]device define succeeded\n");
			//CREATE_WORK(reboot, EVENT_TIMER_WORK, maintain_handle);
			//INIT_WORK(reboot, 10, 0);
			//SCHEDULE_DEFAULT_WORK(reboot);
			CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0, 0, 0, 1, 0), (WORK_CALLBACK)maintain_handle);
		}
		break;

	case DANAVIDEOCMD_DEVREBOOT:
		{
			DANAVIDEOCMD_DEVREBOOT_ARG *devreboot_arg = (DANAVIDEOCMD_DEVREBOOT_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]device reboot: ch_no[%d]\n",
				devreboot_arg->ch_no);

			//CREATE_WORK(reboot, EVENT_TIMER_WORK, maintain_handle);
			//INIT_WORK(reboot, 10, 0);
			//SCHEDULE_DEFAULT_WORK(reboot);
			CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0, 0, 0, 1, 0), (WORK_CALLBACK)maintain_handle);

			if (!lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"", trans_id,
				error_code, code_msg))
			{
				PRINT_ERR("[danavideoconn_command_handler]device reboot fail\n");
				return;
			}

			PRINT_INFO("[danavideoconn_command_handler]device reboot succeeded\n");
		}
		break;

	case DANAVIDEOCMD_GETSCREEN://截屏
		{
			DANAVIDEOCMD_GETSCREEN_ARG *getscreen_arg = (DANAVIDEOCMD_GETSCREEN_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]get creen: ch_no[%d]\n",
				getscreen_arg->ch_no);

			if (!lib_danavideo_cmd_exec_response(danavideoconn, cmd, "", trans_id,
				error_code, code_msg))
			{
				PRINT_ERR("[danavideoconn_command_handler]get creen fail\n");
				return;
			}

			PRINT_INFO("[danavideoconn_command_handler]get creen succeeded\n");
		}
		break;

	case DANAVIDEOCMD_GETCOLOR:
		{
			DANAVIDEOCMD_GETCOLOR_ARG *getcolor_arg = (DANAVIDEOCMD_GETCOLOR_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]getcolor_arg ch_no: %d\n",
				getcolor_arg->ch_no);

			uint32_t brightness = runImageCfg.brightness;
			uint32_t contrast = runImageCfg.contrast;
			uint32_t saturation = runImageCfg.saturation;
			uint32_t hue = runImageCfg.hue;
			if (lib_danavideo_cmd_getcolor_response(danavideoconn, trans_id,
				error_code, code_msg, brightness, contrast, saturation, hue))
			{
				PRINT_ERR("[danavideoconn_command_handler]getcolor_arg succeeded\n");
			}
			else
			{
				PRINT_INFO("[danavideoconn_command_handler]getcolor_arg failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_GETFUNLIST:
		{
			DANAVIDEOCMD_GETFUNLIST_ARG *getfunlist_arg = (DANAVIDEOCMD_GETFUNLIST_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]getfunlist_arg ch_no: %d\n",
				getfunlist_arg->ch_no);

			uint32_t methodes_count = 49;

			char *methods[] = {
				(char *)"DANAVIDEOCMD_DEVDEF",
				(char *)"DANAVIDEOCMD_DEVREBOOT",
				(char *)"DANAVIDEOCMD_GETSCREEN",
				(char *)"DANAVIDEOCMD_GETALARM",
				(char *)"DANAVIDEOCMD_GETBASEINFO",
				(char *)"DANAVIDEOCMD_GETCOLOR",
				(char *)"DANAVIDEOCMD_GETFLIP",
				(char *)"DANAVIDEOCMD_GETFUNLIST",
				(char *)"DANAVIDEOCMD_GETNETINFO",
				(char *)"DANAVIDEOCMD_GETPOWERFREQ",
				(char *)"DANAVIDEOCMD_GETTIME",
				(char *)"DANAVIDEOCMD_GETWIFIAP",
				(char *)"DANAVIDEOCMD_GETWIFI",
				(char *)"DANAVIDEOCMD_PTZCTRL",
				(char *)"DANAVIDEOCMD_SDCFORMAT",
				(char *)"DANAVIDEOCMD_SETALARM",
				(char *)"DANAVIDEOCMD_SETCHAN",
				(char *)"DANAVIDEOCMD_SETCOLOR",
				(char *)"DANAVIDEOCMD_SETFLIP",
				(char *)"DANAVIDEOCMD_SETNETINFO",
				(char *)"DANAVIDEOCMD_SETPOWERFREQ",
				(char *)"DANAVIDEOCMD_SETTIME",
				(char *)"DANAVIDEOCMD_SETVIDEO",
				(char *)"DANAVIDEOCMD_SETWIFIAP",
				(char *)"DANAVIDEOCMD_SETWIFI",
				(char *)"DANAVIDEOCMD_STARTAUDIO",
				(char *)"DANAVIDEOCMD_STARTTALKBACK",
				(char *)"DANAVIDEOCMD_STARTVIDEO",
				(char *)"DANAVIDEOCMD_STOPAUDIO",
				(char *)"DANAVIDEOCMD_STOPTALKBACK",
				(char *)"DANAVIDEOCMD_STOPVIDEO",
				(char *)"DANAVIDEOCMD_RECLIST",
				(char *)"DANAVIDEOCMD_RECPLAY",
				(char *)"DANAVIDEOCMD_RECSTOP",
				(char *)"DANAVIDEOCMD_RECACTION",
				(char *)"DANAVIDEOCMD_RECSETRATE",
				(char *)"DANAVIDEOCMD_RECPLANGET",
				(char *)"DANAVIDEOCMD_RECPLANSET",
				(char *)"DANAVIDEOCMD_EXTENDMETHOD",
				(char *)"DANAVIDEOCMD_SETOSD",
				(char *)"DANAVIDEOCMD_GETOSD",
				(char *)"DANAVIDEOCMD_SETCHANNAME",
				(char *)"DANAVIDEOCMD_GETCHANNAME",
				(char *)"DANAVIDEOCMD_CALLPSP",
				(char *)"DANAVIDEOCMD_GETPSP",
				(char *)"DANAVIDEOCMD_SETPSP",
				(char *)"DANAVIDEOCMD_SETPSPDEF",
				(char *)"DANAVIDEOCMD_GETLAYOUT",
				(char *)"DANAVIDEOCMD_SETCHANADV" };
			if (lib_danavideo_cmd_getfunlist_response(danavideoconn, trans_id,
				error_code, code_msg, methodes_count, (const char**)methods))
			{
				PRINT_ERR("[danavideoconn_command_handler]getfunlist_arg succeeded\n");
			}
			else
			{
				PRINT_INFO("[danavideoconn_command_handler]getfunlist_arg failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_GETNETINFO:
		{
			DANAVIDEOCMD_GETNETINFO_ARG *getnetinfo_arg = (DANAVIDEOCMD_GETNETINFO_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]get net info ch_no: %d\n",
				getnetinfo_arg->ch_no);

			uint32_t dns_type = runNetworkCfg.lan.dhcpDns;
			ST_SDK_NETWORK_ATTR net_attr;
			bzero(&net_attr, sizeof(net_attr));
            strcpy(net_attr.name,"eth0");
			netcam_net_get(&net_attr);

			if (lib_danavideo_cmd_getnetinfo_response(danavideoconn, trans_id,
				error_code, code_msg, net_attr.dhcp, net_attr.ip, net_attr.mask, net_attr.gateway,
				dns_type, net_attr.dns1, net_attr.dns2, net_attr.httpPort))
			{
				PRINT_ERR("[danavideoconn_command_handler]get net info succeeded\n");
			}
			else
			{
				PRINT_INFO("[danavideoconn_command_handler]get net info failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_GETWIFIAP:
		{
			DANAVIDEOCMD_GETWIFIAP_ARG *getwifiap_arg = (DANAVIDEOCMD_GETWIFIAP_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]getwifiap_arg ch_no: %d\n",
				getwifiap_arg->ch_no);

			char *WifiName = NULL;
			WIFI_SCAN_LIST_t list[20];
			libdanavideo_wifiinfo_t wifi_list[20];
			memset(wifi_list,0,sizeof(wifi_list));
			int i, number = 20;
			uint32_t wifi_device = 1;
			uint32_t wifi_list_count = 20;

			memset(list, 0, sizeof(list));
			WifiName = netcam_net_wifi_get_devname();
			if(WifiName == NULL)
			{
				PRINT_ERR("WifiName is NULL\n");
				wifi_device = 0;
				goto GETWIFIAP;
			}
			if(netcam_net_wifi_isOn() != 1)
			{
				wifi_device = 0;
				goto GETWIFIAP;
			}

			netcam_net_wifi_on();
			if(netcam_net_wifi_get_scan_list(list,&number) != 0)
			{
				wifi_device = 0;
				goto GETWIFIAP;
			}
			wifi_list_count = number < wifi_list_count ? number : wifi_list_count;
			for(i = 0; i < wifi_list_count && list[i].vaild; i++)
			{
				wifi_list[i].enc_type = list[i].security;
				strncpy(wifi_list[i].essid, list[i].essid, sizeof(wifi_list[i].essid));
				wifi_list[i].quality = list[i].quality;
			}

GETWIFIAP:
			if (lib_danavideo_cmd_getwifiap_response(danavideoconn, trans_id,
				error_code, code_msg, wifi_device, wifi_list_count, wifi_list))
			{
				PRINT_ERR("[danavideoconn_command_handler]getwifiap_arg succeeded\n");
			}
			else
			{
				PRINT_INFO("[danavideoconn_command_handler]getwifiap_arg failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_GETWIFI:
		{
			DANAVIDEOCMD_GETWIFI_ARG *getwifi_arg = (DANAVIDEOCMD_GETWIFI_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]getwifi_arg ch_no: %d\n",
				getwifi_arg->ch_no);
			WIFI_LINK_INFO_t linkInfo;
			memset(&linkInfo,0,sizeof(WIFI_LINK_INFO_t));
			netcam_net_wifi_get_connect_info(&linkInfo);
			if (lib_danavideo_cmd_getwifi_response(danavideoconn, trans_id,
				error_code, code_msg, linkInfo.linkEssid, linkInfo.linkPsd, linkInfo.linkScurity))
			{
				PRINT_INFO("[danavideoconn_command_handler]getwifi_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]getwifi_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_PTZCTRL:
		{
			DANAVIDEOCMD_PTZCTRL_ARG *ptzctrl_arg = (DANAVIDEOCMD_PTZCTRL_ARG *)cmd_arg;
			int step = -1; //default value
			int speed = -1; //default value
			PRINT_INFO("[danavideoconn_command_handler]ptzctrl_arg ch_no: %d, code: %d, para1: %d, para2: %d\n",
				ptzctrl_arg->ch_no, ptzctrl_arg->code,
				ptzctrl_arg->para1, ptzctrl_arg->para2);

			switch (ptzctrl_arg->code) {
			case DANAVIDEO_PTZ_CTRL_MOVE_UP:
				PRINT_INFO("DANAVIDEO_PTZ_CTRL_MOVE_UP\n");
				netcam_ptz_up( step, speed);
				break;
			case DANAVIDEO_PTZ_CTRL_MOVE_DOWN:
				PRINT_INFO("DANAVIDEO_PTZ_CTRL_MOVE_DOWN\n");

				netcam_ptz_down( step, speed);
				break;
			case DANAVIDEO_PTZ_CTRL_MOVE_LEFT:
				PRINT_INFO("DANAVIDEO_PTZ_CTRL_MOVE_LEFT\n");
				netcam_ptz_left( step, speed);
				break;
			case DANAVIDEO_PTZ_CTRL_MOVE_RIGHT:
				PRINT_INFO("DANAVIDEO_PTZ_CTRL_MOVE_RIGHT\n");
				netcam_ptz_right( step, speed);
				break;
			case DANAVIDEO_PTZ_CTRL_MOVE_UPRIGHT:
				PRINT_INFO("DANAVIDEO_PTZ_CTRL_MOVE_UPRIGHT\n");
				netcam_ptz_right_up( step, speed);
				break;
			case DANAVIDEO_PTZ_CTRL_MOVE_DOWNRIGHT:
				PRINT_INFO("DANAVIDEO_PTZ_CTRL_MOVE_DOWNRIGHT\n");
				netcam_ptz_right_down( step, speed);
				break;
			case DANAVIDEO_PTZ_CTRL_MOVE_UPLEFT:
				PRINT_INFO("DANAVIDEO_PTZ_CTRL_MOVE_UPLEFT\n");
				netcam_ptz_left_up( step, speed);
				break;
			case DANAVIDEO_PTZ_CTRL_MOVE_DOWNLEFT:
				PRINT_INFO("DANAVIDEO_PTZ_CTRL_MOVE_DOWNLEFT\n");
				netcam_ptz_left_down( step, speed);
				break;
			case DANAVIDEO_PTZ_CTRL_STOP:
				PRINT_INFO("DANAVIDEO_PTZ_CTRL_STOP\n");
				netcam_ptz_stop();
				break;

			default:
				break;
			}

			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"",
				trans_id, error_code, code_msg))
			{
				PRINT_INFO("[danavideoconn_command_handler]ptzctrl_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]ptzctrl_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_SDCFORMAT://格式化SD卡
		{
			DANAVIDEOCMD_SDCFORMAT_ARG *sdcformat_arg = (DANAVIDEOCMD_SDCFORMAT_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]sdcformat_arg ch_no: %d\n", sdcformat_arg->ch_no);
			int ret = 0;
			ret = grd_sd_format();
			if (ret == 0)
			{
				if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"",
				trans_id, error_code, code_msg))
				{
					PRINT_INFO("[danavideoconn_command_handler]sdcformat_arg send response succeeded\n");
				}
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]sdcformat_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_SETALARM:
		{
			DANAVIDEOCMD_SETALARM_ARG *setalarm_arg = (DANAVIDEOCMD_SETALARM_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]setalarm_arg\n");
			PRINT_INFO("ch_no: %d,\n", setalarm_arg->ch_no);
			PRINT_INFO("motion_detection: %d,\n", setalarm_arg->motion_detection);//移动侦测0:close 1:low 2:mid 3:high
			PRINT_INFO("opensound_detection: %d,\n", setalarm_arg->opensound_detection);
			PRINT_INFO("openi2o_detection: %d,\n", setalarm_arg->openi2o_detection);
			PRINT_INFO("smoke_detection: %d,\n", setalarm_arg->smoke_detection);
			PRINT_INFO("shadow_detection: %d,\n", setalarm_arg->shadow_detection);
			PRINT_INFO("other_detection: %d\n\n", setalarm_arg->other_detection);
			if (0 == setalarm_arg->motion_detection){
				runMdCfg.enable = 0;
			}else if(1 == setalarm_arg->motion_detection){
				runMdCfg.enable = 1;
				runMdCfg.sensitive = 70;
			}else if(2 == setalarm_arg->motion_detection){
				runMdCfg.enable = 1;
				runMdCfg.sensitive = 40;
			}else if(3 == setalarm_arg->motion_detection){
				runMdCfg.enable = 1;
				runMdCfg.sensitive = 10;
			}
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"",
				trans_id, error_code, code_msg))
			{
				PRINT_INFO("[danavideoconn_command_handler]setalarm_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]setalarm_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_SETCHAN:
		{
			DANAVIDEOCMD_SETCHAN_ARG *setchan_arg = (DANAVIDEOCMD_SETCHAN_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]setchan_arg\n");
			PRINT_INFO("ch_no: %d,", setchan_arg->ch_no);
			PRINT_INFO("chans_count: %zd\n", setchan_arg->chans_count);

			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"",
				trans_id, error_code, code_msg))
			{
				PRINT_INFO("[danavideoconn_command_handler]setchan_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]setchan_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_SETCOLOR:
		{
			DANAVIDEOCMD_SETCOLOR_ARG *setcolor_arg = (DANAVIDEOCMD_SETCOLOR_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]setcolor_arg\n");
			PRINT_INFO("ch_no: %d,", setcolor_arg->ch_no);
			PRINT_INFO("video_rate: %d,", setcolor_arg->video_rate);
			PRINT_INFO("brightness: %d,", setcolor_arg->brightness);
			PRINT_INFO("contrast: %d,", setcolor_arg->contrast);
			PRINT_INFO("saturation: %d,", setcolor_arg->saturation);
			PRINT_INFO("hue: %d\n", setcolor_arg->hue);

			GK_NET_IMAGE_CFG stImagingConfig;
    		memset(&stImagingConfig, 0, sizeof(GK_NET_IMAGE_CFG));
    		netcam_image_get(&stImagingConfig);

#define VALUE_CONVERT(value, min, max) \
				((value) > (max) ? (max) : (value) < (min) ? (min) : (value))

			stImagingConfig.brightness = VALUE_CONVERT(setcolor_arg->brightness, 0, 100);
			stImagingConfig.saturation = VALUE_CONVERT(setcolor_arg->saturation, 0, 100);
			stImagingConfig.contrast = VALUE_CONVERT(setcolor_arg->contrast, 0, 100);
			stImagingConfig.hue = VALUE_CONVERT(setcolor_arg->hue, 0, 100);

#undef VALUE_CONVERT

			netcam_image_set(stImagingConfig);
			//CREATE_WORK(SetImage, EVENT_TIMER_WORK, (WORK_CALLBACK)ImageCfgSave);
			//INIT_WORK(SetImage, 20, NULL);
			//SCHEDULE_DEFAULT_WORK(SetImage);
			CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0, 0, 0, 1, 0), (WORK_CALLBACK)ImageCfgSave);

			ST_GK_ENC_STREAM_H264_ATTR h264Attr;
			netcam_video_get(0, setcolor_arg->ch_no, &h264Attr);
			h264Attr.bps = setcolor_arg->video_rate;
			netcam_video_set(0, setcolor_arg->ch_no, &h264Attr);

			//CREATE_WORK(SetVideo, EVENT_TIMER_WORK, (WORK_CALLBACK)VideoCfgSave);
			//INIT_WORK(SetVideo, 20, NULL);
			//SCHEDULE_DEFAULT_WORK(SetVideo);
			CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0, 0, 0, 1, 0), (WORK_CALLBACK)VideoCfgSave);

			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"",
				trans_id, error_code, code_msg))
			{
				PRINT_INFO("[danavideoconn_command_handler]setcolor_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]setcolor_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_SETFLIP:
		{
			DANAVIDEOCMD_SETFLIP_ARG *setflip_arg = (DANAVIDEOCMD_SETFLIP_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]setflip_arg\n");
			PRINT_INFO("ch_no: %d,", setflip_arg->ch_no);
			PRINT_INFO("flip_type: %d\n", setflip_arg->flip_type);

			GK_NET_IMAGE_CFG stImagingConfig;
    		memset(&stImagingConfig, 0, sizeof(GK_NET_IMAGE_CFG));
    		netcam_image_get(&stImagingConfig);

#define TRUE_FALSE_CONVERT(value, vfalse, vtrue) \
				((value) ? (vtrue) : (vfalse))
			if (1 == setflip_arg->flip_type){
				stImagingConfig.mirrorEnabled = 1;//TRUE_FALSE_CONVERT(setflip_arg->flip_type, 0, 1);//水平翻转
				stImagingConfig.flipEnabled = 0;
			}else if (2 == setflip_arg->flip_type || 3 == setflip_arg->flip_type){
				stImagingConfig.flipEnabled = 1;//TRUE_FALSE_CONVERT(setflip_arg->flip_type, 0, 1); //垂直翻转
				stImagingConfig.mirrorEnabled = 0;
			}else if(0 == setflip_arg->flip_type){
				stImagingConfig.flipEnabled = 0;
				stImagingConfig.mirrorEnabled = 0;
			}
#undef TRUE_FALSE_CONVERT

			netcam_image_set(stImagingConfig);
			//CREATE_WORK(SetImage, EVENT_TIMER_WORK, (WORK_CALLBACK)ImageCfgSave);
			//INIT_WORK(SetImage, 20, NULL);
			//SCHEDULE_DEFAULT_WORK(SetImage);
			CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0, 0, 0, 1, 0), (WORK_CALLBACK)ImageCfgSave);

			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"",
				trans_id, error_code, code_msg))
			{
				PRINT_INFO("[danavideoconn_command_handler]setflip_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]setflip_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_SETNETINFO:
		{
			DANAVIDEOCMD_SETNETINFO_ARG *setnetinfo_arg = (DANAVIDEOCMD_SETNETINFO_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]setnetinfo_arg\n");
			PRINT_INFO("ch_no: %d,", setnetinfo_arg->ch_no);
			PRINT_INFO("ip_type: %d,", setnetinfo_arg->ip_type);
			PRINT_INFO("ipaddr: %s,", setnetinfo_arg->ipaddr);
			PRINT_INFO("netmask: %s,", setnetinfo_arg->netmask);
			PRINT_INFO("gateway: %s,", setnetinfo_arg->gateway);
			PRINT_INFO("dns_type: %d,", setnetinfo_arg->dns_type);
			PRINT_INFO("dns_name1: %s,", setnetinfo_arg->dns_name1);
			PRINT_INFO("dns_name2: %s,", setnetinfo_arg->dns_name2);
			PRINT_INFO("http_port: %d\n", setnetinfo_arg->http_port);

			ST_SDK_NETWORK_ATTR net_attr;
    		int ret = 0;

    		if(runNetworkCfg.lan.enable)
        		sprintf(net_attr.name,"eth0");
    		else
        		goto setnetinfo_break;

			net_attr.enable = 1;
    		netcam_net_get(&net_attr);
    		if(setnetinfo_arg->ip_type == 0)
    		{
        		strncpy(net_attr.ip,setnetinfo_arg->ipaddr,SDK_IPSTR_LEN);
        		strncpy(net_attr.mask,setnetinfo_arg->netmask,SDK_DNSSTR_LEN);
        		strncpy(net_attr.gateway,setnetinfo_arg->gateway,SDK_DNSSTR_LEN);
        		strncpy(net_attr.dns1,setnetinfo_arg->dns_name1,SDK_DNSSTR_LEN);
        		strncpy(net_attr.dns2,setnetinfo_arg->dns_name2,SDK_DNSSTR_LEN);
				net_attr.dhcp = 0;
    		}
    		else
    		{
				net_attr.dhcp = 1;
    		}

    		ret = netcam_net_set(&net_attr);
    		if(ret == 0)
    		{
        		netcam_net_cfg_save();
    		}

setnetinfo_break:
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"",
				trans_id, error_code, code_msg))
			{
				PRINT_INFO("[danavideoconn_command_handler]setnetinfo_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]setnetinfo_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_SETPOWERFREQ:
		{
			DANAVIDEOCMD_SETPOWERFREQ_ARG *setpowerfreq_arg = (DANAVIDEOCMD_SETPOWERFREQ_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]setpowerfreq_arg ch_no: %d\n", setpowerfreq_arg->ch_no);
			PRINT_INFO("[danavideoconn_command_handler]setpowerfreq_arg freq: %d\n", setpowerfreq_arg->freq);
			GK_NET_IMAGE_CFG imagAttr;
			netcam_image_get(&imagAttr);
			if (0 == setpowerfreq_arg->freq){
				imagAttr.antiFlickerFreq = 50;
			}else if (1 == setpowerfreq_arg->freq){
				imagAttr.antiFlickerFreq = 60;
			}
			netcam_image_set(imagAttr);
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"",
				trans_id, error_code, code_msg))
			{
				PRINT_INFO("[danavideoconn_command_handler]setpowerfreq_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]setpowerfreq_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_SETTIME:
		{
			char ch;
			int zone;

			DANAVIDEOCMD_SETTIME_ARG *settime_arg = (DANAVIDEOCMD_SETTIME_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]settime_arg");
			PRINT_INFO("ch_no: %d,", settime_arg->ch_no);
			PRINT_INFO("now_time: %lld,", settime_arg->now_time);
			PRINT_INFO("time_zone: %s\n", settime_arg->time_zone);//GMT+08:00

			if (strlen(settime_arg->time_zone) < strlen("GMT+08:00"))
				zone = 0;
			else
			{
				ch = settime_arg->time_zone[3];
				zone = 10 * (settime_arg->time_zone[4] - '0');
				zone += settime_arg->time_zone[5] - '0';

				zone *= 60;
				zone += 10 * (settime_arg->time_zone[7] - '0');
				zone += settime_arg->time_zone[8] - '0';

				zone = ch == '+' ? zone : 0 - zone;
			}

			netcam_sys_set_time_zone_by_utc_second(settime_arg->now_time - zone * 60, zone);

			printf("[danavideoconn_command_handler] current zone[%s]\n", tzname[0]);
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"",
				trans_id, error_code, code_msg))
			{
				PRINT_INFO("[danavideoconn_command_handler]settime_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]settime_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_SETVIDEO:
		{
			DANAVIDEOCMD_SETVIDEO_ARG *setvideo_arg = (DANAVIDEOCMD_SETVIDEO_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]setvideo_arg");
			PRINT_INFO("ch_no: %d,", setvideo_arg->ch_no);
			PRINT_INFO("video_quality: %d\n", setvideo_arg->video_quality);

			dana_setvideo(setvideo_arg->video_quality);
			PRINT_INFO("runVideoCfg.vencStream[%d].h264Conf.quality is %d\n",runVideoCfg.vencStream[stream_dana_id].h264Conf.quality);
			uint32_t set_video_fps = runVideoCfg.vencStream[stream_dana_id].h264Conf.fps;

			//VideoCfgSave();
			if (lib_danavideo_cmd_setvideo_response(danavideoconn, trans_id,
				error_code, code_msg, set_video_fps))
			{
				PRINT_INFO("[danavideoconn_command_handler]setvideo_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]setvideo_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_SETWIFIAP:
		{
			DANAVIDEOCMD_SETWIFIAP_ARG *setwifiap_arg = (DANAVIDEOCMD_SETWIFIAP_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]setwifiap_arg");
			PRINT_INFO("ch_no: %d,", setwifiap_arg->ch_no);
			PRINT_INFO("ip_type: %d,", setwifiap_arg->ip_type);
			PRINT_INFO("ipaddr: %s,", setwifiap_arg->ipaddr);
			PRINT_INFO("netmask: %s,", setwifiap_arg->netmask);
			PRINT_INFO("gateway: %s,", setwifiap_arg->gateway);
			PRINT_INFO("dns_name1: %s,", setwifiap_arg->dns_name1);
			PRINT_INFO("dns_name2: %s,", setwifiap_arg->dns_name2);
			PRINT_INFO("essid: %s,", setwifiap_arg->essid);
			PRINT_INFO("auth_key: %s,", setwifiap_arg->auth_key);
			PRINT_INFO("enc_type: %d\n", setwifiap_arg->enc_type);

#if 1
			ST_SDK_NETWORK_ATTR net_attr;
			int ret = 0;

			if(runNetworkCfg.wifi.enable)
				sprintf(net_attr.name,"ra0");
			else
				goto setwifiap_break;

			net_attr.enable = 1;
			netcam_net_get(&net_attr);
			if(setwifiap_arg->ip_type == 0)
			{
				strncpy(net_attr.ip,setwifiap_arg->ipaddr,SDK_IPSTR_LEN);
				strncpy(net_attr.mask,setwifiap_arg->netmask,SDK_DNSSTR_LEN);
				strncpy(net_attr.gateway,setwifiap_arg->gateway,SDK_DNSSTR_LEN);
				strncpy(net_attr.dns1,setwifiap_arg->dns_name1,SDK_DNSSTR_LEN);
				strncpy(net_attr.dns2,setwifiap_arg->dns_name2,SDK_DNSSTR_LEN);
				net_attr.dhcp = 0;
			}
			else
			{
				net_attr.dhcp = 1;
			}

			ret = netcam_net_set(&net_attr);
			if(ret == 0)
			{
				netcam_net_cfg_save();
			}

setwifiap_break:

			runNetworkCfg.wifi.dhcpIp = setwifiap_arg->ip_type;
			strncpy(runNetworkCfg.wifi.ip, setwifiap_arg->ipaddr,
				sizeof(runNetworkCfg.wifi.ip));
			strncpy(runNetworkCfg.wifi.netmask, setwifiap_arg->netmask,
				sizeof(runNetworkCfg.wifi.netmask));
			strncpy(runNetworkCfg.wifi.gateway, setwifiap_arg->gateway,
				sizeof(runNetworkCfg.wifi.gateway));
			strncpy(runNetworkCfg.wifi.dns1, setwifiap_arg->dns_name1,
				sizeof(runNetworkCfg.wifi.dns1));
			strncpy(runNetworkCfg.wifi.dns2, setwifiap_arg->dns_name2,
				sizeof(runNetworkCfg.wifi.dns2));
			NetworkCfgSave();
#endif

			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"",
				trans_id, error_code, code_msg))
			{
				PRINT_INFO("[danavideoconn_command_handler]setwifiap_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]setwifiap_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_SETWIFI:
		{
			DANAVIDEOCMD_SETWIFI_ARG *setwifi_arg = (DANAVIDEOCMD_SETWIFI_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]setwifi_arg");
			//PRINT_INFO("ch_no: %d,", setwifi_arg->ch_no);
			PRINT_INFO("essid: %s,", setwifi_arg->essid);
			PRINT_INFO("auth_key: %s,", setwifi_arg->auth_key);
			PRINT_INFO("enc_type: %d\n", setwifi_arg->enc_type);
			int ret = 0;
#if 1
			WIFI_LINK_INFO_t linkInfo;
			ST_SDK_NETWORK_ATTR net_attr;
			if(netcam_net_wifi_on() != 0)
			{
				PRINT_INFO("enable wifi function failed");
				return -1;
			}
			strcpy(net_attr.name,netcam_net_wifi_get_devname());
			netcam_net_get(&net_attr);

			memset(&linkInfo,0,sizeof(WIFI_LINK_INFO_t));
			netcam_net_wifi_get_connect_info(&linkInfo);
			linkInfo.isConnect = 1;
			linkInfo.linkScurity = setwifi_arg->enc_type;
			strcpy(linkInfo.linkEssid,setwifi_arg->essid);
			if(setwifi_arg->auth_key != 0)
				strcpy(linkInfo.linkPsd, setwifi_arg->auth_key);
			//netcam_net_wifi_set_connect_info(&linkInfo);
			ret = netcam_net_wifi_set_connect_info(&linkInfo);
			if(WIFI_CONNECT_OK == ret && linkInfo.linkStatus == WIFI_CONNECT_OK)
			{
				net_attr.dhcp = 1;
				netcam_net_set(&net_attr);
				PRINT_INFO("wifi connected succedd!\n");
			}
			else
			{
				PRINT_INFO("wifi connected failed ret %d!\n",ret);
			}
			//CREATE_WORK(SetWifi, EVENT_TIMER_WORK, (WORK_CALLBACK)NetworkCfgSave);
			//INIT_WORK(SetWifi, COMPUTE_TIME(0,0,0,2,0), NULL);
			//SCHEDULE_DEFAULT_WORK(SetWifi);


#endif
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"",
				trans_id, error_code, code_msg))
			{
				PRINT_INFO("[danavideoconn_command_handler]setwifi_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]setwifi_arg send response failed\n");
			}
		}
		break;
	case DANAVIDEOCMD_STARTTALKBACK:
		{
			DANAVIDEOCMD_STARTTALKBACK_ARG *starttalkback_arg = (DANAVIDEOCMD_STARTTALKBACK_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]starttalkback_arg ch_no: %d\n", starttalkback_arg->ch_no);//0

			uint32_t audio_codec = G711A;//PCM,G711A;

			//lib_danavideoconn_readaudio:读取音频数据
			if (lib_danavideo_cmd_starttalkback_response(danavideoconn, trans_id,
				error_code, code_msg, audio_codec))
			{
				PRINT_INFO("[danavideoconn_command_handler]starttalkback_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]starttalkback_arg send response failed\n");
			}
			 // 开启读取音频数据的线程
			 if (acs->run_talkback)
			 {
		 		acs->run_audio_media = false;
		 		PRINT_INFO("ch_no[%u] is already started, enjoy\n", starttalkback_arg->ch_no);
			 }
			 else
			 {
				acs->run_talkback = true;
				acs->run_audio_media = false;
				acs->exit_talback = false;
				if (0 != pthread_create(&(acs->thread_talkback), NULL, &th_talkback,acs))
				{
					memset(&(acs->thread_talkback), 0, sizeof(acs->thread_talkback));
					PRINT_ERR("TEST danavideoconn_command_handler pthread_create th_talkback failed\n");
				}
				else
				{
					PRINT_INFO("TEST danavideoconn_command_handler th_talkback is started, enjoy!\n");
				}
			}

		}
		break;

	case DANAVIDEOCMD_STOPTALKBACK:
		{
			DANAVIDEOCMD_STOPTALKBACK_ARG *stoptalkback_arg = (DANAVIDEOCMD_STOPTALKBACK_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]stoptalkback_arg ch_no: %d\n", stoptalkback_arg->ch_no);
			//acs->run_audio_media = false;
			acs->run_talkback = false;
			acs->exit_talback = true;
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"",
				trans_id, error_code, code_msg))
			{
				PRINT_INFO("[danavideoconn_command_handler]stoptalkback_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]stoptalkback_arg send response failed\n");
			}
		}
		break;
	/*SD卡录像相关设置*/
	case DANAVIDEOCMD_RECLIST://获得录像时间线列表
		{
			DANAVIDEOCMD_RECLIST_ARG *reclist_arg = (DANAVIDEOCMD_RECLIST_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]reclist_arg ch_no: %d\n", reclist_arg->ch_no);
			PRINT_INFO("[danavideoconn_command_handler]reclist_arg get_type: %d\n", reclist_arg->get_type);
			PRINT_INFO("[danavideoconn_command_handler]reclist_arg get_num: %d\n", reclist_arg->get_num);
			PRINT_INFO("[danavideoconn_command_handler]reclist_arg last_time: %lld\n",reclist_arg->last_time);

			JBNV_DANA_TIME BeginTime;
			JBNV_DANA_TIME EndTime;
			struct tm *t_start;
			time_t t;
			t = (time_t)reclist_arg->last_time;
			t_start = localtime(&t);
			PRINT_INFO("%4d-%02d-%02d %02d:%02d:%02d\n",t_start->tm_year+1900,t_start->tm_mon+1,t_start->tm_mday,t_start->tm_hour,t_start->tm_min,t_start->tm_sec);
			BeginTime.dwYear = t_start->tm_year+1900;
			BeginTime.dwMonth = t_start->tm_mon+1;
			BeginTime.dwDay = t_start->tm_mday;
			BeginTime.dwHour = t_start->tm_hour;
			BeginTime.dwMinute = t_start->tm_min;
			BeginTime.dwSecond = t_start->tm_sec;
			EndTime.dwYear = t_start->tm_year+1900;
			EndTime.dwMonth = t_start->tm_mon + 1;
			EndTime.dwDay = t_start->tm_mday;
			EndTime.dwHour = t_start->tm_hour + 23;
			EndTime.dwMinute = t_start->tm_min + 59;
			EndTime.dwSecond = t_start->tm_sec + 59;
			int64_t start = time_to_u64t((AVI_DMS_TIME *)&(BeginTime));
		    int64_t stop =  time_to_u64t((AVI_DMS_TIME *)&(EndTime));

		   	uint32_t rec_lists_count = 20;
			libdanavideo_reclist_recordinfo_t rec_lists[100] = {{0,0,0}};
		    FILE_LIST *list = search_file_by_time(0, 0, start, stop);
			if (list == NULL){
				PRINT_ERR("no sd record file!\n");
				goto GETFILE;
			}
		    int file_num = get_len_list(list);
		    printf("file_num=%d\n", file_num);

		    char *send_buf = malloc(sizeof(JBNV_DANA_FIND_FILE_RESP) + sizeof(JBNV_DANA_FILE_DATA_INFO) * file_num);
		    if(send_buf == NULL) {
		       	PRINT_ERR("send_buf is NULL!\n");
		        return -1;
		    }
		    memset(send_buf, 0, sizeof(JBNV_DANA_FIND_FILE_RESP) + sizeof(JBNV_DANA_FILE_DATA_INFO) * file_num);

		    int i = 0;
		    JBNV_DANA_FILE_DATA_INFO *file_d = NULL;
		    FILE_NODE node;

			rec_lists_count = file_num;
			PRINT_INFO("rec_lists_count : %d\n",rec_lists_count);

		    for (i = 0; i < file_num; i ++) {
		        file_d = (JBNV_DANA_FILE_DATA_INFO *)(send_buf + sizeof(JBNV_DANA_FIND_FILE_RESP) + i * sizeof(JBNV_DANA_FILE_DATA_INFO));

		        memset(&node, 0, sizeof(FILE_NODE));
		        int ret = get_file_node(list, &node);
		        if (ret != 1000) {
		            PRINT_ERR("get_file_node failed, ret = %d\n", ret);
		            break;
		        }
		        //print_node(&node);

		        strcpy(file_d->sFileName, node.path);
				u64t_to_time((AVI_DMS_TIME *)&(file_d->BeginTime), node.start);
		        u64t_to_time((AVI_DMS_TIME *)&(file_d->EndTime), node.stop);

				time_t timep,timep2;
				struct tm st,st2;
				st.tm_year = file_d->BeginTime.dwYear-1900;
				st.tm_mon = file_d->BeginTime.dwMonth-1;
				st.tm_mday = file_d->BeginTime.dwDay;
				st.tm_hour = file_d->BeginTime.dwHour;
				st.tm_min = file_d->BeginTime.dwMinute;
				st.tm_sec =file_d->BeginTime.dwSecond ;

				st2.tm_year = file_d->EndTime.dwYear-1900;
				st2.tm_mon = file_d->EndTime.dwMonth-1;
				st2.tm_mday = file_d->EndTime.dwDay;
				st2.tm_hour = file_d->EndTime.dwHour;
				st2.tm_min = file_d->EndTime.dwMinute;
				st2.tm_sec =file_d->EndTime.dwSecond ;
				timep = mktime(&st);
				timep2 = mktime(&st2);

				rec_lists[i].start_time = (int64_t)timep;
				int rec_len = timep2 - timep;
				rec_lists[i].length = rec_len;
				rec_lists[i].record_type = 1;

				PRINT_INFO("rec_lists[%d].start_time :%ld\n",i,rec_lists[i].start_time);
				PRINT_INFO("rec_lists[%d].length :%d\n",i,rec_lists[i].length);
				//file_d->nChannel = 0;
		        //file_d->nFileSize = get_file_size(node.path);
		    }
			search_close(list);
		    free(send_buf);
 GETFILE:
			if (lib_danavideo_cmd_reclist_response(danavideoconn, trans_id,
				error_code, code_msg, rec_lists_count, rec_lists))
			{
				PRINT_INFO("[danavideoconn_command_handler]reclist_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]reclist_arg send response failed\n");
			}


		}
		break;

	case DANAVIDEOCMD_RECPLAY://录像播放
		{
			DANAVIDEOCMD_RECPLAY_ARG *recplay_arg = (DANAVIDEOCMD_RECPLAY_ARG *)cmd_arg;
			//PRINT_INFO("[danavideoconn_command_handler]recplay_arg");
			//PRINT_INFO("ch_no: %d,", recplay_arg->ch_no);
			PRINT_INFO("time_stamp: %lld\n", recplay_arg->time_stamp);//时间戳???录像时长
			if (acs->playback)
			{
				PRINT_ERR("[danavideoconn_command_handler] playback is already started.\n"
					);
				goto PLAYBACK;
			}
			acs->playback = true;
			acs->run_video_media = false;
			if (pthread_create(&acs->thread_playback, NULL, dana_pb_thread, acs))
			{
				acs->thread_playback = 0;
				acs->playback = false;

				PRINT_ERR("[danavideoconn_command_handler] fail to create playback thread.\n");
				return ;
			}
PLAYBACK:
			PRINT_INFO("[danavideoconn_command_handler] playback thread is started!\n");
			u64t start_64 = 0;
			u64t stop_64 = 0;
			char tmp[20]={0};
			struct tm *t_start,*t_end;
			time_t t,tt;
			t = (time_t)recplay_arg->time_stamp;
			t_start = localtime(&t);
			printf("%4d-%02d-%02d %02d:%02d:%02d\n",t_start->tm_year+1900,t_start->tm_mon+1,t_start->tm_mday,t_start->tm_hour,t_start->tm_min,t_start->tm_sec);
			sprintf(tmp, "%04d%02d%02d%02d%02d%02d", t_start->tm_year+1900, t_start->tm_mon+1, t_start->tm_mday,t_start->tm_hour, t_start->tm_min, t_start->tm_sec);
			start_64 = atoll(tmp);

			tt = (time_t)(recplay_arg->time_stamp + 299);
			t_end = localtime(&tt);
			printf("%4d-%02d-%02d %02d:%02d:%02d\n",t_end->tm_year+1900,t_end->tm_mon+1,t_end->tm_mday,t_end->tm_hour,t_end->tm_min,t_end->tm_sec);
			sprintf(tmp, "%04d%02d%02d%02d%02d%02d", t_end->tm_year+1900, t_end->tm_mon+1, t_end->tm_mday,t_end->tm_hour, t_end->tm_min, t_end->tm_sec);
			stop_64 =  atoll(tmp);

			pDANAPBHandle = dana_pb_bytime(start_64,stop_64);
			if (pDANAPBHandle)
			{
				PRINT_INFO("malloc pBuff succeed!\n");
				his_dana_playback = 1;
				his_dana_playbacking = 0;
				while(his_dana_playbacking == 0)
				{
					usleep(40*1000);
				}
			}
			/*else
			{
				PRINT_ERR("play failed!\n");
				dana_pb_stop(pDANAPBHandle);
				pDANAPBHandle = NULL;
				if(pBuff)
				{
					free(pBuff);

					pBuff = NULL;
				}
				goto PLAYBACK;
			}*/

			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "",
				trans_id, error_code, code_msg))
			{
				PRINT_INFO("[danavideoconn_command_handler]recplay_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]recplay_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_RECSTOP://录像播放停止
		{
			DANAVIDEOCMD_RECSTOP_ARG *recstop_arg = (DANAVIDEOCMD_RECSTOP_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]***DANAVIDEOCMD_RECSTOP*** ch_no: %d\n", recstop_arg->ch_no);
			dana_record_history_stop();
			//dana_is_stop = 1;
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "",
				trans_id, error_code, code_msg))
			{
				PRINT_INFO("[danavideoconn_command_handler]recstop_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]recstop_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_RECACTION://暂停播放
		{
			DANAVIDEOCMD_RECACTION_ARG *recaction_arg = (DANAVIDEOCMD_RECACTION_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]recaction_arg ch_no: %d\n", recaction_arg->ch_no);
			//VODACTION_REQ pb_cmd;
    		//memset(&pb_cmd, 0, sizeof(VODACTION_REQ));
    		//pb_cmd.dwAction = 5;
			//pb_cmd.dwData = 0;
			//pb_cmd.dwSize = 12;

			dana_is_step = 0;
			dana_avi_flag_pause = 1;
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "",
				trans_id, error_code, code_msg)) {
				PRINT_INFO("[danavideoconn_command_handler]recaction_arg send response succeeded\n");
			} else {

				PRINT_ERR("[danavideoconn_command_handler]recaction_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_RECSETRATE://设置播放速率
		{
			DANAVIDEOCMD_RECSETRATE_ARG *recsetrate_arg = (DANAVIDEOCMD_RECSETRATE_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]recsetrate_arg ch_no: %d\n", recsetrate_arg->ch_no);

			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "",
				trans_id, error_code, code_msg))
			{
				PRINT_INFO("[danavideoconn_command_handler]recsetrate_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]recsetrate_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_RECPLANGET://获得已经设置的录像计划
		{
			DANAVIDEOCMD_RECPLANGET_ARG *recplanget_arg = (DANAVIDEOCMD_RECPLANGET_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]recplanget_arg ch_no: %d\n", recplanget_arg->ch_no);

			uint32_t rec_plans_count = 1;//最大值为3，p2p只能设置三个录像计划

			libdanavideo_recplanget_recplan_t rec_plans;
			memset(&rec_plans,0,sizeof(libdanavideo_recplanget_recplan_t));//runRecordCfg

			rec_plans.status = runRecordCfg.enable;
			rec_plans.week_count = 7;
			rec_plans.record_no = 0;
			memset(rec_plans.week,0,sizeof(rec_plans.week));
			char st[33] = {0};
			char end[33] = {0};
			int i;
			int j = 0;
			for (i=0;i<7;i++)
			{
				if(week_re[i] !=0){
					rec_plans.week[j] = week_re[i];
					//PRINT_INFO("rec_plans.week[%d] : %d\n",j,rec_plans.week[j]);
					j++;
				}
			}
			for (i=0;i<7;i++){
				if (runRecordCfg.scheduleTime[i][2].stopHour != 0){
				sprintf(st,"%02d:%02d:%02d",runRecordCfg.scheduleTime[i][0].startHour,runRecordCfg.scheduleTime[i][1].startMin,00);
				sprintf(end,"%02d:%02d:%02d",runRecordCfg.scheduleTime[i][2].stopHour,runRecordCfg.scheduleTime[i][3].stopMin,00);
				}
			}
			strncpy(rec_plans.start_time,st,33);
			strncpy(rec_plans.end_time,end,33);
			/*libdanavideo_recplanget_recplan_t rec_plans[3] = {
				{0, 2,
				{DANAVIDEO_REC_WEEK_MON, DANAVIDEO_REC_WEEK_SAT},
				"12:23", "15:56", DANAVIDEO_REC_PLAN_OPEN},

				{1, 3,
				{DANAVIDEO_REC_WEEK_MON, DANAVIDEO_REC_WEEK_SAT, DANAVIDEO_REC_WEEK_SUN},
				"22:23", "23:24", DANAVIDEO_REC_PLAN_CLOSE},

				{2,4,
				{DANAVIDEO_REC_WEEK_MON,DANAVIDEO_REC_WEEK_WED,DANAVIDEO_REC_WEEK_FRI, DANAVIDEO_REC_WEEK_SUN},
				"00:00","11:11",DANAVIDEO_REC_PLAN_OPEN}
				};
			*/
			if (lib_danavideo_cmd_recplanget_response(danavideoconn, trans_id,
				error_code, code_msg, rec_plans_count, &rec_plans))
			{
				PRINT_INFO("[danavideoconn_command_handler]recplanget_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]recplanget_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_RECPLANSET://设置录像计划
		{
			DANAVIDEOCMD_RECPLANSET_ARG *recplanset_arg = (DANAVIDEOCMD_RECPLANSET_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]recplanset_arg ch_no: %d\n", recplanset_arg->ch_no);
			PRINT_INFO("start_time: %s\n", recplanset_arg->start_time);
			PRINT_INFO("end_time: %s\n", recplanset_arg->end_time);
			PRINT_INFO("status: %d\n", recplanset_arg->status);
			PRINT_INFO("week_count :%d\n",recplanset_arg->week_count);
			PRINT_INFO("\n");
			week_Count = recplanset_arg->week_count;
			int j;
			memset(week_re,0,sizeof(week_re));
			for(j=0;j<7;j++){
				week_re[recplanset_arg->week[j]-1] = recplanset_arg->week[j];
				PRINT_INFO("week_re[recplanset_arg->week[%d]-1]  is %d\n",j,week_re[recplanset_arg->week[j]-1] );
			}
			runRecordCfg.enable = recplanset_arg->status;
			runRecordCfg.stream_no = recplanset_arg->record_no;
			runRecordCfg.preRecordTime = 5 * 60;
			runRecordCfg.recordMode = 0;
			char *start[3];
			char *stop[3];
			memset(start,0,sizeof(start));
			memset(stop,0,sizeof(stop));
			split(start,recplanset_arg->start_time,":");
			split(stop,recplanset_arg->end_time,":");
			int i;
			int start_t[3] = {0};
			int end_t[3] = {0};
			memset(start_t,0,sizeof(start_t));
			memset(end_t,0,sizeof(end_t));
			for(i=0;i<3;i++)
			{
				start_t[i] = atoi(start[i]);
				end_t[i] = atoi(stop[i]);
				//PRINT_INFO("start :%d\n",start_t[i]);
				//PRINT_INFO("stop :%d\n",end_t[i]);
			}
			//danavideo_sd_record();
			for (i=0;i<7;i++){
				printf("recplanset_arg->week[%d] is %d\n",i,recplanset_arg->week[i]);

			}
			memset(runRecordCfg.scheduleTime,0,sizeof(GK_SCHEDTIME));
			 for (i=0; i<7; i++) {
			 	if (DANAVIDEO_REC_WEEK_MON == recplanset_arg->week[i]) {
						PRINT_INFO("week: DANAVIDEO_REC_WEEK_MON\n");
						runRecordCfg.scheduleTime[0][0].startHour = start_t[0];
						runRecordCfg.scheduleTime[0][1].startMin = start_t[1];
						runRecordCfg.scheduleTime[0][2].stopHour = end_t[0];
						runRecordCfg.scheduleTime[0][3].stopMin = end_t[1];
					} else if (DANAVIDEO_REC_WEEK_TUE == recplanset_arg->week[i]) {
						PRINT_INFO("week: DANAVIDEO_REC_WEEK_TUE\n");
						runRecordCfg.scheduleTime[1][0].startHour = start_t[0];
						runRecordCfg.scheduleTime[1][1].startMin = start_t[1];
						runRecordCfg.scheduleTime[1][2].stopHour = end_t[0];
						runRecordCfg.scheduleTime[1][3].stopMin = end_t[1];
					} else if (DANAVIDEO_REC_WEEK_WED == recplanset_arg->week[i]) {
						PRINT_INFO("week: DANAVIDEO_REC_WEEK_WED\n");
						runRecordCfg.scheduleTime[2][0].startHour = start_t[0];
						runRecordCfg.scheduleTime[2][1].startMin = start_t[1];
						runRecordCfg.scheduleTime[2][2].stopHour = end_t[0];
						runRecordCfg.scheduleTime[2][3].stopMin = end_t[1];
					} else if (DANAVIDEO_REC_WEEK_THU == recplanset_arg->week[i]) {

						PRINT_INFO("week: DANAVIDEO_REC_WEEK_THU\n");
						runRecordCfg.scheduleTime[3][0].startHour = start_t[0];
						runRecordCfg.scheduleTime[3][1].startMin = start_t[1];
						runRecordCfg.scheduleTime[3][2].stopHour = end_t[0];
						runRecordCfg.scheduleTime[3][3].stopMin = end_t[1];
					} else if (DANAVIDEO_REC_WEEK_FRI == recplanset_arg->week[i]) {

						PRINT_INFO("week: DANAVIDEO_REC_WEEK_FRI\n");
						runRecordCfg.scheduleTime[4][0].startHour = start_t[0];
						runRecordCfg.scheduleTime[4][1].startMin = start_t[1];
						runRecordCfg.scheduleTime[4][2].stopHour = end_t[0];
						runRecordCfg.scheduleTime[4][3].stopMin = end_t[1];
					} else if (DANAVIDEO_REC_WEEK_SAT == recplanset_arg->week[i]) {

						PRINT_INFO("week: DANAVIDEO_REC_WEEK_SAT\n");
						runRecordCfg.scheduleTime[5][0].startHour = start_t[0];
						runRecordCfg.scheduleTime[5][1].startMin = start_t[1];
						runRecordCfg.scheduleTime[5][2].stopHour = end_t[0];
						runRecordCfg.scheduleTime[5][3].stopMin = end_t[1];
					} else if (DANAVIDEO_REC_WEEK_SUN == recplanset_arg->week[i]) {

						PRINT_INFO("week: DANAVIDEO_REC_WEEK_SUN\n");
						runRecordCfg.scheduleTime[6][0].startHour = start_t[0];
						runRecordCfg.scheduleTime[6][1].startMin = start_t[1];
						runRecordCfg.scheduleTime[6][2].stopHour = end_t[0];
						runRecordCfg.scheduleTime[6][3].stopMin = end_t[1];
					} else {
						PRINT_INFO("Unknown week: %s\n", recplanset_arg->week[i]);
					}

			}

			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "",
				trans_id, error_code, code_msg))
			{
				PRINT_INFO("[danavideoconn_command_handler]recplanset_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]recplanset_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_EXTENDMETHOD:
		{
			DANAVIDEOCMD_EXTENDMETHOD_ARG *extendmethod_arg = (DANAVIDEOCMD_EXTENDMETHOD_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]extendmethod_arg ch_no: %d\n", extendmethod_arg->ch_no);

			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "",
				trans_id, error_code, code_msg))
			{
				PRINT_INFO("[danavideoconn_command_handler]extendmethod_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]extendmethod_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_SETOSD:
		{
			DANAVIDEOCMD_SETOSD_ARG *setosd_arg = (DANAVIDEOCMD_SETOSD_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]set osd ch_no: %d\n", setosd_arg->ch_no);

			PRINT_INFO("osd_info:\n");
			if (DANAVIDEO_OSD_SHOW_OPEN == setosd_arg->osd.chan_name_show) {
				runChannelCfg.channelInfo[0].osdChannelID.enable = 1;
				PRINT_INFO("chan_name_show OPEN\n");
				PRINT_INFO("setosd_arg->osd.chan_name_show is %d\n",setosd_arg->osd.chan_name_show);
				if (setosd_arg->osd.has_show_name_x) {
					//runChannelCfg.channelInfo[0].osdChannelName.x = setosd_arg->osd.has_show_name_x;
					//printf("show_name_x: %d\n", setosd_arg->osd.show_name_x);
				}
				if (setosd_arg->osd.has_show_name_y) {
					//runChannelCfg.channelInfo[0].osdChannelName.y = setosd_arg->osd.has_show_name_y;
					//printf("show_name_y: %d\n", setosd_arg->osd.show_name_y);
				}
			} else if (DANAVIDEO_OSD_SHOW_CLOSE == setosd_arg->osd.chan_name_show) {
				runChannelCfg.channelInfo[0].osdChannelName.enable = 0;
				PRINT_INFO("chan_name_show CLOSE\n");
			} else {
				runChannelCfg.channelInfo[0].osdChannelName.enable = 0;
				PRINT_ERR("chan_name_show unknown type[%d]\n", setosd_arg->osd.chan_name_show);
			}

			if (DANAVIDEO_OSD_SHOW_OPEN == setosd_arg->osd.datetime_show) {
				PRINT_INFO("datetime_show OPEN\n");
				runChannelCfg.channelInfo[0].osdDatetime.enable = 1;
				if (setosd_arg->osd.has_show_datetime_x) {
					runChannelCfg.channelInfo[0].osdDatetime.x = (float)setosd_arg->osd.show_datetime_x / 63;
					PRINT_INFO("show_datetime_x: %d\n", setosd_arg->osd.show_datetime_x);
					PRINT_INFO("runChannelCfg.channelInfo[0].osdDatetime.x: %d\n", runChannelCfg.channelInfo[0].osdDatetime.x);
				}
				if (setosd_arg->osd.has_show_datetime_y) {
					runChannelCfg.channelInfo[0].osdDatetime.y = (float)setosd_arg->osd.show_datetime_y / 84;
					PRINT_INFO("show_datetime_y: %d\n", setosd_arg->osd.show_datetime_y);
					PRINT_INFO("runChannelCfg.channelInfo[0].osdDatetime.y: %d\n", runChannelCfg.channelInfo[0].osdDatetime.y);
				}
				if (setosd_arg->osd.has_show_format) {
					PRINT_INFO("show_format:\n");
					switch (setosd_arg->osd.show_format) {
					case DANAVIDEO_OSD_DATE_FORMAT_YYYY_MM_DD:
						runChannelCfg.channelInfo[0].osdDatetime.dateFormat = 0;
						PRINT_INFO("DANAVIDEO_OSD_DATE_FORMAT_YYYY_MM_DD\n");
						break;
					case DANAVIDEO_OSD_DATE_FORMAT_MM_DD_YYYY:
						runChannelCfg.channelInfo[0].osdDatetime.dateFormat = 1;
						PRINT_INFO("DANAVIDEO_OSD_DATE_FORMAT_MM_DD_YYYY\n");
						break;
					case DANAVIDEO_OSD_DATE_FORMAT_YYYY_MM_DD_CH:
						runChannelCfg.channelInfo[0].osdDatetime.dateFormat = 2;
						PRINT_INFO("DANAVIDEO_OSD_DATE_FORMAT_YYYY_MM_DD_CH\n");
						break;
					case DANAVIDEO_OSD_DATE_FORMAT_MM_DD_YYYY_CH:
						runChannelCfg.channelInfo[0].osdDatetime.dateFormat = 3;
						PRINT_INFO("DANAVIDEO_OSD_DATE_FORMAT_MM_DD_YYYY_CH\n");
						break;
					case DANAVIDEO_OSD_DATE_FORMAT_DD_MM_YYYY:
						runChannelCfg.channelInfo[0].osdDatetime.dateFormat = 4;
						PRINT_INFO("DANAVIDEO_OSD_DATE_FORMAT_DD_MM_YYYY\n");
						break;
					case DANAVIDEO_OSD_DATE_FORMAT_DD_MM_YYYY_CH:
						runChannelCfg.channelInfo[0].osdDatetime.dateFormat = 5;
						PRINT_INFO("DANAVIDEO_OSD_DATE_FORMAT_DD_MM_YYYY_CH\n");
						break;
					default:
						PRINT_INFO("DANAVIDEO_OSD_DATE_FORMAT_XXXX\n");
						break;
					}
				}

				if (setosd_arg->osd.has_hour_format) {
					PRINT_INFO("hour_format:\n");
					switch (setosd_arg->osd.hour_format) {
					case DANAVIDEO_OSD_TIME_24_HOUR:
						PRINT_INFO("DANAVIDEO_OSD_TIME_24_HOUR\n");
						runChannelCfg.channelInfo[0].osdDatetime.timeFmt = 0;
						break;
					case DANAVIDEO_OSD_TIME_12_HOUR:
						runChannelCfg.channelInfo[0].osdDatetime.timeFmt = 1;
						PRINT_INFO("DANAVIDEO_OSD_TIME_12_HOUR\n");
						break;
					default:
						PRINT_INFO("DANAVIDEO_OSD_TIME_XXXX\n");
						break;
					}
				}

				if (setosd_arg->osd.has_show_week) {
					PRINT_INFO("show_week:\n");
					switch (setosd_arg->osd.show_week) {
					case DANAVIDEO_OSD_SHOW_CLOSE:
						runChannelCfg.channelInfo[0].osdDatetime.displayWeek = 0;
						PRINT_INFO("DANAVIDEO_OSD_SHOW_CLOSE\n");
						break;
					case DANAVIDEO_OSD_SHOW_OPEN:
						runChannelCfg.channelInfo[0].osdDatetime.displayWeek = 1;
						PRINT_INFO("DANAVIDEO_OSD_SHOW_OPEN\n");
						break;
					default:
						PRINT_INFO("DANAVIDEO_OSD_SHOW_XXXX\n");
						break;
					}
				}

				if (setosd_arg->osd.has_datetime_attr) {
					printf("datetime_attr:\n");
					switch (setosd_arg->osd.datetime_attr) {
					case DANAVIDEO_OSD_DATETIME_TRANSPARENT:
						PRINT_INFO("DANAVIDEO_OSD_DATETIME_TRANSPARENT\n");
						break;
					case DANAVIDEO_OSD_DATETIME_DISPLAY:
						PRINT_INFO("DANAVIDEO_OSD_DATETIME_DISPLAY\n");
						break;
					default:
						PRINT_INFO("DANAVIDEO_OSD_DATETIME_XXXX\n");
						break;
					}
				}

			} else if (DANAVIDEO_OSD_SHOW_CLOSE == setosd_arg->osd.datetime_show) {
				runChannelCfg.channelInfo[0].osdDatetime.enable = 0;
				PRINT_INFO("datetime_show CLOSE\n");
			} else {
				runChannelCfg.channelInfo[0].osdDatetime.enable = 0;
				PRINT_INFO("datetime_show unknown type[%d]\n", setosd_arg->osd.datetime_show);
			}

			if (DANAVIDEO_OSD_SHOW_OPEN == setosd_arg->osd.custom1_show) {
				PRINT_INFO("custom1_show OPEN\n");
				if (setosd_arg->osd.has_show_custom1_str) {
					PRINT_INFO("show_custom1_str: %s\n", setosd_arg->osd.show_custom1_str);
				}
				if (setosd_arg->osd.has_show_custom1_x) {
					PRINT_INFO("show_custom1_x: %d\n", setosd_arg->osd.show_custom1_x);
				}
				if (setosd_arg->osd.has_show_custom1_y) {
					PRINT_INFO("show_custom1_y: %d\n", setosd_arg->osd.show_custom1_y);
				}
			} else if (DANAVIDEO_OSD_SHOW_CLOSE == setosd_arg->osd.custom1_show) {
				PRINT_INFO("custom1_show CLOSE\n");
			} else {
				PRINT_INFO("custom1_show unknown type[%d]\n", setosd_arg->osd.custom1_show);
			}

			if (DANAVIDEO_OSD_SHOW_OPEN == setosd_arg->osd.custom2_show) {
				PRINT_INFO("custom2_show OPEN\n");
				if (setosd_arg->osd.has_show_custom2_str) {
					PRINT_INFO("show_custom2_str: %s\n", setosd_arg->osd.show_custom2_str);
				}
				if (setosd_arg->osd.has_show_custom2_x) {
					PRINT_INFO("show_custom2_x: %d\n", setosd_arg->osd.show_custom2_x);
				}
				if (setosd_arg->osd.has_show_custom2_y) {
					PRINT_INFO("show_custom2_y: %d\n", setosd_arg->osd.show_custom2_y);
				}
			} else if (DANAVIDEO_OSD_SHOW_CLOSE == setosd_arg->osd.custom2_show) {
				PRINT_INFO("custom2_show CLOSE\n");
			} else {
				PRINT_INFO("custom2_show unknown type[%d]\n", setosd_arg->osd.custom2_show);
			}

			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, "",
				trans_id, error_code, code_msg))
			{
				PRINT_INFO("[danavideoconn_command_handler]set osd send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]set osd send response failed\n");
			}
		}

        netcam_osd_update_title();
        netcam_osd_update_id();
		netcam_timer_add_task(netcam_osd_pm_save, 8, SDK_FALSE, SDK_TRUE);// 8*250ms = 2 s
		memcpy(&runChannelCfg.channelInfo[1],&runChannelCfg.channelInfo[0],sizeof(GK_NET_CHANNEL_INFO));
		memcpy(&runChannelCfg.channelInfo[2],&runChannelCfg.channelInfo[0],sizeof(GK_NET_CHANNEL_INFO));
		memcpy(&runChannelCfg.channelInfo[3],&runChannelCfg.channelInfo[0],sizeof(GK_NET_CHANNEL_INFO));
		ChannelCfgSave();
		break;

	case DANAVIDEOCMD_GETOSD:
		{
			DANAVIDEOCMD_GETOSD_ARG *getosd_arg = (DANAVIDEOCMD_GETOSD_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]get osd ch_no: %d\n", getosd_arg->ch_no);

			libdanavideo_osdinfo_t osdinfo;
			osdinfo.chan_name_show = runChannelCfg.channelInfo[0].osdChannelName.enable;
			osdinfo.datetime_show =  runChannelCfg.channelInfo[0].osdDatetime.enable;
			osdinfo.show_datetime_x = (uint32_t)runChannelCfg.channelInfo[0].osdDatetime.x * 63;
			osdinfo.show_datetime_y = (uint32_t)runChannelCfg.channelInfo[0].osdDatetime.y * 84;
			osdinfo.show_format = runChannelCfg.channelInfo[0].osdDatetime.dateFormat;
			osdinfo.hour_format = runChannelCfg.channelInfo[0].osdDatetime.timeFmt;
			osdinfo.show_week = runChannelCfg.channelInfo[0].osdDatetime.displayWeek;
			osdinfo.datetime_attr = DANAVIDEO_OSD_DATETIME_DISPLAY;

			osdinfo.custom1_show = DANAVIDEO_OSD_SHOW_OPEN;
			strncpy(osdinfo.show_custom1_str, "show_custom1_str",
				sizeof(osdinfo.show_custom1_str) -1);
			osdinfo.show_custom1_x = 5;
			osdinfo.show_custom1_y = 6;

			osdinfo.custom2_show = DANAVIDEO_OSD_SHOW_CLOSE;
			strncpy(osdinfo.show_custom2_str, "show_custom2_str",
				sizeof(osdinfo.show_custom2_str) -1);
			osdinfo.show_custom2_x = 7;
			osdinfo.show_custom2_y = 8;

			if (lib_danavideo_cmd_getosd_response(danavideoconn, trans_id,
				error_code, code_msg, &osdinfo))
			{
				PRINT_INFO("[danavideoconn_command_handler]get osd send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]get osd send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_SETCHANNAME:
		{
			DANAVIDEOCMD_SETCHANNAME_ARG *setchanname_arg = (DANAVIDEOCMD_SETCHANNAME_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]setchanname_arg ");
			PRINT_INFO("ch_no: %d,", setchanname_arg->ch_no);
			PRINT_INFO("chan_name: %s\n", setchanname_arg->chan_name);

			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"",
				trans_id, error_code, code_msg))
			{
				PRINT_INFO("[danavideoconn_command_handler]setchanname_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]setchanname_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_GETCHANNAME:
		{
			DANAVIDEOCMD_GETCHANNAME_ARG *getchanname_arg = (DANAVIDEOCMD_GETCHANNAME_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]getchanname_arg ch_no: %d\n", getchanname_arg->ch_no);

			char *chan_name = (char*)runVideoCfg.vencStream[getchanname_arg->ch_no].h264Conf.name;
			if (lib_danavideo_cmd_getchanname_response(danavideoconn, trans_id,
				error_code, code_msg, chan_name))
			{
				PRINT_INFO("[danavideoconn_command_handler]getchanname_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]getchanname_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_CALLPSP:
		{
			DANAVIDEOCMD_CALLPSP_ARG *callpsp_arg = (DANAVIDEOCMD_CALLPSP_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]callpsp_arg ");
			PRINT_INFO("ch_no: %d,", callpsp_arg->ch_no);
			PRINT_INFO("psp_id: %d\n", callpsp_arg->psp_id);

			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"",
				trans_id, error_code, code_msg))
			{
				PRINT_INFO("[danavideoconn_command_handler]callpsp_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]callpsp_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_GETPSP:
		{
			DANAVIDEOCMD_GETPSP_ARG *getpsp_arg = (DANAVIDEOCMD_GETPSP_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]getpsp_arg ");
			PRINT_INFO("ch_no: %d,", getpsp_arg->ch_no);
			PRINT_INFO("page: %d,", getpsp_arg->page);
			PRINT_INFO("page_size: %d\n", getpsp_arg->page_size);

			uint32_t total = 20;
			uint32_t psp_count = 2;
			libdanavideo_pspinfo_t psp[] = {
				{1, "Psp_1", true, true},
				{2, "Psp_2", false, true}
			};

			if (lib_danavideo_cmd_getpsp_response(danavideoconn, trans_id,
				error_code, code_msg, total, psp_count, psp))
			{
				PRINT_INFO("[danavideoconn_command_handler]getpsp_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]getpsp_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_SETPSP:
		{
			DANAVIDEOCMD_SETPSP_ARG *setpsp_arg = (DANAVIDEOCMD_SETPSP_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]setpsp_arg ");
			PRINT_INFO("ch_no: %d,", setpsp_arg->ch_no);
			PRINT_INFO("psp_id: %d,", setpsp_arg->psp.psp_id);
			PRINT_INFO("psp_name: %s,", setpsp_arg->psp.psp_name);
			PRINT_INFO("psp_default: %s,", setpsp_arg->psp.psp_default?"true":"false");
			PRINT_INFO("is_set: %s\n", setpsp_arg->psp.is_set?"true":"false");

			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"",
				trans_id, error_code, code_msg))
			{
				PRINT_INFO("[danavideoconn_command_handler]setpsp_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]setpsp_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_SETPSPDEF:
		{
			DANAVIDEOCMD_SETPSPDEF_ARG *setpspdef_arg = (DANAVIDEOCMD_SETPSPDEF_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]setpspdef_arg ");
			PRINT_INFO("ch_no: %d,", setpspdef_arg->ch_no);
			PRINT_INFO("psp_id: %d\n", setpspdef_arg->psp_id);

			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"",
				trans_id, error_code, code_msg))
			{
				PRINT_INFO("[danavideoconn_command_handler]setpspdef_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]setpspdef_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_GETLAYOUT:
		{
			DANAVIDEOCMD_GETLAYOUT_ARG *getlayout_arg = (DANAVIDEOCMD_GETLAYOUT_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]getlayout_arg ch_no: %d\n", getlayout_arg->ch_no);

			uint32_t matrix_x = 4;
			uint32_t matrix_y = 4;
			size_t chans_count = 16;
			uint32_t chans[] = {1, 1, 2, 3, 1, 1, 4, 5, 6, 7, 8, 9, 10, 11, 0, 0};
			uint32_t layout_change = 0;
			uint32_t chan_pos_change = 0;
			size_t use_chs_count = 16;
			uint32_t use_chs[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16};

			if (lib_danavideo_cmd_getlayout_response(danavideoconn, trans_id,
				error_code, code_msg, matrix_x, matrix_y, chans_count,
				chans, layout_change, chan_pos_change, use_chs_count, use_chs))
			{
				PRINT_INFO("[danavideoconn_command_handler]getlayout_arg send response succeeded\n");
			}
			else
			{
				PRINT_INFO("[danavideoconn_command_handler]getlayout_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_SETCHANADV:
		{
			DANAVIDEOCMD_SETCHANADV_ARG *setchanadv_arg = (DANAVIDEOCMD_SETCHANADV_ARG *)cmd_arg;
			PRINT_INFO("[danavideoconn_command_handler]setchanadv_arg ");
			PRINT_INFO("ch_no: %d,", setchanadv_arg->ch_no);
			PRINT_INFO("matrix_x: %d,", setchanadv_arg->matrix_x);
			PRINT_INFO("matrix_y: %d\n", setchanadv_arg->matrix_y);

			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"",
				trans_id, error_code, code_msg))
			{
				PRINT_INFO("[danavideoconn_command_handler]setchanadv_arg send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler]setchanadv_arg send response failed\n");
			}
		}
		break;

	case DANAVIDEOCMD_RESOLVECMDFAILED:
		{
			PRINT_INFO("[danavideoconn_command_handler] DANAVIDEOCMD_RESOLVECMDFAILED\n");
			error_code = 20145;
			code_msg = (char *)"danavideocmd_resolvecmdfailed";
			if (lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)cmd_arg,
				trans_id, error_code, code_msg))
			{
				PRINT_INFO("[danavideoconn_command_handler] DANAVIDEOCMD_RESOLVECMDFAILED send response succeeded\n");
			}
			else
			{
				PRINT_ERR("[danavideoconn_command_handler] DANAVIDEOCMD_RESOLVECMDFAILED send response failed\n");
			}
		}
		break;

	default:
		lib_danavideo_cmd_exec_response(danavideoconn, cmd, (char *)"",
			trans_id, error_code, code_msg);
		PRINT_INFO("[danavideoconn_command_handler] cmd id[%d] is not handle.\n", cmd);
		break;
	}

	return;
}

dana_video_callback_funs_t danavideocallbackfuns = {
	.danavideoconn_created = danavideoconn_created,
	.danavideoconn_aborted = danavideoconn_aborted,
	.danavideoconn_command_handler = danavideoconn_command_handler,
};


static danacloud_realtime_upload_core_t realtimeuploadcore = {
	.run_realtime_upload = false,
	.thread_realtime_upload = 0,
	.handle = 0
};

static void* th_realtime_upload(void *arg)
{
	MEDIA_FRAME* mfr;
	danacloud_realtime_upload_core_t *realtimeuploadcore
		= (danacloud_realtime_upload_core_t *)arg;

	mfr = (MEDIA_FRAME*)malloc(sizeof(MEDIA_FRAME));
	if (!mfr)
	{
		PRINT_INFO("[th_realtime_upload] Fail to alloc space for media frame.\n");
		return 0;
	}

	while (realtimeuploadcore->run_realtime_upload)
	{
		mfr->size = 0;
		//memset(mfr->buffer, 0, MEDIA_FRAME_BUFFER_SIZE);
		if (get_media(realtimeuploadcore->handle, mfr))
			continue;

		if (!lib_danavideo_cloud_realtime_upload(DANACLOUD_CHAN_NO, mfr->type,
			mfr->encode_type, mfr->iskey, mfr->timestamp, DANAVIDEO_CLOUD_ALARM_NO,
			(const char*)mfr->buffer, mfr->size, DANA_SENDING_TIMEOUT))
		{
			//printf("[th_realtime_upload] media frame upload failed\n");
			//usleep(10*1000);
		}
	}


	free(mfr);
	PRINT_INFO("[th_realtime_upload] media upload thread exit\n");
	return 0;
}

static bool danavideo_cloud_enter_realtime_upload()
{
	if (realtimeuploadcore.run_realtime_upload)
	{
		PRINT_INFO("[danavideo_cloud_enter_realtime_upload] readtime mode already started\n");
		return true;
	}

	if (!lib_danavideo_cloud_realtime_on())
	{
		PRINT_INFO("[danavideo_cloud_enter_realtime_upload] readtime mode start failed\n");
		return false;
	}

	realtimeuploadcore.run_realtime_upload = true;
	realtimeuploadcore.handle = mediabuf_add_reader(GK_NET_STREAM_TYPE_STREAM1);
	mediabuf_set_newest_frame(realtimeuploadcore.handle);

	if (pthread_create(&realtimeuploadcore.thread_realtime_upload, NULL,
		&th_realtime_upload, &realtimeuploadcore))
	{
		realtimeuploadcore.thread_realtime_upload = 0;
		realtimeuploadcore.run_realtime_upload = false;
		PRINT_INFO("[danavideo_cloud_enter_realtime_upload] fail to create realtime upload thread\n");
		return false;
	}

	PRINT_INFO("[danavideo_cloud_enter_realtime_upload] create realtime upload thread succeeded\n");
	return true;
}

bool danavideo_cloud_leave_realtime_upload()
{
	if (!realtimeuploadcore.run_realtime_upload)
	{
		PRINT_INFO("[danavideo_cloud_leave_realtime_upload] realtime upload is stopped\n");
		return true;
	}

	realtimeuploadcore.run_realtime_upload = false;

	if (realtimeuploadcore.thread_realtime_upload == 0)
		return true;

	pthread_join(realtimeuploadcore.thread_realtime_upload, NULL);
	realtimeuploadcore.thread_realtime_upload = 0;
	lib_danavideo_cloud_realtime_off();

	PRINT_INFO("[danavideo_cloud_leave_realtime_upload] readltime upload stop\n");
	return true;
}

void danacloud_mode_changed(const danavideo_cloud_mode_t cloud_mode)
{
	switch (cloud_mode)
	{
	case DANAVIDEO_CLOUD_MODE_UNKNOWN:
		{
			PRINT_INFO("[danacloud_mode_changed] mode: DANAVIDEO_CLOUD_MODE_UNKNOWN\n");
			danavideo_cloud_leave_realtime_upload();
			//danavideo_cloud_leave_alarm_upload();
		}
		break;

	case DANAVIDEO_CLOUD_MODE_REALTIME:
		{
			PRINT_INFO("[danacloud_mode_changed] mode: DANAVIDEO_CLOUD_MODE_REALTIME\n");
			//danavideo_cloud_leave_alarm_upload();
			danavideo_cloud_enter_realtime_upload();
		}
		break;

	case DANAVIDEO_CLOUD_MODE_ALARM:
		{
			PRINT_INFO("[danacloud_mode_changed] mode: DANAVIDEO_CLOUD_MODE_ALARM\n");
			danavideo_cloud_leave_realtime_upload();
			//danavideo_cloud_enter_alarm_upload();
		}
		break;

	default:
		PRINT_INFO("[danacloud_mode_changed] mode unknown\n");
		return;
	}
}



