#ifndef _ZBAR_QRCODE_LINK_H_
#define _ZBAR_QRCODE_LINK_H_


typedef int (*qrcode_parse_callback)(char *data,char *ssid,char *pw,char *enMode);

int zbar_qrcode_link_init(void);
int zbar_qrcode_link_deinit(void);
/*
 *	function: smart link connect
 *	note:	when smart link is working, net_wifi_cli_connecting is invild.
 */
int zbar_qrcode_link_start(void);
/*
 *	function: stop smart link connecting
 */
void zbar_qrcode_link_stop(void);

/*
 *	function: look status of smart link connect , judge to smart link whether success.
 */
extern int zbar_qrcode_link_status(void);


int zbar_qrcode_link_support(void);

void zbar_qrcode_set_parse_callback(qrcode_parse_callback parse_func);

void zbar_qrcode_set_yuv_channel(int streamId);

#endif

