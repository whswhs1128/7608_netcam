#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "sdk_debug.h"
#include "sdk_api.h"
#include "common.h"
#include "eventalarm.h"
#include "netcam_api.h"
#include "utility_api.h"
#include "mmc_api.h"
#include "work_queue.h"
#include "avi_rec.h"
#include "sdk_pda.h"
#include "sdk_vision.h"

#define MAX_INTERVAL_TIME  120
#define NETCAM_MD_MAX_DRAP 16

static int md_range_max = 500;
static int debug_on = 0;

static int md_sensitivity_map[] = {
	[SYS_MD_SENSITIVITY_LOWEST]  = 40,
	[SYS_MD_SENSITIVITY_LOW]     = 15,
	[SYS_MD_SENSITIVITY_MEDIUM]  = 8,
	[SYS_MD_SENSITIVITY_HIGH]    = 3,
	[SYS_MD_SENSITIVITY_HIGHEST] = 1,
};
static int md_light_start = 0;
static int md_smart_start = 0;  //智能全彩
static int md_smart_time_use = 0;
static int md_light_startType = 0; //移动侦测：0， 人形报警：1
static int md_light_startMode = 0; //触发光报警时当前夜视状态 ISP_DAY:0
static int md_mix_play_start = 0;
static int playCnt = 0;
static unsigned int last_light_md_sec = 0;
static int audioThreadInited = 0;
static int alarmPlayCnt = 0;
static char audioPlayFile[128] = {0};
static int mdDelayRecord = 0;
static int mdCheat = 0;
static int smartDelayTime = 10;

static pthread_mutex_t playStatus = PTHREAD_MUTEX_INITIALIZER;

extern void motion_rec_start(int num, int rec_time, int rec_time_once);
#define MD_PARAM_FILE "gk_md_param_cfg.cjson"
#define MAX_INTERVAL_TIME  		120
#define MD_LIGHT_AlARM_TIME     10
#define MD_FACTOR_MAX			1800
#define MD_USE_DEBUG_MODE		0

static int md_is_in_schedule_time()
{
    if (runMdCfg.enable == 0)
    {
        //PRINT_INFO("md disable.\n");
        return 0;
    }
    //modify by heyong, Deprecated schedule_mode & runMdCfg.scheduleSlice, direct check schedule timepoint.
    return is_in_schedule_timepoint(&(runMdCfg.scheduleTime[0][0]));
}
static int SnapSaveSdCard(char *filename)
{
	struct tm tt;
	char snap_path[64] = {0};
	char cmd_line[64]  = {0};

    if(mmc_get_sdcard_stauts() != SD_STATUS_OK)
    {
        return 0;
    }

	while(grd_sd_get_free_size() < 100)
	{
        if(grd_sd_del_oldest_avi_file() == 0)
            return 0;
	}

    sys_get_tm(&tt);
    sprintf(snap_path, "%s/%04d%02d%02d/snapshot", GRD_SD_MOUNT_POINT, tt.tm_year, tt.tm_mon, tt.tm_mday);

	sprintf(cmd_line, "mkdir -p %s", snap_path);
	new_system_call(cmd_line);

	sprintf(cmd_line, "cp -f %s %s", filename, snap_path);
	new_system_call(cmd_line);
	return 1;
}
static int md_is_in_schedule_time_report(int* valid_schedule)
{
    int ret;
    if (runMdCfg.enable == 0)
    {
        return 0;
    }

    return is_in_schedule_timepoint_report(&(runMdCfg.scheduleTime[0][0]), valid_schedule);
}

#ifdef MODULE_SUPPORT_GOOLINK
static void glink_alarm_beep(int index)
{
	if(index >= (sizeof(runP2PCfg.is_beep)/sizeof(runP2PCfg.is_beep[0])))
		return;

    if(runP2PCfg.is_beep[index] == 1)
    {
		//CREATE_WORK(beep, EVENT_TIMER_WORK, netcam_audio_out);	xqq

        switch(runP2PCfg.alarmAudioType)
        {
            case 1:
				INIT_WORK(beep, COMPUTE_TIME(0,0,0,1,0), AUDIO_FILE_MD_ALARM);//AUDIO_FILE_ALERT_DEFAULT_1);
                break;

            case 2:
				INIT_WORK(beep, COMPUTE_TIME(0,0,0,1,0), AUDIO_FILE_MD_ALARM);//AUDIO_FILE_ALERT_DEFAULT_2);
                break;

            case 10:
            default:
				INIT_WORK(beep, COMPUTE_TIME(0,0,0,1,0), AUDIO_FILE_MD_ALARM);
                break;
        }

        SCHEDULE_DEFAULT_WORK(beep);
        PRINT_INFO("glink_alarm_beep.\n");
    }
}
#endif


static int num_md_light_control = 0;

void md_light_close_delay(void)
{
    sdk_isp_set_md_light_mode(0);
}

static void md_light_control(void)
{
    int value = 1;
    struct timeval t1 = {0, 0};

    gettimeofday(&t1, NULL);
    num_md_light_control++;
    last_light_md_sec = t1.tv_sec;
    if(num_md_light_control > runAlarmCfg.alarmInfo[md_light_startType].lightAlarmTime || md_light_start == 0)
    {
        num_md_light_control = 0;
        md_light_start = 0;
        sdk_isp_light_led_set(0);
        netcam_timer_del_task(md_light_control);
        printf("md_light_control delay close\n");
        netcam_timer_add_task(md_light_close_delay, NETCAM_TIMER_ONE_SEC * 5, SDK_FALSE, SDK_TRUE);
        return;
    }

    if (runAlarmCfg.alarmInfo[md_light_startType].lightAlarmType == 2)
    {
        //闪烁频率，越低，下面控制灯间隔时间越长
        if (num_md_light_control % runAlarmCfg.alarmInfo[md_light_startType].lightAlarmFrequency != 0)
        {
            return;
        }
    }

    //sdk_gpio_get_output_value(sdk_cfg.gpio_light_led, &value);
    printf("md_light_control time:%d, status:%d, mode:%d, %d\n", num_md_light_control, sdk_isp_light_led_get_status(), md_light_startMode, runAlarmCfg.alarmInfo[md_light_startType].lightAlarmType);
    
    if (sdk_isp_light_led_get_status() == 0)
    {
        //sdk_gpio_set_output_value(sdk_cfg.gpio_light_led, sdk_cfg.gpio_light_led_on_value==1?1:0);
        sdk_isp_light_led_set(1);
    }
    else if (runAlarmCfg.alarmInfo[md_light_startType].lightAlarmType == 2 && md_light_startMode == 0)
    {
        //只在白天才闪烁
        printf("--->sdk_isp_light_led_set 0\n");
        sdk_isp_light_led_set(0);
    }
}


//智能全彩
static void md_light_control_smart(void)
{
    md_smart_time_use++;
    //printf("md_light_control %d\n", num);
    if(md_smart_time_use > smartDelayTime || md_smart_start == 0)
    {
        md_smart_time_use = 0;
        md_smart_start = 0;
        if (runImageCfg.lightMode == ISP_BULB_MODE_MIX || runImageCfg.lightMode == ISP_BULB_MODE_IR)
            sdk_isp_light_led_set(0);
        netcam_timer_del_task(md_light_control_smart);        
        netcam_timer_del_task(md_light_control);
        md_light_start = 0;
        num_md_light_control = 0;
        sdk_isp_set_bulb_mode(runImageCfg.lightMode);
        sdk_isp_set_md_light_mode(0);
        return;
    }

    //sdk_gpio_get_output_value(sdk_cfg.gpio_light_led, &value);
    
    if (sdk_isp_light_led_get_status() == 0)
    {
        //sdk_gpio_set_output_value(sdk_cfg.gpio_light_led, sdk_cfg.gpio_light_led_on_value==1?1:0);
        sdk_isp_light_led_set(1);
    }
}

static void md_bulb_control(void)
{
    //if ()
    {
        sdk_isp_set_bulb_mode(ISP_BULB_MODE_MIX);
        md_mix_play_start = 0;
    }
}

void md_light_alarm(void)
{
    int schedule_index = 0;

    #if 0
    if (ISP_BULB_MODE_MIX == runImageCfg.lightMode)
    {
        sdk_isp_set_bulb_mode(ISP_BULB_MODE_ALL_COLOR);
        netcam_timer_add_task(md_bulb_control, NETCAM_TIMER_ONE_SEC*runMdCfg.handle.colorModeTime, SDK_FALSE, SDK_TRUE);
    }
    #endif
    if(sdk_cfg.gpio_light_led > 0)
    {
        if(runMdCfg.handle.is_light[schedule_index])
        {
            if (md_light_start == 0)
            {
                //int value = 1;
                //sdk_gpio_get_output_value(sdk_cfg.gpio_light_led, &value);
                //printf("value ,sdk_cfg.gpio_light_led_on_value = %d, ,,,,,,,%d\n", value, sdk_cfg.gpio_light_led_on_value);
                if (sdk_isp_light_led_get_status() == 0)//白光灯没有打开的情况下sdk_isp_ircut_led_get_status() && 
                {
                    printf("md_light_alarm start light.\n");
                    md_light_start = 1;
                    sdk_isp_set_md_light_mode(1);
                    netcam_timer_add_task(md_light_control, NETCAM_TIMER_ONE_SEC, SDK_TRUE, SDK_FALSE);
                }
            }
        }
    }   
}



#ifdef MODULE_SUPPORT_MOJING_V4

void md_light_alarm_mojing(int type)
{
    int schedule_index = 0;
    struct tm *ptm;
    long ts;
    struct tm tt = {0};

    #if 0
    if (ISP_BULB_MODE_MIX == runImageCfg.lightMode)
    {
        sdk_isp_set_bulb_mode(ISP_BULB_MODE_ALL_COLOR);
        netcam_timer_add_task(md_bulb_control, NETCAM_TIMER_ONE_SEC*runMdCfg.handle.colorModeTime, SDK_FALSE, SDK_TRUE);
    }
    #endif
    
    if(runAlarmCfg.alarmInfo[type].lightAlarmStatus == 1)
    {
        if (md_light_start == 0)
        {
            md_light_startMode = sdk_isp_get_cur_mode();
            printf("md_light_alarm_mojing mode:%d\n", md_light_startMode);
            //int value = 1;
            //sdk_gpio_get_output_value(sdk_cfg.gpio_light_led, &value);
            //printf("value ,sdk_cfg.gpio_light_led_on_value = %d, ,,,,,,,%d\n", value, sdk_cfg.gpio_light_led_on_value);
            if (md_smart_start == 1)
            {
                printf("md_light_alarm_mojing md_smart_start:%d, return\n", md_smart_start);
                return;
            }

            //全彩、智能全彩，夜间模式下不亮灯
            if (sdk_isp_ircut_led_get_status() && (runImageCfg.lightMode == 1 || runImageCfg.lightMode == 2))
            {
                printf("md_light_alarm_mojing light mode:%d, status:%d\n", runImageCfg.lightMode, sdk_isp_ircut_led_get_status());
                return;
            }

            if (runImageCfg.lightMode == 2)
            {
                ts = time(NULL);
                ptm = localtime_r(&ts, &tt);
                printf("md_light_alarm_mojing cur h:%d\n", ptm->tm_hour);
                if (ptm->tm_hour >= 20 || ptm->tm_hour < 8)
                {
                    return;
                }
            }
            
            if (sdk_isp_light_led_get_status() == 0)//sdk_isp_ircut_led_get_status() && 白光灯没有打开的情况下
            {
                printf("md_light_alarm start light.\n");
                md_light_start = 1;
                sdk_isp_set_md_light_mode(1);
                netcam_timer_add_task(md_light_control, NETCAM_TIMER_ONE_SEC, SDK_TRUE, SDK_FALSE);
            }
            md_light_startType = type;
        }
    }
}

void md_light_alarm_smart(void)
{
    if (md_smart_start == 0)
    {
        //int value = 1;
        //sdk_gpio_get_output_value(sdk_cfg.gpio_light_led, &value);
        //printf("value ,sdk_cfg.gpio_light_led_on_value = %d, ,,,,,,,%d\n", value, sdk_cfg.gpio_light_led_on_value);
        if (sdk_isp_ircut_led_get_status() && sdk_isp_light_led_get_status() == 0)//白光灯没有打开的情况下 
        {
            printf("md_light_alarm_smart start light.\n");
            md_smart_start = 1;
            sdk_isp_set_md_light_mode(1);
            sdk_isp_set_bulb_mode(ISP_BULB_MODE_ALL_COLOR);
            netcam_timer_add_task(md_light_control_smart, NETCAM_TIMER_ONE_SEC, SDK_TRUE, SDK_FALSE);
        }
    }
    else
    {
        md_smart_time_use = 0;
    }
    #if 0
    else
    {
        md_smart_time_use = 0;
    }
    #endif
}


void *netcam_md_audio_alarm_thread(void *pData)
{
    while(1)
    {
        if (alarmPlayCnt > 0)
        {
            while(alarmPlayCnt > 0)
            {
                netcam_audio_out_mojing(audioPlayFile);
                alarmPlayCnt--;
                sleep(2);
            }
        }
        usleep(100000);
    }
}

int netcam_md_alarm_play(char *playFile, int playCnt)
{
    printf("netcam_md_alarm_play:%s, alarmPlayCnt:%d, playCnt:%d\n", playFile, alarmPlayCnt, playCnt);
    if (playFile != NULL && alarmPlayCnt <= 0)
    {
        alarmPlayCnt = playCnt;
        if (strcmp(audioPlayFile, playFile) != 0)
        {
            strcpy(audioPlayFile, playFile);
        }
    }

    if (playCnt == 0)
    {
        alarmPlayCnt = playCnt;
    }
}

void netcam_md_alarm_play_thread_start(void)
{
    pthread_t audioThread;
    if (audioThreadInited == 0)
    {
        pthread_create(&audioThread, NULL, netcam_md_audio_alarm_thread, NULL);
        audioThreadInited = 1;
    }
}

static void netcam_md_delay_record(void)
{
    motion_rec_start(runMdCfg.handle.recStreamNo, runMdCfg.handle.recTime, runRecordCfg.recordLen);
    mdDelayRecord = 0;
}

#endif


void netcam_md_handle(int vin, int range)
{
    int schedule_index = 0;
	int sensitive = 0;
    int forceMd = 0;
    
    sensitive = md_sensitivity_map[netcam_md_get_sensitivity(0, NULL)];    
    //printf("sensitive =++++++ %d, range ++++++= %d\n", sensitive, range);
    if(vin < 0 || vin > SDK_MAX_VIN_NUM || range < 0 || range > 100)
    {
        return;
    }

    #if 1
    if (mdCheat == 1 && access("/tmp/md", F_OK) == 0)
    {
        forceMd = 1;
        new_system_call("rm -f /tmp/md");
    }
    #endif
    
    if(sensitive > range && forceMd == 0)
    {
        return;
    }

#ifndef MODULE_SUPPORT_GOOLINK
    if(netcam_ptz_is_moving() || !sdk_isp_change_timeout() || (0 == md_is_in_schedule_time()))
    {
        return;// 正在移动或者不在在时间排程内，则返回
    }
#else
	if(netcam_ptz_is_moving() || !sdk_isp_change_timeout() || (0 == md_is_in_schedule_time_report(&schedule_index)))
	{
        #ifdef MODULE_SUPPORT_MOJING_V4
        if (runMdCfg.mojingMdCfg.defenceStatus == 2 && mojing_time_in_shedule(&(runMdCfg.mojingMdCfg.defenceWeek[0]), 256) == 0)
        #endif
		return;// 正在移动或者不在在时间排程内，则返回
	}
#endif

    //PRINT_INFO("catch md, in schedule time.\n");
    /* 报警间隔时间内(秒)，不重复响应同一种报警 */
    static unsigned int last_md_sec = 0;
    struct timeval t1 = {0, 0};
    int interval = runMdCfg.handle.intervalTime;
    if (interval <= 0 || interval > MAX_INTERVAL_TIME)
    {
		PRINT_ERR("interval %d set to 10 seconds\n", interval);
		interval = 10;
    }
    //PRINT_INFO("interval %d\n", interval);
    gettimeofday(&t1, NULL);
	if(t1.tv_sec < last_md_sec)
    {
		last_md_sec = t1.tv_sec;
		PRINT_INFO("time is change!\n");
	}
	if((t1.tv_sec - last_md_sec) <= interval)
    {
		//PRINT_ERR("t1.tv_sec-last_md_sec:%d,time is too closed, so not to notify.\n",(int)t1.tv_sec - last_md_sec);
        return;
    }


    /* 报警联动通知 */
    //PRINT_INFO("notify md alarm to all client.\n");
    event_alarm_touch(0, GK_ALARM_TYPE_VMOTION,1,NULL);


    /* 蜂鸣报警 */
    #ifndef MODULE_SUPPORT_GOOLINK
    if ((t1.tv_sec - last_light_md_sec) > 4)
    {
        md_light_alarm();
        if(runMdCfg.handle.is_beep)
        {
            //CREATE_WORK(beep, EVENT_TIMER_WORK, netcam_audio_out);	xqq
	    CREATE_WORK(beep, EVENT_TIMER_WORK, md_light_alarm);	//xqq
            INIT_WORK(beep, COMPUTE_TIME(0,0,0,1,0), AUDIO_FILE_MD_ALARM);
            SCHEDULE_DEFAULT_WORK(beep);
            PRINT_INFO("md action beep.\n");
        }
    }
    #else

    //printf("-->md time:%ld--%ld, %ld\n", t1.tv_sec, last_light_md_sec, last_md_sec);
    if ((t1.tv_sec - last_light_md_sec) > 4)
    {
        md_light_alarm();
        glink_alarm_beep(schedule_index);
    }

    #endif
    
#ifdef MODULE_SUPPORT_MOJING_V4
    if ((t1.tv_sec - last_light_md_sec) > 4)
    {
        md_light_alarm_mojing(0);
        if (runAlarmCfg.alarmInfo[0].audioAlarmStatus == 1)
        {
            netcam_md_alarm_play_thread_start();
            netcam_md_alarm_play(runAlarmCfg.alarmInfo[0].audioFilesCur, runAlarmCfg.alarmInfo[0].audioPlayCnt);
        }
    }
#endif  
    if ((t1.tv_sec - last_light_md_sec) > 4)
    {
        last_light_md_sec = t1.tv_sec;
    }

    last_md_sec = t1.tv_sec;
    /* PTZ巡航 */
    if(runMdCfg.handle.is_ptz)
    {
        GK_NET_CRUISE_GROUP cruise_info;
        cruise_info.byPointNum    = 1;
        cruise_info.byCruiseIndex = 0;
        cruise_info.struCruisePoint[0].byPointIndex = 0;
        cruise_info.struCruisePoint[0].byPresetNo   = runMdCfg.handle.ptzLink.value;
        cruise_info.struCruisePoint[0].byRemainTime = 0;
        cruise_info.struCruisePoint[0].bySpeed      = -1;
        netcam_ptz_preset_cruise(&cruise_info);
        PRINT_INFO("md action ptz.\n");
    }
    /* 报警联动抓图 */
    if(runMdCfg.handle.is_snap)
    {
        int snap_index = 0;
        int snap_num   = runMdCfg.handle.snapNum;
        PRINT_INFO("SNAP TRIGGER snap_num:%d\n!", snap_num);
        for( snap_index = 0;  snap_index < snap_num; snap_index ++ )
        {
            struct tm tt;
            sys_get_tm(&tt);
            char md_snap_path[64] = {0};
            sprintf(md_snap_path, "/tmp/md_%04d%02d%02d_%02d%02d%02d_%d.jpg", tt.tm_year, tt.tm_mon, tt.tm_mday, tt.tm_hour, tt.tm_min, tt.tm_sec, snap_index);

            int ret = netcam_video_snapshot(runVideoCfg.vencStream[0].h264Conf.width, runVideoCfg.vencStream[0].h264Conf.height, md_snap_path, GK_ENC_SNAPSHOT_QUALITY_MEDIUM);
            if(ret != 0)
            {
                PRINT_ERR("netcam_video_snapshot failed.\n");
                break;
            }
            //todo send to cms
            if(runMdCfg.handle.isSnapUploadToCms)
            {
                PRINT_INFO("Snap jpg, then send to cms  <-- not to do.\n");
            }
            if(runMdCfg.handle.isSnapUploadToWeb)
            {
                PRINT_INFO("Snap jpg, then send to web  <-- not to do.\n");
            }
            if(runMdCfg.handle.isSnapSaveToSd)
            {
				SnapSaveSdCard(md_snap_path);
                PRINT_INFO("Snap jpg, save to sd.\n");
            }

            #ifdef MODULE_SUPPORT_FTP
			if(runMdCfg.handle.isSnapUploadToFtp && netcam_ftp_is_ok() == 1 )
			{
                copy_file(md_snap_path, FTP_JPG_FILE);
                netcam_ftp_upload_set_status(1);
                PRINT_INFO("Snap jpg, hen send to ftp enable.\n");
			}
            #endif
            #ifdef MODULE_SUPPORT_MAIL
            if(runMdCfg.handle.is_email && netcam_mail_setting_is_ok() == 1)
            {
                PRINT_INFO("Snap jpg, hen send to email <--enable.\n");
                copy_file(md_snap_path, MAIL_JPG_FILE);
                netcam_mail_send_set_status(1);
            }
            #endif
            unlink(md_snap_path);//delete file
            PRINT_INFO("%s delete %s.\n",md_snap_path, access(md_snap_path,0) ? "success" : "failed");
        }
    }

	    /* 报警联动录像 */
    #ifdef MODULE_SUPPORT_LOCAL_REC
    if(runMdCfg.handle.is_rec)
    {
#ifdef MODULE_SUPPORT_MOJING_V4
        //优先检测人形
        if (mdDelayRecord == 0)
        {
            if (runMdCfg.mojingMdCfg.areaEventStatus == 1)
            {
                mdDelayRecord = 1;
                netcam_timer_add_task(netcam_md_delay_record, NETCAM_TIMER_TWO_SEC, SDK_FALSE, SDK_TRUE);
            }
            else
            {
                mdDelayRecord = 0;
                motion_rec_start(runMdCfg.handle.recStreamNo, runMdCfg.handle.recTime, runRecordCfg.recordLen);
            }
        }
#else
		motion_rec_start(runMdCfg.handle.recStreamNo, runMdCfg.handle.recTime, runRecordCfg.recordLen);
#endif
        //PRINT_INFO("md action reccord.\n");
    }
    #endif
    return;
}

void netcam_md_enable(int enable)
{
    GK_NET_MD_CFG md;
    get_param(MD_PARAM_ID, &md);
    md.enable = enable?1:0;
    set_param(MD_PARAM_ID, &md);
    MdCfgSave();
}

int netcam_md_set_sensitivity_percentage(int vin, int sensitive)
{
    if(vin < 0 || vin > SDK_MAX_VIN_NUM || sensitive < 0 || sensitive > 100)
        return -1;
    GK_NET_MD_CFG md;
    get_param(MD_PARAM_ID, &md);
    md.sensitive = sensitive;
    set_param(MD_PARAM_ID, &md);
    MdCfgSave();
	return 0;
}

int netcam_md_set_sensitivity(int vin, SYS_MD_SENSITIVITY_t sensi)
{
    if(vin < 0 || vin > SDK_MAX_VIN_NUM)
        return -1;
	return netcam_md_set_sensitivity_percentage(vin, 20*sensi);;
}

int netcam_md_get_sensitivity(int vin, SYS_MD_SENSITIVITY_t *sensi)
{
    if(vin < 0 || vin > SDK_MAX_VIN_NUM)
        return -1;

    GK_NET_MD_CFG md;
    get_param(MD_PARAM_ID, &md);
    int sensitivity = SYS_MD_SENSITIVITY_LOWEST;
	if (md.sensitive < 20)
	{
        sensitivity = SYS_MD_SENSITIVITY_HIGHEST;
	}
    else if (md.sensitive < 40)
	{
        sensitivity = SYS_MD_SENSITIVITY_HIGH;
	}
    else if (md.sensitive < 60)
	{
        sensitivity = SYS_MD_SENSITIVITY_MEDIUM;
	}
    else if (md.sensitive < 80)
	{
        sensitivity = SYS_MD_SENSITIVITY_LOW;
	}
    else
    {
        sensitivity = SYS_MD_SENSITIVITY_LOWEST;
    }
    
    if (sensi != NULL)
    {
        *sensi = sensitivity;
    }
	return sensitivity;
}

static int md_smd_callback()
{
    /* 报警间隔时间内(秒)，不重复响应同一种报警 */
    static unsigned int last_smd_sec = 0;
    struct timeval t1 = {0, 0};
    int interval = runMdCfg.handle.intervalTime;
    if (interval <= 0 || interval > MAX_INTERVAL_TIME)
    {
		PRINT_ERR("smd interval %d set to 10 seconds\n", interval);
		interval = 10;
    }
    gettimeofday(&t1, NULL);
	if(t1.tv_sec < last_smd_sec)
    {
		last_smd_sec = t1.tv_sec;
		PRINT_INFO("smd time is change!\n");
	}
	if((t1.tv_sec - last_smd_sec) <= interval)
    {
		//PRINT_ERR("t1.tv_sec-last_md_sec:%d,time is too closed, so not to notify.\n",(int)t1.tv_sec - last_md_sec);
        return 0;
    }
    last_smd_sec = t1.tv_sec;
    /* 报警联动通知 */
    PRINT_INFO("notify smd alarm to all client.\n");
    return event_alarm_touch(0, GK_ALARM_TYPE_AREA_ALARM,1,NULL);
}

static int md_md_callback(int vin, SDK_PDA_RECT_ARRAY_S *mdRect)
{
    netcam_md_handle(vin, mdRect->u16Num);
    return 0;
}

int netcam_md_init(int n_md)
{
    int i, ret;

#ifdef MODULE_SUPPORT_LARGE_SECOND_STREAM
    //sdk_pda_init(PIC_640x360, 2);	xqq
    sdk_pda_init();
#else
    //sdk_pda_init(PIC_640x360, 1);	xqq
    sdk_pda_init();
#endif
    //sdk_pda_set_md_cb(md_md_callback);	xqq
    LOG_INFO( "pda init ok\n");

#if MD_USE_DEBUG_MODE
	if(netcam_md_param_load() != 0)
		netcam_md_param_save();
#endif

    if (access("/opt/custom/cfg/cc", F_OK) == 0)
    {
        mdCheat = 1;
    }
    
    return 0;
}

int netcam_md_destroy()
{
	sdk_pda_exit();
    return 0;
}

int netcam_md_light_auto_start(void)
{
    if (ISP_BULB_MODE_MIX == runImageCfg.lightMode)
    {
        sdk_isp_set_bulb_mode(ISP_BULB_MODE_ALL_COLOR);
        netcam_timer_add_task(md_bulb_control, NETCAM_TIMER_TWO_SEC, SDK_TRUE, SDK_FALSE);
        md_mix_play_start = 1;
    }
    return 0;
}

int netcam_md_light_auto_stop(void)
{
    if (ISP_BULB_MODE_MIX == runImageCfg.lightMode)
    {
        if (sdk_isp_get_bulb_mode() != ISP_BULB_MODE_MIX)
        {
            netcam_timer_add_task(md_bulb_control, NETCAM_TIMER_TWO_SEC, SDK_FALSE, SDK_TRUE);
        }
    }
    return 0;
}

void netcam_md_play_start_one(void)
{
    int ircutLedStatus = 0;

#ifdef MODULE_SUPPORT_MOJING_V4
    return;
#else
    if (ISP_BULB_MODE_MIX == runImageCfg.lightMode)
    {
        pthread_mutex_lock(&playStatus);
        playCnt++;

        ircutLedStatus = sdk_isp_ircut_led_get_status();
        printf("-->ircutLedStatus:%d\n", ircutLedStatus);
        if (ircutLedStatus == 1)
        {
            sdk_isp_set_bulb_mode(ISP_BULB_MODE_ALL_COLOR);
            sdk_isp_light_led_set(1);
        }
        pthread_mutex_unlock(&playStatus);
    }
    printf("netcam_md_play_start_one:%d, cnt:%d\n", runImageCfg.lightMode, playCnt);
#endif    
}

void netcam_md_play_stop_one(void)
{    
    int lightLedStatus = 0;

#ifdef MODULE_SUPPORT_MOJING_V4
    return;
#else
    if (ISP_BULB_MODE_MIX == runImageCfg.lightMode)
    {
        pthread_mutex_lock(&playStatus);
        playCnt--;
        if (playCnt < 0)
        {
            playCnt == 0;
        }
        lightLedStatus = sdk_isp_light_led_get_status();
        if (lightLedStatus == 1)
        {
            if (playCnt == 0)
            {
                sdk_isp_set_bulb_mode(ISP_BULB_MODE_MIX);
                sdk_isp_light_led_set(0);
            }

        }
        pthread_mutex_unlock(&playStatus);
    }
    printf("netcam_md_play_stop_one:%d, cnt:%d\n", runImageCfg.lightMode, playCnt);
#endif    
}


int netcam_md_play_get_cnt(void)
{
    return playCnt;
}

int netcam_md_get_md_light_status(void)
{
    return md_light_start;
}

void netcam_md_set_md_light_status(int mdLightStatus)
{
    md_light_start = mdLightStatus;
}

