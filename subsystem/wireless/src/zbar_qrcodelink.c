#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "zbar.h"
//#include "adi_types.h"
//#include "adi_pda.h"
#include <stdint.h>
//#include "common.h"
//#include "adi_sys_error.h"
#include <pthread.h>
#include "sdk_vin.h"
#include "net_wifi_commom.h"
//#include "netcam_api.h"
#include "zbar_qrcodelink.h"

pthread_t ThreadZbarLinkWifi;
static int supportQRCodeLink= 0;/*0:not support QRCodeLink, 1:support QRCodeLink*/
static qrcode_parse_callback qrcode_parse_func = NULL;
static int yuvStreamId = 1;


int zbar_scan_running = 0;
int zbar_qrcode_link_status(void)
{
	//return (smartLinkIsRun != 0);
	if(zbar_scan_running == 1 || ThreadZbarLinkWifi != 0)
	{
        return 1;
    }
    else
    {
        return 0;
    }
}

int zbar_qrcode_link_support(void)
{
    return supportQRCodeLink;
}

int zbar_qrcode_link_init(void)
{
    supportQRCodeLink = 1;
    return 0;
}
int zbar_qrcode_link_deinit(void)
{
    supportQRCodeLink = 0;
    return 0;
}

// data是从二维码中获取的字符串，该函数根据自定义的字符串格式去解析ssid和pwd.
//date    子串格式:
//"XXXX:\r ssid:\r xxxx:\r passwd:\r enmode:\r"
static int sdk_vin_parse_wifi_info(char *data,char *ssid,char *pw,char *enMode)
{
#if 1

	if(qrcode_parse_func != NULL)
		return qrcode_parse_func(data,ssid,pw,enMode);
	
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

#endif
    
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
static int sdk_vin_convert_zbar(int width,int height,char *data,char *ssid,char *pw,char *enMode)
{
	int ret = 0;
    printf(" (c) Goke Microelectronics China 2009 - 2016   \n");
	if(data == NULL)
	{
		printf("param error!");
		return -1;
	}

	zbar_image_scanner_t *scanner = NULL;
    scanner = zbar_image_scanner_create();

    /* configure the reader */
    zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_ENABLE, 1);

    /* obtain image data */
    printf("width:%d,height:%d\n",width,height);
    void *raw = NULL;
    raw = malloc(width*height);
	if(raw == NULL)
	{
		printf("sdk_vin_convert_zbar malloc failed!");
		zbar_image_scanner_destroy(scanner);
		return -1;
	}
	
    memset(raw, 0, width*height);
    memcpy(raw, data, width*height);
	
    printf("size: %d\n", width*height);


    //get_data(argv[1], width, height, &raw);
    /* wrap image data */
    zbar_image_t *image = zbar_image_create();
    zbar_image_set_format(image, *(int*)"Y800");
    zbar_image_set_size(image, width, height);
    zbar_image_set_data(image, raw, width * height, zbar_image_free_data);

    /* scan the image for barcodes */
    ret = zbar_scan_image(scanner, image);

    #if 1
    printf("nsyms size: %d\n", ret);
	if (ret == 0)
	{
		 zbar_image_destroy(image);
    	 zbar_image_scanner_destroy(scanner);
		 return -1;
	}

    #endif
    /* extract results */
    const zbar_symbol_t *symbol = zbar_image_first_symbol(image);

	for(; symbol; symbol = zbar_symbol_next(symbol)) {
        /* do something useful with results */
        zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
        const char *data = zbar_symbol_get_data(symbol);
        printf("decoded %s symbol \"%s\"\n",zbar_get_symbol_name(typ), data);
		ret = sdk_vin_parse_wifi_info(data,ssid,pw,enMode);
    }
    zbar_image_destroy(image);
    zbar_image_scanner_destroy(scanner);
	if (ret != 0)
		return -1;
    return 0;
}

int zbar_vin_scanner(char *ssid,char *pw,char *enMode)
{
    int ret = 0;
    int width, height;
    char* data = NULL;
    ret = sdk_vin_capture_data(0, &data, &width, &height);	//xqq rm yuvchn = 1

    if(ret < 0)
	{
		printf("vin_capture: failed\n");
		return -1;
	}
	else
	{
        ret = sdk_vin_convert_zbar(width, height,data,ssid,pw,enMode);
        //free(data);
		data = NULL;
        if (ret < 0)
        {
            return -1;
        }
	}
	return 0;
}

static void *zbar_thread(void *arg)
{
    int ret = 0;
    char psd[256] = {0};
    char ssid[256] = {0};
    char enMode[1] = {0};
    
	pthread_detach(pthread_self());

	WIFI_HANDLE wifi_handle = (WIFI_HANDLE)arg;

    while(zbar_scan_running)
    {
        ret = zbar_vin_scanner(ssid,psd,enMode);
        if(ret != 0)
        {
            printf("---zbar scaning ,scan failed----\n");
            usleep(2000000);
            continue;
        }
        WIFI_LINK_INFO_t linkInfo;
        memset(&linkInfo, 0, sizeof(linkInfo));
        strncpy(linkInfo.linkEssid, ssid, sizeof(linkInfo.linkEssid));
        strncpy(linkInfo.linkPsd, psd, sizeof(linkInfo.linkPsd));
        linkInfo.linkScurity = atoi(enMode);
        //printf("linkScurity:%d\n",linkInfo.linkScurity);
        linkInfo.isConnect = 1;

        //netcam_audio_hint(9); 	xqq
        printf("recv ssid:>%s<  pwd:>%s<,encMode:%d\n", ssid, psd, linkInfo.linkScurity);
        int old_wifi_mode = netcam_net_wifi_getWorkMode();
        printf("old_wifi_mode:%s\n",(old_wifi_mode == 1) ? "sta":"ap");
        ret = netcam_net_setwifi(1,linkInfo);

        if (ret <0)
        {
            sleep(5);
            
            //netcam_audio_hint(1);  xqq
            printf("failed to set sta wifi parament ,back to AP mode!%d\n", old_wifi_mode);
            if(old_wifi_mode == 2)
            {
                netcam_net_setwifi(2,linkInfo);
            }
        }
        else
        {
            //netcam_audio_hint(0);  xqq
            #if 0
            netcam_net_get(&net_attr);
            //netcam_audio_hint(SYSTEM_AUDIO_HINT_USE_WIRELESS);  xqq
            //netcam_audio_ip(net_attr.ip);	xqq
            #endif
            break;
        }

#if 0
        ret = netcam_net_wifi_set_connect_info(&linkInfo);

        //连接失败，则继续循环
        if(ret != WIFI_OK) 
        {
            printf("link error, rescanning result.\n");
            net_wifi_cli_disconnect(wifi_handle);
            
        } 
        else //连接成功，则退出循环，复位ip
        {
            printf("link success, ssid:>%s<  pwd:>%s<\n", ssid, psd);
            netcam_net_reset_net_ip(net_wifi_open(),NULL);
            break;
        }
#endif
        printf("---zbar scaning connect failed\n----\n");
        usleep(2000000);
        
    }

    ThreadZbarLinkWifi = 0;

    zbar_scan_running = 0;

    pthread_exit(0);
}

int zbar_qrcode_link_start()
{
    static WIFI_HANDLE wifi_handle;

	wifi_handle = net_wifi_open();
	if(wifi_handle == NULL){
		return 0;
	}
    printf("zbar_qrcode_link_start \n");
    zbar_scan_running = 1;
	if( pthread_create(&ThreadZbarLinkWifi, NULL, &zbar_thread, wifi_handle)!= 0)
	{
	    zbar_scan_running = 0;
		printf("pthread_create failed\n");
		return -1;
	}

}

void zbar_qrcode_link_stop()
{
    if(ThreadZbarLinkWifi == 0) 
    {
		printf("zbar scan pthread is not running.");
		return;
	}
    zbar_scan_running = 0;

    
}

void zbar_qrcode_set_parse_callback(qrcode_parse_callback parse_func)
{
    qrcode_parse_func = parse_func;
}

void zbar_qrcode_set_yuv_channel(int streamId)
{
    yuvStreamId = streamId;
}

