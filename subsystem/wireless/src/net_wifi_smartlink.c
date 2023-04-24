#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <signal.h>
#include <stdbool.h>
#include <unistd.h>
#include "net_wifi_commom.h"
#include "net_wifi_smartlink.h"
#include "net_wifi_sta.h"

static bool smartLinkIsRun;
static NETCAM_SmartLinkFuncT smartLinkFunc;
static pthread_t smartlink_pid;
static pthread_rwlock_t smartlink_lock;
static int supportSL= 0;/*0:not support smartlink, 1:support smartlink*/

void _exit_smartlink_proccess(int signal)
{
	WIFI_HANDLE wifi_handle = net_wifi_open();
	if(wifi_handle == NULL){
		NETWORK_PRINT_ERROR("exit smartlink_proccess.");
		smartlink_pid = 0;
		pthread_exit(0);
		return;
	}
	//net_wifi_cli_smartlink_stop(wifi_handle);
	//net_wifi_cli_smartlink_clear(wifi_handle);
	smartLinkFunc.stopSmartLink();
	NETWORK_PRINT_INFO("exit smartlink_proccess.");
	smartlink_pid = 0;
	pthread_exit(0);
}

void *_wifi_smartlink_proccess(void *params)
{
    sdk_sys_thread_set_name("wifi_smartlink");
	WIFI_HANDLE wifi_handle = (WIFI_HANDLE)params;
	int ret;
    char ssid[128];
    char psd[128];
    WIFI_ENC_MODE encMode;
	pthread_detach(pthread_self());
	signal(SIGQUIT, _exit_smartlink_proccess);
	signal(SIGINT, _exit_smartlink_proccess);
	signal(SIGKILL, _exit_smartlink_proccess);
	signal(SIGUSR1, _exit_smartlink_proccess);

	//net_wifi_cli_smartlink_stop(wifi_handle);
	//net_wifi_cli_smartlink_start(wifi_handle, 0xff);//use 0xff, scan all of channels
    smartLinkFunc.stopSmartLink();
    smartLinkFunc.startSmartLink();

	while(smartLinkIsRun){
		/* running programe */
        ret = -1;
		//net_wifi_cli_smartlink_get_result(wifi_handle, &result);
        if(smartLinkFunc.getSmartLinkInfo)
        {
            memset(ssid,0,sizeof(ssid));
            memset(psd,0,sizeof(psd));
            encMode = 0;
            ret = smartLinkFunc.getSmartLinkInfo(ssid,psd,&encMode);
        }
		if(ret == 0) {  //get smartlink info ok
		    
			WIFI_LINK_INFO_t linkInfo;
			strncpy(linkInfo.linkEssid, ssid, sizeof(linkInfo.linkEssid));
			strncpy(linkInfo.linkPsd, psd, sizeof(linkInfo.linkPsd));
            linkInfo.linkScurity = encMode;

            linkInfo.isConnect = 1;
			//net_wifi_cli_smartlink_stop(wifi_handle);//stop scanning elian, exit smartlink
			//net_wifi_cli_smartlink_clear(wifi_handle);
            smartLinkFunc.stopSmartLink();
			//netcam_audio_hint(9);	xqq
			NETWORK_PRINT_INFO("recv ssid:>%s<  pwd:>%s<,encMode:%d\n", ssid, psd,encMode);
            ret = netcam_net_wifi_set_connect_info(&linkInfo);

			if(ret != WIFI_OK) {
				NETWORK_PRINT_ERROR("link error, rescanning result.\n");
				net_wifi_cli_disconnect(wifi_handle);
                smartLinkFunc.startSmartLink();
				//net_wifi_cli_smartlink_start(wifi_handle, 0xff);//use 0xff, scan all of channels
			} else {
				NETWORK_PRINT_INFO("link success, ssid:>%s<  pwd:>%s<\n", ssid, psd);
                netcam_net_reset_net_ip(net_wifi_open(),NULL);
				break;
			}
		}
		sleep(ELIAN_INTVAL_TIME);
	}
	//net_wifi_cli_smartlink_stop(wifi_handle);//stop scanning elian, exit smartlink
	//net_wifi_cli_smartlink_clear(wifi_handle);
    smartLinkFunc.stopSmartLink();

	pthread_rwlock_wrlock(&smartlink_lock);
	smartlink_pid = 0;
	smartLinkIsRun = 0;
	pthread_rwlock_unlock(&smartlink_lock);
	pthread_exit(0);
}

int net_wifi_smartlink_register(NETCAM_SmartLinkFuncT *func)
{
    if(func == NULL || func->startSmartLink == NULL || func->stopSmartLink == NULL || func->getSmartLinkInfo == NULL)
    {
        NETWORK_PRINT_ERROR("register smartlink has null func:%p, start:%p, stop:%p, get:%p\n", func, 
                func->startSmartLink, func->stopSmartLink, func->getSmartLinkInfo);
        return -1;
    }
    memcpy(&smartLinkFunc,func,sizeof(NETCAM_SmartLinkFuncT));

    return 0;
}

int net_wifi_smartlink_start(void)
{
	WIFI_HANDLE wifi_handle;
	if(!supportSL){
		NETWORK_PRINT_ERROR("smart link no support.");
		return -1;
	}
	if(smartlink_pid != 0 || smartLinkIsRun) {
		NETWORK_PRINT_INFO("smart link already run.");
		return 0;
	}

	wifi_handle = net_wifi_open();
	if(wifi_handle == NULL){
		return 0;
	}
    if(smartLinkFunc.startSmartLink == NULL || smartLinkFunc.stopSmartLink == NULL)
    {
        return -1;
    }

	pthread_rwlock_wrlock(&smartlink_lock);
	smartLinkIsRun = 1;
	pthread_rwlock_unlock(&smartlink_lock);
	/* background running protect process */
	if(pthread_create(&smartlink_pid, NULL, _wifi_smartlink_proccess, wifi_handle) != 0)
	{
		NETWORK_PRINT_ERROR("smart link pthread create failed.");
		pthread_rwlock_wrlock(&smartlink_lock);
		smartLinkIsRun = 0;
		pthread_rwlock_unlock(&smartlink_lock);
		return -1;
    }
    return 0;
}

void net_wifi_smartlink_stop(void)
{
    if(smartlink_pid == 0) {
		NETWORK_PRINT_ERROR("smart link pthread is not running.");
		return;
	}
    if(smartLinkFunc.startSmartLink == NULL || smartLinkFunc.stopSmartLink == NULL)
    {
        return ;
    }

    smartLinkFunc.stopSmartLink();

	pthread_rwlock_wrlock(&smartlink_lock);
	smartLinkIsRun = 0;
	pthread_rwlock_unlock(&smartlink_lock);
}

int net_wifi_smartlink_status(void)
{
	//return (smartLinkIsRun != 0);
	if(smartLinkIsRun == 1 || smartlink_pid != 0)
	{
        return 1;
    }
    else
    {
        return 0;
    }
}

int net_wifi_smartlink_support(void)
{
    return supportSL;
}

int net_wifi_smartlink_init(void)
{
    memset(&smartLinkFunc,0,sizeof(smartLinkFunc));
    supportSL = 1;
    pthread_rwlock_init(&smartlink_lock, NULL);
    return 0;
}

int net_wifi_smartlink_deinit(void)
{
    supportSL = 0;
    net_wifi_smartlink_stop();
    memset(&smartLinkFunc,0,sizeof(smartLinkFunc));

    return 0;
}

static int net_smartlink_start()
{
    WIFI_HANDLE wifi_handle = net_wifi_open();
    int ret = net_wifi_cli_smartlink_start(wifi_handle, 0xff);//use 0xff, scan all of channels
    if(ret != 0)
    {
        NETWORK_PRINT_ERROR("net_smartlink_start error:%d\n",ret);
    }
    return ret;
}

static int net_smartlink_stop()
{
    int ret = 0;
    WIFI_HANDLE wifi_handle = net_wifi_open();
    ret = net_wifi_cli_smartlink_stop(wifi_handle);
    //net_wifi_cli_smartlink_clear(wifi_handle);
    return ret;
}

static int net_smartlink_get(char *ssid, char *psd,WIFI_ENC_MODE *ulu_enc)
{
    WIFI_HANDLE wifi_handle = net_wifi_open();
	ELIAN_ResultT result;
    if(ssid == NULL || psd == NULL)
    {
        NETWORK_PRINT_ERROR("net_smartlink_get ssid or psd is NULL\n");
        return -1;
    }
    memset(&result, 0, sizeof(ELIAN_ResultT));
    net_wifi_cli_smartlink_get_result(wifi_handle, &result);
    if(strlen(result.ssid) > 0 || result.cust_data_len > 0) 
    {
        strcpy(ssid, result.ssid);
        strcpy(psd, result.pwd);
        *ulu_enc = strlen(psd) == 0 ? WIFI_ENC_NONE : WIFI_ENC_WPA2PSK_TKIP;
        net_wifi_cli_smartlink_clear(wifi_handle);
        return 0;
    }
    else
    {
        return -1;
    }
}

int net_wifi_smartlink_private()
{
    NETCAM_SmartLinkFuncT func;
    func.getSmartLinkInfo = net_smartlink_get;
    func.startSmartLink = net_smartlink_start;
    func.stopSmartLink = net_smartlink_stop;
    return net_wifi_smartlink_register(&func);
}

