#include "gk_cms_common.h"
#include "gk_cms_media.h"
#include "gk_cms_sock.h"
#include "gk_cms_protocol.h"
#include "gk_cms_utility.h"
#include "gk_cms_net_api.h"
#include "cfg_all.h"
#include "netcam_api.h"
#include "ntp_client.h"
#include "ipcsearch.h"

#define LED_TEST "LED_INDICATOR_TEST"
#define GPIO_RESET_TEST_START "GPIO_RESET_TEST_START"
#define AIO_TEST "AIO_TEST"
#define KEY_TEST "KEY_TEST"
#define SERVER_KEY_TEST "SERVER_KEY_TEST"
#define SERVER_TEST_TMP "SERVER_TEST_TMP"
#define RED_LED_ON "RED_LED_ON"
#define RED_LED_OFF "RED_LED_OFF"
#define COLOR_LED_ON "COLOR_LED_ON"
#define COLOR_LED_OFF "COLOR_LED_OFF"
#define FACTORY_KEY_TEST "FACTORY_KEY_TEST"
#define FACTORY_KEY_TEST_OFF "FACTORY_KEY_TEST_OFF"
#define WHITE_LED_OFF "WHITE_LED_OFF"



static int factoryKeyTestStatus = 0;
static int factoryKeyTestResult = 0;
extern int serverKeyRet;
static char curTestCmd[64] = {0};
static int whileLedTest = 0;



int Gk_CmsGetDevice(int sock)
{

    //todo
    // dwSysFlags 参照 eSysFlag
    // dwServerType 参照 DMS_SERVER_TYPE_E
    DMS_NET_DEVICE_INFO device_info;
	memset(&device_info, 0, sizeof(DMS_NET_DEVICE_INFO));
	device_info.dwSize = sizeof(DMS_NET_DEVICE_INFO);
	strncpy(device_info.csDeviceName, runSystemCfg.deviceInfo.deviceName, sizeof(device_info.csDeviceName));
    device_info.byRecordLen     = runRecordCfg.recordLen;
    device_info.byRecycleRecord = runRecordCfg.recycleRecord;
    device_info.byVideoStandard = runSystemCfg.deviceInfo.videoType;
    device_info.byLanguage      = runSystemCfg.deviceInfo.languageType;
    device_info.byDateFormat    = runChannelCfg.channelInfo[0].osdDatetime.dateFormat;
    device_info.byDateSprtr     = runChannelCfg.channelInfo[0].osdDatetime.dateSprtr;
    device_info.byTimeFmt       = runChannelCfg.channelInfo[0].osdDatetime.timeFmt;

    strncpy(device_info.bySoftwareVersion,      runSystemCfg.deviceInfo.softwareVersion,    MAX_SYSTEM_STR_SIZE);
    strncpy(device_info.bySoftwareBuildDate,    runSystemCfg.deviceInfo.softwareBuildDate,  MAX_SYSTEM_STR_SIZE);
    strncpy(device_info.byDspSoftwareVersion,   runSystemCfg.deviceInfo.firmwareVersion,    MAX_SYSTEM_STR_SIZE);
    strncpy(device_info.byDspSoftwareBuildDate, runSystemCfg.deviceInfo.firmwareReleaseDate,MAX_SYSTEM_STR_SIZE);
    strncpy(device_info.byHardwareVersion,      runSystemCfg.deviceInfo.hardwareVersion,    MAX_SYSTEM_STR_SIZE);
    strncpy(device_info.byHardwareBuildDate,    runSystemCfg.deviceInfo.hardwareBuildDate,  MAX_SYSTEM_STR_SIZE);
    strncpy(device_info.byWebVersion,           runSystemCfg.deviceInfo.webVersion,         MAX_SYSTEM_STR_SIZE);
    strncpy(device_info.byWebBuildDate,         runSystemCfg.deviceInfo.webBuildDate,       MAX_SYSTEM_STR_SIZE);
    strncpy(device_info.csSerialNumber,         runSystemCfg.deviceInfo.serialNumber,       MAX_SYSTEM_STR_SIZE);
    device_info.dwServerCPUType = 1;

    //eSysFlag
    device_info.dwSysFlags = 0;
    device_info.dwSysFlags |= DMS_SYS_FLAG_ENCODE_D1;
    device_info.dwSysFlags |= DMS_SYS_FLAG_HD_RECORD;
    device_info.dwSysFlags |= DMS_SYS_FLAG_MD_RECORD;
    //device_info.dwSysFlags |= DMS_SYS_FLAG_PPPOE_DDNS;
    device_info.dwSysFlags |= DMS_SYS_FLAG_DECODE_H264;
    device_info.dwSysFlags |= DMS_SYS_FLAG_FTPUP;
    device_info.dwSysFlags |= DMS_SYS_FLAG_EMAIL;
    device_info.dwSysFlags |= DMS_SYS_FLAG_WIFI;
    //device_info.dwSysFlags |= DMS_SYS_FLAG_ACTIVE_CONNECTION;
    device_info.dwSysFlags |= DMS_SYS_FLAG_PTZ;
    device_info.dwSysFlags |= DMS_SYS_FLAG_DEVICEMAINTAIN;
    device_info.dwSysFlags |= DMS_SYS_FLAG_NETWORK;
	#ifdef MODULE_SUPPORT_RF433
	device_info.dwSysFlags |= DMS_SYS_FLAG_RF433_ALARM;
	#endif
    device_info.dwServerType = 1;
	device_info.byVideoInNum = 1;
    device_info.byAudioInNum = 1;
    device_info.byAlarmInNum = ALARM_IN_NUM;
    device_info.byAlarmOutNum = ALARM_OUT_NUM;
    device_info.byDiskNum = 0;
    device_info.byRS232Num = 1;
    device_info.byRS485Num = 0;
    device_info.byNetworkPortNum = 1;
    device_info.byDecordChans = 0;
    device_info.byVGANum = 0;
    device_info.byUSBNum = 0;
    device_info.byDiskCtrlNum = 0;
    device_info.byAuxOutNum = 0;
    device_info.byStreamNum = 3;

    return GkCmsCmdResq(sock, &device_info, sizeof(DMS_NET_DEVICE_INFO), DMS_NET_GET_DEVICECFG);
}

int Gk_CmsSetDevice(int sock)
{
    DMS_NET_DEVICE_INFO device;
	int i;
    int ret = GkSockRecv(sock, (char *)&device, sizeof(DMS_NET_DEVICE_INFO));
    if (ret != sizeof(DMS_NET_DEVICE_INFO)) {
        PRINT_ERR();
        return -1;
    }

    //todo
	strncpy(runSystemCfg.deviceInfo.deviceName, device.csDeviceName, MAX_SYSTEM_STR_SIZE);
    runRecordCfg.recordLen = device.byRecordLen;
    runRecordCfg.recycleRecord = device.byRecycleRecord;
    runSystemCfg.deviceInfo.videoType = device.byVideoStandard;
    runSystemCfg.deviceInfo.languageType = device.byLanguage;
    runChannelCfg.channelInfo[0].osdDatetime.dateFormat = device.byDateFormat;
    runChannelCfg.channelInfo[0].osdDatetime.dateSprtr = device.byDateSprtr;
    runChannelCfg.channelInfo[0].osdDatetime.timeFmt = device.byTimeFmt;

    strncpy(runSystemCfg.deviceInfo.softwareVersion,    device.bySoftwareVersion,       MAX_SYSTEM_STR_SIZE);
    strncpy(runSystemCfg.deviceInfo.softwareBuildDate,  device.bySoftwareBuildDate,     MAX_SYSTEM_STR_SIZE);
    strncpy(runSystemCfg.deviceInfo.firmwareVersion,    device.byDspSoftwareVersion,    MAX_SYSTEM_STR_SIZE);
    strncpy(runSystemCfg.deviceInfo.firmwareReleaseDate,device.byDspSoftwareBuildDate,  MAX_SYSTEM_STR_SIZE);
    strncpy(runSystemCfg.deviceInfo.hardwareVersion,    device.byHardwareVersion,       MAX_SYSTEM_STR_SIZE);
    strncpy(runSystemCfg.deviceInfo.hardwareBuildDate,  device.byHardwareBuildDate,     MAX_SYSTEM_STR_SIZE);
    strncpy(runSystemCfg.deviceInfo.webVersion,         device.byWebVersion,            MAX_SYSTEM_STR_SIZE);
    strncpy(runSystemCfg.deviceInfo.webBuildDate,       device.byWebBuildDate,          MAX_SYSTEM_STR_SIZE);
    strncpy(runSystemCfg.deviceInfo.serialNumber,       device.csSerialNumber,  sizeof(runSystemCfg.deviceInfo.serialNumber));
	for( i = 0; i < netcam_video_get_channel_number(); i++)
	{
		if(device.byDateFormat < 0 || device.byDateFormat > 5)
		{
			device.byDateFormat = 0;
		}
		runChannelCfg.channelInfo[i].osdDatetime.dateFormat = device.byDateFormat;
	}
	SystemCfgSave();
	RecordCfgSave();
	ChannelCfgSave();
    return 0;
}


int Gk_CmsGetDefDevice(int sock)
{
    //todo
    // dwSysFlags 参照 eSysFlag
    // dwServerType 参照 DMS_SERVER_TYPE_E

    DMS_NET_DEVICE_INFO device_info;
	memset(&device_info, 0, sizeof(DMS_NET_DEVICE_INFO));
	device_info.dwSize = sizeof(DMS_NET_DEVICE_INFO);

    CfgGetDefByName(deviceInfoMap, "deviceName", device_info.csDeviceName);

    CfgGetDefByName(recordMap, "recordLen", &(device_info.byRecordLen));
    CfgGetDefByName(recordMap, "recycleRecord", &(device_info.byRecycleRecord));
    CfgGetDefByName(deviceInfoMap, "videoType", &(device_info.byVideoStandard));
    CfgGetDefByName(deviceInfoMap, "languageType", &(device_info.byLanguage));
    CfgGetDefByName(osdMap[0], "osdDatetime_dateFormat", &(device_info.byDateFormat));
    CfgGetDefByName(osdMap[0], "osdDatetime_dateSprtr", &(device_info.byDateSprtr));
    CfgGetDefByName(osdMap[0], "osdDatetime_timeFmt", &(device_info.byTimeFmt));

    /* 以下不修改 */
    strncpy(device_info.bySoftwareVersion,      runSystemCfg.deviceInfo.softwareVersion,    MAX_SYSTEM_STR_SIZE);
    strncpy(device_info.bySoftwareBuildDate,    runSystemCfg.deviceInfo.softwareBuildDate,  MAX_SYSTEM_STR_SIZE);
    strncpy(device_info.byDspSoftwareVersion,   runSystemCfg.deviceInfo.firmwareVersion,    MAX_SYSTEM_STR_SIZE);
    strncpy(device_info.byDspSoftwareBuildDate, runSystemCfg.deviceInfo.firmwareReleaseDate,MAX_SYSTEM_STR_SIZE);
    strncpy(device_info.byHardwareVersion,      runSystemCfg.deviceInfo.hardwareVersion,    MAX_SYSTEM_STR_SIZE);
    strncpy(device_info.byHardwareBuildDate,    runSystemCfg.deviceInfo.hardwareBuildDate,  MAX_SYSTEM_STR_SIZE);
    strncpy(device_info.byWebVersion,           runSystemCfg.deviceInfo.webVersion,         MAX_SYSTEM_STR_SIZE);
    strncpy(device_info.byWebBuildDate,         runSystemCfg.deviceInfo.webBuildDate,       MAX_SYSTEM_STR_SIZE);
    strncpy(device_info.csSerialNumber,         runSystemCfg.deviceInfo.serialNumber, sizeof(runSystemCfg.deviceInfo.serialNumber));
    device_info.dwServerCPUType = 1;

    //eSysFlag
    device_info.dwSysFlags = 0;
    device_info.dwSysFlags |= DMS_SYS_FLAG_ENCODE_D1;
    device_info.dwSysFlags |= DMS_SYS_FLAG_HD_RECORD;
    device_info.dwSysFlags |= DMS_SYS_FLAG_MD_RECORD;
    //device_info.dwSysFlags |= DMS_SYS_FLAG_PPPOE_DDNS;
    device_info.dwSysFlags |= DMS_SYS_FLAG_DECODE_H264;
    device_info.dwSysFlags |= DMS_SYS_FLAG_FTPUP;
    device_info.dwSysFlags |= DMS_SYS_FLAG_EMAIL;
    device_info.dwSysFlags |= DMS_SYS_FLAG_WIFI;
    //device_info.dwSysFlags |= DMS_SYS_FLAG_ACTIVE_CONNECTION;
    device_info.dwSysFlags |= DMS_SYS_FLAG_PTZ;
    device_info.dwSysFlags |= DMS_SYS_FLAG_DEVICEMAINTAIN;
    device_info.dwSysFlags |= DMS_SYS_FLAG_NETWORK;
	#ifdef MODULE_SUPPORT_RF433
	device_info.dwSysFlags |= DMS_SYS_FLAG_RF433_ALARM;
	#endif
    device_info.dwServerType = 1;
	device_info.byVideoInNum = 1;
    device_info.byAudioInNum = 1;
    device_info.byAlarmInNum = 0;
    device_info.byAlarmOutNum = 0;
    device_info.byDiskNum = 0;
    device_info.byRS232Num = 1;
    device_info.byRS485Num = 0;
    device_info.byNetworkPortNum = 1;
    device_info.byDecordChans = 0;
    device_info.byVGANum = 0;
    device_info.byUSBNum = 0;
    device_info.byDiskCtrlNum = 0;
    device_info.byAuxOutNum = 0;
    device_info.byStreamNum = 3;

    return GkCmsCmdResq(sock, &device_info, sizeof(DMS_NET_DEVICE_INFO), DMS_NET_GET_DEF_DEVICECFG);
}

const int gk_cms_time_zone[] = {
	-12*60, -11*60, -10*60, -9*60, -8*60, -7*60, -6*60,
	-5*60, -4*60-30, -4*60, -3*60-30, -3*60, -2*60,
	-60, 0, 60, 2*60, 3*60, 3*60+30, 4*60, 4*60+30, 5*60,
	5*60+30, 5*60+45, 6*60, 6*60+30, 7*60, 8*60, 9*60,
	9*60+30, 10*60, 11*60, 12*60
};

int Gk_CmsGetZoneAndDst(int sock)
{
    //to do
    int i;
    DMS_NET_ZONEANDDST zone_dst;
    memset(&zone_dst, 0, sizeof(DMS_NET_ZONEANDDST));
    zone_dst.dwSize = sizeof(DMS_NET_ZONEANDDST);

	for (i = 0; i < sizeof(gk_cms_time_zone)/sizeof(int); i++)
	{
		if (runSystemCfg.timezoneCfg.timezone == gk_cms_time_zone[i])
			break;
	}

	zone_dst.nTimeZone = i == sizeof(gk_cms_time_zone)/sizeof(int) ? 0 : i;
	PRINT_INFO("board zone:%d, cms zone:%d\n",
		runSystemCfg.timezoneCfg.timezone, zone_dst.nTimeZone);
    return GkCmsCmdResq(sock, &zone_dst, sizeof(DMS_NET_ZONEANDDST), DMS_NET_GET_ZONEANDDSTCFG);
}

int Gk_CmsSetZoneAndDst(int sock)
{
    DMS_NET_ZONEANDDST zone_dst;
    int ret = GkSockRecv(sock, (char *)&zone_dst, sizeof(DMS_NET_ZONEANDDST));
    if (ret != sizeof(DMS_NET_ZONEANDDST)) {
        PRINT_ERR();
        return -1;
    }

    //todo
    PRINT_INFO("cms zone:%d\n", zone_dst.nTimeZone);
    if (zone_dst.nTimeZone >= sizeof(gk_cms_time_zone)/sizeof(int))
		zone_dst.nTimeZone = 0;

	if (zone_dst.nTimeZone < 0)
		zone_dst.nTimeZone = 0;

	netcam_sys_set_time_zone_by_utc_second(0, gk_cms_time_zone[zone_dst.nTimeZone]);
	SystemCfgSave();
    return 0;
}

int Gk_CmsGetNtp(int sock)
{
    //to do
    DMS_NET_NTP_CFG ntp;
	//ntp_param param;
    //memset(&param, 0, sizeof(ntp_param));
	memset(&ntp, 0, sizeof(DMS_NET_NTP_CFG));
	//read_ntp_param(&param);

	ntp.dwSize = sizeof(DMS_NET_NTP_CFG);
	strncpy(ntp.csNTPServer, runSystemCfg.ntpCfg.serverDomain, sizeof(ntp.csNTPServer));
	ntp.wInterval = runSystemCfg.ntpCfg.sync_time / 3600;
	ntp.byEnableNTP = runSystemCfg.ntpCfg.enable;
	ntp.cTimeDifferenceH = 0;
	ntp.cTimeDifferenceM = 0;
	ntp.wNtpPort = runSystemCfg.ntpCfg.port;

	PRINT_INFO("ntp client %s\n", ntp.byEnableNTP ? "on" : "off");
    return GkCmsCmdResq(sock, &ntp, sizeof(DMS_NET_NTP_CFG), DMS_NET_GET_NTPCFG);
}

int Gk_CmsDefNtp(int sock)
{
    //to do
    DMS_NET_NTP_CFG ntp;
	//ntp_param param;
    //memset(&param, 0, sizeof(ntp_param));
	memset(&ntp, 0, sizeof(DMS_NET_NTP_CFG));
	//read_ntp_param(&param);

	ntp.dwSize = sizeof(DMS_NET_NTP_CFG);
	strncpy(ntp.csNTPServer, runSystemCfg.ntpCfg.serverDomain, sizeof(ntp.csNTPServer));
	ntp.wInterval = runSystemCfg.ntpCfg.sync_time / 3600;
	ntp.byEnableNTP = runSystemCfg.ntpCfg.enable;
	ntp.cTimeDifferenceH = 0;
	ntp.cTimeDifferenceM = 0;
	ntp.wNtpPort = runSystemCfg.ntpCfg.port;

	PRINT_INFO("ntp client def %s\n", ntp.byEnableNTP ? "on" : "off");
    return GkCmsCmdResq(sock, &ntp, sizeof(DMS_NET_NTP_CFG), DMS_NET_GET_NTPCFG);
}


int Gk_CmsSetNtp(int sock)
{
    DMS_NET_NTP_CFG ntp;
	//ntp_param param;
    int ret = GkSockRecv(sock, (char *)&ntp, sizeof(DMS_NET_NTP_CFG));
    if (ret != sizeof(DMS_NET_NTP_CFG)) {
        PRINT_ERR();
        return -1;
    }

    //todo
    //memset(&param, 0, sizeof(ntp_param));
	//read_ntp_param(&param);
	netcam_sys_ntp_start_stop(ntp.byEnableNTP);
	ntpc_set_addr(ntp.csNTPServer, sizeof(ntp.csNTPServer), ntp.wNtpPort);
	ntpc_set_synctime(ntp.wInterval * 3600);
	
	//param.flag = ntp.byEnableNTP ? NTP_AUTO_SYNC : NTP_OFF;
	//strncpy(param.server, ntp.csNTPServer, strlen(ntp.csNTPServer));
	//param.is_overlap = 1;
	//param.port = ntp.wNtpPort;
	//param.sync_time = ntp.wInterval * 3600;

	//save_ntp_param(&param);
    return 0;
}

int Gk_CmsGetSystime(int sock)
{
    //todo
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

    return GkCmsCmdResq(sock, &systime, sizeof(DMS_TIME), DMS_NET_CMD_GET_SYSTIME);
}


int Gk_CmsSetSystime(int sock)
{
    DMS_TIME systime;
    int ret = GkSockRecv(sock, (char *)&systime, sizeof(DMS_TIME));
    if (ret != sizeof(DMS_TIME)) {
        PRINT_ERR();
        return -1;
    }

    //todo
    #if 1
    char command[128] = {0};
    DMS_TIME *set_systime = (DMS_TIME *)&systime;

    sprintf(command, "%lu-%lu-%lu %lu:%lu:%lu", set_systime->dwYear, set_systime->dwMonth,
            set_systime->dwDay, set_systime->dwHour, set_systime->dwMinute,set_systime->dwSecond);
    netcam_sys_set_time_zone_by_utc_string(command, INVALID_TIEM_ZONE);
    #else

    
    //暂时不关录像，现在单文件回放，如果每次CMS同步时间(切换网络重新登录也会同步时间)
    //会导致频繁关开录像，产生不同时间的录像文件
    #if 0
    //close record before set time
    int old_mode =0;
    old_mode = runRecordCfg.recordMode;
    if(3 != runRecordCfg.recordMode)
    {
        runRecordCfg.recordMode = 3;
    }
    
    thread_record_close();
    printf("close record before Gk_CmsSetSystime\n");
    #endif
	#if 0
    char command[128] = {0};
    snprintf(command, sizeof(command), "date -s %lu.%lu.%lu-%lu:%lu:%lu", systime.dwYear, systime.dwMonth, systime.dwDay, systime.dwHour, systime.dwMinute, systime.dwSecond);
    new_system_call(command);
    new_system_call("hwclock -w");
	#else
	struct timeval tv;
    struct tm tmIn;

    tmIn.tm_year   = systime.dwYear - 1900;
    tmIn.tm_mon    = systime.dwMonth -1;
    tmIn.tm_mday   = systime.dwDay;
    tmIn.tm_hour   = systime.dwHour;
    tmIn.tm_min    = systime.dwMinute;
    tmIn.tm_sec    = systime.dwSecond;

    tv.tv_sec =  mktime(&tmIn);
	tv.tv_sec = tv.tv_sec;
	netcam_sys_set_time_zone_by_utc_second(tv.tv_sec ,runSystemCfg.timezoneCfg.timezone);
	#endif
    printf("Set System time : year = %lu, month = %lu, day = %lu, : hour = %lu, minute = %lu, senond = %lu\n",
                    systime.dwYear, systime.dwMonth, systime.dwDay, systime.dwHour, systime.dwMinute, systime.dwSecond);

    time_t timep;
    struct tm *p;
    time(&timep);
    struct tm tt = {0};
    p = localtime_r(&timep, &tt);

    time_t timep2;
    struct tm *p2;
    time(&timep2);
    struct tm tt2 = {0};
    p2 = gmtime_r(&timep2, &tt2);


	printf("localtime: %04d%02d%02d%02d%02d%02d\n", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday,
                                                   p->tm_hour, p->tm_min, p->tm_sec);

	printf("gmtime: %04d%02d%02d%02d%02d%02d\n", p2->tm_year + 1900, p2->tm_mon + 1, p2->tm_mday,
                                                   p2->tm_hour, p2->tm_min, p2->tm_sec);

    //record recover after setted time
    //runRecordCfg.recordMode = old_mode ;

    #endif
    return 0;
}

/*************************************************************
* 函数介绍：
* 输入参数：
* 输出参数：
* 返回值  ：
* 备注    :
*************************************************************/
int Gk_GetSystime(int sock)
{
    DMS_SYSTEMTIME systime;
    DMS_SYSTEMTIME *get_systime = (DMS_SYSTEMTIME *)&systime;

    struct timeval time_tmp;
    gettimeofday(&time_tmp, NULL);

    long ts = time(NULL);
    struct tm tt = {0};
    struct tm *t = localtime_r(&ts, &tt);

    get_systime->wYear = t->tm_year + 1900;
    get_systime->wMonth = t->tm_mon + 1;
    get_systime->wDay = t->tm_mday;
    get_systime->wHour = t->tm_hour;
    get_systime->wMinute = t->tm_min;
    get_systime->wSecond = t->tm_sec;
    get_systime->wDayOfWeek = t->tm_wday;
    get_systime->wMilliseconds = time_tmp.tv_usec/1000;

#if 0 /* debug message */
    printf("Get System time : year = %d, month = %d, day = %d, : hour = %d, minute = %d, senond = %d,  week = %d, Milliseconds = %d\n",
                    get_systime->wYear,get_systime->wMonth,get_systime->wDay ,get_systime->wHour,
                    get_systime->wMinute,get_systime->wSecond,get_systime->wDayOfWeek,get_systime->wMilliseconds);
#endif


    GkSockSend(sock, (char *)&systime, sizeof(DMS_SYSTEMTIME), 100);

    return 0;
}

int Gk_SetSystime(int sock)
{
    DMS_SYSTEMTIME systime;
    int ret = GkSockRecv(sock, (char *)&systime, sizeof(DMS_SYSTEMTIME));
    if (ret != sizeof(DMS_SYSTEMTIME)) {
        PRINT_ERR();
        return -1;
    }

    PrintDmsTime(&systime);

#if 1
    char command[128] = {0};
    DMS_SYSTEMTIME *set_systime = (DMS_SYSTEMTIME *)&systime;

    sprintf(command, "%d-%d-%d %d:%d:%d", set_systime->wYear, set_systime->wMonth,
            set_systime->wDay, set_systime->wHour, set_systime->wMinute,set_systime->wSecond);
    netcam_sys_set_time_zone_by_utc_string(command, 0XFFFF);
#else
    //暂时不关录像，现在单文件回放，如果每次CMS同步时间(切换网络重新登录也会同步时间)
    //会导致频繁关开录像，产生不同时间的录像文件
#if 0
    char command[128] = {0};
    snprintf(command, sizeof(command), "date -s %lu.%lu.%lu-%lu:%lu:%lu", systime.dwYear, systime.dwMonth, systime.dwDay, systime.dwHour, systime.dwMinute, systime.dwSecond);
    new_system_call(command);
    new_system_call("hwclock -w");
#else
	struct timeval tv; // CMS 传输UTC时间到设备
    struct tm tmIn;

    tmIn.tm_year   = systime.wYear - 1900;
    tmIn.tm_mon    = systime.wMonth -1;
    tmIn.tm_mday   = systime.wDay;
    tmIn.tm_hour   = systime.wHour;
    tmIn.tm_min    = systime.wMinute;
    tmIn.tm_sec    = systime.wSecond;

    tv.tv_sec =  mktime(&tmIn);
	tv.tv_sec = tv.tv_sec;

	netcam_sys_set_time_zone_by_utc_second(tv.tv_sec, runSystemCfg.timezoneCfg.timezone);
#endif


    PRINT_INFO("Set System time : year = %d, month = %d, day = %d, : hour = %d, minute = %d, senond = %d\n",
                    systime.wYear, systime.wMonth, systime.wDay, systime.wHour, systime.wMinute, systime.wSecond);

    time_t timep;
    struct tm *p;
    time(&timep);
    struct tm tt = {0};
    p = localtime_r(&timep, &tt);

    time_t timep2;
    struct tm *p2;
    time(&timep2);
    struct tm tt2 = {0};
    p2 = gmtime_r(&timep2, &tt2);


	printf("localtime: %04d%02d%02d%02d%02d%02d\n", p->tm_year + 1900, p->tm_mon + 1, p->tm_mday,
                                                   p->tm_hour, p->tm_min, p->tm_sec);

	printf("gmtime: %04d%02d%02d%02d%02d%02d\n", p2->tm_year + 1900, p2->tm_mon + 1, p2->tm_mday,
                                                   p2->tm_hour, p2->tm_min, p2->tm_sec);

    //record recover after setted time
    //runRecordCfg.recordMode = old_mode;

#endif
    return 0;
}


int Gk_CmsGetMaintain(int sock)
{
    DMS_NET_DEVICEMAINTAIN maintain;
    memset(&maintain, 0, sizeof(DMS_NET_DEVICEMAINTAIN));
    maintain.dwSize = sizeof(DMS_NET_DEVICEMAINTAIN);

    //to do
	maintain.byEnable = runSystemCfg.maintainCfg.enable;
	maintain.byIndex = runSystemCfg.maintainCfg.index;
    maintain.stDeviceMaintain.wHour = runSystemCfg.maintainCfg.hour;
    maintain.stDeviceMaintain.wMinute = runSystemCfg.maintainCfg.minute;
    maintain.stDeviceMaintain.wSecond = runSystemCfg.maintainCfg.second;

    return GkCmsCmdResq(sock, &maintain, sizeof(DMS_NET_DEVICEMAINTAIN), DMS_DEV_GET_DEVICEMAINTAINCFG);
}

int Gk_CmsSetMaintain(int sock)
{
    DMS_NET_DEVICEMAINTAIN maintain;

    int ret = GkSockRecv(sock, (char *)&maintain, sizeof(DMS_NET_DEVICEMAINTAIN));
    if (ret != sizeof(DMS_NET_DEVICEMAINTAIN)) {
        PRINT_ERR();
        return -1;
    }

    //todo
	runSystemCfg.maintainCfg.enable = maintain.byEnable;
	runSystemCfg.maintainCfg.index = maintain.byIndex;
    runSystemCfg.maintainCfg.hour = maintain.stDeviceMaintain.wHour;
    runSystemCfg.maintainCfg.minute = maintain.stDeviceMaintain.wMinute;
    runSystemCfg.maintainCfg.second = maintain.stDeviceMaintain.wSecond;

	SystemCfgSave();
	netcam_sys_init_maintain();

    return 0;
}




int Gk_CmsGetException(int sock)
{
    DMS_NET_EXCEPTION exception;
    memset(&exception, 0, sizeof(DMS_NET_EXCEPTION));
    exception.dwSize = sizeof(DMS_NET_EXCEPTION);

    //todo

    return GkCmsCmdResq(sock, &exception, sizeof(DMS_NET_EXCEPTION), DMS_NET_GET_EXCEPTIONCFG);
}


int Gk_CmsSetException(int sock)
{
    DMS_NET_EXCEPTION exception;
    int ret = GkSockRecv(sock, (char *)&exception, sizeof(DMS_NET_EXCEPTION));
    if (ret != sizeof(DMS_NET_EXCEPTION)) {
        PRINT_ERR();
        return -1;
    }

    //todo

    return 0;
}


int Gk_CmsGetRestore(int sock)
{
    DMS_NET_RESTORECFG restore;
    memset(&restore, 0, sizeof(DMS_NET_RESTORECFG));
    restore.dwSize = sizeof(DMS_NET_RESTORECFG);
    //todo
    //mask参照RESTORE_MASK_E

    return GkCmsCmdResq(sock, &restore, sizeof(DMS_NET_RESTORECFG), DMS_NET_GET_RESTORECFG);
}

int Gk_CmsSetRestore(int sock)
{
    DMS_NET_RESTORECFG restore;
    int ret = GkSockRecv(sock, (char *)&restore, sizeof(DMS_NET_RESTORECFG));
    if (ret != sizeof(DMS_NET_RESTORECFG)) {
        PRINT_ERR();
        return -1;
    }

    //todo

    return 0;
}

int Gk_CmsSetSave(int sock)
{
    DMS_NET_RESTORECFG restore;
    int ret = GkSockRecv(sock, (char *)&restore, sizeof(DMS_NET_RESTORECFG));
    if (ret != sizeof(DMS_NET_RESTORECFG)) {
        PRINT_ERR();
        return -1;
    }

    //todo

    return 0;
}

void Gk_CmsSave()
{
    CfgSaveAll();
}

void Gk_CmsRestore()
{
    netcam_sys_operation((void*)0,(void*)SYSTEM_OPERATION_HARD_DEFAULT);
}

void Gk_CmsTestSpeaker()
{
    //netcam_audio_out(HINT_SUCCESS);	xqq
}

void Gk_CmsTestIRCut()
{
    GK_NET_IMAGE_CFG imagAttr,imagTempAttr;
    netcam_image_get(&imagAttr);
    memcpy(&imagTempAttr,&imagAttr,sizeof(GK_NET_IMAGE_CFG));
    // 设置成黑夜模式
    imagTempAttr.irCutMode = ISP_IRCUT_MODE_NIGHT;
    netcam_image_set(imagTempAttr);
    sleep(5);
    // 设置成白天模式
    imagTempAttr.irCutMode = ISP_IRCUT_MODE_DAYLIGHT;
    netcam_image_set(imagTempAttr);
    sleep(5);
    //恢复默认设置
    netcam_image_set(imagAttr);
}

void Gk_CmsTestWhiteLed()
{
    if (sdk_cfg.gpio_light_led >= 0)
    {
        whileLedTest = 1;
        sdk_isp_light_led_set(1);
    }
}

int Gk_CmsTestGetTestResult(int sock)
{
    NET_TEST_RESULT_INFO stTestResultPar;
    IPCsearch_get_test_result(&stTestResultPar);
    return GkCmsCmdResq(sock, (char *)&stTestResultPar, sizeof(NET_TEST_RESULT_INFO), CMD_TEST_GET_TEST_RESULT);
}

int Gk_CmsTestSetTestResult(int sock)
{
    NET_TEST_RESULT_INFO stTestResultPar;
    int ret = GkSockRecv(sock, (char *)&stTestResultPar, sizeof(NET_TEST_RESULT_INFO));
    if (ret != sizeof(NET_TEST_RESULT_INFO)) {
        PRINT_ERR();
        return -1;
    }
    return IPCsearch_set_test_result(&stTestResultPar);
}

int Gk_CmsTestCmd(int sock)
{


    NET_TEST_CMD_INFO stTestCmdPar={0};
    int ret = GkSockRecv(sock, (char *)&stTestCmdPar, sizeof(NET_TEST_CMD_INFO));
    if (ret != sizeof(NET_TEST_CMD_INFO)) {
        PRINT_ERR();
        return -1;
    }

    #if 0
	if(strstr(stTestCmdPar.cmd,LED_TEST))
		return Gk_CmsLedIndicatorTest();

	if(strstr(stTestCmdPar.cmd,GPIO_RESET_TEST_START))
	{
		netcam_sys_set_reset_flag(0);

		#if 0
		netcam_timer_del_task(clear_rest_status);
		netcam_timer_add_task(clear_rest_status, 20*NETCAM_TIMER_ONE_SEC, SDK_FALSE, SDK_TRUE);
		#else
		Gk_CmsResetTest();
		#endif
		return 0;
	}

	if(strstr(stTestCmdPar.cmd,AIO_TEST))
		return Gk_CmsAioTest();
    #endif

    strcpy(curTestCmd, stTestCmdPar.cmd);
	if(!strcmp(stTestCmdPar.cmd, KEY_TEST))
	{        
        #ifdef MODULE_SUPPORT_MOJING_V5_PRIVACY
        printf("key test open\n");
        mojing_qiuji_get_key();
        #endif
		return 0;
	}
    else if (!strcmp(stTestCmdPar.cmd, SERVER_KEY_TEST))
    {
        return 0;
    }
    else if (!strcmp(stTestCmdPar.cmd, SERVER_TEST_TMP))
    {

        return 0;
    }
#if 0
    else if (!strcmp(stTestCmdPar.cmd, RED_LED_ON))
    {
        sdk_isp_indicator_led_set(sdk_cfg.gpio_red_led, 1);
        sdk_isp_indicator_led_set(sdk_cfg.gpio_color_led, 0);
    }
    else if (!strcmp(stTestCmdPar.cmd, RED_LED_OFF))
    {
        sdk_isp_indicator_led_set(sdk_cfg.gpio_red_led, 0);
    }
    else if (!strcmp(stTestCmdPar.cmd, COLOR_LED_ON))
    {
        sdk_isp_indicator_led_set(sdk_cfg.gpio_color_led, 1);
        sdk_isp_indicator_led_set(sdk_cfg.gpio_red_led, 0);
    }
    else if (!strcmp(stTestCmdPar.cmd, COLOR_LED_OFF))
    {
        sdk_isp_indicator_led_set(sdk_cfg.gpio_color_led, 0);
    }
    else if (!strcmp(stTestCmdPar.cmd, FACTORY_KEY_TEST))
    {
        factoryKeyTestStatus = 1;
        factoryKeyTestResult = 0;
    }
    else if(!strcmp(stTestCmdPar.cmd, FACTORY_KEY_TEST_OFF))
    {
        factoryKeyTestStatus = 0;
        factoryKeyTestResult = 0;
    }
    else if(!strcmp(stTestCmdPar.cmd, AIO_TEST))
    {
        return 0;
    }	
    else if (!strcmp(stTestCmdPar.cmd, WHITE_LED_OFF))
    {        
        if (sdk_cfg.gpio_light_led >= 0)
        {
            sdk_isp_light_led_set(0);
            whileLedTest = 0;
        }
    }
#endif	//xqq
    return 0;
}

int Gk_CmsGetLog(int sock)
{
	int ret = 0;
	
    DMS_NET_LOG_INFO log;
    //to do

	memset(&log,0,sizeof(DMS_NET_LOG_INFO));

    if (strcmp(curTestCmd, KEY_TEST) == 0)
    {
#ifdef MODULE_SUPPORT_MOJING_V5_PRIVACY
    	if(mojing_qiuji_get_key())
    	{
    		log.szReserve[0] = 1;
    	}
#endif    
    }
    else if (strcmp(curTestCmd, SERVER_KEY_TEST) == 0)
    {
        log.szReserve[0] = serverKeyRet;
        serverKeyRet = -1;
    }
    else if (strcmp(curTestCmd, FACTORY_KEY_TEST) == 0)
    {
        log.szReserve[0] = factoryKeyTestResult;
        factoryKeyTestResult = 0;
    }
#if 0
    else if (strcmp(curTestCmd, AIO_TEST) == 0)
    {
        log.szReserve[0] = netcam_audio_get_db();
    }
    else if (strcmp(curTestCmd, RED_LED_ON) == 0)
    {
        log.szReserve[0] = sdk_isp_indicator_led_get_status(sdk_cfg.gpio_red_led);
    }
    else if (strcmp(curTestCmd, COLOR_LED_ON) == 0)
    {
        log.szReserve[0] = sdk_isp_indicator_led_get_status(sdk_cfg.gpio_color_led);
    }  
    else if (strcmp(curTestCmd, COLOR_LED_ON) == 0)
    {
        log.szReserve[0] = sdk_isp_indicator_led_get_status(sdk_cfg.gpio_color_led);
    }      
    else if (whileLedTest)
    {
        log.szReserve[0] = sdk_isp_light_led_get_status();
    }
#endif		//xqq
	printf("Gk_CmsGetLog:szReserve[0]:%d\n",(int)(log.szReserve[0]));

    ret = GkCmsCmdResq(sock, &log, sizeof(DMS_NET_LOG_INFO), DMS_NET_GET_LOGINFO);

	if(!ret)
		printf("\nGk_CmsGetLog send success!\n");
	else
		printf("\nGk_CmsGetLog send failed!\n");

	return ret;
}

int Gk_CmsGetAFOffset(int sock)
{
    NET_AF_OFFSET_INFO stAFLastPar;
    /* send last af len param to ipc tool as the flag */
    memset(&stAFLastPar, 0, sizeof(NET_AF_OFFSET_INFO));

    return GkCmsCmdResq(sock, (char *)&stAFLastPar, sizeof(NET_AF_OFFSET_INFO), CMD_GET_AF_OFFSET);
}

int Gk_CmsSetAFOffset(void)
{
    return 0;
}

int Gk_CmsGetFactoryKeyTestStatus(void)
{
    return factoryKeyTestStatus;
}

void Gk_CmsSetFactoryKeyTestResult(int keyStatus)
{
    factoryKeyTestResult = keyStatus;
}

