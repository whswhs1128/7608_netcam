/*******************************************************************************
 *
 * Copyright (c)
 *
 *  \file parser_cfg.c
 *
 *  \brief parse config file.
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
#include <stdarg.h>
#include <stdint.h>
#include <libgen.h>
#include <time.h>
#include <getopt.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/mount.h>
#include <sys/time.h>

#include "utility_api.h"
#include "parser_cfg.h"
#include "flash_ctl.h"
#include "md5.h"

#include "sdk_cfg.h"
#include "cfg_system.h"
#include "partition_info.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
#define WR_FLASH_BPMS 98     /* 擦写flash 的数度, 单位字节每毫秒. */

#define FORCE_UPDATE_IMAGE_VERSION          "FFFFFFFF"

/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */

/* 结构体用于读取 flash_map.ini 配置表的 [FIRMWARE]  段选项 */
struct firmware_info
{
    unsigned int flash_size;    /* 整个flash 容量大小 */
    unsigned int upgrade_num;   /* 需要升级的MTD 分区个数, 注意不是flash上所有的MTD分区个数 */
    char  magic_num[32];        /* 幻数 */
    char  image_ver[32];        /* 镜像版本号 */
    char  Platform[32];         /* 平台类型 */
    char  machine_type[32];     /* 产品类型 */
};


/* 结构体用于读取 flash_map.ini 配置表的 [FLASH] 段选项 */
struct flash_info
{
    unsigned int mtd_addr;      /* (16进制) 该区相对整个flash空间0地址的偏移地址 */
    unsigned int mtd_size;      /* (16进制)要升级的MTD 所占分区的大小 */
    unsigned int mtd_offset;    /* 要升级的所在分区内的便宜地址, 而非整体flash的偏移地址 */
    unsigned int block;         /* 要升级的块数 (暂时没有使用) */
    unsigned int upgrade_flag;  /* 升级标志, 0:表示不升级; 1:表示升级  */
    int   format_bin;           /* 制作升级程序格式是否为 bin   (暂时没有使用) */
    int   format_rom;           /* 制作升级程序格式是否为 rom   (暂时没有使用) */
    char  mtd_part[32];         /* 要升级的MTD 分区如: /dev/mtd3   */
    char  mtd_name[32];         /* 要升级的MTD 分区名如:  rootfs  */
    char  upgrade_file[96];     /* 需要升级的文件地址路径 */
};


/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */



/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */

static int       g_upgrade_start_flag = 0;
static long int  g_start_time         = 0;
static long int  g_upgrade_total_time = 0;
static unsigned int g_upgrade_rate    = 0;


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

static void set_upgrade_start(unsigned int file_size)
{
    struct timeval cur_time;
    memset(&cur_time, 0, sizeof(cur_time));

    g_upgrade_rate         = 0;
    g_upgrade_start_flag   = 1;
    g_upgrade_total_time = file_size / WR_FLASH_BPMS;

    gettimeofday(&cur_time, NULL);
    g_start_time = (cur_time.tv_sec * 1000) + (cur_time.tv_usec / 1000);
    DEBUG_PRT("g_start_time:%ldms  tv_sec:%ld  tv_usec:%ld \n",
                                              g_start_time,
                                              cur_time.tv_sec,
                                              cur_time.tv_usec);
}

void set_upgrade_end(void)
{
    g_upgrade_rate       = 100;
    g_upgrade_start_flag = 0;
    DEBUG_PRT("upgrade flash end! rate:100%%\n");
}

/******************************************************************************
 * 函数介绍:  获取当前进度百分比
 * 输入参数:  无
 *
 * 输出参数:  0--100的值. 即表示 0%--100%
 * 返回值      :
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
unsigned int get_upgrade_rate(void)
{
    long int       tmp_time     = 0;
    long int       user_time    = 0;
    struct timeval cur_time;
    memset(&cur_time, 0, sizeof(cur_time));

    if (g_upgrade_start_flag)
    {
        gettimeofday(&cur_time, NULL);
        tmp_time = (cur_time.tv_sec * 1000) + (cur_time.tv_usec / 1000);
        if (tmp_time >= g_start_time)
        {
            user_time    = tmp_time - g_start_time;
            g_upgrade_rate = (user_time * 100) / g_upgrade_total_time;

            /* 由于完成的进度是时间估算的, 实际完成需要根据
                       * set_upgrade_end 来确认完成时间.因此估算比例最大不能为100%
                       */
            if (g_upgrade_rate > 98)
            {
                g_upgrade_rate = 98;
            }
        }
        else
        {
            g_start_time = tmp_time;
        }
    }

    DEBUG_PRT("Current upgrade flag:%d total:%ld  start:%ld  curr:%ld  used:%ld  rate:%u%% \n",
                                                g_upgrade_start_flag,
                                                g_upgrade_total_time,
                                                g_start_time,
                                                tmp_time,
                                                user_time,
                                                g_upgrade_rate);

    if (g_upgrade_rate > 100)
    {
        g_upgrade_rate = 100;
    }

    return g_upgrade_rate;
}

static int printf_md5(const char *md5, char *md5str)
{
    if (NULL == md5||NULL == md5str)
    {
        ERROR_PRT("md5 is NULL error!\n");
        return -1;
    }
    int i;
    for(i=0; i<16; i++)
    {
        sprintf(md5str+i*2,"%02hhx", md5[i]);
    }
    return 0;
}

/******************************************************************************
 * 函数介绍:
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
static int printf_upgrade_header_info(const struct gk_upgrade_header *pheader)
{
    if (NULL == pheader)
    {
        ERROR_PRT("pheader is NULL error!\n");
        return -1;
    }
    char md5str[64] = {0};
    printf_md5(pheader->pack_md5, md5str);

    DEBUG_PRT("--> pheader->magic_num   :%s \n",   pheader->magic_num);
    DEBUG_PRT("--> pheader->image_ver   :%s \n",   pheader->image_ver);
    DEBUG_PRT("--> pheader->Platform    :%s \n",   pheader->Platform);
    DEBUG_PRT("--> pheader->machine_type:%s \n",   pheader->machine_type);
    DEBUG_PRT("--> pheader->flash_size  :0x%x \n", pheader->flash_size);
    DEBUG_PRT("--> pheader->upgrade_flag:%d \n",   pheader->upgrade_flag);
    DEBUG_PRT("--> pheader->mtd_part    :%s \n",   pheader->mtd_part);
    DEBUG_PRT("--> pheader->mtd_name    :%s \n",   pheader->mtd_name);
    DEBUG_PRT("--> pheader->mtd_addr    :0x%x \n", pheader->mtd_addr);
    DEBUG_PRT("--> pheader->mtd_offset  :0x%x \n", pheader->mtd_offset);
    DEBUG_PRT("--> pheader->mtd_size    :0x%x \n", pheader->mtd_size);
    DEBUG_PRT("--> pheader->file_size   :0x%x \n", pheader->file_size);
    DEBUG_PRT("--> pheader->file_crc32  :%x \n",   pheader->file_crc32);
	DEBUG_PRT("--> pheader->pack_md5    :%s \n\n", md5str);

    return 0;
}


/******************************************************************************
 * 函数介绍:
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
static int parse_firmware_info(char *file_name, struct firmware_info *pfw_info)
{
    char  *pret     = NULL;
    FILE  *fp       = NULL;
    char   line_buf[256]  = {0};
    int    parse_flag  = 0;

    if (NULL == file_name || NULL == pfw_info)
    {
        ERROR_PRT("file_name:%p  pfw_info:%p error!\n", file_name, pfw_info);
        return -1;
    }

    fp = fopen(file_name, "r");
    if (NULL == fp)
    {
        ERROR_PRT("fopen %s ERROR! %s\n", file_name, strerror(errno));
        return -1;
    }

    parse_flag  = 0;
    while(1)
    {
        memset(line_buf, 0, sizeof(line_buf));

        /* read flash_map.ini  file */
        pret = fgets(line_buf, sizeof(line_buf) - 1, fp);
        if(NULL == pret)
        {
            /* 没有解析到[FIRMWARE_END] 就结束了, 说明配置文件有问题 */
            ERROR_PRT("The section if not [FIRMWARE_END] end! error!\n");
            fclose(fp);
            return -1;
        }

        /* 1. 解析FIRMWARE 的开始段 */
        if (NULL != (pret = strstr(line_buf, FIRMWARE_START)))
        {
            if (0 == parse_flag)
            {
                DEBUG_PRT("==========  parse [FIRMWARE_END] section start  =========\n");
                parse_flag = 1;
                continue;
            }
            else
            {
                /* 两次解析到[FIRMWARE] 报错 */
                ERROR_PRT("twice parsed [FIRMWARE] section error!\n");
                fclose(fp);
                return -1;
            }
        }

        /* 2. 解析FIRMWARE 的结束段 */
        if (NULL != (pret = strstr(line_buf, FIRMWARE_END)))
        {
            DEBUG_PRT("==========  parse [FIRMWARE_END] section end!  =========\n\n");
            fclose(fp);
            return 0;
        }

        /* 3. 当解析到[FIRMWARE] 后开始解析每个字段的值 */
        if (parse_flag)
        {
            if (NULL != (pret = strstr(line_buf, "magic=")))
            {
                sscanf(line_buf, "%*[^=]=%s", pfw_info->magic_num);
                DEBUG_PRT("parse [magic_num]:\t\t%s\n", pfw_info->magic_num);

                /* 校验幻数 check magic_num  */
                if (NULL == strstr(pfw_info->magic_num, MAGIC_NUM))
                {
                    ERROR_PRT("check magic_num error! ok_magic is [%s]\n", MAGIC_NUM);
                    fclose(fp);
                    return 0;
                }
            }
            else if (NULL != (pret = strstr(line_buf, "Platform=")))
            {
                sscanf(line_buf, "%*[^=]=%s", pfw_info->Platform);
                DEBUG_PRT("parse [Platform]:\t\t%s\n", pfw_info->Platform);
            }
            else if (NULL != (pret = strstr(line_buf, "machine_type=")))
            {
                sscanf(line_buf, "%*[^=]=%s", pfw_info->machine_type);
                DEBUG_PRT("parse [machine_type]:\t%s\n", pfw_info->machine_type);
            }
            else if (NULL != (pret = strstr(line_buf, "image_ver=")))
            {
                sscanf(line_buf, "%*[^=]=%s", pfw_info->image_ver);
                DEBUG_PRT("parse [image_ver]:\t\t%s\n", pfw_info->image_ver);
            }
            else if (NULL != (pret = strstr(line_buf, "flash_size=")))
            {
                sscanf(line_buf, "%*[^=]=%x", &(pfw_info->flash_size));
                DEBUG_PRT("parse [flash_size]:\t\t0x%x (%dKB)\n", pfw_info->flash_size, pfw_info->flash_size/1024);
                /* nor_flash size 必须大于2MB  */
                if(pfw_info->flash_size < MIMI_NORFLASH_SIZE
                   || pfw_info->flash_size > MAX_NORFLASH_SIZE)
                {
                    ERROR_PRT("flash_size 0x%x (%dKB) error!\n",
                                                    pfw_info->flash_size, pfw_info->flash_size/1024);
                    fclose(fp);
                    return -1;
                }
            }
            else if (NULL != (pret = strstr(line_buf, "upgrade_num=")))
            {
                sscanf(line_buf, "%*[^=]=%d", &(pfw_info->upgrade_num));
                DEBUG_PRT("parse [upgrade_num]:\t%d\n", pfw_info->upgrade_num);
                /* upgrade_num 不能大于16个分区  */
                if(pfw_info->upgrade_num > MAX_MTD_NUM)
                {
                    ERROR_PRT("upgrade_num:%u is too more MAX_MTD_NUM[%d]! error! \n", pfw_info->upgrade_num, MAX_MTD_NUM);
                    fclose(fp);
                    return -1;
                }
            }
            else
            {
                ERROR_PRT("line:%s is don't parse!\n", line_buf);
            }
        }
    }

    fclose(fp);
    return 0;
}


/******************************************************************************
 * 函数介绍:
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
static int parse_flash_info(char *file_name, char *flash_start, char *flash_end, struct flash_info *pflash)
{
    char  *pret     = NULL;
    FILE  *fp       = NULL;
    char   line_buf[256]  = {0};
    int    parse_flag  = 0;

    if (NULL == file_name || NULL == pflash || NULL == flash_start || NULL == flash_end)
    {
        ERROR_PRT("file_name:%p  pflash:%p  flash_start:%p  flash_end:%p error!\n", file_name, pflash, flash_start, flash_end);
        return -1;
    }

    fp = fopen(file_name, "r");
    if (NULL == fp)
    {
        ERROR_PRT("fopen %s ERROR!\n", file_name);
        return -1;
    }

    parse_flag  = 0;
    while(1)
    {
        memset(line_buf, 0, sizeof(line_buf));

        /* read flash_map.ini  file */
        pret = fgets(line_buf, sizeof(line_buf) - 1, fp);
        if(NULL == pret)
        {
            /* 没有解析到[FIRMWARE_END] 就结束了, 说明配置文件有问题 */
            ERROR_PRT("The section if not [FIRMWARE_END] end! error!\n");
            fclose(fp);
            return -1;
        }

        /* 1. 解析[FLASH] 的开始段 */
        if (NULL != (pret = strstr(line_buf, flash_start)))
        {
            if (0 == parse_flag)
            {
                DEBUG_PRT("==========  parse %s section start  =========\n", flash_start);
                parse_flag = 1;
                continue;
            }
            else
            {
                /* 两次解析到[FLASH] 报错 */
                ERROR_PRT("twice parsed [FIRMWARE] section error!\n");
                fclose(fp);
                return -1;
            }
        }

        /* 2. 解析[FLASH_END] 的结束段 */
        if (NULL != (pret = strstr(line_buf, flash_end)))
        {
            DEBUG_PRT("==========  parse %s section end  =========\n\n", flash_end);
            fclose(fp);
            return 0;
        }

        /* 3. 当解析到[FLASH] 后开始解析每个字段的值 */
        if (parse_flag)
        {
            if (NULL != (pret = strstr(line_buf, "mtd_part=")))
            {
                sscanf(line_buf, "%*[^=]=%s", pflash->mtd_part);
                DEBUG_PRT("parse [mtd_part]:\t%s\n", pflash->mtd_part);
            }
            else if (NULL != (pret = strstr(line_buf, "mtd_name=")))
            {
                sscanf(line_buf, "%*[^=]=%s", pflash->mtd_name);
                DEBUG_PRT("parse [mtd_name]:\t%s\n", pflash->mtd_name);
            }
            else if (NULL != (pret = strstr(line_buf, "mtd_addr=")))
            {
                sscanf(line_buf, "%*[^=]=%x", &(pflash->mtd_addr));
                DEBUG_PRT("parse [mtd_addr]:\t0x%x (%dKB)\n", pflash->mtd_addr, pflash->mtd_addr/1024);
                /* pflash->mtd_addr 不能大于32MB  */
                if(pflash->mtd_addr >= MAX_MTD_SIZE)
                {
                    ERROR_PRT("upgrade address is too longht, error! 0x%x (%dKB)\n",
                                                    pflash->mtd_addr, pflash->mtd_addr/1024);
                    fclose(fp);
                    return -1;
                }
            }
            else if (NULL != (pret = strstr(line_buf, "mtd_size=")))
            {
                sscanf(line_buf, "%*[^=]=%x", &(pflash->mtd_size));
                DEBUG_PRT("parse [mtd_size]:\t0x%x (%dKB)\n", pflash->mtd_size, pflash->mtd_size/1024);
                /* pflash->mtd_size 不能大于32MB  */
                if(pflash->mtd_size > MAX_MTD_SIZE)
                {
                    ERROR_PRT("upgrade address is too longht, error! 0x%x (%dKB)\n",
                                                    pflash->mtd_size, pflash->mtd_size/1024);
                    fclose(fp);
                    return -1;
                }
            }
            else if (NULL != (pret = strstr(line_buf, "mtd_offset=")))
            {
                sscanf(line_buf, "%*[^=]=%x", &(pflash->mtd_offset));
                DEBUG_PRT("parse [mtd_offset]:\t0x%x (%dKB)\n", pflash->mtd_offset, pflash->mtd_offset/1024);
                /* pflash->mtd_offset 不能大于32MB  */
                if(pflash->mtd_offset > MAX_MTD_SIZE)
                {
                    ERROR_PRT("upgrade address is too longht, error! 0x%x (%dKB)\n",
                                                    pflash->mtd_offset, pflash->mtd_offset/1024);
                    fclose(fp);
                    return -1;
                }
            }
            else if (NULL != (pret = strstr(line_buf, "block=")))
            {
                sscanf(line_buf, "%*[^=]=%x", &(pflash->block));
                DEBUG_PRT("parse [block]:\t\t%d\n", pflash->block);
            }
            else if (NULL != (pret = strstr(line_buf, "upgrade_file=")))
            {
                sscanf(line_buf, "%*[^=]=%s", pflash->upgrade_file);
                DEBUG_PRT("parse [upgrade_file]:\t%s\n", pflash->upgrade_file);
            }
            else if (NULL != (pret = strstr(line_buf, "format_rom=")))
            {
                sscanf(line_buf, "%*[^=]=%d", &(pflash->format_rom));
                DEBUG_PRT("parse [format_rom]:\t0x%x\n", pflash->format_rom);
            }
            else if (NULL != (pret = strstr(line_buf, "format_bin=")))
            {
                sscanf(line_buf, "%*[^=]=%d", &(pflash->format_bin));
                DEBUG_PRT("parse [format_bin]:\t0x%x\n", pflash->format_bin);
            }
            else if (NULL != (pret = strstr(line_buf, "upgrade_flag=")))
            {
                sscanf(line_buf, "%*[^=]=%x", &(pflash->upgrade_flag));
                DEBUG_PRT("parse [upgrade_flag]:\t%d\n", pflash->upgrade_flag);
            }
            else
            {
                ERROR_PRT("line:%s is don't parse!\n", line_buf);
            }
        }
    }

    fclose(fp);
    return 0;
}


/******************************************************************************
 * 函数介绍:
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
static int get_upgradeinfo_by_cfgfile(char *file_name, struct firmware_info *pfw, struct flash_info *pfh)
{
    int    ret      = 0;
    int    cnt      = 0;
    char   start_line[32] = {0};
    char   end_line[32]   = {0};

    if (NULL == file_name || NULL == pfw || NULL == pfh)
    {
        ERROR_PRT("file_name:%p  pfw:%p  pfh:%p  \n", file_name, pfw, pfh);
        return -1;
    }

    ret = parse_firmware_info(file_name, pfw);
    if (ret < 0)
    {
        ERROR_PRT("Do parse_firmware_info error!  ret:%d\n", ret);
        return -1;
    }

    for (cnt = 0; cnt < pfw->upgrade_num; cnt++)
    {
        memset(start_line, 0, sizeof(start_line));
        memset(end_line,   0, sizeof(end_line));
        sprintf(start_line, "[FLASH%d]",     cnt);
        sprintf(end_line,   "[FLASH%d_END]", cnt);
        ret = parse_flash_info(file_name, start_line, end_line, (pfh+cnt));
        if (ret < 0)
        {
            ERROR_PRT("parse %s  section:%s error! ret:%d \n", file_name, start_line, ret);
            return -1;
        }
    }

    return 0;
}


/******************************************************************************
 * 函数介绍:
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int get_mtdinfo_by_proc_mtd(struct proc_mtd_info *pmtd_info,  int *pmtd_num)
{
    int    cnt  = 0;
    FILE  *fp   = NULL;
    char  *pret = NULL;
    char   line_buf[256];
    char   name[32]    = {0};
    char   mtd_part[32] = {0};
    signed int size = 0;
    signed int erase_size = 0;
    signed int addr_offset = 0;

    if (NULL == pmtd_info || NULL == pmtd_num)
    {
        ERROR_PRT("mtd_info:%p  mtd_num:%p is NULL !\n", pmtd_info, pmtd_num);
        return -1;
    }

    fp = fopen(MTD_PATH, "r");
    if (NULL == fp)
    {
        ERROR_PRT("fopen %s ERROR!\n", MTD_PATH);
        return -1;
    }

    while(1)
    {
        memset(line_buf, 0, sizeof(line_buf));
        memset(name,     0, sizeof(name));
        memset(mtd_part, 0, sizeof(mtd_part));

        /* read /proc/mtd file end or error! */
        pret = fgets(line_buf, sizeof(line_buf) - 1, fp);
        if(NULL == pret)
            break;

        /* filter title line: "dev:    size   erasesize  name"  */
        if (NULL == (pret = strstr(line_buf, "mtd")))
            continue;

        cnt++;
        if (cnt >= MAX_MTD_NUM)
        {
            ERROR_PRT("MTD partition more than %d, error!\n", MAX_MTD_NUM);
            fclose(fp);
            return -1;
        }

        /* parse the line: " mtd0: 00030000 00010000 "uboot" "  */

        sscanf(line_buf, "%[^:]: %x %x%*[^\"]\"%[^\"]", mtd_part, &size, &erase_size, name);
        pmtd_info[cnt].mtd_size = size;
        pmtd_info[cnt].mtd_addr = addr_offset;
        strcpy(pmtd_info[cnt].mtd_part, "/dev/");
        strcat(pmtd_info[cnt].mtd_part, mtd_part);
        strncpy(pmtd_info[cnt].mtd_name, name, sizeof(name)-1);
        addr_offset += size;

        DEBUG_PRT("cnt[%d]  mtd_part:%s  size:0x%x  mtd_addr:0x%x  name:%s \n",
                                                    cnt, pmtd_info[cnt].mtd_part,
                                                    pmtd_info[cnt].mtd_size,
                                                    pmtd_info[cnt].mtd_addr,
                                                    pmtd_info[cnt].mtd_name);
    }

    *pmtd_num = cnt;
    DEBUG_PRT("The total mtd_num:%d \n\n", cnt);

    fclose(fp);
    return 0;
}


/******************************************************************************
 * 函数介绍:
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int get_all_mtd_info_by_proc(struct proc_mtd_info *pall_mtd)
{
    int    cnt  = 0;
    FILE  *fp   = NULL;
    char  *pret = NULL;
    char   line_buf[256];
    char   name[32]    = {0};
    char   mtd_part[32] = {0};
    signed int size = 0;
    signed int erase_size = 0;

    if (NULL == pall_mtd)
    {
        ERROR_PRT("pall_mtd:%p is NULL !\n", pall_mtd);
        return -1;
    }

    fp = fopen(MTD_PATH, "r");
    if (NULL == fp)
    {
        ERROR_PRT("fopen %s ERROR!\n", MTD_PATH);
        return -1;
    }

    while(1)
    {
        memset(line_buf, 0, sizeof(line_buf));
        memset(name,     0, sizeof(name));
        memset(mtd_part, 0, sizeof(mtd_part));

        /* read /proc/mtd file end or error! */
        pret = fgets(line_buf, sizeof(line_buf) - 1, fp);
        if(NULL == pret)
        {
            ERROR_PRT("Don't find the all_mtd partition in /proc/mtd .\n");
            fclose(fp);
            return -1;
        }

        /* filter title line: "dev:    size   erasesize  name"  */
        if (NULL == (pret = strstr(line_buf, "mtd")))
            continue;

        cnt++;
        if (cnt >= MAX_MTD_NUM)
        {
            ERROR_PRT("MTD partition more than %d, error!\n", MAX_MTD_NUM);
            fclose(fp);
            return -1;
        }

        /* parse the line:  mtd0: 00030000 00010000 "all "  */
        if (NULL != strstr(line_buf, "all"))
        {
            sscanf(line_buf, "%[^:]: %x %x%*[^\"]\"%[^\"]", mtd_part, &size, &erase_size, name);
            pall_mtd->mtd_size = size;
            pall_mtd->mtd_addr = 0;
            strcpy(pall_mtd->mtd_part, "/dev/");
            strcat(pall_mtd->mtd_part, mtd_part);
            strncpy(pall_mtd->mtd_name, name, sizeof(name)-1);

            DEBUG_PRT("cnt[%d]  all_mtd_part:%s  size:0x%x  mtd_addr:0x%x  name:%s \n",
                                                        cnt, pall_mtd->mtd_part,
                                                        pall_mtd->mtd_size,
                                                        pall_mtd->mtd_addr,
                                                        pall_mtd->mtd_name);
            fclose(fp);
            return 0;
        }
    }

    fclose(fp);
    return 0;
}


/******************************************************************************
 * 函数介绍:
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
static int fill_upgrade_header(struct firmware_info *pfw, struct flash_info *pfh, struct gk_upgrade_header *pheader)
{
    int    ret  = 0;
    int    fd   = -1;
    char  *pbuf = NULL;
    unsigned int tmp_size = 0;
    MD5_CTX md5;

    memset(&md5, 0, sizeof(md5));
    if (NULL == pfw || NULL == pfh || NULL == pheader)
    {
        ERROR_PRT("pfw:%p  pfh:%p  pheader:%p is NULL!\n", pfw, pfh, pheader);
        return -1;
    }

    if (pfw->upgrade_num > MAX_MTD_NUM)
    {
        ERROR_PRT("MTD upgrade_num:%d more than %d, error!\n", pfw->upgrade_num, MAX_MTD_NUM);
        return -1;
    }

    if (NULL == pfh->upgrade_file)
    {
        ERROR_PRT("pfh->upgrade_file is NULL!\n");
        return -1;
    }

    strncpy(pheader->magic_num,    pfw->magic_num,    sizeof(pheader->magic_num)-1);
    strncpy(pheader->image_ver,    pfw->image_ver,    sizeof(pheader->image_ver)-1);
    strncpy(pheader->Platform,     pfw->Platform,     sizeof(pheader->Platform)-1);
    strncpy(pheader->machine_type, pfw->machine_type, sizeof(pheader->machine_type)-1);
    pheader->flash_size = pfw->flash_size;

    memcpy(pheader->pack_md5, pheader->machine_type, sizeof(pheader->pack_md5));
    strncpy(pheader->mtd_part,  pfh->mtd_part, sizeof(pheader->mtd_part)-1);
    strncpy(pheader->mtd_name,  pfh->mtd_name, sizeof(pheader->mtd_name)-1);
    pheader->mtd_addr     = pfh->mtd_addr;
    pheader->mtd_offset   = pfh->mtd_offset;
    pheader->mtd_size     = pfh->mtd_size;
    pheader->upgrade_flag = pfh->upgrade_flag;

    if (pfw->flash_size > MAX_MTD_SIZE)
    {
        ERROR_PRT("pfw->flash_size:0x%x is too large! MAX_MTD_SIZE:0x%x\n", pfw->flash_size, MAX_MTD_SIZE);
        return -1;
    }

    tmp_size = pfh->mtd_addr + pfh->mtd_offset + pfh->mtd_size;
    if (tmp_size > pfw->flash_size)
    {
        ERROR_PRT("mtd_name:%s mtd_addr:0x%x + mtd_offset:0x%x + mtd_size:0x%x = 0x%x > flash_size:0x%x  error!\n",
                    pfh->mtd_name,
                    pfh->mtd_addr, pfh->mtd_offset, pfh->mtd_size,
                    tmp_size, pfw->flash_size);
        return -1;
    }

    fd = open(pfh->upgrade_file, O_RDONLY);
    if (fd < 0)
    {
        ERROR_PRT("open %s ERROR! %s\n", pfh->upgrade_file, strerror(errno));
        return -1;
    }
    pheader->file_size = lseek(fd, 0, SEEK_END);
    if (pheader->file_size < 0)
	{
        ERROR_PRT("lseek %s ERROR!\n", pfh->upgrade_file);
        close(fd);
        return -1;
    }
    lseek(fd, 0, SEEK_SET);

    /* 校验升级文件大小 */
    if(pheader->file_size > MAX_MTD_SIZE || pheader->file_size > pheader->mtd_size)
    {
        ERROR_PRT("upgrade file_size error! MAX_MTD_SIZE:0x%x  file_size:0x%x  mtd_size:0x%x\n",
                                            MAX_MTD_SIZE, pheader->file_size, pheader->mtd_size);
        close(fd);
        return -1;
    }

    pbuf = malloc(pheader->file_size + 2);
    if (NULL == pbuf)
    {
        ERROR_PRT("malloc pheader->file_size:%u ERROR!\n", pheader->file_size);
        close(fd);
        return -1;
    }

    ret = read(fd, pbuf, pheader->file_size);
    if (ret <= 0 || ret != pheader->file_size)
    {
        ERROR_PRT("pheader->file_size:%u ret:%d %s\n", pheader->file_size, ret, strerror(errno));
        free(pbuf);
        close(fd);
        return -1;
    }
    pheader->file_crc32 = utility_crc32(0, pbuf, pheader->file_size);

    MD5Init(&md5);
	MD5Update(&md5, (unsigned char *)pheader, sizeof(GK_UPGRADE_HEADER_S));
	MD5Update(&md5, (unsigned char *)pbuf, pheader->file_size);
    MD5Final(&md5, (unsigned char *)pheader->pack_md5);

    printf_upgrade_header_info(pheader);

    free(pbuf);
    close(fd);
    return 0;
}


/******************************************************************************
 * 函数介绍:
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int check_upgrade_header(struct gk_upgrade_header *pheader)
{
    if (NULL == pheader)
    {
        ERROR_PRT("pheader is NULL!\n");
        return -1;
    }

    /* 校验幻数 check magic_num  */
    if (NULL == strstr(pheader->magic_num, MAGIC_NUM))
    {
        ERROR_PRT("check magic_num error! magic_num:%s MAGIC_NUM:%s\n", pheader->magic_num, MAGIC_NUM);
        return -1;
    }

    /* 校验芯片平台 , GK7101, GK7102 ... check Platform  */
    //#ifdef DECODER
    //if (NULL == strstr(pheader->Platform, DECODER))
    //{
    //    ERROR_PRT("check Platform error! Platform:%s DECODER:%s\n", pheader->Platform, DECODER);
    //    return -1;
    //}
    //#endif

    if (0 != strcasecmp(pheader->image_ver, FORCE_UPDATE_IMAGE_VERSION ))
    {
        /* 校验产品平台 , imx222, ov9712 ... check machine_type  */
        #ifdef MODULE_UPGRADE_IN_FIFO
        if (0 != strcasecmp(pheader->machine_type, readFifoUpgradeSdkName()))
        {
            ERROR_PRT("check machine_type error:SDK-[%s], UP-FILE-[%s]\n", readFifoUpgradeSdkName(),pheader->machine_type);
            return -1;
        }
        DEBUG_PRT("check machine_type ok:SDK-[%s], UP-FILE-[%s]\n", readFifoUpgradeSdkName(),pheader->machine_type);

        /* 校验软件版本 , ... check image_ver  */
        if (0 == strcasecmp(pheader->image_ver, readFifoUpgradeCurVer()))
        {
            ERROR_PRT("check bin_ver %s == %s, need not upgrade!\n", pheader->image_ver, readFifoUpgradeCurVer());
            return -1;
        }
        DEBUG_PRT("check bin_ver ok:SDK-[%s], UP-FILE-[%s]\n", readFifoUpgradeCurVer(), pheader->image_ver);
        #else
        if (0 != strcasecmp(pheader->machine_type, sdk_cfg.name))
        {
            ERROR_PRT("check machine_type error:SDK-[%s], UP-FILE-[%s]\n", sdk_cfg.name,pheader->machine_type);
            return -1;
        }
        DEBUG_PRT("check machine_type ok:SDK-[%s], UP-FILE-[%s]\n", sdk_cfg.name,pheader->machine_type);

        /* 校验软件版本 , ... check image_ver  */
    	if (0 == strcasecmp(pheader->image_ver, runSystemCfg.deviceInfo.upgradeVersion))
    	{
    		ERROR_PRT("check bin_ver %s == %s, need not upgrade!\n", pheader->image_ver, runSystemCfg.deviceInfo.upgradeVersion);
    		return -1;
    	}
        DEBUG_PRT("check bin_ver ok:SDK-[%s], UP-FILE-[%s]\n", runSystemCfg.deviceInfo.upgradeVersion, pheader->image_ver);
        #endif
    }
    else
    {
        DEBUG_PRT("check machine_type ok:SDK-[%s], UP-FILE-[%s],Image version: Force updating\n", sdk_cfg.name,pheader->machine_type);
    }
    #if 0
    if (NULL == strstr(pheader->machine_type, CUSTOM_BOARD_SENSOR))
    {
        ERROR_PRT("check machine_type error! machine_type:%s CUSTOM_BOARD_SENSOR:%s\n", pheader->machine_type, CUSTOM_BOARD_SENSOR);
        return -1;
    }
    #endif

    return 0;
}


/******************************************************************************
 * 函数介绍:
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int pack_product_file(char *config_file, char *upgrade_bin)
{
    int    ret        = 0;
    int    cnt        = 0;
    int    image_fd   = -1;
    int    pack_fd    = -1;
    char   buff[1024] = {0};
    unsigned int             total_size = 0;
    unsigned int             file_size  = 0;
    struct firmware_info     fw_info;
    struct flash_info        fl_info[MAX_MTD_NUM];
    struct gk_upgrade_header upgrade_header[MAX_MTD_NUM];

    memset(&fw_info, 0, sizeof(fw_info));
    memset(&fl_info, 0, sizeof(fl_info));
    memset(&upgrade_header, 0, sizeof(upgrade_header));

    if (NULL == config_file || NULL == upgrade_bin)
    {
        ERROR_PRT("config_file:%p or upgrade_file:%p is NULL!\n", config_file, upgrade_bin);
        return -1;
    }

    /* 读取升级flash_map.ini 配置文件信息 */
    ret = get_upgradeinfo_by_cfgfile(config_file, &fw_info, fl_info);
    if (ret < 0)
    {
        ERROR_PRT("Call get_upgradeinfo_by_cfgfile error! ret:%d\n", ret);
        return -1;
    }

    /* 填充打包的升级头结构体 , 用于升级时解包的信息*/
    for (cnt = 0; cnt < fw_info.upgrade_num; cnt++)
    {
        ret = fill_upgrade_header(&fw_info, &fl_info[cnt], &upgrade_header[cnt]);
        if (ret < 0)
        {
            ERROR_PRT("Call fill_upgrade_header error! cnt:%d ret:%d\n", cnt, ret);
            return -1;
        }
    }

    /* 将多个指定的升级镜像, 整合成一个完整的upgrade.bin 文件 */
    pack_fd = open(upgrade_bin, O_RDWR|O_CREAT);
    if (pack_fd < 0)
    {
        ERROR_PRT("open %s ERROR! %s\n", upgrade_bin, strerror(errno));
        return -1;
    }

    total_size = 0;
    for (cnt = 0; cnt < fw_info.upgrade_num; cnt++)
    {
        /* 总体文件大小不应大于 flash 容量大小 */
        if (total_size + upgrade_header[cnt].file_size > fw_info.flash_size)
        {
            ERROR_PRT("cur_size:%u + file_size:%u > flash_size:%u\n", total_size,
                                                                      upgrade_header[cnt].file_size,
                                                                      fw_info.flash_size);
            ret = -1;
            goto pack_flash_error;
        }

        file_size = 0;
        image_fd = open(fl_info[cnt].upgrade_file, O_RDONLY);
        if (image_fd < 0)
        {
            ERROR_PRT("open %s ERROR! %s\n", fl_info[cnt].upgrade_file, strerror(errno));
            ret = -1;
            goto pack_flash_error;
        }

        /* 写入升级文件内容 */
        while(1)
        {
            /* 总体文件大小不应大于 flash 容量大小 */
            if (total_size >= fw_info.flash_size)
            {
                DEBUG_PRT("cur_size:%u >= flash_size:%u\n", total_size, fw_info.flash_size);
                ret = 0;
                goto pack_flash_error;
            }

            memset(buff, 0, sizeof(buff));
            ret = read(image_fd, buff, sizeof(buff));
            if (ret <= 0)
            {
                if (file_size != upgrade_header[cnt].file_size)
                {
                    ERROR_PRT("file:%s size error! upgrade_header.file_size:%u  file_size:%u \n",
                                        fl_info[cnt].upgrade_file, upgrade_header[cnt].file_size, file_size);
                    goto pack_flash_error;
                }
                DEBUG_PRT("read %s wr_size:[%u <-> %u] end! ret:%d\n", fl_info[cnt].upgrade_file,
                                                                        upgrade_header[cnt].file_size,
                                                                        file_size,
                                                                        ret);
                close(image_fd);
                image_fd = -1;
                break;
            }

            ret = write(pack_fd, buff, ret);
            if (ret <= 0)
            {
                ERROR_PRT("write %s error! %s\n", fl_info[cnt].upgrade_file, strerror(errno));
                goto pack_flash_error;
            }

            file_size  += ret;
            total_size += ret;
        }

        /* check 分区大小，对比已写入的文件大小，若不满分区大小,
                *  则用0xff 填充剩余空间.
                */
        if (file_size >= fl_info[cnt].mtd_size)
        {
            close(image_fd);
            image_fd = -1;
        }
        else if (file_size < fl_info[cnt].mtd_size)
        {
            memset(buff, 0xff, sizeof(buff));
            while (file_size < fl_info[cnt].mtd_size)
            {
                if ((fl_info[cnt].mtd_size - file_size) > sizeof(buff))
                {
                    ret = write(pack_fd, buff, sizeof(buff));
                }
                else
                {
                    ret = write(pack_fd, buff, (fl_info[cnt].mtd_size - file_size));
                }
                if (ret <= 0)
                {
                    ERROR_PRT("write %s  file_size:%u error! ret:%d %s\n", upgrade_bin, file_size, ret, strerror(errno));
                    goto pack_flash_error;
                }
                file_size += ret;
            }
        }
    }

    ret = 0;
pack_flash_error:
    if(image_fd > 0)
    {
        close(image_fd);
    }
    if(pack_fd > 0)
    {
        close(pack_fd);
    }
    return ret;
}


/******************************************************************************
 * 函数介绍:
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int pack_upgrade_file(char *config_file, char *upgrade_bin)
{
    int    ret        = 0;
    int    cnt        = 0;
    int    image_fd   = -1;
    int    pack_fd    = -1;
    char   buff[1024] = {0};
    unsigned int             file_size = 0;
    struct firmware_info     fw_info;
    struct flash_info        fl_info[MAX_MTD_NUM];
    struct gk_upgrade_header upgrade_header[MAX_MTD_NUM];

    memset(&fw_info, 0, sizeof(fw_info));
    memset(&fl_info, 0, sizeof(fl_info));
    memset(&upgrade_header, 0, sizeof(upgrade_header));

    if (NULL == config_file || NULL == upgrade_bin)
    {
        ERROR_PRT("config_file:%p or upgrade_file:%p is NULL!\n", config_file, upgrade_bin);
        return -1;
    }

    /* 读取升级flash_map.ini 配置文件信息 */
    ret = get_upgradeinfo_by_cfgfile(config_file, &fw_info, fl_info);
    if (ret < 0)
    {
        ERROR_PRT("Call get_upgradeinfo_by_cfgfile error! ret:%d\n", ret);
        return -1;
    }

    /* 填充打包的升级头结构体 , 用于升级时解包的信息*/
    for (cnt = 0; cnt < fw_info.upgrade_num; cnt++)
    {
        /* 若升级标志为0, 则不把镜像打包进升级文件包中 */
        if (0 == fl_info[cnt].upgrade_flag)
        {
            DEBUG_PRT("The file:%s don't upgrade. don't fill the upgrade_header.\n", fl_info[cnt].upgrade_file);
            continue;
        }

        ret = fill_upgrade_header(&fw_info, &fl_info[cnt], &upgrade_header[cnt]);
        if (ret < 0)
        {
            ERROR_PRT("Call fill_upgrade_header error! cnt:%d ret:%d\n", cnt, ret);
            return -1;
        }
    }

    /* 将多个指定的升级镜像, 整合成一个完整的upgrade.bin 文件 */
    pack_fd = open(upgrade_bin, O_RDWR|O_CREAT);
    if (pack_fd < 0)
    {
        ERROR_PRT("open %s ERROR! %s\n", upgrade_bin, strerror(errno));
        return -1;
    }
    for (cnt = 0; cnt < fw_info.upgrade_num; cnt++)
    {
        /* 若升级标志为0, 则不把镜像打包进升级文件包中 */
        if (0 == fl_info[cnt].upgrade_flag)
        {
            DEBUG_PRT("The file:%s don't upgrade. no pack in %s \n",
                                          fl_info[cnt].upgrade_file, upgrade_bin);
            continue;
        }

        file_size = 0;
        image_fd = open(fl_info[cnt].upgrade_file, O_RDONLY);
        if (image_fd < 0)
        {
            ERROR_PRT("open %s ERROR! %s\n", fl_info[cnt].upgrade_file, strerror(errno));
            ret = -1;
            goto pack_bin_error;
        }

        /* 写入升级结构体头文件信息 */
        ret = write(pack_fd, &upgrade_header[cnt], sizeof(struct gk_upgrade_header));
        if (ret <= 0)
        {
            ERROR_PRT("write %s ERROR! %s\n", upgrade_bin, strerror(errno));
            goto pack_bin_error;
        }
        /* 写入升级文件内容 */
        while(1)
        {
            memset(buff, 0, sizeof(buff));
            ret = read(image_fd, buff, sizeof(buff));
            if (ret <= 0)
            {
                if (file_size != upgrade_header[cnt].file_size)
                {
                    ERROR_PRT("file:%s size error! upgrade_header.file_size:%u  file_size:%u \n",
                                        fl_info[cnt].upgrade_file, upgrade_header[cnt].file_size, file_size);
                    goto pack_bin_error;
                }
                DEBUG_PRT("read %s file_size:%u end! %s\n", fl_info[cnt].upgrade_file, file_size, strerror(errno));
                close(image_fd);
                image_fd = -1;
                break;
            }

            ret = write(pack_fd, buff, ret);
            if (ret <= 0)
            {
                ERROR_PRT("write %s error! %s\n", fl_info[cnt].upgrade_file, strerror(errno));
                goto pack_bin_error;
            }

            file_size += ret;
        }
    }

    ret = 0;
pack_bin_error:
    if(image_fd > 0)
    {
        close(image_fd);
    }
    if(pack_fd > 0)
    {
        close(pack_fd);
    }
    return ret;
}


/******************************************************************************
 * 函数介绍:
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int unpack_upgrade_file(char *upgrade_bin)
{
    int    ret        = 0;
    int    image_fd   = -1;
    int    pack_fd    = -1;
    char   *pbuf      = NULL;
    char   name[256]  = {0};
    unsigned int  total_size = 0;
    unsigned int  tmp_size   = 0;
    unsigned int  crc32_val  = 0;
    struct gk_upgrade_header upgrade_header;
    char   pack_md5[16]  = {0};
	MD5_CTX md5;

    memset(&upgrade_header, 0, sizeof(upgrade_header));
    memset(&md5, 0, sizeof(md5));

    if (NULL == upgrade_bin)
    {
        ERROR_PRT("upgrade_bin is NULL!\n");
        return -1;
    }

    pack_fd = open(upgrade_bin, O_RDONLY);
    if (pack_fd < 0)
    {
        ERROR_PRT("open %s ERROR! %s\n", upgrade_bin, strerror(errno));
        return -1;
    }
    total_size = lseek(pack_fd, 0, SEEK_END);
    if (total_size < 0)
	{
        ERROR_PRT("lseek %s ERROR! %s\n", upgrade_bin, strerror(errno));
        ret = -1;
        goto unpack_bin_error;
    }
    ret = lseek(pack_fd, 0, SEEK_SET);
    if (ret < 0)
	{
        ERROR_PRT("lseek %s ERROR! %s\n", upgrade_bin, strerror(errno));
        ret = -1;
        goto unpack_bin_error;
    }
    while (1)
    {
        /* 1. 读取文件头信息 */
        ret = read(pack_fd, &upgrade_header, sizeof(struct gk_upgrade_header));
        if (ret <= 0)
        {
            if (total_size != tmp_size)
            {
                ERROR_PRT("total_size:%u != tmp_size:%u ret:%d %s\n", total_size, tmp_size, ret, strerror(errno));
                goto unpack_bin_error;
            }
            DEBUG_PRT("read %s  total_size:%u end! ret:%d %s\n", upgrade_bin, tmp_size, ret, strerror(errno));
            break;
        }
        tmp_size += ret;

        /* 校验升级文件头信息, 判断是否与本系统匹配  */
        ret = check_upgrade_header(&upgrade_header);
        if (ret)
        {
            ERROR_PRT("check upgrade_header error!\n");
            ret = -1;
            goto unpack_bin_error;
        }

        /* 校验升级文件大小 */
        if(upgrade_header.file_size >= MAX_MTD_SIZE)
        {
            ERROR_PRT("upgrade address is too longht, error! 0x%x (%dKB)\n",
                                            upgrade_header.file_size, upgrade_header.file_size/1024);
            ret = -1;
            goto unpack_bin_error;
        }

        DEBUG_PRT("header_addr:0x%x  image_addr:0x%x \n", (tmp_size - ret), tmp_size);
        printf_upgrade_header_info(&upgrade_header);

        /*2.  读取需要升级的镜像到buff 中, 用于CRC 校验和升级*/
        if (pbuf)
        {
            free(pbuf);
            pbuf = NULL;
        }
        pbuf = malloc(upgrade_header.file_size + 2);
        if (NULL == pbuf)
        {
            ERROR_PRT("malloc pheader->file_size:%u ERROR!\n", upgrade_header.file_size);
            ret = -1;
            goto unpack_bin_error;
        }

        ret = read(pack_fd, pbuf, upgrade_header.file_size);
        if (ret <= 0)
        {
            ERROR_PRT("total_size:%u != tmp_size:%u ret:%d %s\n", total_size, tmp_size, ret, strerror(errno));
            goto unpack_bin_error;
        }
        tmp_size += ret;

        memcpy(pack_md5, upgrade_header.pack_md5, sizeof(pack_md5));
		memcpy(upgrade_header.pack_md5, upgrade_header.machine_type, sizeof(upgrade_header.pack_md5));
		MD5Init(&md5);
		MD5Update(&md5, (unsigned char *)&upgrade_header, sizeof(GK_UPGRADE_HEADER_S));
		MD5Update(&md5, (unsigned char *)pbuf, upgrade_header.file_size);
		MD5Final(&md5, (unsigned char *)upgrade_header.pack_md5);
		if (0 != memcmp(pack_md5, upgrade_header.pack_md5, sizeof(upgrade_header.pack_md5)))
        {
			char md5str[2][64] = {0};
			printf_md5(upgrade_header.pack_md5, md5str[0]);
			printf_md5(pack_md5, md5str[1]);
            ERROR_PRT("The mtd:%s  name:%s md5 verify error! md5_val:%s, header_md5:%s\n",
                        upgrade_header.mtd_part, upgrade_header.mtd_name,  md5str[0],  md5str[1]);
            ret = -1;
            goto unpack_bin_error;
        }

        crc32_val = utility_crc32(0, pbuf, upgrade_header.file_size);
        if (crc32_val != upgrade_header.file_crc32)
        {
            ERROR_PRT("The mtd:%s  name:%s crc32 verify error! header_crc:%u  crc32_val:%u\n",
                        upgrade_header.mtd_part, upgrade_header.mtd_name,
                        upgrade_header.file_crc32, crc32_val);
            ret = -1;
            goto unpack_bin_error;
        }

        /* 3. 从升级包提取出相应的升级文件,  即解包升级文件 */
        if(image_fd > 0)
        {
            close(image_fd);
            image_fd = -1;
        }
        memset(name, 0, sizeof(name));
        strcat(name, upgrade_header.mtd_name);
        strcat(name, ".bin");
        image_fd = open(name, O_RDWR|O_CREAT);
        if (image_fd < 0)
        {
            ERROR_PRT("open %s ERROR! %s\n", name, strerror(errno));
            return -1;
        }

        ret = write(image_fd, pbuf, upgrade_header.file_size);
        if (ret <= 0)
        {
            ERROR_PRT("write %s error! %s\n", name, strerror(errno));
            goto unpack_bin_error;
        }
    }

    ret = 0;
unpack_bin_error:
    if (pbuf)
    {
        free(pbuf);
    }
    if(image_fd > 0)
    {
        close(image_fd);
    }
    if(pack_fd > 0)
    {
        close(pack_fd);
    }
    return ret;
}


/******************************************************************************
 * 函数介绍:
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int verify_upgrade_file(char *upgrade_bin)
{
    int    ret        = 0;
    int    pack_fd    = -1;
    char  *pbuf       = NULL;
    unsigned int  crc32_val  = 0;
    unsigned int  file_size  = 0;
    struct gk_upgrade_header upgrade_header;
    char   pack_md5[16]  = {0};
	MD5_CTX md5;

    memset(&upgrade_header, 0, sizeof(upgrade_header));
    memset(&md5, 0, sizeof(md5));

    if (NULL == upgrade_bin)
    {
        ERROR_PRT("upgrade_bin is NULL!\n");
        return -1;
    }

    pack_fd = open(upgrade_bin, O_RDONLY);
    if (pack_fd < 0)
    {
        ERROR_PRT("open %s ERROR! %s\n", upgrade_bin, strerror(errno));
        return -1;
    }

    while (1)
    {
        /* 1. 读取文件头信息 */
        ret = read(pack_fd, &upgrade_header, sizeof(struct gk_upgrade_header));
        if (ret <= 0)
        {
            DEBUG_PRT("read %s  total_size:%u end! ret:%d %s\n", upgrade_bin, file_size, ret, strerror(errno));
            break;
        }
        file_size += ret;

        /* 校验升级文件头信息, 判断是否与本系统匹配  */
        if (check_upgrade_header(&upgrade_header))
        {
            ERROR_PRT("check upgrade_header error!\n");
            ret = -1;
            goto verify_upgrade_err;
        }

        /* 校验升级文件大小 */
        if(upgrade_header.file_size >= MAX_MTD_SIZE)
        {
            ERROR_PRT("upgrade address is too longht, error! 0x%x (%dKB)\n",
                                            upgrade_header.file_size, upgrade_header.file_size/1024);
            ret = -1;
            goto verify_upgrade_err;
        }

        DEBUG_PRT("header_addr:0x%x  image_addr:0x%x \n", (file_size - ret), file_size);
        printf_upgrade_header_info(&upgrade_header);

        /*2.  读取需要升级的镜像到buff 中, 用于CRC 校验和升级*/
        if (pbuf)
        {
            free(pbuf);
            pbuf = NULL;
        }
        pbuf = malloc(upgrade_header.file_size + 2);
        if (NULL == pbuf)
        {
            ERROR_PRT("malloc pheader->file_size:%u ERROR!\n", upgrade_header.file_size);
            ret = -1;
            goto verify_upgrade_err;
        }

        ret = read(pack_fd, pbuf, upgrade_header.file_size);
        if (ret <= 0 || ret != upgrade_header.file_size)
        {
            ERROR_PRT("file_size:%u ret:%d %s\n", file_size, ret, strerror(errno));
            goto verify_upgrade_err;
        }
        file_size += ret;

        memcpy(pack_md5, upgrade_header.pack_md5, sizeof(pack_md5));
		memcpy(upgrade_header.pack_md5, upgrade_header.machine_type, sizeof(upgrade_header.pack_md5));
		MD5Init(&md5);
		MD5Update(&md5, (unsigned char *)&upgrade_header, sizeof(GK_UPGRADE_HEADER_S));
		MD5Update(&md5, (unsigned char *)pbuf, upgrade_header.file_size);
		MD5Final(&md5, (unsigned char *)upgrade_header.pack_md5);
		if (0 != memcmp(pack_md5, upgrade_header.pack_md5, sizeof(upgrade_header.pack_md5)))
        {
            char md5str[2][64] = {0};
			printf_md5(upgrade_header.pack_md5, md5str[0]);
			printf_md5(pack_md5, md5str[1]);
            ERROR_PRT("The mtd:%s  name:%s md5 verify error! md5_val:%s, header_md5:%s\n",
                        upgrade_header.mtd_part, upgrade_header.mtd_name,  md5str[0],  md5str[1]);
            ret = -1;
            goto verify_upgrade_err;
        }

        /* 3. 与打包时的头结构体中的CRC，进行校验 */
        crc32_val = utility_crc32(0, pbuf, upgrade_header.file_size);
        if (crc32_val != upgrade_header.file_crc32)
        {
            ERROR_PRT("The mtd:%s  name:%s crc32 verify error! header_crc:%u  crc32_val:%u\n",
                        upgrade_header.mtd_part, upgrade_header.mtd_name,
                        upgrade_header.file_crc32, crc32_val);
            ret = -1;
            goto verify_upgrade_err;
        }
    }

    ret = 0;
verify_upgrade_err:
    if (pbuf)
    {
        free(pbuf);
    }
    if(pack_fd > 0)
    {
        close(pack_fd);
    }
    return ret;
}


/******************************************************************************
 * 函数介绍:
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int verify_upgrade_buff(char *upgrade_buf, unsigned int buf_size)
{
    unsigned int  pos        = 0;
    unsigned int  crc32_val  = 0;
    struct gk_upgrade_header upgrade_header;
    char   pack_md5[16]  = {0};
	MD5_CTX md5;

    memset(&upgrade_header, 0, sizeof(upgrade_header));
    memset(&md5, 0, sizeof(md5));

    if (NULL == upgrade_buf)
    {
        ERROR_PRT("upgrade_buf is NULL error!\n");
        return -1;
    }

    if (buf_size > MAX_NORFLASH_SIZE)
    {
        ERROR_PRT("file_size:0x%x is too large! MAX_NORFLASH_SIZE:0x%x\n", buf_size, MAX_NORFLASH_SIZE);
        return -1;
    }

    pos = 0;
    while (1)
    {
        if (pos >= buf_size)
        {
            DEBUG_PRT("total_size:%u pos:%d\n", buf_size, pos);
            return 0;
        }

        /* 1. 读取文件头信息 */
        memcpy(&upgrade_header, upgrade_buf + pos, sizeof(struct gk_upgrade_header));
        pos += sizeof(struct gk_upgrade_header);

        /* 2. 校验升级文件头信息, 判断是否与本系统匹配  */
        if (check_upgrade_header(&upgrade_header))
        {
            ERROR_PRT("check upgrade_header error!\n");
            return -1;
        }
        /* 3. 校验升级文件是否越界 */
        if((upgrade_header.file_size + pos) > buf_size)
        {
            ERROR_PRT("(upgrade_header.file_size + gk_upgrade_header):0x%x > buf_size:0x%x error!\n",
                                            (upgrade_header.file_size + sizeof(struct gk_upgrade_header)), buf_size);
            return -1;
        }

        /* 4. 校验升级文件大小 */
        if(upgrade_header.file_size >= MAX_MTD_SIZE)
        {
            ERROR_PRT("upgrade address is too longht, error! 0x%x (%dKB)\n",
                                            upgrade_header.file_size, upgrade_header.file_size/1024);
            return -1;
        }

        DEBUG_PRT("mtd_name:%s  file_addr:0x%x \n", upgrade_header.mtd_name, pos);
        printf_upgrade_header_info(&upgrade_header);

        memcpy(pack_md5, upgrade_header.pack_md5, sizeof(pack_md5));
		memcpy(upgrade_header.pack_md5, upgrade_header.machine_type, sizeof(upgrade_header.pack_md5));
		MD5Init(&md5);
		MD5Update(&md5, (unsigned char *)&upgrade_header, sizeof(GK_UPGRADE_HEADER_S));
		MD5Update(&md5, (unsigned char *)upgrade_buf + pos, upgrade_header.file_size);
		MD5Final(&md5, (unsigned char *)upgrade_header.pack_md5);
		if (0 != memcmp(pack_md5, upgrade_header.pack_md5, sizeof(upgrade_header.pack_md5)))
        {
            char md5str[2][64] = {0};
			printf_md5(upgrade_header.pack_md5, md5str[0]);
			printf_md5(pack_md5, md5str[1]);
            ERROR_PRT("The mtd:%s  name:%s md5 verify error! md5_val:%s, header_md5:%s\n",
                        upgrade_header.mtd_part, upgrade_header.mtd_name,  md5str[0],  md5str[1]);
            return -1;
        }

        /* 4. 与打包时的头结构体中的CRC，进行校验 */
        crc32_val = utility_crc32(0, upgrade_buf + pos, upgrade_header.file_size);
        if (crc32_val != upgrade_header.file_crc32)
        {
            ERROR_PRT("The mtd:%s name:%s crc32 verify error! header_crc:%x crc32_val:%x\n",
                        upgrade_header.mtd_part, upgrade_header.mtd_name,
                        upgrade_header.file_crc32, crc32_val);
            return -1;
        }
        pos += upgrade_header.file_size;
    }

    return 0;
}


/******************************************************************************
 * 函数介绍:
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int get_upgradeinfo_by_file(char *upgrade_bin, struct upgrade_file_info *pinfo, int * mod_num)
{
    int    ret        = 0;
    int    mtd_cnt    = 0;
    int    pack_fd    = -1;
    unsigned int  file_size  = 0;

    if (NULL == upgrade_bin || NULL == pinfo || NULL == mod_num)
    {
        ERROR_PRT("upgrade_bin:%p pinfo:%p mod_num:%p is NULL!\n", upgrade_bin, pinfo, mod_num);
        return -1;
    }

    pack_fd = open(upgrade_bin, O_RDONLY);
    if (pack_fd < 0)
    {
        ERROR_PRT("open %s ERROR! %s\n", upgrade_bin, strerror(errno));
        return -1;
    }

    file_size = 0;
    mtd_cnt   = 0;
    while (1)
    {
        if (mtd_cnt >= MAX_MTD_NUM)
        {
            ERROR_PRT("mtd_cnt >= MAX_MTD_NUM error!\n");
            ret = -1;
            goto get_upgradeinfo_err;
        }

        /* 1. 读取文件头信息 */
        ret = read(pack_fd, &(pinfo[mtd_cnt].upgrade_header), sizeof(struct gk_upgrade_header));
        if (ret <= 0)
        {
            DEBUG_PRT("read %s  total_size:%u end! ret:%d %s\n", upgrade_bin, file_size, ret, strerror(errno));
            break;
        }
        file_size += ret;

        /* 2. 校验升级文件头信息, 判断是否与本系统匹配  */
        if (check_upgrade_header(&pinfo[mtd_cnt].upgrade_header))
        {
            ERROR_PRT("check upgrade_header error!\n");
            ret = -1;
            goto get_upgradeinfo_err;
        }

        /* 3. 校验升级文件大小 */
        if(pinfo[mtd_cnt].upgrade_header.file_size >= MAX_MTD_SIZE)
        {
            ERROR_PRT("upgrade address is too longht, error! 0x%x (%dKB)\n",
                                            pinfo[mtd_cnt].upgrade_header.file_size,
                                            pinfo[mtd_cnt].upgrade_header.file_size/1024);
            ret = -1;
            goto get_upgradeinfo_err;
        }

        /* 4. 获取相应参数*/
        pinfo[mtd_cnt].file_addr = lseek(pack_fd, 0, SEEK_CUR);
        pinfo[mtd_cnt].file_size = pinfo[mtd_cnt].upgrade_header.file_size;

        DEBUG_PRT("header_addr:0x%x  image_addr:0x%x \n",
                                        pinfo[mtd_cnt].file_addr - sizeof(struct gk_upgrade_header),
                                        pinfo[mtd_cnt].file_addr);
        printf_upgrade_header_info(&(pinfo[mtd_cnt].upgrade_header));

        /* 5.  跳到下一个升级模块位置*/
        ret = lseek(pack_fd, pinfo[mtd_cnt].upgrade_header.file_size, SEEK_CUR);
        if (ret < 0)
    	{
            ERROR_PRT("lseek %s ERROR! %s\n", upgrade_bin, strerror(errno));
            ret = -1;
            goto get_upgradeinfo_err;
        }
        file_size += pinfo[mtd_cnt].upgrade_header.file_size;

        mtd_cnt++;
    }

    *mod_num = mtd_cnt;

    ret = 0;
get_upgradeinfo_err:
    if(pack_fd > 0)
    {
        close(pack_fd);
    }
    return ret;
}


/******************************************************************************
 * 函数介绍:
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int get_upgradeinfo_by_buff(char *upgrade_buf,
                                    unsigned int buf_size,
                                    struct upgrade_file_info *pinfo,
                                    int * mod_num)
{
    int    mtd_cnt     = 0;
    unsigned int  pos  = 0;

    if (NULL == upgrade_buf || NULL == pinfo || NULL == mod_num)
    {
        ERROR_PRT("upgrade_buff:%p pinfo:%p mod_num:%p is NULL!\n", upgrade_buf, pinfo, mod_num);
        return -1;
    }

    if (buf_size > MAX_NORFLASH_SIZE)
    {
        ERROR_PRT("file_size:0x%x is too large! MAX_NORFLASH_SIZE:0x%x\n", buf_size, MAX_NORFLASH_SIZE);
        return -1;
    }

    pos = 0;
    mtd_cnt   = 0;
    while (1)
    {
        if (mtd_cnt >= MAX_MTD_NUM)
        {
            ERROR_PRT("mtd_cnt >= MAX_MTD_NUM error!\n");
            return -1;
        }

        if (pos >= buf_size)
        {
            DEBUG_PRT("read_pos:%u >= buf_size:%u return!\n", pos, buf_size);
            break;
        }

        /* 1. 读取文件头信息 */
        memcpy(&(pinfo[mtd_cnt].upgrade_header), upgrade_buf + pos, sizeof(struct gk_upgrade_header));
        pos += sizeof(struct gk_upgrade_header);

        /* 2. 校验升级文件头信息, 判断是否与本系统匹配  */
        if (check_upgrade_header(&pinfo[mtd_cnt].upgrade_header))
        {
            ERROR_PRT("check upgrade_header error!\n");
            return -1;
        }

        /* 3. 校验升级文件大小 */
        if(pinfo[mtd_cnt].upgrade_header.file_size >= MAX_MTD_SIZE)
        {
            ERROR_PRT("upgrade address is too longht, error! 0x%x (%dKB)\n",
                                            pinfo[mtd_cnt].upgrade_header.file_size,
                                            pinfo[mtd_cnt].upgrade_header.file_size/1024);
            return -1;
        }

        /* 4. 获取相应参数*/
        pinfo[mtd_cnt].file_addr = pos;
        pinfo[mtd_cnt].file_size = pinfo[mtd_cnt].upgrade_header.file_size;

        DEBUG_PRT("header_addr:0x%x  image_addr:0x%x \n",
                                        pinfo[mtd_cnt].file_addr - sizeof(struct gk_upgrade_header),
                                        pinfo[mtd_cnt].file_addr);
        printf_upgrade_header_info(&(pinfo[mtd_cnt].upgrade_header));

        /* 5.  跳到下一个升级模块位置*/
        pos += pinfo[mtd_cnt].upgrade_header.file_size;

        /* 6. 升级包中的镜像个数加1 */
        mtd_cnt++;
    }

    *mod_num = mtd_cnt;

    return 0;
}


/******************************************************************************
 * 函数介绍: 该函数是主要是升级策略, 根据现有分区信息和
 *                           升级文件信息, 判断是对普通分区进行升级，还
 *                           是对 ALL 分区进行操作.
 * 输入参数: pmtd_info       : 是大小为MAX_MTD_NUM 的结构体数组!
 *                           mtd_num        : pmtd_info中有效个数
 *                           pupgrade_info : 输出的升级镜像信息
 *
 * 输出参数: 无
 * 返回值      : <0-错误代码
 *                           1-表示只需要升级非ALL_MTD 的普通分区, 如: /dev/mtd1
 *                           2-表示分区表和分区大小改变了, 只能对ALL_MTD 分区操作
 *****************************************************************************/
int judge_upgrade_by_info(struct proc_mtd_info *pmtd_info,
                                  int mtd_num,
                                  struct upgrade_file_info *pupgrade_info)
{
    unsigned int tmp        = 0;
    unsigned int total_size = 0;

    if (NULL == pmtd_info || NULL == pupgrade_info)
    {
        ERROR_PRT("pmtd_info:%p  pupgrade_info:%p error!\n", pmtd_info, pupgrade_info);
        return -1;
    }

    if (mtd_num > MAX_MTD_NUM)
    {
        ERROR_PRT("Input mtd_num:%d too large MAX_MTD_NUM:%d\n", mtd_num, MAX_MTD_NUM);
        return -1;
    }

    if (pmtd_info[mtd_num].mtd_addr == pupgrade_info->upgrade_header.mtd_addr
       && pmtd_info[mtd_num].mtd_size == pupgrade_info->upgrade_header.mtd_size
       && pmtd_info[mtd_num].mtd_size >= pupgrade_info->file_size)
    {
        /* 升级指定的非ALL 分区,  如:  uboot, kernel 对应的/dev/mtd0 , /dev/mtd3 等 */
        DEBUG_PRT("Upgrade  part:%s  name:%s  flash_addr:0x%x  mtd_size:0x%x  file_size:0x%x\n",
                  pmtd_info[mtd_num].mtd_part, pmtd_info[mtd_num].mtd_name,
                  pmtd_info[mtd_num].mtd_addr, pmtd_info[mtd_num].mtd_size,
                  pupgrade_info->file_size);
        return UPGRADE_NORMAL_MTD;
    }
    else
    {
        /* 此处说明分区已经改变, 需要对 ALL 分析进行操作 */
        tmp        = pupgrade_info->upgrade_header.mtd_addr + pupgrade_info->upgrade_header.mtd_size;
        total_size = pupgrade_info->upgrade_header.flash_size;
        if (tmp <= total_size)
        {
            DEBUG_PRT("Upgrade to ALL_MTD [%s] name:%s  flash_addr:0x%x  mtd_size:0x%x  file_size:0x%x\n",
                      pupgrade_info->upgrade_header.mtd_part, pupgrade_info->upgrade_header.mtd_name,
                      pupgrade_info->upgrade_header.mtd_addr, pupgrade_info->upgrade_header.mtd_size,
                      pupgrade_info->file_size);
            return UPGRADE_ALL_MTD;
        }
        else
        {
            ERROR_PRT("upgrade_file_info error addr:0x%x + file_size:0x%x > flash_size:0x%x \n\n",
                                                       pupgrade_info->upgrade_header.mtd_addr,
                                                       pupgrade_info->upgrade_header.mtd_size,
                                                       pupgrade_info->upgrade_header.flash_size);
            return -1;
        }
    }

    return 0;
}

#ifdef MODULE_SUPPORT_UPGRADE_BACKUPS
typedef struct backups_head
{
    char partitinon_name[32];
    int update_flag;
    int partitnon_addr;
    int partitnon_size;
    int partitnon_data_size;
    unsigned int partitnon_crc;
    unsigned int magic;
}BACKUPS_HEAD_T;

#define BACKUPS_PARTITNON_NAME MTD_PART_BACKUPS
#define BACKUPS_HEAD_MAGIC (0X12345678)

#define BACKUPS_MAX_SIZE MTD_SIZE_BACKUPS
#define OLD_HEAD_SIZE (64*1024)//|update head(64K)|reserver(64K)|
#define NEW_HEAD_SIZE (64*1024)
#define OLD_HEAD_OFFSET (0)
#define OLD_PART_DATA_OFFSET (OLD_HEAD_OFFSET + OLD_HEAD_SIZE)
#define NEW_HEAD_OFFSET (BACKUPS_MAX_SIZE - NEW_HEAD_SIZE)
#define RESERVER_SIZE  (64*1024)

int save_old_part_info(struct upgrade_file_info *upgrade_file_info)
{
    int ret, rdsize, total=0, fd = -1, crc_size;
    unsigned int crc = 0;    
    BACKUPS_HEAD_T *old_head;
    char *opd_slice = NULL;

    printf("[%s, %d]Start backup old partition, file_size:0x%x mtd size:0x%x\n", 
        __func__, __LINE__, upgrade_file_info->upgrade_header.file_size, upgrade_file_info->upgrade_header.mtd_size);

    opd_slice = malloc(RESERVER_SIZE);
    if(NULL == opd_slice)
    {
        ERROR_PRT("malloc error\n");
        goto save_old_part_info_end;
    }
    fd = open(upgrade_file_info->upgrade_header.mtd_part, O_RDONLY);
    if(fd < 0)
    {
        ERROR_PRT("open [%s] error\n", upgrade_file_info->upgrade_header.mtd_part);
        goto save_old_part_info_end;
    }
    ret = erase_flash_all(BACKUPS_PARTITNON_NAME, 0, upgrade_file_info->upgrade_header.mtd_size + OLD_HEAD_SIZE);
    if (ret < 0)
    {
        /* 校验失败, 错误退出 */
        ERROR_PRT("erase_flash_all %s error!!\n", BACKUPS_PARTITNON_NAME);
        goto save_old_part_info_end;
    }
    
    while(total < upgrade_file_info->upgrade_header.mtd_size)
    {
        rdsize = upgrade_file_info->upgrade_header.mtd_size-total;
        rdsize = rdsize/RESERVER_SIZE ? RESERVER_SIZE : rdsize%RESERVER_SIZE;
        if(rdsize <=0 )
            break;        
        memset(opd_slice, 0, rdsize);
        ret = read(fd, opd_slice, rdsize);
        if(ret != rdsize)
        {
            ERROR_PRT("read [%s] error, read size:%d,%d\n", upgrade_file_info->upgrade_header.mtd_part, rdsize, ret);
            goto save_old_part_info_end;
        }
        crc_size = total+rdsize;
        if(crc_size > upgrade_file_info->upgrade_header.file_size)
        {
            crc_size -= upgrade_file_info->upgrade_header.file_size;
            crc_size = (crc_size < rdsize)?upgrade_file_info->upgrade_header.file_size-total:0;
        }
        else
            crc_size = rdsize;
            
        if(crc_size)
            crc = utility_crc32(crc, opd_slice, crc_size);

        ret = flash_copy(BACKUPS_PARTITNON_NAME, OLD_HEAD_SIZE+total, opd_slice, rdsize);
        if (ret < 0)
        {
            /* 校验失败, 错误推出 */
            ERROR_PRT("flash_copy %s error!!\n", BACKUPS_PARTITNON_NAME);
            //return -1;
            goto save_old_part_info_end;
        }
        total += rdsize;
    }
    
    close(fd);
    fd = -1;
    memset(opd_slice, 0, OLD_HEAD_SIZE);
    old_head = (BACKUPS_HEAD_T *)opd_slice;
    strcpy(old_head->partitinon_name, upgrade_file_info->upgrade_header.mtd_part);
    old_head->partitnon_addr = upgrade_file_info->upgrade_header.mtd_addr;
    old_head->partitnon_data_size = upgrade_file_info->upgrade_header.file_size;
    old_head->partitnon_size = upgrade_file_info->upgrade_header.mtd_size;
    old_head->update_flag = 1;
    old_head->magic = BACKUPS_HEAD_MAGIC;
    old_head->partitnon_crc = crc;
    //save old head
    ret = flash_copy(BACKUPS_PARTITNON_NAME, OLD_HEAD_OFFSET, opd_slice, OLD_HEAD_SIZE);
    if (ret < 0)
    {
        /* 校验失败, 错误推出 */
        ERROR_PRT("flash_copy %s error!!\n", BACKUPS_PARTITNON_NAME);
        //return -1;
        goto save_old_part_info_end;
    }
    printf("[%s, %d] Backup old partition complete, old head crc:0x%x, file size:0x%x, mtd size:0x%x, addr:0x%x\n", 
        __func__, __LINE__, old_head->partitnon_crc, old_head->partitnon_data_size, old_head->partitnon_size, old_head->partitnon_addr);

save_old_part_info_end:
    if(opd_slice != NULL)
        free(opd_slice);
    if(fd > 0)
        close(fd);
    return 0;
}

int save_new_part_info(struct upgrade_file_info *upgrade_file_info)
{
    int ret;
    BACKUPS_HEAD_T *new_head;
    char *new_head_buffer;

    new_head_buffer = malloc(OLD_HEAD_SIZE);
    if(new_head_buffer == NULL)
    {
        ERROR_PRT("malloc size:%d fail!\n", OLD_HEAD_SIZE);
        goto save_new_part_info_end;
    }

    memset(new_head_buffer, 0, OLD_HEAD_SIZE);
    new_head = (BACKUPS_HEAD_T *)new_head_buffer;
    strcpy(new_head->partitinon_name, upgrade_file_info->upgrade_header.mtd_part);
    new_head->partitnon_addr = upgrade_file_info->upgrade_header.mtd_addr;
    new_head->partitnon_crc = upgrade_file_info->upgrade_header.file_crc32;
    new_head->partitnon_data_size = upgrade_file_info->upgrade_header.file_size;
    new_head->partitnon_size = upgrade_file_info->upgrade_header.mtd_size;
    new_head->update_flag = 1;
    new_head->magic = BACKUPS_HEAD_MAGIC;
    printf("[%s, %d] Save new partition, new head crc:0x%x, file size:0x%x\n", 
        __func__, __LINE__, new_head->partitnon_crc, new_head->partitnon_data_size);
    ret = erase_flash_all(BACKUPS_PARTITNON_NAME, NEW_HEAD_OFFSET, NEW_HEAD_SIZE);
    if (ret < 0)
    {
        /* 校验失败, 错误退出 */
        ERROR_PRT("erase_flash_all %s error!!\n", BACKUPS_PARTITNON_NAME);
        //return -1;
        goto save_new_part_info_end;
    }
    ret = flash_copy(BACKUPS_PARTITNON_NAME, NEW_HEAD_OFFSET, new_head_buffer, NEW_HEAD_SIZE);
    if (ret < 0)
    {
        /* 校验失败, 错误推出 */
        ERROR_PRT("flash_copy %s error!!\n", BACKUPS_PARTITNON_NAME);
        //return -1;
        goto save_new_part_info_end;
    }
save_new_part_info_end:
    if(new_head_buffer != NULL)
        free(new_head_buffer);
    return 0;
}
#endif
/******************************************************************************
 * 函数介绍:
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :
 *                               v1.0.0    2015.08.23  wangguixing
 *****************************************************************************/
 int upgrade_flash_by_buf(char *upgrade_buf, unsigned int buf_size)
 {
    int   ret         = 0;
    int   cnt         = 0;
    int   verify_cnt  = 0;
    int   upgrade_mtd = 0;
    int   mod_num     = 0;
    int   mtd_num     = 0;
    struct proc_mtd_info     mtd_info[MAX_MTD_NUM];
    struct proc_mtd_info     all_mtd;
    struct upgrade_file_info upgrade_info[MAX_MTD_NUM];

    memset(&mtd_info, 0, sizeof(mtd_info));
    memset(&all_mtd, 0, sizeof(all_mtd));
    memset(&upgrade_info, 0, sizeof(upgrade_info));

    if (NULL == upgrade_buf)
    {
        ERROR_PRT("upgrade_buf is NULL error!\n");
        return -1;
    }

    if (buf_size > MAX_NORFLASH_SIZE)
    {
        ERROR_PRT("file_size:0x%x is too large! MAX_NORFLASH_SIZE:0x%x\n", buf_size, MAX_NORFLASH_SIZE);
        return -1;
    }

    /* 初始化 升级进度 */
    set_upgrade_start(buf_size);

    /* 1. 校验升级文件 */
    ret = verify_upgrade_buff(upgrade_buf, buf_size);
    if (ret < 0)
    {
        ERROR_PRT("Call verify_upgrade_buff ret:%d \n", ret);
        return -1;
    }

    /* 2. 从升级文件中获取, 各个升级模块的位置和头信息, 用于升级 */
    ret = get_upgradeinfo_by_buff(upgrade_buf, buf_size, upgrade_info, &mod_num);
    if (ret < 0)
    {
        ERROR_PRT("Call get_upgradeinfo_by_buff error! ret:%d \n", ret);
        return -1;
    }
    DEBUG_PRT(" get_upgradeinfo_by_buff -> mod_num:%d \n\n", mod_num);

    /* 3.  获取本地linux 系统的ALL mtd分区信息, 用于分区大小等改变后的升级 */
    ret = get_all_mtd_info_by_proc(&all_mtd);
    if (ret < 0)
    {
        ERROR_PRT("Call get_all_mtd_info_by_proc error! ret:%d \n", ret);
        return -1;
    }

    /* 4. 获取本地linux 系统的 /proc/mtd 信息 */
    ret = get_mtdinfo_by_proc_mtd(mtd_info, &mtd_num);
    if (ret < 0)
    {
        ERROR_PRT("Call get_mtdinfo_by_proc_mtd error! ret:%d \n", ret);
        return -1;
    }

    /* 5. 根据升级模块头信息和/proc/mtd 进行升级 */
    for (cnt = 0; cnt < mod_num; cnt++)
    {
        printf("\n=============== [%d] mtd_name:%s ====================\n\n", cnt, upgrade_info[cnt].upgrade_header.mtd_name);

        /* 6. 判断升级分区和升级方式 */
        upgrade_mtd = judge_upgrade_by_info(mtd_info, cnt, &upgrade_info[cnt]);
        if (upgrade_mtd < 0)
        {
            ERROR_PRT("Call judge_upgrade_by_info error! upgrade_mtd:%d \n", upgrade_mtd);
            return -1;
        }
        DEBUG_PRT(" judge_upgrade_by_info cnt:%d  upgrade_mtd:%d \n", cnt, upgrade_mtd);

        /* 7. 读入升级镜像*/
        if ((upgrade_info[cnt].file_addr + upgrade_info[cnt].file_size) > buf_size)
        {
            ERROR_PRT("file_addr:%u + file_size:%u > buf_size:%u\n",
                                                        upgrade_info[cnt].file_addr,
                                                        upgrade_info[cnt].file_size,
                                                        buf_size);
            return -1;
        }

        /* 8. 强制卸载所升级的 MTD 分区 */
        //umount2("/mnt/jffs2_mtd", MNT_FORCE);

        /* 9. 对分区进行升级操作, 先校验, 再擦写, 最后写flash */
        if (UPGRADE_NORMAL_MTD == upgrade_mtd)
        {
            #ifdef MODULE_SUPPORT_UPGRADE_BACKUPS
            printf("[%s, %d] Backup upgrade mode:%d, save old start\n", __func__, __LINE__, upgrade_mtd);
            save_old_part_info(&upgrade_info[cnt]);
            printf("[%s, %d] Backup upgrade mode:%d, save old end\n", __func__, __LINE__, upgrade_mtd);
            #endif
            printf("\n===============erase and copy [%d] mtd_name:%s ====================\n\n", cnt, upgrade_info[cnt].upgrade_header.mtd_name);
            verify_cnt = 3;
            while (verify_cnt--)
            {
                /* 升级镜像和MTD分区内容不同, 进行升级 */
                ret = erase_flash_all(upgrade_info[cnt].upgrade_header.mtd_part,
                                      0,
                                      upgrade_info[cnt].upgrade_header.mtd_size);
                if (ret < 0)
                {
                    /* 校验失败, 错误退出 */
                    ERROR_PRT("erase_flash_all %s error!!\n", upgrade_info[cnt].upgrade_header.mtd_part);
                    //return -1;
                    continue;
                }

                ret = flash_copy(upgrade_info[cnt].upgrade_header.mtd_part,
                                 0,
                                 upgrade_buf + upgrade_info[cnt].file_addr,
                                 upgrade_info[cnt].file_size);
                if (ret < 0)
                {
                    /* 校验失败, 错误推出 */
                    ERROR_PRT("flash_copy %s error!!\n", upgrade_info[cnt].upgrade_header.mtd_part);
                    //return -1;
                    continue;
                }

                break;  /* 注意 若一次擦写成果后, 就退出while 循环.*/
            }
            #ifdef MODULE_SUPPORT_UPGRADE_BACKUPS
            printf("[%s, %d] Backup upgrade mode:%d, save new start\n", __func__, __LINE__, upgrade_mtd);
            //save new head
            save_new_part_info(&upgrade_info[cnt]);
            printf("[%s, %d] Backup upgrade mode:%d, save new end\n", __func__, __LINE__, upgrade_mtd);
            #endif
        }
        else if(UPGRADE_ALL_MTD == upgrade_mtd)
        {
            DEBUG_PRT("upgrade mtd:%s  name:%s  addr:0x%x size:0x%x\n", all_mtd.mtd_part, all_mtd.mtd_name,
                                                                        upgrade_info[cnt].upgrade_header.mtd_addr,
                                                                        upgrade_info[cnt].upgrade_header.mtd_size);
            printf("\n===============erase and copy [%d] mtd_name:%s ====================\n\n", cnt, upgrade_info[cnt].upgrade_header.mtd_name);
            #ifdef MODULE_SUPPORT_UPGRADE_BACKUPS
            printf("[%s, %d] Backup upgrade mode:%d, save old start\n", __func__, __LINE__, upgrade_mtd);
            save_old_part_info(&upgrade_info[cnt]);
            printf("[%s, %d] Backup upgrade mode:%d, save old end\n", __func__, __LINE__, upgrade_mtd);
            #endif
            verify_cnt = 3;
            while (verify_cnt--)
            {
                /* 格式化 ALL 分区里面的对应地址和长度 */
                ret = erase_flash_all(all_mtd.mtd_part,
                                      upgrade_info[cnt].upgrade_header.mtd_addr,
                                      upgrade_info[cnt].upgrade_header.mtd_size);
                if (ret < 0)
                {
                    /* 校验失败, 错误推出 */
                    ERROR_PRT("erase_flash_all %s error!!\n", upgrade_info[cnt].upgrade_header.mtd_part);
                    //return -1;
                    continue;
                }

                ret = flash_copy(all_mtd.mtd_part,
                                 upgrade_info[cnt].upgrade_header.mtd_addr,
                                 upgrade_buf + upgrade_info[cnt].file_addr,
                                 upgrade_info[cnt].file_size);
                if (ret < 0)
                {
                    /* 校验失败, 错误推出 */
                    ERROR_PRT("flash_copy %s error!!\n", all_mtd.mtd_part);
                    //return -1;
                    continue;
                }

                break;  /* 注意 若一次擦写成果后, 就退出while 循环.*/
            }
            
            #ifdef MODULE_SUPPORT_UPGRADE_BACKUPS            
            printf("[%s, %d] Backup upgrade mode:%d, save new start\n", __func__, __LINE__, upgrade_mtd);
            //save new head
            save_new_part_info(&upgrade_info[cnt]);            
            printf("[%s, %d] Backup upgrade mode:%d, save new end\n", __func__, __LINE__, upgrade_mtd);
            #endif
        }
        else
        {
            ERROR_PRT("Call judge_upgrade_by_info error! ret:%d \n", upgrade_mtd);
            return -1;
        }
    }


    return 0;
}


/******************************************************************************
 * 函数介绍:
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :
 *                               v1.0.0    2015.08.23  wangguixing
 *****************************************************************************/
int upgrade_flash_by_file(char *upgrade_file)
{
    int   ret         = 0;
    int   cnt         = 0;
    int   upgrade_mtd = 0;
    int   mod_num     = 0;
    int   mtd_num     = 0;
    int   pack_fd     = -1;
    char *pbuf        = NULL;
    struct proc_mtd_info     mtd_info[MAX_MTD_NUM];
    struct proc_mtd_info     all_mtd;
    struct upgrade_file_info upgrade_info[MAX_MTD_NUM];

    memset(&mtd_info, 0, sizeof(mtd_info));
    memset(&all_mtd, 0, sizeof(all_mtd));
    memset(&upgrade_info, 0, sizeof(upgrade_info));

    if (NULL == upgrade_file)
    {
        ERROR_PRT("upgrade_file is NULL error!\n");
        return -1;
    }

    /* 1. 校验升级文件 */
    ret = verify_upgrade_file(upgrade_file);
    if (ret < 0)
    {
        ERROR_PRT("Call verify_upgrade_file:%s error! ret:%d \n", upgrade_file, ret);
        return -1;
    }

    /* 2. 从升级文件中获取, 各个升级模块的位置和头信息, 用于升级 */
    ret = get_upgradeinfo_by_file(upgrade_file, upgrade_info, &mod_num);
    if (ret < 0)
    {
        ERROR_PRT("Call get_upgradeinfo_by_file error! ret:%d \n", ret);
        return -1;
    }
    DEBUG_PRT(" get_upgradeinfo_by_file -> mod_num:%d \n\n", mod_num);

    /* 3.  获取本地linux 系统的ALL mtd分区信息, 用于分区大小等改变后的升级 */
    ret = get_all_mtd_info_by_proc(&all_mtd);
    if (ret < 0)
    {
        ERROR_PRT("Call get_all_mtd_info_by_proc error! ret:%d \n", ret);
        return -1;
    }

    /* 4. 获取本地linux 系统的 /proc/mtd 信息 */
    ret = get_mtdinfo_by_proc_mtd(mtd_info, &mtd_num);
    if (ret < 0)
    {
        ERROR_PRT("Call get_mtdinfo_by_proc_mtd error! ret:%d \n", ret);
        return -1;
    }

    /* 5. 根据升级模块头信心和/proc/mtd 进行升级 */
    pack_fd = open(upgrade_file, O_RDONLY);
    if (pack_fd < 0)
    {
        ERROR_PRT("open %s ERROR! %s\n", upgrade_file, strerror(errno));
        return -1;
    }
    for (cnt = 0; cnt < mod_num; cnt++)
    {
        printf("\n=============== [%d] mtd_name:%s ====================\n\n", cnt, upgrade_info[cnt].upgrade_header.mtd_name);

        /* 6. 判断升级分区和升级方式 */
        upgrade_mtd = judge_upgrade_by_info(mtd_info, cnt, &upgrade_info[cnt]);
        if (upgrade_mtd < 0)
        {
            ERROR_PRT("Call judge_upgrade_by_info error! upgrade_mtd:%d \n", upgrade_mtd);
            ret = -1;
            goto upgrade_flash_err;
        }
        DEBUG_PRT(" judge_upgrade_by_info cnt:%d  upgrade_mtd:%d \n", cnt, upgrade_mtd);

        /* 7. 读入升级镜像*/
        if (NULL != pbuf)
        {
            free(pbuf);
            pbuf = NULL;
        }
        pbuf = malloc(upgrade_info[cnt].file_size + 2);
        if (NULL == pbuf)
        {
            ERROR_PRT("malloc pheader->file_size:%u ERROR! %s\n", upgrade_info[cnt].file_size, strerror(errno));
            ret = -1;
            goto upgrade_flash_err;
        }
        ret = lseek(pack_fd, upgrade_info[cnt].file_addr, SEEK_SET);
        if (ret < 0)
    	{
            ERROR_PRT("lseek %s ERROR! %s\n", upgrade_file, strerror(errno));
            ret = -1;
            goto upgrade_flash_err;
        }
        ret = read(pack_fd, pbuf, upgrade_info[cnt].file_size);
        if (ret < 0 || ret != upgrade_info[cnt].file_size)
        {
            ERROR_PRT("file_size:%u ret:%d %s\n", upgrade_info[cnt].file_size, ret, strerror(errno));
            goto upgrade_flash_err;
        }

        /* 8. 强制卸载所升级的 MTD 分区 */
        //umount2("/mnt/jffs2_mtd", MNT_FORCE);

        int verify_cnt = 3;
        /* 9. 对分区进行升级操作, 先校验, 再擦写, 最后写flash */
        if (UPGRADE_NORMAL_MTD == upgrade_mtd)
        {
            ret = verify_flash(upgrade_info[cnt].upgrade_header.mtd_part,
                               0,
                               pbuf,
                               upgrade_info[cnt].file_size);
            if (ret > 0)
            {
                /* 升级镜像和MTD分区内容不同, 进行升级 */
                DEBUG_PRT("verify_flash %s different! upgrade!!\n",
                                                upgrade_info[cnt].upgrade_header.mtd_part);
                verify_cnt = 3;
                while(verify_cnt --)
                {
                    ret = erase_flash_all(upgrade_info[cnt].upgrade_header.mtd_part,
                            0,
                            upgrade_info[cnt].upgrade_header.mtd_size);
                    if (ret < 0)
                    {
                        /* 校验失败, 错误推出 */
                        ERROR_PRT("erase_flash_all %s error!!\n", upgrade_info[cnt].upgrade_header.mtd_part);
                        continue;
                    }

                    ret = flash_copy(upgrade_info[cnt].upgrade_header.mtd_part,
                            0,
                            pbuf,
                            upgrade_info[cnt].file_size);
                    if (ret < 0)
                    {
                        /* 校验失败, 错误推出 */
                        ERROR_PRT("flash_copy %s error!!\n", upgrade_info[cnt].upgrade_header.mtd_part);
                        continue;
                    }
					break;
                }
            }
            else if (0 == ret)
            {
                /* 升级镜像和MTD分区内容一样,  不升级升级 */
                DEBUG_PRT("verify_flash %s the same image_file! Don't upgrage! ret:%d\n",
                                                upgrade_info[cnt].upgrade_header.mtd_part, ret);
                continue;
            }
            else
            {
                /* 校验失败, 错误退出 */
                ERROR_PRT("verify_flash %s error!!\n", upgrade_info[cnt].upgrade_header.mtd_part);
                goto upgrade_flash_err;
            }
        }
        else if(UPGRADE_ALL_MTD == upgrade_mtd)
        {
            DEBUG_PRT("upgrade mtd:%s  name:%s  addr:0x%x size:0x%x\n", all_mtd.mtd_part, all_mtd.mtd_name,
                                                                        upgrade_info[cnt].upgrade_header.mtd_addr,
                                                                        upgrade_info[cnt].upgrade_header.mtd_size);
            /* 格式化 ALL 分区里面的对应地址和长度 */
            ret = erase_flash_all(all_mtd.mtd_part,
                                  upgrade_info[cnt].upgrade_header.mtd_addr,
                                  upgrade_info[cnt].upgrade_header.mtd_size);
            if (ret < 0)
            {
                /* 校验失败, 错误推出 */
                ERROR_PRT("erase_flash_all %s error!!\n", upgrade_info[cnt].upgrade_header.mtd_part);
                goto upgrade_flash_err;
            }

            ret = flash_copy(all_mtd.mtd_part,
                             upgrade_info[cnt].upgrade_header.mtd_addr,
                             pbuf,
                             upgrade_info[cnt].file_size);
            if (ret < 0)
            {
                /* 校验失败, 错误推出 */
                ERROR_PRT("flash_copy %s error!!\n", all_mtd.mtd_part);
                goto upgrade_flash_err;
            }
            continue;  /////////////////////
        }
        else
        {
            ERROR_PRT("Call judge_upgrade_by_info error! ret:%d \n", upgrade_mtd);
            ret = -1;
            goto upgrade_flash_err;
        }
    }

    ret = 0;
upgrade_flash_err:
    if (NULL != pbuf)
    {
        free(pbuf);
    }
    if (pack_fd > 0)
    {
        close(pack_fd);
    }
    return ret;
}



