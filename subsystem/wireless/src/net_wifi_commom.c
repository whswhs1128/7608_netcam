#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <signal.h>
#include <netinet/in.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>


#include "net_utils.h"
#include "net_wifi_commom.h"
#include "net_wifi_wpa.h"

//#define NO_LOCK
#define CMD_LEN_MIN  	64
#define CMD_LEN_MAX  	128
#define MAC_LEN			6
#define USB_VENDER_ID_REALTEK 	0x0bda
#define USB_VENDER_ID_RALINK 	0x148f
#define USB_VENDER_ID_LINUXHUB 	0x1d6b
#define USB_VENDER_ID_ABOCOM 	0x07B8
#define USB_VENDER_ID_SCI 	    0x2310

static char gwifi_handle[IFNAMSIZ] = {0}, * gpwifi_handle = NULL;
static unsigned char getDevName = 0;
static const char dev_name_arry[2][8] = {"ra", "wlan"};


NET_WIFI_MapVenderNameT MapVenderList[] =
{
	{USB_VENDER_ID_LINUXHUB, 	"Linux"},//linux foundation, Hub
	{USB_VENDER_ID_RALINK, 		"Ralink"},
	{USB_VENDER_ID_REALTEK, 	"Realtek"},
	{USB_VENDER_ID_ABOCOM, 		"Abocom"},
	{USB_VENDER_ID_SCI, 		"Sci"},

};

NET_WIFI_MapDeviceNameT MapDeviceList[] =
{
	{{USB_VENDER_ID_RALINK,  0x6370}, NET_WIFI_DEVICENAME_MT7601, NET_WIFI_DRIVERNAME_RT2870},/* Ralink 6370 */
	{{USB_VENDER_ID_RALINK,  0x7650}, NET_WIFI_DEVICENAME_MT7601, NET_WIFI_DRIVERNAME_RT2870},/* MT 6370 */
	{{USB_VENDER_ID_RALINK,  0x7601}, NET_WIFI_DEVICENAME_MT7601, NET_WIFI_DRIVERNAME_RT2870},/* MT 6370 */
	{{USB_VENDER_ID_REALTEK, 0x8179}, NET_WIFI_DEVICENAME_RTL8188, NET_WIFI_DRIVERNAME_RTL817X},/*8188EU*/
	{{USB_VENDER_ID_REALTEK, 0x0179}, NET_WIFI_DEVICENAME_RTL8188, NET_WIFI_DRIVERNAME_RTL817X},/*8188EU*/
	{{USB_VENDER_ID_REALTEK, 0x8177}, NET_WIFI_DEVICENAME_RTL8188, NET_WIFI_DRIVERNAME_RTL817X},
	{{USB_VENDER_ID_REALTEK, 0x817E}, NET_WIFI_DEVICENAME_RTL8188, NET_WIFI_DRIVERNAME_RTL817X},
	{{USB_VENDER_ID_REALTEK, 0xF179}, NET_WIFI_DEVICENAME_RTL8188FTV, NET_WIFI_DRIVERNAME_RTL817X},
	{{USB_VENDER_ID_ABOCOM,  0x8179}, NET_WIFI_DEVICENAME_RTL8188, NET_WIFI_DRIVERNAME_RTL817X},/*custom 8188EU*/
    {{USB_VENDER_ID_SCI,     0x9082}, NET_WIFI_DEVICENAME_SCI9083, NET_WIFI_DRIVERNAME_SCI9083}, /*sci 9083*/
	/*specific device id*/
    {{0x2955, 0x1001},  NET_WIFI_DEVICENAME_MT7601, NET_WIFI_DRIVERNAME_RT2870},/*custom*/
	{{0x0b05,  0x1791}, NET_WIFI_DEVICENAME_RTL8188, NET_WIFI_DRIVERNAME_RTL817X},
	{{0x13D3,  0x3311}, NET_WIFI_DEVICENAME_RTL8188, NET_WIFI_DRIVERNAME_RTL817X},
	{{0x13D3,  0x3311}, NET_WIFI_DEVICENAME_RTL8188, NET_WIFI_DRIVERNAME_RTL817X},

};

static int _is_wifi_dev2(const char *dev)
{
	int ret = -1;
	char cmd[CMD_LEN_MAX] = {0};
	if(isNull(dev))
		return 0;
	snprintf(cmd, sizeof(cmd),"%s -a|%s -e \"s/^\\w*/&<</\" -e \"s/<<.*//\"|%s %s",
			IFCONFIG_CMD, SED_CMD, GREP_CMD, dev);
	ret = new_system_call(cmd);
	if(ret == 0)
		return 1;
	else
		return 0;
}

/* get wifi device name, if no success this is not wifi device.*/
static int _is_wifi_dev(const char *dev)
{

	int ret;
	struct ifreq req;
	int s;

	if(dev == NULL){
		//perror("net wifi\n");
		return 0;
	}

	s = socket(AF_INET, SOCK_DGRAM, 0 );
	if (s < 0) {
		//perror("Cannot open socket");
		return 0;
	}
	strncpy(req.ifr_name, dev, sizeof(req.ifr_name));

/* solution net/if.h and linux/if.h conflict */
#ifndef SIOCGIWNAME
#define SIOCGIWNAME	0x8B01		/* get name == wireless protocol */
#endif
	ret = ioctl(s, SIOCGIWNAME, &req);
	if (ret < 0) {
		close(s);
		//fprintf(stderr, "No wireless extension\n");
		return 0;
	}
	close(s);
	return 1;
}

/* get all wifi dev name, list len max is DEF_MAX_LIST=10 */
/* If the search to multiple wireless devices, defualt use frist device. */
/* if need look all wireless devices, please use net_wifi_get_wifidevlist(). */
static int _wifi_get_wifidev(char *dev, int len, int index)
{
#define DEF_MAX_LIST 10

	int i=0, j=0, k = 0;
	int sockfd;
	int ret = 0;
    char buff[CMD_LEN_MAX] ={0};
	struct ifconf ifconf;
	unsigned char buf[sizeof (struct ifreq) * DEF_MAX_LIST];
	struct ifreq *ifreq;
	//init recv buff
	ifconf.ifc_len = sizeof (struct ifreq) * DEF_MAX_LIST;
	ifconf.ifc_buf = (char *)buf;

	if(dev == NULL || len <= 0){
		printf("bad params\n");
		return -1;
	}
	memset(buf, 0, sizeof (struct ifreq) * DEF_MAX_LIST);
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket open failure!\n" );
		return -1;
	}
	ret = ioctl(sockfd, SIOCGIFCONF, &ifconf); //get all dev name
	if(ret){
		perror("[SIOCGIFCONF]");
		close(sockfd);
		return -1;
	}
	close(sockfd);
	//loop seclect dev name
	ifreq = (struct ifreq*)buf;
	for (i = 0; i < DEF_MAX_LIST; i++)
	{
		if(ifreq->ifr_flags == AF_INET)
		{
			if(_is_wifi_dev(ifreq[i].ifr_name)){
				if(j == index){
					strncpy(dev, ifreq[i].ifr_name, len-1);
					//printf("find dev:%s\n", dev);
					return 1;
				} else {
					j++;
				}
			}
		}
	}

    /* if network device not enable, mannul detect device name */
    for(i=0, k=0; i < 2; i++)
    {
        for(j=0; j < 2; j++){
            memset(buff, 0, sizeof(buff));
			sprintf(buff, "%s%d",dev_name_arry[i], j);
            //sprintf(buff, "%s %s%d >> /dev/null", IFCONFIG_CMD, dev_name_arry[i], j);
            //if(system(buff) == 0){
            if(_is_wifi_dev2(buff)){
                snprintf(dev, len, "%s%d", dev_name_arry[i], j);
				//printf("find dev:%s\n", dev);
                return 1;
            } else {
				k++;
			}
        }
    }

	return 0;
}

int _wifi_cli_getshellresult(char *cmd, char *buffer, int buffsize)
{
	FILE *fp = NULL;

    if (NULL == (fp = popen(cmd, "r")))
    {
        goto err;
    }
    if (NULL == fgets(buffer, buffsize, fp))
    {
        goto err;
    }
    pclose(fp);
	NETWORK_PRINT_INFO(">>%s\n", buffer);
    return 0;
err:
	pclose(fp);
    return -1;
}
static int _read_line(int fd, char *buf, int size)
{
	int i;
	for(i = 0; i < size; i++) {
		if(read(fd, buf+i, 1) <= 0){
			return i;
		}
		if(buf[i] == '\n')
			break;
	}
	return (i+1);
}
static int _wifi_check_file_iscontainstr(const char* path, const char *str)
{
	int fd = -1;
	char buf[256] = {0};
	if(isNull(path) || isNull(str))
		return -1;
	fd = open(path, O_RDONLY);
	if(fd < 0){
		return -1;
	}
	while(_read_line(fd, buf, sizeof(buf)) > 0) {
		if(NULL != strstr(buf, str)){
			close(fd);
			return 1;
		}
		memset(buf, 0, sizeof(buf));
	}
	close(fd);
	return 0;
}


int net_wifi_getUsbDeviceID(NET_WIFI_DeviceNumT *device, int bus, int deviceNum)
{
	int ret = 0;
	char buff[32] = {0};
	char cmd[CMD_LEN_MAX] = {0};
	snprintf(cmd, sizeof(cmd),
		"%s |%s \"Bus %03x Device %03x\"|%s -e \"2d\" -e \"s/^.*ID //\"",
		LSUSB_CMD, GREP_CMD, bus, deviceNum, SED_CMD);

	ret = _wifi_cli_getshellresult(cmd, buff, sizeof(buff));
	if(ret != 0) {
		return ret;
	}
	sscanf(buff, "%04x:%04x", &(device->venderID), &(device->deviceID));
	return 0;
}

int net_wifi_getVenderNameByDeviceNum(NET_WIFI_DeviceNumT device, char **pName)
{
	int i;
	for(i = 0; i < (sizeof(MapVenderList)/sizeof(NET_WIFI_MapVenderNameT)); i++) {
		if(device.venderID == MapVenderList[i].venderID) {
			*pName = MapVenderList[i].venderName;
			return 0;
		}
	}
	return -1;
}

int net_wifi_getDeviceNameByDeviceNum(NET_WIFI_DeviceNumT device, char **pName)
{
	int i;
	for(i = 0; i < sizeof(MapDeviceList)/sizeof(NET_WIFI_MapDeviceNameT); i++) {
		if(device.venderID == MapDeviceList[i].deviceNum.venderID &&
			device.deviceID == MapDeviceList[i].deviceNum.deviceID) {
			*pName = MapDeviceList[i].deviceName;
			return 0;
		}
	}
	return -1;
}

int net_wifi_getDriverNameByDeviceNum(NET_WIFI_DeviceNumT device, char **pName)
{
	int i;
	for(i = 0; i < sizeof(MapDeviceList)/sizeof(NET_WIFI_MapDeviceNameT); i++) {
		if(device.venderID == MapDeviceList[i].deviceNum.venderID &&
			device.deviceID == MapDeviceList[i].deviceNum.deviceID) {
			*pName = MapDeviceList[i].driverName;
			return 0;
		}
	}
	return -1;
}


WIFI_HANDLE net_wifi_open(void)
{
	int ret = 0;
	if(getDevName == 0)
	{
		getDevName = 1;
		memset(gwifi_handle, 0, IFNAMSIZ);
		ret = _wifi_get_wifidev(gwifi_handle, IFNAMSIZ, 0);
		if(ret <= 0){
		    gpwifi_handle = NULL;
	        return NULL;//dev_name_arry[0];
		}
		gpwifi_handle = gwifi_handle;//save
	}
	return (WIFI_HANDLE)gpwifi_handle;
}

void net_wifi_close(WIFI_HANDLE dev)
{
	getDevName = 1;
}


int net_wifi_up(const WIFI_HANDLE dev)
{
	int ret = 0;
	char cmd[64];

	if(dev == NULL) return -1;
	snprintf(cmd, CMD_LEN_MIN, "%s %s up", IFCONFIG_CMD, dev);
	cmd[CMD_LEN_MIN - 1] = '\0';

	new_system_call(cmd, ret);
	return ret;
}
int net_wifi_up_ap(const WIFI_HANDLE dev)
{
	int ret = 0;
	char cmd[64];

	if(dev == NULL) return -1;
	snprintf(cmd, CMD_LEN_MIN, "%s %s %s up", IFCONFIG_CMD, dev,UDHCPD_IP);
	cmd[CMD_LEN_MIN - 1] = '\0';

	new_system_call(cmd, ret);
	return ret;
}

int net_wifi_down(const WIFI_HANDLE dev)
{
	int ret = 0;
	char cmd[64];

	if(dev == NULL) return -1;
	snprintf(cmd, CMD_LEN_MIN, "%s %s down", IFCONFIG_CMD, dev);
	cmd[CMD_LEN_MIN - 1] = '\0';

	new_system_call(cmd, ret);
	return ret;

}

int net_wifi_set_ip(const WIFI_HANDLE dev,  const char * ipaddr)
{
	struct ifreq ifr;
    struct sockaddr_in *sin;
	const char *ifname = dev;
	int skfd;
    int ret=0;

    if(!dev || !ipaddr)
    {
        NETWORK_PRINT_ERROR("%s, %d\n", __func__, __LINE__);
        return -1;
    }

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(skfd < 0)
    {
        NETWORK_PRINT_ERROR("socket create error\n");
        return -1;
    }

    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, ifname);
    sin = (struct sockaddr_in*) &ifr.ifr_addr;
    sin->sin_addr.s_addr = inet_addr(ipaddr);
    sin->sin_family = AF_INET;

	if ((ret = ioctl(skfd, SIOCSIFADDR, &ifr)) != 0){
        perror("SIOCSIFADDR ?");
        NETWORK_PRINT_ERROR("set [%s]ip[%s] error!!\n", ifname, ipaddr);
        close(skfd);
        return  -1;
    }

	close(skfd);

	return 0;
}

int net_wifi_get_ip(const WIFI_HANDLE dev,  char * const ipaddr)
{
	unsigned int localip = 0;
	struct sockaddr_in addr;

	if(dev == NULL || ipaddr == NULL){
		perror("net wifi");
		return -1;
	}
	//ret = get_ip(dev, (struct sockaddr *)&addr);
	//if(ret) return ret;

	localip = ntohl(addr.sin_addr.s_addr);
	net_ipbyte2str((unsigned char*)&localip, ipaddr);
	return 0;
}

int net_wifi_set_mask(const WIFI_HANDLE dev,  const char *  netmask)
{
	struct ifreq ifr;
    struct sockaddr_in *sin;
	const char *ifname = dev;
	int skfd;

    if(dev == NULL || netmask == NULL)
    {
        NETWORK_PRINT_ERROR("%s, %d\n", __func__, __LINE__);
        return -1;
    }

    skfd = socket(AF_INET, SOCK_DGRAM, 0);
    if(skfd < 0)
    {
        NETWORK_PRINT_ERROR("socket create error\n");
        return -1;
    }
    memset(&ifr, 0, sizeof(struct ifreq));
    strcpy(ifr.ifr_name, ifname);
    sin = (struct sockaddr_in*) &ifr.ifr_addr;
    sin->sin_addr.s_addr = inet_addr(netmask);
    sin->sin_family = AF_INET;
//	if (ioctl(skfd, SIOCSIFADDR, &ifr) == 0){
        if (ioctl(skfd, SIOCSIFNETMASK, &ifr) < 0)
        {
            NETWORK_PRINT_ERROR("set netmask error!!\n");
            close(skfd);
            return -1;
        }
//    }

	close(skfd);

	return 0;
}


int net_wifi_get_mask(const WIFI_HANDLE dev,  char * const netmask)
{
	int ret = 0;
	struct sockaddr_in addr;

	if(dev == NULL || netmask == NULL){
		perror("net wifi");
		return -1;
	}
	//ret = get_netmask(dev, (struct sockaddr *)&addr);
	if(ret) return ret;

	net_ipbyte2str((unsigned char*)&addr.sin_addr, netmask);
	return 0;
}

int net_wifi_get_mac(const WIFI_HANDLE dev,  char * const macstr)
{
	int ret = 0;
	unsigned char mac[MAC_LEN] = {0};

	if(dev == NULL || macstr == NULL){
		perror("net wifi\n");
		return -1;
	}
	//ret = get_hwaddr(dev, mac);
	if(ret) return -1;
	ret = net_bytetomacstr(macstr, mac, MAC_LEN);
	if(ret) return -1;
	return 0;
}

int net_wifi_set_mac(const WIFI_HANDLE dev, const char *macstr)
{
	int ret = 0;
	//unsigned char mac[MAC_LEN+1] = {0};

	if(dev == NULL || macstr == NULL){
		perror("net wifi\n");
		return -1;
	}
	//ret = net_macstrtobyte(mac, macstr, MAC_LEN);
	//if(ret) return -1;

	net_wifi_down(dev);

	//ret = set_hwaddr(dev, (unsigned char*)macstr);
	if(ret){
		perror("net wifi\n");
		return -1;
	}

	return 0;
}
int net_wifi_check_driver_exist(const char *driver_name)
{
	int ret = 0;
	if(isNull(driver_name))
		return 0;
	ret = _wifi_check_file_iscontainstr("/proc/modules", driver_name);
	NETWORK_PRINT_INFO("file:%s ret:%d\n", driver_name, ret);
	if(ret == 1)
		return 1;
	else
		return 0;
}

int net_wifi_load_driver(const char *driver_name)
{
	int ret = 0;
	char cmd[CMD_LEN_MAX] = {0};
	if(isNull(driver_name))
		return -1;
	int len = strlen(driver_name);

	if((len >= 3) && (strcmp(driver_name + len-3, ".ko") == 0)){
		snprintf(cmd, sizeof(cmd),"%s %s -name \"%s\"|%s '2d'|%s %s",
			FIND_CMD, DRIVER_MODULES, driver_name, SED_CMD, XARGS_CMD, INSMOD_CMD);
	} else {
		snprintf(cmd, sizeof(cmd),"%s %s -name \"%s.ko\"|%s '2d'|%s %s",
			FIND_CMD, DRIVER_MODULES, driver_name, SED_CMD, XARGS_CMD, INSMOD_CMD);
	}
	NETWORK_PRINT_INFO("clear memory, avoid allocate memory failed \n");
	/* use 'sync' command for many more, beacuse avoid synchronize cache is incomplete. */
	new_system_call("sync; sync; sync; echo 1 > /proc/sys/vm/drop_caches");
	NETWORK_PRINT_INFO("clean memory complete. \n");
    NETWORK_PRINT_INFO("cmd:%s\n", cmd);
	ret = new_system_call(cmd);
	return ret;
}

int net_wifi_unload_driver(const char *driver_name)
{
	int ret;
	char cmd[CMD_LEN_MAX] = {0};
	if(isNull(driver_name))
		return -1;
	snprintf(cmd, sizeof(cmd),"%s %s", RMMOD_CMD, driver_name);
	NETWORK_PRINT_INFO("cmd:%s\n", cmd);
	ret = new_system_call(cmd);
	return ret;
}

int wifi_find_seq(const WIFI_HANDLE dev, const char *ssid)
{
    int i, ret;
    char des_ssid[128];
    int max_seq = -1;
    if(isNull(dev) || isNull(ssid))
        return WIFI_FAIL;
    int len = strlen(ssid);

    len += 1;
    len <<= 1;
    char *ascii_ssid = malloc(len);
    if(ascii_ssid == NULL)
        return WIFI_FAIL;
    max_seq = wpa_get_max_seq(dev);
    memset(ascii_ssid, 0, len);
    net_chartoascii(ssid, ascii_ssid);
	//NETWORK_PRINT_INFO("ssid:%s \nascii ssid:%s\nmaxseq:%d\n", ssid, ascii_ssid, max_seq);
    for(i=0; i <= max_seq; i++)
    {
        ret = net_wifi_cli_get_network(dev, i,
            WIFI_SET_SSID, des_ssid, sizeof(des_ssid));
        if(ret < 0){
            free(ascii_ssid);
            return i;
        }
		//NETWORK_PRINT_INFO("des ssid:%s\n", des_ssid);
		if(strncmp(ssid, &des_ssid[1], (IFNAMSIZ-1)) == 0){
            free(ascii_ssid);
            return i;
        }
        if(strncmp(ascii_ssid, des_ssid, ((IFNAMSIZ<<1)-1)) == 0){
            free(ascii_ssid);
            return i;
        }
    }
    free(ascii_ssid);
    return WIFI_FAIL;
}



#ifdef WIFI_COMMOM_TEST
static void net_wifi_usage(void)
{
	printf("Wifi commom Usage:\n");
	printf("  ./wifi_commom up\n");
	printf("\t==>enable wifi device.\n");
	printf("  ./wifi_commom down\n");
	printf("\t==>disable wifi device.\n");
	printf("  ./wifi_commom setip [ip address]\n");
	printf("\t==>set IP address of wifi device.\n");
	printf("  ./wifi_commom getip\n");
	printf("\t==>look IP address of wifi device.\n");
	printf("  ./wifi_commom setmac [mac address]\n");
	printf("\t==>set MAC address of wifi device.\n");
	printf("  ./wifi_commom getmac\n");
	printf("\t==>look IP address of wifi device.\n");
}

int main(int argc, char *argv[])
{
	int ret = 0;

 	if(argc < 2)return -1;
	WIFI_HANDLE wifi_handle = net_wifi_open();

	if(wifi_handle == NULL){
		printf("wifi_handle get error\n");
		return 0;
	}
	printf("wireless device name:%s\n", wifi_handle);
	if(strcmp(argv[1], "up") == 0) {
		ret = net_wifi_up(wifi_handle);
		if(ret)printf("up dev fail\n");
	} else if (strcmp(argv[1], "down") == 0){
		ret = net_wifi_down(wifi_handle);
		if(ret)printf("down dev fail\n");
	} else if (strcmp(argv[1], "setip") == 0 && argc == 3){
		ret = net_wifi_set_ip(wifi_handle, argv[2]);
		if(ret)printf("set dev fail\n");
		printf("getip:%s\n", argv[2]);
	} else if (strcmp(argv[1], "getip") == 0){
		char ipaddr[32] = {0};
		net_wifi_get_ip(wifi_handle, ipaddr);
		printf("ip:%s\n", ipaddr);
	} else if (strcmp(argv[1], "setmac") == 0 && argc == 3){
		net_wifi_set_mac(wifi_handle, argv[2]);
		printf("setmac:%s\n", argv[2]);
	} else if (strcmp(argv[1], "getmac") == 0){
		char mac[32] = {0};
		net_wifi_get_mac(wifi_handle, mac);
		printf("getmac:%s\n", mac);
	} else if (strcmp(argv[1], "help") == 0 || strcmp(argv[1], "?") == 0) {
		net_wifi_usage();
	}
	net_wifi_close(wifi_handle);
	return 0;
}

#endif

