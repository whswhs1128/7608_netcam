/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif-no-gsoap/src/soap_packet.c
**
** \brief       soap packet
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include "soap_packet.h"
#include "event_handle.h"
#include "onvif_priv.h"

extern EVENT_PullMessages_Res_S g_stEventPullMessageRes;

/*************************** common data **************************************/

GK_CHAR xml_hdr[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";

GK_CHAR onvif_xmlns[] = "<s:Envelope "
					    "xmlns:s=\"http://www.w3.org/2003/05/soap-envelope\" "
					    "xmlns:e=\"http://www.w3.org/2003/05/soap-encoding\" "
					    "xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" "
					    "xmlns:wsa=\"http://www.w3.org/2005/08/addressing\" "
					    "xmlns:xmime=\"http://www.w3.org/2005/05/xmlmime\" "
					    "xmlns:tns1=\"http://www.onvif.org/ver10/topics\" "
					    "xmlns:xs=\"http://www.w3.org/2001/XMLSchema\" "
					    "xmlns:xop=\"http://www.w3.org/2004/08/xop/include\" "
					    "xmlns:wsnt=\"http://docs.oasis-open.org/wsn/b-2\" "
					    "xmlns:wstop=\"http://docs.oasis-open.org/wsn/t-1\" "
					    "xmlns:tds=\"http://www.onvif.org/ver10/device/wsdl\" "
                        "xmlns:tt=\"http://www.onvif.org/ver10/schema\" "
                        "xmlns:hikwsd=\"http://www.onvifext.com/onvif/ext/ver10/wsdl\" "
                        "xmlns:hikxsd=\"http://www.onvifext.com/onvif/ext/ver10/schema\" "
					    "xmlns:tan=\"http://www.onvif.org/ver20/analytics/wsdl\" "
					    "xmlns:trt=\"http://www.onvif.org/ver10/media/wsdl\" "
					    "xmlns:tev=\"http://www.onvif.org/ver10/events/wsdl\" "
					    "xmlns:tptz=\"http://www.onvif.org/ver20/ptz/wsdl\" "
					    "xmlns:timg=\"http://www.onvif.org/ver20/imaging/wsdl\" "
					    "xmlns:ter=\"http://www.onvif.org/ver10/error\"  "
					    "xmlns:tmd=\"http://www.onvif.org/ver10/deviceIO/wsdl\" >";

GK_CHAR soap_head[] = "<s:Header>"
					  	"<wsa:Action>%s</wsa:Action>"
					  "</s:Header>";

GK_CHAR soap_body[] = "<s:Body>";

GK_CHAR soap_tailer[] = "</s:Body></s:Envelope>";


/********************* soap_packet_response_GetCapabilities *************************/

GK_CHAR g_aszMedia_cap[] = "<tt:Media>"
                           	"<tt:XAddr>%s</tt:XAddr>"
                            "<tt:StreamingCapabilities>"
                                "<tt:RTPMulticast>%s</tt:RTPMulticast>"
                                "<tt:RTP_TCP>%s</tt:RTP_TCP>"
                                "<tt:RTP_RTSP_TCP>%s</tt:RTP_RTSP_TCP>"
                            "</tt:StreamingCapabilities>"
                            "<tt:Extension>"
                                "<tt:ProfileCapabilities>"
                                    "<tt:MaximumNumberOfProfiles>%d</tt:MaximumNumberOfProfiles>"
                                "</tt:ProfileCapabilities>"
                            "</tt:Extension>"
                           "</tt:Media>";

GK_CHAR g_aszDevice_cap[] = "<tt:Device>"
                                "<tt:XAddr>%s</tt:XAddr>"
                                "<tt:Network>"
                            	    "<tt:IPFilter>%s</tt:IPFilter>"
                            	    "<tt:ZeroConfiguration>%s</tt:ZeroConfiguration>"
                            	    "<tt:IPVersion6>%s</tt:IPVersion6>"
                            	    "<tt:DynDNS>%s</tt:DynDNS>"
                                "</tt:Network>"
                                "<tt:System>"
                            	    "<tt:DiscoveryResolve>%s</tt:DiscoveryResolve>"
                            	    "<tt:DiscoveryBye>%s</tt:DiscoveryBye>"
                            	    "<tt:RemoteDiscovery>%s</tt:RemoteDiscovery>"
                            	    "<tt:SystemBackup>%s</tt:SystemBackup>"
                            	    "<tt:SystemLogging>%s</tt:SystemLogging>"
                            	    "<tt:FirmwareUpgrade>%s</tt:FirmwareUpgrade>"
                            	    "<tt:SupportedVersions>"
                            		    "<tt:Major>%d</tt:Major>"
                            		    "<tt:Minor>%d</tt:Minor>"
                            	    "</tt:SupportedVersions>"
                                "</tt:System>"
                            	"<tt:IO>"
                            		"<tt:InputConnectors>%d</tt:InputConnectors>"
                            		"<tt:RelayOutputs>%d</tt:RelayOutputs>"
                            	"</tt:IO>"
                            	"<tt:Security>"
                            		"<tt:TLS1.1>%s</tt:TLS1.1>"
                            		"<tt:TLS1.2>%s</tt:TLS1.2>"
                            		"<tt:OnboardKeyGeneration>%s</tt:OnboardKeyGeneration>"
                            		"<tt:AccessPolicyConfig>%s</tt:AccessPolicyConfig>"
                            		"<tt:X.509Token>%s</tt:X.509Token>"
                            		"<tt:SAMLToken>%s</tt:SAMLToken>"
                            		"<tt:KerberosToken>%s</tt:KerberosToken>"
                            		"<tt:RELToken>%s</tt:RELToken>"
                            	"</tt:Security>"
                            "</tt:Device>";

GK_CHAR g_aszAnalytics_cap[] = "<tt:Analytics>"
                            	 "<tt:XAddr>%s</tt:XAddr>"
                            	 "<tt:RuleSupport>%s</tt:RuleSupport>"
                            	 "<tt:AnalyticsModuleSupport>%s</tt:AnalyticsModuleSupport>"
                               "</tt:Analytics>";

GK_CHAR g_aszEvent_cap[] = "<tt:Events>"
                               "<tt:XAddr>%s</tt:XAddr>"
                               "<tt:WSSubscriptionPolicySupport>%s</tt:WSSubscriptionPolicySupport>"
                               "<tt:WSPullPointSupport>%s</tt:WSPullPointSupport>"
                               "<tt:WSPausableSubscriptionManagerInterfaceSupport>%s</tt:WSPausableSubscriptionManagerInterfaceSupport>"
                           "</tt:Events>";

GK_CHAR g_aszImage_cap[] = "<tt:Imaging>"
                            	"<tt:XAddr>%s</tt:XAddr>"
                           "</tt:Imaging>";

GK_CHAR g_aszPtz_cap[] = "<tt:PTZ>"
                            "<tt:XAddr>%s</tt:XAddr>"
                         "</tt:PTZ>";

GK_CHAR g_aszExDeviceIO_cap[] = "<tt:DeviceIO>"
    					            "<tt:XAddr>%s</tt:XAddr>"
    					            "<tt:VideoSources>%d</tt:VideoSources>"
    					            "<tt:VideoOutputs>%d</tt:VideoOutputs>"
    					            "<tt:AudioSources>%d</tt:AudioSources>"
    					            "<tt:AudioOutputs>%d</tt:AudioOutputs>"
    					            "<tt:RelayOutputs>%d</tt:RelayOutputs>"
				                "</tt:DeviceIO>";

GK_CHAR g_aszHik_cap[] = "<hikxsd:hikCapabilities>"
				            "<hikxsd:XAddr>%s</hikxsd:XAddr>"
				            "<hikxsd:IOInputSupport>%s</hikxsd:IOInputSupport>"
				            "<hikxsd:PrivacyMaskSupport>%s</hikxsd:PrivacyMaskSupport>"
				            "<hikxsd:PTZ3DZoomSupport>%s</hikxsd:PTZ3DZoomSupport>"
				            "<hikxsd:PTZPatternSupport>%s</hikxsd:PTZPatternSupport>"
				            "<hikxsd:Language>%d</hikxsd:Language> "
		                "</hikxsd:hikCapabilities>";

/*************** soap_packet_response_GetServiceCapabilities **************************/

GK_CHAR g_aszDevCapabilities[] =
    "<tds:Capabilities>"
        "<tds:Network NTP=\"%d\" HostnameFromDHCP=\"%s\" Dot11Configuration=\"%s\" DynDNS=\"%s\" IPVersion6=\"%s\" ZeroConfiguration=\"%s\" IPFilter=\"%s\" />"
        "<tds:Security RELToken=\"%s\" HttpDigest=\"%s\" UsernameToken=\"%s\" KerberosToken=\"%s\" SAMLToken=\"%s\" X.509Token=\"%s\" RemoteUserHandling=\"%s\" Dot1X=\"%s\" DefaultAccessPolicy=\"%s\" AccessPolicyConfig=\"%s\" OnboardKeyGeneration=\"%s\" TLS1.2=\"%s\" TLS1.1=\"%s\" TLS1.0=\"%s\" />"
        "<tds:System HTTPSupportInformation=\"%s\" HTTPSystemLogging=\"%s\" HttpSystemBackup=\"%s\" HttpFirmwareUpgrade=\"%s\" FirmwareUpgrade=\"%s\" SystemLogging=\"%s\" SystemBackup=\"%s\" RemoteDiscovery=\"%s\" DiscoveryBye=\"%s\" DiscoveryResolve=\"%s\" />"
    "</tds:Capabilities>";

GK_CHAR g_aszMediaCapabilities[] =
    "<trt:Capabilities SnapshotUri=\"%s\" Rotation=\"%s\" VideoSourceMode=\"%s\" OSD=\"%s\">"
        "<trt:ProfileCapabilities MaximumNumberOfProfiles=\"%d\" />"
        "<trt:StreamingCapabilities RTPMulticast=\"%s\" RTP_TCP=\"%s\" RTP_RTSP_TCP=\"%s\" "
            "NonAggregateControl=\"%s\" NoRTSPStreaming=\"%s\" />"
    "</trt:Capabilities>";
    
GK_CHAR g_aszPtzCapabilities[] = "<tptz:Capabilities Reverse=\"%s\" EFlip=\"%s\" GetCompatibleConfigurations=\"%s\"></tptz:Capabilities>";

GK_CHAR g_aszImageCapabilities[] = "<timg:Capabilities ImageStabilization=\"%s\"></timg:Capabilities>";

GK_CHAR g_aszEventCapabilities[] = "<tev:Capabilities WSPausableSubscriptionManagerInterfaceSupport=\"%s\" WSPullPointSupport=\"%s\" WSSubscriptionPolicySupport=\"%s\" MaxNotificationProducers=\"%d\" MaxPullPoints=\"%d\" PersistenNotificationStorage=\"%s\" />";

GK_CHAR g_aszAnalyticsCapabilities[] = "<tan:Capabilities RuleSupport=\"%s\" AnalyticsModuleSupport=\"%s\" CellBasedSceneDescriptionSupported=\"%s\" />";

/***************************************************************************************/

char video_source_config[] =
    "<tt:Name>%s</tt:Name>"
    "<tt:UseCount>%d</tt:UseCount>"
    "<tt:SourceToken>%s</tt:SourceToken>"
    "<tt:Bounds height=\"%d\" width=\"%d\" y=\"%d\" x=\"%d\"></tt:Bounds>";

char audio_source_config[] =
    "<tt:Name>%s</tt:Name>"
    "<tt:UseCount>%d</tt:UseCount>"
    "<tt:SourceToken>%s</tt:SourceToken>";


char audio_encoder_config[] =
	"<tt:Name>%s</tt:Name>"
	"<tt:UseCount>%d</tt:UseCount>"
	"<tt:Encoding>%s</tt:Encoding>"
	"<tt:Bitrate>%d</tt:Bitrate>"
	"<tt:SampleRate>%d</tt:SampleRate>"
	"<tt:Multicast>"
	    "<tt:Address>"
	        "<tt:Type>IPv4</tt:Type>"
	        "<tt:IPv4Address>%s</tt:IPv4Address>"
	    "</tt:Address>"
	    "<tt:Port>%d</tt:Port>"
	    "<tt:TTL>%d</tt:TTL>"
	    "<tt:AutoStart>%s</tt:AutoStart>"
	"</tt:Multicast>"
    "<tt:SessionTimeout>PT%dS</tt:SessionTimeout>";

/***************************************************************************************/
static GK_CHAR *get_H264Profile_string(GONVIF_MEDIA_H264Profile_E profile);
static GK_CHAR *get_Mpeg4Profile_string(GONVIF_MEDIA_Mepg4Profile_E profile);
static GK_CHAR *get_videoEncodingType_string(GONVIF_MEDIA_VideoEncoding_E encoding);
static GK_CHAR *get_audioEncodingType_string(GONVIF_MEDIA_AudioEncoding_E encoding);

static GK_CHAR *onvif_get_ptz_movestatus(GONVIF_PTZ_MoveStatus_E movestatus);




static GK_S32 packet_VideoEncoderConfiguration(GK_CHAR *p_buf, GK_S32 mlen, GONVIF_MEDIA_VideoEncoderConfiguration_S *argv);
static GK_S32 packet_VideoAnalyticsConfiguration(GK_CHAR *p_buf, GK_S32 mlen, GONVIF_MEDIA_VideoAnalyticsConfiguration_S *argv);
static GK_S32 packet_PTZConfiguration(GK_CHAR *p_buf, GK_S32 mlen, GONVIF_PTZ_Configuration_S *argv);

/***************************** error handing *************************************/

GK_S32 soap_response_error(GK_CHAR *p_buf, GK_S32 mlen, const GK_CHAR *code,
	const GK_CHAR *subcode, const GK_CHAR *subcode_ex, const GK_CHAR *reason, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);	
    if(header != NULL)
    {
        GONVIF_Soap_S *pstHeader = (GONVIF_Soap_S *)header;
        offset += snprintf(p_buf+offset, mlen-offset, soap_head, pstHeader->stHeader.pszAction); 
    }
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<s:Fault><s:Code><s:Value>%s</s:Value>", code);
    offset += snprintf(p_buf+offset, mlen-offset, "<s:Subcode><s:Value>%s</s:Value>", subcode);
    if (NULL != subcode_ex)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<s:Subcode><s:Value>%s</s:Value></s:Subcode>", subcode_ex);
    }
    offset += snprintf(p_buf+offset, mlen-offset, "</s:Subcode></s:Code>");
	offset += snprintf(p_buf+offset, mlen-offset, "<s:Reason><s:Text xml:lang=\"en\">%s</s:Text></s:Reason></s:Fault>", reason);

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}
/***************************** error handing end **********************************/

/******************************** Device *****************************************/

GK_S32 soap_response_Device_GetServiceCapabilities(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GONVIF_DEVMGN_GetServiceCapabilities_Res_S *pstDM_GetServiceCapabilitiesRes = (GONVIF_DEVMGN_GetServiceCapabilities_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetServiceCapabilitiesResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, g_aszDevCapabilities,
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stNetwork.ntp,
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stNetwork.enHostnameFromDHCP? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stNetwork.enDot11Configuration? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stNetwork.enDynDNS? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stNetwork.enIPVersion6? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stNetwork.enZeroConfiguration? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stNetwork.enIPFilter? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enRELToken? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enHttpDigest? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enUsernameToken? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enKerberosToken? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enSAMLToken? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enX_x002e509Token? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enRemoteUserHandling? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enDot1X? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enDefaultAccessPolicy? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enAccessPolicyConfig? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enOnboardKeyGeneration? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enTLS1_x002e2? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enTLS1_x002e1? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSecurity.enTLS1_x002e0? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSystem.enHttpSupportInformation? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSystem.enHttpSystemLogging? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSystem.enHttpSystemBackup? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSystem.enHttpFirmwareUpgrade? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSystem.enFirmwareUpgrade? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSystem.enSystemLogging? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSystem.enSystemBackup? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSystem.enRemoteDiscovery? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSystem.enDiscoveryBye? "true" : "false",
		pstDM_GetServiceCapabilitiesRes->stCapabilities.stSystem.enDiscoveryResolve? "true" : "false");
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:GetServiceCapabilitiesResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

GK_S32 soap_packet_response_GetWsdlUrl(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_DEVMNG_GetWsdlUrl_Res_S *pstDM_GetWsdlUrlRes = (GONVIF_DEVMNG_GetWsdlUrl_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset,
		"<tds:GetWsdlUrlResponse>"
			"<tds:WsdlUrl>%s</tds:WsdlUrl>"
		"</tds:GetWsdlUrlResponse>", pstDM_GetWsdlUrlRes->aszWsdlUrl);

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetServices(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 i = 0;
    GONVIF_DEVMGN_GetServices_Res_S *pstDM_GetServicesRes = (GONVIF_DEVMGN_GetServices_Res_S *)argv;

    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetServicesResponse>");

    for(i = 0; i < pstDM_GetServicesRes->sizeService; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tds:Service>"
            "<tds:Namespace>%s</tds:Namespace>"
            "<tds:XAddr>%s</tds:XAddr>",
            pstDM_GetServicesRes->stService[i].aszNamespace, pstDM_GetServicesRes->stService[i].aszXAddr);
        if(pstDM_GetServicesRes->stService[i].stCapabilities.any[0] != '\0')
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tds:Capabilities>");
            offset += snprintf(p_buf+offset, mlen-offset, pstDM_GetServicesRes->stService[i].stCapabilities.any);
            offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>");
        }

        offset += snprintf(p_buf+offset, mlen-offset,
    		"<tds:Version>"
        	    "<tt:Major>%d</tt:Major>"
        	    "<tt:Minor>%d</tt:Minor>"
    	    "</tds:Version>"
	    "</tds:Service>", pstDM_GetServicesRes->stService[i].stVersion.major, pstDM_GetServicesRes->stService[i].stVersion.minor);
    }

	offset += snprintf(p_buf+offset, mlen-offset, "</tds:GetServicesResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetCapabilities(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{

	GONVIF_DEVMNG_GetCapabilities_Res_S *pstDM_GetCapabilitiesRes = (GONVIF_DEVMNG_GetCapabilities_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetCapabilitiesResponse>"
	                                                "<tds:Capabilities>");

    if(pstDM_GetCapabilitiesRes->stCapabilities.stAnalytics.aszXAddr[0] != '\0')
    {
        offset += snprintf(p_buf+offset, mlen-offset, g_aszAnalytics_cap,
            pstDM_GetCapabilitiesRes->stCapabilities.stAnalytics.aszXAddr,
            pstDM_GetCapabilitiesRes->stCapabilities.stAnalytics.enRuleSupport? "true" : "false",
            pstDM_GetCapabilitiesRes->stCapabilities.stAnalytics.enAnalyticsModuleSupport? "true" : "false");
    }
    if(pstDM_GetCapabilitiesRes->stCapabilities.stDevice.aszXAddr[0] != '\0')
    {
        offset += snprintf(p_buf+offset, mlen-offset, g_aszDevice_cap,
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.aszXAddr,
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stNetwork.enIPFilter? "true" : "false",
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stNetwork.enZeroConfiguration? "true" : "false",
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stNetwork.enIPVersion6? "true" : "false",
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stNetwork.enDynDNS? "true" : "false",
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSystem.enDiscoveryResolve? "true" : "false",
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSystem.enDiscoveryBye? "true" : "false",
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSystem.enRemoteDiscovery? "true" : "false",
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSystem.enSystemBackup? "true" : "false",
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSystem.enSystemLogging? "true" : "false",
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSystem.enFirmwareUpgrade? "true" : "false",
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSystem.stSupportedVersions.major,
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSystem.stSupportedVersions.minor,
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stIO.inputConnectors,
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stIO.relayOutputs,
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSecurity.enTLS1_1? "true" : "false",
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSecurity.enTLS1_2? "true" : "false",
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSecurity.enOnboardKeyGeneration? "true" : "false",
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSecurity.enAccessPolicyConfig? "true" : "false",
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSecurity.enX_x002e509Token? "true" : "false",
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSecurity.enSAMLToken? "true" : "false",
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSecurity.enKerberosToken? "true" : "false",
            pstDM_GetCapabilitiesRes->stCapabilities.stDevice.stSecurity.enRELToken? "true" : "false");
    }
    if(pstDM_GetCapabilitiesRes->stCapabilities.stEvent.aszXAddr[0] != '\0')
    {
        offset += snprintf(p_buf+offset, mlen-offset, g_aszEvent_cap,
            pstDM_GetCapabilitiesRes->stCapabilities.stEvent.aszXAddr,
            pstDM_GetCapabilitiesRes->stCapabilities.stEvent.enWSSubscriptionPolicySupport? "true" : "false",
            pstDM_GetCapabilitiesRes->stCapabilities.stEvent.enWSPullPointSupport? "true" : "false",
            pstDM_GetCapabilitiesRes->stCapabilities.stEvent.enWSPausableSubscriptionManagerInterfaceSupport? "true" : "false");
    }
    if(pstDM_GetCapabilitiesRes->stCapabilities.stImaging.aszXAddr[0] != '\0')
    {
        offset += snprintf(p_buf+offset, mlen-offset, g_aszImage_cap, pstDM_GetCapabilitiesRes->stCapabilities.stImaging.aszXAddr);
    }
    if(pstDM_GetCapabilitiesRes->stCapabilities.stMedia.aszXAddr[0] != '\0')
    {
        offset += snprintf(p_buf+offset, mlen-offset, g_aszMedia_cap,
            pstDM_GetCapabilitiesRes->stCapabilities.stMedia.aszXAddr,
            pstDM_GetCapabilitiesRes->stCapabilities.stMedia.stStreamingCapabilities.enRTPMulticast? "true" : "false",
            pstDM_GetCapabilitiesRes->stCapabilities.stMedia.stStreamingCapabilities.enRTP_USCORETCP? "true" : "false",
            pstDM_GetCapabilitiesRes->stCapabilities.stMedia.stStreamingCapabilities.enRTP_USCORERTSP_USCORETCP? "true" : "false",
            MAX_PROFILE_NUM);
    }
    if(pstDM_GetCapabilitiesRes->stCapabilities.stPTZ.aszXAddr[0] != '\0')
    {
        offset += snprintf(p_buf+offset, mlen-offset, g_aszPtz_cap, pstDM_GetCapabilitiesRes->stCapabilities.stPTZ.aszXAddr);
    }
    if(pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stDeviceIO.aszXAddr[0] != '\0' ||
       pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stHik.aszXAddr[0] != '\0')
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Extension>");
        if(pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stHik.aszXAddr[0] != '\0')
        {
            offset += snprintf(p_buf+offset, mlen-offset, g_aszHik_cap,
                pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stHik.aszXAddr,
                pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stHik.IOInputSupport? "true" : "false",
                pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stHik.PrivacyMaskSupport? "true" : "false",
                pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stHik.PTZ3DZoomSupport? "true" : "false",
                pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stHik.PTZPatternSupport? "true" : "false",
                pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stHik.Language);
        }
        if(pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stDeviceIO.aszXAddr[0] != '\0')
        {
            offset += snprintf(p_buf+offset, mlen-offset, g_aszExDeviceIO_cap,
                pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stDeviceIO.aszXAddr,
                pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stDeviceIO.videoSources,
                pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stDeviceIO.videoOutputs,
                pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stDeviceIO.AudioSources,
                pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stDeviceIO.AudioOutputs,
                pstDM_GetCapabilitiesRes->stCapabilities.stExtension.stDeviceIO.RelayOutputs);
        }
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:Extension>");
    }

	offset += snprintf(p_buf+offset, mlen-offset, "</tds:Capabilities>"
	                                            "</tds:GetCapabilitiesResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_SystemReboot(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    
    GONVIF_DEVMGN_SystemReboot_Res_S *pstDM_SystemRebootRes = (GONVIF_DEVMGN_SystemReboot_Res_S *)argv;
    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SystemRebootResponse>"
	                                                "<tds:Message>%s</tds:Message>"
	                                              "</tds:SystemRebootResponse>",
	                                              pstDM_SystemRebootRes->aszMessage);
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetHostname(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_DEVMNG_GetHostname_Res_S *pstDM_GetHostnameRes = (GONVIF_DEVMNG_GetHostname_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset,
        "<tds:GetHostnameResponse>"
            "<tds:HostnameInformation>"
                "<tt:FromDHCP>%s</tt:FromDHCP>"
                "<tt:Name>%s</tt:Name>"
       	    "</tds:HostnameInformation>"
       	"</tds:GetHostnameResponse>",
      	pstDM_GetHostnameRes->stHostnameInformation.enFromDHCP? "true" : "false",
      	pstDM_GetHostnameRes->stHostnameInformation.aszName);

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_SetHostname(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetHostnameResponse></tds:SetHostnameResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetDNS(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 i = 0;
    GONVIF_DEVMNG_GetDNS_Res_S *pstDM_GetDNSRes = (GONVIF_DEVMNG_GetDNS_Res_S *)argv;
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetDNSResponse>"
	                                                "<tds:DNSInformation>");

    offset += snprintf(p_buf+offset, mlen-offset, "<tt:FromDHCP>%s</tt:FromDHCP>", 
    	pstDM_GetDNSRes->stDNSInformation.enFromDHCP ? "true" : "false");

	for(i = 0; i < pstDM_GetDNSRes->stDNSInformation.sizeSearchDomain; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:SearchDomain>%s</tt:SearchDomain>",
			pstDM_GetDNSRes->stDNSInformation.aszSearchDomain[i]);
	}

    if(pstDM_GetDNSRes->stDNSInformation.enFromDHCP == Boolean_TRUE)
    {
        for(i = 0; i < pstDM_GetDNSRes->stDNSInformation.sizeDNSFromDHCP; i++)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:DNSFromDHCP>");
            if(pstDM_GetDNSRes->stDNSInformation.stDNSFromDHCP[i].enType == IPType_IPv4)
            {
                offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>IPv4</tt:Type>"
                											  "<tt:IPv4Address>%s</tt:IPv4Address>",
                    pstDM_GetDNSRes->stDNSInformation.stDNSFromDHCP[i].aszIPv4Address);
            }
            else if(pstDM_GetDNSRes->stDNSInformation.stDNSManual[i].enType == IPType_IPv6)
            {
                offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>IPv6</tt:Type>"
                											  "<tt:IPv6Address>%s</tt:IPv6Address>",
                    pstDM_GetDNSRes->stDNSInformation.stDNSFromDHCP[i].aszIPv6Address);
            }
            offset += snprintf(p_buf+offset, mlen-offset, "</tt:DNSFromDHCP>");
        }
    }
    else
    {
        for(i = 0; i < pstDM_GetDNSRes->stDNSInformation.sizeDNSManual; i++)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:DNSManual>");
            if(pstDM_GetDNSRes->stDNSInformation.stDNSManual[i].enType == IPType_IPv4)
            {
                offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>IPv4</tt:Type>"
                											  "<tt:IPv4Address>%s</tt:IPv4Address>",
                    pstDM_GetDNSRes->stDNSInformation.stDNSManual[i].aszIPv4Address);
            }
            else if(pstDM_GetDNSRes->stDNSInformation.stDNSManual[i].enType == IPType_IPv6)
            {
                offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>IPv6</tt:Type>"
                											  "<tt:IPv6Address>%s</tt:IPv6Address>",
                    pstDM_GetDNSRes->stDNSInformation.stDNSManual[i].aszIPv6Address);
            }
            offset += snprintf(p_buf+offset, mlen-offset, "</tt:DNSManual>");
        }
    }

	offset += snprintf(p_buf+offset, mlen-offset, "</tds:DNSInformation>"
	                                            "</tds:GetDNSResponse>");
	                                            
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

GK_S32 soap_packet_response_SetDNS(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetDNSResponse></tds:SetDNSResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetNTP(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 i = 0;
    GONVIF_DEVMNG_GetNTP_Res_S *pstDM_GetNTPRes = (GONVIF_DEVMNG_GetNTP_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetNTPResponse><tds:NTPInformation>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tt:FromDHCP>%s</tt:FromDHCP>", pstDM_GetNTPRes->stNTPInformation.enFromDHCP ? "true" : "false");
    if(pstDM_GetNTPRes->stNTPInformation.enFromDHCP == Boolean_TRUE)
    {
        for (i = 0; i < pstDM_GetNTPRes->stNTPInformation.sizeNTPFromDHCP; i++)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:NTPFromDHCP>");
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>IPv4</tt:Type>");
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:IPv4Address>%s</tt:IPv4Address>", pstDM_GetNTPRes->stNTPInformation.stNTPFromDHCP[i].aszIPv4Address);
            offset += snprintf(p_buf+offset, mlen-offset, "</tt:NTPFromDHCP>");
        }
    }
    else
    {
        for (i = 0; i < pstDM_GetNTPRes->stNTPInformation.sizeNTPManual; i++)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:NTPManual>");
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>IPv4</tt:Type>");
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:IPv4Address>%s</tt:IPv4Address>", pstDM_GetNTPRes->stNTPInformation.stNTPManual[i].aszIPv4Address);
            offset += snprintf(p_buf+offset, mlen-offset, "</tt:NTPManual>");
        }
    }
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:NTPInformation></tds:GetNTPResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_SetNTP(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetNTPResponse></tds:SetNTPResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetNetworkInterfaces(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GONVIF_DEVMNG_GetNetworkInterfaces_Res_S *pstDM_GetNetworkInterfacesRes = (GONVIF_DEVMNG_GetNetworkInterfaces_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetNetworkInterfacesResponse>");
	GK_S32 i = 0;
    for(i = 0; i < pstDM_GetNetworkInterfacesRes->sizeNetworkInterfaces; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tds:NetworkInterfaces token=\"%s\">"
                                            		    "<tt:Enabled>%s</tt:Enabled>",
		    pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[i].asztoken, 
		    pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[i].enEnabled ? "true" : "false");

		offset += snprintf(p_buf+offset, mlen-offset,"<tt:Info>"
                                                		 "<tt:Name>%s</tt:Name>"
                                                		 "<tt:HwAddress>%s</tt:HwAddress>"
                                                		 "<tt:MTU>%d</tt:MTU>"
                                            		 "</tt:Info>",
			pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[i].stInfo.aszName, 
			pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[i].stInfo.aszHwAddress,
			pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[i].stInfo.MTU);

        offset += snprintf(p_buf+offset, mlen-offset, "<tt:IPv4>"
                                                        "<tt:Enabled>%s</tt:Enabled>"
                                                        "<tt:Config>", 
            pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[i].stIPv4.enEnabled? "true" : "false");

		if (pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[i].stIPv4.stConfig.enDHCP == Boolean_FALSE)
		{
		    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Manual>"
		                                                    "<tt:Address>%s</tt:Address>"
		                                                    "<tt:PrefixLength>%d</tt:PrefixLength>"
		                                                  "</tt:Manual>",
		        pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[i].stIPv4.stConfig.stManual[0].aszAddress, pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[i].stIPv4.stConfig.stManual[0].prefixLength);
		}
		else
		{
		    offset += snprintf(p_buf+offset, mlen-offset, "<tt:FromDHCP>"
		                                                    "<tt:Address>%s</tt:Address>"
		                                                    "<tt:PrefixLength>%d</tt:PrefixLength>"
		                                                  "</tt:FromDHCP>",
		        pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[i].stIPv4.stConfig.stFromDHCP.aszAddress, pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[i].stIPv4.stConfig.stFromDHCP.prefixLength);
		}

		offset += snprintf(p_buf+offset, mlen-offset, "<tt:DHCP>%s</tt:DHCP>"
		                                            "</tt:Config>"
	                                              "</tt:IPv4>"
	                                            "</tds:NetworkInterfaces>", 
	        pstDM_GetNetworkInterfacesRes->stNetworkInterfaces[i].stIPv4.stConfig.enDHCP? "true" : "false");
	}

	offset += snprintf(p_buf+offset, mlen-offset, "</tds:GetNetworkInterfacesResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_SetNetworkInterfaces(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_DEVMNG_SetNetworkInterfaces_Res_S *pstDM_SetNetworkInterfacesRes = (GONVIF_DEVMNG_SetNetworkInterfaces_Res_S *)argv;
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetNetworkInterfacesResponse>"
                                        		    "<tds:RebootNeeded>%s</tds:RebootNeeded>"
                                        		  "</tds:SetNetworkInterfacesResponse>", 
        pstDM_SetNetworkInterfacesRes->enRebootNeeded ? "true" : "false");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetNetworkProtocols(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{

	GK_S32 i = 0;
	GK_S32 j = 0;
    GONVIF_DEVMNG_GetNetworkProtocols_Res_S *pstDM_GetNetworkProtocolsRes = (GONVIF_DEVMNG_GetNetworkProtocols_Res_S *)argv;
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetNetworkProtocolsResponse>");
    for(i = 0; i < pstDM_GetNetworkProtocolsRes->sizeNetworkProtocols; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tds:NetworkProtocols>");
        if (pstDM_GetNetworkProtocolsRes->stNetworkProtocols[i].enName == NetworkProtocolType_HTTP)
        {
            offset += snprintf(p_buf+offset, mlen-offset,
                "<tt:Name>HTTP</tt:Name>"
                "<tt:Enabled>%s</tt:Enabled>", 
                pstDM_GetNetworkProtocolsRes->stNetworkProtocols[i].enEnabled? "true" : "false");
        }
        else if (pstDM_GetNetworkProtocolsRes->stNetworkProtocols[i].enName == NetworkProtocolType_HTTPS)
        {
            offset += snprintf(p_buf+offset, mlen-offset,
                "<tt:Name>HTTPS</tt:Name>"
                "<tt:Enabled>%s</tt:Enabled>",
                pstDM_GetNetworkProtocolsRes->stNetworkProtocols[i].enEnabled? "true" : "false");
        }
        else
        {
            offset += snprintf(p_buf+offset, mlen-offset,
                "<tt:Name>RTSP</tt:Name>"
                "<tt:Enabled>%s</tt:Enabled>",
                pstDM_GetNetworkProtocolsRes->stNetworkProtocols[i].enEnabled? "true" : "false");
        }
		for (j = 0; j < pstDM_GetNetworkProtocolsRes->stNetworkProtocols[i].sizePort; j++)
		{
			offset += snprintf(p_buf+offset, mlen-offset, 
			    "<tt:Port>%d</tt:Port>",
			    pstDM_GetNetworkProtocolsRes->stNetworkProtocols[i].port[j]);
		}
		offset += snprintf(p_buf+offset, mlen-offset, "</tds:NetworkProtocols>");
    }

	offset += snprintf(p_buf+offset, mlen-offset, "</tds:GetNetworkProtocolsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_SetNetworkProtocols(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetNetworkProtocolsResponse></tds:SetNetworkProtocolsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetNetworkDefaultGateway(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 i =0;
    
    GONVIF_DEVMNG_GetNetworkDefaultGateway_Res_S *pstDM_GetNetworkDefaultGatewayRes = (GONVIF_DEVMNG_GetNetworkDefaultGateway_Res_S *)argv;
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetNetworkDefaultGatewayResponse>"
	                                                "<tds:NetworkGateway>");

	for (i = 0; i < pstDM_GetNetworkDefaultGatewayRes->stNetworkGateway.sizeIPv4Address; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:IPv4Address>%s</tt:IPv4Address>", pstDM_GetNetworkDefaultGatewayRes->stNetworkGateway.aszIPv4Address[i]);
	}

	offset += snprintf(p_buf+offset, mlen-offset, "</tds:NetworkGateway>"
	                                            "</tds:GetNetworkDefaultGatewayResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_SetNetworkDefaultGateway(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetNetworkDefaultGatewayResponse>"
	                                              "</tds:SetNetworkDefaultGatewayResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetSystemDateAndTime(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_DEVMNG_GetSystemDateAndTime_Res_S *pstDM_GetSystemDateAndTimeRes = (GONVIF_DEVMNG_GetSystemDateAndTime_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset,
		"<tds:GetSystemDateAndTimeResponse>"
		    "<tds:SystemDateAndTime>"
			    "<tt:DateTimeType>%s</tt:DateTimeType>"
			    "<tt:DaylightSavings>%s</tt:DaylightSavings>",
        pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.enDateTimeType? "NTP" : "Manual",
        pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.enDaylightSavings? "true" : "false");

	if(pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stTimeZone.aszTZ[0] != '\0')
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		        "<tt:TimeZone>"
		            "<tt:TZ>%s</tt:TZ>"
		        "</tt:TimeZone>",
            pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stTimeZone.aszTZ);
	}

	offset += snprintf(p_buf+offset, mlen-offset,
			    "<tt:UTCDateTime>"
				    "<tt:Time><tt:Hour>%d</tt:Hour><tt:Minute>%d</tt:Minute><tt:Second>%d</tt:Second></tt:Time>"
				    "<tt:Date><tt:Year>%d</tt:Year><tt:Month>%d</tt:Month><tt:Day>%d</tt:Day></tt:Date>"
			    "</tt:UTCDateTime>"
			    "<tt:LocalDateTime>"
				    "<tt:Time><tt:Hour>%d</tt:Hour><tt:Minute>%d</tt:Minute><tt:Second>%d</tt:Second></tt:Time>"
				    "<tt:Date><tt:Year>%d</tt:Year><tt:Month>%d</tt:Month><tt:Day>%d</tt:Day></tt:Date>"
			    "</tt:LocalDateTime>"
		    "</tds:SystemDateAndTime>"
		"</tds:GetSystemDateAndTimeResponse>",
		pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stUTCDateTime.stTime.hour,pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stUTCDateTime.stTime.minute,pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stUTCDateTime.stTime.second,
		pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stUTCDateTime.stDate.year,pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stUTCDateTime.stDate.month,pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stUTCDateTime.stDate.day,
		pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stLocalDateTime.stTime.hour,pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stLocalDateTime.stTime.minute,pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stLocalDateTime.stTime.second,
		pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stLocalDateTime.stDate.year,pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stLocalDateTime.stDate.month,pstDM_GetSystemDateAndTimeRes->stSystemDataAndTime.stLocalDateTime.stDate.day);

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_SetSystemDateAndTime(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetSystemDateAndTimeResponse>"
	                                              "</tds:SetSystemDateAndTimeResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetDeviceInformation(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_DEVMNG_GetDeviceInformation_Res_S *pstDM_GetDeviceInformationRes = (GONVIF_DEVMNG_GetDeviceInformation_Res_S *)argv;
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset,
		"<tds:GetDeviceInformationResponse>"
		    "<tds:Manufacturer>%s</tds:Manufacturer>"
		    "<tds:Model>%s</tds:Model>"
		    "<tds:FirmwareVersion>%s</tds:FirmwareVersion>"
		    "<tds:SerialNumber>%s</tds:SerialNumber>"
		    "<tds:HardwareId>%s</tds:HardwareId>"
	    "</tds:GetDeviceInformationResponse>",
    	pstDM_GetDeviceInformationRes->aszManufacturer,
    	pstDM_GetDeviceInformationRes->aszModel,
    	pstDM_GetDeviceInformationRes->aszFirmwareVersion,
    	pstDM_GetDeviceInformationRes->aszSerialNumber,
    	pstDM_GetDeviceInformationRes->aszHardwareId);

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	//ONVIF_ERR("p_buf: \n%s\n", p_buf);
	return offset;
}

GK_S32 soap_packet_response_GetScopes(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_U8 i;
	GK_BOOL isFixed = GK_FALSE;
	GONVIF_DEVMNG_GetScopes_Res_S *pstDM_GetScopesRes = (GONVIF_DEVMNG_GetScopes_Res_S *)argv;
	
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetScopesResponse>");
	for (i = 0; i < pstDM_GetScopesRes->sizeScopes; i++)
	{
		if(pstDM_GetScopesRes->pstScopes[i].enScopeDef == ScopeDefinition_Fixed)
		{
			isFixed = GK_TRUE;
		}
		else
		{
			isFixed = GK_FALSE;
		}
		offset += snprintf(p_buf+offset, mlen-offset, 
		"<tds:Scopes>"
		    "<tt:ScopeDef>%s</tt:ScopeDef>"
			"<tt:ScopeItem>%s</tt:ScopeItem>"
	    "</tds:Scopes>",
			isFixed ? "Fixed" : "Configurable", pstDM_GetScopesRes->pstScopes[i].aszScopeItem);
	}
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:GetScopesResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

GK_S32 soap_packet_response_SetScopes(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetScopesResponse>"
                                                  "</tds:SetScopesResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_AddScopes(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<tds:AddScopesResponse>"
                                                  "</tds:AddScopesResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_RemoveScopes(GK_CHAR*p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_U8 i;

	GONVIF_DEVMNG_RemoveScopes_Res_S *pstDM_RemoveScopesRes = (GONVIF_DEVMNG_RemoveScopes_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<tds:RemoveScopesResponse>");
    for (i = 0; i < pstDM_RemoveScopesRes->sizeScopeItem; i++)
	{
	    if(pstDM_RemoveScopesRes->pszScopeItem[i] != NULL)
	    {
		    offset += snprintf(p_buf+offset, mlen-offset, "<tds:ScopeItem>%s</tds:ScopeItem>", pstDM_RemoveScopesRes->pszScopeItem[i]);
        }
	}
    offset += snprintf(p_buf+offset, mlen-offset, "</tds:RemoveScopesResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetDiscoveryMode(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GONIVF_DEVMNG_GetDiscoveryMode_Res_S *pstDM_GetDiscoveryModeRes = (GONIVF_DEVMNG_GetDiscoveryMode_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset,
		"<tds:GetDiscoveryModeResponse>"
   		"<tds:DiscoveryMode>%s</tds:DiscoveryMode>"
	  	"</tds:GetDiscoveryModeResponse>",
	  	pstDM_GetDiscoveryModeRes->enDiscoveryMode ? "NonDiscoverable" : "Discoverable");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_SetDiscoveryMode(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetDiscoveryModeResponse>"
	                                              "</tds:SetDiscoveryModeResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetUsers(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 i = 0;
    GONVIF_DEVMNG_GetUsers_Res_S *pstDM_GetUsersRes = (GONVIF_DEVMNG_GetUsers_Res_S *)argv;
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetUsersResponse>");

    for(i = 0; i < pstDM_GetUsersRes->sizeUser; i++)
    {

        if(pstDM_GetUsersRes->stUser[i].enUserLevel == UserLevel_Administrator)
        {
            offset += snprintf(p_buf+offset, mlen-offset,
                "<tds:User>"
                    "<tt:Username>%s</tt:Username>"
                    "<tt:UserLevel>Administrator</tt:UserLevel>"
                "</tds:User>",
                pstDM_GetUsersRes->stUser[i].aszUsername);
        }
        else if(pstDM_GetUsersRes->stUser[i].enUserLevel == UserLevel_Operator)
        {
            offset += snprintf(p_buf+offset, mlen-offset,
                "<tds:User>"
                    "<tt:Username>%s</tt:Username>"
                    "<tt:UserLevel>Operator</tt:UserLevel>"
                "</tds:User>",
                pstDM_GetUsersRes->stUser[i].aszUsername);
        }
        else if(pstDM_GetUsersRes->stUser[i].enUserLevel == UserLevel_User)
        {
            offset += snprintf(p_buf+offset, mlen-offset,
                "<tds:User>"
                    "<tt:Username>%s</tt:Username>"
                    "<tt:UserLevel>User</tt:UserLevel>"
                "</tds:User>",
                pstDM_GetUsersRes->stUser[i].aszUsername);
        }
        else if(pstDM_GetUsersRes->stUser[i].enUserLevel == UserLevel_Anonymous)
        {
            offset += snprintf(p_buf+offset, mlen-offset,
                "<tds:User>"
                    "<tt:Username>%s</tt:Username>"
                    "<tt:UserLevel>Anonymous</tt:UserLevel>"
                "</tds:User>",
                pstDM_GetUsersRes->stUser[i].aszUsername);
        }
        else
        {
            offset += snprintf(p_buf+offset, mlen-offset,
                "<tds:User>"
                    "<tt:Username>%s</tt:Username>"
                    "<tt:UserLevel>Extended</tt:UserLevel>"
                "</tds:User>",
                pstDM_GetUsersRes->stUser[i].aszUsername);
        }
    }
	offset += snprintf(p_buf+offset, mlen-offset, "</tds:GetUsersResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_CreateUsers(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:CreateUsersResponse>"
	                                              "</tds:CreateUsersResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_DeleteUsers(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:DeleteUsersResponse>"
	                                              "</tds:DeleteUsersResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_SetUser(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetUserResponse>"
	                                              "</tds:SetUserResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_SetSystemFactoryDefault(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetSystemFactoryDefaultResponse>"
	                                              "</tds:SetSystemFactoryDefaultResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

/****************************** Device end ***************************************/

/******************************* deviceIO ****************************************/

GK_S32 soap_packet_response_GetVideoSources(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 i = 0;
    GONVIF_DEVICEIO_GetVideoSources_Res_S *pstIO_GetVideoSourcesRes = (GONVIF_DEVICEIO_GetVideoSources_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoSourcesResponse>");

    for(i = 0;i < pstIO_GetVideoSourcesRes->sizeVideoSources; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<trt:VideoSources token=\"%s\">"
                                                        "<tt:Framerate>%d</tt:Framerate>"
                                                        "<tt:Resolution>"
                                                            "<tt:Width>%d</tt:Width>"
                                                            "<tt:Height>%d</tt:Height>"
                                                        "</tt:Resolution>"
                                                      "</trt:VideoSources>",
        pstIO_GetVideoSourcesRes->stVideoSources[i].aszToken,
        (GK_U32)pstIO_GetVideoSourcesRes->stVideoSources[i].framerate,
        pstIO_GetVideoSourcesRes->stVideoSources[i].stResolution.width,
        pstIO_GetVideoSourcesRes->stVideoSources[i].stResolution.height);
    }
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoSourcesResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

GK_S32 soap_packet_response_GetAudioSources(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 i = 0;
    GONVIF_DEVICEIO_GetAudioSources_Res_S *pstIO_GetAudioSourcesRes = (GONVIF_DEVICEIO_GetAudioSources_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioSourcesResponse>");

    for(i = 0;i < pstIO_GetAudioSourcesRes->sizeAudioSources; i++)
    {
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:AudioSources token=\"%s\">", pstIO_GetAudioSourcesRes->stAudioSources[i].aszToken);
	    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Channels>%d</tt:Channels>", pstIO_GetAudioSourcesRes->stAudioSources[i].channels);
	    offset += snprintf(p_buf+offset, mlen-offset, "</trt:AudioSources>");
	}

	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetAudioSourcesResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetRelayOutputs(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 i = 0;
    GONVIF_DEVICEIO_GetRelayOutputs_Res_S *pstIO_GetRelayOutputsRes = (GONVIF_DEVICEIO_GetRelayOutputs_Res_S *)argv;

    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

    offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
    offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<tds:GetRelayOutputsResponse>");

    for(i = 0;i < pstIO_GetRelayOutputsRes->sizeRelayOutputs; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tds:RelayOutputs token=\"%s\">"
                                                        "<tt:Properties>"
                                                            "<tt:Mode>%s</tt:Mode>"
                                                            "<tt:DelayTime>PT%lldS</tt:DelayTime>"
                                                            "<tt:IdleState>%s</tt:IdleState>"
                                                        "</tt:Properties>"
                                                      "</tds:RelayOutputs>",
                                                      pstIO_GetRelayOutputsRes->stRelayOutputs[i].aszToken,
                                                      pstIO_GetRelayOutputsRes->stRelayOutputs[i].stProperties.enMode ? "Bistable" : "Monostable",
                                                      pstIO_GetRelayOutputsRes->stRelayOutputs[i].stProperties.delayTime,
                                                      pstIO_GetRelayOutputsRes->stRelayOutputs[i].stProperties.enIdleState ? "open" : "closed");
    }

    offset += snprintf(p_buf+offset, mlen-offset, "</tds:GetRelayOutputsResponse>");
    offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

    return offset;
}

GK_S32 soap_packet_response_SetRelayOutputSettings(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

    offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
    offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetRelayOutputSettingsResponse>");
    offset += snprintf(p_buf+offset, mlen-offset, "</tds:SetRelayOutputSettingsResponse>");
    offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

    return offset;
}

GK_S32 soap_packet_response_SetRelayOutputState(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

    offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
    offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<tds:SetRelayOutputStateResponse>");
    offset += snprintf(p_buf+offset, mlen-offset, "</tds:SetRelayOutputStateResponse>");
    offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

    return offset;
}

/****************************** deviceIO end *************************************/

/********************************** media ***************************************/

GK_S32 soap_response_Media_GetServiceCapabilities(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GONVIF_MEDIA_GetServiceCapabilities_Res_S *pstMD_GetServiceCapabilitiesRes = (GONVIF_MEDIA_GetServiceCapabilities_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetServiceCapabilitiesResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, g_aszMediaCapabilities,
		pstMD_GetServiceCapabilitiesRes->stCapabilities.enSnapshotUri? "true" : "false",
		pstMD_GetServiceCapabilitiesRes->stCapabilities.enRotation? "true" : "false",
		pstMD_GetServiceCapabilitiesRes->stCapabilities.enVideoSourceMode? "true" : "false",
		pstMD_GetServiceCapabilitiesRes->stCapabilities.enOSD? "true" : "false",
		pstMD_GetServiceCapabilitiesRes->stCapabilities.stProfileCapabilities.maximumNumberOfProfiles,
		pstMD_GetServiceCapabilitiesRes->stCapabilities.stStreamingCapabilities.enRTPMulticast? "true" : "false",
		pstMD_GetServiceCapabilitiesRes->stCapabilities.stStreamingCapabilities.enRTP_USCORETCP? "true" : "false",
		pstMD_GetServiceCapabilitiesRes->stCapabilities.stStreamingCapabilities.enRTP_USCORERTSP_USCORETCP? "true" : "false",
		pstMD_GetServiceCapabilitiesRes->stCapabilities.stStreamingCapabilities.enNonAggregateControl? "true" : "false",
		pstMD_GetServiceCapabilitiesRes->stCapabilities.stStreamingCapabilities.enNoRTSPStreaming? "true" : "false",
		pstMD_GetServiceCapabilitiesRes->stCapabilities.stStreamingCapabilities.enRTPMulticast? "true" : "false");

	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetServiceCapabilitiesResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

GK_S32 soap_packet_response_CreateProfile(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GONVIF_MEDIA_CreateProfile_Res_S *pstMD_CreateProfileRes = (GONVIF_MEDIA_CreateProfile_Res_S *)argv;
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:CreateProfileResponse>");

    offset += snprintf(p_buf+offset, mlen-offset,
        "<trt:Profile fixed=\"%s\" token=\"%s\">"
            "<tt:Name>%s</tt:Name>",
        pstMD_CreateProfileRes->stProfile.enFixed ? "true" : "false", 
        pstMD_CreateProfileRes->stProfile.pszToken, 
        pstMD_CreateProfileRes->stProfile.pszName);

    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Profile>");
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:CreateProfileResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetProfile(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{    
    GONVIF_MEDIA_GetProfile_Res_S *pstMD_GetProfileRes = (GONVIF_MEDIA_GetProfile_Res_S *)argv;
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetProfileResponse>");

    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Profile fixed=\"%s\" token=\"%s\">"
                                                    "<tt:Name>%s</tt:Name>",
        pstMD_GetProfileRes->stProfile.enFixed? "true" : "false", 
        pstMD_GetProfileRes->stProfile.pszToken,
        pstMD_GetProfileRes->stProfile.pszName);
    //VideoSourceConfiguration
    if(pstMD_GetProfileRes->stProfile.pstVideoSourceConfiguration)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:VideoSourceConfiguration token=\"%s\">",
            pstMD_GetProfileRes->stProfile.pstVideoSourceConfiguration->pszToken);
        offset += snprintf(p_buf+offset, mlen-offset, video_source_config,
            pstMD_GetProfileRes->stProfile.pstVideoSourceConfiguration->pszName,
            pstMD_GetProfileRes->stProfile.pstVideoSourceConfiguration->useCount,
            pstMD_GetProfileRes->stProfile.pstVideoSourceConfiguration->pszSourceToken,
            pstMD_GetProfileRes->stProfile.pstVideoSourceConfiguration->stBounds.height, pstMD_GetProfileRes->stProfile.pstVideoSourceConfiguration->stBounds.width,
            pstMD_GetProfileRes->stProfile.pstVideoSourceConfiguration->stBounds.y, pstMD_GetProfileRes->stProfile.pstVideoSourceConfiguration->stBounds.x);
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:VideoSourceConfiguration>");
    }
    //AudioSourceConfiguration
    if(pstMD_GetProfileRes->stProfile.pstAudioSourceConfiguration)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:AudioSourceConfiguration token=\"%s\">",
            pstMD_GetProfileRes->stProfile.pstAudioSourceConfiguration->pszToken);
        offset += snprintf(p_buf+offset, mlen-offset, audio_source_config, 
            pstMD_GetProfileRes->stProfile.pstAudioSourceConfiguration->pszName,
            pstMD_GetProfileRes->stProfile.pstAudioSourceConfiguration->useCount, 
            pstMD_GetProfileRes->stProfile.pstAudioSourceConfiguration->pszSourceToken);
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:AudioSourceConfiguration>");
    }
    //VideoEncoderConfiguration
    if (pstMD_GetProfileRes->stProfile.pstVideoEncoderConfiguration)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:VideoEncoderConfiguration token=\"%s\">",
            pstMD_GetProfileRes->stProfile.pstVideoEncoderConfiguration->pszToken);
        offset += packet_VideoEncoderConfiguration(p_buf+offset, mlen-offset, pstMD_GetProfileRes->stProfile.pstVideoEncoderConfiguration);
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:VideoEncoderConfiguration>");
    }
    //AudioEncoderConfiguration
    if (pstMD_GetProfileRes->stProfile.pstAudioEncoderConfiguration)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:AudioEncoderConfiguration token=\"%s\">", 
            pstMD_GetProfileRes->stProfile.pstAudioEncoderConfiguration->pszToken);
        offset += snprintf(p_buf+offset, mlen-offset, audio_encoder_config,
            pstMD_GetProfileRes->stProfile.pstAudioEncoderConfiguration->pszName,
    	    pstMD_GetProfileRes->stProfile.pstAudioEncoderConfiguration->useCount,
    	    get_audioEncodingType_string(pstMD_GetProfileRes->stProfile.pstAudioEncoderConfiguration->enEncoding),
    	    pstMD_GetProfileRes->stProfile.pstAudioEncoderConfiguration->bitRate,
    	    pstMD_GetProfileRes->stProfile.pstAudioEncoderConfiguration->sampleRate,
    	    pstMD_GetProfileRes->stProfile.pstAudioEncoderConfiguration->stMulticast.stIPAddress.pszIPv4Address,
    	    pstMD_GetProfileRes->stProfile.pstAudioEncoderConfiguration->stMulticast.port,
    	    pstMD_GetProfileRes->stProfile.pstAudioEncoderConfiguration->stMulticast.ttl,
    	    pstMD_GetProfileRes->stProfile.pstAudioEncoderConfiguration->stMulticast.enAutoStart? "true" : "false",
            pstMD_GetProfileRes->stProfile.pstAudioEncoderConfiguration->sessionTimeout);
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:AudioEncoderConfiguration>");
    }
    //VideoAnalyticsConfiguration
    if(pstMD_GetProfileRes->stProfile.pstVideoAnalyticsConfiguration)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:VideoAnalyticsConfiguration token=\"%s\">", 
            pstMD_GetProfileRes->stProfile.pstVideoAnalyticsConfiguration->pszToken);
    	offset += packet_VideoAnalyticsConfiguration(p_buf+offset, mlen-offset, pstMD_GetProfileRes->stProfile.pstVideoAnalyticsConfiguration);

        offset += snprintf(p_buf+offset, mlen-offset, "</tt:VideoAnalyticsConfiguration>");
    }
    //PTZConfiguration
    if (pstMD_GetProfileRes->stProfile.pstPTZConfiguration)
    {
    	offset += packet_PTZConfiguration(p_buf+offset, mlen-offset, pstMD_GetProfileRes->stProfile.pstPTZConfiguration);
    }
    //MetadataConfiguration
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Profile>");


    offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetProfileResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetProfiles(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	//ONVIF_INFO("===========>enter soap_packet_response_GetProfiles\n");

    GK_S32 i = 0;
    GONVIF_MEDIA_GetProfiles_Res_S *pstMD_GetProfilesRes = (GONVIF_MEDIA_GetProfiles_Res_S *)argv;
    
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetProfilesResponse>");

    for(i = 0; i < pstMD_GetProfilesRes->sizeProfiles; i++)
    {	
		//ONVIF_INFO("===========>enter soap_packet_response_GetProfiles==>for\n");
        offset += snprintf(p_buf+offset, mlen-offset,
            "<trt:Profiles fixed=\"%s\" token=\"%s\">"
            "<tt:Name>%s</tt:Name>",
            pstMD_GetProfilesRes->stProfiles[i].enFixed? "true" : "false", 
            pstMD_GetProfilesRes->stProfiles[i].pszToken, 
            pstMD_GetProfilesRes->stProfiles[i].pszName);
            
        //VideoSourceConfiguration
        if(pstMD_GetProfilesRes->stProfiles[i].pstVideoSourceConfiguration)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:VideoSourceConfiguration token=\"%s\">", 
                pstMD_GetProfilesRes->stProfiles[i].pstVideoSourceConfiguration->pszToken);
            offset += snprintf(p_buf+offset, mlen-offset, video_source_config,
                pstMD_GetProfilesRes->stProfiles[i].pstVideoSourceConfiguration->pszName,
                pstMD_GetProfilesRes->stProfiles[i].pstVideoSourceConfiguration->useCount,
                pstMD_GetProfilesRes->stProfiles[i].pstVideoSourceConfiguration->pszSourceToken,
                pstMD_GetProfilesRes->stProfiles[i].pstVideoSourceConfiguration->stBounds.height, pstMD_GetProfilesRes->stProfiles[i].pstVideoSourceConfiguration->stBounds.width,
                pstMD_GetProfilesRes->stProfiles[i].pstVideoSourceConfiguration->stBounds.y, pstMD_GetProfilesRes->stProfiles[i].pstVideoSourceConfiguration->stBounds.x);
            offset += snprintf(p_buf+offset, mlen-offset, "</tt:VideoSourceConfiguration>");
        }
        
        //AudioSourceConfiguration
        if(pstMD_GetProfilesRes->stProfiles[i].pstAudioSourceConfiguration)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:AudioSourceConfiguration token=\"%s\">", 
                pstMD_GetProfilesRes->stProfiles[i].pstAudioSourceConfiguration->pszToken);
            offset += snprintf(p_buf+offset, mlen-offset, audio_source_config, 
                pstMD_GetProfilesRes->stProfiles[i].pstAudioSourceConfiguration->pszName,
                pstMD_GetProfilesRes->stProfiles[i].pstAudioSourceConfiguration->useCount, 
                pstMD_GetProfilesRes->stProfiles[i].pstAudioSourceConfiguration->pszSourceToken);
            offset += snprintf(p_buf+offset, mlen-offset, "</tt:AudioSourceConfiguration>");
        }

        //VideoEncoderConfiguration
        if (pstMD_GetProfilesRes->stProfiles[i].pstVideoEncoderConfiguration)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:VideoEncoderConfiguration token=\"%s\">",
                pstMD_GetProfilesRes->stProfiles[i].pstVideoEncoderConfiguration->pszToken);
            offset += packet_VideoEncoderConfiguration(p_buf+offset, mlen-offset, pstMD_GetProfilesRes->stProfiles[i].pstVideoEncoderConfiguration);
            offset += snprintf(p_buf+offset, mlen-offset, "</tt:VideoEncoderConfiguration>");
        }
        //AudioEncoderConfiguration
        if (pstMD_GetProfilesRes->stProfiles[i].pstAudioEncoderConfiguration)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:AudioEncoderConfiguration token=\"%s\">", 
                pstMD_GetProfilesRes->stProfiles[i].pstAudioEncoderConfiguration->pszToken);
            offset += snprintf(p_buf+offset, mlen-offset, audio_encoder_config,
                pstMD_GetProfilesRes->stProfiles[i].pstAudioEncoderConfiguration->pszName,
                pstMD_GetProfilesRes->stProfiles[i].pstAudioEncoderConfiguration->useCount,
                get_audioEncodingType_string(pstMD_GetProfilesRes->stProfiles[i].pstAudioEncoderConfiguration->enEncoding),
                pstMD_GetProfilesRes->stProfiles[i].pstAudioEncoderConfiguration->bitRate,
                pstMD_GetProfilesRes->stProfiles[i].pstAudioEncoderConfiguration->sampleRate,
                pstMD_GetProfilesRes->stProfiles[i].pstAudioEncoderConfiguration->stMulticast.stIPAddress.pszIPv4Address,
                pstMD_GetProfilesRes->stProfiles[i].pstAudioEncoderConfiguration->stMulticast.port,
                pstMD_GetProfilesRes->stProfiles[i].pstAudioEncoderConfiguration->stMulticast.ttl,
                pstMD_GetProfilesRes->stProfiles[i].pstAudioEncoderConfiguration->stMulticast.enAutoStart? "true" : "false",
                pstMD_GetProfilesRes->stProfiles[i].pstAudioEncoderConfiguration->sessionTimeout);
            offset += snprintf(p_buf+offset, mlen-offset, "</tt:AudioEncoderConfiguration>");
        }
        //VideoAnalyticsConfiguration
        if(pstMD_GetProfilesRes->stProfiles[i].pstVideoAnalyticsConfiguration)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:VideoAnalyticsConfiguration token=\"%s\">",
                pstMD_GetProfilesRes->stProfiles[i].pstVideoAnalyticsConfiguration->pszToken);
            
            offset += packet_VideoAnalyticsConfiguration(p_buf+offset, mlen-offset, pstMD_GetProfilesRes->stProfiles[i].pstVideoAnalyticsConfiguration);

            offset += snprintf(p_buf+offset, mlen-offset, "</tt:VideoAnalyticsConfiguration>");
        }
        //PTZConfiguration
        if (pstMD_GetProfilesRes->stProfiles[i].pstPTZConfiguration)
        {
            offset += packet_PTZConfiguration(p_buf+offset, mlen-offset, pstMD_GetProfilesRes->stProfiles[i].pstPTZConfiguration);
        }
        
        //MetadataConfiguration
        offset += snprintf(p_buf+offset, mlen-offset, "</trt:Profiles>");
        
		//ONVIF_INFO("===========>goout soap_packet_response_GetProfiles==>for\n");
    }

    offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetProfilesResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	//ONVIF_INFO("===========>goout soap_packet_response_GetProfiles\n");
	return offset;
}

GK_S32 soap_packet_response_DeleteProfile(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<trt:DeleteProfileResponse>"
                                                  "</trt:DeleteProfileResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_AddVideoSourceConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<trt:AddVideoSourceConfigurationResponse>"
                                                  "</trt:AddVideoSourceConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_AddVideoEncoderConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:AddVideoEncoderConfigurationResponse>"
	                                              "</trt:AddVideoEncoderConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_AddAudioSourceConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<trt:AddAudioSourceConfigurationResponse>"
                                                  "</trt:AddAudioSourceConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_AddAudioEncoderConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:AddAudioEncoderConfigurationResponse>"
	                                              "</trt:AddAudioEncoderConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_AddPTZConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:AddPTZConfigurationResponse>"
	                                              "</trt:AddPTZConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_RemoveVideoSourceConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<trt:RemoveVideoSourceConfigurationResponse>"
                                                  "</trt:RemoveVideoSourceConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_RemoveVideoEncoderConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:RemoveVideoEncoderConfigurationResponse>"
	                                              "</trt:RemoveVideoEncoderConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}


GK_S32 soap_packet_response_RemoveAudioSourceConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<trt:RemoveAudioSourceConfigurationResponse>"
                                                  "</trt:RemoveAudioSourceConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_RemoveAudioEncoderConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:RemoveAudioEncoderConfigurationResponse>"
	                                              "</trt:RemoveAudioEncoderConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_RemovePTZConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:RemovePTZConfigurationResponse>"
	                                              "</trt:RemovePTZConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetVideoSourceConfigurations(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 i = 0;
    GONVIF_MEDIA_GetVideoSourceConfigurations_Res_S *pstMD_GetVideoSourceConfigurationsRes = (GONVIF_MEDIA_GetVideoSourceConfigurations_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoSourceConfigurationsResponse>");

	for(i = 0; i < pstMD_GetVideoSourceConfigurationsRes->sizeConfigurations; i++)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", pstMD_GetVideoSourceConfigurationsRes->stConfigurations[i].pszToken);
	    offset += snprintf(p_buf+offset, mlen-offset, video_source_config,  
	        pstMD_GetVideoSourceConfigurationsRes->stConfigurations[i].pszName,
	         pstMD_GetVideoSourceConfigurationsRes->stConfigurations[i].useCount,  
	         pstMD_GetVideoSourceConfigurationsRes->stConfigurations[i].pszSourceToken,
	         pstMD_GetVideoSourceConfigurationsRes->stConfigurations[i].stBounds.height,
	         pstMD_GetVideoSourceConfigurationsRes->stConfigurations[i].stBounds.width,
	         pstMD_GetVideoSourceConfigurationsRes->stConfigurations[i].stBounds.y,
	         pstMD_GetVideoSourceConfigurationsRes->stConfigurations[i].stBounds.x);
	    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
	}

	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoSourceConfigurationsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetVideoSourceConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_MEDIA_GetVideoSourceConfiguration_Res_S *pstMD_GetVideoSourceConfigurationRes = (GONVIF_MEDIA_GetVideoSourceConfiguration_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoSourceConfigurationResponse>");

    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configuration token=\"%s\">", 
        pstMD_GetVideoSourceConfigurationRes->stConfiguration.pszToken);
    offset += snprintf(p_buf+offset, mlen-offset, video_source_config, 
        pstMD_GetVideoSourceConfigurationRes->stConfiguration.pszName,
        pstMD_GetVideoSourceConfigurationRes->stConfiguration.useCount, 
        pstMD_GetVideoSourceConfigurationRes->stConfiguration.pszSourceToken,
        pstMD_GetVideoSourceConfigurationRes->stConfiguration.stBounds.height,
        pstMD_GetVideoSourceConfigurationRes->stConfiguration.stBounds.width,
        pstMD_GetVideoSourceConfigurationRes->stConfiguration.stBounds.y, 
        pstMD_GetVideoSourceConfigurationRes->stConfiguration.stBounds.x);
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configuration>");

	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoSourceConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetCompatibleVideoSourceConfigurations(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 i = 0;
    GONVIF_MEDIA_GetCompatibleVideoSourceConfigurations_Res_S *pstMD_GetCompatibleVideoSourceConfigurationsRes = (GONVIF_MEDIA_GetCompatibleVideoSourceConfigurations_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetCompatibleVideoSourceConfigurationsResponse>");

	for(i = 0; i < pstMD_GetCompatibleVideoSourceConfigurationsRes->sizeConfigurations; i++)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", 
	        pstMD_GetCompatibleVideoSourceConfigurationsRes->stConfigurations[i].pszToken);
	    offset += snprintf(p_buf+offset, mlen-offset, video_source_config,  
	         pstMD_GetCompatibleVideoSourceConfigurationsRes->stConfigurations[i].pszName,
	         pstMD_GetCompatibleVideoSourceConfigurationsRes->stConfigurations[i].useCount,  
	         pstMD_GetCompatibleVideoSourceConfigurationsRes->stConfigurations[i].pszSourceToken,
	         pstMD_GetCompatibleVideoSourceConfigurationsRes->stConfigurations[i].stBounds.height,
	         pstMD_GetCompatibleVideoSourceConfigurationsRes->stConfigurations[i].stBounds.width,
	         pstMD_GetCompatibleVideoSourceConfigurationsRes->stConfigurations[i].stBounds.y, 
	         pstMD_GetCompatibleVideoSourceConfigurationsRes->stConfigurations[i].stBounds.x);
	    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
	}

	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetCompatibleVideoSourceConfigurationsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetVideoSourceConfigurationOptions(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 i = 0;
    GONVIF_MEDIA_VideoSourceConfigurationOptions_S *pstGetVideoSourceCfgOptionsRes = (GONVIF_MEDIA_VideoSourceConfigurationOptions_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoSourceConfigurationOptionsResponse>");

    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Options>");

    offset += snprintf(p_buf+offset, mlen-offset, "<tt:BoundsRange>"
        "<tt:XRange><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:XRange>"
        "<tt:YRange><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:YRange>"
        "<tt:WidthRange><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:WidthRange>"
        "<tt:HeightRange><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:HeightRange></tt:BoundsRange>",
        pstGetVideoSourceCfgOptionsRes->stBoundsRange.stXRange.min, pstGetVideoSourceCfgOptionsRes->stBoundsRange.stXRange.max,
        pstGetVideoSourceCfgOptionsRes->stBoundsRange.stYRange.min, pstGetVideoSourceCfgOptionsRes->stBoundsRange.stYRange.max,
        pstGetVideoSourceCfgOptionsRes->stBoundsRange.stWidthRange.min, pstGetVideoSourceCfgOptionsRes->stBoundsRange.stWidthRange.max,
        pstGetVideoSourceCfgOptionsRes->stBoundsRange.stHeightRange.min, pstGetVideoSourceCfgOptionsRes->stBoundsRange.stHeightRange.max);

    for(i = 0; i < pstGetVideoSourceCfgOptionsRes->sizeVideoSourceTokensAvailable; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:VideoSourceTokensAvailable>%s</tt:VideoSourceTokensAvailable>", 
            pstGetVideoSourceCfgOptionsRes->pszVideoSourceTokensAvailable[i]);
    }
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Options>");
   
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoSourceConfigurationOptionsResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_SetVideoSourceConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:SetVideoSourceConfigurationResponse>"
	                                              "</trt:SetVideoSourceConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetVideoEncoderConfigurations(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_MEDIA_GetVideoEncoderConfigurations_Res_S *pstMD_GetVideoEncoderConfigurationsRes = (GONVIF_MEDIA_GetVideoEncoderConfigurations_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoEncoderConfigurationsResponse>");
    GK_S32 i = 0;
	for(i = 0; i < pstMD_GetVideoEncoderConfigurationsRes->sizeConfigurations; i++)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", 
	        pstMD_GetVideoEncoderConfigurationsRes->stConfigurations[i].pszToken);
    	offset += packet_VideoEncoderConfiguration(p_buf+offset, mlen-offset, &pstMD_GetVideoEncoderConfigurationsRes->stConfigurations[i]);
    	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
   	}

	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoEncoderConfigurationsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

GK_S32 soap_packet_response_GetVideoEncoderConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_MEDIA_GetVideoEncoderConfiguration_Res_S *pstMD_GetVideoEncoderConfigurationRes = (GONVIF_MEDIA_GetVideoEncoderConfiguration_Res_S *)argv;
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoEncoderConfigurationResponse>");

    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configuration token=\"%s\">", 
        pstMD_GetVideoEncoderConfigurationRes->stConfiguration.pszToken);
    offset += packet_VideoEncoderConfiguration(p_buf+offset, mlen-offset, &pstMD_GetVideoEncoderConfigurationRes->stConfiguration);
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configuration>");

	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoEncoderConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

GK_S32 soap_packet_response_GetCompatibleVideoEncoderConfigurations(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_MEDIA_GetCompatibleVideoEncoderConfigurations_Res_S *pstMD_GetCompatibleVideoEncoderConfigurationsRes = (GONVIF_MEDIA_GetCompatibleVideoEncoderConfigurations_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetCompatibleVideoEncoderConfigurationsResponse>");
    GK_S32 i = 0;
	for(i = 0; i < pstMD_GetCompatibleVideoEncoderConfigurationsRes->sizeConfigurations; i++)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", 
	        pstMD_GetCompatibleVideoEncoderConfigurationsRes->stConfigurations[i].pszToken);
    	offset += packet_VideoEncoderConfiguration(p_buf+offset, mlen-offset, &pstMD_GetCompatibleVideoEncoderConfigurationsRes->stConfigurations[i]);
    	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
   	}

	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetCompatibleVideoEncoderConfigurationsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

GK_S32 soap_packet_response_GetVideoEncoderConfigurationOptions(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GONVIF_MEDIA_GetVideoEncoderConfigurationOptions_Res_S *pstMD_GetVideoEncoderConfigurationOptionsRes = (GONVIF_MEDIA_GetVideoEncoderConfigurationOptions_Res_S *)argv;
    GK_S32 i = 0;
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetVideoEncoderConfigurationOptionsResponse>");
    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Options>");
    
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:QualityRange>"
    												"<tt:Min>%d</tt:Min>"
    												"<tt:Max>%d</tt:Max>"
												  "</tt:QualityRange>",
        pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stQualityRange.min, 
        pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stQualityRange.max);
#if 0
//JPEG options
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:JPEG>");

    for(i = 0; i < pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stJPEG.sizeResolutionsAvailable; i++)
    {       
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:ResolutionsAvailable>"
                                                        "<tt:Width>%d</tt:Width>"
                                                        "<tt:Height>%d</tt:Height>"
                                                      "</tt:ResolutionsAvailable>",
            pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stJPEG.pstResolutionsAvailable[i].width,
            pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stJPEG.pstResolutionsAvailable[i].height);
    }

    offset += snprintf(p_buf+offset, mlen-offset, "<tt:FrameRateRange>"
                                                    "<tt:Min>%d</tt:Min>"
                                                    "<tt:Max>%d</tt:Max>"
                                                  "</tt:FrameRateRange>"
                                                  "<tt:EncodingIntervalRange>"
                                                    "<tt:Min>%d</tt:Min>"
                                                    "<tt:Max>%d</tt:Max>"
                                                  "</tt:EncodingIntervalRange>",
        pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stJPEG.stFrameRateRange.min,
        pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stJPEG.stFrameRateRange.max,
        pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stJPEG.stEncodingIntervalRange.min,
        pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stJPEG.stEncodingIntervalRange.max);
        
    offset += snprintf(p_buf+offset, mlen-offset, "</tt:JPEG>");
#endif
//H264 options        
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:H264>");

    for (i = 0; i < pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.sizeResolutionsAvailable; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:ResolutionsAvailable>"
                                                        "<tt:Width>%d</tt:Width>"
                                                        "<tt:Height>%d</tt:Height>"
                                                      "</tt:ResolutionsAvailable>",
            pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.pstResolutionsAvailable[i].width,
            pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.pstResolutionsAvailable[i].height);
    }            

    offset += snprintf(p_buf+offset, mlen-offset, "<tt:GovLengthRange>"
                                                    "<tt:Min>%d</tt:Min>"
                                                    "<tt:Max>%d</tt:Max>"
                                                  "</tt:GovLengthRange>"
                                                  "<tt:FrameRateRange>"
                                                    "<tt:Min>%d</tt:Min>"
                                                    "<tt:Max>%d</tt:Max>"
                                                  "</tt:FrameRateRange>"
                                                  "<tt:EncodingIntervalRange>"
                                                    "<tt:Min>%d</tt:Min>"
                                                    "<tt:Max>%d</tt:Max>"
                                                  "</tt:EncodingIntervalRange>",
        pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.stGovLengthRange.min,
        pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.stGovLengthRange.max, 
        pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.stFrameRateRange.min,
        pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.stFrameRateRange.max,
        pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.stEncodingIntervalRange.min,
        pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.stEncodingIntervalRange.max);

    for(i = 0; i < pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.sizeH264ProfilesSupported; i++)
    {
        if(pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.penH264ProfilesSupported[i] == H264Profile_BaseLine)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:H264ProfilesSupported>Baseline</tt:H264ProfilesSupported>");
        }
        else if (pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.penH264ProfilesSupported[i] == H264Profile_Main)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:H264ProfilesSupported>Main</tt:H264ProfilesSupported>");
        }
        else if (pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.penH264ProfilesSupported[i] == H264Profile_High)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:H264ProfilesSupported>High</tt:H264ProfilesSupported>");
        }
        else
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:H264ProfilesSupported>Extended</tt:H264ProfilesSupported>");
        }
    }
    offset += snprintf(p_buf+offset, mlen-offset, "</tt:H264>");  
    
//Extension for returning Bitrate range. But it will occur error in ONVIF test tool.
#if 1
    //H264 Extension
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Extension>");
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:H264>");  

    for (i = 0; i < pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.sizeResolutionsAvailable; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:ResolutionsAvailable>"
                                                        "<tt:Width>%d</tt:Width>"
                                                        "<tt:Height>%d</tt:Height>"
                                                      "</tt:ResolutionsAvailable>",
            pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.pstResolutionsAvailable[i].width,
            pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.pstResolutionsAvailable[i].height);
    }            

    offset += snprintf(p_buf+offset, mlen-offset, "<tt:GovLengthRange>"
                                                    "<tt:Min>%d</tt:Min>"
                                                    "<tt:Max>%d</tt:Max>"
                                                  "</tt:GovLengthRange>"
                                                  "<tt:FrameRateRange>"
                                                    "<tt:Min>%d</tt:Min>"
                                                    "<tt:Max>%d</tt:Max>"
                                                  "</tt:FrameRateRange>"
                                                  "<tt:EncodingIntervalRange>"
                                                    "<tt:Min>%d</tt:Min>"
                                                    "<tt:Max>%d</tt:Max>"
                                                  "</tt:EncodingIntervalRange>",
        pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.stGovLengthRange.min,
        pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.stGovLengthRange.max, 
        pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.stFrameRateRange.min,
        pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.stFrameRateRange.max,
        pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.stEncodingIntervalRange.min,
        pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.stEncodingIntervalRange.max);
        
    for(i = 0; i < pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.sizeH264ProfilesSupported; i++)
    {
        if(pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.penH264ProfilesSupported[i] == H264Profile_BaseLine)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:H264ProfilesSupported>Baseline</tt:H264ProfilesSupported>");
        }
        else if (pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.penH264ProfilesSupported[i] == H264Profile_Main)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:H264ProfilesSupported>Main</tt:H264ProfilesSupported>");
        }
        else if (pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stH264.penH264ProfilesSupported[i] == H264Profile_High)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:H264ProfilesSupported>High</tt:H264ProfilesSupported>");
        }
        else
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:H264ProfilesSupported>Extended</tt:H264ProfilesSupported>");
        }
    }
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:BitrateRange>"
                                                    "<tt:Min>%d</tt:Min>"
                                                    "<tt:Max>%d</tt:Max>"
                                                  "</tt:BitrateRange>",
        pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stExtension.stH264.stBitrateRange.min,
        pstMD_GetVideoEncoderConfigurationOptionsRes->stOptions.stExtension.stH264.stBitrateRange.max);
    offset += snprintf(p_buf+offset, mlen-offset, "</tt:H264>");  
    offset += snprintf(p_buf+offset, mlen-offset, "</tt:Extension>");
#endif

    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Options>");      
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetVideoEncoderConfigurationOptionsResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

GK_S32 soap_packet_response_SetVideoEncoderConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:SetVideoEncoderConfigurationResponse></trt:SetVideoEncoderConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetGuaranteedNumberOfVideoEncoderInstances(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_MEDIA_GetGuaranteedNumberOfVideoEncoderInstances_Res_S *pstMD_GetGuaranteedNumberOfVideoEncoderInstancesRes = (GONVIF_MEDIA_GetGuaranteedNumberOfVideoEncoderInstances_Res_S *)argv;
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tptz:GetGuaranteedNumberOfVideoEncoderInstancesResponse>"
		    "<trt:TotalNumber>%d</trt:TotalNumber>"
		    "<trt:JPEG>%d</trt:JPEG>"
		    "<trt:H264>%d</trt:H264>"
		    "<trt:MPEG4>%d</trt:MPEG4>"
		"</tptz:GetGuaranteedNumberOfVideoEncoderInstancesResponse>",
		pstMD_GetGuaranteedNumberOfVideoEncoderInstancesRes->totalNumber,
		pstMD_GetGuaranteedNumberOfVideoEncoderInstancesRes->JPEG,
		pstMD_GetGuaranteedNumberOfVideoEncoderInstancesRes->H264, 
		pstMD_GetGuaranteedNumberOfVideoEncoderInstancesRes->MPEG4);
		
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetAudioSourceConfigurations(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 i = 0;
    GONVIF_MEDIA_GetAudioSourceConfigurations_Res_S *pstMD_GetAudioSourceConfigurationRes = (GONVIF_MEDIA_GetAudioSourceConfigurations_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioSourceConfigurationsResponse>");

	for(i = 0; i < pstMD_GetAudioSourceConfigurationRes->sizeConfigurations; i++)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", pstMD_GetAudioSourceConfigurationRes->stConfigurations[i].pszToken);
	    offset += snprintf(p_buf+offset, mlen-offset, audio_source_config,
            pstMD_GetAudioSourceConfigurationRes->stConfigurations[i].pszName,
            pstMD_GetAudioSourceConfigurationRes->stConfigurations[i].useCount,
            pstMD_GetAudioSourceConfigurationRes->stConfigurations[i].pszSourceToken);
	    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
	}

	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetAudioSourceConfigurationsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetAudioSourceConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_MEDIA_GetAudioSourceConfiguration_Res_S *pstMD_GetAudioSourceConfigurationRes = (GONVIF_MEDIA_GetAudioSourceConfiguration_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioSourceConfigurationResponse>");

    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configuration token=\"%s\">", 
        pstMD_GetAudioSourceConfigurationRes->stConfiguration.pszToken);
    offset += snprintf(p_buf+offset, mlen-offset, audio_source_config,
        pstMD_GetAudioSourceConfigurationRes->stConfiguration.pszName,
        pstMD_GetAudioSourceConfigurationRes->stConfiguration.useCount,
        pstMD_GetAudioSourceConfigurationRes->stConfiguration.pszSourceToken);
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configuration>");

	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetAudioSourceConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetCompatibleAudioSourceConfigurations(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 i = 0;
    GONVIF_MEDIA_GetCompatibleAudioSourceConfigurations_Res_S *pstMD_GetCompatibleAudioSourceConfigurationsRes = (GONVIF_MEDIA_GetCompatibleAudioSourceConfigurations_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetCompatibleAudioSourceConfigurationsResponse>");

	for(i = 0; i < pstMD_GetCompatibleAudioSourceConfigurationsRes->sizeConfigurations; i++)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">",
	        pstMD_GetCompatibleAudioSourceConfigurationsRes->stConfigurations[i].pszToken);
	    offset += snprintf(p_buf+offset, mlen-offset, audio_source_config,
            pstMD_GetCompatibleAudioSourceConfigurationsRes->stConfigurations[i].pszName,
            pstMD_GetCompatibleAudioSourceConfigurationsRes->stConfigurations[i].useCount,
            pstMD_GetCompatibleAudioSourceConfigurationsRes->stConfigurations[i].pszSourceToken);
	    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
	}

	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetCompatibleAudioSourceConfigurationsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetAudioSourceConfigurationOptions(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 i = 0, j = 0;
    GONVIF_MEDIA_GetAudioSourceConfigurationOptions_Res_S *pstMD_GetAudioSourceConfigurationOptionsRes = (GONVIF_MEDIA_GetAudioSourceConfigurationOptions_Res_S *)argv;

    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

    offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
    offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioSourceConfigurationOptionsResponse>");

    for(i = 0; i < g_GkIpc_OnvifInf.AudioSourceOptionNum; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<trt:Options>");
        for(j = 0; j < pstMD_GetAudioSourceConfigurationOptionsRes->stOptions.sizeInputTokensAvailable; j++)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:InputTokensAvailable>%s</tt:InputTokensAvailable>",
            pstMD_GetAudioSourceConfigurationOptionsRes->stOptions.pszInputTokensAvailable[j]);
        }
        offset += snprintf(p_buf+offset, mlen-offset, "</trt:Options>");
    }
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetAudioSourceConfigurationOptionsResponse>");

    offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

    return offset;
}

GK_S32 soap_packet_response_SetAudioSourceConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:SetAudioSourceConfigurationResponse>"
	                                              "</trt:SetAudioSourceConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetAudioEncoderConfigurations(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_MEDIA_GetAudioEncoderConfigurations_Res_S *pstMD_GetAudioEncoderConfigurationsRes = (GONVIF_MEDIA_GetAudioEncoderConfigurations_Res_S *)argv;

    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

    offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
    offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioEncoderConfigurationsResponse>");
    GK_S32 i = 0;
    for(i = 0; i < pstMD_GetAudioEncoderConfigurationsRes->sizeConfigurations; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">",
            pstMD_GetAudioEncoderConfigurationsRes->stConfigurations[i].pszToken);
            
        offset += snprintf(p_buf+offset, mlen-offset, audio_encoder_config,
            pstMD_GetAudioEncoderConfigurationsRes->stConfigurations[i].pszName,
            pstMD_GetAudioEncoderConfigurationsRes->stConfigurations[i].useCount,
            get_audioEncodingType_string(pstMD_GetAudioEncoderConfigurationsRes->stConfigurations[i].enEncoding),
            pstMD_GetAudioEncoderConfigurationsRes->stConfigurations[i].bitRate,
            pstMD_GetAudioEncoderConfigurationsRes->stConfigurations[i].sampleRate,
            pstMD_GetAudioEncoderConfigurationsRes->stConfigurations[i].stMulticast.stIPAddress.pszIPv4Address,
            pstMD_GetAudioEncoderConfigurationsRes->stConfigurations[i].stMulticast.port,
            pstMD_GetAudioEncoderConfigurationsRes->stConfigurations[i].stMulticast.ttl,
            pstMD_GetAudioEncoderConfigurationsRes->stConfigurations[i].stMulticast.enAutoStart? "true" : "false",
            pstMD_GetAudioEncoderConfigurationsRes->stConfigurations[i].sessionTimeout);
            
    	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
	}

	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetAudioEncoderConfigurationsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

GK_S32 soap_packet_response_GetAudioEncoderConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_MEDIA_GetAudioEncoderConfiguration_Res_S *pstMD_GetAudioEncoderConfigurationRes = (GONVIF_MEDIA_GetAudioEncoderConfiguration_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioEncoderConfigurationResponse>");

    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configuration token=\"%s\">", 
        pstMD_GetAudioEncoderConfigurationRes->stConfiguration.pszToken);
	offset += snprintf(p_buf+offset, mlen-offset, audio_encoder_config,
        pstMD_GetAudioEncoderConfigurationRes->stConfiguration.pszName,
        pstMD_GetAudioEncoderConfigurationRes->stConfiguration.useCount,
        get_audioEncodingType_string(pstMD_GetAudioEncoderConfigurationRes->stConfiguration.enEncoding),
        pstMD_GetAudioEncoderConfigurationRes->stConfiguration.bitRate,
        pstMD_GetAudioEncoderConfigurationRes->stConfiguration.sampleRate,
        pstMD_GetAudioEncoderConfigurationRes->stConfiguration.stMulticast.stIPAddress.pszIPv4Address,
        pstMD_GetAudioEncoderConfigurationRes->stConfiguration.stMulticast.port,
        pstMD_GetAudioEncoderConfigurationRes->stConfiguration.stMulticast.ttl,
        pstMD_GetAudioEncoderConfigurationRes->stConfiguration.stMulticast.enAutoStart? "true" : "false",
        pstMD_GetAudioEncoderConfigurationRes->stConfiguration.sessionTimeout);
	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configuration>");

	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetAudioEncoderConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

GK_S32 soap_packet_response_GetCompatibleAudioEncoderConfigurations(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 i = 0;
    GONVIF_MEDIA_GetCompatibleAudioEncoderConfigurations_Res_S *pstMD_GetCompatibleAudioEncoderConfigurationsRes = (GONVIF_MEDIA_GetCompatibleAudioEncoderConfigurations_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetCompatibleAudioEncoderConfigurationsResponse>");

	for(i = 0; i < pstMD_GetCompatibleAudioEncoderConfigurationsRes->sizeConfigurations; i++)
	{
	    offset += snprintf(p_buf+offset, mlen-offset, "<trt:Configurations token=\"%s\">", 
	        pstMD_GetCompatibleAudioEncoderConfigurationsRes->stConfigurations[i].pszToken);
        offset += snprintf(p_buf+offset, mlen-offset, audio_encoder_config,
            pstMD_GetCompatibleAudioEncoderConfigurationsRes->stConfigurations[i].pszName,
            pstMD_GetCompatibleAudioEncoderConfigurationsRes->stConfigurations[i].useCount,
            get_audioEncodingType_string(pstMD_GetCompatibleAudioEncoderConfigurationsRes->stConfigurations[i].enEncoding),
            pstMD_GetCompatibleAudioEncoderConfigurationsRes->stConfigurations[i].bitRate,
            pstMD_GetCompatibleAudioEncoderConfigurationsRes->stConfigurations[i].sampleRate,
            pstMD_GetCompatibleAudioEncoderConfigurationsRes->stConfigurations[i].stMulticast.stIPAddress.pszIPv4Address,
            pstMD_GetCompatibleAudioEncoderConfigurationsRes->stConfigurations[i].stMulticast.port,
            pstMD_GetCompatibleAudioEncoderConfigurationsRes->stConfigurations[i].stMulticast.ttl,
            pstMD_GetCompatibleAudioEncoderConfigurationsRes->stConfigurations[i].stMulticast.enAutoStart? "true" : "false",
            pstMD_GetCompatibleAudioEncoderConfigurationsRes->stConfigurations[i].sessionTimeout);
	    offset += snprintf(p_buf+offset, mlen-offset, "</trt:Configurations>");
	}

	offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetCompatibleAudioEncoderConfigurationsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}


GK_S32 soap_packet_response_GetAudioEncoderConfigurationOptions(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GONVIF_MEDIA_GetAudioEncoderConfigurationOptions_Res_S *pstMD_GetAudioEncoderConfigurationOptionsRes = (GONVIF_MEDIA_GetAudioEncoderConfigurationOptions_Res_S *)argv;
    GK_S32 i = 0, j = 0;
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetAudioEncoderConfigurationOptionsResponse>"
	                                                "<trt:Options>");

    for(i = 0; i < pstMD_GetAudioEncoderConfigurationOptionsRes->stOptions.sizeOptions; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Options>");
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Encoding>%s</tt:Encoding>",
    	    get_audioEncodingType_string(pstMD_GetAudioEncoderConfigurationOptionsRes->stOptions.pstOptions[i].enEncoding));

        offset += snprintf(p_buf+offset, mlen-offset, "<tt:BitrateList>");
        for(j = 0; j < pstMD_GetAudioEncoderConfigurationOptionsRes->stOptions.pstOptions[i].stBitrateList.sizeItems; j++)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:Items>%d</tt:Items>",
                pstMD_GetAudioEncoderConfigurationOptionsRes->stOptions.pstOptions[i].stBitrateList.pItems[j]);
        }
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:BitrateList>");

        offset += snprintf(p_buf+offset, mlen-offset, "<tt:SampleRateList>");
        for(j = 0; j < pstMD_GetAudioEncoderConfigurationOptionsRes->stOptions.pstOptions[i].stSampleRateList.sizeItems; j++)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:Items>%d</tt:Items>",
                pstMD_GetAudioEncoderConfigurationOptionsRes->stOptions.pstOptions[i].stSampleRateList.pItems[j]);
        }
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:SampleRateList>");

        offset += snprintf(p_buf+offset, mlen-offset, "</tt:Options>");
    }

	offset += snprintf(p_buf+offset, mlen-offset, "</trt:Options>"
                                                "</trt:GetAudioEncoderConfigurationOptionsResponse>");
                                                
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_SetAudioEncoderConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:SetAudioDecoderConfigurationResponse>"
	                                              "</trt:SetAudioDecoderConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetMetadataConfigurations(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetMetadataConfigurationsResponse>"
	                                              "</trt:GetMetadataConfigurationsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetMetadataConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetMetadataConfigurationResponse>"
	                                              "</trt:GetMetadataConfigurationResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetCompatibleMetadataConfigurations(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetCompatibleMetadataConfigurationsResponse>"
	                                              "</trt:GetCompatibleMetadataConfigurationsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetMetadataConfigurationOptions(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetMetadataConfigurationOptionsResponse>"
	                                              "</trt:GetMetadataConfigurationOptionsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetStreamUri(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_MEDIA_GetStreamUri_Res_S *pstMD_GetStreamUriRes = (GONVIF_MEDIA_GetStreamUri_Res_S *)argv;
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	
    offset += snprintf(p_buf+offset, mlen-offset,
    	"<trt:GetStreamUriResponse>"
		    "<trt:MediaUri>"
			    "<tt:Uri>%s</tt:Uri>"
			    "<tt:InvalidAfterConnect>%s</tt:InvalidAfterConnect>"
			    "<tt:InvalidAfterReboot>%s</tt:InvalidAfterReboot>"
			    "<tt:Timeout>PT%lldS</tt:Timeout>"
		    "</trt:MediaUri>"
	    "</trt:GetStreamUriResponse>", 
	    pstMD_GetStreamUriRes->stMediaUri.aszUri,
	    pstMD_GetStreamUriRes->stMediaUri.enInvalidAfterConnect?"true" : "false",
	    pstMD_GetStreamUriRes->stMediaUri.enInvalidAfterReboot?"true" : "false",
	    pstMD_GetStreamUriRes->stMediaUri.timeout);
	    
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

GK_S32 soap_packet_response_GetSnapshotUri(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_MEIDA_GetSnapShotUri_Res_S *pstMD_GetSnapShotUriRes = (GONVIF_MEIDA_GetSnapShotUri_Res_S *)argv;
    

    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset,
        "<trt:GetSnapshotUriResponse>"
            "<trt:MediaUri>"
                "<tt:Uri>%s</tt:Uri>"
                "<tt:InvalidAfterConnect>%s</tt:InvalidAfterConnect>"
                "<tt:InvalidAfterReboot>%s</tt:InvalidAfterReboot>"
                "<tt:Timeout>PT%lldS</tt:Timeout>"
            "</trt:MediaUri>"
        "</trt:GetSnapshotUriResponse>",
        pstMD_GetSnapShotUriRes->stMediaUri.aszUri,
        pstMD_GetSnapShotUriRes->stMediaUri.enInvalidAfterConnect?  "true" : "false",
	    pstMD_GetSnapShotUriRes->stMediaUri.enInvalidAfterReboot ? "true" : "false",
	    pstMD_GetSnapShotUriRes->stMediaUri.timeout);
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

GK_S32 soap_packet_response_StartMulticastStreaming(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:StartMulticastStreamingResponse>"
	                                              "</trt:StartMulticastStreamingResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_StopMulticastStreaming(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<trt:StopMulticastStreamingResponse>"
	                                              "</trt:StopMulticastStreamingResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_GetOSDOptions(GK_CHAR * p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 i;
    GONVIF_MEDIA_GetOSDOptions_S *pstGetOSDOptionsRes = (GONVIF_MEDIA_GetOSDOptions_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, "<trt:GetOSDOptionsResponse>");
    offset += snprintf(p_buf+offset, mlen-offset, "<trt:OSDOptions>");

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tt:MaximumNumberOfOSDs Total=\"%d\" Image=\"%d\" PlainText=\"%d\" Date=\"%d\" Time=\"%d\" DateAndTime=\"%d\"></tt:MaximumNumberOfOSDs>"
        "<tt:Type>%s</tt:Type>",
                                                  pstGetOSDOptionsRes->stOSDOptions.stMaximumNumberOfOSDs.Total,
                                                  pstGetOSDOptionsRes->stOSDOptions.stMaximumNumberOfOSDs.Image,
                                                  pstGetOSDOptionsRes->stOSDOptions.stMaximumNumberOfOSDs.PlainText,
                                                  pstGetOSDOptionsRes->stOSDOptions.stMaximumNumberOfOSDs.Date,
                                                  pstGetOSDOptionsRes->stOSDOptions.stMaximumNumberOfOSDs.Time,
                                                  pstGetOSDOptionsRes->stOSDOptions.stMaximumNumberOfOSDs.DateAndTime,
                                                  
                                                  pstGetOSDOptionsRes->stOSDOptions.stType ? "Text" : "Text");
    for(i = 0;i < pstGetOSDOptionsRes->stOSDOptions.sizePositionOption; i++)
    {
        if(pstGetOSDOptionsRes->stOSDOptions.penPositionOption[i] == PositionOption_UpperLeft)
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:PositionOption>UpperLeft</tt:PositionOption>");
        else if(pstGetOSDOptionsRes->stOSDOptions.penPositionOption[i] == PositionOption_LowerLeft)
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:PositionOption>LowerLeft</tt:PositionOption>");
        else if(pstGetOSDOptionsRes->stOSDOptions.penPositionOption[i] == PositionOption_Custom)
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:PositionOption>Custom</tt:PositionOption>");
    }
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:TextOption>");
    for(i = 0;i < pstGetOSDOptionsRes->stOSDOptions.stTextOption.sizeType; i++)
    {
        if(pstGetOSDOptionsRes->stOSDOptions.stTextOption.penType[i] == TextType_Plain)
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>Plain</tt:Type>");
        else if(pstGetOSDOptionsRes->stOSDOptions.stTextOption.penType[i] == TextType_Date)
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>Date</tt:Type>");
        else if(pstGetOSDOptionsRes->stOSDOptions.stTextOption.penType[i] == TextType_Time)
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>Time</tt:Type>");
        else if(pstGetOSDOptionsRes->stOSDOptions.stTextOption.penType[i] == TextType_DateAndTime)
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>DateAndTime</tt:Type>");
    }
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:FontSizeRange><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:FontSizeRange>",
                                                  pstGetOSDOptionsRes->stOSDOptions.stTextOption.FontSizeRange.min,
                                                  pstGetOSDOptionsRes->stOSDOptions.stTextOption.FontSizeRange.max);

    for(i = 0;i < pstGetOSDOptionsRes->stOSDOptions.stTextOption.sizeDateFormat; i++)
    {
        if(pstGetOSDOptionsRes->stOSDOptions.stTextOption.penDateFormat[i] == DateFormat_MMddyyyy)
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:DateFormat>MM/dd/yyyy</tt:DateFormat>");
        else if(pstGetOSDOptionsRes->stOSDOptions.stTextOption.penDateFormat[i] == DateFormat_ddMMyyyy)
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:DateFormat>dd/MM/yyyy</tt:DateFormat>");
        else if(pstGetOSDOptionsRes->stOSDOptions.stTextOption.penDateFormat[i] == DateFormat_yyyyMMdd)
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:DateFormat>yyyy/MM/dd</tt:DateFormat>");
        else if(pstGetOSDOptionsRes->stOSDOptions.stTextOption.penDateFormat[i] == DateFormat_yyyy_MM_dd)
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:DateFormat>yyyy-MM-dd</tt:DateFormat>");
    }

    for(i = 0;i < pstGetOSDOptionsRes->stOSDOptions.stTextOption.sizeTimeFormat; i++)
    {
        if(pstGetOSDOptionsRes->stOSDOptions.stTextOption.penTimeFormat[i] == TimeFormat_HHmmss)
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:TimeFormat>HH:mm:ss</tt:TimeFormat>");
        else if(pstGetOSDOptionsRes->stOSDOptions.stTextOption.penTimeFormat[i] == TimeFormat_hhmmsstt)
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:TimeFormat>hh:mm:ss tt</tt:TimeFormat>");
    }
    offset += snprintf(p_buf+offset, mlen-offset, 
                        "<tt:FontColor>"
                        "<tt:Color>"
                        "<tt:ColorspaceRange>"
                        "<tt:X><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:X>"
                        "<tt:Y><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:Y>"
                        "<tt:Z><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:Z>"
                        "<tt:Colorspace>%s</tt:Colorspace>"
                        "</tt:ColorspaceRange>"
                        "</tt:Color>"
                        "</tt:FontColor>",
                        pstGetOSDOptionsRes->stOSDOptions.stTextOption.stFontColor.stColor.stColorspaceRange.stX.min,
                        pstGetOSDOptionsRes->stOSDOptions.stTextOption.stFontColor.stColor.stColorspaceRange.stX.max,
                        pstGetOSDOptionsRes->stOSDOptions.stTextOption.stFontColor.stColor.stColorspaceRange.stY.min,
                        pstGetOSDOptionsRes->stOSDOptions.stTextOption.stFontColor.stColor.stColorspaceRange.stY.max,
                        pstGetOSDOptionsRes->stOSDOptions.stTextOption.stFontColor.stColor.stColorspaceRange.stZ.min,
                        pstGetOSDOptionsRes->stOSDOptions.stTextOption.stFontColor.stColor.stColorspaceRange.stZ.max,
                        pstGetOSDOptionsRes->stOSDOptions.stTextOption.stFontColor.stColor.stColorspaceRange.stColorspace);
    offset += snprintf(p_buf+offset, mlen-offset, "</tt:TextOption>");

    offset += snprintf(p_buf+offset, mlen-offset, "</trt:OSDOptions>");
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:GetOSDOptionsResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}

GK_S32 soap_packet_response_GetOSDs(GK_CHAR * p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 i;
    GONVIF_MEDIA_GetOSDs_S *pstGetOSDOptionsRes = (GONVIF_MEDIA_GetOSDs_S *)argv;
    const char *child_name = (const char *)header;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, "<trt:%sResponse>", child_name);
    for(i = 0;i < pstGetOSDOptionsRes->sizeOSDs; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<trt:OSDs token=\"%s\">", pstGetOSDOptionsRes->stOSDs[i].aszOsdToken);
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:VideoSourceConfigurationToken>%s</tt:VideoSourceConfigurationToken>", pstGetOSDOptionsRes->stOSDs[0].aszConfigurationToken);
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>%s</tt:Type>", pstGetOSDOptionsRes->stOSDs[i].stType ? "Text" : "Text");

        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Position>");
        if(pstGetOSDOptionsRes->stOSDs[i].stPosition.penType == PositionOption_UpperLeft)
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>UpperLeft</tt:Type>");
        else if(pstGetOSDOptionsRes->stOSDs[i].stPosition.penType == PositionOption_LowerLeft)
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>LowerLeft</tt:Type>");
        else if(pstGetOSDOptionsRes->stOSDs[i].stPosition.penType == PositionOption_Custom)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>Custom</tt:Type>");
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:Pos x=\"%f\" y=\"%f\"/>",
                                                      pstGetOSDOptionsRes->stOSDs[i].stPosition.stPos.x,
                                                      pstGetOSDOptionsRes->stOSDs[i].stPosition.stPos.y);
        }
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:Position>");
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:TextString>");
        if(pstGetOSDOptionsRes->stOSDs[i].stTextString.penType == TextType_Plain)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>Plain</tt:Type>");
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:PlainText>%s</tt:PlainText>", 
                pstGetOSDOptionsRes->stOSDs[i].stTextString.stType.stPlain.aszPlainText);
        }
        else if(pstGetOSDOptionsRes->stOSDs[i].stTextString.penType == TextType_Date)
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>Date</tt:Type>");
        else if(pstGetOSDOptionsRes->stOSDs[i].stTextString.penType == TextType_Time)
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>Time</tt:Type>");
        else if(pstGetOSDOptionsRes->stOSDs[i].stTextString.penType == TextType_DateAndTime)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:Type>DateAndTime</tt:Type>");
            if(pstGetOSDOptionsRes->stOSDs[i].stTextString.stType.stDateAndTime.penDateFormat == DateFormat_MMddyyyy)
                offset += snprintf(p_buf+offset, mlen-offset, "<tt:DateFormat>MM/dd/yyyy</tt:DateFormat>");
            else if(pstGetOSDOptionsRes->stOSDs[i].stTextString.stType.stDateAndTime.penDateFormat == DateFormat_ddMMyyyy)
                offset += snprintf(p_buf+offset, mlen-offset, "<tt:DateFormat>dd/MM/yyyy</tt:DateFormat>");
            else if(pstGetOSDOptionsRes->stOSDs[i].stTextString.stType.stDateAndTime.penDateFormat == DateFormat_yyyyMMdd)
                offset += snprintf(p_buf+offset, mlen-offset, "<tt:DateFormat>yyyy/MM/dd</tt:DateFormat>");
            else if(pstGetOSDOptionsRes->stOSDs[i].stTextString.stType.stDateAndTime.penDateFormat == DateFormat_yyyy_MM_dd)
                offset += snprintf(p_buf+offset, mlen-offset, "<tt:DateFormat>yyyy-MM-dd</tt:DateFormat>");
            
            if(pstGetOSDOptionsRes->stOSDs[i].stTextString.stType.stDateAndTime.penTimeFormat== TimeFormat_HHmmss)
                offset += snprintf(p_buf+offset, mlen-offset, "<tt:TimeFormat>HH:mm:ss</tt:TimeFormat>");
            else if(pstGetOSDOptionsRes->stOSDs[i].stTextString.stType.stDateAndTime.penTimeFormat == TimeFormat_hhmmsstt)
                offset += snprintf(p_buf+offset, mlen-offset, "<tt:TimeFormat>hh:mm:ss tt</tt:TimeFormat>");
        }
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:FontSize>32</tt:FontSize>");    
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Extension><tt:ChannelName>%s</tt:ChannelName></tt:Extension>", 
            pstGetOSDOptionsRes->stOSDs[i].stTextString.stExtension.enChannelName? "true" : "false");

        offset += snprintf(p_buf+offset, mlen-offset, "</tt:TextString>");
        offset += snprintf(p_buf+offset, mlen-offset, "</trt:OSDs>");
    }
    offset += snprintf(p_buf+offset, mlen-offset, "</trt:%sResponse>", child_name);
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}

GK_S32 soap_packet_response_SetOSD(GK_CHAR * p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    const char *child_name = (const char *)header;
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<trt:%sResponse/>", child_name);
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}

GK_S32 soap_packet_response_DeleteOSD(GK_CHAR * p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<trt:DeleteOSDResponse/>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}

GK_S32 soap_packet_response_GetPrivacyMaskOptions(GK_CHAR * p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_MEDIA_GetPrivacyMaskOptions_S *pstGetPrivacyMaskOptionsRes = (GONVIF_MEDIA_GetPrivacyMaskOptions_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, 
        "<hikwsd:GetPrivacyMaskOptionsResponse>"
        "<hikwsd:PrivacyMaskOptions>"
        "<hikxsd:MaximumNumberOfAreas>%d</hikxsd:MaximumNumberOfAreas>"
        "<hikxsd:Position>"
        "<hikxsd:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></hikxsd:XRange>"
        "<hikxsd:YRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></hikxsd:YRange>"
        "</hikxsd:Position>"
        "</hikwsd:PrivacyMaskOptions>"
        "</hikwsd:GetPrivacyMaskOptionsResponse>",
        pstGetPrivacyMaskOptionsRes->stPrivacyMaskOptions.MaximumNumberOfAreas,
        pstGetPrivacyMaskOptionsRes->stPrivacyMaskOptions.stPosition.stXRange.min,
        pstGetPrivacyMaskOptionsRes->stPrivacyMaskOptions.stPosition.stXRange.max,
        pstGetPrivacyMaskOptionsRes->stPrivacyMaskOptions.stPosition.stYRange.min,
        pstGetPrivacyMaskOptionsRes->stPrivacyMaskOptions.stPosition.stYRange.max);
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}

GK_S32 soap_packet_response_GetPrivacyMasks(GK_CHAR * p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 i, j;
    GONVIF_MEDIA_GetPrivacyMasks_S *pstGetPrivacyMasksRes = (GONVIF_MEDIA_GetPrivacyMasks_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<hikwsd:GetPrivacyMasksResponse>");
    for(i=0; i<4; i++)
    {
        if(pstGetPrivacyMasksRes->stPrivacyMasks[i].enMaskAreaPoint)
        {
            offset += snprintf(p_buf+offset, mlen-offset, 
                                "<hikwsd:PrivacyMask token=\"%s\">"
                                "<hikxsd:VideoSourceToken>%s</hikxsd:VideoSourceToken>"
                                "<hikxsd:MaskArea>",
                                pstGetPrivacyMasksRes->stPrivacyMasks[i].aszPrivacyMaskToken,
                                pstGetPrivacyMasksRes->stPrivacyMasks[i].aszVideoSourceToken);
            for(j=0;j<4;j++)
            {
                offset += snprintf(p_buf+offset, mlen-offset, "<tt:Point x=\"%f\" y=\"%f\"/>",
                                    pstGetPrivacyMasksRes->stPrivacyMasks[i].stMaskAreaPoint[j].x,
                                    pstGetPrivacyMasksRes->stPrivacyMasks[i].stMaskAreaPoint[j].y);
            }
            offset += snprintf(p_buf+offset, mlen-offset, 
                                "</hikxsd:MaskArea>"
                                "</hikwsd:PrivacyMask>");
        }
    }
    offset += snprintf(p_buf+offset, mlen-offset, "</hikwsd:GetPrivacyMasksResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}

GK_S32 soap_packet_response_DeletePrivacyMask(GK_CHAR * p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<hikwsd:DeletePrivacyMaskResponse/>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}

GK_S32 soap_packet_response_CreatePrivacyMask(GK_CHAR * p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_MEDIA_PrivacyMask_S *pstCreatePrivacyMaskRes = (GONVIF_MEDIA_PrivacyMask_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
    offset += snprintf(p_buf+offset, mlen-offset, "<hikwsd:CreatePrivacyMaskResponse>");
    offset += snprintf(p_buf+offset, mlen-offset, "<hikxsd:PrivacyMaskToken>%s</hikxsd:PrivacyMaskToken>",
                                pstCreatePrivacyMaskRes->aszPrivacyMaskToken);
    offset += snprintf(p_buf+offset, mlen-offset, "</hikwsd:CreatePrivacyMaskResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}

/******************************* media end **************************************/

/********************************* evnet ****************************************/
GK_S32 soap_response_Event_GetServiceCapabilities(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GONVIF_EVENT_GetServiceCapabilities_Res_S *pstEV_GetServiceCapabilitiesRes = (GONVIF_EVENT_GetServiceCapabilities_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
    GONVIF_Soap_S *pstSoap = (GONVIF_Soap_S *)header;
	offset += snprintf(p_buf+offset, mlen-offset, soap_head, pstSoap->stHeader.pszAction);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<tev:GetServiceCapabilitiesResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, g_aszEventCapabilities,
		pstEV_GetServiceCapabilitiesRes->stCapabilities.enWSPausableSubscriptionManagerInterfaceSupport? "true" : "false",
		pstEV_GetServiceCapabilitiesRes->stCapabilities.enWSPullPointSupport? "true" : "false",
		pstEV_GetServiceCapabilitiesRes->stCapabilities.enWSSubscriptionPolicySupport? "true" : "false",
		pstEV_GetServiceCapabilitiesRes->stCapabilities.maxNotificationProducers,
		pstEV_GetServiceCapabilitiesRes->stCapabilities.maxPullPoints,
		pstEV_GetServiceCapabilitiesRes->stCapabilities.enPersistentNotificationStorage? "true" : "false");
								
	offset += snprintf(p_buf+offset, mlen-offset, "</tev:GetServiceCapabilitiesResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

GK_S32 soap_packet_response_GetEventProperties(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 i = 0;
    GONVIF_Soap_S *pstSoap = (GONVIF_Soap_S *)header;
    GONVIF_EVENT_GetEventProperties_Res_S *pstEV_GetEventPropertiesRes = (GONVIF_EVENT_GetEventProperties_Res_S *)argv;
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_head, pstSoap->stHeader.pszAction);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset,"<tev:GetEventPropertiesResponse>");
    for(i = 0; i < pstEV_GetEventPropertiesRes->sizeTopicNamespaceLocation; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tev:TopicNamespaceLocation>%s</tev:TopicNamespaceLocation>",
            pstEV_GetEventPropertiesRes->aszTopicNamespaceLocation[i]);
    }
    offset += snprintf(p_buf+offset, mlen-offset,
        "<wsnt:FixedTopicSet>%s</wsnt:FixedTopicSet>", 
        pstEV_GetEventPropertiesRes->enFixedTopicSet? "true" : "false");
        
    #if 1
    offset += snprintf(p_buf+offset, mlen-offset, "<wstop:TopicSet>");
    for(i = 0; i < pstEV_GetEventPropertiesRes->stTopicSet.size; i++)
    {
        if(pstEV_GetEventPropertiesRes->stTopicSet.any[i][0] != '\0')
        {
            offset += snprintf(p_buf+offset, mlen-offset, "%s",
                pstEV_GetEventPropertiesRes->stTopicSet.any[i]);
        }
    }
    offset += snprintf(p_buf+offset, mlen-offset, "</wstop:TopicSet>");
    #endif
    for(i = 0; i < pstEV_GetEventPropertiesRes->sizeTopicExpressionDialect; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset,
            "<wsnt:TopicExpressionDialect>%s</wsnt:TopicExpressionDialect>",
            pstEV_GetEventPropertiesRes->aszTopicExpressionDialect[i]);
    }
    for(i = 0; i < pstEV_GetEventPropertiesRes->sizeMessageContentFilterDialect; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tev:MessageContentFilterDialect>%s</tev:MessageContentFilterDialect>",
            pstEV_GetEventPropertiesRes->aszMessageContentFilterDialect[i]);
    }
    for(i = 0; i < pstEV_GetEventPropertiesRes->sizeMessageContentSchemaLocation; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tev:MessageContentSchemaLocation>%s</tev:MessageContentSchemaLocation>",
            pstEV_GetEventPropertiesRes->aszMessageContentSchemaLocation[i]);
    }
	offset += snprintf(p_buf+offset, mlen-offset,"</tev:GetEventPropertiesResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
	return offset;
}

GK_S32 soap_packet_response_Subscribe(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    struct  tm  *subtime;
    GONVIF_Soap_S *pstSoap = (GONVIF_Soap_S *)header;
    GONVIF_EVENT_Subscribe_Res_S *pstEV_SubscribeRes = (GONVIF_EVENT_Subscribe_Res_S *)argv;
    
    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_head, pstSoap->stHeader.pszAction);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);


	GK_CHAR cur_time[100], term_time[100];
    memset(cur_time, 0, sizeof(cur_time));
    memset(term_time, 0, sizeof(term_time));
    subtime = localtime(&pstEV_SubscribeRes->currentTime);
	sprintf(cur_time, "%04d-%02d-%02dT%02d:%02d:%02dZ",
		subtime->tm_year+1900, subtime->tm_mon + 1, subtime->tm_mday,
		subtime->tm_hour, subtime->tm_min, subtime->tm_sec);

    subtime = localtime(&pstEV_SubscribeRes->terminationTime);
	sprintf(term_time, "%04d-%02d-%02dT%02d:%02d:%02dZ",
		subtime->tm_year+1900, subtime->tm_mon + 1, subtime->tm_mday,
		subtime->tm_hour, subtime->tm_min, subtime->tm_sec);

    offset += snprintf(p_buf+offset, mlen-offset,
        "<wsnt:SubscribeResponse>"
            "<wsnt:SubscriptionReference>"
                "<wsa:Address>%s</wsa:Address>"
            "</wsnt:SubscriptionReference>"
            "<wsnt:CurrentTime>%s</wsnt:CurrentTime>"
            "<wsnt:TerminationTime>%s</wsnt:TerminationTime>"
        "</wsnt:SubscribeResponse>",
        pstEV_SubscribeRes->stSubscriptionReference.aszAddress, cur_time, term_time);

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}


GK_S32 soap_packet_response_Unsubscribe(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
    GONVIF_Soap_S *pstSoap = (GONVIF_Soap_S *)header;
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_head, pstSoap->stHeader.pszAction);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<wsnt:UnsubscribeResponse>"
	                                              "</wsnt:UnsubscribeResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_Renew(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    struct  tm  *subtime;
    GONVIF_Soap_S *pstSoap = (GONVIF_Soap_S *)header;
    GONVIF_EVENT_Renew_Res_S *pstEV_RenewRes = (GONVIF_EVENT_Renew_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_head, pstSoap->stHeader.pszAction);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	GK_CHAR cur_time[100], term_time[100];
    memset(cur_time, 0, sizeof(cur_time));
    memset(term_time, 0, sizeof(term_time));
    subtime = localtime(&pstEV_RenewRes->currentTime);
	sprintf(cur_time, "%04d-%02d-%02dT%02d:%02d:%02dZ",
		subtime->tm_year+1900, subtime->tm_mon + 1, subtime->tm_mday,
		subtime->tm_hour, subtime->tm_min, subtime->tm_sec);

    subtime = localtime(&pstEV_RenewRes->terminationTime);
	sprintf(term_time, "%04d-%02d-%02dT%02d:%02d:%02dZ",
		subtime->tm_year+1900, subtime->tm_mon + 1, subtime->tm_mday,
		subtime->tm_hour, subtime->tm_min, subtime->tm_sec);

    offset += snprintf(p_buf+offset, mlen-offset,
        "<wsnt:RenewResponse>"
            "<wsnt:TerminationTime>%s</wsnt:TerminationTime>"
            "<wsnt:CurrentTime>%s</wsnt:CurrentTime>"
        "</wsnt:RenewResponse>",
        term_time,cur_time);

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_CreatePullPointSubscription(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_Soap_S *pstSoap = (GONVIF_Soap_S *)header;
    GONVIF_EVENT_CreatePullPointSubscription_Res_S *pstEV_CreatePullPointSubscriptionRes = (GONVIF_EVENT_CreatePullPointSubscription_Res_S *)argv;

    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_head, pstSoap->stHeader.pszAction);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset,"<tev:CreatePullPointSubscriptionResponse>");

    offset += snprintf(p_buf+offset, mlen-offset,"<tev:SubscriptionReference>"
                                                    "<wsa:Address>%s</wsa:Address>"
                                                 "</tev:SubscriptionReference>",
                                                 pstEV_CreatePullPointSubscriptionRes->stSubscriptionReference.aszAddress);

    struct tm *subtime;
    subtime = localtime(&pstEV_CreatePullPointSubscriptionRes->aszCurrentTime);
    offset += snprintf(p_buf+offset, mlen-offset,"<wsnt:CurrentTime>%04d-%02d-%02dT%02d:%02d:%02dZ</wsnt:CurrentTime>",
    		                subtime->tm_year+1900, subtime->tm_mon + 1, subtime->tm_mday,
		                    subtime->tm_hour, subtime->tm_min, subtime->tm_sec);

    subtime = localtime(&pstEV_CreatePullPointSubscriptionRes->aszTerminationTime);

    offset += snprintf(p_buf+offset, mlen-offset,"<wsnt:TerminationTime>%04d-%02d-%02dT%02d:%02d:%02dZ</wsnt:TerminationTime>",
                    		subtime->tm_year+1900, subtime->tm_mon + 1, subtime->tm_mday,
                    		subtime->tm_hour, subtime->tm_min, subtime->tm_sec);


    offset += snprintf(p_buf+offset, mlen-offset, "</tev:CreatePullPointSubscriptionResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_PullMessages(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 index = 0;
    
    GONVIF_Soap_S *pstSoap = (GONVIF_Soap_S *)header;
    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_head, pstSoap->stHeader.pszAction);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset,"<tev:PullMessagesResponse>");
    
    offset += snprintf(p_buf+offset, mlen-offset,
        "<tev:CurrentTime>%s</tev:CurrentTime>"
        "<tev:TerminationTime>%s</tev:TerminationTime>",
        g_stEventPullMessageRes.aszCurrentTime,
        g_stEventPullMessageRes.aszTerminationTime);

    for(index = 0; index < g_stEventPullMessageRes.sizeNotificationMessage; index++)
    {
        offset += snprintf(p_buf+offset, mlen-offset,
            "<wsnt:NotificationMessage>"
                "%s"
            "</wsnt:NotificationMessage>",
            g_stEventPullMessageRes.pstNotificationMessage[index].aszEvent);
    }
    offset += snprintf(p_buf+offset, mlen-offset,"</tev:PullMessagesResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_SetSynchronizationPoint(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	int offset = snprintf(p_buf, mlen, xml_hdr);
    GONVIF_Soap_S *pstSoap = (GONVIF_Soap_S *)header;
    
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_head, pstSoap->stHeader.pszAction);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset,
		"<tev:SetSynchronizationPointResponse>"
		"</tev:SetSynchronizationPointResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

/******************************* evnet end **************************************/

/******************************** Image ****************************************/
GK_S32 soap_response_Imaging_GetServiceCapabilities(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GONVIF_IMAGING_GetServiceCapabilities_Res_S *pstIMG_GetServiceCapabilitiesRes = (GONVIF_IMAGING_GetServiceCapabilities_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<timg:GetServiceCapabilitiesResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, g_aszImageCapabilities,
		pstIMG_GetServiceCapabilitiesRes->stCapabilities.enImageStabilization? "true" : "false");
	
	offset += snprintf(p_buf+offset, mlen-offset, "</timg:GetServiceCapabilitiesResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

GK_S32 soap_packet_response_GetImagingSettings(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_IMAGE_GetSettings_S *pstGetImagingSettingsRes = (GONVIF_IMAGE_GetSettings_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, "<timg:GetImagingSettingsResponse>");
    offset += snprintf(p_buf+offset, mlen-offset, "<timg:ImagingSettings>");

    offset += snprintf(p_buf+offset, mlen-offset, //"<tt:BacklightCompensation>"
                                                    //"<tt:Mode>%s</tt:Mode>"
                                                    //"<tt:Level>%f</tt:Level>"
                                                  //"</tt:BacklightCompensation>"
                                                  "<tt:Brightness>%f</tt:Brightness>"
                                                  "<tt:ColorSaturation>%f</tt:ColorSaturation>"
                                                  "<tt:Contrast>%f</tt:Contrast>",
                                                  //"<tt:Exposure>"
                                                    //"<tt:Mode>%s</tt:Mode>"
                                                    //"<tt:MinExposureTime>%f</tt:MinExposureTime>"
                                                    //"<tt:MaxExposureTime>%f</tt:MaxExposureTime>"
                                                    //"<tt:MinGain>%f</tt:MinGain>"
                                                    //"<tt:MaxGain>%f</tt:MaxGain>"
                                                    //"<tt:ExposureTime>%f</tt:ExposureTime>"
                                                    //"<tt:Gain>%f</tt:Gain>"
                                                  //"</tt:Exposure>"
                                                  //"<tt:Focus>"
                                                    //"<tt:AutoFocusMode>%s</tt:AutoFocusMode>"
                                                    //"<tt:DefaultSpeed>%f</tt:DefaultSpeed>"
                                                    //"<tt:NearLimit>%f</tt:NearLimit>"
                                                    //"<tt:FarLimit>%f</tt:FarLimit>"
                                                  //"</tt:Focus>",
                                                    #if 0
                                                    pstGetImagingSettingsRes->stImageSettings.stBacklightCompensation.enMode ? "ON" : "OFF",
                                                    pstGetImagingSettingsRes->stImageSettings.stBacklightCompensation.level,
                                                    #endif
                                                    pstGetImagingSettingsRes->stImageSettings.brightness,
                                                    pstGetImagingSettingsRes->stImageSettings.colorSaturation,
                                                    pstGetImagingSettingsRes->stImageSettings.contrast);
                                                    #if 0
                                                    pstGetImagingSettingsRes->stImageSettings.stExposure.enMode ? "MANUAL" : "AUTO",
                                                    pstGetImagingSettingsRes->stImageSettings.stExposure.minExposureTime,
                                                    pstGetImagingSettingsRes->stImageSettings.stExposure.maxExposureTime,
                                                    pstGetImagingSettingsRes->stImageSettings.stExposure.minGain,
                                                    pstGetImagingSettingsRes->stImageSettings.stExposure.maxGain,
                                                    pstGetImagingSettingsRes->stImageSettings.stExposure.exposureTime,
                                                    pstGetImagingSettingsRes->stImageSettings.stExposure.gain,
                                                    pstGetImagingSettingsRes->stImageSettings.stFocus.enAutoFocusMode ? "MANUAL" : "AUTO",
                                                    pstGetImagingSettingsRes->stImageSettings.stFocus.defaultSpeed,
                                                    pstGetImagingSettingsRes->stImageSettings.stFocus.nearLimit,
                                                    pstGetImagingSettingsRes->stImageSettings.stFocus.farLimit
                                                    #endif
    if(pstGetImagingSettingsRes->stImageSettings.enIrCutFilter == IrCutFilterMode_ON)
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:IrCutFilter>ON</tt:IrCutFilter>");
    else if(pstGetImagingSettingsRes->stImageSettings.enIrCutFilter == IrCutFilterMode_OFF)
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:IrCutFilter>OFF</tt:IrCutFilter>");
    else if(pstGetImagingSettingsRes->stImageSettings.enIrCutFilter == IrCutFilterMode_AUTO)
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:IrCutFilter>AUTO</tt:IrCutFilter>");

    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Sharpness>%f</tt:Sharpness>",
                                                  //"<tt:WideDynamicRange>"
                                                    //"<tt:Mode>%s</tt:Mode>"
                                                    //"<tt:Level>%f</tt:Level>"
                                                  //"</tt:WideDynamicRange>"
                                                  //"<tt:WhiteBalance>"
                                                    //"<tt:Mode>%s</tt:Mode>"
                                                    //"<tt:CrGain>%f</tt:CrGain>"
                                                    //"<tt:CbGain>%f</tt:CbGain>"
                                                  //"</tt:WhiteBalance>",
                                                  pstGetImagingSettingsRes->stImageSettings.sharpness);
                                                  //pstGetImagingSettingsRes->stImageSettings.stWideDynamicRange.enMode ? "ON" : "OFF",
                                                  //pstGetImagingSettingsRes->stImageSettings.stWideDynamicRange.level,
                                                  //pstGetImagingSettingsRes->stImageSettings.stWhiteBalance.enMode ? "MANUAL" : "AUTO" );
                                                  //pstGetImagingSettingsRes->stImageSettings.stWhiteBalance.crGain,
                                                  //pstGetImagingSettingsRes->stImageSettings.stWhiteBalance.cbGain
                                                 

    offset += snprintf(p_buf+offset, mlen-offset, "</timg:ImagingSettings>");
    offset += snprintf(p_buf+offset, mlen-offset, "</timg:GetImagingSettingsResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}

GK_S32 soap_packet_response_SetImagingSettings(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, "<timg:SetImagingSettingsResponse>");
    offset += snprintf(p_buf+offset, mlen-offset, "</timg:SetImagingSettingsResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}
GK_S32 soap_packet_response_GetOptions(GK_CHAR * p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 i;
    GONVIF_IMAGE_GetOptions_S *pstGetImageOptionsRes = (GONVIF_IMAGE_GetOptions_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, "<timg:GetOptionsResponse>");
    offset += snprintf(p_buf+offset, mlen-offset, "<timg:ImagingOptions>");

    offset += snprintf(p_buf+offset, mlen-offset, "<tt:BacklightCompensation>"
                                                  "<tt:Mode>%s</tt:Mode><tt:Mode>%s</tt:Mode>"
                                                  "<tt:Level><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:Level>"
                                                  "</tt:BacklightCompensation>"

                                                  "<tt:Brightness><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:Brightness>"
                                                  "<tt:ColorSaturation><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:ColorSaturation>"
                                                  "<tt:Contrast><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:Contrast>"
                                                  "<tt:Exposure>"
                                                  "<tt:Mode>%s</tt:Mode>""<tt:Mode>%s</tt:Mode>"
                                                  "<tt:MinExposureTime><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:MinExposureTime>"
                                                  "<tt:MaxExposureTime><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:MaxExposureTime>"
                                                  "<tt:MinGain><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:MinGain>"
                                                  "<tt:MaxGain><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:MaxGain>"
                                                  "<tt:ExposureTime><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:ExposureTime>"
                                                  "<tt:Gain><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:Gain>"
                                                  "</tt:Exposure>"
                                                  "<tt:Focus>"
                                                  "<tt:AutoFocusModes>%s</tt:AutoFocusModes>"
                                                  "<tt:AutoFocusModes>%s</tt:AutoFocusModes>"
                                                  "<tt:DefaultSpeed><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:DefaultSpeed>"
                                                  "<tt:NearLimit><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:NearLimit>"
                                                  "<tt:FarLimit><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:FarLimit>"
                                                  "</tt:Focus>",
                                                  pstGetImageOptionsRes->stGetImageOptions.stBacklightCompensation.penMode[0] ? "ON" : "OFF",
                                                  pstGetImageOptionsRes->stGetImageOptions.stBacklightCompensation.penMode[1] ? "ON" : "OFF",
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stBacklightCompensation.stLevel.min,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stBacklightCompensation.stLevel.max,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stBrightness.min,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stBrightness.max,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stColorSaturation.min,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stColorSaturation.max,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stContrast.min,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stContrast.max,
                                                  pstGetImageOptionsRes->stGetImageOptions.stExposure.penMode[0] ? "AUTO" : "MANUAL",
                                                  pstGetImageOptionsRes->stGetImageOptions.stExposure.penMode[1] ? "AUTO" : "MANUAL",
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stExposure.stMinExposureTime.min,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stExposure.stMinExposureTime.max,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stExposure.stMaxExposureTime.min,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stExposure.stMaxExposureTime.max,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stExposure.stMinGain.min,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stExposure.stMinGain.max,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stExposure.stMaxGain.min,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stExposure.stMaxGain.max,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stExposure.stExposureTime.min,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stExposure.stExposureTime.max,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stExposure.stGain.min,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stExposure.stGain.max,
                                                  pstGetImageOptionsRes->stGetImageOptions.stFocus.penAutoFocusModes[0] ? "AUTO" : "MANUAL",
                                                  pstGetImageOptionsRes->stGetImageOptions.stFocus.penAutoFocusModes[1] ? "AUTO" : "MANUAL",
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stFocus.stDefaultSpeed.min,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stFocus.stDefaultSpeed.max,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stFocus.stNearLimit.min,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stFocus.stNearLimit.max,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stFocus.stFarLimit.min,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stFocus.stFarLimit.max);
    for(i = 0;i < pstGetImageOptionsRes->stGetImageOptions.sizeIrCutFilterModes; i++)
    {
        if(pstGetImageOptionsRes->stGetImageOptions.penIrCutFilterModes[i] == IrCutFilterMode_ON)
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:IrCutFilterModes>ON</tt:IrCutFilterModes>");
        else if(pstGetImageOptionsRes->stGetImageOptions.penIrCutFilterModes[i] == IrCutFilterMode_OFF)
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:IrCutFilterModes>OFF</tt:IrCutFilterModes>");
        else if(pstGetImageOptionsRes->stGetImageOptions.penIrCutFilterModes[i] == IrCutFilterMode_AUTO)
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:IrCutFilterModes>AUTO</tt:IrCutFilterModes>");
    }
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Sharpness><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:Sharpness>"

                                                  "<tt:WideDynamicRange>"
                                                  "<tt:Mode>%s</tt:Mode><tt:Mode>%s</tt:Mode>"
                                                  "<tt:Level><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:Level>"
                                                  "</tt:WideDynamicRange>"

                                                  "<tt:WhiteBalance>"
                                                  "<tt:Mode>%s</tt:Mode><tt:Mode>%s</tt:Mode>"
                                                  "<tt:YrGain><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:YrGain>"
                                                  "<tt:YbGain><tt:Min>%d</tt:Min><tt:Max>%d</tt:Max></tt:YbGain>"
                                                  "</tt:WhiteBalance>",
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stSharpness.min,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stSharpness.max,
                                                  pstGetImageOptionsRes->stGetImageOptions.stWideDynamicRange.penMode[0]? "ON" : "OFF",
                                                  pstGetImageOptionsRes->stGetImageOptions.stWideDynamicRange.penMode[1]? "ON" : "OFF",
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stWideDynamicRange.stLevel.min,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stWideDynamicRange.stLevel.max,
                                                  pstGetImageOptionsRes->stGetImageOptions.stWhiteBalance.penMode[0] ? "AUTO" : "MANUAL",
                                                  pstGetImageOptionsRes->stGetImageOptions.stWhiteBalance.penMode[1] ? "AUTO" : "MANUAL",
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stWhiteBalance.stYrGain.min,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stWhiteBalance.stYrGain.max,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stWhiteBalance.stYbGain.min,
                                                  (int)pstGetImageOptionsRes->stGetImageOptions.stWhiteBalance.stYbGain.max);

    offset += snprintf(p_buf+offset, mlen-offset, "</timg:ImagingOptions>");
    offset += snprintf(p_buf+offset, mlen-offset, "</timg:GetOptionsResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}

GK_S32 soap_packet_response_GetMoveOptions(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_IMAGE_GetMoveOptions_S *pstGetMoveOptionsRes = (GONVIF_IMAGE_GetMoveOptions_S *)argv;
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, "<timg:GetMoveOptionsResponse>"
                                                  "<timg:MoveOptions>");

    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Absolute>"
                                                    "<tt:Position><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:Position>"
                                                    "<tt:Speed><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:Speed>"
                                                  "</tt:Absolute>"
                                                  "<tt:Relative>"
                                                    "<tt:Distance><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:Distance>"
                                                    "<tt:Speed><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:Speed>"
                                                  "</tt:Relative>"
                                                  "<tt:Continuous>"
                                                    "<tt:Speed><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:Speed>"
                                                  "</tt:Continuous>",
                                                  pstGetMoveOptionsRes->stMoveOptions.stAbsolute.stPosition.min,
                                                  pstGetMoveOptionsRes->stMoveOptions.stAbsolute.stPosition.max,
                                                  pstGetMoveOptionsRes->stMoveOptions.stAbsolute.stSpeed.min,
                                                  pstGetMoveOptionsRes->stMoveOptions.stAbsolute.stSpeed.max,
                                                  pstGetMoveOptionsRes->stMoveOptions.stRelative.stDistance.min,
                                                  pstGetMoveOptionsRes->stMoveOptions.stRelative.stDistance.max,
                                                  pstGetMoveOptionsRes->stMoveOptions.stRelative.stSpeed.min,
                                                  pstGetMoveOptionsRes->stMoveOptions.stRelative.stSpeed.max,
                                                  pstGetMoveOptionsRes->stMoveOptions.stContinuous.stSpeed.min,
                                                  pstGetMoveOptionsRes->stMoveOptions.stContinuous.stSpeed.max);

    offset += snprintf(p_buf+offset, mlen-offset, "</timg:MoveOptions>"
                                                  "</timg:GetMoveOptionsResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}

GK_S32 soap_packet_response_Move(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, "<timg:MoveResponse>"
                                                  "</timg:MoveResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}

GK_S32 soap_response_Imaging_GetStatus(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_IMAGE_GetStatus_S *pstImageGetStatusRes = (GONVIF_IMAGE_GetStatus_S *)argv;
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, "<timg:GetStatusResponse>"
                                                  "<timg:Status>"
                                                  "<tt:FocusStatus20>");


    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Position>%f</tt:Position>", pstImageGetStatusRes->stImageStatus.stFocusStatus.position);

    if(pstImageGetStatusRes->stImageStatus.stFocusStatus.enMoveStatus == MoveStatus_IDLE)
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:MoveStatus>IDLE</tt:MoveStatus>");
    else if(pstImageGetStatusRes->stImageStatus.stFocusStatus.enMoveStatus == MoveStatus_MOVING)
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:MoveStatus>MOVING</tt:MoveStatus>");
    else if(pstImageGetStatusRes->stImageStatus.stFocusStatus.enMoveStatus == MoveStatus_UNKNOWN)
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:MoveStatus>MoveStauts</tt:MoveStatus>");

    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Error>%s</tt:Error>", pstImageGetStatusRes->stImageStatus.stFocusStatus.aszError);

    offset += snprintf(p_buf+offset, mlen-offset, "</tt:FocusStatus20>"
                                                  "</timg:Status>"
                                                  "</timg:GetStatusResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}
GK_S32 soap_response_Imaging_Stop(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, "<timg:StopResponse></timg:StopResponse>");
   
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}

/************************************ ptz ***************************************/

GK_S32 soap_response_PTZ_GetServiceCapabilities(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GONVIF_PTZ_GetServiceCapabilities_Res_S *pstPTZ_GetServiceCapabilitiesRes = (GONVIF_PTZ_GetServiceCapabilities_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GetServiceCapabilitiesResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, g_aszPtzCapabilities,
		pstPTZ_GetServiceCapabilitiesRes->stCapabilities.enReverse ? "true" : "false",
		pstPTZ_GetServiceCapabilitiesRes->stCapabilities.enEFlip ? "true" : "false",
		pstPTZ_GetServiceCapabilitiesRes->stCapabilities.enGetCompatibleConfigurations ? "true" : "false");
	
	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:GetServiceCapabilitiesResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

GK_S32 soap_packet_response_GetNodes(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 i = 0, j = 0;
    GONVIF_PTZ_GetNodes_Res_S *pstPTZ_GetNodesRes = (GONVIF_PTZ_GetNodes_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GetNodesResponse>");
    for(i = 0; i < pstPTZ_GetNodesRes->sizePTZNode; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tptz:PTZNode token=\"%s\" FixedHomePosition=\"%s\">"
                "<tt:Name>%s</tt:Name>",
            pstPTZ_GetNodesRes->stPTZNode[i].token,
            pstPTZ_GetNodesRes->stPTZNode[i].enFixedHomePosition ? "true" : "false",
            pstPTZ_GetNodesRes->stPTZNode[i].name);
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tt:SupportedPTZSpaces>");
        //absolute
        for(j = 0; j < pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.sizeAbsolutePanTiltPositionSpace;j++)
        {            
            offset += snprintf(p_buf+offset, mlen-offset,
                "<tt:AbsolutePanTiltPositionSpace>"
                    "<tt:URI>%s</tt:URI>"
                    "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
                    "<tt:YRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:YRange>"
                "</tt:AbsolutePanTiltPositionSpace>",
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stAbsolutePanTiltPositionSpace[j].aszURI,
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stAbsolutePanTiltPositionSpace[j].stXRange.min,
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stAbsolutePanTiltPositionSpace[j].stXRange.max,
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stAbsolutePanTiltPositionSpace[j].stYRange.min ,
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stAbsolutePanTiltPositionSpace[j].stYRange.max);
        }
        for(j = 0; j < pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.sizeAbsoluteZoomPositionSpace;j++)
        {            
            offset += snprintf(p_buf+offset, mlen-offset,
                "<tt:AbsoluteZoomPositionSpace>"
                    "<tt:URI>%s</tt:URI>"
                    "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
                "</tt:AbsoluteZoomPositionSpace>",
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stAbsoluteZoomPositionSpace[j].aszURI,
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stAbsoluteZoomPositionSpace[j].stXRange.min,
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stAbsoluteZoomPositionSpace[j].stXRange.max);
        }
        //relative
        for(j = 0; j < pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.sizeRelativePanTiltTranslationSpace;j++)
        {            
            offset += snprintf(p_buf+offset, mlen-offset,
                "<tt:RelativePanTiltTranslationSpace>"
                    "<tt:URI>%s</tt:URI>"
                    "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
                    "<tt:YRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:YRange>"
                "</tt:RelativePanTiltTranslationSpace>",
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stRelativePanTiltTranslationSpace[j].aszURI,
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stRelativePanTiltTranslationSpace[j].stXRange.min,
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stRelativePanTiltTranslationSpace[j].stXRange.max,
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stRelativePanTiltTranslationSpace[j].stYRange.min ,
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stRelativePanTiltTranslationSpace[j].stYRange.max);
        }
        for(j = 0; j < pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.sizeRelativeZoomTranslationSpace;j++)
        {            
            offset += snprintf(p_buf+offset, mlen-offset,
                "<tt:RelativeZoomTranslationSpace>"
                    "<tt:URI>%s</tt:URI>"
                    "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
                "</tt:RelativeZoomTranslationSpace>",
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stRelativeZoomTranslationSpace[j].aszURI,
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stRelativeZoomTranslationSpace[j].stXRange.min,
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stRelativeZoomTranslationSpace[j].stXRange.max);
        }
        //continuous
        for(j = 0; j < pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.sizeContinuousPanTiltVelocitySpace;j++)
        {            
            offset += snprintf(p_buf+offset, mlen-offset,
                "<tt:ContinuousPanTiltVelocitySpace>"
                    "<tt:URI>%s</tt:URI>"
                    "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
                    "<tt:YRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:YRange>"
                "</tt:ContinuousPanTiltVelocitySpace>",
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stContinuousPanTiltVelocitySpace[j].aszURI,
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stContinuousPanTiltVelocitySpace[j].stXRange.min,
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stContinuousPanTiltVelocitySpace[j].stXRange.max,
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stContinuousPanTiltVelocitySpace[j].stYRange.min ,
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stContinuousPanTiltVelocitySpace[j].stYRange.max);
        }
        for(j = 0; j < pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.sizeContinuousZoomVelocitySpace;j++)
        {            
            offset += snprintf(p_buf+offset, mlen-offset,
                "<tt:ContinuousZoomVelocitySpace>"
                    "<tt:URI>%s</tt:URI>"
                    "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
                "</tt:ContinuousZoomVelocitySpace>",
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stContinuousZoomVelocitySpace[j].aszURI,
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stContinuousZoomVelocitySpace[j].stXRange.min,
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stContinuousZoomVelocitySpace[j].stXRange.max);
        }
        //Speed
        for(j = 0; j < pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.sizePanTiltSpeedSpace;j++)
        {            
            offset += snprintf(p_buf+offset, mlen-offset,
                "<tt:PanTiltSpeedSpace>"
                    "<tt:URI>%s</tt:URI>"
                    "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
                "</tt:PanTiltSpeedSpace>",
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stPanTiltSpeedSpace[j].aszURI,
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stPanTiltSpeedSpace[j].stXRange.min,
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stPanTiltSpeedSpace[j].stXRange.max);
        }
        for(j = 0; j < pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.sizeContinuousZoomVelocitySpace;j++)
        {            
            offset += snprintf(p_buf+offset, mlen-offset,
                "<tt:ZoomSpeedSpace>"
                    "<tt:URI>%s</tt:URI>"
                    "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
                "</tt:ZoomSpeedSpace>",
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stZoomSpeedSpace[j].aszURI,
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stZoomSpeedSpace[j].stXRange.min,
                pstPTZ_GetNodesRes->stPTZNode[i].stSupportedPTZSpaces.stZoomSpeedSpace[j].stXRange.max);
        }
        offset += snprintf(p_buf+offset, mlen-offset,
            "</tt:SupportedPTZSpaces>");
        //preset
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tt:MaximumNumberOfPresets>%d</tt:MaximumNumberOfPresets>"
            "<tt:HomeSupported>%s</tt:HomeSupported>",
            pstPTZ_GetNodesRes->stPTZNode[i].maximumNumberOfPresets,
            pstPTZ_GetNodesRes->stPTZNode[i].enHomeSupported ? "true" : "false");
                                                                                                         
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tt:Extension>"
                "<tt:SupportedPresetTour>"
                "<tt:MaximumNumberOfPresetTours>%d</tt:MaximumNumberOfPresetTours>",
            pstPTZ_GetNodesRes->stPTZNode[i].stExtension.stSupportedPresetTour.maximumNumberOfPresetTours);
        for(j = 0; j < pstPTZ_GetNodesRes->stPTZNode[i].stExtension.stSupportedPresetTour.sizePTZPresetTourOperation; j++)
        {
            if(pstPTZ_GetNodesRes->stPTZNode[i].stExtension.stSupportedPresetTour.enPTZPresetTourOperation[j] == PTZPresetTourOperation_Start)
                offset += snprintf(p_buf+offset, mlen-offset, "<tt:PTZPresetTourOperation>Start</tt:PTZPresetTourOperation>");
            else if(pstPTZ_GetNodesRes->stPTZNode[i].stExtension.stSupportedPresetTour.enPTZPresetTourOperation[j] == PTZPresetTourOperation_Stop)
                offset += snprintf(p_buf+offset, mlen-offset, "<tt:PTZPresetTourOperation>Stop</tt:PTZPresetTourOperation>");
            else if(pstPTZ_GetNodesRes->stPTZNode[i].stExtension.stSupportedPresetTour.enPTZPresetTourOperation[j] == PTZPresetTourOperation_Pause)
                offset += snprintf(p_buf+offset, mlen-offset, "<tt:PTZPresetTourOperation>Pause</tt:PTZPresetTourOperation>");
        }
        offset += snprintf(p_buf+offset, mlen-offset, 
                "</tt:SupportedPresetTour>"
            "</tt:Extension>");
        
        offset += snprintf(p_buf+offset, mlen-offset, "</tptz:PTZNode>");
    }

    offset += snprintf(p_buf+offset, mlen-offset, "</tptz:GetNodesResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}

GK_S32 soap_packet_response_GetNode(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_PTZ_GetNode_Res_S *pstPTZ_GetNodeRes = (GONVIF_PTZ_GetNode_Res_S *)argv;
    GK_S32 j = 0;
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GetNodeResponse>");
    
    offset += snprintf(p_buf+offset, mlen-offset,
        "<tptz:PTZNode token=\"%s\" FixedHomePosition=\"%s\">"
            "<tt:Name>%s</tt:Name>",
        pstPTZ_GetNodeRes->stPTZNode.token,
        pstPTZ_GetNodeRes->stPTZNode.enFixedHomePosition ? "true" : "false",
        pstPTZ_GetNodeRes->stPTZNode.name);
    offset += snprintf(p_buf+offset, mlen-offset,
        "<tt:SupportedPTZSpaces>");
    //absolute
    for(j = 0; j < pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.sizeAbsolutePanTiltPositionSpace;j++)
    {            
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tt:AbsolutePanTiltPositionSpace>"
                "<tt:URI>%s</tt:URI>"
                "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
                "<tt:YRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:YRange>"
            "</tt:AbsolutePanTiltPositionSpace>",
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stAbsolutePanTiltPositionSpace[j].aszURI,
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stAbsolutePanTiltPositionSpace[j].stXRange.min,
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stAbsolutePanTiltPositionSpace[j].stXRange.max,
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stAbsolutePanTiltPositionSpace[j].stYRange.min ,
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stAbsolutePanTiltPositionSpace[j].stYRange.max);
    }
    for(j = 0; j < pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.sizeAbsoluteZoomPositionSpace;j++)
    {            
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tt:AbsoluteZoomPositionSpace>"
                "<tt:URI>%s</tt:URI>"
                "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
            "</tt:AbsoluteZoomPositionSpace>",
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stAbsoluteZoomPositionSpace[j].aszURI,
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stAbsoluteZoomPositionSpace[j].stXRange.min,
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stAbsoluteZoomPositionSpace[j].stXRange.max);
    }
    //relative
    for(j = 0; j < pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.sizeRelativePanTiltTranslationSpace;j++)
    {            
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tt:RelativePanTiltTranslationSpace>"
                "<tt:URI>%s</tt:URI>"
                "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
                "<tt:YRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:YRange>"
            "</tt:RelativePanTiltTranslationSpace>",
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stRelativePanTiltTranslationSpace[j].aszURI,
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stRelativePanTiltTranslationSpace[j].stXRange.min,
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stRelativePanTiltTranslationSpace[j].stXRange.max,
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stRelativePanTiltTranslationSpace[j].stYRange.min ,
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stRelativePanTiltTranslationSpace[j].stYRange.max);
    }
    for(j = 0; j < pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.sizeRelativeZoomTranslationSpace;j++)
    {            
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tt:RelativeZoomTranslationSpace>"
                "<tt:URI>%s</tt:URI>"
                "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
            "</tt:RelativeZoomTranslationSpace>",
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stRelativeZoomTranslationSpace[j].aszURI,
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stRelativeZoomTranslationSpace[j].stXRange.min,
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stRelativeZoomTranslationSpace[j].stXRange.max);
    }
    //continuous
    for(j = 0; j < pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.sizeContinuousPanTiltVelocitySpace;j++)
    {            
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tt:ContinuousPanTiltVelocitySpace>"
                "<tt:URI>%s</tt:URI>"
                "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
                "<tt:YRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:YRange>"
            "</tt:ContinuousPanTiltVelocitySpace>",
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stContinuousPanTiltVelocitySpace[j].aszURI,
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stContinuousPanTiltVelocitySpace[j].stXRange.min,
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stContinuousPanTiltVelocitySpace[j].stXRange.max,
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stContinuousPanTiltVelocitySpace[j].stYRange.min ,
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stContinuousPanTiltVelocitySpace[j].stYRange.max);
    }
    for(j = 0; j < pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.sizeContinuousZoomVelocitySpace;j++)
    {            
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tt:ContinuousZoomVelocitySpace>"
                "<tt:URI>%s</tt:URI>"
                "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
            "</tt:ContinuousZoomVelocitySpace>",
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stContinuousZoomVelocitySpace[j].aszURI,
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stContinuousZoomVelocitySpace[j].stXRange.min,
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stContinuousZoomVelocitySpace[j].stXRange.max);
    }
    //Speed
    for(j = 0; j < pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.sizePanTiltSpeedSpace;j++)
    {            
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tt:PanTiltSpeedSpace>"
                "<tt:URI>%s</tt:URI>"
                "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
            "</tt:PanTiltSpeedSpace>",
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stPanTiltSpeedSpace[j].aszURI,
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stPanTiltSpeedSpace[j].stXRange.min,
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stPanTiltSpeedSpace[j].stXRange.max);
    }
    for(j = 0; j < pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.sizeContinuousZoomVelocitySpace;j++)
    {            
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tt:ZoomSpeedSpace>"
                "<tt:URI>%s</tt:URI>"
                "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
            "</tt:ZoomSpeedSpace>",
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stZoomSpeedSpace[j].aszURI,
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stZoomSpeedSpace[j].stXRange.min,
            pstPTZ_GetNodeRes->stPTZNode.stSupportedPTZSpaces.stZoomSpeedSpace[j].stXRange.max);
    }
    offset += snprintf(p_buf+offset, mlen-offset,
        "</tt:SupportedPTZSpaces>");
    //preset
    offset += snprintf(p_buf+offset, mlen-offset,
        "<tt:MaximumNumberOfPresets>%d</tt:MaximumNumberOfPresets>"
        "<tt:HomeSupported>%s</tt:HomeSupported>",
        pstPTZ_GetNodeRes->stPTZNode.maximumNumberOfPresets,
        pstPTZ_GetNodeRes->stPTZNode.enHomeSupported ? "true" : "false");
                                                                                                     
    offset += snprintf(p_buf+offset, mlen-offset,
        "<tt:Extension>"
            "<tt:SupportedPresetTour>"
            "<tt:MaximumNumberOfPresetTours>%d</tt:MaximumNumberOfPresetTours>",
        pstPTZ_GetNodeRes->stPTZNode.stExtension.stSupportedPresetTour.maximumNumberOfPresetTours);
    for(j = 0; j < pstPTZ_GetNodeRes->stPTZNode.stExtension.stSupportedPresetTour.sizePTZPresetTourOperation; j++)
    {
        if(pstPTZ_GetNodeRes->stPTZNode.stExtension.stSupportedPresetTour.enPTZPresetTourOperation[j] == PTZPresetTourOperation_Start)
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:PTZPresetTourOperation>Start</tt:PTZPresetTourOperation>");
        else if(pstPTZ_GetNodeRes->stPTZNode.stExtension.stSupportedPresetTour.enPTZPresetTourOperation[j] == PTZPresetTourOperation_Stop)
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:PTZPresetTourOperation>Stop</tt:PTZPresetTourOperation>");
        else if(pstPTZ_GetNodeRes->stPTZNode.stExtension.stSupportedPresetTour.enPTZPresetTourOperation[j] == PTZPresetTourOperation_Pause)
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:PTZPresetTourOperation>Pause</tt:PTZPresetTourOperation>");
    }
    offset += snprintf(p_buf+offset, mlen-offset, 
            "</tt:SupportedPresetTour>"
        "</tt:Extension>");

    offset += snprintf(p_buf+offset, mlen-offset, "</tptz:PTZNode>");
    offset += snprintf(p_buf+offset, mlen-offset, "</tptz:GetNodeResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}

GK_S32 soap_packet_response_GetConfigurations(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_PTZ_GetConfigutations_Res_S *pstGetConfigurationsRes = (GONVIF_PTZ_GetConfigutations_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GetConfigurationsResponse>");
    GK_S32 i = 0;
    for(i = 0; i < pstGetConfigurationsRes->sizePTZConfiguration; i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
        "<tptz:PTZConfiguration token=\"%s\">"
            "<tt:Name>%s</tt:Name>"
            "<tt:UseCount>%d</tt:UseCount>"
            "<tt:NodeToken>%s</tt:NodeToken>"
            "<tt:DefaultAbsolutePantTiltPositionSpace>%s</tt:DefaultAbsolutePantTiltPositionSpace>"
            "<tt:DefaultAbsoluteZoomPositionSpace>%s</tt:DefaultAbsoluteZoomPositionSpace>"
            "<tt:DefaultRelativePanTiltTranslationSpace>%s</tt:DefaultRelativePanTiltTranslationSpace>"
            "<tt:DefaultRelativeZoomTranslationSpace>%s</tt:DefaultRelativeZoomTranslationSpace>"
            "<tt:DefaultContinuousPanTiltVelocitySpace>%s</tt:DefaultContinuousPanTiltVelocitySpace>"
            "<tt:DefaultContinuousZoomVelocitySpace>%s</tt:DefaultContinuousZoomVelocitySpace>"
            "<tt:DefaultPTZSpeed>"
                "<tt:PanTilt space=\"%s\" y=\"%0.1f\" x=\"%0.1f\"></tt:PanTilt>"
                "<tt:Zoom space=\"%s\" x=\"%0.1f\"></tt:Zoom>"
            "</tt:DefaultPTZSpeed>"
            "<tt:DefaultPTZTimeout>PT%lldH%lldM%lldS</tt:DefaultPTZTimeout>"
            "<tt:PanTiltLimits>"
                "<tt:Range>"
                    "<tt:URI>%s</tt:URI>"
                    "<tt:XRange><tt:Min>%0.1f</tt:Min><tt:Max>%0.1f</tt:Max></tt:XRange>"
                    "<tt:YRange><tt:Min>%0.1f</tt:Min><tt:Max>%0.1f</tt:Max></tt:YRange>"
                "</tt:Range>"
            "</tt:PanTiltLimits>"
            "<tt:ZoomLimits>"
                "<tt:Range>"
                    "<tt:URI>%s</tt:URI>"
                    "<tt:XRange><tt:Min>%0.1f</tt:Min><tt:Max>%0.1f</tt:Max></tt:XRange>"
                "</tt:Range>"
            "</tt:ZoomLimits>"
        "</tptz:PTZConfiguration>",
        pstGetConfigurationsRes->stPTZConfiguration[i].token,
        pstGetConfigurationsRes->stPTZConfiguration[i].name,
        pstGetConfigurationsRes->stPTZConfiguration[i].useCount,
        pstGetConfigurationsRes->stPTZConfiguration[i].nodeToken,
        pstGetConfigurationsRes->stPTZConfiguration[i].defaultAbsolutePantTiltPositionSpace,
        pstGetConfigurationsRes->stPTZConfiguration[i].defaultAbsoluteZoomPositionSpace,
        pstGetConfigurationsRes->stPTZConfiguration[i].defaultRelativePanTiltTranslationSpace,
        pstGetConfigurationsRes->stPTZConfiguration[i].defaultRelativeZoomTranslationSpace,
        pstGetConfigurationsRes->stPTZConfiguration[i].defaultContinuousPanTiltVelocitySpace,
        pstGetConfigurationsRes->stPTZConfiguration[i].defaultContinuousZoomVelocitySpace,
        pstGetConfigurationsRes->stPTZConfiguration[i].stDefaultPTZSpeed.stPanTilt.aszSpace,
        pstGetConfigurationsRes->stPTZConfiguration[i].stDefaultPTZSpeed.stPanTilt.y,
        pstGetConfigurationsRes->stPTZConfiguration[i].stDefaultPTZSpeed.stPanTilt.x,
        pstGetConfigurationsRes->stPTZConfiguration[i].stDefaultPTZSpeed.stZoom.aszSpace,
        pstGetConfigurationsRes->stPTZConfiguration[i].stDefaultPTZSpeed.stZoom.x,
        (pstGetConfigurationsRes->stPTZConfiguration[i].defaultPTZTimeout)/3600,
        ((pstGetConfigurationsRes->stPTZConfiguration[i].defaultPTZTimeout)%3600)/60,
        ((pstGetConfigurationsRes->stPTZConfiguration[i].defaultPTZTimeout)%3600)%60,
        pstGetConfigurationsRes->stPTZConfiguration[i].stPanTiltLimits.stRange.aszURI,
        pstGetConfigurationsRes->stPTZConfiguration[i].stPanTiltLimits.stRange.stXRange.min,
        pstGetConfigurationsRes->stPTZConfiguration[i].stPanTiltLimits.stRange.stXRange.max,
        pstGetConfigurationsRes->stPTZConfiguration[i].stPanTiltLimits.stRange.stYRange.min,
        pstGetConfigurationsRes->stPTZConfiguration[i].stPanTiltLimits.stRange.stYRange.max,
        pstGetConfigurationsRes->stPTZConfiguration[i].stZoomLimits.stRange.aszURI,
        pstGetConfigurationsRes->stPTZConfiguration[i].stZoomLimits.stRange.stXRange.min,
        pstGetConfigurationsRes->stPTZConfiguration[i].stZoomLimits.stRange.stXRange.max);
    }

    offset += snprintf(p_buf+offset, mlen-offset, "</tptz:GetConfigurationsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	
    return offset;
}


GK_S32 soap_packet_response_GetConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_PTZ_GetConfiguration_Res_S *pstGetConfigurationRes = (GONVIF_PTZ_GetConfiguration_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tptz:GetConfigurationResponse>");
    offset += snprintf(p_buf+offset, mlen-offset, 
            "<tptz:PTZConfiguration token=\"%s\">"
                "<tt:Name>%s</tt:Name>"
                "<tt:UseCount>%d</tt:UseCount>"
                "<tt:NodeToken>%s</tt:NodeToken>"
                "<tt:DefaultAbsolutePantTiltPositionSpace>%s</tt:DefaultAbsolutePantTiltPositionSpace>"
                "<tt:DefaultAbsoluteZoomPositionSpace>%s</tt:DefaultAbsoluteZoomPositionSpace>"
                "<tt:DefaultRelativePanTiltTranslationSpace>%s</tt:DefaultRelativePanTiltTranslationSpace>"
                "<tt:DefaultRelativeZoomTranslationSpace>%s</tt:DefaultRelativeZoomTranslationSpace>"
                "<tt:DefaultContinuousPanTiltVelocitySpace>%s</tt:DefaultContinuousPanTiltVelocitySpace>"
                "<tt:DefaultContinuousZoomVelocitySpace>%s</tt:DefaultContinuousZoomVelocitySpace>"
                "<tt:DefaultPTZSpeed>"
                "<tt:PanTilt space=\"%s\" y=\"%0.1f\" x=\"%0.1f\"></tt:PanTilt>"
                "<tt:Zoom space=\"%s\" x=\"%0.1f\"></tt:Zoom>"
                "</tt:DefaultPTZSpeed>"
                "<tt:DefaultPTZTimeout>PT%lldH%lldM%lldS</tt:DefaultPTZTimeout>"
                "<tt:PanTiltLimits>"
                    "<tt:Range>"
                        "<tt:URI>%s</tt:URI>"
                        "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%0.1f</tt:Max></tt:XRange>"
                        "<tt:YRange><tt:Min>%f</tt:Min><tt:Max>%0.1f</tt:Max></tt:YRange>"
                    "</tt:Range>"
                "</tt:PanTiltLimits>"

                "<tt:ZoomLimits>"
                    "<tt:Range>"
                        "<tt:URI>%s</tt:URI>"
                        "<tt:XRange><tt:Min>%0.1f</tt:Min><tt:Max>%0.1f</tt:Max></tt:XRange>"
                    "</tt:Range>"
                "</tt:ZoomLimits>",

                pstGetConfigurationRes->stPTZConfiguration.token,
                pstGetConfigurationRes->stPTZConfiguration.name,
                pstGetConfigurationRes->stPTZConfiguration.useCount,
                pstGetConfigurationRes->stPTZConfiguration.nodeToken,
                pstGetConfigurationRes->stPTZConfiguration.defaultAbsolutePantTiltPositionSpace,
                pstGetConfigurationRes->stPTZConfiguration.defaultAbsoluteZoomPositionSpace,
                pstGetConfigurationRes->stPTZConfiguration.defaultRelativePanTiltTranslationSpace,
                pstGetConfigurationRes->stPTZConfiguration.defaultRelativeZoomTranslationSpace,
                pstGetConfigurationRes->stPTZConfiguration.defaultContinuousPanTiltVelocitySpace,
                pstGetConfigurationRes->stPTZConfiguration.defaultContinuousZoomVelocitySpace,
                pstGetConfigurationRes->stPTZConfiguration.stDefaultPTZSpeed.stPanTilt.aszSpace,
                pstGetConfigurationRes->stPTZConfiguration.stDefaultPTZSpeed.stPanTilt.y,
                pstGetConfigurationRes->stPTZConfiguration.stDefaultPTZSpeed.stPanTilt.x,
                pstGetConfigurationRes->stPTZConfiguration.stDefaultPTZSpeed.stZoom.aszSpace,
                pstGetConfigurationRes->stPTZConfiguration.stDefaultPTZSpeed.stZoom.x,
                (pstGetConfigurationRes->stPTZConfiguration.defaultPTZTimeout)/3600,
                ((pstGetConfigurationRes->stPTZConfiguration.defaultPTZTimeout)%3600)/60,
                ((pstGetConfigurationRes->stPTZConfiguration.defaultPTZTimeout)%3600)%60,
                pstGetConfigurationRes->stPTZConfiguration.stPanTiltLimits.stRange.aszURI,
                pstGetConfigurationRes->stPTZConfiguration.stPanTiltLimits.stRange.stXRange.min,
                pstGetConfigurationRes->stPTZConfiguration.stPanTiltLimits.stRange.stXRange.max,
                pstGetConfigurationRes->stPTZConfiguration.stPanTiltLimits.stRange.stYRange.min,
                pstGetConfigurationRes->stPTZConfiguration.stPanTiltLimits.stRange.stYRange.max,
                pstGetConfigurationRes->stPTZConfiguration.stZoomLimits.stRange.aszURI,
                pstGetConfigurationRes->stPTZConfiguration.stZoomLimits.stRange.stXRange.min,
                pstGetConfigurationRes->stPTZConfiguration.stZoomLimits.stRange.stXRange.max);
                
    offset += snprintf(p_buf+offset, mlen-offset, 
            "</tptz:PTZConfiguration>"
        "</tptz:GetConfigurationResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}

GK_S32 soap_packet_response_GetConfigurationOptions(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_PTZ_GetConfigurationOptions_Res_S *pstPTZ_GetConfigurationOptionsRes = (GONVIF_PTZ_GetConfigurationOptions_Res_S *)argv;
    GK_S32 i = 0;
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GetConfigurationOptionsResponse>"
                                                    "<tptz:PTZConfigurationOptions>");

    offset += snprintf(p_buf+offset, mlen-offset, "<tt:Spaces>");
    
    for(i = 0;i < pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.sizeAbsolutePanTiltPositionSpace;i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tt:AbsolutePanTiltPositionSpace>"
                "<tt:URI>%s</tt:URI>"
                "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
                "<tt:YRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:YRange>"
            "</tt:AbsolutePanTiltPositionSpace>",
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stAbsolutePanTiltPositionSpace[i].aszURI,
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stAbsolutePanTiltPositionSpace[i].stXRange.min,
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stAbsolutePanTiltPositionSpace[i].stXRange.max,
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stAbsolutePanTiltPositionSpace[i].stYRange.min,
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stAbsolutePanTiltPositionSpace[i].stYRange.max);
    }
    for(i = 0;i < pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.sizeAbsoluteZoomPositionSpace;i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tt:AbsoluteZoomPositionSpace>"
                "<tt:URI>%s</tt:URI>"
                "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
            "</tt:AbsoluteZoomPositionSpace>",
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stAbsoluteZoomPositionSpace[i].aszURI,
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stAbsoluteZoomPositionSpace[i].stXRange.min,
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stAbsoluteZoomPositionSpace[i].stXRange.max);
    }
    for(i = 0;i < pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.sizeRelativePanTiltTranslationSpace;i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tt:RelativePanTiltTranslationSpace>"
                "<tt:URI>%s</tt:URI>"
                "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
                "<tt:YRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:YRange>"
            "</tt:RelativePanTiltTranslationSpace>",
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stRelativePanTiltTranslationSpace[i].aszURI,
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stRelativePanTiltTranslationSpace[i].stXRange.min,
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stRelativePanTiltTranslationSpace[i].stXRange.max,
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stRelativePanTiltTranslationSpace[i].stYRange.min,
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stRelativePanTiltTranslationSpace[i].stYRange.max);
    }
    for(i = 0;i < pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.sizeRelativeZoomTranslationSpace;i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tt:RelativeZoomTranslationSpace>"
                "<tt:URI>%s</tt:URI>"
                "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
            "</tt:RelativeZoomTranslationSpace>",
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stRelativeZoomTranslationSpace[i].aszURI,
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stRelativeZoomTranslationSpace[i].stXRange.min,
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stRelativeZoomTranslationSpace[i].stXRange.max);
    }
    for(i = 0;i < pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.sizeContinuousPanTiltVelocitySpace;i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset,
            "<tt:ContinuousPanTiltVelocitySpace>"
                "<tt:URI>%s</tt:URI>"
                "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
                "<tt:YRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:YRange>"
            "</tt:ContinuousPanTiltVelocitySpace>",
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stContinuousPanTiltVelocitySpace[i].aszURI,
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stContinuousPanTiltVelocitySpace[i].stXRange.min,
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stContinuousPanTiltVelocitySpace[i].stXRange.max,
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stContinuousPanTiltVelocitySpace[i].stYRange.min,
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stContinuousPanTiltVelocitySpace[i].stYRange.max);
    }
    for(i = 0;i < pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.sizeContinuousZoomVelocitySpace;i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tt:ContinuousZoomVelocitySpace>"
                "<tt:URI>%s</tt:URI>"
                "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
            "</tt:ContinuousZoomVelocitySpace>",        
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stContinuousZoomVelocitySpace[i].aszURI,
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stContinuousZoomVelocitySpace[i].stXRange.min,
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stContinuousZoomVelocitySpace[i].stXRange.max);
    }
    for(i = 0;i < pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.sizePanTiltSpeedSpace;i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tt:PanTiltSpeedSpace>"
                "<tt:URI>%s</tt:URI>"
                "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
            "</tt:PanTiltSpeedSpace>",
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stPanTiltSpeedSpace[i].aszURI,
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stPanTiltSpeedSpace[i].stXRange.min,
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stPanTiltSpeedSpace[i].stXRange.max);
    }
    for(i = 0;i < pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.sizeZoomSpeedSpace;i++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, 
            "<tt:ZoomSpeedSpace>"
                "<tt:URI>%s</tt:URI>"
                "<tt:XRange><tt:Min>%f</tt:Min><tt:Max>%f</tt:Max></tt:XRange>"
            "</tt:ZoomSpeedSpace>",
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stZoomSpeedSpace[i].aszURI,
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stZoomSpeedSpace[i].stXRange.min,
            pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stSpaces.stZoomSpeedSpace[i].stXRange.max);
        
    }
    offset += snprintf(p_buf+offset, mlen-offset, 
        "</tt:Spaces>");

                                              
    offset += snprintf(p_buf+offset, mlen-offset, 
        "<tt:PTZTimeout>"
            "<tt:Min>PT%lldH%lldM%lldS</tt:Min>"
            "<tt:Max>PT%lldH%lldM%lldS</tt:Max>"
        "</tt:PTZTimeout>",
        (pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stPTZTimeout.min)/3600,
        ((pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stPTZTimeout.min)%3600)/60,
        ((pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stPTZTimeout.min)%3600)%60,
        (pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stPTZTimeout.max)/3600,
        ((pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stPTZTimeout.max)%3600)/60,
        ((pstPTZ_GetConfigurationOptionsRes->stPTZConfigurationOptions.stPTZTimeout.max)%3600)%60);

    offset += snprintf(p_buf+offset, mlen-offset,
        "</tptz:PTZConfigurationOptions>"
    "</tptz:GetConfigurationOptionsResponse>");

    offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}

GK_S32 soap_packet_response_SetConfiguration(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, "<tptz:SetConfigurationResponse>"
                                                  "</tptz:SetConfigurationResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}

GK_S32 soap_packet_response_ContinuousMove(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
    offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
    offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, "<tptz:ContinuousMoveResponse>"
                                                  "</tptz:ContinuousMoveResponse>");

    offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}

GK_S32 soap_response_PTZ_Stop(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, "<tptz:StopResponse>"
    											  "</tptz:StopResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}

GK_S32 soap_packet_response_GetPresets(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 i = 0;
    GONVIF_PTZ_GetPresets_Res_S *pstPTZ_GetPresetsRes = (GONVIF_PTZ_GetPresets_Res_S *)argv;
    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GetPresetsResponse>");

	for (i = 0; i < pstPTZ_GetPresetsRes->sizePreset; i++)
	{
    	offset += snprintf(p_buf+offset, mlen-offset,
            "<tptz:Preset token=\"%s\">"
            	"<tt:Name>%s</tt:Name>"
            	"<tt:PTZPosition>"
            		"<tt:PanTilt x=\"%0.1f\" y=\"%0.1f\" space=\"%s\">"
            		"</tt:PanTilt>"
            		"<tt:Zoom x=\"%0.1f\" space=\"%s\">"
            		"</tt:Zoom>"
            	"</tt:PTZPosition>"
            "</tptz:Preset>",
            pstPTZ_GetPresetsRes->stPreset[i].aszToken,
            pstPTZ_GetPresetsRes->stPreset[i].aszName,
            pstPTZ_GetPresetsRes->stPreset[i].stPTZPosition.stPanTilt.x,
            pstPTZ_GetPresetsRes->stPreset[i].stPTZPosition.stPanTilt.y,
            pstPTZ_GetPresetsRes->stPreset[i].stPTZPosition.stPanTilt.aszSpace,
            pstPTZ_GetPresetsRes->stPreset[i].stPTZPosition.stZoom.x,
            pstPTZ_GetPresetsRes->stPreset[i].stPTZPosition.stZoom.aszSpace);
    }

	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:GetPresetsResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_SetPreset(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_PTZ_SetPreset_Res_S *pstPTZ_SetPresetRes = (GONVIF_PTZ_SetPreset_Res_S *)argv;
    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset,
	    "<tptz:SetPresetResponse>"
	    	"<tptz:PresetToken>%s</tptz:PresetToken>"
    	"</tptz:SetPresetResponse>", 
    	pstPTZ_SetPresetRes->aszPresetToken);
    	
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}

GK_S32 soap_packet_response_GotoPreset(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GotoPresetResponse>"
												  "</tptz:GotoPresetResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}

GK_S32 soap_packet_response_RemovePreset(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
#if 1
    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:RemovePresetResponse></tptz:RemovePresetResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;


#else
    GK_CHAR offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:RemovePresetResponse></tptz:RemovePresetResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
#endif
}

GK_S32 soap_packet_response_AbsoluteMove(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
    offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
    offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, "<tptz:AbsoluteMoveResponse>"
                                                  "</tptz:AbsoluteMoveResponse>");

    offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}

GK_S32 soap_packet_response_RelativeMove(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
    offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
    offset += snprintf(p_buf+offset, mlen-offset, soap_body);

    offset += snprintf(p_buf+offset, mlen-offset, "<tptz:RelativeMoveResponse>"
                                                  "</tptz:RelativeMoveResponse>");

    offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
    return offset;
}

GK_S32 soap_response_PTZ_GetStatus(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
    GONVIF_PTZ_GetStatus_S *pstGetStatusRes = (GONVIF_PTZ_GetStatus_S *)argv;
	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:GetStatusResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tptz:PTZStatus>");

	offset += snprintf(p_buf+offset, mlen-offset,
		"<tt:Position>"
			"<tt:PanTilt x=\"%0.1f\" y=\"%0.1f\" space=\"%s\" />"
			"<tt:Zoom x=\"%0.1f\" space=\"%s\" />"
		"</tt:Position>"
		"<tt:MoveStatus>"
			"<tt:PanTilt>%s</tt:PanTilt>"
			"<tt:Zoom>%s</tt:Zoom>"
		"</tt:MoveStatus>",
		pstGetStatusRes->stPosition.stPanTilt.x, pstGetStatusRes->stPosition.stPanTilt.y, pstGetStatusRes->stPosition.stPanTilt.aszSpace,
		pstGetStatusRes->stPosition.stZoom.x,pstGetStatusRes->stPosition.stZoom.aszSpace,
		onvif_get_ptz_movestatus(pstGetStatusRes->stMoveStatus.enPanTilt), onvif_get_ptz_movestatus(pstGetStatusRes->stMoveStatus.enZoom));

	if (pstGetStatusRes->aszError[0] != '\0')
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Error>%s</tt:Error>",pstGetStatusRes->aszError);
	}

	offset += snprintf(p_buf+offset, mlen-offset, "<tt:UtcTime>%s</tt:UtcTime>", pstGetStatusRes->aszUtcTime);

	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:PTZStatus>");
	offset += snprintf(p_buf+offset, mlen-offset, "</tptz:GetStatusResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

    return offset;
}

/************************************ ptz end ***************************************/

/********************************* video analytics *************************************/
GK_S32 soap_response_VideoAnalytics_GetServiceCapabilities(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GONVIF_ANALYTICS_GetServiceCapabilities_Res_S *pstAN_GetServiceCapabilitiesRes = (GONVIF_ANALYTICS_GetServiceCapabilities_Res_S *)argv;

	GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);

	offset += snprintf(p_buf+offset, mlen-offset, "<tan:GetServiceCapabilitiesResponse>");
	
	offset += snprintf(p_buf+offset, mlen-offset, g_aszAnalyticsCapabilities,
		pstAN_GetServiceCapabilitiesRes->stCapabilities.enRuleSupport ? "true" : "false",
		pstAN_GetServiceCapabilitiesRes->stCapabilities.enAnalyticsModulesSupport ? "true" : "false",
		pstAN_GetServiceCapabilitiesRes->stCapabilities.enCellBasedSceneDescriptionSupported ? "true" : "false");
		
	offset += snprintf(p_buf+offset, mlen-offset, "</tan:GetServiceCapabilitiesResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);
	return offset;
}


GK_S32 soap_packet_response_GetSupportedAnalyticsModules(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GONVIF_ANALYTICS_GetSupportedAnalyticsModulesRes_S *pstGetSupportedAnalyticsModulesRes = (GONVIF_ANALYTICS_GetSupportedAnalyticsModulesRes_S *)argv;
	GK_S32 i, j, k;
    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<tan:GetSupportedAnalyticsModulesResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tan:SupportedAnalyticsModules>");

	for(i = 0;i < pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.sizeAnalyticsModuleContentSchemaLocation; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:AnalyticsModuleContentSchemaLocation>%s</tt:AnalyticsModuleContentSchemaLocation>",
			pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.pszAnalyticsModuleContentSchemaLocation[i]);
	}
	for(i = 0;i < pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.sizeAnalyticsModuleDescription; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:AnalyticsModuleDescription Name=\"tt:%s\">",
			pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[i].name);

		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Parameters>");
		for(j = 0; j < pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[i].stParameters.sizeSimpleItemDescription; j++)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:SimpleItemDescription Type=\"%s\" Name=\"%s\">",
				pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[i].stParameters.stSimpleItemDescription[j].type,
				pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[i].stParameters.stSimpleItemDescription[j].name);
			offset += snprintf(p_buf+offset, mlen-offset, "</tt:SimpleItemDescription>");
		}
		for(j = 0; j < pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[i].stParameters.sizeElementItemDescription; j++)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:ElementItemDescription Type=\"%s\" Name=\"%s\">",
				pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[i].stParameters.stElementItemDescription[j].type,
				pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[i].stParameters.stElementItemDescription[j].name);
			offset += snprintf(p_buf+offset, mlen-offset, "</tt:ElementItemDescription>");
		}
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Parameters>");

		for(j = 0; j < pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[i].sizeMessages; j++)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Messages IsProperty=\"%s\">",
				pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[i].stMessages[j].enIsProperty ? "true":"flase");
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Source>");
			for(k = 0; k < pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[i].stMessages[j].stSource.sizeSimpleItemDescription; k++)
			{
				offset += snprintf(p_buf+offset, mlen-offset, "<tt:SimpleItemDescription Type=\"%s\" Name=\"%s\">",
					pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[i].stMessages[j].stSource.stSimpleItemDescription[k].type,
					pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[i].stMessages[j].stSource.stSimpleItemDescription[k].name);
				offset += snprintf(p_buf+offset, mlen-offset, "</tt:SimpleItemDescription>");
			}
			offset += snprintf(p_buf+offset, mlen-offset, "</tt:Source>");
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Data>");
			for(k = 0; k < pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[i].stMessages[j].stData.sizeSimpleItemDescription; k++)
			{
				offset += snprintf(p_buf+offset, mlen-offset, "<tt:SimpleItemDescription Type=\"%s\" Name=\"%s\">",
					pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[i].stMessages[j].stData.stSimpleItemDescription[k].type,
					pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[i].stMessages[j].stData.stSimpleItemDescription[k].name);
				offset += snprintf(p_buf+offset, mlen-offset, "</tt:SimpleItemDescription>");
			}
			offset += snprintf(p_buf+offset, mlen-offset, "</tt:Data>");

			offset += snprintf(p_buf+offset, mlen-offset, "<tt:ParentTopic>%s</tt:ParentTopic>",
				pstGetSupportedAnalyticsModulesRes->stSupportedAnalyticsModules.stAnalyticsModuleDescription[i].stMessages[j].aszParentTopic);

			offset += snprintf(p_buf+offset, mlen-offset, "</tt:Messages>");
		}

		offset += snprintf(p_buf+offset, mlen-offset, "</tt:AnalyticsModuleDescription>");
	}

	offset += snprintf(p_buf+offset, mlen-offset, "</tan:SupportedAnalyticsModules>");
	offset += snprintf(p_buf+offset, mlen-offset, "</tan:GetSupportedAnalyticsModulesResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;

}

GK_S32 soap_packet_response_GetAnalyticsModules(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GONVIF_ANALYTICS_GetAnalyticsModulesRes_S *pstGetAnalyticsModulesRes = (GONVIF_ANALYTICS_GetAnalyticsModulesRes_S *)argv;

	GK_S32 i, j;
    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<tan:GetAnalyticsModulesResponse>");

	for(i = 0; i < pstGetAnalyticsModulesRes->sizeAnalyticsModule; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tan:AnalyticsModule Type=\"tt:%s\" Name=\"%s\">",
			pstGetAnalyticsModulesRes->stAnalyticsModule[i].type,
			pstGetAnalyticsModulesRes->stAnalyticsModule[i].name);
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Parameters>");
		for(j = 0; j < pstGetAnalyticsModulesRes->stAnalyticsModule[i].stParameters.sizeSimpleItem; j++)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:SimpleItem Value=\"%s\" Name=\"%s\">",
				pstGetAnalyticsModulesRes->stAnalyticsModule[i].stParameters.stSimpleItem[j].value,
				pstGetAnalyticsModulesRes->stAnalyticsModule[i].stParameters.stSimpleItem[j].name);
			offset += snprintf(p_buf+offset, mlen-offset, "</tt:SimpleItem>");
		}
		for(j = 0; j < pstGetAnalyticsModulesRes->stAnalyticsModule[i].stParameters.sizeElementItem; j++)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:ElementItem Name=\"%s\">"
													 	  "%s",
				pstGetAnalyticsModulesRes->stAnalyticsModule[i].stParameters.stElementItem[j].name,
				pstGetAnalyticsModulesRes->stAnalyticsModule[i].stParameters.stElementItem[j].any);
			offset += snprintf(p_buf+offset, mlen-offset, "</tt:ElementItem>");
		}
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Parameters>");
		offset += snprintf(p_buf+offset, mlen-offset, "</tan:AnalyticsModule>");
	}
	offset += snprintf(p_buf+offset, mlen-offset, "</tan:GetAnalyticsModulesResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;

}

GK_S32 soap_packet_response_GetSupportedRules(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GONVIF_ANALYITICS_GetSupportedRulesRes_S *pstGetSupportedRulesRes = (GONVIF_ANALYITICS_GetSupportedRulesRes_S *)argv;
	GK_S32 i, j, k;
    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);

	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);

	offset += snprintf(p_buf+offset, mlen-offset, "<tan:GetSupportedRulesResponse>");
	offset += snprintf(p_buf+offset, mlen-offset, "<tan:SupportedRules>");

	for(i = 0;i < pstGetSupportedRulesRes->stSupportedRules.sizeRuleContentSchemaLocation; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:RuleContentSchemaLocation>%s</tt:RuleContentSchemaLocation>",
			pstGetSupportedRulesRes->stSupportedRules.pszRuleContentSchemaLocation[i]);
	}
	for(i = 0;i < pstGetSupportedRulesRes->stSupportedRules.sizeRuleDescription; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:RuleDescription Name=\"tt:%s\">",
			pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[i].name);

		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Parameters>");
		for(j = 0; j < pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[i].stParameters.sizeSimpleItemDescription; j++)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:SimpleItemDescription Type=\"%s\" Name=\"%s\">",
				pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[i].stParameters.stSimpleItemDescription[j].type,
				pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[i].stParameters.stSimpleItemDescription[j].name);
			offset += snprintf(p_buf+offset, mlen-offset, "</tt:SimpleItemDescription>");
		}
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Parameters>");

		for(j = 0; j < pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[i].sizeMessages; j++)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Messages IsProperty=\"%s\">",
				pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[i].stMessages[j].enIsProperty ? "true":"flase");

			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Source>");
			for(k = 0; k < pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[i].stMessages[j].stSource.sizeSimpleItemDescription; k++)
			{
				offset += snprintf(p_buf+offset, mlen-offset, "<tt:SimpleItemDescription Type=\"%s\" Name=\"%s\">",
					pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[i].stMessages[j].stSource.stSimpleItemDescription[k].type,
					pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[i].stMessages[j].stSource.stSimpleItemDescription[k].name);
				offset += snprintf(p_buf+offset, mlen-offset, "</tt:SimpleItemDescription>");
			}
			offset += snprintf(p_buf+offset, mlen-offset, "</tt:Source>");

			offset += snprintf(p_buf+offset, mlen-offset, "<tt:Data>");
			for(k = 0; k < pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[i].stMessages[j].stData.sizeSimpleItemDescription; k++)
			{
				offset += snprintf(p_buf+offset, mlen-offset, "<tt:SimpleItemDescription Type=\"%s\" Name=\"%s\">",
					pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[i].stMessages[j].stData.stSimpleItemDescription[k].type,
					pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[i].stMessages[j].stData.stSimpleItemDescription[k].name);
				offset += snprintf(p_buf+offset, mlen-offset, "</tt:SimpleItemDescription>");
			}
			offset += snprintf(p_buf+offset, mlen-offset, "</tt:Data>");

			offset += snprintf(p_buf+offset, mlen-offset, "<tt:ParentTopic>%s</tt:ParentTopic>",
				pstGetSupportedRulesRes->stSupportedRules.stRuleDescription[i].stMessages[j].aszParentTopic);

			offset += snprintf(p_buf+offset, mlen-offset, "</tt:Messages>");
		}


		offset += snprintf(p_buf+offset, mlen-offset, "</tt:RuleDescription>");
	}

	offset += snprintf(p_buf+offset, mlen-offset, "</tan:SupportedRules>");
	offset += snprintf(p_buf+offset, mlen-offset, "</tan:GetSupportedRulesResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;

}

GK_S32 soap_packet_response_GetRules(GK_CHAR *p_buf, GK_S32 mlen, const void *argv, const void *header)
{
	GONVIF_ANALYTICS_GetRulesRes_s *pstGetRulesRes = (GONVIF_ANALYTICS_GetRulesRes_s *)argv;

	GK_S32 i, j;
    GK_S32 offset = snprintf(p_buf, mlen, xml_hdr);
	offset += snprintf(p_buf+offset, mlen-offset, onvif_xmlns);
	offset += snprintf(p_buf+offset, mlen-offset, soap_body);
	offset += snprintf(p_buf+offset, mlen-offset, "<tan:GetGetRulesResponse>");

	for(i = 0; i < pstGetRulesRes->sizeRule; i++)
	{
		offset += snprintf(p_buf+offset, mlen-offset, "<tan:Rule Type=\"tt:%s\" Name=\"%s\">",
			pstGetRulesRes->stRule[i].type,
			pstGetRulesRes->stRule[i].name);
		offset += snprintf(p_buf+offset, mlen-offset, "<tt:Parameters>");
		for(j = 0; j < pstGetRulesRes->stRule[i].stParameters.sizeSimpleItem; j++)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:SimpleItem Value=\"%s\" Name=\"%s\">",
				pstGetRulesRes->stRule[i].stParameters.stSimpleItem[j].value,
				pstGetRulesRes->stRule[i].stParameters.stSimpleItem[j].name);
			offset += snprintf(p_buf+offset, mlen-offset, "</tt:SimpleItem>");
		}
		for(j = 0; j < pstGetRulesRes->stRule[i].stParameters.sizeElementItem; j++)
		{
			offset += snprintf(p_buf+offset, mlen-offset, "<tt:ElementItem Name=\"%s\">"
													 	  "%s",
				pstGetRulesRes->stRule[i].stParameters.stElementItem[j].name,
				pstGetRulesRes->stRule[i].stParameters.stElementItem[j].any);
			offset += snprintf(p_buf+offset, mlen-offset, "</tt:ElementItem>");
		}
		offset += snprintf(p_buf+offset, mlen-offset, "</tt:Parameters>");
		offset += snprintf(p_buf+offset, mlen-offset, "</tan:Rule>");
	}
	offset += snprintf(p_buf+offset, mlen-offset, "</tan:GetGetRulesResponse>");

	offset += snprintf(p_buf+offset, mlen-offset, soap_tailer);

	return offset;
}


/*************************************************************************************************************/
static GK_CHAR *get_H264Profile_string(GONVIF_MEDIA_H264Profile_E profile)
{
    switch (profile)
    {
        case H264Profile_BaseLine:
            return "Baseline";
        case H264Profile_Main:
            return "Main";
        case H264Profile_High:
            return "High";
        case H264Profile_Extended:
            return "Extended";
     }
    return "Baseline";
}

static GK_CHAR *get_Mpeg4Profile_string(GONVIF_MEDIA_Mepg4Profile_E profile)
{
    switch (profile)
    {
        case Mpeg4Profile_SP:
            return "SP";

        case Mpeg4Profile_ASP:
            return "ASP";
    }
    return "SP";
}

static GK_CHAR *get_videoEncodingType_string(GONVIF_MEDIA_VideoEncoding_E encoding)
{
    switch (encoding)
    {
        case VideoEncoding_JPEG:
            return "JPEG";

        case VideoEncoding_MPEG:
            return "MPEG";

        case VideoEncoding_H264:
            return "H264";
    }
    return "H264";
}

static GK_CHAR *get_audioEncodingType_string(GONVIF_MEDIA_AudioEncoding_E encoding)
{
    switch(encoding)
    {
        case AudioEncoding_G711:
            return "G711";

        case AudioEncoding_G726:
        	return "G726";

        case AudioEncoding_AAC:
        	return "AAC";
    }
    
    return "G711";
}

static GK_CHAR *onvif_get_ptz_movestatus(GONVIF_PTZ_MoveStatus_E movestatus)
{
	if (PTZ_MoveStatus_IDLE == movestatus)
	{
		return "IDLE";
	}
	else if (PTZ_MoveStatus_MOVING == movestatus)
	{
		return "MOVING";
	}
	else if (PTZ_MoveStatus_UNKNOWN == movestatus)
	{
		return "UNKNOWN";
	}
    return NULL;
}

static GK_S32 packet_VideoEncoderConfiguration(GK_CHAR *p_buf, GK_S32 mlen, GONVIF_MEDIA_VideoEncoderConfiguration_S *argv)
{
	GK_S32 offset = 0;
	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tt:Name>%s</tt:Name>"
	    "<tt:UseCount>%d</tt:UseCount>"
	    "<tt:Encoding>%s</tt:Encoding>"
	    "<tt:Resolution>"
	        "<tt:Width>%d</tt:Width>"
	        "<tt:Height>%d</tt:Height>"
	    "</tt:Resolution>"
	    "<tt:Quality>%f</tt:Quality>"
	    "<tt:RateControl>"
	        "<tt:FrameRateLimit>%d</tt:FrameRateLimit>"
	        "<tt:EncodingInterval>%d</tt:EncodingInterval>"
	        "<tt:BitrateLimit>%d</tt:BitrateLimit>"
	    "</tt:RateControl>",
	    argv->pszName, 
	    argv->useCount, 
	    get_videoEncodingType_string(argv->enEncoding),
	    argv->stResolution.width, 
	    argv->stResolution.height, 
	    argv->quality, 
	    argv->stRateControl.frameRateLimit,
	    argv->stRateControl.encodingInterval, 
	    argv->stRateControl.bitrateLimit);

	if(VideoEncoding_H264 == argv->enEncoding)
	{
		offset += snprintf(p_buf+offset, mlen-offset, 
		    "<tt:H264>"
		    "<tt:GovLength>%d</tt:GovLength>"
    		"<tt:H264Profile>%s</tt:H264Profile>"
    		"</tt:H264>", 
    		argv->stH264.govLength,
	    	get_H264Profile_string(argv->stH264.enH264Profile));
	}
	else if(VideoEncoding_MPEG == argv->enEncoding)
	{
		offset += snprintf(p_buf+offset, mlen-offset,
		    "<tt:MPEG4>"
		        "<tt:GovLength>%d</tt:GovLength>"
    		    "<tt:Mpeg4Profile>%s</tt:Mpeg4Profile>"
    		"</tt:MPEG4>", 
    		argv->stMPEG4.govLength,
	    	get_Mpeg4Profile_string(argv->stMPEG4.enMpeg4Profile));
	}

	offset += snprintf(p_buf+offset, mlen-offset, 
	    "<tt:Multicast>"
	        "<tt:Address>"
	            "<tt:Type>IPv4</tt:Type>"
		        "<tt:IPv4Address>%s</tt:IPv4Address>"
		    "</tt:Address>"
		    "<tt:Port>%d</tt:Port>"
		    "<tt:TTL>%d</tt:TTL>"
		    "<tt:AutoStart>%s</tt:AutoStart>"
		"</tt:Multicast>"
	    "<tt:SessionTimeout>PT%lldS</tt:SessionTimeout>",
	    argv->stMulticast.stIPAddress.pszIPv4Address,
	    argv->stMulticast.port,
	    argv->stMulticast.ttl,
	    argv->stMulticast.enAutoStart? "true" : "false",
	    argv->sessionTimeout);

	return offset;
}

static GK_S32 packet_VideoAnalyticsConfiguration(GK_CHAR *p_buf, GK_S32 mlen, GONVIF_MEDIA_VideoAnalyticsConfiguration_S *argv)
{    
    GK_S32 j = 0;
    GK_S32 k = 0;
	GK_S32 offset = 0;
	GONVIF_MEDIA_VideoAnalyticsConfiguration_S *pstVideoAnalyticsConfiguration = (GONVIF_MEDIA_VideoAnalyticsConfiguration_S *)argv;
	
    offset += snprintf(p_buf+offset, mlen-offset,
        "<tt:Name>%s</tt:Name>"
        "<tt:UseCount>%d</tt:UseCount>",
       pstVideoAnalyticsConfiguration->pszName,
       pstVideoAnalyticsConfiguration->useCount);
    
    /*AnalyticsEngine*/
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:AnalyticsEngineConfiguration>");
    for(j = 0; j <pstVideoAnalyticsConfiguration->stAnalyticsEngineConfiguration.sizeAnalyticsModule;j++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:AnalyticsModule Name=\"%s\" Type=\"%s\">",
           pstVideoAnalyticsConfiguration->stAnalyticsEngineConfiguration.pstAnalyticsModule[j].pszName,
           pstVideoAnalyticsConfiguration->stAnalyticsEngineConfiguration.pstAnalyticsModule[j].pszType);

        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Parameters>");
        for(k = 0; k <pstVideoAnalyticsConfiguration->stAnalyticsEngineConfiguration.pstAnalyticsModule[j].stParameters.sizeSimpleItem;k++)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:SimpleItem Name=\"%s\" Value=\"%s\"/>",
               pstVideoAnalyticsConfiguration->stAnalyticsEngineConfiguration.pstAnalyticsModule[j].stParameters.pstSimpleItem[k].pszName,
               pstVideoAnalyticsConfiguration->stAnalyticsEngineConfiguration.pstAnalyticsModule[j].stParameters.pstSimpleItem[k].pszValue);
        }
        #if 0
        for(k = 0; k < pstVideoAnalyticsConfiguration->stAnalyticsEngineConfiguration.pstAnalyticsModule[j].stItemList.sizeElementItem;k++)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:ElementItem Name=\"%s\"/>",
                pstVideoAnalyticsConfiguration->stAnalyticsEngineConfiguration.pstAnalyticsModule[j].stItemList.pstElement[k].pszName);
        }
        #endif
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:Parameters>");
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:AnalyticsModule>");


    }
    offset += snprintf(p_buf+offset, mlen-offset, "</tt:AnalyticsEngineConfiguration>");

    /*RuleEngine*/
    offset += snprintf(p_buf+offset, mlen-offset, "<tt:RuleEngineConfiguration>");
    for(j = 0; j <pstVideoAnalyticsConfiguration->stRuleEngineConfiguration.sizeRule;j++)
    {
        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Rule Name=\"%s\" Type=\"%s\">",
           pstVideoAnalyticsConfiguration->stRuleEngineConfiguration.pstRule[j].pszName,
           pstVideoAnalyticsConfiguration->stRuleEngineConfiguration.pstRule[j].pszType);

        offset += snprintf(p_buf+offset, mlen-offset, "<tt:Parameters>");
        for(k = 0; k <pstVideoAnalyticsConfiguration->stRuleEngineConfiguration.pstRule[j].stParameters.sizeSimpleItem;k++)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:SimpleItem Name=\"%s\" Value=\"%s\"/>",
               pstVideoAnalyticsConfiguration->stRuleEngineConfiguration.pstRule[j].stParameters.pstSimpleItem[k].pszName,
               pstVideoAnalyticsConfiguration->stRuleEngineConfiguration.pstRule[j].stParameters.pstSimpleItem[k].pszValue);
        }

        for(k = 0; k <pstVideoAnalyticsConfiguration->stRuleEngineConfiguration.pstRule[j].stParameters.sizeElementItem;k++)
        {
            offset += snprintf(p_buf+offset, mlen-offset, "<tt:ElementItem Name=\"%s\"/>",
               pstVideoAnalyticsConfiguration->stRuleEngineConfiguration.pstRule[j].stParameters.pstElementItem[k].pszName);
        }
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:Parameters>");
        offset += snprintf(p_buf+offset, mlen-offset, "</tt:Rule>");
    }
    offset += snprintf(p_buf+offset, mlen-offset, "</tt:RuleEngineConfiguration>");

	return offset;
}

static GK_S32 packet_PTZConfiguration(GK_CHAR *p_buf, GK_S32 mlen, GONVIF_PTZ_Configuration_S *argv)
{
	GK_S32 offset = 0;

	offset += snprintf(p_buf+offset, mlen-offset,
   		"<tt:PTZConfiguration token=\"%s\">"
    	"<tt:Name>%s</tt:Name>"
    	"<tt:UseCount>%d</tt:UseCount>"
    	"<tt:NodeToken>%s</tt:NodeToken>",
    	argv->token, argv->name, argv->useCount, argv->nodeToken);

   offset += snprintf(p_buf+offset, mlen-offset,
	    "<tt:DefaultAbsolutePantTiltPositionSpace>%s</tt:DefaultAbsolutePantTiltPositionSpace>"
	    "<tt:DefaultAbsoluteZoomPositionSpace>%s</tt:DefaultAbsoluteZoomPositionSpace>"
	    "<tt:DefaultRelativePanTiltTranslationSpace>%s</tt:DefaultRelativePanTiltTranslationSpace>"
	    "<tt:DefaultRelativeZoomTranslationSpace>%s</tt:DefaultRelativeZoomTranslationSpace>"
	    "<tt:DefaultContinuousPanTiltVelocitySpace>%s</tt:DefaultContinuousPanTiltVelocitySpace>"
	    "<tt:DefaultContinuousZoomVelocitySpace>%s</tt:DefaultContinuousZoomVelocitySpace>",
	    argv->defaultAbsolutePantTiltPositionSpace, argv->defaultAbsoluteZoomPositionSpace,
	    argv->defaultRelativePanTiltTranslationSpace, argv->defaultRelativeZoomTranslationSpace,
	    argv->defaultContinuousPanTiltVelocitySpace, argv->defaultContinuousZoomVelocitySpace);

	offset += snprintf(p_buf+offset, mlen-offset,
		"<tt:DefaultPTZSpeed>"
			"<tt:PanTilt x=\"%0.1f\" y=\"%0.1f\" space=\"%s\" />"
			"<tt:Zoom x=\"%0.1f\" space=\"%s\" />"
		"</tt:DefaultPTZSpeed>",
		argv->stDefaultPTZSpeed.stPanTilt.x,
		argv->stDefaultPTZSpeed.stPanTilt.y,
		argv->stDefaultPTZSpeed.stPanTilt.aszSpace,
		argv->stDefaultPTZSpeed.stZoom.x, 
		argv->stDefaultPTZSpeed.stZoom.aszSpace);

	offset += snprintf(p_buf+offset, mlen-offset,
    	"<tt:DefaultPTZTimeout>PT%lldH%lldM%lldS</tt:DefaultPTZTimeout>", 
        (argv->defaultPTZTimeout)/3600,
        ((argv->defaultPTZTimeout)%3600)/60,
        ((argv->defaultPTZTimeout)%3600)%60);
  
    offset += snprintf(p_buf+offset, mlen-offset,
		"<tt:PanTiltLimits>"
			"<tt:Range>"
				"<tt:URI>%s</tt:URI>"
				"<tt:XRange>"
					"<tt:Min>%0.1f</tt:Min>"
					"<tt:Max>%0.1f</tt:Max>"
				"</tt:XRange>"
				"<tt:YRange>"
					"<tt:Min>%0.1f</tt:Min>"
					"<tt:Max>%0.1f</tt:Max>"
				"</tt:YRange>"
			"</tt:Range>"
		"</tt:PanTiltLimits>",
		argv->stPanTiltLimits.stRange.aszURI,
		argv->stPanTiltLimits.stRange.stXRange.min, argv->stPanTiltLimits.stRange.stXRange.max,
		argv->stPanTiltLimits.stRange.stYRange.min, argv->stPanTiltLimits.stRange.stYRange.max);

	offset += snprintf(p_buf+offset, mlen-offset,
		"<tt:ZoomLimits>"
			"<tt:Range>"
				"<tt:URI>%s</tt:URI>"
				"<tt:XRange>"
					"<tt:Min>%0.1f</tt:Min>"
					"<tt:Max>%0.1f</tt:Max>"
				"</tt:XRange>"
			"</tt:Range>"
		"</tt:ZoomLimits>",
		argv->stZoomLimits.stRange.aszURI,
		argv->stZoomLimits.stRange.stXRange.min,
		argv->stZoomLimits.stRange.stXRange.max);

	offset += snprintf(p_buf+offset, mlen-offset, "</tt:PTZConfiguration>");

	return offset;
}

