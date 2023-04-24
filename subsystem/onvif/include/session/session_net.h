/*!
*****************************************************************************
** \file        packages/onvif/src/gk7101/gk7101_net.h
**
** \brief       Onvif Network Control Interface
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/


#ifndef  _GK7101_NET_H_
#define  _GK7101_NET_H_

#include "onvif_std.h"

//*****************************************************************************
//*****************************************************************************
//** Defines and Macros
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** Enumerated types
//*****************************************************************************
//*****************************************************************************

typedef enum 
{
    IPV4 = 0,
    IPV6
}IPADDR_TYPE;


//*****************************************************************************
//*****************************************************************************
//** Data Structures
//*****************************************************************************
//*****************************************************************************
typedef struct NetworkAdapterConfiguration
{

    struct NetworkAdapterConfiguration * next;
    char *name;
    GK_S32 (*get_mac)(const GK_S8 *netinterface, GK_S8 *mac);
	GK_S32 (*get_name)(GK_S8 *name[], GK_S32 *count_adapter);
	GK_S32 (*get_mtu)(const GK_S8 *adapter_name, GK_S32 *mtu);
	GK_S32 (*set_mtu)(const GK_S8 *adapter_name, const GK_S32 mtu);
	GK_S32 (*get_hwaddr)(const GK_S8 *adapter_name, GK_U8 hwaddr[]);
	GK_S32 (*set_hwaddr)(const GK_S8 *adapter_name, const GK_U8 hwaddr[]);
	GK_S32 (*get_ip)(const GK_S8 *adapter_name, struct sockaddr *addr);
	GK_S32 (*set_ip)(const GK_S8 *adapter_name, const struct sockaddr *addr);
	GK_S32 (*set_dhcp)(const GK_S8 *adapter_name);
	GK_S32 (*get_netmask)(const GK_S8 *adapter_name, struct sockaddr *netmask);
	GK_S32 (*set_netmask)(const GK_S8 *adapter_name, const struct sockaddr *netmask);
	GK_S32 (*get_broadaddr)(const GK_S8 *adapter_name, struct sockaddr *broadaddr);
	GK_S32 (*set_broadaddr)(const GK_S8 *adapter_name, struct sockaddr *broadaddr);
	GK_S32 (*get_ipv6)(const GK_S8 *adapter_name, struct sockaddr_in6 *ipv6);
	GK_S32 (*get_gateway)(const GK_S8 *adapter_name, struct sockaddr *gateway[], GK_S32 *count_gateway);
	GK_S32 (*set_gateway)(const GK_S8 *adapter_name, struct sockaddr *gateway[], GK_S32 count_gateway);
	GK_S32 (*get_ntp)(GK_S8 *server, GK_S32 *count, GK_BOOL *is_dhcp);;
	GK_S32 (*set_ntp)(GK_S8 *server, GK_S32 count, GK_BOOL is_dhcp);
	GK_S32 (*get_dns)(GK_S8 ***search, GK_S8 ***nameserver, GK_S8 **domain, GK_S32 *size_search, GK_S32 *size_ns);
	GK_S32 (*set_dns)(GK_CHAR search[MAX_SEARCHDOMAIN_NUM][MAX_SEARCHDOMAIN_LEN], GK_S8 **nameserver, GK_S8 *domain, GK_S32 size_search, GK_S32 size_ns);
	GK_S32 (*is_ipaddr)(const GK_S8 *ip_addr, IPADDR_TYPE iptype);
	GK_S32 (*covprefixlen)(struct sockaddr *paddr);

	/*not used*/
    GK_S32 (*get_gateway6)(const GK_S8 *adapter_name, struct sockaddr_in6 *gateway6[], GK_S32 *count_gateway6);
    GK_S32 (*get_ddns)(const GK_S8 *adapter_name, struct sockaddr_in6 *gateway6[], GK_S32 *count_gateway6);
    GK_S32 (*set_ddns)(const GK_S8 *adapter_name, struct sockaddr_in6 *gateway6[], GK_S32 *count_gateway6);
    GK_S32 (*get_hostname)(const GK_S8 *adapter_name, struct sockaddr_in6 *gateway6[], GK_S32 *count_gateway6);
    GK_S32 (*set_hostname)(const GK_S8 *adapter_name, struct sockaddr_in6 *gateway6[], GK_S32 *count_gateway6);
    GK_S32 (*is_running)(const GK_S8 *adapter_name, GK_S32 *is_running);
}tNetworkAdapterConfiguration;


//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************

#ifndef __GK7101_NET_C__
extern tNetworkAdapterConfiguration gNetworkAdapterConfiguration;
#endif   /* ----- #ifndef __GK7101_NET_C__  ----- */


//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************



#ifdef __cplusplus
extern "C" {
#endif



#ifdef __cplusplus
}
#endif

#endif   /* _GK7101_NET_H_ */
