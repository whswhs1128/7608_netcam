/*!
*****************************************************************************
** FileName     : gk_cms_protocol.h
**
** Description  : api about cms protocol.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Date         : 2015-9-9, create.
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_CMS_PROTOCOL_H_
#define _GK_CMS_PROTOCOL_H_

#ifdef __cplusplus
extern "C"
{
#endif

//#include "DMSType.h"

#ifndef _HAVE_TYPE_BYTE
#define _HAVE_TYPE_BYTE
	typedef unsigned char       BYTE;
#endif

#ifndef _HAVE_TYPE_Bool
#define _HAVE_TYPE_Bool
typedef unsigned char       Bool;
#endif

#ifndef BOOL
#ifndef _HAVE_TYPE_BOOL
#define _HAVE_TYPE_BOOL
	typedef int					BOOL;
#endif
#endif

#ifndef _HAVE_TYPE_WORD
#define _HAVE_TYPE_WORD
	typedef unsigned short      WORD;
#endif

#ifndef _HAVE_TYPE_DWORD
#define _HAVE_TYPE_DWORD
	//typedef unsigned long		DWORD;
	typedef unsigned int         DWORD;	//xqq
	typedef unsigned long		ULONG;
#endif

#ifndef _HAVE_TYPE_ULONGLONG
#define _HAVE_TYPE_ULONGLONG
	typedef unsigned long long		ULONGLONG;
#endif

#ifndef _HAVE_TYPE_LONG
#define _HAVE_TYPE_LONG
	typedef long				LONG;
#endif

#ifndef _HAVE_TYPE_UINT
#define _HAVE_TYPE_UINT
	typedef unsigned int		UINT;
#endif

#ifndef _HAVE_TYPE_VOID
#define _HAVE_TYPE_VOID
	typedef void				VOID;
#endif

#ifndef _HAVE_TYPE_DWORD64
#define _HAVE_TYPE_DWORD64
	typedef unsigned long long  DWORD64;
	typedef unsigned long long	QWORD;
#endif

#ifndef _HAVE_TYPE_UINT8
#define _HAVE_TYPE_UINT8
	typedef unsigned char   	UINT8;
#endif


#ifndef _HAVE_TYPE_UINT16
#define _HAVE_TYPE_UINT16
	typedef unsigned short 		UINT16;
#endif

#ifndef _HAVE_TYPE_UINT32
#define _HAVE_TYPE_UINT32
	typedef unsigned int    	UINT32;
#endif

#ifndef _HAVE_TYPE_HWND
#define _HAVE_TYPE_HWND
	typedef unsigned int 		HWND;
#endif

#ifndef _HAVE_TYPE_WPARAM
#define _HAVE_TYPE_WPARAM
	typedef unsigned int    	WPARAM;
#endif

#ifndef _HAVE_TYPE_LPARAM
#define _HAVE_TYPE_LPARAM
	typedef unsigned long   	LPARAM;
#endif

#ifndef _HAVE_TYPE_SINT64
#define _HAVE_TYPE_SINT64
	typedef signed long long	SINT64;
#endif

#ifndef _HAVE_TYPE_UINT64
#define _HAVE_TYPE_UINT64
	typedef unsigned long long 	UINT64;
#endif

#ifndef _HAVE_TYPE_Int
#define _HAVE_TYPE_Int
	typedef int		Int;
#endif

#ifndef _HAVE_TYPE_Char
#define _HAVE_TYPE_Char
	typedef char	Char;
#endif

//服务器命令定义
#define		JBNV_FLAG		9000
#define		JB_STANDARD_ALARM_MSG_HEADER	0x200000
/* 6, video decode use ffmpeg; 4 hisi */
#define VIDEO_DECODE_H264_V2		4
#define VIDEO_DECODE_H265			1006
#define DMS_MAX_STREAMNUM 3


typedef enum {
    NETCMD_NOT_JBDATA = 5005,
    NETCMD_NOT_LOGON = 5006,
    NETCMD_RECV_ERR = 5018,
    NETCMD_CHANNEL_INVALID = 5019,
} GK_CMS_NETCMD;

typedef enum {
    WAVE_FORMAT_ALAW = 0x0006,
    WAVE_FORMAT_MPEGLAYER3 = 0x0055,
    WAVE_FORMAT_G722_ADPCM = 0x0065,
    WAVE_FORMAT_G711A = 0x003E,
    WAVE_FORMAT_ADPCM = 0x0002,
    WAVE_FORMAT_G726_ADPCM = 0x0064,	//海思编码的ADPCM
} GK_CMS_WAVE_TYPE;

typedef enum {
	JBCMD_SERVER_INFO =		5002,		//服务器信息
	JBCMD_PASSWORD_UNMATCH =	5003,		//密码错误
	JBCMD_MAX_LINK =			5004,		//超过最大连接数
	JBCMD_KEEP_ALIVE =		5001,		//心跳命令
	JBCMD_SERVER_MSG =		1006,		//服务器消息
	JBCMD_CH_CONNECT_BACK =	5016,		//通道反向连接
	JBCMD_PLAY_CLOSE =		5009,		//关闭通道
	JBCMD_MULTI_OPEN =		5008,		//打开多播
	JBCMD_PLAYBACK_CMD =	5007,
} emJBCMD;


typedef enum {
	JB_MSG_VIDEOLOST =					0x40000001,  //视频丢失
	JB_MSG_MOVEDETECT =                  0x40000002,  //移动报警
	JB_MSG_SENSOR_ALARM =              0x40000003,  //探头报警
	JB_MSG_RESETSERVER =                 0x40000004,  //服务器因为某些参数被改变，将要重新启动
	JB_MSG_JPEGSNAP =                   0x40000005,  //JPEG抓图
	JB_MSG_UPGRADE =                    0x40000006,  //升级进度通知
	JB_MSG_CRCERROR =                   0x40000007,  //升级CRC错
	JB_MSG_SERVER_BUSY =                 0x40000008,  //服务器忙，设置失败
	JB_MSG_SERVER_BREAK =                0x40000009,  //服务器中断连接
	JB_MSG_CHANNEL_BREAK =               0x4000000A,  //通道中断连接
	JB_MSG_TALK_REQUEST =				0x4000000B,	//远程服务器请求对讲
	JB_MSG_UPGRADEOK =          		0x4000000C,  //升级完成
	JB_MSG_VIDEORESUME =					0x4000000D,	//视频恢复
	JB_MSG_COMDATA =						0x4000000E,	//串口数据
	JB_MSG_USERDATA =					0x4000000F,	//用户数据
	JB_MSG_DISK_REMOVE	=				0x40000010,	//磁盘被探出或移出
	JB_MSG_ALARM_SNAP =					0x40000011,
	JB_MSG_FILE_DATA =					0x40000012,	//前端发回的文件，附加结构为：JBNV_DVR_FILE_DATA
	JB_MSG_DISK_ERROR =					0x40000013,	//磁盘错，文件不可读写。
	JB_MSG_TEL_ALARM =					0x40000014,	//电话告警报警
	JB_MSG_CHANNEL_RECONN_SUCCESS =		0x40000015,	//通道连接成功
	//Add 2009-02-24
	JB_MSG_TEMPHUM_LOST_ALARM =			0x40000016,	//TempHum Device is Off
	JB_MSG_TEMPHUM_TEMP_LOWER =			0x40000017,	//TempHum Temp is Lower
	JB_MSG_TEMPHUM_TEMP_UPPER =			0x40000018,	//TempHum Temp is Upper
	JB_MSG_TEMPHUM_HUM_LOWER =			0x40000019,	//TempHum Hum is Lower
	JB_MSG_TEMPHUM_HUM_UPPER =			0x4000001A,	//TempHum Hum is Upper
	JB_MSG_POWER_220V_OFF =				0x4000001B,	//220V Power is Off
	JB_MSG_POWER_48V_OFF =				0x4000001C,	//220V Power is Off
	JB_MSG_POWER_DEVICE_LOST =			0x4000001D,  //Power Device is Off
	JB_MSG_FILE_NAME_DATA =				0x4000001E, //前端发回的文件名  附加结构：JBNV_FILE_DATA_INFO
	JB_MSG_ERROR_DEVICE =				0x4000001F, //不支持的前端设备
	JB_MSG_IVS_DATA =					0x40000020, //烟火报警
	JB_MSG_SENSOR_RESUME =				0x40000021, //探头报警恢复
	JB_MSG_SERVER_LINK_OK =              0x40000022, //服务器连接成功
	#ifdef MODULE_SUPPORT_RF433
	JB_MSG_RF433_DEV_ALARM_BASENUM =	0x50000000,//433设备报警起始ID号
	JB_MSG_RF433_DEV_ALARM_MAXNUM =		0x50000032,//433设备报警最大ID号
	#endif
} emMsg;


typedef enum emJBERR
{
	JB_SUCCESS =                        0x00000000,
	JBERR_BASE	 =						0x10000000,
	JBERR_PASSWORD_UNMATCH = 			(JBERR_BASE|0x01),			//密码不匹配
	JBERR_TIME_OVER = 					(JBERR_BASE|0x02),			//操作超时
	JBERR_INVALID_PARAM = 				(JBERR_BASE|0x03),			//无效参数
	JBERR_MAX_LINK = 					(JBERR_BASE|0x04),			//超过最大连接数
	JBERR_INVALID_HANDLE = 				(JBERR_BASE|0x05),			//句柄非法或错误
	JBERR_INVALID_COMMAND = 			(JBERR_BASE|0x06),			//不被接受的命令
	JBERR_SENDCMD_FAILD	= 				(JBERR_BASE|0x07),			//发送命令数据失败
	JBERR_GETCONFIG_FAILD = 			(JBERR_BASE|0x08),			//取得命令数据失败
	JBERR_NO_LOGON = 					(JBERR_BASE|0x09),			//没有登录
	JBERR_ALLOC_FAILD = 				(JBERR_BASE|0x0a),			//分配内存失败
	JBERR_INVALID_NETADDRESS = 			(JBERR_BASE|0x0b),			//无效的或无法解析的网络地址
	JBERR_FILE_CRC32 =					(JBERR_BASE|0x0C),			//文件校验错
	JBERR_SOFTVER_ERR =					(JBERR_BASE|0x0D),			//程序版本过低，无法校验升级文件
	JBERR_CPUTYPE_ERR =					(JBERR_BASE|0x0E),			//升级文件不适用于此CPU类型的主机
	JBERR_ERROR_10054 =					(JBERR_BASE|0x0F),			//连接被服务器强迫关闭!
	JBERR_ERROR_10061 =					(JBERR_BASE|0x10),			//服务器没有在指定端口打开服务!
	JBERR_ERROR_10060 =					(JBERR_BASE|0x11),			//没有发现指定IP的服务器!
	JBERR_ERROR_10065 =					(JBERR_BASE|0x12),			//网络未准备好!
	JBERR_INITSURFACE =					(JBERR_BASE|0x13),			//初始化显示表面错误
	JBERR_UNSUPPORT	=					(JBERR_BASE|0x14),			//服务器不支持此功能
	JBERR_TALK_REJECTED =				(JBERR_BASE|0x15),			//对讲请求被服务器拒绝
	JBERR_TALK_INITAUDIO =			    (JBERR_BASE|0x16),			//启动对讲时音频初始化失败
	JBERR_OPEN_FILE =					(JBERR_BASE|0x17),			//打开文件错
	JBERR_BIND_PORT =					(JBERR_BASE|0x18),			//绑定本地端口失败
	JBERR_NO_FILE	=					(JBERR_BASE|0x19),			//没有找到文件
	JBERR_NOMORE_FILE	=				(JBERR_BASE|0x1A),			//没有更多文件
	JBERR_FILE_FINDING	=				(JBERR_BASE|0x1B),			//正在查找
	JBERR_DISK_NOTEXIST =				(JBERR_BASE|0x1C),			//格式化/分区的硬盘不存在
	JBERR_FILE_ERROR =					(JBERR_BASE|0x1D),			//文件不正确或者不匹配此服务器
	JBERR_UNINITOBJ =					(JBERR_BASE|0x1E),			//对象没有初始化，稍候再试
	JBERR_UNKNOW_SERVER =				(JBERR_BASE|0x1F),			//服务器无法识别
	JBERR_CHANNEL_NOT_OPEN =			(JBERR_BASE|0x20),			//通道没有打开，操作失败
	JBERR_INVALID_FILE =				(JBERR_BASE|0x21),			//
	JBERR_ENCRYPT_IC_NO_FIND =			(JBERR_BASE|0x22),			//服务器没有找到加密IC
	JBERR_ENCRYPT_IC_NO_MATCH =			(JBERR_BASE|0x23),			//加密IC不匹配
	JBERR_RTSP_GET_DESCRIBE =			(JBERR_BASE|0x24),			//获取RTSP描述失败
	JBERR_RTSP_SETUPAUDIO =				(JBERR_BASE|0x25),			//RTSP设置音频失败
	JBERR_RTSP_SETUPVIDIO =				(JBERR_BASE|0x26),			//RTSP设置视频失败
	JBERR_RTSP_NOSTREAM	=				(JBERR_BASE|0x27),			//RTSP没有流数据
	JBERR_RTSP_PLAY	=					(JBERR_BASE|0x28),			//RTSP播放启动失败
	JBERR_IP_ERROR	=					(JBERR_BASE|0x29),			//本地IP错误，不能连接指定服务器
	JBERR_SERVER_UPDATAING	=			(JBERR_BASE|0x2A),			//服务器正在升级，不接受客户端的连接操作
} GK_JBERR_TYPE_E;




/* 补充的遗漏命令 */
//重启
#define GK_NET_CMD_REBOOT           0x00000001
//恢复出厂设置
#define GK_NET_CMD_RESTORE			0x00000002
//保存用户配置到flash上
#define GK_NET_CMD_SAVE             0x00000003
//抓图
#define GK_NET_CMD_SNAP             0x00000004
#define GK_NET_CMD_NOTIFY           0x00200000
//服务器端主动连接配置
#define GK_NET_CMD_CONNECT_CMS      0x00030308

#define CMD_GETSYSTIME      0x10000000

typedef enum emSetServerCfg
{
	CMD_REBOOT	=				0x00000001,		//Param:
	CMD_RESTORE	=				0x00000002,		//Param:
	CMD_UPDATEFLASH	=			0x00000003,		//Param:
	CMD_SNAPSHOT =				0x00000004,		//Param:int
	CMD_SETSYSTIME =				0x00000005,		//Param:SYSTEMTIME
	CMD_SET_OSDINFO =				0x00000006,		//Param:JB_CHANNEL_OSDINFO
	CMD_SET_SHELTER =				0x00000007,		//Param:JB_CHANNEL_SHELTER
	CMD_SET_LOGO =				0x00000008,		//Param:JB_CHANNEL_LOGO
	CMD_SET_CHANNEL_CONFIG =		0x00000009,		//Param:JB_CHANNEL_CONFIG
	CMD_SET_COLOR =				0x0000000A,		//Param:JB_CHANNEL_COLOR
	CMD_SET_MOTION_DETECT =		0x0000000B,		//Param:JB_CHANNEL_MOTION_DETECT
	CMD_SET_SENSOR_ALARM =		0x0000000C,		//Param:JB_SENSOR_ALARM
	CMD_SET_VIDEO_LOST =			0x0000000D,		//Param:JB_CHANNEL_VIDEO_LOST
	CMD_SET_COMINFO =				0x0000000E,		//Param:JB_SERVER_COMINFO
	CMD_SET_USERINFO =			0x0000000F,		//Param:JB_SERVER_USER
	CMD_SET_NETWORK =				0x00000010,		//Param:JB_SERVER_NETWORK
	CMD_UPLOAD_PTZ_PROTOCOL =		0x00000011,		//Param:JB_UPLOAD_PTZ_PROTOCOL
	CMD_SEND_COMDATA =			0x00000012,		//Param:JB_COM_DATA
	CMD_SET_FTPUPDATA_PARAM =		0x00000013,		//Param:JB_FTPUPDATA_PARAM
	CMD_CLEAR_ALARM_OUT	=		0x00000014,		//Param:
	CMD_SET_SERVER_COM2	=		0x00000015,		//Param:JB_SERVER_COM2INFO
	CMD_SET_ALARM_OUT =			0x00000016,		//Param:JBNV_ALARM_OUT_INFO
	CMD_SET_NOTIFY_SERVER =		0x00000017,		//Param:JB_NOTIFY_SERVER
	CMD_SET_PPPOE_DDNS =			0x00000018,		//Param:JB_PPPOE_DDNS_CONFIG
	CMD_SET_SENSOR_STATE =		0x00000019,		//Param:JBNV_SENSOR_STATE
	CMD_SET_SERVER_RECORD =		0x00000020,		//Param:JB_SERVER_RECORD_SET
	CMD_RECORD_BEGIN =			0x00000021,		//Param:
	CMD_RECORD_STOP =				0x00000022,		//Param:
	CMD_SET_CENTER_INFO =			0x00000023,		//Param:JB_CENTER_INFO
	CMD_UPDATE_CENTER_LICENCE =	0x00000024,
	CMD_SET_CHANNEL_ALARM_CONFIG = 0x00000025,		//Param:JB_CHANNEL_ALARM_CONFIG
	CMD_SET_EMAIL_PARAM =			0x00000026,		//Param:JBNV_EMAIL_PARAM
	CMD_SET_COMMODE	=			0x00000027,		//Param:JBNV_SERVER_COMMODE
	CMD_SET_3322DDNS =			0x00000028,		//Param:JBNV_3322DDNS_CONFIG
	CMD_STOP_FILE_DOWNLOAD =		0x00000029,
	CMD_SET_NVD_SENSOR_ALARM =	0x0000002A,		//Param:JB_NVD_SENSOR_ALARM_SET
	CMD_SET_WIFI =				0x00000046,		//Param:JB_WIFI_CONFIG
	CMD_SET_TDSCDMA	 =			0x00000047,		//Param:JB_TDSCDMA_CONFIG
	CMD_SET_PERIPH_CONFIG =		0x00000048,		//Param:JB_PERIPH_CONFIG
	CMD_SET_TEL_ALARM = 	 		0x00000049,		//Param:JB_TEL_ALARM_SET
	CMD_SET_TEMPHUM_SENSOR =		0x0000004A,		//Param:JB_TEMP_HUM_SENSOR_CONFIG
	CMD_SET_POWER_DEVICE =		0x0000004B,		//Param:JB_POWER_DEVICE_CONFIG
	CMD_SET_RECORD_CONFIG =		0x0000004C,		//Param:JB_SERVER_RECORD_CONFIG
	CMD_SET_UPNP =				0x0000004D, 	//Param:JB_UPNP_CONFIG
	CMD_SET_PLAYBACK_NAME =		0x0000004E,		//Param: char *csPlayBackFileName
	CMD_SET_PLAYBACK_TIME =		0x0000004F,		//Param:JBNV_TIME
	CMD_PLAYBACK_VODACTION =		0x00000050,		//Param:JBNV
	CMD_SET_MOBILE_CENTER =		0x00000051,		//Param:JB_MOBILE_CENTER_INFO
	CMD_SET_CHANNEL_SUBSTREAM_CONFIG = 0x00000052,	//Param:JB_CHANNEL_CONFIG
	CMD_FDISK_DISK =				0x00000061,		//Param:JBNV_FDISK_DISK
	CMD_FORMAT_DISK =	 			0x00000062,		//Param:JBNV_FORMAT_DISK
	CMD_REQUEST_IFRAME =				0x00000063, //Param:JB_CHANNEL_FRAMEREQ
	CMD_SET_CENTER_INFO_EX =			0x00000064, //Param:JBNV_NXSIGHT_SERVER_ADDR_EX
	CMD_GET_CENTER_INFO_EX =			0x00000065, //Param:JBNV_NXSIGHT_SERVER_ADDR_EX
	CMD_SET_AUDIO_PARA =				0x00000066, //Param:HI_CFG_AUDIO_ATTR_S
	CMD_GET_AUDIO_PARA	=			0x00000067, //Param:HI_CFG_AUDIO_ATTR_S
	CMD_FORMAT_DISK_NEW =				0x00000068, //Param:HI_HDI_FORMAT
	CMD_TEST_SPEAKER    =           0x00000069, //Param:
	CMD_TEST_IRCUT      =           0x00000070, //Param:
	CMD_TEST_GET_TEST_RESULT        =   0x00000071, //Param: NET_TEST_RESULT_INFO
	CMD_TEST_SET_TEST_RESULT        =   0x00000072, //Param: NET_TEST_RESULT_INFO
    CMD_GET_FAC_CFG                 =   0x00000073, //Param: DMS_NET_FAC_CFG
    CMD_SET_FAC_CFG                 =   0x00000074, //Param: DMS_NET_FAC_CFG
    CMD_GET_AF_OFFSET               =   0x00000075, //Param: NET_AF_OFFSET_INFO
	CMD_SET_AF_OFFSET               =   0x00000076, //Param: NET_AF_OFFSET_INFO
    CMD_TEST_WHITE                  =   0x00000077, //Param: NET_AF_OFFSET_INFO
    CMD_TEST_CMD                    =   0x00000078, //Param: NET_AF_OFFSET_INFO
} GK_CMD_TYPE_E;


#if 0
typedef enum emSetServerCfg {
    DMS_NET_SET_EXCEPTIONCFG = 0x010901, //异常报警参数
    DMS_NET_SET_RESTORECFG = 0x020301,  //恢复出厂值配置
    DMS_NET_SET_SAVECFG = 0x020303, //保存配置
    DMS_NET_CMD_ALARMOUT_CONTROL = 0xA0200008, //报警输出控制
    DMS_NET_SET_ALARMOUTCFG = 0x010421, //设置报警输出参数
    DMS_NET_SET_ALARMINCFG = 0x010411,  //设置报警输入参数
    DMS_NET_SET_CRUISE_INFO = 0x010429, //设置一个巡航
    DMS_NET_SET_SHELTERCFG = 0x010231, //图像遮挡
    //DMS_NET_CMD_PTZ_CONTROL = 0xA0200007, //云台控制
    DMS_NET_CMD_PTZ_CONTROL = 0x00000050, //云台控制
    DMS_NET_SET_DEVICECFG = 0x010001, //设备信息
    DMS_NET_SET_WANWIRELESSCFG_V2 = 0x010173, //广域网无线
    DMS_NET_SET_PICCFG = 0x010201, //设置图象压缩参数
    DMS_NET_SET_COLORCFG_SINGLE = 0x010223, //单独设置某个图象色彩参数,
    DMS_NET_SET_COLOR_BLACK_DETECTION = 0x010227, //设置彩转黑检测参数
    DMS_UPNP_SET_CONFIG = 0x010181, //设置UPNP参数
    DMS_NET_SET_FTPCFG = 0x010151, //设置FTP参数
    DMS_NET_SET_OSDCFG = 0x010211, //设置图象字符叠加参数
    DMS_SET_MOBILE_CENTER_INFO = 0x010191, //设置手机平台参数
    DMS_NET_SET_RTSPCFG = 0x020305, //rtsp设置
    DMS_NET_SET_NETCFG = 0x010101, //设置网络参数
    DMS_NET_SET_PPPOECFG = 0x010121, //设置PPPOE参数
    DMS_NET_SET_P2PCFG = 0x020411, //设置P2P参数
    DMS_DEV_SET_DEVICEMAINTAINCFG = 0x020601, //设备维护
    DMS_NET_SET_EMAILCFG = 0x010141,  //设置EMAIL参数
    DMS_NET_SET_RECORDCFG = 0x010271, //设置图象录像参数
    DMS_NET_SET_RECORDSTREAMMODE = 0x010274l, //设置录像流类型
    DMS_NET_SET_RECORDMODECFG = 0x010281, //设置图象手动录像参数
    DMS_NET_SET_SNMPCFG = 0x040701, //设置SNMP参数
    DMS_NET_CMD_SET_SYSTIME = 0xA0200014, //设置系统时间
    DMS_NET_SET_NTPCFG = 0x010521, //设置NTP 参数
    DMS_NET_DEF_NTPCFG = 0x010522, //默认NTP 参数
    DMS_NET_SET_ZONEANDDSTCFG = 0x010511, //时区和夏时制
    DMS_NET_SET_SNAPTIMERCFG = 0x010301, //设置图像定时抓拍参数
    DMS_NET_SET_SNAPEVENTCFG = 0x010303, //设置图像事件抓拍参数
    DMS_NET_SET_RS232CFG = 0x010311, //设置232串口参数
    DMS_NET_GET_DEF_SERIAL = 0x010312, //默认串口参数
    DMS_NET_SET_USERCFG = 0x010801, //设置用户参数
    DMS_NET_SET_USERGROUPCFG = 0x010811, //设置用户组参数
    DMS_NET_SET_MOTIONCFG = 0x010241, //设置图象移动侦测参数
    DMS_NET_DEF_MOTIONCFG = 0x010242, //默认图像移动侦测参数
    DMS_NET_SET_VLOSTCFG = 0x010251, //设置图象视频丢失参数
    DMS_NET_DEF_VLOSTCFG = 0x010252, //默认图像视频丢失参数
    DMS_NET_SET_HIDEALARMCFG = 0x010261, //设置图象遮挡报警参数
    DMS_NET_DEF_HIDEALARMCFG = 0x010262, //默认图像遮挡报警参数
    DMS_NET_SET_WIFICFG = 0x010161, //设置WIFI参数
    DMS_NET_SET_DDNSCFG = 0x010131, //设置DDNS参数
    DMS_NET_SET_WIFI_WPS_START = 0x010163, //设置WPS开始
    DMS_NET_CMD_REBOOT = 0xA020000C, // 重启设备
    DMS_NET_CMD_SHUTDOWN = 0xA020000D, // 设备关机
    DMS_NET_CMD_SNAPSHOT = 0xA020000E, // 抓拍图片
} GK_CMS_CMD_SET;

typedef enum emGetServerCfg {
    DMS_NET_GET_EXCEPTIONCFG = 0x010900,  //获取异常参数
    DMS_NET_GET_RESTORECFG = 0x020300, //恢复出厂值配置
    DMS_NET_GET_DEF_RESTORECFG = 0x020302, //获取默认恢复出厂值配置
    DMS_NET_GET_ALARMOUTCFG = 0x010420, //获取报警输出参数
    DMS_NET_GET_ALARMOUT_STATE = 0xA0200009, // 获取报警输出状态
    DMS_NET_GET_ALARMINCFG = 0x010410, //获取报警输入参数
    DMS_NET_GET_ALL_PRESET = 0x010427, //获取所有已设置的预置点的信息
    DMS_NET_GET_CRUISE_CFG = 0x010428, //获取通道的所有已经设置的巡航组
    DMS_NET_GET_SHELTERCFG = 0x010230, //获取图象遮挡参数
    DMS_NET_GET_DEVICECFG = 0x010000, //获取设备参数
    DMS_NET_GET_DEF_DEVICECFG = 0x010003, //设置默认设备参数
    DMS_NET_GET_WANWIRELESSCFG_V2 = 0x010172, //获取广域网无线参数
    DMS_NET_GET_SUPPORT_STREAM_FMT = 0x010202, // //获取系统支持的图像能力
    DMS_NET_GET_PICCFG = 0x010200, //获取图象压缩参数
    DMS_NET_GET_DEF_PICCFG = 0x010203, //获取默认图像压缩参数
    DMS_NET_GET_COLOR_SUPPORT = 0x010222, //获取图象色彩参数
    DMS_NET_GET_ENHANCED_COLOR_SUPPORT = 0x010224, //获取图象色彩支持高级参数
    DMS_NET_GET_ENHANCED_COLOR = 0x010226, //获取图象色彩高级参数
    DMS_NET_GET_COLOR_BLACK_DETECTION = 0x010228, //获取彩转黑检测参数
    DMS_NET_GET_COLORCFG = 0x010220, //获取图象色彩参数
    DMS_NET_GET_DEF_COLORCFG = 0x010225, //获取图像色彩参数
    DMS_UPNP_GET_CONFIG = 0x010180, //获取UPNP参数
    DMS_NET_GET_FTPCFG = 0x010150, //获取FTP参数
    DMS_NET_GET_OSDCFG = 0x010210, //获取图象字符叠加参数
    DMS_GET_MOBILE_CENTER_INFO = 0x010190, //获取手机平台参数
    DMS_NET_GET_NETCFG = 0x010100, //获取网络参数
    DMS_NET_GET_DEF_NETCFG = 0x010103, //获取默认网络参数
    DMS_NET_GET_RTSPCFG = 0x020304, //获取RTSP参数
    DMS_NET_GET_PPPOECFG = 0x010120, //获取PPPOE参数
    DMS_NET_GET_P2PCFG = 0x020410, //获取P2P参数
    DMS_DEV_GET_DEVICEMAINTAINCFG = 0x020600, //获取设备维护
    DMS_NET_GET_LOGINFO = 0x040500, //获取LOG信息
    DMS_NET_GET_REC_DAY_IN_MONTH = 0x050101,
    DMS_NET_GET_EMAILCFG = 0x010140, //获取EMAIL参数
    DMS_NET_GET_DEF_RECORDCFG = 0x010272, //获取默认的录像参数
    DMS_NET_GET_RECORDCFG = 0x010270, //获取图象录像参数
    DMS_NET_GET_RECORDSTREAMMODE = 0x010273,//获取录像流类型
    DMS_NET_GET_RECORDMODECFG = 0x010280, //获取图象手动录像参数
    DMS_NET_GET_SNMPCFG = 0x040700, //获取snmp参数
    DMS_NET_GET_ZONEANDDSTCFG = 0x010510, //获取时区和夏时制参数
    DMS_NET_GET_NTPCFG = 0x010520, ////获取NTP 参数
    DMS_NET_GET_SNAPTIMERCFG = 0x010300,//获取图像定时抓拍参数
    DMS_NET_GET_SNAPEVENTCFG = 0x010302, //获取图像事件抓拍参数
    DMS_NET_GET_RS232CFG = 0x010310, //获取232串口参数
    DMS_NET_GET_USERCFG = 0x010800, //获取用户参数
    DMS_NET_GET_MOTIONCFG = 0x010240, //获取图象移动侦测参数
    DMS_NET_GET_VLOSTCFG = 0x010250, //获取图象视频丢失参数
    DMS_NET_GET_HIDEALARMCFG = 0x010260, //获取图象遮挡报警参数
    DMS_NET_GET_WIFI_SITE_LIST = 0x010162, //获取WIFI 站点列表
    DMS_NET_GET_WIFICFG = 0x010160, //获取WIFI参数
    DMS_NET_GET_DDNSCFG = 0x010130, //获取DDNS参数
    DMS_NET_CMD_UPGRADE_REQ = 0xA0200021, //文件系统升级请求
    DMS_NET_CMD_UPGRADE_RESP = 0xA0200022, //文件系统升级请求
    DMS_NET_CMD_UPGRADE_DATA = 0xA0200023, //文件系统升级数据
    DMS_NET_CMD_UPDATE_FLASH = 0xA0200027, //更新Flash
} GK_CMS_CMD_GET;
#endif

#if 0
/************************************************************************/
/*       参数配置相关子命令                                              */
/************************************************************************/
//1) 设备（DMS_NET_DEVICE_INFO结构）
#define DMS_NET_GET_DEVICECFG				0x020000		//获取设备参数
#define DMS_NET_SET_DEVICECFG				0x020001		//设置设备参数
#define DMS_NET_GET_DEF_DEVICECFG			0x020003		//设置设备参数

//2）网络
//本地网络（DMS_NET_NETWORK_CFG结构）
#define DMS_NET_GET_NETCFG					0x020100		//获取网络参数
#define DMS_NET_SET_NETCFG					0x020101		//设置网络参数
#define DMS_NET_GET_DEF_NETCFG				0x020103

//中心管理平台（DMS_NET_PLATFORM_INFO_V2结构）
#define DMS_NET_GET_PLATFORMCFG				0x020110		//获取中心管理平台参数
#define DMS_NET_SET_PLATFORMCFG				0x020111		//设置中心管理平台参数

//PPPOE（DMS_NET_PPPOECFG结构）
#define DMS_NET_GET_PPPOECFG				0x020120		//获取PPPOE参数
#define DMS_NET_SET_PPPOECFG				0x020121		//设置PPPOE参数
//DDNS（DMS_NET_DDNSCFG结构）
#define DMS_NET_GET_DDNSCFG					0x020130		//获取DDNS参数
#define DMS_NET_SET_DDNSCFG					0x020131		//设置DDNS参数
//EMAIL（DMS_NET_EMAIL_PARAM结构）
#define DMS_NET_GET_EMAILCFG				0x020140		//获取EMAIL参数
#define DMS_NET_SET_EMAILCFG				0x020141		//设置EMAIL参数
//FTP（DMS_NET_FTP_PARAM结构）
#define DMS_NET_GET_FTPCFG					0x020150		//获取FTP参数
#define DMS_NET_SET_FTPCFG					0x020151		//设置FTP参数
//WIFI（DMS_NET_WIFI_CONFIG结构）
#define DMS_NET_GET_WIFICFG					0x020160		//获取WIFI参数
#define DMS_NET_SET_WIFICFG					0x020161		//设置WIFI参数
#define DMS_NET_GET_WIFI_SITE_LIST			0x020162		//获取WIFI 站点列表
//广域网无线（DMS_NET_WANWIRELESS_CONFIG结构）
#define DMS_NET_GET_WANWIRELESSCFG			0x020170		//获取WANWIRELESS参数
#define DMS_NET_SET_WANWIRELESSCFG			0x020171		//设置WANWIRELESS参数
//UPNP (DMS_UPNP_CONFIG结构)
#define DMS_UPNP_GET_CONFIG					0x020180			//获取UPNP参数
#define DMS_UPNP_SET_CONFIG					0x020181			//设置UPNP参数
//手机注册平台参数(DMS_MOBILE_CENTER_INFO结构)
#define DMS_GET_MOBILE_CENTER_INFO			0x020190			//获取手机平台参数
#define DMS_SET_MOBILE_CENTER_INFO			0x020191			//设置手机平台参数


//3）音视频通道
//图象压缩（DMS_NET_CHANNEL_PIC_INFO结构）
#define DMS_NET_GET_PICCFG					0x020200		//获取图象压缩参数
#define DMS_NET_GET_SUPPORT_STREAM_FMT  	0x020202        //获取系统支持的图像能力
#define DMS_NET_SET_PICCFG					0x020201		//设置图象压缩参数
#define DMS_NET_GET_DEF_PICCFG				0x020203

//图像字符叠加（DMS_NET_CHANNEL_OSDINFO结构）
#define DMS_NET_GET_OSDCFG					0x020210		//获取图象字符叠加参数
#define DMS_NET_SET_OSDCFG					0x020211		//设置图象字符叠加参数
//图像色彩（DMS_NET_CHANNEL_COLOR结构）
#define DMS_NET_GET_COLORCFG				0x020220		//获取图象色彩参数
#define DMS_NET_SET_COLORCFG				0x020221		//设置图象色彩参数

//(DMS_NET_COLOR_SUPPORT结构)
#define DMS_NET_GET_COLOR_SUPPORT			0x020222		//获取图象色彩参数
//图像色彩(DMS_NET_CHANNEL_COLOR_SINGLE结构)
#define DMS_NET_SET_COLORCFG_SINGLE			0x020223		//单独设置某个图象色彩参数,
#define DMS_NET_GET_DEF_COLORCFG			0x020225

//(DMS_NET_ENHANCED_COLOR_SUPPORT结构)
#define DMS_NET_GET_ENHANCED_COLOR_SUPPORT	0x020224		//获取图象色彩支持高级参数
//(DMS_NET_CHANNEL_ENHANCED_COLOR结构)
#define DMS_NET_GET_ENHANCED_COLOR			0x020226		//获取图象色彩高级参数


//图像遮挡（DMS_NET_CHANNEL_SHELTER结构）
#define DMS_NET_GET_SHELTERCFG				0x020230		//获取图象遮挡参数
#define DMS_NET_SET_SHELTERCFG				0x020231		//设置图象遮挡参数
//图像移动侦测（DMS_NET_CHANNEL_MOTION_DETECT结构）
#define DMS_NET_GET_MOTIONCFG				0x020240		//获取图象移动侦测参数
#define DMS_NET_SET_MOTIONCFG				0x020241		//设置图象移动侦测参数
#define DMS_NET_DEF_MOTIONCFG				0x020242
//图像视频丢失（DMS_NET_CHANNEL_VILOST结构）
#define DMS_NET_GET_VLOSTCFG				0x020250		//获取图象视频丢失参数
#define DMS_NET_SET_VLOSTCFG				0x020251		//设置图象视频丢失参数
#define DMS_NET_DEF_VLOSTCFG				0x020252
//图像遮挡报警（DMS_NET_CHANNEL_HIDEALARM结构）
#define DMS_NET_GET_HIDEALARMCFG			0x020260		//获取图象遮挡报警参数
#define DMS_NET_SET_HIDEALARMCFG			0x020261		//设置图象遮挡报警参数
#define DMS_NET_DEF_HIDEALARMCFG			0x020262

//图像录像（DMS_NET_CHANNEL_RECORD结构）
#define DMS_NET_GET_RECORDCFG				0x020270		//获取图象录像参数
#define DMS_NET_SET_RECORDCFG				0x020271		//设置图象录像参数
#define DMS_NET_GET_DEF_RECORDCFG			0x020272
//图像手动录像（DMS_NET_CHANNEL_RECORD结构)
#define DMS_NET_GET_RECORDMODECFG			0x020280		//获取图象手动录像参数
#define DMS_NET_SET_RECORDMODECFG			0x020281		//设置图象手动录像参数

//解码器（DMS_NET_DECODERCFG结构）
#define DMS_NET_GET_DECODERCFG				0x020290		//获取解码器参数
#define DMS_NET_SET_DECODERCFG				0x020291		//设置解码器参数
#define DMS_NET_GET_DEF_DECODERCFG		    0x020292		//设置解码器参数

//图像定时抓拍（DMS_NET_SNAP_TIMER结构）
#define DMS_NET_GET_SNAPTIMERCFG			0x020300		//获取图像定时抓拍参数
#define DMS_NET_SET_SNAPTIMERCFG			0x020301		//设置图像定时抓拍参数

//4）串口（DMS_NET_RS232CFG结构）
#define DMS_NET_GET_RS232CFG 				0x020310		//获取232串口参数
#define DMS_NET_SET_RS232CFG				0x020311		//设置232串口参数
#define DMS_NET_GET_DEF_SERIAL				0x020312

//(DMS_NET_PTZ_PROTOCOLCFG结构)
#define DMS_NET_GET_PTZ_PROTOCOLCFG 		0x020316		//获取支持的云台协议信息
//(DMS_NET_PTZ_PROTOCOL_DATA 结构)
#define DMS_NET_ADD_PTZ_PROTOCOL            0x020317		//添加云台协议

//5）报警
//输入（DMS_NET_ALARMINCFG结构）
#define DMS_NET_GET_ALARMINCFG 				0x020410		//获取报警输入参数
#define DMS_NET_SET_ALARMINCFG				0x020411		//设置报警输入参数
#define DMS_NET_DEF_ALARMINCFG				0x020412

//输出（DMS_NET_ALARMOUTCFG结构）
#define DMS_NET_GET_ALARMOUTCFG 			0x020420		//获取报警输出参数
#define DMS_NET_SET_ALARMOUTCFG				0x020421		//设置报警输出参数
#define DMS_NET_DEF_ALARMOUTCFG				0x020422


//6）时间
//时区和夏时制（DMS_NET_ZONEANDDST结构）
#define DMS_NET_GET_ZONEANDDSTCFG 			0x020510		//获取时区和夏时制参数
#define DMS_NET_SET_ZONEANDDSTCFG			0x020511		//设置时区和夏时制参数

//7）本地预览（DMS_NET_PREVIEWCFG结构）
#define DMS_NET_GET_PREVIEWCFG 				0x020600		//获取预览参数
#define DMS_NET_SET_PREVIEWCFG				0x020601		//设置预览参数
#define DMS_NET_GET_DEF_PREVIEWCFG			0x020602
//8）视频输出（DMS_NET_VIDEOOUT结构）
#define DMS_NET_GET_VIDEOOUTCFG 			0x020700		//获取视频输出参数
#define DMS_NET_SET_VIDEOOUTCFG				0x020701		//设置视频输出参数
#define DMS_NET_GET_DEF_VIDEOOUTCFG			0x020702		//设置视频输出参数
//9) 用户、用户组
//用户（DMS_NET_USER_INFO结构）
#define DMS_NET_GET_USERCFG 				0x020800		//获取用户参数
#define DMS_NET_SET_USERCFG					0x020801		//设置用户参数
//用户组（DMS_NET_USER_GROUP_INFO结构）
#define DMS_NET_GET_USERGROUPCFG 			0x020810		//获取用户组参数
#define DMS_NET_SET_USERGROUPCFG			0x020811		//设置用户组参数
//10)异常（DMS_NET_EXCEPTION结构）
#define DMS_NET_GET_EXCEPTIONCFG 			0x020900		//获取异常参数
#define DMS_NET_SET_EXCEPTIONCFG			0x020901		//设置异常参数
//11)硬盘
//本地硬盘信息（DMS_NET_HDCFG结构）
#define DMS_NET_GET_HDCFG 					0x030100		//获取硬盘参数
#define DMS_NET_SET_HDCFG					0x030101		//设置(单个)硬盘参数
#define DMS_NET_HD_FORMAT           		0x030102        //格式化硬盘
#define DMS_NET_GET_HD_FORMAT_STATUS        0x030103        //格式化硬盘状态以及进度

//本地盘组信息配置（DMS_NET_HDGROUP_CFG结构）
#define DMS_NET_GET_HDGROUPCFG 				0x030200		//获取硬盘组参数
#define DMS_NET_SET_HDGROUPCFG				0x030201		//设置硬盘组参数
//12)恢复出厂值配置（DMS_NET_RESTORECFG）
#define DMS_NET_GET_RESTORECFG          	0x030300    	//恢复出厂值配置
#define DMS_NET_SET_RESTORECFG          	0x030301
#define DMS_NET_GET_DEF_RESTORECFG          0x030302
#define DMS_NET_SET_SAVECFG          0x030303 //保存配置
//13) RTSP
#define DMS_NET_GET_RTSPCFG          0x030304
#define DMS_NET_SET_RTSPCFG          0x030305 //rtsp设置

//配置回调函数
// 配置音频对讲数据回调函数(OnNetAudioStreamCallBack)
#define DMS_NET_REG_AUDIOCALLBACK          	0xA0100001
// 配置视频数据回调函数(DMS_NET_VIDEOCALLBACK)
// 每个通道的不通码流可以分别配置不同的回调函数接口
#define DMS_NET_REG_VIDEOCALLBACK          	0xA0100002
// 配置报警消息回调函数(OnNetAlarmCallback)
#define DMS_NET_REG_ALARMCALLBACK          	0xA0100003
// 配置串口数据回调函数(OnNetSerialDataCallback)
#define DMS_NET_REG_SERIALDATACALLBACK      0xA0100004
// 打开透明串口传输
#define  DMS_NET_CMD_OPEN_SERIALTRANSP  	0xA0100005
// 关闭透明串口传输
#define  DMS_NET_CMD_CLOSE_SERIALTRANSP 	0xA0100006
// 云台控制( DMS_NET_PTZ_CONTROL 结构体)
#define DMS_NET_CMD_PTZ_CONTROL             0xA0100007
// 报警输出控制(命令模式DMS_NET_ALARMOUT_CONTROL)
#define DMS_NET_CMD_ALARMOUT_CONTROL        0xA0100008
// 获取报警输出状态(DMS_NET_SENSOR_STATE)
#define	DMS_NET_GET_ALARMOUT_STATE			0xA0100009
// 获取报警输入状态(DMS_NET_SENSOR_STATE)
#define	DMS_NET_GET_ALARMIN_STATE			0xA010000A
// 请求视频流关键帧
#define	DMS_NET_CMD_IFRAME_REQUEST        	0xA010000B
// 重启设备
#define 	DMS_NET_CMD_REBOOT		       	0xA010000C
// 设备关机
#define 	DMS_NET_CMD_SHUTDOWN		   	0xA010000D
// 抓拍图片(DMS_NET_SNAP_DATA)
#define 	DMS_NET_CMD_SNAPSHOT		   	0xA010000E
//录像控制(DMS_NET_REC_CONTROL)
#define 	DMS_NET_CMD_REC_CONTROL		    	0xA010000F
//注册回放回调函数
#define   DMS_NET_REG_PLAYBACKCALLBACK      0xA0100010
//开始音频对讲
#define 	DMS_NET_CMD_START_TALKAUDIO	0xA0100011
//停止音频对讲
#define 	DMS_NET_CMD_STOP_TALKAUDIO		0xA0100012
//获取系统时间
#define 	DMS_NET_CMD_GET_SYSTIME			0xA0100013
//设置系统时间
#define 	DMS_NET_CMD_SET_SYSTIME 			0xA0100014
//打开某通道视频
#define 	DMS_NET_CMD_START_VIDEO  		0xA0100015
//关闭某通道视频
#define 	DMS_NET_CMD_STOP_VIDEO 			0xA0100016
// 透明串口传输
#define  DMS_NET_CMD_DATA_SERIALTRANSP  	0xA0100017


//hujh add 2012-05-11
#define   DMS_NET_CMD_GET_SPSPPSBASE64ENCODE_DATA 0xA0100018
#define 	DMS_NET_CMD_CLOSE_RS485 			0xA0100019
#define 	DMS_NET_CMD_GET_VIDEO_STATE   	0xA0100020

//文件系统升级请求(DMS_NET_UPGRADE_REQ)
#define   DMS_NET_CMD_UPGRADE_REQ		0xA0100021
//文件系统升级请求(DMS_NET_UPGRADE_RESP)
#define   DMS_NET_CMD_UPGRADE_RESP		0xA0100022
//文件系统升级数据(DMS_NET_UPGRADE_DATA)
#define   DMS_NET_CMD_UPGRADE_DATA		0xA0100023
//更新Flash
#define  DMS_NET_CMD_UPDATE_FLASH 0xA0100027


//显示指定OSD文本--DMS_NET_SPRCIAL_ESD
#define DMS_NET_CMD_SHOW_SPECIAL_OSD        0xA0100024

//获取所有已设置的预置点的信息
#define DMS_NET_GET_ALL_PRESET  0x010427
//获取通道的所有已经设置的巡航组
#define DMS_NET_GET_CRUISE_CFG 0x010428
//设置一个巡航
#define DMS_NET_SET_CRUISE_INFO 0x010429

//广域网无线参数
#define DMS_NET_GET_WANWIRELESSCFG_V2 0x010172
#define DMS_NET_SET_WANWIRELESSCFG_V2 0x010173

//彩转黑检测参数
#define DMS_NET_GET_COLOR_BLACK_DETECTION 0x010228
#define DMS_NET_SET_COLOR_BLACK_DETECTION 0x010227

//P2P参数
#define DMS_NET_GET_P2PCFG 0x020410
#define DMS_NET_SET_P2PCFG 0x020411

//设备维护
#define DMS_DEV_GET_DEVICEMAINTAINCFG 0x020600
#define DMS_DEV_SET_DEVICEMAINTAINCFG 0x020601

//获取LOG信息
#define DMS_NET_GET_LOGINFO 0x040500

//录像流类型
#define DMS_NET_GET_REC_DAY_IN_MONTH 0x050101
#define DMS_NET_GET_RECORDSTREAMMODE 0x010273
#define DMS_NET_SET_RECORDSTREAMMODE 0x010274

//snmp参数
#define DMS_NET_GET_SNMPCFG  0x040700
#define DMS_NET_SET_SNMPCFG  0x040701

//NTP 参数
#define DMS_NET_GET_NTPCFG 0x010520
#define DMS_NET_SET_NTPCFG 0x010521
#define DMS_NET_DEF_NTPCFG 0x010522

//获取图像事件抓拍参数
#define DMS_NET_GET_SNAPEVENTCFG 0x010302
#define DMS_NET_SET_SNAPEVENTCFG 0x010303

//设置WPS开始
#define DMS_NET_SET_WIFI_WPS_START 0x010163
#endif

/************************************************************************/
/*       参数配置相关子命令                                              */
/************************************************************************/
//1) 设备（DMS_NET_DEVICE_INFO结构）
#define DMS_NET_GET_DEVICECFG               0x020000        //获取设备参数
#define DMS_NET_SET_DEVICECFG               0x020001        //设置设备参数
#define DMS_NET_GET_DEF_DEVICECFG           0x020003        //设置设备参数
#define DMS_NET_GET_DEVICE_EXTCFG			0x020004		//获取设备扩展信息
#define DMS_NET_SET_DEVICE_EXTCFG			0x020005		//配置设备扩展信息


//2）网络
//本地网络（DMS_NET_NETWORK_CFG结构）
#define DMS_NET_GET_NETCFG                  0x020100        //获取网络参数
#define DMS_NET_SET_NETCFG                  0x020101        //设置网络参数
#define DMS_NET_GET_DEF_NETCFG              0x020103

//中心管理平台（DMS_NET_PLATFORM_INFO_V2结构）
#define DMS_NET_GET_PLATFORMCFG             0x020110        //获取中心管理平台参数
#define DMS_NET_SET_PLATFORMCFG             0x020111        //设置中心管理平台参数

//PPPOE（DMS_NET_PPPOECFG结构）
#define DMS_NET_GET_PPPOECFG                0x020120        //获取PPPOE参数
#define DMS_NET_SET_PPPOECFG                0x020121        //设置PPPOE参数

//DDNS（DMS_NET_DDNSCFG结构）
#define DMS_NET_GET_DDNSCFG                 0x020130        //获取DDNS参数
#define DMS_NET_SET_DDNSCFG                 0x020131        //设置DDNS参数

//EMAIL（DMS_NET_EMAIL_PARAM结构）
#define DMS_NET_GET_EMAILCFG                0x020140        //获取EMAIL参数
#define DMS_NET_SET_EMAILCFG                0x020141        //设置EMAIL参数
#define DMS_NET_TEST_SEND_EMAIL             0x020142        //测试发送EMAIL

//FTP（DMS_NET_FTP_PARAM结构）
#define DMS_NET_GET_FTPCFG                  0x020150        //获取FTP参数
#define DMS_NET_SET_FTPCFG                  0x020151        //设置FTP参数
#define DMS_NET_TEST_FTP_CONNECT            0x020152        //测试FTP连接

//WIFI（DMS_NET_WIFI_CONFIG结构）
#define DMS_NET_GET_WIFICFG                 0x020160        //获取WIFI参数
#define DMS_NET_SET_WIFICFG                 0x020161        //设置WIFI参数
#define DMS_NET_GET_WIFI_SITE_LIST          0x020162        //获取WIFI 站点列表
#define DMS_NET_SET_WIFI_WPS_START          0x020163        //设置WPS开始

#define DMS_NET_TEST_WIFI_CONNECT			0x020164        //测试WIFI 连接

//广域网无线（DMS_NET_WANWIRELESS_CONFIG结构）
#define DMS_NET_GET_WANWIRELESSCFG          0x020170        //获取WANWIRELESS参数
#define DMS_NET_SET_WANWIRELESSCFG          0x020171        //设置WANWIRELESS参数

//广域网无线（DMS_NET_WANWIRELESS_CONFIG_V2结构）
#define DMS_NET_GET_WANWIRELESSCFG_V2       0x020172        //获取WANWIRELESS参数
#define DMS_NET_SET_WANWIRELESSCFG_V2       0x020173        //设置WANWIRELESS参数

//UPNP (DMS_UPNP_CONFIG结构)
#define DMS_UPNP_GET_CONFIG                 0x020180            //获取UPNP参数
#define DMS_UPNP_SET_CONFIG                 0x020181            //设置UPNP参数
//手机注册平台参数(DMS_MOBILE_CENTER_INFO结构)
#define DMS_GET_MOBILE_CENTER_INFO          0x020190            //获取手机平台参数
#define DMS_SET_MOBILE_CENTER_INFO          0x020191            //设置手机平台参数


//3）音视频通道
//图象压缩（DMS_NET_CHANNEL_PIC_INFO结构）
#define DMS_NET_GET_PICCFG                  0x020200        //获取图象压缩参数
#define DMS_NET_SET_PICCFG                  0x020201        //设置图象压缩参数
#define DMS_NET_GET_DEF_PICCFG              0x020203
//DMS_NET_SUPPORT_STREAM_FMT  图像能力
#define DMS_NET_GET_SUPPORT_STREAM_FMT      0x020202        //获取系统支持的图像能力

//图像字符叠加（DMS_NET_CHANNEL_OSDINFO结构）
#define DMS_NET_GET_OSDCFG                  0x020210        //获取图象字符叠加参数
#define DMS_NET_SET_OSDCFG                  0x020211        //设置图象字符叠加参数
//图像色彩（DMS_NET_CHANNEL_COLOR结构）
#define DMS_NET_GET_COLORCFG                0x020220        //获取图象色彩参数
#define DMS_NET_SET_COLORCFG                0x020221        //设置图象色彩参数

//(DMS_NET_COLOR_SUPPORT结构)
#define DMS_NET_GET_COLOR_SUPPORT           0x020222        //获取图象色彩参数
//图像色彩(DMS_NET_CHANNEL_COLOR_SINGLE结构)
#define DMS_NET_SET_COLORCFG_SINGLE         0x020223        //单独设置某个图象色彩参数,
#define DMS_NET_GET_DEF_COLORCFG            0x020225

//(DMS_NET_ENHANCED_COLOR_SUPPORT结构)
#define DMS_NET_GET_ENHANCED_COLOR_SUPPORT  0x020224        //获取图象色彩支持高级参数
//(DMS_NET_CHANNEL_ENHANCED_COLOR结构)
#define DMS_NET_GET_ENHANCED_COLOR          0x020226        //获取图象色彩高级参数

//DMS_NET_DAY_NIGHT_DETECTION_EX
#define DMS_NET_SET_COLOR_BLACK_DETECTION   0x020227        //设置彩转黑检测参数
#define DMS_NET_GET_COLOR_BLACK_DETECTION   0x020228        //获取彩转黑检测参数

//图像遮挡（DMS_NET_CHANNEL_SHELTER结构）
#define DMS_NET_GET_SHELTERCFG              0x020230        //获取图象遮挡参数
#define DMS_NET_SET_SHELTERCFG              0x020231        //设置图象遮挡参数
//图像移动侦测（DMS_NET_CHANNEL_MOTION_DETECT结构）
#define DMS_NET_GET_MOTIONCFG               0x020240        //获取图象移动侦测参数
#define DMS_NET_SET_MOTIONCFG               0x020241        //设置图象移动侦测参数
#define DMS_NET_DEF_MOTIONCFG               0x020242
//图像视频丢失（DMS_NET_CHANNEL_VILOST结构）
#define DMS_NET_GET_VLOSTCFG                0x020250        //获取图象视频丢失参数
#define DMS_NET_SET_VLOSTCFG                0x020251        //设置图象视频丢失参数
#define DMS_NET_DEF_VLOSTCFG                0x020252
//图像遮挡报警（DMS_NET_CHANNEL_HIDEALARM结构）
#define DMS_NET_GET_HIDEALARMCFG            0x020260        //获取图象遮挡报警参数
#define DMS_NET_SET_HIDEALARMCFG            0x020261        //设置图象遮挡报警参数
#define DMS_NET_DEF_HIDEALARMCFG            0x020262

//图像录像（DMS_NET_CHANNEL_RECORD结构）
#define DMS_NET_GET_RECORDCFG               0x020270        //获取图象录像参数
#define DMS_NET_SET_RECORDCFG               0x020271        //设置图象录像参数
#define DMS_NET_GET_DEF_RECORDCFG           0x020272
//get/set record stream type (DMS_NET_RECORD_STREAMMODE)
#define DMS_NET_GET_RECORDSTREAMMODE        0x020273
#define DMS_NET_SET_RECORDSTREAMMODE        0x020274

//图像手动录像（DMS_NET_CHANNEL_RECORD结构)
#define DMS_NET_GET_RECORDMODECFG           0x020280        //获取图象手动录像参数
#define DMS_NET_SET_RECORDMODECFG           0x020281        //设置图象手动录像参数

//解码器（DMS_NET_DECODERCFG结构）
#define DMS_NET_GET_DECODERCFG              0x020290        //获取解码器参数
#define DMS_NET_SET_DECODERCFG              0x020291        //设置解码器参数
#define DMS_NET_GET_DEF_DECODERCFG          0x020292        //设置解码器参数

//图像定时抓拍（DMS_NET_SNAP_TIMER结构）
#define DMS_NET_GET_SNAPTIMERCFG            0x020300        //获取图像定时抓拍参数
#define DMS_NET_SET_SNAPTIMERCFG            0x020301        //设置图像定时抓拍参数

//图像事件抓拍（DMS_NET_SNAP_EVENT结构）
#define DMS_NET_GET_SNAPEVENTCFG            0x020302        //获取图像事件抓拍参数
#define DMS_NET_SET_SNAPEVENTCFG            0x020303        //设置图像事件抓拍参数

//4）串口（DMS_NET_RS232CFG结构）
#define DMS_NET_GET_RS232CFG                0x020310        //获取232串口参数
#define DMS_NET_SET_RS232CFG                0x020311        //设置232串口参数
#define DMS_NET_GET_DEF_SERIAL              0x020312
//(DMS_NET_PTZ_PROTOCOLCFG结构)
#define DMS_NET_GET_PTZ_PROTOCOLCFG         0x020316        //获取支持的云台协议信息
//(DMS_NET_PTZ_PROTOCOL_DATA 结构)
#define DMS_NET_ADD_PTZ_PROTOCOL            0x020317        //添加云台协议
//(DMS_NET_PTZ_PROTOCOL_DATA 结构)
#define DMS_NET_GET_PTZ_PROTOCOL_DATA       0x020318        //获取云台协议数据
//5）报警
//输入（DMS_NET_ALARMINCFG结构）
#define DMS_NET_GET_ALARMINCFG              0x020410        //获取报警输入参数
#define DMS_NET_SET_ALARMINCFG              0x020411        //设置报警输入参数
#define DMS_NET_DEF_ALARMINCFG              0x020412

//输出（DMS_NET_ALARMOUTCFG结构）
#define DMS_NET_GET_ALARMOUTCFG             0x020420        //获取报警输出参数
#define DMS_NET_SET_ALARMOUTCFG             0x020421        //设置报警输出参数
#define DMS_NET_DEF_ALARMOUTCFG             0x020422
//默认配置
#define DMS_NET_GET_DEFAULTCFG_FILE         0x020423
#define DMS_NET_SET_DEFAULTCFG_FILE         0x020424
//logo文件
#define DMS_NET_GET_LOGO_FILE               0x020425
#define DMS_NET_SET_LOGO_FILE               0x020426
//获取所有已设置的预置点的信息-DMS_NET_PRESET_INFO
#define DMS_NET_GET_ALL_PRESET              0x020427
//获取通道的所有已经设置的巡航组 DMS_NET_CRUISE_CFG
#define DMS_NET_GET_CRUISE_CFG              0x020428
//设置一个巡航   DMS_NET_CRUISE_INFO
#define DMS_NET_SET_CRUISE_INFO				0x020429

//6）时间
//时区和夏时制（DMS_NET_ZONEANDDST结构）
#define DMS_NET_GET_ZONEANDDSTCFG           0x020510        //获取时区和夏时制参数
#define DMS_NET_SET_ZONEANDDSTCFG           0x020511        //设置时区和夏时制参数
//NTP （DMS_NET_NTP_CFG结构）
#define DMS_NET_GET_NTPCFG					0x020520        //获取NTP 参数
#define DMS_NET_SET_NTPCFG					0x020521        //设置NTP 参数
#define DMS_NET_DEF_NTPCFG					0x020522

//7）本地预览（DMS_NET_PREVIEWCFG结构）
#define DMS_NET_GET_PREVIEWCFG              0x020600        //获取预览参数
#define DMS_NET_SET_PREVIEWCFG              0x020601        //设置预览参数
#define DMS_NET_GET_DEF_PREVIEWCFG          0x020602
//8）视频输出（DMS_NET_VIDEOOUT结构）
#define DMS_NET_GET_VIDEOOUTCFG             0x020700        //获取视频输出参数
#define DMS_NET_SET_VIDEOOUTCFG             0x020701        //设置视频输出参数
#define DMS_NET_GET_DEF_VIDEOOUTCFG         0x020702        //设置视频输出参数
//9) 用户、用户组
//用户（DMS_NET_USER_INFO结构）
#define DMS_NET_GET_USERCFG                 0x020800        //获取用户参数
#define DMS_NET_SET_USERCFG                 0x020801        //设置用户参数
//用户组（DMS_NET_USER_GROUP_INFO结构）
#define DMS_NET_GET_USERGROUPCFG            0x020810        //获取用户组参数
#define DMS_NET_SET_USERGROUPCFG            0x020811        //设置用户组参数
//10)异常（DMS_NET_EXCEPTION结构）
#define DMS_NET_GET_EXCEPTIONCFG            0x020900        //获取异常参数
#define DMS_NET_SET_EXCEPTIONCFG            0x020901        //设置异常参数
//11)硬盘
//本地硬盘信息（DMS_NET_HDCFG结构）
#define DMS_NET_GET_HDCFG                   0x030100        //获取硬盘参数
#define DMS_NET_SET_HDCFG                   0x030101        //设置(单个)硬盘参数
#define DMS_NET_HD_FORMAT                   0x030102        //格式化硬盘
#define DMS_NET_GET_HD_FORMAT_STATUS        0x030103        //格式化硬盘状态以及进度
#define DMS_NET_UNLOAD_DISK                 0x030104        //卸载磁盘
//#define DMS_NET_UNLOAD_DISK                 0x10000013        //卸载磁盘


#define DMS_NET_SET_NASCFG                     0x030105        //设置NAS参数
#define DMS_NET_GET_NASCFG                     0x030106        //获取NAS参数

//本地盘组信息配置（DMS_NET_HDGROUP_CFG结构）
#define DMS_NET_GET_HDGROUPCFG              0x030200        //获取硬盘组参数
#define DMS_NET_SET_HDGROUPCFG              0x030201        //设置硬盘组参数
//12)恢复出厂值配置（DMS_NET_RESTORECFG）
#define DMS_NET_GET_RESTORECFG              0x030300
#define DMS_NET_SET_RESTORECFG              0x030301
#define DMS_NET_GET_DEF_RESTORECFG          0x030302
#define DMS_NET_SET_SAVECFG					0x030303 		//保存配置

//13) RTSP
#define DMS_NET_GET_RTSPCFG					0x030304
#define DMS_NET_SET_RTSPCFG					0x030305 		//rtsp设置


//14)  P2P
#define DMS_NET_GET_P2PCFG                  0x030410
#define DMS_NET_SET_P2PCFG                  0x030411

//15)用户密钥数据
#define DMS_NET_GET_USERKEY                 0x030500
#define DMS_NET_SET_USERKEY                 0x030501

//16)HVR 设备维护
#define DMS_DEV_GET_DEVICEMAINTAINCFG		0x030600
#define DMS_DEV_SET_DEVICEMAINTAINCFG		0X030601

//
#define DMS_NET_CMD_AUTO_TEST               0x040411
#define DMS_NET_GET_LOGINFO					0x050500

//远程录像回放
#define  DMS_NET_GET_REC_DAY_IN_MONTH				0x060101
#define	  DMS_NET_GET_TS_INFO									0x060001
#define  DMS_NET_SET_TS_INFO									0x060002
#define DMS_NET_GET_UPGRADE_STATUS					0x060003
#define DMS_NET_CMD_REMOTE_PLAYBACK			0x050600

#define		CMD_GET_FILELIST		0x1000004E	//Param:JBNV_FIND_FILE_RESP


//SNMP
#define DMS_NET_GET_SNMPCFG                  0x050700
#define DMS_NET_SET_SNMPCFG                  0x050701
#ifdef MODULE_SUPPORT_RF433
#define DMS_RF433_DEV_LEARN_CODE			0x070001
#define DMS_RF433_DEV_GET_DEV_LIST			0x070002
#define DMS_RF433_DEV_MODIFY_NAME			0x070003
#define DMS_RF433_DEV_DELETE_DEV			0x070004
#define DMS_RF433_DEV_EXIT_LEARN_CODE		0x070005
#endif
//配置回调函数
// 配置音频对讲数据回调函数(OnNetAudioStreamCallBack)
#define DMS_NET_REG_AUDIOCALLBACK           0xA0100001
// 配置视频数据回调函数(DMS_NET_VIDEOCALLBACK)
// 每个通道的不通码流可以分别配置不同的回调函数接口
#define DMS_NET_REG_VIDEOCALLBACK           0xA0100002
// 配置报警消息回调函数(OnNetAlarmCallback)
#define DMS_NET_REG_ALARMCALLBACK           0xA0100003
// 配置串口数据回调函数(OnNetSerialDataCallback)
#define DMS_NET_REG_SERIALDATACALLBACK      0xA0100004
// 打开透明串口传输
#define  DMS_NET_CMD_OPEN_SERIALTRANSP      0xA0100005
// 关闭透明串口传输
#define  DMS_NET_CMD_CLOSE_SERIALTRANSP     0xA0100006
// 云台控制( DMS_NET_PTZ_CONTROL 结构体)
#define DMS_NET_CMD_PTZ_CONTROL             0xA0100007
// 报警输出控制(命令模式DMS_NET_ALARMOUT_CONTROL)
#define DMS_NET_CMD_ALARMOUT_CONTROL        0xA0100008
// 获取报警输出状态(DMS_NET_SENSOR_STATE)
#define DMS_NET_GET_ALARMOUT_STATE          0xA0100009
// 获取报警输入状态(DMS_NET_SENSOR_STATE)
#define DMS_NET_GET_ALARMIN_STATE           0xA010000A
// 请求视频流关键帧
#define DMS_NET_CMD_IFRAME_REQUEST          0xA010000B
// 重启设备
#define     DMS_NET_CMD_REBOOT              0xA010000C

// 设备关机
#define     DMS_NET_CMD_SHUTDOWN            0xA010000D
// 抓拍图片(DMS_NET_SNAP_DATA)
#define     DMS_NET_CMD_SNAPSHOT            0xA010000E
#define     DMS_NET_CMD_SNAPSHOT_V2         0xA020000E
//录像控制(DMS_NET_REC_CONTROL)
#define     DMS_NET_CMD_REC_CONTROL					0xA010000F
//注册回放回调函数
#define   DMS_NET_REG_PLAYBACKCALLBACK				0xA0100010
//开始音频对讲
#define     DMS_NET_CMD_START_TALKAUDIO				0xA0100011
//停止音频对讲
#define     DMS_NET_CMD_STOP_TALKAUDIO				0xA0100012
//获取系统时间
#define     DMS_NET_CMD_GET_SYSTIME                 0xA0100013
//设置系统时间
#define     DMS_NET_CMD_SET_SYSTIME                 0xA0100014
//打开某通道视频
#define     DMS_NET_CMD_START_VIDEO                 0xA0100015
//关闭某通道视频
#define     DMS_NET_CMD_STOP_VIDEO                  0xA0100016

//hujh add 2012-05-11
#define   DMS_NET_CMD_GET_SPSPPSBASE64ENCODE_DATA   0xA0100018
#define     DMS_NET_CMD_CLOSE_RS485                 0xA0100019
#define     DMS_NET_CMD_GET_VIDEO_STATE             0xA0100020

//文件系统升级请求(DMS_NET_UPGRADE_REQ)
#define   DMS_NET_CMD_UPGRADE_REQ                   0xA0100021
//文件系统升级请求(DMS_NET_UPGRADE_RESP)
#define   DMS_NET_CMD_UPGRADE_RESP                  0xA0100022
//文件系统升级数据(DMS_NET_UPGRADE_DATA)
#define   DMS_NET_CMD_UPGRADE_DATA                  0xA0100023

//显示指定OSD文本--DMS_NET_SPRCIAL_ESD
#define DMS_NET_CMD_SHOW_SPECIAL_OSD                0xA0100024
//2013-1-25 15:05:47--FTP上传图片
#define DMS_NET_CMD_UPLOAD_FTP                      0xA0100025
//2013-1-25 15:05:47--发送EMAIL
#define DMS_NET_CMD_SEND_MAIL                       0xA0100026

#define DMS_NET_CMD_UPDATE_FLASH                    0xA0100027

#define DMS_NET_CMD_COM_DATA						0xA0100028
//获取周界防范 -- DMS_PERIMETER_RULE_PARA_S
#define DMS_NET_CMD_GET_PERIMETER_PARA				0xA0100029
//设置周界防范 -- DMS_PERIMETER_RULE_PARA_S
#define DMS_NET_CMD_SET_PERIMETER_PARA				0xA010002A
//推送智能分析目标坐标--DMS_TGT_SET_S
#define DMS_NET_CMD_ITEV_PUSH_TARGET				0xA010002B
//获取智能分析能力集信息
#define DMS_NET_CMD_GET_ITEV_CTRLCFG				0xA010002C
//设置智能分析能力集信息
#define DMS_NET_CMD_SET_ITEV_CTRLCFG				0xA010002D
//设置单绊线
#define DMS_NET_CMD_SET_TRIPWIRE_PARA				0xA010002E
//获取单绊线
#define DMS_NET_CMD_GET_TRIPWIRE_PARA				0xA010002F
//设置视频异常
#define DMS_NET_CMD_SET_VIDEO_ANOMALY_PARA                    0xA0100030
//获取视频异常
#define DMS_NET_CMD_GET_VIDEO_ANOMALY_PARA                    0xA0100031
#define DMS_NET_CMD_GET_RECORDFILEENDTIME				0xA0100032
#define DMS_NET_CMD_REG_TEMPERATUREHUMIDITY_CALL		0xA0100033
#define DMS_NET_CMD_REG_HEADCOUNTREG_CALL				0xA0100034

////////////
#ifdef MODULE_SUPPORT_RF433
/*
	433 device:
	                id,type,name
*/
typedef enum{
	GKCMS_DEV_433_TYPE_REMOTE_CONTROL = 10,	// 遥控
	GKCMS_DEV_433_TYPE_ALARM,					// 报警
	GKCMS_DEV_433_TYPE_OTHER,					// 其他
}ENUM_GKCMS_DEV_433_TYPE;

typedef struct{
	int 	id;				// 433 标识
	int		type;			// 433 设备类型 
	char	name[64];		// 设备名称		
}GkCmsCfg433Req,GkCmsSet433Req,GkCmsDel433Req,GkCmsS433Dev;;
typedef struct{
	int		num;			// 设备数量
	GkCmsS433Dev	dev[32];		// 设备
}GkCmsGet433Resp;

typedef struct{
	int		result;			// ENUM_AVIOTC_CFG_433_RET,0 :success,1:failure
	int		none;			// 预留
}GkCmsGet433Req,GkCmsCfg433Resp,GkCmsSet433Resp,GkCmsDel433Resp,GkCmsCfg433ExitReq,GkCmsCfg433ExitResp;
#endif


typedef struct _SYSTEMTIME
{
    WORD wYear;
    WORD wMonth;
    WORD wDayOfWeek;
    WORD wDay;
    WORD wHour;
    WORD wMinute;
    WORD wSecond;
    WORD wMilliseconds;
} SYSTEMTIME;

typedef struct tagDMS_SIZE
{
	int           nWidth;
	int           nHeight;
}DMS_SIZE;

typedef struct tagDMS_RANGE
{
	int			nMin;
	int			nMax;
}DMS_RANGE, *LDMS_RANGE;

/*网络连接注册数据结构体*/
typedef struct _ACCEPT_HEAD
{
	unsigned long	nFlag;  //协议版本标识
	unsigned long	nSockType;
}ACCEPT_HEAD,*PACCEPT_HEAD;

typedef struct _USER_INFO
{
	char	szUserName[32];
	char	szUserPsw[32];
	char	MacCheck[36];
} USER_INFO,*PUSER_INFO;

typedef enum
{
    NETCMD_NOT_USER = 5003,
    NETCMD_MAX_LINK = 5004,
    JBERR_IPMAC_UNMATCH = 0x10000014,
} LOGIN_ERR;

typedef enum _PROTOCOL_TYPE
{
    PROTOCOL_TCP = 0,
    PROTOCOL_UDP = 1,
    PROTOCOL_MULTI = 2
}PROTOCOL_TYPE;

//CHECK_USER_INFO
typedef struct tagCHECK_USER_INFO
{
	ULONG	nFlag;			//JBNV_FLAG
	ULONG	nSockType;		//SOCK_TYPE枚举中 SOCK_LOGON
	char	szUserName[32];		//用户名
	char	szPassword[32];		//密码
	BYTE	bMacAddress[36];	//pc mac地址
}CHECK_USER_INFO,*PCHECK_USER_INFO;


//SOCK_TYPE
typedef enum _SOCK_TYPE
{
	SOCK_LOGON = 0,
	SOCK_DATA = 1,
	SOCK_FILE = 2,
	SOCK_TALKBACK = 3,
	SOCK_CENTER,
}SOCK_TYPE;


//JB_NET_PACK_HEAD
typedef struct tagJB_NET_PACK_HEAD
{
    ULONG	nFlag;			//9000
    ULONG	nCommand; 		//服务器返回结果
    ULONG	nChannel;		//可忽略
    ULONG	nErrorCode;		//可忽略
    ULONG	nBufSize;		//成功后返回的数据大小
}JB_NET_PACK_HEAD,*PJB_NET_PACK_HEAD;


//JBNV_SERVER_INFO
typedef struct tagJBNV_SERVER_INFO
{
	DWORD	dwSize;			//结构大小
	DWORD	dwServerFlag;		//服务器类型
	DWORD	dwServerIp;		//服务器IP(整数表示形式)
	char		szServerIp[16];		//服务器IP(字符串表示形式)
	WORD		wServerPort;		//服务器端口
	WORD		wChannelNum;		//通道数量
	DWORD	dwVersionNum;		//版本
	char		szServerName[32];	//服务器名称
	DWORD	dwServerCPUType;	//当前CPU类型
	BYTE		bServerSerial[48];  //服务器序列号，具有唯一标识功能
	BYTE		byMACAddr[6];		//服务器的物理地址
	DWORD	dwAlarmInCount;
	DWORD	dwAlarmOutCount;
	DWORD	dwSysFlags;		//系统支持的功能
	DWORD	dwUserRight;		//当前用户权限
	DWORD	dwNetPriviewRight;	//网络观看权限
	char		csServerDes[64];
}JBNV_SERVER_INFO,*PJBNV_SERVER_INFO;



//CMD_SERVER_INFO
typedef struct tagCMD_SERVER_INFO{
	DWORD	dwMultiAddr;
	DWORD	dwServerIndex;
	JBNV_SERVER_INFO ServerInfo;
}CMD_SERVER_INFO;


//JBNV_CHANNEL_INFO
typedef struct tagJBNV_CHANNEL_INFO
{
	DWORD	dwSize;
	DWORD	dwStream1Height;	//视频高(1)
	DWORD	dwStream1Width;	//视频宽
	DWORD	dwStream1CodecID;	//视频编码类型号（MPEG4为								0，JPEG2000为1,H264为2）
	DWORD	dwStream2Height;	//视频高(2)
	DWORD	dwStream2Width;	//视频宽
	DWORD	dwStream2CodecID;	//视频编码类型号（MPEG4为								0，JPEG2000为1,H264为2）
	DWORD	dwAudioChannels;	//音频通道数
	DWORD	dwAudioBits;		//音频比特率
	DWORD	dwAudioSamples;	//音频采样率
	DWORD	dwWaveFormatTag;	//音频编码类型号
	char		csChannelName[32];	//通道名称
}JBNV_CHANNEL_INFO,*PJBNV_CHANNEL_INFO;

//JBNV_SENSOR_INFO
typedef struct tagJBNV_SENSOR_INFO
{
    DWORD	dwSize;
    DWORD	dwIndex;			//探头索引
    DWORD	dwSensorType;			//探头类型
    char		csSensorName[32];		//探头名称
}JBNV_SENSOR_INFO,*PJBNV_SENSOR_INFO;


//OPEN_HEAD
typedef struct _OPEN_HEAD
{
	ULONG		nFlag2;			// JBNV_FLAG
	ULONG		nID;			//设备登录时								CMD_SERVER_INFO结构体中的dwServerIndex
	DWORD   	nProtocolType;		//0
	ULONG		nStreamType;		//码流类型 0 主码流 1 子码流
	ULONG		nSerChannel;		//通道号
	ULONG		nClientChannel;		//要显示的窗口号
}OPEN_HEAD,*POPEN_HEAD;

//JB_FRAME_HEADER
typedef struct tagJB_FRAME_HEADER
{
    WORD		wMotionDetect;		//运动量
    WORD		wFrameIndex;		//帧索引
    DWORD	nVideoSize;		//视频大小
    DWORD	nTimeTick;		//时间戳
    WORD		nAudioSize;		//音频大小
    BYTE		bKeyFrame;		//是否关键帧
    BYTE		nPackIndex;		//包索引
}JB_FRAME_HEADER,*PJB_FRAME_HEADER;

//JB_DATAPACK
#define		JB_PACKDATA_SIZE	(64 *1024)
typedef struct tagJB_DATAPACK
{
    WORD	IsSampleHead;		//是否一帧的第一个数据包
    WORD	BufSize;			// Buf 中有多少可用
    JB_FRAME_HEADER	jbFrameHeader;
    BYTE	PackData[JB_PACKDATA_SIZE];		//音视频数据
}JB_DATAPACK, *PJB_DATAPACK;

//JB_DATA_PACKET
typedef struct tagJB_DATA_PACKET
{
    ULONG			nFlag;		//JBNV_FLAG
    ULONG			nSize;		//packData 数据大小
    JB_DATAPACK		packData;
}JB_DATA_PACKET,*PJB_DATA_PACKET;


//CHANNEL_OPTION
typedef struct tagCHANNEL_OPTION
{
	ULONG		nFlag;		//JBNV_FLAG
	ULONG		nID;		//设备登录时CMD_SERVER_INFO结构							体中的dwServerIndex
	DWORD	nProtocolType;	//0
	ULONG		nStreamType;	//码流类型 0 主码流 1 子码流
	ULONG		nSerChannel;	//通道号
	ULONG		nClientChannel;	//要显示的窗口号
}CHANNEL_OPTION,*PCHANNEL_OPTION;

//CMD_CLOSE_CHANNEL
typedef struct tagCMD_CLOSE_CHANNEL
{
	JB_NET_PACK_HEAD	packhead;
	CHANNEL_OPTION	option;
}CMD_CLOSE_CHANNEL;



//JBNV_PTZ_CONTROL






//JB_SERVER_NETWORK
typedef struct tagJB_SERVER_NETWORK
{
	DWORD	dwSize;
	DWORD	dwNetIpAddr;			//IP地址
	DWORD       dwNetMask;			//掩码
	DWORD       dwGateway;			//网关
	BYTE		bEnableDHCP;			//DHCP
	BYTE		bSetDNS;
	BYTE		bReserve;			//保留
	BYTE		bVideoStandard;			//0 - NTSC, 1 - PAL
	DWORD       dwHttpPort;			//Http端口
	DWORD       dwDataPort;			//数据端口
	DWORD	dwDNSServer;			//DNS服务器
	DWORD	dwTalkBackIp;		//设备告警时自动连接的对讲IP
	Char		szMacAddr[6];			//网卡MAC地址
	char		szServerName[32];		//服务器名称
}JB_SERVER_NETWORK,*PJB_SERVER_NETWORK;


//DMS_NET_SUPPORT_STREAM_FMT
typedef struct tagDMS_NET_SUPPORT_STREAM_FMT
{
    DWORD       dwSize;                     //struct size
    DWORD           dwChannel;
    DWORD       dwVideoSupportFmt[DMS_MAX_STREAMNUM][4]; // Video Format.
    DMS_RANGE   stVideoBitRate[DMS_MAX_STREAMNUM];
    DMS_SIZE    stVideoSize[DMS_MAX_STREAMNUM][10];// Video Size(height,width)
    DWORD       dwAudioFmt[4];              //Audio Format
    DWORD       dwAudioSampleRate[4];       //Audio Sample Rate
    BOOL        bSupportAudioAEC;           //b Support Audio Echo Cancellation
    BYTE            byStreamCount;           //max is DMS_MAX_STREAMNUM
    BYTE            byReserve[31];
}DMS_NET_SUPPORT_STREAM_FMT;



#define PACK_SIZE (8 * 1024)
typedef struct _NET_DATA_HEAD
{
	unsigned long	nFlag;
	unsigned long	nSize;
} NET_DATA_HEAD,*PNET_DATA_HEAD;

typedef struct tagFRAME_HEADER
{
	WORD		wMotionDetect;
	WORD		wFrameIndex;
	DWORD	dwVideoSize;
	DWORD	dwTimeTick;
	WORD		wAudioSize;
	BYTE		byKeyFrame;
	BYTE		byReserved;
}FRAME_HEADER,*PFRAME_HEADER;

typedef struct tagDATA_PACKET
{
	WORD			wIsSampleHead;
	WORD			wBufSize;
	FRAME_HEADER	stFrameHeader;
	unsigned char		byPackData[PACK_SIZE];
}DATA_PACKET,*PDATA_PACKET;

typedef struct _NET_DATA_PACKET
{
    NET_DATA_HEAD       stPackHead;
    DATA_PACKET         stPackData;
} NET_DATA_PACKET,*PNET_DATA_PACKET;

/*搜索*/
typedef struct tagJB_SERVER_MSG{
	DWORD		dwMsg;		//消息类型代码
	DWORD		dwChannel;	//通道
	SYSTEMTIME	st;			//服务器时间
	DWORD		cbSize;		//附加数据长度
}JB_SERVER_MSG,*PJB_SERVER_MSG;

typedef struct tagJBNV_SERVER_PACK
{
	char	szIp[16];			//服务器Ip
	WORD	wMediaPort;			//流端口
	WORD	wWebPort;			//Http端口号
	WORD	wChannelCount;		//通道数量
	char	szServerName[32];	//服务器名
	DWORD	dwDeviceType;		//服务器类型
	DWORD	dwServerVersion;	//服务器版本
	WORD	wChannelStatic;		//通道状态(是否视频丢失)
	WORD	wSensorStatic;		//探头状态
	WORD	wAlarmOutStatic;	//报警输出状态
}JBNV_SERVER_PACK;

typedef struct tagJBNV_SERVER_PACK_EX
{
	JBNV_SERVER_PACK jspack;
	BYTE	        bMac[6];
	BOOL	        bEnableDHCP;
	BOOL			bEnableDNS;
	DWORD			dwNetMask;
	DWORD			dwGateway;
	DWORD			dwDNS;
	DWORD			dwComputerIP;
	BOOL	        bEnableCenter;
	DWORD			dwCenterIpAddress;
	DWORD			dwCenterPort;
	char			csServerNo[64];
	int				bEncodeAudio;
}JBNV_SERVER_PACK_EX;

typedef struct tagJBNV_SERVER_MSG_DATA
{
	DWORD						dwSize;
	DWORD						dwPackFlag;
	JBNV_SERVER_PACK			jbServerPack;
	JB_SERVER_MSG				jbMsg;
}JBNV_SERVER_MSG_DATA;

typedef struct tagJBNV_SERVER_MSG_DATA_EX
{
	DWORD					dwSize;
	DWORD					dwPackFlag; // == SERVER_PACK_FLAG
	JBNV_SERVER_PACK_EX		jbServerPack;
}JBNV_SERVER_MSG_DATA_EX;

/*时间*/
typedef struct  _tagDMS_SYSTEMTIME
{
	WORD wYear;
	WORD wMonth;
	WORD wDayOfWeek;
	WORD wDay;
	WORD wHour;
	WORD wMinute;
	WORD wSecond;
	WORD wMilliseconds;
}DMS_SYSTEMTIME;

//请求关键帧
typedef struct tagJB_CHANNEL_FRAMEREQ
{
	DWORD	dwSize;
	DWORD	dwChannel;
	DWORD	dwStreamType;
	DWORD	dwFrameType;		// 0 - I，其余保留
}JB_CHANNEL_FRAMEREQ, *PJB_CHANNEL_FRAMEREQ;

////////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////
//增加部分
///////////////////////////////////////////////


#define DMS_MAX_OSD_LEN 	128
#define DMS_MAX_VIDEO_HIDE_RECT 		5
#define DMS_MAX_OSD_NUM 				5  //1路视频编码最大支持视频叠加个数
#define DMS_NAME_LEN 				32
#define DMS_SERIALNO_LEN 				48
#define DMS_MACADDR_LEN 				6
#define DMS_MAX_ETHERNET 				2
#define DMS_PATHNAME_LEN 			128
#define DMS_PASSWD_LEN 		 		32
#define DMS_MAX_CHANNUM 			32
#define DMS_MAX_STREAMNUM 		3
#define DMS_MAX_ALARMIN 				16
#define DMS_MAX_ALARMOUT 			8
#define DMS_MAX_DAYS 				8
#define DMS_MAX_TIMESEGMENT 			4
#define DMS_MAX_PRESET					255
#define DMS_MAX_CRUISE_GROUP_NUM 	    5
#define DMS_MAX_CRUISE_POINT_NUM 		32
#define DMS_PHONENUMBER_LEN 			32
#define DMS_MAX_DISKNUM 			16
#define DMS_MAX_WINDOW				16
#define DMS_MAX_PLAYER_WINDOW 	16
#define DMS_MAX_VGA 			 		1
#define DMS_MAX_USERNUM 				32
#define DMS_MAX_USERGROUPNUM 			8
#define DMS_MAX_EXCEPTIONNUM 			16
#define DMS_MAX_LINK 					6
#define DMS_MAXCARD_NUM 			40
#define DMS_MAX_SERIALNUM 		3
#define DMS_CARDNUM_LEN 				20
#define DMS_MAX_VIDEOOUT 				2
#define DMS_MAX_DISPLAY_REGION			16
#define DMS_MAX_RIGHT					32
#define DMS_MAX_DOMAIN_NAME 			64
#define DMS_MAX_PREVIEW_MODE 			8
#define DMS_PTZ_PROTOCOL_NUM  	200   /* 最大支持的云台协议数 */
#define DMS_MAX_HD_GROUP 			16
#define DMS_MAX_DDNS_NUMS 		3
#define DMS_MAX_IP_LENGTH 		16
#define DMS_MAX_IPV6_LENGTH          128
#define DMS_MAX_PATH_LEN 			256
#define DMS_EMAIL_ADDR_MAXNUM (3)

#define DMS_MD_STRIDE_WIDTH     44
#define DMS_MD_STRIDE_HEIGHT    36
#define DMS_MD_STRIDE_SIZE        (DMS_MD_STRIDE_WIDTH*DMS_MD_STRIDE_HEIGHT/8)

typedef struct
{
    char  csIpV4[DMS_MAX_IP_LENGTH];
    BYTE  byRes[DMS_MAX_IPV6_LENGTH];
}DMS_NET_IPADDR, *LPDMS_NET_IPADDR;


typedef enum __DMS_DDNS_TYPE
{
    DDNS_3322 = 1,
    DDNS_DYNDNS,
    DDNS_NIGHTOWLDVR,
    DDNS_NOIP,
    DDNS_MYEYE,
    DDNS_PEANUTHULL,
    DDNS_ANSHIBAO,//see-world.cc
	DDNS_OURVIDEO,//0328
    DDNS_BUTT,
}DMS_DDNS_TYPE;

typedef struct
{
    DMS_NET_IPADDR  strIPAddr;
    DMS_NET_IPADDR  strIPMask;  //掩码地址
    DWORD dwNetInterface;       //网络接口 1-10MBase-T 2-10MBase-T全双工 3-100MBase-TX 4-100M全双工 5-10M/100M自适应
    WORD  wDataPort;             //数据端口
    WORD  wMTU;                 //MTU大小
    BYTE  byMACAddr[DMS_MACADDR_LEN];   //Mac地址
    char   csNetName[DMS_NAME_LEN]; //网卡名称，用于多网卡区别
    BYTE  byRes[2];
}DMS_NET_ETHERNET, *LPDMS_NET_ETHERNET;

typedef struct tagDMS_TIME
{
    DWORD    dwYear;
    DWORD    dwMonth;
    DWORD    dwDay;
    DWORD    dwHour;
    DWORD    dwMinute;
    DWORD    dwSecond;
}DMS_TIME, *LPDMS_TIME;

typedef struct DMS_NET_EMAIL_ADDR
{
    char  csName[DMS_NAME_LEN];/**< 邮件地址对应的用户名 */
    char  csAddress[DMS_MAX_DOMAIN_NAME];   /**< 邮件地址 如: hw@huawei.com */
    BYTE byReserve[32];               /**< 保留字,按讨论结果预留32字节 */
} DMS_NET_EMAIL_ADDR;

/** 录像计划*/
typedef struct
{
    BYTE    byRecordType[7][48]; /* 录象类型 按位有效，
    								见DMS_NET_RECORD_TYPE_SCHED/MOTION/ALARM (不包含DMS_NET_RECORD_TYPE_MANU)*/
    BYTE    byHour[7][48];     /**< 录像计划时间半点为单位*/
} DMS_RECORDSCHED;




typedef struct hiOSD_Net_S
{
    BOOL        bOsdEnable;         /**< OSD使能*/
    DWORD   dwOsdContentType;    /**< OSD内容类型HI_CFG_OSD_TYPE_E */
    DWORD   dwLayer;             /**< 区域层次，在多个区域叠加时有用,0 ~100*/
    DWORD   dwAreaAlpha;         /**< OSD区域Alpha值*/
    DWORD   dwFgColor;           /**< OSD颜色, 象素格式RGB8888 */
    DWORD   dwBgColor;           /**< OSD背景颜色, 象素格式RGB8888 */
    int     nLeftX;       /**< 区域左上角相对于窗口左上角X坐标比例:0~255*/
    int     nLeftY;       /**< 区域左上角相对于窗口左上角Y坐标比例:0~255*/
    int     nWidth;       /**< 区域宽，为绝对值,图片时有效*/
    int     nHeight;       /**< 区域高，为绝对值,图片时有效*/
    char        csOsdCotent[DMS_MAX_OSD_LEN];/**< OSD为字符时，支持输入string；
                                                 OSD为图片时，支持path: "/bin/osd.bmp";
                                                 如果是图片内存，前4位为内存地址*/
}DMS_NET_OSD_S;


//遮挡区域设置
typedef struct tagDMS_NET_SHELTER_RECT
{
    int     nType;          /* 遮挡使能 0-禁用, 1-编码遮挡, 2-预览遮挡, 3-全部遮挡 */
    WORD    wLeft;          /* 遮挡区域左上角的x坐标 */
    WORD    wTop;           /* 遮挡区域左上角的y坐标 */
    WORD    wWidth;         /* 遮挡区域宽度 */
    WORD    wHeight;        /* 遮挡区域高度 */
    DWORD   dwColor;        /* 遮挡颜色, 默认 0:黑色 按RGB格式*/
}DMS_NET_SHELTER_RECT;

//视频通道压缩参数
typedef struct tagDMS_NET_COMPRESSION_INFO
{
    DWORD   dwCompressionType;      //
    DWORD   dwFrameRate;            //帧率 (1-25/30) PAL为25，NTSC为30
    DWORD   dwStreamFormat;         //视频分辨率 (0为CIF,1为D1,2为HALF-D1,3为QCIF) DMS_VIDEO_FORMAT_CIF
    WORD    wHeight;            //不做实际用途，只用于显示
    WORD    wWidth;             //不做实际用途，只用于显示
    DWORD   dwRateType;         //流模式(0为定码流，1为变码流)
    DWORD   dwBitRate;          //码率 (16000-4096000)
    DWORD   dwImageQuality;     //编码质量(0-4),0为最好
    DWORD   dwMaxKeyInterval;   //关键帧间隔(1-100)

    WORD    wEncodeAudio;       //是否编码音频
    WORD    wEncodeVideo;       //是否编码视频

    WORD    wFormatTag;        /* format type */
    WORD    wBitsPerSample;    /* Number of bits per sample of mono data */

    BYTE    byReseved[16];
}DMS_NET_COMPRESSION_INFO, *LPDMS_NET_COMPRESSION_INFO;


typedef struct tagDMS_SCHEDTIME
{
    //开始时间
    BYTE byStartHour;
    BYTE byStartMin;
    //结束时间
    BYTE byStopHour;
    BYTE byStopMin;
}DMS_SCHEDTIME;


// 云台联动
typedef struct
{
    BYTE  byType;//详见DMS_PTZ_LINK_TYPE，0:无效，1:预置点，2:点间巡航，3:启用轨迹
    BYTE  byValue;
    BYTE  Reserve[2];
} DMS_PTZ_LINK, *LPDMS_PTZ_LINK;

// 报警联动结构体
typedef struct
{
    /* 当前报警所支持的处理方式，按位掩码表示 */
    WORD                	wActionMask;

    /* 触发动作，按位掩码表示，具体动作所需要的参数在各自的配置中体现 */
    WORD                	wActionFlag;

    /* 报警触发的输出通道，报警触发的输出，为1表示触发该输出 */
    BYTE                		byRelAlarmOut[DMS_MAX_ALARMOUT/8];

    /* 联动录象 */
    BYTE                		byRecordChannel[DMS_MAX_CHANNUM/8]; /* 报警触发的录象通道，为1表示触发该通道 */

    /* 抓图通道 */
    BYTE                		bySnap[DMS_MAX_CHANNUM/8]; /*0表示不抓拍该通道，1表示抓拍当前通道*/
    /* 轮巡通道 */
    BYTE                		byTour[DMS_MAX_CHANNUM/8]; /* 该版本暂不生效*/

    /* 云台联动 */
    DMS_PTZ_LINK        stPtzLink[DMS_MAX_CHANNUM];
    WORD                	wDuration;              /* 报警输出持续时间 */
    WORD                	wRecTime;               /* 联动录象持续时间 */
    WORD                	wSnapNum;              /* 连续抓拍图片数量*/
    BYTE                		wDelayTime;             /* 报警间隔时间，s为单位*/
    BYTE                		wBuzzerTime;            /* 蜂鸣器输出时间*/

} DMS_HANDLE_EXCEPTION;






//升级文件头信息
typedef struct tagDMS_UPGRADE_FILE_HEADER
{
    DWORD   dwFlag;         //升级文件类型(DMS_UPGRADE_FILE_FLAG)
    DWORD   dwSize;         //本结构体大小
    DWORD   dwVersion;      //文件版本
    DWORD   dwItemCount;    //包内包含的文件总数
    DWORD   dwPackVer;      //打包的版本
    DWORD   dwCPUType;      //CPU类型
    DWORD   dwChannel;      //服务器通道数量0x01 | 0x04 | 0x08
    DWORD   dwServerType;   //服务器类型
    char        csDescrip[64];      //描述信息
    BYTE    byReserved[32];
}DMS_UPGRADE_FILE_HEADER, *LPDMS_UPGRADE_FILE_HEADER;

//升级文件数据头信息
typedef struct tagUPGRADE_ITEM_HEADER
{
    DWORD   dwSize;
    DWORD   dwDataType;    //数据区类型
    DWORD   dwDataLen;     //数据长度
    DWORD   dwCRC;         //数据CRC校验
    DWORD   dwDataPos;     //数据位置
    DWORD   dwVersion;
}UPGRADE_ITEM_HEADER;

//升级程序请求
typedef struct tagDMS_NET_UPGRADE_REQ
{
    DMS_UPGRADE_FILE_HEADER  FileHdr;
    UPGRADE_ITEM_HEADER         stItemHdr[10];
}DMS_NET_UPGRADE_REQ;

//升级程序应答
typedef struct tagDMS_NET_UPGRADE_RESP
{
    DWORD   dwSize;
    int     nResult;        //升级应答结果
    char        reserved[32];
}DMS_NET_UPGRADE_RESP;

//升级程序
typedef struct tagDMS_NET_UPGRADE_DATA
{
    DWORD   dwSize;
    DWORD   dwFileLength;       //升级包总文件长度
    DWORD   dwPackNo;           //分包序号，从0开始
    DWORD   dwPackSize;         //分包大小
    BOOL    bEndFile;           //是否是最后一个分包
    char        reserved[32];
    BYTE    *pData;
}DMS_NET_UPGRADE_DATA;



/*时区和夏时制参数*/
typedef struct
{
    DWORD dwMonth;  //月 0-11表示1-12个月
    DWORD dwWeekNo;  //第几周：0－第1周；1－第2周；2－第3周； 3－第4周；4－最后一周
    DWORD dwWeekDate;  //星期几：0－星期日；1－星期一；2－星期二；3－星期三；4－星期四；5－星期五；6－星期六
    DWORD dwHour; //小时，开始时间取值范围0－23； 结束时间取值范围1－23
    DWORD dwMin; //分0－59
}DMS_NET_TIMEPOINT;





/*异常参数配置*/
typedef struct
{
    DWORD dwSize;
    DMS_HANDLE_EXCEPTION stExceptionHandleType[DMS_MAX_EXCEPTIONNUM];
    /*数组0-盘满,1- 硬盘出错,2-网线断,3-局域网内IP 地址冲突,4-非法访问, 5-输入/输出视频制式不匹配*/
}DMS_NET_EXCEPTION,*LPDMS_NET_EXCEPTION;

typedef struct
{
    DWORD   dwSize;           //本结构大小
    DWORD   dwMask;           //掩码，按位有效
    BYTE        byRes[16];
}DMS_NET_RESTORECFG, *LPDMS_NET_RESTORECFG;

//传感器输出控制
typedef struct tagDMS_NET_ALARMOUT_CONTROL
{
    DWORD   dwSize;
    int     nChannel;
    WORD    wAction;     //0:停止输出 1:开始输出
    WORD    wDuration;   //持续时间以秒为单位，0xFFFFFFFF表示不停止，0表示立即停止
}DMS_NET_ALARMOUT_CONTROL;


/*报警输出*/
typedef struct
{
    DWORD   dwSize; /* 此结构的大小 */
    DWORD   dwChannel;
    char    csAlarmOutName[DMS_NAME_LEN]; /* 名称 */
    DMS_SCHEDTIME stScheduleTime[7][4];/**该通道的videoloss的布防时间*/
}DMS_NET_ALARMOUTCFG, *LPDMS_NET_ALARMOUTCFG;

//传感器状态获取
typedef struct tagDMS_NET_SENSOR_STATE
{
    DWORD   dwSize;
    //0xfffffff标识所有报警输入输出通道，按位有效，
    //从低到高前16位表示报警输入通道号位索引，后16位表示报警输出通道号索引
    DWORD   dwSensorID;
    //0xfffffff标识所有报警输入输出通道状态，按位有效，
    //从低到高前16位表示报警输入通道状态位，后16位表示报警输出通道状态索引
    DWORD   dwSensorOut;
}DMS_NET_SENSOR_STATE;


/*报警输入*/
typedef struct
{
    DWORD   dwSize;                 /* 此结构的大小 */
    DWORD   dwChannel;
    char    csAlarmInName[DMS_NAME_LEN];    /* 名称 */
    BYTE    byAlarmType;                /* 报警器类型（目前版本保留）,0：常开,1：常闭 */
    BYTE    byAlarmInHandle;            /* 是否处理 0：不处理,1：处理*/
    char    reserve[2];
    DMS_HANDLE_EXCEPTION stHandle; /* 处理方式 */
    DMS_SCHEDTIME stScheduleTime[7][4];/**该通道的videoloss的布防时间*/
}DMS_NET_ALARMINCFG, *LPDMS_NET_ALARMINCFG;

typedef struct tagDMS_NET_PRESET_INFO
{
    DWORD 		dwSize;
    unsigned short   nChannel;
    unsigned short   nPresetNum;                   //预置点个数
    unsigned int   no[DMS_MAX_PRESET];        //预置点序号
    char           csName[DMS_MAX_PRESET][64];//预置点名称
}DMS_NET_PRESET_INFO;

typedef struct
{
	BYTE 	byPointIndex;	//巡航组中的下标,如果值大于DMS_MAX_CRUISE_POINT_NUM表示添加到末尾
	BYTE 	byPresetNo;	//预置点编号
	BYTE 	byRemainTime;	//预置点滞留时间
	BYTE 	bySpeed;		//到预置点速度
}DMS_NET_CRUISE_POINT;

typedef struct
{
	BYTE byPointNum; 		//预置点数量
	BYTE byCruiseIndex;	//本巡航的序号
	BYTE byRes[2];
	DMS_NET_CRUISE_POINT struCruisePoint[DMS_MAX_CRUISE_POINT_NUM];
}DMS_NET_CRUISE_INFO;

typedef struct
{
	DWORD dwSize;
	int     nChannel;
	BYTE     byIsCruising;		//是否在巡航
	BYTE     byCruisingIndex;	//正在巡航的巡航编号
	BYTE     byPointIndex;		//正在巡航的预置点序号(数组下标)
	BYTE     byEnableCruise;;	//是否开启巡航
	DMS_NET_CRUISE_INFO struCruise[DMS_MAX_CRUISE_GROUP_NUM];
}DMS_NET_CRUISE_CFG;


//云台控制
typedef struct tagDMS_NET_PTZ_CONTROL
{
    DWORD   dwSize;
    int     nChannel;		//通道号
    DWORD  	dwCommand;		//云台控制命令字DMS_PTZ_CMD_UP
    DWORD 	dwParam;		//云台控制参数(巡航编号)	//速度取值范围[1,64)
    BYTE    byRes[64];		//预置点名称  (设置巡航点的时候为DMS_NET_CRUISE_POINT参数)
}DMS_NET_PTZ_CONTROL;



typedef struct tagDMS_NET_CHANNEL_SHELTER
{
    DWORD   dwSize;
    DWORD   dwChannel;
    BOOL    bEnable;        //是否进行区域遮挡
    DMS_NET_SHELTER_RECT    strcShelter[DMS_MAX_VIDEO_HIDE_RECT];   //遮挡区域，最多支持5块的区域遮挡。RECT以D1为准
}DMS_NET_CHANNEL_SHELTER,*PDMS_NET_CHANNEL_SHELTER;


// 语言种类
typedef enum __LANGUAGE_TYPE_E
{
    DMS_LANGUAGE_CHINESE_SIMPLIFIED = 0,	 //中文
    DMS_LANGUAGE_ENGLISH = 1,		         //英语     
    DMS_LANGUAGE_RUSSIAN = 2,		         //俄国     
    DMS_LANGUAGE_PORTUGUESE = 3,	         //葡萄牙   
    DMS_LANGUAGE_SPANISH = 4,		         //西班牙   
    DMS_LANGUAGE_TURKEY = 5,		         //土耳其   
    DMS_LANGUAGE_ITALIAN = 6,		         //意大利 
    DMS_LANGUAGE_FARSI = 7,		             //波斯     
    DMS_LANGUAGE_FRENCH = 8,		         //法国     
    DMS_LANGUAGE_POLISH = 9,		         //波兰     
    DMS_LANGUAGE_THAI = 10,			         //泰国     
    DMS_LANGUAGE_SERBIAN = 11,		         //塞尔维亚 
    DMS_LANGUAGE_CHINESE_TRADITIONAL = 12,	 //繁体     
    DMS_LANGUAGE_JAPANESE = 13,		         //日本     
    DMS_LANGUAGE_SABRA = 14,		         //希伯莱
    DMS_LANGUAGE_KOREAN = 15,		         //朝鲜     
    DMS_LANGUAGE_VIETNAMESE = 16,	         //越南     
    DMS_LANGUAGE_KAZAKHSTAN = 17,            //哈萨克斯坦      
	DMS_LANGUAGE_GERMAN = 18 ,               //德语
	DMS_LANGUAGE_BUTT,                       //结束
}DMS_LANGUAGE_TYPE_E;


/* DEVICE SYSTEM INFO 设备信息*/
typedef struct tagDMS_NET_DEVICE_INFO {
    DWORD		dwSize;
    char		csDeviceName[DMS_NAME_LEN];	//DVR名称
    DWORD		dwDeviceID;					//DVR ID,用于遥控器

    BYTE		byRecordLen;				//录像文件打包时长,以分钟为单位
    BYTE		byLanguage;					//语言类型,详细见 DMS_LANGUAGE_TYPE
    BYTE		byRecycleRecord;			//是否循环录像,0:不是; 1:是
    BYTE		byOverWritePeriod;			//录像覆盖周期/ 以小时为单位

    BYTE		byVideoStandard;			//视频制式
    BYTE		byDateFormat;				/*日期格式(年月日格式)：
    											0－YYYY-MM-DD 		年月日
    											1－MM-DD-YYYY 		月日年
    											2－YYYY/MM/DD 		年月日
											3－MM/DD/YYYY 		月日年
											4－DD-MM-YYYY 		日月年
											5－DD/MM/YYYY 		日月年

    										*/

    BYTE		byDateSprtr;				//日期分割符(0：":"，1："-"，2："/"). 2011-01-21 yxhu: 无效字段
    BYTE		byTimeFmt;					//时间格式 (0-24小时，1－12小时).

    BYTE		byConfigWizard;				//是否启用了开机向导
    BYTE		byReserve[3];

    //以下不能修改
    char		bySoftwareVersion[32];          //软件版本号，高16位是主版本，低16位是次版本
    char		bySoftwareBuildDate[32];        //软件生成日期，0xYYYYMMDD
    char		byDspSoftwareVersion[32];
    char		byDspSoftwareBuildDate[32];
    char		byPanelVersion[32];
    char		byPanelSoftwareBuildDate[32];
    char		byHardwareVersion[32];
    char		byHardwareBuildDate[32];
    char		byWebVersion[32];
    char		byWebBuildDate[32];

    char		csSerialNumber[DMS_SERIALNO_LEN];//序列号
    DWORD		dwServerCPUType;			//当前CPU类型(DMS_CPU_TYPE_E)
    DWORD		dwSysFlags;					//系统支持的功能
    DWORD		dwServerType;				//设备类型(DMS_SERVER_TYPE_E)

    BYTE		byVideoInNum;				/* 最大支持的Video Channel个数 */
    BYTE		byAudioInNum;				/* 最大支持的Audio Channel个数 */
    BYTE		byAlarmInNum;				//DVR报警输入个数
    BYTE		byAlarmOutNum;				//DVR报警输出个数

    BYTE		byDiskNum;					//DVR 硬盘个数
    BYTE		byRS232Num;					//DVR 232串口个数
    BYTE		byRS485Num;					//DVR 485串口个数
    BYTE		byNetworkPortNum;			//网络口个数

    BYTE		byDecordChans;				//DVR 解码路数
    BYTE		byVGANum;					//VGA口的个数
    BYTE		byUSBNum;					//USB口的个数
    BYTE		byDiskCtrlNum;				//硬盘控制器个数

    BYTE		byAuxOutNum;				//辅口的个数
    BYTE		byStreamNum;				//每路视频最大可以支持视频流数
	BYTE		byIPChanNum;				//数字通道数
    BYTE		byResvered;
} DMS_NET_DEVICE_INFO;


typedef struct tagDMS_NET_WANWIRELESS_CONFIG_V2
{
	DWORD       dwSize;
	BOOL        Enable3G;
	BYTE        byCardNum3G; 			//3G卡号
	BYTE        bySeviceProvider3G;		//3G运营商:自定义和移动,联通,电信
	BYTE        byStatus;				//0:断开连接, 1:拨号成功, 2:正在拨号
	BYTE        bySigs;					//信号强度
	char	    csApn3G[32];			//3G APN:  自定义和cmnet\3gnet\ctnet
	char       	csUsrName3G[32];		//3G用户名
	char		csUsrPSWD3G[32];		//3G用户密码
	char		csPhoneNumber3G[18];	//3G电话号码
	BYTE        byReserve2[14];
	DMS_NET_IPADDR stNetIpaddr;			//3G的IP
}DMS_NET_WANWIRELESS_CONFIG_V2, *LPDMS_NET_WANWIRELESS_CONFIG_V2;



typedef struct tagDMS_NET_CHANNEL_PIC_INFO
{
    DWORD   dwSize;
    DWORD   dwChannel;
    char    csChannelName[DMS_NAME_LEN];    //通道名称

    DMS_NET_COMPRESSION_INFO   stRecordPara; /* 录像 */
    DMS_NET_COMPRESSION_INFO   stNetPara;    /* 网传 */
    DMS_NET_COMPRESSION_INFO   stPhonePara;  /* 手机监看 */
    DMS_NET_COMPRESSION_INFO   stEventRecordPara; /*事件触发录像压缩参数*/
}DMS_NET_CHANNEL_PIC_INFO, *LPDMS_NET_CHANNEL_PIC_INFO;


//Color DMS_RANGE
typedef struct tagDMS_NET_COLOR_SUPPORT
{
    DWORD           dwSize;                     //struct size
    DWORD           dwMask;              //按位有效(DMS_COLOR_SET_BRIGHTNESS)

    DMS_RANGE       strBrightness;
    DMS_RANGE       strHue;
    DMS_RANGE       strSaturation;
    DMS_RANGE       strContrast;
    DMS_RANGE       strDefinition;
    BYTE                byReseved[32];
}DMS_NET_COLOR_SUPPORT;

typedef struct tagDMS_NET_ENHANCED_COLOR_SUPPORT
{
    DWORD           dwSize;                     //struct size
    DWORD           dwMask;              //按位有效(DMS_COLOR_SET_BRIGHTNESS)

    DMS_RANGE       strDenoise; 		//降噪
    DMS_RANGE       strIrisBasic;		//光圈基准值
    DMS_RANGE       strRed;
    DMS_RANGE       strBlue;
    DMS_RANGE       strGreen;
    DMS_RANGE       strGamma;

    DMS_RANGE       strEC;		//曝光阀值
    DMS_RANGE       strGC;		//增益阀值
    DMS_RANGE       strWD;		//宽动态档次
    BYTE                byReseved[32];
}DMS_NET_ENHANCED_COLOR_SUPPORT;

//通道颜色设置
typedef struct tagDMS_NET_CHANNEL_ENHANCED_COLOR
{
    DWORD   dwSize;
    DWORD   dwChannel;
    DWORD   dwSetFlag;              //0,设置但不保存;1,保存参数;2,恢复上一次保存的

    BOOL    bEnableAutoDenoise;     //自动降噪使能
    int     nDenoise;               //降噪阀值

    BOOL    bEnableAWB;             //自动白平衡
    int     nRed;
    int     nBlue;
    int     nGreen;

    BOOL    bEnableAEC;             //自动曝光(电子快门)使能
    int     nEC;                    //自动曝光(电子快门)阀值

    BOOL    bEnableAGC;             //自动增益/手动增益
    int     GC;                     //自动增益/手动增益阀值

    int     nMirror;                //0:不镜像, 1: 上下镜像, 2: 左右镜像  3: 上下左右镜像
    BOOL    bEnableBAW;             //手动彩转黑使能0,自动1,夜视,2 白天

    int     nIrisBasic;             //锐度
    int     nGamma;
    BOOL    bWideDynamic;           //宽动态使能
    int     nWDLevel;               //宽动态等级

    int     nSceneMode;             //场景模式
                                    //0x00    /*scene，outdoor*/
                                    //0x01    /*scene，indoor*/
                                    //0x02    /*scene，manual*/
                                    //0x03    /*scene，auto*/
    BOOL    bEnableAIris;           //自动光圈
    BOOL    bEnableBLC;             //背光控制使能
    BYTE    byReseved[28];
}DMS_NET_CHANNEL_ENHANCED_COLOR, *PDMS_NET_CHANNEL_ENHANCED_COLOR;


//Save Flag
typedef enum emSaveFlag
{
	DMS_COLOR_NO_SAVE = 0,
	DMS_COLOR_SAVE = 1,
	DMS_COLOR_SET_DEF = 2,
} eSaveFlag;

typedef struct tagDMS_NET_CHANNEL_COLOR_SINGLE
{
    DWORD       dwSize;                     //struct size
    DWORD           dwChannel;
    DWORD       dwSetFlag;
    DWORD       dwSaveFlag;
    int             nValue;
    int             nParam;
}DMS_NET_CHANNEL_COLOR_SINGLE;

//通道颜色设置
typedef struct tagDMS_NET_CHANNEL_COLOR
{
    DWORD   dwSize;
    DWORD   dwChannel;
    DWORD   dwSetFlag;      //0,设置但不保存;1,保存参数;2,恢复上一次保存的
    int     nHue;           //色调 0-255
    int     nSaturation;        //饱和度 0-255
    int     nContrast;      //对比度 0-255
    int     nBrightness;    //亮度 0-255
    int     nDefinition;    //清晰度 0-255
}DMS_NET_CHANNEL_COLOR, *PDMS_NET_CHANNEL_COLOR;


typedef struct tagDMS_NET_DAY_NIGHT_DETECTION_EX
{
    DWORD dwSize;
    DWORD dwChannel;
    BYTE byMode;      // 0：自动检测，1：光敏电阻检测，2：视频检测，3：时间检测
    //光敏电阻检测
    BYTE byTrigger;    // 光敏电阻检测-0：低电平有效，1：高电平有效
    //视频检测
    BYTE byAGCSensitivity;// 灵敏度,0-5
    BYTE byDelay;      // 切换延时时间0-10s。

    BYTE byIRCutLevel;// IRCUT控制-0：低电平有效，1：高电平有效
    BYTE byLedLevel; // 红外灯控制: 0:低电平 1:高电平
    BYTE reserve[2];
    //时间检测
    DMS_SCHEDTIME stColorTime; //彩色的时间段，不在此时间段内认为是黑白。
}DMS_NET_DAY_NIGHT_DETECTION_EX;

/**UPNP**/
typedef struct tagDMS_UPNP_CONFIG
{
    DWORD       dwSize;
    BOOL        bEnable;                /*是否启用upnp*/
    DWORD       dwMode;                 /*upnp工作方式.0为自动端口映射，1为指定端口映射*/
    DWORD       dwLineMode;             /*upnp网卡工作方式.0为有线网卡,1为无线网卡*/
    char            csServerIp[32];         /*upnp映射主机.即对外路由器IP*/
    DWORD       dwDataPort;             /*upnp映射数据端口*/
    DWORD       dwWebPort;              /*upnp映射网络端口*/
    DWORD       dwMobilePort;           /*upnp映射手机端口*/

    WORD        dwDataPort1;            /*upnp已映射成功的数据端口*/
    WORD        dwWebPort1;             /*upnp已映射成功的网络端口*/
    WORD        dwMobilePort1;          /*upnp映射成功的手机端口*/
    WORD            wDataPortOK;
    WORD            wWebPortOK;
    WORD            wMobilePortOK;
}DMS_UPNP_CONFIG, *LPDMS_UPNP_CONFIG;

/* ftp上传参数*/
typedef struct tagDMS_NET_FTP_PARAM
{
    DWORD   dwSize;
    BOOL    bEnableFTP;                             /* 是否启动ftp上传功能*/
    char    csFTPIpAddress[DMS_MAX_DOMAIN_NAME];    /* ftp 服务器，可以是IP地址或域名*/
    DWORD   dwFTPPort;                              /* ftp 端口*/
    char    csUserName[DMS_NAME_LEN];               /* 用户名*/
    char    csPassword[DMS_PASSWD_LEN];             /* 密码      */
    WORD    wTopDirMode;                /* 0x0 = 使用设备ip地址, 0x1 = 使用设备名,0x2 = OFF*/
    WORD    wSubDirMode;                /* 0x0 = 使用通道号 ,        0x1 = 使用通道名,0x2 = OFF*/
    BYTE    byTestStatus;			    /* 0: 连接成功  1：连接失败    BYTE    reservedData[27];  */
    BYTE    reservedData[27];           /* 保留 */
}DMS_NET_FTP_PARAM, *LPDMS_NET_FTP_PARAM;

typedef struct tagDMS_NET_CHANNEL_OSDINFO
{
    DWORD   dwSize;
    DWORD   dwChannel;
    BOOL    bShowTime;
    BOOL    bDispWeek; /*是否显示星期*/
    DWORD   dwTimeTopLeftX; /*OSD左上角的x坐标*/
    DWORD   dwTimeTopLeftY;/*OSD左上角的y坐标*/
    BYTE    byReserve1;// 2011-01-21 yxhu: 无效字段，直接参照tagDMS_NET_DEVICE_INFO 结构体
                 /*OSD类型(年月日格式)：
                */
    BYTE       byOSDAttrib;
                /*
                OSD属性（透明/闪烁）：
                1－透明，闪烁
                2－透明，不闪烁
                3－闪烁，不透明
                4－不透明，不闪烁
                */
    BYTE    byReserve2;  // 2011-01-21 yxhu: 无效字段，直接参照tagDMS_NET_DEVICE_INFO 结构体

    BYTE    byShowChanName ;    //预览的图象上是否显示通道名称：0-不显示，1-显示（区域大小704*576）
    DWORD   dwShowNameTopLeftX ;    //通道名称显示位置的x坐标
    DWORD   dwShowNameTopLeftY ;    //通道名称显示位置的y坐标
    char    csChannelName[DMS_NAME_LEN];    //通道名称
    DMS_NET_OSD_S stOsd[DMS_MAX_OSD_NUM];/**源通道编码OSD 2011-01-21 yxhu: 字段预留*/
}DMS_NET_CHANNEL_OSDINFO, *LPDMS_NET_CHANNEL_OSDINFO;


/**手机平台注册参数**/
typedef struct tagDMS_MOBILE_CENTER_INFO
{
    DWORD   dwSize;
    BOOL    bEnable;
    char        szIp[64];                   //服务器Ip
    DWORD   dwCenterPort;               //中心端口
    char        csServerNo[64];             //服务器序列号
    DWORD   dwStatus;                   //服务器连接状态 0为未连接 1为连接中 2连接成功
    char    csUserName[32];             //用户名
    char    csPassWord[32];             //密码
    BYTE    byAlarmPush;
    BYTE    reservedData[3];
    DWORD   reserve1[1];            //保留
}DMS_MOBILE_CENTER_INFO, *LPDMS_MOBILE_CENTER_INFO;

//RTSP设置
typedef struct tagDMS_NET_RTSP_CFG
{
    DWORD       dwSize;
    DWORD       dwPort;             //RTSP端口
    BYTE        byReserve[32];
}DMS_NET_RTSP_CFG,*PDMS_NET_RTSP_CFG;

typedef struct tagDMS_NET_NETWORK_CFG
{
    DWORD       dwSize;
    DMS_NET_ETHERNET    stEtherNet[DMS_MAX_ETHERNET];

    DMS_NET_IPADDR    stMulticastIpAddr;
    DMS_NET_IPADDR    stGatewayIpAddr;
    DMS_NET_IPADDR    stManagerIpAddr;    //远程管理主机地址(服务器告警时，自动连接的对讲IP)
    DMS_NET_IPADDR    stDnsServer1IpAddr;
    DMS_NET_IPADDR    stDnsServer2IpAddr;

    BYTE        byEnableDHCP;         //
    BYTE        byMaxConnect;         //网络最大连接数
    BYTE        byReserve[2];         //保留
    WORD            wHttpPort;            //Http端口
    WORD        wReserve1;            //yxhu 2013-03-07 预留
    WORD            wManagerPort;         //远程管理主机端口
    WORD            wMulticastPort;       //多播端口
    //BYTE        byAutoDHCPDNS;      //yxhu 废弃，dns和ip地址共用byEnableDHCP
    DWORD       dwReserve2;
}DMS_NET_NETWORK_CFG, *PDMS_NET_NETWORK_CFG;

typedef struct
{
    DWORD dwSize;
    BOOL  bPPPoEEnable;              //0-不启用,1-启用
    char  csPPPoEUser[DMS_NAME_LEN]; //PPPoE用户名
    char  csPPPoEPassword[DMS_PASSWD_LEN]; //PPPoE密码
    DMS_NET_IPADDR  stPPPoEIP; //PPPoE IP地址(只读)
    DWORD dwSecurityProtocol; /**< 加密协议 值范围参考:HI_CFG_PPPOE_SECURITY_PROTOCOL_E*/
}DMS_NET_PPPOECFG, *LPDMS_NET_PPPOECFG;

typedef struct tagDMS_NET_P2P_CFG
{
	DWORD dwSize;
	DWORD dwEnable;
	char  	  csID[64];
	BYTE 	  byStatus;
	//char      csRes[63];
	BYTE	u8p2ptype;	//0:tutk  1:langtao
	char      u8mac[20];
	char      csRes[42];
}DMS_NET_P2P_CFG;

typedef struct tagDMS_NET_FAC_CFG
{
    DWORD dwSize;
    char  language; //0:Chinese; 1:English
    char  csRes[128];
}DMS_NET_FAC_CFG;


//设备自动维护设置
typedef struct tagDMS_NET_DEVICEMAINTAIN
{
	DWORD dwSize;
	BYTE byEnable;
	BYTE byIndex;
	SYSTEMTIME stDeviceMaintain;
}DMS_NET_DEVICEMAINTAIN;

typedef struct tagDMS_NET_LOG_INFO
{
    DMS_TIME   stSysStartTime;
    DMS_TIME   stSysRunTime;
    char       szReserve[16];
}DMS_NET_LOG_INFO;


typedef struct tagDMS_NET_RECDAYINMONTH
{
	DWORD dwSize;
	DWORD nChannel;
	unsigned char	year;//查询年,0~255取值范围，代表2000~2255年
	unsigned char	month;//查询月，1~12为取值范围
	unsigned char	type;//文件类型 ：0xff － 全部，0 － 定时录像，1 - 移动侦测，2 － 报警触发，3  － 手动录像
	LONG lCalendarMap;
}DMS_NET_RECDAYINMONTH, *LPDMS_NET_RECDAYINMONTH;


typedef struct tagDMS_NET_EMAIL_PARAM
{
    DWORD	dwSize;

    BOOL	bEnableEmail;                   //是否启用

    BYTE	byAttachPicture;                //是否带附件
    BYTE	bySmtpServerVerify;             //发送服务器要求身份验证
    BYTE	byMailInterval;                 //最少2s钟(1-2秒；2-3秒；3-4秒；4-5秒)
    BYTE	byTestStatus;                   //0: 发送成功  1：发送失败

    WORD	wServicePort;					/**< 服务器端口,一般为25，用户根据具体服务器设置 */
    WORD	wEncryptionType;				/**< 加密类型 ssl*/

    char	csEMailUser[DMS_NAME_LEN];      //账号
    char	csEmailPass[DMS_PASSWD_LEN];    //密码

    char    csSmtpServer[DMS_MAX_DOMAIN_NAME]; //smtp服务器 //用于发送邮件
    char    csPop3Server[DMS_MAX_DOMAIN_NAME]; //pop3服务器 //用于接收邮件,和IMAP性质类似

    DMS_NET_EMAIL_ADDR stToAddrList[DMS_EMAIL_ADDR_MAXNUM]; /**< 收件人地址列表  */
    DMS_NET_EMAIL_ADDR stCcAddrList[DMS_EMAIL_ADDR_MAXNUM]; /**< 抄送地址列表 */
    DMS_NET_EMAIL_ADDR stBccAddrList[DMS_EMAIL_ADDR_MAXNUM];/**< 密送地址列表 */
    DMS_NET_EMAIL_ADDR stSendAddrList;                          /**< 发送人地址 */

	BYTE    byEmailType[1];					//复用为发送邮件类型 8 - Motion 4 - Video Loss  2 - IOAlarm  1- Other
	char    csReserved[31];
}DMS_NET_EMAIL_PARAM, *PDMS_NET_EMAIL_PARAM;


/*
录像功能逻辑

1. 录像模式分为: 计划录像/开机录像/手工录像/不录像
	由DMS_NET_CHANNEL_RECORD 结构体中的byRecordMode 定义。
	当该值设置成不录像时，该通道将不接收任何录像指令，包括报警和手工录像。
2. 定时录像逻辑
	计划录像: 根据stRecordSched 时间表录像计划进行录像，
	条件1: 录像模式byRecordMode 是定时录像模式，录像时段中判断当前时间点的录像类型(byRecordType) 字段值按位与0x000001 为真
3. 移动侦测录像逻辑
	条件1: 视频通道触发移动，并且配置联动通道录像
	条件2: a. 录像模式byRecordMode 是定时录像模式，录像时段中判断当前时间点的录像类型byRecordType 字段值按位与0x000002 为真
			或者b. 录像模式byRecordMode 是开机录像模式
4. 探头报警录像逻辑
	条件1: 探头触发报警，并且配置联动视频通道录像
	条件2: a.录像模式byRecordMode 是定时录像模式，录像时段中判断当前时间点的录像类型byRecordType 字段值按位与0x000004 为真
				或者b. 录像模式byRecordMode 是开机录像模式
*/

typedef struct
{
    DWORD			dwSize;    				/* 此结构的大小 */
    DWORD			dwChannel;
    DWORD			dwReserve;  			/* 2014-12-23 yxhu: 直接用byRecordMode， 该字段停止使用，保留字段*/
    DMS_RECORDSCHED stRecordSched;			/* 录像时间段，星期一到星期天*/
    DWORD			dwPreRecordTime;		/* 预录时间，单位是s，0表示不预录*/
    DWORD			dwRecorderDuration; 	/* 录像保存的最长时间, 2014-12-25 yxhu: IPC 版本不支持*/
    BYTE				byRedundancyRec;		/* 是否冗余录像,重要数据双备份：0/1, 默认为不启用2014-12-25 yxhu: 目前版本不支持*/
    BYTE				byAudioRec;				/* 录像时复合流编码时是否记录音频数据：国外有此法规,目前版本不支持*/
    BYTE				byRecordMode;			//0:自动模式（按定时录像配置）1:开机录像 2:手动录像模式3:禁止所有方式触发录像
    BYTE  				byStreamType;			// 录像码流类型 0:主码流 1:子码流
    BYTE  				byReserve[8];
}DMS_NET_CHANNEL_RECORD, *LPDMS_NET_CHANNEL_RECORD;

typedef struct tagDMS_NET_RECORD_STREAMMODE
{
    DWORD         dwSize;
    DWORD         dwStreamType;//0:first stream,1:second stream,2,third stream
    unsigned char byRes[16];
}DMS_NET_RECORD_STREAMMODE;

// SNMP
typedef struct tagDMS_SNMP_CFG{
	DWORD	dwSize;
	BYTE	byEnable;				//是否有效
	BYTE	nSnmpVersion;			//v1=0 v2=1 v3=2
	int  nSnmpPort;					//1-65535
	char szROCommunity[32];
	char szRWCommunity[32];
	char szTrapAddr[16];
	int  nTrapPort;					//1-65535
	char szUserName[32];			//v3 using
	char szPassword[16];
}DMS_SNMP_CFG,*PDMS_SNMP_CFG;


typedef struct
{
    DWORD			dwSize;				//本结构大小
    char			csNTPServer[64];	//NTP服务器域名或者IP地址
    WORD			wInterval;			//校时间隔时间（以小时为单位）
    BYTE			byEnableNTP;		//NTP校时是否启用：0－否，1－是
    signed char		cTimeDifferenceH;	//与国际标准时间的时差（小时），-12 ... +13
    signed char		cTimeDifferenceM;	//与国际标准时间的时差（分钟），0, 30, 45
    BYTE			res1;
    WORD			wNtpPort;			//NTP服务器端口，设备默认为123
    BYTE			res2[8];
}DMS_NET_NTP_CFG,*LDMS_NET_NTP_CFG;

typedef struct
{
    DWORD				dwSize;			//本结构大小
    int					nTimeZone;		//时区
    BYTE                byDstMode;		//夏令时模式，0-默认，1-自定义
    BYTE                byStartDst;     //是否已经开始执行Dst;
    BYTE				byRes1[10];		//保留
    DWORD				dwEnableDST;	//是否启用夏时制 0－不启用 1－启用
    BYTE				byDSTBias;		//夏令时偏移值，30min, 60min, 90min, 120min, 以分钟计，传递原始数值
    BYTE				byRes2[3];		//保留
    DMS_NET_TIMEPOINT	stBeginPoint;	//夏时制开始时间
    DMS_NET_TIMEPOINT	stEndPoint;		//夏时制停止时间
}DMS_NET_ZONEANDDST, *LPDMS_NET_ZONEANDDST;

/* 定时抓图 */
typedef struct tagDMS_NET_SNAP_TIMER
{
    DWORD       	dwSize;
    BOOL                bEnable;
    DMS_SCHEDTIME stScheduleTime[7][4];/**该通道的videoloss的布防时间*/
    DWORD          	dwInterval;      //单位：毫秒
    int                 	nPictureQuality;//编码质量(0-4),0为最好
    int                 	nImageSize;     // 画面大小；有效值必须是主码流支持的分辨率
    BYTE			byStoragerMode; //存储模式(0:本地, 1:FTP, 2: FTP|LOCAL(优先FTP,FTP失败后录像本地))
    BYTE			byRes1;
    WORD			wChannel;	//0813
    BYTE        		byRes[4];
}DMS_NET_SNAP_TIMER, *LPDMS_NET_SNAP_TIMER;

typedef struct tagDMS_NET_SNAP_EVENT
{
    DWORD       	dwSize;
    BOOL                bEnable;
    DWORD          	dwInterval;           //抓拍间隔，单位：毫秒
    int                 	nPictureQuality;   //编码质量(0-4),0为最好
    int                 	nImageSize;         // 画面大小；有效值必须是主码流支持的分辨率
    BYTE                 byPicNum;           //抓拍数量
    BYTE        		byRes[3];
}DMS_NET_SNAP_EVENT, *LPDMS_NET_SNAP_EVENT;

/*232串口参数*/
typedef struct
{
    DWORD dwSize; /* 此结构的大小 */
    DWORD dwBaudRate; /* 波特率(bps) */
    BYTE byDataBit; /* 数据有几位 0－5位，1－6位，2－7位，3－8位; */
    BYTE byStopBit; /* 停止位 0－1位，1－2位; */
    BYTE byParity; /* 校验 0－无校验，1－奇校验，2－偶校验; */
    BYTE byFlowcontrol; /* 0－无，1－软流控,2-硬流控 */
    DWORD dwWorkMode; /* 工作模式，0－窄带传输（232串口用于PPP拨号），1－控制台（232串口用于参数控制），2－透明通道 */
}DMS_NET_RS232CFG, *LPDMS_NET_RS232CFG;


typedef struct
{
    DWORD   dwSize;
    int             bEnable;              /*0:无效用户，禁用, 1:启用*/
    DWORD   dwIndex;          /*用户编号*/

    char        csUserName[DMS_NAME_LEN]; /* 用户名最大32字节*/
    char        csPassword[DMS_PASSWD_LEN]; /* 密码 */

    DWORD   dwUserRight; /* 权限 */

    BYTE  byLocalPreviewRight[DMS_MAX_CHANNUM]; //本地可以预览的通道 1-有权限，0-无权限

    BYTE  byNetPreviewRight[DMS_MAX_CHANNUM]; //远程可以预览的通道 1-有权限，0-无权限

    BYTE  byLocalPlaybackRight[DMS_MAX_CHANNUM]; //本地可以回放的通道 1-有权限，0-无权限

    BYTE  byNetPlaybackRight[DMS_MAX_CHANNUM]; //远程可以回放的通道 1-有权限，0-无权限

    BYTE  byLocalRecordRight[DMS_MAX_CHANNUM]; //本地可以录像的通道 1-有权限，0-无权限

    BYTE  byNetRecordRight[DMS_MAX_CHANNUM];//远程可以录像的通道 1-有权限，0-无权限

    BYTE  byLocalPTZRight[DMS_MAX_CHANNUM];//本地可以PTZ的通道 1-有权限，0-无权限

    BYTE  byNetPTZRight[DMS_MAX_CHANNUM];//远程可以PTZ的通道 1-有权限，0-无权限

    BYTE  byLocalBackupRight[DMS_MAX_CHANNUM];//本地备份权限通道 1-有权限，0-无权限

    BYTE  byNetBackupRight[DMS_MAX_CHANNUM];//远程备份权限通道 1-有权限，0-无权限

    DMS_NET_IPADDR   stIPAddr;
    BYTE    byMACAddr[DMS_MACADDR_LEN]; /* 物理地址 */
    BYTE    byPriority;                                 /* 优先级，0xff-无，0--低，1--中，2--高，3--最高 */
                                                                    /*
                                                                    无……表示不支持优先级的设置
                                                                    低……默认权限:包括本地和远程回放,本地和远程查看日志和状态,本地和远程关机/重启
                                                                    中……包括本地和远程控制云台,本地和远程手动录像,本地和远程回放,语音对讲和远程预览
                                                                                    本地备份,本地/远程关机/重启
                                                                    高……可以执行除了为 Administrators 组保留的任务外的其他任何操作系统任务
                                                                    最高……管理员
                                                                    */

    BYTE    byRes[17];
}DMS_NET_USER_INFO,*LPDMS_NET_USER_INFO;

typedef struct tagDMS_NET_MOTION_DETECT
{
    DWORD           dwSize;
    DWORD           dwChannel;
    BOOL            bEnable;                //是否进行布防
    DWORD           dwSensitive;            //灵敏度 取值0 - 100, 越小越灵敏*/
    BOOL                bManualDefence;     //(yxhu 20130203 废弃) 手动布防标志，==YES(1)启动，==NO(0)按定时判断
    BYTE            byMotionArea[DMS_MD_STRIDE_SIZE];   //布防区域块组.布防区域共有44*36个块,数据以行BIT 为单位的内存宽度。
    DMS_HANDLE_EXCEPTION   stHandle;
    DMS_SCHEDTIME stScheduleTime[7][4];/**该通道的videoloss的布防时间*/
}DMS_NET_CHANNEL_MOTION_DETECT, *LPDMS_NET_CHANNEL_MOTION_DETECT;

//信号丢失报警
typedef struct{
    DWORD    dwSize;
    DWORD   dwChannel;
    BOOL     bEnable; /* 是否处理信号丢失报警 */
    DMS_HANDLE_EXCEPTION   stHandle; /* 处理方式 */
    DMS_SCHEDTIME stScheduleTime[7][4];/**该通道的videoloss的布防时间*/
}DMS_NET_CHANNEL_VILOST,*LPDMS_NET_CHANNEL_VILOST;

//遮挡报警区域大小704*576
typedef struct
{
    DWORD dwSize;
    DWORD   dwChannel;
    BOOL bEnable; /* 是否启动遮挡报警 ,0-否,1-低灵敏度 2-中灵敏度 3-高灵敏度*/
    WORD wHideAlarmAreaTopLeftX; /* 遮挡区域的x坐标 */
    WORD wHideAlarmAreaTopLeftY; /* 遮挡区域的y坐标 */
    WORD wHideAlarmAreaWidth; /* 遮挡区域的宽 */
    WORD wHideAlarmAreaHeight; /*遮挡区域的高*/
    DMS_HANDLE_EXCEPTION stHandle; /* 处理方式 */
    DMS_SCHEDTIME stScheduleTime[7][4];/**该通道的videoloss的布防时间*/
}DMS_NET_CHANNEL_HIDEALARM,*LPDMS_NET_CHANNEL_HIDEALARM;

typedef struct tagDMS_NET_WIFI_CONFIG
{
    DWORD           dwSize;
    BYTE            bWifiEnable;
    BYTE			byWifiMode;         // 0:station, 1:ap
    BYTE			byWpsStatus;        // wps状态(0:关闭,1:启用)
	BYTE			byConnectStatus;    // 0: ether connect,  1: wifi connect.
    DMS_NET_IPADDR  dwNetIpAddr;
    DMS_NET_IPADDR  dwNetMask;
    DMS_NET_IPADDR  dwGateway;
    DMS_NET_IPADDR  dwDNSServer;
    char            csEssid[32];
    BYTE            nSecurity;
						// 0: none  (wifi enable)
						// 1: wep 64 assii (wifi enable)
						// 2: wep 64 hex (wifi enable)
						// 3: wep 128 assii (wifi enable)
						// 4: wep 128 hex (wifi enable)
						// 5: WPAPSK-TKIP
						// 6: WPAPSK-AES
						// 7: WPA2PSK-TKIP
						// 8: WPA2PSK-AES
    BYTE        	byNetworkType;  // 1. Infra 2. ad-hoc
    BYTE            byEnableDHCP;
    BYTE            byStatus; //    0:  成功,其他值是错误码
    char            csWebKey[32];
}DMS_NET_WIFI_CONFIG, *LPDMS_NET_WIFI_CONFIG;

typedef struct tagDMS_NET_DDNSCFG
{
    DWORD   dwSize;
    BOOL    bEnableDDNS;                //是否启用DDNS
    struct {
        DMS_DDNS_TYPE    enDDNSType;                  //DDNS服务器类型, 域名解析类型：DMS_DDNS_TYPE
        char    csDDNSUsername[DMS_NAME_LEN];
        char    csDDNSPassword[DMS_PASSWD_LEN];
        char    csDDNSDomain[DMS_MAX_DOMAIN_NAME];          //在DDNS服务器注册的域名地址
        char    csDDNSAddress[DMS_MAX_DOMAIN_NAME];         //DNS服务器地址，可以是IP地址或域名
        int nDDNSPort;                  //DNS服务器端口，默认为6500
        int nUpdateTime;              /*更新时间*/
        int     nStatus;                        /*状态,0:成功,其他值代表错误号*/

        BYTE byReserve[32];               /**< 保留字,按讨论结果预留32字节 */
    }stDDNS[DMS_MAX_DDNS_NUMS];
}DMS_NET_DDNSCFG, *PDMS_NET_DDNSCFG;

typedef struct tagDMS_NET_SNAP_DATA_V2
{
    DWORD   dwSize;
    int     nChannel;
    int     nLevel;       // 0-5 :0 最高
    DWORD   dwFileType;   // 0:jpg图片，1:bmp图片
    DWORD   dwBufSize;    // 数据长度
    char   *pDataBuffer;  //数据
    int     nWidth;
    int     nHeight;
}DMS_NET_SNAP_DATA_V2;

///////////////////////////////////////////////////////////
///////////////////// 增加部分2 ///////////////////////////
typedef enum __RESTORE_MASK_E
{
    NORMAL_CFG = 0x00000001,       //设备基本参数
    VENCODER_CFG = 0x00000002,     //视频编码参数
    RECORD_CFG = 0x00000004,           //录像参数
    RS232_CFG = 0x00000008,            //RS232参数
    NETWORK_CFG = 0x00000010,          //网络参数
    ALARM_CFG = 0x00000020,          //报警输入/输出参数
    VALARM_CFG = 0x00000040,           //视频报警检测参数
    PTZ_CFG = 0x00000080,              //云台参数
    PREVIEW_CFG = 0x00000100,          //本地输出参数
    USER_CFG = 0x00000200,            //用户参数
    TVMODE_CFG = 0x00000400,		  //制式
    COLOR_CFG = 0x00000800, 		  //参数颜色
} RESTORE_MASK_E;

typedef enum emGKSysFlag
{
    DMS_SYS_FLAG_ENCODE_D1 =         0x00000001,
    DMS_SYS_FLAG_HD_RECORD =         0x00000002,
    #ifdef MODULE_SUPPORT_RF433
	DMS_SYS_FLAG_RF433_ALARM =		0x00000003,
	#endif
    DMS_SYS_FLAG_CI_IDENTIFY =        0x00000004,
    DMS_SYS_FLAG_MD_RECORD	=		0x00000008,	//磁盘录像
    DMS_SYS_FLAG_PPPOE_DDNS =        0x00000010,
    DMS_SYS_FLAG_DECODE_H264 =       0x00000020,
    DMS_SYS_FLAG_ITEV =			0x00000040,  //增加智能分析
    DMS_SYS_FLAG_VIDEO_ALARM_MODE2 =  0x00010000,
    DMS_SYS_FLAG_FTPUP =              0x00020000,  // FTPUP
    DMS_SYS_FLAG_EMAIL =              0x00040000,	// EMAIL
    DMS_SYS_FLAG_WIFI =               0x00080000, /* WLAN */
    DMS_SYS_FLAG_ACTIVE_CONNECTION =  0x00100000,  //支持主动连接
    DMS_SYS_FLAG_TELALARM =           0x00200000,  //支持电话报警
    DMS_SYS_FLAG_TEMPHUM =            0x00400000,  //支持温湿度，电源
    DMS_SYS_FLAG_UPNP =               0x00800000,	// upnp
    DMS_SYS_FLAG_PLATFORM =           0x01000000,	/* 支持平台功能*/
    DMS_SYS_FLAG_3G =				  0x02000000, /* 3G */
    DMS_SYS_FLAG_PTZ =				  0x04000000,	//PTZ
    DMS_SYS_FLAG_DEVICEMAINTAIN =     0x08000000,  //设备维护使能
    DMS_SYS_FLAG_NETWORK =			  0x20000000,	//有线网线
    DMS_SYS_FLAG_SNMP =   			  0x40000000,	//SNMP
    DMS_SYS_FLAG_EXCEPTALARM =		  0x80000000,	//异常报警配置使能
} eSysFlag;

typedef enum __DMS_SERVER_TYPE_E
{
    //初版本
    NVS_FLAG=100001	,  // 初版本DVS
    NVD_FLAG=100002, //  初版本NVD
    NVR_FLAG=100003, //  初版本NVR
    IPC_FLAG=100004, //  初版本IPCAM
    DVR_D3_0001_FLAG=0x10D30001, // D3初版设备
    DVR_D2_0001_FLAG=0x10D30002, // D3初版设备
    DMS_HVR_FLAG=0x10D30003,//HVR
    //IPC
    IPC_6C_D	=0xA1010002,
    IPC_6C_P	=0xA1010003,
    IPC_6D_M	=0xA1020004,
    IPC_6E_M	=0xA1030004,
    IPC_6E_3M	=0xA1030005,
    IPC_6F_P	=0xB1000003,
    IPC_6F_10P	=0xA1080003,	//3518 720P
    IPC_6F_13P	=0xA1080006,	//3518 960P
    IPC_6F_20P	=0xA1080004,	//3518 1080P
    IPC_6F_30P	=0xA1080005,	//3518 300M
    IPC_6H_P	=0xC1080003,

    //DVS
    DVS_S6C00D=0xA2010002,
    DVS_S6D00D=0xA2020002,
    DVS_S6D00C=0xA2020001,
    DVS_S6D00D_S=0xA2020002,
    //NVD
    NVD_S7D00D=0xA5050002,
    //DVR
    DVR_DA3700FC=0xA3060001,
    DVR_DA3700FD=0xA3070002,
}DMS_SERVER_TYPE_E;

typedef enum {
    DMS_NTSC = 0,
    DMS_PAL = 1,
} DMS_FORMAT;

typedef enum {
    DMS_VIDEO_FORMAT_CIF = 0,
    DMS_VIDEO_FORMAT_D1 = 1,
    DMS_VIDEO_FORMAT_HD1 = 2,
    DMS_VIDEO_FORMAT_QCIF = 3,
    DMS_VIDEO_FORMAT_VGA = 4,
    DMS_VIDEO_FORMAT_QVGA = 5,
    DMS_VIDEO_FORMAT_720P = 6,
    DMS_VIDEO_FORMAT_130H = 7,
    DMS_VIDEO_FORMAT_300H = 8,
    DMS_VIDEO_FORMAT_QQVGA = 9,
    DMS_VIDEO_FORMAT_UXGA = 10,
    DMS_VIDEO_FORMAT_SVGA = 11,
    DMS_VIDEO_FORMAT_1080P = 12,
    DMS_VIDEO_FORMAT_XGA = 13,
    DMS_VIDEO_FORMAT_960P = 14,
    DMS_VIDEO_FORMAT_640 = 15,
    DMS_VIDEO_FORMAT_5M = 17,
    DMS_VIDEO_FORMAT_4M = 18,
    DMS_VIDEO_FORMAT_2048x1520 = 19,
} DMS_VIDEO_FORMAT_TYPE;

//Set Flag
typedef enum emColorSetFlag
{
	DMS_COLOR_SET_BRIGHTNESS =			0x00000001,//亮度
	DMS_COLOR_SET_HUE =					0x00000002,//色度
	DMS_COLOR_SET_SATURATION =			0x00000004,//饱和度
	DMS_COLOR_SET_CONTRAST =			0x00000008,//对比对
	DMS_COLOR_SET_IRISBASIC =			0x00000010,//光圈基准值
	DMS_COLOR_SET_RED =					0x00000020,//
	DMS_COLOR_SET_BLUE =				0x00000040,//
	DMS_COLOR_SET_GREEN =				0x00000080,//
	DMS_COLOR_SET_GAMMA =				0x00000100,//
	DMS_COLOR_SET_DEFINITION =			0x00000200,//清晰度、锐度
	DMS_COLOR_SET_SCENE =				0x00000400, //场景环境
	DMS_COLOR_SET_ADNSWITCH =			0x00000800, //开启降噪
	DMS_COLOR_SET_DN = 					0x00001000, //降噪阀值,开启降噪有效
	DMS_COLOR_SET_AWBSWITCH = 			0x00002000, //自动白平衡切换
	DMS_COLOR_SET_AECSWITCH = 			0x00004000, //自动曝光切换
	DMS_COLOR_SET_EC = 					0x00008000, //曝光阀值
	DMS_COLOR_SET_AGCSWITCH = 			0x00010000, //自动增益切换
	DMS_COLOR_SET_GC = 					0x00020000, //增益阀值
	DMS_COLOR_SET_MIRROR = 				0x00040000, //镜像
	DMS_COLOR_SET_BAW = 				0x00080000, //彩色转黑白
	DMS_COLOR_SET_EWD = 				0x00100000, //打开宽动态
	DMS_COLOR_SET_WD = 					0x00200000, //宽动态档次
	DMS_COLOR_SET_AIRIS = 				0x00400000, //自动光圈使能
	DMS_COLOR_SET_BLC = 				0x00800000, // 背光补偿
} eColorSetFlag;

typedef enum emRecordType
{
    GK_NET_RECORD_TYPE_SCHED   = 0x000001,   //定时录像
    GK_NET_RECORD_TYPE_MOTION  = 0x000002,  //移到侦测录像
    GK_NET_RECORD_TYPE_ALARM   = 0x000004,  //报警录像
    GK_NET_RECORD_TYPE_CMD     = 0x000008, //命令录像
    GK_NET_RECORD_TYPE_MANU    = 0x000010,  //手工录像
    GK_NET_RECORD_TYPE_SCHED_2 = 0x000011 //手工录像
} eRecordType;

#define REC_ACTION_STOP   0
#define REC_ACTION_START  1
//录像控制
typedef struct tagDMS_NET_REC_CONTROL
{
	DWORD	dwSize;
	int    	nChannel;
	int    	nRecordType; //录像类型，参见DMS_NET_RECORD_TYPE_
	WORD    	wAction;     //动作：0 停止录像，1 开始录像
	WORD     wDuration;  //0xFFFFFFFF表示不停止，直到发送再次停止命令。
}DMS_NET_REC_CONTROL;

typedef enum tagWIRELESS_STATUS{
    WL_OK = 0,
    WL_NOT_CONNECT,     //没有在连接
    WL_DEVICE_NOT_EXIST,    //ipc的wifi硬件不存在
    WL_ESSID_NOT_EXIST, //essid不存在
    WL_DHCP_ERROR,          //dhcp获取不到ip
    WL_ENCRYPT_FAIL,        //密码认证错误
    WL_IP_CONFLICT,         //IP地址冲突
	WL_UNSUPPORT_WPS,
}WIRELESS_STATUS_E;

typedef enum
{
    PT_PCMU          = 0,
    PT_1016          = 1,
    PT_G721          = 2,
    PT_GSM           = 3,
    PT_G723          = 4,
    PT_DVI4_8K       = 5,
    PT_DVI4_16K      = 6,
    PT_LPC           = 7,
    PT_PCMA          = 8,
    PT_G722          = 9,
    PT_S16BE_STEREO  = 10,
    PT_S16BE_MONO    = 11,
    PT_QCELP         = 12,
    PT_CN            = 13,
    PT_MPEGAUDIO     = 14,
    PT_G728          = 15,
    PT_DVI4_3        = 16,
    PT_DVI4_4        = 17,
    PT_G729          = 18,
    PT_G711A         = 19,
    PT_G711U         = 20,
    PT_G726          = 21,
    PT_G729A         = 22,
    PT_LPCM          = 23,
    PT_CelB          = 25,
    PT_JPEG          = 26,
    PT_CUSM          = 27,
    PT_NV            = 28,
    PT_PICW          = 29,
    PT_CPV           = 30,
    PT_H261          = 31,
    PT_MPEGVIDEO     = 32,
    PT_MPEG2TS       = 33,
    PT_H263          = 34,
    PT_SPEG          = 35,
    PT_MPEG2VIDEO    = 36,
    PT_AAC           = 37,
    PT_WMA9STD       = 38,
    PT_HEAAC         = 39,
    PT_PCM_VOICE     = 40,
    PT_PCM_AUDIO     = 41,
    PT_AACLC         = 42,
    PT_MP3           = 43,
    PT_ADPCMA        = 49,
    PT_AEC           = 50,
    PT_X_LD          = 95,
    PT_H264          = 96,
    PT_D_GSM_HR      = 200,
    PT_D_GSM_EFR     = 201,
    PT_D_L8          = 202,
    PT_D_RED         = 203,
    PT_D_VDVI        = 204,
    PT_D_BT656       = 220,
    PT_D_H263_1998   = 221,
    PT_D_MP1S        = 222,
    PT_D_MP2P        = 223,
    PT_D_BMPEG       = 224,
    PT_MP4VIDEO      = 230,
    PT_MP4AUDIO      = 237,
    PT_VC1           = 238,
    PT_JVC_ASF       = 255,
    PT_D_AVI         = 256,
    PT_DIVX3		= 257,
    PT_AVS		= 258,
    PT_REAL8		= 259,
    PT_REAL9		= 260,
    PT_VP6		= 261,
    PT_VP6F		= 262,
    PT_VP6A		= 263,
    PT_SORENSON	 =264,
    PT_MAX           = 265,
    /* add by hisilicon */
    PT_AMR           = 1001,
    PT_MJPEG         = 1002,
	PT_H264_HIGHPROFILE = 1003,
	PT_H264_MAINPROFILE = 1004,
	PT_H264_BASELINE = 1005,
	PT_H265 = 1006,
}PAYLOAD_TYPE_E;

typedef struct tagPAYLOAD_NAME{
	int		nPayloadID;
	char	*lpName;
}PAYLOAD_NAME;

typedef struct tagDMS_VIDEO_FORMAT{
	int nVideoFormat;
	int nWidth;
	int nHeight;
	int nPAL;
	char *lpName;
}DMS_VIDEO_FORMAT;

/*      WIFI   */
typedef struct tagDMS_NET_WIFI_SITE
{
    DWORD           dwSize;
    int             nType;
    char            csEssid[DMS_NAME_LEN];
    int             nRSSI;    //wifi 信号强度
    BYTE            byMac[DMS_MACADDR_LEN];
    BYTE            byRes1[2];
    BYTE            bySecurity;
								//0: none  (wifi enable)
								//1: wep 64 assii (wifi enable)
								//2: wep 64 hex (wifi enable)
								//3: wep 128 assii (wifi enable)
								//4: wep 128 hex (wifi enable)
								//5: WPAPSK-TKIP
								//6: WPAPSK-AES
								//7: WPA2PSK-TKIP
								//8: WPA2PSK-AES
    BYTE            byRes2[3];
    int             nChannel;
}DMS_NET_WIFI_SITE;

typedef struct tagDMS_NET_WIFI_SITE_LIST
{
    DWORD           dwSize; //== sizeof(DMS_NET_WIFI_SITE)*nCount
    int             nCount;
    DMS_NET_WIFI_SITE    stWifiSite[100];// 实际传输
}DMS_NET_WIFI_SITE_LIST;

/*硬盘信息*/
/*单个硬盘信息*/
typedef struct
{
	DWORD  dwSize;
	DWORD  dwHDNo;		//硬盘号, 取值0～DMS_MAX_DISKNUM-1
	DWORD  dwHDType;    //硬盘类型(不可设置) 0:SD卡,1:U盘,2:硬盘
    DWORD  dwCapacity;	//硬盘容量(不可设置)
    DWORD  dwFreeSpace; //硬盘剩余空间(不可设置)
    DWORD  dwHdStatus;	//硬盘状态(不可设置)： 0－正常；1－未格式化；2－错误；3－S.M.A.R.T状态；4－不匹配；5－休眠
    BYTE  byHDAttr;		//0－默认；1－冗余；2－只读
    BYTE  byRecStatus;  //是否正在录像--0:空闲,1:正在录像
    BYTE  byRes1[2];	//保留参数
    DWORD  dwHdGroup;	//属于哪个盘组，取值 0～DMS_MAX_HD_GROUP-1
    BYTE  byRes2[120];	//保留
}DMS_NET_SINGLE_HD, *LPDMS_NET_SINGLE_HD;
/*本地硬盘信息配置*/
typedef struct
{

	DWORD  dwSize;
    DWORD  dwHDCount; //硬盘数
	DMS_NET_SINGLE_HD  stHDInfo[DMS_MAX_DISKNUM];
}DMS_NET_HDCFG, *LPDMS_NET_HDCFG;
/*格式化磁盘命令*/
typedef struct
{
	DWORD  dwSize;
    DWORD  dwHDNo; //硬盘号
	BYTE  byRes2[32];//保留
}DMS_NET_DISK_FORMAT, *LPDMS_NET_DISK_FORMAT;
//格式化硬盘状态以及进度
typedef struct
{
	DWORD  dwSize;
    DWORD  dwHDNo; //硬盘号
    DWORD  dwHdStatus;//硬盘状态： 0－格式化开始；1－正在格式化磁盘；2－格式化完成
    DWORD  dwProcess;//格式化进度 0-100
    DWORD  dwResult;//格式化结果 0：成功，1：失败
	BYTE  byRes[16];//保留
}DMS_NET_DISK_FORMAT_STATUS, *LPDMS_NET_DISK_FORMAT_STATUS;


/*单个盘组信息 第一版本不支持*/
typedef struct
{

	DWORD dwHDGroupNo;			//盘组号(不可设置) 0～DMS_MAX_HD_GROUP-1
    BYTE  byHDGroupChans[64];  //盘组对应的录像通道, 0－表示该通道不录象到该盘组；1－表示录象到该盘组
    BYTE  byRes[8];
}DMS_NET_SINGLE_HDGROUP, *LPDMS_NET_SINGLE_HDGROUP;

/*本地盘组信息配置 第一版本不支持*/
typedef struct
{
    DWORD  dwSize;
    DWORD  dwHDGroupCount;
    DMS_NET_SINGLE_HDGROUP  stHDGroupAttr[DMS_MAX_HD_GROUP];
}DMS_NET_HDGROUP_CFG, *LPDMS_NET_HDGROUP_CFG;


typedef struct tagGK_NET_CMS_FRAME_HEADER
{
    DWORD  dwSize;
    DWORD   dwFrameIndex;
    DWORD   dwVideoSize;
    DWORD   dwTimeTick;
    WORD    wAudioSize;
    BYTE    byFrameType; //0：I帧；1：P帧；2：B帧，只针对视频
    BYTE    byVideoCode;
    BYTE    byAudioCode;
    BYTE    byReserved1;// 按4位对齐 ;0: 主码流  1： 子码流
    BYTE    byReserved2; // 按4位对齐  1: I 2:p
    BYTE    byReserved3; // 按4位对齐
    WORD    wVideoWidth;
    WORD    wVideoHeight;
} GK_NET_CMS_FRAME_HEADER;

typedef enum
{
	GK_CMS_PT_PCMU = 0,
	GK_CMS_PT_1016 = 1,
	GK_CMS_PT_G721 = 2,
	GK_CMS_PT_GSM = 3,
	GK_CMS_PT_G723 = 4,
	GK_CMS_PT_DVI4_8K = 5,
	GK_CMS_PT_DVI4_16K = 6,
	GK_CMS_PT_LPC = 7,
	GK_CMS_PT_PCMA = 8,
	GK_CMS_PT_G722 = 9,
	GK_CMS_PT_S16BE_STEREO,
	GK_CMS_PT_S16BE_MONO = 11,
	GK_CMS_PT_QCELP = 12,
	GK_CMS_PT_CN = 13,
	GK_CMS_PT_MPEGAUDIO = 14,
	GK_CMS_PT_G728 = 15,
	GK_CMS_PT_DVI4_3 = 16,
	GK_CMS_PT_DVI4_4 = 17,
	GK_CMS_PT_G729 = 18,
	GK_CMS_PT_G711A = 19,
	GK_CMS_PT_G711U = 20,
	GK_CMS_PT_G726 = 21,
	GK_CMS_PT_G729A = 22,
	GK_CMS_PT_LPCM = 23,
	GK_CMS_PT_CelB = 25,
	GK_CMS_PT_JPEG = 26,
	GK_CMS_PT_CUSM = 27,
	GK_CMS_PT_NV = 28,
	GK_CMS_PT_PICW = 29,
	GK_CMS_PT_CPV = 30,
	GK_CMS_PT_H261 = 31,
	GK_CMS_PT_MPEGVIDEO = 32,
	GK_CMS_PT_MPEG2TS = 33,
	GK_CMS_PT_H263 = 34,
	GK_CMS_PT_SPEG = 35,
	GK_CMS_PT_MPEG2VIDEO = 36,
	GK_CMS_PT_AAC = 37,
	GK_CMS_PT_WMA9STD = 38,
	GK_CMS_PT_HEAAC = 39,
	GK_CMS_PT_PCM_VOICE = 40,
	GK_CMS_PT_PCM_AUDIO = 41,
	GK_CMS_PT_AACLC = 42,
	GK_CMS_PT_MP3 = 43,
	GK_CMS_PT_ADPCMA = 49,
	GK_CMS_PT_AEC = 50,
	GK_CMS_PT_X_LD = 95,
	GK_CMS_PT_H264 = 96,
	GK_CMS_PT_D_GSM_HR = 200,
	GK_CMS_PT_D_GSM_EFR = 201,
	GK_CMS_PT_D_L8 = 202,
	GK_CMS_PT_D_RED = 203,
	GK_CMS_PT_D_VDVI = 204,
	GK_CMS_PT_D_BT656 = 220,
	GK_CMS_PT_D_H263_1998 = 221,
	GK_CMS_PT_D_MP1S = 222,
	GK_CMS_PT_D_MP2P = 223,
	GK_CMS_PT_D_BMPEG = 224,
	GK_CMS_PT_MP4VIDEO = 230,
	GK_CMS_PT_MP4AUDIO = 237,
	GK_CMS_PT_VC1 = 238,
	GK_CMS_PT_JVC_ASF = 255,
	GK_CMS_PT_D_AVI = 256,
	GK_CMS_PT_MAX = 257,

	GK_CMS_PT_AMR = 1001, /* add by mpp */
	GK_CMS_PT_MJPEG = 1002,
}GK_CMS_PT_TYPE_E;

typedef struct tagDMS_NET_USER
{
	DWORD dwSize;
	DMS_NET_USER_INFO stUser[DMS_MAX_USERNUM];
} DMS_NET_USER,*LPDMS_NET_USER;

typedef struct tagJBNV_TIME{
	DWORD dwYear;
	DWORD dwMonth;
	DWORD dwDay;
	DWORD dwHour;
	DWORD dwMinute;
	DWORD dwSecond;
}JBNV_TIME;

typedef struct tagJBNV_FindFileReq{		//查询条件
	DWORD		dwSize;
	DWORD		nChannel;				//0xff：全部通道，0，1，2 ......
	DWORD		nFileType;				//文件类型 ：0xff － 全部，0 － 定时录像，1 - 移动侦测，2 － 报警触发，3  － 手动录像
	JBNV_TIME	BeginTime;				//
	JBNV_TIME	EndTime;				//StartTime StopTime 的值都为0000-00-00 00:00:00表示查找所有时间
}JBNV_FIND_FILE_REQ;

typedef struct tagJBNV_FILE_DATA_INFO{
	char		sFileName[256];			//文件名
	JBNV_TIME	BeginTime;				//
	JBNV_TIME	EndTime;
	DWORD		nChannel;
	DWORD		nFileSize;				//文件的大小
	DWORD		nState;					//文件转储状态
}JBNV_FILE_DATA_INFO;

typedef struct tagJBNV_FindFileResp{
	DWORD		dwSize;
	DWORD		nResult;		//0:success ;1:find file error ; 2:the number of file more than the memory size, and the result contains part of the data
	DWORD		nCount;
	DWORD		dwReserved[3];
}JBNV_FIND_FILE_RESP;

/*云台解码器485串口参数*/
typedef struct
{
    DWORD dwSize; /* 此结构的大小 */
    DWORD   dwChannel;
    DWORD dwBaudRate; /* 实际波特率(bps)*/
    BYTE byDataBit; /* 数据有几位 0－5位，1－6位，2－7位，3－8位 */
    BYTE byStopBit; /* 停止位 0－1位，1－2位  */
    BYTE byParity; /* 校验 0－无校验，1－奇校验，2－偶校验; */
    BYTE byFlowcontrol; /* 0－无，1－软流控,2-硬流控 */
    char  csDecoderType[DMS_NAME_LEN]; /* 解码器类型, 见下表*/
    WORD wDecoderAddress; /*解码器地址:0 - 255*/
    BYTE byHSpeed;      //云台H速度
    BYTE byVSpeed;      //云台V速度
    BYTE byWatchPos;  //看守位
    BYTE byRes;
    WORD dwRes;
 //   DWORD dwRes;
}DMS_NET_DECODERCFG,*LPDMS_NET_DECODERCFG;

//云台协议表结构配置
typedef struct
{
    DWORD dwType;               /*解码器类型值，从1开始连续递增*/
    char  csDescribe[DMS_NAME_LEN]; /*解码器的描述符*/
}DMS_PTZ_PROTOCOL;

typedef struct
{
    DWORD   dwSize;
    DMS_PTZ_PROTOCOL stPtz[DMS_PTZ_PROTOCOL_NUM];/*最大200中PTZ协议*/
    DWORD   dwPtzNum;           /*有效的ptz协议数目*/
    BYTE    byRes[8];           // 保留参数
}DMS_NET_PTZ_PROTOCOLCFG, *LDMS_NET_PTZ_PROTOCOLCFG;


/*NAS*/
typedef struct
{
    char   csServerIP[DMS_MAX_IP_LENGTH];
    char   byPath[64];  //服务器文件路径
    BYTE   byEnable;	//0:关闭,1:启用
    BYTE   byRes2[79];   //保留
}DMS_NAS_SERVER_INFO;

typedef struct
{
    DWORD  dwSize;
    DMS_NAS_SERVER_INFO stServerInfo[8];
}DMS_NET_NAS_CFG;


typedef struct _OPEN_FILE
{
	DWORD		nFileType; // 1 open by filename ,2 open by time
	char		csFileName[256];
	DWORD		nChannel;
	DWORD		nProtocolType;
	DWORD		nStreamType;

	JBNV_TIME	struStartTime;
	JBNV_TIME	struStopTime;
}OPEN_FILE,*POPEN_FILE;

typedef struct _OPEN_FILE_EX
{
	DWORD	nFlag2;
	DWORD	nID;
	DWORD		nFileType; // 1 open by filename ,2 open by time
	char		csFileName[256];
	DWORD		nChannel;
	DWORD		nProtocolType;
	DWORD		nStreamType;
	JBNV_TIME	struStartTime;
	JBNV_TIME	struStopTime;
}OPEN_FILE_EX,*POPEN_FILE_EX;

typedef struct tagFilePlay_INFO
{
    DWORD   dwSize;
    DWORD   dwStream1Height;	//视频高(1)
    DWORD   dwStream1Width;		//视频宽
	DWORD   dwStream1CodecID;	//视频编码类型号（MPEG4为0，JPEG2000为1,H264为2）
    DWORD   dwAudioChannels;	//音频通道数
    DWORD   dwAudioBits;		//音频比特率
    DWORD   dwAudioSamples;		//音频采样率
    DWORD   dwWaveFormatTag;	//音频编码类型号
	char	csChannelName[32];	//通道名称
	DWORD	dwFileSize;
	DWORD	dwTotalTime;
	DWORD	dwPlayStamp;		//时间戳
}FilePlay_INFO,*PFilePlay_INFO;

typedef struct tagNET_AUDIO_INFO
{
	ULONG	nFlag;
	ULONG	nChannel;
	ULONG	nBitRate;
	ULONG	nSmapleRate;
}NET_AUDIO_INFO,*PNET_AUDIO_INFO;

typedef enum
{
	DMS_NET_PLAYSTART = 0,
	DMS_NET_PLAYPAUSE,
	DMS_NET_PLAYRESTART,
	DMS_NET_FASTFORWARD2X,
	DMS_NET_FASTFORWARD4X,
	DMS_NET_FASTFORWARD8X,
	DMS_NET_FASTFORWARD16X,
	DMS_NET_FASTFORWARD32X,
	DMS_NET_SLOWPLAY2X,
	DMS_NET_SLOWPLAY4X,
	DMS_NET_SLOWPLAY8X,
	DMS_NET_SLOWPLAY16X,
	DMS_NET_SLOWPLAY32X,
	DMS_NET_PLAYNORMAL,
	DMS_NET_PLAYFRAME,
	DMS_NET_PLAYSETPOS,
	DMS_NET_PLAYGETPOS,
	DMS_NET_PLAYGETTIME,
	DMS_NET_PLAYGETFRAME,
	DMS_NET_GETTOTALFRAMES,
	DMS_NET_GETTOTALTIME,
	DMS_NET_THROWBFRAME,
} DMS_PLAY_BACK_CONTROL_E;


typedef enum
{
    JB_PLAY_CMD_PLAY = 1,
    JB_PLAY_CMD_PLAYPREV = 2,
    JB_PLAY_CMD_STEPIN = 3,
    JB_PLAY_CMD_STEPOUT = 4,
    JB_PLAY_CMD_PAUSE = 5,
    JB_PLAY_CMD_RESUME = 6,
    JB_PLAY_CMD_FASTPLAY = 7,
    JB_PLAY_CMD_FASTBACK = 8,
    JB_PLAY_CMD_STOP = 9,
    JB_PLAY_CMD_SLOWPLAY = 10,
    JB_PLAY_CMD_FIRST = 11,   /*定位到文件头*/
    JB_PLAY_CMD_LAST = 12,    /*定位到文件末尾*/
    JB_PLAY_CMD_SEEK = 13,    /*定位播放*/
    JB_PLAY_CMD_SOUND = 14,
}  JB_PLAY_CMD_E;

typedef struct tagVODActionReq{
	DWORD		dwSize;
	DWORD		dwAction; //1.2......
	DWORD		dwData;
}VODACTION_REQ ,*pVODACTION_REQ;

typedef enum
{
    JB_TEST_RESULT_DNSUP = 0,   // 不支持
    JB_TEST_RESULT_DNT,         // 尚未测试
    JB_TEST_RESULT_SUCCESS,     // 测试成功
    JB_TEST_RESULT_FAILED,      // 测试失败
    JB_TEST_RESULT_BUTT,
}  JB_TEST_RESULT_E;

typedef struct tagNET_TEST_RESULT_INFO
{
    JB_TEST_RESULT_E emSpeaker;
    JB_TEST_RESULT_E emMic;
    JB_TEST_RESULT_E emLed;
    JB_TEST_RESULT_E emSd;
    JB_TEST_RESULT_E emIRCut;
    JB_TEST_RESULT_E emWifi;
    JB_TEST_RESULT_E emPTZ;
    JB_TEST_RESULT_E emRestore;
    JB_TEST_RESULT_E emView;
    JB_TEST_RESULT_E emWhiteLed;
    JB_TEST_RESULT_E emResetBtn;
    JB_TEST_RESULT_E reserved1;
    JB_TEST_RESULT_E reserved2;
}NET_TEST_RESULT_INFO,*PNET_TEST_RESULT_INFO;

typedef struct tagNET_AF_OFFSET_INFO
{
	DWORD		dwValue;
	DWORD		dwReserved[3];
}NET_AF_OFFSET_INFO;

typedef struct tagNET_TEST_CMD_INFO
{
	Char cmd[128];
}NET_TEST_CMD_INFO;

#define		TMNV4NET_FLAG			904
#define		TMNVHISINET_FLAG		931
#define		DMS_DVR_HI_FLAG			932
#define		DMS_DVR_HI2_FLAG		933
#define		DMS_G711A_160			934


#define ALARM_IN_NUM 0
#define ALARM_OUT_NUM 0
#define CMS_FUNC_MASK1 (DMS_SYS_FLAG_ENCODE_D1 | DMS_SYS_FLAG_HD_RECORD \
                       | DMS_SYS_FLAG_DECODE_H264 | DMS_SYS_FLAG_NETWORK \
                       | DMS_SYS_FLAG_PTZ | DMS_SYS_FLAG_DEVICEMAINTAIN)

#define CMS_FUNC_MASK2 (DMS_SYS_FLAG_ENCODE_D1 | DMS_SYS_FLAG_HD_RECORD \
                       | DMS_SYS_FLAG_DECODE_H264 | DMS_SYS_FLAG_FTPUP \
                       | DMS_SYS_FLAG_EMAIL | DMS_SYS_FLAG_WIFI \
                       | DMS_SYS_FLAG_DEVICEMAINTAIN \
                       | DMS_SYS_FLAG_NETWORK)


void PrintDmsTime(DMS_SYSTEMTIME *ptime);
int Gk_LogOn(int sock);
int GkDealCmdDispatch(int sock);
int GkHandleMediaRequest(int sock);
int GkCmsCmdResq(int sock, void *body, int body_size, int cmd);
int GkCmsCmdResqByIndex(int sock, void *body, int body_size, int cmd, int stream_no);
int Gk_CmsNotify(int sock, unsigned long type, void *data, int data_size);
int cms_notify_to_all_client(unsigned long type);

#ifdef __cplusplus
}
#endif
#endif


