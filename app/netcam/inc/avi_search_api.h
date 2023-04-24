/*!
*****************************************************************************
** FileName     : avi_search_api.h
**
** Description  : head file for search file.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create       : 2015-9-17, create
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _AVI_SEARCH_API_H__
#define _AVI_SEARCH_API_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "avi_common.h"

/************************************************************************/
/* 获取查找到的返回结果*/
/************************************************************************/
#define GK_NET_FILE_SUCCESS 1000// 获取文件信息成功 
#define GK_NET_FILE_NOFIND 1001// 未查找到文件 
#define GK_NET_ISFINDING 1002// 正在查找请等待 
#define GK_NET_NOMOREFILE  1003// 没有更多的文件，查找结束 
#define GK_NET_FILE_EXCEPTION  1004// 查找文件时异常 

/************************************************************************/
/* 获取查找到的返回结果*/
/************************************************************************/
#define DMS_NET_FILE_SUCCESS 1000// 获取文件信息成功 
#define DMS_NET_FILE_NOFIND 1001// 未查找到文件 
#define DMS_NET_ISFINDING 1002// 正在查找请等待 
#define DMS_NET_NOMOREFILE  1003// 没有更多的文件，查找结束 
#define DMS_NET_FILE_EXCEPTION  1004// 查找文件时异常 

typedef  struct
{
    unsigned long    dwSize;
    char             csFileName[100];
    unsigned long    dwFileType; // 取值范围 GK_NET_RECORD_TYPE_
    AVI_DMS_TIME     stStartTime;
    AVI_DMS_TIME     stStopTime;
    unsigned long    dwFileSize;
    unsigned char    byLocked;// 是否锁定：0-未锁定文件，1-锁定文件，0xff表示所有文件（包括锁定和未锁定） 
    unsigned char    byRes[3];
}GK_NET_FINDDATA,*LPGK_NET_FINDDATA;

typedef enum emRECORD_TYPE_E
{
    RECORD_TYPE_SCHED   = 0x000001,   //定时录像
    RECORD_TYPE_MOTION  = 0x000002,  //移到侦测录像
    RECORD_TYPE_ALARM   = 0x000004,  //报警录像
    RECORD_TYPE_CMD     = 0x000008, //命令录像
    RECORD_TYPE_MANU    = 0x000010,  //手工录像
    RECORD_TYPE_SCHED_2 = 0x000011 //手工录像
} RECORD_TYPE_E;



/**********************************************************************
查找SD卡中AVI类型文件函数步骤描述：
1，调用 search_file_by_time 得到查找文件控制句柄 FILE_LIST *
2，调用 get_file_node，得到FILE_LIST中的每一个FILE_NODE，即查找出来的
   单个文件
3，调用 search_close 结束
**********************************************************************/
/**********************************************************************
函数描述：根据时间段查找文件
入口参数：u32t type: 安普定义如下
          GK_NET_RECORD_TYPE_SCHED   定时录像
          GK_NET_RECORD_TYPE_MOTION  移到侦测录像
          GK_NET_RECORD_TYPE_ALARM   手动报警录像
          GK_NET_RECORD_TYPE_MANU    手工录像
          u32t ch_num: 通道号
          u64t start: 起始时间
          u64t stop:  结束时间
返回值：  FILE_LIST *: 返回的查找文件控制句柄       
**********************************************************************/
extern FILE_LIST *search_file_by_time(u32t type, u32t ch_num, u64t start, u64t stop);

/**********************************************************************
函数描述：查找当月录像天数
入口参数：u32t type: 安普定义如下
          RECORD_TYPE_SCHED   定时录像
          RECORD_TYPE_MOTION  移到侦测录像
          RECORD_TYPE_ALARM   手动报警录像
          RECORD_TYPE_MANU    手工录像
          u32t ch_num: 通道号
          u64t start: 起始时间
          u64t stop:  结束时间
返回值：  FILE_LIST *: 返回的查找文件控制句柄       
**********************************************************************/
extern unsigned int search_day_by_month(u32t type, u32t ch_num, int year, int month);

/**********************************************************************
函数描述：根据时间段查找文件
入口参数：FILE_LIST *: 查找文件的控制句柄
          FILE_NODE *node: 得到文件信息指针
返回值：  安普定义如下:
          GK_NET_FILE_SUCCESS: 获取文件信息成功
          GK_NET_NOMOREFILE: 没有更多的文件，查找结束 
          GK_NET_FILE_NOFIND: 未查找到文件
**********************************************************************/
extern int get_file_node(FILE_LIST *list, FILE_NODE *node);

/**********************************************************************
函数描述：查找文件结束
入口参数：FILE_LIST *: 查找文件的控制句柄
返回值：  无
**********************************************************************/
extern void search_close(FILE_LIST *list);

/**********************************************************************
函数描述：得到文件的大小
入口参数：FILE_NODE *node: 单个文件的信息节点
返回值：  u32t: 文件的大小,以字节为单位
**********************************************************************/
extern u32t get_file_size(char *path);


/**********************************************************************
函数描述：打印管理文件中的一次录像动作的信息 
入口参数：INDEX_NODE *n: 录像动作的信息指针
返回值：  无
**********************************************************************/
extern void print_index_node(INDEX_NODE *n);

/**********************************************************************
函数描述：打印查找文件句柄中的单个文件信息 
入口参数：FILE_NODE *n: 查找后得到的单个文件信息 
返回值：  无
**********************************************************************/
extern void print_node(FILE_NODE *n);

/**********************************************************************
函数描述：打印查找文件控制句柄中的所有文件信息 
入口参数：FILE_LIST *list: 查找文件控制句柄
返回值：  无
**********************************************************************/
extern void print_list(FILE_LIST *list);

/**********************************************************************
函数描述：打印安普接口的 GK_NET_FINDDATA 信息 
入口参数：GK_NET_FINDDATA *p: 查找后得到的单个文件信息 
返回值：  无
**********************************************************************/
extern void print_finddata(GK_NET_FINDDATA *p);

/**********************************************************************
函数描述：判断是否查找到文件
入口参数：FILE_LIST *list: 查找文件的控制句柄
返回值：  1: 空，没有找到文件
          0: 非空，查找到了文件
**********************************************************************/
extern int is_empty_list(FILE_LIST *list);

/**********************************************************************
函数描述：得到查找到的文件数目
入口参数：FILE_LIST *list: 查找文件的控制句柄
返回值：  int: 查找到的文件数目
**********************************************************************/
extern int get_len_list(FILE_LIST *list);



#ifdef __cplusplus
}
#endif
#endif



