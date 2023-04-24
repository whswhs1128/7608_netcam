#include "bellcall.h"
#include "netcam_api.h"
#include "ntp_client.h"
#include "parser_cfg.h"
#include "flash_ctl.h"
#include "work_queue.h"
#include "mmc_api.h"
#include "avi_search_api.h"
#include "avi_utility.h"

int PPCgiModuleInit()
{
	return 0;
}

int PPCgiSetAlias(const char * Alias)
{
	strncpy(runSystemCfg.deviceInfo.deviceName, Alias, MAX_SYSTEM_STR_SIZE);
	CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,1,0), (WORK_CALLBACK)SystemCfgSave);
	return 0;
}

int PPCgiGetAlias(char * Alias,int Lens)
{
	strncpy(Alias, runSystemCfg.deviceInfo.deviceName, Lens);
	return 0;
}

int PPCgiSetAudio(PPCGI_AUDIO_T * AudioParamSet)
{
/*
	runAudioCfg.sampleBitWidth = AudioParamSet->BitRate / AudioParamSet->SampleRate;
	runAudioCfg.type = AudioParamSet->EncodeType;
	runAudioCfg.mode = AudioParamSet->Mute;
	runAudioCfg.sampleRate = AudioParamSet->SampleRate;

	CREATE_WORK(SetAudio, EVENT_TIMER_WORK, (WORK_CALLBACK)AudioCfgSave);
	INIT_WORK(SetAudio, 20, NULL);
	SCHEDULE_DEFAULT_WORK(SetAudio);*/
	return 0;
}

int PPCgiGetAudio(PPCGI_AUDIO_T * AudioParamGet)
{
/*
	if (AudioParamGet->Range == 0)
	{
		AudioParamGet->BitRate = 8;
		AudioParamGet->EncodeType = 0;
		AudioParamGet->Mute = 0;
		AudioParamGet->SampleRate = 8000;
	}
	else if (AudioParamGet->Range == 1)
	{
		AudioParamGet->BitRate = 32;
		AudioParamGet->EncodeType = 2;
		AudioParamGet->Mute = 0;
		AudioParamGet->SampleRate = 32000;
	}
	else
	{
		AudioParamGet->BitRate = runAudioCfg.sampleBitWidth;
		AudioParamGet->EncodeType = runAudioCfg.type;
		AudioParamGet->Mute = runAudioCfg.mode;
		AudioParamGet->SampleRate = runAudioCfg.sampleRate;
	}*/
	return 0;
}

int PPCgiSetAudioDefault()
{
	/*
	AudioCfgLoadDefValue();
	CREATE_WORK(SetAudioDefault, EVENT_TIMER_WORK, (WORK_CALLBACK)AudioCfgSave);
	INIT_WORK(SetAudioDefault, 20, NULL);
	SCHEDULE_DEFAULT_WORK(SetAudioDefault);
	*/
	return 0;
}

int PPCgiSetImage(PPCGI_IMAGE_T * ImageParamSet)
{
	int ret;
	GK_NET_IMAGE_CFG stImagingConfig;

    memset(&stImagingConfig, 0, sizeof(GK_NET_IMAGE_CFG));
    ret = netcam_image_get(&stImagingConfig);
    if (ret != 0) {
        PRINT_ERR("netcam_image_get failed. ret=%d", ret);
        return -1;
    }

	stImagingConfig.brightness = VALUE_CONVERT(ImageParamSet->Brightness, 0, 100);
	stImagingConfig.saturation = VALUE_CONVERT(ImageParamSet->Saturation, 0, 100);
	stImagingConfig.contrast = VALUE_CONVERT(ImageParamSet->Contrast, 0, 100);
	stImagingConfig.sharpness = VALUE_CONVERT(ImageParamSet->Sharpness, 0, 100);
	stImagingConfig.hue = VALUE_CONVERT(ImageParamSet->Hue, 0, 100);

	stImagingConfig.antiFlickerFreq = VALUE_CONVERT(ImageParamSet->FilckFrequency, 50, 60);
	stImagingConfig.irCutMode = ImageParamSet->IRCut;
	stImagingConfig.flipEnabled = TRUE_FALSE_CONVERT(ImageParamSet->FlipHor, 0, 1);
	stImagingConfig.mirrorEnabled = TRUE_FALSE_CONVERT(ImageParamSet->MirrorVer, 0, 1);

	ret = netcam_image_set(stImagingConfig);
    if (ret != 0) {
        PRINT_ERR("netcam_image_set failed. ret=%d", ret);
        return -1;
    }

	CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,2,0), (WORK_CALLBACK)ImageCfgSave);
	return ret;
}

int PPCgiGetImage(PPCGI_IMAGE_T * ImageParamGet)
{
	if (ImageParamGet->Range == 0)
	{
		ImageParamGet->Brightness = 0;
		ImageParamGet->Saturation = 0;
		ImageParamGet->Contrast = 0;
		ImageParamGet->Sharpness = 0;
		ImageParamGet->Hue = 0;

		ImageParamGet->FilckFrequency = 50;
		ImageParamGet->FlipHor = 0;
		ImageParamGet->IRCut = 0;
		ImageParamGet->MirrorVer = 0;
	}
	else if (ImageParamGet->Range == 1)
	{
		ImageParamGet->Brightness = 100;
		ImageParamGet->Saturation = 100;
		ImageParamGet->Contrast = 100;
		ImageParamGet->Sharpness = 100;
		ImageParamGet->Hue = 100;

		ImageParamGet->FilckFrequency = 60;
		ImageParamGet->FlipHor = 1;
		ImageParamGet->IRCut = 2;
		ImageParamGet->MirrorVer = 1;
	}
	else
	{
		ImageParamGet->Brightness = runImageCfg.brightness;
		ImageParamGet->Saturation = runImageCfg.saturation;
		ImageParamGet->Contrast = runImageCfg.contrast;
		ImageParamGet->Sharpness = runImageCfg.sharpness;
		ImageParamGet->Hue = runImageCfg.hue;

		ImageParamGet->FilckFrequency = runImageCfg.antiFlickerFreq;
		ImageParamGet->FlipHor = runImageCfg.flipEnabled;
		ImageParamGet->IRCut = runImageCfg.irCutMode;
		ImageParamGet->MirrorVer = runImageCfg.mirrorEnabled;
	}

	return 0;
}

int PPCgiSetImageDefault()
{
	netcam_image_reset_default();
	CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,2,0), (WORK_CALLBACK)ImageCfgSave);
	return 0;
}

int PPCgiSetVideo(PPCGI_VIDEO_T * VideoParamSet)
{
	int StreamID;
	ST_GK_ENC_STREAM_H264_ATTR h264Attr;

	if (VideoParamSet->StreamID < 0 || VideoParamSet->StreamID > 2)
	{
		PRINT_ERR("VideoParamSet->StreamID = %d error\n", VideoParamSet->StreamID);
		return -1;
	}

	//printf("------------------->set frame<------------------------\n");
	//printf("------------------->StreamID: %d<---------------------\n", VideoParamSet->StreamID);
	//printf("------------------->BitRate:  %d<---------------------\n", VideoParamSet->BitRate);
	//printf("------------------->FPS:      %d<---------------------\n", VideoParamSet->FPS);
	//printf("------------------->set frame<-----------------------\n");
	StreamID = CHANNEL_ID_CONVERT(VideoParamSet->StreamID);
	//printf("------------------->StreamID: %d<---------------------\n", StreamID);

	netcam_video_get(0, StreamID, &h264Attr);
	h264Attr.bps = VideoParamSet->BitRate;
	h264Attr.fps = VideoParamSet->FPS;
	netcam_video_set(0, StreamID, &h264Attr);

	//CREATE_WORK(SetVideo, EVENT_TIMER_WORK, (WORK_CALLBACK)VideoCfgSave);
	//INIT_WORK(SetVideo, COMPUTE_TIME(0,0,0,2,0), NULL);
	//SCHEDULE_DEFAULT_WORK(SetVideo);
	CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0, 0, 0, 2, 0), (WORK_CALLBACK)VideoCfgSave);
	return 0;
}

int PPCgiGetVideo(PPCGI_VIDEO_T * VideoParamGet)
{
	int StreamID;
	ST_GK_ENC_STREAM_H264_ATTR h264Attr;

	if (VideoParamGet->StreamID < 0 || VideoParamGet->StreamID > 2)
	{
		PRINT_ERR("VideoParamSet->StreamID = %d error\n", VideoParamGet->StreamID);
		return -1;
	}

	StreamID = CHANNEL_ID_CONVERT(VideoParamGet->StreamID);
	if (VideoParamGet->Range == 0)
	{
		VideoParamGet->BitRate = 1;
		VideoParamGet->FPS = 1;
	}
	else if (VideoParamGet->Range == 1)
	{
		VideoParamGet->BitRate = 8000;
		VideoParamGet->FPS = 25;
	}
	else
	{
		netcam_video_get(0, StreamID, &h264Attr);
		VideoParamGet->BitRate = h264Attr.bps;
		VideoParamGet->FPS = h264Attr.fps;
	}
	return 0;
}

int PPCgiSetVideoDefault(int StreamID)
{
	ST_GK_ENC_STREAM_H264_ATTR h264Attr;

	if (StreamID < 0 || StreamID > 2)
	{
		PRINT_ERR("StreamID = %d error\n", StreamID);
		return -1;
	}

	StreamID = CHANNEL_ID_CONVERT(StreamID);
	netcam_video_get(0, StreamID, &h264Attr);
	h264Attr.bps = runVideoCfg.vencStream[StreamID].h264Conf.bps;//maxbps
	h264Attr.fps = 25;
	netcam_video_set(0, StreamID, &h264Attr);

	CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,2,0), (WORK_CALLBACK)VideoCfgSave);
	return 0;
}

int PPCgiSetOSD(PPCGI_OSD_T * OSDParamSet)
{
	return 0;//++
}

int PPCgiGetOSD(PPCGI_OSD_T * OSDParamGet)
{
	return 0;//++
}

int PPCgiSetRecord(PPCGI_RECORD_T * RecordParamSet)
{
	int i, j;
	printf("======================PPCgiSetRecord=========================\n");
	runRecordCfg.recycleRecord = RecordParamSet->Overwrite;
	//runRecordCfg.enable = RecordParamSet->RecorderEnable;
	runRecordCfg.recordLen = RecordParamSet->RecorderTime;
	runRecordCfg.audioRecEnable = RecordParamSet->RecorderEnableAudio;
	runRecordCfg.recordMode = TRUE_FALSE_CONVERT(RecordParamSet->RecorderEnable, 3, 0);

	printf("======RecordParamSet->RecorderEnable=%d,runRecordCfg.recordMode=%d\n",
		RecordParamSet->RecorderEnable,runRecordCfg.recordMode);

	for (i = 0; i < 7; i++)
	{
		for (j = 0; j < 3; j++)
		{
			runRecordCfg.scheduleSlice[i][j] = RecordParamSet->RecorderSchedules[i][j];
			printf("RecordParamSet===Schedules:%d\n", RecordParamSet->RecorderSchedules[i][j]);
		}
	}

	CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,2,0), (WORK_CALLBACK)RecordCfgSave);
	return 0;
}

int PPCgiGetRecord(PPCGI_RECORD_T * RecordParamGet)
{
	int i, j;
	printf("======================PPCgiGetAlarm=========================\n");
	RecordParamGet->Overwrite = runRecordCfg.recycleRecord;
	//RecordParamGet->RecorderEnable = runRecordCfg.enable;
	RecordParamGet->RecorderSize = 0;
	RecordParamGet->RecorderTime = runRecordCfg.recordLen;
	RecordParamGet->RecorderEnableAudio = runRecordCfg.audioRecEnable;

	if (runRecordCfg.recordMode == 3)
		RecordParamGet->RecorderEnable = 0;
	else if (runRecordCfg.recordMode == 0)
		RecordParamGet->RecorderEnable = 1;
	else
		RecordParamGet->RecorderEnable = 0;

	for (i = 0; i < 7; i++)
	{
		for (j = 0; j < 3; j++)
		{
			RecordParamGet->RecorderSchedules[i][j] = runRecordCfg.scheduleSlice[i][j];
			printf("RecordParamGet===Schedules:%d\n", RecordParamGet->RecorderSchedules[i][j]);
		}
	}

	return 0;
}


int PPCgiSetTime(PPCGI_TIME_T * TimeParamSet)
{
	//ntp_param ntp;

	netcam_sys_set_time_zone_by_utc_second
		(TimeParamSet->TimeUTCs, 0 - TimeParamSet->TimeZone / 60);

	//memset(&ntp, 0, sizeof(ntp_param));
	//read_ntp_param(&ntp);

	//ntp.flag = TRUE_FALSE_CONVERT(TimeParamSet->NTPEnable, NTP_OFF, NTP_AUTO_SYNC);
	//strncpy(ntp.server, TimeParamSet->NTPServer, strlen(TimeParamSet->NTPServer));
	//ntp.is_overlap = 1;

	//save_ntp_param(&ntp);
	netcam_sys_ntp_start_stop(TimeParamSet->NTPEnable);
	ntpc_set_addr(TimeParamSet->NTPServer, sizeof(TimeParamSet->NTPServer), -1);
	return 0;
}

int PPCgiGetTime(PPCGI_TIME_T * TimeParamGet)
{
	//ntp_param ntp;
	struct tm tt;
	time_t t;

	t = time(NULL);
	gmtime_r(&t, &tt);

	TimeParamGet->TimeUTCs = mktime(&tt);
	TimeParamGet->TimeZone = 0 - runSystemCfg.timezoneCfg.timezone * 60;

	//memset(&ntp, 0, sizeof(ntp_param));
	//read_ntp_param(&ntp);
	TimeParamGet->NTPEnable = runSystemCfg.ntpCfg.enable;
	strncpy(TimeParamGet->NTPServer, runSystemCfg.ntpCfg.serverDomain, sizeof(TimeParamGet->NTPServer));

	return 0;
}

int PPCgiGetAlarm(PPCGI_ALARM_T * AlarmParamGet)
{
	printf("======================PPCgiGetAlarm=========================\n");

    AlarmParamGet->MotionEnable = runMdCfg.enable;
    AlarmParamGet->MotionSensitivity = runMdCfg.sensitive;
	AlarmParamGet->AlarmByMail = runMdCfg.handle.is_email;
	AlarmParamGet->AlarmSnapshotEnable = runMdCfg.handle.isSnapUploadToFtp;
	AlarmParamGet->UploadInterval = runMdCfg.handle.intervalTime;
	AlarmParamGet->AlarmRecorderEnable = runMdCfg.handle.is_rec;

	AlarmParamGet->InputEnable = runAlarmCfg.cfg.gpio_enable;
	AlarmParamGet->ILevel = runAlarmCfg.cfg.in_level;
	AlarmParamGet->OLevel = runAlarmCfg.cfg.out_level;
	AlarmParamGet->IOLinkage = runAlarmCfg.cfg.io_linkage;
	//AlarmParamGet->UploadInterval = runAlarmCfg.cfg.upload_interval;
	//AlarmParamGet->AlarmByMail = runAlarmCfg.cfg.alarmbymail;
	AlarmParamGet->AlarmScheduleEnable = runAlarmCfg.cfg.schedule_enable;
	//AlarmParamGet->AlarmSnapshotEnable = runAlarmCfg.cfg.snapshot_enable;
	//AlarmParamGet->AlarmRecorderEnable = runAlarmCfg.cfg.recorder_enable;
	//AlarmParamGet->AlarmMovePerset = runAlarmCfg.cfg.move_perset;
	AlarmParamGet->AlarmMovePerset = runMdCfg.handle.ptzLink.value + 1;
	//AlarmParamGet->AlarmMute = runAlarmCfg.cfg.alarmMute;
	AlarmParamGet->AlarmMute = runMdCfg.handle.is_beep;

    int i, j;
    for (i = 0; i < 7; i ++) {
        for (j = 0; j < 3; j ++) {
            AlarmParamGet->AlarmSchedules[i][j] = (int)runMdCfg.scheduleSlice[i][j];
        }
    }

	return 0;
}

int PPCgiSetAlarm(PPCGI_ALARM_T * AlarmParamSet)
{
	printf("======================PPCgiSetAlarm=========================\n");
	//printf("=AlarmMute %d, UploadInterval: %d, AlarmRecorderEnable: %d\n",
	//	AlarmParamSet->AlarmMute, AlarmParamSet->UploadInterval,
	//	AlarmParamSet->AlarmRecorderEnable);

    runMdCfg.enable = AlarmParamSet->MotionEnable;
    runMdCfg.sensitive = AlarmParamSet->MotionSensitivity;
	runMdCfg.handle.is_email = AlarmParamSet->AlarmByMail;
	runMdCfg.handle.isSnapUploadToFtp = AlarmParamSet->AlarmSnapshotEnable;
	runMdCfg.handle.intervalTime = AlarmParamSet->UploadInterval;
	runMdCfg.handle.is_rec = AlarmParamSet->AlarmRecorderEnable;
	runMdCfg.handle.is_beep = AlarmParamSet->AlarmMute;
	runMdCfg.handle.is_snap = AlarmParamSet->AlarmByMail ||
		AlarmParamSet->AlarmSnapshotEnable || AlarmParamSet->AlarmMute;

	runAlarmCfg.cfg.gpio_enable = AlarmParamSet->InputEnable;
	runAlarmCfg.cfg.in_level = AlarmParamSet->ILevel;
	runAlarmCfg.cfg.out_level = AlarmParamSet->OLevel;
	runAlarmCfg.cfg.io_linkage = AlarmParamSet->IOLinkage;
	//runAlarmCfg.cfg.upload_interval = AlarmParamSet->UploadInterval;
	//runAlarmCfg.cfg.alarmbymail = AlarmParamSet->AlarmByMail;
	runAlarmCfg.cfg.schedule_enable = AlarmParamSet->AlarmScheduleEnable;
	//runAlarmCfg.cfg.snapshot_enable = AlarmParamSet->AlarmSnapshotEnable;
	//runAlarmCfg.cfg.recorder_enable = AlarmParamSet->AlarmRecorderEnable;
	if (AlarmParamSet->AlarmMovePerset > 0 && AlarmParamSet->AlarmMovePerset < 17)
	{
		runMdCfg.handle.is_ptz = 1;
		runMdCfg.handle.ptzLink.type = 1;
		runMdCfg.handle.ptzLink.value = AlarmParamSet->AlarmMovePerset - 1;
		//runAlarmCfg.cfg.move_perset = AlarmParamSet->AlarmMovePerset;
	}
	//runAlarmCfg.cfg.alarmMute = AlarmParamSet->AlarmMute;

	runNetworkCfg.ftp.enableFTP = AlarmParamSet->AlarmSnapshotEnable;
	runNetworkCfg.email.enableEmail = AlarmParamSet->AlarmByMail;

    int i, j;
    for (i = 0; i < 7; i ++) {
        for (j = 0; j < 3; j ++) {
            runMdCfg.scheduleSlice[i][j] = (unsigned int)AlarmParamSet->AlarmSchedules[i][j];
        }
    }

	CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,1,0), (WORK_CALLBACK)MdCfgSave);
	CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,1,0), (WORK_CALLBACK)AlarmCfgSave);
	CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,1,0), (WORK_CALLBACK)NetworkCfgSave);
	return 0;
}


int PPCgiGetMail(PPCGI_MAIL_T * MailParamGet)
{
	MailParamGet->EncryptType = runNetworkCfg.email.encryptionType;

	strncpy(MailParamGet->MailFrom,  runNetworkCfg.email.fromAddr,
		sizeof(MailParamGet->MailFrom));

	strncpy(MailParamGet->MailTos[0], runNetworkCfg.email.toAddrList0,
		sizeof(MailParamGet->MailTos[0]));

	strncpy(MailParamGet->MailTos[1], runNetworkCfg.email.toAddrList1,
		sizeof(MailParamGet->MailTos[1]));

	strncpy(MailParamGet->MailTos[2], runNetworkCfg.email.toAddrList2,
		sizeof(MailParamGet->MailTos[2]));

	strncpy(MailParamGet->MailTos[3], runNetworkCfg.email.toAddrList3,
		sizeof(MailParamGet->MailTos[3]));

	strncpy(MailParamGet->Pass, runNetworkCfg.email.eMailPass,
		sizeof(MailParamGet->Pass));

	MailParamGet->Port = runNetworkCfg.email.smtpPort;

	strncpy(MailParamGet->SMTPSvr, runNetworkCfg.email.smtpServer,
		sizeof(MailParamGet->SMTPSvr));

	strncpy(MailParamGet->User, runNetworkCfg.email.eMailUser,
		sizeof(MailParamGet->User));

	return 0;
}

int PPCgiSetMail(PPCGI_MAIL_T * MailParamSet)
{
	runNetworkCfg.email.encryptionType = MailParamSet->EncryptType;

	strncpy(runNetworkCfg.email.fromAddr, MailParamSet->MailFrom,
		sizeof(runNetworkCfg.email.fromAddr));

	strncpy(runNetworkCfg.email.toAddrList0, MailParamSet->MailTos[0],
		sizeof(runNetworkCfg.email.toAddrList0));

	strncpy(runNetworkCfg.email.toAddrList1, MailParamSet->MailTos[1],
		sizeof(runNetworkCfg.email.toAddrList1));

	strncpy(runNetworkCfg.email.toAddrList2, MailParamSet->MailTos[2],
		sizeof(runNetworkCfg.email.toAddrList2));

	strncpy(runNetworkCfg.email.toAddrList3, MailParamSet->MailTos[3],
		sizeof(runNetworkCfg.email.toAddrList3));

	strncpy(runNetworkCfg.email.eMailPass, MailParamSet->Pass,
		sizeof(runNetworkCfg.email.eMailPass));

	runNetworkCfg.email.smtpPort = MailParamSet->Port;

	strncpy(runNetworkCfg.email.smtpServer, MailParamSet->SMTPSvr,
		sizeof(runNetworkCfg.email.smtpServer));

	strncpy(runNetworkCfg.email.eMailUser, MailParamSet->User,
		sizeof(runNetworkCfg.email.eMailUser));

	//CREATE_WORK(SetMail, EVENT_TIMER_WORK, (WORK_CALLBACK)NetworkCfgSave);
	//INIT_WORK(SetMail, COMPUTE_TIME(0,0,0,1,0), NULL);
	//SCHEDULE_DEFAULT_WORK(SetMail);
	CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0, 0, 0, 1, 0), (WORK_CALLBACK)NetworkCfgSave);
	return 0;
}

int PPCgiGetNetwork(PPCGI_NETWORK_T * NetworkParamGet)
{
	if(runNetworkCfg.lan.enable)
	{
        strcpy(NetworkParamGet->Interface, "eth0");
		strcpy(NetworkParamGet->IPv4, runNetworkCfg.lan.ip);
    	strcpy(NetworkParamGet->Mask, runNetworkCfg.lan.netmask);
    	strcpy(NetworkParamGet->Gate, runNetworkCfg.lan.gateway);
 		NetworkParamGet->DHCP = runNetworkCfg.lan.dhcpIp;
		strcpy(NetworkParamGet->DNS1, runNetworkCfg.lan.dns1);
		strcpy(NetworkParamGet->DNS2, runNetworkCfg.lan.dns2);
		strcpy(NetworkParamGet->MAC, runNetworkCfg.lan.mac);
	}
    else
    {
        strcpy(NetworkParamGet->Interface, "ra0");
		strcpy(NetworkParamGet->IPv4, runNetworkCfg.wifi.ip);
    	strcpy(NetworkParamGet->Mask, runNetworkCfg.wifi.netmask);
    	strcpy(NetworkParamGet->Gate, runNetworkCfg.wifi.gateway);
 		NetworkParamGet->DHCP = runNetworkCfg.wifi.dhcpIp;
		strcpy(NetworkParamGet->DNS1, runNetworkCfg.wifi.dns1);
		strcpy(NetworkParamGet->DNS2, runNetworkCfg.wifi.dns2);
		strcpy(NetworkParamGet->MAC, runNetworkCfg.wifi.mac);
	}

	return 0;
}

int PPCgiSetNetwork(PPCGI_NETWORK_T * NetworkParamSet)
{
	ST_SDK_NETWORK_ATTR net_attr;
    int ret = 0;

    if(runNetworkCfg.lan.enable)
        sprintf(net_attr.name,"eth0");
    else
        sprintf(net_attr.name,"ra0");

	net_attr.enable = 1;
    netcam_net_get(&net_attr);
    if(NetworkParamSet->DHCP == 0)
    {
        strncpy(net_attr.ip,NetworkParamSet->IPv4,SDK_IPSTR_LEN);
        strncpy(net_attr.mask,NetworkParamSet->Mask,SDK_DNSSTR_LEN);
        strncpy(net_attr.gateway,NetworkParamSet->Gate,SDK_DNSSTR_LEN);
        strncpy(net_attr.dns1,NetworkParamSet->DNS1,SDK_DNSSTR_LEN);
        strncpy(net_attr.dns2,NetworkParamSet->DNS2,SDK_DNSSTR_LEN);
		net_attr.dhcp = 0;
    }
    else
    {
		net_attr.dhcp = 1;
    }

    ret = netcam_net_set( &net_attr);
    if(ret == 0)
    {
        netcam_net_cfg_save();
    }

	return ret;
}

int PPCgiSetWIFI(PPCGI_WIFI_T * WIFIParamSet)
{
	char *WifiName = NULL;

	printf("======================PPCgiSetWIFI=========================\n");
	//设置ssid等参数
	WIFI_LINK_INFO_t linkInfo;
	ST_SDK_NETWORK_ATTR net_attr;

	WifiName = netcam_net_wifi_get_devname();
	if(WifiName == NULL) {
		return -1;
	}

	netcam_net_wifi_on();

	strncpy(net_attr.name, WifiName, sizeof(net_attr.name));
	netcam_net_get(&net_attr);

	memset(&linkInfo,0,sizeof(WIFI_LINK_INFO_t));
	linkInfo.isConnect = 1;
	strcpy(linkInfo.linkEssid,WIFIParamSet->SSID);
	if(WIFIParamSet->Pass[0] != 0)
		strcpy(linkInfo.linkPsd,WIFIParamSet->Pass);
	linkInfo.linkScurity = WIFIParamSet->EncryptType;

	if(netcam_net_wifi_set_connect_info(&linkInfo) == WIFI_CONNECT_OK && linkInfo.linkStatus == WIFI_CONNECT_OK)
	{
		if(WIFIParamSet->Network.DHCP == 1)
		{
			net_attr.dhcp = 1;
		}
		else
		{
			net_attr.dhcp = 0;
			strcpy(net_attr.dns1,WIFIParamSet->Network.DNS1);
			strcpy(net_attr.dns2,WIFIParamSet->Network.DNS2);
			strcpy(net_attr.ip,WIFIParamSet->Network.IPv4);
			strcpy(net_attr.gateway,WIFIParamSet->Network.Gate);
			strcpy(net_attr.mask,WIFIParamSet->Network.Mask);

		}
		netcam_net_set(&net_attr);
	}

	CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,2,0), (WORK_CALLBACK)NetworkCfgSave);
	return 0;
}

int PPCgiGetWIFI(PPCGI_WIFI_T * WIFIParamGet)
{
	//返回ssid及网络信息
	WIFI_LINK_INFO_t linkInfo;

	printf("======================PPCgiGetWIFI=========================\n");
	PPCGI_NETWORK_T* NetworkParamGet = &WIFIParamGet->Network;
	char *WifiName = netcam_net_wifi_get_devname();
	if(WifiName == NULL)
	{
		return -1;
	}

	memset(WIFIParamGet,0,sizeof(PPCGI_WIFI_T));
	if(runNetworkCfg.wifi.enable && WifiName != NULL) {
        strncpy(NetworkParamGet->Interface, WifiName, sizeof(NetworkParamGet->Interface));
	} else
		return -1;

	if (netcam_net_wifi_get_connect_info(&linkInfo) != WIFI_CONNECT_OK)
	{
		PRINT_INFO("Fail to connect wifi\n");
		return -1;
	}

	strncpy(WIFIParamGet->SSID, linkInfo.linkEssid, sizeof(WIFIParamGet->SSID));
	strncpy(WIFIParamGet->Pass, linkInfo.linkPsd, sizeof(WIFIParamGet->Pass));
	WIFIParamGet->EncryptType = linkInfo.linkScurity;

	WIFIParamGet->Mode = linkInfo.mode;
	WIFIParamGet->Quality = 100;
	if(WifiName != NULL){
	    strncpy(NetworkParamGet->Interface, WifiName, sizeof(NetworkParamGet->Interface));
		strncpy(NetworkParamGet->IPv4, runNetworkCfg.wifi.ip, sizeof(NetworkParamGet->IPv4));
	    strncpy(NetworkParamGet->Mask, runNetworkCfg.wifi.netmask, sizeof(NetworkParamGet->Mask));
	    strncpy(NetworkParamGet->Gate, runNetworkCfg.wifi.gateway, sizeof(NetworkParamGet->Gate));
	 	NetworkParamGet->DHCP = runNetworkCfg.wifi.dhcpIp;
		strncpy(NetworkParamGet->DNS1, runNetworkCfg.wifi.dns1, sizeof(NetworkParamGet->DNS1));
		strncpy(NetworkParamGet->DNS2, runNetworkCfg.wifi.dns2, sizeof(NetworkParamGet->DNS2));
		strncpy(NetworkParamGet->MAC, runNetworkCfg.wifi.mac, sizeof(NetworkParamGet->MAC));
	}
	return 0;
}

int PPCgiLstWIFI(PPCGI_WIFI_SCAN_T * WIFIScanParamLst)
{
	char *WifiName = NULL;
	WIFI_SCAN_LIST_t list[20];
	int i, number = 20;

	printf("======================PPCgiLstWIFI=========================\n");
	if (!WIFIScanParamLst || !WIFIScanParamLst->Lists)
	{
		printf("WIFIScanParamLst param is null\n");
		return -1;
	}

	memset(list, 0, sizeof(list));
	WifiName = netcam_net_wifi_get_devname();
	if(WifiName == NULL)
	{
		printf("WifiName is NULL\n");
		return -1;
	}

	if(netcam_net_wifi_isOn() != 1)
	{
		printf("Wifi is not enable\n");
		return -1;
	}

	netcam_net_wifi_on();
	if(netcam_net_wifi_get_scan_list(list,&number) != 0)
	{
		printf("Fail to get wifi list\n");
		return -1;
	}

	WIFIScanParamLst->ListSize = number < WIFIScanParamLst->ListSize ?
		number : WIFIScanParamLst->ListSize;
	for(i = 0; i < WIFIScanParamLst->ListSize && list[i].vaild; i++)
	{
		WIFIScanParamLst->Lists[i].EncryptType = list[i].security;
		strncpy(WIFIScanParamLst->Lists[i].SSID, list[i].essid,
			sizeof(WIFIScanParamLst->Lists[i].SSID));
		strncpy(WIFIScanParamLst->Lists[i].MAC, list[i].mac,
			sizeof(WIFIScanParamLst->Lists[i].MAC));
		WIFIScanParamLst->Lists[i].Quality = VALUE_CONVERT(list[i].quality, 0, 100);
		WIFIScanParamLst->Lists[i].Channel = list[i].channel;
	}

	return 0;
}


int PPCgiSetFTPClient(PPCGI_FTPC_T * FTPClientParamSet)
{
	printf("======================PPCgiSetFTPClient=========================\n");
	strncpy(runNetworkCfg.ftp.address, FTPClientParamSet->FTPSvr, sizeof(runNetworkCfg.ftp.address));
	strncpy(runNetworkCfg.ftp.datapath, FTPClientParamSet->DataPath, sizeof(runNetworkCfg.ftp.datapath));
	strncpy(runNetworkCfg.ftp.password, FTPClientParamSet->Pass, sizeof(runNetworkCfg.ftp.password));
	strncpy(runNetworkCfg.ftp.userName, FTPClientParamSet->User, sizeof(runNetworkCfg.ftp.userName));
	strncpy(runNetworkCfg.ftp.filename, FTPClientParamSet->FileName, sizeof(runNetworkCfg.ftp.filename));
	runNetworkCfg.ftp.port = FTPClientParamSet->Port;
	runNetworkCfg.ftp.interval = FTPClientParamSet->UploadInterval;

	CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,1,0), (WORK_CALLBACK)NetworkCfgSave);
	return 0;
}

int PPCgiGetFTPClient(PPCGI_FTPC_T * FTPClientParamGet)
{
	printf("======================PPCgiGetFTPClient=========================\n");
	strncpy(FTPClientParamGet->FTPSvr, runNetworkCfg.ftp.address, sizeof(FTPClientParamGet->FTPSvr));
	strncpy(FTPClientParamGet->DataPath, runNetworkCfg.ftp.datapath, sizeof(FTPClientParamGet->DataPath));
	strncpy(FTPClientParamGet->Pass, runNetworkCfg.ftp.password, sizeof(FTPClientParamGet->Pass));
	strncpy(FTPClientParamGet->User, runNetworkCfg.ftp.userName, sizeof(FTPClientParamGet->User));
	strncpy(FTPClientParamGet->FileName, runNetworkCfg.ftp.filename, sizeof(FTPClientParamGet->FileName));
	FTPClientParamGet->Port = runNetworkCfg.ftp.port;
	FTPClientParamGet->UploadInterval = runNetworkCfg.ftp.interval;
	return 0;
}

int PPCgiSetCIFS(PPCGI_CIFS_T * CIFSParamSet)
{
	return 0;//--
}

int PPCgiGetCIFS(PPCGI_CIFS_T * CIFSParamGet)
{
	return 0;//--
}

int PPCgiGetHTTPPort()
{
	return 80;
}

int PPCgiSetHTTPPort(int HTTPPort)
{
	return 0;
}

static pthread_mutex_t sd_mutex = PTHREAD_MUTEX_INITIALIZER;

int PPCgiSetStorage(PPCGI_STORAGE_T * StorageParamSet)
{
	printf("======================PPCgiSetStorage=========================\n");
	switch (StorageParamSet->Command)
	{
	case CMD_STORAGE_FORMAT:
		if (pthread_mutex_trylock(&sd_mutex) == 0)
		{
			grd_sd_format();
			pthread_mutex_unlock(&sd_mutex);
		} else
			printf("++++++++++++StorageParamSet formatting!++++++++++++++\n");
		break;

	case CMD_STORAGE_UMOUNT:
		if (pthread_mutex_trylock(&sd_mutex) == 0)
		{
			grd_sd_umount();
			pthread_mutex_unlock(&sd_mutex);
		} else
			printf("++++++++++++StorageParamSet umounting!++++++++++++++\n");
		break;

	case CMD_STORAGE_NONE:
	default:
		break;
	}
	return 0;
}

int PPCgiGetStorage(PPCGI_STORAGE_T * StorageParamGet)
{
	printf("======================PPCgiGetStorage=========================\n");
	StorageParamGet->Command = 0;
	strncpy(StorageParamGet->DeviceName, GRD_SD_PARTITION_PATHNAME_0, sizeof(StorageParamGet->DeviceName));
	strncpy(StorageParamGet->Filesystem, "FAT32", sizeof(StorageParamGet->Filesystem));
	strncpy(StorageParamGet->Path, GRD_SD_MOUNT_POINT, sizeof(StorageParamGet->Path));
	StorageParamGet->Size = grd_sd_get_all_size();
	StorageParamGet->Status = grd_sd_is_mount();
	StorageParamGet->StorageType = 2;
	StorageParamGet->Used = StorageParamGet->Size - grd_sd_get_free_size();

	printf("===========Size:%lld, Used:%lld\n", StorageParamGet->Size, StorageParamGet->Used);
	return 0;
}

// 媒体文件检索
// 获取检索的结果
int PPCgiLstMedia(PPCGI_MEDIA_INFO_T * MediaInfoLst)
{
	FILE_NODE node;

	printf("======================PPCgiLstMedia=========================\n");
	if (get_file_node(MediaInfoLst->hSearch, &node) != DMS_NET_FILE_SUCCESS)
	{
		printf("Fail to get file node!\n");
		return -1;
	}

	strcpy(MediaInfoLst->Filename, node.path);
	MediaInfoLst->Size = get_file_size(node.path);
	MediaInfoLst->State = 0;
	u64t_to_time((AVI_DMS_TIME *)&(MediaInfoLst->Times[0]), node.start);
	u64t_to_time((AVI_DMS_TIME *)&(MediaInfoLst->Times[1]), node.stop);

	return 0;
}

// 关闭检索操作
int PPCgiPutMediaSearch(PPCGI_MEDIA_SEARCH_T * MeidaSearchPut)
{
	printf("======================PPCgiPutMediaSearch=========================\n");
	search_close((FILE_LIST*)MeidaSearchPut->hSearch);
	return 0;
}

// 打开并设置检索条件
int PPCgiGetMediaSearch(PPCGI_MEDIA_SEARCH_T * MeidaSearchGet)
{
	printf("======================PPCgiGetMediaSearch=========================\n");
	u64t start = time_to_u64t((AVI_DMS_TIME *)&(MeidaSearchGet->Times[0]));
    u64t stop  = time_to_u64t((AVI_DMS_TIME *)&(MeidaSearchGet->Times[1]));
	FILE_LIST *list = search_file_by_time(MeidaSearchGet->Type,MeidaSearchGet->Channel, start, stop);

	MeidaSearchGet->Size = get_len_list(list);
	MeidaSearchGet->hSearch = (void*)list;
	return 0;
}


int PPCgiSetProduct(PPCGI_PRODUCT_T * ProductParamSet)
{
	struct device_info_mtd device_info;
	memset(&device_info, 0, sizeof(struct device_info_mtd));


    load_info_to_mtd_reserve(MTD_YUNNI_P2P,&device_info,sizeof(device_info));
	if (strlen(ProductParamSet->ServerStr))
	{
		memset(device_info.manufacturer_id, 0, sizeof(device_info.manufacturer_id));
		strncpy(device_info.manufacturer_id, ProductParamSet->ServerStr,
		sizeof(device_info.manufacturer_id));
	}

	if (strlen(ProductParamSet->ID))
	{
		memset(device_info.device_id, 0, sizeof(device_info.device_id));
		strncpy(device_info.device_id, ProductParamSet->ID,
			sizeof(device_info.device_id));
	}

	if (save_info_to_mtd_reserve(MTD_YUNNI_P2P, (char*)&device_info,sizeof(device_info)))
	{
		PRINT_INFO("Fail to set yunni device id\n");
		return -1;
	}

	if (!ProductParamSet->UpgradeData || ProductParamSet->UpgradeDataLens == 0)
		return 0;


	if (verify_upgrade_buff(ProductParamSet->UpgradeData, ProductParamSet->UpgradeDataLens))
	{
        PRINT_INFO("yunni check updating package, failed!\n");
        return -1;
    }

	if (upgrade_flash_by_buf(ProductParamSet->UpgradeData, ProductParamSet->UpgradeDataLens))
	{
		PRINT_INFO("yunni upgrade fail!\n");
		return -1;
	}

    while (1)
    {
        PRINT_INFO("Reboot system .... \n");
	netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT
	//system("reboot");
        sleep(3);
        PRINT_INFO("netcam_watchdog_set_timeout for reboot.... \n");
        netcam_watchdog_set_timeout(1);
        sleep(8);
    }
	return 0;
}

int PPCgiGetProduct(PPCGI_PRODUCT_T * ProductParamGet)
{
	struct device_info_mtd device_info;
	memset(&device_info, 0, sizeof(struct device_info_mtd));

	if(load_info_to_mtd_reserve(MTD_YUNNI_P2P,&device_info,sizeof(device_info)))
    {
        PRINT_INFO("no yunni device id\n");
        return -1;
    }

	strncpy(ProductParamGet->ServerStr, device_info.manufacturer_id,
		strlen(device_info.manufacturer_id));
	strncpy(ProductParamGet->ID, device_info.device_id,
		strlen(device_info.device_id));

	ProductParamGet->UpgradeStatus = get_upgrade_rate();
	return 0;
}

int PPCgiSetDefault()
{
	CfgLoadDefValueAll();
    CfgSaveAll();
    sync();
    usleep(20);
    netcam_sys_operation(0,SYSTEM_OPERATION_REBOOT);
	return 0;
}


int PPCgiGetUser(PPCGI_USER_T * UserListGet,int * ListSize)
{
	int i, j;

	if (!UserListGet || !ListSize)
	{
        PRINT_INFO("PPCgiGetUser UserListGet or ListSize is null\n");
        return -1;
    }

    for (i = 0, j = 0; i < GK_MAX_USER_NUM && j < *ListSize; i++)
	{
        if (runUserCfg.user[i].enable != 0)
		{
            strncpy(UserListGet[j].User, runUserCfg.user[i].userName,
				sizeof(UserListGet[j].User));
            strncpy(UserListGet[j].Pass, runUserCfg.user[i].password,
				sizeof(UserListGet[j].Pass));

			UserListGet[j].UserID = j;
			j++;
        }
    }

	*ListSize = j;
	return 0;
}

int PPCgiAddUser(PPCGI_USER_T * UserSet)
{
	GK_NET_USER_INFO userInfo;

	if (!UserSet)
	{
        PRINT_INFO("PPCgiAddUser UserSet is null\n");
        return -1;
    }

	memset(&userInfo, 0, sizeof(GK_NET_USER_INFO));
	strncpy(userInfo.userName, UserSet->User, sizeof(userInfo.userName));
    strncpy(userInfo.password, UserSet->Pass, sizeof(userInfo.password));

	UserAdd(&userInfo);
	UserCfgSave();
	return 0;
}

int PPCgiDelUser(PPCGI_USER_T * UserSet)
{
	if (!UserSet)
	{
        PRINT_INFO("PPCgiDelUser UserSet is null\n");
        return -1;
    }

	UserDel(UserSet->User);
	UserCfgSave();
	return 0;
}

int PPCgiSetUser(PPCGI_USER_T * UserSet)
{
	GK_NET_USER_INFO userInfo;

	if (!UserSet)
	{
        PRINT_INFO("PPCgiSetUser UserSet is null\n");
        return -1;
    }

	memset(&userInfo, 0, sizeof(GK_NET_USER_INFO));
	strncpy(userInfo.userName, UserSet->User, sizeof(userInfo.userName));
    strncpy(userInfo.password, UserSet->Pass, sizeof(userInfo.password));

	if (UserModify(&userInfo))
		UserAdd(&userInfo);
	UserCfgSave();
	return 0;
}


int PPCgiSetAnonymousEnable(int Value){return 0;}
int PPCgiGetAnonymousEnable(){return 0;}

#define MAX_PTZ_SPEED  5
static int ptz_speed_map[] = {
	MAX_PTZ_SPEED,MAX_PTZ_SPEED,
	MAX_PTZ_SPEED,MAX_PTZ_SPEED,
	MAX_PTZ_SPEED
}; // 方向 0:上 1:下 2:左 3:右	4:手动

static void ptz_prefab(int Direct, int Step)
{
	int set_or_run;
	int pre_pos;
    GK_NET_CRUISE_GROUP cruise_info;

	if (Direct < CMD_PTZ_PREFAB_BIT_SET0 ||
		Direct > CMD_PTZ_PREFAB_BIT_RUNF)
	return;

	if (Direct % 2 == 0)
		set_or_run = 0;      //set
	else
		set_or_run = 1;      //run

	pre_pos = set_or_run ? CMD_PTZ_PREFAB_BIT_RUN0 : CMD_PTZ_PREFAB_BIT_SET0;
	pre_pos = (Direct - pre_pos) / 2 + 1;

	if (set_or_run)
    {
        cruise_info.byPointNum    = 1;
        cruise_info.byCruiseIndex = 0;
        cruise_info.struCruisePoint[0].byPointIndex = 0;
        cruise_info.struCruisePoint[0].byPresetNo   = pre_pos;
        cruise_info.struCruisePoint[0].byRemainTime = 0;
        cruise_info.struCruisePoint[0].bySpeed      = -1;
        netcam_ptz_preset_cruise(&cruise_info);
    }
    else
		netcam_ptz_set_preset(pre_pos, NULL);
}

static int verDirect = 0;
static int horDirect = 0;

int PPCgiSetPTZMove(
	int Direct,
	int Step
){
	int speed;

	if (Direct < 0)
		return -1;
	else if (Direct == CMD_PTZ_UP)
		speed = ptz_speed_map[0];
	else if (Direct == CMD_PTZ_DOWN)
		speed = ptz_speed_map[1];
	else if (Direct == CMD_PTZ_LEFT)
		speed = ptz_speed_map[2];
	else if (Direct == CMD_PTZ_RIGHT)
		speed = ptz_speed_map[3];
	else
    	speed = 3;

	printf("===================>step: %d, Direct:%d\n", Step, Direct);
	if(Step < 10)
		Step = 10;
    Step = -1;
    speed = -1;
	switch (Direct)
	{
	case CMD_PTZ_UP:
		netcam_ptz_up(Step, speed);
		CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
		break;
	case CMD_PTZ_UP_STOP:
		netcam_ptz_stop();
		break;
	case CMD_PTZ_DOWN:
		netcam_ptz_down(Step, speed);
		CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
		break;
	case CMD_PTZ_DOWN_STOP:
		netcam_ptz_stop();
		break;
	case CMD_PTZ_LEFT:
		netcam_ptz_left(Step, speed);
		CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
		break;
	case CMD_PTZ_LEFT_STOP:
		netcam_ptz_stop();
		break;
	case CMD_PTZ_RIGHT:
		netcam_ptz_right(Step, speed);
		CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
		break;
	case CMD_PTZ_RIGHT_STOP:
		netcam_ptz_stop();
		break;
	case CMD_PTZ_CENTER:
		break;
	case CMD_PTZ_UP_DOWN:
		if(horDirect){
			netcam_ptz_hor_ver_cruise(speed);
		}else{
			netcam_ptz_vertical_cruise(speed);
		}
		verDirect = 1;
		break;
	case CMD_PTZ_UP_DOWN_STOP:
		netcam_ptz_stop();
		verDirect = 0;
		if (horDirect)
			netcam_ptz_horizontal_cruise(speed);
		break;
	case CMD_PTZ_LEFT_RIGHT:
		if(verDirect){
			netcam_ptz_hor_ver_cruise(speed);
		}else{
			netcam_ptz_horizontal_cruise(speed);
		}
		horDirect = 1;
		break;
	case CMD_PTZ_LEFT_RIGHT_STOP:
		netcam_ptz_stop();
		horDirect = 0;
		if (verDirect)
			netcam_ptz_vertical_cruise(speed);
		break;
	case CMD_PTZ_LEFT_UP:
		netcam_ptz_left_up(Step, speed);
		CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
		break;
	case CMD_PTZ_RIGHT_UP:
		netcam_ptz_right_up(Step, speed);
		CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
		break;
	case CMD_PTZ_LEFT_DOWN:
		netcam_ptz_left_down(Step, speed);
		CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
		break;
	case CMD_PTZ_RIGHT_DOWN:
		netcam_ptz_right_down(Step, speed);
		CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
		break;
	default:
        if (Direct >= CMD_PTZ_PREFAB_BIT_SET0 ||
		    Direct <= CMD_PTZ_PREFAB_BIT_RUNF)
        {
            ptz_prefab(Direct, Step);
        }
        else
        {
            netcam_ptz_stop();
        }
		break;
	}

	return 0;
}

int PPCgiGetPTZRunTimes(){
	return 0;
}

int PPCgiSetPTZRunTimes(
	int Value
){
	return 0;
}

int PPCgiGetPTZSpeed(
	int Direct			// 方向 0:上 1:下 2:左 3:右	4:手动
){
	if (Direct < 0 || Direct > 4)
	{
		PRINT_ERR("PPCgiGetPTZSpeed Direct = %d error\n", Direct);
		return -1;
	}

	return ptz_speed_map[Direct];
}

int PPCgiSetPTZSpeed(
	int Direct,			// 方向 0:上 1:下 2:左 3:右 4:手动
	int Value			// 值
){
	if (Direct < 0 || Direct > 4)
	{
		PRINT_ERR("PPCgiSetPTZSpeed Direct = %d error\n", Direct);
		return -1;
	}

	Value = VALUE_CONVERT(Value, 1, MAX_PTZ_SPEED);
	ptz_speed_map[Direct] = Value;
	return 0;
}


