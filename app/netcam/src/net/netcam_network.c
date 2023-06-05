#include "sdk_debug.h"
#include "sdk_api.h"
#include "cfg_com.h"
#include "media_fifo.h"
#include "common.h"
#include "sdk_network.h"
#include "net_dhcp.h"
#include "netcam_api.h"
#include "net_wifi_sta.h"
#include "net_wifi_commom.h"
#include "net_utils.h"
#include "netcam_api.h"
#include "net_wifi_host.h"
#include "net_arping.h"
#ifdef MODULE_SUPPORT_ZK_WAPI
#include "httpClient.h"
#endif
/*wifi lib*/
#include "network_wifi.h"

#define NET_ENABLE_CONTROL   1
#ifdef MODULE_SUPPORT_WORKQUEUE
#include "work_queue.h"
#endif

#define LINK_UP     0
#define LINK_DOWN   1

static int is_set_wire = 0;
static int is_set_wireless = 0;
static pthread_rwlock_t network_lock;
static pthread_mutex_t network_autotracking_lock = PTHREAD_MUTEX_INITIALIZER;
static bool hint_audio = 1;
extern char * netcam_net_wifi_get_devname(void);
static int _netcam_net_wifi_enable(void);
static unsigned char icute_random_number = 0;

static inline void _net_hint_audio_on(void)
{
	hint_audio = 1;
}

static inline void _net_hint_audio_off(void)
{
	hint_audio = 0;
}

static inline int _net_enter_lock(void)
{
    return pthread_rwlock_wrlock(&network_lock);
}

static int _net_leave_lock(void)
{
    return pthread_rwlock_unlock(&network_lock);
}

/*!
******************************************
** Description   @设置WIFI配置文件的开关，由netcam_net_wifi_on,和netcam_net_wifi_off调用
** Param[in]     @flag: off:0，on:1
**
** Return        @NULL
******************************************
*/
static void _net_set_wifiCfg_switch(int flag)
{
    runNetworkCfg.wifi.enable = flag;
	netcam_net_cfg_save();

}
static int _net_wifi_linkinfo_isempty(void)
{
	return (runNetworkCfg.wifilink[0].valid == 0);
}
static int _net_set_wificfg_mode(int mode)
{
    runNetworkCfg.wireless.mode = mode;
    return 0;
}
static void _net_add_wifilink_info( WIFI_LINK_INFO_t *info)
{
	int i;
	int changed = 0;
	for(i = 0; i < 20;i++)
	{
		if(runNetworkCfg.wifilink[i].valid != 0)
		{
			if(strcmp((char *)runNetworkCfg.wifilink[i].essid,(char *)info->linkEssid) == 0)
			{
				if(strcmp((char *)runNetworkCfg.wifilink[i].passd,(char *)info->linkPsd) != 0)
				{
					strcpy((char *)runNetworkCfg.wifilink[i].passd,(char *)info->linkPsd);
				}
				changed = 1;
				break;
			}
		}
	}
	if(i == 20)
	{
		for(i = 0; i < 20;i++)
		{
			if(runNetworkCfg.wifilink[i].valid == 0)
			{
				strcpy((char *)runNetworkCfg.wifilink[i].passd,(char *)info->linkPsd);
				strcpy((char *)runNetworkCfg.wifilink[i].essid,(char *)info->linkEssid);
				runNetworkCfg.wifilink[i].encMode = info->linkScurity;
				runNetworkCfg.wifilink[i].valid = 1;
				changed = 1;
				break;
			}
		}

		if(i == 20)
		{
			memmove(&runNetworkCfg.wifilink[0],&runNetworkCfg.wifilink[1],sizeof(runNetworkCfg.wifilink)-sizeof(runNetworkCfg.wifilink[0])) ;
			strcpy((char *)runNetworkCfg.wifilink[19].passd,(char *)info->linkPsd);
			strcpy((char *)runNetworkCfg.wifilink[19].essid,(char *)info->linkEssid);
			runNetworkCfg.wifilink[19].encMode = info->linkScurity;
			runNetworkCfg.wifilink[19].valid = 1;
			changed = 1;
			i = 19;

		}
	}

	if(changed == 1)
	{
		GK_NET_WIRELESS_LINK_CFG curLink ;
		memcpy(&curLink,&runNetworkCfg.wifilink[i],sizeof(GK_NET_WIRELESS_LINK_CFG));
		if(i != 0)
		{
			memmove(&runNetworkCfg.wifilink[1],&runNetworkCfg.wifilink[0],sizeof(GK_NET_WIRELESS_LINK_CFG) * i);
			memcpy(&runNetworkCfg.wifilink[0],&curLink,sizeof(GK_NET_WIRELESS_LINK_CFG));
		}

		PRINT_INFO("netcam_cfg_network_wifi_save!!");
        NetworkCfgSave();
	}
	ipc_search_save_to_resave_wifi((char *)info->linkEssid,(char *)info->linkPsd,info->linkScurity);
}
/*!
******************************************
** Description   @只在初始化wifi时执行
** Param[in]     @
**
** Return        @
******************************************
*/
static int _netcam_net_wifi_enable()
{
    if(0 == network_wifi_enable())
        return 0;

    return -1;
}

static void _monitor_call_back(int status)
{
#define AUDIO_WIFI_SMART_LINK_PLAY_ONCE 1
    switch(status)
    {
        case AUDIO_NET_PLAY_OFF:
            _net_hint_audio_off();
            break;
        case AUDIO_NET_PLAY_ON:
            _net_hint_audio_on();
            break;
        case AUDIO_WIFI_HARDWARE_NORMAL:
            //netcam_audio_out(AUDIO_FILE_WIFI_NORMAL);		xqq
            break;
        case AUDIO_WIFI_SMART_LINK:{
            static int only_play_once = 0;
            //if(!only_play_once)
                //netcam_audio_out(AUDIO_FILE_WIFI_SMART_LINK);		xqq
            if(AUDIO_WIFI_SMART_LINK_PLAY_ONCE)
                only_play_once = 1;
            break;
        }
        case AUDIO_WIFI_LINK_FAIL:
            //netcam_audio_hint(SYSTEM_AUDIO_HINT_LINK_FAIL);	xqq
            break;
        default:
            break;
    }
}

static int _wifi_monitor(void *arg)
{
    if(WIFI_STATUS_CONNECTED == network_wifi_monitor(runNetworkCfg.wifilink, WIFI_CONNECT_LIST_MAX_NUMBER, _monitor_call_back))
        ;//netcam_net_reset_net_ip(net_wifi_open(),NULL);
    return 0;
}


static int _set_wire_param(void* arg)
{
	int* set_wire = (int*)arg;
	char buffer[128];
	ST_SDK_NETWORK_ATTR netAttr;

	memset(&netAttr, 0, sizeof(ST_SDK_NETWORK_ATTR));
	strncpy(netAttr.name, "eth0", sizeof(netAttr.name));

	if(netcam_net_wifi_getWorkMode() == NETCAM_WIFI_STA)
	{
		char *wifi_name = netcam_net_wifi_get_devname();
		if(wifi_name)
		{
			memset(buffer,0,sizeof(buffer));
			sprintf(buffer,"ifconfig %s 0.0.0.0",wifi_name);
			new_system_call(buffer);
			memset(buffer, 0, sizeof(buffer));
			sprintf(buffer,"route  del default dev %s",wifi_name);
			new_system_call(buffer);
			PRINT_INFO("WIFI use default 0.0.0.0 \n");
		}
	}
    else if (netcam_net_wifi_getWorkMode() == NETCAM_WIFI_AP)
    {
		char *wifi_name = netcam_net_wifi_get_devname();
		if(wifi_name)
		{
            new_system_call("killall hostapd");
            new_system_call("killall udhcpd");
			memset(buffer,0,sizeof(buffer));
			sprintf(buffer,"route del 255.255.255.255");
			new_system_call(buffer);
			PRINT_INFO("delete wifi 255.255.255.255 route\n");
		}
    }


    #ifdef AP_STA_SUPPORT

    printf("AP_STA_SUPPORT down wifi wlan0\n");
	if(netcam_net_wifi_getWorkMode() == NETCAM_WIFI_AP)
	{
	    printf("NETCAM_WIFI_AP mode\n");
		char *wifi_name = netcam_net_wifi_get_devname();
		if(wifi_name)
		{
			memset(buffer,0,sizeof(buffer));
			sprintf(buffer,"ifconfig %s 0.0.0.0",wifi_name);
			new_system_call(buffer);
            #if 0
			memset(buffer, 0, sizeof(buffer));
			sprintf(buffer,"route  del default dev %s",wifi_name);
			new_system_call(buffer);
            #endif
			PRINT_INFO("WIFI use default 0.0.0.0 \n");
		}
	}

    #endif
	netcam_net_getcfg(&netAttr);
	netcam_net_set(&netAttr);
	new_system_call("ifconfig eth0:1 192.168.189.189");

	netcam_net_get(&netAttr);
    #if 0
	if (hint_audio)
		netcam_audio_hint(SYSTEM_AUDIO_HINT_USE_WIRE);
	netcam_audio_ip(netAttr.ip);
    #endif
	*set_wire = 1;

#ifdef MODULE_SUPPORT_GOOLINK
		int ret = glnk_set_ifname(netAttr.name);
		PRINT_INFO("goolink set wire ifname:%s, ret:%d\n", netAttr.name, ret);
#endif

    //取消静音

    //netcam_audio_mute(0);	xqq
	if(runAudioCfg.rebootMute !=0 )
	{
    	runAudioCfg.rebootMute = 0;
    	AudioCfgSave();
	}
	return 0;
}

static int _set_wireless_param(void* arg)
{
	int* set_wireless = (int*)arg;
	ST_SDK_NETWORK_ATTR netAttr;
	WIFI_LINK_INFO_t linkInfo;
	NETCAM_WIFI_WorkModeEnumT mode;
    int isInit = 0;

    if(netcam_net_wifi_get_devname() == NULL)
    {
        return 0;
    }

	if(netcam_net_wifi_isOn() != 1)
	{
		PRINT_ERR("wifi is not start\n");
		return -1;
	}

    isInit = netcam_net_wifi_start(runNetworkCfg.wireless.mode);

	memset(&linkInfo, 0, sizeof(WIFI_LINK_INFO_t));
	memset(&netAttr, 0, sizeof(ST_SDK_NETWORK_ATTR));
	strcpy((char *)&netAttr.name,netcam_net_wifi_get_devname());

	mode = netcam_net_wifi_getWorkMode();
	if(mode == NETCAM_WIFI_AP ||
		(netcam_net_wifi_get_connect_info(&linkInfo) == 0 &&
		linkInfo.linkStatus == WIFI_CONNECT_OK))
	{
		if(mode == NETCAM_WIFI_STA)
		{
			netcam_net_get(&netAttr);
			netcam_net_set(&netAttr);
			PRINT_INFO("Reset wifi IP address\n");
            //if (hint_audio)
			    //netcam_audio_hint(SYSTEM_AUDIO_HINT_USE_WIRELESS);	xqq
		}

		if(mode == NETCAM_WIFI_AP)
		{
            if (!(runNetworkCfg.wireless.mode == 0 && isInit == 1))
            {
                network_wifi_set_status(NETCAM_WIFI_STA);
                network_wifi_switch_workmode(NETCAM_WIFI_AP);
            }
			char cmd[128]={0};
            sprintf(cmd,"route add -host 255.255.255.255 dev %s",netcam_net_wifi_get_devname());
            new_system_call(cmd);
            //if (hint_audio)
                //netcam_audio_out(AUDIO_FILE_WIFI_SMART_LINK);		xqq
		}

        #ifdef AP_STA_SUPPORT
		if(mode == NETCAM_WIFI_AP)
		{
			//netcam_net_get(&netAttr);
			//netcam_net_set(&netAttr);
			char cmd[128]={0};

            memset(cmd,0,sizeof(cmd));
            sprintf(cmd,"ifconfig wlan0 %s up",UDHCPD_IP);
            //new_system_call("ifconfig wlan0 192.168.1.88 up");
            printf("xxxx  %s\n",cmd);
            new_system_call(cmd);

            memset(cmd,0,sizeof(cmd));
            sprintf(cmd,"route add -host 255.255.255.255 dev %s",netcam_net_wifi_get_devname());
            printf("xxxx 1 %s\n",cmd);
            new_system_call(cmd);
			PRINT_INFO("Reset wifi IP address\n");
		}

        #endif
		new_system_call("ifconfig eth0 0.0.0.0");
        new_system_call("ifconfig eth0:1 0.0.0.0");
		new_system_call("route del default dev eth0");
        PRINT_INFO("ETH0 use default 0.0.0.0 \n");
		netcam_net_get(&netAttr);
        #if 0
		if (hint_audio)
			netcam_audio_hint(SYSTEM_AUDIO_HINT_USE_WIRELESS);
		netcam_audio_ip(netAttr.ip);
        #endif
        *set_wireless = 1;

#ifdef MODULE_SUPPORT_GOOLINK
		int ret = glnk_set_ifname(netAttr.name);
		PRINT_INFO("goolink set wifi ifname:%s, ret:%d\n", netAttr.name, ret);
#endif
        //取消静音
        //netcam_audio_mute(0);		xqq
		if(runAudioCfg.rebootMute != 0 )
		{
        	runAudioCfg.rebootMute = 0;
        	AudioCfgSave();
		}
	}
    return 0;
}
static int _new_check_network(void *arg)
{
    static int lastNetState = -1;
    char logTime[64];
    struct timezone Ztest;
    struct tm timeNow;
    struct timeval systime;
    
	if (netcam_net_get_detect("eth0") != 0)
	{
		if(!is_set_wireless)
    	{
            _set_wireless_param(&is_set_wireless);
    		is_set_wire = 0;
    	}
        if(lastNetState != LINK_DOWN)
        {
            memset(logTime, 0, sizeof(logTime));
            gettimeofday(&systime,&Ztest);
            if(systime.tv_sec > 24*60*60)
            {
                localtime_r(&systime.tv_sec, &timeNow);
                #ifdef MODULE_SUPPORT_ZDFF_LOG
                sprintf(logTime, "%04d-%02d-%02d %02d:%02d:%02d.%03d",\
                    timeNow.tm_year + 1900, timeNow.tm_mon + 1, \
                    timeNow.tm_mday,timeNow.tm_hour, timeNow.tm_min,\
                    timeNow.tm_sec, systime.tv_usec/1000);
                
                ite_eXosip_link_state_log_save(LINK_DOWN, logTime);
                #endif
            }
            else
                return 0;
        }
        lastNetState = LINK_DOWN;
	}
    else
    {
		if(!is_set_wire)
    	{
            _set_wire_param(&is_set_wire);
    		is_set_wireless = 0;
    	}
        if(lastNetState != LINK_UP)
        {
            memset(logTime, 0, sizeof(logTime));
            gettimeofday(&systime,&Ztest);
            if(systime.tv_sec > 24*60*60)
            {
                localtime_r(&systime.tv_sec, &timeNow);
                #ifdef MODULE_SUPPORT_ZDFF_LOG
                sprintf(logTime, "%04d-%02d-%02d %02d:%02d:%02d.%03d",\
                    timeNow.tm_year + 1900, timeNow.tm_mon + 1, \
                    timeNow.tm_mday,timeNow.tm_hour, timeNow.tm_min,\
                    timeNow.tm_sec, systime.tv_usec/1000);
                
                ite_eXosip_link_state_log_save(LINK_UP, logTime);
                #endif
            }
            else
                return 0;
        }
        lastNetState = LINK_UP;
    }
    return 0;
}

#ifdef MODULE_SUPPORT_TUTK
static int parse_qrcode_wifi_info(char *data,char *ssid,char *pw,char *enMode)
{
	// xuhui icute eye	format->   TP-IPC:22222222:7:USER_SIGN:random_num:
	char user_sign[32];
	int i = 0,j = 0;
	int separator[5];
	char random[10];
	GK_P2P_CFG p2pCfg;

	memset(separator,0,sizeof(separator));
	memset(random,0,sizeof(random));
	memset(ssid,0,MAX_WIFI_NAME);
	memset(pw,0,MAX_WIFI_NAME);
	memset(user_sign,0,sizeof(user_sign));
	memset(&p2pCfg,0,sizeof(GK_P2P_CFG));

	for(i=0;i<strlen(data);i++)
	{
		if(data[i] == ':')
			separator[j++] = i;
	}

	if(j != 5 && j != 4)
	{
		printf("icute eye qrcode format error!\n");
		return -1;
	}

	if(strncmp(&data[separator[1]],":7:",3))
	{
		printf("icute eye format error, position %d,%s!\n",separator[1],(char *)&data[separator[1]]);
		return -1;
	}

	strncpy(ssid,&data[0],separator[0]);
	printf("SSID = %s\n",ssid);

	if(separator[1]-separator[0] == 1)
	{
		printf("WIFI NO PSW\n");
	}
	else
	{
		strncpy(pw,&data[separator[0]+1],separator[1]-separator[0]-1);
		printf("PSW = %s\n",pw);
	}

	strncpy(user_sign,&data[separator[2]+1],separator[3]-separator[2]-1);
	printf("user_sign = %s\n",user_sign);

	get_param(P2P_PARAM_ID, &p2pCfg);

	if(!strlen(p2pCfg.user_sign))
	{
		strncpy(p2pCfg.user_sign,user_sign,sizeof(user_sign));
		set_param(P2P_PARAM_ID, &p2pCfg);
		P2PCfgSave();
	}

	if(j == 5)
	{
		strncpy(random,&data[separator[3]+1],separator[4]-separator[3]-1);
		icute_random_number = (unsigned char)atoi(random);
		printf("icute_random_number = %u\n",icute_random_number);
	}

	return 0;

}

#endif

#ifdef MODULE_SUPPORT_WAVE_CHANNEL

#include "adi_audio.h"
#include "wt_proto_common.h"
#include "wave_trans_recv.h"

static int wave_recv_running = 1;

static int sdk_audio_input_enable(bool flag)
{
    int ret;

    if(flag)
    {
        ret = gadi_audio_ai_enable();
#ifdef AUDIO_ECHO_CANCELLATION_SUPPORT
        ret = gadi_audio_ai_aec_enable();
#endif
    }
    else
    {
        ret = gadi_audio_ai_disable();
#ifdef AUDIO_ECHO_CANCELLATION_SUPPORT
        ret = gadi_audio_ai_aec_disable();
#endif

    }

    return 0;
}

static int parse_icute_wave_wifi_info(const char *data,char *ssid,char *pw,char *enMode)
{
	// xuhui icute eye	wave format-> TP-IPC:22222222:7:CRC32:
	//CRC32 = CRC32(TP-IPC:22222222:7:)
	//icute_random_number = CRC32 & 0xFF

	char user_sign[32];
	int i = 0,j = 0;
	int separator[5];
	char random[10];
	unsigned int crc32 = 0,crc32_real = 0;
	char tmp[64];

	memset(separator,0,sizeof(separator));
	memset(random,0,sizeof(random));
	memset(ssid,0,MAX_WIFI_NAME);
	memset(pw,0,MAX_WIFI_NAME);
	memset(user_sign,0,sizeof(user_sign));

	for(i=0;i<strlen(data);i++)
	{
		if(data[i] == ':')
			separator[j++] = i;
	}

	if(j != 4)
	{
		LOG_INFO("icute eye wave format error!\n");
		return -1;
	}

	if(strncmp(&data[separator[1]],":7:",3))
	{
		LOG_INFO("icute eye fwave ormat error, position %d,%s!\n",separator[1],(char *)&data[separator[1]]);
		return -1;
	}

	strncpy(ssid,&data[0],separator[0]);
	LOG_INFO("SSID = %s\n",ssid);

	if(separator[1]-separator[0] == 1)
	{

		LOG_INFO("WIFI NO PSW\n");
	}
	else
	{
		strncpy(pw,&data[separator[0]+1],separator[1]-separator[0]-1);
		LOG_INFO("PSW = %s\n",pw);
	}

	if(j == 4)
	{
	#if 0
		strncpy(random,&data[separator[2]+1],separator[3]-separator[2]-1);
		icute_random_number = (unsigned char)atoi(random);
		LOG_INFO("icute_random_number = %u\n",icute_random_number);
	#else

		strncpy(random,&data[separator[2]+1],separator[3]-separator[2]-1);

		crc32 = strtoul(random, NULL, 16);

		LOG_INFO("crc32 = 0x%x\n",crc32);

		memcpy(tmp,data,separator[2]+1);

		crc32_real = utility_crc32(0, tmp, separator[2]+1);

		LOG_INFO("%s,crc32_real = 0x%x\n",tmp,crc32_real);

		if(crc32_real != crc32)
		{
			LOG_INFO("CRC ERROR!\n");
			return -1;
		}

		icute_random_number = (crc32_real & 0xFF);

		LOG_INFO("icute_random_number = 0x%x\n",icute_random_number);

	#endif
	}

	return 0;
}

static void* net_wifi_WaveRecv_thread(void *arg)
{
	int ret;
	int readlen;
	char *dataBuf;
	int data_size = 0;
	char encode_aduio[512];
	char *tmp;
	GK_NET_FRAME_HEADER header = {0};
	MEDIABUF_HANDLE mediabufHandle = NULL;
    GADI_AUDIO_AioAttrT aiAttr;
	char path[128];
	int isHaveData = 0;
	int noDataCnt = 0;
	char data_temp[10] = { 0 };
    char psd[64] = {0};
    char ssid[64] = {0};
    char enMode[1] = {0};
    WIFI_LINK_INFO_t linkInfo;
    char audio_buf[2048] = {0};

    pthread_detach(pthread_self());
    sdk_sys_thread_set_name("net_wifi_WaveRev_thread");

	memset(&aiAttr,0,sizeof(GADI_AUDIO_AioAttrT));

	sdk_audio_input_enable(0);
    ret = gadi_audio_ai_get_attr(&aiAttr);
	aiAttr.sampleRate = GADI_AUDIO_SAMPLE_RATE_16000;
    ret = gadi_audio_ai_set_attr(&aiAttr);
	sdk_audio_input_enable(1);

	mediabufHandle = mediabuf_add_reader(GK_NET_STREAM_TYPE_STREAM1);
	if(mediabufHandle == NULL)
	{
		LOG_INFO("mediabuf_add_reader failed\n");
		return NULL;
	}

	if (WaveTransRecvInit() != 0)
	{
		LOG_INFO("wave trans recv init failed\n");
		return NULL;
	}

	memset(encode_aduio, 0, sizeof(encode_aduio));

	while(wave_recv_running)
	{

        while(netcam_get_update_status() < 0)
    	{
            LOG_INFO("wait at updating\n");
    		sleep(3);
    	}

		data_size = 0;
		dataBuf = NULL;
		readlen = mediabuf_read_frame(mediabufHandle, (void **)&dataBuf, &data_size, &header);
		if(readlen > 0)
		{
			if (header.frame_type != GK_NET_FRAME_TYPE_A)
				continue;
			else
			{
				memset(data_temp,0,sizeof(data_temp));
				memset(audio_buf,0,sizeof(audio_buf));
				memcpy(audio_buf,dataBuf,data_size);

				WaveTransRecvSetPcm(audio_buf, data_size / sizeof(RecvAudioType));

				ret = WaveTransRecvGetContext(data_temp, sizeof(data_temp));
				if (ret != 0)
				{
					data_temp[ret] = '\0';
					strcat(encode_aduio, data_temp);
					isHaveData = 1;
					noDataCnt = 0;
				}
				else
				{
					noDataCnt++;
				}

				if (isHaveData && noDataCnt > 20)
				{
					LOG_INFO("--> %s\n", encode_aduio);
					isHaveData = 0;

					memset(ssid,0,sizeof(ssid));
					memset(psd,0,sizeof(psd));
					ret = parse_icute_wave_wifi_info(encode_aduio,ssid,psd,enMode);
					if(ret == 0)
					{
						if(strlen(ssid) > 0)
						{
							memset(&linkInfo,0,sizeof(linkInfo));
							linkInfo.isConnect = 1;
							linkInfo.linkScurity = WIFI_ENC_WPAPSK_AES;
							strncpy(linkInfo.linkEssid, ssid, sizeof(linkInfo.linkEssid));
							strncpy(linkInfo.linkPsd, psd, sizeof(linkInfo.linkPsd));

						    LOG_INFO("linkInfo.essid:%s, pswd:%s\n", linkInfo.linkEssid, linkInfo.linkPsd);

							ret = netcam_net_setwifi(NETCAM_WIFI_STA,linkInfo);

							LOG_INFO("1. net_wifi_WaveRev_thread!\n");
							break;
						}
					}

					memset(encode_aduio, 0, sizeof(encode_aduio));
				}

			}
		}
		else
		{
			usleep(10000);
			memset(&linkInfo, 0, sizeof(WIFI_LINK_INFO_t));
			if(netcam_net_wifi_get_connect_info(&linkInfo) == 0 && linkInfo.linkStatus == WIFI_CONNECT_OK)
			{
				LOG_INFO("2. net_wifi_WaveRev_thread!\n");
				break;
			}
		}
	}

	sdk_audio_input_enable(0);
	aiAttr.sampleRate = GADI_AUDIO_SAMPLE_RATE_8000;
    ret = gadi_audio_ai_set_attr(&aiAttr);
	sdk_audio_input_enable(1);

	WaveTransRecvExit();
	mediabuf_del_reader(mediabufHandle);

	return NULL;
}

static void net_wifi_stop_wave_recv_thread(void)
{
	wave_recv_running = 0;
}

static void net_wifi_wave_recv_thread_start(void)
{
    pthread_t WaveRecvPid;
	int ret = 0;

	ret = pthread_create(&WaveRecvPid, NULL, net_wifi_WaveRecv_thread, NULL);
	if(ret != 0)
	{
		LOG_ERR("net_wifi_wave_recv_thread_start failed!\n");
	}
}

#endif

unsigned char netcam_net_get_icute_number(void)
{
	return icute_random_number;
}

static int _qrcode_check(void *arg)
{
    int ret;

	char * wifi_name = NULL;

    printf("_qrcode_check 1\n");
#if 0
#if ETHERNET_LINK_STOP_SMART_LINK
    if(0 == sdk_net_get_detect("eth0") )
    {
        if(zbar_qrcode_link_support() && zbar_qrcode_link_status())
        {
				zbar_qrcode_link_stop();

                NETWORK_PRINT_INFO("eth0 connected ,zbar_qrcode_link_stop\n");
        }
        return 0;
    }
#endif
#endif
	wifi_name = net_wifi_open();
	if(wifi_name == NULL) {
        NETWORK_PRINT_ERROR("wifi name is NULL\n");
        return 0;
	}
    printf("_qrcode_check 2\n");

	ret = net_wifi_cli_status(wifi_name);
	if(ret == WIFI_STATUS_CONNECTED /*|| ret == WIFI_STATUS_ERROR*/
		|| ret == WIFI_STATUS_UNKNOW)
    {
		NETWORK_PRINT_INFO("net_wifi_cli_status RET:%d \n",ret);

        if(zbar_qrcode_link_support() && zbar_qrcode_link_status())
        {
				zbar_qrcode_link_stop();
                NETWORK_PRINT_INFO("wifi connected ,zbar_qrcode_link_stop\n");
        }

		return 0;
	}

    printf("_qrcode_check 3\n");

	if( zbar_qrcode_link_support())
    {
		ret = net_wifi_cli_status(wifi_name);
		if(ret != WIFI_STATUS_CONNECTED) {
			if(!zbar_qrcode_link_status()) {
				zbar_qrcode_link_start();
			}
		}
	}


    return 0;
}

static int _net_check_attr(ST_SDK_NETWORK_ATTR *attr)
{
    int ret = 0;
    if(attr->dhcp == 0)
    {
        ret = sdk_net_is_ipaddr(attr->ip, IPV4);
        if(ret != 0)
        {
            LOG_ERR("dev:%s, ip:%s, format error\n",  attr->name, attr->ip);
            return ret;
        }

        ret = sdk_net_is_ipaddr(attr->mask, IPV4);
        if(ret != 0)
        {
            LOG_ERR("dev:%s, mask:%s, format error\n",  attr->name, attr->mask);
            return ret;
        }

        ret = sdk_net_is_ipaddr(attr->gateway, IPV4);
        if(ret != 0)
        {
            LOG_ERR("dev:%s, gateway:%s, format error\n",  attr->name, attr->gateway);
            return ret;
        }

        if(attr->dns1[0]!= 0)
        {
            ret = sdk_net_is_ipaddr(attr->dns1, IPV4);
            if(ret != 0)
            {
                LOG_ERR("dev:%s, dns1:%s, format error\n",  attr->name, attr->dns1);
                return ret;
            }
        }

        if(attr->dns2[0]!= 0)
        {
            ret = sdk_net_is_ipaddr(attr->dns2, IPV4);
            if(ret != 0)
            {
                LOG_ERR("dev:%s, dns2:%s, format error\n",  attr->name, attr->dns2);
                return ret;
            }
        }
    }

    return ret;
}
static int _net_sync_attr2dev(ST_SDK_NETWORK_ATTR *attr)
{
    #if NET_ENABLE_CONTROL
    if(attr->enable != 1)
    {
        sdk_net_up_down_network(attr->name, attr->enable);
        return 0;
    }
    if(attr->enable == 1)
    sdk_net_up_down_network(attr->name, 0);
//	 sdk_net_up_down_network(attr->name, attr->enable);

    #endif

    printf("begin set\n");
    if(sdk_net_set_ip(attr->name, attr->ip) < 0)
    { LOG_ERR("set :%s, ip:%s error",attr->name,attr->ip);}
    if(sdk_net_set_hwaddr(attr->name, (char *)attr->mac) < 0)
        LOG_ERR("set :%s, hwaddr:%s error",attr->name,attr->mac);//	xqq set hwaddr failed
//	printf("after sdk_net_set_ip\n");
    if(sdk_net_set_netmask(attr->name, attr->mask) < 0)
    {        LOG_ERR("set :%s, mask:%s error",attr->name,attr->mask);}
  //  printf("sdk_net_set_netmask\n");
    if(sdk_net_set_gateway(attr->name, attr->gateway) < 0)
    {        LOG_ERR("set :%s, gateway:%s error",attr->name,attr->gateway);}
    // printf("after sdk_net_set_gateway\n");
    if(sdk_net_set_dns(attr->dns1, attr->dns2) < 0)
    {        LOG_ERR("set :%s, dns1:%s, dns2:%s error",attr->name,attr->dns1,attr->dns2);}
//	printf("after sdk_net_set_dns\n");
	sdk_net_up_down_network(attr->name, attr->enable);
	sleep(1);
	// and udhcpc should be killed, or it will get dynamic IP when IPC connects to router.
	
	if(sdk_net_set_dhcp(attr->name,attr->dhcp, netcam_sys_get_name()) < 0)
	{        LOG_ERR("set:%s, dhcp:%d\n", attr->name, attr->dhcp);}



    return 0;

}

static int _net_set_attr2dev(ST_SDK_NETWORK_ATTR *attr, GK_NET_ETH *cfg_attr)
{
    if(!attr || !cfg_attr)
    {
        LOG_ERR("gk_network_set_attr fail atrr:%p, cfg_attr:%p!\n", attr, cfg_attr);
        return -1;
    }
    if(_net_check_attr(attr)!= 0)
    {
        LOG_ERR("not valid network attr fail !\n");
        return -1;
    }

    #if NET_ENABLE_CONTROL
    if(cfg_attr->enable != attr->enable)
    {
        if(cfg_attr->enable == 0) //原来的为0， 新的则为1
        {
            sdk_net_up_down_network(attr->name, 1);
            _net_sync_attr2dev(attr);
        }
        else //原来的为1， 新的则为0
        {
            sdk_net_up_down_network(attr->name, 0);
            if(cfg_attr->dhcpIp == 1)
            {
               sdk_net_set_dhcp(attr->name,0, netcam_sys_get_name());
            }
        }
        return 0;
    }
    #endif
   _net_sync_attr2dev(attr);		
    return 0;
}
static int _net_sync_dev2attr(ST_SDK_NETWORK_ATTR *attr, char *ethName)
{
    int ret = 0;

    ret = sdk_net_get_ip(ethName, attr->ip);
    if(ret < 0)
        LOG_ERR("Fail to get IP.");
    ret = sdk_net_get_hwaddr(ethName, (char *)attr->mac);
    if(ret < 0)
        LOG_ERR("Fail to get mac.");
    ret = sdk_net_get_netmask(ethName, attr->mask);
    if(ret < 0)
        LOG_ERR("Fail to get mask.");
    ret = sdk_net_get_gateway(ethName, attr->gateway);
    if(ret < 0)
        LOG_ERR("Fail to get gateway.");
    ret = sdk_net_get_dns( attr->dns1,attr->dns2);
    if(ret < 0)
        LOG_ERR("Fail to get dns.");

    /*
     * TODO: get gateway, dns, httpPort
     */
    return 0;
}

static int _net_sync_cfg2attr(GK_NET_NETWORK_CFG *cfg, ST_SDK_NETWORK_ATTR *net_attr, char *name)
{
    if(!cfg || !net_attr || !name){
        PRINT_ERR("param error, cfg:%p, net_attr:%p, name:%p\n", cfg, net_attr, name);
        return -1;
    }

    if(!strcmp(name, cfg->lan.netName)){
        net_attr->enable = cfg->lan.enable;
        net_attr->dhcp = cfg->lan.dhcpIp;
        net_attr->httpPort = cfg->port.httpPort;
        strcpy((char*)net_attr->mac, cfg->lan.mac);
        strcpy(net_attr->name,  cfg->lan.netName);
        strcpy(net_attr->ip,  cfg->lan.ip);
        strcpy(net_attr->mask,cfg->lan.netmask);
        strcpy(net_attr->gateway,cfg->lan.gateway);
        strcpy(net_attr->dns1, cfg->lan.dns1);
        strcpy(net_attr->dns2, cfg->lan.dns2);
    }else if(!strcmp(name, cfg->wifi.netName)){
        net_attr->enable = cfg->wifi.enable;
        net_attr->dhcp = cfg->wifi.dhcpIp;
        net_attr->httpPort = cfg->port.httpPort;
        strcpy(net_attr->name,  cfg->wifi.netName);
        strcpy((char*)net_attr->mac, (char*)cfg->wifi.mac);
        strcpy(net_attr->ip,  cfg->wifi.ip);
        strcpy(net_attr->mask,cfg->wifi.netmask);
        strcpy(net_attr->gateway,cfg->wifi.gateway);
        strcpy(net_attr->dns1, cfg->wifi.dns1);
        strcpy(net_attr->dns2, cfg->wifi.dns2);
    }else{
        PRINT_ERR("net name error:%s,lan:%s, wifi:%s\n", name, cfg->lan.netName, cfg->wifi.netName);
        return -1;
    }
    return 0;
}

static int _net_sync_attr2cfg(GK_NET_NETWORK_CFG *cfg, ST_SDK_NETWORK_ATTR *net_attr, char *name)
{
    if(!cfg || !net_attr || !name){
        PRINT_ERR("param error, cfg:%p, net_attr:%p, name:%p\n", cfg, net_attr, name);
        return -1;
    }

    if(!strcmp(net_attr->name, cfg->lan.netName)){
        runNetworkCfg.lan.enable = net_attr->enable;
        runNetworkCfg.lan.dhcpIp = net_attr->dhcp;
        runNetworkCfg.port.httpPort = net_attr->httpPort;
        strcpy(runNetworkCfg.lan.mac,(char*)net_attr->mac);
        strcpy(runNetworkCfg.lan.ip,net_attr->ip);
        strcpy(runNetworkCfg.lan.netmask, net_attr->mask);
        strcpy(runNetworkCfg.lan.gateway,net_attr->gateway);
        strcpy(runNetworkCfg.lan.dns1,net_attr->dns1);
        strcpy(runNetworkCfg.lan.dns2,net_attr->dns2);
    }else if(!strcmp(net_attr->name, cfg->wifi.netName)){
        runNetworkCfg.wifi.enable = net_attr->enable;
        runNetworkCfg.wifi.dhcpIp = net_attr->dhcp;
        runNetworkCfg.port.httpPort = net_attr->httpPort;
        strcpy(runNetworkCfg.wifi.mac,(char*)net_attr->mac);
        strcpy(runNetworkCfg.wifi.ip,net_attr->ip);
        strcpy(runNetworkCfg.wifi.netmask, net_attr->mask);
        strcpy(runNetworkCfg.wifi.gateway,net_attr->gateway);
        strcpy(runNetworkCfg.wifi.dns1,net_attr->dns1);
        strcpy(runNetworkCfg.wifi.dns2,net_attr->dns2);
    }else{
        PRINT_ERR("net name error:%s\n", name);
        return -1;
    }
    return 0;
}

int netcam_net_register_smartlink (NETCAM_SmartLinkFuncT *func)
{
    return net_wifi_smartlink_register(func);
}

int netcam_net_private_smartlink(void)
{
    return net_wifi_smartlink_private();
}

int netcam_net_wifi_getConfigHostAP(NETCAM_WIFI_HostAPConfigT *apCfg)
{
    return network_wifi_getConfigHostAP(apCfg);
}

int netcam_net_wifi_setConfigHostAP(NETCAM_WIFI_HostAPConfigT *apCfg)
{
    return network_wifi_setConfigHostAP(apCfg);
}
int netcam_net_wifi_get_scan_list(WIFI_SCAN_LIST_t *list, int *number)
{
    return network_wifi_get_scan_list(list, number);
}

int netcam_net_wifi_get_scan_list_ext(WIFI_SCAN_LIST_t *list, int *number)
{
    return network_wifi_get_scan_list_ext(list, number);
}
int netcam_net_wifi_get_scan_list_ap(WIFI_SCAN_LIST_t *list, int *number)
{
    return network_wifi_get_scan_list_ap(list, number);
}

int netcam_net_wifi_get_connect_info(WIFI_LINK_INFO_t *linkInfo)
{
    return network_wifi_get_connect_info(linkInfo);
}
int netcam_net_wifi_probe(void)
{
    return network_wifi_probe();
}
int netcam_net_wifi_switchWorkMode(NETCAM_WIFI_WorkModeEnumT mode)
{
    if(0 == network_wifi_switch_workmode(mode)){
        if(mode == NETCAM_WIFI_AP)
            _net_set_wificfg_mode(0);
        else
            _net_set_wificfg_mode(1);

        _net_set_wifiCfg_switch(1);
	    netcam_net_cfg_save();
        return 0;
    }
    return -1;
}

#ifdef MODULE_SUPPORT_ZK_WAPI
static ST_SDK_NETWORK_ATTR wapi_net_attr={0};
int netcam_net_set_wapi(ST_SDK_NETWORK_ATTR *net_attr)
{
    if(!net_attr){
        PRINT_ERR("param is NULL\n");
        return -1;
    }

    int dataLength, dataRecvLen;
    char *post=NULL;
    cJSON *Array, *Item;
    Item = cJSON_CreateObject();
    if(Item)
    {
        cJSON_AddItemToObject(Item, "proto", cJSON_CreateString(net_attr->dhcp?"dhcp":"static"));
        cJSON_AddItemToObject(Item, "ipaddr", cJSON_CreateString(net_attr->ip));
        cJSON_AddItemToObject(Item, "netmask", cJSON_CreateString(net_attr->mask));
        cJSON_AddItemToObject(Item, "gateway", cJSON_CreateString(net_attr->gateway));
        Array = cJSON_CreateArray();
        if(Array)
        {
            if(strlen(net_attr->dns1))
                cJSON_AddItemToArray(Array, cJSON_CreateString(net_attr->dns1));
            if(strlen(net_attr->dns2))
                cJSON_AddItemToArray(Array, cJSON_CreateString(net_attr->dns2));
            cJSON_AddItemToObject(Item, "dns", Array);
        }
        post = cJSON_Print(Item);
        cJSON_Delete(Item);
    }
    if(post == NULL)
    {
        PRINT_ERR("Faile to get network info.");
		return -1;
    }
    if(memcmp(&wapi_net_attr, net_attr, sizeof(ST_SDK_NETWORK_ATTR))!=0)
        memcpy(&wapi_net_attr, net_attr, sizeof(ST_SDK_NETWORK_ATTR));
    dataLength = strlen(post);
    char *json = NULL;
    json = goke_http_request("http://192.168.2.1:80/set_wan.wapi", &dataLength, &dataRecvLen, HTTP_POST, post);
    if(NULL != json)
    {
        PRINT_INFO("netcam_net_set_wapi:%s\n", json);
        free(json);
    }
    free(post);
    return 0;
}

int netcam_net_get_wapi(ST_SDK_NETWORK_ATTR *net_attr)
{
    static unsigned int cnt=0;
    int ret = -1;
    int dataLength, dataRecvLen;
    char *json = NULL;

    if(!net_attr)
    {
        PRINT_ERR("%s fail !", __func__);
        return -1;
    }
    memcpy(net_attr, &wapi_net_attr, sizeof(ST_SDK_NETWORK_ATTR));
    if(cnt%10==0)
        json = goke_http_request("http://192.168.2.1:80/get_wan.wapi", &dataLength, &dataRecvLen, HTTP_GET, NULL);
    //if(wapi_net_attr.enable)
    //    cnt++;
    if(NULL != json)
    {
        int ret,size=0;
        cJSON *obj,*Array, *Item = cJSON_Parse(json);
        free(json);
        //memset(net_attr, 0, sizeof(ST_SDK_NETWORK_ATTR));
		if(Item == NULL)
		{
			PRINT_ERR("Faile to get network info, http body json error\n");
			return -1;
		}
        net_attr->enable = 1;
        obj = cJSON_GetObjectItem(Item,"proto");
        if(obj)
            net_attr->dhcp = strcmp(obj->valuestring,"dhcp")==0?1:0;
        obj = cJSON_GetObjectItem(Item,"macaddr");
        if(obj)
            strncpy(net_attr->mac, obj->valuestring, sizeof(net_attr->mac)-1);
        obj = cJSON_GetObjectItem(Item,"ipaddr");
        if(obj)
            strncpy(net_attr->ip, obj->valuestring, sizeof(net_attr->ip)-1);
        obj = cJSON_GetObjectItem(Item,"netmask");
        if(obj)
            strncpy(net_attr->mask, obj->valuestring, sizeof(net_attr->mask)-1);
        obj = cJSON_GetObjectItem(Item,"gateway");
        if(obj)
            strncpy(net_attr->gateway, obj->valuestring, sizeof(net_attr->gateway)-1);
        Array = cJSON_GetObjectItem(Item, "dns");
        if(Array)
        {
            size = cJSON_GetArraySize(Array);
            if(size > 0)
            {
                obj = cJSON_GetArrayItem(Array,0);
                if(obj)
                    strncpy(net_attr->dns1, obj->valuestring, sizeof(net_attr->dns1)-1);
            }
            if(size > 1)
            {
                obj = cJSON_GetArrayItem(Array,1);
                if(obj)
                    strncpy(net_attr->dns2, obj->valuestring, sizeof(net_attr->dns2)-1);
            }
        }
        cJSON_Delete(Item);
        if(memcmp(&wapi_net_attr, net_attr, sizeof(ST_SDK_NETWORK_ATTR))!=0)
            memcpy(&wapi_net_attr, net_attr, sizeof(ST_SDK_NETWORK_ATTR));
    }
    return 0;
}
#endif

int netcam_net_set(ST_SDK_NETWORK_ATTR *net_attr)
{
    int is_restart = 0;
	int ret=0;

    if(!net_attr){
        PRINT_ERR("param is NULL\n");
        return -1;
    }
	if(net_attr->dhcp==0)
	{
        /*检测IP冲突*/
        #if 0
        ret = sdk_net_get_ip(net_attr->name, net_attr->ip);
        if(ret < 0)
            PRINT_ERR("Fail to get IP.");
        else
        {
            if(check_ip(net_attr->name,net_attr->ip, NULL) != 0)
            {
                PRINT_ERR("check_ip err\n");
                return 1;
            }      
        }
        #endif
	}
    _net_enter_lock();
    if(!strcmp(net_attr->name, runNetworkCfg.lan.netName)){
        if(_net_set_attr2dev(net_attr, &runNetworkCfg.lan) < 0)
	{
	    printf("==========goto error===========\n");
            goto error;
	}
    }else if(!strcmp(net_attr->name, runNetworkCfg.wifi.netName)){
        if(_net_set_attr2dev(net_attr, &runNetworkCfg.wifi) < 0)
	{
		printf("==========goto error2===========\n");
            goto error;
	}
    }
    _net_sync_attr2cfg(&runNetworkCfg, net_attr, net_attr->name);
    _net_leave_lock();
    if (is_restart)
    {
		netcam_net_cfg_save();
		netcam_sys_operation(NULL, SYSTEM_OPERATION_REBOOT);
    }else{
		netcam_net_cfg_save();
    }
    return ret;
error:
    _net_leave_lock();
    return -1;
}


int netcam_net_getcfg(ST_SDK_NETWORK_ATTR *net_attr)
{
    return  _net_sync_cfg2attr(&runNetworkCfg, net_attr, net_attr->name);
}
int netcam_net_get(ST_SDK_NETWORK_ATTR *net_attr)
{
    int ret = -1;

    if(!net_attr)
    {
        PRINT_ERR("%s fail !", __func__);
        return -1;
    }

    _net_enter_lock();
    ret = _net_sync_cfg2attr(&runNetworkCfg, net_attr, net_attr->name);
    _net_leave_lock();
    if(ret < 0){
        PRINT_ERR("get net attr fail\n");
        return -1;
    }
	_net_sync_dev2attr(net_attr, net_attr->name);

    return 0;
}

void netcam_net_cfg_save(void)
{

    if(0 == _net_enter_lock())
    {
        PRINT_INFO("netcam_cfg_network_save!!");
        NetworkCfgSave();
        _net_leave_lock();
    }

}

static unsigned char random_ip(unsigned char nip)
{
	unsigned char ip = nip;
	unsigned int seed = 0;
	struct timespec tv;
	seed = (unsigned int)getpid()+(unsigned int)&nip+(unsigned int)&ip;
	usleep(500000);
	clock_gettime(CLOCK_REALTIME, &tv);
	seed += tv.tv_sec + tv.tv_nsec;
	//PRINT_INFO("random_ip seed: %u\n", seed);
	srand(seed);
	do
	{
		ip = (rand() % 254)+1;
	}while(ip == nip);
	//PRINT_INFO("random_ip: %d<-%d\n", ip, nip);
	return ip;
}

int netcam_net_autotracking(char *ip)
{
	ST_SDK_NETWORK_ATTR net_attr;
	int i=0, ret = 0;
	unsigned char dip[4] = {0}, nip[4] = {0}, nmask[4] = {0};

	if(0 == runNetworkCfg.lan.autotrack)
	{
		//LOG_ERR("autotrack is off\n");
		return 0;
	}
	pthread_mutex_lock(&network_autotracking_lock);

	memset(&net_attr, 0, sizeof(ST_SDK_NETWORK_ATTR));
	strncpy(net_attr.name, LAN, sizeof(net_attr.name)-1);
	ret = netcam_net_get(&net_attr);
	if(ret != 0)
	{
		LOG_ERR("Faile to Get net_attr_local.");
		pthread_mutex_unlock(&network_autotracking_lock);
		return -1;
	}
	LOG_INFO("autotracking:%s<-%s\n", ip, net_attr.ip);
	sscanf(ip, "%hhu.%hhu.%hhu.%hhu", &dip[0],&dip[1],&dip[2],&dip[3]);
	sscanf(net_attr.ip, "%hhu.%hhu.%hhu.%hhu", &nip[0],&nip[1],&nip[2],&nip[3]);
	if(dip[0] && (dip[0]!=nip[0] || dip[1]!=nip[1] || dip[2]!=nip[2]))
	{
		memcpy(nip, dip, sizeof(nip));
		net_attr.dhcp = 0;
		sscanf(net_attr.mask, "%hhu.%hhu.%hhu.%hhu", &nmask[0],&nmask[1],&nmask[2],&nmask[3]);
		sprintf(net_attr.gateway, "%hhu.%hhu.%hhu.1", nip[0]&nmask[0],nip[1]&nmask[1],nip[2]&nmask[2]);
		do{
			nip[3] = random_ip(dip[3]);
			sprintf(net_attr.ip, "%hhu.%hhu.%hhu.%hhu", nip[0],nip[1],nip[2],nip[3]);
			LOG_INFO("tracking net ip for the %dth:%s, gateway:%s\n", i, net_attr.ip, net_attr.gateway);
			ret = netcam_net_set(&net_attr);
			i++;
		}while(0 != ret && i<10);
	}

	pthread_mutex_unlock(&network_autotracking_lock);
	return ret;
}

/*
 * char *name:  eth0 or ra0
 * return  0, connect
 * return -1, disconnect or disable
 */
int netcam_net_get_detect(char *name)
{
    return sdk_net_get_detect(name);
}

int netcam_net_wifi_isenbale(void)
{
	return runNetworkCfg.wifi.enable;
}

NETCAM_WIFI_WorkModeEnumT netcam_net_wifi_getWorkMode(void)
{
	return network_wifi_get_status();
}

int netcam_net_wifi_set_connect_info(WIFI_LINK_INFO_t *linkInfo)
{
    int ret;
    ret = network_wifi_connect(linkInfo);
    if(WIFI_OK == ret){
        _net_add_wifilink_info(linkInfo);
        return ret;
    }
    return ret;
}
int netcam_net_wifi_connect_test(WIFI_LINK_INFO_t *linkInfo)
{
    int ret;

    PRINT_INFO("WIFI TEST: .linkEssid    :%s ", linkInfo->linkEssid);
    PRINT_INFO("WIFI TEST: .linkPsd      :%s ", linkInfo->linkPsd);
    PRINT_INFO("WIFI TEST: .linkScurity  :%d ", linkInfo->linkScurity);
    PRINT_INFO("WIFI TEST: .enable       :%d ", linkInfo->enable);
    PRINT_INFO("WIFI TEST: .isConnect    :%d ", linkInfo->isConnect);
    PRINT_INFO("WIFI TEST: .linkStatus   :%d ", linkInfo->linkStatus);

    // disconnet the connected wifi.
    ret = network_wifi_link_test(linkInfo);
    LOG_INFO("network_wifi_link_test: %d", ret);
    if(ret == WIFI_OK){
        //netcam_audio_hint(SYSTEM_AUDIO_HINT_LINK_SUCCESS);	xqq
        linkInfo->linkStatus = WIFI_CONNECT_OK;
    } else {
        //netcam_audio_hint(SYSTEM_AUDIO_HINT_LINK_FAIL);	xqq
        linkInfo->linkStatus = WIFI_CONNECT_FAIL;
    }
    return ret;
}

char* netcam_net_wifi_get_devname(void)
{
	return (char*)net_wifi_open();
}
/*!
******************************************
** Description   @开始wps方式连接
** Param[in]     @NULL
**
** Return        @连接成功0，失败-1
******************************************
*/
int netcam_net_wifi_wps(void)
{
	char *ifname = NULL;

    if(network_wifi_wps_connect() < 0)
        return -1;

	ifname = netcam_net_wifi_get_devname();
	if(ifname == NULL) {
		PRINT_ERR("No wireless network card device.");
		return -1;
	}
	if(net_wifi_cli_status(ifname) == WIFI_STATUS_CONNECTED){
		//player hint audio, and reset ip.
		netcam_net_reset_net_ip(ifname,NULL);
		//if (hint_audio)
			//netcam_audio_hint(SYSTEM_AUDIO_HINT_LINK_SUCCESS);	xqq
        return 0;
	}  else {
		//if (hint_audio)
			//netcam_audio_hint(SYSTEM_AUDIO_HINT_LINK_FAIL);	xqq
        return -1;
	}
    return -1;
}

int netcam_net_reset_net_ip(char *name, ST_SDK_NETWORK_ATTR *newInfo)
{
	int ret = 0;
	char buffer[256];

    if(name == NULL)
    {
        return 0;
    }

	if(newInfo != NULL && strcmp(newInfo->name,name) != 0)
	{
		return 0;
	}

	ret = netcam_net_get_detect("eth0");
	if(ret != 0)
	{
		//PRINT_INFO("eth0 is not running.");
		if(netcam_net_wifi_isOn() == 1)
		{
			ST_SDK_NETWORK_ATTR netAttr;
			WIFI_LINK_INFO_t linkInfo;
			memset(&linkInfo, 0, sizeof(WIFI_LINK_INFO_t));
			memset(&netAttr, 0, sizeof(ST_SDK_NETWORK_ATTR));
			if(netcam_net_wifi_get_connect_info(&linkInfo) == 0 && linkInfo.linkStatus == WIFI_CONNECT_OK)
			{
				if(newInfo  == NULL)
				{
					strcpy(netAttr.name,netcam_net_wifi_get_devname());
					netcam_net_get(&netAttr);
				}
				else
				{
					memcpy(&netAttr,newInfo,sizeof(ST_SDK_NETWORK_ATTR));
				}
                new_system_call("ifconfig eth0 0.0.0.0");
                new_system_call("route  del default dev eth0");
				PRINT_INFO("ETH0 use default 0.0.0.0 \n");
				netcam_net_set(&netAttr);
				PRINT_INFO("Reset wifi IP address\n");
			}
		}
	}
	else
	{
		//PRINT_INFO("eth0 is running.");
		WIFI_LINK_INFO_t linkInfo;
		ST_SDK_NETWORK_ATTR netAttr;
		if(newInfo == NULL)
		{
			memset(&netAttr, 0, sizeof(ST_SDK_NETWORK_ATTR));
			strncpy(netAttr.name, "eth0", SDK_ETHERSTR_LEN-1);
			ret = netcam_net_get(&netAttr);
			if(ret != 0)
			{
				PRINT_ERR("Faile to Get net Attr.");
				return 0;
			}
		}
		else
		{
			memcpy(&netAttr,newInfo,sizeof(ST_SDK_NETWORK_ATTR));
		}
		ret = netcam_net_set(&netAttr);
		if(ret != 0)
		{
			PRINT_ERR("Fail to set HDCP");
			return 0;
		}

		memset(&linkInfo, 0, sizeof(WIFI_LINK_INFO_t));
		memset(buffer,0,sizeof(buffer));
		if(netcam_net_wifi_get_connect_info(&linkInfo) == 0 && linkInfo.linkStatus == WIFI_CONNECT_OK)
		{
            char *wifi_name = netcam_net_wifi_get_devname();
            if(wifi_name)
            {
                sprintf(buffer,"ifconfig %s 0.0.0.0",wifi_name);
                new_system_call(buffer);
                memset(buffer, 0, sizeof(buffer));
                sprintf(buffer,"route  del default dev %s",wifi_name);
                new_system_call(buffer);
                PRINT_INFO("WIFI use default 0.0.0.0 \n");
            }
		}
	}
    return 0;
}

#ifdef MODULE_SUPPORT_WORKQUEUE
void netcam_net_monitor(void)
{
	//wifi check every 10 second

	CREATE_WORK(wifiCycleTaskOne, CONDITION_TIMER_WORK, (WORK_CALLBACK)_wifi_monitor);
	INIT_WORK(wifiCycleTaskOne, COMPUTE_TIME(0,0,0,1,0), NULL); // running one times
	SCHEDULE_DEFAULT_WORK(wifiCycleTaskOne);

	CREATE_WORK(wifiCycleTask, TIME_TIMER_WORK, (WORK_CALLBACK)_wifi_monitor);
	INIT_WORK(wifiCycleTask, COMPUTE_TIME(0,0,0,30,0), NULL); //30 second > connect time 20 second
	SCHEDULE_DEFAULT_WORK(wifiCycleTask);


	CREATE_WORK(netCheck, TIME_TIMER_WORK, _new_check_network);
	INIT_WORK(netCheck, COMPUTE_TIME(0,0,0,1,0), NULL); // 1 second
	SCHEDULE_DEFAULT_WORK(netCheck);

    #ifdef MODULE_SUPPORT_QRCODE_LINK

	#ifdef MODULE_SUPPORT_TUTK
	zbar_qrcode_set_parse_callback(parse_qrcode_wifi_info);
	#endif

	CREATE_WORK(qrcodeCheckOnce, CONDITION_TIMER_WORK, (WORK_CALLBACK)_qrcode_check);
	INIT_WORK(qrcodeCheckOnce, COMPUTE_TIME(0,0,0,1,0), NULL); // running one times
	SCHEDULE_DEFAULT_WORK(qrcodeCheckOnce);

	CREATE_WORK(qrcodeCheck, TIME_TIMER_WORK, (WORK_CALLBACK)_qrcode_check);
	INIT_WORK(qrcodeCheck, COMPUTE_TIME(0,0,0,30,0), NULL); // 3 second
	SCHEDULE_DEFAULT_WORK(qrcodeCheck);
    #endif

	#ifdef MODULE_SUPPORT_WAVE_CHANNEL
	net_wifi_wave_recv_thread_start();
	#endif
}
#endif

int netcam_net_wifi_isOn(void)
{
	return (runNetworkCfg.wifi.enable);
}

int netcam_net_wifi_on(void)
{
    if(!network_wifi_is_init()){
        netcam_net_wifi_start(1);//sta mode
    }

    if(0 == network_wifi_on()){
        _net_set_wifiCfg_switch(1);
        return 0;
    }
    return -1;
}

int netcam_net_wifi_off(void)
{
    if(0 == network_wifi_off()){
        _net_set_wifiCfg_switch(0);
        return 0;
    }
    return -1;
}
int netcam_net_init(void)
{
    int ret;
    char *pname = NULL;
    WIFI_LINK_INFO_t linkInfo;
    ST_SDK_NETWORK_ATTR net_attr2;
    ST_SDK_NETWORK_ATTR attr;
    if(-1 == access(NETWOR_TOOLS_CTRL_FILE, W_OK))
    {
        //unlink(NETWOR_TOOLS_CTRL_FILE);
        if(mkfifo(NETWOR_TOOLS_CTRL_FILE, 0666))
        {
            printf("mkfifo fail :%s\n", NETWOR_TOOLS_CTRL_FILE);
            perror("mkfifo fail:\n");
            return -1;
        }
    }

    pthread_rwlock_init(&network_lock, NULL);
    pname = runNetworkCfg.lan.netName;
    if(_net_sync_cfg2attr(&runNetworkCfg, &attr, pname) < 0)
        PRINT_ERR("cfg to attr error\n");
    netcam_net_set(&attr);
    return 0;
}

int netcam_net_wifi_start(int mode)
{
    int isInit = 0;
    //runNetworkCfg.wireless.mode
    if(!network_wifi_is_init() && 0 == network_wifi_init(mode)){
        char *WifiName = NULL;
        char ssidName[64] = {0};
        WifiName = netcam_net_wifi_get_devname();
        if(WifiName != NULL && strncmp(runNetworkCfg.wifi.netName, WifiName, sizeof(runNetworkCfg.wifi.netName)) != 0){
            strncpy(runNetworkCfg.wifi.netName, WifiName, sizeof(runNetworkCfg.wifi.netName));
            //save wifi netname
            netcam_net_cfg_save();
        }

#ifdef MODULE_SUPPORT_GB28181
        if (runNetworkCfg.wireless.mode == 0)
        {
            if (!strcmp(runGB28181Cfg.DeviceUID,"0000000000000000000"))
            {
                gb28181_load_id();
            }

            sprintf(ssidName, "IPCamera_%s", runGB28181Cfg.DeviceUID + strlen(runGB28181Cfg.DeviceUID) - 6);
            printf("set ap ssid:%s\n", ssidName);
            net_wifi_hostap_set_init_ssid(ssidName);
        }
#endif
        network_wifi_on();
        isInit = 1;
    }

    return isInit;
}

int netcam_net_wifi_init(void)
{
    char ssidName[64] = {0};
    FILE *fp;
    char *getsRet;
    char wifiInfo[64];
    WIFI_LINK_INFO_t info;
    int wifiMode = runNetworkCfg.wireless.mode;
    if(runNetworkCfg.wifi.enable == 1)
    {
        if (access("/mnt/sd_card/ding_ping_config/ding_ping.sh",R_OK) == 0)
        {
            //定频模式停止所有录像
            runRecordCfg.recordMode = 3;
            runMdCfg.handle.is_rec = 0;
            printf("enter ding_ping mode\n");
			new_system_call("/mnt/sd_card/ding_ping_config/ding_ping.sh &");
			return 0;
        }
                
        if (access("/mnt/sd_card/wifi.txt",R_OK) == 0)
        {
            printf("wifi file is exist\n");
            wifiMode = 1;

            fp = fopen("/mnt/sd_card/wifi.txt", "r");
            if (fp != NULL)
            {
                getsRet = fgets(wifiInfo, 64, fp);
                if (getsRet != NULL)
                {
                    memset(&info, 0, sizeof(info));
                    getsRet = strstr(wifiInfo, ",");
                    if (getsRet != NULL)
                    {
                        strncpy(info.linkEssid, wifiInfo, strlen(wifiInfo) - strlen(getsRet));
                        strcpy(info.linkPsd, getsRet + 1);
                        getsRet = strstr(info.linkPsd, "\r\n");
                        if (getsRet != NULL)
                        {
                            info.linkPsd[strlen(info.linkPsd) - strlen(getsRet)] = 0;
                        }
                        getsRet = strstr(info.linkPsd, "\n");
                        if (getsRet != NULL)
                        {
                            info.linkPsd[strlen(info.linkPsd) - strlen(getsRet)] = 0;
                        }
                        info.isConnect = 1;
                        printf("sd wifi info, ssid:%s/pwd:%s\n", info.linkEssid, info.linkPsd);
                        _net_add_wifilink_info(&info);
                    }
                }

                if(0 == network_wifi_init(wifiMode))
                {
                    char *WifiName = NULL;
                    WifiName = netcam_net_wifi_get_devname();
                    if(WifiName != NULL && strcmp(WifiName, runNetworkCfg.wifi.netName) != 0)
                    {
                        strncpy(runNetworkCfg.wifi.netName, WifiName, sizeof(runNetworkCfg.wifi.netName));
                        //save wifi netname
                        netcam_net_cfg_save();
                    }
        
#ifdef MODULE_SUPPORT_GB28181
                    if (runNetworkCfg.wireless.mode == 0)
                    {
                        if (!strcmp(runGB28181Cfg.DeviceUID,"0000000000000000000"))
                        {
                            gb28181_load_id();
                        }
        
                        sprintf(ssidName, "IPCamera_%s", runGB28181Cfg.DeviceUID + strlen(runGB28181Cfg.DeviceUID) - 6);
                        printf("set ap ssid:%s\n", ssidName);
                        net_wifi_hostap_set_init_ssid(ssidName);
                    }
#endif
                    network_wifi_on();
        
                }
                fclose(fp);
            }
        }
        else
        {
            printf("wifi file not exist\n");
        }
        
    #ifdef AP_STA_SUPPORT
        WIFI_SCAN_LIST_t list[20];
        int i, number = 20;
        netcam_net_wifi_get_scan_list_ap(list,&number);
    #endif
    }
	return 0;
}

int netcam_net_exit(void)
{
    network_wifi_deinit();
	new_system_call("/usr/bin/killall udhcpc");
	new_system_call("/usr/bin/killall udhcpd");
    return 0;
}

int netcam_net_setwifi(int wifiMode,WIFI_LINK_INFO_t linkInfo)
{
	char *WifiName = NULL;
	ST_SDK_NETWORK_ATTR net_attr;
    int ret = 0;

	WifiName = netcam_net_wifi_get_devname();
	if(WifiName == NULL) {
		PRINT_INFO("Not WiFi device.");
		return -1;
	}

    new_system_call("killall hostapd");
    new_system_call("killall udhcpd");
	if(wifiMode == NETCAM_WIFI_STA)
	{
		netcam_net_wifi_switchWorkMode(NETCAM_WIFI_STA);
		snprintf(net_attr.name,sizeof(net_attr.name), WifiName);
        system("ifconfig");

		netcam_net_get(&net_attr);
        net_attr.dhcp = 1;
		ret = netcam_net_wifi_set_connect_info(&linkInfo);
		if( ret == 0 &&  linkInfo.linkStatus == WIFI_CONNECT_OK )
		{
            netcam_net_set(&net_attr);
		}
		else
		{
            system("ifconfig");
			LOG_INFO("come herrrr\n");
		    return -1;
		}
	}
    else if(wifiMode == NETCAM_WIFI_AP)
	{
		netcam_net_wifi_switchWorkMode(NETCAM_WIFI_AP);
	}

    return 0;
}
