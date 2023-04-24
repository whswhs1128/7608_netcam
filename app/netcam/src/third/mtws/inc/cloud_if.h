#ifndef __CLOUND_IF_H__
#define __CLOUND_IF_H__

#if defined (__cplusplus)
extern "C" {
#endif

typedef struct 
{	
	char codec[16];
	int  width;
	int  height;
	int  framerate;
	int  bitrate;
	char config[512];
}VIDEO_STREAM_PARAM;

typedef struct
{
	int  enable;
	char codec[16];
	int  samplerate;
	int  samplewidth;//8 or 16
	int  channels;
	int  bitrate;
}AUDIO_STREAM_PARAM;


typedef enum _FRAME_TYPE_en
{
	FRAME_TYPE_I =  0x0001,
	FRAME_TYPE_P =  0x0002,	
	FRAME_TYPE_B =	0x0004,
	FRAME_TYPE_AUDIO =	0x0008, 
	
}FRAME_TYPE_EN;

#if 0
typedef  enum _VIDEO_STREAM_TYPE_en
{
	VIDEO_MAIN_STREAM = 0x0000,
	VIDEO_SUB_STREAM = 0x0001,
}VIDEO_STREAM_TYPE_EN;
#endif

typedef enum _REQ_MSG_TYPE_en
{
	NOTIFY_SVR_LOGIN, 
	REQ_USER_AUTH,
	REQ_STREAM_OPEN, 
	REQ_STREAM_CLOSE,
	REQ_STREAM_QUALITY_SET,
	REQ_PTZ_CONTROL, 
	REQ_CONFIG_GET,
	REQ_CONFIG_SET, 
	REQ_TRANSPARENT_CHANNEL_OPEN,
	REQ_TRANSPARENT_CHANNEL_CLOSE,
	REQ_TRANSPARENT_CHANNEL_DATA,
	REQ_REPLAY_QUERY,
	REQ_REPLAY_START,
	REQ_REPLAY_STOP,
	REQ_REPLAY_PAUSE,
	REQ_REPLAY_RESUME,
	REQ_REPLAY_SEEK,
	REQ_REPLAY_FILE_QUERY, 		//for ipc recorded files query
	REQ_FILE_REPLAY_CONTROL, 	//for IPC recorded files replay
}REQ_MSG_TYPE_EN;

enum
{
	STREAM_INDEX_BASE=0,
	STREAM_INDEX1,
	STREAM_INDEX2,
};

enum
{
	RECORD_MODE_BASE=0,
	RECORD_MODE_MANUAL = 1,
	RECORD_MODE_SCHEDULE = 2,
	RECORD_MODE_MOTIONDETECT = 4,
	RECORD_MODE_ALARM = 8,
};

#if 0
enum
{
	MEDIA_TYPE_BASE=0,
	MEDIA_TYPE_AUDIO,
	MEDIA_TYPE_VIDEO,
	MEDIA_TYPE_AUDIOVIDEO,
	MEDIA_TYPE_IMAGE,
};
#endif

#define MAX_RECORD_FILE_NAME_LEN 100
#define MAX_QUERY_RESULT_COUNT 20

typedef struct
{
	char file_name[MAX_RECORD_FILE_NAME_LEN];
	unsigned long create_time;
	unsigned long close_time;
	unsigned long file_duration;
	unsigned long file_size;
}SEARCH_FILE_ENTRY;

typedef struct
{
	int totalcount;
	int count;
	SEARCH_FILE_ENTRY filelist[MAX_QUERY_RESULT_COUNT];
}REPLAY_QUERY_RESULT;

typedef struct
{
	int record_mode;
	struct tm start_time;
	struct tm end_time;
	int media_type;
	int stream_index;
	unsigned long min_size;
	unsigned long max_size;
	int skip_count;
	int page_size;
	REPLAY_QUERY_RESULT result;
}REPLAY_QUERY_CONDITION;


typedef enum _REQ_SYSTEM_CONTROL_MSGCODE_TYPE_en
{
	SYSTEM_CONTROL_MSGCODE_CMD_GET_SYSTEMCONTROLSTRING = 10020,	
	SYSTEM_CONTROL_MSGCODE_CMD_GET_RECORD_FILE_LIST = 10021,
	SYSTEM_CONTROL_MSGCODE_CMD_REMOVE_FILE = 10024,
	
}REQ_SYSTEM_CONTROL_MSGCODE_TYPE_EN;


typedef enum _REQ_REPLAY_CONTROL_MSGCODE_TYPE_en
{
	ACTION_PLAY=0,
	ACTION_PAUSE,
	ACTION_RESUME,
	ACTION_FAST,
	ACTION_SLOW,
	ACTION_SEEK,
	ACTION_FRAMESKIP,
	ACTION_STOP
}REQ_REPLAY_CONTROL_MSGCODE_TYPE_EN;

typedef struct
{
	char file_name[256];
	int start_pos;
	int play_speed;
	REQ_REPLAY_CONTROL_MSGCODE_TYPE_EN playCmd;
}FILE_REPLAY_CONTROL_MSG;

typedef enum _STREAM_TYPE_en
{
	STREAM_VIDEO_MAIN,
	STREAM_VIDEO_SUB,
	STREAM_AUDIO,

}VIDEO_STREAM_TYPE_EN;



typedef enum _CLIENT_STATE_en
{
	CLIENT_STATE_DISCONNECTED=0,
	CLIENT_STATE_CONNECTED,
	CLIENT_STATE_AUTH_OK,
	CLIENT_STATE_AUTH_FAILED,
}CLIENT_STATE_EN;



typedef enum _SVR_TYPE_en
{
	SVR_WEB=0,
	SVR_P2P=1,
}SVR_TYPE_EN;


typedef struct _NOTIFY_SVR_LOGIN_s
{
	int  svrType;
	char devId[32];
}NOTIFY_SVR_LOGIN_T;



typedef struct _REQ_USER_AUTH_s	
{
	int  sessionId;
	char username[64];
	char password[64];
}REQ_USER_AUTH_T;


typedef struct _REQ_STREAM_OPEN_s
{
	int sessionId;
	int channelId;
	int videoStreamType;
	int audioOpen;
	int frameType;
}REQ_CHANNEL_OPEN_T;

typedef struct _REQ_STREAM_CLOSE_s
{
	int sessionId;
	int channelId;
}REQ_CHANNEL_CLOSE_T;

typedef struct _REQ_STREAM_QUALITY_s
{
	int sessionId;
	int frameType;
}REQ_STREAM_QUALITY_T;


typedef struct _REQ_FORCE_I_FRAME_s
{
	int sessionId;
}REQ_FORCE_I_FRAME_T;


typedef struct _REQ_PTZ_ACTION_s
{
	int sessionId;
	char xml[1024];

}REQ_PTZ_ACTION_T;


typedef struct _RSP_PTZ_ACTION_s
{
	int sessionId;
	int ret;
	char xml[1024];

}RSP_PTZ_ACTION_T;


typedef struct _REQ_CONFIG_GET_s
{
	int sessionId;
	int configId;
}REQ_CONFIG_GET_T;

typedef struct _RSP_CONFIG_GET_s
{
	int sessionId;
	int ret;
	int configId;
	int xmllen;
	char *pxml;
}RSP_CONFIG_GET_T;


typedef struct _REQ_CONFIG_SET_s
{
	int sessionId;
	int configId;
	char xml[4096];
}REQ_CONFIG_SET_T;

typedef struct _REQ_TRANS_CHANNEL_OPEN_s
{
	int sessionId;

}REQ_TRANS_CHANNEL_OPEN_T;


typedef struct _REQ_TRANS_CHANNEL_CLOSE_s
{
	int sessionId;

}REQ_TRANS_CHANNEL_CLOSE_T;

typedef struct _REQ_TRANS_CHANNEL_DATA_s
{
	int sessionId;
	char *data;
	int  dataLen;
}REQ_TRANS_CHANNEL_DATA_T;


typedef struct _DATE_s
{
	int year;
	int mon;
	int day;
}DATE_t;


typedef struct _TIME_s
{
	int year;
	int mon;
	int day;
	int hour;
	int minute;
	int second;


}TIME_t;


typedef struct _REQ_REPLAY_QUERY_s
{

	int sessionId;
	int channelId;
	DATE_t date;

}REQ_REPLAY_QUERY_t;


typedef struct _REQ_REPLAY_START_s
{
	int sessionId;
	int channelId;
	TIME_t time;
}REQ_REPLAY_START_t;


typedef struct _REQ_REPLAY_STOP_s
{
	int sessionId;
	int channelId;
		
}REQ_REPLAY_STOP_t;

typedef struct _REQ_REPLAY_PAUSE_s
{
	int sessionId;
	int channelId;
		
}REQ_REPLAY_PAUSE_t;
typedef struct _REQ_REPLAY_RESUME_s
{
	int sessionId;
	int channelId;
		
}REQ_REPLAY_RESUME_t;

typedef struct _REQ_REPLAY_SEEK_s
{
	int sessionId;
	int channelId;
	TIME_t time;	
}REQ_REPLAY_SEEK_t;




#define NVR_RECORDTYPE_TIMER             'A'  // 定时录像, 对应 1
#define NVR_RECORDTYPE_ALARM             'B'  // 报警录像, 对应 3
#define NVR_RECORDTYPE_NORECORD          'C'  // 无录像


typedef struct _REPLAY_QUERY_RESULT_s
{
	int ret;
	DATE_t  date;
	char recordTypes[1440]; //每分钟的录像类型，see NVR_RECORDTYPE_*
}REPLAY_QUERY_RESULT_t;



typedef struct _RECORD_VIDEO_PARAM_s
{
    char codec[256];
    int width;
    int height;
    int colorbits;
    int framerate;
    int bitrate;
    char volData[256];
    int volLength;

}RECORD_VIDEO_PARAM_t;


typedef struct _RECORD_AUDIO_PARAM_s
{
    char codec[256];
    int samplerate;
    int bitspersample;
    int channels;
    int framerate;
    int bitrate;

}RECORD_AUDIO_PARAM_t;



typedef struct _RECORD_AV_PARAM_s
{
    int hasVideo;
    int hasAudio;
    RECORD_VIDEO_PARAM_t videoParam;
    RECORD_AUDIO_PARAM_t audioParam;
}RECORD_AV_PARAM_T;


typedef struct _REPLAY_START_RESULT_s
{
	int ret;
	RECORD_AV_PARAM_T recordAVParam;
}REPLAY_START_RESULT_t;

typedef struct _REPLAY_RESUME_RESULT_s
{
	int ret;
	RECORD_AV_PARAM_T recordAVParam;
}REPLAY_RESUME_RESULT_t;



enum
{
    NVR_REC_QUERY =   3001,
    NVR_REC_PLAYSTART,
    NVR_REC_PLAYSTOP,
    NVR_REC_SETSPEED,

    NVR_REC_PLAYPAUSE = 3005,
    NVR_REC_PLAYRESUME,
    NVR_REC_PLAYFAST,
    NVR_REC_PLAYSLOW,
    NVR_REC_PLAYNORMAL,

    NVR_REC_PLAYSEEK = 3010,
    NVR_REC_PLAYFRAMESTEP,
    NVR_REC_PLAYAVPARAM
};


typedef struct 
{
	unsigned long frame_timestamp;			//此帧对应的时间戳，用于音视频同步，一帧中的不同包时间戳相同
	unsigned long keyframe_timestamp;		//如果是非I帧，记录其前一I帧的timestamp，如果解码器没有收到前面那个I帧，所有非I帧丢掉丢包不解码
	unsigned short pack_seq;						//包序号0-65535，到最大后从0开始
	unsigned short payload_size;				//此包中包含有效数据的长度
	unsigned char pack_type;						//0x01第一包，0x10最后一包, 0x11第一包也是最后一包，0x00中间包
	unsigned char frame_type;    				//帧类型1：I帧，0：非I帧
	unsigned char stream_type;					//0: video, 1: audio，2：发送报告，3：接收报告，4：打洞包
	unsigned char stream_index; 
	unsigned long frame_index;					//added by johnnyling 20100722, index for one stream_type;
}MEDIA_DATA_HEADER;


typedef int (*NET_EVENT_CALLBACK)(int msgType,  void *pMsgContext, void *pUserContext);
typedef int (*AUDIO_TALKBACK_CALLBACK)(char *audio_data, int audio_len, void *pUserContext); //20160927
typedef int (*SYSTEMCONTROL_CALLBACK)(int msg_code, char *xml, char *rsp_xml, int rsp_len, void *pUserContext); //20161008

typedef void (*p2p_log_func)(const char* message);
void CloudSdkSeLogFunc(p2p_log_func log_func, int sdk_log_level);

//int CloudSdkInit(int devType, int maxChannel, char *pDevSn);
int CloudSdkInit(char *vendorId, char *vendorPassword, int devType, int maxChannel, char *pDevSn, char *pDevUsername, char *pDevPassword);
int CloudSdkUpdateAccount(char *pDevUsername, char *pDevPassword); //20160412

int CloudSdkRelease();

int CloudRegisterNetEventCallBack(NET_EVENT_CALLBACK fNetReadCallBack, void *pUserContext);
int CloudRegisterAudioTalkbackCallBack(AUDIO_TALKBACK_CALLBACK fAudioCallBack, void *pUserContext);
int CloudRegisterSystemcontrolCallBack(SYSTEMCONTROL_CALLBACK fSystemControlCallback, void *pUserContext); //20161008

//int CloudSetDevSn(char *pSN);

int CloudSetUserAuthInfo(char *pUsername, char *pPasswd);
void CloudSdkSetVersion(char *kernel_ver, char *fs_ver, char *firmware_ver); //20160423

int CloundSetChannelState(int channelId, int state);

int CloudSetVideoParam(int channelId, int streamType, VIDEO_STREAM_PARAM videoParam);

int CloudSetAudioParam(int channelId, AUDIO_STREAM_PARAM   audioParam);

int CloudLoginServer();

int CloudGetDeviceId(char *pDevId);

int CloudWriteMainStreamData(int channelId, char* pFrameData, int length, int frameType, unsigned long timestamp);

int CloudWriteSubStreamData(int channelId, char* pFrameData, int length, int frameType, unsigned long timestamp);

//int CloudWriteAudioStreamData(int channelId, char* pFrameData, int length);
int CloudWriteAudioStreamData(int channelId, char* pFrameData, int length, unsigned long timestamp);


int CloudSendAlarm(int channelId, int alarmType, int alarmLevel, char *alarmDesc);

int CloudSendPtzRsp(int sessionId,  RSP_PTZ_ACTION_T rsp);

int CloudSendConfig(int sessionId, RSP_CONFIG_GET_T rsp);




int CloundTransChnlSend(int sessionId, char *pData, int dataLen);


int SystemGetTimeofRun(struct timeval *tv, struct timezone *tz);


int CloudRelaySendQueryResult(int sessionId, int channelId, REPLAY_QUERY_RESULT_t *pQueryResult);

int CloundRelaySendStartResult(int sessionId, int channelId, REPLAY_START_RESULT_t *pStartResult);

int CloundRelaySendResumeResult(int sessionId, int channelId, REPLAY_RESUME_RESULT_t *pResumeResult);



int CloudRelaySendVideo(int sessionId, int channelId, char* pFrameData, int length,unsigned long timestamp, int is_key);

int CloundRelaySendAudio(int sessionId, int channelId, char* pFrameData, int len, unsigned long timestamp);

#define tps_log(format, arg...)   { printf("%s "format, __func__, ##arg);}

#if defined (__cplusplus)
}
#endif

#endif





