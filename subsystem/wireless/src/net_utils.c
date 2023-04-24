

#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

#include <sys/socket.h>
#include <net/if.h>
#include <sys/ioctl.h>

#include "net_utils.h"


void net_ipbyte2str(unsigned char * const ip_num, char *ip)
{
	unsigned int *ip_addr = NULL;
	struct in_addr in;

	if(ip_num == NULL) return;
	ip_addr = (unsigned int *)ip_num;
	in.s_addr = htonl(*ip_addr);
    strcpy(ip, (char*)inet_ntoa(in));
}

int net_macstrtobyte(unsigned char* mac, const char* mac_str, int mac_len)
{
    int i;
    char c, val;
    for (i = 0; i < mac_len; i++) {
        if (!(c = tolower(*mac_str++))) {
            printf("Invalid hardware address!\n");
            return -1;
        }
        if ('0' <= c && c <= '9') {
            val = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            val = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            val = c - 'A' + 10;
        } else {
            printf("Invalid hardware address!\n");
            return -1;
        }
        *mac = val << 4;
        if (!(c = tolower(*mac_str++))) {
            printf("Invalid hardware address!\n");
            return -1;
        }
        if ('0' <= c && c <= '9') {
            val = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            val = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            val = c - 'A' + 10;
        } else {
            printf("Invalid hardware address!\n");
            return -1;
        }
        *mac++ |= val;
        if (*mac_str == ':') {
            mac_str++;
        } else {
            if (i < 5) {
                printf("Invalid hardware address 5 , %c!\n", *mac_str);
                return -1;
            }
        }
    }
    if (i < 6) {
        printf("Invalid hardware address 6!\n");
        return -1;
    }

    return 0;
}

//mac_len:number bit mac; default 6
int net_bytetomacstr(char * const mac, const unsigned char *mac_num, int mac_len)
{
	int str_len = 0, i = 0;

	if(mac_len <= 0)return -1;
	memset(mac, 0 , mac_len*3);
	for(i = 0; i < mac_len; i++)
	{
		sprintf(&mac[i*3], "%02X:", mac_num[i] & 0xff);
	}
	str_len = strlen(mac);
	if(str_len <= 0)return -1;
	mac[str_len - 1] = '\0'; //delete ':'

	return 0;
}

int net_chartoascii(const char *str, char *ascii_str)
{
    const char *src = str;
    char *des = ascii_str;
    const char ch_x[16] = {'0','1','2','3',
        '4','5','6','7','8','9','a','b','c','d','e','f'};
    if(str == NULL || ascii_str == NULL)
        return -1;
    while(*src != '\0'){
        unsigned char tmp = *src;
        *des++ = ch_x[tmp>>4];
        *des++ = ch_x[tmp&0x0f];
        src++;
    }
    *des = '\0';
    return 0;
}
void net_utils_ipstr2byte(unsigned char * const bytes,const char *ip)
{
	unsigned int *ip_addr = NULL;

	if(bytes == NULL) return;
	ip_addr = (unsigned int *)bytes;
   	*ip_addr = ntohl(inet_addr(ip));
}


void net_utils_ipbyte2str(unsigned char * const ip_num, char *ip)
{
	unsigned int *ip_addr = NULL;
	struct in_addr in;

	if(ip_num == NULL) return;
	ip_addr = (unsigned int *)ip_num;
	in.s_addr = htonl(*ip_addr);
    strcpy(ip, (char*)inet_ntoa(in));
}

int net_utils_macstrtobyte(unsigned char* mac, const char* mac_str, int mac_len)
{
    int i;
    char c, val;
    for (i = 0; i < mac_len; i++) {
        if (!(c = tolower(*mac_str++))) {
            printf("Invalid hardware address!\n");
            return -1;
        }
        if ('0' <= c && c <= '9') {
            val = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            val = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            val = c - 'A' + 10;
        } else {
            printf("Invalid hardware address!\n");
            return -1;
        }
        *mac = val << 4;
        if (!(c = tolower(*mac_str++))) {
            printf("Invalid hardware address!\n");
            return -1;
        }
        if ('0' <= c && c <= '9') {
            val = c - '0';
        } else if (c >= 'a' && c <= 'f') {
            val = c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
            val = c - 'A' + 10;
        } else {
            printf("Invalid hardware address!\n");
            return -1;
        }
        *mac++ |= val;
        if (*mac_str == ':') {
            mac_str++;
        } else {
            if (i < 5) {
                printf("Invalid hardware address 5 , %c!\n", *mac_str);
                return -1;
            }
        }
    }
    if (i < 6) {
        printf("Invalid hardware address 6!\n");
        return -1;
    }

    return 0;
}

int net_utils_get_ifaddr(char *eth, char * const ip, char * const mac)
{
    int sock;
    //
    struct sockaddr_in sin;
    struct ifreq ifr;

    if(ip) 			ip[0] = 0;
    if(mac) 		mac[0]= 0;

    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) {
        perror("socket");
        return -1;
    }

    strncpy(ifr.ifr_name, eth, IFNAMSIZ);
    ifr.ifr_name[IFNAMSIZ - 1] = 0;

    if(ip) {
        if (ioctl(sock, SIOCGIFADDR, &ifr) < 0) {
            perror("ioctl");
            close(sock);
            return -1;
        }
        memcpy(&sin, &ifr.ifr_addr, sizeof(sin));
        strcpy(ip, inet_ntoa(sin.sin_addr));
    }

    if(mac) {
        if (ioctl(sock, SIOCGIFHWADDR, &ifr) < 0) {
            perror("ioctl");
            close(sock);
            return -1;
        }
        sprintf(mac, "%02X:%02X:%02X:%02X:%02X:%02X", ifr.ifr_hwaddr.sa_data[0] & 0xff,ifr.ifr_hwaddr.sa_data[1] & 0xff,ifr.ifr_hwaddr.sa_data[2] & 0xff,
                ifr.ifr_hwaddr.sa_data[3] & 0xff,ifr.ifr_hwaddr.sa_data[4] & 0xff,ifr.ifr_hwaddr.sa_data[5] & 0xff);
    }

    close(sock);
    return 0;
}

