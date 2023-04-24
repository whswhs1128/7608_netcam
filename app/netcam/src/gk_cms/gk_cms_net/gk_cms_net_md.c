#include "gk_cms_common.h"
#include "gk_cms_media.h"
#include "gk_cms_sock.h"
#include "gk_cms_protocol.h"
#include "gk_cms_utility.h"
#include "gk_cms_net_api.h"
#include "cfg_all.h"
#include "netcam_api.h"

int Gk_CmsGetMd(int sock)
{
    DMS_NET_CHANNEL_MOTION_DETECT md;

    //to do
    memset(&md, 0, sizeof(DMS_NET_CHANNEL_MOTION_DETECT));
    md.dwSize = sizeof(DMS_NET_CHANNEL_MOTION_DETECT);
    md.bEnable = runMdCfg.enable;
    md.dwSensitive = runMdCfg.sensitive;
    //md.bManualDefence = runMdCfg.schedule_mode;
    md.stHandle.wDelayTime = runMdCfg.handle.intervalTime;
    md.stHandle.wActionFlag = 0;

    if (runMdCfg.handle.is_rec == 1) {
        md.stHandle.wActionFlag |= GK_ALARM_EXCEPTION_TOREC;
        md.stHandle.byRecordChannel[0] |= 0x01;
        md.stHandle.wRecTime = runMdCfg.handle.recTime;
    } else {
        md.stHandle.wActionFlag &= ~(GK_ALARM_EXCEPTION_TOREC);
        md.stHandle.byRecordChannel[0] &= ~(0x01);
    }

	if (runMdCfg.handle.isSnapUploadToFtp == 1) {
        md.stHandle.wActionFlag |= GK_ALARM_EXCEPTION_TOFTP;
    } else {
        md.stHandle.wActionFlag &= ~(GK_ALARM_EXCEPTION_TOFTP);
    }
	
    if (runMdCfg.handle.isSnapSaveToSd == 1) {
        md.stHandle.wActionFlag |= GK_ALARM_EXCEPTION_TOSNAP;
        md.stHandle.bySnap[0] |= 0x01;
        md.stHandle.wSnapNum = runMdCfg.handle.snapNum;
    } else {
        md.stHandle.wActionFlag &= ~(GK_ALARM_EXCEPTION_TOSNAP);
        //md.stHandle.wActionFlag &= ~(GK_ALARM_EXCEPTION_TOFTP);
        md.stHandle.bySnap[0] &= ~(0x01);
    }

    if (runMdCfg.handle.is_alarmout == 1) {
        md.stHandle.wActionFlag |= GK_ALARM_EXCEPTION_TOALARMOUT;
        md.stHandle.byRelAlarmOut[0] |= 0x01;
        md.stHandle.wDuration = runMdCfg.handle.duration;
    } else {
        md.stHandle.wActionFlag &= ~(GK_ALARM_EXCEPTION_TOALARMOUT);
        md.stHandle.byRelAlarmOut[0] &= ~(0x01);
    }

    if (runMdCfg.handle.is_ptz == 1) {
        md.stHandle.stPtzLink[0].byType = runMdCfg.handle.ptzLink.type;
		md.stHandle.stPtzLink[0].byValue = runMdCfg.handle.ptzLink.value + 1;
    }

    if (runMdCfg.handle.is_beep == 1) {
        md.stHandle.wActionFlag |= GK_ALARM_EXCEPTION_TOBEEP;
        md.stHandle.wBuzzerTime = runMdCfg.handle.beepTime;
    } else {
        md.stHandle.wActionFlag &= ~(GK_ALARM_EXCEPTION_TOBEEP);
    }

    if (runMdCfg.handle.is_email == 1) {
        md.stHandle.wActionFlag |= GK_ALARM_EXCEPTION_TOEMAIL;
    } else {
        md.stHandle.wActionFlag &= ~(GK_ALARM_EXCEPTION_TOEMAIL);
    }

    //md.stHandle.wActionMask = ~0;
    md.stHandle.wActionMask = 0;
    md.stHandle.wActionMask |= GK_ALARM_EXCEPTION_TOREC;
    md.stHandle.wActionMask |= GK_ALARM_EXCEPTION_TOSNAP;
    md.stHandle.wActionMask |= GK_ALARM_EXCEPTION_TOFTP;
    //md.stHandle.wActionMask |= GK_ALARM_EXCEPTION_TOALARMOUT;
    md.stHandle.wActionMask |= GK_ALARM_EXCEPTION_TOPTZ;
    md.stHandle.wActionMask |= GK_ALARM_EXCEPTION_TOBEEP;
    md.stHandle.wActionMask |= GK_ALARM_EXCEPTION_TOEMAIL;

    int i, j;
    for(i = 0 ; i < 7; i ++) {
        for(j = 0 ; j < 4; j ++) {
            #if 0
            PRINT_INFO("get day[%d]-index[%d] time:[%d:%d] - [%d:%d]\n", i, j, 
                    runMdCfg.scheduleTime[i][j].startHour, runMdCfg.scheduleTime[i][j].startMin,
                    runMdCfg.scheduleTime[i][j].stopHour, runMdCfg.scheduleTime[i][j].stopMin);
            #endif
            md.stScheduleTime[i][j].byStartHour = runMdCfg.scheduleTime[i][j].startHour;
            md.stScheduleTime[i][j].byStartMin = runMdCfg.scheduleTime[i][j].startMin;
            md.stScheduleTime[i][j].byStopHour = runMdCfg.scheduleTime[i][j].stopHour;
            md.stScheduleTime[i][j].byStopMin = runMdCfg.scheduleTime[i][j].stopMin;
        }
    }

    #if 0
    printf("get: \n");
    printf("flag:%u\n", md.stHandle.wActionFlag);
    printf("wDuration:%u\n", md.stHandle.wDuration);
    printf("wRecTime:%u\n", md.stHandle.wRecTime);
    printf("wSnapNum:%u\n", md.stHandle.wSnapNum);
    printf("wDelayTime:%u\n", md.stHandle.wDelayTime);
    printf("wBuzzerTime:%u\n", md.stHandle.wBuzzerTime);
    #endif

    return GkCmsCmdResq(sock, &md, sizeof(DMS_NET_CHANNEL_MOTION_DETECT), DMS_NET_GET_MOTIONCFG);
}


int Gk_CmsSetMd(int sock)
{
    DMS_NET_CHANNEL_MOTION_DETECT md;
    int ret = GkSockRecv(sock, (char *)&md, sizeof(DMS_NET_CHANNEL_MOTION_DETECT));
    if (ret != sizeof(DMS_NET_CHANNEL_MOTION_DETECT)) {
        PRINT_ERR();
        return -1;
    }

    #if 0
    printf("set: \n");
    printf("flag:%u\n", md.stHandle.wActionFlag);
    printf("wDuration:%u\n", md.stHandle.wDuration);
    printf("wRecTime:%u\n", md.stHandle.wRecTime);
    printf("wSnapNum:%u\n", md.stHandle.wSnapNum);
    printf("wDelayTime:%u\n", md.stHandle.wDelayTime);
    printf("wBuzzerTime:%u\n", md.stHandle.wBuzzerTime);
    #endif

    //todo
    if (md.dwSize != sizeof(DMS_NET_CHANNEL_MOTION_DETECT)) {
        PRINT_ERR("read error: struct size error.\n");
        return -1;
    }
    runMdCfg.enable = md.bEnable;
    runMdCfg.sensitive = md.dwSensitive;
    netcam_md_set_sensitivity_percentage(0, runMdCfg.sensitive);

    //runMdCfg.schedule_mode = md.bManualDefence;
    //printf("snesitive:%d bManualDefence:%d\n", runMdCfg.sensitive, runMdCfg.schedule_mode);
    runMdCfg.handle.intervalTime = md.stHandle.wDelayTime;

    if ((md.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOREC) == GK_ALARM_EXCEPTION_TOREC) {
        runMdCfg.handle.is_rec = 1;
        //runMdCfg.handle.recTime = md.stHandle.wRecTime;
        runMdCfg.handle.recStreamNo = 1;
    } else {
        runMdCfg.handle.is_rec = 0;
    }

	//set the snap flag
	if ((md.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOSNAP) ||
		(md.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOFTP) ||
		(md.stHandle.stPtzLink[0].byType == 1) ||
		(md.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOBEEP) ||
		(md.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOEMAIL))
		{runMdCfg.handle.is_snap = 1;}
	else
		{runMdCfg.handle.is_snap = 0;}
		

    if ((md.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOSNAP) == GK_ALARM_EXCEPTION_TOSNAP) {
        runMdCfg.handle.isSnapSaveToSd = 1;
        //runMdCfg.handle.snapNum = md.stHandle.wSnapNum;
    } else {
        runMdCfg.handle.isSnapSaveToSd = 0;
    }

	if ((md.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOFTP) == GK_ALARM_EXCEPTION_TOFTP) {
		runMdCfg.handle.isSnapUploadToFtp = 1;
		runNetworkCfg.ftp.enableFTP = 1;
    } else {
		runMdCfg.handle.isSnapUploadToFtp = 0;
		runNetworkCfg.ftp.enableFTP = 0;
    }

    if ((md.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOALARMOUT) == GK_ALARM_EXCEPTION_TOALARMOUT) {
        runMdCfg.handle.is_alarmout = 1;
        runMdCfg.handle.duration = md.stHandle.wDuration;
    } else {
        runMdCfg.handle.is_alarmout = 0;
    }

    if (md.stHandle.stPtzLink[0].byType == 1 && 
		md.stHandle.stPtzLink[0].byValue > 0 &&
		md.stHandle.stPtzLink[0].byValue < 256) {
		memset(&runMdCfg.handle.ptzLink, 0, sizeof(runMdCfg.handle.ptzLink));
		runMdCfg.handle.ptzLink.type = md.stHandle.stPtzLink[0].byType;
		runMdCfg.handle.ptzLink.value = md.stHandle.stPtzLink[0].byValue - 1;
        runMdCfg.handle.is_ptz = 1; 
    } else {
        runMdCfg.handle.is_ptz = 0;
    }

    if ((md.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOBEEP) == GK_ALARM_EXCEPTION_TOBEEP) {
        runMdCfg.handle.is_beep = 1;
        runMdCfg.handle.beepTime = md.stHandle.wBuzzerTime;
    } else {
        runMdCfg.handle.is_beep = 0;
    }

    if ((md.stHandle.wActionFlag & GK_ALARM_EXCEPTION_TOEMAIL) == GK_ALARM_EXCEPTION_TOEMAIL) {
        runMdCfg.handle.is_email = 1;
		runNetworkCfg.email.enableEmail = 1;
    } else {
        runMdCfg.handle.is_email = 0;
		runNetworkCfg.email.enableEmail = 0;
    }

    #if 0
    int k;
    for(k = 0 ; k < DMS_MD_STRIDE_SIZE; k ++) {
        printf("%d:%u ", k, md.byMotionArea[k]);
    }
    printf("\n");
    #endif

    int i, j;
    for(i = 0 ; i < 7; i ++) {
        for(j = 0 ; j < 4; j ++) {
            #if 0
            PRINT_INFO("set day1 [%d]-index[%d] time:[%d:%d] - [%d:%d]\n", i, j, 
                    runMdCfg.scheduleTime[i][j].startHour, runMdCfg.scheduleTime[i][j].startMin,
                    runMdCfg.scheduleTime[i][j].stopHour, runMdCfg.scheduleTime[i][j].stopMin);
            #endif

            runMdCfg.scheduleTime[i][j].startHour = md.stScheduleTime[i][j].byStartHour;
            runMdCfg.scheduleTime[i][j].startMin = md.stScheduleTime[i][j].byStartMin;
            runMdCfg.scheduleTime[i][j].stopHour = md.stScheduleTime[i][j].byStopHour;
            runMdCfg.scheduleTime[i][j].stopMin = md.stScheduleTime[i][j].byStopMin;

            #if 0
            PRINT_INFO("set day2 [%d]-index[%d] time:[%d:%d] - [%d:%d]\n", i, j, 
                    runMdCfg.scheduleTime[i][j].startHour, runMdCfg.scheduleTime[i][j].startMin,
                    runMdCfg.scheduleTime[i][j].stopHour, runMdCfg.scheduleTime[i][j].stopMin);
            #endif

        }
    }

    return 0;
}


int Gk_CmsGetVlost(int sock)
{
    DMS_NET_CHANNEL_VILOST vlost;

    //to do
    memset(&vlost, 0, sizeof(DMS_NET_CHANNEL_VILOST));
    vlost.dwSize = sizeof(DMS_NET_CHANNEL_VILOST);

    return GkCmsCmdResq(sock, &vlost, sizeof(DMS_NET_CHANNEL_VILOST), DMS_NET_GET_VLOSTCFG);
}

int Gk_CmsSetVlost(int sock)
{
    DMS_NET_CHANNEL_VILOST vlost;
    int ret = GkSockRecv(sock, (char *)&vlost, sizeof(DMS_NET_CHANNEL_VILOST));
    if (ret != sizeof(DMS_NET_CHANNEL_VILOST)) {
        PRINT_ERR();
        return -1;
    }

    //todo

    return 0;
}


int Gk_CmsGetHideAlarm(int sock)
{
    DMS_NET_CHANNEL_HIDEALARM hide_alarm;

    //to do
    memset(&hide_alarm, 0, sizeof(DMS_NET_CHANNEL_HIDEALARM));
    hide_alarm.dwSize = sizeof(DMS_NET_CHANNEL_HIDEALARM);

    return GkCmsCmdResq(sock, &hide_alarm, sizeof(DMS_NET_CHANNEL_HIDEALARM), DMS_NET_GET_HIDEALARMCFG);
}

int Gk_CmsSetHideAlarm(int sock)
{
    DMS_NET_CHANNEL_HIDEALARM hide_alarm;
    int ret = GkSockRecv(sock, (char *)&hide_alarm, sizeof(DMS_NET_CHANNEL_HIDEALARM));
    if (ret != sizeof(DMS_NET_CHANNEL_HIDEALARM)) {
        PRINT_ERR();
        return -1;
    }

    //todo

    return 0;
}
