

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
#include <ctype.h>
#include "work_queue.h"


#include "common.h"
#include <sys/mount.h>
#include "sdk_sys.h"
//#include "linux_network.h"
#include "ipcsearch.h"
//#include "search_ipc.h"
#include "sdk_network.h"
#include "netcam_api.h"
#include "http_export.h"
#include "flash_ctl.h"
#include "parser_cfg.h"
#include "netcam_api.h"
#include "media_fifo.h"
#include "cfg_network.h"
#include "cfg_gb28181.h"
//#include "app_yunni.h"
//#include "sdk_common.h"
#include "sdk_cfg.h"
#include "sdk_network.h"
#include "gk_cms_utility.h"
#include "avi_utility.h"

#define RECV_BUF_SIZE       (15 * 1024)
#define SEND_BUF_SIZE       (4 * 1024)
#define CMD_SIZE             12

#define START_CODE          0x4844
#define CMD_GET_PC			0x0201
#define CMD_GET_RET         0x0801
#define CMD_SET_RET         0x0802
#define CMD_GET             0x0101
#define CMD_SET             0x0102
#define CMD_SET_TCP 	  	0x0102
#define CMD_UPGRADE 		0x0103
#define CMD_STATUS          0x0104
#define CMD_SET_P2PINFO     0x0105
#define CMD_SET_RESET       0x0106
#define CMD_SET_P2PINFO_pc 	0x0107
#define CMD_SET_REBOOT   	0x0108
#define CMD_PHONE_SEARCH    0x0109
#define CMD_SET_TELNET  	0x0202
#define CMD_UPGRADE_FAST  	0x0204
#define CMD_UPGRADE_BIN  	0x0205
#define CMD_UPGRADE_GET_NAME  	0x0206

#define CMD_MOJING_SEARCH           0x0301
#define CMD_MOJING_SEARCH_RET       0x0302
#define CMD_MOJING_SEARCH_RESET     0x0303
#define CMD_MOJING_GET_LIST_GET     0x0304
#define CMD_MOJING_GET_LIST_RET     0x0305

#define CMD_GET_DS_INFO     0x0113
#define CMD_SET_DS_INFO     0x0114
#define CMD_BATCH_SET_DS_INFO     0x0115//批量设置参数(设备id除外)，用广播方式，
#define CMD_GET_FAC_CFG     0x0116




#define SEARCH_BROADCAST_ADDR   "255.255.255.255"
#define SEARCH_BROADCAST_SEND_PORT   0x8888//20909
#define SEARCH_BROADCAST_RECV_PORT   0x9888//9095
#define SEARCH_TOOL_BROADCAST_RECV_PORT   9095
#define UPGRADE_PORT         16888
#define GK_CMS_TCP_LISTEN_PORT 0x9123
#define	SERVER_PACK_FLAG 0x03404324
#define GK_CMS_BUFLEN 4096
#define DMS_MAX_IP_LENGTH 		16
#define IPTOSBUFFERS    12

#define LAN_DEV     "eth0"
#define DANA_FILE "/tmp/dana/danale.conf"
#define ULU_CONF_DEVICEID_PATH "/opt/custom/ulucu"
#define UPGRADE_FAST_PATH "/tmp/up.bin"




#pragma pack(1)
typedef struct _stBcastParam{
	char           szIpAddr[16];     //IP地址，可以修改
	char           szMask[16];       //子网掩码，可以修改
	char           szGateway[16];    //网关，可以修改
	char           szDns1[16];       //dns1，可以修改
	char           szDns2[16];       //dns2，可以修改
	char           szMacAddr[6];     //设备有线MAC地址
	unsigned short nPort;            //设备端口
	char           dwDeviceID[32];   //platform deviceid
	char           szDevName[80];    //设备名称
	char           sysver[16];       //固件版本
	char           appver[16];       //软件版本
	char           szUserName[32];   //修改时会对用户认证
	char           szPassword[32];   //修改时会对用户认证
	char           sysmode;          //0->baby 1->HDIPCAM
	char           dhcp;             //0->禁止dhcp,1->充许DHCP
	char           other[2];         //other
	char           other1[20];       //other1
	char           type_name[32];
	char           upgradeVersion[32];
	char		   upgrade_status;//升级状态
}BCASTPARAM, *PBCASTPARAM;
#pragma pack()

#pragma pack(1)
typedef struct _stBcastDDNS{
    char    szProxySvr[64];          //服务名字
    char    szDdnsName[64];          //ddns名字
    char    szUserName[32];          //用户名
    char    szPassword[32];          //密码
}BCASTDDNS,*PBCASTDDNS;
#pragma pack()




typedef struct _P2P_Supportinfo
{
	char		P2P_Type;			//P2P类型
	struct device_info_mtd yunni_info;
    #ifdef MODULE_SUPPORT_ULUCU
	Dev_SN_Info ulu_info;
    #endif
	char dana_info[32];
	struct device_info_mtd tutk_info;
}SupportP2P_INFO;

static int g_search_thread_run    = 0;
static int g_IphasVaild = 0;
static SupportP2P_INFO g_get_sup_p2p_info_s[10];
//static SupportP2P_INFO g_set_sup_p2p_info;


pthread_mutex_t up_send_cmd_mutex = PTHREAD_MUTEX_INITIALIZER;

typedef struct Upgrade_FILE_HEAD
{
	int cmd;
	unsigned long nBufSize;		//??????????
}GK_UPGRADE_FILE_HEAD,*UPGRADE_FILE_HEAD;

struct P2P_Support_info
{
	int P2P_Type[10] ;
	int P2P_Count ;
};

typedef struct tagSEARCH_CMD
{
    unsigned long dwFlag; //0x4844
    unsigned long dwCmd;

}SEARCH_CMD;

typedef struct tagSEARCH_CMD_RET
{
    unsigned long dwFlag; //0x4844
    unsigned long dwCmd;
    unsigned long dwCmdFrom;
    unsigned long status;
}SEARCH_CMD_RET;

typedef struct tagSEARCH_CMD_RESET
{
    unsigned long dwFlag; //0x4844
    unsigned long dwCmd;
    char          devId[32];
}SEARCH_CMD_RESET;



typedef struct tagSEARCH_CMD_MOJING_GET
{
    unsigned long dwFlag; //0x4844
    unsigned long dwCmd;
    char          devId[32];
    char          ip[SDK_IPSTR_LEN];
    char          bMac[SDK_MACSTR_LEN];
    int           mojingStatus;
    int           dnsStatus;
}SEARCH_CMD_MOJING_GET;


typedef struct tagJBNV_SERVER_PACK
{
	char	szIp[16];			//服务器Ip
	WORD	wMediaPort;			//流端口
	WORD	wWebPort;			//Http端口号
	WORD	wChannelCount;		//通道数量
	char	szServerName[32];	//服务器名
	DWORD	dwDeviceType;		//服务器类型,柔乐协议,0-卡片机, 1-摇头机, 2-鱼眼摄像机...,3,门铃单向，4 门铃双向，5模组
	DWORD	dwServerVersion;	//服务器版本
	WORD	wChannelStatic;		//通道状态(是否视频丢失)
	WORD	wSensorStatic;		//探头状态
	WORD	wAlarmOutStatic;	//报警输出状态
}JBNV_SERVER_PACK;

typedef struct tagJBNV_SERVER_PACK_EX
{
	JBNV_SERVER_PACK jspack;
	BYTE	        bMac[6];
	BOOL	        bEnableDHCP;
	BOOL			bEnableDNS;
	DWORD			dwNetMask;
	DWORD			dwGateway;
	DWORD			dwDNS;
	DWORD			dwComputerIP;
	BOOL	        bEnableCenter;
	DWORD			dwCenterIpAddress;
	DWORD			dwCenterPort;
	char			csServerNo[64];
	int				bEncodeAudio;
}JBNV_SERVER_PACK_EX;

typedef struct tagJBNV_SERVER_MSG_DATA_EX
{
	DWORD					dwSize;
	DWORD					dwPackFlag; // == SERVER_PACK_FLAG
	JBNV_SERVER_PACK_EX		jbServerPack;
}JBNV_SERVER_MSG_DATA_EX;

//用于获取或者设置设备的基本信息，和搜索协议一起，广播方式
typedef struct
{
	char 			p2pid[32];			// p2pid号
	unsigned char 	devType;			// 设备类型: 0-卡片机, 1-摇头机, 2-鱼眼摄像机...
	unsigned char 	netFamily;			// 网络提供商: 1-迈特威视, 2-tutk...
	unsigned char 	serverID;			// 服务器ID: 保留,默认为0
	unsigned char 	language;			// 设备固件语言版本: 0为自动适应(即多语言),1为简体中文,2为繁体中文,3为英文
	unsigned int 	odmID;				// odm商ID: 0-东舜自己, 01-TCL, 02-康佳...
	unsigned int 	panoramaMode;		// 全景安装模式: 0-吸顶式,1-壁挂式
    unsigned char   version[16];        //版本号，取固件编译日期年月日时分秒20161222110101
    unsigned char   model[16];          //子型号。比如鱼眼可能有多种子型号的机型
    unsigned int    web_port;           //web端口
	unsigned int 	manufacture;		// 制造商号: 请与我们联系获取,0x676b647a
	unsigned char	reserve2[24];    	// 暂时保留，可能还要加mac地址等信息
    char       my_key[256];         // 加密使用

}P2P_DEVICE_INFO;
//用于获取或者设置设备的基本信息，和搜索协议一起，广播方式
typedef struct
{

	DWORD					dwSize;
	DWORD					dwPackFlag; // == SERVER_PACK_FLAG

    P2P_DEVICE_INFO         p2pDeviceInfo;
}P2P_DEVICE_INFO_EX;

typedef struct tagJBNV_PHONE_WIFI_INFO
{
	char ssid[64];
    char psd[64];
    char devId[32];
}JBNV_PHONE_WIFI_INFO;

typedef enum {
	GK_MSG_UPGRADE =                    0x40000000,  //??????
	GK_MSG_CRCERROR =                   0x40000001,  //??CRC?
	GK_MSG_UPGRADEOK =          		0x40000002,  //????
} gkMsg;
typedef enum
{
    WF_READ,
    WF_WRITE
}WAIT_EVENT;

typedef enum
{
    JB_TEST_RESULT_DNSUP = 0,   // 不支持
    JB_TEST_RESULT_DNT,         // 尚未测试
    JB_TEST_RESULT_SUCCESS,     // 测试成功
    JB_TEST_RESULT_FAILED,      // 测试失败
    JB_TEST_RESULT_BUTT,
}  JB_TEST_RESULT_E;

typedef struct tagNET_TEST_RESULT_INFO
{
    JB_TEST_RESULT_E emSpeaker;
    JB_TEST_RESULT_E emMic;
    JB_TEST_RESULT_E emLed;
    JB_TEST_RESULT_E emSd;
    JB_TEST_RESULT_E emIRCut;
    JB_TEST_RESULT_E emWifi;
    JB_TEST_RESULT_E emPTZ;
    JB_TEST_RESULT_E emRestore;
    JB_TEST_RESULT_E emView;
    JB_TEST_RESULT_E emWhiteLed;
}NET_TEST_RESULT_INFO,*PNET_TEST_RESULT_INFO;

struct send_msg {
		char upgrade_state;
};
static struct P2P_Support_info P2P_sup_info_s = {{0},0};
static BCASTPARAM g_bcast_param_s;
static BCASTDDNS  g_bcast_ddns_s;
static WIFI_LINK_INFO_t g_link_info={0};

int BoardCastHandleMsg(int send_sock, int send_port, void* pBuf);
int gk_cms_change_mac(unsigned char *hwaddr, unsigned char *addr);
int dwip_to_str(DWORD ip, char ip_str[32]);


static unsigned int search_get_startup_time_seconds()
{
    char buff[128];
    int i;
    FILE *fp = fopen("/proc/uptime","r");

    if(fp == NULL)
    {
        printf("open /proc/uptime failed\n");
        return 0;
    }
    memset(buff,0,sizeof(buff));
    if(fgets(buff,sizeof(buff),fp) == NULL)
    {
    	printf("read /proc/uptime failed\n");
    	fclose(fp);
    	return 0;
    }
    fclose(fp);
    i = 0;
    while(buff[i]!= 0 && buff[i] != '.' && i < sizeof(buff))
    {
    	i++;
    }
    buff[i] = 0;
    return atoi(buff);

}


static int  search_wifi_connect(WIFI_LINK_INFO_t *linkinfoPtr)
{
    WIFI_LINK_INFO_t linkInfo;
    int ret;
    ST_SDK_NETWORK_ATTR net_attr2;
    int wifiMode = runNetworkCfg.wireless.mode;
    memset(&linkInfo,0,sizeof(WIFI_LINK_INFO_t));
    PRINT_INFO("link wifi: ssid:<%s>,psd:<%s>\n",g_link_info.linkEssid,g_link_info.linkPsd);
    network_wifi_get_connect_info(&linkInfo);
    g_link_info.isConnect = 1;
    if(linkInfo.linkStatus == WIFI_CONNECT_OK &&
       strcmp(linkInfo.linkEssid,g_link_info.linkEssid) == 0 &&
       strcmp(linkInfo.linkPsd,g_link_info.linkPsd) == 0 )
    {
        PRINT_INFO("link wifi has connected\n");

        //netcam_audio_hint(SYSTEM_AUDIO_HINT_LINK_SUCCESS);
    }
    else
    {
        //netcam_audio_hint(SYSTEM_AUDIO_HINT_USE_WIRELESS);	xqq
        memset(&linkInfo,0,sizeof(WIFI_LINK_INFO_t));
		linkInfo.isConnect = 1;
        linkInfo.linkScurity = WIFI_ENC_WPAPSK_AES;
		strncpy(linkInfo.linkEssid,g_link_info.linkEssid,sizeof(linkInfo.linkEssid));
		strncpy(linkInfo.linkPsd,g_link_info.linkPsd,sizeof(linkInfo.linkPsd));
		ret = netcam_net_setwifi(NETCAM_WIFI_STA,linkInfo);

        if (ret <0)
        {
            //netcam_audio_hint(SYSTEM_AUDIO_HINT_LINK_FAIL);	xqq
            if (wifiMode == 0)
            {
                //NETCAM_WIFI_HostAPConfigT apCfg;
                printf("xxxx failed to set sta wifi parament ,back to AP mode!\n");
                netcam_net_setwifi(NETCAM_WIFI_AP,linkInfo);

                char cmd[128]={0};
                sprintf(cmd,"route add -host 255.255.255.255 dev %s",netcam_net_wifi_get_devname());
                new_system_call(cmd);
            }
        }
        else
        {
            printf("xxxx set wifi success\n");
		    netcam_net_get(&net_attr2);
            //netcam_audio_hint(SYSTEM_AUDIO_HINT_LINK_SUCCESS);	xqq
	        //netcam_audio_ip(net_attr2.ip);
        }
        netcam_net_cfg_save();
    }
    g_link_info.isConnect = 0;
    //netcam_timer_del_task2(search_wifi_connect);
    return 0;
}

static void Search_mac_tansform(char* src,char *dst)
{
	char temp[SDK_MACSTR_LEN];
	int i = 0;
	memset(temp,0,sizeof(temp));
	for(i = 0;i < 6; i++)
	{
		char _temp = src[i]>>4;
		if(_temp < 10)
			temp[i*3] = '0' +  _temp;
		else
			temp[i*3] = 'W'  + _temp;
		_temp = src[i]&0x0F;
		if(_temp < 10)
			temp[i*3+1] = '0' + _temp;
		else
			temp[i*3+1] = 'W' + _temp;
		if(i != 5)
			temp[3*i+2] = ':';
	}
	memcpy(dst, temp, SDK_MACSTR_LEN);
}

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

static void Search_fill_p2p_info()
{
	int _cun = 0;

	struct goke_factory factory;
	memset(&factory, 0, sizeof(struct goke_factory));
	load_info_to_mtd_reserve(MTD_GOKE_INFO,&factory,sizeof(struct goke_factory));

	while(_cun < P2P_sup_info_s.P2P_Count)
	{
		switch(P2P_sup_info_s.P2P_Type[_cun])
		{
			case MTD_YUNNI_P2P:
				{
					struct device_info_mtd yunni_info;

					memset(&yunni_info, 0, sizeof(struct device_info_mtd));
					if(load_info_to_mtd_reserve(P2P_sup_info_s.P2P_Type[_cun],&yunni_info,sizeof(struct device_info_mtd)) < 0)
					{
						PRINT_ERR("Get P2P_info failed!!!\n");
						g_get_sup_p2p_info_s[_cun].P2P_Type = P2P_sup_info_s.P2P_Type[_cun];
					}
					else
					{
						g_get_sup_p2p_info_s[_cun].P2P_Type = P2P_sup_info_s.P2P_Type[_cun];
						memcpy(&g_get_sup_p2p_info_s[_cun].yunni_info,&yunni_info, sizeof(struct device_info_mtd));
					}

					strncpy(g_get_sup_p2p_info_s[_cun].yunni_info.eth_mac_addr,factory.mac,32);
				}
				break;
			case MTD_ULUCU_P2P:
				{
                    #ifdef MODULE_SUPPORT_ULUCU
					Dev_SN_Info ulu_info;
					memset(&ulu_info, 0, sizeof( Dev_SN_Info));
					if(load_info_to_mtd_reserve(P2P_sup_info_s.P2P_Type[_cun],&ulu_info,sizeof(Dev_SN_Info)) < 0)
					{
						PRINT_ERR("Get P2P_info failed!!!\n");
						g_get_sup_p2p_info_s[_cun].P2P_Type = P2P_sup_info_s.P2P_Type[_cun];

					}
					else
					{
						g_get_sup_p2p_info_s[_cun].P2P_Type = P2P_sup_info_s.P2P_Type[_cun];
						memcpy(&g_get_sup_p2p_info_s[_cun].ulu_info,&ulu_info, sizeof(Dev_SN_Info));
					}

					strncpy(g_get_sup_p2p_info_s[_cun].ulu_info.MAC,factory.mac,18);
                    #endif      
				}
				break;
			case MTD_DANA_P2P:
				{
					struct Dana_info dana_info;
					if(load_info_to_mtd_reserve(P2P_sup_info_s.P2P_Type[_cun],&dana_info,sizeof(struct Dana_info)) < 0)
					{
						PRINT_ERR("Get P2P_info failed!!!\n");
						g_get_sup_p2p_info_s[_cun].P2P_Type = P2P_sup_info_s.P2P_Type[_cun];
					}
					else
					{
						g_get_sup_p2p_info_s[_cun].P2P_Type = P2P_sup_info_s.P2P_Type[_cun];
						memcpy(&g_get_sup_p2p_info_s[_cun].dana_info,&dana_info.Dana_ID, 32);
					}


				}
				break;
			case MTD_TUTK_P2P:
				{
					struct device_info_mtd tutk_info;

					memset(&tutk_info, 0, sizeof(struct device_info_mtd));

					if(load_info_to_mtd_reserve(P2P_sup_info_s.P2P_Type[_cun],&tutk_info,sizeof(struct device_info_mtd)) < 0)
					{
						PRINT_ERR("Get P2P_info failed!!!\n");
						g_get_sup_p2p_info_s[_cun].P2P_Type = P2P_sup_info_s.P2P_Type[_cun];
					}
					else
					{
						g_get_sup_p2p_info_s[_cun].P2P_Type = P2P_sup_info_s.P2P_Type[_cun];
						memcpy(&g_get_sup_p2p_info_s[_cun].tutk_info,&tutk_info, sizeof(struct device_info_mtd));
					}
					strncpy(g_get_sup_p2p_info_s[_cun].tutk_info.eth_mac_addr,factory.mac,32);
				}
				break;
			case MTD_GB28181_INFO:
				{
					struct device_info_mtd gb28181_info;

					memset(&gb28181_info, 0, sizeof(struct device_info_mtd));

					if(load_info_to_mtd_reserve(P2P_sup_info_s.P2P_Type[_cun],&gb28181_info,sizeof(struct device_info_mtd)) < 0)
					{
						PRINT_ERR("Get gb28181_info failed!!!\n");
						g_get_sup_p2p_info_s[_cun].P2P_Type = P2P_sup_info_s.P2P_Type[_cun];
					}
					else
					{
						g_get_sup_p2p_info_s[_cun].P2P_Type = P2P_sup_info_s.P2P_Type[_cun];
						memcpy(&g_get_sup_p2p_info_s[_cun].tutk_info,&gb28181_info, sizeof(struct device_info_mtd));
					}
				}
				break;

			default:
				break;
		}
		_cun++;
	}
}

static void fill_dana_file(char *file_data,int length)
{
	FILE *fd;
	fd = fopen("/tmp/danale.conf","wb+");
	if(fd == NULL)
	{
		PRINT_ERR("ERROR!!!Failed to open or create the file!!!");
		return ;
	}
	int ret = fwrite(file_data,1,length,fd);
	if(ret != length)
	{
		PRINT_ERR("the ret is %d\n",ret);
		PRINT_ERR("Failed to write data to file!!!");
	}
	fclose(fd);
}

#ifdef MODULE_SUPPORT_ULUCU
static void fill_uluc_file(char *file_data,int length)
{
	FILE *fd;
	fd = fopen("/tmp/uluc.conf","wb+");
	if(fd == NULL)
	{
		PRINT_ERR("ERROR!!!Failed to open or create the file!!!");
		return ;
	}
	int ret = fwrite(file_data,1,length,fd);
	if(ret != length)
	{
		PRINT_ERR("the ret is %d\n",ret);
		PRINT_ERR("Failed to write data to file!!!");
		return ;
	}
	fclose(fd);
}
#endif

static int Search_print_bcast_param(PBCASTPARAM pbcast_param)
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
	DEBUG_PRT("type_name    :%s\n", pbcast_param->type_name);
	DEBUG_PRT("upgradeVersion :%s\n", pbcast_param->upgradeVersion);
	DEBUG_PRT("upgrade_status :%d\n", pbcast_param->upgrade_status);
	return 0;
}
static int Upgrade_NotifyUpgrade(int sock, int write_size, int type)
{
	struct send_msg send_data;
	send_data.upgrade_state = write_size&0xff;

	int ret = send(sock,&send_data, sizeof(struct send_msg), 0);//Upgrade_SockSend(sock, &send_data, sizeof(struct send_msg), 100);
	if (ret != (sizeof(struct send_msg))) {
	    PRINT_ERR("send:%d size:%d", ret, sizeof(struct send_msg));
	    return -1;
	} else {
	    //PRINT_INFO("send %d data_size:%d, head_size:%d\n", ret, data_size, sizeof(JB_NET_PACK_HEAD) + sizeof(JB_SERVER_MSG));
	}

	return 0;
}
static int Upgrade_NotifyUpgradeDoing(int sock, int write_size)
{
	return Upgrade_NotifyUpgrade(sock, write_size, GK_MSG_UPGRADE);
}

static int Upgrade_NotifyUpgradeDone(int sock, int write_size)
{
	return Upgrade_NotifyUpgrade(sock, write_size, GK_MSG_UPGRADEOK);
}
static int UpgradeSetSockAttr(int fd)
{

	int opt = 1;
	int ret = setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (ret < 0) {
		PRINT_ERR("setsockopt SO_REUSEADDR failed, error:%s\n", strerror(errno));
		return -1;
	}

	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1) {
		PRINT_ERR("Fail to get old flags [set_nonblocking]! errno[%d] errinfo[%s]\n",
			errno, strerror(errno));
		return -1;
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1) {
		PRINT_ERR("Fail to set flags to O_NONBLOCK [set_nonblocking]! errno[%d] errinfo[%s]\n",
			errno, strerror(errno));
		return -1;
	}


	// set nodelay
	int on = 1;
	setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));

	return 0;
}
static int Get_Upgrade_Status(int sock)
{
	int ret;
	int cnt = netcam_update_get_process();
	if (cnt < 99)
	{
		ret = Upgrade_NotifyUpgradeDoing(sock, cnt);
		usleep(218 * 1000);
		PRINT_INFO("sock:%d cnt:%d  ret:%d \n", sock, cnt, ret);
	}
	else
	{
		ret = Upgrade_NotifyUpgradeDone(sock, 100);

		PRINT_INFO("Gk_CmsNotifyUpgradeDone! sock:%d  ret:%d\n", sock, ret);
		return 1;
	}

	return 0;
}
static int time_wait_search(int sock, WAIT_EVENT e, int sec, int usec, int times)
{
	int ret = 0;
	fd_set fset;
	struct timeval tv;
	while(times > 0)
	{
		FD_ZERO(&fset);
		FD_SET(sock,&fset);

		tv.tv_sec = sec;
		tv.tv_usec = usec;

		if(e ==WF_READ)
			ret = select(sock+1, &fset, NULL, NULL, &tv);
		else
			ret = select(sock+1, NULL, &fset, NULL, &tv);
		if(ret > 0)
			goto finish;
		else if(ret < 0){
			if(errno != EINTR && errno != EAGAIN)
				goto error;
			}
		times--;
	}
error:
	PRINT_ERR("select %s [time_wait] in IPCSearch!fd[%d] times[%d] errno[%d] errinfo[%s]\n",errno == 0?"timeout":"error",sock,times,errno,strerror(errno));
finish:
	return ret;
}
static int recv_data(int sock,char* buffer,int len)
{
	int n , ret;
	char* pbuf =buffer;
	int offset = 0;
	while(len > 0)
	{
		n = recv(sock,pbuf,len,0);
		if(n < 0)
		{
			if(errno == EINTR || errno == EAGAIN||errno == EWOULDBLOCK)
			{
				ret = time_wait_search(sock, WF_READ, 0, 200000, 25);
				if(ret > 0)
					continue;
				else if (ret == 0)
					break;
				else
					goto error;
			}
			else
			{
				ret = 1;
				goto error;
			}
		}
		if (n == 0)
			break;
		len -= n;
		offset += n;
		pbuf += n;
	}
	return offset;
error:
	PRINT_ERR("Failed to recv data[recv_data]! n[%d] fd[%d] ret[%d] errno[%d] errinfo[%s]\n",n, sock, ret, errno, strerror(errno));
	return -1;
}


#ifndef _GET_ERRNO
#define _GET_ERRNO() errno
#endif
static int wr_timedwait(int sock, int read_opt, int sec, int usec, int times)
{
	int ret = 0;
	fd_set fset;
	struct timeval tv;

	while (times > 0) {
		FD_ZERO(&fset);
		FD_SET(sock, &fset);
		tv.tv_sec = sec;
		tv.tv_usec = usec;

		if (read_opt) {
			ret = select(sock + 1, &fset, NULL, NULL, &tv);
		} else {
			ret = select(sock + 1, NULL, &fset, NULL, &tv);
		}
 		if ((ret > 0) || ((ret < 0) &&
 		        (_GET_ERRNO() != EINTR) && (_GET_ERRNO() !=EAGAIN))) {
            break;
		}
		times--;
        printf("wr_timedwait: %s sock[%d] select timeout, times[%d], ret = %d, errno = %d\n",read_opt?"read":"write", sock, times,ret, errno);
	}
	return ret;
}

static int sock_wrn(int sock, char *buffer, int size, int timeout_sec, int
read_opt)
{
    int ret = 0;
	int len = 0;

    /* read data until size is reached, or tired of timeout */
    while (len < size) {
        if (read_opt) {
            ret = recv(sock, (buffer + len), (size - len), 0);
        } else {
            ret = send(sock, (buffer + len), (size - len), 0);
        }
		if (ret <= 0) {
		    if ((_GET_ERRNO() == EINTR) || (_GET_ERRNO() == EAGAIN)) {
    			ret = wr_timedwait(sock, read_opt, 1, 0, timeout_sec);
    			if (ret > 0) {
    				continue;//read again
    			} else {
                    goto ERROR;
    			}
			} else {
                goto ERROR;
            }
		}
        len += ret;
    }
    return ret;
ERROR:
    printf("sock_wr_n: %s-> sock[%d] ret[%d] errno[%d] [%d - %d = %d]\n",
	    read_opt?"read":"write", sock, ret, _GET_ERRNO(), size, len, (size - len)
);
    return ret;
}

static int Upgrade_CmdUpgradeData(int sock, int ext_len)
{
	int   ret       = 0;
	char *pRecv_buf = NULL;
	//runSystemCfg.deviceInfo.upgradestatus = netcam_get_update_status();
	if(netcam_get_update_status() < 0)
	{
	    PRINT_ERR("is updating..............\n");
	    Upgrade_NotifyUpgradeDoing(sock,0);
	    return 0;
	}

	PRINT_INFO("sock:%d  ext_len:%d\n", sock, ext_len);

	netcam_update_relase_system_resource();

    pRecv_buf = netcam_update_malloc(ext_len + 2);
	if (NULL == pRecv_buf)
	{
		PRINT_ERR("malloc upgrade_size:%u ERROR! %s\n", ext_len + 2, strerror(errno));
		goto UP_UPGRADE_EXIT;
	}
	sock_wrn(sock, pRecv_buf, ext_len, 30, 1);


	//校验数据
	ret = netcam_update(pRecv_buf, ext_len, NULL);
	if (ret < 0)
	{
		PRINT_ERR("Check updating package, failed: %d \n", ret);
		Upgrade_NotifyUpgradeDoing(sock,0);
		goto UP_UPGRADE_EXIT;
	}

	return 0;

UP_UPGRADE_EXIT:
	usleep(500 * 1000);
	netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT
	return 0;
}

static int sock_wrn_fast(int sock, int size, int timeout_sec, int
read_opt)
{
    int ret = 0;
    int len = 0;
    int left = 0;
    int tmpSize = 64 * 1024;
    FILE *upFp = NULL;

    char *buffer = (char*)malloc(tmpSize);
    
    if (buffer == NULL)
    {
        printf("sock_wrn_fast malloc error:%d\n", tmpSize);
        return 0;
    }
    upFp = fopen(UPGRADE_FAST_PATH, "wb");
    if (upFp == NULL)
    {
        return -1;
    }

    /* read data until size is reached, or tired of timeout */
    left = size;
    while (len < size) {
        tmpSize = left > 65536?65536:left;
        if (read_opt) {
            ret = recv(sock, buffer, tmpSize, 0);
        } else {
            ret = send(sock, buffer, tmpSize, 0);
        }
        if (ret <= 0) {
            if ((_GET_ERRNO() == EINTR) || (_GET_ERRNO() == EAGAIN)) {
                ret = wr_timedwait(sock, read_opt, 1, 0, timeout_sec);
                if (ret > 0) {
                    continue;//read again
                } else {
                    goto ERROR;
                }
            } else {
                goto ERROR;
            }
        }
        left -= ret;
        fwrite(buffer, 1, ret, upFp);
        len += ret;
    }
    fclose(upFp);
    if (buffer)
        free(buffer);
    return len;
ERROR:
    printf("sock_wr_n: %s-> sock[%d] ret[%d] errno[%d] [%d - %d = %d]\n",
        read_opt?"read":"write", sock, ret, _GET_ERRNO(), size, len, (size - len)
);
        

    
    if (buffer)
        free(buffer);
    return ret;
}


static int Upgrade_CmdUpgradeDataFast(int sock, int ext_len)
{
	int   ret       = 0;
    char cmds[512] = {0};
	//runSystemCfg.deviceInfo.upgradestatus = netcam_get_update_status();
	if(netcam_get_update_status() < 0)
	{
	    PRINT_ERR("is updating..............\n");
	    Upgrade_NotifyUpgradeDoing(sock,0);
	    return 0;
	}

	PRINT_INFO("sock:%d  ext_len:%d\n", sock, ext_len);

	netcam_update_relase_system_resource();

    ret = sock_wrn_fast(sock, ext_len, 30, 1);
    printf("sock_wrn_fast %d,%d\n", ret, ext_len);
    if (ret >= ext_len)
    {
        sprintf(cmds, "touch /tmp/feeddog;killall -9 netcam;/usr/local/bin/upgrade_flash %s;reboot -f", UPGRADE_FAST_PATH);
        new_system_call(cmds);
    }

	return 0;

	usleep(500 * 1000);
	netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT
	return 0;
}

static int Search_set_bcast_param(PBCASTPARAM pbcast_param)
{
	struct goke_factory factory;
	int ret = 0;
	char wifi_name[SDK_ETHERSTR_LEN] = {0};

	if (NULL == pbcast_param)
	{
		ERROR_PRT("pbcast_param:%p is NULL! \n", pbcast_param);
		return -1;
	}

	PRINT_INFO("Set ....\n");
	ST_SDK_NETWORK_ATTR net_attr;
	memset(&net_attr, 0, sizeof(ST_SDK_NETWORK_ATTR));
	ret = netcam_net_get_detect(LAN_DEV);
	if(ret == 0)
	{
		strncpy(net_attr.name, LAN_DEV, SDK_ETHERSTR_LEN-1);
	}
	else
	{
		if(netcam_net_wifi_get_devname() != NULL)
		{
			strcpy(wifi_name, netcam_net_wifi_get_devname());
			if((ret = netcam_net_get_detect(wifi_name)))
			{
				PRINT_ERR("Detect net device LAN_DEV:%s  WIFI_DEV%s error!\n", LAN_DEV, wifi_name);
				return -1;
			}
			strncpy(net_attr.name, wifi_name, SDK_ETHERSTR_LEN-1);
		}
		else
		{
		    return -1;
		}
	}
	netcam_net_get(&net_attr);
	//net_attr.enable = 1;
	net_attr.dhcp = pbcast_param->dhcp;
	strncpy(net_attr.ip,pbcast_param->szIpAddr,sizeof(net_attr.ip));
	strncpy(net_attr.mask,pbcast_param->szMask,sizeof(net_attr.mask));
	strncpy(net_attr.gateway,pbcast_param->szGateway,sizeof(net_attr.gateway));
	strncpy(net_attr.dns1,pbcast_param->szDns1,sizeof(net_attr.dns1));
	strncpy(net_attr.dns2,pbcast_param->szDns2,sizeof(net_attr.dns2));
	if(netcam_net_get_detect(LAN_DEV) == 0)
		Search_mac_tansform((char *)pbcast_param->szMacAddr,(char *)net_attr.mac);

	load_info_to_mtd_reserve(MTD_GOKE_INFO,&factory, sizeof(struct goke_factory));
	memcpy(factory.mac, net_attr.mac, MAX_STR_LEN_20);
	save_info_to_mtd_reserve(MTD_GOKE_INFO,&factory, sizeof(struct goke_factory));
	ret = netcam_net_set(&net_attr) ;
	if (ret == 0)
		netcam_net_cfg_save();
	else
		PRINT_ERR("Set net info error!");
	return ret;
}


int UpgradeDealCmdDispatch(int sock)
{
	int ret = 0;
	char *pbuf = 0;
	fd_set fdr;
	char recv_buf[RECV_BUF_SIZE] = {0};
	char cmd[3] = {0};
	char data[8] = {0};
	char send_buf[4];
	unsigned short start_code = 0;
	unsigned short cmd_code   = 0;
	int type = 0;

#ifdef MODULE_SUPPORT_ULUCU
	static Dev_SN_Info ulu_info;
#endif
	struct device_info_mtd yunni_info;
	struct Dana_info dana_info;
	struct goke_factory factory;
    GK_UPGRADE_FILE_HEAD cmd_head = {0};

	ret = UpgradeSetSockAttr(sock);
	if (ret < 0) {
		ERROR_PRT("UpgradeSetSockAttr error\n");
		close(sock);
		return -1;
	}
	int opt = 1;
	ret = setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	if (ret < 0) {
		ERROR_PRT("setsockopt SO_REUSEADDR failed, error:%s\n", strerror(errno));
		return -1;
	}
	memset(send_buf,0x0,sizeof(send_buf));
	while (1) {
		memset(&cmd_head, 0x0, sizeof(GK_UPGRADE_FILE_HEAD));
		memset(recv_buf,0x0,sizeof(recv_buf));
		ret = recv_data(sock,recv_buf,12);
		if(ret <= 0)
		{
			ERROR_PRT("Recv cmd error ret:%d\n", ret);
		}
		start_code = ((recv_buf[1] << 8) | recv_buf[0]);
		cmd_code   = ((recv_buf[3] << 8) | recv_buf[2]);
		//PRINT_INFO("===start_code is %x==\n==cmd_code is %x========\n",start_code,cmd_code);
		//start_code = ((recv_buf[1] << 8) | recv_buf[0]);
		if (START_CODE!= start_code)
		{
			ERROR_PRT("start_code:0x%x is err!\n", start_code);
			close(sock);
			return -1;
		}

    	memcpy(cmd,recv_buf+2,2);
		cmd_head.cmd = atoi(cmd);
		switch (cmd_code ) {
			case CMD_UPGRADE:
				strncpy(data,recv_buf+4,8);
				cmd_head.nBufSize = atoi(data);
				PRINT_INFO("Ready upgrade  cmd_head.nBufSize is %d\n ",(int)cmd_head.nBufSize);
				Upgrade_CmdUpgradeData(sock,cmd_head.nBufSize);
				break;
            case CMD_UPGRADE_FAST:  
                strncpy(data,recv_buf+4,8);
				cmd_head.nBufSize = atoi(data);
				PRINT_INFO("Ready upgrade fast  cmd_head.nBufSize is %d\n ",(int)cmd_head.nBufSize);
				Upgrade_CmdUpgradeDataFast(sock,cmd_head.nBufSize);
				break;
			case CMD_STATUS:
				ret = Get_Upgrade_Status(sock);
				if(ret)
				{
					close(sock);
				}
				break;
			case CMD_SET_TCP:
				memset(recv_buf,0x0,sizeof(recv_buf));
				ret = recv_data(sock,recv_buf,sizeof(recv_buf));
				pbuf = &recv_buf[0];
				memcpy(&g_bcast_param_s, pbuf, sizeof(g_bcast_param_s));
				Search_print_bcast_param(&g_bcast_param_s);
				ret = Search_set_bcast_param(&g_bcast_param_s);
				if (ret < 0)
				{
					ERROR_PRT("Do set_bcast_param ret:%d is err!\n", ret);
					close(sock);
					return -1;
				}
				close(sock);
				return 0;
				break;
            case CMD_SET_RESET:
            {
				PRINT_INFO("Reset from pc\n ");
                netcam_sys_operation((void*)0,(void*)SYSTEM_OPERATION_HARD_DEFAULT);
                break;
            }
            case CMD_SET_REBOOT:
            {
				PRINT_INFO("reboot from pc\n ");
                netcam_sys_operation(0,SYSTEM_OPERATION_REBOOT);
                break;
            }
            case CMD_SET_TELNET:
            {
                char status = 0;
                if (access("/opt/custom/cfg/telnet", F_OK) != 0)
                {
                    PRINT_INFO("open telnet\n ");
                    new_system_call("touch /opt/custom/cfg/telnet");
                    new_system_call("telnetd");
                    status = 1;
                }
                else
                {
                    PRINT_INFO("close telnet\n ");
                    new_system_call("rm -f /opt/custom/cfg/telnet");
                    new_system_call("killall telnetd");
                    status = 0;
                }
                int ret = send(sock, &status, 1, 0);
            	if (ret != 1) {
            	    PRINT_ERR("send:%d size:%d", ret, 1);
            	} 
                close(sock);
				return 0;
                break;
            }
			case CMD_SET_P2PINFO_pc:
				memset(recv_buf,0x0,sizeof(recv_buf));
				ret = recv_data(sock,recv_buf,sizeof(recv_buf));
				type = recv_buf[0];
				load_info_to_mtd_reserve(MTD_GOKE_INFO,&factory, sizeof(struct goke_factory));
				switch(type)
				{
					case MTD_YUNNI_P2P:
						pbuf = &recv_buf[1];
						memset(&yunni_info, 0, sizeof(yunni_info));
						memcpy(&yunni_info,pbuf,sizeof(struct device_info_mtd));
						memcpy(factory.mac, yunni_info.eth_mac_addr, MAX_STR_LEN_20);
						ret = save_info_to_mtd_reserve(MTD_YUNNI_P2P, &yunni_info, sizeof(yunni_info));
						break;
					case MTD_ULUCU_P2P:
                        
                        #ifdef MODULE_SUPPORT_ULUCU
						pbuf = &recv_buf[1];
						memset(&ulu_info,0,sizeof(Dev_SN_Info));
						memcpy(&ulu_info,pbuf,sizeof(Dev_SN_Info));
						memcpy(factory.mac, ulu_info.MAC, 18);
						ret = save_info_to_mtd_reserve(MTD_ULUCU_P2P,&ulu_info, sizeof(Dev_SN_Info));
						delete_path(ULU_CONF_DEVICEID_PATH);
                        #endif
						break;
					case MTD_DANA_P2P:
						memset(&dana_info,0,sizeof(struct Dana_info));

						pbuf = &recv_buf[1];
						memcpy(&dana_info,pbuf,sizeof(struct Dana_info ));

						printf("length is %d++++++++++++++++++\n",dana_info.file_len);
						ret = save_info_to_mtd_reserve(MTD_DANA_P2P, &dana_info,sizeof(struct Dana_info));
						break;
					case MTD_TUTK_P2P:
						pbuf = &recv_buf[1];
						memset(&yunni_info, 0, sizeof(yunni_info));
						memcpy(&yunni_info,pbuf,sizeof(struct device_info_mtd));
						memcpy(factory.mac, yunni_info.eth_mac_addr,MAX_STR_LEN_20);
						ret = save_info_to_mtd_reserve(MTD_TUTK_P2P, &yunni_info, sizeof(yunni_info));
						break;
                    case MTD_GB28181_INFO:
                        pbuf = &recv_buf[1];
                        memset(&yunni_info, 0, sizeof(yunni_info));
                        memcpy(&yunni_info,pbuf,sizeof(struct device_info_mtd));
                        memcpy(factory.mac, yunni_info.eth_mac_addr,MAX_STR_LEN_20);
                        ret = save_info_to_mtd_reserve(MTD_GB28181_INFO, &yunni_info, sizeof(yunni_info));
                        break;
					case MTD_GOKE_INFO:
						//save_info_to_mtd_reserve(MTD_GOKE_INFO, &factory, sizeof(struct goke_factory));
						break;
                    case MTD_KEY_INFO:
                        pbuf = &recv_buf[1];
                        printf("MTD_KEY_INFO key:%s\n", pbuf);
                        IPCsearch_set_p2p_info(MTD_KEY_INFO, pbuf, NULL);
                        break;
					default:
						break;
				}
				send_buf[0] = 0x44;
				send_buf[1] = 0x48;
				if(ret == 0)
				{
					send_buf[2] = 0x05;
					send_buf[3] = 0x01;
				}
				else
				{
					send_buf[2] = 0x05;
					send_buf[3] = 0x02;
				}
				ret = send(sock,send_buf,sizeof(send_buf),0);
				if(ret  != sizeof(send_buf))
				{
					PRINT_ERR("Send p2p_info set-info error!");
					close(sock);
					return -1;
				}
				close(sock);
				//netcam_timer_add_task2(netcam_sys_operation,1,SDK_FALSE,SDK_FALSE,0,(void*)SYSTEM_OPERATION_REBOOT);
				break;
		    default:
				PRINT_INFO("cmd:0x%x unknow\n ", cmd_code);
	        	break;
		}
	}
	if(sock > 0)
	{
		FD_CLR(sock, &fdr);
		close(sock);
	}
	return 0;
}

static int UpCreateTcpSock(char *ipaddr, int port)
{
	int ret;
	int sockfd;
	in_addr_t s_addr;
	int reuseaddr = 1;
	struct sockaddr_in localaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		return -1;

	if (ipaddr && ipaddr[0] && inet_pton(AF_INET, ipaddr, &s_addr) != 1) {
		goto fail;
	} else {
		s_addr = htonl(INADDR_ANY);
	}

	memset(&localaddr, 0, sizeof(localaddr));
	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(port);
	localaddr.sin_addr.s_addr = s_addr;

	/* Create socket for listening connections. */
	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
	if (ret < 0)
		goto fail;

	ret = bind(sockfd, (struct sockaddr *)&localaddr, sizeof(localaddr));
	if (ret < 0)
		goto fail;

	ret = listen(sockfd, 20);
	if (ret < 0)
		goto fail;

	return sockfd;

fail:
	close(sockfd);
	return -1;
}

void * ipcsearch_thread(void *param)
{

	int ret           = 0;
	int fSockSearchfd = 0;
	int listenfd = 0 , connectfd ;
	int fd_max;
	fd_set readfd;

	struct timeval timeout;
	struct sockaddr_in addr;
	int sin_size;
	int recvLen = 0;
	char bufRecv[GK_CMS_BUFLEN] = {0};

	// 广播地址
	struct sockaddr_in from;

	int len = 0;

	DEBUG_PRT("in tool_discovery_thread\n");
    pthread_detach(pthread_self());
	//sdk_SetThreadName("ipcsearch_thread");
	sdk_sys_thread_set_name("ipcsearch_thread");
	g_search_thread_run = 1;

	listenfd = UpCreateTcpSock(NULL, UPGRADE_PORT);
	if (listenfd < 0) {
		PRINT_ERR("create listen socket fail");
		return NULL;
	}
	//SOCK_SET_NONBLOCKING(listenfd);
	int flags = fcntl(listenfd,F_GETFL,0);
	fcntl(listenfd,F_SETFL,flags|O_NONBLOCK);
	while (1)
	{
		if(g_IphasVaild == 0&& 1 == g_search_thread_run)
		{
			DEBUG_PRT("ipcsearch_thread start g_IphasVaild:%d, g_search_thread_run:%d \n", g_IphasVaild, g_search_thread_run);
			/* create listen the search message. */
			//search_socket = create_ipcsearch_sock();
			fSockSearchfd = CreateBroadcastSock(SEARCH_BROADCAST_RECV_PORT);
			if(fSockSearchfd < 0)
			{
				ERROR_PRT("create ipcsearch_thread_socket error.\n");
				g_search_thread_run = 0;
				return NULL;
			}
			DEBUG_PRT("ipcsearch_socket id:%d \n", fSockSearchfd);
			g_IphasVaild = 1;
		}

		if(0 == g_search_thread_run)
		{
		    break;
		}

		FD_ZERO(&readfd);
		FD_SET(fSockSearchfd, &readfd);
		fd_max= fSockSearchfd;

		timeout.tv_sec = 3;
		timeout.tv_usec = 0;
		//FD_ZERO(&readfd);
		FD_SET(listenfd, &readfd);
		if(listenfd > fd_max)
			fd_max = listenfd;
		ret = select(fd_max + 1, &readfd, NULL, NULL, &timeout);

		if(0 == ret)
			continue;
		else if(ret < 0){
			break;
		}
		sin_size = sizeof(struct sockaddr_in);
		if(FD_ISSET(fSockSearchfd, &readfd))
		{
			// 初始化置0
			memset(bufRecv, 0, sizeof(bufRecv));
			bzero(&from, sizeof(struct sockaddr_in));

			//接收发来的消息
			len = sizeof(struct sockaddr_in);
			printf("==============Begin receive serarch data===================\n");
			recvLen = recvfrom(fSockSearchfd, bufRecv, sizeof(bufRecv)-1, 0, (struct sockaddr *)&from, (socklen_t *)&len);
			if (recvLen < 0) {
			    PRINT_ERR("recvfrom error:%s\n", strerror(errno));
			    continue;
			}
			char ip[16] = {0};
			sprintf(ip, "%s", (char *)inet_ntoa(from.sin_addr));
			netcam_net_autotracking(ip);
			//打印接收到的地址信息

			PRINT_INFO("search from ip:%s, port:%d, len:%d", (char *)inet_ntoa(from.sin_addr), ntohs(from.sin_port), recvLen);
        	//BoardCastHandleMsg(fSockSearchfd, SEARCH_BROADCAST_SEND_PORT, bufRecv);
		printf("==========bufRecv =%s=============\n", bufRecv);
		SEARCH_CMD *testCmd = (SEARCH_CMD*)bufRecv;
    PRINT_INFO("==testCmd->dwFlag=====%x\n", testCmd->dwFlag);
    PRINT_INFO("==testCmd->dwCmd=====%x\n", testCmd->dwCmd);
            BoardCastHandleMsg(fSockSearchfd, ntohs(from.sin_port), bufRecv);
		}
		if(FD_ISSET(listenfd, &readfd))
		{
			if ((connectfd = accept(listenfd, (struct sockaddr *)&addr, (socklen_t *) & sin_size)) == -1) {
				PRINT_ERR("Server: accept failed%s\n", strerror(errno));
				continue;
			}
			else
			{
				PRINT_INFO("accept session_socket:%d, remote ip:%s, remote port:%d\n",
			            connectfd, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
				UpgradeDealCmdDispatch(connectfd);
				//CreateDetachThread(UpgradeSessionThread, (void *)connectfd, NULL);
			}
		}

	}
	if(fSockSearchfd > 0)
	{
		close(fSockSearchfd);
		close(listenfd);
	}

	PRINT_INFO("ipc_discovery_thread end\n");
	return NULL;
}

void P2PName_add(int Type)
{
	PRINT_INFO("+++++++++++p2p_type is %d!!!++++++\n",Type);
	P2P_sup_info_s.P2P_Type[P2P_sup_info_s.P2P_Count] = Type;
	P2P_sup_info_s.P2P_Count++;
}

int IPCsearch_init(void)
{
	int ret = 0;
	pthread_t thread_id;

	PRINT_INFO("Search Tool init\n");

	ret = pthread_create(&thread_id, NULL, ipcsearch_thread, NULL);
	if(ret < 0)
	{
		PRINT_ERR("create tool_discovery_thread failed! %s\n", strerror(errno));
		return -1;
	}
	return 0;
}

int IPCsearch_exit()
{
	g_search_thread_run = 0;
	usleep(200000);
	PRINT_INFO("descovery_deinit...\n");
	return 0;
}

int BoardCastLoop(int fSockSearchfd)
{
    int recvLen = 0;
    char bufRecv[GK_CMS_BUFLEN] = {0};
    // 广播地址
    struct sockaddr_in from;

    int len = 0;
    struct timeval timeout;
    fd_set readfd; //读文件描述符集合
    int ret = 0;
    while (1) {
        if (0 == g_search_thread_run) {
            PRINT_INFO("bc, g_cms_pthread_run is 0\n");
            break;
        }
        timeout.tv_sec = 5;  //超时时间为5秒
        timeout.tv_usec = 0;
        //文件描述符清0
        FD_ZERO(&readfd);
        //将套接字文件描述符加入到文件描述符集合中
        FD_SET(fSockSearchfd, &readfd);

        ret = select(fSockSearchfd + 1, &readfd, NULL, NULL, &timeout);
		if (ret <= 0) {
			//PRINT_INFO("cms select timeout\n");
			continue;
		}

        if (FD_ISSET(fSockSearchfd, &readfd) == 0) {
			PRINT_INFO("cms select set error\n");

            continue;
        }
    	// 初始化置0
    	memset(bufRecv, 0, sizeof(bufRecv));
    	bzero(&from, sizeof(struct sockaddr_in));

    	//接收发来的消息
    	len = sizeof(struct sockaddr_in);
    	PRINT_INFO("-------------Begin receive serarch data--------------------\n");
    	recvLen = recvfrom(fSockSearchfd, bufRecv, sizeof(bufRecv)-1, 0, (struct sockaddr *)&from, (socklen_t *)&len);
    	if (recvLen < 0) {
    	    PRINT_ERR("recvfrom error:%s\n", strerror(errno));
    	    continue;
    	}
    	//打印接收到的地址信息
        PRINT_INFO("search from ip:%s, port:%d ", (char *)inet_ntoa(from.sin_addr), ntohs(from.sin_port));
        BoardCastHandleMsg(fSockSearchfd, SEARCH_BROADCAST_SEND_PORT, bufRecv);
    }
    PRINT_INFO("BoardCastLoop end.\n");
    return 0;
}

static int Search_fill_bcast_param(PBCASTPARAM pbcast_param)
{
	int ret = 0;
	char wifi_name[SDK_ETHERSTR_LEN] = {0};
	char wifi_mac[6];
	char use_wifi_mac;
	ST_SDK_NETWORK_ATTR     net_attr_local,net_attr_wifi;
	struct goke_factory  deviceInfo;
    unsigned int startTime = search_get_startup_time_seconds();

	if (NULL == pbcast_param)
	{
		ERROR_PRT("pbcast_param:%p is NULL! \n", pbcast_param);
		return -1;
	}
	//get net info
	memset(&net_attr_local, 0, sizeof(ST_SDK_NETWORK_ATTR));
	strncpy(net_attr_local.name, LAN_DEV, SDK_ETHERSTR_LEN-1);
	ret = netcam_net_get(&net_attr_local);
	if(ret != 0)
	{
		PRINT_ERR("Faile to Get net_attr_local.");
		return -1;
	}

	//get wifi info
	memset(&net_attr_wifi, 0, sizeof(ST_SDK_NETWORK_ATTR));
	if(netcam_net_wifi_get_devname() != NULL && runNetworkCfg.wifi.enable)
	{
		strcpy(wifi_name, netcam_net_wifi_get_devname());
		strncpy(net_attr_wifi.name, wifi_name, SDK_ETHERSTR_LEN-1);
		ret = netcam_net_get(&net_attr_wifi);
		if(ret != 0)
		{
			PRINT_ERR("Faile to Get net_attr_wifi.");
			//return -1;
		}
	}
	ret = netcam_net_get_detect(LAN_DEV);
	if(0 == ret)
	{
		use_wifi_mac = 0;
		pbcast_param->dhcp = net_attr_local.dhcp;
		strncpy(pbcast_param->szIpAddr,  net_attr_local.ip,        sizeof(pbcast_param->szIpAddr));
		strncpy(pbcast_param->szMask,    net_attr_local.mask,      sizeof(pbcast_param->szMask));
		strncpy(pbcast_param->szGateway, net_attr_local.gateway,   sizeof(pbcast_param->szGateway));
		strncpy(pbcast_param->szDns1,    net_attr_local.dns1,      sizeof(pbcast_param->szDns1));
		strncpy(pbcast_param->szDns2,    net_attr_local.dns2,      sizeof(pbcast_param->szDns2));
	}
	else
	{
		if(netcam_net_wifi_get_devname() != NULL && runNetworkCfg.wifi.enable)
		{
			if((ret = netcam_net_get_detect(wifi_name)))
			{
				PRINT_ERR("Detect net device LAN_DEV:%s  WIFI_DEV%s error!\n", LAN_DEV, wifi_name);
				return -1;
			}
			use_wifi_mac = 1;
			pbcast_param->dhcp = net_attr_wifi.dhcp;
			strncpy(pbcast_param->szIpAddr,  net_attr_wifi.ip,        sizeof(pbcast_param->szIpAddr));
			strncpy(pbcast_param->szMask,    net_attr_wifi.mask,      sizeof(pbcast_param->szMask));
			strncpy(pbcast_param->szGateway, net_attr_wifi.gateway,   sizeof(pbcast_param->szGateway));
			strncpy(pbcast_param->szDns1,    net_attr_wifi.dns1,      sizeof(pbcast_param->szDns1));
			strncpy(pbcast_param->szDns2,    net_attr_wifi.dns2,      sizeof(pbcast_param->szDns2));
		}
		else
			return -1;
	}



	mac_str_to_bin(net_attr_local.mac, pbcast_param->szMacAddr);
	mac_str_to_bin(net_attr_wifi.mac, wifi_mac);

	pbcast_param->nPort = UPGRADE_PORT;//net_attr.httpPort;

	//yunni_p2p_get_device_info_load(&deviceInfo);
	memset(&deviceInfo, 0, sizeof(struct goke_factory));
	if(load_info_to_mtd_reserve(MTD_GOKE_INFO,  &deviceInfo,sizeof(struct goke_factory)))
	{
		PRINT_ERR("fail to get MTD_GOKE_INFO\n");
	}


	strncpy(pbcast_param->other1,wifi_mac,sizeof(wifi_mac));
	pbcast_param->other1[sizeof(wifi_mac)] = use_wifi_mac;
    memcpy(&(pbcast_param->other1[sizeof(wifi_mac)+1]),&startTime,sizeof(unsigned int));// send system up time,seconds


	strncpy(pbcast_param->szDevName,  runSystemCfg.deviceInfo.deviceName,      sizeof(pbcast_param->szDevName));
	strncpy(pbcast_param->sysver,     runSystemCfg.deviceInfo.firmwareVersion, sizeof(pbcast_param->sysver));
	strncpy(pbcast_param->appver,     runSystemCfg.deviceInfo.softwareVersion, sizeof(pbcast_param->appver));
	strncpy(pbcast_param->type_name,sdk_cfg.name,sizeof(pbcast_param->type_name));
	strncpy(pbcast_param->upgradeVersion,runSystemCfg.deviceInfo.upgradeVersion,sizeof(pbcast_param->upgradeVersion));
	pbcast_param->upgrade_status = netcam_get_update_status();
	PRINT_INFO("pbcast_param->upgrade_status IS %x,size is %d,pbcast_param->upgrade_status is %x\n",netcam_get_update_status(),sizeof(int),pbcast_param->upgrade_status );
	PRINT_INFO("runSystemCfg.deviceInfo.upgradeVersion %s\n",runSystemCfg.deviceInfo.upgradeVersion);
	Search_print_bcast_param(pbcast_param);

	return 0;
}

static int Search_print_bcast_ddns(PBCASTDDNS pbcast_ddns)
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
static int Search_fill_bcast_ddns(PBCASTDDNS pbcast_ddns)
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

	Search_print_bcast_ddns(pbcast_ddns);

	return 0;
}

int BoardCastHandleMsg(int send_sock, int send_port, void* pBuf)
{
    PRINT_INFO("bc get search.\n");
    SEARCH_CMD *pSearchCmd = (SEARCH_CMD*)pBuf;
    PRINT_INFO("==pSearchCmd->dwFlag=====%x\n", pSearchCmd->dwFlag);
    PRINT_INFO("==START_CODE =====%x\n", START_CODE);
    if((int)pSearchCmd->dwFlag != 18500)
    {
    	PRINT_INFO("==START_CODE ERROR!=====%x\n", (unsigned int)pSearchCmd->dwFlag);
        return 0;
    }
    int type = 0;
    int ret = 0;
    char send_buf[RECV_BUF_SIZE] = {0};

    char *pTmp =  (char*)pBuf + sizeof(SEARCH_CMD);
    
#ifdef MODULE_SUPPORT_ULUCU
    static Dev_SN_Info ulu_info;
#endif
	struct device_info_mtd yunni_info;
	struct Dana_info dana_info;
	printf("==========pSearchCmd->dwCmd = %x\n",pSearchCmd->dwCmd);
	pSearchCmd->dwCmd = 257;
	printf("==========pSearchCmd->dwCmd = %x\n",pSearchCmd->dwCmd);
    switch(pSearchCmd->dwCmd)
    {
        case CMD_GET:
        {
            JBNV_SERVER_MSG_DATA_EX msg_data = {0};
            msg_data.dwSize = sizeof(JBNV_SERVER_MSG_DATA_EX);
            printf("dwSize:%lu\n", msg_data.dwSize);
            msg_data.dwPackFlag = SERVER_PACK_FLAG;

            ST_SDK_NETWORK_ATTR net_attr;
            memset(&net_attr, 0, sizeof(ST_SDK_NETWORK_ATTR));

            int ret = netcam_net_get_detect(runNetworkCfg.lan.netName);
            if (ret != 0) {
                PRINT_INFO("eth0 is not running.");
                strcpy(net_attr.name, runNetworkCfg.wifi.netName);

                if(netcam_net_get(&net_attr) != 0) {
                    PRINT_ERR("get network config error.\n");
                    return -1;
                }
                PRINT_INFO("bc -- wifi ip :%s & static ip :%s\n", net_attr.ip, runNetworkCfg.wifi.ip);
                strncpy(msg_data.jbServerPack.jspack.szIp, (char *)net_attr.ip, DMS_MAX_IP_LENGTH);
            } else {
                PRINT_INFO("eth0 is ok.");
                strcpy(net_attr.name, runNetworkCfg.lan.netName);

                if(netcam_net_get(&net_attr) != 0) {
                PRINT_ERR("get network config error.\n");
                return -1;
                }
                PRINT_INFO("bc -- lan ip :%s & static ip :%s\n", net_attr.ip, runNetworkCfg.lan.ip);
                strncpy(msg_data.jbServerPack.jspack.szIp, (char *)net_attr.ip, DMS_MAX_IP_LENGTH);
		printf("========msg_data.jbServerPack.jspack.szIp===========%s\n", msg_data.jbServerPack.jspack.szIp);
            }
            //add mac
            gk_cms_change_mac(msg_data.jbServerPack.bMac, net_attr.mac);
            printf("mac: %02x-%02x-%02x-%02x-%02x-%02x\n", msg_data.jbServerPack.bMac[0], msg_data.jbServerPack.bMac[1],
                                msg_data.jbServerPack.bMac[2], msg_data.jbServerPack.bMac[3],
                                msg_data.jbServerPack.bMac[4], msg_data.jbServerPack.bMac[5]);


            strncpy(msg_data.jbServerPack.csServerNo, runSystemCfg.deviceInfo.serialNumber, sizeof(runSystemCfg.deviceInfo.serialNumber));
            printf("serialNumber:%s, csServerNo: %s \n", runSystemCfg.deviceInfo.serialNumber, msg_data.jbServerPack.csServerNo);
            msg_data.jbServerPack.jspack.wMediaPort = GK_CMS_TCP_LISTEN_PORT;
            msg_data.jbServerPack.jspack.wWebPort = runNetworkCfg.port.httpPort;
            msg_data.jbServerPack.jspack.wChannelCount = 1;

            //strncpy(msg_data.jbServerPack.jspack.szServerName, runSystemCfg.deviceInfo.deviceName, sizeof(runSystemCfg.deviceInfo.deviceName));
            strncpy(msg_data.jbServerPack.jspack.szServerName, runChannelCfg.channelInfo[0].osdChannelName.text, sizeof(msg_data.jbServerPack.jspack.szServerName));
            printf("szServerName:%s\n", msg_data.jbServerPack.jspack.szServerName);
            //msg_data.jbServerPack.jspack.dwDeviceType = (DWORD)strtoul(runSystemCfg.deviceInfo.deviceType, NULL, 0);
            msg_data.jbServerPack.jspack.dwDeviceType = 1;
            //msg_data.jbServerPack.jspack.dwServerVersion = (DWORD)strtoul(runSystemCfg.deviceInfo.softwareVersion, NULL, 0);
            msg_data.jbServerPack.jspack.dwServerVersion = 1;
            msg_data.jbServerPack.jspack.wChannelStatic = 0;
            msg_data.jbServerPack.jspack.wSensorStatic = 0;
            msg_data.jbServerPack.jspack.wAlarmOutStatic = 0;

            //从广播地址发送消息
            usleep(500000); //睡眠500毫秒，以免nvr搜索数据太多，溢出缓冲区，保证nvr每次都能搜到设备
            BoardCastSendTo(send_sock, SEARCH_BROADCAST_ADDR, send_port, (char *)&msg_data, sizeof(JBNV_SERVER_MSG_DATA_EX));

            usleep(500000); //睡眠500毫秒，以免nvr搜索数据太多，溢出缓冲区，保证nvr每次都能搜到设备
            BoardCastSendTo(send_sock, SEARCH_BROADCAST_ADDR, send_port, (char *)&msg_data, sizeof(JBNV_SERVER_MSG_DATA_EX));
        }
        break;
        case CMD_PHONE_SEARCH:
        {
            JBNV_PHONE_WIFI_INFO *wifi_info = (JBNV_PHONE_WIFI_INFO*)(pBuf + sizeof(SEARCH_CMD));
            LOG_INFO("from bc, wifi ssid:<%s>, psd:<%s>", wifi_info->ssid, wifi_info->psd);


            #ifdef AP_STA_SUPPORT

            static int SETTING_WIFI = 0;
            ST_SDK_NETWORK_ATTR net_attr2;
            WIFI_LINK_INFO_t linkInfo;

			char *WifiName = NULL;

            //有ssid的情况(可能ap模式设置wifi)，才走此路,无ssid时可能就是广播搜索功能
            if(wifi_info->ssid[0]!= 0)
            {
    			WifiName = netcam_net_wifi_get_devname();

    			strncpy(net_attr2.name, WifiName, sizeof(net_attr2.name));
                PRINT_INFO("AP_STA_SUPPORT SET WIFI\n");
                memset(&linkInfo,0,sizeof(WIFI_LINK_INFO_t));
                network_wifi_get_connect_info(&linkInfo);
                if(linkInfo.linkStatus == WIFI_CONNECT_OK &&
                    strcmp(linkInfo.linkEssid,wifi_info->ssid) == 0 &&
                    strcmp(linkInfo.linkPsd,wifi_info->psd) == 0 )
                {
                    PRINT_INFO("xxxx allready connected\n");
                    //break;
                }
                else
                {

                    memset(&linkInfo,0,sizeof(WIFI_LINK_INFO_t));
        			linkInfo.isConnect = 1;
                    linkInfo.linkScurity = WIFI_ENC_WPAPSK_AES;
        			strncpy(linkInfo.linkEssid,wifi_info->ssid,sizeof(linkInfo.linkEssid));
        			strncpy(linkInfo.linkPsd,wifi_info->psd,sizeof(linkInfo.linkPsd));
        			ret = netcam_net_setwifi(NETCAM_WIFI_STA,linkInfo);

                    if (ret <0)
                    {
                        sleep(5);

                        //netcam_audio_hint(SYSTEM_AUDIO_HINT_LINK_FAIL);xqq
                        printf("xxxx failed to set sta wifi parament ,back to AP mode!\n");
                        netcam_net_setwifi(NETCAM_WIFI_AP,linkInfo);
                    }
                    else
                    {
                        printf("xxxx set wifi success\n");
        			    netcam_net_get(&net_attr2);
                        //netcam_audio_hint(SYSTEM_AUDIO_HINT_USE_WIRELESS); xqq
        		        netcam_audio_ip(net_attr2.ip);
                    }
                }
            }
            #else

            PRINT_INFO(" SET WIFI\n");
            //todo save ssid & psd to cfg file.
            if(wifi_info->ssid[0]!= 0)
            {
                int i;
                g_link_info.enable = 1;
                //g_link_info.isConnect = 1;

                for(i = 0; i < strlen(wifi_info->ssid) && i < MAX_WIFI_NAME; i++)
                {
                    if(wifi_info->ssid[i]== '\r' || wifi_info->ssid[i]== '\n')
                    {
                        wifi_info->ssid[i]= 0;
                        break;
                    }
                }

                for(i = 0; i < strlen(wifi_info->psd)&& i < MAX_WIFI_NAME; i++)
                {
                    if(wifi_info->psd[i]== '\r' || wifi_info->psd[i]== '\n')
                    {
                        wifi_info->psd[i]= 0;
                        break;
                    }
                }
                g_link_info.linkScurity = WIFI_ENC_WEP_64_ASSII;
                strncpy(g_link_info.linkEssid,wifi_info->ssid, sizeof(g_link_info.linkEssid));
                strncpy(g_link_info.linkPsd,wifi_info->psd, sizeof(g_link_info.linkPsd));
                CREATE_WORK(wifiLINKTaskOne, EVENT_TIMER_WORK, (WORK_CALLBACK)search_wifi_connect);
                INIT_WORK(wifiLINKTaskOne, COMPUTE_TIME(0,0,0,1,0), &g_link_info); // running one times
                SCHEDULE_DEFAULT_WORK(wifiLINKTaskOne);
            }
            #endif

            JBNV_SERVER_MSG_DATA_EX msg_data = {0};
            msg_data.dwSize = sizeof(JBNV_SERVER_MSG_DATA_EX);
            printf("dwSize:%lu\n", msg_data.dwSize);
            msg_data.dwPackFlag = SERVER_PACK_FLAG;

            ST_SDK_NETWORK_ATTR net_attr;
            memset(&net_attr, 0, sizeof(ST_SDK_NETWORK_ATTR));

            int ret = netcam_net_get_detect(runNetworkCfg.lan.netName);
            if (ret != 0) {
                PRINT_INFO("eth0 is not running.");
                strcpy(net_attr.name, runNetworkCfg.wifi.netName);

                if(netcam_net_get(&net_attr) != 0) {
                    PRINT_ERR("get network config error.\n");
                    return -1;
                }
                PRINT_INFO("bc -- wifi ip :%s & static ip :%s\n", net_attr.ip, runNetworkCfg.wifi.ip);
                strncpy(msg_data.jbServerPack.jspack.szIp, (char *)net_attr.ip, DMS_MAX_IP_LENGTH);
            } else {
                PRINT_INFO("eth0 is ok.");
                strcpy(net_attr.name, runNetworkCfg.lan.netName);

                if(netcam_net_get(&net_attr) != 0) {
                PRINT_ERR("get network config error.\n");
                return -1;
                }
                PRINT_INFO("bc -- lan ip :%s & static ip :%s\n", net_attr.ip, runNetworkCfg.lan.ip);
                strncpy(msg_data.jbServerPack.jspack.szIp, (char *)net_attr.ip, DMS_MAX_IP_LENGTH);
            }
            //add mac
            gk_cms_change_mac(msg_data.jbServerPack.bMac, net_attr.mac);
            printf("mac: %02x-%02x-%02x-%02x-%02x-%02x\n", msg_data.jbServerPack.bMac[0], msg_data.jbServerPack.bMac[1],
                                msg_data.jbServerPack.bMac[2], msg_data.jbServerPack.bMac[3],
                                msg_data.jbServerPack.bMac[4], msg_data.jbServerPack.bMac[5]);


            strncpy(msg_data.jbServerPack.csServerNo, runSystemCfg.deviceInfo.serialNumber, sizeof(runSystemCfg.deviceInfo.serialNumber));
            printf("serialNumber:%s, csServerNo: %s \n", runSystemCfg.deviceInfo.serialNumber, msg_data.jbServerPack.csServerNo);
            msg_data.jbServerPack.jspack.wMediaPort = GK_CMS_TCP_LISTEN_PORT;
            msg_data.jbServerPack.jspack.wWebPort = runNetworkCfg.port.httpPort;
            msg_data.jbServerPack.jspack.wChannelCount = 1;

            //strncpy(msg_data.jbServerPack.jspack.szServerName, runSystemCfg.deviceInfo.deviceName, sizeof(runSystemCfg.deviceInfo.deviceName));
            strncpy(msg_data.jbServerPack.jspack.szServerName, runChannelCfg.channelInfo[0].osdChannelName.text, sizeof(msg_data.jbServerPack.jspack.szServerName));
            printf("szServerName:%s\n", msg_data.jbServerPack.jspack.szServerName);
            //msg_data.jbServerPack.jspack.dwDeviceType = (DWORD)strtoul(runSystemCfg.deviceInfo.deviceType, NULL, 0);


            struct device_info_mtd info;
            load_info_to_mtd_reserve(MTD_TUTK_P2P, &info, sizeof(info));
            //msg_data.jbServerPack.jspack.dwDeviceType = info.devType;
            msg_data.jbServerPack.jspack.dwDeviceType = runSystemCfg.deviceInfo.devType;
            //0-卡片机, 1-摇头机, 2-鱼眼摄像机.,3,门铃单向，4 门铃双向，5模组
            PRINT_INFO("device type option: 0:kpj,1:ytj ,2:overall view, 3:single door ring,4:dual door ring,5:38 model");
            PRINT_INFO("current device type:%d\n", (int)msg_data.jbServerPack.jspack.dwDeviceType);
            //msg_data.jbServerPack.jspack.dwServerVersion = (DWORD)strtoul(runSystemCfg.deviceInfo.softwareVersion, NULL, 0);
            msg_data.jbServerPack.jspack.dwServerVersion = 1;
            msg_data.jbServerPack.jspack.wChannelStatic = 0;
            msg_data.jbServerPack.jspack.wSensorStatic = 0;
            msg_data.jbServerPack.jspack.wAlarmOutStatic = 0;

            //从广播地址发送消息
            usleep(500000); //睡眠500毫秒，以免nvr搜索数据太多，溢出缓冲区，保证nvr每次都能搜到设备
            BoardCastSendTo(send_sock, SEARCH_BROADCAST_ADDR, send_port, (char *)&msg_data, sizeof(JBNV_SERVER_MSG_DATA_EX));

            usleep(500000); //睡眠500毫秒，以免nvr搜索数据太多，溢出缓冲区，保证nvr每次都能搜到设备
            BoardCastSendTo(send_sock, SEARCH_BROADCAST_ADDR, send_port, (char *)&msg_data, sizeof(JBNV_SERVER_MSG_DATA_EX));
        }
        break;

        case CMD_MOJING_SEARCH:
        {
            SEARCH_CMD_RET retData;
            JBNV_PHONE_WIFI_INFO *wifi_info = (JBNV_PHONE_WIFI_INFO*)(pBuf + sizeof(SEARCH_CMD));
            LOG_INFO("from bc2, wifi ssid:<%s>, psd:<%s>", wifi_info->ssid, wifi_info->psd);

            if (runNetworkCfg.wireless.mode != 0)
            {
                LOG_INFO("not ap mode, return;\n");
                return 0;
            }

            if (strlen(wifi_info->devId) > 0)
            {
                int ret = load_info_to_mtd_reserve(MTD_TUTK_P2P, &yunni_info, sizeof(struct device_info_mtd));
                if((ret < 0))
                {
                    LOG_INFO("Fail to load gb28181 id from flash!\n");
                }
                else
                {
                    if (strcmp(yunni_info.device_id, wifi_info->devId) != 0)
                    {
                        LOG_INFO("cur id:%s, wifi set id:%s;not the same\n", yunni_info.device_id, wifi_info->devId);
                        return 0;
                    }
                }
            }

            retData.dwFlag = START_CODE;
            retData.dwCmd = CMD_MOJING_SEARCH_RET;
            retData.dwCmdFrom = CMD_MOJING_SEARCH;
            retData.status = 1;

            BoardCastSendTo(send_sock, SEARCH_BROADCAST_ADDR, send_port, (char*)&retData, sizeof(SEARCH_CMD_RET));
            usleep(500000); //睡眠500毫秒，以免nvr搜索数据太多，溢出缓冲区，保证nvr每次都能搜到设备


            #if 0
            ST_SDK_NETWORK_ATTR net_attr2;
            WIFI_LINK_INFO_t linkInfo;

			char *WifiName = NULL;
            int wifiMode = runNetworkCfg.wireless.mode;

            memset(&retData, 0, sizeof(retData));
            //有ssid的情况(可能ap模式设置wifi)，才走此路,无ssid时可能就是广播搜索功能
            if(wifi_info->ssid[0]!= 0)
            {
    			WifiName = netcam_net_wifi_get_devname();

    			strncpy(net_attr2.name, WifiName, sizeof(net_attr2.name));
                PRINT_INFO("AP_STA_SUPPORT SET WIFI\n");
                memset(&linkInfo,0,sizeof(WIFI_LINK_INFO_t));
                network_wifi_get_connect_info(&linkInfo);
                if(linkInfo.linkStatus == WIFI_CONNECT_OK &&
                    strcmp(linkInfo.linkEssid,wifi_info->ssid) == 0 &&
                    strcmp(linkInfo.linkPsd,wifi_info->psd) == 0 )
                {
                    PRINT_INFO("xxxx allready connected\n");
                    retData.status = 1;
                    //break;
                }
                else
                {

                    memset(&linkInfo,0,sizeof(WIFI_LINK_INFO_t));
        			linkInfo.isConnect = 1;
                    linkInfo.linkScurity = WIFI_ENC_WPAPSK_AES;
        			strncpy(linkInfo.linkEssid,wifi_info->ssid,sizeof(linkInfo.linkEssid));
        			strncpy(linkInfo.linkPsd,wifi_info->psd,sizeof(linkInfo.linkPsd));
        			ret = netcam_net_setwifi(NETCAM_WIFI_STA,linkInfo);

                    if (ret <0)
                    {
                        netcam_audio_hint(SYSTEM_AUDIO_HINT_LINK_FAIL);
                        if (wifiMode == 0)
                        {
                            printf("xxxx failed to set sta wifi parament ,back to AP mode!\n");
                            netcam_net_setwifi(NETCAM_WIFI_AP,linkInfo);
                        }

                        retData.status = 0;
                    }
                    else
                    {
                        printf("xxxx set wifi success\n");
        			    netcam_net_get(&net_attr2);
                        netcam_audio_hint(SYSTEM_AUDIO_HINT_USE_WIRELESS);
        		        //netcam_audio_ip(net_attr2.ip);
                        retData.status = 1;
                    }
                }
            }
            #else
            if(wifi_info->ssid[0]!= 0 && g_link_info.isConnect == 0)
            {
                int i;
                g_link_info.enable = 1;
                //g_link_info.isConnect = 1;

                for(i = 0; i < strlen(wifi_info->ssid) && i < MAX_WIFI_NAME; i++)
                {
                    if(wifi_info->ssid[i]== '\r' || wifi_info->ssid[i]== '\n')
                    {
                        wifi_info->ssid[i]= 0;
                        break;
                    }
                }

                for(i = 0; i < strlen(wifi_info->psd)&& i < MAX_WIFI_NAME; i++)
                {
                    if(wifi_info->psd[i]== '\r' || wifi_info->psd[i]== '\n')
                    {
                        wifi_info->psd[i]= 0;
                        break;
                    }
                }
                g_link_info.linkScurity = WIFI_ENC_WEP_64_ASSII;
                strncpy(g_link_info.linkEssid,wifi_info->ssid, sizeof(g_link_info.linkEssid));
                strncpy(g_link_info.linkPsd,wifi_info->psd, sizeof(g_link_info.linkPsd));
                #if 1
                CREATE_WORK(wifiLINKTaskOne, EVENT_TIMER_WORK, (WORK_CALLBACK)search_wifi_connect);
                INIT_WORK(wifiLINKTaskOne, COMPUTE_TIME(0,0,0,1,0), &g_link_info);
                SCHEDULE_DEFAULT_WORK(wifiLINKTaskOne);
                #else
                netcam_timer_add_task2(search_wifi_connect, NETCAM_TIMER_ONE_SEC, SDK_TRUE, SDK_TRUE, NULL, NULL);
                #endif
            }
            #endif

            //从广播地址发送消息
            usleep(500000); //睡眠500毫秒，以免nvr搜索数据太多，溢出缓冲区，保证nvr每次都能搜到设备
            BoardCastSendTo(send_sock, SEARCH_BROADCAST_ADDR, send_port, (char*)&retData, sizeof(SEARCH_CMD_RET));

        }
        break;
        case CMD_MOJING_SEARCH_RESET:
        {
            int checkIdOk = 1;
            SEARCH_CMD_RET retData;
            SEARCH_CMD_RESET *resetInfo = (SEARCH_CMD_RESET*)(pBuf);
            LOG_INFO("from bc, reset id:<%s>", resetInfo->devId);

            struct device_info_mtd yunni_info;
            memset(&yunni_info,0,sizeof(struct device_info_mtd));
            int ret = load_info_to_mtd_reserve(MTD_TUTK_P2P, &yunni_info, sizeof(struct device_info_mtd));
            if((ret < 0) || (yunni_info.device_id[0] == 0))
            {
                printf("Fail to load gb28181 id from flash!\n");
                checkIdOk = 0;
            }

            if (checkIdOk && strcmp(resetInfo->devId, yunni_info.device_id) != 0)
            {
                checkIdOk = 0;
            }

            retData.dwFlag = START_CODE;
            retData.dwCmd = CMD_MOJING_SEARCH_RET;
            retData.dwCmdFrom = CMD_MOJING_SEARCH_RESET;
            retData.status = checkIdOk;

            //从广播地址发送消息
            usleep(500000); //睡眠500毫秒，以免nvr搜索数据太多，溢出缓冲区，保证nvr每次都能搜到设备
            BoardCastSendTo(send_sock, SEARCH_BROADCAST_ADDR, send_port, (char*)&retData, sizeof(SEARCH_CMD_RET));

            usleep(500000); //睡眠500毫秒，以免nvr搜索数据太多，溢出缓冲区，保证nvr每次都能搜到设备
            BoardCastSendTo(send_sock, SEARCH_BROADCAST_ADDR, send_port, (char*)&retData, sizeof(SEARCH_CMD_RET));
            if (checkIdOk)
            {
                printf("start to reset..\n");
                sleep(1);
                netcam_sys_operation((void*)0, (void*)SYSTEM_OPERATION_HARD_DEFAULT);
            }
            else
            {
                printf("id %s is not the same:%s\n", resetInfo->devId, yunni_info.device_id);
            }
        }
        break;
        case CMD_MOJING_GET_LIST_GET:
        {
            SEARCH_CMD_MOJING_GET msg_data = {0};
            msg_data.dwFlag = START_CODE;
            msg_data.dwCmd = CMD_MOJING_GET_LIST_RET;

            ST_SDK_NETWORK_ATTR net_attr;
            memset(&net_attr, 0, sizeof(ST_SDK_NETWORK_ATTR));

            int ret = netcam_net_get_detect(runNetworkCfg.lan.netName);
            if (ret != 0) {
                PRINT_INFO("eth0 is not running.");
                strcpy(net_attr.name, runNetworkCfg.wifi.netName);

                if(netcam_net_get(&net_attr) != 0) {
                    PRINT_ERR("get network config error.\n");
                    return -1;
                }
                PRINT_INFO("bc -- wifi ip :%s & static ip :%s\n", net_attr.ip, runNetworkCfg.wifi.ip);
                strncpy(msg_data.ip, (char *)net_attr.ip, DMS_MAX_IP_LENGTH);
            } else {
                PRINT_INFO("eth0 is ok.");
                strcpy(net_attr.name, runNetworkCfg.lan.netName);

                if(netcam_net_get(&net_attr) != 0) {
                PRINT_ERR("get network config error.\n");
                return -1;
                }
                PRINT_INFO("bc -- lan ip :%s & static ip :%s\n", net_attr.ip, runNetworkCfg.lan.ip);
                strncpy(msg_data.ip, (char *)net_attr.ip, DMS_MAX_IP_LENGTH);
            }
            //add mac
            strncpy(msg_data.bMac, (char*)net_attr.mac, SDK_MACSTR_LEN);
            printf("mac: %02x-%02x-%02x-%02x-%02x-%02x\n", msg_data.bMac[0], msg_data.bMac[1],
                                msg_data.bMac[2], msg_data.bMac[3],
                                msg_data.bMac[4], msg_data.bMac[5]);

#ifdef MODULE_SUPPORT_GB28181
            strcpy(msg_data.devId, runGB28181Cfg.DeviceUID);
#endif
            msg_data.mojingStatus = 1;
            msg_data.dnsStatus = 1;

            //从广播地址发送消息
            usleep(500000); //睡眠500毫秒，以免nvr搜索数据太多，溢出缓冲区，保证nvr每次都能搜到设备
            BoardCastSendTo(send_sock, SEARCH_BROADCAST_ADDR, send_port, (char *)&msg_data, sizeof(SEARCH_CMD_MOJING_GET));
        }
        break;

	    case CMD_GET_PC:
		ret = Search_fill_bcast_param(&g_bcast_param_s);
		if (ret < 0)
		{
			ERROR_PRT("fill_bcast_param ret:0x%x is err!\n", ret);
			return -1;
		}
		ret = Search_fill_bcast_ddns(&g_bcast_ddns_s);
		if (ret < 0)
		{
			ERROR_PRT("fill_bcast_ddns ret:0x%x is err!\n", ret);
			return -1;
		}
		Search_fill_p2p_info();
		memset(send_buf, 0x0, sizeof(send_buf));
       	send_buf[0] = 0x44;
       	send_buf[1] = 0x48;
    		send_buf[2] = 0x01;
		send_buf[3] = 0x08;
		char *p = NULL;
		p = &send_buf[4];
		memcpy(p, &g_bcast_param_s, sizeof(g_bcast_param_s));
		p = &send_buf[4 + sizeof(g_bcast_param_s)];
		memcpy(p, &g_bcast_ddns_s, sizeof(g_bcast_ddns_s));
		p = &send_buf[4+sizeof(g_bcast_param_s)+sizeof(g_bcast_ddns_s)];
		memcpy(p,&g_get_sup_p2p_info_s,sizeof(SupportP2P_INFO)*P2P_sup_info_s.P2P_Count);
		PRINT_INFO("===P2P_sup_info_s.P2P_Count===is %d=====\n",P2P_sup_info_s.P2P_Count);
		int cmd_buf_len = 4 + sizeof(g_bcast_param_s) + sizeof(g_bcast_ddns_s)+sizeof(SupportP2P_INFO)*P2P_sup_info_s.P2P_Count;
	     	usleep(500000); //睡眠500毫秒，以免nvr搜索数据太多，溢出缓冲区，保证nvr每次都能搜到设备
        BoardCastSendTo(send_sock, SEARCH_BROADCAST_ADDR, SEARCH_TOOL_BROADCAST_RECV_PORT, (char *)&send_buf, cmd_buf_len);
		BoardCastSendTo(send_sock, SEARCH_BROADCAST_ADDR, SEARCH_TOOL_BROADCAST_RECV_PORT, (char *)&send_buf, cmd_buf_len);
		BoardCastSendTo(send_sock, SEARCH_BROADCAST_ADDR, SEARCH_TOOL_BROADCAST_RECV_PORT, (char *)&send_buf, cmd_buf_len);

	 	break;
        case CMD_SET:
        {
            JBNV_SERVER_MSG_DATA_EX *pMsg_data = (JBNV_SERVER_MSG_DATA_EX*)(pBuf + sizeof(SEARCH_CMD));
            ST_SDK_NETWORK_ATTR net_attr;
            sprintf(net_attr.name,"eth0");
            LOG_INFO("_____fun:BoardCastHandleMsg szIp:%s, Mask:%ld, Gateway:%ld, dns:%ld________\n", pMsg_data->jbServerPack.jspack.szIp, pMsg_data->jbServerPack.dwNetMask,
                                                             pMsg_data->jbServerPack.dwGateway, pMsg_data->jbServerPack.dwDNS);
            net_attr.enable = 1;
            netcam_net_get(&net_attr);

            unsigned char setMac[SDK_MACSTR_LEN] = {0};
            unsigned char setMacTemp[SDK_MACSTR_LEN] = {0};
            memcpy(setMacTemp, pMsg_data->jbServerPack.bMac, 7);
            sprintf((char*)setMac,"%02X:%02X:%02X:%02X:%02X:%02X",setMacTemp[0],setMacTemp[1],setMacTemp[2],setMacTemp[3],setMacTemp[4],setMacTemp[5]);
            if(strcasecmp((char*)setMac,(char*)net_attr.mac) == 0)
            {
                net_attr.dhcp = pMsg_data->jbServerPack.bEnableDHCP;
                strncpy(net_attr.ip,pMsg_data->jbServerPack.jspack.szIp,sizeof(net_attr.ip));
                char strTemp[32] = {0};
                dwip_to_str(pMsg_data->jbServerPack.dwNetMask,strTemp);
                strncpy(net_attr.mask,strTemp,sizeof(net_attr.mask));
                dwip_to_str(pMsg_data->jbServerPack.dwGateway,strTemp);
                strncpy(net_attr.gateway,strTemp,sizeof(net_attr.gateway));
                dwip_to_str(pMsg_data->jbServerPack.dwDNS,strTemp);
                strncpy(net_attr.dns1,strTemp,sizeof(net_attr.dns1));

                if (netcam_net_set(&net_attr) == 0)
                    netcam_net_cfg_save();
            }
            break;
        }
        case CMD_GET_DS_INFO:
        {
            //徐辉app使用，避免魔镜测试一直搜索
            P2P_DEVICE_INFO_EX msg_data = {0} ;
            struct device_info_mtd info;

            msg_data.dwSize = sizeof(P2P_DEVICE_INFO_EX)-sizeof(msg_data.p2pDeviceInfo.my_key);
            printf("dwSize:%lu\n", msg_data.dwSize);
            msg_data.dwPackFlag = SERVER_PACK_FLAG;


            load_info_to_mtd_reserve(MTD_TUTK_P2P, &info, sizeof(info));

            strncpy(msg_data.p2pDeviceInfo.p2pid,info.device_id,sizeof(msg_data.p2pDeviceInfo.p2pid));
            msg_data.p2pDeviceInfo.devType = info.devType;
            msg_data.p2pDeviceInfo.netFamily = info.netFamily ;
            msg_data.p2pDeviceInfo.language = info.language ;
            msg_data.p2pDeviceInfo.odmID = info.odmID ;
            msg_data.p2pDeviceInfo.panoramaMode = info.panoramaMode ;
			msg_data.p2pDeviceInfo.serverID = netcam_net_get_icute_number();
            strncpy((char*)msg_data.p2pDeviceInfo.model,(char*)info.model,sizeof(msg_data.p2pDeviceInfo.model));
            memcpy(msg_data.p2pDeviceInfo.my_key,info.my_key,sizeof(msg_data.p2pDeviceInfo.my_key));

            int i = last_index_at(runSystemCfg.deviceInfo.upgradeVersion, '.');
            strcpy((char*)msg_data.p2pDeviceInfo.version, runSystemCfg.deviceInfo.upgradeVersion + i + 1);

            msg_data.p2pDeviceInfo.web_port = 80;//固定80
            msg_data.p2pDeviceInfo.manufacture = 0x676b647a;

            PRINT_INFO("GET ds device info:\n");
            PRINT_INFO("p2pid:%s\n",msg_data.p2pDeviceInfo.p2pid);
            #if 0
            PRINT_INFO("devType:%u\n",msg_data.p2pDeviceInfo.devType);
            PRINT_INFO("netFamily:%u\n",msg_data.p2pDeviceInfo.netFamily);
            PRINT_INFO("language:%u\n",msg_data.p2pDeviceInfo.language);
            PRINT_INFO("odmID:%u\n",msg_data.p2pDeviceInfo.odmID);
            PRINT_INFO("panoramaMode:%u\n",msg_data.p2pDeviceInfo.panoramaMode);
            PRINT_INFO("version:%s\n",msg_data.p2pDeviceInfo.version);
            PRINT_INFO("model:%s\n",msg_data.p2pDeviceInfo.model);
            PRINT_INFO("web_port:%u\n",msg_data.p2pDeviceInfo.web_port);

            PRINT_INFO("manufacture:%x\n",msg_data.p2pDeviceInfo.manufacture);
            PRINT_INFO("%d %d %d %d %d\n",msg_data.p2pDeviceInfo.my_key[0],
                msg_data.p2pDeviceInfo.my_key[1],
                msg_data.p2pDeviceInfo.my_key[2],
                msg_data.p2pDeviceInfo.my_key[3],
                msg_data.p2pDeviceInfo.my_key[4]
                );
            PRINT_INFO("BoardCastSendTo total size :%d,send size :%d\n",sizeof(P2P_DEVICE_INFO_EX),sizeof(P2P_DEVICE_INFO_EX)-sizeof(msg_data.p2pDeviceInfo.my_key));
            #endif
            BoardCastSendTo(send_sock, SEARCH_BROADCAST_ADDR, send_port, (char *)&msg_data, sizeof(P2P_DEVICE_INFO_EX)-sizeof(msg_data.p2pDeviceInfo.my_key));

            break;
        }
        case CMD_SET_DS_INFO://设置所有参数，前提是对方发来方式应该是指定了本机器ip地址。如果是广播方式，可能造成其他机器的id也被改成相同的值
        {
            P2P_DEVICE_INFO *device_info = (P2P_DEVICE_INFO*)(pBuf + sizeof(SEARCH_CMD));
            struct device_info_mtd info;
            PRINT_INFO("will set ds device info:\n");
            PRINT_INFO("p2pid:%s\n",device_info->p2pid);
            PRINT_INFO("devType:%u\n",device_info->devType);
            PRINT_INFO("netFamily:%u\n",device_info->netFamily);
            PRINT_INFO("language:%u\n",device_info->language);
            PRINT_INFO("odmID:%u\n",device_info->odmID);
            PRINT_INFO("panoramaMode:%u\n",device_info->panoramaMode);

            PRINT_INFO("model:%s\n",device_info->model);

            PRINT_INFO("manufacture:%x\n",device_info->manufacture);
            PRINT_INFO("%d,%d,%d,%d,%d\n",device_info->my_key[0],
                device_info->my_key[1],
                device_info->my_key[2],
                device_info->my_key[3],
                device_info->my_key[4]);

            int ret = load_info_to_mtd_reserve(MTD_TUTK_P2P, &info, sizeof(info));
            strncpy(info.device_id,device_info->p2pid,sizeof(info.device_id));
            info.devType = device_info->devType;
            info.netFamily = device_info->netFamily;
            info.language= device_info->language;
            info.odmID= device_info->odmID;
            info.panoramaMode= device_info->panoramaMode;

            strncpy((char*)info.model,(char*)device_info->model,sizeof(info.model));
            memcpy(info.my_key,device_info->my_key,sizeof(info.my_key));

            ret = save_info_to_mtd_reserve(MTD_TUTK_P2P, &info, sizeof(info));
            if (ret < 0)
            {
                PRINT_ERR("[p2p] set ds device info to flash error.\n");
                return -1;
            }
            else
            {
                PRINT_INFO("[p2p] set ds device info %s to flash ok.\n", info.device_id);
            }

            break;
        }
        case CMD_BATCH_SET_DS_INFO://批量设置参数(设备id除外)，用广播方式，
        {

            P2P_DEVICE_INFO *device_info = (P2P_DEVICE_INFO*)(pBuf + sizeof(SEARCH_CMD));
            struct device_info_mtd info;
            PRINT_INFO("will set ds device info:\n");
            //PRINT_INFO("p2pid:%s\n",device_info->p2pid);
            PRINT_INFO("devType:%u\n",device_info->devType);
            PRINT_INFO("netFamily:%u\n",device_info->netFamily);
            PRINT_INFO("language:%u\n",device_info->language);
            PRINT_INFO("odmID:%u\n",device_info->odmID);
            PRINT_INFO("panoramaMode:%u\n",device_info->panoramaMode);
            PRINT_INFO("manufacture:%x\n",device_info->manufacture);

            PRINT_INFO("model:%s\n",device_info->model);

            int ret = load_info_to_mtd_reserve(MTD_TUTK_P2P, &info, sizeof(info));
            //strncpy(info.device_id,device_info->p2pid,sizeof(info.device_id));
            info.devType = device_info->devType;
            info.netFamily = device_info->netFamily;
            info.language= device_info->language;
            info.odmID= device_info->odmID;
            info.panoramaMode= device_info->panoramaMode;

            strncpy((char*)info.model,(char*)device_info->model,sizeof(info.model));


            ret = save_info_to_mtd_reserve(MTD_TUTK_P2P, &info, sizeof(info));
            if (ret < 0)
            {
                PRINT_ERR("[p2p] set ds device info to flash error.\n");
                return -1;
            }
            else
            {
                PRINT_INFO("[p2p] set ds device info %s to flash ok.\n", info.device_id);
            }

            break;
        }
        case CMD_SET_P2PINFO:
            type = *((int *)pTmp);
    		switch(type)
            {
                case MTD_YUNNI_P2P:
                	memset(&yunni_info, 0, sizeof(yunni_info));
                	memcpy(&yunni_info,(pTmp + 1),sizeof(struct device_info_mtd));
                	save_info_to_mtd_reserve(MTD_YUNNI_P2P, &yunni_info, sizeof(yunni_info));
                	break;
                case MTD_ULUCU_P2P:                    
                    #ifdef MODULE_SUPPORT_ULUCU
                	fill_dana_file((pTmp + 1),sizeof(Dev_SN_Info));
                	memset(&ulu_info,0,sizeof(Dev_SN_Info));
                	memcpy(&ulu_info,(pTmp + 1),sizeof(Dev_SN_Info));
                	fill_uluc_file((char *)&ulu_info,sizeof(Dev_SN_Info));
                	save_info_to_mtd_reserve(MTD_ULUCU_P2P,&ulu_info, sizeof(Dev_SN_Info));
                    #endif                
                	break;
                case MTD_DANA_P2P:
                	memset(&dana_info,0,sizeof(struct Dana_info));

                	memcpy(&dana_info,(pTmp + 1),sizeof(struct Dana_info ));
                	LOG_INFO("length is %d\n",dana_info.file_len);
                	fill_dana_file(dana_info.Dana_file,dana_info.file_len);
                	save_info_to_mtd_reserve(MTD_DANA_P2P, &dana_info,sizeof(struct Dana_info));
                	break;
                case MTD_GOKE_INFO:
                	//save_info_to_mtd2(MTD_GOKE_INFO, &factory, sizeof(struct goke_factory));
                	break;
                default:
                	break;
             }
             break;
        default:
	     printf("===========into default=============\n");
             break;
    }
    return 0;
}


int dwip_to_str(DWORD ip, char ip_str[32])
{
    ///char a0, a1, a2, a3;
    int  i0, i1, i2, i3;

    if (NULL == ip_str)
        return -1;
    i0 = ip & 0xff;
    i1 = (ip>>8)  & 0xff;
    i2 = (ip>>16) & 0xff;
    i3 = (ip>>24) & 0xff;

    sprintf(ip_str, "%d.%d.%d.%d",i0, i1, i2, i3);
    printf("xbb: %s", ip_str);

    return 0;
}

int gk_cms_change_mac(unsigned char *hwaddr, unsigned char *addr)
{
	unsigned char *paddr = addr;
	unsigned char str[4];
	int number;
	int i;

	if(!addr || !hwaddr)
	{
		PRINT_INFO("bad parameter");
		return -1;
	}

	i = 0;
	while(*paddr && (i < strlen((char *)addr)) && i < 6)
	{
		memset(str,0,sizeof(str));
		if(':' != *paddr)
		{
			str[0] = toupper(*paddr);
			if(str[0]>='A' && str[0] <='Z')
			{
			  number = 0xf0&((str[0]-'A'+10)<<4);
			}
			else if(str[0]>='0' && str[0] <='9')
			{
			  number = 0xf0&((str[0]-'0')<<4);
			}
			else
			{
				PRINT_ERR("hwaddr content error");
				return -1;
			}

			paddr ++;
			if(':' != *paddr)
			{
				str[0] = toupper(*paddr);
				paddr ++;

				if(str[0]>='A' && str[0] <='Z')
				{
					number |= ((str[0]-'A'+10)&0x0f);
				}
				else if(str[0]>='0' && str[0] <='9')
				{
					number |= ((str[0]-'0')&0x0f);
				}
				else
				{
					PRINT_ERR("hwaddr content error");
					return -1;
				}
			}

			hwaddr[i] = number;
			i ++;
		}
		else
		  paddr ++;
	}


	return 0;
}

int IPCsearch_get_p2p_info(char deviceID[5][64], char mac[5][64])
{
	int ret = 0;

#ifdef MODULE_SUPPORT_ULUCU
    static Dev_SN_Info ulu_info;
#endif
    struct device_info_mtd yunni_info;
	//struct Dana_info dana_info;
    struct device_info_mtd gb28181_info;
    char serverKey[64] = {0};

    PRINT_INFO("[p2p] get p2p info.\n");

    ret = load_info_to_mtd_reserve(MTD_YUNNI_P2P,&yunni_info,sizeof(struct device_info_mtd));
    if(ret !=0)
    {
        PRINT_ERR("[p2p] get p2p error: type[MTD_YUNNI_P2P].\n");
    }
    else
    {
        memcpy(deviceID[0], yunni_info.device_id, 32);
        PRINT_INFO("[p2p] get p2p info: type[MTD_YUNNI_P2P] ID[%s].\n",deviceID[0]);
    }
    
#ifdef MODULE_SUPPORT_ULUCU
    ret = load_info_to_mtd_reserve(MTD_ULUCU_P2P,&ulu_info,sizeof(Dev_SN_Info));
    if(ret !=0)
    {
        PRINT_ERR("[p2p] get p2p error: type[MTD_ULUCU_P2P].\n");

    }
    else
    {
        memcpy(deviceID[1], ulu_info.SN, 17);
        PRINT_INFO("[p2p] get p2p info: type[MTD_ULUCU_P2P] ID[%s].\n",deviceID[1]);
    }
#endif    
    ret = load_info_to_mtd_reserve(MTD_KEY_INFO, serverKey, sizeof(serverKey));
    if(ret !=0)
    {
        PRINT_ERR("[p2p] get p2p error: type[MTD_KEY_INFO].\n");
    }
    else
    {
        memcpy(deviceID[2], serverKey, 64);
        PRINT_INFO("[p2p] get p2p info: type[MTD_KEY_INFO] ID[%s].\n",deviceID[2]);
    }
    ret = load_info_to_mtd_reserve(MTD_TUTK_P2P,&yunni_info,sizeof(struct device_info_mtd));
    if(ret !=0)
    {
        PRINT_ERR("[p2p] get p2p error: type[MTD_TUTK_P2P].\n");

    }
    else
    {
        memcpy(deviceID[3], yunni_info.device_id, 32);
        memcpy(mac[3], yunni_info.eth_mac_addr, 32);
        PRINT_INFO("[p2p] get p2p info: type[MTD_TUTK_P2P] ID[%s].mac:%s\n",deviceID[3], mac[3]);
    }

    ret = load_info_to_mtd_reserve(MTD_GB28181_INFO,&gb28181_info,sizeof(struct device_info_mtd));
    if(ret !=0)
    {
        PRINT_ERR("[p2p] get p2p error: type[MTD_GB28181_INFO].\n");

    }
    else
    {
        memcpy(deviceID[4], gb28181_info.device_id, 32);
        PRINT_INFO("[p2p] get p2p info: type[MTD_GB28181_INFO] ID[%s].\n",deviceID[4]);
    }

	return ret;
}



int IPCsearch_set_p2p_info(int P2P_Type, char *deviceID, char *mac)
{
    int ret = -1;
#ifdef MODULE_SUPPORT_ULUCU
    static Dev_SN_Info ulu_info;
#endif
	struct device_info_mtd yunni_info;
	struct Dana_info dana_info;

    PRINT_INFO("[p2p] set p2p info: type[%d] id[%s].\n", P2P_Type, deviceID);
    switch(P2P_Type)
    {
        case MTD_YUNNI_P2P:
            memset(&yunni_info, 0, sizeof(yunni_info));
            load_info_to_mtd_reserve(MTD_YUNNI_P2P, &yunni_info, sizeof(yunni_info));
            memcpy(&yunni_info.device_id,deviceID,32);
            ret = save_info_to_mtd_reserve(MTD_YUNNI_P2P, &yunni_info, sizeof(yunni_info));
            break;
        case MTD_ULUCU_P2P:
            
#ifdef MODULE_SUPPORT_ULUCU
            memset(&ulu_info,0,sizeof(Dev_SN_Info));
            load_info_to_mtd_reserve(MTD_ULUCU_P2P, &ulu_info, sizeof(Dev_SN_Info));
            memcpy(&ulu_info.SN,deviceID,16);
            ret = save_info_to_mtd_reserve(MTD_ULUCU_P2P,&ulu_info, sizeof(Dev_SN_Info));
            delete_path(ULU_CONF_DEVICEID_PATH);
#endif            
            break;
        case MTD_DANA_P2P:
            memset(&dana_info,0,sizeof(struct Dana_info));
            load_info_to_mtd_reserve(MTD_DANA_P2P, &dana_info, sizeof(struct Dana_info));
            memcpy(&dana_info.Dana_ID,deviceID,32);
            ret = save_info_to_mtd_reserve(MTD_DANA_P2P, &dana_info,sizeof(struct Dana_info));
            break;
        case MTD_TUTK_P2P:
            memset(&yunni_info, 0, sizeof(yunni_info));
            load_info_to_mtd_reserve(MTD_TUTK_P2P, &yunni_info, sizeof(yunni_info));
            memcpy(&yunni_info.device_id,deviceID,32);
            strcpy(yunni_info.eth_mac_addr, mac);
            ret = save_info_to_mtd_reserve(MTD_TUTK_P2P, &yunni_info, sizeof(yunni_info));
            
            #ifndef MODULE_SUPPORT_MOJING
            //未定义魔镜时需要保存mac地址
            struct goke_factory factory;
            strcpy(factory.mac, mac);
            save_info_to_mtd_reserve(MTD_GOKE_INFO, &factory, sizeof(struct goke_factory));
            #endif
            break;
        case MTD_GB28181_INFO:
            memset(&yunni_info, 0, sizeof(yunni_info));
            load_info_to_mtd_reserve(MTD_GB28181_INFO, &yunni_info, sizeof(yunni_info));
            memcpy(&yunni_info.device_id,deviceID,32);
            strcpy(yunni_info.eth_mac_addr, mac);
            ret = save_info_to_mtd_reserve(MTD_GB28181_INFO, &yunni_info, sizeof(yunni_info));
            break;
        case MTD_GOKE_INFO:
            //save_info_to_mtd_reserve(MTD_GOKE_INFO, &factory, sizeof(struct goke_factory));
            break;
        case MTD_KEY_INFO:
            //save_info_to_mtd_reserve(MTD_GOKE_INFO, &factory, sizeof(struct goke_factory));

            break;
        default:
            break;
    }

    if(ret !=0)
    {
        PRINT_ERR("[p2p] set p2p error: type[%d] id[%s].\n", P2P_Type, deviceID);
    }
    else
    {
        PRINT_INFO("[p2p] set p2p success: type[%d] id[%s].\n", P2P_Type, deviceID);
    }
    if (mac != NULL && strlen(mac) > 10 && strcmp(mac, "00:00:00:00:00:00") != 0 
        && strcmp(runNetworkCfg.lan.mac, mac) != 0)
    {
        memset(runNetworkCfg.lan.mac, 0, sizeof(runNetworkCfg.lan.mac));
        strncpy(runNetworkCfg.lan.mac, mac, sizeof(runNetworkCfg.lan.mac));
        NetworkCfgSave();
    }
    return ret;
}

int IPCsearch_get_test_result(void *pTestResultInfo)
{
    NET_TEST_RESULT_INFO stTestResult;
    memset(&stTestResult,0,sizeof(stTestResult));//runSystemCfg.deviceInfo.devType
    int ret = load_info_to_mtd_reserve(MTD_TEST_RESULT,&stTestResult, sizeof(stTestResult));
    if(ret != 0)
    {
        PRINT_ERR("[test result] get test result failed, use default values.\n");
        // 首次连接测试软件
        // 根据设备类型，不支持的功能置JB_TEST_RESULT_DNSUP，支持的功能置JB_TEST_RESULT_DNT
        stTestResult.emIRCut = JB_TEST_RESULT_DNT;
        stTestResult.emLed = JB_TEST_RESULT_DNT;
        stTestResult.emMic = JB_TEST_RESULT_DNT;
        stTestResult.emPTZ = JB_TEST_RESULT_DNT;
        stTestResult.emRestore = JB_TEST_RESULT_DNT;
        stTestResult.emSd = JB_TEST_RESULT_DNT;
        stTestResult.emSpeaker = JB_TEST_RESULT_DNT;
        stTestResult.emWifi = JB_TEST_RESULT_DNT;
        stTestResult.emView = JB_TEST_RESULT_DNT;
        stTestResult.emWhiteLed = JB_TEST_RESULT_DNT;
        save_info_to_mtd_reserve(MTD_TEST_RESULT,&stTestResult, sizeof(stTestResult));
    }
    else
    {
        PRINT_INFO("[test result] get test result success.\n");
    }
    memcpy(pTestResultInfo,&stTestResult,sizeof(stTestResult));
    return ret;
}

int IPCsearch_set_test_result(void *pTestResultInfo)
{
    NET_TEST_RESULT_INFO *pTestResult = (NET_TEST_RESULT_INFO *)pTestResultInfo;
    int ret = save_info_to_mtd_reserve(MTD_TEST_RESULT,pTestResult, sizeof(NET_TEST_RESULT_INFO));
    if(ret != 0)
    {
        PRINT_ERR("[test result] set test result error.\n");
    }
    else
    {
        PRINT_INFO("[test result] set test result success.\n");
    }
    return ret;
}

int ipc_search_save_to_resave_wifi(char *ssid, char* psd,int enc_type)
{
    struct device_info_mtd info ;
    int ret = load_info_to_mtd_reserve(MTD_TUTK_P2P,&info, sizeof(struct device_info_mtd));
    if(ret != 0)
    {
        PRINT_ERR("Get reserver info error\n");
		return -1;
    }
    else
    {
    	if(strcmp(info.wifi_psd,psd) || strcmp(info.wifi_ssid,ssid) || info.wifi_enc_type != enc_type)
    	{
			PRINT_INFO("Wifi info old [ssid][psd][type] [%s]:[%s]:[%d]",info.wifi_psd,info.wifi_ssid,info.wifi_enc_type);
			PRINT_INFO("Wifi info new [ssid][psd] [%s]:[%s]:[%d]",ssid,psd,enc_type);

			strncpy(info.wifi_ssid,ssid,32);
			strncpy(info.wifi_psd,psd,32);
			info.wifi_enc_type = enc_type;
			ret = save_info_to_mtd_reserve(MTD_TUTK_P2P,&info, sizeof(struct device_info_mtd));
			if(ret == 0)
			{
				PRINT_INFO("Save wifi to reserve success");
			}
			else
			{
				PRINT_INFO("Save wifi to reserve failed");
			}
		}
		else
		{
			PRINT_INFO("Wifi ssid and psd equal with reserve");
		}
    }
    return ret;

}


