

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/uio.h>
#include <sys/time.h>
#include <libgen.h>
#include <net/if.h>
#include <net/route.h>
#include <netinet/tcp.h>
#include <pthread.h>

#include "common.h"
#include <sys/mount.h>

//#include "linux_network.h"
#include "ppcs_search.h"
#include "sdk_network.h"
#include "netcam_api.h"
//#include "http_export.h"
#include "flash_ctl.h"
#include "parser_cfg.h"
#include "netcam_api.h"
#include "media_fifo.h"
//#include "sdk_common.h"
extern int g_s32Quit;


#define LAN_DEV     "eth0"
#define WIFI_DEV    "ra0"

#define YUNNI_TYPE_I_FRAME    0x0
#define YUNNI_TYPE_P_FRAME    0x1
#define YUNNI_TYPE_NULL       0x2
#define YUNNI_TYPE_JPEG       0x3
#define YUNNI_TYPE_AUDIO      0x8  /* (ADPCM) */

#define YUNNI_RES_640X480     0x0
#define YUNNI_RES_320X240     0x1
#define YUNNI_RES_160X120     0x2
#define YUNNI_RES_1280X720    0x3
#define YUNNI_RES_640X360     0x4
#define YUNNI_RES_1280X960    0x5
#define YUNNI_RES_1600X1200   0x6
#define YUNNI_RES_1920X1080   0x7


#define RL_IP02 	0
#define RL_IPC  	1
#define RL_IP03 	2
#define RL_B17WIFI 	3


#define HTTP_HEAD_OK    "HTTP/1.1 200 OK\r\n"\
                        "Date: Fri Jul 31 02:11:21 2015\r\n"\
                        "Server: GoAhead-Webs\r\n"\
                        "Last-modified: Thu Mar  5 06:26:56 1970\r\n"\
                        "Content-type: text/html\r\n"\
                        "Cache-Control:no-cache\r\n"\
                        "Connection: close\r\n"\
                        "Content-length: %d\r\n\r\n"

#define HTTP_HEAD_ERR   "HTTP/1.1 404 Not Found\r\n"\
                        "Date: Fri Jul 31 02:11:21 2015\r\n"\
                        "Server: GoAhead-Webs\r\n"\
                        "Last-modified: Thu Mar  5 06:26:56 1978\r\n"\
                        "Content-type: text/html\r\n"\
                        "Cache-Control:no-cache\r\n"\
                        "Connection: close\r\n"\
                        "Content-length: 0\r\n\r\n"

#define GET_FACTORY_PARAM   "var factory_server=\"%s\";\r\n"\
                            "var factory_user=\"\";\r\n"\
                            "var factory_passwd=\"\";\r\n"\
                            "var factory_heatbeat=180;\r\n"\
                            "var factory_port=808;\r\n"\
                            "var factory_index=10;\r\n"\
                            "var factory_mode=0;\r\n"\
                            "var factory_status=0;\r\n"


#define GET_STATUS_PARAM    "var alias=\"GK_IPCAM\";\r\n"\
                            "var deviceid=\"%s\";\r\n"\
                            "var software_ver=\"%s\";\r\n"\
                            "var hardware_ver=\"%s\";\r\n"\
                            "var now=1238260638;\r\n"\
                            "var alarm_status=0;\r\n"\
                            "var upnp_status=1;\r\n"\
                            "var dnsenable=0;\r\n"\
                            "var osdenable=0;\r\n"\
                            "var syswifi_mode=1;\r\n"\
                            "var mac=\"%s\";\r\n"\
                            "var wifimac=\"%s\";\r\n"\
                            "var dns_status=0;\r\n"\
                            "var authuser=0;\r\n"\
                            "var devicetype=21037151;\r\n"\
                            "var devicesubtype=0;\r\n"\
                            "var externwifi=1;\r\n"\
                            "var record_sd_status=0;\r\n"\
                            "var sdtotal=0;\r\n"\
                            "var sdfree=0;\r\n"


#define GET_WIFI_SCAN       "var ap_number=%d;\r\n"\
                            "var ap_ssid[%d] =\"%s\";\r\n"\
                            "var ap_mode[%d]=%d;\r\n"\
                            "var ap_security[%d]=%d;\r\n"\
                            "var ap_dbm0[%d]=\'%s\';\r\n"\
                            "var ap_dbm1[%d]=\'%s\';\r\n"\
                            "var ap_mac[%d]=\"%s\";\r\n"\
                            "var ap_channel[%d]=%d;\r\n"


#if 0
#define DEBUG_PRT(fmt, args...) \
do{ \
    printf("[FUN]%s [LINE]%d  "fmt, __FUNCTION__, __LINE__, ##args); \
}while(0)
//#else
#define DEBUG_PRT(fmt, args...)
#endif


#define ERROR_PRT(fmt, args...) \
do{ \
    printf("\033[0;32;31m ERROR! [FUN]%s [LINE]%d  "fmt"\033[0m", __FUNCTION__, __LINE__, ##args); \
}while(0)


typedef struct tag_FRAME_HEAD{
	unsigned int 	StartCode;
	unsigned char 	Type;
	unsigned char	FrameID;
	unsigned short	MilTime;
	unsigned int	SecTime;
	unsigned int	FrameNumber;
	unsigned int	FrameLength;
	unsigned char	Version;
	unsigned char	SessionID;
	unsigned char	D0[2];
	unsigned char	D1[8];
	char			FrameData[0];
}FRAME_HEAD,*PFRAME_HEAD;


extern GK_NET_SYSTEM_CFG runSystemCfg;

static BCASTPARAM g_bcast_param;
static BCASTDDNS  g_bcast_ddns;
static int g_search_thread_run    = 0;


static int mac_str_to_bin(unsigned char *str, char *mac)
{
    int i;
    char *s, *e;

    if ((mac == NULL) || (str == NULL))
    {
        return -1;
    }

    s = (char *) str;
    for (i = 0; i < 6; ++i)
    {
        mac[i] = s ? strtoul(s, &e, 16) : 0;
        if (s)
           s = (*e) ? e + 1 : e;
    }

    return 0;
}


static int socket_send_data(int socket, char *data, int len)
{
    int ret = -1;
    int count = 0;
    int sendLen = 0;
    while(count < 3 && (ret < 0 || sendLen < len))
    {
         ret = send(socket, data+sendLen, len-sendLen, 0);
         if (ret <= 0)
         {
            count++;
            if(errno == EINTR)
            {
                PRINT_INFO(" send error1:%s\n",strerror(errno));
            }
            else if(errno == EAGAIN)
            {
                PRINT_INFO(" send error2:%s\n",strerror(errno));
                usleep(5000);
            }
            else
            {
                PRINT_INFO(" send error3:%s\n",strerror(errno));
                count = 100;
            }
         }
         else
         {
            sendLen += ret;
            if(sendLen != len)
            {
                PRINT_INFO(" send data len error:%d, %d\n",sendLen , len);
            }
         }
         count++;
    }
    if(sendLen != len)
    {
        PRINT_INFO(" send data len error: send %d, %d,count:%d\n",sendLen , len,count);
    }else if(count > 1)
    {
        PRINT_INFO(" send data len %d ok,count:%d\n",sendLen, count);
    }
    return ret;
}


static int print_bcast_param(PBCASTPARAM pbcast_param)
{
    if (NULL == pbcast_param)
    {
        ERROR_PRT("pbcast_param:%p is NULL! \n", pbcast_param);
        return -1;
    }

    DEBUG_PRT("szIpAddr    :%s\n", pbcast_param->szIpAddr);
    DEBUG_PRT("szMask      :%s\n", pbcast_param->szMask);
    DEBUG_PRT("szGateway   :%s\n", pbcast_param->szGateway);
    DEBUG_PRT("szDns1      :%s\n", pbcast_param->szDns1);
    DEBUG_PRT("szDns2      :%s\n", pbcast_param->szDns2);
    DEBUG_PRT("nPort       :%d\n", pbcast_param->nPort);
    DEBUG_PRT("dwDeviceID  :%s\n", pbcast_param->dwDeviceID);
    DEBUG_PRT("szDevName   :%s\n", pbcast_param->szDevName);
    DEBUG_PRT("sysver      :%s\n", pbcast_param->sysver);
    DEBUG_PRT("appver      :%s\n", pbcast_param->appver);
    DEBUG_PRT("szUserName  :%s\n", pbcast_param->szUserName);
    DEBUG_PRT("szPassword  :%s\n", pbcast_param->szPassword);
    DEBUG_PRT("sysmode     :%d\n", pbcast_param->sysmode);
    DEBUG_PRT("dhcp        :%d\n", pbcast_param->dhcp);

    return 0;
}


static int print_bcast_ddns(PBCASTDDNS pbcast_ddns)
{
    if (NULL == pbcast_ddns)
    {
        ERROR_PRT("pbcast_ddns:%p is NULL! \n", pbcast_ddns);
        return -1;
    }

    DEBUG_PRT("szProxySvr   :%s\n", pbcast_ddns->szProxySvr);
    DEBUG_PRT("szDdnsName   :%s\n", pbcast_ddns->szDdnsName);
    DEBUG_PRT("szUserName   :%s\n", pbcast_ddns->szUserName);
    DEBUG_PRT("szPassword   :%s\n", pbcast_ddns->szPassword);

    return 0;
}


static int fill_yunni_frame_head(PFRAME_HEAD pframe_head, unsigned char frame_type, unsigned int data_size)
{
    struct timeval tv;

    if (NULL == pframe_head)
    {
        ERROR_PRT("Input pframe_head:%p is NULL! \n", pframe_head);
        return -1;
    }

    gettimeofday(&tv, NULL);
    pframe_head->StartCode   = 0xA815AA55;
    pframe_head->FrameID     = pframe_head->FrameNumber++;
    pframe_head->MilTime     = tv.tv_usec / 1000;
    pframe_head->SecTime     = tv.tv_sec;
    pframe_head->Type        = frame_type;
    pframe_head->FrameLength = data_size;

    return 0;
}


static int fill_bcast_param(PBCASTPARAM pbcast_param)
{
	 int ret = 0;
	 char *devname = NULL;
	 char wifi_name[SDK_ETHERSTR_LEN] = {0};
	 char szlocalip[64]  = {0};
	 char szmask[64]  = {0};
	 char gateway[64]  = {0};
	 char szMacAddr[64]  = {0};
	 char dns1[64]	= {0};
	 char dns2[64]	= {0};
	 ST_SDK_NETWORK_ATTR	 net_attr;
	 //struct device_info_mtd  deviceInfo;
	 //printf("fill_bcast_param 1\n");
	 if (NULL == pbcast_param)
	 {
		 ERROR_PRT("pbcast_param:%p is NULL! \n", pbcast_param);
		 return -1;
	 }
	// printf("fill_bcast_param 2\n");
	 memset(&net_attr, 0, sizeof(ST_SDK_NETWORK_ATTR));
	 ret = netcam_net_get_detect(LAN_DEV);
	 if(0 == ret)
	 {
		 //printf("fill_bcast_param 3\n");
		 strncpy(net_attr.name, LAN_DEV, SDK_ETHERSTR_LEN-1);
		 ret = sdk_net_get_ip(net_attr.name,szlocalip);
		 if(ret== -1)
		 {
					 devname = netcam_net_wifi_get_devname();
					 if(devname!= NULL)
					 {
					 strcpy(wifi_name, netcam_net_wifi_get_devname());
					 ret = sdk_net_get_ip(wifi_name,szlocalip);
					 if(ret == -1)
					 {
						 sprintf(szlocalip,"%s",runNetworkCfg.lan.ip);
						 sprintf(szmask,"%s",runNetworkCfg.lan.netmask);
						 sprintf(gateway,"%s",runNetworkCfg.lan.gateway);
						 sprintf(szMacAddr,"%s",runNetworkCfg.lan.mac);
						 sprintf(dns1,"%s",runNetworkCfg.lan.dns1);
						 sprintf(dns2,"%s",runNetworkCfg.lan.dns2);
					 }
					 else
					 {
						 sdk_net_get_netmask(wifi_name,szmask);
						 sdk_net_set_gateway(wifi_name,gateway);
						 sdk_net_get_hwaddr(wifi_name,szMacAddr);
						 sdk_net_get_dns(dns1,dns2);
	
					 }
					 }
	
		 }
		 else
		 {
			 sdk_net_get_netmask(net_attr.name,szmask);
			 sdk_net_set_gateway(net_attr.name,gateway);
			 sdk_net_get_hwaddr(net_attr.name,szMacAddr);
			 sdk_net_get_dns(dns1,dns2);
		 }
	 }
	 else
	 {
	  //  printf("fill_bcast_param 31\n");
		 devname = netcam_net_wifi_get_devname();
		 if(devname!= NULL)
		 {
		 strcpy(wifi_name, netcam_net_wifi_get_devname());
		 ret = sdk_net_get_ip(wifi_name,szlocalip);
		 if(ret == -1)
		 {
						 sprintf(szlocalip,"%s",runNetworkCfg.wifi.ip);
						 sprintf(szmask,"%s",runNetworkCfg.wifi.netmask);
						 sprintf(gateway,"%s",runNetworkCfg.wifi.gateway);
						 sprintf(szMacAddr,"%s",runNetworkCfg.wifi.mac);
						 sprintf(dns1,"%s",runNetworkCfg.wifi.dns1);
						 sprintf(dns2,"%s",runNetworkCfg.wifi.dns2);	 
		 }
		 else
		 {
			 sdk_net_get_netmask(wifi_name,szmask);
			 sdk_net_set_gateway(wifi_name,gateway);
			 sdk_net_get_hwaddr(wifi_name,szMacAddr);
			 sdk_net_get_dns(dns1,dns2);
		 
		 }
	
		 }
	 }
	// printf("fill_bcast_param 4\n");
	/*
	 ret = netcam_net_get(&net_attr);
	 if(ret != 0)
	 {
		 printf("Faile to Get net_attr.");
		 return -1;
	 }
	 */
	 strncpy(pbcast_param->szHostIpAddr,  szlocalip,		sizeof(pbcast_param->szHostIpAddr));
	 strncpy(pbcast_param->szIpAddr,  szlocalip,		sizeof(pbcast_param->szIpAddr));
	 strncpy(pbcast_param->szMask,	  szmask,	   sizeof(pbcast_param->szMask));
	 strncpy(pbcast_param->szGateway, gateway,	 sizeof(pbcast_param->szGateway));
	 strncpy(pbcast_param->szDns1,	  dns1, 	 sizeof(pbcast_param->szDns1));
	 strncpy(pbcast_param->szDns2,	  dns2, 	 sizeof(pbcast_param->szDns2));
	 mac_str_to_bin((unsigned char *)szMacAddr, pbcast_param->szMacAddr);
	 pbcast_param->nPort = runNetworkCfg.port.httpPort;
	#if 0
	 //yunni_p2p_get_device_info_load(&deviceInfo);
	 memset(&deviceInfo, 0, sizeof(struct device_info_mtd));
	 if(get_dev_info_from_mtd("/dev/mtd2", 0x1000, &deviceInfo))
	 {
		 printf("fail to get %s\n","/dev/mtd2");
	 }
	 pbcast_param->dhcp = runNetworkCfg.lan.dhcpIp;
	
	 strncpy(pbcast_param->dwDeviceID, deviceInfo.device_id, sizeof(pbcast_param->dwDeviceID));
	 sprintf(pbcast_param->szDevName, "%s", runSystemCfg.deviceInfo.deviceName);
	 sprintf(pbcast_param->szManufacturerId, "%s", deviceInfo.manufacturer_id);
	 sprintf(pbcast_param->dwAeskey, "%s", deviceInfo.aeskey);
	 //sprintf(pbcast_param->szMacAddr, "%s", szMacAddr);
	 sprintf(pbcast_param->szWifiMacAddr, "%s", deviceInfo.wifi_mac_addr);
	 strncpy(pbcast_param->sysver,	   runSystemCfg.deviceInfo.firmwareVersion, sizeof(pbcast_param->sysver));
	 strncpy(pbcast_param->appver,	   runSystemCfg.deviceInfo.softwareVersion, sizeof(pbcast_param->appver));
	#endif
	
	strcpy(pbcast_param->dwDeviceID, "GOHN-000001-GNWZP");//, sizeof(pbcast_param->dwDeviceID));
	 strcpy(pbcast_param->szDevName, "goke_test");
	 pbcast_param->sysmode = RL_IPC;
	
	 print_bcast_param(pbcast_param);
	
	 return 0;

}


static int fill_bcast_ddns(PBCASTDDNS pbcast_ddns)
{
    if (NULL == pbcast_ddns)
    {
        ERROR_PRT("pbcast_ddns:%p is NULL! \n", pbcast_ddns);
        return -1;
    }

    strncpy(pbcast_ddns->szProxySvr,  "www.wangguixing.com", sizeof(pbcast_ddns->szProxySvr));
    strncpy(pbcast_ddns->szDdnsName,  "guixing", sizeof(pbcast_ddns->szDdnsName));
    strncpy(pbcast_ddns->szUserName,  "guixing_gk", sizeof(pbcast_ddns->szUserName));
    strncpy(pbcast_ddns->szPassword,  "12345678", sizeof(pbcast_ddns->szPassword));

 //   print_bcast_ddns(pbcast_ddns);

    return 0;
}


static int set_bcast_param(PBCASTPARAM pbcast_param)
{
    if (NULL == pbcast_param)
    {
        ERROR_PRT("pbcast_param:%p is NULL! \n", pbcast_param);
        return -1;
    }
#if 0
	printf("+++++++++++++++++++++++++++++++++\n");
	/*
	ST_SDK_NETWORK_ATTR net_attr;
	sprintf(net_attr.name,"eth0");

	net_attr.enable = 1;
	netcam_net_get(&net_attr);

	net_attr.dhcp = pbcast_param->dhcp;
	strncpy(net_attr.ip,pbcast_param->szIpAddr,sizeof(net_attr.ip));
    strncpy(net_attr.mask,pbcast_param->szMask,sizeof(net_attr.mask));
    strncpy(net_attr.gateway,pbcast_param->szGateway,sizeof(net_attr.gateway));
    strncpy(net_attr.dns1,pbcast_param->szDns1,sizeof(net_attr.dns1));
    strncpy(net_attr.dns2,pbcast_param->szDns2,sizeof(net_attr.dns2));

	if (netcam_net_set(&net_attr) == 0)
		netcam_net_cfg_save();
    */

	struct device_info_mtd device_info;
	memset(&device_info, 0, sizeof(struct device_info_mtd));

	get_dev_info_from_mtd("/dev/mtd2", 0x1000, &device_info);



	if (strlen(pbcast_param->szMacAddr))
	{
		memset(device_info.eth_mac_addr, 0, sizeof(device_info.eth_mac_addr));
		sprintf(device_info.eth_mac_addr,"%s", pbcast_param->szMacAddr);
	}
	else
	{
         memset(device_info.eth_mac_addr, 0, sizeof(device_info.eth_mac_addr));
	}

	
	if (strlen(pbcast_param->szWifiMacAddr))
	{
		memset(device_info.wifi_mac_addr, 0, sizeof(device_info.wifi_mac_addr));
		sprintf(device_info.wifi_mac_addr,"%s", pbcast_param->szWifiMacAddr);
	}
	else
	{
         memset(device_info.wifi_mac_addr, 0, sizeof(device_info.wifi_mac_addr));
	}
    if (strlen(pbcast_param->dwDeviceID))
	{
		memset(device_info.device_id, 0, sizeof(device_info.device_id));
		sprintf(device_info.device_id,"%s", pbcast_param->dwDeviceID);
	}
	else
	{
        memset(device_info.device_id, 0, sizeof(device_info.device_id));
	}

	if (strlen(pbcast_param->szDevName))
	{
		memset(runSystemCfg.deviceInfo.deviceName, 0, sizeof(runSystemCfg.deviceInfo.deviceName));
		sprintf(runSystemCfg.deviceInfo.deviceName,"%s", pbcast_param->szDevName);
	}
	else
	{
		memset(runSystemCfg.deviceInfo.deviceName, 0, sizeof(runSystemCfg.deviceInfo.deviceName));
	}
	if (strlen(pbcast_param->szManufacturerId))
	{
		memset(device_info.manufacturer_id, 0, sizeof(device_info.manufacturer_id));
		sprintf(device_info.manufacturer_id,"%s", pbcast_param->szManufacturerId);
	}
	else
	{
		memset(device_info.manufacturer_id, 0, sizeof(device_info.manufacturer_id));
	}


	

	if (strlen(pbcast_param->dwDeviceID1))
	{
		memset(device_info.ndt_id, 0, sizeof(device_info.ndt_id));
		sprintf(device_info.ndt_id,"%s", pbcast_param->dwDeviceID1);
	}
	else
	{
        memset(device_info.ndt_id, 0, sizeof(device_info.ndt_id));
	}
	
	if (strlen(pbcast_param->dwAeskey))
	{
		memset(device_info.aeskey, 0, sizeof(device_info.aeskey));
		sprintf(device_info.aeskey,"%s", pbcast_param->dwAeskey);
	}
	else
	{
        memset(device_info.aeskey, 0, sizeof(device_info.aeskey));
	}
	if (strlen(pbcast_param->dwDeviceID3))
	{
		memset(device_info.urvideo_id, 0, sizeof(device_info.urvideo_id));
		sprintf(device_info.urvideo_id,"%s", pbcast_param->dwDeviceID3);
	}
	else
	{
        memset(device_info.urvideo_id, 0, sizeof(device_info.urvideo_id));
	}

    if (strlen(pbcast_param->dwDeviceID2))
	{
		memset(device_info.wipn_id, 0, sizeof(device_info.wipn_id));
		sprintf(device_info.wipn_id,"%s", pbcast_param->dwDeviceID2);
	}
	else
	{
        memset(device_info.wipn_id, 0, sizeof(device_info.wipn_id));
	}
	

	if (set_dev_info_to_mtd("/dev/mtd2", 0x1000, &device_info))
	{
		PRINT_INFO("Fail to set yunni device id\n");
		return -1;
	}
	
#endif
    return 0;

}


static int set_bcast_ddns(PBCASTDDNS pbcast_ddns)
{
    if (NULL == pbcast_ddns)
    {
        ERROR_PRT("pbcast_ddns:%p is NULL! \n", pbcast_ddns);
        return -1;
    }

    return 0;
}


static int process_cmd(char *cmd_buf, unsigned int cmd_len, unsigned short *cmd_type)
{
	  int  ret	 = 0;
	  char *pbuf = 0;
	  unsigned short start_code = 0;
	  unsigned short cmd_code	= 0;
	  char szlocalip[64];
	  char szwifilocalip[64];
	  memset(szwifilocalip,0,sizeof(szwifilocalip));
	  memset(szlocalip,0,sizeof(szlocalip));
	  if (NULL == cmd_buf || NULL == cmd_type)
	  {
		  ERROR_PRT("cmd_buf:%p  cmd_type:%p  is NULL! \n", cmd_buf, cmd_type);
		  return -1;
	  }
	
	  if (cmd_len < 4 || cmd_len >= RECV_BUF_SIZE)
	  {
		  ERROR_PRT("cmd_len:%d is err!\n", cmd_len);
		  return -1;
	  }
	 
	  char *wifi_name = netcam_net_wifi_get_devname();
	  if(wifi_name== NULL)
	  {
			  sdk_net_get_ip(runNetworkCfg.lan.netName,szlocalip);
	  }
	  else
	  {
			  ret =  sdk_net_get_ip(runNetworkCfg.wifi.netName,szwifilocalip);
			  if(ret == -1)
			  {
				  sdk_net_get_ip(runNetworkCfg.lan.netName,szlocalip);
			  }
	  }
	  start_code = ((cmd_buf[1] << 8) | cmd_buf[0]);
	  cmd_code	 = ((cmd_buf[3] << 8) | cmd_buf[2]);
	
	  if (START_CODE != start_code)
	  {
		  ERROR_PRT("start_code:0x%x is err!\n", start_code);
		  return -1;
	  }
	  printf("start_code:0x%x  cmd_code:0x%x \n", start_code, cmd_code);
	  switch(cmd_code)
	  {
		  case CMD_SET:
			  pbuf = &cmd_buf[4];
			  memcpy(&g_bcast_param, pbuf, sizeof(g_bcast_param));
					if(strcmp(szlocalip,g_bcast_param.szHostIpAddr)==0 || strcmp(szwifilocalip,g_bcast_param.szHostIpAddr)==0)
					{								  
			  print_bcast_param(&g_bcast_param);
			  ret = set_bcast_param(&g_bcast_param);
			  if (ret < 0)
			  {
				  ERROR_PRT("Do set_bcast_param ret:0x%x is err!\n", ret);
				  return -1;
			  }
			  }
			  break;
	
		  case CMD_GET:
			  memset(&g_bcast_param,0,sizeof(BCASTPARAM));
			  ret = fill_bcast_param(&g_bcast_param);
			  if (ret < 0)
			  {
				  ERROR_PRT("fill_bcast_param ret:0x%x is err!\n", ret);
				  return -1;
			  }
			  memset(&g_bcast_ddns,0,sizeof(BCASTDDNS));
			  ret = fill_bcast_ddns(&g_bcast_ddns);
			  if (ret < 0)
			  {
				  ERROR_PRT("fill_bcast_ddns ret:0x%x is err!\n", ret);
				  return -1;
			  }
			  break;
	
		  default:
			  ERROR_PRT("cmd_code:0x%x is err!\n", cmd_code);
			  return -1;
			  break;
	  }
	
	  *cmd_type = cmd_code;
	
	//	printf("start_code:0x%x  cmd_code:0x%x \n", start_code, cmd_code);
	
	  return 0;

}


static int make_cmd_buf(unsigned int cmd_type, char cmd_buf[RECV_BUF_SIZE], unsigned int *cmd_buf_len)
{
	   char  *p  = NULL;
	
	   if (NULL == cmd_buf || NULL == cmd_buf_len)
	   {
		   ERROR_PRT("cmd_buf:%p  cmd_buf:%p  is NULL! \n", cmd_buf, cmd_buf_len);
		   return -1;
	   }
	
	   switch(cmd_type)
	   {
		   case CMD_GET:
			   cmd_buf[0] = 0x44;
			   cmd_buf[1] = 0x48;
			   cmd_buf[2] = 0x01;
			   cmd_buf[3] = 0x08;
			   p = &cmd_buf[4];
			   memcpy(p, &g_bcast_param, sizeof(g_bcast_param));
			   p = &cmd_buf[4 + sizeof(g_bcast_param)];
			   memcpy(p, &g_bcast_ddns, sizeof(g_bcast_ddns));
			   *cmd_buf_len = 4 + sizeof(g_bcast_param) + sizeof(g_bcast_ddns);
			   break;
	
		   case CMD_SET:
			   cmd_buf[0] = 0x44;
			   cmd_buf[1] = 0x48;
			   cmd_buf[2] = 0x02;
			   cmd_buf[3] = 0x08;
			   p = &cmd_buf[4];
			   memcpy(p, &g_bcast_param, sizeof(g_bcast_param));
			   cmd_buf[4 + sizeof(g_bcast_param)]	  = 0x0;
			   cmd_buf[4 + sizeof(g_bcast_param) + 1] = 0x80;
			   *cmd_buf_len = 4 + sizeof(g_bcast_param) + 2;
			   break;
	
		   default:
			   ERROR_PRT("cmd_type:0x%x  error!\n", cmd_type);
			   return -1;
			   break;
	   }
	
	 //  DEBUG_PRT(" cmd_buf_len:%d   0x%x 0x%x 0x%x 0x%x  cmd_buf:%s \n",
	 // 												 *cmd_buf_len,
	 // 												  cmd_buf[0], cmd_buf[1],
	 // 												  cmd_buf[2], cmd_buf[3],
	//													  cmd_buf);
	   return 0;

}


/*
* broadcast
*/
static int create_search_sock()
{
    int ret = 0;
	int opt;

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if(sock < 0)
	{
        PRINT_ERR("socket failed, error:%s\n", strerror(errno));
        goto cleanup;
    }

	//设置IP重用，避免bind时出现Address already in use
    opt = 1;
    ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    if(ret < 0)
	{
        PRINT_ERR("setsockopt SO_REUSEADDR failed, error:%s\n", strerror(errno));
        goto cleanup;
    }

	//set SO_BROADCAST
    opt = 1;
    ret = setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &opt, sizeof(opt));
    if(ret < 0)
	{
        PRINT_ERR("setsockopt SO_BROADCAST failed, error:%s\n", strerror(errno));
        goto cleanup;
    }

	struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(SEARCH_PORT);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    ret = bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    if(ret < 0)
	{
        PRINT_ERR("bind failed, error:%s\n", strerror(errno));
        goto cleanup;
    }

    return sock;

cleanup:
    if(sock >= 0)
        close(sock);
    return -1;
}


static int search_sendto(int sock, const char* inData, const int inLength)
{
	int ret = 0;
	int sendlen = 0;
    struct sockaddr_in addr;

    memset(&addr, 0,sizeof(&addr));
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = inet_addr(SEARCH_BROADCAST_ADDR);
    addr.sin_port        = htons(SEARCH_PORT);

	//因为UDP 发送时，没有缓冲区，故需要对发送后的返回值进行判断后，多次发送
	while(sendlen < inLength)
	{
	    ret = sendto(sock, inData + sendlen, inLength - sendlen, 0, (struct sockaddr*)&addr,  sizeof(addr));
		if(ret < 0)
		{
			if (errno != EINTR && errno != EAGAIN )//no blocked error = EAGAIN
			{
				PRINT_ERR("send() socket %d error :%s\n", sock, strerror(errno));
				return -1;
			}
			else
				continue;
		}

     DEBUG_PRT("sendto ip:%s:%d ret:%d \n", inet_ntoa(addr.sin_addr),ntohs(addr.sin_port),ret);

		sendlen += ret;
	}

	return sendlen;
}


static int remote_sendto(int sock, struct sockaddr_in * paddr, int addrlen,const char* inData, const int inLength)
{
	int ret = 0;
	int sendlen = 0;

	//因为UDP 发送时，没有缓冲区，故需要对发送后的返回值进行判断后，多次发送
	while(sendlen < inLength)
	{
	    ret = sendto(sock, inData + sendlen, inLength - sendlen, 0, (struct sockaddr*)paddr,  addrlen);
		if(ret < 0)
		{
			if (errno != EINTR && errno != EAGAIN )//no blocked error = EAGAIN
			{
				PRINT_ERR("send() socket %d error :%s\n", sock, strerror(errno));
				return -1;
			}
			else
				continue;
		}

    DEBUG_PRT("sendto ip:%s:%d ret:%d  len = %d\n", inet_ntoa(paddr->sin_addr),ntohs(paddr->sin_port), ret,inLength);
		sendlen += ret;
	}

	return sendlen;
}


void * ppcs_tool_discovery_thread(void *param)
{
    int ret           = 0;
    int ipHasValid    = 0;
    int search_socket = 0;
    int addr_len      = 0;
    int recv_len      = 0;
    unsigned int   cmd_len    = 0;
    unsigned short cmd_code   = 0;
    fd_set fdr;
    struct timeval time = {0, 100};
    struct sockaddr_in  remote_addr;
    char recv_buf[RECV_BUF_SIZE] = {0};

    DEBUG_PRT("in tool_discovery_thread\n");
	sdk_sys_thread_set_name("ppcs_tool_discovery");
    g_search_thread_run = 1;
    while (!g_s32Quit)
    {
        if(ipHasValid == 0 && 1 == g_search_thread_run)
        {
            /* create listen the search message. */
            search_socket = create_search_sock();
            if(search_socket < 0)
            {
                ERROR_PRT("create search_socket error.\n");
                g_search_thread_run = 0;
                return NULL;
            }
            DEBUG_PRT("search_socket id:%d \n", search_socket);
            ipHasValid = 1;
        }

        if(0 == g_search_thread_run)
        {
            break;
        }

        time.tv_sec  = 0;
        time.tv_usec = 500;
        FD_ZERO(&fdr);
        FD_SET(search_socket, &fdr);
        select(search_socket + 1, &fdr, NULL, NULL, &time);

        if (!FD_ISSET(search_socket, &fdr))
        {
			      usleep(10000);
            continue;
        }

        memset(recv_buf, 0x0, sizeof(recv_buf));
        addr_len = sizeof(remote_addr);
        bzero(&remote_addr, sizeof(remote_addr));
        recv_len = recvfrom(search_socket, recv_buf, RECV_BUF_SIZE, 0, (struct sockaddr *)&remote_addr, (socklen_t * )&addr_len);
        if (recv_len < 0)
        {
            ERROR_PRT("recv error.\n");
            break;
        }
        else if (recv_len == 0) {
            ERROR_PRT("recv ==== 0 !\n");
						usleep(10000);
            continue;
        }

        recv_buf[recv_len] = 0x00;

        DEBUG_PRT("\n\n====================================================\n");
        DEBUG_PRT("remote_addr: %s:%d recv_len:%d  recv_buf:%s\n", inet_ntoa(remote_addr.sin_addr), ntohs(remote_addr.sin_port), recv_len, recv_buf);

        ret = process_cmd(recv_buf, recv_len, &cmd_code);
        if (ret < 0)
        {
            ERROR_PRT("process_cmd ret:%d error!\n", ret);
            continue;
        }

        memset(recv_buf, 0x0, sizeof(recv_buf));
        ret = make_cmd_buf(cmd_code, recv_buf, &cmd_len);
        if (0 == ret)
        {
           // search_sendto(search_socket, recv_buf, cmd_len);
            addr_len = sizeof(remote_addr);
            remote_sendto(search_socket, &remote_addr, addr_len,recv_buf, cmd_len);
        }
    }

    if(search_socket > 0)
    {
        close(search_socket);
    }

    DEBUG_PRT("cms_discovery_thread end\n");
    return NULL;
}



#if 0
static int yunni_decoder_ptz(HTTP_OPS* ops, void* arg)
{
    int  ret   = 0;
    int  cmd   = 0;
    const char *pbuf = 0;
    static int test_flag = 0;

    if (NULL == (pbuf = ops->get_param_string(ops, "command")))
    {
        PRINT_ERR("get_param_string command don't find!\n");
        return -1;
    }

    cmd = ops->get_param_int(ops, "command");

    DEBUG_PRT("======  yunni_decoder_ptz  cmd:%d  ==================\n", cmd);

    switch (cmd)
    {
        case 0:     /* up */
            if ((ret = netcam_ptz_up(50, 3)))
            {
                PRINT_ERR("call  netcam_ptz_up error!\n");
                return -1;
            }
            break;
        case 2:     /* down */
            if ((ret = netcam_ptz_down(50, 3)))
            {
                PRINT_ERR("call  netcam_ptz_down error!\n");
                return -1;
            }
            break;
        case 4:     /* left */
            if ((ret = netcam_ptz_left(50, 3)))
            {
                PRINT_ERR("call  netcam_ptz_left error!\n");
                return -1;
            }
            break;
        case 6:     /* right */
            if ((ret = netcam_ptz_right(50, 3)))
            {
                PRINT_ERR("call  netcam_ptz_right error!\n");
                return -1;
            }
            break;
        case 90:     /* left_up */
            if ((ret = netcam_ptz_left_up(50, 3)))
            {
                PRINT_ERR("call  netcam_ptz_left_up error!\n");
                return -1;
            }
            break;
        case 91:     /* right_up */
            if ((ret = netcam_ptz_right_up(50, 3)))
            {
                PRINT_ERR("call  netcam_ptz_right_up error!\n");
                return -1;
            }
            break;
        case 92:     /* left_down */
            if ((ret = netcam_ptz_left_down(50, 3)))
            {
                PRINT_ERR("call  netcam_ptz_left_down error!\n");
                return -1;
            }
            break;
        case 93:     /* right_down */
            if ((ret = netcam_ptz_right_down(50, 3)))
            {
                PRINT_ERR("call  netcam_ptz_right_down error!\n");
                return -1;
            }
            break;
        case 26:     /* up_down     */
            if ((ret = netcam_ptz_vertical_cruise(3)))
            {
                PRINT_ERR("call  netcam_ptz_vertical_cruise error!\n");
                return -1;
            }
            break;
        case 28:     /* left_right      */
            if ((ret = netcam_ptz_horizontal_cruise(3)))
            {
                PRINT_ERR("call  netcam_ptz_horizontal_cruise error!\n");
                return -1;
            }
            break;

        case 1:     /* up stop */
        case 3:     /* down stop */
        case 5:     /* left stop */
        case 7:     /* right stop */
        case 27:    /* up_down_stop */
        case 29:    /* left_right_stop */
            if ((ret = netcam_ptz_stop()))
            {
                PRINT_ERR("call  netcam_ptz_stop error!\n");
                return -1;
            }
            test_flag = 0;
            break;

        case 255:    /* testing ptz motor  */
            if (0 == test_flag)
            {
                netcam_ptz_hor_ver_cruise(3);
                test_flag = 1;
            }
            else
            {
                netcam_ptz_stop();
                test_flag = 0;
            }
            break;

        case 25:     /* center point */
        case 30:     /* set prefab bit for number 1   */
        case 31:     /* call perfab bit for number 1   */
        case 60:     /* set prefab bit for number 16 */
        case 61:     /* call perfab bit for number 16 */
        case 94:     /* set ptz IO high */
        case 95:     /* set ptz IO low  */
            PRINT_INFO("This command:%d  don't support now!\n", cmd);
            break;

        default:
            PRINT_ERR("This command:%d is error!\n", cmd);
            return -1;
            break;
    }

    return 0;
}
#endif





#if 0
static int device_upgrade_state(HTTP_OPS* ops, void* arg)
{
	char retData[256];
	int rate;
	memset(retData, 0, 256);
	//int method = ops->get_method(ops);

	rate = netcam_update_get_process();

	sprintf(retData,"{\"upgrade_state\": \"%d\"}",rate);
	ops->set_body_ex(ops,retData,strlen(retData));

	return HPE_RET_SUCCESS;
}
#endif



int ppcs_search_daemon_init(void)
{
    int ret = 0;
    pthread_t thread_id;

    ret = pthread_create(&thread_id, NULL, ppcs_tool_discovery_thread, NULL);
    if(ret < 0)
    {
        PRINT_ERR("create tool_discovery_thread failed! %s\n", strerror(errno));
        return -1;
    }


    return 0;
}


int ppcs_search_daemon_exit()
{
    g_search_thread_run = 0;
    usleep(200000);
    PRINT_INFO("descovery_deinit...\n");
    return 0;
}


