/*******************************************************************************
 *
 * Copyright (c)
 *
 *  \file  upgrade_flash.c
 *
 *  \brief upgrade file to flash
 * 
 *   create version  v 1.0.0  wangguixing
 *
 ******************************************************************************/

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>

#include "crc32.h"
#include "parser_cfg.h"
#include "flash_ctl.h"


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */




/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */




/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */



/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */



/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */


/******************************************************************************
 * 函数介绍: 
 * 输入参数: 
 *                           
 * 输出参数: 
 * 返回值      : 
 *                               v1.0.0    2015.08.23  wangguixing
 *****************************************************************************/
int main(int argc,char *argv[])
{
    int   ret      = 0;
    int   pack_fd  = -1;
    char *pbuf     = NULL;
    unsigned int  file_size  = 0;

    setvbuf(stdout, NULL, _IOLBF, 0);

    if (2 != argc)
    {
        ERROR_PRT("Do  upgrade_flash error!\n");
        printf("Usage: upgrade_flash upgrade.bin\n");
        return -1;
    }

    #if 1
    pack_fd = open(argv[1], O_RDONLY);
    if (pack_fd < 0)
    {
        ERROR_PRT("open %s ERROR! %s\n", argv[1], strerror(errno));
        return -1;
    }

    file_size = lseek(pack_fd, 0, SEEK_END);
    if (file_size < 0)
	{
        ERROR_PRT("lseek %s ERROR! %s\n", argv[1], strerror(errno));
        close(pack_fd);
        return -1;
    }
    lseek(pack_fd, 0, SEEK_SET);

    pbuf = malloc(file_size + 2);
    if (NULL == pbuf)
    {
        ERROR_PRT("malloc pheader->file_size:%u ERROR!\n", file_size);
        close(pack_fd);
        return -1;
    }

    ret = read(pack_fd, pbuf, file_size);
    if (ret <= 0 || ret != file_size)
    {
        ERROR_PRT("file_size:%u ret:%d %s\n", file_size, ret, strerror(errno));
        free(pbuf);
        close(pack_fd);
        return -1;
    }

    DEBUG_PRT("pbuf:%p, file_size:%u \n", pbuf, file_size);
    
    ret = upgrade_flash_by_buf(pbuf, file_size);
    if (ret < 0)
    {
        ERROR_PRT("Call upgrade_flash_by_buf:%s error! ret:%d \n", argv[1], ret);
        free(pbuf);
        close(pack_fd);
        return -1;
    }

    free(pbuf);
    close(pack_fd);

    #else
    /* 获取打包升级文件对flash 进行升级 */
    ret = upgrade_flash_by_file(argv[1]);
    if (ret < 0)
    {
        ERROR_PRT("Call upgrade_flash_by_file:%s error! ret:%d \n", argv[1], ret);
        return -1;
    }
    #endif

    return 0;
}

