/*******************************************************************************
 *
 * Copyright (c)
 *
 *  \file  pack_upgrade_tool.c
 *
 *  \brief  pack upgrade file to one bin file
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
#include "convert_ascii.h"


/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

#define PROGRAM_NAME        "pack_upgrade_tool.bin"
#define UBOOTENV_MAGIC      "gk_uboot_env"
//#define UBOOTENV_MTD_SIZE   0x20000

extern int conver_big_little_endian(char* input_file, char* output_file);
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

static void help_info(void)
{
    printf("Usage: %s [ -p | -u | -m | -b]  file:<flash_cfg.ini or upgrade.bin>  <mtd_env_size>\n", PROGRAM_NAME);
    
    printf(" -p : pack upgrade.bin by flash_cfg.ini. pack uboot,kernel,rootfs in upgrade.bin\n");
    printf("      e.g %s -p flash_cfg.ini upgrade.bin\n", PROGRAM_NAME);
    
    printf(" -u : unpack one file of upgrade.bin to some image. e.g uboot kernel rootfs...\n");
    printf("      e.g %s -u upgrade.bin\n", PROGRAM_NAME);
    
    printf(" -m : pack one flash size(8MB/16MB/32MB) upgrade.bin. This image no struct header.\n");
    printf("      e.g %s -m flash_cfg.ini upgrade.bin\n", PROGRAM_NAME);
    
    printf(" -b : make uboot env image by config file.\n");
    printf("      e.g %s -b uboot_env_cfg.txt uboot_env.bin 32k/64/128k \n", PROGRAM_NAME);
	printf(" -e : convert file's big to little mode.\n");
    printf("      e.g %s -e source_in.bin conver_out.bin \n", PROGRAM_NAME);

	printf(" -c : convert hex file to ascii,it can used in .c file.\n");
	printf("	  e.g %s -c source_in.bin conver_out.bin \n", PROGRAM_NAME);
	
}

static int test_uboot_crc32(char * env_file, unsigned int mtd_env_size)
{
    int   ret            = 0;
    int   cfg_fd         = -1;
    int   env_fd         = -1;
    int   file_size      = 0;
    int   cnt            = 0;
    unsigned int val     = 0;
    char *pbuf           = NULL;
    char  tmp            = 0;
    char  line_buf[1024] = {0};

    if (NULL == env_file)
    {
        ERROR_PRT("env_file:%p is NULL!\n", env_file);
        return -1;
    }

    env_fd = open(env_file, O_RDONLY);
    if (env_fd < 0)
    {
        ERROR_PRT("open %s ERROR! %s\n", env_file, strerror(errno));
        ret = -1;
        goto make_env_err;
    }

    memset(line_buf, 0, sizeof(line_buf));

    pbuf = malloc(mtd_env_size + 2);
    if (NULL == pbuf)
    {
        ERROR_PRT("malloc pheader->file_size:%u ERROR!\n", file_size);
        ret = -1;
        goto make_env_err;
    }

    ret = read(env_fd, line_buf, 4);
    if (ret <= 0)
    {
        DEBUG_PRT("read %s file_size:%d end! ret:%d %s\n", env_file, file_size, ret, strerror(errno));
        goto make_env_err;
    }

    ret = read(env_fd, pbuf, mtd_env_size - 4);
    if (ret <= 0)
    {
        DEBUG_PRT("read %s file_size:%d end! ret:%d %s\n", env_file, file_size, ret, strerror(errno));
        goto make_env_err;
    }

    DEBUG_PRT("ret : 0x%x \n", ret);
    DEBUG_PRT(" pbuf:\n%s\n", pbuf);

    val = mtd_crc32(0, pbuf, mtd_env_size - 4);

    DEBUG_PRT(" mtd_crc32 val : 0x%x \n", val);

    ret = 0;
make_env_err:
    if (NULL == pbuf)
    {
        free(pbuf);
    }
    if (cfg_fd < 0)
    {
        close(cfg_fd);
    }
    if (env_fd < 0)
    {
        close(env_fd);
    }
    return ret;
}

static int make_uboot_env(char * uboot_cfgfile, char * env_file, unsigned int mtd_env_size)
{
    int   ret            = 0;
    int   cfg_fd         = -1;
    int   env_fd         = -1;
    int   file_size      = 0;
    int   cnt            = 0;
    unsigned int val     = 0;
    char *pbuf           = NULL;
    char  tmp            = 0;
    char  line_buf[1024] = {0};

    if (NULL == uboot_cfgfile || NULL == env_file)
    {
        ERROR_PRT("uboot_cfgfile:%p  env_file:%p is NULL!\n", uboot_cfgfile, env_file);
        return -1;
    }

    cfg_fd = open(uboot_cfgfile, O_RDONLY);
    if (cfg_fd < 0)
    {
        ERROR_PRT("open %s ERROR! %s\n", uboot_cfgfile, strerror(errno));
        return -1;
    }

    env_fd = open(env_file, O_RDWR|O_CREAT);
    if (env_fd < 0)
    {
        ERROR_PRT("open %s ERROR! %s\n", env_file, strerror(errno));
        ret = -1;
        goto make_env_err;
    }

    memset(line_buf, 0, sizeof(line_buf));

    /* 预先填写4 字节的校验位 */
    file_size = 0;
    ret = write(env_fd, line_buf, 4);
    if (ret <= 0)
    {
        ERROR_PRT("write %s error! ret:%d %s\n", env_file, ret, strerror(errno));
        goto make_env_err;
    }
    file_size += ret;

    /* 读取配置信息, 每条命令用 0 隔开 */
    while(file_size < mtd_env_size)
    {
        ret = read(cfg_fd, &tmp, sizeof(tmp));
        if (ret <= 0)
        {
            DEBUG_PRT("read %s file_size:%d end! ret:%d %s\n", uboot_cfgfile, file_size, ret, strerror(errno));
            break;
        }

        if (0x0d == tmp)
            tmp = 0x0;
        else if (0x0a == tmp)
            continue;

        ret = write(env_fd, &tmp, sizeof(tmp));
        if (ret <= 0)
        {
            ERROR_PRT("write %s error! ret:%d %s\n", env_file, ret, strerror(errno));
            goto make_env_err;
        }
        file_size += ret;
    }

    /* 将其余字节用 0 补齐 */
    if (file_size < mtd_env_size)
    {
        while (file_size < mtd_env_size)
        {
            if ((mtd_env_size - file_size) > sizeof(line_buf))
            {
                ret = write(env_fd, line_buf, sizeof(line_buf));
            }
            else
            {
                ret = write(env_fd, line_buf, (mtd_env_size - file_size));
            }
            if (ret <= 0)
            {
                ERROR_PRT("write %s  file_size:%u error! ret:%d %s\n", env_file, file_size, ret, strerror(errno));
                goto make_env_err;
            }
            file_size += ret;
        }
    }

    DEBUG_PRT("write %s  file_size:%u end! ret:%d %s\n", env_file, file_size, ret, strerror(errno));

    pbuf = malloc(mtd_env_size + 2);
    if (NULL == pbuf)
    {
        ERROR_PRT("malloc pheader->file_size:%u ERROR!\n", file_size);
        ret = -1;
        goto make_env_err;
    }

    ret = lseek(env_fd, 4, SEEK_SET);
    if (ret < 0)
    {
        ERROR_PRT("lseek %s ERROR! %s\n", env_file, strerror(errno));
        ret = -1;
        goto make_env_err;
    }

    ret = read(env_fd, pbuf, mtd_env_size - 4);
    if (ret <= 0)
    {
        DEBUG_PRT("read %s file_size:%d end! ret:%d %s\n", uboot_cfgfile, file_size, ret, strerror(errno));
        goto make_env_err;
    }

    val = mtd_crc32(0, pbuf, mtd_env_size - 4);

    DEBUG_PRT(" val : 0x%x \n", val);

    lseek(env_fd, 0, SEEK_SET);
    ret = write(env_fd, &val, sizeof(val));
    if (ret <= 0)
    {
        DEBUG_PRT("read %s file_size:%d end! ret:%d %s\n", uboot_cfgfile, file_size, ret, strerror(errno));
        goto make_env_err;
    }

    ret = 0;
make_env_err:
    if (NULL != pbuf)
    {
        free(pbuf);
    }
    if (cfg_fd > 0)
    {
        close(cfg_fd);
    }
    if (env_fd > 0)
    {
        close(env_fd);
    }
    return ret;
}

int main(int argc,char *argv[])
{
    int  ret      = 0;
    int  i        = 0;
    int  mod_num  = 0;
    unsigned int mtd_env_size = 0;

    if (3 == argc && NULL == strstr(argv[1], "-u"))
    {
        help_info();
        return -1;
    }
    else if ((4 == argc) 
             && NULL == strstr(argv[1], "-p") 
             && NULL == strstr(argv[1], "-m")
			 && NULL == strstr(argv[1], "-e")
			 && NULL == strstr(argv[1], "-c"))
    {
        help_info();
        return -1;
    }
    else if ((5 == argc) && NULL == strstr(argv[1], "-b"))
    {
        help_info();
        return -1;
    }
    else if ((1 == argc) || (2 == argc))
    {
        help_info();
        return -1;
    }

    /* 将 upgrade.bin 打包文件解包成多个镜像, 如: uboot, kernel, rootfs ... */
    if (3 == argc && NULL != strstr(argv[1], "-u"))
    {
        printf("\n argv[1]:%s  argv[2]:%s \n", argv[1], argv[2]);

        ret = verify_upgrade_file(argv[2]);
        if (ret < 0)
        {
            ERROR_PRT("Call verify_upgrade_file:%s error! ret:%d \n", argv[2], ret);
            return -1;
        }

        ret = unpack_upgrade_file(argv[2]);
        if (ret < 0)
        {
            ERROR_PRT("Call unpack_upgrade_file:%s error! ret:%d \n", argv[2], ret);
            return -1;
        }
    }
    else if (4 == argc && NULL != strstr(argv[1], "-p"))
    {
        printf("\n argv[1]:%s  argv[2]:%s  argv[3]:%s \n", argv[1], argv[2], argv[3]);

        ret = pack_upgrade_file(argv[2], argv[3]);
        if (ret < 0)
        {
            ERROR_PRT("Call pack_upgrade_file %s %s error! ret:%d \n", argv[2], argv[3], ret);
            return -1;
        }
    }
    else if (4 == argc && NULL != strstr(argv[1], "-m"))
    {
        printf("\n argv[1]:%s  argv[2]:%s  argv[3]:%s \n", argv[1], argv[2], argv[3]);

        ret = pack_product_file(argv[2], argv[3]);
        if (ret < 0)
        {
            ERROR_PRT("Call pack_product_file %s %s error! ret:%d \n", argv[2], argv[3], ret);
            return -1;
        }
    }
    else if (5 == argc && NULL != strstr(argv[1], "-b"))
    {
        printf("\n argv[1]:%s  argv[2]:%s  argv[3]:%s  argv[4]:%s \n", argv[1], argv[2], argv[3], argv[4]);

        if (NULL != strstr(argv[4], "32k") || NULL != strstr(argv[4], "32K"))
        {
            mtd_env_size = 0x8000;
        }
        else if (NULL != strstr(argv[4], "64k") || NULL != strstr(argv[4], "64K"))
        {
            mtd_env_size = 0x10000;
        }
        else if (NULL != strstr(argv[4], "128k") || NULL != strstr(argv[4], "128K"))
        {
            mtd_env_size = 0x20000;
        }
        else
        {
            ERROR_PRT("Input the mtd size of env error! Just support 32k/64k/128k size. argv[4]:%s\n", argv[4]);
            return -1;
        }
        
        #if 0
        ret = test_uboot_crc32(argv[2], mtd_env_size);
        if (ret < 0)
        {
            ERROR_PRT("Call test_uboot_crc32 %s %s error! ret:%d \n", argv[2], argv[3], ret);
            return -1;
        }
        #else
        ret = make_uboot_env(argv[2], argv[3], mtd_env_size);
        if (ret < 0)
        {
            ERROR_PRT("Call test_uboot_crc32 %s %s error! ret:%d \n", argv[2], argv[3], ret);
            return -1;
        }
        #endif
    }
	else if (4 == argc && NULL != strstr(argv[1], "-c"))
    {
        //printf("\n argv[1]:%s  argv[2]:%s  argv[3]:%s \n", argv[1], argv[2], argv[3]);

        ret = conver_bin_ascii(argv[2], argv[3],0,0);
        if (ret < 0)
        {
            ERROR_PRT("Call conver_bin_ascii %s %s error! ret:%d \n", argv[2], argv[3], ret);
            return -1;
        }
        
        
    }
	else if (4 == argc && NULL != strstr(argv[1], "-e"))
    {
        //printf("\n argv[1]:%s  argv[2]:%s  argv[3]:%s \n", argv[1], argv[2], argv[3]);

        ret = conver_big_little_endian(argv[2], argv[3]);
        if (ret < 0)
        {
            ERROR_PRT("Call conver_big_little_endian %s %s error! ret:%d \n", argv[2], argv[3], ret);
            return -1;
        }
        
        
    }
    else
    {
        help_info();
        return -1;
    }

    return 0;
}


