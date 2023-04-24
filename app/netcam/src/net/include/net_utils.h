#ifndef __NET_UTILS_H__
#define __NET_UTILS_H__

#include <arpa/inet.h>

#ifdef __cplusplus
extern "C" {
#endif
/*
 *  function:ip string convert to network byte stream.
 */
extern void net_utils_ipstr2byte(unsigned char * const bytes,const char *ip);
/*
 *  function:network byte stream convert to ip string.
 */
extern void net_utils_ipbyte2str(unsigned char * const ip_num, char *ip);
/*
 *  function:network byte stream convert to ip string.
 */

extern int net_utils_macstrtobyte(unsigned char * mac, const char* mac_str, int mac_len);
extern int net_utils_get_ifaddr(char *eth, char * const ip, char * const mac);

#ifdef __cplusplus
}
#endif


#endif


