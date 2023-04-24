#ifndef __NET_ARPING_H__
#define __NET_ARPING_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
	interface : 网卡
	ipaddr    : 检测的IP
	macaddr	  : 如果要获取检测IP的MAC，必须有18个字符的空间用于存取MAC
 	return 	  :
 				-1 : 读取网卡配置信息失败
 				0  : 检测的IP可以使用
 				1  : 检测的IP不可以使用
*/
extern int check_ip(char *interface, char *ipaddr, char *macaddr);

#ifdef __cplusplus
}
#endif


#endif //__NET_ARPING_H__

