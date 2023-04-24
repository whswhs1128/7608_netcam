/*******************************************************************************
 *
 * Copyright (c)
 *
 *  \file flash_ctl.h
 *
 *  \brief update progress, and control the flash MTD dev
 * 
 *   create version  v 1.0.0  wangguixing
 *
 ******************************************************************************/

#ifndef _FLASH_CTL_H_
#define _FLASH_CTL_H_

/* ========================================================================== */
/*                             Include Files                                  */
/* ========================================================================== */
/* None */

#ifdef __cplusplus
extern "C" {
#endif

/* ========================================================================== */
/*                           Macros & Typedefs                                */
/* ========================================================================== */
#define MTD_UBOOT           "/dev/mtd0"
#define MTD_UBOOT_ENV       "/dev/mtd1"
#define MTD_USER            "/dev/mtd2"
#define MTD_CONFIG          "/dev/mtd3"
#define MTD_KERNEL          "/dev/mtd4"
#define MTD_ROOTFS          "/dev/mtd5"
#define MTD_RESOURCE        "/dev/mtd6"
#define MTD_ALL             "/dev/mtd6"

#define MTD_UBOOT_NAME      "uboot"
#define MTD_UBOOT_ENV_NAME  "ubootenv"
#define MTD_USER_NAME       "user"
#define MTD_CONFIG_NAME     "config"
#define MTD_KERNEL_NAME     "kernel"
#define MTD_ROOTFS_NAME     "rootfs"
#define MTD_RESOURCE_NAME   "resource"
#define MTD_ALL_NAME        "all"

#define DEV_MTD0            "/dev/mtd0"
#define DEV_MTD1            "/dev/mtd1"
#define DEV_MTD2            "/dev/mtd2"
#define DEV_MTD3            "/dev/mtd3"
#define DEV_MTD4            "/dev/mtd4"
#define DEV_MTD5            "/dev/mtd5"
#define DEV_MTD6            "/dev/mtd6"
#define DEV_MTD7            "/dev/mtd7"
#define DEV_MTD8            "/dev/mtd8"

#define MAC_ADDR_STRSIZE    32
#define DEVICE_ID_STRSIZE   32


#define DEBUG_PRT(fmt, args...) \
do{ \
    printf("[FUN]%s [LINE]%d  "fmt, __FUNCTION__, __LINE__, ##args); \
}while(0)


#define ERROR_PRT(fmt, args...) \
do{ \
    printf("\033[0;32;31m ERROR! [FUN]%s [LINE]%d  "fmt"\033[0m", __FUNCTION__, __LINE__, ##args); \
}while(0)


/* ========================================================================== */
/*                         Structure Declarations                             */
/* ========================================================================== */
struct device_info_mtd
{
    unsigned int crc32_val;  /* crc32 校验值  */
    char eth_mac_addr[32];   /* mac 地址          */
    char wifi_mac_addr[32];  /* mac 地址          */
    char device_id[32];      /* device id 号         */
    char reserve[256];       /* 保留位           */
};


/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/******************************************************************************
 * 函数介绍: 
 * 输入参数: 
 *                           
 * 输出参数: 
 * 返回值      : 
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int display_mtd_info(char * mtd_device);


/******************************************************************************
 * 函数介绍: 
 * 输入参数: 
 *                           
 * 输出参数: 
 * 返回值      : 
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int dump_flash(char * device, char * file_name);


/******************************************************************************
 * 函数介绍: 校验数据
 * 输入参数: 
 *                           
 * 输出参数: 
 * 返回值      : 
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int verify_flash(char * device, unsigned int mtd_offset, char * file_buff, unsigned int file_size);


/******************************************************************************
 * 函数介绍: 
 * 输入参数: 
 *                           
 * 输出参数: 
 * 返回值      : 
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int erase_flash_all(char * mtd_device, unsigned int erase_addr, unsigned int erase_size);


/******************************************************************************
 * 函数介绍:  往flash  里面拷贝数据
 * 输入参数: 
 *                           
 * 输出参数: 
 * 返回值      : 
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int flash_copy(char * mtd_device, unsigned int mtd_offset, char * file_buff, unsigned int file_size);


/******************************************************************************
 * 函数介绍: 
 * 输入参数: 
 *                           
 * 输出参数: 
 * 返回值      : 
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int get_dev_info_from_mtd(char *mtd_device, unsigned int addr_offset, struct device_info_mtd *pdevice_info);


/******************************************************************************
 * 函数介绍: 
 * 输入参数: 
 *                           
 * 输出参数: 
 * 返回值      : 
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int set_dev_info_to_mtd(char *mtd_device, unsigned int addr_offset, struct device_info_mtd *pdevice_info);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FLASH_CTL_H_ */

