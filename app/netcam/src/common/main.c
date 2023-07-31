#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "sys/stat.h"
#include "unistd.h"
#include "netcam_api.h"
#include "onvif.h"
#include "sdk_debug.h"
#include "sdk_cfg.h"
#include "utility_api.h"
#include <time.h>
#include "ot_common_aio.h"
#ifdef MODULE_SUPPORT_RTSP
#include "gk_rtsp.h"
#endif

#ifdef MODULE_SUPPORT_HTTP
#include "http_export.h"
#endif

#ifdef MODULE_SUPPORT_CMS
#include "gk_cms_session.h"
#endif

#ifdef MODULE_SUPPORT_OLD_CMS
#include "cms.h"
#endif

#ifdef MODULE_SUPPORT_DANA
#include "dana_access_export.h"
#endif

#ifdef MODULE_SUPPORT_TUTK
#include "tutk_access_export.h"
#include "ds_search.h"
#endif

//#ifdef MODULE_SUPPORT_NTP
//#include "ntp_client.h"
//#endif

#ifdef MODULE_SUPPORT_LOCAL_REC
#include "avi_rec.h"
#endif

#ifdef MODULE_SUPPORT_YUNNI
#include "app_yunni.h"
#endif

#ifdef MODULE_SUPPORT_WORKQUEUE
#include "work_queue.h"
#endif

#ifdef MODULE_SUPPORT_REC_PCM
#include "rec_pcm.h"
#endif

#ifdef MODULE_SUPPORT_ULUCU
#include "app_ulucu.h"
#endif

#ifdef AUDIO_ECHO_CANCELLATION_SUPPORT
#include "webrtc_aec.h"
#endif

#ifdef MODULE_SUPPORT_MTWS_P2P
#include "phone_mtws.h"
#include "ds_search.h"
#endif

#ifdef MODULE_SUPPORT_RF433
#include "rf433.h"
#endif

#ifdef MODULE_SUPPORT_FACTORY_TEST
#include "factory_test.h"
#endif

#ifdef MODULE_SUPPORT_PPCS
#include "ppcs_access_export.h"
#endif

#ifdef MODULE_SUPPORT_GOOLINK
#include "goolink_access_export.h"
#endif

#ifdef MODULE_SUPPORT_K24C02_EEPROM
#include "k24c02_eeprom.h"
#endif

#ifdef MODULE_SUPPORT_SHELL
#include "shell_start.h"
#endif

#ifdef MODULE_SUPPORT_MOJING
#include "mojing.h"
#endif

#ifdef MODULE_SUPPORT_GKVISION
#include "gkvision.h"
#endif

#ifdef MODULE_SUPPORT_GK_SEARCH
#include "ipcsearch.h"
#endif

#ifdef MODULE_SUPPORT_NTP
#include "ntp_client.h"
#endif

#include "eventalarm.h"


static sem_t  main_run_sem;
extern ot_aio_attr aio_attr;
static void http_stop()
{
}

static void *agingTestThread(void *args)
{
    int cnt = 0;
    sleep(10);
    sdk_isp_ircut_led_set(0);
    sdk_isp_led_test_start();
    while(1)
    {
        printf("agingTestThread, cnt:%d\n", cnt++);
        sdk_isp_light_led_set(1);
        sleep(5);
        sdk_isp_light_led_set(0);
        sleep(1);
        sdk_isp_ircut_led_set(1);
        sleep(5);
        sdk_isp_ircut_led_set(0);
        sleep(1);        
        //netcam_audio_out(AUDIO_FILE_STARTING);
        sleep(30);
    }
    return NULL;
}

static void agingTestCheck(void)
{
    pthread_t testThread;
    if (access("/mnt/sd_card/agingTestGoke", F_OK) == 0)
    {
        pthread_create(&testThread, NULL, agingTestThread, NULL);
    }
    else
    {
        printf("not start agingtest.\n");
    }
}



static int http_start()
{
    if (http_mini_server_init("0.0.0.0", 80, RESOURCE_WEB_DIR) < 0)
    {
        printf("Create http server failed\n");
    }
    else
    {
        printf("Create http server success\n");
    }

    #ifndef MODULE_NOT_SUPPORT_WEB
    netcam_http_web_init();
    #endif
    #ifdef MODULE_SUPPORT_ONVIF    
        #ifdef MODULE_SUPPORT_MOJING
            if (mojing_get_onvif_status() == 1)
        #endif
                netcam_http_onvif_init();
    #endif
    http_mini_server_run();
    return 0;
}

static int netcam_audio_boot_finish(void* arg)
{
	if(sdk_ptz_get_startup_adjust_flag())
		return -1;

	//netcam_audio_out(AUDIO_FILE_START_FINISH);

	return 0;
}

static void start_all_module()
{
    //agingTestCheck();

#ifdef MODULE_SUPPORT_ENCRYPT_AUTHOR
	if(encrypt_authentication() != 0)
	{
		LOG_INFO("Hard authentication error\n");
		while(1)
		sleep(1);
	}
#endif

#ifdef MODULE_SUPPORT_K24C02_EEPROM
	if(k24c02_eeprom_data_check() != 0)
	{
		LOG_INFO("k24c02_eeprom_data_check error\n");
		#ifdef MODULE_SUPPORT_WATCHDOG
		while(1)
		sleep(1);
		#else
		netcam_sys_operation(0,SYSTEM_OPERATION_REBOOT);
		#endif
	}
#endif


#ifdef MODULE_SUPPORT_MTWS_P2P
    printf("support mtws p2p\n");
    mtws_app_init();
    ds_search_start();
#endif

#ifdef MODULE_SUPPORT_QRCODE_LINK
    //魔镜在有线没有插入时才启用
    #ifndef MODULE_SUPPORT_MOJING
    printf("MODULE_SUPPORT_QRCODE_LINK\n");
    zbar_qrcode_link_init();
    #endif
#endif

	netcam_net_monitor();

    // 事件告警模块
    event_alarm_init();

#ifdef MODULE_SUPPORT_GK_SEARCH
    printf("support search tools\n");
    IPCsearch_init();
#endif

#ifdef MODULE_SUPPORT_CMS
    printf("support cms\n");
    gk_cms_start();
#endif

#ifdef MODULE_SUPPORT_OLD_CMS
    cms_start();
#endif

#ifdef MODULE_SUPPORT_TUTK
	printf("support tutk p2p\n");
	tutklib_start();
    //ds_search_start();
#endif

#ifdef MODULE_SUPPORT_LOCAL_REC
    printf("support local record\n");
    local_record_init();
#endif

    //uvc_start("-h264", "-1080p");
    //uvc_start("-mjpg", "-1080p");

#ifdef MODULE_SUPPORT_NTP
    printf("support NTP\n");
    start_ntp_client();
#endif

#ifdef MODULE_SUPPORT_YUNNI_P2P
    printf("support yunnikeji p2p\n");
    yunni_p2p_start();
#endif

#ifdef MODULE_YUDP
	netcam_yudp_start();
#endif

#ifdef MODULE_SUPPORT_REC_PCM
    printf("rec pcm file to test\n");
    rec_pcm_test_start();
#endif

#ifdef MODULE_SUPPORT_ULUCU
	printf("support ulucu p2p\n");
	ulucu_p2p_start();
#endif

#ifdef AUDIO_ECHO_CANCELLATION_SUPPORT
#ifndef MODULE_SUPPORT_MTWS_P2P
    //printf("support audio send loop\n");
    //audio_send_start();
#endif
#else
    printf("not support audio send loop\n");
#endif

#ifdef MODULE_SUPPORT_WORKQUEUE
	CRTSCH_DEFAULT_WORK(CONDITION_TIMER_WORK, COMPUTE_TIME(0,0,0,10,0), (WORK_CALLBACK)netcam_sys_init_maintain);
    if(0 == runAudioCfg.rebootMute)
    {
        printf("runAudioCfg.rebootMute = %d,enable audio play system finished\n",runAudioCfg.rebootMute);
		
//		if(!strstr(sdk_cfg.name, "CUS_TB_GC2053_V20"))
//        	CRTSCH_DEFAULT_WORK1(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), netcam_audio_out, AUDIO_FILE_START_FINISH);
//		else
			CRTSCH_DEFAULT_WORK1(CONDITION_TIMER_WORK, COMPUTE_TIME(0,0,0,2,0), netcam_audio_boot_finish, NULL);
    }
    else
    {
        printf("runAudioCfg.rebootMute = %d,disable audio play system finished\n",runAudioCfg.rebootMute);
    }
#endif

#ifdef MODULE_SUPPORT_RF433
	rf433_Init(NULL);
#endif

#ifdef MODULE_SUPPORT_FACTORY_TEST
	vFactoryWorkMode_Init();
#endif

#ifdef MODULE_SUPPORT_PPCS
	ppcs_start();
#endif

#ifdef MODULE_SUPPORT_ZRAM
	//start zram
	system("echo 8M > /sys/block/zram0/disksize");
	system("mkswap /dev/zram0");
	system("swapon /dev/zram0");
#endif


#ifdef MODULE_SUPPORT_DANA
    printf("support dana\n");
    danalib_start();
#endif

#ifdef MODULE_SUPPORT_XMAI
    printf("support xmai protocol\n");
    lib_xmai_start();
#endif

#ifdef MODULE_SUPPORT_DAHUA
    printf("support dahua protocol\n");
    dahua_protocol_start();
#endif

#ifdef MODULE_SUPPORT_HIK
    printf("support hik protocol\n");
    lib_hik_start();
#endif

#ifdef MODULE_SUPPORT_GKVISION
    printf("support gkvision\n");
    GK_VISION_HandleT visionParam;
    memset(&visionParam, 0, sizeof(GK_VISION_HandleT));

    visionParam.ptzCfg.leftRightKp = 120;
    visionParam.ptzCfg.upDownKp = 50;
    visionParam.ptzCfg.speed = 16;
    visionParam.debugEnable = 1;
    visionParam.osdEnable   = 0;
    visionParam.imgUpload   = 0;

    gk_vision_init(&visionParam);
#endif

#ifdef MODULE_SUPPORT_GB28181
    printf("support gb28181\n");
    gb28181_start();
#endif

#ifdef MODULE_SUPPORT_MOJING
    printf("support mojing\n");
    mojing_start();
#endif

#ifdef MODULE_SUPPORT_RTSP
    printf("support rtsp\n");
#ifdef MODULE_SUPPORT_MOJING
    if (mojing_get_onvif_status() == 1)
#endif
    rtsp_start();
#endif


#ifdef MODULE_SUPPORT_ONVIF
	//!!!onvif server must be called before starting HTTP server below.
    printf("support onvif\n");
    #ifdef MODULE_SUPPORT_MOJING
    if (mojing_get_onvif_status() == 1)
    #endif
    if(access("/opt/custom/cfg/onvifDisable", F_OK) != 0)
       onvif_start(GK_TRUE, 80);
    
#endif

#ifdef MODULE_SUPPORT_HTTP
    printf("support http\n");
    http_start();
#endif


#ifdef MODULE_SUPPORT_GOKE_UPGRADE
    printf("support goke upgrade\n");
    goke_upgrade_start();
#endif

#ifdef MODULE_SUPPORT_GOKE_SHARE_INFO
    printf("support goke share info\n");
    goke_share_start();
#endif

#ifdef MODULE_SUPPORT_GOKE_SERVER
    printf("support goke server\n");
    goke_server_start();
#endif

#ifdef MODULE_SUPPORT_AGING
    printf("support aging test\n");
    aging_test_start();
#endif


#ifdef MODULE_SUPPORT_DOULIAN_SERVER
    printf("support doulian server\n");
    doulian_server_start();
#endif


#ifdef MODULE_SUPPORT_GUI
    printf("support lvgl ui\n");
    gapp_ui_init();
#endif

#ifdef MODULE_SUPPORT_FACE_6323
    printf("support face6323\n");
    goke_face_start();
#endif

#ifdef MODULE_SUPPORT_GOOLINK
    printf("support goolink\n");
	goolink_start();
#endif

#ifdef MODULE_SUPPORT_YUNTUN
    printf("support yuntun\n");
    yuntunRtmpStart();
#endif	

}

static void stop_all_module()
{
    netcam_net_exit();
#ifdef MODULE_SUPPORT_WORKQUEUE
	stop_default_workqueue();
#endif

#ifdef MODULE_SUPPORT_RTSP
    rtsp_stop();
#endif

#ifdef MODULE_SUPPORT_ONVIF
    onvif_stop();
#endif

#ifdef MODULE_SUPPORT_CMS
    gk_cms_stop();
#endif

#ifdef MODULE_SUPPORT_OLD_CMS
    cms_stop();
#endif

#ifdef MODULE_SUPPORT_HTTP
    http_stop();
#endif

#ifdef MODULE_SUPPORT_DANA
    danalib_exit();
#endif

#ifdef MODULE_SUPPORT_LOCAL_REC
    printf("support local record\n");
    local_record_uninit();
#endif

#ifdef MODULE_SUPPORT_YUNNI
    printf("support yunnikeji p2p\n");
    //yunni_p2p_stop(); // it will cause system halt
#endif

#ifdef MODULE_SUPPORT_ULUCU
    printf("support ulucu p2p\n");
    ulucu_p2p_stop();
#endif

#ifdef MODULE_SUPPORT_TUTK
	printf("support tutk p2p\n");
	tutklib_exit();
    ds_search_exit();
#endif


#ifdef MODULE_SUPPORT_MTWS_P2P
    printf("support mtws p2p\n");
    mtws_app_exit();
    ds_search_exit();
#endif


#ifdef AUDIO_ECHO_CANCELLATION_SUPPORT
#ifndef MODULE_SUPPORT_MTWS_P2P
    printf("support audio send loop\n");
    audio_send_stop();
#endif
#endif

#ifdef MODULE_SUPPORT_RF433
	rf433_Exit();
#endif
#ifdef MODULE_SUPPORT_FACTORY_TEST
	vFactoryWorkMode_Exit();
#endif

#ifdef MODULE_SUPPORT_PPCS
	ppcs_exit();
#endif

#ifdef MODULE_SUPPORT_GOOLINK
	goolink_exit();
#endif

#ifdef MODULE_SUPPORT_DAHUA
	dahua_protocol_stop();
#endif

#ifdef MODULE_SUPPORT_XMAI
    lib_xmai_stop();
#endif

#ifdef MODULE_SUPPORT_HIK
    printf("support hik protocol\n");
    lib_hik_stop();
#endif

#ifdef MODULE_SUPPORT_GKVISION
    printf("support gkvision exit\n");
    gk_vision_exit();
#endif

}

static int netcam_start()
{
    //atexit(netcam_video_exit);

#ifdef MODULE_SUPPORT_SHELL
	shell_test_start();
#endif

#ifdef MODULE_SUPPORT_GB28181
	GB28181Cfg_init();
#endif

#if defined MODULE_SUPPORT_TUTK || defined MODULE_SUPPORT_GOOLINK
	P2PCfg_init();
#endif
    //装载配置文件
    CfgInit();

	//netcam_p2pid_init();
    //优先启动升级程序
    netcam_update_init(NULL);


#ifdef MODULE_SUPPORT_GOKE_UPGRADE
    if (access("/opt/resource/audio/zh/dev_starting.alaw", F_OK) != 0)
    {
        //音频文件访问失败，则表明资源分区失效，升级恢复
        new_system_call("touch /tmp/feeddog");
        netcam_net_init();
        printf("support goke upgrade\n");
        goke_upgrade_start();
        while(1)
        {
            printf("wait for upgrade..\n");
            sleep(1);
        }
    }
#endif

    #ifdef MODULE_SUPPORT_WATCHDOG
    //初始化看门狗
    if(netcam_watchdog_init(45) != 0)
    {
        LOG_ERR("Watch dog up failed, force reboot");
        //netcam_sys_reboot(0);
		netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT
    }
    #endif
    /**/
    //hi need 
    int vencStream_cnt =  netcam_video_get_channel_number();
    int i;
    int enSize[16];
    for(i=0; i<vencStream_cnt; i++)
    {
        enSize[i] = (int)netcam_video_get_pic_format(runVideoCfg.vencStream[i].h264Conf.width,
                                        runVideoCfg.vencStream[i].h264Conf.height);
    }
	//sdk_sys_init(vencStream_cnt, enSize);
	sdk_sys_init(vencStream_cnt);
//	netcam_ptz_init();
    netcam_sys_init();
    //初始化OSD更新任务
    netcam_timer_init();
    //初始化定时器任务, 因为升级功能需要用到该任务。
    start_default_workqueue();
    netcam_net_init();
    
    //netcam_audio_init();
    netcam_video_init();
    //venc_audio_start();
	netcam_md_init(1);
    
	//netcam_ftp_start();
    #ifdef MODULE_SUPPORT_MAIL
	netcam_mail_start();
	#endif
    #if 0
	netcam_osd_init();
    #endif
    //netcam_pm_init();
    //venc_audio_start();
    svp_main();
    netcam_image_init();
	//netcam_autolight_init();
    netcam_net_wifi_init();

    printf("==================================netcam_start end==============\n");
    printf("==================================netcam_start end==============\n");
    printf("==================================netcam_start end==============\n");

	//netcam_sys_use_timer_init();
    return 0;
}


void ignore_SIGPIPE(void)
{
    //忽略tcp send 出错发出的SIGPIPE，避免程序因为该信号被终止
    sigset_t signal_mask;
    sigemptyset (&signal_mask);
    sigaddset (&signal_mask, SIGPIPE);
    int rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);
    if (rc != 0)
    {
        printf("block sigpipe error/n");
    }
}

int main(int argc, char *argv[])
{
#ifdef MODULE_SUPPORT_WATCHDOG
    struct tm *ptm;
    long ts;
    struct tm tt = {0};
    char str[64] = {0};
#endif

    capture_all_signal();
    ignore_SIGPIPE();
    sem_init(&main_run_sem,0,0);
    //设置为行缓冲，避免重定向时数据不及时
    setvbuf(stdout, NULL, _IOLBF, 0);


    netcam_start();
//#ifdef MODULE_SUPPORT_WATCHDOG
//    netcam_watchdog_feed();
//#else
//    netcam_update_feed_dog();
//#endif
    start_all_module();
    sample_audio_adec_ao(1, aio_attr);

    //阻塞等待，不用循环
    while(1)
    {
	    sleep(15);
#if 0
        // 如果video异常，那么不进行feed操作
        #if 0
        if (netcam_video_check_alive() < 0)
        {
            LOG_WARNING("encode exception...");
            system("cat /proc/interrupts ");
			sleep(1);
			if(netcam_get_update_status() == 0)
            	continue;
        }
        #endif


        #ifdef MODULE_SUPPORT_WATCHDOG
        ts = time(NULL);
        ptm = localtime_r(&ts, &tt);
        sprintf(str, "%04d%02d%02d_%02d:%02d:%02d", ptm->tm_year+1900,
                                                 ptm->tm_mon+1,
                                                 ptm->tm_mday,
                                                 ptm->tm_hour,
                                                 ptm->tm_min,
                                                 ptm->tm_sec);

        printf("[%s]feed dog\n", str);
        netcam_watchdog_feed();
        #else
        printf("feed dog netcam\n");
        netcam_update_feed_dog();
        #endif

        #if 0
        gettimeofday(&timeVal, &tt);
        timeout.tv_sec = timeVal.tv_sec + 20; // 20 second timeout
        ret = sem_timedwait(&main_run_sem, &timeout);
        if(ret == 0)
            break;
        #else
        sleep(15);
        #endif
#endif
    }
    stop_all_module();
    return 0;
}

