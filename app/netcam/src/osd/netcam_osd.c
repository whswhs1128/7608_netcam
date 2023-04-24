/*!
*****************************************************************************
** file        netcam_osd.c
**
** version     2016-05-11 16:27 heyong -> v1.0
**
** brief       the implementation of netcam osd.
**
** attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2016-2020 BY GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include "sdk_debug.h"
#include "sdk_api.h"
#include "netcam_api.h"
#include "cfg_com.h"
#include "common.h"
#include "osd.h"
#include "avi_rec.h"

#define NETCAM_OSD_TITLE_AREA   0
#define NETCAM_OSD_CLOCK_AREA   1
#define NETCAM_OSD_ID_AREA      2

char device_local_time[128] = {""};
char device_local_week = 0;

static pthread_rwlock_t overlay_lock;
static pthread_t osd_pid = 0;
static int osd_running = 0;

static inline int osd_enter_lock(void)
{
	return pthread_rwlock_wrlock(&overlay_lock);
}

static int osd_leave_lock(void)
{
	return pthread_rwlock_unlock(&overlay_lock);
}

static void *osd_time_refresh(void *arg)
{
    sdk_sys_thread_set_name("osd_time_refresh");
	while (osd_running)
    {
        netcam_osd_update_clock();
        usleep(300*1000);
    }
	return NULL;
}

int netcam_osd_init()
{
    if(!osd_running)
    {
        osd_running = 1;
        osd_init();
        pthread_rwlock_init(&overlay_lock, NULL);

        int i=0;
        for (i = 0; i < netcam_video_get_channel_number(); i++) {
            //set default title and id.
            //netcam_osd_set_title(i, (char *)runVideoCfg.vencStream[i].h264Conf.name);
            //netcam_osd_set_title(i, (char *)runChannelCfg.channelInfo[i].osdChannelName.text);
            //netcam_osd_set_id(i, "ID");
        }
        netcam_osd_update_title();
        netcam_osd_update_id();
        #if 0
    	netcam_timer_add_task(netcam_osd_update_clock, NETCAM_TIMER_HALF_ONE_SEC, SDK_TRUE, SDK_FALSE);
    	//netcam_timer_add_task(netcam_osd_update_title, NETCAM_TIMER_ONE_SEC, SDK_TRUE, SDK_FALSE);
    	//netcam_timer_add_task(netcam_osd_update_id, 1, SDK_TRUE, SDK_FALSE);
    	#else
    	if (pthread_create(&osd_pid, NULL, osd_time_refresh, NULL) != 0)
    	{
    		LOG_ERR("osd create time failed\n");
    	}
        #endif
    }
	return 0;
}

int netcam_osd_deinit()
{
    LOG_INFO("osd exit");
    #if 0
    netcam_timer_del_task(netcam_osd_update_clock);    
    usleep(500*1000);// wait osd clock update over
    #else
    osd_running = 0;
    if (osd_pid)
    {
		pthread_join(osd_pid, NULL);
		osd_pid = 0;
	}
    #endif
	LOG_INFO("osd exit2");
	
	osd_enter_lock();
    osd_deinit();
	osd_leave_lock();
	
	LOG_INFO("osd exit3");
    pthread_rwlock_destroy(&overlay_lock);
	LOG_INFO("osd exit4");
	
    return 0;
}

static char* weekday_map_chs[] = {
    "星期日", "星期一", "星期二", "星期三", "星期四", "星期五", "星期六",};
static char* weekday_map_eng[] = {
    "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT",};
static int _osd_clock_format(int ch, time_t t, char *clock_str)
{
    char date_buf[128] = {0}, time_buf[128] = {0};
    const char *meridiem = NULL;
    const char *weekday = NULL;

    struct tm tm_now = {0};
    int year = 0, month = 0, mday = 0, hour = 0, min = 0, sec = 0;

    // get system time
    localtime_r(&t, &tm_now);

    // the date
    year = tm_now.tm_year + 1900;
    month = tm_now.tm_mon + 1;
    mday = tm_now.tm_mday;
    hour = tm_now.tm_hour;
    min = tm_now.tm_min;
    static int secTemp = -1;
    if (secTemp != tm_now.tm_sec) {
        secTemp = tm_now.tm_sec;
        sec = tm_now.tm_sec;
    } else {
        // the second is not update, just return.
        return -1;
    }

    switch (runChannelCfg.channelInfo[ch].osdDatetime.dateFormat) {
        case 0:
            snprintf(date_buf, sizeof(date_buf), "%04d-%02d-%02d", year, month, mday);
            break;
        case 1:
            snprintf(date_buf, sizeof(date_buf), "%02d-%02d-%04d", month, mday,year );
            break;
        case 2:
            snprintf(date_buf, sizeof(date_buf), "%04d/%02d/%02d", year, month, mday);
            break;
        case 3:
            snprintf(date_buf, sizeof(date_buf), "%02d/%02d/%04d", month, mday, year);
            break;
        case 4:
            snprintf(date_buf, sizeof(date_buf), "%02d-%02d-%04d", mday, month, year);
            break;
        case 5:
        default:
            snprintf(date_buf, sizeof(date_buf), "%02d/%02d/%04d", mday, month,year);
            break;
    }
#if 0
    if (runSystemCfg.deviceInfo.languageType == 0) {//ch
        if (hour < 6) {
            meridiem = "凌晨";
        } else if (hour < 12) {
            meridiem = "上午";
        } else if (hour < 18) {
            meridiem = "下午";
        } else if (hour < 20) {
            meridiem = "傍晚";
        } else {
            meridiem = "晚上";
        }
    } else {//en
        if (hour < 12) {
            meridiem = "AM";
        } else {
            meridiem = "PM";
        }
    }
    #else
    if (hour < 12) {
            meridiem = "AM";
        } else {
            meridiem = "PM";
        }
    #endif

    switch (runChannelCfg.channelInfo[ch].osdDatetime.timeFmt) {
        default:
        case 0:
            snprintf(time_buf, sizeof(time_buf), "%02d:%02d:%02d", hour, min, sec);
            break;
        case 1:
            #if 0
            if(runSystemCfg.deviceInfo.languageType == 0) {//ch
                snprintf(time_buf, sizeof(time_buf), "%s %d:%02d:%02d", meridiem, hour < 12 ? hour : hour - 12, min, sec);
            } else {//en
                snprintf(time_buf, sizeof(time_buf), "%02d:%02d:%02d %s", hour < 12 ? hour : hour - 12, min, sec, meridiem);
            }
            #else
            snprintf(time_buf, sizeof(time_buf), "%s %02d:%02d:%02d", meridiem, hour < 12 ? hour : hour - 12, min, sec);
            #endif
            break;
    }

    if (runChannelCfg.channelInfo[ch].osdDatetime.displayWeek) {
        if (runSystemCfg.deviceInfo.languageType == 0) {//ch
            weekday = weekday_map_chs[tm_now.tm_wday];
        } else {//en
            weekday = weekday_map_eng[tm_now.tm_wday];
        }
    } else {
        weekday = "";
    }

    // update local time.
    snprintf(device_local_time, sizeof(device_local_time), "%s %s", date_buf, time_buf);
    device_local_week = tm_now.tm_wday;

    sprintf(clock_str, "%s %s %s", date_buf, weekday, time_buf);
    return 0;
}

void netcam_osd_update_clock()
{
    int i = 0;
	int font_size = 32;
    char clock_str[256] = {0};
    time_t t = time(NULL);
    static int clock_count = 0;
    //printf("---osd update clock time from 1970:%ld\n",t);

    
    if (!osd_running)
    {
        LOG_ERR("osd is not running\n");
        return;
    }

    if (_osd_clock_format(0, t, clock_str) < 0)
        return ;
    #if 0
    clock_count++;
    if(clock_count > 30)//every 20s save time 
    {
        clock_count = 0;
        runSystemCfg.maintainCfg.newest_time = t;
        SystemCfgSave();
        PRINT_INFO("xxx save utc time : %ld\n",t);
    }
    #endif
    osd_enter_lock();
	int stream_cnt = netcam_video_get_channel_number();
#ifdef MODULE_SUPPORT_MOJING
    stream_cnt = 2;
#endif
    for (i = 0; i < stream_cnt; ++i) {
        if (runChannelCfg.channelInfo[i].osdDatetime.enable) {
            // update osd display
            
#ifdef MODULE_SUPPORT_VECTOR_FONT
            if(runVideoCfg.vencStream[i].h264Conf.height <= 576) 
            {
                font_size = 16;
            }
            else if(runVideoCfg.vencStream[i].h264Conf.height <= 720) 
            {
                font_size = 32;
            }
            else if(runVideoCfg.vencStream[i].h264Conf.height <= 1080)
                font_size = 48;
            else if(runVideoCfg.vencStream[i].h264Conf.height <= 1296)
                font_size = 64;
            else
            {
                font_size = 64;
            }
#else
            if(runVideoCfg.vencStream[i].h264Conf.height <= 576) 
            {
                font_size = 16;
            }
            else if(runVideoCfg.vencStream[i].h264Conf.height <= 1080)
            {
                font_size = 32;
            }
            else
            {
                font_size = 32;
            }
#endif

			
#ifdef MODULE_SUPPORT_MOJING
			int str_width = 0;
			extern int mojing_get_osd_compositions_area_id(void);
			if(NETCAM_OSD_ID_AREA == mojing_get_osd_compositions_area_id())
			{
				runChannelCfg.channelInfo[i].osdDatetime.y = (float)font_size / runVideoCfg.vencStream[i].h264Conf.height;
				str_width = ((strlen(clock_str)+1)/2 + 1) * font_size;
				runChannelCfg.channelInfo[i].osdDatetime.x = 1.0000 - (float)str_width / runVideoCfg.vencStream[i].h264Conf.width;
			}
#endif
            //sdk_osd_set_channel_attr(i, NETCAM_OSD_CLOCK_AREA, font_size, 50);
            osd_set_display(i, NETCAM_OSD_CLOCK_AREA,
                runChannelCfg.channelInfo[i].osdDatetime.x,
                runChannelCfg.channelInfo[i].osdDatetime.y,
                clock_str, font_size);
            osd_set_enable(i, NETCAM_OSD_CLOCK_AREA, 1);
        } else {
            osd_set_enable(i, NETCAM_OSD_CLOCK_AREA, 0);
        }
    }
    osd_leave_lock();
}

void netcam_osd_update_title()
{
    int i = 0;
	int font_size = 32;
    if (!osd_running)
    {
        LOG_ERR("osd is not running\n");
        return;
    }

    osd_enter_lock();

	int stream_cnt = netcam_video_get_channel_number();
#ifdef MODULE_SUPPORT_MOJING
    stream_cnt = 2;
#endif
    for (i = 0; i < stream_cnt; ++i) {
        if (runChannelCfg.channelInfo[i].osdChannelName.enable) {
            // update osd display
#ifdef MODULE_SUPPORT_VECTOR_FONT
            if(runVideoCfg.vencStream[i].h264Conf.height <= 576) 
            {
                font_size = 16;
            }
            else if(runVideoCfg.vencStream[i].h264Conf.height <= 720) 
            {
                font_size = 32;
            }
            else if(runVideoCfg.vencStream[i].h264Conf.height <= 1080)
                font_size = 48;
            else if(runVideoCfg.vencStream[i].h264Conf.height <= 1296)
                font_size = 64;
            else
            {
                font_size = 64;
            }
#else
            if(runVideoCfg.vencStream[i].h264Conf.height <= 576) 
            {
                font_size = 16;
            }
            else if(runVideoCfg.vencStream[i].h264Conf.height <= 1080)
            {
                font_size = 32;
            }
            else
            {
                font_size = 32;
            }
#endif

#ifdef MODULE_SUPPORT_MOJING
			extern int mojing_get_osd_compositions_area_id(void);
			if(NETCAM_OSD_ID_AREA == mojing_get_osd_compositions_area_id())
			{
				runChannelCfg.channelInfo[i].osdChannelName.x = (float)font_size / runVideoCfg.vencStream[i].h264Conf.width;
				runChannelCfg.channelInfo[i].osdChannelName.y = 1.0000 - (float)font_size*2 / runVideoCfg.vencStream[i].h264Conf.height;
			}
#endif

            osd_set_display(i, NETCAM_OSD_TITLE_AREA,
                    runChannelCfg.channelInfo[i].osdChannelName.x,
                    runChannelCfg.channelInfo[i].osdChannelName.y,
                    runChannelCfg.channelInfo[i].osdChannelName.text, font_size);
            osd_set_enable(i, NETCAM_OSD_TITLE_AREA, 1);
        } else {
            osd_set_enable(i, NETCAM_OSD_TITLE_AREA, 0);
        }
    }
    osd_leave_lock();
}
void netcam_osd_update_id(void)
{
    int i = 0;
	int font_size = 32;
    if (!osd_running)
    {
        LOG_ERR("osd is not running\n");
        return;
    }
#ifdef MODULE_SUPPORT_MOJING
	extern int mojing_get_osd_compositions_area_id(void);
	if(NETCAM_OSD_ID_AREA == mojing_get_osd_compositions_area_id())
		return;
#endif


    osd_enter_lock();
    char rec_status[128] = {0};
    int sd_status = mmc_get_sdcard_stauts();
    LOG_INFO("sd_status: %d\n", sd_status);
	if (runSystemCfg.deviceInfo.languageType == 0) {//ch
	    switch(sd_status)
	    {
	        case SD_STATUS_NOTFORMAT:
				if(runRecordCfg.sdCardFormatFlag==0) 
	            	sprintf(rec_status, "%s %s", "[SD卡异常，请格式化]", runChannelCfg.channelInfo[i].osdChannelID.text);
				else	
	            	sprintf(rec_status, "%s %s", "[SD卡未格式化]", runChannelCfg.channelInfo[i].osdChannelID.text);
	            break;
	        case SD_STATUS_READONLY:
				if(runRecordCfg.sdCardFormatFlag==0) 
	            	sprintf(rec_status, "%s %s", "[SD卡异常，请格式化]", runChannelCfg.channelInfo[i].osdChannelID.text);
				else	
	            	sprintf(rec_status, "%s %s", "[SD卡只读]", runChannelCfg.channelInfo[i].osdChannelID.text);
	            break;
	        case SD_STATUS_OK:
	            if(is_thread_record_running())
	                sprintf(rec_status, "%s %s", "[录像中]", runChannelCfg.channelInfo[i].osdChannelID.text);
	            else
	                sprintf(rec_status, "%s", runChannelCfg.channelInfo[i].osdChannelID.text);
	            break;
	        case SD_STATUS_FORMATING:
	            sprintf(rec_status, "%s %s", "[SD卡正在格式化]", runChannelCfg.channelInfo[i].osdChannelID.text);
	            break;
	        default:
	            sprintf(rec_status, "%s", runChannelCfg.channelInfo[i].osdChannelID.text);
	            break;
	    }


	}
	else{
	    switch(sd_status)
	    {
	        case SD_STATUS_NOTFORMAT:
				if(runRecordCfg.sdCardFormatFlag==0) 
	            	sprintf(rec_status, "%s %s", "[SD Card error,please format]", runChannelCfg.channelInfo[i].osdChannelID.text);
				else	
	            	sprintf(rec_status, "%s %s", "[SD NOTFORMAT]", runChannelCfg.channelInfo[i].osdChannelID.text);
	            break;
	        case SD_STATUS_READONLY:
				if(runRecordCfg.sdCardFormatFlag==0) 
	           		sprintf(rec_status, "%s %s", "[SD Card error,please format]", runChannelCfg.channelInfo[i].osdChannelID.text);
				else	
	            	sprintf(rec_status, "%s %s", "[SD READONLY]", runChannelCfg.channelInfo[i].osdChannelID.text);
	            break;
	        case SD_STATUS_OK:
	            if(is_thread_record_running())
	                sprintf(rec_status, "%s %s", "[REC]", runChannelCfg.channelInfo[i].osdChannelID.text);
	            else
	                sprintf(rec_status, "%s", runChannelCfg.channelInfo[i].osdChannelID.text);
	            break;
	        case SD_STATUS_FORMATING:
	            sprintf(rec_status, "%s %s", "[SD formating]", runChannelCfg.channelInfo[i].osdChannelID.text);
	            break;
	        default:
	            sprintf(rec_status, "%s", runChannelCfg.channelInfo[i].osdChannelID.text);
	            break;
	    }
	}
	int stream_cnt = netcam_video_get_channel_number();
#ifdef MODULE_SUPPORT_MOJING
    stream_cnt = 2;
#endif
    for (i = 0; i < stream_cnt; ++i) {
        if (runChannelCfg.channelInfo[i].osdChannelID.enable) {
            // update osd display
#ifdef MODULE_SUPPORT_VECTOR_FONT
            if(runVideoCfg.vencStream[i].h264Conf.height <= 576) 
            {
                font_size = 16;
            }
            else if(runVideoCfg.vencStream[i].h264Conf.height <= 720) 
            {
                font_size = 32;
            }
            else if(runVideoCfg.vencStream[i].h264Conf.height <= 1080)
                font_size = 48;
            else if(runVideoCfg.vencStream[i].h264Conf.height <= 1296)
                font_size = 64;
            else
            {
                font_size = 64;
            }
#else
            if(runVideoCfg.vencStream[i].h264Conf.height <= 576) 
            {
                font_size = 16;
            }
            else if(runVideoCfg.vencStream[i].h264Conf.height <= 1080)
            {
                font_size = 32;
            }
            else
            {
                font_size = 32;
            }
#endif
            osd_set_display(i, NETCAM_OSD_ID_AREA,
                    runChannelCfg.channelInfo[i].osdChannelID.x,
                    runChannelCfg.channelInfo[i].osdChannelID.y,
                    rec_status, font_size);
            osd_set_enable(i, NETCAM_OSD_ID_AREA, 1);
        } else {
            osd_set_enable(i, NETCAM_OSD_ID_AREA, 0);
        }
    }
    osd_leave_lock();
}

int netcam_osd_text_copy(char *dsttext, char *text, int dstlen)
{
    if(!text || !dsttext)
    {
		return -1;
	}
    memset(dsttext, 0, dstlen);
    strncpy(dsttext, text, dstlen-1);
    return dstlen - 1;
}

void netcam_osd_set_title(int ch, char *title)
{
    if (!osd_running)
    {
        LOG_ERR("osd is not running\n");
        return;
    }
    osd_enter_lock();
    //strcpy(runChannelCfg.channelInfo[ch].osdChannelName.text, title);
	netcam_osd_text_copy(runChannelCfg.channelInfo[ch].osdChannelName.text, title, sizeof(runChannelCfg.channelInfo[ch].osdChannelName.text));
    osd_leave_lock();
}

void netcam_osd_set_id(int ch, char *id)
{
    if (!osd_running)
    {
        LOG_ERR("osd is not running\n");
        return;
    }
    osd_enter_lock();
    //strcpy(runChannelCfg.channelInfo[ch].osdChannelID.text, id);	
	netcam_osd_text_copy(runChannelCfg.channelInfo[ch].osdChannelID.text, id, sizeof(runChannelCfg.channelInfo[ch].osdChannelID.text));
    osd_leave_lock();
}

void netcam_osd_pm_save(void)
{
    if (!osd_running)
    {
        LOG_ERR("osd is not running\n");
        return;
    }
    osd_enter_lock();
	LOG_INFO("netcam_osd_pm_save!!\n");
	ChannelCfgSave();
	osd_leave_lock();
}

char *netcam_osd_cfg_json_string(int channelId)
{
    //char *buf=NULL;
	if(channelId < 0 || channelId > 3)
	{
		LOG_ERR("id error\n");
		return NULL;
	}
    if (!osd_running)
    {
        LOG_ERR("osd is not running\n");
        return NULL;
    }
    osd_enter_lock();
    char *buf = ChannelCfgLoadOverlayJson(channelId);
    osd_leave_lock();
    return buf;
}

char *netcam_pm_get_cfg_json_string(int channelId)
{
    //char *buf=NULL;
	if(channelId < 0 || channelId > 3)
	{
        LOG_ERR("id error\n");
		return NULL;
	}
    if (!osd_running)
    {
        LOG_ERR("osd is not running\n");
        return NULL;
    }
    osd_enter_lock();
    char *buf = ChannelCfgLoadCoverJson(channelId);
    osd_leave_lock();
    return buf;
}

int netcam_osd_get_info(int channel,GK_NET_CHANNEL_INFO *channelInfo)
{
	if(channel < 0 || channel > 3)
	{
        LOG_ERR("id error\n");
		return -1;
	}
    if (!osd_running)
    {
        LOG_ERR("osd is not running\n");
        return -1;
    }
    osd_enter_lock();
    memcpy(channelInfo, &runChannelCfg.channelInfo[channel],sizeof(GK_NET_CHANNEL_INFO));
    osd_leave_lock();
    return 0;
}

int netcam_osd_set_info(int channel, GK_NET_CHANNEL_INFO *channelInfo)
{
	if(channel < 0 || channel > 3)
	{
        LOG_ERR("id error\n");
		return -1;
	}
    if (!osd_running)
    {
        LOG_ERR("osd is not running\n");
        return -1;
    }
    osd_enter_lock();
    memcpy(&runChannelCfg.channelInfo[channel], channelInfo, sizeof(GK_NET_CHANNEL_INFO));
    osd_leave_lock();
    return 0;
}


int netcam_osd_update_id_rect(float x,float y, float w, float h,int enable, char *text)
{
	int i = 0;
	int font_size = 32;
	int wf;
	int hf;

    if (!osd_running)
    {
        LOG_ERR("osd is not running\n");
        return -1;
    }
	
#ifdef MODULE_SUPPORT_MOJING
	extern int mojing_get_osd_compositions_area_id(void);
	if(NETCAM_OSD_ID_AREA == mojing_get_osd_compositions_area_id())
		return -1;
#endif

	osd_enter_lock();
	
	int stream_cnt = netcam_video_get_channel_number();
#ifdef MODULE_SUPPORT_MOJING
    stream_cnt = 2;
#endif
	for (i = 0; i < stream_cnt; ++i) {
		if (enable && i == 1) {
			// update osd display
#ifdef MODULE_SUPPORT_VECTOR_FONT
            if(runVideoCfg.vencStream[i].h264Conf.height <= 576) 
            {
                font_size = 16;
            }
            else if(runVideoCfg.vencStream[i].h264Conf.height <= 720) 
			{
				font_size = 32;
			}
			else if(runVideoCfg.vencStream[i].h264Conf.height <= 1080)
				font_size = 48;
            else if(runVideoCfg.vencStream[i].h264Conf.height <= 1296)
                font_size = 64;
            else
            {
                font_size = 64;
            }
#else
            if(runVideoCfg.vencStream[i].h264Conf.height <= 576) 
            {
                font_size = 16;
            }
			else if(runVideoCfg.vencStream[i].h264Conf.height <= 1080)
            {
                font_size = 32;
            }
            else
            {
                font_size = 32;
            }
#endif
			wf = (int)(w*runVideoCfg.vencStream[i].h264Conf.width);
			hf = (int)(h*runVideoCfg.vencStream[i].h264Conf.height);
			printf("draw channel %d:x y w h %d-%d-%d-%d\n",i,
				(int)(runVideoCfg.vencStream[i].h264Conf.width*x),
				(int)(runVideoCfg.vencStream[i].h264Conf.height*y),
				wf,hf);	
			osd_set_display_rect(i, NETCAM_OSD_ID_AREA,
					x,
					y,wf,hf,
					text, font_size);
			osd_set_enable(i, NETCAM_OSD_ID_AREA, 1);
		} else {
			osd_set_enable(i, NETCAM_OSD_ID_AREA, 0);
		}
	}
	osd_leave_lock();
    return 0; 
}

int netcam_osd_update_id_text(float x,float y, int font_size, int enable, char *text)
{
    int i = 0, stream_cnt;

    if (!osd_running)
    {
        LOG_ERR("osd is not running\n");
        return -1;
    }
    osd_enter_lock();
    stream_cnt = netcam_video_get_channel_number();

    for (i = 0; i < stream_cnt; ++i)
    {
#ifdef MODULE_SUPPORT_VECTOR_FONT
        if(runVideoCfg.vencStream[i].h264Conf.height <= 576) 
            font_size = 32;
        else if(runVideoCfg.vencStream[i].h264Conf.height <= 720)
            font_size = 48;
        else if(runVideoCfg.vencStream[i].h264Conf.height <= 1296)
            font_size = 64;
        else
            font_size = 72;
#else
        font_size = 32;
#endif
        osd_set_display(i, NETCAM_OSD_ID_AREA, x, y, text, font_size);
        osd_set_enable(i, NETCAM_OSD_ID_AREA, enable);
    }
    osd_leave_lock();
    return 0; 
}

void netcam_pm_init( void )
{
	int i;
	ST_GK_VIN_COVER_ATTR cover;

	//sdk1.1 need init vin here
	sdk_vin_init();
    for(i=0; i < 4;i++)
	{
		cover.x = (float)runChannelCfg.shelterRect[i].x ;//runVideoCfg.vencStream[i].streamFormat.width;
		cover.y = (float)runChannelCfg.shelterRect[i].y ;//runVideoCfg.vencStream[i].streamFormat.height;
		cover.width = (float)runChannelCfg.shelterRect[i].width ;//runVideoCfg.vencStream[i].streamFormat.width;
		cover.height = (float)runChannelCfg.shelterRect[i].height ;// runVideoCfg.vencStream[i].streamFormat.height;
		cover.color = runChannelCfg.shelterRect[i].color;
		cover.enable= runChannelCfg.shelterRect[i].enable;

		sdk_vin_set_cover( 0, i, &cover );
	}
}

int netcam_pm_set_cover( int vin, int id, GK_NET_SHELTER_RECT cover )
{
    int ret = 0;
	ST_GK_VIN_COVER_ATTR gkCover;
	if(id < 0 || id > 3)
	{
        LOG_ERR("id error\n");
		return -1;
	}
    if (!osd_running)
    {
        LOG_ERR("osd is not running\n");
        return -1;
    }
    osd_enter_lock();
    //LOG_INFO("netcam_pm_set_cover: (id = %d)\n", id);
    //LOG_INFO("enable: %d\n", cover.enable);
    //LOG_INFO("   x-y: %f-%f\n", cover.x, cover.y);
    //LOG_INFO(" w-h-c: %f-%f-0x%06X\n", cover.width, cover.height, cover.color);
    memset(&gkCover,0,sizeof(gkCover));
	gkCover.color = cover.color;
	gkCover.enable = cover.enable;
	gkCover.height = cover.height;
	gkCover.width = cover.width;
	gkCover.x = cover.x;
	gkCover.y = cover.y;

    ret = sdk_vin_set_cover( vin, id, &gkCover );
    if(ret == 0)
    {
		memcpy(&runChannelCfg.shelterRect[id],&cover,sizeof(GK_NET_SHELTER_RECT));
	}
    osd_leave_lock();
    return ret;
}

int netcam_pm_get_cover( int vin, int id, GK_NET_SHELTER_RECT *cover )
{
	if(id < 0 || id > 3)
	{
        LOG_ERR("id error\n");
		return -1;
	}
    if (!osd_running)
    {
        LOG_ERR("osd is not running\n");
        return -1;
    }
    osd_enter_lock();
    memcpy(cover,&runChannelCfg.shelterRect[id],sizeof(GK_NET_SHELTER_RECT));
    osd_leave_lock();
    return 0;
}

void netcam_pm_exit_cover(void)
{
	int i=4;
    if (!osd_running)
    {
        LOG_ERR("osd is not running\n");
        return;
    }
    osd_enter_lock();
	while(i--)
		runChannelCfg.shelterRect[i].enable = 0;
    osd_leave_lock();
}
