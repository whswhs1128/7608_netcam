/*!
*****************************************************************************
** FileName     : cfg_network.h
**
** Description  : config for network.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-7-29
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_CFG_NETWORK_H__
#define _GK_CFG_NETWORK_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"
#include "netcam_network.h"

#define WIFI_CONNECT_LIST_MAX_NUMBER 20

/***********************************/
/***       network               ***/
/***********************************/
//网络配置结构
typedef enum {
    GK_NET_TYPE_DHCP = 0,
    GK_NET_TYPE_STATIC,
} GK_NET_TYPE_E;

typedef struct {
    SDK_S32  enable;
    SDK_S8   netName[MAX_STR_LEN_64]; //网卡名称，用于多网卡区别
    SDK_S32  ipVersion; //v4,v6
    SDK_S8   mac[MAX_STR_LEN_20];   //Mac地址
    SDK_S32  dhcpIp; // 0 static, 1 dhcp
    SDK_S32  upnpEnable; // 0 disable, 1 enable
    SDK_S8   ip[MAX_STR_LEN_16];
    SDK_S8   netmask[MAX_STR_LEN_16];  //掩码地址
    SDK_S8   gateway[MAX_STR_LEN_16];
    SDK_S8   multicast[MAX_STR_LEN_16];
    SDK_S32  dhcpDns; // 0 static, 1 dhcp
    SDK_S8   dns1[MAX_STR_LEN_16];
    SDK_S8   dns2[MAX_STR_LEN_16];
	SDK_S32  autotrack;
} GK_NET_ETH, *LPGK_NET_ETH;

struct goke_factory{
    SDK_S8   mac[MAX_STR_LEN_20];   //Mac地址
};


/**UPNP**/
typedef struct {
    SDK_U32  enable;               /*是否启用upnp*/
    SDK_U32  mode;                 /*upnp工作方式.0为自动端口映射，1为指定端口映射*/
    SDK_U32  lineMode;             /*upnp网卡工作方式.0为有线网卡,1为无线网卡*/
    SDK_S8   serverIp[MAX_STR_LEN_32];         /*upnp映射主机.即对外路由器IP*/
    SDK_U32  dataPort;             /*upnp映射数据端口*/
    SDK_U32  webPort;              /*upnp映射网络端口*/
    SDK_U32  mobilePort;           /*upnp映射手机端口*/
    SDK_U16  dataPort1;            /*upnp已映射成功的数据端口*/
    SDK_U16  webPort1;             /*upnp已映射成功的网络端口*/
    SDK_U16  mobilePort1;          /*upnp映射成功的手机端口*/
    SDK_U16  dataPortOK;
    SDK_U16  webPortOK;
    SDK_U16  mobilePortOK;
} GK_UPNP_CFG, *LPGK_UPNP_CFG;

typedef struct {
    SDK_U32 enable;       //0-不启用,1-启用
    SDK_S8  user[MAX_STR_LEN_64];     //PPPoE用户名
    SDK_S8  password[MAX_STR_LEN_64]; //PPPoE密码
} GK_NET_PPPOECFG, *LPGK_NET_PPPOECFG;

typedef enum {
    GK_DDNS_DYNDNS = 0,
    GK_DDNS_3322,
    GK_DDNS_NIGHTOWLDVR,
    GK_DDNS_NOIP,
    GK_DDNS_MYEYE,
    GK_DDNS_PEANUTHULL,
    GK_DDNS_BUTT,
    GK_DDNS_CHANGEIP,
    GK_DDNS_POPDVR,
    GK_DDNS_SKYBEST,
    GK_DDNS_DVRTOP,
} GK_DDNS_TYPE;

typedef struct {
    SDK_U32     enableDDNS;   //是否启用DDNS
    GK_DDNS_TYPE type;         //DDNS服务器类型, 域名解析类型：GK_DDNS_TYPE
    SDK_S8      username[MAX_STR_LEN_64];
    SDK_S8      password[MAX_STR_LEN_64];
    SDK_S8      domain[MAX_STR_LEN_256];   //在DDNS服务器注册的域名地址
    SDK_S8      address[MAX_STR_LEN_256];  //DNS服务器地址，可以是IP地址或域名 www.dynddns.org
    SDK_S32     port;         //DNS服务器端口，默认为6500
} GK_NET_DDNSCFG, *LPGK_NET_DDNSCFG;

typedef struct {
    SDK_S32   enableEmail;      //是否启用
    SDK_S32    attachPicture;    //是否带附件
    SDK_U8    smtpServerVerify; //发送服务器要求身份验证
    SDK_U8    mailInterval;     //最少2s钟(1-2秒；2-3秒；3-4秒；4-5秒)

    SDK_S8    eMailUser[MAX_STR_LEN_64];    //账号
    SDK_S8    eMailPass[MAX_STR_LEN_64];    //密码
    SDK_S32   encryptionType;   /**< 加密类型 ssl*/
    SDK_S8    smtpServer[MAX_STR_LEN_128];   //smtp服务器  //用于发送邮件    
    SDK_S32   smtpPort;      /**< 服务器端口,一般为25，用户根据具体服务器设置 */
    SDK_S8    pop3Server[MAX_STR_LEN_128];   //pop3服务器  //用于接收邮件,和IMAP性质类似   
    SDK_S32   pop3Port;      /**< 服务器端口,一般为25，用户根据具体服务器设置 */
    SDK_S8    fromAddr[MAX_STR_LEN_64]; 	/**< 发送人地址 */
    SDK_S8    toAddrList0[MAX_STR_LEN_64];  /**< 收件人地址 1 */    
    SDK_S8    toAddrList1[MAX_STR_LEN_64];  /**< 收件人地址 2 */    
    SDK_S8    toAddrList2[MAX_STR_LEN_64];  /**< 收件人地址 3 */
    SDK_S8    toAddrList3[MAX_STR_LEN_64];  /**< 收件人地址 4 */
    SDK_S8    ccAddrList0[MAX_STR_LEN_64];  /**< 收件人地址 0 */
    SDK_S8    ccAddrList1[MAX_STR_LEN_64];  /**< 收件人地址 1 */
    SDK_S8    ccAddrList2[MAX_STR_LEN_64];  /**< 收件人地址 2 */
    SDK_S8    ccAddrList3[MAX_STR_LEN_64];  /**< 收件人地址 3 */    
    SDK_S8    bccAddrList0[MAX_STR_LEN_64]; /**< 密送人地址 0 */
    SDK_S8    bccAddrList1[MAX_STR_LEN_64]; /**< 密送人地址 1 */
	SDK_S8	  bccAddrList2[MAX_STR_LEN_64]; /**< 密送人地址 2 */
	SDK_S8	  bccAddrList3[MAX_STR_LEN_64]; /**< 密送人地址 3 */
} GK_NET_EMAIL_PARAM, *LPGK_NET_EMAIL_PARAM;

/* ftp上传参数*/
typedef struct {
    SDK_S32   enableFTP;       /*是否启动ftp上传功能*/
    SDK_S8    address[MAX_STR_LEN_128];     /*ftp 服务器，可以是IP地址或域名*/
    SDK_S32   port;            /*ftp端口*/
    SDK_S8    userName[MAX_STR_LEN_64];    /*用户名*/
    SDK_S8    password[MAX_STR_LEN_64];    /*密码*/
	SDK_S8    datapath[MAX_STR_LEN_128];
	SDK_S8    filename[MAX_STR_LEN_128];
	SDK_S32   interval;
} GK_NET_FTP_PARAM, *LPGK_NET_FTP_PARAM;

typedef struct {
    SDK_U32 enable;
	SDK_U32 isConnect;	
    SDK_S32 mode; //0 accessPoSDK_S32, 1 stationMode
    SDK_S32 staMode; //802.11bgn mixed
    SDK_S32 apBssId;
    SDK_S32 apEssId;
    SDK_S32 apPsk;
    SDK_S32 fixedBpsModeEnabled;
    SDK_S32 bssId;
    SDK_S32 essId;
    SDK_S32 Psk;
    SDK_S32 apMode; //["802.11b","802.11g","802.11n","802.11bg","802.11bgn"]
    SDK_S32 apMode80211nChannel; // ["Auto","1","2","3","4","5","6","7","8","9","10","11","12","13","14"]
    SDK_S32 essIdBroadcastingEnabled;
    SDK_S32 wpaMode; //["WPA_PSK","WPA2_PSK"]
} GK_NET_WIRELESS_CFG, *LPGK_NET_WIRELESS_CFG;

typedef struct {
    SDK_S32            httpPort;   //Http端口
    SDK_S32            rtspPort;   //554
} GK_NET_PORT_CFG;

typedef struct {
    GK_NET_ETH          lan;
    GK_NET_ETH          wifi;
    GK_NET_WIRELESS_CFG wireless;
    GK_UPNP_CFG         upnp;
    GK_NET_PPPOECFG     pppoe;
    GK_NET_DDNSCFG      ddns;
    GK_NET_EMAIL_PARAM  email;
    GK_NET_FTP_PARAM    ftp;
    GK_NET_PORT_CFG     port;
	GK_NET_WIRELESS_LINK_CFG  wifilink[WIFI_CONNECT_LIST_MAX_NUMBER];
} GK_NET_NETWORK_CFG, *LPGK_NET_NETWORK_CFG;

extern int NetworkCfgSave();
extern int NetworkCfgLoad();
extern void NetworkCfgPrint();
extern int NetworkCfgLoadDefValue();
extern char *NetworkCfgGetJosnString(int type);

#define NETWORK_CFG_FILE "gk_network_cfg.cjson"

extern GK_NET_NETWORK_CFG runNetworkCfg;
extern GK_CFG_MAP lanMap[];
extern GK_CFG_MAP wifiMap[];
extern GK_CFG_MAP emailMap[];
extern GK_CFG_MAP ftpMap[];
extern GK_CFG_MAP portMap[];
extern GK_CFG_MAP wifiLinkMap[];

#ifdef __cplusplus
}
#endif
#endif

