
#ifndef __NET_DHCP_H__
#define __NET_DHCP_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <pthread.h>

/*
 other net-utils
*/
/*
*	note: found available ip
*	return:
*		failed return -1, return 0 unused, other number of ip found
*	parameter:
*		-- sz_l_ip: ip address
*		-- sz_l_netmask: netmask
*		-- ip_start: ip address to search start(null to use default: network + 15)
*		-- ip_end : ip address to search end (0xffffffff)
*		-- need_n: the number of available IPs needed
*		-- multi_tasks : the number of pallel tasks to use when searching available IPs
*		-- timeout_s : timeout in second
*		-- hook : search found hook
*/
typedef void (*f_ip_found)(char *origin_ip, char *origin_netmask, char *szip, char *szmac);

extern void NET_get_ip_conflict(void);
extern int NET_adapt_ip(char *shelf_ip, char *client_ip, char *ip_mask, f_ip_found hook);

extern int g_ip_conflict_flag;
#ifdef __cplusplus
}
#endif

#endif
