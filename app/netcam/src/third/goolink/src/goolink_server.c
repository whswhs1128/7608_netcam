/*!
*****************************************************************************
** \file        goolink/goolink_server.c
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
#include <netdb.h>
   
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

#include "goolink_apiv2.h"
#include "goolink_access_export.h"
#include "goolink_callback.h"
#include "goolink_cc_switch.h"
#include "LT_WifiConfigure.h"
#include "audioRecorder.h"
#include "avi_rec.h"
#include "sdk_cfg.h"
#include "cfg_p2p.h"
#include "eventalarm.h"


//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************

#define SR_RED          "\033[0;32;31m"
#define SR_GREEN        "\033[0;32;32m"
#define SR_BLUE         "\033[0;32;34m"
#define SR_YELLOW       "\033[1;33m"
#define SR_WHITE        "\033[1;37m"

#define GOOLINK_SERVER_DEBUG
#ifdef GOOLINK_SERVER_DEBUG
#define goolink_printk(S,...)      printf("\n\n%s[%s-%d]"S"\033[0;0m\n",SR_GREEN,__func__,__LINE__, ##__VA_ARGS__)
#define goolink_error(S,...)       printf("\n\n%s[%s-%d]"S"\033[0;0m\n",SR_RED,__func__,__LINE__, ##__VA_ARGS__)
#define goolink_warning(S,...)     printf("\n\n%s[%s-%d]"S"\033[0;0m\n",SR_YELLOW,__func__,__LINE__, ##__VA_ARGS__)
#else
#define goolink_printk(S,...)
#define goolink_error(S,...)       printf("\n\n%s[%s-%d]"S"\033[0;0m\n",SR_RED,__func__,__LINE__, ##__VA_ARGS__)
#define goolink_warning(S,...)
#endif

#define USE_CUSTOM_DEVICE_INFO

#define VERSION_FILED_MAX_SIZE_TOTAL (20)
#define VERSION_FILED_MAX_SIZE (20 - 1) //glnk那边规定的拼接字段长度, 超过就会segmentfault

//华创服务器升级包的版型名称在此确定
#define LABEL_APP      "NETDVR"
#define LABEL_SOL      "IPC"
#define LABEL_SOL_CDS  "CDSIPC" //从浪涛服务器上迁移过来的老设备
#define LABEL_HARDWARE_FH56W16F     "FH56W16F"     //FuHan的摇头机&一体机, 20181018



#define GOOLINK_MAX_CLIENT_NUMBER 4  //client max number
#define BLUE_LED_GPIO 13


/*  wifi connect status  */
#define RTWIFI_STATUS_ERROR            -1  //error
#define RTWIFI_STATUS_DISCONNECTED      0   //disconnect
#define RTWIFI_STATUS_CONNECTING        1   //connecting
#define RTWIFI_STATUS_CONNECTED         2   //connect complete
#define RTWIFI_STATUS_INACTIVE          3   //network is no active
#define RTWIFI_STATUS_UNKNOW        	4	//get status is unknow
#define RTWIFI_CONNECT_PASSWORD_ERROR  -4 //wifi connect password error

//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************



typedef struct
{
	unsigned short year;	// The number of year.
	unsigned char month;	// The number of months since January, in the range 1 to 12.
	unsigned char day;		// The day of the month, in the range 1 to 31.
	unsigned char wday;		// The number of days since Sunday, in the range 0 to 6. (Sunday = 0, Monday = 1, ...)
	unsigned char hour;     // The number of hours past midnight, in the range 0 to 23.
	unsigned char minute;   // The number of minutes after the hour, in the range 0 to 59.
	unsigned char second;   // The number of seconds after the minute, in the range 0 to 59.
}STimeDay;

/*!
*******************************************************************************
** \brief goolink playback cmd.
*******************************************************************************
*/
typedef enum
{
    GLINK_PLAYBACK_RESUME_CMD = 0,     // 0 resume playback
    GLINK_PLAYBACK_PAUSE_CMD,         //  1 pause  playback
    GLINK_PLAYBACK_CLOSE_CMD,         //  2 close  playback
    GLINK_PLAYBACK_PLUS_CMD,          //  3 plus   playback
    GLINK_PLAYBACK_MINUS_CMD,		  //  4 minus  playback
    GLINK_PLAYBACK_JUMP_CMD,		  //  5 jump freame playback
}GLINK_PLAYBACK_CMD_ENUMT;


enum{
    GLINK_SD_STATUS_NOTINIT = 0, //sd 卡不存在
    GLINK_SD_STATUS_OK,          //sd 状态正常
    GLINK_SD_STATUS_NOTFORMAT,   //sd 未格式化
    GLINK_SD_STATUS_FORMAT_OK,   //sd 格式化OK
    GLINK_SD_STATUS_READONLY,    //sd 只读
    GLINK_SD_STATUS_FORMATING,   //sd 正在格式化
    GLINK_SD_STATUS_FILE_FIX     //sd 卡已修复
}GLINK_SD_STATUS_ENMUT;


struct video_channel_info
{
	int chn;
	int isms;
	int enable;
	MEDIABUF_HANDLE mhandle;
};

struct audio_channel_info
{
	int chn;
	int enable;
	MEDIABUF_HANDLE mhandle;
};

typedef enum
{
	RT_VOICE_ERR_EXIT     = 0,
	RT_VOICE_ERR_CONTINUE = 1,
	RT_VOICE_ERR_NORMAL   = 2,
}RT_VOICE_ERR_ENUM_T;

typedef enum{
	LED_STATUS_DOWN = 0,    //led lights down
	LED_STATUS_UP,         //led lights up
	LED_STATUS_NOTINIT,    //led start
	LED_STATUS_BLINK,      //led blinking
	LED_STATUS_ERROR       //led lights error
}RT_BLUE_LED_ENUM_T;

//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************


//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************

static pthread_mutex_t goolinkMutex; //让升级实现互斥, 免得下个升级过来干扰

//////////////////////////////////////////////////////////////////////////
//20190507 之前的调用有泄露, 封装成简单的新函数
//中途的打印可以这么搞, 但是最终提供给外面的字串最好不要这么搞

/* add for goolink p2p start */
#define JS_PRINT_LEN 2048
static char szFormatBuffer[JS_PRINT_LEN];
static void *(*cJSON_malloc)(size_t sz) = malloc;
static void (*cJSON_free)(void *ptr)    = free;

static AviPBHandle* glnkAviPbhandle       = NULL;
static int glnk_send_playback_flag        = GLINK_PLAYBACK_CLOSE_CMD;
static char glnk_send_playback_target     = 0;
static int glnk_playback_thread_video_exit_flag = 0;
static int glnk_playback_thread_audio_exit_flag = 0;
static int glnk_send_playback_is_idle = 0;

static int glnk_playback_frame_interval   = 0;
static AVI_DMS_TIME timePos_start         = {0};
static AVI_DMS_TIME timePlayBackStart     = {0};


static const char *g_ContrlCmd[6]={
    "ResumeSendStream",      // 0,			     //恢复发送
    "PauseSendStream",      //  1,              // 暂停发送
    "CloseSendStream",      //  2,				// 关闭发送
    "PlusSendStream",      //   3,				// 快放
    "MinusSendStream",     //   4,				// 慢放
    "JumpFrameSendStream", //   5				// 拖动播放
};

static struct video_channel_info video_stream_main[GOOLINK_MAX_CLIENT_NUMBER] = {{0}, {0}, {0}, {0}};
static struct video_channel_info video_stream_sub[GOOLINK_MAX_CLIENT_NUMBER]  = {{0}, {0}, {0}, {0}};
static struct audio_channel_info audio_stream_enable = {0};

//exit goolink thread flag
static int rtExitVideoFrameThreadFlag = 0;// 0 open send video stream;1 close send video stream
static int rtExitAudioFrameThreadFlag = 0;// 0 open send audio stream;1 close send audio stream
static int rtExitPlayBackThreadFlag   = 0;
static int rtExitVoiceInfoThreadFlag  = 0;
static int RTExitRTBlueLEDFlag		  = 0;



static unsigned long s_timestampRTStream;


static int rtwifi_sta_mode_connect_enable       = 0;
static int rtwifi_sta_mode_change_enable        = 0;
static int rtwifi_voice_sta_mode_connect_enable = 0;
static int rtwifi_connect_successful_flag       = 0;
static WIFI_LINK_INFO_t RTWiFiLinkInfo;


static int glb_rtLed_status        = LED_STATUS_NOTINIT;
static pthread_mutex_t rtLed_mutex = PTHREAD_MUTEX_INITIALIZER;
static void *glnk_alarm_handle = NULL;


//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************
static int RTMutex_init(void);
static int RTMutex_lock(void);
static int RTMutex_unlock(void);
static int RTMutex_destrory(void);


static unsigned int RTGetTodayMillis(void);
static unsigned int RTGetTimeStamp(void);

static int RTLoad_p2p_id(void);

static void* RTMainThread(void* arg);

static int RTVoiceDetectListen(void);
static void* RTVoiceInfoThread(void* arg);
static int RTExitVoiceInfoThread(void);
static int RTExitRTBlueLEDThread(void);


static void* RTWifiThread(void* arg);

static void* RTBlueLEDThread(void* arg);


static int RTSendVideoFrameStream(struct video_channel_info *pvideoInfo);
static void *RTVideoFrameThread(void *arg);
static int RTExitSendVideoFrameStream(void);

static void *RTAudioFrameThread(void *arg);
static int RTExitAudioFrameThread(void);

static void *RTPlayBackThread(void *arg);
static int RTExitPlayBackThread(void);

static int RTCreateStreamoutThread(void);

static AviPBHandle *RTPlayBackByTime(AVI_DMS_TIME *lpStartTime, AVI_DMS_TIME *lpStopTime);
static int RTAviGetTimeDay(AviPBHandle* pPBHandle,STimeDay *td);
static int RTAviTimeToTick(STimeDay *td, int zone);
static int RTAviCalStopTime(AVI_DMS_TIME *lpStartTime, AVI_DMS_TIME *lpStopTime);


//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************

char * Jscpy2data(char * psztmp)
{
    memset(szFormatBuffer, 0, JS_PRINT_LEN);
    strncpy(szFormatBuffer, psztmp, JS_PRINT_LEN);
    cJSON_free(psztmp);
    return szFormatBuffer;
}

char * JsPrintFmt(cJSON * Json)
{
    char * psztmp = cJSON_Print(Json); //有malloc
    return Jscpy2data(psztmp); //free
}


char * JsPrintStr(cJSON * Json)
{
    char * psztmp = cJSON_PrintUnformatted(Json); //有malloc
    return Jscpy2data(psztmp); //free
}

//最终提供给外面的直接用这个函数, 上面两个函数担心字串长度不足
char * JsToBuffer(cJSON * Json, char * pbuf)
{
    //1 先将内容打印一下, 不泄露的
    printf("%s\n", JsPrintFmt(Json));

    //2 复制给到外面的字串
    char * psztmp = cJSON_PrintUnformatted(Json);
    strncpy(pbuf, psztmp, strlen(psztmp));
    cJSON_free(psztmp);
	return pbuf;
}

void addtoMgr(JS_HEAP_OBJ_MGR * pJsMgr, cJSON * pNewNode)
{
    pJsMgr->Json[pJsMgr->num] = pNewNode;
    pJsMgr->num++;
}

cJSON *cJSON_CreateObjectEx(JS_HEAP_OBJ_MGR * pJsMgr)
{
    if (pJsMgr->num >= MAX_OBJ){
        goolink_error("json obj overflow !! return !! \n");
        return NULL;
    }
    cJSON * pNewNode = cJSON_CreateObject();
    addtoMgr(pJsMgr, pNewNode);
    return pNewNode;
}

cJSON *cJSON_CreateArrayEx(JS_HEAP_OBJ_MGR * pJsMgr)
{
    if (pJsMgr->num >= MAX_OBJ){
        goolink_error("json obj overflow !! return !! \n");
        return NULL;
    }
    cJSON * pNewNode = cJSON_CreateArray();
    addtoMgr(pJsMgr, pNewNode);
    return pNewNode;
}

cJSON *cJSON_ParseEx(JS_HEAP_OBJ_MGR * pJsMgr, const char *value)
{
    if (pJsMgr->num >= MAX_OBJ){
        goolink_error("json obj overflow !! return !! \n");
        return NULL;
    }
    cJSON * pNewNode = cJSON_Parse(value);
    addtoMgr(pJsMgr, pNewNode);
    return pNewNode;
}

void JsHeapObjFree(JS_HEAP_OBJ_MGR * pJsMgr)
{
    int i = 0;
    for (i=0; i<pJsMgr->num; i++){
        if (pJsMgr->Json[i]){
            cJSON_Delete(pJsMgr->Json[i]);
        }
    }
}

static int RTMutex_init(void)
{
	return pthread_mutex_init(&goolinkMutex, NULL);
}

static int RTMutex_lock(void)
{
	return pthread_mutex_lock(&goolinkMutex);
}

static int RTMutex_unlock(void)
{
	return pthread_mutex_unlock(&goolinkMutex);
}

static int RTMutex_destrory(void)
{
	return pthread_mutex_destroy(&goolinkMutex);
}

static unsigned int RTGetTodayMillis(void)
{
    /*
    昨天填的时间戳，改成这个函数获取到的值，填给我们
    这个是我们其他客户有在用了，统一
    */
    struct tm curTime;
    struct timeval tv;
    gettimeofday( &tv, NULL );
    localtime_r(&tv.tv_sec, &curTime); //要取得localtime
    return (curTime.tm_hour * 3600 +
            curTime.tm_min * 60 +
            curTime.tm_sec)  * 1000
            + tv.tv_usec/1000;
}

static unsigned int RTGetTimeStamp(void)
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec*1000 + tv.tv_usec/1000);
}

static int RTLoad_p2p_id(void)
{
    #if 1
    struct device_info_mtd info;
    int ret = load_info_to_mtd_reserve(MTD_GB28181_INFO, &info, sizeof(info));
    if((ret < 0) || (info.device_id[0] == 0))
    {
        PRINT_ERR("[p2p] Fail to load p2p id from flash, p2p id:%s\n", info.device_id);
#ifdef MODULE_SUPPORT_GB28181
        return -1;
#endif
		//80400002   804268aa  900000010000
		/* ipc tool will strstr the id with "000000000", do not remove it. */
        strncpy(info.device_id, (char*)&runSystemCfg.deviceInfo.serialNumber, sizeof(info.device_id));
        strncpy(info.manufacturer_id, "goke_ds", sizeof(info.manufacturer_id));

        /*NEW ADD ,TUTK APP获取的参数,在进行生产时，先使用默认方式得到配置，然后生产人员
        读取flash相应位置的数据并根据实际情况改写数据，如读到ODM_DS处，并改写实际的ODM类型
        之后，再利用这个写好的flash去烧写其他机器，量产，app读取写着数据时，在Handle_IOCTRL_Cmd
        里面，从flash读取数据并返回app相应数据。
        */
        #if 0
        strncpy(info.device_type, "YTJ_NORMAL", sizeof(info.device_type));
        strncpy(info.odm_id, "ODM_DS", sizeof(info.odm_id));
        strncpy(info.p2p_type, "P2P_TUTK", sizeof(info.p2p_type));
        strncpy(info.panorama_mode, "PANORAMA_TOP", sizeof(info.panorama_mode));
        strncpy(info.language, "SIMPLE_CH", sizeof(info.panorama_mode));
        #endif
        info.devType      = 1;// 设备类型: 0-卡片机, 1-摇头机, 2-鱼眼摄像机...
        info.netFamily    = 2;// 网络提供商: 1-迈特威视, 2-tutk...
        info.serverID     = 0; // 服务器ID: 保留,默认为0
        info.language     = 1; // 设备固件语言版本: 0为自动适应(即多语言),1为简体中文,2为繁体中文,3为英文
        info.odmID        = 0;// odm商ID: 0-东舜自己, 01-TCL, 02-康佳...
        info.panoramaMode = 0;// 全景安装模式: 0-吸顶式,1-壁挂式
        strncpy((char*)info.model, "ts-7000-w", sizeof(info.model));


        ret = save_info_to_mtd_reserve(MTD_GB28181_INFO, &info, sizeof(info));
        if (ret < 0)
        {
            goolink_error("[p2p] set p2p id to flash error.\n");
            return -1;
        }
        else
        {
            goolink_printk("[p2p] set p2p id %s to flash ok.\n", info.device_id);
        }
    }

    PRINT_INFO("P2P ID :%s\n",info.device_id);
    PRINT_INFO("devType :%u\n",info.devType);
    PRINT_INFO("netFamily :%u\n",info.netFamily);
    PRINT_INFO("language :%u\n",info.language);
    PRINT_INFO("odmID ID:%u\n",info.odmID);
    PRINT_INFO("panoramaMode :%u\n",info.panoramaMode);
    PRINT_INFO("model :%s\n",info.model);

    #ifdef MODULE_SUPPORT_GB28181
    #else
    if (strcmp(runSystemCfg.deviceInfo.serialNumber, info.device_id) != 0)
    {
        strncpy(runSystemCfg.deviceInfo.serialNumber, info.device_id,sizeof(runSystemCfg.deviceInfo.serialNumber));
        SystemCfgSave();
    }
    #endif
#else
    sprintf(runSystemCfg.deviceInfo.serialNumber, "%s", "EFPA9D5WKV74AG6GY1XJ");
#endif
    PRINT_INFO("[p2p] get from mtd, serialNumber:%s\n", runSystemCfg.deviceInfo.serialNumber);

    return 0;
}


static int RTAviGetTimeDay(AviPBHandle* pPBHandle,STimeDay *td)
{
    char file_name[128] = {0};

    int i = last_index_at(pPBHandle->file_path, '/');
    strcpy(file_name, pPBHandle->file_path + i + 1);
    u64t time_start_64 = 0;
    time_start_64 = avi_get_start_time(file_name);

    AVI_DMS_TIME t1 = {0};
    u64t_to_time(&t1, time_start_64);
    td->year   = t1.dwYear;
    td->month  = t1.dwMonth;
    td->day    = t1.dwDay;
    td->hour   = t1.dwHour;
    td->minute = t1.dwMinute;
    td->second = t1.dwSecond;

    //PRINT_INFO("file:%s, %s,---- GET timeday %d-%d-%d  %d:%d:%d\n",
    //    pPBHandle->file_path, file_name, td->year,td->month,td->day,td->hour,td->minute,td->second);
    return 0;
}

static int RTAviTimeToTick(STimeDay *td, int zone)
{

	struct tm temp;
	time_t tt;

	memset(&temp,0,sizeof(temp));
	memset(&tt,0,sizeof(tt));

	temp.tm_year = td->year - 1900;
	temp.tm_mon  = td->month - 1;
	temp.tm_mday = td->day;
	temp.tm_hour = td->hour;
	temp.tm_min  = td->minute;
	temp.tm_sec  = td->second;
	tt = mktime(&temp);
	tt += zone;
    //PRINT_INFO("time_to_tick :%u\n",timep1);
    return tt;

}

static int RTAviCalStopTime(AVI_DMS_TIME *lpStartTime, AVI_DMS_TIME *lpStopTime)
{
	if ((NULL == lpStartTime) || (NULL == lpStopTime))
	{
		goolink_error("parameter error!\n");
		return -1;
	}

	RTMutex_lock();
    lpStopTime->dwYear   = lpStartTime->dwYear;
    lpStopTime->dwMonth  = lpStartTime->dwMonth;
    lpStopTime->dwDay    = lpStartTime->dwDay;
    lpStopTime->dwHour   = 23;
    lpStopTime->dwMinute = 59;
    lpStopTime->dwSecond = 59;
	RTMutex_unlock();

    return 0;
}

static AviPBHandle *RTPlayBackByTime(AVI_DMS_TIME *lpStartTime, AVI_DMS_TIME *lpStopTime)
{
    goolink_printk("call RTPlayBackByTime\n");
	int ret     = 0;
    u32t ch_num = 0;

	if ((NULL == lpStartTime) || (NULL == lpStopTime))
	{
		goolink_error("parameter error!\n");
		return NULL;
	}

    u64t start  = time_to_u64t(lpStartTime);
    u64t stop   = time_to_u64t(lpStopTime);

    AviPBHandle *pPBHandle = NULL;
    pPBHandle = (AviPBHandle *)create_pb_handle();
    if (NULL == pPBHandle) {
        goolink_error(
"Fail to create playback handle\n");
        return NULL;
    }

    pPBHandle->list = search_file_by_time(0, ch_num, start, stop);  //0， 所有的情况
    if (NULL == pPBHandle->list) {
        goolink_error("playback list is null.\n");
		avi_pb_close(pPBHandle);
        return NULL;
    }
#if 0
    //读下一个文件
    ret = avi_pb_slide_to_next_file(pPBHandle);
    if (ret != 0)
    {
        goolink_error("avi_pb_slide_to_next_file error.");
        return NULL;
    }
#endif
    //按起始时间定位 pPBHandle 的no
    int seekPos = cal_pb_seek_time(pPBHandle, lpStartTime);
    if(seekPos < 0)
    {
        goolink_error("playback seek position error\n");
		//search_close(pPBHandle->list);
		avi_pb_close(pPBHandle);
        return NULL;
    }
    ret = avi_pb_open(pPBHandle->file_path, pPBHandle);
    if (ret < 0) {
        goolink_error("avi_pb_open %s error.\n", pPBHandle->file_path);
		//search_close(pPBHandle->list);
		avi_pb_close(pPBHandle);
        return NULL;
    }
    avi_pb_set_pos(pPBHandle, &seekPos);

	print_pb_handle(pPBHandle);

    return pPBHandle;
}


static void* RTMainThread(void* arg)
{
	int ret;
	InitNetParam netparam;
	memset(&netparam, 0, sizeof(InitNetParam));

	pthread_detach(pthread_self());
    sdk_sys_thread_set_name("RTMainInitThread");
	goolink_printk("RTMainThread start OK!\n");
    struct device_info_mtd info;
    ret = load_info_to_mtd_reserve(MTD_GB28181_INFO, &info, sizeof(info));
    if((ret < 0) || (info.device_id[0] == 0))
    {
        return NULL;
    }

	if(netcam_net_get_detect("eth0")==0)
	{
		strncpy(netparam.dev, "eth0", sizeof(netparam.dev));
	}
	else
	{
		char *WifiName = netcam_net_wifi_get_devname();
		if(WifiName != NULL)
		{
			strncpy(netparam.dev, WifiName, sizeof(netparam.dev));
		}else{
			goolink_error("netcam_net_wifi_get_devname error.use lan.\n");
            strncpy(netparam.dev, "eth0", sizeof(netparam.dev));
		}
	}

	strncpy(netparam.udid, info.device_id, sizeof(netparam.udid));

	netparam.localTCPport            = 49000;
	netparam.localUDPport            = 59000;
	netparam.channelnum              = 1;
	netparam.issupportsubvideostream = 1;
	netparam.maxsession              = 16;
	goolink_printk("goolink version is 0x%x \n", (int)glnk_get_version());

	while(1)
	{
		ret = glnk_init(&netparam);
		if (0 == ret)
		{
			goolink_printk("goolink start success!\n");
			break;
		}
		sleep(2);
	}
	goolink_printk("goolink_main end!\n");
	return 0;
}

static int RTVoiceDetectListen(void)
{
	int ret                    = 0;
	char szSsid[128]           = {0};
	char szWiFiPass[128]       = {0};
	char szgid[128]            = {0};
	char *pSsid                = NULL;
	char *pPasswd              = NULL;
	int g_audio_wifi_configing = 0;
	int bGotWiFiSSID           = 0;
	int ethDetectCount         = 0;
	WIFI_LINK_INFO_t linkInfo;
	ST_SDK_NETWORK_ATTR net_attr;
	struct device_info_mtd info;

	while(1)
	{
		if (netcam_net_get_detect("eth0")==0)
			sleep(3);
		else if (runNetworkCfg.wireless.mode == 0) //ap mode,用户从有线切换到无线，又是ap模式的话，还是可以配网
			break;
		else if (runNetworkCfg.wireless.mode == 1) //sta mode，用户通过AP配网方式配上网之后，就不需要声波配网了。
			return RT_VOICE_ERR_EXIT;

	}

	while(1)
	{
		memset(&info,0,sizeof(info));
		ret = load_info_to_mtd_reserve(MTD_GB28181_INFO, &info, sizeof(info));

		if (ret == 0)
		{
			strcpy(szgid,info.device_id);
			break;
		}else
			usleep(300*1000);

	}

    goolink_printk("DoDetectListen, Get_UUID_CharLow, gid=%s \n", szgid);

	//goke is 16000 , not is 44100  16000
    if(!LT_VoiceWifiConfigStart(szgid, 16000)) { //直接进入
        goolink_error("DoDetectListen, LT_VoiceWifiConfigStart fail!\n");
        g_audio_wifi_configing = 0;
        return RT_VOICE_ERR_EXIT;
    }
    usleep(100 * 1000); //因为下面有poll, 这里千万不能去掉!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	while(1)
	{
		if (netcam_net_get_detect("eth0")==0)
		{
			RTMutex_lock();
			rtwifi_voice_sta_mode_connect_enable = 0;
			RTMutex_unlock();
			LT_VoiceWifiConfigClose();

			return RT_VOICE_ERR_CONTINUE;
		}

		if(runNetworkCfg.wifi.enable == 1){

			if( (runNetworkCfg.wireless.mode == 1) && (strlen((char*)runNetworkCfg.wifilink[0].essid)!=0) ) //sta mode
			{
				WIFI_LINK_INFO_t linkInfo;
				memset(&linkInfo,0,sizeof(linkInfo));
				linkInfo.linkStatus = -1;
				netcam_net_wifi_get_connect_info(&linkInfo);
				if (linkInfo.linkStatus == WIFI_CONNECT_OK)
					// in sta mode,no acoustic mesh cna be used,
					// you have to reboot to use the mesh again.
					break;
			}
		}
		else  // wifi disable
		{
			break;
		}

		if (runNetworkCfg.wireless.mode == 0) // AP mode
		{
			if(LT_VoiceWifiConfigPoll(&pSsid,&pPasswd))
			{
				RTMutex_lock();
				goolink_printk("ok ssid %s pw %s\n", pSsid, pPasswd);
				memset(&RTWiFiLinkInfo,0,sizeof(RTWiFiLinkInfo));
				strcpy(RTWiFiLinkInfo.linkEssid,pSsid);
				strcpy(RTWiFiLinkInfo.linkPsd,pPasswd);
				RTWiFiLinkInfo.linkScurity = WIFI_ENC_WPAPSK_AES;
#if 0
				usleep(1000*1000);

				LT_VoiceWifiConfigClose();
				usleep(1000*1000);
				LT_VoiceWifiConfigStart(szgid, 16000);
				usleep(600*1000);

#else
				if (rtwifi_voice_sta_mode_connect_enable == 0){
					rtwifi_voice_sta_mode_connect_enable = 1;
				}

#endif

				RTMutex_unlock();
			}

		}

		usleep(100*1000);
	}

	RTMutex_lock();
	//LT_voice_restore_init_status(0);
	rtwifi_voice_sta_mode_connect_enable = 0;
	RTMutex_unlock();
	LT_VoiceWifiConfigClose();

	return RT_VOICE_ERR_NORMAL;

}

static void* RTVoiceInfoThread(void* arg)
{
	int ret = 0;

	pthread_detach(pthread_self());
	sdk_sys_thread_set_name("RTVoiceInfo");
	goolink_printk("RTVoiceInfo start OK!\n");

	while(1)
	{
		if (rtExitVoiceInfoThreadFlag == 1)
			break;

		if (netcam_net_get_detect("eth0")!=0) // not
		{
			ret = RTVoiceDetectListen();
			if (ret == RT_VOICE_ERR_EXIT)
				break;
			else if (ret == RT_VOICE_ERR_CONTINUE)
				continue;
			else if (ret == RT_VOICE_ERR_NORMAL)
				break;
		}
		else
			sleep(2);
	}

	return NULL;
}

static int RTExitVoiceInfoThread(void)
{
	int ret = -1;

	rtExitVoiceInfoThreadFlag = 1;

	return 0;

}


/*sdcard error status used to osd show
*/


static int RTBlueLedSetUp(void)
{
	int ret = 0;
	sdk_gpio_set_output_value(BLUE_LED_GPIO,1);
	if (ret != 0)
	{
		goolink_error("sdk_gpio_set_output_value 1 failed!\n");
		return -1;
	}

	return 0;

}

static int RTBlueLedSetDown(void)
{
	int ret = 0;
	ret = sdk_gpio_set_output_value(BLUE_LED_GPIO,0);
	if (ret != 0)
	{
		goolink_error("sdk_gpio_set_output_value 0 failed!\n");
		return -1;
	}

	return 0;
}


static char RTBlueGetLedEnable(void)
{
	if (access("/opt/custom/cfg/blueLed.conf", R_OK) != 0)
	{

		return LED_STATUS_NOTINIT;
	}
	FILE *ledFp = NULL;

	ledFp = fopen("/opt/custom/cfg/blueLed.conf","r");
	if (NULL == ledFp)
	{
		goolink_error("fopen /opt/custom/cfg/blueLed.conf failed!\n");
		return LED_STATUS_ERROR;
	}
	fseek(ledFp,0L,SEEK_SET);

	char ledEnableValue =0;
	char buf[6]={0};
	int readCnt = fread(buf,sizeof(buf),1,ledFp);  /* 返回值为0 */
	fclose(ledFp);

	if (strcmp(buf,"false") == 0)
		ledEnableValue = LED_STATUS_DOWN;
	else if (strcmp(buf,"true") == 0)
		ledEnableValue = LED_STATUS_UP;

	//printf("readCnt = %d,ledEnableValue = %d\n",readCnt,ledEnableValue);

	return ledEnableValue;
}

static char RTBlueSetLedEnable(char enableValue)
{


	char getEnableValue = -1;
	getEnableValue = RTBlueGetLedEnable();
	if (getEnableValue == LED_STATUS_ERROR)
	{
		goolink_error("getEnableValue = %d error!\n",getEnableValue);
		return -1;
	}

	goolink_printk("======getEnableValue :%d  enableValue:%d\n",getEnableValue,enableValue);
	if (getEnableValue != enableValue)
	{
		FILE *ledFp = NULL;
		ledFp = fopen("/opt/custom/cfg/blueLed.conf","w");
		if (NULL == ledFp)
		{
			goolink_error("fopen /opt/custom/cfg/blueLed.conf failed!\n");
			return -1;
		}

		fseek(ledFp,0L,SEEK_SET);
		char buf[6] = {0};

		if (enableValue == LED_STATUS_DOWN)
			strcpy(buf,"false");
		else if (enableValue == LED_STATUS_UP)
			strcpy(buf,"true");

		fwrite(buf,sizeof(buf),1,ledFp);
		fclose(ledFp);
	}
}



static void* RTBlueLEDThread(void* arg)
{
	pthread_detach(pthread_self());
	sdk_sys_thread_set_name("RTBlueLed");

	int audioConfigWifiCount = 0;
	int BlueSetUpCount = 0;
	int BlueSetUpCount2 = 0;
    int value = 0;
	int ledEnable = 0;

	while(1)
	{
		if (RTExitRTBlueLEDFlag == 1)
			break;

		if (0 == netcam_net_get_detect("eth0")) // eth0
		{
			if ((BlueSetUpCount== 1))
			{
				RTBlueLedSetUp();
			}
			BlueSetUpCount++;
			if(BlueSetUpCount == 1024)
			{
				BlueSetUpCount =2;
			}
		}

		else   //wireless
		{
			if(strlen(runNetworkCfg.wifilink[0].essid) == 0)//AP
			{
				if (runNetworkCfg.wireless.mode == 0) //ap mode
				{
					if ((audioConfigWifiCount== 1))
					{
						RTMutex_lock();
						//netcam_audio_hint(SYSTEM_AUDIO_HINT_CONFIG_WIFI);
						RTMutex_unlock();
					}
					audioConfigWifiCount++;
					if(audioConfigWifiCount == 1024)
					{
						audioConfigWifiCount =2;
					}
					RTBlueLedSetDown();
					usleep(500*1000);
					RTBlueLedSetUp();
					usleep(500*1000);
				}
				continue;
			}
			else  //STA
			{
				if ((BlueSetUpCount2== 1))
					{
						RTBlueLedSetUp();
					}
				BlueSetUpCount2++;
				if(BlueSetUpCount2 == 1024)
				{
					BlueSetUpCount2 =2;
				}

			}
		}//end of wireless

		ledEnable = RTBlueGetLedEnable();
		if (ledEnable == -1)
		{
			goolink_error("RTBlueGetLedEnable failed!\n");
			break;
		}

		switch(ledEnable)
		{
			case LED_STATUS_NOTINIT:
				break;

			case LED_STATUS_UP:
				RTBlueLedSetUp();
				break;

			case LED_STATUS_DOWN:
				RTBlueLedSetDown();
				break;
		}//end of switch

		usleep(500*1000);

	}
	audioConfigWifiCount = 0;
	BlueSetUpCount = 0;
	BlueSetUpCount2 = 0;
	return NULL;
}

static int RTExitRTBlueLEDThread(void)
{
	RTExitRTBlueLEDFlag = 1;
    return 0;
}

static void RTMonitor_call_back(int status)
{

}

static void* RTWifiThread(void* arg)
{
	int ret              = 0;
	int reset_count      = 0;
	char szSsid[128]     = {0};
	char szWiFiPass[128] = {0};
	char szgid[128]      = {0};
	char *pSsid          = NULL;
	char *pPasswd        = NULL;
	WIFI_LINK_INFO_t linkInfo;
	ST_SDK_NETWORK_ATTR net_attr;
	struct device_info_mtd info;
	int netDeviceErrorCount = 0;

	pthread_detach(pthread_self());
    sdk_sys_thread_set_name("RTWifiThread");
	goolink_printk("RTWifiThread start OK!\n");

	while(1)
	{
		if ((rtwifi_sta_mode_connect_enable == 1) || (rtwifi_voice_sta_mode_connect_enable == 1))
		{
			RTMutex_lock();
			//netcam_audio_hint(SYSTEM_AUDIO_HINT_GET_WIRELESS_INFO_SUCCESS);
			RTMutex_unlock();

			//netcam_net_wifi_on();
			if (strlen(RTWiFiLinkInfo.linkEssid)> 0){
				memset(&linkInfo,0,sizeof(WIFI_LINK_INFO_t));

				strncpy(linkInfo.linkEssid, RTWiFiLinkInfo.linkEssid, sizeof(linkInfo.linkEssid)-1);
				if (strlen(RTWiFiLinkInfo.linkPsd) > 0){
					strncpy(linkInfo.linkPsd,RTWiFiLinkInfo.linkPsd, sizeof(linkInfo.linkPsd)-1);
					linkInfo.linkScurity = RTWiFiLinkInfo.linkScurity;
				}
				else
				{
					linkInfo.linkScurity = WIFI_ENC_NONE;
				}

				linkInfo.isConnect   = 1;

			}
			memset(&RTWiFiLinkInfo,0,sizeof(RTWiFiLinkInfo));

			goolink_printk("sta linkinfo  ssid : %s, psd:%s\n",linkInfo.linkEssid,linkInfo.linkPsd);

			ret = netcam_net_setwifi(NETCAM_WIFI_STA,linkInfo);
			if(ret==0)
			{

				if (rtwifi_connect_successful_flag == 0){
					RTMutex_lock();
					rtwifi_connect_successful_flag = 1;
					RTMutex_unlock();
				}
				netcam_audio_hint(SYSTEM_AUDIO_HINT_LINK_SUCCESS);

			}
			else if (ret < 0)// connect failed
			{
				if (ret == RTWIFI_CONNECT_PASSWORD_ERROR)
				{
					goolink_error("wifi connrct password error,ret = %d\n",ret);
				}
				netcam_audio_hint(SYSTEM_AUDIO_HINT_LINK_FAIL);

				if (rtwifi_sta_mode_change_enable == 0)
				{
					goolink_error("connect wifi sta mode failed! ,switch to ap mode\n");
					//netcam_net_wifi_on();
					RTMutex_lock();
					runNetworkCfg.wireless.mode = 0;
					memset(&linkInfo,0,sizeof(WIFI_LINK_INFO_t));
					RTMutex_unlock();
					ret = netcam_net_setwifi(NETCAM_WIFI_AP,linkInfo);
					if (ret < 0)
					{
						goolink_error("netcam_net_setwifi error,ret = %d\n",ret);
						reset_count++;
						if (reset_count > 2)
						{
							//todo reset system
							goolink_error("&&&&&&&&&&&&&&&&& wifi error reset system &&&&&&&&&&&&\n");
							netcam_sys_operation(NULL, (void*)SYSTEM_OPERATION_HARD_DEFAULT);
						}
					}

					char cmd[128]={0};
					sprintf(cmd,"route add -host 255.255.255.255 dev %s",netcam_net_wifi_get_devname());
					new_system_call(cmd);
					netcam_net_cfg_save();
				}
				else if (rtwifi_sta_mode_change_enable == 1)
				{
					goolink_error("connect wifi sta mode failed! ,switch to old ssid\n");
					ret = network_wifi_monitor(runNetworkCfg.wifilink, WIFI_CONNECT_LIST_MAX_NUMBER, RTMonitor_call_back);
					if (ret == RTWIFI_STATUS_CONNECTED )
					{
						goolink_printk("link old ssid sucessful\n");
					}
				}
			}

			RTMutex_lock();
			if (rtwifi_sta_mode_connect_enable == 1)
				rtwifi_sta_mode_connect_enable = 0;

			if (rtwifi_voice_sta_mode_connect_enable == 1)
				rtwifi_voice_sta_mode_connect_enable = 0;
			RTMutex_unlock();

		}
		else{

			sleep(1);
		}

	}
	return 0;
}


static int RTSendVideoFrameStream(struct video_channel_info *pvideoInfo)
{
	int ret            = 0;
	int recv_frame_len = 0;
	int frametype      = 0;
	char* buffer       = NULL;
	if (pvideoInfo == NULL)
	{
		goolink_error("pvideoInfo is NULL\n");
		return -1;
	}

	if (pvideoInfo->enable == 0)
	{
		goolink_error("pvideoInfo->enable = 0\n");
		return -1;
	}

	GK_NET_FRAME_HEADER frame_header;
	memset(&frame_header, 0, sizeof(GK_NET_FRAME_HEADER));

	MEDIABUF_HANDLE mhandle   = pvideoInfo->mhandle;
	unsigned long dwChannel   = pvideoInfo->chn;
	unsigned char ismainorsub = pvideoInfo->isms;

	if (glnk_playback_thread_video_exit_flag == 1)
	{
		RTMutex_lock();
		glnk_playback_thread_video_exit_flag = 0;
		RTMutex_unlock();
		mediabuf_set_newest_frame(mhandle);
		int stream_id = ismainorsub;
		netcam_video_force_i_frame(stream_id);
	}

	ret = mediabuf_read_frame(mhandle, (void**)&buffer, &recv_frame_len, &frame_header);
	if (ret <= 0)
	{
		goolink_error("ERROR! read no data.\n");
		return -1;
	}

	if (frame_header.frame_type == GK_NET_FRAME_TYPE_I)
		frametype = 1;
	else if (frame_header.frame_type == GK_NET_FRAME_TYPE_P)
		frametype = 0;
	else
		return 0;

    s_timestampRTStream = RTGetTodayMillis();

	ret = glnk_sendvideostream(dwChannel, ismainorsub, frametype, buffer, recv_frame_len, s_timestampRTStream);
	if (ret != recv_frame_len){
		//goolink_error("glnk send video data. frametype=%d,recv_frame_len=%d,ret=%d\n", frametype,recv_frame_len,ret);
		return -1;
	}

	return 0;
}

static void *RTVideoFrameThread(void *arg)
{
	int ret = 0;
	int i   = 0;
	unsigned int tStart = 0, tEnd  = 0;
	int enable_count = 0;
	//unsigned int sendFrameRoundTick = 0;

	pthread_detach(pthread_self());
    sdk_sys_thread_set_name("VideoFrameThread");

	goolink_printk("RTVideoFrameThread start OK!\n");

	RTMutex_lock();

	for(i = 0; i < GOOLINK_MAX_CLIENT_NUMBER;i++){
		video_stream_main[i].mhandle = mediabuf_add_reader(GK_NET_STREAM_TYPE_STREAM0);
		video_stream_sub[i].mhandle  = mediabuf_add_reader(GK_NET_STREAM_TYPE_STREAM1);
	}
	RTMutex_unlock();

	while(1)
	{
		enable_count = 0;
        for(i = 0 ; i < GOOLINK_MAX_CLIENT_NUMBER; i++)
        {
			if (video_stream_main[i].enable == 1)
			{
				RTSendVideoFrameStream(&video_stream_main[i]);
				enable_count = 1;
			}
			else if (video_stream_sub[i].enable == 1)
			{
				RTSendVideoFrameStream(&video_stream_sub[i]);
				enable_count = 1;
			}

        }
		if(0 == enable_count)
			usleep(300000);
		if (rtExitVideoFrameThreadFlag == 1)
			break;
	}

	RTMutex_lock();
	for(i = 0; i < GOOLINK_MAX_CLIENT_NUMBER;i++){
		mediabuf_del_reader(video_stream_main[i].mhandle);
		mediabuf_del_reader(video_stream_sub[i].mhandle);
	}
	RTMutex_unlock();

	return NULL;

}

static int RTExitSendVideoFrameStream(void)
{
	int i = 0;
	for(i = 0; i < GOOLINK_MAX_CLIENT_NUMBER;i++){
		video_stream_main[i].enable = 0;
		video_stream_sub[i].enable  = 0;
	}

	rtExitVideoFrameThreadFlag = 1;
	return 0;
}

static void *RTAudioFrameThread(void *arg)
{
	unsigned int tStart = 0, tEnd   = 0;
	//unsigned int sendFrameRoundTick = 0;
	int ret, recv_frame_len;
	int alaw_len = 0;
	char alaw_data[4096] = {0};
	char* buffer = NULL;
	GK_NET_FRAME_HEADER frame_header;
	char send_failed = 0;

	pthread_detach(pthread_self());
    sdk_sys_thread_set_name("AudioFrameThread");
	RTMutex_lock();

	goolink_printk("AudioFrameThread start OK!\n");
	memset(&audio_stream_enable,0,sizeof(audio_stream_enable));

	if (!audio_stream_enable.mhandle)
		audio_stream_enable.mhandle = mediabuf_add_reader(GK_NET_STREAM_TYPE_STREAM0);

	RTMutex_unlock();

	while(1)
	{
		//tStart = RTGetTimeStamp();
		if(audio_stream_enable.enable == 1)
		{
			buffer = NULL;
			recv_frame_len = 0;
			memset(&frame_header, 0, sizeof(GK_NET_FRAME_HEADER));

			if (glnk_playback_thread_audio_exit_flag == 1){
				RTMutex_lock();
				glnk_playback_thread_audio_exit_flag = 0;
				RTMutex_unlock();
				mediabuf_set_newest_frame(audio_stream_enable.mhandle);
			}


			ret = mediabuf_read_frame(audio_stream_enable.mhandle, (void**)&buffer, &recv_frame_len, &frame_header);
			if (ret <= 0)
			{
				goolink_error("ERROR! read no data.\n");
				usleep(30000);
				continue;
			}

			if (frame_header.frame_type != GK_NET_FRAME_TYPE_A)
				continue;

			if(recv_frame_len / 2 > sizeof(alaw_data))
			{
				goolink_error("data_size[%d] error\n", recv_frame_len);
				continue;
			}

			alaw_len = audio_alaw_encode(alaw_data, buffer, recv_frame_len);
			ret = glnk_sendaudiostream(audio_stream_enable.chn, alaw_data, alaw_len, s_timestampRTStream);
			if (ret != alaw_len)
			{
				send_failed++;
				if(send_failed >= 30)
				{
					goolink_error("glnk send live audio data continue %d times failed \n",send_failed);
					send_failed = 0;
					usleep(30000);
				}
				//goolink_error("glnk send audio data. recv_frame_len=%d,ret=%d\n", alaw_len,ret);
			}
			else
			{
				send_failed = 0;
			}
		}

		//tEnd = RTGetTimeStamp();
		//sendFrameRoundTick = tEnd - tStart;

		if(audio_stream_enable.enable == 0)
		{
			usleep(30000);
			send_failed = 0;
		}

		if (rtExitAudioFrameThreadFlag == 1)
			break;
	}
	RTMutex_lock();
	audio_stream_enable.enable = 0;
	mediabuf_del_reader(audio_stream_enable.mhandle);
	RTMutex_unlock();
	return NULL;
}

static int RTExitAudioFrameThread(void)
{
	rtExitAudioFrameThreadFlag = 1;
	return 0;
}

static void *RTPlayBackThread(void *arg)
{
	int ret  = 0;
	int size = 0;
	int alaw_len = 0;
	char alaw_data[4096] = {0};
	float t1, read_no, index_count, video_count, ffps;

	pthread_detach(pthread_self());
	sdk_sys_thread_set_name("PlayBackThread");
	goolink_printk("PlayBackThread start OK!\n");

	while (1)
	{
		if (rtExitPlayBackThreadFlag == 1)
			break;//exit thread

		if (glnk_send_playback_flag == GLINK_PLAYBACK_PAUSE_CMD)//暂停发送
		{
			if(glnk_send_playback_is_idle != 1)
			{
				RTMutex_lock();
				glnk_send_playback_is_idle = 1;
				RTMutex_unlock();
			}
			usleep(300000);
			continue;
		}
		else if ((glnk_send_playback_flag == GLINK_PLAYBACK_CLOSE_CMD)|| (mmc_get_sdcard_stauts() != SD_STATUS_OK))//关闭
		{
			RTMutex_lock();
			if (glnkAviPbhandle != NULL)
			{
				avi_pb_close(glnkAviPbhandle);
				glnkAviPbhandle = NULL;
			}

			//if (glnk_playback_thread_video_exit_flag == 0)
			//	glnk_playback_thread_video_exit_flag = 1;

			//if (glnk_playback_thread_audio_exit_flag == 0)
			//	glnk_playback_thread_audio_exit_flag = 1;
			RTMutex_unlock();

			usleep(500000);
			continue;
		}
		else if (glnk_send_playback_flag == GLINK_PLAYBACK_RESUME_CMD) //resume cmd
		{
			if (glnkAviPbhandle == NULL){
				AVI_DMS_TIME reume_start_time  = {0};
				AVI_DMS_TIME reume_stop_time   = {0};

				memcpy((void *)&reume_start_time,(void*)&timePlayBackStart,sizeof(AVI_DMS_TIME));

				RTAviCalStopTime(&reume_start_time, &reume_stop_time);
				//search for playback files by time period
				glnkAviPbhandle = RTPlayBackByTime(&reume_start_time, &reume_stop_time);
				if (!glnkAviPbhandle){
					goolink_error("create glnkAviPbhandle failed!\n");
					usleep(300000);
					continue;
				}

				memset(&timePlayBackStart,0,sizeof(timePlayBackStart));

			}

		}
		else if (glnk_send_playback_flag == GLINK_PLAYBACK_JUMP_CMD)//跳转
		{

			if (glnkAviPbhandle != NULL)
			{
				avi_pb_close(glnkAviPbhandle);
				glnkAviPbhandle = NULL;
			}

			AVI_DMS_TIME t_start = {0};
			AVI_DMS_TIME t_stop  = {0};

			if (timePos_start.dwYear != 0){
				memcpy((void *)&t_start,(void *)&timePos_start,sizeof(AVI_DMS_TIME));
			}

			RTAviCalStopTime(&t_start, &t_stop);

	        glnkAviPbhandle = RTPlayBackByTime(&t_start, &t_stop);
	        if(glnkAviPbhandle == NULL)
	        {
	            goolink_error("RTPlayBackByTime error.\n");
				usleep(300000);
	    		continue;
	        }

			RTMutex_lock();
			glnk_send_playback_flag = GLINK_PLAYBACK_RESUME_CMD;

			memset(&timePos_start,0,sizeof(AVI_DMS_TIME));
			RTMutex_unlock();

		}

		// open avi file
		if (glnkAviPbhandle != NULL)
		{
			//RTMutex_lock();
			size = 0;
			ret = avi_pb_get_frame(glnkAviPbhandle, NULL, &size);
			//RTMutex_unlock();
			if (ret < 0) {
				goolink_error("call avi_pb_get_frame failed. slide to next file\n");
				ret = avi_pb_slide_to_next_file(glnkAviPbhandle);
				if (ret == 0)
				{
					continue; //继续下一个文件
				}
				else
				{
					RTMutex_lock();
					goolink_error("no more avi files.\n");
					glnk_send_playback_flag = GLINK_PLAYBACK_PAUSE_CMD;//暂停发送,没有录像文件了
					RTMutex_unlock();
					continue;
				}
			}
			RTMutex_lock();
			if(glnk_send_playback_flag != GLINK_PLAYBACK_RESUME_CMD)
			{
				RTMutex_unlock();
				continue;
			}
			read_no     = glnkAviPbhandle->no;
			index_count = glnkAviPbhandle->index_count;
			video_count = glnkAviPbhandle->video_count;
			ffps        = glnkAviPbhandle->fps;

			t1 = read_no / index_count;
			float t2 = t1 * video_count / ffps;
			int  t3 = (int )t2;

			STimeDay fisrtTimeDay;
			memset(&fisrtTimeDay,0,sizeof(fisrtTimeDay));
			RTAviGetTimeDay(glnkAviPbhandle,&fisrtTimeDay);

			u64t getLinkTime = RTAviTimeToTick(&fisrtTimeDay, 0);//s

			fisrtTimeDay.hour   = 0;
			fisrtTimeDay.minute = 0;
			fisrtTimeDay.second = 0;
			u64t timeOfDay   = RTAviTimeToTick(&fisrtTimeDay, 0);//s
			getLinkTime = getLinkTime - timeOfDay ;

			unsigned long long StreamTime = (getLinkTime + t3)*1000;// ms

			static unsigned int print_count = 0;

			if(print_count ++ % 100 == 0)
			{
				u64t count_tmp = (getLinkTime + t3);
			    int h, m , s;
			    h = count_tmp/(60*60);
			    m = (count_tmp - h*(60*60))/60;
			    s = count_tmp - h*(60*60) - m * 60;
			    printf("pb StreamTime:%d-%d-%d\n", h, m, s);
			}
			//goolink_error("StreamTime = %llu\n",StreamTime);
			//uint32_t StreamTime = (fisrtTimeDay.hour * 60 * 60 + fisrtTimeDay.minute * 60 + fisrtTimeDay.second + t3) * 1000;
			// 0: audio; 0x11: I frame, 0x10: P frame
			char channelid = glnk_send_playback_target;
			RTMutex_unlock();

			if (glnkAviPbhandle->node.frame_type == 0x11)// I frame
			{
				ret = glnk_sendplaybackdata(channelid, 1, 0,glnkAviPbhandle->node.one_frame_buf, StreamTime,size);
				if (ret == 0)
				{
					usleep(300000);
					goolink_error("glnk_sendplaybackdata error ret=%d, size=%d\n", ret, size);
				}
			}
			else if (glnkAviPbhandle->node.frame_type == 0x10) // P frame
			{
				ret = glnk_sendplaybackdata(channelid, 0, 0,glnkAviPbhandle->node.one_frame_buf, StreamTime,size);
				if (ret == 0)
				{
					usleep(300000);
					goolink_error("glnk_sendplaybackdata error ret=%d, size=%d\n", ret, size);
				}
			}else if (glnkAviPbhandle->node.frame_type == 0x0) //audio
			{

				alaw_len = audio_alaw_encode(alaw_data, glnkAviPbhandle->node.one_frame_buf, size);
    			ret = glnk_sendplaybackdata(channelid, 1, 1, alaw_data, StreamTime, alaw_len);
				if (ret == 0)
				{
					usleep(300000);
					goolink_error("glnk_sendplaybackdata audio error ret=%d, size=%d\n", ret, size);
				}
			}

		}
		else
		{
			usleep(50000);
			continue;
		}

	}

	RTMutex_lock();
	if (glnkAviPbhandle != NULL)
	{
		avi_pb_close(glnkAviPbhandle);
		glnkAviPbhandle = NULL;
	}

	RTMutex_unlock();

	goolink_printk("GLNK_PlayBack thread exit!\n");
	return 0;
}

static int RTExitPlayBackThread(void)
{
	rtExitPlayBackThreadFlag = 1;
	return 0;
}

static int RTCreateStreamoutThread(void)
{
	pthread_t ThreadVideoFrameId;
	pthread_t ThreadAudioFrameId;


	if(pthread_create(&ThreadVideoFrameId, NULL, &RTVideoFrameThread, NULL))
	{
		goolink_error("create ThreadVideoFrameId failed!\n");
		return -1;
	}


	if(pthread_create(&ThreadAudioFrameId, NULL, &RTAudioFrameThread, NULL))
	{
		goolink_error("create ThreadAudioFrameId failed!\n");
		return -1;
	}

	return 0;
}

static void glink_alarm_push_msg(void)
{
	PushAlarm alarmInfo;
	time_t timep;
	struct tm *p;

	time(&timep);
	struct tm tm = {0};
	p = localtime_r(&timep, &tm);

	memset(&alarmInfo, 0, sizeof(alarmInfo));
	alarmInfo.alarm_type = PAT_MOTION_DETECT;
	alarmInfo.channel	 = 0;
	alarmInfo.timestamp.year   = p->tm_year + 1900;
	alarmInfo.timestamp.month  = p->tm_mon + 1;
	alarmInfo.timestamp.day    = p->tm_mday;
	alarmInfo.timestamp.hour   = p->tm_hour;
	alarmInfo.timestamp.minute = p->tm_min;
	alarmInfo.timestamp.second = p->tm_sec;

	goolinklib_push_md_msg(&alarmInfo);

}

static int glink_alarm_cb_func(int nChannel, int nAlarmType, int nAction, void* pParam)
{
    //goolink_printk("glink_alarm_cb_func: Type = %d\n", nAlarmType);
	switch(nAlarmType)
    {
        case GK_ALARM_TYPE_ALARMIN:     //0:信号量报警开始
			break;

        case GK_ALARM_TYPE_DISK_FULL:       //1:硬盘满
			break;

        case GK_ALARM_TYPE_VLOST:            //2:信号丢失
			break;

        case GK_ALARM_TYPE_VMOTION:          //3:移动侦测
        {
			glink_alarm_push_msg();
            break;
        }
        case GK_ALARM_TYPE_DISK_UNFORMAT:    //4:硬盘未格式化
			break;

        case GK_ALARM_TYPE_DISK_RWERR:       //5:读写硬盘出错,
			break;

        case GK_ALARM_TYPE_VSHELTER:         //6:遮挡报警
			break;

        case GK_ALARM_TYPE_ALARMIN_RESUME:         //9:信号量报警恢复
			break;

        case GK_ALARM_TYPE_VLOST_RESUME:         //10:视频丢失报警恢复
			break;

        case GK_ALARM_TYPE_VMOTION_RESUME:         //11:视频移动侦测报警恢复
			break;

        case GK_ALARM_TYPE_NET_BROKEN:       //12:网络断开
			break;

        case GK_ALARM_TYPE_IP_CONFLICT:      //13:IP冲突
			break;

        default:
			goolink_printk("Alarm: Type = %d", nAlarmType);
        	break;
    }

	return 0;
}


void* goolink_start_thread(void* arg)
{
    int ret = -1;
    int waitSec = (int)arg;
	pthread_t ThreadMainId;
	pthread_t ThreadVoiceInfoId;
	pthread_t ThreadRTWifiId;
	pthread_t ThreadBlueLEDId;
	pthread_t ThreadPlayBackId;

	pthread_detach(pthread_self());
    sleep(waitSec);
	RTMutex_init();

	P2PName_add(MTD_GB28181_INFO);
	ret = RTLoad_p2p_id();
	if (ret != 0)
	{
		goolink_error("goolink_load_p2p_id error.\n");
		return -1;
	}

	if (pthread_create(&ThreadMainId, NULL, &RTMainThread, NULL))
	{
		goolink_error("create ThreadMainId fail!\n");
		return -1;
	}

	if ((strstr(sdk_cfg.name, "CUS_TB") == NULL) && (strstr(sdk_cfg.name, "CUS_ELIFE") == NULL))
	{
		if ((runNetworkCfg.wireless.mode == 0) && (strlen(runNetworkCfg.wifilink[0].essid) == 0)) //ap mode
		{
			if (pthread_create(&ThreadVoiceInfoId, NULL, &RTVoiceInfoThread, NULL))
			{
				goolink_error("create ThreadVoiceWifiId fail!\n");
				return -1;
			}
		}
	}

	if (pthread_create(&ThreadRTWifiId, NULL, &RTWifiThread, NULL))
	{
		goolink_error("create ThreadRTWifiId fail!\n");
		return -1;
	}


	if(strstr(sdk_cfg.name, "CUS_HC_GK7202_GC2053_V10"))
	{
		if (pthread_create(&ThreadBlueLEDId, NULL, &RTBlueLEDThread, NULL))
		{
			goolink_error("create ThreadWifiConnectId fail!\n");
			return -1;
		}
	}

	if(pthread_create(&ThreadPlayBackId, NULL, &RTPlayBackThread, NULL))
	{
		goolink_error("create ThreadPlayBackId failed!\n");
		return -1;
	}

	RTCreateStreamoutThread();

    glnk_cc_switch_init();

    glnk_alarm_handle = event_alarm_open(glink_alarm_cb_func);

	return 0;
}

int goolink_start(void)
{
    int waitSec = 2;
	pthread_t ThreadMainId;
	if (pthread_create(&ThreadMainId, NULL, goolink_start_thread, (void*)waitSec))
	{
		goolink_error("create ThreadMainId fail!\n");
		return -1;
	}
}

int goolink_exit(void)
{
	int ret = -1;

    event_alarm_close(glnk_alarm_handle);

	ret = RTExitVoiceInfoThread();

	ret = RTExitRTBlueLEDThread();

	ret = RTExitPlayBackThread();

	ret = RTExitAudioFrameThread();

	ret = RTExitSendVideoFrameStream();

	ret = glnk_destroy();
	if (ret != 0)
	{
		goolink_error("goolink_exit error!\n");
		return -1;
	}

	return 0;
}

void goolinklib_push_md_msg(PushAlarm *alarmInfo)
{
	PushAlarm stAlarmInfo;
	memset(&stAlarmInfo,0,sizeof(stAlarmInfo));

	stAlarmInfo.channel          = alarmInfo->channel;
	stAlarmInfo.timestamp.year   = alarmInfo->timestamp.year;
	stAlarmInfo.timestamp.month  = alarmInfo->timestamp.month;
	stAlarmInfo.timestamp.day    = alarmInfo->timestamp.day;
	stAlarmInfo.timestamp.hour   = alarmInfo->timestamp.hour;
	stAlarmInfo.timestamp.minute = alarmInfo->timestamp.minute;
	stAlarmInfo.timestamp.second = alarmInfo->timestamp.second;

	stAlarmInfo.alarm_type = alarmInfo->alarm_type;

    /*
	printf("yjw function(%s),alarm_type(%d),channel(%d),in %d-%d-%d-%d-%d-%d!!!\n", __FUNCTION__,
		stAlarmInfo.alarm_type,
		stAlarmInfo.channel,
		stAlarmInfo.timestamp.year,
		stAlarmInfo.timestamp.month,
		stAlarmInfo.timestamp.day,
		stAlarmInfo.timestamp.hour,
		stAlarmInfo.timestamp.minute,
		stAlarmInfo.timestamp.second);
	*/

	glnk_push_alarm(stAlarmInfo);
}



unsigned char GLNK_PwdAuth_Callback(char* username, char* pwd)
{
	goolink_printk("%s:%d username:%s  password:%s | username:%s  password:%s\n",
		__func__,__LINE__,username,pwd,runUserCfg.user[0].userName,runUserCfg.user[0].password);

	RTMutex_lock();
	if( strlen(runUserCfg.user[0].userName) == strlen(username)
    && strlen(runUserCfg.user[0].password) == strlen(pwd))
    {
        if(strcmp(runUserCfg.user[0].userName, username) == 0
    	&& strcmp(runUserCfg.user[0].password, pwd) == 0)
    	{
    		return 1;
    	}
    }
	RTMutex_unlock();

	return 0;
}

void GLNK_GetDevInfo_Callback(GLNK_V_DeviceInfo* devinfo)
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
	memset(devinfo->companyId,0,sizeof(devinfo->companyId));
	strncpy(devinfo->companyId, runSystemCfg.deviceInfo.manufacturer, sizeof(devinfo->companyId)-1);

	memset(devinfo->productId,0,sizeof(devinfo->productId));
	strncpy(devinfo->productId, runSystemCfg.deviceInfo.deviceType, sizeof(devinfo->productId)-1);

	memset(devinfo->name,0,sizeof(devinfo->name));
	strncpy(devinfo->name, runSystemCfg.deviceInfo.deviceName, sizeof(devinfo->name)-1);

	memset(devinfo->softwareVersion,0,sizeof(devinfo->softwareVersion));
	strncpy(devinfo->softwareVersion, runSystemCfg.deviceInfo.softwareVersion, sizeof(devinfo->softwareVersion)-1);

	devinfo->manufactureDate.year  = 2015;
	devinfo->manufactureDate.month = 9;
	devinfo->manufactureDate.day   = 28;

	devinfo->channelNum = 1;
	devinfo->alarmType  = 0;
	devinfo->deviceType = 0x11;


	goolink_printk("\n####GLNK_GetDevInfo_Callback:%s,%s,%s,%s\n",devinfo->companyId, devinfo->productId,devinfo->name,devinfo->softwareVersion);
}

int32_t GLNK_RTVideoOpen_Callback(unsigned char channel, unsigned char ismainorsub, GLNK_VideoDataFormat* videoinfo)
{

	struct video_channel_info* vci;
	if (rtExitVideoFrameThreadFlag == 1)
	{
		goolink_error("rtExitVideoFrameThreadFlag is true !\n");
#if 0
		goolink_printk("&&&&&&&&&&&&&&&&&&& reboot sys &&&&&&&&&&&&&\n");
		new_system_call("reboot -f");//force REBOOT
		netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT
#endif
		return 0;
	}

	goolink_printk("%s:%d channel=%d ismainorsub=%d\n",__func__, __LINE__, channel, ismainorsub);
	if (ismainorsub < GK_NET_STREAM_TYPE_STREAM0 || ismainorsub > 1)
	{
		goolink_error("channel error %s:%d channel=%d ismainorsub=%d\n",__func__, __LINE__, channel, ismainorsub);
		return 0;
	}

	if (channel > 3)
		return 0;

	RTMutex_lock();
	if (ismainorsub){
		vci = &video_stream_sub[channel];
		video_stream_main[channel].enable = 0;
	}
	else
	{
		vci = &video_stream_main[channel];
		video_stream_sub[channel].enable = 0;
	}

	vci->chn     = channel;
	vci->isms    = ismainorsub;
	vci->enable  = 1;
    netcam_md_play_start_one();
	RTMutex_unlock();

	if (vci->mhandle != NULL)
	{
		mediabuf_set_newest_frame(vci->mhandle);
		int stream_id = ismainorsub;
		netcam_video_force_i_frame(stream_id);
	}


    if (1 == runVideoCfg.vencStream[ismainorsub].enctype)
    {
        videoinfo->codec = GLNK_CODEC_H264;
    }
    else if (3 == runVideoCfg.vencStream[ismainorsub].enctype)
    {
        videoinfo->codec = GLNK_CODEC_H265;
    }
    else
    {
        videoinfo->codec = GLNK_CODEC_H265;
    }
	videoinfo->bitrate       = runVideoCfg.vencStream[ismainorsub].h264Conf.bps;
	videoinfo->width         = runVideoCfg.vencStream[ismainorsub].h264Conf.width;
	videoinfo->height        = runVideoCfg.vencStream[ismainorsub].h264Conf.height;
	videoinfo->framerate     = runVideoCfg.vencStream[ismainorsub].h264Conf.fps;
	videoinfo->colorDepth    = 24;
	videoinfo->frameInterval = runVideoCfg.vencStream[0].h264Conf.gop;

    //if ()

	return 1;
}

int32_t GLNK_RTVideoClose_Callback(unsigned char channel, unsigned char ismainorsub)
{
	struct video_channel_info* vci;

	goolink_printk("%s:%d channel=%d ismainorsub=%d\n", __func__, __LINE__, channel, ismainorsub);
	if (ismainorsub < GK_NET_STREAM_TYPE_STREAM0 || ismainorsub > 1)
	{
		goolink_error("channel error %s:%d channel=%d ismainorsub=%d\n",__func__, __LINE__, channel, ismainorsub);
		return 0;
	}

	if (channel > 3)
		return 0;

	RTMutex_lock();

	if (ismainorsub)
		vci = &video_stream_sub[channel];
	else
		vci = &video_stream_main[channel];
    
    netcam_md_play_stop_one();
	//vci->enable  = 0;
	//gStopLoolink = 0;
	RTMutex_unlock();

	return 1;
}

int32_t GLNK_AudioEncodeOpen_CallBack(unsigned char channel, GLNK_AudioDataFormat* audioinfo)
{
	if (rtExitAudioFrameThreadFlag == 1)
	{
		goolink_error("rtExitVideoFrameThreadFlag is true !\n");
#if 0
		goolink_printk("&&&&&&&&&&&&&&&&&&& reboot sys &&&&&&&&&&&&&\n");
		new_system_call("reboot -f");//force REBOOT
		netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT
#endif
		return 1;
	}

	RTMutex_lock();
	struct audio_channel_info* aci = &audio_stream_enable;

	goolink_printk("%s channel=%d\n",__func__,channel);
	aci->chn = channel;
	aci->enable = 1;
	if (aci->mhandle != NULL)
	{
		mediabuf_set_newest_frame(aci->mhandle);
	}

	audioinfo->samplesRate   = runAudioCfg.sampleRate;//GADI_AUDIO_SAMPLE_RATE_16000;
	audioinfo->bitrate       = 16;
	audioinfo->waveFormat    = GLNK_WAVE_FORMAT_G711;
	audioinfo->channelNumber = 1;
	audioinfo->blockAlign    = 1;
	audioinfo->bitsPerSample = 16;
	audioinfo->frameInterval = 25;
	audioinfo->reserve       = 0;
	RTMutex_unlock();

	return 1;
}
int32_t GLNK_AudioEncodeClose_CallBack(unsigned char channel)
{
	goolink_printk("%s channel=%d\n",__func__,channel);
	RTMutex_lock();
	audio_stream_enable.enable = 0;
	RTMutex_unlock();

	return 1;
}

int32_t GLNK_AudioDecodeOpen_CallBack(unsigned char channel, GLNK_AudioDataFormat* audioinfo)
{
    goolink_printk("%s channel=%d\n",__func__,channel);

    audioinfo->samplesRate   = GK_AUDIO_SAMPLE_RATE_8000;
	audioinfo->bitrate       = 16;
	audioinfo->waveFormat    = GLNK_WAVE_FORMAT_G711;
	audioinfo->channelNumber = 1;
	audioinfo->blockAlign    = 1;
	audioinfo->bitsPerSample = 16;
	audioinfo->frameInterval = 25;
	audioinfo->reserve       = 0;

	return 1;
}

int32_t GLNK_AudioDecode_CallBack(unsigned char channel, char* buffer, uint32_t length)
{

	netcam_audio_output(buffer, length, NETCAM_AUDIO_ENC_A_LAW, SDK_TRUE);
	return 1;
}

int32_t GLNK_AudioDecodeClose_CallBack(unsigned char channel)
{
	goolink_printk("%s channel=%d\n",__func__,channel);
	return 1;
}

int32_t GLNK_PTZOpen_Callback(uint32_t channel)
{
	goolink_printk("%s channel=%d\n",__func__,channel);
	return 1;
}

int32_t GLNK_PTZCmd_Callback(GLNK_PTZControlCmd ptzcmd, uint32_t channel, GLNK_ControlArgData* arg)
{
	goolink_printk("PTZ channel[%d] ptzcmd[%d] ", channel, (int)ptzcmd);
	goolink_printk("arg1 = %d  arg2 = %d  arg3 = %d  arg4 = %d \n", arg->arg1, arg->arg2, arg->arg3, arg->arg4);

	int ptz_step = 10,ptz_speed = 10;

	if(strstr(sdk_cfg.name, "CUS_HC_GK7202_GC2053_V10"))
	{
		ptz_step = 30;
		ptz_speed = 30;
	}

	switch(ptzcmd)
	{
		case GLNK_PTZ_MV_STOP :
		{
			netcam_ptz_stop();
			break;
		}
		case GLNK_PTZ_ZOOM_DEC :
		{
			printf("zoom decrease!\n");
            //sdk_isp_set_af_zoom(2);
			break;
		}
		case GLNK_PTZ_ZOOM_INC :
		{
			printf("zoom increase!\n");
            //sdk_isp_set_af_zoom(1);
			break;
		}
		case GLNK_PTZ_FOCUS_INC :
		{
			printf("focus increase!\n");
            //sdk_isp_set_af_focus(1);
			break;
		}
		case GLNK_PTZ_FOCUS_DEC :
		{
			printf("focus decrease!\n");
            //sdk_isp_set_af_focus(2);
			break;
		}
		case GLNK_PTZ_MV_UP :
		{
            netcam_ptz_up(ptz_step, ptz_speed);
            CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
			break;
		}
		case GLNK_PTZ_MV_DOWN :
		{
            netcam_ptz_down(ptz_step, ptz_speed);
            CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
			break;
		}
		case GLNK_PTZ_MV_LEFT :
		{
            netcam_ptz_left(ptz_step, ptz_speed);
            CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
			break;
		}
		case GLNK_PTZ_MV_RIGHT :
		{
            netcam_ptz_right(ptz_step, ptz_speed);
            CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
			break;
		}
		case GLNK_PTZ_IRIS_INC :
		{
			printf("iris increase!\n");
			break;
		}
		case GLNK_PTZ_IRIS_DEC :
		{
			printf("iris decrease!\n");
			break;
		}
		case GLNK_PTZ_AUTO_CRUISE :
		{
			netcam_ptz_hor_ver_cruise(ptz_speed);
			break;
		}
		case GLNK_PTZ_GOTO_PRESET :
		{
			GK_NET_CRUISE_GROUP  cruise_info;
			if (arg->arg1 < 0 || arg->arg1 > PTZ_MAX_PRESET)
                break;

            cruise_info.byPointNum    = 1;
            cruise_info.byCruiseIndex = 0;
            cruise_info.struCruisePoint[0].byPointIndex = 0;
            cruise_info.struCruisePoint[0].byPresetNo   = arg->arg1;
            cruise_info.struCruisePoint[0].byRemainTime = 0;
            cruise_info.struCruisePoint[0].bySpeed      = ptz_speed;
            netcam_ptz_stop();

            if (netcam_ptz_preset_cruise(&cruise_info))
            {
                goolink_error("FUN[%s]  LINE[%d]  call netcam_ptz_preset_cruise error!\n", __FUNCTION__, __LINE__);
                return -1;
            }
			break;
		}
		case GLNK_PTZ_SET_PRESET :
		{
			GK_NET_PRESET_INFO gkPresetCfg;
			if (arg->arg1 < 0 || arg->arg1 > PTZ_MAX_PRESET)
                break;

            get_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            gkPresetCfg.nPresetNum++;
            if (gkPresetCfg.nPresetNum >= NETCAM_PTZ_MAX_PRESET_NUM)
            {
                gkPresetCfg.nPresetNum = NETCAM_PTZ_MAX_PRESET_NUM;
            }
            gkPresetCfg.no[gkPresetCfg.nPresetNum-1] = arg->arg1;
            set_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            PresetCruiseCfgSave();
            if (netcam_ptz_set_preset(arg->arg1, NULL))
            {
                goolink_error("FUN[%s]  LINE[%d]  call netcam_ptz_set_preset error!\n", __FUNCTION__, __LINE__);
                return -1;
            }
			break;
		}
		case GLNK_PTZ_CLEAR_PRESET :
		{
			int i;
			GK_NET_PRESET_INFO gkPresetCfg;
			if (arg->arg1 == 0xFFFFFFFF)
			{
				for (i = 0; i < PTZ_MAX_PRESET; i++)
					netcam_ptz_clr_preset(i);
				memset(&gkPresetCfg, 0, sizeof(GK_NET_PRESET_INFO));
				gkPresetCfg.nPresetNum = PTZ_MAX_PRESET;
				set_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
				PresetCruiseCfgSave();
			}
			else if (arg->arg1 >= 0 && arg->arg1 <= PTZ_MAX_PRESET)
			{
            	get_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            	gkPresetCfg.nPresetNum--;
            	if (gkPresetCfg.nPresetNum <= 0)
            	{
                	gkPresetCfg.nPresetNum = 0;
            	}
            	set_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            	PresetCruiseCfgSave();
            	netcam_ptz_clr_preset(arg->arg1);
			}

			break;
		}
		case GLNK_PTZ_ACTION_RESET :
		{
			goolink_error("action reset!\n");
			break;
		}
		case GLNK_PTZ_MV_LEFTUP :
		{
			netcam_ptz_left_up(ptz_step, ptz_speed);
            CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
			break;
		}
		case GLNK_PTZ_MV_LEFTDOWN :
		{
			netcam_ptz_left_down(ptz_step, ptz_speed);
            CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
			break;
		}
		case GLNK_PTZ_MV_RIGHTUP :
		{
			netcam_ptz_right_up(ptz_step, ptz_speed);
            CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
			break;
		}
		case GLNK_PTZ_MV_RIGHTDOWN :
		{
			netcam_ptz_right_down(ptz_step, ptz_speed);
            CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
			break;
		}
		case GLNK_PTZ_CLEAR_TOUR :
		{
			goolink_error("clear tour!\n");
			break;
		}
		case GLNK_PTZ_ADD_PRESET_TO_TOUR :
		{
			printf("preset tour!\n");
			break;
		}
		case GLNK_PTZ_DEL_PRESET_TO_TOUR :
		{
			printf("delete tour!\n");
			break;
		}
		default:
			break;
	}

	return 1;
}

int32_t GLNK_PTZClose_Callback(uint32_t channel)
{
	goolink_printk("%s channel=%d\n",__func__,channel);
	return 1;
}

int32_t GLNK_ResetUsrPsword_Callback(char* username, char* oldpwd, char* newpwd)
{
	goolink_printk(" %s:%d username:%s	oldpwd:%s  newpwd:%s \n",__func__,__LINE__,username,oldpwd,newpwd);

	if( strlen(runUserCfg.user[0].userName) == strlen(username)
    && strlen(runUserCfg.user[0].password) == strlen(oldpwd))
    {
        if(strcmp(runUserCfg.user[0].userName, username) == 0
    	&& strcmp(runUserCfg.user[0].password, oldpwd) == 0)
    	{
    		strncpy(runUserCfg.user[0].password, newpwd, sizeof(runUserCfg.user[0].password));
			UserCfgSave();
    		return 1;
    	}
    }

	return 0;
}
/*
int32_t GLNK_SetRecordConfigure_Callback(GLNK_V_RecordChgRequest *Req)
{
	printf(" %s:%d Req->command = %d \n", __func__,__LINE__, Req->command);

	switch (Req->command)
	{
	case GLNK_RECORD_CLOSE:
		runRecordCfg.recordMode = 3;
		break;

	case GLNK_RECORD_OPEN:
		runRecordCfg.recordMode = 1;
		break;

	case GLNK_RECORD_OPEN_AUTO_BY_TIME:
		runRecordCfg.recordMode = 0;
		break;

	default:
		break;
	}

	if(Req->command == 2)
	{
		printf("startTime = %d-%d-%d %d:%d:%d  \n",Req->startTime.m_year, Req->startTime.m_month, Req->startTime.m_day,
			Req->startTime.m_hour, Req->startTime.m_minute, Req->startTime.m_second);
		printf("endTime = %d-%d-%d %d:%d:%d  \n",Req->endTime.m_year, Req->endTime.m_month, Req->endTime.m_day,
		Req->endTime.m_hour, Req->endTime.m_minute, Req->endTime.m_second);
	}
	return 1 ;
}
*/

int32_t GLNK_SearchWifi_Callback(char ** buf)
{
	goolink_printk(" %s:%d \n",__func__,__LINE__);

	GLNK_V_WifiInfo *WifiInfo;
	char *WifiName = NULL;
	WIFI_SCAN_LIST_t list[20];
	int i, number = 20;

	memset(list, 0, sizeof(list));
	WifiName = netcam_net_wifi_get_devname();
	if(WifiName == NULL)
	{
		goolink_error("WifiName is NULL\n");
		return 0;
	}

	if(netcam_net_wifi_isOn() != 1)
	{
		goolink_error("Wifi is not enable\n");
		return 0;
	}

	netcam_net_wifi_on();
	if(netcam_net_wifi_get_scan_list(list, &number) != 0)
	{
		goolink_error("Fail to get wifi list\n");
		return 0;
	}

	WifiInfo = (GLNK_V_WifiInfo *)calloc(number, sizeof(GLNK_V_WifiInfo));
	if (!WifiInfo)
		return 0;

	for(i = 0; i < number && list[i].vaild; i++)
	{
		strncpy(WifiInfo[i].name, list[i].essid, sizeof(WifiInfo[i].name));
		strncpy(WifiInfo[i].ssid, list[i].essid, sizeof(WifiInfo[i].ssid));
		WifiInfo[i].level = list[i].quality;
	}

	*buf = (char *)WifiInfo;

	return number;

}

int32_t GLNK_WifiConfig_Callback(GLNK_V_WifiConfigRequest *Req)
{
    int ret =-1;
    goolink_printk("GLNK_WifiConfig_Callback >>>>>>>>> ------start configure wifi---------------------\n");
    goolink_printk("GLNK_WifiConfig_Callback, uuid=%s,ssid=%s,password=%s,networkType=%d,encryptType=%d\n",
        Req->name,Req->ssid,Req->password,Req->networkType,Req->encryptType);

	if( (runNetworkCfg.wireless.mode == 1) && (strlen((char*)runNetworkCfg.wifilink[0].essid) != 0) ) //sta mode change network
	{
		struct device_info_mtd info ;
		int ret = load_info_to_mtd_reserve(MTD_GB28181_INFO,&info, sizeof(struct device_info_mtd));
		if(ret != 0)
		{
		    goolink_error("Get reserver info error\n");
			return 0;
		}

		//if(strcmp(info.wifi_psd,Req->password) || strcmp(info.wifi_ssid,Req->ssid))
		if(strcmp(info.wifi_ssid,Req->ssid))
    	{
			goolink_printk("Wifi info old [ssid][psd][type] [%s]:[%s]:[%d]",info.wifi_ssid,info.wifi_psd,info.wifi_enc_type);
			goolink_printk("Wifi info new [ssid][psd] [%s]:[%s]:[%d]",Req->ssid,Req->password,Req->encryptType);

			RTMutex_lock();
			memset(&RTWiFiLinkInfo,0,sizeof(RTWiFiLinkInfo));
			strcpy(RTWiFiLinkInfo.linkEssid,Req->ssid);
			strcpy(RTWiFiLinkInfo.linkPsd,Req->password);

			RTWiFiLinkInfo.linkScurity =  WIFI_ENC_WPAPSK_AES;//Req->encryptType;


			if (rtwifi_sta_mode_connect_enable == 0)
				rtwifi_sta_mode_connect_enable = 1;

			rtwifi_sta_mode_change_enable = 1;

			RTMutex_unlock();
    	}
		else if ((strcmp(info.wifi_ssid,Req->ssid) == 0) )
    	{
			// in STA mode ,if the wifi name does not change,
			// the network will not be switched.
			return 1;

		}


	}
	else if (runNetworkCfg.wireless.mode == 0) // AP config network
	{
		RTMutex_lock();
		memset(&RTWiFiLinkInfo,0,sizeof(RTWiFiLinkInfo));
		strcpy(RTWiFiLinkInfo.linkEssid,Req->ssid);
		strcpy(RTWiFiLinkInfo.linkPsd,Req->password);
		RTWiFiLinkInfo.linkScurity =  WIFI_ENC_WPAPSK_AES;//Req->encryptType;

		if (rtwifi_sta_mode_connect_enable == 0)
			rtwifi_sta_mode_connect_enable = 1;

		RTMutex_unlock();
	}

    return 1; //永远成功
}


/*
int32_t GLNK_OpenDownLoadRecord_Callback(char* recordname, int32_t *recordfd, char mode, uint32_t offset)
{
	FILE *fd;

	fd = fopen(recordname, "r");
	if(fd == NULL )
	{
		printf( "openfile [%s] error [%s]\n", recordname,strerror(errno));
		return 0;
	}

	if(mode == 2)
		fseek(fd, offset, SEEK_SET);

	*recordfd = (int32_t)fd;
	printf("%s, opening file %s fd=%d/%d mode=%d, offset=%d\n",__func__,recordname, *recordfd, (int32_t)fd, mode, offset);
	return 1;
}

int32_t GLNK_CloseDownLoadRecord_Callback(int32_t recordfd)
{
	printf("%s, filefd[%d]\n",__func__,recordfd);
	fclose((FILE *)recordfd);
	return 1;

}

int32_t GLNK_ReadDownLoadRecord_Callback(int32_t recordfd, char* data, int32_t datalen, int32_t *start_pos, int32_t *end_pos)
{
	printf("%s, filefd[%d]  datalen = %d\n",__func__,recordfd, datalen);
	int32_t ret;
	FILE* fd = (FILE *)recordfd;

	*start_pos = ftell(fd);
	ret = fread(data, 1, datalen, fd);
	if(ret != datalen)
	{
		printf("%s, read fail! ret=%d\n",__func__, ret);
		return 0;
	}

	*end_pos = ftell(fd);
	return 1;

}


int32_t GLNK_GetVide_AudioConfig_Callback(int channel, TLV_V_VA_GET_CONFIG_RSP *rsq)
{
	printf("%s, channel[%d]\n",__func__,channel);
	rsq->channel = channel;
	rsq->isOpenAudio = 0;
	rsq->mainStreamBitRate = runVideoCfg.vencStream[0].h264Conf.bps;
	rsq->mainStreamFrameRate = runVideoCfg.vencStream[0].h264Conf.fps;

	if (runVideoCfg.vencStream[0].h264Conf.height == 1080)
		rsq->mainStreamResolution = RESOLUTION_1080P;
	else
		rsq->mainStreamResolution = RESOLUTION_720P;

	rsq->subStreamBitRate = runVideoCfg.vencStream[1].h264Conf.bps;
	rsq->subStreamFrameRate = runVideoCfg.vencStream[1].h264Conf.fps;

	if (runVideoCfg.vencStream[1].h264Conf.height == 480)
		rsq->subStreamResolution = RESOLUTION_VGA;
	else
		rsq->subStreamResolution = RESOLUTION_CIF;
	return 1;
}

static int Stream_Width_Height[8][2] =
{
	{1920, 1080},
	{1280, 720},
	{720, 576},
	{640, 480},

	{360, 288},
	{320, 240},
	{180, 144},
	{160, 120}
};
int32_t GLNK_SetVide_AudioConfig_Callback(int32_t *result, TLV_V_VA_SET_CONFIG_REQ *req)
{
	ST_GK_ENC_STREAM_H264_ATTR h264Attr;

	printf("%s, result[%d]\n",__func__,*result);
	printf("channel = %d\n", req->channel);
	printf("isOpenAudio = %d\n", req->isOpenAudio);
	printf("mainStreamBitRate = %d\n", req->mainStreamBitRate);
	printf("mainStreamFrameRate = %d\n", req->mainStreamFrameRate);
	printf("mainStreamFrameRate = %d\n", req->channel);
	printf("mainStreamResolution = %d\n", req->mainStreamResolution);
	printf("subStreamBitRate = %d\n", req->subStreamBitRate);
	printf("subStreamFrameRate = %d\n", req->subStreamFrameRate);
	printf("subStreamResolution = %d\n", req->subStreamResolution);

	netcam_video_get(0, 0, &h264Attr);
	h264Attr.bps = req->mainStreamBitRate;
	h264Attr.fps = req->mainStreamFrameRate;
	h264Attr.width = Stream_Width_Height[req->mainStreamResolution][0];
	h264Attr.height = Stream_Width_Height[req->mainStreamResolution][1];
	netcam_video_set(0, 0, &h264Attr);

	netcam_video_get(0, 1, &h264Attr);
	h264Attr.bps = req->subStreamBitRate;
	h264Attr.fps = req->subStreamFrameRate;
	h264Attr.width = Stream_Width_Height[req->subStreamResolution][0];
	h264Attr.height = Stream_Width_Height[req->subStreamResolution][1];
	netcam_video_set(0, 1, &h264Attr);

	CREATE_WORK(SetVideo, EVENT_TIMER_WORK, (WORK_CALLBACK)VideoCfgSave);
	INIT_WORK(SetVideo, COMPUTE_TIME(0,0,0,1,0), NULL);
	SCHEDULE_DEFAULT_WORK(SetVideo);

	*result = 1;
	return 1;
}

int32_t GLNK_SetNetWorkConfig_Callback(int32_t *result, TLV_V_Network *req)
{
	ST_SDK_NETWORK_ATTR net_attr;

	printf("%s, result[%d]\n",__func__,*result);
	printf("deviceId = %d\n", req->deviceId);
	printf("hostIP = %s\n", req->hostIP);
	printf("hostName = %s\n", req->hostName);
	printf("gateway = %s\n", req->gateway);
	printf("dnsServer = %s\n", req->dnsServer);
	printf("dnsServer2 = %s\n", req->dnsServer2);
	printf("subnetMask = %s\n", req->subnetMask);

	printf("tcpPort = %s\n", req->wifiHostIP);
	printf("hostName = %s\n", req->wifiHostName);
	printf("gateway = %s\n", req->wifiGateway);
	printf("dnsServer = %s\n", req->wifiDnsServer);
	printf("dnsServer2 = %s\n", req->wifiDnsServer2);
	printf("subnetMask = %s\n", req->wifiSubnetMask);
	printf("mac = %s\n", req->mac);
	printf("mac = %s\n", req->wifiMac);
	printf("IPMode = %d\n", req->IPMode);
	printf("IPMode = %d\n", req->wifiIPMode);

	net_attr.enable = 1;
    netcam_net_get(&net_attr);
    if(runNetworkCfg.lan.enable)
    {
        sprintf(net_attr.name,"eth0");

		strncpy(net_attr.ip,(char*)req->hostIP,sizeof(net_attr.ip));
        strncpy(net_attr.mask,(char*)req->subnetMask,sizeof(net_attr.mask));
        strncpy(net_attr.gateway,(char*)req->gateway,sizeof(net_attr.gateway));
        strncpy(net_attr.dns1,(char*)req->dnsServer,sizeof(net_attr.dns1));
        strncpy(net_attr.dns2,(char*)req->dnsServer2,sizeof(net_attr.dns2));
		memcpy(net_attr.mac,req->mac,sizeof(req->mac));
    }
    else
    {
        sprintf(net_attr.name,"ra0");

		strncpy(net_attr.ip,(char*)req->wifiHostIP,sizeof(net_attr.ip));
        strncpy(net_attr.mask,(char*)req->wifiSubnetMask,sizeof(net_attr.mask));
        strncpy(net_attr.gateway,(char*)req->wifiGateway,sizeof(net_attr.gateway));
        strncpy(net_attr.dns1,(char*)req->wifiDnsServer,sizeof(net_attr.dns1));
        strncpy(net_attr.dns2,(char*)req->wifiDnsServer2,sizeof(net_attr.dns2));
		memcpy(net_attr.mac,req->wifiMac,sizeof(req->wifiMac));
    }

    if(netcam_net_set(&net_attr) == 0)
        netcam_net_cfg_save();

	*result = 1;
	return 1;

}

int32_t GLNK_GetNetWorkConfig_Callback(TLV_V_Network *req)
{
	printf("%s\n",__func__);

	req->deviceId = 0;

	strncpy((char*)req->hostName, runNetworkCfg.lan.netName, sizeof(req->hostName));
	strncpy((char*)req->hostIP, runNetworkCfg.lan.ip, sizeof(req->hostIP));
    strncpy((char*)req->subnetMask, runNetworkCfg.lan.netmask, sizeof(req->subnetMask));
    strncpy((char*)req->gateway, runNetworkCfg.lan.gateway, sizeof(req->gateway));
	strncpy((char*)req->dnsServer, runNetworkCfg.lan.dns1, sizeof(req->dnsServer));
	strncpy((char*)req->dnsServer2, runNetworkCfg.lan.dns2, sizeof(req->dnsServer2));
	strncpy((char*)req->mac, runNetworkCfg.lan.mac, sizeof(req->mac));

	strncpy((char*)req->wifiHostName, runNetworkCfg.wifi.netName, sizeof(req->wifiHostName));
	strncpy((char*)req->wifiHostIP, runNetworkCfg.wifi.ip, sizeof(req->wifiHostIP));
    strncpy((char*)req->wifiSubnetMask, runNetworkCfg.wifi.netmask, sizeof(req->wifiSubnetMask));
    strncpy((char*)req->wifiGateway, runNetworkCfg.wifi.gateway, sizeof(req->wifiGateway));
	strncpy((char*)req->wifiDnsServer, runNetworkCfg.wifi.dns1, sizeof(req->wifiDnsServer));
	strncpy((char*)req->wifiDnsServer2, runNetworkCfg.wifi.dns2, sizeof(req->wifiDnsServer2));
	strncpy((char*)req->wifiDnsServer2, runNetworkCfg.wifi.mac, sizeof(req->wifiDnsServer2));

	req->wifiIPMode = 1;
	req->IPMode = 2;
	return 1;
}*/

static char* upgrade_buffer     = NULL;
static int upgrade_buffer_len   = 0;
static int upgrade_buffer_count = 0;
static int upgrade_start_count  = 0;
static int upgrade_create_count = 0;

static void *goolink_update_thread(void *lParam)
{
    sdk_sys_thread_set_name("goolinkUpdateThread");
    pthread_detach(pthread_self()); //一定要回收, 否则反复调用有泄露
	goolink_printk("start goolinkUpdateThread ok\n");
	int ret = 0;
	int cnt = 0;
	int len = 0;

    //为了让回调快速返回, 这里新建一个线程来执行升级动作
    sleep(2); //等一会儿, 让回调返回, 先让它报信回去, 让对方收到了确切消息我们再执行

    //这里应该加锁互斥下一次升级, 下一次升级会不会干扰到这里来
	RTMutex_lock();

	// check update file
    ret = netcam_update(upgrade_buffer, upgrade_buffer_len, NULL);
    if (ret < 0)
	{
		goolink_error("Check updating package, failed: %d \n", ret);
		RTMutex_unlock();
        goto UPDATE_EXIT;
	}
	RTMutex_unlock();

    cnt = 0;
    while (cnt < 99)
    {
        cnt = netcam_update_get_process();
        usleep(218 * 1000);
        goolink_printk("... upgrading cnt:%d\n",cnt);
    }

	//netcam_audio_hint(SYSTEM_AUDIO_HINT_UPDATE_SUCCESS);

UPDATE_EXIT:

	if(upgrade_buffer)
		free(upgrade_buffer);

	usleep(100 * 1000);

    if(ret < 0)
    {
		goolink_printk("&&&&&&&&&&&&&&&&&&& reboot sys &&&&&&&&&&&&&\n");
		#if 0
		netcam_exit(90);
		new_system_call("reboot -f");//force REBOOT
		#endif
		netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT
    }

	return 0;
}

int32_t GLNK_DeviceUpDate_Callback(int32_t type, char *buffer, int32_t length)
{
	goolink_printk("%s:%d  type = %d   \n",__func__, __LINE__,type);

	int ret = 0;

	//正在升级，退出
	if((netcam_get_update_status() != 0) && (upgrade_start_count > 0))
	{
		goolink_printk("system is updating, don't call update again...,upgrade_start_count=%d\n",upgrade_start_count);
		return 0;
	}

	if(type == 1) //start to updata
	{
		goolink_printk("1. start to updata  type = %d , length = %d\n",type,length);
		if (length <= 0)
		{
			goolink_error("Upgrade file length= %d, is zero, no upgrade\n",length);
			return 0;
		}

		if (upgrade_start_count > 0)
		{
			goolink_error("upgrade_start_count= %d,  no upgrade to reboot sys\n",upgrade_start_count);
			goto GOOLINK_UPGRADE_EXIT;
		}

		upgrade_start_count++;
		netcam_update_relase_system_resource();

		if (upgrade_buffer == NULL)
		{
			upgrade_buffer = (char*)netcam_update_malloc(length);
			if (NULL == upgrade_buffer)
			{
				goolink_error("malloc length=%u, upgrade_buffer failed!\n",length);
				goto GOOLINK_UPGRADE_EXIT;

			}
		}
		else
		{
			goolink_error(" upgrade_buffer:0x%x,haved mallo upgrade_buffer failed!\n",upgrade_buffer);
			goto GOOLINK_UPGRADE_EXIT;
		}

		upgrade_buffer_len = length;

	}
	else if(type == 2) //wirte the buffer(download file,pear 200K)
	{
		goolink_printk(" 2. wirte the buffer type = %d  length = %u\n",type,length);
		if ((NULL == upgrade_buffer) || (NULL == buffer) ||
			(upgrade_buffer_count > upgrade_buffer_len))
		{
			goolink_error("upgrade_buffer_count :%d  > upgrade_buffer_len:%d,upgrade buffer failed!\n",
												upgrade_buffer_count,upgrade_buffer_len);
			goto GOOLINK_UPGRADE_EXIT;

		}

		memcpy(upgrade_buffer + upgrade_buffer_count, buffer, length);
		upgrade_buffer_count += length;
		goolink_printk("get content. len:%d\n", upgrade_buffer_count);

	}
	else if(type == 3) //finish download
	{
		if ((upgrade_buffer_count != upgrade_buffer_len) || (upgrade_create_count > 0))
		{
			goolink_error("buffer_count :%d  != buffer_len:%d, create_count= %d,upgrade buffer failed!\n",
												upgrade_buffer_count,upgrade_buffer_len,upgrade_create_count);
			goto GOOLINK_UPGRADE_EXIT;
		}

		goolink_printk("3.upgrade_buffer_len: %d,type:%d\n", upgrade_buffer_len,type);

        //按照他们的要求, 要创建一个新线程来执行这个动作, 让回调快速返回
        pthread_t m_hUpdateThread;
		upgrade_create_count++;
        if(pthread_create(&m_hUpdateThread, NULL, goolink_update_thread, NULL))
        {
			goolink_error("GLNK_DeviceUpDate_Callback, create goolink_update_thread failed !!!!! \n");
			goto GOOLINK_UPGRADE_EXIT;
        }

	}
	else if(type == 4)// updatafail
	{
		goolink_printk(" updatafail  type = %d  length = %d\n",type,length);
		goto GOOLINK_UPGRADE_EXIT;
	}

	return 1;

GOOLINK_UPGRADE_EXIT:
	 if(upgrade_buffer)
		 free(upgrade_buffer);

	// netcam_audio_hint(SYSTEM_AUDIO_HINT_UPDATE_FAILED);
	 usleep(500 * 1000);
	 goolink_printk("&&&&&&&&&&&&&&&&&&& reboot sys &&&&&&&&&&&&&\n");
	 #if 0
	 netcam_exit(90);
	 new_system_call("reboot -f");//force REBOOT
	 #endif
	 netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT

	 return 0;
}

static void glnk_parse_update_version(const char* ver, char* buf, int len)
{
	char* p = strrchr(ver, '.');
	if (!p)
	{
		strncpy(buf, "null", len);
		return;
	}

	strncpy(buf, p+1, len);
	buf[8] = '\0';
}

int32_t GLNK_GetVersionFirmware_Callback(char* appbuf, char* solbuf, char* date, char* hardware)
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
    glnk_parse_update_version(g_systemInfo.upgrade_version, date, 20);
#if 0
	snprintf(appbuf, 20, "HCIPC");
	snprintf(solbuf, 20, "GK");
#ifdef USE_CUSTOM_DEVICE_INFO
    snprintf(hardware, 64, "7202-2053-V1.0");
#else
	snprintf(hardware, 64, "%s-%s", g_systemInfo.device_type, g_systemInfo.sensor_type);
#endif
#else


	strncpy(appbuf, runSystemCfg.deviceInfo.manufacturer, VERSION_FILED_MAX_SIZE);
	snprintf(solbuf, VERSION_FILED_MAX_SIZE, "GK");
	strncpy(hardware, runSystemCfg.deviceInfo.deviceType,64);

#endif
	return 1;
}

/*
int32_t GLNK_RTVideoOpenEXT_Callback(int32_t connectionID,unsigned char channel, GLNK_VideoDataFormat* videoinfo)
{
	return 0;
}

int32_t GLNK_RTVideoCloseEXT_Callback(int32_t connectionID)
{
	return 0;
}

int32_t GLNK_AudioEncodeOpenEXT_CallBack(int32_t connectionID, unsigned char channel,  GLNK_AudioDataFormat* audioinfo)
{
	return 0;
}

int32_t GLNK_AudioEncodeCloseEXT_CallBack(int32_t connectionID)
{
	return 0;
}

int32_t GLNK_AudioDecodeOpenEXT_CallBack(int32_t connectionID, unsigned char channel, GLNK_AudioDataFormat *audioinfo)
{
	return 0;
}

int32_t GLNK_AudioDecodeEXT_CallBack(int32_t connectionID, char* buffer, unsigned int length, GLNK_V_AudioInfo* audioframeinfo, char* extdata, int32_t extdatalen)
{
	return 0;
}

int32_t GLNK_AudioDecodeCloseEXT_CallBack(int32_t connectionID)
{
	return 0;
}*/

int32_t GLNK_SwitchOfDoor_Callback(char * passwd, int channel, int type,int lockdelay)
{
	return 1;
}
/*

int32_t GLNK_PlayBackSendStreamOpen_CallBack( char*  recordname, unsigned char target, int32_t *recordfd,  GLNK_VideoDataFormat* videoinfo, GLNK_AudioDataFormat* audioinfo,unsigned char *streamtype, unsigned char * sendtype,  int* FileTotalTime)
{

	AviPBHandle* pbhandle;

	pbhandle = create_pb_handle();
	if (!pbhandle)
		return 0;

	if (avi_pb_open(recordname, pbhandle) < 0)
	{
        printf("[%s]ERROR! fail to avi_pb_open file %s.\n", __FUNCTION__, recordname);
        avi_pb_close(pbhandle);
        return 0;
    }

	*recordfd = (int)pbhandle;
	*streamtype = VideoAndAudioStream;
	*sendtype = SendStreamType;
	*FileTotalTime = 0;

	videoinfo->codec = GLNK_CODEC_H264;
	videoinfo->bitrate = 1000;
	videoinfo->width = 704;
	videoinfo->height = 576;
	videoinfo->framerate = 25;
	videoinfo->colorDepth = 24;
	videoinfo->frameInterval = 50;

	audioinfo->samplesRate = 8000;
	audioinfo->bitrate = 16;
	audioinfo->waveFormat = GLNK_WAVE_FORMAT_G711;
	audioinfo->channelNumber = 1;
	audioinfo->blockAlign = 1;
	audioinfo->bitsPerSample =16;
	audioinfo->frameInterval = 25;

	printf("%s, opening file %s framerate=%d, bitrate=%d\n",__func__,recordname, videoinfo->framerate, videoinfo->bitrate);

	return 1;
}

int32_t GLNK_PlayBackSendStreamClose_CallBack(unsigned char target, int32_t recordfd)
{
	AviPBHandle* pbhandle = (AviPBHandle*)recordfd;

	printf("%s, filefd[%d]\n",__func__,target);
	glnk_stopSendplaybackdata(target);

	avi_pb_close(pbhandle);
	return 1;
}

int32_t GLNK_PlayBackSendStreamContrl_Callback(int32_t type, uint32_t value, int32_t target)
{
	int size, ret;
	char MediaBuffer[700000] = {0};
	AviPBHandle* pbhandle = (AviPBHandle*)recordfd;
	printf("%s, pbhandle[0x%x] streamtype[0x%x]\n",__func__, recordfd, (int)streamframetype);

	if (!pbhandle)
		return 0;

	size = 0;
	ret = avi_pb_get_frame(pbhandle, MediaBuffer, &size);
	if (ret < 0)
	{
		printf("[%s]ERROR! avi pb get frame failed\n", __FUNCTION__);
		return 0;
    }

	if (ret == GK_NET_NOMOREFILE)
	{
		printf("[%s]OK! playback file end.\n", __FUNCTION__);
		videoframeindex_s = 0;
        return 0;
    }

	if (pbhandle->node.frame_type == 0x11)
	{
		*streamframetype = 1;
		*videoframeindex = videoframeindex_s;
		videoframeindex_s++;
	}
	else if (pbhandle->node.frame_type == 0x10)
	{
		*streamframetype = 0;
		*videoframeindex = videoframeindex_s;
		videoframeindex_s++;
	}
	else
	{
		*streamframetype = 2; // audioframe
	}

	*streamdata = MediaBuffer;
	*streamdatalen = size;
	*timestamp = 0;

	printf("%s, type = %d,value=%d target[%d]\n",__func__,type,value, target);
	if(type == 0)
	{
		sendstreamflag[target] = 0;
	}
	else if(type == 1)
	{
		sendstreamflag[target] = 1;
		return 0;
	}
	else if(type == 2)
	{
		audiosendtime = 0;
		vodsendtime[target] = 0;
		close(newrecordfd[target]);
		newrecordfd[target] = 0;
		sendstreamflag[target] = 2;
	}
	else if(type == 3)
	{

	}
	else if(type == 4)
	{

	}
	else if(type == 5)
	{

	}
	return 1;
}

void GLNK_UpLoadFileResult_CallBack(char *filename, int32_t result)
{
}*/

int32_t GLNK_DownLoadFileConfig_Callback(int32_t type, int32_t SessionID, char *filename, uint32_t offset, unsigned int *FileLen)
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
	return 1;
}

static int32_t GetVauleFromJsonIntArray(cJSON *Root, const char* ArrayName, int32_t Num, char *ResultStr)
{
	int32_t Ret;
	if(cJSON_GetArrayItem(cJSON_GetObjectItem(Root, ArrayName), Num) != NULL)
	{
		switch(cJSON_GetArrayItem(cJSON_GetObjectItem(Root, ArrayName), Num)->type)
		{
			case cJSON_False:
				sprintf(ResultStr, "%d",cJSON_GetArrayItem(cJSON_GetObjectItem(Root, ArrayName), Num)->valueint);
				break;
			case cJSON_True:
				sprintf(ResultStr, "%d",cJSON_GetArrayItem(cJSON_GetObjectItem(Root, ArrayName), Num)->valueint);
				break;
			case cJSON_Number:
				sprintf(ResultStr, "%d",cJSON_GetArrayItem(cJSON_GetObjectItem(Root, ArrayName), Num)->valueint);
				break;
			case cJSON_String:
				sprintf(ResultStr, "%s",cJSON_GetArrayItem(cJSON_GetObjectItem(Root, ArrayName), Num)->valuestring);
				break;
			default:
				break;
		}
		Ret = 1;
	}
	else
	{
		Ret = 0;
	}
	return Ret;
}

static int32_t GetVauleFromJsonArray(cJSON *Root, const char* ArrayName, int32_t Num, const char *string, char *ResultStr)
{
	int32_t Ret;
	if(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(Root, ArrayName), Num), string) != NULL)
	{
		switch(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(Root, ArrayName), Num), string)->type)
		{
			case cJSON_False:
				sprintf(ResultStr, "%d",cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(Root, ArrayName), Num), string)->valueint);
				break;
			case cJSON_True:
				sprintf(ResultStr, "%d",cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(Root, ArrayName), Num), string)->valueint);
				break;
			case cJSON_Number:
				sprintf(ResultStr, "%d",cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(Root, ArrayName), Num), string)->valueint);
				break;
			case cJSON_String:
				sprintf(ResultStr, "%s",cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(Root, ArrayName), Num), string)->valuestring);
				break;
			default:
				break;
		}
		Ret = 1;
	}
	else
		Ret = 0;
	return Ret;
}


int32_t GLNK_TimeSyn_CallBack(int32_t sec, int32_t usec, int32_t zone, int32_t dst, char istwelve)
{
	printf("%s sec=%d zone=%d, dst = %d, istwelve=%d\n", __func__, sec, zone, dst, istwelve);

    struct tm tt = {0};
	long ts = (long)sec;
    struct tm *pTm = localtime_r(&ts, &tt);
    printf("%s %d-%02d-%02d %02d:%02d:%02d",__func__,pTm->tm_year+1900,pTm->tm_mon+1,pTm->tm_mday,pTm->tm_hour,pTm->tm_min,pTm->tm_sec);
	//sec is local time.
	netcam_sys_set_time_zone_by_utc_second(sec - zone, zone / 60);
	return 1;
}

int32_t GLNK_UpLoadData_Result_CallBack(char * DataPtr, int32_t Result)
{
	goolink_printk("GLNK_UpLoadData_Result_CallBack >>>>>>>>>>>>>>>>>  Result=%d \n", Result);
	//发送图片数据给对方后, 对方会有一个回馈, Result告诉设备上传是否ok
    switch(Result)
    {
    case UPLOADDATASUC: //上传成功
        goolink_printk("GLNK_UpLoadData_Result_CallBack, UPLOADDATASUC \n");
        break;
    case UPLOADDATAFAIL_NETWORK: //网络问题导致上传失败
        goolink_printk("GLNK_UpLoadData_Result_CallBack, UPLOADDATAFAIL_NETWORK \n");
        break;
    case UPLOADDATAFAIL_COMPETENCE: //没有权限导致上传失败
        goolink_printk("GLNK_UpLoadData_Result_CallBack, UPLOADDATAFAIL_COMPETENCE \n");
        break;
    case UPLOADDATAFAIL_UPLOADERROR: //上传中途出错导致上传失败
        goolink_printk("GLNK_UpLoadData_Result_CallBack, UPLOADDATAFAIL_UPLOADERROR \n");
        break;
    case UPLOADDATAFAIL_MODULESTOP: //云存储模块没启动
        goolink_printk("GLNK_UpLoadData_Result_CallBack, UPLOADDATAFAIL_MODULESTOP \n");
        break;
    case UPLOADDATAFAIL_DATAUPLOADING: //已经有数据正在上传导致失败
        goolink_printk("GLNK_UpLoadData_Result_CallBack, UPLOADDATAFAIL_DATAUPLOADING \n");
        break;
    case UPLOADDATAFAIL_NOSTREAMUPLOAD: //没有上传视频码流不能上传图片
        goolink_printk("GLNK_UpLoadData_Result_CallBack, UPLOADDATAFAIL_NOSTREAMUPLOAD \n");
        break;
    default:
        break;
    }

	return 1;
}

int32_t GLNK_bIsOpenCloudStorage_CallBack(int32_t Value)
{
	goolink_printk("%s:%d, value=%d\n",__func__, __LINE__, Value);
	return 1;
}

static int IsAllTheTime = 1;
int32_t Glnk_Set_MotionDetect_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen)
{
	cJSON* Root = NULL;
	cJSON* ResultJson = NULL;
	cJSON* ResultArray = NULL;
	int32_t MDArraySize = 0, i, j, TimeArraySize;

	char ResultStr[20];
	char Channel[4];
	char *pStr = NULL;
	char* p = NULL;

	goolink_printk("[%s] [%d]   pjsondata = %s MDArraySize=%d\n",__func__,__LINE__,pJsonData,MDArraySize);
	Root  = cJSON_Parse(pJsonData);
	if(Root == NULL)
	{
		return 0;
	}

	ResultArray = cJSON_CreateArray();
	MDArraySize = cJSON_GetArraySize(cJSON_GetObjectItem(Root, "MotionDetectList"));

	for(i = 0; i < MDArraySize; i++)
	{
		//GetVauleFromJsonArray(Root, "MotionDetectList", i, "Channel", Channel);
		//printf("Channel : %s \n", Channel);

		GetVauleFromJsonArray(Root, "MotionDetectList", i, "ChannelOpen", ResultStr);
		printf("ChannelOpen : %s\n", ResultStr);
		runMdCfg.enable = atoi(ResultStr);

		//GetVauleFromJsonArray(Root, "MotionDetectList", i, "EnablePush", ResultStr);
		//printf("EnablePush : %s\n", ResultStr);

		GetVauleFromJsonArray(Root, "MotionDetectList", i, "Sensitivity", ResultStr);
		printf("Sensitivity : %s\n", ResultStr);
		runMdCfg.sensitive = atoi(ResultStr) * 20;
        netcam_md_set_sensitivity_percentage(0, runMdCfg.sensitive);

		//GetVauleFromJsonArray(Root, "MotionDetectList", i, "PhotoNum", ResultStr);
		//printf("PhotoNum : %s\n", ResultStr);

		//GetVauleFromJsonArray(Root, "MotionDetectList", i, "PhotoItvMs", ResultStr);
		//printf("PhotoItvMs : %s\n", ResultStr);

		//GetVauleFromJsonArray(Root, "MotionDetectList", i, "PreRecVideoSec", ResultStr);
		//printf("PreRecVideoSec : %s\n", ResultStr);

		//GetVauleFromJsonArray(Root, "MotionDetectList", i, "RecVideoDurSec", ResultStr);
		//printf("RecVideoDurSec : %s\n", ResultStr);

		GetVauleFromJsonArray(Root, "MotionDetectList", i, "LocalPhoto", ResultStr);
		printf("LocalPhoto : %s\n", ResultStr);
		runMdCfg.handle.is_snap = atoi(ResultStr);

		GetVauleFromJsonArray(Root, "MotionDetectList", i, "LocalVideo", ResultStr);
		printf("LocalVideo : %s\n", ResultStr);
		runMdCfg.handle.is_rec = atoi(ResultStr);

		//GetVauleFromJsonArray(Root, "MotionDetectList", i, "CloudPhoto", ResultStr);
		//printf("CloudPhoto : %s\n", ResultStr);

		//GetVauleFromJsonArray(Root, "MotionDetectList", i, "CloudVideo", ResultStr);
		//printf("CloudVideo : %s\n", ResultStr);

		GetVauleFromJsonArray(Root, "MotionDetectList", i, "IsAllTheTime", ResultStr);
		printf("IsAllTheTime : %s\n", ResultStr);
		IsAllTheTime = atoi(ResultStr);

		TimeArraySize = cJSON_GetArraySize(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(Root, "MotionDetectList"), i), "ArmTime"));
		for(j = 0; j < TimeArraySize; j++)
		{
			//GetVauleFromJsonArray(cJSON_GetArrayItem(cJSON_GetObjectItem(Root, "MotionDetectList"), i), "ArmTime", j, "WeekDay", ResultStr);
			//printf("WeekDay : %s\n", ResultStr);

			//GetVauleFromJsonArray(cJSON_GetArrayItem(cJSON_GetObjectItem(Root, "MotionDetectList"), i), "ArmTime", j, "IsEnable", ResultStr);
			//printf("IsEnable : %s\n", ResultStr);

			GetVauleFromJsonArray(cJSON_GetArrayItem(cJSON_GetObjectItem(Root, "MotionDetectList"), i), "ArmTime", j, "StartTime", ResultStr);
			printf("StartTime : %s\n", ResultStr);
			p = ResultStr;
			runMdCfg.scheduleTime[j][0].startHour = (*p -'0') * 10;
			p++;
			runMdCfg.scheduleTime[j][0].startHour += *p -'0';
			p++;
			runMdCfg.scheduleTime[j][0].startMin = (*p -'0') * 10;
			p++;
			runMdCfg.scheduleTime[j][0].startMin += *p -'0';

			GetVauleFromJsonArray(cJSON_GetArrayItem(cJSON_GetObjectItem(Root, "MotionDetectList"), i), "ArmTime", j, "EndTime", ResultStr);
			printf("EndTime : %s\n", ResultStr);
			p = ResultStr;
			runMdCfg.scheduleTime[j][0].stopHour = (*p -'0') * 10;
			p++;
			runMdCfg.scheduleTime[j][0].stopHour += *p -'0';
			p++;
			runMdCfg.scheduleTime[j][0].stopMin = (*p -'0') * 10;
			p++;
			runMdCfg.scheduleTime[j][0].stopMin += *p -'0';

			if (strcmp(ResultStr, "000000") == 0)
				IsAllTheTime = 0;
		}
		ResultJson = cJSON_CreateObject();
		cJSON_AddNumberToObject(ResultJson, "Channel", atoi(Channel));
		cJSON_AddTrueToObject(ResultJson, "ReSuc");
		cJSON_AddItemToArray(ResultArray, ResultJson);
	}

	pStr = cJSON_PrintUnformatted(ResultArray);
	strcpy(pResultData, pStr);
	printf("pStr = %s\n",pStr);
	free(pStr);
	*ResultDataLen = strlen(pResultData);
	cJSON_Delete(ResultArray);
	cJSON_Delete(Root);

	MdCfgSave();
	return 1;
}

int32_t Glnk_Get_MotionDetect_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen)
{
	cJSON* MotionDetectArray;
	cJSON* TimeArray;
	cJSON* ArmTime, *ArmTime2,*ArmTime3,*ArmTime4,*ArmTime5,*ArmTime6,*ArmTime7;
	cJSON* MotionDetect;
	cJSON* MotionDetectroot;
	//cJSON* root;
	cJSON* RecvJson = NULL;
	char *pStr;
	char Channel[4];
	int32_t MDArraySize = 0, i;
	char tmp[8];

	goolink_printk("Glnk_Get_MotionDetect_Opt_Callback >>>>>>>>>>>>>>>>>>>>>>>>>>>>> \n");
    //goolink_printk("pJsonData=%s, JsonDataLen=%d, pResultData=%s, ResultDataLen=%d \n",
        								//pJsonData, JsonDataLen, pResultData, ResultDataLen);

	RecvJson    = cJSON_Parse(pJsonData);
	TimeArray   = cJSON_CreateArray();
	ArmTime	    = cJSON_CreateObject();
	MDArraySize = cJSON_GetArraySize(RecvJson);

	goolink_printk(" pjsondata = %s MDArraySize=%d\n",pJsonData,MDArraySize);

	for(i = 0; i < MDArraySize; i++)
	{
		GetVauleFromJsonIntArray(RecvJson, "Channels", i, Channel);

		cJSON_AddStringToObject(ArmTime,"WeekDay","Sun");
		if (runMdCfg.scheduleTime[0][0].stopHour || runMdCfg.scheduleTime[0][0].stopMin)
		{
			cJSON_AddTrueToObject(ArmTime,"IsEnable");
			snprintf(tmp, sizeof(tmp), "%02d%02d59", runMdCfg.scheduleTime[0][0].stopHour,runMdCfg.scheduleTime[0][0].stopMin);
		}
		else
		{
			cJSON_AddFalseToObject(ArmTime,"IsEnable");
			snprintf(tmp, sizeof(tmp), "000000");
		}

		cJSON_AddStringToObject(ArmTime,"EndTime",tmp);
		snprintf(tmp, sizeof(tmp), "%02d%02d00", runMdCfg.scheduleTime[0][0].startHour,runMdCfg.scheduleTime[0][0].startMin);
		cJSON_AddStringToObject(ArmTime,"StartTime",tmp);

		ArmTime2 = cJSON_CreateObject();
		cJSON_AddStringToObject(ArmTime2,"WeekDay","Mon");
		if (runMdCfg.scheduleTime[1][0].stopHour || runMdCfg.scheduleTime[1][0].stopMin)
		{
			cJSON_AddTrueToObject(ArmTime2,"IsEnable");
			snprintf(tmp, sizeof(tmp), "%02d%02d59", runMdCfg.scheduleTime[1][0].stopHour,runMdCfg.scheduleTime[1][0].stopMin);
		}
		else
		{
			cJSON_AddFalseToObject(ArmTime2,"IsEnable");
			snprintf(tmp, sizeof(tmp), "000000");
		}

		cJSON_AddStringToObject(ArmTime2,"EndTime",tmp);
		snprintf(tmp, sizeof(tmp), "%02d%02d00", runMdCfg.scheduleTime[1][0].startHour,runMdCfg.scheduleTime[1][0].startMin);
		cJSON_AddStringToObject(ArmTime2,"StartTime",tmp);

		ArmTime3 = cJSON_CreateObject();
		cJSON_AddStringToObject(ArmTime3,"WeekDay","Tue");
		if (runMdCfg.scheduleTime[2][0].stopHour || runMdCfg.scheduleTime[2][0].stopMin)
		{
			cJSON_AddTrueToObject(ArmTime3,"IsEnable");
			snprintf(tmp, sizeof(tmp), "%02d%02d59", runMdCfg.scheduleTime[2][0].stopHour,runMdCfg.scheduleTime[2][0].stopMin);
		}
		else
		{
			cJSON_AddFalseToObject(ArmTime3,"IsEnable");
			snprintf(tmp, sizeof(tmp), "000000");
		}
		cJSON_AddStringToObject(ArmTime3,"EndTime",tmp);
		snprintf(tmp, sizeof(tmp), "%02d%02d00", runMdCfg.scheduleTime[2][0].startHour,runMdCfg.scheduleTime[2][0].startMin);
		cJSON_AddStringToObject(ArmTime3,"StartTime",tmp);

		ArmTime4 = cJSON_CreateObject();
		cJSON_AddStringToObject(ArmTime4,"WeekDay","Wed");
		if (runMdCfg.scheduleTime[3][0].stopHour || runMdCfg.scheduleTime[3][0].stopMin)
		{
			cJSON_AddTrueToObject(ArmTime4,"IsEnable");
			snprintf(tmp, sizeof(tmp), "%02d%02d59", runMdCfg.scheduleTime[3][0].stopHour,runMdCfg.scheduleTime[3][0].stopMin);
		}
		else
		{
			cJSON_AddFalseToObject(ArmTime4,"IsEnable");
			snprintf(tmp, sizeof(tmp), "000000");
		}
		cJSON_AddStringToObject(ArmTime4,"EndTime",tmp);
		snprintf(tmp, sizeof(tmp), "%02d%02d00", runMdCfg.scheduleTime[3][0].startHour,runMdCfg.scheduleTime[3][0].startMin);
		cJSON_AddStringToObject(ArmTime4,"StartTime",tmp);

		ArmTime5 = cJSON_CreateObject();
		cJSON_AddStringToObject(ArmTime5,"WeekDay","Thu");
		if (runMdCfg.scheduleTime[4][0].stopHour || runMdCfg.scheduleTime[4][0].stopMin)
		{
			cJSON_AddTrueToObject(ArmTime5,"IsEnable");
			snprintf(tmp, sizeof(tmp), "%02d%02d59", runMdCfg.scheduleTime[4][0].stopHour,runMdCfg.scheduleTime[4][0].stopMin);
		}
		else
		{
			cJSON_AddFalseToObject(ArmTime5,"IsEnable");
			snprintf(tmp, sizeof(tmp), "000000");
		}
		cJSON_AddStringToObject(ArmTime5,"EndTime",tmp);
		snprintf(tmp, sizeof(tmp), "%02d%02d00", runMdCfg.scheduleTime[4][0].startHour,runMdCfg.scheduleTime[4][0].startMin);
		cJSON_AddStringToObject(ArmTime5,"StartTime",tmp);

		ArmTime6 = cJSON_CreateObject();
		cJSON_AddStringToObject(ArmTime6,"WeekDay","Fri");
		if (runMdCfg.scheduleTime[5][0].stopHour || runMdCfg.scheduleTime[5][0].stopMin)
		{
			cJSON_AddTrueToObject(ArmTime6,"IsEnable");
			snprintf(tmp, sizeof(tmp), "%02d%02d59", runMdCfg.scheduleTime[5][0].stopHour,runMdCfg.scheduleTime[5][0].stopMin);
		}
		else
		{
			cJSON_AddFalseToObject(ArmTime6,"IsEnable");
			snprintf(tmp, sizeof(tmp), "000000");
		}
		cJSON_AddStringToObject(ArmTime6,"EndTime",tmp);
		snprintf(tmp, sizeof(tmp), "%02d%02d00", runMdCfg.scheduleTime[5][0].startHour,runMdCfg.scheduleTime[5][0].startMin);
		cJSON_AddStringToObject(ArmTime6,"StartTime",tmp);

		ArmTime7 = cJSON_CreateObject();
		cJSON_AddStringToObject(ArmTime7,"WeekDay","Sat");
		if (runMdCfg.scheduleTime[6][0].stopHour || runMdCfg.scheduleTime[6][0].stopMin)
		{
			cJSON_AddTrueToObject(ArmTime7,"IsEnable");
			snprintf(tmp, sizeof(tmp), "%02d%02d59", runMdCfg.scheduleTime[6][0].stopHour,runMdCfg.scheduleTime[6][0].stopMin);
		}
		else
		{
			cJSON_AddFalseToObject(ArmTime7,"IsEnable");
			snprintf(tmp, sizeof(tmp), "000000");
		}
		cJSON_AddStringToObject(ArmTime7,"EndTime",tmp);
		snprintf(tmp, sizeof(tmp), "%02d%02d00", runMdCfg.scheduleTime[6][0].startHour,runMdCfg.scheduleTime[6][0].startMin);
		cJSON_AddStringToObject(ArmTime7,"StartTime",tmp);

		cJSON_AddItemReferenceToArray(TimeArray, ArmTime);
		cJSON_AddItemReferenceToArray(TimeArray, ArmTime2);
		cJSON_AddItemReferenceToArray(TimeArray, ArmTime3);
		cJSON_AddItemReferenceToArray(TimeArray, ArmTime4);
		cJSON_AddItemReferenceToArray(TimeArray, ArmTime5);
		cJSON_AddItemReferenceToArray(TimeArray, ArmTime6);
		cJSON_AddItemReferenceToArray(TimeArray, ArmTime7);

		MotionDetect = cJSON_CreateObject();
		goolink_printk("channel = %d\n",atoi(Channel));
		cJSON_AddNumberToObject(MotionDetect,"Channel", atoi(Channel));

		if (runMdCfg.enable)
			cJSON_AddTrueToObject(MotionDetect,"ChannelOpen");
		else
			cJSON_AddFalseToObject(MotionDetect,"ChannelOpen");

		cJSON_AddTrueToObject(MotionDetect,"EnablePush");
		cJSON_AddNumberToObject(MotionDetect,"Sensitivity", runMdCfg.sensitive / 20);

		cJSON_AddNumberToObject(MotionDetect,"PhotoNum", 20);
		cJSON_AddNumberToObject(MotionDetect,"PhotoItvMs", 500);
		cJSON_AddNumberToObject(MotionDetect,"PreRecVideoSec", 0);
		cJSON_AddNumberToObject(MotionDetect,"RecVideoDurSec", 60);

		if (runMdCfg.handle.is_snap)
			cJSON_AddTrueToObject(MotionDetect,"LocalPhoto");
		else
			cJSON_AddFalseToObject(MotionDetect,"LocalPhoto");

		if (runMdCfg.handle.is_rec)
			cJSON_AddTrueToObject(MotionDetect,"LocalVideo");
		else
			cJSON_AddFalseToObject(MotionDetect,"LocalVideo");

		cJSON_AddTrueToObject(MotionDetect,"CloudPhoto");
		cJSON_AddTrueToObject(MotionDetect,"CloudVideo");

		if (IsAllTheTime)
			cJSON_AddTrueToObject(MotionDetect,"IsAllTheTime");
		else
			cJSON_AddFalseToObject(MotionDetect,"IsAllTheTime");

		cJSON_AddItemReferenceToObject(MotionDetect, "ArmTime", TimeArray);
		MotionDetectArray = cJSON_CreateArray();
		cJSON_AddItemReferenceToArray(MotionDetectArray, MotionDetect);
	}

	MotionDetectroot = cJSON_CreateObject();
	cJSON_AddFalseToObject(MotionDetectroot,"MDetectClose");
	cJSON_AddItemReferenceToObject(MotionDetectroot, "MotionDetectList", MotionDetectArray);

	pStr = cJSON_PrintUnformatted(MotionDetectroot);

	strcpy(pResultData, pStr);
	goolink_printk("pStr = %s\n",pStr);
	free(pStr);
	*ResultDataLen = strlen(pResultData);
	cJSON_Delete(MotionDetectroot);


	return 1;
}

static int isAllTime_record = 1;
int32_t Glnk_Set_VideoRecord_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen)
{
	int    ret         = 0;
	cJSON* Root        = NULL;
	cJSON* ResultJson  = NULL;
	cJSON* ResultArray = NULL;
	int VRArraySize    = 0;
	int i              = 0;
	int j              = 0;
	int TimeArraySize  = 0;
	char Channel[4]    = {0};
	char ResultStr[20] = {0};
	char *pStr         = NULL;
	char *p            = NULL;

	goolink_printk("Glnk_Set_VideoRecord_Opt_Callback >>>>>>>>>>>>>>>>>>>>>>>>>>>>> \n");
	//goolink_printk("pjsondata = %s\n",pJsonData);

	ResultArray = cJSON_CreateArray();
	Root  = cJSON_Parse(pJsonData);
	if(Root == NULL)
	{
		goolink_error("cJSON_Parse error\n");
		return 0;
	}

	VRArraySize = cJSON_GetArraySize(cJSON_GetObjectItem(Root, "VideoRecordList"));
	goolink_printk("VRArraySize = %d \n", VRArraySize);
	for(i = 0; i < VRArraySize; i++)
	{
		//GetVauleFromJsonArray(Root, "VideoRecordList", i, "Channel", Channel);
		//printf("Channel = %s\n", Channel);

		GetVauleFromJsonArray(Root, "VideoRecordList", i, "ChannelOpen", ResultStr);
		goolink_printk("ChannelOpen = %s\n", ResultStr);
		ret = atoi(ResultStr);
		if (ret == 0){
			runRecordCfg.recordMode = 3;
			runRecordCfg.enable     = 0;
			goolink_printk("runRecordCfg.recordMode = %d\n", runRecordCfg.recordMode);
		}
		else
		{
			runRecordCfg.enable     = 1;
			runRecordCfg.recordMode = 0;
			goolink_printk("runRecordCfg.recordMode = %d,enable=%d\n", runRecordCfg.recordMode,runRecordCfg.enable);
		}

		GetVauleFromJsonArray(Root, "VideoRecordList", i, "StreamLvl", ResultStr);
		goolink_printk("StreamLvl = %s \n", ResultStr);
		ret = atoi(ResultStr);
		if (ret == 0)
			runRecordCfg.stream_no = 0;
		else
			runRecordCfg.stream_no = 1;

		TimeArraySize = cJSON_GetArraySize(cJSON_GetObjectItem(cJSON_GetArrayItem(cJSON_GetObjectItem(Root, "VideoRecordList"), i), "ArmTime"));
		goolink_printk("TimeArraySize = %d \n", TimeArraySize);
		for(j = 0; j < TimeArraySize; j++)
		{
			//GetVauleFromJsonArray(cJSON_GetArrayItem(cJSON_GetObjectItem(Root, "VideoRecordList"), i), "ArmTime", j, "WeekDay", ResultStr);
			//printf("WeekDay : %s\n", ResultStr);

			GetVauleFromJsonArray(cJSON_GetArrayItem(cJSON_GetObjectItem(Root, "VideoRecordList"), i), "ArmTime", j, "IsEnable", ResultStr);
			goolink_printk("IsEnable : %s\n", ResultStr);
			ret = atoi(ResultStr);
			if (ret == 0)
			{
				runP2PCfg.scheduleTimeEnable[j] = 0;
			}else
			{
				runP2PCfg.scheduleTimeEnable[j] = 1;
			}

			GetVauleFromJsonArray(cJSON_GetArrayItem(cJSON_GetObjectItem(Root, "VideoRecordList"), i), "ArmTime", j, "StartTime", ResultStr);
			goolink_printk("StartTime : %s\n", ResultStr);
			p = ResultStr;
			runRecordCfg.scheduleTime[j][0].startHour = (*p -'0') * 10;
			p++;
			runRecordCfg.scheduleTime[j][0].startHour += *p -'0';
			p++;
			runRecordCfg.scheduleTime[j][0].startMin = (*p -'0') * 10;
			p++;
			runRecordCfg.scheduleTime[j][0].startMin += *p -'0';
			p++;
			runP2PCfg.scheduleTimeSec[j][0].startSec = (*p -'0') * 10;
			p++;
			runP2PCfg.scheduleTimeSec[j][0].startSec += *p -'0';

			GetVauleFromJsonArray(cJSON_GetArrayItem(cJSON_GetObjectItem(Root, "VideoRecordList"), i), "ArmTime", j, "EndTime", ResultStr);
			goolink_printk("EndTime : %s\n", ResultStr);
			p = ResultStr;
			runRecordCfg.scheduleTime[j][0].stopHour = (*p -'0') * 10;
			p++;
			runRecordCfg.scheduleTime[j][0].stopHour += *p -'0';
			p++;
			runRecordCfg.scheduleTime[j][0].stopMin = (*p -'0') * 10;
			p++;
			runRecordCfg.scheduleTime[j][0].stopMin += *p -'0';
			p++;
			runP2PCfg.scheduleTimeSec[j][0].stopSec = (*p -'0') * 10;
			p++;
			runP2PCfg.scheduleTimeSec[j][0].stopSec += *p -'0';

			if (strcmp(ResultStr, "000000") == 0)
				isAllTime_record = 0;
		}

		ResultJson = cJSON_CreateObject();

		cJSON_AddNumberToObject(ResultJson, "Channel", atoi(Channel));
		cJSON_AddTrueToObject(ResultJson, "ReSuc");
		cJSON_AddItemToArray(ResultArray, ResultJson);
	}

	pStr = cJSON_PrintUnformatted(ResultArray);

	strcpy(pResultData, pStr);
	//goolink_printk("pStr = %s\n",pStr);

	*ResultDataLen = strlen(pResultData);
	cJSON_Delete(ResultArray);
	free(pStr);
	cJSON_Delete(Root);

	RecordCfgSave();
	P2PCfgSave();

	return 1;
}

int32_t Glnk_Get_VideoRecord_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen)
{
	cJSON* VideoRecordArray = NULL;
	cJSON* VideoRecordroot  = NULL;
	cJSON* VideoRecord      = NULL;
	cJSON* TimeArray        = NULL;
	cJSON* ArmTime, *ArmTime2,*ArmTime3,*ArmTime4,*ArmTime5,*ArmTime6,*ArmTime7;
	cJSON* RecvJson = NULL;
	char *pStr      = NULL;
	char Channel[4] = {0};
	char tmp[64]    = {0};
	int32_t MDArraySize = 0, i;

	goolink_printk("Glnk_Get_VideoRecord_Opt_Callback >>>>>>>>>>>>>>>>>>>>>>>>>>>>> \n");
	goolink_printk("[%s] [%d]   pjsondata = %s\n",__func__,__LINE__,pJsonData);

	RecvJson    = cJSON_Parse(pJsonData);
	VideoRecord = cJSON_CreateObject();
	TimeArray   = cJSON_CreateArray();

	MDArraySize = cJSON_GetArraySize(RecvJson);
	for(i = 0; i < MDArraySize; i++)
	{
		GetVauleFromJsonIntArray(RecvJson, "Channels", i, Channel);

		goolink_error("runP2PCfg.scheduleTimeEnable[0] = %d\n",runP2PCfg.scheduleTimeEnable[0]);
		goolink_error("runP2PCfg.scheduleTimeEnable[1] = %d\n",runP2PCfg.scheduleTimeEnable[1]);
		goolink_error("runP2PCfg.scheduleTimeEnable[2] = %d\n",runP2PCfg.scheduleTimeEnable[2]);
		goolink_error("runP2PCfg.scheduleTimeEnable[3] = %d\n",runP2PCfg.scheduleTimeEnable[3]);
		goolink_error("runP2PCfg.scheduleTimeEnable[4] = %d\n",runP2PCfg.scheduleTimeEnable[4]);
		goolink_error("runP2PCfg.scheduleTimeEnable[5] = %d\n",runP2PCfg.scheduleTimeEnable[5]);
		goolink_error("runP2PCfg.scheduleTimeEnable[6] = %d\n",runP2PCfg.scheduleTimeEnable[6]);

		// sun day
		ArmTime	= cJSON_CreateObject();
		cJSON_AddStringToObject(ArmTime,"WeekDay","Sun");
		if (runP2PCfg.scheduleTimeEnable[0] == 1)
		{
			cJSON_AddTrueToObject(ArmTime,"IsEnable");
		}
		else
		{
			cJSON_AddFalseToObject(ArmTime,"IsEnable");
		}

		if (runRecordCfg.scheduleTime[0][0].stopHour ||
			runRecordCfg.scheduleTime[0][0].stopMin  ||
			runP2PCfg.scheduleTimeSec[0][0].stopSec)
		{

			snprintf(tmp, sizeof(tmp), "%02d%02d%02d", runRecordCfg.scheduleTime[0][0].stopHour,
													   runRecordCfg.scheduleTime[0][0].stopMin,
													   runP2PCfg.scheduleTimeSec[0][0].stopSec);
		}
		else
		{

			snprintf(tmp, sizeof(tmp), "000000");
		}
		cJSON_AddStringToObject(ArmTime,"EndTime",tmp);

		snprintf(tmp, sizeof(tmp), "%02d%02d%02d", runRecordCfg.scheduleTime[0][0].startHour,
												   runRecordCfg.scheduleTime[0][0].startMin,
												   runP2PCfg.scheduleTimeSec[0][0].startSec);
		cJSON_AddStringToObject(ArmTime,"StartTime",tmp);


		// mon day
		ArmTime2	= cJSON_CreateObject();
		cJSON_AddStringToObject(ArmTime2,"WeekDay","Mon");
		if (runP2PCfg.scheduleTimeEnable[1] == 1)
		{
			cJSON_AddTrueToObject(ArmTime2,"IsEnable");
		}
		else
		{
			cJSON_AddFalseToObject(ArmTime2,"IsEnable");
		}

		if (runRecordCfg.scheduleTime[1][0].stopHour ||
			runRecordCfg.scheduleTime[1][0].stopMin  ||
			runP2PCfg.scheduleTimeSec[1][0].stopSec)
		{
			snprintf(tmp, sizeof(tmp), "%02d%02d%02d",  runRecordCfg.scheduleTime[1][0].stopHour,
														runRecordCfg.scheduleTime[1][0].stopMin,
														runP2PCfg.scheduleTimeSec[1][0].stopSec);
		}
		else
		{
			snprintf(tmp, sizeof(tmp), "000000");
		}
		cJSON_AddStringToObject(ArmTime2,"EndTime",tmp);

		snprintf(tmp, sizeof(tmp), "%02d%02d%02d", runRecordCfg.scheduleTime[1][0].startHour,
			                                       runRecordCfg.scheduleTime[1][0].startMin,
			                                       runP2PCfg.scheduleTimeSec[1][0].startSec);

		cJSON_AddStringToObject(ArmTime2,"StartTime",tmp);



		ArmTime3	= cJSON_CreateObject();
		cJSON_AddStringToObject(ArmTime3,"WeekDay","Tue");
		if (runP2PCfg.scheduleTimeEnable[2] == 1)
		{
			cJSON_AddTrueToObject(ArmTime3,"IsEnable");
		}
		else
		{
			cJSON_AddFalseToObject(ArmTime3,"IsEnable");
		}

		if (runRecordCfg.scheduleTime[2][0].stopHour ||
			runRecordCfg.scheduleTime[2][0].stopMin  ||
			runP2PCfg.scheduleTimeSec[2][0].stopSec)
		{
			snprintf(tmp, sizeof(tmp), "%02d%02d%02d",  runRecordCfg.scheduleTime[2][0].stopHour,
														runRecordCfg.scheduleTime[2][0].stopMin,
														runP2PCfg.scheduleTimeSec[2][0].stopSec);
		}
		else
		{
			snprintf(tmp, sizeof(tmp), "000000");
		}
		cJSON_AddStringToObject(ArmTime3,"EndTime",tmp);
		snprintf(tmp, sizeof(tmp), "%02d%02d%02d",   runRecordCfg.scheduleTime[2][0].startHour,
													 runRecordCfg.scheduleTime[2][0].startMin,
													 runP2PCfg.scheduleTimeSec[2][0].startSec);
		cJSON_AddStringToObject(ArmTime3,"StartTime",tmp);



		ArmTime4	= cJSON_CreateObject();
		cJSON_AddStringToObject(ArmTime4,"WeekDay","Wed");
		if (runP2PCfg.scheduleTimeEnable[3] == 1)
		{
			cJSON_AddTrueToObject(ArmTime4,"IsEnable");
		}
		else
		{
			cJSON_AddFalseToObject(ArmTime4,"IsEnable");
		}

		if (runRecordCfg.scheduleTime[3][0].stopHour ||
			runRecordCfg.scheduleTime[3][0].stopMin  ||
			runP2PCfg.scheduleTimeSec[3][0].stopSec)
		{
			snprintf(tmp, sizeof(tmp), "%02d%02d%02d",  runRecordCfg.scheduleTime[3][0].stopHour,
														runRecordCfg.scheduleTime[3][0].stopMin,
														runP2PCfg.scheduleTimeSec[3][0].stopSec);
		}
		else
		{
			snprintf(tmp, sizeof(tmp), "000000");
		}
		cJSON_AddStringToObject(ArmTime4,"EndTime",tmp);
		snprintf(tmp, sizeof(tmp), "%02d%02d%02d", runRecordCfg.scheduleTime[3][0].startHour,
												   runRecordCfg.scheduleTime[3][0].startMin,
												   runP2PCfg.scheduleTimeSec[3][0].startSec);
		cJSON_AddStringToObject(ArmTime4,"StartTime",tmp);



		ArmTime5	= cJSON_CreateObject();
		cJSON_AddStringToObject(ArmTime5,"WeekDay","Thu");
		if (runP2PCfg.scheduleTimeEnable[4] == 1)
		{
			cJSON_AddTrueToObject(ArmTime5,"IsEnable");
		}
		else
		{
			cJSON_AddFalseToObject(ArmTime5,"IsEnable");
		}

		if (runRecordCfg.scheduleTime[4][0].stopHour ||
			runRecordCfg.scheduleTime[4][0].stopMin  ||
			runP2PCfg.scheduleTimeSec[4][0].stopSec)
		{
			snprintf(tmp, sizeof(tmp), "%02d%02d%02d",  runRecordCfg.scheduleTime[4][0].stopHour,
														runRecordCfg.scheduleTime[4][0].stopMin,
														runP2PCfg.scheduleTimeSec[4][0].stopSec);
		}
		else
		{
			snprintf(tmp, sizeof(tmp), "000000");
		}
		cJSON_AddStringToObject(ArmTime5,"EndTime",tmp);
		snprintf(tmp, sizeof(tmp), "%02d%02d%02d", runRecordCfg.scheduleTime[4][0].startHour,
												   runRecordCfg.scheduleTime[4][0].startMin,
												   runP2PCfg.scheduleTimeSec[4][0].startSec);
		cJSON_AddStringToObject(ArmTime5,"StartTime",tmp);


		ArmTime6	= cJSON_CreateObject();
		cJSON_AddStringToObject(ArmTime6,"WeekDay","Fri");
		if (runP2PCfg.scheduleTimeEnable[5] == 1)
		{
			cJSON_AddTrueToObject(ArmTime6,"IsEnable");
		}
		else
		{
			cJSON_AddFalseToObject(ArmTime6,"IsEnable");
		}

		if (runRecordCfg.scheduleTime[5][0].stopHour ||
		runRecordCfg.scheduleTime[5][0].stopMin  ||
		runP2PCfg.scheduleTimeSec[5][0].stopSec)
		{
			snprintf(tmp, sizeof(tmp), "%02d%02d%02d",  runRecordCfg.scheduleTime[5][0].stopHour,
														runRecordCfg.scheduleTime[5][0].stopMin,
														runP2PCfg.scheduleTimeSec[5][0].stopSec);
		}
		else
		{
			snprintf(tmp, sizeof(tmp), "000000");
		}
		cJSON_AddStringToObject(ArmTime6,"EndTime",tmp);
		snprintf(tmp, sizeof(tmp), "%02d%02d%02d",  runRecordCfg.scheduleTime[5][0].startHour,
													runRecordCfg.scheduleTime[5][0].startMin,
													runP2PCfg.scheduleTimeSec[5][0].startSec);
		cJSON_AddStringToObject(ArmTime6,"StartTime",tmp);


		ArmTime7	= cJSON_CreateObject();
		cJSON_AddStringToObject(ArmTime7,"WeekDay","Sat");
		if (runP2PCfg.scheduleTimeEnable[6] == 1)
		{
			cJSON_AddTrueToObject(ArmTime7,"IsEnable");
		}
		else
		{
			cJSON_AddFalseToObject(ArmTime7,"IsEnable");
		}
		if (runRecordCfg.scheduleTime[6][0].stopHour ||
			runRecordCfg.scheduleTime[6][0].stopMin  ||
			runP2PCfg.scheduleTimeSec[6][0].stopSec)
		{
			snprintf(tmp, sizeof(tmp), "%02d%02d%02d",  runRecordCfg.scheduleTime[6][0].stopHour,
														runRecordCfg.scheduleTime[6][0].stopMin,
														runP2PCfg.scheduleTimeSec[6][0].stopSec);
		}
		else
		{
			snprintf(tmp, sizeof(tmp), "000000");
		}
		cJSON_AddStringToObject(ArmTime7,"EndTime",tmp);
		snprintf(tmp, sizeof(tmp), "%02d%02d%02d",  runRecordCfg.scheduleTime[6][0].startHour,
													runRecordCfg.scheduleTime[6][0].startMin,
													runP2PCfg.scheduleTimeSec[6][0].startSec);
		cJSON_AddStringToObject(ArmTime7,"StartTime",tmp);



		cJSON_AddItemReferenceToArray(TimeArray, ArmTime);
		cJSON_AddItemReferenceToArray(TimeArray, ArmTime2);
		cJSON_AddItemReferenceToArray(TimeArray, ArmTime3);
		cJSON_AddItemReferenceToArray(TimeArray, ArmTime4);
		cJSON_AddItemReferenceToArray(TimeArray, ArmTime5);
		cJSON_AddItemReferenceToArray(TimeArray, ArmTime6);
		cJSON_AddItemReferenceToArray(TimeArray, ArmTime7);

		goolink_printk("channel = %d\n",atoi(Channel));
		cJSON_AddNumberToObject(VideoRecord,"Channel", atoi(Channel));
		if (runRecordCfg.enable == 1)
			cJSON_AddTrueToObject(VideoRecord,"ChannelOpen");
		else
			cJSON_AddFalseToObject(VideoRecord,"ChannelOpen");

		if (runRecordCfg.stream_no == 0)
			cJSON_AddNumberToObject(VideoRecord,"StreamLvl", 0);
		else
			cJSON_AddNumberToObject(VideoRecord,"StreamLvl", 1);

		if (isAllTime_record)
			cJSON_AddTrueToObject(VideoRecord,"IsAllTheTime");
		else
			cJSON_AddFalseToObject(VideoRecord,"IsAllTheTime");

		cJSON_AddItemReferenceToObject(VideoRecord, "ArmTime", TimeArray);
		VideoRecordArray = cJSON_CreateArray();
		cJSON_AddItemReferenceToArray(VideoRecordArray, VideoRecord);
	}
	VideoRecordroot = cJSON_CreateObject();
	cJSON_AddTrueToObject(VideoRecordroot,"VideoRecClose");

	cJSON_AddItemReferenceToObject(VideoRecordroot, "VideoRecordList", VideoRecordArray);

	pStr = cJSON_PrintUnformatted(VideoRecordroot);
	strcpy(pResultData, pStr);
	goolink_printk("pStr = %s\n",pStr);
	free(pStr);
	*ResultDataLen = strlen(pResultData);
	cJSON_Delete(VideoRecordroot);
	return 1;
}

int32_t Glnk_Set_Time_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen)
{
	printf("%s:%d\n",__func__, __LINE__);
	return 1;
}

int32_t Glnk_Get_Time_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen)
{
	printf("%s:%d\n",__func__, __LINE__);
	return 1;
}

int32_t Glnk_Set_VideoStream_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen)
{
	printf("%s:%d\n",__func__, __LINE__);
	return 1;
}

int32_t Glnk_Get_VideoStream_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen)
{
	printf("%s:%d\n",__func__, __LINE__);
	return 1;
}

int32_t Glnk_Set_AudioStream_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen)
{
	printf("%s:%d\n",__func__, __LINE__);
	return 1;
}

int32_t Glnk_Get_AudioStream_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen)
{
	printf("%s:%d\n",__func__, __LINE__);
	return 1;
}

int32_t Glnk_Set_Image_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen)
{
	int value;
	char *pStr;
	cJSON* ResultJson = NULL;
	cJSON* Root = NULL;

	printf("[%s][%d]pjsondata = %s\n",__func__,__LINE__,pJsonData);
	Root  = cJSON_Parse(pJsonData);
	if (!Root)
		return 0;

	GK_NET_IMAGE_CFG stImagingConfig;
	memset(&stImagingConfig, 0, sizeof(GK_NET_IMAGE_CFG));
	netcam_image_get(&stImagingConfig);

	value = cJosn_Read_Int(Root,"VerticalRotation");
    if(value != -1)
		stImagingConfig.flipEnabled = value;

	value = cJosn_Read_Int(Root,"HorizontalRotation");
    if(value != -1)
		stImagingConfig.mirrorEnabled = value;

	netcam_image_set(stImagingConfig);

	netcam_timer_add_task(netcam_image_cfg_save, NETCAM_TIMER_TWO_SEC, SDK_FALSE, SDK_TRUE);

	ResultJson = cJSON_CreateObject();
	cJSON_AddTrueToObject(ResultJson, "ReSuc");
	pStr = cJSON_PrintUnformatted(ResultJson);
	strcpy(pResultData, pStr);
	*ResultDataLen = strlen(pResultData);
	free(pStr);
	cJSON_Delete(ResultJson);
	return 1;
}

int32_t Glnk_Get_Image_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen)
{
	char* pStr;
	cJSON* ImageRoot;
	ImageRoot = cJSON_CreateObject();

#ifdef MODULE_SUPPORT_GOKE_UPGRADE
    //if app request flip, refresh now
    goke_upgrade_fresh_now();
#endif

	if (runImageCfg.flipEnabled)
		cJSON_AddTrueToObject(ImageRoot, "VerticalRotation");
	else
		cJSON_AddFalseToObject(ImageRoot, "VerticalRotation");

	if (runImageCfg.mirrorEnabled)
		cJSON_AddTrueToObject(ImageRoot, "HorizontalRotation");
	else
		cJSON_AddFalseToObject(ImageRoot, "HorizontalRotation");

	cJSON_AddNumberToObject(ImageRoot,"Lightness", 8000);
	cJSON_AddNumberToObject(ImageRoot,"Color", 95);
	cJSON_AddNumberToObject(ImageRoot,"Contrast", 38);
	cJSON_AddNumberToObject(ImageRoot,"Saturation", 1024);
	cJSON_AddStringToObject(ImageRoot,"SceneMode", "Outdoor");
	cJSON_AddStringToObject(ImageRoot,"Infrared", "Color");

	pStr = cJSON_PrintUnformatted(ImageRoot);

	strcpy(pResultData, pStr);
	free(pStr);
	cJSON_Delete(ImageRoot);
	goolink_printk("[%s][%d]pResultData = %s",__func__,__LINE__,pResultData);

	return 1;
}

int32_t Glnk_Set_OSD_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen)
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
	return 1;
}

int32_t Glnk_Get_OSD_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen)
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
	return 1;
}

int32_t Glnk_Get_Storage_Opt_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen)
{
	char* pStr;
	cJSON* StorageRoot;

	goolink_printk("[%s][%d]pJsonData = %s",__func__,__LINE__,pJsonData);
	StorageRoot = cJSON_CreateObject();
	cJSON_AddFalseToObject(StorageRoot, "Open");

	cJSON_AddNumberToObject(StorageRoot,"TotalSize", 1024);
	cJSON_AddNumberToObject(StorageRoot,"UsedSize", 998);
	cJSON_AddNumberToObject(StorageRoot,"RemainingSize", 26);

	pStr = cJSON_PrintUnformatted(StorageRoot);

	strcpy(pResultData, pStr);
	free(pStr);
	cJSON_Delete(StorageRoot);
	return 1;
}

int32_t GLNK_DeviceReboot_Callback()
{
	goolink_printk("[%s]\n",__func__);
	netcam_sys_operation(NULL, SYSTEM_OPERATION_REBOOT);
	return 1;
}

int32_t GLNK_FactoryReset_Callback()
{
	goolink_printk("[%s]\n",__func__);
	netcam_sys_operation(NULL, (void*)SYSTEM_OPERATION_HARD_DEFAULT);
	return 1;
}

int32_t Glnk_Set_Smart_Switch_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen)
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
	return 1;
}

int32_t Glnk_Get_Smart_Switch_Callback(char * pJsonData, int32_t JsonDataLen, char *pResultData, int32_t *ResultDataLen)
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
	return 1;
}

int32_t GLNK_Get_EnvOpt_Callback(int32_t *Temperature, int32_t *Humidity)
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
	return 1;
}

int32_t GLNK_AudioFileSearch_CallBack(GLNK_V_SearchAudioFileRequest *SearchFileInfo, GLNK_V_AudioFileInfo **ptr, int32_t *size)
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
	return 1;
}


int32_t GLNK_Audio_Play_Contrl_CallBack(int32_t Type, int32_t PlayMode, char *FileName)
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
	return 1;
}

int32_t GLNK_SetVolume_Callback(char action, char volume)
{
	goolink_printk("action = %d volume = %d\n", (int32_t)action, (int32_t)volume);
	return 1;
}

int32_t GLNK_RCSwitch_Callback(int32_t SType, int32_t SValue)
{
	goolink_printk("GLNK_RCSwitch_Callback >>>>>>>>>>>>>>>>>>>>  SType=%d, SValue=%d \n", SType, SValue);
    switch(SType)
    {
// TODO: add operation here.
        case 4://对应能力集Bomb, app下发命令, 设备喷雾
            //MP4Sys_Set_PWR(SValue, 3*60);
            break;
        case 5://开关人形追踪
            MPU_SetAutoHumanTrace(SValue); //只有set
            break;
        case 6://开关显示蓝色LED灯

			if(strstr(sdk_cfg.name, "CUS_HC_GK7202_GC2053_V10"))
           		RTBlueSetLedEnable(SValue);
			else
				MPU_SetNetLed(SValue); //只有set

            break;
        default:
            break;
    }

    return 1;
}

int32_t GLNK_GetAlarmLogs_Callback(uint32_t LowChannelMask, uint32_t HighChannelMask, int32_t AlarmType, GLNK_DateTime* StartTime, GLNK_DateTime* EndTime, GLNK_V_AlarmInfo *AlarmInfoStr,  int32_t *Sum)
{
	goolink_printk("LowChannelMask = %d HighChannelMask = %d AlarmType = %d  StartTime = %x  EndTime = %x  AlarmInfoStr = %x  Sum = %x\n", LowChannelMask, HighChannelMask, AlarmType, (uint32_t)StartTime, (uint32_t)EndTime, (uint32_t) AlarmInfoStr, (uint32_t)Sum);
	return 1;
}
/*
int32_t GLNK_433DeviceLearnCallback(int32_t Sid, int32_t OperateType, int32_t DeviceType)
{
	printf("Sid = %d  OperateType = %d DeviceType = %d\n", Sid, OperateType, DeviceType);
	return 1;
}

int32_t GLNK_Get433DeviceAddrCodeListCallback(char (*AddrCode)[32],char (*_433Type)[32],int32_t _433DevSum)
{
	int32_t i;
	printf("AddrCode = %x	_433Type = %x _433DevSum = %d\n", (uint32_t)AddrCode,(uint32_t)_433Type ,_433DevSum);
	for(i = 0; i < _433DevSum; i++){
		printf("%s\n", AddrCode[i]);
		printf("%s\n", _433Type[i]);
	}
	return 1;
}*/

int32_t GLNK_OpenListening_Callback(GLNK_AudioDataFormat *audioinfo)
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
	return 1;
}

int32_t GLNK_CloseListening_Callback()
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
	return 1;
}

int32_t GLNK_GetDeviceServer_Callback(char* MDFlag, char* PushFlag, char* emailFlag , char* delayTime)
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
	return 1;
}

int32_t GLNK_SetDeviceServer_Callback(char  MDFlag, char  PushFlag, char emailFlag , char  delayTime)
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
	return 1;
}

int32_t GLNK_GetUserList_Callback(GLNK_V_GetUserListResponse *Rsp)
{
	int i;

	goolink_printk("%s:%d\n",__func__, __LINE__);
	if (!Rsp)
		return 0;

    for (i = 0; i < 1; i++)
	{
        if (runUserCfg.user[i].enable != 0)
		{
            strncpy(Rsp->user[i].username, runUserCfg.user[i].userName,
				sizeof(Rsp->user[i].username));
            strncpy(Rsp->user[i].pwd, runUserCfg.user[i].password,
				sizeof(Rsp->user[i].pwd));
        }
    }

	return 1;
}

int32_t GLNK_SetUserList_Callback(GLNK_V_SetUserListRequest *Req)
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
	if (!Req)
		return 0;

	strncpy(runUserCfg.user[0].userName, Req->user[0].username, sizeof(runUserCfg.user[0].userName));
	strncpy(runUserCfg.user[0].password, Req->user[0].pwd, sizeof(runUserCfg.user[0].password));
	return 1;
}

int32_t GLNK_ResetUnlockPsword_Callback(char* oldpwd, char* newpwd)
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
	strncpy(runUserCfg.user[0].password, newpwd, sizeof(runUserCfg.user[0].password));
	return 1;
}

int32_t GLNK_RTVideoSwitching_Callback(unsigned char channel, unsigned char ismainorsub,GLNK_VideoDataFormat* Invideoinfo,GLNK_VideoDataFormat* Outvideoinfo)
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
	return 1;
}
int32_t GLNK_WaitPlaybackIdle(void)
{
	printf("glnk_send_playback_is_idle:%d, glnk_send_playback_flag:%d, %s, %d\n", glnk_send_playback_is_idle, glnk_send_playback_flag, __func__, __LINE__);
	while(glnk_send_playback_is_idle != 1)
	{
		RTMutex_lock();
		glnk_send_playback_flag = GLINK_PLAYBACK_PAUSE_CMD;
		RTMutex_unlock();
		usleep(30000);
	}

	RTMutex_lock();
	glnk_send_playback_is_idle = 0;
	RTMutex_unlock();
	printf("glnk_send_playback_is_idle:%d, glnk_send_playback_flag:%d, %s, %d\n", glnk_send_playback_is_idle, glnk_send_playback_flag, __func__, __LINE__);
	return 0;
}

int32_t GLNK_PlaybackContrl_Callback(uint8_t type,
											uint8_t value,
											GLNK_DateTime JumpTime,
											int8_t target)
{

    if (type >= 0 && type <=5){
		goolink_printk(" type = %d,value=%d target[%d]\n",type,value, target);
    }
    else{
        goolink_error("GLNK_PlaybackContrl_Callback, type=%d overflow !! \n",type);
		return 0;
    }

	GLNK_WaitPlaybackIdle();
	RTMutex_lock();

	switch(type)
	{
		case GLINK_PLAYBACK_RESUME_CMD :     //继续发送
			goolink_printk("target=%d, type=%s\n",  target, g_ContrlCmd[type]);
			glnk_send_playback_flag = GLINK_PLAYBACK_RESUME_CMD;
		break;

		case GLINK_PLAYBACK_PAUSE_CMD :     //暂停发送
			goolink_printk("target=%d, type=%s\n",  target, g_ContrlCmd[type]);
			glnk_send_playback_flag = GLINK_PLAYBACK_PAUSE_CMD;
		break;

		case GLINK_PLAYBACK_CLOSE_CMD : //关闭
			goolink_printk("target=%d, type=%s\n",  target, g_ContrlCmd[type]);
			glnk_send_playback_flag = GLINK_PLAYBACK_CLOSE_CMD;
		break;

		case GLINK_PLAYBACK_PLUS_CMD :  //快进
			goolink_printk("target=%d, type=%s\n",  target, g_ContrlCmd[type]);
			glnk_playback_frame_interval >>= value;
			glnk_send_playback_flag = GLINK_PLAYBACK_PLUS_CMD;
		break;

		case GLINK_PLAYBACK_MINUS_CMD : //慢放
			goolink_printk("target=%d, type=%s\n",  target, g_ContrlCmd[type]);
			glnk_playback_frame_interval <<= value;
			glnk_send_playback_flag = GLINK_PLAYBACK_MINUS_CMD;
		break;

		case GLINK_PLAYBACK_JUMP_CMD :   //拖动跳转
			goolink_printk("target=%d, type=%s, y:%d,m:%d, d:%d, h:%d,m:%d, s:%d\n",
			    target, g_ContrlCmd[type], JumpTime.m_year, JumpTime.m_month,
			    JumpTime.m_day, JumpTime.m_hour, JumpTime.m_minute, JumpTime.m_second);
			memset(&timePos_start,0,sizeof(timePos_start));
			glnk_send_playback_flag = GLINK_PLAYBACK_JUMP_CMD;
			timePos_start.dwYear    = JumpTime.m_year;
			timePos_start.dwMonth   = JumpTime.m_month;
			timePos_start.dwDay     = JumpTime.m_day;
			timePos_start.dwHour    = JumpTime.m_hour;
			timePos_start.dwMinute  = JumpTime.m_minute;
			timePos_start.dwSecond  = JumpTime.m_second;
		break;

		default:
			break;
	}

	glnk_send_playback_target = target;
	RTMutex_unlock();

	return 1;
}

int32_t GLNK_PlaybackClose_CallBack(int8_t target)
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
	GLNK_WaitPlaybackIdle();
	RTMutex_lock();
	glnk_send_playback_flag = GLINK_PLAYBACK_CLOSE_CMD;
	RTMutex_unlock();

	return 1;
}


static void RTVideoDataFormatInfo(GLNK_VideoDataFormat* videoinfo, int streamId) //补充主子码流参数
{
    //为了动态适应, 要从底层获取再设置
    if (1 == runVideoCfg.vencStream[streamId].enctype)
    {
        videoinfo->codec = GLNK_CODEC_H264;
    }
    else if (3 == runVideoCfg.vencStream[streamId].enctype)
    {
        videoinfo->codec = GLNK_CODEC_H265;
    }
    else
    {
        videoinfo->codec = GLNK_CODEC_H265;
    }

	videoinfo->bitrate		 = runVideoCfg.vencStream[streamId].h264Conf.bps;
	videoinfo->width		 = runVideoCfg.vencStream[streamId].h264Conf.width;
	videoinfo->height		 = runVideoCfg.vencStream[streamId].h264Conf.height;
	videoinfo->framerate	 = runVideoCfg.vencStream[streamId].h264Conf.fps;
	videoinfo->colorDepth	 = 24;
	videoinfo->frameInterval = runVideoCfg.vencStream[streamId].h264Conf.gop;



}


int32_t GLNK_PlaybackOpen_CallBack(int8_t target,
								int8_t channel,
								GLNK_DateTime StartTime,
								uint8_t *StreamTypeResponse ,
								GLNK_VideoDataFormat* VideoinfoResponse,
								GLNK_AudioDataFormat* AudioinfoResponse,
								int32_t *FileTotalTimeResponse)
{

	goolink_printk("[%s][%d]  target = %d,channel = %d time = %d-%d-%d %d:%d:%d:%d\n",__func__,__LINE__,
		target,channel,StartTime.m_year,StartTime.m_month,StartTime.m_day,
		StartTime.m_hour,StartTime.m_minute,StartTime.m_second,StartTime.m_microsecond);

	int ret = 1;
	struct tm stuBeginTime, stuEndTime;
	time_t iBeginSec, iEndSec;
	GLNK_WaitPlaybackIdle();
	RTMutex_lock();

	*StreamTypeResponse = 2;			//音视频都有

	//从文件解析获取帧率和分辨率
    RTVideoDataFormatInfo(VideoinfoResponse, 0); //最好还是要从文件中解析出来

	AudioinfoResponse->samplesRate   = runAudioCfg.sampleRate;
	AudioinfoResponse->bitrate       = 16;
	AudioinfoResponse->waveFormat    = GLNK_WAVE_FORMAT_G711;
	AudioinfoResponse->channelNumber = 1;
	AudioinfoResponse->blockAlign    = 1;
	AudioinfoResponse->bitsPerSample = 16;
	AudioinfoResponse->frameInterval = 25;
	AudioinfoResponse->reserve       = 0;



    //20180522 现在必须要强制成[00:00:00 ~ 23:59:59], 因为底层的缓存是根据起始时间段建立的, 每次搞不同的起始时间可能导致缓存找不到.
    //在这里强制播放[00:00:00 ~ 23:59:59]

    timePlayBackStart.dwYear   = StartTime.m_year;
    timePlayBackStart.dwMonth  = StartTime.m_month;
    timePlayBackStart.dwDay    = StartTime.m_day;
    timePlayBackStart.dwHour   = StartTime.m_hour;
    timePlayBackStart.dwMinute = StartTime.m_minute;
    timePlayBackStart.dwSecond = StartTime.m_second;



    stuBeginTime.tm_sec  = 0;
    stuBeginTime.tm_min  = 0;
    stuBeginTime.tm_hour = 0;
    stuBeginTime.tm_mday = StartTime.m_day;
    stuBeginTime.tm_mon  = StartTime.m_month - 1;
    stuBeginTime.tm_year = StartTime.m_year - 1900;

    stuEndTime.tm_sec  = 59;
    stuEndTime.tm_min  = 59;
    stuEndTime.tm_hour = 23;
    stuEndTime.tm_mday = StartTime.m_day;
    stuEndTime.tm_mon  = StartTime.m_month - 1;
    stuEndTime.tm_year = StartTime.m_year - 1900;


	//mktime之前得到的是零散的时间
	iBeginSec = mktime(&stuBeginTime);
	iEndSec   = mktime(&stuEndTime);
	* FileTotalTimeResponse   = (iEndSec - iBeginSec) * 1000;  //单位:ms
	//*FileTotalTimeResponse = 2122000ULL;

    //20180522 这里一定要下发[00:00:00 ~ 23:59:59], 因为libSTR中的缓冲就是根据时间段来建立的, 要是起始时间不同缓存就不同, 就可能找不到
    //改成按照时间回放后, 每次下发框住的时间是[00:00:00 ~ 23:59:59], 所以要将type设置成全部, 否则按照顺序播放会跳过



	glnk_send_playback_target = target;

	//goolink_printk("start time:%llu,stop time:%llu \n", glnkAviPbhandle->list->start,glnkAviPbhandle->list->stop);
	//goolink_printk("no:%d,video_no:%d,audio_no:%d,fps:%d,video_count:%d,index_count:%d,idx_array_count:%d,width:%d,height:%d\n",
		//glnkAviPbhandle->no, glnkAviPbhandle->video_no,glnkAviPbhandle->audio_no,glnkAviPbhandle->fps,
		//glnkAviPbhandle->video_count,glnkAviPbhandle->index_count,glnkAviPbhandle->idx_array_count,
		//glnkAviPbhandle->video_width,glnkAviPbhandle->video_height);

	glnk_send_playback_flag = GLINK_PLAYBACK_RESUME_CMD;
	RTMutex_unlock();

	return ret;
}

int32_t GLNK_PlaybackByFileorTime_Callback(int32_t *FileorTime_Flags)
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
	*FileorTime_Flags = 1;
	return 1;
}

#if 0
int32_t GLNK_WifiChangePassWord_Callback(TLV_V_ChangeAPPasswordRequest *Req)
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
	return 1;
}
#endif

int32_t GLNK_GetStorageList_Callback(GLNK_DeviceStorageResponse **StorageList)
{
    int sdStatus = 0;
	goolink_printk("%s:%d\n",__func__, __LINE__);

	GLNK_DeviceStorageResponse *List = malloc(sizeof(GLNK_DeviceStorageResponse) + sizeof(GLNK_DeviceStorageList));
	memset(List,0,sizeof(GLNK_DeviceStorageResponse) + sizeof(GLNK_DeviceStorageList));

	List->DeviceStorageList[0].StorageID        = 1;
	List->DeviceStorageList[0].StorageCap       = grd_sd_get_all_size_last();
	List->DeviceStorageList[0].StorageCapRemain = grd_sd_get_free_size_last();
	List->DeviceStorageListLen = sizeof(GLNK_DeviceStorageResponse) + sizeof(GLNK_DeviceStorageList);

    sdStatus = grd_sd_check_device_node();
    if (List->DeviceStorageList[0].StorageCap == 0 && (sdStatus == -2 || sdStatus == 0))
    {
        List->DeviceStorageList[0].StorageCapRemain = 0;
        List->DeviceStorageList[0].StorageCap = 1024;
    }
	*StorageList = List;

	return 1;
}

int32_t GLNK_GetSwitchOfDoor_Callback(unsigned char channel, char *action)
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
	return 1;
}

int32_t GLNK_SetAlarmSwtich_Callback( int32_t channel,int32_t reserve)
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
	return 1;
}


int32_t GLNK_PlaybackSearch_CallBack(GLNK_SearchPlaybackRequest *AppRequest,
										GLNK_SearchPlaybackReply **PlayBackResponse,int32_t *Num)
{
	int ret                = 0;
	int i                  = 0;
	int j                  = 0;
	int lst_len            = 0;
	int count              = 0;
	u64t start             = 0;
    u64t stop              = 0;
	int motion_len         = 0;
	FILE_LIST *flist       = NULL;
	AVI_DMS_TIME avi_start;
	AVI_DMS_TIME avi_stop;
	GLNK_SearchPlaybackReply *FileInfo = NULL;
	FILE_NODE node;

	goolink_printk("channelMask:0x%x, recordTypeMask:0x%x\n", AppRequest->channelMask, AppRequest->recordTypeMask);
	goolink_printk("starttime = %d-%d-%d %d:%d:%d:%d-----",AppRequest->startTime.m_year,
		AppRequest->startTime.m_month,AppRequest->startTime.m_day,AppRequest->startTime.m_hour,
		AppRequest->startTime.m_minute,AppRequest->startTime.m_second,AppRequest->startTime.m_microsecond);
	goolink_printk("endtime = %d-%d-%d %d:%d:%d:%d\n",AppRequest->endTime.m_year,
		AppRequest->endTime.m_month,AppRequest->endTime.m_day,AppRequest->endTime.m_hour,
		AppRequest->endTime.m_minute,AppRequest->endTime.m_second,AppRequest->endTime.m_microsecond);

	memset(&avi_start,0,sizeof(avi_start));
	memset(&avi_stop,0,sizeof(avi_stop));

	avi_start.dwYear   = AppRequest->startTime.m_year;
	avi_start.dwMonth  = AppRequest->startTime.m_month;
	avi_start.dwDay    = AppRequest->startTime.m_day;
	avi_start.dwHour   = AppRequest->startTime.m_hour;
	avi_start.dwMinute = AppRequest->startTime.m_minute;
	avi_start.dwSecond = AppRequest->startTime.m_second;

	avi_stop.dwYear    = AppRequest->endTime.m_year;
	avi_stop.dwMonth   = AppRequest->endTime.m_month;
	avi_stop.dwDay     = AppRequest->endTime.m_day;
	avi_stop.dwHour    = AppRequest->endTime.m_hour;
	avi_stop.dwMinute  = AppRequest->endTime.m_minute;
	avi_stop.dwSecond  = AppRequest->endTime.m_second;

	start = time_to_u64t(&avi_start);
    stop  = time_to_u64t(&avi_stop);

	flist   = search_file_by_time(0, 0xFF, start, stop);
	if (flist)
	{
		lst_len = get_len_list(flist);
		goolink_printk("%s:%d start :%llu,stop:%llu, len:%d\n",__func__, __LINE__,start,stop,lst_len);
		if (lst_len == 0){
			search_close(flist);
			flist = NULL;
			goto out;
		}
	}
	else
	{
		lst_len = 0;
		goto out;
	}



	//申请的空间值被设置成0
	FileInfo = (GLNK_SearchPlaybackReply *)calloc(lst_len,sizeof(GLNK_SearchPlaybackReply));
	if(FileInfo == NULL)
	{
		goolink_error("Fail to calloc. GLNK_PlaybackSearch_CallBack\n");
		search_close(flist);
		return 0;
	}

  	char timeStr[32];
	char index_file[128];
	int year,month,day;

	memset(timeStr,0,sizeof(timeStr));

	sprintf(timeStr,"%llu",start);
	sscanf(timeStr,"%04d%02d%02d",&year,&month,&day);

	memset(index_file, 0, sizeof(index_file));
	sprintf(index_file, "%s/%04d%02d%02d/.motion", GRD_SD_MOUNT_POINT,year,month,day);

	FILE *fp = NULL;
	int motionCount = 0;
	int fileSize    = 0;
	int fileCur     = 0;
	INDEX_NODE motionNode;
	u64t motion_begin = 0;
	u64t motion_end   = 0;
    int motionFindLast = 1;

	goolink_printk("open filename=%s\n\n", index_file);
    fp = fopen(index_file, "rb");
	if (fp != NULL)
	{
	    fseek(fp, 0, SEEK_SET);
	    fseek(fp, 0, SEEK_END);
		fileSize = ftell(fp);
		fseek(fp, 0, SEEK_SET);
		fseek(fp, sizeof(int),SEEK_SET);
		motionCount = (fileSize-sizeof(int))/sizeof(INDEX_NODE);
		goolink_printk("motionCount =%d\n\n", motionCount);
	}
	else
	{
		goolink_error("fopen %s error!\n", index_file);
	}

	for (i = 0; i < lst_len; i++)
	{
		memset(&node, 0, sizeof(FILE_NODE));
		if (get_file_node(flist, &node) != DMS_NET_FILE_SUCCESS)
		{
			goolink_error("%s:%d Fail to get file node!\n", __func__, __LINE__);
			continue;
		}

		u64t_to_time(&avi_start, node.start);
		u64t_to_time(&avi_stop, node.stop);
		FileInfo[i].channel    = flist->ch_num;
		FileInfo[i].deviceId   = 0;
		FileInfo[i].recordType = 0x04;

		for(j = 0; j < motionCount; j++)
		{
			if((fp != NULL) && (feof(fp) == 0))
			{
                if (motionFindLast)
                {
    				memset(&motionNode, 0, sizeof(INDEX_NODE));
    				fread(&motionNode, sizeof(INDEX_NODE), 1, fp);
                }
				//goolink_printk("%d---motionNode.start = %llu, motionNode.stop = %llu\n", motionFindLast, motionNode.start, motionNode.stop);
				//goolink_printk("-hex--motionNode.start = 0x%llx, motionNode.stop = 0x%llx\n", motionNode.start, motionNode.stop);
			    if((start <= motionNode.start) && (motionNode.stop <= stop))
		        {
		            motion_begin = motionNode.start;
		            motion_end = motionNode.stop;
		        }
		        else if((motionNode.start <= start) && (stop <= motionNode.stop))
		        {
		            motion_begin = start;
		            motion_end = stop;
		        }
		        else if((motionNode.start <= start) && (start <= motionNode.stop))
		        {
		            motion_begin = start;
		            motion_end = motionNode.stop;
		        }
		        else if((motionNode.start <= stop) && (stop <= motionNode.stop))
		        {
		            motion_begin = motionNode.start;
		            motion_end = stop;
		        }
				//goolink_printk("---motion_begin = %llu, motion_end = %llu\n", motion_begin, motion_end);
				//goolink_printk("----node.start   = %llu, node.stop    = %llu\n", node.start, node.stop);



				if((motionNode.start >= node.start && motionNode.start < node.stop) ||
					(motionNode.stop > node.start && motionNode.stop <= node.stop) ||
					(node.start >= motionNode.start && node.stop <= motionNode.stop))
				{
					FileInfo[i].recordType = 0x02;// motiondect
					if (motionNode.stop <= node.stop)
					{
					    motionFindLast = 1;
					}
                    else
                    {
					    motionFindLast = 0;
                    }
					break;
				}
				#if 0
				if((node.start <= motion_begin) &&
					(node.stop >= motion_end))
				{
					//goolink_printk("motion_begin = %llu, motion_end = %llu\n", motion_begin, motion_end);
					//goolink_printk("node.start   = %llu, node.stop    = %llu\n", node.start, node.stop);
					FileInfo[i].recordType = 0x02;// motiondect
					break;
				}
				#endif
				else if (motion_begin > node.stop)
				{

                    motionFindLast = 0;
					//fseek(fp, -sizeof(INDEX_NODE), SEEK_CUR);
					break;
				}
                else
                {
                    motionFindLast = 1;
                }
			}
            else
            {
                break;
            }
		}


		FileInfo[i].startTime.m_year   = avi_start.dwYear;
		FileInfo[i].startTime.m_day    = avi_start.dwDay;
		FileInfo[i].startTime.m_month  = avi_start.dwMonth;
		FileInfo[i].startTime.m_hour   = avi_start.dwHour;
		FileInfo[i].startTime.m_minute = avi_start.dwMinute;
		FileInfo[i].startTime.m_second = avi_start.dwSecond;
		FileInfo[i].endTime.m_year     = avi_stop.dwYear;
		FileInfo[i].endTime.m_day      = avi_stop.dwDay;
		FileInfo[i].endTime.m_month    = avi_stop.dwMonth;
		FileInfo[i].endTime.m_hour     = avi_stop.dwHour;
		FileInfo[i].endTime.m_minute   = avi_stop.dwMinute;
		FileInfo[i].endTime.m_second   = avi_stop.dwSecond;
	}
	if (fp)
	{
		fclose(fp);
		fp = NULL;
	}

	count += lst_len;

	*PlayBackResponse = FileInfo;
	*Num = count;

	if(flist )
	{
		search_close(flist);
		flist = NULL;
	}

	goolink_printk("%s:%d *Num:%d\n",__func__, __LINE__,*Num);
	return 1;

out:

	if(flist )
	{
		search_close(flist);
		flist = NULL;
	}


	return 0;
}

int32_t GLNK_FormatStorage_Callback(int32_t StorageID,int32_t *result)
{
	int ret = -1;
	int count = 120; //给一个超时时间
	goolink_printk("[%s][%d] StorageID = %d\n",__func__,__LINE__,StorageID);

	ret = grd_sd_format();
	if (ret == 0)// format ok
	{
		while(count-- > 0){
			sleep(1);
			if (mmc_get_sdcard_stauts() == GLINK_SD_STATUS_OK){
				break;
			}
		}
		*result = 1;
	}
	else // format failed
	{
		*result = 0;
	}

	return 1;
}

int32_t GLNK_GetAlarmSwtich_Callback( int32_t *channel,int32_t *reserve,int32_t *channelnum)
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
	return 1;
}

int32_t GLNK_Get_RCType_Callback(const int32_t SType, int32_t *SValue)
{
	goolink_printk("%s:%d\n",__func__, __LINE__);
	return 1;
}

#define FILE_SNAPSHOT "/tmp/snapshot.jpg"
int32_t GLNK_GetPicture(char **Data,int *len)
{
    goolink_printk("GLNK_GetPicture >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> \n");
    remove(FILE_SNAPSHOT);

    int ret = 0;
    char path[50];
    memset(path, 0, sizeof(path));
    snprintf(path, 50, FILE_SNAPSHOT);

    ret = netcam_video_snapshot(runVideoCfg.vencStream[1].h264Conf.width, runVideoCfg.vencStream[1].h264Conf.height, path, GK_ENC_SNAPSHOT_QUALITY_MEDIUM);
    if (ret != 0)
    {
        printf("GLNK_GetPicture: snap shot failed.");
        return 0;
    }

	struct stat jpg_buf;
    memset(&jpg_buf, 0, sizeof(struct stat));
	stat(FILE_SNAPSHOT,&jpg_buf);
    if (jpg_buf.st_size < 5){
        goolink_error("GLNK_GetPicture, snapshot size=%d !! invalid, return!! \n", jpg_buf.st_size);
        return 0;
    }

    //分配空间
    unsigned char *pfiledata = (unsigned char *)malloc(jpg_buf.st_size);
    if (NULL == pfiledata){
        goolink_error("GLNK_GetPicture, malloc size %d failed! \n", jpg_buf.st_size);
        return 0;
    }

    //打开文件准备拷贝数据
    FILE *jpeg_fd = fopen(FILE_SNAPSHOT, "r");
    if(NULL == jpeg_fd){
        free(pfiledata);
        goolink_error("GLNK_GetPicture, open file failed: %s return !!\n", FILE_SNAPSHOT);
        return 0;
    }
	goolink_printk("GLNK_GetPicture, jpeg snapshot pfiledata=%p, size = %d\n", pfiledata, jpg_buf.st_size);
    fread(pfiledata, jpg_buf.st_size, 1, jpeg_fd);
    fclose(jpeg_fd);

    *len = jpg_buf.st_size;
	*Data = (char *)pfiledata;

    goolink_printk("GLNK_GetPicture <<<<<<<<<<<<<<<<<<<<<< *len=%d, *Data=%p\n", *len, *Data);

	return 1;
}

int32_t GLNK_UploadPanoModel_Callback(int32_t ConnectionID,char *filename,int *filesize)
{
    printf("%s:%d\n",__func__, __LINE__);
	return 1;
}

//对接新回放协议时, 一定要在此补充能力集.
int32_t GLNK_GetDeviceInfo_Callback(char *Data,int32_t *len)
{
    goolink_printk("%s:%d\n",__func__, __LINE__);
    uint32_t dwUserID = -1, nSize = 0;
    int ret = 0;
    char *szDate = NULL;

    /*
    SD卡			 	“SDCard”:			    "int8_t"	(0--无sd卡，1--有sd卡)
    PTZ				    “PTZ”:				"int8_t"	(0--无，1--摇头，2--带光学变焦)
    鱼眼				“FishEye” :			"int8_t"	(0--无，1--有)
    720全景			    “Panorama”:			"int8_t"	(0--无，1--有)
    产品形态			“ProductType”:		"int8_t"	(1--IPC，2--NVR，3--门铃，4--报警网关)
    电池				“Battery”	:		    "int8_t"	(0--无，1--有)
    码流情况			“StreamingClass”:	    "int8_t"	(2--两种，3--三种)
    视频编码类型		“VideoEncoderType”:	"int8_t"	(4--h264，5--h265，0--两种都支持)
    是否支持回音消除	“EchoCancel”:		    "int8_t"	(0--不支持，1--支持)
    是否支持音频全双工	“Full_Duplex”		    "int8_t"	(0--不支持，1--支持)
    全景参数		 	“PanoData”：		    "string" 	(全景参数，这串数据由GLNK_GetPanoData收到的值给他赋值 )
    通道数			    “Channels”		    "int8_t"	(通道数)
    版本日期			“SDKVersion”:		    "string"	(sdk版本号，由glnk_get_version返回，是一个16进制数的日期号)
    固件版本			“Firmware”:		    "string"	(同GLNK_GetVersionFirmware_Callback,用|隔开)
    全景模板版本		“PanoModelVersion”    "string"	（当GLNK_DownLoadPanoModel_Callback生成新模板时要更新此参数
    是一个UTC时间，即模板生成时间)
    */

    char* pStr;
    cJSON* jsDevInfo;
    JS_HEAP_OBJ_MGR heapobjmgr = {0};
    jsDevInfo = cJSON_CreateObjectEx(&heapobjmgr);

#if 0
    switch(boardinfo.BoardFormat)
    {
        case TYPE_FH56W16F:
            {
                cJSON_AddNumberToObject(jsDevInfo,"PTZ", 1); //带摇头
                cJSON_AddNumberToObject(jsDevInfo,"Bulb", 1); //新版摇头机也有白灯
                cJSON_AddNumberToObject(jsDevInfo,"humantrace", 1);
                break;
            }
        default:
            break;
    }
#else
	if(sdk_cfg.ptz.enable)
    	cJSON_AddNumberToObject(jsDevInfo,"PTZ", 1); //带摇头
    else
    	cJSON_AddNumberToObject(jsDevInfo,"PTZ", 0); //带摇头

	if(strstr(sdk_cfg.name, "CUS_HC_GK7202_GC2053_V10") 
        || strcmp(runSystemCfg.deviceInfo.deviceType, "GK7202_GC2053_TB_38_BIG_V3") == 0
        || strcmp(runSystemCfg.deviceInfo.deviceType, "GK7202_GC2053_TB_38_BIG_V4") == 0)
    	cJSON_AddNumberToObject(jsDevInfo,"Bulb", 1); //新版摇头机也有白灯
	else
		cJSON_AddNumberToObject(jsDevInfo,"Bulb", 0);

    //cJSON_AddNumberToObject(jsDevInfo,"humantrace", 1); // open it when humantrace ready by david
    //cJSON_AddNumberToObject(jsDevInfo,"Bomb", 1); //投掷烟雾
    cJSON_AddNumberToObject(jsDevInfo,"AF", 0); //自动变焦AF支持
#endif

    //加入能力集的原因是之前的版本没有这个功能时, app上访问会出现卡死的情况, 故能力集还是也在本次新加进来, 免得客户困惑为什么显示了但不能控制
    if(strstr(sdk_cfg.name, "CUS_HC_GK7202_GC2053_V10"))
		cJSON_AddNumberToObject(jsDevInfo,"netled", 1); //控制LED蓝灯的显示与否, 只在稳定连接时有效; 20190226 所有版本都要有
	else
		cJSON_AddNumberToObject(jsDevInfo,"netled", 0);

    //下面是公共部分, 所有产品都有的能力集:
    cJSON_AddNumberToObject(jsDevInfo,"SDCard", 0); //全部都带SDcard
    cJSON_AddNumberToObject(jsDevInfo,"PlaybackByFileorTime", 1); //20180118以后全部采用新时间回放
    cJSON_AddNumberToObject(jsDevInfo,"ProductType", 1); //1表示IPC
    cJSON_AddNumberToObject(jsDevInfo,"Battery", 0); //电池
    cJSON_AddNumberToObject(jsDevInfo,"StreamingClass", 2); //2路码流
#if 0
    if (1 == runVideoCfg.vencStream[0].enctype)
    {
        cJSON_AddNumberToObject(jsDevInfo, "VideoEncoderType", 4);
    }
    else if (3 == runVideoCfg.vencStream[0].enctype)
    {
        cJSON_AddNumberToObject(jsDevInfo, "VideoEncoderType", 5);
    }
    else
    {
        cJSON_AddNumberToObject(jsDevInfo, "VideoEncoderType", 5);
    }
#endif
    cJSON_AddNumberToObject(jsDevInfo, "VideoEncoderType", 0);
    //cJSON_AddNumberToObject(jsDevInfo,"VideoEncoderType", g_encH265?5:4); //H264, 改为同时支持
    cJSON_AddNumberToObject(jsDevInfo,"EchoCancel", 0); //回声消除
    cJSON_AddNumberToObject(jsDevInfo,"Channels", 1); //通道数

    //sdk日期
    char sdk_version[VERSION_FILED_MAX_SIZE];
    memset(sdk_version, 0, VERSION_FILED_MAX_SIZE);
    sprintf(sdk_version, "%08x", glnk_get_version());
    cJSON_AddStringToObject(jsDevInfo,"SDKVersion", sdk_version); //sdk版本号,直接函数获取

    //获取版型
    char appbuf[VERSION_FILED_MAX_SIZE_TOTAL];
    char solbuf[VERSION_FILED_MAX_SIZE_TOTAL];
    char date[VERSION_FILED_MAX_SIZE_TOTAL];
    char hardware[64];
    char final_version[VERSION_FILED_MAX_SIZE_TOTAL*3+64];
    memset(appbuf, 0, sizeof(appbuf));
    memset(solbuf, 0, sizeof(solbuf));
    memset(date, 0, sizeof(date));
    memset(hardware, 0, sizeof(hardware));
    memset(final_version, 0, sizeof(final_version));
    GLNK_GetVersionFirmware_Callback(appbuf, solbuf, date, hardware);
    sprintf(final_version, "%s|%s|%s|%s", appbuf, solbuf, date, hardware);
    cJSON_AddStringToObject(jsDevInfo,"Firmware", final_version); // Glnk_GetVersionFirmware_Callback

    /////////////////////////////////////////////////////////////////////////////////////
    //上面填充好了, 下面开始格式化成字符串

    JsToBuffer(jsDevInfo, Data);
    *len = strlen(Data);

    JsHeapObjFree(&heapobjmgr);

    return 0;
}

/*	获取设备灯泡模式接口
 *  mode ：			（回填的数据）模式   0--	红外  1--	全彩  2--	智能  3--	手动
 *  key	 :			（回填的数据）亮度 0 - 100（仅手动模式下有效）
 *  return：			0--失败，1--成功	*/
int32_t Glnk_GetBulbMode(int *mode,int *key)
{
    goolink_printk("%s:%d\n",__func__, __LINE__);
    //*mode = sdk_isp_get_bulb_mode();
    //*key = sdk_isp_get_bulb_level();
	if(strstr(sdk_cfg.name, "CUS_HC_GK7202_GC2053_V10")
        || strcmp(runSystemCfg.deviceInfo.deviceType, "GK7202_GC2053_TB_38_BIG_V3") == 0
        || strcmp(runSystemCfg.deviceInfo.deviceType, "GK7202_GC2053_TB_38_BIG_V4") == 0)
	{
	    *mode = runImageCfg.lightMode;
	    *key = runImageCfg.lightLevel;
	}
    printf("Glnk_GetBulbMode mode=%d, key=%d <<<<<<<<<<<<<<\n", *mode, *key);
	return 1;
}

/*	app设置设备灯泡模式接口
 *  mode ：			模式  0--	红外  1--	全彩  2--	智能  3--	手动
 *  key	 :			亮度 0 - 100（仅手动模式下有效）
 *  return：			0--失败，1--成功	*/
int32_t Glnk_SetBulbMode(int mode,int key)
{
    int cfgSave = 0;
    goolink_printk("%s:%d\n",__func__, __LINE__);
	if(strstr(sdk_cfg.name, "CUS_HC_GK7202_GC2053_V10")
        || strcmp(runSystemCfg.deviceInfo.deviceType, "GK7202_GC2053_TB_38_BIG_V3") == 0
        || strcmp(runSystemCfg.deviceInfo.deviceType, "GK7202_GC2053_TB_38_BIG_V4") == 0)
	{
	    if ((mode == runImageCfg.lightMode) && (mode != ISP_BULB_MODE_MANUAL))
	    {
	        goolink_printk("same mode, need not switch\n");
	        return 1;
	    }

	    sdk_isp_set_bulb_mode(mode);

	    if (mode == ISP_BULB_MODE_MANUAL)
	    {
	    	if (runImageCfg.lightLevel != key)
			{
				sdk_isp_set_bulb_level(key);
	        	runImageCfg.lightLevel = key;
	            cfgSave = 1;
			}
	    }

		if (runImageCfg.lightMode != mode)
		{
			runImageCfg.lightMode = mode;
	        cfgSave = 1;
		}

        if (mode == ISP_BULB_MODE_MIX && netcam_md_play_get_cnt() == 0)
        {
            //在app里设置灯泡模式为智能，且没有设置过播放次数时，手动打开一次
	        netcam_timer_add_task(netcam_md_play_start_one, NETCAM_TIMER_ONE_SEC * 4, SDK_FALSE, SDK_TRUE);
        }
        
	    if(1 == cfgSave)
	    {
	        netcam_timer_add_task(netcam_image_cfg_save, NETCAM_TIMER_ONE_SEC, SDK_FALSE, SDK_TRUE);
	    }
	}
	return 1;
}

int32_t GLNK_GetPanoData(char *Data,int32_t len)
{
    goolink_printk("%s:%d\n",__func__, __LINE__);
	return 1;
}

int32_t GLNK_DownLoadPanoModel_Callback(char *filename,int type,int size,char *data,int len)
{
    goolink_printk("%s:%d\n",__func__, __LINE__);
	return 1;
}

unsigned char  GLNK_PwdAuthWithChannel_Callback(char* username, char* pwd, int32_t channel,int32_t sid)
{
	goolink_printk("%s:%d phone app [username:%s  password:%s] | local [username:%s  password:%s]\n",
	__func__,__LINE__,username,pwd,runUserCfg.user[0].userName,runUserCfg.user[0].password);

	//LT_voice_restore_init_status(0);

	if( strlen(runUserCfg.user[0].userName) == strlen(username)
    && strlen(runUserCfg.user[0].password) == strlen(pwd))
    {
        if(strcmp(runUserCfg.user[0].userName, username) == 0
    	&& strcmp(runUserCfg.user[0].password, pwd) == 0)
    	{
			if (glnk_playback_thread_video_exit_flag == 0)
				glnk_playback_thread_video_exit_flag = 1;

			if (glnk_playback_thread_audio_exit_flag == 0)
				glnk_playback_thread_audio_exit_flag = 1;

    		return 1;
    	}
    }

	return 0;
}

void GLNK_UpdateRelaseResource(void)
{
	int ret = 0;

	ret = RTExitVoiceInfoThread();
	goolink_printk("%s:%d, RTExitVoiceInfoThread ret:%d\n",__func__, __LINE__, ret);
	ret = RTExitPlayBackThread();
	goolink_printk("%s:%d, RTExitPlayBackThread ret:%d\n",__func__, __LINE__, ret);
	ret = RTExitAudioFrameThread();
	goolink_printk("%s:%d, RTExitAudioFrameThread ret:%d\n",__func__, __LINE__, ret);
	ret = RTExitSendVideoFrameStream();
	goolink_printk("%s:%d, RTExitSendVideoFrameStream ret:%d\n",__func__, __LINE__, ret);
	usleep(500000);
}

