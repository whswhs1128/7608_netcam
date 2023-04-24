#include <string.h>
#include "sdk_network.h"
#include "cfg_video.h"
#include "common.h"
#include "media_fifo.h"
#include "cfg_image.h"
#include "cfg_channel.h"
#include "netcam_api.h"

enum{
//get
	GET_NETWORK_CFG = 1,
	GET_VIDEO_CFG,
    GET_IPC_TIME,
    GET_IMAGE_CFG,
    GET_OSD_CFG,
    GET_COVER_CFG,
    SYS_IPC_REBOOT,
    GET_SYSTEM_TIME,

//set
SET_NETWORK_CFG,
SET_VIDEO_CFG,
SET_IPC_TIME,
SET_IMAGE_CFG,
SET_OSD_CFG,
SET_COVER_CFG,
SET_SYSTEM_TIME,
SET_PTZ_CMD,

};


#define GK_MAX_IP_LENGTH 		16
#define GK_MAX_VIDEO_CHN		4
#define GK_MAX_STREAMNUM 		4
#define MAX_STR_LEN_16 			16
#define MAX_STR_LEN_20 			20
#define MAX_STR_LEN_32 			32
#define MAX_STR_LEN_64 			64
#define MAX_STR_LEN_128 		128
#define MAX_STR_LEN_256 		256

#ifndef _HAVE_TYPE_DWORD
#define _HAVE_TYPE_DWORD
typedef unsigned long       DWORD;
typedef unsigned long       ULONG;
#endif
typedef struct tagDMS_TIME
{
    DWORD    dwYear;
    DWORD    dwMonth;
    DWORD    dwDay;
    DWORD    dwHour;
    DWORD    dwMinute;
    DWORD    dwSecond;        
}DMS_TIME, *LPDMS_TIME;

static int goke_ptz_contrl(int *cmd)
{
//bruce
#if 0
    int step, speed;

    step = 10;
    speed = 3;
    switch(*cmd)
    {
        case DMS_PTZ_CMD_STOP:
            netcam_ptz_stop();
            break;
        case DMS_PTZ_CMD_UP:
            netcam_ptz_up(step, speed);
            break;
        case DMS_PTZ_CMD_DOWN:
            netcam_ptz_down(step, speed);
            break;
        case DMS_PTZ_CMD_LEFT:
            netcam_ptz_left(step, speed);
            break;
        case DMS_PTZ_CMD_RIGHT:
            netcam_ptz_right(step, speed);
            break;
        case DMS_PTZ_CMD_UP_LEFT:
            netcam_ptz_left_up(step, speed);
            break;
        case DMS_PTZ_CMD_UP_RIGHT:
            netcam_ptz_right_up(step, speed);
            break;
        case DMS_PTZ_CMD_DOWN_LEFT:
            netcam_ptz_left_down(step, speed);
            break;
        case DMS_PTZ_CMD_DOWN_RIGHT:
            netcam_ptz_right_down(step, speed);
            break;
        case DMS_PTZ_CMD_START_CRU://开始巡航
            netcam_ptz_hor_ver_cruise(speed);
            break;
        case DMS_PTZ_CMD_STOP_CRU://停止巡航
            netcam_ptz_stop();
            break;
        case DMS_PTZ_CMD_DEL_PRE_CRU:
        case DMS_PTZ_CMD_PRESET:
        default:
            PRINT_INFO("ptc cmd[%x] not used\n", *cmd);

    }
#endif

    return 0;
}
int goke_api_get_ioctrl(int command, void *param)
{
    int ret = 0;

	if(!param){
		PRINT_ERR("param is NULL\n");
		return -1;
	}
	switch(command)
	{
		case GET_NETWORK_CFG:
			{
			ST_SDK_NETWORK_ATTR netAttr;
			
		    memset(&netAttr, 0, sizeof(ST_SDK_NETWORK_ATTR));
			ret = netcam_net_get_detect("eth0");
			if(ret != 0){
				if(netcam_net_wifi_isOn() == 1){	
				    strcpy(netAttr.name,netcam_net_wifi_get_devname());
				    ret = netcam_net_get(&netAttr);
				    if (ret != 0){
				        PRINT_ERR("Faile to get network info.");
				        return -1;
				    }
				}
			}else{
				strncpy(netAttr.name, "eth0", SDK_ETHERSTR_LEN-1);
			    ret = netcam_net_get(&netAttr);
			    if (ret != 0){
			        PRINT_ERR("Faile to get network info.");
			        return -1;
			    }
			}
			memcpy(param, &netAttr, sizeof(netAttr));
            PRINT_ERR("get mac:%s\n", netAttr.mac);
			break;
			}
		case GET_VIDEO_CFG:
			{
			ret = netcam_video_get_all_channel_par(param);
			break;
			}
        case GET_IPC_TIME:

            break;
        case GET_IMAGE_CFG:
            {
            ret = netcam_image_get(param);
            break;
            }
        case GET_OSD_CFG:
            {
            // test add by hybin 
            GK_NET_CHANNEL_INFO *channel_param = param;
            int channel = channel_param->id;
            //int channel = 0;
            ret = netcam_osd_get_info(channel,param);
            break;
            }
        case GET_COVER_CFG:
            {
                int i;
                GK_NET_SHELTER_RECT cover[4];
                for(i = 0; i < 4; i ++)
                {
                    if(netcam_pm_get_cover(0, i, &cover[i]) < 0)
                    {
                        ret = -1;
                        break;
                    }
                }
                memcpy(param, cover, sizeof(GK_NET_SHELTER_RECT) * 4);
            }
            break;
        case GET_SYSTEM_TIME:
            {
                DMS_TIME systime;
                DMS_TIME *get_systime = (DMS_TIME *)&systime;
                long ts = time(NULL);
                struct tm tt = {0};
                struct tm *t = localtime_r(&ts, &tt);
                get_systime->dwYear = t->tm_year + 1900;
                get_systime->dwMonth = t->tm_mon + 1;
                get_systime->dwDay = t->tm_mday;
                get_systime->dwHour = t->tm_hour;
                get_systime->dwMinute = t->tm_min;
                get_systime->dwSecond = t->tm_sec;
                memcpy(param, &systime, sizeof(DMS_TIME));
                break;
            }
	}
	return ret;
}
int goke_api_set_ioctrl(int command, void *param)
{
    int ret = 0;

	if(!param){
		PRINT_ERR("param is NULL\n");
		return -1;
	}
	switch(command)
	{
		case SET_NETWORK_CFG:
            {
            ST_SDK_NETWORK_ATTR *attr = (ST_SDK_NETWORK_ATTR*)param;
			ret = netcam_net_get_detect("eth0");
			if(ret != 0){
				if(netcam_net_wifi_isOn() == 1){	
				    strcpy(attr->name,netcam_net_wifi_get_devname());
                    ret = netcam_net_set(attr);
				    if (ret != 0){
				        PRINT_ERR("Faile to get network info.");
				        return -1;
				    }
				}
			}else{
				strncpy(attr->name, "eth0", SDK_ETHERSTR_LEN-1);
                ret = netcam_net_set(attr);
			    if (ret != 0){
			        PRINT_ERR("Faile to get network info.");
			        return -1;
			    }
			}
			break;
			}
		case SET_VIDEO_CFG:
            {
                GK_NET_VIDEO_CFG *video_cfg = param;
                //bruce
                #if 0
                GK_ENC_STREAM_H264_ATTR *main_stream = &(video_cfg->vencStream[0].h264conf);
                ret = netcam_video_set(0, 0, (PS_GK_ENC_STREAM_H264_ATTR)main_stream);
                GK_ENC_STREAM_H264_ATTR *second_stream = &(video_cfg->vencStream[1].h264conf);
                ret = netcam_video_set(0, 1, (PS_GK_ENC_STREAM_H264_ATTR)second_stream);
                #endif
                break;
            }
        case SET_OSD_CFG:
            {
                int chn = ((GK_NET_CHANNEL_INFO*)param)->id;
                ret = netcam_osd_set_info(chn, param);
                break;
            }
        case SET_COVER_CFG:
            {
                int i;
                GK_NET_SHELTER_RECT cover;
                for(i = 0; i < 4; i++)
                {
                    memcpy(&cover, param + sizeof(cover) * i, sizeof(cover));
                    if(netcam_pm_set_cover(0, i, cover) < 0)
                    {
                        ret = -1;
                        break;
                    }
                    
                }
                break;
            }
        case SET_IPC_TIME:
            break;
        case SET_IMAGE_CFG:
            {
            GK_NET_IMAGE_CFG image;
            memcpy(&image, param, sizeof(GK_NET_IMAGE_CFG));
            PRINT_INFO("SET_IMAGE_CFG brightness:%d, contrast:%d, saturation:%d, hue:%d\n", 
                    image.brightness, image.contrast, image.saturation, image.hue);
            ret = netcam_image_set(image);
            break;
            }
        case SYS_IPC_REBOOT:
            {
                int delay = 0;//(int)*param;
                //netcam_sys_reboot(delay);
				netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT
            }
            break;
        case SET_SYSTEM_TIME:
            {
            char command[128] = {0};
            DMS_TIME *systime = (DMS_TIME*)param;
            snprintf(command, sizeof(command), "date -s %lu.%lu.%lu-%lu:%lu:%lu", 
                    systime->dwYear, systime->dwMonth, systime->dwDay, systime->dwHour, systime->dwMinute, systime->dwSecond);
            new_system_call(command);
            new_system_call("hwclock -w");
            break;
            }
        case SET_PTZ_CMD:
            goke_ptz_contrl(param);
            break;
	}
	return ret;
}

