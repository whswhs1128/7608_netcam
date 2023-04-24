/*
****************************************************************************
** \file      /home/amik/GokeFiles/plist/pl_main.c
**
** \version   $Id: pl_main.c 0 2016-05-09 11:26:41Z dengbiao $
**
** \brief     videc abstraction layer header file.
**
** \attention THIS SAMPLE CODE IS PROVIDED AS IS. GOFORTUNE SEMICONDUCTOR
**            ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**            OMMISSIONS.
**
** (C) Copyright 2015-2016 by GOKE MICROELECTRONICS CO.,LTD
**
****************************************************************************
*/
#include "pl_types.h"
#include "pl_log.h"
#include "pl_list.h"
#include <getopt.h>
#include <unistd.h>

u32 HZ = 100;
s32 gPID = -1;
s32 gDELAY = 5;

static const char *shortOptions = "ht:d:";

static int usage(const char *program_name)
{
    printf("%s Usage:\n", program_name);
    printf(" #%s -t<PID>        -Print thread cpu use rate.\n", program_name);
    printf("     -d[second]     -Print thread cpu delay time.\n");
    printf("\n");
}
static int getStringBySystem(const char *cmd, char *buff, int bufsize)
{
    FILE *fp = NULL;
    LogI("==>cmd: %s",buff);
    if (NULL == (fp = popen(cmd, "r")))
    {
        goto err;
    }
    if (buff != NULL && NULL == fgets(buff, bufsize, fp))
    {
        goto err;
    }
    pclose(fp);
    return 0;
err:
	pclose(fp);
    return -1;
}

int main(int argc, char *argv[])
{
    u32 pl_HZ;
    int option_index, ch;
    char pl_HZ_unit;
    char pl_buffer[256];

    /* params process */
    while ((ch = getopt_long(argc, argv, shortOptions, NULL, &option_index)) != -1)
    {
        switch (ch)
        {
            case 'h': usage(argv[0]);break;
            case 't': gPID = atoi(optarg);break;
            case 'd': gDELAY = atoi(optarg);break;
            default: break;
        }
    }

    if(gPID <= 0){
        usage(argv[0]);
        return -1;
    }

    Log.off();
    #if 0
    getStringBySystem("dmesg|grep sched_clock: ", pl_buffer, sizeof(pl_buffer));
    sscanf(pl_buffer, "[%*f] sched_clock: %*d bits at %d%cHz, resolution %*dns, wraps every %*dms",
        &pl_HZ, &pl_HZ_unit);
    if(pl_HZ_unit == 'M')
        pl_HZ *= 1000 * 1000;
    else if(pl_HZ_unit == 'k')
        pl_HZ *= 1000;
    HZ = pl_HZ;
    #endif
    LogI("PID:%u HZ:%u", gPID, HZ);

    if (pl_lists_init(gPID))
        return(-1);

    if (pl_display_start())
        return(-1);

    while(getchar() != 'q');

    return(0);
}


