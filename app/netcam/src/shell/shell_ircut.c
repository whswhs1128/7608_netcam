#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "shell.h"
#include "cfg_com.h"
#include "netcam_api.h"

static const char *shortOptions = "hd:n:a:";

static struct option longOptions[] =
{
    {"help",        0, NULL, 'h'},
    {"day",         1, NULL, 'd'},
    {"night",       1, NULL, 'n'},
    {"auto",        1, NULL, 'a'},
    {0,             0, NULL, 0}
};

static int handle_ircut_command(int argc, char* argv[]);

static void Usage(void)
{
    printf("Usage: ircut [option]\n");
    printf("    -h help.\n");
    printf("    -d isp daytime model\n");
    printf("    -n isp night model\n");
    printf("    -a isp auto model\n");	
//    printf("Example:\n");
//    printf("    #vdec -IO\n");
//    printf("    #vdec -S -f /h264/dec_video.h264 -W 1920 -H 1080\n");
}

int ircut_register_testcase(void)
{
    int   retVal =  0;
    (void)shell_registercommand (
        "ircut",
        handle_ircut_command,
        "ircut command",
        "---------------------------------------------------------------------\n"
        "ircut -d \n"
        "   brief : isp daytime model\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "ircut -n \n"
        "   brief : isp night model\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "ircut -a \n"
        "   brief : isp auto model\n"
        "\n"
        /******************************************************************/
    );

    return retVal;
}

static int handle_ircut_command(int argc, char* argv[])
{
    GK_NET_IMAGE_CFG imagAttr;

    int option_index;//ch;
    char ch;
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
                Usage();
                break;
            case 'd':
				memset(&imagAttr,0,sizeof(imagAttr));	
				netcam_image_get(&imagAttr);
				imagAttr.irCutControlMode = 0; // 0 software  ,  1 hardware
				imagAttr.irCutMode        = 1;// ircut日夜模式 0 auto, 1 day, 2 night
				netcam_image_set(imagAttr);
                printf("vin ircut day\n");
                break;
            case 'n':
				memset(&imagAttr,0,sizeof(imagAttr));	
				netcam_image_get(&imagAttr);
				imagAttr.irCutControlMode = 0; // 0 software  ,  1 hardware
				imagAttr.irCutMode        = 2;// ircut日夜模式 0 auto, 1 day, 2 night
				netcam_image_set(imagAttr);
				
				printf("vin ircut night\n");
                break;
            case 'a':
				memset(&imagAttr,0,sizeof(imagAttr));	
				netcam_image_get(&imagAttr);
				imagAttr.irCutControlMode = 0; // 0 software  ,  1 hardware
				imagAttr.irCutMode        = 0;// ircut日夜模式 0 auto, 1 day, 2 night
				netcam_image_set(imagAttr);
				printf("vin ircut auto\n");
                break;
            
            default:
                printf("ircut: bad params\n");
                break;
        }
    }
    optind = 1;
    return 0;
}


