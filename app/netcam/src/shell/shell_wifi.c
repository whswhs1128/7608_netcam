#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include "shell.h"
#include "cfg_com.h"

#include "cfg_system.h"
#include "netcam_api.h"

static const char *shortOptions = "s:p:l";

static struct option longOptions[] =
{
    {"help",         0, NULL, 'h'},
    {"ssid",         1, NULL, 'c'},
    {"pwd",          1, NULL, 'p'},
    {"list",         1, NULL, 'l'},
    {0,              0, NULL, 0}
};

static int handle_wifi_command(int argc, char* argv[]);

static void Usage(void)
{
    printf("Usage: wifi [option]\n");
    printf("    -h help.\n");
    printf("    -s wifi ssid\n");
    printf("    -p wifi psd\n");
    printf("    -l print wifi ssid list\n");
//    printf("Example:\n");
//    printf("    #record -IO\n");
//    printf("    #record -S -f /h264/dec_video.h264 -W 1920 -H 1080\n");
}

int wifi_register_testcase(void)
{
    int   retVal =  0;
    (void)shell_registercommand (
        "wifi",
        handle_wifi_command,
        "wifi command",
        "---------------------------------------------------------------------\n"
        "wifi -s \n"
        "   brief : wifi ssid\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "wifi -p \n"
        "   brief : wifi psd\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "wifi -l \n"
        "   brief : print wifi ssid list\n"
        "\n"
        /******************************************************************/
    );

    return retVal;
}

static int handle_wifi_command(int argc, char* argv[])
{
	int ret = 0;
    static int refresh_flag = 1;
    int option_index;//ch;
    char ch;
    int num = 0;
	WIFI_LINK_INFO_t linkInfo;
	memset(&linkInfo,0,sizeof(WIFI_LINK_INFO_t));

    /*change parameters when giving input options.*/
    optind = 1;
    while (1)
    {
        option_index = 0;
        ch = getopt_long(argc, argv, shortOptions, longOptions, &option_index);
		
        if (ch == 255)
    	{
            break;
    	}
        switch (ch)
        {
            case 'h':
            case '?':
                refresh_flag = 0;
                Usage();
                break;
            case 's':
                refresh_flag = 1;
                strncpy(linkInfo.linkEssid, optarg, sizeof(linkInfo.linkEssid));
                printf("wifi ssid %s\n", linkInfo.linkEssid);
                break;
            case 'p':
                refresh_flag = 1;
                strncpy(linkInfo.linkPsd, optarg, sizeof(linkInfo.linkPsd));
                printf("wifi psd %s\n", linkInfo.linkPsd);
                break;
                
            case 'l':
                refresh_flag = 2;
                printf("print wifi ssid list:\n");
                break;

            default:
                refresh_flag = 0;
                printf("wifi: bad params\n");
                break;
        }
    }
    if(refresh_flag == 0)
    {
        refresh_flag = 1;
        goto exit;
    }
    if(refresh_flag == 1){
		printf("sta linkinfo  ssid : %s, psd:%s\n",linkInfo.linkEssid,linkInfo.linkPsd);
		if (strlen(linkInfo.linkPsd) > 0)
			linkInfo.linkScurity = WIFI_ENC_WPAPSK_AES;
		else
			linkInfo.linkScurity = WIFI_ENC_NONE;
		
		linkInfo.isConnect   = 1;	
        ret = netcam_net_setwifi(NETCAM_WIFI_STA,linkInfo);	
		if (ret == 0)
		{
			printf("[%s:%d]shell setwifi sta successful!\n",__func__,__LINE__);
			printf("wifi connect ok");
			goto exit;
		}else if (ret < 0)
		{
			printf("[%s:%d]shell setwifi sta failed! ret = %d\n",__func__,__LINE__,ret);
			return -1;
		}
    }
    if(refresh_flag == 2)
    {
		char *WifiName = NULL;
		WIFI_SCAN_LIST_t list[20];
		int i, number = 20;

		memset(list, 0, sizeof(list));
		WifiName = netcam_net_wifi_get_devname();
		if(WifiName == NULL)
		{
			printf("[%s:%d]WifiName is NULL\n",__func__,__LINE__);
			return -1;
		}

		if(netcam_net_wifi_isOn() != 1)
		{
			printf("[%s:%d]Wifi is not enable\n",__func__,__LINE__);
			return -1;
		}

		netcam_net_wifi_on();
		if(netcam_net_wifi_get_scan_list(list, &number) != 0)
		{
			printf("[%s:%d]Fail to get wifi list\n",__func__,__LINE__);
			return -1;
		}        
		for(i = 0; i < number && list[i].vaild; i++)
		{
			printf("number:%d, scan wifi ssid:[%s]------signal quality:[%d ]\n",i,list[i].essid,list[i].quality);
		}
		
    }

exit:    
    optind = 1;
    return 0;
}


