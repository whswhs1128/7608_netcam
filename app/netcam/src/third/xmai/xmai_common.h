#ifndef _XMAI_COMMON_H_
#define _XMAI_COMMON_H_

#define XmaiSetThreadName(name) \
{\
    prctl(PR_SET_NAME, (unsigned long)name, 0,0,0); \
    pid_t tid;\
    tid = syscall(SYS_gettid);\
    printf("set pthread name:%d, %s, %s pid:%d tid:%d\n", __LINE__, __func__, name, getpid(), tid);\
}


//调试打印
//出错信息始终打印
/*
#define PRINT_ERR(fmt, args...) do{ \
		fprintf(stderr, "*** [FILE]:%s [FUNCTION]:%s [Line]:%d *** [INFO]: ", __FILE__, __FUNCTION__, __LINE__);\
		fprintf(stderr, fmt, ##args); \
}while(0)
*/
//#define PRINT_ERR(fmt, args...)  printf("[error -- File: %s, Line: %d, Function: %s] " fmt "\n", \
//	                             __FILE__, __LINE__, __FUNCTION__, ##args)
//提示信息可根据需要是否打印
//#define XMAI_DEBUG 1
//#if XMAI_DEBUG
//#define PRINT_INFO(fmt, args...)  printf("[Info -- File: %s, Line: %d, Function: %s] "fmt,\
//								  __FILE__, __LINE__, __FUNCTION__, ##args)


#define PRINT_DELIMITER(fmt, args...)  printf("\n**************************************\n")

#define DEGLINE(fmt, args...)  printf("[Showline -- File: %s, Line: %d, Function: %s] " fmt "\n",\
								  __FILE__, __LINE__, __FUNCTION__, ##args)

//#else
//#define PRINT_INFO(fmt, args...) {}
//#define PRINT_DELIMITER(fmt, args...)  {}
//#define DEGLINE(fmt, args...)  {}
//#endif






//基本宏定义
#define XMAI_MSG_SEND_BUF_SIZE (16*1024)
#define XMAI_STREAM_DATA_SEND_BUF_SIZE (1*1024*1024)
#define XMAI_PACKET_SEND_LEN_ONCE (8*1024)
#define XMAI_MAX_LINK_NUM  10

#define XMAI_TCP_PORT 34567
//#define XMAI_TCP_LISTEN_PORT 8005
#define XMAI_TCP_LISTEN_PORT XMAI_TCP_PORT


#define XMAI_BROADCAST_RCV_PORT 34569
#define XMAI_BROADCAST_SEND_PORT 34569
#define XMAI_SSL_PORT  8443
//#define XMAI_HTTP_PORT  80
#define XMAI_UDP_PORT  34568
#define XMAI_MSG_RECV_BUF_SIZE (16*1024)
#define ETH_DEV "eth0"
//每个音频帧G711A裸流数据的实际大小
#define XMAI_AUDIO_SIZE_PRE_FRAME (320) 

typedef unsigned long		DWORD;

typedef enum{
    LOGIN_REQ = 1000, 
    LOGIN_RSP = 1001,
    KEEPALIVE_REQ = 1006,
    KEEPALIVE_RSP = 1007,
    SYSINFO_REQ	= 1020,
    SYSINFO_RSP	= 1021,
    CONFIG_SET = 1040,
    CONFIG_SET_RSP = 1041,
    CONFIG_GET	= 1042,
    CONFIG_GET_RSP	= 1043,
    CONFIG_CHANNELTILE_SET = 1046,
    CONFIG_CHANNELTILE_SET_RSP = 1047,
    CONFIG_CHANNELTILE_GET = 1048,
    CONFIG_CHANNELTILE_GET_RSP = 1049,
    CONFIG_CHANNELTILE_DOT_SET = 1050,
    CONFIG_CHANNELTILE_DOT_SET_RSP = 1051,
    ABILITY_GET	= 1360,
    ABILITY_GET_RSP = 1361,
    PTZ_REQ = 1400,
    PTZ_RSP = 1401,
    MONITOR_REQ = 1410,
    MONITOR_RSP = 1411,
    MONITOR_DATA = 1412,
    MONITOR_CLAIM = 1413,
    MONITOR_CLAIM_RSP = 1414,
    TALK_REQ = 1430,
    TALK_RSP = 1431,
    TALK_CU_PU_DATA = 1432,
    TALK_PU_CU_DATA = 1433,
    TALK_CLAIM = 1434,
    TALK_CLAIM_RSP = 1435,
    SYSMANAGER_REQ = 1450,
    SYSMANAGER_RSP = 1451,
    TIMEQUERY_REQ = 1452,
    TIMEQUERY_RSP = 1453,
    FULLAUTHORITYLIST_GET = 1470,
    FULLAUTHORITYLIST_GET_RSP = 1471,
    USERS_GET = 1472,
    USERS_GET_RSP = 1473,
    GROUPS_GET = 1474,
    GROUPS_GET_RSP = 1475,
    GUARD_REQ = 1500,	
    GUARD_RSP = 1501,
    UNGUARD_REQ	= 1502,
    UNGUARD_RSP	= 1503,
    ALARM_REQ = 1504,
    IPSEARCH_REQ = 1530,
    IPSEARCH_RSP = 1531,
    IP_SET_REQ = 1532,
    IP_SET_RSP = 1533,
    SET_IFRAME_REQ = 1562,
    SET_IFRAME_RSP = 1563,
    SYNC_TIME_REQ = 1590,
    SYNC_TIME_RSP = 1591,
}XMAI_MESSAGE_ID;

/// 捕获压缩格式类型
typedef enum {
	SDK_CAPTURE_COMP_DIVX_MPEG4,	///< DIVX MPEG4。
	SDK_CAPTURE_COMP_MS_MPEG4,		///< MS MPEG4。
	SDK_CAPTURE_COMP_MPEG2,			///< MPEG2。
	SDK_CAPTURE_COMP_MPEG1,			///< MPEG1。
	SDK_CAPTURE_COMP_H263,			///< H.263
	SDK_CAPTURE_COMP_MJPG,			///< MJPG
	SDK_CAPTURE_COMP_FCC_MPEG4,		///< FCC MPEG4
	SDK_CAPTURE_COMP_H264,			///< H.264
	SDK_CAPTURE_COMP_NR				///< 枚举的压缩标准数目。
}SDK_CAPTURE_COMP_t;

typedef enum {
	SDK_CAPTURE_SIZE_D1,		///< 704*576(PAL)	704*480(NTSC)
	SDK_CAPTURE_SIZE_HD1,		///< 704*288(PAL)	704*240(NTSC)
	SDK_CAPTURE_SIZE_BCIF,		///< 720*288(PAL)	720*240(NTSC)
	SDK_CAPTURE_SIZE_CIF,		///< 352*288(PAL)	352*240(NTSC)
	SDK_CAPTURE_SIZE_QCIF,		///< 176*144(PAL)	176*120(NTSC)
	SDK_CAPTURE_SIZE_VGA,		///< 640*480(PAL)	640*480(NTSC)
	SDK_CAPTURE_SIZE_QVGA,		///< 320*240(PAL)	320*240(NTSC)
	SDK_CAPTURE_SIZE_SVCD,		///< 1024*768
	SDK_CAPTURE_SIZE_QQVGA,		///< 160*128(PAL)	160*128(NTSC)
	SDK_CAPTURE_SIZE_ND1 = 9,     ///< 240*192
	SDK_CAPTURE_SIZE_650TVL,      ///< 960*576 (960H)
	SDK_CAPTURE_SIZE_720P,        ///< 1280*720
	SDK_CAPTURE_SIZE_1_3M,        ///< 1280*960  (960P)
	SDK_CAPTURE_SIZE_UXGA ,       ///< 1900*1200
	SDK_CAPTURE_SIZE_1080P,       ///< 1920*1080
	SDK_CAPTURE_SIZE_WUXGA,       ///< 1920*1200
	SDK_CAPTURE_SIZE_2_5M,        ///< 1872*1408
	SDK_CAPTURE_SIZE_3M,          ///< 2048*1536
	SDK_CAPTURE_SIZE_5M,          ///< 3744*1408
	SDK_CAPTURE_SIZE_NR,
}SDK_CAPTURE_SIZE_t;

///////////////////////////
typedef struct tagVideoIFrameHeader{
    char flag1;
    char flag2;
    char flag3;
    char flag4;
    char flagT;
    char framebit;
    char width;
    char height;
    int  time;
    int  len;
} VideoIFrameHeader;

typedef struct tagVideoPFrameHeader{
    char flag1;
    char flag2;
    char flag3;
    char flag4;
    int  len;
}VideoPFrameHeader;

typedef struct tagAudioDataHeader{
    char flag1;
    char flag2;
    char flag3;
    char flag4;
    char flagT;
    char sampleRate;
    short  len;
}AudioDataHeader;

typedef struct tagXMaiMsgHeader{
    char headFlag;
    char version;
    char reserved1;
    char reserved2;
    int  sessionId;
    int  sequenceNum;
    char totalPacket;
    char curPacket;
    short messageId;
    int dataLen;
} XMaiMsgHeader;

typedef struct tagXMaiMediaStreamMsgHeader{
    char headFlag;
    char version;
    char reserved1;
    char reserved2;
    int  sessionId;
    int  sequenceNum;
    char channel;
    char endflag;
    short messageId;
    int dataLen;
} XMaiMediaStreamMsgHeader;

typedef struct tagXMaiFrameInfo{
    int seq;
    int channel;
    int width;
    int height;
    int fps;
    int frame_len;
    char *pData;
} XMaiFrameInfo;


typedef enum{
    UNINIT_SESSION = 0,
    CMD_SESSION,
    CONNECT_SESSION,
} SESSION_TYPE;

typedef struct tagXMaiSessionCtrl{
	//int flag;
	int fSessionInt;
	char fSessionId[16];
	int accept_sock;
	//XMaiLvSocket lv_sock;
	//int lv_sub_stream_sock;
	int fPeerSeqNum;
	int fSequenceNum;
	char fSendbuf[XMAI_MSG_SEND_BUF_SIZE];
	SESSION_TYPE fSessionType;
} XMaiSessionCtrl;	

typedef enum{
    XMAI_MAIN_STREAM = 0,
    XMAI_SUB_STREAM,
    XMAI_UNKNOWN,
} XMAI_STREAM_TYPE;

typedef struct tagXMaiLvSocketInfo{
	int n_socket; 					
	int n_flag;	// 0关闭，1打开
} XMaiLvSocketInfo;

typedef struct tagXMaiLvSocket{
	XMaiLvSocketInfo main_stream_socket;
	XMaiLvSocketInfo sub_stream_socket;
} XMaiLvSocket;

typedef struct tagXMaiTalkSocket{
	int n_socket; 					
} XMaiTalkSocket;

typedef struct tagSessionGlobal{
	int flag;
	int fSessionInt;
	int main_sock;
	XMaiLvSocket lv_sock;	
	XMaiTalkSocket talk_sock;	
} XMaiSessionGlobal;

typedef struct tagXMaiTimetick
{
	DWORD second            :6;  // 秒      1-60
	DWORD minute            :6;  // 分      1-60
	DWORD hour              :5;  // 时      1-24
	DWORD day               :5;  // 日      1-31
	DWORD month             :4;  // 月      1-12
	DWORD year              :6;  // 年      2000为基准，0-63
} XMaiTimetick;

enum
{
	DMS_ALARM_TYPE_ALARMIN = 0,     //0:信号量报警开始
    DMS_ALARM_TYPE_DISK_FULL,       //1:硬盘满
    DMS_ALARM_TYPE_VLOST,           //2:信号丢失
	DMS_ALARM_TYPE_VMOTION,         //3:移动侦测
	DMS_ALARM_TYPE_DISK_UNFORMAT,   //4:硬盘未格式化
	DMS_ALARM_TYPE_DISK_RWERR,      //5:读写硬盘出错,
	DMS_ALARM_TYPE_VSHELTER,        //6:遮挡报警
	DMS_ALARM_TYPE_VFORMAT_UNMATCH, //7:制式不匹配 
	DMS_ALARM_TYPE_ILLEGAL_ACCESS,  //8:非法访问
	DMS_ALARM_TYPE_ALARMIN_RESUME,  //9:信号量报警恢复
	DMS_ALARM_TYPE_VLOST_RESUME,    //10:视频丢失报警恢复
	DMS_ALARM_TYPE_VMOTION_RESUME,  //11:视频移动侦测报警恢复
	DMS_ALARM_TYPE_NET_BROKEN,      //12:网络断开
	DMS_ALARM_TYPE_IP_CONFLICT     //13:IP冲突
};

typedef	void *(*ThreadEntryPtrType)(void *);

#endif
