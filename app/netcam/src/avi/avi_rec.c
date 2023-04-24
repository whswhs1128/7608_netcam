/*!
*****************************************************************************
** FileName     : avi_rec.c
**
** Description  : avi record.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create       : 2015-8-11, create
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "avi_common.h"
#include "avi_utility.h"
#include "avi_rec.h"
#include "utility_api.h"
#include "media_fifo.h"
#include "cfg_all.h"

#include "avi_rec.h"
#include "g711.h"
#include "adpcm.h"
#include "mmc_api.h"
#include "netcam_api.h"
#include "sdk_sys.h"

#define CAL_TIME 0



int rec_buf_size = 600*1024;
char *data_array = NULL;
int data_array_index = 0;
int data_array_pos = 0;
int rec_module_init = 0;

static AviFile   rec_avi_file;
static AviInitParam  rec_avi_init_param;


pthread_mutex_t rec_time_mutex_t = PTHREAD_MUTEX_INITIALIZER;


int rec_pthread_running = 0;
pthread_t rec_func_pid = 0;


static u64t rec_time_stop = 0;
static u64t motion_rec_time_stop = 0;
static u64t alarm_rec_time_stop = 0;
static int sche_rec_flag = 0;
static int manu_rec_flag = 0;
static u64t sche_rec_time_stop = 0;
static u64t manu_rec_fixtime_stop = 0;
static u64t manu_rec_time_start = 0;
static u64t manu_rec_time_stop = 0;
static u64t sche2_rec_time_start = 0;
static u64t sche2_rec_time_stop = 0;
static int sche2_rec_flag = 0;


MEDIABUF_HANDLE readerId = NULL;
//static unsigned int readerId1 = 0;
//static unsigned int readerId2 = 0;
//static unsigned int readerId3 = 0;
//static char *rec_data_buf = NULL;
//static int rec_buf_length = 512 * 1024;

//static u32t rec_manage_type;


//static FILE *rec_h264_file = NULL;
//static int rec_h264_fd = -1;


//static int alarm_time_len; //IO报警(从界面设置得到)
//static int motion_time_len; //移动监测报警(从界面设置得到)
//static int manu_time_len; //手动报警
//static int cmd_time_len; //手动预设报警

//static unsigned int ch_num = 0;
static int rec_mode = RECORD_FIXED_DURATION;
static int rec_size_m = 0;
volatile int rec_duration = 0;
//int fps = 0;
static int flag_audio_enable = 0;
//static int compression_type = 0;

/*
SD卡可写返回0;否则返回-2
*/

static int init_rec_param(AviInitParam *avi_init_parm)
{
    //从界面得到录像相关参数
    memset((void *)avi_init_parm, 0, sizeof(AviInitParam));
    avi_init_parm->ch_num = runRecordCfg.stream_no;
    PRINT_INFO("channel no = %u.\n", avi_init_parm->ch_num);
    int ch = runRecordCfg.stream_no;

    if (ch < 0 || ch > 3) {
        PRINT_ERR("stream%d is not support.\n", ch);
        return -1;
    }

    if (runVideoCfg.vencStream[ch].enable == 0) {
        PRINT_ERR("stream%d is not encode, so can't be record.\n", ch);
        return -1;
    }

    /* video parameter */
    // TODO: need vido h265 parameter adapt first.
    #if 1
    if (runVideoCfg.vencStream[ch].h264Conf.rc_mode == 0)
        avi_init_parm->bps = runVideoCfg.vencStream[ch].h264Conf.bps * 1000;
    else
        //avi_init_parm->bps = runVideoCfg.vencStream[ch].h264Conf.bps * 1000;//max
        avi_init_parm->bps = (int)videoMap[ch][12].max * 1000;
    #else
    avi_init_parm->bps = runVideoCfg.vencStream[ch].h264Conf.cbrAvgBps * 1000;
    #endif
    avi_init_parm->fps = netcam_video_get_actual_fps(ch); //runVideoCfg.vencStream[ch].h264Conf.fps;
    avi_init_parm->width = runVideoCfg.vencStream[ch].h264Conf.width;
    avi_init_parm->height = runVideoCfg.vencStream[ch].h264Conf.height;

    if ((runRecordCfg.audioRecEnable == 1) && (runAudioCfg.mode != 0)) {
        flag_audio_enable = 1;
        avi_init_parm->audio_enable = 1;
    }

    int enc_type = runVideoCfg.vencStream[ch].enctype;
    switch (enc_type) {
        case 1: {
			avi_init_parm->codec_type = ENC_TYPE_H264;
			PRINT_INFO("compression type is h264.\n");
			break;
        }
		case 2: {
			avi_init_parm->codec_type = ENC_TYPE_MJPEG;
			PRINT_INFO("compression type is mjpeg.\n");
			break;
        }
		case 3: {
			avi_init_parm->codec_type = ENC_TYPE_H265;
			PRINT_INFO("compression type is h265.\n");
			break;
        }
        default:
            PRINT_ERR("encode type error, encode type = %d.\n", enc_type);
            break;
    }

    /* audio parameter */
    #if 1
    avi_init_parm->a_enc_type = runRecordCfg.recAudioType; // 0 a-law; 1 u-law; 2 pcm; 3-adpcm
    avi_init_parm->a_chans = runAudioCfg.chans;
    avi_init_parm->a_rate = runAudioCfg.sampleRate;
    avi_init_parm->a_bits = runAudioCfg.sampleBitWidth;
    #else
    avi_init_parm->a_enc_type = 0; // 0 a-law; 1 u-law; 2 pcm; 3-adpcm
    avi_init_parm->a_chans = 1;
    avi_init_parm->a_rate = 8000;
    avi_init_parm->a_bits = 16;
    #endif
    PRINT_INFO("init avi param: chans:%d rate:%d bit:%d \n", avi_init_parm->a_chans, avi_init_parm->a_rate, avi_init_parm->a_bits);

    /* record parameter */
    if(runRecordCfg.recycleRecord == 1)
        rec_mode = RECORD_FIXED_DURATION |RECORD_DELETE_OLD_FILES;
    else
        rec_mode = RECORD_FIXED_DURATION;

    avi_init_parm->mode = rec_mode;
					       //mode = RECORD_FIXED_SIZE;
					       //mode = RECORD_FIXED_SIZE | RECORD_DELETE_OLD_FILES;
					       //mode = RECORD_FIXED_DURATION | RECORD_DELETE_OLD_FILES;

    avi_init_parm->size_m = rec_size_m;
    avi_init_parm->duration = rec_duration;
    PRINT_INFO("rec_size_m = %d rec_duration = %d .\n", rec_size_m, rec_duration);

    return 0; //init success
}




static int sd_rec_start()
{
    int ret = 0;
    init_rec_param(&rec_avi_init_param); //从配置中得到设置参数
    ret = avi_record_open(&rec_avi_file, &rec_avi_init_param);
    return ret;
}

static int sd_rec_stop(void)
{
    int ret = 0;
	ret = avi_record_close(&rec_avi_file);
	netcam_osd_update_id();

    return ret;
}


static int rec_func_loop(void)
{
    int ret = 0;
    int avi_size = 0;
    int tmp32_video_index = 0;
    int tmp32_audio_index = 0;
    //u32t tmp32_timetick = 0;
    struct timeval ts, te;
    struct timezone tz;
    GK_NET_FRAME_HEADER header = {0};
    int have_audio = 1;
    int readIframe = 1;
    int skip = 1, preRecOffset = 0;

    ret = mmc_get_sdcard_stauts();
    if(SD_STATUS_OK != ret){
        if(SD_STATUS_READONLY == ret){
            return -2;//stop rec
        }
        return -1;
    }
    //check write enable
    ret = mmc_sdcard_write_detect();
    if(ret < 0){
        mmc_sdcard_status_check();
        return -2;//stop rec
    }
    
    gettimeofday(&ts, &tz);
    ts.tv_sec -= tz.tz_minuteswest*60;
    /* 使用预录功能 */
    if (runRecordCfg.preRecordTime > 0) {
        preRecOffset = runRecordCfg.preRecordTime+1;
        if(ts.tv_sec >= rec_avi_file.time_e)
        {
            ts.tv_sec = rec_avi_file.time_e;
            if(rec_avi_file.time_b)
            {
                ts.tv_sec = rec_avi_file.time_b;
                preRecOffset = 0;
            }
            mediabuf_set_oldest_frame(readerId);            
            skip = 0;            
            PRINT_INFO("use pre record %d(%d)s.\n", runRecordCfg.preRecordTime, preRecOffset);
        }
        else
            mediabuf_set_newest_frame(readerId);
        rec_avi_file.time_b = 0;
    }

	ret = sd_rec_start();
    if (ret < 0) {
        PRINT_ERR("call sd_rec_start failed\n");
        return -1;
    }

    PRINT_INFO("1, write avi header ok.");
    netcam_osd_update_id();
    PRINT_INFO("before while, ch_num = %d.\n", rec_avi_init_param.ch_num);

#if CAL_TIME
    struct timeval tt1;
    struct timeval tt2;
    struct timeval tt3;
    struct timeval tt4;
    struct timeval tt5;
    struct timeval tt6;
#endif

    /* encodetype、fps 和 res 用于当编码类型、fps 和 分辨率改变，停止录像  */
    int encodetype = -1; // H.264:CODEC_ID_H264 0;H.265:CODEC_ID_H265 6
    int fps = 0;
    int res = 0;

	while (rec_module_init) {

        int frameType;

#if CAL_TIME
        gettimeofday(&tt1, NULL);
#endif

        //正在升级，退出录像
        if(netcam_get_update_status() != 0)
        {
            break;
        }

        //从zfifo读取数据
        char *recv_buf = NULL;
        int data_length = 0;
        if(readIframe == 1)
        {
            if(skip)
                ret = mediabuf_read_I_frame(readerId, (void **)&recv_buf, &data_length, &header);
            else
            {
                ret = mediabuf_read_next_I_frame(readerId, (void **)&recv_buf, &data_length, &header);
                if(ts.tv_sec > (header.sec+preRecOffset))
                    continue;
            }
            preRecOffset = ts.tv_sec-header.sec;
            PRINT_INFO("preRecordTime:%d(%d)s, rec_stime:%d(%d)s, fps:%d\n", runRecordCfg.preRecordTime, preRecOffset, ts.tv_sec, header.sec, header.frame_rate);
            readIframe = 0;
            encodetype = header.media_codec_type;
            fps = header.frame_rate;
            res = header.video_reso;
            rec_avi_file.time_s = header.sec;
            ts.tv_sec = header.sec;
            if(preRecOffset < 0)
                preRecOffset = 0;
            
            if((header.media_codec_type == 0 && rec_avi_init_param.codec_type != ENC_TYPE_H264) ||
                (header.media_codec_type == 6 && rec_avi_init_param.codec_type != ENC_TYPE_H265))
            {
                PRINT_INFO("Use header I frame para codec_type:%d->%d\n", rec_avi_init_param.codec_type, header.media_codec_type);
                rec_avi_init_param.codec_type = header.media_codec_type;
            }
            if(rec_avi_init_param.fps != header.frame_rate)
            {
                PRINT_INFO("Use header I frame para fps:%d->%d\n", rec_avi_init_param.fps, header.frame_rate);
                rec_avi_init_param.fps = header.frame_rate;
                rec_avi_file.video_fps = rec_avi_init_param.fps;
            }
            int width  = (header.video_reso>>16)&0xFFFF;
            int height = header.video_reso&0xFFFF;
            if((rec_avi_init_param.width != width) ||
               (rec_avi_init_param.height != height))
            {
                PRINT_INFO("Use header I frame para video_reso:%dx%d->%dx%d\n", 
                    rec_avi_init_param.width, rec_avi_init_param.height, width, height);
                rec_avi_init_param.width  = width;
                rec_avi_init_param.height = height;
            }
        }
        else
        {
            ret = mediabuf_read_frame(readerId, (void **)&recv_buf, &data_length, &header);
        }
        if (ret < 0) {
			PRINT_ERR("stram%d : mediabuf_read_frame fail.\n", rec_avi_init_param.ch_num);
    		break;
        } else if (ret == 0) {
            PRINT_INFO("read no data.\n");
            usleep(10000);
            continue;
        }

        /* 当编码类型、fps 和 分辨率改变，停止录像  */
        if ((header.frame_type == GK_NET_FRAME_TYPE_I) || (header.frame_type == GK_NET_FRAME_TYPE_P)) {
            if (encodetype != header.media_codec_type) {
                rec_avi_file.time_b = header.sec;
                PRINT_INFO("stream%d, code type is changed:%d->%d, to stop from rec loop.\n", rec_avi_init_param.ch_num, encodetype, header.media_codec_type);
                break;
            }

            /* 当fps改变，跳出录像循环，停止录像  */
            if (fps != header.frame_rate) {
                rec_avi_file.time_b = header.sec;
                PRINT_INFO("stream%d, fps is changed:%d->%d, to stop from rec loop.\n", rec_avi_init_param.ch_num, fps, header.frame_rate);
                break;
            }

            /* 当分辨率改变，跳出录像循环，停止录像  */
            if (res != header.video_reso) {
                rec_avi_file.time_b = header.sec;
                PRINT_INFO("stram%d, resolution is changed:%d->%d, to stop from rec loop.\n", rec_avi_init_param.ch_num, res, header.video_reso);
                break;
            }
        }

        if (RECORD_IS_FIXED_SIZE(rec_avi_init_param.mode)) {
            //PRINT_INFO("fix size \n");
            avi_size = rec_avi_file.data_offset + 8 + rec_avi_file.index_count*16 + 100; //leave 100 BYTE space
            if(avi_size > rec_avi_init_param.size_m * 1024 * 1024) {
                PRINT_INFO("rec size of one file is %d MB, time to stop.\n", rec_avi_init_param.size_m);
                break;
            }
        } else if(RECORD_IS_FIXED_DURATION(rec_avi_init_param.mode)) {
            //PRINT_INFO("fix duration \n");
            te.tv_sec = header.sec; // gettimeofday(&t2, NULL);
            // 防止当时间有更新，T2<T1时会导致录像文件超大
            if(te.tv_sec < ts.tv_sec)
            {
                PRINT_INFO("rec time of one file is %d(%d, %d) minutes, time to chage.\n", rec_avi_init_param.duration, te.tv_sec, ts.tv_sec);
                break;
            }
            if ((te.tv_sec - ts.tv_sec - preRecOffset) >= (rec_avi_init_param.duration * 60)) {
                if((header.sec - rec_avi_file.time_e) >= (rec_avi_init_param.duration * 60))
                    rec_avi_file.time_b = header.sec;
                PRINT_INFO("rec time of one file is %d(%d, %d) minutes, time to stop.\n", rec_avi_init_param.duration, te.tv_sec, ts.tv_sec);
                break;
            }
        } else {
            PRINT_ERR_MSG("rec mode error.");
            break;
        }

        if (rec_pthread_running == 0) {
            PRINT_INFO("rec_func_loop, stop record thread.\n");
            break;
        }

        u64t tmp64 = get_time_to_u64t(header.sec);// get_u64t_now_time();
        if ((tmp64 >= rec_time_stop) && (tmp64 >= manu_rec_fixtime_stop) && (sche_rec_flag == 0) && (manu_rec_flag == 0) && (sche2_rec_flag == 0)) {
            PRINT_INFO("rec_func_loop, stop record.\n");
            break;
        }

		pthread_mutex_lock(&rec_time_mutex_t);
		int write_data_buf = data_array_pos + 8 + data_length+20;
		// 如果新数据帧比原有buffer大，则先将旧数据刷入SD卡中
		//将最大帧大小做为新的录像buffer。
		if (write_data_buf >= rec_buf_size) 
	    {

			ret = avi_record_write_cache(&rec_avi_file);
			if (ret < 0)
			{
				rec_pthread_running = 0;
				pthread_mutex_unlock(&rec_time_mutex_t);
				break;
			}
			if (data_array)
			{
				free(data_array);
				data_array = NULL;
			}
			
			PRINT_ERR("Rec buf size too small,Need to malloc again.new %d KB : old %d KB\n", write_data_buf/1024,rec_buf_size/1024);
			rec_buf_size = write_data_buf+512;
			
			data_array = malloc(rec_buf_size);
	        if (data_array == NULL){
	            PRINT_ERR("Malloc record buffer:%dKB failed\n",rec_buf_size/1024);
				pthread_mutex_unlock(&rec_time_mutex_t);
				ret = -2;
	            break;
	        }

			//memset(data_array,0,rec_buf_size);
			
	    }
		pthread_mutex_unlock(&rec_time_mutex_t);

        if ((header.frame_type == GK_NET_FRAME_TYPE_A) && flag_audio_enable)
        {
            if (rec_avi_init_param.a_enc_type == 2) { //pcm
                memcpy(data_array + data_array_pos + 8, recv_buf, data_length);
            } else {
                PRINT_ERR("get audio type error:%d.\n", rec_avi_init_param.a_enc_type);
            }
        }
        else if((header.frame_type == GK_NET_FRAME_TYPE_I) || (header.frame_type == GK_NET_FRAME_TYPE_P)) 
        {
            memcpy(data_array + data_array_pos + 8, recv_buf, data_length);
        }
        else
        {
            PRINT_ERR("header.frame_type type error:%d.\n", header.frame_type);
            continue;
        }

#if CAL_TIME
        gettimeofday(&tt2, NULL);
        printf("get buf time: %d us \n", (tt2.tv_sec-tt1.tv_sec)*1000000+(tt2.tv_usec-tt1.tv_usec));
#endif

        #if 1
        /* 打印是否丢帧，并设置 frameType */
        if (header.frame_type == GK_NET_FRAME_TYPE_A) {
            if (have_audio == 1) {
                printf("******** mediabuf have audio ************ \n");
                have_audio = 0;
                if (rec_avi_file.audio_enable)
                    printf("********  rec audio enable ************ \n");
                else
                    printf("********  rec audio disable ************ \n");
            }
            /* 如果不录制音频帧，则continue */
            if (rec_avi_file.audio_enable) {
                frameType = 2;
                if (tmp32_audio_index != 0) {
                    if(header.frame_no - tmp32_audio_index != 1)
                        PRINT_ERR("\n\n ====> in write avi,get buf lost %d audio frame data. last_no=%d now_no = %d\n\n", header.frame_no - tmp32_audio_index, tmp32_audio_index, header.frame_no);
                }
                tmp32_audio_index = header.frame_no;
            }
            else
                continue;

        }else if((header.frame_type == GK_NET_FRAME_TYPE_I) || (header.frame_type == GK_NET_FRAME_TYPE_P)) {
            if(header.frame_type == GK_NET_FRAME_TYPE_I)
                frameType = 1;
            else
                frameType = 0;

            if (tmp32_video_index != 0) {
                if(header.frame_no - tmp32_video_index != 1)
                    PRINT_ERR("\n\n ====> in write avi, get buf lost %d video frame data. last_no=%d now_no = %d\n\n", header.frame_no - tmp32_video_index, tmp32_video_index, header.frame_no);
            }
            tmp32_video_index = header.frame_no;
        } else if(header.frame_type == GK_NET_FRAME_TYPE_JPEG) {
			frameType = 3;

            if (tmp32_video_index != 0) {
                if(header.frame_no - tmp32_video_index != 1)
                    PRINT_ERR("\n\n ====> in write avi, get buf lost %d jpeg frame data.\n\n", header.frame_no - tmp32_video_index);
            }
            tmp32_video_index = header.frame_no;
        } else {
            PRINT_ERR("frametype error.\n");
            continue;
        }
        #endif

        /* 自生成时间戳，放在AVI的隐藏tunk中 */
        u32t time32 = header.sec;        
        rec_avi_file.time_e = header.sec;

#if CAL_TIME
        gettimeofday(&tt3, NULL);
#endif
        /* 向AVI文件写入帧数据 */
        ret = avi_record_write(&rec_avi_file, &rec_avi_init_param, data_array + data_array_pos + 8, data_length, frameType, time32);
		if (ret < 0)
		{
			rec_pthread_running = 0;
			break;
		}
#if CAL_TIME
        gettimeofday(&tt4, NULL);
        printf("write buf time: %d us , datalen=%d, frametype=%d\n",
                    (tt4.tv_sec-tt3.tv_sec)*1000000+(tt4.tv_usec-tt3.tv_usec), data_length,  frameType);
#endif
	}
    //rec_pthread_running = 0;

    PRINT_INFO("2, write stream data ok.");
    //PRINT_INFO("1, rec_pthread_running = %d.\n", rec_pthread_running);
    ret = sd_rec_stop();
    if(ret < 0)
        return ret;
    //PRINT_INFO("2, rec_pthread_running = %d.\n", rec_pthread_running);
    PRINT_INFO("3, end avi file ok.");
    return 0;
}

static void rec_pthread_func(void *args)
{
    sdk_sys_thread_set_name("rec_pthread_func");
    pthread_detach(rec_func_pid);

    int ch_num = runRecordCfg.stream_no;
    PRINT_INFO("init ch_num = %d.\n", ch_num);
    readerId = mediabuf_add_reader(ch_num);
    if(readerId == NULL) {
        PRINT_ERR("add_recoder_user error ");
        return;
    }

    if(data_array == NULL)
    {
        data_array = malloc(rec_buf_size);
        if(data_array == NULL)
        {
            PRINT_ERR("Malloc record buffer:%dKB failed\n",rec_buf_size/1024);
            goto error_rec ;
        }
    }
    //memset(data_array, 0, rec_buf_size);

    u64t tmp64 = 0;
    int ret;
    while (rec_module_init) {
        ret = rec_func_loop();
        if (ret == -2) { //sd card is read only
            rec_pthread_running = 0;
        }
        else if(ret < 0) {
           rec_pthread_running= 0; // exit record
        }

        //正在升级，退出录像
        if(netcam_get_update_status() != 0)
        {
            rec_pthread_running = 0;
        }


        tmp64 = get_u64t_now_time();
        PRINT_INFO("now time = %llu, stop time = %llu\n", tmp64, rec_time_stop);

        pthread_mutex_lock(&rec_time_mutex_t);
        if(rec_pthread_running == 0) {
            PRINT_INFO("stop record thread.\n");
            rec_time_stop = 0;
            motion_rec_time_stop = 0;
            alarm_rec_time_stop = 0;
            sche_rec_time_stop = 0;
            manu_rec_fixtime_stop = 0;
            manu_rec_time_start = 0;
            manu_rec_time_stop = 0;
            pthread_mutex_unlock(&rec_time_mutex_t);

            break;
        }

        if ((tmp64 >= rec_time_stop) && (tmp64 >= manu_rec_fixtime_stop) \
            && (sche_rec_flag == 0) && (manu_rec_flag == 0) && (sche2_rec_flag == 0) ) {
            PRINT_INFO("stop record.\n");
            rec_pthread_running = 0;
            pthread_mutex_unlock(&rec_time_mutex_t);

            break;
        }
        pthread_mutex_unlock(&rec_time_mutex_t);
    }

error_rec:
    if(readerId)
    {
        mediabuf_del_reader(readerId);
        readerId = NULL;
    }

    if(data_array)
    {
    	free(data_array);
        data_array = NULL;
    }

    rec_func_pid = 0;
    rec_pthread_running = 0;

	PRINT_INFO("~~~~~~~~~~~~~record thread over~~~~~~~~~~~~~~~\n");

	netcam_osd_update_id();
    return;
}


void thread_record_create(void)
{
    //正在升级，退出录像
	int retVal;

    PRINT_INFO("begin to init sd card.\n");
    retVal = mmc_get_sdcard_stauts();
	PRINT_ERR("sdcard retVal:%d\n", retVal);
    if(SD_STATUS_OK != retVal){
        return ;
    }

    if (rec_func_pid == 0) {
		netcam_osd_update_id();
        rec_pthread_running = 1;
        PRINT_INFO("Create record thread.\n");
        retVal = pthread_create(&rec_func_pid, NULL, (void *)&rec_pthread_func, NULL);
		if (retVal != 0)
		{
			PRINT_INFO("~~~~~~~~~~~~~record create thread failed~~~~~~~~~~~~~~~\n");
		}
		PRINT_INFO("~~~~~~~~~~~~~record create thread succeed~~~~~~~~~~~~~~~\n");



    } else {
        PRINT_ERR("thread_record is running, can't be created again.");
    }

}

void thread_record_close(void)
{
    if (rec_func_pid != 0) { //如果开了record子线程，则等待回收子线程
		rec_pthread_running = 0;
        PRINT_ERR("to close record pthread\n");
        //pthread_join(rec_func_pid, NULL);
        while(rec_func_pid != 0 || avi_reocrd_check_file_stoped())
        {
            usleep(100*1000);
        }
		
        PRINT_ERR("record pthread is closed.");
    } else {
        LOG_INFO("thread_record is closed, needn't be close.");
    }
}

int is_thread_record_running(void)
{
    return (rec_func_pid != 0);
}



void motion_rec_start(int num, int rec_time, int rec_time_once)
{
    if (runRecordCfg.enable == 0)
    {
        return;
    }
    
    if(rec_module_init == 0 || mmc_get_sdcard_stauts() != SD_STATUS_OK)
    {
        return;
    }
    u64t start_64 = get_u64t_now_time();

    //PRINT_INFO("jump into motion_rec_start.\n");
    //避免时间内重复响应
    if (start_64 < motion_rec_time_stop) {
        if(is_thread_record_running())
            return;
    }
    pthread_mutex_lock(&rec_time_mutex_t);
    //写管理文件
    motion_rec_time_stop = u64t_add_seconds(start_64, rec_time*60);
    if(motion_rec_time_stop > rec_time_stop)
        rec_time_stop = motion_rec_time_stop;
    if(write_manage_info(DMS_NET_RECORD_TYPE_MOTION, num, start_64, motion_rec_time_stop) < 0)
    {
        pthread_mutex_unlock(&rec_time_mutex_t);
        return;
    }
    
    //如果当前没有录像则开始录像
    if (is_thread_record_running()) {
        PRINT_INFO("to create record thread failed, because record thread is running, \n");
        pthread_mutex_unlock(&rec_time_mutex_t);
        return;
    }

    if(rec_time < rec_time_once)
        rec_duration = rec_time;
    else
        rec_duration = rec_time_once;

	PRINT_INFO("motion_rec_start rec_time:%d, rec_time_once:%d, rec_duration:%d\n",rec_time, rec_time_once, rec_duration);
	thread_record_create();
	pthread_mutex_unlock(&rec_time_mutex_t);
    

}


void alarm_rec_start(int num, int rec_time, int rec_time_once)
{
    if(rec_module_init == 0 || mmc_get_sdcard_stauts() != SD_STATUS_OK)
    {
        return;
    }
    PRINT_INFO("jump into alarm_rec_start.\n");
    u64t start_64 = get_u64t_now_time();
    //避免时间内重复响应
    if (start_64 < alarm_rec_time_stop) {
        if(is_thread_record_running())
            return;
    }

    pthread_mutex_lock(&rec_time_mutex_t);
    //写管理文件
    alarm_rec_time_stop = u64t_add_seconds(start_64, rec_time*60);
    if(alarm_rec_time_stop > rec_time_stop)
        rec_time_stop = alarm_rec_time_stop;
    if(write_manage_info(DMS_NET_RECORD_TYPE_ALARM, num, start_64, alarm_rec_time_stop) < 0)
    {
        PRINT_INFO("Write manage info error\n");
        pthread_mutex_unlock(&rec_time_mutex_t);
        return;
    }
    //如果当前没有录像则开始录像
    if (is_thread_record_running()) {
        PRINT_INFO("to create record thread failed, because record thread is running, \n");
        pthread_mutex_unlock(&rec_time_mutex_t);
        return;
    }

    if(rec_time < rec_time_once)
        rec_duration = rec_time;
    else
        rec_duration = rec_time_once;
	thread_record_create();
    pthread_mutex_unlock(&rec_time_mutex_t);
    
	PRINT_INFO("~~~~~~~~~~~~~record create thread~~~~~~~~~~~~~~~\n");
}


void sche_rec_start(int num, int rec_time_once)
{
    if(rec_module_init == 0 || mmc_get_sdcard_stauts() != SD_STATUS_OK)
    {
        return;
    }
    sche_rec_flag = 1;
    u64t start_64 = get_u64t_now_time();

    //避免时间内重复响应
    if(start_64 < sche_rec_time_stop) {
        if(is_thread_record_running())
            return;
    }

    pthread_mutex_lock(&rec_time_mutex_t);
    //写管理文件
    sche_rec_time_stop = u64t_add_seconds(start_64, rec_time_once*60);

#if 0
    if(write_manage_info(DMS_NET_RECORD_TYPE_SCHED, num, start_64, sche_rec_time_stop) < 0)
    {
        PRINT_INFO("Write manage info error\n");
        pthread_mutex_unlock(&rec_time_mutex_t);
        return;
    }
#endif
    PRINT_INFO("start time = %llu, sche_rec_time_stop = %llu, rec_time_stop = %llu\n",
            start_64, sche_rec_time_stop, rec_time_stop);

    //如果当前没有录像则开始录像
    if (is_thread_record_running()) {
        PRINT_INFO("to create record thread failed, because record thread is running, \n");
        pthread_mutex_unlock(&rec_time_mutex_t);
        return;
    }

    rec_duration = rec_time_once;
	thread_record_create();
    pthread_mutex_unlock(&rec_time_mutex_t);
    

}

void sche_rec_stop(void)
{
    //PRINT_INFO("jump into sche_rec_stop.\n");
    sche_rec_flag = 0;
}


void sche2_rec_start(int rec_time_once)
{
    if(rec_module_init == 0 || mmc_get_sdcard_stauts() != SD_STATUS_OK)
    {
        return;
    }
    //避免时间内重复响应
    if (sche2_rec_flag) {
        if(is_thread_record_running())
            return;
    }
    sche2_rec_time_start = get_u64t_now_time();
    sche2_rec_flag = 1;
    sche2_rec_time_stop = 0;

    pthread_mutex_lock(&rec_time_mutex_t);
    //正在录像
    if (is_thread_record_running()) {
        PRINT_INFO("to create record thread failed, because record thread is running, \n");
        pthread_mutex_unlock(&rec_time_mutex_t);
        return;
    }

    rec_duration = rec_time_once;
	thread_record_create();
    pthread_mutex_unlock(&rec_time_mutex_t);
    

}

void sche2_rec_stop(int num)
{
    sche2_rec_time_stop = get_u64t_now_time();

    if (sche2_rec_flag == 1) {
        sche2_rec_flag = 0;
        write_manage_info(DMS_NET_RECORD_TYPE_SCHED_2, num, sche2_rec_time_start, sche2_rec_time_stop);
    }
    else //manu_rec_fixtime_start
        sche2_rec_flag = 0;

}
void manu_rec_start_fixtime(int num, int rec_time, int rec_time_once)
{
    if(rec_module_init == 0 || mmc_get_sdcard_stauts() != SD_STATUS_OK)
    {
        return;
    }

    PRINT_INFO("jump into manu_rec_fixtime_start.\n");
    u64t start_64 = get_u64t_now_time();
    manu_rec_flag = 0;
    //避免时间内重复响应
    if(start_64 < manu_rec_fixtime_stop) {
        if(is_thread_record_running())
            return;
    }

    pthread_mutex_lock(&rec_time_mutex_t);
    //写管理文件
    manu_rec_fixtime_stop = u64t_add_seconds(start_64, rec_time*60);

    if(write_manage_info(DMS_NET_RECORD_TYPE_ALARM, num, start_64, alarm_rec_time_stop) < 0)
    {
        PRINT_INFO("Write manage info error\n");
        pthread_mutex_unlock(&rec_time_mutex_t);
        return;
    }
    //如果当前没有录像则开始录像
    if (is_thread_record_running()) {
        PRINT_INFO("to create record thread failed, because record thread is running, \n");
        pthread_mutex_unlock(&rec_time_mutex_t);
        return;
    }

    if(rec_time < rec_time_once)
        rec_duration = rec_time;
    else
        rec_duration = rec_time_once;
    thread_record_create();

    pthread_mutex_unlock(&rec_time_mutex_t);


}

void manu_rec_start_alltime(int num, int rec_time_once)
{
    if(rec_module_init == 0 || mmc_get_sdcard_stauts() != SD_STATUS_OK)
    {
        return;
    }
    PRINT_INFO("jump into manu_rec_alltime_start.\n");
    //避免时间内重复响应
    if (manu_rec_flag) {
        if(is_thread_record_running())
            return;
    }

    manu_rec_time_start = get_u64t_now_time();
    manu_rec_flag = 1;
    manu_rec_fixtime_stop = 0;

    pthread_mutex_lock(&rec_time_mutex_t);
    //正在录像
    if (is_thread_record_running()) {
        PRINT_INFO("to create record thread failed, because record thread is running, \n");
        pthread_mutex_unlock(&rec_time_mutex_t);
        return;
    }

    rec_duration = rec_time_once;
	
	thread_record_create();
    pthread_mutex_unlock(&rec_time_mutex_t);
    
}

void manu_rec_stop(int num)
{
    if(rec_module_init == 0)
    {
        return;
    }
    PRINT_INFO("jump into manu_rec_stop.\n");
    manu_rec_time_stop = get_u64t_now_time();

    if (manu_rec_flag == 1) {//manu_rec_alltime_start
        manu_rec_flag = 0;
        write_manage_info(DMS_NET_RECORD_TYPE_MANU, num, manu_rec_time_start, manu_rec_time_stop);
    }
    else //manu_rec_fixtime_start
        manu_rec_fixtime_stop = 0;
}


void do_schedule_recode_by_slice(void)
{
    //PRINT_INFO("rec enable:%d recmode:%d \n", runRecordCfg.enable, runRecordCfg.recordMode);
	//判断是否录像

	int shce_rec_flag = 0;
    if(rec_module_init == 0 || mmc_get_sdcard_stauts() != SD_STATUS_OK)
    {
        return;
    }

	if(netcam_get_update_status()  == -1)
	{
		netcam_timer_del_task(do_schedule_recode_by_slice);
		return;
	}

	if (runRecordCfg.recordMode == 3) { //停止录像
        shce_rec_flag = 0;
    } else if(runRecordCfg.enable) { //本地录像打开，支持本地录像
		if(runRecordCfg.recordMode == 1) {
            //PRINT_INFO("slice: all day record.\n");
			shce_rec_flag = 1;
		} else if(runRecordCfg.recordMode == 0) { // 按预设时间录像
		    //PRINT_INFO("slice: check slice.\n");
		    
        #ifdef MODULE_SUPPORT_MOJING_V4
            shce_rec_flag = mojing_time_in_shedule(&(runRecordCfg.mojingRecTime[0]), 256);
        #else
			struct tm tt;
	        sys_get_tm(&tt);
	        //int halfhour = tt.tm_hour * 2 + tt.tm_min /30; //第几个半小时
	        int mask = tt.tm_hour * 4 + (tt.tm_min / 15);
            unsigned int tmp = 0;
            if (mask < 32) {
                tmp = runRecordCfg.scheduleSlice[tt.tm_wday][0];
                mask -= 0;
            } else if ((mask >= 32) && (mask < 64)) {
                tmp = runRecordCfg.scheduleSlice[tt.tm_wday][1];
                mask -= 32;
            } else {
                tmp = runRecordCfg.scheduleSlice[tt.tm_wday][2];
                mask -= 64;
            }

            if (tmp & (1 << mask))
                shce_rec_flag = 1;
            else
                shce_rec_flag = 0;
        #endif        
		}
	}

	//通过flag，进行录像或停止录像
	if (shce_rec_flag == 1) {
        //PRINT_INFO("schedule record or all day record enable.\n");
	    int rec_time_once = runRecordCfg.recordLen;
		int ch = runRecordCfg.stream_no;
		sche_rec_start(ch, rec_time_once);
	} else {
	    //PRINT_INFO("schedule record or all day record disable.\n");
		sche_rec_stop();
	}

	return;
}


void do_schedule_recode_by_period(void)
{
#ifndef MODULE_SUPPORT_GOOLINK
	//判断是否录像
	int shce_rec_flag = 0;
    if(rec_module_init == 0 || mmc_get_sdcard_stauts() != SD_STATUS_OK)
    {
        return;
    }

    if(netcam_get_update_status()  == -1)
	{
		netcam_timer_del_task(do_schedule_recode_by_period);
		return;
	}


    if (runRecordCfg.recordMode == 3) { //停止录像
        shce_rec_flag = 0;
    } else if(runRecordCfg.enable) { //本地录像打开，支持本地录像
		if(runRecordCfg.recordMode == 1)
			shce_rec_flag = 1;
		else if(runRecordCfg.recordMode == 0) { // 按预设时间录像
            char time_now[30] = {0};
            get_nowhm_str(time_now);
            u64t now_64 = atoll(time_now);

        	u64t start_64 = 0;
        	u64t stop_64 = 0;
            int i, j;
            char str[30];
            GK_SCHEDTIME *pSche = NULL;
            for (i = 0; i < 7; i ++) {
                for (j = 0; j < 4; j ++) {
                    pSche = &(runRecordCfg.scheduleTime[i][j]);

                    /* get start time & get stop time */
                    //change_schedtime_u64t(pSche, &start_64, &stop_64);
                	memset(str, 0, sizeof(str));
                	sprintf(str, "%02d%02d00", pSche->startHour, pSche->startMin);
                    start_64 = atoll(str);
                	memset(str, 0, sizeof(str));
                	sprintf(str, "%02d%02d00", pSche->stopHour, pSche->stopMin);
                    stop_64 = atoll(str);

                    if (start_64 <= stop_64) {
                        if ((now_64 >= start_64) && (now_64 <= stop_64)) {
                            shce_rec_flag = 1;
                            break;
                        }

                    } else {
                        if ((now_64 >= start_64) || (now_64 <= stop_64)) {
                            shce_rec_flag = 1;
                            break;
                        }
                    }


                }
            }
		}
	}

	//通过flag，进行录像或停止录像
	if (shce_rec_flag == 1) {
        //PRINT_INFO("schedule record or all day record enable.\n");
        int rec_time_once = runRecordCfg.recordLen;
        sche2_rec_start(rec_time_once);
	} else {
	    //PRINT_INFO("schedule record or all day record disable.\n");
        int ch = runRecordCfg.stream_no;
        sche2_rec_stop(ch);
	}
#else

	//判断是否录像
	int shce_rec_flag = 0;
	u64t start_64	  = 0;
	u64t stop_64	  = 0;
	u64t now_64 	  = 0;
	char time_now[30] = {0};
	char str[30]	  = {0};

	if(rec_module_init == 0 || mmc_get_sdcard_stauts() != SD_STATUS_OK)
	{
		return;
	}

	if(netcam_get_update_status()  == -1)
	{
		netcam_timer_del_task(do_schedule_recode_by_period);
		return;
	}


	if (runRecordCfg.recordMode == 3) { //停止录像
		shce_rec_flag = 0;
	} else if(runRecordCfg.enable) { //本地录像打开，支持本地录像
		if(runRecordCfg.recordMode == 1)
			shce_rec_flag = 1;
		else if(runRecordCfg.recordMode == 0) { // 按预设时间录像
            #ifdef MODULE_SUPPORT_MOJING_V4
            shce_rec_flag = mojing_time_in_shedule(&(runRecordCfg.mojingRecTime[0]), 256);
            #endif

            #ifdef MODULE_SUPPORT_GOOLINK
            if (shce_rec_flag == 0)
            {
    			struct tm tt;
    			sys_get_tm(&tt);
    			get_nowhm_str(time_now);
    			now_64 = (u64t)atoll(time_now);
    			GK_SCHEDTIME *pSche = NULL;
    			SCHEDTIME_EXT *pScheExt = NULL;

    			if (runP2PCfg.scheduleTimeEnable[tt.tm_wday] == 1)
    			{
    				pSche = &(runRecordCfg.scheduleTime[tt.tm_wday][0]);
    				pScheExt = &(runP2PCfg.scheduleTimeSec[tt.tm_wday][0]);

    				/* get start time & get stop time */
    				memset(str, 0, 30);
    				sprintf(str, "%02d%02d%02d", pSche->startHour, pSche->startMin,pScheExt->startSec);
    				start_64 = (u64t)atoll(str);

    				memset(str, 0, 30);
    				if(pSche->stopHour == 23 && pSche->stopMin == 59 && pScheExt->stopSec >=55)
    					sprintf(str, "%02d%02d55", pSche->stopHour, pSche->stopMin);
    				else
    					sprintf(str, "%02d%02d%02d", pSche->stopHour, pSche->stopMin,pScheExt->stopSec);

    				stop_64 = (u64t)atoll(str);

    				if ((start_64 < stop_64) && ((now_64 >= start_64) && (now_64 < stop_64)))
    				{
    					shce_rec_flag = 1;
    				}
    			}
            }
            #endif
		}
	}

	//通过flag，进行录像或停止录像
	if (shce_rec_flag == 1) {
		//PRINT_INFO("schedule record or all day record enable.\n");
        int rec_time_once = runRecordCfg.recordLen;
        sche2_rec_start(rec_time_once);

	} else {
		//PRINT_INFO("schedule record or all day record disable.\n");
		int ch = runRecordCfg.stream_no;
		sche2_rec_stop(ch);
	}

	return;


#endif
    return;
}


#if 0
static int local_record_pcm(void)
{
    int ret = 0;
    int data_length = 0;
    GK_NET_FRAME_HEADER header = {0};

    //添加读者
    MEDIABUF_HANDLE reader = mediabuf_add_reader(0);
    if (reader == NULL)
    {
        PRINT_ERR("mediabuf_add_reader error.\n");
        return -1;
    }

    FILE *fp = fopen("/test/audio_test.pcm", "wb+");
    if (fp == NULL)
    {
        PRINT_ERR("fopen error.\n");
        mediabuf_del_reader(reader);
        return -1;
    }
    PRINT_INFO("1, open ok.");

    int have_audio = 1;
    int have_video = 1;
    char *buf;

    struct timeval t1, t2;
    gettimeofday(&t1, NULL);
	while (1)
    {

        buf = NULL;
        data_length = 0;
        //从zfifo读取数据
        ret = mediabuf_read_frame(reader, &buf, &data_length, &header);
        if (ret <= 0)
        {
            PRINT_INFO("read no data.\n");
            usleep(3000);
            continue;
        }

        if (header.frame_type == GK_NET_FRAME_TYPE_A)
        {
            if (have_audio == 1)
            {
                printf("******** mediabuf have audio, rec pcm ************ \n");
                printf("a len:%d\n", data_length);
                have_audio = 0;
            }
            ret = fwrite(buf, 1, data_length, fp);
            if (ret <= 0)
            {
                PRINT_ERR("write error.\n");
                continue;
            }
        }
        else if ( (header.frame_type == GK_NET_FRAME_TYPE_I) || (header.frame_type == GK_NET_FRAME_TYPE_P) )
        {
            if (have_video == 1)
            {
                printf("******** mediabuf have video ************ \n");
                printf("v len:%d\n", data_length);
                have_video = 0;
            }
        }

        gettimeofday(&t2, NULL);
        if ((t2.tv_sec - t1.tv_sec) >= 30)
        {
            PRINT_INFO("rec pcm, time to stop.\n");
            break;
        }
    }

    PRINT_INFO("2, write audio data ok.");
    if (fp)
    {
        fclose(fp);
        fp = NULL;
    }

    mediabuf_del_reader(reader);
    PRINT_INFO("3, end audio file ok.");
    return 0;

}


static void *local_record_avi_thread(void *param)
{
    local_avi_record();
    return NULL;
}

static void *local_record_pcm_thread(void *param)
{
    local_record_pcm();
    return NULL;
}

int local_record_start()
{
    rec_pthread_running = 1;

    pthread_t thread_id;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    //pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);//detached
    if(0 != pthread_create(&thread_id, &attr, local_record_avi_thread, NULL))
    {
        pthread_attr_destroy(&attr);
        return -1;
    }
    pthread_attr_destroy(&attr);

#if 0
    pthread_t rc_pcm_id;
	if (0 != pthread_create(&rc_pcm_id, NULL, local_record_pcm_thread, NULL))
	{
		PRINT_ERR("fail to start local_record_pcm_thread.\n");
		return -1;
	}
#endif

    return 0;
}


int local_record_stop()
{
    rec_pthread_running = 0;

    return 0;
}
#endif

int local_record_init()
{
    if(rec_module_init)
    {
        PRINT_INFO("Record has inited");
        return 0;
    }
    if (pthread_mutex_init(&rec_time_mutex_t, NULL) < 0) {
        PRINT_ERR("init rec_time_mutex_t error.");
        return -1;
    } else {
        PRINT_INFO("init rec_time_mutex_t success.");
    }
    rec_module_init = 1;


#ifdef MODULE_SUPPORT_GOOLINK
    netcam_timer_add_task(do_schedule_recode_by_period, NETCAM_TIMER_ONE_SEC, SDK_TRUE, SDK_FALSE);	// 4*250ms =  1s
#else
    netcam_timer_add_task(do_schedule_recode_by_slice,  NETCAM_TIMER_ONE_SEC, SDK_TRUE, SDK_FALSE);	// 2s
#endif

    return 0;
}

void local_record_uninit()
{
    if(rec_module_init )
    {
        rec_module_init = 0;
        PRINT_INFO("close record module..");
#ifdef MODULE_SUPPORT_GOOLINK
        netcam_timer_del_task(do_schedule_recode_by_period);
#else
		netcam_timer_del_task(do_schedule_recode_by_slice);
#endif
		thread_record_close();
        PRINT_INFO("close record module ok..");

        pthread_mutex_destroy(&rec_time_mutex_t);
    }
}


