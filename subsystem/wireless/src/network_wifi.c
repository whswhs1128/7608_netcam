#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
//#include "sdk/sdk_trace.h"
/*NETWORK_PRINT_INFO*/
//#include "common.h"

//#include "sdk_network.h"
#include "network_wifi.h"
#include "net_wifi_commom.h"
#include "net_wifi_smartlink.h"
#include "net_wifi_sta.h"
#include "net_wifi_host.h"
#include "zbar_qrcodelink.h"
#include "net_gateway.h"
#include "net_ping.h"

#define DETECT_TIME		60   /* detection time is second */
#define ETHERNET_LINK_STOP_SMART_LINK        1       /*ethernet linked, the smart link function is close*/


/*
 * global wireless support mode
 * default no support all of mode.
 */
static NETCAM_supportWorkModeT  gSupportMode = {
    .supportSTA = false,
    .supportAP  = false,
    0
};

/*global STA support func */
static NETCAM_STASupportFuncT	gStaFunc 	= {
	.supportWPS = false,
	.supportSL	= false,
	0
};

static int wifi_is_init = 0;/*WIFI LIB是否初始化（up网卡，加载驱动，识别型号）*/
static int wifi_cfg_mode = 0;/*与runNetworkCfg.wireless.mode 同步*/
static int wifi_is_ready = 0;/*0:off, 1:on 标志工作模式初始化完成，调用wifi_on wifi_off switch_mode wifi_disable设置*/
static pthread_rwlock_t network_lock;
static bool wifiIsOk;//enable wifi auto connect
static WIFI_HANDLE wifi_handle;
static WIFI_LINK_INFO_t curLinkInfo;
static NET_WIFI_WORK_StatusEnumT g_status = NET_WIFI_NONE;
static char *ap_driver = NULL;
static char *sta_driver = NULL;



static inline int _net_enter_lock(void)
{
    return pthread_rwlock_wrlock(&network_lock);
}

static int _net_leave_lock(void)
{
    return pthread_rwlock_unlock(&network_lock);
}
inline int _wifi_levelToQuality(int level)
{
	if(level < 0) {
		/* RSSI to quilty formula (100 + level)*2, (0 ~ 100)*/
		/* this formula is according to RSSI of Wi-Fi scanner. */
		int total = (105 + level)*4/3;
		if(total > 100)total = 100;
		if(total < 0)total = 0;
		return total;
	} else {
		return (level>100)?100:level;
	}
}
static int _net_wifi_check_ap_driver_configurefile(const char *ap_driver)
{
	char cmd[256] = {0};
	if(ap_driver == NULL)
		return -1;
    if(access(DRIVER_MT7601_AP_CFG, F_OK)){
        snprintf(cmd, sizeof(cmd),"cp %s/udhcpd.conf %s", DEFCFG_DIR, UDHCPD_CFG);
        new_system_call(cmd);
    }
    //check mt7601 ap driver.
	if(strncmp(ap_driver, DRIVER_MT7601_AP, sizeof(DRIVER_MT7601_AP)) == 0) {
		if(access(DRIVER_MT7601_AP_CFG, F_OK)){
			snprintf(cmd, sizeof(cmd),"cp %s/RT2870AP.dat %s", DEFCFG_DIR, DRIVER_MT7601_AP_CFG);
			new_system_call(cmd);
		}
	}
	return 0;
}


static int _net_wifi_probe(void)
{
	int ret = 0;
	char *wifi_device = NULL;
	NET_WIFI_DeviceNumT device_info;
	memset(&device_info, 0, sizeof(NET_WIFI_DeviceNumT));
	ret = net_wifi_getUsbDeviceID(&device_info,
		NET_WIFI_DEFAULT_BUS, NET_WIFI_DEFAULT_DEVICENUM);
	if(ret < 0) {
		printf("getUsbDeviceID is failed\n");
		return ret;
	}
	ret = net_wifi_getDeviceNameByDeviceNum(device_info, &wifi_device);
	if(ret < 0 || wifi_device == NULL) {
		printf("getDeviceNameByDeviceNum is failed\n");
		return ret;
	}
	//mt7601 support smartlink and wps.
	if(strcmp(wifi_device, NET_WIFI_DEVICENAME_MT7601) == 0) {
		ap_driver = DRIVER_MT7601_AP;
		sta_driver = DRIVER_MT7601_STA;
	} else if(strcmp(wifi_device, NET_WIFI_DEVICENAME_RTL8188) == 0) {
	//rtl8188eu no support smartlink.
		ap_driver = DRIVER_RTL8188;
		sta_driver = DRIVER_RTL8188;
	}else if(strcmp(wifi_device, NET_WIFI_DEVICENAME_RTL8188FTV) == 0) {
	//rtl8188eu no support smartlink.
		ap_driver = DRIVER_RTL8188FU;
		sta_driver = DRIVER_RTL8188FU;
	}else if(strcmp(wifi_device, NET_WIFI_DEVICENAME_SCI9083) == 0) {
	//rtl8188eu no support smartlink.
		ap_driver = DRIVER_SCI9083_AP;
		sta_driver = DRIVER_SCI9083_STA;
	}
    else
	{
        ap_driver = NULL;
        sta_driver = NULL;
    }
	return 0;
}



/*加载指定模式的驱动(加载前先卸载之前模式的驱动)
 *	function: switch wifi work status
 *	status: 	NET_WIFI_NONE	normal status
 *			NET_WIFI_STA	work station mode
 *			NET_WIFI_AP		AP mode
 */
static int _wifi_change_status_driver(WIFI_HANDLE wifi_handle, NET_WIFI_WORK_StatusEnumT status)
{
	int ap_status = 0, sta_status = 0;
	if(ap_driver == NULL || sta_driver == NULL)
		_net_wifi_probe();
	if(ap_driver == NULL || sta_driver == NULL)//if only ap or sta
		return -1;
	ap_status = net_wifi_check_driver_exist(ap_driver);
	sta_status = net_wifi_check_driver_exist(sta_driver);
	if(ap_status == 0 && sta_status == 1) g_status = NET_WIFI_STA;
	else if(ap_status == 1 && sta_status == 0) g_status = NET_WIFI_AP;
	else if((sta_status == 0) && (ap_status == 0))
		g_status = NET_WIFI_NONE;
    else
    {
        g_status = status;
        return 1; // ap and sta use same driver, it has load.
    }//is error status

	if(isNull(wifi_handle) && (g_status != NET_WIFI_NONE))//wifi QQQ
		return -1;

	if(status == g_status)
		return 0;

	//unload driver.
	if(g_status == NET_WIFI_STA) {
		net_wifi_cli_off(wifi_handle);
		if(strcmp(sta_driver, ap_driver) != 0) {
			net_wifi_down(wifi_handle);
			g_status = NET_WIFI_NONE;
            sleep(2);//add sleep, wait data is clean.
			net_wifi_unload_driver(sta_driver);
		}
	} else if(g_status == NET_WIFI_AP) {
		net_wifi_host_off(wifi_handle);
		if(strcmp(sta_driver, ap_driver) != 0) {
			net_wifi_down(wifi_handle);
			g_status = NET_WIFI_NONE;
            sleep(2);//ditto
			net_wifi_unload_driver(ap_driver);
		}
	}

	//load driver.
	if(status == NET_WIFI_AP){
		_net_wifi_check_ap_driver_configurefile(ap_driver);
		net_wifi_load_driver(ap_driver);
		sleep(2);//ditto

	} else if(status == NET_WIFI_STA) {
		net_wifi_load_driver(sta_driver);
		sleep(2);//ditto
	}
	g_status = status;
	return 0;
}

/*!
******************************************
** Description   @WIFI lib是否初始化
** Param[in]     @NULL
**
** Return        @已经初始化返回1，否则返回0
******************************************
*/
int network_wifi_is_init(void)
{
    return wifi_is_init;
}

void _net_wifi_set_ready(int status)
{
    wifi_is_ready = status;
}
int _net_wifi_is_ready()
{
    return wifi_is_ready;
}

/*!
******************************************
** Description   @与runNetworkCfg.wireless.mode 同步0/1
** Param[in]     @
**
** Return        @
******************************************
*/
int _net_wifi_get_mode(void)
{
    return wifi_cfg_mode;
}

/*!
******************************************
** Description   @与runNetworkCfg.wireless.mode 同步0/1
** Param[in]     @
**
** Return        @
******************************************
*/
void _net_wifi_set_mode(int mode)
{
    wifi_cfg_mode = mode;
}

static int wifi_hardware_detect(WIFI_MONITOR_CALLBACK callback_func)
{
    static int wifi_hardwre_ok = 0;
    char * wifi_name = NULL;

    if(wifi_hardwre_ok)/*检测OK后不再检测*/
        return 0;
    wifi_name = net_wifi_open();
    if(wifi_name != NULL) {
        WIFI_SCAN_LIST_t scanList[20];
        int count = 20;
        memset(scanList, 0,  sizeof(scanList));
        net_wifi_cli_disconnect(wifi_name);
        if(network_wifi_get_scan_list(scanList, &count) == 0)
        {
            wifi_hardwre_ok = 1;
        //netcam_audio_out(AUDIO_FILE_WIFI_NORMAL);
        	//callback_func(AUDIO_WIFI_HARDWARE_NORMAL);
        }
    }
    return 0;
}
static int _new_ping_gateway(void *arg)
{
    static int timeoutCnt = 0;
    char buff[256] = {'\0'};
    int ret = 0;
    int i;

    ret = net_get_gateway(buff);
    if(ret == 0)
    {
        NETWORK_PRINT_INFO("gateway--> %send\n",buff);
        ret = net_ping(buff,1000);//PING failed ,restart udhcpd
        if(ret != 0)
        {
            timeoutCnt++;
            //new_system_call("/usr/bin/killall udhcpc");
            NETWORK_PRINT_INFO("ping gateway failed,restart udhcpc,ret:%d, cnt:%d\n",ret, timeoutCnt);
            if (timeoutCnt > 3)
            {
                timeoutCnt = 0;
                sdk_net_set_dhcp(netcam_net_wifi_get_devname(),1, netcam_sys_get_name());
            }
        }

    }
    else
    {
        NETWORK_PRINT_ERROR("get gateway failed\n");
    }

    return 0;
}


int network_wifi_get_scan_list_ext(WIFI_SCAN_LIST_t *plist,int *number)
{
	char * wifi_name = NULL;
    int ret = 0;
    int i;
    WIFI_SCAN_LIST_t list[20];
    //int number = 0;
	if(network_wifi_is_init() == 0 || _net_wifi_is_ready()== 0)
	{
		NETWORK_PRINT_ERROR("Wifi is init:%d, wifi_is_ready:%d\n",network_wifi_is_init(), _net_wifi_is_ready());
		return -1;
	}

	wifi_name = net_wifi_open();
	if(wifi_name == NULL) {
        NETWORK_PRINT_ERROR("wifi name is NULL\n");
        return 0;
	}
    #if 0
	//wpa_supplicant test
	if(net_wifi_cli_ping(wifi_name) != 0) {
        network_wifi_off();
        network_wifi_on();
	}


	//XXXXX
    printf("XXXXXXXX %s,%d \n",__FUNCTION__,__LINE__);
#if 1
    net_wifi_cli_disconnect(wifi_name);
    if(ret !=0)
    {
        NETWORK_PRINT_ERROR("wifi disconnect failed!\n");
    }
#endif
#endif
    ret = net_wifi_cli_scan(wifi_name);
    if(ret !=0)
    {
        NETWORK_PRINT_ERROR("wifi scan failed!\n");
        goto exit;
    }
    memset(list,0,sizeof(list));
    ret = net_wifi_cli_scan_result(wifi_name,list, number);
    if(ret !=0)
    {
        NETWORK_PRINT_ERROR("wifi scan res failed!\n");
        goto exit;
    }

#if 1
    printf("ssid     mac      quality       security     channel\n");
    for(i=0;i<*number;i++)
    {
        printf("%s      %d      %d      %d      %s\n",
            list[i].mac,
            list[i].channel,
            list[i].quality,
            list[i].security,
            list[i].essid);
    }
#endif
    memcpy(plist,list,sizeof(list));
exit:
    return ret;
}

static int network_wifi_getstring_info(char *data,char *ssid,char *pw,char *enMode)
{
	int ret = 0;
	char *p = ":";
	char *q;
	int i = 0;
    printf("%s\n",data);
	q = strtok(data,":");
    if(q)
    {
        strcpy(ssid,q);
        printf("ssid >%s<\n",ssid);
    }

	q = strtok(NULL,":");
    if(q)
    {
        strcpy(pw,q);
        printf("pwd >%s<\n",pw);
    }

	q = strtok(NULL,":");
    if(q)
    {
        strcpy(enMode,q);
        printf("enMode >%s<\n",enMode);
    }



    #if 0
	if (strcmp(q,"S"))
		return -1;

	printf("%s\n",q);
	while((q = strtok(NULL,p)))
	{
		if (i ==0)
		{
			strcpy(ssid,q);
			printf("ssid >%s<\n",ssid);
		}
		else if (i == 2)
		{
			strcpy(pw,q);
			printf("passwd >%s<\n",pw);
		}
		else if (i == 3)
		{
			strcpy(enMode,q);
			printf("enMode >%c<\n",*enMode);
		}
		i++;
	}
    #endif

	return 0;
}



int network_wifi_getStringBySystem(const char *cmd, char *buff, int bufsize,int *retErr)
{
    FILE *fp = NULL;
    int ret=0;
    memset(buff, 0, sizeof(bufsize));
    if(NULL == buff)
    {
        printf("cmd buf is null\n");
        return -1;
    }

    fp = popen(cmd, "r");
    if(fp != NULL)
    {
        *retErr = 0;

    }
    else
    {
        *retErr = errno;
    }

    if (NULL == fp)
    {
        perror("error popen");
        return -1;
    }
    fread(buff,1,bufsize,fp);

    //printf("%d cmd exe display :: %s \n", __LINE__, buff);
    pclose(fp);
}

int net_wifi_scan_ap(const WIFI_HANDLE dev,WIFI_SCAN_LIST_t *plist, int *number)
{

	int ret = 0;
	char buff[18000] = {0};
	char cmd[128] = {0};
    char *pBuf = buff;
    char *psessBuf = buff;

    char *pBegin = pBuf;
    char *pEnd = pBuf;
    char *sessBegin = pBuf;
    char *sessEnd = pBuf;
    int len = 0;
    int sessLen = 0;
    int bufLen ;
    int count = 0;
    WIFI_SCAN_LIST_t list[20];
    char tmpBuf[128] = {0};
    char session[1000] = {0};
    int ECY_OFF,WPA,WPA2,AES,TKIP;

    memset(list,0,sizeof(list));

    NETWORK_PRINT_INFO("------network_wifi_getStringBySystem start ---\n");
	snprintf(cmd, sizeof(cmd),"%s %s scan",IWLIST_CMD, dev);
    network_wifi_getStringBySystem(cmd,buff,sizeof(buff),&ret);

	//ret = _wifi_cli_getshellresult(cmd, buff, sizeof(buff));
    //system(cmd);
	if(ret != 0)
    {
        NETWORK_PRINT_ERROR("network_wifi_getStringBySystem failed\n");
		return ret;
	}
    NETWORK_PRINT_INFO("------network_wifi_getStringBySystem ret:%d----\n",ret);

    bufLen = strlen(buff);
    printf("iwlist scan return bufLen:%d\n",bufLen);
    //printf("%s\n len:%d\n",buff,bufLen);

#if 1
    while('\0' != *pBuf)
    {
        sessBegin = strstr(psessBuf,"Cell");
        if(!sessBegin)
        {
            printf("no more cell,find ssid number:%d\n",count);
            break;
        }
        sessBegin +=strlen("Cell");
        psessBuf = sessBegin;

        sessEnd = strstr(psessBuf,"Cell");
        if(!sessEnd)
        {
            sessEnd = buff + bufLen -1;
            printf("last cell ,sessEnd:%d,find ssid number:%d\n",sessEnd,count);
        }
        psessBuf = sessEnd;

        sessLen = sessEnd - sessBegin;
        //session = {'\0'};
        memset(session,0,sizeof(session));
        strncpy(session,sessBegin,sessLen);
        //printf("---session count :%d----\n",count);
        //printf("%s\n",session);
        pBuf = session;
        /*1 essid*/

        pBegin = strstr(pBuf,"ESSID");
        if(!pBegin)
        {
            printf("session :%d not find ESSID",count);
            continue;
        }
        pBegin +=strlen("ESSID") + 2;

        pBuf = pBegin;
        pEnd = strstr(pBuf,"\"");
        if(!pEnd)
        {
            printf("session :%d not find ESSID",count);
            continue;
        }
        len = pEnd - pBegin;
        strncpy(list[count].essid,pBegin,len);
        printf("essid:%s\n",list[count].essid);

#if 0
        //2,channel
        pBegin = strstr(pBuf,"Signal level=")

        pBegin +=strlen("Signal level=");
        pBuf = pBegin;

        pEnd = strstr(pBuf,"\/");
        len = pEnd - pBegin;
        strncpy(tmpBuf,pBegin,len);
        list[count].quality = net_wifi_cli_strtoint(tmpBuf);
        printf("quality:%s\n",list[count].quality);
#endif

        //3,signal level

        pBuf = session;
        pBegin = strstr(pBuf,"Signal level=");
        if(!pBegin)
        {
            printf("session :%d not find Signal level",count);
            continue;
        }

        pBegin +=strlen("Signal level=");
        pBuf = pBegin;

        pEnd = strstr(pBuf,"\/");
        if(!pEnd)
        {
            printf("session :%d not find Signal level",count);
            continue;
        }
        len = pEnd - pBegin;
        //tmpBuf = {'\0'};
        memset(tmpBuf,0,sizeof(tmpBuf));
        strncpy(tmpBuf,pBegin,len);
        list[count].quality = net_wifi_cli_strtoint(tmpBuf);
        printf("tmpBuf:%s,len:%d,quality:%d\n",tmpBuf,len,list[count].quality);

        //4,encrypt
/*
1
        WPA Version +WPA2 Version =WPA
                     WPA2 Version =WPA2

2
        Group Cipher : CCMP  -->AES
        Group Cipher : TKIP  -->SKIP


*/
        ECY_OFF = WPA = WPA2 = AES = TKIP = 0;





        pBuf = session;
        pBegin = strstr(pBuf,"Encryption key:off");
        if(pBegin)
        {
            ECY_OFF = 1;
        }

        pBuf = session;
        pBegin = strstr(pBuf,"WPA Version");
        if(pBegin)
        {
            WPA = 1;
        }


        pBuf = session;
        pBegin = strstr(pBuf,"WPA2 Version");
        if(pBegin)
        {
            WPA2 = 1;
        }


        pBuf = session;
        pBegin = strstr(pBuf,"Group Cipher : CCMP");
        if(pBegin)
        {
            AES = 1;
        }

        pBuf = session;
        pBegin = strstr(pBuf,"Group Cipher : TKIP");
        if(pBegin)
        {
            TKIP = 1;
        }

/*
#define WPAECN_NONE     0
#define WEP_64_ASSCI    1
#define WEP_64_HEX      2
#define WEP_128_ASSCI   3
#define WEP_128_HEX     4
#define WPAPSK_TKIP     5
#define WPAPSK_AES      6
#define WPA2PSK_TKIP    7
#define WPA2PSK_AES     8

*/

        if(ECY_OFF)
        {
            list[count].security = WPAECN_NONE;
        }
        else if (WPA && TKIP)
        {
            list[count].security = WPAPSK_TKIP;
        }
        else if (WPA && AES)
        {
            list[count].security = WPAPSK_AES;
        }
        else if(WPA2 && TKIP)//WPA2PSK_TKIP
        {
            list[count].security = WPA2PSK_TKIP;
        }
        else if(WPA2 && AES)
        {

            list[count].security = WPA2PSK_AES;
        }
        else
        {
            list[count].security = WEP_64_ASSCI;
        }
        printf(" security :ECY_OFF:%d,WPA2:%d,WPA:%d,TKIP:%d,AES:%d\n",ECY_OFF,WPA2,WPA,TKIP,AES);


        printf("wpa securty :%d\n",list[count].security);
        count++;

    }

    *number = count;
#endif
    //strcpy(list[0].essid,"GokeIpc_1");
    //list[0].quality = 100;
    //list[0].security = 0;
    memcpy(plist,list,sizeof(list));
	return 0;
}


int network_wifi_get_scan_list_ap(WIFI_SCAN_LIST_t *plist,int *number)
{
	char * wifi_name = NULL;
    int ret = 0;
    int i;
    WIFI_SCAN_LIST_t list[20];
    //int number = 0;
	if(network_wifi_is_init() == 0 || _net_wifi_is_ready()== 0)
	{
		NETWORK_PRINT_ERROR("Wifi is init:%d, wifi_is_ready:%d\n",network_wifi_is_init(), _net_wifi_is_ready());
		return -1;
	}

	wifi_name = net_wifi_open();
    NETWORK_PRINT_INFO("wifi name :%s\n",wifi_name);
	if(wifi_name == NULL) {
        NETWORK_PRINT_ERROR("wifi name is NULL\n");
        return 0;
	}
    int retry = 10;
    do
    {
        net_wifi_scan_ap(wifi_name,list,number);
        printf(". ");
        sleep(1);
        retry --;
        if(retry <= 0)
        {
            break;
        }

    }while(*number <= 0);
    NETWORK_PRINT_INFO("ap scan success ,find wifi number:%d\n",*number);

#if 0
    ret = net_wifi_cli_scan(wifi_name);
    if(ret !=0)
    {
        NETWORK_PRINT_ERROR("wifi scan failed!\n");
        goto exit;
    }
    memset(list,0,sizeof(list));
    ret = net_wifi_cli_scan_result(wifi_name,list, number);
    if(ret !=0)
    {
        NETWORK_PRINT_ERROR("wifi scan res failed!\n");
        goto exit;
    }
    #endif
#if 0
    printf("ssid     mac      quality       security     channel\n");
    for(i=0;i<*number;i++)
    {
        printf("%s      %d      %d      %d      %s\n",
            list[i].mac,
            list[i].channel,
            list[i].quality,
            list[i].security,
            list[i].essid);
    }
#endif
    memcpy(plist,list,sizeof(list));
exit:
    return ret;
}

//must return 0
int network_wifi_monitor(GK_NET_WIRELESS_LINK_CFG *link_cfg, int num, WIFI_MONITOR_CALLBACK callback_func)
{
	int i,j;
	WIFI_SCAN_LIST_t scanList[20];
	int count = 20;
	int ret;
	WIFI_LINK_INFO_t linkInfo;
	char * wifi_name = NULL;

    if(!link_cfg){
        NETWORK_PRINT_ERROR("link cfg is NULL\n");
        return -1;
    }

	if(network_wifi_is_init() == 0 || _net_wifi_is_ready()== 0 ||
		wifiIsOk == 0 || !gSupportMode.supportSTA
		|| network_wifi_get_status() != NETCAM_WIFI_STA)//No delete wifiIsOk, Please ....
	{
        //printf(" %s, %d, init:%d, ready:%d, isok:%d, suprtSTA:%d, get_staus:%d\n",
        //        __func__, __LINE__, network_wifi_is_init(), _net_wifi_is_ready(),
        //        wifiIsOk,gSupportMode.supportSTA,network_wifi_get_status());
		return 0;
	}

    //WIFI 硬件检测
    wifi_hardware_detect(callback_func);
    if(!link_cfg[0].valid){
        callback_func(AUDIO_WIFI_SMART_LINK);
    }
    //stop smartlink when ehternet linked
    #if ETHERNET_LINK_STOP_SMART_LINK
    if(0 == sdk_net_get_detect("eth0") ) {


        if(net_wifi_smartlink_support() && net_wifi_smartlink_status()) {
				net_wifi_smartlink_stop();
        }
        //move to _qrcode_check()
        #if 0
		//增加二维码扫描连接wifi功能
        if(zbar_qrcode_link_support() && zbar_qrcode_link_status()) {
				zbar_qrcode_link_stop();
        }
        #endif
        return 0;

    }
    #endif

	wifi_name = net_wifi_open();
	if(wifi_name == NULL) {
        NETWORK_PRINT_ERROR("wifi name is NULL\n");
        return 0;
	}
	//wpa_supplicant test
	if(net_wifi_cli_ping(wifi_name) != 0) {
        network_wifi_off();
        network_wifi_on();
	}

    //if wifi is connected or status is error ,return ??;if not connected ,then continue to connect it
	ret = net_wifi_cli_status(wifi_name);
	if(ret == WIFI_STATUS_CONNECTED || ret == WIFI_STATUS_ERROR
		|| ret == WIFI_STATUS_UNKNOW) {
		NETWORK_PRINT_INFO("net_wifi_cli_status RET:%d \n",ret);

        if(net_wifi_smartlink_support() && net_wifi_smartlink_status()) {
				net_wifi_smartlink_stop();
        }

        //move to _qrcode_check()
        #if 0
		//增加二维码扫描连接wifi功能
        if(zbar_qrcode_link_support() && zbar_qrcode_link_status()) {
				zbar_qrcode_link_stop();
        }
        #endif

        _new_ping_gateway(NULL);
		return 0;
	}

    NETWORK_PRINT_INFO("xxx wifi status RET:%d \n",ret);
    //linkInfo[0].valid 必须第一个连接信息可用(只取第一个)
    if(link_cfg[0].valid){
		/* AP主动断开没办法重新连接 */
		memset(scanList, 0,  sizeof(scanList));
		net_wifi_cli_disconnect(wifi_name);
		ret = network_wifi_get_scan_list(scanList, &count);
		if(ret == 0){
			NETWORK_PRINT_INFO("auto scan list :%d\n",count);
			for(i = 0 ; i < WIFI_CONNECT_LIST_MAX_NUMBER; i++){
				if(link_cfg[i].valid != 0){

					for(j = 0; scanList[j].vaild != 0 && j < count ; j++){
						linkInfo.linkScurity = scanList[j].security;
						strcpy(linkInfo.linkEssid,scanList[j].essid);
						if(strcmp((char *)link_cfg[i].essid, (char *)linkInfo.linkEssid) == 0 ){
							linkInfo.isConnect = 1;
							strcpy(linkInfo.linkPsd,(char *)link_cfg[i].passd);
							NETWORK_PRINT_INFO("auto connetc wifi:ssid <%s>,psd:<%s>,enc:<%d>\n",
                                    linkInfo.linkEssid,linkInfo.linkPsd,linkInfo.linkScurity);
							//_net_hint_audio_off();//when auto connect, turn off voice. avoid repeat hint.
                            callback_func(AUDIO_NET_PLAY_OFF);
							if(network_wifi_connect(&linkInfo) == WIFI_CONNECT_OK){
                                callback_func(AUDIO_NET_PLAY_ON);
								break;
							}else{
                                callback_func(AUDIO_NET_PLAY_ON);
                                //callback_func(AUDIO_WIFI_LINK_FAIL);
                            }
						}
					}
					if(j != count)
						break;
				}
			}
		}
    }

	//if support smartlink, enter smartlink status.
	#if ETHERNET_LINK_STOP_SMART_LINK
    // ethernet linked, return
    if(0 == sdk_net_get_detect("eth0"))
    {
        return ret;
    }
    #endif

	if( net_wifi_smartlink_support()) {
		ret = net_wifi_cli_status(wifi_name);
		if(ret != WIFI_STATUS_CONNECTED) {
			if(!net_wifi_smartlink_status()) {
				net_wifi_smartlink_start();
			}
		}
	}


    //move to _qrcode_check()
    #if 0
	if( zbar_qrcode_link_support())
    {
		ret = net_wifi_cli_status(wifi_name);
		if(ret != WIFI_STATUS_CONNECTED) {
			if(!zbar_qrcode_link_status()) {
				zbar_qrcode_link_start();
			}
		}
	}
    #endif
	return ret;
}

int network_wifi_getConfigHostAP(NETCAM_WIFI_HostAPConfigT *apCfg)
{
    if(apCfg == NULL)
        return -1;
    if(!net_wifi_hostap_is_init()){
        NETWORK_PRINT_ERROR("wireless no support AP mode.");
        return -1;
    }
    memset(apCfg, 0, sizeof(NETCAM_WIFI_HostAPConfigT));
    strncpy(apCfg->ip, "192.168.252.1",sizeof(apCfg->ip));
    net_wifi_host_get_SSID(apCfg->ssid, sizeof(apCfg->ssid));
    apCfg->encrypt = net_wifi_host_get_EncrypMethod();
    if(apCfg->encrypt > 0)
        net_wifi_host_get_psk(apCfg->encrypt, apCfg->pwd, sizeof(apCfg->pwd));
    net_wifi_host_get_startPool(apCfg->startLoop, sizeof(apCfg->startLoop));
    net_wifi_host_get_endPool(apCfg->endLoop, sizeof(apCfg->endLoop));
    return 0;
}

int network_wifi_setConfigHostAP(NETCAM_WIFI_HostAPConfigT *apCfg)
{
    if(!net_wifi_hostap_is_init()){
        NETWORK_PRINT_ERROR("wireless no support AP mode.");
        return -1;
    }
    if(network_wifi_get_status() != NETCAM_WIFI_AP){
        NETWORK_PRINT_ERROR("wireless mode is not AP.");
        return -1;
    }
    return net_wifi_host_reconfigure(
        wifi_handle, apCfg->ssid, apCfg->pwd, apCfg->encrypt);
}
int network_wifi_get_scan_list(WIFI_SCAN_LIST_t *list, int *number)
{

	int ret = 0;
	SCAN_DataT scanList[20];
	int count = 20;
	int i;
	WIFI_ENC_MODE mode = WIFI_ENC_NONE;

	*number = 0;

	if(network_wifi_is_init() == 0 || _net_wifi_is_ready()== 0)
	{
		NETWORK_PRINT_ERROR("Wifi is init:%d, wifi_is_ready:%d\n",network_wifi_is_init(), _net_wifi_is_ready());
		return -1;
	}
	//if this is smartlink mode, exit smartlink mode.
	if(net_wifi_smartlink_support() && net_wifi_smartlink_status()) {
		net_wifi_smartlink_stop();
		sleep(2);
		net_wifi_cli_get_scanlist(wifi_handle, scanList, count);//保证wifi能搜索到
		sleep(2);
	}

	_net_enter_lock();
	memset(scanList, 0,  sizeof(scanList));
	ret = net_wifi_cli_get_scanlist(wifi_handle, scanList, count);
	if(ret == 0 )
	{
	    //printf("network_wifi scan wifi count:%d\n",count);
		for(i = 0; i < count && scanList[i].used != 0; i++)
		{
			list[i].vaild = 1;
			list[i].channel = scanList[i].channel;
			list[i].quality = _wifi_levelToQuality(scanList[i].level);

			if(scanList[i].mac.name[0] != '\0')
			{
				strcpy(list[i].mac,scanList[i].mac.name);
			}

			if(scanList[i].ssid.name[0] != '\0')
			{
				strcpy(list[i].essid,scanList[i].ssid.name);
			}

			if(scanList[i].encode == WIFI_ENCRYPT_OFF)
			{
				mode = WIFI_ENC_NONE;
			}else
			{
    			if(scanList[i].gen_ie[0].use_flag == 0)
    			{
					mode = WIFI_ENC_NONE;
    			}
				else
				{
					unsigned char chiper = WIFI_CHIP_UNKNOW;
					unsigned char ie = WIFI_IE_UNKNOW;
					chiper = scanList[i].gen_ie[0].group_chip;
					ie = scanList[i].gen_ie[0].ie;

					switch((chiper << 8)|ie)
					{
					    case (WIFI_CHIP_WEP_40 << 8)|WIFI_IE_WPA1:
							mode = WIFI_ENC_WEP_64_ASSII;
							break;
                        case (WIFI_CHIP_WEP_104 << 8)|WIFI_IE_WPA1:
							mode = WIFI_ENC_WEP_128_ASSII;
							break;
                        case (WIFI_CHIP_WEP_40 << 8)|WIFI_IE_WPA2:
							mode = WIFI_ENC_WEP_64_HEX;
							break;
                        case (WIFI_CHIP_WEP_104 << 8)|WIFI_IE_WPA2:
							mode = WIFI_ENC_WEP_128_HEX;
							break;
                        case (WIFI_CHIP_CCMP << 8)|WIFI_IE_WPA1:
                            mode = WIFI_ENC_WPAPSK_AES;
                            break;
                        case (WIFI_CHIP_TKIP << 8)|WIFI_IE_WPA1:
                            mode = WIFI_ENC_WPAPSK_TKIP;
                            break;
                        case (WIFI_CHIP_CCMP << 8)|WIFI_IE_WPA2:
 	                        mode = WIFI_ENC_WPA2PSK_AES;
                            break;
                        case (WIFI_CHIP_TKIP << 8)|WIFI_IE_WPA2:
                            mode = WIFI_ENC_WPA2PSK_TKIP;
                            break;
                        default:
                            mode = WIFI_ENC_NONE;
                            break;
					}
				}
			}
			list[i].security = mode;
		}
		*number = i;

	}


	_net_leave_lock();

	return ret;

}



int network_wifi_wps_connect(void)
{
	bool oldWifiIsOk = 0;
	char *ifname = NULL;

	if(network_wifi_is_init() == 0 || _net_wifi_is_ready()== 0)
	{
		NETWORK_PRINT_ERROR("Wifi is disable");
		return -1;
	}
	if(!gSupportMode.supportSTA){
		NETWORK_PRINT_ERROR("wireless no support STA mode.");
		return -1;
	}

	/* stop smart link*/
	if(net_wifi_smartlink_support() && net_wifi_smartlink_status()) {
		net_wifi_smartlink_stop();
	}
	ifname = net_wifi_open();
	if(ifname == NULL) {
		NETWORK_PRINT_ERROR("No wireless network card device.");
		return -1;
	}
	_net_enter_lock();
	// stop monitor run.
	oldWifiIsOk = wifiIsOk;
	wifiIsOk = 0;
	net_wifi_cli_wps_pbc(ifname, NULL, 0);
	// restore monitor run.
	wifiIsOk = oldWifiIsOk;
	_net_leave_lock();

	return 0;
}


int network_wifi_connect(WIFI_LINK_INFO_t *linkInfo)
{
	int ret = 0;
	if(network_wifi_is_init() == 0 || _net_wifi_is_ready()== 0)
	{
		NETWORK_PRINT_ERROR("Wifi is disable");
		return -1;
	}
	if(!gSupportMode.supportSTA){
		NETWORK_PRINT_ERROR("wireless no support STA mode.");
		return -1;
	}
	/* stop smart link*/
	if(net_wifi_smartlink_support() && net_wifi_smartlink_status()) {
		net_wifi_smartlink_stop();
	}

    //NEW ADD ,stop qrcode link while connecting!
    if(zbar_qrcode_link_support() && zbar_qrcode_link_status()) {
            zbar_qrcode_link_stop();
    }
	_net_enter_lock();
	wifiIsOk = 0;//stop monitor
	memcpy(&curLinkInfo,linkInfo,sizeof(WIFI_LINK_INFO_t));
	curLinkInfo.mode = 0;
	if(linkInfo->isConnect)
	{
        if((linkInfo->linkScurity != WIFI_ENC_NONE && strlen(linkInfo->linkPsd) >= 8) ||
            linkInfo->linkScurity == WIFI_ENC_NONE)
        {
            if(linkInfo->linkScurity == WIFI_ENC_NONE)
            {
                linkInfo->linkPsd[0] = 0; // set password is null;
            }


            ret = net_wifi_cli_connecting(wifi_handle, NULL,  linkInfo->linkEssid, linkInfo->linkPsd, 1);
    		curLinkInfo.linkStatus = ret;
		//no Operation "ret", thanks
        }
        else
        {
            curLinkInfo.linkStatus = WIFI_CONNECT_PASSWORD_ERROR;
            ret = WIFI_CONNECT_PASSWORD_ERROR;
        }
    	wifiIsOk = 1;//start monitor
	}
	else
	{
		ret = net_wifi_cli_disconnect(wifi_handle);
		if(ret == 0){
			ret = WIFI_DISCONNECT_OK;
			curLinkInfo.linkStatus = WIFI_DISCONNECT_OK;
		} else {
			ret = WIFI_DISCONNECT_FAILED;
			curLinkInfo.linkStatus = WIFI_DISCONNECT_FAILED;
		}
		wifiIsOk = 0;//stop monitor
	}
	linkInfo->linkStatus = curLinkInfo.linkStatus;
	_net_leave_lock();
	return ret;
}

static int64_t _gettime_ms(void)
{
	struct timespec tp;
	if (clock_gettime(CLOCK_MONOTONIC, &tp) < 0)
		return -1;
	return (int64_t)tp.tv_sec * 1000 + (int64_t)tp.tv_nsec / 1000000;
}

int network_wifi_link_test(WIFI_LINK_INFO_t *linkInfo)
{
	int ret = WIFI_OK;

	if(network_wifi_is_init() == 0 || _net_wifi_is_ready()== 0)
	{
		NETWORK_PRINT_ERROR("Wifi is disable");
		return WIFI_FAIL;
	}
	if(!gSupportMode.supportSTA){
		NETWORK_PRINT_ERROR("wireless no support STA mode.");
		return WIFI_FAIL;
	}

	/* stop smart link*/
	if(net_wifi_smartlink_support() && net_wifi_smartlink_status()) {
		net_wifi_smartlink_stop();
	}


    //NEW ADD ,stop qrcode link while connecting!
    if(zbar_qrcode_link_support() && zbar_qrcode_link_status()) {
            zbar_qrcode_link_stop();
    }
	_net_enter_lock();

    NETWORK_PRINT_INFO("network_wifi_link_test: stop the old connect.");
	wifiIsOk = 0;//stop monitor
	if(curLinkInfo.isConnect)//stop the old connect.
	{
		ret = net_wifi_cli_disconnect(wifi_handle);
		if(ret == 0){
			ret = WIFI_DISCONNECT_OK;
			curLinkInfo.linkStatus = WIFI_DISCONNECT_OK;
		} else {
			ret = WIFI_DISCONNECT_FAILED;
			curLinkInfo.linkStatus = WIFI_DISCONNECT_FAILED;
		}
    }

    // connect to the test wifi.
    NETWORK_PRINT_INFO("network_wifi_link_test: connect to the test wifi.");
    if((linkInfo->linkScurity != WIFI_ENC_NONE && strlen(linkInfo->linkPsd) >= 8) ||
        linkInfo->linkScurity == WIFI_ENC_NONE)
    {
        if(linkInfo->linkScurity == WIFI_ENC_NONE)
        {
            linkInfo->linkPsd[0] = 0; // set password is null;
        }

        ret = net_wifi_cli_connecting(wifi_handle, NULL,  linkInfo->linkEssid, linkInfo->linkPsd, 0);
    }

    ret = net_wifi_cli_status(wifi_handle);
    #if 1
    if (ret == WIFI_STATUS_CONNECTED) {
        NETWORK_PRINT_INFO("network_wifi_link_test: test wifi connect ok.");
        ret = WIFI_OK;
        // disconnect the test wifi.
		net_wifi_cli_disconnect(wifi_handle);
    } else {
        NETWORK_PRINT_INFO("network_wifi_link_test: test wifi connect faled.");
        ret = WIFI_FAIL;
    }
	_net_leave_lock();
	#else
    // wait the connect ok in 5s.
	int64_t waittime = _gettime_ms();
    while (ret != WIFI_STATUS_CONNECTED && (_gettime_ms()-waittime) <= 5000)
    {
    	_net_enter_lock();
        ret = net_wifi_cli_status(wifi_handle);
    	_net_leave_lock();
        usleep(1000000);//1s
        NETWORK_PRINT_INFO("network_wifi_link_test: wait wifi connect ok.");
    }

    if (ret == WIFI_STATUS_CONNECTED) {
        NETWORK_PRINT_INFO("network_wifi_link_test: test wifi connect ok.");
        ret = WIFI_OK;
        // disconnect the test wifi.
	    _net_enter_lock();
		net_wifi_cli_disconnect(wifi_handle);
	    _net_leave_lock();
    } else {
        NETWORK_PRINT_INFO("network_wifi_link_test: test wifi connect faled.");
        ret = WIFI_FAIL;
    }
    #endif
    return ret;
}

NET_WIFI_WORK_StatusEnumT network_wifi_get_status(void)
{
	return g_status;
}

void network_wifi_set_status(NET_WIFI_WORK_StatusEnumT status)
{
	g_status = status;
}

int network_wifi_on(void)
{
	int ret = 0;
	NETCAM_WIFI_WorkModeEnumT mode;

	if(!network_wifi_is_init()){
        NETWORK_PRINT_ERROR("network_wifi_is_init:%d\n", network_wifi_is_init());
		return -1;
    }
	//if(_net_wifi_is_ready())
	//	return 0;

	/* get wifi dev name, and check driver. */
	net_wifi_close(NULL);
	wifi_handle = net_wifi_open();
	if(wifi_handle == NULL) {
		NETWORK_PRINT_ERROR("net_wifi_open fail\n");
		return -1;
	}
	/* check driver whether is STA driver, if not.
	  * switch to STA driver and repare get wifi name.
	  */
	mode = network_wifi_get_status();
	/* check wifi status */
	if(mode == NETCAM_WIFI_STA) {
		_net_enter_lock();
		ret = net_wifi_cli_on(wifi_handle, NULL, NULL);
		if(ret < 0){
			NETWORK_PRINT_ERROR("net_wifi_cli_on fail\n");
	        _net_leave_lock();
			return -1;
		}
		else
		{
	    	NETWORK_PRINT_INFO("wifi on is OK\n");
		}
	    wifiIsOk = 1;
		_net_leave_lock();
	} else if(mode == NETCAM_WIFI_AP) {
		_net_enter_lock();
		ret = net_wifi_host_on(wifi_handle, NULL, NULL);
		if(ret < 0){
			NETWORK_PRINT_ERROR("net_wifi_host_on fail\n");
	        _net_leave_lock();
			return -1;
		}
		else
		{
	    	NETWORK_PRINT_INFO("wifi on is OK\n");
		}
		_net_leave_lock();
	} else {
		return -1;
	}
    _net_enter_lock();
    _net_wifi_set_ready(1);
    _net_leave_lock();
    return 0;
}

int network_wifi_off(void)
{
	int ret = 0;
	NETCAM_WIFI_WorkModeEnumT mode;

	if(!network_wifi_is_init())
		return -1;
	if(!_net_wifi_is_ready())
		return 0;

	mode = network_wifi_get_status();
	/* check wifi status */
	if(mode == NETCAM_WIFI_STA) {
		/* check wifi status */
	    _net_enter_lock();
		if(net_wifi_smartlink_support() && net_wifi_smartlink_status()) {
			net_wifi_smartlink_stop();
			NETWORK_PRINT_ERROR("net_wifi_smartlink_stop fail\n");
		}
        //NEW ADD ,stop qrcode link while connecting!
        if(zbar_qrcode_link_support() && zbar_qrcode_link_status()) {
                zbar_qrcode_link_stop();
        }



		ret = net_wifi_cli_off(wifi_handle);
		if(ret < 0){
			NETWORK_PRINT_ERROR("net_wifi_cli_off fail\n");
	        _net_leave_lock();
			return -1;
		}
		else
		{
	    	NETWORK_PRINT_INFO("wifi off is OK\n");
		}
		net_wifi_close(wifi_handle);
		_net_leave_lock();
	} else if(mode == NETCAM_WIFI_AP) {
		_net_enter_lock();
		ret = net_wifi_host_off(wifi_handle);
		if(ret < 0){
			NETWORK_PRINT_ERROR("net_wifi_host_off fail\n");
	        _net_leave_lock();
			return -1;
		}
		else
		{
	    	NETWORK_PRINT_INFO("wifi off is OK\n");
		}
		_net_leave_lock();
	} else {
		return -1;
	}

    _net_enter_lock();
    wifiIsOk = 0;
    _net_wifi_set_ready(0);
    _net_leave_lock();
	return 0;
}
int network_wifi_switch_workmode(NETCAM_WIFI_WorkModeEnumT mode)
{
	//solve issue at no wifi device.
	if(0 == network_wifi_is_init())
		return -1;
	if((mode == NETCAM_WIFI_STA) && !gSupportMode.supportSTA){
		NETWORK_PRINT_ERROR("wireless no support STA mode.");
		return -1;
	} else if((mode == NETCAM_WIFI_AP) && !net_wifi_hostap_is_init()){
		NETWORK_PRINT_ERROR("wireless no support AP mode.");
		return -1;
	}

	if(network_wifi_get_status() == mode)
		return 0;
	_net_enter_lock();
	wifiIsOk = 0;
    _net_wifi_set_ready(0);
	if(gSupportMode.supportSTA && net_wifi_smartlink_support()){
		if(network_wifi_get_status() == NETCAM_WIFI_STA
			&& net_wifi_smartlink_status() != 0) {
			net_wifi_smartlink_stop();
		}
		//wait smartlink all of exit.
		while(net_wifi_smartlink_status() != 0){usleep(100000);}
	}

#if 0
    //支持sta模式，且支持二维码扫描连接wifi的情况下，则在sta切换工作模式到ap的时候
    //需要先关掉sta模式下的二维码扫描，(smartlink一样要关闭)
	if(gSupportMode.supportSTA && zbar_qrcode_link_support()){
		if(network_wifi_get_status() == NETCAM_WIFI_STA
			&& zbar_qrcode_link_status() != 0) {
			zbar_qrcode_link_stop();
		}
		//wait smartlink all of exit.
		while(zbar_qrcode_link_status() != 0){usleep(100000);}
	}
#endif
	int ret = _wifi_change_status_driver(wifi_handle, mode);
	if(ret < 0) {
		NETWORK_PRINT_ERROR("_wifi_change_status_driver failed");
		_net_leave_lock();
		return ret;
	}
    _net_wifi_set_mode(mode);
	_net_leave_lock();
    network_wifi_on();
	return 0;
}


int network_wifi_get_connect_info(WIFI_LINK_INFO_t *linkInfo)
{
	int ret = 0;
	if(network_wifi_is_init() == 0 || _net_wifi_is_ready() == 0)
	{
		NETWORK_PRINT_ERROR("Wifi is disable");
		return -1;
	}
	if(!gSupportMode.supportSTA){
		NETWORK_PRINT_ERROR("wireless no support STA mode.");
		return -1;
	}
	_net_enter_lock();
	//if(curLinkInfo.isConnect)
	{
		ret = net_wifi_cli_status(wifi_handle);
		switch(ret )
		{
			case WIFI_STATUS_CONNECTED:
				curLinkInfo.linkStatus = WIFI_CONNECT_OK;
				break;
			default:
				curLinkInfo.linkStatus = WIFI_CONNECT_TIMEOUT;
				break;
		}
	}

	memcpy(linkInfo,&curLinkInfo,sizeof(WIFI_LINK_INFO_t));
	_net_leave_lock();
	return 0;

}


int network_wifi_probe(void)
{
	int ret = 0;
	char *wifi_device = NULL;
	NET_WIFI_DeviceNumT device_info;
	memset(&device_info, 0, sizeof(NET_WIFI_DeviceNumT));
	ret = net_wifi_getUsbDeviceID(&device_info,
		NET_WIFI_DEFAULT_BUS, NET_WIFI_DEFAULT_DEVICENUM);
	if(ret < 0) {
		NETWORK_PRINT_ERROR("getUsbDeviceID is failed, no wifi usb device.");
		return -2;
	}
	ret = net_wifi_getDeviceNameByDeviceNum(device_info, &wifi_device);
	if(ret < 0 || wifi_device == NULL) {
		NETWORK_PRINT_ERROR("getDeviceNameByDeviceNum is failed");
		return -1;
	}

    printf("wifi_device:%s\n", wifi_device);
	if(strcmp(wifi_device, NET_WIFI_DEVICENAME_MT7601) == 0) {
		//mt7601 support AP& STA:
		gSupportMode.supportAP 	= true;//support AP
        net_wifi_hostap_init();
		gSupportMode.supportSTA = true;//support STA
		gStaFunc.supportWPS 	= true;//support WPS connect at STA work mode.
        net_wifi_smartlink_init();//support SmartLink connect at STA work mode.
        //zbar_qrcode_link_init();//support qrcode link connect at STA work mode.
	} else if(strcmp(wifi_device, NET_WIFI_DEVICENAME_RTL8188) == 0) {
		//rtl8188 support AP& STA:
		gSupportMode.supportAP 	= true;//support AP
        net_wifi_hostap_init();
		gSupportMode.supportSTA = true;//support STA
		gStaFunc.supportWPS 	= true;//support WPS connect at STA work mode.
        //net_wifi_smartlink_init();//not support SmartLink connect at STA work mode.
        //zbar_qrcode_link_init();//support qrcode link connect at STA work mode.
	}else if(strcmp(wifi_device, NET_WIFI_DEVICENAME_RTL8188FTV) == 0) {
		//rtl8188 support AP& STA:
		gSupportMode.supportAP 	= true;//support AP
        net_wifi_hostap_init();
		gSupportMode.supportSTA = true;//support STA
		gStaFunc.supportWPS 	= true;//support WPS connect at STA work mode.
		printf("---->gSupportMode.supportSTA:%d\n", !gSupportMode.supportSTA);
        //net_wifi_smartlink_init();//not support SmartLink connect at STA work mode.
        //zbar_qrcode_link_init();//support qrcode link connect at STA work mode.
	}else if(strcmp(wifi_device, NET_WIFI_DEVICENAME_SCI9083) == 0) {
		//rtl8188 support AP& STA:
		gSupportMode.supportAP 	= 0;//support AP
        net_wifi_hostap_init();
		gSupportMode.supportSTA = true;//support STA
		gStaFunc.supportWPS 	= 0;//support WPS connect at STA work mode.
        //net_wifi_smartlink_init();//not support SmartLink connect at STA work mode.

        //zbar_qrcode_link_init();//support qrcode link connect at STA work mode.
	}
	return 0;
}

/*!
******************************************
** Description   @只在初始化wifi时执行;up 网卡
** Param[in]     @
**
** Return        @
******************************************
*/
int network_wifi_enable()
{
    /* get wifi dev name, and check driver. */
    wifi_handle = net_wifi_open();
    /* check driver whether is STA driver, if not.
     * switch to STA driver and repare get wifi name.
     */

    net_wifi_up(wifi_handle);
    printf("wifi handle:%s\n", wifi_handle);
    _net_enter_lock();
    _net_leave_lock();

    if(_net_wifi_get_mode()== 0) {
        NETWORK_PRINT_INFO("Wifi is AP mode");
        network_wifi_switch_workmode(NETCAM_WIFI_AP);
    } else if(_net_wifi_get_mode()== 1){
        NETWORK_PRINT_INFO("Wifi is STA mode");
        network_wifi_switch_workmode(NETCAM_WIFI_STA);
    }
    return 0;
}
static int network_wifi_disable()
{
    network_wifi_off();
    //net_wifi_down(wifi_handle);
    return 0;
}


int network_wifi_init(int mode)
{
    if(network_wifi_is_init())
    {
        return 0;
    }		//xqq
	memset(&curLinkInfo, 0, sizeof(curLinkInfo));
	curLinkInfo.linkStatus = WIFI_DISCONNECT_OK;
    pthread_rwlock_init(&network_lock, NULL);

    _net_wifi_set_mode(mode);
    int ret = network_wifi_probe();
    if(ret >= 0) {
        if(_net_wifi_get_mode() == 0) // AP MODE
            _wifi_change_status_driver(NULL, NET_WIFI_AP);
        else
            _wifi_change_status_driver(NULL, NET_WIFI_STA);

        wifi_is_init = 1;
        network_wifi_enable();
        return 0;
    }
    return -1;
}
int network_wifi_deinit(void)
{
    if(network_wifi_is_init()){
        wifi_is_init = 0;
        net_wifi_smartlink_deinit();
        zbar_qrcode_link_deinit();
        network_wifi_disable();
        new_system_call("killall wpa_supplicant");
    }
    return 0;
}

void netcam_wifi_app_cb(char  *ssid, char *password, WIFI_ENC_MODE encrypt_type)
{
	WIFI_LINK_INFO_t linkInfo;
	int ret = 0;
	printf(">%s<,>%s<\n",ssid,password);
	memset(&linkInfo,0,sizeof(WIFI_LINK_INFO_t));
	linkInfo.isConnect = 1;
	linkInfo.linkScurity = encrypt_type;
	strcpy(linkInfo.linkEssid,ssid);
	if(password)
	{
		strcpy(linkInfo.linkPsd, password);
        //netcam_audio_hint(9);	xqq
	}
	ret = netcam_net_wifi_set_connect_info(&linkInfo);
	if( ret == 0 &&  linkInfo.linkStatus == WIFI_CONNECT_OK )
	{
		char *wifiNmae = netcam_net_wifi_get_devname();
		if(wifiNmae != NULL)
		{
			netcam_net_reset_net_ip(netcam_net_wifi_get_devname(),NULL);
		}
	}
}


void netcam_wifi_set_callback_ext(void(*wifi_info)(char*,char*,WIFI_ENC_MODE))
{
	int ret = 0;
	int encrypt = 0;
	char *ssid[32] = {0};
	char *psw[32]  = {0};
    char enMode[1] = {0};
	//ret = sdk_vin_scanner(ssid,psw);
	ret = ret = zbar_vin_scanner(ssid,psw,enMode);
	if (ret == 0 && ssid != NULL)
	{
		if (strlen(psw) != 0)
			encrypt = WIFI_ENC_WPA2PSK_TKIP;
		wifi_info(ssid,psw,encrypt);
	}
}

