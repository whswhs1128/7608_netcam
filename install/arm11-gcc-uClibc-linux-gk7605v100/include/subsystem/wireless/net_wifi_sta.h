#ifndef __WIFI_STA_H__
#define __WIFI_STA_H__

#include "net_wifi_commom.h"
//0: none  (wifi enable)
//1: wep 64 assii (wifi enable)
//2: wep 64 hex (wifi enable)
//3: wep 128 assii (wifi enable)
//4: wep 128 hex (wifi enable)
//5: WPAPSK-TKIP
//6: WPAPSK-AES
//7: WPA2PSK-TKIP
//8: WPA2PSK-AES
#define WPAECN_NONE     0
#define WEP_64_ASSCI    1
#define WEP_64_HEX      2
#define WEP_128_ASSCI   3
#define WEP_128_HEX     4
#define WPAPSK_TKIP     5
#define WPAPSK_AES      6   
#define WPA2PSK_TKIP    7
#define WPA2PSK_AES     8

#define WPA_AES_ENC "[WPA-PSK-CCMP]"
#define WPA_TKIP_ENC "[WPA-PSK-TKIP+CCMP]"
#define WPA2_AES_ENC "[WPA2-PSK-CCMP]"
#define WPA2_TKIP_ENC "[WPA2-PSK-TKIP+CCMP]"
#define WPS_ENC "[WPS]"


/* encode way */
#define WIFI_ENCRYPT_UNKNOW         0
#define WIFI_ENCRYPT_OFF            1
#define WIFI_ENCRYPT_OPEN           2
#define WIFI_ENCRYPT_RESTRICTED     3
#define WIFI_ENCRYPT_ENABLED        4
#define WIFI_ENCRYPT_TEMP           5
#define WIFI_ENCRYPT_ON             6
#define WIFI_ENCRYPT_MAXMUN         6


/* Modes of operation */
#define WIFI_MODE_AUTO	0	/* Let the driver decides */
#define WIFI_MODE_ADHOC	1	/* Single cell network */
#define WIFI_MODE_INFRA	2	/* Multi cell network, roaming, ... */
#define WIFI_MODE_MASTER	3	/* Synchronisation master or Access Point */
#define WIFI_MODE_REPEAT	4	/* Wireless Repeater (forwarder) */
#define WIFI_MODE_SECOND	5	/* Secondary master/repeater (backup) */
#define WIFI_MODE_MONITOR	6	/* Passive monitor (listen only) */
#define WIFI_MODE_MESH	7	/* Mesh (IEEE 802.11s) network */

/* group_chip || pair_chip */
#define WIFI_CHIP_UNKNOW          0
#define WIFI_CHIP_NONE            1
#define WIFI_CHIP_WEP_40          2
#define WIFI_CHIP_TKIP            3
#define WIFI_CHIP_WRAP            4
#define WIFI_CHIP_CCMP            5
#define WIFI_CHIP_WEP_104         6
#define WIFI_CHIP_PROPRIETARY     7
#define WIFI_CHIP_MAXNUM          7


/* auth_suite   */
#define WIFI_AUTH_UNKNOW            0
#define WIFI_AUTH_NONE              1
#define WIFI_AUTH_802_1_X           2
#define WIFI_AUTH_PSK               3
#define WIFI_AUTH_PROPRIETARY       4
#define WIFI_AUTH_MAXNUM            4


/* ie */
#define WIFI_IE_UNKNOW              0
#define WIFI_IE_WPA1                1
#define WIFI_IE_WPA2                2
#define WIFI_IE_MAXNUM              2

/*  wifi connect status  */
#define WIFI_STATUS_ERROR           -1  //error
#define WIFI_STATUS_DISCONNECTED    0   //disconnect
#define WIFI_STATUS_CONNECTING      1   //connecting
#define WIFI_STATUS_CONNECTED       2   //connect complete
#define WIFI_STATUS_INACTIVE        3   //network is no active
#define WIFI_STATUS_UNKNOW        	4	//get status is unknow



/* set network flags */
#define WIFI_SET_SSID       0
#define WIFI_SET_PSK        1
#define WIFI_SET_KEY_MGMT   2
#define WIFI_SET_INDENT     3
#define WIFI_SET_PASSWORD   4
#define WIFI_SET_BSSID   	5
#define WIFI_SET_SCAN_SSID	6
#define WIFI_SET_MAXNUM     6

/* control command resulte*/
#define WIFI_OK             0
#define WIFI_FAIL           -1      //function fail
#define WIFI_TIMEOUT        -2      //Link tmeout
#define WIFI_NO_SSID        -3      //no ssid
#define WIFI_LINK_ERROR		-4      //link error


typedef struct gen_ie_data_s
{
    unsigned char use_flag;/* 1, is used */
    unsigned char ie;       /* ie way */
    unsigned char group_chip;
    unsigned int version;
    unsigned char pair_chip[4];
    unsigned char auth_suite[4];
} GEN_IE_DataT;

typedef struct ssid_data_s
{
	char name[32];
	int name_len;
}SSID_DataT;

typedef struct scand_data_s
{
    int used;            /* if use = 1, is used */
	int qual;			/* signal quality */
	int noise;			/* Level + Noise are dBm */
	int level;
    unsigned int mode;  /* work mode */
	unsigned int channel;/* AP used channel */
    unsigned int maxrate; /* max reate, 802.11 rates 500000 b/s units */
	unsigned int encode;    /* AP encode way */
    char        key[32];    /* encode key */
    char        protocol[32]; /* protocol name */
	SSID_DataT mac;		/* AP mac address (string) */
	SSID_DataT ssid;	/* AP ssid (string) */
    GEN_IE_DataT gen_ie[4]; /* ie , Generic IE (WPA, RSN, WMM, ..)*/
}SCAN_DataT;

#ifdef NETWORK_ELIAN_SUPPORT
typedef struct elian_result {
	unsigned char auth_mode;
	unsigned char cust_data_len;
	char ssid[33];
	char pwd[65];
	char user[65];
	char cust_data[256];
} ELIAN_ResultT;
#endif

#ifdef __cplusplus
extern "C" {
#endif
/* on/off client mode of wireless */
extern int net_wifi_cli_on(const WIFI_HANDLE dev, const char *ip, const char *mask);
extern int net_wifi_cli_off(const WIFI_HANDLE dev);


/* get AP message when connected */
extern int net_wifi_cli_get_essid(const WIFI_HANDLE dev, char *const essid, int len);
extern int net_wifi_cli_set_essid(const WIFI_HANDLE dev, char *essid, int len);
extern int net_wifi_cli_get_bssid(const WIFI_HANDLE dev, char * const mac, int len);
extern int net_wifi_cli_set_bssid(const WIFI_HANDLE dev, char *mac, int len);
extern int net_wifi_cli_get_transfreq(const WIFI_HANDLE dev, int * const freq_mhz);
extern int net_wifi_cli_get_transprotocol(const WIFI_HANDLE dev, char *const protocol, int len);

/* wireless operation */
extern int net_wifi_cli_get_scanlist(const WIFI_HANDLE dev, SCAN_DataT *list, int len);

extern int net_wifi_cli_set_workmode(const WIFI_HANDLE dev, int mode);

/* dev is wifi device name, name is SSID of AP, passwd is link key, usedhcp is wether use udhcpc (0:no use !0:use) */
extern int net_wifi_cli_connecting(const WIFI_HANDLE dev, SCAN_DataT *link_data,
    const char *ssid, const char *key, unsigned char usedhcp);
extern int net_wifi_cli_disconnect(const WIFI_HANDLE dev);
extern int net_wifi_cli_reconnect(const WIFI_HANDLE dev);
/* return network status */
extern int net_wifi_cli_status(const WIFI_HANDLE dev);
/* wps opeare */
extern int net_wifi_cli_wps_pbc(const WIFI_HANDLE dev, const char *bssid, int udhcp);
extern int net_wifi_cli_wps_pin(const WIFI_HANDLE dev,
	const char *bssid, const char *pin, int udhcp);
/* return pin space, is malloc. need free */
extern char * net_wifi_cli_wps_gen_pin(const WIFI_HANDLE dev, char *bssid);
extern int net_wifi_cli_wps_reg(const WIFI_HANDLE dev,const char *bssid,
	const char *pin);
int net_wifi_cli_get_network(const WIFI_HANDLE dev, int seq,
    int flag, char *buf, int buflen);

#ifdef NETWORK_ELIAN_SUPPORT
extern int	net_wifi_cli_smartlink_start(const WIFI_HANDLE dev, unsigned char channels);
extern int	net_wifi_cli_smartlink_stop(const WIFI_HANDLE dev);
extern int	net_wifi_cli_smartlink_clear(const WIFI_HANDLE dev);
extern int	net_wifi_cli_smartlink_get_result(const WIFI_HANDLE dev, ELIAN_ResultT *result);
extern int	net_wifi_cli_smartlink_set_ch(const WIFI_HANDLE dev, unsigned char channels);
#endif
int net_wifi_cli_ping(const WIFI_HANDLE dev);//wpa_supplicant check of normal work

#ifdef __cplusplus
}
#endif



#endif
