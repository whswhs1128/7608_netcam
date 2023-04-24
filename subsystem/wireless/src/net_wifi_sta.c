#include <sys/types.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>

#include <signal.h>
#include <string.h>
#include <sys/ioctl.h>
#include <linux/wireless.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#include "net_wifi_sta.h"
#include "net_utils.h"
#include "net_wifi_wpa.h"
#include "net_wifi_commom.h"
#include "network_wifi.h"
//#include "sdk_network.h"


char * menu="bssid / frequency / signal level / flags / ssid\n";


//#include "linux_network.h"
#define CMD_LEN_MIN  	64
#define CMD_LEN_MAX  	128
#define MAC_LEN			6
#define PASSWD_CFG 		"/var/run/passwd.conf"
#define MAC_EMPTY       "00:00:00:00:00:00"

#define UDHCPC_PID		"/var/run/udhcpc_wifi.pid"

#define WIFI_SCAN_ORDER_DBM  3

#ifdef NETWORK_ELIAN_SUPPORT
#ifndef SIOCIWFIRSTPRIV
#define	SIOCIWFIRSTPRIV		0x8BE0
#endif
#define SIOCIWELIAN   		(SIOCIWFIRSTPRIV + 0x17)
#endif


#define IW_EV_LCP_LEN	(sizeof(struct iw_event) - sizeof(union iwreq_data))
#define IW_EV_POINT_LEN	(IW_EV_LCP_LEN + sizeof(struct iw_point) - \
			 IW_EV_POINT_OFF)
#define IW_EV_POINT_OFF (((char *) &(((struct iw_point *) NULL)->length)) - \
				  (char *) NULL)


extern int net_wifi_cli_terminate(const WIFI_HANDLE dev);

static int _wifi_ioctl(const char *dev, unsigned int op_code, void *parm)
{
	int sockfd;
	struct iwreq *wreq = NULL;

	if(dev == NULL || parm == NULL){
		NETWORK_PRINT_ERROR("bad params\n");
		return WIFI_FAIL;
	}

	wreq = (struct iwreq *)parm;
	sprintf(wreq->ifr_ifrn.ifrn_name, "%s", dev);
	/*create a socket to get info from ioctl*/
	if((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		NETWORK_PRINT_ERROR("Cannot creat socket,error\n");
		return WIFI_FAIL;
	}
	else {
		//printf("Socket opened successfully /n");
	}

	if (ioctl(sockfd, op_code, wreq) == -1) {
		//NETWORK_PRINT_ERROR("IOCTL Failed,error");
		close(sockfd);
		return WIFI_FAIL;
	}
	else {
		//printf("IOCTL SIOCGIWESSID Successfull/n");
	}
	close(sockfd);
	return WIFI_OK;
}

static int _wpa_get_connect_status(struct wpa_ctrl *ctrl, char **pstatus, int *plen)
{
	char buf[2048];
	size_t len;
	int ret;
    char *substr = NULL;
    const char *cmd = "STATUS";
    const char *wpa_status = "wpa_state=";

	len = sizeof(buf) - 1;
    memset(buf, 0, sizeof(buf));
	ret = wpa_ctrl_request(ctrl, cmd, strlen(cmd), buf, &len, NULL);
	if (ret == -2) {
		NETWORK_PRINT_ERROR("'%s' command timed out.\n", cmd);
		return WIFI_TIMEOUT;
	} else if (ret < 0) {
		NETWORK_PRINT_ERROR("'%s' command failed.\n", cmd);
		return WIFI_FAIL;
	}

    substr = strstr(buf, wpa_status);
    if(substr != NULL){
        int len = 0;
        substr += strlen(wpa_status);
        while(substr[len] == '_' || (substr[len] <= 'Z' && substr[len] >= 'A') || (substr[len] <= '9' && substr[len] >= '0')){
            len++;
        }
        *pstatus = malloc(len+1);
        if(pstatus == NULL)
            return WIFI_FAIL;
        *plen = len+1;
        memset(*pstatus, 0, len+1);
        memcpy(*pstatus, substr, len);
        return 0;
    }
   return WIFI_FAIL;
}
static int _wpa_build_empty_cfgfile(const char *cfg_path)
{
    int fd = -1;
    char buff[128] = {0};
    if(isNull(cfg_path))
        return WIFI_FAIL;
    /* clean configuration file */
    if(0 == access(cfg_path, F_OK)){
		remove(cfg_path);
	}
    /* open configuration file */
	fd = open(cfg_path, O_RDWR | O_CREAT, 0666);
	if(fd < 0){
		NETWORK_PRINT_ERROR("wifi open");
		return WIFI_FAIL;
	}
#ifdef WPA_CTL_INTERFACE
        memset(buff, 0, sizeof(buff));
        sprintf(buff, "ctrl_interface=%s\n", WPA_CTL_INTERFACE);
        write(fd, buff, strlen(buff));
		sprintf(buff, "ctrl_interface_group=0\n"
					  "ap_scan=1\n");
        write(fd, buff, strlen(buff));
#endif
    close(fd);
    return WIFI_OK;
}

static int _wifi_cli_udpchc(const WIFI_HANDLE dev)
{
	int ret = 0;
	char cmd[CMD_LEN_MAX] = {0};
	if(isNull(dev))
		return -1;
	sprintf(cmd,"%s -b -i%s -R -p %s", UDHCPC_CMD, dev, UDHCPC_PID);
    ret = new_system_call(cmd);
    //ret = sdk_network_tools_ctrl(NETWOR_TOOLS_CTRL_FILE, NETWORK_TOOLS_CTRL_WIFI_DHCP);
	memset(cmd, 0, sizeof(cmd));
	sprintf(cmd,"%s -2 `%s %s`", KILL_CMD, CAT_CMD, UDHCPC_PID);
    ret = new_system_call(cmd);
	return ret;
}

static int _wifi_cli_linkmsg(const WIFI_HANDLE dev, int seconds)
{
	int ret = WIFI_OK;
	int count = 0;

	sleep(3);
	seconds *= 10;
	do
	{
		usleep(300000); //sleep 100 ms
    	ret = net_wifi_cli_status(dev);
		if(ret == WIFI_STATUS_CONNECTED)//could wpa_supplicant no start connect.
		{
			break;
		}
		count++;
	}while((ret == WIFI_STATUS_CONNECTING)
		&& count < seconds); //seconds

    if(ret != WIFI_STATUS_CONNECTED){
        char mac[MAC_LEN*3] = {0};

		if(ret == WIFI_STATUS_DISCONNECTED && count < seconds)
            return WIFI_LINK_ERROR;
        ret = net_wifi_cli_get_bssid(dev, mac, sizeof(mac));
        net_wifi_cli_disconnect(dev);
        if(ret < 0)
            return WIFI_TIMEOUT;
        else if(strcmp(mac, MAC_EMPTY) == 0)
            return WIFI_NO_SSID;
		else
			return WIFI_TIMEOUT;
    }
	return WIFI_OK;
}

static int _wifi_cli_linkwpsmsg(const WIFI_HANDLE dev, int seconds)
{
	int ret = WIFI_OK;
	int count = 0;
	seconds *= 10;
	do
	{
		usleep(100000); //sleep 100 ms
    	ret = net_wifi_cli_status(dev);
		if(ret == WIFI_STATUS_CONNECTED && count > 5)//could wpa_supplicant no start connect.
		{
			break;
		}
		count++;
	}while((ret != WIFI_STATUS_CONNECTED)
		&& count < seconds); //seconds

    if(ret != WIFI_STATUS_CONNECTED){
        net_wifi_cli_disconnect(dev);
		return WIFI_TIMEOUT;
    }
	return WIFI_OK;
}

static unsigned int _freq_to_channel(int freq)
{
    if(freq >= 2407){
        return (freq - 2407)/5;
    }
    return freq;
}

static int _wifi_cli_v19(const WIFI_HANDLE dev)
{
	int ret;
	struct iwreq wreq;
    char *buff[sizeof(struct iw_range)*2];
    struct iw_range *range_raw;

	if(dev == NULL){
		NETWORK_PRINT_ERROR("bad params\n");
		return WIFI_FAIL;
	}
	memset(&wreq, 0, sizeof(struct iwreq));
    wreq.u.data.pointer = buff;
    wreq.u.data.length= sizeof(buff);
    wreq.u.data.flags = 0;
    ret = _wifi_ioctl(dev, SIOCGIWRANGE, &wreq);
	/*get mode from ioctl*/
	if (ret) {
		NETWORK_PRINT_ERROR("IOCTL SIOCGIWAUTH Failed,error");
		return ret;
	}
    range_raw = (struct iw_range *)buff;
    if(range_raw->we_version_compiled < 19){
        return WIFI_FAIL;
	}
	return WIFI_OK;
}

static void _wifi_gen_ie(GEN_IE_DataT *gen_ie, int ie_len,
    unsigned char *buff, int buff_len)
{
    int offset = 0;
    int ie_index = 0;

    if(gen_ie == NULL || buff == NULL
        || ie_len <= 0 || buff_len <=0){
        NETWORK_PRINT_ERROR("Bad params\n");
        return;
    }
    for(ie_index=0; (ie_index < ie_len) && (gen_ie[ie_index].use_flag > 0); ie_index++);

    if(ie_index >= ie_len){
        NETWORK_PRINT_ERROR("Bad params\n");
        return;
    }

    for(;offset <= (buff_len - 2) && ie_len > ie_index;offset += buff[offset+1]+2){
        if(buff[offset] == 0xdd || buff[offset] == 0x30){
            unsigned char *ie_buff = buff + offset;
            int ie_len = ie_buff[1] + 2;
            int index = 2;
            unsigned char wpa1_oui[3] = {0x00, 0x50, 0xf2};
            unsigned char wpa2_oui[3] = {0x00, 0x0f, 0xac};
            unsigned char *wpa_oui = NULL;
            int i;
            int cnt;
            int par_index = 0;
            int auth_index = 0;

            if(ie_len > buff_len)
                ie_len = buff_len;
            if(ie_buff[0] == 0x30)
            {
                if(ie_len < 4){
                    //printf("[%d]data length not enough.\n", __LINE__);
                    continue;
                }
                wpa_oui = wpa2_oui;
                gen_ie[ie_index].ie = WIFI_IE_WPA2;
            }
            else if(ie_buff[0] == 0xdd)
            {
                wpa_oui = wpa1_oui;
                if((ie_len < 8)
                    ||(memcmp(&ie_buff[index], wpa_oui,3) != 0)
                    ||(ie_buff[index + 3] != 0x01))
                {
                    if(ie_len < 8)printf("[%d]data length not enough.\n", __LINE__);
                    continue;
                }
                index += 4;
                gen_ie[ie_index].ie = WIFI_IE_WPA1;
            }
            gen_ie[ie_index].version = ie_buff[index] | (ie_buff[index + 1] << 8);
            index += 2;

            /* get group chip */
            if(ie_len < (index + 4)){
                gen_ie[ie_index].group_chip = WIFI_CHIP_TKIP;
                gen_ie[ie_index].pair_chip[0] = WIFI_CHIP_TKIP;
                gen_ie[ie_index].use_flag = 1;
                ie_index++;
                //printf("[%d]data length not enough.\n", __LINE__);
                continue;
            }
            if(memcmp(&ie_buff[index], wpa_oui, 3) != 0){
                gen_ie[ie_index].group_chip = WIFI_CHIP_PROPRIETARY;
            } else {
                if(ie_buff[index+3] >= WIFI_CHIP_MAXNUM)
                    gen_ie[ie_index].group_chip = WIFI_CHIP_UNKNOW;
                else
                    gen_ie[ie_index].group_chip = ie_buff[index+3] + 1;

            }
            index += 4;

            /* get pair chip */
            if(ie_len < (index + 2)){
                gen_ie[ie_index].pair_chip[0] = WIFI_CHIP_TKIP;
                gen_ie[ie_index].use_flag = 1;
                ie_index++;
                //printf("[%d]data length not enough.\n", __LINE__);
                continue;
            }
            cnt = ie_buff[index] | (ie_buff[index+1] << 8);
            index += 2;
            if(ie_len  < (index + 4*cnt)){
                gen_ie[ie_index].use_flag = 1;
                ie_index++;
                //printf("[%d]data length not enough.\n", __LINE__);
                continue;
            }
            for(i=0; i < cnt; i++){
                if(par_index < sizeof(gen_ie[ie_index].pair_chip)){
                    if(memcmp(&ie_buff[index], wpa_oui,3) != 0)
                        gen_ie[ie_index].pair_chip[par_index] = WIFI_CHIP_PROPRIETARY;
                    else if(ie_buff[index+3] >= WIFI_CHIP_MAXNUM)
                        gen_ie[ie_index].pair_chip[par_index] = WIFI_CHIP_UNKNOW;
                    else
                        gen_ie[ie_index].pair_chip[par_index] = ie_buff[index+3] + 1;
                }
                index += 4;
                par_index++;
            }

            /* get auth */
            if(ie_len < (index + 2)){
                gen_ie[ie_index].use_flag = 1;
                ie_index++;
                //printf("[%d]data length not enough.\n", __LINE__);
                continue;
            }
            cnt = ie_buff[index] | (ie_buff[index+1] << 8);
            index += 2;
            if(ie_len  < (index + 4*cnt)){
                gen_ie[ie_index].use_flag = 1;
                ie_index++;
                //printf("[%d]data length not enough.\n", __LINE__);
                continue;
            }
            for(i=0; i < cnt; i++){
                if(auth_index < sizeof(gen_ie[ie_index].auth_suite)){
                    if(memcmp(&ie_buff[index], wpa_oui,3) != 0)
                        gen_ie[ie_index].auth_suite[auth_index] = WIFI_AUTH_PROPRIETARY;
                    else if(ie_buff[index+3] >= WIFI_AUTH_MAXNUM)
                        gen_ie[ie_index].auth_suite[auth_index] = WIFI_AUTH_UNKNOW;
                    else
                        gen_ie[ie_index].auth_suite[auth_index] = ie_buff[index+3] + 1;
                }
                index += 4;
                auth_index++;
            }
            gen_ie[ie_index].use_flag = 1;
            ie_index++;
       }
    }
}


static int _wifi_cli_scanlist_dbmorder(SCAN_DataT *list, int len)
{
	int i,j;
	int index = 0;
	SCAN_DataT tmp;
	int vild_len;

	if(list == NULL || len <= 0)
		return WIFI_FAIL;
	for(i=0; i<len; i++){
		if(list[i].used != 1)
			break;
	}
	vild_len = i;
	for(i=0; i < vild_len-1; i++){
		index = i;
		for(j=i+1; j < vild_len; j++){
			if((list[j].level - list[j].noise)
				>(list[index].level - list[index].noise)){
				index = j;
			}
		}
		if(index != i){
			tmp = list[i];
			list[i] = list[index];
			list[index] = tmp;
		}
	}

	return WIFI_OK;
}

int _net_wifi_cli_scanorder(SCAN_DataT *list, int len, int order)
{
	int ret = WIFI_OK;
	/* error check */
	if(list == NULL || len <= 0)
		return WIFI_FAIL;
	switch(order)
	{
		case WIFI_SCAN_ORDER_DBM:
			ret = _wifi_cli_scanlist_dbmorder(list, len);
			break;
		default:
			ret = WIFI_FAIL;
			break;
	}
	return ret;
}

int _net_wifi_cli_get_workmode(const WIFI_HANDLE dev, int * const mode)
{
	int ret;
	struct iwreq wreq;

	if(dev == NULL || mode == NULL){
		printf("bad params\n");
		return WIFI_FAIL;
	}
	memset(&wreq, 0, sizeof(struct iwreq));

	ret = _wifi_ioctl(dev, SIOCGIWMODE, &wreq);
	/*get mode from ioctl*/
	if (ret) {
		NETWORK_PRINT_ERROR("IOCTL SIOCGIWMODE Failed,error");
		return ret;
	} else {
		//printf("IOCTL SIOCGIWESSID Successfull/n");
	}
	*mode = wreq.u.mode;
	return WIFI_OK;
}


#ifdef NETWORK_ELIAN_SUPPORT
#define ELIAN_CHAR_MAX	128
static int _wifi_cli_smartlink_ioctl(const WIFI_HANDLE dev, char *cmd, int cmd_len)
{
	int ret = 0;
	struct iwreq wreq;
	if(isNull(dev) || isNull(cmd)) {
		NETWORK_PRINT_ERROR("Smart Link params error");
		return WIFI_FAIL;
	}
	memset(&wreq, 0, sizeof(struct iwreq));
	wreq.u.data.pointer = cmd;//if not write these codes , the program maybe wrong.
	wreq.u.data.length = cmd_len;
	ret = _wifi_ioctl(dev, SIOCIWELIAN, &wreq);
	/*get ESSID from ioctl*/
	if (ret) {
		NETWORK_PRINT_ERROR("IOCTL SIOCGIWESSID Failed,error");
		return ret;
	}
	return WIFI_OK;
}

#define ELIAN_VALUE_STRING	1
#define ELIAN_VALUE_U8		2
#define ELIAN_VALUE_U32		3
static void _wifi_cli_smartlink_strtovaule(char *str, char *label, int type, void *buf, int size)
{
	char *cmd_cur = NULL;
	if(isNull(str) || isNull(label) || isNull(buf) || size <= 0)
		return;

	cmd_cur = strstr(str, label);
    if(cmd_cur != NULL){
        int len = 0;
        cmd_cur += strlen(label);
       	if(type == ELIAN_VALUE_U8){
			*(unsigned char *)buf = 0;
			while(*cmd_cur <= '9' && *cmd_cur >= '0'){
				*(unsigned char *)buf *= 10;
				*(unsigned char *)buf += *cmd_cur - '0';
				cmd_cur++;
			}
			//printf("[ELIAN]%s%d\n",  label, *(unsigned char *)buf);
       	} else if(type == ELIAN_VALUE_U32){
			*(unsigned int *)buf = 0;
			while(*cmd_cur <= '9' && *cmd_cur >= '0'){
				*(unsigned int *)buf *= 10;
				*(unsigned int *)buf += *cmd_cur - '0';
				cmd_cur++;
			}
			//printf("[ELIAN]%s%d\n",  label, *(unsigned int *)buf);
       	} else if(type == ELIAN_VALUE_STRING) {
       		char *pbuf = (char *)buf;
       		while(len < size && *cmd_cur != ',' && *cmd_cur != ' '
				&& *cmd_cur != '\0' && *cmd_cur != '\n') {
				*pbuf++ = *cmd_cur++;
				len++;
			}
			//printf("[ELIAN]%s%s\n", label, (char *)buf);
       	}
    }
}


int net_wifi_cli_get_essid(const WIFI_HANDLE dev, char *const essid, int len)
{
	int ret;
	struct iwreq wreq;

	if(dev == NULL || essid == NULL || len == 0){
		NETWORK_PRINT_ERROR("bad params\n");
		return WIFI_FAIL;
	}
	memset(&wreq, 0, sizeof(struct iwreq));

	wreq.u.essid.pointer = essid;//if not write these codes , the program maybe wrong.
	wreq.u.essid.length = len;
	ret = _wifi_ioctl(dev, SIOCGIWESSID, &wreq);
	/*get ESSID from ioctl*/
	if (ret) {
		NETWORK_PRINT_ERROR("IOCTL SIOCGIWESSID Failed,error");
		return ret;
	}

	return WIFI_OK;
}

int net_wifi_cli_set_essid(const WIFI_HANDLE dev, char *essid, int len)
{
	int ret;
	struct iwreq wreq;

	if(dev == NULL || essid == NULL || len == 0){
		NETWORK_PRINT_ERROR("bad params\n");
		return WIFI_FAIL;
	}
	memset(&wreq, 0, sizeof(struct iwreq));

	wreq.u.essid.pointer = essid;//if not write these codes , the program maybe wrong.
	wreq.u.essid.length = len;
	ret = _wifi_ioctl(dev, SIOCSIWESSID, &wreq);
	/*get ESSID from ioctl*/
	if (ret) {
		NETWORK_PRINT_ERROR("IOCTL SIOCSIWESSID Failed,error");
		return ret;
	}

	return WIFI_OK;
}


int net_wifi_cli_get_transfreq(const WIFI_HANDLE dev, int * const freq_mhz)
{
	int ret;
	struct iwreq wreq;

	if(dev == NULL || freq_mhz == NULL){
		NETWORK_PRINT_ERROR("bad params\n");
		return WIFI_FAIL;
	}
	memset(&wreq, 0, sizeof(struct iwreq));
	ret = _wifi_ioctl(dev, SIOCGIWFREQ, &wreq);
	if (ret) {
		NETWORK_PRINT_ERROR("IOCTL SIOCGIWFREQ Failed,error");
		return ret;
	}
	/* output current freq and channel*/
	if(wreq.u.freq.e){
		int i = 6;
		i -= wreq.u.freq.e;
		if(i < 0){
			*freq_mhz = wreq.u.freq.m;
			for(i = 0; i < wreq.u.freq.e - 6; i++)
				*freq_mhz *= 10;
		} else {
			*freq_mhz = wreq.u.freq.m;
			for(; i > 0; i--)
				*freq_mhz /= 10;
		}
	} else {
		*freq_mhz = wreq.u.freq.m/1000000;
	}

	return WIFI_OK;
}


int net_wifi_cli_get_bssid(const WIFI_HANDLE dev, char * const mac, int len)
{
	int ret;
	struct iwreq wreq;

	if(dev == NULL || mac == NULL || len == 0){
		NETWORK_PRINT_ERROR("bad params\n");
		return WIFI_FAIL;
	}
	memset(&wreq, 0, sizeof(struct iwreq));
	ret = _wifi_ioctl(dev, SIOCGIWAP, &wreq);
	if (ret) {
		NETWORK_PRINT_ERROR("IOCTL SIOCGIWAP Failed,error");
		return ret;
	}
	/* output current connecting AP MAC*/
	unsigned char *APaddr = (unsigned char *) wreq.u.ap_addr.sa_data;
	if(len < 3 * MAC_LEN -1) return WIFI_FAIL;
	else net_bytetomacstr(mac, APaddr, MAC_LEN);

	return WIFI_OK;
}

int net_wifi_cli_set_bssid(const WIFI_HANDLE dev, char *mac, int len)
{
	int ret;
	struct iwreq wreq;

	if(dev == NULL || mac == NULL || len == 0){
		NETWORK_PRINT_ERROR("bad params\n");
		return WIFI_FAIL;
	}
	memset(&wreq, 0, sizeof(struct iwreq));
    net_macstrtobyte((unsigned char*)wreq.u.ap_addr.sa_data, mac, MAC_LEN);
	ret = _wifi_ioctl(dev, SIOCSIWAP, &wreq);
	if (ret) {
		NETWORK_PRINT_ERROR("IOCTL SIOCSIWAP Failed,error");
		return ret;
	}

	return WIFI_OK;
}


int net_wifi_cli_get_transprotocol(const WIFI_HANDLE dev, char *const protocol, int len)
{
	int ret;
	struct iwreq wreq;

	if(dev == NULL || protocol == NULL || len == 0){
		NETWORK_PRINT_ERROR("bad params\n");
		return WIFI_FAIL;
	}
	memset(&wreq, 0, sizeof(struct iwreq));

	ret = _wifi_ioctl(dev, SIOCGIWNAME, &wreq);
	/*get IEEE from ioctl*/
	if (ret) {
		NETWORK_PRINT_ERROR("IOCTL SIOCGIWNAME Failed,error");
		return ret;
	}

	strncpy(protocol, wreq.u.name, len);
	return WIFI_OK;
}

int net_wifi_cli_on(const WIFI_HANDLE dev, const char *ip, const char *mask)
{
	int ret = 0;
    char cmd[CMD_LEN_MAX] = {0};
	if (isNull(dev)) {
		NETWORK_PRINT_ERROR("bad params\n");
		return WIFI_FAIL;
	}
#ifdef AP_STA_SUPPORT
    net_wifi_down(dev);
        
    new_system_call("killall hostapd");
    
    new_system_call("killall udhcpd");
	/* enable network */
	net_wifi_up(dev);
#endif 
	/* enable network */
//	net_wifi_up(dev);

    /* check whether init */
    ret = net_wifi_cli_status(dev);
    if(ret != WIFI_STATUS_ERROR){
        NETWORK_PRINT_ERROR("Alreadly init\n");
        return WIFI_OK;
    }
	/*set wifi ip address */
    if(ip != NULL) {
    	ret = net_wifi_set_ip(dev, ip);
    	if(ret != 0){
    		NETWORK_PRINT_ERROR("net_wifi_set_ip fail");
    		return WIFI_FAIL;
    	}
    }
	/* set wifi device ip mask*/
	if(mask != NULL) {
		/* if mask no equal NULL, should set netmask */
		ret = net_wifi_set_mask(dev, mask);
		if(ret != 0){
			NETWORK_PRINT_ERROR("net_wifi_set_mask fail");
			return WIFI_FAIL;
		}
	}
    ret = _wpa_build_empty_cfgfile(PASSWD_CFG);
    if(ret < 0)
        return WIFI_FAIL;
	/* start wpa_suppliant */
    memset(cmd, 0, sizeof(cmd));
	snprintf(cmd, sizeof(cmd),"%s -Dwext -B -i%s -c%s ", WPA_SUPPLICANT_CMD, dev, PASSWD_CFG);
    printf("new_system_call:%s\n", cmd);
    ret = new_system_call(cmd);
	//ret = sdk_network_tools_ctrl(NETWOR_TOOLS_CTRL_FILE, NETWORK_TOOLS_CTRL_STA_MODE);
	if(ret < 0 && net_wifi_cli_status(dev) == WIFI_STATUS_ERROR)
		return ret;

	return WIFI_OK;
}

int net_wifi_cli_off(const WIFI_HANDLE dev)
{
    int ret;
    int mode = IW_MODE_INFRA;
	if(isNull(dev))
		return -1;
	//new_system_call("killall udhcpc");
    if ((ret = _net_wifi_cli_get_workmode(dev, &mode)) < 0) {
		NETWORK_PRINT_ERROR("ioctl[SIOCGIWMODE]");
		mode = IW_MODE_INFRA;
        net_wifi_cli_set_workmode(dev, IW_MODE_INFRA);
	}

	if (mode == IW_MODE_INFRA) {
		/*
		 * Clear the BSSID selection and set a random SSID to make sure
		 * the driver will not be trying to associate with something
		 * even if it does not understand SIOCSIWMLME commands (or
		 * tries to associate automatically after deauth/disassoc).
		 */
		int i;
		char ssid[32] = {0};
		net_wifi_cli_set_bssid(dev, MAC_EMPTY, strlen(MAC_EMPTY));

		for (i = 0; i < 32; i++)
			ssid[i] = rand() & 0xFF;
		net_wifi_cli_set_essid(dev, ssid, sizeof(ssid));
	}
	net_wifi_cli_terminate(dev);
//    net_wifi_down(dev);

	return WIFI_OK;
}
int net_wifi_cli_get_scanlist(const WIFI_HANDLE dev, SCAN_DataT *list, int len)
{
	int ret;
	struct iwreq wreq;
	unsigned int res_buf_len = IW_SCAN_MAX_DATA;
	unsigned char *res_buf;
	struct iw_event iwe_buf, *iwe = &iwe_buf;
	char *pos, *end, *custom;
	int count = 100;
	SCAN_DataT list_cache[50];
	SCAN_DataT * list_tmp = list_cache;
	int list_max = 50;
	int cur_num = list_max;


	if(dev == NULL || list == NULL || len == 0){
		NETWORK_PRINT_ERROR("bad params\n");
		return WIFI_FAIL;
	}
	/* clean list spase */
	memset(list_cache, 0, sizeof(list_cache));
	/* init scanning */
	memset(&wreq, 0, sizeof(wreq));
	wreq.u.data.pointer = NULL;
	wreq.u.data.length = 0;
	wreq.u.data.flags = IW_SCAN_DEFAULT;
	if ( _wifi_ioctl(dev, SIOCSIWSCAN, &wreq) < 0) {
		NETWORK_PRINT_ERROR("ioctl[SIOCSIWSCAN]: %s",strerror(errno));
		return WIFI_FAIL;
	}
    usleep(500*1000);

	res_buf = malloc(res_buf_len);
	count = 40; // 2 seconds timeout
	do{
		if (res_buf == NULL) {
			res_buf = malloc(res_buf_len);
			if (res_buf == NULL) return -1;
		}

		memset(&wreq, 0, sizeof(struct iwreq));
		wreq.u.data.pointer = res_buf;//if not write these codes , the program maybe wrong.
		wreq.u.data.length = res_buf_len;
		ret = _wifi_ioctl(dev, SIOCGIWSCAN, &wreq);
	//	printf("-----------------------1\n");
		/*get IEEE from ioctl*/
		if (ret == 0) break;
		if (errno == E2BIG && res_buf_len < 65535) {
			free(res_buf);
			res_buf = NULL;
			res_buf_len *= 2;
			if (res_buf_len > 65535)
				res_buf_len = 65535; /* 16-bit length field */
			NETWORK_PRINT_INFO("Scan results did not fit - "
				   "trying larger buffer (%lu bytes)\n",
				   (unsigned long) res_buf_len);
		} else {
			//NETWORK_PRINT_ERROR("ioctl[SIOCGIWSCAN]: ");
			//free(res_buf);
			usleep(50000);
		}
		count--;
	}while(count);

	if(count == 0)
	{
		NETWORK_PRINT_INFO("Scan timeout,No found wifi list");
		if(res_buf != NULL)
		{
			free(res_buf);
		}
		return WIFI_FAIL;
	}

	pos = (char *) res_buf;
	end = (char *) res_buf + wreq.u.data.length;
    #if 0
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    
    printf("len:%d\n",wreq.u.data.length);
    printf("%s\n",pos);
    
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	#endif
    int version = _wifi_cli_v19(dev);
	while (pos + IW_EV_LCP_LEN <= end && cur_num > 0) {
		/* Event data may be unaligned, so make a local, aligned copy
		 * before processing. */
		memcpy(&iwe_buf, pos, IW_EV_LCP_LEN);
		if (iwe->len <= IW_EV_LCP_LEN)
			break;

		custom = pos + IW_EV_POINT_LEN;
		if ((version == 0) && (iwe->cmd == SIOCGIWESSID || iwe->cmd == SIOCGIWENCODE ||
		 iwe->cmd == IWEVGENIE || iwe->cmd == IWEVCUSTOM)) {
			/* WE-19 removed the pointer from struct iw_point */
			char *dpos = (char *) &iwe_buf.u.data.length;
			int dlen = dpos - (char *) &iwe_buf;
			memcpy(dpos, pos + IW_EV_LCP_LEN,
				  sizeof(struct iw_event) - dlen);
		} else {
			memcpy(&iwe_buf, pos, sizeof(struct iw_event));
			custom += IW_EV_POINT_OFF;
		}
		switch (iwe->cmd) {
			case SIOCGIWAP:
			{
                if(list_tmp->used == 1){
    				list_tmp++;
    				cur_num--;
                }
                list_tmp->used = 1;

				memset(list_tmp->mac.name, 0, sizeof(list_tmp->mac.name));
				net_bytetomacstr(list_tmp->mac.name,  (unsigned char*)iwe->u.ap_addr.sa_data, MAC_LEN);
				list_tmp->mac.name_len = MAC_LEN*3-1;
				break;
			}
            case SIOCGIWMODE:
            {
			    list_tmp->mode = iwe->u.mode;
            }
			break;
            case SIOCGIWNAME:
                memcpy(list_tmp->protocol, iwe->u.name, IW_ESSID_MAX_SIZE);
            break;
			case SIOCGIWESSID:
			{
				int ssid_len = iwe->u.essid.length;
				if (custom + ssid_len > end)
					break;
				if (iwe->u.essid.flags) {
				    memset(list_tmp->ssid.name, 0, sizeof(list_tmp->ssid.name));
                    if(ssid_len > 0){
    					if(ssid_len <= IW_ESSID_MAX_SIZE){
                            memcpy(list_tmp->ssid.name, custom, ssid_len);
                            list_tmp->ssid.name_len = ssid_len;
    					} else {
                            memcpy(list_tmp->ssid.name, custom, IW_ESSID_MAX_SIZE);
                            list_tmp->ssid.name_len = IW_ESSID_MAX_SIZE;
                        }
                    } else {
                        //strcpy(list->ssid.name, "Hide");
                        //list->ssid.name_len = strlen("隐藏网络");
                        list_tmp->ssid.name[0] ='\0';
						list_tmp->ssid.name_len= 0;
                    }
                    list_tmp->ssid.name[IW_ESSID_MAX_SIZE-1] = '\0';
				} else {
					list_tmp->ssid.name[0] ='\0';
					list_tmp->ssid.name_len = 0;

                    //list->ssid.name_len = strlen("隐藏网络");
                    //list->ssid.name[IW_ESSID_MAX_SIZE-1] = '\0';

                }
			}break;
			case SIOCGIWFREQ:
			{
                int divi = 1000000, i;
                if(iwe->u.freq.m > 1000){
                	if (iwe->u.freq.e == 0) {
                        list_tmp->channel = _freq_to_channel(iwe->u.freq.m);
                	} else if (iwe->u.freq.e > 6) {
                		list_tmp->channel = 0;
                	} else {
                    	for (i = 0; i < iwe->u.freq.e; i++)
                    		divi /= 10;
                    	list_tmp->channel = _freq_to_channel(iwe->u.freq.m / divi);
                	}
                } else {
                    list_tmp->channel = iwe->u.freq.m;
                }

			}
			break;
			case IWEVQUAL:
			{
				if ((iwe->u.qual.updated & (IW_QUAL_DBM | IW_QUAL_RCPI)) ||
				    ((iwe->u.qual.level != 0))) {
				    /* Deal with quality : always a relative value */
				    if(!(iwe->u.qual.updated & IW_QUAL_QUAL_INVALID)) {
					 	list_tmp->qual = iwe->u.qual.qual;
					} else {
						list_tmp->qual = 0;
					}
					if(iwe->u.qual.updated & IW_QUAL_RCPI)
					{
						/* Deal with signal level in RCPI */
						/* RCPI = int{(Power in dBm +110)*2} for 0dbm > Power > -110dBm */
						if(!(iwe->u.qual.updated & IW_QUAL_LEVEL_INVALID))
					    {
					      list_tmp->level = (iwe->u.qual.level/2) - 110;
					    }
						/* Deal with noise level in dBm (absolute power measurement) */
						if(!(iwe->u.qual.updated & IW_QUAL_NOISE_INVALID))
						{
						  list_tmp->noise = (iwe->u.qual.noise/2) - 110;
						}
					} else {
						if(iwe->u.qual.updated & IW_QUAL_DBM)
					    {
					    	if(!(iwe->u.qual.updated & IW_QUAL_LEVEL_INVALID)) {
								if (iwe->u.qual.level >= 64)
									list_tmp->level = iwe->u.qual.level - 0x100;
								else
									list_tmp->level = iwe->u.qual.level;
					    	} else {
					    		list_tmp->level = 0;
					    	}
							if(!(iwe->u.qual.updated & IW_QUAL_NOISE_INVALID)) {
								if (iwe->u.qual.noise >= 64)
									list_tmp->noise = iwe->u.qual.noise - 0x100;
								else
									list_tmp->noise = iwe->u.qual.noise;
							} else {
								list_tmp->noise = 0;
							}
						} else {
							if(!(iwe->u.qual.updated & IW_QUAL_LEVEL_INVALID)) {
								list_tmp->level = iwe->u.qual.level;
					    	} else {
					    		list_tmp->level = 0;
					    	}
							if(!(iwe->u.qual.updated & IW_QUAL_NOISE_INVALID)) {
								list_tmp->noise = iwe->u.qual.noise;
							} else {
								list_tmp->noise = 0;
							}
						}
					}
				}else {
					list_tmp->qual= iwe->u.qual.qual;
					list_tmp->noise = iwe->u.qual.noise;
					list_tmp->level = iwe->u.qual.level;
				}
			}
			break;

			case SIOCGIWENCODE:
			{
                int i;
                unsigned char key[IW_ENCODING_TOKEN_MAX];
                unsigned char *buff = (unsigned char*)list_tmp->key;
                if(iwe->u.data.pointer){
                    //memcpy(key, iwe->u.data.pointer, iwe->u.data.length);
                }
                else
                    iwe->u.data.flags |= IW_ENCODE_NOKEY;

				if ((iwe->u.data.flags&IW_ENCODE_FLAGS) == IW_ENCODE_DISABLED){
					list_tmp->encode = WIFI_ENCRYPT_OFF;
				} else if((iwe->u.data.flags&IW_ENCODE_FLAGS) == IW_ENCODE_RESTRICTED){
					list_tmp->encode = WIFI_ENCRYPT_RESTRICTED;
				} else if((iwe->u.data.flags&IW_ENCODE_FLAGS) == IW_ENCODE_OPEN){
					list_tmp->encode = WIFI_ENCRYPT_OPEN;
				} else if((iwe->u.data.flags&IW_ENCODE_FLAGS) == IW_ENCODE_ENABLED){
				    list_tmp->encode = WIFI_ENCRYPT_ENABLED;
				} else if((iwe->u.data.flags&IW_ENCODE_FLAGS) == IW_ENCODE_TEMP){
				    list_tmp->encode = WIFI_ENCRYPT_TEMP;
				} else {
				    list_tmp->encode = WIFI_ENCRYPT_UNKNOW;
				}

                if((iwe->u.data.length*3) <= IW_ENCODING_TOKEN_MAX){
                    if(iwe->u.data.flags & IW_ENCODE_NOKEY){
                        if(iwe->u.data.length <= 0){
                            strcpy((char *)buff,"on");
                            list_tmp->encode = WIFI_ENCRYPT_ON;
                        }
                        else {
                            strcpy((char *)buff,"**");
                            buff += 2;
                            for(i=1; i < iwe->u.data.length; i++){
                                if((i&0x01) == 0){
                                    strcpy((char *)buff, "-");
                                    buff++;
                                    strcpy((char *)buff, "**");
                                    buff += 2;
                                }
                            }
                        }
                    } else {
                        sprintf((char *)buff,"%.2x", key[0]);
                        buff += 2;
                        for(i=1; i < iwe->u.data.length; i++){
                            if((i&0x01) == 0){
                                strcpy((char *)buff, "-");
                                buff++;
                                sprintf((char *)buff,"%.2x", key[i]);
                                buff += 2;
                            }
                        }
                    }
                }
			}
			break;

            case SIOCGIWRATE:
            {
                int maxrate;
                char *cust = pos + IW_EV_LCP_LEN;
                struct iw_param p;
                size_t clen;
                clen = iwe->len;

                if(cust + clen > end){
                    list_tmp->used = 1;
    				list_tmp++;
    				cur_num--;
                    break;
                }
                maxrate = 0;
                while(((ssize_t)clen) >= (ssize_t)sizeof(struct iw_param)){
                    memcpy(&p, cust, sizeof(struct iw_param));
                    if(p.value > maxrate)
                        maxrate = p.value;
                    clen -= sizeof(struct iw_param);
                    cust += sizeof(struct iw_param);
                }
                list_tmp->maxrate = maxrate / 500000;
            }
            break;
            case IWEVGENIE:
                if(iwe->u.data.length + custom > end)break;
                _wifi_gen_ie(list_tmp->gen_ie, sizeof(list_tmp->gen_ie)/sizeof(GEN_IE_DataT),
                    (unsigned char*)custom, iwe->u.data.length);
            break;

            case IWEVCUSTOM:
            {

            }break;
		}
		pos += iwe->len;


	}

	/* when cur_num < 1, out of bound */
	if(cur_num < 1)
		cur_num = 1;

	/* scan data dbm order*/
	ret = _net_wifi_cli_scanorder(list_cache, list_max - cur_num + 1, WIFI_SCAN_ORDER_DBM);
	if(ret < 0){
		NETWORK_PRINT_INFO("order fail\n");
	}

	int vild_len = ((list_max - cur_num + 1)<len)?(list_max - cur_num + 1):len;
	memcpy(list, list_cache, sizeof(SCAN_DataT) * vild_len);

	free(res_buf);
	return WIFI_OK;
}


int net_wifi_cli_set_workmode(const WIFI_HANDLE dev, int mode)
{
	int ret;
	struct iwreq wreq;

	if(dev == NULL){
		printf("bad params\n");
		return WIFI_FAIL;
	}
	memset(&wreq, 0, sizeof(struct iwreq));

	wreq.u.mode = mode;

	ret = _wifi_ioctl(dev, SIOCSIWMODE, &wreq);
	/*set mode from ioctl*/
	if (ret) {
		NETWORK_PRINT_ERROR("IOCTL SIOCSIWMODE Failed,error");
		return ret;
	} else {
		//printf("IOCTL SIOCGIWESSID Successfull/n");
	}

	return WIFI_OK;
}

int net_wifi_cli_terminate(const WIFI_HANDLE dev)
{
#ifdef WPA_CTL_INTERFACE
	if(isNull(dev))
        return WIFI_FAIL;
	return net_wpa_ctrl_command(dev, "TERMINATE");
#endif
}

int net_wifi_cli_add_network(const WIFI_HANDLE dev)
{
#ifdef WPA_CTL_INTERFACE
    char buf[2048];
	size_t len;
	int ret, i;
    struct wpa_ctrl *pstWpaCtl;
    if(isNull(dev))
        return WIFI_FAIL;

    pstWpaCtl = wpa_ctrl_open(dev, WPA_CTL_INTERFACE);
    if(pstWpaCtl == NULL)return WIFI_FAIL;

	len = sizeof(buf) - 1;
	ret = wpa_ctrl_request(pstWpaCtl, "ADD_NETWORK", strlen("ADD_NETWORK"), buf, &len, NULL);
	if (ret == -2) {
		printf("command timed out.\n");
		return WIFI_TIMEOUT;
	} else if (ret < 0) {
		printf("command failed.\n");
		return WIFI_FAIL;
	}

    if(buf[0]>='0' && buf[0]<='9'){
        ret = 0;
        for(i=0; buf[i]>='0' && buf[i]<='9'; i++){
            ret *= 10;
            ret = buf[i] - '0';
        }
        wpa_ctrl_close(pstWpaCtl);
        return ret;
    }

    wpa_ctrl_close(pstWpaCtl);
    return WIFI_FAIL;
#endif
	return WIFI_FAIL;
}
int	net_wifi_cli_smartlink_start(const WIFI_HANDLE dev, unsigned char channels)
{
	char cmd[ELIAN_CHAR_MAX] = {0};
	if(isNull(dev)) {
		NETWORK_PRINT_ERROR("Smart Link params error");
		return WIFI_FAIL;
	}
	if(channels == 0xff){
		snprintf(cmd, sizeof(cmd), "start");
	} else {
		snprintf(cmd, sizeof(cmd), "start_ch=%d", channels);
	}
	return _wifi_cli_smartlink_ioctl(dev, cmd, strlen(cmd));
}

int	net_wifi_cli_smartlink_stop(const WIFI_HANDLE dev)
{
	char cmd[ELIAN_CHAR_MAX] = {0};
	if(isNull(dev)) {
		NETWORK_PRINT_ERROR("Smart Link params error");
		return WIFI_FAIL;
	}
	snprintf(cmd, sizeof(cmd), "stop");
	return _wifi_cli_smartlink_ioctl(dev, cmd, strlen(cmd));
}

int	net_wifi_cli_smartlink_clear(const WIFI_HANDLE dev)
{
	char cmd[ELIAN_CHAR_MAX] = {0};
	if(isNull(dev)) {
		NETWORK_PRINT_ERROR("Smart Link params error");
		return WIFI_FAIL;
	}
	snprintf(cmd, sizeof(cmd), "clear");
	return _wifi_cli_smartlink_ioctl(dev, cmd, strlen(cmd));
}

int	net_wifi_cli_smartlink_get_result(const WIFI_HANDLE dev, ELIAN_ResultT *result)
{
	int ret = 0;
	char cmd[ELIAN_CHAR_MAX] = {0};
	if(isNull(dev)) {
		NETWORK_PRINT_ERROR("Smart Link params error");
		return WIFI_FAIL;
	}
	snprintf(cmd, sizeof(cmd), "result");
	ret = _wifi_cli_smartlink_ioctl(dev, cmd, strlen(cmd));
	if(ret < 0)
		return WIFI_FAIL;
	_wifi_cli_smartlink_strtovaule(cmd, "AM=", ELIAN_VALUE_U8,
		(void *)&(result->auth_mode), sizeof(result->auth_mode));
	_wifi_cli_smartlink_strtovaule(cmd, "ssid=", ELIAN_VALUE_STRING,
		(void *)&(result->ssid[0]), sizeof(result->ssid));
	_wifi_cli_smartlink_strtovaule(cmd, "pwd=", ELIAN_VALUE_STRING,
		(void *)&(result->pwd[0]), sizeof(result->pwd));
	_wifi_cli_smartlink_strtovaule(cmd, "cust_data_len=", ELIAN_VALUE_U8,
		(void *)&(result->cust_data_len), sizeof(result->cust_data_len));
	_wifi_cli_smartlink_strtovaule(cmd, "user=", ELIAN_VALUE_STRING,
		(void *)&(result->user[0]), sizeof(result->user));
	_wifi_cli_smartlink_strtovaule(cmd, "cust_data=", ELIAN_VALUE_STRING,
		(void *)&(result->cust_data[0]), sizeof(result->cust_data));

	return WIFI_OK;
}

int	net_wifi_cli_smartlink_set_ch(const WIFI_HANDLE dev, unsigned char channels)
{
	char cmd[ELIAN_CHAR_MAX] = {0};
	if(isNull(dev)) {
		NETWORK_PRINT_ERROR("Smart Link params error");
		return WIFI_FAIL;
	}
	snprintf(cmd, sizeof(cmd), "set_ch=%d", channels);
	return _wifi_cli_smartlink_ioctl(dev, cmd, strlen(cmd));
}


#endif

int net_wifi_cli_remove_network(const WIFI_HANDLE dev, int seq)
{
#ifdef WPA_CTL_INTERFACE
    int ret;
    char cmd[CMD_LEN_MIN] = {0};
    ret = snprintf(cmd, sizeof(cmd), "REMOVE_NETWORK %d", seq);
	if (ret < 0 || (size_t) ret >= sizeof(cmd))
		return -1;
	cmd[sizeof(cmd) - 1] = '\0';

	return net_wpa_ctrl_command(dev, cmd);
#endif
}

int net_wifi_cli_select_network(const WIFI_HANDLE dev, int seq)
{
#ifdef WPA_CTL_INTERFACE
    int ret;
    char cmd[CMD_LEN_MIN] = {0};
    ret = snprintf(cmd, sizeof(cmd), "SELECT_NETWORK %d", seq);
	if (ret < 0 || (size_t) ret >= sizeof(cmd))
		return WIFI_FAIL;
	cmd[sizeof(cmd) - 1] = '\0';

	return net_wpa_ctrl_command(dev, cmd);
#endif
}

int net_wifi_cli_enable_network(const WIFI_HANDLE dev, int seq)
{
#ifdef WPA_CTL_INTERFACE
    int ret;
    char cmd[CMD_LEN_MIN] = {0};
    ret = snprintf(cmd, sizeof(cmd), "ENABLE_NETWORK %d", seq);
	if (ret < 0 || (size_t) ret >= sizeof(cmd))
		return WIFI_FAIL;
	cmd[sizeof(cmd) - 1] = '\0';

	return net_wpa_ctrl_command(dev, cmd);
#endif
}

int net_wifi_cli_disable_network(const WIFI_HANDLE dev, int seq)
{
#ifdef WPA_CTL_INTERFACE
    int ret;
    char cmd[CMD_LEN_MIN] = {0};
    ret = snprintf(cmd, sizeof(cmd), "DISABLE_NETWORK %d", seq);
	if (ret < 0 || (size_t) ret >= sizeof(cmd))
		return -1;
	cmd[sizeof(cmd) - 1] = '\0';

	return net_wpa_ctrl_command(dev, cmd);
#endif
}

#ifdef NETWORK_WPS_SUPPORT
int net_wifi_cli_wps_pbc(const WIFI_HANDLE dev, const char *bssid, int udhcp)
{
#ifdef WPA_CTL_INTERFACE
    int ret;
    char cmd[CMD_LEN_MAX] = {0};
	if(isNull(bssid)){
	    ret = snprintf(cmd, sizeof(cmd), "WPS_PBC");
		if (ret < 0 || (size_t) ret >= sizeof(cmd))
			return WIFI_FAIL;
	} else {
	    ret = snprintf(cmd, sizeof(cmd), "WPS_PBC %s", bssid);
		if (ret < 0 || (size_t) ret >= sizeof(cmd))
			return WIFI_FAIL;
	}
	cmd[sizeof(cmd) - 1] = '\0';
	net_wifi_cli_disconnect(dev);
	ret = net_wpa_ctrl_command(dev, cmd);
	if(ret != WIFI_OK) {
		NETWORK_PRINT_ERROR("No device [%s] or wpa_supplicant version not support\n", dev);
		return WIFI_FAIL;
	}
	ret = _wifi_cli_linkwpsmsg(dev, 120);//120 s
	if(udhcp && (ret == WIFI_OK)) {
		_wifi_cli_udpchc(dev);
	}

	return ret;
#endif
}

int net_wifi_cli_wps_pin(const WIFI_HANDLE dev,
	const char *bssid, const char *pin, int udhcp)
{
#ifdef WPA_CTL_INTERFACE
    int ret;
    char cmd[CMD_LEN_MAX] = {0};
	if(isNull(bssid)| isNull(pin)){
		NETWORK_PRINT_INFO("Bad params device[0x%08d] bssid[0x%08d] pin[0x%08d]\n",
			(int)dev, (int)bssid, (int)pin);
		return WIFI_FAIL;
	}

    ret = snprintf(cmd, sizeof(cmd), "WPS_PIN %s %s", bssid, pin);
	if (ret < 0 || (size_t) ret >= sizeof(cmd))
		return WIFI_FAIL;

	cmd[sizeof(cmd) - 1] = '\0';

	ret = net_wpa_ctrl_command(dev, cmd);
	if(ret != WIFI_OK) {
		NETWORK_PRINT_ERROR("No device [%s] or wpa_supplicant version not support\n", dev);
		return WIFI_FAIL;
	}
	ret = _wifi_cli_linkmsg(dev, 60);//60 s
	if(udhcp && (ret == WIFI_OK)) {
		_wifi_cli_udpchc(dev);
	}

	return ret;
#endif
}


char * net_wifi_cli_wps_gen_pin(const WIFI_HANDLE dev, char *bssid)
{
#ifdef WPA_CTL_INTERFACE
    int ret;
	char buf[2048] = {0};
	size_t len;
    char cmd[CMD_LEN_MAX] = {0};
	struct wpa_ctrl *pstWpaCtl;
	char *pin = NULL;

	if(isNull(dev) || isNull(bssid))
		return NULL;

	ret = snprintf(cmd, sizeof(cmd), "WPS_PIN %s", bssid);
	if (ret < 0 || (size_t) ret >= sizeof(cmd))
		return NULL;
	cmd[sizeof(cmd) - 1] = '\0';

    pstWpaCtl = wpa_ctrl_open(dev, WPA_CTL_INTERFACE);
    if(pstWpaCtl == NULL)return NULL;

	len = sizeof(buf) - 1;
	ret = wpa_ctrl_request(pstWpaCtl, cmd, strlen(cmd), buf, &len, NULL);
	if (ret < 0) {
		NETWORK_PRINT_ERROR("'%s' command failed.\n", cmd);
		return NULL;
	}
	wpa_ctrl_close(pstWpaCtl);

	len = strlen(buf);
	if(len <= 0){
		return NULL;
	}
	pin = malloc(len+1);
	if(pin == NULL) return NULL;
	memset(pin, 0, len+1);
	memcpy(pin, buf, len);

	return pin;
#endif
}

int net_wifi_cli_wps_reg(const WIFI_HANDLE dev,const char *bssid,
	const char *pin)
{
#ifdef WPA_CTL_INTERFACE
    int ret;
    char cmd[CMD_LEN_MAX] = {0};
	if(isNull(bssid) || isNull(pin))
		return WIFI_FAIL;

	ret = snprintf(cmd, sizeof(cmd), "WPS_REG %s %s", bssid, pin);
	if (ret < 0 || (size_t) ret >= sizeof(cmd))
		return WIFI_FAIL;

	cmd[sizeof(cmd) - 1] = '\0';

	return net_wpa_ctrl_command(dev, cmd);
#endif
}

#endif

int net_wifi_cli_set_network(const WIFI_HANDLE dev, int seq,
        int flag, const char *content)
{
#ifdef WPA_CTL_INTERFACE
    int ret;
    char cmd[CMD_LEN_MAX*2] = {0};
    const char flag_arry[][12] ={"ssid", "psk", "key_mgmt",
        "indentity", "password", "bssid", "scan_ssid"};

    if(flag < 0 || flag > WIFI_SET_MAXNUM || content == NULL)
        return WIFI_FAIL;

	if(flag != WIFI_SET_KEY_MGMT
		&& flag != WIFI_SET_BSSID
		&& flag != WIFI_SET_SCAN_SSID)
    	ret = snprintf(cmd, sizeof(cmd), "SET_NETWORK %d %s \"%s\"",
        	seq, flag_arry[flag], content);
	else
	 	ret = snprintf(cmd, sizeof(cmd), "SET_NETWORK %d %s %s",
        	seq, flag_arry[flag], content);

	if (ret < 0 || (size_t) ret >= sizeof(cmd))
		return -1;
	cmd[sizeof(cmd) - 1] = '\0';

	return net_wpa_ctrl_command(dev, cmd);
#endif
}


int net_wifi_cli_save_config(const WIFI_HANDLE dev)

{
#ifdef WPA_CTL_INTERFACE
    return net_wpa_ctrl_command(dev, "SAVE_CONFIG");
#endif
}


int net_wifi_cli_status(const WIFI_HANDLE dev)
{
    int ret;
    static int last_status = -1;

    if(!dev)
        return WIFI_STATUS_ERROR;
#ifdef WPA_CTL_INTERFACE
    struct wpa_ctrl *pstWpaCtl;
    char *status = NULL;
    int stlen = 0;

    pstWpaCtl = wpa_ctrl_open(dev, WPA_CTL_INTERFACE);
    if(pstWpaCtl == NULL)
    {
        return WIFI_STATUS_ERROR;
    }
    ret = _wpa_get_connect_status(pstWpaCtl, &status, &stlen);
    if(ret < 0 || status == NULL){
        wpa_ctrl_close(pstWpaCtl);
        return WIFI_STATUS_ERROR;
    }
    //fprintf(stderr, "\rstatus :%s\n",status);
	//fflush(stderr);
    if(strcmp(status, "COMPLETED") == 0)
        ret = WIFI_STATUS_CONNECTED;
    else if(strcmp(status, "DISCONNECTED") == 0)
        ret = WIFI_STATUS_DISCONNECTED;
    else if(strcmp(status, "INACTIVE") == 0)
        ret = WIFI_STATUS_INACTIVE;
	else if (strcmp(status, "SCANNING") == 0
		|| strcmp(status, "4WAY_HANDSHAKE") == 0
		|| strcmp(status, "GROUP_HANDSHAKE") == 0
		|| strcmp(status, "ASSOCIATED") == 0
		|| strcmp(status, "ASSOCIATING") == 0){
        ret = WIFI_STATUS_CONNECTING;
    } else {
        ret = WIFI_STATUS_UNKNOW;
    }
    if (last_status != ret) {
        printf("\033[0;31m[APP WiFi] [%s, Line:%d] WiFi connect status:%s\033[0;0m\n", __FILE__, __LINE__, status);
        last_status = ret;
    }
    /* get status, need free space */
    free(status);
    wpa_ctrl_close(pstWpaCtl);
    return ret;
#else
    char mac[32];
    ret = net_wifi_cli_get_bssid(dev, mac, MAC_LEN);
    if(ret < 0)
        return WIFI_STATUS_ERROR;
    if(strcmp(mac, MAC_EMPTY) == 0) {
        ret = WIFI_STATUS_DISCONNECTED;
    } else {
        ret = WIFI_STATUS_CONNECTED;
    }

    return ret;
#endif
}



/* dev is wifi device name, name is SSID of AP, passwd is link key, usedhcp is wether use udhcpc (0:no use !0:use) */
int net_wifi_cli_connecting(const WIFI_HANDLE dev, SCAN_DataT *link_data,
    const char *ssid, const char *key, unsigned char usedhcp)
{
    int ret = 0;
    int seq = -1;
    const char *dev_ssid = NULL;

	if(dev == NULL || (link_data == NULL && ssid == NULL) ){
		NETWORK_PRINT_INFO("Bad params device[0x%08d] link_data[0x%08d] ssid[0x%08d]\n",
			(int)dev, (int)link_data, (int)ssid);
		return WIFI_FAIL;
	}

	ret = net_wifi_cli_status(dev);
	if(ret >= 0){
		ret = net_wifi_cli_disconnect(dev);
		if(ret < 0){
	        return WIFI_FAIL;
		}
		usleep(10*1000);
	}

    if(ssid != NULL)
        dev_ssid = ssid;
    else
        dev_ssid = link_data->ssid.name;
	/* SSID string length > 0 */
	if(dev_ssid[0] == '\0'){
        NETWORK_PRINT_ERROR("No ssid\n");
		return WIFI_NO_SSID;
	}
    /* find seq of ssid, if not have seq return -1; */
    seq = wifi_find_seq(dev, dev_ssid);
    if(seq >= 0){
		ret = net_wifi_cli_disable_network(dev, seq);
		if(ret < 0){
	        return WIFI_FAIL;
		}
    } else {
		seq = net_wifi_cli_add_network(dev);
	    if(seq < 0){
			return WIFI_FAIL;
	    }
    }
	NETWORK_PRINT_INFO("Find network number: %d\n", seq);
	/* set wireless SSID*/
    ret = net_wifi_cli_set_network(dev, seq, WIFI_SET_SSID, dev_ssid);
    if(ret < 0){
        NETWORK_PRINT_ERROR("Set network SSID fail\n");
        return WIFI_FAIL;
    }
	ret = net_wifi_cli_set_network(dev, seq, WIFI_SET_SCAN_SSID, "1");
    if(ret < 0){
        NETWORK_PRINT_ERROR("Set network SCAN_SSID fail\n");
        return WIFI_FAIL;
    }
    /* set psk */
	if(key != NULL && key[0] != 0){
	    ret = net_wifi_cli_set_network(dev, seq, WIFI_SET_PSK, key);
	    if(ret < 0){
	        NETWORK_PRINT_ERROR("Password is less than eight or format error\n");
	        return WIFI_FAIL;
	    }
	}
	else
	{
	    ret = net_wifi_cli_set_network(dev, seq, WIFI_SET_KEY_MGMT, "NONE");
	    if(ret < 0){
	        NETWORK_PRINT_ERROR("Set network fail\n");
	        return WIFI_FAIL;
	    }
	}

    ret = net_wifi_cli_select_network(dev, seq);
    if(ret < 0){
        return WIFI_FAIL;
    }

    ret = net_wifi_cli_enable_network(dev, seq);
    if(ret < 0){
        return WIFI_FAIL;
    }

    ret = net_wifi_cli_reconnect(dev);
    if(ret < 0){
        return WIFI_FAIL;
	}

	ret = _wifi_cli_linkmsg(dev, 15);//15 second
    if(usedhcp && (ret == WIFI_OK)) {
		_wifi_cli_udpchc(dev);
	}

	if(ret != WIFI_OK) {
		net_wifi_cli_disconnect(dev);
	}
    return ret;
}

int net_wifi_cli_disconnect(const WIFI_HANDLE dev)
{
#ifdef WPA_CTL_INTERFACE
    return net_wpa_ctrl_command(dev, "DISCONNECT");
#else
    return net_wifi_cli_off(dev);
#endif

}


int net_wifi_cli_reconnect(const WIFI_HANDLE dev)
{
#ifdef WPA_CTL_INTERFACE
    return net_wpa_ctrl_command(dev, "RECONNECT");
#else
    int ret;
    char cmd[128] = {0};
    if(net_wifi_cli_status(dev) == WIFI_STATUS_CONNECTED)
        net_wifi_cli_disconnect(dev);
    memset(cmd, 0, sizeof(cmd));
	sprintf(cmd,"%s -B -i%s -c %s", WPA_SUPPLICANT_CMD, dev, PASSWD_CFG);
    ret = new_system_call(cmd);
    //ret = sdk_network_tools_ctrl(NETWOR_TOOLS_CTRL_FILE, NETWORK_TOOLS_CTRL_STA_MODE);
	if(usedhcp){
		memset(cmd, 0, sizeof(cmd));
		sprintf(cmd,"%s -b -i%s", UDHCPC_CMD, dev);
		new_system_call(cmd);
		//ret = sdk_network_tools_ctrl(NETWOR_TOOLS_CTRL_FILE, NETWORK_TOOLS_CTRL_WIFI_DHCP);
	}
#endif
}

int net_wifi_cli_ping(const WIFI_HANDLE dev)
{
#ifdef WPA_CTL_INTERFACE
    char buf[2048];
	size_t len;
	int ret;
    struct wpa_ctrl *pstWpaCtl;
    if(isNull(dev))
        return WIFI_FAIL;

    pstWpaCtl = wpa_ctrl_open(dev, WPA_CTL_INTERFACE);
    if(pstWpaCtl == NULL)return WIFI_FAIL;

	len = sizeof(buf) - 1;
	ret = wpa_ctrl_request(pstWpaCtl, "PING", strlen("PING"), buf, &len, NULL);
	if (ret == -2) {
		NETWORK_PRINT_ERROR("PING command failed.\n");
		return WIFI_FAIL;
	}
	wpa_ctrl_close(pstWpaCtl);
    if(strncmp("PONG", buf, strlen("PONG"))){
        return WIFI_FAIL;
    }
    return WIFI_OK;
#endif
}

int net_wifi_cli_get_network(const WIFI_HANDLE dev, int seq,
    int flag, char *buf, int buflen)
{
#ifdef WPA_CTL_INTERFACE
    int ret;
    struct wpa_ctrl *pstWpaCtl;
    char cmd[CMD_LEN_MAX*2] = {0};
    const char flag_arry[][12] ={"ssid", "psk", "key_mgmt",
        "indentity", "password", "bssid", "scan_ssid"};

    if(flag < 0 || flag > WIFI_SET_MAXNUM || buf == NULL)
        return WIFI_FAIL;

    ret = snprintf(cmd, sizeof(cmd), "GET_NETWORK %d %s",
        seq, flag_arry[flag]);
	if (ret < 0 || (size_t) ret >= sizeof(cmd))
		return WIFI_FAIL;
	cmd[sizeof(cmd) - 1] = '\0';
    pstWpaCtl = wpa_ctrl_open(dev, WPA_CTL_INTERFACE);
    if(pstWpaCtl == NULL)return WIFI_FAIL;

    memset(buf, 0, buflen);
	ret = wpa_ctrl_request(pstWpaCtl, cmd, strlen(cmd), buf, (unsigned int*)&buflen, NULL);
    buf[buflen - 1] = '\0';
	if (ret < 0 || strcmp(buf, "FAIL") == 0) {
		printf("'%s' command failed.\n", cmd);
        wpa_ctrl_close(pstWpaCtl);
		return WIFI_FAIL;
	}
    wpa_ctrl_close(pstWpaCtl);

	return WIFI_OK;
#endif
}



int net_wifi_cli_scan(const WIFI_HANDLE dev)
{
#ifdef WPA_CTL_INTERFACE
    char buf[2048];
	size_t len;
	int ret;
    struct wpa_ctrl *pstWpaCtl;
    if(isNull(dev))
        return WIFI_FAIL;

    pstWpaCtl = wpa_ctrl_open(dev, WPA_CTL_INTERFACE);
    if(pstWpaCtl == NULL)
    {
        NETWORK_PRINT_ERROR(" wpa_ctrl_open failed\n");
        return WIFI_FAIL;
    }

	len = sizeof(buf) - 1;
	ret = wpa_ctrl_request(pstWpaCtl, "SCAN", strlen("SCAN"), buf, &len, NULL);
	if (ret == -2) 
    {
		NETWORK_PRINT_ERROR(" command SCAN timed out.\n");
		return WIFI_TIMEOUT;
	} else if (ret < 0) 
    {
		NETWORK_PRINT_ERROR("SCAN command failed.\n");
		return WIFI_FAIL;
	}
    
    buf[len] = '\0';
    if(len >= 2 && strncmp(buf, "OK", 2) == 0)
    {        
		NETWORK_PRINT_ERROR("SCAN result:%s.\n",buf);
    }
    else
    {  
        return WIFI_FAIL;
    }

	wpa_ctrl_close(pstWpaCtl);
    return WIFI_OK;
#endif
}


int net_wifi_cli_strtoint(char *str)
{
	char *cmd_cur = str;
    unsigned int buf = 0;
    

    while(*cmd_cur != '\0')
    {  
        if(*cmd_cur <= '9' && *cmd_cur >= '0')
        {
            buf *= 10;
            buf += *cmd_cur - '0';            
            cmd_cur++;
        }
        else
        {
            NETWORK_PRINT_ERROR("not a number\n");
            return -1;
        }
    }  

    return buf;
}


/*
wifi channel rang from 1 ~ 13,with center frequecy range :
2412k,2417,2422,2427,2432,  2437  ,3442,2447,2452,2457,  2462  ,2467,2472
each center frequency is 5M interval ,a channel range 20M between center 
frequency( 2M more between two channels )
e.g 
channe  l range 2401~2423  (2412)
channe  2 range 2406~2428  (2417)
.
channe  6 range 2426~2448  (2437)
.
channe 11 range 2451~2473  (2462)
.

*/
int net_wifi_cli_fre2channel(int freq)
{

    if(freq >= 2407)
    {
        return (freq - 2407)/5;
    }
 
    else
    {
        NETWORK_PRINT_ERROR("frequency :%d is out of range\n",freq);
        return -1;
    }
    
    
}

/*
    return encrypt mode 0~8, -1 for error.
*/
int net_wifi_cli_encrypt(char *buf)
{

    unsigned int encMode = WPAECN_NONE;
    

    if(0==strncmp(buf,WPA_TKIP_ENC,strlen(WPA_TKIP_ENC)))
    {
        encMode = WPAPSK_TKIP;
    } 
    else if(0==strncmp(buf,WPA_AES_ENC,strlen(WPA_AES_ENC)))
    {
        encMode = WPAPSK_AES;
    }
    else if(0==strncmp(buf,WPA2_TKIP_ENC,strlen(WPA2_TKIP_ENC)))
    {
        encMode = WPA2PSK_TKIP;
    }
    else if(0==strncmp(buf,WPA2_AES_ENC,strlen(WPA2_AES_ENC)))
    {
        encMode = WPA2PSK_AES;
    }
    else if(0==strncmp(buf,WPS_ENC,strlen(WPS_ENC)))
    {
        /* need to confirm this value .......*/
        encMode = WEP_64_ASSCI;
    }
    else 
    {
        NETWORK_PRINT_ERROR("unknow encrypt mode:%s!\n",buf);
        
        encMode = WPAECN_NONE;
    }
    

    return encMode;

}

/*
scan result like this:
bssid / frequency / signal level / flags / ssid
4c:e6:76:46:8e:e3       2427    183     [WPA-PSK-TKIP][WPS]     4-508
bc:46:99:91:81:08       2412    187     [WPA-PSK-CCMP][WPA2-PSK-CCMP]   GokeIpc_1

*/
int net_wifi_cli_str2scanlist(char * buff,WIFI_SCAN_LIST_t *list, int *number)
{
    char *pStart,*pEnd;
    char cnt = 0;
    char menuLen=strlen(menu);
    char clo = 0;
    char cloLen =0;
    char tmpBuf[100];
    int chn = 0;
    pStart=pEnd=buff + menuLen;

    

    while(*pEnd!='\0')
    {
    

        if('\t'==*pEnd)
        {
        
    
            if(0==clo)//bssid
            {
            
            //printf("DDDDDDDD %s,%d list[%d] macAddr:0x%x ,%xstart:0x%x,end:0x%x\n",
              //  __FUNCTION__,__LINE__,cnt,list[cnt].mac,pStart,pEnd);
                strncpy(list[cnt].mac,pStart,pEnd-pStart);             
                list[cnt].mac[pEnd-pStart] = '\0';                            
                //printf ("ZZ %s ZZ",list[cnt].mac);                   
            }
            
            if(1==clo)//fre
            {  
                strncpy(tmpBuf,pStart,pEnd-pStart);
                tmpBuf[pEnd-pStart]='\0';
                
                //printf ("ZZ %s ZZ",tmpBuf);
                chn = net_wifi_cli_strtoint(tmpBuf); 
                list[cnt].channel = net_wifi_cli_fre2channel(chn);
                
                //printf ("LL %d LL",list[cnt].channel);
            }
            if(2==clo)//signel level
            {
                strncpy(tmpBuf,pStart,pEnd-pStart);
                tmpBuf[pEnd-pStart]='\0';
                
                //printf ("ZZ %s ZZ",tmpBuf);
                
                list[cnt].quality = net_wifi_cli_strtoint(tmpBuf);
                
                //printf ("LL %d LL",list[cnt].quality);
            }

            if(3==clo)//flags means encryption mode
            {
            
        
                strncpy(tmpBuf,pStart,pEnd-pStart);
                tmpBuf[pEnd-pStart]='\0';
                //printf ("ZZ %s ZZ",tmpBuf);
                list[cnt].security = net_wifi_cli_encrypt(tmpBuf);
                //printf ("LL %d LL",list[cnt].security);
            }
            
        
            pEnd++;
            pStart = pEnd;
            clo++;
            
        }
        else if('\n'==*pEnd)
        {
        
    
            strncpy(list[cnt].essid,pStart,pEnd-pStart);
            list[cnt].essid[pEnd-pStart] = '\0';
            
            //printf ("ZZ %s ZZ\n",list[cnt].essid);
            pEnd++;
            pStart = pEnd;
            clo = 0;
            list[cnt].vaild = 1;
            cnt++;
            if(cnt >=20)
            {
                break;
            }

        }
        else
        {
            pEnd++;
        }
    }
    *number = cnt;
    

    return 0;
    
}
int net_wifi_cli_sortList(WIFI_SCAN_LIST_t *list, int number)
{
    WIFI_SCAN_LIST_t tmpList;
    int i,j;
    unsigned int maxQual;
    float qual;
    for(i = 0;i < number;i++)
    {   //
        for(j=i+1;j<number;j++)
        {
            if(list[j].quality > list[i].quality)
            {
                tmpList = list[i];
                list[i] = list[j];
                list[j] = tmpList;
            }
        }
    }
    maxQual = list[0].quality;
    for(i = 0;i < number;i++)
    {   //
        qual = (float)list[i].quality/(float)maxQual ; 
        
        printf("xzl qual=%f ,%d\n",qual,qual);
        qual= qual * 100;
        list[i].quality =qual;
        printf("xzl qual=%f ,%d\n",qual,list[i].quality);
    }
    
}

int net_wifi_cli_scan_result(const WIFI_HANDLE dev,WIFI_SCAN_LIST_t *list, int *number)
{
#ifdef WPA_CTL_INTERFACE
    char buf[4096];
	size_t len;
	int ret;
    struct wpa_ctrl *pstWpaCtl;
    if(isNull(dev))
        return WIFI_FAIL;

    pstWpaCtl = wpa_ctrl_open(dev, WPA_CTL_INTERFACE);
    if(pstWpaCtl == NULL)
    {
        NETWORK_PRINT_ERROR(" wpa_ctrl_open failed\n");
        return WIFI_FAIL;
    }

	len = sizeof(buf) - 1;
	ret = wpa_ctrl_request(pstWpaCtl, "SCAN_RESULTS", strlen("SCAN_RESULTS"), buf, &len, NULL);
	if (ret == -2) 
    {
		NETWORK_PRINT_ERROR(" command SCAN_RESULTS timed out.\n");
		return WIFI_TIMEOUT;
	} else if (ret < 0) 
	{
		NETWORK_PRINT_ERROR("SCAN_RESULTS command failed.\n");
		return WIFI_FAIL;
	}
    
    printf("#########################\n");
    printf("buf len:%d\n",len);
    printf("%s\n", buf);
    printf("#########################\n");


    net_wifi_cli_str2scanlist(buf,list,number);
    net_wifi_cli_sortList(list,*number);


	wpa_ctrl_close(pstWpaCtl);
    return WIFI_OK;
#endif
}

#if 0
int net_wifi_cli_scan_result_bss(const WIFI_HANDLE dev)
{
#ifdef WPA_CTL_INTERFACE
    char buf[2048];
	size_t len;
	int ret;
    struct wpa_ctrl *pstWpaCtl;
    if(isNull(dev))
        return WIFI_FAIL;
    printf("FFFF   XXXXXXXX %s,%d \n",__FUNCTION__,__LINE__);

    pstWpaCtl = wpa_ctrl_open(dev, WPA_CTL_INTERFACE);
    if(pstWpaCtl == NULL)
    {
        NETWORK_PRINT_ERROR(" wpa_ctrl_open failed\n");
        return WIFI_FAIL;
    }

	len = sizeof(buf) - 1;
	ret = wpa_ctrl_request(pstWpaCtl, "BSS 0", strlen("BSS 0"), buf, &len, NULL);
	if (ret == -2) {
		NETWORK_PRINT_ERROR(" command SCAN_RESULTS timed out.\n");
		return WIFI_TIMEOUT;
	} else if (ret < 0) {
		NETWORK_PRINT_ERROR("SCAN_RESULTS command failed.\n");
		return WIFI_FAIL;
	}
    
    
    printf("#########################\n");
    printf("buf len:%d\n",len);
    printf("%s\n", buf);
    printf("#########################\n");

    printf("FFFF   XXXXXXXX %s,%d \n",__FUNCTION__,__LINE__);

    
	wpa_ctrl_close(pstWpaCtl);
    return WIFI_OK;
#endif
}
#endif
