#ifndef _NET_WIFI_SMARTLINK_H_
#define _NET_WIFI_SMARTLINK_H_
#include "net_wifi_commom.h"

typedef struct
{
    int (*startSmartLink)(void);
    int (*stopSmartLink)(void);
    int (*getSmartLinkInfo)(char *ssid, char *passwd, WIFI_ENC_MODE *enc_mode);
}NETCAM_SmartLinkFuncT;



int net_wifi_smartlink_init(void);
int net_wifi_smartlink_deinit(void);
int net_wifi_smartlink_register(NETCAM_SmartLinkFuncT *func);
/*
 *	function: smart link connect
 *	note:	when smart link is working, net_wifi_cli_connecting is invild.
 */
int net_wifi_smartlink_start(void);
/*
 *	function: stop smart link connecting
 */
void net_wifi_smartlink_stop(void);

/*
 *	function: look status of smart link connect , judge to smart link whether success.
 */
extern int net_wifi_smartlink_status(void);


int net_wifi_smartlink_support(void);

#endif

