#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <signal.h>

#include "Anyan_Device_SDK.h"

#include "sdk_sys.h"
#include "media_fifo.h"
#include "netcam_api.h"
#include "avi_search_api.h"

#include "avi_rec.h"
#include "avi_utility.h"
#include "utility_api.h"
#include "mmc_api.h"
#include "flash_ctl.h"
#include "eventalarm.h"
#include "work_queue.h"
#include "ipcsearch.h"
#include "aac.h"
#include "network_wifi.h"

#define ZBAR 1
#define ULUCU_DEBUG 1
#define ENABLE_PHOTO_WIFI
#define OEM_ID         100029
#define OEM_NAME        "HZ"


static uint8  enable_upload = 0;	//通知设备上传数据（为1时），或者停止上传数据（为0时）
static uint16 upload_rate = 384;	//上传码率，为0表示上传或者停止上传所有码率
static uint8  channel_index = 1;	//通道索引，为0表示上传或者停止上传所有通道数据

static int stream_id = 0;

static char sn_ulk_id[32]={0};
static int  talk_flag=0;

static int  his_playback = 0;
static int  his_playbacking = 0;
static uint16	pb_rate = 384;
static uint16	pb_channelindex = 0;
static FILE_NODE node;

//static int avi_flag_pause;
static AviPBHandle *pUluPbHand = NULL;
static pthread_mutex_t pb_getframe_x = PTHREAD_MUTEX_INITIALIZER;
//static int alarm_time_table_num;
static int ulucuRuning = 0;
static Dev_SN_Info oem_info;
static Dev_Attribut_Struct  attr;
static char cfgDir[64]={"/opt/custom/ulucu/"};
static char sta_psd[128];
static char sta_ssid[128];
static char sta_ssid_prefix[32];
static char sta_smartlin_ok = 0;
static bool get_id_result = false;
static void* ulu_pt = 0;
static WIFI_ENC_MODE encMode;

static int ulucu_motion_alarm();
static int capture_picture_ctrl();

static int app_ulucu_motion_event_upload(int nChannel, int nAlarmType, int nAction, void* pParam)
{
	if(nAction == 0)
	{
		return 0;
	}
    switch(nAlarmType)
    {
        case GK_ALARM_TYPE_VMOTION:          //3:移动侦测

            /*Ulucu 报警上传*/
        	capture_picture_ctrl();
        	ulucu_motion_alarm();
            break;

        default:
			PRINT_INFO("Alarm: Type = %d", nAlarmType);
        	break;
    }

    //PRINT_INFO("alarm_type:%s action:%s\n", eventType, strAction);
    return 0;
}

static void app_ulu_wifi_cb(char  *ssid, char *password, ULK_ENTICATION_MODE encrypt_type)
{
    if(sta_smartlin_ok == 0)
    {
        int len = strlen(ssid);
        int prefix_len = strlen(sta_ssid_prefix);
        int ssid_offset = 0;
        memset(sta_ssid, 0, sizeof(sta_ssid));
        memset(sta_psd, 0, sizeof(sta_psd));
        PRINT_INFO("Anyan app_ulu_wifi_cb,ssid:%s, psk:%s\n", ssid, password);
        if(strstr(ssid, "(null)"))
        {
            ssid_offset = 6;
        }
        else if((len >= prefix_len) && strstr(ssid, sta_ssid_prefix))
        {
            ssid_offset = prefix_len;
        }
        else
        {
            ssid_offset = 0;
        }
        encMode = (WIFI_ENC_MODE)encrypt_type;
        strncpy(sta_ssid, ssid + ssid_offset, sizeof(sta_ssid));
    	strncpy(sta_psd, password, sizeof(sta_psd));
    	sta_smartlin_ok = 1;
        PRINT_INFO("Anyan smartlink call back,ssid:%s,psk:%s,enc:%d\n",sta_ssid,sta_psd,encMode);
	}
}

static int app_ulucu_smartlink_start()
{
    int ret = 0;
    if(!ulucuRuning)
        return ret;
    sta_smartlin_ok = 0;
    ret = Ulu_SDK_Start_Wifi_Monitor();
    if(ret != 0)
    {
        PRINT_ERR("Start ulucu wifi error:%d",ret);
    }
    return ret;
}

static int app_ulucu_smartlink_stop()
{
    if(!ulucuRuning)
        return 0;
    Ulu_SDK_Stop_Wifi_Monitor();
    sta_smartlin_ok = 0;
    return 0;
}

static int app_ulucu_smartlink_get(char *ssid, char *psd,WIFI_ENC_MODE *ulu_enc)
{
    int ret = -1;
    if(ssid == NULL || psd == NULL)
    {
        PRINT_ERR("parameter error");
        return -1;
    }
    if(sta_smartlin_ok == 1)
    {
        strcpy(ssid,sta_ssid);
        strcpy(psd,sta_psd);
        if(strlen(psd) == 0)
        {
            *ulu_enc = WIFI_ENC_NONE;
        }
        else
        {
            *ulu_enc = WIFI_ENC_WPA2PSK_TKIP; // force enc
        }

        sta_smartlin_ok = 0;
        memset(sta_ssid,0,sizeof(sta_psd));
        memset(sta_psd,0,sizeof(sta_psd));
        ret = 0;
    }

    return ret;
}

static int set_device_param(CMD_PARAM_STRUCT * args)
{
	int i;
	ST_GK_ENC_STREAM_H264_ATTR h264Attr;
	GK_NET_IMAGE_CFG stImagingConfig;

	netcam_video_get(0,stream_id, &h264Attr);
    netcam_image_get(&stImagingConfig);
	int command = (args->cmd_args[0]<<8) + args->cmd_args[1];
	switch(command)
	{
		case 1:
			runImageCfg.brightness = args->cmd_args[2];
			runImageCfg.saturation = args->cmd_args[3];
			runImageCfg.contrast = args->cmd_args[4];
			runImageCfg.hue = args->cmd_args[5];
			break;
		case 2:
			PRINT_INFO("args->cmd_args[2] : %d\n",args->cmd_args[2]);
			if(1 == args->cmd_args[2])
			{
				stImagingConfig.flipEnabled = 1;
			}
			else
			{
				stImagingConfig.mirrorEnabled = 1;
			}
			break;
		case 3:
		    //arg = (args->cmd_args[2]<<8) + args->cmd_args[3];
			h264Attr.bps = (args->cmd_args[4]<<8) + args->cmd_args[5];
			h264Attr.fps = (args->cmd_args[6]<<8) + args->cmd_args[7];
			h264Attr.quality = (args->cmd_args[8]<<8) + args->cmd_args[9];
			break;
		case 4:
			strncpy((char *) runVideoCfg.vencStream[0].h264Conf.name,(char *)&args->cmd_args[3],17);
			for(i=0;i<4;i++){
                memcpy(&runChannelCfg.channelInfo[0].osdChannelName, &runChannelCfg.channelInfo[i].osdChannelName, sizeof(GK_NET_OSD_CHANNEL_NAME));
			}
            netcam_osd_update_title();
			break;
		default:
			break;

	}
	netcam_video_set(0, stream_id, &h264Attr);
	netcam_image_set(stImagingConfig);

	return 0;
}
static int get_device_param()
{
	ULK_Video_Param_Ack video_param;
	ULK_Video_Encode_Param_Ack video_enc;
	memset(&video_param,0,sizeof(video_param));
	memset(&video_enc,0,sizeof(video_enc));
	ST_GK_ENC_STREAM_H264_ATTR h264Attr;
	netcam_video_get(0, stream_id, &h264Attr);

	video_param.channel_index = channel_index;
	video_param.brightness = runImageCfg.brightness;
	video_param.Contrast = runImageCfg.contrast;
	video_param.saturation = runImageCfg.saturation;
	video_param.tone = runImageCfg.hue;

	video_enc.bit_rate = h264Attr.bps;
	video_enc.channel_index = channel_index;
	video_enc.frame_rate = h264Attr.fps;
	video_enc.rate_index = 0;
	video_enc.video_quality = h264Attr.quality;

	Ulu_SDK_Param_Vedio_Upload(&video_param);
	Ulu_SDK_Code_Param_Vedio_Upload(&video_enc);
	return 0;
}


static int record_history_list(CMD_PARAM_STRUCT * args)
{
	u64t start_64 = 0;
	u64t stop_64 = 0;
	char tmp[20]={0};
	int i = 0;
	int ret;
    FILE_NODE searchNode;
	struct tm *t_start,*t_end;
    int seq = 0;

	if(grd_sd_is_device_exist() == 0)
	{
		Ulu_SDK_History_Srch_Rslt_None();
		PRINT_ERR("system has not a MMC device\n");
		return -1;
	}

	int rate = args->cmd_args[0] + (args->cmd_args[1]<<8);
	HistoryListQuery_Struct pEvent;
	memset(&pEvent,0,sizeof(HistoryListQuery_Struct));
	pEvent.channel_index = args->channel; //channel_index;
	pEvent.rate = rate;//384;

	pEvent.start_time = args->cmd_args[2] + (args->cmd_args[3]<<8) + (args->cmd_args[4]<<16) + (args->cmd_args[5]<<24);
	pEvent.end_time = args->cmd_args[6] + (args->cmd_args[7]<<8) + (args->cmd_args[8]<<16) + (args->cmd_args[9]<<24);

	t_start = localtime((time_t *)&(pEvent.start_time));
	sprintf(tmp, "%04d%02d%02d%02d%02d%02d", t_start->tm_year+1900, t_start->tm_mon+1, t_start->tm_mday,
										t_start->tm_hour, t_start->tm_min, t_start->tm_sec);
	start_64 = atoll(tmp);
	t_end = localtime((time_t *)&(pEvent.end_time));
	sprintf(tmp, "%04d%02d%02d%02d%02d%02d", t_end->tm_year+1900, t_end->tm_mon+1, t_end->tm_mday,
										t_end->tm_hour, t_end->tm_min, t_end->tm_sec);
	stop_64 = atoll(tmp);
	FILE_LIST *list = search_file_by_time(0, stream_id, start_64, stop_64);
	if(list == NULL)
	{
		Ulu_SDK_History_Srch_Rslt_None();
		PRINT_ERR("list is NULL\n");
		return -1;
	}
	int file_num = get_len_list(list);
    PRINT_INFO("file_num=%d\n", file_num);
    do {
        for (i = 0; i < 128 && i < file_num; i ++)
    	{
            memset(&searchNode, 0, sizeof(FILE_NODE));
            ret = get_file_node(list, &searchNode);
            if (ret != DMS_NET_FILE_SUCCESS)
    		{
                PRINT_ERR("get_file_node failed, ret = %d\n", ret);
                break;
            }
            print_node(&searchNode);
        	AVI_DMS_TIME stime,etime;
            u64t_to_time(&stime, searchNode.start);
            u64t_to_time(&etime, searchNode.stop);
    		struct tm st,st2;
    		time_t timep,timep2;
    		//PRINT_INFO("FileSize:%d\n", get_file_size(node.path));

			st.tm_year = stime.dwYear-1900;
			st.tm_mon = stime.dwMonth-1;
			st.tm_mday = stime.dwDay;
			st.tm_hour = stime.dwHour;
			st.tm_min = stime.dwMinute;
			st.tm_sec = stime.dwSecond ;

			st2.tm_year = etime.dwYear-1900;
			st2.tm_mon = etime.dwMonth-1;
			st2.tm_mday = etime.dwDay;
			st2.tm_hour = etime.dwHour;
			st2.tm_min = etime.dwMinute;
			st2.tm_sec = etime.dwSecond ;

			timep = mktime(&st);
			timep2 = mktime(&st2);
			pEvent.hList[i].start_time = (uint32)(timep);
			pEvent.hList[i].end_time = (uint32)(timep2);
			//PRINT_INFO("pEvent.hList[%d].start_time : %lu\n",i,pEvent.hList[i].start_time);
			//PRINT_INFO("pEvent.hList[%d].end_time : %lu\n",i,pEvent.hList[i].end_time);

    	}
        pEvent.seq = seq;
        pEvent.list_len = i;
    	PRINT_INFO("pEvent.list_len : %lu\n",pEvent.list_len);
    	PRINT_INFO("pEvent.seq : %lu\n",pEvent.seq);
    	//INT_INFO("pEvent.start_time : %d,pEvent.end_time: %d,rate:%d\n",pEvent.start_time,pEvent.end_time,rate);

    	ret = Ulu_SDK_History_List_Send(&pEvent);
    	if(ret == 0 )
    	{
    		PRINT_INFO("send history succeed!\n");
    	}
        file_num -= i;
        if(file_num > 0)
        {
            seq++;
        }
    }while(file_num > 0);
	search_close(list);

	return 0;
}

static AviPBHandle *ulu_PlayBackByTime(u64t start, u64t stop)
{
    AviPBHandle *pUluPbHander = NULL;
    pUluPbHander = (AviPBHandle *)create_pb_handle();
    if (NULL == pUluPbHander)
	{
        PRINT_ERR("pUluPbHander is NULL\n");
        return NULL;
    }

    pUluPbHander->list = search_file_by_time(0,stream_id, start, stop);  //0， 所有的情况
    if (NULL == pUluPbHander->list)
	{
        PRINT_ERR("list is null.\n");
		Ulu_SDK_History_Srch_Rslt_None();
		avi_pb_close(pUluPbHander);
        return NULL;
    }

    print_pb_handle(pUluPbHander);

    return pUluPbHander;
}


static int playback_send_frame(AviPBHandle *pUluPbHandle)
{
    int ret;
    int size = 0;

    // PRINT_INFO("no = %d, count = %d \n", p_frame->no, p_frame->index_count);
    //播放结束时，如果是单文件(链表为NULL)返回200002
    //如果不是，则取下一个文件
    if (pUluPbHandle->no >= pUluPbHandle->index_count)
	{
        PRINT_INFO("playback %s end. no = %d\n", pUluPbHandle->file_path, pUluPbHandle->no);
        //单文件
        if (pUluPbHandle->list == NULL)
		{
            PRINT_INFO("playback end\n");
            return 0;
        }
        //多文件
        memset(&node, 0, sizeof(FILE_NODE));
        ret = get_file_node(pUluPbHandle->list, &node);
        if (ret != DMS_NET_FILE_SUCCESS)
		{
            PRINT_ERR("get_file_node failed, ret = %d\n", ret);
            return -1;
        }
        print_node(&node);
        ret = avi_pb_open(node.path, pUluPbHandle);
        if (ret < 0)
		{
            PRINT_ERR("avi_pb_open %s error.\n", node.path);
            return -2;
        }
    }

    pthread_mutex_lock(&pb_getframe_x);
    ret = avi_pb_get_frame(pUluPbHandle, NULL, &size);
    pthread_mutex_unlock(&pb_getframe_x);
    if (ret < 0)
	{
        PRINT_ERR("call avi_pb_get_frame failed.\n");
        return -3;
    }
    if (ret == GK_NET_NOMOREFILE)
	{
		Ulu_SDK_History_Frame_Send(NULL, ULK_END);
		PRINT_INFO("playback file end.\n");
        return 0;
    }

	Stream_History_Frm_Struct frm_history;

	frm_history.channelnum = pb_channelindex;
	frm_history.bit_rate = pb_rate;
	frm_history.len = size;//pUluPbHandle->node.size;

	if (pUluPbHandle->node.frame_type == 0x00)
	{
		frm_history.frm_type = CH_HIS_AUDIO_FRM;
	}
	else if (pUluPbHandle->node.frame_type == 0x11)// I
	{
        frm_history.frm_type = CH_HIS_I_FRM;
    }
	else if (pUluPbHandle->node.frame_type == 0x10) // P
	{
        frm_history.frm_type = CH_HIS_P_FRM;
    }
	else
	{
		return 1;
	}
	frm_history.frm_ts = pUluPbHandle->pb_timetick_tmp;
	frm_history.pdata = (uint8*)pUluPbHandle->node.one_frame_buf;

	//his_playback_statu = ULK_NO_END;
	int ret_Ulu = Ulu_SDK_History_Frame_Send(&frm_history, ULK_NO_END);

	//PRINT_INFO("1ret_Ulu : %d,stream.frm_ts : %d,stream.frm_len : %d\n",ret_Ulu,frm_history.frm_ts,frm_history.len);
	if (ret_Ulu < 0)
	{
		PRINT_INFO("ret_Ulu : %d,stream.frm_ts : %d,stream.frm_len : %d\n",ret_Ulu,frm_history.frm_ts,frm_history.len);
		if (ret_Ulu == -1)
		{
			PRINT_INFO("No network connection connect!\n");
		}
	}
	return ret_Ulu;
}


static int ulu_PlayBackStop()
{
	PRINT_INFO("Playback stop\n");
    if(pUluPbHand)
    {
        avi_pb_close(pUluPbHand);
        pUluPbHand = NULL;
    }


    return 0;
}

static int record_history_play(CMD_PARAM_STRUCT * args)
{
	u64t start_64 = 0;
	u64t stop_64 = 0;
	char tmp[20]={0};
	int count = 20;
	struct tm *t_start,*t_end;

	if(grd_sd_is_device_exist() == 0)
	{
		Ulu_SDK_History_Srch_Rslt_None();
        PRINT_ERR("system has not a MMC device\n");
		return -1;
	}
    pb_channelindex = args->channel;
	pb_rate = args->cmd_args[0] + (args->cmd_args[1]<<8);
	uint64 start = args->cmd_args[2] + (args->cmd_args[3]<<8) + (args->cmd_args[4]<<16) + (args->cmd_args[5]<<24);
	//int stop = args->cmd_args[6] + (args->cmd_args[7]<<8) + (args->cmd_args[8]<<16) + (args->cmd_args[9]<<24);
	//PRINT_ERR("~~~~~~~~~~~~stop time %llu~~~~~~~~~~~~~~~~~~\n",start);
	//PRINT_ERR("~~~~~~~~~~~~stop time %d~~~~~~~~~~~~~~~~~~\n",stop);
	uint64 s_end = start + 1;

	t_start = localtime((time_t *)&(start));
    PRINT_INFO("Recoder,play start time:%4d-%02d-%02d %02d:%02d:%02d\n",t_start->tm_year+1900,t_start->tm_mon+1,t_start->tm_mday,t_start->tm_hour,t_start->tm_min,t_start->tm_sec);
	sprintf(tmp, "%04d%02d%02d%02d%02d%02d", t_start->tm_year+1900, t_start->tm_mon+1, t_start->tm_mday,
										t_start->tm_hour, t_start->tm_min, t_start->tm_sec);
	start_64 = atoll(tmp);

	t_end = localtime((time_t *)&(s_end));
	PRINT_INFO("Recoder,play start time:%4d-%02d-%02d %02d:%02d:%02d\n",t_end->tm_year+1900,t_end->tm_mon+1,t_end->tm_mday,t_end->tm_hour,t_end->tm_min,t_end->tm_sec);
	sprintf(tmp, "%04d%02d%02d%02d%02d%02d", t_end->tm_year+1900, t_end->tm_mon+1, t_end->tm_mday,
										t_end->tm_hour, t_end->tm_min, t_end->tm_sec);
	stop_64 =  atoll(tmp);

	pUluPbHand = ulu_PlayBackByTime(start_64, stop_64);
	if (pUluPbHand)
	{

		his_playback = 1;
		his_playbacking = 0;
		while(his_playbacking == 0 && count-- > 20)
		{
			usleep(40*1000);
		}
    	//Gk_pb_SendFrameLoop(pUluPbHand,rate);
    	//Gk_PlayBackStop(pUluPbHand);
		//enable_upload = 1;
	}
    //sem_destroy(&step_sem);
	return 0;
}



static int record_history_stop()
{
	int waitTime = 20;
	his_playback = 0;
	while(his_playbacking == 1 && waitTime-- > 0)
	{
		usleep(100000); //wait shend data close
	}
	PRINT_INFO("record_history_stop start!\n");
	if(waitTime <= 0)
	{
		PRINT_INFO("exceed 20s to wait play exit\n");
	}
	PRINT_INFO("record_history_stop succeed!\n");
	return 0;
}

static int sd_card_set(CMD_PARAM_STRUCT * args)
{
	int retVal;
	if (0 == args->cmd_args[0])
	{
		grd_sd_format();
	}
	else if(1 == args->cmd_args[0])
	{
		//uint32 sdc_size = grd_sd_get_all_size();
		//uint32 sdc_free = grd_sd_get_free_size();
	}
	else
	{
		PRINT_ERR("command line error!\n");
	}
	AYE_TYPE_SD_STATUS status = AYE_VAL_SD_NOOP;
	AYE_TYPE_RECORD_MODE mode	= is_thread_record_running();
    mode = AYE_VAL_RECMODE_NOOP;
	retVal = Ulu_SDK_Device_Record_Report(status, mode);
	if (retVal != 0)
	{
		PRINT_INFO("Ulu_SDK_Device_Record_Report failed\n");
	}
	return 0;
}

static int video_record_set(CMD_PARAM_STRUCT * args)
{
	int rec_time_once = runRecordCfg.recordLen;
	if ((args->cmd_args[1]== 8) || (args->cmd_args[1] == 16))
	{
        if (2 == args->cmd_args[0])
		{
           manu_rec_start_alltime(runRecordCfg.stream_no, rec_time_once);
        }
		else if (1 == args->cmd_args[0])
            manu_rec_stop(runRecordCfg.stream_no);
        else if (0 == args->cmd_args[0])
            PRINT_INFO("set record mode.\n");
		else
			PRINT_INFO("wAction error.\n");
    }
	else
	{
        PRINT_INFO("nRecordType error.\n");
    }

    return 0;
}

static int Alarm_Ctrl(CMD_PARAM_STRUCT *args)
{
	int i,j;

	#if 1
	int start_hour,end_hour;
	AY_Alarm_Struct ay_alarm;
  	memcpy(&ay_alarm,args->cmd_args,sizeof(AY_Alarm_Struct));
	runMdCfg.enable = ay_alarm.alarm_flag;
	runMdCfg.handle.is_rec = ay_alarm.alarm_flag;
	runMdCfg.handle.intervalTime = ay_alarm.alarm_interval_mix;
	#if ULUCU_DEBUG
	PRINT_INFO("ay_alarm.alarm_flag : %d\n",ay_alarm.alarm_flag);
	PRINT_INFO("ay_alarm.alarm_interval_mix : %d\n",ay_alarm.alarm_interval_mix);
	PRINT_INFO("to do set Alarm time!\n");
	#endif
	if(ay_alarm.alarm_time_table_num >= 5)
	{
		ay_alarm.alarm_time_table_num = 4;
	}
	for(i = 0;i < 7;i++)
	{
		for(j = 0;j < ay_alarm.alarm_time_table_num;j++)
		{
			runMdCfg.scheduleTime[i][j].startHour = ay_alarm.alarm_time_table[j].start / 3600;
			start_hour = ay_alarm.alarm_time_table[j].start / 3600;
			runMdCfg.scheduleTime[i][j].startMin = (ay_alarm.alarm_time_table[j].start - start_hour*3600) / 60;

			runMdCfg.scheduleTime[i][j].stopHour = ay_alarm.alarm_time_table[j].end / 3600;
			end_hour = ay_alarm.alarm_time_table[j].end / 3600;
			runMdCfg.scheduleTime[i][j].stopMin = (ay_alarm.alarm_time_table[j].end - end_hour*3600) / 60;
		}
	}
	#if ULUCU_DEBUG
	PRINT_INFO("Alarm set time succeed!\n");
	#endif
	#endif
	return 0;
}

static int image_filp_ctrl(CMD_PARAM_STRUCT *args)
{
	GK_NET_IMAGE_CFG stImagingConfig;
	netcam_image_get(&stImagingConfig);
	if(EXT_IMAGE_QUERY_TURN == args->cmd_id)
	{
	   if(Ulu_SDK_EXT_Response_Image_Turn(stImagingConfig.mirrorEnabled, stImagingConfig.flipEnabled)==-1)
	   		PRINT_INFO("Ulu_SDK_EXT_Response_Image_Turn failed hturn: %d,vturn: %d\n",stImagingConfig.mirrorEnabled,stImagingConfig.flipEnabled);
	   return 0;

	}
	stImagingConfig.flipEnabled = args->cmd_args[1];
	stImagingConfig.mirrorEnabled = args->cmd_args[0];

	netcam_image_set(stImagingConfig);
	return 0;
}

static int ulucu_motion_alarm(){
	int retVal = Ulu_SDK_Alarm_Upload_Ex(channel_index,MOVE_DETECT);
	if (retVal < 0){
		PRINT_INFO("Ulu_SDK_Alarm_Upload_Ex failed : %d\n",retVal);
	}
	return 0;
}

static int capture_picture_ctrl()
{
	FILE *f;
	int retVal;
	int snap_index = 0;
	struct tm tt;
	sys_get_tm(&tt);
	char md_snap_path[64] = {0};
	Screen_Shot_Struct ppic;
	memset(&ppic,0,sizeof(Screen_Shot_Struct));
	sprintf(md_snap_path, "/tmp/md_%04d%02d%02d_%02d%02d%02d_%d.jpg", tt.tm_year, tt.tm_mon, tt.tm_mday, tt.tm_hour, tt.tm_min, tt.tm_sec, snap_index);
	netcam_video_snapshot(runVideoCfg.vencStream[1].h264Conf.width, runVideoCfg.vencStream[1].h264Conf.height,md_snap_path, GK_ENC_SNAPSHOT_QUALITY_MEDIUM);
	f=fopen(md_snap_path,"rb");

	if(f==NULL)
		return -1;
	fseek(f,0,SEEK_END);
	ppic.pic_length = ftell(f);
	fseek(f,0,SEEK_SET);
	ppic.pic_data = (uint8*)malloc(ppic.pic_length);
	fread(ppic.pic_data,1,ppic.pic_length,f);

	ppic.update_rate = upload_rate;
	ppic.screenshot_ts = Ulu_SDK_GetTickCount();
	ppic.alarm_type = MOVE_DETECT;
	ppic.channel_index = channel_index;
	ppic.picture_type = JPG;
	retVal = Ulu_SDK_Screen_Shot_Upload(&ppic);
	if (retVal < 0)
	{
		PRINT_INFO("Ulu_SDK_Screen_Shot_Upload failed retVal :%d\n",retVal);
	}
    free(ppic.pic_data);
	fclose(f);
	unlink(md_snap_path);
	if ( access(md_snap_path,0) == 0)
    {
		PRINT_INFO("%s delete failed", md_snap_path);
	}
    else
	{
		PRINT_INFO("%s  delete success ",md_snap_path);
	}
	return 0;
}


/******************************************************************************
* Description	: PTZ_Ctrl_exe处理回调消息消息
* Arguments	:  channel:通道 ,args :命令参数
* Returns	:
* Caller 		: APP layer.
* Notes		:
*******************************************************************************/
static void PTZ_Ctrl (CMD_PARAM_STRUCT *args)
{
	int speed   = -1;
    int step    = 20; //10 最小步长，10,20,30,40，100最大
	#if ULUCU_DEBUG
    PRINT_INFO("ch=%d,", args->channel);
	#endif
    if ((args->cmd_args[0]) == 0)//stop
    {
		#if ULUCU_DEBUG
        PRINT_INFO("ptz stop \n");
		#endif
		netcam_ptz_stop();
        return;
    }
    switch(args->cmd_args[1])
    {
    case PTZ_CTRL_LEFT:
		#if ULUCU_DEBUG
        PRINT_INFO("ptz left start..\n");
		#endif
		netcam_ptz_left(step, speed);
        break;
    case PTZ_CTRL_RIGHT:
		#if ULUCU_DEBUG
        PRINT_INFO("ptz right start..\n");
		#endif
		netcam_ptz_right(step, speed);
        break;
    case PTZ_CTRL_UP:
		#if ULUCU_DEBUG
        PRINT_INFO("ptz up start..\n");
		#endif
		netcam_ptz_up(step, speed);
        break;
    case PTZ_CTRL_DOWN:
		#if ULUCU_DEBUG
        PRINT_INFO("ptz down start..\n");
		#endif
		netcam_ptz_down(step, speed);
        break;
    case PTZ_CTRL_FOCUS_IN:
		#if ULUCU_DEBUG
        PRINT_INFO("ptz focus in start..\n");
		#endif
        break;
    case PTZ_CTRL_FOCUS_OUT:
		#if ULUCU_DEBUG
        PRINT_INFO("ptz focus out start..\n");
		#endif
        break;
	case PTZ_CTRL_PTZ_STOP:
		#if ULUCU_DEBUG
        PRINT_INFO("ptz stop \n");
		#endif
		netcam_ptz_stop();
        break;
    case PTZ_CTRL_OPEN_VIDEO:
		#if ULUCU_DEBUG
        PRINT_INFO("PTZ_CTRL_OPEN_VIDEO..\n");
		#endif
        break;
    case PTZ_CTRL_CLOSE_VIDEO:
		#if ULUCU_DEBUG
        PRINT_INFO("PTZ_CTRL_CLOSE_VIDEO..\n");
		#endif
		netcam_ptz_left_up(step, speed);
        break;
    case PTZ_CTRL_SYS_RESET:
		#if ULUCU_DEBUG
        PRINT_INFO("PTZ_CTRL_SYS_RESET..\n");
		#endif
		netcam_ptz_left_down(step, speed);
        break;
    case PTZ_CTRL_SYS_REBOOT:
		#if ULUCU_DEBUG
        PRINT_INFO("PTZ_CTRL_SYS_REBOOT..\n");
		#endif
		netcam_ptz_right_up(step, speed);
        break;
    case PTZ_CTRL_MIRROR:
		#if ULUCU_DEBUG
        PRINT_INFO("PTZ_CTRL_MIRROR..\n");
		#endif
		netcam_ptz_right_down(step, speed);
        break;
	case PTZ_CTRL_FLIP:
		#if ULUCU_DEBUG
        PRINT_INFO("PTZ_CTRL_FLIP..\n");
		#endif
        break;
    default:
        break;
    }

}

static void PTZ_Preset_Ctrl (CMD_PARAM_STRUCT *args, int flag)
{
	int ret=0;
    if (flag == 0)//stop
    {
    	int i=0;
    	ptz_preset_info_t pstPresetsInfo = {0};
		AY_PRESET_POS preset[255] = {{0}};
		#if ULUCU_DEBUG
        PRINT_INFO("ptz preset get\n");
		#endif
		ret = netcam_ptz_get_presets( &pstPresetsInfo);
		for(i=0; i<pstPresetsInfo.sizePreset; i++)
		{
			preset[i].index = pstPresetsInfo.presetNum[i];
			preset[i].x = 0;
			preset[i].y = 0;
			preset[i].z = 0;
		}
		Ulu_SDK_EXT_Response_All_Preset(preset, pstPresetsInfo.sizePreset);
    }
	else
	{
		GK_NET_CRUISE_GROUP cruise_info={0};
	    switch(args->cmd_args[0])
	    {
	    case 1:
			#if ULUCU_DEBUG
	        PRINT_INFO("ptz preset set\n");
			#endif
			ret = netcam_ptz_set_preset(args->cmd_args[1], NULL);
	        break;
	    case 2:
			#if ULUCU_DEBUG
	        PRINT_INFO("ptz preset del\n");
			#endif
			ret = netcam_ptz_clr_preset(args->cmd_args[1]);
	        break;
	    case 3:
			#if ULUCU_DEBUG
	        PRINT_INFO("ptz preset execution\n");
			#endif
			cruise_info.byPointNum = 1;
			cruise_info.byCruiseIndex = 0;
			cruise_info.byRes[0] = 0;
			cruise_info.byRes[1] = 0;
			cruise_info.struCruisePoint[0].byPointIndex = args->cmd_args[1];
			cruise_info.struCruisePoint[0].byPresetNo= args->cmd_args[1];
			cruise_info.struCruisePoint[0].byRemainTime= 1;
			cruise_info.struCruisePoint[0].bySpeed= -1;
			ret = netcam_ptz_preset_cruise(&cruise_info);
	        break;

	    default:
	        break;
	    }

    	Ulu_SDK_EXT_Response_Preset_Result(args->cmd_args[1],ret==0?1:0);
	}
	#if ULUCU_DEBUG
	PRINT_INFO("ptz preset status:%s!",ret?"success":"fail");
	#endif
}


/******************************************************************************
* Description  : VideoPlay_Ctrl
* Arguments	  :
* Returns      :
* Caller	  : app layer.
* Notes		  :
*******************************************************************************/

static int VideoPlay_Ctrl(CMD_PARAM_STRUCT *args)
{
	static int rate=0;
    uint8 upload_flag = args->cmd_args[0];
    upload_rate =(args->cmd_args[2]<<8) + args->cmd_args[1]; //上传码率，为0表示上传或者停止上传所有码率
    channel_index = args->channel;							 //通道索引，为0表示上传或者停止上传所有通道数据
    if(channel_index > netcam_video_get_channel_number())
    {
        channel_index = netcam_video_get_channel_number();
    }
	stream_id = upload_rate >UPLOAD_RATE_2 ? channel_index-1 : channel_index;
	if(stream_id > 2)
		stream_id = 2;
	#if ULUCU_DEBUG
	PRINT_INFO("VideoPlay_Ctrl->en:%d,rate:%d,ch:%d\n",upload_flag, upload_rate, channel_index);
	#endif
    //------------------------------------------------------

    if (upload_flag == 1)//传所有开关使能
    {
		if(rate != upload_rate)
		{
			ST_GK_ENC_STREAM_H264_ATTR h264Attr;
			netcam_video_get(0,  stream_id, &h264Attr);
			h264Attr.bps = upload_rate;
			//if(0 == stream_id)
			//	h264Attr.bps *=2;
			if(netcam_video_set(0,  stream_id, &h264Attr) == 0)
			{
				netcam_video_cfg_save();
			}
            usleep(100);
			rate = upload_rate;
		}
        if (channel_index == 0 )//上传所有通道数据
        {
            //open(channel_index, upload_rate);
            if(upload_rate == 0)
            {
                //上传所有波特率
                PRINT_INFO("->open all channel all rate-%d,%d\n",channel_index, upload_rate);
            }
            else
            {
                //打开所有通道指定码率数据
                PRINT_INFO("->open all channel special rate=%d,%d\n",channel_index, upload_rate);
            }
        }
        else
        {
            //打开指定通道数据
            PRINT_INFO("->open special channel=%d\n", args->channel);
        }
    }
    else if (upload_flag == 0)//关闭上传所有开关使能
    {
        if (channel_index == 0 )//关闭所有通道数据
        {
            if(upload_rate == 0)
            {
                //关闭所有波特率
                PRINT_INFO("->close all channel all rate-%d,%d\n",channel_index, upload_rate);
            }
            else
            {
                //关闭指定通道指定码率数据
                PRINT_INFO("->close all channel special rate-%d,%d\n",channel_index, upload_rate);
            }
        }
        else//关闭指定通道数据
        {
            PRINT_INFO("->close special channel=%d\n", args->channel);
        }
    }
    enable_upload = upload_flag;
    return 0;
}

/******************************************************************************
 * Description	 : Deal_CBFun处理回调消息消息
 * Arguments	 :	channel:通道 ,args :命令参数
 * Returns	 :
 * Caller		 : APP layer.
 * Notes		 :
 *******************************************************************************/
static void Deal_CBFun (CMD_PARAM_STRUCT *args)
{
    switch (args->cmd_id)
    {
    case PTZ_CTRL://云台控制
		PTZ_Ctrl(args);
        break;
    case VIDEO_CTRL://视频控制
		//enable_upload = 1;
        VideoPlay_Ctrl(args);
        break;
	case EXT_PRESET_SET://预置位点设置
		PTZ_Preset_Ctrl(args, 1);
		break;
	case EXT_PRESET_QUERY://预置位点获取
		PTZ_Preset_Ctrl(args, 0);
		break;
	case RECORD_CTRL:
		#if ULUCU_DEBUG
		PRINT_INFO("RECORD_CTRL\n");
		#endif
		video_record_set(args);
		break;
	case HISTORY_CTRL://历史回放
		#if ULUCU_DEBUG
		PRINT_INFO("HISTORY_CTRL>>rate:%d,st:%d,et:%d\n", args->cmd_args[0] + (args->cmd_args[1]<<8),
			args->cmd_args[2] + (args->cmd_args[3]<<8) + (args->cmd_args[4]<<16) + (args->cmd_args[5]<<24),
			args->cmd_args[6] + (args->cmd_args[7]<<8) + (args->cmd_args[8]<<16) + (args->cmd_args[9]<<24) );
		#endif
		if(his_playback)
		{
			record_history_stop();
		}
		record_history_play(args);

		//else
		//	his_playback_statu = ULK_END;
		break;
	case HISTORY_STOP://停止回放
		#if ULUCU_DEBUG
		PRINT_INFO("HISTORY_STOP\n");
		#endif
		record_history_stop();
		break;
	case HISTORY_LIST://获取回放列表
		#if ULUCU_DEBUG
		PRINT_INFO("HISTORY_LIST\n");
		#endif
		record_history_list(args);
		break;
	case  TIME_SYN:
        {
            unsigned int ts = (uint8)args->cmd_args[0]+((uint8)args->cmd_args[1]<<8)+((uint8)args->cmd_args[2]<<16)+((uint8)args->cmd_args[3]<<24);
            printf("TIME_SYN NTP ,UTC timestamp = %u\n",ts);
        }
        break;
	case SD_CARD_CTRL://SD卡控制
		#if ULUCU_DEBUG
		PRINT_INFO("SD_CARD_CTRL\n");
		#endif
		sd_card_set(args);
		break;
	case CAPTURE_PICTURE_CTRL://抓拍控制
		#if ULUCU_DEBUG
		PRINT_INFO("CAPTURE_PICTURE_CTRL\n");
		#endif
		//Capture_Picture_Ctrl(args);
		break;
	case DEVICE_PARAM_GET:
		#if ULUCU_DEBUG
		PRINT_INFO("get device param!\n");
		#endif
		get_device_param(args);
		break;
	case DEVICE_PARAM_SET:
		#if ULUCU_DEBUG
		PRINT_INFO("DEVICE_PARAM_SET\n");
		#endif
		set_device_param(args);
		break;
	case ERROR_INFO:
		#if ULUCU_DEBUG
		PRINT_INFO("ulu:%s\n", args->cmd_args);
		#endif
		break;
	case ALARM_CTRL://报警控制 alarm control
		#if ULUCU_DEBUG
		PRINT_INFO("ALARM_CTRL:%s\n", args->cmd_args);
		#endif
		Alarm_Ctrl(args);
        break;
	case USER_DATA:// 透传数据
		#if ULUCU_DEBUG
		PRINT_INFO("USER_DATA:%s\n", args->cmd_args);
		#endif
        break;
	case AUDIO_CTRL://音频控制
		#if ULUCU_DEBUG
		PRINT_INFO("AUDIO_CTRL:ch:%d,rate:%d,status:%d\n", args->cmd_args[0],args->cmd_args[1]+(args->cmd_args[2]<<8),args->cmd_args[3]);
		#endif
        break;
	case TALKING_CTRL://对话控制
		#if ULUCU_DEBUG
		PRINT_INFO("TALKING_CTRL:%s\n", args->cmd_args);
		#endif
        break;
	case EXT_IMAGE_QUERY_TURN:// 查询视频翻转
	case EXT_IMAGE_SET_TURN:// 设置视频翻转
		image_filp_ctrl(args);
        break;
	case EXT_REBOOT_DEVICE:// 重启设备 reboot device * args:[0] - subcmd, 1:reboot
		if(args->cmd_args[0])
		{
		    PRINT_INFO("Reboot system\n");
		    netcam_timer_add_task2(netcam_sys_operation,1,SDK_FALSE,SDK_FALSE,0,(void *)SYSTEM_OPERATION_REBOOT);
		}
	 	break;
	case EXT_RESET_DEVICE: // 恢复出厂设置 restore factory settings * args：[0] - subcmd, 1: reset
		if(args->cmd_args[0])
		{
			PRINT_INFO("Reset to factory default\n");
		    netcam_timer_add_task2(netcam_sys_operation,1,SDK_FALSE,SDK_FALSE,0,(void *)SYSTEM_OPERATION_HARD_DEFAULT);
		}
		break;
    default:
        	PRINT_ERR("Nod found command:%d\n",args->cmd_id);
        break;
    }
}

/******************************************************************************
 * Description	 : Talk_CBFun处理回调消息消息
 * Arguments	 :	args :命令参数
 * Returns	 :
 * Caller		 :
 * Notes		 :
 *******************************************************************************/
static void Talk_CBFun (ULK_Audio_Struct *args)
{
	talk_flag=1;

	switch (args->audio_type)
	{
	case	ULK_AUDIO_TYPE_AAC:
		#if ULUCU_DEBUG
		PRINT_INFO("ULK_AUDIO_TYPE_AAC=type:%d,end:%d,seq:%d,len:%d\n",args->audio_type,args->is_end, args->audio_seq,args->audio_len);
		#endif
		CAACDecoder_PushAudioFrame(1, args->audio_buf, args->audio_len, args->is_end);
		#if ULUCU_DEBUG
		PRINT_INFO("m_outLen:%lu, m_samplerate:%lu, m_channels:%d\n", m_outLen,m_samplerate,m_channels);
		#endif
		int dst_len=m_outLen/2;
		for(m_outLen=0;m_outLen<dst_len;m_outLen++)
		{
			m_outBuff[m_outLen]=m_outBuff[2*m_outLen];
		}
		// ulucu input PCM data
		netcam_audio_output((char *)m_outBuff, dst_len, NETCAM_AUDIO_ENC_RAW_PCM, SDK_TRUE);
		#if ULUCU_DEBUG
		PRINT_INFO("=======ULUCUPutAudioFrame=======\n");
		#endif
		break;
	case	ULK_AUDIO_TYPE_G711A:
		#if ULUCU_DEBUG
		PRINT_INFO("ULK_AUDIO_TYPE_G711A:%d\n", args->audio_seq);
		#endif
		break;
	case	ULK_AUDIO_TYPE_G711U:
		#if ULUCU_DEBUG
		PRINT_INFO("ULK_AUDIO_TYPE_G711U:%d\n", args->audio_seq);
		#endif
		break;
	default:
		break;
	}
}

static void get_macaddr(char *macaddr)
{
	char adapter[16]={0};
	if(netcam_net_get_detect("eth0")==0)
	{
		strcpy(adapter, "eth0");
	}
	else
	{
		char *WifiName = netcam_net_wifi_get_devname();
		if(WifiName != NULL)
		{
			strcpy(adapter, WifiName);
		}
		else
		{
			strcpy(macaddr,runNetworkCfg.wifi.mac);
			return;
		}
	}
	sdk_net_get_hwaddr(adapter,macaddr);

}

static int ulucu_p2p_config(Dev_SN_Info *oem_info, Dev_Attribut_Struct  *attr)
{
	FILE *f;
	long len;
	char *data;
	cJSON *json,*OEM,*DEVICE;
    char fileNmae[64];
	//Dev_SN_Info oem_info;
	//Dev_Attribut_Struct  attr;
	//memset(&oem_info,0,sizeof(Dev_SN_Info));
	//memset(&attr,0,sizeof(attr));
     sprintf(fileNmae,"%s/ulucu.cjson",cfgDir);

	f=fopen(fileNmae,"rb");
	if(NULL == f)
	{
		PRINT_ERR("no ulucu config file!");
		return -1;
	}
	fseek(f,0,SEEK_END);
	len=ftell(f);
	fseek(f,0,SEEK_SET);
	data=(char*)malloc(len+1);
	if(NULL == data)
	{
		PRINT_ERR("ulucu data malloc fail!");
		return -2;
	}
	fread(data,1,len,f);
	fclose(f);
	// get string from json
	json=cJSON_Parse(data);
	free(data);

	OEM = cJSON_GetObjectItem(json,"OEM");

	get_macaddr(oem_info->MAC);
	oem_info->OEMID = cJSON_GetObjectItem(OEM, "OEMID")->valueint;//厂商和ULUCU商务联系，获得此ID
	strcpy(oem_info->SN, cJSON_GetObjectItem(OEM, "SN")->valuestring); //厂商自定义的设备sn 最少8个字节最长16个字节, 只能是数字和字母, 厂商自己保证每台设备都要有唯一的SN号
	strcpy(oem_info->OEM_name, cJSON_GetObjectItem(OEM, "OEM_name")->valuestring);//厂商和ULUCU商务联系，获得OEM_NAME
	strcpy(oem_info->Model, cJSON_GetObjectItem(OEM, "Model")->valuestring);//厂商为自己的某一款设备定义的型号，最长64个字节。
	strcpy(oem_info->Factory, cJSON_GetObjectItem(OEM, "Factory")->valuestring);//厂商的全名，最长255个字节。

	DEVICE = cJSON_GetObjectItem(json,"DEVICE");

	attr->block_nums = cJSON_GetObjectItem(DEVICE, "block_nums")->valueint;//缓冲区初始化,单位是8k,200*8k
	attr->channel_num = cJSON_GetObjectItem(DEVICE, "channel_num")->valueint;//设备通道数
	//码率一定要连续.2个码率UPLOAD_RATE_4-UPLOAD_RATE_3,UPLOAD_RATE_3-UPLOAD_RATE_2,UPLOAD_RATE_2-UPLOAD_RATE_1;
	//三个码流  UPLOAD_RATE_4-UPLOAD_RATE_2,UPLOAD_RATE_3-UPLOAD_RATE_1;
	//四个码流  UPLOAD_RATE_4-UPLOAD_RATE_1
	attr->max_rate = cJSON_GetObjectItem(DEVICE, "max_rate")->valueint;
	attr->min_rate = cJSON_GetObjectItem(DEVICE, "min_rate")->valueint;
	attr->ptz_ctrl = cJSON_GetObjectItem(DEVICE, "ptz_ctrl")->valueint;
	attr->dev_type = cJSON_GetObjectItem(DEVICE, "dev_type")->valueint;

	attr->mic_flag = cJSON_GetObjectItem(DEVICE, "mic_flag")->valueint;
	attr->can_rec_voice = cJSON_GetObjectItem(DEVICE, "can_rec_voice")->valueint;		/*可以接受音频 0 不支持, 1 支持*/
	attr->hard_disk = cJSON_GetObjectItem(DEVICE, "hard_disk")->valueint;				/*是否有硬盘  0 没有, 1 有*/
    strcpy(cfgDir,cJSON_GetObjectItem(DEVICE, "p_rw_path")->valuestring);
    attr->p_rw_path = cfgDir;		//可读写目录路径
	attr->audio_type = cJSON_GetObjectItem(DEVICE, "audio_type")->valueint;//0:acc, 1:g711_a, 2:g711_u, 3:mp3
	attr->audio_chnl = cJSON_GetObjectItem(DEVICE, "audio_chnl")->valueint;
	attr->audio_smaple_rt = cJSON_GetObjectItem(DEVICE, "audio_smaple_rt")->valueint;
	attr->audio_bit_width = cJSON_GetObjectItem(DEVICE, "audio_bit_width")->valueint;

	attr->use_type = cJSON_GetObjectItem(DEVICE, "use_type")->valueint; //设备使用类型 0：对外销售设备，1：测试设备，2：演示设备。 默认0。
	attr->has_tfcard= cJSON_GetObjectItem(DEVICE, "has_tfcard")->valueint;//是否有TF卡 0: 没有 1:有

	cJSON_Delete(json);
	return 0;

}

static void get_rand_str(char s[], int number)
 {
	 char str[64] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	 int i;
	 unsigned long int seed = (unsigned long int)str+(unsigned long int)getpid()+(unsigned long int)&i +(unsigned long int)time((time_t *)NULL);
	 srand(seed);
	 for(i=0; i<number; i++)
	 {
	 	s[i] = str[(rand()%62)];
	 }
 }

 static void ulucu_p2p_defconfig(Dev_SN_Info *oem_info, Dev_Attribut_Struct  *attr)
 {
 	FILE *f;
	long len;
	char *data;
	cJSON *json,*OEM,*DEVICE;
    char fileNmae[64];
	//Dev_SN_Info oem_info;
	//Dev_Attribut_Struct  attr;
	//memset(&oem_info,0,sizeof(Dev_SN_Info));
	//memset(&attr,0,sizeof(attr));

    system("mkdir -p /opt/custom/ulucu");
    system("chmod 777 /opt/custom/ulucu -R");


	//strcpy(oem_info->SN, "goke12345678"); //厂商自定义的设备sn 最少8个字节最长16个字节, 只能是数字和字母, 厂商自己保证每台设备都要有唯一的SN号
	if(load_info_to_mtd_reserve(MTD_ULUCU_P2P, oem_info, sizeof(Dev_SN_Info)) < 0)
    {
		PRINT_ERR("no ulucu device id in mtd2\n");
 resetOem:
        memset(oem_info,0,sizeof(Dev_SN_Info));
        get_macaddr(oem_info->MAC);

		oem_info->SN[0] = 'G';
		oem_info->SN[1] = 'H';// for differenc for hzd ulu _v1.1.0('H')
		oem_info->SN[2] = oem_info->MAC[12];
		oem_info->SN[3] = oem_info->MAC[13];
		oem_info->SN[4] = oem_info->MAC[15];
		oem_info->SN[5] = oem_info->MAC[16];

		char *pSN = &oem_info->SN[6];
		get_rand_str(pSN, 10);
        PRINT_INFO("oem_sn:%s\n",oem_info->SN);
    	//strcpy(oem_info.MAC, runNetworkCfg.lan.mac);//设备的MAC地址，厂商自己保证每台设备都要有唯一的MAC地址
    	oem_info->OEMID = OEM_ID;//厂商和ULUCU商务联系，获得此ID， 因为是汇众达版本，此处必须为100029
    	strcpy(oem_info->OEM_name, OEM_NAME);//厂商和ULUCU商务联系，获得OEM_NAME, 因为是汇众达版本，此处必须为HZ


    	strcpy(oem_info->Model, "VX");//厂商为自己的某一款设备定义的型号，最长64个字节。
    	strcpy(oem_info->Factory, "HZD");//厂商的全名，最长255个字节。
        save_info_to_mtd_reserve(MTD_ULUCU_P2P, oem_info, sizeof(Dev_SN_Info));
    }
    else
    {
        PRINT_INFO("ulu read mtd flash data");
        if(oem_info->OEMID != OEM_ID)
        {
            PRINT_INFO("Check oem info error,reset it");
            goto resetOem;

        }
    }
	attr->block_nums = 200;//缓冲区初始化,单位是8k,200*8k
	attr->channel_num = 1;//设备通道数
	attr->ptz_ctrl = 2;
	attr->dev_type = 3;
	attr->mic_flag = 1;

	//码率一定要连续.2个码率UPLOAD_RATE_4-UPLOAD_RATE_3,UPLOAD_RATE_3-UPLOAD_RATE_2,UPLOAD_RATE_2-UPLOAD_RATE_1;
	//三个码流  UPLOAD_RATE_4-UPLOAD_RATE_2,UPLOAD_RATE_3-UPLOAD_RATE_1;
	//四个码流  UPLOAD_RATE_4-UPLOAD_RATE_1
	attr->max_rate = UPLOAD_RATE_4;
	attr->min_rate = UPLOAD_RATE_1;

	attr->can_rec_voice = 1;					//可以接受音频 0 不支持, 1 支持
	attr->hard_disk = 0;						//是否有硬盘  0 没有, 1 有
	attr->p_rw_path = cfgDir;	//可读写目录路径

	attr->audio_type = 1;//0:acc, 1:g711_a, 2:g711_u, 3:mp3

	attr->audio_chnl = 1;
	attr->audio_smaple_rt = 8000;
	attr->audio_bit_width = 16;
	attr->use_type = 0; //设备使用类型 0：对外销售设备，1：测试设备，2：演示设备。 默认0。
	attr->has_tfcard= 1;//是否有TF卡 0: 没有 1:有

	OEM = cJSON_CreateObject();
	cJSON_AddItemToObject(OEM, "MAC", cJSON_CreateString(oem_info->MAC));
	cJSON_AddItemToObject(OEM, "OEMID", cJSON_CreateNumber(oem_info->OEMID));//厂商和ULUCU商务联系，获得此ID
	cJSON_AddItemToObject(OEM, "SN", cJSON_CreateString(oem_info->SN));//厂商自定义的设备sn 最少8个字节最长16个字节, 只能是数字和字母, 厂商自己保证每台设备都要有唯一的SN号
	cJSON_AddItemToObject(OEM, "OEM_name", cJSON_CreateString(oem_info->OEM_name));//厂商和ULUCU商务联系，获得OEM_NAME
	cJSON_AddItemToObject(OEM, "Model", cJSON_CreateString(oem_info->Model));//厂商为自己的某一款设备定义的型号，最长64个字节。
	cJSON_AddItemToObject(OEM, "Factory", cJSON_CreateString(oem_info->Factory));//厂商的全名，最长255个字节。

	DEVICE = cJSON_CreateObject();
	cJSON_AddItemToObject(DEVICE, "block_nums", cJSON_CreateNumber(attr->block_nums));
	cJSON_AddItemToObject(DEVICE, "channel_num", cJSON_CreateNumber(attr->channel_num));
	cJSON_AddItemToObject(DEVICE, "max_rate", cJSON_CreateNumber(attr->max_rate));
	cJSON_AddItemToObject(DEVICE, "min_rate", cJSON_CreateNumber(attr->min_rate));
	cJSON_AddItemToObject(DEVICE, "ptz_ctrl", cJSON_CreateNumber(attr->ptz_ctrl));
	cJSON_AddItemToObject(DEVICE, "dev_type", cJSON_CreateNumber(attr->dev_type));
	cJSON_AddItemToObject(DEVICE, "mic_flag", cJSON_CreateNumber(attr->mic_flag));
	cJSON_AddItemToObject(DEVICE, "can_rec_voice", cJSON_CreateNumber(attr->can_rec_voice));
	cJSON_AddItemToObject(DEVICE, "hard_disk", cJSON_CreateNumber(attr->hard_disk));
	cJSON_AddItemToObject(DEVICE, "p_rw_path", cJSON_CreateString(attr->p_rw_path));
	cJSON_AddItemToObject(DEVICE, "audio_type", cJSON_CreateNumber(attr->audio_type));
	cJSON_AddItemToObject(DEVICE, "audio_chnl", cJSON_CreateNumber(attr->audio_chnl));
	cJSON_AddItemToObject(DEVICE, "audio_smaple_rt", cJSON_CreateNumber(attr->audio_smaple_rt));
	cJSON_AddItemToObject(DEVICE, "audio_bit_width", cJSON_CreateNumber(attr->audio_bit_width));
	cJSON_AddItemToObject(DEVICE, "use_type", cJSON_CreateNumber(attr->use_type));
	cJSON_AddItemToObject(DEVICE, "has_tfcard", cJSON_CreateNumber(attr->has_tfcard));

	json=cJSON_CreateObject();
	cJSON_AddItemToObject(json, "OEM", OEM);
	cJSON_AddItemToObject(json, "DEVICE", DEVICE);

	data=cJSON_Print( json );
	cJSON_Delete(json);

	len=strlen(data);
    sprintf(fileNmae,"%s/ulucu.cjson",cfgDir);
    PRINT_INFO("default ulu.cjson:%s",fileNmae);
	f=fopen(fileNmae,"wb");
	if(NULL == f)
	{
		PRINT_ERR("wirte ulucu config file!");
		return;
	}

	fwrite(data,1,len,f);
	fclose(f);
	free(data);
	return ;
}

char *ulucu_device_id()
{
    FILE *f = NULL;
    long len;
    char fileNmae[64];
    sprintf(fileNmae,"%s/SN_ulk",cfgDir);

	f=fopen(fileNmae,"rb");
	if(NULL == f)
	{
		PRINT_ERR("no SN_ulk!");
		return NULL;
	}
	fseek(f,0,SEEK_END);
	len=ftell(f);
	fseek(f,0,SEEK_SET);
    memset(sn_ulk_id, 0, sizeof(sn_ulk_id));
	fread(sn_ulk_id,1,len,f);
	fclose(f);
	//netcam_timer_del_task(ulucu_device_id);

	return sn_ulk_id;
 }

static void getdevice_id()
{
	char *ulu_id = ulucu_device_id();
    PRINT_INFO("uld_id:%s",ulu_id);
    if(!get_id_result && netcam_get_update_status() == 0)
	{
		if(ulu_id != NULL && strlen(ulu_id) >= 20)
		{
			add_p2p_info("Ulucu",ulu_id);
			get_id_result=true;
		}
	}
	else
	{
		netcam_timer_del_task(getdevice_id);
	}
}

static void *ulucu_pb_thread(void *param)
{
    int bufinfo = 0;
    while(ulucuRuning)
    {
        if( bufinfo == 200 && his_playback == 1)
        {
            //PRINT_INFO("ulucu_pb_thread-->usleep(40000)\n");
            usleep(40000);
        }
        if (his_playback == 0)
        {
            if(his_playbacking == 1)
            {
                Ulu_SDK_History_Frame_Send(NULL, ULK_END);
                his_playbacking = 0;
                ulu_PlayBackStop();
            }
            usleep(100000);
        }
        else
        {
            his_playbacking = 1;
            bufinfo = playback_send_frame(pUluPbHand);
            if( bufinfo < 0)
            {
                his_playback = 0;
            }
        }
        //PRINT_INFO("-->%d\n", err);
        //usleep(40*1000);
    }
    record_history_stop();
    ulu_PlayBackStop();
    PRINT_INFO("ulucu_pb_thread exit\n");
    return NULL;
}

#ifdef ENABLE_PHOTO_WIFI
#define ULUCU_PHOTO_FILE "/tmp/ulucu_snapshot.jpg"

int ulucu_set_photo_data(char *path)
{
	Picture_info photo_info;
	FILE *fp ;

	PRINT_INFO("===========ulucu_set_photo_start===========\n");

	memset(&photo_info,0,sizeof(Picture_info));
    fp = fopen(path, "rb");
    if (fp  == NULL)
    {
        PRINT_ERR("fopen %s error.\n", path);
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    int len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    char *buffer = (char*)malloc(len);
    if (buffer == NULL)
    {
        PRINT_ERR("ulucu_set_photo_data malloc failed. lenth %d.\n", len);
        fclose(fp);
        return -1;
    }
    memset(buffer, 0, len);

    int ret = fread(buffer, 1, len, fp);
    fclose(fp);
	PRINT_INFO("Delete %s %s.\n", path, delete_path(path)==0?"success":"fail");

	photo_info.pic_data = (uint8 *)buffer;
	photo_info.pic_data_len = ret;
	photo_info.pic_type = JPG;
	PRINT_INFO("ulucu_set_photo_data pic_data_len:%d.\n", photo_info.pic_data_len);
	Ulu_SDK_Push_ScreenShot(&photo_info);
    free(buffer);
	return 0;
}

static void *ulucu_snapshot_thread(void *param)
{
	WIFI_LINK_INFO_t linkInfo;
    if(NULL != netcam_net_wifi_get_devname())
    {
    	while(ulucuRuning)
    	{
            if(netcam_get_update_status()!= 0)
            {
                break; // exit in updating
            }
            if(netcam_net_get_detect("eth0") != 0)
            {
        		netcam_net_wifi_get_connect_info(&linkInfo);

        	   	if( linkInfo.linkStatus != WIFI_CONNECT_OK )
        	   	{
                    if(sta_smartlin_ok == 0 )
                    {
						#if ZBAR
						netcam_wifi_set_callback_ext(netcam_wifi_app_cb); 	
						#else
						netcam_video_snapshot(runVideoCfg.vencStream[1].h264Conf.width, runVideoCfg.vencStream[1].h264Conf.height, ULUCU_PHOTO_FILE, GK_ENC_SNAPSHOT_QUALITY_HIGH);
            			ulucu_set_photo_data(ULUCU_PHOTO_FILE);//ulu二维码解析扫描  
						#endif
					}
        	   	}
            }
            sleep(2);
    	}
    }
    PRINT_INFO("ulucu_snapshot_thread exit\n");
    return NULL;
}
#endif

static void *ulucu_p2p_thread(void *param)
{
	int ret = 0;
	int bufinfo =0;
	GK_NET_FRAME_HEADER frame_header = {0};
	Stream_Event_Struct  frm_event;
	MEDIABUF_HANDLE readerid[3]={0,};
    NETCAM_SmartLinkFuncT sm_func;
    int i;
    int readIframe = 0;
    int curStreamId = -1;
    char audio_alaw[4096];
    int alaw_len = 0;
    char *wifi_name;

	sdk_sys_thread_set_name("ulucu_p2p_thread");

	memset(&oem_info,0,sizeof(Dev_SN_Info));
	memset(&attr,0,sizeof(attr));
	if(ulucu_p2p_config(&oem_info, &attr)!=0)
	{
		ulucu_p2p_defconfig(&oem_info, &attr);
		PRINT_INFO("no ulucu.cjson file,use defconfig\n");
	}
	if(3 == attr.dev_type)//iPC
		attr.channel_num = 1;

    sprintf(sta_ssid_prefix, "%s%s", oem_info.OEM_name, oem_info.SN);
    PRINT_INFO("oem_mac:%s\n",oem_info.MAC);
    PRINT_INFO("oem_id:%d\n",oem_info.OEMID);
    PRINT_INFO("oem_sn:%s\n",oem_info.SN);
    PRINT_INFO("oem_name:%s\n",oem_info.OEM_name);
    PRINT_INFO("oem_Model:%s\n",oem_info.Model);
    PRINT_INFO("oem_fac:%s\n",oem_info.Factory);
	if(Ulu_SDK_Init_All(&oem_info, &attr, NULL) != 0)
	{
        PRINT_ERR("ULU SDK INIT FAILED\n");
    }
    else
    {
        PRINT_INFO("ULU SDK INIT success\n");
    }

    Ulu_SDK_Disable_AutoSyncTime();

    //设置回调
    Ulu_SDK_Set_Interact_CallBack(Deal_CBFun);
	Ulu_SDK_Set_Talk_CallBack(Talk_CBFun);

	CAACDecoder_Init();
	if(	2 == attr.dev_type)//NVR
	{
		int i=1;
		for(i=1; i<=attr.channel_num; i++)
			Ulu_SDK_ChannelStatus_Event(i, ULK_ONLINE);
	}
    // set smartlink callback
    wifi_name = netcam_net_wifi_get_devname();
    if(wifi_name == NULL)
        wifi_name = "ra0";
    Ulu_SDK_Set_WIFI_CallBack_Ext(app_ulu_wifi_cb, 0, wifi_name);
    sm_func.startSmartLink = app_ulucu_smartlink_start;
    sm_func.stopSmartLink = app_ulucu_smartlink_stop;
    sm_func.getSmartLinkInfo = app_ulucu_smartlink_get;

    netcam_net_register_smartlink(&sm_func);

    ulu_pt = event_alarm_open(app_ulucu_motion_event_upload);
    if(ulu_pt == NULL)
    {
        PRINT_ERR("Create motion update event failed");
    }

	netcam_timer_add_task(getdevice_id, 10*NETCAM_TIMER_ONE_SEC, SDK_TRUE, SDK_FALSE);
    //程序通过下面的函数不断的推送视频数据
    curStreamId = -1;
    while(ulucuRuning)
    {
    	if( bufinfo == 200)
		{
			PRINT_INFO("-->usleep(40000)\n");
			usleep(40000);
		}

        if (enable_upload == 0)
		{
        	usleep(200000);
			//删除读者，节约资源
			if(curStreamId != -1 && readerid[curStreamId] != 0)
			{
                mediabuf_del_reader(readerid[curStreamId]);
                readerid[curStreamId] = 0;
                LOG_INFO("stream stoped delete reader: id:%d\n",curStreamId);
                curStreamId = -1;
            }
        	continue;
        }

        if(curStreamId != -1 && curStreamId != stream_id)
        {
            if(readerid[curStreamId] != 0)
            {
                mediabuf_del_reader(readerid[curStreamId]);
                readerid[curStreamId] = 0;
                curStreamId = -1;
                LOG_INFO("stream id change,delete old reader: id:%d,\n",curStreamId);
            }
        }

        readIframe = 0;
        //添加读者
        if(curStreamId == -1)
        {
            curStreamId = stream_id;
            readerid[curStreamId] = mediabuf_add_reader(curStreamId);
            if(readerid[curStreamId] == NULL)
            {
    			PRINT_ERR("add reader failed:%d,stop send stream\n",curStreamId);
                enable_upload = 0;
                curStreamId = -1;
                continue;
            }
            LOG_INFO("new stream,add reader: id:%d\n",curStreamId);

            readIframe = 1;
        }


		//Stream_Event_Struct  frm_event;
        //frm_event.channelnum = 1;//通道号最小为1
        //frm_event.bit_rate = UPLOAD_RATE_3;	//码率
        frm_event.pdata = NULL;
		frm_event.frm_size = 0;
        if (readIframe)
		{
			ret = mediabuf_read_I_frame(readerid[curStreamId],(void*)&frm_event.pdata, (int *)&frm_event.frm_size, &frame_header);
		}
		else
			ret = mediabuf_read_frame(readerid[curStreamId],(void*)&frm_event.pdata, (int *)&frm_event.frm_size, &frame_header);
		if(ret>0)
		{
			if (frame_header.frame_type == GK_NET_FRAME_TYPE_I)
			{
				frm_event.frm_type = CH_I_FRM;		//0=I帧,1=P帧,2=音频帧
			}
			else if (frame_header.frame_type == GK_NET_FRAME_TYPE_P)
			{
				frm_event.frm_type = CH_P_FRM;		//0=I帧,1=P帧,2=音频帧
			}
			else if (frame_header.frame_type == GK_NET_FRAME_TYPE_A)
			{
				frm_event.frm_type = CH_AUDIO_FRM;	//0=I帧,1=P帧,2=音频帧
                if(frm_event.frm_size / 2 > sizeof(audio_alaw)){
                    LOG_ERR("audio len[%d] error\n", frm_event.frm_size);
                    continue;
                }
                alaw_len = audio_alaw_encode(audio_alaw, frm_event.pdata, frm_event.frm_size);
                frm_event.pdata = audio_alaw;
                frm_event.frm_size = alaw_len;
			}
			else
			{
                usleep(200000);
                continue;
			}
			frm_event.frm_ts = Ulu_SDK_GetTickCount();//时间戳.ms为单位
			//frm_event.frm_ts = frame_header.pts;//时间戳.ms为单位
			frm_event.channelnum = channel_index;//通道号最小为1
			frm_event.bit_rate = upload_rate; //码率
			bufinfo = Ulu_SDK_Stream_Event_Report(&frm_event);
		}
        else
        {
            usleep(40*1000);
        }
		//PRINT_INFO("-->%d\n", err);
		//usleep(40*1000);
    }

    for(i = 0; i < netcam_video_get_channel_number(); i++)
    {
        if(readerid[i] != 0)
        {
            mediabuf_del_reader(readerid[i]);
            readerid[i] = 0;
        }
    }


    event_alarm_close(ulu_pt);
    //Ulu_SDK_DeInit();
    return NULL;

}

int ulucu_p2p_start()
{
	PRINT_INFO("ulucu p2p server start...\n");
    ulucuRuning = 1;

	//开启悠络客P2P初始化线程
    pthread_t thread_id,playback_id,snapshot_id;
    pthread_attr_t attr,attr_playback,attr_snapshot;
    pthread_attr_init(&attr);
	pthread_attr_init(&attr_playback);
#ifdef ENABLE_PHOTO_WIFI
	pthread_attr_init(&attr_snapshot);
#endif
    //pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);//detached
    pthread_attr_setdetachstate(&attr_playback, PTHREAD_CREATE_DETACHED);//detached
#ifdef ENABLE_PHOTO_WIFI
    pthread_attr_setdetachstate(&attr_snapshot, PTHREAD_CREATE_DETACHED);//detached
#endif
    if(0 != pthread_create(&thread_id, &attr, ulucu_p2p_thread, NULL))
    {
        pthread_attr_destroy(&attr);
        PRINT_INFO("ulucu p2p server start error\n");
        return -1;
    }
	if(0 != pthread_create(&playback_id, &attr_playback,ulucu_pb_thread, NULL))
    {
        pthread_attr_destroy(&attr_playback);
        PRINT_INFO("ulucu p2p playback start error\n");
        return -1;
    }
#ifdef ENABLE_PHOTO_WIFI
	if(0 != pthread_create(&snapshot_id, &attr_snapshot,ulucu_snapshot_thread, NULL))
    {
        pthread_attr_destroy(&attr_snapshot);
        PRINT_INFO("ulucu p2p snapshot start error\n");
        return -1;
    }
#endif
    pthread_attr_destroy(&attr);
	pthread_attr_destroy(&attr_playback);
#ifdef ENABLE_PHOTO_WIFI
	pthread_attr_destroy(&attr_snapshot);
#endif
    P2PName_add(MTD_ULUCU_P2P);
	return 0;
}

int ulucu_p2p_stop()
{
    ulucuRuning  = 0;
    sleep(1);
	CAACDecoder_UnInit();
	Ulu_SDK_DeInit();
    usleep(200000);
    PRINT_INFO("ulucu p2p server stop...\n");
    return 0;
}

int ulucu_p2p_get_run_status()
{
    return ulucuRuning;
}

#ifdef ENABLE_PHOTO_WIFI

typedef void* iconv_t;

iconv_t iconv_open (const char* tocode, const char* fromcode)
{
	return libiconv_open(tocode,fromcode);
}

size_t iconv (iconv_t cd,  char* * inbuf, size_t *inbytesleft, char* * outbuf, size_t *outbytesleft)
{
	return libiconv ( cd,   inbuf, inbytesleft,  outbuf, outbytesleft);
}

int iconv_close (iconv_t cd)
{
	return libiconv_close ( cd);
}

#endif

