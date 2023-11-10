/*!
*****************************************************************************
** \file      $gkprjsdk_network.h
**
** \version	$id: sdk_network.h 15-08-13  8月:08:1439435123
**
** \brief
**
** \attention   this code is provided as is. goke microelectronics
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by GOKE co.,ltd
*****************************************************************************
*/
#ifndef _SDK_NETWORK_H_
#define _SDK_NETWORK_H_
#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <pthread.h>
#include <semaphore.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <dirent.h>



#define LAN     "eth0"
//#define WIFI    "wlan0"
#define SDK_ETHERSTR_LEN    20
#define SDK_MACSTR_LEN      20
#define SDK_IPSTR_LEN       16
#define SDK_MASKSTR_LEN     16
#define SDK_GATEWAYSTR_LEN  16
#define SDK_DNSSTR_LEN      16


#define LEN_NAME_ADAPTER           10
#define MAX_CMD_LENGTH          100
#define MAX_INFO_LENGTH         512

#define _PATH_PROCNET_DEV          "/proc/net/dev"
#define _PATH_PROCNET_IFINET6      "/proc/net/if_inet6"
#define _PATH_PROCNET_ROUTE        "/proc/net/route"
#define _PATH_DNS_CONT             "/etc/resolv.conf"
#define _PATH_NTP_CONT             "/etc/ntp.conf"

#define NETWOR_TOOLS_CTRL_FILE     "/tmp/network_tools_ctrl"
#define NETWORK_TOOLS_CTRL_AP_MODE      "AP_MODE\n"
#define NETWORK_TOOLS_CTRL_STA_MODE     "STA_MODE\n"
#define NETWORK_TOOLS_CTRL_LAN_DHCP     "LAN_DHCP\n"
#define NETWORK_TOOLS_CTRL_WIFI_DHCP     "WIFI_DHCP\n"

typedef enum
{
    IPV4 = 0,
    IPV6
}IPADDR_TYPE;



typedef struct {
    int  enable;
    int dhcp;
    int httpPort;
    //int upnp;
    char name[SDK_ETHERSTR_LEN];
    unsigned char mac[SDK_MACSTR_LEN];
    char ip[SDK_IPSTR_LEN];
    char mask[SDK_MASKSTR_LEN];
    char gateway[SDK_GATEWAYSTR_LEN];
    char dns1[SDK_DNSSTR_LEN];
    char dns2[SDK_DNSSTR_LEN];
}ST_SDK_NETWORK_ATTR, *PT_SDK_NETWORK_ATTR;

int sdk_net_set_dhcp(const char *adapter_name, int enable, char *host_name);
int sdk_net_get_ntp(char *server, int *count, int *is_dhcp);
int sdk_net_set_ntp(char *server, int count, int is_dhcp);
int sdk_net_set_dns(char *dns1, char *dns2);
int sdk_net_get_dns(char *dns1, char *dns2);
int sdk_net_get_mtu(const char *adapter_name, int *mtu);
int sdk_net_set_mtu(const char *adapter_name, const int mtu);
int sdk_net_get_hwaddr(const char *adapter_name,  char *hwaddr);
int sdk_net_set_hwaddr(const char *adapter_name,  char *hwaddr);
int sdk_net_get_netmask(const char *adapter_name,  char *netmask);
int sdk_net_set_netmask(const char *adapter_name, char *netmask);
int sdk_net_get_ip(const char *adapter_name,  char *addr);
int sdk_net_set_ip(const char *adapter_name, const char *addr);
int sdk_net_set_gateway(const char *adapter_name, const char *gateway);
int sdk_net_get_gateway(const char *adapter_name,  char *gateway);
int get_ipv6(const char *adapter_name,  char *ipv6);
int sdk_net_get_broadaddr(const char *adapter_name,  char *broadaddr);
int sdk_net_set_broadaddr(const char *adapter_name,  char *broadaddr);
int sdk_net_get_name(char *name[], int *count_adapter);
int sdk_net_is_ipaddr(const char *ip_addr, IPADDR_TYPE iptype);
int sdk_net_up_down_network(const char *netinterface, int enable);
int new_system_call(const char *cmdline);
int sdk_net_get_detect(char *name);
int sdk_network_tools_ctrl(char *cmd_file, char *cmd);

#ifdef __cplusplus
};
#endif
#endif

