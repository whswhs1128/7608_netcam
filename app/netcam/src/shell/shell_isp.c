#include <getopt.h>
#include <stdio.h>
#include "shell.h"

#include "cfg_com.h"
#include "netcam_api.h"

static const char *shortOptions = "v:l:f:s:b:c:p:";

static struct option longOptions[] =
{
    {"help",        0, NULL, 'h'},
    {"vin",         1, NULL, 'v'},
    {"lr",          1, NULL, 'l'},
    {"flip",        1, NULL, 'f'},
    {"saturation",  1, NULL, 's'},
    {"brightness",  1, NULL, 'b'},
    {"contrast",    1, NULL, 'c'},
    {"sharpness",   1, NULL, 'p'},
    {0,             0, NULL,   0}
};

static int handle_isp_command(int argc, char* argv[]);

static void Usage(void)
{
    printf("Usage: isp [option]\n");
    printf("    -h help.\n");
    printf("    -v set vin\n");
    printf("    -l set isp lr reverse\n");
    printf("    -f set isp picture flip\n");
    printf("    -s set isp saturation\n");
    printf("    -b set isp brightness\n");
    printf("    -c set isp contrast\n");
    printf("    -p set isp sharpness\n");
    printf("    -H set isp hue\n");
    
//    printf("Example:\n");
//    printf("    #vdec -IO\n");
//    printf("    #vdec -S -f /h264/dec_video.h264 -W 1920 -H 1080\n");
}

int isp_register_testcase(void)
{
    int   retVal =  0;
    (void)shell_registercommand (
        "isp",
        handle_isp_command,
        "isp command",
        "---------------------------------------------------------------------\n"
        "isp -v \n"
        "   brief : set vin\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "isp -l \n"
        "   brief : set isp lr reverse\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "isp -f \n"
        "   brief : set isp picture flip\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "isp -s\n"
        "   brief : set isp saturation\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "isp -b\n"
        "   brief : set isp brightness\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "isp -c\n"
        "   brief : set isp contrast\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "isp -p\n"
        "   brief : set isp sharpness\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "isp -H\n"
        "   brief : set isp hue\n"
        "\n"
        /******************************************************************/
    );

    return retVal;
}

static int handle_isp_command(int argc, char* argv[])
{
    static int refresh_flag = 1;
    int option_index;//ch;
    char ch;
    int vin = 0;
    GK_NET_IMAGE_CFG shellImageAttr;
    netcam_image_get(&shellImageAttr);
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
            case 'v':
                vin = atoi(optarg);
                printf("set vin %d\n",vin);
                break;
            case 'l':
                //refresh_flag = 1;
                shellImageAttr.mirrorEnabled = atoi(optarg);
                printf("set vin  isp lr reverse %d\n",  shellImageAttr.mirrorEnabled);
                break;
            case 'f':
                //refresh_flag = 1;
                shellImageAttr.flipEnabled = atoi(optarg);
                printf("set vin isp picture flip %d\n", vin, shellImageAttr.flipEnabled);
                break;
            case 's':
                //refresh_flag = 1;
                //shellImageAttr.saturation = atoi(optarg);
                //printf("set vin %d isp saturation %d\n", vin, shellImageAttr.sensor[vin].saturation);
                break;
            case 'b':
                //refresh_flag = 1;
                //shellImageAttr.sensor[vin].brightness = atoi(optarg);
                //printf("set vin %d isp brightness %d\n", vin, shellImageAttr.sensor[vin].brightness);
                break;
            case 'c':
                //refresh_flag = 1;
                //shellImageAttr.sensor[vin].contrast = atoi(optarg);
                //printf("set vin %d isp contrast %d\n", vin, shellImageAttr.sensor[vin].contrast);
                break;
            case 'p':
                //refresh_flag = 1;
                //shellImageAttr.sensor[vin].sharpness = atoi(optarg);
                //printf("set vin %d isp sharpness %d\n", vin, shellImageAttr.sensor[vin].sharpness);
                break;
            case 'H':
                //shellImageAttr.sensor[vin].hue = atoi(optarg);
                //printf("set vin %d isp hue %d\n", vin, shellImageAttr.sensor[vin].hue);
            default:
                refresh_flag = 0;
                printf("isp: bad params\n");
                break;
        }
    }
    if(refresh_flag == 0)
    {
        refresh_flag = 1;
        goto exit;
    }
        
    netcam_image_set(shellImageAttr);
    printf("==============set:%d isp param============\n", vin);
exit:
    optind = 1;
    return 0;
}


