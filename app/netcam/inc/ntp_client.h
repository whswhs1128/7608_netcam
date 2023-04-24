#ifndef NTP_CLIENT_INCLUDE_H
#define NTP_CLIENT_INCLUDE_H

/**
	使能ntp client
*/
void ntpc_enable();

/**
	不使能ntp client
*/
void ntpc_disable();

/**
	设置ntp server的地址
	ntpserver: ntp server的域名地址
	serLen:    域名地址的长度
	port: ntp server的端口号,  < 0表示使用默认端口号123
*/
void ntpc_set_addr(char *ntpserver, int serLen, int port);

/**
	设置ntp client的同步时间
	synctime: 同步时间单位s
*/
void ntpc_set_synctime(int synctime);

/**
	启动 ntp client
*/
int start_ntp_client();

#endif //NTP_CLIENT_INCLUDE_H

