/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif-no-gsoap/src/onvif_nvt.c
**
** \brief       onvif nvt
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/prctl.h>
#include "http.h"
#include "delaytask.h"
#include "soap_packet.h"
#include "soap_parse.h"
#include "config.h"
#include "onvif_priv.h"
#include "nvt_priv.h"
#include "event_handle.h"
//#include "sdk_sys.h"

#define UDP_SELECT_TIMEOUT			5			
#define UDP_SELECT_TIMEOUT_TIMES	36

struct sockaddr_in addr_eth0_old;
struct sockaddr_in addr_eth0_new;
struct sockaddr_in addr_wlan0_old;
struct sockaddr_in addr_wlan0_new;

typedef struct
{
	//GK_CHAR	hostIP[IPV4_STR_LENGTH];
	//GK_S32  serverPort;
	GK_BOOL isDiscoverable;
	GK_S32    discFd;
	pthread_t discID;
    struct ip_mreq  multicast_opt;
}ONVIF_CLS;

GK_BOOL g_webFlag;
HTTPSRV g_http_srv;
ONVIF_CLS g_onvif_cls;
GkIpc_OnvifInf g_GkIpc_OnvifInf;
static GK_BOOL IsEth0 = GK_TRUE;
static pthread_mutex_t g_xmlMutex;

GOnvif_DEVMNG_CallbackFunc_S    g_DevMngCallBackRegister;
GOnvif_MEDIA_CallbackFunc_S     g_MediaCallBackRegister;
GOnvif_IMAGE_CallbackFunc_S     g_ImageCallBackRegister;
GOnvif_PTZ_CallbackFunc_S       g_PtzCallBackRegister;
GOnvif_DEVICEIO_CallbackFunc_S  g_DeviceIOCallBackRegister;
GOnvif_EVENT_CallbackFunction_S g_EventCallBackRegister;
GOnvif_ANALYTICS_CallbackFunc_S g_AnalyticsCallBackRegister;

extern GK_S32  media_GetVideoEncoderConfigurations(void *argv);
extern GK_S32  media_GetAudioEncoderConfigurations(void *argv);
extern GK_S32  media_GetVideoEncoderConfigurationOptions(void *argv, GK_S32 sizeOptions);
extern GK_S32  media_GetAudioEncoderConfigurationOptions(void *argv, GK_S32 sizeOptions);
extern GK_BOOL devMng_MatchString(const GK_CHAR *client, const GK_CHAR *server);
extern GK_S32  devMng_SynUsers();

/******************************************************************************/
static void   nvt_XmlSave_Handle(GK_S32 xmlType);
static int    nvt_Send_HelloMessage(int fd);
static int    nvt_Send_ByeMessage(int fd);
static GK_S32 nvt_UDP_SoapServer_Init(void);
static GK_S32 nvt_UDP_SoapServer_Start(void);
static void  *nvt_UDP_SoapServer_Thread(void *argv);
static void   nvt_UDP_HTTP_SoapServer_Exit(GK_S32 signal);
static GK_S32 nvt_add_to_multicast(struct sockaddr_in *pstAddr);
//static GK_S32 nvt_remove_from_multicast(struct sockaddr_in *pstAddr);
static GK_S32 nvt_syn_net_info(ONVIF_DeviceNet_S stNetInfo, GK_CHAR *nicName);
static GK_S32 nvt_Probe(GK_CHAR *messageID, GK_CHAR *scopeItem, GK_BOOL scopeVaild, GK_S32 fd, GK_U32 ip, GK_U32 port);


/**********************************************************************
函数描述：环境初始化
入口参数：无
返回值     ：NVT_NO_ERROR: 成功
         		      NVT_SYS_ERROR: 失败，系统初始化错误
**********************************************************************/
GONVIF_NVT_Ret_E GK_NVT_Env_Init(void)
{
	memset(&g_GkIpc_OnvifInf, 0, sizeof(GkIpc_OnvifInf));
	if(pthread_mutex_init(&g_xmlMutex, NULL) < 0)
	{
		ONVIF_ERR("Fail to initialize xml mutex.\n");
		return NVT_SYS_ERROR;
	}

    return NVT_NO_ERROR;
}

GONVIF_NVT_Ret_E GK_NVT_Signal_Init(void)
{
	struct sigaction act;
	memset(&act, 0, sizeof(act));
	act.sa_flags = 0;
	sigemptyset(&act.sa_mask);
	act.sa_handler = nvt_UDP_HTTP_SoapServer_Exit;
	if(sigaction(SIGINT, &act, NULL)
		|| sigaction(SIGQUIT, &act, NULL)
		|| sigaction(SIGTERM, &act, NULL))
	{
		return NVT_SYS_ERROR;
	}


	return NVT_NO_ERROR;
}

/**********************************************************************
函数描述：设备模块初始化
入口参数：无
返回值     ：NVT_NO_ERROR: 成功
         		      NVT_SYS_ERROR: 失败，系统初始化错误
**********************************************************************/
GONVIF_NVT_Ret_E GK_NVT_Device_Init(int type)
{
    GK_S32 i = 0;
    GK_S32 ret = 0;
    if(config_Load_DeviceXml(type) < 0)
    {
        ONVIF_ERR("Load device.xml Failed!\n");
        return NVT_SYS_ERROR;
	}
	ret = devMng_SynUsers();
	if(ret != 0)
	{
		ONVIF_ERR("Fail to synchronise users info.\n");
		return NVT_SYS_ERROR;
	}
	
    ONVIF_DBG("[device][user]user count:%d\n", g_GkIpc_OnvifInf.devmgmt.Usercount);
    for(i = 0; i < g_GkIpc_OnvifInf.devmgmt.Usercount; i++)
    {
	    ONVIF_DBG("[Device][user]Name:%s, pwd:%s, level: %d\n", 
	    	g_GkIpc_OnvifInf.devmgmt.User[i]->aszUsername, g_GkIpc_OnvifInf.devmgmt.User[i]->aszPassword, g_GkIpc_OnvifInf.devmgmt.User[i]->enUserLevel);
	}
    ONVIF_DBG("[device]DHCP:%d, DateTimeType:%d, DaylightSavings:%d, time_zone:%d, NIC_alive:%d\n"
              "[device][NetworkProtocolsInf]sizeNetworkProtocols:%d, HTTP Port:%d enable:%d, RTSP Port:%d enable:%d\n"
              "[device][DeviceInfo]Manufacturer:%s, Model:%s, FirmwareVersion:%s, SerialNumber:%s, HardwareId:%s\n",
        g_GkIpc_OnvifInf.devmgmt.DHCP,
        g_GkIpc_OnvifInf.devmgmt.enDateTimeType,
        g_GkIpc_OnvifInf.devmgmt.bDaylightSavings, g_GkIpc_OnvifInf.devmgmt.time_zone, g_GkIpc_OnvifInf.devmgmt.NIC_alive,
        g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.sizeNetworkProtocols,
        g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.stNetworkProtocols[0].port[0],
        g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.stNetworkProtocols[0].enEnabled,
        g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.stNetworkProtocols[1].port[0],
        g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.stNetworkProtocols[1].enEnabled,
        g_GkIpc_OnvifInf.devmgmt.stDeviceInfo.aszManufacturer,
        g_GkIpc_OnvifInf.devmgmt.stDeviceInfo.aszModel,
        g_GkIpc_OnvifInf.devmgmt.stDeviceInfo.aszFirmwareVersion,
        g_GkIpc_OnvifInf.devmgmt.stDeviceInfo.aszSerialNumber,
        g_GkIpc_OnvifInf.devmgmt.stDeviceInfo.aszHardwareId);

    return NVT_NO_ERROR;

}

GONVIF_NVT_Ret_E GK_NVT_Discover_Init(int type)
{
    GK_U8 i = 0;
    if(config_Load_DiscoverXml(type) < 0)
    {
        return NVT_SYS_ERROR;
	}
	for(i = 0; i < g_GkIpc_OnvifInf.discov.Scopecount; i++)
	{
	    ONVIF_DBG("[discover][scopes]Name:%s\n", g_GkIpc_OnvifInf.discov.Scopes[i]->aszScopeItem);
	}	
	g_GkIpc_OnvifInf.discov.WebServerUrl = (GK_CHAR *)malloc(LEN_WEBSERVICE_URL * sizeof(GK_CHAR));
	
    ONVIF_DBG("[discover]discoverymode:%d, security:%d, hostport:%d\n",
        g_GkIpc_OnvifInf.discov.discoverymode, g_GkIpc_OnvifInf.discov.Is_security, g_GkIpc_OnvifInf.discov.hostport);
        
    return NVT_NO_ERROR;
}

/**********************************************************************
函数描述：媒体模块初始化
入口参数：无
返回值     ：NVT_NO_ERROR: 成功
         		      NVT_SYS_ERROR: 失败，系统初始化错误
**********************************************************************/
GONVIF_NVT_Ret_E GK_NVT_Media_Init(int type)
{
    GK_U8 i = 0;
	GK_S32 ret = 0;

    if (config_Load_MediastaticXml(type) < 0)
    {
        ONVIF_ERR("Load Media Static Xml Failed!\n");
        return NVT_SYS_ERROR;
    }
    // synchronise extern static media configuration options, not form private XML
    ret = media_GetVideoEncoderConfigurationOptions(g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption, g_GkIpc_OnvifInf.VideoEncodeOptionNum);
	if (ret != 0)
	{
		ONVIF_ERR("Fail to synchronise static video configuration options.\n");
		return NVT_SYS_ERROR;
	}
    ret = media_GetAudioEncoderConfigurationOptions(g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption, g_GkIpc_OnvifInf.AudioEncodeOptionNum);
	if (ret != 0)
	{
		ONVIF_ERR("Fail to synchronise static audio configuration options.\n");
		return NVT_SYS_ERROR;
	}
	if (config_Load_MediadynamicXml(MEDIADYNAMIC) < 0)
	{
		ONVIF_ERR("Load Media Dynamic Xml Failed!\n");
		return NVT_SYS_ERROR;
	}
    
	ret = media_GetVideoEncoderConfigurations(g_GkIpc_OnvifInf.gokeIPCameraVideoEncode);
	if (ret != 0)
	{
		ONVIF_ERR("Fail to synchronise dynimic video configurations.\n");
		return NVT_SYS_ERROR;
	}
    ret = media_GetAudioEncoderConfigurations(g_GkIpc_OnvifInf.gokeIPCameraAudioEncode);
	if (ret != 0)
	{
		ONVIF_ERR("Fail to synchronise dynimic audio configurations.\n");
		return NVT_SYS_ERROR;
	}
	for (i = 0; i < g_GkIpc_OnvifInf.VideoSourceOptionNum; i++)
	{
	    ONVIF_DBG("[mediastatic][VideoSourceOption]XRangeMin:%d, XRangeMax:%d, YRangeMin:%d, YRangeMax:%d\n"
	              "[mediastatic][VideoSourceOption]WidthRangeMin:%d, WidthRangeMax:%d, HeightRangeMin:%d, HeightRangeMax:%d\n"
	              "[mediastatic][VideoSourceOption]sizeVideoSourceTokens:%d, VideoSourceTokens:%s\n",
            g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[i].stBoundsRange.stXRange.min,
            g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[i].stBoundsRange.stXRange.max,
            g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[i].stBoundsRange.stYRange.min,
            g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[i].stBoundsRange.stYRange.max,
            g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[i].stBoundsRange.stWidthRange.min,
            g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[i].stBoundsRange.stWidthRange.max,
            g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[i].stBoundsRange.stHeightRange.min,
            g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[i].stBoundsRange.stHeightRange.max,
            g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[i].sizeVideoSourceTokensAvailable,
            *g_GkIpc_OnvifInf.gokeVideoSourceConfigurationOption[i].pszVideoSourceTokensAvailable);
	}

	for(i = 0; i < g_GkIpc_OnvifInf.AudioSourceOptionNum; i++)
	{
	    ONVIF_DBG("[mediastatic][AudioSourceOption]sizeInputTokensAvailable:%d, TokensAvailable:%s\n",
            g_GkIpc_OnvifInf.gokeAudioSourceConfigurationOption[i].sizeInputTokensAvailable,
            *g_GkIpc_OnvifInf.gokeAudioSourceConfigurationOption[i].pszInputTokensAvailable);
	}

	for(i = 0; i < g_GkIpc_OnvifInf.VideoEncodeOptionNum; i++)
	{
		ONVIF_DBG("[mediastatic][VideoEncodeOption]QualityMin:%d, QualityMax:%d\n",
            g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stQualityRange.min,
            g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stQualityRange.max);
        ONVIF_DBG("[VideoEncodeOption][JPEG]sizeResolutions:%d\n",
            g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stJPEG.sizeResolutionsAvailable);
        GK_S32 j;
        for(j = 0; j < g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stJPEG.sizeResolutionsAvailable; j++)
        {
            ONVIF_DBG("[VideoEncodeOption][JPEG]Width:%d, Height:%d\n",
                g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stJPEG.pstResolutionsAvailable[j].width,
                g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stJPEG.pstResolutionsAvailable[j].height);
        }
        ONVIF_DBG("[VideoEncodeOption][JPEG]FrameRateMin:%d, FrameRateMax:%d, EncodingIntervalMin:%d, EncodingIntervalMax:%d\n",
            g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stJPEG.stFrameRateRange.min,
            g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stJPEG.stFrameRateRange.max,
            g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stJPEG.stEncodingIntervalRange.min,
            g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stJPEG.stEncodingIntervalRange.max);


		ONVIF_DBG("[VideoEncodeOption][H264]sizeResolutions:%d\n",
            g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.sizeResolutionsAvailable);
        for(j = 0; j < g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.sizeResolutionsAvailable; j++)
        {
            ONVIF_DBG("[VideoEncodeOption][H264]Width:%d, Height:%d\n",
                g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.pstResolutionsAvailable[j].width,
                g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.pstResolutionsAvailable[j].height);
        }
        ONVIF_DBG("[VideoEncodeOption][H264]GovLengthMin:%d, GovLengthMax:%d\n"
                  "[VideoEncodeOption][H264]FrameRateMin:%d, FrameRateMax:%d\n"
                  "[VideoEncodeOption][H264]EncodingIntervalMin:%d, EncodingIntervalMax:%d\n"
		          "[VideoEncodeOption][H264]sizeH264ProfilesSupported:%d, H264ProfilesSupported:%d\n",
            g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.stGovLengthRange.min,
            g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.stGovLengthRange.max,
            g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.stFrameRateRange.min,
            g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.stFrameRateRange.max,
            g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.stEncodingIntervalRange.min,
            g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.stEncodingIntervalRange.max,
            g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.sizeH264ProfilesSupported,
            *g_GkIpc_OnvifInf.gokeVideoEncoderConfigurationOption[i].stH264.penH264ProfilesSupported);
	}

	for(i = 0; i < g_GkIpc_OnvifInf.AudioEncodeOptionNum; i++)
	{
		ONVIF_DBG("[mediastatic][AudioEncodeOption]sizeOptions:%d, Encoding:%d\n"
		          "[mediastatic][AudioEncodeOption]sizeItems:%d, BLItems0:%d, sizeItems:%d, SRItems0:%d\n",
            g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[i].sizeOptions,
            g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[i].pstOptions->enEncoding,
            g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[i].pstOptions->stBitrateList.sizeItems,
            *g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[i].pstOptions->stBitrateList.pItems,
            g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[i].pstOptions->stSampleRateList.sizeItems,
            *g_GkIpc_OnvifInf.gokeAudioEncoderConfigurationOption[i].pstOptions->stSampleRateList.pItems);
	}

	for(i = 0; i < g_GkIpc_OnvifInf.VideoAnalyticNum; i++)
	{
        ONVIF_DBG("[mediastatic][VideoAnalytic]Token:%s, Name:%s, Usecount:%d\n",
            g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[i].pszToken,
            g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[i].pszName,
            g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[i].useCount);
        GK_S32 j;
        for(j = 0; j < g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[i].stAnalyticsEngineConfiguration.sizeAnalyticsModule; j++)
        {
            ONVIF_DBG("[AnalyticsModule]Name:%s, Type:%s\n",
                g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[i].stAnalyticsEngineConfiguration.pstAnalyticsModule[j].pszName,
                g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[i].stAnalyticsEngineConfiguration.pstAnalyticsModule[j].pszType);

            GK_S32 k;
            for(k = 0;
                k < g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[i].stAnalyticsEngineConfiguration.pstAnalyticsModule[j].stParameters.sizeSimpleItem;
                k++)
            {
                ONVIF_DBG("[SimpItem]Name:%s, Value:%s\n",
                    g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[i].stAnalyticsEngineConfiguration.pstAnalyticsModule[j].stParameters.pstSimpleItem[k].pszName,
                    g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[i].stAnalyticsEngineConfiguration.pstAnalyticsModule[j].stParameters.pstSimpleItem[k].pszValue);


            }
            for(k = 0;
                k < g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[i].stAnalyticsEngineConfiguration.pstAnalyticsModule[j].stParameters.sizeElementItem;
                k++)
            {
                ONVIF_DBG("[ElementItem]Name:%s\n",
                    g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[i].stAnalyticsEngineConfiguration.pstAnalyticsModule[j].stParameters.pstElementItem[k].pszName);
            }
        }
        for(j = 0; j < g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[i].stRuleEngineConfiguration.sizeRule; j++)
        {
            ONVIF_DBG("[AnalyticsModule]Num:%s, Type:%s\n",
                g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[i].stRuleEngineConfiguration.pstRule[j].pszName,
                g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[i].stRuleEngineConfiguration.pstRule[j].pszType);


            GK_S32 k;
            for(k = 0; k < g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[i].stRuleEngineConfiguration.pstRule[j].stParameters.sizeSimpleItem; k++)
            {
                ONVIF_DBG("[SimpItem]Name:%s, Value:%s\n",
                    g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[i].stRuleEngineConfiguration.pstRule[j].stParameters.pstSimpleItem[k].pszName,
                    g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[i].stRuleEngineConfiguration.pstRule[j].stParameters.pstSimpleItem[k].pszValue);
            }
            for(k = 0; k < g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[i].stRuleEngineConfiguration.pstRule[j].stParameters.sizeElementItem; k++)
            {
                ONVIF_DBG("[ElementItem]Name:%s\n",
                    g_GkIpc_OnvifInf.gokeIPCameraVideoAnalytic[i].stRuleEngineConfiguration.pstRule[j].stParameters.pstElementItem[k].pszName);
            }

        }

    }
    
	for(i = 0; i < g_GkIpc_OnvifInf.VideoSourceNum; i++)
	{
	    ONVIF_DBG("[mediadynamic][VideoSource]Token:%s, Name:%s, UseCount:%d, SourceToken:%s, bound-height:%d, bound-width:%d, bound-y:%d, bound-x:%d\n",
            g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].pszToken,
            g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].pszName,
            g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].useCount,
            g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].pszSourceToken,
            g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].stBounds.height,
            g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].stBounds.width,
            g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].stBounds.y,
            g_GkIpc_OnvifInf.gokeIPCameraVideoSource[i].stBounds.x);
	}

	for(i = 0; i < g_GkIpc_OnvifInf.AudioSourceNum; i++)
	{
	    ONVIF_DBG("[mediadynamic][AudioSource]Token:%s, Name:%s, UseCount:%d, SourceToken:%s,\n",
            g_GkIpc_OnvifInf.gokeIPCameraAudioSource[i].pszToken,
            g_GkIpc_OnvifInf.gokeIPCameraAudioSource[i].pszName,
            g_GkIpc_OnvifInf.gokeIPCameraAudioSource[i].useCount,
            g_GkIpc_OnvifInf.gokeIPCameraAudioSource[i].pszSourceToken);
	}

	for(i = 0; i < g_GkIpc_OnvifInf.VideoEncodeNum; i++)
	{
		ONVIF_DBG("[mediadynamic][VideoEncode]Token:%s, Name:%s, UseCount:%d, Type:%d\n"
		          "[mediadynamic][VideoEncode]IPv4Address:%s\n"
		          "[mediadynamic][VideoEncode]IPv6Address:%s\n"
		          "[mediadynamic][VideoEncode]Port:%d, TTL:%d, AutoStart:%d, SessionTimeout:%lld\n",
            g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].pszToken,
            g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].pszName,
            g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].useCount,
            g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stMulticast.stIPAddress.enType,
            g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stMulticast.stIPAddress.pszIPv4Address,
            g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stMulticast.stIPAddress.pszIPv6Address,
            g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stMulticast.port,
            g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stMulticast.ttl,
            g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].stMulticast.enAutoStart,
            g_GkIpc_OnvifInf.gokeIPCameraVideoEncode[i].sessionTimeout);
	}

	for(i = 0; i < g_GkIpc_OnvifInf.AudioEncodeNum; i++)
	{
		ONVIF_DBG("[mediadynamic][AudioEncode]Token:%s, Name:%s, userCount:%d, Encoding:%d, Bitrate:%d, SampleRate:%d\n"
		          "[mediadynamic][AudioEncode]AddrType:%d, IPv4Address:%s, IPv6Address:%s, Port:%d, TTL:%d, AutoStart:%d, SessionTimeout:%lld\n",
            g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].pszToken,
            g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].pszName,
            g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].useCount,
            g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].enEncoding,
            g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].bitRate,
            g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].sampleRate,
            g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].stMulticast.stIPAddress.enType,
            g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].stMulticast.stIPAddress.pszIPv4Address,
            g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].stMulticast.stIPAddress.pszIPv6Address,
            g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].stMulticast.port,
            g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].stMulticast.ttl,
            g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].stMulticast.enAutoStart,
            g_GkIpc_OnvifInf.gokeIPCameraAudioEncode[i].sessionTimeout);
	}

    for(i = 0; i < g_GkIpc_OnvifInf.PTZConfigurationNum; i++)
        {
            ONVIF_DBG("[mediadynamic][PTZ]Name:%s, UseCount:%d, token:%s, NodeToken:%s\n"
                      "[mediadynamic][PTZ]DefaultAbsolutePantTiltPositionSpace:%s\n"
                      "[mediadynamic][PTZ]DefaultAbsoluteZoomPositionSpace:%s\n"
                      "[mediadynamic][PTZ]DefaultRelativePanTiltTranslationSpace:%s\n"
                      "[mediadynamic][PTZ]DefaultRelativeZoomTranslationSpace:%s\n"
                      "[mediadynamic][PTZ]DefaultContinuousPanTiltVelocitySpace:%s\n"
                      "[mediadynamic][PTZ]DefaultContinuousZoomVelocitySpace:%s\n"
                      "[mediadynamic][PTZ]DefaultPTZSpeed PanTilt:space:%s, x:%f, y:%f Zoom:space:%s, x:%f\n"
                      "[mediadynamic][PTZ]DefaultPTZTimeout:%lld\n"
                      "[mediadynamic][PTZ]PanTiltLimits URI:%s, XRangeMin:%f, XRangeMax:%f, YRangeMin:%f, YRangeMax:%f\n"
                      "[mediadynamic][PTZ]ZoomLimits    URI:%s, XRangMin:%f, XRangeMax:%f\n",
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].name,
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].useCount,
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].token,
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].nodeToken,
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].defaultAbsolutePantTiltPositionSpace,
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].defaultAbsoluteZoomPositionSpace,
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].defaultRelativePanTiltTranslationSpace,
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].defaultRelativeZoomTranslationSpace,
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].defaultContinuousPanTiltVelocitySpace,
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].defaultContinuousZoomVelocitySpace,
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].stDefaultPTZSpeed.stPanTilt.aszSpace,
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].stDefaultPTZSpeed.stPanTilt.x,
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].stDefaultPTZSpeed.stPanTilt.y,
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].stDefaultPTZSpeed.stZoom.aszSpace,
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].stDefaultPTZSpeed.stZoom.x,
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].defaultPTZTimeout,
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].stPanTiltLimits.stRange.aszURI,
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].stPanTiltLimits.stRange.stXRange.min,
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].stPanTiltLimits.stRange.stXRange.max,
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].stPanTiltLimits.stRange.stYRange.min,
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].stPanTiltLimits.stRange.stYRange.max,
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].stZoomLimits.stRange.aszURI,
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].stZoomLimits.stRange.stXRange.min,
                g_GkIpc_OnvifInf.gokePTZConfiguration[i].stZoomLimits.stRange.stXRange.max);
    
        }

	for(i = 0; i < g_GkIpc_OnvifInf.relayOutputsNum; i++)
	{
		ONVIF_DBG("[mediadynamic][relayOutputs]Token:%s, Mode:%d, DelatTime:%lld, IdleState:%d\n",
            g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs[i].aszToken,
            g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs[i].stProperties.enMode,
            g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs[i].stProperties.delayTime,
            g_GkIpc_OnvifInf.pstGokeIPCameraRelayOutputs[i].stProperties.enIdleState);
	}

	for(i = 0; i < g_GkIpc_OnvifInf.ProfileNum; i++)
	{
	    ONVIF_DBG("[mediadynamic][Profile]Fix:%d, Token:%s, Name:%s\n",
            g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].enFixed,
            g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszToken,
            g_GkIpc_OnvifInf.gokeIPCameraProfiles[i].pszName);
	}

    return NVT_NO_ERROR;
}


/**********************************************************************
函数描述：图像模块初始化
入口参数：无
返回值     ：NVT_NO_ERROR: 成功
         		      NVT_SYS_ERROR: 失败，系统初始化错误
**********************************************************************/
GONVIF_NVT_Ret_E GK_NVT_Imaging_Init(int type)
{
	if(config_Load_ImagingXml(type) < 0)
	{
		ONVIF_ERR("Load Image Xml Failed!\n");
		return NVT_SYS_ERROR;
	}

    ONVIF_DBG("[imaging][ImagingOptions]BacklightCompensation sizeMode:%d, Mode0:%d, Mode1:%d LevelMin:%f, LevelMax:%f\n"
              "[imaging][ImagingOptions]Brightness Min:%f Max:%f\n"
              "[imaging][ImagingOptions]ColorSaturation Min:%f Max:%f\n"
              "[imaging][ImagingOptions]Contrast Min:%f Max:%f\n"
              "[imaging][ImagingOptions]Exposure sizeMode:%d, Mode0:%d, Mode1:%d\n"
              "[imaging][ImagingOptions]Exposure MinExposureTime Min:%f Max:%f MaxExposureTime Min:%f Max:%f\n"
              "[imaging][ImagingOptions]Exposure MinGain Min:%f Max:%f, MaxGain Min:%f Max:%f\n"
              "[imaging][ImagingOptions]Exposure ExposureTime Min:%f Max:%f, Gain Min:%f Max:%f\n"
              "[imaging][ImagingOptions]Focus sizeAutoFocusModes:%d, AFModes0:%d, AFModes1:%d\n"
              "[imaging][ImagingOptions]Focus DefaultSpeed Min:%f Max:%f, NearLimit Min:%f Max:%f, FarLimit Min:%f Max:%f\n"
              "[imaging][ImagingOptions]IrCutFilter sizeIrCutFilterModes:%d, IrCutFilterMode0:%d, IrCutFilterMode1:%d, IrCutFilterMode2:%d\n"
              "[imaging][ImagingOptions]Sharpness Min:%f Max:%f\n"
              "[imaging][ImagingOptions]WideDynamicRange sizeMode:%d, Mode0:%d, Mode1:%d, Level Min:%f Max:%f\n"
              "[imaging][ImagingOptions]WhiteBalance sizeMode:%d, Mode0:%d, Mode1:%d, YrGain Min:%f Max:%f, YbGain Min:%f Max:%f\n",
            g_GkIpc_OnvifInf.gokeImagingOptions->stBacklightCompensation.sizeMode,
            g_GkIpc_OnvifInf.gokeImagingOptions->stBacklightCompensation.penMode[0],
            g_GkIpc_OnvifInf.gokeImagingOptions->stBacklightCompensation.penMode[1],
            g_GkIpc_OnvifInf.gokeImagingOptions->stBacklightCompensation.stLevel.min,
            g_GkIpc_OnvifInf.gokeImagingOptions->stBacklightCompensation.stLevel.max,
            g_GkIpc_OnvifInf.gokeImagingOptions->stBrightness.min,
            g_GkIpc_OnvifInf.gokeImagingOptions->stBrightness.max,
            g_GkIpc_OnvifInf.gokeImagingOptions->stColorSaturation.min,
            g_GkIpc_OnvifInf.gokeImagingOptions->stColorSaturation.max,
            g_GkIpc_OnvifInf.gokeImagingOptions->stContrast.min,
            g_GkIpc_OnvifInf.gokeImagingOptions->stContrast.max,
            g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.sizeMode,
            g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.penMode[0],
            g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.penMode[1],
            g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMinExposureTime.min,
            g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMinExposureTime.max,
            g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMaxExposureTime.min,
            g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMaxExposureTime.max,
            g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMinGain.min,
            g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMinGain.max,
            g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMaxGain.min,
            g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stMaxGain.max,
            g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stExposureTime.min,
            g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stExposureTime.max,
            g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stGain.min,
            g_GkIpc_OnvifInf.gokeImagingOptions->stExposure.stGain.max,
            g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.sizeAutoFocusModes,
            g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.penAutoFocusModes[0],
            g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.penAutoFocusModes[1],
            g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stDefaultSpeed.min,
            g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stDefaultSpeed.max,
            g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stNearLimit.min,
            g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stNearLimit.max,
            g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stFarLimit.min,
            g_GkIpc_OnvifInf.gokeImagingOptions->stFocus.stFarLimit.max,
            g_GkIpc_OnvifInf.gokeImagingOptions->sizeIrCutFilterModes,
            g_GkIpc_OnvifInf.gokeImagingOptions->penIrCutFilterModes[0],
            g_GkIpc_OnvifInf.gokeImagingOptions->penIrCutFilterModes[1],
            g_GkIpc_OnvifInf.gokeImagingOptions->penIrCutFilterModes[2],
            g_GkIpc_OnvifInf.gokeImagingOptions->stSharpness.min,
            g_GkIpc_OnvifInf.gokeImagingOptions->stSharpness.max,
            g_GkIpc_OnvifInf.gokeImagingOptions->stWideDynamicRange.sizeMode,
            g_GkIpc_OnvifInf.gokeImagingOptions->stWideDynamicRange.penMode[0],
            g_GkIpc_OnvifInf.gokeImagingOptions->stWideDynamicRange.penMode[1],
            g_GkIpc_OnvifInf.gokeImagingOptions->stWideDynamicRange.stLevel.min,
            g_GkIpc_OnvifInf.gokeImagingOptions->stWideDynamicRange.stLevel.max,
            g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.sizeMode,
            g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.penMode[0],
            g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.penMode[1],
            g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.stYrGain.min,
            g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.stYrGain.max,
            g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.stYbGain.min,
            g_GkIpc_OnvifInf.gokeImagingOptions->stWhiteBalance.stYbGain.max);

    ONVIF_DBG("[imaging][ImageMoveOption]Absolute Position Min:%f Max:%f, Speed Min:%f Max:%f\n"
              "[imaging][ImageMoveOption]Relative Distance Min:%f Max:%f, Speed Min:%f Max:%f\n"
              "[imaging][ImageMoveOption]Continuous Speed Min:%f Max:%f\n",
            g_GkIpc_OnvifInf.gokeMoveOptions->stAbsolute.stPosition.min,
            g_GkIpc_OnvifInf.gokeMoveOptions->stAbsolute.stPosition.max,
            g_GkIpc_OnvifInf.gokeMoveOptions->stAbsolute.stSpeed.min,
            g_GkIpc_OnvifInf.gokeMoveOptions->stAbsolute.stSpeed.max,
            g_GkIpc_OnvifInf.gokeMoveOptions->stRelative.stDistance.min,
            g_GkIpc_OnvifInf.gokeMoveOptions->stRelative.stDistance.max,
            g_GkIpc_OnvifInf.gokeMoveOptions->stRelative.stSpeed.min,
            g_GkIpc_OnvifInf.gokeMoveOptions->stRelative.stSpeed.max,
            g_GkIpc_OnvifInf.gokeMoveOptions->stContinuous.stSpeed.min,
            g_GkIpc_OnvifInf.gokeMoveOptions->stContinuous.stSpeed.max);

	return NVT_NO_ERROR;
}

/**********************************************************************
函数描述：PTZ 模块初始化
入口参数：无
返回值     ：NVT_NO_ERROR: 成功
         		      NVT_SYS_ERROR: 失败，系统初始化错误
**********************************************************************/
GONVIF_NVT_Ret_E GK_NVT_Ptz_Init(int type)
{
	if(config_Load_PtzXml(type) < 0)
	{
		ONVIF_ERR("Load Image Xml Failed!\n");
		return NVT_SYS_ERROR;
	}

    return NVT_NO_ERROR;
}

/**********************************************************************
函数描述：事件模块初始化
入口参数：无
返回值     ：NVT_NO_ERROR: 成功
         		      NVT_SYS_ERROR: 失败，系统初始化错误
**********************************************************************/
GONVIF_NVT_Ret_E GK_NVT_Event_Init(void)
{
    EVENT_Ret_E reval;
    reval = EventManagement_Init();
    if(reval != EVENT_MANAGE_OK)
    {
       return NVT_SYS_ERROR;
    }

    return NVT_NO_ERROR;
}


/******************************************************************************
函数描述：对设备管理模块所有回调函数指针进行注册,
				以实现服务端的设备管理功能
入口参数：GOnvif_DEVMNG_CallbackFunc_S *pstNVTDevManageCBFunc  : 设备
				管理回调函数指针结构体的指针
返回值     ：NVT_NO_ERROR: 成功
         		      NVT_SYS_ERROR: 失败
******************************************************************************/
GONVIF_NVT_Ret_E GK_NVT_DevMng_Register(GOnvif_DEVMNG_CallbackFunc_S *pstNVTDevManageCBFunc)
{
    memset(&g_DevMngCallBackRegister, 0 ,sizeof(GOnvif_DEVMNG_CallbackFunc_S));
    
    g_DevMngCallBackRegister.pfnDM_SystemReboot              = pstNVTDevManageCBFunc->pfnDM_SystemReboot;
    g_DevMngCallBackRegister.pfnDM_GetWsdlUrl                = pstNVTDevManageCBFunc->pfnDM_GetWsdlUrl;
    g_DevMngCallBackRegister.pfnDM_GetCapabilities           = pstNVTDevManageCBFunc->pfnDM_GetCapabilities;
    g_DevMngCallBackRegister.pfnDM_GetServices               = pstNVTDevManageCBFunc->pfnDM_GetServices;
    g_DevMngCallBackRegister.pfnDM_GetServiceCapabilities    = pstNVTDevManageCBFunc->pfnDM_GetServiceCapabilities;
    g_DevMngCallBackRegister.pfnDM_GetHostname               = pstNVTDevManageCBFunc->pfnDM_GetHostname;
    g_DevMngCallBackRegister.pfnDM_SetHostname               = pstNVTDevManageCBFunc->pfnDM_SetHostname;
    g_DevMngCallBackRegister.pfnDM_GetDNS                    = pstNVTDevManageCBFunc->pfnDM_GetDNS;
    g_DevMngCallBackRegister.pfnDM_SetDNS                    = pstNVTDevManageCBFunc->pfnDM_SetDNS;
    g_DevMngCallBackRegister.pfnDM_GetNTP                    = pstNVTDevManageCBFunc->pfnDM_GetNTP;
    g_DevMngCallBackRegister.pfnDM_SetNTP                    = pstNVTDevManageCBFunc->pfnDM_SetNTP;
    g_DevMngCallBackRegister.pfnDM_GetNetworkInterfaces      = pstNVTDevManageCBFunc->pfnDM_GetNetworkInterfaces;
    g_DevMngCallBackRegister.pfnDM_SetNetworkInterfaces      = pstNVTDevManageCBFunc->pfnDM_SetNetworkInterfaces;
    g_DevMngCallBackRegister.pfnDM_GetNetworkProtocols       = pstNVTDevManageCBFunc->pfnDM_GetNetworkProtocols;
    g_DevMngCallBackRegister.pfnDM_SetNetworkProtocols       = pstNVTDevManageCBFunc->pfnDM_SetNetworkProtocols;
    g_DevMngCallBackRegister.pfnDM_GetNetworkDefaultGateway  = pstNVTDevManageCBFunc->pfnDM_GetNetworkDefaultGateway;
    g_DevMngCallBackRegister.pfnDM_SetNetworkDefaultGateway  = pstNVTDevManageCBFunc->pfnDM_SetNetworkDefaultGateway;
    g_DevMngCallBackRegister.pfnDM_GetSystemDateAndTime      = pstNVTDevManageCBFunc->pfnDM_GetSystemDateAndTime;
    g_DevMngCallBackRegister.pfnDM_SetSystemDateAndTime      = pstNVTDevManageCBFunc->pfnDM_SetSystemDateAndTime;
    g_DevMngCallBackRegister.pfnDM_GetDeviceInformation      = pstNVTDevManageCBFunc->pfnDM_GetDeviceInformation;
    g_DevMngCallBackRegister.pfnDM_GetUsers                  = pstNVTDevManageCBFunc->pfnDM_GetUsers;
    g_DevMngCallBackRegister.pfnDM_DeleteUsers               = pstNVTDevManageCBFunc->pfnDM_DeleteUsers;
    g_DevMngCallBackRegister.pfnDM_DeleteUsers               = pstNVTDevManageCBFunc->pfnDM_DeleteUsers;
    g_DevMngCallBackRegister.pfnDM_SetUser                   = pstNVTDevManageCBFunc->pfnDM_SetUser;
    g_DevMngCallBackRegister.pfnDM_SetSystemFactoryDefault   = pstNVTDevManageCBFunc->pfnDM_SetSystemFactoryDefault;
    g_DevMngCallBackRegister.pfnDM_GetScopes                 = pstNVTDevManageCBFunc->pfnDM_GetScopes;
    g_DevMngCallBackRegister.pfnDM_SetScopes                 = pstNVTDevManageCBFunc->pfnDM_SetScopes;
    g_DevMngCallBackRegister.pfnDM_AddScopes                 = pstNVTDevManageCBFunc->pfnDM_AddScopes;
    g_DevMngCallBackRegister.pfnDM_RemoveScopes              = pstNVTDevManageCBFunc->pfnDM_RemoveScopes;
    g_DevMngCallBackRegister.pfnDM_GetDiscoveryMode          = pstNVTDevManageCBFunc->pfnDM_GetDiscoveryMode;
    g_DevMngCallBackRegister.pfnDM_SetDiscoveryMode          = pstNVTDevManageCBFunc->pfnDM_SetDiscoveryMode;
    
    return NVT_NO_ERROR;
}

/***************************************************************************
函数描述: 对媒体服务模块所有回调函数指针进行注册,
			    以实现服务端的媒体服务功能
入口参数：GOnvif_MEDIA_CallbackFunc_S *pstNVTMediaCBFunc : 媒体服务
				回调函数指针结构体的指针
返回值     ：NVT_NO_ERROR: 成功
         		      NVT_SYS_ERROR: 失败
***************************************************************************/
GONVIF_NVT_Ret_E GK_NVT_Media_Register(GOnvif_MEDIA_CallbackFunc_S *pstNVTMediaCBFunc)
{
    memset(&g_MediaCallBackRegister, 0 ,sizeof(GOnvif_MEDIA_CallbackFunc_S));
    g_MediaCallBackRegister.pfnMD_GetServiceCapabilities = pstNVTMediaCBFunc->pfnMD_GetServiceCapabilities;
    g_MediaCallBackRegister.pfnMD_CreateProfile = pstNVTMediaCBFunc->pfnMD_CreateProfile;
    g_MediaCallBackRegister.pfnMD_GetProfile = pstNVTMediaCBFunc->pfnMD_GetProfile;
    g_MediaCallBackRegister.pfnMD_GetProfiles = pstNVTMediaCBFunc->pfnMD_GetProfiles;
    g_MediaCallBackRegister.pfnMD_DeleteProfile = pstNVTMediaCBFunc->pfnMD_DeleteProfile;
    g_MediaCallBackRegister.pfnMD_GetStreamUri = pstNVTMediaCBFunc->pfnMD_GetStreamUri;
    g_MediaCallBackRegister.pfnMD_GetSnapshotUri = pstNVTMediaCBFunc->pfnMD_GetSnapshotUri;
    g_MediaCallBackRegister.pfnMD_GetVideoSourceConfiguration = pstNVTMediaCBFunc->pfnMD_GetVideoSourceConfiguration;
    g_MediaCallBackRegister.pfnMD_GetVideoSourceConfigurations = pstNVTMediaCBFunc->pfnMD_GetVideoSourceConfigurations;
    g_MediaCallBackRegister.pfnMD_GetCompatibleVideoSourceConfigurations = pstNVTMediaCBFunc->pfnMD_GetCompatibleVideoSourceConfigurations;
    g_MediaCallBackRegister.pfnMD_GetVideoSourceConfigurationOptions = pstNVTMediaCBFunc->pfnMD_GetVideoSourceConfigurationOptions;
    g_MediaCallBackRegister.pfnMD_AddVideoSourceConfiguration = pstNVTMediaCBFunc->pfnMD_AddVideoSourceConfiguration;
    g_MediaCallBackRegister.pfnMD_RemoveVideoSourceConfiguration = pstNVTMediaCBFunc->pfnMD_RemoveVideoSourceConfiguration;
    g_MediaCallBackRegister.pfnMD_SetVideoSourceConfiguration = pstNVTMediaCBFunc->pfnMD_SetVideoSourceConfiguration;
    g_MediaCallBackRegister.pfnMD_GetVideoEncoderConfiguration = pstNVTMediaCBFunc->pfnMD_GetVideoEncoderConfiguration;
    g_MediaCallBackRegister.pfnMD_GetVideoEncoderConfigurations = pstNVTMediaCBFunc->pfnMD_GetVideoEncoderConfigurations;
    g_MediaCallBackRegister.pfnMD_GetCompatibleVideoEncoderConfigurations = pstNVTMediaCBFunc->pfnMD_GetCompatibleVideoEncoderConfigurations;
    g_MediaCallBackRegister.pfnMD_GetVideoEncoderConfigurationOptions = pstNVTMediaCBFunc->pfnMD_GetVideoEncoderConfigurationOptions;
    g_MediaCallBackRegister.pfnMD_AddVideoEncoderConfiguration = pstNVTMediaCBFunc->pfnMD_AddVideoEncoderConfiguration;
    g_MediaCallBackRegister.pfnMD_RemoveVideoEncoderConfiguration = pstNVTMediaCBFunc->pfnMD_RemoveVideoEncoderConfiguration;
    g_MediaCallBackRegister.pfnMD_SetVideoEncoderConfiguration = pstNVTMediaCBFunc->pfnMD_SetVideoEncoderConfiguration;
    g_MediaCallBackRegister.pfnMD_GetGuaranteedNumberOfVideoEncoderInstances = pstNVTMediaCBFunc->pfnMD_GetGuaranteedNumberOfVideoEncoderInstances;
    g_MediaCallBackRegister.pfnMD_AddPTZConfiguration = pstNVTMediaCBFunc->pfnMD_AddPTZConfiguration;
    g_MediaCallBackRegister.pfnMD_RemovePTZConfiguration = pstNVTMediaCBFunc->pfnMD_RemovePTZConfiguration;
    g_MediaCallBackRegister.pfnMD_GetAudioSourceConfiguration = pstNVTMediaCBFunc->pfnMD_GetAudioSourceConfiguration;
    g_MediaCallBackRegister.pfnMD_GetAudioEncoderConfiguration = pstNVTMediaCBFunc->pfnMD_GetAudioEncoderConfiguration;
    g_MediaCallBackRegister.pfnMD_GetAudioSourceConfigurations = pstNVTMediaCBFunc->pfnMD_GetAudioSourceConfigurations;
    g_MediaCallBackRegister.pfnMD_GetAudioEncoderConfigurations = pstNVTMediaCBFunc->pfnMD_GetAudioEncoderConfigurations;
    g_MediaCallBackRegister.pfnMD_GetCompatibleAudioSourceConfigurations = pstNVTMediaCBFunc->pfnMD_GetCompatibleAudioSourceConfigurations;
    g_MediaCallBackRegister.pfnMD_GetCompatibleAudioEncoderConfigurations = pstNVTMediaCBFunc->pfnMD_GetCompatibleAudioEncoderConfigurations;
    g_MediaCallBackRegister.pfnMD_GetAudioSourceConfigurationOptions = pstNVTMediaCBFunc->pfnMD_GetAudioSourceConfigurationOptions;
    g_MediaCallBackRegister.pfnMD_GetAudioEncoderConfigurationOptions = pstNVTMediaCBFunc->pfnMD_GetAudioEncoderConfigurationOptions;
    g_MediaCallBackRegister.pfnMD_SetAudioSourceConfiguration = pstNVTMediaCBFunc->pfnMD_SetAudioSourceConfiguration;
    g_MediaCallBackRegister.pfnMD_SetAudioEncoderConfiguration = pstNVTMediaCBFunc->pfnMD_SetAudioEncoderConfiguration;
    g_MediaCallBackRegister.pfnMD_AddAudioSourceConfiguration = pstNVTMediaCBFunc->pfnMD_AddAudioSourceConfiguration;
    g_MediaCallBackRegister.pfnMD_AddAudioEncoderConfiguration = pstNVTMediaCBFunc->pfnMD_AddAudioEncoderConfiguration;
    g_MediaCallBackRegister.pfnMD_RemoveAudioSourceConfiguration = pstNVTMediaCBFunc->pfnMD_RemoveAudioSourceConfiguration;
    g_MediaCallBackRegister.pfnMD_RemoveAudioEncoderConfiguration = pstNVTMediaCBFunc->pfnMD_RemoveAudioEncoderConfiguration;

    return NVT_NO_ERROR;
}


/***************************************************************************
函数描述: 对设备云台控制模块所有回调函数指针进行
			    注册,以实现服务端的云台控制功能
入口参数：GOnvif_PTZ_CallbackFunc_S *pstNVTPtzCBFunc : 设备云台控制
				模块回调函数指针结构体的指针
返回值     ：NVT_NO_ERROR: 成功
         		      NVT_SYS_ERROR: 失败
***************************************************************************/
GONVIF_NVT_Ret_E GK_NVT_PTZ_Register(GOnvif_PTZ_CallbackFunc_S *pstNVTPtzCBFunc)
{
    memset(&g_PtzCallBackRegister, 0 ,sizeof(GOnvif_PTZ_CallbackFunc_S));
    g_PtzCallBackRegister.pfnPTZ_GetServiceCapabilities  = pstNVTPtzCBFunc->pfnPTZ_GetServiceCapabilities;
    g_PtzCallBackRegister.pfnPTZ_GetNodes                = pstNVTPtzCBFunc->pfnPTZ_GetNodes;
    g_PtzCallBackRegister.pfnPTZ_GetNode                 = pstNVTPtzCBFunc->pfnPTZ_GetNode;   
    g_PtzCallBackRegister.pfnPTZ_GetConfigurations       = pstNVTPtzCBFunc->pfnPTZ_GetConfigurations;
    g_PtzCallBackRegister.pfnPTZ_GetConfiguration        = pstNVTPtzCBFunc->pfnPTZ_GetConfiguration;
    g_PtzCallBackRegister.pfnPTZ_GetConfigurationOptions = pstNVTPtzCBFunc->pfnPTZ_GetConfigurationOptions;
    g_PtzCallBackRegister.pfnPTZ_SetConfiguration        = pstNVTPtzCBFunc->pfnPTZ_SetConfiguration;
    g_PtzCallBackRegister.pfnPTZ_ContinuousMove          = pstNVTPtzCBFunc->pfnPTZ_ContinuousMove;
    g_PtzCallBackRegister.pfnPTZ_Stop                    = pstNVTPtzCBFunc->pfnPTZ_Stop;
    g_PtzCallBackRegister.pfnPTZ_GetPresets              = pstNVTPtzCBFunc->pfnPTZ_GetPresets;
    g_PtzCallBackRegister.pfnPTZ_SetPreset               = pstNVTPtzCBFunc->pfnPTZ_SetPreset;
    g_PtzCallBackRegister.pfnPTZ_GotoPreset              = pstNVTPtzCBFunc->pfnPTZ_GotoPreset;
    g_PtzCallBackRegister.pfnPTZ_RemovePreset            = pstNVTPtzCBFunc->pfnPTZ_RemovePreset;



    
    g_PtzCallBackRegister.pfnPTZ_AbsoluteMove               = pstNVTPtzCBFunc->pfnPTZ_AbsoluteMove;
    g_PtzCallBackRegister.pfnPTZ_RelativeMove               = pstNVTPtzCBFunc->pfnPTZ_RelativeMove;
    g_PtzCallBackRegister.pfnPTZ_GetNode                    = pstNVTPtzCBFunc->pfnPTZ_GetNode;
    g_PtzCallBackRegister.pfnGetStatus                  = pstNVTPtzCBFunc->pfnGetStatus;
    return NVT_NO_ERROR;
}

/***************************************************************************
函数描述: 对图像控制模块所有回调函数指针进行注册,
			    以实现服务端的图像控制功能
入口参数：GOnvif_IMAGE_CallbackFunc_S *pstNVTImageCBFunc : 图像控制
				回调函数指针结构体的指针
返回值     ：NVT_NO_ERROR: 成功
         		      NVT_SYS_ERROR: 失败
***************************************************************************/
GONVIF_NVT_Ret_E GK_NVT_Image_Register(GOnvif_IMAGE_CallbackFunc_S *pstNVTImageCBFunc)
{
    memset(&g_ImageCallBackRegister, 0 ,sizeof(GOnvif_IMAGE_CallbackFunc_S));
    g_ImageCallBackRegister.pfnIMG_GetServiceCapabilities = pstNVTImageCBFunc->pfnIMG_GetServiceCapabilities;
    g_ImageCallBackRegister.pfnGetImagingSettings = pstNVTImageCBFunc->pfnGetImagingSettings;
    g_ImageCallBackRegister.pfnSetImagingSettings = pstNVTImageCBFunc->pfnSetImagingSettings;
    g_ImageCallBackRegister.pfnGetOptions = pstNVTImageCBFunc->pfnGetOptions;
    g_ImageCallBackRegister.pfnMove = pstNVTImageCBFunc->pfnMove;
    g_ImageCallBackRegister.pfnPTZ_Stop = pstNVTImageCBFunc->pfnPTZ_Stop;
    g_ImageCallBackRegister.pfnGetStatus = pstNVTImageCBFunc->pfnGetStatus;
    g_ImageCallBackRegister.pfnGetMoveOptions = pstNVTImageCBFunc->pfnGetMoveOptions;

    return NVT_NO_ERROR;
}

GONVIF_NVT_Ret_E GK_NVT_Analytics_Register(GOnvif_ANALYTICS_CallbackFunc_S *pstNVTAnalyticsCBFunc)
{
    memset(&g_AnalyticsCallBackRegister, 0 ,sizeof(GOnvif_ANALYTICS_CallbackFunc_S));
    g_AnalyticsCallBackRegister.pfnAN_GetServiceCapabilities = pstNVTAnalyticsCBFunc->pfnAN_GetServiceCapabilities;

    
    g_AnalyticsCallBackRegister.pfnGetSupportedAnalyticsModules = pstNVTAnalyticsCBFunc->pfnGetSupportedAnalyticsModules;
	g_AnalyticsCallBackRegister.pfnGetAnalyticsModules = pstNVTAnalyticsCBFunc->pfnGetAnalyticsModules;
	g_AnalyticsCallBackRegister.pfnGetSupportedRules = pstNVTAnalyticsCBFunc->pfnGetSupportedRules;
	g_AnalyticsCallBackRegister.pfnGetRules = pstNVTAnalyticsCBFunc->pfnGetRules;

	return NVT_NO_ERROR;
}

/***************************************************************************
函数描述: 对设备输入输出模块所有回调函数指针进行
			   注册,以实现服务端的设备输入输出功能
入口参数：GOnvif_DEVICEIO_CallbackFunc_S *pstNVTDeviceIOCBFunc : 设备输
				入输出模块回调函数指针结构体的指针
返回值     ：NVT_NO_ERROR: 成功
         		      NVT_SYS_ERROR: 失败
***************************************************************************/
GONVIF_NVT_Ret_E GK_NVT_DeviceIO_Register(GOnvif_DEVICEIO_CallbackFunc_S *pstNVTDeviceIOCBFunc)
{
    memset(&g_DeviceIOCallBackRegister, 0 ,sizeof(GOnvif_DEVICEIO_CallbackFunc_S));

    g_DeviceIOCallBackRegister.pfnIO_GetVideoSources        = pstNVTDeviceIOCBFunc->pfnIO_GetVideoSources;
    g_DeviceIOCallBackRegister.pfnIO_GetAudioSources        = pstNVTDeviceIOCBFunc->pfnIO_GetAudioSources;
    g_DeviceIOCallBackRegister.pfnIO_GetRelayOutputs        = pstNVTDeviceIOCBFunc->pfnIO_GetRelayOutputs;
    g_DeviceIOCallBackRegister.pfnIO_SetRelayOutputSettings = pstNVTDeviceIOCBFunc->pfnIO_SetRelayOutputSettings;
    g_DeviceIOCallBackRegister.pfnIO_SetRelayOutputState    = pstNVTDeviceIOCBFunc->pfnIO_SetRelayOutputState;
    
    return NVT_NO_ERROR;
}

/***************************************************************************
函数描述: 对设备事件服务模块所有回调函数指针进行
			    注册,以实现服务端的设备事件服务功能
入口参数：GOnvif_EVENT_CallbackFunction_S *pstNVTEventCBFunc : 设备事件
				服务模块回调函数指针结构体的指针
返回值     ：NVT_NO_ERROR: 成功
         		      NVT_SYS_ERROR: 失败
***************************************************************************/
GONVIF_NVT_Ret_E GK_NVT_Event_Register(GOnvif_EVENT_CallbackFunction_S *pstNVTEventCBFunc)
{
    memset(&g_EventCallBackRegister, 0 ,sizeof(GOnvif_EVENT_CallbackFunction_S));
    g_EventCallBackRegister.pfnEV_GetServiceCapabilities = pstNVTEventCBFunc->pfnEV_GetServiceCapabilities;
    g_EventCallBackRegister.pfnEV_Subscribe = pstNVTEventCBFunc->pfnEV_Subscribe;
    g_EventCallBackRegister.pfnEV_Unsubscribe = pstNVTEventCBFunc->pfnEV_Unsubscribe;
    g_EventCallBackRegister.pfnEV_GetEventProperties = pstNVTEventCBFunc->pfnEV_GetEventProperties;
    g_EventCallBackRegister.pfnEV_Renew = pstNVTEventCBFunc->pfnEV_Renew;
    g_EventCallBackRegister.pfnEV_SetSynchronizationPoint = pstNVTEventCBFunc->pfnEV_SetSynchronizationPoint;
    g_EventCallBackRegister.pfnEV_CreatePullPointSubscription = pstNVTEventCBFunc->pfnEV_CreatePullPointSubscription;
    g_EventCallBackRegister.pfnEV_PullMessages = pstNVTEventCBFunc->pfnEV_PullMessages;
    return NVT_NO_ERROR;
}

/*************************************************************************
函数描述: 启动DiscoverServer线程功能,监听并接收处理UDP
			    组播数据,实现设备发现功能和上线下线
入口参数:  无
返回值      :  1: 成功
         		     0 : 失败，系统初始化错误
**************************************************************************/
GONVIF_NVT_Ret_E GK_NVT_Start_UdpSoapService(void)
{
    GK_S32 err;
    pthread_attr_t attr;	
    if(pthread_attr_init(&attr) < 0)
    {
        ONVIF_ERR("UDP server error.\n");
        return NVT_SYS_ERROR;
    }
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

	err = pthread_create(&g_onvif_cls.discID, &attr, nvt_UDP_SoapServer_Thread, NULL);
	if(err != 0)
	{
        ONVIF_ERR("Create UDP server failed.\n");
		return NVT_SYS_ERROR;
	}
	
    ONVIF_INFO("Success to start UDP service in ONVIF.");
    
	return NVT_NO_ERROR;
}


/**********************************************************************
函数描述：启动Http线程服务,处理SOAP消息
入口参数：GK_BOOL isFromWeb : 是否从webServer 中启动
				GK_S32 webSocket : Socket ID
返回值     ：NVT_NO_ERROR: 成功
         		      NVT_SYS_ERROR: 失败，系统初始化错误
**********************************************************************/
GONVIF_NVT_Ret_E GK_NVT_Start_HttpSoapService(GK_BOOL isFromWeb, GK_S32 webSocket)
{
	if(isFromWeb != GK_TRUE && isFromWeb != GK_FALSE)
	{
		ONVIF_ERR("Invalid isFromWeb flag.");
        return NVT_SYS_ERROR;
	}
	g_webFlag = isFromWeb;
	
    memset(&g_http_srv, 0, sizeof(HTTPSRV));
    
	http_bm_sysbuf_init(HTTP_NETBUF_NUM, HTTP_NETBUF_SIZE, HTTP_HDRV_NUM);  
	http_bm_msgbuf_fl_init(HTTP_MSGBUF_NUM);                                
	
	if(http_server_start(&g_http_srv, inet_addr("0.0.0.0"), g_GkIpc_OnvifInf.discov.hostport, HTTP_MAX_CLIENT_NUM, isFromWeb, webSocket) != 0)
    {
        return NVT_SYS_ERROR;
    }

	return NVT_NO_ERROR;
}

/**********************************************************************
函数描述：WebServer中处理SOAP 消息
入口参数：clientFd : 客户端SocketID
				clientIp :	客户端IP 
				clientPort : 客户端端口号 
				clientsize : 数据大小
返回值     ：NVT_NO_ERROR: 成功
         		      NVT_SYS_ERROR: 失败
**********************************************************************/
GONVIF_NVT_Ret_E GK_NVT_SoapProc_InWeb(GK_S32 clientFd, GK_U32 clientIp, GK_U32 clientPort, const GK_CHAR *pszClientStream, GK_U32 clientSize)
{
	if(pszClientStream == NULL || clientSize <= 0)
	{
		ONVIF_ERR("Invalid soap message");
		return NVT_SYS_ERROR;
	}
    GK_S32 ret = 0;
    ret = http_server_soapProc_InWeb(&g_http_srv, clientFd, clientIp, clientPort, pszClientStream, clientSize);
    if(ret == 0)
        return NVT_NO_ERROR;
    else
        return NVT_SYS_ERROR;
}

void GK_NVT_SendHelloBye(ONVIF_HelloBye_E handleType)
{
    if(handleType == GK_Hello)
    {
        nvt_Send_HelloMessage(g_onvif_cls.discFd);
    }
    else if(handleType == GK_Bye)
    {
        nvt_Send_ByeMessage(g_onvif_cls.discFd);
    }
	//config_Save_DiscoverXml(DISCOVER_XML_PATH);
	//config_Save_DeviceXml(DEVICE_XML_PATH);
	//config_Save_MediaXml(MEDIADYNAMIC_XML_PATH);
	
    return;
}

void GK_NVT_SaveXmlConfig(ONVIF_XmlFileType_E xmltype)
{
	pthread_mutex_lock(&g_xmlMutex);
	nvt_XmlSave_Handle(xmltype);
	pthread_mutex_unlock(&g_xmlMutex);

    return ;
}

/********************************* inside API ************************************/
static int nvt_Send_HelloMessage(int fd)
{
    GK_CHAR p_buf[1024*10] = {0};
    GK_S32  offset = 0;
    GK_S32  mlen = sizeof(p_buf);
    GK_CHAR _HwId[1024] = {0};
    #if 0
    sprintf(_HwId, "urn:uuid:2419d68a-2dd2-21b2-a205-%02X%02X%02X%02X%02X%02X", 
        g_GkIpc_OnvifInf.discov.hostmac[0],  g_GkIpc_OnvifInf.discov.hostmac[1],  g_GkIpc_OnvifInf.discov.hostmac[2], 
		g_GkIpc_OnvifInf.discov.hostmac[3],  g_GkIpc_OnvifInf.discov.hostmac[4],  g_GkIpc_OnvifInf.discov.hostmac[5]);
    offset += snprintf(p_buf+offset, mlen-offset,
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" "
		"xmlns:enc=\"http://www.w3.org/2003/05/soap-encoding\" "
		"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
		"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
		"xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
		"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
		"xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" "
		"xmlns:tdn=\"http://www.onvif.org/ver10/network/wsdl\" "
		"xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\" "
		"xmlns:tt=\"http://www.onvif.org/ver10/schema\" "
		"xmlns:wsnt=\"http://docs.oasis-open.org/wsn/b-2\" "
		"xmlns:wstop=\"http://docs.oasis-open.org/wsn/t-1\" "
		"xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\">");

	offset += snprintf(p_buf+offset, mlen-offset, 
		    "<s:Header>"
		        "<wsa:MessageID>%s</wsa:MessageID>"
		        "<wsa:RelatesTo>http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</wsa:RelatesTo>"
		        "<wsa:To s:mustUnderstand=\"true\">urn:schemas-xmlsoap-org:ws:2005:04:discovery</wsa:To>"
		        "<wsa:Action s:mustUnderstand=\"true\">http://schemas.xmlsoap.org/ws/2005/04/discovery/Hello</wsa:Action>"
			"</s:Header>", _HwId);

	offset += snprintf(p_buf+offset, mlen-offset, 
		    "<s:Body>"
		    	"<d:Hello>"
	        		"<wsa:EndpointReference>"
	            		"<wsa:Address>%s</wsa:Address>"
	        		"</wsa:EndpointReference>"
	        		"<d:Types>tdn:NetworkVideoTransmitter</d:Types>",_HwId);

    GK_U8 i = 0;
   	offset += snprintf(p_buf+offset, mlen-offset, "<d:Scopes>");
	for(i = 0; i < g_GkIpc_OnvifInf.discov.Scopecount; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "%s ", g_GkIpc_OnvifInf.discov.Scopes[i]->aszScopeItem);
	}
   	offset += snprintf(p_buf+offset, mlen-offset, "</d:Scopes>");

   	offset += snprintf(p_buf+offset, mlen-offset, 
	   	    		"<d:XAddrs>%s</d:XAddrs>"
	        		"<d:MetadataVersion>4</d:MetadataVersion>"
	        	"</d:Hello>"
	    	"</s:Body>"
        "</s:Envelope>",
        g_GkIpc_OnvifInf.discov.WebServerUrl);
     #else
	 sprintf(_HwId, "uuid:2419d68a-2dd2-21b2-a205-%02X%02X%02X%02X%02X%02X", 
        g_GkIpc_OnvifInf.discov.hostmac[0],  g_GkIpc_OnvifInf.discov.hostmac[1],  g_GkIpc_OnvifInf.discov.hostmac[2], 
		g_GkIpc_OnvifInf.discov.hostmac[3],  g_GkIpc_OnvifInf.discov.hostmac[4],  g_GkIpc_OnvifInf.discov.hostmac[5]);
    offset += snprintf(p_buf+offset, mlen-offset,
        "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
		"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" "
		"xmlns:sc=\"http://www.w3.org/2003/05/soap-encoding\" "
		"xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\" "
		"xmlns:a=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
		"xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\">");
			
	offset += snprintf(p_buf+offset, mlen-offset, 
		    "<s:Header>"
		        "<a:MessageID>%s</a:MessageID>"
		        "<a:To>urn:schemas-xmlsoap-org:ws:2005:04:discovery</a:To>"
		        "<a:Action>http://schemas.xmlsoap.org/ws/2005/04/discovery/Hello</a:Action>"
			"</s:Header>", _HwId);

	offset += snprintf(p_buf+offset, mlen-offset, 
		    "<s:Body>"
		    	"<d:Hello>"
	        		"<a:EndpointReference>"
	            		"<a:Address>%s</a:Address>"
	        		"</a:EndpointReference>"
	        		"<d:Types>dn:NetworkVideoTransmitter</d:Types>",_HwId);

    GK_U8 i = 0;
   	offset += snprintf(p_buf+offset, mlen-offset, "<d:Scopes>");
	#if 1
	for(i = 0; i < g_GkIpc_OnvifInf.discov.Scopecount; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "%s ", g_GkIpc_OnvifInf.discov.Scopes[i]->aszScopeItem);
	}
	#endif
   	offset += snprintf(p_buf+offset, mlen-offset, "</d:Scopes>");

   	offset += snprintf(p_buf+offset, mlen-offset, 
	   	    		"<d:XAddrs>%s</d:XAddrs>"
	        		"<d:MetadataVersion>4</d:MetadataVersion>"
	        	"</d:Hello>"
	    	"</s:Body>"
        "</s:Envelope>",
        g_GkIpc_OnvifInf.discov.WebServerUrl);

	 
	 #endif
	ONVIF_INFO("%s\n",p_buf);
	 struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(MULTICAST_ADDRESS);
	addr.sin_port = htons(MULTICAST_PORT);
	
	GK_S32 sendLen = 0;
	GK_S32 times   = 0;
	while(times < 3)
	{
	    sendLen = sendto(fd, p_buf, offset, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
		if(sendLen != offset)
		{
			ONVIF_ERR("Fail to send Hello message: Length: %d | send: %d", offset, sendLen);
		}
		//ONVIF_ERR("success to send Hello message: Length: %d | send: %d", offset, sendLen);
	    times++;	    
		usleep(100000);
    }
    //ONVIF_INFO(" Try to send HELLO message three times.");

    return 0;
}

static int nvt_Send_ByeMessage(int fd)
{
    GK_CHAR p_buf[1024*10];
    GK_S32  offset = 0;
    GK_S32  mlen = sizeof(p_buf);
    GK_CHAR _HwId[1024] = {0};
   #if 0
    sprintf(_HwId, "urn:uuid:2419d68a-2dd2-21b2-a205-%02X%02X%02X%02X%02X%02X",
		g_GkIpc_OnvifInf.discov.hostmac[0],  g_GkIpc_OnvifInf.discov.hostmac[1],  g_GkIpc_OnvifInf.discov.hostmac[2],
		g_GkIpc_OnvifInf.discov.hostmac[3],  g_GkIpc_OnvifInf.discov.hostmac[4],  g_GkIpc_OnvifInf.discov.hostmac[5]);
    offset += snprintf(p_buf+offset, mlen-offset,
        "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" "
		"xmlns:enc=\"http://www.w3.org/2003/05/soap-encoding\" "
		"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
		"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
		"xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
		"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
		"xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" "
		"xmlns:tdn=\"http://www.onvif.org/ver10/network/wsdl\" "
		"xmlns:ds=\"http://www.w3.org/2000/09/xmldsig#\" "
		"xmlns:tt=\"http://www.onvif.org/ver10/schema\" "
		"xmlns:wsnt=\"http://docs.oasis-open.org/wsn/b-2\" "
		"xmlns:wstop=\"http://docs.oasis-open.org/wsn/t-1\" "
		"xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\">");

	offset += snprintf(p_buf+offset, mlen-offset,
		    "<s:Header>"
		        "<wsa:MessageID>%s</wsa:MessageID>"
		        "<wsa:RelatesTo>http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</wsa:RelatesTo>"
		        "<wsa:To s:mustUnderstand=\"true\">urn:schemas-xmlsoap-org:ws:2005:04:discovery</wsa:To>"
		        "<wsa:Action s:mustUnderstand=\"true\">http://schemas.xmlsoap.org/ws/2005/04/discovery/Bye</wsa:Action>"
			"</s:Header>",_HwId);

	offset += snprintf(p_buf+offset, mlen-offset, 
		    "<s:Body>"
		    	"<d:Bye>"
	            	"<wsa:EndpointReference>"
	                	"<wsa:Address>%s</wsa:Address>"
	            	"</wsa:EndpointReference>"
	        		"<d:Types>tdn:NetworkVideoTransmitter</d:Types>",_HwId);
            
    GK_U8 i = 0;
   	offset += snprintf(p_buf+offset, mlen-offset, "<d:Scopes>");
	for (i = 0; i < g_GkIpc_OnvifInf.discov.Scopecount; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "%s ", g_GkIpc_OnvifInf.discov.Scopes[i]->aszScopeItem);
	}
   	offset += snprintf(p_buf+offset, mlen-offset, "</d:Scopes>");

   	offset += snprintf(p_buf+offset, mlen-offset, 
   	    			"<d:XAddrs>%s</d:XAddrs>"
        			"<d:MetadataVersion>1</d:MetadataVersion>"
        		"</d:Bye>"
    		"</s:Body>"
		"</s:Envelope>",
        g_GkIpc_OnvifInf.discov.WebServerUrl);
#else
sprintf(_HwId, "uuid:2419d68a-2dd2-21b2-a205-%02X%02X%02X%02X%02X%02X", 
	   g_GkIpc_OnvifInf.discov.hostmac[0],	g_GkIpc_OnvifInf.discov.hostmac[1],  g_GkIpc_OnvifInf.discov.hostmac[2], 
	   g_GkIpc_OnvifInf.discov.hostmac[3],	g_GkIpc_OnvifInf.discov.hostmac[4],  g_GkIpc_OnvifInf.discov.hostmac[5]);
   offset += snprintf(p_buf+offset, mlen-offset,
	   "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?>"
	   "<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" "
	   "xmlns:sc=\"http://www.w3.org/2003/05/soap-encoding\" "
	   "xmlns:dn=\"http://www.onvif.org/ver10/network/wsdl\" "
	   "xmlns:a=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
	   "xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\">");
		   
   offset += snprintf(p_buf+offset, mlen-offset, 
		   "<s:Header>"
			   "<a:MessageID>%s</a:MessageID>"
			   "<a:To>urn:schemas-xmlsoap-org:ws:2005:04:discovery</a:To>"
			   "<a:Action>http://schemas.xmlsoap.org/ws/2005/04/discovery/Hello</a:Action>"
		   "</s:Header>", _HwId);

   offset += snprintf(p_buf+offset, mlen-offset, 
		   "<s:Body>"
			   "<d:Bye>"
				   "<a:EndpointReference>"
					   "<a:Address>%s</a:Address>"
				   "</a:EndpointReference>"
				   "<d:Types>dn:NetworkVideoTransmitter</d:Types>",_HwId);

   GK_U8 i = 0;
   offset += snprintf(p_buf+offset, mlen-offset, "<d:Scopes>");
   for(i = 0; i < g_GkIpc_OnvifInf.discov.Scopecount; i++)
   {
	   offset += snprintf(p_buf+offset, mlen-offset, "%s ", g_GkIpc_OnvifInf.discov.Scopes[i]->aszScopeItem);
   }
   offset += snprintf(p_buf+offset, mlen-offset, "</d:Scopes>");

   offset += snprintf(p_buf+offset, mlen-offset, 
				   "<d:XAddrs>%s</d:XAddrs>"
				   "<d:MetadataVersion>4</d:MetadataVersion>"
			   "</d:Bye>"
		   "</s:Body>"
	   "</s:Envelope>",
	   g_GkIpc_OnvifInf.discov.WebServerUrl);


#endif
    ONVIF_INFO("%s\n",p_buf);
    struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr(MULTICAST_ADDRESS);
	addr.sin_port = htons(MULTICAST_PORT);
	
	GK_S32 sendLen = 0;
	GK_S32 times   = 0;
	while(times < 3)
	{
	    sendLen = sendto(fd, p_buf, offset, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
		if(sendLen != offset)
		{
			ONVIF_ERR("Fail to send BYE message: Length: %d | send: %d", offset, sendLen);
		}
		times++;
		usleep(100000);
	}
    ONVIF_INFO(" Try to send BYE message three times.");
    
    return 0;
}


/**********************************************************************
函数描述：保存XML 配置
入口参数:    GK_S32 xmlType : XML 类型
返回值     ：无
**********************************************************************/
static void nvt_XmlSave_Handle(GK_S32 xmlType)
{
	if(xmlType == XML_DISCOVRY)
	{
		config_Save_DiscoverXml(DISCOVER);
	}
	else if(xmlType == XML_DEVICE)
	{
    	config_Save_DeviceXml(DEVICES);
	}
	else if(xmlType == XML_MEIDA)
	{
		config_Save_MediaXml(MEDIADYNAMIC);
	}
	else if(xmlType == XML_ALL)
	{
		config_Save_DiscoverXml(DISCOVER);
        config_Save_DeviceXml(DEVICES);
		config_Save_MediaXml(MEDIADYNAMIC);
	}
	
    return;
}

static GK_S32 nvt_UDP_SoapServer_Init(void)
{
	GK_S32 s32Buflen = 8192;	
	GK_S32 socketfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(socketfd < 0)
	{
		ONVIF_ERR("socket error.\n");
		return NVT_SYS_ERROR;
	}

    struct sockaddr_in addr_in;
	addr_in.sin_family = AF_INET;
    addr_in.sin_addr.s_addr = htons(INADDR_ANY);
	addr_in.sin_port = htons(MULTICAST_PORT);

    GK_S32 opt = 1;
    if (setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (char*)&opt, sizeof(opt)))
    {
        ONVIF_ERR("setsockopt so reusedaddr failed!\n");
        goto UDP_init_error;
    }
	if (bind(socketfd,(struct sockaddr *)&addr_in,sizeof(addr_in)) == -1)
	{
        ONVIF_ERR("bind failed.\n");
        goto UDP_init_error;
    }

	if (setsockopt(socketfd, SOL_SOCKET, SO_SNDBUF, (char*)&s32Buflen, sizeof(int)))
	{
		ONVIF_ERR("setsockopt so sendbuf failed!\n");
        goto UDP_init_error;
	}

	if (setsockopt(socketfd, SOL_SOCKET, SO_RCVBUF, (char*)&s32Buflen, sizeof(int)))
	{
		ONVIF_ERR("setsockopt so rcvbuf failed!\n");
        goto UDP_init_error;
	}

	return socketfd;
	
UDP_init_error:
    close(socketfd);
    return NVT_SYS_ERROR;
}

static GK_S32 nvt_UDP_SoapServer_Start()
{
	GK_S32 ret;	
    GK_BOOL scopeIsValid = GK_TRUE;
	GK_CHAR rxbuf[1024*4] = {'\0'};
    static GK_S32 error_cnt = 0;
    struct sockaddr_in *oldNet = NULL;
    struct sockaddr_in *newNet = NULL;  
	ret = g_stDeviceAdapter.GetNetworkIsRunning(ETHERNET_0);
	if(ret > 0)	
	{
		if(IsEth0 == GK_FALSE)
		{
			error_cnt = 0;
		}

		memset(&addr_wlan0_old, 0, sizeof(addr_wlan0_old));
		memset(&addr_wlan0_new, 0, sizeof(addr_wlan0_new));

		IsEth0 = GK_TRUE;
		ONVIF_DeviceNet_S stNetInfor;
		memset(&stNetInfor, 0, sizeof(ONVIF_DeviceNet_S));
		#ifdef MODULE_SUPPORT_ZK_WAPI
		ret = g_stDeviceAdapter.GetNetworkInfor_loca(ETHERNET_0, &stNetInfor);
		#else
		ret = g_stDeviceAdapter.GetNetworkInfor(ETHERNET_0, &stNetInfor);
		#endif
		if(ret != 0)
		{
			ONVIF_ERR("Fail to get net info.");
			return -1;
		}
		addr_eth0_new.sin_addr.s_addr = inet_addr(stNetInfor.ipAddr);
	    oldNet = &addr_eth0_old;
	    newNet = &addr_eth0_new;
		#ifdef MODULE_SUPPORT_ZK_WAPI
        ret = g_stDeviceAdapter.GetNetworkInfor(ETHERNET_0, &stNetInfor);
		#endif
	    nvt_syn_net_info(stNetInfor, ETHERNET_0);
    }
    else if(ret == 0)	
    {
		if(IsEth0 == GK_TRUE)
		{
			error_cnt = 0;
		}
		memset(&addr_eth0_old, 0, sizeof(addr_eth0_old));
		memset(&addr_eth0_new, 0, sizeof(addr_eth0_new));
		IsEth0 = GK_FALSE;
		GK_CHAR wifiName[10] = {0};
		ret = g_stDeviceAdapter.GetNetworkWifiNICName(wifiName, 20);
		if(ret != 0)
		{
			//ONVIF_ERR("Fail to get name of wifi NIC");
			return -1;
		}
		ONVIF_DeviceNet_S stNetInfor;
		memset(&stNetInfor, 0, sizeof(ONVIF_DeviceNet_S));
		ret = g_stDeviceAdapter.GetNetworkInfor(wifiName, &stNetInfor);
		if(ret != 0)
		{
			ONVIF_ERR("Fail to get net info.");
			return -1;
		}	
		addr_wlan0_new.sin_addr.s_addr = inet_addr(stNetInfor.ipAddr);
	    oldNet = &addr_wlan0_old;
	    newNet = &addr_wlan0_new;
	    nvt_syn_net_info(stNetInfor, wifiName);
    }
    else
    {  
		ONVIF_ERR("network connect error.\n");
		return -1;
    }
    // check: Does IP changes or netcard changes or connected timeout ?
	if((oldNet->sin_addr.s_addr != newNet->sin_addr.s_addr) || error_cnt > UDP_SELECT_TIMEOUT_TIMES)
	{
		// these cases will enter here: 1.select timeout in same nic; 2. ip of one nic which is used changes; 3. change nic.		
		if(error_cnt > UDP_SELECT_TIMEOUT_TIMES)
			ONVIF_INFO("Connected timeout(%d*%d seconds), rebuild search service(%s).", UDP_SELECT_TIMEOUT, UDP_SELECT_TIMEOUT_TIMES, IsEth0 ? "eth0":"wifi");
		else
			ONVIF_INFO("IP of current netcard changes or nic changes.");
			
		error_cnt = 0;
		ONVIF_INFO("Success to get %s IP[%s]", IsEth0 ? "eth0":"wifi", inet_ntoa(newNet->sin_addr));
		
        if(g_onvif_cls.discFd)
        {
            if(g_onvif_cls.multicast_opt.imr_multiaddr.s_addr){
                ret = setsockopt(g_onvif_cls.discFd, IPPROTO_IP, IP_DROP_MEMBERSHIP, (char *)&g_onvif_cls.multicast_opt, sizeof(struct ip_mreq));
                if(ret < 0)
                {
                    ONVIF_ERR("del multicast error\n");
                }
                memset(&g_onvif_cls.multicast_opt, 0, sizeof(struct ip_mreq));
            }
			close(g_onvif_cls.discFd);
			g_onvif_cls.discFd = 0;
		}
		g_onvif_cls.discFd = nvt_UDP_SoapServer_Init();		
		if(g_onvif_cls.discFd < 0)
		{
			ONVIF_ERR("Fail to initialize UDP soap server.");
        	return -1;
		}
		ret = nvt_add_to_multicast(newNet);
		if(ret < 0)
		{
			ONVIF_INFO("Fail to add to multicast.");
			return -1;
		}
		if(oldNet != NULL && newNet != NULL)
		{
			memcpy(oldNet, newNet, sizeof(struct sockaddr_in));
		}
		GK_NVT_SendHelloBye(GK_Hello);
	}

	fd_set fdSet;
	FD_ZERO(&fdSet);
	FD_SET(g_onvif_cls.discFd, &fdSet);
	struct timeval tv;
	tv.tv_sec = UDP_SELECT_TIMEOUT;
	tv.tv_usec = 0;
	ret = select(g_onvif_cls.discFd+1, &fdSet, NULL, NULL, &tv);
	if(ret == 0)	
	{		
		//ONVIF_INFO("Select timeout.");
		error_cnt++;
		return 0;
	}
	else if(ret < 0)
	{
		//ONVIF_ERR("Fail to be connected.");
		error_cnt++;
		return -2;
	}
	ONVIF_INFO("client connected.");
	GK_S32 rxLen;
    GK_S32 sockAddrLen;
	struct sockaddr_in addr;
	sockAddrLen = sizeof(struct sockaddr_in);
	rxLen = recvfrom(g_onvif_cls.discFd, rxbuf, sizeof(rxbuf), 0, (struct sockaddr *)&addr, (socklen_t*)&sockAddrLen);
	//ONVIF_INFO("\n============================================\n%s\n============================================", rxbuf);
	if(rxLen < 0)
	{
		ONVIF_ERR("Fail to receive search message.");
		return -2;
	}
	GK_U32 clientIP   = addr.sin_addr.s_addr;
	GK_U32 clientPort = addr.sin_port;
	    
	GK_CHAR messageID[128];
	GK_CHAR scopeItem[256];
	memset(messageID,0,sizeof(messageID));
	memset(scopeItem,0,sizeof(scopeItem));
	XMLN *p_root = xml_stream_parse(rxbuf, rxLen);
	if(p_root == NULL)
	{
		ONVIF_ERR("Fail to parse xml string of search message.");
	}
	XMLN *p_envelope = p_root;
	if(p_envelope)
	{
		XMLN *p_header = soap_get_child_node(p_envelope, "Header");
		if(p_header)
		{
			XMLN *p_messageID = soap_get_child_node(p_header, "MessageID");
			if(p_messageID && p_messageID->data)
			{
				strncpy(messageID, p_messageID->data, 127);
			}
		}
		XMLN *p_body = soap_get_child_node(p_envelope, "Body");
		if(p_body)
		{
			XMLN *p_probe = soap_get_child_node(p_body, "Probe");
			if(p_probe) 
			{		
                XMLN *p_scope = soap_get_child_node(p_probe, "Scopes");				
                if(p_scope != NULL && p_scope->data != NULL)
                {
                    strncpy(scopeItem, p_scope->data, 255);
                }
                XMLN *p_scopeIsValid = xml_attr_get(p_scope, "MatchBy");
                if(p_scopeIsValid && p_scopeIsValid->data)
                {
                    if(soap_match_tag(p_scopeIsValid->data, "InvalidMatchRule") == 0)
                        scopeIsValid = GK_FALSE;
                    else
                        scopeIsValid = GK_TRUE;
                }
                else
                {
                    scopeIsValid = GK_TRUE;
                }
				XMLN *p_type = soap_get_child_node(p_probe, "Types");
				if(p_type && p_type->data)
				{
					char ip[16] = {0};
                    GK_CHAR *deviceType = strchr(p_type->data, ':');
					sprintf(ip, "%s", (char *)inet_ntoa(addr.sin_addr));
					
                    if(deviceType && deviceType+1)
						ONVIF_INFO("[%s:%d]Probe %s\n", ip, ntohs(clientPort), deviceType+1);
					else
						ONVIF_INFO("[%s:%d]Probe\n", ip, ntohs(clientPort));
					if(soap_match_tag(p_type->data, "NetworkVideoTransmitter") == 0)
					{
						netcam_net_autotracking(ip);
                        nvt_Probe(messageID, scopeItem, scopeIsValid, g_onvif_cls.discFd, clientIP, clientPort);                        
					}
					else if(soap_match_tag(p_type->data, "Device") == 0)
					{
                        ;// Probe Device
					}
					else if(soap_match_tag(p_type->data, "NetworkVideoDisplay") == 0)
					{
                        ;// Probe NetworkVideoDisplay
					}
					else if(soap_match_tag(p_type->data, "hpDevice"))
					{
                        ;// Probe hpDevice
                    }
				}
				else
				{
					//ONVIF_ERR("*********Probe************\n");
					nvt_Probe(messageID, scopeItem, scopeIsValid, g_onvif_cls.discFd, clientIP, clientPort);
					//GK_NVT_SendHelloBye(GK_Hello);
				    //ONVIF_ERR("*********Device************\n");
				}
			}
			XMLN *p_hello = soap_get_child_node(p_body, "Hello");
			if(p_hello)
			{
				ONVIF_INFO("Service[%s:%d] is online\n", inet_ntoa(addr.sin_addr), ntohs(clientPort));
			}
			XMLN *p_bye = soap_get_child_node(p_body, "Bye");
			if(p_bye)
			{
				ONVIF_INFO("Service[%s:%d] is offline\n", inet_ntoa(addr.sin_addr), ntohs(clientPort));
			}
		}
	}
	xml_node_del(p_root);

	return 0;
}

static void *nvt_UDP_SoapServer_Thread(void *argv)
{
    sdk_sys_thread_set_name("nvt_UDP_SoapServer_Thread");
    GK_S32 ret = 0;
    memset(&addr_eth0_old, 0, sizeof(struct sockaddr_in));
    memset(&addr_eth0_new, 0, sizeof(struct sockaddr_in));
    memset(&addr_wlan0_old, 0, sizeof(struct sockaddr_in));
    memset(&addr_wlan0_new, 0, sizeof(struct sockaddr_in));
	g_onvif_cls.isDiscoverable = GK_TRUE;
	while(g_onvif_cls.isDiscoverable)
	{
		ret = nvt_UDP_SoapServer_Start();
		if(ret == -1)
		{
			sleep(3);
		}
	}
	ONVIF_INFO("nvt_UDP_SoapServer_Thread will exit(%d).", g_onvif_cls.isDiscoverable);
	g_onvif_cls.discID = 0;

	return NULL;
}

/**********************************************************************
函数描述：停止Http 线程服务
入口参数：GK_S32 signal : 信号
返回值     :     无
**********************************************************************/
static void nvt_UDP_HTTP_SoapServer_Exit(GK_S32 signal)
{
	if(signal != SIGINT && signal != SIGQUIT && signal != SIGTERM)
	{
		return;
	}

	http_server_deinit(&g_http_srv,g_webFlag);  
	http_bm_msgbuf_fl_deinit();                 
	http_bm_sysbuf_deinit();                   

	g_onvif_cls.isDiscoverable = 0;
	while(g_onvif_cls.discID != 0)
	{
		usleep(1000);
	}

	close(g_onvif_cls.discFd);
	g_onvif_cls.discFd = -1;

	return;

}

static GK_S32 nvt_add_to_multicast(struct sockaddr_in *pstAddr)
{
	GK_S32 msgLoop = 0;
	struct ip_mreq mreq;
	memset(&mreq, 0, sizeof(struct ip_mreq));
	mreq.imr_multiaddr.s_addr = inet_addr(MULTICAST_ADDRESS);
	mreq.imr_interface.s_addr = pstAddr->sin_addr.s_addr;	
	if(setsockopt(g_onvif_cls.discFd, IPPROTO_IP, IP_MULTICAST_LOOP, (void *)&msgLoop, sizeof(msgLoop)) < 0)
	{
		ONVIF_ERR("Fail to do IP_MULTICAST_LOOP");
		return -1;
	}
	if(setsockopt(g_onvif_cls.discFd, IPPROTO_IP, IP_ADD_MEMBERSHIP, (char *)&mreq, sizeof(mreq)) < 0)
	{
		ONVIF_INFO("%s netcard is not ready, fail to add %s ip[%s] to multicast[239.255.255.250](new fd)", IsEth0 ? "eth0":"wifi", IsEth0 ? "eth0":"wifi",inet_ntoa(pstAddr->sin_addr));
		return -1;
	}
	ONVIF_INFO("Success to add %s ip[%s] to multicast[239.255.255.250](new fd)", IsEth0 ? "eth0":"wifi", inet_ntoa(pstAddr->sin_addr));

	return 0;
}


static GK_S32 nvt_syn_net_info(ONVIF_DeviceNet_S stNetInfo, GK_CHAR *nicName)
{
	if(nicName == NULL)
	{
		ONVIF_ERR("Invalid paramters.");
		return -1;
	}
	if(strcmp(g_GkIpc_OnvifInf.discov.nicName, nicName) != 0)
	{
		memset(g_GkIpc_OnvifInf.discov.nicName, 0, NIC_NAME_LENTH);
		strncpy(g_GkIpc_OnvifInf.discov.nicName, nicName, NIC_NAME_LENTH-1);
	}
	memset(g_GkIpc_OnvifInf.discov.hostip, 0, IPV4_STR_LENGTH); 	
	memset(g_GkIpc_OnvifInf.discov.WebServerUrl, 0, LEN_WEBSERVICE_URL);
	
	strncpy(g_GkIpc_OnvifInf.discov.hostip, stNetInfo.ipAddr, IPV4_STR_LENGTH-1);
	snprintf(g_GkIpc_OnvifInf.discov.WebServerUrl, LEN_WEBSERVICE_URL, "http://%s:%d/onvif/device_service", g_GkIpc_OnvifInf.discov.hostip, g_GkIpc_OnvifInf.discov.hostport);
	strncpy(g_GkIpc_OnvifInf.discov.hostmac, stNetInfo.macAddr, 6);
#if 0
	ONVIF_INFO("WebServerUrl: %s", g_GkIpc_OnvifInf.discov.WebServerUrl);
	ONVIF_INFO("mac: %02X%02X%02X%02X%02X%02X",
		g_GkIpc_OnvifInf.discov.hostmac[0], g_GkIpc_OnvifInf.discov.hostmac[1], g_GkIpc_OnvifInf.discov.hostmac[2], 
		g_GkIpc_OnvifInf.discov.hostmac[3], g_GkIpc_OnvifInf.discov.hostmac[4], g_GkIpc_OnvifInf.discov.hostmac[5]);
#endif
	return 0;
}

static GK_S32 nvt_Probe(GK_CHAR *messageID,GK_CHAR *scopeItem, GK_BOOL scopeVaild, GK_S32 fd, GK_U32 ip, GK_U32 port)
{
	if (messageID == NULL || g_GkIpc_OnvifInf.discov.discoverymode == DiscoveryMode_NonDiscoverable)
	{
		return -1;
	}
    if(scopeItem[0] != '\0')
    {
        GK_BOOL match = GK_FALSE;
        GK_CHAR *delimPos = NULL;
        GK_CHAR *p = scopeItem;
        delimPos = strtok(p, " ");
        GK_U8 i = 0;
        for(i = 0; i < g_GkIpc_OnvifInf.discov.Scopecount; i++)
        {
            if(GK_TRUE == devMng_MatchString(delimPos, g_GkIpc_OnvifInf.discov.Scopes[i]->aszScopeItem))
            {
               match = GK_TRUE;
               break;
            }
        }
        
        while((delimPos = strtok(NULL, " ")))
        {
            for(i = 0;i < g_GkIpc_OnvifInf.discov.Scopecount; i++)
            {
                if (GK_TRUE == devMng_MatchString(delimPos, g_GkIpc_OnvifInf.discov.Scopes[i]->aszScopeItem))
                {
                   match = GK_TRUE;
                   break;
                }
            }
            if (i < g_GkIpc_OnvifInf.discov.Scopecount)
            {
                break;
            }
        }

        if(GK_FALSE == match)
        {
            ONVIF_ERR("probe: scope does not matched.");
            return -1;
        }
    }
    
    GK_CHAR EP_Addr[1024] = {0};
    sprintf(EP_Addr, "uuid:2419d68a-2dd2-21b2-a205-%02X%02X%02X%02X%02X%02X",
    	g_GkIpc_OnvifInf.discov.hostmac[0], g_GkIpc_OnvifInf.discov.hostmac[1], g_GkIpc_OnvifInf.discov.hostmac[2], 
    	g_GkIpc_OnvifInf.discov.hostmac[3], g_GkIpc_OnvifInf.discov.hostmac[4], g_GkIpc_OnvifInf.discov.hostmac[5]);
    GK_CHAR sendBuffer[1024 * 10];
    memset(sendBuffer, 0, sizeof(sendBuffer));
	GK_S32  bufferLen = sizeof(sendBuffer);

	GK_S32 offset = snprintf(sendBuffer, bufferLen,
	    "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
		"<s:Envelope xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" "
		"xmlns:enc=\"http://www.w3.org/2003/05/soap-encoding\" "
		"xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
		"xmlns:xsd=\"http://www.w3.org/2001/XMLSchema\" "
		"xmlns:wsa=\"http://schemas.xmlsoap.org/ws/2004/08/addressing\" "
		"xmlns:wsa5=\"http://www.w3.org/2005/08/addressing\" "
		"xmlns:d=\"http://schemas.xmlsoap.org/ws/2005/04/discovery\" "
		"xmlns:tdn=\"http://www.onvif.org/ver10/network/wsdl\" "
		"xmlns:tt=\"http://www.onvif.org/ver10/schema\" "
		"xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\">");

    if(scopeVaild == GK_TRUE)
    {
    	offset += snprintf(sendBuffer+offset, bufferLen-offset, 
    	    "<s:Header>"
			    "<wsa:MessageID>%s</wsa:MessageID>"
			    "<wsa:RelatesTo>%s</wsa:RelatesTo>"
			    "<wsa:To s:mustUnderstand=\"true\">http://schemas.xmlsoap.org/ws/2004/08/addressing/role/anonymous</wsa:To>"
			    "<wsa:Action s:mustUnderstand=\"true\">http://schemas.xmlsoap.org/ws/2005/04/discovery/ProbeMatches</wsa:Action>"
			"</s:Header>", 
			EP_Addr, messageID);

        offset += snprintf(sendBuffer+offset, bufferLen-offset,
            "<s:Body>"
                "<d:ProbeMatches><d:ProbeMatch>"
                "<wsa:EndpointReference>"
                "<wsa:Address>%s</wsa:Address>"
                "</wsa:EndpointReference>"
            "<d:Types>tdn:NetworkVideoTransmitter</d:Types>",
            EP_Addr);
            
        GK_U8 i = 0;
        offset += snprintf(sendBuffer+offset, bufferLen-offset, "<d:Scopes>");
        for (i = 0; i < g_GkIpc_OnvifInf.discov.Scopecount; i++)
        {
            offset += snprintf(sendBuffer+offset, bufferLen-offset, "%s ", g_GkIpc_OnvifInf.discov.Scopes[i]->aszScopeItem);
        }
        offset += snprintf(sendBuffer+offset, bufferLen-offset, "</d:Scopes>");
        offset += snprintf(sendBuffer+offset, bufferLen-offset, 
            "<d:XAddrs>%s</d:XAddrs>"
            "<d:MetadataVersion>1</d:MetadataVersion>"
            "</d:ProbeMatch></d:ProbeMatches>",
            g_GkIpc_OnvifInf.discov.WebServerUrl);

    }
    else
    {
    	offset += snprintf(sendBuffer+offset, bufferLen-offset,
    	    "<s:Header>"
			    "<wsa:RelatesTo>%s</wsa:RelatesTo>"
			    "<wsa:Action s:mustUnderstand=\"true\">http://schemas.xmlsoap.org/ws/2005/04/discovery/fault</wsa:Action>"
			"</s:Header>",
			messageID);

        offset += snprintf(sendBuffer+offset, bufferLen-offset,
            "<s:Body>"
            "<s:Fault>"
                "<s:Code>"
                    "<s:Value>s:Sender</s:Value>"
                    "<s:Subcode>"
                        "<s:Value>d:MatchingRuleNotSupported</s:Value>"
                    "</s:Subcode>"
                "</s:Code>"
                "<s:Reason>"
                    "<s:Text xml:lang=\"en\">the matching rule specified is not supported.</s:Text>"
                "</s:Reason>"
            "</s:Fault>");
    }
    offset += snprintf(sendBuffer+offset, bufferLen-offset,
        "</s:Body>"
        "</s:Envelope>");

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = ip;	
	addr.sin_port = port;		
    GK_S32 txlen;
	//ONVIF_INFO("\n+++++++++++++++++++++++++++++++++++++++\n%s\n+++++++++++++++++++++++++++++++++++++++", sendBuffer);
	GK_S32 times = 0;
	while(times < 3)
	{
		txlen = sendto(fd, sendBuffer, offset, 0, (struct sockaddr *)&addr, sizeof(struct sockaddr_in));
		if(txlen != offset)
		{
			ONVIF_ERR("probe response: Length of response message: %d | send: %d", offset, txlen);
		}
		times++;
		usleep(100000);
	}	
	ONVIF_INFO("Success to send response message three times.");
	return 0;
}
