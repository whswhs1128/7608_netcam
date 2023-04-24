#ifdef MODULE_SUPPORT_LOCAL_REC

#include "gk_cms_common.h"
#include "gk_cms_media.h"
#include "gk_cms_sock.h"
#include "gk_cms_protocol.h"
#include "gk_cms_utility.h"
#include "gk_cms_net_api.h"
#include "cfg_all.h"
#include "netcam_api.h"
#include "work_queue.h"
#include "sdk_sys.h"
#include "mmc_api.h"
#include "ftplib.h"

int Gk_CmsGetSnap(int sock)
{
    DMS_NET_SNAP_TIMER snap;

    //to do
    memset(&snap, 0, sizeof(DMS_NET_SNAP_TIMER));
    snap.bEnable = runSnapCfg.timer_snap.enable;
	snap.byStoragerMode = runSnapCfg.timer_snap.storagerMode;
	snap.dwInterval = runSnapCfg.timer_snap.interval;
	snap.dwSize = sizeof(DMS_NET_SNAP_TIMER);
	snap.nImageSize = runSnapCfg.timer_snap.imageSize;
	snap.nPictureQuality = runSnapCfg.timer_snap.pictureQuality;

    int i, j;
    for(i = 0 ; i < 7; i ++) {
        for(j = 0 ; j < 4; j ++) {
            snap.stScheduleTime[i][j].byStartHour = runSnapCfg.timer_snap.scheduleTime[i][j].startHour;
            snap.stScheduleTime[i][j].byStartMin = runSnapCfg.timer_snap.scheduleTime[i][j].startMin;
            snap.stScheduleTime[i][j].byStopHour = runSnapCfg.timer_snap.scheduleTime[i][j].stopHour;
            snap.stScheduleTime[i][j].byStopMin = runSnapCfg.timer_snap.scheduleTime[i][j].stopMin;
        }
    }
	snap.wChannel = runSnapCfg.timer_snap.channelID;

    GkCmsCmdResq(sock, &snap, sizeof(DMS_NET_SNAP_TIMER), DMS_NET_GET_SNAPTIMERCFG);
    return 0;
}

int Gk_CmsGetSnapEvent(int sock)
{
    DMS_NET_SNAP_EVENT snap_event;

    //to do
    memset(&snap_event, 0, sizeof(DMS_NET_SNAP_EVENT));
    snap_event.bEnable = runSnapCfg.event_snap.enable;
	snap_event.dwInterval = runSnapCfg.event_snap.interval;
	snap_event.dwSize = sizeof(DMS_NET_SNAP_EVENT);
	snap_event.nImageSize = runSnapCfg.event_snap.imageSize;
	snap_event.nPictureQuality = runSnapCfg.event_snap.pictureQuality;
	snap_event.byPicNum = runSnapCfg.event_snap.nums;

    GkCmsCmdResq(sock, &snap_event, sizeof(DMS_NET_SNAP_EVENT), DMS_NET_GET_SNAPEVENTCFG);
    return 0;
}

static int GkTimeSnapSaveLocal(void *param)
{
	int ret;
	struct tm tt;
	char snap_path[256] = {0};
	char snap_filename[256] = {0};
	char cmd_line[256] = {0};

	if (!runSnapCfg.timer_snap.enable)
		return 0;

	if (!grd_sd_is_mount())
		return 0;

	if (grd_sd_get_free_size() < 100)
	{
		sprintf(cmd_line, "rm -rf %s/snapshot/", GRD_SD_MOUNT_POINT);
		delete_path(cmd_line);

		if (grd_sd_get_free_size() < 100)
			return 0; //no space so exit
	}

    sys_get_tm(&tt);
    sprintf(snap_path, "%s/snapshot/%04d%02d%02d",
		GRD_SD_MOUNT_POINT, tt.tm_year, tt.tm_mon, tt.tm_mday);

	sprintf(cmd_line, "mkdir -p %s", snap_path);
	new_system_call(cmd_line);

	sprintf(snap_filename, "/tmp/TimeSnap_%04d%02d%02d_%02d%02d%02d.jpg",
		tt.tm_year, tt.tm_mon, tt.tm_mday,
		tt.tm_hour, tt.tm_min, tt.tm_sec);

	ret = netcam_video_snapshot(runVideoCfg.vencStream[0].h264Conf.width,
		runVideoCfg.vencStream[0].h264Conf.height, snap_filename,
		runSnapCfg.timer_snap.pictureQuality);
    if (ret != 0)
	{
        PRINT_ERR("timer snapshot failed\n");
        return -1;
    }

	sprintf(cmd_line, "cp %s %s", snap_filename, snap_path);
	new_system_call(cmd_line);

    delete_path(snap_filename);

	return 1;
}

#define GK_TIMESNAP_FTP_NAME "/tmp/timesnap.jpg"
#ifdef MODULE_SUPPORT_FTP

static int GkTimeSnapSaveFtp(void *param)
{
	int ret;
	netbuf *conn = (netbuf*)param;
	char snap_path[256] = {0};
	char snap_filename[256] = {0};
	struct tm tt;

	if (!runSnapCfg.timer_snap.enable)
	{
		FtpClose(conn);
		return 0;
	}

	if (!conn)
		return 0;

	sys_get_tm(&tt);
    sprintf(snap_path, "%04d%02d%02d", tt.tm_year, tt.tm_mon, tt.tm_mday);
	FtpMkdir(snap_path, conn);
	FtpChdir(snap_path, conn);
	FtpOptions(FTPLIB_CONNMODE,FTPLIB_PORT,conn);

	sprintf(snap_filename, "TimeSnap_%04d%02d%02d_%02d%02d%02d.jpg",
		tt.tm_year, tt.tm_mon, tt.tm_mday,
		tt.tm_hour, tt.tm_min, tt.tm_sec);

	ret = netcam_video_snapshot(runVideoCfg.vencStream[0].h264Conf.width,
		runVideoCfg.vencStream[0].h264Conf.height, GK_TIMESNAP_FTP_NAME,
		runSnapCfg.timer_snap.pictureQuality);
    if (ret != 0)
	{
        PRINT_ERR("timer snapshot failed\n");
        return -1;
    }

	ret = FtpPut(GK_TIMESNAP_FTP_NAME,snap_filename,FTPLIB_IMAGE,conn);
	if(ret)
	{
		PRINT_INFO("ftp upload \"%s\" is susseccful\r\n", snap_filename);
	}
	else
	{
		PRINT_ERR("ftp update  failed\r\n");
	}

	FtpCDUp(conn);
	delete_path(GK_TIMESNAP_FTP_NAME);
	return 1;
}

static netbuf* GkFtpInit()
{
	char ipaddress[64]={0};
	char snap_path[256] = {0};
	netbuf *conn = NULL;
	snprintf(ipaddress, sizeof(ipaddress), "%s:%d",
		runNetworkCfg.ftp.address, runNetworkCfg.ftp.port);

	FtpInit();
	if (FtpConnect(ipaddress, &conn) == 0)
	{
		PRINT_ERR("Fail to connect ftp\n");
		return 0;
	}

	if (FtpLogin(runNetworkCfg.ftp.userName, runNetworkCfg.ftp.password, conn))
	{
		PRINT_INFO("ftp login is ok \r\n");
	}
	else
	{
		PRINT_ERR("ftp login is error \r\n");
		FtpClose(conn);
		return 0;
	}

	sprintf(snap_path, "TimerSnapshot");
	FtpMkdir(snap_path, conn);
	FtpChdir(snap_path, conn);
	return conn;
}
#endif
int GkInitTimeSnap(void* param)
{
	int interval;
	if (!runSnapCfg.timer_snap.enable)
		return 0;

	interval = runSnapCfg.timer_snap.interval;
	interval /= 1000;
	interval = interval ? interval : 1;  //Ä¬ÈÏ1s

	if (runSnapCfg.timer_snap.storagerMode == 0)
	{
		//local
		CRTSCH_DEFAULT_WORK(CONDITION_TIMER_WORK, COMPUTE_TIME(0,0,0,interval,0), GkTimeSnapSaveLocal);
	}
	else if (runSnapCfg.timer_snap.storagerMode == 1)
	{
		//ftp
		#ifdef MODULE_SUPPORT_FTP
		netbuf *conn = GkFtpInit();
		if (!conn) return -1;

		CRTSCH_DEFAULT_WORK(CONDITION_TIMER_WORK, COMPUTE_TIME(0,0,0,interval,0), GkTimeSnapSaveFtp);
        #endif
    }
	else
	{
		//ÓÅÏÈftp
#ifdef MODULE_SUPPORT_FTP
		netbuf *conn = GkFtpInit();
		if (conn)
		{
			CRTSCH_DEFAULT_WORK(CONDITION_TIMER_WORK, COMPUTE_TIME(0,0,0,interval,0), GkTimeSnapSaveFtp);
		}
		else
#else
		{
			CRTSCH_DEFAULT_WORK(CONDITION_TIMER_WORK, COMPUTE_TIME(0,0,0,interval,0), GkTimeSnapSaveLocal);
		}
#endif
	}

	return 0;
}


int Gk_CmsSetSnap(int sock)
{
    DMS_NET_SNAP_TIMER snap;
    int ret = GkSockRecv(sock, (char *)&snap, sizeof(DMS_NET_SNAP_TIMER));
    if (ret != sizeof(DMS_NET_SNAP_TIMER)) {
        PRINT_ERR();
        return -1;
    }

    //todo
    runSnapCfg.timer_snap.enable = snap.bEnable;
	runSnapCfg.timer_snap.storagerMode = snap.byStoragerMode;
	runSnapCfg.timer_snap.interval = snap.dwInterval;
	runSnapCfg.timer_snap.imageSize = snap.nImageSize;
	runSnapCfg.timer_snap.pictureQuality = snap.nPictureQuality;

    printf("enable:%d storagerMode:%d interval:%lu imageSize:%d pictureQuality:%d channel:%u\n",
           snap.bEnable, snap.byStoragerMode, snap.dwInterval, snap.nImageSize, snap.nPictureQuality, snap.wChannel);

    int i, j;
    for(i = 0 ; i < 7; i ++) {
        for(j = 0 ; j < 4; j ++) {
            runSnapCfg.timer_snap.scheduleTime[i][j].startHour = snap.stScheduleTime[i][j].byStartHour;
            runSnapCfg.timer_snap.scheduleTime[i][j].startMin = snap.stScheduleTime[i][j].byStartMin;
            runSnapCfg.timer_snap.scheduleTime[i][j].stopHour = snap.stScheduleTime[i][j].byStopHour;
            runSnapCfg.timer_snap.scheduleTime[i][j].stopMin = snap.stScheduleTime[i][j].byStopMin;
            printf("i:%d j:%d %d:%d-%d:%d\n", i, j, snap.stScheduleTime[i][j].byStartHour, snap.stScheduleTime[i][j].byStartMin,
                   snap.stScheduleTime[i][j].byStopHour, snap.stScheduleTime[i][j].byStopMin);
        }
    }

	SnapCfgSave();
	GkInitTimeSnap(NULL);

    return 0;
}

int Gk_CmsSetSnapEvent(int sock)
{
    DMS_NET_SNAP_EVENT snap_event;
    int ret = GkSockRecv(sock, (char *)&snap_event, sizeof(DMS_NET_SNAP_EVENT));
    if (ret != sizeof(DMS_NET_SNAP_EVENT)) {
        PRINT_ERR();
        return -1;
    }

    //todo
    runSnapCfg.event_snap.enable = snap_event.bEnable;
	runSnapCfg.event_snap.interval = snap_event.dwInterval;
	runSnapCfg.event_snap.imageSize = snap_event.nImageSize;
	runSnapCfg.event_snap.pictureQuality = snap_event.nPictureQuality;
	runSnapCfg.event_snap.nums = snap_event.byPicNum;

	SnapCfgSave();
    return 0;
}

//static void * GkSnapThread(void *param)
static int GkSnapWork(void *param)
{
    int sock = (int)param;

    struct tm tt;
    sys_get_tm(&tt);
    char snap_path[64] = {0};
    sprintf(snap_path, "/tmp/cmd_%04d%02d%02d_%02d%02d%02d.jpg", tt.tm_year, tt.tm_mon, tt.tm_mday, tt.tm_hour, tt.tm_min, tt.tm_sec);

	int ret = netcam_video_snapshot(runVideoCfg.vencStream[0].h264Conf.width, runVideoCfg.vencStream[0].h264Conf.height, snap_path, GK_ENC_SNAPSHOT_QUALITY_MEDIUM);
    if (ret != 0) {
        PRINT_ERR("snapshot failed\n");
        return -1;
    }

   	FILE *fp = fopen(snap_path, "rb");
	if(fp == NULL) {
        PRINT_ERR("fopen %s failed\n", snap_path);
        return -1;
    }
	fseek(fp, 0, SEEK_END);
	int len = ftell(fp);
	fseek(fp, 0, SEEK_SET);
	char *data= (char*)malloc(len);
    if (data == NULL) {
        PRINT_ERR("malloc %d failed\n", len);
        fclose(fp);
        return -1;
    }
	ret = fread(data, 1, len, fp);
    if (ret != len) {
        PRINT_ERR("malloc %d failed\n", len);
        fclose(fp);
        free(data);
        return -1;
    }
	fclose(fp);

    delete_path(snap_path);
    Gk_CmsNotify(sock, JB_MSG_JPEGSNAP, data, len);

    free(data);
    return 0;
}

int Gk_CmsDoSnapshot(int sock)
{
    //CreateDetachThread(GkSnapThread, (void *)sock, NULL);
	CRTSCH_DEFAULT_WORK1(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,1,0), GkSnapWork, (void *)sock);
    return 0;
}

#endif

