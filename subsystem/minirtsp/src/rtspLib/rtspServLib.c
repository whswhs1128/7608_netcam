/*!
*****************************************************************************
** \file      $gkprjrtspServLib.c
**
** \version	$id: rtspServLib.c 15-08-08  8月:08:1439014790 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#include <string.h>

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "rtspServLib.h"
#include "rtspServ.h"
#include "utils.h"


/**
 * Ladies and gentlemen:
 * Please read following notes before you change the version number!
 * 
 * If you've added some big or new features or made big changes,
 * then you need increase the MAJOR number.
 * Else if you've changed little things or just fixed some small bugs,
 * increased the MINOR number was enough.
 */
#define RTSPSERV_BUILD_VERSION "v.0.1"     /* Version format: MAJOR.MINOR.BUGFIXED */
#define RTSPSERV_BUILD_DATE "2015-07-31"

rtsp_serv_t rtsp_serv;

/**
 * Print RTSP library version in pretty format.
 */
static void dump_version(void)
{
	printf ("\n    \033[32m**********************************************************\033[0m\n"
		"    \033[1;32m*   rtspServLib build information: [%s, %s]    *\033[0m\n"
		"    \033[32m**********************************************************\033[0m\n\n\n",
		RTSPSERV_BUILD_VERSION, RTSPSERV_BUILD_DATE);
	return;
}

int st_rtsp_initLib(void)
{
	dump_version();

	memset(&rtsp_serv, 0, sizeof(rtsp_serv_t));

	rtsp_serv.allsessions   = j_list_alloc();
	rtsp_serv.allmulticasts = j_list_alloc();
	rtsp_serv.allroutes     = j_list_alloc();

	pthread_mutex_init(&rtsp_serv.sess_list_lock, NULL);
	pthread_mutex_init(&rtsp_serv.mcast_list_lock, NULL);
	pthread_mutex_init(&rtsp_serv.route_list_lock, NULL);

	rtsp_serv.rtspsock = -1;
	rtsp_serv.httpsock = -1;

	rtsp_serv.rtsp_port = DFL_RTSP_PORT;
	rtsp_serv.http_port = DFL_HTTP_PORT;

	rtsp_serv.enable_rtcp   = 1;
	rtsp_serv.rtcp_interval = 5;	/* 5 second */

	return 0;
}

int st_rtsp_deinitLib(void)
{
	pthread_mutex_destroy(&rtsp_serv.sess_list_lock);
	pthread_mutex_destroy(&rtsp_serv.mcast_list_lock);
	pthread_mutex_destroy(&rtsp_serv.route_list_lock);

	j_list_free(rtsp_serv.allsessions);
	j_list_free(rtsp_serv.allmulticasts);

	if (rtsp_serv.allroutes) {
		JListNode *n;
		while ((n = j_list_first(rtsp_serv.allroutes)) != NULL) {
			RtspRoute *r = n->data;
			j_list_remove(rtsp_serv.allroutes, r);
			free(r);
		}

		j_list_free(rtsp_serv.allroutes);
	}

	return 0;
}

int st_rtsp_addRoute(const char *route, RtspOps *ops)
{
	if (!ops || !ops->read || !ops->describe)
		return -1;

	RtspRoute *r = malloc(sizeof(RtspRoute));
	if (!r)
		return -1;
	memset(r, 0, sizeof(RtspRoute));

	strncpy(r->route, route, sizeof(r->route) - 1);
	memcpy(&r->ops, ops, sizeof(RtspOps));

	pthread_mutex_lock(&rtsp_serv.route_list_lock);
	j_list_append(rtsp_serv.allroutes, r);
	pthread_mutex_unlock(&rtsp_serv.route_list_lock);

	return 0;
}

int st_rtsp_startService(const char *ip, int rtspport, int httpport)
{
	rtsp_serv.rtsp_port = rtspport;
	rtsp_serv.http_port = httpport;

	return rtsp_startRtspServ();
}

int st_rtsp_stopService(void)
{
	rtsp_stopRtspServ();
	return 0;
}

int st_rtsp_setupRtcp(int enable, int interval_s)
{
	rtsp_serv.enable_rtcp = enable;
	rtsp_serv.rtcp_interval = interval_s;
	return 0;
}

int st_rtsp_getVersion(char *version, char *buildDate)
{
	strcpy(version, RTSPSERV_BUILD_VERSION);
	sprintf(buildDate, "%s %s", __DATE__, __TIME__);
	return 0;
}

int st_rtsp_startMulticastStreaming(char *route)
{
	rtsp_session_t *sessp = rtsp_find_multicast_session(route);
	if (!sessp) {
		char uri[1024];
		snprintf(uri, sizeof(uri), "rtsp://127.0.0.1%s", route);

		sessp = rtsp_create_multicast_session(uri);
		if (!sessp)
			return -1;
	}

	sessp->mcast_refcount++;

	return 0;
}

int st_rtsp_stopMulticastStreaming(char *route)
{
	rtsp_session_t *sessp = rtsp_find_multicast_session(route);
	if (!sessp)
		return -1;

	sessp->mcast_refcount--;
	if (sessp->mcast_refcount == 0) {
		j_list_remove(rtsp_serv.allmulticasts, sessp);
		rtsp_closeRtspMcastSession(sessp);
	}

	return 0;
}
