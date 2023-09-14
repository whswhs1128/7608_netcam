/*!
*****************************************************************************
** FileName     : eventalarm.h
**
** Description  : config for pt callback.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-8-27
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_PT_API_H__
#define _GK_PT_API_H__

#ifdef __cplusplus
extern "C"
{
#endif


/************************************************************************/
/*       设备报警事件类型                                               */
/************************************************************************/
typedef enum __GK_ALARM_TYPE_E
{
    GK_ALARM_TYPE_ALARMIN = 0,     //0:信号量报警开始
    GK_ALARM_TYPE_DISK_FULL,       //1:硬盘满
    GK_ALARM_TYPE_VLOST,           //2:信号丢失
    GK_ALARM_TYPE_VMOTION,         //3:移动侦测
    GK_ALARM_TYPE_DISK_UNFORMAT,   //4:硬盘未格式化
    GK_ALARM_TYPE_DISK_RWERR,      //5:读写硬盘出错,
    GK_ALARM_TYPE_VSHELTER,        //6:遮挡报警
    GK_ALARM_TYPE_VFORMAT_UNMATCH, //7:制式不匹配
    GK_ALARM_TYPE_ILLEGAL_ACCESS,  //8:非法访问
    GK_ALARM_TYPE_ALARMIN_RESUME,  //9:信号量报警恢复
    GK_ALARM_TYPE_VLOST_RESUME,    //10:视频丢失报警恢复
    GK_ALARM_TYPE_VMOTION_RESUME,  //11:视频移动侦测报警恢复
    GK_ALARM_TYPE_NET_BROKEN,      //12:网络断开
    GK_ALARM_TYPE_IP_CONFLICT,     //13:IP冲突
    GK_ALARM_TYPE_ITEV,			//14:智能分析
    GK_ALARM_TYPE_ITEV_RESUME,		//15:智能分析报警恢复
	GK_ALARM_TYPE_TEMPERATURE,		//16:温度
	GK_ALARM_TYPE_HUMIDITY,		//17:湿度
    GK_ALARM_TYPE_ITEV_TRIPWIRE,  //18:绊线
    GK_ALARM_TYPE_ITEV_TRIPWIRE_RESUME,  //19:绊线恢复
    GK_ALARM_TYPE_ITEV_VIDEOFOOL,  //20:视频异常
    GK_ALARM_TYPE_ITEV_VIDEOFOOL_RESUME,  //21:异常恢复恢复
    GK_ALARM_TYPE_AREA_ALARM,  //22:区域入侵
    GK_ALARM_TYPE_UPGRADE_ALARM,  //23:升级失败
    GK_ALARM_TYPE_UPLOAD_ALARM,  //24:上传文件失败
    GK_ALARM_TYPE_BUTT,
}GK_ALARM_TYPE_E;

/*
** 参数说明：
** Channel：通道号，从0开始
** AlarmType：告警类型 GK_ALARM_TYPE_E
** Action：告警状态 1：告警  0：消除
** param: 扩充指针暂不用
*/
typedef int (*event_alarm_cb_func)(int nChannel, int nAlarmType, int nAction, void* pParam);

/* 初始化事件监听 */
int event_alarm_init(void);


/* 注册事件监听，alarm_cb为事件回调函数，返回值为事件句柄，返回NULL，表示失败 */
void* event_alarm_open(event_alarm_cb_func alarm_cb);

/* 关闭事件监听*/
int event_alarm_close(void *eventhandle);

/* 其他模块调用该函数触发一个事件回调函数
** 参数说明：
** Channel：通道号，从0开始
** AlarmType：告警类型 GK_ALARM_TYPE_E
** Action：告警状态 1：告警  0：消除
** param: 扩充指针暂不用
*/
int event_alarm_touch(int channel, GK_ALARM_TYPE_E alarmType, int action, void* param);

/* 销毁事件监听 */
int event_alarm_exit(void);


#ifdef __cplusplus
}
#endif
#endif
