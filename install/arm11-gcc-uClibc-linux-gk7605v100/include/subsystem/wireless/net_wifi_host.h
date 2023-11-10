#ifndef __NET_WIFI_HOST_H__
#define __NET_WIFI_HOST_H__

#define HOST_OPEN		0
#define HOST_WEP		1
#define HOST_WPA_TKIP	2
#define HOST_WPA_AES	3
#define HOST_WPA2_TKIP	4
#define HOST_WPA2_AES	5

#ifdef __cplusplus
extern "C" {
#endif
extern int net_wifi_hostap_init(void);
extern int net_wifi_hostap_deinit(void);
extern int net_wifi_host_on(const WIFI_HANDLE dev, const char *ip, const char *mask);
extern int net_wifi_host_reconfigure(const WIFI_HANDLE dev, const char *ssid,
	const char *passwd, int encryp_method);
extern int net_wifi_host_off(const WIFI_HANDLE dev);
extern int net_wifi_host_start_udhcpd(const WIFI_HANDLE dev);
extern int net_wifi_host_stop_udhcpd(void);
extern int net_wifi_host_get_startPool(char *startip, int size);
extern int net_wifi_host_get_endPool(char *endip, int size);
extern int net_wifi_host_get_SSID(char *ssid, int size);
extern int net_wifi_host_get_EncrypMethod(void);
extern int net_wifi_host_get_psk(int enrypty, char *psk, int size);
extern int net_wifi_hostap_is_init(void);
extern void net_wifi_hostap_set_init_ssid(char *ssid);
#ifdef __cplusplus
}
#endif
#endif

