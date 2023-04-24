/*!
*****************************************************************************
** FileName     : avi_utility.h
**
** Description  : avi utility api.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create       : 2015-8-11, create
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef __AVI_UTILITY_H__
#define __AVI_UTILITY_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "avi_common.h"



/**********************************************************************
函数描述：向AVI文件中写入一个字符
入口参数：FILE *file: 文件指针
          char c: 写入的字符
返回值：  1: 写入正确
          非1: 失败
**********************************************************************/
extern int write_char(FILE *file, char c);

/**********************************************************************
函数描述：向AVI文件中写入一个整型
入口参数：FILE *file: 文件指针
          int i: 写入的整型
返回值：  1: 写入正确
          非1: 失败
**********************************************************************/
extern int write_int32(FILE *file, int i);

/**********************************************************************
函数描述：向AVI文件中写入一个整型
入口参数：FILE *file: 文件指针
          int fourcc: 写入的整型
返回值：  1: 写入正确
          非1: 失败
**********************************************************************/
extern int write_fourcc(FILE *file, int fourcc);

/**********************************************************************
函数描述：向AVI文件中写入一串数据
入口参数：FILE *file: 文件指针
          char *data: 写入的数据地址
          int size: 写入的数据大小
返回值：  1: 写入正确
          非1: 失败
**********************************************************************/
extern int avi_write_data(FILE *file, char *data, int size);

/**********************************************************************
函数描述：从AVI文件中读取一个字符
入口参数：FILE *file: 文件指针
          char c: 读取的字符
返回值：  1: 读取正确
          非1: 失败
**********************************************************************/
extern int read_char(FILE *file, char *c);

/**********************************************************************
函数描述：从AVI文件中读取一个整型
入口参数：FILE *file: 文件指针
          int i: 读取的整型
返回值：  1: 读取正确
          非1: 失败
**********************************************************************/
extern int read_int32(FILE *file, int *i);

/**********************************************************************
函数描述：从AVI文件中读取一个整型
入口参数：FILE *file: 文件指针
          int fourcc: 读取的整型
返回值：  1: 读取正确
          非1: 失败
**********************************************************************/
extern int read_fourcc(FILE *file, int *fourcc);

/**********************************************************************
函数描述：从AVI文件中读取一串数据
入口参数：FILE *file: 文件指针
          char *data: 读取的数据地址
          int size: 读取的数据大小
返回值：  1: 读取正确
          非1: 失败
**********************************************************************/
extern int avi_utility_read_data(FILE *file, unsigned char *data, int size);

/**********************************************************************
函数描述：向内存地址中写入一个整型
入口参数：void *pp: 内存地址
          int w: 写入的整型
返回值：  1: 写入正确
          非1: 失败
**********************************************************************/
extern void avi_set_32(void *pp, int w);

/**********************************************************************
函数描述：从字符串中截取字符串
入口参数：char *dest: 目标字符串
          char *src: 源字符串
          int start: 截取的起始下标位置，0开始
          int end: 截取的结束下标位置，end - start就是截取的字符串长度
返回值：  无
**********************************************************************/
extern void avi_substring(char *dest, char *src, int start, int end);

/**********************************************************************
函数描述：得到字符串中某字符的下标位置
入口参数：char *src: 源字符串
          char c: 查找的字符
返回值：  int: 找到则返回该字符的下标位置，如果没有该字符，则返回-1
**********************************************************************/
extern int last_index_at(char *str, char c);

/**********************************************************************
函数描述：从avi文件名得到通道号
入口参数：char *file_name: avi文件名(不带路径名)
返回值：  unsigned long: 通道号
**********************************************************************/
extern unsigned long avi_get_ch_num(char *file_name);

/**********************************************************************
函数描述：从avi文件名得到起始时间
入口参数：char *file_name: avi文件名(不带路径名)
返回值：  unsigned long long: 起始时间
**********************************************************************/
extern unsigned long long avi_get_start_time(char *file_name);

/**********************************************************************
函数描述：从avi文件名得到结束时间
入口参数：char *file_name: avi文件名(不带路径名)
返回值：  unsigned long long: 结束时间
**********************************************************************/
extern unsigned long long avi_get_stop_time(char *file_name);

/**********************************************************************
函数描述：得到当前日期的字符串格式
入口参数：char *str: 当前日期的字符串格式
返回值：  无
**********************************************************************/
extern void get_date_str(char *str);

/**********************************************************************
函数描述：得到当前时间的字符串格式
入口参数：char *str: 当前时间的字符串格式
返回值：  无
**********************************************************************/
extern void get_nowtime_str(char *str);
extern void get_nowhm_str(char *str);
extern unsigned int get_nowtime_uint();


/**********************************************************************
函数描述：将时间戳 timetick 转换成 AVI_DMS_TIME 类型的数据结构
入口参数：AVI_DMS_TIME *ptime: AVI_DMS_TIME 类型的指针，记录时间
          int timetick: 时间戳，毫秒为单位
返回值：  无
**********************************************************************/
extern void timetick_to_time(AVI_DMS_TIME *ptime, unsigned int timetick);


/**********************************************************************
函数描述：AVI_DMS_TIME 类型时间转换成 unsigned long long 类型时间
入口参数：AVI_DMS_TIME *p: AVI_DMS_TIME 类型时间
返回值：  u64t: unsigned long long 类型时间
**********************************************************************/
extern u64t time_to_u64t(AVI_DMS_TIME *p);

/**********************************************************************
函数描述：unsigned long long 类型时间转换成 AVI_DMS_TIME 类型时间
入口参数：AVI_DMS_TIME *ptime: 得到 AVI_DMS_TIME 类型时间
          u64t tmp64: unsigned long long 类型时间
返回值：  无
**********************************************************************/
extern void u64t_to_time(AVI_DMS_TIME *ptime, u64t tmp64);

/**********************************************************************
函数描述：时间的tm格式转换成时间的AVI_DMS_TIME格式
入口参数：struct tm *src: 源操作数
          AVI_DMS_TIME *dst: 目标操作数
返回值：  无
**********************************************************************/
void tm_to_dmstime(AVI_DMS_TIME *dst, struct tm *src);

/**********************************************************************
函数描述：时间的tm格式加上秒数，得到时间的tm格式
入口参数：struct tm *dst: 目标操作数
          struct tm *src: 第一个操作数
          int seconds: 第二个操作数
返回值：  无
**********************************************************************/
void tm_add_seconds(struct tm *dst, struct tm *src, int time_seconds);

/**********************************************************************
函数描述：时间的u64t格式加上秒数，返回相加后的值
入口参数：u64t opr1: 第一个操作数
          int seconds: 第二个操作数
返回值：  u64t: 相加后的值
**********************************************************************/
u64t u64t_add_seconds(u64t opr1, int seconds);

/**********************************************************************
函数描述：计算2个时间的差值，
入口参数：u64t opr1: 第一个操作数
          u64t opr2: 第二个操作数
返回值：  u64t: opr1 减去 opr2
**********************************************************************/
int cal_time_sub(u64t opr1, u64t opr2);

u64t get_u64t_now_time(void);

void time_to_str(long ts, char *str);
u64t get_time_to_u64t(long ts);

//录像计划的时间数据结构
typedef struct tagGK_SCHEDTIME_POINT
{
    unsigned short  wStartYear;
    unsigned char   byStartMonth;
    unsigned char   byStartDate;
    unsigned char   byStartHour;
    unsigned char   byStartMin;
    unsigned char   byStartSecond;
    unsigned short  wStopYear;
    unsigned char   byStopMonth;
    unsigned char   byStopDate;
    unsigned char   byStopHour;
    unsigned char   byStopMin;
    unsigned char   byStopSecond;
} GK_SCHEDTIME_POINT;


/* 秒级定时器 */
void gk_cms_ssleep(unsigned seconds);

/* 毫秒级别定时器 */
void gk_cms_msleep(unsigned long mSec);


// type =1 返回系统运行秒，type=2 返回系统运行ms

unsigned long avi_get_sys_runtime(int type);

#ifdef __cplusplus
}
#endif
#endif

