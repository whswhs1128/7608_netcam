/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif-no-gsoap/src/soap_parser.c
**
** \brief       soap parser
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include "soap_parse.h"

const char *soap_get_tag_child_name(const char *tag)
{
    if(tag == NULL)
		return NULL;
	const char *child_name = strchr(tag, ':');
	
	if (child_name)
        child_name += 1;
	else
        child_name = tag;
    return child_name;
}

GK_S32 soap_match_tag(const char *str1, const char *str2)
{
	if (strcasecmp(str1, str2) == 0)
		return 0;

	const char *ptr1 = strchr(str1, ':');
	const char *ptr2 = strchr(str2, ':');
	
	if (ptr1 && ptr2)
		return strcasecmp(ptr1+1, ptr2+1);
	else if (ptr1)
		return strcasecmp(ptr1+1, str2);
	else if (ptr2)
		return strcasecmp(str1, ptr2+1);
	else
		return -1;
		
}

XMLN *soap_get_child_node(XMLN *parent, const char *nodeName)
{
	if(parent == NULL || nodeName == NULL)
		return NULL;

	XMLN *p_node = parent->f_child;
	while(p_node != NULL)
	{
		if(soap_match_tag(p_node->name, nodeName) == 0)
			return p_node;

		p_node = p_node->next;
	}

	return NULL;
}

XMLN *soap_get_next_node(XMLN *curNode, const char *nodeName)
{
	if(curNode == NULL || nodeName == NULL)
		return NULL;

	XMLN *p_node = curNode->next;
	while(p_node != NULL)
	{
		if(soap_match_tag(p_node->name, nodeName) == 0)
			return p_node;

		p_node = p_node->next;
	}

	return NULL;
}

/********************************* device ***************************************/
ONVIF_RET soap_parse_request_GetCapabilities(XMLN *p_node, GONVIF_DEVMNG_GetCapabilities_S *pstDM_GetCapabilities)
{

    XMLN * p_Category = soap_get_child_node(p_node, "Category");
    if(p_Category && p_Category->data)
    {
        if(NULL == p_Category->data)
        {
            return ONVIF_ERR_INVALID_SOAP_XML;
        }

        if(strcasecmp(p_Category->data, "Media") == 0)
        {
            pstDM_GetCapabilities->enCategory[0] = CapabilityCategory_Media;
        }
        else if(strcasecmp(p_Category->data, "Device") == 0)
        {
            pstDM_GetCapabilities->enCategory[0] = CapabilityCategory_Device;
        }
        else if(strcasecmp(p_Category->data, "Analytics") == 0)
        {
            pstDM_GetCapabilities->enCategory[0] = CapabilityCategory_Analytics;
        }
        else if(strcasecmp(p_Category->data, "Events") == 0)
        {
            pstDM_GetCapabilities->enCategory[0] = CapabilityCategory_Events;
        }
        else if(strcasecmp(p_Category->data, "Imaging") == 0)
        {
            pstDM_GetCapabilities->enCategory[0] = CapabilityCategory_Imaging;
        }
        else if(strcasecmp(p_Category->data, "PTZ") == 0)
        {
            pstDM_GetCapabilities->enCategory[0] = CapabilityCategory_PTZ;
        }
        else if(strcasecmp(p_Category->data, "All") == 0)
        {
            pstDM_GetCapabilities->enCategory[0] = CapabilityCategory_All;
        }
        else
        {
            return ONVIF_ERR_INVALID_SOAP_XML;
        }
    }


    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_SetHostname(XMLN *p_node, GONVIF_DEVMNG_SetHostname_S *pstDM_SetHostname)
{
    XMLN * p_Name = soap_get_child_node(p_node, "Name");
	if (p_Name && p_Name->data)
	{
	    strncpy(pstDM_SetHostname->aszName, p_Name->data, MAX_NAME_LENGTH-1);
    }
    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_SetDNS(XMLN *p_node, GONVIF_DEVMNG_SetDNS_S *pstDM_SetDNS)
{

	XMLN *p_FromDHCP = soap_get_child_node(p_node, "FromDHCP");
	if (p_FromDHCP && p_FromDHCP->data)
	{
		if (strcasecmp(p_FromDHCP->data, "true") == 0)
		{
			pstDM_SetDNS->enFromDHCP = Boolean_TRUE;
		}
        else
        {
			pstDM_SetDNS->enFromDHCP = Boolean_FALSE;
        }
	}

	GK_S32 i = 0;
	XMLN *p_SearchDomain = soap_get_child_node(p_node, "SearchDomain");
	while(p_SearchDomain && soap_match_tag(p_SearchDomain->name, "SearchDomain") == 0)
	{
		if(p_SearchDomain->data && i < MAX_SEARCHDOMAIN_NUM)
		{
			strncpy(pstDM_SetDNS->aszSearchDomain[i], p_SearchDomain->data, MAX_SEARCHDOMAIN_LEN-1);
			++i;
		}

		p_SearchDomain = p_SearchDomain->next;
	}
    pstDM_SetDNS->sizeSearchDomain = i;

	i = 0;
	XMLN *p_DNSManual = soap_get_child_node(p_node, "DNSManual");
	while(p_DNSManual && soap_match_tag(p_DNSManual->name, "DNSManual") == 0)
	{
	    if(i >= MAX_DNSMANUAL_NUM)
        {
            break;
        }
		XMLN * p_Type = soap_get_child_node(p_DNSManual, "Type");
		if (p_Type && p_Type->data)
		{
			if (strcasecmp(p_Type->data, "IPv4") == 0)
			{
				pstDM_SetDNS->stDNSManual[i].enType = IPType_IPv4;
                XMLN *p_IPv4Address = soap_get_child_node(p_DNSManual, "IPv4Address");
                if(p_IPv4Address && p_IPv4Address->data)
                {
                    strncpy(pstDM_SetDNS->stDNSManual[i].aszIPv4Address, p_IPv4Address->data, IPV4_STR_LENGTH-1);
                    ++i;
                }
			}
            else
            {
				pstDM_SetDNS->stDNSManual[i].enType = IPType_IPv6;
                XMLN *p_IPv6Address = soap_get_child_node(p_DNSManual, "IPv6Address");
                if(p_IPv6Address && p_IPv6Address->data)
                {
                    strncpy(pstDM_SetDNS->stDNSManual[i].aszIPv6Address, p_IPv6Address->data, IPV6_STR_LENGTH-1);
                    ++i;
                }
            }
		}
		p_DNSManual = p_DNSManual->next;
	}
    pstDM_SetDNS->sizeDNSManual = i;
    
	return ONVIF_OK;
	
}

ONVIF_RET soap_parse_request_SetNTP(XMLN *p_node, GONVIF_DEVMNG_SetNTP_S *pstDM_SetNTP)
{
	XMLN *p_FromDHCP = soap_get_child_node(p_node, "FromDHCP");
	if (p_FromDHCP && p_FromDHCP->data)
	{
		if (strcasecmp(p_FromDHCP->data, "true") == 0)
		{
			pstDM_SetNTP->enFromDHCP = Boolean_TRUE;
		}
	}

	GK_S32 i = 0;
	XMLN *p_NTPManual = soap_get_child_node(p_node, "NTPManual");
	while(p_NTPManual && soap_match_tag(p_NTPManual->name, "NTPManual") == 0)
	{
		XMLN *p_Type = soap_get_child_node(p_NTPManual, "Type");
		if (p_Type && p_Type->data)
		{
			if (strcasecmp(p_Type->data, "IPv4") == 0)
			{
                pstDM_SetNTP->stNTPManual[i].enType = NetworkHostType_IPv4;
			}
			else if (strcasecmp(p_Type->data, "IPv6") == 0)
			{
                pstDM_SetNTP->stNTPManual[i].enType = NetworkHostType_IPv6;
			}
            else if (strcasecmp(p_Type->data, "DNS") == 0)
			{
                pstDM_SetNTP->stNTPManual[i].enType = NetworkHostType_DNS;
			}
		}
		XMLN *p_IPv4Address = soap_get_child_node(p_NTPManual, "IPv4Address");
		if (p_IPv4Address && p_IPv4Address->data)
		{
			if (i < MAX_NTPMANUAL_NUM)
			{
				strncpy(pstDM_SetNTP->stNTPManual[i].aszIPv4Address, p_IPv4Address->data, IPV4_STR_LENGTH - 1);
				pstDM_SetNTP->stNTPManual[i].aszIPv4Address[IPV4_STR_LENGTH - 1] = '\0';
                ++i;
			}
		}

		XMLN *p_IPv6Address = soap_get_child_node(p_NTPManual, "IPv6Address");
		if (p_IPv6Address && p_IPv6Address->data)
		{
			if (i < MAX_NTPMANUAL_NUM)
			{
				strncpy(pstDM_SetNTP->stNTPManual[i].aszIPv6Address, p_IPv6Address->data, IPV6_STR_LENGTH - 1);
				pstDM_SetNTP->stNTPManual[i].aszIPv6Address[IPV6_STR_LENGTH - 1] = '\0';
                ++i;
			}
		}

		XMLN *p_DNSname = soap_get_child_node(p_NTPManual, "DNSname");
		if (p_DNSname && p_DNSname->data)
		{
			if (i < MAX_NTPMANUAL_NUM)
			{
				strncpy(pstDM_SetNTP->stNTPManual[i].aszDNSname, p_DNSname->data, MAX_NAME_LENGTH - 1);
				pstDM_SetNTP->stNTPManual[i].aszDNSname[MAX_NAME_LENGTH - 1] = '\0';
                ++i;
			}
		}
		p_NTPManual = p_NTPManual->next;
	}
    pstDM_SetNTP->sizeNTPManual = i;
    
	return ONVIF_OK;
}

ONVIF_RET soap_parse_request_SetNetworkInterfaces(XMLN *p_node, GONVIF_DEVMNG_SetNetworkInterfaces_S *pstDM_SetNetworkInterfaces)
{
    XMLN * p_InterfaceToken = soap_get_child_node(p_node, "InterfaceToken");
	if (p_InterfaceToken && p_InterfaceToken->data)
	{
	    strncpy(pstDM_SetNetworkInterfaces->aszInterfaceToken, p_InterfaceToken->data, MAX_TOKEN_LENGTH - 1);
	}

	XMLN * p_NetworkInterface = soap_get_child_node(p_node, "NetworkInterface");
	if (p_NetworkInterface)
	{

	    XMLN * p_Enabled = soap_get_child_node(p_NetworkInterface, "Enabled");
	    if (p_Enabled && p_Enabled->data)
	    {
	        pstDM_SetNetworkInterfaces->stNetworkInterface.enEnable = (strcasecmp(p_Enabled->data, "true") == 0 ? Boolean_TRUE : Boolean_FALSE);
	    }

	    XMLN * p_MTU = soap_get_child_node(p_NetworkInterface, "MTU");
	    if (p_MTU && p_MTU->data)
	    {
	        pstDM_SetNetworkInterfaces->stNetworkInterface.MTU = atoi(p_MTU->data);
	    }

	    XMLN * p_IPv4 = soap_get_child_node(p_NetworkInterface, "IPv4");
	    if (p_IPv4)
	    {
	        XMLN * p_Enabled = soap_get_child_node(p_IPv4, "Enabled");
	        if (p_Enabled && p_Enabled->data)
    	    {
    	        pstDM_SetNetworkInterfaces->stNetworkInterface.stIPv4.enEnabled = (strcasecmp(p_Enabled->data, "false") == 0 ? Boolean_FALSE : Boolean_TRUE);
    	    }

    	    XMLN * p_DHCP = soap_get_child_node(p_IPv4, "DHCP");
	        if (p_DHCP && p_DHCP->data)
	        {
	            pstDM_SetNetworkInterfaces->stNetworkInterface.stIPv4.enDHCP = (strcasecmp(p_DHCP->data, "true") == 0 ? Boolean_TRUE : Boolean_FALSE);
	        }

	        if (pstDM_SetNetworkInterfaces->stNetworkInterface.stIPv4.enDHCP == Boolean_FALSE)
	        {
	            XMLN * p_Manual = soap_get_child_node(p_IPv4, "Manual");
	            if (p_Manual)
	            {
	                XMLN * p_Address = soap_get_child_node(p_Manual, "Address");
	                if (p_Address && p_Address->data)
	                {
	                    strncpy(pstDM_SetNetworkInterfaces->stNetworkInterface.stIPv4.stManual[0].aszAddress, p_Address->data, MAX_ADDR_LENGTH-1);
	                }

	                XMLN * p_PrefixLength = soap_get_child_node(p_Manual, "PrefixLength");
	                if (p_PrefixLength && p_PrefixLength->data)
	                {
	                    pstDM_SetNetworkInterfaces->stNetworkInterface.stIPv4.stManual[0].prefixLength = atoi(p_PrefixLength->data);
	                }
	            }
	        }
	    }
	}

	return ONVIF_OK;
}

ONVIF_RET soap_parse_request_SetNetworkProtocols(XMLN *p_node, GONVIF_DEVMNG_SetNetworkProtocols_S *pstDM_SetNetworkProtocols)
{
    GK_S32 index = 0;
	XMLN *p_NetworkProtocols = soap_get_child_node(p_node, "NetworkProtocols");
	while (p_NetworkProtocols && strcasecmp(p_NetworkProtocols->name, "NetworkProtocols") == 0)
	{
		/*name*/
		XMLN * p_Name = soap_get_child_node(p_NetworkProtocols, "Name");
		if (p_Name && p_Name->data)
		{
            if (strcasecmp(p_Name->data, "HTTP") == 0)
            {
                pstDM_SetNetworkProtocols->stNetworkProtocols[index].enName = NetworkProtocolType_HTTP;
            }
            else if (strcasecmp(p_Name->data, "HTTPS") == 0)
            {
                pstDM_SetNetworkProtocols->stNetworkProtocols[index].enName = NetworkProtocolType_HTTPS;
            }
            else if (strcasecmp(p_Name->data, "RTSP") == 0)
            {
                pstDM_SetNetworkProtocols->stNetworkProtocols[index].enName = NetworkProtocolType_RTSP;
            }
		}

        /*enable*/
		XMLN *p_Enabled = soap_get_child_node(p_NetworkProtocols, "Enabled");
		if (p_Enabled && p_Enabled->data)
		{
			if (strcasecmp(p_Enabled->data, "true") == 0)
			{
                pstDM_SetNetworkProtocols->stNetworkProtocols[index].enEnabled = Boolean_TRUE;
			}
            else
            {
                pstDM_SetNetworkProtocols->stNetworkProtocols[index].enEnabled = Boolean_FALSE;
            }
		}

        /*port*/
		GK_S32 i = 0;
		XMLN *p_Port = soap_get_child_node(p_NetworkProtocols, "Port");
		while (p_Port && p_Port->data && strcasecmp(p_Port->name, "Port") == 0)
		{
			if (i < MAX_PORT_NUM)
			{
                pstDM_SetNetworkProtocols->stNetworkProtocols[index].port[i++] = atoi(p_Port->data);
                pstDM_SetNetworkProtocols->stNetworkProtocols[index].sizePort++;
			}
			p_Port = p_Port->next;
		}
        index++;
		p_NetworkProtocols = p_NetworkProtocols->next;
	}
	pstDM_SetNetworkProtocols->sizeNetworkProtocols = index;
	
	return ONVIF_OK;
}

ONVIF_RET soap_parse_request_SetNetworkDefaultGateway(XMLN *p_node, GONVIF_DEVMNG_SetNetworkDefaultGateway_S *pstDM_SetNetworkDefaultGateway)
{
	GK_S32 i = 0;

	XMLN *p_IPv4Address = soap_get_child_node(p_node, "IPv4Address");
	while(p_IPv4Address && p_IPv4Address->data && strcasecmp(p_IPv4Address->name, "IPv4Address") == 0)
	{
		if(i < MAX_GATE_COUNT)
		{
			strncpy(pstDM_SetNetworkDefaultGateway->aszIPv4Address[i++], p_IPv4Address->data, IPV4_STR_LENGTH - 1);
		}

		p_IPv4Address = p_IPv4Address->next;
	}
    pstDM_SetNetworkDefaultGateway->sizeIPv4Address = i;

    i = 0;
	XMLN *p_IPv6Address = soap_get_child_node(p_node, "IPv6Address");
	while(p_IPv6Address && p_IPv6Address->data && strcasecmp(p_IPv6Address->name, "IPv6Address") == 0)
	{
		if(i < MAX_GATE_COUNT)
		{
			strncpy(pstDM_SetNetworkDefaultGateway->aszIPv6Address[i++], p_IPv6Address->data, IPV6_STR_LENGTH - 1);
		}

		p_IPv6Address = p_IPv6Address->next;
	}
    pstDM_SetNetworkDefaultGateway->sizeIPv6Address = i;

	return ONVIF_OK;
}

ONVIF_RET soap_parse_request_SetSystemDateAndTime(XMLN *p_node, GONVIF_DEVMNG_SetSystemDateAndTime_S *pstDM_SetSystemDateAndTime)
{
    XMLN * p_DateTimeType = soap_get_child_node(p_node, "DateTimeType");
    if(p_DateTimeType && p_DateTimeType->data)
    {
        pstDM_SetSystemDateAndTime->enDateTimeType = (strcasecmp(p_DateTimeType->data, "NTP") ? SetDateTimeType_Manual : SetDateTimeType_NTP);
    }

    XMLN *p_DaylightSavings = soap_get_child_node(p_node, "DaylightSavings");
    if(p_DaylightSavings && p_DaylightSavings->data)
    {
        pstDM_SetSystemDateAndTime->enDaylightSavings = (strcasecmp(p_DaylightSavings->data, "true") ? GK_FALSE : GK_TRUE);
    }

    XMLN *p_TimeZone = soap_get_child_node(p_node, "TimeZone");
    if(p_TimeZone)
    {
        XMLN *p_TZ = soap_get_child_node(p_TimeZone, "TZ");
		if(p_TZ && p_TZ->data)
		{
			strncpy(pstDM_SetSystemDateAndTime->stTimeZone.aszTZ, p_TZ->data, MAX_TIME_ZONE_LENGTH-1);
		}
    }

    XMLN * p_UTCDateTime = soap_get_child_node(p_node, "UTCDateTime");
    if(p_UTCDateTime)
    {
        XMLN *p_Time = soap_get_child_node(p_UTCDateTime, "Time");
	    if(!p_Time)
	    {
	        return ONVIF_ERR_INVALID_SOAP_XML;
	    }

	    XMLN *p_Hour = soap_get_child_node(p_Time, "Hour");
	    if(!p_Hour || !p_Hour->data)
	    {
	        return ONVIF_ERR_INVALID_SOAP_XML;
	    }
	    pstDM_SetSystemDateAndTime->stUTCDateTime.stTime.hour = atoi(p_Hour->data);

	    XMLN *p_Minute = soap_get_child_node(p_Time, "Minute");
	    if(!p_Minute || !p_Minute->data)
	    {
	        return ONVIF_ERR_MISSINGATTR;
	    }
	    pstDM_SetSystemDateAndTime->stUTCDateTime.stTime.minute = atoi(p_Minute->data);

	    XMLN *p_Second = soap_get_child_node(p_Time, "Second");
	    if(!p_Second || !p_Second->data)
	    {
	        return ONVIF_ERR_INVALID_SOAP_XML;
	    }
	    pstDM_SetSystemDateAndTime->stUTCDateTime.stTime.second = atoi(p_Second->data);

	    XMLN *p_Date = soap_get_child_node(p_UTCDateTime, "Date");
	    if(!p_Date)
	    {
	        return ONVIF_ERR_INVALID_SOAP_XML;
	    }

	    XMLN *p_Year = soap_get_child_node(p_Date, "Year");
	    if(!p_Year || !p_Year->data)
	    {
	        return ONVIF_ERR_INVALID_SOAP_XML;
	    }
	    pstDM_SetSystemDateAndTime->stUTCDateTime.stDate.year = atoi(p_Year->data);

	    XMLN * p_Month = soap_get_child_node(p_Date, "Month");
	    if(!p_Month || !p_Month->data)
	    {
	        return ONVIF_ERR_INVALID_SOAP_XML;
	    }
	    pstDM_SetSystemDateAndTime->stUTCDateTime.stDate.month = atoi(p_Month->data);

	    XMLN * p_Day = soap_get_child_node(p_Date, "Day");
	    if(!p_Day || !p_Day->data)
	    {
	        return ONVIF_ERR_INVALID_SOAP_XML;
	    }
	    pstDM_SetSystemDateAndTime->stUTCDateTime.stDate.day = atoi(p_Day->data);
    }

    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_SetScopes(XMLN *p_SetScopes, GONVIF_DEVMNG_SetScopes_S *pstDM_SetScopes)
{
    GK_S32 num = 0;
	XMLN *p_Scopes = soap_get_child_node(p_SetScopes, "Scopes");
	while(p_Scopes != NULL)
	{
	    num++;
		p_Scopes = p_Scopes->next;
    }
    
    pstDM_SetScopes->sizeScopes = num;
    pstDM_SetScopes->pszScopes = (GK_CHAR **)malloc(num * sizeof(GK_CHAR *));
    memset(pstDM_SetScopes->pszScopes, 0, num * sizeof(GK_CHAR *));
    num = 0;
	p_Scopes = soap_get_child_node(p_SetScopes, "Scopes");
	while(p_Scopes && num < pstDM_SetScopes->sizeScopes)
	{
		pstDM_SetScopes->pszScopes[num] = (GK_CHAR *)malloc(LEN_SCOPE_ITEM * sizeof(GK_CHAR));
		strncpy(pstDM_SetScopes->pszScopes[num], p_Scopes->data, LEN_SCOPE_ITEM-1);
		p_Scopes = p_Scopes->next;
		num++;
	}
	
	return ONVIF_OK;
}

ONVIF_RET soap_parse_request_AddScopes(XMLN *p_AddScopes, GONVIF_DEVMNG_AddScopes_S *pstDM_AddScopes)
{

    GK_S32 num = 0;
	XMLN *p_ScopeItem = soap_get_child_node(p_AddScopes, "ScopeItem");
	while(p_ScopeItem != NULL)
	{
	    num++;
		p_ScopeItem = p_ScopeItem->next;
    }
    
    pstDM_AddScopes->sizeScopeItem = num;
    pstDM_AddScopes->pszScopeItem = (GK_CHAR **)malloc(num * sizeof(GK_CHAR *));
    memset(pstDM_AddScopes->pszScopeItem, 0, num * sizeof(GK_CHAR *));
    num = 0;
	p_ScopeItem = soap_get_child_node(p_AddScopes, "ScopeItem");
	while(p_ScopeItem && num < pstDM_AddScopes->sizeScopeItem)
	{
		pstDM_AddScopes->pszScopeItem[num] = (GK_CHAR *)malloc(LEN_SCOPE_ITEM * sizeof(GK_CHAR));
		strncpy(pstDM_AddScopes->pszScopeItem[num], p_ScopeItem->data, LEN_SCOPE_ITEM-1);
		p_ScopeItem = p_ScopeItem->next;
		num++;
	}
	
	return ONVIF_OK;

}

ONVIF_RET soap_parse_request_RemoveScopes(XMLN *p_RemoveScopes, GONVIF_DEVMNG_RemoveScopes_S *pstDM_RemoveScopes)
{
    GK_S32 num = 0;
	XMLN *p_ScopeItem = soap_get_child_node(p_RemoveScopes, "ScopeItem");
	while(p_ScopeItem != NULL)
	{
	    num++;
		p_ScopeItem = p_ScopeItem->next;
    }
    
    pstDM_RemoveScopes->sizeScopeItem = num;
    pstDM_RemoveScopes->pszScopeItem = (GK_CHAR **)malloc(num * sizeof(GK_CHAR *));
    memset(pstDM_RemoveScopes->pszScopeItem, 0, num * sizeof(GK_CHAR *));
    num = 0;
	p_ScopeItem = soap_get_child_node(p_RemoveScopes, "ScopeItem");
	while(p_ScopeItem && num < pstDM_RemoveScopes->sizeScopeItem)
	{
		pstDM_RemoveScopes->pszScopeItem[num] = (GK_CHAR *)malloc(LEN_SCOPE_ITEM * sizeof(GK_CHAR));
		strncpy(pstDM_RemoveScopes->pszScopeItem[num], p_ScopeItem->data, LEN_SCOPE_ITEM-1);
		p_ScopeItem = p_ScopeItem->next;
		num++;
	}
	
	return ONVIF_OK;

}

ONVIF_RET soap_parse_request_SetDiscoveryMode(XMLN *p_node, GONIVF_DEVMNG_SetDiscoveryMode_S *pstDM_SetDiscoveryMode)
{
	XMLN * p_DiscoveryMode = soap_get_child_node(p_node, "DiscoveryMode");
	if (p_DiscoveryMode && p_DiscoveryMode->data)
	{
		if (strcasecmp(p_DiscoveryMode->data, "NonDiscoverable") == 0)
		{
			pstDM_SetDiscoveryMode->enDiscoveryMode = DiscoveryMode_NonDiscoverable;
		}
		else if (strcasecmp(p_DiscoveryMode->data, "Discoverable") == 0)
		{
			pstDM_SetDiscoveryMode->enDiscoveryMode = DiscoveryMode_Discoverable;
		}
	}

	return ONVIF_OK;
}

ONVIF_RET soap_parse_request_CreateUsers(XMLN *p_node, GONVIF_DEVMNG_CreateUsers_S *pstDM_CreateUsers)
{
    GK_S32 i = 0;
    XMLN *p_User = soap_get_child_node(p_node, "User");
    while(p_User && soap_match_tag(p_User->name, "User") == 0)
    {
        if(i < MAX_USER_NUM)
        {
            XMLN *p_UserLevel = soap_get_child_node(p_User, "UserLevel");
            if(p_UserLevel && p_UserLevel->data)
            {
                if(strcmp(p_UserLevel->data, "Administrator") == 0)
                {
                    pstDM_CreateUsers->stUser[i].enUserLevel = UserLevel_Administrator;
                }
                else if(strcmp(p_UserLevel->data, "Operator") == 0)
                {
                    pstDM_CreateUsers->stUser[i].enUserLevel = UserLevel_Operator;
                }
                else if(strcmp(p_UserLevel->data, "User") == 0)
                {
                    pstDM_CreateUsers->stUser[i].enUserLevel = UserLevel_User;
                }
                else if(strcmp(p_UserLevel->data, "Anonymous") == 0)
                {
                    pstDM_CreateUsers->stUser[i].enUserLevel = UserLevel_Anonymous;
                }
                else
                {
                    pstDM_CreateUsers->stUser[i].enUserLevel = UserLevel_Extended;
                }
            }
            else
            {
                return ONVIF_ERR_INVALID_SOAP_XML;
            }

            XMLN *p_UserName = soap_get_child_node(p_User, "Username");
            if(p_UserName && p_UserName->data)
            {
                strncpy(pstDM_CreateUsers->stUser[i].aszUsername, p_UserName->data, MAX_USERNAME_LENGTH - 1);
            }

            XMLN *p_Password = soap_get_child_node(p_User, "Password");
            if(p_Password && p_Password->data)
            {
                strncpy(pstDM_CreateUsers->stUser[i].aszPassword, p_Password->data, MAX_PASSWORD_LENGTH - 1);
            }
        }
        i++;
		p_User = p_User->next;
    }
    
    pstDM_CreateUsers->sizeUser = i;
    
    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_DeleteUsers(XMLN *p_node, GONVIF_DEVMNG_DeleteUsers_S *pstDM_DeleteUsers)
{
    GK_U32 i = 0;
	XMLN *p_Username = soap_get_child_node(p_node, "Username");
	while(p_Username && soap_match_tag(p_Username->name, "Username") == 0)
	{
		if(p_Username->data && i < MAX_USER_NUM)
		{
		   strncpy(pstDM_DeleteUsers->aszUsername[i], p_Username->data, MAX_USERNAME_LENGTH - 1);

		}
        i++;
		p_Username = p_Username->next;
	}
    pstDM_DeleteUsers->sizeUsername = i;
    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_SetUser(XMLN *p_node, GONVIF_DEVMNG_SetUser_S *pstDM_SetUser)
{
    GK_S32 i = 0;
    XMLN *p_User = soap_get_child_node(p_node, "User");
    while(p_User && soap_match_tag(p_User->name, "User") == 0)
    {
        if (i < MAX_USER_NUM)
        {
            XMLN *p_UserLevel = soap_get_child_node(p_User, "UserLevel");
            if (p_UserLevel && p_UserLevel->data)
            {
                if(strcmp(p_UserLevel->data, "Administrator") == 0)
                {
                    pstDM_SetUser->stUser[i].enUserLevel = UserLevel_Administrator;
                }
                else if(strcmp(p_UserLevel->data, "Operator") == 0)
                {
                    pstDM_SetUser->stUser[i].enUserLevel = UserLevel_Operator;
                }
                else if(strcmp(p_UserLevel->data, "User") == 0)
                {
                    pstDM_SetUser->stUser[i].enUserLevel = UserLevel_User;
                }
                else if(strcmp(p_UserLevel->data, "Anonymous") == 0)
                {
                    pstDM_SetUser->stUser[i].enUserLevel = UserLevel_Anonymous;
                }
                else
                {
                    pstDM_SetUser->stUser[i].enUserLevel = UserLevel_Extended;
                }
            }
            else
            {
                return ONVIF_ERR_INVALID_SOAP_XML;
            }

            XMLN *p_UserName = soap_get_child_node(p_User, "Username");
            if (p_UserName && p_UserName->data)
            {
                strncpy(pstDM_SetUser->stUser[i].aszUsername, p_UserName->data, MAX_USERNAME_LENGTH - 1);
            }

            XMLN *p_Password = soap_get_child_node(p_User, "Password");
            if (p_Password && p_Password->data)
            {
                strncpy(pstDM_SetUser->stUser[i].aszPassword, p_Password->data, MAX_PASSWORD_LENGTH - 1);
            }
        }
        i++;
		p_User = p_User->next;
    }
    pstDM_SetUser->sizeUser = i;
    
    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_SetSystemFactoryDefault(XMLN *p_node, GONVIF_DEVMNG_SetSystemFactoryDefault_S *stDM_SetSystemFactoryDefault)
{
    XMLN *p_FactoryDefault = soap_get_child_node(p_node, "FactoryDefault");
    if (p_FactoryDefault && p_FactoryDefault->data)
    {
    	if(strcasecmp(p_FactoryDefault->data, "Hard") == 0)
    	{
    		stDM_SetSystemFactoryDefault->enFactoryDefault = FactoryDefaultType_Hard;
    	}
        else if(strcasecmp(p_FactoryDefault->data, "Soft") == 0)
        {
    		stDM_SetSystemFactoryDefault->enFactoryDefault = FactoryDefaultType_Soft;
        }
        else
        {
            return ONVIF_ERR_INVALID_SOAP_XML;
        }
    }

    return ONVIF_OK;
}

/*********************************** device end **********************************/

/************************************* media ************************************/

ONVIF_RET soap_parse_request_CreateProfile(XMLN *p_node, GONVIF_MEDIA_CreateProfile_S *pstMD_CreateProfile)
{
	XMLN *p_Name = soap_get_child_node(p_node, "Name");
	if (p_Name && p_Name->data)
	{
		strncpy(pstMD_CreateProfile->aszName, p_Name->data, MAX_NAME_LENGTH - 1);
	}
	
	XMLN *p_Token = soap_get_child_node(p_node, "Token");
	if (p_Token && p_Token->data)
	{
		strncpy(pstMD_CreateProfile->aszToken, p_Token->data, MAX_TOKEN_LENGTH - 1);
	}
	
	return ONVIF_OK;
}

ONVIF_RET soap_parse_request_SetVideoSourceConfiguration(XMLN * p_node, GONVIF_MEDIA_SetVideoSourceConfiguration_S *pstMD_SetVideoSourceConfiguration)
{
	XMLN * p_Configuration = soap_get_child_node(p_node, "Configuration");
	if (p_Configuration)
	{
		const char *token = xml_attr_get_data(p_Configuration, "token");
		if (token)
		{
		    pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.pszToken = malloc(sizeof(GK_CHAR) * MAX_TOKEN_LENGTH);
            memset(pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.pszToken, 0, sizeof(GK_CHAR)*MAX_TOKEN_LENGTH);
			strncpy(pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.pszToken, token, MAX_TOKEN_LENGTH - 1);
		}
	}
	XMLN *p_Name = soap_get_child_node(p_Configuration, "Name");
	if (p_Name && p_Name->data)
	{
        pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.pszName = malloc(sizeof(GK_CHAR) * MAX_TOKEN_LENGTH);
        memset(pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.pszName, 0, sizeof(GK_CHAR)*MAX_TOKEN_LENGTH);
		strncpy(pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.pszName, p_Name->data, MAX_NAME_LENGTH - 1);
	}

	XMLN *p_UseCount = soap_get_child_node(p_Configuration, "UseCount");
	if (p_UseCount && p_UseCount->data)
	{
		pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.useCount = atoi(p_UseCount->data);
	}

	XMLN *p_SourceToken = soap_get_child_node(p_Configuration, "SourceToken");
	if (p_SourceToken && p_SourceToken->data)
	{

        pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.pszSourceToken = malloc(sizeof(GK_CHAR) * MAX_TOKEN_LENGTH);
        memset(pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.pszSourceToken, 0, sizeof(GK_CHAR)*MAX_TOKEN_LENGTH);
		strncpy(pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.pszSourceToken, p_SourceToken->data, MAX_TOKEN_LENGTH - 1);
	}

	XMLN *p_Bounds = soap_get_child_node(p_Configuration, "Bounds");
	if (p_Bounds)
	{
		const char * x = xml_attr_get_data(p_Bounds, "x");
		if (x)
		{
			pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.stBounds.x = atoi(x);
		}

		const char * y = xml_attr_get_data(p_Bounds, "y");
		if (y)
		{
			pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.stBounds.y = atoi(y);
		}

		const char * width = xml_attr_get_data(p_Bounds, "width");
		if (width)
		{
			pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.stBounds.width = atoi(width);
		}

		const char * height = xml_attr_get_data(p_Bounds, "height");
		if (height)
		{
			pstMD_SetVideoSourceConfiguration->stVideoSourceConfiguration.stBounds.height = atoi(height);
		}
	}

	XMLN *p_ForcePersistence = soap_get_child_node(p_node, "ForcePersistence");
	if (p_ForcePersistence && p_ForcePersistence->data)
	{
		if (strcasecmp(p_ForcePersistence->data, "true") == 0)
		{
			pstMD_SetVideoSourceConfiguration->enForcePersistence = Boolean_TRUE;
		}
        else
        {
			pstMD_SetVideoSourceConfiguration->enForcePersistence = Boolean_FALSE;
        }
	}

	return ONVIF_OK;
}

ONVIF_RET soap_parse_request_SetVideoEncoderConfiguration(XMLN *p_node, GONVIF_MEDIA_SetVideoEncoderConfiguration_S *pstMD_SetVideoEncoderConfiguration)
{
    XMLN *p_Configuration = soap_get_child_node(p_node, "Configuration");
    if (!p_Configuration)
    {
        return ONVIF_ERR_MISSINGATTR;
    }

    const char *token = xml_attr_get_data(p_Configuration, "token");
    if (token)
    {
        pstMD_SetVideoEncoderConfiguration->stConfiguration.pszToken = malloc(sizeof(GK_CHAR) * MAX_TOKEN_LENGTH);
        strncpy(pstMD_SetVideoEncoderConfiguration->stConfiguration.pszToken, token, MAX_TOKEN_LENGTH - 1);
    }

    XMLN *p_Name = soap_get_child_node(p_Configuration, "Name");
    if (p_Name && p_Name->data)
    {
        pstMD_SetVideoEncoderConfiguration->stConfiguration.pszName = malloc(sizeof(GK_CHAR) * MAX_NAME_LENGTH);
        strncpy(pstMD_SetVideoEncoderConfiguration->stConfiguration.pszName, p_Name->data, MAX_NAME_LENGTH - 1);
    }

    XMLN *p_UseCount = soap_get_child_node(p_Configuration, "UseCount");
    if (p_UseCount && p_UseCount->data)
    {
        pstMD_SetVideoEncoderConfiguration->stConfiguration.useCount = atoi(p_UseCount->data);
    }

    XMLN *p_Encoding = soap_get_child_node(p_Configuration, "Encoding");
    if (p_Encoding && p_Encoding->data)
    {
        if (strcasecmp(p_Encoding->data, "H264") == 0)
        {
        	pstMD_SetVideoEncoderConfiguration->stConfiguration.enEncoding = VideoEncoding_H264;
        }
        else if (strcasecmp(p_Encoding->data, "JPEG") == 0)
        {
        	pstMD_SetVideoEncoderConfiguration->stConfiguration.enEncoding = VideoEncoding_JPEG;
        }
        else if (strcasecmp(p_Encoding->data, "MPEG4") == 0)
        {
        	pstMD_SetVideoEncoderConfiguration->stConfiguration.enEncoding = VideoEncoding_MPEG;
        }
    }

    XMLN *p_Resolution = soap_get_child_node(p_Configuration, "Resolution");
    if (p_Resolution)
    {
        XMLN *p_Width = soap_get_child_node(p_Resolution, "Width");
	    if (p_Width && p_Width->data)
	    {
	        pstMD_SetVideoEncoderConfiguration->stConfiguration.stResolution.width = atoi(p_Width->data);
	    }

	    XMLN *p_Height = soap_get_child_node(p_Resolution, "Height");
	    if (p_Height && p_Height->data)
	    {
	        pstMD_SetVideoEncoderConfiguration->stConfiguration.stResolution.height = atoi(p_Height->data);
	    }
    }

    XMLN *p_Quality = soap_get_child_node(p_Configuration, "Quality");
    if (p_Quality && p_Quality->data)
    {
        pstMD_SetVideoEncoderConfiguration->stConfiguration.quality = atoi(p_Quality->data);
    }

    XMLN *p_RateControl = soap_get_child_node(p_Configuration, "RateControl");
    if (p_RateControl)
    {
        XMLN *p_FrameRateLimit = soap_get_child_node(p_RateControl, "FrameRateLimit");
	    if (p_FrameRateLimit && p_FrameRateLimit->data)
	    {
	        pstMD_SetVideoEncoderConfiguration->stConfiguration.stRateControl.frameRateLimit = atoi(p_FrameRateLimit->data);
	    }

	    XMLN *p_EncodingInterval = soap_get_child_node(p_RateControl, "EncodingInterval");
	    if (p_EncodingInterval && p_EncodingInterval->data)
	    {
	        pstMD_SetVideoEncoderConfiguration->stConfiguration.stRateControl.encodingInterval = atoi(p_EncodingInterval->data);
	    }

	    XMLN *p_BitrateLimit = soap_get_child_node(p_RateControl, "BitrateLimit");
	    if (p_BitrateLimit && p_BitrateLimit->data)
	    {
	        pstMD_SetVideoEncoderConfiguration->stConfiguration.stRateControl.bitrateLimit = atoi(p_BitrateLimit->data);
	    }
    }

    if (pstMD_SetVideoEncoderConfiguration->stConfiguration.enEncoding == VideoEncoding_H264)
    {
    	XMLN *p_H264 = soap_get_child_node(p_Configuration, "H264");
    	if (p_H264)
    	{
    		XMLN *p_GovLength = soap_get_child_node(p_H264, "GovLength");
		    if (p_GovLength && p_GovLength->data)
		    {
		        pstMD_SetVideoEncoderConfiguration->stConfiguration.stH264.govLength = atoi(p_GovLength->data);
		    }

		    XMLN *p_H264Profile = soap_get_child_node(p_H264, "H264Profile");
		    if (p_H264Profile && p_H264Profile->data)
		    {
		        if (strcasecmp(p_H264Profile->data, "Baseline") == 0)
		        {
		            pstMD_SetVideoEncoderConfiguration->stConfiguration.stH264.enH264Profile = H264Profile_BaseLine;
		        }
		        else if (strcasecmp(p_H264Profile->data, "Main") == 0)
		        {
		            pstMD_SetVideoEncoderConfiguration->stConfiguration.stH264.enH264Profile = H264Profile_Main;
		        }
		        else if (strcasecmp(p_H264Profile->data, "High") == 0)
		        {
		            pstMD_SetVideoEncoderConfiguration->stConfiguration.stH264.enH264Profile = H264Profile_High;
		        }
		        else if (strcasecmp(p_H264Profile->data, "Extended") == 0)
		        {
		            pstMD_SetVideoEncoderConfiguration->stConfiguration.stH264.enH264Profile = H264Profile_Extended;
		        }
		    }
    	}
    }
    else if (pstMD_SetVideoEncoderConfiguration->stConfiguration.enEncoding == VideoEncoding_MPEG)
    {
    	XMLN *p_MPEG4 = soap_get_child_node(p_Configuration, "MPEG4");
    	if (p_MPEG4)
    	{
    		XMLN *p_GovLength = soap_get_child_node(p_MPEG4, "GovLength");
		    if (p_GovLength && p_GovLength->data)
		    {
		        pstMD_SetVideoEncoderConfiguration->stConfiguration.stMPEG4.govLength = atoi(p_GovLength->data);
		    }

		    XMLN * p_Mpeg4Profile = soap_get_child_node(p_MPEG4, "Mpeg4Profile");
		    if (p_Mpeg4Profile && p_Mpeg4Profile->data)
		    {
		        if (strcasecmp(p_Mpeg4Profile->data, "SP") == 0)
		        {
		            pstMD_SetVideoEncoderConfiguration->stConfiguration.stMPEG4.enMpeg4Profile = Mpeg4Profile_SP;
		        }
		        else if (strcasecmp(p_Mpeg4Profile->data, "ASP") == 0)
		        {
		            pstMD_SetVideoEncoderConfiguration->stConfiguration.stMPEG4.enMpeg4Profile = Mpeg4Profile_ASP;
		        }
		    }
    	}
    }
	XMLN *p_Multicast = soap_get_child_node(p_Configuration, "Multicast");
	if (p_Multicast)
	{
    	XMLN *p_Address = soap_get_child_node(p_Multicast, "Address");
    	if (p_Address)
        {
            XMLN *p_IpType = soap_get_child_node(p_Address, "Type");
            if (p_IpType && p_IpType->data)
            {
                pstMD_SetVideoEncoderConfiguration->stConfiguration.stMulticast.stIPAddress.enType = atoi(p_IpType->data);
            }

            XMLN *p_Ipv4Address = soap_get_child_node(p_Address, "IPv4Address");
            if (p_Ipv4Address && p_Ipv4Address->data)
            {
                pstMD_SetVideoEncoderConfiguration->stConfiguration.stMulticast.stIPAddress.pszIPv4Address = malloc(sizeof(GK_CHAR) * IPV4_STR_LENGTH);
                strncpy(pstMD_SetVideoEncoderConfiguration->stConfiguration.stMulticast.stIPAddress.pszIPv4Address, p_Ipv4Address->data, IPV4_STR_LENGTH - 1);
            }
            XMLN *p_Ipv6Address = soap_get_child_node(p_Address, "IPv6Address");
            if (p_Ipv6Address && p_Ipv6Address->data)
            {
                pstMD_SetVideoEncoderConfiguration->stConfiguration.stMulticast.stIPAddress.pszIPv6Address = malloc(sizeof(GK_CHAR) * IPV6_STR_LENGTH);
                strncpy(pstMD_SetVideoEncoderConfiguration->stConfiguration.stMulticast.stIPAddress.pszIPv6Address, p_Ipv6Address->data, IPV6_STR_LENGTH - 1);
            }
        }
    	XMLN *p_Port = soap_get_child_node(p_Multicast, "Port");
        if (p_Port && p_Port->data)
        {
            pstMD_SetVideoEncoderConfiguration->stConfiguration.stMulticast.port = atoi(p_Port->data);
        }
    	XMLN *p_TTL = soap_get_child_node(p_Multicast, "TTL");
        if (p_TTL && p_TTL->data)
        {
            pstMD_SetVideoEncoderConfiguration->stConfiguration.stMulticast.ttl = atoi(p_TTL->data);
        }

    	XMLN *p_AutoStart = soap_get_child_node(p_Multicast, "AutoStart");
    	if (p_AutoStart && p_AutoStart->data)
    	{
    		if (strcasecmp(p_AutoStart->data, "true") == 0)
    		{
    			pstMD_SetVideoEncoderConfiguration->stConfiguration.stMulticast.enAutoStart = Boolean_TRUE;
    		}
            else
            {
    			pstMD_SetVideoEncoderConfiguration->stConfiguration.stMulticast.enAutoStart = Boolean_FALSE;
            }
    	}
	}
	XMLN *p_SessionTimeout = soap_get_child_node(p_Configuration, "SessionTimeout");
	if (p_SessionTimeout && p_SessionTimeout->data)
	{
		pstMD_SetVideoEncoderConfiguration->stConfiguration.sessionTimeout = atoi(p_SessionTimeout->data);
	}

	XMLN *p_ForcePersistence = soap_get_child_node(p_node, "ForcePersistence");
	if (p_ForcePersistence && p_ForcePersistence->data)
	{
		if (strcasecmp(p_ForcePersistence->data, "true") == 0)
		{
			pstMD_SetVideoEncoderConfiguration->enForcePersistence = Boolean_TRUE;
		}
        else
        {
			pstMD_SetVideoEncoderConfiguration->enForcePersistence = Boolean_FALSE;
        }
	}

    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_SetAudioSourceConfiguration(XMLN * p_node, GONVIF_MEDIA_SetAudioSourceConfiguration_S *pstMD_SetAudioSourceConfiguration)
{
	XMLN * p_Configuration = soap_get_child_node(p_node, "Configuration");
	if (p_Configuration)
	{
		const char *token = xml_attr_get_data(p_Configuration, "token");
		if (token)
		{
		    pstMD_SetAudioSourceConfiguration->stConfiguration.pszToken = malloc(sizeof(GK_CHAR) * MAX_TOKEN_LENGTH);
            memset(pstMD_SetAudioSourceConfiguration->stConfiguration.pszToken, 0, sizeof(GK_CHAR)*MAX_TOKEN_LENGTH);
			strncpy(pstMD_SetAudioSourceConfiguration->stConfiguration.pszToken, token, MAX_TOKEN_LENGTH - 1);
		}
	}
	XMLN *p_Name = soap_get_child_node(p_Configuration, "Name");
	if (p_Name && p_Name->data)
	{
        pstMD_SetAudioSourceConfiguration->stConfiguration.pszName = malloc(sizeof(GK_CHAR) * MAX_TOKEN_LENGTH);
        memset(pstMD_SetAudioSourceConfiguration->stConfiguration.pszName, 0, sizeof(GK_CHAR)*MAX_TOKEN_LENGTH);
		strncpy(pstMD_SetAudioSourceConfiguration->stConfiguration.pszName, p_Name->data, MAX_NAME_LENGTH - 1);
	}

	XMLN *p_UseCount = soap_get_child_node(p_Configuration, "UseCount");
	if (p_UseCount && p_UseCount->data)
	{
		pstMD_SetAudioSourceConfiguration->stConfiguration.useCount = atoi(p_UseCount->data);
	}

	XMLN *p_SourceToken = soap_get_child_node(p_Configuration, "SourceToken");
	if (p_SourceToken && p_SourceToken->data)
	{

        pstMD_SetAudioSourceConfiguration->stConfiguration.pszSourceToken = malloc(sizeof(GK_CHAR) * MAX_TOKEN_LENGTH);
        memset(pstMD_SetAudioSourceConfiguration->stConfiguration.pszSourceToken, 0, sizeof(GK_CHAR)*MAX_TOKEN_LENGTH);
		strncpy(pstMD_SetAudioSourceConfiguration->stConfiguration.pszSourceToken, p_SourceToken->data, MAX_TOKEN_LENGTH - 1);
	}

	XMLN *p_ForcePersistence = soap_get_child_node(p_node, "ForcePersistence");
	if (p_ForcePersistence && p_ForcePersistence->data)
	{
		if (strcasecmp(p_ForcePersistence->data, "true") == 0)
		{
			pstMD_SetAudioSourceConfiguration->enForcePersistence = Boolean_TRUE;
		}
        else
        {
			pstMD_SetAudioSourceConfiguration->enForcePersistence = Boolean_FALSE;
        }
	}

	return ONVIF_OK;
}

ONVIF_RET soap_parse_request_SetAudioEncoderConfiguration(XMLN *p_node, GONVIF_MEDIA_SetAudioEncoderConfiguration_S *pstSetAudioEncoderConfiguration)
{
    XMLN *p_Configuration = soap_get_child_node(p_node, "Configuration");
    if (!p_Configuration)
    {
        return ONVIF_ERR_MISSINGATTR;
    }

    const char *token = xml_attr_get_data(p_Configuration, "token");
    if (token)
    {
        pstSetAudioEncoderConfiguration->stConfiguration.pszToken = malloc(sizeof(GK_CHAR) * MAX_TOKEN_LENGTH);
        strncpy(pstSetAudioEncoderConfiguration->stConfiguration.pszToken, token, MAX_TOKEN_LENGTH - 1);
    }

    XMLN *p_Name = soap_get_child_node(p_Configuration, "Name");
    if (p_Name && p_Name->data)
    {
        pstSetAudioEncoderConfiguration->stConfiguration.pszName = malloc(sizeof(GK_CHAR) * MAX_NAME_LENGTH);
        strncpy(pstSetAudioEncoderConfiguration->stConfiguration.pszName, p_Name->data, MAX_NAME_LENGTH - 1);
    }

    XMLN *p_UseCount = soap_get_child_node(p_Configuration, "UseCount");
    if (p_UseCount && p_UseCount->data)
    {
        pstSetAudioEncoderConfiguration->stConfiguration.useCount = atoi(p_UseCount->data);
    }

    XMLN *p_Encoding = soap_get_child_node(p_Configuration, "Encoding");
    if (p_Encoding && p_Encoding->data)
    {
        if (strcasecmp(p_Encoding->data, "G711") == 0)
        {
        	pstSetAudioEncoderConfiguration->stConfiguration.enEncoding = AudioEncoding_G711;
        }
        else if (strcasecmp(p_Encoding->data, "G726") == 0)
        {
        	pstSetAudioEncoderConfiguration->stConfiguration.enEncoding = AudioEncoding_G726;
        }
        else if (strcasecmp(p_Encoding->data, "AAC") == 0)
        {
        	pstSetAudioEncoderConfiguration->stConfiguration.enEncoding = AudioEncoding_AAC;
        }
    }

    XMLN * p_Bitrate = soap_get_child_node(p_Configuration, "Bitrate");
    if (p_Bitrate && p_Bitrate->data)
    {
        pstSetAudioEncoderConfiguration->stConfiguration.bitRate = atoi(p_Bitrate->data);
    }

    XMLN * p_SampleRate = soap_get_child_node(p_Configuration, "SampleRate");
    if (p_SampleRate && p_SampleRate->data)
    {
        pstSetAudioEncoderConfiguration->stConfiguration.sampleRate = atoi(p_SampleRate->data);
    }

	XMLN *p_Multicast = soap_get_child_node(p_Configuration, "Multicast");
	if (p_Multicast)
	{
    	XMLN *p_Address = soap_get_child_node(p_Multicast, "Address");
    	if (p_Address)
        {
            XMLN *p_IpType = soap_get_child_node(p_Address, "Type");
            if (p_IpType && p_IpType->data)
            {
                pstSetAudioEncoderConfiguration->stConfiguration.stMulticast.stIPAddress.enType = atoi(p_IpType->data);
            }

            XMLN *p_Ipv4Address = soap_get_child_node(p_Address, "IPv4Address");
            if (p_Ipv4Address && p_Ipv4Address->data)
            {
                pstSetAudioEncoderConfiguration->stConfiguration.stMulticast.stIPAddress.pszIPv4Address = malloc(sizeof(GK_CHAR) * IPV4_STR_LENGTH);
                strncpy(pstSetAudioEncoderConfiguration->stConfiguration.stMulticast.stIPAddress.pszIPv4Address, p_Ipv4Address->data, IPV4_STR_LENGTH - 1);
            }
            XMLN *p_Ipv6Address = soap_get_child_node(p_Address, "IPv6Address");
            if (p_Ipv6Address && p_Ipv6Address->data)
            {
                pstSetAudioEncoderConfiguration->stConfiguration.stMulticast.stIPAddress.pszIPv6Address = malloc(sizeof(GK_CHAR) * IPV6_STR_LENGTH);
                strncpy(pstSetAudioEncoderConfiguration->stConfiguration.stMulticast.stIPAddress.pszIPv6Address, p_Ipv6Address->data, IPV6_STR_LENGTH - 1);
            }
        }
    	XMLN *p_Port = soap_get_child_node(p_Multicast, "Port");
        if (p_Port && p_Port->data)
        {
            pstSetAudioEncoderConfiguration->stConfiguration.stMulticast.port = atoi(p_Port->data);
        }
    	XMLN *p_TTL = soap_get_child_node(p_Multicast, "TTL");
        if (p_TTL && p_TTL->data)
        {
            pstSetAudioEncoderConfiguration->stConfiguration.stMulticast.ttl = atoi(p_TTL->data);
        }

    	XMLN *p_AutoStart = soap_get_child_node(p_Multicast, "AutoStart");
    	if (p_AutoStart && p_AutoStart->data)
    	{
    		if (strcasecmp(p_AutoStart->data, "true") == 0)
    		{
    			pstSetAudioEncoderConfiguration->stConfiguration.stMulticast.enAutoStart = Boolean_TRUE;
    		}
            else
            {
    			pstSetAudioEncoderConfiguration->stConfiguration.stMulticast.enAutoStart = Boolean_FALSE;
            }
    	}
	}

	XMLN *p_SessionTimeout = soap_get_child_node(p_Configuration, "SessionTimeout");
	if (p_SessionTimeout && p_SessionTimeout->data)
	{
		pstSetAudioEncoderConfiguration->stConfiguration.sessionTimeout = atoi(p_SessionTimeout->data);
	}

	XMLN *p_ForcePersistence = soap_get_child_node(p_node, "ForcePersistence");
	if (p_ForcePersistence && p_ForcePersistence->data)
	{
		if (strcasecmp(p_ForcePersistence->data, "true") == 0)
		{
			pstSetAudioEncoderConfiguration->enForcePersistence = Boolean_TRUE;
		}
        else
        {
			pstSetAudioEncoderConfiguration->enForcePersistence = Boolean_FALSE;
        }
	}

    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_GetStreamUri(XMLN *p_node, GONVIF_MEDIA_GetStreamUri_S *pstMD_GetStreamUri)
{
	XMLN *p_StreamSetup = soap_get_child_node(p_node, "StreamSetup");
	if (p_StreamSetup)
	{
		XMLN * p_Stream = soap_get_child_node(p_StreamSetup, "Stream");
		if (p_Stream && p_Stream->data)
		{
			if (strcasecmp(p_Stream->data, "RTP-Unicast") == 0)
			{
				pstMD_GetStreamUri->stStreamSetup.enStream = StreamType_RTP_Unicast;
			}
			else if (strcasecmp(p_Stream->data, "RTP-Multicast") == 0)
			{
				pstMD_GetStreamUri->stStreamSetup.enStream = StreamType_RTP_Multicast;
			}
		}

		XMLN * p_Transport = soap_get_child_node(p_StreamSetup, "Transport");
		if (p_Transport)
		{
			XMLN * p_Protocol = soap_get_child_node(p_Transport, "Protocol");
			if (p_Protocol && p_Protocol->data)
			{
				if (strcasecmp(p_Protocol->data, "UDP") == 0)
				{
					pstMD_GetStreamUri->stStreamSetup.stTransport.enProtocol = TransportProtocol_UDP;
				}
				else if (strcasecmp(p_Protocol->data, "TCP") == 0)
				{
					pstMD_GetStreamUri->stStreamSetup.stTransport.enProtocol = TransportProtocol_TCP;
				}
				else if (strcasecmp(p_Protocol->data, "RTSP") == 0)
				{
					pstMD_GetStreamUri->stStreamSetup.stTransport.enProtocol = TransportProtocol_RTSP;
				}
				else if (strcasecmp(p_Protocol->data, "HTTP") == 0)
				{
					pstMD_GetStreamUri->stStreamSetup.stTransport.enProtocol = TransportProtocol_HTTP;
				}
			}
		}
	}

	XMLN * p_ProfileToken = soap_get_child_node(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
	{
		strncpy(pstMD_GetStreamUri->aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
	}
	return ONVIF_OK;
}

ONVIF_RET soap_parse_request_GetOSDOptions(XMLN *p_body, GONVIF_MEDIA_GetOSDOptions_Token_S *pstGetOSDOptionsReq)
{
    XMLN *p_GetOptions = soap_get_child_node(p_body, "GetOSDOptions");
    if(p_GetOptions != NULL)
    {
        XMLN *p_ConfigurationToken = soap_get_child_node(p_GetOptions, "ConfigurationToken");
        if ((p_ConfigurationToken != NULL) && (p_ConfigurationToken->data != NULL))
        {
            strcpy(pstGetOSDOptionsReq->aszConfigurationToken, p_ConfigurationToken->data);
        }
        else
        {
            return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
        }
    }
    else
    {
        return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
    }
    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_GetOSDs(XMLN *p_body, GONVIF_MEDIA_GetOSDs_Token_S *pstGetOSDsRes)
{
    XMLN *p_GetOSDs = NULL;
    const char *child_name = soap_get_tag_child_name(p_body->f_child->name);
	if (child_name)
        p_GetOSDs = soap_get_child_node(p_body, child_name); // soap_get_child_node(p_body, "GetOSDs");
    if(p_GetOSDs != NULL)
    {
        XMLN *p_ConfigurationToken = soap_get_child_node(p_GetOSDs, "ConfigurationToken");
        XMLN *p_OSDToken = soap_get_child_node(p_GetOSDs, "OSDToken");
        if ((p_ConfigurationToken != NULL) && (p_ConfigurationToken->data != NULL))
        {
            strcpy(pstGetOSDsRes->aszConfigurationToken, p_ConfigurationToken->data);
        }
        else if ((p_OSDToken != NULL) && (p_OSDToken->data != NULL))
        {
            strcpy(pstGetOSDsRes->aszOsdToken, p_OSDToken->data);
        }
        else
        {
            return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
        }
    }
    else
    {
        return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
    }
    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_SetOSD(XMLN *p_body, GONVIF_MEDIA_SetOSD_S *pstSetOSDRes)
{
    XMLN *p_node = NULL;
    const char *child_name = soap_get_tag_child_name(p_body->f_child->name);
	if (child_name)
        p_node = soap_get_child_node(p_body, child_name); // soap_get_child_node(p_body, "SetOSD");
    if(p_node == NULL)
    {
        return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
    }
    XMLN *p_OSD = soap_get_child_node(p_node, "OSD");
    if(p_OSD == NULL)
    {
        return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
    }
    const GK_CHAR *p_OSDToken = xml_attr_get_data(p_OSD, "token");
    if(p_OSDToken)
    {
        strcpy(pstSetOSDRes->stOSD.aszOsdToken, p_OSDToken);
    }
    
    XMLN * p_VideoSourceToken = soap_get_child_node(p_OSD, "VideoSourceConfigurationToken");
    if(p_VideoSourceToken && p_VideoSourceToken->data)
    {
        strcpy(pstSetOSDRes->stOSD.aszConfigurationToken, p_VideoSourceToken->data);
    }
    XMLN * p_Type = soap_get_child_node(p_OSD, "Type");
    if(p_Type && p_Type->data)
    {
        if(0 == strcasecmp(p_Type->data, "Text"))
            pstSetOSDRes->stOSD.stType = OSDType_Text;
        else
            pstSetOSDRes->stOSD.stType = OSDType_Iamge;
    }
    
    XMLN * p_Position = soap_get_child_node(p_OSD, "Position");
    if(p_Position)
    {
        XMLN * p_Type = soap_get_child_node(p_Position, "Type");
        if(p_Type && p_Type->data)
        {
            if(0 == strcasecmp(p_Type->data, "Custom"))
            {
                pstSetOSDRes->stOSD.stPosition.penType = PositionOption_Custom;
                XMLN * p_Pos = soap_get_child_node(p_Position, "Pos");
                if(p_Pos)
                {
                    const GK_CHAR *p_Pos_x, *p_Pos_y;
                    p_Pos_x = xml_attr_get_data(p_Pos, "x");
                    p_Pos_y = xml_attr_get_data(p_Pos, "y");
            		if(p_Pos_x && p_Pos_y)
            		{
            			pstSetOSDRes->stOSD.stPosition.stPos.x = atof(p_Pos_x);
            			pstSetOSDRes->stOSD.stPosition.stPos.y = atof(p_Pos_y);
            		}
                }
            }
            else if(0 == strcasecmp(p_Type->data, "UpperLeft"))
            {
                pstSetOSDRes->stOSD.stPosition.penType = PositionOption_UpperLeft;
            }
            else if(0 == strcasecmp(p_Type->data, "LowerLeft"))
            {
                pstSetOSDRes->stOSD.stPosition.penType = PositionOption_LowerLeft;
            }
        }
    }

    XMLN * p_TextString = soap_get_child_node(p_OSD, "TextString");
    if(p_TextString)
    {
        XMLN * p_Type = soap_get_child_node(p_TextString, "Type");
        if(p_Type && p_Type->data)
        {
            if(0 == strcasecmp(p_Type->data, "Plain"))
            {
                pstSetOSDRes->stOSD.stTextString.penType = TextType_Plain;
                XMLN * p_PlainText = soap_get_child_node(p_TextString, "PlainText");
                if(p_PlainText && p_PlainText->data)
                {
                    strcpy(pstSetOSDRes->stOSD.stTextString.stType.stPlain.aszPlainText, p_PlainText->data);
                    pstSetOSDRes->stOSD.stTextString.stExtension.enChannelName = GK_TRUE;
                }
                else
                    pstSetOSDRes->stOSD.stTextString.stExtension.enChannelName = GK_FALSE;
            }
            else
            {
                XMLN * p_DateFormat = soap_get_child_node(p_TextString, "DateFormat");
                XMLN * p_TimeFormat = soap_get_child_node(p_TextString, "TimeFormat");
                if(p_DateFormat && p_DateFormat->data)
                {
                    pstSetOSDRes->stOSD.stTextString.penType = TextType_Date;
                    if(0 == strcasecmp(p_DateFormat->data, "MM/dd/yyyy"))
                        pstSetOSDRes->stOSD.stTextString.stType.stDateAndTime.penDateFormat = DateFormat_MMddyyyy;
                    else if(0 == strcasecmp(p_DateFormat->data, "dd/MM/yyyy"))
                        pstSetOSDRes->stOSD.stTextString.stType.stDateAndTime.penDateFormat = DateFormat_ddMMyyyy;
                    else if(0 == strcasecmp(p_DateFormat->data, "yyyy/MM/dd"))
                        pstSetOSDRes->stOSD.stTextString.stType.stDateAndTime.penDateFormat = DateFormat_yyyyMMdd;
                    else if(0 == strcasecmp(p_DateFormat->data, "yyyy-MM-dd"))
                        pstSetOSDRes->stOSD.stTextString.stType.stDateAndTime.penDateFormat = DateFormat_yyyy_MM_dd;
                }
                if(p_TimeFormat && p_TimeFormat->data)
                {
                    pstSetOSDRes->stOSD.stTextString.penType |= TextType_Time;
                    if(0 == strcasecmp(p_TimeFormat->data, "HH:mm:ss"))
                        pstSetOSDRes->stOSD.stTextString.stType.stDateAndTime.penTimeFormat = TimeFormat_HHmmss;
                    else if(0 == strcasecmp(p_TimeFormat->data, "hh:mm:ss tt"))
                        pstSetOSDRes->stOSD.stTextString.stType.stDateAndTime.penTimeFormat = TimeFormat_hhmmsstt;
                }
            }
        }
    }

    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_DeleteOSD(XMLN *p_body, GONVIF_MEDIA_DeleteOSD_S *pstDeleteOSDRes)
{
    XMLN *p_DeleteOSD = soap_get_child_node(p_body, "DeleteOSD");
    if(p_DeleteOSD != NULL)
    {
        XMLN *p_OSDToken = soap_get_child_node(p_DeleteOSD, "OSDToken");
        if ((p_OSDToken != NULL) && (p_OSDToken->data != NULL))
        {
            strcpy(pstDeleteOSDRes->aszOsdToken, p_OSDToken->data);
        }
        else
        {
            return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
        }
    }
    else
    {
        return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
    }
    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_GetPrivacyMaskOptions(XMLN *p_body, GONVIF_MEDIA_GetPrivacyMaskOptions_Token_S *pstGetPrivacyMaskOptionsRes)
{
    XMLN *p_GetOptions = soap_get_child_node(p_body, "GetPrivacyMaskOptions");
    if(p_GetOptions != NULL)
    {
        XMLN *p_VideoSourceToken = soap_get_child_node(p_GetOptions, "VideoSourceToken");
        if ((p_VideoSourceToken != NULL) && (p_VideoSourceToken->data != NULL))
        {
            strcpy(pstGetPrivacyMaskOptionsRes->aszVideoSourceToken, p_VideoSourceToken->data);
        }
        else
        {
            return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
        }
    }
    else
    {
        return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
    }
    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_GetPrivacyMasks(XMLN *p_body, GONVIF_MEDIA_GetPrivacyMasks_Token_S *pstPrivacyMasksRes)
{
    XMLN *p_GetOptions = soap_get_child_node(p_body, "GetPrivacyMasks");
    if(p_GetOptions != NULL)
    {
        XMLN *p_VideoSourceToken = soap_get_child_node(p_GetOptions, "VideoSourceToken");
        if ((p_VideoSourceToken != NULL) && (p_VideoSourceToken->data != NULL))
        {
            strcpy(pstPrivacyMasksRes->aszVideoSourceToken, p_VideoSourceToken->data);
        }
        else
        {
            return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
        }
    }
    else
    {
        return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
    }
    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_DeletePrivacyMask(XMLN *p_body, GONVIF_MEDIA_DeletePrivacyMask_Token_S *pstPrivacyMasksRes)
{
    XMLN *p_GetOptions = soap_get_child_node(p_body, "DeletePrivacyMask");
    if(p_GetOptions != NULL)
    {
        XMLN *p_VideoSourceToken = soap_get_child_node(p_GetOptions, "PrivacyMaskToken");
        if ((p_VideoSourceToken != NULL) && (p_VideoSourceToken->data != NULL))
        {
            strcpy(pstPrivacyMasksRes->aszPrivacyMaskToken, p_VideoSourceToken->data);
        }
        else
        {
            return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
        }
    }
    else
    {
        return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
    }
    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_CreatePrivacyMask(XMLN *p_body, GONVIF_MEDIA_PrivacyMask_S *pstCreatePrivacyMaskRes)
{
    XMLN *p_node = soap_get_child_node(p_body, "CreatePrivacyMask");
    if(p_node == NULL)
    {
        return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
    }
    XMLN *p_PrivacyMask = soap_get_child_node(p_node, "PrivacyMask");
    if(p_PrivacyMask == NULL)
    {
        return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
    }
    const GK_CHAR *p_PrivacyMaskToken = xml_attr_get_data(p_PrivacyMask, "token");
    if(p_PrivacyMaskToken)
    {
        strcpy(pstCreatePrivacyMaskRes->aszPrivacyMaskToken, p_PrivacyMaskToken);
    }
    
    XMLN * p_VideoSourceToken = soap_get_child_node(p_PrivacyMask, "VideoSourceToken");
    if(p_VideoSourceToken && p_VideoSourceToken->data)
    {
        strcpy(pstCreatePrivacyMaskRes->aszVideoSourceToken, p_VideoSourceToken->data);
    }
    XMLN * p_MaskArea = soap_get_child_node(p_PrivacyMask, "MaskArea");
    if(p_MaskArea)
    {
        int i;
        const GK_CHAR *p_Point_x, *p_Point_y;
        XMLN * p_Point = soap_get_child_node(p_MaskArea, "Point");
        for(i=0;i<4;i++)
        {
            if(p_Point == NULL)
                continue;
            p_Point_x = xml_attr_get_data(p_Point, "x");
            p_Point_y = xml_attr_get_data(p_Point, "y");
    		if(p_Point_x && p_Point_y)
    		{
    			pstCreatePrivacyMaskRes->stMaskAreaPoint[i].x = atof(p_Point_x);
    			pstCreatePrivacyMaskRes->stMaskAreaPoint[i].y = atof(p_Point_y);
    		}
            p_Point = soap_get_next_node(p_Point, "Point");
        }
        if(i>=4)
            pstCreatePrivacyMaskRes->enMaskAreaPoint = Boolean_TRUE;
    }

    return ONVIF_OK;
}


/*********************************** media end **********************************/


/************************************************* event  **************************************************/
ONVIF_RET soap_parse_request_Subscribe(XMLN *p_node, GONVIF_EVENT_Subscribe_S *pstSubscribe)
{
	XMLN *p_ConsumerReference = soap_get_child_node(p_node, "ConsumerReference");
	if(NULL == p_ConsumerReference)
	{
		return ONVIF_ERR_MISSINGATTR;
	}

	XMLN *p_Address = soap_get_child_node(p_ConsumerReference, "Address");
	if(p_Address && p_Address->data)
	{
		strncpy(pstSubscribe->stConsumerReference.aszAddress, p_Address->data, MAX_ADDR_LENGTH - 1);
	}

	XMLN *p_InitialTerminationTime = soap_get_child_node(p_node, "InitialTerminationTime");
	if(p_InitialTerminationTime && p_InitialTerminationTime->data)
	{
		strncpy(pstSubscribe->aszInitialTerminationTime, p_InitialTerminationTime->data, MAX_PHHMS_TIME_LEN-1);
	}

    XMLN *p_Filter  = soap_get_child_node(p_node, "Filter");
	if(p_Filter != NULL)
    {
        XMLN *p_TopicExpression  = soap_get_child_node(p_Filter, "TopicExpression");
        if(p_TopicExpression && p_TopicExpression->data)
        {
            strncpy(pstSubscribe->stFilter.aszTopicExpression, p_TopicExpression->data, MAX_TOPICEXPRESSION_LEN - 1);
        }
        XMLN *p_MessageContent  = soap_get_child_node(p_Filter, "MessageContent");
        if(p_MessageContent && p_MessageContent->data)
        {
            strncpy(pstSubscribe->stFilter.aszMessageContent, p_MessageContent->data, MAX_MESSAGECONTENT_LEN - 1);
        }
    }

	return ONVIF_OK;
}

ONVIF_RET soap_parse_request_CreatePullPointSubscription(XMLN *p_node, GONVIF_EVENT_CreatePullPointSubscription_S *pstCreatePullPoint)
{
	XMLN *p_InitialTerminationTime = soap_get_child_node(p_node, "InitialTerminationTime");
	if (p_InitialTerminationTime && p_InitialTerminationTime->data)
	{
		strncpy(pstCreatePullPoint->aszInitialTerminationTime, p_InitialTerminationTime->data , MAX_PHHMS_TIME_LEN-1);
	}

    XMLN *p_Filter  = soap_get_child_node(p_node, "Filter");
	if (p_Filter != NULL)
    {
        XMLN *p_TopicExpression  = soap_get_child_node(p_Filter, "TopicExpression");
        if (p_TopicExpression && p_TopicExpression->data)
        {
            strncpy(pstCreatePullPoint->stFilter.aszTopicExpression, p_TopicExpression->data, MAX_TOPICEXPRESSION_LEN - 1);
        }
        XMLN *p_MessageContent  = soap_get_child_node(p_Filter, "MessageContent");
        if (p_MessageContent && p_MessageContent->data)
        {
            strncpy(pstCreatePullPoint->stFilter.aszMessageContent, p_MessageContent->data, MAX_MESSAGECONTENT_LEN - 1);
        }
    }

	return ONVIF_OK;
}


/************************************************* image **************************************************/
ONVIF_RET soap_parse_request_GetImagingSettings(XMLN *p_body, GONVIF_IMAGE_GetSettings_Token_S *pstGetSettingsReq)
{
    XMLN *p_GetImagingSettings = soap_get_child_node(p_body, "GetImagingSettings");
    if(p_GetImagingSettings != NULL)
    {
        XMLN *p_VideoSourceToken = soap_get_child_node(p_GetImagingSettings, "VideoSourceToken");
        if ((p_VideoSourceToken != NULL) && (p_VideoSourceToken->data != NULL))
        {
            strcpy(pstGetSettingsReq->aszVideoSourceToken, p_VideoSourceToken->data);
        }
        else
        {
            return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
        }
    }
    else
    {
        return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
    }
    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_SetImagingSettings(XMLN *p_body, GONVIF_IMAGE_SetSettings_S *pstSetImagingSettingsReq)
{
    XMLN *p_SetImagingSettings = soap_get_child_node(p_body, "SetImagingSettings");
    if(p_SetImagingSettings == NULL)
    {
        ONVIF_ERR();
        goto XML_invalid;
    }
    //token
    XMLN *p_VideoSourceToken = soap_get_child_node(p_SetImagingSettings, "VideoSourceToken");
    if ((p_VideoSourceToken != NULL) && (p_VideoSourceToken->data != NULL))
    {
        strncpy(pstSetImagingSettingsReq->aszVideoSourceToken, p_VideoSourceToken->data, MAX_TOKEN_LENGTH-1);
        pstSetImagingSettingsReq->aszVideoSourceToken[MAX_TOKEN_LENGTH-1] = '\0';
    }
    else
    {
        ONVIF_ERR();
        goto XML_invalid;
    }
    //ImagingSettings
    XMLN *p_ImagingSettings = soap_get_child_node(p_SetImagingSettings, "ImagingSettings");
    if(p_ImagingSettings != NULL)
    {
        //BacklightCompensation
        XMLN *p_BacklightCompensation = soap_get_child_node(p_ImagingSettings, "BacklightCompensation");
        if(p_BacklightCompensation != NULL)
        {
            //Mode
            XMLN *p_BacklightCompensation_Mode = soap_get_child_node(p_BacklightCompensation, "Mode");
            if((p_BacklightCompensation_Mode != NULL) && (p_BacklightCompensation_Mode->data != NULL))
            {
                if(strcmp(p_BacklightCompensation_Mode->data, "ON") == 0)
                {
                    pstSetImagingSettingsReq->stImageSettings.stBacklightCompensation.enMode = BacklightCompensationMode_ON;
                    //Level
                    XMLN *p_BacklightCompensation_Level = soap_get_child_node(p_BacklightCompensation, "Level");
                    if(p_BacklightCompensation_Level != NULL)
                    {
                        if(p_BacklightCompensation_Level->data != NULL)
                        {
                            pstSetImagingSettingsReq->stImageSettings.stBacklightCompensation.level = atof(p_BacklightCompensation_Level->data);
                        }
                        else
                        {
                            ONVIF_ERR();
                            goto XML_invalid;
                        }
                    }

                }
                else if(strcmp(p_BacklightCompensation_Mode->data, "OFF") == 0)
                {
                    pstSetImagingSettingsReq->stImageSettings.stBacklightCompensation.enMode = BacklightCompensationMode_OFF;
                }
                else
                {
                    ONVIF_ERR();
                    goto XML_invalid;
                }
            }
            else
            {
                ONVIF_ERR();
                goto XML_invalid;
            }
        }

        //Brightness
        XMLN *p_Brightness = soap_get_child_node(p_ImagingSettings, "Brightness");
        if(p_Brightness != NULL)
        {
            if(p_Brightness->data != NULL)
            {
                pstSetImagingSettingsReq->stImageSettings.brightness = atof(p_Brightness->data);
            }
            else
            {
                ONVIF_ERR();
                goto XML_invalid;
            }
        }
        //ColorSaturation
        XMLN *p_ColorSaturation = soap_get_child_node(p_ImagingSettings, "ColorSaturation");
        if(p_ColorSaturation != NULL)
        {
            if(p_ColorSaturation->data != NULL)
            {
                pstSetImagingSettingsReq->stImageSettings.colorSaturation = atof(p_ColorSaturation->data);
            }
            else
            {
                ONVIF_ERR();
                goto XML_invalid;
            }
        }
        //Contrast
        XMLN *p_Contrast = soap_get_child_node(p_ImagingSettings, "Contrast");
        if(p_Contrast != NULL)
        {
            if(p_Contrast->data != NULL)
            {
                pstSetImagingSettingsReq->stImageSettings.contrast = atof(p_Contrast->data);
            }
            else
            {
                ONVIF_ERR();
                goto XML_invalid;
            }
        }
        //IrCutFilter
        XMLN *p_IrCutFilter = soap_get_child_node(p_ImagingSettings, "IrCutFilter");
        if(p_IrCutFilter != NULL)
        {
            if(p_IrCutFilter->data != NULL)
            {
                if(strcmp(p_IrCutFilter->data, "ON") == 0)
                    pstSetImagingSettingsReq->stImageSettings.enIrCutFilter = IrCutFilterMode_ON;
                else if(strcmp(p_IrCutFilter->data, "OFF") == 0)
                    pstSetImagingSettingsReq->stImageSettings.enIrCutFilter = IrCutFilterMode_OFF;
                else if(strcmp(p_IrCutFilter->data, "AUTO") == 0)
                    pstSetImagingSettingsReq->stImageSettings.enIrCutFilter = IrCutFilterMode_AUTO;
                else
                {
                    ONVIF_ERR();
                    goto XML_invalid;
                }
            }
            else
            {
                ONVIF_ERR();
                goto XML_invalid;
            }
        }
        //Sharpness
        XMLN *p_Sharpness = soap_get_child_node(p_ImagingSettings, "Sharpness");
        if(p_Sharpness != NULL)
        {
            if(p_Sharpness->data != NULL)
            {
                pstSetImagingSettingsReq->stImageSettings.sharpness = atof(p_Sharpness->data);
            }
            else
            {
                ONVIF_ERR();
                goto XML_invalid;
            }
        }
        //Exposure
        XMLN *p_Exposure = soap_get_child_node(p_ImagingSettings, "Exposure");
        if(p_Exposure != NULL)
        {
            //Mode
            XMLN *p_Exposure_Mode = soap_get_child_node(p_Exposure, "Mode");
            if((p_Exposure_Mode != NULL) && (p_Exposure_Mode->data != NULL))
            {
                if(strcmp(p_Exposure_Mode->data, "AUTO") == 0)
                {
                    pstSetImagingSettingsReq->stImageSettings.stExposure.enMode = ExposureMode_AUTO;
                    //MinExposureTime
                    XMLN *p_Exposure_MinExposureTime = soap_get_child_node(p_Exposure, "MinExposureTime");
                    if(p_Exposure_MinExposureTime != NULL)
                    {
                        if(p_Exposure_MinExposureTime->data != NULL)
                        {
                            pstSetImagingSettingsReq->stImageSettings.stExposure.minExposureTime = atof(p_Exposure_MinExposureTime->data);
                        }
                        else
                        {
                            ONVIF_ERR();
                            goto XML_invalid;
                        }
                    }
                    //MaxExposureTime
                    XMLN *p_Exposure_MaxExposureTime = soap_get_child_node(p_Exposure, "MaxExposureTime");
                    if(p_Exposure_MaxExposureTime != NULL)
                    {
                        if(p_Exposure_MaxExposureTime->data != NULL)
                        {
                            pstSetImagingSettingsReq->stImageSettings.stExposure.maxExposureTime = atof(p_Exposure_MaxExposureTime->data);
                        }
                        else
                        {
                            ONVIF_ERR();
                            goto XML_invalid;
                        }
                    }
                    //MinGain
                    XMLN *p_Exposure_MinGain = soap_get_child_node(p_Exposure, "MinGain");
                    if(p_Exposure_MinGain != NULL)
                    {
                        if(p_Exposure_MinGain->data != NULL)
                        {
                            pstSetImagingSettingsReq->stImageSettings.stExposure.minGain = atof(p_Exposure_MinGain->data);
                        }
                        else
                        {
                            ONVIF_ERR();
                            goto XML_invalid;
                        }
                    }
                    //MaxGain
                    XMLN *p_Exposure_MaxGain = soap_get_child_node(p_Exposure, "MaxGain");
                    if(p_Exposure_MaxGain != NULL)
                    {
                        if(p_Exposure_MaxGain->data != NULL)
                        {
                            pstSetImagingSettingsReq->stImageSettings.stExposure.maxGain = atof(p_Exposure_MaxGain->data);
                        }
                        else
                        {
                            ONVIF_ERR();
                            goto XML_invalid;
                        }
                    }
                }
                else if(strcmp(p_Exposure_Mode->data, "MANUAL") == 0)
                {
                    pstSetImagingSettingsReq->stImageSettings.stExposure.enMode = ExposureMode_MANUAL;
                    //ExposureTime
                    XMLN *p_Exposure_ExposureTime = soap_get_child_node(p_Exposure, "ExposureTime");
                    if(p_Exposure_ExposureTime != NULL)
                    {
                        if(p_Exposure_ExposureTime->data != NULL)
                        {
                            pstSetImagingSettingsReq->stImageSettings.stExposure.exposureTime = atof(p_Exposure_ExposureTime->data);
                        }
                        else
                        {
                            ONVIF_ERR();
                            goto XML_invalid;
                        }
                    }
                    //Gain
                    XMLN *p_Exposure_Gain = soap_get_child_node(p_Exposure, "Gain");
                    if(p_Exposure_Gain != NULL)
                    {
                        if(p_Exposure_Gain->data != NULL)
                        {
                            pstSetImagingSettingsReq->stImageSettings.stExposure.gain = atof(p_Exposure_Gain->data);
                        }
                        else
                        {
                            ONVIF_ERR();
                            goto XML_invalid;
                        }
                    }
                }
                else
                {
                    ONVIF_ERR();
                    goto XML_invalid;
                }
            }
            else
            {
                ONVIF_ERR();
                goto XML_invalid;
            }
        }
        //Focus
        XMLN *p_Focus = soap_get_child_node(p_ImagingSettings, "Focus");
        if(p_Focus != NULL)
        {
            //Mode
            XMLN *p_Focus_AutoFocusMode = soap_get_child_node(p_Focus, "AutoFocusMode");
            if((p_Focus_AutoFocusMode != NULL) && (p_Focus_AutoFocusMode->data != NULL))
            {
                if(strcmp(p_Focus_AutoFocusMode->data, "AUTO") ==0)
                {
                    pstSetImagingSettingsReq->stImageSettings.stFocus.enAutoFocusMode = AutoFocusMode_AUTO;
                    //NearLimit
                    XMLN *p_Focus_NearLimit = soap_get_child_node(p_Focus, "NearLimit");
                    if(p_Focus_NearLimit != NULL)
                    {
                        if(p_Focus_NearLimit->data != NULL)
                        {
                            pstSetImagingSettingsReq->stImageSettings.stFocus.nearLimit = atof(p_Focus_NearLimit->data);
                        }
                        else
                        {
                            ONVIF_ERR();
                            goto XML_invalid;
                        }
                    }
                    //FarLimit
                    XMLN *p_Focus_FarLimit = soap_get_child_node(p_Focus, "FarLimit");
                    if(p_Focus_FarLimit != NULL)
                    {
                        if(p_Focus_FarLimit->data != NULL)
                        {
                            pstSetImagingSettingsReq->stImageSettings.stFocus.farLimit = atof(p_Focus_FarLimit->data);
                        }
                        else
                        {
                            ONVIF_ERR();
                            goto XML_invalid;
                        }
                    }
                }
                else if(strcmp(p_Focus_AutoFocusMode->data, "MANUAL") ==0)
                {
                    pstSetImagingSettingsReq->stImageSettings.stFocus.enAutoFocusMode = AutoFocusMode_MANUAL;
                    //DefaultSpeed
                    XMLN *p_Focus_DefaultSpeed = soap_get_child_node(p_Focus, "DefaultSpeed");
                    if(p_Focus_DefaultSpeed != NULL)
                    {
                        if(p_Focus_DefaultSpeed->data != NULL)
                        {
                            pstSetImagingSettingsReq->stImageSettings.stFocus.defaultSpeed = atof(p_Focus_DefaultSpeed->data);
                        }
                        else
                        {
                            ONVIF_ERR();
                            goto XML_invalid;
                        }
                    }
                }
                else
                {
                    ONVIF_ERR();
                    goto XML_invalid;
                }
            }
            else
            {
                ONVIF_ERR();
                goto XML_invalid;
            }
        }
        //WideDynamicRange
        XMLN *p_WideDynamicRange = soap_get_child_node(p_ImagingSettings, "WideDynamicRange");
        if(p_WideDynamicRange != NULL)
        {
            //Mode
            XMLN *p_WideDynamicRange_Mode = soap_get_child_node(p_WideDynamicRange, "Mode");
            if((p_WideDynamicRange_Mode != NULL) && (p_WideDynamicRange_Mode->data != NULL))
            {
                if(strcmp(p_WideDynamicRange_Mode->data, "ON") ==0)
                {
                    pstSetImagingSettingsReq->stImageSettings.stWideDynamicRange.enMode = WideDynamicMode_ON;
                    //Level
                    XMLN *p_WideDynamicRange_Level = soap_get_child_node(p_WideDynamicRange, "Level");
                    if(p_WideDynamicRange_Level != NULL)
                    {
                        if(p_WideDynamicRange_Level->data != NULL)
                        {
                            pstSetImagingSettingsReq->stImageSettings.stWideDynamicRange.level = atof(p_WideDynamicRange_Level->data);
                        }
                        else
                        {
                            ONVIF_ERR();
                            goto XML_invalid;
                        }
                    }
                }
                else if(strcmp(p_WideDynamicRange_Mode->data, "OFF") ==0)
                {
                    pstSetImagingSettingsReq->stImageSettings.stWideDynamicRange.enMode = WideDynamicMode_OFF;

                }
                else
                {
                    ONVIF_ERR();
                    goto XML_invalid;
                }
            }
            else
            {
                ONVIF_ERR();
                goto XML_invalid;
            }
        }
        //WhiteBalance
        XMLN *p_WhiteBalance = soap_get_child_node(p_ImagingSettings, "WhiteBalance");
        if(p_WhiteBalance != NULL)
        {
            XMLN *p_WhiteBalance_Mode = soap_get_child_node(p_WhiteBalance, "Mode");
            if((p_WhiteBalance_Mode != NULL) && (p_WhiteBalance_Mode->data != NULL))
            {
                if(strcmp(p_WhiteBalance_Mode->data, "AUTO") ==0)
                {
                    pstSetImagingSettingsReq->stImageSettings.stWhiteBalance.enMode = WhiteBalanceMode_AUTO;
                }
                else if(strcmp(p_WhiteBalance_Mode->data, "MANUAL") ==0)
                {
                    pstSetImagingSettingsReq->stImageSettings.stWhiteBalance.enMode = WhiteBalanceMode_MANUAL;
                    //CrGain
                    XMLN *p_WhiteBalance_CrGain = soap_get_child_node(p_WhiteBalance, "CrGain");
                    if(p_WhiteBalance_CrGain != NULL)
                    {
                        if(p_WhiteBalance_CrGain->data != NULL)
                        {
                            pstSetImagingSettingsReq->stImageSettings.stWhiteBalance.crGain = atof(p_WhiteBalance_CrGain->data);
                        }
                        else
                        {
                            ONVIF_ERR();
                            goto XML_invalid;
                        }
                    }
                    //CbGain
                    XMLN *p_WhiteBalance_CbGain = soap_get_child_node(p_WhiteBalance, "CbGain");
                    if(p_WhiteBalance_CbGain != NULL)
                    {
                        if(p_WhiteBalance_CbGain->data != NULL)
                        {
                            pstSetImagingSettingsReq->stImageSettings.stWhiteBalance.cbGain = atof(p_WhiteBalance_CbGain->data);
                        }
                        else
                        {
                            ONVIF_ERR();
                            goto XML_invalid;
                        }
                    }
                }
                else
                {
                    ONVIF_ERR();
                    goto XML_invalid;
                }
            }
            else
            {
                ONVIF_ERR();
                goto XML_invalid;
            }
        }
    }
    else
    {
        ONVIF_ERR();
        goto XML_invalid;
    }
    //ForcePersistence
    XMLN *p_ForcePersistence = soap_get_child_node(p_SetImagingSettings, "ForcePersistence");
    if(p_ForcePersistence != NULL)
    {
        if(p_ForcePersistence->data != NULL)
        {
            if(strcmp(p_ForcePersistence->data, "false") == 0)
                pstSetImagingSettingsReq->enForcePersistence = Boolean_FALSE;
            else if(strcmp(p_ForcePersistence->data, "true") == 0)
                pstSetImagingSettingsReq->enForcePersistence = Boolean_TRUE;
            else
            {
                ONVIF_ERR();
                goto XML_invalid;
            }
        }
        else
        {
            ONVIF_ERR();
            goto XML_invalid;
        }
    }
    /*
	ONVIF_DBG("%s,%f,%f,%f,%d,%f,%d,%f,%d,%f,%f,%f,%f,%f,%f,%f,%d,%f,%f,%f,%f,%d,%d,%f,%d\n",
        pstSetImagingSettingsReq->aszVideoSourceToken,
        pstSetImagingSettingsReq->stImageSettings.brightness,
        pstSetImagingSettingsReq->stImageSettings.colorSaturation,
        pstSetImagingSettingsReq->stImageSettings.contrast,
        pstSetImagingSettingsReq->stImageSettings.enIrCutFilter,
        pstSetImagingSettingsReq->stImageSettings.sharpness,
        pstSetImagingSettingsReq->stImageSettings.stBacklightCompensation.enMode,
        pstSetImagingSettingsReq->stImageSettings.stBacklightCompensation.level,
        pstSetImagingSettingsReq->stImageSettings.stExposure.enMode,
        pstSetImagingSettingsReq->stImageSettings.stExposure.exposureTime,
        pstSetImagingSettingsReq->stImageSettings.stExposure.gain,
        pstSetImagingSettingsReq->stImageSettings.stExposure.maxExposureTime,
        pstSetImagingSettingsReq->stImageSettings.stExposure.maxGain,
        pstSetImagingSettingsReq->stImageSettings.stExposure.minExposureTime,
        pstSetImagingSettingsReq->stImageSettings.stExposure.minGain,
        pstSetImagingSettingsReq->stImageSettings.stFocus.defaultSpeed,
        pstSetImagingSettingsReq->stImageSettings.stFocus.enAutoFocusMode,
        pstSetImagingSettingsReq->stImageSettings.stFocus.farLimit,
        pstSetImagingSettingsReq->stImageSettings.stFocus.nearLimit,
        pstSetImagingSettingsReq->stImageSettings.stWhiteBalance.cbGain,
        pstSetImagingSettingsReq->stImageSettings.stWhiteBalance.crGain,
        pstSetImagingSettingsReq->stImageSettings.stWhiteBalance.enMode,
        pstSetImagingSettingsReq->stImageSettings.stWideDynamicRange.enMode,
        pstSetImagingSettingsReq->stImageSettings.stWideDynamicRange.level,
        pstSetImagingSettingsReq->enForcePersistence);
        */
    return ONVIF_OK;

XML_invalid:
    return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;

}

ONVIF_RET soap_parse_request_GetOptions(XMLN *p_body, GONVIF_IMAGE_GetOptions_Token_S *pstGetImageOptionsReq)
{
    XMLN *p_GetOptions = soap_get_child_node(p_body, "GetOptions");
    if(p_GetOptions != NULL)
    {
        XMLN *p_VideoSourceToken = soap_get_child_node(p_GetOptions, "VideoSourceToken");
        if ((p_VideoSourceToken != NULL) && (p_VideoSourceToken->data != NULL))
        {
            strcpy(pstGetImageOptionsReq->aszVideoSourceToken, p_VideoSourceToken->data);
        }
        else
        {
            return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
        }
    }
    else
    {
        return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
    }
    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_GetMoveOptions(XMLN *p_body, GONVIF_IMAGE_GetMoveOptions_Token_S *pstGetMoveOptionsReq)
{
    XMLN *p_GetMoveOptions = soap_get_child_node(p_body, "GetMoveOptions");
    if(p_GetMoveOptions != NULL)
    {
        XMLN *p_VideoSourceToken = soap_get_child_node(p_GetMoveOptions, "VideoSourceToken");
        if ((p_VideoSourceToken != NULL) && (p_VideoSourceToken->data != NULL))
        {
            strcpy(pstGetMoveOptionsReq->aszVideoSourceToken, p_VideoSourceToken->data);
        }
        else
        {
            return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
        }
    }
    else
    {
        return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
    }

    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_Move(XMLN *p_body, GONVIF_IMAGE_Move_S *pstMoveReq)
{
    XMLN *p_Move = soap_get_child_node(p_body, "Move");
    if(p_Move != NULL)
    {
        //toke
        XMLN *p_VideoSourceToken = soap_get_child_node(p_Move, "VideoSourceToken");
        if ((p_VideoSourceToken != NULL) || (p_VideoSourceToken->data != NULL))
        {
            strcpy(pstMoveReq->aszVideoSourceToken, p_VideoSourceToken->data);
        }
        else
        {
            ONVIF_ERR();
            goto XML_invalid;
        }
        //Move
        XMLN *p_Focus = soap_get_child_node(p_Move, "Focus");
        if (p_Focus != NULL)
        {
            XMLN *p_MoveMode = soap_get_child_node(p_Focus, "Absolute");
            if(p_MoveMode != NULL)
            {
                //Absolute
                pstMoveReq->stFocus.stAbsolute = (GONVIF_IMAGE_Focus_Absolute_S *)malloc(sizeof(GONVIF_IMAGE_Focus_Absolute_S));
                memset(pstMoveReq->stFocus.stAbsolute, 0, sizeof(GONVIF_IMAGE_Focus_Absolute_S));
                pstMoveReq->stFocus.stAbsolute->position = NULL;
                pstMoveReq->stFocus.stAbsolute->speed = NULL;
                XMLN *p_Absolute_Position = soap_get_child_node(p_MoveMode, "Position");
                if((p_Absolute_Position != NULL) && (p_Absolute_Position->data != NULL))
                {
                    pstMoveReq->stFocus.stAbsolute->position = (GK_FLOAT *)malloc(sizeof(GK_FLOAT));
                    memset(pstMoveReq->stFocus.stAbsolute->position, 0, sizeof(GK_FLOAT));
                    *(pstMoveReq->stFocus.stAbsolute->position) = atof(p_Absolute_Position->data);
                }
                else
                {
                    ONVIF_ERR();
                    goto XML_invalid;
                }
                XMLN *p_Absolute_Speed = soap_get_child_node(p_MoveMode, "Speed");
                if(p_Absolute_Speed != NULL)
                {

                    if(p_Absolute_Speed->data != NULL)
                    {
                        pstMoveReq->stFocus.stAbsolute->speed = (GK_FLOAT *)malloc(sizeof(GK_FLOAT));
                        memset(pstMoveReq->stFocus.stAbsolute->speed, 0, sizeof(GK_FLOAT));
                        *(pstMoveReq->stFocus.stAbsolute->speed) = atof(p_Absolute_Speed->data);
                    }
                    else
                    {
                        ONVIF_ERR();
                        goto XML_invalid;
                    }
                }
            }
            else
            {
                p_MoveMode = soap_get_child_node(p_Focus, "Relative");
                if(p_MoveMode != NULL)
                {
                    //Relative
                    pstMoveReq->stFocus.stRelative = (GONVIF_IMAGE_Focus_Relative_S *)malloc(sizeof(GONVIF_IMAGE_Focus_Relative_S));
                    memset(pstMoveReq->stFocus.stRelative, 0, sizeof(GONVIF_IMAGE_Focus_Relative_S));
                    pstMoveReq->stFocus.stRelative->distance = NULL;
                    pstMoveReq->stFocus.stRelative->speed = NULL;
                    XMLN *p_Relative_Distance = soap_get_child_node(p_MoveMode, "Distance");
                    if((p_Relative_Distance != NULL) && (p_Relative_Distance->data != NULL))
                    {
                        pstMoveReq->stFocus.stRelative->distance = (GK_FLOAT *)malloc(sizeof(GK_FLOAT));
                        memset(pstMoveReq->stFocus.stRelative->distance, 0, sizeof(GK_FLOAT));
                        *(pstMoveReq->stFocus.stRelative->distance) = atof(p_Relative_Distance->data);
                    }
                    else
                    {
                        ONVIF_ERR();
                        goto XML_invalid;
                    }
                    XMLN *p_Relative_Speed = soap_get_child_node(p_MoveMode, "Speed");
                    if(p_Relative_Speed != NULL)
                    {
                        if(p_Relative_Speed->data != NULL)
                        {
                            pstMoveReq->stFocus.stRelative->speed = (GK_FLOAT *)malloc(sizeof(GK_FLOAT));
                            memset(pstMoveReq->stFocus.stRelative->speed, 0, sizeof(GK_FLOAT));
                            *(pstMoveReq->stFocus.stRelative->speed) = atof(p_Relative_Speed->data);
                        }
                        else
                        {
                            ONVIF_ERR();
                            goto XML_invalid;
                        }
                    }
                }
                else
                {
                    //Continuous
                    p_MoveMode = soap_get_child_node(p_Focus, "Continuous");
                    if(p_MoveMode != NULL)
                    {
                        pstMoveReq->stFocus.stContinuous = (GONVIF_IMAGE_Focus_Continuous_S *)malloc(sizeof(GONVIF_IMAGE_Focus_Continuous_S));
                        memset(pstMoveReq->stFocus.stContinuous, 0, sizeof(GONVIF_IMAGE_Focus_Continuous_S));
                        pstMoveReq->stFocus.stContinuous->speed = NULL;
                        XMLN *p_Continuous_Speed = soap_get_child_node(p_MoveMode, "Speed");
                        if((p_Continuous_Speed != NULL) && (p_Continuous_Speed->data != NULL))
                        {
                            pstMoveReq->stFocus.stContinuous->speed = (GK_FLOAT *)malloc(sizeof(GK_FLOAT));
                            memset(pstMoveReq->stFocus.stContinuous->speed, 0, sizeof(GK_FLOAT));
                            *(pstMoveReq->stFocus.stContinuous->speed) = atof(p_Continuous_Speed->data);
                        }
                        else
                        {
                            ONVIF_ERR();
                            goto XML_invalid;
                        }
                    }
                    else
                    {
                        ONVIF_ERR();
                        goto XML_invalid;
                    }
                }
            }
        }
        else
        {
            ONVIF_ERR();
            goto XML_invalid;
        }
    }
    else
    {
        ONVIF_ERR();
        goto XML_invalid;
    }

    return ONVIF_OK;

XML_invalid:
    return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
}

ONVIF_RET soap_parse_request_GetStatus(XMLN *p_body, GONVIF_IMAGE_GetStatus_Token_S *pstImageGetStatusReq)
{
    XMLN *p_GetStatus = soap_get_child_node(p_body, "GetStatus");
    if(p_GetStatus != NULL)
    {
        XMLN *p_VideoSourceToken = soap_get_child_node(p_GetStatus, "VideoSourceToken");
        if ((p_VideoSourceToken != NULL) && (p_VideoSourceToken->data != NULL))
        {
            strcpy(pstImageGetStatusReq->aszVideoSourceToken, p_VideoSourceToken->data);
        }
        else
        {
            return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
        }
    }
    else
    {
        return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
    }
    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_ImageStop(XMLN *p_body, GONVIF_IMAGE_Stop_Token_S *pstStopReq)
{
    XMLN *p_Stop = soap_get_child_node(p_body, "Stop");
    if(p_Stop != NULL)
    {
        XMLN *p_VideoSourceToken = soap_get_child_node(p_Stop, "VideoSourceToken");
        if ((p_VideoSourceToken != NULL) && (p_VideoSourceToken->data != NULL))
        {
            strcpy(pstStopReq->aszVideoSourceToken, p_VideoSourceToken->data);
        }
        else
        {
            return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
        }
    }
    else
    {
        return ONVIF_ERR_IMAGE_SENDER_XML_INVALID;
    }
    return ONVIF_OK;
}

/*********************************** ptz ****************************************/

ONVIF_RET soap_parse_request_GetNodes(XMLN *p_body)
{
    XMLN *p_GetNodes = soap_get_child_node(p_body, "GetNodes");
    if(p_GetNodes == NULL)
    {
		return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
    }
    
    return ONVIF_OK;
}


ONVIF_RET soap_parse_request_GetNode(XMLN *p_body, GONVIF_PTZ_GetNode_S *pstPTZ_GetNode)
{
    XMLN *p_GetNode = soap_get_child_node(p_body, "GetNode");
    if(p_GetNode != NULL)
    {
        XMLN *p_NodeToken = soap_get_child_node(p_GetNode, "NodeToken");
        if ((p_NodeToken != NULL) && (p_NodeToken->data != NULL))
        {
            strncpy(pstPTZ_GetNode->aszNodeToken, p_NodeToken->data, MAX_TOKEN_LENGTH-1);
        }
        else
        {
            return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
        }
    }
    else
    {
        return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
    }
    
    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_GetConfiguration(XMLN *p_body, GONVIF_PTZ_GetConfiguration_S *pstPTZ_GetConfiguration)
{
    XMLN *p_GetConfiguration = soap_get_child_node(p_body, "GetConfiguration");
    if(p_GetConfiguration != NULL)
    {
        XMLN *p_PTZConfigurationToken = soap_get_child_node(p_GetConfiguration, "PTZConfigurationToken");
        if ((p_PTZConfigurationToken != NULL) && (p_PTZConfigurationToken->data != NULL))
        {
            strncpy(pstPTZ_GetConfiguration->aszPTZConfiguationToken, p_PTZConfigurationToken->data, MAX_TOKEN_LENGTH-1);
        }
        else
        {
            return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
        }
    }
    else
    {
        return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
    }
    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_GetConfigurationOptions(XMLN *p_body, GONVIF_PTZ_GetConfigurationOptions_S *pstPTZ_GetConfigurationOptions)
{

    XMLN *p_GetConfigurationOptions = soap_get_child_node(p_body, "GetConfigurationOptions");
    if(p_GetConfigurationOptions != NULL)
    {
        XMLN *p_ConfigurationToken = soap_get_child_node(p_GetConfigurationOptions, "ConfigurationToken");
        if ((p_ConfigurationToken != NULL) && (p_ConfigurationToken->data != NULL))
        {
            strncpy(pstPTZ_GetConfigurationOptions->aszConfigurationToken, p_ConfigurationToken->data, MAX_TOKEN_LENGTH-1);
        }
        else
        {
            return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
        }
    }
    else
    {
        return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
    }
    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_SetConfiguration(XMLN *p_body, GONVIF_PTZ_SetConfiguration_S *pstPTZ_SetConfiguration)
{

    XMLN *p_SetConfiguration = soap_get_child_node(p_body, "SetConfiguration");
    if(p_SetConfiguration != NULL)
    {
        XMLN *p_PTZConfiguration = soap_get_child_node(p_SetConfiguration, "PTZConfiguration");
        if(p_PTZConfiguration != NULL)
        {
            const GK_CHAR *token = xml_attr_get_data(p_PTZConfiguration, "token");
            if(token != NULL)
            {
                strncpy(pstPTZ_SetConfiguration->stPTZConfiguration.token, token, MAX_TOKEN_LENGTH-1);
            }

            XMLN *p_Name = soap_get_child_node(p_PTZConfiguration, "Name");
            if (p_Name != NULL)
            {
                if(p_Name->data != NULL)
                {
                    strncpy(pstPTZ_SetConfiguration->stPTZConfiguration.name, p_Name->data, MAX_NAME_LENGTH-1);
                }
            }

            XMLN *p_UseCount = soap_get_child_node(p_PTZConfiguration, "UseCount");
            if (p_UseCount != NULL)
            {
                if(p_UseCount->data != NULL)
                {
                    pstPTZ_SetConfiguration->stPTZConfiguration.useCount = atoi(p_UseCount->data);
                }
            }

            XMLN *p_NodeToken = soap_get_child_node(p_PTZConfiguration, "NodeToken");
            if (p_NodeToken != NULL)
            {
                if(p_NodeToken->data != NULL)
                {
                    strncpy(pstPTZ_SetConfiguration->stPTZConfiguration.nodeToken, p_NodeToken->data, MAX_TOKEN_LENGTH-1);
                }
            }

            //DefaultAbsolute
            XMLN *p_DefaultAbsolutePantTiltPositionSpace = soap_get_child_node(p_PTZConfiguration, "DefaultAbsolutePantTiltPositionSpace");
            if (p_DefaultAbsolutePantTiltPositionSpace != NULL)
            {
                if(p_DefaultAbsolutePantTiltPositionSpace->data != NULL)
                {
                    strncpy(pstPTZ_SetConfiguration->stPTZConfiguration.defaultAbsolutePantTiltPositionSpace, 
                        p_DefaultAbsolutePantTiltPositionSpace->data, MAX_URI_LENGTH-1);
                }
            }
            XMLN *p_DefaultAbsoluteZoomPositionSpace = soap_get_child_node(p_PTZConfiguration, "DefaultAbsoluteZoomPositionSpace");
            if (p_DefaultAbsoluteZoomPositionSpace != NULL)
            {
                if(p_DefaultAbsoluteZoomPositionSpace->data != NULL)
                {
                    strncpy(pstPTZ_SetConfiguration->stPTZConfiguration.defaultAbsoluteZoomPositionSpace, 
                        p_DefaultAbsoluteZoomPositionSpace->data, MAX_URI_LENGTH-1);
                }
            }
            //DefaultRelative
            XMLN *p_DefaultRelativePanTiltTranslationSpace = soap_get_child_node(p_PTZConfiguration, "DefaultRelativePanTiltTranslationSpace");
            if (p_DefaultRelativePanTiltTranslationSpace != NULL)
            {
                if(p_DefaultRelativePanTiltTranslationSpace->data != NULL)
                {
                    strncpy(pstPTZ_SetConfiguration->stPTZConfiguration.defaultRelativePanTiltTranslationSpace,
                        p_DefaultRelativePanTiltTranslationSpace->data, MAX_URI_LENGTH-1);
                }
            }

            XMLN *p_DefaultRelativeZoomTranslationSpace = soap_get_child_node(p_PTZConfiguration, "DefaultRelativeZoomTranslationSpace");
            if (p_DefaultRelativeZoomTranslationSpace != NULL)
            {
                if(p_DefaultRelativeZoomTranslationSpace->data != NULL)
                {
                    strncpy(pstPTZ_SetConfiguration->stPTZConfiguration.defaultRelativeZoomTranslationSpace,
                        p_DefaultRelativeZoomTranslationSpace->data, MAX_URI_LENGTH-1);
                }
            }
            //DefaultContinuous
            XMLN *p_DefaultContinuousPanTiltVelocitySpace = soap_get_child_node(p_PTZConfiguration, "DefaultContinuousPanTiltVelocitySpace");
            if (p_DefaultContinuousPanTiltVelocitySpace != NULL)
            {
                if(p_DefaultContinuousPanTiltVelocitySpace->data != NULL)
                {
                    strncpy(pstPTZ_SetConfiguration->stPTZConfiguration.defaultContinuousPanTiltVelocitySpace, 
                        p_DefaultContinuousPanTiltVelocitySpace->data, MAX_URI_LENGTH-1);
                }
            }

            XMLN *p_DefaultContinuousZoomVelocitySpace = soap_get_child_node(p_PTZConfiguration, "DefaultContinuousZoomVelocitySpace");
            if (p_DefaultContinuousZoomVelocitySpace != NULL)
            {
                if(p_DefaultContinuousZoomVelocitySpace->data != NULL)
                {
                    strncpy(pstPTZ_SetConfiguration->stPTZConfiguration.defaultContinuousZoomVelocitySpace,
                        p_DefaultContinuousZoomVelocitySpace->data, MAX_URI_LENGTH-1);
                }
            }

            //DefaultPTZSpeed
            XMLN *p_DefaultPTZSpeed = soap_get_child_node(p_PTZConfiguration, "DefaultPTZSpeed");
            if (p_DefaultPTZSpeed != NULL)
            {
                XMLN *p_PanTilt = soap_get_child_node(p_DefaultPTZSpeed, "PanTilt");
                if (p_PanTilt != NULL)
                {
                    const GK_CHAR *x = xml_attr_get_data(p_PanTilt, "x");
                    if (x != NULL)
                    {
                        pstPTZ_SetConfiguration->stPTZConfiguration.stDefaultPTZSpeed.stPanTilt.x = atof(x);
                    }
                    const GK_CHAR *y = xml_attr_get_data(p_PanTilt, "y");
                    if (y != NULL)
                    {
                        pstPTZ_SetConfiguration->stPTZConfiguration.stDefaultPTZSpeed.stPanTilt.y = atof(y);
                    }
                    const GK_CHAR *space = xml_attr_get_data(p_PanTilt, "space");
                    if (space != NULL)
                    {
                        strncpy(pstPTZ_SetConfiguration->stPTZConfiguration.stDefaultPTZSpeed.stPanTilt.aszSpace, space, MAX_SPACE_LENGTH-1);
                    }
                }

                XMLN *p_Zoom = soap_get_child_node(p_DefaultPTZSpeed, "Zoom");
                if (p_Zoom != NULL)
                {
                    const GK_CHAR *x = xml_attr_get_data(p_Zoom, "x");
                    if(x != NULL)
                    {
                        pstPTZ_SetConfiguration->stPTZConfiguration.stDefaultPTZSpeed.stZoom.x = atof(x);
                    }
                    const GK_CHAR *space = xml_attr_get_data(p_Zoom, "space");
                    if (space != NULL)
                    {
                        strncpy(pstPTZ_SetConfiguration->stPTZConfiguration.stDefaultPTZSpeed.stZoom.aszSpace, space, MAX_SPACE_LENGTH-1);
                    }
                }
            }

            //p_DefaultPTZTimeout
            XMLN *p_DefaultPTZTimeout = soap_get_child_node(p_PTZConfiguration, "DefaultPTZTimeout");
            if (p_DefaultPTZTimeout != NULL)
            {
                if(p_DefaultPTZTimeout->data != NULL)
                {
                    GK_U32 Length;
                    GK_U32 Hour;
                    GK_U32 Min;
                    GK_U32 Sec;
                    GK_CHAR Num[10] = {0};
                    GK_CHAR *SearchChr1;
                    GK_CHAR *SearchChr2;
                    SearchChr1 = strchr(p_DefaultPTZTimeout->data, 'T');
                    SearchChr2 = strchr(p_DefaultPTZTimeout->data, 'H');
                    if((SearchChr1 != NULL) && (SearchChr2 != NULL))
                    {
                        Length = SearchChr2 - SearchChr1;
                        strncpy(Num, SearchChr1+1, Length-1);
                        Num[9] = '\0';
                        Hour = atoi(Num);
                        SearchChr1 = strchr(p_DefaultPTZTimeout->data, 'M');
                        if(SearchChr1 != NULL)
                        {
                            Length = SearchChr1 - SearchChr2;
                            strncpy(Num, SearchChr2+1, Length-1);
                            Num[9] = '\0';
                            Min = atoi(Num);
                            SearchChr2 = strchr(p_DefaultPTZTimeout->data, 'S');
                            if(SearchChr2 != NULL)
                            {
                                Length = SearchChr2 - SearchChr1;
                                strncpy(Num, SearchChr1+1, Length-1);
                                Num[9] = '\0';
                                Sec = atoi(Num);
                                pstPTZ_SetConfiguration->stPTZConfiguration.defaultPTZTimeout = (GK_S64)(Hour*3600+Min*60+Sec);
                            }
                        }
                    }
                }
            }

            //PanTiltLimits
            XMLN *p_PanTiltLimits = soap_get_child_node(p_PTZConfiguration, "PanTiltLimits");
            if (p_PanTiltLimits != NULL)
            {
                XMLN *p_Range = soap_get_child_node(p_PanTiltLimits, "Range");
                if(p_Range != NULL)
                {
                    XMLN *p_URI = soap_get_child_node(p_Range, "URI");
                    if(p_URI != NULL)
                    {
                        if(p_URI->data != NULL)
                        {
                            strncpy(pstPTZ_SetConfiguration->stPTZConfiguration.stPanTiltLimits.stRange.aszURI, p_URI->data, MAX_URI_LENGTH-1);
                        }
                    }
                    XMLN *p_XRange = soap_get_child_node(p_Range, "XRange");
                    if(p_XRange != NULL)
                    {
                        XMLN *p_Min = soap_get_child_node(p_XRange, "Min");
                        XMLN *p_Max = soap_get_child_node(p_XRange, "Max");
                        if((p_Min != NULL) && (p_Min->data != NULL) && (p_Max != NULL) && (p_Max->data != NULL))
                        {
                            pstPTZ_SetConfiguration->stPTZConfiguration.stPanTiltLimits.stRange.stXRange.min = atof(p_Min->data);
                            pstPTZ_SetConfiguration->stPTZConfiguration.stPanTiltLimits.stRange.stXRange.max = atof(p_Max->data);
                        }
                    }

                    XMLN *p_YRange = soap_get_child_node(p_Range, "YRange");
                    if(p_YRange != NULL)
                    {
                        XMLN *p_Min = soap_get_child_node(p_YRange, "Min");
                        XMLN *p_Max = soap_get_child_node(p_YRange, "Max");
                        if((p_Min != NULL) && (p_Min->data != NULL) && (p_Max != NULL) && (p_Max->data != NULL))
                        {
                            pstPTZ_SetConfiguration->stPTZConfiguration.stPanTiltLimits.stRange.stYRange.min = atof(p_Min->data);
                            pstPTZ_SetConfiguration->stPTZConfiguration.stPanTiltLimits.stRange.stYRange.max = atof(p_Max->data);
                        }
                    }
                }
            }

            //ZoomLimits
            XMLN *p_ZoomLimits = soap_get_child_node(p_PTZConfiguration, "ZoomLimits");
            if (p_ZoomLimits != NULL)
            {
                XMLN *p_Range = soap_get_child_node(p_ZoomLimits, "Range");
                if(p_Range != NULL)
                {
                    XMLN *p_URI = soap_get_child_node(p_Range, "URI");
                    if(p_URI != NULL)
                    {
                        if(p_URI->data != NULL)
                        {
                            strncpy(pstPTZ_SetConfiguration->stPTZConfiguration.stZoomLimits.stRange.aszURI, p_URI->data, MAX_URI_LENGTH-1);
                        }
                    }

                    XMLN *p_XRange = soap_get_child_node(p_Range, "XRange");
                    if(p_XRange != NULL)
                    {
                        XMLN *p_Min = soap_get_child_node(p_XRange, "Min");
                        XMLN *p_Max = soap_get_child_node(p_XRange, "Max");
                        if((p_Min != NULL) && (p_Min->data != NULL) && (p_Max != NULL) && (p_Max->data != NULL))
                        {
                            pstPTZ_SetConfiguration->stPTZConfiguration.stZoomLimits.stRange.stXRange.min = atof(p_Min->data);
                            pstPTZ_SetConfiguration->stPTZConfiguration.stZoomLimits.stRange.stXRange.max = atof(p_Max->data);
                        }
                    }
                }
            }
        }
        //ForcePersistence
        XMLN *p_ForcePersistence = soap_get_child_node(p_SetConfiguration, "ForcePersistence");
        if(p_ForcePersistence != NULL)
        {
            if(p_ForcePersistence->data != NULL)
            {
                if(strcmp(p_ForcePersistence->data, "true") == 0)
                {
                    pstPTZ_SetConfiguration->enForcePersistence = Boolean_TRUE;
                }
                else if(strcmp(p_ForcePersistence->data, "false") == 0)
                {
                    pstPTZ_SetConfiguration->enForcePersistence = Boolean_FALSE;
                }
                else
                {
                    ONVIF_ERR();
                    goto XML_invalid;
                }
            }
            else
            {
                ONVIF_ERR();
                goto XML_invalid;
            }
        }
    }
    else
    {
        ONVIF_ERR();
        goto XML_invalid;
    }

    return ONVIF_OK;

XML_invalid:
    return ONVIF_ERR_PTZ_SENDER_XML_INVALID;

}

ONVIF_RET soap_parse_request_AbsoluteMove(XMLN *p_body, GONVIF_PTZ_AbsoluteMove_S *pstPTZ_AbsoluteMove)
{
    XMLN *p_AbsoluteMove = soap_get_child_node(p_body, "AbsoluteMove");
    if(p_AbsoluteMove != NULL)
    {
        //ProfileToken
        XMLN *p_ProfileToken = soap_get_child_node(p_AbsoluteMove, "ProfileToken");
        if((p_ProfileToken != NULL) && (p_ProfileToken->data != NULL))
        {
            strcpy(pstPTZ_AbsoluteMove->aszProfileToken, p_ProfileToken->data);
        }
        else
        {
            return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
        }
        //Position
        XMLN *p_Position = soap_get_child_node(p_AbsoluteMove, "Position");
        if (p_Position != NULL)
        {
            XMLN *p_Position_PanTilt = soap_get_child_node(p_Position, "PanTilt");
            if(p_Position_PanTilt != NULL)
            {
                const GK_CHAR *x = xml_attr_get_data(p_Position_PanTilt, "x");
                const GK_CHAR *y = xml_attr_get_data(p_Position_PanTilt, "y");
                if((x != NULL)  && (y != NULL))
                {
                    pstPTZ_AbsoluteMove->stPosition.stPanTilt.x = atof(x);
                    pstPTZ_AbsoluteMove->stPosition.stPanTilt.y = atof(y);
                }
                const GK_CHAR *space = xml_attr_get_data(p_Position_PanTilt, "space");
                if(space != NULL)
                {
                    strcpy(pstPTZ_AbsoluteMove->stPosition.stPanTilt.aszSpace, space);
                }
            }
            XMLN *p_Position_Zoom = soap_get_child_node(p_Position, "Zoom");
            if(p_Position_Zoom != NULL)
            {
                const GK_CHAR *x = xml_attr_get_data(p_Position_Zoom, "x");
                if(x != NULL)
                {
                    pstPTZ_AbsoluteMove->stPosition.stZoom.x = atof(x);
                }
                const GK_CHAR *space = xml_attr_get_data(p_Position_Zoom, "space");
                if(space != NULL)
                {
                    strcpy(pstPTZ_AbsoluteMove->stPosition.stZoom.aszSpace, space);
                }
            }

        }
        else
        {
            return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
        }
        //speed
        XMLN *p_Speed = soap_get_child_node(p_AbsoluteMove, "Speed");
        if(p_Speed != NULL)
        {
            XMLN *p_Speed_PanTilt = soap_get_child_node(p_Speed, "PanTilt");
            if(p_Speed_PanTilt != NULL)
            {
                const GK_CHAR *x = xml_attr_get_data(p_Speed_PanTilt, "x");
                const GK_CHAR *y = xml_attr_get_data(p_Speed_PanTilt, "y");
                if((x != NULL)  && (y != NULL))
                {
                    pstPTZ_AbsoluteMove->stSpeed.stPanTilt.x = atof(x);
                    pstPTZ_AbsoluteMove->stSpeed.stPanTilt.y = atof(y);
                }
                const GK_CHAR *space = xml_attr_get_data(p_Speed_PanTilt, "space");
                if(space != NULL)
                {
                    strcpy(pstPTZ_AbsoluteMove->stSpeed.stPanTilt.aszSpace, space);
                }
            }
            XMLN *p_Speed_Zoom = soap_get_child_node(p_Speed, "Zoom");
            if(p_Speed_Zoom != NULL)
            {
                const GK_CHAR *x = xml_attr_get_data(p_Speed_Zoom, "x");
                if(x != NULL)
                {
                    pstPTZ_AbsoluteMove->stSpeed.stZoom.x = atof(x);
                }
                const GK_CHAR *space = xml_attr_get_data(p_Speed_Zoom, "space");
                if(space != NULL)
                {
                    strcpy(pstPTZ_AbsoluteMove->stSpeed.stZoom.aszSpace, space);
                }
            }
        }
    }
    else
    {
        return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
    }
    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_RelativeMove(XMLN *p_body, GONVIF_PTZ_RelativeMove_S *pstPTZ_RelativeMove)
{
    XMLN *p_RelativeMove = soap_get_child_node(p_body, "RelativeMove");
    if(p_RelativeMove != NULL)
    {
        //ProfileToken
        XMLN *p_ProfileToken = soap_get_child_node(p_RelativeMove, "ProfileToken");
        if((p_ProfileToken != NULL) && (p_ProfileToken->data != NULL))
        {
            strcpy(pstPTZ_RelativeMove->aszProfileToken, p_ProfileToken->data);
        }
        else
        {
            return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
        }
        //Translation
        XMLN *p_Translation = soap_get_child_node(p_RelativeMove, "Translation");
        if (p_Translation != NULL)
        {
            XMLN *p_Translation_PanTilt = soap_get_child_node(p_Translation, "PanTilt");
            if(p_Translation_PanTilt != NULL)
            {
                const GK_CHAR *x = xml_attr_get_data(p_Translation_PanTilt, "x");
                const GK_CHAR *y = xml_attr_get_data(p_Translation_PanTilt, "y");
                if((x != NULL)  && (y != NULL))
                {
                    pstPTZ_RelativeMove->stTranslation.stPanTilt.x = atof(x);
                    pstPTZ_RelativeMove->stTranslation.stPanTilt.y = atof(y);
                }
                else
                {
                    return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
                }
                const GK_CHAR *space = xml_attr_get_data(p_Translation_PanTilt, "space");
                if(space != NULL)
                {
                    strcpy(pstPTZ_RelativeMove->stTranslation.stPanTilt.aszSpace, space);
                }
            }
            XMLN *p_Translation_Zoom = soap_get_child_node(p_Translation, "Zoom");
            if(p_Translation_Zoom != NULL)
            {
                const GK_CHAR *x = xml_attr_get_data(p_Translation_Zoom, "x");
                if(x != NULL)
                {
                    pstPTZ_RelativeMove->stTranslation.stZoom.x = atof(x);
                }
                else
                {
                    return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
                }
                const GK_CHAR *space = xml_attr_get_data(p_Translation_Zoom, "space");
                if(space != NULL)
                {
                    strcpy(pstPTZ_RelativeMove->stTranslation.stZoom.aszSpace, space);
                }
            }

        }
        else
        {
            return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
        }
        //speed
        XMLN *p_Speed = soap_get_child_node(p_RelativeMove, "Speed");
        if(p_Speed != NULL)
        {
            XMLN *p_Speed_PanTilt = soap_get_child_node(p_Speed, "PanTilt");
            if(p_Speed_PanTilt != NULL)
            {
                const GK_CHAR *x = xml_attr_get_data(p_Speed_PanTilt, "x");
                const GK_CHAR *y = xml_attr_get_data(p_Speed_PanTilt, "y");
                if((x != NULL)  && (y != NULL))
                {
                    pstPTZ_RelativeMove->stSpeed.stPanTilt.x = atof(x);
                    pstPTZ_RelativeMove->stSpeed.stPanTilt.y = atof(y);
                }
                else
                {
                    return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
                }
                const GK_CHAR *space = xml_attr_get_data(p_Speed_PanTilt, "space");
                if(space != NULL)
                {
                    strcpy(pstPTZ_RelativeMove->stSpeed.stPanTilt.aszSpace, space);
                }
            }
            XMLN *p_Speed_Zoom = soap_get_child_node(p_Speed, "Zoom");
            if(p_Speed_Zoom != NULL)
            {
                const GK_CHAR *x = xml_attr_get_data(p_Speed_Zoom, "x");
                if(x != NULL)
                {
                    pstPTZ_RelativeMove->stSpeed.stZoom.x = atof(x);
                }
                else
                {
                    return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
                }
                const GK_CHAR *space = xml_attr_get_data(p_Speed_Zoom, "space");
                if(space != NULL)
                {
                    strcpy(pstPTZ_RelativeMove->stSpeed.stZoom.aszSpace, space);
                }
            }

        }
    }
    else
    {
        return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
    }
    return ONVIF_OK;

}

ONVIF_RET soap_parse_request_ContinuousMove(XMLN *p_body, GONVIF_PTZ_ContinuousMove_S *pstPTZ_ContinuousMove)
{
    XMLN *p_ContinuousMove = soap_get_child_node(p_body, "ContinuousMove");
    if(p_ContinuousMove != NULL)
    {
        //ProfileToken
        XMLN *p_ProfileToken = soap_get_child_node(p_ContinuousMove, "ProfileToken");
        if((p_ProfileToken != NULL) && (p_ProfileToken->data != NULL))
        {
            strncpy(pstPTZ_ContinuousMove->aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH-1);
        }
        else
        {
            return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
        }
        //Velocity
        XMLN *p_Velocity = soap_get_child_node(p_ContinuousMove, "Velocity");
        if(p_Velocity != NULL)
        {
            XMLN *p_Velocity_PanTilt = soap_get_child_node(p_Velocity, "PanTilt");
            if(p_Velocity_PanTilt != NULL)
            {
                const GK_CHAR *x = xml_attr_get_data(p_Velocity_PanTilt, "x");
                const GK_CHAR *y = xml_attr_get_data(p_Velocity_PanTilt, "y");
                if((x != NULL)  && (y != NULL))
                {
                    pstPTZ_ContinuousMove->stVelocity.stPanTilt.x = atof(x);
                    pstPTZ_ContinuousMove->stVelocity.stPanTilt.y = atof(y);
                }
                else
                {
                    return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
                }
                //space can be omitted
                const GK_CHAR *space = xml_attr_get_data(p_Velocity_PanTilt, "space");
                if(space != NULL)
                {
                    strncpy(pstPTZ_ContinuousMove->stVelocity.stPanTilt.aszSpace, space, MAX_SPACE_LENGTH-1);
                }

            }
            XMLN *p_Velocity_Zoom = soap_get_child_node(p_Velocity, "Zoom");
            if(p_Velocity_Zoom != NULL)
            {
                const GK_CHAR *x = xml_attr_get_data(p_Velocity_Zoom, "x");
                if(x != NULL)
                {
                    pstPTZ_ContinuousMove->stVelocity.stZoom.x = atof(x);
                }
                else
                {
                    return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
                }
                //space can be omitted
                const GK_CHAR *space = xml_attr_get_data(p_Velocity_Zoom, "space");
                if(space != NULL)
                {
                    strncpy(pstPTZ_ContinuousMove->stVelocity.stZoom.aszSpace, space, MAX_SPACE_LENGTH-1);
                }
            }
        }
        else
        {
            return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
        }
    }
    else
    {
        return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
    }

	return ONVIF_OK;

}

ONVIF_RET soap_parse_request_PtzStop(XMLN *p_node, GONVIF_PTZ_Stop_S *pstPTZ_Stop)
{
	XMLN *p_ProfileToken = soap_get_child_node(p_node, "ProfileToken");
	if (p_ProfileToken && p_ProfileToken->data)
		strncpy(pstPTZ_Stop->aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
	else
        return ONVIF_ERR_PTZ_SENDER_XML_INVALID;

	XMLN *p_PanTilt = soap_get_child_node(p_node, "PanTilt");
	if(p_PanTilt)
	{
	    if(p_PanTilt->data)
	    {
    		if (strcasecmp(p_PanTilt->data, "false") == 0)
                pstPTZ_Stop->enPanTilt = Boolean_FALSE;
            else if(strcasecmp(p_PanTilt->data, "true") == 0)
                pstPTZ_Stop->enPanTilt = Boolean_TRUE;
            else
                return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
        }
        else
        {
            return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
        }
	}

	XMLN *p_Zoom = soap_get_child_node(p_node, "Zoom");
	if(p_Zoom)
	{
	    if(p_Zoom->data)
	    {
    		if (strcasecmp(p_Zoom->data, "false") == 0)
                pstPTZ_Stop->enZoom = Boolean_FALSE;
            else if(strcasecmp(p_Zoom->data, "true") == 0)
                pstPTZ_Stop->enZoom = Boolean_TRUE;
            else
                return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
        }
        else
        {
            return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
        }
	}

	return ONVIF_OK;
}

ONVIF_RET soap_parse_request_GotoPreset(XMLN *p_node, GONVIF_PTZ_GotoPreset_S *pstGotoPreset)
{
	XMLN *p_ProfileToken = soap_get_child_node(p_node, "ProfileToken");
 	if (p_ProfileToken && p_ProfileToken->data)
 	{
 		strncpy(pstGotoPreset->aszProfileToken, p_ProfileToken->data, MAX_TOKEN_LENGTH - 1);
 	}

 	XMLN *p_PresetToken = soap_get_child_node(p_node, "PresetToken");
 	if (p_PresetToken && p_PresetToken->data)
 	{
 		strncpy(pstGotoPreset->aszPresetToken, p_PresetToken->data, MAX_TOKEN_LENGTH - 1);
 	}

 	XMLN *p_Speed = soap_get_child_node(p_node, "Speed");
    if (p_Speed)
 	{
 	    XMLN *p_PanTilt = soap_get_child_node(p_Speed, "PanTilt");
 		if (p_PanTilt)
 		{
 			const GK_CHAR *p_x = xml_attr_get_data(p_PanTilt, "x");
 			if (p_x)
 			{
 				pstGotoPreset->stSpeed.stPanTilt.x = atof(p_x);
 			}

 			const char * p_y = xml_attr_get_data(p_PanTilt, "x");
 			if (p_y)
 			{
 				pstGotoPreset->stSpeed.stPanTilt.y = atof(p_y);
 			}
 		}

 		XMLN *p_Zoom = soap_get_child_node(p_Speed, "Zoom");
 		if (p_Zoom)
 		{
 			const char * p_x = xml_attr_get_data(p_PanTilt, "x");
 			if (p_x)
 			{
 				pstGotoPreset->stSpeed.stZoom.x = atof(p_x);
 			}
 		}
 	}
 	return ONVIF_OK;
}

ONVIF_RET soap_parse_request_SetRelayOutputSettings(XMLN *p_body, GONVIF_DEVICEIO_SetRelayOutputSettings_S *pstIO_SetRelayOutputSettings)
{
    XMLN *p_SetRelayOutputSettings = soap_get_child_node(p_body, "SetRelayOutputSettings");
    if(p_SetRelayOutputSettings != NULL)
    {
        XMLN *p_RelayOutputToken = soap_get_child_node(p_SetRelayOutputSettings, "RelayOutputToken");
        if ((p_RelayOutputToken != NULL) && (p_RelayOutputToken->data != NULL))
        {
            strncpy(pstIO_SetRelayOutputSettings->aszRelayOutputToken, p_RelayOutputToken->data, MAX_TOKEN_LENGTH-1);
            pstIO_SetRelayOutputSettings->aszRelayOutputToken[MAX_TOKEN_LENGTH-1] = '\0';
        }

        XMLN *p_Properties = soap_get_child_node(p_SetRelayOutputSettings, "Properties");
        if(p_Properties != NULL)
        {
            XMLN *p_Mode = soap_get_child_node(p_Properties, "Mode");
            if(p_Mode != NULL && p_Mode->data != NULL)
            {
            	if(!strcmp(p_Mode->data, "Monostable"))
                	pstIO_SetRelayOutputSettings->stProperties.enMode = RelayMode_Monostable;
				else if(!strcmp(p_Mode->data, "Bistable"))
					pstIO_SetRelayOutputSettings->stProperties.enMode = RelayMode_Bistable;
            }
            XMLN *p_DelayTime = soap_get_child_node(p_Properties, "DelayTime");
            if(p_DelayTime != NULL && p_DelayTime->data != NULL)
            {
                pstIO_SetRelayOutputSettings->stProperties.delayTime = atoll((p_DelayTime->data)+2);
            }

            XMLN *p_IdleState = soap_get_child_node(p_Properties, "IdleState");
            if(p_IdleState != NULL && p_IdleState->data != NULL)
            {
            	if(!strcmp(p_IdleState->data, "closed"))
                	pstIO_SetRelayOutputSettings->stProperties.enIdleState = RelayIdleState_closed;
				else if(!strcmp(p_IdleState->data, "open"))
					pstIO_SetRelayOutputSettings->stProperties.enIdleState = RelayIdleState_open;
            }
        }
    }

    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_SetRelayOutputState(XMLN *p_body, GONVIF_DEVICEIO_SetRelayOutputState_S *pstIO_SetRelayOutputState)
{
    XMLN *p_SetRelayOutputState = soap_get_child_node(p_body, "SetRelayOutputState");
    if(p_SetRelayOutputState != NULL)
    {
        XMLN *p_RelayOutputToken = soap_get_child_node(p_SetRelayOutputState, "RelayOutputToken");
        if ((p_RelayOutputToken != NULL) && (p_RelayOutputToken->data != NULL))
        {
            strncpy(pstIO_SetRelayOutputState->aszRelayOutputToken, p_RelayOutputToken->data, MAX_TOKEN_LENGTH-1);
            pstIO_SetRelayOutputState->aszRelayOutputToken[MAX_TOKEN_LENGTH-1] = '\0';
        }

        XMLN *p_LogicalState = soap_get_child_node(p_SetRelayOutputState, "LogicalState");
        if(p_LogicalState != NULL && p_LogicalState->data)
        {
            if(!strcmp(p_LogicalState->data, "inactive"))
                pstIO_SetRelayOutputState->enLogicalState = RelayLogicalState_inactive;
            else if(!strcmp(p_LogicalState->data, "active"))
                pstIO_SetRelayOutputState->enLogicalState = RelayLogicalState_active;
        }
    }

    return ONVIF_OK;
}

ONVIF_RET soap_parse_request_GetSupportedAnalyticsModules(XMLN *p_body, GONVIF_ANALYTICS_GetSupportedAnalyticsModules_S *pstGetSupportedAnalyticsModulesReq)
{
    XMLN *p_GetSupportedAnalyticsModules = soap_get_child_node(p_body, "GetSupportedAnalyticsModules");
	if(p_GetSupportedAnalyticsModules == NULL)
    {
        return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
    }
    XMLN *p_ConfigurationToken = soap_get_child_node(p_GetSupportedAnalyticsModules, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(pstGetSupportedAnalyticsModulesReq->aszConfigurationToken, p_ConfigurationToken->data, MAX_TOKEN_LENGTH - 1);
	}

    return ONVIF_OK;

}

ONVIF_RET soap_parse_request_GetAnalyticsModules(XMLN *p_body, GONVIF_ANALYTICS_GetAnalyticsModules_S *pstGetAnalyticsModulesReq)
{
    XMLN *p_GetAnalyticsModules = soap_get_child_node(p_body, "GetAnalyticsModules");
	if(p_GetAnalyticsModules == NULL)
    {
        return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
    }
    XMLN *p_ConfigurationToken = soap_get_child_node(p_GetAnalyticsModules, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(pstGetAnalyticsModulesReq->aszConfigurationToken, p_ConfigurationToken->data, MAX_TOKEN_LENGTH - 1);
	}

    return ONVIF_OK;

}

ONVIF_RET soap_parse_request_GetSupportedRules(XMLN *p_body, GONVIF_ANALYTICS_GetSupportedRules_S *pstGetSupportedRulesReq)
{
    XMLN *p_GetSupportedRules = soap_get_child_node(p_body, "GetSupportedRules");
	if(p_GetSupportedRules == NULL)
    {
        return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
    }
    XMLN *p_ConfigurationToken = soap_get_child_node(p_GetSupportedRules, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(pstGetSupportedRulesReq->aszConfigurationToken, p_ConfigurationToken->data, MAX_TOKEN_LENGTH - 1);
	}

    return ONVIF_OK;

}

ONVIF_RET soap_parse_request_GetRules(XMLN *p_body, GONVIF_ANALYTICS_GetRules_S *pstGetRulesReq)
{
    XMLN *p_GetAnalyticsModules = soap_get_child_node(p_body, "GetRules");
	if(p_GetAnalyticsModules == NULL)
    {
        return ONVIF_ERR_PTZ_SENDER_XML_INVALID;
    }
    XMLN *p_ConfigurationToken = soap_get_child_node(p_GetAnalyticsModules, "ConfigurationToken");
	if (p_ConfigurationToken && p_ConfigurationToken->data)
	{
		strncpy(pstGetRulesReq->aszConfigurationToken, p_ConfigurationToken->data, MAX_TOKEN_LENGTH - 1);
	}

    return ONVIF_OK;

}

