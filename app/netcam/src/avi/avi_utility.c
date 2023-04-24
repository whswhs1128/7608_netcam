/*!
*****************************************************************************
** FileName     : avi_utility.c
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

#include "avi_utility.h"

int write_char(FILE *file, char c)
{
    return fwrite(&c, sizeof(char), 1, file);
}
int write_int32(FILE *file, int i)
{
    return fwrite(&i, sizeof(int), 1, file);
}
int write_fourcc(FILE *file, int fourcc)
{
    return fwrite(&fourcc, sizeof(char)*4, 1, file);
}
int avi_write_data(FILE *file, char *data, int size)
{
    return fwrite(data, size, 1, file);
}

int read_char(FILE *file, char *c)
{
    return fread(c, sizeof(char), 1, file);
}
int read_int32(FILE *file, int *i)
{
    return fread(i, sizeof(int), 1, file);
}
int read_fourcc(FILE *file, int *fourcc)
{
    return fread(fourcc, sizeof(char)*4, 1, file);
}
int avi_utility_read_data(FILE *file, unsigned char *data, int size)
{
    return fread(data, size, 1, file);
}



void avi_set_32(void *pp, int w)
{
    char *p = pp;

    p[0] = ( w      )&0xff;
    p[1] = ( w >> 8 )&0xff;
    p[2] = ( w >> 16)&0xff;
    p[3] = ( w >> 24)&0xff;
}

/**********************************************************************
函数描述：从字符串中截取字符串
入口参数：char *dest: 目标字符串
          char *src: 源字符串
          int start: 截取的起始下标位置，0开始
          int end: 截取的结束下标位置，end - start就是截取的字符串长度
返回值：  无
**********************************************************************/
void avi_substring(char *dest, char *src, int start, int end)
{
    int i=start;
    if(start>strlen(src))
        return;
    if(end>strlen(src))
        end=strlen(src);
    while(i<end)
    {
        dest[i-start]=src[i];
        i++;
    }
    dest[i-start]='\0';
    return;
}

/**********************************************************************
函数描述：得到字符串中某字符的下标位置
入口参数：char *src: 源字符串
          char c: 查找的字符
返回值：  int: 找到则返回该字符的下标位置，如果没有该字符，则返回-1
**********************************************************************/
int last_index_at(char *str, char c)
{
    int n;
    n = strlen(str);

    while(n--)
    {
        if(c == str[n-1])
            return n-1 ;
    }
    return -1;
}


/**********************************************************************
函数描述：从avi文件名得到通道号
入口参数：char *file_name: avi文件名(不带路径名)
返回值：  unsigned long: 通道号
**********************************************************************/
unsigned long avi_get_ch_num(char *file_name)
{
    unsigned long tmp;
    tmp = (unsigned long)(file_name[2] - 48);
    return tmp;
}

/**********************************************************************
函数描述：从avi文件名得到起始时间
入口参数：char *file_name: avi文件名(不带路径名)
返回值：  unsigned long long: 起始时间
**********************************************************************/
unsigned long long avi_get_start_time(char *file_name)
{
    char tmp_str[20] = {0};
    unsigned long long tmp64;
    avi_substring(tmp_str, file_name, 4, 4+14);
    tmp64 = (unsigned long long)atoll(tmp_str);
    return tmp64;
}

/**********************************************************************
函数描述：从avi文件名得到结束时间
入口参数：char *file_name: avi文件名(不带路径名)
返回值：  unsigned long long: 结束时间
**********************************************************************/
unsigned long long avi_get_stop_time(char *file_name)
{
    char tmp_str[20] = {0};
    unsigned long long tmp64;
    avi_substring(tmp_str, file_name, 19, 19+14);
    tmp64 = (unsigned long long)atoll(tmp_str);
    return tmp64;
}

/**********************************************************************
函数描述：得到当前日期的字符串格式
入口参数：char *str: 当前日期的字符串格式
返回值：  无
**********************************************************************/
void get_date_str(char *str)
{
    struct tm *ptm;
    long ts;

    ts = time(NULL);
    struct tm tt = {0};
    ptm = localtime_r(&ts, &tt);

	sprintf(str, "%04d%02d%02d", ptm->tm_year+1900,
								 ptm->tm_mon+1,
								 ptm->tm_mday);

    return;
}

/**********************************************************************
函数描述：得到当前时间的字符串格式
入口参数：char *str: 当前时间的字符串格式
返回值：  无
**********************************************************************/
void get_nowtime_str(char *str)
{
    struct tm *ptm;
    long ts;

    ts = time(NULL);
    struct tm tt = {0};
    ptm = localtime_r(&ts, &tt);

	sprintf(str, "%04d%02d%02d%02d%02d%02d", ptm->tm_year+1900,
											 ptm->tm_mon+1,
											 ptm->tm_mday,
											 ptm->tm_hour,
											 ptm->tm_min,
											 ptm->tm_sec);

    return;
}

unsigned int get_nowtime_uint()
{
    struct tm *ptm;
    long ts;

    ts = time(NULL);
    struct tm tt = {0};
    ptm = localtime_r(&ts, &tt);

    unsigned int timep = (unsigned int)mktime(ptm);

    #if 0
    //timep += duration;
    timep += 0;
    memset(&tt, 0, sizeof(struct tm));
    ptm = localtime_r(&timep, &tt);
    char str[20] = {0};
	sprintf(str, "%04d%02d%02d%02d%02d%02d", ptm->tm_year+1900,
											 ptm->tm_mon+1,
											 ptm->tm_mday,
											 ptm->tm_hour,
											 ptm->tm_min,
											 ptm->tm_sec);

    

    ptm = gmtime_r(&timep, &tt);
    char str2[20] = {0};
	sprintf(str2, "%04d%02d%02d%02d%02d%02d", ptm->tm_year+1900,
											 ptm->tm_mon+1,
											 ptm->tm_mday,
											 ptm->tm_hour,
											 ptm->tm_min,
											 ptm->tm_sec);
    printf("local time str:%s, gm time str:%s \n", str, str2);
    #endif

    return timep;
}


/**********************************************************************
函数描述：得到当前时间的字符串格式
入口参数：char *str: 当前时间的字符串格式
返回值：  无
**********************************************************************/
void get_nowhm_str(char *str)
{
    struct tm *ptm;
    long ts;

    ts = time(NULL);
    struct tm tt = {0};
    ptm = localtime_r(&ts, &tt);

	sprintf(str, "%02d%02d%02d", ptm->tm_hour,
								 ptm->tm_min,
								 ptm->tm_sec);

    return;
}


#if 0
/**********************************************************************
函数描述：根据参数打印出错误提示.
入口参数：int ret: avi_record_open,avi_record_write,avi_record_close
                   的返回值作为参数
返回值：空
**********************************************************************/
void avi_print_err(int ret)
{
    switch(ret)
    {
        case STS_OPEN_FAILED:       PRINT_INFO("open the file failed. \n"); break;
        case STS_INVALID_INPUT:     PRINT_INFO("invalid input. \n"); break;
        case STS_MKDIR_ERROR:       PRINT_INFO("mkdir error. \n"); break;
        case STS_INVALID_FORAMT:    PRINT_INFO("invalid format. \n"); break;
        case STS_MALLOC_FAILED:     PRINT_INFO("malloc error. \n"); break;
        case STS_FTRUNCATE_FAILED:  PRINT_INFO("ftruncate file failed. \n"); break;
        case STS_RENAME_FAILED:     PRINT_INFO("rename error. \n"); break;
        case STS_POLL_FAILED:       PRINT_INFO("poll error. \n"); break;

        case STS_RECORD_MODE_ERR:   PRINT_INFO("record mode error. \n"); break;
        case STS_WRITE_FAILED:      PRINT_INFO("write failed. \n"); break;
        case STS_READ_FAILED:       PRINT_INFO("read failed. \n"); break;
        case STS_SDCARD_NOT_MOUNT:  PRINT_INFO("sd card is not mounted. \n"); break;
        case STS_SDCARD_NO_SPACE:   PRINT_INFO("sd card don't have enough space. \n"); break;
        case STS_FISTFRAME_NOT_KEY: PRINT_INFO("the fist frame is not key frame. \n"); break;

        case STS_INDEX_COUNT_ERR:   PRINT_INFO("the avi index count malloc error. \n"); break;
        case STS_DOUBLE_CLOSE:      PRINT_INFO("double call close function. \n"); break;
        default: PRINT_INFO(" error. \n"); break;
    }
}
#endif

/**********************************************************************
函数描述：时间的tm格式转换成时间的AVI_DMS_TIME格式
入口参数：struct tm *src: 源操作数
          AVI_DMS_TIME *dst: 目标操作数
返回值：  无
**********************************************************************/
void tm_to_dmstime(AVI_DMS_TIME *dst, struct tm *src)
{
	dst->dwYear = src->tm_year;
	dst->dwMonth = src->tm_mon;
	dst->dwDay =  src->tm_mday;
	dst->dwHour = src->tm_hour;
	dst->dwMinute =src->tm_min;
	dst->dwSecond = src->tm_sec;
}

/**********************************************************************
函数描述：时间的tm格式加上秒数，得到时间的tm格式
入口参数：struct tm *dst: 目标操作数
          struct tm *src: 第一个操作数
          int seconds: 第二个操作数
返回值：  无
**********************************************************************/
void tm_add_seconds(struct tm *dst, struct tm *src, int time_seconds)
{
	time_t timep;
    struct tm t1 = {0};
	struct tm *p = &t1;

	p->tm_year = src->tm_year - 1900;
	p->tm_mon = src->tm_mon - 1;
	p->tm_mday = src->tm_mday;
	p->tm_hour = src->tm_hour;
	p->tm_min = src->tm_min;
	p->tm_sec = src->tm_sec;


	timep = mktime(p);

	timep += time_seconds;
    memset(&t1, 0, sizeof(struct tm));
	p = localtime_r(&timep, &t1);

	dst->tm_year = p->tm_year + 1900;
	dst->tm_mon = p->tm_mon + 1;
	dst->tm_mday =  p->tm_mday;
	dst->tm_hour = p->tm_hour;
	dst->tm_min =p->tm_min;
	dst->tm_sec = p->tm_sec;

}

/**********************************************************************
函数描述：时间的u64t格式加上秒数，返回相加后的值
入口参数：u64t opr1: 第一个操作数
          int seconds: 第二个操作数
返回值：  u64t: 相加后的值
**********************************************************************/
u64t u64t_add_seconds(u64t opr1, int seconds)
{
    AVI_DMS_TIME src;
    u64t_to_time(&src, opr1);

    time_t timep;
	struct tm *p;

	time(&timep);

    struct tm t1 = {0};
    p = localtime_r(&timep, &t1);

	p->tm_year = src.dwYear - 1900;
	p->tm_mon = src.dwMonth - 1;
	p->tm_mday = src.dwDay;
	p->tm_hour = src.dwHour;
	p->tm_min = src.dwMinute;
	p->tm_sec = src.dwSecond;

	timep = mktime(p);
	timep += seconds;
    memset(&t1, 0, sizeof(struct tm));
	p = localtime_r(&timep, &t1);

    AVI_DMS_TIME dst;
	dst.dwYear = p->tm_year + 1900;
	dst.dwMonth = p->tm_mon + 1;
	dst.dwDay =  p->tm_mday;
	dst.dwHour = p->tm_hour;
	dst.dwMinute = p->tm_min;
	dst.dwSecond = p->tm_sec;

    u64t opr2 = time_to_u64t(&dst);

    return opr2;
}

/**********************************************************************
函数描述：计算2个时间的差值，
入口参数：u64t opr1: 第一个操作数
          u64t opr2: 第二个操作数
返回值：  u64t: opr1 减去 opr2
**********************************************************************/
int cal_time_sub(u64t opr1, u64t opr2)
{
    AVI_DMS_TIME src;
    u64t_to_time(&src, opr1);

    time_t timep1;
    time_t timep2;
	struct tm *p;

	time(&timep1);
    struct tm t1 = {0};
    p = localtime_r(&timep1, &t1);

	p->tm_year = src.dwYear - 1900;
	p->tm_mon = src.dwMonth - 1;
	p->tm_mday = src.dwDay;
	p->tm_hour = src.dwHour;
	p->tm_min = src.dwMinute;
	p->tm_sec = src.dwSecond;

	timep1 = mktime(p);

    memset(&src, 0, sizeof(AVI_DMS_TIME));
    u64t_to_time(&src, opr2);
	p->tm_year = src.dwYear - 1900;
	p->tm_mon = src.dwMonth - 1;
	p->tm_mday = src.dwDay;
	p->tm_hour = src.dwHour;
	p->tm_min = src.dwMinute;
	p->tm_sec = src.dwSecond;
    timep2 = mktime(p);

    return (timep1 - timep2);
}


/**********************************************************************
函数描述：AVI_DMS_TIME 类型时间转换成 unsigned long long 类型时间
入口参数：AVI_DMS_TIME *p: AVI_DMS_TIME 类型时间
返回值：  u64t: unsigned long long 类型时间
**********************************************************************/
u64t time_to_u64t(AVI_DMS_TIME *p)
{
    char tmp[20];
    u64t tmp64 = 0;
    memset(tmp, 0, sizeof(tmp));
    sprintf(tmp, "%04lu%02lu%02lu%02lu%02lu%02lu", p->dwYear, p->dwMonth, p->dwDay,
                                            p->dwHour, p->dwMinute, p->dwSecond);
    tmp64 = atoll(tmp);
    return tmp64;
}

/**********************************************************************
函数描述：unsigned long long 类型时间转换成 AVI_DMS_TIME 类型时间
入口参数：AVI_DMS_TIME *ptime: 得到 AVI_DMS_TIME 类型时间
          u64t tmp64: unsigned long long 类型时间
返回值：  无
**********************************************************************/
void u64t_to_time(AVI_DMS_TIME *ptime, u64t tmp64)
{
    char tmp_str[20];
    char tmp[20];

    memset(tmp_str, 0, sizeof(tmp_str));
    sprintf(tmp_str, "%lld", tmp64);

    memset(tmp, 0, sizeof(tmp));
    avi_substring(tmp, tmp_str, 0, 0+4);
    ptime->dwYear = atoi(tmp);

    memset(tmp, 0, sizeof(tmp));
    avi_substring(tmp, tmp_str, 4, 4+2);
    ptime->dwMonth = atoi(tmp);

    memset(tmp, 0, sizeof(tmp));
    avi_substring(tmp, tmp_str, 6, 6+2);
    ptime->dwDay = atoi(tmp);

    memset(tmp, 0, sizeof(tmp));
    avi_substring(tmp, tmp_str, 8, 8+2);
    ptime->dwHour = atoi(tmp);

    memset(tmp, 0, sizeof(tmp));
    avi_substring(tmp, tmp_str, 10, 10+2);
    ptime->dwMinute = atoi(tmp);

    memset(tmp, 0, sizeof(tmp));
    avi_substring(tmp, tmp_str, 12, 12+2);
    ptime->dwSecond = atoi(tmp);

}

u64t get_u64t_now_time(void)
{
    char time_now[30];
    memset(time_now, 0, sizeof(time_now));
    get_nowtime_str(time_now);
    u64t time_64 = atoll(time_now);
    return time_64;
}

void time_to_str(long ts, char *str)
{
	struct tm *ptm = gmtime(&ts);
    if(ptm && str)
        sprintf(str, "%04d%02d%02d%02d%02d%02d", ptm->tm_year+1900,
    											 ptm->tm_mon+1,
    											 ptm->tm_mday,
    											 ptm->tm_hour,
    											 ptm->tm_min,
    											 ptm->tm_sec);
	return;
}

u64t get_time_to_u64t(long ts)
{
    char time_now[30];
    memset(time_now, 0, sizeof(time_now));
    time_to_str(ts, time_now);
    u64t time_64 = atoll(time_now);
    return time_64;
}

/* 秒级定时器 */
void gk_cms_ssleep(unsigned seconds)
{
    struct timeval tv;
    tv.tv_sec = seconds;
    tv.tv_usec = 0;
    int err;
    do {
       err = select(0, NULL, NULL, NULL, &tv);
    } while (err < 0 && errno == EINTR);
}

/* 毫秒级别定时器 */
void gk_cms_msleep(unsigned long mSec)
{
    struct timeval tv;
    tv.tv_sec = mSec/1000;
    tv.tv_usec = (mSec % 1000) * 1000;
    int err;
    do {
       err = select(0, NULL, NULL, NULL, &tv);
    } while (err < 0 && errno == EINTR);
}
// type =1 返回系统运行秒，type=2 返回系统运行ms

unsigned long avi_get_sys_runtime(int type)
{
    struct timespec times = {0, 0};
    unsigned long time;

    clock_gettime(CLOCK_MONOTONIC, &times);
    //LOG_INFO("CLOCK_MONOTONIC: %lu, %lu\n", times.tv_sec, times.tv_nsec);

    if (1 == type)
    {
        time = times.tv_sec;
    }
    else
    {
        time = times.tv_sec * 1000 + times.tv_nsec / 1000000;
    }

    //LOG_INFO("time = %ld\n", time);
    return time;
}


