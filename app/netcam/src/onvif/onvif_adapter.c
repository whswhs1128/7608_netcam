/*!
*****************************************************************************
** \file        applications/netcam/onvif/src/onvif_adapter.c
**
** \version     $Id$
**
** \brief       onvif adapter
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************

#include "onvif_adapter.h"
#include "cfg_com.h"
#include "sdk_enc.h"
#include "sdk_isp.h"
#include "sdk_network.h"
#include "netcam_api.h"

#define LINUX_ONVIF_CFG_DIR                 "/opt/custom/onvif/"
#define LINUX_ONVIF_CFG_DEF_DIR             "/usr/local/defcfg/onvif/"
#define IMAGING_XML_DEF_PATH		        "imaging.xml"
#define MEDIASTATIC_XML_DEF_PATH	        "mediastatic.xml"
#define MEDIADYNAMIC_XML_DEF_PATH	        "mediadynamic.xml"
#define DEVICE_XML_DEF_PATH					"device.xml"
#define DISCOVER_XML_DEF_PATH				"discover.xml"
#define PTZ_XML_DEF_PATH                    "ptz.xml"
#define PTZ_PRESET_CFG_DEF_PATH             "ptz_preset.cfg"

#ifdef MODULE_SUPPORT_AF
#define PTZ_ZOOM_CONTROL
#endif

char onvif_file[][32] =
			            {
			                "device.xml",
			                "discover.xml",
			                "imaging.xml",
			                "mediadynamic.xml",
			                "mediastatic.xml",
			                "ptz.xml",
			            };


static int mac_to_array(unsigned char *macStr, char *macArr);
static int onvif_get_resolution(GONVIF_MEDIA_VideoResolution_S **optArr, int streamID, int *maxOpt);

//device
static int Device_Network_GetMac(char *netCardName, char *macStr);
static int Device_Network_GetIP(char *netCardName, struct sockaddr *addr);
static int Device_Network_SetIP(char *netCardName, char *ipStr);
static int Device_Network_SetMask(char *netCardName, char *maskStr);
static int Device_Network_SetDNS(char *netCardName, ONVIF_DeviceNet_S netInfor);
static int Device_Network_GetNTP(ONVIF_DeviceNTP_S *ntpInfor);
static int Device_Network_SetNTP(ONVIF_DeviceNTP_S ntpInfor);
static int Device_Network_SetGateway(char *netCardName, char *gwStr);
static int Device_Network_GetMTU(char *netCardName, int *mtu);
static int Device_Network_SetMTU(char *netCardName, int mtu);
static int Device_Network_SetDHCP(char *netCardName, int isDHCP);
static int Device_Network_GetIsRunning(char *netCardName);
static int Device_Network_GetWifiNICName(char *netCardName, int nameLen);
static int Device_Network_GetInfor(char *netCardName, ONVIF_DeviceNet_S *pstNicInfor);
static int Device_Network_SetInfor(char *netCardName, ONVIF_DeviceNet_S stNicInfor);
static int Device_System_SetFactoryDefault(ONVIF_DeviceFactory_E enFactory);
static int Device_System_GetDeviceInformation(char *netCardName, GONVIF_DEVMNG_GetDeviceInformation_Res_S *pstADP_GetDeviceInformationRes);
static int Device_system_GetDateAndTime(char *timeBuf, int *timeZone);
static int Device_System_SetDateAddTime(char *timeBuf, int timeZone);
static int Device_System_SetReboot(int delayTime);
static int Device_System_GetUsers(GONVIF_DEVMNG_GetUsers_Res_S *pstUsersCfg);
static int Device_System_CreateUsers(GONVIF_DEVMNG_User_S userCfg);
static int Device_System_DeleteUsers(GK_CHAR *userName);
static int Device_System_SetUser(GONVIF_DEVMNG_User_S userCfg);

static int Device_System_SaveFile(ONVIF_FILE_TYPE type,char *file_name);

static int Device_System_GetFile(ONVIF_FILE_TYPE type,char *buffer,int *len);

//media
static int Media_GetVideoEncoderInfor(ONVIF_MediaVideoEncoderInfor_S *pstEncodeInfor, int encodeNum);
static int Media_GetAudioEncoderInfor(ONVIF_MediaAudioEncoderInfor_S *pstEncodeInfor, int encodeNum);
static int Media_GetStreamUri(unsigned char streamID, GONVIF_MEDIA_StreamType_E enStreamType, char *pstStreamUri, int uriLen);
static int Media_SetResolution(int streamID, int width, int height, int encodetype);
static int Media_SetFrameRate(int streamID, int frameRate);
static int Media_SetBitRate(int streamID, int bitRate);
static int Media_SetInterval(int streamID, int interval);
static int Media_SetQuality(int streamID, float quality);
static int Media_SetGop(int streamID, int gop);
static int Media_Snapshot(void);
static int Media_SetAudioEncodingType(int value);
static int Media_SetAudioBitrate(int value);
static int Media_SetAudioSampleRate(int value);
static int Media_SynVideoEncoderOptions(GONVIF_MEDIA_VideoEncoderConfigurationOptions_S *pstStaticCfg, GK_S32 sizeOptions);
static int Media_SynAudioEncoderOptions(GONVIF_MEDIA_AudioEncoderConfigurationOptions_S *pstStaticCfg, GK_S32 sizeOptions);
static void Media_SaveCfg(void);
static int Media_GetOsdInfor(ONVIF_OsdInfor_S *osdInfor);
static int Media_SetOsdInfor(ONVIF_OsdInfor_S *osdInfor);
static int Media_GetPMInfor(ONVIF_PMInfor_S *pmInfor);
static int Media_SetPMInfor(ONVIF_PMInfor_S *pmInfor);

//imaging
static int Imaging_GetImagingInfor(ONVIF_ImagingInfor_S *imagingInfor);
static int Imaging_SetBrightness(float brightness);
static int Imaging_SetSaturation(float saturation);
static int Imaging_SetContrast(float contrast);
static int Imaging_SetIrCutFilter(int mode);
static int Imaging_SetSharpness(float sharpdess);
static int Imaging_SetWbMode(int mode);
static void Imaging_SaveCfg(void);

#ifdef MODULE_SUPPORT_AF
static int imagingSetFocusMode(int mode);
static int imagingSetAfNearLimit(int nearLimit);
static int imagingSetAfFarLimit(int farLimit);
static int imagingSetMfDefaultSpeed(int defaultSpeed);
#endif

//ptz
static int PTZ_ContinuousMove(ONVIF_PTZContinuousMove_S stContinuousMove);
static int PTZ_AbsoluteMove(ONVIF_PTZAbsoluteMove_S stRelativeMove);
static int PTZ_RelativeMove(ONVIF_PTZRelativeMove_S stRelativeMove);
static int PTZ_Stop(ONVIF_PTZStop_S stStop);
static int PTZ_GetPresets(ONVIF_PTZ_GetPresets_S *pstAllPresets);
static int PTZ_SetPreset(ONVIF_PTZ_Preset_S stPreset);
static int PTZ_GotoPreset(int presetNum);
static int PTZ_RemovePreset(int presetNum);
//event

//analytics

//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************


GONVIF_ADAPTER_Media_S g_stMediaAdapter =
{
	.GetVideoEncoderInfor   = Media_GetVideoEncoderInfor,
	.GetAudioEncoderInfor   = Media_GetAudioEncoderInfor,

    .GetStreamUri           = Media_GetStreamUri,
    .SetResolution          = Media_SetResolution,
    .SetFrameRate           = Media_SetFrameRate,
    .SetBitRate             = Media_SetBitRate,
    .SetInterval            = Media_SetInterval,
    .SetQuality             = Media_SetQuality,
    .SetGovLength           = Media_SetGop,
    .Snapshot               = Media_Snapshot,

    .SetAudioEncodingType   = Media_SetAudioEncodingType,
    .SetAudioBitrate        = Media_SetAudioBitrate,
    .SetAudioSampleRate     = Media_SetAudioSampleRate,
	.SynVideoEncoderOptions	= Media_SynVideoEncoderOptions,	// If SynVideoEncoderOptions == NULL, options use private configuration.
	.SynAudioEncoderOptions = Media_SynAudioEncoderOptions,	// If SynAudioEncoderOptions == NULL, options use private configuration.
    .SaveCfg                = Media_SaveCfg,

    .GetOsdInfor	        = Media_GetOsdInfor,
    .SetOsdInfor	        = Media_SetOsdInfor,
    .GetPMInfor	            = Media_GetPMInfor,
    .SetPMInfor	            = Media_SetPMInfor,
};


ONVIF_ADAPTER_Imaging_S g_stImagingAdapter =
{
	.GetImagingInfor	= Imaging_GetImagingInfor,
    .SetBrightness      = Imaging_SetBrightness,
    .SetColorSaturation	= Imaging_SetSaturation,
    .SetContrast        = Imaging_SetContrast,
    .SetIrCutFilter     = Imaging_SetIrCutFilter,
    .SetSharpness       = Imaging_SetSharpness,
    .SetWbMode		    = Imaging_SetWbMode,
    .SaveCfg            = Imaging_SaveCfg,
#if 0
    .imagingSetBlcMode       	= imagingSetBlcMode,
    .imagingSetExposureMode    	= imagingSetExposureMode,
    .imagingSetAeShutterTimeMin = imagingSetAeShutterTimeMin,
    .imagingSetAeShutterTimeMax	= imagingSetAeShutterTimeMax,
    .imagingSetAeGainMin        = imagingSetAeGainMin,
    .imagingSetAeGainMax     	= imagingSetAeGainMax,
    .imagingSetMeShutterTime	= imagingSetMeShutterTime,
	.imagingSetMeGain			= imagingSetMeGain,
#endif 	

#ifdef MODULE_SUPPORT_AF
    .ImagingSetFocusMode        = imagingSetFocusMode,
    .ImagingSetAfNearLimit      = imagingSetAfNearLimit,
    .ImagingSetAfFarLimit       = imagingSetAfFarLimit,
    .ImagingSetMfDefaultSpeed   = imagingSetMfDefaultSpeed,
#endif 

#if 0    
    .imagingSetWdrMode          = imagingSetWdrMode,
    .imagingSetWdrLevel         = imagingSetWdrLevel,
    .imagingSetMwbRGain   	    = imagingSetMwbRGain,
    .imagingSetMwbBGain         = imagingSetMwbBGain,
    .imagingSetForcePersistence = imagingSetForcePersistence,
#endif
};


ONVIF_ADAPTER_PTZ_S g_stPTZAdapter =
{
	.ContinuousMove  = PTZ_ContinuousMove,
    .AbsoluteMove    = PTZ_AbsoluteMove,
    .RelativeMove    = PTZ_RelativeMove,
    .Stop            = PTZ_Stop,
    .GetPresets		 = PTZ_GetPresets,
    .SetPreset       = PTZ_SetPreset,
    .GotoPreset		 = PTZ_GotoPreset,
    .RemovePreset	 = PTZ_RemovePreset,
};

ONVIF_ADAPTER_Device_S g_stDeviceAdapter =
{
	//network
    .GetNetworkInfor         = Device_Network_GetInfor,
    .SetNetworkInfor	     = Device_Network_SetInfor,
    .GetNetworkMac           = Device_Network_GetMac,
    .SetNetworkMac           = NULL,
    .GetNetworkIP            = Device_Network_GetIP,
    .SetNetworkIP            = Device_Network_SetIP,
    .GetNetworkMask          = NULL,
    .SetNetworkMask          = Device_Network_SetMask,
    .GetNetworkDNS           = NULL,
    .SetNetworkDNS           = Device_Network_SetDNS,
    .GetNetworkNTP		     = Device_Network_GetNTP,
    .SetNetworkNTP		     = Device_Network_SetNTP,
    .GetNetworkGateway	     = NULL,
    .SetNetworkGateway	     = Device_Network_SetGateway,
    .GetNetworkMTU           = Device_Network_GetMTU,
    .SetNetworkMTU           = Device_Network_SetMTU,
    .SetNetworkDHCP          = Device_Network_SetDHCP,
    .GetNetworkIsRunning     = Device_Network_GetIsRunning,
    .GetNetworkWifiNICName   = Device_Network_GetWifiNICName,
    //system
	.SetSystemFactoryDefault = Device_System_SetFactoryDefault,
    .GetSystemDeviceInfor    = Device_System_GetDeviceInformation,
	.GetSystemDateAddTime    = Device_system_GetDateAndTime,
	.SetSystemDateAddTime    = Device_System_SetDateAddTime,
	.SetSystemReboot	     = Device_System_SetReboot,
	.SystemGetUsers		 	 = Device_System_GetUsers,
	.SystemCreateUsers	 	 = Device_System_CreateUsers,
	.SystemDeleteUsers	 	 = Device_System_DeleteUsers,
	.SystemSetUser		 	 = Device_System_SetUser,
};

ONVIF_XML_FILE XmlFileHandle =
{
	.OnvifSaveFile           = Device_System_SaveFile,
    .OnvifGetFile            = Device_System_GetFile,
};


static int Device_Network_GetInfor(char *netCardName, ONVIF_DeviceNet_S *pstNicInfor)
{
    if (netCardName == NULL || pstNicInfor == NULL)
    {
        ONVIF_ERR("Invalid parameters.");
        return -1;
    }
    int ret = 0;
    ST_SDK_NETWORK_ATTR netAttr;
    memset(&netAttr, 0, sizeof(ST_SDK_NETWORK_ATTR));
    strncpy(netAttr.name, netCardName, SDK_ETHERSTR_LEN-1);
    ret = netcam_net_get(&netAttr);
    if (ret != 0)
    {
        ONVIF_ERR("Faile to get network info.");
        return -1;
    }

    pstNicInfor->isDHCP = netAttr.dhcp;
    ret = mac_to_array(netAttr.mac, pstNicInfor->macAddr);
    if (ret != 0)
    {
        ONVIF_ERR("faile to set mac to array.");
        return -1;
    }
    strncpy(pstNicInfor->ipAddr, netAttr.ip, MAX_16_LENGTH-1);
    strncpy(pstNicInfor->maskAddr, netAttr.mask, MAX_16_LENGTH-1);
    strncpy(pstNicInfor->gwAddr, netAttr.gateway, MAX_16_LENGTH-1);
    strncpy(pstNicInfor->dnsAddr1, netAttr.dns1, MAX_16_LENGTH-1);
    strncpy(pstNicInfor->dnsAddr2, netAttr.dns2, MAX_16_LENGTH-1);
    pstNicInfor->mtu = 1500;

    return 0;
}

static int Device_Network_SetInfor(char *netCardName, ONVIF_DeviceNet_S stNicInfor)
{
	if (netCardName == NULL)
	{
		ONVIF_ERR("Invalid parameters.");
		return -1;
	}
	int ret = 0;
	ST_SDK_NETWORK_ATTR netAttr;
	memset(&netAttr, 0, sizeof(ST_SDK_NETWORK_ATTR));
	strncpy(netAttr.name, netCardName, SDK_ETHERSTR_LEN-1);

	ret = netcam_net_get(&netAttr);
	if (ret != 0)
	{
		ONVIF_ERR("Faile to get network info.");
		return -1;
	}

	netAttr.dhcp = stNicInfor.isDHCP;
	strncpy(netAttr.ip, stNicInfor.ipAddr, SDK_IPSTR_LEN-1);
	strncpy(netAttr.mask, stNicInfor.maskAddr, SDK_MASKSTR_LEN-1);
	strncpy(netAttr.gateway, stNicInfor.gwAddr, SDK_GATEWAYSTR_LEN-1);
	strncpy(netAttr.dns1, stNicInfor.dnsAddr1, SDK_DNSSTR_LEN-1);
	strncpy(netAttr.dns2, stNicInfor.dnsAddr2, SDK_DNSSTR_LEN-1);

	return 0;
}

static int Device_Network_GetMac(char *netCardName, char *macStr)
{
    if (netCardName == NULL || macStr == NULL)
    {
        ONVIF_ERR("Invalid parameters.");
        return -1;
    }
    int ret = 0;
    ST_SDK_NETWORK_ATTR netAttr;
    memset(&netAttr, 0, sizeof(ST_SDK_NETWORK_ATTR));
    strncpy(netAttr.name, netCardName, SDK_ETHERSTR_LEN-1);

	ret = netcam_net_get(&netAttr);
    if (ret != 0)
    {
        ONVIF_ERR("Faile to get network info.");
        return -1;
    }

	ret = mac_to_array(netAttr.mac, macStr);
    if (ret != 0)
    {
        ONVIF_ERR("faile to set mac to array.");
        return -1;
    }

    return 0;
}

static int Device_Network_GetIP(char *netCardName, struct sockaddr *addr)
{
    if (netCardName == NULL || addr == NULL)
    {
        ONVIF_ERR("Invalid parameters.");
        return -1;
    }
    int ret = 0;
    ST_SDK_NETWORK_ATTR netAttr;
    memset(&netAttr, 0, sizeof(ST_SDK_NETWORK_ATTR));
    strncpy(netAttr.name, netCardName, SDK_ETHERSTR_LEN-1);
    ret = netcam_net_get(&netAttr);
    if (ret != 0)
    {
        ONVIF_ERR("Faile to get network info.");
        return -1;
    }
    struct sockaddr_in netAddr;
    memset(&netAddr, 0, sizeof(struct sockaddr_in));
    netAddr.sin_addr.s_addr = inet_addr(netAttr.ip);
    memcpy(addr, &netAddr, sizeof(struct sockaddr));

    return 0;
}


static int Device_Network_SetIP(char *netCardName, char *ipStr)
{
    if(netCardName == NULL || ipStr == NULL)
    {
        ONVIF_ERR("Invalid parameters.");
        return -1;
    }
    int ret = 0;
    ST_SDK_NETWORK_ATTR netAttr;
    memset(&netAttr, 0, sizeof(ST_SDK_NETWORK_ATTR));
    strncpy(netAttr.name, netCardName, SDK_ETHERSTR_LEN-1);
    ret = netcam_net_get(&netAttr);
    if (ret != 0)
    {
        ONVIF_ERR("Faile to get network info.");
        return -1;
    }
    strncpy(netAttr.ip, ipStr, SDK_IPSTR_LEN-1);

    ret = netcam_net_set(&netAttr);
    if (ret != 0)
    {
        ONVIF_ERR("Fail to set IP");
        return -1;
    }
    netcam_net_cfg_save();
    return 0;
}

static int Device_Network_SetMask(char *netCardName, char *maskStr)
{
    if (netCardName == NULL || maskStr == NULL)
    {
        ONVIF_ERR("Invalid parameters.");
        return -1;
    }
    int ret = 0;
    ST_SDK_NETWORK_ATTR netAttr;
    memset(&netAttr, 0, sizeof(ST_SDK_NETWORK_ATTR));
    strncpy(netAttr.name, netCardName, SDK_ETHERSTR_LEN-1);
    ret = netcam_net_get(&netAttr);
    if (ret != 0)
    {
        ONVIF_ERR("Faile to Get net Attr.");
        return -1;
    }
    strncpy(netAttr.mask, maskStr, SDK_MASKSTR_LEN-1);

    ret = netcam_net_set(&netAttr);
    if (ret != 0)
    {
        ONVIF_ERR("Fail to set Mask");
        return -1;
    }

    netcam_net_cfg_save();

    return 0;
}

static int Device_Network_SetDNS(char *netCardName, ONVIF_DeviceNet_S netInfor)
{
    if (netCardName == NULL)
    {
		ONVIF_ERR("Invalid NIC name.");
		return -1;
    }
    int ret = 0;
    ST_SDK_NETWORK_ATTR netAttr;
    memset(&netAttr, 0, sizeof(ST_SDK_NETWORK_ATTR));
    strncpy(netAttr.name, netCardName, SDK_ETHERSTR_LEN-1);
    ret = netcam_net_get(&netAttr);
    if (ret != 0)
    {
        ONVIF_ERR("Faile to Get net Attr.");
        return -1;
    }
    if (netInfor.dnsAddr1[0] != 0)
    	strncpy(netAttr.dns1, netInfor.dnsAddr1, SDK_DNSSTR_LEN-1);
    if (netInfor.dnsAddr2[0] != 0)
    	strncpy(netAttr.dns2, netInfor.dnsAddr2, SDK_DNSSTR_LEN-1);

    ret = netcam_net_set(&netAttr);
    if (ret != 0)
    {
        ONVIF_ERR("Fail to set dns");
        return -1;
    }

    netcam_net_cfg_save();
    return 0;
}

static int Device_Network_GetNTP(ONVIF_DeviceNTP_S *ntpInfor)
{
	if (ntpInfor == NULL)
	{
		ONVIF_ERR("Invalid parameters.");
		return -1;
	}
    int ret = 0;
    ret = netcam_sys_ntp_get(&ntpInfor->isNTP, ntpInfor->ntpAddr, MAX_32_LENGTH, &ntpInfor->port);
    if (ret != 0)
    {
        ONVIF_ERR("Faile to get NTP.");
        return -1;
    }
	return 0;
}

static int Device_Network_SetNTP(ONVIF_DeviceNTP_S ntpInfor)
{
	int ret = 0;
	ret = netcam_sys_ntp_start_stop(ntpInfor.isNTP);
	if (ret != 0)
	{
		ONVIF_ERR("Fail to start/stop NTP service.");
		return -1;
	}
	if (ntpInfor.isNTP != 0)
	{
		ret = netcam_sys_ntp_set(ntpInfor.ntpAddr, MAX_32_LENGTH, ntpInfor.port);
		if (ret != 0)
		{
			ONVIF_ERR("Fail to set NTP.");
			return -1;
		}
	}

	return 0;
}

static int Device_Network_SetGateway(char *netCardName, char *gwStr)
{
    if (netCardName == NULL || gwStr == NULL)
    {
        ONVIF_ERR("Invalid parameters.");
        return -1;
    }
	int ret = 0;
	ST_SDK_NETWORK_ATTR netAttr;
	memset(&netAttr, 0, sizeof(ST_SDK_NETWORK_ATTR));
	strncpy(netAttr.name, netCardName, SDK_ETHERSTR_LEN-1);
	ret = netcam_net_get(&netAttr);
	if (ret != 0)
	{
		ONVIF_ERR("Faile to Get net Attr.");
		return -1;
	}
	strncpy(netAttr.gateway, gwStr, SDK_GATEWAYSTR_LEN-1);

	ONVIF_DBG("Device_Network_SetGateway:gw:%s, dhcp: %d\n", netAttr.gateway, netAttr.dhcp);
	ret = netcam_net_set(&netAttr);
	if (ret != 0)
	{
		ONVIF_ERR("Fail to set Mask");
		return -1;
	}

	netcam_net_cfg_save();
	return 0;
}

static int Device_Network_GetMTU(char *netCardName, int *mtu)
{
    if (netCardName == NULL || mtu == NULL)
    {
        ONVIF_ERR("Invalid parameters.");
        return -1;
    }
    (void)(netCardName);

    *mtu = 1500;
    return 0;
}


static int Device_Network_SetMTU(char *netCardName, int mtu)
{
    if (netCardName == NULL)
    {
        ONVIF_ERR("Invalid parameters.");
        return -1;
    }
    (void)(netCardName);
    (void)(mtu);
	netcam_net_cfg_save();
    return 0;
}

static int Device_System_SetFactoryDefault(ONVIF_DeviceFactory_E enFactory)
{
	NETCAM_SYS_OPERATION enOperation;
    if (enFactory == Factory_soft)
    	enOperation = SYSTEM_OPERATION_SOFT_DEFAULT;
	else
    	enOperation = SYSTEM_OPERATION_HARD_DEFAULT;

    netcam_timer_add_task2(netcam_sys_operation, 1, SDK_FALSE, SDK_FALSE, 0, (void *)enOperation);

    return 0;
}

static int Device_System_GetDeviceInformation(char *netCardName, GONVIF_DEVMNG_GetDeviceInformation_Res_S *pstADP_GetDeviceInformationRes)
{
    if (pstADP_GetDeviceInformationRes == NULL || netCardName == NULL)
    {
        ONVIF_ERR("Invalid parameters.");
        return -1;
    }

    int ret = 0;
    GK_NET_DEVICE_INFO stDevInfo;
    memset(&stDevInfo, 0, sizeof(GK_NET_DEVICE_INFO));
    netcam_sys_get_DevInfor(&stDevInfo);

    strncpy(pstADP_GetDeviceInformationRes->aszManufacturer, stDevInfo.manufacturer, MAX_MANUFACTURER_LENGTH-1);
    strncpy(pstADP_GetDeviceInformationRes->aszModel, stDevInfo.deviceName, MAX_MODEL_LENGTH-1);
    strncpy(pstADP_GetDeviceInformationRes->aszFirmwareVersion, stDevInfo.firmwareVersion, MAX_FIRMWARE_VERSION_LENGTH-1);
    strncpy(pstADP_GetDeviceInformationRes->aszHardwareId, stDevInfo.hardwareVersion, MAX_HARDWARE_ID_LENGTH-1);
#if 1	//private CMS
    ST_SDK_NETWORK_ATTR netAttr;
    memset(&netAttr, 0, sizeof(ST_SDK_NETWORK_ATTR));
    strncpy(netAttr.name, netCardName, SDK_ETHERSTR_LEN-1);
    ret = netcam_net_get(&netAttr);
    if (ret != 0)
    {
        ONVIF_ERR("Faile to get network info.");
        return -1;
    }

    strncpy(pstADP_GetDeviceInformationRes->aszSerialNumber, (char *)netAttr.mac, MAX_SERIAL_NUMBER_LENGTH-1);
#else	//standard ONVIF
    strncpy(pstADP_GetDeviceInformationRes->aszSerialNumber, stDevInfo.serialNumber, MAX_SERIAL_NUMBER_LENGTH-1);
#endif
    return ret;
}

static int Device_system_GetDateAndTime(char *timeBuf, int *timeZone)
{
    if (timeBuf == NULL || timeZone == NULL)
    {
        ONVIF_ERR("Invalid parameters.");
        return -1;
    }
	int secZone = 0;

    netcam_sys_get_local_time_string(timeBuf, &secZone);
    *timeZone = secZone/60;

	return 0;
}

static int Device_System_SetDateAddTime(char *timeBuf, int timeZone)
{
    if (timeBuf == NULL)
    {
        ONVIF_ERR("Invalid parameters.");
        return -1;
    }

    netcam_sys_set_time_zone_by_utc_string(timeBuf, timeZone);
	netcam_sys_save();

    return 0;
}

static int Device_System_SetReboot(int delayTime)
{
	NETCAM_SYS_OPERATION enOperation = SYSTEM_OPERATION_REBOOT;;

    netcam_timer_add_task2(netcam_sys_operation, delayTime, SDK_FALSE, SDK_FALSE, 0, (void *)enOperation);

    return 0;
}

static int Device_System_GetUsers(GONVIF_DEVMNG_GetUsers_Res_S *pstUsersCfg)
{
	int i = 0;
	if (pstUsersCfg == NULL)
	{
        ONVIF_ERR("Invalid parameters.");
        return -1;
	}
	for (i = 0; i < GK_MAX_USER_NUM; i ++)
	{
		if (runUserCfg.user[i].enable == 1)
		{
			if (pstUsersCfg->sizeUser >= MAX_USER_NUM)
			{
				ONVIF_ERR("numbers of users is overflow.");
				return -1;
			}
			strncpy(pstUsersCfg->stUser[pstUsersCfg->sizeUser].aszUsername, runUserCfg.user[i].userName, MAX_USERNAME_LENGTH-1);
			strncpy(pstUsersCfg->stUser[pstUsersCfg->sizeUser].aszPassword, runUserCfg.user[i].password, MAX_PASSWORD_LENGTH-1);
			pstUsersCfg->stUser[pstUsersCfg->sizeUser].enUserLevel = runUserCfg.user[i].userLevel-1;
			pstUsersCfg->sizeUser++;
		}
	}

	return 0;
}

static int Device_System_CreateUsers(GONVIF_DEVMNG_User_S userCfg)
{
	int ret = 0;
	GK_NET_USER_INFO user;
	memset(&user, 0, sizeof(GK_NET_USER_INFO));

	user.enable = 1;
	strncpy(user.userName, userCfg.aszUsername, MAX_STR_LEN_64-1);
	strncpy(user.password, userCfg.aszPassword, MAX_STR_LEN_64-1);
	user.userRight = 2;
	user.userLevel = userCfg.enUserLevel+1;
	ret = UserAdd(&user);
	if (ret != 0)
	{
		ONVIF_ERR("Fail to add new user to system.");
		return -1;
	}

	UserCfgSave();
	return 0;
}

static int Device_System_DeleteUsers(GK_CHAR *userName)
{
	if (userName == NULL)
	{
        ONVIF_ERR("Invalid parameters.");
        return -1;
	}
	int ret = 0;
	ret = UserDel(userName);
	if (ret != 0)
	{
		ONVIF_ERR("Fail to delete user from system.");
		return -1;
	}

	UserCfgSave();
	return 0;
}

static int Device_System_SetUser(GONVIF_DEVMNG_User_S userCfg)
{
	int ret = 0;
	GK_NET_USER_INFO user;
	memset(&user, 0, sizeof(GK_NET_USER_INFO));

	user.enable = 1;
	strncpy(user.userName, userCfg.aszUsername, MAX_STR_LEN_64-1);
	strncpy(user.password, userCfg.aszPassword, MAX_STR_LEN_64-1);
	user.userRight = -1;
	user.userLevel = userCfg.enUserLevel+1;
	ret = UserModify(&user);
	if (ret != 0)
	{
		ONVIF_ERR("Fail to set user to system.");
		return -1;
	}

	UserCfgSave();
	return 0;
}

static int Device_Network_SetDHCP(char *netCardName, int isDHCP)
{
    if (netCardName == NULL)
    {
        ONVIF_ERR("Invalid parameters.");
        return -1;
    }
    int ret = 0;
    ST_SDK_NETWORK_ATTR netAttr;
    memset(&netAttr, 0, sizeof(ST_SDK_NETWORK_ATTR));
    strncpy(netAttr.name, netCardName, SDK_ETHERSTR_LEN-1);
    ret = netcam_net_get(&netAttr);
    if (ret != 0)
    {
        ONVIF_ERR("Faile to Get net Attr.");
        return -1;
    }
    netAttr.dhcp = isDHCP;
    ret = netcam_net_set(&netAttr);
    if (ret != 0)
    {
        ONVIF_ERR("Fail to set HDCP");
        return -1;
    }
    netcam_net_cfg_save();
    return 0;
}

static int Device_Network_GetIsRunning(char *netCardName)
{
	int ret = 0;
	if (netCardName == NULL)
	{
		ONVIF_ERR("Invalid name of NIC.");
		return -1;
	}
	ret = netcam_net_get_detect(netCardName);
	if (ret != 0)
	{
		//ONVIF_ERR("%s is not running.", netCardName);
		return 0;
	}
	return 1;
}

static int Device_Network_GetWifiNICName(char *netCardName, int nameLen)
{
	char *wifiName = NULL;
	if (netCardName == NULL)
	{
		ONVIF_ERR("Invalid name buffer of NIC");
		return -1;
	}
	wifiName = netcam_net_wifi_get_devname();
	if (wifiName == NULL)
	{
		//ONVIF_ERR("Fail to get name of wifi NIC.");
		return -1;
	}
	strncpy(netCardName, wifiName, nameLen-1);

	return 0;
}


static int Device_System_GetFile(ONVIF_FILE_TYPE type,char *buffer,int *len)
{
    char name[128];
    char namedef[128];
    FILE *fp;
    int file_len;
    int ret;
	if (access(LINUX_ONVIF_CFG_DIR, R_OK))
    {
        if(mkdir(LINUX_ONVIF_CFG_DIR, S_IRWXU | S_IRWXG| S_IROTH | S_IXOTH))
           ONVIF_ERR("mkdir %s fail\n", LINUX_ONVIF_CFG_DIR);
    }
    if (type < DEVICES || type > PTZ)
    {
        ONVIF_ERR("Bad file type\n");
        return -1;
    }

    if (buffer == NULL || *len < 0)
    {
        ONVIF_ERR("Bad parameter\n");
        return -1;
    }

    sprintf(name,"%s%s",LINUX_ONVIF_CFG_DIR,onvif_file[type]);
    fp = fopen(name, "r");
    if (fp == NULL)
	{
        sprintf(namedef,"%s%s",LINUX_ONVIF_CFG_DEF_DIR,onvif_file[type]);
        if (copy_file(namedef, name) != 0)
        {
            ONVIF_ERR("copy file failed: src:%s,dst:%s\n",namedef,name);
            return -1;
        }
		ONVIF_INFO("fopen def xml file!\n");
        fp = fopen(name, "r");
        if (fp == NULL)
        {
            ONVIF_ERR("open file failed: file:%s",name);
            return -1;
        }
    }

	fseek(fp,0,SEEK_END);
	file_len = ftell(fp);
	if (*len < file_len)
	{
		ONVIF_ERR("Get file len is small!,name:%s\n",name);
        fclose(fp);
		return -1;
	}
	fseek(fp,0,SEEK_SET);
    memset(buffer,0,*len);
    ret = fread(buffer,1,file_len,fp);
    if (ret != file_len)
    {
		ONVIF_ERR("Read file length error!,read:%d,size:%d,name:%s\n",ret,file_len,name);
        fclose(fp);
		return -1;
    }
    fclose(fp);
	*len = file_len;
	return 0;
}

static int Device_System_SaveFile(ONVIF_FILE_TYPE type,char *file_name)
{
    if (type < DEVICES || type > PTZ)
    {
        ONVIF_ERR("Bad file type\n");
        return -1;
    }

    sprintf(file_name,"%s%s",LINUX_ONVIF_CFG_DIR,onvif_file[type]);
	return 0;

}


static int Media_GetVideoEncoderInfor(ONVIF_MediaVideoEncoderInfor_S *pstEncodeInfor, int encodeNum)
{
    if (pstEncodeInfor == NULL)
    {
        ONVIF_ERR("Invalid parameters.\n");
        return -1;
    }

    int ret = 0;
	int sizeEncoder = 0;

    if (encodeNum > 4 || encodeNum <= 0)
        sizeEncoder = 4;
	else
		sizeEncoder = encodeNum;

    GK_NET_VIDEO_CFG *pstEncoderConfigs;
    pstEncoderConfigs = (GK_NET_VIDEO_CFG *)malloc(sizeof(GK_NET_VIDEO_CFG));
    if (pstEncoderConfigs == NULL)
    {
		ONVIF_ERR("Fail to alloc memory.");
		return -1;
    }
    memset(pstEncoderConfigs, 0, sizeof(GK_NET_VIDEO_CFG));
    ret = netcam_video_get_all_channel_par(pstEncoderConfigs);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: Get all channel configurations error.");
		free(pstEncoderConfigs);
		pstEncoderConfigs = NULL;
        return -1;
    }

    int i = 0;
    for (i = 0; i < sizeEncoder; i++)
    {
        pstEncodeInfor[i].streamid = pstEncoderConfigs->vencStream[i].id;
        pstEncodeInfor[i].encode_type = pstEncoderConfigs->vencStream[i].enctype;
        pstEncodeInfor[i].encode_width = pstEncoderConfigs->vencStream[i].h264Conf.width;
        pstEncodeInfor[i].encode_height = pstEncoderConfigs->vencStream[i].h264Conf.height;
        pstEncodeInfor[i].framerate = pstEncoderConfigs->vencStream[i].h264Conf.fps;
        pstEncodeInfor[i].bitrate = pstEncoderConfigs->vencStream[i].h264Conf.bps/1000;
        //pstEncodeInfor[i].encInterval = pstEncoderConfigs->vencStream[i].h264Conf.idrInterval;
        pstEncodeInfor[i].quality = pstEncoderConfigs->vencStream[i].h264Conf.quality;
        pstEncodeInfor[i].govLength = pstEncoderConfigs->vencStream[i].h264Conf.gop;  
        pstEncodeInfor[i].h264Profile = pstEncoderConfigs->vencStream[i].h264Conf.profile;
    }

    free(pstEncoderConfigs);
    pstEncoderConfigs = NULL;

    return ret;
}

static int Media_GetAudioEncoderInfor(ONVIF_MediaAudioEncoderInfor_S *pstEncodeInfor, int encodeNum)
{
    if (pstEncodeInfor == NULL)
    {
        ONVIF_ERR("Invalid parameters.");
        return -1;
    }
    int ret = 0;
	int sizeEncoder = 0;
    if (encodeNum > 1 || encodeNum <= 0)
        sizeEncoder = 1;
	else
		sizeEncoder = encodeNum;

	(void)sizeEncoder;// ignore for the moment

    pstEncodeInfor[0].encode_type = 0;
    pstEncodeInfor[0].bitrate     = 16;
    pstEncodeInfor[0].sampleRate  = 8;

    return ret;
}


static int Media_GetStreamUri(unsigned char streamID, GONVIF_MEDIA_StreamType_E enStreamType, char *pstStreamUri, int uriLen)
{
    if (pstStreamUri == NULL)
    {
        ONVIF_ERR("Invalid parameters.");
        return -1;
    }

    int ret = 0;
    char stream[MAX_VIDEOENCODE_NUM][32] = {"stream0", "stream1", "stream2", "stream3"};

    snprintf(pstStreamUri, uriLen, "rtsp://%s:554/%s", g_GkIpc_OnvifInf.discov.hostip, stream[streamID]/*, enStreamType ? "false":"true"*/);

    return ret;
}

static int Media_SetResolution(int streamID, int width, int height, int encodetype)
{
    int ret = 0;
    if (encodetype != VideoEncoding_H264)
    {
        ONVIF_ERR("unsupported encodingType.");
        return -1;
    }
    ST_GK_ENC_STREAM_H264_ATTR stH264Config;
    memset(&stH264Config, 0, sizeof(ST_GK_ENC_STREAM_H264_ATTR));
    ret = netcam_video_get(0, streamID, &stH264Config);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: get video parameters failed.");
        return -1;
    }
    //stH264Config.enctype = (encodetype == VideoEncoding_H264)?1:3;
    stH264Config.width   = width;
    stH264Config.height  = height;
    ret = netcam_video_set(0, streamID, &stH264Config);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: set resolution failed.");
        return -1;
    }
    netcam_osd_update_clock();
    netcam_osd_update_title();
    netcam_osd_update_id();

    return ret;
}

static int Media_SetFrameRate(int streamID, int frameRate)
{
    int ret = 0;
    ST_GK_ENC_STREAM_H264_ATTR stH264Config;
    memset(&stH264Config, 0, sizeof(ST_GK_ENC_STREAM_H264_ATTR));
    ret = netcam_video_get(0, streamID, &stH264Config);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: get video parameters failed.");
        return -1;
    }

    stH264Config.fps = frameRate;
    ret = netcam_video_set(0, streamID, &stH264Config);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: set frame Rate failed.");
        return -1;
    }

    return ret;
}

static int Media_SetBitRate(int streamID, int bitRate)
{
    int ret = 0;
    ST_GK_ENC_STREAM_H264_ATTR stH264Config;
    memset(&stH264Config, 0, sizeof(ST_GK_ENC_STREAM_H264_ATTR));
    ret = netcam_video_get(0, streamID, &stH264Config);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: get video parameters failed.");
        return -1;
    }

    stH264Config.bps = bitRate;
    ret = netcam_video_set(0, streamID, &stH264Config);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: set bitrate failed.");
        return -1;
    }

    return ret;
}

static int Media_SetInterval(int streamID, int interval)
{
    int ret = 0;
    ST_GK_ENC_STREAM_H264_ATTR stH264Config;
    memset(&stH264Config, 0, sizeof(ST_GK_ENC_STREAM_H264_ATTR));
    ret = netcam_video_get(0, streamID, &stH264Config);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: get video parameters failed.");
        return -1;
    }
	stH264Config.gop = interval;
	ret = netcam_video_set(0, streamID, &stH264Config);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: set interval failed.");
        return -1;
    }

    return ret;
}

static int Media_SetQuality(int streamID, float quality)
{
    int ret = 0;
    ST_GK_ENC_STREAM_H264_ATTR stH264Config;
    memset(&stH264Config, 0, sizeof(ST_GK_ENC_STREAM_H264_ATTR));
    ret = netcam_video_get(0, streamID, &stH264Config);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: get video parameters failed.");
        return -1;
    }

    int temp = (int)quality;
    stH264Config.quality = temp;
    ret = netcam_video_set(0, streamID, &stH264Config);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: set quality failed.");
        return -1;
    }

    return ret;
}

static int Media_SetGop(int streamID, int gop)
{
    int ret = 0;
    ST_GK_ENC_STREAM_H264_ATTR stH264Config;
    memset(&stH264Config, 0, sizeof(ST_GK_ENC_STREAM_H264_ATTR));
    ret = netcam_video_get(0, streamID, &stH264Config);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: get video parameters failed.");
        return -1;
    }

    stH264Config.gop = gop;
    ret = netcam_video_set(0, streamID, &stH264Config);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: set gop failed.");
        return -1;
    }

    return ret;
}


static int Media_Snapshot(void)
{
    int ret = 0;
    char path[50];
    memset(path, 0, sizeof(path));
	snprintf(path, 50, "/tmp/onvif_snapshot.jpg");

    ret = netcam_video_snapshot(runVideoCfg.vencStream[1].h264Conf.width, runVideoCfg.vencStream[1].h264Conf.height, path, GK_ENC_SNAPSHOT_QUALITY_MEDIUM);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: snap shot failed.");
        return -1;
    }

    return 0;
}

static int Media_SetAudioEncodingType(int value)
{
    int ret = 0;

    return ret;
}

static int Media_SetAudioSampleRate(int value)
{
    int ret = 0;

    return ret;
}

static int Media_SynVideoEncoderOptions(GONVIF_MEDIA_VideoEncoderConfigurationOptions_S *pstStaticCfg, GK_S32 sizeOptions)
{
	if (pstStaticCfg == NULL || sizeOptions <= 0)
	{
		ONVIF_ERR("onvif adapter: invalid parameters.");
		return -1;
	}
	int i = 0;
	int j = 0;
	int ret = 0;
	int options = 0;
	if (sizeOptions > MAX_VENC_STREAM_NUM)
		options = MAX_VENC_STREAM_NUM;
	else
		options = sizeOptions;

	for (i = 0; i < options; i++)
	{
		for (j = 0; j < MAX_VENC_ITEM_NUM && videoMap[i][j].stringName != NULL; j++)
		{
			if (strcmp(videoMap[i][j].stringName, "resolution") == 0)
			{
				if (pstStaticCfg[i].stH264.pstResolutionsAvailable)
				{
					free(pstStaticCfg[i].stH264.pstResolutionsAvailable);
					pstStaticCfg[i].stH264.pstResolutionsAvailable = NULL;
				}
				ret = onvif_get_resolution(&pstStaticCfg[i].stH264.pstResolutionsAvailable, i, &pstStaticCfg[i].stH264.sizeResolutionsAvailable);
				if (ret != 0)
				{
					ONVIF_ERR("onvif adapter: fail to get resolution options.");
					return -1;
				}
			}
			if (strcmp(videoMap[i][j].stringName, "quality") == 0)
			{
				pstStaticCfg[i].stQualityRange.min = (int)videoMap[i][j].min;
				pstStaticCfg[i].stQualityRange.max = (int)videoMap[i][j].max;
			}
			if (strcmp(videoMap[i][j].stringName, "gop") == 0)
			{
				pstStaticCfg[i].stH264.stGovLengthRange.min = (int)videoMap[i][j].min;
				pstStaticCfg[i].stH264.stGovLengthRange.max = (int)videoMap[i][j].max;
			}
			if (strcmp(videoMap[i][j].stringName, "fps") == 0)
			{
				pstStaticCfg[i].stH264.stFrameRateRange.min = (int)videoMap[i][j].min;
				pstStaticCfg[i].stH264.stFrameRateRange.max = (int)videoMap[i][j].max;
			}
			//if (strcmp(videoMap[i][j].stringName, "h264_idrInterval") == 0)
			//{
			//	pstStaticCfg[i].stH264.stEncodingIntervalRange.min = (int)videoMap[i][j].min;
			//	pstStaticCfg[i].stH264.stEncodingIntervalRange.max = (int)videoMap[i][j].max;
			//}
			if (strcmp(videoMap[i][j].stringName, "profile") == 0)
			{
				if (pstStaticCfg[i].stH264.penH264ProfilesSupported)
				{
					free(pstStaticCfg[i].stH264.penH264ProfilesSupported);
					pstStaticCfg[i].stH264.penH264ProfilesSupported = NULL;
				}
				pstStaticCfg[i].stH264.sizeH264ProfilesSupported = 1;
				pstStaticCfg[i].stH264.penH264ProfilesSupported = (GONVIF_MEDIA_H264Profile_E *)malloc(sizeof(GONVIF_MEDIA_H264Profile_E));
				if (pstStaticCfg[i].stH264.penH264ProfilesSupported == NULL)
				{
					ONVIF_ERR("onvif adapter: fail to alloc memory.");
					return -1;
				}
				*pstStaticCfg[i].stH264.penH264ProfilesSupported = 0;
			}
			if (strcmp(videoMap[i][j].stringName, "bps") == 0)
			{
				pstStaticCfg[i].stExtension.stH264.stBitrateRange.min = (int)videoMap[i][j].min/1000;
				pstStaticCfg[i].stExtension.stH264.stBitrateRange.max = (int)videoMap[i][j].max/1000;
			}
		}
	}

	return ret;
}

static int Media_SynAudioEncoderOptions(GONVIF_MEDIA_AudioEncoderConfigurationOptions_S *pstStaticCfg, GK_S32 sizeOptions)
{
	int ret = 0;

	return ret;
}

static int Media_SetAudioBitrate(int value)
{
    int ret = 0;

    return ret;
}

static void Media_SaveCfg(void)
{
    netcam_video_cfg_save();
    //netcam_audio_cfg_save();
}

static int Media_GetOsdInfor(ONVIF_OsdInfor_S *osdInfor)
{
    if (osdInfor == NULL)
    {
        ONVIF_ERR("Invalid parameters.");
        return -1;
    }

    int ret = 0;
    GK_NET_CHANNEL_INFO channelInfo;
    ret = netcam_osd_get_info(0,&channelInfo);;
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: get osd parameters failed.");
        return -1;
    }
    osdInfor->chEnable = channelInfo.osdChannelName.enable;
    //strcpy(osdInfor->chName, channelInfo.osdChannelName.text);
	netcam_osd_text_copy(osdInfor->chName, channelInfo.osdChannelName.text, sizeof(osdInfor->chName));
    osdInfor->chx      = channelInfo.osdChannelName.x;
    osdInfor->chy      = channelInfo.osdChannelName.y;
    
    osdInfor->enable     = channelInfo.osdDatetime.enable;
    osdInfor->dateFormat = channelInfo.osdDatetime.dateFormat;
    osdInfor->dateSprtr  = channelInfo.osdDatetime.dateSprtr;
    osdInfor->timeFmt    = channelInfo.osdDatetime.timeFmt;
    osdInfor->x          = channelInfo.osdDatetime.x;
    osdInfor->y          = channelInfo.osdDatetime.y;
    osdInfor->displayWeek= channelInfo.osdDatetime.displayWeek;
    return 0;
}

static int Media_SetOsdInfor(ONVIF_OsdInfor_S *osdInfor)
{
    if (osdInfor == NULL)
    {
        ONVIF_ERR("Invalid parameters.");
        return -1;
    }

    int i, ret = 0;
    GK_NET_CHANNEL_INFO channelInfo;
    ret = netcam_osd_get_info(0, &channelInfo);;
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: get osd parameters failed.");
        return -1;
    }
    channelInfo.osdChannelName.enable   = osdInfor->chEnable;
    //strcpy(channelInfo.osdChannelName.text, osdInfor->chName);
	netcam_osd_text_copy(channelInfo.osdChannelName.text, osdInfor->chName, sizeof(channelInfo.osdChannelName.text));
    channelInfo.osdChannelName.x        = osdInfor->chx;
    channelInfo.osdChannelName.y        = osdInfor->chy;
    
    channelInfo.osdDatetime.enable      = osdInfor->enable;
    channelInfo.osdDatetime.dateFormat  = osdInfor->dateFormat;
    
    channelInfo.osdDatetime.dateSprtr   = osdInfor->dateSprtr;
    channelInfo.osdDatetime.timeFmt     = osdInfor->timeFmt;
    channelInfo.osdDatetime.x           = osdInfor->x;
    channelInfo.osdDatetime.y           = osdInfor->y;
    channelInfo.osdDatetime.displayWeek = osdInfor->displayWeek;
    for(i=0; i<3; i++)
    {
        netcam_video_set_stream_name(i, channelInfo.osdChannelName.text);
        ret = netcam_osd_set_info(i, &channelInfo);
        if (ret != 0)
        {
            ONVIF_ERR("onvif adapter: set osd parameters failed.");
        }
    }
    netcam_osd_update_clock();
    netcam_osd_update_title();
    netcam_osd_update_id();
    netcam_timer_add_task(netcam_osd_pm_save, NETCAM_TIMER_TWO_SEC, SDK_FALSE, SDK_TRUE);
    netcam_timer_add_task(netcam_video_cfg_save, NETCAM_TIMER_ONE_SEC, SDK_FALSE, SDK_TRUE);
    return 0;
}


static int Media_GetPMInfor(ONVIF_PMInfor_S *pmInfor)
{
    if (pmInfor == NULL)
    {
        ONVIF_ERR("Invalid parameters.");
        return -1;
    }

    int i;
    GK_NET_SHELTER_RECT cover;
    for( i = 0; i < 4; i++)
    {
        if (netcam_pm_get_cover(0, i, &cover) != 0)
        {
            ONVIF_ERR("onvif adapter: get pm parameters failed.");
            continue;
        }
        pmInfor[i].enable = cover.enable;
        pmInfor[i].x      = cover.x;
        pmInfor[i].y      = cover.y;
        pmInfor[i].width  = cover.width;
        pmInfor[i].height = cover.height;
        pmInfor[i].color  = cover.color;
    }
    return 0;
}

static int Media_SetPMInfor(ONVIF_PMInfor_S *pmInfor)
{
    if (pmInfor == NULL)
    {
        ONVIF_ERR("Invalid parameters.");
        return -1;
    }

    int i;
    GK_NET_SHELTER_RECT cover;
    for( i = 0; i < 4; i++)
    {
        cover.enable = pmInfor[i].enable;
        cover.x      = pmInfor[i].x;
        cover.y      = pmInfor[i].y;
        cover.width  = pmInfor[i].width;
        cover.height = pmInfor[i].height;
        cover.color  = pmInfor[i].color;
        if (netcam_pm_set_cover(0, i, cover) != 0)
        {
            ONVIF_ERR("onvif adapter: set pm parameters failed.");
            continue;
        }
    }
    netcam_timer_add_task(netcam_osd_pm_save, NETCAM_TIMER_TWO_SEC, SDK_FALSE, SDK_TRUE);
    return 0;
}


static int Imaging_GetImagingInfor(ONVIF_ImagingInfor_S *imagingInfor)
{
    if (imagingInfor == NULL)
    {
        ONVIF_ERR("Invalid parameters.");
        return -1;
    }

    int ret = 0;
    GK_NET_IMAGE_CFG stImagingConfig;
    memset(&stImagingConfig, 0, sizeof(GK_NET_IMAGE_CFG));
    ret = netcam_image_get(&stImagingConfig);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: get imaging parameters failed.");
        return -1;
    }
    imagingInfor->brightness      = stImagingConfig.brightness;
    imagingInfor->contrast        = stImagingConfig.contrast;
    imagingInfor->colorSaturation = stImagingConfig.saturation;
    imagingInfor->sharpness       = stImagingConfig.sharpness;
    if (stImagingConfig.irCutMode == 1)
    	imagingInfor->irCutFilter = IrCutFilterMode_OFF;
	else if (stImagingConfig.irCutMode == 2)
    	imagingInfor->irCutFilter = IrCutFilterMode_ON;
    else
    	imagingInfor->irCutFilter = IrCutFilterMode_AUTO;

    return 0;
}
static int Imaging_SetBrightness(float brightness)
{
    int ret = 0;
    GK_NET_IMAGE_CFG stImagingConfig;
    memset(&stImagingConfig, 0, sizeof(GK_NET_IMAGE_CFG));
    ret = netcam_image_get(&stImagingConfig);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: get imaging parameters failed.");
        return -1;
    }
    stImagingConfig.brightness = (int)brightness;
    ret = netcam_image_set(stImagingConfig);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: set brightness failed.");
        return -1;
    }

    return ret;
}
static int Imaging_SetSaturation(float saturation)
{
    int ret = 0;
    GK_NET_IMAGE_CFG stImagingConfig;
    memset(&stImagingConfig, 0, sizeof(GK_NET_IMAGE_CFG));
    ret = netcam_image_get(&stImagingConfig);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: get imaging parameters failed.");
        return -1;
    }
    stImagingConfig.saturation = (int)saturation;
    ret = netcam_image_set(stImagingConfig);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: set saturation failed.");
        return -1;
    }

    return ret;

}

static int Imaging_SetContrast(float contrast)
{
    int ret = 0;
    GK_NET_IMAGE_CFG stImagingConfig;
    memset(&stImagingConfig, 0, sizeof(GK_NET_IMAGE_CFG));
    ret = netcam_image_get(&stImagingConfig);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: get imaging parameters failed.");
        return -1;
    }
    stImagingConfig.contrast = (int)contrast;
    ret = netcam_image_set(stImagingConfig);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: set contrast failed.");
        return -1;
    }

    return ret;

}

static int Imaging_SetIrCutFilter(int mode)
{
    int ret = 0;
    GK_NET_IMAGE_CFG stImagingConfig;
    memset(&stImagingConfig, 0, sizeof(GK_NET_IMAGE_CFG));
    ret = netcam_image_get(&stImagingConfig);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: get imaging parameters failed.");
        return -1;
    }
    if (mode == IrCutFilterMode_ON)
    	stImagingConfig.irCutMode = 2;
	else if (mode == IrCutFilterMode_OFF)
    	stImagingConfig.irCutMode = 1;
	else
    	stImagingConfig.irCutMode = 0;
    ret = netcam_image_set(stImagingConfig);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: set IR mode failed.");
        return -1;
    }

    return ret;

}
static int Imaging_SetSharpness(float sharpdess)
{
    int ret = 0;
    GK_NET_IMAGE_CFG stImagingConfig;
    memset(&stImagingConfig, 0, sizeof(GK_NET_IMAGE_CFG));
    ret = netcam_image_get(&stImagingConfig);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: get imaging parameters failed.");
        return -1;
    }
    stImagingConfig.sharpness = (int)sharpdess;
    ret = netcam_image_set(stImagingConfig);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: set sharpdess failed.");
        return -1;
    }

    return ret;
}
static int Imaging_SetWbMode(int mode)
{
    int ret = 0;
    GK_NET_IMAGE_CFG stImagingConfig;
    memset(&stImagingConfig, 0, sizeof(GK_NET_IMAGE_CFG));
    ret = netcam_image_get(&stImagingConfig);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: get imaging parameters failed.");
        return -1;
    }
    stImagingConfig.wbMode = mode;
    ret = netcam_image_set(stImagingConfig);
    if (ret != 0)
    {
        ONVIF_ERR("onvif adapter: set wb mode failed.");
        return -1;
    }

    return ret;
}

static void Imaging_SaveCfg(void)
{
    netcam_image_cfg_save();
}

#ifdef MODULE_SUPPORT_AF
/* Focus mode: 0 auto,1 manual. */
static int imagingSetFocusMode(int mode)
{
	int retVal;

	return retVal;

}

/* auto focus: NearLimit setting. */
static int imagingSetAfNearLimit(int nearLimit)
{
	int retVal;
	retVal =  netcam_ptz_focus_add();

	return retVal;
}

/* auto focus: FarLimit setting. */
static int imagingSetAfFarLimit(int farLimit)
{
	int retVal;

	retVal =  netcam_ptz_focus_sub();

	return retVal;
}

/* manual focus: DefaultSpeed setting. */
static int imagingSetMfDefaultSpeed(int defaultSpeed)
{
	int retVal;

	
	if (defaultSpeed == 0)
	{
		// stop focus
		retVal = netcam_ptz_focus_stop();
	}

	// set speed

	return retVal;
}
#endif

static int PTZ_ContinuousMove(ONVIF_PTZContinuousMove_S stContinuousMove)
{
    int ret = 0;
	int step = -1;

	printf("xspeed is %d, yspeed is %d,zspped is %d\n",stContinuousMove.panSpeed, stContinuousMove.tileSpeed, stContinuousMove.zoomSpeed);	
	if (stContinuousMove.panSpeed != 0 && stContinuousMove.tileSpeed == 0)
	{
		if (stContinuousMove.panSpeed > 0)
        	ret = pelco_set_right(0xFF);
    	else
    	{
        	ret = pelco_set_left(0xFF);
    	}
		if (ret != 0)
		{
			ONVIF_ERR("Continuous pan moving error");
			return -1;
		}
	}
	else if (stContinuousMove.tileSpeed != 0 && stContinuousMove.panSpeed == 0)
	{
		if (stContinuousMove.tileSpeed > 0)
        	ret = pelco_set_up(0xFF);
    	else
    	{
        	ret = pelco_set_down(0xFF);
    	}
		if (ret != 0)
		{
			ONVIF_ERR("Continuous tile moving error");
			return -1;
		}
	}
	else if (stContinuousMove.panSpeed != 0 && stContinuousMove.tileSpeed != 0)
	{
		int speed = 0;
		if (abs(stContinuousMove.panSpeed) >= abs(stContinuousMove.tileSpeed))
			speed = abs(stContinuousMove.tileSpeed);
		else
		{
			speed = abs(stContinuousMove.panSpeed);
		}

		if (stContinuousMove.panSpeed > 0 && stContinuousMove.tileSpeed > 0)
		{
			ret = pelco_set_right_top(0xFF);
		}
		else if (stContinuousMove.panSpeed > 0 && stContinuousMove.tileSpeed < 0)
		{
			ret = pelco_set_right_bottom(0xFF);
		}
		else if (stContinuousMove.panSpeed < 0 && stContinuousMove.tileSpeed > 0)
		{
			ret = pelco_set_left_top(0xFF);
		}
		else if (stContinuousMove.panSpeed < 0 && stContinuousMove.tileSpeed < 0)
		{
			ret = pelco_set_left_bottom(0xFF);
		}
		if (ret != 0)
		{
			ONVIF_ERR("Continuous tile moving error");
			return -1;
		}
	}
// #ifdef PTZ_ZOOM_CONTROL
	else if (stContinuousMove.zoomSpeed != 0)
	{
		if (stContinuousMove.zoomSpeed > 0)
			//zoom control
			// #ifdef MODULE_SUPPORT_AF
			pelco_set_zoom_tele();
            // #endif
    	else
			//zoom control
			pelco_set_zoom_wide();
			// #ifdef MODULE_SUPPORT_AF
            // #endif
		if (ret != 0)
		{
			ONVIF_ERR("Continuous zoom moving error");
			return -1;
		}
	}
// #endif
	else
	{
		//TODO: auto moving
	}
    return 0;
}

#include "ttl.h"
#include "sdk_af.h"
extern int uart;
static int PTZ_AbsoluteMove(ONVIF_PTZAbsoluteMove_S stAbsoluteMove)
{
	printf("whs:absolutemove...\n");
     	printf("pos_x is %f\n",stAbsoluteMove.panPosition);
	printf("pos_y is %f\n",stAbsoluteMove.tiltPosition);
	printf("pos_z is %f\n",stAbsoluteMove.zoomPosition);

    	send_absolutemove_value(uart, stAbsoluteMove.panPosition, stAbsoluteMove.tiltPosition, stAbsoluteMove.zoomPosition);

	return 0;
}

static int PTZ_RelativeMove(ONVIF_PTZRelativeMove_S stRelativeMove)
{
    int ret = 0;
    int panTempTrans  = (int)((stRelativeMove.panTranslation + 1.0) * 100);
    int panTempSpeed  = (int)((stRelativeMove.panSpeed) * 5);
    int tiltTempTrans = (int)((stRelativeMove.tiltTranslation + 1.0) * 100);
    int tiltTempSpeed = (int)((stRelativeMove.tiltSpeed) * 5);
    //int zoomTempTrans = (int)((stRelativeMove.zoomTranslation + 1.0) * 100);
    //int zoomTempSpeed = (int)((stRelativeMove.zoomSpeed) * 5);
    ONVIF_DBG("%d-%d, %d-%d, %d-%d\n", panTempTrans, panTempSpeed, tiltTempTrans, tiltTempSpeed, zoomTempTrans, zoomTempSpeed);

    if (panTempTrans > 100 && panTempTrans <= 200)
        ret = netcam_ptz_right(panTempTrans - 100, panTempSpeed);
    else if (panTempTrans >= 0 && panTempTrans < 100)
        ret = netcam_ptz_left(100 - panTempTrans, panTempSpeed);
    if (ret != 0)
    {
        ONVIF_ERR("pan moving failed.");
        return -1;
    }

    if (tiltTempTrans > 100 && tiltTempTrans <= 200)
        ret = netcam_ptz_up(tiltTempTrans - 100, tiltTempSpeed);
    else if (tiltTempTrans >= 0 && tiltTempTrans < 100)
        ret = netcam_ptz_down(100 - tiltTempTrans, tiltTempSpeed);
    if (ret != 0)
    {
        ONVIF_ERR("tilt moving failed.");
        return -1;
    }
#ifdef PTZ_ZOOM_CONTROL
    if (zoomTempTrans > 100 && zoomTempTrans <= 200)
        //ret = netcam_ptz_forward();
        #ifdef MODULE_SUPPORT_AF
        ret = netcam_ptz_zoom_add();
        #endif
    else
        //ret = netcam_ptz_backward();
        #ifdef MODULE_SUPPORT_AF
        ret = netcam_ptz_zoom_sub();
        #endif
    if (ret != 0)
    {
        ONVIF_ERR("zoom moving failed.");
        return -1;
    }
#endif
    return 0;
}


static int PTZ_Stop(ONVIF_PTZStop_S stStop)
{
    int ret = 0;
    if (stStop.stopPanTilt == GK_TRUE)
    {
        // ret = netcam_ptz_stop();
        ret = pelco_set_stop();
        if(ret != 0)
            goto out;
    }
//	#ifdef PTZ_ZOOM_CONTROL
    if (stStop.stopZoom == GK_TRUE)
    {
		//stop zoom control
	//	#ifdef MODULE_SUPPORT_AF
        	ret = pelco_set_stop();
        //#endif
    }
    //#endif

    return 0;
out:
    ONVIF_ERR("Stop PTZ failed.");
    return -1;
}

static int PTZ_GetPresets(ONVIF_PTZ_GetPresets_S *pstAllPresets)
{
    if (pstAllPresets == NULL)
    {
		ONVIF_ERR("Invalid parameter.");
		return -1;
    }
    int i = 0;
	int ret = 0;
    ptz_preset_info_t stPresetsInfo;
    memset(&stPresetsInfo, 0, sizeof(ptz_preset_info_t));
    ret = netcam_ptz_get_presets(&stPresetsInfo);
	if (ret != 0)
	{
		ONVIF_ERR("Fail to get all presets.");
		return -1;
	}

	pstAllPresets->sizePreset = stPresetsInfo.sizePreset;
    for (i = 0; i < stPresetsInfo.sizePreset; i++)
    {
        pstAllPresets->stPreset[i].presetNum = stPresetsInfo.presetNum[i];
        strncpy(pstAllPresets->stPreset[i].presetName, stPresetsInfo.presetName[i], MAX_NAME_LENGTH-1);
    }

    return 0;
}

static int PTZ_SetPreset(ONVIF_PTZ_Preset_S stPreset)
{
	int ret = 0;
	pelco_set_preset(stPreset.presetNum);
	ret = netcam_ptz_set_preset(stPreset.presetNum, stPreset.presetName);
	if (ret != 0)
	{
		ONVIF_ERR("Fail to set preset.");
		return -1;
	}

	return 0;
}

static int PTZ_GotoPreset(int presetNum)
{
	int ret = 0;
	GK_NET_CRUISE_GROUP cruiseArr;
	memset(&cruiseArr, 0, sizeof(GK_NET_CRUISE_GROUP));
	cruiseArr.byPointNum	  = 1;
	cruiseArr.byCruiseIndex = 0;
	cruiseArr.struCruisePoint[0].byPointIndex = 0;
	cruiseArr.struCruisePoint[0].byPresetNo	  = presetNum;
	cruiseArr.struCruisePoint[0].byRemainTime = 0;
	cruiseArr.struCruisePoint[0].bySpeed	  = -1;
	pelco_call_preset(presetNum);	
	ret = netcam_ptz_preset_cruise(&cruiseArr);
	if (ret != 0)
	{
		ONVIF_ERR("Fail to goto corresponding preset.");
		return -1;
	}

	return 0;
}

static int PTZ_RemovePreset(int presetNum)
{
	int ret = 0;
	ret = netcam_ptz_clr_preset(presetNum);
	if (ret != 0)
	{
		ONVIF_ERR("Fail to remove preset.");
		return -1;
	}
	return 0;
}
static int mac_to_array(unsigned char *macStr, char *macArr)
{
	if (macStr == NULL || macArr == NULL)
	{
		ONVIF_ERR("invalid parameters.");
		return -1;
	}

    char prev = 0x00;
    char last = 0x00;
    int i = 0, j = 0;
    for (i = 0, j = 0;j < 6; i+=3, j++)
    {
        if (macStr[i] >= 'A' && macStr[i] <='Z')
            prev = macStr[i]-'A'+10;
        else if (macStr[i] >= 'a' && macStr[i] <='z')
            prev = macStr[i]-'a'+10;
        else if (macStr[i] >= '0' && macStr[i] <='9')
            prev = macStr[i]-'0';
        else
            return -1;
        if (macStr[i+1] >= 'A' && macStr[i+1] <='Z')
            last = macStr[i+1]-'A'+10;
        else if (macStr[i+1] >= 'a' && macStr[i+1] <='z')
            last = macStr[i+1]-'a'+10;
        else if (macStr[i+1] >= '0' && macStr[i+1] <='9')
            last = macStr[i+1]-'0';
        else
            return -1;

        macArr[j] = ((prev&0x0F)<<4)|(last&0x0F);
    }
    return 0;
}

static int onvif_get_resolution(GONVIF_MEDIA_VideoResolution_S **optArr, int streamID, int *maxOpt)
{
	int j = 0;
	char *des = NULL, *resp = NULL;
	char resStr[20][20];
	memset(resStr, 0, sizeof(resStr));
	if (optArr == NULL || maxOpt == NULL || (streamID < 0 || streamID > MAX_VENC_STREAM_NUM))
	{
		ONVIF_ERR("onvif adapter: invalid paramters.");
		return -1;
	}
	for (j = 0; j < MAX_VENC_ITEM_NUM; j++)
	{
		if (videoMap[streamID][j].stringName != NULL)
		{
			if (strcmp("resolution", videoMap[streamID][j].stringName) == 0)
			{
				des = (char *)videoMap[streamID][j].dataAddress;
				break;
			}
		}
	}
	if (j >= MAX_VENC_ITEM_NUM)
	{
		ONVIF_ERR("onvif adapter: no resolution options found.");
		return -1;
	}
	if (des != NULL)
	{
		cJSON *json = NULL;
		json = cJSON_Parse(des);
		if (!json)
		{
			ONVIF_ERR("onvif adapter: fail to parse Json.");
			return -1;
		}
	    cJSON *opt = NULL;
	    opt = cJSON_GetObjectItem(json, "opt");
	    if (opt == NULL)
	    {
	        ONVIF_ERR("onvif adapter: fail to get opt element.");
			cJSON_Delete(json);
	        return -1;
	    }

		int arraySize = cJSON_GetArraySize(opt);
		cJSON *arrayItem = NULL;

		for (j = 0; j < arraySize; j++)
		{
			arrayItem = cJSON_GetArrayItem(opt, j);
			if (arrayItem == NULL)
			{
				ONVIF_ERR("onvif adapter: fail to get array item.");
				cJSON_Delete(json);
				return -1;
			}
			if (arrayItem->valuestring != NULL)
				strncpy(resStr[j], arrayItem->valuestring, 19);
		}
		cJSON_Delete(json);

		*maxOpt = arraySize;
		*optArr = (GONVIF_MEDIA_VideoResolution_S *)malloc(sizeof(GONVIF_MEDIA_VideoResolution_S)*arraySize);
		if (*optArr == NULL)
		{
			ONVIF_ERR("onvif adapter: fail to allow memory.");
			return -1;
		}
		memset(*optArr, 0, sizeof(GONVIF_MEDIA_VideoResolution_S)*arraySize);
		for (j = 0; j < arraySize; j++)
		{
			(*optArr)[j].width = atoi(resStr[j]);
			resp = strchr(resStr[j], 'x');
			if (resp == NULL)
			{
				ONVIF_ERR("onvif adapter: invalid resolution format.");
				(*optArr)[j].width = 0;
				return -1;
			}
			(*optArr)[j].height = atoi(resp+1);
		}
	}
	return 0;
}

