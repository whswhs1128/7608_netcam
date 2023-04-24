/**
 * goolink_apiv2.h
 *
 * External APIs for device side implementation.
 *
 * Created by Wang  2017-02-07
 *
 * utf-8
 */

#ifndef __GOO_LINK_APIV2_H__
#define __GOO_LINK_APIV2_H__

#ifdef __cplusplus
extern "C" {
#endif


#include<stdint.h>
#include<inttypes.h>

/*****************************************************************************
 							数据长度宏
 *****************************************************************************/
#define GLNKPACKET_STR_LEN_64						64
#define GLNKPACKET_STR_LEN_32						32
#define GLNKPACKET_STR_LEN_16						16

/*****************************************************************************
 							音频格式
 *****************************************************************************/
//G711a wave format
#ifndef GLNK_WAVE_FORMAT_G711
#define GLNK_WAVE_FORMAT_G711						0x7A19
#endif
//G711U wave format
#ifndef GLNK_WAVE_FORMAT_G711U
#define GLNK_WAVE_FORMAT_G711U                   	0x7A25
#endif
//AAC  wave format
#ifndef WAVE_FORMAT_AAC
#define WAVE_FORMAT_AAC                   			0x7A26
#endif

/*****************************************************************************
 							视频格式
 *****************************************************************************/
//H264 wave format
#define GLNK_CODEC_H264								0x34363248
//mjpeg
#define CODEC_MJPEG									0x45464758
//H265
#define GLNK_CODEC_H265								0x56565268

//G726 wave format
#ifndef GLNK_WAVE_FORMAT_G726
#define GLNK_WAVE_FORMAT_G726						0x7A20
#endif

//AMR_NB CBR wave format
#ifndef GLNK_WAVE_FORMAT_AMR_CBR
#define GLNK_WAVE_FORMAT_AMR_CBR 					0x7A21
#endif

//AMR VBR Not support yet
#ifndef GLNK_WAVE_FORMAT_AMR_VBR
#define GLNK_WAVE_FORMAT_AMR_VBR 					0x7A22
#endif

//SPEEX nb mode wave format
#ifndef GLNK_WAVE_FORMAT_SPEEX_NB_MODE
#define GLNK_WAVE_FORMAT_SPEEX_NB_MODE           	0x7A23
#endif

//SPEEX wb mode wave format
#ifndef GLNK_WAVE_FORMAT_SPEEX_WB_MODE
#define GLNK_WAVE_FORMAT_SPEEX_WB_MODE           	0x7A24
#endif

//AMR_WB Wave format
#ifndef GLNK_WAVE_FORMAT_VOICEAGE_AMR_WB
#define GLNK_WAVE_FORMAT_VOICEAGE_AMR_WB			0xA104
#endif

/*****************************************************************************
 							错误码及一些枚举
 *****************************************************************************/
#define GLNK_ERR_NOERR             0       // No error
#define GLNK_ERR_MUTEX             -0x101  // Mutex creation error
#define GLNK_ERR_THREAD            -0x102  // Thread creation error
#define GLNK_ERR_SOCKET            -0x103  // Socket creation error
#define GLNK_ERR_SOCK_OPT          -0x104  // Socket option setting error
#define GLNK_ERR_SOCK_BIND         -0x105  // Socket bind error
#define GLNK_ERR_TIMEOUT           -0x106  // timeout error
#define GLNK_ERR_NOMEM             -0x107  // memory error
#define GLNK_ERR_PARAM             -0x108  // invalid param
#define GLNK_ERR_IVALSID           -0x109  // invalid session id
#define GLNK_ERR_NAT_TRAVERSAL     -0x10A  // NAT traversal failure
#define GLNK_ERR_PROTOCOL     		-0x10B  //
#define GLNK_ERR_MAX               -0x200  //
#define GLNK_ERR_BUFNOTEXIST       -0x202  // buffer not exist
#define GLNK_ERR_NOTSTART          -0x203  // goolink module not start
#define GLNK_ERR_CLOSING           -0x204  // goolink module is closing
#define GLNK_ERR_NOENOUTGMEM       -0X205  // not enough memery
#define GLNK_ERR_APINOTSUPPORT     -0x206  // in the case the api is not support
#define GLNK_ERR_BUSY              -0x205  // busy

/*		文件下载类型			*/
typedef enum _OpenDownLoadFIleType
{
	NORMALDOWNLOAD = 1,							// 正常下载
	BREAKPOINTDOWNLOAD,							// 断点下载
	ENDDOWNLOAD,								// 结束下载
}OpenDownLoadFIleType;

/*		云储存上传图片返回结果  */
typedef enum _UPLOADDATARESULT
{
	UPLOADDATASUC 					= 1,		// 上传成功
	UPLOADDATAFAIL_NETWORK			= 2,		// 网络问题导致上传失败
	UPLOADDATAFAIL_COMPETENCE		= 3,		// 没有权限导致上传失败
	UPLOADDATAFAIL_UPLOADERROR		= 4,		// 上传中途出错导致上传失败
	UPLOADDATAFAIL_MODULESTOP		= 5,		// 云存储模块没启动
	UPLOADDATAFAIL_DATAUPLOADING	= 6,		// 已经有数据正在上传导致失败
	UPLOADDATAFAIL_NOSTREAMUPLOAD	= 7,		// 没有上传视频码流不能上传图片
}UPLOADDATARESULT;


/*		鱼眼接口类型宏			*/
enum SwitchType
{
	MUSIC_SWITCH = 1, 							// 音乐开关	对应SwtichOCValue
	FEEDING_SWITCH,  							// 喂食开关	对应SwtichOCValue
	FISHEYE_SWITCH,   							// 鱼眼控制	对应FishEyeModeValue
};

enum SwtichOCValue
{
	CLOSE_VALUE = 0,
	OPEN_VALUE = 1,
};

enum FishEyeModeValue
{
	//吸顶模式
	CEILINGMODE_DEFAULT = 0x10,					// 默认模式
	CEILINGMODE_360PANORAMIC = 0x11,			// 360度全景
	CEILINGMODE_QUAD = 0x12, 					// 四分割
	CEILINGMODE_PTZ = 0x13, 					// PTZ
	//桌面放置
	DESKTOPMODE_DEFAULT = 0x20,					// 默认模式
	DESKTOPMODE_360PANORAMIC = 0x21,			// 360度全景
	DESKTOPMODE_QUAD = 0X22, 					// 四分割
	DESKTOPMODE_PTZ = 0X23,						// PTZ
	//壁装模式
	WALLMODE_DEFAULT = 0X30, 					// 默认模式
	WALLMODE_180HORIZONTALLY = 0x31,			// 水平180度
	WALLMODE_QUAD = 0x32,						// 四分割
	WALLMODE_PTZ  = 0x33,						// PTZ
};

/*		视频分辨率宏	*/
typedef enum _OWSP_VIDEO_RESOLUTION
{
	RESOLUTION_1080P = 0,
	RESOLUTION_720P,
	RESOLUTION_D1,
	RESOLUTION_VGA,
	RESOLUTION_CIF,
	RESOLUTION_QVGA,
	RESOLUTION_QCIF,
	RESOLUTION_QQVGA
}OWSP_VIDEO_RESOLUTION;

/*****************************************************************************
 							其他
 *****************************************************************************/
/*		日期时间定义		*/
typedef struct _GLNK_DateTime
{
	uint32_t		m_microsecond;				// 毫秒	0-1000
	uint32_t 		m_year;						// 年,2009
	uint32_t		m_month;					// 月,1-12
	uint32_t		m_day;						// 日,1-31
	uint32_t		m_hour;						// 0-24
	uint32_t		m_minute;					// 0-59
	uint32_t		m_second;					// 0-59
} GLNK_DateTime;

typedef struct _GLNK_V_AudioInfo
{
	unsigned char  channelId;
	unsigned char  reserve;
	unsigned short checksum;
	unsigned int time;
}GLNK_V_AudioInfo;

typedef struct _GLNK_V_RecordChgRequest
{
    uint32_t  		command;
    GLNK_DateTime   startTime;
    GLNK_DateTime   endTime;
}GLNK_V_RecordChgRequest;
/*
 *	TLV: TLV_T_VA_GET_CONFIG_RSP
 *	Direction: Deivce -> Client
 *	Description:
 *		channel: the channel of
 */
typedef struct _TLV_V_VA_GET_CONFIG_RSP
{
	unsigned char 	channel;
	unsigned char  isOpenAudio;
	unsigned char	reverse[2];
	OWSP_VIDEO_RESOLUTION mainStreamResolution;
	unsigned int	mainStreamFrameRate;
	unsigned int mainStreamBitRate;
	OWSP_VIDEO_RESOLUTION subStreamResolution;
	unsigned int subStreamFrameRate;
	unsigned int subStreamBitRate;
}TLV_V_VA_GET_CONFIG_RSP;
/*
 *	TLV: TLV_T_VA_SET_CONFIG_REQ
 *	Direction: Client -> Device
 *	Description:
 *
 */
typedef struct _TLV_V_VA_SET_CONFIG_REQ
{
	unsigned char 	channel;
	unsigned char  isOpenAudio;
	unsigned char	reverse[2];
	OWSP_VIDEO_RESOLUTION mainStreamResolution;
	unsigned int	mainStreamFrameRate;
	unsigned int mainStreamBitRate;

	OWSP_VIDEO_RESOLUTION subStreamResolution;
	unsigned int subStreamFrameRate;
	unsigned int subStreamBitRate;
}TLV_V_VA_SET_CONFIG_REQ;

typedef struct _TLV_V_Network
{
	unsigned int deviceId;
	unsigned char  hostIP[4];
	unsigned char    hostName[32];
	unsigned char  gateway[4];
	unsigned char  dnsServer[4];
	unsigned char  dnsServer2[4];
	unsigned char  subnetMask[4];

	unsigned char  wifiHostIP[4];
	unsigned char  wifiHostName[32];
	unsigned char  wifiGateway[4];
	unsigned char  wifiDnsServer[4];
	unsigned char  wifiDnsServer2[4];
	unsigned char  wifiSubnetMask[4];
	unsigned char  wifiMac[8];
	unsigned char  mac[8];
	unsigned char  wifiIPMode;
	unsigned char  IPMode;
	unsigned char  reverse[2];
} TLV_V_Network;

typedef enum _OWSP_StreamType
{
	OWSP_STREAM_MAIN	= 0,        //主码流
	OWSP_STREAM_SUB		= 1,        //次码流
	OWSP_STREAM_VOD		= 2,
	OWSP_MODE_SETTING	= 3,
	OWSP_VIEWMODE_LLD   = 4,				//very Low Display resolution  eg:QQVGA
	OWSP_VIEWMODE_LD    = 5,				//Low Display resolution	  eg:QVGA QCIF
	OWSP_VIEWMODE_SD    = 6,				//Stand Display resolution	eg:VGA D1
	OWSP_VIEWMODE_HD    = 7				//High Display resolution	  eg:720p 1080p
} OWSP_StreamType;

typedef struct _GLNK_V_VideoModeRequest
{
	uint32_t deviceId;
	uint16_t channel;
	uint16_t reserve;
	OWSP_StreamType mode;
}GLNK_V_ViewModeRequest;


/*		搜索音乐类型			*/
typedef struct _GLNK_V_SearchAudioFileRequest
{
	uint32_t  AudioType;      					// 音乐类型掩码 0x01 = 流行音乐, 0x02 = 爵士音频, 0x04 = 摇滚音乐, 0x08 = 古典音乐， 0xFF = 全部音乐*/
} GLNK_V_SearchAudioFileRequest;

/*		搜索音乐结构			*/
typedef struct _GLNK_V_AudioFileInfo
{
	char	       fileName[128];   			// 文件名字
	uint32_t       length;        				// 音乐时间：单位秒
	uint8_t        AudioType;     				// 音乐类型 /* AudioType : 音乐类型掩码 0x01 = 流行音乐, 0x02 = 爵士音频, 0x04 = 摇滚音乐, 0x08 = 古典音乐， 0xFF = 全部音乐*/
	uint8_t        reserve[3];
}GLNK_V_AudioFileInfo;


typedef struct _GLNK_V_AlarmInfo{
	uint64_t AlarmTime;							//报警时间单位：秒（s）  1970-1-1 0:0:0 算起到报警时间经过的秒数
    int32_t  AlarmType;							//报警类型
    int32_t  Channel;							//报警通道
}GLNK_V_AlarmInfo;

typedef enum _SendVodStreamType
{
	SendFileType = 1,
	SendStreamType = 2,
} SendVodStreamType;

typedef enum _UPDATECTRL
{
	UPDATECTRL_ENABLE = 1,
	UPDATECTRL_DISABLE,
}UPDATECTRL;

/*****************************************************************************
 							初始化结构
 *****************************************************************************/
typedef struct InitNetParam
{
	char 				dev[8];						//网卡名称（重要）
	uint16_t 			localTCPport;				//直连tcp端口
	uint16_t 			localUDPport;				//直连udp端口
	char 				udid[32];					//goolink id
	unsigned char		channelnum;					//通道数 1-64
	unsigned char 		issupportsubvideostream;	//是否支持视频子码流0-表示不支持，1表示支持
	unsigned char		maxsession;					//设备最大同时连接数
	char 				reserve;
}InitNetParam;

/*****************************************************************************
 							日期定义
 *****************************************************************************/
typedef struct _GooTime
{
    uint16_t  year;    						// YYYY, eg. 2009
    uint16_t  month;   						// MM, 1-12
    uint8_t   day;     						// DD, 1-31
    uint8_t   hour;    						// HH, 0-24
    uint8_t   minute;  						// MM, 0-59
    uint8_t   second;  						// SS, 0-59
} GooTime;

typedef struct _GLNK_Date
{
	uint16_t year;		       				// YYYY, 2012
	uint8_t month;		       				// MM, 1-12
	uint8_t day;			   				// DD, 01-31
} GLNK_Date;

/*****************************************************************************
 							报警接口结构
 *****************************************************************************/
typedef struct _PushAlarm
{
    short   	alarm_type;					// 报警类型，见下
    short   	channel;					// 通道号
    GooTime   	timestamp;					// 报警时间
	char 		CustomBuf[64];				// 报警文本（不用填）
} PushAlarm;

/*		报警类型		*/
typedef enum _PushAlarmType
{
	PAT_VIDEO_FRAME    			= 0,       	// IP地址冲突报警	(禁用，没什么用)
	PAT_DEVICE_RESTART 			= 1,       	// 设备重启
	PAT_MOTION_DETECT  			= 2,       	// 移动侦测报警
	PAT_VIDEO_LOSS    			= 3,       	// 视频丢失
	PAT_VIDEO_SHEILD      		= 4,       	// 视频遮挡报警
	PAT_BORDER_DETECT   		= 5,       	// 越界侦测报警
	PAT_ZONE_INTRUSION       	= 6,       	// 区域入侵报警
	PAT_SWITCH_SENSOR  			= 7,       	// 开关量探头报警
	PAT_PIR_ALARM      			= 8,       	// 红外报警
	PAT_SMOKE_ALARM   			= 9,       	// 烟雾报警
	PAT_NOISE_ALARM    			= 10,      	// 噪声报警
	PAT_TEMPERATURE_ALARM    	= 11,      	// 温度异常报警
	PAT_HUMIDITY_ALARM          = 12,      	// 湿度异常报警
	PAT_GAS_ALARM       		= 13,      	// 气体报警
	PAT_CALLING_ALARM  			= 14,	  	// 门铃呼叫
	PAT_SDCARDERROR_ALARM		= 15,		// SD卡/硬盘异常报警
	PAT_SDCARDOUT_ALARM 		= 16, 		// SD卡/硬盘拔出报警
	PAT_SDCARDFULL_ALARM 		= 17, 		// SD卡/硬盘容量满报警
	PAT_DEVICEMOVE_ALARM 		= 18, 		// 设备移动报警
	PAT_ENERGYREMOVE_ALARM 		= 19, 		// 电源拔出报警
	PAT_EXTERNALPOWER_ALARM 	= 20, 		// 外部电源输入报警
	PAT_LOWPOWER_ALARM 			= 21, 		// 低电量报警
	PAT_IO_ALARM 				= 22,  		// I/O报警
	PAT_RINGING_ALARM			= 25,		// 有人呼叫您
	PAT_CALLANSWERED_ALARM		= 26,		// 呼叫已接听
	PAT_DOORBELL_ALAEM			= 100,  	// 门口机来电
} PushAlarmType;

/*     433报警接口结构   */
typedef struct _Push433Alarm
{
	char 		AddrCode[20];				// 433设备地址码
	char 		Mcode;						// 键值
	int32_t 	Tcode;						// 设备类型
	GooTime   	timestamp;					// 报警时间
}Push433Alarm;

/*****************************************************************************
 							编码接口结构
 *****************************************************************************/
/*		视频编码接口	*/
typedef struct _GLNK_VideoDataFormat
{
	unsigned int codec;					// 编码方式
	unsigned int bitrate;        			// 比特率, bps
	unsigned short width;					// 图像宽度
	unsigned short height;					// 图像高度
	unsigned char framerate;				// 帧率, fps
	unsigned char colorDepth;				// should be 24 bits
	unsigned char frameInterval;   		// I帧间隔(s)
	unsigned char reserve;
}GLNK_VideoDataFormat;

/*		音频编码接口	*/
typedef struct _GLNK_AudioDataFormat
{
	unsigned int samplesRate;				// 每秒采样
	unsigned int bitrate;					// 比特率, bps
	unsigned short waveFormat;				// 编码格式
	unsigned short channelNumber;			// 音频通道 1单通道 2双通道一般为单通道。
	unsigned short blockAlign;				// 块对齐, channelSize * (bitsSample/8)
	unsigned short bitsPerSample;			// 每采样比特数
	unsigned short frameInterval;			// 帧间隔, 单位ms
	unsigned short reserve;
} GLNK_AudioDataFormat;

/*****************************************************************************
 							设备信息接口
 *****************************************************************************/
typedef struct _GLNK_V_DeviceInfo
{
	char companyId[GLNKPACKET_STR_LEN_16];			// 公司id
	char productId[GLNKPACKET_STR_LEN_16];			// 产品描述
	char name[GLNKPACKET_STR_LEN_16];				// 产品名称
	char softwareVersion[GLNKPACKET_STR_LEN_16];	// 软件版本
	GLNK_Date	manufactureDate;					// 固件日期
	unsigned char channelNum;						// 通道数
	unsigned char alarmType;						// 报警种类
	unsigned char deviceType;						// 设备类型
	unsigned char reserve2;
} GLNK_V_DeviceInfo;

/*****************************************************************************
 							云台接口
 *****************************************************************************/
typedef struct _GLNK_V_PTZControlRequest
{
	unsigned int deviceId;					// 设备ID
	unsigned char  channel;				// 通道号
	unsigned char  cmdCode;				// 控制命令字, 参见GLNK_PTZControlCmd
	unsigned short size;					// 控制参数数据长度,如果size==0 表示无控制参数
} GLNK_V_PTZControlRequest;

// ControlArgData
//   GLNK_PTZ_MV_STOP   	：无
//   GLNK_PTZ_ZOOM_DEC  	：arg1, 步长
//   GLNK_PTZ_ZOOM_INC  	：arg1, 步长
//   GLNK_PTZ_FOCUS_INC 	：arg1, 步长
//   GLNK_PTZ_FOCUS_DEC 	：arg1, 步长
//   GLNK_PTZ_MV_UP     	：arg1, 水平速度; arg2, 垂直速度; arg3, 步长
//   GLNK_PTZ_MV_DOWN   	：arg1, 水平速度; arg2, 垂直速度; arg3, 步长
//   GLNK_PTZ_MV_LEFT   	：arg1, 水平速度; arg2, 垂直速度; arg3, 步长
//   GLNK_PTZ_MV_RIGHT  	：arg1, 水平速度; arg2, 垂直速度; arg3, 步长
//   GLNK_PTZ_IRIS_INC  	：arg1, 步长
//   GLNK_PTZ_IRIS_DEC 		：arg1, 步长
//   GLNK_PTZ_AUTO_CRUISE  : arg1, 1 = 开始巡航, 0 = 停止巡航; arg2, 水平速度; arg3, 垂直速度
//   GLNK_PTZ_GOTO_PRESET  : arg1, 预置点编号
//   GLNK_PTZ_SET_PRESET   : arg1, 预置点编号
//   GLNK_PTZ_CLEAR_PRESET : arg1, 预置点编号, 如果为0xFFFFFFFF标识清除全部
//   GLNK_PTZ_ACTION_RESET
//   GLNK_PTZ_MV_LEFTUP    ：arg1, 水平速度; arg2, 垂直速度; arg3, 步长
//   GLNK_PTZ_MV_LEFTDOWN  ：arg1, 水平速度; arg2, 垂直速度; arg3, 步长
//   GLNK_PTZ_MV_RIGHTUP   ：arg1, 水平速度; arg2, 垂直速度; arg3, 步长
//   GLNK_PTZ_MV_RIGHTDOWN ：arg1, 水平速度; arg2, 垂直速度; arg3, 步长
//   GLNK_PTZ_CLEAR_TOUR   ：arg1, 线路编号
//   GLNK_PTZ_ADD_PRESET_TO_TOUR : arg1, 预置点编号; arg2, 线路编号
//   GLNK_PTZ_DEL_PRESET_TO_TOUR : arg1, 预置点编号; arg2, 线路编号

typedef enum _GLNK_PTZControlCmd
{
	GLNK_PTZ_MV_STOP      = 0,    	//停止运动
	GLNK_PTZ_ZOOM_DEC     = 5,		//放大
	GLNK_PTZ_ZOOM_INC     = 6,		//缩小
	GLNK_PTZ_FOCUS_INC    = 7,    	//焦距放大
	GLNK_PTZ_FOCUS_DEC    = 8,		//焦距缩小
	GLNK_PTZ_MV_UP        = 9,    	//向上
	GLNK_PTZ_MV_DOWN      = 10,   	//向下
	GLNK_PTZ_MV_LEFT      = 11,   	//向左
	GLNK_PTZ_MV_RIGHT     = 12,   	//向右
	GLNK_PTZ_IRIS_INC     = 13,   	//光圈放大
	GLNK_PTZ_IRIS_DEC     = 14,   	//光圈缩小
	GLNK_PTZ_AUTO_CRUISE  = 15,	  	//自动巡航
	GLNK_PTZ_GOTO_PRESET  = 16,   	//跳转预置位
	GLNK_PTZ_SET_PRESET   = 17,   	//设置预置位点; 255: 锁焦, 254:复位
	GLNK_PTZ_CLEAR_PRESET = 18,   	//清除预置位点
	GLNK_PTZ_ACTION_RESET = 20,   	//PTZ复位
	GLNK_PTZ_MV_LEFTUP    = 21,
	GLNK_PTZ_MV_LEFTDOWN  = 22,
	GLNK_PTZ_MV_RIGHTUP   = 23,
	GLNK_PTZ_MV_RIGHTDOWN = 24,
	GLNK_PTZ_CLEAR_TOUR   = 25,
	GLNK_PTZ_ADD_PRESET_TO_TOUR  = 26,
	GLNK_PTZ_DEL_PRESET_TO_TOUR  = 27
} GLNK_PTZControlCmd;

/*		云台控制接口		*/
typedef struct _GLNK_ControlArgData
{
	unsigned int arg1;
	unsigned int arg2;
	unsigned int arg3;
	unsigned int arg4;
} GLNK_ControlArgData;

/*****************************************************************************
 							录像接口
 *****************************************************************************/
/*		录像搜索结构		*/
typedef struct _GLNK_V_SearchFileRequest
{
	uint32_t 		deviceId;					// 设备编号（默认为0）
	uint32_t		channelMask;				// 低通道掩码1-32通道, 需要搜索哪些通道的录像, 将该位置1. 0xFFFF FFFF表示所有通道
	GLNK_DateTime 	startTime;					// 查询起始时间
	GLNK_DateTime 	endTime;					// 查询终止时间
	uint32_t  		recordTypeMask;				// 录像类型掩码 0x01 = 开关量告警录像, 0x02 = 移动侦测录像, 0x04 = 常规录像, 0x08 = 手动录像， 0xFF = 全部录像
	uint32_t  		index;						// 文件索引, 设为0
	uint32_t		count;						// 返回数据个数, 默认为10
	uint32_t  		channelMask2;				// 高通道掩码33-64通道
} GLNK_V_SearchFileRequest;

/*		录像搜索结构		*/
typedef struct _GLNK_V_FileInfo
{
	char	        fileName[260];				// 文件名, 包括路径
	uint32_t       	deviceId;					// 设备ID（默认为0）
	uint32_t       	length;						// 文件总长度
	uint32_t       	frames;						// 总帧数
	GLNK_DateTime	startTime;					// 开始时间
	GLNK_DateTime	endTime;					// 结束时间
	uint8_t	      	channel;					// 录像通道号
	uint8_t        	recordType;					// 录像类型掩码 0x01 = 开关量告警录像, 0x02 = 移动侦测录像,0x04 = 常规录像, 0x08 = 手动录像
	uint8_t        	reserve[2];
} GLNK_V_FileInfo;

/*****************************************************************************
			 	 	 	 	 新录像接口
*****************************************************************************/
typedef struct _GLNK_SearchPlaybackRequest
{
	uint32_t  		deviceId;			//设备id，默认是0
	uint32_t  		channelMask;		//低通道掩码，0x88表示1000 1000表示通道4和通道8，共可搜索1-32个通道
	uint32_t  		channelMask2;		//高通道掩码，可搜索33-64个通道
	GLNK_DateTime  	startTime;			//搜索起始时间
	GLNK_DateTime 	endTime;			//搜索结束时间
	uint32_t  		recordTypeMask;		//录像类型掩码 0x01 = 开关量告警录像, 0x02 = 移动侦测录像, 0x04 = 常规录像, 0x08 = 手动录像， 0xFF = 全部录像
	uint32_t  		reserve;			//保留位
} GLNK_SearchPlaybackRequest;

typedef struct _GLNK_SearchPlaybackReply
{
	uint8_t       	recordType;			//录像类型掩码。0x01 = 开关量告警录像, 0x02 = 移动侦测录像, 0x04 = 常规录像, 0x08 = 手动录像
	uint8_t	      	channel; 			//通道号
	uint32_t       	deviceId;			//设备id，默认是0
	GLNK_DateTime	startTime;			//录像起始时间
	GLNK_DateTime	endTime;			//录像结束时间
	uint32_t        reserve;			//保留位
} GLNK_SearchPlaybackReply;

/*		录像配置说明		*/
typedef enum _ContrlSendVodStreamType
{
	ResumeSendStream 		= 0,				// 继续发送
	PauseSendStream 		= 1,				// 暂停发送
	StopSendStream 			= 2,				// 停止发送
	PlusSendStream 			= 3,				// 快放
	MinusSendStream 		= 4,				// 慢放
	JumpFrameSendStream 	= 5					// 拖动播放
} ContrlSendVodStreamType;

/*		录像类型说明		*/
typedef enum _VodStreamType
{
	VideoStream 			= 0,				// 视频
	AudioStream 			= 1,				// 音频
	VideoAndAudioStream 	= 2					// 视频和音频
} VodStreamType;

/*****************************************************************************
			 	 	 	 	 ap接口
*****************************************************************************/
/*		wifi信号强度		*/
typedef enum _GLNK_WifiSignalLevel
{
	GLNK_Signal_Strong = 0,						// 强
	GLNK_Signal_Mid,							// 中
	GLNK_Signal_Weak							// 差
}GLNK_WifiSignalLevel;

/*		设备搜索wifi名字		*/
typedef struct _GLNK_V_WifiInfo
{
	char name[GLNKPACKET_STR_LEN_32];			// goolink id
	char ssid[GLNKPACKET_STR_LEN_32];			// ssid
	GLNK_WifiSignalLevel level;					// wifi信号强度
}GLNK_V_WifiInfo;

/*		app返回wifi配置信息		*/
typedef struct _GLNK_V_WifiConfigRequest
{
	char name[GLNKPACKET_STR_LEN_32];			// goolink id
	char ssid[GLNKPACKET_STR_LEN_32];			// ssid
	char password[GLNKPACKET_STR_LEN_32];		// 密码
	uint32_t networkType;						//
	uint32_t encryptType;						// 加密类型
}GLNK_V_WifiConfigRequest;

/*		 AP配置新协议 			*/
typedef struct _GLNK_SSIDList
{
	char SSID[32];
	GLNK_WifiSignalLevel SignalLevel;				//信号强度 0=强，1=中，2=弱
	int32_t reverse;								//保留位
}GLNK_SSIDList;

typedef struct _GLNK_GetSSIDListResponse
{
	int32_t SSIDListLen;							//列表的长度=sizeof(GLNK_GetSSIDListResponset) + sizeof(GLNK_SSIDList)*ListNum,num为列表的个数
	GLNK_SSIDList SSIDList[0];						//wifi列表变长结构体
}GLNK_GetSSIDListResponset;

/*****************************************************************************
			 	 	 	 	 用户列表设置
*****************************************************************************/
/* 		用户密码定义 			*/
typedef struct _GLNK_V_UserPWD
{
	char username[32];							// 1-32个字符
	char pwd[16]; 								// 1-16个字符
  uint32_t reserve;								// 保留字节
}GLNK_V_UserPWD;

/*		用户列表数据结构		*/
typedef struct _GLNK_V_SetUserListRequest
{
	GLNK_V_UserPWD  user[8];					//用户列表
	uint8_t  		reserve[8];					// 保留字节
}GLNK_V_SetUserListRequest;

/*		用户列表数据结构		*/
typedef struct _GLNK_V_GetUserListResponse
{
	GLNK_V_UserPWD  user[8];					//用户列表
	uint8_t  reserve[8];						//保留字节
}GLNK_V_GetUserListResponse;

/*****************************************************************************
			 	 	 	 	 获取设备硬盘(sd卡)容量的接口
*****************************************************************************/
typedef struct _GLNK_DeviceStorageList
{
	int32_t StorageID;								//硬盘(sd卡)ID	第一块是1，第二块是2依次类推
	int32_t StorageCap;								//总存储容量(MB)
	int32_t StorageCapRemain;						//剩余容量(MB)
	int32_t reverse;								//保留位
}GLNK_DeviceStorageList;

typedef struct _GLNK_DeviceStorageResponse
{
	int32_t DeviceStorageListLen;							//列表的长度=sizeof(GLNK_DeviceStorageResponset) + sizeof(GLNK_DeviceStorageList)*ListNum,num为硬盘或sd卡的个数
	GLNK_DeviceStorageList DeviceStorageList[0];			//硬盘(sd卡)列表变长结构体
}GLNK_DeviceStorageResponse;


/*****************************************************************************
			 	 	 	 	切屏报警协议
 *****************************************************************************
	如下示屏幕格数顺序为书写顺序，切为4分屏，1屏 4屏侦测报警，则
	GLNK_SetScreenAlarmRequest->ScreenListLen = sizeof(GLNK_SetScreenAlarmRequest) + sizeof(GLNK_ScreenList) * 4;
	GLNK_SetScreenAlarmReques->ScreenList[0].ifSetAlarm = 1;
	GLNK_SetScreenAlarmReques->ScreenList[3].ifSetAlarm = 1;
  						---------
  						| 1 | 2 |
  						---------
  						| 3 | 4 |
  						---------												*/
typedef struct _GLNK_ScreenList
{
	int32_t ifSetAlarm;				//1为设置为侦测报警，0为不检测
	int32_t reserve;				//保留字段
}GLNK_ScreenList;

typedef struct _GLNK_SetScreenAlarmRequest
{
	int32_t ScreenChannel; 				//选定侦测报警的通道号
	int32_t ScreenListLen;				//列表的长度 = sizeof(GLNK_SetScreenAlarmRequest) + sizeof(GLNK_ScreenList)*ListNum,num为切屏数量
	GLNK_ScreenList ScreenList[0];			//保留字段
}GLNK_SetScreenAlarmRequest;

// 发送数据为要获取的通道号
typedef struct
{
	int32_t ScreenChannel; 				//选定侦测报警的通道号
}GLNK_GetScreenAlarmRequest;

/*****************************************************************************
			 	 	 	 	app请求更改ap密码及开关
*****************************************************************************/
typedef struct _TLV_V_ChangeAPPasswordRequest
{
	char oldpassword[GLNKPACKET_STR_LEN_32];	// 原密码密码
	char newpassword[GLNKPACKET_STR_LEN_32];	// 新密码密码
	int32_t isopenorclose;						// 0 -- 关wifi 1 -- 开wifi 2 -- 修改wifi密码
	int32_t reserve;				//保留字段
}TLV_V_ChangeAPPasswordRequest;


/*****************************************************************************
			 	 	 	 	API	接口
 *****************************************************************************/

/*	获取goolink版本号
 * 	retutn : 16进制的SDK版本号，如返回0x15160412则表示，1.5版本2016年4月12日更新的版本 		*/
unsigned long glnk_get_version();


/*	goolink版本号
 *  netparam ：见上
 * 	retutn : 0成功	*/
int glnk_init( InitNetParam* netparam);


/*	销毁goolink
 * 	retutn : 0--成功   其他--失败															*/
int glnk_destroy();


/*	推送报警接口
 * 	retutn : 1--成功   0--失败																*/
int glnk_push_alarm(PushAlarm alarm);


/*	推送433报警接口
 * 	retutn : 1--成功
 * 			 2--两次推送间隔小于5s
 * 			 3--设备没有此地址码
 * 			 4--撤防模式或地址不予许推送													*/
int32_t glnk_push_alarm2(Push433Alarm _433alarm);


/*	获取goolink连接服务器状态值
 * 	retutn : 0--连接失败，模块未启动
 * 			 1--连接中
 * 			 2--打开连接
 * 			 3--等待响应
 * 			 4--等待服务器中
 * 			 5--连接成功
 * 			 7--连接失败																	*/
int glnk_get_state_to_server();


/*	通知设备状态变更接口
 *  Data：一个json字段
	SD卡				“SDCard”:			"int8_t"	(0--无sd卡，1--有sd卡)
	PTZ				“PTZ”:				"int8_t"	(0--无，1--摇头，2--带光学变焦)
	鱼眼				“FishEye” :			"int8_t"	(0--无，1--有)
	720全景			“Panorama”:			"int8_t"	(0--无，1--有)
	产品形态			“ProductType”:		"int8_t"	(1--IPC，2--NVR，3--门铃，4--报警网关)
	电池				“Battery”	:		"int8_t"	(0--无，1--有)
	码流情况			“StreamingClass”:	"int8_t"	(2--两种，3--三种)
	视频编码类型		“VideoEncoderType”:	"int8_t"	(4--h264，5--h265，0--两种都支持)
	是否支持回音消除	“EchoCancel”:		"int8_t"	(0--不支持，1--支持)
	是否支持音频全双工	“Full_Duplex”		"int8_t"	(0--不支持，1--支持)
	全景参数			“PanoData”：			“string” 	(全景参数，这串数据由GLNK_GetPanoData收到的值给他赋值 )
	通道数			“Channels”			"int8_t"	(通道数)
//	版本日期			“SDKVersion”:		"string"	(sdk版本号，由glnk_get_version返回，是一个16进制数的日期号)
 	固件版本			“Firmware”:			"string"	(同GLNK_GetVersionFirmware_Callback,用|隔开)
 	全景模板版本		“PanoModelVersion”	"string"	（当GLNK_DownLoadPanoModel_Callback生成新模板时要更新此参数
 													是一个UTC时间，即模板生成时间)
	.....（可以拓展）
 *  len:	json字段的长度
 * 	retutn : 0--失败   1--成功														*/
int glnk_SendDeviceInfo(void *Data,int32_t len);


/*	发送视频码流接口（推荐使用）
 *  channel	：		通道号
 *  ismainorsub：	主码流--0 次码流--1
 *  isIFrame：		是否为I帧 0为p帧，1为I帧
 *  videoData：		原始H264或H265数据
 *  videoDataLen:	原始H264或H265数据长度
 *  StreamTime:		当天时间的绝对时间（ms）如：23:30:30:500 = (23*60*60 + 30*60 + 30)*1000 + 500
 * 	retutn : 数据长度--成功   0--失败														*/
int32_t glnk_sendvideostream(unsigned char channel,unsigned char ismainorsub,char isIframe,void *videoData,uint32_t videoLen,uint32_t StreamTime);


/*	发送音频码流接口（推荐使用）
 *  channel	：		通道号
 *  audioData：		原始数据
 *  audioLen:		原始数据长度
 *  StreamTime:		当天时间的绝对时间（ms）如：23:30:30:500 = (23*60*60 + 30*60 + 30)*1000 + 500
 * 	retutn : 数据长度--成功   0--失败														*/
int32_t glnk_sendaudiostream(unsigned char channel,void *audioData,uint32_t audioLen,uint32_t StreamTime);


/*	发送视频码流接口（旧）
 *  channel	：		通道号
 *  ismainorsub：	主码流--0 次码流--1
 *  isIFrame：		是否为I帧 0为p帧，1为I帧
 *  videoData：		原始H264或H265数据
 *  videoDataLen:	原始H264或H265数据长度
 * 	retutn : 数据长度--成功   0--失败														*/
int glnk_sendvideodata( unsigned char  channel,unsigned char ismainorsub,char isIFrame,void* videoData,uint32_t videoDataLen);


/*	发送音频码流接口（旧）
 *  channel	：		通道号
 *  audioData：		原始数据
 *  audioLen:		原始数据长度
 * 	retutn : 数据长度--成功   0--失败														*/
int glnk_sendaudiodata(	unsigned char channel,void* audioData,uint32_t audioDataLen);


/*	发送回放码流接口
 *  channel	：		码流缓存通道（GLNK_PlaybackOpen_CallBack的target值）
 *  isIFrame：		是否为I帧 0为p帧，1为I帧
 *  AudioOrVideo:	音视频标志位（1为音频，0为视频）
 *  Data：			码流数据
 *  Steamtime：		码流时间戳（当前 时: 分: 秒: 毫秒 至0点的毫秒数）如当前的是2016-6-14 16:20:30:20则时间戳忽略年月日，为 （16*60*60 + 20*60 + 30）*1000 + 20
 *  DataLen：		码流数据长度
 * 	retutn : 数据长度--成功   0--失败														*/
int32_t glnk_sendplaybackdata(	unsigned char channel, char isIFrame, char AudioOrVideo,void* Data, uint32_t StreamTime,uint32_t DataLen);


/*	结束回放码流
 *  channel:		码流缓存通道（GLNK_PlaybackOpen_CallBack的target值）
 * 	retutn : 0--失败   1--成功												*/
int32_t glnk_stopSendplaybackdata(unsigned char channel);


/*	更换网卡接口
 * 	ifname : 网卡名称（字符串）
 * 	retutn : 0--成功   其他--失败															*/
int glnk_set_ifname(char* ifname);


/*	获取从服务器得到的utc时间
 * 	retutn : 0--失败   其他--utc时间														*/
int32_t glnk_get_utc_from_server();


/*	文件下载接口
 *  sessionID ： 	发送数据的ID号(GLNK_DownLoadFileConfig_Callback返回)
 *  data：    			数据指针
 *  datalen：		数据长度
 *  start_pos：		数据在文件中的偏移的开始位置（字节）
 *  end_pos：		数据在文件中的偏移的结束位置（字节）
 * 	retutn : 0--失败   -1--发送超时，下次可继续发送这一数据   >0--为返回数据长度，发送成功	*/
int32_t glnk_SendDownLoadFileData(int32_t sessionID,char * data,int32_t datalen,int32_t start_pos,int32_t end_pos);


/*	结束文件下载接口
 * 	sessionID ：发送数据的ID号
 * 	filesize ： 文件的大小
 * 	retutn : 0--失败   filesize--成功														*/
int32_t glnk_FinishDownLoadFileData(int32_t sessionID, int filesize);

/*	文件下载接口
 *  sessionID ： 	发送数据的ID号(GLNK_DownLoadPanoModel_Callback返回)
 *  filename:		文件名
 *  data：    			数据指针
 *  datalen：		数据长度 (每片固定1024字节，直到最后一片)
 *  size:			文件总大小
 * 	retutn : 0--失败   -1--发送超时，下次可继续发送这一数据   >0--为返回数据长度，发送成功	*/
int32_t glnk_SendPanoModelData(int32_t sessionID,char *filename,char *data,int32_t datalen,int32_t size);

/*	结束文件下载接口
 * 	sessionID ：		发送数据的ID号
 * 	filename:		文件名
 * 	size:			文件的总大小 - 下载成功，0 -- 失败结束调用
 * 	retutn : 0--失败   1--成功														*/
int32_t glnk_FinishPanoModelData(int32_t sessionID,char *filename,int size);

/*	上传历史报警接口
 * 	FileName ： 图片、录像文件名（包含路径）
 * 	FileType ： 文件类型 1 录像 2 图片
 * 	Channel：   视频通道
 * 	AlarmType：	报警类型（433报警的类型由一个6位的16进制组成，如xx(433厂商编号)xx(433类型码)xx(433动作键值)，普通类型见glnk_push_alarm）
 * 	flag：		一个报警中可能会有多个图片和录像文件名，每次调用此函数上传一个文件名，在一次报警的最后一次调用函数时flag置为1其余置为0
 * 	retutn : 0--失败   1--成功														*/
int32_t glnk_AlarmFileStoreInfo(const char *FileName, int32_t FileType, int32_t Channel, uint32_t AlarmType, int32_t flag);


/*	主动发送433设备地址码接口
 * 	Sid ：			和GLNK_433DeviceLearnCallback函数的Sid对应
 * 	DeviceType ： 	学习433设备类型
 * 	CodeAddr：		433设备地址码（十六进制字符串形式）
 * 	retutn : 0--失败   1--成功														*/
int32_t glnk_Send433DeviceCodeAddr(int32_t Sid, int32_t DeviceType, char *CodeAddr);


/*	设置回放缓冲区帧数大小接口（可不设置）
 * 	value ：		帧数大小														*/
void SetPlayBackFrameBuffer(int value);

/*	设备主动断开接口
 * 	sid ：		GLNK_PwdAuthWithChannel_Callback返回的sid
 * 	reserve ： 	保留位，可忽略
 * 	retutn : 0--失败   1--成功														*/
int32_t glnk_CloseSession(int32_t sid,int32_t reserve);


/*	用户获取当前连接数接口
 * 	channel ：			想要查询的通道号
 * 	connectionnum ： 	查询的通道号当前的连接数
 * 	connectionsum ：	设备当前所有的连接数
 * 	retutn : 0--失败   1--成功														*/
int32_t glnk_GetSessionNum(int32_t channel,int32_t *connectionnum,int32_t *connectionsum);


/*	设置视频格式接口
 * 	channel ：		视频通道号
 * 	videoinfo ： 	视频格式
 * 	retutn : 0--失败   1--成功														*/
int glnk_SetVideoDataFormat(unsigned char  channel, GLNK_VideoDataFormat* videoinfo);


/*	设置服务器433开关状态接口
 * 	AddrCode ：		设备的地址码
 * 	isopen ： 		开关状态，0--关，1--开
 * 	retutn : 0--失败   1--成功														*/
int32_t glnk_Set433Switch(char *AddrCode,char isopen);


/*	发送直播流视频码流接口
 *  channel	：		通道号
 *  ismainorsub：	主码流--0 次码流--1
 *  videoData：		原始H264或H265数据
 *  videoDataLen:	原始H264或H265数据长度
 * 	retutn : 数据长度--成功   0--失败														*/
int glnk_sendRtmpvideodata( unsigned char  channel,unsigned char ismainorsub,char* videoData,uint32_t videoDataLen);


/*	发送直播流音频码流接口
 *  channel	：		通道号
 *  audioData：		原始数据
 *  audioLen:		原始数据长度
 * 	retutn : 数据长度--成功   0--失败														*/
int glnk_sendRtmpaudiodata(	unsigned char channel,char* audioData,uint32_t audioDataLen);

/*	固件主动升级接口（已废弃，改为app选择升级）
 * 	type：	 1--开始成功
 * 	retutn : 0--失败   1--成功														*/
int glnk_upload_Control(int type);


/*	打印连接当前连接数（已废弃）													*/
void glnk_print_sessions();

/*	上传图片数据接口，调用推送报警接口后才能调用此接口（已废弃，不需上传图片）
 * 	DataPtr ：		上传数据指针
 * 	DataLen ： 		上传数据长度
 * 	retutn : 0--失败   1--成功开始上传												*/
int32_t glnk_UploadData(char *DataPtr, int32_t DataLen);


/*
设备云存时区。若设备能正确设置时区，则不能调用,否则会出错
tz 相对于零时区的偏移值,东区就是负数，西区是正数
return -1 --失败
*/
int glnk_SetTimeZoneFunc(int tz);


/*
强插I帧注册接口,有新用户登录或切换码流时，直播强出I帧。
void(*fun)(int channel, int videotype)；
channel 为通道， videotype 为码流类型（主、次码流）
*/
void SetInsertIFrameFun(void(*fun)(int channel, int videotype));


#ifdef __cplusplus
}
#endif

#endif // __GOO_LINK_API_H__
