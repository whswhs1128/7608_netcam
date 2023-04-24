#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <signal.h>
//#include <netinet/ip.h>
//#include <netinet/in.h>
#include <string.h>
//#include <arpa/inet.h>
//#include <netinet/ip_icmp.h>
//#include <sys/socket.h>
//#include <net/if.h>
#include <sys/ioctl.h>
#include <linux/wireless.h>
#include <sys/socket.h>
#include <net/if_arp.h>
#include <linux/if_ether.h>
#include <netinet/in.h>

#include "net_wifi_commom.h"
#include "net_utils.h"
#include "net_wifi_host.h"
//#include "sdk_cfg.h"

#define DEFCFG_DIR "/usr/local/defcfg/"

#define CMD_LEN_MIN  	64
#define CMD_LEN_MAX  	128

#define DEFAULT_HOST_SSID	"IPC_GOKE_AP[NONE]"
//#define HOST_CFG "/tmp/hostapd.conf"

/*************** host AP label ******************/
#define LABEL_SSID			"SSID"
#define LABLE_HIDE_SSID		"HideSSID"
#define LABEL_AUTH_MODE		"AuthMode"
#define LABEL_ENCRYP_TYPE	"EncrypType"
#define LABEL_PASSWORD		"WPAPSK"
#define LABEL_WEP_KEY		"Key1Str"


//params of hide ssid
#ifdef LABLE_HIDE_SSID
#define VALUE_HIDE_SSID_OFF		0
#define VALUE_HIDE_SSID_ON		1
#endif

//auth mode
#ifdef LABEL_AUTH_MODE
#define VALUE_AUTH_MODE_OPEN	"OPEN"
#define VALUE_AUTH_MODE_SHARED	"SHARED"
#define VALUE_AUTH_MODE_WPAPSK	"WPAPSK"
#define VALUE_AUTH_MODE_WPA2PSK	"WPA2PSK"
#define VALUE_AUTH_MODE_WPANONE	"WPANONE"
#endif

//Encryp type
#ifdef LABEL_ENCRYP_TYPE
#define VALUE_ENCRYP_TYPE_NONE	"NONE"
#define VALUE_ENCRYP_TYPE_WEP	"WEP"
#define VALUE_ENCRYP_TYPE_TKIP	"TKIP"
#define VALUE_ENCRYP_TYPE_AES	"AES"
#endif
/*************** udhcpd label ******************/
#define LABEL_UDHCPD_IF			"interface"
#define LABEL_UDHCPD_START		"start"
#define LABEL_UDHCPD_END		"end"
#define LABEL_UDHCPD_OPT		"opt"
#define LABEL_UDHCPD_OPTION		"option"
#define LABEL_UDHCPD_DNS		"dns"
#define LABEL_UDHCPD_SUBNET		"subnet"
#define LABEL_UDHCPD_ROUTER		"router"
/*************** hostapd label ******************/
#define LABEL_HOSTAPD_IF		"interface"
#define LABEL_HOSTAPD_DRIVER	"driver"
#define LABEL_HOSTAPD_SSID		"ssid"
#define LABEL_HOSTAPD_WPA		"wpa"
#define LABEL_HOSTAPD_KEYMGMT	"wpa_key_mgmt"
#define LABEL_HOSTAPD_WPA_PAIR	"wpa_pairwise"
#define LABEL_HOSTAPD_RSN_PAIR	"rsn_pairwise"
#define LABEL_HOSTAPD_WEP_key0	"wep_key0"
#define LABEL_HOSTAPD_PWD		"wpa_passphrase"
#define LABEL_HOSTAPD_WEP		"wep_default_key"

/*************** hostapd value ******************/
#define VALUE_HOSTAPD_OPEN		"OPEN"
#define VALUE_HOSTAPD_WEP		"WPA-EAP"
#define VALUE_HOSTAPD_WPA_PSK	"WPA-PSK"
#define VALUE_HOSTAPD_0			"0"
#define VALUE_HOSTAPD_1			"1"
#define VALUE_HOSTAPD_2			"2"
#define VALUE_HOSTAPD_TKIP		"TKIP"
#define VALUE_HOSTAPD_CCMP		"CCMP"
#define VALUE_HOSTAPD_TKIP_CCMP	"TKIP CCMP"
#define VALUE_HOSTAPD_EMPTY		" "


#define HOSTAPD_CFG		"/opt/custom/cfg/hostapd.conf"


/* support hostapd */
static int support_hostapd = 0;
static char support_driver[16] = {0};
static int hostapd_init = 0;
static char wifi_ssid[64]= {0};

static int _getStringBySystem(const char *cmd, char *buff, int bufsize)
{
    FILE *fp = NULL;
	NETWORK_PRINT_INFO("==> %s\n", cmd);
    if (NULL == (fp = popen(cmd, "r")))
    {
        goto err;
    }
    if (buff != NULL && NULL == fgets(buff, bufsize, fp))
    {
        goto err;
    }
    pclose(fp);
    return 0;
err:
	pclose(fp);
    return -1;
}

static int _net_wifi_host_probe(void)
{
	int ret = 0;
	char *wifi_device = NULL;
	NET_WIFI_DeviceNumT device_info;
	memset(&device_info, 0, sizeof(NET_WIFI_DeviceNumT));
	ret = net_wifi_getUsbDeviceID(&device_info,
		NET_WIFI_DEFAULT_BUS, NET_WIFI_DEFAULT_DEVICENUM);
	if(ret < 0) {
		NETWORK_PRINT_ERROR("getUsbDeviceID is failed");
		return ret;
	}
	ret = net_wifi_getDeviceNameByDeviceNum(device_info, &wifi_device);
	if(ret < 0 || wifi_device == NULL) {
		NETWORK_PRINT_ERROR("getDeviceNameByDeviceNum is failed");
		return ret;
	}
	//mt7601 support smartlink and wps.
	if(strcmp(wifi_device, NET_WIFI_DEVICENAME_MT7601) == 0) {
		support_hostapd = 0;
	} else if(strcmp(wifi_device, NET_WIFI_DEVICENAME_RTL8188) == 0) {
	//rtl8188eu no support smartlink.
		char *dri_name = NULL;
		ret = net_wifi_getDriverNameByDeviceNum(device_info, &dri_name);
		if(dri_name == NULL) {
			return -1;
		}
		strncpy(support_driver, dri_name, sizeof(support_driver));
		support_hostapd = 1;
	} else if(strcmp(wifi_device, NET_WIFI_DEVICENAME_RTL8188FTV) == 0) {
	//rtl8188eu no support smartlink.
		char *dri_name = NULL;
		ret = net_wifi_getDriverNameByDeviceNum(device_info, &dri_name);
		if(dri_name == NULL) {
			return -1;
		}
		strncpy(support_driver, dri_name, sizeof(support_driver));
		support_hostapd = 1;
	} else if(strcmp(wifi_device, NET_WIFI_DEVICENAME_SCI9083) == 0) {
	//sc9083 no support smartlink.
		char *dri_name = NULL;
		ret = net_wifi_getDriverNameByDeviceNum(device_info, &dri_name);
		if(dri_name == NULL) {
			return -1;
		}
		strncpy(support_driver, dri_name, sizeof(support_driver));
		support_hostapd = 1;
	}
	return 0;
}

static int _wifi_hostapd_edit_cfgfile(const char *label, const char *value)
{
	int ret = 0;
	char cmd[CMD_LEN_MAX] = {0};
	if(isNull(value) || isNull(label)) {
		NETWORK_PRINT_ERROR("Bad parm");
		return -1;
	}
	snprintf(cmd, sizeof(cmd),"%s -i \"s/^%s=.*$/%s=%s/\" %s",
		SED_CMD, label, label, value, HOSTAPD_CFG);

	ret = _getStringBySystem(cmd, NULL, 0);
	if(ret < 0){
		printf("Net wifi edit configure file:%s label:%s", HOSTAPD_CFG, label);
	}
	return ret;
}

static int _wifi_hostapd_get_cfgfile(const char *label, char *value, int size)
{
	int ret = 0;
	char cmd[CMD_LEN_MAX] = {0};
	if(isNull(value) || isNull(label) || size <= 0) {
		NETWORK_PRINT_ERROR("Bad parm");
		return -1;
	}
	snprintf(cmd, sizeof(cmd),"%s '/^%s=/{print $1}' %s|%s -e \"2d\" -e \"s/^%s=//\"",
		AWK_CMD, label, HOSTAPD_CFG, SED_CMD, label);

	ret = _getStringBySystem(cmd, value, size);
	if(ret < 0){
		printf("Net wifi edit configure file:%s", HOSTAPD_CFG);
	}
	return ret;
}


static int _wifi_host_edit_cfgfile(const char *label, const char *value)
{
	int ret = 0;
	char cmd[CMD_LEN_MAX] = {0};
	if(isNull(value) || isNull(label)) {
		NETWORK_PRINT_ERROR("Bad parm");
		return -1;
	}
	snprintf(cmd, sizeof(cmd),"%s -i \"s/^%s=.*$/%s=%s/\" %s",
		SED_CMD, label, label, value, DRIVER_MT7601_AP_CFG);

	ret = _getStringBySystem(cmd, NULL, 0);
	if(ret < 0){
		printf("Net wifi edit configure file:%s label:%s", DRIVER_MT7601_AP_CFG, label);
	}
	return ret;
}

static int _wifi_host_get_cfgfile(const char *label, char *value, int size)
{
	int ret = 0;
	char cmd[CMD_LEN_MAX] = {0};
	if(isNull(value) || isNull(label) || size <= 0) {
		NETWORK_PRINT_ERROR("Bad parm");
		return -1;
	}
	snprintf(cmd, sizeof(cmd),"%s '/^%s=/{print $1}' %s|%s -e \"2d\" -e \"s/^%s=//\"",
		AWK_CMD, label, DRIVER_MT7601_AP_CFG, SED_CMD, label);

	ret = _getStringBySystem(cmd, value, size);
	if(ret < 0){
		printf("Net wifi edit configure file:%s", DRIVER_MT7601_AP_CFG);
	}
	return ret;
}

static int _wifi_host_edit_udhcpdcfgfile2(const char *label1, const char *label2, const char *value)
{
	int ret = 0;
	char cmd[CMD_LEN_MAX] = {0};
	if(isNull(value) || isNull(label1)) {
		NETWORK_PRINT_ERROR("Bad parm");
		return -1;
	}
	snprintf(cmd, sizeof(cmd),"%s -i \"s/^%s\\s*%s.*$/%s %s %s/\" %s",
		SED_CMD, label1, label2, label1, label2, value, UDHCPD_CFG);

	ret = _getStringBySystem(cmd, NULL, 0);
	if(ret < 0){
		printf("Net wifi edit configure file:%s label:%s %s", UDHCPD_CFG, label1, label2);
	}
	return ret;
}

static int _wifi_host_edit_udhcpdcfgfile(const char *label, const char *value)
{
	int ret = 0;
	char cmd[CMD_LEN_MAX] = {0};
	if(isNull(value) || isNull(label)) {
		NETWORK_PRINT_ERROR("Bad parm");
		return -1;
	}
	snprintf(cmd, sizeof(cmd),"%s -i \"s/^%s.*$/%s %s/\" %s",
		SED_CMD, label, label, value, UDHCPD_CFG);

	ret = _getStringBySystem(cmd, NULL, 0);
	if(ret < 0){
		printf("Net wifi edit configure file:%s label:%s", UDHCPD_CFG, label);
	}
	return ret;
}

static int _wifi_host_get_udhcpdcfgfile(const char *label, char *value, int size)
{
	int ret = 0;
	char cmd[CMD_LEN_MAX] = {0};
	if(isNull(value) || isNull(label) || size <= 0) {
		NETWORK_PRINT_ERROR("Bad parm");
		return -1;
	}
	snprintf(cmd, sizeof(cmd),"%s '/^%s/{print $2}' %s|%s -e \"2d\"",
		AWK_CMD, label, UDHCPD_CFG, SED_CMD);

	ret = _getStringBySystem(cmd, value, size);
	if(ret < 0){
		printf("Net wifi edit configure file:%s", UDHCPD_CFG);
	}
	return ret;
}

static int _wifi_host_set_interface(const WIFI_HANDLE dev)
{
	if(isNull(dev))
		return -1;
	return _wifi_host_edit_udhcpdcfgfile(LABEL_UDHCPD_IF, dev);
}

static int _wifi_host_set_dns(const char *dns)
{
	if(isNull(dns))
		return -1;
	return _wifi_host_edit_udhcpdcfgfile2(LABEL_UDHCPD_OPT, LABEL_UDHCPD_DNS, dns);
}

static int _wifi_host_set_subnet(const char *subnet)
{
	if(isNull(subnet))
		return -1;
	return _wifi_host_edit_udhcpdcfgfile2(LABEL_UDHCPD_OPTION, LABEL_UDHCPD_SUBNET, subnet);
}

static int _wifi_host_set_router(const char *router)
{
	if(isNull(router))
		return -1;
	return _wifi_host_edit_udhcpdcfgfile2(LABEL_UDHCPD_OPT, LABEL_UDHCPD_ROUTER, router);
}

static int _wifi_host_set_startPool(const char *startip)
{
	if(isNull(startip))
		return -1;
	return _wifi_host_edit_udhcpdcfgfile(LABEL_UDHCPD_START, startip);
}

static int _wifi_host_set_endPool(const char *endip)
{
	if(isNull(endip))
		return -1;
	return _wifi_host_edit_udhcpdcfgfile(LABEL_UDHCPD_END, endip);
}
static int _wifi_host_set_SSID(const char *ssid)
{
	if(support_hostapd) {
		if(isNull(ssid))
			return _wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_SSID, DEFAULT_HOST_SSID);
		else
			return _wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_SSID, ssid);
	} else {
		if(isNull(ssid))
			return _wifi_host_edit_cfgfile(LABEL_SSID, DEFAULT_HOST_SSID);
		else
			return _wifi_host_edit_cfgfile(LABEL_SSID, ssid);
	}
}
static int _wifi_host_set_authmode(const char *mode)
{
	if(isNull(mode))
		return -1;
	else
		return _wifi_host_edit_cfgfile(LABEL_AUTH_MODE, mode);
}

static int _wifi_host_get_authmode(char *mode, int size)
{
	if(isNull(mode))
		return -1;
	else
		return _wifi_host_get_cfgfile(LABEL_AUTH_MODE, mode, size);
}

static int _wifi_host_set_encryptype(const char *encryp_type)
{
	if(isNull(encryp_type))
		return -1;
	else
		return _wifi_host_edit_cfgfile(LABEL_ENCRYP_TYPE, encryp_type);
}

static int _wifi_host_get_encryptype(char *encryp_type, int size)
{
	if(isNull(encryp_type))
		return -1;
	else
		return _wifi_host_get_cfgfile(LABEL_ENCRYP_TYPE, encryp_type, size);
}

static int _wifi_host_set_password(const char *key)
{
	if(isNull(key))
		return -1;
	else if(support_hostapd)
		return _wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_PWD, key);
	else
		return _wifi_host_edit_cfgfile(LABEL_PASSWORD, key);
}

static int _wifi_host_get_password(char *key, int size)
{
	if(isNull(key))
		return -1;
	else if(support_hostapd)
		return _wifi_hostapd_get_cfgfile(LABEL_HOSTAPD_PWD, key, size);
	else
		return _wifi_host_get_cfgfile(LABEL_PASSWORD, key, size);
}

static int _wifi_host_set_wep_key(const char *key)
{
	if(isNull(key))
		return -1;
	else if(support_hostapd)
		return _wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_WEP_key0, key);
	else
		return _wifi_host_edit_cfgfile(LABEL_WEP_KEY, key);
}

static int _wifi_host_get_wep_key(char *key, int size)
{
	if(isNull(key))
		return -1;
	else if(support_hostapd)
		return _wifi_hostapd_get_cfgfile(LABEL_HOSTAPD_WEP_key0, key, size);
	else
		return _wifi_host_get_cfgfile(LABEL_WEP_KEY, key, size);
}

static void _wifi_cli_udcpd(void)
{
	char cmd[64] = {0};
	snprintf(cmd, sizeof(cmd), "%s -fS %s &", UDHCPD_CMD, UDHCPD_CFG);
	new_system_call("killall udhcpd");
	new_system_call(cmd);
}

static void _wifi_host_reload_configure(const WIFI_HANDLE dev)
{
	char cmd[64] = {0};
	net_wifi_down(dev);
	if(support_hostapd){
		snprintf(cmd, sizeof(cmd), "%s hostapd", KILLALL_CMD);
		new_system_call(cmd);
	}
	net_wifi_up(dev);
	if(support_hostapd){
		memset(cmd, 0, sizeof(cmd));
		snprintf(cmd, sizeof(cmd),"%s %s ", HOSTAPD_CMD, HOSTAPD_CFG);
	    new_system_call(cmd);
	}
}


int net_wifi_host_set_EncrypMethod(int encryp_method)
{
	if(support_hostapd) {
		switch(encryp_method)
		{
			case HOST_OPEN:
				_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_WEP, VALUE_HOSTAPD_EMPTY);
				_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_WPA, VALUE_HOSTAPD_0);
				_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_KEYMGMT, VALUE_HOSTAPD_OPEN);
				break;
			case HOST_WEP:
				_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_WEP, VALUE_HOSTAPD_0);
				_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_WPA, VALUE_HOSTAPD_0);
				_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_KEYMGMT, VALUE_HOSTAPD_WEP);
				break;
			case HOST_WPA_TKIP:
				_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_WEP, VALUE_HOSTAPD_EMPTY);
				_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_WPA, VALUE_HOSTAPD_1);
				_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_KEYMGMT, VALUE_HOSTAPD_WPA_PSK);
				_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_WPA_PAIR, VALUE_HOSTAPD_TKIP);
				break;
			case HOST_WPA_AES:
				_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_WEP, VALUE_HOSTAPD_EMPTY);
				_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_WPA, VALUE_HOSTAPD_1);
				_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_KEYMGMT, VALUE_HOSTAPD_WPA_PSK);
				_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_WPA_PAIR, VALUE_HOSTAPD_CCMP);
				break;
			case HOST_WPA2_TKIP:
				_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_WEP, VALUE_HOSTAPD_EMPTY);
				_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_WPA, VALUE_HOSTAPD_2);
				_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_KEYMGMT, VALUE_HOSTAPD_WPA_PSK);
				_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_RSN_PAIR, VALUE_HOSTAPD_TKIP);
				break;
			case HOST_WPA2_AES:
				_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_WEP, VALUE_HOSTAPD_EMPTY);
				_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_WPA, VALUE_HOSTAPD_2);
				_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_KEYMGMT, VALUE_HOSTAPD_WPA_PSK);
				_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_RSN_PAIR, VALUE_HOSTAPD_CCMP);
				break;
			default:
				return -1;
		}
	} else {
		switch(encryp_method)
		{
			case HOST_OPEN:
				_wifi_host_set_authmode(VALUE_AUTH_MODE_OPEN);
				_wifi_host_set_encryptype(VALUE_ENCRYP_TYPE_NONE);
				break;
			case HOST_WEP:
				_wifi_host_set_authmode(VALUE_AUTH_MODE_SHARED);
				_wifi_host_set_encryptype(VALUE_ENCRYP_TYPE_WEP);
				break;
			case HOST_WPA_TKIP:
				_wifi_host_set_authmode(VALUE_AUTH_MODE_WPAPSK);
				_wifi_host_set_encryptype(VALUE_ENCRYP_TYPE_TKIP);
				break;
			case HOST_WPA_AES:
				_wifi_host_set_authmode(VALUE_AUTH_MODE_WPAPSK);
				_wifi_host_set_encryptype(VALUE_ENCRYP_TYPE_AES);
				break;
			case HOST_WPA2_TKIP:
				_wifi_host_set_authmode(VALUE_AUTH_MODE_WPA2PSK);
				_wifi_host_set_encryptype(VALUE_ENCRYP_TYPE_TKIP);
				break;
			case HOST_WPA2_AES:
				_wifi_host_set_authmode(VALUE_AUTH_MODE_WPA2PSK);
				_wifi_host_set_encryptype(VALUE_ENCRYP_TYPE_AES);
				break;
			default:
				return -1;
		}
	}
	return 0;
}

int net_wifi_host_get_EncrypMethod(void)
{
	char authMode[16] = {0};
	char encryptMode[16] = {0};
	char pair[16] = {0};
	if(support_hostapd){
		_wifi_hostapd_get_cfgfile(LABEL_HOSTAPD_KEYMGMT, encryptMode, sizeof(encryptMode));
		if(strncmp(encryptMode, VALUE_HOSTAPD_OPEN, strlen(VALUE_HOSTAPD_OPEN)) == 0)
			return HOST_OPEN;
		else if(strncmp(encryptMode, VALUE_HOSTAPD_WEP, strlen(VALUE_HOSTAPD_WEP)) == 0)
			return HOST_WEP;
		else if(strncmp(encryptMode, VALUE_HOSTAPD_WPA_PSK, strlen(VALUE_HOSTAPD_WPA_PSK)) == 0) {
			_wifi_hostapd_get_cfgfile(LABEL_HOSTAPD_WPA, authMode, sizeof(authMode));
			if(strncmp(authMode, VALUE_HOSTAPD_1, strlen(VALUE_HOSTAPD_1)) == 0){
				_wifi_hostapd_get_cfgfile(LABEL_HOSTAPD_WPA_PAIR, pair, sizeof(pair));
				if(strncmp(pair, VALUE_HOSTAPD_TKIP, strlen(VALUE_HOSTAPD_TKIP)) == 0)
					return HOST_WPA_TKIP;
				else
					return HOST_WPA_AES;
			} else {
				_wifi_hostapd_get_cfgfile(LABEL_HOSTAPD_RSN_PAIR, pair, sizeof(pair));
				if(strncmp(pair, VALUE_HOSTAPD_TKIP, strlen(VALUE_HOSTAPD_TKIP)) == 0)
					return HOST_WPA2_TKIP;
				else
					return HOST_WPA2_AES;
			}
		} else
			return HOST_OPEN;
	} else {
		_wifi_host_get_authmode(authMode, sizeof(authMode));
		_wifi_host_get_encryptype(encryptMode, sizeof(encryptMode));
		if(strncmp(authMode, VALUE_AUTH_MODE_OPEN, strlen(VALUE_AUTH_MODE_OPEN)) == 0)
			return HOST_OPEN;
		else if(strncmp(authMode, VALUE_AUTH_MODE_SHARED, strlen(VALUE_AUTH_MODE_SHARED)) == 0)
			return HOST_WEP;
		else if(strncmp(authMode, VALUE_AUTH_MODE_WPAPSK, strlen(VALUE_AUTH_MODE_WPAPSK)) == 0) {
			if(strncmp(encryptMode, VALUE_ENCRYP_TYPE_TKIP, strlen(VALUE_ENCRYP_TYPE_TKIP)) == 0)
				return HOST_WPA_TKIP;
			else
				return HOST_WPA_AES;
		} else if(strncmp(authMode, VALUE_AUTH_MODE_WPAPSK, strlen(VALUE_AUTH_MODE_WPAPSK)) == 0) {
			if(strncmp(encryptMode, VALUE_ENCRYP_TYPE_TKIP, strlen(VALUE_ENCRYP_TYPE_TKIP)) == 0)
				return HOST_WPA2_TKIP;
			else
				return HOST_WPA2_AES;
		} else
			return HOST_OPEN;
	}
}

int net_wifi_host_get_psk(int enrypty, char *psk, int size)
{
	if(psk == NULL || size <= 0)
		return -1;
	if(enrypty == HOST_WEP) {
		return _wifi_host_get_wep_key(psk, size);
	} else {
		return _wifi_host_get_password(psk, size);
	}
}
int net_wifi_host_get_SSID(char *ssid, int size)
{
	if(isNull(ssid))
		return -1;
	else if(support_hostapd)
		return _wifi_hostapd_get_cfgfile(LABEL_HOSTAPD_SSID, ssid, size);
	else
		return _wifi_host_get_cfgfile(LABEL_SSID, ssid, size);
}


int net_wifi_host_reconfigure(const WIFI_HANDLE dev, const char *ssid,
	const char *passwd, int encryp_method)
{
	int ret = 0;
	if(isNull(dev) || (isNull(passwd) && (encryp_method != HOST_OPEN))){
		NETWORK_PRINT_ERROR("Bad params\n");
		return -1;
	}
	ret = _wifi_host_set_SSID(ssid);//set ssid
	if(ret < 0)
		return -1;

	ret = net_wifi_host_set_EncrypMethod(encryp_method);//set encryp  method
	if(ret < 0)
		return -1;
	//set password
	if(!isNull(passwd)) {
		if(encryp_method == HOST_WEP) {
			_wifi_host_set_wep_key(passwd);
		} else {
			_wifi_host_set_password(passwd);
		}
	}
	_wifi_host_reload_configure(dev);

	return 0;
}

int net_wifi_host_start_udhcpd(const WIFI_HANDLE dev)
{
	int ret = _wifi_host_set_interface(dev);
	_wifi_host_set_startPool(UDHCPD_START);
	_wifi_host_set_endPool(UDHCPD_END);
	_wifi_host_set_dns(UDHCPD_DNS);
	_wifi_host_set_subnet(UDHCPD_NETMASK);
	_wifi_host_set_router(UDHCPD_ROUTE);
	//_wifi_cli_udcpd();
	return ret;
}

int net_wifi_host_stop_udhcpd(void)
{
	new_system_call("killall udhcpd");
	return 0;
}

int net_wifi_host_get_endPool(char *endip, int size)
{
	if(isNull(endip))
		return -1;
	return _wifi_host_get_udhcpdcfgfile(LABEL_UDHCPD_END, endip, size);
}


int net_wifi_host_get_startPool(char *startip, int size)
{
	if(isNull(startip))
		return -1;
	return _wifi_host_get_udhcpdcfgfile(LABEL_UDHCPD_START, startip, size);
}


int net_wifi_host_on(const WIFI_HANDLE dev, const char *ip,
	const char *mask)
{
	int ret = 0;
	char cmd[128] = {0};
	if(isNull(dev)){
		perror("net_wifi_host_init");
		return -1;
	}

	_net_wifi_host_probe();
	if(support_hostapd)
    {

        char cmd_tmp[64] = {0};
        net_wifi_down(dev);

        new_system_call("killall wpa_supplicant");
		//snprintf(cmd_tmp, sizeof(cmd_tmp), "%s hostapd", KILLALL_CMD);
		//new_system_call(cmd_tmp);
	}

    //system("ifconfig");
	net_wifi_up(dev);
    if(ip != NULL) {
    	ret = net_wifi_set_ip(dev, ip);
    	if(ret != 0){
    		perror("net_wifi_host_init");
    		return -1;
    	}
    } else {
    	ret = net_wifi_set_ip(dev, UDHCPD_IP);
    	if(ret != 0){
    		perror("net_wifi_host_init");
    		return -1;
    	}
    }

	if(mask != NULL){
    	/* if mask no equal NULL, should set netmask */
    	ret = net_wifi_set_mask(dev, mask);
    	if(ret != 0){
    		perror("net_wifi_host_init");
    		return -1;
    	}
	}
	//_net_wifi_host_probe();
	if(support_hostapd) {

		if(access(HOSTAPD_CFG, F_OK))
        {
            NETWORK_PRINT_INFO("--------hostapd.conf not exit------\n");
            if(copy_file(DEFCFG_DIR"hostapd.conf", HOSTAPD_CFG))
            {

                NETWORK_PRINT_ERROR("copy_file /usr/local/defcfg/hostapd.conf error\n");
            }
            else
            {
                NETWORK_PRINT_INFO("--------copy hostapd.conf success------\n");
            }
		}
        else
        {
            NETWORK_PRINT_INFO("-------hostapd.conf eixt-------\n");
        }
		/* edit configure file, include: interface, driver */
		_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_IF, dev);
		_wifi_hostapd_edit_cfgfile(LABEL_HOSTAPD_DRIVER, support_driver);
        char ssid[32] = {0};
        char mac[32] = {0};
        char mac_end[6] = {0};
        //net_wifi_get_mac(dev,mac);
        system("ifconfig");
        sdk_net_get_hwaddr(dev,mac);
        NETWORK_PRINT_INFO("get %s mac :%s\n",dev,mac);
        strncpy(mac_end,mac + 9 ,2);
        strncpy(mac_end+2,mac + 12 ,2);
        strncpy(mac_end+4,mac + 15 ,2);

        //对接柔乐app使用AP_开头的ssid名字
        //snprintf(ssid,sizeof(ssid),"IPCam_%s",mac_end);
        //snprintf(ssid,sizeof(ssid),"AP_%s",mac_end);//
        if (strlen(wifi_ssid) > 0)
        {
            strcpy(ssid, wifi_ssid);
        }
        else
        {
            strcpy(ssid, "IPCamera");
        }

        NETWORK_PRINT_INFO("get mac:%s,mac_end:%s,set ssid:%s\n",mac,mac_end,ssid);
        ret = _wifi_host_set_SSID(ssid);//set ssid
        if(ret < 0)
        {
            printf("_wifi_host_set_SSID failed\n");

        }
		/* start wpa_suppliant */
	    //memset(cmd, 0, sizeof(cmd));
		//snprintf(cmd, sizeof(cmd),"%s %s &", HOSTAPD_CMD, HOSTAPD_CFG);
	    //ret = new_system_call(cmd);
        //NETWORK_PRINT_INFO(" ---xxxx hsotapd run 1 ret:%d\n ",ret);

	}
	net_wifi_host_start_udhcpd(dev);
	sdk_network_tools_ctrl(NETWOR_TOOLS_CTRL_FILE, NETWORK_TOOLS_CTRL_AP_MODE);

    memset(cmd,0,sizeof(cmd));
    #if 0
    sprintf(cmd,"route add -host 255.255.255.255 dev %s",dev);
    ret = new_system_call(cmd);
    printf("xxxxx cmd:%s ret = %d\n ",cmd,ret);
    #endif

    return 0;
}

int net_wifi_host_off(const WIFI_HANDLE dev)
{
	if(support_hostapd) {
		char cmd[64]= {0};
		snprintf(cmd, sizeof(cmd), "%s hostapd", KILLALL_CMD);
		new_system_call(cmd);
	}
	net_wifi_host_stop_udhcpd();
	net_wifi_down(dev);
	return 0;
}

int net_wifi_hostap_is_init(void)
{
    return hostapd_init;
}
int net_wifi_hostap_init(void)
{
    return hostapd_init = 1;
}

int net_wifi_hostap_deinit(void)
{
    return hostapd_init = 0;
}

void net_wifi_hostap_set_init_ssid(char *ssid)
{
    strcpy(wifi_ssid, ssid);
}


