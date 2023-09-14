#ifndef __NET_WIFI_COMMOM_H__
#define __NET_WIFI_COMMOM_H__

#define NETWORK_PRINT_INFO(fmt, args...)  \
    do{\
		char bname[64];\
		strncpy(bname, __FILE__, sizeof(bname));\
        printf("[NET INFO] [ %s, Line: %d ]  " fmt "\n", __FUNCTION__, __LINE__,  ##args); \
    }while(0)


#define NETWORK_PRINT_ERROR(fmt, args...)  \
		do{\
			char bname[64];\
			strncpy(bname, __FILE__, sizeof(bname));\
			printf("\033[0;31m[NET ERR] [ %s, Line:%d ]  " fmt "\033[0;0m\n",__FUNCTION__, __LINE__,  ##args); \
		}while(0)




typedef char * WIFI_HANDLE;
#define isNull(n) ((n) == NULL)

/* proc command  */
#define IFCONFIG_CMD	"/sbin/ifconfig"
#define REOUTE_CMD		"/sbin/route"
#define WPA_SUPPLICANT_CMD	"/usr/local/bin/wpa_supplicant"
#define IWLIST_CMD	"/usr/local/bin/iwlist"

#define HOSTAPD_CMD		"/usr/local/bin/hostapd"
#define UDHCPC_CMD		"/sbin/udhcpc"
#define SED_CMD			"/bin/sed"
#define INSMOD_CMD		"/sbin/insmod"
#define RMMOD_CMD		"/sbin/rmmod"
#define XARGS_CMD		"/usr/bin/xargs"
#define FIND_CMD		"/usr/bin/find"
#define LSMOD_CMD		"/sbin/lsmod"
#define GREP_CMD		"/bin/grep"
#define CAT_CMD			"/bin/cat"
#define KILL_CMD		"/bin/kill"
#define LSUSB_CMD		"/usr/bin/lsusb"
#define UDHCPD_CMD		"/usr/sbin/udhcpd"
#define AWK_CMD 		"/usr/bin/awk"
#define KILLALL_CMD 	"/usr/bin/killall"

//default load rt3070 drivers
#define DRIVER_MODULES			"/lib/modules"
/*/opt/resource/lib/modules soft link to /lib/modeules,bug find not support soft link*/
//#define DRIVER_MODULES  "/opt/resource/lib/modules"
#define	DRIVER_MT7601_STA		"mt7601Usta"
#define DRIVER_MT7601_AP		"mt7601Uap"
#define DRIVER_MT7601_AP_CFG	"/opt/custom/cfg/RT2870AP.dat"
#define NETWORK_ELIAN_SUPPORT	//support smart link
#define NETWORK_WPS_SUPPORT		//support WPS

#define	DRIVER_RTL8188			"8188eu"
#define	DRIVER_RTL8188FU		"8188fu"

#define UDHCPD_CFG		"/opt/custom/cfg/udhcpd.conf"
#ifdef AP_STA_SUPPORT
#define UDHCPD_IP		"192.168.1.1"
#define UDHCPD_GW		"192.168.1.1"
#define UDHCPD_NETMASK	"255.255.255.0"
#define UDHCPD_DNS		UDHCPD_GW
#define UDHCPD_ROUTE	UDHCPD_GW
#define UDHCPD_START	"192.168.1.100"
#define UDHCPD_END		"192.168.1.120"

#else
#define UDHCPD_GW		"192.168.252.1"
#define UDHCPD_IP		UDHCPD_GW
#define UDHCPD_NETMASK	"255.255.255.0"
#define UDHCPD_DNS		UDHCPD_GW
#define UDHCPD_ROUTE	UDHCPD_GW
#define UDHCPD_START	"192.168.252.100"
#define UDHCPD_END		"192.168.252.120"
#endif
#define	DRIVER_SCI9083_STA		"9083xu"
#define DRIVER_SCI9083_AP		"9083xu"

#define NET_WIFI_DEFAULT_BUS 		1
#define NET_WIFI_DEFAULT_DEVICENUM 	2

#define NET_WIFI_DEVICENAME_MT7601		"mt7601"
#define NET_WIFI_DRIVERNAME_RT2870		"rt2870drv"

#define NET_WIFI_DEVICENAME_RTL8188		"rtl8188"
#define NET_WIFI_DEVICENAME_RTL8188FTV		"rtl8188ftv"
#define NET_WIFI_DRIVERNAME_RTL817X		"rtl871xdrv"

#define NET_WIFI_DEVICENAME_SCI9083		"sci9083"
#define NET_WIFI_DRIVERNAME_SCI9083		"tilk908x"


#define ELIAN_INTVAL_TIME	1
#define MAX_WIFI_NAME   32
/* control command resulte*/
#define WIFI_OK             0
#define WIFI_FAIL           -1      //function fail
#define WIFI_TIMEOUT        -2      //Link tmeout
#define WIFI_NO_SSID        -3      //no ssid
#define WIFI_LINK_ERROR		-4      //link error


/* set network flags */
#define WIFI_SET_SSID       0
#define WIFI_SET_PSK        1
#define WIFI_SET_KEY_MGMT   2
#define WIFI_SET_INDENT     3
#define WIFI_SET_PASSWORD   4
#define WIFI_SET_BSSID   	5
#define WIFI_SET_SCAN_SSID	6
#define WIFI_SET_MAXNUM     6

#define WPA_CTL_INTERFACE "/var/run/wpa_supplicant"



#define NETWOR_TOOLS_CTRL_FILE     "/tmp/network_tools_ctrl"
#define NETWORK_TOOLS_CTRL_AP_MODE      "AP_MODE\n"
#define NETWORK_TOOLS_CTRL_STA_MODE     "STA_MODE\n"
#define NETWORK_TOOLS_CTRL_LAN_DHCP     "LAN_DHCP\n"
#define NETWORK_TOOLS_CTRL_WIFI_DHCP     "WIFI_DHCP\n"

typedef struct {
	unsigned int venderID;
	unsigned int deviceID;
} NET_WIFI_DeviceNumT;

typedef struct {
	unsigned int 	venderID;
	char 			venderName[12];
} NET_WIFI_MapVenderNameT;

typedef struct {
	NET_WIFI_DeviceNumT deviceNum;
	char 				deviceName[12];
	char 				driverName[12];
} NET_WIFI_MapDeviceNameT;

typedef enum
{
	WIFI_CONNECT_OK 				=  0,   //connect ok
 	WIFI_CONNECT_FAIL 				= -1,   //function fail
	WIFI_CONNECT_TIMEOUT   			= -2,   //Link tmeout
	WIFI_CONNECT_NO_SSID    		= -3,   //no ssid
	WIFI_CONNECT_PASSWORD_ERROR 	= -4,    //password error
	WIFI_DISCONNECT_OK 				= -8000,	//disconnect ok
	WIFI_DISCONNECT_FAILED			= -8001, //disconnect failed
	WIFI_CONNECT_STATUSERROR		= -9001 //status error


}WIFI_CONNECT_STATUS;

typedef enum
{
	WIFI_ENC_NONE,
	WIFI_ENC_WEP_64_ASSII,
	WIFI_ENC_WEP_64_HEX,
	WIFI_ENC_WEP_128_ASSII,
	WIFI_ENC_WEP_128_HEX,
	WIFI_ENC_WPAPSK_TKIP,
	WIFI_ENC_WPAPSK_AES,
	WIFI_ENC_WPA2PSK_TKIP,
	WIFI_ENC_WPA2PSK_AES

}WIFI_ENC_MODE;
typedef struct
{
	int						enable;  	// 0:disable 1: enable
	int						isConnect;  // 1: connet or 0: disconnect wireless
	WIFI_CONNECT_STATUS				linkStatus; //0 connected, other not connected
	char            		linkEssid[MAX_WIFI_NAME];
	char            		linkPsd[MAX_WIFI_NAME];
    WIFI_ENC_MODE   		linkScurity;
								//0: none  (wifi enable)
								//1: wep 64 assii (wifi enable)
								//2: wep 64 hex (wifi enable)
								//3: wep 128 assii (wifi enable)
								//4: wep 128 hex (wifi enable)
								//5: WPAPSK-TKIP
								//6: WPAPSK-AES
								//7: WPA2PSK-TKIP
								//8: WPA2PSK-AES
  int    					mode; //0 station ,1 ap
}WIFI_LINK_INFO_t;


#ifdef __cplusplus
extern "C" {
#endif
extern WIFI_HANDLE net_wifi_open(void);
extern void net_wifi_close(WIFI_HANDLE dev);
extern int net_wifi_up(const WIFI_HANDLE dev);
extern int net_wifi_down(const WIFI_HANDLE dev);
extern int net_wifi_get_mac(const WIFI_HANDLE dev,  char * const macstr);
/* Note: use after setting MAC function, equipment will be down  */
extern int net_wifi_set_mac(const WIFI_HANDLE dev, const char *macstr);

extern int net_wifi_get_mask(const WIFI_HANDLE dev,  char * const netmask);
extern int net_wifi_set_mask(const WIFI_HANDLE dev,  const char *  netmask);

extern int net_wifi_get_ip(const WIFI_HANDLE dev,  char * const ipaddr);
extern int net_wifi_set_ip(const WIFI_HANDLE dev,  const char * ipaddr);

extern int net_wifi_load_driver(const char *driver_name);
extern int net_wifi_unload_driver(const char *driver_name);
extern int net_wifi_check_driver_exist(const char *driver_name);
/* get usb device ID */
extern int net_wifi_getUsbDeviceID(NET_WIFI_DeviceNumT *device, int bus, int
	deviceNum);
extern int net_wifi_getVenderNameByDeviceNum(NET_WIFI_DeviceNumT device, char
	**pName);
extern int net_wifi_getDeviceNameByDeviceNum(NET_WIFI_DeviceNumT device, char
	**pName);
extern int net_wifi_getDriverNameByDeviceNum(NET_WIFI_DeviceNumT device, char **pName);
extern int wifi_find_seq(const WIFI_HANDLE dev, const char *ssid);
/* get all wifi dev name */
//extern int net_wifi_get_wifidevlist(WIFI_DEV_ListT *listbuf, int list_len);
#ifdef __cplusplus
}
#endif



#endif

