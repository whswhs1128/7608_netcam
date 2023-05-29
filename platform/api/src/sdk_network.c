/*!
*****************************************************************************
** \file      $gkprjgk7101_net.c
**
** \version	$id: gk7101_net.c 15-08-13  8月:08:1439457543
**
** \brief
**
** \attention   this code is provided as is. goke microelectronics
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#include <netinet/in.h>
#include <arpa/nameser.h>
#include <resolv.h>
#include <netdb.h>
#include "sys/wait.h"

#include "sdk_network.h"
#include "sdk_debug.h"
#include "ctype.h"

static int _get_ethernet_linked(const char *ether_name)
{
	int skfd = 0;
	struct ifreq ifr;
	int ret = -1;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd < 0)
	{
		return -1;
	}

	strcpy(ifr.ifr_name, ether_name);

	if(ioctl(skfd, SIOCGIFFLAGS, &ifr) <0 )
	{
		close(skfd);
		return -1;
	}

	close(skfd);
	if(ifr.ifr_flags & IFF_RUNNING)
	{
		ret = 1;
	}
    return ret;
}


static int _get_dhcp_pid_name(const char *adapter_name)
{
	int pid = -1;
	char cmd[128];
	char tmpname[] = "/tmp/dhcp_0.txt";
	sprintf(cmd,"ps -e | grep \"udhcpc -i %s \" > /tmp/dhcp_0.txt",adapter_name);
	new_system_call(cmd);
	if(access(tmpname,F_OK) == 0)
	{
	   	char line_buf[500];
		char grep_name[16] = "grep";
		char *pret;
		FILE *fp;



	    fp = fopen(tmpname, "r");
	    if (NULL == fp)
	    {
	        LOG_ERR("fopen %s ERROR!\n", tmpname);
	        return -1;
	    }


	    while(1)
	    {
	        memset(line_buf, 0, sizeof(line_buf));

	        /* read flash_map.ini  file */
	        pret = fgets(line_buf, sizeof(line_buf) - 1, fp);
			if(NULL == pret)
			{
				break;
			}

			if (NULL != strstr(line_buf, adapter_name) &&
				NULL ==  strstr(line_buf, grep_name))
			{
				sscanf(line_buf, "%d", &pid);
				printf("find %s udhcp pid:%d\n",grep_name,pid);
				break;
			}

	    }
		fclose(fp);
		unlink(tmpname);
	}

	return pid;

}

static void _kill_dhcp_process(const char *adapter_name)
{
	int pid = _get_dhcp_pid_name(adapter_name);
	char cmd[64];
	if(pid > 0)
	{
		sprintf(cmd,"kill -9 %d ",pid);
		new_system_call(cmd);
	}

}

int sdk_net_set_dhcp(const char *adapter_name, int enable, char *host_name)
{
    char buf[64] = {0};
    char hostName[128] = {0};

    if (host_name == NULL)
    {
        strcpy(hostName, "gokeipc");
    }
    else
    {
        strcpy(hostName, host_name);
    }
	_kill_dhcp_process(adapter_name);
    if(enable == 1)
    {
        if(strcmp(adapter_name,"eth0") == 0 )
        {
            if( _get_ethernet_linked(adapter_name) == 1)
            {
                //snprintf(buf,128,"udhcpc -i %s -b -x hostname:%s",adapter_name, hostName);
		new_system_call("killall -9 udhcpc");
		sprintf(buf,"%s","udhcpc &");
		printf("=============network_buf = %s=================\n", buf);
                new_system_call(buf) ;
		//		sdk_network_tools_ctrl(NETWOR_TOOLS_CTRL_FILE, NETWORK_TOOLS_CTRL_LAN_DHCP);
                LOG_INFO("DHCP enable:%s",adapter_name);
            }
            else
            {
                LOG_INFO("DHCP enable:%s, but not inserted\n",adapter_name);
            }
        }
        else
        {
            //snprintf(buf,128,"udhcpc -i %s -b -x hostname:%s",adapter_name, hostName);
	    new_system_call("killall -9 udhcpc");
            sprintf(buf,"%s","udhcpc &");
	    printf("=========udhcpc enable===========0\n");
	    printf("=============network_buf = %s=================\n",buf);

            new_system_call(buf) ;
			//sdk_network_tools_ctrl(NETWOR_TOOLS_CTRL_FILE, NETWORK_TOOLS_CTRL_WIFI_DHCP);
            LOG_INFO("DHCP enable:%s",adapter_name);


        }
    }
    else
    {
        LOG_INFO("DHCP disable:%s",adapter_name );
    }

    return 0;
}

int sdk_net_is_ipaddr(const char *ip_addr, IPADDR_TYPE iptype)
{
    unsigned int ip[9]={0};
    int ret;
    const char *p = ip_addr;
    int integer = 0, point_num;

    if(!ip_addr)
    {
        printf("%s, %d\n", __func__, __LINE__);
        return -1;
    }
    switch(iptype)
    {
        case IPV4:
            ret = sscanf(ip_addr, "%3d.%3d.%3d.%3d", &ip[0], &ip[1], &ip[2],&ip[3]);
            if(ret != 4) return -1;
            point_num = 0;
            while(*p != '\0')
            {
                if((*p >= '0') && (*p <= '9'))
                {
                    integer = integer * 10 + *p - '0';
                }
                else
                {
                    if(integer > 255)
                    {
                        printf("%s, %d error!integer:%d\n", __func__, __LINE__, integer);
                        printf("ip0:%d, ip1:%d, ip2:%d, ip3:%d\n", ip[0], ip[1], ip[2], ip[3]);
                        return -1;
                    }
                    integer = 0;
                    point_num ++;
                }
                p ++;
            }
            if(point_num > 3)
            {
                printf("%s, %d error!point_num:%d\n", __func__, __LINE__, point_num);
                printf("ip0:%d, ip1:%d, ip2:%d, ip3:%d\n", ip[0], ip[1], ip[2], ip[3]);
                return -1;
            }
            break;
        case IPV6:
            return -1;
            break;
        default:
            return -1;
            break;
    }
    return 0;
}
int sdk_net_get_ntp(char *server, int *count, int *is_dhcp)
{
    FILE *fp  = NULL;
    int r=0;
    char buf[100]={0};
    char tmp[100]={0};
    int dhcp;
    char *head = buf;
    int ntp_count=0;

    fp = fopen(_PATH_NTP_CONT, "r");
    if (fp == NULL)
        return -1;

    if(!server || !is_dhcp)
    {
        LOG_ERR("%s, %d\n", __func__, __LINE__);
        return -1;
    }
    while((r = fscanf(fp, "%s", buf)) == 1)
    {
        if(buf[0]=='#')
        {
            if (fscanf(fp, "%*[^\n]\n") < 0) { /* Skip the first line. */
                goto error;                /* Empty or missing line, or read error. */
            }
            continue;
        }
        if(0 == strncmp(buf, "NTP_IS_DHCP", sizeof ("NTP_IS_DHCP")-1))
        {
            r = fscanf(fp, "%*[^0-1]%i", &dhcp);/* skip space */
            *is_dhcp = dhcp;
        }
        else if(0 == strncmp(buf, "NTPSERVERS", sizeof ("NTPSERVERS")-1))
        {
            r = sscanf(buf, "%*[^\"]%*[\"]%s", tmp);/* skip "" */
            tmp[strlen(tmp)-1] = '\0';
            head = tmp;
            if (strlen(tmp)!=0)
            ntp_count=1;
            while((head = strchr(head, ' ')))
            {
                ntp_count++;
            }
            strcpy(server, tmp);
            *count = ntp_count;
        }
    }

    fclose(fp);
    return 0;

error:
    return -1;
}
int sdk_net_set_ntp(char *server, int count, int is_dhcp)
{
    FILE *fp = NULL;
    char brief[]={"# Dynamic ntp.conf file for onvif generated by onvif_server\n#     DO NOT EDIT THIS FILE BY HAND -- YOUR CHANGES WILL BE OVERWRITTEN\n"};
    char ntp_buf[100]={0};
    (void)count;


    if(!server)
    {
        LOG_ERR("%s, %d\n", __func__, __LINE__);
        return -1;
    }
    fp = fopen(_PATH_NTP_CONT, "w");
    if (fp == NULL)
        return -1;

    fprintf(fp, "%s", brief);
    if (-1 == is_dhcp)
        strcpy(ntp_buf,"NTP_IS_DHCP = 0\n");
    else
        strcpy(ntp_buf,"NTP_IS_DHCP = 1\n");

    strcat(ntp_buf, "NTPSERVERS=\"");
    strcat(ntp_buf, server);
    strcat(ntp_buf, "\"");

    fprintf(fp, "%s", ntp_buf);

    fclose(fp);
    return 0;
}

int sdk_net_set_dns(char *dns1, char *dns2)
{
    FILE *fp = NULL;

    if(!dns1 || !dns2)
    {
        LOG_ERR("%s, %d\n", __func__, __LINE__);
        return -1;
    }
    if(sdk_net_is_ipaddr(dns1, IPV4) < -1)
        return -1;
    if(sdk_net_is_ipaddr(dns2, IPV4) < -1)
        return -1;
    fp= fopen(_PATH_DNS_CONT, "w+");
    if (fp == NULL)
        return -1;


    if(dns1[0] != 0)
        fprintf(fp, "nameserver %s\n", dns1);
    if(dns2[0] != 0)
        fprintf(fp, "nameserver %s\n", dns2);
    fclose(fp);

    return 0;
}


int sdk_net_get_dns(char *dns1, char *dns2)
{
	FILE *fp;

    char line_buf[1000];
	int i = 0;
	char name[16] = "nameserver";
	char *pret;



    fp = fopen(_PATH_DNS_CONT, "r");
    if (NULL == fp)
    {
        LOG_ERR("fopen %s ERROR!\n", _PATH_DNS_CONT);
        return -1;
    }


    while(1)
    {
        memset(line_buf, 0, sizeof(line_buf));

        /* read flash_map.ini  file */
        pret = fgets(line_buf, sizeof(line_buf) - 1, fp);
		if(NULL == pret)
		{
			fclose(fp);
			return 0;
		}

		if (NULL != (pret = strstr(line_buf, name)))
		{
			if(i == 0)
			{
				sscanf(line_buf, "%*[^ ] %s", dns1);
			}
			else if (i == 1)
			{
				sscanf(line_buf, "%*[^ ] %s", dns2);
			}
			else if(i > 1)
			{
				break;
			}
			i++;
		}

    }
	fclose(fp);

    return 0;
}


int sdk_net_set_mtu(const char* adapter_name, const int mtu)
{
	struct ifreq ifr;
	const char *ifname = adapter_name;
	int skfd;

    if(!adapter_name)
    {
        LOG_ERR("%s, %d\n", __func__, __LINE__);
        return -1;
    }
    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(skfd < 0)
    {
        LOG_ERR("socket create error\n");
        return -1;
    }
    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, ifname);

    ifr.ifr_mtu = mtu;
	if (ioctl(skfd, SIOCSIFMTU, &ifr) < 0)
    {
        LOG_ERR("error!!:%s,device:%s\n",strerror(errno),adapter_name);
        close(skfd);
        return -1;
    }

	close(skfd);
	return 0;

}

int sdk_net_set_hwaddr(const char *adapter_name,  char *addr)
{
    struct ifreq ifr;
    const char *ifname = adapter_name;
    int skfd;
    int i = 0;
    char *paddr = addr, hwaddr[6];
    char str[4];
    int number;

    if(!addr || !adapter_name)
    {
        LOG_ERR("sdk_net_set_hwaddr fail! addr is NULL\n");
        return -1;
    }
    memset(hwaddr, 0, sizeof(hwaddr));
    i = 0;
    while(*paddr && (i < sizeof(hwaddr)))
    {
        memset(str,0,sizeof(str));
        if(':' != *paddr)
        {
            str[0] = toupper(*paddr);
            if(str[0]>='A' && str[0] <='Z')
            {
                number = 0xf0&((str[0]-'A'+10)<<4);
            }
            else if(str[0]>='0' && str[0] <='9')
            {
                number = 0xf0&((str[0]-'0')<<4);
            }
            else
            {
                LOG_ERR("hwaddr content error,device:%s\n", ifname);
                return -1;
            }

            paddr ++;
            if(':' != *paddr)
            {
                str[0] = toupper(*paddr);
                paddr ++;

                if(str[0]>='A' && str[0] <='Z')
                {
                    number |= ((str[0]-'A'+10)&0x0f);
                }
                else if(str[0]>='0' && str[0] <='9')
                {
                    number |= ((str[0]-'0')&0x0f);
                }
                else
                {
                    LOG_ERR("hwaddr content error,device:%s\n", ifname);
                    return -1;
                }
            }


            hwaddr[i] = number;
            i ++;
        }
        else
            paddr ++;
    }
    if(i != 6)
    {
        LOG_ERR("hwaddr content error,device:%s\n", ifname);
        return -1;
    }
    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(skfd < 0)
    {
        LOG_ERR("socket create error\n");
        return -1;
    }
    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name)-1);

    ioctl(skfd, SIOCGIFHWADDR, &ifr);

    memcpy((void *)ifr.ifr_hwaddr.sa_data,hwaddr, 6);
    if(ioctl(skfd, SIOCSIFHWADDR, &ifr) < 0)
    {
        LOG_ERR("error!:%s, NIC:%s\n",strerror(errno),adapter_name);
        close(skfd);
        return -1;
    }

	close(skfd);
	return 0;

}
int sdk_net_set_netmask(const char *adapter_name, char *netmask)
{
	struct ifreq ifr;
    struct sockaddr_in *sin;
	const char *ifname = adapter_name;
	int skfd;

    if(!adapter_name || !netmask)
    {
        LOG_ERR("%s, %d\n", __func__, __LINE__);
        return -1;
    }
    if(sdk_net_is_ipaddr(netmask, IPV4) < -1)
        return -1;
    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(skfd < 0)
    {
        LOG_ERR("socket create error\n");
        return -1;
    }
    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, ifname);
	if(ioctl(skfd, SIOCGIFNETMASK, &ifr) < 0)
	{
        LOG_ERR("error!!:%s,device:%s\n",strerror(errno),adapter_name);
        close(skfd);
        return -1;
    }

    sin = (struct sockaddr_in*) &ifr.ifr_addr;
    sin->sin_addr.s_addr = inet_addr(netmask);
    sin->sin_family = AF_INET;
    if (ioctl(skfd, SIOCSIFNETMASK, &ifr) < 0)
    {
        LOG_ERR("error!!:%s,device:%s\n",strerror(errno),adapter_name);
        close(skfd);
        return -1;
    }


	close(skfd);

	return 0;

}
int sdk_net_set_ip(const char *adapter_name, const char *addr)
{
	struct ifreq ifr;
    struct sockaddr_in *sin;
	const char *ifname = adapter_name;
	int skfd;
    int ret=0;

    if(!adapter_name || !addr)
    {
        LOG_ERR("%s, %d\n", __func__, __LINE__);
        return -1;
    }
    if(sdk_net_is_ipaddr(addr, IPV4) < -1)
        return -1;
    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(skfd < 0)
    {
        LOG_ERR("socket create error\n");
        return -1;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    strncpy(ifr.ifr_name, ifname, sizeof(ifr.ifr_name)-1);
    sin = (struct sockaddr_in*) &ifr.ifr_addr;
    sin->sin_addr.s_addr = inet_addr(addr);
    sin->sin_family = AF_INET;

	if ((ret = ioctl(skfd, SIOCSIFADDR, &ifr)) != 0)
	{
        LOG_ERR("error!!:%s,device:%s\n",strerror(errno),adapter_name);
        close(skfd);
        return  -1;
    }

	close(skfd);

	return 0;

}
int sdk_net_set_gateway(const char *adapter_name, const char *gateway)
{
    char cmd[MAX_CMD_LENGTH];

    if(!adapter_name || !gateway)
    {
        LOG_ERR("param error %s, %d\n", __func__, __LINE__);
        return -1;
    }
    if(sdk_net_is_ipaddr(gateway, IPV4) < -1)
    {
        LOG_ERR("ip error %s, %d\n", __func__, __LINE__);
        return -1;
    }

    sprintf(cmd, "route del default gw 0.0.0.0 dev %s", adapter_name);
    new_system_call(cmd);

    memset(cmd, 0, sizeof(cmd));
    sprintf(cmd, "route add default gw %s %s", gateway, adapter_name);
    new_system_call(cmd);
    return 0;
}

int  sdk_net_get_gateway(const char *adapter_name, char *gateway)
{
    FILE *fp;
    char buf[1024];
    char iface[16];
    unsigned char tmp[100]={'\0'};
    unsigned int dest_addr=0, gate_addr=0;
    if(NULL == gateway)
    {
        LOG_ERR("gateway is NULL \n");
        return -1;
    }
    fp = fopen(_PATH_PROCNET_ROUTE, "r");
    if(fp == NULL)
    {
        LOG_ERR("fopen route error \n");
        return -1;
    }

    fgets(buf, sizeof(buf), fp);
    while(fgets(buf, sizeof(buf), fp))
    {
        if((sscanf(buf, "%s\t%X\t%X", iface, &dest_addr, &gate_addr) == 3)
            && (memcmp(adapter_name, iface, strlen(adapter_name)) == 0)
            && gate_addr != 0)
        {
                memcpy(tmp, (unsigned char *)&gate_addr, 4);
                sprintf(gateway, "%d.%d.%d.%d", (unsigned char)*tmp, (unsigned char)*(tmp+1), (unsigned char)*(tmp+2), (unsigned char)*(tmp+3));
                break;
        }
    }

    fclose(fp);
    return 0;
}

int sdk_net_get_broadaddr(const char *adapter_name, char *broadaddr)
{
    struct ifreq ifr;
    const char *ifname = adapter_name;
    int skfd;
    struct sockaddr_in *broad;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(skfd < 0)
    {
        LOG_ERR("socket create error\n");
        return -1;
    }
    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_addr.sa_family = AF_INET;


    if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0){
        if (ioctl(skfd, SIOCGIFBRDADDR, &ifr) >= 0)
            broad = (struct sockaddr_in *)&ifr.ifr_broadaddr;
            strcpy(broadaddr,inet_ntoa(broad->sin_addr));
            printf("broad addr:%s\n",broadaddr);
    }
    else
    {
        close(skfd);
        LOG_ERR("error!!:%s,device:%s\n",strerror(errno),adapter_name);
        return -1;
    }

    close(skfd);
    return 0;

}
int sdk_net_set_broadaddr(const char *adapter_name,  char *broadaddr)
{
    struct ifreq ifr;
    const char *ifname = adapter_name;
    int skfd;
    struct sockaddr_in *mask;

    //if(sdk_net_is_ipaddr(broadaddr, IPV4) < -1)
    //    return -1;
    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(skfd < 0)
    {
        LOG_ERR("socket create error\n");
        return -1;
    }
    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, ifname);

    ioctl(skfd, SIOCGIFBRDADDR, &ifr);

    mask = (struct sockaddr_in *)&ifr.ifr_broadaddr;
    mask->sin_addr.s_addr = inet_addr(broadaddr);

    if (ioctl(skfd, SIOCSIFBRDADDR, &ifr) < 0)
    {
        LOG_ERR("error!!:%s,device:%s\n",strerror(errno),adapter_name);
        close(skfd);
        return -1;
    }


    close(skfd);

    return 0;

}
int sdk_net_get_netmask(const char *adapter_name,  char *netmask)
{
    struct ifreq ifr;
    const char *ifname = adapter_name;
    int skfd;
    struct sockaddr_in *mask;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(skfd < 0)
    {
        LOG_ERR("socket create error\n");
        return -1;
    }
    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, ifname);

    if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0){
        if (ioctl(skfd, SIOCGIFNETMASK, &ifr) >= 0)
            mask = (struct sockaddr_in *)&ifr.ifr_netmask;
            strcpy(netmask,inet_ntoa(mask->sin_addr));
            //printf("netmask:%s\n",netmask);
    }
    else
    {
        close(skfd);
        LOG_ERR("error!!:%s,device:%s\n",strerror(errno),adapter_name);
        return -1;
    }

    close(skfd);
    return 0;

}
int sdk_net_get_ip(const char *adapter_name,  char *addr)
{
    struct ifreq ifr;
    const char *ifname = adapter_name;
    int skfd;
    struct sockaddr_in *ipad;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(skfd < 0)
    {
        LOG_ERR("socket create error\n");
        return -1;
    }
    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_addr.sa_family = AF_INET;

    if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0)
    {
        ipad = (struct sockaddr_in *)&ifr.ifr_addr;
        strcpy(addr,inet_ntoa(ipad->sin_addr));
    }
    else
    {
        close(skfd);
        LOG_ERR("error!!:%s,device:%s\n",strerror(errno),adapter_name);
        return -1;
    }

    close(skfd);
    return 0;

}
int sdk_net_get_hwaddr(const char *adapter_name,  char *hwaddr)
{
    struct ifreq ifr;
    const char *ifname = adapter_name;
    int skfd;
    int i;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(skfd < 0)
    {
        LOG_ERR("socket create error\n");
        return -1;
    }
    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, ifname);

    if (ioctl(skfd, SIOCGIFHWADDR, &ifr) < 0)
    {
        close(skfd);
        LOG_ERR("get mac failed\n");
        LOG_ERR("error!!:%s,device:%s\n",strerror(errno),adapter_name);
        return -1;
    }

    for(i = 0; i < 6; i++)
    {
        sprintf(hwaddr+3*i,"%02x:",(unsigned char)ifr.ifr_hwaddr.sa_data[i]);
    }
    hwaddr[17] = 0;
    //printf("get mac :%s \n",hwaddr);

    close(skfd);

    return 0;

}
int sdk_net_get_mtu(const char *adapter_name, int *mtu)
{
    struct ifreq ifr;
    const char *ifname = adapter_name;
    int skfd;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(skfd < 0)
    {
        LOG_ERR("socket create error\n");
        return -1;
    }
    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, ifname);

    *mtu = 0;
    if (ioctl(skfd, SIOCGIFMTU, &ifr) >= 0)
        *mtu = ifr.ifr_mtu;
    else
    {
        close(skfd);
        LOG_ERR("error!!:%s,device:%s\n",strerror(errno),adapter_name);
        return -1;
    }
    close(skfd);
    return 0;
}

int sdk_net_get_name(char *name[], int *count_adapter)
{
    int numreqs = 30;
    struct ifconf ifc;
    struct ifreq *ifr;
    int n, err = -1;
    int skfd;

    *count_adapter = 0;
    ifc.ifc_buf = NULL;

    /* SIOCGIFCONF currently seems to only work properly on AF_INET sockets (as of 2.1.128) */
    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(skfd < 0)
    {
        LOG_ERR("error: no inet socket available\n");
        return -1;
    }
    for(;;)
    {
        ifc.ifc_len = sizeof(struct ifreq) * numreqs;
        ifc.ifc_buf = realloc(ifc.ifc_buf, ifc.ifc_len);

        if(ioctl(skfd, SIOCGIFCONF, &ifc) < 0)
        {
            goto out;
        }
        if(ifc.ifc_len == (int)(sizeof(struct ifreq) * numreqs))
        {
            /* assume it overflowed and try again */
            numreqs += 10;
            continue;
        }
        break;
    }

    ifr = ifc.ifc_req;
    for(n = 0; n < ifc.ifc_len; n += sizeof(struct ifreq))
    {
        name[*count_adapter] = (char *)malloc(LEN_NAME_ADAPTER * sizeof(char));
        strcpy(name[*count_adapter], ifr->ifr_name);
        (*count_adapter)++ ;
        ifr++;
    }
    err = 0;

out:
    close(skfd);
    free(ifc.ifc_buf);
    return err;
}

int sdk_net_up_down_network(const char *netinterface, int enable)
{
    char cfg[24] = {0};
    if(enable == 1)
    {
        sprintf(cfg,"ifconfig %s up",netinterface);
    }
    else
    {
        sprintf(cfg,"ifconfig %s down",netinterface);
    }
    printf("in sdk_net_up_down_network\n");
    new_system_call(cfg);
    return 0;
}
#if 0
static struct hostent *_gethostnameinfo (const char *host)
{
    struct hostent hostbuf, *hp;
    size_t hstbuflen;
    char tmphstbuf[1024];
    int res;
    int herr;

    hstbuflen = 1024;

    res = gethostbyname_r (host, &hostbuf, tmphstbuf, hstbuflen,
            &hp, &herr);
    /*  Check for errors.  */
    if (res || hp == NULL)
        return NULL;
    return hp;
}

static pthread_mutex_t gethost_lock = PTHREAD_MUTEX_INITIALIZER;
struct hostent* sdk_net_gethostbyname(char *hname)
{
	pthread_mutex_lock(&gethost_lock);
	//res_init();
	struct hostent *p_host;
	//p_host = gethostbyname2(hname, AF_INET);
	p_host = _gethostnameinfo(hname);
	//p_host = gethostbyname_proc2(hname);
	pthread_mutex_unlock(&gethost_lock);
	return p_host;
}
#endif
int new_system_call(const char *cmdline)
{
	pid_t pid;
	int ret;

	if(cmdline == NULL) {
		return 1;
	}

	if((pid = vfork()) < 0) {
		printf("create new proccess failed\n");
		ret = -1;
	} else if (pid == 0) {
		printf("  #%s\n", cmdline);
		execl("/bin/sh", "sh", "-c", cmdline, (char *)0);
		_exit(127);
	} else {
		printf("enter waitpid\n");
		while(waitpid(pid, &ret, 0) < 0){
			if (errno != EINTR) {
				ret = -1;
				break;
			}
		}
		printf("exit waitpid[%d]\n", ret);
	}
	return ret;
}


/*
 * char *name:  eth0 or ra0
 * return  0, connect
 * return -1, disconnect or disable
 */
int sdk_net_get_detect(char *name)
{
	int skfd = 0;
	struct ifreq ifr;
	int ret = -1;

	skfd = socket(AF_INET, SOCK_DGRAM, 0);
	if(skfd < 0)
	{
		return -1;
	}

	strcpy(ifr.ifr_name, name);

	if(ioctl(skfd, SIOCGIFFLAGS, &ifr) <0 )
	{
		close(skfd);
		return -1;
	}

	close(skfd);
	if(ifr.ifr_flags & IFF_RUNNING)
	{
		ret = 0;
	}

	return ret;
}



int sdk_network_tools_ctrl(char *cmd_file, char *cmd)
{

    int fd, ret, writeSize;

    fd = open(cmd_file, O_WRONLY);
    if(fd < 0)
    {
        printf("open %s failt\n", cmd_file);
        return -1;
    }
    writeSize = strlen(cmd);
    ret = write(fd, cmd, writeSize);
    if(ret != writeSize)
    {
        printf("write %s failt\n", cmd_file);
        perror("write error:\n");
        close(fd);
        return -1;
    }
    close(fd);

    return 0;
}

int sdk_net_resolve_host(struct in_addr *sin_addr, const char *hostname)
{

	if (!inet_aton(hostname, sin_addr)) {
		struct addrinfo *ai, *cur;
		struct addrinfo hints = { 0 };
		hints.ai_family = AF_INET;
		if (getaddrinfo(hostname, NULL, &hints, &ai))
			return -1;
		/* getaddrinfo returns a linked list of addrinfo structs.
		 * Even if we set ai_family = AF_INET above, make sure
		 * that the returned one actually is of the correct type. */
		for (cur = ai; cur; cur = cur->ai_next) {
			if (cur->ai_family == AF_INET) {
				*sin_addr = ((struct sockaddr_in *)cur->ai_addr)->sin_addr;
				freeaddrinfo(ai);
				return 0;
			}
		}
		freeaddrinfo(ai);
		return -1;
	}
	return 0;
}

