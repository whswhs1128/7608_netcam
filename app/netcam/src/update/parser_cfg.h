/*******************************************************************************
 *
 * Copyright (c)
 *
 *  \file parser_cfg.h
 *
 *  \brief parse config file.
 *
 *   create version  v 1.0.0  wangguixing
 *
 ******************************************************************************/

#ifndef _PARSER_CFG_H_
#define _PARSER_CFG_H_

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
#define MAGIC_NUM           "GOKE_IPCAM_FIRMWARE"    /* 幻数 GKIC */

#define MAX_MTD_NUM         16              /*  最多支持16 个MTD分区 */
#define MAX_MTD_SIZE        0x2000000       /*  32MB . MTD 最大支持的容量 */
#define MIMI_MTD_SIZE       0x1000          /*  4KB.  MTD 最小支持的容量   */

#define MAX_NORFLASH_SIZE   0x2000000       /*  32MB . nor_flash最大支持的容量 */
#define MIMI_NORFLASH_SIZE  0x200000        /*  2MB.    nor_flash最小支持的容量  */

#define MTD_PATH            "/proc/mtd"
#define FIRMWARE_START      "[FIRMWARE]"
#define FIRMWARE_END        "[FIRMWARE_END]"

#define UPGRADE_NORMAL_MTD  1
#define UPGRADE_ALL_MTD     2

#define PCFG_DEBUG 0
#if PCFG_DEBUG
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

/*
  * 该结构体要求4字节对其, 用于写入升级文件的头部
  * 注意该结构体是写入在升级文件中的，为了避免产生
  * 版本前后兼容问题,  所以该结构体一旦确定尽量不要修改
  * 为此, 已经预留出了64 个字节的保留位, 方便以后扩展. 4字节对其
*/
struct gk_upgrade_header
{
    char  magic_num[32];        /* 幻数                         */
    char  image_ver[32];        /* 镜像版本号       */
    char  Platform[32];         /* 平台类型             */
    char  machine_type[32];     /* 产品类型             */

    unsigned int  upgrade_flag; /* 升级标志, 0:表示不升级; 1:表示升级  */
    unsigned int  flash_size;   /* 整个flash 容量大小 */
    unsigned int  file_type;    /* 升级文件类型 */
    unsigned int  file_size;    /* 升级文件大小不包括头(gk_file_header) */
    unsigned int  file_crc32;   /* 升级文件crc32校验和      */

    unsigned int  mtd_addr;     /* 擦写偏移位置 (相对整个flash容量如: 8MB or 16MB)    */
    unsigned int  mtd_offset;   /* 要升级的所在分区内的便宜地址, 而非整体flash的偏移地址 */
    unsigned int  mtd_size;     /* 升级文件指定flash 分区的大小 */
    unsigned int  mtd_num;      /* 升级文件指定flash 分区号 */
    char          mtd_part[32]; /* 指定要升级的MTD 分区设备如: /dev/mtd3  */
    char          mtd_name[32]; /* 指定要升级的MTD 分区名子如: rootfs , uboot */
    char          pack_md5[16];   /* 升级文件md5*/

    char          reserve[48];
};
typedef struct gk_upgrade_header GK_UPGRADE_HEADER_S;


/* 结构体用于解析linux 下/proc/mtd 的mtd 信息 */
struct proc_mtd_info
{
    unsigned int mtd_size;      /* 该 MTD 分区的大小 */
    unsigned int mtd_addr;      /* 该 MTD 分区相对于整个FLASH 空间的地址 */
    char mtd_part[32];          /* 该 MTD 的分区号 如: /dev/mtd0,  /dev/mtd1  ...  */
    char mtd_name[32];          /* 该 MTD 的分区名 如: uboot, kernel, rootfs ... */
};


/*
  * 结构体用于描述upgrade.bin 内一个升级模块的信息,
  * 该信息使用在板端的升级。
  */
struct upgrade_file_info
{
    struct gk_upgrade_header upgrade_header; /* 升级原文件的头信息  */
    unsigned int file_addr;     /* 升级原文件在打包文件的位置 (不包含头)  */
    unsigned int file_size;     /* 升级原文件的大小 (不包含头)  */
};


/*
  * 结构体用于描述IPC模块信息,
  * 该信息使用在板端的升级，用于判断升级文件和IPC是否匹配。
  */
struct gk_platform_info
{
    char  Platform[32];         /* 平台类型             */
    char  machine_type[32];     /* 产品类型             */
    char  image_ver[32];        /* 镜像版本号       */
};

/* ========================================================================== */
/*                          Function Declarations                             */
/* ========================================================================== */

/******************************************************************************
 * 函数介绍:  获取当前进度百分比, 该函数需要在调用
 *                            upgrade_flash_by_buf 函数后, 才能开始使用获取正确的值.
 * 输入参数:  无
 *                           
 * 输出参数:  0--100的值. 即表示 0%--100%
 * 返回值      : 
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
unsigned int get_upgrade_rate(void);


/******************************************************************************
 * 函数介绍: 通过linux系统的/proc/mtd 信息获取当前的MTD 分区信息
 * 输入参数: 无
 *
 * 输出参数: pmtd_info: 填充好的MTD信息结构体数组,
 *                                             注意该数组长度为MAX_MTD_NUM(16)
 *                            pmtd_num: 返回有效的MTD信息结构体个数
 *
 * 返回值      : =0: 成功
 *                            <0: 失败
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int get_mtdinfo_by_proc_mtd(struct proc_mtd_info *pmtd_info, int *pmtd_num);


/******************************************************************************
 * 函数介绍: 通过linux /proc/mtd 获取分区名为ALL 的MTD 分区信息
 * 输入参数: 无
 *
 * 输出参数: pmtd_info: 填充好的MTD信息结构体,该结构体是1个变量指针
 *                                              而非数组.
 *
 * 返回值      : =0: 成功
 *                            <0: 失败
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int get_all_mtd_info_by_proc(struct proc_mtd_info *pall_mtd);


/******************************************************************************
 * 函数介绍:
 * 输入参数:
 *
 * 输出参数:
 * 返回值      :
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int pack_product_file(char *config_file, char *upgrade_bin);


/******************************************************************************
 * 函数介绍: 通过打包配置文件的内容, 生成升级镜像文件
 * 输入参数: config_file: 打包配置文件的文件名
 *                           upgrade_bin: 要生成升级镜像文件的文件名
 *
 * 输出参数: 无
 *
 * 返回值      : =0: 成功
 *                            <0: 失败
 *                               v1.0.0    2015.08.21  wangguixing
 *****************************************************************************/
int pack_upgrade_file(char *config_file, char *upgrade_bin);

int unpack_upgrade_file(char *upgrade_bin);

int verify_upgrade_file(char *upgrade_bin);

int verify_upgrade_buff(char *upgrade_buf, unsigned int buf_size);

int get_upgradeinfo_by_file(char *upgrade_bin, struct upgrade_file_info *pinfo, int * mod_num);

int get_upgradeinfo_by_buff(char *upgrade_buf,
                                    unsigned int buf_size,
                                    struct upgrade_file_info *pinfo,
                                    int * mod_num);

int judge_upgrade_by_info(struct proc_mtd_info *pmtd_info,
                                  int mtd_num,
                                  struct upgrade_file_info *pupgrade_info);

int upgrade_flash_by_buf(char *upgrade_buf, unsigned int buf_size);

int upgrade_flash_by_file(char *upgrade_file);

/*检测升级头信息，升级包中的数据
magic_num必须为"GK7101_IPCAM_FIRMWARE"
若image_ver为"FFFFFFFF"，则强制升级该软件
若image_ver不同，且machine_type与本机相同，则升级
检验通过返回0，失败返回-1
*/
int check_upgrade_header(struct gk_upgrade_header *pheader);


#ifdef __cplusplus
}
#endif

#endif /* #ifndef _PARSER_CFG_H_ */

