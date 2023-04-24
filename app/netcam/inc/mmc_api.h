#ifndef __GRD_MMC_API_H__
#define __GRD_MMC_API_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <time.h>
#include <assert.h>
#include <sys/vfs.h>
#include <dirent.h>
#include <sys/stat.h>
#include <sys/mount.h>
#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/syscall.h>

#define GRD_SD_PATHNAME_HEADER      "/dev/mmcblk"
#define GRD_SD_PATHNAME             "/dev/mmcblk[0-9]"
#define GRD_SD_PARTITION_PATHNAME   "/dev/mmcblk[0-9]p[0-9]"
#define GRD_SD_MOUNT_POINT          "/mnt/sd_card"
#define GRD_SD_PATHNAME_0           "/dev/mmcblk0"
#define GRD_SD_PARTITION_PATHNAME_0 "/dev/mmcblk0p1"

#define utl_SetThreadName(name) \
{\
    prctl(PR_SET_NAME, (unsigned long)name, 0,0,0); \
    pid_t tid;\
    tid = syscall(SYS_gettid);\
    printf("set pthread name:%d, %s, %s pid:%d tid:%d\n", __LINE__, __func__, name, getpid(), tid);\
}

typedef enum {
        GRD_SD_DISKTYPE_MMC = 0,
        GRD_SD_DISKTYPE_SDA,
} GRD_SD_DISKTYPE;

typedef enum {
        GRD_SD_FSTYPE_FAT32 = 0,
        GRD_SD_FSTYPE_NTFS,
        GRD_SD_FSTYPE_REISERFS,
        GRD_SD_FSTYPE_EXTENDED,
        GRD_SD_FSTYPE_UNKNOWN,
} GRD_SD_FSTYPE;


/**********************************************************************
函数描述：判断需要修复的文件路径名是否符合自定义的格式
入口参数：char *file_path: 需要修复的文件路径名
返回值：  1: 符合
          0: 不符合
**********************************************************************/
extern int is_fix_format_path(char *file_path);

/**********************************************************************
函数描述：判断路径名是否符合自定义的格式
入口参数：char *str: 需要判断的路径名
返回值：  1: 符合
          0: 不符合
**********************************************************************/
extern int is_avi_dir_format(char *str);

/**********************************************************************
函数描述：判断文件名是否符合自定义的格式
入口参数：char *str: 需要判断的文件名
返回值：  1: 符合
          0: 不符合
**********************************************************************/
extern int is_avi_file_format(char *str);

/**********************************************************************
函数描述：判断文件路径名是否符合自定义的格式
入口参数：char *file_path: 需要判断的文件路径名
返回值：  1: 符合
          0: 不符合
**********************************************************************/
extern int is_avi_file_path_format(char *str);



/**********************************************************************
函数描述：mount sd卡
入口参数：无
返回值：  <0: 错误
          0 : 正常
**********************************************************************/
extern int grd_sd_mount(void);

/**********************************************************************
函数描述：reumount sd卡
入口参数：无
返回值：  <0: 错误
          0 : 正常
**********************************************************************/
extern int grd_sd_remount(void);

/**********************************************************************
函数描述：umount sd卡
入口参数：无
返回值：  <0: 错误
          0 : 正常
**********************************************************************/
extern int grd_sd_umount(void);

/**********************************************************************
函数描述：判断sd卡是否被mount上
入口参数：无
返回值：  1: 被mount上
          0: 没有被mount上
**********************************************************************/
extern int grd_sd_is_mount(void); // if sd card is mounted, return 1; else return 0


extern int grd_sd_check_device_node(void);


/**********************************************************************
函数描述：格式化 sd卡
入口参数：无
返回值：  <0: 错误
          0 : 正常
**********************************************************************/
extern int grd_sd_format(void);

/**********************************************************************
函数描述：得到sd卡的剩余容量
入口参数：无
返回值：  int: 剩余的大小，以MB位单位
**********************************************************************/
extern int grd_sd_get_free_size(void);

/**********************************************************************
函数描述：得到sd卡的剩余容量
入口参数：无
返回值：  int: 剩余的大小，以MB位单位
**********************************************************************/
extern int grd_sd_get_free_size_last(void);


/**********************************************************************
函数描述：得到sd卡的全部容量
入口参数：无
返回值：  int: 剩余的大小，以MB位单位
**********************************************************************/
extern int grd_sd_get_all_size(void);

/**********************************************************************
函数描述：得到sd卡的全部容量
入口参数：无
返回值：  int: 剩余的大小，以MB位单位
**********************************************************************/
extern int grd_sd_get_all_size_last(void);


/**********************************************************************
函数描述：检测sd卡的文件类型，格式化成FAT32格式
入口参数：无
返回值：  -1：失败
		  0：成功
**********************************************************************/
extern int grd_sd_init(void);


/**********************************************************************
函数描述：找到最老的avi文件夹中最老的avi文件，然后删除该文件。
          如果在最老的avi文件夹中没有找到avi文件，则删除该文件夹。
          如果找不到最老的avi文件夹，则返回0
入口参数：无
返回值：  1: 成功,删除了最老的文件或者最老的文件夹，正常返回
          0: 失败，找不到最老的文件夹，没有进行删除动作
**********************************************************************/
extern int grd_sd_del_oldest_avi_file(void);

/**********************************************************************
函数描述：判断sd卡设备是否存在
入口参数：无
返回值：-1：错误
		 0：不存在
		 1：存在
**********************************************************************/
extern int grd_sd_is_device_exist(void);

extern int grd_sd_get_format_process();


#ifdef __cplusplus
}
#endif

#endif /* __GRD_MMC_API_H__ */

