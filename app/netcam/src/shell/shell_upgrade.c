#include <getopt.h>
#include <stdio.h>
#include "shell.h"
#include "mmc_api.h"
#include <errno.h>
#include "netcam_api.h"
#include "avi_rec.h"
#include "flash_ctl.h"

static const char *shortOptions = "s";

static struct option longOptions[] =
{
    {"help",         0, NULL, 'h'},
    {"start",        0, NULL, 's'},
    {0,              0, NULL, 0}
};

static int handle_eraseappcommand(int argc, char* argv[]);
static void shell_eraseappcommand(void);
//static int handle_upgrade_command(int argc, char* argv[]);
static int shell_update(void);


static void Usage(void)
{
    printf("Usage: upgrade [option]\n");
    printf("    -h help.\n");
    printf("    -s start.\n");
//    printf("Example:\n");
//    printf("    #record -IO\n");
//    printf("    #record -S -f /h264/dec_video.h264 -W 1920 -H 1080\n");
}


int upgrade_register_testcase(void)
{
#if 0

    int   retVal =  0;
    (void)shell_registercommand (
        "upgrade",
        handle_upgrade_command,
        "upgrade command",
        "---------------------------------------------------------------------\n"
        "upgrade -s \n"
        "   brief : upgrade start\n"
        "\n"
        /******************************************************************/
    );

    return retVal;
#endif	
}


int eraseapp_register_testcase(void)
{
    int   retVal =  0;
    (void)shell_registercommand (
        "eraseapp",
        handle_eraseappcommand,
        "erase flash app partion command",
        "---------------------------------------------------------------------\n"
        "eraseapp -s \n"
        "   brief : upgrade start\n"
        "\n"
        /******************************************************************/
    );

    return retVal;
}

#define SHELL_MTD_2    "/dev/mtd2"
#define SHELL_MTD_3    "/dev/mtd3"
#define SHELL_MTD_4    "/dev/mtd4"
#define SHELL_MTD_5    "/dev/mtd5"
#define SHELL_MTD_6    "/dev/mtd6"

static void shell_eraseappcommand(void)
{
	int retVal = 0;
    //erase flash
    retVal = erase_flash_mtd(SHELL_MTD_2, 0);
	retVal = erase_flash_mtd(SHELL_MTD_3, 0);
	retVal = erase_flash_mtd(SHELL_MTD_4, 0);
	retVal = erase_flash_mtd(SHELL_MTD_5, 0);
	//retVal = erase_flash_mtd(SHELL_MTD_6, 0);
    if (0 != retVal)
    {
        printf("gadi_flash_erase failed!,retVal = %d\n",(int)retVal);
    }
    else
    {
        printf("erase flash app sector successfully!\n");
    }

	printf("&&&&&&&&&&&&&&&&&&& shell reboot sys &&&&&&&&&&&&&\n");
	//new_system_call("reboot -f");//force REBOOT
	netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT
}


static int handle_eraseappcommand(int argc, char* argv[])
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
                shell_eraseappcommand();
                break;

            default:
                printf("upgrade: bad params\n");
                break;
        }
    }
    optind = 1;
    return 0;
}
#if 0

static int handle_upgrade_command(int argc, char* argv[])
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
                shell_update();
                break;

            default:
                printf("upgrade: bad params\n");
                break;
        }
    }
    optind = 1;
    return 0;
}


static void * shell_upgrade_thread(void *param)
{    
    int ret = 0;
    
    while(SD_STATUS_OK != mmc_get_sdcard_stauts())
    {
        //printf("sd card status:%d\n",mmc_get_sdcard_stauts());
        sleep(1);
    }
        
    if(netcam_get_update_status() < 0)
    {
        printf("is updating..............\n");
        return NULL;
    }
    netcam_update_relase_system_resource();
    ret = netcam_update_for_sdcard();
    if(ret == -1)
    {
        netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT
    }
    return NULL;
}

int shell_update(void)
{
	int ret;

    static pthread_t shellupTid ;
	ret = pthread_create(&shellupTid, NULL, shell_upgrade_thread,NULL);
	if(ret != 0)
	{
		printf("Create shell update thread failed,Please try again:%s\n",strerror(errno));
	}
	else
	{
		 printf("Create shell update thread success\n");
	}

	return ret;
}
#endif


