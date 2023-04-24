/*******************************************************************************
 *
 * Copyright (c)
 *
 *  \file flash_ctl.c
 *
 *  \brief update progress, and control the flash MTD dev.
 *     create version  v 1.0.0  wangguixing
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
#include <ctype.h>
#include <time.h>
#include <getopt.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/mount.h>

#include "flash_ctl.h"
#include "mtd-abi.h"
#include "mtd-user.h"

#include "utility_api.h"
#include "parser_cfg.h"
#include "partition_info.h"

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */

/* size of read/write buffer */
#define BUFSIZE (128 * 1024)


#define KB(x) ((x) / 1024)
#define PERCENTAGE(x,total) (((x) * 100) / (total))



/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */
/* None */

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */
/* None */


/* ========================================================================== */
/*                            Global Variables                                */
/* ========================================================================== */
/* None */


/* ========================================================================== */
/*                          Function Definitions                              */
/* ========================================================================== */

/******************************************************************************
 * 函数介绍:
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int get_mtd_info(char * mtd_device, struct mtd_info_user *data)
{
    int  fd;
    int  ret;
    struct mtd_info_user mtd_info;

    memset(&mtd_info, 0, sizeof(mtd_info));

    if (NULL == mtd_device)
    {
        ERROR_PRT("mtd_device is NULL!\n");
        return -1;
    }

    /* get MTD info form the flash device */
    if ((fd = open(mtd_device, O_RDWR)) < 0)
    {
        ERROR_PRT("open %s error! %s\n", mtd_device, strerror(errno));
        return -1;
    }

    if ((ret = ioctl(fd, MEMGETINFO, &mtd_info)) != 0)
    {
        ERROR_PRT("%s:  unable to get MTD device info\n", mtd_device);
        close(fd);
        return ret;
    }
    close(fd);
    if(data)
        memcpy(data, &mtd_info, sizeof(struct mtd_info_user));

    //DEBUG_PRT("mtd_info.type      :%d\n",  mtd_info.type);
    //DEBUG_PRT("mtd_info.flags     :%u\n",  mtd_info.flags);
    //DEBUG_PRT("mtd_info.size      :%u\n",  mtd_info.size);
    //DEBUG_PRT("mtd_info.erasesize :%u\n",  mtd_info.erasesize);
    //DEBUG_PRT("mtd_info.writesize :%u\n",  mtd_info.writesize);
    //DEBUG_PRT("mtd_info.oobsize   :%u\n",  mtd_info.oobsize);
    //DEBUG_PRT("mtd_info.ecctype   :%u\n",  mtd_info.ecctype);
    //DEBUG_PRT("mtd_info.eccsize   :%u\n",  mtd_info.eccsize);
    return mtd_info.size;
}


/******************************************************************************
 * 函数介绍:
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int dump_flash(char * device, char * file_name)
{
    int ret       = 0;
    int read_len  = 0;
    int tmp_len   = 0;
    int count     = 0;
    int dev_fd    = -1;
    int dump_fd   = -1;
    int result    = 0;
    unsigned int  size = 0;
    unsigned char dest[BUFSIZE] = {0};

    struct mtd_info_user mtd_info;

    memset(&mtd_info, 0, sizeof(mtd_info));

    if (NULL == device || NULL == file_name)
    {
        ERROR_PRT("device:%p file_name:%p \n", device, file_name);
        return -1;
    }

    /* get MTD info form the flash device */
    if ((dev_fd = open(device, O_SYNC | O_RDWR)) < 0)
    {
        ERROR_PRT("open %s error!\n", file_name);
        return -1;
    }

    if ((ret = ioctl(dev_fd, MEMGETINFO, &mtd_info)) < 0)
    {
        ERROR_PRT("dev_fd[%d] ioctl MEMGETINFO error!\n", dev_fd);
        close(dev_fd);
        return -1;
    }

    if ((dump_fd = open(file_name, O_WRONLY|O_CREAT)) < 0)
    {
        ERROR_PRT("open %s error!\n", device);
        close(dev_fd);
        return -1;
    }

    size    = mtd_info.size;
    count   = BUFSIZE;
    while(size)
    {
        if (size < BUFSIZE)
        {
            count = size;
        }

        if ((read_len = read(dev_fd, dest, count)) < 0)
        {
            ERROR_PRT("dev_fd[%d] read error!\n", dev_fd);
            close(dev_fd);
            close(dump_fd);
            return -1;
        }

        do{
            if ((result = write(dump_fd, dest, read_len)) < 0)
            {
                ERROR_PRT(" write mtd data error!  size:%u \n", size);
                close(dev_fd);
                close(dump_fd);
                return -1;
            }
            tmp_len += result;
        }while(read_len > tmp_len);
        size    -= read_len;
    }

    close(dev_fd);
    close(dump_fd);
    return 0;
}


 /*校验数据*/
 /******************************************************************************
 * 函数介绍:
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int verify_flash(char * device, unsigned int mtd_offset, char * file_buff, unsigned int file_size)
{
    int ret;
    int count;
    int dev_fd;
    int result    = 0;
    int read_len  = 0;
    unsigned int  size = 0;
    unsigned char dest[BUFSIZE] = {0};
    char * pos = NULL;
    int written;
    struct mtd_info_user mtd_info;

    memset(&mtd_info, 0, sizeof(mtd_info));

    if (NULL == device || NULL == file_buff)
    {
        ERROR_PRT("device:%p file_buff:%p \n", device, file_buff);
        return -1;
    }

    /* get MTD info form the flash device */
    if ((dev_fd = open(device, O_SYNC | O_RDWR)) < 0)
    {
        ERROR_PRT("open %s error!\n", device);
        return -1;
    }

    if ((ret = ioctl(dev_fd, MEMGETINFO, &mtd_info)) < 0)
    {
        ERROR_PRT("dev_fd[%d] ioctl MEMGETINFO error!\n", dev_fd);
        close(dev_fd);
        return -1;
    }

    /* check update file_size and mtd_x size */
    if ((file_size + mtd_offset) > mtd_info.size)
    {
        ERROR_PRT("Input upgrade file of size is too big! update_size:%u  mtd.size:%u \n", (file_size + mtd_offset), mtd_info.size);
        close(dev_fd);
        return -1;
    }

    if (lseek(dev_fd, mtd_offset, SEEK_SET) < 0)
    {
        ERROR_PRT("dev_fd[%d] lseek error!\n", dev_fd);
        close(dev_fd);
        return -1;
    }

    /* verify that flash == file data */
    size    = file_size;
    count   = BUFSIZE;
    written = 0;
    pos     = file_buff;
    while (size)
    {
        if (size < BUFSIZE)
        {
            count = size;
        }
        DEBUG_PRT("\rVerifying data: %dk/%uk (%u%%)", KB(written + count),
                                                      KB(file_size),
                                                      PERCENTAGE(written + count, file_size));

        if ((read_len = read(dev_fd, dest, count)) < 0)
        {
            ERROR_PRT("dev_fd[%d] read error!\n", dev_fd);
            close(dev_fd);
            return -1;
        }

        if ((result = memcmp(pos, dest, read_len)) != 0)
        {
            ERROR_PRT("Verifying mtd data error! written:%u [start:%p pos:%p] size:%u\n", written, file_buff, pos, size);
            ERROR_PRT("pos:[0x%x] [0x%x] [0x%x] [0x%x]\n", *pos, *(pos+1), *(pos+2), *(pos+3));
            ERROR_PRT("dst:[0x%x] [0x%x] [0x%x] [0x%x]\n", *dest, *(dest+1), *(dest+2), *(dest+3));
            close(dev_fd);
            return -2;
        }
        written += read_len;
        pos     += read_len;
        size    -= read_len;
        usleep(10000);
    }

    close(dev_fd);
    return 0;
}


/*****************************************************************************
 * 函数介绍:
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int erase_flash_mtd(char * mtd_device, unsigned int file_offset)
{
    int  fd;
    int  ret;
    erase_info_t erase;
    struct mtd_info_user mtd_info;

    memset(&erase, 0, sizeof(erase));
    memset(&mtd_info, 0, sizeof(mtd_info));

    if (NULL == mtd_device)
    {
        ERROR_PRT("mtd_device is NULL!\n");
        return -1;
    }

    /* get MTD info form the flash device */
    if ((fd = open(mtd_device, O_RDWR)) < 0)
    {
        ERROR_PRT("open %s error! %s\n", mtd_device, strerror(errno));
        return -1;
    }

    if ((ret = ioctl(fd, MEMGETINFO, &mtd_info)) != 0)
    {
        ERROR_PRT("%s:  unable to get MTD device info\n", mtd_device);
        close(fd);
        return ret;
    }

    if (lseek(fd, file_offset, SEEK_SET) < 0)
    {
        ERROR_PRT("lseek %s error! fd:%d %s\n", mtd_device, fd, strerror(errno));
        close(fd);
        return -1;
    }

    /* get the flash mtd erase size */
    erase.length = mtd_info.erasesize;

    DEBUG_PRT("Start erase flash:%s  erase_size:0x%x\n", mtd_device, erase.length);

    /* start erase the mtd partition  */
    for (erase.start = file_offset; erase.start < mtd_info.size; erase.start += mtd_info.erasesize)
    {
        loff_t offset = erase.start;
        if ((ret = ioctl(fd, MEMGETBADBLOCK, &offset)) < 0)
        {
            if (errno != EOPNOTSUPP)
            {
                ERROR_PRT("\n %s: MTD get bad block failed: %s\n",  mtd_device, strerror(errno));
                close(fd);
                return ret;
            }
        }
        else if (ret > 0)
        {
            DEBUG_PRT("\nSkipping bad block at 0x%08x  ret:%d\n", erase.start, ret);
            continue;
        }

        DEBUG_PRT("Erasing %d Kibyte @ %x -- %2llu %% complete.\n",
                                            mtd_info.erasesize / 1024,
                                            erase.start,
                                            (unsigned long long)(erase.start + erase.length) * 100 / mtd_info.size);

        if ((ret = ioctl(fd, MEMERASE, &erase)) != 0)
        {
            ERROR_PRT("\n %s: MTD Erase failure: %s  ret:%d\n", mtd_device, strerror(errno), ret);
            continue;
        }
    }

    DEBUG_PRT("\nSkipping bad block at 0x%08x  ret:%d\n", erase.start, ret);
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
int erase_flash_all(char * mtd_device, unsigned int erase_addr, unsigned int erase_size)
{
    int  dev_fd;
    int  ret;
    int  blocks     = 0;
    int  cnt        = 0;
    erase_info_t erase;
    struct mtd_info_user mtd_info;

    memset(&erase, 0, sizeof(erase));
    memset(&mtd_info, 0, sizeof(mtd_info));

    if (NULL == mtd_device)
    {
        ERROR_PRT("mtd_device is NULL!\n");
        return -1;
    }

    /* get MTD info form the flash device */
    if ((dev_fd = open(mtd_device, O_RDWR)) < 0)
    {
        ERROR_PRT("open %s error! %s\n", mtd_device, strerror(errno));
        return -1;
    }

    if ((ret = ioctl(dev_fd, MEMGETINFO, &mtd_info)) != 0)
    {
        ERROR_PRT("%s:  unable to get MTD device info\n", mtd_device);
        close(dev_fd);
        return ret;
    }

    if ((erase_addr + erase_size) > mtd_info.size )
    {
        ERROR_PRT("erase_size biger than mtd_size!  offset:0x%x + erase_size:0x%x = [0x%x] > mtd_size:0x%x\n",
                                            erase_addr, erase_size, (erase_addr + erase_size), mtd_info.size);
        close(dev_fd);
        return -1;
    }

    /* get the flash mtd erase size */
    erase.start  = ((erase_addr + mtd_info.erasesize - 1) / mtd_info.erasesize) * mtd_info.erasesize;
    erase.length = mtd_info.erasesize;
    if (0 == erase_size)
    {
        erase_size = mtd_info.size - erase.start;
    }
    blocks = (erase_size + mtd_info.erasesize - 1) / mtd_info.erasesize;

    DEBUG_PRT("Start erase flash:%s [blocks:%d erase.length:0x%x] earse_start:0x%x  earse_size:0x%x\n",
                                                  mtd_device, blocks, erase.length, erase.start, erase_size);

    /* start erase the mtd partition  */
    for (cnt = 1; cnt <= blocks; cnt++)
	{
        printf("\rErasing blocks: %2d/%2d (%3d%%)\n", cnt, blocks, PERCENTAGE(cnt,blocks));
		if (ioctl (dev_fd, MEMERASE, &erase) < 0)
		{
			printf("\n");
			ERROR_PRT("While erasing blocks 0x%.8x-0x%.8x on %s:\n",
                                                    (unsigned int)erase.start,
                                					(unsigned int)(erase.start + erase.length),
                                					mtd_device);
            close(dev_fd);
			return -1;
		}
		erase.start += mtd_info.erasesize;
        usleep(10000);
	}

    close(dev_fd);
    return 0;
}



/* 往flash  里面拷贝数据*/
/******************************************************************************
 * 函数介绍:
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int flash_copy(char * mtd_device, unsigned int mtd_offset, char * file_buff, unsigned int file_size)
{
    int ret       = 0;
    int dev_fd    = -1;
    int count     = 0;
    int result    = 0;
    unsigned int size    = 0;
    unsigned int written = 0;
    char * pos  = NULL;
    struct mtd_info_user mtd_info;

    memset(&mtd_info, 0, sizeof(mtd_info));

    if (NULL == mtd_device || NULL == file_buff)
    {
        ERROR_PRT("device:%p  file_buff:%p \n", mtd_device, file_buff);
        return -1;
    }

    /* get some info form the flash device */
    if ((dev_fd = open(mtd_device, O_SYNC | O_RDWR)) < 0)
    {
        ERROR_PRT("open %s error! %s\n", mtd_device, strerror(errno));
        return -1;
    }

    if ((ret = ioctl(dev_fd, MEMGETINFO, &mtd_info)) < 0)
    {
        ERROR_PRT("ioctl fd:%d MEMGETINFO error! %s\n", dev_fd, strerror(errno));
        close(dev_fd);
        return ret;
    }

    /* check update file_size and mtd_x size */
    if ((file_size + mtd_offset) > mtd_info.size)
    {
        ERROR_PRT("file too big! mtd:size (%d), file:size (%d) file:offset(%d)\n", mtd_info.size, file_size, mtd_offset);
        close(dev_fd);
        return -1;
    }

    if (lseek(dev_fd, mtd_offset, SEEK_SET) < 0)
    {
        ERROR_PRT("lseek dev_fd:%d error! \n", dev_fd);
        close(dev_fd);
        return -1;
    }

    DEBUG_PRT("Start copy flash:%s  mtd_offset:0x%x  file_size:0x%x\n", mtd_device, mtd_offset, file_size);

    /*  Start write the entire file to flash  */
    size = file_size;
    count = BUFSIZE;
    written = 0;
    pos = file_buff;
    while (size)
    {
        if (size < BUFSIZE)
        {
            count = size;
        }
        DEBUG_PRT("Writing data: %dk/%uk (%u%%)\n", KB(written + count),
                                                    KB(file_size),
                                                    PERCENTAGE(written + count, file_size));

        if ((result = write(dev_fd, pos, count)) < 0)
        {
            ERROR_PRT("wirte flash error! dev_fd:%d, pos:%p, count:%d  %s\n",dev_fd, pos, count, strerror(errno));
            close(dev_fd);
            return result;
        }

        pos     += result;
        written += result;
        size    -= result;
        usleep(10000);
    }

    /* verify write date is right */
    ret = verify_flash(mtd_device, mtd_offset, file_buff, file_size);
    if (0 != ret)
    {
        ERROR_PRT("verify the %s error! ret:%d \n", mtd_device, ret);
        close(dev_fd);
        return ret;
    }

    close(dev_fd);
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
int make_dev_info_crc32(struct device_info_mtd *pdevice_info)
{
    int crc32_val = 0;
    char *p = NULL;

    if (NULL == pdevice_info)
    {
        ERROR_PRT("pdevice_info:%p is NULL!\n", pdevice_info);
        return -1;
    }

    p = (char *)pdevice_info;
    crc32_val = utility_crc32(0, p + sizeof(unsigned int), sizeof(struct device_info_mtd) - sizeof(unsigned int));

    pdevice_info->crc32_val = crc32_val;

    DEBUG_PRT("device_info->crc32_val:0x%x \n", crc32_val);

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
int check_dev_info_crc32(struct device_info_mtd *pdevice_info)
{
    int crc32_val = 0;
    char *p = NULL;

    if (NULL == pdevice_info)
    {
        ERROR_PRT("pdevice_info:%p is NULL!\n", pdevice_info);
        return -1;
    }

    p = (char *)pdevice_info;
    crc32_val = utility_crc32(0, p + sizeof(unsigned int), sizeof(struct device_info_mtd) - sizeof(unsigned int));

    DEBUG_PRT("crc32_val:0x%x  device_info->crc32_val:0x%x \n", crc32_val, pdevice_info->crc32_val);

    if (pdevice_info->crc32_val != crc32_val)
    {
        ERROR_PRT("check device info crc val error!\n");
        return -1;
    }

    return 0;
}


#define PARAM_BLOCK_SIZE    (5 * 1024)
#define PARAM_NAME_LEN      32
#define PARAM_MAGIC_LEN     32
#define PARAM_MAGIC "PARAM_USED"
struct param_head{
    char  magic[PARAM_MAGIC_LEN];        /* 幻数 */
    char name[PARAM_NAME_LEN];/*param name*/
    unsigned int crc32;/*crc valu*/
    int block_size;//5K 包括头
    int data_len; /*真实数据大小*/
    char notuse[64];/*目前没有用*/
};

static pthread_mutex_t mtd_reserve_lock = PTHREAD_MUTEX_INITIALIZER;

/*mtd:YUNNI_P2P+ULUCU_P2P */
int save_info_to_mtd_reserve(RESERVE_DATA_TYPE type, void* data, int len)
{
    int fd = -1;
    int offset = 0;
    int mtd_size = 0;
    struct param_head *r_head, w_head;
    char name[PARAM_NAME_LEN];
    char *buffer = NULL, *pbuffer = NULL;
    struct mtd_info_user mtd_info;

    memset(&mtd_info, 0, sizeof(mtd_info));

    pthread_mutex_lock(&mtd_reserve_lock);
    if(get_mtd_info(MTD_RESERV_NAME, &mtd_info) <= 0)
    {
        pthread_mutex_unlock(&mtd_reserve_lock);
        return -1;
    }
    else
        mtd_size = mtd_info.size;

    fd = open(MTD_RESERV_NAME, O_RDWR);
    if(fd < 0)
    {
        ERROR_PRT("open %s error\n",MTD_RESERV_NAME);
        pthread_mutex_unlock(&mtd_reserve_lock);
        return -1;
    }
    switch(type)
    {
        case MTD_YUNNI_P2P:
            memset(name, 0, PARAM_NAME_LEN);
            snprintf(name,PARAM_NAME_LEN, YNNI_P2P_NAME);
            break;
        case MTD_ULUCU_P2P:
            memset(name, 0, PARAM_NAME_LEN);
            snprintf(name,PARAM_NAME_LEN, ULUCU_P2P_NAME);
            break;
        case MTD_GOKE_INFO:
            memset(name, 0, PARAM_NAME_LEN);
            snprintf(name,PARAM_NAME_LEN, GOKE_INFO);
            break;
    	case MTD_DANA_P2P:
    	     memset(name,0,PARAM_NAME_LEN);
    	     snprintf(name,PARAM_NAME_LEN, DANA_P2P_NAME);
    	     break;
    	case MTD_TUTK_P2P:
    	     memset(name,0,PARAM_NAME_LEN);
    	     snprintf(name,PARAM_NAME_LEN, TUTK_P2P_NAME);
    	     break;
        case MTD_MTWS_P2P:
    	     memset(name,0,PARAM_NAME_LEN);
    	     snprintf(name,PARAM_NAME_LEN, MTWS_P2P_NAME);
    	     break;
        case MTD_GB28181_INFO:
            memset(name,0,PARAM_NAME_LEN);
            snprintf(name,PARAM_NAME_LEN, GB28181_NAME);
            break;
        case MTD_TEST_RESULT:
              memset(name,0,PARAM_NAME_LEN);
              snprintf(name,PARAM_NAME_LEN, TEST_RESULT_NAME);
              break;
        case MTD_AF_OFFSET:
            memset(name,0,PARAM_NAME_LEN);
            snprintf(name,PARAM_NAME_LEN, AF_OFFSET_NAME);
            break;
        case MTD_MD5_INFO:
            memset(name,0,PARAM_NAME_LEN);
            snprintf(name,PARAM_NAME_LEN, MD5_OFFSET_NAME);
            break;
        case MTD_KEY_INFO:
            memset(name,0,PARAM_NAME_LEN);
            snprintf(name,PARAM_NAME_LEN, KEY_NAME);
            break;
        case MTD_KEY_SIGNED_INFO:
            memset(name,0,PARAM_NAME_LEN);
            snprintf(name,PARAM_NAME_LEN, KEY_SIGNED_NAME);
            break;
        default:
            ERROR_PRT("unknow param type:%x\n", type);
            goto error;
    }
    memset(&w_head, 0, sizeof(struct param_head));
    snprintf(w_head.magic,sizeof(w_head.magic), PARAM_MAGIC);
    snprintf(w_head.name,sizeof(w_head.name), name);

    DEBUG_PRT("save mtd no crc------\n");
    //w_head.crc32 = utility_crc32(0, data, len);
    w_head.block_size = PARAM_BLOCK_SIZE;
    w_head.data_len = sizeof(struct device_info_mtd);

    //read all data to buffer
    buffer = malloc(mtd_size);
    if(!buffer)
        goto error;
    pbuffer = buffer;
    if(read(fd, buffer, mtd_size) != mtd_size)
        goto error;
    //find head
    while(offset <= (mtd_size - 1))
    {
        pbuffer = buffer + offset;
        r_head = (struct param_head*)pbuffer;
        if(!strncmp(r_head->name, name, PARAM_NAME_LEN)){ //如果写了
            //find ok, update head and data
            memcpy(pbuffer, &w_head, sizeof(struct param_head));
            pbuffer += sizeof(struct param_head);
            memcpy(pbuffer, data, len);
            break;
        }else{ // 如果没写，比如空
            if(strncmp(r_head->magic, PARAM_MAGIC, PARAM_MAGIC_LEN))
            {
                DEBUG_PRT("!!!! offset:%d type:%d !!!!\n", offset, type);
                //当前块已经是末尾
                memcpy(pbuffer, &w_head, sizeof(struct param_head));
                pbuffer += sizeof(struct param_head);
                memcpy(pbuffer, data, len);
                break;
            }else{
                //jump data len and  read again
                offset += PARAM_BLOCK_SIZE;
                if(offset >= (mtd_size - 1))
                    goto error;
            }
        }
    }
    if(offset >= (mtd_size - 1))
        goto error;
    close(fd);
    //erase flash
    erase_flash_mtd(MTD_RESERV_NAME, 0);
    //write buffer data to flash
    flash_copy(MTD_RESERV_NAME, 0, buffer, mtd_size);
    DEBUG_PRT("set reserve mtd %s info ok\n",MTD_RESERV_NAME);
    if(buffer)
        free(buffer);
    pthread_mutex_unlock(&mtd_reserve_lock);
    return 0;
error:
    ERROR_PRT("set %s info error\n",MTD_RESERV_NAME);
    if(buffer)
        free(buffer);
    close(fd);
    pthread_mutex_unlock(&mtd_reserve_lock);
    return -1;
}

/*mtd:YUNNI_P2P+ULUCU_P2P */
int load_info_to_mtd_reserve(RESERVE_DATA_TYPE type, void* data, int len)
{
    static int mtd_size = 0;
    int fd = -1;
    off_t offset = 0;
    struct param_head r_head;
    char name[PARAM_NAME_LEN] = {0};
    struct mtd_info_user mtd_info;

    memset(&r_head, 0, sizeof(r_head));
    memset(&mtd_info, 0, sizeof(mtd_info));
    pthread_mutex_lock(&mtd_reserve_lock);
    if(get_mtd_info(MTD_RESERV_NAME, &mtd_info) <= 0)
    {
        pthread_mutex_unlock(&mtd_reserve_lock);
        return -1;
    }
    else
        mtd_size = mtd_info.size;

    fd = open(MTD_RESERV_NAME, O_RDWR);
    if(fd < 0)
    {
        ERROR_PRT("open %s error\n", MTD_RESERV_NAME);
        pthread_mutex_unlock(&mtd_reserve_lock);
        return -1;
    }

    switch(type)
    {
        case MTD_YUNNI_P2P:
            memset(name, 0, PARAM_NAME_LEN);
            snprintf(name,PARAM_NAME_LEN, YNNI_P2P_NAME);
            DEBUG_PRT("YNNI_P2P\n");
            break;
        case MTD_ULUCU_P2P:
            memset(name, 0, PARAM_NAME_LEN);
            snprintf(name,PARAM_NAME_LEN, ULUCU_P2P_NAME);
            DEBUG_PRT("ULUCU_P2P\n");
            break;
        case MTD_GOKE_INFO:
            memset(name, 0, PARAM_NAME_LEN);
            snprintf(name,PARAM_NAME_LEN, GOKE_INFO);
            DEBUG_PRT("GOKE_INFO\n");
            break;
        case MTD_DANA_P2P:
            memset(name,0,PARAM_NAME_LEN);
            snprintf(name,PARAM_NAME_LEN,DANA_P2P_NAME);
            DEBUG_PRT("DANA_P2P\n");
            break;
        case MTD_TUTK_P2P:
            memset(name,0,PARAM_NAME_LEN);
            snprintf(name,PARAM_NAME_LEN,TUTK_P2P_NAME);
            DEBUG_PRT("TUTK_P2P\n");
            break;
        case MTD_MTWS_P2P:
            memset(name,0,PARAM_NAME_LEN);
            snprintf(name,PARAM_NAME_LEN,MTWS_P2P_NAME);
            DEBUG_PRT("MTWS_P2P\n");
            break;
        case MTD_GB28181_INFO:
            memset(name,0,PARAM_NAME_LEN);
            snprintf(name,PARAM_NAME_LEN,GB28181_NAME);
            DEBUG_PRT("GB28181_INFO\n");
            break;
        case MTD_TEST_RESULT:
            memset(name,0,PARAM_NAME_LEN);
            snprintf(name,PARAM_NAME_LEN, TEST_RESULT_NAME);
            DEBUG_PRT("TEST_RESULT\n");
            break;
        case MTD_AF_OFFSET:
            memset(name,0,PARAM_NAME_LEN);
            snprintf(name,PARAM_NAME_LEN, AF_OFFSET_NAME);
            DEBUG_PRT("AF_OFFSET\n");
            break;
        case MTD_MD5_INFO:
            memset(name,0,PARAM_NAME_LEN);
            snprintf(name,PARAM_NAME_LEN, MD5_OFFSET_NAME);
            DEBUG_PRT("MD5_INFO\n");
            break;
        case MTD_KEY_INFO:
            memset(name,0,PARAM_NAME_LEN);
            snprintf(name,PARAM_NAME_LEN, KEY_NAME);
            DEBUG_PRT("KEY_INFO\n");
            break;
        case MTD_KEY_SIGNED_INFO:
            memset(name,0,PARAM_NAME_LEN);
            snprintf(name,PARAM_NAME_LEN, KEY_SIGNED_NAME);
            DEBUG_PRT("KEY_SIGNED_INFO\n");
            break;
        default:
            ERROR_PRT("unknow type:%x\n", type);
            goto error;
    }

    //find head
    while(offset <= (mtd_size - 1))
    {
        if(read(fd, &r_head, sizeof(struct param_head)) != sizeof(struct param_head)){
            ERROR_PRT("read mtd2 error\n");
            goto error;
        }

        if(!strncmp(r_head.name, name, PARAM_NAME_LEN)){
            //find ok, read data

            if(read(fd, data, len) != len){
                ERROR_PRT("read mtd2 error\n");
                goto error;
            }
            DEBUG_PRT("load mtd no crc------\n");
            #if 0
            unsigned int  crc32 = utility_crc32(0, data, len);
            if(crc32 != r_head.crc32){
                ERROR_PRT("crc32 error len:%d\n", len);
                goto error;
            }
            #endif
            break;

        }else{
            //当前块已经是末尾
            if(strncmp(r_head.magic, PARAM_MAGIC, PARAM_MAGIC_LEN))
            {
                ERROR_PRT("find end\n");
                goto error;
            }else{
                //jump data read again
                off_t jump = PARAM_BLOCK_SIZE - sizeof(struct param_head);
                offset += PARAM_BLOCK_SIZE;
                if(offset >= (mtd_size - 1)){
                    ERROR_PRT("find end\n");
                    goto error;
                }
                lseek(fd, jump, SEEK_CUR);
            }
        }
    }
    if(offset >= (mtd_size - 1)){
        ERROR_PRT("find end\n");
        goto error;
    }
    DEBUG_PRT("load %s info ok\n",MTD_RESERV_NAME);
    close(fd);
    pthread_mutex_unlock(&mtd_reserve_lock);
    return 0;
error:
    ERROR_PRT("load %s info:%s. error\n",MTD_RESERV_NAME, name);
    close(fd);
    pthread_mutex_unlock(&mtd_reserve_lock);
    return -1;
}


#if 0  //Just for testing!
/* 对ALL_MTD分区进行操作 */
int main(void)
{
    int fd     = -1;
    int ret    = 0;
    unsigned int cnt         = 0;
    unsigned int file_size   = 0;
    unsigned int file_offset = 0;
    char *pos = NULL;
    char file_buf[4 * 1024 * 1024];

    display_mtd_info("/dev/mtd4");

#if 0  /* MTD4 的操作 */
    erase_flash_mtd("/dev/mtd4", 0);
    //erase_flash_new("/dev/mtd4", 0, 0);

    file_offset = 0x100000;
    file_size   = 0x80000;
    memset (file_buf, 0xaa, file_size);
    DEBUG_PRT("file_size:0x%x  file_offset:0x%x\n", file_size, file_offset);
    flash_copy("/dev/mtd4", file_offset, file_buf, file_size);

    dump_flash("/dev/mtd4", "dump_erase_file_0xAA.hex");
#endif

#if 0
    file_offset = 0x30000 + 0x10000 + 0x200000 + 0x300000 + 0x120000;
    file_size   = 0x20000;
    memset (file_buf, 0x55, file_size);
    DEBUG_PRT("file_size:0x%x  file_offset:0x%x\n", file_size, file_offset);

    erase_flash_new("/dev/mtd4", file_size, 0x120000);

    flash_copy("/dev/mtd5", file_offset, file_buf, file_size);

    dump_flash("/dev/mtd4", "dump_erase_file_0x55.hex");
#endif

#if 0
    file_offset = 0x30000 + 0x10000 + 0x200000 + 0x300000 + 0x120000;
    file_size   = 0x20000;
    memset (file_buf, 0x18, file_size);
    DEBUG_PRT("file_size:0x%x  file_offset:0x%x\n", file_size, file_offset);

    erase_flash_new("/dev/mtd5", file_size, file_offset);

    flash_copy("/dev/mtd5", file_offset, file_buf, file_size);

    dump_flash("/dev/mtd4", "dump_erase_file_0x18.hex");
#endif

#if 1

    fd = open("test.jffs2", O_RDONLY);
    if (fd < 1)
    {
        ERROR_PRT("open ./test.jffs2 error!\n");
    }
    cnt = 0;
    pos = file_buf;
    while(1)
    {
        ret = read(fd, pos, 0x1000);

        cnt += ret;
        pos += ret;

        //DEBUG_PRT("read cnt:%u  ret:%d\n", cnt, ret);
        if((ret <= 0) || (cnt >= (4*1024*1024)))
        {
            break;
        }
    }
    DEBUG_PRT("read cnt:%u[0x%x]  ret:%d\n", cnt, cnt, ret);

    ret = lseek(fd, 0, SEEK_END);
    if (ret < 0)
    {
        ERROR_PRT("lseek /root/test.jffs2 error!  ret:%d\n", ret);
        return -1;
    }

    file_size   = ret;
    file_offset = 0x30000 + 0x10000 + 0x200000 + 0x300000;

    DEBUG_PRT("file_size:0x%x  file_offset:0x%x\n", file_size, file_offset);

    erase_flash_all("/dev/mtd5", file_offset, file_size);

    flash_copy("/dev/mtd5", file_offset, file_buf, file_size);

    dump_flash("/dev/mtd4", "dump_erase_file_jffs2.hex");
#endif

    return 0;
}

#endif



