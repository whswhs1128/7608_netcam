/*!
*****************************************************************************
** \file        packages/onvif/src/gk7101/gk7101_net.c
**
**
** \brief       Onvif Network Control Interface
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include"gk7101_net.h"

//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************
#define LEN_NAME_ADAPTER           10
#define _PATH_PROCNET_DEV          "/proc/net/dev"
#define _PATH_PROCNET_IFINET6      "/proc/net/if_inet6"
#define _PATH_PROCNET_ROUTE        "/proc/net/route"
#define _PATH_DNS_CONT             "/etc/resolv.conf"
#define _PATH_NTP_CONT             "/etc/ntp.conf"


//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************


//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************
static GK_S32 get_mac(const GK_S8 *netinterface, GK_S8 *mac);
static GK_S32 get_name(GK_S8 *name[], GK_S32 *count_adapter);
static GK_S32 get_mtu(const GK_S8 *adapter_name, GK_S32 *mtu);
static GK_S32 set_mtu(const GK_S8* adapter_name, const GK_S32 mtu);
static GK_S32 get_hwaddr(const GK_S8 *adapter_name, GK_U8 hwaddr[]);
static GK_S32 set_hwaddr(const GK_S8 *adapter_name, const GK_U8 hwaddr[]);
static GK_S32 get_ip(const GK_S8 *adapter_name, struct sockaddr *addr);
static GK_S32 set_ip(const GK_S8 *adapter_name, const struct sockaddr *addr);
static GK_S32 set_dhcp(const GK_S8 *adapter_name);
static GK_S32 get_netmask(const GK_S8 *adapter_name, struct sockaddr *netmask);
static GK_S32 set_netmask(const GK_S8 *adapter_name, const struct sockaddr *netmask);
static GK_S32 get_broadaddr(const GK_S8 *adapter_name, struct sockaddr *broadaddr);
static GK_S32 set_broadaddr(const GK_S8 *adapter_name, struct sockaddr *broadaddr);
static GK_S32 get_ipv6(const GK_S8 *adapter_name, struct sockaddr_in6 *ipv6);
static GK_S32 get_gateway(const GK_S8 *adapter_name, struct sockaddr *gateway[], GK_S32 *count_gateway);
static GK_S32 set_gateway(const GK_S8 *adapter_name, struct sockaddr *gateway[], GK_S32 count_gateway);
static GK_S32 get_ntp(GK_S8 *server, GK_S32 *count, GK_BOOL *is_dhcp);
static GK_S32 set_ntp(GK_S8 *server, GK_S32 count, GK_BOOL is_dhcp);
static GK_S32 get_dns(GK_S8 ***search, GK_S8 ***nameserver, GK_S8 **domain, GK_S32 *size_search, GK_S32 *size_ns);
static GK_S32 set_dns(GK_CHAR search[MAX_SEARCHDOMAIN_NUM][MAX_SEARCHDOMAIN_LEN], GK_S8 **nameserver, GK_S8 *domain, GK_S32 size_search, GK_S32 size_ns);
static GK_S32 is_ipaddr(const GK_S8 *ip_addr, IPADDR_TYPE iptype);
static GK_S32 covprefixlen(struct sockaddr *paddr);



//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************

tNetworkAdapterConfiguration gNetworkAdapterConfiguration =
{
    .get_mac        = get_mac,
    .get_name       = get_name,
    .get_mtu        = get_mtu,
    .set_mtu        = set_mtu,
    .get_hwaddr     = get_hwaddr,
    .set_hwaddr     = set_hwaddr,
    .get_ip         = get_ip,
    .set_ip         = set_ip,
    .get_netmask    = get_netmask,
    .set_netmask    = set_netmask,
    .get_broadaddr  = get_broadaddr,
    .set_broadaddr  = set_broadaddr,
    .get_ipv6       = get_ipv6,
    .get_gateway    = get_gateway,
    .set_gateway    = set_gateway,
    .get_ntp        = get_ntp,
    .set_ntp        = set_ntp,
    .get_dns        = get_dns,
    .set_dns        = set_dns,
    .is_ipaddr      = is_ipaddr,
    .covprefixlen   = covprefixlen,
    .set_dhcp       = set_dhcp,
    .get_gateway6	= NULL,
	.get_ddns		= NULL,
	.set_ddns		= NULL,
	.get_hostname	= NULL,
	.set_hostname	= NULL,
	.is_running		= NULL,
};




//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************



//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************
static GK_S32 set_dhcp(const GK_S8 *adapter_name)
{
    FILE *fp = NULL;
    GK_S8 cmd[MAX_CMD_LENGTH];
    GK_S8 buffer[MAX_INFO_LENGTH];

    if (0 == sprintf(cmd, "udhcpc -i %s  2>&1", adapter_name))
    {
        ONVIF_ERR();
        goto err;
    }

    if (NULL == (fp = popen(cmd, "r")))
    {
        ONVIF_ERR();
        goto err;
    }
    if (NULL != fgets(buffer, sizeof(buffer), fp))
    {
        ONVIF_ERR("%s\n", buffer);
        goto err;
    }
    if (-1 == pclose(fp))
    {
        goto err;
    }

    return 0;
err:
    return -1;
}

static GK_S32 covprefixlen(struct sockaddr *paddr)
{
    GK_U32 nmask = 0;
    GK_S32 countbit=0;

    /*IPv4 */
    if (AF_INET == paddr->sa_family)
    {
        nmask = (*((struct sockaddr_in *)paddr)).sin_addr.s_addr;
        while (nmask)
        {
            nmask >>= 1;
            countbit++;
        }
    }

    return countbit;
}

static GK_S32 is_ipaddr(const GK_S8 *ip_addr, IPADDR_TYPE iptype)
{
    GK_U32 ip[9]={0};
    GK_S32 ret;

    if(IPV4 == iptype)
    {
        ret = sscanf(ip_addr, "%3d.%3d.%3d.%3d", &ip[0], &ip[1], &ip[2],&ip[3]);
        if(ret != 4)
        {
            return GK_FALSE;
        }
        if(0xFFFFFFFF == inet_addr(ip_addr))
        {
            return GK_FALSE;
        }
    }
    else if(IPV6 == iptype)
    {
        return GK_FALSE;
    }
    else
    {
        return GK_FALSE;
    }
    
    return GK_TRUE;
}
static GK_S32 get_ntp(GK_S8 *server, GK_S32 *count, GK_BOOL *is_dhcp)
{
    FILE *fp  = NULL;
    GK_S32 r=0;
    GK_S8 buf[100]={0};
    GK_S8 tmp[100]={0};
    GK_S32 dhcp;
    GK_S8 *head = buf;
    GK_S32 ntp_count=0;

    fp = fopen(_PATH_NTP_CONT, "r");
    if (fp == NULL)
        return -1;
        
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
static GK_S32 set_ntp(GK_S8 *server, GK_S32 count, GK_BOOL is_dhcp)
{
    FILE *fp = NULL;
    GK_S8 brief[]={"# Dynamic ntp.conf file for onvif generated by onvif_server\n#     DO NOT EDIT THIS FILE BY HAND -- YOUR CHANGES WILL BE OVERWRITTEN\n"};
    GK_S8 ntp_buf[100]={0};
    (void)count;
    fp = fopen(_PATH_NTP_CONT, "w");
    if (fp == NULL)
        return -1;
        
    fprintf(fp, "%s", brief);
    if (GK_FALSE == is_dhcp)
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

static GK_S32 set_dns(GK_CHAR search[MAX_SEARCHDOMAIN_NUM][MAX_SEARCHDOMAIN_LEN], GK_S8 **nameserver, GK_S8 *domain, GK_S32 size_search, GK_S32 size_ns)
{
    GK_S32 i=0;
    FILE *fp = NULL;
    GK_S8 buf[]={"# Dynamic resolv.conf file for onvif resolver generated by onvif_server\n#     DO NOT EDIT THIS FILE BY HAND -- YOUR CHANGES WILL BE OVERWRITTEN\n"};

    while (i<size_search)
    {
        ONVIF_DBG("search[%d]:[%s]\n", i, search[i]);
        i++;
    }
    i = 0;
    while (i<size_ns)
    {
        ONVIF_DBG("nameserver[%d]:[%s]\n", i, nameserver[i]);
        i++;
    }
    i=0;
    if (domain)
        ONVIF_DBG("domain[%s]\n", domain);

    fp= fopen(_PATH_DNS_CONT, "aw+");
    if (fp == NULL)
        return -1;

    fprintf(fp, "%s", buf);
    while (i<size_ns)
    {
        fprintf(fp, "nameserver %s\n", nameserver[i]);
        i++;
    }
    i=0;
    while (i<size_search)
    {
        fprintf(fp, "search %s\n", search[i]);
        i++;
    }
    fprintf(fp, "%s", "\n");
    if (domain)
    {
        fprintf(fp, "domain %s\n", domain);
    }
    fclose(fp);

    return 0;
}
static GK_S32 get_dns(GK_S8 ***search, GK_S8 ***nameserver, GK_S8 **domain, GK_S32 *size_search, GK_S32 *size_ns)
{
    GK_S8 buf[100]={0};
    GK_S32 r=0;
    GK_S8 ns[100]={0};  //Think about array bound if there are much date in resolv.conf. It may case error.
    GK_S8 sch[255]={0}; //Think about array bound if there are much date in resolv.conf. It may case error.
    GK_S8 dm[100]={0};  //Think about array bound if there are much date in resolv.conf. It may case error.
    GK_S32 count_ns=0;
    GK_BOOL havedm=GK_FALSE;
    GK_S32 count_sch=0;
    GK_S32 i=0;
    char *delimPos = NULL;

    FILE *fp = fopen(_PATH_DNS_CONT, "r");
    if (fp == NULL)
        return -1;
        
    while((r = fscanf(fp, "%s", buf)) == 1)
    {
        if(buf[0]=='#')
        {
            if (fscanf(fp, "%*[^\n]\n") < 0) { /* Skip the first line. */
                goto error;                /* Empty or missing line, or read error. */
            }
            continue;
        }

        if(0 == strncmp(buf, "nameserver", sizeof ("nameserver")))
        {
            r = fscanf(fp, "%s", buf);
            if (r != 1)
                break;
            count_ns++;
            if (count_ns > 1)
            {
                strcat(ns, " ");
            }
            strcat(ns, buf);
        }
        else if(0 == strncmp(buf, "search", sizeof ("search")))
        {
            r = fscanf(fp, "%*[ ]%[^\n]s", buf);/* skip space */
            if (r != 1)
                break;
            count_sch++;
            if (count_sch > 1)
            {
                strcat(sch, " ");
            }
            strcat(sch, buf);
        }
        else if(0 == strncmp(buf, "domain", sizeof ("domain")))
        {
            r = fscanf(fp, "%s", buf);
            if (r != 1)
                break;
            strcat(dm, buf);
            havedm = GK_TRUE;
        }
    }

    fclose(fp);

    if(count_sch)
    {
        *search = (char **)malloc(count_sch * sizeof(char *));
        memset(*search, 0, count_sch * sizeof(char *));
        delimPos = strtok(sch, " ");
        while(delimPos)
        {
            (*search)[i] = (char *)malloc(100 * sizeof(char));
            strcpy((*search)[i], delimPos);
            i++;
            delimPos = strtok(NULL, " \n");
        }
    }
    if(count_ns)
    {
        *nameserver = (char **)malloc(count_ns * sizeof(char *));
        memset(*nameserver, 0, sizeof(char *) * count_ns);
        i = 0;
        delimPos = strtok(ns, " ");
        while(delimPos)
        {
            (*nameserver)[i] = (char *)malloc(100 * sizeof(char));
            strcpy((*nameserver)[i], delimPos);
            i++;
            delimPos = strtok(NULL, " \n");
        }
    }
    if (havedm)
    {
        *domain = (char *)malloc(64 * sizeof(char));
        strcpy(*domain, dm);
    }
    *size_search = count_sch;
    *size_ns = count_ns;

    return 0;
error:
    return -1;
}

static GK_S32 set_mtu(const GK_S8* adapter_name, const GK_S32 mtu)
{
	struct ifreq ifr;
	const GK_S8 *ifname = adapter_name;
	GK_S32 skfd;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, ifname);

    ifr.ifr_mtu = mtu;
	if (ioctl(skfd, SIOCSIFMTU, &ifr) < 0)
        ONVIF_ERR("set mtu error!!\n");

	close(skfd);
	return 0;

}
static GK_S32 set_hwaddr(const GK_S8 *adapter_name, const GK_U8 hwaddr[])
{
    struct ifreq ifr;
    const GK_S8 *ifname = adapter_name;
    GK_S32 skfd;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, ifname);
    memcpy((void *) ifr.ifr_hwaddr.sa_data,hwaddr, 8);

	if (ioctl(skfd, SIOCSIFHWADDR, &ifr) < 0)
        ONVIF_ERR("set hwaddr error!!\n");

	close(skfd);
	return 0;

}
static GK_S32 set_netmask(const GK_S8 *adapter_name, const struct sockaddr *netmask)
{
	struct ifreq ifr;
	const GK_S8 *ifname = adapter_name;
	GK_S32 skfd;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_addr = *netmask;

//	if (ioctl(skfd, SIOCSIFADDR, &ifr) == 0){
        if (ioctl(skfd, SIOCSIFNETMASK, &ifr) < 0)
            ONVIF_ERR("set netmask error!!\n");
//    }

	close(skfd);

	return 0;

}
static GK_S32 set_ip(const GK_S8 *adapter_name, const struct sockaddr *addr)
{
	struct ifreq ifr;
	const GK_S8 *ifname = adapter_name;
	GK_S32 skfd;
    GK_S32 ret=0;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_addr = *addr;

	if ((ret = ioctl(skfd, SIOCSIFADDR, &ifr)) != 0){
        perror("?");
        ONVIF_ERR("set ip error!!\n");
    }

	close(skfd);

	return ret;

}

static GK_S32 set_gateway(const GK_S8 *adapter_name, struct sockaddr *gateway[], GK_S32 count_gateway)
{
    FILE *fp = NULL;
    GK_S8 cmd[MAX_CMD_LENGTH];
    GK_S8 buffer[MAX_INFO_LENGTH];
    GK_S32 i=0;
    for(i = 0; i< count_gateway; i++)
    {

        if(0 == sprintf(cmd, "route add default gw %s %s 2>&1", inet_ntoa((*(struct sockaddr_in *)gateway[i]).sin_addr), adapter_name))
        {
            ONVIF_ERR();
            goto err;
        }

        if(NULL == (fp = popen(cmd, "r")))
        {
            ONVIF_ERR();
            goto err;
        }
        
        if(NULL != fgets(buffer, sizeof(buffer), fp))
        {
            ONVIF_ERR("%s\n", buffer);
            goto err;
        }
        
        if (-1 == pclose(fp))
        {
            goto err;
        }
        
    }

    return 0;
err:
    return -1;
}
static GK_S32 get_gateway(const GK_S8 *adapter_name, struct sockaddr *gateway[], GK_S32 *count_gateway)
{
	GK_S8 devname[64] /*flags[16],*/ /* * sgw*//**sdest*/;
    GK_S8 *sdest = NULL;//[18];
	GK_ULONG d, g, m;
	GK_S32 flgs, ref, use, metric, mtu, win, ir;
	struct sockaddr_in s_addr;
    GK_S32 count = 0;

	FILE *fp = fopen(_PATH_PROCNET_ROUTE, "r");



	if (fscanf(fp, "%*[^\n]\n") < 0) { /* Skip the first line. */
		goto ERROR;                /* Empty or missing line, or read error. */
	}
	while(1)
	{
		int r;
		r = fscanf(fp, "%63s%lx%lx%X%d%d%d%lx%d%d%d\n",
				devname, &d, &g, &flgs, &ref, &use, &metric, &m,
				&mtu, &win, &ir);
		if(r != 11)
		{
			if((r < 0) && feof(fp)) 
			{ /* EOF with no (nonspace) chars read. */
				break;
			}
 			ERROR:
            	ONVIF_ERR("ERROR read route table!");
		}
		memset(&s_addr, 0, sizeof(struct sockaddr_in));
		s_addr.sin_family = AF_INET;
		s_addr.sin_addr.s_addr = d;

        sdest = strdup(inet_ntoa(s_addr.sin_addr));

        if (0 == strcmp(sdest, "0.0.0.0"))
        {
           s_addr.sin_addr.s_addr = g;
           gateway[count] = (struct sockaddr *)malloc(sizeof(struct sockaddr));
           memcpy(gateway[count], (void *)&s_addr, sizeof(struct sockaddr));
           count++;
        }
        free(sdest);
	}
    *count_gateway = count;
	fclose(fp);
    return 0;
}
//hwaddr
static GK_S32 get_ipv6(const GK_S8 *adapter_name, struct sockaddr_in6 *ipv6)
{
    FILE *f;
    GK_S8 addr6[40], devname[20];
    struct sockaddr_in6 sap;
    GK_S32 plen, scope, dad_status, if_idx;
    GK_S8 addr6p[8][5];

    memset(ipv6, 0, sizeof(struct sockaddr_in6));

    f = fopen(_PATH_PROCNET_IFINET6, "r");

    if (f == NULL)
        return -1;

    while (fscanf
            (f, "%4s%4s%4s%4s%4s%4s%4s%4s %08x %02x %02x %02x %20s\n",
             addr6p[0], addr6p[1], addr6p[2], addr6p[3], addr6p[4],
             addr6p[5], addr6p[6], addr6p[7], &if_idx, &plen, &scope,
             &dad_status, devname) != EOF
          ) {
        if (!strcmp(devname, adapter_name)) {
            sprintf(addr6, "%s:%s:%s:%s:%s:%s:%s:%s",
                    addr6p[0], addr6p[1], addr6p[2], addr6p[3],
                    addr6p[4], addr6p[5], addr6p[6], addr6p[7]);
            inet_pton(AF_INET6, addr6,
                    (struct sockaddr *) &sap.sin6_addr);
            sap.sin6_family = AF_INET6;
            *ipv6 = sap;
        }
    }
	fclose(f);
	return 0;

}

static GK_S32 get_broadaddr(const GK_S8 *adapter_name, struct sockaddr *broadaddr)
{
    struct ifreq ifr;
    const GK_S8 *ifname = adapter_name;
    GK_S32 skfd;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_addr.sa_family = AF_INET;
    memset(broadaddr, 0, sizeof(struct sockaddr));

    if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0){
        if (ioctl(skfd, SIOCGIFBRDADDR, &ifr) >= 0)
            *broadaddr = ifr.ifr_broadaddr;
    }

    close(skfd);
    return 0;

}
static GK_S32 set_broadaddr(const GK_S8 *adapter_name, struct sockaddr *broadaddr)
{
    struct ifreq ifr;
    const GK_S8 *ifname = adapter_name;
    GK_S32 skfd;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_addr.sa_family = AF_INET;
    ifr.ifr_broadaddr = *broadaddr;

    if (ioctl(skfd, SIOCSIFADDR, &ifr) == 0){
        if (ioctl(skfd, SIOCSIFBRDADDR, &ifr) < 0)
            ONVIF_ERR("set netmask error!!\n");
    }

    close(skfd);

    return 0;

}
static GK_S32 get_netmask(const GK_S8 *adapter_name, struct sockaddr *netmask)
{
    struct ifreq ifr;
    const GK_S8 *ifname = adapter_name;
    GK_S32 skfd;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_addr.sa_family = AF_INET;
    memset(netmask, 0, sizeof(struct sockaddr));

    if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0){
        if (ioctl(skfd, SIOCGIFNETMASK, &ifr) >= 0)
            *netmask = ifr.ifr_netmask;
    }

    close(skfd);
    return 0;

}
static GK_S32 get_ip(const GK_S8 *adapter_name, struct sockaddr *addr)
{
    struct ifreq ifr;
    const GK_S8 *ifname = adapter_name;
    GK_S32 skfd;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, ifname);
    ifr.ifr_addr.sa_family = AF_INET;
    memset(addr, 0, sizeof(struct sockaddr));

    if (ioctl(skfd, SIOCGIFADDR, &ifr) == 0)
    {
        *addr = ifr.ifr_addr;
    }

    close(skfd);
    return 0;

}
static GK_S32 get_hwaddr(const GK_S8 *adapter_name, GK_U8 hwaddr[])
{
    struct ifreq ifr;
    const GK_S8 *ifname = adapter_name;
    GK_S32 skfd;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, ifname);

    memset(hwaddr, 0, 6);
    if (ioctl(skfd, SIOCGIFHWADDR, &ifr) >= 0)
        memcpy(hwaddr, ifr.ifr_hwaddr.sa_data, 6);

    close(skfd);

    return 0;

}
static GK_S32 get_mtu(const GK_S8 *adapter_name, GK_S32 *mtu)
{
    struct ifreq ifr;
    const GK_S8 *ifname = adapter_name;
    GK_S32 skfd;

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    strcpy(ifr.ifr_name, ifname);

    *mtu = 0;
    if (ioctl(skfd, SIOCGIFMTU, &ifr) >= 0)
        *mtu = ifr.ifr_mtu;

    close(skfd);
    return 0;
}
static GK_S32 get_name(GK_S8 *name[], GK_S32 *count_adapter)
{
    GK_S32 numreqs = 30;
    struct ifconf ifc;
    struct ifreq *ifr;
    GK_S32 n, err = -1;
    GK_S32 skfd;

    *count_adapter = 0;
    ifc.ifc_buf = NULL;

    /* SIOCGIFCONF currently seems to only work properly on AF_INET sockets
       (as of 2.1.128) */
    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(skfd < 0) 
    {
        ONVIF_ERR("error: no inet socket available\n");
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

    return 0;
}

static GK_S32 get_mac(const GK_S8 *netinterface, GK_S8 *mac)
{
	struct ifreq ifreq;
	GK_S32 sockfd = 0;
	GK_CHAR aszmac[6] = {0};

	if((NULL == netinterface) || (NULL == mac))
	{
		ONVIF_ERR("get ip:  == NULL\r\n");
		return -1;
	}

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("netGetMac socket");
		return -1;
	}

	strcpy(ifreq.ifr_name, netinterface);

	if(ioctl(sockfd, SIOCGIFHWADDR, &ifreq) < 0)
	{
		perror("netGetMac ioctl");
		close(sockfd);
		return -2;
	}

	memcpy(aszmac, ifreq.ifr_hwaddr.sa_data, 6);
	if(mac != NULL)
	{
		memcpy(mac, aszmac, 6);
	}

	close(sockfd);

	return 0;
}

