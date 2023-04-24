/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif-no-gsoap/src/onvif_device.c
**
** \brief       onvif device
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include "delaytask.h"
#include "onvif_device.h"
#include "config.h"

#define HOSTNAME   "/tmp/hostname"

GK_BOOL   devMng_MatchString(const GK_CHAR *client, const GK_CHAR *server);
ONVIF_RET devMng_RemoveConfigurableScope(GONVIF_DEVMNG_Scopes_S *scope);
GK_S32    devMng_AddConfigurableScope(GONVIF_DEVMNG_Scopes_S *scope);
GK_S32    devMng_SynUsers();

static GK_S32 devMng_SetNetConfigurations(ONVIF_DeviceNet_S *stNet);
static GK_S32 devMng_GetFrefixLength(GK_CHAR *netmask);
static GK_S32 devMng_check_IP(const GK_CHAR *ipAddr, GONVIF_DEVMNG_NetworkHostType_E ipType);
static GK_S32 devMng_GetMask(GK_S32 prefixLength, GK_CHAR *maskStr);

//*****************************************************************************
//*****************************************************************************
//** API Functions : Capabilities
//*****************************************************************************
//*****************************************************************************

ONVIF_RET GK_NVT_DevMng_GetWsdlUrl(GONVIF_DEVMNG_GetWsdlUrl_Res_S *pstDM_GetWsdlUrlRes)
{
    strncpy(pstDM_GetWsdlUrlRes->aszWsdlUrl, ONVIF_WSDL_URL, MAX_WSDL_URL_LENGTH-1);

    return ONVIF_OK;
}

ONVIF_RET GK_NVT_DevMng_GetServices(GONVIF_DEVMGN_GetServices_S *pstDM_GetServices, GONVIF_DEVMGN_GetServices_Res_S *pstDM_GetServicesRes)
{
    GK_S8 aszIPAddr[MAX_ADDR_LENGTH];
    GK_U8 i = 0;
    const GK_CHAR *tds	= "http://www.onvif.org/ver10/device/wsdl";
    const GK_CHAR *tan	= "http://www.onvif.org/ver20/analytics/wsdl";
    const GK_CHAR *tev  = "http://www.onvif.org/ver10/events/wsdl";
    const GK_CHAR *tmd  = "http://www.onvif.org/ver10/deviceIO/wsdl";
    const GK_CHAR *timg = "http://www.onvif.org/ver20/imaging/wsdl";
    const GK_CHAR *trt	= "http://www.onvif.org/ver10/media/wsdl";
    const GK_CHAR *tptz = "http://www.onvif.org/ver20/ptz/wsdl";
    const GK_CHAR *services[]={tds, tan, tev, tmd, timg, trt, tptz, NULL};
    const GK_CHAR *xaddr[]={"device_service", "analytics_service", "event_service", "deviceIO_service", "imaging_service", "media_service", "ptz_service", NULL};

    const GK_CHAR *device    =  "<tds:Capabilities>" \
                                    "<tds:Network NTP=\"0\" HostnameFromDHCP=\"false\" Dot11Configuration=\"false\" DynDNS=\"false\" IPVersion6=\"false\" ZeroConfiguration=\"false\" IPFilter=\"false\" /><tds:Security RELToken=\"false\" HttpDigest=\"false\" false=\"true\" KerberosToken=\"false\" SAMLToken=\"false\" X.509Token=\"false\" RemoteUserHandling=\"false\" Dot1X=\"false\" DefaultAccessPolicy=\"false\" AccessPolicyConfig=\"false\" OnboardKeyGeneration=\"false\" TLS1.2=\"false\" TLS1.1=\"false\" TLS1.0=\"false\" /><tds:System HTTPSupportInformation=\"false\" HTTPSystemLogging=\"false\" HttpSystemBackup=\"false\" HttpFirmwareUpgrade=\"false\" FirmwareUpgrade=\"true\" SystemLogging=\"true\" SystemBackup=\"false\" RemoteDiscovery=\"false\" DiscoveryBye=\"true\" DiscoveryResolve=\"false\" />" \
                                "</tds:Capabilities>";

    const GK_CHAR *analytics = "<tan:Capabilities RuleSupport=\"true\" AnalyticsModuleSupport=\"true\" CellBasedSceneDescriptionSupported=\"false\" />";

    const GK_CHAR *event     = "<tev:Capabilities WSSubscriptionPolicySupport=\"false\" WSPullPointSupport=\"true\" WSPausableSubscriptionManagerInterfaceSupport=\"false\" MaxNotificationProducers=\"10\" MaxPullPoints=\"5\" PersistenNotificationStorage=\"false\" />";

    const GK_CHAR *image     = "<timg:Capabilities ImageStabilization=\"false\" />";
    
    const GK_CHAR *media     = "<trt:Capabilities SnapshotUri=\"true\" Rotation=\"false\">" \
                                    "<trt:ProfileCapabilities MaximumNumberOfProfiles=\"4\" />" \
                                    "<trt:StreamingCapabilities RTPMulticast=\"true\" RTP_TCP=\"true\" RTP_RTSP_TCP=\"true\" NonAggregateControl=\"false\" />" \
                               "</trt:Capabilities>";

    const GK_CHAR *ptz       = "<tptz:Capabilities EFlip=\"false\" Reverse=\"false\" />";

    const GK_CHAR *deviceio  = "<tmd:Capabilities VideoSources=\"1\" VideoOutputs=\"1\" AudioSources=\"1\" AudioOutputs=\"1\" RelayOutputs=\"2\" SerialPorts=\"1\" DigitalInputs=\"1\" />";
	const GK_CHAR *service_capabilities[]={device, analytics, event, deviceio, image, media, ptz, NULL};


    snprintf(aszIPAddr, MAX_ADDR_LENGTH, "http://%s:%d/onvif", g_GkIpc_OnvifInf.discov.hostip, g_GkIpc_OnvifInf.discov.hostport);
    pstDM_GetServicesRes->sizeService = sizeof(services)/sizeof(char *) - 1;
    for(i = 0; NULL != services[i]; i++)
    {
        strncpy(pstDM_GetServicesRes->stService[i].aszNamespace, services[i], MAX_ADDR_LENGTH-1);
        snprintf(pstDM_GetServicesRes->stService[i].aszXAddr, MAX_ADDR_LENGTH, "%s/%s", aszIPAddr, xaddr[i]);
        if(Boolean_TRUE == pstDM_GetServices->enIncludeCapability)
        {
            if(service_capabilities[i])
            {
                sprintf(pstDM_GetServicesRes->stService[i].stCapabilities.any, "%s", service_capabilities[i]);
            }
        }
        pstDM_GetServicesRes->stService[i].stVersion.minor = 0;
        pstDM_GetServicesRes->stService[i].stVersion.major = 3;
    }

    return ONVIF_OK;
}

ONVIF_RET GK_NVT_DevMng_GetCapabilities(GONVIF_DEVMNG_GetCapabilities_S *pstDM_GetCapabilities, GONVIF_DEVMNG_GetCapabilities_Res_S *pstDM_GetCapabilitiesRes)
{
    GK_S8  _IPv4Address[MAX_ADDR_LENGTH];
    GK_BOOL isAll = GK_FALSE;
    //!!!!!!!!pstDM_GetCapabilitiesRes->stCapabilities.enCategory: ONVIF protocol has not this element in GONVIF_DEVMNG_GetCapabilities_Res_S.
    //This shoud be modified later.!!!!!!!!
    sprintf(_IPv4Address, "http://%s:%d/onvif",g_GkIpc_OnvifInf.discov.hostip,g_GkIpc_OnvifInf.discov.hostport);
    
    switch(pstDM_GetCapabilities->enCategory[0])
    {
        case CapabilityCategory_All:
            isAll = GK_TRUE;
        case CapabilityCategory_Analytics:
            pstDM_GetCapabilitiesRes->stCapabilities.stAnalytics.enAnalyticsModuleSupport = Boolean_TRUE;
            pstDM_GetCapabilitiesRes->stCapabilities.stAnalytics.enRuleSupport = Boolean_TRUE;
            sprintf(pstDM_GetCapabilitiesRes->stCapabilities.stAnalytics.aszXAddr, "%s/analytics_service", _IPv4Address);
            if(!isAll)
            {
                break;
            }
        case CapabilityCategory_Device:
            sprintf(pstDM_GetCapabilitiesRes->stCapabilities.stDevice.aszXAddr, "%s/device_service", _IPv4Address);
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stNetwork.enIPFilter = Boolean_FALSE;
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stNetwork.enZeroConfiguration = Boolean_FALSE;
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stNetwork.enIPVersion6 = Boolean_FALSE;
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stNetwork.enDynDNS = Boolean_FALSE;
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSystem.enDiscoveryResolve = Boolean_FALSE;
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSystem.enDiscoveryBye = Boolean_TRUE;
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSystem.enRemoteDiscovery = Boolean_FALSE;
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSystem.enSystemBackup = Boolean_FALSE;
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSystem.enSystemLogging = Boolean_TRUE;
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSystem.enFirmwareUpgrade = Boolean_TRUE;
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSystem.stSupportedVersions.minor = 41;
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSystem.stSupportedVersions.major = 2;
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stIO.inputConnectors = 1;
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stIO.relayOutputs = 2;
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSecurity.enTLS1_1 = Boolean_FALSE;
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSecurity.enTLS1_2 = Boolean_FALSE;
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSecurity.enOnboardKeyGeneration = Boolean_FALSE;
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSecurity.enAccessPolicyConfig = Boolean_FALSE;
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSecurity.enX_x002e509Token = Boolean_FALSE;
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSecurity.enSAMLToken = Boolean_FALSE;
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSecurity.enKerberosToken = Boolean_FALSE;
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSecurity.enRELToken = Boolean_FALSE;
            if(!isAll)
            {
                break;
            }
        case CapabilityCategory_Events:
            sprintf(pstDM_GetCapabilitiesRes->stCapabilities.stEvent.aszXAddr, "%s/event_service", _IPv4Address);
            pstDM_GetCapabilitiesRes->stCapabilities.stEvent.enWSSubscriptionPolicySupport = Boolean_FALSE;
            pstDM_GetCapabilitiesRes->stCapabilities.stEvent.enWSPullPointSupport = Boolean_TRUE;
            pstDM_GetCapabilitiesRes->stCapabilities.stEvent.enWSPausableSubscriptionManagerInterfaceSupport = Boolean_FALSE;
            if(!isAll)
            {
                break;
            }
        case CapabilityCategory_Imaging:
            sprintf(pstDM_GetCapabilitiesRes->stCapabilities.stImaging.aszXAddr, "%s/image_service", _IPv4Address);
            if(!isAll)
            {
                break;
            }

        case CapabilityCategory_Media:
            sprintf(pstDM_GetCapabilitiesRes->stCapabilities.stMedia.aszXAddr, "%s/media_service", _IPv4Address);
            //pstDM_GetCapabilitiesRes->stCapabilities.stMedia.stStreamingCapabilities.enRTPMulticast = Boolean_TRUE;
            //pstDM_GetCapabilitiesRes->stCapabilities.stMedia.stStreamingCapabilities.enRTP_USCORERTSP_USCORETCP = Boolean_TRUE;
            //pstDM_GetCapabilitiesRes->stCapabilities.stMedia.stStreamingCapabilities.enRTP_USCORETCP = Boolean_TRUE;
            if(!isAll)
            {
				break;
			}

        case CapabilityCategory_PTZ:
            sprintf(pstDM_GetCapabilitiesRes->stCapabilities.stPTZ.aszXAddr, "%s/ptz_service", _IPv4Address);
            break;
        default:
            ONVIF_ERR("DeviceManage: No such service.");
            return ONVIF_ERR_DEVMNG_NO_SUCH_SERVICE;
    }
    
    if(isAll)
    {
        sprintf(pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stHik.aszXAddr, "%s/device_service", _IPv4Address);
        pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stHik.IOInputSupport     = Boolean_FALSE;
        pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stHik.PrivacyMaskSupport = Boolean_TRUE;
        pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stHik.PTZ3DZoomSupport   = Boolean_FALSE;
        pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stHik.PTZPatternSupport  = Boolean_TRUE;
        pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stHik.Language           = 1;
        
        sprintf(pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stDeviceIO.aszXAddr, "%s/DeviceIO_service", _IPv4Address);
        pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stDeviceIO.videoSources = 1;
        pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stDeviceIO.videoOutputs = 1;
        pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stDeviceIO.AudioSources = 1;
        pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stDeviceIO.AudioOutputs = 1;
        pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stDeviceIO.RelayOutputs = 2;

    }
    
    return ONVIF_OK;
}

ONVIF_RET GK_NVT_DevMng_GetServiceCapabilities(GONVIF_DEVMGN_GetServiceCapabilities_Res_S *pstDM_GetServiceCapabilitiesRes)
{
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stNetwork.enIPFilter = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stNetwork.enZeroConfiguration = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stNetwork.enIPVersion6 = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stNetwork.enDynDNS = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stNetwork.enDot11Configuration = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stNetwork.dot1XConfigurations = 0;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stNetwork.enHostnameFromDHCP = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stNetwork.ntp = 0;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stNetwork.enDHCPv6 = Boolean_FALSE;

    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enTLS1_x002e0 = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enTLS1_x002e1 = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enTLS1_x002e2 = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enOnboardKeyGeneration = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enAccessPolicyConfig = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enDefaultAccessPolicy = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enDot1X = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enRemoteUserHandling = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enX_x002e509Token = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enSAMLToken = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enKerberosToken = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enUsernameToken = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enHttpDigest = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enRELToken= Boolean_FALSE;

    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSystem.enDiscoveryResolve = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSystem.enDiscoveryBye = Boolean_TRUE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSystem.enRemoteDiscovery = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSystem.enSystemBackup = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSystem.enSystemLogging = Boolean_TRUE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSystem.enFirmwareUpgrade = Boolean_TRUE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSystem.enHttpFirmwareUpgrade = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSystem.enHttpSystemBackup = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSystem.enHttpSystemLogging = Boolean_FALSE;
    pstDM_GetServiceCapabilitiesRes->stCapabilities.stSystem.enHttpSupportInformation = Boolean_FALSE;

    return ONVIF_OK;
}


//*****************************************************************************
//*****************************************************************************
//** API Functions : network
//*****************************************************************************
//*****************************************************************************

ONVIF_RET GK_NVT_DevMng_GetHostname(GONVIF_DEVMNG_GetHostname_Res_S *pstDM_GetHostnameRes)
{
	GK_S32 ret = 0;
	FILE *fp = NULL;
    GK_S8 buffer[MAX_NAME_LENGTH] = {0};
	fp = fopen(HOSTNAME, "rb");
	if(fp == NULL)
	{
		ONVIF_ERR("Fail to open configuration file of hostname.");
		goto run_error;
	}
	ret = fread(buffer, 1, MAX_NAME_LENGTH-1, fp);
	fclose(fp);
	if(ret < 0)
	{
		ONVIF_ERR("Fail to read hostname.");
		goto run_error;
	}
run_error:
	strncpy(pstDM_GetHostnameRes->stHostnameInformation.aszName, buffer, MAX_NAME_LENGTH-1);
    return ONVIF_OK;
}

ONVIF_RET GK_NVT_DevMng_SetHostname(GONVIF_DEVMNG_SetHostname_S *pstDM_SetHostname)
{
    GK_CHAR *p = pstDM_SetHostname->aszName;
    while(*p)
    {
        if(*p == '_')
        {    
            goto arg_error;
        }
        else
        {
            p++;
            continue;
        }
    }

	GK_S32 ret = 0;
	FILE *fp = NULL;
	//fp = fopen("/etc/hostname", "w");
	fp = fopen(HOSTNAME, "w");
	if(fp == NULL)
	{
		ONVIF_ERR("Fail to open configuration file of hostname.");
		goto run_error;
	}
	fclose(fp);
	fp = NULL;
	fp = fopen(HOSTNAME, "wb");
	if(fp == NULL)
	{
		ONVIF_ERR("Fail to open configuration file of hostname.");
		goto run_error;
	}
	ret = fwrite(pstDM_SetHostname->aszName, 1, strlen(pstDM_SetHostname->aszName), fp);	
	if(ret < 0)
	{
		ONVIF_ERR("Fail to write hostname.");
		fclose(fp);
		goto run_error;
	}
	ret = fwrite("\n", 1, 1, fp);	
	if(ret < 0)
	{
		ONVIF_ERR("Fail to write hostname.");
		fclose(fp);
		goto run_error;
	}
	fclose(fp);

    return ONVIF_OK;
    
run_error:
    return ONVIF_ERR_DEVMNG_SERVER_RUN_ERROR;  
arg_error:
    return ONVIF_ERR_DEVMNG_INVALID_HOSTNAME;
    
}

ONVIF_RET GK_NVT_DevMng_GetDNS(GONVIF_DEVMNG_GetDNS_Res_S *pstDM_GetDNSRes)
{
	GK_S32 ret = 0;
    ONVIF_DeviceNet_S stNicInfor;
	memset(&stNicInfor, 0, sizeof(ONVIF_DeviceNet_S));

	ret = g_stDeviceAdapter.GetNetworkInfor(g_GkIpc_OnvifInf.discov.nicName, &stNicInfor);
	if(ret != 0)
	{
		ONVIF_ERR("Fail to get NIC infor.");
		goto run_error;
	}
	if(stNicInfor.isDHCP != 0)
	{
		pstDM_GetDNSRes->stDNSInformation.enFromDHCP = Boolean_TRUE;
		pstDM_GetDNSRes->stDNSInformation.sizeDNSFromDHCP = 2;	
		pstDM_GetDNSRes->stDNSInformation.stDNSFromDHCP[0].enType = IPType_IPv4;
        if(!strcmp(stNicInfor.dnsAddr1, "0.0.0.0")||stNicInfor.dnsAddr1[0] == 0)
        {
            memset(pstDM_GetDNSRes->stDNSInformation.stDNSFromDHCP[0].aszIPv4Address, 0, 
            sizeof(pstDM_GetDNSRes->stDNSInformation.stDNSFromDHCP[0].aszIPv4Address));
        }
        else
        {
		    strncpy(pstDM_GetDNSRes->stDNSInformation.stDNSFromDHCP[0].aszIPv4Address, stNicInfor.dnsAddr1, IPV4_STR_LENGTH-1);
        }
		pstDM_GetDNSRes->stDNSInformation.stDNSFromDHCP[1].enType = IPType_IPv4;
        
        if(!strcmp(stNicInfor.dnsAddr2, "0.0.0.0")||stNicInfor.dnsAddr1[0] == 0)
        {
            memset(pstDM_GetDNSRes->stDNSInformation.stDNSFromDHCP[1].aszIPv4Address, 0, 
            sizeof(pstDM_GetDNSRes->stDNSInformation.stDNSFromDHCP[1].aszIPv4Address));
        }
        else
        {
		    strncpy(pstDM_GetDNSRes->stDNSInformation.stDNSFromDHCP[1].aszIPv4Address, stNicInfor.dnsAddr2, IPV4_STR_LENGTH-1);
        }
	}
	else
	{
		pstDM_GetDNSRes->stDNSInformation.enFromDHCP = Boolean_FALSE;
		pstDM_GetDNSRes->stDNSInformation.sizeDNSManual = 2;
		pstDM_GetDNSRes->stDNSInformation.stDNSManual[0].enType = IPType_IPv4;
        if(!strcmp(stNicInfor.dnsAddr1, "0.0.0.0")||stNicInfor.dnsAddr1[0] == 0)
        {
            memset(pstDM_GetDNSRes->stDNSInformation.stDNSManual[0].aszIPv4Address, 0, 
            sizeof(pstDM_GetDNSRes->stDNSInformation.stDNSManual[0].aszIPv4Address));
        }
        else
        {
		    strncpy(pstDM_GetDNSRes->stDNSInformation.stDNSManual[0].aszIPv4Address, stNicInfor.dnsAddr1, IPV4_STR_LENGTH-1);
        }
		pstDM_GetDNSRes->stDNSInformation.stDNSManual[1].enType = IPType_IPv4;
        if(!strcmp(stNicInfor.dnsAddr2, "0.0.0.0")||stNicInfor.dnsAddr1[0] == 0)
        {
            memset(pstDM_GetDNSRes->stDNSInformation.stDNSManual[1].aszIPv4Address, 0, 
            sizeof(pstDM_GetDNSRes->stDNSInformation.stDNSManual[1].aszIPv4Address));
        }
        else
        {
		    strncpy(pstDM_GetDNSRes->stDNSInformation.stDNSManual[1].aszIPv4Address, stNicInfor.dnsAddr2, IPV4_STR_LENGTH-1);	
        }
        
	}
    pstDM_GetDNSRes->stDNSInformation.sizeSearchDomain = 1;
    strcpy(pstDM_GetDNSRes->stDNSInformation.aszSearchDomain[0], "domain.name");

    return ONVIF_OK;
    
run_error:
	return ONVIF_ERR_DEVMNG_SERVER_RUN_ERROR;  
}

//There has some bugs here:
//1. When calling SetDNS interface many times, GetDNS will be running failed.Because much 
//date had been writeen to /etc/reslv.conf continuously, this case bound problem of array in get_dns. 
//2. Key bug: Server did not implement DNS service!!!
ONVIF_RET GK_NVT_DevMng_SetDNS(GONVIF_DEVMNG_SetDNS_S *pstDM_SetDNS)
{
	GK_S32 ret = 0;
    if(pstDM_SetDNS->enFromDHCP == Boolean_TRUE)
    {
        /*ret = g_stDeviceAdapter.SetNetworkDHCP(g_GkIpc_OnvifInf.discov.nicName, 1);
      		  if(ret != 0)
        	 {
                	ONVIF_ERR("Fail to open DHCP(1).");
            		goto run_error;
        	 }*/
    }
    else if(pstDM_SetDNS->enFromDHCP == Boolean_FALSE)
    {
        ret = g_stDeviceAdapter.SetNetworkDHCP(g_GkIpc_OnvifInf.discov.nicName, 0);
        if(ret != 0)
        {
            ONVIF_ERR("Fail to close DHCP(0).");
            goto run_error;
        }
		ONVIF_DeviceNet_S netInfo;
		memset(&netInfo, 0, sizeof(ONVIF_DeviceNet_S));
    	if(pstDM_SetDNS->sizeDNSManual <= 0)
    	{
			ONVIF_ERR("No manual DNS.");
			//goto invalid_DNSManual;
			strcpy(netInfo.dnsAddr1, "0.0.0.0");
			strcpy(netInfo.dnsAddr2, "0.0.0.0");
            ret = g_stDeviceAdapter.SetNetworkDNS(g_GkIpc_OnvifInf.discov.nicName, netInfo);
            if(ret != 0)
            {
                ONVIF_ERR("Fail to set DNS.");
                goto run_error;
            }
    	}
        else
        {

    		if(pstDM_SetDNS->stDNSManual[0].enType != IPType_IPv4)
    		{
    			ONVIF_ERR("Ipv6 Unsupported.");
    			goto unsupported_IPv6;
    		}
            #if 0
            ret = devMng_check_IP(pstDM_SetDNS->stDNSManual[0].aszIPv4Address, NetworkHostType_IPv4);
            if(ret != 0)
            {
                ONVIF_ERR("Invalid IPv4 format.");
                goto invalid_IPv4;
            }    
            #endif
    		memcpy(netInfo.dnsAddr1, pstDM_SetDNS->stDNSManual[0].aszIPv4Address, MAX_16_LENGTH-1);	
    		if(pstDM_SetDNS->sizeDNSManual > 1)
    		{
    			if(pstDM_SetDNS->stDNSManual[1].enType != IPType_IPv4)
    			{
    				ONVIF_ERR("Ipv6 Unsupported.");
    				goto unsupported_IPv6;
    			}
    			ret = devMng_check_IP(pstDM_SetDNS->stDNSManual[1].aszIPv4Address, NetworkHostType_IPv4);
    			if(ret != 0)
    			{
    				ONVIF_ERR("Invalid IPv4 format.");
    				goto invalid_IPv4;
    			}		 
    			memcpy(netInfo.dnsAddr2, pstDM_SetDNS->stDNSManual[1].aszIPv4Address, MAX_16_LENGTH-1);	
    		}
            ret = g_stDeviceAdapter.SetNetworkDNS(g_GkIpc_OnvifInf.discov.nicName, netInfo);
            if(ret != 0)
            {
                ONVIF_ERR("Fail to set DNS.");
                goto run_error;
            }
        }
    }
    
    ONVIF_DeviceNet_S stNicInfor;
	memset(&stNicInfor, 0, sizeof(ONVIF_DeviceNet_S));

	ret = g_stDeviceAdapter.GetNetworkInfor(g_GkIpc_OnvifInf.discov.nicName, &stNicInfor);
	if(ret != 0)
	{
		ONVIF_ERR("Fail to get NIC infor.");
		goto run_error;
	}
    
    GK_NVT_SaveXmlConfig(XML_DEVICE);
    
    return ONVIF_OK;
    
invalid_DNSManual:
    return ONVIF_ERR_DEVMNG_INVALID_DNS_MANUAL;  
invalid_IPv4:
    return ONVIF_ERR_DEVMNG_INVALID_IPV4_ADDR;
unsupported_IPv6:
    return ONVIF_ERR_DENMNG_UNSUPPORTED_IPV6;
run_error:
	return ONVIF_ERR_DEVMNG_SERVER_RUN_ERROR;      
}

ONVIF_RET GK_NVT_DevMng_GetNTP(GONVIF_DEVMNG_GetNTP_Res_S *pstDM_GetNTPRes)
{
	GK_S32 ret = 0;
	ONVIF_DeviceNTP_S ntpConfig;
	memset(&ntpConfig, 0, sizeof(ONVIF_DeviceNTP_S));
	ret = g_stDeviceAdapter.GetNetworkNTP(&ntpConfig);
	if(ret != 0)
	{
		ONVIF_ERR("Fail to get NTP configuration.");
		goto run_error;
	}
	pstDM_GetNTPRes->stNTPInformation.enFromDHCP = Boolean_FALSE;
	pstDM_GetNTPRes->stNTPInformation.sizeNTPManual = 1;
	pstDM_GetNTPRes->stNTPInformation.stNTPManual[0].enType = NetworkHostType_IPv4;
	strncpy(pstDM_GetNTPRes->stNTPInformation.stNTPManual[0].aszIPv4Address, ntpConfig.ntpAddr, IPV4_STR_LENGTH-1);

    return ONVIF_OK;
    
run_error:
    return ONVIF_ERR_DEVMNG_SERVER_RUN_ERROR;
}

//There has some bugs here: 
//1. Does ntp.conf has "NTP_IS_DHCP" element? 
//2. There has g_GkIpc_OnvifInf.devmgmt.DHCP to restroe HDCP mode(HDCP or not DHCP). Why
//does ntp.conf has  "NTP_IS_DHCP" element? 
//3. Server did not implement NTP service.
ONVIF_RET GK_NVT_DevMng_SetNTP(GONVIF_DEVMNG_SetNTP_S *pstDM_SetNTP)
{
	GK_S32 ret = 0;
    if(pstDM_SetNTP->enFromDHCP == Boolean_FALSE && pstDM_SetNTP->sizeNTPManual == 0)
    {
    	ONVIF_ERR("No such manual NTP configuration.");
        goto invalid_NTPManual;
    }
	if(pstDM_SetNTP->enFromDHCP == Boolean_TRUE)
	{
		ONVIF_INFO("DHCP for NTP: unsupported.");
	}
	else
	{
		if(pstDM_SetNTP->stNTPManual[0].enType == NetworkHostType_IPv6)
		{
			ONVIF_ERR("IPv6: unsupported.");
			goto unsupported_IPv6;
		}
		else if(pstDM_SetNTP->stNTPManual[0].enType == NetworkHostType_IPv4)
		{
			if(devMng_check_IP(pstDM_SetNTP->stNTPManual[0].aszIPv4Address, NetworkHostType_IPv4))
			{
				ONVIF_ERR("Invalid IPv4 format.");
				goto invalid_IPv4;
			}
			ONVIF_DeviceNTP_S ntpConfig;
			memset(&ntpConfig, 0, sizeof(ONVIF_DeviceNTP_S));
			ntpConfig.isNTP = 1;
			strncpy(ntpConfig.ntpAddr, pstDM_SetNTP->stNTPManual[0].aszIPv4Address, MAX_32_LENGTH);
			ntpConfig.port = 123;
			ret = g_stDeviceAdapter.SetNetworkNTP(ntpConfig);
			if(ret != 0)
			{
				ONVIF_ERR("Fail to set NTP configuration.");
				goto run_error;
			}			
		}
		else if(pstDM_SetNTP->stNTPManual[0].enType == NetworkHostType_DNS)
		{
			;
		}

	}
	
    GK_NVT_SaveXmlConfig(XML_DEVICE);
    
    return ONVIF_OK;

invalid_IPv4:
    return ONVIF_ERR_DEVMNG_INVALID_IPV4_ADDR;
unsupported_IPv6:
    return ONVIF_ERR_DENMNG_UNSUPPORTED_IPV6;
invalid_NTPManual:
    return ONVIF_ERR_DEVMNG_INVALID_NTP_MANUAL;
run_error:
	return ONVIF_ERR_DEVMNG_SERVER_RUN_ERROR;
}

ONVIF_RET GK_NVT_DevMng_GetNetworkInterfaces(GONVIF_DEVMNG_GetNetworkInterfaces_Res_S *pstDM_GetNetworkInterfacesRes)
{
	GK_S32 ret = 0;
    ONVIF_DeviceNet_S stNicInfor;
	memset(&stNicInfor, 0, sizeof(ONVIF_DeviceNet_S));
	ret = g_stDeviceAdapter.GetNetworkInfor(g_GkIpc_OnvifInf.discov.nicName, &stNicInfor);
	if(ret != 0)
	{
		ONVIF_ERR("Fail to get NIC infor.");
		goto Run_error;
	}
    pstDM_GetNetworkInterfacesRes->sizeNetworkInterfaces = 1;
	strncpy(pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[0].asztoken, g_GkIpc_OnvifInf.discov.nicName, MAX_TOKEN_LENGTH-1);
	pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[0].enEnabled = Boolean_TRUE;
	
	strncpy(pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[0].stInfo.aszName, g_GkIpc_OnvifInf.discov.nicName, MAX_NAME_LENGTH-1);
	snprintf(pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[0].stInfo.aszHwAddress, MAX_ADDR_LENGTH, 
		"%02X:%02X:%02X:%02X:%02X:%02X", 
		stNicInfor.macAddr[0], stNicInfor.macAddr[1], stNicInfor.macAddr[2], 
		stNicInfor.macAddr[3], stNicInfor.macAddr[4], stNicInfor.macAddr[5]);
	pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[0].stInfo.MTU = stNicInfor.mtu;
	
	pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[0].stLink.stAdminSetting.enAutoNegotiation = Boolean_TRUE;
	pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[0].stLink.stAdminSetting.speed = SPEED_ETH;
	pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[0].stLink.stAdminSetting.enDuplex = Duplex_Full;
	pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[0].stLink.stOperSettings.enAutoNegotiation = Boolean_TRUE;
	pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[0].stLink.stOperSettings.speed = SPEED_ETH;
	pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[0].stLink.stOperSettings.enDuplex = Duplex_Full;
	pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[0].stLink.interfaceType = ETHERNET;
	
	pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[0].stIPv4.enEnabled = Boolean_TRUE;
	pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[0].stIPv4.stConfig.sizeManual = 1;
	pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[0].stIPv4.stConfig.enDHCP = stNicInfor.isDHCP;
	if(stNicInfor.isDHCP == 0)
	{
		strncpy(pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[0].stIPv4.stConfig.stManual[0].aszAddress, stNicInfor.ipAddr, MAX_ADDR_LENGTH-1);
		pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[0].stIPv4.stConfig.stManual[0].prefixLength = devMng_GetFrefixLength(stNicInfor.maskAddr);
	}
	else
	{
		strncpy(pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[0].stIPv4.stConfig.stFromDHCP.aszAddress, stNicInfor.ipAddr, MAX_ADDR_LENGTH-1);
		pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[0].stIPv4.stConfig.stFromDHCP.prefixLength = devMng_GetFrefixLength(stNicInfor.maskAddr);
	}
	strncpy(pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[0].stIPv4.stConfig.stLinkLocal.aszAddress, stNicInfor.ipAddr, MAX_ADDR_LENGTH-1);
	pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[0].stIPv4.stConfig.stLinkLocal.prefixLength = devMng_GetFrefixLength(stNicInfor.maskAddr);
	
    return ONVIF_OK;
    
Run_error:
    return ONVIF_ERR_DEVMNG_SERVER_RUN_ERROR;
    
}

void *config_network_static_thread(void* arg)
{
    ONVIF_DeviceNet_S *stNetConfigs = NULL;
    int ret;

    pthread_detach(pthread_self());
    sleep(2);
    stNetConfigs = (ONVIF_DeviceNet_S*)arg;
    if(NULL == stNetConfigs)
    {
        ONVIF_ERR("param error\n");
        return;
    }

    ret = devMng_SetNetConfigurations(stNetConfigs);
    if(ret != 0)
    {
        ONVIF_ERR("Fail to set net info");
    }    
    free(stNetConfigs);
}
void *config_network_dhcp_thread(void* arg)
{
    int ret;

    pthread_detach(pthread_self());
    sleep(2);

    ret = g_stDeviceAdapter.SetNetworkDHCP(g_GkIpc_OnvifInf.discov.nicName, 1);
    if(ret != 0)
    {
        ONVIF_ERR("Fail to set DHCP(1).");
    }
    //GK_NVT_SendHelloBye(GK_Hello);
         
    
}

ONVIF_RET GK_NVT_DevMng_SetNetworkInterfaces(GONVIF_DEVMNG_SetNetworkInterfaces_S *pstDM_SetNetworkInterfaces, GONVIF_DEVMNG_SetNetworkInterfaces_Res_S *pstDM_SetNetworkInterfacesRes)
{
    GK_S32 ret = 0;        
    ONVIF_DeviceNet_S *stNetConfigs = NULL;
        
    if(pstDM_SetNetworkInterfaces->aszInterfaceToken[0] == '\0' ||
        (pstDM_SetNetworkInterfaces->aszInterfaceToken[0] != '\0' && 
            strcmp(g_GkIpc_OnvifInf.discov.nicName, pstDM_SetNetworkInterfaces->aszInterfaceToken) != 0))
    {
        ONVIF_ERR("Invalied NIC token.");
        goto invalid_adapt;
    }

    if (Boolean_FALSE == pstDM_SetNetworkInterfaces->stNetworkInterface.enEnable)
    {
       g_GkIpc_OnvifInf.devmgmt.NIC_alive = GK_FALSE;
    }

    ret = g_stDeviceAdapter.SetNetworkMTU(g_GkIpc_OnvifInf.discov.nicName, pstDM_SetNetworkInterfaces->stNetworkInterface.MTU);
    if(ret != 0)
    {
        ONVIF_ERR("Invalied NIC token.");
        goto Run_Error;
    }

    ONVIF_INFO("pstDM_SetNetworkInterfaces->stNetworkInterface.stIPv4.enDHCP: %d\n", pstDM_SetNetworkInterfaces->stNetworkInterface.stIPv4.enDHCP);
    if(pstDM_SetNetworkInterfaces->stNetworkInterface.stIPv4.enDHCP == Boolean_FALSE)
    {
        ret = g_stDeviceAdapter.SetNetworkDHCP(g_GkIpc_OnvifInf.discov.nicName, 0);
        if(ret != 0)
        {
            ONVIF_ERR("Fail to set DHCP(0).");
            goto Run_Error;
        }
        ONVIF_INFO("ip-->:%s\n", pstDM_SetNetworkInterfaces->stNetworkInterface.stIPv4.stManual[0].aszAddress);
        ret = devMng_check_IP(pstDM_SetNetworkInterfaces->stNetworkInterface.stIPv4.stManual[0].aszAddress, NetworkHostType_IPv4);
        if(ret != 0)
        {
            ONVIF_ERR("Invalid IP format.");
            goto invalid_IPv4;
        }  
        stNetConfigs = malloc(sizeof(ONVIF_DeviceNet_S));
        if(NULL == stNetConfigs)
        {
            ONVIF_ERR("malloc fail\n");
            goto Run_Error;
        }
            
        memset(stNetConfigs, 0, sizeof(ONVIF_DeviceNet_S));
        strncpy(stNetConfigs->ipAddr, pstDM_SetNetworkInterfaces->stNetworkInterface.stIPv4.stManual[0].aszAddress, MAX_16_LENGTH);
        ret = devMng_GetMask(pstDM_SetNetworkInterfaces->stNetworkInterface.stIPv4.stManual[0].prefixLength, stNetConfigs->maskAddr);
        if(ret != 0)
        {
            ONVIF_ERR("Fail to get mask according to prefixLength");
            goto Run_Error;
        }
        pthread_t pid;
        if(pthread_create(&pid, NULL, config_network_static_thread, stNetConfigs))
            goto Run_Error;
    }
    else if(pstDM_SetNetworkInterfaces->stNetworkInterface.stIPv4.enDHCP == Boolean_TRUE)
    {
        pthread_t pid;
        if(pthread_create(&pid, NULL, config_network_dhcp_thread, NULL))
            goto Run_Error;
    }
    pstDM_SetNetworkInterfacesRes->enRebootNeeded = Boolean_FALSE;
    //if enRebootNeeded is true, client will send SystemReboot command to server,
    //After server started, the IP of Server should be what that client set right now, and Server
    //should use current IP to send Hello message to client., but the problem is that IP will not change
    //when system rebooted currently.

    GK_NVT_SaveXmlConfig(XML_DEVICE);
    return ONVIF_OK;
Run_Error:
    return ONVIF_ERR_DEVMNG_SERVER_RUN_ERROR;
invalid_adapt:
    return ONVIF_ERR_DEVMNG_INVALID_NETWORK_TOKEN;
invalid_IPv4:
    return ONVIF_ERR_DEVMNG_INVALID_IPV4_ADDR;
}

ONVIF_RET GK_NVT_DevMng_GetNetworkProtocols(GONVIF_DEVMNG_GetNetworkProtocols_Res_S *pstDM_GetNetworkProtocolRes)
{
    GK_S32 i = 0;
    GK_S32 size = 0;
    size = g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.sizeNetworkProtocols;
    pstDM_GetNetworkProtocolRes->sizeNetworkProtocols = size;
    for(i = 0; i < size; i++)
    {
        pstDM_GetNetworkProtocolRes->stNetworkProtocols[i].enName = g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.stNetworkProtocols[i].enName;
        pstDM_GetNetworkProtocolRes->stNetworkProtocols[i].enEnabled = g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.stNetworkProtocols[i].enEnabled;
        pstDM_GetNetworkProtocolRes->stNetworkProtocols[i].sizePort = g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.stNetworkProtocols[i].sizePort;
        pstDM_GetNetworkProtocolRes->stNetworkProtocols[i].port[0] = g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.stNetworkProtocols[i].port[0];
    }

    return ONVIF_OK;
}

ONVIF_RET GK_NVT_DevMng_SetNetworkProtocols(GONVIF_DEVMNG_SetNetworkProtocols_S *pstDM_SetNetworkProtocols)
{
    GK_S32 i = 0, j = 0;
    for(i = 0; i < pstDM_SetNetworkProtocols->sizeNetworkProtocols; i++)
    {
        for(j = 0; j < g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.sizeNetworkProtocols; j++)
        {
            if(g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.stNetworkProtocols[j].enName == pstDM_SetNetworkProtocols->stNetworkProtocols[i].enName)
            {
                g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.stNetworkProtocols[j].enEnabled = pstDM_SetNetworkProtocols->stNetworkProtocols[i].enEnabled;
                g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.stNetworkProtocols[j].sizePort = pstDM_SetNetworkProtocols->stNetworkProtocols[i].sizePort;
                GK_S32 index = 0;
                for(index = 0; index < pstDM_SetNetworkProtocols->stNetworkProtocols[i].sizePort; index++)
                {
                    g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.stNetworkProtocols[j].port[index] = pstDM_SetNetworkProtocols->stNetworkProtocols[i].port[index];
                }
                break;
            }
        }
        if(j == g_GkIpc_OnvifInf.devmgmt.stNetworkProtocolsInf.sizeNetworkProtocols)
        {
            goto invalid_protocols;
        }
    }
    
    GK_NVT_SaveXmlConfig(XML_DEVICE);
    return ONVIF_OK;
    
invalid_protocols:
    return ONVIF_ERR_DEVMNG_NETWORK_PROTOCOL_NOT_SUPPORT;
}

ONVIF_RET GK_NVT_DevMng_GetNetworkDefaultGateway(GONVIF_DEVMNG_GetNetworkDefaultGateway_Res_S *pstDM_GetNetworkDefaultGatewayRes)
{
	GK_S32 ret = 0;
    ONVIF_DeviceNet_S stNicInfor;
    memset(&stNicInfor, 0, sizeof(ONVIF_DeviceNet_S));
    ret = g_stDeviceAdapter.GetNetworkInfor(g_GkIpc_OnvifInf.discov.nicName, &stNicInfor);
    if(ret != 0)
    {
        ONVIF_ERR("Fail to get NIC infor.");
        goto run_error;
    }
    pstDM_GetNetworkDefaultGatewayRes->stNetworkGateway.sizeIPv4Address = 1;
    strncpy(pstDM_GetNetworkDefaultGatewayRes->stNetworkGateway.aszIPv4Address[0], stNicInfor.gwAddr, IPV4_STR_LENGTH-1);

    return ONVIF_OK;
    
run_error:
    return ONVIF_ERR_DEVMNG_SERVER_RUN_ERROR;
}


ONVIF_RET GK_NVT_DevMng_SetNetworkDefaultGateway(GONVIF_DEVMNG_SetNetworkDefaultGateway_S *pstDM_SetNetworkDefaultGateway)
{
    GK_S32 i = 0;
    GK_S32 ret = 0;
    if(pstDM_SetNetworkDefaultGateway->aszIPv4Address[0][0] == '\0')
		goto invalid_IPv4;
		
    while(i < pstDM_SetNetworkDefaultGateway->sizeIPv4Address)
    {
        if(pstDM_SetNetworkDefaultGateway->aszIPv4Address[i][0] != '\0')
        {
        	if(devMng_check_IP(pstDM_SetNetworkDefaultGateway->aszIPv4Address[i], NetworkHostType_IPv4))
            	goto invalid_IPv4;
        }
        i++;
    }

	GK_CHAR gwStr[IPV4_STR_LENGTH] = {0};
	strncpy(gwStr, pstDM_SetNetworkDefaultGateway->aszIPv4Address[0], IPV4_STR_LENGTH-1);
	ret = g_stDeviceAdapter.SetNetworkGateway(g_GkIpc_OnvifInf.discov.nicName, gwStr);
	if(ret != 0)
	{
		ONVIF_ERR("Fail to set gateway.");
		goto run_error;
	}
	
    return ONVIF_OK;

run_error:
    return ONVIF_ERR_DEVMNG_SERVER_RUN_ERROR;
invalid_IPv4:
    return ONVIF_ERR_DEVMNG_INVALID_IPV4_ADDR;
//invalid_IPv6:
    //return ONVIF_ERR_DEVMNG_INVALID_IPV6_ADDR;
}


/****************************** network end ************************************/

//*****************************************************************************
//*****************************************************************************
//** API Functions : System
//*****************************************************************************
//*****************************************************************************

ONVIF_RET GK_NVT_DevMng_SystemReboot(GONVIF_DEVMGN_SystemReboot_Res_S *pstDM_SystemRebootRes)
{
    strncpy(pstDM_SystemRebootRes->aszMessage, "Rebooting in 5 seconds.", MAX_MESSAGE_LENGTH-1);
    
    g_stDeviceAdapter.SetSystemReboot(5);
    GK_NVT_SendHelloBye(GK_Bye);
    return ONVIF_OK;
}

ONVIF_RET GK_NVT_DevMng_GetSystemDateAndTime(GONVIF_DEVMNG_GetSystemDateAndTime_Res_S *pstDM_GetSystemDateAndTimeRes)
{
	GK_S32 ret = 0;    
	GK_S32 timeZone = 0;
    GK_CHAR localTimeStr[MAX_TIME_LEN] = {0};
    ONVIF_DeviceNTP_S deviceNTP;
    memset(&deviceNTP, 0, sizeof(ONVIF_DeviceNTP_S));
    ret = g_stDeviceAdapter.GetNetworkNTP(&deviceNTP);
    if(ret != 0)
    {
		ONVIF_ERR("Fail to get NTP parameters.");
		goto run_error;
    }
    if(deviceNTP.isNTP == 0)
		pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.enDateTimeType = SetDateTimeType_Manual;
	else
		pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.enDateTimeType = SetDateTimeType_NTP;
	
    pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.enDaylightSavings = 0;
    ret = g_stDeviceAdapter.GetSystemDateAddTime(localTimeStr, &timeZone);
    if(ret != 0)
    {
		ONVIF_ERR("Fail to get local time.");
		goto run_error;
    }
    if(timeZone < 0)
    	snprintf(pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stTimeZone.aszTZ, MAX_TIME_ZONE_LENGTH, "GMT-%d", timeZone);
	else
    	snprintf(pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stTimeZone.aszTZ, MAX_TIME_ZONE_LENGTH, "GMT+%d", timeZone);

    time_t now;
    time(&now);    
    struct tm *timenow;
    timenow = localtime(&now);

    pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stLocalDateTime.stTime.hour   = timenow->tm_hour;
    pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stLocalDateTime.stTime.minute = timenow->tm_min;
    pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stLocalDateTime.stTime.second = timenow->tm_sec;
    pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stLocalDateTime.stDate.year   = timenow->tm_year + START_TIME_YEAR;
    pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stLocalDateTime.stDate.month  = timenow->tm_mon + 1;
    pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stLocalDateTime.stDate.day    = timenow->tm_mday;
   
    timenow = gmtime(&now);
    pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stUTCDateTime.stTime.hour   = timenow->tm_hour;
    pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stUTCDateTime.stTime.minute = timenow->tm_min;
    pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stUTCDateTime.stTime.second = timenow->tm_sec;
    pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stUTCDateTime.stDate.year   = timenow->tm_year + START_TIME_YEAR;
    pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stUTCDateTime.stDate.month  = timenow->tm_mon + 1;
    pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stUTCDateTime.stDate.day    = timenow->tm_mday;
    
    return ONVIF_OK;
    
run_error:
	return ONVIF_ERR_DEVMNG_SERVER_RUN_ERROR;
}

ONVIF_RET GK_NVT_DevMng_SetSystemDateAndTime(GONVIF_DEVMNG_SetSystemDateAndTime_S *pstDM_SetSystemDateAndTime)
{
	GK_S32  ret = 0;
    GK_S32  time_zone = 0;
    GK_CHAR timeBuf[MAX_TIME_LENGTH];
    if(pstDM_SetSystemDateAndTime->enDateTimeType == SetDateTimeType_Manual)
    {
		if(pstDM_SetSystemDateAndTime->stTimeZone.aszTZ[0] != '\0')
		{
			GK_CHAR *zoneFormat = pstDM_SetSystemDateAndTime->stTimeZone.aszTZ;			
			ONVIF_INFO("time zone format: %s", zoneFormat);
			if(zoneFormat != NULL)
			{
				if(strncmp(zoneFormat, "GMT--8", 6) != 0 && strncmp(zoneFormat, "GMT+8", 5) != 0)
				{
					GK_S32 i = 0;
					GK_CHAR *flag = NULL;
					GK_CHAR *second = NULL;
					flag = strchr(zoneFormat, '-');
					while(!isdigit(*zoneFormat) && i < MAX_TIME_ZONE_LENGTH)
					{
						zoneFormat++;
						i++;
					}
					if(i >= MAX_TIME_ZONE_LENGTH)
					{
						time_zone = 8*60;
					}
					else
					{
						if(flag != NULL)
							time_zone = 60*atoi(zoneFormat);
						else
							time_zone = -60*atoi(zoneFormat);
						second = strchr(zoneFormat, ':');
						if(second != NULL)
						{
							if(flag != NULL)
								time_zone += atoi(second+1);
							else
								time_zone -= atoi(second+1);
						}
					}
				}
				else if(strncmp(zoneFormat, "GMT+8", 5) == 0 || strncmp(zoneFormat, "GMT--8", 6) == 0)	
				{			
					time_zone = 480;
					if(strncmp(zoneFormat, "GMT--8", 6) == 0)
						pstDM_SetSystemDateAndTime->stUTCDateTime.stTime.hour -= 8;
				}
				ONVIF_INFO("time zone: %d", time_zone);
				if(time_zone < -720 || time_zone > 720)
				{
					ONVIF_ERR("Invalid time zone.");
					goto invalid_timezone;
				}
			}
			else
			{
				time_zone = 8*60;
			}

			
		}
		else
		{
			time_zone = 8*60;
		}
        if((pstDM_SetSystemDateAndTime->stUTCDateTime.stDate.year < 0)  ||
            (pstDM_SetSystemDateAndTime->stUTCDateTime.stDate.month < 1) || (pstDM_SetSystemDateAndTime->stUTCDateTime.stDate.month > 12) ||
            (pstDM_SetSystemDateAndTime->stUTCDateTime.stDate.day < 1) || (pstDM_SetSystemDateAndTime->stUTCDateTime.stDate.day > 31) ||
            (pstDM_SetSystemDateAndTime->stUTCDateTime.stTime.hour < 0) || (pstDM_SetSystemDateAndTime->stUTCDateTime.stTime.hour > 59) ||
            (pstDM_SetSystemDateAndTime->stUTCDateTime.stTime.minute < 0) || (pstDM_SetSystemDateAndTime->stUTCDateTime.stTime.minute > 59)||
            (pstDM_SetSystemDateAndTime->stUTCDateTime.stTime.second < 0) || (pstDM_SetSystemDateAndTime->stUTCDateTime.stTime.second > 59))
        {
        	ONVIF_ERR("Invalid UTC time.");
            goto invalid_datatime;
        }
        snprintf(timeBuf, MAX_TIME_LENGTH, "%04d-%02d-%02d %02d:%02d:%02d",
            pstDM_SetSystemDateAndTime->stUTCDateTime.stDate.year,
            pstDM_SetSystemDateAndTime->stUTCDateTime.stDate.month,
            pstDM_SetSystemDateAndTime->stUTCDateTime.stDate.day,
            pstDM_SetSystemDateAndTime->stUTCDateTime.stTime.hour,
            pstDM_SetSystemDateAndTime->stUTCDateTime.stTime.minute,
            pstDM_SetSystemDateAndTime->stUTCDateTime.stTime.second);
        ONVIF_INFO("UTCDateTime: %s", timeBuf);
		ret = g_stDeviceAdapter.SetSystemDateAddTime(timeBuf, time_zone);
		if(ret != 0)
		{
			ONVIF_ERR("Fail to set UTC or timeZone.");
			goto run_error;
		}
    }
    else if(pstDM_SetSystemDateAndTime->enDateTimeType == SetDateTimeType_NTP)
    {
		ONVIF_DeviceNTP_S ntpConfig;
		memset(&ntpConfig, 0, sizeof(ONVIF_DeviceNTP_S));
		ntpConfig.isNTP = 1;
		ret = g_stDeviceAdapter.SetNetworkNTP(ntpConfig);
		if(ret != 0)
		{
			ONVIF_ERR("Fail to start NTP service.");
			goto run_error;
		}
    }
    else
    {
       goto invalid_DateTimeType;     
    }

    GK_NVT_SaveXmlConfig(XML_DEVICE);

    return ONVIF_OK;

run_error:
    return ONVIF_ERR_DEVMNG_SERVER_RUN_ERROR;
invalid_DateTimeType:
    return ONVIF_ERR_DEVMNG_INVALID_DATETIMETYPE;
invalid_timezone:
    return ONVIF_ERR_DEVMNG_INVALID_TIMEZONE;
invalid_datatime:
    return ONVIF_ERR_DEVMNG_INVALID_DATETIME;
    
}

ONVIF_RET GK_NVT_DevMng_GetDeviceInformation(GONVIF_DEVMNG_GetDeviceInformation_Res_S *pstDM_GetDeviceInformationRes)
{
	
    g_stDeviceAdapter.GetSystemDeviceInfor(g_GkIpc_OnvifInf.discov.nicName, pstDM_GetDeviceInformationRes);  
    return ONVIF_OK;
}

ONVIF_RET GK_NVT_DevMng_SetSystemFactoryDefault(GONVIF_DEVMNG_SetSystemFactoryDefault_S *pstDM_SetSystemFactoryDefault)
{
	GK_S32 ret = 0;	
	ONVIF_DeviceFactory_E enFactoryType = Factory_soft;
    if (pstDM_SetSystemFactoryDefault->enFactoryDefault == FactoryDefaultType_Soft)
		enFactoryType = Factory_soft;
    else if(pstDM_SetSystemFactoryDefault->enFactoryDefault == FactoryDefaultType_Hard)
		enFactoryType = Factory_hard;
    
	ret = g_stDeviceAdapter.SetSystemFactoryDefault(enFactoryType);
	if(ret != 0)
	{
		ONVIF_ERR("Fail to set factory default.");
		goto Run_Error;
	}

    GK_NVT_SendHelloBye(GK_Bye);

    return ONVIF_OK;
    
Run_Error:
	return ONVIF_ERR_DEVMNG_SERVER_RUN_ERROR;
}

ONVIF_RET GK_NVT_DevMng_GetScopes(GONVIF_DEVMNG_GetScopes_Res_S *pstDM_GetScopesRes)
{
    GK_U8 i;
    pstDM_GetScopesRes->sizeScopes = g_GkIpc_OnvifInf.discov.Scopecount;
    if(pstDM_GetScopesRes->sizeScopes <= 0)
    {
        goto empty_scope;
    }
    else
    {
        if(pstDM_GetScopesRes->sizeScopes > MAX_SCOPE_NUM)
            pstDM_GetScopesRes->sizeScopes = MAX_SCOPE_NUM;
    }
    pstDM_GetScopesRes->pstScopes = (GONVIF_DEVMNG_Scopes_S *)malloc((pstDM_GetScopesRes->sizeScopes)*sizeof(GONVIF_DEVMNG_Scopes_S));
    memset(pstDM_GetScopesRes->pstScopes, 0, (pstDM_GetScopesRes->sizeScopes) * sizeof(GONVIF_DEVMNG_Scopes_S));
    for(i = 0; i < pstDM_GetScopesRes->sizeScopes; i++)
    {
        strncpy(pstDM_GetScopesRes->pstScopes[i].aszScopeItem, g_GkIpc_OnvifInf.discov.Scopes[i]->aszScopeItem, LEN_SCOPE_ITEM-1);
        pstDM_GetScopesRes->pstScopes[i].enScopeDef = g_GkIpc_OnvifInf.discov.Scopes[i]->enScopeDef;
    }

    return ONVIF_OK;

empty_scope:
    return ONVIF_ERR_DEVMNG_EMPTY_SCOPE;
}

ONVIF_RET GK_NVT_DevMng_SetScopes(GONVIF_DEVMNG_SetScopes_S *pstDM_SetScopes)
{
    GK_U8 i = 0,j = 0;
    
    while(j < pstDM_SetScopes->sizeScopes)
    {
        for(i = 0; i < g_GkIpc_OnvifInf.discov.Scopecount; i++)
        {
            if(GK_TRUE == devMng_MatchString(pstDM_SetScopes->pszScopes[j], g_GkIpc_OnvifInf.discov.Scopes[i]->aszScopeItem))
   			{
   			    if(g_GkIpc_OnvifInf.discov.Scopes[i]->enScopeDef == ScopeDefinition_Fixed)
                {
                    goto scope_overwrite;
                }
    		}
    	}
        j++;
    }
    //Here has a question: remove all configurable scope or remove coresponding configurable scope like 'name'/'location'/'hardware'/'type' ?    
    /*remove all configurable scope*/
    for(i = 0; i < g_GkIpc_OnvifInf.discov.Scopecount; i++)
    {
        if(g_GkIpc_OnvifInf.discov.Scopes[i]->enScopeDef == ScopeDefinition_Configurable)
        {
            GK_U8 curIndex = i;
            for(j = i + 1; j < g_GkIpc_OnvifInf.discov.Scopecount; j++)
            {
                g_GkIpc_OnvifInf.discov.Scopes[curIndex]->enScopeDef = g_GkIpc_OnvifInf.discov.Scopes[j]->enScopeDef;
                memset(g_GkIpc_OnvifInf.discov.Scopes[curIndex]->aszScopeItem, 0, LEN_WEBSERVICE_URL * sizeof(GK_CHAR));
                strncpy(g_GkIpc_OnvifInf.discov.Scopes[curIndex]->aszScopeItem,g_GkIpc_OnvifInf.discov.Scopes[j]->aszScopeItem, LEN_SCOPE_ITEM-1);
                curIndex++;
            }
            g_GkIpc_OnvifInf.discov.Scopecount--;
            free(g_GkIpc_OnvifInf.discov.Scopes[g_GkIpc_OnvifInf.discov.Scopecount]);
            g_GkIpc_OnvifInf.discov.Scopes[g_GkIpc_OnvifInf.discov.Scopecount] = NULL;
            i--;
        }
    }

    GONVIF_DEVMNG_Scopes_S scope;
    for (i = 0; i< pstDM_SetScopes->sizeScopes; i++)
    {
        memset(&scope, 0, sizeof(scope));
        strncpy(scope.aszScopeItem, pstDM_SetScopes->pszScopes[i], LEN_SCOPE_ITEM-1);
        scope.enScopeDef = ScopeDefinition_Configurable;
        if(g_GkIpc_OnvifInf.discov.Scopecount == MAX_SCOPE_NUM)
        {
            goto too_many_scopes;
        }
        devMng_AddConfigurableScope(&scope);
    }
    
	GK_NVT_SaveXmlConfig(XML_DISCOVRY);
	GK_NVT_SendHelloBye(GK_Hello);
	
    return ONVIF_OK;

scope_overwrite:
    return ONVIF_ERR_DEVMNG_SCOPE_OVERWRITE;
too_many_scopes:
    return ONVIF_ERR_DEVMNG_TOO_MANY_SCOPES;

}

#ifdef MODULE_SUPPORT_ZK_WAPI
void GK_NVT_DevMng_GetScopes_name_location(char *name, char *location)
{
    GK_CHAR *sp;
    GK_U8 i = 0;
    
    for(i = 0; i < g_GkIpc_OnvifInf.discov.Scopecount; i++)
    {
        if(g_GkIpc_OnvifInf.discov.Scopes[i]->enScopeDef == ScopeDefinition_Configurable)
        {
            sp = strstr(g_GkIpc_OnvifInf.discov.Scopes[i]->aszScopeItem, "name");
            if(NULL != sp)
                strcpy(name, sp+5);
            sp = strstr(g_GkIpc_OnvifInf.discov.Scopes[i]->aszScopeItem, "location");
            if(NULL != sp)
                strcpy(location, sp+9);
        }
    }
}

void GK_NVT_DevMng_SetScopes_name_location(char *name, char *location)
{
    GK_CHAR *sp;
    GK_U8 i = 0;
    
    for(i = 0; i < g_GkIpc_OnvifInf.discov.Scopecount; i++)
    {
        if(g_GkIpc_OnvifInf.discov.Scopes[i]->enScopeDef == ScopeDefinition_Configurable)
        {
            sp = strstr(g_GkIpc_OnvifInf.discov.Scopes[i]->aszScopeItem, "name");
            if(NULL != sp)
                strcpy(sp+5, name);
            sp = strstr(g_GkIpc_OnvifInf.discov.Scopes[i]->aszScopeItem, "location");
            if(NULL != sp)
                strcpy(sp+9, location);
        }
    }	
	GK_NVT_SaveXmlConfig(XML_DISCOVRY);
	GK_NVT_SendHelloBye(GK_Hello);
}
#endif

ONVIF_RET GK_NVT_DevMng_AddScopes(GONVIF_DEVMNG_AddScopes_S *pstDM_AddScopes)
{
    GK_U8 i = 0;
    GONVIF_DEVMNG_Scopes_S stScope;

    for(i = 0; i < pstDM_AddScopes->sizeScopeItem; i++)
    {
        if(g_GkIpc_OnvifInf.discov.Scopecount == MAX_SCOPE_NUM)
        {
            goto too_many_scopes;
        }
        memset(&stScope, 0, sizeof(stScope));
        stScope.enScopeDef = ScopeDefinition_Configurable;
        strncpy(stScope.aszScopeItem, pstDM_AddScopes->pszScopeItem[i], LEN_SCOPE_ITEM-1);
        devMng_AddConfigurableScope(&stScope);
    }
    
	GK_NVT_SaveXmlConfig(XML_DISCOVRY);
	GK_NVT_SendHelloBye(GK_Hello);

    return ONVIF_OK;
    
too_many_scopes:
    return ONVIF_ERR_DEVMNG_TOO_MANY_SCOPES;
}


ONVIF_RET GK_NVT_DevMng_RemoveScopes(GONVIF_DEVMNG_RemoveScopes_S *pstDM_RemoveScopes, GONVIF_DEVMNG_RemoveScopes_Res_S *pstDM_RemoveScopesRes)
{
    ONVIF_RET ret = ONVIF_OK;
    GK_U8 i = 0;
    GONVIF_DEVMNG_Scopes_S stScope;
    
    for(i = 0; i < pstDM_RemoveScopes->sizeScopeItem; i++)
    {
        memset(&stScope, 0, sizeof(GONVIF_DEVMNG_Scopes_S));
        stScope.enScopeDef = ScopeDefinition_Configurable;
        strncpy(stScope.aszScopeItem, pstDM_RemoveScopes->pszScopeItem[i], LEN_SCOPE_ITEM-1);
        
        ret = devMng_RemoveConfigurableScope(&stScope);
        if(ret == ONVIF_ERR_DEVMNG_REMOVE_FIXED_SCOPE)
            goto remove_fixed_scope;
        else if(ret == ONVIF_ERR_DEVMNG_NO_SUCH_SCOPE)
            goto no_such_scope;
    }
    memcpy(pstDM_RemoveScopesRes, pstDM_RemoveScopes, sizeof(GONVIF_DEVMNG_RemoveScopes_S));
    
	GK_NVT_SaveXmlConfig(XML_DISCOVRY);
	GK_NVT_SendHelloBye(GK_Hello);

    return ONVIF_OK;

remove_fixed_scope:
    return ONVIF_ERR_DEVMNG_REMOVE_FIXED_SCOPE;
no_such_scope:
    return ONVIF_ERR_DEVMNG_NO_SUCH_SCOPE;
}


ONVIF_RET GK_NVT_DevMng_GetDiscoveryMode(GONIVF_DEVMNG_GetDiscoveryMode_Res_S *pstDM_GetDiscoveryModeRes)
{
    pstDM_GetDiscoveryModeRes->enDiscoveryMode = g_GkIpc_OnvifInf.discov.discoverymode;
    
	GK_NVT_SendHelloBye(GK_Hello);

    return ONVIF_OK;
}

ONVIF_RET GK_NVT_DevMng_SetDiscoveryMode(GONIVF_DEVMNG_SetDiscoveryMode_S *pstDM_SetDiscoveryMode)
{
	g_GkIpc_OnvifInf.discov.discoverymode = pstDM_SetDiscoveryMode->enDiscoveryMode;

	GK_NVT_SaveXmlConfig(XML_DISCOVRY);	
	GK_NVT_SendHelloBye(GK_Hello);

	return ONVIF_OK;
}

/***************************** System end ****************************************/

//*****************************************************************************
//*****************************************************************************
//** API Functions : Security
//*****************************************************************************
//*****************************************************************************

ONVIF_RET GK_NVT_DevMng_GetUsers(GONVIF_DEVMNG_GetUsers_Res_S *pstDM_GetUsersRes)
{
    GK_U8 i = 0;
    GK_S32 ret = 0;
	ret = devMng_SynUsers();
	if(ret != 0)
	{
		ONVIF_ERR("Fail to synchronise user info");
		goto run_error;
	}
	pstDM_GetUsersRes->sizeUser = g_GkIpc_OnvifInf.devmgmt.Usercount;
	while(i < pstDM_GetUsersRes->sizeUser)
	{
		strncpy(pstDM_GetUsersRes->stUser[i].aszUsername, g_GkIpc_OnvifInf.devmgmt.User[i]->aszUsername, MAX_USERNAME_LENGTH-1);
		pstDM_GetUsersRes->stUser[i].enUserLevel = g_GkIpc_OnvifInf.devmgmt.User[i]->enUserLevel;
		i++;
	}

    return ONVIF_OK;
run_error:
	return ONVIF_ERR_DEVMNG_SERVER_RUN_ERROR;
}

ONVIF_RET GK_NVT_DevMng_CreateUsers(GONVIF_DEVMNG_CreateUsers_S *pstDM_CreateUsers)
{
    GK_U8  i = 0;
    GK_S32 ret = 0;
    GK_U8  count = pstDM_CreateUsers->sizeUser;
    GK_S8  *userName = NULL;
	ret = devMng_SynUsers();
	if(ret != 0)
	{
		ONVIF_ERR("Fail to synchronise user info");
		goto run_error;
	}
    for(i = 0; i < count; i++)
    {
        if(strlen(pstDM_CreateUsers->stUser[i].aszUsername) < 5)
            goto username_too_short;
        else if(strlen(pstDM_CreateUsers->stUser[i].aszUsername) > 28)
            goto username_too_long;
        else if(strlen(pstDM_CreateUsers->stUser[i].aszPassword) < 5)
            goto password_too_short;
        else if(strlen(pstDM_CreateUsers->stUser[i].aszPassword) > 28)
            goto password_too_long;
        else if(pstDM_CreateUsers->stUser[i].enUserLevel < UserLevel_Administrator || pstDM_CreateUsers->stUser[i].enUserLevel > UserLevel_Extended)
            goto userLevel_not_allowed;
  
        userName = pstDM_CreateUsers->stUser[i].aszUsername;
        
        if(config_FindUser(userName, g_GkIpc_OnvifInf.devmgmt.User) >= 0)
        {
            goto name_clash;
        }
        if(g_GkIpc_OnvifInf.devmgmt.Usercount >= MAX_USER_NUM)
        {
            goto mum_exceeded;
        }
		if(g_stDeviceAdapter.SystemGetUsers && g_stDeviceAdapter.SystemCreateUsers && \
			g_stDeviceAdapter.SystemDeleteUsers && g_stDeviceAdapter.SystemSetUser)
		{
			ret = g_stDeviceAdapter.SystemCreateUsers(pstDM_CreateUsers->stUser[i]);
			if(ret != 0)
			{
				ONVIF_ERR("Fail to add new user to system by ONVIF.");
				goto run_error;
			}			
		}
		config_AddUser(&pstDM_CreateUsers->stUser[i]);
    }

    GK_NVT_SaveXmlConfig(XML_DEVICE);
    return ONVIF_OK;
    
name_clash:
    return ONVIF_ERR_DEVMNG_USERNAME_CLASH;
mum_exceeded:
    return ONVIF_ERR_DEVMNG_USER_MUM_EXCEED;
username_too_short:
    return ONVIF_ERR_DEVMNG_USERNAME_TOO_SHORT;
password_too_short:
    return ONVIF_ERR_DEVMNG_PASSWORD_TOO_SHORT;
username_too_long:
    return ONVIF_ERR_DEVMNG_USERNAME_TOO_LONG;
password_too_long:
    return ONVIF_ERR_DEVMNG_PASSWORD_TOO_LONG;
userLevel_not_allowed:
    return ONVIF_ERR_DEVMNG_USERLEVEL_NOT_ALLOWED;
run_error:
	return ONVIF_ERR_DEVMNG_SERVER_RUN_ERROR;
}


ONVIF_RET GK_NVT_DevMng_DeleteUsers(GONVIF_DEVMNG_DeleteUsers_S *pstDM_DeleteUsers)
{
	GK_S32 count = pstDM_DeleteUsers->sizeUsername;
	GK_S32 i = 0;
	GK_S32 index;
	GK_CHAR *name = NULL;
	GK_S32 ret = 0;
	ret = devMng_SynUsers();
	if(ret != 0)
	{
		ONVIF_ERR("Fail to synchronise user info");
		goto run_error;
	}
	for(i = 0; i < count; i++)
	{
		name = pstDM_DeleteUsers->aszUsername[i];
		if ((index = config_FindUser(name, g_GkIpc_OnvifInf.devmgmt.User)) < 0)
		{
			goto username_missing;
		}
		if(strcmp(g_GkIpc_OnvifInf.devmgmt.User[index]->aszUsername, "admin") == 0)
		{
			goto fixed_user;
		}
	}
	for(i = 0; i < count; i++)
	{
		name = pstDM_DeleteUsers->aszUsername[i];
		if(g_stDeviceAdapter.SystemGetUsers && g_stDeviceAdapter.SystemCreateUsers && \
		  g_stDeviceAdapter.SystemDeleteUsers && g_stDeviceAdapter.SystemSetUser)
		{
			ret = g_stDeviceAdapter.SystemDeleteUsers(name);
			if(ret != 0)
			{
				ONVIF_ERR("Fail to delete user from system by ONVIF.");
				goto run_error;
			}
		}
		config_DeleteUser(name);
	}

	GK_NVT_SaveXmlConfig(XML_DEVICE);
	return ONVIF_OK;
  
username_missing:
	return ONVIF_ERR_DEVMNG_USERNAME_MISSING;
fixed_user:
	return ONVIF_ERR_DEVMNG_USERNAME_FIXED;
run_error:
	return ONVIF_ERR_DEVMNG_SERVER_RUN_ERROR;
}


ONVIF_RET GK_NVT_DevMng_SetUser(GONVIF_DEVMNG_SetUser_S *pstDM_SetUser)
{
    GK_S32 count = pstDM_SetUser->sizeUser;
    GK_S32 i = 0;
    GK_S32 index;
    GK_CHAR *name = NULL;
	GK_S32 ret = 0;
	ret = devMng_SynUsers();
	if(ret != 0)
	{
		ONVIF_ERR("Fail to synchronise user info");
		goto run_error;
	}
    for(i = 0; i < count; i++)
    {
        name = pstDM_SetUser->stUser[i].aszUsername;
        if(config_FindUser(name, g_GkIpc_OnvifInf.devmgmt.User) < 0)
        {
            goto username_missing;
        }
        else if(strlen(pstDM_SetUser->stUser[i].aszPassword) < 5)
        {
            goto password_too_short;
        }
        else if(strlen(pstDM_SetUser->stUser[i].aszPassword) > 28)
        {
            goto password_too_long;
        }
        else if(pstDM_SetUser->stUser[i].enUserLevel < UserLevel_Administrator || pstDM_SetUser->stUser[i].enUserLevel > UserLevel_Extended)
        {
            goto userLevel_not_allowed;
        }
    }
    for(i = 0; i < count; i++)
    {
        name = pstDM_SetUser->stUser[i].aszUsername;
		if(g_stDeviceAdapter.SystemGetUsers && g_stDeviceAdapter.SystemCreateUsers && \
		  g_stDeviceAdapter.SystemDeleteUsers && g_stDeviceAdapter.SystemSetUser)
		{
			ret = g_stDeviceAdapter.SystemSetUser(pstDM_SetUser->stUser[i]);
			if(ret != 0)
			{
				ONVIF_ERR("Fail to set user to system by ONVIF.");
				goto run_error;
			}
		}
        index = config_FindUser(name, g_GkIpc_OnvifInf.devmgmt.User);
        memset(g_GkIpc_OnvifInf.devmgmt.User[index]->aszPassword, 0, MAX_PASSWORD_LENGTH);
        strncpy(g_GkIpc_OnvifInf.devmgmt.User[index]->aszPassword, pstDM_SetUser->stUser[i].aszPassword, MAX_PASSWORD_LENGTH-1);
        g_GkIpc_OnvifInf.devmgmt.User[index]->enUserLevel = pstDM_SetUser->stUser[i].enUserLevel;
    }

    GK_NVT_SaveXmlConfig(XML_DEVICE);
    return ONVIF_OK;
    
username_missing:
    return ONVIF_ERR_DEVMNG_USERNAME_MISSING;
password_too_short:
    return ONVIF_ERR_DEVMNG_PASSWORD_TOO_SHORT;
password_too_long:
    return ONVIF_ERR_DEVMNG_PASSWORD_TOO_LONG;
userLevel_not_allowed:
    return ONVIF_ERR_DEVMNG_USERLEVEL_NOT_ALLOWED;
run_error:
	return ONVIF_ERR_DEVMNG_SERVER_RUN_ERROR;
}

/****************************** Security end *************************************/

//*****************************************************************************
//*****************************************************************************
//** API Functions : inside API
//*****************************************************************************
//*****************************************************************************
static GK_S32 devMng_SetNetConfigurations(ONVIF_DeviceNet_S *stNet)
{
    GK_S32 ret = 0;
    
    ret = g_stDeviceAdapter.SetNetworkIP(g_GkIpc_OnvifInf.discov.nicName, stNet->ipAddr);
    if(ret != 0)
    {
        ONVIF_ERR("Fail to set IP");
        return -1;
    }
    ret = g_stDeviceAdapter.SetNetworkMask(g_GkIpc_OnvifInf.discov.nicName, stNet->maskAddr);
    if(ret != 0)
    {
        ONVIF_ERR("Fail to set Mask");
        return -1;
    }

    return 0;    
}

static GK_S32 devMng_GetFrefixLength(GK_CHAR *netmask)
{
    if(netmask == NULL)
    {
        ONVIF_ERR("Invalid netmask.");
        return -1;
    }
    ONVIF_INFO("netmask: %s\n" , netmask);
    GK_S32 length = 0;
    GK_U32 mask = 0;
    mask = (GK_U32)inet_addr(netmask);
    while(mask)
    {
        mask >>= 1;
        length++;
    }
    
    return length;
}

static GK_S32 devMng_check_IP(const GK_CHAR *ipAddr, GONVIF_DEVMNG_NetworkHostType_E ipType)
{    
    GK_S32 ret = 0;
    GK_U32 ip[5] = {0};
    if(!ipAddr)
    {
        ONVIF_ERR("Have no IP string");
        goto Invalid_IP;
    }
    
    switch(ipType)
    {
        case NetworkHostType_IPv4:
            ret = sscanf(ipAddr, "%3d.%3d.%3d.%3d", &ip[0], &ip[1], &ip[2],&ip[3]);
            if(ret != 4) 
            {                
                ONVIF_ERR("Has no 4 data in IP string.");
                goto Invalid_IP;
            }   
            else if((ip[0] > 255 || ip[1] > 255 || ip[2] > 255 || ip[3] > 255) ||
            	(ip[0] < 0 || ip[1] < 0 || ip[2] < 0 || ip[3] < 0)||(ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0)) 
            {
                ONVIF_ERR("Any element is larger than 255 in IP string.");
                goto Invalid_IP;
            }
            break;
        case NetworkHostType_IPv6:    
            ONVIF_ERR("Unsupported IP format.");
            goto Invalid_IP;
            break;
        default:
            ONVIF_ERR("Unsupported IP format.");
            goto Invalid_IP;
            break;
    }
    
    return 0;
    
Invalid_IP:
    return -1;
}

static GK_S32 devMng_GetMask(GK_S32 prefixLength, GK_CHAR *maskStr)
{
    if(maskStr == NULL)
    {
        ONVIF_ERR("No enough memory in mask string.");
        return -1;
    }
    if(prefixLength != 8 && prefixLength != 16 
        && prefixLength != 24 && prefixLength != 32)
    {
        ONVIF_ERR("Invalid mask format.");
        return -1;
    }
    else if(prefixLength == 8)
        strncpy(maskStr, "255.0.0.0", MAX_16_LENGTH-1);
    else if(prefixLength == 16)
        strncpy(maskStr, "255.255.0.0", MAX_16_LENGTH-1);
    else if(prefixLength == 24)
        strncpy(maskStr, "255.255.255.0", MAX_16_LENGTH-1);
    else if(prefixLength == 32)
        strncpy(maskStr, "255.255.255.255", MAX_16_LENGTH-1);

    return 0;
}

GK_BOOL devMng_MatchString(const GK_CHAR *client, const GK_CHAR *server)
{
    const GK_CHAR *p = client;
    const GK_CHAR *s = server;

    while (*p!='\0' && *s!='\0')
    {
        if (*p != *s)
        {
            break;
        }
        p++;
        s++;
    }
    if (*p == '\0' && (*s == '/' || *s == '\0'))
        return GK_TRUE;
    else
        return GK_FALSE;
}
ONVIF_RET devMng_RemoveConfigurableScope(GONVIF_DEVMNG_Scopes_S *scope)
{
    GK_U8 i = 0, j = 0;  
    GK_BOOL doesExsit = GK_FALSE;
    for(i = 0; i < g_GkIpc_OnvifInf.discov.Scopecount; i++)
    {    
        if (GK_TRUE == devMng_MatchString(g_GkIpc_OnvifInf.discov.Scopes[i]->aszScopeItem, scope->aszScopeItem))
        {
            if(g_GkIpc_OnvifInf.discov.Scopes[i]->enScopeDef == ScopeDefinition_Configurable)
            {
                GK_U8 curIndex = i;
                for(j = i + 1; j < g_GkIpc_OnvifInf.discov.Scopecount; j++)
                {
                    g_GkIpc_OnvifInf.discov.Scopes[curIndex]->enScopeDef = g_GkIpc_OnvifInf.discov.Scopes[j]->enScopeDef;
                    memset(g_GkIpc_OnvifInf.discov.Scopes[curIndex]->aszScopeItem, 0, LEN_WEBSERVICE_URL * sizeof(GK_CHAR));
                    strncpy(g_GkIpc_OnvifInf.discov.Scopes[curIndex]->aszScopeItem,g_GkIpc_OnvifInf.discov.Scopes[j]->aszScopeItem, LEN_SCOPE_ITEM-1);
                    curIndex++;
                }
                g_GkIpc_OnvifInf.discov.Scopecount--;
                free(g_GkIpc_OnvifInf.discov.Scopes[g_GkIpc_OnvifInf.discov.Scopecount]);
                g_GkIpc_OnvifInf.discov.Scopes[g_GkIpc_OnvifInf.discov.Scopecount] = NULL;
                doesExsit = GK_TRUE;
                break;
            }
            else if(g_GkIpc_OnvifInf.discov.Scopes[i]->enScopeDef == ScopeDefinition_Fixed)
            {
                return ONVIF_ERR_DEVMNG_REMOVE_FIXED_SCOPE;
            }
        }
    }

    if(doesExsit == GK_FALSE)
    {
        return ONVIF_ERR_DEVMNG_NO_SUCH_SCOPE;
    }
   
    return ONVIF_OK;
}

GK_S32 devMng_AddConfigurableScope(GONVIF_DEVMNG_Scopes_S *scope)
{
    GONVIF_DEVMNG_Scopes_S *Scope = scope;
    if(Scope->aszScopeItem[0] != '\0')
    {
        GK_U8 count = g_GkIpc_OnvifInf.discov.Scopecount;
        g_GkIpc_OnvifInf.discov.Scopes[count] = (GONVIF_DEVMNG_Scopes_S *)malloc(sizeof(GONVIF_DEVMNG_Scopes_S));
        strncpy(g_GkIpc_OnvifInf.discov.Scopes[count]->aszScopeItem, Scope->aszScopeItem, LEN_SCOPE_ITEM-1);
        g_GkIpc_OnvifInf.discov.Scopes[count]->enScopeDef = Scope->enScopeDef;
        g_GkIpc_OnvifInf.discov.Scopecount++;
    }

    return 0;
}

GK_S32 devMng_SynUsers()
{
	GK_S32 i = 0;
    GK_S32 ret = 0;
    if(g_stDeviceAdapter.SystemGetUsers && g_stDeviceAdapter.SystemCreateUsers && \
    	g_stDeviceAdapter.SystemDeleteUsers && g_stDeviceAdapter.SystemSetUser)
    {
		ONVIF_INFO("users cfg is from exterior.");
		GONVIF_DEVMNG_GetUsers_Res_S usersCfg;
		memset(&usersCfg, 0, sizeof(GONVIF_DEVMNG_GetUsers_Res_S));
    	ret = g_stDeviceAdapter.SystemGetUsers(&usersCfg);
    	if(ret != 0)
    	{
			ONVIF_ERR("Fail to get users info from exterior");
			return -1;
    	}
		if(usersCfg.sizeUser >= MAX_USER_NUM)
		{
			ONVIF_INFO("Too many users.");
			return -1;
		}
		config_DeleteAllUsers();
		for(i = 0; i < usersCfg.sizeUser; i++)
		{
			config_AddUser(&usersCfg.stUser[i]);
		}
		GK_NVT_SaveXmlConfig(XML_DEVICE);
    }
	else
	{
		ONVIF_INFO("users cfg is from XML.");		
	}

	return 0;
}

/******************************* inside API end ***********************************/



