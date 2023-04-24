/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif-no-gsoap/src/soap.c
**
** \brief       soap
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include "http.h"
#include "soap_packet.h"
#include "soap_parse.h"
#include "config.h"
#include "event_handle.h"
#include "nvt_priv.h"


extern GOnvif_DEVMNG_CallbackFunc_S    g_DevMngCallBackRegister;
extern GOnvif_MEDIA_CallbackFunc_S     g_MediaCallBackRegister;
extern GOnvif_IMAGE_CallbackFunc_S     g_ImageCallBackRegister;
extern GOnvif_PTZ_CallbackFunc_S 	   g_PtzCallBackRegister;
extern GOnvif_DEVICEIO_CallbackFunc_S  g_DeviceIOCallBackRegister;
extern GOnvif_EVENT_CallbackFunction_S g_EventCallBackRegister;
extern GOnvif_ANALYTICS_CallbackFunc_S g_AnalyticsCallBackRegister;


extern EVENT_PullMessages_Res_S g_stEventPullMessageRes;
extern GK_S32 devMng_SynUsers();

static ONVIF_Http_package_S stHttpPackage;

/***************************************************************************************/
typedef GK_S32 (*soap_build_xml)(GK_CHAR *p_buf, GK_S32 mlen, const void *body, const void *header);

/***************************************************************************************/
static void    soap_proc_calc_digest(const GK_CHAR *created, GK_U8 *nonce, GK_S32 noncelen, const GK_CHAR *password, GK_U8 hash[20]);
static GK_BOOL soap_proc_auth_process(XMLN *p_Security);
static GK_S32  soap_proc_http_rly(HTTPCLN *p_cln, HTTPMSG *rx_msg, const GK_CHAR *p_xml, GK_S32 len);
static GK_S32  soap_proc_http_err_rly(HTTPCLN *p_cln, HTTPMSG *rx_msg, GK_S32 err_code, const GK_CHAR *err_str, const GK_CHAR *p_xml, GK_S32 len);
static GK_S32  soap_proc_err_rly(HTTPCLN *p_cln, HTTPMSG *rx_msg, const GK_CHAR *code, const GK_CHAR *subcode,
	const GK_CHAR *subcode_ex,const GK_CHAR *reason, GK_S32 http_err_code, const GK_CHAR *http_err_str, const GK_CHAR *header);
static GK_S32  soap_proc_security_rly(HTTPCLN *p_cln, HTTPMSG *rx_msg);
static GK_S32  soap_proc_build_err_rly(HTTPCLN *p_cln, HTTPMSG *rx_msg, ONVIF_RET err, const void *header);
static GK_S32 soap_proc_build_send_rly(HTTPCLN *p_cln, HTTPMSG *rx_msg, soap_build_xml build_xml, const void *pfn_body, const void *pfn_header);


//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************



/******************************* device function **********************************/

GK_S32 soap_Device_GetWsdlUrl(HTTPCLN * p_cln, HTTPMSG * rx_msg, XMLN * p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetWsdlUrl = soap_get_child_node(p_body, "GetWsdlUrl");
    if(p_GetWsdlUrl == NULL)
    {
        return -1;
    }
    GONVIF_DEVMNG_GetWsdlUrl_Res_S stDM_GetWsdlUrlRes;
    memset(&stDM_GetWsdlUrlRes, 0, sizeof(stDM_GetWsdlUrlRes));
    if(g_DevMngCallBackRegister.pfnDM_GetWsdlUrl != NULL)
    {
        ret = g_DevMngCallBackRegister.pfnDM_GetWsdlUrl(&stDM_GetWsdlUrlRes);
    }
    else
    {
    	ret = GK_NVT_DevMng_GetWsdlUrl(&stDM_GetWsdlUrlRes);
    }
    if(ret == ONVIF_OK)
    {
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetWsdlUrl, &stDM_GetWsdlUrlRes, NULL);
	}

	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
	
}

GK_S32 soap_Device_GetServices(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetServices = soap_get_child_node(p_body, "GetServices");
    if(p_GetServices == NULL)
    {
        return -1;
    }
    GONVIF_DEVMGN_GetServices_S stDM_GetServices;
    memset(&stDM_GetServices, 0, sizeof(GONVIF_DEVMGN_GetServices_S));
    stDM_GetServices.enIncludeCapability = Boolean_FALSE;
    
    GONVIF_DEVMGN_GetServices_Res_S stDM_GetServicesRes;
    memset(&stDM_GetServicesRes, 0, sizeof(GONVIF_DEVMGN_GetServices_Res_S));
    
    XMLN *p_IncludeCapability = soap_get_child_node(p_GetServices, "IncludeCapability");
    if (p_IncludeCapability && p_IncludeCapability->data)
    {
        stDM_GetServices.enIncludeCapability = (strcasecmp(p_IncludeCapability->data, "true") ? Boolean_FALSE : Boolean_TRUE);
    }
    if(g_DevMngCallBackRegister.pfnDM_GetServices != NULL)
    {
        ret = g_DevMngCallBackRegister.pfnDM_GetServices(&stDM_GetServices, &stDM_GetServicesRes);
    }
    else
    {
    	ret = GK_NVT_DevMng_GetServices(&stDM_GetServices, &stDM_GetServicesRes);
    }

    if(ret == ONVIF_OK)
    {
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetServices, &stDM_GetServicesRes, NULL);
	}

	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
	
}

GK_S32 soap_Device_GetCapabilities(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetCapabilities = soap_get_child_node(p_body, "GetCapabilities");
    if(p_GetCapabilities == NULL)
    {
        return -1;
    }
    GONVIF_DEVMNG_GetCapabilities_S stDM_GetCapabilities;
    GONVIF_DEVMNG_GetCapabilities_Res_S stDM_GetCapabilitiesRes;
    memset(&stDM_GetCapabilities, 0, sizeof(stDM_GetCapabilities));
    memset(&stDM_GetCapabilitiesRes, 0, sizeof(stDM_GetCapabilitiesRes));
    stDM_GetCapabilities.sizeCategory = 1;
    ret = soap_parse_request_GetCapabilities(p_GetCapabilities, &stDM_GetCapabilities);
	if (ret == ONVIF_OK)
	{
	    if(g_DevMngCallBackRegister.pfnDM_GetCapabilities != NULL)
        {
            ret = g_DevMngCallBackRegister.pfnDM_GetCapabilities(&stDM_GetCapabilities, &stDM_GetCapabilitiesRes);
        }
        else
        {
            ret = GK_NVT_DevMng_GetCapabilities(&stDM_GetCapabilities, &stDM_GetCapabilitiesRes);
        }
	    if(ret == ONVIF_OK)
	    {
			return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetCapabilities, &stDM_GetCapabilitiesRes, NULL);
		}
	}

	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
	
}

GK_S32 soap_Device_GetServiceCapabilities(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetServiceCapabilities = soap_get_child_node(p_body, "GetServiceCapabilities");
    if(p_GetServiceCapabilities == NULL)
    {
        return -1;
    }
	GONVIF_DEVMGN_GetServiceCapabilities_Res_S stDM_GetServiceCapabilitiesRes;
	memset(&stDM_GetServiceCapabilitiesRes, 0, sizeof(stDM_GetServiceCapabilitiesRes));
	if(g_DevMngCallBackRegister.pfnDM_GetServiceCapabilities != NULL)
	{
		ret = g_DevMngCallBackRegister.pfnDM_GetServiceCapabilities(&stDM_GetServiceCapabilitiesRes);
	}
	else
	{
		ret = GK_NVT_DevMng_GetServiceCapabilities(&stDM_GetServiceCapabilitiesRes);
	}
	if(ret == ONVIF_OK)
	{
		return	soap_proc_build_send_rly(p_cln, rx_msg, soap_response_Device_GetServiceCapabilities, &stDM_GetServiceCapabilitiesRes, NULL);
	}

	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Device_SystemReboot(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{    
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_SystemReboot = soap_get_child_node(p_body, "SystemReboot");
    if(p_SystemReboot == NULL)
    {
        return -1;
    }
    GONVIF_DEVMGN_SystemReboot_Res_S stDM_SystemRebootRes;
    memset(&stDM_SystemRebootRes, 0, sizeof(GONVIF_DEVMGN_SystemReboot_Res_S));
    
    if(g_DevMngCallBackRegister.pfnDM_SystemReboot != NULL)
    {
        ret = g_DevMngCallBackRegister.pfnDM_SystemReboot(&stDM_SystemRebootRes);
    }
    else
    {
	    ret = GK_NVT_DevMng_SystemReboot(&stDM_SystemRebootRes);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_SystemReboot, &stDM_SystemRebootRes, NULL);
    }
    
	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
	
}

GK_S32 soap_Device_GetHostname(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetHostName = soap_get_child_node(p_body, "GetHostname");
    if(p_GetHostName == NULL)
    {
        return -1;
    }
    GONVIF_DEVMNG_GetHostname_Res_S stDM_GetHostnameRes;
    memset(&stDM_GetHostnameRes, 0, sizeof(stDM_GetHostnameRes));
    if(g_DevMngCallBackRegister.pfnDM_GetHostname != NULL)
    {
        ret = g_DevMngCallBackRegister.pfnDM_GetHostname(&stDM_GetHostnameRes);
    }
    else
    {
        ret = GK_NVT_DevMng_GetHostname(&stDM_GetHostnameRes);
    }
    if(ret == ONVIF_OK)
    {
        return  soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetHostname, &stDM_GetHostnameRes, NULL);
    }
    
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Device_SetHostname(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_SetHostname = soap_get_child_node(p_body, "SetHostname");
    if(p_SetHostname == NULL)
    {
        return -1;
    }
    GONVIF_DEVMNG_SetHostname_S stDM_SetHostname;
    memset(&stDM_SetHostname, 0, sizeof(stDM_SetHostname));
    ret = soap_parse_request_SetHostname(p_SetHostname, &stDM_SetHostname);
	if(ret == ONVIF_OK)
	{
        if(g_DevMngCallBackRegister.pfnDM_SetHostname != NULL)
        {
            ret = g_DevMngCallBackRegister.pfnDM_SetHostname(&stDM_SetHostname);
        }
        else
        {
            ret = GK_NVT_DevMng_SetHostname(&stDM_SetHostname);
        }
	    if(ret == ONVIF_OK)
	    {
			return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_SetHostname, NULL, NULL);
		}
	}
	
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
    
}

GK_S32 soap_Device_GetDNS(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetDNS = soap_get_child_node(p_body, "GetDNS");
    if(p_GetDNS == NULL)
    {
        return -1;
    }

    GONVIF_DEVMNG_GetDNS_Res_S stDM_GetDNSRes;
    memset(&stDM_GetDNSRes, 0, sizeof(stDM_GetDNSRes));
    if(g_DevMngCallBackRegister.pfnDM_GetDNS != NULL)
    {
        ret = g_DevMngCallBackRegister.pfnDM_GetDNS(&stDM_GetDNSRes);
    }
    else
    {
        ret = GK_NVT_DevMng_GetDNS(&stDM_GetDNSRes);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetDNS, &stDM_GetDNSRes, NULL);
    }
    
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
    
}

GK_S32 soap_Device_SetDNS(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_SetDNS = soap_get_child_node(p_body, "SetDNS");
    if(p_SetDNS == NULL)
    {
        return -1;
    }

    GONVIF_DEVMNG_SetDNS_S stDM_SetNDS;
    memset(&stDM_SetNDS, 0, sizeof(stDM_SetNDS));
	ret = soap_parse_request_SetDNS(p_SetDNS, &stDM_SetNDS);
    if(ret == ONVIF_OK)
    {
        if(g_DevMngCallBackRegister.pfnDM_SetDNS != NULL)
        {
            ret = g_DevMngCallBackRegister.pfnDM_SetDNS(&stDM_SetNDS);
        }
        else
        {
            ret = GK_NVT_DevMng_SetDNS(&stDM_SetNDS);
        }

    	if(ret == ONVIF_OK)
    	{
    		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_SetDNS, NULL, NULL);
    	}
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Device_GetNTP(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_GetNTP = soap_get_child_node(p_body, "GetNTP");
    if(p_GetNTP == NULL)
    {
        return -1;
    }
    GONVIF_DEVMNG_GetNTP_Res_S stDM_GetNTPRes;
    memset(&stDM_GetNTPRes, 0, sizeof(stDM_GetNTPRes));
    if(g_DevMngCallBackRegister.pfnDM_GetNTP != NULL)
    {
        ret = g_DevMngCallBackRegister.pfnDM_GetNTP(&stDM_GetNTPRes);
    }
    else
    {
        ret = GK_NVT_DevMng_GetNTP(&stDM_GetNTPRes);
    }
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetNTP, &stDM_GetNTPRes, NULL);
	}

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
    
}

GK_S32 soap_Device_SetNTP(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_SetNTP = soap_get_child_node(p_body, "SetNTP");
    if(p_SetNTP == NULL)
    {
        return -1;
    }
    GONVIF_DEVMNG_SetNTP_S stDM_SetNTP;
    memset(&stDM_SetNTP, 0, sizeof(stDM_SetNTP));
	ret = soap_parse_request_SetNTP(p_SetNTP, &stDM_SetNTP);
    if (ONVIF_OK == ret)
    {

        if(g_DevMngCallBackRegister.pfnDM_SetNTP != NULL)
        {
            ret = g_DevMngCallBackRegister.pfnDM_SetNTP(&stDM_SetNTP);
        }
        else
        {
            ret = GK_NVT_DevMng_SetNTP(&stDM_SetNTP);
        }
    	if(ret == ONVIF_OK)
    	{
    		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_SetNTP, NULL, NULL);
    	}
    }
    
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
    
}

GK_S32 soap_Device_GetNetworkInterfaces(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_GetNetworkInterfaces = soap_get_child_node(p_body, "GetNetworkInterfaces");
    if(p_GetNetworkInterfaces == NULL)
    {
        return -1;
    }

    GONVIF_DEVMNG_GetNetworkInterfaces_Res_S stDM_GetNetworkInterfacesRes;
    memset(&stDM_GetNetworkInterfacesRes, 0, sizeof(stDM_GetNetworkInterfacesRes));
    if(g_DevMngCallBackRegister.pfnDM_GetNetworkInterfaces != NULL)
    {
        ret = g_DevMngCallBackRegister.pfnDM_GetNetworkInterfaces(&stDM_GetNetworkInterfacesRes);
    }
    else
    {
        ret = GK_NVT_DevMng_GetNetworkInterfaces(&stDM_GetNetworkInterfacesRes);
    }
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetNetworkInterfaces, &stDM_GetNetworkInterfacesRes, NULL);
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Device_SetNetworkInterfaces(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_SetNetworkInterfaces = soap_get_child_node(p_body, "SetNetworkInterfaces");
    if(p_SetNetworkInterfaces == NULL)
    {
        return -1;
    }

	GONVIF_DEVMNG_SetNetworkInterfaces_S stDM_SetNetworkInterfaces;
    GONVIF_DEVMNG_SetNetworkInterfaces_Res_S stDN_SetNetworkInterfacesRes;
	memset(&stDM_SetNetworkInterfaces, 0, sizeof(stDM_SetNetworkInterfaces));
	memset(&stDN_SetNetworkInterfacesRes, 0, sizeof(stDN_SetNetworkInterfacesRes));
	ret = soap_parse_request_SetNetworkInterfaces(p_SetNetworkInterfaces, &stDM_SetNetworkInterfaces);
	if (ONVIF_OK == ret)
	{
        if(g_DevMngCallBackRegister.pfnDM_SetNetworkInterfaces != NULL)
        {
            ret = g_DevMngCallBackRegister.pfnDM_SetNetworkInterfaces(&stDM_SetNetworkInterfaces, &stDN_SetNetworkInterfacesRes);
        }
        else
        {
            ret = GK_NVT_DevMng_SetNetworkInterfaces(&stDM_SetNetworkInterfaces, &stDN_SetNetworkInterfacesRes);
        }

		if(ret == ONVIF_OK)
		{
			return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_SetNetworkInterfaces, &stDN_SetNetworkInterfacesRes, NULL);
		}
	}

	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Device_GetNetworkProtocols(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_GetNetworkProtocols = soap_get_child_node(p_body, "GetNetworkProtocols");
    if(p_GetNetworkProtocols == NULL)
    {
        return -1;
    }
    GONVIF_DEVMNG_GetNetworkProtocols_Res_S stDM_GetNetworkProtocolsRes;
    memset(&stDM_GetNetworkProtocolsRes, 0, sizeof(stDM_GetNetworkProtocolsRes));
    if(g_DevMngCallBackRegister.pfnDM_GetNetworkProtocols != NULL)
    {
        ret = g_DevMngCallBackRegister.pfnDM_GetNetworkProtocols(&stDM_GetNetworkProtocolsRes);
    }
    else
    {
        ret = GK_NVT_DevMng_GetNetworkProtocols(&stDM_GetNetworkProtocolsRes);
    }
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetNetworkProtocols, &stDM_GetNetworkProtocolsRes, NULL);
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Device_SetNetworkProtocols(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_SetNetworkProtocols = soap_get_child_node(p_body, "SetNetworkProtocols");
    if(p_SetNetworkProtocols == NULL)
    {
        return -1;
    }

	GONVIF_DEVMNG_SetNetworkProtocols_S stDM_SetNetworkProtocols;
	memset(&stDM_SetNetworkProtocols, 0, sizeof(stDM_SetNetworkProtocols));
	ret = soap_parse_request_SetNetworkProtocols(p_SetNetworkProtocols, &stDM_SetNetworkProtocols);
    if(ret == ONVIF_OK)
    {
        if(g_DevMngCallBackRegister.pfnDM_SetNetworkProtocols != NULL)
        {
            ret = g_DevMngCallBackRegister.pfnDM_SetNetworkProtocols(&stDM_SetNetworkProtocols);
        }
        else
        {
            ret = GK_NVT_DevMng_SetNetworkProtocols(&stDM_SetNetworkProtocols);
        }
    	if(ret == ONVIF_OK)
    	{
    		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_SetNetworkProtocols, NULL, NULL);
    	}
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Device_GetNetworkDefaultGateway(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_GetNetworkGateway = soap_get_child_node(p_body, "GetNetworkDefaultGateway");
    if(p_GetNetworkGateway == NULL)
    {
        return -1;
    }
    GONVIF_DEVMNG_GetNetworkDefaultGateway_Res_S stDM_GetNetworkDefaultGatewayRes;
    memset(&stDM_GetNetworkDefaultGatewayRes, 0, sizeof(stDM_GetNetworkDefaultGatewayRes));
    if(g_DevMngCallBackRegister.pfnDM_GetNetworkDefaultGateway != NULL)
    {
        ret = g_DevMngCallBackRegister.pfnDM_GetNetworkDefaultGateway(&stDM_GetNetworkDefaultGatewayRes);
    }
    else
    {
        ret = GK_NVT_DevMng_GetNetworkDefaultGateway(&stDM_GetNetworkDefaultGatewayRes);
    }

	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetNetworkDefaultGateway, &stDM_GetNetworkDefaultGatewayRes, NULL);
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);

}

GK_S32 soap_Device_SetNetworkDefaultGateway(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_SetNetworkDefaultGateway = soap_get_child_node(p_body, "SetNetworkDefaultGateway");
    if(p_SetNetworkDefaultGateway == NULL)
    {
        return -1;
    }

	GONVIF_DEVMNG_SetNetworkDefaultGateway_S stDM_SetNetworkDefaultGateway;
	memset(&stDM_SetNetworkDefaultGateway, 0, sizeof(stDM_SetNetworkDefaultGateway));

	ret = soap_parse_request_SetNetworkDefaultGateway(p_SetNetworkDefaultGateway, &stDM_SetNetworkDefaultGateway);
    if(ret == ONVIF_OK)
    {
        if(g_DevMngCallBackRegister.pfnDM_SetNetworkDefaultGateway != NULL)
        {
            ret = g_DevMngCallBackRegister.pfnDM_SetNetworkDefaultGateway(&stDM_SetNetworkDefaultGateway);
        }
        else
        {
            ret = GK_NVT_DevMng_SetNetworkDefaultGateway(&stDM_SetNetworkDefaultGateway);
        }
    	if(ret == ONVIF_OK)
    	{
    		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_SetNetworkDefaultGateway, NULL, NULL);
    	}
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}


GK_S32 soap_Device_GetSystemDateAndTime(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_GetSystemDateAndTime = soap_get_child_node(p_body, "GetSystemDateAndTime");
    if(p_GetSystemDateAndTime == NULL)
    {
        return -1;
    }
    GONVIF_DEVMNG_GetSystemDateAndTime_Res_S stDM_GetSystemDateAndTimeRes;
    memset(&stDM_GetSystemDateAndTimeRes, 0, sizeof(stDM_GetSystemDateAndTimeRes));
    if(g_DevMngCallBackRegister.pfnDM_GetSystemDateAndTime != NULL)
    {
        ret = g_DevMngCallBackRegister.pfnDM_GetSystemDateAndTime(&stDM_GetSystemDateAndTimeRes);
    }
    else
    {
        ret = GK_NVT_DevMng_GetSystemDateAndTime(&stDM_GetSystemDateAndTimeRes);
    }
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetSystemDateAndTime, &stDM_GetSystemDateAndTimeRes, NULL);
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);

}

GK_S32 soap_Device_SetSystemDateAndTime(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_SetSystemDateAndTime = soap_get_child_node(p_body, "SetSystemDateAndTime");
    if(p_SetSystemDateAndTime == NULL)
    {
        return -1;
    }

	GONVIF_DEVMNG_SetSystemDateAndTime_S stDM_SetSystemDateAndTime;
	memset(&stDM_SetSystemDateAndTime, 0, sizeof(stDM_SetSystemDateAndTime));
	ret = soap_parse_request_SetSystemDateAndTime(p_SetSystemDateAndTime, &stDM_SetSystemDateAndTime);
    if(ret == ONVIF_OK)
    {
        if(g_DevMngCallBackRegister.pfnDM_GetSystemDateAndTime != NULL)
        {
            ret = g_DevMngCallBackRegister.pfnDM_SetSystemDateAndTime(&stDM_SetSystemDateAndTime);
        }
        else
        {
            ret = GK_NVT_DevMng_SetSystemDateAndTime(&stDM_SetSystemDateAndTime);
        }
        if(ret == ONVIF_OK)
        {
        	return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_SetSystemDateAndTime, NULL, NULL);
        }
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}


GK_S32 soap_Device_GetDeviceInformation(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN * p_GetDeviceInformation = soap_get_child_node(p_body, "GetDeviceInformation");
    if(p_GetDeviceInformation == NULL)
    {
        return -1;
    }
    GONVIF_DEVMNG_GetDeviceInformation_Res_S stDM_GetDeviceInformationRes;
	memset(&stDM_GetDeviceInformationRes, 0, sizeof(stDM_GetDeviceInformationRes));
    if(g_DevMngCallBackRegister.pfnDM_GetDeviceInformation != NULL)
    {
        ret = g_DevMngCallBackRegister.pfnDM_GetDeviceInformation(&stDM_GetDeviceInformationRes);
    }
    else
    {
        ret = GK_NVT_DevMng_GetDeviceInformation(&stDM_GetDeviceInformationRes);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetDeviceInformation, &stDM_GetDeviceInformationRes, NULL);
    }
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Device_GetScopes(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GK_S32 retVal = 0;
    XMLN *p_GetScopes = soap_get_child_node(p_body, "GetScopes");
    if(p_GetScopes == NULL)
    {
        return -1;
    }
    
    GONVIF_DEVMNG_GetScopes_Res_S stDM_GetScopesRes;
    memset(&stDM_GetScopesRes, 0, sizeof(stDM_GetScopesRes));
    if(g_DevMngCallBackRegister.pfnDM_GetScopes != NULL)
    {
        ret = g_DevMngCallBackRegister.pfnDM_GetScopes(&stDM_GetScopesRes);
    }
    else
    {
        ret = GK_NVT_DevMng_GetScopes(&stDM_GetScopesRes);
    }
    if(ret == ONVIF_OK)
    {
        retVal = soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetScopes, &stDM_GetScopesRes, NULL);
        free(stDM_GetScopesRes.pstScopes);
        stDM_GetScopesRes.pstScopes = NULL;
        return retVal;
    }
    free(stDM_GetScopesRes.pstScopes);
    stDM_GetScopesRes.pstScopes = NULL;
    
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Device_SetScopes(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GK_S32 retVal = 0;
    GK_S32  i = 0; 
    XMLN *p_SetScopes = soap_get_child_node(p_body, "SetScopes");
    if(p_SetScopes == NULL)
    {
        return -1;
    }
    
    GONVIF_DEVMNG_SetScopes_S stDM_SetScopes;
    memset(&stDM_SetScopes, 0, sizeof(stDM_SetScopes));
    ret = soap_parse_request_SetScopes(p_SetScopes, &stDM_SetScopes);
    if(ret == ONVIF_OK)
    {
        if(g_DevMngCallBackRegister.pfnDM_SetScopes != NULL)
        {
            ret = g_DevMngCallBackRegister.pfnDM_SetScopes(&stDM_SetScopes);
        }
        else
        {
            ret = GK_NVT_DevMng_SetScopes(&stDM_SetScopes);
        }
        if(ret == ONVIF_OK)
        {
    		retVal = soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_SetScopes, NULL, NULL);
            while(i < stDM_SetScopes.sizeScopes && stDM_SetScopes.pszScopes[i] != NULL)
            {
                free(stDM_SetScopes.pszScopes[i]);
                stDM_SetScopes.pszScopes[i] = NULL;
                i++;
            }
            free(stDM_SetScopes.pszScopes);
            stDM_SetScopes.pszScopes = NULL;

            return retVal;
    	}
    }
    while(i < stDM_SetScopes.sizeScopes && stDM_SetScopes.pszScopes[i] != NULL)
    {
        free(stDM_SetScopes.pszScopes[i]);
        stDM_SetScopes.pszScopes[i] = NULL;
        i++;
    }
    free(stDM_SetScopes.pszScopes);
    stDM_SetScopes.pszScopes = NULL;
    
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Device_AddScopes(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GK_S32 retVal = 0;
    GK_S32 i = 0;
    XMLN *p_AddScopes = soap_get_child_node(p_body, "AddScopes");
    if(p_AddScopes == NULL)
    {
        return -1;
    }

    GONVIF_DEVMNG_AddScopes_S stDM_AddScopes;
    memset(&stDM_AddScopes, 0, sizeof(stDM_AddScopes));
    ret = soap_parse_request_AddScopes(p_AddScopes, &stDM_AddScopes);
    if(ret == ONVIF_OK)
    {
        if(g_DevMngCallBackRegister.pfnDM_AddScopes != NULL)
        {
            ret = g_DevMngCallBackRegister.pfnDM_AddScopes(&stDM_AddScopes);
        }
        else
        {
            ret = GK_NVT_DevMng_AddScopes(&stDM_AddScopes);
        }
        if(ret == ONVIF_OK)
        {
    		retVal = soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_AddScopes, NULL, NULL);
            while(i < stDM_AddScopes.sizeScopeItem && stDM_AddScopes.pszScopeItem[i] != NULL)
            {
                free(stDM_AddScopes.pszScopeItem[i]);
                stDM_AddScopes.pszScopeItem[i] = NULL;
                i++;
            }
            free(stDM_AddScopes.pszScopeItem);
            stDM_AddScopes.pszScopeItem = NULL;

            return retVal;
    	}
    }
    while(i < stDM_AddScopes.sizeScopeItem && stDM_AddScopes.pszScopeItem[i] != NULL)
    {
        free(stDM_AddScopes.pszScopeItem[i]);
        stDM_AddScopes.pszScopeItem[i] = NULL;
        i++;
    }
    free(stDM_AddScopes.pszScopeItem);
    stDM_AddScopes.pszScopeItem = NULL;
    
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Device_RemoveScopes(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;    
    GK_S32 retVal = 0;
    GK_S32 i = 0;
    XMLN *p_RemoveScopes = soap_get_child_node(p_body, "RemoveScopes");
    if(p_RemoveScopes == NULL)
    {
        return -1;
    }
	GONVIF_DEVMNG_RemoveScopes_S stDM_RemoveScopes;
	GONVIF_DEVMNG_RemoveScopes_Res_S stDM_RemoveScopesRes;
    memset(&stDM_RemoveScopes, 0, sizeof(stDM_RemoveScopes));
    memset(&stDM_RemoveScopesRes, 0, sizeof(stDM_RemoveScopesRes));
    ret = soap_parse_request_RemoveScopes(p_RemoveScopes, &stDM_RemoveScopes);
    if (ret == ONVIF_OK)
    {
        if(g_DevMngCallBackRegister.pfnDM_RemoveScopes != NULL)
        {
            ret = g_DevMngCallBackRegister.pfnDM_RemoveScopes(&stDM_RemoveScopes, &stDM_RemoveScopesRes);
        }
        else
        {
            ret = GK_NVT_DevMng_RemoveScopes(&stDM_RemoveScopes, &stDM_RemoveScopesRes);
        }
        if(ret == ONVIF_OK)
        {
    		retVal = soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_RemoveScopes, &stDM_RemoveScopesRes, NULL);
            while(i < stDM_RemoveScopes.sizeScopeItem && stDM_RemoveScopes.pszScopeItem[i] != NULL)
            {
                free(stDM_RemoveScopes.pszScopeItem[i]);
                stDM_RemoveScopes.pszScopeItem[i] = NULL;
                i++;
            }
            free(stDM_RemoveScopes.pszScopeItem);
            stDM_RemoveScopes.pszScopeItem = NULL;

            return retVal;
    	}
    }
    while(i < stDM_RemoveScopes.sizeScopeItem && stDM_RemoveScopes.pszScopeItem[i] != NULL)
    {
        free(stDM_RemoveScopes.pszScopeItem[i]);
        stDM_RemoveScopes.pszScopeItem[i] = NULL;
        i++;
    }
    free(stDM_RemoveScopes.pszScopeItem);
    stDM_RemoveScopes.pszScopeItem = NULL;
    
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}


GK_S32 soap_Device_GetDiscoveryMode(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetDiscoveryMode = soap_get_child_node(p_body, "GetDiscoveryMode");
    if(p_GetDiscoveryMode == NULL)
    {
        return -1;
    }
	GONIVF_DEVMNG_GetDiscoveryMode_Res_S stDM_GetDiscoveryModeRes;
	memset(&stDM_GetDiscoveryModeRes, 0, sizeof(GONIVF_DEVMNG_GetDiscoveryMode_Res_S));
    if(g_DevMngCallBackRegister.pfnDM_GetDiscoveryMode != NULL)
    {
        ret = g_DevMngCallBackRegister.pfnDM_GetDiscoveryMode(&stDM_GetDiscoveryModeRes);
    }
    else
    {
        ret = GK_NVT_DevMng_GetDiscoveryMode(&stDM_GetDiscoveryModeRes);
    }
    if (ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetDiscoveryMode, &stDM_GetDiscoveryModeRes, NULL);
    }    
    ONVIF_ERR();
	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Device_SetDiscoveryMode(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_SetDiscoveryMode = soap_get_child_node(p_body, "SetDiscoveryMode");
    if(p_SetDiscoveryMode == NULL)
    {
        return -1;
    }

	GONIVF_DEVMNG_SetDiscoveryMode_S stDM_SetDiscoveryMode;
	memset(&stDM_SetDiscoveryMode, 0, sizeof(GONIVF_DEVMNG_SetDiscoveryMode_S));
	ret = soap_parse_request_SetDiscoveryMode(p_SetDiscoveryMode, &stDM_SetDiscoveryMode);
	if(ret == ONVIF_OK)
	{

        if(g_DevMngCallBackRegister.pfnDM_GetDiscoveryMode != NULL)
        {
            ret = g_DevMngCallBackRegister.pfnDM_SetDiscoveryMode(&stDM_SetDiscoveryMode);
        }
        else
        {
    	    ret = GK_NVT_DevMng_SetDiscoveryMode(&stDM_SetDiscoveryMode);
        }
	    if(ret == ONVIF_OK)
	    {
			return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_SetDiscoveryMode, NULL, NULL);
		}
	}

	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Device_GetUsers(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN * p_GetUsers = soap_get_child_node(p_body, "GetUsers");
    if(p_GetUsers == NULL)
    {
        return -1;
    }
    
    GONVIF_DEVMNG_GetUsers_Res_S stDM_GetUsersRes;
	memset(&stDM_GetUsersRes, 0, sizeof(stDM_GetUsersRes));
    if(g_DevMngCallBackRegister.pfnDM_GetUsers != NULL)
    {
        ret = g_DevMngCallBackRegister.pfnDM_GetUsers(&stDM_GetUsersRes);
    }
    else
    {
        ret = GK_NVT_DevMng_GetUsers(&stDM_GetUsersRes);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetUsers, &stDM_GetUsersRes, NULL);
    }
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Device_CreateUsers(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_CreateUsers = soap_get_child_node(p_body, "CreateUsers");
    if(p_CreateUsers == NULL)
    {
        return -1;
    }
    GONVIF_DEVMNG_CreateUsers_S stDM_CreateUsers;
	memset(&stDM_CreateUsers, 0, sizeof(stDM_CreateUsers));
	ret = soap_parse_request_CreateUsers(p_CreateUsers, &stDM_CreateUsers);
    if (ret == ONVIF_OK)
    {
        if(g_DevMngCallBackRegister.pfnDM_CreateUsers != NULL)
        {
            ret = g_DevMngCallBackRegister.pfnDM_CreateUsers(&stDM_CreateUsers);
        }
        else
        {
            ret = GK_NVT_DevMng_CreateUsers(&stDM_CreateUsers);
        }
        if(ret == ONVIF_OK)
        {
			return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_CreateUsers, NULL, NULL);
        }

    }
    
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Device_DeleteUsers(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_DeleteUsers = soap_get_child_node(p_body, "DeleteUsers");
    if(p_DeleteUsers == NULL)
    {
        return -1;
    }
    GONVIF_DEVMNG_DeleteUsers_S stDM_DeleteUsers;
	memset(&stDM_DeleteUsers, 0, sizeof(stDM_DeleteUsers));

	ret = soap_parse_request_DeleteUsers(p_DeleteUsers, &stDM_DeleteUsers);
    if(ret == ONVIF_OK)
    {
        if(g_DevMngCallBackRegister.pfnDM_DeleteUsers != NULL)
        {
            ret = g_DevMngCallBackRegister.pfnDM_DeleteUsers(&stDM_DeleteUsers);
        }
        else
        {
            ret = GK_NVT_DevMng_DeleteUsers(&stDM_DeleteUsers);
        }
        if(ret == ONVIF_OK)
        {
            return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_DeleteUsers, NULL, NULL);
        }
    }
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Device_SetUser(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    //GK_S32 i = 0;
	XMLN *p_SetUser = soap_get_child_node(p_body, "SetUser");
    if(p_SetUser == NULL)
    {
        return -1;
    }
    GONVIF_DEVMNG_SetUser_S stDM_SetUser;
	memset(&stDM_SetUser, 0, sizeof(stDM_SetUser));
	ret = soap_parse_request_SetUser(p_SetUser, &stDM_SetUser);
    if(ret == ONVIF_OK)
    {
        if(g_DevMngCallBackRegister.pfnDM_SetUser != NULL)
        {
            ret = g_DevMngCallBackRegister.pfnDM_SetUser(&stDM_SetUser);
        }
        else
        {
            ret = GK_NVT_DevMng_SetUser(&stDM_SetUser);
        }
        if(ret == ONVIF_OK)
        {
            return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_SetUser, NULL, NULL);
        }

    }
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Device_SetSystemFactoryDefault(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_SetSystemFactoryDefault = soap_get_child_node(p_body, "SetSystemFactoryDefault");
    if(p_SetSystemFactoryDefault == NULL)
    {
        return -1;
    }
	GONVIF_DEVMNG_SetSystemFactoryDefault_S stDM_SetSystemFactoryDefault;
	memset(&stDM_SetSystemFactoryDefault, 0, sizeof(stDM_SetSystemFactoryDefault));
	ret = soap_parse_request_SetSystemFactoryDefault(p_SetSystemFactoryDefault, &stDM_SetSystemFactoryDefault);
    if(ret == ONVIF_OK)
    {
        if(g_DevMngCallBackRegister.pfnDM_SetSystemFactoryDefault != NULL)
        {
            ret = g_DevMngCallBackRegister.pfnDM_SetSystemFactoryDefault(&stDM_SetSystemFactoryDefault);
        }
        else
        {
            ret = GK_NVT_DevMng_SetSystemFactoryDefault(&stDM_SetSystemFactoryDefault);
        }
        if(ret == ONVIF_OK)
        {
        	return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_SetSystemFactoryDefault, NULL, NULL);
        }
    }
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

/******************************** device end *************************************/

/********************************* deviceIO **************************************/
GK_S32 soap_DeviceIO_GetVideoSource(HTTPCLN * p_cln, HTTPMSG * rx_msg, XMLN * p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetVideoSources = soap_get_child_node(p_body, "GetVideoSources");
    if(p_GetVideoSources == NULL)
    {
        return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
    }

    GONVIF_DEVICEIO_GetVideoSources_Res_S stIO_GetVideoSourcesRes;
    memset(&stIO_GetVideoSourcesRes, 0, sizeof(stIO_GetVideoSourcesRes));
    if(g_DeviceIOCallBackRegister.pfnIO_GetVideoSources != NULL)
    {
        ret = g_DeviceIOCallBackRegister.pfnIO_GetVideoSources(&stIO_GetVideoSourcesRes);
    }
    else
    {
        ret = GK_NVT_DeviceIO_GetVideoSources(&stIO_GetVideoSourcesRes);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetVideoSources, &stIO_GetVideoSourcesRes, NULL);
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);

}

GK_S32 soap_DeviceIO_GetAudioSource(HTTPCLN * p_cln, HTTPMSG * rx_msg, XMLN * p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetAudioSources = soap_get_child_node(p_body, "GetAudioSources");
    if(p_GetAudioSources == NULL)
    {
        return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
    }

    GONVIF_DEVICEIO_GetAudioSources_Res_S stIO_GetAudioSourcesRes;
    memset(&stIO_GetAudioSourcesRes, 0, sizeof(stIO_GetAudioSourcesRes));
    if(g_DeviceIOCallBackRegister.pfnIO_GetAudioSources != NULL)
    {
        ret = g_DeviceIOCallBackRegister.pfnIO_GetAudioSources(&stIO_GetAudioSourcesRes);
    }
    else
    {
        ret = GK_NVT_DeviceIO_GetAudioSources(&stIO_GetAudioSourcesRes);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetAudioSources, &stIO_GetAudioSourcesRes, NULL);
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);

}

GK_S32 soap_DeviceIO_GetRelayOutputs(HTTPCLN * p_cln, HTTPMSG * rx_msg, XMLN * p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetRelayOutputs = soap_get_child_node(p_body, "GetRelayOutputs");
    if(p_GetRelayOutputs == NULL)
    {
        return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
    }

    GONVIF_DEVICEIO_GetRelayOutputs_Res_S stIO_GetRelayOutputsRes;
    memset(&stIO_GetRelayOutputsRes, 0, sizeof(stIO_GetRelayOutputsRes));
    if(g_DeviceIOCallBackRegister.pfnIO_GetRelayOutputs != NULL)
    {
        ret = g_DeviceIOCallBackRegister.pfnIO_GetRelayOutputs(&stIO_GetRelayOutputsRes);
    }
    else
    {
        ret = GK_NVT_DeviceIO_GetRelayOutputs(&stIO_GetRelayOutputsRes);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetRelayOutputs, &stIO_GetRelayOutputsRes, NULL);
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);

}

GK_S32 soap_DeviceIO_SetRelayOutputSettings(HTTPCLN * p_cln, HTTPMSG * rx_msg, XMLN * p_body)
{
    ONVIF_RET ret = ONVIF_OK;

    GONVIF_DEVICEIO_SetRelayOutputSettings_S stIO_SetRelayOutputSettings;
    memset(&stIO_SetRelayOutputSettings, 0, sizeof(stIO_SetRelayOutputSettings));
    ret = soap_parse_request_SetRelayOutputSettings(p_body, &stIO_SetRelayOutputSettings);
    if(g_DeviceIOCallBackRegister.pfnIO_SetRelayOutputSettings != NULL)
    {
        ret = g_DeviceIOCallBackRegister.pfnIO_SetRelayOutputSettings(&stIO_SetRelayOutputSettings);
    }
    else
    {
        ret = GK_NVT_DeviceIO_SetRelayOutputSettings(&stIO_SetRelayOutputSettings);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_SetRelayOutputSettings, NULL, NULL);
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);

}

GK_S32 soap_DeviceIO_SetRelayOutputState(HTTPCLN * p_cln, HTTPMSG * rx_msg, XMLN * p_body)
{
    ONVIF_RET ret = ONVIF_OK;

    GONVIF_DEVICEIO_SetRelayOutputState_S stIO_SetRelayOutputState;
    memset(&stIO_SetRelayOutputState, 0, sizeof(stIO_SetRelayOutputState));
    ret = soap_parse_request_SetRelayOutputState(p_body, &stIO_SetRelayOutputState);
    if(g_DeviceIOCallBackRegister.pfnIO_SetRelayOutputState != NULL)
    {
        ret = g_DeviceIOCallBackRegister.pfnIO_SetRelayOutputState(&stIO_SetRelayOutputState);
    }
    else
    {
        ret = GK_NVT_DeviceIO_SetRelayOutputState(&stIO_SetRelayOutputState);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_SetRelayOutputState, NULL, NULL);
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);

}

/******************************** deviceIO end ************************************/

/*********************************** meida **************************************/

GK_S32 soap_Media_GetServiceCapabilities(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetServiceCapabilities = soap_get_child_node(p_body, "GetServiceCapabilities");
    if(p_GetServiceCapabilities == NULL)
    {
        return -1;
    }
	GONVIF_MEDIA_GetServiceCapabilities_Res_S stMD_GetServiceCapabilitiesRes;
	memset(&stMD_GetServiceCapabilitiesRes, 0, sizeof(stMD_GetServiceCapabilitiesRes));
	if(g_MediaCallBackRegister.pfnMD_GetServiceCapabilities != NULL)
	{
		ret = g_MediaCallBackRegister.pfnMD_GetServiceCapabilities(&stMD_GetServiceCapabilitiesRes);
	}
	else
	{
		ret = GK_NVT_Media_GetServiceCapabilities(&stMD_GetServiceCapabilitiesRes);
	}
	if(ret == ONVIF_OK)
	{
		return	soap_proc_build_send_rly(p_cln, rx_msg, soap_response_Media_GetServiceCapabilities, &stMD_GetServiceCapabilitiesRes, NULL);
	}

	
	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_CreateProfile(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_CreateProfile = soap_get_child_node(p_body, "CreateProfile");
    if(p_CreateProfile == NULL)
    {
        return -1;
    }
    GONVIF_MEDIA_CreateProfile_S stMD_CreateProfile;
    GONVIF_MEDIA_CreateProfile_Res_S stMD_CreateProfileRes;
    memset(&stMD_CreateProfile, 0, sizeof(stMD_CreateProfile));
    memset(&stMD_CreateProfileRes, 0, sizeof(stMD_CreateProfileRes));
    ret = soap_parse_request_CreateProfile(p_CreateProfile, &stMD_CreateProfile);
    if(ret == ONVIF_OK)
    {
        if(g_MediaCallBackRegister.pfnMD_CreateProfile != NULL)
        {
            ret = g_MediaCallBackRegister.pfnMD_CreateProfile(&stMD_CreateProfile, &stMD_CreateProfileRes);
        }
        else
        {
            ret = GK_NVT_Media_CreateProfile(&stMD_CreateProfile, &stMD_CreateProfileRes);
        }
    	if(ret == ONVIF_OK)
    	{
    		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_CreateProfile, &stMD_CreateProfileRes, NULL);
    	}
    }
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_GetProfile(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetProfile = soap_get_child_node(p_body, "GetProfile");
    if(p_GetProfile == NULL)
    {
        return -1;
    }
    GONVIF_MEDIA_GetProfile_S stMD_GetProfile;
    GONVIF_MEDIA_GetProfile_Res_S stMD_GetProfileRes;
    XMLN *p_ProfileToken = soap_get_child_node(p_GetProfile, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
	    strncpy(stMD_GetProfile.aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
	}
    if(g_MediaCallBackRegister.pfnMD_GetProfile != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetProfile(&stMD_GetProfile, &stMD_GetProfileRes);
    }
    else
    {
        ret = GK_NVT_Media_GetProfile(&stMD_GetProfile, &stMD_GetProfileRes);
    }
    if (ONVIF_OK == ret)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetProfile, &stMD_GetProfileRes, NULL);
    }
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_GetProfiles(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetProfiles = soap_get_child_node(p_body, "GetProfiles");
    if(p_GetProfiles == NULL)
    {
        return -1;
    }
    GONVIF_MEDIA_GetProfiles_Res_S stMD_GetProfilesRes;
    memset(&stMD_GetProfilesRes, 0 ,sizeof(stMD_GetProfilesRes));
    if(g_MediaCallBackRegister.pfnMD_GetProfiles != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetProfiles(&stMD_GetProfilesRes);
    }
    else
    {
    	//ONVIF_INFO("===========>enter GK_NVT_Media_GetProfiles\n");
        ret = GK_NVT_Media_GetProfiles(&stMD_GetProfilesRes);        
    	//ONVIF_INFO("===========>goout GK_NVT_Media_GetProfiles\n");
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetProfiles, &stMD_GetProfilesRes, NULL);
    }
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_DeleteProfile(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_DeleteProfile = soap_get_child_node(p_body, "DeleteProfile");
    if(p_DeleteProfile == NULL)
    {
        return -1;
    }
	GONVIF_MEDIA_DeleteProfile_S stMD_DeleteProfile;
    memset(&stMD_DeleteProfile, 0, sizeof(stMD_DeleteProfile));
    XMLN *p_ProfileToken = soap_get_child_node(p_DeleteProfile, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
        strncpy(stMD_DeleteProfile.aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
    }
    if(g_MediaCallBackRegister.pfnMD_DeleteProfile != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_DeleteProfile(&stMD_DeleteProfile);
    }
    else
    {
        ret = GK_NVT_Media_DeleteProfile(&stMD_DeleteProfile);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_DeleteProfile, NULL, NULL);
    }
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}


GK_S32 soap_Media_AddVideoSourceConfiguration(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_AddVideoSourceConfiguration = soap_get_child_node(p_body, "AddVideoSourceConfiguration");
    if(p_AddVideoSourceConfiguration == NULL)
    {
        return -1;
    }

	GONVIF_MEDIA_AddVideoSourceConfiguration_S stMD_AddVideoSourceConfiguration;
	memset(&stMD_AddVideoSourceConfiguration, 0, sizeof(stMD_AddVideoSourceConfiguration));
	XMLN *p_ProfileToken = soap_get_child_node(p_AddVideoSourceConfiguration, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(stMD_AddVideoSourceConfiguration.aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
	}
	XMLN *p_ConfigurationToken = soap_get_child_node(p_AddVideoSourceConfiguration, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(stMD_AddVideoSourceConfiguration.aszConfigurationToken, p_ConfigurationToken->data, MAX_TOKEN_LENGTH - 1);
	}

    if(g_MediaCallBackRegister.pfnMD_AddVideoSourceConfiguration != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_AddVideoSourceConfiguration(&stMD_AddVideoSourceConfiguration);
    }
    else
    {
        ret = GK_NVT_Media_AddVideoSourceConfiguration(&stMD_AddVideoSourceConfiguration);
    }

	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_AddVideoSourceConfiguration, NULL, NULL);
	}

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_AddVideoEncoderConfiguration(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN * p_AddVideoEncoderConfiguration = soap_get_child_node(p_body, "AddVideoEncoderConfiguration");
	if(p_AddVideoEncoderConfiguration == NULL)
    {
        return -1;
    }

	GONVIF_MEDIA_AddVideoEncoderConfiguration_S stMD_AddVideoEncoderConfiguration;
	memset(&stMD_AddVideoEncoderConfiguration, 0, sizeof(stMD_AddVideoEncoderConfiguration));
	XMLN * p_ProfileToken = soap_get_child_node(p_AddVideoEncoderConfiguration, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(stMD_AddVideoEncoderConfiguration.aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
	}
	XMLN * p_ConfigurationToken = soap_get_child_node(p_AddVideoEncoderConfiguration, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(stMD_AddVideoEncoderConfiguration.aszConfigurationToken, p_ConfigurationToken->data, MAX_TOKEN_LENGTH - 1);
	}
    if(g_MediaCallBackRegister.pfnMD_AddVideoEncoderConfiguration != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_AddVideoEncoderConfiguration(&stMD_AddVideoEncoderConfiguration);
    }
    else
    {
        ret = GK_NVT_Media_AddVideoEncoderConfiguration(&stMD_AddVideoEncoderConfiguration);
    }
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_AddVideoEncoderConfiguration, NULL, NULL);
	}

	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_AddAudioSourceConfiguration(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_AddAudioSourceConfiguration = soap_get_child_node(p_body, "AddAudioSourceConfiguration");
    if(p_AddAudioSourceConfiguration == NULL)
    {
        return -1;
    }

	GONVIF_MEDIA_AddAudioSourceConfiguration_S stMD_AddAudioSourceConfiguration;
	memset(&stMD_AddAudioSourceConfiguration, 0, sizeof(stMD_AddAudioSourceConfiguration));
	XMLN *p_ProfileToken = soap_get_child_node(p_AddAudioSourceConfiguration, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(stMD_AddAudioSourceConfiguration.aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
	}
	XMLN *p_ConfigurationToken = soap_get_child_node(p_AddAudioSourceConfiguration, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(stMD_AddAudioSourceConfiguration.aszConfigurationToken, p_ConfigurationToken->data, MAX_TOKEN_LENGTH - 1);
	}

    if(g_MediaCallBackRegister.pfnMD_AddAudioSourceConfiguration != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_AddAudioSourceConfiguration(&stMD_AddAudioSourceConfiguration);
    }
    else
    {
        ret = GK_NVT_Media_AddAudioSourceConfiguration(&stMD_AddAudioSourceConfiguration);
    }

	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_AddAudioSourceConfiguration, NULL, NULL);
	}

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}


GK_S32 soap_Media_AddAudioEncoderConfiguration(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_AddAudioEncoderConfiguration = soap_get_child_node(p_body, "AddAudioEncoderConfiguration");
	if(p_AddAudioEncoderConfiguration == NULL)
    {
        return -1;
    }

	GONVIF_MEDIA_AddAudioEncoderConfiguration_S stMD_AddAudioEncoderConfiguration;
	memset(&stMD_AddAudioEncoderConfiguration, 0, sizeof(stMD_AddAudioEncoderConfiguration));
	XMLN * p_ProfileToken = soap_get_child_node(p_AddAudioEncoderConfiguration, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(stMD_AddAudioEncoderConfiguration.aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
	}
	XMLN * p_ConfigurationToken = soap_get_child_node(p_AddAudioEncoderConfiguration, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(stMD_AddAudioEncoderConfiguration.aszConfigurationToken, p_ConfigurationToken->data, MAX_TOKEN_LENGTH - 1);
	}
    if(g_MediaCallBackRegister.pfnMD_AddAudioEncoderConfiguration != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_AddAudioEncoderConfiguration(&stMD_AddAudioEncoderConfiguration);
    }
    else
    {
        ret = GK_NVT_Media_AddAudioEncoderConfiguration(&stMD_AddAudioEncoderConfiguration);
    }
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_AddAudioEncoderConfiguration, NULL, NULL);
	}

	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}


GK_S32 soap_Media_AddPTZConfiguration(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_AddPTZConfiguration = soap_get_child_node(p_body, "AddPTZConfiguration");
	if(p_AddPTZConfiguration == NULL)
    {
        return -1;
    }

	GONVIF_MEDIA_AddPTZConfiguration_S stMD_AddPTZConfiguration;
	memset(&stMD_AddPTZConfiguration, 0, sizeof(stMD_AddPTZConfiguration));
	XMLN *p_ProfileToken = soap_get_child_node(p_AddPTZConfiguration, "ProfileToken");
	if(p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(stMD_AddPTZConfiguration.aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
	}

	XMLN *p_ConfigurationToken = soap_get_child_node(p_AddPTZConfiguration, "ConfigurationToken");
	if(p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(stMD_AddPTZConfiguration.aszConfigurationToken, p_ConfigurationToken->data, MAX_TOKEN_LENGTH - 1);
	}
    if(g_MediaCallBackRegister.pfnMD_AddPTZConfiguration != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_AddPTZConfiguration(&stMD_AddPTZConfiguration);
    }
    else
    {
        ret = GK_NVT_Media_AddPTZConfiguration(&stMD_AddPTZConfiguration);
    }
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_AddPTZConfiguration, NULL, NULL);
	}

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_RemoveVideoSourceConfiguration(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN * p_RemoveVideoSourceConfiguration = soap_get_child_node(p_body, "RemoveVideoSourceConfiguration");
    if(p_RemoveVideoSourceConfiguration == NULL)
    {
        return -1;
    }
    GONVIF_MEDIA_RemoveVideoSourceConfiguration_S stMD_RemoveVideoSourceConfiguration;
    memset(&stMD_RemoveVideoSourceConfiguration, 0, sizeof(stMD_RemoveVideoSourceConfiguration));


	XMLN *p_ProfileToken = soap_get_child_node(p_RemoveVideoSourceConfiguration, "ProfileToken");
    if(p_ProfileToken && p_ProfileToken->data)
    {
		strncpy(stMD_RemoveVideoSourceConfiguration.aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
    }
    if(g_MediaCallBackRegister.pfnMD_RemoveVideoSourceConfiguration != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_RemoveVideoSourceConfiguration(&stMD_RemoveVideoSourceConfiguration);
    }
    else
    {
        ret = GK_NVT_Media_RemoveVideoSourceConfiguration(&stMD_RemoveVideoSourceConfiguration);
    }
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_RemoveVideoSourceConfiguration, NULL, NULL);
	}

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}


GK_S32 soap_Media_RemoveVideoEncoderConfiguration(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_RemoveVideoEncoderConfiguration = soap_get_child_node(p_body, "RemoveVideoEncoderConfiguration");
	if(p_RemoveVideoEncoderConfiguration == NULL)
    {
        return -1;
    }
	GONVIF_MEDIA_RemoveVideoEncoderConfiguration_S stMD_RemoveVideoEncoderConfiguration;
	memset(&stMD_RemoveVideoEncoderConfiguration, 0, sizeof(stMD_RemoveVideoEncoderConfiguration));
	XMLN *p_ProfileToken = soap_get_child_node(p_RemoveVideoEncoderConfiguration, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
        strncpy(stMD_RemoveVideoEncoderConfiguration.aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
    }
    if(g_MediaCallBackRegister.pfnMD_RemoveVideoEncoderConfiguration != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_RemoveVideoEncoderConfiguration(&stMD_RemoveVideoEncoderConfiguration);
    }
    else
    {
        ret = GK_NVT_Media_RemoveVideoEncoderConfiguration(&stMD_RemoveVideoEncoderConfiguration);
    }
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_RemoveVideoEncoderConfiguration, NULL, NULL);
	}

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_RemoveAudioSourceConfiguration(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_RemoveAudioSourceConfiguration = soap_get_child_node(p_body, "RemoveAudioSourceConfiguration");
    if(p_RemoveAudioSourceConfiguration == NULL)
    {
        return -1;
    }
    GONVIF_MEDIA_RemoveAudioSourceConfiguration_S stMD_RemoveAudioSourceConfiguration;
    memset(&stMD_RemoveAudioSourceConfiguration, 0, sizeof(stMD_RemoveAudioSourceConfiguration));


	XMLN *p_ProfileToken = soap_get_child_node(p_RemoveAudioSourceConfiguration, "ProfileToken");
    if(p_ProfileToken && p_ProfileToken->data)
    {
		strncpy(stMD_RemoveAudioSourceConfiguration.aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
    }
    if(g_MediaCallBackRegister.pfnMD_RemoveAudioSourceConfiguration != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_RemoveAudioSourceConfiguration(&stMD_RemoveAudioSourceConfiguration);
    }
    else
    {
        ret = GK_NVT_Media_RemoveAudioSourceConfiguration(&stMD_RemoveAudioSourceConfiguration);
    }
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_RemoveAudioSourceConfiguration, NULL, NULL);
	}

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_RemoveAudioEncoderConfiguration(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_RemoveAudioEncoderConfiguration = soap_get_child_node(p_body, "RemoveAudioEncoderConfiguration");
	if(p_RemoveAudioEncoderConfiguration == NULL)
    {
        return -1;
    }
	GONVIF_MEDIA_RemoveAudioEncoderConfiguration_S stMD_RemoveAudioEncoderConfiguration;
	memset(&stMD_RemoveAudioEncoderConfiguration, 0, sizeof(stMD_RemoveAudioEncoderConfiguration));
	XMLN *p_ProfileToken = soap_get_child_node(p_RemoveAudioEncoderConfiguration, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
        strncpy(stMD_RemoveAudioEncoderConfiguration.aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
    }
    if(g_MediaCallBackRegister.pfnMD_RemoveAudioEncoderConfiguration != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_RemoveAudioEncoderConfiguration(&stMD_RemoveAudioEncoderConfiguration);
    }
    else
    {
        ret = GK_NVT_Media_RemoveAudioEncoderConfiguration(&stMD_RemoveAudioEncoderConfiguration);
    }
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_RemoveAudioEncoderConfiguration, NULL, NULL);
	}

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_RemovePTZConfiguration(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_RemovePTZConfiguration = soap_get_child_node(p_body, "RemovePTZConfiguration");
	if(p_RemovePTZConfiguration == NULL)
    {
        return -1;
    }
    GONVIF_MEDIA_RemovePTZConfiguration_S stMD_RemovePTZConfiguration;
	XMLN *p_ProfileToken = soap_get_child_node(p_RemovePTZConfiguration, "ProfileToken");
    if (p_ProfileToken && p_ProfileToken->data)
    {
		strncpy(stMD_RemovePTZConfiguration.aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
    }

    if(g_MediaCallBackRegister.pfnMD_RemovePTZConfiguration != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_RemovePTZConfiguration(&stMD_RemovePTZConfiguration);
    }
    else
    {
        ret = GK_NVT_Media_RemovePTZConfiguration(&stMD_RemovePTZConfiguration);
    }
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_RemovePTZConfiguration, NULL, NULL);
	}
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_GetVideoSourceConfigurations(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetVideoSourceConfigurations = soap_get_child_node(p_body, "GetVideoSourceConfigurations");
    if(p_GetVideoSourceConfigurations == NULL)
    {
        return -1;
    }

    GONVIF_MEDIA_GetVideoSourceConfigurations_Res_S stMD_GetVideoSourceConfigurationsRes;
    memset(&stMD_GetVideoSourceConfigurationsRes, 0, sizeof(stMD_GetVideoSourceConfigurationsRes));
    if(g_MediaCallBackRegister.pfnMD_GetVideoSourceConfigurations != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetVideoSourceConfigurations(&stMD_GetVideoSourceConfigurationsRes);
    }
    else
    {
        ret = GK_NVT_Media_GetVideoSourceConfigurations(&stMD_GetVideoSourceConfigurationsRes);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetVideoSourceConfigurations, &stMD_GetVideoSourceConfigurationsRes, NULL);
    }
    
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_GetVideoSourceConfiguration(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetVideoSourceConfiguration = soap_get_child_node(p_body, "GetVideoSourceConfiguration");
    if(p_GetVideoSourceConfiguration == NULL)
    {
        return -1;
    }
    GONVIF_MEDIA_GetVideoSourceConfiguration_S stMD_GetVideoSourceConfiguration;
    GONVIF_MEDIA_GetVideoSourceConfiguration_Res_S stMD_GetVideoSourceConfigurationRes;
    memset(&stMD_GetVideoSourceConfiguration, 0, sizeof(stMD_GetVideoSourceConfiguration));
    memset(&stMD_GetVideoSourceConfigurationRes, 0, sizeof(stMD_GetVideoSourceConfigurationRes));

	XMLN *p_ConfigurationToken = soap_get_child_node(p_GetVideoSourceConfiguration, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
        strncpy(stMD_GetVideoSourceConfiguration.aszConfigurationToken, p_ConfigurationToken->data, MAX_TOKEN_LENGTH - 1);
	}
    if(g_MediaCallBackRegister.pfnMD_GetVideoSourceConfiguration != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetVideoSourceConfiguration(&stMD_GetVideoSourceConfiguration, &stMD_GetVideoSourceConfigurationRes);
    }
    else
    {
        ret = GK_NVT_Media_GetVideoSourceConfiguration(&stMD_GetVideoSourceConfiguration, &stMD_GetVideoSourceConfigurationRes);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetVideoSourceConfiguration, &stMD_GetVideoSourceConfigurationRes, NULL);
    }
    
	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_GetCompatibleVideoSourceConfigurations(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetCompatibleVideoSourceConfigurations = soap_get_child_node(p_body, "GetCompatibleVideoSourceConfigurations");
    if(p_GetCompatibleVideoSourceConfigurations == NULL)
    {
        return -1;
    }
    GONVIF_MEDIA_GetCompatibleVideoSourceConfigurations_S stMD_GetCompatibleVideoSourceConfigurations;
    GONVIF_MEDIA_GetCompatibleVideoSourceConfigurations_Res_S stMD_GetCompatibleVideoSourceConfigurationsRes;
    memset(&stMD_GetCompatibleVideoSourceConfigurations, 0, sizeof(stMD_GetCompatibleVideoSourceConfigurations));
    memset(&stMD_GetCompatibleVideoSourceConfigurationsRes, 0, sizeof(stMD_GetCompatibleVideoSourceConfigurationsRes));
	XMLN *p_ProfileToken = soap_get_child_node(p_GetCompatibleVideoSourceConfigurations, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
	    strncpy(stMD_GetCompatibleVideoSourceConfigurations.aszProfileToken,p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
    }
    if(g_MediaCallBackRegister.pfnMD_GetCompatibleVideoSourceConfigurations != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetCompatibleVideoSourceConfigurations(&stMD_GetCompatibleVideoSourceConfigurations, &stMD_GetCompatibleVideoSourceConfigurationsRes);
    }
    else
    {
        ret = GK_NVT_Media_GetCompatibleVideoSourceConfigurations(&stMD_GetCompatibleVideoSourceConfigurations, &stMD_GetCompatibleVideoSourceConfigurationsRes);
    }
    if (ONVIF_OK == ret)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetCompatibleVideoSourceConfigurations, &stMD_GetCompatibleVideoSourceConfigurationsRes, NULL);
    }
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

//According to ONVIF standard frame,  stMD_GetVideoSourceConfigurationOptions just be  pointer type normally, not structure array.
GK_S32 soap_Media_GetVideoSourceConfigurationOptions(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetVideoSourceConfigurationOptions = soap_get_child_node(p_body, "GetVideoSourceConfigurationOptions");
    if(p_GetVideoSourceConfigurationOptions == NULL)
    {
        return -1;
    }

	GONVIF_MEDIA_GetVideoSourceConfigurationOptions_S stMD_GetVideoSourceConfigurationOptions;
    GONVIF_MEDIA_GetVideoSourceConfigurationOptions_Res_S stMD_GetVideoSourceConfigurationOptionsRes;
	memset(&stMD_GetVideoSourceConfigurationOptions, 0, sizeof(stMD_GetVideoSourceConfigurationOptions));
	memset(&stMD_GetVideoSourceConfigurationOptionsRes, 0, sizeof(stMD_GetVideoSourceConfigurationOptionsRes));
	XMLN *p_ConfigurationToken = soap_get_child_node(p_GetVideoSourceConfigurationOptions, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(stMD_GetVideoSourceConfigurationOptions.aszConfigurationToken, p_ConfigurationToken->data, MAX_TOKEN_LENGTH-1);
	}

	XMLN *p_ProfileToken = soap_get_child_node(p_GetVideoSourceConfigurationOptions, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(stMD_GetVideoSourceConfigurationOptions.aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH-1);
	}

    if(g_MediaCallBackRegister.pfnMD_GetVideoSourceConfigurationOptions != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetVideoSourceConfigurationOptions(&stMD_GetVideoSourceConfigurationOptions, &stMD_GetVideoSourceConfigurationOptionsRes);
    }
    else
    {
        ret = GK_NVT_Media_GetVideoSourceConfigurationOptions(&stMD_GetVideoSourceConfigurationOptions, &stMD_GetVideoSourceConfigurationOptionsRes);
    }


	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetVideoSourceConfigurationOptions, &stMD_GetVideoSourceConfigurationOptionsRes, NULL);
	}

	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_SetVideoSourceConfiguration(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GK_S32 retVal = 0;
    XMLN *p_SetVideoSourceConfiguration = soap_get_child_node(p_body, "SetVideoSourceConfiguration");
    if(p_SetVideoSourceConfiguration == NULL)
    {
        return -1;
    }

    GONVIF_MEDIA_SetVideoSourceConfiguration_S stMD_SetVideoSourceConfiguration;
    memset(&stMD_SetVideoSourceConfiguration, 0, sizeof(stMD_SetVideoSourceConfiguration));

    ret = soap_parse_request_SetVideoSourceConfiguration(p_SetVideoSourceConfiguration, &stMD_SetVideoSourceConfiguration);
    if(ret == ONVIF_OK)
    {
        if(g_MediaCallBackRegister.pfnMD_SetVideoSourceConfiguration != NULL)
        {
            ret = g_MediaCallBackRegister.pfnMD_SetVideoSourceConfiguration(&stMD_SetVideoSourceConfiguration);
        }
        else
        {
            ret = GK_NVT_Media_SetVideoSourceConfiguration(&stMD_SetVideoSourceConfiguration);
        }
    	if(ret == ONVIF_OK)
    	{
    		retVal = soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_SetVideoSourceConfiguration, NULL, NULL);
            if(stMD_SetVideoSourceConfiguration.stVideoSourceConfiguration.pszToken)
            {
                free(stMD_SetVideoSourceConfiguration.stVideoSourceConfiguration.pszToken);
                stMD_SetVideoSourceConfiguration.stVideoSourceConfiguration.pszToken = NULL;
            }
            if(stMD_SetVideoSourceConfiguration.stVideoSourceConfiguration.pszSourceToken)
            {
                free(stMD_SetVideoSourceConfiguration.stVideoSourceConfiguration.pszSourceToken);
                stMD_SetVideoSourceConfiguration.stVideoSourceConfiguration.pszSourceToken = NULL;
            }
            if(stMD_SetVideoSourceConfiguration.stVideoSourceConfiguration.pszName)
            {
                free(stMD_SetVideoSourceConfiguration.stVideoSourceConfiguration.pszName);
                stMD_SetVideoSourceConfiguration.stVideoSourceConfiguration.pszName = NULL;
            }

            return retVal;
    	}
    }
    if(stMD_SetVideoSourceConfiguration.stVideoSourceConfiguration.pszToken)
    {
        free(stMD_SetVideoSourceConfiguration.stVideoSourceConfiguration.pszToken);
        stMD_SetVideoSourceConfiguration.stVideoSourceConfiguration.pszToken = NULL;
    }
    if(stMD_SetVideoSourceConfiguration.stVideoSourceConfiguration.pszSourceToken)
    {
        free(stMD_SetVideoSourceConfiguration.stVideoSourceConfiguration.pszSourceToken);
        stMD_SetVideoSourceConfiguration.stVideoSourceConfiguration.pszSourceToken = NULL;
    }
    if(stMD_SetVideoSourceConfiguration.stVideoSourceConfiguration.pszName)
    {
        free(stMD_SetVideoSourceConfiguration.stVideoSourceConfiguration.pszName);
        stMD_SetVideoSourceConfiguration.stVideoSourceConfiguration.pszName = NULL;
    }
    
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_GetVideoEncoderConfigurations(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetVideoEncoderConfigurations = soap_get_child_node(p_body, "GetVideoEncoderConfigurations");
    if(p_GetVideoEncoderConfigurations == NULL)
    {
        return -1;
    }

    GONVIF_MEDIA_GetVideoEncoderConfigurations_Res_S pstMD_GetVideoEncoderConfigurationsRes;
    memset(&pstMD_GetVideoEncoderConfigurationsRes, 0, sizeof(pstMD_GetVideoEncoderConfigurationsRes));
    if(g_MediaCallBackRegister.pfnMD_GetVideoEncoderConfigurations != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetVideoEncoderConfigurations(&pstMD_GetVideoEncoderConfigurationsRes);
    }
    else
    {
        ret = GK_NVT_Media_GetVideoEncoderConfigurations(&pstMD_GetVideoEncoderConfigurationsRes);
    }
  	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetVideoEncoderConfigurations, &pstMD_GetVideoEncoderConfigurationsRes, NULL);
	}
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}


GK_S32 soap_Media_GetVideoEncoderConfiguration(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetVideoEncoderConfiguration = soap_get_child_node(p_body, "GetVideoEncoderConfiguration");
    if(p_GetVideoEncoderConfiguration == NULL)
    {
        return -1;
    }
    GONVIF_MEDIA_GetVideoEncoderConfiguration_S stMD_GetVideoEncoderConfiguration;
    GONVIF_MEDIA_GetVideoEncoderConfiguration_Res_S stMD_GetVideoEncoderConfigurationRes;
    memset(&stMD_GetVideoEncoderConfiguration, 0, sizeof(stMD_GetVideoEncoderConfiguration));
    memset(&stMD_GetVideoEncoderConfigurationRes, 0, sizeof(stMD_GetVideoEncoderConfigurationRes));
	XMLN *p_ConfigurationToken = soap_get_child_node(p_GetVideoEncoderConfiguration, "ConfigurationToken");
	if(p_ConfigurationToken && p_ConfigurationToken->data)
	{
	    strncpy(stMD_GetVideoEncoderConfiguration.aszConfigurationToken, p_ConfigurationToken->data, MAX_TOKEN_LENGTH - 1);
	}
    if(g_MediaCallBackRegister.pfnMD_GetVideoEncoderConfiguration != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetVideoEncoderConfiguration(&stMD_GetVideoEncoderConfiguration, &stMD_GetVideoEncoderConfigurationRes);
    }
    else
    {
        ret = GK_NVT_Media_GetVideoEncoderConfiguration(&stMD_GetVideoEncoderConfiguration, &stMD_GetVideoEncoderConfigurationRes);
    }
  	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetVideoEncoderConfiguration, &stMD_GetVideoEncoderConfigurationRes, NULL);
	}
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_GetCompatibleVideoEncoderConfigurations(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_GetCompatibleVideoEncoderConfigurations = soap_get_child_node(p_body, "GetCompatibleVideoEncoderConfigurations");
    if(p_GetCompatibleVideoEncoderConfigurations == NULL)
    {
        return -1;
    }
    GONVIF_MEDIA_GetCompatibleVideoEncoderConfigurations_S stMD_GetCompatibleVideoEncoderConfigurations;
    GONVIF_MEDIA_GetCompatibleVideoEncoderConfigurations_Res_S stMD_GetCompatibleVideoEncoderConfigurationsRes;
    memset(&stMD_GetCompatibleVideoEncoderConfigurations, 0, sizeof(stMD_GetCompatibleVideoEncoderConfigurations));
    memset(&stMD_GetCompatibleVideoEncoderConfigurationsRes, 0, sizeof(stMD_GetCompatibleVideoEncoderConfigurationsRes));
    XMLN *p_ProfileToken = soap_get_child_node(p_GetCompatibleVideoEncoderConfigurations, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
	    strncpy(stMD_GetCompatibleVideoEncoderConfigurations.aszProfileToken,p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
    }
    if(g_MediaCallBackRegister.pfnMD_GetCompatibleVideoEncoderConfigurations != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetCompatibleVideoEncoderConfigurations(&stMD_GetCompatibleVideoEncoderConfigurations, &stMD_GetCompatibleVideoEncoderConfigurationsRes);
    }
    else
    {
        ret = GK_NVT_Media_GetCompatibleVideoEncoderConfigurations(&stMD_GetCompatibleVideoEncoderConfigurations, &stMD_GetCompatibleVideoEncoderConfigurationsRes);
    }
  	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetCompatibleVideoEncoderConfigurations, &stMD_GetCompatibleVideoEncoderConfigurationsRes, NULL);
	}
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

//According to ONVIF standard frame,  stMD_GetVideoEncoderConfigurationOptionsRes just be pointer type normally, not structure array.
GK_S32 soap_Media_GetVideoEncoderConfigurationOptions(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_GetVideoEncoderConfigurationOptions = soap_get_child_node(p_body, "GetVideoEncoderConfigurationOptions");
    if(p_GetVideoEncoderConfigurationOptions == NULL)
    {
        return -1;
    }

    GONVIF_MEDIA_GetVideoEncoderConfigurationOptions_S stMD_GetVideoEncoderConfigurationOptions;
    GONVIF_MEDIA_GetVideoEncoderConfigurationOptions_Res_S stMD_GetVideoEncoderConfigurationOptionsRes;
    memset(&stMD_GetVideoEncoderConfigurationOptions, 0, sizeof(stMD_GetVideoEncoderConfigurationOptions));
    memset(&stMD_GetVideoEncoderConfigurationOptionsRes, 0, sizeof(stMD_GetVideoEncoderConfigurationOptionsRes));
	XMLN *p_ConfigurationToken = soap_get_child_node(p_GetVideoEncoderConfigurationOptions, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(stMD_GetVideoEncoderConfigurationOptions.aszConfigurationToken, p_ConfigurationToken->data, MAX_TOKEN_LENGTH - 1);
	}

	XMLN *p_ProfileToken = soap_get_child_node(p_GetVideoEncoderConfigurationOptions, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(stMD_GetVideoEncoderConfigurationOptions.aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
	}
    if(g_MediaCallBackRegister.pfnMD_GetVideoEncoderConfigurationOptions != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetVideoEncoderConfigurationOptions(&stMD_GetVideoEncoderConfigurationOptions, &stMD_GetVideoEncoderConfigurationOptionsRes);
    }
    else
    {
        ret = GK_NVT_Media_GetVideoEncoderConfigurationOptions(&stMD_GetVideoEncoderConfigurationOptions, &stMD_GetVideoEncoderConfigurationOptionsRes);
    }
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetVideoEncoderConfigurationOptions, &stMD_GetVideoEncoderConfigurationOptionsRes, NULL);
	}

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_SetVideoEncoderConfiguration(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GK_S32 retVal = 0;
    XMLN *p_SetVideoEncoderConfiguration = soap_get_child_node(p_body, "SetVideoEncoderConfiguration");
    if(p_SetVideoEncoderConfiguration == NULL)
    {
        return -1;
    }

	GONVIF_MEDIA_SetVideoEncoderConfiguration_S stMD_SetVideoEncoderConfiguration;
	memset(&stMD_SetVideoEncoderConfiguration, 0, sizeof(stMD_SetVideoEncoderConfiguration));
	ret = soap_parse_request_SetVideoEncoderConfiguration(p_SetVideoEncoderConfiguration, &stMD_SetVideoEncoderConfiguration);
    if(ret == ONVIF_OK)
    {
        if(g_MediaCallBackRegister.pfnMD_SetVideoEncoderConfiguration != NULL)
        {
            ret = g_MediaCallBackRegister.pfnMD_SetVideoEncoderConfiguration(&stMD_SetVideoEncoderConfiguration);
        }
        else
        {
            ret = GK_NVT_Media_SetVideoEncoderConfiguration(&stMD_SetVideoEncoderConfiguration);
        }
        if(ret == ONVIF_OK)
        {
        	retVal =  soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_SetVideoEncoderConfiguration, NULL, NULL);
            if(stMD_SetVideoEncoderConfiguration.stConfiguration.pszToken)
            {
                free(stMD_SetVideoEncoderConfiguration.stConfiguration.pszToken);
                stMD_SetVideoEncoderConfiguration.stConfiguration.pszToken = NULL;
            }
            if(stMD_SetVideoEncoderConfiguration.stConfiguration.pszName)
            {
                free(stMD_SetVideoEncoderConfiguration.stConfiguration.pszName);
                stMD_SetVideoEncoderConfiguration.stConfiguration.pszName = NULL;
            }
            if(stMD_SetVideoEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv4Address)
            {
                free(stMD_SetVideoEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv4Address);
                stMD_SetVideoEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv4Address = NULL;
            }
            if(stMD_SetVideoEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv6Address)
            {
                free(stMD_SetVideoEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv6Address);
                stMD_SetVideoEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv6Address = NULL;
            }

            return retVal;
        }
    }
    if(stMD_SetVideoEncoderConfiguration.stConfiguration.pszToken)
    {
        free(stMD_SetVideoEncoderConfiguration.stConfiguration.pszToken);
        stMD_SetVideoEncoderConfiguration.stConfiguration.pszToken = NULL;
    }
    if(stMD_SetVideoEncoderConfiguration.stConfiguration.pszName)
    {
        free(stMD_SetVideoEncoderConfiguration.stConfiguration.pszName);
        stMD_SetVideoEncoderConfiguration.stConfiguration.pszName = NULL;
    }
    if(stMD_SetVideoEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv4Address)
    {
        free(stMD_SetVideoEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv4Address);
        stMD_SetVideoEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv4Address = NULL;
    }
    if(stMD_SetVideoEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv6Address)
    {
        free(stMD_SetVideoEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv6Address);
        stMD_SetVideoEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv6Address = NULL;
    }
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_GetGuaranteedNumberOfVideoEncoderInstances(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetGuaranteedNumberOfVideoEncoderInstances = soap_get_child_node(p_body, "GetGuaranteedNumberOfVideoEncoderInstances");
	if(p_GetGuaranteedNumberOfVideoEncoderInstances == NULL)
    {
        return -1;
    }
    GONVIF_MEDIA_GetGuaranteedNumberOfVideoEncoderInstances_S stMD_GetGuaranteedNumberOfVideoEncoderInstances;
    GONVIF_MEDIA_GetGuaranteedNumberOfVideoEncoderInstances_Res_S stMD_GetGuaranteedNumberOfVideoEncoderInstancesRes;
	memset(&stMD_GetGuaranteedNumberOfVideoEncoderInstances, 0, sizeof(stMD_GetGuaranteedNumberOfVideoEncoderInstances));
	memset(&stMD_GetGuaranteedNumberOfVideoEncoderInstancesRes, 0, sizeof(stMD_GetGuaranteedNumberOfVideoEncoderInstancesRes));
	XMLN *p_ConfigurationToken = soap_get_child_node(p_GetGuaranteedNumberOfVideoEncoderInstances, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
        strncpy(stMD_GetGuaranteedNumberOfVideoEncoderInstances.aszConfigurationToken, p_ConfigurationToken->data, MAX_TOKEN_LENGTH - 1);
	}
    if(g_MediaCallBackRegister.pfnMD_GetGuaranteedNumberOfVideoEncoderInstances != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetGuaranteedNumberOfVideoEncoderInstances(&stMD_GetGuaranteedNumberOfVideoEncoderInstances, &stMD_GetGuaranteedNumberOfVideoEncoderInstancesRes);
    }
    else
    {
        ret = GK_NVT_Media_GetGuaranteedNumberOfVideoEncoderInstances(&stMD_GetGuaranteedNumberOfVideoEncoderInstances, &stMD_GetGuaranteedNumberOfVideoEncoderInstancesRes);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetGuaranteedNumberOfVideoEncoderInstances, &stMD_GetGuaranteedNumberOfVideoEncoderInstancesRes, NULL);
    }
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_GetAudioSourceConfigurations(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetAudioSourceConfigurations = soap_get_child_node(p_body, "GetAudioSourceConfigurations");
    if(p_GetAudioSourceConfigurations == NULL)
    {
        return -1;
    }

    GONVIF_MEDIA_GetAudioSourceConfigurations_Res_S stMD_GetAudioSourceConfigurationRes;
    memset(&stMD_GetAudioSourceConfigurationRes, 0, sizeof(stMD_GetAudioSourceConfigurationRes));
    if(g_MediaCallBackRegister.pfnMD_GetAudioSourceConfigurations != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetAudioSourceConfigurations(&stMD_GetAudioSourceConfigurationRes);
    }
    else
    {
        ret = GK_NVT_Media_GetAudioSourceConfigurations(&stMD_GetAudioSourceConfigurationRes);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetAudioSourceConfigurations, &stMD_GetAudioSourceConfigurationRes, NULL);
    }
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_GetAudioSourceConfiguration(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetAudioSourceConfiguration = soap_get_child_node(p_body, "GetAudioSourceConfiguration");
    if(p_GetAudioSourceConfiguration == NULL)
    {
        return -1;
    }
    GONVIF_MEDIA_GetAudioSourceConfiguration_S stMD_GetAudioSourceConfiguration;
    GONVIF_MEDIA_GetAudioSourceConfiguration_Res_S stMD_GetAudioSourceConfigurationRes;
    memset(&stMD_GetAudioSourceConfiguration, 0, sizeof(stMD_GetAudioSourceConfiguration));
    memset(&stMD_GetAudioSourceConfigurationRes, 0, sizeof(stMD_GetAudioSourceConfigurationRes));
    XMLN *p_ConfigurationToken = soap_get_child_node(p_GetAudioSourceConfiguration, "ConfigurationToken");
    if (p_ConfigurationToken && p_ConfigurationToken->data)
    {
        strncpy(stMD_GetAudioSourceConfiguration.aszConfigurationToken, p_ConfigurationToken->data, MAX_TOKEN_LENGTH - 1);
    }
    if(g_MediaCallBackRegister.pfnMD_GetAudioSourceConfiguration != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetAudioSourceConfiguration(&stMD_GetAudioSourceConfiguration, &stMD_GetAudioSourceConfigurationRes);
    }
    else
    {
        ret = GK_NVT_Media_GetAudioSourceConfiguration(&stMD_GetAudioSourceConfiguration, &stMD_GetAudioSourceConfigurationRes);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetAudioSourceConfiguration, &stMD_GetAudioSourceConfigurationRes, NULL);
    }
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_GetCompatibleAudioSourceConfigurations(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetCompatibleAudioSourceConfigurations = soap_get_child_node(p_body, "GetCompatibleAudioSourceConfigurations");
    if(p_GetCompatibleAudioSourceConfigurations == NULL)
    {
        return -1;
    }
    GONVIF_MEDIA_GetCompatibleAudioSourceConfigurations_S stMD_GetCompatibleAudioSourceConfigurations;
    GONVIF_MEDIA_GetCompatibleAudioSourceConfigurations_Res_S stMD_GetCompatibleAudioSourceConfigurationsRes;
    memset(&stMD_GetCompatibleAudioSourceConfigurations, 0, sizeof(stMD_GetCompatibleAudioSourceConfigurations));
    memset(&stMD_GetCompatibleAudioSourceConfigurationsRes, 0, sizeof(stMD_GetCompatibleAudioSourceConfigurationsRes));
	XMLN *p_ProfileToken = soap_get_child_node(p_GetCompatibleAudioSourceConfigurations, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
	    strncpy(stMD_GetCompatibleAudioSourceConfigurations.aszProfileToken,p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
    }
    if(g_MediaCallBackRegister.pfnMD_GetCompatibleAudioSourceConfigurations != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetCompatibleAudioSourceConfigurations(&stMD_GetCompatibleAudioSourceConfigurations, &stMD_GetCompatibleAudioSourceConfigurationsRes);
    }
    else
    {
        ret = GK_NVT_Media_GetCompatibleAudioSourceConfigurations(&stMD_GetCompatibleAudioSourceConfigurations, &stMD_GetCompatibleAudioSourceConfigurationsRes);
    }
    if (ONVIF_OK == ret)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetCompatibleAudioSourceConfigurations, &stMD_GetCompatibleAudioSourceConfigurationsRes, NULL);
    }
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_GetAudioSourceConfigurationOptions(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetAudioSourceConfigurationOptions = soap_get_child_node(p_body, "GetAudioSourceConfigurationOptions");
    if(p_GetAudioSourceConfigurationOptions == NULL)
    {
        return -1;
    }

	GONVIF_MEDIA_GetAudioSourceConfigurationOptions_S stMD_GetAudioSourceConfigurationOptions;
    GONVIF_MEDIA_GetAudioSourceConfigurationOptions_Res_S stMD_GetAudioSourceConfigurationOptionsRes;
	memset(&stMD_GetAudioSourceConfigurationOptions, 0, sizeof(stMD_GetAudioSourceConfigurationOptions));
	memset(&stMD_GetAudioSourceConfigurationOptionsRes, 0, sizeof(stMD_GetAudioSourceConfigurationOptionsRes));
	XMLN *p_ConfigurationToken = soap_get_child_node(p_GetAudioSourceConfigurationOptions, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(stMD_GetAudioSourceConfigurationOptions.aszConfigurationToken, p_ConfigurationToken->data, MAX_TOKEN_LENGTH-1);
	}

	XMLN *p_ProfileToken = soap_get_child_node(p_GetAudioSourceConfigurationOptions, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(stMD_GetAudioSourceConfigurationOptions.aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH-1);
	}

    if(g_MediaCallBackRegister.pfnMD_GetAudioSourceConfigurationOptions != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetAudioSourceConfigurationOptions(&stMD_GetAudioSourceConfigurationOptions, &stMD_GetAudioSourceConfigurationOptionsRes);
    }
    else
    {
        ret = GK_NVT_Media_GetAudioSourceConfigurationOptions(&stMD_GetAudioSourceConfigurationOptions, &stMD_GetAudioSourceConfigurationOptionsRes);
    }


	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetAudioSourceConfigurationOptions, &stMD_GetAudioSourceConfigurationOptionsRes, NULL);
	}

	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_SetAudioSourceConfiguration(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GK_S32 retVal = 0;
    XMLN *p_SetAudioSourceConfiguration = soap_get_child_node(p_body, "SetAudioSourceConfiguration");
    if(p_SetAudioSourceConfiguration == NULL)
    {
        return -1;
    }

    GONVIF_MEDIA_SetAudioSourceConfiguration_S stMD_SetAudioSourceConfiguration;
    memset(&stMD_SetAudioSourceConfiguration, 0, sizeof(stMD_SetAudioSourceConfiguration));

    ret = soap_parse_request_SetAudioSourceConfiguration(p_SetAudioSourceConfiguration, &stMD_SetAudioSourceConfiguration);
    if(ret == ONVIF_OK)
    {
        if(g_MediaCallBackRegister.pfnMD_SetAudioSourceConfiguration != NULL)
        {
            ret = g_MediaCallBackRegister.pfnMD_SetAudioSourceConfiguration(&stMD_SetAudioSourceConfiguration);
        }
        else
        {
            ret = GK_NVT_Media_SetAudioSourceConfiguration(&stMD_SetAudioSourceConfiguration);
        }
    	if(ret == ONVIF_OK)
    	{
    		retVal =  soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_SetAudioSourceConfiguration, NULL, NULL);
            if(stMD_SetAudioSourceConfiguration.stConfiguration.pszToken)
            {
                free(stMD_SetAudioSourceConfiguration.stConfiguration.pszToken);
                stMD_SetAudioSourceConfiguration.stConfiguration.pszToken = NULL;
            }
            if(stMD_SetAudioSourceConfiguration.stConfiguration.pszSourceToken)
            {
                free(stMD_SetAudioSourceConfiguration.stConfiguration.pszSourceToken);
                stMD_SetAudioSourceConfiguration.stConfiguration.pszSourceToken = NULL;
            }
            if(stMD_SetAudioSourceConfiguration.stConfiguration.pszName)
            {
                free(stMD_SetAudioSourceConfiguration.stConfiguration.pszName);
                stMD_SetAudioSourceConfiguration.stConfiguration.pszName = NULL;
            }

            return retVal;
    	}
    }
    if(stMD_SetAudioSourceConfiguration.stConfiguration.pszToken)
    {
        free(stMD_SetAudioSourceConfiguration.stConfiguration.pszToken);
        stMD_SetAudioSourceConfiguration.stConfiguration.pszToken = NULL;
    }
    if(stMD_SetAudioSourceConfiguration.stConfiguration.pszSourceToken)
    {
        free(stMD_SetAudioSourceConfiguration.stConfiguration.pszSourceToken);
        stMD_SetAudioSourceConfiguration.stConfiguration.pszSourceToken = NULL;
    }
    if(stMD_SetAudioSourceConfiguration.stConfiguration.pszName)
    {
        free(stMD_SetAudioSourceConfiguration.stConfiguration.pszName);
        stMD_SetAudioSourceConfiguration.stConfiguration.pszName = NULL;
    }
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_GetAudioEncoderConfigurations(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetAudioEncoderConfigurations = soap_get_child_node(p_body, "GetAudioEncoderConfigurations");
    if(p_GetAudioEncoderConfigurations == NULL)
    {
        return -1;
    }

    GONVIF_MEDIA_GetAudioEncoderConfigurations_Res_S stMD_GetAudioEncoderConfigurationsRes;
    memset(&stMD_GetAudioEncoderConfigurationsRes, 0, sizeof(stMD_GetAudioEncoderConfigurationsRes));
    if(g_MediaCallBackRegister.pfnMD_GetAudioEncoderConfigurations != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetAudioEncoderConfigurations(&stMD_GetAudioEncoderConfigurationsRes);
    }
    else
    {
        ret = GK_NVT_Media_GetAudioEncoderConfigurations(&stMD_GetAudioEncoderConfigurationsRes);
    }
  	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetAudioEncoderConfigurations, &stMD_GetAudioEncoderConfigurationsRes, NULL);
	}
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_GetAudioEncoderConfiguration(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetAudioEncoderConfiguration = soap_get_child_node(p_body, "GetAudioEncoderConfiguration");
    if(p_GetAudioEncoderConfiguration == NULL)
    {
        return -1;
    }
    GONVIF_MEDIA_GetAudioEncoderConfiguration_S stMD_GetAudioEncoderConfiguration;
    GONVIF_MEDIA_GetAudioEncoderConfiguration_Res_S stMD_GetAudioEncoderConfigurationRes;
    memset(&stMD_GetAudioEncoderConfiguration, 0, sizeof(stMD_GetAudioEncoderConfiguration));
    memset(&stMD_GetAudioEncoderConfigurationRes, 0, sizeof(stMD_GetAudioEncoderConfigurationRes));
    XMLN *p_ConfigurationToken = soap_get_child_node(p_GetAudioEncoderConfiguration, "ConfigurationToken");
    if (p_ConfigurationToken && p_ConfigurationToken->data)
    {
        strncpy(stMD_GetAudioEncoderConfiguration.aszConfigurationToken, p_ConfigurationToken->data, MAX_TOKEN_LENGTH - 1);
    }
    if(g_MediaCallBackRegister.pfnMD_GetAudioEncoderConfiguration != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetAudioEncoderConfiguration(&stMD_GetAudioEncoderConfiguration,&stMD_GetAudioEncoderConfigurationRes);
    }
    else
    {
        ret = GK_NVT_Media_GetAudioEncoderConfiguration(&stMD_GetAudioEncoderConfiguration,&stMD_GetAudioEncoderConfigurationRes);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetAudioEncoderConfiguration, &stMD_GetAudioEncoderConfigurationRes, NULL);
    }
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}


GK_S32 soap_Media_GetCompatibleAudioEncoderConfigurations(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetCompatibleAudioEncoderConfigurations = soap_get_child_node(p_body, "GetCompatibleAudioEncoderConfigurations");
    if(p_GetCompatibleAudioEncoderConfigurations == NULL)
    {
        return -1;
    }
    GONVIF_MEDIA_GetCompatibleAudioEncoderConfigurations_S stGetComAudioEncoderCfgReq;
    GONVIF_MEDIA_GetCompatibleAudioEncoderConfigurations_Res_S stGetComAudioEncoderCfgRes;
    memset(&stGetComAudioEncoderCfgReq, 0, sizeof(stGetComAudioEncoderCfgReq));
    memset(&stGetComAudioEncoderCfgRes, 0, sizeof(stGetComAudioEncoderCfgRes));
	XMLN *p_ProfileToken = soap_get_child_node(p_GetCompatibleAudioEncoderConfigurations, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
	    strncpy(stGetComAudioEncoderCfgReq.aszProfileToken,p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
    }
    if(g_MediaCallBackRegister.pfnMD_GetCompatibleAudioEncoderConfigurations != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetCompatibleAudioEncoderConfigurations(&stGetComAudioEncoderCfgReq,&stGetComAudioEncoderCfgRes);
    }
    else
    {
        ret = GK_NVT_Media_GetCompatibleAudioEncoderConfigurations(&stGetComAudioEncoderCfgReq,&stGetComAudioEncoderCfgRes);
    }
    if (ONVIF_OK == ret)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetCompatibleAudioEncoderConfigurations, &stGetComAudioEncoderCfgRes, NULL);
    }
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_GetAudioEncoderConfigurationOptions(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_GetAudioEncoderConfigurationOptions = soap_get_child_node(p_body, "GetAudioEncoderConfigurationOptions");
    if(p_GetAudioEncoderConfigurationOptions == NULL)
    {
        return -1;
    }

    GONVIF_MEDIA_GetAudioEncoderConfigurationOptions_S stMD_GetAudioEncoderConfigurationOptions;
    GONVIF_MEDIA_GetAudioEncoderConfigurationOptions_Res_S stMD_GetAudioEncoderConfigurationOptionsRes;
    memset(&stMD_GetAudioEncoderConfigurationOptions, 0, sizeof(stMD_GetAudioEncoderConfigurationOptions));
    memset(&stMD_GetAudioEncoderConfigurationOptionsRes, 0, sizeof(stMD_GetAudioEncoderConfigurationOptionsRes));
	XMLN *p_ConfigurationToken = soap_get_child_node(p_GetAudioEncoderConfigurationOptions, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(stMD_GetAudioEncoderConfigurationOptions.aszConfigurationToken, p_ConfigurationToken->data, MAX_TOKEN_LENGTH - 1);
	}

	XMLN *p_ProfileToken = soap_get_child_node(p_GetAudioEncoderConfigurationOptions, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(stMD_GetAudioEncoderConfigurationOptions.aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
	}
    if(g_MediaCallBackRegister.pfnMD_GetAudioEncoderConfigurationOptions != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetAudioEncoderConfigurationOptions(&stMD_GetAudioEncoderConfigurationOptions, &stMD_GetAudioEncoderConfigurationOptionsRes);
    }
    else
    {
        ret = GK_NVT_Media_GetAudioEncoderConfigurationOptions(&stMD_GetAudioEncoderConfigurationOptions, &stMD_GetAudioEncoderConfigurationOptionsRes);
    }
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetAudioEncoderConfigurationOptions, &stMD_GetAudioEncoderConfigurationOptionsRes, NULL);
	}

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_SetAudioEncoderConfiguration(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GK_S32 retVal = 0;
    XMLN *p_SetAudioEncoderConfiguration = soap_get_child_node(p_body, "SetAudioEncoderConfiguration");
    if(p_SetAudioEncoderConfiguration == NULL)
    {
        return -1;
    }

	GONVIF_MEDIA_SetAudioEncoderConfiguration_S stMD_SetAudioEncoderConfiguration;
	memset(&stMD_SetAudioEncoderConfiguration, 0, sizeof(stMD_SetAudioEncoderConfiguration));
	ret = soap_parse_request_SetAudioEncoderConfiguration(p_SetAudioEncoderConfiguration, &stMD_SetAudioEncoderConfiguration);
    if(ret == ONVIF_OK)
    {
        if(g_MediaCallBackRegister.pfnMD_SetAudioEncoderConfiguration != NULL)
        {
            ret = g_MediaCallBackRegister.pfnMD_SetAudioEncoderConfiguration(&stMD_SetAudioEncoderConfiguration);
        }
        else
        {
            ret = GK_NVT_Media_SetAudioEncoderConfiguration(&stMD_SetAudioEncoderConfiguration);
        }
        if(ret == ONVIF_OK)
        {
        	retVal =  soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_SetAudioEncoderConfiguration, NULL, NULL);
            if(stMD_SetAudioEncoderConfiguration.stConfiguration.pszName)
            {
                free(stMD_SetAudioEncoderConfiguration.stConfiguration.pszName);
                stMD_SetAudioEncoderConfiguration.stConfiguration.pszName = NULL;
            }
            
            if(stMD_SetAudioEncoderConfiguration.stConfiguration.pszToken)
            {
                free(stMD_SetAudioEncoderConfiguration.stConfiguration.pszToken);
                stMD_SetAudioEncoderConfiguration.stConfiguration.pszToken = NULL;
            }
            if(stMD_SetAudioEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv4Address)
            {
                free(stMD_SetAudioEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv4Address);
                stMD_SetAudioEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv4Address = NULL;
            }
            if(stMD_SetAudioEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv6Address)
            {
                free(stMD_SetAudioEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv6Address);
                stMD_SetAudioEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv6Address = NULL;
            }
            return retVal;
        }
    }
    if(stMD_SetAudioEncoderConfiguration.stConfiguration.pszName)
    {
        free(stMD_SetAudioEncoderConfiguration.stConfiguration.pszName);
        stMD_SetAudioEncoderConfiguration.stConfiguration.pszName = NULL;
    }
    
    if(stMD_SetAudioEncoderConfiguration.stConfiguration.pszToken)
    {
        free(stMD_SetAudioEncoderConfiguration.stConfiguration.pszToken);
        stMD_SetAudioEncoderConfiguration.stConfiguration.pszToken = NULL;
    }
    if(stMD_SetAudioEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv4Address)
    {
        free(stMD_SetAudioEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv4Address);
        stMD_SetAudioEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv4Address = NULL;
    }
    if(stMD_SetAudioEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv6Address)
    {
        free(stMD_SetAudioEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv6Address);
        stMD_SetAudioEncoderConfiguration.stConfiguration.stMulticast.stIPAddress.pszIPv6Address = NULL;
    }
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_GetMetadataConfigurations(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_GetMetadataConfigurations = soap_get_child_node(p_body, "GetMetadataConfigurations");
	if(p_GetMetadataConfigurations == NULL)
    {
        return -1;
    }
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetMetadataConfigurations, NULL, NULL);
	}

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_GetMetadataConfiguration(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_GetMetadataConfiguration = soap_get_child_node(p_body, "GetMetadataConfiguration");
	if(p_GetMetadataConfiguration == NULL)
    {
        return -1;
    }
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetMetadataConfiguration, NULL, NULL);
	}

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}


GK_S32 soap_Media_GetCompatibleMetadataConfigurations(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_GetCompatibleMetadataConfigurations = soap_get_child_node(p_body, "GetCompatibleMetadataConfigurations");
	if(p_GetCompatibleMetadataConfigurations == NULL)
    {
        return -1;
    }
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetCompatibleMetadataConfigurations, NULL, NULL);
	}

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}
GK_S32 soap_Media_GetMetadataConfigurationOptions(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_GetMetadataConfigurationOptions = soap_get_child_node(p_body, "GetMetadataConfigurationOptions");
	if(p_GetMetadataConfigurationOptions == NULL)
    {
        return -1;
    }
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetMetadataConfigurationOptions, NULL, NULL);
	}

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_GetStreamUri(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN * p_GetStreamUri = soap_get_child_node(p_body, "GetStreamUri");
    if(p_GetStreamUri == NULL)
    {
        return -1;
    }

	GONVIF_MEDIA_GetStreamUri_S stMD_GetStreamUri;
    GONVIF_MEDIA_GetStreamUri_Res_S stMD_GetStreamUriRes;
	memset(&stMD_GetStreamUri, 0, sizeof(stMD_GetStreamUri));
	memset(&stMD_GetStreamUriRes, 0, sizeof(stMD_GetStreamUriRes));
	ret = soap_parse_request_GetStreamUri(p_GetStreamUri, &stMD_GetStreamUri);
	if(ret == ONVIF_OK)
	{
	    if(g_MediaCallBackRegister.pfnMD_GetStreamUri != NULL)
        {
            ret = g_MediaCallBackRegister.pfnMD_GetStreamUri(&stMD_GetStreamUri, &stMD_GetStreamUriRes);
        }
        else
        {
            ret = GK_NVT_Media_GetStreamUri(&stMD_GetStreamUri, &stMD_GetStreamUriRes);
        }
        if(ret == ONVIF_OK)
        {
    		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetStreamUri, &stMD_GetStreamUriRes, NULL);
        }
	}

	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_GetSnapshotUri(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetSnapshotUri = soap_get_child_node(p_body, "GetSnapshotUri");
    if(p_GetSnapshotUri == NULL)
    {
        return -1;
    }
    GONVIF_MEIDA_GetSnapShotUri_S stMD_GetSnapShotUri;
    GONVIF_MEIDA_GetSnapShotUri_Res_S stMD_GetSnapShotUriRes;
	memset(&stMD_GetSnapShotUri, 0, sizeof(stMD_GetSnapShotUri));
	memset(&stMD_GetSnapShotUriRes, 0, sizeof(stMD_GetSnapShotUriRes));

    XMLN *p_ProfileToken = soap_get_child_node(p_GetSnapshotUri, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
	    strncpy(stMD_GetSnapShotUri.aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
    }

    if(g_MediaCallBackRegister.pfnMD_GetSnapshotUri != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetSnapshotUri(&stMD_GetSnapShotUri, &stMD_GetSnapShotUriRes);
    }
    else
    {
        ret = GK_NVT_Media_GetSnapshotUri(&stMD_GetSnapShotUri, &stMD_GetSnapShotUriRes);
    }
    if(ret == ONVIF_OK)
    {  
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetSnapshotUri, &stMD_GetSnapShotUriRes, NULL);
    }
	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_StartMulticastStreaming(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_StartMulticastStreaming = soap_get_child_node(p_body, "StartMulticastStreaming");
	if(p_StartMulticastStreaming == NULL)
    {
        return -1;
    }
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_StartMulticastStreaming, NULL, NULL);
	}

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_StopMulticastStreaming(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_StopMulticastStreaming = soap_get_child_node(p_body, "StopMulticastStreaming");
	if(p_StopMulticastStreaming == NULL)
    {
        return -1;
    }
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_StopMulticastStreaming, NULL, NULL);
	}

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_GetOSDOptions(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GONVIF_MEDIA_GetOSDOptions_Token_S stGetMediaOptionsReq;
    memset(&stGetMediaOptionsReq, 0, sizeof(stGetMediaOptionsReq));
    ret = soap_parse_request_GetOSDOptions(p_body, &stGetMediaOptionsReq);
    if(ret != ONVIF_OK)
    {
        return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
    }

    GONVIF_MEDIA_GetOSDOptions_S stGetMediaOSDOptionsRes;
    memset(&stGetMediaOSDOptionsRes, 0, sizeof(stGetMediaOSDOptionsRes));
    if(g_MediaCallBackRegister.pfnMD_GetOSDOptions != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetOSDOptions(&stGetMediaOptionsReq, &stGetMediaOSDOptionsRes);
    }
    else
    {
        ret = GK_NVT_Media_GetOSDOptions(&stGetMediaOptionsReq, &stGetMediaOSDOptionsRes);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetOSDOptions, &stGetMediaOSDOptionsRes, NULL);
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_GetOSDs(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GONVIF_MEDIA_GetOSDs_Token_S stGetOSDsReq;
    memset(&stGetOSDsReq, 0, sizeof(stGetOSDsReq));
    ret = soap_parse_request_GetOSDs(p_body, &stGetOSDsReq);
    if(ret != ONVIF_OK)
    {
        return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
    }

    GONVIF_MEDIA_GetOSDs_S stGetOSDsRes;
    memset(&stGetOSDsRes, 0, sizeof(stGetOSDsRes));
    if(g_MediaCallBackRegister.pfnMD_GetOSDs != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetOSDs(&stGetOSDsReq, &stGetOSDsRes);
    }
    else
    {
        ret = GK_NVT_Media_GetOSDs(&stGetOSDsReq, &stGetOSDsRes);
    }
    if(ret == ONVIF_OK)
    {
        const char *child_name = soap_get_tag_child_name(p_body->f_child->name);
        if (child_name)
            return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetOSDs, &stGetOSDsRes, child_name);
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_SetOSD(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GONVIF_MEDIA_SetOSD_S stSetOSDRes;
    memset(&stSetOSDRes, 0, sizeof(stSetOSDRes));
    ret = soap_parse_request_SetOSD(p_body, &stSetOSDRes);
    if(ret != ONVIF_OK)
    {
        return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
    }

    if(g_MediaCallBackRegister.pfnMD_SetOSD != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_SetOSD(&stSetOSDRes);
    }
    else
    {
        ret = GK_NVT_Media_SetOSD(&stSetOSDRes);
    }
    if(ret == ONVIF_OK)
    {
        const char *child_name = soap_get_tag_child_name(p_body->f_child->name);
        if (child_name)
            return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_SetOSD, NULL, child_name);
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_DeleteOSD(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GONVIF_MEDIA_DeleteOSD_S stDeleteOSDRes;
    memset(&stDeleteOSDRes, 0, sizeof(stDeleteOSDRes));
    ret = soap_parse_request_DeleteOSD(p_body, &stDeleteOSDRes);
    if(ret != ONVIF_OK)
    {
        return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
    }

    if(g_MediaCallBackRegister.pfnMD_DeleteOSD != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_DeleteOSD(&stDeleteOSDRes);
    }
    else
    {
        ret = GK_NVT_Media_DeleteOSD(&stDeleteOSDRes);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_DeleteOSD, NULL, NULL);
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}


GK_S32 soap_Media_GetPrivacyMaskOptions(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GONVIF_MEDIA_GetPrivacyMaskOptions_Token_S stGetPrivacyMaskOptionsReq;
    memset(&stGetPrivacyMaskOptionsReq, 0, sizeof(stGetPrivacyMaskOptionsReq));
    ret = soap_parse_request_GetPrivacyMaskOptions(p_body, &stGetPrivacyMaskOptionsReq);
    if(ret != ONVIF_OK)
    {
        return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
    }

    GONVIF_MEDIA_GetPrivacyMaskOptions_S stGetPrivacyMaskOptionsRes;
    memset(&stGetPrivacyMaskOptionsRes, 0, sizeof(stGetPrivacyMaskOptionsRes));
    if(g_MediaCallBackRegister.pfnMD_GetPrivacyMaskOptions != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetPrivacyMaskOptions(&stGetPrivacyMaskOptionsReq, &stGetPrivacyMaskOptionsRes);
    }
    else
    {
        ret = GK_NVT_Media_GetPrivacyMaskOptions(&stGetPrivacyMaskOptionsReq, &stGetPrivacyMaskOptionsRes);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetPrivacyMaskOptions, &stGetPrivacyMaskOptionsRes, NULL);
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_GetPrivacyMasks(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GONVIF_MEDIA_GetPrivacyMasks_Token_S stGetPrivacyMasksReq;
    memset(&stGetPrivacyMasksReq, 0, sizeof(stGetPrivacyMasksReq));
    ret = soap_parse_request_GetPrivacyMasks(p_body, &stGetPrivacyMasksReq);
    if(ret != ONVIF_OK)
    {
        return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
    }

    GONVIF_MEDIA_GetPrivacyMasks_S stGetPrivacyMasksRes;
    memset(&stGetPrivacyMasksRes, 0, sizeof(stGetPrivacyMasksRes));
    if(g_MediaCallBackRegister.pfnMD_GetPrivacyMasks != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_GetPrivacyMasks(&stGetPrivacyMasksReq, &stGetPrivacyMasksRes);
    }
    else
    {
        ret = GK_NVT_Media_GetPrivacyMasks(&stGetPrivacyMasksReq, &stGetPrivacyMasksRes);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetPrivacyMasks, &stGetPrivacyMasksRes, NULL);
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_DeletePrivacyMask(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GONVIF_MEDIA_DeletePrivacyMask_Token_S stDeletePrivacyMaskReq;
    memset(&stDeletePrivacyMaskReq, 0, sizeof(stDeletePrivacyMaskReq));
    ret = soap_parse_request_DeletePrivacyMask(p_body, &stDeletePrivacyMaskReq);
    if(ret != ONVIF_OK)
    {
        return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
    }

    if(g_MediaCallBackRegister.pfnMD_DeletePrivacyMask != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_DeletePrivacyMask(&stDeletePrivacyMaskReq);
    }
    else
    {
        ret = GK_NVT_Media_DeletePrivacyMask(&stDeletePrivacyMaskReq);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_DeletePrivacyMask, NULL, NULL);
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Media_CreatePrivacyMask(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GONVIF_MEDIA_PrivacyMask_S stCreatePrivacyMaskReq;
    memset(&stCreatePrivacyMaskReq, 0, sizeof(stCreatePrivacyMaskReq));
    
    ret = soap_parse_request_CreatePrivacyMask(p_body, &stCreatePrivacyMaskReq);
    if(ret != ONVIF_OK)
    {
        return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
    }
    if(g_MediaCallBackRegister.pfnMD_CreatePrivacyMask != NULL)
    {
        ret = g_MediaCallBackRegister.pfnMD_CreatePrivacyMask(&stCreatePrivacyMaskReq);
    }
    else
    {
        ret = GK_NVT_Media_CreatePrivacyMask(&stCreatePrivacyMaskReq);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_CreatePrivacyMask, &stCreatePrivacyMaskReq, NULL);
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}


/*****************media function end******************/

/*******************event function*******************/

GK_S32 soap_Event_GetServiceCapabilities(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetServiceCapabilities = soap_get_child_node(p_body, "GetServiceCapabilities");
    if(p_GetServiceCapabilities == NULL)
    {
        return -1;
    }

	GONVIF_Soap_S stSoap;
	memset(&stSoap, 0, sizeof(GONVIF_Soap_S));
	GONVIF_EVENT_GetServiceCapabilities_Res_S stEV_GetServiceCapabilitiesRes;
	memset(&stEV_GetServiceCapabilitiesRes, 0, sizeof(GONVIF_EVENT_GetServiceCapabilities_Res_S));
	if(g_EventCallBackRegister.pfnEV_GetServiceCapabilities!= NULL)
	{
		ret = g_EventCallBackRegister.pfnEV_GetServiceCapabilities(&stSoap, &stEV_GetServiceCapabilitiesRes);
	}
	else
	{
		ret = GK_NVT_Event_GetServiceCapabilities(&stSoap, &stEV_GetServiceCapabilitiesRes);
	}
	if(ret == ONVIF_OK)
	{
		return	soap_proc_build_send_rly(p_cln, rx_msg, soap_response_Event_GetServiceCapabilities, &stEV_GetServiceCapabilitiesRes, &stSoap);
	}

	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);

}

GK_S32 soap_Event_GetEventProperties(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetEventProperties = soap_get_child_node(p_body, "GetEventProperties");
    if(p_GetEventProperties == NULL)
    {
        return -1;
    }
    GONVIF_Soap_S stSoap;
    GONVIF_EVENT_GetEventProperties_Res_S stEV_GetEventPropertiesRes;
    memset(&stSoap, 0, sizeof(GONVIF_Soap_S));
    memset(&stEV_GetEventPropertiesRes, 0, sizeof(GONVIF_EVENT_GetEventProperties_Res_S));
    if(g_EventCallBackRegister.pfnEV_GetEventProperties != NULL)
    {
        ret = g_EventCallBackRegister.pfnEV_GetEventProperties(&stSoap, &stEV_GetEventPropertiesRes);
    }
    else
    {
        ret = GK_NVT_Event_GetEventProperties(&stSoap, &stEV_GetEventPropertiesRes);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetEventProperties, &stEV_GetEventPropertiesRes, &stSoap);

    }
        
	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Event_Subscribe(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_Subscribe = soap_get_child_node(p_body, "Subscribe");
    if(p_Subscribe == NULL)
    {
        return -1;
    }
    GONVIF_Soap_S stSoap;
	GONVIF_EVENT_Subscribe_S stEV_Subscribe;
    GONVIF_EVENT_Subscribe_Res_S stEV_SubscribeRes;
    memset(&stSoap, 0, sizeof(GONVIF_Soap_S));
	memset(&stEV_Subscribe, 0, sizeof(GONVIF_EVENT_Subscribe_S));
	memset(&stEV_SubscribeRes, 0, sizeof(GONVIF_EVENT_Subscribe_Res_S));
	ret = soap_parse_request_Subscribe(p_Subscribe, &stEV_Subscribe);
	if(ret == ONVIF_OK)
	{
        if(g_EventCallBackRegister.pfnEV_Subscribe != NULL)
        {
            ret = g_EventCallBackRegister.pfnEV_Subscribe(&stSoap, &stEV_Subscribe, &stEV_SubscribeRes);
        }
        else
        {
            ret = GK_NVT_Event_Subscribe(&stSoap, &stEV_Subscribe, &stEV_SubscribeRes);
        }
	    if(ret == ONVIF_OK)
	    {
			return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_Subscribe, &stEV_SubscribeRes, &stSoap);
		}
	}

	return soap_proc_build_err_rly(p_cln, rx_msg, ret, &stSoap);
}

GK_S32 soap_Event_Unsubscribe(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_header)
{
    ONVIF_RET ret = ONVIF_OK;
    GONVIF_Soap_S stSoap;
    GONVIF_EVENT_Unsubscribe_S stEV_Unsubscribe;    
    memset(&stSoap, 0, sizeof(GONVIF_Soap_S));
    memset(&stEV_Unsubscribe, 0, sizeof(GONVIF_EVENT_Unsubscribe_S));
    XMLN *p_To = soap_get_child_node(p_header, "To");
	if (p_To && p_To->data)
	{
	    strncpy(stSoap.stHeader.aszTo, p_To->data, MAX_TO_LEN - 1);
	}
    if(g_EventCallBackRegister.pfnEV_Unsubscribe != NULL)
    {
        ret = g_EventCallBackRegister.pfnEV_Unsubscribe(&stSoap, &stEV_Unsubscribe);
    }
    else
    {
        ret = GK_NVT_Event_Unsubscribe(&stSoap, &stEV_Unsubscribe);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_Unsubscribe, NULL, &stSoap);
    }
	return soap_proc_build_err_rly(p_cln, rx_msg, ret, &stSoap);
}

GK_S32 soap_Event_Renew(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body, XMLN *p_header)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_Renew = soap_get_child_node(p_body, "Renew");
	if(p_Renew == NULL)
    {
        return -1;
    }
    
    GONVIF_Soap_S stSoap;
	GONVIF_EVENT_Renew_S stEV_Renew;
    GONVIF_EVENT_Renew_Res_S stEV_RenewRes;
    memset(&stSoap, 0, sizeof(GONVIF_Soap_S));
	memset(&stEV_Renew, 0, sizeof(GONVIF_EVENT_Renew_S));
	memset(&stEV_RenewRes, 0, sizeof(GONVIF_EVENT_Renew_Res_S));
	XMLN *p_To = soap_get_child_node(p_header, "To");
	if(p_To && p_To->data)
	{
		strncpy(stSoap.stHeader.aszTo, p_To->data, MAX_TO_LEN-1);
	}
	XMLN *p_TerminationTime = soap_get_child_node(p_Renew, "TerminationTime");
	if(p_TerminationTime && p_TerminationTime->data)
	{
		strncpy(stEV_Renew.aszTerminationTime, p_TerminationTime->data, MAX_PHHMS_TIME_LEN-1);
	}
    if(g_EventCallBackRegister.pfnEV_Renew != NULL)
    {
        ret = g_EventCallBackRegister.pfnEV_Renew(&stSoap, &stEV_Renew, &stEV_RenewRes);
    }
    else
    {
        ret = GK_NVT_Event_Renew(&stSoap, &stEV_Renew, &stEV_RenewRes);
    }
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_Renew, &stEV_RenewRes, &stSoap);
	}

	return soap_proc_build_err_rly(p_cln, rx_msg, ret, &stSoap);
}

GK_S32 soap_Event_CreatePullPointSubscription(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_CreatePullPointSubscription = soap_get_child_node(p_body, "CreatePullPointSubscription");
    if(p_CreatePullPointSubscription == NULL)
    {
        return -1;
    }
    GONVIF_Soap_S stSoap;
	GONVIF_EVENT_CreatePullPointSubscription_S stEV_CreatePullPointSubscription;
    GONVIF_EVENT_CreatePullPointSubscription_Res_S stEV_CreatePullPointSubscriptionRes;    
    memset(&stSoap, 0, sizeof(GONVIF_Soap_S));
	memset(&stEV_CreatePullPointSubscription, 0, sizeof(stEV_CreatePullPointSubscription));
	memset(&stEV_CreatePullPointSubscriptionRes, 0, sizeof(stEV_CreatePullPointSubscriptionRes));
	ret = soap_parse_request_CreatePullPointSubscription(p_CreatePullPointSubscription, &stEV_CreatePullPointSubscription);
	if(ret == ONVIF_OK)
	{
	    if(g_EventCallBackRegister.pfnEV_CreatePullPointSubscription != NULL)
        {
            ret = g_EventCallBackRegister.pfnEV_CreatePullPointSubscription(&stSoap, &stEV_CreatePullPointSubscription, &stEV_CreatePullPointSubscriptionRes);
        }
        else
        {
            ret = GK_NVT_Event_CreatePullPointSubscription(&stSoap, &stEV_CreatePullPointSubscription, &stEV_CreatePullPointSubscriptionRes);
        }
	    if(ret == ONVIF_OK)
	    {
			return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_CreatePullPointSubscription, &stEV_CreatePullPointSubscriptionRes, &stSoap);
		}
	}
	return soap_proc_build_err_rly(p_cln, rx_msg, ret, &stSoap);
}

GK_S32 soap_Event_PullMessages(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body, XMLN *p_header)
{
    ONVIF_RET ret = ONVIF_OK;
    GK_S32 retVal = 0;
    XMLN *p_PullMessages = soap_get_child_node(p_body, "PullMessages");
    if(p_PullMessages == NULL)
    {
        return -1;
    }
    
    GONVIF_Soap_S stSoap;
	GONVIF_EVENT_PullMessages_S stEV_PullMessages;
	GONVIF_EVENT_PullMessages_Res_S stEV_PullMessagesRes;	
    memset(&stSoap, 0, sizeof(GONVIF_Soap_S));
	memset(&stEV_PullMessages, 0, sizeof(stEV_PullMessages));	
	memset(&stEV_PullMessagesRes, 0, sizeof(stEV_PullMessagesRes));
	XMLN *p_To = soap_get_child_node(p_header, "To");
	if (p_To && p_To->data)
	{
		strncpy(stSoap.stHeader.aszTo, p_To->data, MAX_TO_LEN-1);
	}

    XMLN *p_Timeout = soap_get_child_node(p_PullMessages, "Timeout");
	if (p_Timeout && p_Timeout->data)
	{
		stEV_PullMessages.timeout = atoi(p_Timeout->data + 2);
	}
	XMLN *p_MessageLimit = soap_get_child_node(p_PullMessages, "MessageLimit");
	if (p_MessageLimit && p_MessageLimit->data)
	{
		stEV_PullMessages.messageLimit = atoi(p_MessageLimit->data);
	}
    if(g_EventCallBackRegister.pfnEV_PullMessages != NULL)
    {
        ret = g_EventCallBackRegister.pfnEV_PullMessages(&stSoap, &stEV_PullMessages, &stEV_PullMessagesRes);
    }
    else
    {
        ret = GK_NVT_Event_PullMessages(&stSoap, &stEV_PullMessages, &stEV_PullMessagesRes);
    }
    if(ONVIF_OK == ret)
    {
		retVal = soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_PullMessages, NULL, &stSoap);
        if(g_stEventPullMessageRes.pstNotificationMessage)
        {
            free(g_stEventPullMessageRes.pstNotificationMessage);
            g_stEventPullMessageRes.pstNotificationMessage = NULL;
        }
        memset(&g_stEventPullMessageRes, 0, sizeof(g_stEventPullMessageRes));
        return retVal;
	}
    if(g_stEventPullMessageRes.pstNotificationMessage)
    {
        free(g_stEventPullMessageRes.pstNotificationMessage);
        g_stEventPullMessageRes.pstNotificationMessage = NULL;
    }
    memset(&g_stEventPullMessageRes, 0, sizeof(g_stEventPullMessageRes));
    
	return soap_proc_build_err_rly(p_cln, rx_msg, ret, &stSoap);
}

GK_S32 soap_Event_SetSynchronizationPoint(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_header)
{
    ONVIF_RET ret = ONVIF_OK;    
    GONVIF_Soap_S stSoap;
    GONVIF_EVENT_SetSynchronizationPoint_S stEV_SetSynchronizationPoint;
    memset(&stSoap, 0, sizeof(GONVIF_Soap_S));
    memset(&stEV_SetSynchronizationPoint, 0, sizeof(stEV_SetSynchronizationPoint));
    XMLN *p_To = soap_get_child_node(p_header, "To");
	if (p_To && p_To->data)
	{
	    strncpy(stSoap.stHeader.aszTo, p_To->data, MAX_TO_LEN-1);
	}
    if(g_EventCallBackRegister.pfnEV_SetSynchronizationPoint != NULL)
    {
        ret = g_EventCallBackRegister.pfnEV_SetSynchronizationPoint(&stSoap, &stEV_SetSynchronizationPoint);
    }
    else
    {
        ret = GK_NVT_Event_SetSynchronizationPoint(&stSoap, &stEV_SetSynchronizationPoint);
    }
	if(ret == ONVIF_OK)
	{
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_SetSynchronizationPoint, NULL, &stSoap);
	}

	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

/********************* event end **********************/

/*********************** PTZ *************************/

GK_S32 soap_PTZ_GetServiceCapabilities(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetServiceCapabilities = soap_get_child_node(p_body, "GetServiceCapabilities");
    if(p_GetServiceCapabilities == NULL)
    {
        return -1;
    }

	GONVIF_PTZ_GetServiceCapabilities_Res_S stPTZ_GetServiceCapabilitiesRes;
	memset(&stPTZ_GetServiceCapabilitiesRes, 0, sizeof(GONVIF_PTZ_GetServiceCapabilities_Res_S));
	if(g_PtzCallBackRegister.pfnPTZ_GetServiceCapabilities!= NULL)
	{
		ret = g_PtzCallBackRegister.pfnPTZ_GetServiceCapabilities(&stPTZ_GetServiceCapabilitiesRes);
	}
	else
	{
		ret = GK_NVT_PTZ_GetServiceCapabilities(&stPTZ_GetServiceCapabilitiesRes);
	}
	if(ret == ONVIF_OK)
	{
		return	soap_proc_build_send_rly(p_cln, rx_msg, soap_response_PTZ_GetServiceCapabilities, &stPTZ_GetServiceCapabilitiesRes, NULL);
	}

	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);

}

GK_S32 soap_PTZ_GetNodes(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GONVIF_PTZ_GetNodes_Res_S stPTZ_GetNodesRes;
    memset(&stPTZ_GetNodesRes, 0, sizeof(GONVIF_PTZ_GetNodes_Res_S));
    ret = soap_parse_request_GetNodes(p_body);
    if(ret == ONVIF_OK)
    {
	    if(g_PtzCallBackRegister.pfnPTZ_GetNodes != NULL)
	    {
	        ret = g_PtzCallBackRegister.pfnPTZ_GetNodes(&stPTZ_GetNodesRes);
	    }
	    else
	    {
	        ret = GK_NVT_PTZ_GetNodes(&stPTZ_GetNodesRes);
	    }
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetNodes, &stPTZ_GetNodesRes, NULL);
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_PTZ_GetNode(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GONVIF_PTZ_GetNode_S stPTZ_GetNode;    
    GONVIF_PTZ_GetNode_Res_S stPTZ_GetNodeRes;
    memset(&stPTZ_GetNode, 0, sizeof(GONVIF_PTZ_GetNode_S));
    memset(&stPTZ_GetNodeRes, 0, sizeof(GONVIF_PTZ_GetNode_Res_S));
    ret = soap_parse_request_GetNode(p_body, &stPTZ_GetNode);
    if(ret == ONVIF_OK)
    {
        if(g_PtzCallBackRegister.pfnPTZ_GetNode != NULL)
        {
            ret = g_PtzCallBackRegister.pfnPTZ_GetNode(&stPTZ_GetNode, &stPTZ_GetNodeRes);
        }
        else
        {
            ret = GK_NVT_PTZ_GetNode(&stPTZ_GetNode, &stPTZ_GetNodeRes);
        }
        if(ret == ONVIF_OK)
        {
            return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetNode, &stPTZ_GetNodeRes, NULL);
        }
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_PTZ_GetConfigurations(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetConfigurations = soap_get_child_node(p_body, "GetConfigurations");
    if(p_GetConfigurations == NULL)
    {
        return -1;
    }

    GONVIF_PTZ_GetConfigutations_Res_S stGetConfigurationsRes;
    memset(&stGetConfigurationsRes, 0, sizeof(stGetConfigurationsRes));
    if(g_PtzCallBackRegister.pfnPTZ_GetConfigurations != NULL)
    {
        ret = g_PtzCallBackRegister.pfnPTZ_GetConfigurations(&stGetConfigurationsRes);
    }
    else
    {
        ret = GK_NVT_PTZ_GetConfigurations(&stGetConfigurationsRes);
    }

    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetConfigurations, &stGetConfigurationsRes, NULL);
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}


GK_S32 soap_PTZ_GetConfiguration(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GONVIF_PTZ_GetConfiguration_S stPTZ_GetConfiguration;
    GONVIF_PTZ_GetConfiguration_Res_S stPTZ_GetConfigurationRes;
    memset(&stPTZ_GetConfiguration, 0, sizeof(GONVIF_PTZ_GetConfiguration_S));    
    memset(&stPTZ_GetConfigurationRes, 0, sizeof(GONVIF_PTZ_GetConfiguration_Res_S));
    ret = soap_parse_request_GetConfiguration(p_body, &stPTZ_GetConfiguration);
    if(ret == ONVIF_OK)
    {
        if(g_PtzCallBackRegister.pfnPTZ_GetConfiguration != NULL)
        {
            ret = g_PtzCallBackRegister.pfnPTZ_GetConfiguration(&stPTZ_GetConfiguration, &stPTZ_GetConfigurationRes);
        }
        else
        {
            ret = GK_NVT_PTZ_GetConfiguration(&stPTZ_GetConfiguration, &stPTZ_GetConfigurationRes);
        }
        if(ret == ONVIF_OK)
        {
            return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetConfiguration, &stPTZ_GetConfigurationRes, NULL);
        }
    }
    
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_PTZ_GetConfigurationOptions(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GONVIF_PTZ_GetConfigurationOptions_S stPTZ_GetConfigurationOptions;    
    GONVIF_PTZ_GetConfigurationOptions_Res_S stPTZ_GetConfigurationOptionsRes;
    memset(&stPTZ_GetConfigurationOptions, 0, sizeof(GONVIF_PTZ_GetConfigurationOptions_S));
    memset(&stPTZ_GetConfigurationOptionsRes, 0, sizeof(GONVIF_PTZ_GetConfigurationOptions_Res_S));
    ret = soap_parse_request_GetConfigurationOptions(p_body, &stPTZ_GetConfigurationOptions);
    if(ret == ONVIF_OK)
    {
        if(g_PtzCallBackRegister.pfnPTZ_GetConfigurationOptions != NULL)
        {
            ret = g_PtzCallBackRegister.pfnPTZ_GetConfigurationOptions(&stPTZ_GetConfigurationOptions, &stPTZ_GetConfigurationOptionsRes);
        }
        else
        {
            ret = GK_NVT_PTZ_GetConfigurationOptions(&stPTZ_GetConfigurationOptions, &stPTZ_GetConfigurationOptionsRes);
        }
        if(ret == ONVIF_OK)
        {
            return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetConfigurationOptions, &stPTZ_GetConfigurationOptionsRes, NULL);
        }
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_PTZ_SetConfiguration(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GONVIF_PTZ_SetConfiguration_S stPTZ_SetConfiguration;
    memset(&stPTZ_SetConfiguration, 0, sizeof(GONVIF_PTZ_SetConfiguration_S));
    ret = soap_parse_request_SetConfiguration(p_body, &stPTZ_SetConfiguration);
    if(ret == ONVIF_OK)
    {
        if(g_PtzCallBackRegister.pfnPTZ_SetConfiguration != NULL)
        {
            ret = g_PtzCallBackRegister.pfnPTZ_SetConfiguration(&stPTZ_SetConfiguration);
        }
        else
        {
            ret = GK_NVT_PTZ_SetConfiguration(&stPTZ_SetConfiguration);
        }
        if(ret == ONVIF_OK)
        {
            return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_SetConfiguration, NULL, NULL);
        }
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_PTZ_ContinuousMove(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GONVIF_PTZ_ContinuousMove_S stPTZ_ContinuousMove;
    memset(&stPTZ_ContinuousMove, 0, sizeof(GONVIF_PTZ_ContinuousMove_S));
    ret = soap_parse_request_ContinuousMove(p_body, &stPTZ_ContinuousMove);
    if(ret == ONVIF_OK)
    {
        if(g_PtzCallBackRegister.pfnPTZ_ContinuousMove != NULL)
        {
            ret = g_PtzCallBackRegister.pfnPTZ_ContinuousMove(&stPTZ_ContinuousMove);
        }
        else
        {
            ret = GK_NVT_PTZ_ContinuousMove(&stPTZ_ContinuousMove);
        }
        if(ret == ONVIF_OK)
        {
            return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_ContinuousMove, NULL, NULL);
        }
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_PTZ_Stop(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_Stop = soap_get_child_node(p_body, "Stop");
	if(p_Stop == NULL)
    {
        return -1;
    }
    
	GONVIF_PTZ_Stop_S stPTZ_Stop;
	memset(&stPTZ_Stop, 0, sizeof(GONVIF_PTZ_Stop_S));
	ret = soap_parse_request_PtzStop(p_Stop, &stPTZ_Stop);
	if(ret == ONVIF_OK)
	{
        if(g_PtzCallBackRegister.pfnPTZ_Stop != NULL)
        {
            ret = g_PtzCallBackRegister.pfnPTZ_Stop(&stPTZ_Stop);
        }
        else
        {
            ret = GK_NVT_PTZ_Stop(&stPTZ_Stop);
        }
		if(ret == ONVIF_OK)
		{
			return soap_proc_build_send_rly(p_cln, rx_msg, soap_response_PTZ_Stop, NULL, NULL);
		}
	}
    
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_PTZ_GetPresets(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetPresets = soap_get_child_node(p_body, "GetPresets");
	if(p_GetPresets == NULL)
    {
        return -1;
    }
    GONVIF_PTZ_GetPresets_S stPTZ_GetPresets;
    GONVIF_PTZ_GetPresets_Res_S stPTZ_GetPresetsRes;
	memset(&stPTZ_GetPresets, 0, sizeof(GONVIF_PTZ_GetPresets_S));
	memset(&stPTZ_GetPresetsRes, 0, sizeof(GONVIF_PTZ_GetPresets_Res_S));
    XMLN *p_ProfileToken = soap_get_child_node(p_GetPresets, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
        strncpy(stPTZ_GetPresets.aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
	}
    if(g_PtzCallBackRegister.pfnPTZ_GetPresets != NULL)
    {
        ret = g_PtzCallBackRegister.pfnPTZ_GetPresets(&stPTZ_GetPresets, &stPTZ_GetPresetsRes);
    }
    else
    {
        ret = GK_NVT_PTZ_GetPresets(&stPTZ_GetPresets, &stPTZ_GetPresetsRes);
    }
    if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetPresets, &stPTZ_GetPresetsRes, NULL);
	}
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_PTZ_SetPreset(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_SetPreset = soap_get_child_node(p_body, "SetPreset");
	if(p_SetPreset == NULL)
    {
        return -1;
    }

	GONVIF_PTZ_SetPreset_S stPTZ_SetPreset;
    GONVIF_PTZ_SetPreset_Res_S stPTZ_SetPresetRes;
	memset(&stPTZ_SetPreset, 0, sizeof(GONVIF_PTZ_SetPreset_S));
	memset(&stPTZ_SetPresetRes, 0, sizeof(GONVIF_PTZ_SetPreset_Res_S));
	
	XMLN *p_ProfileToken = soap_get_child_node(p_SetPreset, "ProfileToken");
	if(p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(stPTZ_SetPreset.aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
	}
	else
	{
		return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
	}
	XMLN *p_PresetName = soap_get_child_node(p_SetPreset, "PresetName");//optional
	if(p_PresetName && p_PresetName->data)
	{
		strncpy(stPTZ_SetPreset.aszPresetName, p_PresetName->data, MAX_NAME_LENGTH - 1);
	}

	XMLN *p_PresetToken = soap_get_child_node(p_SetPreset, "PresetToken");//optionnal
	if(p_PresetToken && p_PresetToken->data)
	{
		strncpy(stPTZ_SetPreset.aszPresetToken, p_PresetToken->data, MAX_TOKEN_LENGTH - 1);
	}
	
    if(g_PtzCallBackRegister.pfnPTZ_SetPreset != NULL)
    {
        ret = g_PtzCallBackRegister.pfnPTZ_SetPreset(&stPTZ_SetPreset, &stPTZ_SetPresetRes);
    }
    else
    {
        ret = GK_NVT_PTZ_SetPreset(&stPTZ_SetPreset, &stPTZ_SetPresetRes);
    }
    if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_SetPreset, &stPTZ_SetPresetRes, NULL);
	}

	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_PTZ_GotoPreset(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GotoPreset = soap_get_child_node(p_body, "GotoPreset");
	if(p_GotoPreset == NULL)
    {
        return -1;
    }

	GONVIF_PTZ_GotoPreset_S stPTZ_GotoPreset;
	memset(&stPTZ_GotoPreset, 0, sizeof(GONVIF_PTZ_GotoPreset_S));
	ret = soap_parse_request_GotoPreset(p_GotoPreset, &stPTZ_GotoPreset);
	if(ret == ONVIF_OK)
	{
        if(g_PtzCallBackRegister.pfnPTZ_GotoPreset != NULL)
        {
            ret = g_PtzCallBackRegister.pfnPTZ_GotoPreset(&stPTZ_GotoPreset);
        }
        else
        {
            ret = GK_NVT_PTZ_GotoPreset(&stPTZ_GotoPreset);
        }
        if(ret == ONVIF_OK)
		{
			return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GotoPreset, NULL, NULL);
		}
	}
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_PTZ_RemovePreset(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_RemovePreset = soap_get_child_node(p_body, "RemovePreset");
	if(p_RemovePreset == NULL)
    {
        return -1;
    }

	GONVIF_PTZ_RemovePreset_S stPTZ_RemovePreset;
	memset(&stPTZ_RemovePreset, 0, sizeof(GONVIF_PTZ_RemovePreset_S));

    XMLN *p_ProfileToken = soap_get_child_node(p_RemovePreset, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(stPTZ_RemovePreset.aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
	}

	XMLN *p_PresetToken = soap_get_child_node(p_RemovePreset, "PresetToken");
	if (p_PresetToken && p_PresetToken->data)
	{
		strncpy(stPTZ_RemovePreset.aszPresetToken, p_PresetToken->data, MAX_TOKEN_LENGTH - 1);
	}

    if(g_PtzCallBackRegister.pfnPTZ_RemovePreset != NULL)
    {
        ret = g_PtzCallBackRegister.pfnPTZ_RemovePreset(&stPTZ_RemovePreset);
    }
    else
    {
        ret = GK_NVT_PTZ_RemovePreset(&stPTZ_RemovePreset);
    }
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_RemovePreset, NULL, NULL);
	}

	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_PTZ_AbsoluteMove(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GONVIF_PTZ_AbsoluteMove_S stPTZ_AbsoluteMove;
    memset(&stPTZ_AbsoluteMove, 0, sizeof(GONVIF_PTZ_AbsoluteMove_S));
    ret = soap_parse_request_AbsoluteMove(p_body, &stPTZ_AbsoluteMove);
    if(ret == ONVIF_OK)
    {
        if(g_PtzCallBackRegister.pfnPTZ_AbsoluteMove != NULL)
        {
            ret = g_PtzCallBackRegister.pfnPTZ_AbsoluteMove(&stPTZ_AbsoluteMove);
        }
        else
        {
            ret = GK_NVT_PTZ_AbsoluteMove(&stPTZ_AbsoluteMove);
        }
        if(ret == ONVIF_OK)
        {
            return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_AbsoluteMove, NULL, NULL);
        }
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_PTZ_RelativeMove(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GONVIF_PTZ_RelativeMove_S stPTZ_RelativeMove;
    memset(&stPTZ_RelativeMove, 0, sizeof(GONVIF_PTZ_RelativeMove_S));
    ret = soap_parse_request_RelativeMove(p_body, &stPTZ_RelativeMove);
    if(ret == ONVIF_OK)
    {
        if(g_PtzCallBackRegister.pfnPTZ_RelativeMove != NULL)
        {
            ret = g_PtzCallBackRegister.pfnPTZ_RelativeMove(&stPTZ_RelativeMove);
        }
        else
        {
            ret = GK_NVT_PTZ_RelativeMove(&stPTZ_RelativeMove);
        }
        if(ret == ONVIF_OK)
        {
            return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_RelativeMove, NULL, NULL);
        }
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);

}

GK_S32 soap_PTZ_GetStatus(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_GetStatus = soap_get_child_node(p_body, "GetStatus");
	if(p_GetStatus == NULL)
    {
        return -1;
    }
    
	GONVIF_PTZ_GetStatus_Token_S stPtzGetStatusReq;
	GONVIF_PTZ_GetStatus_S stPtzGetStatusRes;
	memset(&stPtzGetStatusReq, 0, sizeof(stPtzGetStatusReq));
	memset(&stPtzGetStatusRes, 0, sizeof(stPtzGetStatusRes));
	XMLN *p_ProfileToken = soap_get_child_node(p_GetStatus, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(stPtzGetStatusReq.aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH-1);
	}
	
	if(g_PtzCallBackRegister.pfnGetStatus != NULL)
	{
		ret = g_PtzCallBackRegister.pfnGetStatus(&stPtzGetStatusReq,&stPtzGetStatusRes);
	}
	else
	{
		ret = GK_NVT_PTZ_GetStatus(&stPtzGetStatusReq,&stPtzGetStatusRes);
	}
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_response_PTZ_GetStatus, &stPtzGetStatusRes, NULL);
	}
	
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);	
}

/***************** PTZ end *******************/

/******************* image ********************/

GK_S32 soap_Imaging_GetServiceCapabilities(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetServiceCapabilities = soap_get_child_node(p_body, "GetServiceCapabilities");
    if(p_GetServiceCapabilities == NULL)
    {
        return -1;
    }
    
	GONVIF_IMAGING_GetServiceCapabilities_Res_S stIMG_GetServiceCapabilitiesRes;
	memset(&stIMG_GetServiceCapabilitiesRes, 0, sizeof(GONVIF_IMAGING_GetServiceCapabilities_Res_S));
	if(g_ImageCallBackRegister.pfnIMG_GetServiceCapabilities!= NULL)
	{
		ret = g_ImageCallBackRegister.pfnIMG_GetServiceCapabilities(&stIMG_GetServiceCapabilitiesRes);
	}
	else
	{
		ret = GK_NVT_Imaging_GetServiceCapabilities(&stIMG_GetServiceCapabilitiesRes);
	}
	if(ret == ONVIF_OK)
	{
		return	soap_proc_build_send_rly(p_cln, rx_msg, soap_response_Imaging_GetServiceCapabilities, &stIMG_GetServiceCapabilitiesRes, NULL);
	}
	
	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Imaging_GetImagingSettings(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GONVIF_IMAGE_GetSettings_Token_S stGetSettingsReq;
    memset(&stGetSettingsReq, 0, sizeof(stGetSettingsReq));
    ret = soap_parse_request_GetImagingSettings(p_body, &stGetSettingsReq);
    if(ret != ONVIF_OK)
    {
        return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
    }

    GONVIF_IMAGE_GetSettings_S stGetImagingSettingsRes;
    memset(&stGetImagingSettingsRes, 0, sizeof(stGetImagingSettingsRes));

    if(g_ImageCallBackRegister.pfnGetImagingSettings != NULL)
    {
        ret = g_ImageCallBackRegister.pfnGetImagingSettings(&stGetSettingsReq, &stGetImagingSettingsRes);
    }
    else
    {
        ret = GK_NVT_Image_GetImagingSettings(&stGetSettingsReq, &stGetImagingSettingsRes);
    }

    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetImagingSettings, &stGetImagingSettingsRes, NULL);
    }
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);


}

GK_S32 soap_Imaging_SetImagingSettings(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;

    GONVIF_IMAGE_SetSettings_S stSetImagingSettingsReq;
    memset(&stSetImagingSettingsReq, 0, sizeof(stSetImagingSettingsReq));
    GK_NVT_Image_GetImaging(&stSetImagingSettingsReq.stImageSettings);
    ret = soap_parse_request_SetImagingSettings(p_body, &stSetImagingSettingsReq);
    if(ret != ONVIF_OK)
    {
        return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
    }
    if(g_ImageCallBackRegister.pfnSetImagingSettings != NULL)
    {
        ret = g_ImageCallBackRegister.pfnSetImagingSettings(&stSetImagingSettingsReq);
    }
    else
    {
        ret = GK_NVT_Image_SetImagingSettings(&stSetImagingSettingsReq);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_SetImagingSettings, NULL, NULL);
    }
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}


GK_S32 soap_Imaging_GetOptions(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GONVIF_IMAGE_GetOptions_Token_S stGetImageOptionsReq;
    memset(&stGetImageOptionsReq, 0, sizeof(stGetImageOptionsReq));
    ret = soap_parse_request_GetOptions(p_body, &stGetImageOptionsReq);
    if(ret != ONVIF_OK)
    {
        return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
    }

    GONVIF_IMAGE_GetOptions_S stGetImageOptionsRes;
    memset(&stGetImageOptionsRes, 0, sizeof(stGetImageOptionsRes));
    if(g_ImageCallBackRegister.pfnGetOptions != NULL)
    {
        ret = g_ImageCallBackRegister.pfnGetOptions(&stGetImageOptionsReq, &stGetImageOptionsRes);
    }
    else
    {
        ret = GK_NVT_Image_GetOptions(&stGetImageOptionsReq, &stGetImageOptionsRes);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetOptions, &stGetImageOptionsRes, NULL);
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Imaging_GetMoveOptions(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;

    GONVIF_IMAGE_GetMoveOptions_Token_S stGetMoveOptionsReq;
    memset(&stGetMoveOptionsReq, 0, sizeof(stGetMoveOptionsReq));
    ret = soap_parse_request_GetMoveOptions(p_body, &stGetMoveOptionsReq);
    if(ret != ONVIF_OK)
    {
        return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
    }

    GONVIF_IMAGE_GetMoveOptions_S stGetMoveOptionsRes;
    memset(&stGetMoveOptionsRes, 0, sizeof(stGetMoveOptionsRes));
    if(g_ImageCallBackRegister.pfnGetMoveOptions != NULL)
    {
        ret = g_ImageCallBackRegister.pfnGetMoveOptions(&stGetMoveOptionsReq, &stGetMoveOptionsRes);
    }
    else
    {
        ret = GK_NVT_Image_GetMoveOptions(&stGetMoveOptionsReq, &stGetMoveOptionsRes);
    }
    if(ret == ONVIF_OK)
    {
        return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetMoveOptions, &stGetMoveOptionsRes, NULL);
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Imaging_Move(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    GK_S32 retVal = 0;
    GONVIF_IMAGE_Move_S stMoveReq;
    memset(&stMoveReq, 0, sizeof(stMoveReq));
    ret = soap_parse_request_Move(p_body, &stMoveReq);
    if(ret == ONVIF_OK)
    {
        if(g_ImageCallBackRegister.pfnMove != NULL)
        {
            ret = g_ImageCallBackRegister.pfnMove(&stMoveReq);
        }
        else
        {
            ret = GK_NVT_Image_Move(&stMoveReq);
        }
        if(ret == ONVIF_OK)
        {
            retVal = soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_Move, NULL, NULL);
            if(stMoveReq.stFocus.stContinuous != NULL)
            {
                if(stMoveReq.stFocus.stContinuous->speed != NULL)
                {
                    free(stMoveReq.stFocus.stContinuous->speed);
                    stMoveReq.stFocus.stContinuous->speed = NULL;
                }
                free(stMoveReq.stFocus.stContinuous);
                stMoveReq.stFocus.stContinuous = NULL;
            }
            if(stMoveReq.stFocus.stRelative != NULL)
            {
                if(stMoveReq.stFocus.stRelative->speed != NULL)
                {
                    free(stMoveReq.stFocus.stRelative->speed);
                    stMoveReq.stFocus.stRelative->speed = NULL;
                }
                free(stMoveReq.stFocus.stRelative->distance);
                stMoveReq.stFocus.stRelative->distance = NULL;
            
                free(stMoveReq.stFocus.stRelative);
                stMoveReq.stFocus.stRelative = NULL;
            }
            if(stMoveReq.stFocus.stAbsolute != NULL)
            {
                if(stMoveReq.stFocus.stAbsolute->speed != NULL)
                {
                    free(stMoveReq.stFocus.stAbsolute->speed);
                    stMoveReq.stFocus.stAbsolute->speed = NULL;
                }
                free(stMoveReq.stFocus.stAbsolute->position);
                stMoveReq.stFocus.stAbsolute->position = NULL;
            
                free(stMoveReq.stFocus.stAbsolute);
                stMoveReq.stFocus.stAbsolute = NULL;
            }
            return retVal;
        }

    }
    if(stMoveReq.stFocus.stContinuous != NULL)
    {
        if(stMoveReq.stFocus.stContinuous->speed != NULL)
        {
            free(stMoveReq.stFocus.stContinuous->speed);
            stMoveReq.stFocus.stContinuous->speed = NULL;
        }
        free(stMoveReq.stFocus.stContinuous);
        stMoveReq.stFocus.stContinuous = NULL;
    }
    if(stMoveReq.stFocus.stRelative != NULL)
    {
        if(stMoveReq.stFocus.stRelative->speed != NULL)
        {
            free(stMoveReq.stFocus.stRelative->speed);
            stMoveReq.stFocus.stRelative->speed = NULL;
        }
        free(stMoveReq.stFocus.stRelative->distance);
        stMoveReq.stFocus.stRelative->distance = NULL;
    
        free(stMoveReq.stFocus.stRelative);
        stMoveReq.stFocus.stRelative = NULL;
    }
    if(stMoveReq.stFocus.stAbsolute != NULL)
    {
        if(stMoveReq.stFocus.stAbsolute->speed != NULL)
        {
            free(stMoveReq.stFocus.stAbsolute->speed);
            stMoveReq.stFocus.stAbsolute->speed = NULL;
        }
        free(stMoveReq.stFocus.stAbsolute->position);
        stMoveReq.stFocus.stAbsolute->position = NULL;
    
        free(stMoveReq.stFocus.stAbsolute);
        stMoveReq.stFocus.stAbsolute = NULL;
    }
    
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Imaging_GetStatus(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_GetStatus = soap_get_child_node(p_body, "GetStatus");
	if(p_GetStatus == NULL)
    {
        return -1;
    }

    GONVIF_IMAGE_GetStatus_Token_S stImageGetStatusReq;
    memset(&stImageGetStatusReq, 0, sizeof(stImageGetStatusReq));
    ret = soap_parse_request_GetStatus(p_body, &stImageGetStatusReq);
    if(ret == ONVIF_OK)
    {
        GONVIF_IMAGE_GetStatus_S stImageGetStatusRes;
        memset(&stImageGetStatusRes, 0, sizeof(stImageGetStatusRes));
        if(g_ImageCallBackRegister.pfnGetStatus != NULL)
        {
            ret = g_ImageCallBackRegister.pfnGetStatus(&stImageGetStatusReq,&stImageGetStatusRes);
        }
        else
        {
            ret = GK_NVT_Image_GetStatus(&stImageGetStatusReq, &stImageGetStatusRes);
        }
        if(ret == ONVIF_OK)
        {
            return soap_proc_build_send_rly(p_cln, rx_msg, soap_response_Imaging_GetStatus, &stImageGetStatusRes, NULL);
        }
    }

    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_Imaging_Stop(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
	XMLN *p_Stop = soap_get_child_node(p_body, "Stop");
	if(p_Stop == NULL)
    {
        return -1;
    }

    GONVIF_IMAGE_Stop_Token_S stImageStopReq;
    memset(&stImageStopReq, 0, sizeof(stImageStopReq));
    ret = soap_parse_request_ImageStop(p_body, &stImageStopReq);
    if(ret == ONVIF_OK)
    {
        if(g_ImageCallBackRegister.pfnPTZ_Stop != NULL)
        {
            ret = g_ImageCallBackRegister.pfnPTZ_Stop(&stImageStopReq);
        }
        else
        {
            ret = GK_NVT_Image_Stop(&stImageStopReq);
        }
        if(ret == ONVIF_OK)
        {
            return soap_proc_build_send_rly(p_cln, rx_msg, soap_response_Imaging_Stop, NULL, NULL);
        }
    }
 
    return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

/***************** imaging end *******************/

/**************** VideoAnalitics *******************/

GK_S32 soap_VideoAnalytics_GetServiceCapabilities(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;
    XMLN *p_GetServiceCapabilities = soap_get_child_node(p_body, "GetServiceCapabilities");
    if(p_GetServiceCapabilities == NULL)
    {
        return -1;
    }

	GONVIF_ANALYTICS_GetServiceCapabilities_Res_S stAN_GetServiceCapabilitiesRes;
	memset(&stAN_GetServiceCapabilitiesRes, 0, sizeof(stAN_GetServiceCapabilitiesRes));
	if(g_AnalyticsCallBackRegister.pfnAN_GetServiceCapabilities != NULL)
	{
		ret = g_AnalyticsCallBackRegister.pfnAN_GetServiceCapabilities(&stAN_GetServiceCapabilitiesRes);
	}
	else
	{
		ret = GK_NVT_Analytics_GetServiceCapabilities(&stAN_GetServiceCapabilitiesRes);
	}
	if(ret == ONVIF_OK)
	{
		return	soap_proc_build_send_rly(p_cln, rx_msg, soap_response_VideoAnalytics_GetServiceCapabilities, &stAN_GetServiceCapabilitiesRes, NULL);
	}		 

	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);

}

GK_S32 soap_VideoAnalytics_GetSupportedAnalyticsModules(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
    ONVIF_RET ret = ONVIF_OK;

	GONVIF_ANALYTICS_GetSupportedAnalyticsModules_S stGetSupportedAnalyticsModulesReq;
	memset(&stGetSupportedAnalyticsModulesReq, 0, sizeof(stGetSupportedAnalyticsModulesReq));
	GONVIF_ANALYTICS_GetSupportedAnalyticsModulesRes_S stGetSupportedAnalyticsModulesRes;
	memset(&stGetSupportedAnalyticsModulesRes, 0, sizeof(stGetSupportedAnalyticsModulesRes));

    ret = soap_parse_request_GetSupportedAnalyticsModules(p_body, &stGetSupportedAnalyticsModulesReq);
    if(ret != ONVIF_OK)
    {
        return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
    }

    if(g_AnalyticsCallBackRegister.pfnGetSupportedAnalyticsModules != NULL)
    {
        ret = g_AnalyticsCallBackRegister.pfnGetSupportedAnalyticsModules(&stGetSupportedAnalyticsModulesReq, &stGetSupportedAnalyticsModulesRes);
    }
    else
    {
        ret = GK_NVT_Analytics_GetSupportedAnalyticsModules(&stGetSupportedAnalyticsModulesReq, &stGetSupportedAnalyticsModulesRes);
    }
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetSupportedAnalyticsModules, &stGetSupportedAnalyticsModulesRes, NULL);
	}

	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_VideoAnalytics_GetAnalyticsModules(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
	ONVIF_RET ret = ONVIF_OK;

	GONVIF_ANALYTICS_GetAnalyticsModules_S stGetAnalyticsModulesReq;
	memset(&stGetAnalyticsModulesReq, 0, sizeof(stGetAnalyticsModulesReq));
	GONVIF_ANALYTICS_GetAnalyticsModulesRes_S stGetAnalyticsModulesRes;
	memset(&stGetAnalyticsModulesRes, 0, sizeof(stGetAnalyticsModulesRes));

    ret = soap_parse_request_GetAnalyticsModules(p_body, &stGetAnalyticsModulesReq);
    if(ret != ONVIF_OK)
    {
        return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
    }
	if(g_AnalyticsCallBackRegister.pfnGetAnalyticsModules != NULL)
	{
		ret = g_AnalyticsCallBackRegister.pfnGetAnalyticsModules(&stGetAnalyticsModulesReq, &stGetAnalyticsModulesRes);
	}
	else
	{
		ret = GK_NVT_Analytics_GetAnalyticsModules(&stGetAnalyticsModulesReq, &stGetAnalyticsModulesRes);
	}
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetAnalyticsModules, &stGetAnalyticsModulesRes, NULL);
	}

	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
}

GK_S32 soap_VideoAnalytics_GetSupportedRules(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
	ONVIF_RET ret = ONVIF_OK;

	GONVIF_ANALYTICS_GetSupportedRules_S stGetSupportedRulesReq;
	memset(&stGetSupportedRulesReq, 0, sizeof(stGetSupportedRulesReq));
	GONVIF_ANALYITICS_GetSupportedRulesRes_S stGetSupportedRulesRes;
	memset(&stGetSupportedRulesRes, 0, sizeof(stGetSupportedRulesRes));

    ret = soap_parse_request_GetSupportedRules(p_body, &stGetSupportedRulesReq);
    if(ret != ONVIF_OK)
    {
        return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
    }
	if(g_AnalyticsCallBackRegister.pfnGetSupportedRules != NULL)
	{
		ret = g_AnalyticsCallBackRegister.pfnGetSupportedRules(&stGetSupportedRulesReq, &stGetSupportedRulesRes);
	}
	else
	{
		ret = GK_NVT_Analytics_GetSupportedRules(&stGetSupportedRulesReq, &stGetSupportedRulesRes);
	}
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetSupportedRules, &stGetSupportedRulesRes, NULL);
	}

	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);

}

GK_S32 soap_VideoAnalytics_GetRules(HTTPCLN *p_cln, HTTPMSG *rx_msg, XMLN *p_body)
{
	ONVIF_RET ret = ONVIF_OK;

	GONVIF_ANALYTICS_GetRules_S stGetRulesReq;
	memset(&stGetRulesReq, 0, sizeof(stGetRulesReq));
	GONVIF_ANALYTICS_GetRulesRes_s stGetRulesRes;
	memset(&stGetRulesRes, 0, sizeof(stGetRulesRes));

    ret = soap_parse_request_GetRules(p_body, &stGetRulesReq);
    if(ret != ONVIF_OK)
    {
        return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);
    }
	if(g_AnalyticsCallBackRegister.pfnGetRules != NULL)
	{
		ret = g_AnalyticsCallBackRegister.pfnGetRules(&stGetRulesReq, &stGetRulesRes);
	}
	else
	{
		ret = GK_NVT_Analytics_GetRules(&stGetRulesReq, &stGetRulesRes);
	}
	if(ret == ONVIF_OK)
	{
		return soap_proc_build_send_rly(p_cln, rx_msg, soap_packet_response_GetRules, &stGetRulesRes, NULL);
	}

	return soap_proc_build_err_rly(p_cln, rx_msg, ret, NULL);

}


/*************************************************************************************************************/
static void soap_proc_calc_digest(const GK_CHAR *created, GK_U8 *nonce, GK_S32 noncelen, const GK_CHAR *password, GK_U8 hash[20])
{
	sha1_context ctx;

	sha1_starts(&ctx);
	sha1_update(&ctx, (GK_U8 *)nonce, noncelen);
	sha1_update(&ctx, (GK_U8 *)created, strlen(created));
	sha1_update(&ctx, (GK_U8 *)password, strlen(password));
	sha1_finish(&ctx, (GK_U8 *)hash);
}

static GK_BOOL soap_proc_auth_process(XMLN *p_Security)
{
	int ret = 0;
	XMLN *p_UsernameToken = soap_get_child_node(p_Security, "wsse:UsernameToken");
	if (NULL == p_UsernameToken)
	{
		return GK_FALSE;
	}

	XMLN *p_Username = soap_get_child_node(p_UsernameToken, "wsse:Username");
	XMLN *p_Password = soap_get_child_node(p_UsernameToken, "wsse:Password");
	XMLN *p_Nonce = soap_get_child_node(p_UsernameToken, "wsse:Nonce");
	XMLN *p_Created = soap_get_child_node(p_UsernameToken, "wsse:Created");

	if (NULL == p_Username || NULL == p_Username->data ||
		NULL == p_Password || NULL == p_Password->data ||
		NULL == p_Nonce || NULL == p_Nonce->data ||
		NULL == p_Created || NULL == p_Created->data)
	{
		return GK_FALSE;
	}
	ret = devMng_SynUsers();
	if(ret != 0)
	{
		ONVIF_ERR("Fail to synchronise user info");
		return GK_FALSE;
	}

	const GK_CHAR *auth_pass = (const GK_CHAR *)config_GetPassword(p_Username->data);
	if (NULL == auth_pass)	// user not exist
	{
		return GK_FALSE;
	}

	GK_U8 nonce[200];
	memset(nonce, 0, sizeof(nonce));
	GK_S32 nonce_len = onvif_base64_decode(p_Nonce->data, nonce, sizeof(nonce));
	GK_U8 HA[20];
	memset(HA, 0, sizeof(HA));
	soap_proc_calc_digest(p_Created->data, nonce, nonce_len, auth_pass, HA);	
	GK_CHAR HABase64[100];
	memset(HABase64, 0, sizeof(HABase64));
	onvif_base64_encode(HA, 20, HABase64, sizeof(HABase64));

	if (strcmp(HABase64, p_Password->data) == 0)
	{
		return GK_TRUE;
	}

	return GK_FALSE;
}

ONVIF_Http_package_S *soap_proc_GetHttpPackage(void)
{
    return &stHttpPackage;
}

GK_S32 soap_proc_ReleaseHttpPackage(void)
{
	if(stHttpPackage.buffer)
	{
    	free(stHttpPackage.buffer);
    	stHttpPackage.buffer = NULL;
	}
    return 0;
}

static GK_S32 soap_proc_http_rly(HTTPCLN *p_cln, HTTPMSG *rx_msg, const GK_CHAR *p_xml, GK_S32 len)
{
	GK_CHAR *p_bufs = (GK_CHAR *)malloc(len + 1024);
	if (NULL == p_bufs)
	{
		return -1;
	}

	GK_S32 tlen = sprintf(p_bufs,"HTTP/1.1 200 OK\r\n"
								 "Content-Type: %s\r\n"
								 "Content-Length: %d\r\n"
								 "Connection: close\r\n\r\n",
								 http_parse_get_headline(rx_msg, "Content-Type"), len);

	memcpy(p_bufs+tlen, p_xml, len);
	tlen += len;
	
#ifdef BOA_OR_SELF_WEBSERVER
	send(p_cln->cfd, p_bufs, tlen, 0);
	free(p_bufs);
#else
    //ONVIF_INFO("%s\n\n%d\n\n", p_bufs, tlen);
    memset(&stHttpPackage, 0, sizeof(ONVIF_Http_package_S));
    stHttpPackage.buffer = p_bufs;
    stHttpPackage.Len    = tlen;
#endif  

	return tlen;
}

static GK_S32 soap_proc_http_err_rly(HTTPCLN *p_cln, HTTPMSG *rx_msg, GK_S32 err_code, const GK_CHAR *err_str, const GK_CHAR *p_xml, GK_S32 len)
{
	GK_CHAR *p_bufs = (GK_CHAR *)malloc(1024 * 16);
	if (NULL == p_bufs)
	{
		return -1;
	}

	GK_S32 tlen = sprintf(p_bufs,"HTTP/1.1 %d %s\r\n"
								 "Content-Type: %s\r\n"
								 "Content-Length: %d\r\n"
								 "Connection: close\r\n\r\n",
								err_code, err_str,
								http_parse_get_headline(rx_msg, "Content-Type"), len);

	memcpy(p_bufs+tlen, p_xml, len);
	tlen += len;
	
#ifdef BOA_OR_SELF_WEBSERVER
	send(p_cln->cfd, p_bufs, tlen, 0);
    free(p_bufs);
#else
    memset(&stHttpPackage, 0, sizeof(ONVIF_Http_package_S));
    stHttpPackage.buffer = p_bufs;
    stHttpPackage.Len    = tlen;
#endif
	return tlen;
}

static GK_S32 soap_proc_err_rly(HTTPCLN *p_cln, HTTPMSG *rx_msg, const GK_CHAR *code, const GK_CHAR *subcode,
	const GK_CHAR *subcode_ex,const GK_CHAR *reason, GK_S32 http_err_code, const GK_CHAR *http_err_str, const GK_CHAR *header)
{
	ONVIF_DBG("soap process error!!!\n");

	GK_S32 ret = -1, mlen = 1024*16, xlen;

	GK_CHAR *p_xml = (GK_CHAR *)malloc(mlen);
	if (NULL == p_xml)
	{
		goto soap_rly_err;
	}

	xlen = soap_response_error(p_xml, mlen, code, subcode, subcode_ex, reason, header);
	if (xlen < 0 || xlen >= mlen)
	{
		goto soap_rly_err;
	}

	ret = soap_proc_http_err_rly(p_cln, rx_msg, http_err_code, http_err_str, p_xml, xlen);

soap_rly_err:

	if (p_xml)
	{
		free(p_xml);
	}

	return ret;
}

static GK_S32 soap_proc_security_rly(HTTPCLN *p_cln, HTTPMSG *rx_msg)
{
	ONVIF_ERR("Sender not Authorized!\n");

    return soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_NOTAUTHORIZED, NULL, "Sender not Authorized", 401, "Unauthorized", NULL);
}

static GK_S32 soap_proc_build_err_rly(HTTPCLN *p_cln, HTTPMSG *rx_msg, ONVIF_RET err, const void *header)
{
	GK_S32 ret = 0;

	switch(err)
	{
//common error
	    case ONVIF_ERR_INVALID_SOAP_XML:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidArgVal", "Invalid soap message", 400, "Bad Request", NULL);
            break;
            
//DevMng error
        case ONVIF_ERR_DEVMNG_NO_SUCH_SERVICE:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_ACTIONNOTSUPPORTED, "ter:NoSuchService", "No Such Service.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_SERVER_RUN_ERROR:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_SERVERRUNERROR, "ter:ServerError", "Device service error.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_INVALID_HOSTNAME:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidHostname", "Invalid Hostname.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_INVALID_DNS_MANUAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidArgVal", "No manual DNS.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_INVALID_IPV4_ADDR:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidIPv4Address", "Invalid IPv4 address.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DENMNG_UNSUPPORTED_IPV6:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_ACTIONNOTSUPPORTED, "ter:ActionNotSupported", "IPv6 unsupported.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_INVALID_IPV6_ADDR:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidIPv6Address", "Invalid IPv6 address.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_INVALID_NTP_MANUAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidArgVal", "No manual NTP.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_INVALID_NETWORK_TOKEN:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidNetworkInterface", "Invalid network interface token.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_NETWORK_PROTOCOL_NOT_SUPPORT:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:ServiceNotSupported", "Network protocol not supported.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_INVALID_DATETIMETYPE:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidArgVal", "Invalid dateTime type.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_INVALID_DATETIME:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidDateTime", "Invalid date or time.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_INVALID_TIMEZONE:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidTimeZone", "Invalid timezone.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_USERNAME_CLASH:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:UsernameClash", "username already exists", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_USER_MUM_EXCEED:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:TooManyUsers", "Maximum number of supported users exceeded.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_USERNAME_MISSING:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:UsernameMissing", "Username not recognized.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_USERNAME_FIXED:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:FixedUser", "Username may not be deleted.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_USERNAME_TOO_SHORT:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:UsernameTooShort", "The username is too short.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_PASSWORD_TOO_SHORT:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:Password", "Too weak password.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_USERLEVEL_NOT_ALLOWED:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:AnonymousNotAllowed", "User level anonymous is not allowed.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_USERNAME_TOO_LONG:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:UsernameTooLong", "The username is too long.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_PASSWORD_TOO_LONG:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:PasswordTooLong", "The password is too long.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_EMPTY_SCOPE:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:EmptyScope", "Scope list is empty.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_TOO_MANY_SCOPES:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_ACTION, "ter:TooManyScopes", "The requested scope list exceeds the supported number of scopes.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_REMOVE_FIXED_SCOPE:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:FixedScope", "Trying to remove fixed scope parameter, command rejected.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_DEVMNG_NO_SUCH_SCOPE:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoScope", "Trying to remove scope which does not exist.", 400, "Bad Request", NULL);
            break;
//DeviceIO error
        case ONVIF_ERR_DEVICEIO_INVALID_RELAY_TOKEN:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:RelayToken", "Invalid Relay Token.", 400, "Bad Request", NULL);
//Event error
        case ONVIF_ERR_EVENT_SUBSCRIBE_OVERFLOW:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:overflow", "event manager overflow.", 500, "Bad Request", header);
            break;
        case ONVIF_ERR_EVENT_SUBSCRIPTION_NOTEXIST:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:not exist", "event manager not exist.", 500, "Bad Request", header);
            break;
        case ONVIF_ERR_EVENT_INVALID_TOPICEXPRESSION:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_INVALIDARGVAL, "ter:InvalidArgVal", "Invaild TopicExpression.", 500, "Bad Request", header);
            break;
        case ONVIF_ERR_EVENT_INVALID_SUBSCRIPTION_ADDR:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_INVALIDARGVAL, "ter:InvalidArgVal", "Invalid subscription address.", 500, "Bad Request", header);
            break;
        case ONVIF_ERR_EVENT_SERVER_RUN_ERROR:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_SERVERRUNERROR, "ter:ServerError", "Event service error.", 500, "Bad Request", header);
            break;
//Media error      
        case ONVIF_ERR_MEDIA_INVAILD_ARG:
    		ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidArgVal", "Invalid Name or token.", 400, "Bad Request", NULL);
    		break;
        case ONVIF_ERR_MEDIA_PROFILE_EXIST:
    		ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:ProfileExists", "Profile exists.", 400, "Bad Request", NULL);
    		break;
        case ONVIF_ERR_MEDIA_MAX_NVT_PROFILES:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:MaxNVTProfiles", "The maximum mumber of Profiles reached.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_MEDIA_NO_SUCH_PROFILE_EXIST:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoProfile", "Such profile does not exist.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_MEDIA_NO_SUCH_CONFIG_EXIST:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoConfig", "No such configuration exists.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_MEDIA_CONFIGURATION_CONFLICT:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:ConfigurationConflict", "Configuration conflicts with other.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_MEDIA_AUDIO_NOT_SUPPORTED:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_ACTIONNOTSUPPORTED, "ter:AudioNotSupported", "Audio is not supported.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_MEDIA_SERVER_RUN_ERROR:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_SERVERRUNERROR, "ter:ServerError", "Media service error.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_MEDIA_DELETE_FIXED_PROFILE:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_ACTION, "ter:DeletionOfFixedProfile", "Can not delete fixed profile.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_MEDIA_NO_VIDEO_SOURCE:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoVideoSource", "No video source", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_MEDIA_NO_VIDEO_ENCODER:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_ACTIONNOTSUPPORTED, "ter:ActionNotSupported", "No video encoder.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_MEDIA_NO_PTZ_SUPPORTED:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_ACTIONNOTSUPPORTED, "ter:PTZNotSupport", "PTZ is not supported.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_MEDIA_INCOMPLETE_CONFIG:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:IncompleteConfiguration", "No source configuration or encoder configuration.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_MEDIA_CONFIG_CANNOT_MODIFY:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:ConfigModify", "The configuration parameters are not possible to set.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_MEDIA_INVALID_STREAMSETUP:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidStreamSetup", "StreamType or Transport part is not supported.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_MEDIA_STREAM_CONFLICT:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:StreamConflict", "Conflict with other streams.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_MEDIA_INVALID_MULTICAST_SETTINGS:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidMulticastSettings", "Not all configurations are configured for multicast.", 400, "Bad Request", NULL);
            break;
//PTZ error     
        case ONVIF_ERR_PTZ_SENDER_XML_INVALID:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Invalid PTZ XML", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_PTZ_NOT_SUPPORTED:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_ACTIONNOTSUPPORTED, "ter:PTZNotSupported", "Device do not support PTZ.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_PTZ_NOT_SUCH_NODE:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoEntity", "No such PTZNode on the device.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_PTZ_NOT_SUCH_PROFILE_EXIST:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoProfile", "Invalid Profile Token", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_PTZ_NOT_REFERRNCE_PTZCONFIG:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoPTZProfile", "Requested profile does not reference a PTZ configuration.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_PTZ_NOT_UNSUPPORTED_SPACE:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SpaceNotSupported", "Space is not supported by the PTZ node.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_PTZ_INVALID_TRANSLATION:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidTranslation", "Invalid relative translation", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_PTZ_INVALID_SPEED:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidSpeed", "Invalid relative speed", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_PTZ_SERVER_RUN_ERROR:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:ServerError", "PTZ server error.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_PTZ_NO_SUCH_CONFIG:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoConfig", "The requested configuration does not exist.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_PTZ_CAN_NOT_MODIFY:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:ConfigModify", "The configuration parameters are not possible to set.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_PTZ_CONFIG_CONFLICT:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:ConfigurationConflict", "The new setting conflict with other uses of the configuration.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_PTZ_INVALID_POSITION:
			ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidPosition", "The requested position is out of bounds.", 400, "Bad Request", NULL);
			break;
		case ONVIF_ERR_PTZ_TOO_MANY_PRESETS:
			ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:TooManyPresets", "Maximum number of Presets reached.", 400, "Bad Request", NULL);
			break;
		case ONVIF_ERR_PTZ_PRESET_EXIST:
			ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:PresetExist", "The requested name already exist for another preset.", 400, "Bad Request", NULL);
			break;
		case ONVIF_ERR_PTZ_INVALID_PRESET_NAME:
			ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidPresetName", "The PresetName is either too long or contains invalid characters.", 400, "Bad Request", NULL);
			break;
		case ONVIF_ERR_PTZ_IS_MOVING:
			ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:MovingPTZ", "Preset cannot be set while PTZ unit is moving.", 400, "Bad Request", NULL);
			break;
		case ONVIF_ERR_PTZ_NO_SUCH_PRESET_TOKEN:
			ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoToken", "The requested preset token does not exist.", 400, "Bad Request", NULL);
			break;
			
//not had done
/*******************************************************************************/         
		case ONVIF_ERR_MISSINGATTR:
    		ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_MISSINGATTR, NULL, "Missing Attribute", 400, "Bad Request", NULL);
    		break;
    	case ONVIF_ERR_FIXED_SCOPE:
    		ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:FixedScope", "Trying to Remove fixed scope parameter, command rejected.", 400, "Bad Request", NULL);
    		break;
    	case ONVIF_ERR_DEVMNG_SCOPE_OVERWRITE:
    		ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:ScopeOverwrite", "Scope Overwrite", 400, "Bad Request", NULL);
    		break;
        case ONVIF_ERR_SERVICE_NOT_SUPPORT:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:ServiceNotSupported", "Service Not Supported", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_USERLEVEL_NOTSET:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_OPERATIONPROHIBITED, "ter:AnonymousNotAllowed", "user level not set", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_PROFILE_EXIST:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:ProfileExists", "Profile Exist", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_NO_PTZCFG:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoConfig", "No PTZ Config", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_INVALID_STREAM:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:InvalidStreamSetup", "Invalid Stream Setup", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_NO_VIDEOSOURCE:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoVideoSource", "No Video Source", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_NO_VIDEOENCODER:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoVideoEncoder", "No Video Encoder", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_NO_AUDIO_SUPPORT:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_ACTIONNOTSUPPORTED, "ter:AudioOutputNotSupported", "Audio Output Not Support", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_NO_AUDIOSOURCE:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoConfig", "No Audio Source Config", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_NO_AUDIOENCODER:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoConfig", "No Audio Encoder Config", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_COM_CONNECT:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_ACTION, "ter:ActionNotSupported", "Communication failed", 400, "Bad Request", NULL);
            break;
//Imaging error
        case ONVIF_ERR_IMAGE_INVALID_VIDEO_SOURCE_TOKEN:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoSource", "Invalid Video Source Token", 400, "Bad Request", NULL);
        case ONVIF_ERR_IMAGE_SENDER_XML_INVALID:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Invalid Image XML", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_SERVER_RUN_ERROR:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_SERVERRUNERROR, "ter:ServerRunError", "Server error.", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_BLC_MODE_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: backlightCompensation mode", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_BLC_LEVEL_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: backlightCompensation level", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_BRIGHTNESS_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: brightness", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_COLORSATURETION_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: colorsaturation", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_CONTRAST_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: contrast", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_EXPOSURE_MODE_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: exposure mode", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_EXPOSURE_MIN_TIME_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: exposure minTime", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_EXPOSURE_MAX_TIME_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: exposure maxTime", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_EXPOSURE_TIME_MIN_GREATER_MAX_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: exposure minTime greater than maxTime", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_EXPOSURE_MIN_GAIN_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: exposure minGain", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_EXPOSURE_MAX_GAIN_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: exposure maxGain", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_EXPOSURE_GAIN_MIN_GREATER_MAX_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: exposure minGain greater than maxGain", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_EXPOSURE_TIME_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: exposure time", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_EXPOSURE_GAIN_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: exposure gain", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_FOCUS_MODE_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: focus mode", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_FOCUS_DEFAULT_SPEED_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: focus defaultSpeed", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_FOCUS_NEAR_LIMIT_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: focus nearLimit", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_FOCUS_FAR_LIMIT_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: focus farLimit", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_FOCUS_LIMIT_NEAR_GREATER_FAR_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: focus nearLimit greater than farLimit", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_IRCUTFILTER_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: ircutfilter", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_SHARPNESS_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: sharpness", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_WIDEDYNAMICRANGE_MDOE_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: wideDynamicRange mode", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_WIDEDYNAMICRANGE_LEVEL_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: wideDynamicRange level", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_WHITEBALANCE_MODE_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: whiteBalance mode", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_WHITEBALANCE_CR_GAIN_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: whiteBalance Cr gain", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_WHITEBALANCE_CB_GAIN_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: whiteBalance Cb gain", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IAMGE_FORCE_PERSISTENCE_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: force persistence", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_FOCUS_MOVE_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: Move", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_FOCUS_MOVE_ABSOLUTE_POSITION_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: absolute position", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_FOCUS_MOVE_ABSOLUTE_SPEED_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: absolute speed", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_FOCUS_MOVE_RELATIVE_DISTANCE_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: relative distance", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_FOCUS_MOVE_RELATIVE_SPEED_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: relative speed", 400, "Bad Request", NULL);
            break;
        case ONVIF_ERR_IMAGE_FOCUS_MOVE_CONTINUOUS_SPEED_INVALID_ARGVAL:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:SettingsInvalid", "Error: continuos speed", 400, "Bad Request", NULL);
            break;
//Analytics error
        case ONVIF_ERR_ANALYTICS_INVALID_TOKEN:
            ret = soap_proc_err_rly(p_cln, rx_msg, ERR_SENDER, ERR_INVALIDARGVAL, "ter:NoConfig", "No such config", 400, "Bad Request", NULL);
            break;
        default:
    		ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_ACTIONNOTSUPPORTED, NULL, "Action Not Support", 400, "Bad Request", NULL);
    		break;
	}

	return ret;
}

static GK_S32 soap_proc_build_send_rly(HTTPCLN *p_cln, HTTPMSG *rx_msg, soap_build_xml build_xml, const void *pfn_body, const void *pfn_header)
{
	GK_S32 ret = -1, mlen = 1024*16, xlen;

	GK_CHAR *p_xml = (GK_CHAR *)malloc(mlen);
	if (NULL == p_xml)
	{
		return -1;
	}

	xlen = build_xml(p_xml, mlen, pfn_body, pfn_header);
    if (xlen < 0 || xlen >= mlen)
	{
		ret = soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_ACTIONNOTSUPPORTED, NULL, "Action Not Support", 400, "Bad Request", NULL);
	}
	else
	{
		ret = soap_proc_http_rly(p_cln, rx_msg, p_xml, xlen);
	}

	free(p_xml);

	return ret;
}
/*************************************************************************************************************/

void soap_process(HTTPCLN *p_cln, HTTPMSG *rx_msg)
{
	GK_BOOL isAuth = GK_FALSE;
	
	GK_CHAR *p_xml = http_parse_get_cttdata(rx_msg);
	if(NULL == p_xml)
	{
		ONVIF_ERR("Fail to get xml buffer.");
		return;
	}
	//ONVIF_ERR("\n%s\nlen: %d", p_xml, strlen(p_xml));
	XMLN *root = xml_stream_parse(p_xml, strlen(p_xml));
	if(NULL == root || NULL == root->name)
	{
		ONVIF_ERR("Fail to parse xml buffer. len:%d, xml:\n%s\n", strlen(p_xml), p_xml);
		return;
	}

	if(soap_match_tag(root->name, "Envelope") != 0)
	{
		ONVIF_ERR("Fail to find element of Envelope.  len:%d, xml:\n", strlen(p_xml));
		xml_node_del(root);
		return;
	}
	
	XMLN *p_header = soap_get_child_node(root, "Header");
	if(p_header)
	{
		XMLN * p_Security = soap_get_child_node(p_header, "Security");
		if (p_Security)
		{
			ONVIF_INFO("security start\n");
			isAuth = soap_proc_auth_process(p_Security);
			ONVIF_INFO("security ended\n");
		}
	}
	if(g_GkIpc_OnvifInf.discov.Is_security && !isAuth)
	{
		ONVIF_ERR("Fail to pass the security.  len:%d, xml:\n%s\n", strlen(p_xml), p_xml);
		soap_proc_security_rly(p_cln, rx_msg);
		xml_node_del(root);		
		return;
	}

	XMLN *p_body = soap_get_child_node(root, "Body");
	if(NULL == p_body)
	{
		ONVIF_ERR("Fail to find element of Body.  len:%d, xml:\n%s\n", strlen(p_xml), p_xml);
		xml_node_del(root);
		return;
	}
	
	ONVIF_INFO("soap function name[%s]\n", p_body->f_child->name);
	
    if(soap_match_tag(p_body->f_child->name, "GetServiceCapabilities") == 0 && strstr(rx_msg->first_line.value_string, "device_service") != NULL)
	{
		soap_Device_GetServiceCapabilities(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetServiceCapabilities") == 0 && strstr(rx_msg->first_line.value_string, "media_service") != NULL)
	{
		soap_Media_GetServiceCapabilities(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetServiceCapabilities") == 0 && strstr(rx_msg->first_line.value_string, "imaging_service") != NULL)
	{
		soap_Imaging_GetServiceCapabilities(p_cln, rx_msg, p_body);
	}
    else if(soap_match_tag(p_body->f_child->name, "GetServiceCapabilities") == 0 && strstr(rx_msg->first_line.value_string, "ptz_service") != NULL)
	{
		soap_PTZ_GetServiceCapabilities(p_cln, rx_msg, p_body);
	}
    else if(soap_match_tag(p_body->f_child->name, "GetServiceCapabilities") == 0 && strstr(rx_msg->first_line.value_string, "event_service") != NULL)
	{
		soap_Event_GetServiceCapabilities(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetServiceCapabilities") == 0 && strstr(rx_msg->first_line.value_string, "analytics_service") != NULL)
	{
		soap_VideoAnalytics_GetServiceCapabilities(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetScopes") == 0)
	{
		soap_Device_GetScopes(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "AddScopes") == 0)
	{
		soap_Device_AddScopes(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "SetScopes") == 0)
	{
		soap_Device_SetScopes(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "RemoveScopes") == 0)
	{
		soap_Device_RemoveScopes(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetDiscoveryMode") == 0)
	{
		soap_Device_GetDiscoveryMode(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "SetDiscoveryMode") == 0)
	{
		soap_Device_SetDiscoveryMode(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "SystemReboot") == 0)
	{
		soap_Device_SystemReboot(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetCapabilities") == 0)
	{
        soap_Device_GetCapabilities(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetWsdlUrl") == 0)
	{
		soap_Device_GetWsdlUrl(p_cln, rx_msg, p_body);
	}
    else if(soap_match_tag(p_body->f_child->name, "GetServices") == 0)
	{
		soap_Device_GetServices(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetHostname") == 0)
	{
		soap_Device_GetHostname(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "SetHostname") == 0)
	{
		soap_Device_SetHostname(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetNTP") == 0)
	{
		soap_Device_GetNTP(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "SetNTP") == 0)
	{
		soap_Device_SetNTP(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetDNS") == 0)
	{
		soap_Device_GetDNS(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "SetDNS") == 0)
	{
		soap_Device_SetDNS(p_cln, rx_msg, p_body);
	}
    else if(soap_match_tag(p_body->f_child->name, "GetNetworkInterfaces") == 0)
	{
		soap_Device_GetNetworkInterfaces(p_cln, rx_msg, p_body);
	}
    else if(soap_match_tag(p_body->f_child->name, "SetNetworkInterfaces") == 0)
	{
		soap_Device_SetNetworkInterfaces(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetNetworkProtocols") == 0)
	{
		soap_Device_GetNetworkProtocols(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "SetNetworkProtocols") == 0)
	{
		soap_Device_SetNetworkProtocols(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetNetworkDefaultGateway") == 0)
	{
		soap_Device_GetNetworkDefaultGateway(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "SetNetworkDefaultGateway") == 0)
	{
		soap_Device_SetNetworkDefaultGateway(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetSystemDateAndTime") == 0)
	{
		soap_Device_GetSystemDateAndTime(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "SetSystemDateAndTime") == 0)
	{
		soap_Device_SetSystemDateAndTime(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "SetSystemFactoryDefault") == 0)
	{
		soap_Device_SetSystemFactoryDefault(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetDeviceInformation") == 0)
	{
		soap_Device_GetDeviceInformation(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetUsers") == 0)
	{
		soap_Device_GetUsers(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "CreateUsers") == 0)
	{
		soap_Device_CreateUsers(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "DeleteUsers") == 0)
	{
		soap_Device_DeleteUsers(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "SetUser") == 0)
	{
		soap_Device_SetUser(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "CreateProfile") == 0)
	{
		soap_Media_CreateProfile(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetProfile") == 0)
	{
		soap_Media_GetProfile(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetProfiles") == 0)
	{
		soap_Media_GetProfiles(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "DeleteProfile") == 0)
	{
		soap_Media_DeleteProfile(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetStreamUri") == 0)
	{
		soap_Media_GetStreamUri(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetSnapshotUri") == 0)
	{
		soap_Media_GetSnapshotUri(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetVideoSourceConfiguration") == 0)
	{
		soap_Media_GetVideoSourceConfiguration(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetVideoSourceConfigurations") == 0)
	{
		soap_Media_GetVideoSourceConfigurations(p_cln, rx_msg, p_body);
	}
    else if(soap_match_tag(p_body->f_child->name, "GetCompatibleVideoSourceConfigurations") == 0)
	{
        soap_Media_GetCompatibleVideoSourceConfigurations(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetVideoSourceConfigurationOptions") == 0)
	{
		soap_Media_GetVideoSourceConfigurationOptions(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "AddVideoSourceConfiguration") == 0)
	{
        soap_Media_AddVideoSourceConfiguration(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "RemoveVideoSourceConfiguration") == 0)
	{
        soap_Media_RemoveVideoSourceConfiguration(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "SetVideoSourceConfiguration") == 0)
	{
        soap_Media_SetVideoSourceConfiguration(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetVideoEncoderConfiguration") == 0)
	{
        soap_Media_GetVideoEncoderConfiguration(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetVideoEncoderConfigurations") == 0)
	{
        soap_Media_GetVideoEncoderConfigurations(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetCompatibleVideoEncoderConfigurations") == 0)
	{
        soap_Media_GetCompatibleVideoEncoderConfigurations(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetVideoEncoderConfigurationOptions") == 0)
	{
        soap_Media_GetVideoEncoderConfigurationOptions(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "AddVideoEncoderConfiguration") == 0)
	{
        soap_Media_AddVideoEncoderConfiguration(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "RemoveVideoEncoderConfiguration") == 0)
	{
        soap_Media_RemoveVideoEncoderConfiguration(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "SetVideoEncoderConfiguration") == 0)
	{
        soap_Media_SetVideoEncoderConfiguration(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetGuaranteedNumberOfVideoEncoderInstances") == 0)
	{
        soap_Media_GetGuaranteedNumberOfVideoEncoderInstances(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "AddPTZConfiguration") == 0)
	{
        soap_Media_AddPTZConfiguration(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "RemovePTZConfiguration") == 0)
	{
        soap_Media_RemovePTZConfiguration(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetAudioSourceConfiguration") == 0)
	{
		soap_Media_GetAudioSourceConfiguration(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetAudioEncoderConfiguration") == 0)
	{
		soap_Media_GetAudioEncoderConfiguration(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetAudioSourceConfigurations") == 0)
	{
		soap_Media_GetAudioSourceConfigurations(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetAudioEncoderConfigurations") == 0)
	{
		soap_Media_GetAudioEncoderConfigurations(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetCompatibleAudioSourceConfigurations") == 0)
	{
		soap_Media_GetCompatibleAudioSourceConfigurations(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetCompatibleAudioEncoderConfigurations") == 0)
	{
		soap_Media_GetCompatibleAudioEncoderConfigurations(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetAudioSourceConfigurationOptions") == 0)
	{
		soap_Media_GetAudioSourceConfigurationOptions(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetAudioEncoderConfigurationOptions") == 0)
	{
		soap_Media_GetAudioEncoderConfigurationOptions(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "SetAudioSourceConfiguration") == 0)
	{
		soap_Media_SetAudioSourceConfiguration(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "SetAudioEncoderConfiguration") == 0)
	{
		soap_Media_SetAudioEncoderConfiguration(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "AddAudioSourceConfiguration") == 0)
	{
		soap_Media_AddAudioSourceConfiguration(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "AddAudioEncoderConfiguration") == 0)
	{
		soap_Media_AddAudioEncoderConfiguration(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "RemoveAudioSourceConfiguration") == 0)
	{
		soap_Media_RemoveAudioSourceConfiguration(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "RemoveAudioEncoderConfiguration") == 0)
	{
		soap_Media_RemoveAudioEncoderConfiguration(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetMetadataConfiguration") == 0)
	{
		soap_Media_GetMetadataConfiguration(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetMetadataConfigurations") == 0)
	{
		soap_Media_GetMetadataConfigurations(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetCompatibleMetadataConfigurations") == 0)
	{
		soap_Media_GetCompatibleMetadataConfigurations(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetMetadataConfigurationOptions") == 0)
	{
		soap_Media_GetMetadataConfigurationOptions(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "StartMulticastStreaming") == 0)
	{
		soap_Media_StartMulticastStreaming(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "StopMulticastStreaming") == 0)
	{
		soap_Media_StopMulticastStreaming(p_cln, rx_msg, p_body);
	}
    else if(soap_match_tag(p_body->f_child->name, "GetOSDOptions") == 0)
	{
        soap_Media_GetOSDOptions(p_cln, rx_msg, p_body);
	}
    else if(soap_match_tag(p_body->f_child->name, "GetOSDs") == 0 ||
            soap_match_tag(p_body->f_child->name, "GetOSD") == 0)
	{
        soap_Media_GetOSDs(p_cln, rx_msg, p_body);
	}
    else if(soap_match_tag(p_body->f_child->name, "SetOSD") == 0 || 
            soap_match_tag(p_body->f_child->name, "CreateOSD") == 0)
	{
        soap_Media_SetOSD(p_cln, rx_msg, p_body);
	}
    else if(soap_match_tag(p_body->f_child->name, "DeleteOSD") == 0)
	{
        soap_Media_DeleteOSD(p_cln, rx_msg, p_body);
	}
    else if(soap_match_tag(p_body->f_child->name, "GetPrivacyMaskOptions") == 0)
	{
        soap_Media_GetPrivacyMaskOptions(p_cln, rx_msg, p_body);
	}
    else if(soap_match_tag(p_body->f_child->name, "GetPrivacyMasks") == 0)
	{
        soap_Media_GetPrivacyMasks(p_cln, rx_msg, p_body);
	}
    else if(soap_match_tag(p_body->f_child->name, "DeletePrivacyMask") == 0)
	{
        soap_Media_DeletePrivacyMask(p_cln, rx_msg, p_body);
	}
    else if(soap_match_tag(p_body->f_child->name, "CreatePrivacyMask") == 0)
	{
        soap_Media_CreatePrivacyMask(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "Subscribe") == 0)
	{
        soap_Event_Subscribe(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "Unsubscribe") == 0)
	{
        soap_Event_Unsubscribe(p_cln, rx_msg, p_header);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetEventProperties") == 0)
	{
        soap_Event_GetEventProperties(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "Renew") == 0)
	{
        soap_Event_Renew(p_cln, rx_msg, p_body, p_header);
	}
#if 0	
	else if(soap_match_tag(p_body->f_child->name, "SetSynchronizationPoint") == 0)
	{
        soap_Event_SetSynchronizationPoint(p_cln, rx_msg, p_header);
	}
	else if(soap_match_tag(p_body->f_child->name, "CreatePullPointSubscription") == 0)
	{
        soap_Event_CreatePullPointSubscription(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "PullMessages") == 0)
	{
        soap_Event_PullMessages(p_cln, rx_msg, p_body, p_header);
	}
#endif	
	else if(soap_match_tag(p_body->f_child->name, "GetVideoSources") == 0)
	{
        soap_DeviceIO_GetVideoSource(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetAudioSources") == 0)
	{
        soap_DeviceIO_GetAudioSource(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetRelayOutputs") == 0)
	{
        soap_DeviceIO_GetRelayOutputs(p_cln, rx_msg, p_body);
	}
    else if(soap_match_tag(p_body->f_child->name, "SetRelayOutputSettings") == 0)
	{
        soap_DeviceIO_SetRelayOutputSettings(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "SetRelayOutputState") == 0)
	{
        soap_DeviceIO_SetRelayOutputState(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetImagingSettings") == 0)
	{
        soap_Imaging_GetImagingSettings(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetOptions") == 0)
	{
        soap_Imaging_GetOptions(p_cln, rx_msg, p_body);
	}
    else if(soap_match_tag(p_body->f_child->name, "SetImagingSettings") == 0)
	{
        soap_Imaging_SetImagingSettings(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetStatus") == 0 && strstr(rx_msg->first_line.value_string, "imaging_service") != NULL)
	{
        soap_Imaging_GetStatus(p_cln, rx_msg, p_body);
	}
#if 1	
	else if(soap_match_tag(p_body->f_child->name, "GetMoveOptions") == 0)
	{
        soap_Imaging_GetMoveOptions(p_cln, rx_msg, p_body);
	}		
	else if(soap_match_tag(p_body->f_child->name, "Move") == 0)
	{
        soap_Imaging_Move(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "Stop") == 0 && strstr(rx_msg->first_line.value_string, "imaging_service") != NULL)
	{
        soap_Imaging_Stop(p_cln, rx_msg, p_body);
	}
#endif
	else if(soap_match_tag(p_body->f_child->name, "GetNodes") == 0)
	{
        soap_PTZ_GetNodes(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetNode") == 0)
	{
        soap_PTZ_GetNode(p_cln, rx_msg, p_body);
	}
    else if(soap_match_tag(p_body->f_child->name, "GetConfigurations") == 0)
    {
        soap_PTZ_GetConfigurations(p_cln, rx_msg, p_body);
    }
    else if(soap_match_tag(p_body->f_child->name, "GetConfiguration") == 0)
    {
        soap_PTZ_GetConfiguration(p_cln, rx_msg, p_body);
    }
    else if(soap_match_tag(p_body->f_child->name, "GetConfigurationOptions") == 0)
    {
        soap_PTZ_GetConfigurationOptions(p_cln, rx_msg, p_body);
    }
    else if(soap_match_tag(p_body->f_child->name, "SetConfiguration") == 0)
    {
        soap_PTZ_SetConfiguration(p_cln, rx_msg, p_body);
    }
    else if(soap_match_tag(p_body->f_child->name, "AbsoluteMove") == 0)
    {
        soap_PTZ_AbsoluteMove(p_cln, rx_msg, p_body);
    }
    else if(soap_match_tag(p_body->f_child->name, "RelativeMove") == 0)
    {
        soap_PTZ_RelativeMove(p_cln, rx_msg, p_body);
    }
    else if(soap_match_tag(p_body->f_child->name, "ContinuousMove") == 0)
    {
        soap_PTZ_ContinuousMove(p_cln, rx_msg, p_body);
    }
	else if(soap_match_tag(p_body->f_child->name, "GetStatus") == 0 && strstr(rx_msg->first_line.value_string, "ptz_service") != NULL)
	{
        soap_PTZ_GetStatus(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "Stop") == 0 && strstr(rx_msg->first_line.value_string, "ptz_service") != NULL)
	{
        soap_PTZ_Stop(p_cln, rx_msg, p_body);
	}	
    else if(soap_match_tag(p_body->f_child->name, "GetPresets") == 0)
    {
        soap_PTZ_GetPresets(p_cln, rx_msg, p_body);
    }
    else if(soap_match_tag(p_body->f_child->name, "SetPreset") == 0)
    {
        soap_PTZ_SetPreset(p_cln, rx_msg, p_body);
    }
    else if(soap_match_tag(p_body->f_child->name, "GotoPreset") == 0)
    {
        soap_PTZ_GotoPreset(p_cln, rx_msg, p_body);
    }
    else if(soap_match_tag(p_body->f_child->name, "RemovePreset") == 0)
    {
        soap_PTZ_RemovePreset(p_cln, rx_msg, p_body);
    } 
#if 0
	else if(soap_match_tag(p_body->f_child->name, "GetSupportedAnalyticsModules") == 0)
	{
        soap_VideoAnalytics_GetSupportedAnalyticsModules(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetAnalyticsModules") == 0)
	{
        soap_VideoAnalytics_GetAnalyticsModules(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetSupportedRules") == 0)
	{
        soap_VideoAnalytics_GetSupportedRules(p_cln, rx_msg, p_body);
	}
	else if(soap_match_tag(p_body->f_child->name, "GetRules") == 0)
	{
        soap_VideoAnalytics_GetRules(p_cln, rx_msg, p_body);
	}
#endif
	else
	{
        soap_proc_err_rly(p_cln, rx_msg, ERR_RECEIVER, ERR_ACTIONNOTSUPPORTED, NULL, "Action Not Implemented", 400, "Bad Request", NULL);
	}
	
    xml_node_del(root);

}


