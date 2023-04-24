#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <string.h>


#include "sdk_network.h"
#include "cfg_video.h"
#include "common.h"
#include "media_fifo.h"
#include "cfg_image.h"
#include "cfg_channel.h"
#include "netcam_api.h"

#include "dahua_session.h"


#define DEBUG 0
#define ADD_MD_INFO	0

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

typedef struct dahua_sock{
    int fd;
    int n_flag;
    pthread_mutex_t send_mutex;
}DAHUA_SOCK_T;



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

static int g_dahua_protocol_run = 1;
static unsigned long g_video_sequence_main_stream = 0;
static unsigned long g_audio_sequence_main_stream = 0;
static unsigned short g_video_timestample_main_stream = 0;
static unsigned short g_audio_timestample_main_stream = 0;
static unsigned long g_video_sequence_second_stream = 0;
static unsigned long g_audio_sequence_second_stream = 0;
static unsigned short g_video_timestample_second_stream = 0;
static unsigned short g_audio_timestample_second_stream = 0;
static pthread_mutex_t main_second_stream_mutex = PTHREAD_MUTEX_INITIALIZER;
static DAHUA_SOCK_T* g_socket_av_send[MAX_LINK_NUM];

unsigned int audio_alaw_encode(unsigned char *dst, short *src, unsigned int srcSize);


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
                //bruce
                //ret = netcam_osd_set_info(chn, param);
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




static int string2hex(unsigned char s[],unsigned char bits[]) {
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
static int net_create_broadcast_socket(int port)
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
#if 0
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

static int dahua_ack_search(int sock_send)
{
    PRINT_INFO("dahua_ack_search\n");
    int n_offset = 0;
    char extra_data[88] = {0};
    int i = 0, ii = 0;
    int width= 0;
    int height= 0;
    int n_max_resolution = 0;
    int n_extra_len = 0;
    char a_extra_sting[255];    
    ZhiNuo_Dev_Search * str_dev_search = (ZhiNuo_Dev_Search *)extra_data;

    //»ñÈ¡±¾»úÍøÂç²ÎÊý£¬
    ST_SDK_NETWORK_ATTR st_ipc_net_info;
    if(0 != goke_api_get_ioctrl(GET_NETWORK_CFG, &st_ipc_net_info))
    {
        PRINT_ERR("GET_NETWORK_CFG fail\n");
        return 0;
    }    

    str_dev_search->Version[0] = 0X32;  //°æ±¾ÐÅÏ¢ 
    str_dev_search->Version[1] = 0X33; 
    str_dev_search->Version[2] = 0X30; 
    str_dev_search->Version[3] = 0X32;  

    str_dev_search->HostName[0] = 0x44; //Ö÷»úÃû
    str_dev_search->HostName[1] = 0x56;
    str_dev_search->HostName[2] = 0x52;

    struct sockaddr_in adr_inet; 
    //ÅäÖÃ
    struct in_addr net_addr;
    inet_aton(st_ipc_net_info.ip, &adr_inet.sin_addr);
    str_dev_search->HostIP = (unsigned long)adr_inet.sin_addr.s_addr;
    memcpy(&net_addr, &str_dev_search->HostIP, sizeof(net_addr));
    PRINT_INFO("ip:%s\n", (char *)inet_ntoa(net_addr));
    inet_aton(st_ipc_net_info.mask, &adr_inet.sin_addr);
    str_dev_search->Submask = (unsigned long)adr_inet.sin_addr.s_addr;
    inet_aton(st_ipc_net_info.gateway, &adr_inet.sin_addr);
    str_dev_search->GateWayIP = (unsigned long)adr_inet.sin_addr.s_addr;  
    inet_aton(st_ipc_net_info.dns1, &adr_inet.sin_addr);
    str_dev_search->DNSIP = (unsigned long)adr_inet.sin_addr.s_addr; 					

    inet_aton("10.1.0.2", &adr_inet.sin_addr);
    str_dev_search->AlarmServerIP = (unsigned long)adr_inet.sin_addr.s_addr; 
    str_dev_search->HttpPort = st_ipc_net_info.httpPort;
    str_dev_search->TCPPort = TCP_LISTEN_PORT;
    str_dev_search->TCPMaxConn = MAX_LINK_NUM;   
    str_dev_search->SSLPort = 8443;
    str_dev_search->UDPPort = 8001;
    inet_aton("239.255.42.42", &adr_inet.sin_addr);
    str_dev_search->McastIP = (unsigned long)adr_inet.sin_addr.s_addr;   
    str_dev_search->McastPort = 36666;

    //rcv_msg_len = DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen;
    char send_msg[512] = {0};
    DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)send_msg;                
    memset(send_msg, 0, sizeof(send_msg));
    t_msg_head->c[2] = 17 + strlen(DEV_NAME);   //macµØÖ·³¤¶È¼ÓÉÏÉè±¸ÀàÐÍ
    t_msg_head->dvrip.dvrip_extlen = 0x58;  //Îªextra_dataÊý×éµÄ³¤¶È           
    t_msg_head->dvrip.cmd = ACK_DEV_SEARCH;   
    t_msg_head->c[16] = 2; //¹Ì¶¨Îª2
    t_msg_head->c[19] = 0; //·µ»ØÂë 0:Õý³£ 1:ÎÞÈ¨ÏÞ 2:ÎÞ¶ÔÓ¦ÅäÖÃÌá¹©
    t_msg_head->c[20] = 0xa3; //¸ù¾Ý×¥°ü±¨ÎÄÀïÃæµÄÖµ×öµÄÌî³ä

    struct sockaddr_in udpaddrto;  
    bzero(&udpaddrto, sizeof(struct sockaddr_in));  
    udpaddrto.sin_family = AF_INET;  
    udpaddrto.sin_addr.s_addr = inet_addr("255.255.255.255"); //htonl(INADDR_BROADCAST);  //
    udpaddrto.sin_port = htons(DAHUA_SEND_PORT);   
    n_offset += DVRIP_HEAD_T_SIZE;

    memcpy(send_msg + n_offset, &extra_data, sizeof(extra_data));
    n_offset += sizeof(extra_data);
    strcpy(send_msg + n_offset, (char*)st_ipc_net_info.mac);//xx:xx:xx:xx:xx:xx¸ñÊ½
    //memcpy(temp, &st_net_info.szMac, 17);
    n_offset += 17;
    memcpy(send_msg + n_offset, DEV_NAME, strlen(DEV_NAME));
    n_offset += strlen(DEV_NAME);

    n_max_resolution = 0;
    GK_NET_VIDEO_CFG video_cfg;
    goke_api_get_ioctrl(GET_VIDEO_CFG,&video_cfg);
    for(i = 0; i < GK_MAX_VIDEO_CHN; i++)
    {
        for(ii = 0; ii < GK_MAX_STREAMNUM; ii ++)
        {
            if(video_cfg.vencStream[i].h264Conf.width* video_cfg.vencStream[i].h264Conf.height> n_max_resolution)
            {
                width= video_cfg.vencStream[i].h264Conf.width;
                height= video_cfg.vencStream[i].h264Conf.height;
                n_max_resolution = width* height;
            }
        }                       
    } 

    n_extra_len = strlen("Name:PZC3EW11102007\r\nDevice:IPC-IPVM3150F\r\nIPv6Addr:2001:250:3000:1::1:2/112;gateway:2001:250:3000:1::1:1\r\nIPv6Addr:fe80::9202:a9ff:fe1b:f6b3/64;gateway:fe80::\r\n\r\n");
    sprintf(a_extra_sting, "Name:IPC%5d*%-5d\r\nDevice:IPC-IPVM3150F\r\nIPv6Addr:2001:250:3000:1::1:2/112;gateway:2001:250:3000:1::1:1\r\nIPv6Addr:fe80::9202:a9ff:fe1b:f6b3/64;gateway:fe80::\r\n\r\n", width, height);
    strncpy(send_msg + n_offset, a_extra_sting, n_extra_len);
    //strcpy(send_msg + n_offset, "Name:IPC%d*%d\r\nDevice:IPC-IPVM3150F\r\nIPv6Addr:2001:250:3000:1::1:2/112;gateway:2001:250:3000:1::1:1\r\nIPv6Addr:fe80::9202:a9ff:fe1b:f6b3/64;gateway:fe80::\r\n\r\n");
    n_offset += n_extra_len;

    //´Ó¹ã²¥µØÖ··¢ËÍÏûÏ¢  
    usleep(500000); //Ë¯Ãß500ºÁÃë£¬ÒÔÃânvrËÑË÷Êý¾ÝÌ«¶à£¬Òç³ö»º³åÇø£¬±£Ö¤nvrÃ¿´Î¶¼ÄÜËÑµ½Éè±¸
    int ret = sendto(sock_send, send_msg, n_offset, 0, (struct sockaddr*)&udpaddrto, sizeof(udpaddrto));  
    if(ret <= 0)  
    {  
        PRINT_ERR("send error....:%d\n", ret);  
    }   
    return 0;
}
static int dahua_ack_search_log(int socket_fd, char *recv_msg)
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

        struct sockaddr_in udpaddrto;  
        bzero(&udpaddrto, sizeof(struct sockaddr_in));  
        udpaddrto.sin_family = AF_INET;  
        udpaddrto.sin_addr.s_addr = inet_addr("255.255.255.255"); //htonl(INADDR_BROADCAST);  //
        udpaddrto.sin_port = htons(DAHUA_SEND_PORT);  

        t_msg_head->dvrip.cmd = ACK_LOG_ON; 
        //´Ó¹ã²¥µØÖ··¢ËÍÏûÏ¢  
        int ret = sendto(socket_fd, send_msg, n_offset, 0, (struct sockaddr*)&udpaddrto, sizeof(udpaddrto));  
        if(ret <= 0)  
        {  
            PRINT_ERR("send error....:%d\n", ret);  
        }  
    }
    return 0;
}

static int dahua_ack_change_net_attr(int socket_fd, char *recv_info)
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
        char ch_param_effect_flag = 0;
        char ch_child_type_limit = 0;
        char tem_buf[DVRIP_HEAD_T_SIZE] = {0};
        memcpy(tem_buf, recv_info, DVRIP_HEAD_T_SIZE);
        DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)tem_buf;    

        PRINT_INFO("hl:%d, v:%d, vextralen:%d\n", t_msg_head->dvrip.dvrip_hl, 
                t_msg_head->dvrip.dvrip_v, t_msg_head->dvrip.dvrip_extlen);

        ch_param_type = t_msg_head->c[16];
        ch_child_type = t_msg_head->c[17];
        ch_config_edition = t_msg_head->c[18];
        ch_param_effect_flag = t_msg_head->c[20];
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

        struct sockaddr_in udpaddrto;  
        bzero(&udpaddrto, sizeof(struct sockaddr_in));  
        udpaddrto.sin_family = AF_INET;  
        udpaddrto.sin_addr.s_addr = inet_addr("255.255.255.255"); //htonl(INADDR_BROADCAST);  //
        udpaddrto.sin_port = htons(DAHUA_SEND_PORT);  

        t_msg_head->dvrip.cmd = ACK_SET_CONFIG; 

        //´Ó¹ã²¥µØÖ··¢ËÍÏûÏ¢  
        int ret = sendto(socket_fd, send_msg, n_offset, 0, (struct sockaddr*)&udpaddrto, sizeof(udpaddrto));  
        if(ret <= 0)  
        {  
            PRINT_ERR("send error....:%d\n", ret);  
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
 * º¯Êý½éÉÜ£º´Ósocket½ÓÊÕÊý¾Ý
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_SockRecv(int sock, char *szbuf, int len, int timeout)
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

    //PRINT_INFO("1finish ZhiNuo_SockRecv:%d, %s\n", len, strerror(errno));
    return len;
}

/*************************************************************
 * º¯Êý½éÉÜ£º´Ósocket·¢ËÍÊý¾Ý
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_SockSend(DAHUA_SOCK_T* sock, char *szbuf, int len, int timeout)
{
    int ret = -1;
    fd_set send_set, excpt_set;
    struct timeval tv;
    int send_total = 0;

    if((sock->fd < 0) || (sock->n_flag == -1))
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
            return -3;
                goto error;
            }
        }
        if(ret < len - send_total)
        {            
            PRINT_ERR("SockSend  data short short short short:%d\n", ret); 
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

/*************************************************************
 * º¯Êý½éÉÜ£ºÃ¿¸ô5ms·¢ËÍ3kÊý¾Ý
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_Send_Main_Unit(DAHUA_SOCK_T *sock, char *buf, unsigned long len, int nOnePacketSize)
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
    t_msg_head.c[24] = 0; //Ö÷ÂëÁ÷
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
 * º¯Êý½éÉÜ£ºÃ¿¸ô5ms·¢ËÍ3kÊý¾Ý
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_Send_Second_Unit(DAHUA_SOCK_T* sock, char *buf, unsigned long len, int nOnePacketSize)
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
int ZhiNuo_Package_Second_Stream(char* send_buff, const char * data, DWORD data_len, char frame_type, 
        char ch_width, char ch_high)
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
#ifdef DAHA_PRTCL_OK
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
        DMS_NET_CHANNEL_PIC_INFO t_pic_info;
        ret = dms_sysnetapi_ioctrl(g_dahua_handle, DH_NET_GET_SUPPORT_STREAM_FMT, 0, (void *)(&t_pic_info), sizeof(t_pic_info)); 
        if(0 != ret)
        {
            PRINT_ERR("DH_NET_GET_SUPPORT_STREAM_FMT error\n");

        }
        t_expand_playback_type->ch_frame_rate = t_pic_info.stRecordPara.dwFrameRate; //»ñÈ¡Ö¡ÂÊ
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

    for(i = 0; i < MAX_LINK_NUM; i++)
    {   
        if((NULL != g_socket_av_send[i]) && (ZHINUO_NVR == g_socket_av_send[i]->n_flag))
        {    
            printf("send second fd:%d\n", g_socket_av_send[i]->fd);
            ret = ZhiNuo_Send_Second_Unit(g_socket_av_send[i], send_buff, n_offset + data_len + sizeof(ZhiNuo_Media_Frame_Tail), ZHINUO_ONE_PACKAGE_SIZE);
            if(ret <= 0)
            {
                PRINT_ERR("exit second av socket:%d\n", g_socket_av_send[i]->fd);  
                g_socket_av_send[i]->n_flag = -1;
            }
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
int ZhiNuo_Send_Second_Stream()
{
    int i = 0;
    BOOL b_have_connect = FALSE;    
    int ret;
    GK_NET_FRAME_HEADER frame_header;

    void *p_data_buff = NULL;
    int data_len = 0;

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

    while(g_dahua_protocol_run)
    {   
        //±£Ö¤Ã»ÓÐÁ¬½ÓµÄÊ±ºò²»»ñÈ¡ÒôÊÓÆµÊý¾Ý
        b_have_connect = FALSE;
        for(i = 0; i < MAX_LINK_NUM; i++)
        {   
            if((NULL != g_socket_av_send[i]) && (ZHINUO_NVR == g_socket_av_send[i]->n_flag))
            {
                b_have_connect = TRUE; 
            }
        }  
        if(FALSE == b_have_connect)
        {
            usleep(20000);
            continue;
        } 
        while(g_dahua_protocol_run)
        {
            memset(&frame_header, 0, sizeof(frame_header));
            p_data_buff= NULL;
            ret = mediabuf_read_frame((MEDIABUF_HANDLE)handle, &p_data_buff, &data_len, &frame_header);
            if (ret <= 0)
                goto SEND_SECOND_STREAM_END;          
            break;    
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
        pthread_mutex_lock(&main_second_stream_mutex); 
        ret = ZhiNuo_Package_Second_Stream(send_buff, p_data_buff, data_len, ch_frame_type, 
                (char)(((frame_header.video_reso >> 16) & 0xff)/8), (char)((frame_header.video_reso & 0xff)/8)); //8ÏñËØµãÎª1µ¥Î»
        pthread_mutex_unlock(&main_second_stream_mutex);             
        if(ret < 0)
        {
            break;
        }  
    }  
	SEND_SECOND_STREAM_END:
    free(send_buff); 
	mediabuf_del_reader(handle);
    return 0;     
}

/*************************************************************
 * º¯Êý½éÉÜ£º¸øÒôÊÓÆµ´ò°ü
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_Package_Main_Stream(char* send_buff, const char * data, DWORD data_len, char frame_type, 
        char ch_width, char ch_high, unsigned char fps)
{
    int ret = 0;
    int nOnePacketSize = 0;
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
        t_media_data_head->n_frame_sequence = g_audio_sequence_main_stream;
        t_media_data_head->s_timestamp = g_audio_timestample_main_stream;
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
        t_expand_playback_type->ch_reserve = 0xff; //±£ÁôÎ»Îª0
        t_expand_playback_type->ch_video_type = (char)H264; 

        t_expand_playback_type->ch_frame_rate = fps;        
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

    i = 0;
    for(i = 0; i < MAX_LINK_NUM; i++)
    {   
        if((NULL != g_socket_av_send[i]) && (g_socket_av_send[i]->n_flag != -1))
        {
            if(DAHUA_NVR == g_socket_av_send[i]->n_flag)
            {
                nOnePacketSize = DAHUA_ONE_PACKAGE_SIZE;
            }
            else
            {
                nOnePacketSize = ZHINUO_ONE_PACKAGE_SIZE;
            }
            ret = ZhiNuo_Send_Main_Unit(g_socket_av_send[i], send_buff, n_offset + data_len + sizeof(ZhiNuo_Media_Frame_Tail), nOnePacketSize);
            if(ret <= 0)
            {
                PRINT_ERR("closesocket main av socket:%d\n", g_socket_av_send[i]->fd);  
                g_socket_av_send[i]->n_flag = -1;  
            }
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
int ZhiNuo_Send_Main_Stream()
{
    int i = 0;
    BOOL b_have_connect = FALSE;
    int ret;
    GK_NET_FRAME_HEADER frame_header;
    void *p_data_buff = NULL;
    int data_len = 0;

    char *send_buff = (char*)malloc(MAX_SEND_BUFF_SIZE);
    if(NULL == send_buff)
    {
        PRINT_ERR("malloc failure\n");
        return 0;
    }


    //IÖ¡ÇëÇó
    //Ìí¼Ó¶ÁÖ¸Õë
    MEDIABUF_HANDLE handle = mediabuf_add_reader(0);
    if(handle)
        mediabuf_set_newest_frame(handle);

    char ch_frame_type = 0;   

    /*struct timeval tv_begin, tv_end;
      gettimeofday(&tv_begin, NULL);  
      int j = 0;*/
    while(g_dahua_protocol_run)
    { 
        b_have_connect = FALSE;
        for(i = 0; i < MAX_LINK_NUM; i++)
        {   
            if((NULL != g_socket_av_send[i]) && (-1 != g_socket_av_send[i]->n_flag))
            {
                b_have_connect = TRUE; 
            }
        }  
        if(FALSE == b_have_connect)
        {
            usleep(20000);
            continue;
        }
        while(g_dahua_protocol_run)
        {
            memset(&frame_header, 0, sizeof(frame_header));
            p_data_buff= NULL;
            ret = mediabuf_read_frame((MEDIABUF_HANDLE)handle, &p_data_buff, &data_len, &frame_header);
            if (ret <= 0)
                goto SEND_MAIN_STREAM_END;        
            break;    
        }

        //»ñÈ¡´ËÖ¡Êý¾ÝµÄÀàÐÍ
        if (frame_header.frame_type == GK_NET_FRAME_TYPE_A) 
        {
            ch_frame_type = AUDIO_TYPE;
            g_audio_sequence_main_stream++;
            g_audio_timestample_main_stream = frame_header.pts;

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
            data_len = alaw_len;
        }else if ( (frame_header.frame_type == GK_NET_FRAME_TYPE_I) || (frame_header.frame_type == GK_NET_FRAME_TYPE_P) ){
            if(frame_header.frame_type == GK_NET_FRAME_TYPE_I){
                ch_frame_type = FRAME_TYPE_I;
            }else{
                ch_frame_type = FRAME_TYPE_P;
            }
            g_video_sequence_main_stream++;
            g_video_timestample_main_stream = frame_header.pts;
        }else{
        }       

        pthread_mutex_lock(&main_second_stream_mutex); 
        ret = ZhiNuo_Package_Main_Stream(send_buff, p_data_buff, data_len, ch_frame_type, 
                (char)(((frame_header.video_reso >> 16) & 0xffff)/8), (char)((frame_header.video_reso & 0xffff)/8), frame_header.frame_rate);
        //(char)(1280 / 8), (char)(720 / 8)); //8ÏñËØµãÎª1µ¥Î»
        pthread_mutex_unlock(&main_second_stream_mutex); 
        if(ret < 0)
        {
            break;
        }
    }  
	SEND_MAIN_STREAM_END:
    free(send_buff); 
	mediabuf_del_reader(handle);
    return 0;     
}
/*************************************************************
 * º¯Êý½éÉÜ£ºµÇÂ¼nvr
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_LogOn(char* bufPost, DAHUA_SOCK_T *sock)
{
    int rcv_msg_len = 0;

    char tem_buf[DVRIP_HEAD_T_SIZE] = {0};
    memcpy(tem_buf, bufPost, DVRIP_HEAD_T_SIZE);
    DVRIP_HEAD_T *t_msg_head_rcv = (DVRIP_HEAD_T *)tem_buf;
    char * extra = bufPost + DVRIP_HEAD_T_SIZE;  

    PRINT_INFO("name:%s, pass:%s, cmd:%x, extralen:%d, extra:%s, clienttype:%d, locktype:%d, hl:%d, v:%d\n", 
            t_msg_head_rcv->dvrip.dvrip_p, &t_msg_head_rcv->dvrip.dvrip_p[8], t_msg_head_rcv->dvrip.cmd
            , t_msg_head_rcv->dvrip.dvrip_extlen, extra, t_msg_head_rcv->dvrip.dvrip_p[18], 
            t_msg_head_rcv->dvrip.dvrip_p[19], t_msg_head_rcv->dvrip.dvrip_hl, t_msg_head_rcv->dvrip.dvrip_v);

    rcv_msg_len = DVRIP_HEAD_T_SIZE + t_msg_head_rcv->dvrip.dvrip_extlen;
    char send_msg[BUFLEN] = {0};
    DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)send_msg;             
    ZERO_DVRIP_HEAD_T(t_msg_head)
        t_msg_head->dvrip.cmd = ACK_LOG_ON;   
    t_msg_head->c[1] = 0; //0£ºÃ»ÓÐ 1£º¾ßÓÐ¶à»­ÃæÔ¤ÀÀ¹¦ÄÜ 
    t_msg_head->c[2] = 0; //0: Ö§³Ö16Â· 1:Ö§³Ö32Â·
    t_msg_head->c[8] = 0;//1; //0: ±íÊ¾µÇÂ¼³É¹¦ 1: ±íÊ¾µÇÂ¼Ê§°Ü 3: ÓÃ»§ÒÑµÇÂ¼

    /* µÇÂ¼Ê§°Ü·µ»ØÂë  0:ÃÜÂë²»ÕýÈ· 1:ÕÊºÅ²»´æÔÚ 3:ÕÊºÅÒÑ¾­µÇÂ¼ 4:ÕÊºÅ±»Ëø¶¨ 
5:ÕÊºÅ±»ÁÐÈëºÚÃûµ¥ 6:×ÊÔ´²»×ã£¬ÏµÍ³Ã¦ 7:°æ±¾²»¶Ô£¬ÎÞ·¨µÇÂ½ */
    t_msg_head->c[9] = 0; //µÇÂ¼Ê§°Ü·µ»ØÂë       
    if(0 == t_msg_head->c[8])
    {
        //½«socket¼ÓÈëÊý×é¹ÜÀí£¬ÒÔ±ãÓÚ±¨¾¯Ê±£¬Öð¸öÖ÷¶¯·¢ËÍ±¨¾¯ÏûÏ¢
        int i = 0;
        BOOL b_connect_full = FALSE;

        if(TRUE == b_connect_full)
        {
            t_msg_head->c[8] = 1; //0: ±íÊ¾µÇÂ¼³É¹¦ 1: ±íÊ¾µÇÂ¼Ê§°Ü 3: ÓÃ»§ÒÑµÇÂ¼

            /* µÇÂ¼Ê§°Ü·µ»ØÂë  0:ÃÜÂë²»ÕýÈ· 1:ÕÊºÅ²»´æÔÚ 3:ÕÊºÅÒÑ¾­µÇÂ¼ 4:ÕÊºÅ±»Ëø¶¨ 
5:ÕÊºÅ±»ÁÐÈëºÚÃûµ¥ 6:×ÊÔ´²»×ã£¬ÏµÍ³Ã¦ 7:°æ±¾²»¶Ô£¬ÎÞ·¨µÇÂ½ */
            t_msg_head->c[9] = 6; //µÇÂ¼Ê§°Ü·µ»ØÂë        
        }
    }
    else
    {
        PRINT_ERR("usrname passwd wrong\n"); 
    }

    t_msg_head->c[10] = 1;  //Í¨µÀÊý    
    t_msg_head->c[11] = 9;   //ÊÓÆµ±àÂë·½Ê½ 8:MPEG4 9:H.264
    t_msg_head->c[12] = 51;  //Éè±¸ÀàÐÍ  51:IPCÀà²úÆ·
    if(CMD_ZHINUO_LOG_ON == t_msg_head_rcv->dvrip.cmd)
    {
        t_msg_head->l[4] = ZHINUO_NVR;    //Éè±¸·µ»ØµÄÎ¨Ò»±êÊ¶ºÅ£¬±êÖ¾¸ÃÁ¬½Ó
    }
    if(CMD_DAHUA_LOG_ON == t_msg_head_rcv->dvrip.cmd)
    {
        t_msg_head->l[4] = DAHUA_NVR;    //Éè±¸·µ»ØµÄÎ¨Ò»±êÊ¶ºÅ£¬±êÖ¾¸ÃÁ¬½Ó
    }    
    t_msg_head->c[28] = 0;  //ÊÓÆµÖÆÊ½£¬0: ±íÊ¾PALÖÆ     1: ±íÊ¾NTSCÖÆ

    t_msg_head->c[24] = 0x06;  
    t_msg_head->c[26] = 0xf9;  
    t_msg_head->c[29] = 0x01;      
    t_msg_head->c[30] = 0x64;  
    t_msg_head->c[31] = 0x02;     
    //t_msg_head->s[15] = 0x8101; //µÚ30¡¢31×Ö½Ú±£ÁôÒÔÏÂÖµ ´úÀíÍø¹Ø²úÆ·ºÅ0x8101, 0x8001, 0x8002, 0x8003,Õâ¸öÖµnvrÃ»ÓÃµ½Ëæ±ãÌî

    ZhiNuo_SockSend(sock, send_msg, DVRIP_HEAD_T_SIZE, 100);
    return rcv_msg_len;
}

/*************************************************************
 * º¯Êý½éÉÜ£º²éÑ¯¹¤×÷¼°±¨¾¯×´Ì¬
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 * ±¸×¢    :ÊÕµ½0ºÍ4µÄ²éÑ¯ÀàÐÍ
 *************************************************************/
int ZhiNuo_Search_Status(char* bufPost, DAHUA_SOCK_T *sock)
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
    t_msg_head->c[13] = 0xfb;

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

    ZhiNuo_SockSend(sock, send_msg, DVRIP_HEAD_T_SIZE, 100);

    return rcv_msg_len;
}

/*************************************************************
 * º¯Êý½éÉÜ£º²éÑ¯Éè±¸ÊÂ¼þ
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 * ±¸×¢    :ÊÕµ½²Ù×÷ÀàÐÍÎª2:¶©ÔÄ±¨¾¯×´Ì¬ ÊÂ¼þÀàÐÍÎª1:$Íâ²¿$±¨¾¯2:$¶¯Ì¬¼ì²â$±¨¾¯3:$ÊÓÆµ¶ªÊ§$±¨¾¯4:$ÕÚµ²¼ì²â$±¨¾¯
 *************************************************************/
int ZhiNuo_Dev_Event(char* bufPost, DAHUA_SOCK_T * sock)
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
int ZhiNuo_Media_Request(char* bufPost, DAHUA_SOCK_T * sock)
{
    int rcv_msg_len = 0;
    char tem_buf[DVRIP_HEAD_T_SIZE] = {0};
    memcpy(tem_buf, bufPost, DVRIP_HEAD_T_SIZE);
    DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)tem_buf;
    char stream_type = *(char*)(bufPost + DVRIP_HEAD_T_SIZE);

    PRINT_INFO("hl:%d, v:%d,stream:%d, vextralen:%d, addr:%p\n", t_msg_head->dvrip.dvrip_hl,
            t_msg_head->dvrip.dvrip_v, stream_type, t_msg_head->dvrip.dvrip_extlen, &(t_msg_head->dvrip.dvrip_extlen));


    rcv_msg_len = DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen;
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

    return rcv_msg_len;
}

/*************************************************************
 * º¯Êý½éÉÜ£º²éÑ¯Éè±¸Ã½ÌåÄÜÁ¦ÐÅÏ¢
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_Search_Media_Capacity(char* bufPost, DAHUA_SOCK_T *sock)
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
int ZhiNuo_Search_Config(char* bufPost, DAHUA_SOCK_T *sock)
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
    PRINT_INFO("Search_type:%d£¬ child_type:%d\n", t_msg_head->c[16], t_msg_head->c[24]);

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
                PRINT_INFO("search_type:%d, »ñµÃÑÕÉ«ÅäÖÃ\n", search_type);
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
int ZhiNuo_Search_Channel_Name(char* bufPost, DAHUA_SOCK_T *sock)
{
    int ret = 0, i, offset;
    int rcv_msg_len = 0;
    int channel_name_type = 0;
    char tem_buf[DVRIP_HEAD_T_SIZE] = {0};
    char strName[CHANNEL_NAME_LEN];	//!Ò»¸öÍ¨µÀµÄÍ¨µÀÃû³Æ    
    GK_NET_CHANNEL_INFO channelInfo;
    char send_msg[BUFLEN] = {0};               

        gadi_sys_memcpy(tem_buf, bufPost, DVRIP_HEAD_T_SIZE);
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
int ZhiNuo_Search_System_Info(char* bufPost, DAHUA_SOCK_T *sock)
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
                PRINT_INFO("search_type:%d, ²éÑ¯ÉãÏñÍ·ÊôÐÔÐÅÏ¢\n", search_type);
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
                PRINT_INFO("search_type:%d, ²éÑ¯Éè±¸¹¦ÄÜÁÐ±íftp,alarm...\n", search_type);
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
 * º¯Êý½éÉÜ£º·¢ËÍ¸±ÂëÁ÷Êý¾ÝÏß³Ì
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
void * ZhiNuo_Send_Main_Stream_Pthread(void *n_sock)
{
    prctl(PR_SET_NAME, (unsigned long)"ZhiNuo_Send_Main_Stream_Pthread", 0,0,0);  
    //PRINT_INFO("ZhiNuo_Send_Main_Stream_Pthread pid:%d tid:%d\n", getpid(), gettid());

    ZhiNuo_Send_Main_Stream();
    PRINT_INFO("ZhiNuo_Send_Main_Stream_Pthread end !\n");

    return NULL;
}


/*************************************************************
 * º¯Êý½éÉÜ£º·¢ËÍÖ÷ÂëÁ÷Êý¾ÝÏß³Ì
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
void * ZhiNuo_Send_Second_Stream_Pthread(void *n_sock)
{
    prctl(PR_SET_NAME, (unsigned long)"ZhiNuo_Send_Second_Stream_Pthread", 0,0,0);  
    //PRINT_INFO("ZhiNuo_Send_Second_Stream_Pthread pid:%d tid:%d\n", getpid(), gettid());

    ZhiNuo_Send_Second_Stream();

    PRINT_INFO("ZhiNuo_Send_Second_Stream_Pthread end !\n");
    return NULL;    
}


/*************************************************************
 * º¯Êý½éÉÜ£ºÇëÇó½¨Á¢Á¬½Ó¹ØÏµ
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_Connect_Request(char* bufPost, DAHUA_SOCK_T* sock)
{
    int rcv_msg_len = 0;
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

    rcv_msg_len = DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen;
    char send_msg[BUFLEN] = {0};
    t_msg_head = (DVRIP_HEAD_T *)send_msg;                
    ZERO_DVRIP_HEAD_T(t_msg_head)
        t_msg_head->dvrip.cmd = ACK_CONNECT_REQUEST;   
    t_msg_head->l[2] = n_flag; //µÇÂ½µÄÎ¨Ò»±êÊ¶ºÅ,Ò²¾ÍÊÇ0xb0·µ»ØµÄÄÇ¸ö
    t_msg_head->c[12] = ch_request_type; //ÇëÇóÀàÐÍ
    t_msg_head->c[13] = ch_channel_num; //ÇëÇóÍ¨µÀºÅ

    t_msg_head->c[14] = 0; //·µ»ØÂë 0:Á¬½ÓÓ³Éä³É¹¦ 1:Ê§°Ü 2:×ÓÁ´½ÓÒÑ½¨Á¢

    //bruce
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
    
    int i = 0;
    BOOL b_connect_full = TRUE;
    for(i = 0; i < MAX_LINK_NUM; i++)
    {   
        if((NULL != g_socket_av_send[i]) && (sock->fd == g_socket_av_send[i]->fd))
        {
            t_msg_head->c[14] = 2; //2:×ÓÁ´½ÓÒÑ½¨Á¢
            ZhiNuo_SockSend(sock, send_msg, DVRIP_HEAD_T_SIZE, 100);
            return rcv_msg_len;            
        }
    }

    for(i = 0; i < MAX_LINK_NUM; i++)
    {   
        if(NULL == g_socket_av_send[i])
        {
            g_socket_av_send[i] = sock;
            g_socket_av_send[i]->n_flag = n_flag;
            PRINT_ERR("connect sock:%d\n", sock->fd);
            b_connect_full = FALSE;
            break;
        }
    }

    if(FALSE == b_connect_full)
    {
        ZhiNuo_SockSend(sock, send_msg, DVRIP_HEAD_T_SIZE, 100);
    }
    else
    {
        sock->n_flag = -1;
        return rcv_msg_len;
    }

    return rcv_msg_len;
}


/*************************************************************
 * º¯Êý½éÉÜ£ºÇ¿ÖÆIÖ¡ÇëÇó
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_I_Frame_Request(char* bufPost, DAHUA_SOCK_T * sock)
{
    int rcv_msg_len = 0;
    char tem_buf[DVRIP_HEAD_T_SIZE] = {0};
    memcpy(tem_buf, bufPost, DVRIP_HEAD_T_SIZE);
    DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)tem_buf;    

    PRINT_INFO("hl:%d, v:%d, vextralen:%d\n", t_msg_head->dvrip.dvrip_hl, 
            t_msg_head->dvrip.dvrip_v, t_msg_head->dvrip.dvrip_extlen);

    //ÂëÁ÷Í¨µÀºÅ£¨0~64£© 0-Ö÷ÂëÁ÷ 1-¸¨ÂëÁ÷1  2-¸¨ÂëÁ÷2  3 -¸¨ÂëÁ÷3
    PRINT_INFO("requeset_type:%d, channel_num:%d\n", t_msg_head->c[9], t_msg_head->c[8]);

    if(0 == t_msg_head->c[9])
    {
        //Ö÷ÂëÁ÷ÇëÇóIÖ¡
    }    
    if(1 == t_msg_head->c[9])
    {
        //´ÎÂëÁ÷ÇëÇóIÖ¡
    }

    rcv_msg_len = DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen;
    char send_msg[BUFLEN] = {0};
    t_msg_head = (DVRIP_HEAD_T *)send_msg;                
    ZERO_DVRIP_HEAD_T(t_msg_head)
        t_msg_head->dvrip.cmd = ACK_I_FRAME_REQUEST;   

    t_msg_head->c[8] = 0; //·µ»ØÂë ¨D¨D 0-³É¹¦£¬1-Ê§°Ü
    ZhiNuo_SockSend(sock, send_msg, DVRIP_HEAD_T_SIZE, 100);

    return rcv_msg_len;
}

/*************************************************************
 * º¯Êý½éÉÜ£ºÉè±¸²Ù×÷
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_DEV_Control(char* bufPost, DAHUA_SOCK_T *sock)
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
int ZhiNuo_Set_Config(char* bufPost, DAHUA_SOCK_T *sock)
{
    int rcv_msg_len = 0;
    char ch_param_type = 0;
    char ch_child_type = 0;
    char ch_config_edition = 0;
    char ch_param_effect_flag = 0;
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
    ch_param_effect_flag = t_msg_head->c[20];
    ch_child_type_limit = t_msg_head->c[24];

    //´òÓ¡ËÑË÷ÀàÐÍ 16:±àÂëÄÜÁ¦ÐÅÏ¢ 127:Ë«ÂëÁ÷Í¼ÏñÅäÖÃ 126:ÑÕÉ«ÅäÖÃ 124:»ñµÃÊÂ¼þÅäÖÃ 
    PRINT_INFO("ch_param_type:%d, ch_child_type:%d, ch_config_edition:%d, ch_param_effect_flag:%d, ch_child_type_limit:%d\n",
            t_msg_head->c[16], t_msg_head->c[17], t_msg_head->c[18], t_msg_head->c[20], t_msg_head->c[24]); 

    rcv_msg_len = DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen;

    char send_msg[BUFLEN] = {0};
    t_msg_head = (DVRIP_HEAD_T *)send_msg;                
    ZERO_DVRIP_HEAD_T(t_msg_head)
        t_msg_head->dvrip.cmd = ACK_SET_CONFIG;      
    
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
        PRINT_INFO("search_type:%d, ÉèÖÃÉãÏñÍ·²ÎÊýÅäÖÃ\n", ch_param_type);
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
int ZhiNuo_Time_Manage(char* bufPost, DAHUA_SOCK_T *sock)
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
    PRINT_INFO("ch_cmd_code:%d, year:%d, month:%d, day:%d, hour:%d, min:%d, sec:%d\n", t_msg_head->c[8], t_msg_head->c[16], t_msg_head->c[17],
            t_msg_head->c[18], t_msg_head->c[19], t_msg_head->c[20], t_msg_head->c[21]);  
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
int ZhiNuo_Set_Channel_Name(char* bufPost, DAHUA_SOCK_T *sock)
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
int ZhiNuo_PTZ_Control(char* bufPost, DAHUA_SOCK_T *sock)
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

/*************************************************************
 * º¯Êý½éÉÜ£º»Ø·ÅÏÂÔØ
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_Unkown_Cmd(char* bufPost, DAHUA_SOCK_T *sock)
{
    int rcv_msg_len = 0;
    char tem_buf[DVRIP_HEAD_T_SIZE] = {0};
    memcpy(tem_buf, bufPost, DVRIP_HEAD_T_SIZE);
    DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)tem_buf;  
    char ch_unkown_num = t_msg_head->c[8];
    char ch_id = t_msg_head->c[10];
    unsigned long n_session = t_msg_head->l[6];

    PRINT_INFO("hl:%d, v:%d, vextralen:%d\n", t_msg_head->dvrip.dvrip_hl, 
            t_msg_head->dvrip.dvrip_v, t_msg_head->dvrip.dvrip_extlen);

    //
    PRINT_INFO("ch_unkown_num:%d, id:%d, n_session:%d\n", t_msg_head->c[8], 
            t_msg_head->c[10], t_msg_head->l[6]);

    rcv_msg_len = DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen;
    char send_msg[BUFLEN] = {0};
    t_msg_head = (DVRIP_HEAD_T *)send_msg;                
    ZERO_DVRIP_HEAD_T(t_msg_head)
        t_msg_head->dvrip.cmd = ACK_UNKOWN;  

    t_msg_head->c[8] = ch_unkown_num; 
    t_msg_head->c[10] = ch_id; 

    t_msg_head->l[6] = n_session;

    sprintf(send_msg + DVRIP_HEAD_T_SIZE, "{ \"id\" : %d, \"params\" : null, \"result\" : 1129658112, \"session\" : %ld }\n", ch_id, n_session);
    t_msg_head->dvrip.dvrip_extlen = strlen(send_msg + DVRIP_HEAD_T_SIZE);
    t_msg_head->l[4] = t_msg_head->dvrip.dvrip_extlen;    
    ZhiNuo_SockSend(sock, send_msg, DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen, 100);

    return rcv_msg_len;
}


/*************************************************************
 * º¯Êý½éÉÜ£º´¦ÀíÃüÁîµÄÖ÷Ìå
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_CMDDealBody(char* bufPost, DAHUA_SOCK_T *sock, int datalen)
{
    int n_deal_len = 0;
    int n_offset = 0;
    char tem_buf[DVRIP_HEAD_T_SIZE] = {0};

    //ÓÐ¿ÉÄÜnvr¼¸ÌõÃüÁîÁ¬×ÅÒ»Æð·¢¹ýÀ´£¬ËùÒÔÒª²ð·ÖÒ»¸öÃüÁîÒ»¸öÃüÁî´¦Àí
    while(1)
    {
        memcpy(tem_buf, bufPost + n_offset, DVRIP_HEAD_T_SIZE);
        DVRIP_HEAD_T *t_msg_head = (DVRIP_HEAD_T *)tem_buf; 

        //PRINT_ERR("sock:%d, rcv cmd:%x, datalen:%d, address:%p, have_deal_len:%d, extralen:%d\n", sock,
        //            t_msg_head->dvrip.cmd, datalen, bufPost + n_offset, n_deal_len, t_msg_head->dvrip.dvrip_extlen);    
        PRINT_INFO("t_msg_head->dvrip.cmd @@@>>>>>>>:%x\n", t_msg_head->dvrip.cmd);
        switch(t_msg_head->dvrip.cmd)
        {
            case CMD_DAHUA_LOG_ON:
                PRINT_INFO("dahua log\n");
                n_deal_len += ZhiNuo_LogOn(bufPost + n_offset, sock); //µÇÂ¼
                break;
            case CMD_ZHINUO_LOG_ON:
                PRINT_INFO("dahua log\n");
                n_deal_len += ZhiNuo_LogOn(bufPost + n_offset, sock); //µÇÂ¼
                break;                
            case CMD_STATUS_SEARCH:
                PRINT_INFO("STATUS_SEARCH \n");
                n_deal_len += ZhiNuo_Search_Status(bufPost + n_offset, sock); //²éÑ¯¹¤×÷¼°±¨¾¯×´Ì¬ 1
                break;     
            case CMD_DEV_EVENT_SEARCH:
                PRINT_INFO("EVENT_SEARCH \n");
                n_deal_len += ZhiNuo_Dev_Event(bufPost + n_offset, sock); //²éÑ¯Éè±¸ÊÂ¼þ 1
                break;  
            case CMD_MEDIA_REQUEST:
                PRINT_INFO("MEDIA_REQUEST \n");
                n_deal_len += ZhiNuo_Media_Request(bufPost + n_offset, sock); //Ã½ÌåÊý¾ÝÇëÇó
                break;  
            case CMD_MEDIA_CAPACITY_SEARCH:
                PRINT_INFO("MEDIA_CAPACITY_SEARCH \n");
                n_deal_len += ZhiNuo_Search_Media_Capacity(bufPost + n_offset, sock); //²éÑ¯Éè±¸Ã½ÌåÄÜÁ¦ÐÅÏ¢ ok
                break;                 
            case CMD_CONFIG_SEARCH:
                PRINT_INFO("CONFIG_SEARCH \n");
                n_deal_len += ZhiNuo_Search_Config(bufPost + n_offset, sock); //²éÑ¯ÅäÖÃ²ÎÊý
                break;     
            case CMD_CHANNEL_NAME_SEARCH:
                PRINT_INFO("CHANNEL_NAME_SEARCH \n");
                n_deal_len += ZhiNuo_Search_Channel_Name(bufPost + n_offset, sock); //²éÑ¯Í¨µÀÃû³Æ
                break;   
            case CMD_SYSTEM_INFO_SEARCH:
                PRINT_INFO("SYSTEM_INFO_SEARCH \n");
                n_deal_len += ZhiNuo_Search_System_Info(bufPost + n_offset, sock); //²éÑ¯ÏµÍ³ÐÅÏ¢
                break;  
            case CMD_CONNECT_REQUEST:
                PRINT_INFO("CONNECT_REQUEST \n");
                n_deal_len += ZhiNuo_Connect_Request(bufPost + n_offset, sock); //ÇëÇó½¨Á¢Á¬½Ó¹ØÏµ
                break;    
            case CMD_I_FRAME_REQUEST:
                PRINT_INFO("I_FRAME_REQUEST \n");
                n_deal_len += ZhiNuo_I_Frame_Request(bufPost + n_offset, sock); //Ç¿ÖÆIÖ¡
                break;  
            case CMD_DEV_CONTROL:
                PRINT_INFO("DEV_CONTROL\n");
                n_deal_len += ZhiNuo_DEV_Control(bufPost + n_offset, sock); //Éè±¸¿ØÖÆ
                break;   
            case CMD_SET_CONFIG:
                PRINT_INFO("SET_CONFIG datalen:%d\n", datalen);
                n_deal_len += ZhiNuo_Set_Config(bufPost + n_offset, sock); //ÐÞ¸ÄÅäÖÃ²ÎÊý
                break;    
            case CMD_TIME_MANAGE:
                PRINT_INFO("TIME_MANAGE\n");
                n_deal_len += ZhiNuo_Time_Manage(bufPost + n_offset, sock); //Ê±¼ä¹ÜÀí
                break;        
            case CMD_SET_CHANNEL_NAME:
                PRINT_INFO("SET_CHANNEL_NAME\n");
                n_deal_len += ZhiNuo_Set_Channel_Name(bufPost + n_offset, sock); //ÐÞ¸ÄÍ¨µÀÃû³Æ
                break;                     
            case CMD_PTZ_CONTROL:
                PRINT_INFO("PTZ_CONTROL\n");
                n_deal_len += ZhiNuo_PTZ_Control(bufPost + n_offset, sock); //ÔÆÌ¨¿ØÖÆ
                break;     
            case CMD_TALK_REQUEST:
                PRINT_INFO("TALK_REQUEST \n");
                //n_deal_len += ZhiNuo_Talk_Request(bufPost + n_offset, sock); //ÇëÇó¶Ô½²
                break;   
            case CMD_TALK:
                PRINT_INFO("TALK \n");
                //n_deal_len += ZhiNuo_Talk(bufPost + n_offset, sock); //¿ªÊ¼¶Ô½²
                break;    
            case CMD_TALK_CONTROL:
                PRINT_INFO("TALK_CONTROL\n");
                //n_deal_len += ZhiNuo_Talk_Control(bufPost + n_offset, sock); //¶Ô½²¿ØÖÆ
                break;    
            case CMD_RECORD_SEARCH:
                PRINT_INFO("RECORD_SEARCH\n");
                //n_deal_len += ZhiNuo_Record_Search(bufPost + n_offset, sock); //Â¼ÏñËÑË÷
                break;    
            case CMD_RECORD_PLAYBACK:
                PRINT_INFO("RECORD_PLAYBACK\n");
                //n_deal_len += ZhiNuo_Record_Playback(bufPost + n_offset, sock); //Â¼Ïñ»Ø·Å
                break;                  
            case CMD_RECORD_DOWNLOAD:
                PRINT_INFO("RECORD_DOWNLOAD\n");
                //n_deal_len += ZhiNuo_Record_Download(bufPost + n_offset, sock); //Â¼ÏñÏÂÔØ
                break;    
            case CMD_UNKOWN:
                PRINT_ERR("unkown protocol [0x%02x] not support\n", t_msg_head->dvrip.cmd);
                n_deal_len += ZhiNuo_Unkown_Cmd(bufPost + n_offset, sock); //²»ÖªÃûµÄÃüÁî
                break;                 
            default:                
                PRINT_ERR("cmd not support 0x%x datalen:%d\n", t_msg_head->dvrip.cmd, datalen); 

                n_deal_len += DVRIP_HEAD_T_SIZE + t_msg_head->dvrip.dvrip_extlen;
                break;
        }

        //ÁÙÊ±´òÓ¡±¨ÎÄÍ·µÄÃ¿¸ö×Ö½ÚÄÚÈÝ
#if DEBUG
        int i = 0;
        char *extra_tmp = (char *)t_msg_head;
        printf("datalen:%d\n", datalen);
        for(i = 0; i < DVRIP_HEAD_T_SIZE; i++)
        {
            //printf("bufPost:%d:%d\n", i, *extra_tmp);
            printf("%x ", (char)*extra_tmp);
            extra_tmp = extra_tmp + 1;
        } 
        printf("\n");
#endif

        if((n_deal_len >= datalen) || (n_deal_len < 0))
        {
            //±¨ÎÄ´¦ÀíÍê±ÏÍË³ö
            break;
        }
        n_offset = n_deal_len;
    }
    return 0;
}


/*************************************************************
 * º¯Êý½éÉÜ£º»á»°Ïß³Ì
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
void *ZhiNuo_Session_Thread(void *param)
{
    int n_rcv_len = 0;
    int ret = 0, i;
    //int n_socket = *((int*)param);
    DAHUA_SOCK_T dahua_sock;
    char arrstr_rcv_buff[BUFLEN] = {0};  

    prctl(PR_SET_NAME, (unsigned long)"ZhiNuo_session_Thread", 0,0,0); 
    //PRINT_INFO("ZhiNuo_session_Thread pid:%d tid:%d\n", getpid(), gettid());
	dahua_sock.fd = *((int*)param);
	dahua_sock.n_flag = 1;
	pthread_mutex_init(&dahua_sock.send_mutex,NULL);
	*((int*)param) = -1;
    //½¨Á¢Á¬½ÓºóÒ»Ö±Ñ­»·½ÓÊÕÃüÁî
    while(1)
    {
        //½ÓÊÕÃüÁî
        memset(arrstr_rcv_buff, 0, BUFLEN);
        n_rcv_len = ZhiNuo_SockRecv(dahua_sock.fd, arrstr_rcv_buff, sizeof(arrstr_rcv_buff), 100);
        if(0 == g_dahua_protocol_run)
        {
            break;
        }
        if (n_rcv_len <= 0) 
        {
            PRINT_ERR("ZhiNuo_SockRecv err fd:%d\n", dahua_sock.fd);
            break;
        }

        //´¦ÀíÃüÁî
        ret = ZhiNuo_CMDDealBody(arrstr_rcv_buff, &dahua_sock, n_rcv_len);
        if(ret < 0) //Èç¹ûÊÇÁ¬½ÓÇëÇóÔòÕâ¸öÏß³Ì´¦ÀíÍê¾Í¿ÉÒÔ¹Ø±ÕÁË
        {
           break;
        }
        if(dahua_sock.n_flag == -1)
            break;
    }  

    //´ÓÊý×éÖÐ½«¶Ï¿ªµÄsocketÉèÖÃÎª-1
    for(i = 0; i < MAX_LINK_NUM; i++)
    {   
        if((NULL != g_socket_av_send[i]) && (dahua_sock.fd == g_socket_av_send[i]->fd)){
            g_socket_av_send[i] = NULL;
        }
    }
    sleep(6);//wait send timeout
    close(dahua_sock.fd);
    pthread_mutex_destroy(&dahua_sock.send_mutex);

    return NULL;
}


/*************************************************************
 * º¯Êý½éÉÜ£ºÆô¶¯Ö÷×ÓÂëÁ÷ÊÓÆµ·¢ËÍ£¬ÑÕÉ«±£´æµÄÏß³Ì£¬×¢²á±¨¾¯»Øµ÷¡£
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
int ZhiNuo_Function_Thread()
{

    //´´½¨·¢ËÍÖ÷ÂëÁ÷µÄÏß³Ì
    pthread_t main_thread_id;
    pthread_attr_t main_attr;        
    pthread_attr_init(&main_attr);  
    pthread_attr_setdetachstate(&main_attr, PTHREAD_CREATE_DETACHED); 
    if(0 != pthread_create(&main_thread_id, &main_attr, ZhiNuo_Send_Main_Stream_Pthread, NULL))
    {
        PRINT_ERR("erro:creat ZhiNuo_Send_Main_Stream_Pthread fail\n");
        g_dahua_protocol_run = 0;
        return -1;
    } 

    //´´½¨·¢ËÍ×ÓÂëÁ÷µÄÏß³Ì
    pthread_t second_thread_id;
    pthread_attr_t second_attr;        
    pthread_attr_init(&second_attr);  
    pthread_attr_setdetachstate(&second_attr, PTHREAD_CREATE_DETACHED); 
    if(0 != pthread_create(&second_thread_id, &second_attr, ZhiNuo_Send_Second_Stream_Pthread, NULL))
    {
        PRINT_ERR("erro:creat ZhiNuo_Send_Second_Stream_Pthread fail\n");
        g_dahua_protocol_run = 0;
        return -1;
    }

    return 0;
}


/*************************************************************
 * º¯Êý½éÉÜ£ºµÇÂ¼nvr£¬È»ºó½éÉÜ´Ónvr·¢ËÍ¹ýÀ´µÄÃüÁî
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
void * ZhiNuo_Main_Thread(void *param)
{
    int n_len;
    int i = 0;
    int n_function_flag = 0;
    struct sockaddr_in t_serv_addr, t_clnt_addr;
    int main_sock = 0;
    int n_peer[MAX_LINK_NUM], sock_fd;

    g_dahua_protocol_run = 1;

    prctl(PR_SET_NAME, (unsigned long)"ZhiNuo_Main_Thread", 0,0,0);   

    //³õÊ¼»¯»á»°Á¬½ÓsocketÊý×é
    for(i = 0; i < MAX_LINK_NUM; i++)
    {
        g_socket_av_send[i] = NULL;
    }

    //³õÊ¼»¯Ã½ÌåÁ¬½ÓsocketÊý×é
    for(i = 0; i < MAX_LINK_NUM; i ++)
    {
        n_peer[i] = -1;
    }

    //´´½¨¼àÌý»á»°µÄsocket
    main_sock = socket(AF_INET, SOCK_STREAM, 0);
    if (main_sock < 0) 
    {
        PRINT_ERR("socket error%s\n", strerror(errno));
        goto end;
    }

    /*  ÅäÖÃ¼àÌý»á»°µÄsocket */
    t_serv_addr.sin_family = AF_INET;
    t_serv_addr.sin_port = htons(37777);
    t_serv_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(t_serv_addr.sin_zero), 8);

    /* °ó¶¨¼àÌý»á»°µÄsocket */
    if (bind(main_sock, (struct sockaddr *)&t_serv_addr, sizeof(struct sockaddr)) < 0) 
    {
        PRINT_ERR("bind err%s\n", strerror(errno));
        goto end;
    }

    /* Ö÷¶¯socket×ªÎª±»¶¯¼àÌýsocket */
    if (listen(main_sock, 1) < 0) 
    {
        PRINT_ERR("listen error%s\n", strerror(errno));
        goto end;
    }

    /* Ñ­»·¼àÌý£¬µÈ´ýnvrÖ÷¶¯Á¬½Ó */
    n_len = sizeof(t_clnt_addr);
    while (1) 
    {
        PRINT_INFO("begin to accept\n");
        sock_fd = accept(main_sock, (struct sockaddr *)&t_clnt_addr,(socklen_t *)&n_len);/* ½ÓÊÕÁ¬½Ó */
        if(0 == g_dahua_protocol_run)
        {
            break;
        }
        if (sock_fd < 0) 
        {
            PRINT_ERR("Server: accept failed%s\n", strerror(errno));
            continue;
        }

        if(0 == n_function_flag)
        {
            ZhiNuo_Function_Thread();
            n_function_flag = 1;
        }
        
        PRINT_INFO("Entry ZhiNuo_Session_Thread £¬after to accept:socket:%d============remote ip:%s,remote port:%d\n", 
                sock_fd, (char *)inet_ntoa(t_clnt_addr.sin_addr), t_clnt_addr.sin_port);
        for(i = 0; i < MAX_LINK_NUM; i ++)
        {
            if(n_peer[i] < 0){
                n_peer[i] = sock_fd;
                break;
            }
        }
        if(i == MAX_LINK_NUM)
        {
            PRINT_ERR("[%d]close accept socket:%d\n", i, sock_fd);
            close(sock_fd);
            continue;
        }
        //´´½¨»á»°Ïß³Ì£¬Ã¿ÓÐÒ»¸önvrÁ¬½Ó¾Í´´½¨Ò»¸ö»á»°Ïß³Ì
        pthread_t main_thread_id;
        pthread_attr_t attr;        
        pthread_attr_init(&attr);  
        pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); 
        if(0 != pthread_create(&main_thread_id, &attr, ZhiNuo_Session_Thread, (void *)&n_peer[i]))
        {
            PRINT_ERR("erro:creat ZhiNuo_Session_Thread fail\n");
            break;
        } 
    }
    PRINT_INFO("ZhiNuo_Main_Thread end !\n");

end:
    close(main_sock); 
    return NULL;
}

/*************************************************************
 * º¯Êý½éÉÜ£º´¦ÀínvrµÄ·¢¹ýÀ´µÄËÑË÷Éè±¸¹ã²¥°ü
 * ÊäÈë²ÎÊý£º
 * Êä³ö²ÎÊý£º
 * ·µ»ØÖµ  £º
 *************************************************************/
void * Dahua_Broadcast_Thread(void *param)
{
    prctl(PR_SET_NAME, (unsigned long)"Dahua_Broadcast_Thread", 0,0,0); 
    int g_dahua_sock = 0;

    if((g_dahua_sock = net_create_broadcast_socket(DAHUA_RCV_PORT)) < 0)
    {
        PRINT_ERR("create dahua broadcast fail\n");
        return NULL;
    }
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
        timeout.tv_sec = 10;  //³¬Ê±Ê±¼äÎª10Ãë
        timeout.tv_usec = 0;
        FD_ZERO(&readfd);
        FD_SET(g_dahua_sock, &readfd);
        ret = select(g_dahua_sock + 1, &readfd, NULL, NULL, &timeout);
        if(0 == g_dahua_protocol_run)
        {
            break;
        }

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
                        dahua_ack_search(g_dahua_sock);
                        break;
                    case CMD_DAHUA_LOG_ON:
                        PRINT_INFO("name:%s, pass:%s, cmd:%x, extralen:%d, extra:%s, clienttype:%d, locktype:%d, hl:%d, v:%d\n", 
                                t_msg_head->dvrip.dvrip_p, &t_msg_head->dvrip.dvrip_p[8], t_msg_head->dvrip.cmd
                                , t_msg_head->dvrip.dvrip_extlen, extra, t_msg_head->dvrip.dvrip_p[18], 
                                t_msg_head->dvrip.dvrip_p[19], t_msg_head->dvrip.dvrip_hl, t_msg_head->dvrip.dvrip_v);
                        dahua_ack_search_log(g_dahua_sock, smsg);
                        break;
                    case 0xc1:
                        dahua_ack_change_net_attr(g_dahua_sock, smsg);
                        break;
                    default:
                        PRINT_ERR("Dahua_Broadcast unknown cmd[%x] not support\n", t_msg_head->dvrip.cmd);
                        break;
                }
            }
        }
    }  

    close(g_dahua_sock);
    PRINT_INFO("DaHua_Broadcast_Thread end !\n");
    return NULL;  
}  


int dahua_protocol_start()
{
    pthread_t main_thread_id;
    pthread_t dahua_broadcast_thread_id;    
    pthread_attr_t attr;
    pthread_attr_t dahua_attr_broadcast;    

    //´´½¨Ö÷Ïß³Ì
    pthread_attr_init(&attr);  
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED); 
    if(0 != pthread_create(&main_thread_id, &attr, ZhiNuo_Main_Thread, NULL))
    {
        PRINT_ERR("erro:creat dahua_main_thread fail\n");
        return 0;
    }
    //´´½¨Éè±¸ËÑË÷´¦ÀíÏß³Ì£¬´¦Àí¹ã²¥°ü
    pthread_attr_init(&dahua_attr_broadcast);  
    pthread_attr_setdetachstate(&dahua_attr_broadcast, PTHREAD_CREATE_DETACHED); 
    if(0 != pthread_create(&dahua_broadcast_thread_id, &dahua_attr_broadcast, Dahua_Broadcast_Thread, NULL))
    {
        PRINT_ERR("erro:creat dahua_main_thread fail\n");
        return 0;
    } 

    return 0;
}

int dahua_protocol_stop()
{
    int ret = 0;
    int sock_send = -1;

    g_dahua_protocol_run = 0; 
    sock_send = socket(AF_INET, SOCK_DGRAM, 0);/*½¨Á¢Êý¾Ý±¨Ì×½Ó×Ö*/
    if(sock_send < 0)
    {
        PRINT_ERR(" socket(AF_INET, SOCK_DGRAM, 0) fail\n");  
        return 0;
    }

    //¹Ø±ÕÖÇÅµºÍ´ó»ªµÄÉè±¸ËÑË÷Ïß³Ì
    const int opt = 1; 
    if(setsockopt(sock_send, SOL_SOCKET, SO_BROADCAST, (char *)&opt, sizeof(opt)) == -1)
    {  
        PRINT_ERR("set sock_send error...\n");  
    }     

    struct sockaddr_in udpaddrto;  
    bzero(&udpaddrto, sizeof(struct sockaddr_in));  
    udpaddrto.sin_family = AF_INET;  
    udpaddrto.sin_addr.s_addr = inet_addr("255.255.255.255"); //htonl(INADDR_BROADCAST);  //
    udpaddrto.sin_port = htons(DAHUA_SEND_PORT);      

    //´Ó¹ã²¥µØÖ··¢ËÍÏûÏ¢  
    char send_msg[] = "shutdown";
    int msg_len = strlen(send_msg);
    ret = sendto(sock_send, send_msg, msg_len, 0, (struct sockaddr*)&udpaddrto, sizeof(udpaddrto));  
    if(ret <= 0)  
    {  
        PRINT_ERR("send error....:%d\n", ret);  
    }     
    usleep(200000);
    close(sock_send);
    PRINT_INFO("lib_dahua_close !\n");
    return 0;
}

