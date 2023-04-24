#include "work_queue.h"
#include "netcam_api.h"

int maintain_handle(void* arg)
{
	PRINT_INFO("Maintain system reboot...");
    runAudioCfg.rebootMute = 1;
    AudioCfgSave();
    sleep(1);

    int old_mode =0;
    old_mode = runRecordCfg.recordMode;
    if(3 != runRecordCfg.recordMode)
    {
        runRecordCfg.recordMode = 3;
    }
    
    thread_record_close();

    time_t t = time(NULL);
    runSystemCfg.maintainCfg.newest_time = t;
    SystemCfgSave();
    PRINT_INFO("before reboot save utc time : %ld\n",t);
    sleep(1);

    netcam_exit(90);
	netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT
	system("/sbin/reboot");
	sleep(2);
    #ifdef MODULE_SUPPORT_WATCHDOG
	netcam_watchdog_set_timeout(1);
    #endif

	return 0;
}


