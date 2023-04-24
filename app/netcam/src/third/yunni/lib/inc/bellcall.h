#ifndef _BELL_CALL_H_
#define _BELL_CALL_H_

//
// 系统设置相关结构体
//

//
// ELOG 类型
//
enum{
	ELOG_MOTION_DETECT,	// 移动侦测事件
	ELOG_LOGIN,			// 登录事件
	ELOG_OPERATION		// 操作事件
};

//
// PTZ 控制指令
//
enum{
    CMD_PTZ_UP,
    CMD_PTZ_UP_STOP,
    CMD_PTZ_DOWN,
    CMD_PTZ_DOWN_STOP,
	CMD_PTZ_LEFT,
	CMD_PTZ_LEFT_STOP,
	CMD_PTZ_RIGHT,
	CMD_PTZ_RIGHT_STOP,
	CMD_PTZ_CENTER=25,
	CMD_PTZ_UP_DOWN=26,

	CMD_PTZ_UP_DOWN_STOP=27,
	CMD_PTZ_LEFT_RIGHT,
	CMD_PTZ_LEFT_RIGHT_STOP,

    CMD_PTZ_PREFAB_BIT_SET0,
    CMD_PTZ_PREFAB_BIT_RUN0,

    CMD_PTZ_PREFAB_BIT_SETF=60,
    CMD_PTZ_PREFAB_BIT_RUNF,
    CMD_PTZ_LEFT_UP=90,
    CMD_PTZ_RIGHT_UP,
    CMD_PTZ_LEFT_DOWN,
    CMD_PTZ_RIGHT_DOWN,
    CMD_PTZ_IO_HIGH,
    CMD_PTZ_IO_LOW,
    CMD_PTZ_MOTO_TEST=255,
};

//
// 磁盘控制指令
//
enum{
	CMD_STORAGE_NONE=0,
	CMD_STORAGE_FORMAT,
	CMD_STORAGE_UMOUNT
};

//
// 媒体控制指令
//
enum{
	CMD_MEDIA_NULL=0,	// 指令执行完成后清零
	CMD_MEDIA_PLAY,		// 播放新文件
	CMD_MEDIA_TURN,		// 播放暂停切换
	CMD_MEDIA_DRAG,		// 拖动到位置
	CMD_MEDIA_STOP		// 停止播放
};

//
// 媒体线程状态
//
enum{
	STA_MEDIA_IDLE=0,	// 空闲模式
	STA_MEDIA_PLAY,		// 播放模式
	STA_MEDIA_PAUSE		// 暂停模式
};

//
// 图像控制参数
//
typedef struct{
    int Brightness;		// 0-255
    int Contrast;		// 0-255
    int Sharpness;		// 0-255
    int Hue;        	// 0-255
    int Saturation; 	// 0-255
    int FilckFrequency;	// 0->50HZ,1->60HZ
    int FlipHor; 		// 0 not; 1, flip turn
    int MirrorVer; 		// 0 not; 2, mirror turn
    int IRCut; 			// 0 close, 1, enable
    int	Range;			// 0 Min Value 1, Max Value 2, Set Value
}PPCGI_IMAGE_T;

//
// 音频参数设置
//
typedef struct{
	int SampleRate;	// 采样率
	int BitRate;	// 比特率
	int	EncodeType; // 编码方式
	int Mute;		// 静音
    int	Range;		// 0 Min Value 1, Max Value 2, Set Value
}PPCGI_AUDIO_T;

//
// 流控制参数
//
typedef struct{
    int StreamID;	// 0-2 //3 stream
    int FPS;		// 1-30
    int BitRate;	// 10-8000 kbps
    int	Range;		// 0 Min Value 1, Max Value 2, Set Value
}PPCGI_VIDEO_T;

//
// OSD 参数
//
typedef struct{
	int 	OSDEnable;
	char 	OSDStr;
	int		FontSize;
	int 	FontType;
	int 	FontColor;
	int 	Pos[4];
}PPCGI_OSD_T;

//
// 时间设置参数
//
typedef struct{
    int  NTPEnable;		// 0: 开启时间同步 1: 关闭时间同步
    char NTPServer[32];	// 时间同步服务器
    int  TimeZone;   	// seconds for UTC
    int  TimeUTCs; 		// senconds from 1970
}PPCGI_TIME_T;

//
// 报警设置
//
typedef struct{
	int	MotionEnable;			// 移动侦测开关
	int MotionSensitivity;		// 移动侦测灵敏度
	int	InputEnable;			// GPIO 输入使能
	int ILevel;					// 输入有效电压
	int OLevel;					// 输出有效电压
	int IOLinkage;				// GPIO 输入输出联动
	int UploadInterval;			// 报警上传周期
	int AlarmByMail;			// 邮件报警开关
	int AlarmScheduleEnable;	// 报警计划开关
	int AlarmSnapshotEnable;	// 报警抓图开关
	int AlarmRecorderEnable;	// 报警录像开关
	int AlarmMovePerset;		// 报警移动预置位
	int AlarmMute;				// 报警静音
	int AlarmSchedules[7][3];	// 报警计划内容
}PPCGI_ALARM_T;

//
// 邮件设置
//
typedef struct{
	char SMTPSvr[128];			// SMTP 发送邮件服务器
	int  Port;					// SMTP 发送端口
	int  EncryptType;			// 邮件发送加密方式 0: 不启用加密 1: SSL 2:TLS
	char User[32];				// SMTP 登录用户
	char Pass[32];				// SMTP 登录密码
	char MailFrom[128];			// 发送邮箱地址
	char MailTos[4][128];		// 接收邮箱地址,总共4个
}PPCGI_MAIL_T;

//
// FTP 客户端 上传设置
//
typedef struct{
	char FTPSvr[128];			// FTP 服务器
	int	 Port;					// 端口
	int  Mode;					// 连接模式 0: Port 1: Pasv
	char User[32];				// 登录用户
	char Pass[32];				// 密码
	char DataPath[255];			// 数据上传路径
	char FileName[255];			// 保存文件名
	int	 UploadInterval;		// 上传时间间隔
}PPCGI_FTPC_T;

//
// CIFS 上传设置
//
typedef struct{
	char CIFSSvr[128];			// CIFS 服务器
	char User[32];				// 登录用户
	char Pass[32];				// 密码
	char DataPath[255];			// 数据上传路径
	char CIFSPath[255];			// 挂载路径
}PPCGI_CIFS_T;

//
// 网络接口设置
//
typedef struct{
	char Interface[32];			// 网络接口
	char IPv4[32];				// 地址
	char Mask[32];				// 掩码
	char Gate[32];				// 网管
	int	 DHCP;					// 开启DHCP
	char DNS1[32];				// DNS1
	char DNS2[32];				// DNS2
	char MAC[32];				// 物理地址
	int  Type;					// 0: 有线 1: 无线
}PPCGI_NETWORK_T;

//
// WIFI设置
//
typedef struct{
	char SSID[32];				// SSID
	char Pass[32];				// 认证密码
	char EncryptType;			// 加密方式
	int	 Mode;					// WIFI模式 0:AP 1:中继 2:客户端
	int	 Channel;				// 信道
	int	 Quality;				// 通信质量
	PPCGI_NETWORK_T Network;	// 硬件信息
}PPCGI_WIFI_T;

//
// WIFI扫描结果
//
typedef struct{
	char SSID[32];				// SSID
	char MAC[32];
	char EncryptType;			// 加密方式
	int  Channel;
	int	 Quality;				// 通信质量
}PPCGI_WIFI_LIST_T;

typedef struct{
	PPCGI_WIFI_LIST_T * Lists;	// WIFI 列表数组
	int	 ListSize;				// 数组大小
}PPCGI_WIFI_SCAN_T;

//
// 录像设置
//
typedef struct{
	int	RecorderEnable;					// 0: 开启录像 1: 停止录像
	int RecorderTime;					// 录像时长
	unsigned long long RecorderSize;	// 录像大小
	int	Overwrite;						// 0: 不覆盖旧文件 1: 覆盖旧文件
	int RecorderEnableAudio;			// 录制音频
	int RecorderSchedules[7][3];		// 录像计划内容
}PPCGI_RECORD_T;

//
// 媒体检索
//
typedef struct{
	int Year;
	int Month;
	int Day;
	int Hour;
	int Min;
	int Sec;
}COMMO_TIME_T;

typedef struct{
	int Size;					// 文件数目
	int	Channel;				//0xff：全部通道，0，1，2 ......
	int Type;					//0xff － 全部，0 － 定时录像，1 - 移动侦测，2 － 报警触发，3  － 手动录像
	COMMO_TIME_T Times[2];		// 开始结束时间
	void *	hSearch;			// 搜索句柄
}PPCGI_MEDIA_SEARCH_T;

typedef struct{
	char Filename[256];			// 文件名
	COMMO_TIME_T Times[2];		// 开始结束时间
	int	Channel;				// 通道
	int Size;					// 大小
	int State;					// 转储状态
	void *	hSearch;			// 搜索句柄
}PPCGI_MEDIA_INFO_T;

//
// 存储信息
//
typedef struct{
	int					StorageType;	// 0:HDD 1:USB 2:SD 3:I-SCSI 4:E-SATA
	unsigned long long 	Size;			// 大小
	unsigned long long 	Used;			// 用量
	char 				Filesystem[32];	// 文件系统
	char				Status;			// 0: 未挂载 1: 已挂载 2: 不存在 300-399: 格式化中,用于进度返回
	char				Path[255];		// 挂载路径
	char				DeviceName[32];	// 设备名称
	int					Command;		// 磁盘控制指令 参见枚举 CMD_STORAGE_*
}PPCGI_STORAGE_T;

//
// 产品信息和升级接口
//
typedef struct{
	char 				ServerStr[1024];	// P2P 服务器字串
	char 				ID[32];				// P2P ID
	char 				Model[128];			// 产品型号
	char 				Product[128];		// 产品厂商
	char 				Version[128];		// 系统版本
	char *				UpgradeData;		// 升级包缓冲区
	int					UpgradeDataLens;	// 缓冲区长度
	int					UpgradeArea;		// 升级区域编号
	volatile int 		UpgradeStatus; 		// 升级进度
}PPCGI_PRODUCT_T;

//
// 用户信息
//
typedef struct{
	int					UserID;				// 用户标识符
	char				User[32];			// 用户名
	char				Pass[32];			// 密码
}PPCGI_USER_T;

typedef struct{
	unsigned long long 	CPUFrequency;		// CPU 主频
	int					RamSize;			// 内存大小
	int					MaxVideoEncoder;	// 视频编码器数量
	int					MaxVoiceEncoder;	// 音频编码器数量
	int *				VideoBufferSizes;	// 视频各路缓冲区参考值
	int *				VoiceBufferSizes;	// 音频各路缓冲区参考值
}PPDEV_BROAD_T;

typedef struct{
	int 			MaxChannels;
	int				Handle;
}PPDEV_VIDEO_T;

typedef struct{
	int				Handle;
}PPDEV_AUDIO_T;

typedef struct{
	int 			Handle;
}PPDEV_VOICE_T;

typedef struct{
	char	 		File[256];
	COMMO_TIME_T	Time;
	int				Handle;
}PPDEV_MEDIA_T;

//
// 设备控制接口
// 由平台端完成相关函数,供上层应用逻辑调用.
// 没有实现的函数,请返回:[-1].
//

//
// 设备初始化
//
int PPDevInit(
	void *	Data,
	int		DataLens
);

//
// 关闭设备
//
int PPDevFree(
	void *	Data,
	int		DataLens
);

//
// 设备 GPIO 初始化
//
int PPDevGPIOsInit(
	void *	Data,
	int 	DataLens
);

//
// 设备 GPIO 控制, 由上层循环调用
//
int PPDevGPIOsCall(
	void *	Data,
	int 	DataLens
);

//
// 设备 GPIO 释放
//
int PPDevGPIOsFree(
	void *	Data,
	int 	DataLens
);

//
// 媒体文件流初始化
//
int PPDevMediaInit(
	void *	Data,
	int		DataLens
);

//
// 视频流初始化
//
int PPDevVideoInit(
	void *	Data,
	int		DataLens
);

//
// 音频流初始化
//
int PPDevAudioInit(
	void *	Data,
	int		DataLens
);

//
// 语音流初始化
//
int PPDevVoiceInit(
	void *	Data,
	int		DataLens
);

//
// 视频流释放
//
int PPDevVideoFree(
	void *	Data,
	int		DataLens
);

//
// 音频流释放
//
int PPDevAudioFree(
	void *	Data,
	int		DataLens
);

//
// 语音流释放
//
int PPDevVoiceFree(
	void *	Data,
	int		DataLens
);

//
// 媒体文件流释放
//
int PPDevMediaFree(
	void *	Data,
	int		DataLens
);

//
// 通过文件捕获音视频帧
//
int PPDevGetMediaFrame(
	int		MediaHandle,		// 句柄
	char *	MediaBuffer,		// 缓冲区
	int		MediaBufferSize,	// 缓冲区大小
	int  *	MediaBufferType		// 类型
);

//
// 通过设备捕获视频帧
//
int PPDevGetVideoFrame(
	int		VideoHandle,		// 句柄
	char **	VideoBuffer,		// 缓冲区
	int		VideoBufferSize,	// 缓冲区大小
	int	 *	VideoBufferType,	// 图像类型
	int  	Channel,			// 通道
	int	 	Refresh				// 刷新缓冲
);

//
// 通过设备捕获音频帧
//
int PPDevGetVoiceFrame(
	int 	VoiceHandle,		// 句柄
	char *	VoiceBuffer,		// 缓冲区
	int		VoiceBufferSize,	// 缓冲区大小
	int	 *	VoiceBufferType,	// 音频类型
	int  	Channel,			// 通道
	int	 	Refresh				// 刷新缓冲
);

//
// 通过设备播放音频帧
//
int PPDevPutAudioFrame(
	char *	AudioBuffer,		// 缓冲区
	int		AudioBufferLens,	// 缓冲区大小
	int	 *	AudioBufferType		// 音频类型
);

//
// 获取设备 GPIO 的状态
//
int PPDevGetGPIO(
	int		Pin
);

//
// 设置设备 GPIO 的状态
//
int PPDevSetGPIO(
	int 	Pin,
	int		Val
);

//
//	休眠动作
//
int PPDevSetStandby(
	void *	Data,
	int		DataLens
);

//
//	唤醒动作
//
int PPDevGetStandby(
	void *	Data,
	int		DataLens
);

//
//	重启系统
//
int PPDevReboot(
	void *	Data,
	int		DataLens
);

//
// P2P CGI 指令接口
// 由平台端完成相关函数,供上层应用逻辑调用.
// 没有实现的函数,请返回:[-1].
//

// CGI 调用模块初始化
int PPCgiModuleInit();

// CGI 调用模块销毁
int PPCgiModuleFree();

// 主机名设置获取
int PPCgiSetAlias(const char * Alias);
int PPCgiGetAlias(char * Alias,int Lens);

// 音频控制
int PPCgiSetAudio(PPCGI_AUDIO_T * AudioParamSet);
int PPCgiGetAudio(PPCGI_AUDIO_T * AudioParamGet);
int PPCgiSetAudioDefault();

// 图像控制
int PPCgiSetImage(PPCGI_IMAGE_T * ImageParamSet);
int PPCgiGetImage(PPCGI_IMAGE_T * ImageParamGet);
int PPCgiSetImageDefault();

// 视频控制
int PPCgiSetVideo(PPCGI_VIDEO_T * VideoParamSet);
int PPCgiGetVideo(PPCGI_VIDEO_T * VideoParamGet);
int PPCgiSetVideoDefault();

// OSD 控制
int PPCgiSetOSD(PPCGI_OSD_T * OSDParamSet);
int PPCgiGetOSD(PPCGI_OSD_T * OSDParamGet);

// 录像管理
int PPCgiSetRecord(PPCGI_RECORD_T * RecordParamSet);
int PPCgiGetRecord(PPCGI_RECORD_T * RecordParamGet);

// 时间设置
int PPCgiSetTime(PPCGI_TIME_T * TimeParamSet);
int PPCgiGetTime(PPCGI_TIME_T * TimeParamGet);

// 报警设置
int PPCgiGetAlarm(PPCGI_ALARM_T * AlarmParamGet);
int PPCgiSetAlarm(PPCGI_ALARM_T * AlarmParamSet);

// 邮件设置
int PPCgiGetMail(PPCGI_MAIL_T * MailParamGet);
int PPCgiSetMail(PPCGI_MAIL_T * MailParamSet);

// 网络设置
int PPCgiGetNetwork(PPCGI_NETWORK_T * NetworkParamGet);
int PPCgiSetNetwork(PPCGI_NETWORK_T * NetworkParamSet);

// 无线设置
int PPCgiSetWIFI(PPCGI_WIFI_T * WIFIParamSet);
int PPCgiGetWIFI(PPCGI_WIFI_T * WIFIParamGet);
int PPCgiLstWIFI(PPCGI_WIFI_SCAN_T * WIFIScanParamLst);

// FTP  上传设置
int PPCgiSetFTPClient(PPCGI_FTPC_T * FTPClientParamSet);
int PPCgiGetFTPClient(PPCGI_FTPC_T * FTPClientParamGet);

// CIFS 上传设置
int PPCgiSetCIFS(PPCGI_CIFS_T * CIFSParamSet);
int PPCgiGetCIFS(PPCGI_CIFS_T * CIFSParamGet);

// HTTP 端口设置
int PPCgiGetHTTPPort();
int PPCgiSetHTTPPort(int HTTPPort);

// 存储管理
int PPCgiSetStorage(PPCGI_STORAGE_T * StorageParamSet);
int PPCgiGetStorage(PPCGI_STORAGE_T * StorageParamGet);

// 媒体文件检索
int PPCgiLstMedia(PPCGI_MEDIA_INFO_T * MediaInfoLst);			// 获取检索的结果
int PPCgiPutMediaSearch(PPCGI_MEDIA_SEARCH_T * MeidaSearchPut);	// 关闭检索操作
int PPCgiGetMediaSearch(PPCGI_MEDIA_SEARCH_T * MeidaSearchGet);	// 打开并设置检索条件

// 产品信息设置
int PPCgiSetProduct(PPCGI_PRODUCT_T * ProductParamSet);
int PPCgiGetProduct(PPCGI_PRODUCT_T * ProductParamGet);

// 恢复出场设置
int PPCgiSetDefault();

// 用户设置
int PPCgiGetUser(PPCGI_USER_T * UserListGet,int * ListSize); // 获取用户
int PPCgiAddUser(PPCGI_USER_T * UserSet); // 创建用户
int PPCgiDelUser(PPCGI_USER_T * UserSet); // 删除用户
int PPCgiSetUser(PPCGI_USER_T * UserSet); // 设置用户

int PPCgiSetAnonymousEnable(int Value); // 设置匿名用户启用开关
int PPCgiGetAnonymousEnable(); 			// 获取匿名用户启用开关

// PTZ 控制
int PPCgiSetPTZMove(int Direct,int Step);

// 获取云台巡航次数
int PPCgiGetPTZRunTimes();

// 设置云台巡航次数
int PPCgiSetPTZRunTimes(int Value);

// 获取云台运动速度
// 方向 0:上 1:下 2:左 3:右	4:手动
int PPCgiGetPTZSpeed(int Direct );

// 设置云台运动速度
int PPCgiSetPTZSpeed(
	int Direct,			// 方向 0:上 1:下 2:左 3:右 4:手动
	int Value			// 值
);

//
// 功能性外部调用
//

// 报警调用
int PPMsgPush(
	int 	PID,		// 平台 ID
	void *	Params,		// 参数
	int		Lens		// 参数长度
	);

// 初始化工作线程
int PPSvrInit(
	const char * ServerID,
	const char * DeviceID,
	int			 DeviceType
);

// 测试函数
int PPSvrTest(
	int			 DeviceType
);

// 销毁工作线程
int PPSvrDestroy();

//extern struct device_info_mtd device_info;

//macro
#define VALUE_CONVERT(value, min, max) \
	((value) > (max) ? (max) : (value) < (min) ? (min) : (value))

#define TRUE_FALSE_CONVERT(value, vfalse, vtrue) \
	((value) ? (vtrue) : (vfalse))

#define CHANNEL_ID_CONVERT(Channel) \
	((Channel) == 0 ? 1 : (Channel) == 1 ? 2 : (Channel) == 2 ? 0 : 0)

#endif

