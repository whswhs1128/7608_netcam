#include <getopt.h>
#include <stdio.h>
#include "shell.h"

static const char *shortOptions = "sudlrnf";

static struct option longOptions[] =
{
    {"help",        0, NULL, 'h'},
    {"stop",        0, NULL, 's'},
    {"up",          0, NULL, 'u'},
    {"down",        0, NULL, 'd'},
    {"left",        0, NULL, 'l'},
    {"right",       0, NULL, 'r'},
    {"near",        0, NULL, 'n'},
    {"far",         0, NULL, 'f'},
    {0,             0, NULL,   0}
};

static int handle_ptz_command(int argc, char* argv[]);

static void Usage(void)
{
    printf("Usage: ptz [option]\n");
    printf("    -h help.\n");
    printf("    -s set ptz mv stop\n");
    printf("    -u set ptz mv up\n");
    printf("    -d set ptz mv down\n");
    printf("    -l set ptz mv left\n");
    printf("    -r set ptz mv right\n");
    printf("    -n set ptz zoom near\n");
    printf("    -f set ptz zoom far\n");
    
//    printf("Example:\n");
//    printf("    #vdec -IO\n");
//    printf("    #vdec -S -f /h264/dec_video.h264 -W 1920 -H 1080\n");
}

int ptz_register_testcase(void)
{
    int   retVal =  0;
    (void)shell_registercommand (
        "ptz",
        handle_ptz_command,
        "ptz command",
        "---------------------------------------------------------------------\n"
        "ptz -s \n"
        "   brief : set ptz mv stop\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "ptz -u \n"
        "   brief : set ptz mv up\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "ptz -d \n"
        "   brief : set ptz mv down\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "ptz -l\n"
        "   brief : set ptz mv left\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "ptz -r\n"
        "   brief : set ptz mv right\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "ptz -n\n"
        "   brief : set ptz mv near\n"
        "\n"
        "---------------------------------------------------------------------\n"
        "ptz -f\n"
        "   brief : set ptz mv far\n"
        "\n"
        /******************************************************************/
    );

    return retVal;
}

#define shell_ptz_step     10
#define shell_ptz_speed    5

static int handle_ptz_command(int argc, char* argv[])
{
    int option_index;//ch;
    char ch;
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
                Usage();
                break;
            case 's':
                printf("set ptz mv stop\n");
                netcam_ptz_stop();
                break;
            case 'u':
                printf("set ptz mv up\n");
                netcam_ptz_up(shell_ptz_step, shell_ptz_speed);
                break;
            case 'd':
                printf("set ptz mv down\n");
                netcam_ptz_down(shell_ptz_step, shell_ptz_speed);
                break;
            case 'l':
                printf("set ptz mv left\n");
                netcam_ptz_right(shell_ptz_step, shell_ptz_speed);
                break;
            case 'r':
                printf("set ptz mv right\n");
                netcam_ptz_left(shell_ptz_step, shell_ptz_speed);
                break;
            case 'n':
                printf("set ptz zoom near\n");
#ifdef MODULE_SUPPORT_AF
                sdk_isp_set_af_zoom(2);
#endif
                break;
            case 'f':
                printf("set ptz zoom far\n");
#ifdef MODULE_SUPPORT_AF
                sdk_isp_set_af_zoom(1);
#endif
                break;
            default:
                printf("ptz: bad params\n");
                break;
        }
    }        
    optind = 1;
    return 0;
}


