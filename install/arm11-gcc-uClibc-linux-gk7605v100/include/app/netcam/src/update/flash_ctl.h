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
#if 0
#define MTD_UBOOT           "/dev/mtd0"
#define MTD_UBOOT_ENV       "/dev/mtd1"
#define MTD_CONFIG			"/dev/mtd2"
#define MTD_KERNEL          "/dev/mtd3"
#define MTD_ROOTFS          "/dev/mtd4"
#define MTD_RESOURCE        "/dev/mtd5"
#define MTD_USRFS			"/dev/mtd6"
#define MTD_BACKUPS			"/dev/mtd7"
#define MTD_USER            "/dev/mtd8"
#define MTD_ALL             "/dev/mtd9"


#define MTD_UBOOT_NAME      "uboot"
#define MTD_UBOOT_ENV_NAME  "ubootenv"
#define MTD_CONFIG_NAME     "config"
#define MTD_KERNEL_NAME     "kernel"
#define MTD_ROOTFS_NAME     "rootfs"
#define MTD_RESOURCE_NAME   "resource"
#define MTD_USRFS_NAME      "usrfs"
#define MTD_BACKUPS_NAME    "backups"
#define MTD_USER_NAME       "user"
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
#endif

#define MAC_ADDR_STRSIZE    32
#define DEVICE_ID_STRSIZE   32
typedef enum{
    MTD_YUNNI_P2P = 1,
    MTD_ULUCU_P2P,
    MTD_DANA_P2P,
    MTD_GOKE_INFO,
    MTD_TUTK_P2P,
    MTD_MTWS_P2P,
    MTD_GB28181_INFO,
    MTD_TEST_RESULT,
    MTD_AF_OFFSET,
    MTD_MD5_INFO,
    MTD_KEY_INFO,
    MTD_KEY_SIGNED_INFO,
    MTD_MAX,
}RESERVE_DATA_TYPE;

//#define MTD_RESERV_NAME     "/dev/mtd6"
#define MTD_RESERV_NAME     MTD_PART_USER

#define YNNI_P2P_NAME 		"YNNI_P2P"
#define ULUCU_P2P_NAME 	    "ULUCU_P2P"
#define GOKE_INFO			"GOKE_INFO"
#define DANA_P2P_NAME		"DANA_P2P"
#define TUTK_P2P_NAME		"TUTK_P2P"
#define MTWS_P2P_NAME		"MTWS_P2P"
#define GB28181_NAME		"GB28181_INFO"
#define TEST_RESULT_NAME	"TEST_RESULT"
#define AF_OFFSET_NAME      "AF_OFFSET"
#define MD5_OFFSET_NAME     "MD5_INFO"
#define KEY_NAME            "KEY_INFO"
#define KEY_SIGNED_NAME     "KEY_SIGNED_INFO"

#define FCTL_DEBUG 0
#if FCTL_DEBUG
#define DEBUG_PRT(fmt, args...) \
do{ \
    printf("[FUN]%s [LINE]%d  "fmt, __FUNCTION__, __LINE__, ##args); \
}while(0)
#else
#define DEBUG_PRT(fmt, args...)
#endif

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
    char manufacturer_id[1024]; /*厂家id*/
	unsigned char 	devType;			// 设备类型: 0-卡片机, 1-摇头机, 2-鱼眼摄像机...
	unsigned char 	netFamily;			// 网络提供商: 1-迈特威视, 2-tutk...
	unsigned char 	serverID;			// 服务器ID: 保留,默认为0
	unsigned char 	language;			// 设备固件语言版本: 0为自动适应(即多语言),1为简体中文,2为繁体中文,3为英文
	unsigned int 	odmID;				// odm商ID: 0-东舜自己, 01-TCL, 02-康佳...
	unsigned int 	panoramaMode;		// 全景安装模式: 0-吸顶式,1-壁挂式	
    unsigned char   model[16];          //子型号。比如鱼眼可能有多种子型号的机型
    char		my_key[256];         // 加密使用
    char        wifi_ssid[32];		 // wifi SSID
	char 		wifi_psd[32];		// wifi 密码
	int 		wifi_enc_type;		// wifi 加密类型
    char reserve[1024-28-256-68];  /* 保留位           */

    //char reserve[1024-28];       /* 保留位           */
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
 * 函数介绍:保存参数到mtd分区中,注意: data最大为5KB数据
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :0:success -1:fail
 *                               v1.0.0    2016.01.19 heyongbing
 *****************************************************************************/
int save_info_to_mtd_reserve(RESERVE_DATA_TYPE type, void* data, int len);
/******************************************************************************
 * 函数介绍:从mtd分区中读出参数, 注意: data最大为5KB数据
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :0:success -1:fail
 *                               v1.0.0    2016.01.19 heyongbing
 *****************************************************************************/
int load_info_to_mtd_reserve(RESERVE_DATA_TYPE type, void* data, int len);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef _FLASH_CTL_H_ */

