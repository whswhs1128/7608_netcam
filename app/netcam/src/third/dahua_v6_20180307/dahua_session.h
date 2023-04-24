#ifndef __DAHUA_SESSION_H__
#define __DAHUA_SESSION_H__

#include <fcntl.h>
#include <netdb.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <sys/prctl.h>
#include <pthread.h>
#include <semaphore.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <dirent.h>
#include <linux/unistd.h>
#include <errno.h>
#include <sys/syscall.h>

#include "dahua_common.h"


/*************************************************
********************协议报文命令定义 ****************
*************************************************/
//报文命令字
#define CMD_ZHINUO_LOG_ON 0xd2    //登录
#define CMD_DAHUA_LOG_ON 0xa0    //登录
#define CMD_STATUS_SEARCH 0xa1    //查询工作及报警状态
#define CMD_DEV_EVENT_SEARCH 0x68    //查询设备事件
#define CMD_MEDIA_REQUEST 0x11    //媒体数据请求
#define CMD_MEDIA_CAPACITY_SEARCH 0x83    //查询设备媒体能力信息
#define CMD_CONFIG_SEARCH 0xa3    //查询配置参数
#define CMD_CHANNEL_NAME_SEARCH 0xa8    //查询通道名称
#define CMD_SYSTEM_INFO_SEARCH 0xa4    //查询系统信息
#define CMD_CONNECT_REQUEST 0xf1    //请求建立连接
#define CMD_I_FRAME_REQUEST 0x80    //强制I帧
#define CMD_DEV_CONTROL 0x60    //设备操作
#define CMD_SET_CONFIG 0xc1    //修改参数配置
#define CMD_TIME_MANAGE 0x24    //时间管理
#define CMD_SET_CHANNEL_NAME 0xC6    //修改通道名字
#define CMD_PTZ_CONTROL 0x12    //云台控制
#define CMD_TALK_REQUEST 0x1d    //对讲请求
#define CMD_TALK 0xc0    //对讲
#define CMD_TALK_CONTROL 0x1e    //对讲控制
#define CMD_RECORD_SEARCH 0xa5    //录像搜索
#define CMD_RECORD_PLAYBACK 0xc2    //录像回放
#define CMD_RECORD_DOWNLOAD 0xbb    //录像下载
#define CMD_DEV_SEARCH 0xa3    //设备搜索
#define CMD_INFO_NEW 0xf6    //未知命令
#define CMD_CONTROL_CONNECT 0xf4    //控制连接


//回复报文命令字
#define ACK_LOG_ON 0xb0    //登录
#define ACK_STATUS_SEARCH 0xb1    //查询工作及报警状态
#define ACK_DEV_EVENT_SEARCH 0x69    //查询设备事件
#define ACK_MEDIA_REQUEST 0xbc    //媒体数据请求
#define ACK_MEDIA_CAPACITY_SEARCH 0x83    //查询设备媒体能力信息
#define ACK_CONFIG_SEARCH 0xb3    //查询配置参数
#define ACK_CHANNEL_NAME_SEARCH 0xb8   //查询通道名称
#define ACK_SYSTEM_INFO_SEARCH 0xb4    //查询系统信息
#define ACK_CONNECT_REQUEST 0xf1    //请求建立连接
#define ACK_I_FRAME_REQUEST 0x80    //强制I帧
#define ACK_DEV_CONTROL 0x60    //设备操作
#define ACK_SET_CONFIG 0xC1    //修改参数配置
#define ACK_TIME_MANAGE 0x24    //时间管理
#define ACK_SET_CHANNEL_NAME 0xb8   //修改通道名字
#define ACK_PTZ_CONTROL 0x12    //云台控制
#define ACK_TALK_REQUEST 0xc0    //对讲请求
#define ACK_TALK 0x1d    //对讲
#define ACK_TALK_CONTROL 0x1e    //对讲控制
#define ACK_RECORD_SEARCH 0xb6   //录像搜索
#define ACK_RECORD_PLAYBACK 0xc2    //录像回放
#define ACK_RECORD_DOWNLOAD 0xbb    //录像下载
#define ACK_DEV_SEARCH 0xb3    //设备搜索
#define ACK_UNKOWN 0xf6    //未知命令
#define ACK_CONTROL_CONNECT 0xf4    //控制连接



#define DMS_PTZ_CMD_UP              1
#define DMS_PTZ_CMD_DOWN            2
#define DMS_PTZ_CMD_LEFT            3
#define DMS_PTZ_CMD_RIGHT           4
#define DMS_PTZ_CMD_UP_LEFT			31
#define DMS_PTZ_CMD_UP_RIGHT		32
#define DMS_PTZ_CMD_DOWN_LEFT		33
#define DMS_PTZ_CMD_DOWN_RIGHT		34

#define DMS_PTZ_CMD_FOCUS_SUB       5       //Focus Far
#define DMS_PTZ_CMD_FOCUS_ADD       6       //Focus Near
#define DMS_PTZ_CMD_ZOOM_SUB        7       //Zoom Wide
#define DMS_PTZ_CMD_ZOOM_ADD        8       //Zoom Tele
#define DMS_PTZ_CMD_IRIS_SUB        9       //Iris Close
#define DMS_PTZ_CMD_IRIS_ADD        10      //Iris Open
#define DMS_PTZ_CMD_STOP            11
#define DMS_PTZ_CMD_PRESET          12      //预置
#define DMS_PTZ_CMD_CALL            13      //调用

#define DMS_PTZ_CMD_AUTO_STRAT      14      //自动      //STRAT shoule be START
#define DMS_PTZ_CMD_AUTO_STOP       15
#define DMS_PTZ_CMD_LIGHT_OPEN      16      //灯光
#define DMS_PTZ_CMD_LIGHT_CLOSE     17
#define DMS_PTZ_CMD_BRUSH_START     18      //雨刷
#define DMS_PTZ_CMD_BRUSH_STOP      19
#define DMS_PTZ_CMD_TRACK_START     20      //轨迹
#define DMS_PTZ_CMD_TRACK_STOP      21
#define DMS_PTZ_CMD_TRACK_RUN       22
#define DMS_PTZ_CMD_PRESET_CLS      23      //清除预置点

#define DMS_PTZ_CMD_ADD_POS_CRU		24		/* 将预置点加入巡航序列 */
#define DMS_PTZ_CMD_DEL_POS_CRU		25		/* 将巡航点从巡航序列中删除 */
#define DMS_PTZ_CMD_DEL_PRE_CRU		26		/* 将预置点从巡航序列中删除 */
#define DMS_PTZ_CMD_MOD_POINT_CRU	27		/* 修改巡航点*/
#define DMS_PTZ_CMD_START_CRU		28		/* 开始巡航 */
#define DMS_PTZ_CMD_STOP_CRU		29		/* 停止巡航 */
#define DMS_PTZ_CMD_CRU_STATUS		30		/* 巡航状态 */


////////////////////////////////////

//基本宏定义
#define DAHUA_MAX_LINK_NUM  10
//#define	DVRIP_VERSION	5
#define	DVRIP_VERSION	6

#define RE_CONNECT_TIME                          (10)    //如果登录失败的重连的时间
#define BUFLEN (10 * 1024)  //新的nvr接收区很大
#define SEND_TIMEOUT                              (15) //socket发送数据的超时时间
#define USER_NAME "admin"
#define PASSWD "admin"
#define MAX_VIDEO_FRAME_SIZE                     (524 * 1024)   //实时视频允许获取的帧的最大字节数
#define MAX_SEND_BUFF_SIZE                       (524 * 1024)   //send的最大发送buf的大小
 
#define ZHINUO_ONE_PACKAGE_SIZE    (40*1024)  
//#define DAHUA_ONE_PACKAGE_SIZE    (20*1024)  //这个不能设置超过40k不然大华的视频出不来
#define DAHUA_ONE_PACKAGE_SIZE    (32*1024)  //新的NVR是32K

#define AUDIO_SIZE_PRE_FRAME    (320) //每个音频帧G711A裸流数据的实际大小
//#define DEV_NAME "IPC-IPVM3150F" //"IPC_3507_720P"
//#define DEV_NAME "IPC"
//#define DEV_NAME "TP5"
#define DEV_NAME "IPC-HDW1025C"
//#define DEV_NAME "IPC"


#define DEV_DEVICE_NAME "IPC"


//#define DEV_DEVICE "IPC"
//#define DEV_DEVICE "IPC-HDW1025C"

//#define DEV_SERIAL "3L04587PAG02470"
//#define DEV_SERIAL "1K026D1PAA00928"
#define DEV_SERIAL "000000000000000"

#define DEV_VER "2.621.0000000.8.R"
#define DEV_WEB_VER "3.2.1.514808"


#define ZHINUO_RCV_PORT 6060
#define ZHINUO_SEND_PORT 6061
#define IFNAME "eth0"
#define DAHUA_RCV_PORT 5050
#define DAHUA_SEND_PORT 5050
#define DAHUA_TCP_LISTEN_PORT 37777
#define ZHINUO_NVR    45
#define DAHUA_NVR    46

//#define DEV_FUNCTION_SEARCH "FTP:1:Record,Snap&&SMTP:1:AlarmText,AlarmSnap&&NTP:2:AdjustSysTime&&VideoCover:1:MutiCover&&AutoRegister:1:Login&&AutoMaintain:1:Reboot,DeleteFiles,ShutDown&&UPNP:1:SearchDevice&&DHCP:1:RequestIP&&STORE POSITION:1:FTP&&DefaultQuery:1:DQuery&&ACFControl:1:ACF&&ENCODE OPTION:1:AssiCompression&&DavinciModule:1:WorkSheetCFGApart,StandardGOP&&Dahua.a4.9:1:Login&&Dahua.Device.Record.General:1:General&&IPV6:1:IPV6Config&&Log:1:PageForPageLog&&QueryURL:1:CONFIG&&DriverTypeInfo:1:DriverType&&ProtocolFramework:1:V3_1"
#define DEV_FUNCTION_SEARCH "FTP:1:Record,Snap&&SMTP:1:AlarmText,AlarmSnap,HealthMail&&NTP:2:AdjustSysTime&&VideoCover:1:MutiCover&&AutoRegister:1:Login&&DriverTypeInfo:1:DriverType&&AutoMaintain:1:Reboot,DeleteFiles,ShutDown&&UPNP:1:SearchDevice&&DHCP:1:RequestIP&&STORE POSITION:1:FTP&&DefaultQuery:1:DQuery&&ImportantRecID:1:RECID&&SNMP:1:SNMPConfig&&ISCSI:1:ISCSIConfig&&PlayBackSpeedControl:1:SpeedUp&&ACFControl:1:ACF&&IPV6:1:IPV6Config&&DavinciModule:1:WorkSheetCFGApart,StandardGOP&&Dahua.a4.9:1:Login&&Dahua.Device.Record.General:1:General&&Log:1:PageForPageLog&&QueryURL:1:CONFIG&&SearchRecord:1:V3&&BackupVideoExtFormat:1:DAV,ASF&&Dahua_Config:1:Json,V3,MotionDetect_F6&&ProtocolFramework:1:V3_1&&DynamicMultiConnecting:1:TalkSnd&&Dahua.MultiNet:1:Config&&CommSniffer:1:CommATM&&NetSniffer:1:NetSniffer&&Net:1:RSA,A3"


//结构体相关宏
#define MAX_ENC_CHIP_NR 32
#define MD_REGION_ROW 32
#define N_SYS_CH 16
#define EXTRATYPES 3
#define N_COLOR_SECTION 2


//扩展帧头命令字
#define EXPAND_PICTURE_SIZE 0x80
#define EXPAND_PLAY_BACK_TYPE 0x81
#define EXPAND_AUDIO_FORMAT 0x83
#define EXPAND_DATA_CHECK 0x88

//帧类型
#define 	FRAME_TYPE_I  0xFD  
#define 	FRAME_TYPE_P  0XFC
#define 	FRAME_TYPE_B  0XFE
#define 	AUDIO_TYPE  0XF0

typedef struct __prevideo_capture{
    unsigned char iCifCaps;     // 大小
    char iFrameCaps;    // 帧率
    unsigned char reserved[30]; // 保留字节
}Prevideo_Capture;

typedef enum capture_size_t {
    CAPTURE_SIZE_D1,        ///< 704*576(PAL)   704*480(NTSC)
    CAPTURE_SIZE_HD1,       ///< 352*576(PAL)   352*480(NTSC)
    CAPTURE_SIZE_BCIF,      ///< 704*288(PAL)   704*240(NTSC)
    CAPTURE_SIZE_CIF,       ///< 352*288(PAL)   352*240(NTSC)
    CAPTURE_SIZE_QCIF,      ///< 176*144(PAL)   176*120(NTSC)
    CAPTURE_SIZE_VGA,       ///< 640*480(PAL)   640*480(NTSC)
    CAPTURE_SIZE_QVGA,      ///< 320*240(PAL)   320*240(NTSC)
    CAPTURE_SIZE_SVCD,      ///< 480*480(PAL)   480*480(NTSC)
    CAPTURE_SIZE_QQVGA,     ///< 160*128(PAL)   160*128(NTSC)
    CAPTURE_SIZE_SVGA,      ///< 800*592
    CAPTURE_SIZE_XVGA,      ///< 1024*768
    CAPTURE_SIZE_WXGA,      ///< 1280*800
    CAPTURE_SIZE_SXGA,      ///< 1280*1024 
    CAPTURE_SIZE_WSXGA,     ///< 1600*1024 
    CAPTURE_SIZE_UXGA,      ///< 1600*1200
    CAPTURE_SIZE_WUXGA,     ///< 1920*1200
    CAPTURE_SIZE_LTF,       ///< 240*192(PAL);浪涛平台使用
    CAPTURE_SIZE_720p ,     ///< 1280*720
    CAPTURE_SIZE_1080p,     ///< 1920*1080
    CAPTURE_SIZE_1_3M,      ////<1280*960
    CAPTURE_SIZE_NR         ///< 枚举的图形大小种类的数目。
}CAPTURE_SIZE_T;

typedef struct tagZhiNuo_Dev_Search
{
    unsigned char Version[8]; // 8字节的版本信息
    char HostName[16]; // 主机名
    unsigned long HostIP; // IP 地址
    unsigned long Submask; // 子网掩码
    unsigned long GateWayIP; // 网关IP
    unsigned long DNSIP; // DNS IP 40个字节位置

    // 外部接口
    unsigned long	AlarmServerIP; // 报警中心IP
    unsigned short  AlarmServerPort; // 报警中心端口
    unsigned long	SMTPServerIP; // SMTP server IP
    unsigned short  SMTPServerPort; // SMTP server port
    unsigned short  resPort1; 
    unsigned long	LogServerIP; // Log server IP
    unsigned short  LogServerPort; // Log server port

    // 本机服务端口
    unsigned short  HttpPort; // HTTP服务端口号
    unsigned short  HttpsPort; // HTTPS服务端口号
    unsigned short  TCPPort; // TCP 侦听端口   68个字节位置
    unsigned short  TCPMaxConn; // TCP 最大连接数
    unsigned short  SSLPort; // SSL 侦听端口 72
    unsigned short  UDPPort; // UDP 侦听端口 76
    unsigned short  resPort2; // 
    unsigned long	McastIP; // 组播IP  80
    unsigned short  McastPort; // 组播端口

    // 其他
    unsigned char  MonMode; // 监视协议0-TCP, 1-UDP, 2-MCAST //待确定-TCP
    unsigned char  PlayMode; // 回放协议0-TCP, 1-UDP, 2-MCAST//待确定-TCP 84
    unsigned char  AlmSvrStat; // 报警中心状态0-关闭, 1-打开 88
}ZhiNuo_Dev_Search;


//时间日期结构
typedef struct tagZhiNuo_DateTime					                        
{
    DWORD second	:6;		//	秒	0-59
    DWORD minute	:6;		//	分	0-59
    DWORD hour		:5;		//	时	0-23
    DWORD day		:5;		//	日	1-31
    DWORD month		:4;		//	月	1-12
    DWORD year		:6;		//	年	2000-2063	
}ZhiNuo_DateTime;

//扩展帧头图像尺寸
typedef struct tagZhiNuo_Expand_Picture_Size
{
    unsigned char ch_cmd;
    unsigned char ch_coding_type; //ZhiNuo_coding_type_e
    unsigned char ch_width;
    unsigned char ch_high;    
}ZhiNuo_Expand_Picture_Size;

//扩展帧头回放类型。
typedef struct tagZhiNuo_Expand_PlayBack_Type
{
    unsigned char ch_cmd;
    unsigned char ch_reserve;
    unsigned char ch_video_type; //ZhiNuo_video_type_e
    unsigned char ch_frame_rate;
}ZhiNuo_Expand_PlayBack_Type;

typedef struct
{
    unsigned char ch_cmd;
    unsigned char ch_reserve;
    unsigned char ch_video_type; //ZhiNuo_video_type_e
    unsigned char ch_frame_rate;
    unsigned char cmd2[8];
}ZhiNuo_Expand_PlayBack_Type2;


//扩展帧头音频格式。
typedef struct tagZhiNuo_Expand_Audio_Format
{
    unsigned char ch_cmd;
    unsigned char ch_audio_channel; //ZhiNuo_audio_channel_e
    unsigned char ch_audio_type;   //ZhiNuo_audio_type_e
    unsigned char ch_sample_frequency; //ZhiNuo_sample_sequence_e
}ZhiNuo_Expand_Audio_Format;

#pragma pack(1)
//扩展帧头数据校验。
typedef struct tagZhiNuo_Expand_Data_Check
{
    unsigned char ch_cmd;
    unsigned int  n_check_result;
    unsigned short s_reserve;
    unsigned char ch_check_type; //ZhiNuo_data_check_type_e
}ZhiNuo_Expand_Data_Check;
#pragma pack()


//音视频报文头结构体 24个字节
typedef struct tagZhiNuo_Media_Frame_Head
{
    unsigned char sz_tag[4];
    unsigned char ch_media_type;
    unsigned char ch_child_type;
    unsigned char ch_channel_num;
    unsigned char ch_child_sequence;
    unsigned int  n_frame_sequence;
    unsigned long   n_frame_len;
    ZhiNuo_DateTime t_date;
    unsigned short s_timestamp;
    unsigned char ch_expand_len;
    unsigned char ch_checksum;    
}ZhiNuo_Media_Frame_Head;

typedef struct tagZhiNuo_Media_Frame_Tail
{
    unsigned char sz_tag[4];
    unsigned long  unl_data_len;   
}ZhiNuo_Media_Frame_Tail;

//录像下载请求回复结构
typedef struct tagZhiNuo_sVideoRecordDownloadInfo
{
    DWORD  begin_seek;
    DWORD  end_seek;
    DWORD  record_id;  
}ZhiNuo_sVideoRecordDownloadInfo;

typedef struct 
{
    char				szIP[16];		// IP
    int					nPort;							// 端口
    char				szSubmask[16];	// 子网掩码
    char				szGateway[16];	// 网关
    char				szMac[40];			// MAC地址
    char				szDeviceType[32];	// 设备类型
    BYTE				bReserved[32];					// 保留字节
} ZhiNuo_DEVICE_NET_INFO;

//命令报文头结构体
typedef struct dvrip
{
    unsigned char	cmd;			/* command  */
    unsigned char	dvrip_r0;		/* reserved */
    unsigned char	dvrip_r1;		/* reserved */
    unsigned char	dvrip_hl:4,		/* header length */
                    dvrip_v :4;		/* version */
    unsigned int	dvrip_extlen;	/* ext data length */
    unsigned char	dvrip_p[24];	/* inter params */
}DVRIP;

typedef union
{
    struct dvrip	dvrip; /* struct def */
    unsigned char	c[32]; /* 1 byte def */
    unsigned short	s[16]; /* 2 byte def */
    unsigned int	l[8];  /* 4 byte def */
}DVRIP_HEAD_T;

#define DVRIP_HEAD_T_SIZE sizeof(DVRIP)
#define ZERO_DVRIP_HEAD_T(X) memset((X), 0, DVRIP_HEAD_T_SIZE);	\
    (X)->dvrip.dvrip_hl = DVRIP_HEAD_T_SIZE/4;\
(X)->dvrip.dvrip_v = DVRIP_VERSION;

//编码类型
typedef enum
{
    ONE_FRAME,  //0:编码时只有一场(帧)
    MIX_FRAME,  //1:编码时两场交织
    TWO_FRAME,	 //2:编码时分两场
}ZhiNuo_coding_type_e;

//视频帧类型
typedef enum
{
    MPEG4 = 1,  
    H264,
    MPEG4_LB,	
    H264_GBE,
    JPEG,
    JPEG2000,
    AVS,
}ZhiNuo_video_type_e;

//音频帧类型
typedef enum
{
    PCM8 = 7,  
    G729 ,
    IMA_ADPCM,	
    G711U,
    G721,
    PCM8_VWIS ,
    MS_ADPCM,
    G711A,
    PCM16,
}ZhiNuo_audio_type_e;

//音频声道类型
typedef enum
{
    SINGLE_CHANNEL = 1, //单声道
    DOUBLE_CHANNEL,//双声道     
}ZhiNuo_audio_channel_e;

//音频采样率
typedef enum
{
    SAMPLE_FREQ_4000 = 1,  
    SAMPLE_FREQ_8000,
    SAMPLE_FREQ_11025,	
    SAMPLE_FREQ_16000,
    SAMPLE_FREQ_20000,
    SAMPLE_FREQ_22050 ,
    SAMPLE_FREQ_32000,
    SAMPLE_FREQ_44100,
    SAMPLE_FREQ_48000,
}ZhiNuo_sample_sequence_e;

typedef enum
{
    check_type_add,  
    check_type_xor, 
    check_type_CRC32, 	
}ZhiNuo_data_check_type_e;

typedef enum 
{
    ZhiNuo_PTZ_CMD_UP = 0x00, // 上
    ZhiNuo_PTZ_CMD_DOWN, // 下
    ZhiNuo_PTZ_CMD_LEFT, // 左
    ZhiNuo_PTZ_CMD_RIGHT, // 右
    ZhiNuo_PTZ_CMD_ZOOM_ADD, // 变倍大     
    ZhiNuo_PTZ_CMD_ZOOM_SUB, // 变倍小 
    ZhiNuo_PTZ_CMD_FOCUS_ADD = 0x07,    // 变焦大    
    ZhiNuo_PTZ_CMD_FOCUS_SUB, // 变焦小
    ZhiNuo_PTZ_CMD_IRIS_ADD, // 光圈+
    ZhiNuo_PTZ_CMD_IRIS_SUB, // 光圈-
    ZhiNuo_PTZ_CMD_LIGHT_BRUSH = 0x0e,//   灯光 
    ZhiNuo_PTZ_CMD_AUTO = 0x0f, // 巡航 
    ZhiNuo_PTZ_CMD_PRESET_GO = 0X10,//跳到预置点   
    ZhiNuo_PTZ_CMD_PRESET_SET = 0X11,//设置预置点  
    ZhiNuo_PTZ_CMD_PRESET_CLR = 0X12,//清除预置点  
    ZhiNuo_PTZ_CMD_LEFT_UP = 0x20, // 左上
    ZhiNuo_PTZ_CMD_RIGHT_UP, // 右上
    ZhiNuo_PTZ_CMD_LEFT_DOWN, // 左下
    ZhiNuo_PTZ_CMD_RIGHT_DOWN, // 右下 
    ZhiNuo_PTZ_CMD_STOP = 0xff, //停止
    ZhiNuo_PTZ_CMD_ADD_POS_CRU = 0x24,   
    ZhiNuo_PTZ_CMD_DEL_POS_CRU, 
    ZhiNuo_PTZ_CMD_CLR_POS_CRU,     
} ZhiNuo_PT_PTZCMD_E;

/// 捕获压缩格式类型
enum capture_comp_t {
    CAPTURE_COMP_DIVX_MPEG4,	///< DIVX MPEG4。
    CAPTURE_COMP_MS_MPEG4,		///< MS MPEG4。
    CAPTURE_COMP_MPEG2,			///< MPEG2。
    CAPTURE_COMP_MPEG1,			///< MPEG1。
    CAPTURE_COMP_H263,			///< H.263
    CAPTURE_COMP_MJPG,			///< MJPG
    CAPTURE_COMP_FCC_MPEG4,		///< FCC MPEG4
    CAPTURE_COMP_H264,			///< H.264
    CAPTURE_COMP_NR				///< 枚举的压缩标准数目。
};

enum rec_type
{
    REC_TYP_TIM = 0,		/*定时录像*/
    REC_TYP_MTD,					
    REC_TYP_ALM,
    REC_TYP_NUM,
};

enum capture_brc_t {
    CAPTURE_BRC_CBR,
    CAPTURE_BRC_VBR,
    CAPTURE_BRC_MBR,
    CAPTURE_BRC_NR,
};

typedef struct tagAv_Connect_Info
{
    int n_socket; 					
    int n_flag;						
}Av_Connect_Info;

typedef struct __frame_caps
{
    DWORD Compression; //!压缩模式,  支持的压缩标准的掩码，位序号对应枚举类型						
    //!capture_comp_t的每一个值。
    /// 置1表示支持该枚举值对应的特性，置0表示不支持。
    DWORD ImageSize;	//!分辨率, /// 支持的图像大小类型的掩码，位序号对应枚举类型						
    //!capture_size_t的每一个值。
    ///! 置1表示支持该枚举值对应的特性，置0表示不支持
}FRAME_CAPS;

typedef struct tagCAPTURE_EXT_STREAM
{
    DWORD ExtraStream;							//用channel_t的位来表示支持的功能
    DWORD CaptureSizeMask[64];	//每一个值表示对应分辨率支持的辅助码流。
}CAPTURE_EXT_STREAM, *pCAPTURE_EXT_STREAM;	

typedef struct {	// 定时时段
    BYTE	StartHour;
    BYTE	StartMin;
    BYTE	StartSec;
    BYTE	EndHour;
    BYTE    EndMin;
    BYTE	EndSec;
    BYTE	State;			//第二位是定时，第三位是动态检测，第四位是报警
    BYTE	Reserve;		/*!< Reserve已经被使用，更改的话请通知录像模块 */
} TSECT;

//! 颜色设置内容
typedef struct 
{
    TSECT 	Sector;				/*!< 对应的时间段*/
    BYTE	Brightness;			/*!< 亮度	0-100		*/
    BYTE	Contrast;			/*!< 对比度	0-100		*/
    BYTE	Saturation;			/*!< 饱和度	0-100		*/
    BYTE	Hue;				/*!< 色度	0-100		*/
    BYTE	Gain;				/*!< 增益	0-100		*/
    BYTE	Reserve[3];
}COLOR_PARAM;

//! 颜色结构
typedef struct  
{
    BYTE ColorVersion[8];	
    COLOR_PARAM Color[N_COLOR_SECTION];
}CONFIG_COLOR_OLD;

//! 编码选项
typedef struct  
{
    BYTE ImageSize;   /*!< 分辨率 参照枚举capture_size_t(DVRAPI.H) */
    BYTE BitRateControl;  /*!< 码流控制 参照枚举capture_brc_t(DVRAPI.H) */
    BYTE ImgQlty;   /*!< 码流的画质 档次1-6  */
    BYTE Frames;    /*!< 帧率　档次N制1-6,P制1-5 */
    BYTE AVEnable;   /*!< 音视频使能 1位为视频，2位为音频。ON为打开，OFF为关闭 */
    BYTE IFrameInterval;  /*!< I帧间隔帧数量，描述两个I帧之间的P帧个数，0-149, 255表示此功能不支持设置*/
    WORD usBitRate;
}ENCODE_OPTION;

//! 标题结构
typedef struct  
{
    DWORD	TitlefgRGBA;			/*!< 标题的前景RGB，和透明度 */
    DWORD	TitlebgRGBA;		/*!< 标题的后景RGB，和透明度*/
    WORD	TitleLeft;			/*!< 标题距左边的距离与整长的比例*8192 */
    WORD	TitleTop;			/*!< 标题的上边的距离与整长的比例*8192 */
    WORD	TitleRight;			/*!< 标题的右边的距离与整长的比例*8192 */
    WORD	TitleBottom;			/*!< 标题的下边的距离与整长的比例*8192 */
    BYTE	TitleEnable;			/*!< 标题使能 */
    BYTE	Reserved[3];
}ENCODE_TITLE;

typedef struct {
    int left;
    int top;
    int right;
    int bottom;
}RECT,*PRECT;

//! 编码信息结构(双码流在用)
typedef struct {
    BYTE				CapVersion[8];				/*!< 版本号			*/		
    ENCODE_OPTION		MainOption[REC_TYP_NUM];	/*!< 主码流，REC_TYP_NUM不同录像类型*/
    ENCODE_OPTION		AssiOption[EXTRATYPES];	/*!< 支持3 路辅码流 */
    BYTE				Compression;				/*!< 压缩模式 */;	
    BYTE    			CoverEnable;				/*!< 区域遮盖开关　0x00不使能遮盖，0x01仅遮盖预览，0x10仅遮盖录像，0x11都遮盖	*/
    BYTE 				alignres[2];			/*!< 保留对齐用 */
    RECT				Cover;						/*!< 区域遮盖范围	*/	

    ENCODE_TITLE 		TimeTitle;					/*!< 时间标题*/
    ENCODE_TITLE 		ChannelTitle;				/*!< 通道标题*/

    ENCODE_OPTION		PicOption[2];				//定时抓图、触发抓图

    BYTE	Volume;				/*!< 保存音量的阀值 */
    BYTE    VolumeEnable;       /*!< 音量阀值使能*/
    BYTE	 Reserved[46];
}CONFIG_CAPTURE_OLD;

typedef struct tagPTZ_LINK
{
    int iType;				/*!< 联动的类型 */
    int iValue;				/*!< 联动的类型对应的值 */
}PTZ_LINK;


//! 事件处理结构
typedef struct tagEVENT_HANDLER
{
    unsigned long	dwRecord;
    int		iRecordLatch;
    unsigned long	dwTour;
    unsigned long	dwSnapShot;
    unsigned long	dwAlarmOut;
    int		iAOLatch;
    PTZ_LINK	PtzLink[16];

    int 	bRecordEn;
    int 	bTourEn;
    int 	bSnapEn;
    int 	bAlarmOutEn;
    int 	bPtzEn;
    int 	bTip;
    int 	bMail;
    int 	bMessage;
    int 	bBeep;
    int 	bVoice;
    int 	bFtp;

    unsigned long	dwWorkSheet;

    unsigned long	dwMatrix;				/*!< 矩阵掩码 */
    int 	bMatrixEn;				/*!< 矩阵使能 */
    int 	bLog;					/*!< 日志使能，目前只有在WTN动态检测中使用 */
    int		iEventLatch;			/*!< 联动开始延时时间，s为单位 范围是0－－15 默认值是0*/
    int 	bMessagetoNet;			/*!< 消息上传给网络使能 */
    unsigned long	wiAlarmOut; 			/*!< 无线报警输出 */
    unsigned char	bMMSEn;					/*!< 短信报警使能  */
    unsigned char	SnapshotTimes;          /*!< 抓图张数 */
    char	dReserved[22];			/*!< 保留字节 */
} EVENT_HANDLER, *LPEVENT_HANDLER;

//!动态检测设置
typedef struct tagCONFIG_MOTIONDETECT
{
    BOOL bEnable;					/*!< 动态检测开启 */
    int iLevel;						/*!< 灵敏度 */
    DWORD mRegion[MD_REGION_ROW];	/*!< 区域，每一行使用一个二进制串 */	
    EVENT_HANDLER hEvent;			/*!< 动态检测联动 */
} CONFIG_MOTIONDETECT;

typedef struct CAPTURE_DSPINFO 
{
    unsigned int	nMaxEncodePower;	///< DSP 支持的最高编码能力。
    unsigned short	nMaxSupportChannel;	///< DSP 支持最多输入视频通道数。
    unsigned short	bChannelMaxSetSync;	///< DSP 每通道的最大编码设置是否同步 0-不同步, 1 -同步。
    unsigned short	nExpandChannel;		///< DSP 支持的扩展通道数，主要是多路回放使用，目前只是一个
    unsigned short  rev;
}CAPTURE_DSPINFO, *PCAPTURE_DSPINFO;

typedef struct CAPTURE_ENCCHIPCAPABILITY
{
    int EncChipNR;
    CAPTURE_DSPINFO EncChipInfo[MAX_ENC_CHIP_NR];
}CAPTURE_ENCCHIPCAPABILITY,*PCAPTURE_ENCCHIPCAPABILITY;

#pragma pack(1)
typedef struct _CONFIG_TIME_
{
    char c[22];
}CONFIG_TIME;
#pragma pack()


#define ZHINUO_SET_BIT(addr, index)\
    (addr) |= (1 << ((index) - 1));

/// 摄像头控制结构
typedef struct cam_control{
    unsigned char  Exposure;        ///< 曝光速度等级，取值范围取决于设备能力集：0-自动曝光，1-曝光等级1，2-曝光等级2…n-最大曝光等级数
    unsigned char  Backlight;      /*< 背光补偿等级 取值范围取决于设备能力集，0-关闭，1-背光补偿强度1，
                                     2-背光补偿强度2…n-最大背光补偿等级数*/
    unsigned char AutoColor2BW;    ///< 自动彩黑转换 0-总是彩色 1- 自动 2-总是黑白
    unsigned char  Mirror;       //< 镜像  1 开， 0关
    unsigned char  Flip;             //< 翻转  1 开， 0关
    unsigned char   LensFunction;   //< 高4位表示功能能力，低4位表示功能开关
    //< bit4 = 1:支持自动光圈功能；
    //< bit0 = 0:关闭自动光圈；bit0 = 1:开启自动光圈；
    unsigned char  WhiteBalance;     //白平衡 0:Disabled,1:Auto 2:sunny 3:cloudy 4:home 5:office 6:night 
    unsigned char  SignalFormatMask;  //信号格式0-Inside(内部输入) 1- BT656 2-720p 3-1080i  4-1080p  5-1080sF
    unsigned char  iRev[120];  // 保留
}CAM_CONTROL;              //总长度128字节

typedef struct __cam_caps{
    unsigned char iBrightnessEn;  // 亮度 可调 1 可， 0不可
    unsigned char iContrastEn;        // 对比度 可调
    unsigned char iColorEn;       // 色度 可调
    unsigned char iGainEn;            // 增益 可调
    unsigned char iSaturationEn;   // 饱和度 可调
    unsigned char iBacklightEn;   // 背光补偿 支持的背光补偿等级数 0表示不支持背光补偿,1表示支持一级补偿（开,关），2表示支持两级补偿（关,高,低），3表示支持三级补偿（关,高,中,低）
    unsigned char iExposureEn;  // 曝光选择 支持的曝光速度等级数 0表示不支持曝光控制，1表示只支持自动曝光，其他表示支持手动控制的等级数+1（1 对应自动曝光）
    unsigned char iColorConvEn;   // 自动彩黑转换 可调
    unsigned char iAttrEn;            // 属性选项  1 可， 0不可
    unsigned char iMirrorEn;       // 镜像  1 支持， 0不支持
    unsigned char iFlipEn;             // 翻转   1 支持， 0不支持
    unsigned char iWhiteBalance;      //白平衡 0-不支持白平衡，1-支持自动白平衡，2-支持预置白平衡（即情景模式） 
    unsigned char iSignalFormatMask;  //信号格式掩码，按位从低到高位分别为：0-Inside(内部输入) 1- BT656 2-720p 3-1080i  4-1080p  5-1080sF
    unsigned char iRev[123];      // 保留
}CAM_CAPS;


#define N_WEEKS 7
#define N_TSECT 6

typedef struct tagTIMESECTION
{
    //!使能
    int enable;
    //!开始时间:小时
    int startHour;
    //!开始时间:分钟
    int startMinute;
    //!开始时间:秒钟
    int startSecond;
    //!结束时间:小时
    int endHour;
    //!结束时间:分钟
    int endMinute;
    //!结束时间:秒钟
    int endSecond;
}TIMESECTION;   


typedef struct tagWORKSHEET
{
    int         iName;                          /*!< 时间表名称 */  
    TIMESECTION tsSchedule[N_WEEKS][N_TSECT];   /*!< 时间段 */
}CONFIG_WORKSHEET;
//使能位的定义：
#define REC_SYS                 0x0001      /*!< 系统预录事件   */
#define REC_TIM                 0x0002      /*!< 定时录像事件   */
#define REC_MTD                 0x0004      /*!< 动检录像事件   */
#define REC_ALM                 0x0008      /*!< 报警录像事件   */
#define REC_CRD                 0x0010      /*!< 卡号录像事件   */
#define REC_MAN                 0x0020      /*!< 手动录像事件   */
#define SNAP_TIM                0x20000     /*!< 定时抓图事件   */
#define SNAP_MTD                0x40000     /*!< 动检抓图事件   */
#define SNAP_ALM                0x80000     /*!< 报警抓图事件   */

//移动侦测信息 
typedef struct 
{
    DWORD dwVersion;
    BYTE dbReserve[4];
}VER_CONFIG;

typedef struct {
    VER_CONFIG sVerConfig;              /*! 配置文件版本信息 */
    DWORD   MotionRecordMask;           /*! 录象掩码 */
    DWORD   MotionAlarmMask;            /*! 报警输出: 按位表示报警通道 */
    DWORD   MotionTimeDelay;            /*! 延时：10～300 sec */
    DWORD   MotionRegion[18];           /*! 区域 */
    DWORD   BlindAlarmMask;             /*! 黑屏检测: 按位表示报警通道 */
    DWORD   LossAlarmMask;              /*! 丢失检测: 按位表示报警通道 */
    BYTE    MotionLever;                /*! 灵敏度：1～6 */
    BYTE    dbReserved[2];              /*! 保留字段 */
    BYTE    BlindTipEn;                 /*! 黑屏检测提示使能，0:不提示1:提示 */
    BYTE    LossTipEn;                  /*! 丢失检测提示使能，0:不提示1:提示*/
    BYTE    Mailer;                     /*! 发送邮件掩码（1-8位表示最多8通道的图像） */
    BYTE    SendAlarmMsg;               /*! 发送报警信息（给报警服务器等） */
    BYTE    adbReserved[2];             /*! 保留字段 */
} CONFIG_DETECT;

typedef struct tagMOTION_DETECT_CAPS
{
    DWORD Enabled;       // 置1表示支持动态检测，置0表示不支持动态检测。
    DWORD GridLines;     // 动态检测的区域需要划分成的列数。
    DWORD GridRows;      // 动态检测的区域需要划分成的行数。
    BYTE  Result;        // 是否能得到每块区域的检测结果。
    BYTE  Hint;          // 是否能在检测成功时给予提示。
}MOTION_DETECT_CAPS;


//事件
#define DMS_MD_STRIDE_WIDTH     44
#define DMS_MD_STRIDE_HEIGHT    36
#define DMS_MD_STRIDE_SIZE        (DMS_MD_STRIDE_WIDTH*DMS_MD_STRIDE_HEIGHT/8)


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


#define DMS_MAX_ALARMOUT 			8
#define DMS_MAX_CHANNUM 			32
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

//osd 覆盖
typedef struct __video_cover_t
{
    unsigned char iChannel;     // 通道号，[1~n]
    unsigned char iCoverNum;    // 覆盖的区域个数
    unsigned char unknown1;
    unsigned char unknown2;
    unsigned char unknown3;
    unsigned char unknown4;
    unsigned char iRev[26];     // 保留
}VIDEO_COVER_T;

typedef struct __video_cover_attr_t
{
    RECT tBlock;    // 覆盖的区域坐标
    unsigned char Color;      // 覆盖的颜色
    unsigned char iBlockType;  // 覆盖方式：0-黑块，1-马赛克
    unsigned char Encode;     //! 编码级遮挡, 1—生效, 0—不生郊
    unsigned char Priview;    //!预览遮挡, 1—生效, 0—不生效
    unsigned char iRev[32];    // 保留
}VIDEO_COVER_ATTR_T;

typedef struct __video_cover_caps_t
{
    unsigned char iEnabled;         // 置1表示支持 区域遮挡，0 不支持
    unsigned char iBlockNum;        // 支持的遮挡块数
    unsigned char iRev[62];         // 动态检测的区域需要划分成的行数。
}VIDEO_COVER_CAPS_T;

//extern pid_t gettid();	//system_msg.c中已有定义
//extern int errno;

#define DAHUA_MSG_SEND_BUF_SIZE (10*1024)


typedef enum{
    UNINIT_SESSION = 0,
    CMD_SESSION,
    CONNECT_SESSION,
} DAHUA_SESSION_TYPE;

typedef struct {
	int fd; //accept_sock;
	int fSessionInt;
    char c_ip[16];
    DAHUA_SESSION_TYPE fSessionType;
	char fSendbuf[DAHUA_MSG_SEND_BUF_SIZE];
    unsigned int nSendLen;

    pthread_mutex_t send_mutex; //cmd_resq_send_mutex
} DAHUA_SESSION_CTRL;	

typedef struct {
    int msg_seq;
    int v_seq;
    int v_timestmp;
    int a_seq;
    int a_timestmp;
    unsigned char ch_frame_type;
    unsigned char ch_width;
    unsigned char ch_height;
    unsigned char ch_fps;
    int frame_len;
    char *pData;
} DahuaFrameInfo;

typedef struct {
    unsigned int transaction_id;
    char method[100];
    char parameter_name[1000];
    unsigned int sid;
    unsigned int connect_id;
    unsigned int connect_protocol;
    int channel;
    int stream;
    int state;
} DahuaMethodField;

typedef struct {
    int enable;
    int brightness;
    int chromaSuppress;
    int contrast;
    int gamma;
    int hue;
    int saturation;
    int sharpness;
    int day_night_color;
    int flip;
    int mirror;
    int nSwitchMode;
} DahuaColorProfile;


typedef struct {
    int sock;
    char ip[16];
} DahuaClientInfo;


typedef	void *(*ThreadEntryPtrType)(void *);


#endif
