#include "stdio.h"
#include "stdlib.h"
#include <sys/time.h>
#include <sys/resource.h>

#include "ntp_client.h"
#include "cfg_all.h"
#include "work_queue.h"
#include "netcam_api.h"
#include "flash_ctl.h"
#include "sdk_cfg.h"
#include "sdk_sys.h"
#include "avi_rec.h"
#ifdef MODULE_SUPPORT_GKVISION
#include "gkvision.h"
#endif
#ifdef MODULE_SUPPORT_GK_SEARCH
#include "ipcsearch.h"
#endif

#ifdef MODULE_SUPPORT_CMS
#include "gk_cms_net_api.h"
#endif




#define SYS_CMD_REBOOT		"/sbin/reboot"

static pthread_rwlock_t sys_lock;

static inline int sys_enter_lock(void);
static int sys_leave_lock(void);
static void sys_set_timezone(int gmt);
static void gk_gpio_reset(void);
static void ircut_read_status(void);
extern int ntp_done ;
void netcam_sys_set_time_zone_by_utc_string(char *buf, int zone)
{

    time_t timep,timep1;
    struct tm  tv;
    struct timeval tval;


    time(&timep1);

    printf("-----get time from 1970:%ld\n",timep1);
    if(zone != INVALID_TIEM_ZONE)
    {
        printf("set zone1:%d\n",zone);
        if(zone >= -720 && zone <= 720)
        {
            printf("set zone2:%d\n",zone);
            runSystemCfg.timezoneCfg.timezone = zone;
            sys_set_timezone(zone);
        }
    }

    if(buf != NULL)
    {
        printf("data:%s\n",buf);

        memset(&tv,0,sizeof(struct tm ));
        sscanf(buf,"%d-%d-%d %d:%d:%d", &tv.tm_year ,
                &tv.tm_mon,
                &tv.tm_mday,
                &tv.tm_hour,
                &tv.tm_min,
                &tv.tm_sec);
        tv.tm_year -=1900;
        tv.tm_mon -= 1;


        timep = mktime(&tv);
        printf("mktime %ld will set \n",timep);
        tval.tv_sec = timep+(runSystemCfg.timezoneCfg.timezone*60);
        tval.tv_usec = 0;
        printf("timezone:%d\n",runSystemCfg.timezoneCfg.timezone);
        settimeofday(&tval,NULL);
        timep = time(NULL);

        printf("-----after set,get time from 1970:%ld\n",timep);
        printf("local time:%s\n",ctime(&timep));
    }
}

void netcam_sys_set_time_zone_by_utc_second(int utcOffset, int zone)
{
    time_t timep;
    struct timeval tval;
    //struct timezone tzSet;
    int ret = 0;
    //struct timeval tv;
    //struct timezone tz;
	#if 0
    printf("---------------- 11 ------------\n");
    gettimeofday (&tv, &tz);
    printf("tv_sec; %ld\n", tv.tv_sec);
    printf("tv_usec; %ld\n", tv.tv_usec);
    printf("tz_minuteswest; %ld\n", tz.tz_minuteswest);
    printf("tz_dsttime, %ld\n", tz.tz_dsttime);
	#endif
    timep = time(NULL);
    printf("time() from 1970 :%ld\n",timep);

    //only set timezone
    if(zone != INVALID_TIEM_ZONE && utcOffset == 0)
    {
        printf("set zone1:%d\n",zone);
        if(zone >= -720 && zone <= 720)
        {
            #if 0
            gettimeofday (&tv, NULL);
            tval.tv_sec = tv.tv_sec + (zone - runSystemCfg.timezoneCfg.timezone)*60;
            tval.tv_usec = 0;
            ret = settimeofday(&tval,NULL);
            printf("settimeofday ret =:%d\n",ret);
            timep = time(NULL);
            printf("after set,get time from 1970:%ld\n",timep );
            printf("local time:%s\n",ctime(&timep));

            #endif
            printf("set zone2:%d\n",zone);
            runSystemCfg.timezoneCfg.timezone = zone;
            sys_set_timezone(zone);
            timep = time(NULL);
            printf("after set zone,get time from 1970:%ld\n",timep );
            printf("local time:%s\n",ctime(&timep));

        }
    }
    //set both timezone and utc time
    if(utcOffset != 0 && zone != INVALID_TIEM_ZONE)
    {
        printf("set device time\n");
        //tval.tv_sec必须转为UTC时间
        tval.tv_sec = utcOffset+(runSystemCfg.timezoneCfg.timezone*60);
        printf("tv_sec :%d,offset:%d,timezone:%d\n", (int)tval.tv_sec,utcOffset,runSystemCfg.timezoneCfg.timezone);
        tval.tv_usec = 0;
        ret = settimeofday(&tval,NULL);
        printf("settimeofday ret =:%d\n",ret);
        timep = time(NULL);

        printf("after set,get time from 1970:%ld\n",timep );
        printf("local time:%s\n",ctime(&timep));


        printf("set zone:%d\n",zone);
        if(zone >= -720 && zone <= 720)
        {
            runSystemCfg.timezoneCfg.timezone = zone;
            sys_set_timezone(zone);
        }

    }

}


void netcam_sys_init()
{
    time_t timep;

    pthread_rwlock_init(&sys_lock, NULL);
    //set system time

    if(runSystemCfg.maintainCfg.newest_time)
    {
        struct timeval tval;
        int ret = 0;
        tval.tv_sec = runSystemCfg.maintainCfg.newest_time + 15;
        tval.tv_usec = 0;
        printf("netcam_sys_init set time :%ld\n",tval.tv_sec);
        ret = settimeofday(&tval,NULL);
        printf("settimeofday ret =:%d\n",ret);
    }


    sys_set_timezone(runSystemCfg.timezoneCfg.timezone);

    timep = time(NULL);
    printf(" netcam_sys_init set time,get time from 1970:%ld\n",timep );
    printf(" local time:%s\n",ctime(&timep));
}

void netcam_sys_get_DevInfor(GK_NET_DEVICE_INFO *pstDevInfo)
{
    sys_enter_lock();
    memcpy(pstDevInfo, &runSystemCfg.deviceInfo, sizeof(GK_NET_DEVICE_INFO));
    sys_leave_lock();
}

void netcam_sys_use_timer_init()
{
    // 3 second call one ir-crt check

	if(strstr(sdk_cfg.name, "CUS_HC_GK7202_GC2053_V10") == NULL)
    	netcam_timer_add_task(netcam_iamge_ircut_auto_check, 1*NETCAM_TIMER_ONE_SEC, SDK_FALSE, SDK_FALSE);
    // gpio 复位检查
    netcam_timer_add_task(gk_gpio_reset, NETCAM_TIMER_ONE_SEC, SDK_TRUE, SDK_FALSE);
    netcam_timer_add_task(ircut_read_status, 5*NETCAM_TIMER_ONE_SEC, SDK_FALSE, SDK_FALSE);
    //tf卡状态检查
    #if 0
    netcam_timer_add_task(mmc_sdcard_status_check,  4*NETCAM_TIMER_ONE_SEC, SDK_TRUE, SDK_FALSE);
	#else
	mmc_sdcard_status_check_init();
	#endif
}

void netcam_sys_save()
{

    sys_enter_lock();
    SystemCfgSave();
    sys_leave_lock();
}

void netcam_sys_reboot(int second)
{
	char cmd[32] = {0};
	if(second < 0)
		second = 0;
	snprintf(cmd, sizeof(cmd),"%s -f -d %d ", SYS_CMD_REBOOT, second);
	//netcam_audio_mute(1);
	sleep(second);
	netcam_exit(252);
	new_system_call(cmd);
}

void netcam_sys_get_local_time_string(char *localTimeStr, int *timeZone)
{
    time_t timep;
    struct tm  *tv;

    timep = time(NULL);
    tv = localtime(&timep);
    sprintf(localTimeStr,"%04d-%02d-%02d %02d:%02d:%02d", tv->tm_year+1900 ,
            tv->tm_mon+1,
            tv->tm_mday,
            tv->tm_hour,
            tv->tm_min,
            tv->tm_sec);


   *timeZone = runSystemCfg.timezoneCfg.timezone;
   PRINT_INFO("timeZone:%d  %d\n",*timeZone , runSystemCfg.timezoneCfg.timezone);
}


void netcam_sys_operation(void *fps, void *operation)
{
    NETCAM_SYS_OPERATION oper = (NETCAM_SYS_OPERATION)operation;
    //int ret;

	if((netcam_get_update_status() != 0))
	{
		printf("<<<<<<<netcam_sys_operation %d!, Upgrading, Discard it!>>>>>>>>>\n", oper);
		return;
	}
    switch(oper)
    {
        case SYSTEM_OPERATION_REBOOT:
            PRINT_INFO("system reboot...");

			local_record_uninit();
        
#ifdef MODULE_SUPPORT_GKVISION
            printf("exit gkvision\n");
            gk_vision_exit();
#endif

#ifdef MODULE_SUPPORT_GOKE_UPGRADE
            goke_upgrade_udp_info(1, NULL);
#endif

			netcam_exit(252);
            #ifdef MODULE_SUPPORT_WATCHDOG
            netcam_watchdog_set_timeout(0);
			sleep(2);
            #endif
            netcam_sys_reboot(0);
			sleep(2);

            break;
		case SYSTEM_OPERATION_RESTART_APP:
			PRINT_INFO("app restart ...");

			local_record_uninit();
#ifdef MODULE_SUPPORT_GOKE_UPGRADE
            goke_upgrade_udp_info(1, NULL);
#endif

			netcam_exit(252);
            #ifdef MODULE_SUPPORT_WATCHDOG
            netcam_watchdog_set_timeout(0);
			sleep(2);
            #endif
            netcam_sys_reboot(0);
			sleep(2);

            while(1)
            {
                ;
            }
            
			exit(111);
            
			break;
        case SYSTEM_OPERATION_HARD_DEFAULT:	// revet all configuration file including cfg/* and onvif/* except onvif/factory.
		{

			local_record_uninit();

			char str_null[2];
			str_null[0]= 0;
			PRINT_INFO("system will be reset to the factory default(hard)...");
            PRINT_INFO("app restart...");
            if(delete_cjson_file_frome_dir(CFG_DIR))
            {
                PRINT_ERR("del cjson file form dir:%s fail\n", CFG_DIR);
            }

            if(delete_path("/opt/custom/onvif/"))
                PRINT_ERR("rmdir %s fail\n", "/opt/custom/onvif/");
            #ifdef MODULE_SUPPORT_MOFANG_PLUGIN
            new_system_call("rm -f /opt/custom/cfg/Plugin");
            #endif
            PRINT_INFO("sync end...");
            //netcam_audio_hint(SYSTEM_AUDIO_HINT_RECOVERY_SUCCESS);
#ifdef MODULE_SUPPORT_GK_SEARCH
            ipc_search_save_to_resave_wifi(str_null,str_null,0); //delete reserve wifi info
#endif
            //netcam_audio_out(HINT_SYS_RECOVERY);
            
#ifdef MODULE_SUPPORT_MOJING_V5
            new_system_call("rm -rf /opt/custom/cfg/gk_mj_*");
#endif
            
#ifdef MODULE_SUPPORT_GOKE_UPGRADE
            goke_upgrade_udp_info(2, NULL);
#endif
            sleep(3);
			netcam_exit(252);
            #ifdef MODULE_SUPPORT_WATCHDOG
            netcam_watchdog_set_timeout(10);
            #endif
            sleep(3);
            new_system_call("reboot -f");

            while(1)
            {
                ;
            }
			break;
        }
		case SYSTEM_OPERATION_SOFT_DEFAULT:	//except network configuration file.

			local_record_uninit();

			PRINT_INFO("system will be reset to the factory default(soft)...");
			PRINT_INFO("app restart...");
            //rename("/opt/custom/cfg/gk_network_cfg.cjson", "/tmp/gk_network_cfg.cjson");
            new_system_call("cp /opt/custom/cfg/gk_network_cfg.cjson /tmp/gk_network_cfg.cjson");
            if(delete_cjson_file_frome_dir(CFG_DIR))
                PRINT_ERR("del cjson file form dir:%s fail\n", CFG_DIR);
            //rename("/tmp/gk_network_cfg.cjson", "/opt/custom/cfg/gk_network_cfg.cjson");
            new_system_call("cp -f /tmp/gk_network_cfg.cjson /opt/custom/cfg/gk_network_cfg.cjson");
            #ifdef MODULE_SUPPORT_MOFANG_PLUGIN
            new_system_call("rm -f /opt/custom/cfg/Plugin");
            #endif
            PRINT_INFO("sync file end");
            //netcam_audio_out(HINT_SYS_RECOVERY);
#ifdef MODULE_SUPPORT_GOKE_UPGRADE
            goke_upgrade_udp_info(2, NULL);
#endif
            sleep(3);
            netcam_exit(252);
            #ifdef MODULE_SUPPORT_WATCHDOG
            netcam_watchdog_set_timeout(10);
            #endif
            sleep(3);
            new_system_call("reboot -f");
            while(1)
            {
                ;
            }
			break;
        default:
            break;
    }

	return ;
}

int netcam_sys_ntp_get(int *enNTP, char *ntpserver, int serLen, int *port)
{
	*enNTP = runSystemCfg.ntpCfg.enable;
	strncpy(ntpserver, runSystemCfg.ntpCfg.serverDomain, serLen-1);
	*port = runSystemCfg.ntpCfg.port;
	return 0;
}

int netcam_sys_ntp_set(char *ntpserver, int serLen, int port)
{
	ntpc_set_addr(ntpserver, serLen, port);
	return 0;
}

int netcam_sys_ntp_start_stop(int enNTP)
{
	if(enNTP == 0)
		ntpc_disable();
	else
		ntpc_enable();

	return 0;
}

int netcam_sys_init_maintain()
{
	if (runSystemCfg.maintainCfg.enable == 0)
		return 0;
	#if 0
	time_t t = time(NULL);
    struct tm pt;
	localtime_r(&t, &pt);
    printf("netcam_sys_init_maintain get localtime = %d-%d-%d %d:%d:%d\n",
        pt.tm_year,
        pt.tm_mon,
        pt.tm_mday,
        pt.tm_hour,
        pt.tm_min,
        pt.tm_sec);

	if (pt.tm_year + 1900 < 2015)
		return -1;
    #endif
    if(0 == ntp_done )
    {
        return -1;
    }
    else
    {
        printf("ntp done! run maintain!\n");

    }
    int hour, min, second, index;
    hour = runSystemCfg.maintainCfg.hour;
    min = runSystemCfg.maintainCfg.minute;
    second = runSystemCfg.maintainCfg.second;
    index = runSystemCfg.maintainCfg.index;

    PRINT_INFO("set index:%d time %d:%d:%d\n", index, hour, min, second);

    if (index == 7)
    {
		CRTSCH_DEFAULT_WORK(EVERY_DAY_TIMER_WORK, COMPUTE_TIME(0, hour, min, second, 0), (WORK_CALLBACK)maintain_handle);
    }
    else
    {
		CRTSCH_DEFAULT_WORK(EVERY_WEEK_TIMER_WORK, COMPUTE_TIME(index, hour, min, second, 0), (WORK_CALLBACK)maintain_handle);
    }

    return 0;
}

char *netcam_sys_get_name(void)
{
    static char *devName = NULL;

    if (devName == NULL)
    {
        devName = (char*)malloc(64);
    }

    if (devName == NULL)
    {
        return NULL;
    }

    sprintf(devName, "%s_%s", runSystemCfg.deviceInfo.deviceType, runSystemCfg.deviceInfo.serialNumber);
    return devName;
}

int netcam_sys_shell_result(char *cmd, char *buf, int buflen)
{
    FILE *fp = NULL;
	int len = 0;

	if(!cmd || !buf)
		return -1;

	if (NULL == (fp = popen(cmd, "r")))
	{
		goto err;
	}

	if (NULL == fgets(buf, buflen, fp))
	{
		goto err;
	}

	len = strlen(buf);
	if(len > 0)
	{
		if(buf[len-1] == '\n')
			buf[len-1] = '\0';
	}
	else
		goto err;

	pclose(fp);

	return 0;

err:
	if(fp)
		pclose(fp);

	return -1;
}


static inline int sys_enter_lock(void)
{
	return pthread_rwlock_wrlock(&sys_lock);
}

static int sys_leave_lock(void)
{
	return pthread_rwlock_unlock(&sys_lock);
}

//mintinus -720,+720
static void sys_set_timezone(int gmt)
{
    char *retChar = NULL;
	if(gmt >= -720 && gmt <= 720)
	{
		int const hour = abs(gmt) / 60;
		int const min = abs(gmt) % 60;

		if(min >= 0 && min < 60)
		{
			char text[32] = {""};
			//struct timezone tz;
			//tz.tz_minuteswest = gmt;
			//tz.tz_dsttime = 0;



            retChar = getenv("TZ");
            if(retChar != NULL)
                {
                    printf("befor se TZ:get env TZ:%s\n",retChar);
                }
            else
                {
                    printf("befor se TZ:get env TZ IS NULL\n");
                }

            snprintf(text, sizeof(text), "UTC%c%02d:%02d", gmt < 0 ? '+' : '-',  hour, min); // opposite to GMT
            printf("set TZ :%s\n",text);

            setenv("TZ", text, 1);
			tzset();
            sleep(1);

            retChar = getenv("TZ");
            if(retChar != NULL)
                {
                    printf("after set TZ:get env TZ:%s\n",retChar);
                }
            else
                {
                    printf("after set TZ:get env TZ IS NULL\n");
                }
			//gettimeofday(NULL,&tz);
            //printf("tz:%d,%d\n",tz.tz_minuteswest,tz.tz_dsttime);
		}
	}

}

static int saved_ircut_value = 0;
void ircut_read_status(void)
{
    int value;
    sdk_gpio_get_intput_value(52, &value);
    if(value != saved_ircut_value) {
	    sdk_isp_set_ircut_mode(value + 1);
	    saved_ircut_value = value;
	}
}


static void gk_gpio_reset(void)//gpio0 valu==0 is enble
{
    int valu = 1;
    int valu2 = 1;
	static int reset_count = 0;


    if(sdk_cfg.reset.gpio_reset_value == -1 && sdk_cfg.reset2.gpio_reset_value == -1 ) {
        //printf("error, gpio_reset:%d, gpio_reset_value:%d\n", sdk_cfg.reset.gpio_reset, sdk_cfg.reset.gpio_reset_value);
        return ;
    }

    //printf("ok, gpio_reset:%d, gpio_reset_value:%d\n", sdk_cfg.reset.gpio_reset, sdk_cfg.reset.gpio_reset_value);

    int ret = sdk_gpio_get_intput_value(sdk_cfg.reset.gpio_reset, &valu);
	if (ret != 0) {
        printf("gk_gpio_reset %d, error, ret:%d\n", sdk_cfg.reset.gpio_reset, ret);
		return ;
    }


    if (sdk_cfg.reset2.gpio_reset != -1)
    {
        ret = sdk_gpio_get_intput_value(sdk_cfg.reset2.gpio_reset, &valu2);
    	if (ret != 0) {
            printf("gk_gpio_reset %d, error, ret:%d\n", sdk_cfg.reset2.gpio_reset, ret);
    		return ;
        }
    }
    //printf("ok, valu:%d\n", valu);

	//printf("===========valu:%d, gpio:%d, value:%d, reset_count:%d\n", valu, sdk_cfg.reset.gpio_reset,
	//	sdk_cfg.reset.gpio_reset_value, reset_count);
	if (valu != sdk_cfg.reset.gpio_reset_value && valu2 != sdk_cfg.reset2.gpio_reset_value)
	{
		reset_count = 0;
		return ;
	}

	reset_count++;
#ifdef MODULE_SUPPORT_CMS
    if (reset_count > 3 || Gk_CmsGetFactoryKeyTestStatus())
    {
        //todo reset system
        PRINT_INFO("&&&&&&&&&&&&&&&&& gpio reset &&&&&&&&&&&&-%d\n", Gk_CmsGetFactoryKeyTestStatus());
        if (Gk_CmsGetFactoryKeyTestStatus())
        {
            Gk_CmsSetFactoryKeyTestResult(1);
        }
        else
        {
            netcam_sys_operation(NULL, (void*)SYSTEM_OPERATION_HARD_DEFAULT);
        }
    }    
#else      
	if (reset_count > 3)
	{
		//todo reset system
        PRINT_INFO("&&&&&&&&&&&&&&&&& gpio reset &&&&&&&&&&&&\n");
        netcam_sys_operation(NULL, (void *)SYSTEM_OPERATION_HARD_DEFAULT);
	}
#endif


}

