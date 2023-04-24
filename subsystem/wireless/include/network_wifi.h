#ifndef _NETWORK_WIFI_H_
#define _NETWORK_WIFI_H_
#include <stdbool.h>
#include "net_wifi_commom.h"
#include "net_wifi_smartlink.h"
/* support wireless work mode */
typedef struct {
	bool	supportSTA;	//support STA
	bool	supportAP;	//support AP
	bool	reserve0;	//reserve ...
	bool	reserve1;
} NETCAM_supportWorkModeT;

/* STA work mode support function. */
typedef struct {
	bool	supportWPS; //support WPS connect
	bool	supportSL;	//support Smart Link connect
	bool	reserve0;	//reserve ...
	bool	reserve1;
} NETCAM_STASupportFuncT;


/* AP work mode support function. */
typedef struct {
	bool	reserve0;	//reserve ...
	bool	reserve1;
} NETCAM_APSupportFuncT;

typedef struct
{
	int				vaild;
	char            essid[MAX_WIFI_NAME];
    int             quality;    /* signal quality */
    char            mac[MAX_WIFI_NAME];
    WIFI_ENC_MODE   security;
								//0: none  (wifi enable)
								//1: wep 64 assii (wifi enable)
								//2: wep 64 hex (wifi enable)
								//3: wep 128 assii (wifi enable)
								//4: wep 128 hex (wifi enable)
								//5: WPAPSK-TKIP
								//6: WPAPSK-AES
								//7: WPA2PSK-TKIP
								//8: WPA2PSK-AES
    int             channel;


}WIFI_SCAN_LIST_t;


typedef enum
{
	NETCAM_WIFI_NONE = 0,
	NETCAM_WIFI_STA,
	NETCAM_WIFI_AP,
} NETCAM_WIFI_WorkModeEnumT;

typedef enum
{
	NETCAM_HOST_OPEN = 0,
	NETCAM_HOST_WEP,
	NETCAM_HOST_WPA_TKIP,
	NETCAM_HOST_WPA_AES,
	NETCAM_HOST_WPA2_TKIP,
	NETCAM_HOST_WPA2_AES,
} NETCAM_WIFI_HostAPEncryteEnumT;

typedef struct
{
	char ssid[32];
	char pwd[128];
	char ip[16];
	char startLoop[16];
	char endLoop[16];
	NETCAM_WIFI_HostAPEncryteEnumT encrypt;
} NETCAM_WIFI_HostAPConfigT;

typedef struct
{
	unsigned char essid[32];
	unsigned char	  passd[32];
	unsigned char    encMode;
	unsigned char    valid;
}GK_NET_WIRELESS_LINK_CFG;

typedef enum
{
	NET_WIFI_NONE = 0,
	NET_WIFI_STA,
	NET_WIFI_AP,
} NET_WIFI_WORK_StatusEnumT;


typedef enum
{
    AUDIO_NET_PLAY_ON = 0,
    AUDIO_NET_PLAY_OFF,
    AUDIO_WIFI_HARDWARE_NORMAL,
    AUDIO_WIFI_SMART_LINK,
    AUDIO_WIFI_LINK_FAIL,
} NET_AUDIO_PLAY_STATUS;

#define WIFI_RUNING		(1)
#define WIFI_STOP		(0)

#define WIFI_CONNECT_LIST_MAX_NUMBER 20
#ifndef CFG_DIR
#define CFG_DIR "/opt/custom/cfg/"
#endif
#ifndef DEFCFG_DIR
#define DEFCFG_DIR "/usr/local/defcfg/"
#endif

typedef void(*WIFI_MONITOR_CALLBACK)(int);

int network_wifi_init(int mode);
int network_wifi_deinit(void);
int network_wifi_is_init(void);
/*!
******************************************
** Description   @只在初始化wifi lib时执行;up 网卡
** Param[in]     @
**
** Return        @
******************************************
*/
int network_wifi_enable();

int network_wifi_monitor(GK_NET_WIRELESS_LINK_CFG *link_cfg, int num, WIFI_MONITOR_CALLBACK callback_func);
int network_wifi_wps_connect(void);
int network_wifi_connect(WIFI_LINK_INFO_t *linkInfo);



/*
 *	function: get work status of wifi
 *	status: 	NET_WIFI_NONE	normal status
 *			NET_WIFI_STA	work station mode
 *			NET_WIFI_AP 	AP mode
 */
extern NET_WIFI_WORK_StatusEnumT network_wifi_get_status(void);
extern void network_wifi_set_status(NET_WIFI_WORK_StatusEnumT status);
extern void net_wifi_protect_running(void);
extern void net_wifi_protect_stop(WIFI_HANDLE wifi_handle);
extern int net_wifi_protect_status(void);
int network_wifi_getConfigHostAP(NETCAM_WIFI_HostAPConfigT *apCfg);
int network_wifi_setConfigHostAP(NETCAM_WIFI_HostAPConfigT *apCfg);
int network_wifi_get_scan_list(WIFI_SCAN_LIST_t *list, int *number);
int network_wifi_get_scan_list_ext(WIFI_SCAN_LIST_t *list, int *number);

int network_wifi_get_connect_info(WIFI_LINK_INFO_t *linkInfo);
int network_wifi_probe(void);
int network_wifi_switch_workmode(NETCAM_WIFI_WorkModeEnumT mode);
int network_wifi_on(void);
int network_wifi_off(void);
void netcam_wifi_app_cb(char  *ssid, char *password, WIFI_ENC_MODE encrypt_type);

typedef void(*NETCAM_WIFI)(char  *ssid, char *password, WIFI_ENC_MODE encrypt_type);
int	netcam_wifi_set_callback(NETCAM_WIFI callback_fun);




#endif

