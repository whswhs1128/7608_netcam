#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>
#include <time.h>


#include "sdk_network.h"
#include "cfg_video.h"
#include "common.h"
#include "media_fifo.h"
#include "cfg_image.h"
#include "cfg_channel.h"
#include "netcam_api.h"

#include "dahua_session.h"
#include "util_sock.h"
#include "utility_api.h"

#include "g711.h"

#include "eventalarm.h"

static void *g_dahua_alarm_handle = 0;


static pthread_mutex_t g_dahua_stream_mutex = PTHREAD_MUTEX_INITIALIZER;


#define DEBUG 1
#define ADD_MD_INFO	0
#define DAHUA_PRINT_CJSON_MSG 1 



#define GK_MAX_IP_LENGTH 		16
#define GK_MAX_VIDEO_CHN		4
#define GK_MAX_STREAMNUM 		4
#define MAX_STR_LEN_16 			16
#define MAX_STR_LEN_20 			20
#define MAX_STR_LEN_32 			32
#define MAX_STR_LEN_64 			64
#define MAX_STR_LEN_128 		128
#define MAX_STR_LEN_256 		256


#if DEBUG
#else
#define PRINT_INFO(fmt...) {}
#endif



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
    GET_IMAGE_LAST_CFG,

    //set
    SET_NETWORK_CFG,
    SET_VIDEO_CFG,
    SET_IPC_TIME,
    SET_IMAGE_CFG,
    SET_OSD_CFG,
    SET_COVER_CFG,
    SET_SYSTEM_TIME,
    SET_PTZ_CMD,
    SAVE_IMAGE_CFG,
    SET_SAVE_IMAGE_CFG,
    RESTORE_IMAGE_CFG
};



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

UtSession g_dahua_session;
static int g_dahua_thread_run = 0;

#define MULTI_CONNECT 1

#if !MULTI_CONNECT
int main_sock = -1;
int sub_sock = -1;
int send_main = 0;
int send_sub = 0;
#endif
////

static DahuaColorProfile stDahuaColorProfile[3];
static int profile_index = 0;
static GK_NET_IMAGE_CFG tmp_image_cfg;

int DahuaCreateThread(ThreadEntryPtrType entry, void *para, pthread_t *pid)
{
    pthread_t ThreadId;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);//°ó¶¨
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);//·ÖÀë
    if(pthread_create(&ThreadId, &attr, entry, para) == 0)//´´½¨Ïß³Ì
    {
        pthread_attr_destroy(&attr);
		if(pid)
			*pid = ThreadId;

		return 0;
    }

    pthread_attr_destroy(&attr);
    return -1;
}



static int goke_ptz_contrl(int *cmd)
{
//bruce
#if 1
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
        case DMS_PTZ_CMD_START_CRU://å¼€å§‹å·¡èˆª
            netcam_ptz_hor_ver_cruise(speed);
            break;
        case DMS_PTZ_CMD_STOP_CRU://åœæ­¢å·¡èˆª
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
//            PRINT_ERR("get mac:%s\n", netAttr.mac);
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
        case GET_IMAGE_LAST_CFG:
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
            break;
        }
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
            PRINT_ERR("SET_VIDEO_CFG\n");
            #if 1
            GK_ENC_STREAM_H264_ATTR *main_stream = &(video_cfg->vencStream[0].h264Conf);
            ret = netcam_video_set(0, 0, (PS_GK_ENC_STREAM_H264_ATTR)main_stream);
            GK_ENC_STREAM_H264_ATTR *second_stream = &(video_cfg->vencStream[1].h264Conf);
            ret = netcam_video_set(0, 1, (PS_GK_ENC_STREAM_H264_ATTR)second_stream);
            #endif
            break;
        }
        case SET_OSD_CFG:
        {
            int chn = ((GK_NET_CHANNEL_INFO*)param)->id;
            //bruce
            PRINT_ERR("set osd\n");
            //ret = netcam_osd_set_info(chn, param);
            netcam_osd_set_id(chn, param);
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
            ret = netcam_image_set2(image, 0);
            break;
        }
        case SET_SAVE_IMAGE_CFG:
        {
            GK_NET_IMAGE_CFG image;
            memcpy(&image, param, sizeof(GK_NET_IMAGE_CFG));
            ret = netcam_image_set2(image, 1);
            break;
        }
        case RESTORE_IMAGE_CFG:
        {
            GK_NET_IMAGE_CFG image;
            memcpy(&image, param, sizeof(GK_NET_IMAGE_CFG));
            ret = netcam_image_set2(image, 2);
            break;
        }
        case SYS_IPC_REBOOT:
        {
            int delay = 0;//(int)*param;
            //netcam_sys_reboot(delay);
			netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT
            break;
        }
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




static int string2hex(unsigned char s[],unsigned char bits[])
{
    int i,n = 0;
    for(i = 0; s[i]; i += 2) {
        //¸ßËÄÎ»
        if(s[i] >= 'a' && s[i] <= 'f'){
            bits[n] = s[i] - 'a' + 10;
        }else{
            bits[n] = s[i] - '0';
        }
        //printf("¸ßËÄÎ»s:%x, bit:%x\n", s[i], bits[n]);

        //µÍËÄÎ»
        if(s[i + 1] >= 'a' && s[i + 1] <= 'f'){
            bits[n] = (bits[n] << 4) | (s[i + 1] - 'a' + 10);
        }else {
            bits[n] = (bits[n] << 4) | (s[i + 1] - '0');
        }
        //printf("µÍËÄÎ»s:%x, bit:%x\n", s[i + 1], bits[n]);
        ++n;
    }
    return n;
}



/*´ó»ªÐ­ÒéµÄ·Ö±çÂÊ×ªÎªgokeµÄ·Ö±æÂÊ
 */
static int imagesize_dahua2goke(CAPTURE_SIZE_T dahua_image_size, GK_ENC_STREAM_H264_ATTR *h264Conf)
{
    switch(dahua_image_size)
    {
        case CAPTURE_SIZE_1080p:
            h264Conf->width = 1920;
            h264Conf->height = 1080;
            break;
        case CAPTURE_SIZE_1_3M:
            h264Conf->width = 1080;
            h264Conf->height = 960;
            break;
        case CAPTURE_SIZE_720p:
            h264Conf->width = 1280;
            h264Conf->height = 720;
            break;
        case CAPTURE_SIZE_SVGA:
            break;
        case CAPTURE_SIZE_D1:
            h264Conf->width = 704;
            h264Conf->height = 576;
            break;
        case CAPTURE_SIZE_VGA:
            h264Conf->width = 640;
            h264Conf->height = 480;
            break;
        case CAPTURE_SIZE_QVGA:
            h264Conf->width = 320;
            h264Conf->height = 240;
            break;
        case CAPTURE_SIZE_CIF:
            h264Conf->width = 352;
            h264Conf->height = 288;
            break;
        case CAPTURE_SIZE_QCIF:
            //break;
        case CAPTURE_SIZE_HD1:
            //break;
        case CAPTURE_SIZE_UXGA:
            //break;
        case CAPTURE_SIZE_QQVGA:
            //break;
        case CAPTURE_SIZE_XVGA:
            //break;
        default:
            h264Conf->width = 352;
            h264Conf->height = 288;
            break;
    }
    return 0;
}

//set osd ¿ª¹Ø¼°×ø±ê,ÑÕÉ«
static int dahua_set_osd_attr(CONFIG_CAPTURE_OLD * str_captue)
{
    PRINT_INFO("set osd channle name, enable:%d, left:%d, right:%d, bottom:%d, top:%d\n", str_captue->ChannelTitle.TitleEnable, 
            str_captue->ChannelTitle.TitleLeft, str_captue->ChannelTitle.TitleRight, 
            str_captue->ChannelTitle.TitleBottom, str_captue->ChannelTitle.TitleTop);
    PRINT_INFO("set osd time, enable:%d, left:%d, right:%d, bottom:%d, top:%d\n", str_captue->TimeTitle.TitleEnable, 
            str_captue->TimeTitle.TitleLeft, str_captue->TimeTitle.TitleRight, 
            str_captue->TimeTitle.TitleBottom, str_captue->TimeTitle.TitleTop);
    PRINT_INFO("str_captue->CoverEnable:%d\n", str_captue->CoverEnable);
    PRINT_INFO("set cover left:%d, right:%d, bottom:%d, top:%d\n", str_captue->Cover.left, str_captue->Cover.right, 
            str_captue->Cover.bottom, str_captue->Cover.top);

    //set osd ¿ª¹Ø¼°×ø±ê,ÑÕÉ«
    GK_NET_CHANNEL_INFO channel_info;
    channel_info.id = 0;
    if(0 == goke_api_get_ioctrl(GET_OSD_CFG, &channel_info))
    {
        channel_info.osdChannelName.enable = str_captue->ChannelTitle.TitleEnable;
        channel_info.osdChannelName.x = (float)str_captue->ChannelTitle.TitleLeft / 8192;
        channel_info.osdChannelName.y = (float)str_captue->ChannelTitle.TitleBottom / 8192;
        channel_info.osdDatetime.enable = str_captue->TimeTitle.TitleEnable;
        channel_info.osdDatetime.x = (float)str_captue->TimeTitle.TitleLeft / 8192;
        channel_info.osdDatetime.y = (float)str_captue->TimeTitle.TitleBottom / 8192;
        PRINT_INFO("set channel info id:%d, chnx:%f, chny:%f, timex:%f, timey:%f\n", channel_info.id, channel_info.osdChannelName.x,
                channel_info.osdChannelName.y, channel_info.osdDatetime.x, channel_info.osdDatetime.y);
        goke_api_set_ioctrl(SET_OSD_CFG, &channel_info);
    }
    //set ÊÓÆµÕÚµ² ¿ª¹Ø¼°×ø±ê,ÑÕÉ« is cmd 34

    return 0;
}
static int dahua_set_enc_attr(CONFIG_CAPTURE_OLD * str_captue)
{
//bruce
#if 1
    PRINT_ERR("dahua_set_enc_attr\n");
    GK_NET_VIDEO_CFG channel_pic_info;
    //»ñÈ¡Éè±¸µÄÒôÊÓÆµÍ¨µÀÑ¹Ëõ²ÎÊý
    if(0 != goke_api_get_ioctrl(GET_VIDEO_CFG,&channel_pic_info))
    {
        PRINT_ERR("DH_NET_GET_SUPPORT_STREAM_FMT fail\n");
        return -1;
    }  
    GK_ENC_STREAM_H264_ATTR *main_h264Conf = &channel_pic_info.vencStream[0].h264Conf;
    imagesize_dahua2goke(str_captue->MainOption[0].ImageSize, main_h264Conf);
    if(CAPTURE_BRC_CBR == str_captue->MainOption[0].BitRateControl )// ¹Ì¶¨ÂëÁ÷
    {
        main_h264Conf->rc_mode = 0;
    }else /*if(CAPTURE_BRC_VBR == str_captue->MainOption[0].BitRateControl) //¿É±äÂëÁ÷*/
    {
        main_h264Conf->rc_mode = 1;
    }  
    switch(str_captue->MainOption[0].ImgQlty)/*!< ÂëÁ÷µÄ»­ÖÊ µµ´Î1-6 6×îºÃ */
    {
        case 6:
            main_h264Conf->quality = 3;
            break;
        case 5:
            main_h264Conf->quality = 3;
            break;
        case 4:
            main_h264Conf->quality = 2;
            break;
        case 3:
            main_h264Conf->quality = 2;
            break;                
        case 2:
            main_h264Conf->quality = 2;
            break;
        case 1:
            main_h264Conf->quality = 1;
            break;
        default:
            main_h264Conf->quality = 2;
            break;
    }
    main_h264Conf->fps= str_captue->MainOption[0].Frames;    /*!< Ö¡ÂÊ¡¡µµ´ÎNÖÆ1-6,PÖÆ1-5 */
    main_h264Conf->bps = str_captue->MainOption[0].usBitRate * 8;    //ÂëÂÊ µ¥Î»ÎªK
    PRINT_INFO("set main fps:%d, bps:%d\n", main_h264Conf->fps, main_h264Conf->bps);

    //ÉèÖÃ´ÎÂëÁ÷²ÎÊý
    GK_ENC_STREAM_H264_ATTR *second_h264Conf = &channel_pic_info.vencStream[1].h264Conf;
    imagesize_dahua2goke(str_captue->AssiOption[0].ImageSize, second_h264Conf);
    if(CAPTURE_BRC_CBR == str_captue->AssiOption[0].BitRateControl )// ¹Ì¶¨ÂëÁ÷
    {
        second_h264Conf->rc_mode = 0;
    }
    if(CAPTURE_BRC_VBR == str_captue->AssiOption[0].BitRateControl) //¿É±äÂëÁ÷
    {
        second_h264Conf->rc_mode = 1;
    }
    switch(str_captue->AssiOption[0].ImgQlty)/*!< ÂëÁ÷µÄ»­ÖÊ µµ´Î1-6 6×îºÃ */
    {
        case 6:
            second_h264Conf->quality = 3;
            break;
        case 5:
            second_h264Conf->quality = 3;
            break;
        case 4:
            second_h264Conf->quality = 2;
            break;
        case 3:
            second_h264Conf->quality = 2;
            break;                
        case 2:
            second_h264Conf->quality = 2;
            break;
        case 1:
            second_h264Conf->quality = 1;
            break;
        default:
            second_h264Conf->quality = 2;
            break;
    }
    second_h264Conf->fps= str_captue->AssiOption[0].Frames;;    /*!< Ö¡ÂÊ¡¡µµ´ÎNÖÆ1-6,PÖÆ1-5 */
    second_h264Conf->bps = str_captue->AssiOption[0].usBitRate * 8;    //ÂëÂÊ µ¥Î»ÎªK
    PRINT_INFO("set second fps:%d, bps:%d\n", second_h264Conf->fps, second_h264Conf->bps);
    goke_api_set_ioctrl(SET_VIDEO_CFG, &channel_pic_info);

#endif
    return 0;
}

static int dahua_get_md_attr(char *send_msg)
{
    CONFIG_DETECT md_detect;
    int x, y;

    memset(&md_detect, 0, sizeof(CONFIG_DETECT));
    md_detect.MotionRecordMask |= 1<<0;
    md_detect.MotionAlarmMask  |= 1<<0;
    md_detect.MotionTimeDelay = 10;
    for(y = 0; y < 18; y ++)
    {
        for(x = 0; x < 22; x ++)
            md_detect.MotionRegion[y] |= 1 <<x;
    }
    md_detect.BlindAlarmMask |= 1 << 0;
    md_detect.LossAlarmMask  |= 1 << 0;
    md_detect.MotionLever = 2;
    md_detect.BlindTipEn = 1;
    md_detect.LossTipEn = 1;
    md_detect.Mailer |= 1 << 0;
    md_detect.SendAlarmMsg = 1;

    memcpy(send_msg + DVRIP_HEAD_T_SIZE, &md_detect, sizeof(md_detect));  
    return sizeof(md_detect);
}

static int dahua_get_md_worksheet(char *send_msg)
{
    CONFIG_WORKSHEET md_worksheet;
    memset(&md_worksheet, 0, sizeof(md_worksheet));
    md_worksheet.tsSchedule[0][0].enable |= REC_SYS;

    memcpy(send_msg + DVRIP_HEAD_T_SIZE, &md_worksheet, sizeof(CONFIG_WORKSHEET));  
    return sizeof(CONFIG_WORKSHEET);
}
//ÒÆ¶¯Õì²âÐÅÏ¢ --------end


/*************************************************************
 * º¯Êý½éÉÜ£ºsocketÖ÷¶¯Á¬½Ó
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_SockConnect(int sock, struct sockaddr_in *server, int TimeOut)
{
    int ret;
    struct timeval timeout;
    fd_set set_fd;


    //ÉèÖÃ·Ç×èÈû·½Ê½Á¬½Ó
    //ret = fcntl(sock, F_SETFL, O_NONBLOCK);
    //if(-1 == ret)
    //	return -1;

    //Á¬½Ó
    if((ret = connect(sock, (struct sockaddr *)server, sizeof(*server))) != 0)
    {

        PRINT_ERR("Connect fail1\n");
        return -1;
    }

    //select Ä£ÐÍ£¬¼´ÉèÖÃ³¬Ê±
    FD_ZERO(&set_fd);
    FD_SET(sock, &set_fd);
    timeout.tv_sec = TimeOut; //Á¬½Ó³¬Ê±15Ãë
    timeout.tv_usec = 0;

    ret = select(sock+1, 0, &set_fd, 0, &timeout);
    if(ret <= 0)
    {
        PRINT_ERR("Connect fail\n");
        return -1;
    }

    //Ò»°ã·ÇËø¶¨Ä£Ê½Ì×½Ó±È½ÏÄÑ¿ØÖÆ£¬¿ÉÒÔ¸ù¾ÝÊµ¼ÊÇé¿ö¿¼ÂÇ ÔÙÉè»Ø×èÈûÄ£Ê½
    //ret = fcntl(sock, F_SETFL, O_NONBLOCK); 

    return ret;
}

/*************************************************************
 * º¯Êý½éÉÜ£º´Ósocket·¢ËÍÊý¾Ý
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_SockSend(DAHUA_SESSION_CTRL* sock, char *szbuf, int len, int timeout)
{
    int ret = -1;
    fd_set send_set, excpt_set;
    struct timeval tv;
    int send_total = 0;

    if(sock->fd < 0)
        return -1;
    
    pthread_mutex_lock(&sock->send_mutex);
    while(send_total < len)
    {
        timeout = 3;
        tv.tv_sec = timeout;
        tv.tv_usec = 0;

        FD_ZERO(&send_set);
        FD_ZERO(&excpt_set);
        FD_SET(sock->fd, &send_set);
        FD_SET(sock->fd, &excpt_set);

        ret = select(sock->fd + 1, NULL, &send_set, &excpt_set, &tv);
        if (ret < 0) 
        {
            PRINT_ERR("select error SockSend Failed ret:%d, sock:%d, len:%d errno:%s\n", ret, sock->fd, len, strerror(errno));
            goto error;
        }
        if(0 == ret)
        {
            PRINT_ERR("select timeout SockSend Failed ret:%d, sock:%d, len:%d errno:%s\n", ret, sock->fd, len, strerror(errno));
            goto error;
        }
        if(FD_ISSET(sock->fd, &excpt_set))
            goto error;   
        if(FD_ISSET(sock->fd, &send_set))
        {
            ret = send(sock->fd, szbuf + send_total, len - send_total, 0);
            if(ret <= 0)
            {
                PRINT_ERR("SockSend Failed ret:%d, sock:%d, len:%d errno:%s\n", ret, sock->fd, len, strerror(errno));
                goto error;
            }
        }
        if(ret < len - send_total)
        {            
            PRINT_ERR("SockSend  data short short short short:%d\n", ret);
            goto error;
        }

        send_total += ret;
    }    
    pthread_mutex_unlock(&sock->send_mutex);
    //PRINT_INFO("Leave ==========> SockSend sock:%d, send_total:%d\n",sock,  send_total);     
    return send_total;
    
error:
    pthread_mutex_unlock(&sock->send_mutex);
    return -1;
}

int Dahua_SockSend(int fd, char *szbuf, int len, int timeout)
{
    int ret = -1;
    fd_set send_set, excpt_set;
    struct timeval tv;
    int send_total = 0;

    if(fd < 0)
        return -1;
    
    //pthread_mutex_lock(&sock->send_mutex);
    while(send_total < len)
    {
        timeout = 3;
        tv.tv_sec = timeout;
        tv.tv_usec = 0;

        FD_ZERO(&send_set);
        FD_ZERO(&excpt_set);
        FD_SET(fd, &send_set);
        FD_SET(fd, &excpt_set);

        ret = select(fd + 1, NULL, &send_set, &excpt_set, &tv);
        if (ret < 0) 
        {
            PRINT_ERR("select error SockSend Failed ret:%d, sock:%d, len:%d errno:%s\n", ret, fd, len, strerror(errno));
            goto error;
        }
        if(0 == ret)
        {
            PRINT_ERR("select timeout SockSend Failed ret:%d, sock:%d, len:%d errno:%s\n", ret, fd, len, strerror(errno));
            goto error;
        }
        if(FD_ISSET(fd, &excpt_set))
            goto error;   
        if(FD_ISSET(fd, &send_set))
        {
            ret = send(fd, szbuf + send_total, len - send_total, 0);
            if(ret <= 0)
            {
                PRINT_ERR("SockSend Failed ret:%d, sock:%d, len:%d errno:%s\n", ret, fd, len, strerror(errno));
                goto error;
            }
        }
        if(ret < len - send_total)
        {            
            PRINT_ERR("SockSend  data short short short short:%d\n", ret);
            goto error;
        }

        send_total += ret;
    }    
    //pthread_mutex_unlock(&sock->send_mutex);
    //PRINT_INFO("Leave ==========> SockSend sock:%d, send_total:%d\n",sock,  send_total);     
    return send_total;
    
error:
    //pthread_mutex_unlock(&sock->send_mutex);
    return -1;
}


#if 0

/*************************************************************
 * º¯Êý½éÉÜ£ºÃ¿¸ô5ms·¢ËÍ3kÊý¾Ý
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_Send_Second_Unit(DAHUA_SESSION_CTRL* sock, char *buf, unsigned long len, int nOnePacketSize)
{
    int ret = 0;
    unsigned long nOffSet = 0;
    unsigned long buf_len = len;
    int send_len = 0;

    DVRIP_HEAD_T t_msg_head;
    ZERO_DVRIP_HEAD_T(&t_msg_head)
        t_msg_head.dvrip.cmd = ACK_MEDIA_REQUEST;  
    t_msg_head.c[9] = 0; //Êý¾ÝÐòºÅ£¬ÀÛ¼Ó£¨Ã¿¿Í»§¶Ëµ¥¶ÀÀÛ¼Ó£©
    t_msg_head.c[8] = 0;  //Í¨µÀºÅ£¬ÈçÎªÔ¤ÀÀÊý¾Ý£¬ÌîÍ¨µÀÊýÁ¿£¬Èç16Â·»úÆ÷£¬Ìî16

    /* ·µ»ØÂë 0:Õý³£ 1:ÎÞÈ¨ÏÞ 2:²»Ö§³Ö´Ë²Ù×÷ 3:×ÊÔ´²»×ã 4:ÎÞ·¨»ñµÃËùÇëÇóÐòÁÐºÅµÄÊý¾Ý
10:Êý¾Ý½áÊø£¨Õë¶ÔÍ¼Æ¬¡¢I Ö¡µÈÓÐÏÞÊý¾Ý£©*/
    t_msg_head.c[16] = 0; 
    t_msg_head.c[24] = 5; //¸±ÂëÁ÷
    t_msg_head.dvrip.dvrip_extlen = nOnePacketSize;    

    //PRINT_INFO("ZhiNuo_FlvSendUnit len:%d\n", buf_len);
    while(buf_len > nOnePacketSize)
    {
        ret = ZhiNuo_SockSend(sock, (char *)&t_msg_head, sizeof(DVRIP_HEAD_T), SEND_TIMEOUT);
        if(ret != sizeof(DVRIP_HEAD_T))
        {
            PRINT_ERR("ZhiNuo_SockSend fail0:%d\n", ret);
            return -1;
        }

        ret = ZhiNuo_SockSend(sock, buf + nOffSet, nOnePacketSize, SEND_TIMEOUT);
        if(ret != nOnePacketSize)
        {
            PRINT_ERR("ZhiNuo_SockSend fail0:%d\n", ret);
            return -1;
        }
        nOffSet += nOnePacketSize;
        buf_len -= nOnePacketSize;
        send_len += ret;
    }  

    if(buf_len > 0)
    {   
        t_msg_head.dvrip.dvrip_extlen = buf_len;
        ret = ZhiNuo_SockSend(sock, (char *)&t_msg_head, sizeof(DVRIP_HEAD_T), SEND_TIMEOUT);
        if(ret != sizeof(DVRIP_HEAD_T))
        {
            PRINT_ERR("ZhiNuo_SockSend fail0:%d,%ld\n", ret, buf_len);
            return -1;
        }

        ret = ZhiNuo_SockSend(sock, buf + nOffSet, buf_len, SEND_TIMEOUT);
        if(ret != buf_len)
        {
            PRINT_ERR("ZhiNuo_SockSend fail1:%d\n", ret);
            return -1;
        }
        nOffSet += buf_len;
        send_len += ret;        
    }
    return send_len;

}

/*************************************************************
 * º¯Êý½éÉÜ£º¸øÒôÊÓÆµ´ò°ü
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int DahuaPackSubStream(char* send_buff, const char * data, DWORD data_len, char frame_type, 
        char ch_width, char ch_high, int fps)
{
    int ret = 0;
    unsigned long unl_sum = 0;
    int n_offset = 0;

    ZhiNuo_Media_Frame_Head *t_media_data_head = (ZhiNuo_Media_Frame_Head *)(send_buff);
    //PRINT_INFO("t_media_data_head:%d, t_expand_audio_format:%d, t_expand_picture_size:%d, t_expand_playback_type:%d, t_frame_tail:%d\n",
    //   sizeof(ZhiNuo_Media_Frame_Head), sizeof(ZhiNuo_Expand_Audio_Format), 
    //  sizeof(ZhiNuo_Expand_Picture_Size), sizeof(ZhiNuo_Expand_PlayBack_Type),
    //  sizeof(ZhiNuo_Media_Frame_Tail));

    //Ö¡Í·Ìî³ä
    t_media_data_head->sz_tag[0] = 'D';
    t_media_data_head->sz_tag[1] = 'H';
    t_media_data_head->sz_tag[2] = 'A';
    t_media_data_head->sz_tag[3] = 'V';
    t_media_data_head->ch_media_type = frame_type;
    t_media_data_head->ch_child_type = 0; //Ã»ÓÐ×ÓÀàÐÍ
    t_media_data_head->ch_channel_num = 0;  //Í¨µÀºÅÎª  
    t_media_data_head->ch_child_sequence = 0;
    if(frame_type == AUDIO_TYPE)
    {
        ZhiNuo_Expand_Audio_Format *t_expand_audio_format = (ZhiNuo_Expand_Audio_Format *)
            (send_buff + sizeof(ZhiNuo_Media_Frame_Head));

        //ÒôÆµÖ¡Í·µÄÌØÊâ²¿·ÖÌî³ä
        t_media_data_head->n_frame_sequence = g_audio_sequence_second_stream;
        t_media_data_head->s_timestamp = g_audio_timestample_second_stream;
        t_media_data_head->n_frame_len = sizeof(ZhiNuo_Media_Frame_Head) + sizeof(ZhiNuo_Expand_Audio_Format) 
            + data_len + sizeof(ZhiNuo_Media_Frame_Tail);
        t_media_data_head->ch_expand_len = sizeof(ZhiNuo_Expand_Audio_Format);

        //ÒôÆµ¸ñÊ½À©Õ¹Ö¡Í·Ìî³ä
        t_expand_audio_format->ch_cmd = EXPAND_AUDIO_FORMAT;
        t_expand_audio_format->ch_audio_channel = (char)SINGLE_CHANNEL;
        t_expand_audio_format->ch_audio_type =   (char)G711A;
        t_expand_audio_format->ch_sample_frequency = (char)SAMPLE_FREQ_8000;
    }
    else if(frame_type == FRAME_TYPE_P)
    {
        //ÊÓÆµÖ¡Í·µÄÌØÊâ²¿·ÖÌî³ä
        t_media_data_head->n_frame_sequence = g_video_sequence_second_stream;
        t_media_data_head->s_timestamp = g_video_timestample_second_stream;  

        //PÖ¡Ö¡Í·µÄÌØÊâ²¿·ÖÌî³ä
        t_media_data_head->n_frame_len = sizeof(ZhiNuo_Media_Frame_Head) + data_len 
            + sizeof(ZhiNuo_Media_Frame_Tail);  
        t_media_data_head->ch_expand_len = 0; //Ã»ÓÐÀ©Õ¹Ö¡Í·
    }
    else
    {
#if 1
        ZhiNuo_Expand_Picture_Size *t_expand_picture_size = (ZhiNuo_Expand_Picture_Size *)
            (send_buff + sizeof(ZhiNuo_Media_Frame_Head));
        ZhiNuo_Expand_PlayBack_Type *t_expand_playback_type = (ZhiNuo_Expand_PlayBack_Type *)
            (send_buff + sizeof(ZhiNuo_Media_Frame_Head) + sizeof(ZhiNuo_Expand_Picture_Size));

        //ÊÓÆµÖ¡Í·µÄÌØÊâ²¿·ÖÌî³ä
        t_media_data_head->n_frame_sequence = g_video_sequence_second_stream;
        t_media_data_head->s_timestamp = g_video_timestample_second_stream;              

        //IÖ¡Ö¡Í·µÄÌØÊâ²¿·ÖÌî³ä
        t_media_data_head->n_frame_len = sizeof(ZhiNuo_Media_Frame_Head) + sizeof(ZhiNuo_Expand_Picture_Size)
            + sizeof(ZhiNuo_Expand_PlayBack_Type) + data_len + sizeof(ZhiNuo_Media_Frame_Tail); 
        t_media_data_head->ch_expand_len = sizeof(ZhiNuo_Expand_Picture_Size) + sizeof(ZhiNuo_Expand_PlayBack_Type);

        //Í¼Ïñ³ß´çÀ©Õ¹Ö¡Í·Ìî³ä
        t_expand_picture_size->ch_cmd = EXPAND_PICTURE_SIZE;
        t_expand_picture_size->ch_coding_type = ONE_FRAME;
        t_expand_picture_size->ch_width = ch_width;
        t_expand_picture_size->ch_high = ch_high;

        //»Ø·ÅÀàÐÍÀ©Õ¹Ö¡Í·Ìî³ä
        t_expand_playback_type->ch_cmd = EXPAND_PLAY_BACK_TYPE;
        t_expand_playback_type->ch_reserve = 0xff; //±£ÁôÎ»Îª0
        t_expand_playback_type->ch_video_type = (char)H264;         
        //bruce
        t_expand_playback_type->ch_frame_rate = fps; //»ñÈ¡Ö¡ÂÊ
#endif

    } 

    //Ö¡Í·Ê±¼äÈÕÆÚÌî³ä
    struct tm *tmip;
    time_t timep;        
    time(&timep);
    tmip = localtime(&timep);
    t_media_data_head->t_date.second = tmip->tm_sec; /* Ãë¨CÈ¡ÖµÇø¼äÎª[0,59] */
    t_media_data_head->t_date.minute = tmip->tm_min; /* ·Ö - È¡ÖµÇø¼äÎª[0,59] */
    t_media_data_head->t_date.hour = tmip->tm_hour; /* Ê± - È¡ÖµÇø¼äÎª[0,23] */
    t_media_data_head->t_date.day = tmip->tm_mday; /* Ò»¸öÔÂÖÐµÄÈÕÆÚ - È¡ÖµÇø¼äÎª[1,31] */
    t_media_data_head->t_date.month = tmip->tm_mon + 1; /* ÔÂ·ÝÈ¡ÖµÇø¼äÎª[1,12] */
    t_media_data_head->t_date.year = tmip->tm_year + 1900 - 2000; /* Äê·Ý£¬ÆäÖµ[0-63],2000-2063 */  

    //Ö¡Í·Ð£ÑéºÍÌî³ä
    int i = 0;
    unsigned char * p_temp = (unsigned char *)t_media_data_head;
    unl_sum = 0;
    for(i = 0; i < 23; i++) //Ç°23¸ö×Ö½ÚÀÛ¼ÓºÍ
    {
        unl_sum += *p_temp;
        p_temp = p_temp + 1;
    }
    t_media_data_head->ch_checksum = (unsigned char)unl_sum;

    ZhiNuo_Media_Frame_Tail t_frame_tail; 

    //Ö¡Î²Ìî³ä
    t_frame_tail.sz_tag[0] = 'd';
    t_frame_tail.sz_tag[1] = 'h';
    t_frame_tail.sz_tag[2] = 'a';        
    t_frame_tail.sz_tag[3] = 'v';  
    t_frame_tail.unl_data_len = t_media_data_head->n_frame_len;    

    n_offset += (t_media_data_head->n_frame_len - data_len) - sizeof(ZhiNuo_Media_Frame_Tail);     

    memcpy(send_buff + n_offset, data, data_len); 
    memcpy(send_buff + n_offset + data_len, &t_frame_tail, sizeof(ZhiNuo_Media_Frame_Tail));


    if (g_sock_sub.fd > 0)
    {
        PRINT_INFO("fd:%d send sub stream.\n", g_sock_sub.fd);
        ret = ZhiNuo_Send_Second_Unit(&g_sock_sub, send_buff, n_offset + data_len + sizeof(ZhiNuo_Media_Frame_Tail), DAHUA_ONE_PACKAGE_SIZE);
        if(ret <= 0)
        {
            PRINT_ERR("exit second av socket:%d\n", g_sock_sub.fd);  
            g_sock_sub.fd = -1;
        }
    }

    return 0;

}

/*************************************************************
 * º¯Êý½éÉÜ£ºsocketÖ÷¶¯Á¬½Ó
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int DahuaSendSubStream()
{
    int i = 0;
    BOOL b_have_connect = FALSE;    
    int ret;

    char *send_buff = (char*)malloc(MAX_SEND_BUFF_SIZE);
    if(NULL == send_buff)
    {
        PRINT_ERR("malloc failure\n");
        return 0;
    }

    //IÖ¡ÇëÇó
    //Ìí¼Ó¶ÁÖ¸Õë
    MEDIABUF_HANDLE handle = mediabuf_add_reader(1);
    if(handle)
        mediabuf_set_newest_frame(handle);

    char ch_frame_type = 0; 

    /*struct timeval tv_begin, tv_end;
      gettimeofday(&tv_begin, NULL);  
      int j = 0; */   

    GK_NET_FRAME_HEADER frame_header = {0};

    while(1)
    {   
        //±£Ö¤Ã»ÓÐÁ¬½ÓµÄÊ±ºò²»»ñÈ¡ÒôÊÓÆµÊý¾Ý
        if(g_sock_sub.fd < 0 || send_sub == 0)
        {
            usleep(40000);
            continue;
        }

        if (g_dahua_thread_run != 1) {
            PRINT_ERR("end loop\n");
            break;
        }

        char *p_data_buff = NULL;
        int data_len = 0;

        ret = mediabuf_read_frame((MEDIABUF_HANDLE)handle, (void **)&p_data_buff, &data_len, &frame_header);
        if (ret < 0) {
			PRINT_ERR("read sub stream fail.\n");
    		break;
        } else if (ret == 0) {
            PRINT_INFO("read no data.\n");
            usleep(3000);
            continue;
        }            
  

        //×ÓÂëÁ÷²»Òª´«ÒôÆµ
        if (frame_header.frame_type == GK_NET_FRAME_TYPE_A) 
        {
            continue;
            ch_frame_type = AUDIO_TYPE;
            g_audio_sequence_second_stream++;            
            g_audio_timestample_second_stream = frame_header.pts;
            /*
            //pcm->alaw
            char alaw_data[4096];
            int alaw_len;
            if(data_len / 2 > sizeof(alaw_data))
            {
            PRINT_ERR("data_size[%d] error\n", data_len);
            continue;
            }
            alaw_len = audio_alaw_encode(alaw_data, p_data_buff, data_len);
            p_data_buff = alaw_data;
            data_len = alaw_len;*/
        }else if ( (frame_header.frame_type == GK_NET_FRAME_TYPE_I) || (frame_header.frame_type == GK_NET_FRAME_TYPE_P) ){
            if(frame_header.frame_type == GK_NET_FRAME_TYPE_I){
                ch_frame_type = FRAME_TYPE_I;
            }else{
                ch_frame_type = FRAME_TYPE_P;
            }
            g_video_sequence_second_stream++;
            g_video_timestample_second_stream = frame_header.pts;
        }
        //pthread_mutex_lock(&main_second_stream_mutex); 
        ret = DahuaPackSubStream(send_buff, p_data_buff, data_len, ch_frame_type, 
                (char)(((frame_header.video_reso >> 16) & 0xff)/8), 
                (char)((frame_header.video_reso & 0xff)/8),
                frame_header.frame_rate); //8ÏñËØµãÎª1µ¥Î»
        //pthread_mutex_unlock(&main_second_stream_mutex);             
        if(ret < 0)
        {
            PRINT_ERR("send sub stream fail.\n");
            break;
        }  
    }  

    free(send_buff); 
    return 0;     
}

/*************************************************************
 * º¯Êý½éÉÜ£ºÃ¿¸ô5ms·¢ËÍ3kÊý¾Ý
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_Send_Main_Unit(DAHUA_SESSION_CTRL *sock, char *buf, unsigned long len, int nOnePacketSize)
{
    int ret = 0;
    unsigned long nOffSet = 0;
    unsigned long buf_len = len;
    int send_len = 0;

    int nOnePacketSize = DAHUA_ONE_PACKAGE_SIZE;
    DVRIP_HEAD_T t_msg_head;
    memset(&t_msg_head, 0, DVRIP_HEAD_T_SIZE);
    t_msg_head.dvrip.cmd = ACK_MEDIA_REQUEST;  
    t_msg_head.c[9] = 0; //Êý¾ÝÐòºÅ£¬ÀÛ¼Ó£¨Ã¿¿Í»§¶Ëµ¥¶ÀÀÛ¼Ó£©
    t_msg_head.c[8] = 0;  //Í¨µÀºÅ£¬ÈçÎªÔ¤ÀÀÊý¾Ý£¬ÌîÍ¨µÀÊýÁ¿£¬Èç16Â·»úÆ÷£¬Ìî16

    /* ·µ»ØÂë 0:Õý³£ 1:ÎÞÈ¨ÏÞ 2:²»Ö§³Ö´Ë²Ù×÷ 3:×ÊÔ´²»×ã 4:ÎÞ·¨»ñµÃËùÇëÇóÐòÁÐºÅµÄÊý¾Ý
10:Êý¾Ý½áÊø£¨Õë¶ÔÍ¼Æ¬¡¢I Ö¡µÈÓÐÏÞÊý¾Ý£©*/
    t_msg_head.c[16] = 0; 
    t_msg_head.c[24] = 0; //Ö÷ÂëÁ÷  
    //t_msg_head.s[7] = g_media_sequence_main_stream ++;

    //PRINT_INFO("ZhiNuo_FlvSendUnit len:%d\n", buf_len);
    while(buf_len > nOnePacketSize)
    {
        t_msg_head.dvrip.dvrip_extlen = nOnePacketSize;
        t_msg_head.s[7] = g_media_sequence_main_stream ++;
        ret = ZhiNuo_SockSend(sock, (char *)&t_msg_head, sizeof(DVRIP_HEAD_T), SEND_TIMEOUT);
        if(ret != sizeof(DVRIP_HEAD_T))
        {
            PRINT_ERR("ZhiNuo_SockSend fail0:%d\n", ret);
            return -1;
        }

        ret = ZhiNuo_SockSend(sock, buf + nOffSet, nOnePacketSize, SEND_TIMEOUT);
        if(ret != nOnePacketSize)
        {
            PRINT_ERR("ZhiNuo_SockSend fail0:%d\n", ret);
            return -1;
        }
        nOffSet += nOnePacketSize;
        buf_len -= nOnePacketSize;
        send_len += ret;
    }  

    if(buf_len > 0)
    {   
        t_msg_head.dvrip.dvrip_extlen = buf_len;
        t_msg_head.s[7] = g_media_sequence_main_stream ++;
        ret = ZhiNuo_SockSend(sock, (char *)&t_msg_head, sizeof(DVRIP_HEAD_T), SEND_TIMEOUT);
        if(ret != sizeof(DVRIP_HEAD_T))
        {
            PRINT_ERR("ZhiNuo_SockSend fail, ret:%d,len:%ld\n", ret, buf_len);            
            return -1;
        }

        ret = ZhiNuo_SockSend(sock, buf + nOffSet, buf_len, SEND_TIMEOUT);
        if(ret != buf_len)
        {
            PRINT_ERR("ZhiNuo_SockSend fail, ret%d\n", ret);
            return -1;
        }
        nOffSet += buf_len;
        send_len += ret;        
    }
    return send_len;

}


/*************************************************************
 * º¯Êý½éÉÜ£º¸øÒôÊÓÆµ´ò°ü
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int DahuaPackMainStream(DAHUA_SESSION_CTRL *p_ctrl, char* send_buff, const char * data, DWORD data_len, char frame_type, 
        char ch_width, char ch_high, unsigned char fps)
{
    int ret = 0;
    int nOnePacketSize = 0;
    unsigned long unl_sum = 0;

    ZhiNuo_Media_Frame_Head *t_media_data_head = (ZhiNuo_Media_Frame_Head *)(send_buff);
    //PRINT_INFO("t_media_data_head:%d, t_expand_audio_format:%d, t_expand_picture_size:%d, t_expand_playback_type:%d, t_frame_tail:%d\n",
    //   sizeof(ZhiNuo_Media_Frame_Head), sizeof(ZhiNuo_Expand_Audio_Format), 
    //  sizeof(ZhiNuo_Expand_Picture_Size), sizeof(ZhiNuo_Expand_PlayBack_Type),
    //  sizeof(ZhiNuo_Media_Frame_Tail));

    //Ö¡Í·Ìî³ä
    t_media_data_head->sz_tag[0] = 'D';
    t_media_data_head->sz_tag[1] = 'H';
    t_media_data_head->sz_tag[2] = 'A';
    t_media_data_head->sz_tag[3] = 'V';
    t_media_data_head->ch_media_type = frame_type;
    t_media_data_head->ch_child_type = 0; //Ã»ÓÐ×ÓÀàÐÍ
    t_media_data_head->ch_channel_num = 0;  //Í¨µÀºÅÎª  
    t_media_data_head->ch_child_sequence = 0;
    if(frame_type == AUDIO_TYPE)
    {
        ZhiNuo_Expand_Audio_Format *t_expand_audio_format = (ZhiNuo_Expand_Audio_Format *)
            (send_buff + sizeof(ZhiNuo_Media_Frame_Head));

        //ÒôÆµÖ¡Í·µÄÌØÊâ²¿·ÖÌî³ä
        t_media_data_head->n_frame_sequence = g_audio_sequence_main_stream;
        t_media_data_head->s_timestamp = g_audio_timestample_main_stream;
        t_media_data_head->n_frame_len = sizeof(ZhiNuo_Media_Frame_Head) + sizeof(ZhiNuo_Expand_Audio_Format) 
            + data_len + sizeof(ZhiNuo_Media_Frame_Tail);
        t_media_data_head->ch_expand_len = sizeof(ZhiNuo_Expand_Audio_Format);

        //ÒôÆµ¸ñÊ½À©Õ¹Ö¡Í·Ìî³ä
        t_expand_audio_format->ch_cmd = EXPAND_AUDIO_FORMAT;
        t_expand_audio_format->ch_audio_channel = (char)SINGLE_CHANNEL;
        t_expand_audio_format->ch_audio_type =   (char)G711A;
        t_expand_audio_format->ch_sample_frequency = (char)SAMPLE_FREQ_8000; // ×¥°ü 8000 ºÍ 16000 ¶¼ÊÇ4 ? 
    }
    else if(frame_type == FRAME_TYPE_P)
    {
        //ÊÓÆµÖ¡Í·µÄÌØÊâ²¿·ÖÌî³ä
        t_media_data_head->n_frame_sequence = g_video_sequence_main_stream;
        t_media_data_head->s_timestamp = g_video_timestample_main_stream;  

        //PÖ¡Ö¡Í·µÄÌØÊâ²¿·ÖÌî³ä
        t_media_data_head->n_frame_len = sizeof(ZhiNuo_Media_Frame_Head) + data_len 
            + sizeof(ZhiNuo_Media_Frame_Tail);  
        t_media_data_head->ch_expand_len = 0; //Ã»ÓÐÀ©Õ¹Ö¡Í·
    }
    else
    {
        ZhiNuo_Expand_Picture_Size *t_expand_picture_size = (ZhiNuo_Expand_Picture_Size *)
            (send_buff + sizeof(ZhiNuo_Media_Frame_Head));
        ZhiNuo_Expand_PlayBack_Type *t_expand_playback_type = (ZhiNuo_Expand_PlayBack_Type *)
            (send_buff + sizeof(ZhiNuo_Media_Frame_Head) + sizeof(ZhiNuo_Expand_Picture_Size));

        //ÊÓÆµÖ¡Í·µÄÌØÊâ²¿·ÖÌî³ä
        t_media_data_head->n_frame_sequence = g_video_sequence_main_stream;
        t_media_data_head->s_timestamp = g_video_timestample_main_stream;              

        //IÖ¡Ö¡Í·µÄÌØÊâ²¿·ÖÌî³ä
        t_media_data_head->n_frame_len = sizeof(ZhiNuo_Media_Frame_Head) + sizeof(ZhiNuo_Expand_Picture_Size)
            + sizeof(ZhiNuo_Expand_PlayBack_Type) + data_len + sizeof(ZhiNuo_Media_Frame_Tail); 
        t_media_data_head->ch_expand_len = sizeof(ZhiNuo_Expand_Picture_Size) + sizeof(ZhiNuo_Expand_PlayBack_Type);

        //Í¼Ïñ³ß´çÀ©Õ¹Ö¡Í·Ìî³ä
        t_expand_picture_size->ch_cmd = EXPAND_PICTURE_SIZE;
        t_expand_picture_size->ch_coding_type = ONE_FRAME;
        t_expand_picture_size->ch_width = ch_width;
        t_expand_picture_size->ch_high = ch_high;

        //»Ø·ÅÀàÐÍÀ©Õ¹Ö¡Í·Ìî³ä
        t_expand_playback_type->ch_cmd = EXPAND_PLAY_BACK_TYPE;
        t_expand_playback_type->ch_frame_rate = fps;  
        #if 1
        t_expand_playback_type->ch_reserve = 0xff; //±£ÁôÎ»Îª0
        t_expand_playback_type->ch_video_type = (char)H264;
        #else
        t_expand_playback_type->ch_reserve = 0x32; //±£ÁôÎ»Îª0
        t_expand_playback_type->ch_video_type = 0x08;
        t_expand_playback_type->cmd2[0] = 0x88;
        t_expand_playback_type->cmd2[1] = 0xaf;
        t_expand_playback_type->cmd2[2] = 0x23;
        t_expand_playback_type->cmd2[3] = 0xbb;
        t_expand_playback_type->cmd2[4] = 0xcd;
        t_expand_playback_type->cmd2[5] = 0x00;
        t_expand_playback_type->cmd2[6] = 0x00;
        t_expand_playback_type->cmd2[7] = 0x00;
        #endif

    } 

    //Ö¡Í·Ê±¼äÈÕÆÚÌî³ä
	long ts = time(NULL);
	struct tm tt = {0};
	struct tm *tmip = localtime_r(&ts, &tt);
    
    t_media_data_head->t_date.second = tmip->tm_sec; /* Ãë¨CÈ¡ÖµÇø¼äÎª[0,59] */
    t_media_data_head->t_date.minute = tmip->tm_min; /* ·Ö - È¡ÖµÇø¼äÎª[0,59] */
    t_media_data_head->t_date.hour = tmip->tm_hour; /* Ê± - È¡ÖµÇø¼äÎª[0,23] */
    t_media_data_head->t_date.day = tmip->tm_mday; /* Ò»¸öÔÂÖÐµÄÈÕÆÚ - È¡ÖµÇø¼äÎª[1,31] */
    t_media_data_head->t_date.month = tmip->tm_mon + 1; /* ÔÂ·ÝÈ¡ÖµÇø¼äÎª[1,12] */
    t_media_data_head->t_date.year = tmip->tm_year + 1900 - 2000; /* Äê·Ý£¬ÆäÖµ[0-63],2000-2063 */  

    //Ö¡Í·Ð£ÑéºÍÌî³ä
    int i = 0;
    unsigned char * p_temp = (unsigned char *)t_media_data_head;
    unl_sum = 0;
    for(i = 0; i < 23; i++) //Ç°23¸ö×Ö½ÚÀÛ¼ÓºÍ
    {
        unl_sum += *p_temp;
        p_temp = p_temp + 1;
    }
    t_media_data_head->ch_checksum = (unsigned char)unl_sum;


    ZhiNuo_Media_Frame_Tail t_frame_tail;    

    //Ö¡Î²Ìî³ä
    t_frame_tail.sz_tag[0] = 'd';
    t_frame_tail.sz_tag[1] = 'h';
    t_frame_tail.sz_tag[2] = 'a';        
    t_frame_tail.sz_tag[3] = 'v';  
    t_frame_tail.unl_data_len = t_media_data_head->n_frame_len;    

    if (t_media_data_head->n_frame_len > MAX_SEND_BUFF_SIZE) {
        PRINT_ERR("dahua send buf is not enough.\n");
        send_buff = (char *)realloc(send_buff, t_media_data_head->n_frame_len);
        if (send_buff == NULL)
        {
            PRINT_ERR_MSG("dahua send buf realloc failed.\n");
            exit(1);
        }
    }

    int n_offset = t_media_data_head->n_frame_len - data_len - sizeof(ZhiNuo_Media_Frame_Tail);  
    memcpy(send_buff + n_offset, data, data_len); 
    memcpy(send_buff + n_offset + data_len, &t_frame_tail, sizeof(ZhiNuo_Media_Frame_Tail));

    

    if (g_sock_main.fd > 0)
    {
        //PRINT_INFO("fd:%d send main stream.\n", g_sock_main.fd);
        ret = ZhiNuo_Send_Main_Unit(&g_sock_main, send_buff, t_media_data_head->n_frame_len, DAHUA_ONE_PACKAGE_SIZE);
        if(ret <= 0)
        {
            PRINT_ERR("exit main av socket:%d\n", g_sock_main.fd);  
            g_sock_main.fd = -1;
        }
    }


    return 0;

}
#endif

#if 1
int DahuaTcpSelectSend(int sock, char *szbuf, int len, int timeout, int re_counts)
{
    int ret = 0;
    fd_set send_set;
    struct timeval tv;
    int send_total = 0;
    int n_timeout_num = 0;
    if (timeout == 0)
        timeout = 6;
    if (re_counts == 0)
        re_counts = 2;
    while(send_total < len) {
        if(re_counts == n_timeout_num) {//Èç¹û³¬Ê±3´ÎÔò¹Ø±Õ´Ë´Î»á»°
            PRINT_ERR("sock:%d -- select send timeout. timeout is set %d seconds and counts is set %d\n", sock, timeout, n_timeout_num);
            return -2;
        }

    	FD_ZERO(&send_set);
    	FD_SET(sock, &send_set);
    	tv.tv_sec = timeout;
    	tv.tv_usec = 0;
        ret = select(sock + 1, NULL, &send_set, NULL, &tv);
       	if (ret < 0) {
            PRINT_ERR("sock:%d select send error:%s\n", sock, strerror(errno));
          	return -1;
    	}
        if(0 == ret) {
            n_timeout_num++;
            PRINT_ERR("sock:%d select send timeout. count:%d\n", sock, n_timeout_num);
            sleep(1);
            continue;
        }

		if(FD_ISSET(sock, &send_set)) {
        	ret = send(sock, szbuf + send_total, len - send_total, 0);
            if(ret <= 0) {
                PRINT_ERR("SockSend Failed ret:%d, sock:%d, len:%d error:%s\n", ret, sock, len, strerror(errno));
                return -3;
            }

    		n_timeout_num = 0;
			//break;
		}

        send_total += ret;
    }

    //PRINT_INFO("Leave ==========> SockSend sock:%d, send_total:%d\n",sock,  send_total);
	return send_total;
}

//·Ç×èÈû·½Ê½
int DahuaTcpSend(int sock, char *szbuf, int len, int re_counts)
{
    int ret = 0;
    int send_total = 0;
    int re_send_count = 0;
    while (send_total < len) {
    	ret = send(sock, szbuf + send_total, len - send_total, 0);
        if (ret <= 0) {
            re_send_count ++;
            if (re_send_count > re_counts) {
                PRINT_ERR("sock %d send error, to break. send_count:%d\n", sock, re_counts);
                return -1;
            }

            if(errno == EINTR) { /* ÖÐ¶Ï´íÎó ÎÒÃÇ¼ÌÐøÐ´*/
                //sleep(1); //µÈ´ýÒ»Ãë,±ÜÃâSDK bug
                PRINT_ERR("errno==EINTR continue, sock:%d, re_send_count:%d\n", sock, re_send_count);
                continue;  
            } else if(errno == EAGAIN) {/* EAGAIN : Resource temporarily unavailable*/   
                usleep(40000); //µÈ´ý40 ºÁÃë£¬Ï£Íû·¢ËÍ»º³åÇøÄÜµÃµ½ÊÍ·Å 
                PRINT_ERR("error errno==EAGAIN continue, sock:%d, re_send_count:%d\n", sock, re_send_count);
                continue;
            } else {
                PRINT_ERR("SockSend Failed ret:%d, sock:%d, len:%d error:%s\n", ret, sock, len, strerror(errno));
                return -3;
            }
        }
        re_send_count = 0;
        send_total += ret;
    }

    //PRINT_INFO("Leave ==========> SockSend sock:%d, send_total:%d\n",sock,  send_total);
	return send_total;
}


static int DahuaAddMediaHead(char *buf, DahuaFrameInfo *p_frame_info, unsigned long media_len)
{
    ZhiNuo_Media_Frame_Head *t_media_data_head = (ZhiNuo_Media_Frame_Head *)buf;

    //Ö¡Í·Ìî³ä
    t_media_data_head->sz_tag[0] = 'D';
    t_media_data_head->sz_tag[1] = 'H';
    t_media_data_head->sz_tag[2] = 'A';
    t_media_data_head->sz_tag[3] = 'V';
    t_media_data_head->ch_media_type = p_frame_info->ch_frame_type;
    t_media_data_head->ch_child_type = 0; //Ã»ÓÐ×ÓÀàÐÍ
    t_media_data_head->ch_channel_num = 0;  //Í¨µÀºÅÎª  
    t_media_data_head->ch_child_sequence = 0;


    if(p_frame_info->ch_frame_type == AUDIO_TYPE)
    {
        //ÒôÆµÖ¡Í·µÄÌØÊâ²¿·ÖÌî³ä
        t_media_data_head->n_frame_sequence = p_frame_info->a_seq;
        t_media_data_head->s_timestamp = p_frame_info->a_timestmp;
        t_media_data_head->n_frame_len = media_len;
        t_media_data_head->ch_expand_len = sizeof(ZhiNuo_Expand_Audio_Format);

        //ÒôÆµ¸ñÊ½À©Õ¹Ö¡Í·Ìî³ä
        ZhiNuo_Expand_Audio_Format *t_expand_audio_format = (ZhiNuo_Expand_Audio_Format *)
            (buf + sizeof(ZhiNuo_Media_Frame_Head));
        t_expand_audio_format->ch_cmd = EXPAND_AUDIO_FORMAT;
        t_expand_audio_format->ch_audio_channel = (char)SINGLE_CHANNEL;
        t_expand_audio_format->ch_audio_type =   (char)G711A;
        t_expand_audio_format->ch_sample_frequency = (char)SAMPLE_FREQ_8000; // ×¥°ü 8000 ºÍ 16000 ¶¼ÊÇ4 ? 
    }
    else if(p_frame_info->ch_frame_type == FRAME_TYPE_P)
    {
        //ÊÓÆµÖ¡Í·µÄÌØÊâ²¿·ÖÌî³ä
        t_media_data_head->n_frame_sequence = p_frame_info->v_seq;
        t_media_data_head->s_timestamp = p_frame_info->v_timestmp;  
        t_media_data_head->n_frame_len = media_len;  
        t_media_data_head->ch_expand_len = 0; //Ã»ÓÐÀ©Õ¹Ö¡Í·
    }
    else if(p_frame_info->ch_frame_type == FRAME_TYPE_I)
    {
        //ÊÓÆµÖ¡Í·µÄÌØÊâ²¿·ÖÌî³ä
        t_media_data_head->n_frame_sequence = p_frame_info->v_seq;
        t_media_data_head->s_timestamp = p_frame_info->v_timestmp;  
        t_media_data_head->n_frame_len = media_len; 
        t_media_data_head->ch_expand_len = sizeof(ZhiNuo_Expand_Picture_Size) + sizeof(ZhiNuo_Expand_PlayBack_Type);


        //Í¼Ïñ³ß´çÀ©Õ¹Ö¡Í·Ìî³ä
        ZhiNuo_Expand_Picture_Size *t_expand_picture_size = (ZhiNuo_Expand_Picture_Size *)
            (buf + sizeof(ZhiNuo_Media_Frame_Head));
        t_expand_picture_size->ch_cmd = EXPAND_PICTURE_SIZE;
        t_expand_picture_size->ch_coding_type = ONE_FRAME;
        t_expand_picture_size->ch_width = p_frame_info->ch_width;
        t_expand_picture_size->ch_high = p_frame_info->ch_height;

        //»Ø·ÅÀàÐÍÀ©Õ¹Ö¡Í·Ìî³ä
        ZhiNuo_Expand_PlayBack_Type *t_expand_playback_type = (ZhiNuo_Expand_PlayBack_Type *)
            (buf + sizeof(ZhiNuo_Media_Frame_Head) + sizeof(ZhiNuo_Expand_Picture_Size)); 
        t_expand_playback_type->ch_cmd = EXPAND_PLAY_BACK_TYPE;
        t_expand_playback_type->ch_frame_rate = p_frame_info->ch_fps;  
#if 1
        t_expand_playback_type->ch_reserve = 0xff; //±£ÁôÎ»Îª0
        t_expand_playback_type->ch_video_type = (char)H264;
#else
        t_expand_playback_type->ch_reserve = 0x32; //±£ÁôÎ»Îª0
        t_expand_playback_type->ch_video_type = 0x08;
        t_expand_playback_type->cmd2[0] = 0x88;
        t_expand_playback_type->cmd2[1] = 0xaf;
        t_expand_playback_type->cmd2[2] = 0x23;
        t_expand_playback_type->cmd2[3] = 0xbb;
        t_expand_playback_type->cmd2[4] = 0xcd;
        t_expand_playback_type->cmd2[5] = 0x00;
        t_expand_playback_type->cmd2[6] = 0x00;
        t_expand_playback_type->cmd2[7] = 0x00;
#endif

    } 

    //Ö¡Í·Ê±¼äÈÕÆÚÌî³ä
	long ts = time(NULL);
	struct tm tt = {0};
	struct tm *tmip = localtime_r(&ts, &tt);
    
    t_media_data_head->t_date.second = tmip->tm_sec; /* Ãë¨CÈ¡ÖµÇø¼äÎª[0,59] */
    t_media_data_head->t_date.minute = tmip->tm_min; /* ·Ö - È¡ÖµÇø¼äÎª[0,59] */
    t_media_data_head->t_date.hour = tmip->tm_hour; /* Ê± - È¡ÖµÇø¼äÎª[0,23] */
    t_media_data_head->t_date.day = tmip->tm_mday; /* Ò»¸öÔÂÖÐµÄÈÕÆÚ - È¡ÖµÇø¼äÎª[1,31] */
    t_media_data_head->t_date.month = tmip->tm_mon + 1; /* ÔÂ·ÝÈ¡ÖµÇø¼äÎª[1,12] */
    t_media_data_head->t_date.year = tmip->tm_year + 1900 - 2000; /* Äê·Ý£¬ÆäÖµ[0-63],2000-2063 */  

    //Ö¡Í·Ð£ÑéºÍÌî³ä
    int i = 0;
    unsigned char * p_temp = (unsigned char *)t_media_data_head;
    unsigned long unl_sum = 0;
    for(i = 0; i < 23; i++) //Ç°23¸ö×Ö½ÚÀÛ¼ÓºÍ
    {
        unl_sum += *p_temp;
        p_temp = p_temp + 1;
    }
    t_media_data_head->ch_checksum = (unsigned char)unl_sum;

    //*all_len = t_media_data_head->n_frame_len;

    return 0;
}

static int DahuaAddAlarmHead(char *buf, unsigned long media_len)
{
    ZhiNuo_Media_Frame_Head *t_media_data_head = (ZhiNuo_Media_Frame_Head *)buf;

    //Ö¡Í·Ìî³ä
    t_media_data_head->sz_tag[0] = 'D';
    t_media_data_head->sz_tag[1] = 'H';
    t_media_data_head->sz_tag[2] = 'A';
    t_media_data_head->sz_tag[3] = 'V';
    t_media_data_head->ch_media_type = 0xf1;
    t_media_data_head->ch_child_type = 0x0d; //Ã»ÓÐ×ÓÀàÐÍ
    t_media_data_head->ch_channel_num = 0;  //Í¨µÀºÅÎª  
    t_media_data_head->ch_child_sequence = 0;
    t_media_data_head->n_frame_sequence = 0;
    t_media_data_head->n_frame_len = media_len;
    //t_media_data_head->ch_expand_len = 0; //Ã»ÓÐÀ©Õ¹Ö¡Í· ×¥°üÊÇ8
    t_media_data_head->ch_expand_len = 8;

    //Ö¡Í·Ð£ÑéºÍÌî³ä
    int i = 0;
    unsigned char * p_temp = (unsigned char *)t_media_data_head;
    unsigned long unl_sum = 0;
    for(i = 0; i < 23; i++) //Ç°23¸ö×Ö½ÚÀÛ¼ÓºÍ
    {
        unl_sum += *p_temp;
        p_temp = p_temp + 1;
    }
    t_media_data_head->ch_checksum = (unsigned char)unl_sum;

    return 0;
}

static int DahuaAddExDataCheck(char *buf)
{
    ZhiNuo_Expand_Data_Check *t_ex_data_check = (ZhiNuo_Expand_Data_Check *)buf;
    memset(t_ex_data_check, 0, sizeof(ZhiNuo_Expand_Data_Check));
    
    //Ö¡Í·Ìî³ä
    t_ex_data_check->ch_cmd = EXPAND_DATA_CHECK;
    t_ex_data_check->n_check_result = 0x28c06196;

    return 0;
}

static int DahuaGetTimetick(short *timetick)
{
	if(timetick == NULL)
		return -1;

	long ts = time(NULL);
    printf("ts:%x\n", (unsigned int)ts);
    
    struct timeval tt2;
    gettimeofday(&tt2, NULL);
    printf("tt2.sec:%x\n", (unsigned int)tt2.tv_sec);

    *timetick = (short)tt2.tv_sec;
	return 0;
}


static int DahuaAddAlarmMsgHead(char *buf, unsigned char cmd, unsigned int media_len)
{
    DVRIP_HEAD_T *p_msg_head = (DVRIP_HEAD_T *)buf;
    memset(p_msg_head, 0, DVRIP_HEAD_T_SIZE);
    p_msg_head->dvrip.cmd = cmd;  
    p_msg_head->dvrip.dvrip_extlen = media_len;

	short timetick;
	DahuaGetTimetick(&timetick);

    //p_msg_head->s[7] = timetick;  // ÐèÒª½øÒ»²½È·¶¨ÊÇ·ñÊÇ short, todo

    p_msg_head->s[7] = 0x0326;

    return 0;
}



static int DahuaAddMsgHead(char *buf, unsigned char cmd, int stream_type, int seq, int pack_len)
{
    DVRIP_HEAD_T *p_msg_head = (DVRIP_HEAD_T *)buf;

    p_msg_head->dvrip.cmd = cmd;  
    p_msg_head->c[9] = 0; //Êý¾ÝÐòºÅ£¬ÀÛ¼Ó£¨Ã¿¿Í»§¶Ëµ¥¶ÀÀÛ¼Ó£©
    p_msg_head->c[8] = 0;  //Í¨µÀºÅ£¬ÈçÎªÔ¤ÀÀÊý¾Ý£¬ÌîÍ¨µÀÊýÁ¿£¬Èç16Â·»úÆ÷£¬Ìî16

    /* ·µ»ØÂë 0:Õý³£ 1:ÎÞÈ¨ÏÞ 2:²»Ö§³Ö´Ë²Ù×÷ 3:×ÊÔ´²»×ã 4:ÎÞ·¨»ñµÃËùÇëÇóÐòÁÐºÅµÄÊý¾Ý
    10:Êý¾Ý½áÊø£¨Õë¶ÔÍ¼Æ¬¡¢I Ö¡µÈÓÐÏÞÊý¾Ý£©*/
    p_msg_head->c[16] = 0;
    if (stream_type == 0)
        p_msg_head->c[24] = 0; //Ö÷ÂëÁ÷  todo
    else if (stream_type == 1)
        p_msg_head->c[24] = 5; //×ÓÂëÁ÷
    //t_msg_head.s[7] = g_media_seq

    p_msg_head->dvrip.dvrip_extlen = pack_len;
    p_msg_head->s[7] = seq;  // ÐèÒª½øÒ»²½È·¶¨ÊÇ·ñÊÇ short, todo

    return 0;
}


static char * DahuaPackFrame(DahuaFrameInfo *p_frame_info, unsigned long *all_len)	
{
    //¼ÆËãÃ½ÌåÊý¾Ý×Ü³¤¶È
    unsigned long media_len = 0;
    if (p_frame_info->ch_frame_type == AUDIO_TYPE) {
        media_len = sizeof(ZhiNuo_Media_Frame_Head) + sizeof(ZhiNuo_Expand_Audio_Format) 
            + p_frame_info->frame_len + sizeof(ZhiNuo_Media_Frame_Tail);
    } else if(p_frame_info->ch_frame_type == FRAME_TYPE_P) {
        media_len = sizeof(ZhiNuo_Media_Frame_Head) + p_frame_info->frame_len 
            + sizeof(ZhiNuo_Media_Frame_Tail);  
    } else if(p_frame_info->ch_frame_type == FRAME_TYPE_I){
        media_len = sizeof(ZhiNuo_Media_Frame_Head) + sizeof(ZhiNuo_Expand_Picture_Size)
            + sizeof(ZhiNuo_Expand_PlayBack_Type) + p_frame_info->frame_len + sizeof(ZhiNuo_Media_Frame_Tail); 
    }

    //ÉêÇëÃ½ÌåÊý¾Ý»º´æ
    char *media_buf = (char*)malloc(media_len);
    if (NULL == media_buf) {
        PRINT_ERR("malloc failure\n");
        return NULL;
    }
    memset(media_buf, 0, media_len);


    //Ö¡Í·Ìî³ä
    DahuaAddMediaHead(media_buf, p_frame_info, media_len);

    //Ö¡Î²Ìî³ä
    ZhiNuo_Media_Frame_Tail t_frame_tail;
    t_frame_tail.sz_tag[0] = 'd';
    t_frame_tail.sz_tag[1] = 'h';
    t_frame_tail.sz_tag[2] = 'a';        
    t_frame_tail.sz_tag[3] = 'v';  
    t_frame_tail.unl_data_len = media_len;  

    //Ö¡Êý¾ÝÌî³ä
    int media_head_len = media_len - p_frame_info->frame_len - sizeof(ZhiNuo_Media_Frame_Tail);
    memcpy(media_buf + media_head_len, p_frame_info->pData, p_frame_info->frame_len); 
    memcpy(media_buf + media_head_len + p_frame_info->frame_len, &t_frame_tail, sizeof(ZhiNuo_Media_Frame_Tail));

    *all_len = media_len;

    return media_buf;
}

static int DahuaSendFrameUnit(DahuaFrameInfo *p_frame_info, int sock, int sid, int stream_type, char *media_buf, unsigned long media_len)
{
    //·Ö°ü·¢ËÍ£¬Ò»´Î·¢ËÍÊý¾Ý´óÐ¡: DAHUA_ONE_PACKAGE_SIZE + DVRIP_HEAD_T_SIZE
    int nOnePacketSize = DAHUA_ONE_PACKAGE_SIZE; 

    int ret = 0;
    unsigned long nOffSet = 0;
    char *buf = media_buf;
    unsigned long buf_len = media_len;

    DVRIP_HEAD_T t_msg_head;
    int seq = p_frame_info->msg_seq;
    while (buf_len > nOnePacketSize) {
        memset(&t_msg_head, 0, DVRIP_HEAD_T_SIZE);
        seq ++;
        DahuaAddMsgHead((char *)&t_msg_head, ACK_MEDIA_REQUEST, stream_type, seq, nOnePacketSize);
        
        ret = DahuaTcpSend(sock, (char *)&t_msg_head, DVRIP_HEAD_T_SIZE, 2);
        if (ret < 0) {
            PRINT_ERR("send stream data header error.\n");
            return -1;
        }

        ret = DahuaTcpSelectSend(sock, buf + nOffSet, nOnePacketSize, 0, 0);
        if (ret < 0) {
            PRINT_ERR("send stream data error.\n");
            return -1;
        }
        
        nOffSet += nOnePacketSize;
        buf_len -= nOnePacketSize;
    }  

    if (buf_len > 0) {
        memset(&t_msg_head, 0, DVRIP_HEAD_T_SIZE);
        seq ++;
        DahuaAddMsgHead((char *)&t_msg_head, ACK_MEDIA_REQUEST, stream_type, seq, buf_len);

        ret = DahuaTcpSend(sock, (char *)&t_msg_head, DVRIP_HEAD_T_SIZE, 2);
        if (ret < 0) {
            PRINT_ERR("send stream data header error.\n");
            return -1;
        }

        ret = DahuaTcpSelectSend(sock, buf + nOffSet, buf_len, 0, 0);
        if (ret < 0) {
            PRINT_ERR("send stream data error.\n");
            return -1;
        }
        
        nOffSet += buf_len;
        buf_len -= buf_len;
    }

    return 0;
}

static int DahuaCalMsgSeq(DahuaFrameInfo *p_frame_info, int media_len)
{
    //·Ö°ü·¢ËÍ£¬Ò»´Î·¢ËÍÊý¾Ý´óÐ¡: DAHUA_ONE_PACKAGE_SIZE + DVRIP_HEAD_T_SIZE
    int nOnePacketSize = DAHUA_ONE_PACKAGE_SIZE;

    int num = (media_len + nOnePacketSize - 1) / nOnePacketSize;
    p_frame_info->msg_seq += num;
	return 0;
}



static int DahuaArraySendStream(DahuaFrameInfo *p_frame_info, int stream_type, char *media_buf, unsigned long media_len)
{
    #if MULTI_CONNECT
    int ret = 0;
    int is_wait_i;
    int sock = -1;
    int sid = 0;
    int i = 0;

    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
		sock = UtGetLvSockByIndex(i, stream_type, &g_dahua_session);
        if (sock > 0) {
            //ÅÐ¶ÏÊÇ·ñµÈ´ýIÖ¡
            is_wait_i = UtGetLvSockWaitIByIndex(i, stream_type, &g_dahua_session);		
            if (is_wait_i) {
                if (p_frame_info->ch_frame_type != FRAME_TYPE_I)
                    continue;
                else
                    UtSetLvSockWaitIByIndex(i, stream_type, 0, &g_dahua_session);
            }
            //»ñÈ¡sid
            sid = UtGetSidByIndex(i, &g_dahua_session);

            //PRINT_INFO("send stream%d, sock:%d sid:%d", stream_type, sock, sid);
            ret = DahuaSendFrameUnit(p_frame_info, sock, sid, stream_type, media_buf, media_len);
            //Èç¹û·¢ËÍÊ§°Ü£¬Ôò¹Ø±Õ¶ÔÓ¦µÄdata sock
            if (ret <0) {
                UtCloseLvSockByIndex(i, stream_type, &g_dahua_session);
                continue;
            }
        }
    }

    #else
    if (stream_type == 0)
        DahuaSendFrameUnit(p_frame_info, main_sock, 0, stream_type, media_buf, media_len);
    else if (stream_type == 1)
        DahuaSendFrameUnit(p_frame_info, sub_sock, 0, stream_type, media_buf, media_len);
    #endif

    // p_frame_info->msg_seq Ôö¼Ó
    DahuaCalMsgSeq(p_frame_info, media_len);

    return 0;
}


static int DahuaPackSendStream(DahuaFrameInfo *p_frame_info, int stream_type)
{
    // pack stream
    unsigned long media_len = 0;
    char *media_buf = DahuaPackFrame(p_frame_info, &media_len);
    if (NULL == media_buf) {
        PRINT_ERR("DahuaPackFrame failed\n");
        return -1;
    }

    // send stream
    pthread_mutex_lock(&g_dahua_stream_mutex);
    DahuaArraySendStream(p_frame_info, stream_type, media_buf, media_len);
    pthread_mutex_unlock(&g_dahua_stream_mutex);
    
    free(media_buf);
    return 0;
}


static int DahuaReadSendStream(MEDIABUF_HANDLE reader, int stream_type, DahuaFrameInfo *p_frame_info)
{
    //PRINT_INFO();
    if (reader == NULL) {
        PRINT_ERR("not add reader.\n");
        return -1;
    }

    GK_NET_FRAME_HEADER frame_header = {0};
    void *frame_data = NULL;
    int frame_len = 0;
    int ret;

    ret = mediabuf_read_frame(reader, (void **)&frame_data, &frame_len, &frame_header);
    if (ret < 0) {
		PRINT_ERR("cmd read stream%d : mediabuf_read_frame fail.\n", stream_type);
		usleep(300*1000);
		return -1;
    } else if (ret == 0) {
        PRINT_INFO("mediabuf_read_frame = 0\n");
		usleep(300*1000);
        return 0;
    } else {
        //PRINT_INFO("cmd read stream%d : mediabuf_read_frame OK.\n", tBST);
    }

    
    char alaw_data[4096] = {0};
    //»ñÈ¡´ËÖ¡Êý¾ÝµÄÀàÐÍ
    if (frame_header.frame_type == GK_NET_FRAME_TYPE_A) {
        p_frame_info->ch_frame_type = AUDIO_TYPE;
        p_frame_info->a_seq ++;
        p_frame_info->a_timestmp = frame_header.pts;
    
        //pcm->alaw
        if(frame_len / 2 > sizeof(alaw_data)) {
            PRINT_ERR("data_size[%d] error\n", frame_len);
            return -1;
        }
        int alaw_len = audio_alaw_encode((unsigned char *)alaw_data, (short *)frame_data, (unsigned int)frame_len);
        p_frame_info->frame_len = alaw_len;
        p_frame_info->pData = alaw_data;
    } else if ((frame_header.frame_type == GK_NET_FRAME_TYPE_I) || (frame_header.frame_type == GK_NET_FRAME_TYPE_P) ) {
        if (frame_header.frame_type == GK_NET_FRAME_TYPE_I)  {
            p_frame_info->ch_frame_type = FRAME_TYPE_I;
        } else if (frame_header.frame_type == GK_NET_FRAME_TYPE_P) {
            p_frame_info->ch_frame_type = FRAME_TYPE_P;
        }
        p_frame_info->v_seq ++;
        p_frame_info->v_timestmp = frame_header.pts;
        p_frame_info->ch_width = (unsigned char)(((frame_header.video_reso >> 16) & 0xFFFF) / 8);
        p_frame_info->ch_height = (unsigned char)((frame_header.video_reso & 0xFFFF) / 8);
        p_frame_info->ch_fps = frame_header.frame_rate;
        p_frame_info->frame_len = frame_len;
        p_frame_info->pData = frame_data;
    } else {
        PRINT_ERR("frame type %u error\n", frame_header.frame_type);
        return -1;
    }

    DahuaPackSendStream(p_frame_info, stream_type);

    return 0;
}


/*************************************************************
 * º¯Êý½éÉÜ£ºsocketÖ÷¶¯Á¬½Ó
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int DahuaSendStreamLoop(GK_NET_STREAM_TYPE type)
{
    //Ìí¼Ó¶ÁÖ¸Õë
    MEDIABUF_HANDLE reader = mediabuf_add_reader(type);
    if (reader == NULL) {
        PRINT_ERR("Add reader to stream%d error.\n", type);
        return -1;
    }

    DahuaFrameInfo frame_info = {0};
    while(1)
    {
        if (g_dahua_thread_run != 1) {
            PRINT_ERR("end loop\n");
            break;
        }

        #if MULTI_CONNECT
        //¼ì²éÊÇ·ñÓÐ¿Í»§¶ËÁ¬½Óµ½cms£¬Èç¹ûÃ»ÓÐÔòµÈ´ý
        if(UtGetStreamOpenNum(type, &g_dahua_session) == 0) {
            //PRINT_INFO("no online open, so not to send stream%d\n", stream_id);
            usleep(300*1000);
            continue;
        }
        #else
        //´ó»ªVERSION5°æ±¾£¬Ö»ÄÜ·¢Ö÷ÂëÁ÷»òÕß´ÎÂëÁ÷
        if (type == GK_NET_STREAM_TYPE_STREAM0) {
            if ((send_main == 0) || (main_sock < 0)) {
                usleep(300*1000);
                continue;
            }
            PRINT_INFO("send main stream, sock:%d", main_sock);
        } else if (type == GK_NET_STREAM_TYPE_STREAM1) {
            if ((send_sub == 0) || (sub_sock < 0)) {
                usleep(300*1000);
                continue;
            }
            PRINT_INFO("send sub stream, sock:%d", sub_sock);
        }
        #endif

        /* 1 »ñÈ¡ÂëÂÊ 2 ·¢ËÍ´«Êä */
        //if (send_data)
            DahuaReadSendStream(reader, type, &frame_info);
    }  

    if (reader)
        mediabuf_del_reader(reader);

    return 0;
}


/*************************************************************
 * º¯Êý½éÉÜ£º·¢ËÍ¸±ÂëÁ÷Êý¾ÝÏß³Ì
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
void * DahuaSendMainStreamPthread(void *n_sock)
{
    DahuaSetThreadName("DahuaSendMainStreamPthread");

    DahuaSendStreamLoop(GK_NET_STREAM_TYPE_STREAM0);

    PRINT_INFO("DahuaSendMainStreamPthread end !\n");
    return NULL;
}


/*************************************************************
 * º¯Êý½éÉÜ£º·¢ËÍÖ÷ÂëÁ÷Êý¾ÝÏß³Ì
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
void * DahuaSendSubStreamPthread(void *n_sock)
{
    DahuaSetThreadName("DahuaSendSubStreamPthread");

    DahuaSendStreamLoop(GK_NET_STREAM_TYPE_STREAM1);

    PRINT_INFO("DahuaSendSubStreamPthread end !\n");
    return NULL;    
}

#endif

// ·µ»Ø 0 £¬ÑéÖ¤Í¨¹ý£» ·Ç0£¬ÑéÖ¤Ê§°Ü
static int dahua_check_user(char *name, char *pass)
{
    if (strlen(name) == 0) {
        return -1;
    }
    
    PRINT_INFO("cfg   user name:%s, pass:%s\n", runUserCfg.user[0].userName, runUserCfg.user[0].password);
    PRINT_INFO("login user name:%s, pass:%s\n", name, pass);

    int ret = -1;
    if (strncmp(runUserCfg.user[0].userName, name, strlen(name)) == 0) {
        if (strlen(pass) == 0) {
            if (strlen(runUserCfg.user[0].password) == 0)
                ret = 0;
        } else {
            if (strncmp(runUserCfg.user[0].password, pass, strlen(pass)) == 0)
                ret = 0;
        }
    }

    return ret;
}


/*************************************************************
 * º¯Êý½éÉÜ£ºµÇÂ¼nvr
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int Dahua_LogOn(char* bufPost, DAHUA_SESSION_CTRL *pCtrl)
{
    char tem_buf[DVRIP_HEAD_T_SIZE] = {0};
    memcpy(tem_buf, bufPost, DVRIP_HEAD_T_SIZE);
    DVRIP_HEAD_T *t_msg_head_rcv = (DVRIP_HEAD_T *)tem_buf;

    //DVRIP_VERSION
    char name[64] = {0};
    char pass[64] = {0};
    if (t_msg_head_rcv->dvrip.dvrip_extlen == 0) {
        memcpy(name, t_msg_head_rcv->dvrip.dvrip_p, 8);
        memcpy(pass, t_msg_head_rcv->dvrip.dvrip_p + 8, 8);
    } else {
        char * extra = bufPost + DVRIP_HEAD_T_SIZE; 
        sscanf(extra, "%[^&]&&%s", name, pass);
    }

    #if 1
    int check = dahua_check_user(name, pass);
    #else
    int check = 0; //Ö±½ÓÑéÖ¤Í¨¹ý
    #endif

    char send_msg[BUFLEN] = {0};
    DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)send_msg;             
    ZERO_DVRIP_HEAD_T(t_msg_head)
        t_msg_head->dvrip.cmd = ACK_LOG_ON;   
    t_msg_head->c[1] = 0; //0£ºÃ»ÓÐ 1£º¾ßÓÐ¶à»­ÃæÔ¤ÀÀ¹¦ÄÜ 
    t_msg_head->c[2] = 0; //0: Ö§³Ö16Â· 1:Ö§³Ö32Â·

    int login_ok = 0;
    if (check == 0) {
        //²úÉúsession id
        pCtrl->fSessionInt = UtGenSessionInt(&g_dahua_session);

        //½«»á»°Ö÷socket¼ÓÈë g_xmai_socket Êý×é
        int flag = UtAddCmdSock(pCtrl->fd, pCtrl->fSessionInt, &g_dahua_session);

        if (flag == 0) {
            t_msg_head->c[8] = 0; //1; //0: ±íÊ¾µÇÂ¼³É¹¦ 1: ±íÊ¾µÇÂ¼Ê§°Ü 3: ÓÃ»§ÒÑµÇÂ¼

            /* µÇÂ¼Ê§°Ü·µ»ØÂë  0:ÃÜÂë²»ÕýÈ· 1:ÕÊºÅ²»´æÔÚ 3:ÕÊºÅÒÑ¾­µÇÂ¼ 4:ÕÊºÅ±»Ëø¶¨ 
            5:ÕÊºÅ±»ÁÐÈëºÚÃûµ¥ 6:×ÊÔ´²»×ã£¬ÏµÍ³Ã¦ 7:°æ±¾²»¶Ô£¬ÎÞ·¨µÇÂ½ */
            //t_msg_head->c[9] = 0; //µÇÂ¼³É¹¦
            t_msg_head->c[9] = 8; //V6°æ±¾µÇÂ¼³É¹¦
            t_msg_head->l[4] = pCtrl->fSessionInt;
            pCtrl->fSessionType = CMD_SESSION;
            login_ok = 1;
        } else if (flag == 1) {
            t_msg_head->c[8] = 3;
            t_msg_head->c[9] = 3; //3:ÕÊºÅÒÑ¾­µÇÂ¼
        } else if (flag == -1) {
            PRINT_ERR("link to dahua is max.\n"); 
            t_msg_head->c[8] = 1;
            t_msg_head->c[9] = 6; //6:×ÊÔ´²»×ã
        }
    } else {
        t_msg_head->c[8] = 1; //0: ±íÊ¾µÇÂ¼³É¹¦ 1: ±íÊ¾µÇÂ¼Ê§°Ü 3: ÓÃ»§ÒÑµÇÂ¼

        /* µÇÂ¼Ê§°Ü·µ»ØÂë  0:ÃÜÂë²»ÕýÈ· 1:ÕÊºÅ²»´æÔÚ 3:ÕÊºÅÒÑ¾­µÇÂ¼ 4:ÕÊºÅ±»Ëø¶¨ 
        5:ÕÊºÅ±»ÁÐÈëºÚÃûµ¥ 6:×ÊÔ´²»×ã£¬ÏµÍ³Ã¦ 7:°æ±¾²»¶Ô£¬ÎÞ·¨µÇÂ½ */
        t_msg_head->c[9] = 0; //µÇÂ¼Ê§°Ü·µ»ØÂë
        t_msg_head->l[4] = 0;
    }

    t_msg_head->c[10] = 1;  //Í¨µÀÊý    
    //t_msg_head->c[11] = 9;   //ÊÓÆµ±àÂë·½Ê½ 8:MPEG4 9:H.264
    t_msg_head->c[11] = 0;   //ÊÓÆµ±àÂë·½Ê½ 8:MPEG4 9:H.264
    t_msg_head->c[12] = 51;  //Éè±¸ÀàÐÍ  51:IPCÀà²úÆ·

    PRINT_ERR("sock:%d, sid:%d(0x%08x), sid:%d(0x%08x)\n", 
        pCtrl->fd, t_msg_head->l[4], t_msg_head->l[4], pCtrl->fSessionInt, pCtrl->fSessionInt);

    #if 0
    t_msg_head->c[28] = 0;  //ÊÓÆµÖÆÊ½£¬0: ±íÊ¾PALÖÆ     1: ±íÊ¾NTSCÖÆ
    t_msg_head->c[24] = 0x06;  
    t_msg_head->c[26] = 0xf9;  
    t_msg_head->c[29] = 0x01;      
    t_msg_head->c[30] = 0x64;  
    t_msg_head->c[31] = 0x02;     
    //t_msg_head->s[15] = 0x8101; //µÚ30¡¢31×Ö½Ú±£ÁôÒÔÏÂÖµ ´úÀíÍø¹Ø²úÆ·ºÅ0x8101, 0x8001, 0x8002, 0x8003,Õâ¸öÖµnvrÃ»ÓÃµ½Ëæ±ãÌî
    #else
    t_msg_head->c[20] = 1;
    t_msg_head->c[24] = 0x06;
    t_msg_head->c[26] = 0xf9;
    if (login_ok) {
        t_msg_head->c[29] = 0;
    } else {
        static int login_count = 10; 
        t_msg_head->c[29] = login_count --;
    }
    t_msg_head->c[31] = 0x02;
    #endif
    ZhiNuo_SockSend(pCtrl, send_msg, DVRIP_HEAD_T_SIZE, 100);

    return 0;
}

/*************************************************************
 * º¯Êý½éÉÜ£º²éÑ¯¹¤×÷¼°±¨¾¯×´Ì¬
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 * ±¸×¢    :ÊÕµ½0ºÍ4µÄ²éÑ¯ÀàÐÍ
 *************************************************************/
int ZhiNuo_Search_Status(char* bufPost, DAHUA_SESSION_CTRL *pCtrl)
{
    int rcv_msg_len = 0;
    char search_content = 0;
    char tem_buf[DVRIP_HEAD_T_SIZE] = {0};
    memcpy(tem_buf, bufPost, DVRIP_HEAD_T_SIZE);    
    DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)bufPost;
    search_content = t_msg_head->c[8];

    PRINT_INFO("1Search content:%d, hl:%d, v:%d, vextralen:%d\n", t_msg_head->c[8], 
            t_msg_head->dvrip.dvrip_hl, t_msg_head->dvrip.dvrip_v, t_msg_head->dvrip.dvrip_extlen);

    rcv_msg_len = DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen;
    char send_msg[BUFLEN] = {0};
    t_msg_head = (DVRIP_HEAD_T *)send_msg;      

    ZERO_DVRIP_HEAD_T(t_msg_head)
        t_msg_head->dvrip.cmd = ACK_STATUS_SEARCH; 
    t_msg_head->c[8] = search_content;

    //Â¼Ïñ¹¤×÷×´Ì¬£¬0~31°´Î»±íÊ¾32¸öÍ¨µÀµÄÂ¼Ïñ×´Ì¬£¬0±íÊ¾Ã»ÓÐÂ¼Ïñ£¬1±íÊ¾ÕýÔÚÂ¼Ïñ
    t_msg_head->c[9] = 0;
    t_msg_head->c[10] = 0;
    t_msg_head->c[11] = 0;    
    t_msg_head->c[12] = 0; 

    /*±¨¾¯·µ»ØÓÐÐ§ÐÔ±êÊ¶£¬Ã¿Î»¶ÔÓ¦Ò»ÖÖÀàÐÍµÄ±¨¾¯£¬Öµ0±íÊ¾ÓÐÏàÓ¦±¨¾¯£¬Öµ1±íÊ¾ÎÞÏàÓ¦±¨¾¯
      µÚÒ»Î»£º±íÊ¾Íâ²¿±¨¾¯
      µÚ¶þÎ»£º±íÊ¾ÊÓÆµ¶ªÊ§
      µÚÈýÎ»£º±íÊ¾¶¯Ì¬¼ì²â
      µÚËÄÎ»£º±íÊ¾µÚÈý·½±¨¾¯£¨µ±W08µÄÖµÎª2Ê±£¬´ËÎ»ÎÞÐ§£©
      µÚÎåÎ»£º±íÊ¾´®¿Ú¹ÊÕÏ±¨¾¯
     */
    //t_msg_head->c[13] = 0xfb;
    t_msg_head->c[13] = 0;

    //Íâ²¿±¨¾¯×´Ì¬£¬ÓÉµÍµ½¸ß°´Î»±íÊ¾32¸ö±¨¾¯
    t_msg_head->l[4] = 0;

    //ÊÓÆµ¶ªÊ§±¨¾¯×´Ì¬£¬ÓÉµÍµ½¸ß°´Î»±íÊ¾32¸ö±¨¾¯
    t_msg_head->l[5] = 0;
    //¶¯Ì¬¼ì²â±¨¾¯×´Ì¬£¬ÓÉµÍµ½¸ß°´Î»±íÊ¾32¸ö±¨¾¯
    //if( g_VideoMotionState == 1)
    if(0)
        t_msg_head->l[6] = 1;
    else
        t_msg_head->l[6] = 0;

    //´®¿Ú¹ÊÕÏ±¨¾¯×´Ì¬£¬ÓÉµÍµ½¸ß°´Î»±íÊ¾8¸ö±¨¾¯
    t_msg_head->c[28] = 0;

    //ÒôÆµÊäÈë×´Ì¬£¬ÓÉµÍµ½¸ß°´Î»±íÊ¾16¸ö±¨¾¯
    t_msg_head->c[29] = 0;

    ZhiNuo_SockSend(pCtrl, send_msg, DVRIP_HEAD_T_SIZE, 100);

    return rcv_msg_len;
}

/*************************************************************
 * º¯Êý½éÉÜ£º²éÑ¯Éè±¸ÊÂ¼þ
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 * ±¸×¢    :ÊÕµ½²Ù×÷ÀàÐÍÎª2:¶©ÔÄ±¨¾¯×´Ì¬ ÊÂ¼þÀàÐÍÎª1:$Íâ²¿$±¨¾¯2:$¶¯Ì¬¼ì²â$±¨¾¯3:$ÊÓÆµ¶ªÊ§$±¨¾¯4:$ÕÚµ²¼ì²â$±¨¾¯
 *************************************************************/
int ZhiNuo_Dev_Event(char* bufPost, DAHUA_SESSION_CTRL * sock)
{
    int rcv_msg_len = 0;
    char ch_operate_type = 0;
    char ch_event_type = 0;
    char ch_mask = 0;
    char tem_buf[DVRIP_HEAD_T_SIZE] = {0};
    memcpy(tem_buf, bufPost, DVRIP_HEAD_T_SIZE);
    DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)tem_buf;  
    ch_operate_type = t_msg_head->c[8];
    ch_event_type = t_msg_head->c[12];
    ch_mask = t_msg_head->c[13];

    /*²Ù×÷ÀàÐÍ:0:±£Áô1:²éÑ¯±¨¾¯×´Ì¬2:¶©ÔÄ±¨¾¯×´Ì¬3:*È¡Ïû*¶©ÔÄ±¨¾¯×´Ì¬   
      W28 ~W31 ËÄ¸ö×Ö½Ú°´Î»±íÊ¾Ê®Áù¸öÍ¨µÀ£¬ÖÃ 1Îª¶©ÔÄ×´Ì¬*/
    PRINT_INFO("operate type:%d, event_type:%d, mask:%d, hl:%d, v:%d, vextralen:%d\n", 
            t_msg_head->c[8], t_msg_head->c[12], t_msg_head->c[13],
            t_msg_head->dvrip.dvrip_hl, t_msg_head->dvrip.dvrip_v, t_msg_head->dvrip.dvrip_extlen);

    rcv_msg_len = DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen;
    char send_msg[BUFLEN] = {0};
    t_msg_head = (DVRIP_HEAD_T *)send_msg;                
    ZERO_DVRIP_HEAD_T(t_msg_head)
        t_msg_head->dvrip.cmd = ACK_DEV_EVENT_SEARCH;   
    t_msg_head->c[8] = ch_operate_type ;
    t_msg_head->c[12] = ch_event_type;
    t_msg_head->c[13] = ch_mask ;
    t_msg_head->c[16] = 0;   //·µ»ØÂë 0:³É¹¦1:Ê§°Ü2:ÎÞÈ¨ÏÞ3:Éè±¸Ã¦ 

    ZhiNuo_SockSend(sock, send_msg, DVRIP_HEAD_T_SIZE, 100);

    t_msg_head->dvrip.dvrip_extlen = 4;
    t_msg_head->c[8] = 1;    

    //Í¨¹ý×¥ÖÇÅµipc·¢ÏÖ»¹»Ø¸´ÁËÕâÑùÒ»¸ö±¨ÎÄ
    ZhiNuo_SockSend(sock, send_msg, DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen, 100);

    return rcv_msg_len;
}
/*************************************************************
 * º¯Êý½éÉÜ£ºÃ½ÌåÊý¾ÝÇëÇó
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_Media_Request(char* bufPost, DAHUA_SESSION_CTRL * sock)
{
    char tem_buf[DVRIP_HEAD_T_SIZE] = {0};
    memcpy(tem_buf, bufPost, DVRIP_HEAD_T_SIZE);
    DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)tem_buf;
    char stream_type = *(char*)(bufPost + DVRIP_HEAD_T_SIZE);

    PRINT_INFO("hl:%d, v:%d,stream:%d, vextralen:%d, addr:%p\n", t_msg_head->dvrip.dvrip_hl,
            t_msg_head->dvrip.dvrip_v, stream_type, t_msg_head->dvrip.dvrip_extlen, &(t_msg_head->dvrip.dvrip_extlen));


    char send_msg[BUFLEN] = {0};
    t_msg_head = (DVRIP_HEAD_T *)send_msg;                
    ZERO_DVRIP_HEAD_T(t_msg_head)
        t_msg_head->dvrip.cmd = ACK_MEDIA_REQUEST;  
    t_msg_head->c[9] = 0; //Êý¾ÝÐòºÅ£¬ÀÛ¼Ó£¨Ã¿¿Í»§¶Ëµ¥¶ÀÀÛ¼Ó£©
    t_msg_head->c[8] = 0;  //Í¨µÀºÅ£¬ÈçÎªÔ¤ÀÀÊý¾Ý£¬ÌîÍ¨µÀÊýÁ¿£¬Èç16Â·»úÆ÷£¬Ìî16

    /* ·µ»ØÂë 0:Õý³£ 1:ÎÞÈ¨ÏÞ 2:²»Ö§³Ö´Ë²Ù×÷ 3:×ÊÔ´²»×ã 4:ÎÞ·¨»ñµÃËùÇëÇóÐòÁÐºÅµÄÊý¾Ý
10:Êý¾Ý½áÊø£¨Õë¶ÔÍ¼Æ¬¡¢I Ö¡µÈÓÐÏÞÊý¾Ý£©*/
    t_msg_head->c[16] = 0; 
    if((1 == stream_type) || (0 == stream_type))
    {
        t_msg_head->c[16] = 0; 
    }

    ZhiNuo_SockSend(sock, send_msg, DVRIP_HEAD_T_SIZE, 100);

    return 0;
}

/*************************************************************
 * º¯Êý½éÉÜ£º²éÑ¯Éè±¸Ã½ÌåÄÜÁ¦ÐÅÏ¢
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_Search_Media_Capacity(char* bufPost, DAHUA_SESSION_CTRL *sock)
{
    int i = 0;
    int rcv_msg_len = 0;
    char ch_channel_num = 0;
    char ch_search_type = 0;
    char tem_buf[DVRIP_HEAD_T_SIZE] = {0};
    memcpy(tem_buf, bufPost, DVRIP_HEAD_T_SIZE);
    DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)tem_buf; 

    PRINT_INFO("hl:%d, v:%d, vextralen:%d\n", t_msg_head->dvrip.dvrip_hl, t_msg_head->dvrip.dvrip_v, 
            t_msg_head->dvrip.dvrip_extlen);

    //Í¨µÀºÅ²éÑ¯1-16, 0-±íÊ¾²»°´Í¨µÀ²éÑ¯   ²éÑ¯ÐÅÏ¢Àà±ð 0:²éÑ¯Ö÷ÂëÁ÷   1:²éÑ¯¸¨ÂëÁ÷ 2:²éÑ¯·Ö±çÂÊÐÅÏ¢
    PRINT_INFO("Search_channel:%d Search_type:%d\n", t_msg_head->c[8], t_msg_head->c[9]);
    ch_channel_num = t_msg_head->c[8];
    ch_search_type = t_msg_head->c[9];

    rcv_msg_len = DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen;
    char send_msg[BUFLEN] = {0};
    t_msg_head = (DVRIP_HEAD_T *)send_msg;                
    ZERO_DVRIP_HEAD_T(t_msg_head)
        t_msg_head->dvrip.cmd = ACK_MEDIA_CAPACITY_SEARCH; 

    //·µ»ØÂë 0:Õý³£ 1:Ê§°Ü 5:Ê§°Ü£¬Ô­Òò£º²»Ö§³Ö²»°´Í¨µÀºÅ²éÑ¯ÄÜÁ¦¼¯ 6:Ê§°Ü£¬Ô­Òò£¬²»Ö§³Ö°´Í¨µÀºÅ²éÑ¯ÄÜÁ¦
    t_msg_head->c[8] = 0; 
    t_msg_head->c[9] = ch_search_type; //·µ»ØÐÅÏ¢Àà±ð£¨Óë²éÑ¯¶ÔÓ¦£©
    t_msg_head->c[10] = ch_channel_num; //Í¨µÀºÅ²éÑ¯£¨Óë²éÑ¯¶ÔÓ¦£©

    GK_NET_VIDEO_CFG stSupportFmt;
    goke_api_get_ioctrl(GET_VIDEO_CFG,&stSupportFmt);

    //Ö÷ÂëÁ÷²éÑ¯
    if(0 == ch_search_type)
    {       
        t_msg_head->dvrip.dvrip_extlen = sizeof(FRAME_CAPS);
        FRAME_CAPS str_fram_caps;
        memset(&str_fram_caps, 0, sizeof(FRAME_CAPS));   
        str_fram_caps.Compression |= (1 << CAPTURE_COMP_H264);

        PRINT_INFO("stSupportFmt.vencStream[0].resolution:%s\n", stSupportFmt.vencStream[0].resolution);
        const char *resolution_str = (char*) stSupportFmt.vencStream[0].resolution;
        if(strstr(resolution_str, "1280x1080")){
            str_fram_caps.ImageSize |= (1 << CAPTURE_SIZE_1080p);  
            PRINT_INFO("1080p\n");
        }
        if(strstr(resolution_str, "1280x960")){
            str_fram_caps.ImageSize |= (1 << CAPTURE_SIZE_1_3M);  
            PRINT_INFO("960 p\n");
        }
        if(strstr(resolution_str, "1280x720")){
            str_fram_caps.ImageSize |= (1 << CAPTURE_SIZE_720p);
            PRINT_INFO("720 p\n");
        }
        if(strstr(resolution_str, "1024x576") || strstr(resolution_str, "960x576") ||
                strstr(resolution_str, "720x576")){
            str_fram_caps.ImageSize |= (1 << CAPTURE_SIZE_D1);
            PRINT_INFO("d1 p\n");
        }
        if(strstr(resolution_str, "800x600")){
            str_fram_caps.ImageSize |= (1 << CAPTURE_SIZE_SVGA);
            PRINT_INFO("600 p\n");
        }
        if(strstr(resolution_str, "720x480") || strstr(resolution_str, "640x480") ||
                strstr(resolution_str, "640x360")){
            str_fram_caps.ImageSize |= (1 << CAPTURE_SIZE_VGA);  
            PRINT_INFO("480 p\n");
        }
        if(strstr(resolution_str, "320x240")){
            str_fram_caps.ImageSize |= (1 << CAPTURE_SIZE_QVGA);  
            PRINT_INFO("240 p\n");
        }
        if(strstr(resolution_str, "352x288") || strstr(resolution_str, "352x240")){
            str_fram_caps.ImageSize |= (1 << CAPTURE_SIZE_CIF);  
            PRINT_INFO("288 p\n");
        }
        if(strstr(resolution_str, "176x144")){
            str_fram_caps.ImageSize |= (1 << CAPTURE_SIZE_QCIF);  
            PRINT_INFO("144 p\n");
        }
        if(strstr(resolution_str, "352x576")){
            str_fram_caps.ImageSize |= (1 << CAPTURE_SIZE_HD1);  
            PRINT_INFO("hd1 p\n");
        }
        if(strstr(resolution_str, "160x128")){
            str_fram_caps.ImageSize |= (1 << CAPTURE_SIZE_QQVGA);  
            PRINT_INFO("128 p\n");
        }
        if(strstr(resolution_str, "1600x1200")){
            str_fram_caps.ImageSize |= (1 << CAPTURE_SIZE_UXGA);  
            PRINT_INFO("1200 p\n");
        }
        if(strstr(resolution_str, "800x592")){
            str_fram_caps.ImageSize |= (1 << CAPTURE_SIZE_SVGA);  
            PRINT_INFO("592 p\n");
        }
        if(strstr(resolution_str, "1024x768") || strstr(resolution_str, "1024x576")){
            str_fram_caps.ImageSize |= (1 << CAPTURE_SIZE_XVGA);  
            PRINT_INFO("768 p\n");
        }              
        memcpy(send_msg + DVRIP_HEAD_T_SIZE, &str_fram_caps, t_msg_head->dvrip.dvrip_extlen);
    }

    //×ÓÂëÁ÷²éÑ¯
    if(1 == ch_search_type)
    {
        t_msg_head->c[11] = 1; //×¥°ü·¢ÏÖÕâ¸ö×Ö¶ÎÌî³äµÄ1µ«ÊÇ²»ÖªµÀÊÇÊ²Ã´ÒâË¼
        t_msg_head->c[15] = 1; //ÅäÖÃ°æ±¾ºÅ
        t_msg_head->dvrip.dvrip_extlen = sizeof(CAPTURE_EXT_STREAM);
        CAPTURE_EXT_STREAM str_fram_caps;
        DWORD un_CaptureSizeMask = 0;
        memset(&str_fram_caps, 0, sizeof(CAPTURE_EXT_STREAM));
        str_fram_caps.ExtraStream |= 1; //±íÊ¾Ö§³ÖÖ÷ÂëÁ÷
        str_fram_caps.ExtraStream |= 2; //±íÊ¾Ö§³Ö¸±ÂëÁ÷1
        str_fram_caps.ExtraStream |= 16;//±íÊ¾Ö§³Öjpg×¥Í¼


        PRINT_INFO("stSupportFmt.vencStream[1].resolution:%s\n", stSupportFmt.vencStream[1].resolution);
        char *resolution_str = (char*)stSupportFmt.vencStream[1].resolution;
        if(strstr(resolution_str, "1280x1080")){
            un_CaptureSizeMask |=(1 << CAPTURE_SIZE_1080p);  
            PRINT_INFO("1080p\n");
        }
        if(strstr(resolution_str, "1280x960")){
            un_CaptureSizeMask |=(1 << CAPTURE_SIZE_1_3M);  
            PRINT_INFO("960 p\n");
        }
        if(strstr(resolution_str, "1280x720")){
            un_CaptureSizeMask |=(1 << CAPTURE_SIZE_720p);
            PRINT_INFO("720 p\n");
        }
        if(strstr(resolution_str, "1024x576") || strstr(resolution_str, "960x576") ||
                strstr(resolution_str, "720x576")){
            un_CaptureSizeMask |=(1 << CAPTURE_SIZE_D1);
            PRINT_INFO("d1 p\n");
        }
        if(strstr(resolution_str, "800x600")){
            un_CaptureSizeMask |=(1 << CAPTURE_SIZE_SVGA);
            PRINT_INFO("600 p\n");
        }
        if(strstr(resolution_str, "720x480") || strstr(resolution_str, "640x480") ||
                strstr(resolution_str, "640x360")){
            un_CaptureSizeMask |=(1 << CAPTURE_SIZE_VGA);  
            PRINT_INFO("480 p\n");
        }
        if(strstr(resolution_str, "320x240")){
            un_CaptureSizeMask |=(1 << CAPTURE_SIZE_QVGA);  
            PRINT_INFO("240 p\n");
        }
        if(strstr(resolution_str, "352x288") || strstr(resolution_str, "352x240")){
            un_CaptureSizeMask |=(1 << CAPTURE_SIZE_CIF);  
            PRINT_INFO("288 p\n");
        }
        if(strstr(resolution_str, "176x144")){
            un_CaptureSizeMask |=(1 << CAPTURE_SIZE_QCIF);  
            PRINT_INFO("144 p\n");
        }
        if(strstr(resolution_str, "352x576")){
            un_CaptureSizeMask |=(1 << CAPTURE_SIZE_HD1);  
            PRINT_INFO("hd1 p\n");
        }
        if(strstr(resolution_str, "160x128")){
            un_CaptureSizeMask |=(1 << CAPTURE_SIZE_QQVGA);  
            PRINT_INFO("128 p\n");
        }
        if(strstr(resolution_str, "1600x1200")){
            un_CaptureSizeMask |=(1 << CAPTURE_SIZE_UXGA);  
            PRINT_INFO("1200 p\n");
        }
        if(strstr(resolution_str, "800x592")){
            un_CaptureSizeMask |=(1 << CAPTURE_SIZE_SVGA);  
            PRINT_INFO("592 p\n");
        }
        if(strstr(resolution_str, "1024x768") || strstr(resolution_str, "1024x576")){
            un_CaptureSizeMask |=(1 << CAPTURE_SIZE_XVGA);  
            PRINT_INFO("768 p\n");
        }              
        for(i = 0; i < 64; i++)
        {
            str_fram_caps.CaptureSizeMask[i] = un_CaptureSizeMask;
        }
        memcpy(send_msg + DVRIP_HEAD_T_SIZE, &str_fram_caps, t_msg_head->dvrip.dvrip_extlen);

    }

    //·Ö±æÂÊ²éÑ¯
    if(2 == ch_search_type)
    {
        t_msg_head->dvrip.dvrip_extlen = sizeof(FRAME_CAPS);
        FRAME_CAPS str_fram_caps;
        memset(&str_fram_caps, 0, sizeof(FRAME_CAPS));

        //µÚÒ»Î»±íÊ¾Ö§³ÖÖ÷ÂëÁ÷µÚ¶þÎ»±íÊ¾Ö§³Ö¸¨ÂëÁ÷1µÚÈýÎ»±íÊ¾Ö§³Ö¸¨ÂëÁ÷2µÚÎåÎ»±íÊ¾Ö§³Öjpg×¥Í¼
        str_fram_caps.Compression |= 1; 
        str_fram_caps.Compression |= 2;
        //str_fram_caps.ImageSize |= (1 << CAPTURE_SIZE_720p);
        //str_fram_caps.ImageSize |= (1 << CAPTURE_SIZE_D1);
        //str_fram_caps.ImageSize |= (1 << CAPTURE_SIZE_SVGA);

    }   

    ZhiNuo_SockSend(sock, send_msg, DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen, 100);

    return rcv_msg_len;
}

/*************************************************************
 * º¯Êý½éÉÜ£º²éÑ¯ÅäÖÃ²ÎÊý
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_Search_Config(char* bufPost, DAHUA_SESSION_CTRL *sock)
{
    int rcv_msg_len = 0;
    unsigned char search_type = 0;
    char search_child_type = 0;
    char tem_buf[DVRIP_HEAD_T_SIZE] = {0};
    memcpy(tem_buf, bufPost, DVRIP_HEAD_T_SIZE);
    DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)tem_buf;    
    search_type = t_msg_head->c[16];
    search_child_type = t_msg_head->c[24];
    PRINT_INFO("hl:%d, v:%d, vextralen:%d\n", t_msg_head->dvrip.dvrip_hl, 
            t_msg_head->dvrip.dvrip_v, t_msg_head->dvrip.dvrip_extlen);

    //´òÓ¡ËÑË÷ÀàÐÍ 16:±àÂëÄÜÁ¦ÐÅÏ¢ 38:ÉãÏñÍ·²ÎÊýÅäÖÃ£¬127:Ë«ÂëÁ÷Í¼ÏñÅäÖÃ 126:ÑÕÉ«ÅäÖÃ 124:»ñµÃÊÂ¼þÅäÖÃ 
    PRINT_INFO("Search_type:%d, child_type:%d\n", t_msg_head->c[16], t_msg_head->c[24]);

    rcv_msg_len = DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen;
    char send_msg[BUFLEN] = {0};
    t_msg_head = (DVRIP_HEAD_T *)send_msg;                
    ZERO_DVRIP_HEAD_T(t_msg_head)
        t_msg_head->dvrip.cmd = ACK_CONFIG_SEARCH;   
    t_msg_head->c[16] = search_type; //ËÑË÷ÀàÐÍ(39	:Íø¿¨ÅäÖÃ )
    t_msg_head->c[19] = 0; //·µ»ØÂë 0:Õý³£ 1:ÎÞÈ¨ÏÞ 2:ÎÞ¶ÔÓ¦ÅäÖÃÌá¹©
    t_msg_head->c[8] =  'c';
    t_msg_head->c[9] =  'o';
    t_msg_head->c[10] = 'n';
    t_msg_head->c[11] = 'f';
    t_msg_head->c[12] = 'i';
    t_msg_head->c[13] = 'g';
    t_msg_head->c[24] = search_child_type;

    switch(search_type)
    {
        #if 0
		#if ADD_MD_INFO
        case 0:
            {
                unsigned char ack[]="b300005816000000636f6e66696700000000000000000000010000000000000044484156fc000000080000010108000001000042acf2";
                int ret, i;
                ret = string2hex(ack, send_msg);
                for(i = 0; i < ret; i++)
                {
                    printf("%02x", send_msg[i]);
                }
                printf("\n");

            }
            break;
		#endif
        #endif
        case 0:
        {
            t_msg_head->dvrip.dvrip_extlen = sizeof(CONFIG_TIME);
            t_msg_head->c[24] = 1;
            CONFIG_TIME stTime;
            memset(&stTime, 0, sizeof(CONFIG_TIME));
            stTime.c[8] = 0x8;
            stTime.c[11] = 0x1;
            stTime.c[12] = 0x1;
            stTime.c[13] = 0x5;
            switch (runChannelCfg.channelInfo[0].osdDatetime.dateFormat)
            {
                case 0:
                    stTime.c[15] = 0; // 0 ÄêÔÂÈÕ£¬ 1 ÔÂÈÕÄê£¬ 2 ÈÕÔÂÄê
                    stTime.c[16] = 1; // 0 . ; 1 - ; 2 /  
                    break;
                case 1:
                    stTime.c[15] = 1;
                    stTime.c[16] = 1; // 0 . ; 1 - ; 2 /  
                    break;
                case 2:
                    stTime.c[15] = 0; // 0 ÄêÔÂÈÕ£¬ 1 ÔÂÈÕÄê£¬ 2 ÈÕÔÂÄê
                    stTime.c[16] = 2; // 0 . ; 1 - ; 2 /  
                    break;
                case 3:
                    stTime.c[15] = 1;
                    stTime.c[16] = 2; // 0 . ; 1 - ; 2 /  
                    break;
                case 4:
                    stTime.c[15] = 2;
                    stTime.c[16] = 1; // 0 . ; 1 - ; 2 /  
                    break;
                case 5:
                    stTime.c[15] = 2;
                    stTime.c[16] = 2; // 0 . ; 1 - ; 2 /  
                    break;
                default:
                    stTime.c[15] = 0; // 0 ÄêÔÂÈÕ£¬ 1 ÔÂÈÕÄê£¬ 2 ÈÕÔÂÄê
                    stTime.c[16] = 1; // 0 . ; 1 - ; 2 /  
                    break;
            }

            switch (runChannelCfg.channelInfo[0].osdDatetime.dateSprtr)
            {
                case 0:
                    stTime.c[16] = 1; // 0 . ; 1 - ; 2 /  
                    break;
                case 1:
                    stTime.c[16] = 1; // 0 . ; 1 - ; 2 /  
                    break;
                case 2:
                    stTime.c[16] = 2; // 0 . ; 1 - ; 2 /  
                    break;
                case 3:
                    stTime.c[16] = 0; // 0 . ; 1 - ; 2 /  
                    break;
                default:
                    stTime.c[16] = 1; // 0 . ; 1 - ; 2 /  
                    break;
            }
            
            stTime.c[17] = runChannelCfg.channelInfo[0].osdDatetime.timeFmt;
            memcpy(send_msg + DVRIP_HEAD_T_SIZE, &stTime, sizeof(CONFIG_TIME));
        }
        break;

            
        case 6://ÒÆ¶¯Õì²âÐÅÏ¢²éÑ¯(¶¯Ì¬¼ì²â)
            t_msg_head->dvrip.dvrip_extlen = dahua_get_md_attr(send_msg);
            break;
        case 16:
            {
                PRINT_INFO("search_type:%d, ±àÂëÄÜÁ¦ÐÅÏ¢\n", search_type);
                t_msg_head->dvrip.dvrip_extlen = sizeof(CAPTURE_ENCCHIPCAPABILITY); 
                int width_main = 0;
                int height_main = 0;
                int n_max_resolution_main = 0;     
                int width_second = 0;
                int height_second = 0;
                int n_max_resolution_second = 0;         
                CAPTURE_ENCCHIPCAPABILITY str_encchip_capability;
                GK_NET_VIDEO_CFG stSupportFmt;
                goke_api_get_ioctrl(GET_VIDEO_CFG,&stSupportFmt);
                //Ö÷ÂëÁ÷×î´ó·Ö±æÂÊ
                if(stSupportFmt.vencStream[0].h264Conf.width * stSupportFmt.vencStream[0].h264Conf.height > n_max_resolution_main)
                {
                    width_main = stSupportFmt.vencStream[0].h264Conf.width;
                    height_main = stSupportFmt.vencStream[0].h264Conf.height;
                    n_max_resolution_main = width_main * height_main;
                }
                //´ÎÂëÁ÷×î´ó·Ö±æÂÊ
                if(stSupportFmt.vencStream[1].h264Conf.width * stSupportFmt.vencStream[1].h264Conf.height > n_max_resolution_second)
                {
                    width_second = stSupportFmt.vencStream[1].h264Conf.width;
                    height_second = stSupportFmt.vencStream[1].h264Conf.height;
                    n_max_resolution_second = width_second * height_second;
                }            

                //test
                //n_max_resolution_main = 1280 * 720;/*×î´óµÄ·Ö±æÂÊ*/
                //n_max_resolution_second = 640 * 480;/*×î´óµÄ·Ö±æÂÊ*/

                PRINT_INFO("sw:%d, sh:%d, mw:%d, mh:%d\n", width_second, height_second, width_main, height_main);
                memset(&str_encchip_capability, 0, sizeof(CAPTURE_ENCCHIPCAPABILITY));
                str_encchip_capability.EncChipNR = 1;
                str_encchip_capability.EncChipInfo[0].nMaxEncodePower = n_max_resolution_main * 60 + n_max_resolution_second * 60; //Ö÷ÂëÁ÷ºÍ×ÓÂëÁ÷µÄÄÜÁ¦Ö®ºÍ)///< DSP Ö§³ÖµÄ×î¸ß±àÂëÄÜÁ¦¡£
                str_encchip_capability.EncChipInfo[0].nMaxSupportChannel = 1;	///< DSP Ö§³Ö×î¶àÊäÈëÊÓÆµÍ¨µÀÊý¡£
                str_encchip_capability.EncChipInfo[0].bChannelMaxSetSync = 0;	///< DSP Ã¿Í¨µÀµÄ×î´ó±àÂëÉèÖÃÊÇ·ñÍ¬²½ 0-²»Í¬²½, 1 -Í¬²½¡£
                str_encchip_capability.EncChipInfo[0].nExpandChannel = 0; ////< DSP Ö§³ÖµÄÀ©Õ¹Í¨µÀÊý£¬Ö÷ÒªÊÇ¶àÂ·»Ø·ÅÊ¹ÓÃ£¬Ä¿Ç°Ö»ÊÇÒ»¸ö

                memcpy(send_msg + DVRIP_HEAD_T_SIZE, &str_encchip_capability, sizeof(CAPTURE_ENCCHIPCAPABILITY));  
            }
            break;
        case 34://osd ¸²¸Ç
            {//osd ¸²¸Ç
                PRINT_INFO("search_type:%d, »ñÈ¡ÊÓÆµÇøÓòÕÚµ²\n", search_type);
                VIDEO_COVER_T video_cover;
                VIDEO_COVER_ATTR_T cover_attr[4];
                GK_NET_SHELTER_RECT cover[4];

                memset(&video_cover, 0, sizeof(video_cover));
                memset(&cover_attr, 0, sizeof(cover_attr));
                if(0 == goke_api_get_ioctrl(GET_COVER_CFG, cover))
                {
                    video_cover.iChannel = 0;
                    int i, j;
                    for(i = 0,j = 0; i < 4; i++)
                    {
                        if(cover[i].enable == 1 && (cover[i].width > 0) && (cover[i].height > 0))
                        {
                            video_cover.iCoverNum ++;
                            cover_attr[j].tBlock.left = (int)(cover[i].x * 8192.0);
                            cover_attr[j].tBlock.top= (int)(cover[i].y * 8192.0);
                            cover_attr[j].tBlock.right = (int)(cover[i].width * 8192.0);
                            cover_attr[j].tBlock.bottom = (int)(cover[i].height * 8192.0);
                            cover_attr[j].Color = 0;
                            cover_attr[j].iBlockType = 0;
                            cover_attr[j].Encode = 0;
                            cover_attr[j].Priview = 0;
                            cover_attr[j].iRev[0] = 0x00;//×¥°üµÃÖª
                            cover_attr[j].iRev[1] = 0x01;//×¥°üµÃÖª
                            PRINT_INFO("left:%d, top:%d, right:%d, bottom:%d, color:%d, type:%d, enc:%d, view:%d\n", cover_attr[j].tBlock.left,
                                    cover_attr[j].tBlock.top, cover_attr[j].tBlock.right,cover_attr[j].tBlock.bottom,
                                    cover_attr[j].Color,cover_attr[j].iBlockType,cover_attr[j].Encode,cover_attr[j].Priview);
                            j ++;
                        }            
                    }
                }
                PRINT_ERR("iCoverNum:%d\n", video_cover.iCoverNum);
                t_msg_head->c[17] = 0;
                memcpy(send_msg + DVRIP_HEAD_T_SIZE, &video_cover, sizeof(VIDEO_COVER_T));          
                t_msg_head->dvrip.dvrip_extlen = sizeof(VIDEO_COVER_T);
                memcpy(send_msg + DVRIP_HEAD_T_SIZE + sizeof(VIDEO_COVER_T), cover_attr, sizeof(VIDEO_COVER_ATTR_T) * video_cover.iCoverNum);          
                t_msg_head->dvrip.dvrip_extlen += sizeof(VIDEO_COVER_ATTR_T) * video_cover.iCoverNum;
            }
            break;
        case 38://38:ÉãÏñÍ·²ÎÊýÅäÖÃ
            {
                PRINT_INFO("search_type:%d, ÉãÏñÍ·²ÎÊýÅäÖÃ\n", search_type);
                CAM_CONTROL cam_control;
                memset(&cam_control, 1, sizeof(cam_control));
                cam_control.Exposure = 0;
                cam_control.Backlight = 1;
                cam_control.AutoColor2BW = 1;
                cam_control.Mirror = 1;
                cam_control.Flip = 1;
                cam_control.LensFunction = 0x10;
                cam_control.WhiteBalance = 1;
                cam_control.SignalFormatMask = 0;
                memcpy(send_msg + DVRIP_HEAD_T_SIZE, &cam_control, sizeof(cam_control));
                t_msg_head->dvrip.dvrip_extlen = sizeof(cam_control);

            }
            break;
        case 39://ÍøÂçÐÅÏ¢
            {
                PRINT_INFO("search_type:%d, »ñµÃÍøÂçÐÅÏ¢\n", search_type);
                printf("### search_type:%d, search_child_type=%d\n", search_type, search_child_type);
                ST_SDK_NETWORK_ATTR stNetInfo;
                if(search_child_type==1){//Ä¬ÈÏÍø¿¨ÅäÖÃ
                    if(0 != goke_api_get_ioctrl(GET_NETWORK_CFG,&stNetInfo))
                    {
                        PRINT_ERR("DMS_NET_GET_PLATFORMCFG fail\n");
                        return -1;
                    } 
                    sprintf(send_msg + DVRIP_HEAD_T_SIZE, "eth0,%d,%d,%02x:%02x:%02x:%02x:%02x:%02x::&&eth0", 0, 1,
                            stNetInfo.mac[0], stNetInfo.mac[1],
                            stNetInfo.mac[2], stNetInfo.mac[3],
                            stNetInfo.mac[4], stNetInfo.mac[5]);
                    t_msg_head->dvrip.dvrip_extlen=strlen(send_msg + DVRIP_HEAD_T_SIZE);
                }
            }
            break;        
		#if ADD_MD_INFO  
        case 124://124:»ñµÃÊÂ¼þÅäÖÃ
            {
                PRINT_INFO("search_type:%d,search_child_type:%d, »ñµÃÊÂ¼þÅäÖÃ\n", search_type, search_child_type);
                if(4 == search_child_type)
                { 
                    PRINT_INFO("»ñµÃ¶¯Ì¬¼ì²âÊÂ¼þ\n");
                    //×¥°ü»ñµÃ
                    unsigned char ack[]="0000000003000000ffff3f00ffff3f00ffff3f00ffff3f00ffff3f00ffff3f00ffff3f00ffff3f00ffff3f00ffff3f00ffff3f00ffff3f00ffff3f00ffff3f00ffff3f00ffff3f00ffff3f00ffff3f000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010000000a0000000100000001000000010000000a0000000000000001000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000010000000000000000000000010000000000000000000000000000000000000000000000000000000000000000000000010000000000000001000000050000000000000001000000000000000000000000000000000000000000000000000000";
                    unsigned char tmp_buf[640];
                    string2hex(ack, tmp_buf);
                    DMS_NET_CHANNEL_MOTION_DETECT str_dms_motion_detect;

                    //»ñÈ¡Éè±¸µÄÒôÊÓÆµÍ¨µÀÑ¹Ëõ²ÎÊý
                    str_dms_motion_detect.bEnable = 0;
                    t_msg_head->dvrip.dvrip_extlen = sizeof(CONFIG_MOTIONDETECT) * 16;   //Òª·¢»Ø16¸öÕâÑù½á¹¹Ìå´óÐ¡µÄÊý¾Ý

                    CONFIG_MOTIONDETECT str_motion_detect;
                    memset(&str_motion_detect, 0, sizeof(CONFIG_MOTIONDETECT));
                    PRINT_INFO("str_dms_motion_detect.bEnable:%d\n", str_dms_motion_detect.bEnable);
		            memcpy(&str_motion_detect, tmp_buf, sizeof(str_motion_detect));
                    memcpy(send_msg + DVRIP_HEAD_T_SIZE, &str_motion_detect, sizeof(CONFIG_MOTIONDETECT));  

                }
            }
            break;
        case 125://»ñÈ¡¹¤×÷±í
            {
                //×¥°ü»ñµÃ
                unsigned char ack[]="b30000589c040000636f6e66696700007d0000000000000004010000000000004448415601000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000001000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000001000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000001000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000001000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000001000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000001000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b00000000000000000000000000000000000000170000003b0000003b000000";
                int ret;
                ret = string2hex(ack, send_msg);
                t_msg_head->dvrip.dvrip_extlen = 1180;
            }
			#endif
        case 126://ÑÕÉ«ÅäÖÃ
            {
                PRINT_INFO("search_type:%d, to get color\n", search_type);
                GK_NET_IMAGE_CFG str_channel_color_info;

                //»ñÈ¡Éè±¸µÄÑÕÉ«²ÎÊý
                if(0 != goke_api_get_ioctrl(GET_IMAGE_CFG,&str_channel_color_info))
                {
                    PRINT_ERR("DMS_NET_GET_COLORCFG fail\n");
                    return rcv_msg_len;
                }  

                //Ä¿Ç°Ö»ÓÐÊý×é0ÔªËØÉúÐ§
                t_msg_head->dvrip.dvrip_extlen = sizeof(CONFIG_COLOR_OLD) * 16;   //Òª·¢»Ø16¸öÕâÑù½á¹¹Ìå´óÐ¡µÄÊý¾Ý
                CONFIG_COLOR_OLD str_color;
                memset(&str_color, 0, sizeof(CONFIG_COLOR_OLD));
                strcpy((char*)str_color.ColorVersion, "DHAV");
                str_color.ColorVersion[4] = 0xf0;
                str_color.Color[0].Sector.State = 2;
                str_color.Color[0].Sector.StartHour = 0;
                str_color.Color[0].Sector.StartMin = 0;
                str_color.Color[0].Sector.StartSec = 0;
                str_color.Color[0].Sector.EndHour = 0x18;
                str_color.Color[0].Sector.EndMin = 0;
                str_color.Color[0].Sector.EndSec = 0;         

                /*!< ÁÁ¶È	0-100 */
                str_color.Color[0].Brightness = str_channel_color_info.brightness;

                /*!< ¶Ô±È¶È	0-100 */
                str_color.Color[0].Contrast = str_channel_color_info.contrast;

                /*!< ±¥ºÍ¶È	0-100 */
                str_color.Color[0].Saturation = str_channel_color_info.saturation;

                /*!< É«¶È	0-100 */
                str_color.Color[0].Hue = str_channel_color_info.hue;
                str_color.Color[0].Gain = 50;				/*!< ÔöÒæ	0-100		*/ //½çÃæÉÏÃ»ÓÃµ½£¬ÔÝÊ±²»ÓÃ           

                PRINT_INFO("Brightness:%d,Contrast:%d,Saturation:%d,Hue:%d\n",
                        str_channel_color_info.brightness,
                        str_channel_color_info.contrast,
                        str_channel_color_info.saturation,
                        str_channel_color_info.hue);

                memcpy(&str_color.Color[1], &str_color.Color[0], sizeof(COLOR_PARAM));
                memcpy(send_msg + DVRIP_HEAD_T_SIZE, &str_color, sizeof(CONFIG_COLOR_OLD));
            }
            break;
        case 127://Ë«ÂëÁ÷Í¼ÏñÅäÖÃ
            {
                PRINT_INFO("search_type:%d, »ñµÃ±àÂëÅäÖÃ\n", search_type);
                int i = 0;
                GK_NET_VIDEO_CFG channel_pic_info;

                //»ñÈ¡Éè±¸µÄÒôÊÓÆµÍ¨µÀÑ¹Ëõ²ÎÊý
                if(0 != goke_api_get_ioctrl(GET_VIDEO_CFG,&channel_pic_info))
                {
                    PRINT_ERR("DH_NET_GET_SUPPORT_STREAM_FMT fail\n");
                    return rcv_msg_len;
                }  

                t_msg_head->dvrip.dvrip_extlen = sizeof(CONFIG_CAPTURE_OLD) * 16;  //Òª·¢»Ø16¸öÕâÑù½á¹¹Ìå´óÐ¡µÄÊý¾Ý
                CONFIG_CAPTURE_OLD str_captue;
                memset(&str_captue, 0, sizeof(CONFIG_CAPTURE_OLD));
                strcpy((char*)str_captue.CapVersion, "anni");

                int w,h;
                h = channel_pic_info.vencStream[0].h264Conf.height;
                w = channel_pic_info.vencStream[0].h264Conf.width;
                //Ö÷ÂëÁ÷ÅäÖÃ
                switch(w * h)
                {
                    case 1920 * 1080:
                        str_captue.MainOption[0].ImageSize = CAPTURE_SIZE_1080p;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    case 1080 * 960:
                        str_captue.MainOption[0].ImageSize = CAPTURE_SIZE_1_3M;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    case 1280 * 720:
                        str_captue.MainOption[0].ImageSize = CAPTURE_SIZE_720p;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    case 704 * 576:
                    case 704 * 480:
                        str_captue.MainOption[0].ImageSize = CAPTURE_SIZE_D1;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    case 640*480:
                    case 640*360:
                        str_captue.MainOption[0].ImageSize = CAPTURE_SIZE_VGA;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    case 352*576:
                    case 352*480:
                        str_captue.MainOption[0].ImageSize = CAPTURE_SIZE_HD1;
                        break;
                    case 352*288:
                    case 352*240:
                        str_captue.MainOption[0].ImageSize = CAPTURE_SIZE_CIF;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    case 320*240:
                        str_captue.MainOption[0].ImageSize = CAPTURE_SIZE_QVGA;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    case 176*144:
                    case 176*120:
                        str_captue.MainOption[0].ImageSize = CAPTURE_SIZE_QCIF;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    default:
                        PRINT_ERR("unknown w:%d,h:%d\n", w, h);
                        break;
                }

                if(0 == channel_pic_info.vencStream[0].h264Conf.rc_mode){// ¹Ì¶¨ÂëÁ÷
                    str_captue.MainOption[0].BitRateControl = CAPTURE_BRC_CBR;  /*!< ÂëÁ÷¿ØÖÆ ²ÎÕÕÃ¶¾Ùcapture_brc_t(DVRAPI.H) */
                    PRINT_INFO("CAPTURE_BRC_CBR\n");
                }else{
                    str_captue.MainOption[0].BitRateControl = CAPTURE_BRC_VBR;  /*!< ÂëÁ÷¿ØÖÆ ²ÎÕÕÃ¶¾Ùcapture_brc_t(DVRAPI.H) */
                    PRINT_INFO("CAPTURE_BRC_VBR\n");
                }
                switch(channel_pic_info.vencStream[0].h264Conf.quality)
                {
                    case 0:
                        str_captue.MainOption[0].ImgQlty = 1;  /*!< ÂëÁ÷µÄ»­ÖÊ µµ´Î1-6 6×îºÃ */
                        break;
                    case 1:
                        str_captue.MainOption[0].ImgQlty = 4;
                        break;
                    case 2:
                        str_captue.MainOption[0].ImgQlty = 5;
                        break;
                    case 3:
                        str_captue.MainOption[0].ImgQlty = 6;
                        break;
                    default:
                        str_captue.MainOption[0].ImgQlty = 4;
                        break;
                }
                PRINT_INFO("str_captue.MainOption[0].ImgQlty:%d\n", str_captue.MainOption[0].ImgQlty);
                str_captue.MainOption[0].Frames = channel_pic_info.vencStream[0].h264Conf.fps;    /*!< Ö¡ÂÊ¡¡µµ´ÎNÖÆ1-6,PÖÆ1-5 */
                str_captue.MainOption[0].AVEnable = 3;   /*!< ÒôÊÓÆµÊ¹ÄÜ 1Î»ÎªÊÓÆµ£¬2Î»ÎªÒôÆµ¡£ONÎª´ò¿ª£¬OFFÎª¹Ø±Õ */
                str_captue.MainOption[0].IFrameInterval = channel_pic_info.vencStream[0].h264Conf.gop;  /*!< IÖ¡¼ä¸ôÖ¡ÊýÁ¿£¬ÃèÊöÁ½¸öIÖ¡Ö®¼äµÄPÖ¡¸öÊý£¬0-149, 255±íÊ¾´Ë¹¦ÄÜ²»Ö§³ÖÉèÖÃ*/
                str_captue.MainOption[0].usBitRate = channel_pic_info.vencStream[0].h264Conf.bps/8;    //ÂëÂÊ µ¥Î»ÎªK
                PRINT_INFO("w:%d, h:%d, imagesize:%d, fps:%d, gop:%d, bps:%d\n", w,h,str_captue.MainOption[0].ImageSize, 
                        str_captue.MainOption[0].Frames, 
                        str_captue.MainOption[0].IFrameInterval, str_captue.MainOption[0].usBitRate);

                for(i = 1; i < 3; i++)
                {
                    memcpy(&str_captue.MainOption[i], &str_captue.MainOption[0], sizeof(ENCODE_OPTION));
                }

                //×ÓÂëÁ÷ÅäÖÃ
                h = channel_pic_info.vencStream[1].h264Conf.height;
                w = channel_pic_info.vencStream[1].h264Conf.width;
                switch(w * h)
                {
                    case 1920 * 1080:
                        str_captue.AssiOption[0].ImageSize = CAPTURE_SIZE_1080p;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    case 1080 * 960:
                        str_captue.AssiOption[0].ImageSize = CAPTURE_SIZE_1_3M;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    case 1280 * 720:
                        str_captue.AssiOption[0].ImageSize = CAPTURE_SIZE_720p;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    case 704 * 576:
                    case 704 * 480:
                        str_captue.AssiOption[0].ImageSize = CAPTURE_SIZE_D1;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    case 640*480:
                    case 640*360:
                        str_captue.AssiOption[0].ImageSize = CAPTURE_SIZE_VGA;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    case 352*576:
                    case 352*480:
                        str_captue.AssiOption[0].ImageSize = CAPTURE_SIZE_HD1;
                        break;
                    case 352*288:
                    case 352*240:
                        str_captue.AssiOption[0].ImageSize = CAPTURE_SIZE_CIF;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    case 320*240:
                        str_captue.AssiOption[0].ImageSize = CAPTURE_SIZE_QVGA;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    case 176*144:
                    case 176*120:
                        str_captue.AssiOption[0].ImageSize = CAPTURE_SIZE_QCIF;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    default:
                        PRINT_ERR("unknown enc  w:%d,h:%d\n", w, h);
                        break;
                }

                if(0 == channel_pic_info.vencStream[1].h264Conf.rc_mode){// ¹Ì¶¨ÂëÁ÷
                    str_captue.AssiOption[0].BitRateControl = CAPTURE_BRC_CBR;  /*!< ÂëÁ÷¿ØÖÆ ²ÎÕÕÃ¶¾Ùcapture_brc_t(DVRAPI.H) */
                    PRINT_INFO("CAPTURE_BRC_CBR\n");
                }else{
                    str_captue.AssiOption[0].BitRateControl = CAPTURE_BRC_VBR;  /*!< ÂëÁ÷¿ØÖÆ ²ÎÕÕÃ¶¾Ùcapture_brc_t(DVRAPI.H) */
                    PRINT_INFO("CAPTURE_BRC_VBR\n");
                }
                switch(channel_pic_info.vencStream[1].h264Conf.quality)
                {
                    case 0:
                        str_captue.AssiOption[0].ImgQlty = 1;  /*!< ÂëÁ÷µÄ»­ÖÊ µµ´Î1-6 6×îºÃ */
                        break;
                    case 1:
                        str_captue.AssiOption[0].ImgQlty = 4;
                        break;
                    case 2:
                        str_captue.AssiOption[0].ImgQlty = 5;
                        break;
                    case 3:
                        str_captue.AssiOption[0].ImgQlty = 6;
                        break;
                    default:
                        break;
                }
                PRINT_INFO("str_captue.MainOption[0].ImgQlty:%d\n", str_captue.MainOption[0].ImgQlty);
                str_captue.AssiOption[0].Frames = channel_pic_info.vencStream[1].h264Conf.fps;    /*!< Ö¡ÂÊ¡¡µµ´ÎNÖÆ1-6,PÖÆ1-5 */
                str_captue.AssiOption[0].AVEnable = 3;   /*!< ÒôÊÓÆµÊ¹ÄÜ 1Î»ÎªÊÓÆµ£¬2Î»ÎªÒôÆµ¡£ONÎª´ò¿ª£¬OFFÎª¹Ø±Õ */
                str_captue.AssiOption[0].IFrameInterval = channel_pic_info.vencStream[1].h264Conf.gop;  /*!< IÖ¡¼ä¸ôÖ¡ÊýÁ¿£¬ÃèÊöÁ½¸öIÖ¡Ö®¼äµÄPÖ¡¸öÊý£¬0-149, 255±íÊ¾´Ë¹¦ÄÜ²»Ö§³ÖÉèÖÃ*/
                str_captue.AssiOption[0].usBitRate = channel_pic_info.vencStream[1].h264Conf.bps/ 8;    //ÂëÂÊ µ¥Î»ÎªK
                PRINT_INFO("w:%d, h:%d, imagesize:%d, fps:%d, gop:%d, bps:%d\n", w,h,str_captue.AssiOption[0].ImageSize,
                        str_captue.AssiOption[0].Frames,
                        str_captue.AssiOption[0].IFrameInterval, str_captue.AssiOption[0].usBitRate);

                for(i = 1; i < 3; i++)
                {
                    memcpy(&str_captue.AssiOption[i], &str_captue.AssiOption[0], sizeof(ENCODE_OPTION));
                }


                str_captue.Compression = CAPTURE_COMP_H264;
                //Êµ¼ÊÀ´¿´´Ë´¦µÄcoverÃ»ÓÐÓÃ
                //GK_NET_SHELTER_RECT cover;
                //if(0 == goke_api_get_ioctrl(GET_COVER_CFG, &cover))
                {
                    str_captue.CoverEnable = 0x11;
                    //str_captue.CoverEnable = cover.enable;
                    str_captue.Cover.left = 0;
                    str_captue.Cover.top = 0;
                    str_captue.Cover.right = 5120;
                    str_captue.Cover.bottom = 5120;
                }

                GK_NET_CHANNEL_INFO channel_info;
                channel_info.id = 0;
                if(0 == goke_api_get_ioctrl(GET_OSD_CFG, &channel_info))
                {

                    str_captue.TimeTitle.TitlefgRGBA = 0xffffff;			/*!< ±êÌâµÄÇ°¾°RGB£¬ºÍÍ¸Ã÷¶È */
                    str_captue.TimeTitle.TitlebgRGBA = 0x80000000;		/*!< ±êÌâµÄºó¾°RGB£¬ºÍÍ¸Ã÷¶È*/
                    str_captue.TimeTitle.TitleLeft = channel_info.osdDatetime.x * 8192;			/*!< ±êÌâ¾à×ó±ßµÄ¾àÀëÓëÕû³¤µÄ±ÈÀý*8192 */
                    str_captue.TimeTitle.TitleTop =  channel_info.osdDatetime.x * 8192;				/*!< ±êÌâµÄÉÏ±ßµÄ¾àÀëÓëÕû³¤µÄ±ÈÀý*8192 */
                    str_captue.TimeTitle.TitleRight = channel_info.osdDatetime.y * 8192;;				/*!< ±êÌâµÄÓÒ±ßµÄ¾àÀëÓëÕû³¤µÄ±ÈÀý*8192 */
                    str_captue.TimeTitle.TitleBottom = channel_info.osdDatetime.y * 8192;;				/*!< ±êÌâµÄÏÂ±ßµÄ¾àÀëÓëÕû³¤µÄ±ÈÀý*8192 */        
                    str_captue.TimeTitle.TitleEnable = channel_info.osdDatetime.enable;				/*!< ±êÌâÊ¹ÄÜ */    

                    str_captue.ChannelTitle.TitlefgRGBA = 0xffffff;			/*!< ±êÌâµÄÇ°¾°RGB£¬ºÍÍ¸Ã÷¶È */
                    str_captue.ChannelTitle.TitlebgRGBA = 0x80000000;		/*!< ±êÌâµÄºó¾°RGB£¬ºÍÍ¸Ã÷¶È*/
                    str_captue.ChannelTitle.TitleLeft = channel_info.osdChannelName.x * 8192;			/*!< ±êÌâ¾à×ó±ßµÄ¾àÀëÓëÕû³¤µÄ±ÈÀý*8192 */
                    str_captue.ChannelTitle.TitleTop = channel_info.osdChannelName.x * 8192;				/*!< ±êÌâµÄÉÏ±ßµÄ¾àÀëÓëÕû³¤µÄ±ÈÀý*8192 */
                    str_captue.ChannelTitle.TitleRight = channel_info.osdChannelName.y * 8192;				/*!< ±êÌâµÄÓÒ±ßµÄ¾àÀëÓëÕû³¤µÄ±ÈÀý*8192 */
                    str_captue.ChannelTitle.TitleBottom = channel_info.osdChannelName.y * 8192;				/*!< ±êÌâµÄÏÂ±ßµÄ¾àÀëÓëÕû³¤µÄ±ÈÀý*8192 */        
                    str_captue.ChannelTitle.TitleEnable = channel_info.osdChannelName.enable;				/*!< ±êÌâÊ¹ÄÜ */    
                }

                //»ñÈ¡Éè±¸×¥ÅÄ²ÎÊý

                //×¥ÅÄ²ÎÊýnvrÆäÊµÃ»ÓÐÓÃµ½£¬
                // »­Ãæ´óÐ¡£»0£ºQCIF£¬1£ºCIF£¬2£ºD1
                h = channel_pic_info.vencStream[0].h264Conf.height;
                w = channel_pic_info.vencStream[0].h264Conf.width;
                //Ö÷ÂëÁ÷ÅäÖÃ
                switch(w * h)
                {
                    case 1920 * 1080:
                        str_captue.PicOption[0].ImageSize = CAPTURE_SIZE_1080p;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    case 1080 * 960:
                        str_captue.PicOption[0].ImageSize = CAPTURE_SIZE_1_3M;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    case 1280 * 720:
                        str_captue.PicOption[0].ImageSize = CAPTURE_SIZE_720p;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    case 704 * 576:
                    case 704 * 480:
                        str_captue.PicOption[0].ImageSize = CAPTURE_SIZE_D1;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    case 640*480:
                    case 640*360:
                        str_captue.PicOption[0].ImageSize = CAPTURE_SIZE_VGA;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    case 352*576:
                    case 352*480:
                        str_captue.PicOption[0].ImageSize = CAPTURE_SIZE_HD1;
                        break;
                    case 352*288:
                    case 352*240:
                        str_captue.PicOption[0].ImageSize = CAPTURE_SIZE_CIF;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    case 320*240:
                        str_captue.PicOption[0].ImageSize = CAPTURE_SIZE_QVGA;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    case 176*144:
                    case 176*120:
                        str_captue.PicOption[0].ImageSize = CAPTURE_SIZE_QCIF;   /*!< ·Ö±æÂÊ ²ÎÕÕÃ¶¾Ùcapture_size_t(DVRAPI.H) */
                        break;
                    default:
                        PRINT_ERR("unknown enc w:%d,h:%d\n", w, h);
                        break;
                }

                str_captue.PicOption[0].BitRateControl = CAPTURE_BRC_CBR;  /*!< ÂëÁ÷¿ØÖÆ ²ÎÕÕÃ¶¾Ùcapture_brc_t(DVRAPI.H) */
                switch(channel_pic_info.vencStream[0].h264Conf.quality)
                {
                    case 0:
                        str_captue.PicOption[0].ImgQlty = 1;  /*!< ÂëÁ÷µÄ»­ÖÊ µµ´Î1-6 6×îºÃ */
                        break;
                    case 1:
                        str_captue.PicOption[0].ImgQlty = 4;
                        break;
                    case 2:
                        str_captue.PicOption[0].ImgQlty = 5;
                        break;
                    case 3:
                        str_captue.PicOption[0].ImgQlty = 6;
                        break;
                    default:
                        break;
                }        
                str_captue.PicOption[0].Frames = 15;    /*!< Ö¡ÂÊ¡¡µµ´ÎNÖÆ1-6,PÖÆ1-5 */
                str_captue.PicOption[0].AVEnable = 1;   /*!< ÒôÊÓÆµÊ¹ÄÜ 1Î»ÎªÊÓÆµ£¬2Î»ÎªÒôÆµ¡£ONÎª´ò¿ª£¬OFFÎª¹Ø±Õ */
                str_captue.PicOption[0].IFrameInterval = 50;  /*!< IÖ¡¼ä¸ôÖ¡ÊýÁ¿£¬ÃèÊöÁ½¸öIÖ¡Ö®¼äµÄPÖ¡¸öÊý£¬0-149, 255±íÊ¾´Ë¹¦ÄÜ²»Ö§³ÖÉèÖÃ*/
                str_captue.PicOption[0].usBitRate = 4096;  

                str_captue.Volume = 3;
                str_captue.VolumeEnable = 0; 

                memcpy(send_msg + DVRIP_HEAD_T_SIZE, &str_captue, sizeof(CONFIG_CAPTURE_OLD));          
                break;
            }
        default:
            PRINT_ERR("search_type:%d, not support\n", search_type);
            break;
    }
    //´¦Àí39 Íø¿¨ÅäÖÃ 
    ZhiNuo_SockSend(sock, send_msg, DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen, 100);

    return rcv_msg_len;
}

/*************************************************************
 * º¯Êý½éÉÜ£º²éÑ¯Í¨µÀÃû³Æ
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/

#define CHANNEL_NAME_LEN 32
int ZhiNuo_Search_Channel_Name(char* bufPost, DAHUA_SESSION_CTRL *sock)
{
    int ret = 0, i, offset;
    int rcv_msg_len = 0;
    int channel_name_type = 0;
    char tem_buf[DVRIP_HEAD_T_SIZE] = {0};
    char strName[CHANNEL_NAME_LEN];	//!Ò»¸öÍ¨µÀµÄÍ¨µÀÃû³Æ    
    GK_NET_CHANNEL_INFO channelInfo;
    char send_msg[BUFLEN] = {0};               

    memcpy(tem_buf, bufPost, DVRIP_HEAD_T_SIZE);
    DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)tem_buf;    
    PRINT_INFO("hl:%d, v:%d, vextralen:%d\n", t_msg_head->dvrip.dvrip_hl, 
            t_msg_head->dvrip.dvrip_v, t_msg_head->dvrip.dvrip_extlen);

    //²éÑ¯Í¨µÀÃû³ÆÀà±ð 0:ÀÏµÄÍ¨µÀÐ­Òé 1:ÐÂµÄ×Ö·û´®ÐÎÊ½µÄÐ­Òé
    PRINT_INFO("Search_Channel_Name_type:%d\n", t_msg_head->c[8]);
    channel_name_type = t_msg_head->c[8];
    rcv_msg_len = DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen;
    ZERO_DVRIP_HEAD_T(t_msg_head)
    t_msg_head = (DVRIP_HEAD_T *)send_msg; 


    //»ñÈ¡Í¨µÀÃû
    channelInfo.id = 0;
    ret = goke_api_get_ioctrl(GET_OSD_CFG, &channelInfo);
    if((0 == ret) && (strlen(channelInfo.osdChannelName.text) > 0))
    {
        sprintf(strName, "%s", channelInfo.osdChannelName.text); 
    } else{
        sprintf(strName, " ");      
    }
    offset = DVRIP_HEAD_T_SIZE;
    for(i = 0; i < 16; i ++)
    {
        memcpy(send_msg + offset, strName, CHANNEL_NAME_LEN); 
        offset += CHANNEL_NAME_LEN;
    }
    t_msg_head->dvrip.dvrip_extlen = CHANNEL_NAME_LEN * 16; //°Ñ×Ö·û´®×îºóÄÇÁ½¸ö&&·ûºÅ·¢³öÈ¥
    PRINT_INFO("channel name:%s, len:%d\n", send_msg + DVRIP_HEAD_T_SIZE, strlen(send_msg + DVRIP_HEAD_T_SIZE));
    t_msg_head->dvrip.cmd = ACK_CHANNEL_NAME_SEARCH;   
    t_msg_head->c[8] = channel_name_type;

    ZhiNuo_SockSend(sock, send_msg, DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen, 100);

    return rcv_msg_len;
}

/*************************************************************
 * º¯Êý½éÉÜ£º²éÑ¯ÏµÍ³ÐÅÏ¢
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_Search_System_Info(char* bufPost, DAHUA_SESSION_CTRL *sock)
{
    int rcv_msg_len = 0;
    char search_type = 0;
    char ex_type = 0;
    int function_capacity = 0;
    char tem_buf[DVRIP_HEAD_T_SIZE] = {0};

    memcpy(tem_buf, bufPost, DVRIP_HEAD_T_SIZE);
    DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)tem_buf;    
    PRINT_INFO("hl:%d, v:%d, vextralen:%d\n", t_msg_head->dvrip.dvrip_hl, 
            t_msg_head->dvrip.dvrip_v, t_msg_head->dvrip.dvrip_extlen);

    //31:¹¦ÄÜÄÜÁ¦²éÑ¯£¬W20=¹¦ÄÜ±àºÅ£¬²Î¼û±à³Ì·þÎñÊÖ²á
    PRINT_INFO("Search_type:%d, capacity:%d\n", t_msg_head->c[8], t_msg_head->c[20]);
    search_type = t_msg_head->c[8];
    function_capacity = t_msg_head->c[20];
    ex_type = t_msg_head->c[12];

    rcv_msg_len = DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen;
    char send_msg[BUFLEN] = {0};
    t_msg_head = (DVRIP_HEAD_T *)send_msg;                
    ZERO_DVRIP_HEAD_T(t_msg_head)

    //v6 ·¢ÁË 1£¬ 7 £¬2
    switch(search_type)
    {
#if ADD_MD_INFO
        case 1:
            {
                PRINT_INFO("search_type:%d, ²éÑ¯Éè±¸ÊôÐÔÐÅÏ¢\n", search_type);
                //×¥°ü»ñµÃ
                unsigned char ack[]="b400005820000000010000000000000000000000000000000000000000000000022801010101040102010000dd070c1f0000332e320000000000000000000000";
                int ret, i;
                ret = string2hex(ack, send_msg);
                for(i = 0; i < ret; i++)
                {
                    printf("%02x", send_msg[i]);
                }
                printf("\n");
            }
            break;
        case 7:
            {
                PRINT_INFO("search_type:%d, ²éÑ¯Éè±¸IDºÅ\n", search_type);
                //×¥°ü»ñµÃ
                unsigned char ack[]="b40000580f000000080000000000000000000000000000000000000000000000322e3430302e303030302e302e5200";
                int ret, i;
                ret = string2hex(ack, send_msg);
                for(i = 0; i < ret; i++)
                {
                    printf("%02x", send_msg[i]);
                }
                printf("\n");
            }
            break;
        case 8:
            {
                PRINT_INFO("search_type:%d, ²éÑ¯Éè±¸°æ±¾\n", search_type);
                //×¥°ü»ñµÃ
                unsigned char ack[]="b40000580f000000080000000000000000000000000000000000000000000000322e3430302e303030302e302e5200";
                int ret, i;
                ret = string2hex(ack, send_msg);
                for(i = 0; i < ret; i++)
                {
                    printf("%02x", send_msg[i]);
                }
                printf("\n");
            }
            break;
        case 14:
            {
                PRINT_INFO("search_type:%d, ²éÑ¯ÊÓÆµ¶¯Ì¬¼ì²âÊôÐÔÐÅÏ¢\n", search_type);
                MOTION_DETECT_CAPS motion_detect;
                memset(&motion_detect, 0, sizeof(motion_detect));
                motion_detect.Enabled = 1;
                motion_detect.GridLines = 22;
                motion_detect.GridRows = 18;
                motion_detect.Result = 1;
                motion_detect.Hint = 1;
                t_msg_head->dvrip.dvrip_extlen = sizeof(motion_detect);
                memcpy(send_msg + DVRIP_HEAD_T_SIZE, &motion_detect, sizeof(motion_detect));
                //test
                /*
                    //×¥°ü»ñµÃ
                   unsigned char ack[]="b4000058100000000e000000000000000000000000000000000000000000000001000000160000001200000001000000";
                   int ret;
                   ret = string2hex(ack, send_msg);
                   t_msg_head->dvrip.dvrip_extlen = ret - 32;
                   PRINT_ERR("ack a4 14 len:%d\n", ret-32);
                 */
            }
            break;
#endif
        case 15:
            {
                PRINT_INFO("search_type:%d, ²éÑ¯ÊÓÆµÇøÓòÕÚµ²ÊôÐÔÐÅÏ¢\n", search_type);

                VIDEO_COVER_CAPS_T video_cover;
                memset(&video_cover, 0, sizeof(video_cover));
                video_cover.iEnabled = 1;
                video_cover.iBlockNum = 4;
                t_msg_head->dvrip.dvrip_extlen = sizeof(video_cover);
                memcpy(send_msg + DVRIP_HEAD_T_SIZE, &video_cover, sizeof(video_cover));
            }
            break;
        case 16:
            {
                //²éÑ¯ÉãÏñÍ·ÊôÐÔÐÅÏ¢
                PRINT_INFO("search_type:%d, to get cam color\n", search_type);
                CAM_CAPS cam_caps;

                t_msg_head->dvrip.dvrip_extlen = sizeof(cam_caps);
                memset(&cam_caps, 1, sizeof(cam_caps));
                cam_caps.iBrightnessEn = 1;
                cam_caps.iContrastEn = 1;
                cam_caps.iColorEn = 1;
                cam_caps.iGainEn = 1;
                cam_caps.iSaturationEn = 1;
                cam_caps.iBacklightEn = 1;
                cam_caps.iExposureEn = 1;
                cam_caps.iColorConvEn = 1;
                cam_caps.iAttrEn = 1;
                cam_caps.iMirrorEn = 1;
                cam_caps.iFlipEn = 1;
                cam_caps.iWhiteBalance = 1;
                cam_caps.iSignalFormatMask = 0;
                memcpy(send_msg + DVRIP_HEAD_T_SIZE, &cam_caps, sizeof(cam_caps));  
            }
            break;
        case 26:
            {//²éÑ¯Éè±¸¹¦ÄÜÁÐ±í
                PRINT_INFO("search_type:%d, ftp,alarm...\n", search_type);
                t_msg_head->dvrip.dvrip_extlen = strlen(DEV_FUNCTION_SEARCH);
                strcpy(send_msg + DVRIP_HEAD_T_SIZE, DEV_FUNCTION_SEARCH);   
            }
            break;
        case 31:
            {//31:¹¦ÄÜÄÜÁ¦²éÑ¯
                PRINT_INFO("search_type:%d, ²éÑ¯¹¦ÄÜÄÜÁ¦²éÑ¯\n", search_type);
                //²éÑ¯µÄ×¥ÅÄ²ÎÊýnvrÔÝÊ±Ã»ÓÐÓÃµ½£¬¾ßÌåÖµÒÔºó¿ÉÒÔÔÙ¸ü¸Ä
                t_msg_head->dvrip.dvrip_extlen = 
                    strlen("SNAP&1&1::SIZE:3:4:5:6:8::FREQUENCE:2:3:4:5:6:7:8:9::MODE:0::FORMAT:1::QUALITY:4&1");
                strcpy(send_msg + DVRIP_HEAD_T_SIZE, 
                        "SNAP&1&1::SIZE:3:4:5:6:8::FREQUENCE:2:3:4:5:6:7:8:9::MODE:0::FORMAT:1::QUALITY:4&1");    
            }
            break;
        case 32://ÊÓÆµÇ°¶Ë²É¼¯ÄÜÁ¦²éÑ¯
            {
                PRINT_INFO("search_type:%d, ²éÑ¯ÊÓÆµÇ°¶Ë²É¼¯ÄÜÁ¦\n", search_type);
                Prevideo_Capture capture[3];
                memset(&capture, 0, sizeof(capture));
                capture[0].iCifCaps = CAPTURE_SIZE_720p;
                capture[0].iFrameCaps = 25;
                capture[1].iCifCaps = CAPTURE_SIZE_D1;
                capture[1].iFrameCaps = 25;
                capture[2].iCifCaps = CAPTURE_SIZE_VGA;
                capture[2].iFrameCaps = 25;
                t_msg_head->dvrip.dvrip_extlen = sizeof(capture);
                memcpy(send_msg + DVRIP_HEAD_T_SIZE, &capture, sizeof(capture));
            }
            break;
        default:
#if DEBUG
            PRINT_ERR("comd A4 unknown type:%d\n", search_type);
#endif
            break;
    }
        


    t_msg_head->dvrip.cmd = ACK_SYSTEM_INFO_SEARCH;   
    t_msg_head->c[8] = search_type; 
    t_msg_head->c[12] = ex_type;  //Õâ¸öÔÝÊ±Ã»ÓÃ£¬µ«ÊÇÖÇÅµÒªÇó×îºÃÌîÉÏ¡£
    t_msg_head->c[16] = 0; //·µ»ØÂë 0:Õý³£ 1:ÎÞÈ¨ÏÞ 5:ÎÞ¶ÔÓ¦ÐÅÏ¢Ìá¹©
    t_msg_head->c[20] = function_capacity; //¹¦ÄÜÄÜÁ¦ºÅ

    ZhiNuo_SockSend(sock, send_msg, DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen, 100);

    return rcv_msg_len;
}



/*************************************************************
 * º¯Êý½éÉÜ£ºÇëÇó½¨Á¢Á¬½Ó¹ØÏµ
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int Dahua_ConnectRequest(char* bufPost, DAHUA_SESSION_CTRL* pCtrl)
{
    int n_flag = 0;
    char ch_request_type = 0;
    char ch_channel_num = 0;
    char tem_buf[DVRIP_HEAD_T_SIZE] = {0};
    memcpy(tem_buf, bufPost, DVRIP_HEAD_T_SIZE);
    DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)tem_buf;    

    PRINT_INFO("hl:%d, v:%d, vextralen:%d\n", t_msg_head->dvrip.dvrip_hl, 
            t_msg_head->dvrip.dvrip_v, t_msg_head->dvrip.dvrip_extlen);

    //ÇëÇóÀàÐÍ(1ÎªÊµÊ±¼à¿Ø£¬2£¬¶à»­ÃæÔ¤ÀÀ£¬3£¬ÓïÒô¶Ô½²£¬4£¬ÏÂÔØ»Ø·Å£©
    PRINT_INFO("id:%d, requeset_type:%d, channel_num:%d\n", t_msg_head->l[2], t_msg_head->c[12], t_msg_head->c[13]);
    ch_request_type = t_msg_head->c[12];
    ch_channel_num = t_msg_head->c[13];
    n_flag = t_msg_head->l[2];

    char send_msg[BUFLEN] = {0};
    DVRIP_HEAD_T *t_resp_msg_head = (DVRIP_HEAD_T *)send_msg;                
    ZERO_DVRIP_HEAD_T(t_resp_msg_head)
    t_resp_msg_head->dvrip.cmd = ACK_CONNECT_REQUEST;   
    t_resp_msg_head->l[2] = n_flag; //µÇÂ½µÄÎ¨Ò»±êÊ¶ºÅ,Ò²¾ÍÊÇ0xb0·µ»ØµÄÄÇ¸ö ´ó»ªNVRÊ¼ÖÕÊÇ46
    t_resp_msg_head->c[12] = ch_request_type; //ÇëÇóÀàÐÍ
    t_resp_msg_head->c[13] = ch_channel_num; //ÇëÇóÍ¨µÀºÅ

    t_resp_msg_head->c[14] = 0; //·µ»ØÂë 0:Á¬½ÓÓ³Éä³É¹¦ 1:Ê§°Ü 2:×ÓÁ´½ÓÒÑ½¨Á¢

    //bruce
    //ch_request_type, 1 Ö÷ÂëÁ÷£¬ 2 ´ÎÂëÁ÷
    PRINT_ERR("ch_request_type:%d n_flag:%d\n", ch_request_type, n_flag);
#if 0
    //´ó»ªµÄÉè±¸Ö»´«ÊäÖ÷ÂëÁ÷
    if((2 == ch_request_type) && (DAHUA_NVR == n_flag))
    {
        t_msg_head->c[14] = 1;
        ZhiNuo_SockSend(sock, send_msg, DVRIP_HEAD_T_SIZE, 100);
        sock->n_flag = -1;
        PRINT_ERR("closesocket dahua socket\n");
        return rcv_msg_len;        
    }
#endif

    #if MULTI_CONNECT
	/* ½«µ±Ç°socketÌí¼Ó½øÊý×é£¬ÓÃÀ´×öÃ½ÌåÁ÷´«Êä */
    int ret = UtAddLvSock(pCtrl->fd, n_flag, ch_request_type - 1, &g_dahua_session);
	if(ret == -1) { // ³ö´í£¬ÕÒ²»µ½¶ÔÓ¦µÄsession_id
		PRINT_ERR("add lv sock err. data_sock:%d, sid:%d, stream_type:%d", 
            pCtrl->fd, n_flag, ch_request_type - 1);                         
		t_resp_msg_head->c[14] = 1;
	}
    #else
    if (ch_request_type == 1)
        main_sock = pCtrl->fd;
    else if (ch_request_type == 2)
        sub_sock = pCtrl->fd;
    #endif

    ZhiNuo_SockSend(pCtrl, send_msg, DVRIP_HEAD_T_SIZE, 100);

    pCtrl->fSessionType = CONNECT_SESSION;
    return 0;
}

#if 1
static int dahua_parse_fields(char *str, DahuaMethodField *p)
{
    char opt1[100] = {0};
    char opt2[1000] = {0};
    //unsigned int tmp_i = 0;

    sscanf(str, "%[^:]:%s", opt1, opt2);

    if (!strcmp(opt1, "TransactionID")) {
        p->transaction_id = (unsigned int)strtoul(opt2, NULL, 0);
    } else if (!strcmp(opt1, "SessionID")) {
        p->sid = (unsigned int)strtoul(opt2, NULL, 0);
    } else if (!strcmp(opt1, "ConnectionID")) {
        p->connect_id = (unsigned int)strtoul(opt2, NULL, 0);
    } else if (!strcmp(opt1, "Method")) {
        sprintf(p->method, "%s", opt2);
    } else if (!strcmp(opt1, "ParameterName")) {
        sprintf(p->parameter_name, "%s", opt2);
    } else if (!strcmp(opt1, "ConnectProtocol")) {
        p->connect_protocol = (unsigned int)strtoul(opt2, NULL, 0);
    } else if (!strcmp(opt1, "channel")) {
        p->channel = (unsigned int)strtoul(opt2, NULL, 0);
    } else if (!strcmp(opt1, "stream")) {
        p->stream = (unsigned int)strtoul(opt2, NULL, 0);
    } else if (!strcmp(opt1, "state")) {
        p->state = (unsigned int)strtoul(opt2, NULL, 0);
    }
    
    return 0;
}

static int dahua_parse_msg(char *exbuf, DahuaMethodField *pField)
{
    char *src = exbuf;
    char *p;
    int len = 0;

    while (1) {
        if (!strcmp(src, "\r\n"))
            break;

        p = strstr(src, "\r\n");
        len = p - src;

        char tmpbuf[1000] = {0};
        strncpy(tmpbuf, src, len);
        tmpbuf[len] = '\0';
        dahua_parse_fields(tmpbuf, pField);

        src = p + 2;
    }

    return 0;
}


static void dahua_print_fields(DahuaMethodField *p)
{
    printf("***************************\n");
    printf("TransactionID:%u\n", p->transaction_id);
    printf("Method:%s\n", p->method);
    printf("ParameterName:%s\n", p->parameter_name);
    printf("SessionID:%u\n", p->sid);
    printf("ConnectionID:%u\n", p->connect_id);
    printf("***************************\n");
}



#endif

int Dahua_ControlConnect(char* bufPost, DAHUA_SESSION_CTRL* pCtrl)
{
    int ret = 0;
    DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)bufPost;

    char exbuf[10240] = {0};
    memcpy(exbuf, (char *)(bufPost + DVRIP_HEAD_T_SIZE), t_msg_head->dvrip.dvrip_extlen);

    printf("***************************\n");
    PRINT_INFO("exlen:%d, exdata:%s\n", t_msg_head->dvrip.dvrip_extlen, exbuf);
    printf("***************************\n");

    DahuaMethodField m_f;
    memset(&m_f, 0, sizeof(DahuaMethodField));
    dahua_parse_msg(exbuf, &m_f);
    //dahua_print_fields(&m_f);

    PRINT_INFO("sock:%d, sid:%d, ip:%s\n", pCtrl->fd, pCtrl->fSessionInt, pCtrl->c_ip);
    
    // »Ø¸´ÏûÏ¢
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    char *ack_ex_data = pCtrl->fSendbuf + DVRIP_HEAD_T_SIZE;
    if (!strcmp(m_f.parameter_name, "Dahua.Device.Network.Monitor.General")) {
        // step 3 : start stream
        sprintf(ack_ex_data, "TransactionID:%u\r\nMethod:%sResponse\r\nParameterName:%s\r\nFaultCode:OK\r\n\r\n",
            m_f.transaction_id, m_f.method, m_f.parameter_name);

        #if MULTI_CONNECT
        //start stream
        PRINT_INFO("sock:%d, sid:%d, start stream%d, state:%d, ip:%s\n", 
            pCtrl->fd, pCtrl->fSessionInt, m_f.stream, m_f.state, pCtrl->c_ip);
        if (m_f.stream == UT_MAIN_STREAM) {
            //°Ñ tmp_sock Ìí¼Ó³É main_stream_sock, ²¢´ò¿ªÂëÁ÷
            UtSetLvSockStatus2(pCtrl->fSessionInt, 1, UT_MAIN_STREAM, &g_dahua_session);
        } else {
            //°Ñ tmp_sock Ìí¼Ó³É sub_stream_sock, ²¢´ò¿ªÂëÁ÷
            UtSetLvSockStatus2(pCtrl->fSessionInt, 1, UT_SUB_STREAM, &g_dahua_session);
        }
        #endif
    } else if (!strcmp(m_f.parameter_name, "Dahua.Device.Network.ControlConnection.Passive")) {
        // step 1
        m_f.connect_id = UtGenConnectId(&g_dahua_session);
        
        sprintf(ack_ex_data, "TransactionID:%u\r\nMethod:%sResponse\r\nParameterName:%s\r\nFaultCode:OK\r\nIP:0.0.0.0\r\nPort:%d\r\nConnectionID:%u\r\n\r\n",
            m_f.transaction_id, m_f.method, m_f.parameter_name, DAHUA_TCP_LISTEN_PORT, m_f.connect_id);
    } else if (!strcmp(m_f.parameter_name, "Dahua.Device.Network.ControlConnection.AckSubChannel")) {
        // step 2
        sprintf(ack_ex_data, "TransactionID:%u\r\nMethod:%sResponse\r\nParameterName:%s\r\nSessionID:%u\r\nConnectionID:%u\r\nFaultCode:OK\r\n\r\n",
            m_f.transaction_id, m_f.method, m_f.parameter_name, m_f.sid, m_f.connect_id);

        // °Ñsock Í¨¹ý sid ºÍÃüÁîsockÁªÏµÔÚÒ»Æð
        #if MULTI_CONNECT
        /* ½«µ±Ç°socketÌí¼Ó½øÊý×é£¬ÓÃÀ´×öÃ½ÌåÁ÷´«Êä
         * µ±Ç°»¹²»È·¶¨ÊÇÓÃÓÚ´«ÊäÖ÷ÂëÁ÷»¹ÊÇ×ÓÂëÁ÷£¬¶àÒ»¸öconnection_id
         */
        int ret = UtAddLvSock2(pCtrl->fd, pCtrl->c_ip, m_f.sid, m_f.connect_id, &g_dahua_session);
        if(ret == -1) { // ³ö´í£¬ÕÒ²»µ½¶ÔÓ¦µÄsession_id
            PRINT_ERR("add lv sock err. data_sock:%d, ip:%s, sid:%d, connect_id:%d", pCtrl->fd, pCtrl->c_ip, m_f.sid, m_f.connect_id);                         
            close(pCtrl->fd);
            return -1;
        }
        pCtrl->fSessionType = CONNECT_SESSION;

        #endif
        
    }
    PRINT_INFO("ack_ex_data:%s", ack_ex_data);

  	// »Ø¸´ÏûÏ¢¼Ó°üÍ·
    DVRIP_HEAD_T *pMsgHeader = (DVRIP_HEAD_T *)pCtrl->fSendbuf;     
    ZERO_DVRIP_HEAD_T(pMsgHeader)
        pMsgHeader->dvrip.cmd = ACK_CONTROL_CONNECT;  
    pMsgHeader->dvrip.dvrip_extlen = strlen(ack_ex_data);


	//·¢ËÍ
    ret = ZhiNuo_SockSend(pCtrl, pCtrl->fSendbuf, strlen(ack_ex_data) + DVRIP_HEAD_T_SIZE, 100);
    if (ret < 0) {
        PRINT_ERR();
		return -1;	
    }

    return 0;
}


/*************************************************************
 * º¯Êý½éÉÜ£ºÇ¿ÖÆIÖ¡ÇëÇó
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_I_Frame_Request(char* bufPost, DAHUA_SESSION_CTRL * pCtrl)
{
    int rcv_msg_len = 0;
    char tem_buf[DVRIP_HEAD_T_SIZE] = {0};
    memcpy(tem_buf, bufPost, DVRIP_HEAD_T_SIZE);
    DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)tem_buf;    

    PRINT_INFO("hl:%d, v:%d, vextralen:%d\n", t_msg_head->dvrip.dvrip_hl, 
            t_msg_head->dvrip.dvrip_v, t_msg_head->dvrip.dvrip_extlen);

    //ÂëÁ÷Í¨µÀºÅ c[12] --£¨0~64£© 0-Ö÷ÂëÁ÷ 1-¸¨ÂëÁ÷1  2-¸¨ÂëÁ÷2  3 -¸¨ÂëÁ÷3
    PRINT_INFO("requeset_type:%d, channel_num:%d\n", t_msg_head->c[12], t_msg_head->c[8]);

    if(0 == t_msg_head->c[12]) { //Ö÷ÂëÁ÷ÇëÇóIÖ¡
        #if MULTI_CONNECT
        PRINT_INFO("strat send main stream");
        //UtSetLvSockStatus(pCtrl->fSessionInt, 1, UT_MAIN_STREAM, &g_dahua_session);
        //UtSetLvSockStatus(pCtrl->fSessionInt, 0, UT_SUB_STREAM, &g_dahua_session);
        UtSetLvSockStatusByIp(pCtrl->c_ip, 1, UT_MAIN_STREAM, &g_dahua_session);
        UtSetLvSockStatusByIp(pCtrl->c_ip, 0, UT_SUB_STREAM, &g_dahua_session);
        
        //UtSetLvSockWaitI(pCtrl->fSessionInt, UT_MAIN_STREAM, 1, &g_dahua_session);
        #else
        PRINT_ERR("set main stream\n");
        send_main = 1;
        send_sub = 0;
        #endif
    } else if(1 == t_msg_head->c[12]) { //´ÎÂëÁ÷ÇëÇóIÖ¡
        #if MULTI_CONNECT
        PRINT_INFO("strat send sub stream");
        UtSetLvSockStatusByIp(pCtrl->c_ip, 0, UT_MAIN_STREAM, &g_dahua_session);
        UtSetLvSockStatusByIp(pCtrl->c_ip, 1, UT_SUB_STREAM, &g_dahua_session);
        #else
        PRINT_ERR("set sub stream\n");
        send_main = 0;
        send_sub = 1;
        #endif
    }

    rcv_msg_len = DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen;
    char send_msg[BUFLEN] = {0};
    t_msg_head = (DVRIP_HEAD_T *)send_msg;                
    ZERO_DVRIP_HEAD_T(t_msg_head)
        t_msg_head->dvrip.cmd = ACK_I_FRAME_REQUEST;   

    t_msg_head->c[8] = 0; //·µ»ØÂë ¨D¨D 0-³É¹¦£¬1-Ê§°Ü
    ZhiNuo_SockSend(pCtrl, send_msg, DVRIP_HEAD_T_SIZE, 100);

    return rcv_msg_len;
}

/*************************************************************
 * º¯Êý½éÉÜ£ºÉè±¸²Ù×÷
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_DEV_Control(char* bufPost, DAHUA_SESSION_CTRL *sock)
{
    int rcv_msg_len = 0;
    char ch_child_dev_num = 0;
    char ch_control_type = 0;
    char ch_control_code = 0;
    char tem_buf[DVRIP_HEAD_T_SIZE] = {0};
    memcpy(tem_buf, bufPost, DVRIP_HEAD_T_SIZE);
    DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)tem_buf;    

    PRINT_INFO("hl:%d, v:%d, vextralen:%d\n", t_msg_head->dvrip.dvrip_hl, 
            t_msg_head->dvrip.dvrip_v, t_msg_head->dvrip.dvrip_extlen);

    ch_child_dev_num = t_msg_head->c[12];
    ch_control_type = t_msg_head->c[8];
    ch_control_code = t_msg_head->c[16];

    //¿ØÖÆÀàÐÍ1:ÖØÆôÉè±¸ 2:¹Ø±ÕÉè±¸
    PRINT_INFO("control_type:%d, ch_child_dev_num:%d, ch_control_code:%d\n", t_msg_head->c[8], t_msg_head->c[12],
            t_msg_head->c[16]);  
    rcv_msg_len = DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen;

    char send_msg[BUFLEN] = {0};
    t_msg_head = (DVRIP_HEAD_T *)send_msg;                
    ZERO_DVRIP_HEAD_T(t_msg_head)
        t_msg_head->dvrip.cmd = ACK_DEV_CONTROL;   

    t_msg_head->c[8] = ch_control_type; 
    t_msg_head->c[12] = ch_control_type;//·µ»ØÂë0:³É¹¦1:Ã»È¨ÏÞ2:ÔÝÊ±ÎÞ·¨Ö´ÐÐ
    t_msg_head->c[14] = ch_child_dev_num; 
    t_msg_head->c[16] = ch_control_code; 
    ZhiNuo_SockSend(sock, send_msg, DVRIP_HEAD_T_SIZE, 100);

    //ÖØÆô
    if(1 == ch_control_type)
    {
        int delay = 0;
        goke_api_set_ioctrl(SYS_IPC_REBOOT, &delay);
    }

    //¹Ø»ú
    if(2 == ch_control_type)
    {
        //dms_sysnetapi_ioctrl(0,  DMS_NET_CMD_SHUTDOWN, 0, 0, 0);
    }    

    return rcv_msg_len;

}
/*************************************************************
 * º¯Êý½éÉÜ£ºÐÞ¸ÄÅäÖÃ²ÎÊý
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_Set_Config(char* bufPost, DAHUA_SESSION_CTRL *sock)
{
    int rcv_msg_len = 0;
    char ch_param_type = 0;
    char ch_child_type = 0;
    char ch_config_edition = 0;
    //char ch_param_effect_flag = 0;
    char ch_child_type_limit = 0;
    char tem_buf[DVRIP_HEAD_T_SIZE] = {0};
    memcpy(tem_buf, bufPost, DVRIP_HEAD_T_SIZE);
    DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)tem_buf;    
    char * extra = bufPost + DVRIP_HEAD_T_SIZE;

    PRINT_INFO("hl:%d, v:%d, vextralen:%d\n", t_msg_head->dvrip.dvrip_hl, 
            t_msg_head->dvrip.dvrip_v, t_msg_head->dvrip.dvrip_extlen);

    ch_param_type = t_msg_head->c[16];
    ch_child_type = t_msg_head->c[17];
    ch_config_edition = t_msg_head->c[18];
    //ch_param_effect_flag = t_msg_head->c[20];
    ch_child_type_limit = t_msg_head->c[24];

    //´òÓ¡ËÑË÷ÀàÐÍ 16:±àÂëÄÜÁ¦ÐÅÏ¢ 127:Ë«ÂëÁ÷Í¼ÏñÅäÖÃ 126:ÑÕÉ«ÅäÖÃ 124:»ñµÃÊÂ¼þÅäÖÃ 
    PRINT_INFO("ch_param_type:%d, ch_child_type:%d, ch_config_edition:%d, ch_param_effect_flag:%d, ch_child_type_limit:%d\n",
            t_msg_head->c[16], t_msg_head->c[17], t_msg_head->c[18], t_msg_head->c[20], t_msg_head->c[24]); 

    rcv_msg_len = DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen;

    char send_msg[BUFLEN] = {0};
    t_msg_head = (DVRIP_HEAD_T *)send_msg;                
    ZERO_DVRIP_HEAD_T(t_msg_head)
        t_msg_head->dvrip.cmd = ACK_SET_CONFIG;      

    if(0 == ch_param_type)
    {
        CONFIG_TIME *pTime = (CONFIG_TIME*)extra;

        if (pTime->c[16] == 0) { // 0 . ; 1 - ; 2 /  
            runChannelCfg.channelInfo[0].osdDatetime.dateSprtr = 3;
            if (pTime->c[15] == 0) { // 0 ÄêÔÂÈÕ£¬ 1 ÔÂÈÕÄê£¬ 2 ÈÕÔÂÄê
                runChannelCfg.channelInfo[0].osdDatetime.dateFormat = 0;
            } else if (pTime->c[15] == 1) {
                runChannelCfg.channelInfo[0].osdDatetime.dateFormat = 1;
            } else if (pTime->c[15] == 2) {
                runChannelCfg.channelInfo[0].osdDatetime.dateFormat = 4;
            }
        } else if (pTime->c[16] == 1) {
            runChannelCfg.channelInfo[0].osdDatetime.dateSprtr = 1;
            if (pTime->c[15] == 0) {
                runChannelCfg.channelInfo[0].osdDatetime.dateFormat = 0;
            } else if (pTime->c[15] == 1) {
                runChannelCfg.channelInfo[0].osdDatetime.dateFormat = 1;
            } else if (pTime->c[15] == 2) {
                runChannelCfg.channelInfo[0].osdDatetime.dateFormat = 4;
            }
        } else if (pTime->c[16] == 2) {
            runChannelCfg.channelInfo[0].osdDatetime.dateSprtr = 2;
            if (pTime->c[15] == 0) {
                runChannelCfg.channelInfo[0].osdDatetime.dateFormat = 2;
            } else if (pTime->c[15] == 1) {
                runChannelCfg.channelInfo[0].osdDatetime.dateFormat = 3;
            } else if (pTime->c[15] == 2) {
                runChannelCfg.channelInfo[0].osdDatetime.dateFormat = 5;
            }
        } 

        if (pTime->c[17] == 1) { // 12Ð¡Ê±ÖÆ
            runChannelCfg.channelInfo[0].osdDatetime.timeFmt = 1;
        } else { // 24Ð¡Ê±ÖÆ
            runChannelCfg.channelInfo[0].osdDatetime.timeFmt = 0;
        }

        runChannelCfg.channelInfo[1].osdDatetime.dateFormat = runChannelCfg.channelInfo[0].osdDatetime.dateFormat;
        runChannelCfg.channelInfo[1].osdDatetime.dateSprtr = runChannelCfg.channelInfo[0].osdDatetime.dateSprtr;

        netcam_osd_update_clock();
    }

    if(34 == ch_param_type)
    {
        VIDEO_COVER_T *video_cover = (VIDEO_COVER_T*)extra;
        VIDEO_COVER_ATTR_T *cover_attr = NULL;

        PRINT_INFO("search_type:%d, ÉèÖÃÊÓÆµÇøÓòÕÚµ², num:%d, check_recv_len:%d\n", ch_param_type, video_cover->iCoverNum, 
                sizeof(DVRIP_HEAD_T_SIZE) + sizeof(VIDEO_COVER_T) + sizeof(VIDEO_COVER_ATTR_T) * video_cover->iCoverNum);
        //set ÊÓÆµÕÚµ² ¿ª¹Ø¼°×ø±ê,ÑÕÉ« is cmd 34
        GK_NET_SHELTER_RECT cover[4];
        if(0 == goke_api_get_ioctrl(GET_COVER_CFG, cover))
        {
            int i;
            for(i = 0; i < video_cover->iCoverNum; i++)
            {
                cover_attr = (VIDEO_COVER_ATTR_T*)(extra + sizeof(VIDEO_COVER_T) + sizeof(VIDEO_COVER_ATTR_T ) * i);
                if(1024 != cover_attr->tBlock.left && 1024 != cover_attr->tBlock.top && 
                        2048 != cover_attr->tBlock.right && 2048 != cover_attr->tBlock.bottom)
                {
                    cover[i].x = (float)cover_attr->tBlock.left / 8192.0;
                    cover[i].y = (float)cover_attr->tBlock.top / 8192.0;
                    cover[i].width = (float)(cover_attr->tBlock.right - cover_attr->tBlock.left)/ 8192.0;
                    cover[i].height= (float)(cover_attr->tBlock.bottom - cover_attr->tBlock.top)/ 8192.0;
                    cover[i].color = 0;
                    cover[i].enable = 1;
                }else{
                    cover[i].enable = 0;
                }
                PRINT_INFO("chan:%d, num:%d, lef:%d, top:%d, right:%d, bottom:%d, type:%d, encode:%d, view:%d\n", 
                        video_cover->iChannel, video_cover->iCoverNum, cover_attr->tBlock.left, 
                        cover_attr->tBlock.top, cover_attr->tBlock.right, cover_attr->tBlock.bottom,
                        cover_attr->iBlockType,cover_attr->Encode,cover_attr->Priview);
                PRINT_INFO("x:%f, y:%f, w:%f, w:%f\n", cover[i].x,cover[i].y,cover[i].width,cover[i].height);
            }
            goke_api_set_ioctrl(SET_COVER_CFG, cover);
        }

    }
    if(38 == ch_param_type)
    {
        PRINT_ERR("search_type:%d, image config\n", ch_param_type);
        CAM_CONTROL *cam_control = (CAM_CONTROL *)extra;

        //»ñÈ¡Éè±¸µÄÑÕÉ«²ÎÊý×î´óÖµ
        //»ñÈ¡Éè±¸µÄÑÕÉ«²ÎÊý
        GK_NET_IMAGE_CFG str_channel_color_info;
        //»ñÈ¡Éè±¸µÄÑÕÉ«²ÎÊý
        if(0 != goke_api_get_ioctrl(GET_IMAGE_CFG,&str_channel_color_info))
        {
            PRINT_ERR("DMS_NET_GET_COLORCFG fail\n");
            return rcv_msg_len;
        }  
        switch(cam_control->AutoColor2BW)/*0-×ÜÊÇ²ÊÉ« 1- ×Ô¶¯ 2-×ÜÊÇºÚ°×*/
        {
            case 0:
                str_channel_color_info.irCutMode = 1;
                break;
            case 1:
                str_channel_color_info.irCutMode = 0;
                break;
            case 2:
                str_channel_color_info.irCutMode = 2;
                break;
            default:
                str_channel_color_info.irCutMode = 0;
                break;
        }
        str_channel_color_info.flipEnabled = cam_control->Flip;
        str_channel_color_info.mirrorEnabled = cam_control->Mirror;
        PRINT_INFO("ircut:%d, flip:%d, mirr:%d\n", 
                str_channel_color_info.irCutMode, str_channel_color_info.flipEnabled, str_channel_color_info.mirrorEnabled);
        //ÉèÖÃÉè±¸µÄÑÕÉ«²ÎÊý
        if(0 != goke_api_set_ioctrl(SET_IMAGE_CFG,&str_channel_color_info))
        {
            PRINT_ERR("SET_IMAGE_CFG fail\n");
            return rcv_msg_len;
        }  
        t_msg_head->c[9] = 0; //0£º²»ÐèÒªÖØÆô 1£ºÐèÒªÖØÆô²ÅÄÜÉúÐ§  
    }
    if(126 == ch_param_type)
    {       
        CONFIG_COLOR_OLD * str_color = (CONFIG_COLOR_OLD *)extra;

        //»ñÈ¡Éè±¸µÄÑÕÉ«²ÎÊý×î´óÖµ
        //»ñÈ¡Éè±¸µÄÑÕÉ«²ÎÊý
        GK_NET_IMAGE_CFG str_channel_color_info;

        //»ñÈ¡Éè±¸µÄÑÕÉ«²ÎÊý
        if(0 != goke_api_get_ioctrl(GET_IMAGE_CFG,&str_channel_color_info))
        {
            PRINT_ERR("DMS_NET_GET_COLORCFG fail\n");
            return rcv_msg_len;
        }  
        /* !< ÁÁ¶È0-100 */
        str_channel_color_info.brightness = str_color->Color[0].Brightness; 
        str_channel_color_info.contrast = str_color->Color[0].Contrast;			/*!< ¶Ô±È¶È	0-100		*/
        str_channel_color_info.saturation = str_color->Color[0].Saturation;			/*!< ±¥ºÍ¶È	0-100		*/
        str_channel_color_info.hue = str_color->Color[0].Hue;	        
        PRINT_ERR("set Brightness:%d,Contrast:%d,Saturation:%d,Hue%d\n", str_channel_color_info.brightness,
                str_channel_color_info.contrast,
                str_channel_color_info.saturation,
                str_channel_color_info.hue);

        //ÉèÖÃÉè±¸µÄÑÕÉ«²ÎÊý
        if(0 != goke_api_set_ioctrl(SET_IMAGE_CFG,&str_channel_color_info))
        {
            PRINT_ERR("SET_IMAGE_CFG fail\n");
            return rcv_msg_len;
        }  
        t_msg_head->c[9] = 0; //0£º²»ÐèÒªÖØÆô 1£ºÐèÒªÖØÆô²ÅÄÜÉúÐ§  
    }

    else if(127 == ch_param_type)//set enc attr and osd attr
    {
        CONFIG_CAPTURE_OLD * str_captue = (CONFIG_CAPTURE_OLD *)extra;

        if(dahua_set_osd_attr(str_captue) < 0)
            return rcv_msg_len;
        if(dahua_set_enc_attr(str_captue) < 0)
            return rcv_msg_len;
        t_msg_head->c[9] = 0; //0£º²»ÐèÒªÖØÆô 1£ºÐèÒªÖØÆô²ÅÄÜÉúÐ§  
    }

    else if(124 == ch_param_type)
    {
        if(4 == ch_child_type_limit)
        {
#ifdef DAHA_PRTCL_OK
            DMS_NET_CHANNEL_MOTION_DETECT str_dms_motion_detect;

            //»ñÈ¡Éè±¸µÄ¶¯Ì¬¼à²â²ÎÊý
            if(0 != dms_sysnetapi_ioctrl(g_dahua_handle, DMS_NET_GET_MOTIONCFG, 0, &str_dms_motion_detect, sizeof(DMS_NET_CHANNEL_MOTION_DETECT)))
            {
                PRINT_ERR("DMS_NET_GET_MOTIONCFG fail\n");
                return rcv_msg_len;
            }            

            CONFIG_MOTIONDETECT * str_motion_detect = (CONFIG_MOTIONDETECT *)extra;   

            PRINT_INFO("enble:%d,level:%d, liuer str_motion_detect1:%x,%x\n", str_motion_detect->bEnable, str_motion_detect->iLevel, (unsigned int)str_motion_detect->mRegion[0],
                    (unsigned int)str_motion_detect->mRegion[1]);

            memset(str_dms_motion_detect.byMotionArea, 0, sizeof(str_dms_motion_detect.byMotionArea));
            //2014-05-14£ºÖÇÅµ3.3°æ±¾Ö®Ç°NVR£¬str_motion_detect.bEnable×Ö¶ÎÊÇÎÞÐ§µÄ£¬Òò´ËÓÃÇøÓòÊÇ·ñÓÐÐ§×÷ÎªÊÇ·ñÆô¶¯ÒÆ¶¯Õì²â±ê¼Ç
            //ÖÇÅµÊÇ22*18¸ñ×Ó£¬ÕûÐÎÊý×éµÚÒ»¸öÔªËØµÄÇ°22Î»ÓÐÐ§£¬Ò»¸öÎ»±êÊ¾Ò»¸ö¸ñ×ÓÊÇ·ñÑ¡ÖÐ£¬×Ü¹²18¸öÔªËØÓÐÐ§
            int x = 0;
            int y = 0;
            int enable_flag = 0;
            for(y = 0; y < 18; y++)
            {
                for(x = 0; x < 22; x++)
                {
                    if(1 == (1 & (str_motion_detect->mRegion[y] >> x)))
                    {
                        enable_flag = 1;
                        if(0 == (((y*44*2) + (2 * x + 1)) % 8))
                        {
                            ZHINUO_SET_BIT(str_dms_motion_detect.byMotionArea[((y*44*2) + (2 * x + 1)) / 8 - 1], 8)
                        }
                        else                        
                        {
                            ZHINUO_SET_BIT(str_dms_motion_detect.byMotionArea[((y*44*2) + (2 * x + 1)) / 8], ((y*44*2) + (2 * x + 1)) % 8)
                        }

                        if(0 == (((y*44*2) + (2 * x + 1) + 44) % 8))
                        {
                            ZHINUO_SET_BIT(str_dms_motion_detect.byMotionArea[((y*44*2) + (2 * x + 1) + 44) / 8 - 1], 8)                            
                        }
                        else                         
                        {
                            ZHINUO_SET_BIT(str_dms_motion_detect.byMotionArea[((y*44*2) + (2 * x + 1) + 44) / 8], ((y*44*2) + (2 * x + 1) + 44) % 8)                            
                        }

                        if(0 == (((y*44*2) + (2 * x + 1) + 1) % 8))
                        {
                            ZHINUO_SET_BIT(str_dms_motion_detect.byMotionArea[((y*44*2) + (2 * x + 1) + 1) / 8 - 1], 8)
                        }
                        else                         
                        {
                            ZHINUO_SET_BIT(str_dms_motion_detect.byMotionArea[((y*44*2) + (2 * x + 1) + 1) / 8], ((y*44*2) + (2 * x + 1) + 1) % 8)
                        }

                        if(0 == (((y*44*2) + (2 * x + 1) + 1 + 44) % 8))
                        {
                            ZHINUO_SET_BIT(str_dms_motion_detect.byMotionArea[((y*44*2) + (2 * x + 1) + 1 + 44) / 8 - 1], 8)                            
                        }
                        else
                        {
                            ZHINUO_SET_BIT(str_dms_motion_detect.byMotionArea[((y*44*2) + (2 * x + 1) + 1 + 44) / 8], ((y*44*2) + (2 * x + 1) + 1 + 44) % 8)
                        }
                    }
                }
            }

            PRINT_INFO("str_motion_detect->bEnable:%d\n", str_motion_detect->bEnable);
            if(1 == enable_flag)
            {
                str_motion_detect->bEnable = 1;
                int day = 0;
                for(day = 0; day < 7; day++)
                {
                    str_dms_motion_detect.stScheduleTime[day][0].byStartHour = 0;
                    str_dms_motion_detect.stScheduleTime[day][0].byStartMin = 0;                          
                    str_dms_motion_detect.stScheduleTime[day][0].byStopHour = 23;
                    str_dms_motion_detect.stScheduleTime[day][0].byStopMin = 59;  
                    str_dms_motion_detect.stScheduleTime[day][1].byStartHour = 0;
                    str_dms_motion_detect.stScheduleTime[day][1].byStartMin = 0;                          
                    str_dms_motion_detect.stScheduleTime[day][1].byStopHour = 23;
                    str_dms_motion_detect.stScheduleTime[day][1].byStopMin = 59;                     
                }                 
            }
            else
            {
                str_motion_detect->bEnable = 0;   
                memset(str_dms_motion_detect.stScheduleTime, 0, sizeof(str_dms_motion_detect.stScheduleTime));               
            }
            str_dms_motion_detect.bEnable =  str_motion_detect->bEnable;            
            str_dms_motion_detect.dwSensitive = (str_motion_detect->iLevel - 1) * 100 / 5; //ÖÇÅµµÈ¼¶Îª1-6×ª»¯ÎªÎÒÃÇµÄ0-100

            //ÉèÖÃÉè±¸µÄ¶¯Ì¬¼à²â²ÎÊý
            if(0 != dms_sysnetapi_ioctrl(g_dahua_handle, DMS_NET_SET_MOTIONCFG, 0, &str_dms_motion_detect, sizeof(DMS_NET_CHANNEL_MOTION_DETECT)))
            {
                PRINT_ERR("DMS_NET_SET_MOTIONCFG fail\n");
                return rcv_msg_len;
            }   

            ret = dms_sysnetapi_ioctrl(g_dahua_handle, DMS_NET_SET_SAVECFG, 0, 0, 0); 
            if(0 != ret)
            {
                PRINT_ERR("DMS_NET_SET_SAVECFG error\n");
            }            
#endif

            t_msg_head->c[9] = 0; //0£º²»ÐèÒªÖØÆô 1£ºÐèÒªÖØÆô²ÅÄÜÉúÐ§  
        }
    }    

    t_msg_head->c[8] = 0; //·µ»ØÂë0:³É¹¦1:Ê§°Ü2:Êý¾Ý²»ºÏ·¨3:ÔÝÊ±ÎÞ·¨ÉèÖÃ4:Ã»ÓÐÈ¨ÏÞ
    t_msg_head->c[16] = ch_param_type;
    t_msg_head->c[17] = ch_child_type; 
    t_msg_head->c[18] = ch_config_edition; 
    t_msg_head->c[24] = ch_child_type_limit;

    ZhiNuo_SockSend(sock, send_msg, DVRIP_HEAD_T_SIZE, 100);            
    return rcv_msg_len;
}


/*************************************************************
 * º¯Êý½éÉÜ£ºÊ±¼ä¹ÜÀí
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_Time_Manage(char* bufPost, DAHUA_SESSION_CTRL *sock)
{
    int rcv_msg_len = 0;
    char ch_cmd_code = 0;
    char ch_year = 0;
    char ch_month = 0;
    char ch_day = 0;
    char ch_hour = 0;
    char ch_min = 0;
    char ch_sec = 0;    
    char tem_buf[DVRIP_HEAD_T_SIZE] = {0};
    memcpy(tem_buf, bufPost, DVRIP_HEAD_T_SIZE);
    DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)tem_buf;    

    PRINT_INFO("hl:%d, v:%d, vextralen:%d\n", t_msg_head->dvrip.dvrip_hl, 
            t_msg_head->dvrip.dvrip_v, t_msg_head->dvrip.dvrip_extlen);


    ch_cmd_code = t_msg_head->c[8];
    ch_year = t_msg_head->c[16];
    ch_month = t_msg_head->c[17];
    ch_day = t_msg_head->c[18];
    ch_hour = t_msg_head->c[19];
    ch_min = t_msg_head->c[20];
    ch_sec = t_msg_head->c[21];

    //ÃüÁîÂë£º0£­²éÑ¯£¬1£­ÉèÖÃ
    PRINT_INFO("ch_cmd_code:%d, year:%d, month:%d, day:%d, hour:%d, min:%d, sec:%d\n", 
        t_msg_head->c[8], t_msg_head->c[16], t_msg_head->c[17], t_msg_head->c[18],
        t_msg_head->c[19], t_msg_head->c[20], t_msg_head->c[21]);  
    rcv_msg_len = DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen;

    if(0 == ch_cmd_code)
    {
        DMS_TIME SysGetTime;
        goke_api_get_ioctrl(GET_SYSTEM_TIME, &SysGetTime);
        t_msg_head->c[9] = 0;  //·µ»ØÂë0:³É¹¦1:³ö´í2:ÎÞÈ¨ÏÞ3: Ê±¼ä³åÍ»£¬ÉèÖÃÊ§°Ü

        char send_msg[BUFLEN] = {0};
        t_msg_head = (DVRIP_HEAD_T *)send_msg;                
        ZERO_DVRIP_HEAD_T(t_msg_head)
            t_msg_head->dvrip.cmd = ACK_TIME_MANAGE;  

        t_msg_head->c[8] = ch_cmd_code;
        t_msg_head->c[16] = SysGetTime.dwYear - 2000;
        t_msg_head->c[17] = SysGetTime.dwMonth;
        t_msg_head->c[18] = SysGetTime.dwDay;
        t_msg_head->c[19] = SysGetTime.dwHour;
        t_msg_head->c[20] = SysGetTime.dwMinute;
        t_msg_head->c[21] = SysGetTime.dwSecond;
        PRINT_INFO("ch_cmd_code:%d, year:%d, month:%d, day:%d, hour:%d, min:%d, sec:%d\n", t_msg_head->c[8], t_msg_head->c[16], t_msg_head->c[17],
                t_msg_head->c[18], t_msg_head->c[19], t_msg_head->c[20], t_msg_head->c[21]);         
        ZhiNuo_SockSend(sock, send_msg, DVRIP_HEAD_T_SIZE, 100);  
    }
    if(1 == ch_cmd_code)
    {
        //PRINT_INFO("set time:year:%d, month:%d, day:%d, hour:%d, min:%d,sec:%d\n", 
                //ch_year,ch_day,ch_hour,ch_min,ch_sec);
        DMS_TIME SysGetTime;
        SysGetTime.dwYear = 2000 + ch_year;
        SysGetTime.dwMonth = ch_month;
        SysGetTime.dwDay = ch_day;
        SysGetTime.dwHour = ch_hour;
        SysGetTime.dwMinute = ch_min;
        SysGetTime.dwSecond = ch_sec;        
        goke_api_set_ioctrl(SET_SYSTEM_TIME, &SysGetTime);

        t_msg_head->c[9] = 0;  //·µ»ØÂë0:³É¹¦1:³ö´í2:ÎÞÈ¨ÏÞ3: Ê±¼ä³åÍ»£¬ÉèÖÃÊ§°Ü
        char send_msg[BUFLEN] = {0};
        t_msg_head = (DVRIP_HEAD_T *)send_msg;                
        ZERO_DVRIP_HEAD_T(t_msg_head)
            t_msg_head->dvrip.cmd = ACK_TIME_MANAGE;  

        t_msg_head->c[8] = ch_cmd_code;
        t_msg_head->c[16] = ch_year;
        t_msg_head->c[17] = ch_month;
        t_msg_head->c[18] = ch_day;
        t_msg_head->c[19] = ch_hour;
        t_msg_head->c[20] = ch_min;
        t_msg_head->c[21] = ch_sec;
        ZhiNuo_SockSend(sock, send_msg, DVRIP_HEAD_T_SIZE, 100);            
    }
    return rcv_msg_len;
}

//#endif

/*************************************************************
 * º¯Êý½éÉÜ£ºÐÞ¸ÄÍ¨µÀÃû²¢ÇÒosdÏÔÊ¾
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_Set_Channel_Name(char* bufPost, DAHUA_SESSION_CTRL *sock)
{
    int ret = 0;
    int rcv_msg_len = 0;
    char channel_name_type = 0;
    int n_extra_len = 0;

    char tem_buf[DVRIP_HEAD_T_SIZE] = {0};
    memcpy(tem_buf, bufPost, DVRIP_HEAD_T_SIZE);
    DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)tem_buf; 
    char *extra = bufPost + DVRIP_HEAD_T_SIZE;

    PRINT_INFO("hl:%d, v:%d, vextralen:%d\n", t_msg_head->dvrip.dvrip_hl, 
            t_msg_head->dvrip.dvrip_v, t_msg_head->dvrip.dvrip_extlen);

    channel_name_type = t_msg_head->c[8];
    n_extra_len = t_msg_head->dvrip.dvrip_extlen;

    PRINT_INFO("extra:%s, channel_name_type:%d\n", extra, t_msg_head->c[8]);  
    PRINT_INFO("set channle name:%s, len:%d\n", extra, n_extra_len);
    rcv_msg_len = DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen;

    char send_msg[BUFLEN] = {0};
    t_msg_head = (DVRIP_HEAD_T *)send_msg;                
    ZERO_DVRIP_HEAD_T(t_msg_head)

        //»ñÈ¡Í¨µÀÃû
        GK_NET_CHANNEL_INFO channel_info;
    channel_info.id = 0;
    ret = goke_api_get_ioctrl(GET_OSD_CFG, &channel_info);
    if((0 == ret) && (strlen(channel_info.osdChannelName.text) > 0))
    {
        t_msg_head->dvrip.dvrip_extlen = n_extra_len; 
        memcpy(channel_info.osdChannelName.text, extra, n_extra_len);//°Ñ"&&"Õâ¸öÈ¥µô
        channel_info.osdChannelName.text[n_extra_len] = 0;
        goke_api_set_ioctrl(SET_OSD_CFG, &channel_info);
        memcpy(send_msg + DVRIP_HEAD_T_SIZE, extra, n_extra_len);
    } 
    else
    {
        t_msg_head->dvrip.dvrip_extlen = 0;
    }

    t_msg_head->dvrip.cmd = ACK_SET_CHANNEL_NAME;   
    t_msg_head->c[8] = channel_name_type;

    ZhiNuo_SockSend(sock, send_msg, DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen, 100);          
    return rcv_msg_len;
}


/*************************************************************
 * º¯Êý½éÉÜ£ºÔÆÌ¨¿ØÖÆ
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_PTZ_Control(char* bufPost, DAHUA_SESSION_CTRL *sock)
{
    int rcv_msg_len = 0;
    char cmd = 0;
    char param1 = 0;
    char param2 = 0;
    char param3 = 0;    
    char tem_buf[DVRIP_HEAD_T_SIZE] = {0};
    memcpy(tem_buf, bufPost, DVRIP_HEAD_T_SIZE);
    DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)tem_buf;    

    PRINT_INFO("hl:%d, v:%d, vextralen:%d\n", t_msg_head->dvrip.dvrip_hl, 
            t_msg_head->dvrip.dvrip_v, t_msg_head->dvrip.dvrip_extlen);

    //Í¨µÀºÅ >=0 ÃüÁî ²ÎÊý1 ²ÎÊý2 ²ÎÊý3
    PRINT_INFO("channel_num:%d, cmd:%d, param1:%d, param2:%d, param3:%d, localtion1:%d, localtion2:%d, localtion3:%d\n",
            t_msg_head->c[9], t_msg_head->c[10], t_msg_head->c[11], t_msg_head->c[12], t_msg_head->c[16], 
            t_msg_head->l[5], t_msg_head->l[6], t_msg_head->l[7]);
    cmd = t_msg_head->c[10];
    param1 = t_msg_head->c[11];
    param2 = t_msg_head->c[12];
    param3 = t_msg_head->c[16];     
    //localtion1 = t_msg_head->l[5];
    //localtion2 = t_msg_head->l[6];
    //localtion3 = t_msg_head->l[7];      

    rcv_msg_len = DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen;

    if((0 == param1) && (0 == param2) && (0 == param3) && (ZhiNuo_PTZ_CMD_LIGHT_BRUSH != cmd))
    {
        cmd = ZhiNuo_PTZ_CMD_STOP;
    }

    int ctrol_cmd = -1;
    switch(cmd)
    {
        case ZhiNuo_PTZ_CMD_STOP:		/*Í£Ö¹ÔÆÌ¨»î¶¯*/
            {
                PRINT_INFO("PTZ STOP\n");
                ctrol_cmd = DMS_PTZ_CMD_STOP;
                goke_api_set_ioctrl(SET_PTZ_CMD, &ctrol_cmd);
                break;
            }             
        case ZhiNuo_PTZ_CMD_UP:        /*ÉÏ*/
            {
                PRINT_INFO("PTZ UP\n");
                ctrol_cmd = DMS_PTZ_CMD_UP;
                goke_api_set_ioctrl(SET_PTZ_CMD, &ctrol_cmd);
                break;
            }
        case ZhiNuo_PTZ_CMD_DOWN:        /*ÏÂ*/
            {
                PRINT_INFO("PTZ DOWN\n");
                ctrol_cmd = DMS_PTZ_CMD_DOWN;
                goke_api_set_ioctrl(SET_PTZ_CMD, &ctrol_cmd);
                break;
            }
        case ZhiNuo_PTZ_CMD_LEFT:        /*×ó*/
            {
                PRINT_INFO("PTZ LEFT\n");
                ctrol_cmd = DMS_PTZ_CMD_LEFT;
                goke_api_set_ioctrl(SET_PTZ_CMD, &ctrol_cmd);
                break;
            }
        case ZhiNuo_PTZ_CMD_RIGHT:        /*ÓÒ*/
            {
                PRINT_INFO("PTZ RIGHT\n");
                ctrol_cmd = DMS_PTZ_CMD_RIGHT;
                goke_api_set_ioctrl(SET_PTZ_CMD, &ctrol_cmd);
                break;
            }
        case ZhiNuo_PTZ_CMD_LEFT_UP:
            ctrol_cmd = DMS_PTZ_CMD_UP_LEFT;
            goke_api_set_ioctrl(SET_PTZ_CMD, &ctrol_cmd);
            break;
        case ZhiNuo_PTZ_CMD_RIGHT_UP:
            ctrol_cmd = DMS_PTZ_CMD_UP_RIGHT;
            goke_api_set_ioctrl(SET_PTZ_CMD, &ctrol_cmd);
            break;
        case ZhiNuo_PTZ_CMD_LEFT_DOWN:
            ctrol_cmd = DMS_PTZ_CMD_DOWN_LEFT;
            goke_api_set_ioctrl(SET_PTZ_CMD, &ctrol_cmd);
            break;
        case ZhiNuo_PTZ_CMD_RIGHT_DOWN:
            ctrol_cmd = DMS_PTZ_CMD_DOWN_RIGHT;
            goke_api_set_ioctrl(SET_PTZ_CMD, &ctrol_cmd);
            break;
        case ZhiNuo_PTZ_CMD_ZOOM_SUB:	/*±ä±¶+*/
            {
                PRINT_INFO("PTZ ZOOM SUB\n");
                break;
            }
        case ZhiNuo_PTZ_CMD_ZOOM_ADD:	/*±ä±¶-*/
            {
                PRINT_INFO("PTZ ZOOM ADD\n");
                break;
            }
        case ZhiNuo_PTZ_CMD_FOCUS_ADD:	/*¾Û½¹+*/
            {
                PRINT_INFO("PTZ FOCUS ADD\n");
                break;
            }
        case ZhiNuo_PTZ_CMD_FOCUS_SUB:	/*¾Û½¹-*/
            {
                PRINT_INFO("PTZ FOCUS SUB\n");
                break;
            }
        case ZhiNuo_PTZ_CMD_IRIS_ADD: 	/*¹âÈ¦+*/
            {
                PRINT_INFO("PTZ IRIS ADD\n");
                break;
            }
        case ZhiNuo_PTZ_CMD_IRIS_SUB: 	/*¹âÈ¦-*/
            {
                PRINT_INFO("PTZ IRIS SUB\n");
                break;
            }
        case ZhiNuo_PTZ_CMD_LIGHT_BRUSH: /*µÆ¹âÓêË¢*/
            {
                if(0x01 == param1) //0x01¿ªÆô 0x00¹Ø±Õ
                {
                    PRINT_INFO("PTZ BRUSH  LIGHT OPEN\n");
                }
                else
                {
                    PRINT_INFO("PTZ BRUSH  LIGHT CLOSE\n");
                }
                return rcv_msg_len;
                break;
            }                    
        case ZhiNuo_PTZ_CMD_AUTO:			/*ÔÆÌ¨×Ô¶¯É¨Ãè*/
            {
                // 76 ¿ªÊ¼  99 ×Ô¶¯ 96 Í£Ö¹
                if(76 == param3)
                {
                    PRINT_INFO("PTZ START CRU\n");
                    ctrol_cmd = DMS_PTZ_CMD_START_CRU;
                    goke_api_set_ioctrl(SET_PTZ_CMD, &ctrol_cmd);
                }
                if(99 == param3)
                {
                    PRINT_INFO("PTZ AUTO STRAT\n");
                    ctrol_cmd = DMS_PTZ_CMD_START_CRU;
                    goke_api_set_ioctrl(SET_PTZ_CMD, &ctrol_cmd);
                }
                if(96 == param3)
                {
                    PRINT_INFO("PTZ STOP CRU\n");
                    ctrol_cmd = DMS_PTZ_CMD_STOP_CRU;
                    goke_api_set_ioctrl(SET_PTZ_CMD, &ctrol_cmd);
                } 
                break;
            }  
        case ZhiNuo_PTZ_CMD_ADD_POS_CRU:		/* ½«Ô¤ÖÃµã¼ÓÈëÑ²º½ÐòÁÐ */
            {
                PRINT_INFO("PTZ ADD POS CRU\n");
                break;
            } 
        case ZhiNuo_PTZ_CMD_DEL_POS_CRU:		/* ½«Ô¤ÖÃµã´ÓÑ²º½ÐòÁÐÖÐÉ¾³ý */
            {
                PRINT_INFO("PTZ DEL POS CRU\n");
                return rcv_msg_len;
            } 
        case ZhiNuo_PTZ_CMD_CLR_POS_CRU:		/* Çå¿ÕÑ²º½µã */
            {
                PRINT_INFO("PTZ CLR POS CRU\n");
                ctrol_cmd = DMS_PTZ_CMD_DEL_PRE_CRU;
                goke_api_set_ioctrl(SET_PTZ_CMD, &ctrol_cmd);
                return rcv_msg_len;
            }             
        case ZhiNuo_PTZ_CMD_PRESET_GO:		/*µ÷µ½Ô¤ÉèµÄÔ¤ÖÃµã*/
            {
                PRINT_INFO("PTZ PRESET GO\n");
                ctrol_cmd = DMS_PTZ_CMD_CALL;
                goke_api_set_ioctrl(SET_PTZ_CMD, &ctrol_cmd);
                break;
            }    
        case ZhiNuo_PTZ_CMD_PRESET_SET:		/*ÉèÖÃÔ¤ÖÃµã*/
            {
                PRINT_INFO("PTZ PRESET SET\n");
                ctrol_cmd = DMS_PTZ_CMD_PRESET;
                goke_api_set_ioctrl(SET_PTZ_CMD, &ctrol_cmd);
                break;
            }

        default:
            PRINT_ERR("Not support PTZ command!!\n");
            return rcv_msg_len;
    }

    return rcv_msg_len;
}

int DahuaMakeMsg(DAHUA_SESSION_CTRL *pCtrl,int id, int sid, unsigned char cmd)
{
    DVRIP_HEAD_T *pMsgHeader = (DVRIP_HEAD_T *)(pCtrl->fSendbuf);  
    ZERO_DVRIP_HEAD_T(pMsgHeader)
        pMsgHeader->dvrip.cmd = cmd;  

    pMsgHeader->l[1] = pCtrl->nSendLen - sizeof(DVRIP_HEAD_T);
    pMsgHeader->l[4] = pCtrl->nSendLen - sizeof(DVRIP_HEAD_T);
    pMsgHeader->l[2] = id;
    pMsgHeader->l[6] = sid;

    return 0;
}



static int Dahua_Alarm_ListMethod(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, int result)
{
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON *params, *method;
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());

    cJSON_AddItemToObject(params, "method", method = cJSON_CreateArray());
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.factory.instance"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.destroy"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.getInSlots"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.getOutSlots"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.getInState"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.getOutState"));
    
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.getExAlarmCaps"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.listMethod"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.startAlarmBell"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.stopAlarmBell"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.getChannelsState"));

    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.getAlarmCaps"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.setArmMode"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.getArmMode"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.setBypassMode"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.getBypassMode"));

    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.getActivateDefenceArea"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.addressBinding"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.getAllInSlots"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.getAllOutSlots"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.getAlarmInChannels"));

    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.setSilentAlarm"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.setSoundAlarm"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.setDefenceMode"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.getSubSystem"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.getDefenceArmMode"));

    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.getExModule"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.getConnectionStatus"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.getExAlarmBoxCaps"));
    cJSON_AddItemToArray(method, cJSON_CreateString("alarm.getExAlarmChannels"));

    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);


    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}

static int Dahua_DevVideoInput_ListMethod(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, int result)
{
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON *params, *method;
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());

    cJSON_AddItemToObject(params, "method", method = cJSON_CreateArray());
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.factory.instance"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.destroy"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.getCollect"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.getMaxSize"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.getCaps"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.autoFocus"));
    
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.getFocusStatus"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.adjustFocus"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.getCoverType"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.getCapsEx"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.resetEQ"));

    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.autoSetMargin"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.getVideoInStatus"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.testSyncPhaseBright"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.cancelTestSyncPhaseBright"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.adjustFocusContinuously"));

    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.listMethod"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.setColor"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.getMaxFrameRate"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.getCoverCount"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.getTitleCount"));

    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.setCover"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.setBackgroundColor"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.ptz"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.getCurrentWindow"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.setCurrentWindow"));


    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.enableAutoIris"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.adjustIris"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.generateStrobe"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.setTempVideoInOptions"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.getTempVideoInOptions"));

    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.restoreVideoInOptions"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.focusRegion"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.stopAutoFocus"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.getSyncPhaseInfo"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.setFishEyeEPtz"));

    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.getFishEyeWinInfo"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.setVideoSource"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.getVideoSource"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.getDenoiseInfo"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.getExternalSyncState"));

    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.setFishEyeOverlay"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoInput.resetFocusOptions"));

    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);


    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}

static int Dahua_DevVideoInput_GetCaps(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, int result)
{
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON *params, *caps;
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "caps", caps = cJSON_CreateObject());
    
    cJSON_AddFalseToObject(caps, "AutoSyncPhase");
    cJSON_AddTrueToObject(caps, "AutofocusPeak"); //true
    cJSON_AddNumberToObject(caps, "Backlight", 2);
    cJSON_AddTrueToObject(caps, "BrightnessCompensation"); //true ÁÁ¶È²¹³¥
    cJSON_AddNumberToObject(caps, "ChipID", 0);
    cJSON_AddNumberToObject(caps, "CoverCount", 4);

    cJSON_AddNumberToObject(caps, "CoverType", 1);
    cJSON_AddTrueToObject(caps, "CustomManualExposure"); //true
    cJSON_AddTrueToObject(caps, "DayNightColor"); //true 
    cJSON_AddNumberToObject(caps, "DayNightColorIO", 0);
    cJSON_AddTrueToObject(caps, "Defog"); //true 
    cJSON_AddNumberToObject(caps, "DoubleExposure", 0);

    cJSON_AddFalseToObject(caps, "DownScaling");
    cJSON_AddNumberToObject(caps, "EEModeRange", 100);
    cJSON_AddFalseToObject(caps, "ElectricFocus");
    cJSON_AddNumberToObject(caps, "Exposure", 16);
    cJSON_AddNumberToObject(caps, "ExposureMode", 401);
    cJSON_AddNumberToObject(caps, "ExternalSyncInput", 0);

    cJSON_AddFalseToObject(caps, "FishEye"); //ÓãÑÛ
    cJSON_AddFalseToObject(caps, "FlashAdjust");
    cJSON_AddTrueToObject(caps, "Flip"); //true 
    cJSON_AddNumberToObject(caps, "FormatCount", 5);
    cJSON_AddTrueToObject(caps, "Gain"); //true 
    cJSON_AddTrueToObject(caps, "GainAuto"); //true 
    cJSON_AddTrueToObject(caps, "Gamma"); //true 
    cJSON_AddNumberToObject(caps, "GammaModeRange", 100);

    cJSON_AddNumberToObject(caps, "GlareInhibition", 1); // Ç¿¹âÒÖÖÆ
    cJSON_AddNumberToObject(caps, "HorizontalBinning", 0);
    cJSON_AddTrueToObject(caps, "IRCUT"); //true 
    cJSON_AddFalseToObject(caps, "ImageStabilization"); 
    cJSON_AddTrueToObject(caps, "InfraRed"); //true 
    cJSON_AddFalseToObject(caps, "Iris");
    cJSON_AddFalseToObject(caps, "IrisAuto");

    cJSON_AddNumberToObject(caps, "LadenBitrate", 576000);
    cJSON_AddTrueToObject(caps, "LimitedAutoExposure");
    cJSON_AddNumberToObject(caps, "MaxExposureTime", 300); 
    cJSON_AddNumberToObject(caps, "MaxExposureTime1", 0);
    cJSON_AddNumberToObject(caps, "MaxHeight", 1080);
    cJSON_AddNumberToObject(caps, "MaxMultiProfile", 3); // 3


    cJSON_AddNumberToObject(caps, "MaxWidth", 1920);
    cJSON_AddNumberToObject(caps, "MeteringRegionCount", 0);
    cJSON_AddNumberToObject(caps, "MinExposureTime", 1);
    cJSON_AddNumberToObject(caps, "MinExposureTime1", 0);
    cJSON_AddTrueToObject(caps, "Mirror"); //true 
    cJSON_AddFalseToObject(caps, "MultiOptions");
    cJSON_AddTrueToObject(caps, "NightOptions"); //true 

    cJSON_AddFalseToObject(caps, "ReferenceLevel"); 
    cJSON_AddTrueToObject(caps, "Rotate90");  //true
    cJSON_AddTrueToObject(caps, "SetColor");  //true
    const char *str1[1] = {"Inside"}; 
    cJSON_AddItemToObject(caps, "SignalFormats", cJSON_CreateStringArray(str1, 1));
    cJSON_AddFalseToObject(caps, "SmartIRExposure");
    cJSON_AddFalseToObject(caps, "SnapshotExposure");

    cJSON_AddNumberToObject(caps, "SubChannel", 0);
    cJSON_AddTrueToObject(caps, "SupportProfile");
    cJSON_AddFalseToObject(caps, "SupportWhiteLevel");
    cJSON_AddFalseToObject(caps, "SyncChipChannels");
    cJSON_AddNumberToObject(caps, "SyncFocus", 0);
    cJSON_AddNumberToObject(caps, "TitleCount", 2);
    cJSON_AddNumberToObject(caps, "TridimDenoise", 1);

    cJSON_AddNumberToObject(caps, "TridimDenoiseDetails", 1);
    cJSON_AddNumberToObject(caps, "UTC", 0);
    cJSON_AddFalseToObject(caps, "UpScaling"); 
    cJSON_AddNumberToObject(caps, "Version", 0);
    cJSON_AddNumberToObject(caps, "VerticalBinning", 0);
    cJSON_AddNumberToObject(caps, "WhiteBalance", 3);
    cJSON_AddNumberToObject(caps, "WideDynamicRange", 1); 


    //
    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);


    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}


static int Dahua_DevVideoInput_GetCapsEx(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, int result)
{
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON *params, *caps;
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "caps", caps = cJSON_CreateObject());
    
    cJSON *ABFFunction;
    cJSON_AddItemToObject(caps, "ABFFunction", ABFFunction = cJSON_CreateObject());
    cJSON_AddFalseToObject(ABFFunction, "Support");
    
    cJSON *Defog;
    cJSON_AddItemToObject(caps, "Defog", Defog = cJSON_CreateObject());
    cJSON_AddTrueToObject(Defog, "Support"); //true

    cJSON *FaceAutoExposure;
    cJSON_AddItemToObject(caps, "FaceAutoExposure", FaceAutoExposure = cJSON_CreateObject());
    cJSON_AddFalseToObject(FaceAutoExposure, "Support");

    cJSON *ImageStabilization;
    cJSON_AddItemToObject(caps, "ImageStabilization", ImageStabilization = cJSON_CreateObject());
    cJSON_AddFalseToObject(ImageStabilization, "Support");

    cJSON *LightingControl, *Correction, *Sensitivity;
    cJSON_AddItemToObject(caps, "LightingControl", LightingControl = cJSON_CreateObject());
    const int strAngleControl[3] = {0, 0, 0};
    cJSON_AddItemToObject(LightingControl, "AngleControl", cJSON_CreateIntArray(strAngleControl, 3));
    cJSON_AddItemToObject(LightingControl, "Correction", Correction = cJSON_CreateObject());
    cJSON_AddNumberToObject(Correction, "Range", 0);
    cJSON_AddNumberToObject(Correction, "Support", 0);
    cJSON_AddStringToObject(LightingControl, "DefaultMode", "Auto");
    cJSON_AddNumberToObject(LightingControl, "FarLightNumber", 0);
    cJSON_AddNumberToObject(LightingControl, "LightType", 0);
    cJSON_AddNumberToObject(LightingControl, "MiddleLightNumber", 1);
    const char *strModes[3] = {"Manual", "Auto", "Off"};   
    cJSON_AddItemToObject(LightingControl, "Modes", cJSON_CreateStringArray(strModes, 3));
    cJSON_AddNumberToObject(LightingControl, "NearLightNumber", 0);
    const int strPower[3] = {0, 0, 0};
    cJSON_AddItemToObject(LightingControl, "Power", cJSON_CreateIntArray(strPower, 3));
    cJSON_AddItemToObject(LightingControl, "Sensitivity", Sensitivity = cJSON_CreateObject());
    cJSON_AddNumberToObject(Sensitivity, "Range", 0);
    cJSON_AddNumberToObject(Sensitivity, "Support", 0);
    cJSON_AddTrueToObject(LightingControl, "Support");

    cJSON *SensorInfo;
    cJSON_AddItemToObject(caps, "SensorInfo", SensorInfo = cJSON_CreateObject());
    cJSON_AddStringToObject(SensorInfo, "Type", "Normal");

    cJSON *VideoImageControl;
    cJSON_AddItemToObject(caps, "VideoImageControl", VideoImageControl = cJSON_CreateObject());
    cJSON_AddTrueToObject(VideoImageControl, "Flip");
    cJSON_AddTrueToObject(VideoImageControl, "Mirror");
    cJSON_AddTrueToObject(VideoImageControl, "Rotate90");
    cJSON_AddNumberToObject(VideoImageControl, "Stable", 0);
    cJSON_AddTrueToObject(VideoImageControl, "Support");
    
    cJSON *VideoInAutoContrast;
    cJSON_AddItemToObject(caps, "VideoInAutoContrast", VideoInAutoContrast = cJSON_CreateObject());
    const char *strModes2[1] = {"Disable"};
    cJSON_AddItemToObject(VideoInAutoContrast, "Modes", cJSON_CreateStringArray(strModes2, 1));
    cJSON_AddFalseToObject(VideoInAutoContrast, "Support");

    cJSON *VideoInBacklight;
    cJSON_AddItemToObject(caps, "VideoInBacklight", VideoInBacklight = cJSON_CreateObject());
    const char *strBacklightMode[2] = {"Default", "Region"};
    cJSON_AddItemToObject(VideoInBacklight, "BacklightMode", cJSON_CreateStringArray(strBacklightMode, 2));
    cJSON_AddStringToObject(VideoInBacklight, "GlareInhibition", "Default");
    const char *strVIBacklightModes[4] = {"Off","Backlight","GlareInhibition","WideDynamic"};
    cJSON_AddItemToObject(VideoInBacklight, "Modes", cJSON_CreateStringArray(strVIBacklightModes, 4));
    const char *strSSAIntensity[1] = {"Disable"};
    cJSON_AddItemToObject(VideoInBacklight, "SSAIntensity", cJSON_CreateStringArray(strSSAIntensity, 1));
    cJSON_AddTrueToObject(VideoInBacklight, "Support");
    cJSON_AddNumberToObject(VideoInBacklight, "WideDynamicRange", 1);

    cJSON *VideoInColor, *Style;
    cJSON_AddItemToObject(caps, "VideoInColor", VideoInColor = cJSON_CreateObject());
    cJSON_AddTrueToObject(VideoInColor, "Brightness");
    cJSON_AddFalseToObject(VideoInColor, "ChromaSuppress");
    cJSON_AddTrueToObject(VideoInColor, "Contrast");
    cJSON_AddTrueToObject(VideoInColor, "Gamma");
    cJSON_AddTrueToObject(VideoInColor, "Saturation");
    cJSON_AddItemToObject(VideoInColor, "Style", Style = cJSON_CreateObject());
    cJSON_AddFalseToObject(Style, "Support");
    cJSON_AddNullToObject(Style, "SupportList");
    cJSON_AddTrueToObject(VideoInColor, "Support");

    cJSON *VideoInConflict, *ConflictTable;
    cJSON_AddItemToObject(caps, "VideoInConflict", VideoInConflict = cJSON_CreateObject());
    cJSON_AddItemToObject(VideoInConflict, "ConflictTable", ConflictTable = cJSON_CreateArray());
    const char *strTmp0[2] = {"SmartEnc","SetGOP"};
    const char *strTmp1[2] = {"SmartEnc","ROI"};
    const char *strTmp2[2] = {"SmartEnc","SVC"};
    const char *strTmp3[2] = {"SmartEnc","Extra2"};
    const char *strTmp4[2] = {"SmartEnc","MJPEG"};
    const char *strTmp5[2] = {"SmartEnc","Rotate_90"};
    const char *strTmp6[2] = {"SmartEnc","TVOut"};
    cJSON_AddItemToArray(ConflictTable, cJSON_CreateStringArray(strTmp0, 2));
    cJSON_AddItemToArray(ConflictTable, cJSON_CreateStringArray(strTmp1, 2));
    cJSON_AddItemToArray(ConflictTable, cJSON_CreateStringArray(strTmp2, 2));
    cJSON_AddItemToArray(ConflictTable, cJSON_CreateStringArray(strTmp3, 2));
    cJSON_AddItemToArray(ConflictTable, cJSON_CreateStringArray(strTmp4, 2));
    cJSON_AddItemToArray(ConflictTable, cJSON_CreateStringArray(strTmp5, 2));
    cJSON_AddItemToArray(ConflictTable, cJSON_CreateStringArray(strTmp6, 2));
    cJSON_AddTrueToObject(VideoInConflict, "IsConflict"); 

    cJSON *VideoInDayNight;
    cJSON_AddItemToObject(caps, "VideoInDayNight", VideoInDayNight = cJSON_CreateObject());
    const int nDelayRange[2] = {2, 10};
    cJSON_AddItemToObject(VideoInDayNight, "DelayRange", cJSON_CreateIntArray(nDelayRange, 2));
    const char *strVIDayNightModes[3] = {"Color", "BlackWhite", "Brightness"};
    cJSON_AddItemToObject(VideoInDayNight, "Modes", cJSON_CreateStringArray(strVIDayNightModes, 3));
    cJSON_AddTrueToObject(VideoInDayNight, "Support");

    cJSON *VideoInDefog, *LightIntensity;
    cJSON_AddItemToObject(caps, "VideoInDefog", VideoInDefog = cJSON_CreateObject());
    cJSON_AddItemToObject(VideoInDefog, "LightIntensity", LightIntensity = cJSON_CreateObject());
    cJSON_AddTrueToObject(LightIntensity, "Support");
    const char *strVIDefogModes[3] = {"Off","Manual","Auto"};
    cJSON_AddItemToObject(VideoInDefog, "Modes", cJSON_CreateStringArray(strVIDefogModes, 3));
    cJSON_AddTrueToObject(VideoInDefog, "Support");

    cJSON *VideoInDenoise, *Denoise3D;
    cJSON_AddItemToObject(caps, "VideoInDenoise", VideoInDenoise = cJSON_CreateObject());
    cJSON_AddItemToObject(VideoInDenoise, "3D", Denoise3D = cJSON_CreateObject());
    cJSON_AddTrueToObject(Denoise3D, "Support");
    cJSON_AddTrueToObject(VideoInDenoise, "Support");

    cJSON *VideoInExposure, *AntiFlicker;
    cJSON_AddItemToObject(caps, "VideoInExposure", VideoInExposure = cJSON_CreateObject());
    cJSON_AddItemToObject(VideoInExposure, "AntiFlicker", AntiFlicker = cJSON_CreateObject());
    cJSON_AddTrueToObject(AntiFlicker, "Support");
    const char *strSupportList[3] = {"Outdoor","50Hz","60Hz"};
    cJSON_AddItemToObject(AntiFlicker, "SupportList", cJSON_CreateStringArray(strSupportList, 3));
    cJSON_AddFalseToObject(VideoInExposure, "IrisAuto");
    const char *strVIExposureModes[4] = {"Auto","Gain","Shutter","Manual"};
    cJSON_AddItemToObject(VideoInExposure, "Modes", cJSON_CreateStringArray(strVIExposureModes, 4));
    cJSON_AddTrueToObject(VideoInExposure, "Support");
    cJSON_AddNumberToObject(VideoInExposure, "SupportGainUpperLimit", 0);
    cJSON_AddNumberToObject(VideoInExposure, "SupportIrisRange", 0);

    cJSON *VideoInFocus;
    cJSON_AddItemToObject(caps, "VideoInFocus", VideoInFocus = cJSON_CreateObject());
    cJSON_AddFalseToObject(VideoInFocus, "SupportFocusRegion");

    cJSON *VideoInIRExposure;
    cJSON_AddItemToObject(caps, "VideoInIRExposure", VideoInIRExposure = cJSON_CreateObject());
    cJSON_AddFalseToObject(VideoInIRExposure, "Support");

    cJSON *VideoInMode;
    cJSON_AddItemToObject(caps, "VideoInMode", VideoInMode = cJSON_CreateObject());
    cJSON_AddFalseToObject(VideoInMode, "SwitchByDayNight");

    cJSON *VideoInSharpness, *Restrain;
    cJSON_AddItemToObject(caps, "VideoInSharpness", VideoInSharpness = cJSON_CreateObject());
    cJSON_AddItemToObject(VideoInSharpness, "Restrain", Restrain = cJSON_CreateObject());
    cJSON_AddFalseToObject(Restrain, "Support");
    cJSON_AddTrueToObject(VideoInSharpness, "Support");


    cJSON *VideoInWhiteBalance;
    cJSON_AddItemToObject(caps, "VideoInWhiteBalance", VideoInWhiteBalance = cJSON_CreateObject());
    const char *strVIWBModes[6] = {"Auto","Natural","StreetLamp","Outdoor","Manual","ManualDatum"};
    cJSON_AddItemToObject(VideoInWhiteBalance, "Modes", cJSON_CreateStringArray(strVIWBModes, 6));
    cJSON_AddTrueToObject(VideoInWhiteBalance, "Support");

    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);


    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}


static int Dahua_DevVideoDetect_ListMethod(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, int result)
{
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON *params, *method;
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());

    cJSON_AddItemToObject(params, "method", method = cJSON_CreateArray());
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoDetect.factory.instance"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoDetect.destroy"));

    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoDetect.getCaps"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoDetect.attachMotionData"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoDetect.detachMotionData"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoDetect.listMethod"));

    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);


    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}

static int Dahua_DevVideoDetect_GetCaps(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, int result)
{
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON *params, *caps;
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());

    cJSON_AddItemToObject(params, "caps", caps = cJSON_CreateObject());

    //caps
    const char *sDetectVersion[3] = {"V1.0", "V3.0"};   
    cJSON_AddItemToObject(caps, "DetectVersion", cJSON_CreateStringArray(sDetectVersion, 2));
    cJSON_AddNumberToObject(caps, "MotionColumns", 22);
    cJSON_AddNumberToObject(caps, "MotionDetectWindow", 4);
    cJSON_AddFalseToObject(caps, "MotionLinkPtzPattern");
    cJSON_AddFalseToObject(caps, "MotionLinkPtzPreset");
    cJSON_AddFalseToObject(caps, "MotionLinkPtzTour");
    cJSON_AddNumberToObject(caps, "MotionResult", 1);
    cJSON_AddNumberToObject(caps, "MotionRows", 18);
    cJSON_AddNumberToObject(caps, "SupportBlind", 0); // 1
    cJSON_AddNumberToObject(caps, "SupportLoss", 0);
    cJSON_AddNumberToObject(caps, "SupportMotion", 1);
    cJSON_AddNumberToObject(caps, "SupportMovedDetect", 0); // 1
    cJSON_AddNumberToObject(caps, "UnFocusDetect", 0);

    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}


static int Dahua_Ptz_ListMethod(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, int result)
{
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON *params, *method;
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());

    cJSON_AddItemToObject(params, "method", method = cJSON_CreateArray());
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.factory.instance"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.destroy"));

    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.factory.getCollect"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.getProtocolList"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.getProtocol"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.getCurrentProtocolCaps"));

    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.reset"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.moveAbsolutely"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.moveRelatively"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.moveDirectly"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.moveContinuously"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.stopMove"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.isMoving"));

    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.getStatus"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.attachStatusProc"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.detachStatusProc"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.setFocusMode"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.setFocusPoint"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.focusManually"));

    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.setIrisMode"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.setIrisValue"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.adjustIris"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.setPreset"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.getPresets"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.gotoPreset"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.removePreset"));

    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.setHomePosition"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.gotoHomePosition"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.setTour"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.getTours"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.removeTour"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.addTourPoint"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.removeTourPoint"));

    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.getTourPoints"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.startTour"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.autoTour"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.stopTour"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.startPatternRecord"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.stopPatternRecord"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.getPatterns"));

    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.removePattern"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.startPatternReplay"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.stopPatternReplay"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.menuControl"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.auxControl"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.start"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.stop"));

    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.focusContinuously"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.stopFocus"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.listMethod"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.getMotorSteps"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.setMotorSteps"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.continueMoveDirectly"));

    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.focusAbsolutely"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.focusRelatively"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.setScanLimit"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.startScan"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.stopScan"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.moveAutoPan"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.restartPtz"));

    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.restartCamera"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.markLimit"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.gotoLimit"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.enableLimit"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.attachViewRangeStatus"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.detachViewRangeStatus"));

    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.attachWeatherInfo"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.detachWeatherInfo"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.setViewRange"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.getViewRangeStatus"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.focusRegion"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.setDirection"));

    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.lensInit"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.directionCalibration"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.getOperationStatus"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.getStartupStatus"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.getLifetimeEncrypt"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.setStatisticPlan"));

    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.getZoomValue"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.gotoStatisticPlan"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ptz.addSoftTourPoint"));


    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);


    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}


static int Dahua_DevVideoEncode_ListMethod(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, int result)
{
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON *params, *method;
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());

    cJSON_AddItemToObject(params, "method", method = cJSON_CreateArray());
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.factory.instance"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.destroy"));

    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.getChipId"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.getCLB"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.getCompTypes"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.getCoverCount"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.getTitleCount"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.getMaxSnapFps"));

    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.getTempFmt"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.setTempFmt"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.resPreFmt"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.setIFrame"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.start"));

    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.stop"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.getState"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.getBitrate"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.getChipType"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.getCaps"));

    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.listMethod"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.getChipsLadenBitrate"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.getCompressionTypes"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.getTemporaryFormat"));

    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.setTemporaryFormat"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.restorePresetFormat"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoEncode.isAlive"));

    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}

static int Dahua_DevVideoEncode_GetCaps(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, int result)
{
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON *params, *caps, *encodeTypes, *bpp, *svcEncodeTypes;
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());

    cJSON_AddItemToObject(params, "caps", caps = cJSON_CreateObject());
    cJSON_AddFalseToObject(caps, "DynamicMosaic");
    cJSON_AddFalseToObject(caps, "DynamicTrackROI");
    cJSON_AddItemToObject(caps, "EncodeTypes", encodeTypes = cJSON_CreateArray());
    cJSON_AddItemToArray(encodeTypes, cJSON_CreateString("H.264"));
    cJSON_AddNumberToObject(caps, "EncryptMask", 0);
    cJSON_AddNumberToObject(caps, "MaxCoverCount", 4);
    cJSON_AddNumberToObject(caps, "MaxROICount", 0);

    cJSON_AddNumberToObject(caps, "MaxROIHeight", 0);
    cJSON_AddNumberToObject(caps, "MaxROIWidth", 0);
    cJSON_AddNumberToObject(caps, "MaxSVCTLevel", 3);
    cJSON_AddNumberToObject(caps, "MaxSnapFps", 1);
    cJSON_AddNumberToObject(caps, "MaxTitleCount", 3);

    cJSON_AddItemToObject(caps, "OverlayPicBPP", bpp = cJSON_CreateArray());
    cJSON_AddItemToArray(bpp, cJSON_CreateString("BPP8"));

    cJSON_AddNumberToObject(caps, "Smart264", 0);
    cJSON_AddFalseToObject(caps, "SnapByOtherSize");
    cJSON_AddItemToObject(caps, "SvcEncodeTypes", svcEncodeTypes = cJSON_CreateArray());
    cJSON_AddItemToArray(svcEncodeTypes, cJSON_CreateString("H.264"));


    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}


static int Dahua_DevAudioEncode_ListMethod(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, int result)
{
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON *params, *method;
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());

    cJSON_AddItemToObject(params, "method", method = cJSON_CreateArray());
    cJSON_AddItemToArray(method, cJSON_CreateString("devAudioEncode.factory.instance"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devAudioEncode.destroy"));

    cJSON_AddItemToArray(method, cJSON_CreateString("devAudioEncode.getTypes"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devAudioEncode.getTemporaryFormat"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devAudioEncode.setTemporaryFormat"));
    
    cJSON_AddItemToArray(method, cJSON_CreateString("devAudioEncode.restorePresetFormat"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devAudioEncode.start"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devAudioEncode.stop"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devAudioEncode.getState"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devAudioEncode.getFormatCaps"));

    cJSON_AddItemToArray(method, cJSON_CreateString("devAudioEncode.getStreamUri"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devAudioEncode.getCaps"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devAudioEncode.listMethod"));


    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}

static int Dahua_DevAudioEncode_GetFormatCaps(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, int result)
{
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON *params, *formats, *formats1;
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());

    cJSON_AddItemToObject(params, "formats", formats = cJSON_CreateArray());
    cJSON_AddItemToArray(formats, formats1 = cJSON_CreateObject());
    cJSON_AddItemToObject(formats1, "Compression", cJSON_CreateString("G.711A"));
    cJSON_AddItemToObject(formats1, "Depth", cJSON_CreateNumber(16));
    cJSON_AddItemToObject(formats1, "Frequency", cJSON_CreateNumber(8000));

    #if 0
    cJSON *formats2;
    cJSON_AddItemToArray(formats, formats2 = cJSON_CreateObject());
    cJSON_AddItemToObject(formats2, "Compression", cJSON_CreateString("G.711Mu"));
    cJSON_AddItemToObject(formats2, "Depth", cJSON_CreateNumber(16));
    cJSON_AddItemToObject(formats2, "Frequency", cJSON_CreateNumber(8000));
    #endif

    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}


static int Dahua_DevVideoAnalyse_ListMethod(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, int result)
{
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON *params, *method;
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());

    cJSON_AddItemToObject(params, "method", method = cJSON_CreateArray());
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoAnalyse.factory.instance"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoAnalyse.destroy"));

    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoAnalyse.getCollect"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoAnalyse.getCaps"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoAnalyse.start"));
    
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoAnalyse.stop"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoAnalyse.setModuleState"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoAnalyse.testCalibrateWithScreenPoints"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoAnalyse.getAlgorithmVersion"));

    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoAnalyse.listMethod"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoAnalyse.getTemplateRule"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoAnalyse.resetInternalOptions"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoAnalyse.getInternalOptions"));

    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoAnalyse.tuneInternalOptions"));
    cJSON_AddItemToArray(method, cJSON_CreateString("devVideoAnalyse.setAnalyseObject"));

    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}

static int Dahua_DevVideoAnalyse_GetCaps(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, int result)
{
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    //cJSON *params;
    cJSON_AddNumberToObject(root, "id", id);
    //cJSON_AddItemToObject(root, "params", cJSON_CreateNull());
    cJSON_AddFalseToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}


static int Dahua_MagicBox_ListMethod(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, int result)
{
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON *params, *method;
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());

    cJSON_AddItemToObject(params, "method", method = cJSON_CreateArray());
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.factory.instance"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.destroy"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.listMethod"));

    #if 1
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getSystemInfo"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.config"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.exit"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.restart"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.shutdown"));
    
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.reboot"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.beep"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getProcessInfo"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getDeviceType"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getDeviceClass"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getSerialNo"));

    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getCPUCount"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getCPUUsage"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getMemoryInfo"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getLocalNo"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getMachineName"));
    
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getProductDefinition"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getExitTime"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getExitState"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.isAppAutoStart"));
    
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.resetSystem"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getSystemInfoNew"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getVideoChipType"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getProcessInfoEx"));
    
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getMainBoardCount"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getCaps"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getUpTime"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getVendor"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.setDeviceParam"));

    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getSubModules"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getSoftwareVersion"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getHardwareVersion"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.get2DCode"));

    
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getEnv"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.setEnv"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getBootParameter"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.getHardwareType"));
    cJSON_AddItemToArray(method, cJSON_CreateString("magicBox.needReboot"));
    #endif

    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}

static int Dahua_MagicBox_GetProductDefinition(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, cJSON *json)
{
    char name[100] = {0};
    cJSON *pj = cJSON_GetObjectItem(json, "params");
    if (pj) {
        cJSON *cjname = cJSON_GetObjectItem(pj, "name");
        sprintf(name, "%s", cjname->valuestring);
    }
    PRINT_INFO("name:%s\n", name);

    ////
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    if (!strcmp(name, "CifPFrameSize")) {
        cJSON *params, *definition;
        cJSON_AddNumberToObject(root, "id", id);
        cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());

        cJSON_AddItemToObject(params, "definition", definition = cJSON_CreateArray());
        cJSON_AddItemToArray(definition, cJSON_CreateNumber(2));
        cJSON_AddItemToArray(definition, cJSON_CreateNumber(40));
        cJSON_AddTrueToObject(root, "result");
        cJSON_AddNumberToObject(root, "session", sid);
    } else if (!strcmp(name, "SnapshotInterval")) {
        cJSON *cjerror, *params;	
        cJSON_AddItemToObject(root, "error", cjerror = cJSON_CreateObject());
        cJSON_AddNumberToObject(cjerror, "code", 268959743);
        cJSON_AddStringToObject(cjerror, "message", "Unknown error! error code was not set in service!");
        cJSON_AddNumberToObject(root, "id", id);
        cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
        cJSON_AddNullToObject(params, "definition");
        cJSON_AddFalseToObject(root, "result");
        cJSON_AddNumberToObject(root, "session", sid);
    } else {
        PRINT_ERR("not support name:%s\n", name);
        cJSON_AddNumberToObject(root, "id", id);
        cJSON_AddFalseToObject(root, "result");
        cJSON_AddNumberToObject(root, "session", sid);
    }
    
    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}

static int Dahua_MagicBox_GetSoftwareVersion(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, cJSON *json)
{
    ////
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON *params, *version;
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    
    cJSON_AddItemToObject(params, "version", version = cJSON_CreateObject());
    cJSON_AddStringToObject(version, "Build", "20171109");
    cJSON_AddStringToObject(version, "BuildDate", "2017-11-09");

    cJSON_AddStringToObject(version, "Version", DEV_VER);
    cJSON_AddStringToObject(version, "WebVersion", DEV_WEB_VER);

    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}

static int Dahua_MagicBox_GetSerialNo(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, cJSON *json)
{
    ////
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON *params;
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    
    cJSON_AddStringToObject(params, "sn", DEV_SERIAL);

    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);


    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}

static int Dahua_MagicBox_GetDeviceClass(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, cJSON *json)
{
    ////
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON *params;
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    
    cJSON_AddStringToObject(params, "type", DEV_DEVICE_NAME);

    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);


    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}

static void Dahua_Init_Cfg_Profile()
{
    GK_NET_IMAGE_CFG image;
    memset(&image, 0, sizeof(GK_NET_IMAGE_CFG));
    goke_api_get_ioctrl(GET_IMAGE_CFG, &image);

    static int first_get_cfg_profile = 1;
    if (first_get_cfg_profile == 1) {
        PRINT_INFO("Dahua_Init_Cfg_Profile\n");
        memcpy(&tmp_image_cfg, &runImageCfg, sizeof(GK_NET_IMAGE_CFG));

        int nDayNightColor = 1;
        if (image.irCutMode == 1) { //²ÊÉ« °×Ìì
            nDayNightColor = 0;
        } else if (image.irCutMode == 2) { //ºÚ°× ÍíÉÏ
            nDayNightColor = 2;
        } else { //×Ô¶¯
            nDayNightColor = 1;
        } 
        
        int k;
        DahuaColorProfile *pColor = NULL;
        for (k = 0; k < 3; k ++) {
            pColor = &stDahuaColorProfile[k];
            pColor->enable = 0;
            pColor->brightness = 50;
            pColor->chromaSuppress = 50;
            pColor->contrast= 50;
            pColor->gamma = 50;
            pColor->hue = 50;
            pColor->saturation = 50;
            pColor->sharpness = 50;
            pColor->day_night_color = 1; //×Ô¶¯
            pColor->flip = 0; //×Ô¶¯
            pColor->mirror = 0; //×Ô¶¯
        }
        stDahuaColorProfile[0].enable = 1;
        stDahuaColorProfile[0].brightness = image.brightness;
        stDahuaColorProfile[0].contrast = image.contrast;
        stDahuaColorProfile[0].hue = image.hue;
        stDahuaColorProfile[0].saturation = image.saturation;
        stDahuaColorProfile[0].sharpness = image.sharpness;
        stDahuaColorProfile[0].day_night_color = nDayNightColor;
        stDahuaColorProfile[0].flip = image.flipEnabled;
        stDahuaColorProfile[0].mirror = image.mirrorEnabled;

        stDahuaColorProfile[0].nSwitchMode = 0;
        stDahuaColorProfile[1].nSwitchMode = 0;
        stDahuaColorProfile[2].nSwitchMode = 0;

        
        first_get_cfg_profile = 0;
    }

}

static int Dahua_ConfigManager_ListMethod(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, int result)
{
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON *params, *method;	
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "method", method = cJSON_CreateArray());
    
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.factory.instance"));
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.destroy"));
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.getConfig"));
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.setConfig"));
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.saveFile"));
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.deleteConfig"));
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.getDefault"));
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.setDefault"));

    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.getMemberNames"));
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.restore"));
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.attach"));
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.detach"));
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.restoreExcept"));
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.copyConfig"));
    
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.getConfigEx"));
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.setConfigEx"));
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.exportPackConfig"));
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.importPackConfig"));
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.setChannelConfig"));
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.getVideoAnalyseConfig"));
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.setVideoAnalyseConfig"));

    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.getFailureDetail"));
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.deleteFile"));
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.setTemporaryConfig"));
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.restoreTemporaryConfig"));
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.secGetDefault"));
    cJSON_AddItemToArray(method, cJSON_CreateString("configManager.listMethod"));

    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    Dahua_Init_Cfg_Profile();

    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}


static void add_MotionDetectWindow(cJSON *MotionDetectWindow, int index)
{
    cJSON *MotionDetectWindow0;
    cJSON_AddItemToArray(MotionDetectWindow, MotionDetectWindow0 = cJSON_CreateObject());
    
    cJSON_AddNumberToObject(MotionDetectWindow0, "Id", index);
    char tmp[20] = {0};
    sprintf(tmp, "Region%d", index + 1);
    cJSON_AddStringToObject(MotionDetectWindow0, "Name", tmp);
    int nRegion[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    if (index == 0) {
        int i = 0;
        for (i = 0; i < 18; i ++) {
            nRegion[i] = 4194303;
        }
    }
    cJSON_AddItemToObject(MotionDetectWindow0, "Region", cJSON_CreateIntArray(nRegion, 18));
    int nSensitive = runMdCfg.sensitive;
    cJSON_AddNumberToObject(MotionDetectWindow0, "Sensitive", nSensitive);
    cJSON_AddNumberToObject(MotionDetectWindow0, "Threshold", 8);

    int nWindow[4] = {0, 0, 0, 0};
    if (index == 0) {
        nWindow[2] = 8191;
        nWindow[3] = 8191;
    }
    cJSON_AddItemToObject(MotionDetectWindow0, "Window", cJSON_CreateIntArray(nWindow, 4));
}

static void add_md_MonitorTour(cJSON *MonitorTour)
{
    cJSON *MonitorTour0;

    cJSON_AddItemToArray(MonitorTour, MonitorTour0 = cJSON_CreateObject());
    cJSON_AddFalseToObject(MonitorTour0, "Enable");

}

static void add_md_ptzlink(cJSON *PtzLink)
{
    cJSON *PtzLink0;

    cJSON_AddItemToArray(PtzLink, PtzLink0 = cJSON_CreateArray());
    cJSON_AddItemToArray(PtzLink0, cJSON_CreateString("None"));
    cJSON_AddItemToArray(PtzLink0, cJSON_CreateNumber(0));
}

//flag == 1, get default
static int Dahua_Cfg_GetMotionDetect(int id, int sid, cJSON *root, int flag)
{
    //
    cJSON *params, *table;   
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "table", table = cJSON_CreateObject());

    cJSON_AddStringToObject(table, "DetectVersion", "V3.0");

    //table
    if (flag == 1) {
        cJSON_AddTrueToObject(table, "Enable");
    } else {
        if (runMdCfg.enable == 1)
            cJSON_AddTrueToObject(table, "Enable");
        else
            cJSON_AddFalseToObject(table, "Enable");
    }

    cJSON *EventHandler;
    const int tmp[1] = {0};

    cJSON_AddItemToObject(table, "EventHandler", EventHandler = cJSON_CreateObject());

    cJSON_AddNumberToObject(EventHandler, "AlarmOut", 1); // 1
    cJSON_AddItemToObject(EventHandler, "AlarmOutChannels", cJSON_CreateIntArray(tmp, 1));

    if (flag == 1) {
        cJSON_AddTrueToObject(EventHandler, "AlarmOutEnable");
    } else {
        if (runMdCfg.handle.is_alarmout == 1)
            cJSON_AddTrueToObject(EventHandler, "AlarmOutEnable");
        else
            cJSON_AddFalseToObject(EventHandler, "AlarmOutEnable");
    }
    
    cJSON_AddNumberToObject(EventHandler, "AlarmOutLatch", 10);

    if (flag == 1) {
        cJSON_AddFalseToObject(EventHandler, "BeepEnable");
    } else {
        if (runMdCfg.handle.is_beep == 1)
            cJSON_AddTrueToObject(EventHandler, "BeepEnable");
        else
           cJSON_AddFalseToObject(EventHandler, "BeepEnable");
    }

    cJSON_AddNumberToObject(EventHandler, "Dejitter", 30);
    cJSON_AddNumberToObject(EventHandler, "Delay", 0);

    cJSON_AddNumberToObject(EventHandler, "ExAlarmOut", 1); // 1
    cJSON_AddItemToObject(EventHandler, "ExAlarmOutChannels", cJSON_CreateIntArray(tmp, 1));
    cJSON_AddFalseToObject(EventHandler, "ExAlarmOutEnable");

    #if 0
    if (flag == 1) {
        cJSON_AddFalseToObject(EventHandler, "FTPEnable");
    } else {
        if (runMdCfg.handle.isSnapUploadToFtp == 1)
            cJSON_AddTrueToObject(EventHandler, "FTPEnable");
        else
           cJSON_AddFalseToObject(EventHandler, "FTPEnable");
    }
    #endif

    cJSON_AddFalseToObject(EventHandler, "FlashEnable");
    cJSON_AddNumberToObject(EventHandler, "FlashLatch", 10);

    cJSON *LightingLink;
    cJSON_AddItemToObject(EventHandler, "LightingLink", LightingLink = cJSON_CreateObject());
    cJSON_AddFalseToObject(LightingLink, "Enable");
    cJSON_AddNumberToObject(LightingLink, "FilckerIntevalTime", 5);
    cJSON_AddStringToObject(LightingLink, "FilckerLightType", "WhiteLight");
    cJSON_AddNumberToObject(LightingLink, "FilckerTimes", 5);

    cJSON_AddTrueToObject(EventHandler, "LogEnable");
    //cJSON_AddFalseToObject(EventHandler, "MMSEnable");

    if (flag == 1) {
        cJSON_AddFalseToObject(EventHandler, "MailEnable");
    } else {
        if (runMdCfg.handle.is_email == 1)
            cJSON_AddTrueToObject(EventHandler, "MailEnable");
        else
            cJSON_AddFalseToObject(EventHandler, "MailEnable");
    }

    cJSON_AddNumberToObject(EventHandler, "Matrix", 1);
    cJSON_AddItemToObject(EventHandler, "MatrixChannels", cJSON_CreateIntArray(tmp, 1));
    cJSON_AddFalseToObject(EventHandler, "MatrixEnable");
    //cJSON_AddFalseToObject(EventHandler, "MessageEnable");
    cJSON_AddTrueToObject(EventHandler, "MessageEnable");
    //cJSON_AddNumberToObject(EventHandler, "MessageToNetEnable", 0);

    cJSON *MonitorTour;
    cJSON_AddItemToObject(EventHandler, "MonitorTour", MonitorTour = cJSON_CreateArray());
    add_md_MonitorTour(MonitorTour);
    add_md_MonitorTour(MonitorTour);

    cJSON *PtzLink;
    cJSON_AddItemToObject(EventHandler, "PtzLink", PtzLink = cJSON_CreateArray());
    int m;
    for (m = 0; m < 16; m ++) {
        add_md_ptzlink(PtzLink);
    }
    cJSON_AddFalseToObject(EventHandler, "PtzLinkEnable");

    cJSON_AddNumberToObject(EventHandler, "Record", 1);
    cJSON_AddItemToObject(EventHandler, "RecordChannels", cJSON_CreateIntArray(tmp, 1));

    if (flag == 1) {
        cJSON_AddFalseToObject(EventHandler, "RecordEnable");
    } else {
        if (runMdCfg.handle.is_rec == 1)
            cJSON_AddTrueToObject(EventHandler, "RecordEnable");
        else
            cJSON_AddFalseToObject(EventHandler, "RecordEnable");
    }

    cJSON_AddNumberToObject(EventHandler, "RecordLatch", 10);

    cJSON_AddNumberToObject(EventHandler, "Snapshot", 1);
    cJSON_AddItemToObject(EventHandler, "SnapshotChannels", cJSON_CreateIntArray(tmp, 1));
    if (flag == 1) {
        cJSON_AddFalseToObject(EventHandler, "SnapshotEnable");
    } else {
        if (runMdCfg.handle.is_snap == 1)
            cJSON_AddTrueToObject(EventHandler, "SnapshotEnable");
        else
            cJSON_AddFalseToObject(EventHandler, "SnapshotEnable");
    }

    cJSON_AddNumberToObject(EventHandler, "SnapshotPeriod", 0);
    cJSON_AddNumberToObject(EventHandler, "SnapshotTimes", 1);


    cJSON *TimeSection;
    cJSON_AddItemToObject(EventHandler,"TimeSection", TimeSection = cJSON_CreateArray());
    int day = 0, i = 0;
	for (day = 0; day < 7; day++) {
        #if 1
		char strings1[6][20] = {{0}}; //6¸öÊ±¼ä¶Î
		for(i = 0; i < 4; i++)
		{

			sprintf(strings1[i], "1 %02u:%02u:00-%02u:%02u:00", 
									runMdCfg.scheduleTime[day][i].startHour,
									runMdCfg.scheduleTime[day][i].startMin,
									runMdCfg.scheduleTime[day][i].stopHour,
									runMdCfg.scheduleTime[day][i].stopMin);

		}
		sprintf(strings1[4], "%s", "0 00:00:00-00:00:00");
		sprintf(strings1[5], "%s", "0 00:00:00-00:00:00");

		cJSON *tmp = cJSON_CreateArray();
		for(i = 0; i < 6; i++)
		{
			cJSON_AddItemToArray(tmp, cJSON_CreateString(strings1[i]));
		}		
		cJSON_AddItemToArray(TimeSection, tmp);
        #else
	    const char *strings1[6]={
	                            "1 00:00:00-24:00:00", 
	                            "0 00:00:00-24:00:00", 
	                            "0 00:00:00-24:00:00", 
	                            "0 00:00:00-24:00:00", 
	                            "0 00:00:00-24:00:00", 
	                            "0 00:00:00-24:00:00"};	
		cJSON_AddItemToArray(TimeSection, cJSON_CreateStringArray(strings1, 6));
        #endif
	}

    cJSON_AddFalseToObject(EventHandler, "TipEnable");

    cJSON_AddNumberToObject(EventHandler, "Tour", 1);
    cJSON_AddItemToObject(EventHandler, "TourChannels", cJSON_CreateIntArray(tmp, 1));

    if (flag == 1) {
        cJSON_AddFalseToObject(EventHandler, "TourEnable");
    } else {
        if (runMdCfg.handle.is_ptz == 1)
            cJSON_AddTrueToObject(EventHandler, "TourEnable");
        else
            cJSON_AddFalseToObject(EventHandler, "TourEnable");
    }

    cJSON_AddFalseToObject(EventHandler, "VoiceEnable");

    cJSON_AddNumberToObject(table, "Level", 3);

    cJSON *MotionDetectWindow;
    cJSON_AddItemToObject(table, "MotionDetectWindow", MotionDetectWindow = cJSON_CreateArray());
    add_MotionDetectWindow(MotionDetectWindow, 0);
    add_MotionDetectWindow(MotionDetectWindow, 1);
    add_MotionDetectWindow(MotionDetectWindow, 2);
    add_MotionDetectWindow(MotionDetectWindow, 3);

    cJSON_AddFalseToObject(table, "OsdTwinkleEnable");
    cJSON_AddNumberToObject(table, "PirMotionLevel", 4);
    cJSON_AddTrueToObject(table, "PtzManualEnable");

    int nRegion[18] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
    int k = 0;
    for (k = 0; k < 18; k ++) {
        nRegion[k] = 4194303;
    }
    cJSON_AddItemToObject(table, "Region", cJSON_CreateIntArray(nRegion, 18));

    //
    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    return 0;
}

static int Dahua_Cfg_GetVideoEncodeROI(int id, int sid, cJSON *root)
{
    //
    cJSON *params, *table;   
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "table", table = cJSON_CreateObject());

    //table
    cJSON_AddFalseToObject(table, "DynamicTrack");
    cJSON_AddFalseToObject(table, "Extra1");
    cJSON_AddFalseToObject(table, "Extra2");
    cJSON_AddFalseToObject(table, "Extra3");
    cJSON_AddTrueToObject(table, "Main");
    cJSON_AddNumberToObject(table, "Quality", 6);
    cJSON_AddFalseToObject(table, "Snapshot");

    //
    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    return 0;
}

static int Dahua_Encode_ListMethod(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, int result)
{
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON *params, *method;	
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "method", method = cJSON_CreateArray());
    
    cJSON_AddItemToArray(method, cJSON_CreateString("encode.factory.instance"));
    cJSON_AddItemToArray(method, cJSON_CreateString("encode.destroy"));
    
    cJSON_AddItemToArray(method, cJSON_CreateString("encode.getConfigCaps"));
    cJSON_AddItemToArray(method, cJSON_CreateString("encode.getCaps"));
    cJSON_AddItemToArray(method, cJSON_CreateString("encode.saveMode"));
    cJSON_AddItemToArray(method, cJSON_CreateString("encode.setDefinitionMode"));

    cJSON_AddItemToArray(method, cJSON_CreateString("encode.getDefinitionMode"));
    cJSON_AddItemToArray(method, cJSON_CreateString("encode.updateOverlayPicture"));
    cJSON_AddItemToArray(method, cJSON_CreateString("encode.listMethod"));
    cJSON_AddItemToArray(method, cJSON_CreateString("encode.getEncodeType"));
    cJSON_AddItemToArray(method, cJSON_CreateString("encode.getSmartCaps"));

    cJSON_AddItemToArray(method, cJSON_CreateString("encode.getCropCaps"));
    cJSON_AddItemToArray(method, cJSON_CreateString("encode.getNAACaps"));


    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}

static int Dahua_Cfg_GetMemberNames(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, int result)
{
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON *params, *method;	
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "names", method = cJSON_CreateArray());
    
    cJSON_AddItemToArray(method, cJSON_CreateString("ARP&Ping"));
    cJSON_AddItemToArray(method, cJSON_CreateString("AccessFilter"));
    cJSON_AddItemToArray(method, cJSON_CreateString("AccessPoint"));
    cJSON_AddItemToArray(method, cJSON_CreateString("AlarmServer"));
    //cJSON_AddItemToArray(method, cJSON_CreateString("AudioDetect"));
    cJSON_AddItemToArray(method, cJSON_CreateString("AudioInDenoise"));
    cJSON_AddItemToArray(method, cJSON_CreateString("AudioInput"));
    cJSON_AddItemToArray(method, cJSON_CreateString("AudioInputVolume"));

    cJSON_AddItemToArray(method, cJSON_CreateString("AudioOutputVolume"));
    cJSON_AddItemToArray(method, cJSON_CreateString("AutoMaintain"));
    cJSON_AddItemToArray(method, cJSON_CreateString("BackupCompact"));
    cJSON_AddItemToArray(method, cJSON_CreateString("BatteryLowPowerAlarm"));
    cJSON_AddItemToArray(method, cJSON_CreateString("BlindDetect"));
    cJSON_AddItemToArray(method, cJSON_CreateString("Bonjour"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ChannelTitle"));

    cJSON_AddItemToArray(method, cJSON_CreateString("Comm"));
    cJSON_AddItemToArray(method, cJSON_CreateString("DDNS"));
    cJSON_AddItemToArray(method, cJSON_CreateString("DVRIP"));
    cJSON_AddItemToArray(method, cJSON_CreateString("DenoiseParam"));
    cJSON_AddItemToArray(method, cJSON_CreateString("DeviceInfo"));
    cJSON_AddItemToArray(method, cJSON_CreateString("EPtzPreset"));
    cJSON_AddItemToArray(method, cJSON_CreateString("EPtzTour"));

    cJSON_AddItemToArray(method, cJSON_CreateString("Email"));
    cJSON_AddItemToArray(method, cJSON_CreateString("EmergencyRecordForPull"));
    cJSON_AddItemToArray(method, cJSON_CreateString("Encode"));
    cJSON_AddItemToArray(method, cJSON_CreateString("EncodeAuth"));
    cJSON_AddItemToArray(method, cJSON_CreateString("EncodeEncrypt"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ExAlarm"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ExAlarmOut"));

    cJSON_AddItemToArray(method, cJSON_CreateString("ExRemoteCtrl"));
    cJSON_AddItemToArray(method, cJSON_CreateString("FaceSnapshot"));
    cJSON_AddItemToArray(method, cJSON_CreateString("FlashLight"));
    cJSON_AddItemToArray(method, cJSON_CreateString("FlashPartitions"));
    cJSON_AddItemToArray(method, cJSON_CreateString("General"));
    cJSON_AddItemToArray(method, cJSON_CreateString("Holiday"));
    cJSON_AddItemToArray(method, cJSON_CreateString("IPConflict"));
    cJSON_AddItemToArray(method, cJSON_CreateString("IPv6"));
    cJSON_AddItemToArray(method, cJSON_CreateString("IntelliTrackScene"));

    cJSON_AddItemToArray(method, cJSON_CreateString("LDCorrection"));
    cJSON_AddItemToArray(method, cJSON_CreateString("Language"));
    cJSON_AddItemToArray(method, cJSON_CreateString("LeAudioLocate"));
    cJSON_AddItemToArray(method, cJSON_CreateString("LeCollectionPoint"));
    cJSON_AddItemToArray(method, cJSON_CreateString("LeCruisePlan"));
    cJSON_AddItemToArray(method, cJSON_CreateString("LeHeadDetect"));
    cJSON_AddItemToArray(method, cJSON_CreateString("LeSmartTrack"));

    cJSON_AddItemToArray(method, cJSON_CreateString("LeTimedCruise"));
    cJSON_AddItemToArray(method, cJSON_CreateString("LightGlobal"));
    cJSON_AddItemToArray(method, cJSON_CreateString("Lighting"));
    cJSON_AddItemToArray(method, cJSON_CreateString("Locales"));
    cJSON_AddItemToArray(method, cJSON_CreateString("LoginFailureAlarm"));
    cJSON_AddItemToArray(method, cJSON_CreateString("LossDetect"));
    cJSON_AddItemToArray(method, cJSON_CreateString("MacConflict"));

    cJSON_AddItemToArray(method, cJSON_CreateString("MediaFileReaderGlobal"));
    cJSON_AddItemToArray(method, cJSON_CreateString("MediaGlobal"));
    cJSON_AddItemToArray(method, cJSON_CreateString("MobilePushNotification"));
    cJSON_AddItemToArray(method, cJSON_CreateString("MotionDetect"));
    cJSON_AddItemToArray(method, cJSON_CreateString("MovedDetect"));
    cJSON_AddItemToArray(method, cJSON_CreateString("MultiRTPDH"));
    cJSON_AddItemToArray(method, cJSON_CreateString("MultiTemperatureOSD"));
    cJSON_AddItemToArray(method, cJSON_CreateString("MultiVideoSensor"));
    cJSON_AddItemToArray(method, cJSON_CreateString("Multicast"));
    cJSON_AddItemToArray(method, cJSON_CreateString("MusicPlayer"));


    cJSON_AddItemToArray(method, cJSON_CreateString("NAS"));
    cJSON_AddItemToArray(method, cJSON_CreateString("NTP"));
    cJSON_AddItemToArray(method, cJSON_CreateString("NetAbort"));
    cJSON_AddItemToArray(method, cJSON_CreateString("NetAccessPriority"));
    cJSON_AddItemToArray(method, cJSON_CreateString("Network"));
    cJSON_AddItemToArray(method, cJSON_CreateString("OnvifAnalytics"));
    cJSON_AddItemToArray(method, cJSON_CreateString("OnvifDevice"));

    cJSON_AddItemToArray(method, cJSON_CreateString("OnvifMedia"));
    cJSON_AddItemToArray(method, cJSON_CreateString("OnvifPTZ"));
    cJSON_AddItemToArray(method, cJSON_CreateString("P2PLimit"));
    cJSON_AddItemToArray(method, cJSON_CreateString("PSIA"));
    cJSON_AddItemToArray(method, cJSON_CreateString("PaaSDevRegion"));
    cJSON_AddItemToArray(method, cJSON_CreateString("PaaS_ENCRYPT_INFO"));
    cJSON_AddItemToArray(method, cJSON_CreateString("PaaS_UPGRADE"));

    cJSON_AddItemToArray(method, cJSON_CreateString("PortsReuse"));
    cJSON_AddItemToArray(method, cJSON_CreateString("PrivacyMasking"));
    cJSON_AddItemToArray(method, cJSON_CreateString("Ptz"));
    cJSON_AddItemToArray(method, cJSON_CreateString("PtzDevice"));
    cJSON_AddItemToArray(method, cJSON_CreateString("PtzExtern"));
    cJSON_AddItemToArray(method, cJSON_CreateString("PtzPreset"));
    cJSON_AddItemToArray(method, cJSON_CreateString("PtzTour"));

    cJSON_AddItemToArray(method, cJSON_CreateString("Qos"));
    cJSON_AddItemToArray(method, cJSON_CreateString("RTSP"));
    cJSON_AddItemToArray(method, cJSON_CreateString("Raid"));
    cJSON_AddItemToArray(method, cJSON_CreateString("RainBrush"));
    cJSON_AddItemToArray(method, cJSON_CreateString("RainBrushMode"));
    cJSON_AddItemToArray(method, cJSON_CreateString("Record"));
    cJSON_AddItemToArray(method, cJSON_CreateString("RecordMode"));

    cJSON_AddItemToArray(method, cJSON_CreateString("RecordStoragePoint"));
    cJSON_AddItemToArray(method, cJSON_CreateString("ReplaceWideDynamicRangeMode"));
    cJSON_AddItemToArray(method, cJSON_CreateString("SSHD"));
    cJSON_AddItemToArray(method, cJSON_CreateString("Smart433Dev"));
    cJSON_AddItemToArray(method, cJSON_CreateString("SmartDevList"));
    cJSON_AddItemToArray(method, cJSON_CreateString("SmartEncode"));
    cJSON_AddItemToArray(method, cJSON_CreateString("Snap"));
    cJSON_AddItemToArray(method, cJSON_CreateString("SnapMode"));
    cJSON_AddItemToArray(method, cJSON_CreateString("Sound"));

    cJSON_AddItemToArray(method, cJSON_CreateString("StereoCalibrate"));
    cJSON_AddItemToArray(method, cJSON_CreateString("StereoVideoStream"));
    cJSON_AddItemToArray(method, cJSON_CreateString("StorageFailure"));
    cJSON_AddItemToArray(method, cJSON_CreateString("StorageGlobal"));
    cJSON_AddItemToArray(method, cJSON_CreateString("StorageGroup"));
    cJSON_AddItemToArray(method, cJSON_CreateString("StorageHealthAlarm"));
    cJSON_AddItemToArray(method, cJSON_CreateString("StorageLowSpace"));

    cJSON_AddItemToArray(method, cJSON_CreateString("StorageNotExist"));
    cJSON_AddItemToArray(method, cJSON_CreateString("StorageQuota"));
    cJSON_AddItemToArray(method, cJSON_CreateString("StreamAuthority"));
    cJSON_AddItemToArray(method, cJSON_CreateString("Syslog"));
    cJSON_AddItemToArray(method, cJSON_CreateString("T2UServer"));
    cJSON_AddItemToArray(method, cJSON_CreateString("Talkback"));
    cJSON_AddItemToArray(method, cJSON_CreateString("Telnet"));

    cJSON_AddItemToArray(method, cJSON_CreateString("TwoRefEncode"));
    cJSON_AddItemToArray(method, cJSON_CreateString("UPnP"));
    cJSON_AddItemToArray(method, cJSON_CreateString("UserGlobal"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VSP_GAYS"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VSP_PaaS"));
    //cJSON_AddItemToArray(method, cJSON_CreateString("VideoAnalyseGlobal"));
    //cJSON_AddItemToArray(method, cJSON_CreateString("VideoAnalyseModule"));
    //cJSON_AddItemToArray(method, cJSON_CreateString("VideoAnalyseRule"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VideoColor"));
    //cJSON_AddItemToArray(method, cJSON_CreateString("VideoEncodeROI"));


    cJSON_AddItemToArray(method, cJSON_CreateString("VideoImageControl"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VideoInBacklight"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VideoInColor"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VideoInDayNight"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VideoInDefog"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VideoInDenoise"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VideoInExposure"));

    cJSON_AddItemToArray(method, cJSON_CreateString("VideoInFocus"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VideoInIRExposure"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VideoInMetering"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VideoInMode"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VideoInOptions"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VideoInPreviewOptions"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VideoInRotate"));

    cJSON_AddItemToArray(method, cJSON_CreateString("VideoInSceneMode"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VideoInSensor"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VideoInSharpness"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VideoInWhiteBalance"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VideoInZoom"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VideoOut"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VideoStandard"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VideoSubColor"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VideoWaterMark"));
    cJSON_AddItemToArray(method, cJSON_CreateString("VideoWidget"));

    cJSON_AddItemToArray(method, cJSON_CreateString("WLan"));
    cJSON_AddItemToArray(method, cJSON_CreateString("WMM"));
    cJSON_AddItemToArray(method, cJSON_CreateString("Web"));
    cJSON_AddItemToArray(method, cJSON_CreateString("Wire802_1X"));
    cJSON_AddItemToArray(method, cJSON_CreateString("_CE_Player_"));
    cJSON_AddItemToArray(method, cJSON_CreateString("_DHCloudUpgradeRecord_"));
    cJSON_AddItemToArray(method, cJSON_CreateString("_DHCloudUpgrade_"));


    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}


/////////////// instance /////////////////

static int Dahua_Alarm_GetAllInSlots(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, int result)
{
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON *cjerror, *params;	
    cJSON_AddItemToObject(root, "error", cjerror = cJSON_CreateObject());
    cJSON_AddNumberToObject(cjerror, "code", 268632080);
    cJSON_AddStringToObject(cjerror, "message", "");
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddNullToObject(params, "inputs");
    cJSON_AddFalseToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}


static int Dahua_Instance(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, int result)
{
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddNumberToObject(root, "result", 42070872);
    cJSON_AddNumberToObject(root, "session", sid);

    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);

    return 0;
}

static int Dahua_System_ListMethod(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, int result)
{
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON *cjerror, *params;	
    cJSON_AddItemToObject(root, "error", cjerror = cJSON_CreateObject());
    cJSON_AddNumberToObject(cjerror, "code", 268959743);
    cJSON_AddStringToObject(cjerror, "message", "Unknown error! error code was not set in service!");
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddNullToObject(params, "table");
    cJSON_AddFalseToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}

static void add_CapsExtraFormat(cJSON *extraFormat)
{
    cJSON *extraFormat1;
    cJSON *e_audio, *e_a_compressionTypes;
    cJSON *e_video, *e_bitRateOptions, *e_v_compressionTypes, *e_h264Profile, *e_resolutionTypes;

    cJSON_AddItemToArray(extraFormat, extraFormat1 = cJSON_CreateObject());
    cJSON_AddItemToObject(extraFormat1, "Audio", e_audio = cJSON_CreateObject());
    cJSON_AddItemToObject(e_audio, "CompressionTypes", e_a_compressionTypes = cJSON_CreateArray());
    cJSON_AddItemToArray(e_a_compressionTypes, cJSON_CreateString("G.711A"));
    //cJSON_AddItemToArray(e_a_compressionTypes, cJSON_CreateString("G.711Mu"));
    //cJSON_AddItemToArray(e_a_compressionTypes, cJSON_CreateString("AAC"));

    cJSON_AddItemToObject(extraFormat1, "Video", e_video = cJSON_CreateObject());
    cJSON_AddItemToObject(e_video, "BitRateOptions", e_bitRateOptions = cJSON_CreateArray());
    cJSON_AddItemToArray(e_bitRateOptions, cJSON_CreateNumber(256));
    cJSON_AddItemToArray(e_bitRateOptions, cJSON_CreateNumber(2048));
    cJSON_AddItemToObject(e_video, "BitRateSuggested", cJSON_CreateNumber(512));
    cJSON_AddItemToObject(e_video, "CompressionTypes", e_v_compressionTypes = cJSON_CreateArray());
    cJSON_AddItemToArray(e_v_compressionTypes, cJSON_CreateString("H.264"));
    //cJSON_AddItemToArray(e_v_compressionTypes, cJSON_CreateString("MJPG"));
    //cJSON_AddItemToArray(e_v_compressionTypes, cJSON_CreateString("H.265"));
    cJSON_AddItemToObject(e_video, "FPSMax", cJSON_CreateNumber(25));
    cJSON_AddItemToObject(e_video, "H264Profile", e_h264Profile = cJSON_CreateArray());
    cJSON_AddItemToArray(e_h264Profile, cJSON_CreateString("High"));
    cJSON_AddItemToObject(e_video, "MaxROICount", cJSON_CreateNumber(0));
    cJSON_AddItemToObject(e_video, "ResolutionTypes", e_resolutionTypes = cJSON_CreateArray());
    cJSON_AddItemToArray(e_resolutionTypes, cJSON_CreateString("VGA"));
    cJSON_AddItemToArray(e_resolutionTypes, cJSON_CreateString("QVGA"));
    //cJSON_AddItemToArray(e_resolutionTypes, cJSON_CreateString("640x360"));

}


static void add_CapsMainFormat(cJSON *mainFormat)
{
    cJSON *mainFormat1;
    cJSON *m_audio, *m_a_compressionTypes;
    cJSON *m_video, *m_bitRateOptions, *m_v_compressionTypes, *m_h264Profile, *m_resolutionTypes;
    cJSON_AddItemToArray(mainFormat, mainFormat1 = cJSON_CreateObject());
    
    cJSON_AddItemToObject(mainFormat1, "Audio", m_audio = cJSON_CreateObject());
    cJSON_AddItemToObject(m_audio, "CompressionTypes", m_a_compressionTypes = cJSON_CreateArray());
    cJSON_AddItemToArray(m_a_compressionTypes, cJSON_CreateString("G.711A"));
    //cJSON_AddItemToArray(m_a_compressionTypes, cJSON_CreateString("G.711Mu"));
    //cJSON_AddItemToArray(m_a_compressionTypes, cJSON_CreateString("AAC"));

    cJSON_AddItemToObject(mainFormat1, "Video", m_video = cJSON_CreateObject());
    cJSON_AddItemToObject(m_video, "BitRateOptions", m_bitRateOptions = cJSON_CreateArray());
    cJSON_AddItemToArray(m_bitRateOptions, cJSON_CreateNumber(512)); //diff
    cJSON_AddItemToArray(m_bitRateOptions, cJSON_CreateNumber(4096));
    cJSON_AddItemToObject(m_video, "BitRateSuggested", cJSON_CreateNumber(2048)); //diff
    cJSON_AddItemToObject(m_video, "CompressionTypes", m_v_compressionTypes = cJSON_CreateArray());
    cJSON_AddItemToArray(m_v_compressionTypes, cJSON_CreateString("H.264"));
    //cJSON_AddItemToArray(e_v_compressionTypes, cJSON_CreateString("MJPG"));
    //cJSON_AddItemToArray(e_v_compressionTypes, cJSON_CreateString("H.265"));
    cJSON_AddItemToObject(m_video, "FPSMax", cJSON_CreateNumber(25));
    cJSON_AddItemToObject(m_video, "H264Profile", m_h264Profile = cJSON_CreateArray());
    cJSON_AddItemToArray(m_h264Profile, cJSON_CreateString("High"));
    cJSON_AddItemToObject(m_video, "MaxROICount", cJSON_CreateNumber(0));
    cJSON_AddItemToObject(m_video, "ResolutionTypes", m_resolutionTypes = cJSON_CreateArray());
    cJSON_AddItemToArray(m_resolutionTypes, cJSON_CreateString("1080P")); //diff
    //cJSON_AddItemToArray(m_resolutionTypes, cJSON_CreateString("SXGA")); // 1280 * 1024
    cJSON_AddItemToArray(m_resolutionTypes, cJSON_CreateString("1_3M")); // 1280 * 960
    cJSON_AddItemToArray(m_resolutionTypes, cJSON_CreateString("720P"));

}

static void add_CapsSnapFormat(cJSON *snapFormat)
{
    cJSON *snapFormat1;
    cJSON *snapVideo, *snapCompressionTypes, *snapResolutionTypes;

    cJSON_AddItemToArray(snapFormat, snapFormat1 = cJSON_CreateObject());

    cJSON_AddFalseToObject(snapFormat1, "SupportIndividualResolution");
    cJSON_AddItemToObject(snapFormat1, "Video", snapVideo = cJSON_CreateObject());
    cJSON_AddItemToObject(snapVideo, "CompressionTypes", snapCompressionTypes = cJSON_CreateArray());
    cJSON_AddItemToArray(snapCompressionTypes, cJSON_CreateString("MJPG"));
    cJSON_AddNullToObject(snapVideo, "H264Profile");
    cJSON_AddItemToObject(snapVideo, "ResolutionTypes", snapResolutionTypes = cJSON_CreateArray());
    cJSON_AddItemToArray(snapResolutionTypes, cJSON_CreateString("1080P"));
    //cJSON_AddItemToArray(snapResolutionTypes, cJSON_CreateString("SXGA")); 
    cJSON_AddItemToArray(snapResolutionTypes, cJSON_CreateString("1_3M"));
    cJSON_AddItemToArray(snapResolutionTypes, cJSON_CreateString("720P"));
    cJSON_AddItemToArray(snapResolutionTypes, cJSON_CreateString("VGA"));
    cJSON_AddItemToArray(snapResolutionTypes, cJSON_CreateString("QVGA"));
}


static int Dahua_Encode_GetConfigCaps(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, int result)
{
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON *params, *caps, *caps1, *bitRateRange;

    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "caps", caps = cJSON_CreateArray());
    cJSON_AddItemToArray(caps, caps1 = cJSON_CreateObject());
    
    cJSON_AddItemToObject(caps1, "BitRateRange", bitRateRange = cJSON_CreateArray());
    cJSON_AddItemToArray(bitRateRange, cJSON_CreateNumber(3));
    cJSON_AddItemToArray(bitRateRange, cJSON_CreateNumber(14848));

    /* ExtraFormat */
    cJSON *extraFormat;
    cJSON_AddItemToObject(caps1, "ExtraFormat", extraFormat = cJSON_CreateArray());
    add_CapsExtraFormat(extraFormat);



    /* MainFormat */
    cJSON *mainFormat;
    cJSON_AddItemToObject(caps1, "MainFormat", mainFormat = cJSON_CreateArray());
    add_CapsMainFormat(mainFormat);
    add_CapsMainFormat(mainFormat);
    add_CapsMainFormat(mainFormat);

    /* SnapFormat */
    cJSON *snapFormat;
    cJSON_AddItemToObject(caps1, "SnapFormat", snapFormat = cJSON_CreateArray());
    add_CapsSnapFormat(snapFormat); // ×¥°ü£¬·¢ÁË3´Î
    add_CapsSnapFormat(snapFormat);
    add_CapsSnapFormat(snapFormat);


    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;

}


//flag == 1, get default
static void add_Color(cJSON *table00, int index, int flag)
{
    if (flag == 1) {
        cJSON_AddNumberToObject(table00, "Brightness", 50);
        cJSON_AddNumberToObject(table00, "ChromaSuppress", 50);
        cJSON_AddNumberToObject(table00, "Contrast", 50);
        cJSON_AddNumberToObject(table00, "Gamma", 50);
        cJSON_AddNumberToObject(table00, "Hue", 50);
        cJSON_AddNumberToObject(table00, "Saturation", 50);
        cJSON_AddStringToObject(table00, "Style", "Standard");
        int enable = 0;
        if (index == 0)
            enable = 1;
        char str[100] = {0};
        sprintf(str, "%d 00:00:00-24:00:00", enable);
        cJSON_AddStringToObject(table00, "TimeSection", str);
    } else {
        DahuaColorProfile *pColor = &stDahuaColorProfile[index];

        cJSON_AddNumberToObject(table00, "Brightness", pColor->brightness);
        cJSON_AddNumberToObject(table00, "ChromaSuppress", pColor->chromaSuppress);
        cJSON_AddNumberToObject(table00, "Contrast", pColor->contrast);
        cJSON_AddNumberToObject(table00, "Gamma", pColor->gamma);
        cJSON_AddNumberToObject(table00, "Hue", pColor->hue);
        cJSON_AddNumberToObject(table00, "Saturation", pColor->saturation);
        cJSON_AddStringToObject(table00, "Style", "Standard");
        char str[100] = {0};
        sprintf(str, "%d 00:00:00-24:00:00", pColor->enable);
        cJSON_AddStringToObject(table00, "TimeSection", str);
    }
}

//flag==1, get default
static int Dahua_Cfg_GetVideoColor(int id, int sid, cJSON *root, int flag)
{
    cJSON *params, *table;   
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "table", table = cJSON_CreateArray());

    cJSON *table0, *table00, *table01, *table02; 
    cJSON_AddItemToArray(table, table0 = cJSON_CreateArray());

    cJSON_AddItemToArray(table0, table00 = cJSON_CreateObject());
    add_Color(table00, 0, flag);

    cJSON_AddItemToArray(table0, table01 = cJSON_CreateObject());
    add_Color(table01, 1, flag);

    cJSON_AddItemToArray(table0, table02 = cJSON_CreateObject());
    add_Color(table02, 2, flag);

    
    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    return 0;
}

static int Dahua_Cfg_GetVideoInSharpness(int id, int sid, cJSON *root)
{
    GK_NET_IMAGE_CFG image;
    memset(&image, 0, sizeof(GK_NET_IMAGE_CFG));
    goke_api_get_ioctrl(GET_IMAGE_CFG, &image);

    //
    cJSON *params, *table;   
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "table", table = cJSON_CreateArray());

    cJSON *table0, *table00; 
    cJSON_AddItemToArray(table, table0 = cJSON_CreateArray());

    cJSON_AddItemToArray(table0, table00 = cJSON_CreateObject());
    cJSON_AddNumberToObject(table00, "Level", 50);
    cJSON_AddNumberToObject(table00, "Mode", 1);
    cJSON_AddNumberToObject(table00, "Sharpness", image.sharpness);

    #if 1
    cJSON *table01; 
    cJSON_AddItemToArray(table0, table01 = cJSON_CreateObject());
    cJSON_AddNumberToObject(table01, "Level", 50);
    cJSON_AddNumberToObject(table01, "Mode", 1);
    cJSON_AddNumberToObject(table01, "Sharpness", 50);

    cJSON *table02; 
    cJSON_AddItemToArray(table0, table02 = cJSON_CreateObject());
    cJSON_AddNumberToObject(table02, "Level", 50);
    cJSON_AddNumberToObject(table02, "Mode", 1);
    cJSON_AddNumberToObject(table02, "Sharpness", 50);

    #endif
    
    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    return 0;
}

static void add_VideoInDenoise(cJSON *table00, int index)
{
    cJSON *AutoType3D, *ManulType3D, *DenoiseAlgorithm1; 

    cJSON_AddTrueToObject(table00, "2DEnable");
    cJSON_AddNumberToObject(table00, "2DLevel", 3);
    
    cJSON_AddItemToObject(table00, "3DAutoType", AutoType3D = cJSON_CreateObject());
    cJSON_AddNumberToObject(AutoType3D, "AutoLevel", 50);
    
    cJSON_AddItemToObject(table00, "3DManulType", ManulType3D = cJSON_CreateObject());
    cJSON_AddNumberToObject(ManulType3D, "SnfLevel", 50);
    cJSON_AddNumberToObject(ManulType3D, "TnfLevel", 50);

    if (index == 0) {
        GK_NET_IMAGE_CFG image;
        memset(&image, 0, sizeof(GK_NET_IMAGE_CFG));
        goke_api_get_ioctrl(GET_IMAGE_CFG, &image);

        if (image.enableDenoise3d)
            cJSON_AddStringToObject(table00, "3DType", "Auto");
        else
            cJSON_AddStringToObject(table00, "3DType", "Off");

    } else {
        cJSON_AddStringToObject(table00, "3DType", "Auto");
    }

    cJSON_AddItemToObject(table00, "DenoiseAlgorithm1", DenoiseAlgorithm1 = cJSON_CreateObject());
    cJSON_AddNumberToObject(DenoiseAlgorithm1, "SnfLevel", 50);
    cJSON_AddNumberToObject(DenoiseAlgorithm1, "TnfLevel", 50);
    cJSON_AddStringToObject(DenoiseAlgorithm1, "Type", "Manual");

}

static int Dahua_Cfg_GetVideoInDenoise(int id, int sid, cJSON *root)
{
    cJSON *params, *table;   
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "table", table = cJSON_CreateArray());

    cJSON *table0; 
    cJSON_AddItemToArray(table, table0 = cJSON_CreateArray());

    cJSON *table00;
    cJSON_AddItemToArray(table0, table00 = cJSON_CreateObject());
    add_VideoInDenoise(table00, 0);

    cJSON *table01;
    cJSON_AddItemToArray(table0, table01 = cJSON_CreateObject());
    add_VideoInDenoise(table01, 1);
    cJSON *table02;
    cJSON_AddItemToArray(table0, table02 = cJSON_CreateObject());
    add_VideoInDenoise(table02, 2);
    
    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    return 0;
}

//flag == 1, get default
static void add_DayNightOptions(cJSON *options, int flag, int index, int flip, int mirror)
{
    cJSON_AddNumberToObject(options, "AlarmDayNightColorMode", 0);
    cJSON_AddNumberToObject(options, "AntiFlicker", 0);

    cJSON_AddNumberToObject(options, "Backlight", 0);
    const int nBacklightRegion[4] = {3096, 3096, 5096, 5096};
    cJSON_AddItemToObject(options, "BacklightRegion", cJSON_CreateIntArray(nBacklightRegion, 4));
    cJSON_AddNumberToObject(options, "BrightnessThreshold", 50);

    if (flag == 1) {
        cJSON_AddNumberToObject(options, "DayNightColor", 1);
    } else {
        cJSON_AddNumberToObject(options, "DayNightColor", stDahuaColorProfile[index].day_night_color);
    }

    cJSON_AddNumberToObject(options, "DayNightSensitivity", 2);
    cJSON_AddNumberToObject(options, "DayNightSwitchDelay", 6);

    cJSON_AddNumberToObject(options, "ExposureIris", 50);
    cJSON_AddNumberToObject(options, "ExposureMode", 0);
    cJSON_AddNumberToObject(options, "ExposureSpeed", 0);
    cJSON_AddNumberToObject(options, "ExposureValue1", 0);
    cJSON_AddNumberToObject(options, "ExposureValue2", 40);
    cJSON_AddNumberToObject(options, "ExternalSyncPhase", 125);

    if (flag == 1) {
        cJSON_AddFalseToObject(options, "Flip");
    } else {
        if (flip  == 1)
            cJSON_AddTrueToObject(options, "Flip");
        else
            cJSON_AddFalseToObject(options, "Flip");
    }

    cJSON_AddNumberToObject(options, "Gain", 50);
    cJSON_AddTrueToObject(options, "GainAuto");
    cJSON_AddNumberToObject(options, "GainBlue", 50);
    cJSON_AddNumberToObject(options, "GainGreen", 50);
    cJSON_AddNumberToObject(options, "GainMax", 50);
    cJSON_AddNumberToObject(options, "GainMin", 0);
    cJSON_AddNumberToObject(options, "GainRed", 50);
    cJSON_AddNumberToObject(options, "GlareInhibition", 0);
    cJSON_AddFalseToObject(options, "InfraRed");
    cJSON_AddNumberToObject(options, "InfraRedLevel", 0);
    cJSON_AddTrueToObject(options, "IrisAuto");
    cJSON_AddNumberToObject(options, "IrisAutoSensitivity", 50);

    if (flag == 1) {
        cJSON_AddFalseToObject(options, "Mirror");
    } else {
        if (mirror == 1)
            cJSON_AddTrueToObject(options, "Mirror");
        else
            cJSON_AddFalseToObject(options, "Mirror");
    }

    if (index == 1)
        cJSON_AddNumberToObject(options, "Profile", 3);
    else
        cJSON_AddNumberToObject(options, "Profile", 0);
    cJSON_AddNumberToObject(options, "ReferenceLevel", 50);
    //cJSON_AddNumberToObject(options, "Rotate90", is_rotate90);
    cJSON_AddNumberToObject(options, "Rotate90", 0);
    cJSON_AddNumberToObject(options, "SunriseHour", 0);
    cJSON_AddNumberToObject(options, "SunriseMinute", 0);
    cJSON_AddNumberToObject(options, "SunriseSecond", 0);
    cJSON_AddNumberToObject(options, "SunsetHour", 23);
    cJSON_AddNumberToObject(options, "SunsetMinute", 59);
    cJSON_AddNumberToObject(options, "SunsetSecond", 59);

    if (flag == 1) {
        cJSON_AddNumberToObject(options, "SwitchMode", 0);
    } else {
        cJSON_AddNumberToObject(options, "SwitchMode", stDahuaColorProfile[index].nSwitchMode);
    }

    cJSON *TemporaryConfigs;
    cJSON_AddItemToObject(options, "TemporaryConfigs", TemporaryConfigs = cJSON_CreateObject());
    cJSON_AddNumberToObject(TemporaryConfigs, "AntiFlicker", 0);
    cJSON_AddNumberToObject(TemporaryConfigs, "ExposureMode", 0);
    cJSON_AddNumberToObject(TemporaryConfigs, "ExposureValue1", 0.100000); //0.100000
    cJSON_AddNumberToObject(TemporaryConfigs, "ExposureValue2", 80);
    cJSON_AddNumberToObject(TemporaryConfigs, "GainMax", 50);
    cJSON_AddNumberToObject(TemporaryConfigs, "GainMin", 10);

    cJSON_AddStringToObject(options, "WhiteBalance", "Auto");
    const int nWhiteBalanceDatumRect[4] = {0, 0, 1000, 1000};
    cJSON_AddItemToObject(options, "WhiteBalanceDatumRect", cJSON_CreateIntArray(nWhiteBalanceDatumRect, 4));

    cJSON_AddNumberToObject(options, "WideDynamicRange", 0);
    cJSON_AddNumberToObject(options, "WideDynamicRangeMode", 0);
}

// flag == 1, get default
static int Dahua_Cfg_GetVideoInOptions(int id, int sid, cJSON *root, int flag)
{
    GK_NET_IMAGE_CFG image;
    memset(&image, 0, sizeof(GK_NET_IMAGE_CFG));
    goke_api_get_ioctrl(GET_IMAGE_CFG, &image);

    //
    cJSON *params, *table;   
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "table", table = cJSON_CreateArray());

    cJSON *table0; 
    cJSON_AddItemToArray(table, table0 = cJSON_CreateObject());

    cJSON_AddNumberToObject(table0, "AlarmDayNightColorMode", 0);
    cJSON_AddNumberToObject(table0, "AntiFlicker", 0);
    cJSON_AddFalseToObject(table0, "AutoSyncPhase");
    cJSON_AddNumberToObject(table0, "Backlight", 0);
    const int nBacklightRegion[4] = {3096, 3096, 5096, 5096};
    cJSON_AddItemToObject(table0, "BacklightRegion", cJSON_CreateIntArray(nBacklightRegion, 4));
    cJSON_AddNumberToObject(table0, "ColorTemperatureLevel", 100);

    if (flag == 1) {
        cJSON_AddNumberToObject(table0, "DayNightColor", 1);
    } else {
        PRINT_INFO("day_night_color, %d-%d-%d", stDahuaColorProfile[0].day_night_color,
            stDahuaColorProfile[1].day_night_color,
            stDahuaColorProfile[2].day_night_color);
        cJSON_AddNumberToObject(table0, "DayNightColor", stDahuaColorProfile[0].day_night_color);
    }

    cJSON_AddNumberToObject(table0, "DayNightSensitivity", 2);
    cJSON_AddNumberToObject(table0, "DayNightSwitchDelay", 6);
    cJSON_AddNumberToObject(table0, "DoubleExposure", 0);
    cJSON *DuskOptions; 
    cJSON_AddItemToObject(table0, "DuskOptions", DuskOptions = cJSON_CreateObject());
    cJSON_AddNumberToObject(DuskOptions, "ExternalSyncPhase", 125);
    cJSON_AddNumberToObject(table0, "ExposureIris", 50);
    cJSON_AddNumberToObject(table0, "ExposureMode", 0);
    cJSON_AddNumberToObject(table0, "ExposureSpeed", 0);
    cJSON_AddNumberToObject(table0, "ExposureValue1", 0);
    cJSON_AddNumberToObject(table0, "ExposureValue2", 40);
    cJSON_AddNumberToObject(table0, "ExternalSync", 0);
    cJSON_AddNumberToObject(table0, "ExternalSyncPhase", 125);
    cJSON *FlashControl; 
    cJSON_AddItemToObject(table0, "FlashControl", FlashControl = cJSON_CreateObject());
    cJSON_AddNumberToObject(FlashControl, "Mode", 0);
    cJSON_AddNumberToObject(FlashControl, "Pole", 0);
    cJSON_AddNumberToObject(FlashControl, "PreValue", 40);
    cJSON_AddNumberToObject(FlashControl, "Value", 0);

    if (flag == 1) {
        cJSON_AddFalseToObject(table0, "Flip");
    } else {
        if (image.flipEnabled == 1)
            cJSON_AddTrueToObject(table0, "Flip");
        else
            cJSON_AddFalseToObject(table0, "Flip");
    }

    cJSON_AddNumberToObject(table0, "FocusMode", 0);
    const int nFocusRect[4] = {0, 0, 8191, 8191};
    cJSON_AddItemToObject(table0, "FocusRect", cJSON_CreateIntArray(nFocusRect, 4));
    cJSON_AddNumberToObject(table0, "Gain", 50);
    cJSON_AddTrueToObject(table0, "GainAuto");
    cJSON_AddNumberToObject(table0, "GainBlue", 50);
    cJSON_AddNumberToObject(table0, "GainGreen", 50);
    cJSON_AddNumberToObject(table0, "GainMax", 50);
    cJSON_AddNumberToObject(table0, "GainMin", 0);
    cJSON_AddNumberToObject(table0, "GainRed", 50);
    cJSON_AddNumberToObject(table0, "GlareInhibition", 0);
    cJSON_AddFalseToObject(table0, "InfraRed");
    cJSON_AddNumberToObject(table0, "InfraRedLevel", 0);
    cJSON_AddTrueToObject(table0, "IrisAuto");
    cJSON_AddNumberToObject(table0, "IrisAutoSensitivity", 50);

    if (flag == 1) {
        cJSON_AddFalseToObject(table0, "Mirror");
    } else {
        if (image.mirrorEnabled == 1)
            cJSON_AddTrueToObject(table0, "Mirror");
        else
            cJSON_AddFalseToObject(table0, "Mirror");
    }

    int is_rotate90 = 0;
    cJSON *NightOptions;
    cJSON_AddItemToObject(table0, "NightOptions", NightOptions = cJSON_CreateObject());
    add_DayNightOptions(NightOptions, flag, 1, image.flipEnabled, image.mirrorEnabled);

    cJSON *NormalOptions; 
    cJSON_AddItemToObject(table0, "NormalOptions", NormalOptions = cJSON_CreateObject());
    add_DayNightOptions(NormalOptions, flag, 2, image.flipEnabled, image.mirrorEnabled);

    cJSON_AddNumberToObject(table0, "ReferenceLevel", 50);
    cJSON_AddFalseToObject(table0, "ReferenceLevelEnable");
    if (flag == 1) {
        cJSON_AddNumberToObject(table0, "Rotate90", 0);
    } else {
        cJSON_AddNumberToObject(table0, "Rotate90", is_rotate90);
    }
    cJSON_AddStringToObject(table0, "SignalFormat", "Inside");
    cJSON *Snapshot; 
    cJSON_AddItemToObject(table0, "Snapshot", Snapshot = cJSON_CreateObject());
    cJSON_AddNumberToObject(Snapshot, "ColorTemperatureLevel", 50);
    cJSON_AddNumberToObject(Snapshot, "ExposureSpeed", 0);
    cJSON_AddNumberToObject(Snapshot, "ExposureValue1", 0);
    cJSON_AddNumberToObject(Snapshot, "ExposureValue2", 40);
    cJSON_AddNumberToObject(Snapshot, "Gain", 50);
    cJSON_AddTrueToObject(Snapshot, "GainAuto");
    cJSON_AddNumberToObject(Snapshot, "GainBlue", 50);
    cJSON_AddNumberToObject(Snapshot, "GainGreen", 50);
    cJSON_AddNumberToObject(Snapshot, "GainRed", 50);
    cJSON_AddStringToObject(Snapshot, "WhiteBalance", "Disable");

    if (flag == 1) {
        cJSON_AddNumberToObject(table0, "SwitchMode", 0);
    } else {
        cJSON_AddNumberToObject(table0, "SwitchMode", stDahuaColorProfile[0].nSwitchMode);
    }
    
    cJSON *TemporaryConfigs;
    cJSON_AddItemToObject(table0, "TemporaryConfigs", TemporaryConfigs = cJSON_CreateObject());
    cJSON_AddNumberToObject(TemporaryConfigs, "AntiFlicker", 0);
    cJSON_AddNumberToObject(TemporaryConfigs, "ExposureMode", 0);
    cJSON_AddNumberToObject(TemporaryConfigs, "ExposureValue1", 0.100000); //0.100000
    cJSON_AddNumberToObject(TemporaryConfigs, "ExposureValue2", 80);
    cJSON_AddNumberToObject(TemporaryConfigs, "GainMax", 50);
    cJSON_AddNumberToObject(TemporaryConfigs, "GainMin", 10);

    cJSON_AddStringToObject(table0, "WhiteBalance", "Auto");
    const int nWhiteBalanceDatumRect[4] = {0, 0, 1000, 1000};
    cJSON_AddItemToObject(table0, "WhiteBalanceDatumRect", cJSON_CreateIntArray(nWhiteBalanceDatumRect, 4));

    cJSON_AddNumberToObject(table0, "WideDynamicRange", 0);
    cJSON_AddNumberToObject(table0, "WideDynamicRangeMode", 0);

    //
    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    return 0;
}

static int Dahua_Cfg_GetVideoInMode(int id, int sid, cJSON *root)
{
    GK_NET_IMAGE_CFG image;
    memset(&image, 0, sizeof(GK_NET_IMAGE_CFG));
    goke_api_get_ioctrl(GET_IMAGE_CFG, &image);

    //
    cJSON *params, *table;   
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "table", table = cJSON_CreateArray());

    cJSON *table0, *TimeSection; 
    cJSON_AddItemToArray(table, table0 = cJSON_CreateObject());
    const int nIndex[1] = {0};
    cJSON_AddItemToObject(table0, "Config", cJSON_CreateIntArray(nIndex, 1));
    cJSON_AddNumberToObject(table0, "Mode", 0);
    cJSON_AddItemToObject(table0, "TimeSection", TimeSection = cJSON_CreateArray());


    const char *strTime[6] = {"0 00:00:00-24:00:00","0 00:00:00-24:00:00","0 00:00:00-24:00:00","0 00:00:00-24:00:00","0 00:00:00-24:00:00","0 00:00:00-24:00:00"};
    int i = 0;
    for (i = 0; i < 7; i++) {
        cJSON_AddItemToArray(TimeSection, cJSON_CreateStringArray(strTime, 6));
    }

    //
    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    return 0;
}


static int Dahua_Cfg_GetVideoImageControl(int id, int sid, cJSON *root)
{
    GK_NET_IMAGE_CFG image;
    memset(&image, 0, sizeof(GK_NET_IMAGE_CFG));
    goke_api_get_ioctrl(GET_IMAGE_CFG, &image);

    //
    cJSON *params, *table;   
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "table", table = cJSON_CreateArray());

    cJSON *table0; 
    cJSON_AddItemToArray(table, table0 = cJSON_CreateObject());

    if (image.flipEnabled)
        cJSON_AddTrueToObject(table0, "Flip");
    else
        cJSON_AddFalseToObject(table0, "Flip");

    cJSON_AddFalseToObject(table0, "Freeze");

    if (image.mirrorEnabled)
        cJSON_AddTrueToObject(table0, "Mirror");
    else
        cJSON_AddFalseToObject(table0, "Mirror");

    cJSON_AddNumberToObject(table0, "Rotate90", 0);
    cJSON_AddNumberToObject(table0, "Stable", 0);

    //
    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    return 0;
}

static void add_VideoInWhiteBalance(cJSON *table00, int index)
{
    cJSON_AddNumberToObject(table00, "ColorTemperatureLevel", 50);
    cJSON_AddNumberToObject(table00, "GainBlue", 50);
    cJSON_AddNumberToObject(table00, "GainGreen", 50);
    cJSON_AddNumberToObject(table00, "GainRed", 50);
    const int str[4] = {0,0,1000,1000};
    cJSON_AddItemToObject(table00, "ManualDatumRect", cJSON_CreateIntArray(str, 4));

    cJSON_AddStringToObject(table00, "Mode", "Auto");
}

static void add_VideoInExposure(cJSON *item)
{
    cJSON_AddNumberToObject(item, "AntiFlicker", 0);
    cJSON_AddNumberToObject(item, "AutoGainMax", 2);
    cJSON_AddNumberToObject(item, "Backlight", 0);
    const int nBacklightRegion[4] = {3096, 3096, 5096, 5096};
    cJSON_AddItemToObject(item, "BacklightRegion", cJSON_CreateIntArray(nBacklightRegion, 4));
    cJSON_AddNumberToObject(item, "Compensation", 50);
    cJSON_AddNumberToObject(item, "DoubleExposure", 0);

    cJSON_AddNumberToObject(item, "Gain", 1);
    cJSON_AddNumberToObject(item, "GainMax", 50);
    cJSON_AddNumberToObject(item, "GainMin", 0);
    cJSON_AddNumberToObject(item, "GlareInhibition", 0);
    
    cJSON_AddNumberToObject(item, "Iris", 50);
    cJSON_AddTrueToObject(item, "IrisAuto");
    cJSON_AddNumberToObject(item, "IrisMax", 50);
    cJSON_AddNumberToObject(item, "IrisMin", 0);
    cJSON_AddNumberToObject(item, "Mode", 0);

    cJSON_AddNumberToObject(item, "RecoveryTime", 900);
    const int nRect[4] = {0, 0, 0, 0};
    cJSON_AddItemToObject(item, "Rect", cJSON_CreateIntArray(nRect, 4));
    cJSON_AddNumberToObject(item, "SlowAutoExposure", 0);
    cJSON_AddFalseToObject(item, "SlowShutter");
    cJSON_AddNumberToObject(item, "SlowSpeed", 25);
    
    cJSON_AddNumberToObject(item, "Speed", 0);
    cJSON_AddNumberToObject(item, "Value1", 0);
    cJSON_AddNumberToObject(item, "Value2", 40);
    cJSON_AddNumberToObject(item, "WideDynamicRange", 0);
    cJSON_AddNumberToObject(item, "WideDynamicRangeMode", 0);

}

static int Dahua_Cfg_GetVideoInExposure(int id, int sid, cJSON *root)
{
    //
    cJSON *params, *table;   
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "table", table = cJSON_CreateArray());

    cJSON *table0; 
    cJSON_AddItemToArray(table, table0 = cJSON_CreateArray());

    cJSON *item0; 
    cJSON_AddItemToArray(table0, item0 = cJSON_CreateObject());
    add_VideoInExposure(item0);

    cJSON *item1; 
    cJSON_AddItemToArray(table0, item1 = cJSON_CreateObject());
    add_VideoInExposure(item1);

    cJSON *item2; 
    cJSON_AddItemToArray(table0, item2 = cJSON_CreateObject());
    add_VideoInExposure(item2);

    //
    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    return 0;
}

static void add_VideoInBacklight(cJSON *item)
{
    cJSON_AddStringToObject(item, "BacklightMode", "Default");
    const int nBacklightRegion[4] = {3096, 3096, 5096, 5096};
    cJSON_AddItemToObject(item, "BacklightRegion", cJSON_CreateIntArray(nBacklightRegion, 4));
    cJSON_AddNumberToObject(item, "GlareInhibition", 50);
    cJSON_AddNumberToObject(item, "Intensity", 50);
    cJSON_AddStringToObject(item, "IntensityMode", "Disable");
    cJSON_AddStringToObject(item, "Mode", "Off");
    cJSON_AddNumberToObject(item, "WideDynamicRange", 50);
}

static int Dahua_Cfg_GetVideoInBacklight(int id, int sid, cJSON *root)
{
    //
    cJSON *params, *table;   
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "table", table = cJSON_CreateArray());

    cJSON *table0; 
    cJSON_AddItemToArray(table, table0 = cJSON_CreateArray());

    cJSON *item0; 
    cJSON_AddItemToArray(table0, item0 = cJSON_CreateObject());
    add_VideoInBacklight(item0);

    cJSON *item1; 
    cJSON_AddItemToArray(table0, item1 = cJSON_CreateObject());
    add_VideoInBacklight(item1);

    cJSON *item2; 
    cJSON_AddItemToArray(table0, item2 = cJSON_CreateObject());
    add_VideoInBacklight(item2);
  
    //
    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    return 0;
}


static int Dahua_Cfg_GetVideoInWhiteBalance(int id, int sid, cJSON *root)
{
    GK_NET_IMAGE_CFG image;
    memset(&image, 0, sizeof(GK_NET_IMAGE_CFG));
    goke_api_get_ioctrl(GET_IMAGE_CFG, &image);

    //
    cJSON *params, *table;   
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "table", table = cJSON_CreateArray());

    cJSON *table0; 
    cJSON_AddItemToArray(table, table0 = cJSON_CreateObject());

    cJSON *item0; 
    cJSON_AddItemToArray(table0, item0 = cJSON_CreateObject());
    add_VideoInWhiteBalance(item0, 0);

    cJSON *item1; 
    cJSON_AddItemToArray(table0, item1 = cJSON_CreateObject());
    add_VideoInWhiteBalance(item1, 1);

    cJSON *item2; 
    cJSON_AddItemToArray(table0, item2 = cJSON_CreateObject());
    add_VideoInWhiteBalance(item2, 2);

  
    //
    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    return 0;
}

static void add_VideoInDayNight(cJSON *table00, int index)
{
    cJSON_AddNumberToObject(table00, "Delay", 6);

    // "Color","BlackWhite","Brightness"
    PRINT_INFO("daynight-- mode:%d\n", stDahuaColorProfile[index].day_night_color);
    
    if (stDahuaColorProfile[index].day_night_color == 0)
        cJSON_AddStringToObject(table00, "Mode", "Color"); //°×Ìì£¬²ÊÉ«
    else if (stDahuaColorProfile[index].day_night_color == 2)
        cJSON_AddStringToObject(table00, "Mode", "BlackWhite"); //ÍíÉÏ£¬ºÚ°×
    else
        cJSON_AddStringToObject(table00, "Mode", "Brightness"); //×Ô¶¯
    
    cJSON_AddNumberToObject(table00, "Sensitivity", 2);
    cJSON_AddStringToObject(table00, "Type", "Mechanism");
}


static int Dahua_Cfg_GetVideoInDayNight(int id, int sid, cJSON *root)
{
    //
    cJSON *params, *table;   
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "table", table = cJSON_CreateArray());

    cJSON *table0; 
    cJSON_AddItemToArray(table, table0 = cJSON_CreateObject());

    cJSON *item0; 
    cJSON_AddItemToArray(table0, item0 = cJSON_CreateObject());
    add_VideoInDayNight(item0, 0);

    cJSON *item1; 
    cJSON_AddItemToArray(table0, item1 = cJSON_CreateObject());
    add_VideoInDayNight(item1, 1);

    cJSON *item2; 
    cJSON_AddItemToArray(table0, item2 = cJSON_CreateObject());
    add_VideoInDayNight(item2, 2);

    //
    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    return 0;
}

static int Dahua_Cfg_GetChannelTitle(int id, int sid, cJSON *root)
{
    //
    cJSON *params, *table;   
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "table", table = cJSON_CreateArray());

    //table
    cJSON *item;
    cJSON_AddItemToArray(table, item = cJSON_CreateObject());
    cJSON_AddStringToObject(item, "Name", runChannelCfg.channelInfo[0].osdChannelName.text);

    //
    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    return 0;
}

static int Dahua_Cfg_GetVideoAnalyseRule(int id, int sid, cJSON *root)
{
    cJSON *params, *table;   
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "table", table = cJSON_CreateArray());
    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    return 0;
}


static void add_EncMainFormat(cJSON *mainFormat)
{
    cJSON *mainFormat1, *m_audio1, *m_a_channels1, *m_video1;
    cJSON_AddItemToArray(mainFormat, mainFormat1 = cJSON_CreateObject());
    cJSON_AddItemToObject(mainFormat1, "Audio", m_audio1 = cJSON_CreateObject());
    cJSON_AddItemToObject(m_audio1, "Bitrate", cJSON_CreateNumber(64));
    cJSON_AddItemToObject(m_audio1, "Channels", m_a_channels1 = cJSON_CreateArray());
    cJSON_AddItemToArray(m_a_channels1, cJSON_CreateNumber(0));
    cJSON_AddItemToObject(m_audio1, "Compression", cJSON_CreateString("G.711A"));
    cJSON_AddItemToObject(m_audio1, "Depth", cJSON_CreateNumber(16));
    cJSON_AddItemToObject(m_audio1, "Frequency", cJSON_CreateNumber(8000));
    cJSON_AddItemToObject(m_audio1, "Mode", cJSON_CreateNumber(0));
    cJSON_AddItemToObject(m_audio1, "Pack", cJSON_CreateString("DHAV"));
    cJSON_AddItemToObject(mainFormat1, "AudioEnable", cJSON_CreateTrue());
    
    cJSON_AddItemToObject(mainFormat1, "Video", m_video1 = cJSON_CreateObject());
    cJSON_AddNumberToObject(m_video1, "BitRate", runVideoCfg.vencStream[0].h264Conf.bps);
    if (runVideoCfg.vencStream[0].h264Conf.rc_mode == 0)
        cJSON_AddItemToObject(m_video1, "BitRateControl", cJSON_CreateString("CBR"));
    else
        cJSON_AddItemToObject(m_video1, "BitRateControl", cJSON_CreateString("VBR"));
    cJSON_AddItemToObject(m_video1, "Compression", cJSON_CreateString("H.264"));
    if (runVideoCfg.vencStream[0].h264Conf.height == 720)
        cJSON_AddItemToObject(m_video1, "CustomResolutionName", cJSON_CreateString("720P"));
    else if (runVideoCfg.vencStream[0].h264Conf.height == 960)
        cJSON_AddItemToObject(m_video1, "CustomResolutionName", cJSON_CreateString("1.3M"));
    else
        cJSON_AddItemToObject(m_video1, "CustomResolutionName", cJSON_CreateString("1080P"));
    
    cJSON_AddNumberToObject(m_video1, "FPS", runVideoCfg.vencStream[0].h264Conf.fps);
    cJSON_AddNumberToObject(m_video1, "GOP", runVideoCfg.vencStream[0].h264Conf.gop);
    cJSON_AddNumberToObject(m_video1, "Height", runVideoCfg.vencStream[0].h264Conf.height);
    cJSON_AddStringToObject(m_video1, "Pack", "DHAV");
    cJSON_AddNumberToObject(m_video1, "Priority", 0);
    cJSON_AddStringToObject(m_video1, "Profile", "High");
    cJSON_AddNumberToObject(m_video1, "Quality", runVideoCfg.vencStream[0].h264Conf.quality + 1);
    //cJSON_AddNumberToObject(m_video1, "QualityRange", 6);
    cJSON_AddNumberToObject(m_video1, "QualityRange", 4);
    cJSON_AddNumberToObject(m_video1, "SVCTLayer", 1);
    cJSON_AddNumberToObject(m_video1, "Width", runVideoCfg.vencStream[0].h264Conf.width);
    cJSON_AddTrueToObject(mainFormat1, "VideoEnable");

}

static void add_EncExtraFormat(cJSON *extraFormat, int v_enable, int a_enable)
{
    cJSON *extraFormat1, *e_audio, *e_a_channels, *e_video;
    cJSON_AddItemToArray(extraFormat, extraFormat1 = cJSON_CreateObject());

    //audio
    cJSON_AddItemToObject(extraFormat1, "Audio", e_audio = cJSON_CreateObject());
    cJSON_AddItemToObject(e_audio, "Bitrate", cJSON_CreateNumber(64));
    cJSON_AddItemToObject(e_audio, "Channels", e_a_channels = cJSON_CreateArray());
    cJSON_AddItemToArray(e_a_channels, cJSON_CreateNumber(0));
    cJSON_AddItemToObject(e_audio, "Compression", cJSON_CreateString("G.711A"));
    cJSON_AddItemToObject(e_audio, "Depth", cJSON_CreateNumber(16));
    cJSON_AddItemToObject(e_audio, "Frequency", cJSON_CreateNumber(8000));
    cJSON_AddItemToObject(e_audio, "Pack", cJSON_CreateString("DHAV"));

    if (a_enable)
        cJSON_AddTrueToObject(extraFormat1, "AudioEnable");
    else
        cJSON_AddFalseToObject(extraFormat1, "AudioEnable");

    //video
    cJSON_AddItemToObject(extraFormat1, "Video", e_video = cJSON_CreateObject());
    cJSON_AddNumberToObject(e_video, "BitRate", runVideoCfg.vencStream[1].h264Conf.bps);
    if (runVideoCfg.vencStream[1].h264Conf.bps == 0)
        cJSON_AddItemToObject(e_video, "BitRateControl", cJSON_CreateString("CBR"));
    else
        cJSON_AddItemToObject(e_video, "BitRateControl", cJSON_CreateString("VBR"));
    cJSON_AddItemToObject(e_video, "Compression", cJSON_CreateString("H.264"));
    if (runVideoCfg.vencStream[1].h264Conf.height == 480)
        cJSON_AddItemToObject(e_video, "CustomResolutionName", cJSON_CreateString("VGA"));
    else if (runVideoCfg.vencStream[1].h264Conf.height == 240)
        cJSON_AddItemToObject(e_video, "CustomResolutionName", cJSON_CreateString("QVGA"));
    else
        cJSON_AddItemToObject(e_video, "CustomResolutionName", cJSON_CreateString("VGA"));
    cJSON_AddNumberToObject(e_video, "FPS", runVideoCfg.vencStream[1].h264Conf.fps);
    cJSON_AddNumberToObject(e_video, "GOP", runVideoCfg.vencStream[1].h264Conf.gop);
    cJSON_AddNumberToObject(e_video, "Height", runVideoCfg.vencStream[1].h264Conf.height);
    cJSON_AddStringToObject(e_video, "Pack", "DHAV");
    cJSON_AddNumberToObject(e_video, "Priority", 0);
    cJSON_AddStringToObject(e_video, "Profile", "High");
    cJSON_AddNumberToObject(e_video, "Quality", runVideoCfg.vencStream[1].h264Conf.quality + 1);
    //cJSON_AddNumberToObject(e_video, "QualityRange", 6);
    cJSON_AddNumberToObject(e_video, "QualityRange", 4);
    cJSON_AddNumberToObject(e_video, "SVCTLayer", 1);
    cJSON_AddNumberToObject(e_video, "Width", runVideoCfg.vencStream[1].h264Conf.width);

    if (v_enable)
        cJSON_AddTrueToObject(extraFormat1, "VideoEnable");
    else
        cJSON_AddFalseToObject(extraFormat1, "VideoEnable");
}

static void add_EncSnapFormat(cJSON *snapFormat)
{
    cJSON *snapFormat1, *snapAudio1, *snapVideo1;
    cJSON_AddItemToArray(snapFormat, snapFormat1 = cJSON_CreateObject());
    cJSON_AddItemToObject(snapFormat1, "Audio", snapAudio1 = cJSON_CreateObject());
    cJSON_AddItemToObject(snapAudio1, "Bitrate", cJSON_CreateNumber(64));
    cJSON_AddItemToObject(snapAudio1, "Compression", cJSON_CreateString("G.711A"));
    cJSON_AddItemToObject(snapAudio1, "Depth", cJSON_CreateNumber(16));
    cJSON_AddItemToObject(snapAudio1, "Frequency", cJSON_CreateNumber(8000));
    cJSON_AddItemToObject(snapAudio1, "Pack", cJSON_CreateString("DHAV"));
    cJSON_AddFalseToObject(snapFormat1, "AudioEnable");
    
    cJSON_AddItemToObject(snapFormat1, "Video", snapVideo1 = cJSON_CreateObject());
    cJSON_AddNumberToObject(snapVideo1, "BitRate", 5120);
    cJSON_AddItemToObject(snapVideo1, "BitRateControl", cJSON_CreateString("VBR"));
    cJSON_AddItemToObject(snapVideo1, "Compression", cJSON_CreateString("MJPG"));
    cJSON_AddItemToObject(snapVideo1, "CustomResolutionName", cJSON_CreateString("1080P"));
    cJSON_AddNumberToObject(snapVideo1, "FPS", 1.0);
    cJSON_AddNumberToObject(snapVideo1, "GOP", 3);
    cJSON_AddNumberToObject(snapVideo1, "Height", 1080);
    cJSON_AddStringToObject(snapVideo1, "Pack", "DHAV");
    cJSON_AddNumberToObject(snapVideo1, "Priority", 0);
    cJSON_AddStringToObject(snapVideo1, "Profile", "Main");
    //cJSON_AddNumberToObject(snapVideo1, "Quality", 5);
    //cJSON_AddNumberToObject(snapVideo1, "QualityRange", 6);
    cJSON_AddNumberToObject(snapVideo1, "Quality", 3);
    cJSON_AddNumberToObject(snapVideo1, "QualityRange", 4);
    cJSON_AddNumberToObject(snapVideo1, "SVCTLayer", 1);
    cJSON_AddNumberToObject(snapVideo1, "Width", 1920);
    cJSON_AddTrueToObject(snapFormat1, "VideoEnable");

}


static int Dahua_Cfg_GetEncode(int id, int sid, cJSON *root)
{
    cJSON *params, *table;  
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "table", table = cJSON_CreateObject());
    
    /* ExtraFormat */
    cJSON *extraFormat;
    cJSON_AddItemToObject(table, "ExtraFormat", extraFormat = cJSON_CreateArray());
    
    add_EncExtraFormat(extraFormat, 1, 1);
    add_EncExtraFormat(extraFormat, 1, 1);
    add_EncExtraFormat(extraFormat, 1, 1);
    
    /* MainFormat */
    cJSON *mainFormat;
    cJSON_AddItemToObject(table, "MainFormat", mainFormat = cJSON_CreateArray());
    add_EncMainFormat(mainFormat); // 3ÖÖÖ÷ÂëÁ÷·Ö±æÂÊ
    add_EncMainFormat(mainFormat);
    add_EncMainFormat(mainFormat);
    
    /* SnapFormat */
    cJSON *snapFormat;
    cJSON_AddItemToObject(table, "SnapFormat", snapFormat = cJSON_CreateArray());
    
    add_EncSnapFormat(snapFormat);  // 3ÖÖÖ÷ÂëÁ÷·Ö±æÂÊ
    add_EncSnapFormat(snapFormat);  // 3ÖÖÖ÷ÂëÁ÷·Ö±æÂÊ
    add_EncSnapFormat(snapFormat);  // 3ÖÖÖ÷ÂëÁ÷·Ö±æÂÊ
    
    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    return 0;
}

static int Dahua_Cfg_GetANY(int id, int sid, cJSON *root)
{
    cJSON *cjerror, *params;    
    cJSON_AddItemToObject(root, "error", cjerror = cJSON_CreateObject());
    cJSON_AddNumberToObject(cjerror, "code", 268959743);
    cJSON_AddStringToObject(cjerror, "message", "Unknown error! error code was not set in service!");
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddNullToObject(params, "table");
    cJSON_AddFalseToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    return 0;
}

//flag == 1, get default, not use in Dahua_Cfg_GetVideoInPreviewOptions
static int Dahua_Cfg_GetVideoInPreviewOptions(int id, int sid, cJSON *root, int flag)
{
    //
    cJSON *params, *table;   
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "table", table = cJSON_CreateArray());

    //table
    cJSON *item;
    cJSON_AddItemToArray(table, item = cJSON_CreateObject());
    cJSON_AddNumberToObject(item, "AESensitivity", 5);
    cJSON_AddNumberToObject(item, "ApertureStepLevel", 5);
    cJSON_AddNumberToObject(item, "BlackLevel", 15);
    cJSON_AddNumberToObject(item, "BlackLevelMode", 0);
    cJSON_AddFalseToObject(item, "BrightnessCompensation");
    cJSON_AddNumberToObject(item, "BrightnessCompensation1", 10);
    cJSON_AddNumberToObject(item, "BrightnessCompensation2", 10);

    cJSON *DayOptions;
    cJSON_AddItemToObject(item, "DayOptions", DayOptions = cJSON_CreateObject());
    cJSON_AddNumberToObject(DayOptions, "BeginHour", 0);
    cJSON_AddNumberToObject(DayOptions, "BeginMinute", 0);
    cJSON_AddNumberToObject(DayOptions, "BeginSecond", 0);
    cJSON_AddNumberToObject(DayOptions, "BrightnessThreshold", 50);
    cJSON_AddNumberToObject(DayOptions, "CCMMode", 1);
    cJSON_AddNumberToObject(DayOptions, "CFAMode", 1);
    cJSON_AddNumberToObject(DayOptions, "CGSMode", 1);
    cJSON_AddNumberToObject(DayOptions, "EEMode", 50);
    cJSON_AddNumberToObject(DayOptions, "EndHour", 23);
    cJSON_AddNumberToObject(DayOptions, "EndMinute", 59);
    cJSON_AddNumberToObject(DayOptions, "EndSecond", 59);
    cJSON_AddNumberToObject(DayOptions, "GammaAuto", 1);
    cJSON_AddNumberToObject(DayOptions, "GammaMode", 50);
    cJSON_AddNumberToObject(DayOptions, "ImageStabilization", 0);
    cJSON_AddNumberToObject(DayOptions, "NFMode", 1);
    cJSON *Snapshot;
    cJSON_AddItemToObject(DayOptions, "Snapshot", Snapshot = cJSON_CreateObject());
    cJSON_AddNumberToObject(Snapshot, "EEMode", 1);
    cJSON_AddNumberToObject(Snapshot, "GammaAuto", 1);
    cJSON_AddNumberToObject(Snapshot, "GammaMode", 50);
    cJSON_AddNumberToObject(Snapshot, "NFMode", 1);
    //cJSON_AddNumberToObject(DayOptions, "SwitchMode", 3);
    cJSON_AddNumberToObject(DayOptions, "TridimDenoise", 1);
    cJSON_AddNumberToObject(DayOptions, "TridimEstimation", 50);
    cJSON_AddNumberToObject(DayOptions, "TridimIntensity", 0);

    cJSON_AddNumberToObject(item, "GainStepLevel", 5);

    cJSON *NightOptions;
    cJSON_AddItemToObject(item, "NightOptions", NightOptions = cJSON_CreateObject());
    cJSON_AddNumberToObject(NightOptions, "BeginHour", 23);
    cJSON_AddNumberToObject(NightOptions, "BeginMinute", 59);
    cJSON_AddNumberToObject(NightOptions, "BeginSecond", 59);
    cJSON_AddNumberToObject(NightOptions, "BrightnessThreshold", 30);
    cJSON_AddNumberToObject(NightOptions, "CCMMode", 1);
    cJSON_AddNumberToObject(NightOptions, "CFAMode", 1);
    cJSON_AddNumberToObject(NightOptions, "CGSMode", 1);
    cJSON_AddNumberToObject(NightOptions, "EEMode", 50);
    cJSON_AddNumberToObject(NightOptions, "EndHour", 0);
    cJSON_AddNumberToObject(NightOptions, "EndMinute", 0);
    cJSON_AddNumberToObject(NightOptions, "EndSecond", 0);
    cJSON_AddNumberToObject(NightOptions, "GammaAuto", 1);
    cJSON_AddNumberToObject(NightOptions, "GammaMode", 50);
    cJSON_AddNumberToObject(NightOptions, "ImageStabilization", 0);
    cJSON_AddNumberToObject(NightOptions, "NFMode", 1);
    cJSON *Snapshot2;
    cJSON_AddItemToObject(NightOptions, "Snapshot", Snapshot2 = cJSON_CreateObject());
    cJSON_AddNumberToObject(Snapshot2, "EEMode", 1);
    cJSON_AddNumberToObject(Snapshot2, "GammaAuto", 1);
    cJSON_AddNumberToObject(Snapshot2, "GammaMode", 50);
    cJSON_AddNumberToObject(Snapshot2, "NFMode", 1);
    //cJSON_AddNumberToObject(NightOptions, "SwitchMode", 3);
    cJSON_AddNumberToObject(NightOptions, "TridimDenoise", 1);
    cJSON_AddNumberToObject(NightOptions, "TridimEstimation", 50);
    cJSON_AddNumberToObject(NightOptions, "TridimIntensity", 0);

    cJSON *NormalOptions;
    cJSON_AddItemToObject(item, "NormalOptions", NormalOptions = cJSON_CreateObject());
    cJSON_AddNumberToObject(NormalOptions, "BeginHour", 23);
    cJSON_AddNumberToObject(NormalOptions, "BeginMinute", 59);
    cJSON_AddNumberToObject(NormalOptions, "BeginSecond", 59);
    cJSON_AddNumberToObject(NormalOptions, "BrightnessThreshold", 30);
    cJSON_AddNumberToObject(NormalOptions, "CCMMode", 1);
    cJSON_AddNumberToObject(NormalOptions, "CFAMode", 1);
    cJSON_AddNumberToObject(NormalOptions, "CGSMode", 1);
    cJSON_AddNumberToObject(NormalOptions, "EEMode", 50);
    cJSON_AddNumberToObject(NormalOptions, "EndHour", 0);
    cJSON_AddNumberToObject(NormalOptions, "EndMinute", 0);
    cJSON_AddNumberToObject(NormalOptions, "EndSecond", 0);
    cJSON_AddNumberToObject(NormalOptions, "GammaAuto", 1);
    cJSON_AddNumberToObject(NormalOptions, "GammaMode", 50);
    cJSON_AddNumberToObject(NormalOptions, "ImageStabilization", 0);
    cJSON_AddNumberToObject(NormalOptions, "NFMode", 1);
    cJSON *Snapshot3;
    cJSON_AddItemToObject(NormalOptions, "Snapshot", Snapshot3 = cJSON_CreateObject());
    cJSON_AddNumberToObject(Snapshot3, "EEMode", 1);
    cJSON_AddNumberToObject(Snapshot3, "GammaAuto", 1);
    cJSON_AddNumberToObject(Snapshot3, "GammaMode", 50);
    cJSON_AddNumberToObject(Snapshot3, "NFMode", 1);
    //cJSON_AddNumberToObject(NormalOptions, "SwitchMode", 3);
    cJSON_AddNumberToObject(NormalOptions, "TridimDenoise", 1);
    cJSON_AddNumberToObject(NormalOptions, "TridimEstimation", 50);
    cJSON_AddNumberToObject(NormalOptions, "TridimIntensity", 0);

    cJSON *OtherOptions;
    cJSON_AddItemToObject(item, "OtherOptions", OtherOptions = cJSON_CreateObject());
    const int nBrightnessThreshold[2] = {30, 50};
    cJSON_AddItemToObject(OtherOptions, "BrightnessThreshold", cJSON_CreateIntArray(nBrightnessThreshold, 2));
    cJSON_AddNumberToObject(OtherOptions, "CCMMode", 1);
    cJSON_AddNumberToObject(OtherOptions, "CFAMode", 1);
    cJSON_AddNumberToObject(OtherOptions, "CGSMode", 1);

    cJSON *DuskTimeSection;
    cJSON_AddItemToObject(OtherOptions, "DuskTimeSection", DuskTimeSection = cJSON_CreateObject());
    cJSON_AddNumberToObject(DuskTimeSection, "BeginHour", 17);
    cJSON_AddNumberToObject(DuskTimeSection, "BeginMinute", 0);
    cJSON_AddNumberToObject(DuskTimeSection, "BeginSecond", 0);
    cJSON_AddNumberToObject(DuskTimeSection, "EndHour", 19);
    cJSON_AddNumberToObject(DuskTimeSection, "EndMinute", 0);
    cJSON_AddNumberToObject(DuskTimeSection, "EndSecond", 0);

    cJSON_AddNumberToObject(OtherOptions, "EEMode", 1);
    cJSON_AddNumberToObject(OtherOptions, "GammaAuto", 1);
    cJSON_AddNumberToObject(OtherOptions, "GammaMode", 50);

    cJSON *MorningTimeSection;
    cJSON_AddItemToObject(OtherOptions, "MorningTimeSection", MorningTimeSection = cJSON_CreateObject());
    cJSON_AddNumberToObject(MorningTimeSection, "BeginHour", 5);
    cJSON_AddNumberToObject(MorningTimeSection, "BeginMinute", 0);
    cJSON_AddNumberToObject(MorningTimeSection, "BeginSecond", 0);
    cJSON_AddNumberToObject(MorningTimeSection, "EndHour", 7);
    cJSON_AddNumberToObject(MorningTimeSection, "EndMinute", 0);
    cJSON_AddNumberToObject(MorningTimeSection, "EndSecond", 0);

    cJSON_AddNumberToObject(OtherOptions, "NFMode", 1);
    cJSON *Snapshot4;
    cJSON_AddItemToObject(OtherOptions, "Snapshot", Snapshot4 = cJSON_CreateObject());
    cJSON_AddNumberToObject(Snapshot4, "EEMode", 1);
    cJSON_AddNumberToObject(Snapshot4, "GammaAuto", 1);
    cJSON_AddNumberToObject(Snapshot4, "GammaMode", 50);
    cJSON_AddNumberToObject(Snapshot4, "NFMode", 1);
    cJSON_AddNumberToObject(OtherOptions, "TridimDenoise", 0);
    cJSON_AddNumberToObject(OtherOptions, "TridimEstimation", 0);
    cJSON_AddNumberToObject(OtherOptions, "TridimIntensity", 0);

    cJSON_AddNumberToObject(item, "ReferenceLevel", 50);
    cJSON_AddNumberToObject(item, "ShutterStepLevel", 5);
    //cJSON_AddNumberToObject(item, "SwitchMode", 3);
    cJSON_AddNumberToObject(item, "SwitchMode", 3);
    cJSON_AddNumberToObject(item, "WhiteLevel", 100);

    //
    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    return 0;
}

static int Dahua_Cfg_GetWeb(int id, int sid, cJSON *root)
{
    //
    cJSON *params, *table;   
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "table", table = cJSON_CreateObject());

    cJSON_AddTrueToObject(table, "Enable");
    cJSON_AddNumberToObject(table, "Port", 80);

    //
    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    return 0;
}


static void add_title(cJSON *title, int nEncodeBlend, int nPreviewBlend, int rect[])
{
    const int nBackColor[4] = {0, 0, 0, 128};
    cJSON_AddItemToObject(title, "BackColor", cJSON_CreateIntArray(nBackColor, 4));
    
    if (nEncodeBlend == 1)
        cJSON_AddTrueToObject(title, "EncodeBlend");
    else
        cJSON_AddFalseToObject(title, "EncodeBlend");
    
    const int nFrontColor[4] = {255, 255, 255, 0};
    cJSON_AddItemToObject(title, "FrontColor", cJSON_CreateIntArray(nFrontColor, 4));
    
    if (nPreviewBlend == 1)
        cJSON_AddTrueToObject(title, "PreviewBlend");
    else
        cJSON_AddFalseToObject(title, "PreviewBlend");
    
    cJSON_AddItemToObject(title, "Rect", cJSON_CreateIntArray(rect, 4));

}

static int Dahua_Cfg_GetVideoWidget(int id, int sid, cJSON *root)
{
    //
    cJSON *params, *table;   
    cJSON_AddNumberToObject(root, "id", id);
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    cJSON_AddItemToObject(params, "table", table = cJSON_CreateObject());

    //table
    cJSON *ChannelTitle;
    cJSON_AddItemToObject(table, "ChannelTitle", ChannelTitle = cJSON_CreateObject());
    int rect[4] = {148, 7511, 1773, 7928};
    add_title(ChannelTitle, 0, 1, rect);
    //todo
    
    
    //
    cJSON_AddTrueToObject(root, "result");
    cJSON_AddNumberToObject(root, "session", sid);

    return 0;
}


static int Dahua_Cfg_Get(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, cJSON *json)
{
    char name[100] = {0};
    cJSON *pj = cJSON_GetObjectItem(json, "params");
    if (pj) {
        cJSON *cjname = cJSON_GetObjectItem(pj, "name");
        sprintf(name, "%s", cjname->valuestring);
    }
    PRINT_INFO("name:%s\n", name);
    
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    if (!strcmp(name, "ANY")) {
        Dahua_Cfg_GetANY(id, sid, root);
    } else if (!strcmp(name, "Encode")) {
        Dahua_Cfg_GetEncode(id, sid, root);
    } else if (!strcmp(name, "VideoAnalyseRule")) {
        Dahua_Cfg_GetVideoAnalyseRule(id, sid, root);
    } else if (!strcmp(name, "ChannelTitle")) {
        Dahua_Cfg_GetChannelTitle(id, sid, root);
    } else if (!strcmp(name, "VideoColor")) {
        Dahua_Cfg_GetVideoColor(id, sid, root, 0);
    } else if (!strcmp(name, "VideoInSharpness")) {
        Dahua_Cfg_GetVideoInSharpness(id, sid, root);
    } else if (!strcmp(name, "VideoInDenoise")) {
        Dahua_Cfg_GetVideoInDenoise(id, sid, root);
    } else if (!strcmp(name, "VideoInOptions")) {
        Dahua_Cfg_GetVideoInOptions(id, sid, root, 0);
    } else if (!strcmp(name, "VideoInMode")) {
        Dahua_Cfg_GetVideoInMode(id, sid, root);
    } else if (!strcmp(name, "VideoImageControl")) {
        Dahua_Cfg_GetVideoImageControl(id, sid, root);
    } else if (!strcmp(name, "VideoInExposure")) {
        Dahua_Cfg_GetVideoInExposure(id, sid, root);
    } else if (!strcmp(name, "VideoInBacklight")) {
        Dahua_Cfg_GetVideoInBacklight(id, sid, root);
    } else if (!strcmp(name, "VideoInWhiteBalance")) {
        Dahua_Cfg_GetVideoInWhiteBalance(id, sid, root);
    } else if (!strcmp(name, "VideoInDayNight")) {
        Dahua_Cfg_GetVideoInDayNight(id, sid, root);
    } else if (!strcmp(name, "MotionDetect")) {
        Dahua_Cfg_GetMotionDetect(id, sid, root, 0);
    } else if (!strcmp(name, "VideoEncodeROI")) {
        Dahua_Cfg_GetVideoEncodeROI(id, sid, root);
    } else if (!strcmp(name, "ChannelTitle")) {
        Dahua_Cfg_GetChannelTitle(id, sid, root);
    } else if (!strcmp(name, "VideoInPreviewOptions")) {
        Dahua_Cfg_GetVideoInPreviewOptions(id, sid, root, 0);
    } else if (!strcmp(name, "Web")) {
        Dahua_Cfg_GetWeb(id, sid, root);
    } else {
        PRINT_ERR("cfg get, not support name:%s\n", name);
        cJSON_AddNumberToObject(root, "id", id);
        cJSON_AddFalseToObject(root, "result");
        cJSON_AddNumberToObject(root, "session", sid);
    }

    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}

static int Dahua_Cfg_GetDefault(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, cJSON *json)
{
    char name[100] = {0};
    cJSON *pj = cJSON_GetObjectItem(json, "params");
    if (pj) {
        cJSON *cjname = cJSON_GetObjectItem(pj, "name");
        sprintf(name, "%s", cjname->valuestring);
    }
    PRINT_INFO("name:%s\n", name);

    ////
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    if (!strcmp(name, "VideoAnalyseRule")) {
        cJSON *params, *table;   
        cJSON_AddNumberToObject(root, "id", id);
        cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
        cJSON_AddItemToObject(params, "table", table = cJSON_CreateArray());
        cJSON_AddTrueToObject(root, "result");
        cJSON_AddNumberToObject(root, "session", sid);
    } else if (!strcmp(name, "MotionDetect")) {
        Dahua_Cfg_GetMotionDetect(id, sid, root, 1);
    } else if (!strcmp(name, "VideoEncodeROI")) {
        Dahua_Cfg_GetVideoEncodeROI(id, sid, root);
    } else if (!strcmp(name, "VideoColor")) {
        Dahua_Cfg_GetVideoColor(id, sid, root, 1);
    } else if (!strcmp(name, "VideoInOptions")) {
        Dahua_Cfg_GetVideoInOptions(id, sid, root, 1);
    } else if (!strcmp(name, "VideoInPreviewOptions")) {
        Dahua_Cfg_GetVideoInPreviewOptions(id, sid, root, 1);
    } else if (!strcmp(name, "MotionDetect")) {
        Dahua_Cfg_GetMotionDetect(id, sid, root, 1);
    } else {
        PRINT_ERR("cfg get default, not support name:%s\n", name);
        cJSON_AddNumberToObject(root, "id", id);
        cJSON_AddFalseToObject(root, "result");
        cJSON_AddNumberToObject(root, "session", sid);
    }

    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;

}


static int Dahua_Restore_VideoColor()
{
    static int first_restore_v_color = 1;
    if (first_restore_v_color == 1) {
        first_restore_v_color = 0;
        PRINT_INFO("first restore video color\n");
        return -1;
    }
    PRINT_INFO("restore video color\n");
    
    GK_NET_IMAGE_CFG image_cfg;
    memcpy(&image_cfg, &runImageCfg, sizeof(GK_NET_IMAGE_CFG));

    image_cfg.contrast = tmp_image_cfg.contrast;
    image_cfg.brightness = tmp_image_cfg.brightness;
    image_cfg.saturation = tmp_image_cfg.saturation;
    //image_cfg.sharpness = tmp_image_cfg.sharpness;
    image_cfg.hue = tmp_image_cfg.hue;

    goke_api_set_ioctrl(RESTORE_IMAGE_CFG, &image_cfg);
    tmp_image_cfg.contrast = runImageCfg.contrast;
    tmp_image_cfg.brightness = runImageCfg.brightness;
    tmp_image_cfg.saturation = runImageCfg.saturation;
    //tmp_image_cfg.sharpness = runImageCfg.sharpness;
    tmp_image_cfg.hue = runImageCfg.hue;

    return 0;
}

static void Dahua_Restore_VideoInSharpness()
{
    GK_NET_IMAGE_CFG image_cfg;
    memcpy(&image_cfg, &runImageCfg, sizeof(GK_NET_IMAGE_CFG));

    image_cfg.sharpness = tmp_image_cfg.sharpness;

    goke_api_set_ioctrl(RESTORE_IMAGE_CFG, &image_cfg);

    tmp_image_cfg.sharpness = runImageCfg.sharpness;
}

static void Dahua_Restore_VideoImageControl()
{
    GK_NET_IMAGE_CFG image_cfg;
    memcpy(&image_cfg, &runImageCfg, sizeof(GK_NET_IMAGE_CFG));

    image_cfg.flipEnabled = tmp_image_cfg.flipEnabled;
    image_cfg.mirrorEnabled = tmp_image_cfg.mirrorEnabled;

    goke_api_set_ioctrl(RESTORE_IMAGE_CFG, &image_cfg);

    tmp_image_cfg.flipEnabled = runImageCfg.flipEnabled;
    tmp_image_cfg.mirrorEnabled = runImageCfg.mirrorEnabled;
}

static void Dahua_Restore_VideoInDayNight()
{
    GK_NET_IMAGE_CFG image_cfg;
    memcpy(&image_cfg, &runImageCfg, sizeof(GK_NET_IMAGE_CFG));

    image_cfg.irCutMode = tmp_image_cfg.irCutMode;

    goke_api_set_ioctrl(RESTORE_IMAGE_CFG, &image_cfg);

    tmp_image_cfg.irCutMode = runImageCfg.irCutMode;
}

static int Dahua_Cfg_RestoreTemp(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, cJSON *json)
{
    char name[100] = {0};
    cJSON *pj = cJSON_GetObjectItem(json, "params");
    if (pj) {
        cJSON *cjname = cJSON_GetObjectItem(pj, "name");
        sprintf(name, "%s", cjname->valuestring);
    }
    PRINT_INFO("name:%s\n", name);
    
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿
    int ret = 0;
    
    if (!strcmp(name, "VideoColor")) {
        ret = Dahua_Restore_VideoColor();
    } else if (!strcmp(name, "VideoInSharpness")) {
        Dahua_Restore_VideoInSharpness();
    } else if (!strcmp(name, "VideoInDenoise")) {
        PRINT_ERR("RestoreTemp, do nothing, name:%s\n", name);
    } else if (!strcmp(name, "VideoInPreviewOptions")) {
        PRINT_ERR("RestoreTemp, do nothing, name:%s\n", name);
    } else if (!strcmp(name, "VideoImageControl")) {
        Dahua_Restore_VideoImageControl();
    } else if (!strcmp(name, "VideoInDayNight")) {
        Dahua_Restore_VideoInDayNight();
    } else {
        PRINT_ERR("RestoreTemp, not support name:%s\n", name);
    }

    if (ret == 0) {
        cJSON *params;
        cJSON_AddNumberToObject(root, "id", id);
        cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
        cJSON_AddNullToObject(params, "options");
        cJSON_AddTrueToObject(root, "result");
        cJSON_AddNumberToObject(root, "session", sid);
    } else {
        cJSON *cjerror, *params;	
        cJSON_AddItemToObject(root, "error", cjerror = cJSON_CreateObject());
        cJSON_AddNumberToObject(cjerror, "code", 268959743);
        cJSON_AddStringToObject(cjerror, "message", "Unknown error! error code was not set in service!");
        cJSON_AddNumberToObject(root, "id", id);
        cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
        const char *str_err[1] = {"WriteFileError"}; 
        cJSON_AddItemToObject(params, "options", cJSON_CreateStringArray(str_err, 1));
        cJSON_AddFalseToObject(root, "result");
        cJSON_AddNumberToObject(root, "session", sid);
    }

    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}


static int DahuaGetRect(cJSON *item, int a[])
{
    cJSON *EncodeBlend = cJSON_GetObjectItem(item, "EncodeBlend");
    if (!EncodeBlend) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    }

    if (EncodeBlend->type != cJSON_True) {
        return -1;
    }

    cJSON *Rect = cJSON_GetObjectItem(item, "Rect");
    if (!Rect) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    }

    int arraySize = cJSON_GetArraySize(Rect);

    int index;
    cJSON *arrayItem = NULL;
    for(index = 0; index < arraySize; index++) {
        arrayItem = cJSON_GetArrayItem(Rect, index);
        if(!arrayItem){
            PRINT_ERR("cJSON_GetArrayItem error\n");
            return -1;
        }
        a[index] = arrayItem->valueint;
    }
        
    return 0;
}

static int Dahua_Cfg_SetVideoWidget(cJSON *params)
{
    cJSON *table = cJSON_GetObjectItem(params, "table");
    if (!table) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    }

    int ret = 0;
    /* ChannelTitle */
    cJSON *ChannelTitle = cJSON_GetObjectItem(table, "ChannelTitle");
    if (!ChannelTitle) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    } else {
        int x[4] = {0, 0, 0, 0};
        ret = DahuaGetRect(ChannelTitle, x);
        if (ret == 0) {
            PRINT_INFO("ChannelTitle, %d %d %d %d\n", x[0], x[1], x[2], x[3]);
            //todo set channel osd
            runChannelCfg.channelInfo[0].osdChannelName.enable = 1;
            runChannelCfg.channelInfo[0].osdChannelName.x = (float) x[0] / 8192;
            runChannelCfg.channelInfo[0].osdChannelName.y = (float) x[1] / 8192;

            runChannelCfg.channelInfo[1].osdChannelName.enable = 1;
            runChannelCfg.channelInfo[1].osdChannelName.x = (float) x[0] / 8192;
            runChannelCfg.channelInfo[1].osdChannelName.y = (float) x[1] / 8192;
            
            netcam_osd_update_title();
        }
    }

 
    /* Covers */
    cJSON *Covers = cJSON_GetObjectItem(table, "Covers");
    if (!Covers) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    } else {
        int index;
        int arraySize = cJSON_GetArraySize(Covers);
        cJSON *arrayItem = NULL;
        for(index = 0; index < arraySize; index++) {
            arrayItem = cJSON_GetArrayItem(Covers, index);
            if(!arrayItem){
                PRINT_ERR("cJSON_GetArrayItem error\n");
                return -1;
            }
            
            int x[4] = {0, 0, 0, 0};
            ret = DahuaGetRect(arrayItem, x);
            if (ret == 0) {
                PRINT_INFO("Cover[%d], %d %d %d %d\n", index, x[0], x[1], x[2], x[3]);
                //todo set cover

            }
        }
    }

    /* TimeTitle */
    cJSON *TimeTitle = cJSON_GetObjectItem(table, "TimeTitle");
    if (!TimeTitle) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    } else {
        int x[4] = {0, 0, 0, 0};
        ret = DahuaGetRect(TimeTitle, x);
        if (ret == 0) {
            PRINT_INFO("TimeTitle, %d %d %d %d\n", x[0], x[1], x[2], x[3]);
            //todo set channel osd
            runChannelCfg.channelInfo[0].osdDatetime.enable = 1;
            runChannelCfg.channelInfo[0].osdDatetime.x = (float) x[0] / 8192;
            runChannelCfg.channelInfo[0].osdDatetime.y = (float) x[1] / 8192;

            runChannelCfg.channelInfo[1].osdDatetime.enable = 1;
            runChannelCfg.channelInfo[1].osdDatetime.x = (float) x[0] / 8192;
            runChannelCfg.channelInfo[1].osdDatetime.y = (float) x[1] / 8192;

            netcam_osd_update_clock();
        }
    }
    
    return 0;
}

static int DahuaModifyEncode(cJSON *format, GK_NET_VIDEO_CFG *video_cfg, int stream_id)
{
    GK_ENC_STREAM_H264_ATTR *pstream = &(video_cfg->vencStream[stream_id].h264Conf);

    cJSON *format0 = cJSON_GetArrayItem(format, 0);
    
    cJSON *AudioEnable = cJSON_GetObjectItem(format0, "AudioEnable");
    if (!AudioEnable) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    }
    if (AudioEnable->type == cJSON_True) {
        //ÒôÆµÃ»ÓÐ¸Ä¶¯. do nothing
    }

    cJSON *VideoEnable = cJSON_GetObjectItem(format0, "VideoEnable");
    if (!VideoEnable) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    }
    if (VideoEnable->type == cJSON_True) {
        cJSON *Video = cJSON_GetObjectItem(format0, "Video");
        if (!Video) {
            PRINT_ERR("cjson get item error!\n");
            return -1;
        }

        cJSON *BitRate = cJSON_GetObjectItem(Video, "BitRate");
        if (BitRate) {
            pstream->bps = BitRate->valueint;
        }

        cJSON *BitRateControl = cJSON_GetObjectItem(Video, "BitRateControl");
        if (BitRateControl) {
            if (strcmp(BitRateControl->valuestring, "VBR") == 0) {
                pstream->rc_mode = 1;
            } else {
                pstream->rc_mode = 0;
            }
        }

        cJSON *FPS = cJSON_GetObjectItem(Video, "FPS");
        if (FPS) {
            pstream->fps = FPS->valueint;
        }
        pstream->gop = pstream->fps * 2;

        cJSON *Width = cJSON_GetObjectItem(Video, "Width");
        if (Width) {
            pstream->width= Width->valueint;
        }
        
        cJSON *Height = cJSON_GetObjectItem(Video, "Height");
        if (Height) {
            pstream->height = Height->valueint;
        }

        cJSON *Quality = cJSON_GetObjectItem(Video, "Quality");
        if (Quality) {
            pstream->quality= Quality->valueint - 1;
        }
        PRINT_INFO("set fps:%d, gop:%d, bps:%d, remode:%s, %d * %d, quality:%d\n",
            pstream->fps, pstream->gop, pstream->bps,
            pstream->rc_mode == 1?"VBR":"CBR", pstream->width, 
            pstream->height, pstream->quality);
    }

    return 0;
}


static int Dahua_Cfg_SetEncode(cJSON *params)
{
    cJSON *table = cJSON_GetObjectItem(params, "table");
    if (!table) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    }

    GK_NET_VIDEO_CFG video_cfg;
    memset(&video_cfg, 0, sizeof(GK_NET_VIDEO_CFG));
    goke_api_get_ioctrl(GET_VIDEO_CFG, &video_cfg);

    /* ExtraFormat */
    cJSON *ExtraFormat = cJSON_GetObjectItem(table, "ExtraFormat");
    if (!ExtraFormat) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    } else {
        DahuaModifyEncode(ExtraFormat, &video_cfg, 1);
    }

    /* MainFormat */
    cJSON *MainFormat = cJSON_GetObjectItem(table, "MainFormat");
    if (!MainFormat) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    } else {
        DahuaModifyEncode(MainFormat, &video_cfg, 0);
    }

    goke_api_set_ioctrl(SET_VIDEO_CFG, &video_cfg);
    
    return 0;
}

static int Dahua_Cfg_SetChannelTitle(cJSON *params)
{
    cJSON *table = cJSON_GetObjectItem(params, "table");
    if (!table) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    }

    cJSON *cjname = cJSON_GetObjectItem(table, "name");
    if (!cjname) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    }
    //char name[128] = {0};
    //sprintf(name, "%s", cjname->valuestring);
    //PRINT_INFO("set name:%s\n", name);

    runChannelCfg.channelInfo[0].osdChannelName.enable = 1;
    sprintf(runChannelCfg.channelInfo[0].osdChannelName.text, "%s", cjname->valuestring);
    runChannelCfg.channelInfo[1].osdChannelName.enable = 1;
    sprintf(runChannelCfg.channelInfo[1].osdChannelName.text, "%s", cjname->valuestring);

    PRINT_INFO("set name:%s\n", runChannelCfg.channelInfo[0].osdChannelName.text);
    netcam_osd_update_title();
    
    return 0;
}

static int DahuaSetVideoColor(cJSON *tmp, int index, int is_save)
{
    int is_change = 0;
    char sTimeSection[30] = {0};
    cJSON *TimeSection = cJSON_GetObjectItem(tmp, "TimeSection");
    if (!TimeSection) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    } else {
        sprintf(sTimeSection, "%s", TimeSection->valuestring);

        if (sTimeSection[0] == '1') {
            is_change = 1;
        } else {
            PRINT_INFO("not enable,index:%d.\n", index);
            is_change = 0;
        }
    }

    int nBrightness = 0;
    int nChromaSuppress = 0;
    int nContrast = 0;
    int nGamma = 0;
    int nHue = 0;
    int nSaturation = 0;

    cJSON *Brightness = cJSON_GetObjectItem(tmp, "Brightness");
    if (Brightness) {
        nBrightness = Brightness->valueint;
    }

    cJSON *ChromaSuppress = cJSON_GetObjectItem(tmp, "ChromaSuppress");
    if (ChromaSuppress) {
        nChromaSuppress = ChromaSuppress->valueint;
    }

    cJSON *Contrast = cJSON_GetObjectItem(tmp, "Contrast");
    if (Contrast) {
        nContrast = Contrast->valueint;
    }

    cJSON *Gamma = cJSON_GetObjectItem(tmp, "Gamma");
    if (Gamma) {
        nGamma = Gamma->valueint;
    }

    cJSON *Hue = cJSON_GetObjectItem(tmp, "Hue");
    if (Hue) {
        nHue = Hue->valueint;
    }

    cJSON *Saturation = cJSON_GetObjectItem(tmp, "Saturation");
    if (Saturation) {
        nSaturation = Saturation->valueint;
    }



    DahuaColorProfile *pColor = &stDahuaColorProfile[index];
    if (is_save == 1) {
        pColor->enable = is_change;
        pColor->brightness = nBrightness;
        pColor->chromaSuppress = nChromaSuppress;
        pColor->contrast= nContrast;
        pColor->gamma = nGamma;
        pColor->hue = nHue;
        pColor->saturation = nSaturation;
    }

    if (is_change == 1) {
        profile_index = index;
        GK_NET_IMAGE_CFG image;
        memset(&image, 0, sizeof(GK_NET_IMAGE_CFG));
        goke_api_get_ioctrl(GET_IMAGE_CFG, &image);
        image.brightness = nBrightness;
        image.contrast = nContrast;
        image.hue = nHue;
        image.saturation= nSaturation;
        if (pColor->day_night_color == 0) //²ÊÉ«
            image.irCutMode = 1;
        else if (pColor->day_night_color == 2) //ºÚ°×
            image.irCutMode = 2;
        else
            image.irCutMode = 0;
        image.flipEnabled = pColor->flip;
        image.mirrorEnabled = pColor->mirror;

        stDahuaColorProfile[0].flip = pColor->flip;
        stDahuaColorProfile[1].flip = pColor->flip;
        stDahuaColorProfile[2].flip = pColor->flip;
        stDahuaColorProfile[0].mirror = pColor->mirror;
        stDahuaColorProfile[1].mirror = pColor->mirror;
        stDahuaColorProfile[2].mirror = pColor->mirror;

        tmp_image_cfg.brightness = nBrightness;
        tmp_image_cfg.contrast = nContrast;
        tmp_image_cfg.hue = nHue;
        tmp_image_cfg.saturation= nSaturation;
        tmp_image_cfg.irCutMode= image.irCutMode;
        tmp_image_cfg.flipEnabled = image.flipEnabled;
        tmp_image_cfg.mirrorEnabled = image.mirrorEnabled;

   

        PRINT_INFO("index:%d, day_night_color:%d-%d-%d, flip:%d-%d-%d, mirror:%d-%d-%d\n", 
            index, stDahuaColorProfile[0].day_night_color, 
            stDahuaColorProfile[1].day_night_color, stDahuaColorProfile[2].day_night_color, 
            stDahuaColorProfile[0].flip, stDahuaColorProfile[1].flip, stDahuaColorProfile[2].flip,
            stDahuaColorProfile[0].mirror, stDahuaColorProfile[1].mirror,stDahuaColorProfile[2].mirror);

        if (is_save == 1)
            goke_api_set_ioctrl(SET_SAVE_IMAGE_CFG, &image);
        else
            goke_api_set_ioctrl(SET_IMAGE_CFG, &image);
    }

    return 0;
}


static int Dahua_Cfg_SetVideoColor(cJSON *params, int is_save)
{
    cJSON *table = cJSON_GetObjectItem(params, "table");
    if (!table) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    }

    cJSON *table0 = cJSON_GetArrayItem(table, 0);
    if (!table0) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    }

    cJSON *table00 = cJSON_GetArrayItem(table0, 0);
    if (!table00) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    } else {
        DahuaSetVideoColor(table00, 0, is_save);
    }

    cJSON *table01 = cJSON_GetArrayItem(table0, 1);
    if (!table01) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    } else {
        DahuaSetVideoColor(table01, 1, is_save);
    }

    cJSON *table02 = cJSON_GetArrayItem(table0, 2);
    if (!table02) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    } else {
        DahuaSetVideoColor(table02, 2, is_save);
    }

    PRINT_INFO("profile_index=%d\n", profile_index);
    return 0;
}

static int DahuaSetVideoInSharpness(cJSON *tmp, int index, int is_save)
{
    int nSharpness = 0;

    cJSON *Sharpness = cJSON_GetObjectItem(tmp, "Sharpness");
    if (Sharpness) {
        nSharpness = Sharpness->valueint;
    }

    PRINT_INFO("nSharpness:%d\n", nSharpness);

    DahuaColorProfile *pColor = &stDahuaColorProfile[index];
    if (is_save == 1) {
        pColor->sharpness = nSharpness;
    }

    if (index == 0) {
        GK_NET_IMAGE_CFG image;
        memset(&image, 0, sizeof(GK_NET_IMAGE_CFG));
        goke_api_get_ioctrl(GET_IMAGE_CFG, &image);
        image.sharpness = nSharpness;
        tmp_image_cfg.sharpness = nSharpness;
        
        if (is_save == 1)
            goke_api_set_ioctrl(SET_SAVE_IMAGE_CFG, &image);
        else
            goke_api_set_ioctrl(SET_IMAGE_CFG, &image);
    }

    return 0;
}


static int Dahua_Cfg_SetVideoInSharpness(cJSON *params, int is_save)
{
    cJSON *table = cJSON_GetObjectItem(params, "table");
    if (!table) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    }

    cJSON *table0 = cJSON_GetArrayItem(table, 0);
    if (!table0) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    }

    cJSON *table00 = cJSON_GetArrayItem(table0, 0);
    if (!table00) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    } else {
        DahuaSetVideoInSharpness(table00, 0, is_save);
    }

    return 0;
}


static int DahuaSetVideoInDayNight(cJSON *tmp, int index, int is_save)
{
    cJSON *Mode = cJSON_GetObjectItem(tmp, "Mode");
    if (!Mode) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    }
    
    GK_NET_IMAGE_CFG image;
    memset(&image, 0, sizeof(GK_NET_IMAGE_CFG));
    goke_api_get_ioctrl(GET_IMAGE_CFG, &image);

    if (!strcmp(Mode->valuestring, "Brightness")) {
        image.irCutMode = 0;
        tmp_image_cfg.irCutMode = 0;
    } else if (!strcmp(Mode->valuestring, "BlackWhite")) {
        image.irCutMode = 2;
        tmp_image_cfg.irCutMode = 2;
    } else if (!strcmp(Mode->valuestring, "Color")) {
        image.irCutMode = 1;
        tmp_image_cfg.irCutMode = 1;
    }

    if (is_save == 1)
        goke_api_set_ioctrl(SET_SAVE_IMAGE_CFG, &image);
    else
        goke_api_set_ioctrl(SET_IMAGE_CFG, &image);

    return 0;
}


static int Dahua_Cfg_SetVideoInDayNight(cJSON *params, int is_save)
{
    cJSON *table = cJSON_GetObjectItem(params, "table");
    if (!table) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    }

    cJSON *table0 = cJSON_GetArrayItem(table, 0);
    if (!table0) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    }

    cJSON *table00 = cJSON_GetArrayItem(table0, 0);
    if (!table00) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    } else {
        DahuaSetVideoInDayNight(table00, 0, is_save);
    }

    #if 0
    cJSON *table01 = cJSON_GetArrayItem(table0, 1);
    if (!table01) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    } else {
        DahuaSetVideoInDayNight(table01, 1);
    }

    cJSON *table02 = cJSON_GetArrayItem(table0, 2);
    if (!table02) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    } else {
        DahuaSetVideoInDayNight(table02, 2);
    }
    #endif
    return 0;
}

static int Dahua_Cfg_SetMotionDetect(cJSON *params, int is_save)
{
    cJSON *table = cJSON_GetObjectItem(params, "table");
    if (!table) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    }

    cJSON *Enable = cJSON_GetObjectItem(table, "Enable");
    if (Enable) {
        if (Enable->type == cJSON_True)
            runMdCfg.enable = 1;
        else
            runMdCfg.enable = 0;
    }

    cJSON *EventHandler = cJSON_GetObjectItem(table, "EventHandler");
    if (!EventHandler) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    }

    cJSON *AlarmOutEnable = cJSON_GetObjectItem(EventHandler, "AlarmOutEnable");
    if (AlarmOutEnable) {
        if (AlarmOutEnable->type == cJSON_True)
            runMdCfg.handle.is_alarmout = 1;
        else
            runMdCfg.handle.is_alarmout = 0;
    }

    cJSON *BeepEnable = cJSON_GetObjectItem(EventHandler, "BeepEnable");
    if (BeepEnable) {
        if (BeepEnable->type == cJSON_True)
            runMdCfg.handle.is_beep = 1;
        else
            runMdCfg.handle.is_beep = 0;
    }

    cJSON *MailEnable = cJSON_GetObjectItem(EventHandler, "MailEnable");
    if (MailEnable) {
        if (MailEnable->type == cJSON_True)
            runMdCfg.handle.is_email = 1;
        else
            runMdCfg.handle.is_email = 0;
    }

    #if 0
    cJSON *MessageEnable = cJSON_GetObjectItem(EventHandler, "MessageEnable");
    if (MessageEnable) {
        if (MessageEnable->type == cJSON_True)
            runMdCfg.handle.is_pushMsg = 1;
        else
            runMdCfg.handle.is_pushMsg = 0;
    }
    #endif

    cJSON *RecordEnable = cJSON_GetObjectItem(EventHandler, "RecordEnable");
    if (RecordEnable) {
        if (RecordEnable->type == cJSON_True)
            runMdCfg.handle.is_rec = 1;
        else
            runMdCfg.handle.is_rec = 0;
    }

    cJSON *SnapshotEnable = cJSON_GetObjectItem(EventHandler, "SnapshotEnable");
    if (SnapshotEnable) {
        if (SnapshotEnable->type == cJSON_True)
            runMdCfg.handle.is_snap = 1;
        else
            runMdCfg.handle.is_snap = 0;
    }
    MdCfgSave();
    
    PRINT_INFO("set md, enable:%d, is_alarmout:%d, is_beep:%d, is_email:%d, is_rec:%d, is_snap:%d\n",
        runMdCfg.enable, runMdCfg.handle.is_alarmout, runMdCfg.handle.is_beep,
        runMdCfg.handle.is_email, runMdCfg.handle.is_rec, runMdCfg.handle.is_snap);

    return 0;
}


static void get_from_options(cJSON *table0, int i, int nColor[], int nFlip[], int nMirror[], int nSwitchMode[])
{
    cJSON *DayNightColor0 = cJSON_GetObjectItem(table0, "DayNightColor");
    if (DayNightColor0) {
        nColor[i] = DayNightColor0->valueint;
    }

    cJSON *Flip0 = cJSON_GetObjectItem(table0, "Flip");
    if (Flip0) {
        if (Flip0->type == cJSON_True)
            nFlip[i] = 1;
        else
            nFlip[i] = 0;
    }

    cJSON *Mirror0 = cJSON_GetObjectItem(table0, "Mirror");
    if (Mirror0) {
        if (Mirror0->type == cJSON_True)
            nMirror[i] = 1;
        else
            nMirror[i] = 0;
    }

    cJSON *SwitchMode = cJSON_GetObjectItem(table0, "SwitchMode");
    if (SwitchMode) {
        nSwitchMode[i] = SwitchMode->valueint;
    }

#if 0
    int nRotate90 = 0;
    cJSON *Rotate90 = cJSON_GetObjectItem(table0, "Rotate90");
    if (Rotate90) {
        nRotate90 = Rotate90->valueint;
    }

    char sWhiteBalance[30] = {0};
    cJSON *WhiteBalance = cJSON_GetObjectItem(table0, "WhiteBalance");
    if (WhiteBalance) {
        sprintf(sWhiteBalance, "%s", WhiteBalance->valuestring);
    }

    PRINT_INFO("nDayNightColor:%d, nFlip:%d, nMirror:%d, nRotate90:%d, sWhiteBalance:%s\n", 
        nDayNightColor, nFlip, nMirror, nRotate90, sWhiteBalance);
#endif

}

static int Dahua_Cfg_SetVideoInOptions(cJSON *params, int is_save)
{
    cJSON *table = cJSON_GetObjectItem(params, "table");
    if (!table) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    }

    cJSON *table0 = cJSON_GetArrayItem(table, 0);
    if (!table0) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    }

    int day_night_color[3] = {0, 0, 0};
    int nFlip[3] = {0, 0, 0};
    int nMirror[3] = {0, 0, 0};
    int nSwitchMode[3] = {0, 0, 0};
    get_from_options(table0, 0, day_night_color, nFlip, nMirror, nSwitchMode);


    /* NightOptions */
    cJSON *NightOptions = cJSON_GetObjectItem(table0, "NightOptions");
    if (NightOptions) {
        get_from_options(NightOptions, 1, day_night_color, nFlip, nMirror, nSwitchMode);
    }

    /* NormalOptions */
    cJSON *NormalOptions = cJSON_GetObjectItem(table0, "NormalOptions");
    if (NormalOptions) {
        get_from_options(NormalOptions, 2, day_night_color, nFlip, nMirror, nSwitchMode);
    }

    if (is_save == 1) {
        stDahuaColorProfile[0].day_night_color = day_night_color[0];
        stDahuaColorProfile[1].day_night_color = day_night_color[1];
        stDahuaColorProfile[2].day_night_color = day_night_color[2];
        stDahuaColorProfile[0].flip = nFlip[0];
        stDahuaColorProfile[1].flip = nFlip[1];
        stDahuaColorProfile[2].flip = nFlip[2];
        stDahuaColorProfile[0].mirror = nMirror[0];
        stDahuaColorProfile[1].mirror = nMirror[1];
        stDahuaColorProfile[2].mirror = nMirror[2];
        stDahuaColorProfile[0].nSwitchMode = nSwitchMode[0];
        stDahuaColorProfile[1].nSwitchMode = nSwitchMode[1];
        stDahuaColorProfile[2].nSwitchMode = nSwitchMode[2];

    }

    PRINT_INFO("set options, index:%d, day_night_color:%d-%d-%d, flip:%d-%d-%d, mirror:%d-%d-%d, switchMode:%d-%d-%d\n", 
        profile_index, stDahuaColorProfile[0].day_night_color,
        stDahuaColorProfile[1].day_night_color, stDahuaColorProfile[2].day_night_color,
        stDahuaColorProfile[0].flip, stDahuaColorProfile[1].flip, stDahuaColorProfile[2].flip,
        stDahuaColorProfile[0].mirror, stDahuaColorProfile[1].mirror, stDahuaColorProfile[2].mirror,
        stDahuaColorProfile[0].nSwitchMode, stDahuaColorProfile[1].nSwitchMode, stDahuaColorProfile[2].nSwitchMode);
    
    GK_NET_IMAGE_CFG image;
    memset(&image, 0, sizeof(GK_NET_IMAGE_CFG));
    goke_api_get_ioctrl(GET_IMAGE_CFG, &image);

    
    image.flipEnabled = nFlip[profile_index];
    image.mirrorEnabled = nMirror[profile_index];
    if (day_night_color[profile_index] == 0) {
        image.irCutMode = 1; //²ÊÉ« °×Ìì
    } else if (day_night_color[profile_index] == 2) {
        image.irCutMode = 2; //ºÚ°× ÍíÉÏ
    } else {
        image.irCutMode = 0; //×Ô¶¯
    }
    
    tmp_image_cfg.flipEnabled = image.flipEnabled;
    tmp_image_cfg.mirrorEnabled = image.mirrorEnabled;
    tmp_image_cfg.irCutMode = image.irCutMode;

    if (is_save == 1)
        goke_api_set_ioctrl(SET_SAVE_IMAGE_CFG, &image);
    else
        goke_api_set_ioctrl(SET_IMAGE_CFG, &image);

    return 0;
}

static int Dahua_Cfg_SetVideoInPreviewOptions(cJSON *params, int is_save)
{
    cJSON *table = cJSON_GetObjectItem(params, "table");
    if (!table) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    }

    //nothing todo

    return 0;
}


static int Dahua_Cfg_SetVideoImageControl(cJSON *params, int is_save)
{
    cJSON *table = cJSON_GetObjectItem(params, "table");
    if (!table) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    }

    cJSON *table0 = cJSON_GetArrayItem(table, 0);
    if (!table0) {
        PRINT_ERR("cjson get item error!\n");
        return -1;
    }

    int nFlip = 0;
    int nMirror = 0;
    int nRotate90 = 0;
    cJSON *Flip = cJSON_GetObjectItem(table0, "Flip");
    if (Flip) {
        if (Flip->type == cJSON_True)
            nFlip = 1;
        else
            nFlip = 0;
    }

    cJSON *Mirror = cJSON_GetObjectItem(table0, "Mirror");
    if (Mirror) {
        if (Mirror->type == cJSON_True)
            nMirror = 1;
        else
            nMirror = 0;
    }

    cJSON *Rotate90 = cJSON_GetObjectItem(table0, "Rotate90");
    if (Rotate90) {
        nRotate90 = Rotate90->valueint;
    }

    char sWhiteBalance[30] = {0};
    cJSON *WhiteBalance = cJSON_GetObjectItem(table0, "WhiteBalance");
    if (WhiteBalance) {
        sprintf(sWhiteBalance, "%s", WhiteBalance->valuestring);
    }

    PRINT_INFO("nFlip:%d, nMirror:%d, nRotate90:%d, sWhiteBalance:%s\n", 
        nFlip, nMirror, nRotate90, sWhiteBalance);

    GK_NET_IMAGE_CFG image;
    memset(&image, 0, sizeof(GK_NET_IMAGE_CFG));
    goke_api_get_ioctrl(GET_IMAGE_CFG, &image);
    image.flipEnabled = nFlip;
    image.mirrorEnabled = nMirror;
    tmp_image_cfg.flipEnabled = nFlip;
    tmp_image_cfg.mirrorEnabled = nMirror;

    if (is_save == 1)
        goke_api_set_ioctrl(SET_SAVE_IMAGE_CFG, &image);
    else
        goke_api_set_ioctrl(SET_IMAGE_CFG, &image);

    return 0;
}


static int Dahua_Cfg_Set(DAHUA_SESSION_CTRL *pCtrl, int id, int sid, cJSON *json, int is_save)
{
    int ret = 0;
    char name[100] = {0};
    cJSON *pj = cJSON_GetObjectItem(json, "params");
    if (pj) {
        cJSON *cjname = cJSON_GetObjectItem(pj, "name");
        sprintf(name, "%s", cjname->valuestring);
    }
    PRINT_INFO("name:%s\n", name);

    if (!strcmp(name, "VideoWidget")) {
        Dahua_Cfg_SetVideoWidget(pj);
    } else if (!strcmp(name, "Encode")) {
        Dahua_Cfg_SetEncode(pj);
    } else if (!strcmp(name, "ChannelTitle")) {
        Dahua_Cfg_SetChannelTitle(pj);
    } else if (!strcmp(name, "VideoInOptions")) {
        Dahua_Cfg_SetVideoInOptions(pj, is_save);
    } else if (!strcmp(name, "VideoInPreviewOptions")) {
        Dahua_Cfg_SetVideoInPreviewOptions(pj, is_save);
    } else if (!strcmp(name, "VideoImageControl")) {
        Dahua_Cfg_SetVideoImageControl(pj, is_save);
    } else if (!strcmp(name, "VideoColor")) {
        Dahua_Cfg_SetVideoColor(pj, is_save);
    } else if (!strcmp(name, "VideoInSharpness")) {
        Dahua_Cfg_SetVideoInSharpness(pj, is_save);
    } else if (!strcmp(name, "VideoInDayNight")) {
        Dahua_Cfg_SetVideoInDayNight(pj, is_save);
    } else if (!strcmp(name, "MotionDetect")) {
        Dahua_Cfg_SetMotionDetect(pj, is_save);
    } else if (!strcmp(name, "VideoInDenoise")) {
        PRINT_ERR("do nothing, name:%s\n", name);
    } else {
        PRINT_ERR("not support name:%s\n", name);
        ret = -1;
    }

    /* »Ø¸´ÏûÏ¢ */
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    /* Ìí¼ÓÏûÏ¢ÄÚÈÝ */
    if (ret <0) {
        cJSON_AddNumberToObject(root, "id", id);
        cJSON_AddFalseToObject(root, "result");
        cJSON_AddNumberToObject(root, "session", sid);
    } else {
        cJSON *params;    
        cJSON_AddNumberToObject(root, "id", id);
        cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
        cJSON_AddNullToObject(params, "options");
        cJSON_AddTrueToObject(root, "result");
        cJSON_AddNumberToObject(root, "session", sid);
    }

    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    memset(pCtrl->fSendbuf, 0, sizeof(pCtrl->fSendbuf));
    sprintf(pCtrl->fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    pCtrl->nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);	

    return 0;
}


/*************************************************************
 * º¯Êý½éÉÜ£º»Ø·ÅÏÂÔØ
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_InfoNew_Cmd(char* bufPost, DAHUA_SESSION_CTRL *pCtrl)
{
    int ret = 0;
    DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)bufPost;
    PRINT_INFO("l1-exlen:%d, l4-exlen:%d, id:%d(0x%08x), n_session:%d(0x%08x)\n",
        t_msg_head->dvrip.dvrip_extlen, t_msg_head->l[4], 
        t_msg_head->l[2], t_msg_head->l[2], 
        t_msg_head->l[6], t_msg_head->l[6]);

    //PRINT_INFO("exlen:%d, exdata:%s\n", t_msg_head->dvrip.dvrip_extlen, (char *)(bufPost + DVRIP_HEAD_T_SIZE));
    PRINT_INFO("exlen:%d", t_msg_head->dvrip.dvrip_extlen);

    if (t_msg_head->dvrip.dvrip_extlen > 0) {
        cJSON *json, *pj;
        char *out = NULL;
        int id = 0;
        int sid = 0;
        char method[100] = {0};
        //char params[100] = {0};
        int result = 0;
        
        json = cJSON_Parse(bufPost + DVRIP_HEAD_T_SIZE);
        if (!json){
            PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
            return -1;
        }
        
        out = cJSON_Print(json);
        PRINT_INFO("recv:%s\n",out);
  
        pj = cJSON_GetObjectItem(json, "id");
        if(pj)
            id = pj->valueint;

        pj = cJSON_GetObjectItem(json, "session");
        if(pj)
            sid = pj->valueint;
      
        pj = cJSON_GetObjectItem(json, "method");
        if(pj)
            sprintf(method, "%s", pj->valuestring);

        //pj = cJSON_GetObjectItem(json, "params");
        //if(pj)
        //    sprintf(params, "%s", pj->valuestring);

        #if 0
        if ((id != t_msg_head->l[2]) || (sid != t_msg_head->l[6])) {
            //´íÎó´¦Àí
            result = 1;
        }
        #endif

        pCtrl->nSendLen = sizeof(DVRIP_HEAD_T);

        PRINT_INFO("method:%s\n", method);

        if (!strcmp(method, "alarm.listMethod")) {
            ret = Dahua_Alarm_ListMethod(pCtrl, id, sid, result);
        } else if (!strcmp(method, "alarm.getAllInSlots")) {
            ret = Dahua_Alarm_GetAllInSlots(pCtrl, id, sid, result);
        } else if (!strcmp(method, "system.listMethod")) { // todo
            ret = Dahua_System_ListMethod(pCtrl, id, sid, result);
        } else if (!strcmp(method, "devVideoInput.listMethod")) {
            ret = Dahua_DevVideoInput_ListMethod(pCtrl, id, sid, result);
        } else if (!strcmp(method, "devVideoInput.factory.instance")) {
            ret = Dahua_Instance(pCtrl, id, sid, 37392784);
        } else if (!strcmp(method, "devVideoInput.getCaps")) {
            ret = Dahua_DevVideoInput_GetCaps(pCtrl, id, sid, result);
        } else if (!strcmp(method, "devVideoInput.getCapsEx")) {
            ret = Dahua_DevVideoInput_GetCapsEx(pCtrl, id, sid, result);
        } else if (!strcmp(method, "devVideoDetect.listMethod")) {
            ret = Dahua_DevVideoDetect_ListMethod(pCtrl, id, sid, result);
        } else if (!strcmp(method, "devVideoDetect.factory.instance")) {
            ret = Dahua_Instance(pCtrl, id, sid, 42070872);
        } else if (!strcmp(method, "devVideoDetect.getCaps")) {
            ret = Dahua_DevVideoDetect_GetCaps(pCtrl, id, sid, result);
        }  else if (!strcmp(method, "ptz.listMethod")) {
            ret = Dahua_Ptz_ListMethod(pCtrl, id, sid, result);
        } else if (!strcmp(method, "ptz.factory.instance")) {
            ret = Dahua_Instance(pCtrl, id, sid, 815831920);
        } else if (!strcmp(method, "devVideoEncode.listMethod")) {
            ret = Dahua_DevVideoEncode_ListMethod(pCtrl, id, sid, result);
        } else if (!strcmp(method, "devVideoEncode.factory.instance")) {
            ret = Dahua_Instance(pCtrl, id, sid, 62142552);
        } else if (!strcmp(method, "devVideoEncode.getCaps")) {
            ret = Dahua_DevVideoEncode_GetCaps(pCtrl, id, sid, result);
        } else if (!strcmp(method, "devAudioEncode.listMethod")) {
            ret = Dahua_DevAudioEncode_ListMethod(pCtrl, id, sid, result);
        } else if (!strcmp(method, "devAudioEncode.factory.instance")) {
            ret = Dahua_Instance(pCtrl, id, sid, 1925390160);
        } else if (!strcmp(method, "devAudioEncode.getFormatCaps")) {
            ret = Dahua_DevAudioEncode_GetFormatCaps(pCtrl, id, sid, result);
        } else if (!strcmp(method, "devVideoAnalyse.listMethod")) {
            ret = Dahua_DevVideoAnalyse_ListMethod(pCtrl, id, sid, result);
        } else if (!strcmp(method, "devVideoAnalyse.factory.instance")) {
            ret = Dahua_Instance(pCtrl, id, sid, 58235848);
        } else if (!strcmp(method, "devVideoAnalyse.getCaps")) {
            ret = Dahua_DevVideoAnalyse_GetCaps(pCtrl, id, sid, result);
        } else if (!strcmp(method, "magicBox.listMethod")) {
            ret = Dahua_MagicBox_ListMethod(pCtrl, id, sid, result);
        } else if (!strcmp(method, "magicBox.factory.instance")) {
            ret = Dahua_Instance(pCtrl, id, sid, 62064864);
        } else if (!strcmp(method, "magicBox.getProductDefinition")) {
            ret = Dahua_MagicBox_GetProductDefinition(pCtrl, id, sid, json);
        } else if (!strcmp(method, "magicBox.getSoftwareVersion")) {
            ret = Dahua_MagicBox_GetSoftwareVersion(pCtrl, id, sid, json);
        } else if (!strcmp(method, "magicBox.getSerialNo")) {
            ret = Dahua_MagicBox_GetSerialNo(pCtrl, id, sid, json);
        } else if (!strcmp(method, "magicBox.getDeviceClass")) {
            ret = Dahua_MagicBox_GetDeviceClass(pCtrl, id, sid, json);
        } else if (!strcmp(method, "configManager.listMethod")) {
            ret = Dahua_ConfigManager_ListMethod(pCtrl, id, sid, result);
        } else if (!strcmp(method, "configManager.getMemberNames")) {
            ret = Dahua_Cfg_GetMemberNames(pCtrl, id, sid, result);
        } else if (!strcmp(method, "configManager.factory.instance")) {
            ret = Dahua_Instance(pCtrl, id, sid, 35907912);
        } else if (!strcmp(method, "configManager.getConfig")) {
            ret = Dahua_Cfg_Get(pCtrl, id, sid, json);
        } else if (!strcmp(method, "configManager.getDefault")) {
            ret = Dahua_Cfg_GetDefault(pCtrl, id, sid, json);
        } else if (!strcmp(method, "configManager.setConfig")) {
            ret = Dahua_Cfg_Set(pCtrl, id, sid, json, 1);
        } else if (!strcmp(method, "configManager.restoreTemporaryConfig")) {
            ret = Dahua_Cfg_RestoreTemp(pCtrl, id, sid, json);
        } else if (!strcmp(method, "configManager.setTemporaryConfig")) {
            ret = Dahua_Cfg_Set(pCtrl, id, sid, json, 0);
        } else if (!strcmp(method, "encode.listMethod")) {
            ret = Dahua_Encode_ListMethod(pCtrl, id, sid, result);
        } else if (!strcmp(method, "encode.factory.instance")) {
            ret = Dahua_Instance(pCtrl, id, sid, 62142552);
        } else if (!strcmp(method, "encode.getConfigCaps")) {
            ret = Dahua_Encode_GetConfigCaps(pCtrl, id, sid, result);
        } else {
            PRINT_ERR("not support method:%s\n", method);
        }

        //del && free
        cJSON_Delete(json);
        free(out);

        //¼Ó°üÍ·
        DahuaMakeMsg(pCtrl, t_msg_head->l[2], t_msg_head->l[6], ACK_UNKOWN);

        //·¢ËÍ
        ret = ZhiNuo_SockSend(pCtrl, pCtrl->fSendbuf, pCtrl->nSendLen, 100);
        if (ret < 0) {
            PRINT_ERR();
            return -1;  
        }
    }

    return ret;
}

#if 1

/*************************************************************
 * º¯Êý½éÉÜ£º´Ósocket½ÓÊÕÊý¾Ý
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
static int DahuaRecvMsg(int sock, char *szbuf, int len, int timeout)
{
    int ret = 0;
    fd_set read_set, excpt_set;
    struct timeval tv;
    int n_timeout_num = 0;

    if (timeout == 0)
        timeout = 100;
    memset(szbuf, 0, len);
    do
    {
        if(2 == n_timeout_num) //Èç¹û³¬Ê±2´ÎÔò¹Ø±Õ´Ë´Î»á»°
        {
            PRINT_ERR("session 200s timeout close connect\n");
            return -1;
        }
        FD_ZERO(&read_set);
        FD_ZERO(&excpt_set);

        FD_SET(sock, &read_set);
        FD_SET(sock, &excpt_set);

        tv.tv_sec = timeout;
        tv.tv_usec = 0;
        if ((ret = select(sock + 1, &read_set, NULL, &excpt_set, &tv)) < 0) 
        {
            PRINT_ERR("recv error:%s\n", strerror(errno));
            return -1; 
        }
        if(0 == ret)
        {
            //PRINT_ERR("recv istimeout\n");
            n_timeout_num++;
            continue;
        }
        if(FD_ISSET(sock, &excpt_set))
        {
            PRINT_ERR("select recv error:%s\n", strerror(errno));
            return -1;
        }

        if(FD_ISSET(sock, &read_set))
        {
            //PRINT_INFO("recv have data\n");
            n_timeout_num = 0;
            break;
        }
    }
    while(1);

    len = recv(sock, szbuf, len, 0);
    if (len <= 0)
    {
        PRINT_ERR("recv error:%s\n", strerror(errno));
    }

#if 0
    //¿ÉÒÔÁÙÊ±´òÓ¡Ã¿¸ö±¨ÎÄµÄÃ¿¸ö×Ö½ÚÄÚÈÝ
    int i = 0;
    char *extra_tmp = szbuf;
    for(i = 0; i < len; i++)
    {
        printf("bufPost:%d:%d\n", i, *extra_tmp);
        extra_tmp = extra_tmp + 1;
    }
#endif

    return len;
}

static int DahuaRecvMsg2(int sock, char *szbuf, int len)
{
	memset(szbuf, 0, len);
	int recvlen = recv(sock, szbuf, len, 0);
	if (recvlen < 0) //³ö´í
	{
    	PRINT_ERR("sock %d recv error:%s\n", sock, strerror(errno));
		return -1;
    }
	if(recvlen == 0) //¶ÔÃæ¹Ø±Õsocket
	{
		PRINT_INFO("tcp recv 0 len, to close sock: %d\n", sock);
		//close(sock);
		return -1;
	}
	//if(len == 0)
	//	PRINT_ERR("recv 0 len??? sock = %d\n", sock);
	
	return recvlen;
}




/*************************************************************
 * º¯Êý½éÉÜ£º´¦ÀíÃüÁîµÄÖ÷Ìå
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
static int DahuaHandleMsg(DAHUA_SESSION_CTRL *pCtrl, char* bufPost, int datalen)
{
    int ret = 0;
    int n_offset = 0;
    char tem_buf[DVRIP_HEAD_T_SIZE] = {0};

    //ÓÐ¿ÉÄÜnvr¼¸ÌõÃüÁîÁ¬×ÅÒ»Æð·¢¹ýÀ´£¬ËùÒÔÒª²ð·ÖÒ»¸öÃüÁîÒ»¸öÃüÁî´¦Àí
    while(1)
    {
        memcpy(tem_buf, bufPost + n_offset, DVRIP_HEAD_T_SIZE);
        DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)tem_buf; 

        //ÁÙÊ±´òÓ¡±¨ÎÄÍ·µÄÃ¿¸ö×Ö½ÚÄÚÈÝ
        #if 0
        int i = 0;
        char *extra_tmp = (char *)t_msg_head;
        PRINT_INFO("datalen:%d\n", datalen);
        for(i = 0; i < DVRIP_HEAD_T_SIZE; i++)
        {
            //printf("bufPost:%d:%d\n", i, *extra_tmp);
            printf("%x ", (char)*extra_tmp);
            extra_tmp = extra_tmp + 1;
        } 
        printf("\n");
        #endif

        printf("\n**************************************************\n");

        //PRINT_ERR("sock:%d, rcv cmd:%x, datalen:%d, address:%p, have_deal_len:%d, extralen:%d\n", sock,
        //            t_msg_head->dvrip.cmd, datalen, bufPost + n_offset, n_deal_len, t_msg_head->dvrip.dvrip_extlen);    
        PRINT_INFO("sock:%d, sid:%d, ip:%s, t_msg_head->dvrip.cmd @@@>>>>>>>:%x\n", pCtrl->fd, pCtrl->fSessionInt, pCtrl->c_ip, t_msg_head->dvrip.cmd);
        switch(t_msg_head->dvrip.cmd)
        {
            case CMD_DAHUA_LOG_ON:
                PRINT_INFO("dahua logon, sock:%d\n", pCtrl->fd);
                ret = Dahua_LogOn(bufPost + n_offset, pCtrl); //µÇÂ¼
                break;
            case CMD_ZHINUO_LOG_ON:
                PRINT_INFO("zhinuo log\n");
                ret = Dahua_LogOn(bufPost + n_offset, pCtrl); //µÇÂ¼
                break;                
            case CMD_STATUS_SEARCH:
                PRINT_INFO("STATUS_SEARCH \n");
                ret = ZhiNuo_Search_Status(bufPost + n_offset, pCtrl); //²éÑ¯¹¤×÷¼°±¨¾¯×´Ì¬ 1
                break;     
            case CMD_DEV_EVENT_SEARCH:
                PRINT_INFO("EVENT_SEARCH \n");
                ret = ZhiNuo_Dev_Event(bufPost + n_offset, pCtrl); //²éÑ¯Éè±¸ÊÂ¼þ 1
                break;  
            case CMD_MEDIA_REQUEST:
                PRINT_INFO("MEDIA_REQUEST \n");
                ret = ZhiNuo_Media_Request(bufPost + n_offset, pCtrl); //Ã½ÌåÊý¾ÝÇëÇó
                break;  
            case CMD_MEDIA_CAPACITY_SEARCH:
                PRINT_INFO("MEDIA_CAPACITY_SEARCH \n");
                ret = ZhiNuo_Search_Media_Capacity(bufPost + n_offset, pCtrl); //²éÑ¯Éè±¸Ã½ÌåÄÜÁ¦ÐÅÏ¢ ok
                break;                 
            case CMD_CONFIG_SEARCH:
                PRINT_INFO("CONFIG_SEARCH \n");
                ret = ZhiNuo_Search_Config(bufPost + n_offset, pCtrl); //²éÑ¯ÅäÖÃ²ÎÊý
                break;     
            case CMD_CHANNEL_NAME_SEARCH:
                PRINT_INFO("CHANNEL_NAME_SEARCH \n");
                ret = ZhiNuo_Search_Channel_Name(bufPost + n_offset, pCtrl); //²éÑ¯Í¨µÀÃû³Æ
                break;   
            case CMD_SYSTEM_INFO_SEARCH:
                PRINT_INFO("SYSTEM_INFO_SEARCH \n");
                ret = ZhiNuo_Search_System_Info(bufPost + n_offset, pCtrl); //²éÑ¯ÏµÍ³ÐÅÏ¢
                break;  
            case CMD_CONNECT_REQUEST:
                PRINT_INFO("CONNECT_REQUEST \n");
                ret = Dahua_ConnectRequest(bufPost + n_offset, pCtrl); //ÇëÇó½¨Á¢Á¬½Ó¹ØÏµ
                //if (pCtrl->fSessionType == CONNECT_SESSION)
                //    return 1; //²»¹Ø±Õsock²¢¹Ø±ÕÕâ¸ö»Ø»°Ïß³Ì
                break;
            case CMD_I_FRAME_REQUEST:
                PRINT_INFO("I_FRAME_REQUEST \n");
                ret = ZhiNuo_I_Frame_Request(bufPost + n_offset, pCtrl); //Ç¿ÖÆIÖ¡
                break;  
            case CMD_DEV_CONTROL:
                PRINT_INFO("DEV_CONTROL\n");
                ret = ZhiNuo_DEV_Control(bufPost + n_offset, pCtrl); //Éè±¸¿ØÖÆ
                break;   
            case CMD_SET_CONFIG:
                PRINT_INFO("SET_CONFIG datalen:%d\n", datalen);
                ret = ZhiNuo_Set_Config(bufPost + n_offset, pCtrl); //ÐÞ¸ÄÅäÖÃ²ÎÊý
                break;    
            case CMD_TIME_MANAGE:
                PRINT_INFO("TIME_MANAGE\n");
                ret = ZhiNuo_Time_Manage(bufPost + n_offset, pCtrl); //Ê±¼ä¹ÜÀí
                break;        
            case CMD_SET_CHANNEL_NAME:
                PRINT_INFO("SET_CHANNEL_NAME\n");
                ret = ZhiNuo_Set_Channel_Name(bufPost + n_offset, pCtrl); //ÐÞ¸ÄÍ¨µÀÃû³Æ
                break;                     
            case CMD_PTZ_CONTROL:
                PRINT_INFO("PTZ_CONTROL\n");
                ret = ZhiNuo_PTZ_Control(bufPost + n_offset, pCtrl); //ÔÆÌ¨¿ØÖÆ
                break;     
            case CMD_TALK_REQUEST:
                PRINT_INFO("TALK_REQUEST \n");
                //ret = ZhiNuo_Talk_Request(bufPost + n_offset, pCtrl); //ÇëÇó¶Ô½²
                break;   
            case CMD_TALK:
                PRINT_INFO("TALK \n");
                //ret = ZhiNuo_Talk(bufPost + n_offset, pCtrl); //¿ªÊ¼¶Ô½²
                break;    
            case CMD_TALK_CONTROL:
                PRINT_INFO("TALK_CONTROL\n");
                //ret = ZhiNuo_Talk_Control(bufPost + n_offset, pCtrl); //¶Ô½²¿ØÖÆ
                break;    
            case CMD_RECORD_SEARCH:
                PRINT_INFO("RECORD_SEARCH\n");
                //ret = ZhiNuo_Record_Search(bufPost + n_offset, pCtrl); //Â¼ÏñËÑË÷
                break;    
            case CMD_RECORD_PLAYBACK:
                PRINT_INFO("RECORD_PLAYBACK\n");
                //ret = ZhiNuo_Record_Playback(bufPost + n_offset, pCtrl); //Â¼Ïñ»Ø·Å
                break;                  
            case CMD_RECORD_DOWNLOAD:
                PRINT_INFO("RECORD_DOWNLOAD\n");
                //ret = ZhiNuo_Record_Download(bufPost + n_offset, pCtrl); //Â¼ÏñÏÂÔØ
                break;    
            case CMD_INFO_NEW:
                PRINT_INFO("CMD_INFO_NEW [0x%02x]\n", t_msg_head->dvrip.cmd);
                ret = ZhiNuo_InfoNew_Cmd(bufPost + n_offset, pCtrl); //²»ÖªÃûµÄÃüÁî
                break;
            case CMD_CONTROL_CONNECT:
                PRINT_INFO("CMD_CONTROL_CONNECT [0x%02x]\n", t_msg_head->dvrip.cmd);
                ret = Dahua_ControlConnect(bufPost + n_offset, pCtrl); //²»ÖªÃûµÄÃüÁî
                //if (pCtrl->fSessionType == CONNECT_SESSION)
                //    return 1; //²»¹Ø±Õsock²¢¹Ø±ÕÕâ¸ö»Ø»°Ïß³Ì

                break; 

            default:                
                PRINT_ERR("cmd not support 0x%x datalen:%d\n", t_msg_head->dvrip.cmd, datalen); 
                break;
        }        

        if (ret < 0)
        {
            PRINT_ERR("deal cmd:%0x err.", t_msg_head->dvrip.cmd);
            break;
        }

        n_offset += (DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen);
        if (n_offset >= datalen)
        {
            //±¨ÎÄ´¦ÀíÍê±ÏÍË³ö
            break;
        }
        else
        {
            PRINT_ERR("deal second cmd\n");
        }
    }

    return 0;
}


/*************************************************************
 * º¯Êý½éÉÜ£º»á»°Ïß³Ì
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int DahuaSessionProcess(DAHUA_SESSION_CTRL *pCtrl)
{
    int ret = 0;
    int n_rcv_len = 0;
    char rcv_buff[BUFLEN] = {0};  


    //½¨Á¢Á¬½ÓºóÒ»Ö±Ñ­»·½ÓÊÕÃüÁî
    while(1)
    {
        if(0 == g_dahua_thread_run)
        {
            break;
        }

        //½ÓÊÕÃüÁî
        //PRINT_INFO("tid:%d, recvmsg -- sock:%d, sid:%d, ip:%s\n", syscall(SYS_gettid), pCtrl->fd, pCtrl->fSessionInt, pCtrl->c_ip);
        memset(rcv_buff, 0, sizeof(rcv_buff));
        n_rcv_len = DahuaRecvMsg(pCtrl->fd, rcv_buff, sizeof(rcv_buff), 100);
        //n_rcv_len = DahuaRecvMsg2(pCtrl->fd, rcv_buff, sizeof(rcv_buff));
        if (n_rcv_len <= 0) 
        {
            PRINT_ERR("DahuaRecvMsg err fd:%d, n_rcv_len:%d\n", pCtrl->fd, n_rcv_len);
            break;
            //continue; // ²»ÍË³ö
        }

        //´¦ÀíÃüÁî
        //PRINT_INFO("tid:%d, handlemsg -- sock:%d, sid:%d, ip:%s\n", syscall(SYS_gettid), pCtrl->fd, pCtrl->fSessionInt, pCtrl->c_ip);
        ret = DahuaHandleMsg(pCtrl, rcv_buff, n_rcv_len);
        if(ret < 0)
        {
            PRINT_ERR("DahuaHandleMsg err, ret=%d\n", ret);
            break;
        }

        //Èç¹ûÊÇÁ¬½ÓÇëÇóÔòÕâ¸öÏß³Ì´¦ÀíÍê¾Í¿ÉÒÔ¹Ø±ÕÁË
        if(pCtrl->fSessionType == CONNECT_SESSION)
        {
            PRINT_INFO("return cmd loop\n");
            //Ö±½Ó·µ»Ø£¬²»¹Ø±Õ socket
            return 0;
        }
 
    }  

    ret = UtCloseCmdSock(pCtrl->fd, &g_dahua_session);
    if ((ret < 0) && (pCtrl->fd > 0)) {
        PRINT_INFO("close socket:%d\n", pCtrl->fd);
        close(pCtrl->fd);        
    }

    #if !MULTI_CONNECT
    send_main = 0;
    send_sub = 0;
    main_sock = -1;
    sub_sock = -1;
    #endif

    return 0;
}

void *DahuaSessionThread(void *param)
{
    DahuaSetThreadName("DahuaSessionThread");

    DahuaClientInfo *pClientInfo = (DahuaClientInfo *)param;

	DAHUA_SESSION_CTRL stSessionCtrl;
	memset(&stSessionCtrl, 0, sizeof(DAHUA_SESSION_CTRL));
	stSessionCtrl.fd = pClientInfo->sock;
    strncpy(stSessionCtrl.c_ip, pClientInfo->ip, sizeof(stSessionCtrl.c_ip));
    stSessionCtrl.fSessionInt = 0;
    stSessionCtrl.fSessionType = UNINIT_SESSION;

    
	DahuaSessionProcess(&stSessionCtrl);

	PRINT_INFO("DahuaSessionThread end\n");
	return NULL;
}



int DahuaCreateTcpServerSock(int port, int listen_num)
{
    int fListenfd = socket(AF_INET, SOCK_STREAM, 0);
    if (fListenfd < 0) 
    {
        PRINT_ERR("socket error%s\n", strerror(errno));
        return -1;
    }

    /*  ÅäÖÃ¼àÌý»á»°µÄsocket */
    struct sockaddr_in t_serv_addr;
    bzero(&t_serv_addr, sizeof(t_serv_addr));
    t_serv_addr.sin_family = AF_INET;
    t_serv_addr.sin_port = htons(port);
    t_serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /* °ó¶¨¼àÌý»á»°µÄsocket */
    if (bind(fListenfd, (struct sockaddr *)&t_serv_addr, sizeof(struct sockaddr)) < 0) 
    {
        PRINT_ERR("bind err%s\n", strerror(errno));
        close(fListenfd);
        return -1;
    }

    /* Ö÷¶¯socket×ªÎª±»¶¯¼àÌýsocket */
    if (listen(fListenfd, listen_num) < 0) 
    {
        PRINT_ERR("listen error%s\n", strerror(errno));
        close(fListenfd);
        return -1;
    }

    return fListenfd;
}


/*************************************************************
 * º¯Êý½éÉÜ£ºµÇÂ¼nvr£¬È»ºó½éÉÜ´Ónvr·¢ËÍ¹ýÀ´µÄÃüÁî
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
void * DahuaMainThread(void *param)
{
    DahuaSetThreadName("DahuaMainThread");

    //´´½¨¼àÌý»á»°µÄsocket
    int fListenfd = DahuaCreateTcpServerSock(DAHUA_TCP_LISTEN_PORT, DAHUA_MAX_LINK_NUM);
    if (fListenfd < 0)
    {
        PRINT_ERR("DahuaCreateTcpServerSock err.");
        return NULL;
    }

    int sock_fd;
    struct sockaddr_in t_clnt_addr;
    int n_len = sizeof(t_clnt_addr);
    /* Ñ­»·¼àÌý£¬µÈ´ýnvrÖ÷¶¯Á¬½Ó */
    n_len = sizeof(t_clnt_addr);
    while (1) 
    {
        PRINT_INFO("begin to accept\n");
        sock_fd = accept(fListenfd, (struct sockaddr *)&t_clnt_addr, (socklen_t *)&n_len);/* ½ÓÊÕÁ¬½Ó */
        if(0 == g_dahua_thread_run)
        {
            break;
        }
        if (sock_fd < 0) 
        {
            PRINT_ERR("Server: accept failed%s\n", strerror(errno));
            continue;
        }

        PRINT_ERR("accept sockfd :%d, remote ip:%s, remote port:%d\n", 
                    sock_fd, inet_ntoa(t_clnt_addr.sin_addr), ntohs(t_clnt_addr.sin_port));

        DahuaClientInfo c_info;
        c_info.sock = sock_fd;
        sprintf(c_info.ip, "%s", inet_ntoa(t_clnt_addr.sin_addr));

        //´´½¨»á»°Ïß³Ì£¬Ã¿ÓÐÒ»¸önvrÁ¬½Ó¾Í´´½¨Ò»¸ö»á»°Ïß³Ì
        CreateDetachThread2(DahuaSessionThread, (void *)&c_info, sizeof(DahuaClientInfo), NULL);
    }
    PRINT_INFO("DahuaMainThread end !\n");

    if(fListenfd > 0)
    {
        close(fListenfd); 
        fListenfd = -1;
    }
    UtCloseAllSock(&g_dahua_session);

    return NULL;
}

#endif

#if 1
static int DahuaCreateBroadcastSock(int port)
{
    int socket_fd = -1;

    if ((socket_fd= socket(AF_INET, SOCK_DGRAM, 0)) == -1)   
    {     
        PRINT_ERR("socket create error\n");   
        return -1;  
    }     
    //ÉèÖÃ¸ÃÌ×½Ó×ÖÎª¹ã²¥ÀàÐÍ£¬  
    const int opt = 1; 
    int ret = 0;  
    ret = setsockopt(socket_fd, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt));  
    if(ret == -1)  
    {  
        PRINT_ERR("set socket_fd error...\n");  
        close(socket_fd);
        return -1;  
    } 
    struct ifreq ifr;
    strcpy(ifr.ifr_name,IFNAME);
    if(ioctl(socket_fd,SIOCGIFBRDADDR,&ifr) == -1)
    {
        perror("ioctl error");
        close(socket_fd);
        return -1;
    }
    // °ó¶¨µØÖ·  
    struct sockaddr_in addrto;  
    bzero(&addrto, sizeof(struct sockaddr_in));  
    addrto.sin_family = AF_INET;  
    addrto.sin_addr.s_addr = htonl(INADDR_ANY);  
    addrto.sin_port = htons(port);  
    if(bind(socket_fd,(struct sockaddr *)&(addrto), sizeof(struct sockaddr_in)) == -1)   
    {     
        PRINT_ERR("bind error...\n");  
        close(socket_fd);
        return -1;  
    }  

    return socket_fd;
}


static int DahuaBoardCastSendTo(int fSockSearchfd, int port, const char* inData, const int inLength)
{
	int ret = 0;
	int sendlen = 0;
    struct sockaddr_in addr;

    memset(&addr, 0,sizeof(&addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("255.255.255.255");
    addr.sin_port = htons(port);

	//ÒòÎªUDP ·¢ËÍÊ±£¬Ã»ÓÐ»º³åÇø£¬¹ÊÐèÒª¶Ô·¢ËÍºóµÄ·µ»ØÖµ½øÐÐÅÐ¶Ïºó£¬¶à´Î·¢ËÍ
	while(sendlen < inLength)
	{
	    ret = sendto(fSockSearchfd, inData + sendlen, inLength - sendlen, 0, (struct sockaddr*)&addr,  sizeof(addr));
		if(ret < 0)
		{
			//perror("Send error");
			//·Ç×èÈû²ÅÓÐ EAGAIN
			if (errno != EINTR && errno != EAGAIN )
			{
				PRINT_ERR("Send() socket %d error :%s\n", fSockSearchfd, strerror(errno));
				return -1;
			}
			else
				continue;
		}

		sendlen += ret;
	}

	//PRINT_INFO("ip search SendLen: %d\n", sendlen);
	
	return sendlen;
}

static int DahuaAckSearch(int sock_send)
{
    //PRINT_INFO("DahuaAckSearch\n");
    int n_offset = 0;

    //int i = 0, ii = 0;
    //int width= 0;
    //int height= 0;
    //int n_max_resolution = 0;
    int n_extra_len = 0;
    char a_extra_sting[255] = {0};   

    //»ñÈ¡±¾»úÍøÂç²ÎÊý£¬
    ST_SDK_NETWORK_ATTR st_ipc_net_info;
    if(0 != goke_api_get_ioctrl(GET_NETWORK_CFG, &st_ipc_net_info))
    {
        PRINT_ERR("GET_NETWORK_CFG fail\n");
        return 0;
    }    

    //char *dev_mode = "IPC";
    //char *dev_mode = "IPC-HDW1025C";
    char *dev_mode = "TP5";

    //////////////////////DVRIP_HEAD_T_SIZE
    char send_msg[512] = {0};
    DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)send_msg;                
    memset(send_msg, 0, sizeof(send_msg));
    t_msg_head->dvrip.cmd = ACK_DEV_SEARCH;  
    t_msg_head->c[2] = 17 + strlen(dev_mode);   //macµØÖ·³¤¶È¼ÓÉÏÉè±¸ÀàÐÍ
    t_msg_head->c[3] = 1;
    t_msg_head->dvrip.dvrip_extlen = 0x58;  //Îªextra_dataÊý×éµÄ³¤¶È  88 = 0x58        
    t_msg_head->c[16] = 2; //¹Ì¶¨Îª2
    t_msg_head->c[19] = 0; //·µ»ØÂë 0:Õý³£ 1:ÎÞÈ¨ÏÞ 2:ÎÞ¶ÔÓ¦ÅäÖÃÌá¹©
    t_msg_head->c[20] = 0xa6; //¸ù¾Ý×¥°ü±¨ÎÄÀïÃæµÄÖµ×öµÄÌî³ä
    //t_msg_head->c[20] = 0xb7; //¸ù¾Ý×¥°ü±¨ÎÄÀïÃæµÄÖµ×öµÄÌî³ä
    t_msg_head->c[22] = 0x96;
    t_msg_head->c[23] = 0x1; 
    t_msg_head->c[24] = 0x1; 
    n_offset += DVRIP_HEAD_T_SIZE;

    //////////////////////extra_data
    char extra_data[88] = {0}; // 88 = 0x58
    ZhiNuo_Dev_Search * str_dev_search = (ZhiNuo_Dev_Search *)extra_data;

    //str_dev_search->Version[0] = 0X32;  //°æ±¾ÐÅÏ¢ 
    //str_dev_search->Version[1] = 0X33; 
    //str_dev_search->Version[2] = 0X30; 
    //str_dev_search->Version[3] = 0X32;  

    str_dev_search->Version[0] = 0X02;  //°æ±¾ÐÅÏ¢ 
    str_dev_search->Version[1] = 0X00; 
    str_dev_search->Version[2] = 0X6d; 
    str_dev_search->Version[3] = 0X02; 
    str_dev_search->Version[4] = 0X00;
    str_dev_search->Version[5] = 0X00; 
    str_dev_search->Version[6] = 0X08; 
    str_dev_search->Version[7] = 0X00; 

    strcpy(str_dev_search->HostName, "IPC"); //Ö÷»úÃû

    struct sockaddr_in adr_inet; 
    //ÅäÖÃ
    struct in_addr net_addr;
    inet_aton(st_ipc_net_info.ip, &adr_inet.sin_addr);
    str_dev_search->HostIP = (unsigned long)adr_inet.sin_addr.s_addr;
    memcpy(&net_addr, &str_dev_search->HostIP, sizeof(net_addr));
//    PRINT_INFO("ip:%s\n", (char *)inet_ntoa(net_addr));
    inet_aton(st_ipc_net_info.mask, &adr_inet.sin_addr);
    str_dev_search->Submask = (unsigned long)adr_inet.sin_addr.s_addr;
    inet_aton(st_ipc_net_info.gateway, &adr_inet.sin_addr);
    str_dev_search->GateWayIP = (unsigned long)adr_inet.sin_addr.s_addr;  
    inet_aton(st_ipc_net_info.dns1, &adr_inet.sin_addr);
    str_dev_search->DNSIP = (unsigned long)adr_inet.sin_addr.s_addr;                    

    inet_aton("10.7.8.9", &adr_inet.sin_addr);
    str_dev_search->AlarmServerIP = (unsigned long)adr_inet.sin_addr.s_addr;
    str_dev_search->AlarmServerPort = (unsigned short)0x22b8; //8888

    //inet_aton("0.0.51.0", &adr_inet.sin_addr);
    //str_dev_search->SMTPServerIP = (unsigned long)adr_inet.sin_addr.s_addr;
    str_dev_search->resPort1 = 0x33;
    
    str_dev_search->HttpPort = st_ipc_net_info.httpPort; //0x80
    str_dev_search->HttpsPort = 0x1bb; //443
    str_dev_search->TCPPort = DAHUA_TCP_LISTEN_PORT; // 37777
    str_dev_search->TCPMaxConn = DAHUA_MAX_LINK_NUM; // 10
    str_dev_search->SSLPort = 0x9390; //37776
    str_dev_search->UDPPort = 0x9392; //37778
    str_dev_search->resPort2 = 0x1388; //5000
    inet_aton("239.255.42.42", &adr_inet.sin_addr); //0x ef ff 2a 2a
    str_dev_search->McastIP = (unsigned long)adr_inet.sin_addr.s_addr;   
    str_dev_search->McastPort = 0x8f3a; // 36666


    memcpy(send_msg + n_offset, &extra_data, sizeof(extra_data));
    n_offset += sizeof(extra_data); // sizeof(extra_data) = 88

    ////////////////////
    strcpy(send_msg + n_offset, (char*)st_ipc_net_info.mac);//xx:xx:xx:xx:xx:xx¸ñÊ½
    //memcpy(temp, &st_net_info.szMac, 17);
    n_offset += 17; // mac len = 17
    memcpy(send_msg + n_offset, dev_mode, strlen(dev_mode));
    n_offset += strlen(dev_mode);

    #if 1
    char *dev_serial = "3L04587PAG02123";
    char *dev_ver = "2.621.0000000.8.R";
    sprintf(a_extra_sting, "Name:%15s\r\nDevice:IPC\r\nIPv6Addr:/0;gateway:\r\nIPv6Addr:fe80::9202:a9ff:fe1b:f6b3/64;gateway:fe80::\r\nSerialNo:%s\r\nVersion:%s\r\n\r\n", dev_serial, dev_serial, dev_ver);
    #else
    //n_extra_len = strlen("Name:PZC3EW11102007\r\nDevice:IPC-IPVM3150F\r\nIPv6Addr:2001:250:3000:1::1:2/112;gateway:2001:250:3000:1::1:1\r\nIPv6Addr:fe80::9202:a9ff:fe1b:f6b3/64;gateway:fe80::\r\n\r\n");
    //tp5
    //sprintf(a_extra_sting, "Name:%15s\r\nDevice:%s\r\nIPv6Addr:/0;gateway:\r\nIPv6Addr:fe80::9202:a9ff:fe1b:f6b3/64;gateway:fe80::\r\nSerialNo:%s\r\nVersion:%s\r\n\r\n", DEV_SERIAL, DEV_DEVICE, DEV_SERIAL, DEV_VER);
    char str_name[15] = "IPC";
    sprintf(a_extra_sting, "Name:%9s\r\nDevice:%s\r\nIPv6Addr:2001:250:3000:1::1:2/112;gateway:2001:250:3000:1::1:1\r\nIPv6Addr:fe80::9202:a9ff:fe1b:f6b3/64;gateway:fe80::\r\nSerialNo:%s\r\n\r\n", str_name, DEV_NAME, DEV_SERIAL);
    //sprintf(a_extra_sting, "Name:IPC%5d*%-5d\r\nDevice:IPC-IPVM3150F\r\nIPv6Addr:2001:250:3000:1::1:2/112;gateway:2001:250:3000:1::1:1\r\nIPv6Addr:fe80::9202:a9ff:fe1b:f6b3/64;gateway:fe80::\r\n\r\n", nWidth, nHeight);
    //n_extra_len = strlen(a_extra_sting);
    #endif

    //n_extra_len = strlen(a_extra_sting);
    n_extra_len = strlen(a_extra_sting) + 1; //¶à¸ö 00
    strncpy(send_msg + n_offset, a_extra_sting, n_extra_len);
    n_offset += n_extra_len;

    //´Ó¹ã²¥µØÖ··¢ËÍÏûÏ¢  
    usleep(300000); //Ë¯Ãß500ºÁÃë£¬ÒÔÃânvrËÑË÷Êý¾ÝÌ«¶à£¬Òç³ö»º³åÇø£¬±£Ö¤nvrÃ¿´Î¶¼ÄÜËÑµ½Éè±¸
    //sleep(2);
    DahuaBoardCastSendTo(sock_send, DAHUA_SEND_PORT, send_msg, n_offset);
    
    return 0;
}

static int DahuaAckSearchLogon(int socket_fd, char *recv_msg)
{
    char mac[64];
    int n_offset = 0;
    ST_SDK_NETWORK_ATTR st_ipc_net_info;

    //»ñÈ¡±¾»úÍøÂç²ÎÊý£¬
    if(0 != goke_api_get_ioctrl(GET_NETWORK_CFG,&st_ipc_net_info))
    {
        PRINT_ERR("DMS_NET_GET_PLATFORMCFG fail\n");
        return -1;
    } 
    strcpy(mac, (char*)st_ipc_net_info.mac);
    PRINT_INFO("change net info cmd, mac:%s\n", mac);

    if(0 == strncmp(mac, recv_msg + DVRIP_HEAD_T_SIZE, 17))
    {
        char send_msg[BUFLEN] = {0};
        DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)send_msg;                
        ZERO_DVRIP_HEAD_T(t_msg_head)

        t_msg_head->c[1] = 0; //0£ºÃ»ÓÐ 1£º¾ßÓÐ¶à»­ÃæÔ¤ÀÀ¹¦ÄÜ 
        t_msg_head->c[2] = 17 + strlen(DEV_NAME); //uuid
        t_msg_head->c[8] = 0;
        /* µÇÂ¼Ê§°Ü·µ»ØÂë  0:ÃÜÂë²»ÕýÈ· 1:ÕÊºÅ²»´æÔÚ 3:ÕÊºÅÒÑ¾­µÇÂ¼ 4:ÕÊºÅ±»Ëø¶¨ 
5:ÕÊºÅ±»ÁÐÈëºÚÃûµ¥ 6:×ÊÔ´²»×ã£¬ÏµÍ³Ã¦ 7:°æ±¾²»¶Ô£¬ÎÞ·¨µÇÂ½ */
        t_msg_head->c[9] = 2; //µÇÂ¼Ê§°Ü·µ»ØÂë
        t_msg_head->c[10] = 1;  //Í¨µÀÊý    
        t_msg_head->c[11] = 9;   //ÊÓÆµ±àÂë·½Ê½ 8:MPEG4 9:H.264
        t_msg_head->c[12] = 51;  //Éè±¸ÀàÐÍ  51:IPCÀà²úÆ·
        t_msg_head->l[4] = 45;    //Éè±¸·µ»ØµÄÎ¨Ò»±êÊ¶ºÅ£¬±êÖ¾¸ÃÁ¬½Ó
        t_msg_head->c[28] = 0;  //ÊÓÆµÖÆÊ½£¬0: ±íÊ¾PALÖÆ     1: ±íÊ¾NTSCÖÆ
        t_msg_head->s[15] = 0x8101; //µÚ30¡¢31×Ö½Ú±£ÁôÒÔÏÂÖµ ´úÀíÍø¹Ø²úÆ·ºÅ0x8101, 0x8001, 0x8002, 0x8003

        n_offset = DVRIP_HEAD_T_SIZE;
        memcpy(send_msg + n_offset, mac, 17);
        n_offset += 17;
        memcpy(send_msg + n_offset, DEV_NAME, strlen(DEV_NAME));
        n_offset += strlen(DEV_NAME);                            

        t_msg_head->dvrip.cmd = ACK_LOG_ON; 

        //´Ó¹ã²¥µØÖ··¢ËÍÏûÏ¢
        DahuaBoardCastSendTo(socket_fd, DAHUA_SEND_PORT, send_msg, n_offset);

    }
    return 0;
}

static int DahuaAckChangeNetAttr(int socket_fd, char *recv_info)
{
    char mac[64];
    int n_offset = 0;

    //»ñÈ¡±¾»úÍøÂç²ÎÊý£¬
    ST_SDK_NETWORK_ATTR st_ipc_net_info;
    if(0 != goke_api_get_ioctrl(GET_NETWORK_CFG,&st_ipc_net_info))
    {
        PRINT_ERR("DMS_NET_GET_PLATFORMCFG fail\n");
        return -1;
    } 
    strcpy(mac, (char*)st_ipc_net_info.mac);
    PRINT_INFO("change net info cmd, mac:%s\n", mac);

    //check ±¾»úµÄmacÓë½ÓÊÕµÄmacÊÇ·ñÒ»ÖÂ
    if(0 == strncmp(mac, recv_info + DVRIP_HEAD_T_SIZE + sizeof(ZhiNuo_Dev_Search), 17)) 
    {
        ZhiNuo_Dev_Search * str_dev_search = (ZhiNuo_Dev_Search *)(recv_info + DVRIP_HEAD_T_SIZE);
        struct in_addr net_addr;
        memcpy(&net_addr, &str_dev_search->HostIP, sizeof(net_addr));
        PRINT_INFO("%d,HostIP:%s \n" , sizeof(ZhiNuo_Dev_Search), (char *)inet_ntoa(net_addr));
        memcpy(&net_addr, &str_dev_search->Submask, sizeof(net_addr));
        PRINT_INFO("Submask:%s\n" , (char *)inet_ntoa(net_addr));
        memcpy(&net_addr, &str_dev_search->GateWayIP, sizeof(net_addr));
        PRINT_INFO("GateWayIP:%s\n" , (char *)inet_ntoa(net_addr));
        memcpy(&net_addr, &str_dev_search->DNSIP, sizeof(net_addr));
        PRINT_INFO("DNSIP:%s\n" , (char *)inet_ntoa(net_addr));

        memcpy(&net_addr, &str_dev_search->HostIP, sizeof(net_addr));
        memset(st_ipc_net_info.ip, 0, GK_MAX_IP_LENGTH);
        strcpy(st_ipc_net_info.ip, (char*)inet_ntoa(net_addr));
        memset(st_ipc_net_info.mask, 0, GK_MAX_IP_LENGTH);
        memcpy(&net_addr, &str_dev_search->Submask, sizeof(net_addr));
        strcpy(st_ipc_net_info.mask, (char*)inet_ntoa(net_addr));
        memset(st_ipc_net_info.gateway, 0, GK_MAX_IP_LENGTH);
        memcpy(&net_addr, &str_dev_search->GateWayIP, sizeof(net_addr));
        strcpy(st_ipc_net_info.gateway, (char*)inet_ntoa(net_addr));
        memset(st_ipc_net_info.dns1, 0, GK_MAX_IP_LENGTH);
        memcpy(&net_addr, &str_dev_search->DNSIP, sizeof(net_addr));
        strcpy(st_ipc_net_info.dns1, (char*)inet_ntoa(net_addr));
        st_ipc_net_info.dhcp= 0;

        //ÉèÖÃ±¾»úÍøÂç²ÎÊý£¬
        if(0 != goke_api_set_ioctrl(SET_NETWORK_CFG,&st_ipc_net_info))
        {
            PRINT_ERR("DMS_NET_GET_PLATFORMCFG fail\n");
            return -1;
        } 

        char ch_param_type = 0;
        char ch_child_type = 0;
        char ch_config_edition = 0;
        //char ch_param_effect_flag = 0;
        char ch_child_type_limit = 0;
        char tem_buf[DVRIP_HEAD_T_SIZE] = {0};
        memcpy(tem_buf, recv_info, DVRIP_HEAD_T_SIZE);
        DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)tem_buf;    

        PRINT_INFO("hl:%d, v:%d, vextralen:%d\n", t_msg_head->dvrip.dvrip_hl, 
                t_msg_head->dvrip.dvrip_v, t_msg_head->dvrip.dvrip_extlen);

        ch_param_type = t_msg_head->c[16];
        ch_child_type = t_msg_head->c[17];
        ch_config_edition = t_msg_head->c[18];
        //ch_param_effect_flag = t_msg_head->c[20];
        ch_child_type_limit = t_msg_head->c[24];

        //
        PRINT_INFO("ch_param_type:%d, ch_child_type:%d, ch_config_edition:%d, ch_param_effect_flag:%d, ch_child_type_limit:%d\n",
                t_msg_head->c[16], t_msg_head->c[17], t_msg_head->c[18], t_msg_head->c[20], t_msg_head->c[24]); 

        char send_msg[BUFLEN] = {0};
        t_msg_head = (DVRIP_HEAD_T *)send_msg;                
        ZERO_DVRIP_HEAD_T(t_msg_head)
            t_msg_head->dvrip.cmd = ACK_SET_CONFIG;  
        t_msg_head->c[2] = 17 + strlen(DEV_NAME);   //macµØÖ·³¤¶È¼ÓÉÏÉè±¸ÀàÐÍ
        t_msg_head->c[8] = 0; //·µ»ØÂë0:³É¹¦1:Ê§°Ü2:Êý¾Ý²»ºÏ·¨3:ÔÝÊ±ÎÞ·¨ÉèÖÃ4:Ã»ÓÐÈ¨ÏÞ
        t_msg_head->c[9] = 1; //0£º²»ÐèÒªÖØÆô 1£ºÐèÒªÖØÆô²ÅÄÜÉúÐ§
        t_msg_head->c[16] = ch_param_type;
        t_msg_head->c[17] = ch_child_type; 
        t_msg_head->c[18] = ch_config_edition; 
        t_msg_head->c[24] = ch_child_type_limit;

        n_offset = DVRIP_HEAD_T_SIZE;
        memcpy(send_msg + n_offset, mac, 17);
        n_offset += 17;
        memcpy(send_msg + n_offset, DEV_NAME, strlen(DEV_NAME));
        n_offset += strlen(DEV_NAME);  

        t_msg_head->dvrip.cmd = ACK_SET_CONFIG; 

        //´Ó¹ã²¥µØÖ··¢ËÍÏûÏ¢
        DahuaBoardCastSendTo(socket_fd, DAHUA_SEND_PORT, send_msg, n_offset);                   
    }

    return 0;
}

static void DahuaSearchHandleLoop(int g_dahua_sock)
{
    // ¹ã²¥µØÖ·  
    struct sockaddr_in from;  
    bzero(&from, sizeof(struct sockaddr_in));   

    int len = sizeof(struct sockaddr_in);  
    char smsg[1024] = {0};  

    int ret = 0;
    struct timeval timeout;
    fd_set readfd; //¶ÁÎÄ¼þÃèÊö·û¼¯ºÏ
    while(1)  
    {
        if(0 == g_dahua_thread_run)
        {
            break;
        }

        timeout.tv_sec = 10;  //³¬Ê±Ê±¼äÎª10Ãë
        timeout.tv_usec = 0;
        FD_ZERO(&readfd);
        FD_SET(g_dahua_sock, &readfd);
        ret = select(g_dahua_sock + 1, &readfd, NULL, NULL, &timeout);
        if (FD_ISSET(g_dahua_sock, &readfd))
        {
            //´Ó¹ã²¥µØÖ·½ÓÊÜÏûÏ¢  
            memset(smsg, 0, sizeof(smsg));
            ret = recvfrom(g_dahua_sock, smsg, 1024 - 1, 0, (struct sockaddr*)&from,(socklen_t*)&len);  
            if(ret <= 0)  
            {  
                PRINT_ERR("read error....%d:%s\n", ret, strerror(errno));  
            }  
            else  
            {         
                DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)smsg; 
                char * extra = smsg + DVRIP_HEAD_T_SIZE; 
                //PRINT_INFO("type:%d, cmd:%x,len:%d,%s,extra:%d,fromip:%s,fromeport:%d\n", t_msg_head->c[16], t_msg_head->dvrip.cmd, ret, smsg, t_msg_head->dvrip.dvrip_extlen, inet_ntoa(from.sin_addr), from.sin_port);                  
                switch(t_msg_head->dvrip.cmd)
                {
                    case CMD_DEV_SEARCH:
                        //PRINT_INFO("search dev");
                        DahuaAckSearch(g_dahua_sock);
                        break;

                    case 0xb3:
                        PRINT_INFO("b3");
                        break;
                    case CMD_DAHUA_LOG_ON:
                        //´ó»ªÐÞ¸Äip²»ÓÃµÇÂ½£¬ÒÔÏÂ´úÂëÃ»ÓÐÖ´ÐÐµ½£¬Èç¹û´ó»ªÒÔºó¼ÓÁË¹¦ÄÜ¿ÉÒÔÓÃµ½
                        PRINT_INFO("name:%s, pass:%s, cmd:%x, extralen:%d, extra:%s, clienttype:%d, locktype:%d, hl:%d, v:%d\n", 
                                t_msg_head->dvrip.dvrip_p, &t_msg_head->dvrip.dvrip_p[8], t_msg_head->dvrip.cmd
                                , t_msg_head->dvrip.dvrip_extlen, extra, t_msg_head->dvrip.dvrip_p[18], 
                                t_msg_head->dvrip.dvrip_p[19], t_msg_head->dvrip.dvrip_hl, t_msg_head->dvrip.dvrip_v);
                        DahuaAckSearchLogon(g_dahua_sock, smsg);
                        break;
                    case 0xc1:
                        PRINT_INFO("change ip");
                        DahuaAckChangeNetAttr(g_dahua_sock, smsg);
                        break;
                    default:
                        PRINT_ERR("Dahua_Broadcast unknown cmd[%x] not support\n", t_msg_head->dvrip.cmd);
                        break;
                }
            }
        }
    }  

    return;
}



/*************************************************************
 * º¯Êý½éÉÜ£º´¦ÀínvrµÄ·¢¹ýÀ´µÄËÑË÷Éè±¸¹ã²¥°ü
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
void * DahuaBroadcastThread(void *param)
{
    DahuaSetThreadName("DahuaBroadcastThread");
    
    int g_dahua_sock = DahuaCreateBroadcastSock(DAHUA_RCV_PORT);
    if( g_dahua_sock < 0)
    {
        PRINT_ERR("create dahua broadcast fail\n");
        return NULL;
    }

    DahuaSearchHandleLoop(g_dahua_sock);
 

	if(g_dahua_sock > 0)
	{
		// ·¢ËÍ shutdown
		char send_msg[] = "shutdown";
    	int send_len = strlen(send_msg);
		DahuaBoardCastSendTo(g_dahua_sock, DAHUA_SEND_PORT, send_msg, send_len);

		//¹Ø±Õsocket
		close(g_dahua_sock);		
	}

    PRINT_INFO("DaHua_Broadcast_Thread end !\n");
    return NULL;  
}  

#endif

static int Dahua_Notify_Md(int sock, int id, int sid2, int sid)
{
    char *out;
    cJSON *root;	
    root = cJSON_CreateObject();//´´½¨ÏîÄ¿

    cJSON_AddNumberToObject(root, "id", id); //134955
    cJSON_AddStringToObject(root, "method", "client.notifySnapFile");
    cJSON *params;
    cJSON_AddItemToObject(root, "params", params = cJSON_CreateObject());
    
    cJSON_AddNumberToObject(params, "SID", sid2); //262148
    
    cJSON *info;
    cJSON_AddItemToObject(params, "info", info = cJSON_CreateObject());
    cJSON_AddNumberToObject(info, "Channel", 0);
    cJSON_AddNumberToObject(info, "EOF", 1);
    cJSON *Events, *item;
    cJSON_AddItemToObject(info, "Events", Events = cJSON_CreateArray());
    cJSON_AddItemToArray(Events, item = cJSON_CreateObject());
    cJSON_AddStringToObject(item, "Action", "Start");
    cJSON_AddStringToObject(item, "Code", "VideoMotion");
    cJSON *Data;
    cJSON_AddItemToObject(item, "Data", Data = cJSON_CreateObject());
    const char *sRegionName[1] = {"Region1"}; 
    cJSON_AddItemToObject(Data, "RegionName", cJSON_CreateStringArray(sRegionName, 1));

    cJSON_AddNumberToObject(item, "Index", 0);

    cJSON_AddNumberToObject(info, "PictureType", -1);
    cJSON_AddNumberToObject(info, "SOF", 0);

    time_t timep2;
    struct tm *p2;
    time(&timep2);
    struct tm tt2 = {0};
    //p2 = gmtime_r(&timep2, &tt2);
    p2 = localtime_r(&timep2, &tt2);

    int year, mon, day, hour, min, sec;
    year = p2->tm_year + 1900;
    mon = p2->tm_mon + 1;
    day = p2->tm_mday;
    hour = p2->tm_hour;
    min = p2->tm_min;
    sec = p2->tm_sec;

    char str[30] = {0};
    sprintf(str, "%04d-%02d-%02d %02d:%02d:%02d", year, mon, day, hour, min, sec);
    cJSON_AddStringToObject(info, "Time", str);


    cJSON_AddNumberToObject(root, "session", sid);

    /////////////////
    out=cJSON_PrintUnformatted(root);
#if DAHUA_PRINT_CJSON_MSG
    char *out2 = cJSON_Print(root);
    PRINT_INFO("send:%s\n",out2);
    free(out2);
#endif

    // °Ñ cjson ÐÅÏ¢¼ÓÈë·¢ËÍ»º³åÇø
    char fSendbuf[10 * 1024] ={0};
    int nSendLen = 0;
    memset(fSendbuf, 0, sizeof(fSendbuf));
    sprintf(fSendbuf + sizeof(DVRIP_HEAD_T), "%s", out);
    nSendLen = sizeof(DVRIP_HEAD_T) + strlen(out);
    
    free(out);
    cJSON_Delete(root);

    
    //¼Ó°üÍ·
    DVRIP_HEAD_T *pMsgHeader = (DVRIP_HEAD_T *)(fSendbuf);  
    ZERO_DVRIP_HEAD_T(pMsgHeader)
        pMsgHeader->dvrip.cmd = ACK_UNKOWN;  

    pMsgHeader->l[1] = nSendLen - sizeof(DVRIP_HEAD_T);
    pMsgHeader->l[4] = nSendLen - sizeof(DVRIP_HEAD_T);
    pMsgHeader->l[2] = id;
    pMsgHeader->l[6] = sid;

    //·¢ËÍ
    int ret = DahuaTcpSend(sock, fSendbuf, nSendLen, 2);
    if (ret < 0) {
        PRINT_ERR();
        return -1;  
    }

    return 0;
}

int dahua_notify_to_all_client()
{
    int id = 134955;
    int sid2 = 262148;

    int ret = 0;
    int sock = -1;
    int sid = 0;
    int i;
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        sock = UtGetCmdSockByIndex(i, &g_dahua_session);
        if (sock > 0) {
            //»ñÈ¡sid
            sid = UtGetSidByIndex(i, &g_dahua_session);
            PRINT_INFO("alarm md send to sock:%d, sid:%d\n", sock, sid);
            
            ret = Dahua_Notify_Md(sock, id, sid2, sid);
            //Èç¹û·¢ËÍÊ§°Ü£¬Ôò¹Ø±Õ¶ÔÓ¦µÄdata sock
            if (ret <0) {
                continue;
            }
        }  
    }
    return 0;
}



#if 0
static char xml_content1[] = "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\" ?>\
<tt:MetaDataStream xmlns:tt=\"http://www.onvif.org/ver10/schema\" \
xmlns:wsnt=\"http://docs.oasis-open.org/wsn/b-2\">\
<tt:VideoAnalytics><tt:Frame UtcTime=\"";

static char xml_content2[] = "\"><tt:Transformation>\
<tt:Translate x=\"-1\" y=\"-1\"/><tt:Scale x=\"0.00625\" y=\"0.00834\"/>\
</tt:Transformation><tt:Extension><tt:MotionInCells Columns=\"22\" Rows=\"18\" Cells=\"zwA=\"/>\
</tt:Extension></tt:Frame></tt:VideoAnalytics><tt:Event><wsnt:NotificationMessage>\
<wsnt:Topic Dialect=\"http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet\">\
tns1:RuleEngine/CellMotionDetector/Motion</wsnt:Topic><wsnt:Message><tt:Message UtcTime=\"";

static char xml_content3[] = "\" PropertyOperation=\"Changed\">\
<tt:Source><tt:SimpleItem Name=\"VideoSourceConfigurationToken\" Value=\"000\"/>\
<tt:SimpleItem Name=\"VideoAnalyticsConfigurationToken\" Value=\"000\"/>\
<tt:SimpleItem Name=\"Rule\" Value=\"000\"/></tt:Source><tt:Data>\
<tt:SimpleItem Name=\"IsMotion\" Value=\"true\"/></tt:Data></tt:Message>\
</wsnt:Message></wsnt:NotificationMessage></tt:Event></tt:MetaDataStream>";
#endif

int dahua_notify_to_all_client2()
{
    int i = 0;
    int sock = 0;
    int ret = 0;
    char buf[2000] = {0};

    int year, mon, day, hour, min, sec;

    time_t timep2;
    struct tm *p2;
    time(&timep2);
    struct tm tt2 = {0};
    p2 = gmtime_r(&timep2, &tt2);

    #if 0
    year = p2->tm_year + 1900;
    mon = p2->tm_mon + 1;
    day = p2->tm_mday;
    hour = p2->tm_hour;
    min = p2->tm_min;
    sec = p2->tm_sec;
    #else
    year = 2018;
    mon = 3;
    day = 6;
    hour = 13;
    min = 15;
    sec = 04;

    #endif
    
    char xml_buf[2000] = {0};
    sprintf(xml_buf, "<?xml version=\"1.0\" encoding=\"utf-8\" standalone=\"yes\" ?>"
        "<tt:MetaDataStream xmlns:tt=\"http://www.onvif.org/ver10/schema\" "
        "xmlns:wsnt=\"http://docs.oasis-open.org/wsn/b-2\"><tt:VideoAnalytics>"
        "<tt:Frame UtcTime=\"%04d-%02d-%02dT%02d:%02d:%02dZ\"><tt:Transformation>"
        "<tt:Translate x=\"-1\" y=\"-1\"/><tt:Scale x=\"0.00625\" y=\"0.00834\"/>"
        "</tt:Transformation><tt:Extension><tt:MotionInCells Columns=\"22\" Rows=\"18\" "
        "Cells=\"zwA=\"/>"
        "</tt:Extension></tt:Frame></tt:VideoAnalytics><tt:Event><wsnt:NotificationMessage>"
        "<wsnt:Topic Dialect=\"http://www.onvif.org/ver10/tev/topicExpression/ConcreteSet\">"
        "tns1:RuleEngine/CellMotionDetector/Motion</wsnt:Topic><wsnt:Message>"
        "<tt:Message UtcTime=\"%04d-%02d-%02dT%02d:%02d:%02dZ\" PropertyOperation=\"Changed\">"
        "<tt:Source><tt:SimpleItem Name=\"VideoSourceConfigurationToken\" Value=\"000\"/>"
        "<tt:SimpleItem Name=\"VideoAnalyticsConfigurationToken\" Value=\"000\"/>"
        "<tt:SimpleItem Name=\"Rule\" Value=\"000\"/></tt:Source><tt:Data>"
        "<tt:SimpleItem Name=\"IsMotion\" Value=\"true\"/></tt:Data></tt:Message>"
        "</wsnt:Message></wsnt:NotificationMessage></tt:Event></tt:MetaDataStream>", 
        year, mon, day, hour, min, sec, year, mon, day, hour, min, sec);
    int xml_len = strlen(xml_buf);
    
    
    int media_len = sizeof(ZhiNuo_Media_Frame_Head) + sizeof(ZhiNuo_Expand_Data_Check) + xml_len + sizeof(ZhiNuo_Media_Frame_Tail); 

    //MSGÍ·Ìî³ä
    int offset = 0;
    DahuaAddAlarmMsgHead(buf + offset, ACK_MEDIA_REQUEST, media_len);
    offset += DVRIP_HEAD_T_SIZE;

    //Ö¡Í·Ìî³ä
    DahuaAddAlarmHead(buf + offset, media_len);
    offset += sizeof(ZhiNuo_Media_Frame_Head);
    DahuaAddExDataCheck(buf + offset);
    offset += sizeof(ZhiNuo_Expand_Data_Check);

    //Ö¡Î²Ìî³ä
    ZhiNuo_Media_Frame_Tail t_frame_tail;
    t_frame_tail.sz_tag[0] = 'd';
    t_frame_tail.sz_tag[1] = 'h';
    t_frame_tail.sz_tag[2] = 'a';        
    t_frame_tail.sz_tag[3] = 'v';  
    t_frame_tail.unl_data_len = media_len;  

    //¿½±´Êý¾Ý
    memcpy(buf + offset, xml_buf, xml_len);
    offset += xml_len;
    memcpy(buf + offset, &t_frame_tail, sizeof(ZhiNuo_Media_Frame_Tail));


    int nSendLen = media_len + DVRIP_HEAD_T_SIZE;

    pthread_mutex_lock(&g_dahua_stream_mutex);
    //·¢ËÍ¸ødata sock
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
		sock = UtGetLvSockByIndex(i, 0, &g_dahua_session);
        if (sock > 0) {
            PRINT_INFO("stream0, alarm md send to sock:%d\n", sock);
            ret = DahuaTcpSend(sock, buf, nSendLen, 2);
            //Èç¹û·¢ËÍÊ§°Ü£¬Ôò¹Ø±Õ¶ÔÓ¦µÄdata sock
            if (ret <0) {
                continue;
            }
        }
    }

    //·¢ËÍ¸ødata sock
    for (i = 0; i < UT_MAX_LINK_NUM; i++) {
        sock = UtGetLvSockByIndex(i, 1, &g_dahua_session);
        if (sock > 0) {
            PRINT_INFO("stream1, alarm md send to sock:%d\n", sock);
            ret = DahuaTcpSend(sock, buf, nSendLen, 2);
            //Èç¹û·¢ËÍÊ§°Ü£¬Ôò¹Ø±Õ¶ÔÓ¦µÄdata sock
            if (ret <0) {
                continue;
            }
        }
    }
    pthread_mutex_unlock(&g_dahua_stream_mutex);

    
    return 0;
}


int dahua_notify_alarm_md()
{
    #if 0
    static int last_md_sec = 0;
    struct timeval t1 = {0, 0};
    // 3ÃëÄÚ£¬²»ÖØ¸´ÏìÓ¦Í¬Ò»ÖÖ±¨¾¯
    int interval = runMdCfg.handle.intervalTime;
    if (interval <= 0 || interval > 60) {
        //PRINT_INFO("interval %d set to 3 seconds\n", interval);
        interval = 3;
    }
    //PRINT_INFO("interval %d\n", interval);
    gettimeofday(&t1, NULL);
    if ((t1.tv_sec - last_md_sec) <= interval) {
        //PRINT_INFO("time is too closed, so not to notify.\n");
        return 0;
    }
    last_md_sec = t1.tv_sec;
    #endif

    dahua_notify_to_all_client2();
    dahua_notify_to_all_client();

    return 0;
}


int dahua_alarm_cb_func(int nChannel, int nAlarmType, int nAction, void* pParam)
{
    PRINT_INFO("nAlarmType:%d, nAction:%d\n", nAlarmType, nAction);

	if(nAction == 0)
	{
		//PRINT_INFO("\n");
		//return 0;
	}

    char eventType[20] = {0};
	char strAction[20] = {0};

    #if 0
    long ts = time(NULL);
    struct tm tt = {0};
    struct tm *pTm = localtime_r(&ts, &tt);
    char startTime[128] = {0};
    sprintf(startTime,"%d-%02d-%02d %02d:%02d:%02d",pTm->tm_year+1900,pTm->tm_mon+1,pTm->tm_mday,pTm->tm_hour,pTm->tm_min,pTm->tm_sec);
    #endif

    switch(nAlarmType) {
        case GK_ALARM_TYPE_ALARMIN:     //0:ÐÅºÅÁ¿±¨¾¯¿ªÊ¼
        	sprintf(eventType ,"%s", "LocalIO");
			sprintf(strAction ,"%s", "Start");
			break;

        case GK_ALARM_TYPE_DISK_FULL:       //1:Ó²ÅÌÂú
			sprintf(eventType ,"%s", "StorrageLowSpace");
			break;

        case GK_ALARM_TYPE_VLOST:            //2:ÐÅºÅ¶ªÊ§
        	sprintf(eventType ,"%s", "VideoLoss");
			sprintf(strAction ,"%s", "Start");
			break;

        case GK_ALARM_TYPE_VMOTION:          //3:ÒÆ¶¯Õì²â
			sprintf(eventType ,"%s", "VideoMotion");
			sprintf(strAction ,"%s", "Start");
            dahua_notify_alarm_md();
            break;
        case GK_ALARM_TYPE_DISK_UNFORMAT:    //4:Ó²ÅÌÎ´¸ñÊ½»¯
			sprintf(eventType ,"%s", "StorageNotExist");
			break;

        case GK_ALARM_TYPE_DISK_RWERR:       //5:¶ÁÐ´Ó²ÅÌ³ö´í,
			sprintf(eventType ,"%s", "StorageFailure");
			break;

        case GK_ALARM_TYPE_VSHELTER:         //6:ÕÚµ²±¨¾¯
			sprintf(eventType ,"%s", "VideoBlind");
			break;

        case GK_ALARM_TYPE_ALARMIN_RESUME:         //9:ÐÅºÅÁ¿±¨¾¯»Ö¸´
			sprintf(eventType ,"%s", "LocalIO");
			sprintf(strAction ,"%s", "Stop");
			break;

        case GK_ALARM_TYPE_VLOST_RESUME:         //10:ÊÓÆµ¶ªÊ§±¨¾¯»Ö¸´
			sprintf(eventType ,"%s", "VideoLoss");
			sprintf(strAction ,"%s", "Stop");
			break;

        case GK_ALARM_TYPE_VMOTION_RESUME:         //11:ÊÓÆµÒÆ¶¯Õì²â±¨¾¯»Ö¸´
			sprintf(eventType ,"%s", "VideoMotion");
			sprintf(strAction ,"%s", "Stop");
			break;

        case GK_ALARM_TYPE_NET_BROKEN:       //12:ÍøÂç¶Ï¿ª
			sprintf(eventType ,"%s", "NetAbort");
			break;

        case GK_ALARM_TYPE_IP_CONFLICT:      //13:IP³åÍ»
			sprintf(eventType ,"%s", "IPConfict");
			break;

        default:
			PRINT_INFO("Alarm: Type = %d", nAlarmType);
        	break;
    }

    PRINT_INFO("alarm_type:%s action:%s\n", eventType, strAction);

    return 0;
}


int dahua_protocol_start()
{   
    printf("dahua_alarm_cb_func:%x\n", (int)dahua_alarm_cb_func);
    g_dahua_alarm_handle = event_alarm_open(dahua_alarm_cb_func);


    //////
    UtSessionGlobalInit(&g_dahua_session);
    pthread_mutex_init(&g_dahua_stream_mutex,NULL);
    
    g_dahua_thread_run = 1;

    //´´½¨Ö÷Ïß³Ì
    DahuaCreateThread(DahuaMainThread, NULL, NULL);
    //´´½¨Éè±¸ËÑË÷´¦ÀíÏß³Ì£¬´¦Àí¹ã²¥°ü
    DahuaCreateThread(DahuaBroadcastThread, NULL, NULL);

    //´´½¨Êý¾Ý·¢ËÍÏß³Ì
    DahuaCreateThread(DahuaSendMainStreamPthread, NULL, NULL);
    DahuaCreateThread(DahuaSendSubStreamPthread, NULL, NULL);


    return 0;
}


int dahua_protocol_stop()
{
    /////
    g_dahua_thread_run = 0;

    event_alarm_close(g_dahua_alarm_handle);
    
    PRINT_INFO("lib_dahua_close !\n");
    return 0;
}

