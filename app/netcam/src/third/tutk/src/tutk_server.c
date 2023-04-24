#include <stdio.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "IOTCAPIs.h"
#include "AVAPIs.h"
#include "AVFRAMEINFO.h"
#include "AVIOCTRLDEFs.h"
#include "tutk_access_export.h"
#include "tutk_debug.h"
#include "tutk_push_func.h"
#include "tutk_net_update.h"

#include "netcam_api.h"
#include "media_fifo.h"
#include "cfg_all.h"
#include "ntp_client.h"
#include "parser_cfg.h"
#include "flash_ctl.h"
#include "work_queue.h"
#include "mmc_api.h"
#include "avi_search_api.h"
#include "avi_utility.h"
#include "eventalarm.h"
#include "utility_api.h"
#include "ipcsearch.h"
#include "sdk_cfg.h"
#include "md5.h"


/*录像回放功能，宏天顺和柔乐&徐辉的不一样*/
//#define USE_RL_PROTOCOL 1
//#define USE_HTS_PROTOCOL 0

#define USE_V_ADJUST 0
#define TUTK_DEBUG 0
int tutk_odmID = 10;
char *tutk_model = "GOKE_YTJ_AP_001";

#ifdef AUDIO_ECHO_CANCELLATION_SUPPORT
#include "zfifo.h"
#endif

#ifdef MODULE_SUPPORT_RF433
#include "rf433.h"
#endif


#define SERVTYPE_STREAM_SERVER       0
#define MAX_CLIENT_NUMBER	         8
#define MAX_SIZE_IOCTRL_BUF		     1024
#define AUDIO_USED_G711A    		 1
#define VIN_NUM						 1
#define DASA_ENABLED				 1
#define AV_SERV_START_TIME_OUT       4
#define MAX_ALARM_EVENT 			 60*24

#define SET_THREAD_NAME() \
    prctl(PR_SET_NAME, (unsigned long)__func__, 0,0,0)


#define VIDEO_BUF_SIZE	(1024 * 300)
#define AUDIO_BUF_SIZE	4096

typedef enum
{
    E_PB_BY_FILE = 0,
    E_PB_BY_TIME,
} ENUM_PB_TYPE;

typedef struct _AV_Client
{
	int avIndex;
	unsigned char bEnableAudio;
	unsigned char bEnableVideo;
	unsigned char bEnableSpeaker;
	unsigned char bStopPlayBack;
	unsigned char bPausePlayBack;
	int speakerCh;
	int playBackCh;
	int speakerAvIndex;
	
	SMsgAVIoctrlPlayRecord playRecord;
	pthread_mutex_t sLock;
	
	unsigned char bSensorNo;
	unsigned char bStreamNo;
	unsigned char bQuality;
	unsigned char bPBSensorNo;

	unsigned char bNextIFrame;
    unsigned char bNewConnection;
	unsigned char bSeekTimeCmd;
	AVI_DMS_TIME  stSeekTimeDay;
}AV_Client;

typedef struct _AV_FRAME{
    char* addr;    ///< the start address of the frame
    unsigned int type;
    unsigned int size;      ///< the size of the frame
    unsigned int ts;        ///< time stamp in the video(video start with ts = 0), If need the absolute timestamp from system powerup, do in system level.
}AV_FRAME;

typedef struct {

	u64t start;
	u64t end;
	unsigned char flag;//0:all event;1:alarm event
}stTime;


static AV_Client gClientInfo[MAX_CLIENT_NUMBER];
#if 0
static pthread_mutex_t gVideoMutex;
static pthread_cond_t  gVideoCond;
static pthread_mutex_t gAudioMutex;
static pthread_cond_t  gAudioCond;
#endif
static int gOnlineNum = 0;
#if 0
static unsigned char audioInEnFlag = 1;
static unsigned char audioOutEnFlag = 1;
#endif
AviPBHandle* pbhandle[MAX_CLIENT_NUMBER] = {NULL};
//AviPBHandle* pbhandle = NULL;
static unsigned long long local_now_ver = 0;

static unsigned int odm_id = ODM_DS;
static unsigned char model[16];


static pthread_mutex_t gSendVideoStreamMutex = PTHREAD_MUTEX_INITIALIZER;

//static pthread_mutex_t tutk_pb_getframe_x = PTHREAD_MUTEX_INITIALIZER;
//static int is_pb_doing = 0;



#define PUSH_CONF "/opt/custom/cfg/msgpush.dat"
#define PUSH_MAX  2

#define GET_LOCK(X) pthread_mutex_lock(X)
#define PUT_LOCK(X) pthread_mutex_unlock(X)
#define TRY_LOCK(X) pthread_mutex_trylock(X)

//
// 移动侦测报警生效时间
//
#if 0
typedef struct IDC_ALARM_SCHEDULE{
	int				StartHour;
	int				CloseHour;
	int				StartMins;
	int				CloseMins;
	int				AlarmFrequency;
}DAS,*PDAS;
#endif

//static DAS sDAS;
//static int LastAlarmTime = 0;
static int format_runing = 0;
static SMsgAVIoctrlSetPushReq PushDevs[PUSH_MAX];
static pthread_mutex_t PushDevsLock = PTHREAD_MUTEX_INITIALIZER;
//static pthread_mutex_t PushExecLock = PTHREAD_MUTEX_INITIALIZER;
//static pthread_mutex_t WorkStatLock = PTHREAD_MUTEX_INITIALIZER;

static const int tutk_time_zone_table[] = {
	-12*60, -11*60, -10*60, -9*60, -8*60, -7*60, -6*60,
	-5*60, -4*60-30, -4*60, -3*60-30, -3*60, -2*60,
	-60, 0, 60, 2*60, 3*60, 3*60+30, 4*60, 4*60+30, 5*60,
	5*60+30, 5*60+45, 6*60, 6*60+30, 7*60, 8*60, 9*60,
	9*60+30, 10*60, 11*60, 12*60
};
#if 1
static int gSpeakerStart = 0;
volatile char loadPushEndFlg = 0;
struct sockaddr_in gPushMsgSrvAddr;
static char gUID[21];
static MEDIABUF_HANDLE AVHandleStream0[VIN_NUM][MAX_CLIENT_NUMBER];
static MEDIABUF_HANDLE AVHandleStream1[VIN_NUM][MAX_CLIENT_NUMBER];
static void tutk_venc_change_stream_attr(int vi,int streamId,int level);

static char *GetPushMessageString(char *UID, int eventType)
{
	static char msgBuf[2048];

	sprintf(msgBuf, "GET /apns/apns.php?cmd=raise_event&uid=%s&event_type=%d&event_time=%lu HTTP/1.1\r\n"
	"Host: %s\r\n"
	"Connection: keep-alive\r\n"
	"User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.52 Safari/536.5\r\n"
	"Accept: */*\r\n"
	"Accept-Encoding: gzip,deflate,sdch\r\n"
	"Accept-Language: zh-TW,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n"
	"Accept-Charset: Big5,utf-8;q=0.7,*;q=0.3\r\n"
	"Pragma: no-cache\r\n"
	"Cache-Control: no-cache\r\n"
	"\r\n", UID, eventType, time((time_t *)NULL), inet_ntoa(gPushMsgSrvAddr.sin_addr));

	return msgBuf;
}

static void SendPushMessage(int eventType)
{
	int skt;
    int ret = 0;
	if(gPushMsgSrvAddr.sin_addr.s_addr == 0)
	{
		printf("No push message server\n");
		return;
	}
	if((skt =(int) socket(AF_INET, SOCK_STREAM, 0)) >= 0)
	{
		if(connect(skt, (struct sockaddr *)&gPushMsgSrvAddr, sizeof(struct sockaddr_in)) == 0)
		{
		
            char buf[1024];
			printf("connect HTTP OK\n");
			char *msg = GetPushMessageString(gUID, eventType);
            
			printf("SendPushMessage  = %s\n", msg);
			ret = send(skt, msg, strlen(msg), 0);

            recv(skt, buf, 1024, 0);
			printf("SendPushMessage ret = %s\n", buf);
            printf("SendPushMessage send ret :%d\n",ret);
		}

		close(skt);
	}
}

static char *GetRegMessageString(char *UID)
{
	static char msgBuf[2048];
	sprintf(msgBuf, "GET /apns/apns.php?cmd=reg_server&uid=%s HTTP/1.1\r\n"
	"Host: %s\r\n"
	"Connection: keep-alive\r\n"
	"User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.52 Safari/536.5\r\n"
	"Accept: */*\r\n"
	"Accept-Encoding: gzip,deflate,sdch\r\n"
	"Accept-Language: zh-TW,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n"
	"Accept-Charset: Big5,utf-8;q=0.7,*;q=0.3\r\n"
	"Pragma: no-cache\r\n"
	"Cache-Control: no-cache\r\n"
	"\r\n", UID, inet_ntoa(gPushMsgSrvAddr.sin_addr));

	return msgBuf;
}


char *gDnsIP[4]={"54.149.183.34","54.149.194.214","54.191.44.244","54.149.150.149"};
static void SendRegister()
{
    int ret =0;
	struct hostent *host = gethostbyname("push.iotcplatform.com");
	if(host != NULL)
	{
		memcpy(&gPushMsgSrvAddr.sin_addr, host->h_addr_list[0], host->h_length);
        PRINT_INFO("ip[0]:%s\n",inet_ntoa(gPushMsgSrvAddr.sin_addr));
		gPushMsgSrvAddr.sin_port = htons(80);
		gPushMsgSrvAddr.sin_family = AF_INET;
        //printf("SendRegister addr :%s\n",gPushMsgSrvAddr.sin_addr);
	}
	else
	{
		printf("faile to resolve,SendRegister failed \n");
		memset(&gPushMsgSrvAddr, 0, sizeof(gPushMsgSrvAddr));
		return;
	}

	int skt;
	if((skt =(int) socket(AF_INET, SOCK_STREAM, 0)) >= 0)
	{
		if(connect(skt, (struct sockaddr *)&gPushMsgSrvAddr, sizeof(struct sockaddr_in)) == 0)
		{

			char *msg = GetRegMessageString(gUID);
            char buf[1024];
			printf("SendRegister send %s \n", msg);
			ret = send(skt, msg, strlen(msg), 0);
            printf("SendRegister send ret=%d\n",ret);
			recv(skt, buf, 1024, 0);
			printf("Reg = %s\n", buf);
			printf("Register OK\n");
		}

		close(skt);
	}
}
#endif


//
// 加载用户推送信息
//
static int LoadPushDevs(void)
{
	int ret = -1,i = 0;
	
	FILE * FileHandle = fopen(PUSH_CONF,"rb");
	if(FileHandle == NULL)
	{
		P2P_ERR("[p2p] OPEN %s TO LOAD DATA FAILED.\n",PUSH_CONF);
		return ret;
	}

	GET_LOCK(&PushDevsLock);

	memset(&PushDevs,0,sizeof(SMsgAVIoctrlSetPushReq)*PUSH_MAX);

	for(i=0; i<PUSH_MAX; i++)
	{
		int nBytes = fread(&PushDevs[i],1,sizeof(SMsgAVIoctrlSetPushReq),FileHandle);
		if(sizeof(SMsgAVIoctrlSetPushReq) != nBytes)
		{
            P2P_ERR("[p2p] fread %s error.\n", PUSH_CONF);
			break;
		}
		
		PRINT_INFO("[p2p] index:%d, LOAD DB PUSH ALIAS:[%s].\n", i, PushDevs[i].Alias);

		if( strlen(PushDevs[i].Alias) == 0 || 
			strlen(PushDevs[i].AppKey) == 0 || 
			strlen(PushDevs[i].Master) == 0 ||
			PushDevs[i].Type != BELL_MOTION_DETECTIVE)
		{
			memset(&PushDevs[i],0,sizeof(SMsgAVIoctrlSetPushReq));
			continue;
		}
		else
		{
			ret = 0;
		}
	}

	PUT_LOCK(&PushDevsLock);
    fclose(FileHandle);
	return ret;
}

//
// 保存用户推送信息
//
static int SavePushDevs(void)
{
	int i = 0;

	FILE * FileHandle = fopen(PUSH_CONF,"wb");
	if(FileHandle == NULL){
		PRINT_INFO("OPEN %s TO SAVE DATA FAILED.\n",PUSH_CONF);
		return -1;
	}

	GET_LOCK(&PushDevsLock);
	
	for(i=0;i<PUSH_MAX;i++)
	{
		if(strlen(PushDevs[i].Alias) != 0 && strlen(PushDevs[i].AppKey) != 0 && strlen(PushDevs[i].Master) != 0)
		{
			int nBytes = fwrite((const char*)&PushDevs[i],1,sizeof(SMsgAVIoctrlSetPushReq),FileHandle);
			if(sizeof(SMsgAVIoctrlSetPushReq) != nBytes)
			{
				PRINT_INFO("SAVE DB ALIAS:[%s] FAILED. [%d][%d]\n",PushDevs[i].Alias,sizeof(SMsgAVIoctrlSetPushReq),nBytes);
				break;
			}

			PRINT_INFO("SAVE DB ALIAS:[%s].\n",PushDevs[i].Alias);
		}
	}

	PUT_LOCK(&PushDevsLock);

	fclose(FileHandle);

	return 0;
}
static int speaker_start(void)
{
    if(gSpeakerStart == 1){
        return -1;
    }

    gSpeakerStart = 1;
    return 0;
}

static void speaker_stop(void)
{
    gSpeakerStart = 0;
}

unsigned int audio_alaw_decode(short *dst, const unsigned char *src, unsigned int srcSize);


void PrintErrHandling (int nErr)
{
	switch (nErr)
	{
	case IOTC_ER_SERVER_NOT_RESPONSE :
		//-1 IOTC_ER_SERVER_NOT_RESPONSE
		PRINT_INFO ("[Error code : %d]\n", IOTC_ER_SERVER_NOT_RESPONSE );
		PRINT_INFO ("Master doesn't respond.\n");
		PRINT_INFO ("Please check the network wheather it could connect to the Internet.\n");
		break;
	case IOTC_ER_FAIL_RESOLVE_HOSTNAME :
		//-2 IOTC_ER_FAIL_RESOLVE_HOSTNAME
		PRINT_INFO ("[Error code : %d]\n", IOTC_ER_FAIL_RESOLVE_HOSTNAME);
		PRINT_INFO ("Can't resolve hostname.\n");
		break;
	case IOTC_ER_ALREADY_INITIALIZED :
		//-3 IOTC_ER_ALREADY_INITIALIZED
		PRINT_INFO ("[Error code : %d]\n", IOTC_ER_ALREADY_INITIALIZED);
		PRINT_INFO ("Already initialized.\n");
		break;
	case IOTC_ER_FAIL_CREATE_MUTEX :
		//-4 IOTC_ER_FAIL_CREATE_MUTEX
		PRINT_INFO ("[Error code : %d]\n", IOTC_ER_FAIL_CREATE_MUTEX);
		PRINT_INFO ("Can't create mutex.\n");
		break;
	case IOTC_ER_FAIL_CREATE_THREAD :
		//-5 IOTC_ER_FAIL_CREATE_THREAD
		PRINT_INFO ("[Error code : %d]\n", IOTC_ER_FAIL_CREATE_THREAD);
		PRINT_INFO ("Can't create thread.\n");
		break;
	case IOTC_ER_UNLICENSE :
		//-10 IOTC_ER_UNLICENSE
		PRINT_INFO ("[Error code : %d]\n", IOTC_ER_UNLICENSE);
		PRINT_INFO ("This UID is unlicense.\n");
		PRINT_INFO ("Check your UID.\n");
		break;
	case IOTC_ER_NOT_INITIALIZED :
		//-12 IOTC_ER_NOT_INITIALIZED
		PRINT_INFO ("[Error code : %d]\n", IOTC_ER_NOT_INITIALIZED);
		PRINT_INFO ("Please initialize the IOTCAPI first.\n");
		break;
	case IOTC_ER_TIMEOUT :
		//-13 IOTC_ER_TIMEOUT
		break;
	case IOTC_ER_INVALID_SID :
		//-14 IOTC_ER_INVALID_SID
		PRINT_INFO ("[Error code : %d]\n", IOTC_ER_INVALID_SID);
		PRINT_INFO ("This SID is invalid.\n");
		PRINT_INFO ("Please check it again.\n");
		break;
	case IOTC_ER_EXCEED_MAX_SESSION :
		//-18 IOTC_ER_EXCEED_MAX_SESSION
		PRINT_INFO ("[Error code : %d]\n", IOTC_ER_EXCEED_MAX_SESSION);
		PRINT_INFO ("[Warning]\n");
		PRINT_INFO ("The amount of session reach to the maximum.\n");
		PRINT_INFO ("It cannot be connected unless the session is released.\n");
		break;
	case IOTC_ER_CAN_NOT_FIND_DEVICE :
		//-19 IOTC_ER_CAN_NOT_FIND_DEVICE
		PRINT_INFO ("[Error code : %d]\n", IOTC_ER_CAN_NOT_FIND_DEVICE);
		PRINT_INFO ("Device didn't register on server, so we can't find device.\n");
		PRINT_INFO ("Please check the device again.\n");
		PRINT_INFO ("Retry...\n");
		break;
	case IOTC_ER_SESSION_CLOSE_BY_REMOTE :
		//-22 IOTC_ER_SESSION_CLOSE_BY_REMOTE
		PRINT_INFO ("[Error code : %d]\n", IOTC_ER_SESSION_CLOSE_BY_REMOTE);
		PRINT_INFO ("Session is closed by remote so we can't access.\n");
		PRINT_INFO ("Please close it or establish session again.\n");
		break;
	case IOTC_ER_REMOTE_TIMEOUT_DISCONNECT :
		//-23 IOTC_ER_REMOTE_TIMEOUT_DISCONNECT
		PRINT_INFO ("[Error code : %d]\n", IOTC_ER_REMOTE_TIMEOUT_DISCONNECT);
		PRINT_INFO ("We can't receive an acknowledgement character within a TIMEOUT.\n");
		PRINT_INFO ("It might that the session is disconnected by remote.\n");
		PRINT_INFO ("Please check the network wheather it is busy or not.\n");
		PRINT_INFO ("And check the device and user equipment work well.\n");
		break;
	case IOTC_ER_DEVICE_NOT_LISTENING :
		//-24 IOTC_ER_DEVICE_NOT_LISTENING
		PRINT_INFO ("[Error code : %d]\n", IOTC_ER_DEVICE_NOT_LISTENING);
		PRINT_INFO ("Device doesn't listen or the sessions of device reach to maximum.\n");
		PRINT_INFO ("Please release the session and check the device wheather it listen or not.\n");
		break;
	case IOTC_ER_CH_NOT_ON :
		//-26 IOTC_ER_CH_NOT_ON
		PRINT_INFO ("[Error code : %d]\n", IOTC_ER_CH_NOT_ON);
		PRINT_INFO ("Channel isn't on.\n");
		PRINT_INFO ("Please open it by IOTC_Session_Channel_ON() or IOTC_Session_Get_Free_Channel()\n");
		PRINT_INFO ("Retry...\n");
		break;
	case IOTC_ER_SESSION_NO_FREE_CHANNEL :
		//-31 IOTC_ER_SESSION_NO_FREE_CHANNEL
		PRINT_INFO ("[Error code : %d]\n", IOTC_ER_SESSION_NO_FREE_CHANNEL);
		PRINT_INFO ("All channels are occupied.\n");
		PRINT_INFO ("Please release some channel.\n");
		break;
	case IOTC_ER_TCP_TRAVEL_FAILED :
		//-32 IOTC_ER_TCP_TRAVEL_FAILED
		PRINT_INFO ("[Error code : %d]\n", IOTC_ER_TCP_TRAVEL_FAILED);
		PRINT_INFO ("Device can't connect to Master.\n");
		PRINT_INFO ("Don't let device use proxy.\n");
		PRINT_INFO ("Close firewall of device.\n");
		PRINT_INFO ("Or open device's TCP port 80, 443, 8080, 8000, 21047.\n");
		break;
	case IOTC_ER_TCP_CONNECT_TO_SERVER_FAILED :
		//-33 IOTC_ER_TCP_CONNECT_TO_SERVER_FAILED
		PRINT_INFO ("[Error code : %d]\n", IOTC_ER_TCP_CONNECT_TO_SERVER_FAILED);
		PRINT_INFO ("Device can't connect to server by TCP.\n");
		PRINT_INFO ("Don't let server use proxy.\n");
		PRINT_INFO ("Close firewall of server.\n");
		PRINT_INFO ("Or open server's TCP port 80, 443, 8080, 8000, 21047.\n");
		PRINT_INFO ("Retry...\n");
		break;
	case IOTC_ER_NO_PERMISSION :
		//-40 IOTC_ER_NO_PERMISSION
		PRINT_INFO ("[Error code : %d]\n", IOTC_ER_NO_PERMISSION);
		PRINT_INFO ("This UID's license doesn't support TCP.\n");
		break;
	case IOTC_ER_NETWORK_UNREACHABLE :
		//-41 IOTC_ER_NETWORK_UNREACHABLE
		PRINT_INFO ("[Error code : %d]\n", IOTC_ER_NETWORK_UNREACHABLE);
		PRINT_INFO ("Network is unreachable.\n");
		PRINT_INFO ("Please check your network.\n");
		PRINT_INFO ("Retry...\n");
		break;
	case IOTC_ER_FAIL_SETUP_RELAY :
		//-42 IOTC_ER_FAIL_SETUP_RELAY
		PRINT_INFO ("[Error code : %d]\n", IOTC_ER_FAIL_SETUP_RELAY);
		PRINT_INFO ("Client can't connect to a device via Lan, P2P, and Relay mode\n");
		break;
	case IOTC_ER_NOT_SUPPORT_RELAY :
		//-43 IOTC_ER_NOT_SUPPORT_RELAY
		PRINT_INFO ("[Error code : %d]\n", IOTC_ER_NOT_SUPPORT_RELAY);
		PRINT_INFO ("Server doesn't support UDP relay mode.\n");
		PRINT_INFO ("So client can't use UDP relay to connect to a device.\n");
		break;

	default :
		break;
	}
}

static int AuthCallBackFn(char *viewAcc,char *viewPwd)
{
	PRINT_INFO("[%s],[%s] [%s],[%s]\n", viewAcc, viewPwd,
		runUserCfg.user[0].userName,
		runUserCfg.user[0].password);
	/*比较有问题，不能按长度比较。因为可能长度不同，但是前面的字串是相同的话，可能导致比较字串相同*/
    if( strlen(runUserCfg.user[0].userName) == strlen(viewAcc) 
    && strlen(runUserCfg.user[0].password) == strlen(viewPwd))
    {
        if(strcmp(runUserCfg.user[0].userName, viewAcc) == 0
    	&& strcmp(runUserCfg.user[0].password, viewPwd) == 0) 
    	{
    		return 1;
    	}
    }
	return 0;
}

#if 1//USE_RL_PROTOCOL
static void regedit_client_to_video(int SID, int avIndex)
{
#if 0
	pthread_mutex_lock(&gVideoMutex);

	AV_Client *p = &gClientInfo[SID];
	p->avIndex = avIndex;
	p->bEnableVideo = 1;

	//pthread_cond_signal(&gVideoCond);
	//pthread_cond_broadcast(&gVideoCond);
	pthread_mutex_unlock(&gVideoMutex);
#else

	AV_Client *p = &gClientInfo[SID];
	p->avIndex = avIndex;
	p->bEnableVideo = 1;
	p->bNextIFrame = 1;
	p->bNewConnection = 1;

#endif
}

#else
static void regedit_client_to_video2(int SID, int avIndex, unsigned char stream_id,unsigned char audioEnable)
{
	pthread_mutex_lock(&gVideoMutex);

	AV_Client *p = &gClientInfo[SID];
	p->avIndex = avIndex;
	p->bEnableVideo = 1;
    p->bVideoOpenStreamId = stream_id;
    p->bVideoSendKeyFrame = 1;
    sdk_enc_request_stream_h264_keyframe(0,stream_id);
    PRINT_INFO("stream_id:%d,audio:%d\n",stream_id,audioEnable);
    if(1 == audioEnable)
    {
        p->bEnableAudio = 1; 
    	pthread_cond_signal(&gAudioCond);
    }
    else
    {   
        p->bEnableAudio = 0; 
    }
	//pthread_cond_signal(&gVideoCond);
	//pthread_cond_broadcast(&gVideoCond);
	pthread_mutex_unlock(&gVideoMutex);
}
#endif

static void unregedit_client_from_video(int SID)
{
#if 0
	pthread_mutex_lock(&gVideoMutex);
	AV_Client *p = &gClientInfo[SID];
	p->bEnableVideo = 0;
	p->bEnableAudio = 0;

	pthread_mutex_unlock(&gVideoMutex);
#else

	AV_Client *p = &gClientInfo[SID];
	p->bEnableVideo = 0;

#endif
}

static void regedit_client_to_audio(int SID, int avIndex)
{
#if 0
	pthread_mutex_lock(&gAudioMutex);

	AV_Client *p = &gClientInfo[SID];
	p->bEnableAudio = 1;

	pthread_cond_signal(&gAudioCond);
	pthread_mutex_unlock(&gAudioMutex);
#else

	AV_Client *p = &gClientInfo[SID];
	p->bEnableAudio = 1;

#endif
}

static void unregedit_client_from_audio(int SID)
{
#if 0
	pthread_mutex_lock(&gAudioMutex);

	AV_Client *p = &gClientInfo[SID];
	p->bEnableAudio = 0;

	pthread_mutex_unlock(&gAudioMutex);
#else

	AV_Client *p = &gClientInfo[SID];
	p->bEnableAudio = 0;

#endif
}

#ifdef AUDIO_ECHO_CANCELLATION_SUPPORT
extern ZFIFO_DESC *writer;
#endif

#define SUPPORT_OPUS 1

#ifdef SUPPORT_OPUS
#include "opus.h"
#endif

#if 1
static void * thread_ReceiveAudio(void *arg)
{
    SET_THREAD_NAME();
	int SID = (int)arg;
	int avIndex, nResend = 0;
	unsigned int servType, frmNo = 0;
	AV_Client *p;
	char buf[AUDIO_BUF_SIZE] = {0};
	char dec[AUDIO_BUF_SIZE*4] = {0};
	int  declens = 0;
	FRAMEINFO_t frameInfo;

	int error = -1;
    
    #ifdef SUPPORT_OPUS
	OpusDecoder * hOpusDec = NULL;
	hOpusDec = opus_decoder_create(8000,1,&error);
	if(error != OPUS_OK){
		PRINT_INFO("[p2p] opus decoder init failed.\n");
		speaker_stop();
		return NULL;
	}
    #endif
    
	int adpcm_pre_sample = 0;
	int adpcm_idx = 0;

	//SET_THREAD_NAME();
	p = &gClientInfo[SID];
	avIndex = avClientStart2(SID, NULL, NULL, 30, &servType, p->speakerCh, &nResend);
	PRINT_INFO("[thread_ReceiveAudio] start ok! SID[%d], avIndex[%d], channel[%d]\n", SID, avIndex, p->speakerCh);

	if (avIndex < 0)
	{
		speaker_stop();
		return NULL;
	}	

	avClientCleanAudioBuf(p->speakerCh);
	while(p->bEnableSpeaker)
	{
	#if 0
        //喇叭开关控制。禁止时，轮空
        if(audioOutEnFlag == 0)
        {
            PRINT_INFO(" audio disable for all\n");
            usleep(200000);
            continue;
        }
	#endif
		int ret = avRecvAudioData(avIndex, buf, AUDIO_BUF_SIZE, (char*)&frameInfo, sizeof(FRAMEINFO_t), &frmNo);
        //PRINT_INFO(" ret:%d for avRecvAudioData\n",ret);

//		PRINT_INFO("frameInfo.codec_id:0x%x, ret:%d\n", frameInfo.codec_id, ret);
		if(ret == AV_ER_SESSION_CLOSE_BY_REMOTE)
		{
			PRINT_INFO("avRecvAudioData AV_ER_SESSION_CLOSE_BY_REMOTE\n");
			break;
		}
		else if(ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT)
		{
			PRINT_INFO("avRecvAudioData AV_ER_REMOTE_TIMEOUT_DISCONNECT\n");
			break;
		}
		else if(ret == IOTC_ER_INVALID_SID)
		{
			PRINT_INFO("avRecvAudioData Session[%d] cant be used anymore\n", SID);
			break;
		}
		else if(ret == AV_ER_LOSED_THIS_FRAME)
		{
			PRINT_INFO("Audio LOST[%d] \n", frmNo);
			continue;
		}
		else if (ret == AV_ER_DATA_NOREADY)
		{
			usleep(10000);
			continue;
		}
		else if(ret < 0)
		{
			PRINT_INFO("Other error[%d]!!!\n", ret);
			break;
		}

        #ifdef AUDIO_ECHO_CANCELLATION_SUPPORT

        PRINT_INFO(" audio support aec\n");

        if (writer)
        {
            
    		switch (frameInfo.codec_id) {
    			case MEDIA_CODEC_AUDIO_PCM:
    				audiobuf_write_frame(writer,buf,ret);
    				break;
                case MEDIA_CODEC_AUDIO_G711A:

                    declens = audio_alaw_decode((short*)dec, (unsigned char*)buf, ret);
                    audiobuf_write_frame(writer,dec,declens);
                    break;
                    
    			case MEDIA_CODEC_AUDIO_ADPCM:
    				adpcm_pre_sample = 0;
    				adpcm_idx = 0;
    				adpcm_decode(buf, ret, dec, &adpcm_pre_sample, &adpcm_idx);
    				audiobuf_write_frame(writer, dec, ret * 4);
    				break;
                #ifdef SUPPORT_OPUS
    			case MEDIA_CODEC_AUDIO_OPUS:
    				declens = opus_decode(hOpusDec, buf, ret, (opus_int16 *)dec, sizeof(dec), 0);
    				if (declens < 0) {
    					PRINT_INFO("opus decode failed.\n");
    					break;
    				}
    				audiobuf_write_frame(writer, dec, declens * 2);
    				break;
                #endif
    			default:
    				audiobuf_write_frame(writer, buf, ret);
    				break;
    		}
        }
        else
        {
            PRINT_INFO("writer is not exist.\n");
        }

        #else

		if (frameInfo.codec_id == MEDIA_CODEC_AUDIO_PCM)
        {      
            //PRINT_INFO("output audio, format: pcm\n");
            netcam_audio_output(buf, ret, NETCAM_AUDIO_ENC_RAW_PCM, SDK_TRUE);
        }
		if (frameInfo.codec_id == MEDIA_CODEC_AUDIO_G711A)
        {      
            //PRINT_INFO("output audio, format: alaw\n");
            netcam_audio_output(buf, ret, NETCAM_AUDIO_ENC_A_LAW, SDK_TRUE);
        }

        
        #endif

	}


	PRINT_INFO("[thread_ReceiveAudio] exit\n");
	avClientStop(avIndex);

    #ifdef SUPPORT_OPUS
	opus_decoder_destroy(hOpusDec);
    #endif
	
	speaker_stop();
	
	return NULL;
}

#endif

static void time_convert(STimeDay* td, int zone)
{
	struct tm temp;
	time_t tt;

	temp.tm_year = td->year - 1900;
	temp.tm_mon = td->month - 1;
	temp.tm_mday = td->day;
	temp.tm_hour = td->hour;
	temp.tm_min = td->minute;
	temp.tm_sec = td->second;

	tt = mktime(&temp);
	tt += zone;

	localtime_r(&tt, &temp);
	td->year = temp.tm_year + 1900;
	td->month = temp.tm_mon + 1;
	td->day = temp.tm_mday;
	td->hour = temp.tm_hour;
	td->minute = temp.tm_min;
	td->second = temp.tm_sec;
}
static void get_localTime(STimeDay* td)
{

    time_t timep;
    time(&timep);
    struct tm tt = {0};
    struct tm *p = localtime_r(&timep, &tt);

	//localtime_r(&tt, &temp);
	td->year = p->tm_year + 1900;
	td->month = p->tm_mon + 1;
	td->day = p->tm_mday;
	td->hour = p->tm_hour;
	td->minute = p->tm_min;
	td->second = p->tm_sec;
    printf("get_localTime : %d-%d-%d %d:%d:%d\n",
        td->year,td->month,td->day,td->hour,td->minute,td->second);
}


static int get_file_by_time(char* filename, STimeDay* td)
{
	DIR* pDir;
	struct dirent *ent;
	char temp[256] = {0};
	char* p;

	snprintf(temp, sizeof(temp), "%s/%04d%02d%02d", GRD_SD_MOUNT_POINT,
		td->year, td->month, td->day);

	pDir = opendir(temp);
	if (!pDir)
		return -1;

	memset(temp, 0, sizeof(temp));
	snprintf(temp, sizeof(temp), "%04d%02d%02d%02d%02d",
		td->year, td->month, td->day, td->hour, td->minute);

	while((ent = readdir(pDir)) != NULL)
	{
		if (ent->d_type != DT_REG)
			continue;

		if (strlen(ent->d_name) < 29)
			continue;

		p = strstr(ent->d_name, temp);
		if (!p)
			continue;
		if (p - ent->d_name > 5)
			continue;

		sprintf(filename, "%s/%04d%02d%02d/%s", GRD_SD_MOUNT_POINT,
			td->year, td->month, td->day, ent->d_name);
		return 0;
	}

	return -1;
}
int time_to_tick(STimeDay *t1, int zone);

static void tutk_file_list_event_process(int SID, int avIndex, char *buf,unsigned char event)
{
#define EVENT_PACKET_MAX_COUNT 49
	SMsgAVIoctrlListEventReq *p = (SMsgAVIoctrlListEventReq *)buf;
	SMsgAVIoctrlListEventResp* pres = NULL;
	SMsgAVIoctrlListEventResp res;
	AVI_DMS_TIME avi_start;
	AVI_DMS_TIME avi_stop;
	u64t start;
	u64t stop;
	int i,j,cnt = 0,ret,sched_file_num = 0,motion_file_num = 0;
    FILE_NODE node;
	FILE_LIST *sched_list = NULL,*motion_list = NULL;
	stTime *resultEvtbufp = NULL;
	int size, count,abnormalFlg = 0;

	PRINT_INFO("IOTYPE_USER_IPCAM_LISTEVENT_REQ\n\n");
	/*宏天顺发给app的时间不加时区，使用UTC标准时间，其他使用本地时间和osd相同*/
	//PRINT_INFO("+++++\n");
	PRINT_INFO("list type:%d\n",p->event);
	PRINT_INFO("LIST START %d-%d-%d,%d:%d:%d\n",
		p->stStartTime.year,p->stStartTime.month,p->stStartTime.day,
		p->stStartTime.hour,p->stStartTime.minute,p->stStartTime.second);
	PRINT_INFO("LIST END %d-%d-%d,%d:%d:%d\n",
		p->stEndTime.year,p->stEndTime.month,p->stEndTime.day,
		p->stEndTime.hour,p->stEndTime.minute,p->stEndTime.second);

	//PRINT_INFO("+++++\n");
	avi_start.dwYear = p->stStartTime.year;
	avi_start.dwMonth = p->stStartTime.month;
	avi_start.dwDay = p->stStartTime.day;
	avi_start.dwHour = p->stStartTime.hour;
	avi_start.dwMinute = p->stStartTime.minute;
	avi_start.dwSecond = p->stStartTime.second;

	avi_stop.dwYear = p->stEndTime.year;
	avi_stop.dwMonth = p->stEndTime.month;
	avi_stop.dwDay = p->stEndTime.day;
	avi_stop.dwHour = p->stEndTime.hour;
	avi_stop.dwMinute = p->stEndTime.minute;
	avi_stop.dwSecond = p->stEndTime.second;
	if(avi_start.dwYear == 0)
	{
		memcpy(&avi_start,&avi_stop,sizeof(avi_stop));
		avi_start.dwDay -= 1;
		avi_start.dwHour = avi_start.dwSecond= avi_start.dwMinute = 0;
	}

	start = time_to_u64t(&avi_start);
	stop  = time_to_u64t(&avi_stop);
	PRINT_INFO("LIST EVENT start :%llu,stop:%llu\n",start,stop);

	sched_list = search_file_by_time(0, 0xFF, start, stop);
	if(sched_list)
	{
		sched_file_num = get_len_list(sched_list);
		PRINT_INFO("sched_file_num:%d\n",sched_file_num);
	}

	motion_list = search_file_by_time(RECORD_TYPE_MOTION, 0xFF, start, stop);
	if(motion_list)
	{
		motion_file_num = get_len_list(motion_list);
		PRINT_INFO("motion_file_num:%d\n",motion_file_num);
	}

	resultEvtbufp = (stTime *)malloc(sizeof(stTime)*MAX_ALARM_EVENT);
	if(resultEvtbufp)
		memset(resultEvtbufp,0,sizeof(stTime)*MAX_ALARM_EVENT);
	else
	{
		abnormalFlg = 1;
		PRINT_INFO("file_list_event_process 4\n");
		goto LIST_ALL_EVENT_RESP_END;
	}


	for(i=0; i<sched_file_num; i++)
	{
		memset(&node, 0, sizeof(FILE_NODE));
		ret = get_file_node(sched_list, &node);
		if (ret == GK_NET_FILE_SUCCESS) 
		{			
			resultEvtbufp[i].start = node.start;
			resultEvtbufp[i].end = node.stop;
			resultEvtbufp[i].flag = AVIOCTRL_EVENT_ALL;			
		}
	}

	cnt += sched_file_num;
	
	for(i=0; i<motion_file_num; i++)
	{
		memset(&node, 0, sizeof(FILE_NODE));
		ret = get_file_node(motion_list, &node);
		if (ret == GK_NET_FILE_SUCCESS) 
		{
			for(j=0; j<sched_file_num; j++)
			{
				if((resultEvtbufp[j].start == node.start) && 
					(resultEvtbufp[j].end== node.stop))
				{
					resultEvtbufp[j].flag = AVIOCTRL_EVENT_MOTIONDECT;
					break;
				}
			}

			if(j == sched_file_num)
			{
				resultEvtbufp[cnt].start = node.start;
				resultEvtbufp[cnt].end = node.stop;
				resultEvtbufp[cnt].flag = AVIOCTRL_EVENT_MOTIONDECT;
				cnt++;
			}
		}
	}

	i = 0;
	size = cnt;
	while (size > 0)
	{
		count = size > EVENT_PACKET_MAX_COUNT ? EVENT_PACKET_MAX_COUNT : size;
		pres = (SMsgAVIoctrlListEventResp*)malloc(sizeof(SMsgAVIoctrlListEventResp)+sizeof(SAvEvent)*count);
		if (!pres)
		{
			abnormalFlg = 1;
			PRINT_INFO("file_list_event_process 5\n");
			goto LIST_ALL_EVENT_RESP_END;
		}
		
		memset(pres, 0, sizeof(SMsgAVIoctrlListEventResp)+sizeof(SAvEvent)*count);

		pres->total = cnt / EVENT_PACKET_MAX_COUNT + (cnt % EVENT_PACKET_MAX_COUNT ? 1 : 0);
		pres->index = i;
		pres->endflag = pres->total == (i + 1) ? 1 : 0;
		pres->count = count;

		i++;
		size -= EVENT_PACKET_MAX_COUNT;

		for (j = 0; j < count; j++)
		{
			u64t_to_time(&avi_start, resultEvtbufp[(i-1)*EVENT_PACKET_MAX_COUNT + j].start);
			u64t_to_time(&avi_stop, resultEvtbufp[(i-1)*EVENT_PACKET_MAX_COUNT + j].end);

			pres->stEvent[j].stBeginTime.year = avi_start.dwYear;
			pres->stEvent[j].stBeginTime.month = avi_start.dwMonth;
			pres->stEvent[j].stBeginTime.day = avi_start.dwDay;
			pres->stEvent[j].stBeginTime.wday = 0;
			pres->stEvent[j].stBeginTime.hour = avi_start.dwHour;
			pres->stEvent[j].stBeginTime.minute = avi_start.dwMinute;
			pres->stEvent[j].stBeginTime.second = avi_start.dwSecond;
			pres->stEvent[j].stEndTime.year = avi_stop.dwYear;
			pres->stEvent[j].stEndTime.month = avi_stop.dwMonth;
			pres->stEvent[j].stEndTime.day = avi_stop.dwDay;
			pres->stEvent[j].stEndTime.wday = 0;
			pres->stEvent[j].stEndTime.hour = avi_stop.dwHour;
			pres->stEvent[j].stEndTime.minute = avi_stop.dwMinute;
			pres->stEvent[j].stEndTime.second = avi_stop.dwSecond;
			pres->stEvent[j].event = resultEvtbufp[(i-1)*EVENT_PACKET_MAX_COUNT + j].flag;

			//接听状态
			if (node.path[21] == 'a')
			{
				pres->stEvent[j].reserved[0] = 2;
			}
			else
			{
				pres->stEvent[j].reserved[0] = 1;
			}

			pres->stEvent[j].status = 0;

		}
		PRINT_INFO("recode count:%d\n",pres->count);
		avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_LISTEVENT_RESP, (char *)pres,
			sizeof(SMsgAVIoctrlListEventResp)+sizeof(SAvEvent)*pres->count);

		free(pres);
		pres = NULL;
	}

LIST_ALL_EVENT_RESP_END:

	if(resultEvtbufp)
	{
		free(resultEvtbufp);
		resultEvtbufp = NULL;
	}

	if(sched_list)
    	search_close(sched_list);

	if(motion_list)
    	search_close(motion_list);

	if(abnormalFlg)
	{
		memset(&res, 0, sizeof(SMsgAVIoctrlListEventResp));
		res.total = 1;
		res.endflag = 1;
		avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_LISTEVENT_RESP, (char *)&res,sizeof(SMsgAVIoctrlListEventResp));
	}
	
	PRINT_INFO("tutk_file_list_event_process end\n");
}

/*如果使用buf 中的参数进行搜索需要从1970年到2030年，搜索范围太大，考虑到效率问题目前只搜索最近3个月的录制文件*/
static void tutk_list_date_event_process(int SID, int avIndex, char *buf)
{

#define MAX_SEARCH_MONTH_COUNT	3
#define MAX_SEARCH_RECORD_DIR 	MAX_SEARCH_MONTH_COUNT*31

	SMsgAVIoctrlListEventReq *p = (SMsgAVIoctrlListEventReq *)buf;
	SMsgAVIoctrlListEventResp* pres = NULL;
	SMsgAVIoctrlListEventResp  res;
	STimeDay dateRecord[MAX_SEARCH_RECORD_DIR];
    STimeDay td;
	int year,month,i,k,total_cnt = 0,count,index,total,offset;
	int dayMask = 0;
#if 0
	PRINT_INFO("IOTYPE_USER_IPCAM_LISTEVENT_REQ\n\n");
    PRINT_INFO("list type:%d\n",p->event);
    PRINT_INFO("LIST START %d-%d-%d,%d:%d:%d\n",
        p->stStartTime.year,p->stStartTime.month,p->stStartTime.day,
        p->stStartTime.hour,p->stStartTime.minute,p->stStartTime.second);
    PRINT_INFO("LIST END %d-%d-%d,%d:%d:%d\n",
        p->stEndTime.year,p->stEndTime.month,p->stEndTime.day,
        p->stEndTime.hour,p->stEndTime.minute,p->stEndTime.second);
#endif
    get_localTime(&td);

	if(td.year < 2000)
		goto LIST_DATE_EVENT_END;

	year = td.year;

	if(year == 2000)
	{
		PRINT_INFO("IOTYPE_USER_IPCAM_LISTEVENT_DATE_REQ:Year:%d,Month:%d\n",td.year,td.month);
		int start_month = ((td.month - MAX_SEARCH_MONTH_COUNT)>=0)?(td.month - MAX_SEARCH_MONTH_COUNT + 1):1;
		for(month=start_month; month <= td.month; month++)
		{
			dayMask = (long)search_day_by_month(0, 0, td.year-2000, month);
			if(dayMask <= 0)
				continue;

			for(k=1; k<32; k++)
			{
				if(dayMask & (1<<k))
				{
					if(total_cnt >= MAX_SEARCH_RECORD_DIR)
						goto SEND_LIST_DATE_EVENT;

					dateRecord[total_cnt].year = td.year;
					dateRecord[total_cnt].month = month;
					dateRecord[total_cnt].day = k;

					total_cnt++;
				}
			}
		}
	}
	else if(year > 2000 && td.month >= MAX_SEARCH_MONTH_COUNT)
	{

		PRINT_INFO("IOTYPE_USER_IPCAM_LISTEVENT_DATE_REQ:Year:%d,Month:%d\n",td.year,td.month);
		for(month=td.month-MAX_SEARCH_MONTH_COUNT+1; month <= td.month; month++)
		{
			dayMask = (long)search_day_by_month(0, 0, td.year-2000, month);
			if(dayMask <= 0)
				continue;

			for(k=1; k<32; k++)
			{
				if(dayMask & (1<<k))
				{
					if(total_cnt >= MAX_SEARCH_RECORD_DIR)
						goto SEND_LIST_DATE_EVENT;

					dateRecord[total_cnt].year = td.year;
					dateRecord[total_cnt].month = month;
					dateRecord[total_cnt].day = k;

					total_cnt++;
				}
			}
		}
	}
	else if(year > 2000 && td.month < MAX_SEARCH_MONTH_COUNT)
	{
		PRINT_INFO("IOTYPE_USER_IPCAM_LISTEVENT_DATE_REQ:Year:%d,Month:%d\n",td.year,td.month);
		for(month=12-(MAX_SEARCH_MONTH_COUNT-td.month); month <= 12; month++)
		{
			dayMask = (long)search_day_by_month(0, 0, td.year-2000-1, month);
			if(dayMask <= 0)
				continue;

			for(k=1; k<32; k++)
			{
				if(dayMask & (1<<k))
				{
					if(total_cnt >= MAX_SEARCH_RECORD_DIR)
						goto SEND_LIST_DATE_EVENT;

					dateRecord[total_cnt].year = td.year-1;
					dateRecord[total_cnt].month = month;
					dateRecord[total_cnt].day = k;

					total_cnt++;
				}
			}
		}

		for(month=1; month <= td.month; month++)
		{
			dayMask = (long)search_day_by_month(0, 0, td.year-2000, month);
			if(dayMask <= 0)
				continue;

			for(k=1; k<32; k++)
			{
				if(dayMask & (1<<k))
				{
					if(total_cnt >= MAX_SEARCH_RECORD_DIR)
						goto SEND_LIST_DATE_EVENT;

					dateRecord[total_cnt].year = td.year;
					dateRecord[total_cnt].month = month;
					dateRecord[total_cnt].day = k;

					total_cnt++;
				}
			}
		}

	}

SEND_LIST_DATE_EVENT:
	offset = 0;
	index = 0;
	total = total_cnt;

	while(total > 0)
	{
		count = total > 49 ? 49 : total;
		pres = (SMsgAVIoctrlListEventResp*)malloc(sizeof(SMsgAVIoctrlListEventResp)+sizeof(SAvEvent)*count);
		if (!pres)
			break;

		memset(pres,0,(sizeof(SMsgAVIoctrlListEventResp)+sizeof(SAvEvent)*count));

		for(i=0; i<count; i++)
		{
			pres->stEvent[i].stBeginTime.year = dateRecord[offset+i].year;
			pres->stEvent[i].stBeginTime.month = dateRecord[offset+i].month;
			pres->stEvent[i].stBeginTime.day = dateRecord[offset+i].day;
			pres->stEvent[i].stEndTime.year = dateRecord[offset+i].year;
			pres->stEvent[i].stEndTime.month = dateRecord[offset+i].month;
			pres->stEvent[i].stEndTime.day = dateRecord[offset+i].day;
			pres->stEvent[i].status = 0;
			pres->stEvent[i].event = AVIOCTRL_EVENT_ALL;
		}

		offset += count;
		total -= count;

		pres->total = total_cnt;
		pres->count = count;
		pres->index = index++;
		pres->endflag = (total == 0?1:0);
		avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_LISTEVENT_DATE_RESP, (char *)pres,
			(sizeof(SMsgAVIoctrlListEventResp)+sizeof(SAvEvent)*count));

		free(pres);
		pres = NULL;
	}

	return;

LIST_DATE_EVENT_END:
	memset(&res, 0, sizeof(SMsgAVIoctrlListEventResp));
	res.total = 0;
	res.endflag = 1;
	avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_LISTEVENT_RESP, (char *)&res,
		sizeof(SMsgAVIoctrlListEventResp));

}

static int tutk_pb_set_pos(AviPBHandle *pPBHandle, int *pTimePos)
{
    int time_pos = *pTimePos;
    int all_time = cal_pb_all_time(pPBHandle);
	int video_no = (int)(pPBHandle->video_count * time_pos / all_time);
    int no = 0;
	int videoNo  = video_no;
	
    while(video_no && no < pPBHandle->index_count)
    {
        int fcc = pPBHandle->idx_array[4*no];
        if(fcc == MAKE_FOURCC('0','0','d','c'))
            video_no --;

        no++;
    }

	pPBHandle->no = no;
	pPBHandle->video_no = videoNo;

    PRINT_INFO("time_pos = %d, all time = %d,get no = %d, index count = %d, video_no = %d\n", time_pos, all_time, pPBHandle->no, pPBHandle->index_count,pPBHandle->video_no);

    return 0;
}

static AviPBHandle *Gk_Tutk_PlayBackByTime(AVI_DMS_TIME *lpStartTime, AVI_DMS_TIME *lpStopTime)
{
    PRINT_INFO("call Gk_Tutk_PlayBackByTime\n");

    u32t ch_num = 0;
    u64t start = time_to_u64t(lpStartTime);
    u64t stop = time_to_u64t(lpStopTime);

    AviPBHandle *pPBHandle = NULL;
    pPBHandle = (AviPBHandle *)create_pb_handle();
    if (NULL == pPBHandle) {
        PRINT_ERR();
        return NULL;
    }

    pPBHandle->list = search_file_by_time(0, ch_num, start, stop);  //0， 所有的情况
    if (NULL == pPBHandle->list) {
        PRINT_ERR("list is null.\n");
        return NULL;
    }

    print_pb_handle(pPBHandle);
    int ret = 0;
#if 0
    //读下一个文件
    ret = avi_pb_slide_to_next_file(pPBHandle);
    if (ret != 0)
    {
        PRINT_ERR("avi_pb_slide_to_next_file error.");
        return NULL;
    }

    //按起始时间定位 pPBHandle 的no
    int seekPos = cal_pb_seek_time(pPBHandle, lpStartTime);
    if(seekPos < 0)
    {
        PRINT_ERR("seek position err\n");
        return NULL;
    }
    ret = avi_pb_open(pPBHandle->file_path, pPBHandle);
    if (ret < 0) {
        PRINT_ERR("avi_pb_open %s error.\n", pPBHandle->file_path);
        return NULL;
    }
    avi_pb_set_pos(pPBHandle, &seekPos);

    return pPBHandle;
#else
    //按起始时间定位 pPBHandle 的no
    int seekPos = cal_pb_seek_time(pPBHandle, lpStartTime);
    if(seekPos < 0)
    {
        LOG_ERR("seek position err\n");
        goto EXIT;
    }
    ret = avi_pb_open(pPBHandle->file_path, pPBHandle);
    if (ret < 0) {
        LOG_ERR("avi_pb_open %s error.\n", pPBHandle->file_path);
        goto EXIT;
    }
    //avi_pb_set_pos(pPBHandle, &seekPos);

	tutk_pb_set_pos(pPBHandle, &seekPos);

    print_pb_handle(pPBHandle);

    return pPBHandle;

EXIT:

    if(pPBHandle->list)
    {
        search_close(pPBHandle->list);
        pPBHandle->list = NULL;
    }
	
    if(pPBHandle->file)
    {
		fclose(pPBHandle->file);
        pPBHandle->file = NULL;
    }
	
    if(pPBHandle->idx_array)
    {
        free(pPBHandle->idx_array);
        pPBHandle->idx_array = NULL;
    }
	
    if(pPBHandle->pb_buf)
    {
        free(pPBHandle->pb_buf);
        pPBHandle->pb_buf = NULL;
    }
	
    if(pPBHandle)
    {
        free(pPBHandle);
        pPBHandle = NULL;
    }

	return NULL;



#endif
}

int Gk_Tutk_CalStopTime(AVI_DMS_TIME *lpStartTime, AVI_DMS_TIME *lpStopTime)
{
    lpStopTime->dwYear = lpStartTime->dwYear;
    lpStopTime->dwMonth = lpStartTime->dwMonth;
    lpStopTime->dwDay = lpStartTime->dwDay;
    lpStopTime->dwHour = 23;
    lpStopTime->dwMinute = 59;
    lpStopTime->dwSecond = 59;

    return 0;
}

int Gk_Tutk_PlayBackByFile(char *filename)
{
    PRINT_INFO("call Gk_Tutk_PlayBackByFile\n");

    AviPBHandle *pPBHandle = NULL;
    pPBHandle = (AviPBHandle *)create_pb_handle();
    if (NULL == pPBHandle) {
        PRINT_ERR();
        return NULL;
    }

    int ret = avi_pb_open(filename, pPBHandle);
    if (ret < 0) {
        PRINT_ERR("avi_pb_open %s error.\n", filename);
        return NULL;
    }

    return pPBHandle;
}



static void *thread_PlayBack(void *arg)
{
	int ret, size;
    int retVal = 0;
	int SID = (int)arg;
	AV_Client *p = &gClientInfo[SID];
	FRAMEINFO_t videoInfo, audioInfo;
    unsigned int firstTick = 0;   
    unsigned int curTick = 0;
     int firstTime =0; 
     int curTime =0;
     int curPos = 0;
	//AviPBHandle* pbhandle;
    int nResend = 1;
    int count = 0;
    float usage = 0.0;
	SMsgAVIoctrlPlayRecordResp resp;
	int check_cnt = 0;
	
	SET_THREAD_NAME();
    
	//int avIndex = avServStart2(SID, AuthCallBackFn, 0, SERVTYPE_STREAM_SERVER, p->playBackCh);

    
    int avIndex = avServStart3(SID, AuthCallBackFn, AV_SERV_START_TIME_OUT, SERVTYPE_STREAM_SERVER, p->playBackCh,&nResend);
	//int avIndex = avServStart(SID, avID, avPass, 0, SERVTYPE_STREAM_SERVER, p->playBackCh);
	if(avIndex < 0)
	{
		PRINT_INFO("thread_PlayBack avServStart2 failed SID[%d] avIndex[%d]!!!\n", SID, avIndex);
		retVal = -2;
		goto out;
	}
	PRINT_INFO("thread_PlayBack start OK SID:[%d] ,index[%d]\n", SID,avIndex);

    PRINT_INFO("get file time %d-%d-%d %d:%d:%d \n",
        p->playRecord.stTimeDay.year,
        p->playRecord.stTimeDay.month,
        p->playRecord.stTimeDay.day,
        p->playRecord.stTimeDay.hour,
        p->playRecord.stTimeDay.minute,
        p->playRecord.stTimeDay.second);

    #if USE_HTS_PROTOCOL
	time_convert(&p->playRecord.stTimeDay, runSystemCfg.timezoneCfg.timezone * 60);
	#endif

    AVI_DMS_TIME t_start = {0};
    AVI_DMS_TIME t_stop = {0};
    t_start.dwYear = p->playRecord.stTimeDay.year;
    t_start.dwMonth = p->playRecord.stTimeDay.month;
    t_start.dwDay = p->playRecord.stTimeDay.day;
    t_start.dwHour = p->playRecord.stTimeDay.hour;
    t_start.dwMinute = p->playRecord.stTimeDay.minute;
    t_start.dwSecond = p->playRecord.stTimeDay.second;


    if (p->playRecord.Param == E_PB_BY_FILE)
    {
        PRINT_INFO("thread_PlayBack: playback by file\n");
        //根据起始时间得到文件名
        char filename[256] = {0};
        if (get_file_by_time(filename, &p->playRecord.stTimeDay))
        {   
            
            PRINT_ERR("get_file_by_time failed\n");
            retVal = -1;
            goto out;
        }

        //pthread_mutex_lock(&tutk_pb_getframe_x);
        pbhandle[SID] = Gk_Tutk_PlayBackByFile(filename);
		//pbhandle = Gk_Tutk_PlayBackByFile(filename);
        //pthread_mutex_unlock(&tutk_pb_getframe_x);
        if (pbhandle[SID] == NULL)
		//if (pbhandle == NULL)
        {
            PRINT_ERR("Gk_Tutk_PlayBackByFile error.\n");
            retVal = -1;
            goto out;
        }

    }
    else if (p->playRecord.Param == E_PB_BY_TIME)
    {
        PRINT_INFO("thread_PlayBack: playback by time\n");
        /* 按时间段跨文件回放 */
        Gk_Tutk_CalStopTime(&t_start, &t_stop);

        //pthread_mutex_lock(&tutk_pb_getframe_x);
        pbhandle[SID]  = Gk_Tutk_PlayBackByTime(&t_start, &t_stop);
		//pbhandle = Gk_Tutk_PlayBackByTime(&t_start, &t_stop);
        //pthread_mutex_unlock(&tutk_pb_getframe_x);
        if (pbhandle[SID]  == NULL)
		//if (pbhandle == NULL)
        {
            PRINT_ERR("Gk_Tutk_PlayBackByTime error.\n");
            retVal = -1;
            goto out;
        }
    }
    else
    {
        PRINT_ERR("playRecord Param %d error.\n", p->playRecord.Param);
        retVal = -1;
        goto out;
    }
    
	//PRINT_INFO("start time:%llu,stop time:%llu \n", pbhandle->list->start,pbhandle->list->stop);
	//PRINT_INFO("no:%d,video_no:%d,audio_no:%d,fps:%d,video_count:%d,index_count:%d,idx_array_count:%d\n",pbhandle->no, pbhandle->video_no,pbhandle->audio_no,pbhandle->fps,pbhandle->video_count,pbhandle->index_count,pbhandle->idx_array_count);
	//PRINT_INFO("w:%d,h:%d \n", pbhandle->video_width,pbhandle->video_height);
	PRINT_INFO("no:%d,video_no:%d,audio_no:%d,fps:%d,video_count:%d,index_count:%d,idx_array_count:%d\n",pbhandle[SID]->no, pbhandle[SID]->video_no,pbhandle[SID]->audio_no,pbhandle[SID]->fps,pbhandle[SID]->video_count,pbhandle[SID]->index_count,pbhandle[SID]->idx_array_count);
	PRINT_INFO("w:%d,h:%d \n", pbhandle[SID]->video_width,pbhandle[SID]->video_height);


	while(1)
	{
		if(p->bStopPlayBack == 1)
		{
			PRINT_INFO("thread_PlayBack SID %d:%d bStopPlayBack bStopPlayBack\n", SID, avIndex);
			break;
		}

		if(gClientInfo[SID].bPausePlayBack)
		{
			usleep(5000);
			continue;
		}


		pthread_mutex_lock(&gClientInfo[SID].sLock);

		if(gClientInfo[SID].bSeekTimeCmd == 1)
		{
			avi_pb_close(pbhandle[SID]);

			memcpy((void *)&t_start,(void *)&gClientInfo[SID].stSeekTimeDay,sizeof(AVI_DMS_TIME));
			Gk_Tutk_CalStopTime(&t_start, &t_stop);

	        pbhandle[SID] = Gk_Tutk_PlayBackByTime(&t_start, &t_stop);
	        if(pbhandle[SID] == NULL)
	        {
	            LOG_ERR("Gk_Tutk_PlayBackByTime error.\n");
	            retVal = -1;
				pthread_mutex_unlock(&gClientInfo[SID].sLock);

				memset(&resp, 0, sizeof(SMsgAVIoctrlPlayRecordResp));
				resp.command = AVIOCTRL_RECORD_PLAY_END;
				resp.result = -1;
				
				ret = avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP, (char *)&resp, sizeof(SMsgAVIoctrlPlayRecordResp));
	            goto out;
	        }

			memset((void *)&gClientInfo[SID].stSeekTimeDay,0,sizeof(AVI_DMS_TIME));
			gClientInfo[SID].bSeekTimeCmd = 0;

			PRINT_INFO("----------->\n");
		}

		pthread_mutex_unlock(&gClientInfo[SID].sLock);


		if(avResendBufUsageRate(avIndex)>=0.85)
		{
			if(++check_cnt%20 == 0)
				PRINT_INFO("avResendBufUsageRate %f!\n",avResendBufUsageRate(avIndex));
			
			usleep(10*1000);
			continue;
		}
		else
		{
			check_cnt = 0;
		}


		size = 0;
        //pthread_mutex_lock(&tutk_pb_getframe_x);
        //ret = avi_pb_get_frame(pbhandle, NULL, &size);
        ret = avi_pb_get_frame(pbhandle[SID], NULL, &size);
        //pthread_mutex_unlock(&tutk_pb_getframe_x);
        if (ret < 0) {
            PRINT_ERR("call avi_pb_get_frame failed. slide to next file\n");
            //ret = avi_pb_slide_to_next_file(pbhandle);
            ret = avi_pb_slide_to_next_file(pbhandle[SID]);
            if (ret == 0)
            {
                continue; //继续下一个文件
            }
            else
            {
                PRINT_ERR("avi_pb_slide_to_next_file error.");
                ret = GK_NET_NOMOREFILE;
            }
        }
#if 0
		float t1 ;
        //t1 = (float)pbhandle->no / (float)pbhandle->index_count;
        t1 = (float)pbhandle[SID]->no / (float)pbhandle[SID]->index_count;
        //printf("float time 1:%f\n",t1);
        //printf("fps:%d video count:%d\n", pbhandle->fps, pbhandle->video_count);
        //float t2 = t1 * pbhandle->video_count / pbhandle->fps;
        float t2 = t1 * pbhandle[SID]->video_count / pbhandle[SID]->fps;
        //printf("float time 2:%f\n",t2);

        int  t3 = (int )t2;
#else
		int  t3 = pbhandle[SID]->video_no / pbhandle[SID]->fps;
#endif
        STimeDay fisrtTimeDay;      
        //tutk_getTimeDay(pbhandle,&fisrtTimeDay);
        tutk_getTimeDay(pbhandle[SID],&fisrtTimeDay);
        firstTime = time_to_tick(&fisrtTimeDay, runSystemCfg.timezoneCfg.timezone * 60);

        curTime = firstTime + t3;
        //PRINT_INFO("video_no:%d no:%d,frame_type:0x%x,timetick:%u\n", pbhandle->video_no, pbhandle->no, pbhandle->node.frame_type, pbhandle->node.timetick);

        //curTime = pbhandle->node.timetick + runSystemCfg.timezoneCfg.timezone * 60;
        
        //printf("curtime:%d -- %d\n", firstTime +t3, curTime);
    	if (ret == GK_NET_NOMOREFILE)
		{

			PRINT_INFO("[%s]OK! playback file end. avIndex:%d\n", __FUNCTION__, avIndex);
            count =0;
            while(1)
            {
                
				if(p->bStopPlayBack == 1)
				{
					PRINT_INFO("2.thread_PlayBack SID %d:%d bStopPlayBack bStopPlayBack\n", SID, avIndex);
					break;
				}
				
                usage = avResendBufUsageRate(avIndex);
                PRINT_INFO("avResendBufUsageRate :%f,count:%d\n",usage,count);
                if(0.0 == usage)
                {
                    break;
                }
                else
                {
                    count++;
                }
                if(count >=10)
                {    
                    PRINT_INFO("avResendBufUsageRate timeout\n");
                    break;
                }
                sleep(1);
            }
			memset(&resp, 0, sizeof(SMsgAVIoctrlPlayRecordResp));
			resp.command = AVIOCTRL_RECORD_PLAY_END;         
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP, (char *)&resp, sizeof(SMsgAVIoctrlPlayRecordResp));
        	break;
    	}
		
		if(p->bStopPlayBack == 1)
		{
			PRINT_INFO("3.thread_PlayBack SID %d:%d bStopPlayBack bStopPlayBack\n", SID, avIndex);
			break;
		}

		//if (pbhandle->node.frame_type == 0x11)
		if (pbhandle[SID]->node.frame_type == 0x11)
		{
			memset(&videoInfo, 0, sizeof(FRAMEINFO_t));
			videoInfo.codec_id = MEDIA_CODEC_VIDEO_H264;
			videoInfo.flags = IPC_FRAME_FLAG_IFRAME;
            //videoInfo.timestamp = pbhandle->node.timetick;
            videoInfo.timestamp = pbhandle[SID]->node.timetick;
            videoInfo.utctime = curTime;

			//ret = avSendFrameData(avIndex, pbhandle->node.one_frame_buf, size, (void *)&videoInfo, sizeof(FRAMEINFO_t));
			ret = avSendFrameData(avIndex, pbhandle[SID]->node.one_frame_buf, size, (void *)&videoInfo, sizeof(FRAMEINFO_t));
			if(ret < 0 && ret != AV_ER_EXCEED_MAX_SIZE)
			{
				PRINT_ERR("avSendFrameData error0 AV_ER_EXCEED_MAX_SIZE. ret:%d\n", ret);                
                if(ret == AV_ER_CLIENT_NO_AVLOGIN || ret == AV_ER_INVALID_SID 
                    || ret == AV_ER_SESSION_CLOSE_BY_REMOTE || ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT
                    || ret == IOTC_ER_INVALID_SID )
                {            
                    break;
                }
				continue;
			}
            
            if(ret == AV_ER_EXCEED_MAX_SIZE)
            {
                
                PRINT_ERR("avSendFrameData AV_ER_EXCEED_MAX_SIZE\n");
                
                usleep(5*1000);
            }
		}
		//else if (pbhandle->node.frame_type == 0x10)
		else if (pbhandle[SID]->node.frame_type == 0x10)
		{
			memset(&videoInfo, 0, sizeof(FRAMEINFO_t));
			videoInfo.codec_id = MEDIA_CODEC_VIDEO_H264;
			videoInfo.flags = IPC_FRAME_FLAG_PBFRAME;
            //videoInfo.timestamp = pbhandle->node.timetick;
            videoInfo.timestamp = pbhandle[SID]->node.timetick;
            videoInfo.utctime = curTime;

			//ret = avSendFrameData(avIndex, pbhandle->node.one_frame_buf, size, (void *)&videoInfo, sizeof(FRAMEINFO_t));
			ret = avSendFrameData(avIndex, pbhandle[SID]->node.one_frame_buf, size, (void *)&videoInfo, sizeof(FRAMEINFO_t));
            if(ret < 0 && ret != AV_ER_EXCEED_MAX_SIZE)
			{		
                PRINT_ERR("avSendFrameData error1. ret:%d\n", ret);
                if(ret == AV_ER_CLIENT_NO_AVLOGIN || ret == AV_ER_INVALID_SID 
                    || ret == AV_ER_SESSION_CLOSE_BY_REMOTE || ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT
                    || ret == IOTC_ER_INVALID_SID )
                {            
                    break;
                }
				continue;
			}

            if(ret == AV_ER_EXCEED_MAX_SIZE)
            {
                
                PRINT_ERR("avSendFrameData AV_ER_EXCEED_MAX_SIZE\n");
                
                usleep(5*1000);
            }
		}
		else
		{

            #if 1
			memset(&audioInfo, 0, sizeof(FRAMEINFO_t));
            #if USE_HTS_PROTOCOL
			audioInfo.codec_id = MEDIA_CODEC_AUDIO_G711A;//AUDIO_CODEC;MEDIA_CODEC_AUDIO_G711A;
            audioInfo.flags = (AUDIO_SAMPLE_8K << 2) | (AUDIO_DATABITS_16 << 1) | AUDIO_CHANNEL_MONO;
            //audioInfo.timestamp = pbhandle->node.timetick;
            audioInfo.timestamp = pbhandle[SID]->node.timetick;

			//char AudioBuffer[4096] = {0};
			//size = audio_alaw_decode((short *)AudioBuffer, (unsigned char*)MediaBuffer, size);
            
            char AudioBuffer[4096] = {0};
            //int new_len = audio_alaw_encode((unsigned char *)AudioBuffer, (short *)pbhandle->node.one_frame_buf, size);
            int new_len = audio_alaw_encode((unsigned char *)AudioBuffer, (short *)pbhandle[SID]->node.one_frame_buf, size);
                

			//ret = avSendAudioData(avIndex, pbhandle->node.one_frame_buf, size, (void *)&audioInfo, sizeof(FRAMEINFO_t));
			ret = avSendAudioData(avIndex, AudioBuffer, new_len, (void *)&audioInfo, sizeof(FRAMEINFO_t));


            #else
            
			audioInfo.codec_id = MEDIA_CODEC_AUDIO_PCM;//AUDIO_CODEC;MEDIA_CODEC_AUDIO_G711A;
            audioInfo.flags = (AUDIO_SAMPLE_8K << 2) | (AUDIO_DATABITS_16 << 1) | AUDIO_CHANNEL_MONO;
            //audioInfo.timestamp = pbhandle->node.timetick;
            audioInfo.timestamp = pbhandle[SID]->node.timetick;

			//char AudioBuffer[4096] = {0};
			//size = audio_alaw_decode((short *)AudioBuffer, (unsigned char*)MediaBuffer, size);
            
			//ret = avSendAudioData(avIndex, pbhandle->node.one_frame_buf, size, (void *)&audioInfo, sizeof(FRAMEINFO_t));
			ret = avSendAudioData(avIndex, pbhandle[SID]->node.one_frame_buf, size, (void *)&audioInfo, sizeof(FRAMEINFO_t));
            #endif
            if(ret < 0 && ret != AV_ER_EXCEED_MAX_SIZE)
			{
				//audio send fail when size > AV_MAX_AUDIO_DATA_SIZE, so continue not break;
				PRINT_ERR("avSendAudioData error. ret:%d, size:%d\n", ret, size);
                if(ret == AV_ER_CLIENT_NO_AVLOGIN || ret == AV_ER_INVALID_SID 
                    || ret == AV_ER_SESSION_CLOSE_BY_REMOTE || ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT
                    || ret == IOTC_ER_INVALID_SID )
                {            
                    break;
                }

                continue;
			}

            if(ret == AV_ER_EXCEED_MAX_SIZE)
            {
                
                PRINT_ERR("avSendAudioData AV_ER_EXCEED_MAX_SIZE\n");
                usleep(5*1000);
            }


            #else
			memset(&audioInfo, 0, sizeof(FRAMEINFO_t));
			//audioInfo.codec_id = MEDIA_CODEC_AUDIO_PCM;//AUDIO_CODEC;MEDIA_CODEC_AUDIO_G711A;

			audioInfo.codec_id = MEDIA_CODEC_AUDIO_G711A;//AUDIO_CODEC;MEDIA_CODEC_AUDIO_G711A;
            audioInfo.flags = (AUDIO_SAMPLE_8K << 2) | (AUDIO_DATABITS_16 << 1) | AUDIO_CHANNEL_MONO;
            audioInfo.timestamp = pbhandle->node.timetick;

			//char AudioBuffer[4096] = {0};
			//size = audio_alaw_decode((short *)AudioBuffer, (unsigned char*)MediaBuffer, size);
            
            char AudioBuffer[4096] = {0};
            int new_len = audio_alaw_encode((unsigned char *)AudioBuffer, (short *)pbhandle->node.one_frame_buf, size);
                

			//ret = avSendAudioData(avIndex, pbhandle->node.one_frame_buf, size, (void *)&audioInfo, sizeof(FRAMEINFO_t));
			ret = avSendAudioData(avIndex, AudioBuffer, new_len, (void *)&audioInfo, sizeof(FRAMEINFO_t));

            if(ret < 0 && ret != AV_ER_EXCEED_MAX_SIZE)
			{
				//audio send fail when size > AV_MAX_AUDIO_DATA_SIZE, so continue not break;
				PRINT_ERR("avSendAudioData error. ret:%d, size:%d\n", ret, size);
                if(ret == AV_ER_CLIENT_NO_AVLOGIN || ret == AV_ER_INVALID_SID 
                    || ret == AV_ER_SESSION_CLOSE_BY_REMOTE || ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT
                    || ret == IOTC_ER_INVALID_SID )
                {            
                    break;
                }

                continue;
			}
            #endif
		}
        //usleep(8*1000);
	}

    //pthread_mutex_lock(&tutk_pb_getframe_x);
	//avi_pb_close(pbhandle);
	avi_pb_close(pbhandle[SID]);
    //pthread_mutex_unlock(&tutk_pb_getframe_x);
    retVal = 0;

out:
	
	if(avIndex >= 0)
	{
		avServStop(avIndex);
	}
	
	avServExit(SID, p->playBackCh);

	pthread_mutex_lock(&gClientInfo[SID].sLock);
	gClientInfo[SID].bStopPlayBack = 0;
	gClientInfo[SID].playBackCh = -1;
	gClientInfo[SID].bSeekTimeCmd = 0;
	pthread_mutex_unlock(&gClientInfo[SID].sLock);

	PRINT_INFO("thread_PlayBack exit\n");
	return NULL;
}


static int ptz_process(unsigned char chn, unsigned char ptz_cmd, unsigned char speed, unsigned char point)
{
    int ptz_speed = 0;
    int ptz_step  = 0;
    GK_NET_PRESET_INFO   gkPresetCfg;
    GK_NET_CRUISE_GROUP  cruise_info;

    if (chn < 0 || chn > 255)
    {
        PRINT_INFO("FUN[%s]  LINE[%d]  Input chn:%d error!\n", __FUNCTION__, __LINE__, chn);
        return -1;
    }

    if (point < 0 || point > PTZ_MAX_PRESET)
    {
        PRINT_INFO("FUN[%s]  LINE[%d]  Input point:%d error!\n", __FUNCTION__, __LINE__, point);
        return -1;
    }

    ptz_speed = (speed * 5) / 64;
    if (ptz_speed <= 0 || ptz_speed > 5)
    {
        ptz_speed = 3;
    }

    ptz_step = 10;
    switch(ptz_cmd)
    {
        case AVIOCTRL_PTZ_STOP:
            netcam_ptz_stop();
            break;

        case AVIOCTRL_PTZ_UP:
            netcam_ptz_up(ptz_step, ptz_speed);
            CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
            break;

        case AVIOCTRL_PTZ_DOWN:
            netcam_ptz_down(ptz_step, ptz_speed);
            CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
            break;

        case AVIOCTRL_PTZ_LEFT:
            netcam_ptz_left(ptz_step, ptz_speed);
            CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
            break;

        case AVIOCTRL_PTZ_LEFT_UP:
            netcam_ptz_left_up(ptz_step, ptz_speed);
            CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
            break;

        case AVIOCTRL_PTZ_LEFT_DOWN:
            netcam_ptz_left_down(ptz_step, ptz_speed);
            CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
            break;

        case AVIOCTRL_PTZ_RIGHT:
            netcam_ptz_right(ptz_step, ptz_speed);
            CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
            break;

        case AVIOCTRL_PTZ_RIGHT_UP:
            netcam_ptz_right_up(ptz_step, ptz_speed);
            CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
            break;

        case AVIOCTRL_PTZ_RIGHT_DOWN:
            netcam_ptz_right_down(ptz_step, ptz_speed);
            CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_ptz_stop);
            break;

        case AVIOCTRL_PTZ_AUTO:
            netcam_ptz_hor_ver_cruise(speed);
            break;

        case AVIOCTRL_PTZ_SET_POINT:
            if (point > PTZ_MAX_PRESET)
                point = PTZ_MAX_PRESET - 1;
            else if (point > 0)
                point -= 1;
            else
                point = 0;
            get_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            gkPresetCfg.nPresetNum++;
            if (gkPresetCfg.nPresetNum >= NETCAM_PTZ_MAX_PRESET_NUM)
            {
                gkPresetCfg.nPresetNum = NETCAM_PTZ_MAX_PRESET_NUM;
            }
            gkPresetCfg.no[gkPresetCfg.nPresetNum-1] = point;
            set_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            PresetCruiseCfgSave();
            if (netcam_ptz_set_preset(point, NULL))
            {
                PRINT_INFO("FUN[%s]  LINE[%d]  call netcam_ptz_set_preset error!\n", __FUNCTION__, __LINE__);
                return -1;
            }
            break;

        case AVIOCTRL_PTZ_CLEAR_POINT:
            if (point > PTZ_MAX_PRESET)
                point = PTZ_MAX_PRESET - 1;
            else if (point > 0)
                point -= 1;
            else
                point = 0;
            get_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            gkPresetCfg.nPresetNum--;
            if (gkPresetCfg.nPresetNum <= 0)
            {
                gkPresetCfg.nPresetNum = 0;
            }
            set_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            PresetCruiseCfgSave();
            if (netcam_ptz_clr_preset(point))
            {
                PRINT_INFO("FUN[%s]  LINE[%d]  call netcam_ptz_clr_preset error!\n", __FUNCTION__, __LINE__);
                return -1;
            }
            break;

        case AVIOCTRL_PTZ_GOTO_POINT:
            if (point > PTZ_MAX_PRESET)
                point = PTZ_MAX_PRESET - 1;
            else if (point > 0)
                point -= 1;
            else
                point = 0;
            cruise_info.byPointNum    = 1;
            cruise_info.byCruiseIndex = 0;
            cruise_info.struCruisePoint[0].byPointIndex = 0;
            cruise_info.struCruisePoint[0].byPresetNo   = point;
            cruise_info.struCruisePoint[0].byRemainTime = 0;
            cruise_info.struCruisePoint[0].bySpeed      = 3;
            netcam_ptz_stop();

            if (netcam_ptz_preset_cruise(&cruise_info))
            {
                PRINT_INFO("FUN[%s]  LINE[%d]  call netcam_ptz_preset_cruise error!\n", __FUNCTION__, __LINE__);
                return -1;
            }
            break;

        case AVIOCTRL_PTZ_SET_MODE_START:
            break;

        case AVIOCTRL_PTZ_SET_MODE_STOP:
            break;

        case AVIOCTRL_PTZ_MODE_RUN:
            break;

        case AVIOCTRL_PTZ_MENU_OPEN:
            break;

        case AVIOCTRL_PTZ_MENU_EXIT:
            break;

        case AVIOCTRL_PTZ_MENU_ENTER:
            break;

        case AVIOCTRL_PTZ_FLIP:
            break;

        case AVIOCTRL_PTZ_START:
            break;

        case AVIOCTRL_LENS_APERTURE_OPEN:
            break;

        case AVIOCTRL_LENS_APERTURE_CLOSE:
            break;

        case AVIOCTRL_LENS_ZOOM_IN:
            #ifdef MODULE_SUPPORT_AF
            sdk_isp_set_af_zoom(1);
            #endif
            break;

        case AVIOCTRL_LENS_ZOOM_OUT:
            #ifdef MODULE_SUPPORT_AF
            sdk_isp_set_af_zoom(2);
            #endif
            break;

        case AVIOCTRL_LENS_FOCAL_NEAR:
            #ifdef MODULE_SUPPORT_AF
            sdk_isp_set_af_focus(1);
            #endif
            break;

        case AVIOCTRL_LENS_FOCAL_FAR:
            #ifdef MODULE_SUPPORT_AF
            sdk_isp_set_af_focus(2);
            #endif
            break;

        case AVIOCTRL_AUTO_PAN_SPEED:
            break;

        case AVIOCTRL_AUTO_PAN_LIMIT:
            break;

        case AVIOCTRL_AUTO_PAN_START:
            break;

        case AVIOCTRL_PATTERN_START:
            break;

        case AVIOCTRL_PATTERN_STOP:
            break;

        case AVIOCTRL_PATTERN_RUN:
            break;

        case AVIOCTRL_SET_AUX:
            break;

        case AVIOCTRL_CLEAR_AUX:
            break;

        case AVIOCTRL_MOTOR_RESET_POSITION:
            break;
        case AVIOCTRL_PTZ_LEFT_RIGHT_AUTO:
            break;
        case AVIOCTRL_PTZ_UP_DOWN_AUTO:
            break;

        default:
            PRINT_INFO("FUN[%s]  LINE[%d]  Input ptz_cmd:%d error!\n", __FUNCTION__, __LINE__, ptz_cmd);
            return -1;
            break;
    }

    return 0;
}

#define  	V_QUALITY_LEVEL_0       	800 	
#define  	V_QUALITY_LEVEL_1       	500		
#define  	V_QUALITY_LEVEL_2       	300		
#define  	V_QUALITY_LEVEL_3       	100	

/**
800k,15fps
500k,10fps
300k,8fps
150k,6fps
关键帧间隔为4秒

**/
#define V_FPS_LEVEL_0 15
#define V_FPS_LEVEL_1 10
#define V_FPS_LEVEL_2 8
#define V_FPS_LEVEL_3 6


int stream0_bps = V_QUALITY_LEVEL_0;
int stream1_bps = V_QUALITY_LEVEL_1;
int stream_bps_del = 100;
int stream_quality[4] = {0}; 
static int tutk_v_send_sub_stream = 1;

//#if USE_RL_PROTOCOL
#if 0
static int tutk_v_send_sub_stream = 1;
static int v_level = 1;
static int tutk_set_v_quality(int level)
{
    PRINT_INFO("role level:%d\n", level);

    #if 0
    int ret = 0;
    ST_GK_ENC_STREAM_H264_ATTR stH264Config;
    memset(&stH264Config, 0, sizeof(ST_GK_ENC_STREAM_H264_ATTR));

    if (level == 0) { //720P
        ret = netcam_video_get(0, 0, &stH264Config);
        if (ret != 0) {
            PRINT_ERR("CMS: fail to get video parameterss.");
            return -1;
        }
        PRINT_INFO("tutk main bps:%d\n", stH264Config.bps);

        if (stH264Config.bps != V_QUALITY_LEVEL_0) {
            stH264Config.bps = V_QUALITY_LEVEL_0;
            ret = netcam_video_set(0, 0, &stH264Config);
            if (ret != 0) {
                PRINT_ERR("CMS: fail to set video parameterss.");
                return -1;
            }
        }

        tutk_v_send_sub_stream = 0;
    } else {
        ret = netcam_video_get(0, 1, &stH264Config);
        if (ret != 0) {
            PRINT_ERR("CMS: fail to get video parameterss.");
            return -1;
        }
        PRINT_INFO("tutk sub bps:%d\n", stH264Config.bps);

        if (level == 1) {
            if (stH264Config.bps != V_QUALITY_LEVEL_1) {
                stH264Config.bps = V_QUALITY_LEVEL_1;
                ret = netcam_video_set(0, 1, &stH264Config);
                if (ret != 0) {
                    PRINT_ERR("CMS: fail to set video parameterss.");
                    return -1;
                }
            }
        } else if (level == 2) {
            if (stH264Config.bps != V_QUALITY_LEVEL_2) {
                stH264Config.bps = V_QUALITY_LEVEL_2;
                ret = netcam_video_set(0, 1, &stH264Config);
                if (ret != 0) {
                    PRINT_ERR("CMS: fail to set video parameterss.");
                    return -1;
                }
            }

        } else {
            PRINT_ERR("level %d error.\n", level);
            return -1;
        }

        tutk_v_send_sub_stream = 1;
    }

    PRINT_INFO("stream0-bps:%d", runVideoCfg.vencStream[0].h264Conf.bps);
    PRINT_INFO("stream1-bps:%d", runVideoCfg.vencStream[1].h264Conf.bps);

    #else
    if (level == 0) {
        stream0_bps = V_QUALITY_LEVEL_0;
        netcam_adjust_bps(0, stream0_bps);
        tutk_v_send_sub_stream = 0;
    } else if (level == 1) {
        stream1_bps = V_QUALITY_LEVEL_1;
        netcam_adjust_bps(1, stream1_bps);
        tutk_v_send_sub_stream = 1;
    } else if (level == 2) {
        stream1_bps = V_QUALITY_LEVEL_2;
        netcam_adjust_bps(1, stream1_bps);
        tutk_v_send_sub_stream = 1;
    } else {
        PRINT_ERR("level %d error.\n", level);
        return -1;
    }
    v_level = level;
    #endif

    return 0;
}

static int tutk_get_v_quality(int *level)
{
    #if 0
    int ret = 0;
    ST_GK_ENC_STREAM_H264_ATTR stH264Config;
    memset(&stH264Config, 0, sizeof(ST_GK_ENC_STREAM_H264_ATTR));

    PRINT_INFO("get tutk_v_send_sub_stream:%d \n", tutk_v_send_sub_stream);
    
    if (tutk_v_send_sub_stream == 0) {
        *level = 0;

        /* 如果是主码流，强制变化码率为 V_QUALITY_LEVEL_0 */
        ret = netcam_video_get(0, 0, &stH264Config);
        if (ret != 0) {
            PRINT_ERR("CMS: fail to get video parameterss.");
            return -1;
        }
        PRINT_INFO("tutk get -- main bps:%d\n", stH264Config.bps);

        if (stH264Config.bps != V_QUALITY_LEVEL_0) {
            stH264Config.bps = V_QUALITY_LEVEL_0;
            ret = netcam_video_set(0, 0, &stH264Config);
            if (ret != 0) {
                PRINT_ERR("CMS: fail to set video parameterss.");
                return -1;
            }
        }
    } else {
        ret = netcam_video_get(0, 1, &stH264Config);
        if (ret != 0) {
            PRINT_ERR("CMS: fail to get video parameterss.");
            return -1;
        }
        PRINT_INFO("tutk get -- sub bps:%d", stH264Config.bps);

        if (stH264Config.bps > V_QUALITY_LEVEL_2)
            *level = 1;
        else if (stH264Config.bps > 0)
            *level = 2;
        else
            PRINT_ERR("bps error:%d.\n", stH264Config.bps);
    }
    #else
    PRINT_INFO("tutk get level:%d", v_level);
    *level = v_level;
    #endif

    return 0;
}

#endif
//#else
static int tutk_set_v_quality2(int stream_id, int quality)
{
    PRINT_INFO("stream_id:%d quality:%d\n", stream_id, quality);
    int ret = 0;
    ST_GK_ENC_STREAM_H264_ATTR stH264Config;
    memset(&stH264Config, 0, sizeof(ST_GK_ENC_STREAM_H264_ATTR));
    if(1 == stream_id)
    {
        tutk_v_send_sub_stream = 1; 
    }
    else if(0 == stream_id)
    {
        tutk_v_send_sub_stream = 0;
    }
    else
    {
        PRINT_ERR("stream no [%d] err !\n",stream_id);
    }
    
    ret = netcam_video_get(0, stream_id, &stH264Config);
    if (ret != 0) {
        PRINT_ERR("CMS: fail to get video parameterss.");
        return -1;
    }
    PRINT_INFO("tutk stram%d old bps:%d,fps:%d,gop:%d\n", stream_id, stH264Config.bps,stH264Config.fps,stH264Config.gop);

    if (quality == 0) {
        if (stH264Config.bps != V_QUALITY_LEVEL_0) {
            stH264Config.bps = V_QUALITY_LEVEL_0;
            stH264Config.fps = V_FPS_LEVEL_0;
            stH264Config.gop = 2*V_FPS_LEVEL_0;
            ret = netcam_video_set(0, stream_id, &stH264Config);
            if (ret != 0) {
                PRINT_ERR("CMS: fail to set video parameterss.");
                return -1;
            }
        }
    } else if (quality == 1) {
        if (stH264Config.bps != V_QUALITY_LEVEL_1) {
            stH264Config.bps = V_QUALITY_LEVEL_1;            
            stH264Config.fps = V_FPS_LEVEL_1;
            stH264Config.gop = 2*V_FPS_LEVEL_1;
            ret = netcam_video_set(0, stream_id, &stH264Config);
            if (ret != 0) {
                PRINT_ERR("CMS: fail to set video parameterss.");
                return -1;
            }
        }
    } else if (quality == 2) {
        if (stH264Config.bps != V_QUALITY_LEVEL_2) {
            stH264Config.bps = V_QUALITY_LEVEL_2;            
            stH264Config.fps = V_FPS_LEVEL_2;
            stH264Config.gop = 2*V_FPS_LEVEL_2;
            ret = netcam_video_set(0, stream_id, &stH264Config);
            if (ret != 0) {
                PRINT_ERR("CMS: fail to set video parameterss.");
                return -1;
            }
        }
    } else {
        PRINT_ERR("stream%d quality %d error.\n", stream_id, quality);
        return -1;
    }

    PRINT_INFO("stream0  bps:%d,fps:%d,gop:%d", runVideoCfg.vencStream[0].h264Conf.bps,runVideoCfg.vencStream[0].h264Conf.fps,runVideoCfg.vencStream[0].h264Conf.gop);
    PRINT_INFO("stream1  bps:%d,fps:%d,gop:%d", runVideoCfg.vencStream[1].h264Conf.bps,runVideoCfg.vencStream[1].h264Conf.fps,runVideoCfg.vencStream[1].h264Conf.gop);

    return 0;

}

/*
stream_id总是0，根据quality区分主码流和子码流。且quality = 1 ，3 ，5

*/
int quality_hts = 3;
static int tutk_set_v_quality_hts(int stream_id, int quality)
{
    int usd_stread_id = 0;
    int ret = 0;
    int old_quality = 0;
    ST_GK_ENC_STREAM_H264_ATTR stH264Config;
    memset(&stH264Config, 0, sizeof(ST_GK_ENC_STREAM_H264_ATTR));
    #if 0
    if(1 == quality)
    {
        tutk_v_send_sub_stream = 0; 
        usd_stread_id = 0;
        
    }
    else if(3 == quality || 5 == quality)
    {
        tutk_v_send_sub_stream = 1;
        usd_stread_id = 1;
    }
    else
    {
        PRINT_ERR("quality[%d] err !\n",quality);
    }
    #endif
    tutk_v_send_sub_stream = 1;//渡江计划只发第二路。
    usd_stread_id = 1;
    PRINT_INFO("stream_id:%d quality:%d\n", usd_stread_id, quality);
    
    ret = netcam_video_get(0, usd_stread_id, &stH264Config);
    if (ret != 0) {
        PRINT_ERR("CMS: fail to get video parameterss.");
        return -1;
    }
    PRINT_INFO("tutk stram%d old bps:%d,fps:%d,gop:%d\n", usd_stread_id, stH264Config.bps,stH264Config.fps,stH264Config.gop);

    if (quality == 1) {//700kbps
        if (stH264Config.bps != V_QUALITY_LEVEL_0) {
            stH264Config.bps = V_QUALITY_LEVEL_0;
            stH264Config.fps = V_FPS_LEVEL_1;
            stH264Config.gop = 4*V_FPS_LEVEL_1;
            ret = netcam_video_set(0, usd_stread_id, &stH264Config);
            if (ret != 0) {
                PRINT_ERR("CMS: fail to set video parameterss.");
                return -1;
            }
        }
    } else if (quality == 3) {//500kbps
        if (stH264Config.bps != V_QUALITY_LEVEL_2) {
            stH264Config.bps = V_QUALITY_LEVEL_2;            
            stH264Config.fps = V_FPS_LEVEL_1;
            stH264Config.gop = 4*V_FPS_LEVEL_1;
            ret = netcam_video_set(0, usd_stread_id, &stH264Config);
            if (ret != 0) {
                PRINT_ERR("CMS: fail to set video parameterss.");
                return -1;
            }
        }
    } else if (quality == 5) {//500kbps
        if (stH264Config.bps != V_QUALITY_LEVEL_3) {
            stH264Config.bps = V_QUALITY_LEVEL_3;            
            stH264Config.fps = V_FPS_LEVEL_1;
            stH264Config.gop = 4*V_FPS_LEVEL_1;
            ret = netcam_video_set(0, usd_stread_id, &stH264Config);
            if (ret != 0) {
                PRINT_ERR("CMS: fail to set video parameterss.");
                return -1;
            }
        }
    } else {
        PRINT_ERR("stream%d quality %d error.\n", usd_stread_id, quality);
        return -1;
    }
    old_quality = quality_hts;
    quality_hts = quality;
    PRINT_INFO("old quality:%d,set quality :%d\n",old_quality,quality_hts);
    PRINT_INFO("stream0  bps:%d,fps:%d,gop:%d", runVideoCfg.vencStream[0].h264Conf.bps,runVideoCfg.vencStream[0].h264Conf.fps,runVideoCfg.vencStream[0].h264Conf.gop);
    PRINT_INFO("stream1  bps:%d,fps:%d,gop:%d", runVideoCfg.vencStream[1].h264Conf.bps,runVideoCfg.vencStream[1].h264Conf.fps,runVideoCfg.vencStream[1].h264Conf.gop);

    return 0;

}

static int tutk_get_v_quality2(int stream_id, int *quality)
{
    int ret = 0;
    ST_GK_ENC_STREAM_H264_ATTR stH264Config;
    memset(&stH264Config, 0, sizeof(ST_GK_ENC_STREAM_H264_ATTR));

    ret = netcam_video_get(0, stream_id, &stH264Config);
    if (ret != 0) {
        PRINT_ERR("CMS: fail to get video parameterss.");
        return -1;
    }
    PRINT_INFO("tutk get -- stream%d bps:%d,fps:%d,gop:%d\n", stream_id, stH264Config.bps,stH264Config.fps,stH264Config.gop);

    if (stH264Config.bps > V_QUALITY_LEVEL_1)
        *quality = 0;
    else if (stH264Config.bps > V_QUALITY_LEVEL_2)
        *quality = 1;
    else if (stH264Config.bps > 0)
        *quality = 2;
    else
        PRINT_ERR("bps error:%d.\n", stH264Config.bps);

    return 0;
}
static int tutk_get_v_quality_hts(int stream_id, int *quality)
{

    *quality = quality_hts;
    //PRINT_ERR("get quality :%d\n",*quality);
    return 0;
}

//#endif

#ifdef MODULE_SUPPORT_RF433
SMsgAVIoctrlCfg433Resp rf433_learn_resp;
static void * TutkAlarmPushThread(void* arg);

void tutk_rf433_learn_handle(int flag)
{
	PRINT_INFO("rf433_learn_handle:flag=%d\n",flag);
	int avIndex,ii;
	rf433_learn_resp.result = flag;
	avIndex = rf433_learn_resp.none;
	if(flag == AVIOCTRL_CFG_433_OK)
	{
		for (ii = 0; ii < MAX_CLIENT_NUMBER; ii++)
		{
			if (gClientInfo[ii].avIndex < 0)	// 设备不在线
			continue;
			avSendIOCtrl(gClientInfo[ii].avIndex, IOTYPE_USER_IPCAM_CFG_433_RESP, (char *)&rf433_learn_resp, sizeof(SMsgAVIoctrlCfg433Resp));
		}
	}
	else
	{
		avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_CFG_433_RESP, (char *)&rf433_learn_resp, sizeof(SMsgAVIoctrlCfg433Resp));
	}
}

void tutk_rf433_event_handle(int areaid,int type,int code)
{
	PRINT_INFO("areaid=%d,type=%d,code=0x%x\n",areaid,type,code);
	PRINT_INFO("rf433 event.\n");
	int itype;
	itype = areaid + RF433_DEV_ALARM;
	if(RF433_DEV_TYPE_CONTROL == rf433_get_dev_type(areaid))
	{
		if(code & RF433_KEY_SOS)
		{
			goto ENTER_ALARM_THREAD;
		}
		else if(code & RF433_KEY_DEFENCE)
		{
			runMdCfg.enable = 1;
			return;
		}
		else if(code & RF433_KEY_UNDEFENCE)
		{
			runMdCfg.enable = 0;
			return;
		}
		else
		{
			return;
		}
	}
	else if(RF433_DEV_TYPE_ALARM == rf433_get_dev_type(areaid))
	{
		
	}
	else
	{
		return;
	}

	if(!runMdCfg.enable)
	{
		return;
	}
	#if 1
	static time_t LastAlarmTime;
	int sh = runMdCfg.scheduleTime[0][0].startHour;
	int sm = runMdCfg.scheduleTime[0][0].startMin;
	int ch = runMdCfg.scheduleTime[0][0].stopHour;
	int cm = runMdCfg.scheduleTime[0][0].stopMin;
	
		
	if(time(NULL) - LastAlarmTime < 10){
		PRINT_INFO("PUSH FREQUENCY NOT ENOUGH.\n");
		return;
	}else{
		LastAlarmTime = time(NULL);
	}

	struct tm *p;
	
	time_t timep;
	time(&timep); 
	p = localtime(&timep); // 取得当地时间

	int scheme_now = p->tm_hour * 60 + p->tm_min;
	
	int scheme_start = sh * 60 + sm;
	int scheme_close = ch * 60 + cm;

	if(scheme_start < scheme_close){
		if(scheme_now < scheme_start || scheme_now > scheme_close){
			PRINT_INFO("INVLAID ALARM TIME:[S:%d][C:%d][N:%d]\n",
				scheme_start,scheme_close,scheme_now
				);
			return;
		}
	}else{
		if(scheme_now < scheme_start && scheme_now > scheme_close){
			PRINT_INFO("INVLAID ALARM TIME:[S:%d][C:%d][N:%d]\n",
				scheme_start,scheme_close,scheme_now
				);
			return;
		}
	}
#endif
ENTER_ALARM_THREAD:
	CreateDetachThread(TutkAlarmPushThread, (void *)itype, NULL);
}

#endif

static int tutk_get_ver(char *s_ver, unsigned long long *n_ver)
{
    char s_tmp[32] = {0};
    int i = last_index_at(s_ver, '.');
    strcpy(s_tmp, s_ver + i + 1);
    printf("s_tmp:%s\n", s_tmp);
    *n_ver = (unsigned long long)atoll(s_tmp);
}


/********************
执行shell命令，并将执行结果返回retErr，执行输出内容返回buff
********************/
static int tutk_getStringBySystem(const char *cmd, char *buff, int bufsize,int *retErr)
{
    FILE *fp = NULL;
    int ret=0;
    memset(buff, 0, sizeof(bufsize));
    if(NULL == buff)
    {
        printf("cmd buf is null\n");
        return -1;
    }
    
    fp = popen(cmd, "r");
    if(fp != NULL)
    {
        *retErr = 0;
        
    }
    else
    {
        *retErr = errno;
    }
  
    if (NULL == fp)
    {
        perror("error popen");
        return -1;
    }
    fread(buff,1,bufsize,fp);
    
    //printf("%d cmd exe display :: %s \n", __LINE__, buff);
    pclose(fp);
}
int tutk_check(unsigned char *inEncrypt,unsigned int fileSize,unsigned char *outDecrypt)
{

    MD5_CTX md5;  
    MD5Init(&md5);           
    int i;  
    //unsigned char encrypt[] ="admin";//21232f297a57a5a743894a0e4a801fc3  
    //unsigned char decrypt[16];      
    MD5Update(&md5,inEncrypt,fileSize);  
    MD5Final(&md5,outDecrypt);   
       
    //printf("\n befor md5:%s\n after md5 32 bit:\n",inEncrypt);  
    printf("\n after md5 32bit out Decypty :\n");  
#if 0
    for(i=0;i<16;i++)  
    {  
        printf("%02x",outDecrypt[i]);  //02x前需要加上 %  
    }  
    
    printf("\n"); 
#endif
    return 0;

}

int tutk_getTimeDay(AviPBHandle* pPBHandle,STimeDay *td)
{
    char file_name[128] = {0};

    int i = last_index_at(pPBHandle->file_path, '/');
    strcpy(file_name, pPBHandle->file_path + i + 1);
    //PRINT_INFO("in cal_pb_all_time, file name: %s\n", file_name);
    u64t time_start_64 = 0;
    time_start_64 = avi_get_start_time(file_name);

    AVI_DMS_TIME t1 = {0};
    u64t_to_time(&t1, time_start_64);
    td->year = t1.dwYear;
    td->month= t1.dwMonth;
    td->day = t1.dwDay;
    td->hour = t1.dwHour;
    td->minute = t1.dwMinute;
    td->second = t1.dwSecond;

    //PRINT_INFO("---- GET timeday %d-%d-%d  %d:%d:%d\n",
    //    td->year,td->month,td->day,td->hour,td->minute,td->second);
    return 0;
}

/**********************************************************************

**********************************************************************/
int time_to_tick(STimeDay *td, int zone)
{

	struct tm temp;
	time_t tt;
    //printf("%s,%d\n",__func__,__LINE__);

	temp.tm_year = td->year - 1900;
	temp.tm_mon = td->month - 1;
	temp.tm_mday = td->day;
	temp.tm_hour = td->hour;
	temp.tm_min = td->minute;
	temp.tm_sec = td->second;
    //printf("%s,%d\n",__func__,__LINE__);
	tt = mktime(&temp);
	tt += zone;
    //printf("%s,%d\n",__func__,__LINE__);


    //PRINT_INFO("time_to_tick :%u\n",timep1);
    return tt;
    
}


static void Handle_IOCTRL_Cmd(int SID, int avIndex, char *buf, int type,int cmd_len)
{
	PRINT_INFO("*************** Handle CMD,type:0x%x ", type);

	if(tutk_update_get_status() && (type != IOTYPE_USER_IPCAM_HEARTBEAT_REQ) &&
		(type != IOTYPE_USER_IPCAM_UPDATE_PROG_REQ))
		return;

	if(netcam_get_update_status() < 0)
	{
		PRINT_INFO("updating, discard tutk cmd!\n");
		return;
	}
	
	switch(type)
	{
		case IOTYPE_USER_IPCAM_START:
		{
			SMsgAVIoctrlAVStream *p = (SMsgAVIoctrlAVStream *)buf;
			PRINT_INFO("IOTYPE_USER_IPCAM_START, ch:%d, avIndex:%d sid:%d\n\n", p->channel, avIndex, SID);
#if 0

            #if USE_RL_PROTOCOL
            regedit_client_to_video(SID, avIndex);
            #else
            regedit_client_to_video2(SID, avIndex, p->streamNo,p->sendAudio);
            #endif
#else

			#if DASA_ENABLED
			avDASAReset(avIndex, AV_DASA_LEVEL_QUALITY_HIGH);
			#endif
			tutk_venc_change_stream_attr(p->channel, p->streamNo, AV_DASA_LEVEL_QUALITY_HIGH);//Set stream ven
			gClientInfo[SID].bSensorNo = 0;//p->channel;

			pthread_mutex_lock(&gClientInfo[SID].sLock);
			regedit_client_to_video(SID, avIndex);
			#ifdef USE_APP_XUHUI
			if(p->sendAudio == 1)
			{
				PRINT_INFO("audio enable! \n");
				regedit_client_to_audio(SID, avIndex);
			}
			else if(p->sendAudio == 0)
			{
				PRINT_INFO("audio disable! \n");
				unregedit_client_from_audio(SID);
			}

			#endif
			gClientInfo[SID].bStreamNo = p->streamNo;

			pthread_mutex_unlock(&gClientInfo[SID].sLock);
#endif
			PRINT_INFO("regedit_client_to_video OK\n");
		}
		break;

		case IOTYPE_USER_IPCAM_STOP:
		{
			SMsgAVIoctrlAVStream *p = (SMsgAVIoctrlAVStream *)buf;
			PRINT_INFO("IOTYPE_USER_IPCAM_STOP, ch:%d, avIndex:%d sid:%d\n\n", p->channel, avIndex, SID);
#if 0			
			unregedit_client_from_video(SID);
#else
			int ret;
			pthread_mutex_lock(&gClientInfo[SID].sLock);
			unregedit_client_from_video(SID);
			unregedit_client_from_audio(SID);
			gClientInfo[SID].bEnableSpeaker = 0;
			pthread_mutex_unlock(&gClientInfo[SID].sLock);

#endif
			PRINT_INFO("unregedit_client_from_video OK\n");
		}
		break;

		case IOTYPE_USER_IPCAM_AUDIOSTART:
		{
			SMsgAVIoctrlAVStream *p = (SMsgAVIoctrlAVStream *)buf;
			PRINT_INFO("IOTYPE_USER_IPCAM_AUDIOSTART, ch:%d, avIndex:%d sid:%d\n\n", p->channel, avIndex, SID);
			regedit_client_to_audio(SID, avIndex);
			PRINT_INFO("regedit_client_to_audio OK\n");
		}
		break;

		case IOTYPE_USER_IPCAM_AUDIOSTOP:
		{
			SMsgAVIoctrlAVStream *p = (SMsgAVIoctrlAVStream *)buf;
			PRINT_INFO("IOTYPE_USER_IPCAM_AUDIOSTOP, ch:%d, avIndex:%d sid:%d\n\n", p->channel, avIndex, SID);
			unregedit_client_from_audio(SID);
			PRINT_INFO("unregedit_client_from_audio OK\n");
		}
		break;

		case IOTYPE_USER_IPCAM_SPEAKERSTART:
		{
			SMsgAVIoctrlAVStream *p = (SMsgAVIoctrlAVStream *)buf;
			PRINT_INFO("IOTYPE_USER_IPCAM_SPEAKERSTART, SID:%d, avIndex:%d, channel:%d\n\n", SID, avIndex, p->channel);
#if 1
			if(speaker_start() == 0)
			{
				gClientInfo[SID].bEnableSpeaker = 1;
				gClientInfo[SID].speakerCh = IOTC_Session_Get_Free_Channel(SID);

				pthread_t Thread_ID;
				if(pthread_create(&Thread_ID, NULL, &thread_ReceiveAudio, (void *)SID))
				{
					PRINT_INFO("IOTYPE_USER_IPCAM_SPEAKERSTART pthread_create fail\n");
					speaker_stop();
				}
			}
			else
			{
				P2P_ERR("Audio speaker has handled by another seeesion,Discard this request");
			}
#else

			if(speaker_start() == 0)
			{
				pthread_mutex_lock(&gClientInfo[SID].sLock);
				gClientInfo[SID].speakerCh = IOTC_Session_Get_Free_Channel(SID);
				gClientInfo[SID].bEnableSpeaker = 1;
				pthread_mutex_unlock(&gClientInfo[SID].sLock);
			}
			else
			{
				P2P_ERR("Audio speaker has handled by another seeesion,Discard this request");
			}
#endif
		}
		break;

		case IOTYPE_USER_IPCAM_SPEAKERSTOP:
		{
			PRINT_INFO("IOTYPE_USER_IPCAM_SPEAKERSTOP\n\n");
			gClientInfo[SID].bEnableSpeaker = 0;
		}
		break;

        case IOTYPE_USER_IPCAM_SETSTREAMCTRL_REQ:
        {
            PRINT_INFO("cmd: IOTYPE_USER_IPCAM_SETSTREAMCTRL_REQ");
		    SMsgAVIoctrlSetStreamCtrlReq* p = (SMsgAVIoctrlSetStreamCtrlReq *)buf;
#if 0
            SMsgAVIoctrlSetStreamCtrlResp res;

            PRINT_INFO("SET STREAM CTRL:[%d,%d,%d].\n",p->channel,p->streamNo, p->quality);
			memset(&res, 0, sizeof(SMsgAVIoctrlSetStreamCtrlResp));
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETSTREAMCTRL_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetStreamCtrlResq));

            #if USE_HTS_PROTOCOL
            tutk_set_v_quality_hts(p->streamNo, p->quality);
            #else
            tutk_set_v_quality2(p->channel, p->quality);
            #endif
#else
			SMsgAVIoctrlSetStreamCtrlResp resp;
			int curVideoQuality = AV_DASA_LEVEL_QUALITY_NORMAL;
			int ret = 0;

			PRINT_INFO("cmd: IOTYPE_USER_IPCAM_SETSTREAMCTRL_REQ\n");
			PRINT_INFO("cmd: channel:%d, streamNo:%d, quality:%d\n",p->channel,p->streamNo,p->quality);

			if(0 == p->quality)
				curVideoQuality = AV_DASA_LEVEL_QUALITY_HIGH;
			else if(1 == p->quality)
				curVideoQuality = AV_DASA_LEVEL_QUALITY_NORMAL;
			else if(2 == p->quality)
				curVideoQuality = AV_DASA_LEVEL_QUALITY_LOW;
			else
				curVideoQuality = AV_DASA_LEVEL_QUALITY_NORMAL;

			tutk_venc_change_stream_attr(p->channel,p->streamNo, curVideoQuality);//Set stream ven

			memset(&resp, 0, sizeof(SMsgAVIoctrlSetStreamCtrlResp));
			memcpy(&resp, p, sizeof(SMsgAVIoctrlSetStreamCtrlResp));
			gClientInfo[SID].bSensorNo = 0;//p->channel;
			gClientInfo[SID].bStreamNo = p->streamNo;
			gClientInfo[SID].bQuality = p->quality;

			ret = avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETSTREAMCTRL_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetStreamCtrlResp));
			if(ret != 0)
			{
				P2P_ERR("IOTYPE_USER_IPCAM_SETSTREAMCTRL_RESP error:%d ",ret);
			}
#endif
        }
		break;

        case IOTYPE_USER_IPCAM_GETSTREAMCTRL_REQ:
        {
            PRINT_INFO("cmd: IOTYPE_USER_IPCAM_GETSTREAMCTRL_REQ");
            //#if USE_RL_PROTOCOL
            #if 0
			//SMsgAVIoctrlGetStreamCtrlReq* p = (SMsgAVIoctrlGetStreamCtrlReq*)buf;
			SMsgAVIoctrlGetStreamCtrlResq res;

            int level = 0;
            tutk_get_v_quality(&level);
            PRINT_INFO("v get level:%d\n", level);
            
			res.channel = level;
			res.quality = AVIOCTRL_QUALITY_MAX;
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETSTREAMCTRL_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetStreamCtrlResq));
            #endif
            //#else
		    SMsgAVIoctrlGetStreamCtrlReq* p = (SMsgAVIoctrlGetStreamCtrlReq *)buf;
#if 0
            SMsgAVIoctrlGetStreamCtrlResq res;
            PRINT_INFO("GET STREAM CTRL:[streamid:%d].\n", p->streamNo);
            int v_quality = 0;
            
#if USE_HTS_PROTOCOL
            tutk_get_v_quality_hts(p->streamNo, &v_quality);
#else
            tutk_get_v_quality2(p->streamNo, &v_quality);
#endif
            PRINT_INFO("v stream%d get quality:%d\n", p->streamNo, v_quality);
            
			res.streamNo = p->streamNo;
			res.quality = (unsigned char)v_quality;
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETSTREAMCTRL_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetStreamCtrlResq));
#else
			SMsgAVIoctrlGetStreamCtrlResq resp;
			memset(&resp, 0, sizeof(SMsgAVIoctrlGetStreamCtrlResq));

			resp.channel = gClientInfo[SID].bSensorNo;
			resp.streamNo = gClientInfo[SID].bStreamNo;
			resp.quality =	gClientInfo[SID].bQuality;
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETSTREAMCTRL_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetStreamCtrlResq));
#endif
        }
		break;

        case IOTYPE_USER_IPCAM_GET_SYSTEM_REQ:
        {
			SMsgAVIoctrlGetSystemReq* req = (SMsgAVIoctrlGetSystemReq*)buf;

            /* 回复信息 */
			SMsgAVIoctrlGetSystemResp resp;
            memset(&resp, 0, sizeof(SMsgAVIoctrlGetSystemResp));
			resp.power_ctrl = req->power_ctrl;
            if (runSystemCfg.deviceInfo.languageType == 0)
                resp.lang = AVIOCTRL_LANG_ZH;
            else if(runSystemCfg.deviceInfo.languageType == 1)
                resp.lang = AVIOCTRL_LANG_EN;            
            else if(runSystemCfg.deviceInfo.languageType == 2)
                resp.lang = AVIOCTRL_LANG_JA;
            
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_SYSTEM_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetSystemResp));
        }
        break;
        
        case IOTYPE_USER_IPCAM_SET_SYSTEM_REQ:
        {
			SMsgAVIoctrlSetSystemReq* req = (SMsgAVIoctrlSetSystemReq*)buf;

            /* 回复信息 */
			SMsgAVIoctrlSetSystemResp resp;
            memset(&resp, 0, sizeof(SMsgAVIoctrlSetSystemResp));
			resp.power_ctrl = req->power_ctrl;
            resp.lang = req->lang;
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_SYSTEM_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSetSystemResp));

            /* 设置语言 */
            if (req->lang == AVIOCTRL_LANG_ZH)
                runSystemCfg.deviceInfo.languageType = 0;
            else if(req->lang == AVIOCTRL_LANG_EN)
                runSystemCfg.deviceInfo.languageType = 1;
            else if(req->lang == AVIOCTRL_LANG_JA)
                runSystemCfg.deviceInfo.languageType = 2;
            else
                printf("set language %d not support!\n",req->lang);
            SystemCfgSave();
            
            PRINT_INFO("IOTYPE_USER_IPCAM_SET_SYSTEM_REQ, power_ctrl:%d\n", req->power_ctrl);
            if (req->power_ctrl == 0)
            {
                #if 0
                netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT
                #else
                netcam_timer_add_task2(netcam_sys_operation,1,SDK_FALSE,SDK_FALSE,0,(void *)SYSTEM_OPERATION_REBOOT);
                #endif
            }
            else if (req->power_ctrl == 3)
            {
                #if 0
                CfgLoadDefValueAll();
                CfgSaveAll();
                sync();
                usleep(20);
                netcam_sys_operation(0,SYSTEM_OPERATION_REBOOT);
                #else
                netcam_timer_add_task2(netcam_sys_operation,1,SDK_FALSE,SDK_FALSE,0,(void *)SYSTEM_OPERATION_HARD_DEFAULT);
                #endif
            }
            else
            {
                PRINT_INFO("IOTYPE_USER_IPCAM_SET_SYSTEM_REQ, power_ctrl err:%d\n", req->power_ctrl);
            }
        }
		break;


        case IOTYPE_USER_IPCAM_SETMOTIONDETECT_REQ:
        {
            PRINT_INFO("cmd: IOTYPE_USER_IPCAM_SETMOTIONDETECT_REQ\n");

            #if USE_HTS_PROTOCOL
            SMsgAVIoctrlSetMotionDetectReq* p = (SMsgAVIoctrlSetMotionDetectReq*)buf;
            SMsgAVIoctrlGetMotionDetectResp res;
            int sensi = 0;
            memcpy(&res,p,sizeof(SMsgAVIoctrlGetMotionDetectResp));
            PRINT_INFO("SET SETMOTIONDETECT channel:%d,sensitivity:%d\n",p->channel,p->sensitivity);

            if(p->sensitivity == 0)
            {
                runMdCfg.enable = 0;
            }
            else
            {
                runMdCfg.enable = 1;
            }
            switch(p->sensitivity)
            {
                case 0:
                    sensi = 0;
                    break;
                case 25:
                    sensi = 1;
                    break;  
                case 50:
                    sensi = 2;
                    break;   
                case 75:
                    sensi = 3;
                    break;   
                case 100:
                    sensi = 4;
                    break; 
                default:
                    PRINT_INFO("sensitivity :%d not support!\n",p->sensitivity);
                    break;
            }
            netcam_md_set_sensitivity(0,sensi);
            
            //runMdCfg.handle.is_rec = 1 ;
            //runMdCfg.handle.recStreamNo = 1;   
            //runMdCfg.handle.recTime = 10;
            /*
            int i = 0, j = 0;
            for (i = 0; i < 7; i++) 
            {
                for (j = 0; j < 4; j++) 
                {
                    runMdCfg.scheduleTime[i][j].startHour = 0;
                    runMdCfg.scheduleTime[i][j].startMin  = 0;
                    runMdCfg.scheduleTime[i][j].stopHour  = 23;
                    runMdCfg.scheduleTime[i][j].stopMin   = 59;
                }
            }
            */
            #if 0
            runMdCfg.handle.is_alarmout = p->trigAudioOut ;
            runMdCfg.handle.is_beep =p->trigAudioOut ;
            
            runMdCfg.handle.is_rec = p->trigRecord ;
            runMdCfg.handle.recStreamNo = 1;   
            runMdCfg.handle.recTime = 10;
            
            int i = 0, j = 0;
            for (i = 0; i < 7; i++) 
            {
                for (j = 0; j < 4; j++) 
                {
                    runMdCfg.scheduleTime[i][j].startHour = p->startHour;
                    runMdCfg.scheduleTime[i][j].startMin  = p->startMins;
                    runMdCfg.scheduleTime[i][j].stopHour  = p->closeHour;
                    runMdCfg.scheduleTime[i][j].stopMin   = p->closeMins;
                }
            }
            #endif
                
            MdCfgSave();
            PRINT_INFO("SET SETMOTIONDETECT channel:%d,enable:%d,sensitivity:%d\n",p->channel,runMdCfg.enable,p->sensitivity);
            avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETMOTIONDETECT_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetMotionDetectResp));
            
            #else
            SMsgAVIoctrlSetMDPReq* p = (SMsgAVIoctrlSetMDPReq*)buf;

            PRINT_INFO("SET SETMOTIONDETECT enable:%d S:[%d:%d] E:[%d:%d].\n",
                p->MotionEnable,
                p->MotionStartHour,
			    p->MotionStartMins,
			    p->MotionCloseHour,
			    p->MotionCloseMins);


			runMdCfg.enable = p->MotionEnable;
            int i = 0, j = 0;
            for (i = 0; i < 7; i++) {
                for (j = 0; j < 4; j++) {
                    runMdCfg.scheduleTime[i][j].startHour = p->MotionStartHour;
                    runMdCfg.scheduleTime[i][j].startMin  = p->MotionStartMins;
                    runMdCfg.scheduleTime[i][j].stopHour  = p->MotionCloseHour;
                    runMdCfg.scheduleTime[i][j].stopMin   = p->MotionCloseMins;
                }
            }
            
            MdCfgSave();

            SMsgAVIoctrlSetMDPResp res;
			memset(&res, 0, sizeof(SMsgAVIoctrlSetMDPResp));
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETMOTIONDETECT_RESP, (char *)&res, sizeof(SMsgAVIoctrlSetMDPResp));
            #endif
        }
		break;

        case IOTYPE_USER_IPCAM_GETMOTIONDETECT_REQ:
        {
            PRINT_INFO("cmd: IOTYPE_USER_IPCAM_GETMOTIONDETECT_REQ\n");
            #if USE_HTS_PROTOCOL//HTS
            SMsgAVIoctrlGetMotionDetectResp res={0};
            
            int sensi;
            int retSensi;
            netcam_md_get_sensitivity(0, &sensi);
            switch(sensi)
            {
                case 0:
                    retSensi = 0;
                    break;
                case 1:
                    retSensi = 25;
                    break;  
                case 2:
                    retSensi = 50;
                    break;   
                case 3:
                    retSensi = 75;
                    break;   
                case 4:
                    retSensi = 100;
                    break; 
                default:
                    PRINT_INFO("get sensitivity :%d not support!\n",sensi);
                    break;
            }
            res.sensitivity = retSensi;
            #if 0
            res.frequency = runMdCfg.handle.intervalTime;
            res.trigAudioOut =  runMdCfg.handle.is_alarmout;
            res.trigRecord = runMdCfg.handle.is_rec;
           

            res.startHour = runMdCfg.scheduleTime[0][0].startHour;
            res.startMins = runMdCfg.scheduleTime[0][0].startMin;
            res.closeHour = runMdCfg.scheduleTime[0][0].stopHour;
            res.closeMins = runMdCfg.scheduleTime[0][0].stopMin;
            
            #endif
            PRINT_INFO("get md sevsitivity:%d,enable:%d\n",res.sensitivity,runMdCfg.enable);
            if(!runMdCfg.enable)
            {
                res.sensitivity = 0;
            }

            PRINT_INFO("return md sevsitivity:%d,enable:%d\n",res.sensitivity,runMdCfg.enable);
            avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETMOTIONDETECT_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetMotionDetectResp));
            
            #else//XUHUI
            SMsgAVIoctrlGetMDPResp res;

			res.MotionEnable = runMdCfg.enable;
			res.MotionStartHour = runMdCfg.scheduleTime[0][0].startHour;
            res.MotionStartMins = runMdCfg.scheduleTime[0][0].startMin;
            res.MotionCloseHour = runMdCfg.scheduleTime[0][0].stopHour;
            res.MotionCloseMins = runMdCfg.scheduleTime[0][0].stopMin;

            PRINT_INFO("GET GETMOTIONDETECT enable:%d [%d:%d] -- [%d:%d].\n", res.MotionEnable, res.MotionStartHour, 
                   res.MotionStartMins, res.MotionCloseHour, res.MotionCloseMins);

			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETMOTIONDETECT_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetMDPResp));
            #endif
        }
		break;

        case IOTYPE_USER_IPCAM_GETSUPPORTSTREAM_REQ:
        {
			SMsgAVIoctrlGetSupportStreamResp res;
            res.number = 1;
			res.streams[0].channel = 1;
			res.streams[0].index = 1;
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETSUPPORTSTREAM_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetSupportStreamResp));
        }
		break;

        case IOTYPE_USER_IPCAM_DEVINFO_REQ:
        {
			SMsgAVIoctrlDeviceInfoResp res;
			res.channel = 0;
			res.free = grd_sd_get_free_size();
			strncpy((char*)res.model, runSystemCfg.deviceInfo.deviceName, sizeof(res.model));
			res.total = grd_sd_get_all_size();
			strncpy((char*)res.vendor, runSystemCfg.deviceInfo.manufacturer, sizeof(res.vendor));
			//res.version = 1;
			
            unsigned long long now_n_ver = 0;
            tutk_get_ver(runSystemCfg.deviceInfo.upgradeVersion, &now_n_ver);
            unsigned long long tmp1,tmp2,tmp3,tmp4;
            //unsigned long long now_n_ver = 20170901121210;
            unsigned int version,version1,version2,version3,version4,subver;
            //20170901121210->GET  17090112
            subver = (now_n_ver%1000000000000)/10000;
            printf("subver:%d\n",subver);
            tmp1 = subver/1000000;
            printf("tmp1:%d\n",tmp1);
            subver = subver%1000000;
            tmp2 = subver/10000;

            printf("tmp2:%d\n",tmp2);
            subver = subver%10000;
            tmp3 = subver/100;

            printf("tmp3:%d\n",tmp3);
            subver = subver%100;
            tmp4 = subver;
            printf("tmp4:%d\n",tmp4);
            version1 = (char)tmp1 << 24;
            version2 = (char)tmp2 << 16;
            version3 = (char)tmp3 << 8;
            version4 = (char)tmp4 << 0;
            printf("version:0x%x,0x%x,0x%x,0x%x\n",version1,version2,version3,version4);
            version = version1 + version2 + version3 + version4;
            printf("version:0x%x\n",version);
            res.version = version;
            avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_DEVINFO_RESP, (char *)&res, sizeof(SMsgAVIoctrlDeviceInfoResp));
        }
		break;

        case IOTYPE_USER_IPCAM_SETPASSWORD_REQ:
		{
            SMsgAVIoctrlSetPasswdReq * p = (SMsgAVIoctrlSetPasswdReq *)buf;
			SMsgAVIoctrlSetPasswdResp res = {0};
/*
			if(strcmp(runUserCfg.user[0].password,p->oldpasswd) != 0){
				res.result = AVIOCTRL_ERR_PASSWORD;
			}.
*/

            #if USE_HTS_PROTOCOL//HTS

            
			PRINT_INFO("User:[%s] want to change password:[%s] to [%s].\n",runUserCfg.user[0].userName,p->oldpasswd,p->newpasswd);
			if (strncmp(runUserCfg.user[0].password, p->oldpasswd, strlen(runUserCfg.user[0].password)) != 0) {
				res.result = AVIOCTRL_ERR_PASSWORD;
			} else {
				strcpy(runUserCfg.user[0].password, p->newpasswd);
                UserCfgSave();
				//CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,1,0), (WORK_CALLBACK)UserCfgSave, NULL);
				res.result = 0;
			}

            
            #else//XUHUI  

            
			PRINT_INFO("User:[%s] want to change password:[%s] to [%s].\n",p->user,p->oldpasswd,p->newpasswd);
			if (strncmp(runUserCfg.user[0].userName, p->user, strlen(runUserCfg.user[0].userName)) != 0) {
				res.result = AVIOCTRL_ERR_USERNAME;
			} else {
				strcpy(runUserCfg.user[0].password, p->newpasswd);
                UserCfgSave();
				//CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,1,0), (WORK_CALLBACK)UserCfgSave, NULL);
				res.result = 0;
			}
            #endif
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETPASSWORD_RESP, (char *)&res, sizeof(res));
		}
		break;

	    case IOTYPE_USER_IPCAM_LISTWIFIAP_REQ:
        {
			char *WifiName = NULL;
			WIFI_SCAN_LIST_t list[20];
			int i, number = 20;
			SMsgAVIoctrlListWifiApResp res;

			memset(list, 0, sizeof(list));
			memset(&res, 0, sizeof(SMsgAVIoctrlListWifiApResp));
			res.number = number;
            printf("IOTYPE_USER_IPCAM_LISTWIFIAP_REQ\n");
			WifiName = netcam_net_wifi_get_devname();
			if(WifiName == NULL)
				goto LISTWIFIAP;
            #if 0
			if(netcam_net_wifi_isOn() != 1)
				goto LISTWIFIAP;

			netcam_net_wifi_on();
            #endif
            #if 0
            #if 1
			if(netcam_net_wifi_get_scan_list(list,&number) != 0)
				goto LISTWIFIAP;
            #else
			if(netcam_net_wifi_get_scan_list_ext(list,&number) != 0)
                goto LISTWIFIAP;
            #endif
            #endif

            int wifi_mode = _net_wifi_get_mode();
            PRINT_INFO("get wifi mode :%d\n",wifi_mode);
            if(wifi_mode == 0)
            {
    			if(netcam_net_wifi_get_scan_list_ap(list,&number) != 0)
    				goto LISTWIFIAP;
            }
            else
            {
                
                if(netcam_net_wifi_get_scan_list_ext(list,&number) != 0)
                    goto LISTWIFIAP;
            }
			for(i = 0; i < number; i++)
			{
				WIFI_ENC_MODE security = list[i].security;
				if (security == WIFI_ENC_NONE)
					res.stWifiAp[i].enctype = AVIOTC_WIFIAPENC_NONE;
				else if (security == WIFI_ENC_WEP_64_ASSII)
					res.stWifiAp[i].enctype = AVIOTC_WIFIAPENC_WPA_TKIP;
				else if (security == WIFI_ENC_WEP_64_HEX)
					res.stWifiAp[i].enctype = AVIOTC_WIFIAPENC_WPA_AES;
				else if (security == WIFI_ENC_WEP_128_ASSII)
					res.stWifiAp[i].enctype = AVIOTC_WIFIAPENC_WPA2_TKIP;
				else if (security == WIFI_ENC_WEP_128_HEX)
					res.stWifiAp[i].enctype = AVIOTC_WIFIAPENC_WPA2_AES;
				else if (security == WIFI_ENC_WPAPSK_TKIP)
					res.stWifiAp[i].enctype = AVIOTC_WIFIAPENC_WPA_PSK_TKIP;
				else if (security == WIFI_ENC_WPAPSK_AES)
					res.stWifiAp[i].enctype = AVIOTC_WIFIAPENC_WPA_PSK_AES;
				else if (security == WIFI_ENC_WPA2PSK_TKIP)
					res.stWifiAp[i].enctype = AVIOTC_WIFIAPENC_WPA2_PSK_TKIP;
				else if (security == WIFI_ENC_WPA2PSK_AES)
					res.stWifiAp[i].enctype = AVIOTC_WIFIAPENC_WPA2_PSK_AES;

				res.stWifiAp[i].mode = AVIOTC_WIFIAPMODE_ADHOC;
				res.stWifiAp[i].signal = list[i].quality;
				strncpy(res.stWifiAp[i].ssid, list[i].essid, sizeof(res.stWifiAp[i].ssid));
				res.stWifiAp[i].status = 0;
			}

           
            if(1 == _net_wifi_get_mode() && sdk_net_get_detect("eth0"))//STA模式且不是有线状态,才做
            {
                for(i=0;i<number;i++)
                {
                    if(0 == strncmp(res.stWifiAp[i].ssid,runNetworkCfg.wifilink[0].essid,strlen(res.stWifiAp[i].ssid)))
                    {
                        
                        res.stWifiAp[i].status = 4;
                        PRINT_INFO("now connected wifi is :%s\n",res.stWifiAp[i].ssid);
                        break;
                    }

                }
            }
LISTWIFIAP:
            avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_LISTWIFIAP_RESP, (char *)&res, sizeof(SMsgAVIoctrlListWifiApResp));
        }
		break;

        case IOTYPE_USER_IPCAM_SETWIFI_REQ:
        {
			SMsgAVIoctrlSetWifiReq* p = (SMsgAVIoctrlSetWifiReq*)buf;
			SMsgAVIoctrlSetWifiResp res;
			char *WifiName = NULL;
            int ret = 0;

			//设置ssid等参数
			WIFI_LINK_INFO_t linkInfo;
			ST_SDK_NETWORK_ATTR net_attr;

			WifiName = netcam_net_wifi_get_devname();
			if(WifiName == NULL)
				goto SETWIFI;

			//#if USE_HTS_PROTOCOL
			#if 1
			//netcam_net_wifi_on();
			#else
			netcam_net_wifi_on();
			#endif
			strncpy(net_attr.name, WifiName, sizeof(net_attr.name));
			netcam_net_get(&net_attr);

			memset(&linkInfo,0,sizeof(WIFI_LINK_INFO_t));
			linkInfo.isConnect = 1;
			strncpy(linkInfo.linkEssid,(char*)p->ssid,sizeof(linkInfo.linkEssid));
			strncpy(linkInfo.linkPsd,(char*)p->password,sizeof(linkInfo.linkPsd));
			if (p->enctype == AVIOTC_WIFIAPENC_WPA_TKIP)
				linkInfo.linkScurity = WIFI_ENC_WEP_64_ASSII;
			else if (p->enctype == AVIOTC_WIFIAPENC_WPA_AES)
				linkInfo.linkScurity = WIFI_ENC_WEP_64_HEX;
			else if (p->enctype == AVIOTC_WIFIAPENC_WPA2_TKIP)
				linkInfo.linkScurity = WIFI_ENC_WEP_128_ASSII;
			else if (p->enctype == AVIOTC_WIFIAPENC_WPA2_AES)
				linkInfo.linkScurity = WIFI_ENC_WEP_128_HEX;
			else if (p->enctype == AVIOTC_WIFIAPENC_WPA_PSK_TKIP)
				linkInfo.linkScurity = WIFI_ENC_WPAPSK_TKIP;
			else if (p->enctype == AVIOTC_WIFIAPENC_WPA_PSK_AES)
				linkInfo.linkScurity = WIFI_ENC_WPAPSK_AES;
			else if (p->enctype == AVIOTC_WIFIAPENC_WPA2_PSK_TKIP)
				linkInfo.linkScurity = WIFI_ENC_WPA2PSK_TKIP;
			else if (p->enctype == AVIOTC_WIFIAPENC_WPA2_PSK_AES)
				linkInfo.linkScurity = WIFI_ENC_WPA2PSK_AES;
			else
				linkInfo.linkScurity = WIFI_ENC_NONE;
			//#if USE_HTS_PROTOCOL
			#if 1
			ret = netcam_net_setwifi(NETCAM_WIFI_STA,linkInfo);

            if (ret <0)
            {
                sleep(5);
                
                netcam_audio_hint(SYSTEM_AUDIO_HINT_LINK_FAIL);
                printf("failed to set sta wifi parament ,back to AP mode!\n");
                netcam_net_setwifi(NETCAM_WIFI_AP,linkInfo);
            }
            else
            {
			    netcam_net_get(&net_attr);
                netcam_audio_hint(SYSTEM_AUDIO_HINT_USE_WIRELESS);
		        netcam_audio_ip(net_attr.ip);
            }
			#else
			if(netcam_net_wifi_set_connect_info(&linkInfo) == WIFI_CONNECT_OK && linkInfo.linkStatus == WIFI_CONNECT_OK)
			{
				net_attr.dhcp = 1;
				netcam_net_set(&net_attr);
			}
			#endif

			CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,1,0), (WORK_CALLBACK)NetworkCfgSave);

SETWIFI:
            if(ret < 0)  
                res.result = 1;
            else
				res.result = 0;
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETWIFI_RESP, (char *)&res, sizeof(SMsgAVIoctrlSetWifiResp));
        }
		break;

        case IOTYPE_USER_IPCAM_SETRECORD_REQ:
        {
			SMsgAVIoctrlSetRecordReq* p = (SMsgAVIoctrlSetRecordReq*)buf;
			
            PRINT_INFO("SET RECORDCFG type:%u [%d:%d] -- [%d:%d].\n", p->recordType, p->startHour, 
                   p->startMins, p->closeHour, p->closeMins);

            switch (p->recordType) {
                case AVIOTC_RECORDTYPE_OFF:
                    runRecordCfg.recordMode = 3;
                    runMdCfg.handle.is_rec = 0 ;
                    MdCfgSave();
                    break;
                case AVIOTC_RECORDTYPE_FULLTIME:
                    #if USE_HTS_PROTOCOL

                    runRecordCfg.recordMode = 1;
                    runMdCfg.handle.is_rec = 0 ;
                    MdCfgSave();
                    
                    #else
                    
                    runRecordCfg.recordMode = 1;
                    runMdCfg.handle.is_rec = 0 ;
                    MdCfgSave();
                    #endif
                    break;
                #if USE_HTS_PROTOCOL//HTS START
                
                case AVIOTC_RECORDTYPE_ALARM://宏天顺需要在此增加报警录像，暂时使用recordMode =2
                    runRecordCfg.recordMode = 2;                    
                    runMdCfg.handle.is_rec = 1 ;
                    runMdCfg.handle.recStreamNo = 1;   
                    runMdCfg.handle.recTime = 1;
                    MdCfgSave();
                    break;
                    
                #endif//HTS END
                case AVIOTC_RECORDTYPE_SCHEDULE:
                    runRecordCfg.recordMode = 0;
                    break;
                default:
                    PRINT_INFO("not support, recordType:%d", p->recordType);
                    break;
            }

            if (p->recordType == AVIOTC_RECORDTYPE_OFF)
                runRecordCfg.enable = 0;
            else
                runRecordCfg.enable = 1;


            #if USE_HTS_PROTOCOL

            if(p->recordType == AVIOTC_RECORDTYPE_FULLTIME)
            {
                int i = 0, j = 0;
                for (i = 0; i < 7; i++) 
                {
                    for (j = 0; j < 4; j++) 
                    {
                        runMdCfg.scheduleTime[i][j].startHour = 0;
                        runMdCfg.scheduleTime[i][j].startMin  = 0;
                        runMdCfg.scheduleTime[i][j].stopHour  = 23;
                        runMdCfg.scheduleTime[i][j].stopMin   = 59;
                    }
                }

                runMdCfg.handle.is_rec = 0;//全天录像的时候不需要移动正常录像
                MdCfgSave();
            }
            #else
            
			int i = 0,j = 0;
            for (i = 0; i < 7; i ++)
			{
				if((p->startHour > p->closeHour) || ((p->startHour == p->closeHour) && p->startMins > p->closeMins))
				{
					runRecordCfg.scheduleTime[i][0].startHour = 0;
					runRecordCfg.scheduleTime[i][0].startMin = 0;
					runRecordCfg.scheduleTime[i][0].stopHour = p->closeHour;
					runRecordCfg.scheduleTime[i][0].stopMin = p->closeMins;

					runRecordCfg.scheduleTime[i][1].startHour = p->startHour;
					runRecordCfg.scheduleTime[i][1].startMin = p->startMins;
					runRecordCfg.scheduleTime[i][1].stopHour = 23;
					runRecordCfg.scheduleTime[i][1].stopMin = 59;

					runRecordCfg.scheduleTime[i][2].startHour = 0;
					runRecordCfg.scheduleTime[i][2].startMin = 0;
					runRecordCfg.scheduleTime[i][2].stopHour = 0;
					runRecordCfg.scheduleTime[i][2].stopMin = 0;

					runRecordCfg.scheduleTime[i][3].startHour = 0;
					runRecordCfg.scheduleTime[i][3].startMin = 0;
					runRecordCfg.scheduleTime[i][3].stopHour = 0;
					runRecordCfg.scheduleTime[i][3].stopMin = 0;

				}
				else
				{
					for (j = 0; j < 4; j ++)
					{
						runRecordCfg.scheduleTime[i][j].startHour = p->startHour;
						runRecordCfg.scheduleTime[i][j].startMin = p->startMins;
						runRecordCfg.scheduleTime[i][j].stopHour = p->closeHour;
						runRecordCfg.scheduleTime[i][j].stopMin = p->closeMins;
					}
				}

            }

            runRecordCfg.recordLen = p->reserved[0];
            if (p->reserved[0] == 0)
            {
                runRecordCfg.recordLen = 5;
            }
			
            #endif
            //scheduleTime -> scheduleSlice
            schedule_time_to_slice(&(runRecordCfg.scheduleTime[0][0]), &(runRecordCfg.scheduleSlice[0][0]));
            MdCfgSave();
            RecordCfgSave();
            avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETRECORD_RESP, (char *)&p, sizeof(SMsgAVIoctrlSetRecordResp));
        }
		break;

        case IOTYPE_USER_IPCAM_GETRECORD_REQ:
        {
            SMsgAVIoctrlGetRecordResp res;

            switch (runRecordCfg.recordMode) {
                case 0:
                    
                    //#if USE_HTS_PROTOCOL
                    //res.recordType = AVIOTC_RECORDTYPE_FULLTIME;
                    //#else
                    res.recordType = AVIOTC_RECORDTYPE_SCHEDULE; 
                    //#endif
                    break;
                case 1:
                    res.recordType = AVIOTC_RECORDTYPE_FULLTIME;
                    break;

                #if USE_HTS_PROTOCOL
                case 2:
                    res.recordType = AVIOTC_RECORDTYPE_ALARM;
                    break;
                #endif
                case 3:
                    res.recordType = AVIOTC_RECORDTYPE_OFF;
                    break;
                default:
                    PRINT_INFO("not support, recordType:%d", runRecordCfg.recordMode);
                    break;
            }
#if 0
            res.startHour = runRecordCfg.scheduleTime[0][0].startHour;
            res.startMins = runRecordCfg.scheduleTime[0][0].startMin;
            res.closeHour = runRecordCfg.scheduleTime[0][0].stopHour;
            res.closeMins = runRecordCfg.scheduleTime[0][0].stopMin;
#else
			if(runRecordCfg.scheduleTime[0][0].startHour == runRecordCfg.scheduleTime[0][1].startHour &&
				runRecordCfg.scheduleTime[0][0].startMin == runRecordCfg.scheduleTime[0][1].startMin)
			{
				res.startHour = runRecordCfg.scheduleTime[0][0].startHour;
				res.startMins = runRecordCfg.scheduleTime[0][0].startMin;
				res.closeHour = runRecordCfg.scheduleTime[0][0].stopHour;
				res.closeMins = runRecordCfg.scheduleTime[0][0].stopMin;
			}
			else
			{
				res.startHour = runRecordCfg.scheduleTime[0][1].startHour;
				res.startMins = runRecordCfg.scheduleTime[0][1].startMin;
				res.closeHour = runRecordCfg.scheduleTime[0][0].stopHour;
				res.closeMins = runRecordCfg.scheduleTime[0][0].stopMin;
			}
#endif
			res.reserved[0] = runRecordCfg.recordLen;

            PRINT_INFO("GET RECORDCFG type:%u [%d:%d] -- [%d:%d].\n", res.recordType, res.startHour, res.startMins, res.closeHour, res.closeMins);

			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETRECORD_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetRecordResp));
        }
		break;


        case IOTYPE_USER_IPCAM_SEACH_RECORD_IN_MONTH_REQ:
        {

			SMsgAVIoctrlRecordInMonthReq *p = (SMsgAVIoctrlRecordInMonthReq *)buf;
			SMsgAVIoctrlRecordInMonthResp res = {0};

            memcpy(&res,p,sizeof(SMsgAVIoctrlRecordInMonthReq));
            
            res.lCalendarMap = (long)search_day_by_month(0, 0, p->year, p->month);
            
            PRINT_INFO("GET RECORD in MONTH:20%d/%d map:%x \n", res.year, res.month, res.lCalendarMap);

			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SEACH_RECORD_IN_MONTH_RESP, (char *)&res, sizeof(SMsgAVIoctrlRecordInMonthResp));
            
        }
        break;

		case IOTYPE_USER_IPCAM_LISTEVENT_REQ:
		{
			SMsgAVIoctrlListEventReq *p = (SMsgAVIoctrlListEventReq *)buf;
#if 0
			SMsgAVIoctrlListEventResp* pres = NULL;
			SMsgAVIoctrlListEventResp res;
			AVI_DMS_TIME avi_start;
			AVI_DMS_TIME avi_stop;
			u64t start;
    		u64t stop;
			FILE_LIST *list;
            FILE_LIST *list_md;
			int i, j, lst_len, size, count;
            
			PRINT_INFO("IOTYPE_USER_IPCAM_LISTEVENT_REQ\n\n");
            /*宏天顺发给app的时间不加时区，使用UTC标准时间，其他使用本地时间和osd相同*/
            //PRINT_INFO("+++++\n");
            PRINT_INFO("list type:%d\n",p->event);
            #if USE_HTS_PROTOCOL
			time_convert(&p->stStartTime, runSystemCfg.timezoneCfg.timezone * 60);
			time_convert(&p->stEndTime, runSystemCfg.timezoneCfg.timezone * 60);
            #endif
            //PRINT_INFO("+++++\n");
			avi_start.dwYear = p->stStartTime.year;
			avi_start.dwMonth = p->stStartTime.month;
			avi_start.dwDay = p->stStartTime.day;
			avi_start.dwHour = p->stStartTime.hour;
			avi_start.dwMinute = p->stStartTime.minute;
			avi_start.dwSecond = p->stStartTime.second;

			avi_stop.dwYear = p->stEndTime.year;
			avi_stop.dwMonth = p->stEndTime.month;
			avi_stop.dwDay = p->stEndTime.day;
			avi_stop.dwHour = p->stEndTime.hour;
			avi_stop.dwMinute = p->stEndTime.minute;
			avi_stop.dwSecond = p->stEndTime.second;

			start = time_to_u64t(&avi_start);
    		stop  = time_to_u64t(&avi_stop);
            PRINT_INFO("LIST EVENT start :%llu,stop:%llu\n",start,stop);
           
            #if USE_HTS_PROTOCOL
            if(AVIOCTRL_EVENT_MOTIONDECT == p->event)
            {
            
                PRINT_INFO("list type : AVIOCTRL_EVENT_MOTIONDECT\n");
                list = search_file_by_time(RECORD_TYPE_MOTION, 0xFF, start, stop);
                if (!list)
                    goto LISTEVENT_REQ;
                PRINT_INFO("file number:%d,type:%d\n",list->ch_num,list->type);
           
                
            }
            else if(AVIOCTRL_EVENT_ALL == p->event)
            {
                PRINT_INFO("list type : AVIOCTRL_EVENT_ALL\n");
                list = search_file_by_time(0, 0xFF, start, stop);
                if (!list)
                    goto LISTEVENT_REQ;

                
                int all_len = get_len_list(list);
                PRINT_INFO("all file number:%d,type:%d,all_len:%d\n",list->ch_num,list->type,all_len);

                list_md = search_file_by_time(RECORD_TYPE_MOTION, 0xFF, start, stop);
                if (!list_md)
                {
                    PRINT_INFO("no RECORD_TYPE_MOTION\n");
                }
                else
                {
                
                    int md_len = get_len_list(list_md);
                    PRINT_INFO("md file number:%d,type:%d,md_len:%d\n",list_md->ch_num,list_md->type,md_len);
                }


                
            }
            else
            {            
                PRINT_INFO("list type : %d,not support!\n",p->event);
                goto LISTEVENT_REQ;
            }
            

        
            #else
            
            list = search_file_by_time(0, 0xFF, start, stop);
			if (!list)
				goto LISTEVENT_REQ;
            
            #endif
            
			lst_len = get_len_list(list);
			if (lst_len == 0)
				goto LISTEVENT_REQ;

#define EVENT_PACKET_MAX_COUNT 49
			i = 0;
			size = lst_len;
			while (size > 0)
			{
				count = size > EVENT_PACKET_MAX_COUNT ? EVENT_PACKET_MAX_COUNT : size;
				pres = (SMsgAVIoctrlListEventResp*)malloc(sizeof(SMsgAVIoctrlListEventResp)+sizeof(SAvEvent)*count);
				if (!pres)
					goto LISTEVENT_REQ;
				memset(pres, 0, sizeof(SMsgAVIoctrlListEventResp));

				pres->total = lst_len / EVENT_PACKET_MAX_COUNT + (lst_len % EVENT_PACKET_MAX_COUNT ? 1 : 0);
				pres->index = i;
				pres->endflag = pres->total == (i + 1) ? 1 : 0;
				pres->count = count;

				i++;
				size -= EVENT_PACKET_MAX_COUNT;

#undef EVENT_PACKET_MAX_COUNT

				for (j = 0; j < count; j++)
				{
					FILE_NODE node;
					if (get_file_node(list, &node) != DMS_NET_FILE_SUCCESS)
					{
						PRINT_INFO("IOTYPE_USER_IPCAM_LISTEVENT_REQ Fail to get file node!\n");
						continue;
					}
                    
					u64t_to_time(&avi_start, node.start);

					u64t_to_time(&avi_stop, node.stop);
                    #if TUTK_DEBUG
                    PRINT_INFO("avi start time node.start:%llu ->%d-%d-%d  %d:%d:%d\n",
                    node.start,
                    avi_start.dwYear,
                    avi_start.dwMonth,
                    avi_start.dwDay,
                    avi_start.dwHour,
                    avi_start.dwMinute,
                    avi_start.dwSecond);

                    PRINT_INFO("avi stop time node.start:%llu ->%d-%d-%d  %d:%d:%d\n",
                    node.stop,
                    avi_stop.dwYear,
                    avi_stop.dwMonth,
                    avi_stop.dwDay,
                    avi_stop.dwHour,
                    avi_stop.dwMinute,
                    avi_stop.dwSecond);
                    #endif




                    
					pres->stEvent[j].stBeginTime.year = avi_start.dwYear;
					pres->stEvent[j].stBeginTime.month = avi_start.dwMonth;
					pres->stEvent[j].stBeginTime.day = avi_start.dwDay;
					pres->stEvent[j].stBeginTime.wday = 0;
					pres->stEvent[j].stBeginTime.hour = avi_start.dwHour;
					pres->stEvent[j].stBeginTime.minute = avi_start.dwMinute;
					pres->stEvent[j].stBeginTime.second = avi_start.dwSecond;
                    #if USE_HTS_PROTOCOL

                    #else//徐辉，柔乐
                    
					pres->stEvent[j].stEndTime.year = avi_stop.dwYear;
					pres->stEvent[j].stEndTime.month = avi_stop.dwMonth;
					pres->stEvent[j].stEndTime.day = avi_stop.dwDay;
					pres->stEvent[j].stEndTime.wday = 0;
					pres->stEvent[j].stEndTime.hour = avi_stop.dwHour;
					pres->stEvent[j].stEndTime.minute = avi_stop.dwMinute;
					pres->stEvent[j].stEndTime.second = avi_stop.dwSecond;
                    #endif

                    #if USE_HTS_PROTOCOL
                    if(AVIOCTRL_EVENT_MOTIONDECT == p->event)
                    {
                        pres->stEvent[j].event = AVIOCTRL_EVENT_MOTIONDECT;
                    }
                    else if(AVIOCTRL_EVENT_ALL == p->event)
                    {
                        if(!list_md)
                        {
                        
                            pres->stEvent[j].event = AVIOCTRL_EVENT_ALL;
                        }
                        else
                        {
                            pres->stEvent[j].event = AVIOCTRL_EVENT_ALL;

                            FILE_NODE *p;
                            int find = 0;
                            p = list_md->head;
                            PRINT_INFO("cur node.start:%llu\n",node.start);
                            while (p->next != NULL)    
                            {        
                                if( p->next->start == node.start)              
                                {
                                    
                                    PRINT_INFO("cur node.start:%llu,md.start:%llu\n",node.start,p->next->start );
                                    
                                    pres->stEvent[j].event = AVIOCTRL_EVENT_MOTIONDECT;
                                    find = 1;
                                    break;
                                }
                                else
                                {
                                    p = p->next; 
                                }
                                   
                            }
                        }
                            
                    }
                    
                        

                    #else
					pres->stEvent[j].event = AVIOCTRL_EVENT_ALL;
                    #endif
					pres->stEvent[j].status = 0;

#if TUTK_DEBUG

                    PRINT_INFO("resp before zone avi start time node.start:%llu ->%d-%d-%d  %d:%d:%d,event:%d\n",
                    node.start,
                    pres->stEvent[j].stBeginTime.year,
                    pres->stEvent[j].stBeginTime.month,
                    pres->stEvent[j].stBeginTime.day,
                    pres->stEvent[j].stBeginTime.hour,
                    pres->stEvent[j].stBeginTime.minute,
                    pres->stEvent[j].stBeginTime.second,
                    pres->stEvent[j].event);
#endif
                    
                    #if USE_HTS_PROTOCOL//宏天顺使用的是减去时区信息
                    
					time_convert(&pres->stEvent[j].stBeginTime, -runSystemCfg.timezoneCfg.timezone * 60);
                    #else
					//time_convert(&pres->stEvent[j].stEndTime, -runSystemCfg.timezoneCfg.timezone * 60);
                    #endif
#if TUTK_DEBUG

                    PRINT_INFO("resp avi start time node.start:%llu ->%d-%d-%d  %d:%d:%d\n",
                    node.start,
                    pres->stEvent[j].stBeginTime.year,
                    pres->stEvent[j].stBeginTime.month,
                    pres->stEvent[j].stBeginTime.day,
                    pres->stEvent[j].stBeginTime.hour,
                    pres->stEvent[j].stBeginTime.minute,
                    pres->stEvent[j].stBeginTime.second);
#endif
                    #if USE_HTS_PROTOCOL

                    #else
#if TUTK_DEBUG                   
                    PRINT_INFO("resp avi stop  time node.stop :%llu ->%d-%d-%d  %d:%d:%d,event:%d\n",
                    node.stop,
                    pres->stEvent[j].stEndTime.year,
                    pres->stEvent[j].stEndTime.month,
                    pres->stEvent[j].stEndTime.day,
                    pres->stEvent[j].stEndTime.hour,
                    pres->stEvent[j].stEndTime.minute,
                    pres->stEvent[j].stEndTime.second,
                    pres->stEvent[j].event);
#endif
                    #endif
				}

				avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_LISTEVENT_RESP, (char *)pres,
					sizeof(SMsgAVIoctrlListEventResp)+sizeof(SAvEvent)*pres->count);

				free(pres);
				pres = NULL;
			}

			break;

LISTEVENT_REQ:
			memset(&res, 0, sizeof(SMsgAVIoctrlListEventResp));
			res.total = 1;
			res.endflag = 1;
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_LISTEVENT_RESP, (char *)&res,
				sizeof(SMsgAVIoctrlListEventResp));
			PRINT_INFO("send IOTYPE_USER_IPCAM_LISTEVENT_RESP\n");
#else
			tutk_file_list_event_process(SID, avIndex, buf, p->event);
#endif
		}
		break;

#if 0
		case IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL:
		{
			SMsgAVIoctrlPlayRecord *p = (SMsgAVIoctrlPlayRecord *)buf;
			SMsgAVIoctrlPlayRecordResp res;

			//PRINT_INFO("IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL cmd[%d]\n\n", p->command);
            /*开启回放指定的录像*/
            if(p->command == AVIOCTRL_RECORD_PLAY_START)
			{
			    PRINT_INFO("IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL, cmd: AVIOCTRL_RECORD_PLAY_START\n");
				memcpy(&gClientInfo[SID].playRecord, p, sizeof(SMsgAVIoctrlPlayRecord));
				res.command = AVIOCTRL_RECORD_PLAY_START;
                int result;
                
                //if (is_pb_doing == 0)
                {
    				if (gClientInfo[SID].playBackCh < 0)//还没有回放通道，需要IOTC_Session_Get_Free_Channel获取回放通道，并将通过result返回给app
    				{
    					gClientInfo[SID].bPausePlayBack = 0;
    					gClientInfo[SID].bStopPlayBack = 0;
    					gClientInfo[SID].playBackCh = IOTC_Session_Get_Free_Channel(SID);
    					result = gClientInfo[SID].playBackCh;
    				}
    				else//表明已经有回放通道，正在回放
                    {            
    					result = -1;
                    }
                    PRINT_INFO("result:%d\n", result);
                    
    				if (result >= 0)
    				{
    					pthread_t ThreadID;
    					PRINT_INFO("[%d] start playback\n", result);

    					if(pthread_create(&ThreadID, NULL, &thread_PlayBack, (void *)SID))
    					{
    						PRINT_INFO("pthread_create thread_PlayBack fail\n");
    						result = -1;
    					}
    				}
    				else
    					PRINT_INFO("Playback on SID %d is still functioning\n", SID);

                    res.result = result;
    				avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP,
    					(char *)&res, sizeof(SMsgAVIoctrlPlayRecordResp));

                    //is_pb_doing = 1;
                }
                /*
                else
                {
                    PRINT_INFO("Playback is working, just support one client to playback.\n");
                    res.result = -1;
    				avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP,
    					(char *)&res, sizeof(SMsgAVIoctrlPlayRecordResp));
                }
                */
			}
            /*暂停播放录像*/
			else if(p->command == AVIOCTRL_RECORD_PLAY_PAUSE)
			{
			    PRINT_INFO("IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL, cmd: AVIOCTRL_RECORD_PLAY_PAUSE\n");
                res.command = AVIOCTRL_RECORD_PLAY_PAUSE;
                //if (is_pb_doing == 1)
                {
    				gClientInfo[SID].bPausePlayBack = !gClientInfo[SID].bPausePlayBack;
    				res.result = 0;
                }
                /*
                else
                {
                    res.result = -1;
                }
                */
				avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP, (char *)&res,
					sizeof(SMsgAVIoctrlPlayRecordResp));
			}
            /*进度条拖动到指定位置SEEK*/
            else if(p->command == AVIOCTRL_RECORD_PLAY_SEEKTIME)
            {
                PRINT_INFO("IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL, cmd: AVIOCTRL_RECORD_PLAY_SEEKTIME\n");

                //if (is_pb_doing == 1)
                if(gClientInfo[SID].playBackCh >= 0)
                {
                    gClientInfo[SID].bPausePlayBack = 1;
                    usleep(50000);
                    
                    int seekPos;
                    AVI_DMS_TIME timeDay;
                    #if USE_HTS_PROTOCOL //宏天顺的协议，时区是减8的。徐辉和柔乐的按照osd的时间，即本地时间
                    time_convert(&p->stTimeDay, runSystemCfg.timezoneCfg.timezone * 60);
                    #endif
                    timeDay.dwYear = p->stTimeDay.year;
                    timeDay.dwMonth = p->stTimeDay.month;
                    timeDay.dwDay= p->stTimeDay.day;
                    timeDay.dwHour= p->stTimeDay.hour;
                    timeDay.dwMinute= p->stTimeDay.minute;
                    timeDay.dwSecond= p->stTimeDay.second;


                    PRINT_INFO("-----------> %u-%u-%u %u:%u:%u\n", timeDay.dwYear, timeDay.dwMonth, timeDay.dwDay,
                        timeDay.dwHour, timeDay.dwMinute, timeDay.dwSecond);
                    PRINT_INFO("%s\n", p->Param == E_PB_BY_FILE ? "pb seektime by file" : "pb seektime by time");
                    //seekPos = cal_pb_seek_time(pbhandle,&timeDay);
                    seekPos = cal_pb_seek_time(pbhandle[SID],&timeDay);
                    if(seekPos < 0)
                    {
                        PRINT_ERR("seek position err\n");
                        return ;
                    }
                    /*
                    int ret = avi_pb_open(pbhandle->file_path, pbhandle);
                    if (ret < 0) {
                        PRINT_ERR("avi_pb_open %s error.\n", pbhandle->file_path);
                        return ;
                    }
                    */
                    int ret = avi_pb_open(pbhandle[SID]->file_path, pbhandle[SID]);
                    if (ret < 0) {
                        PRINT_ERR("avi_pb_open %s error.\n", pbhandle[SID]->file_path);
						//PRINT_ERR("avi_pb_open %s error.\n", pbhandle->file_path);
                        return ;
                    }

                    PRINT_INFO("----------->\n");
                    //avi_pb_set_pos(pbhandle, &seekPos);
                    avi_pb_set_pos(pbhandle[SID], &seekPos);

                    gClientInfo[SID].bPausePlayBack = 0;

                }
                
            }
                        
            /*停止播放*/
			else if(p->command == AVIOCTRL_RECORD_PLAY_STOP)
			{
			    PRINT_INFO("IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL, cmd: AVIOCTRL_RECORD_PLAY_STOP\n");
                //if (is_pb_doing == 1)
                if(gClientInfo[SID].playBackCh >= 0)                
                {
                    gClientInfo[SID].bStopPlayBack = 1;
                    //is_pb_doing = 0;
                }
                else
                {
                    PRINT_INFO("is already stop, can't sotp twice.\n");;
                }
			}
		}
		break;
#else
		case IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL:
		{

			SMsgAVIoctrlPlayRecord *p = (SMsgAVIoctrlPlayRecord *)buf;
			SMsgAVIoctrlPlayRecordResp resp;

			gClientInfo[SID].bPBSensorNo = p->channel;
			
			/*开启回放指定的录像*/
			if(p->command == AVIOCTRL_RECORD_PLAY_START)
			{
				PRINT_INFO("IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL, cmd: AVIOCTRL_RECORD_PLAY_START\n");
				memcpy(&gClientInfo[SID].playRecord, p, sizeof(SMsgAVIoctrlPlayRecord));
				resp.command = AVIOCTRL_RECORD_PLAY_START;
				int result;
				int cnt = 0;

				while(gClientInfo[SID].bStopPlayBack == 1 && gClientInfo[SID].playBackCh >= 0 && cnt++ < 50)//wait AVIOCTRL_RECORD_PLAY_STOP process
				{
					if(cnt%10 == 0)
						PRINT_INFO("1.Playback on SID %d is still functioning\n", SID);
					usleep(80000);
				}

				#if 0
				pthread_mutex_lock(&gClientInfo[SID].sLock);
				if (gClientInfo[SID].playBackCh < 0)//还没有回放通道，需要IOTC_Session_Get_Free_Channel获取回放通道，并将通过result返回给app
				{
					gClientInfo[SID].bPausePlayBack = 0;
					gClientInfo[SID].bStopPlayBack = 0;
					gClientInfo[SID].playBackCh = IOTC_Session_Get_Free_Channel(SID);
					result = gClientInfo[SID].playBackCh;
				}
				else//表明已经有回放通道，正在回放
				{			 
					result = -1;
					gClientInfo[SID].bStopPlayBack = 1;
				}
				
				pthread_mutex_unlock(&gClientInfo[SID].sLock);
				#else
				static int timeOut = 0;
				while(gClientInfo[SID].playBackCh > 0)//表明已经有回放通道，正在回放
				{
					timeOut++;
					PRINT_INFO("timeout:%d\n",timeOut);
					if(timeOut > AV_SERV_START_TIME_OUT)
					{
						result = -1;
						break;
					}
					gClientInfo[SID].bStopPlayBack = 1;
					sleep(1);
				}
				timeOut = 0;
				
				pthread_mutex_lock(&gClientInfo[SID].sLock);
				if (gClientInfo[SID].playBackCh < 0)//还没有回放通道，需要IOTC_Session_Get_Free_Channel获取回放通道，并将通过result返回给app
				{
					gClientInfo[SID].bPausePlayBack = 0;
					gClientInfo[SID].bStopPlayBack = 0;
					gClientInfo[SID].playBackCh = IOTC_Session_Get_Free_Channel(SID);
					result = gClientInfo[SID].playBackCh;
				}
				pthread_mutex_unlock(&gClientInfo[SID].sLock);
				#endif
				PRINT_INFO("result:%d\n", result);
				
				if (result >= 0)
				{
					pthread_t ThreadID;
					PRINT_INFO("[%d] start playback\n", result);

					if(pthread_create(&ThreadID, NULL, &thread_PlayBack, (void *)SID))
					{
						PRINT_INFO("pthread_create thread_PlayBack fail\n");
						result = -1;
					}
				}
				else
					PRINT_INFO("Playback on SID %d is still running\n", SID);

				resp.result = result;
				avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP,(char *)&resp, sizeof(SMsgAVIoctrlPlayRecordResp));
			}
			/*暂停播放录像*/
			else if(p->command == AVIOCTRL_RECORD_PLAY_PAUSE)
			{
				PRINT_INFO("IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL, cmd: AVIOCTRL_RECORD_PLAY_PAUSE\n");
				resp.command = AVIOCTRL_RECORD_PLAY_PAUSE;
				resp.result = 0;
				gClientInfo[SID].bPausePlayBack = !gClientInfo[SID].bPausePlayBack;

				avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP, (char *)&resp,sizeof(SMsgAVIoctrlPlayRecordResp));
			}
			/*进度条拖动到指定位置SEEK*/
			else if(p->command == AVIOCTRL_RECORD_PLAY_SEEKTIME)
			{
				PRINT_INFO("IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL, ch:%d, cmd: AVIOCTRL_RECORD_PLAY_SEEKTIME\n",p->channel);
				pthread_mutex_lock(&gClientInfo[SID].sLock);
				gClientInfo[SID].bSeekTimeCmd = 1;
				gClientInfo[SID].stSeekTimeDay.dwYear = p->stTimeDay.year;
				gClientInfo[SID].stSeekTimeDay.dwMonth = p->stTimeDay.month;
				gClientInfo[SID].stSeekTimeDay.dwDay= p->stTimeDay.day;
				gClientInfo[SID].stSeekTimeDay.dwHour= p->stTimeDay.hour;
				gClientInfo[SID].stSeekTimeDay.dwMinute= p->stTimeDay.minute;
				gClientInfo[SID].stSeekTimeDay.dwSecond= p->stTimeDay.second;
				pthread_mutex_unlock(&gClientInfo[SID].sLock);

			}			   
			/*停止播放*/
			else if(p->command == AVIOCTRL_RECORD_PLAY_STOP)
			{
				PRINT_INFO("IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL, cmd: AVIOCTRL_RECORD_PLAY_STOP\n");
				pthread_mutex_lock(&gClientInfo[SID].sLock);
				gClientInfo[SID].bStopPlayBack = 1;
				pthread_mutex_unlock(&gClientInfo[SID].sLock);
			}
		}
		break;

#endif
        case IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_REQ:
        {
			SMsgAVIoctrlGetAudioOutFormatReq* p = (SMsgAVIoctrlGetAudioOutFormatReq*)buf;
			SMsgAVIoctrlGetAudioOutFormatResp resp;
			PRINT_INFO("IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_REQ resp MEDIA_CODEC_AUDIO_PCM\n");

			resp.channel = p->channel;
			resp.codecId = MEDIA_CODEC_AUDIO_PCM;

            avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetAudioOutFormatResp));
        }
		break;

        case IOTYPE_USER_IPCAM_SET_ENVIRONMENT_REQ:
        {
			int ret;
			SMsgAVIoctrlSetEnvironmentReq* p = (SMsgAVIoctrlSetEnvironmentReq*)buf;
			SMsgAVIoctrlSetEnvironmentResp res;
			GK_NET_IMAGE_CFG stImagingConfig;

    		memset(&stImagingConfig, 0, sizeof(GK_NET_IMAGE_CFG));
    		ret = netcam_image_get(&stImagingConfig);
    		if (ret != 0)
				goto SET_ENVIRONMENT;

			if (p->mode == AVIOCTRL_ENVIRONMENT_NIGHT)
				stImagingConfig.irCutMode = 2;            
			else if (p->mode == AVIOCTRL_ENVIRONMENT_DAY)  
                stImagingConfig.irCutMode = 1;
			else
				stImagingConfig.irCutMode = 0;

			ret = netcam_image_set(stImagingConfig);
    		if (ret != 0)
				goto SET_ENVIRONMENT;

			//CREATE_WORK(SetImage, EVENT_TIMER_WORK, (WORK_CALLBACK)ImageCfgSave);
			//INIT_WORK(SetImage, COMPUTE_TIME(0,0,0,2,0), NULL);
			//SCHEDULE_DEFAULT_WORK(SetImage);
			CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,2,0), (WORK_CALLBACK)ImageCfgSave);

SET_ENVIRONMENT:
			res.result = 0;
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_ENVIRONMENT_RESP, (char *)&res, sizeof(SMsgAVIoctrlSetEnvironmentResp));
        }
		break;

        case IOTYPE_USER_IPCAM_GET_ENVIRONMENT_REQ:
        {
			SMsgAVIoctrlGetEnvironmentResp res;
			GK_NET_IMAGE_CFG stImagingConfig;

			memset(&stImagingConfig, 0, sizeof(GK_NET_IMAGE_CFG));
			netcam_image_get(&stImagingConfig);

			if (stImagingConfig.irCutMode == 2)
				res.mode = AVIOCTRL_ENVIRONMENT_NIGHT;
			if (stImagingConfig.irCutMode == 1)
				res.mode = AVIOCTRL_ENVIRONMENT_DAY;            
			else
				res.mode = AVIOCTRL_ENVIRONMENT_OUTDOOR;

            avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_ENVIRONMENT_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetEnvironmentResp));
        }
		break;

        case IOTYPE_USER_IPCAM_SET_VIDEOMODE_REQ:
        {
			int ret;
			SMsgAVIoctrlSetVideoModeReq* p = (SMsgAVIoctrlSetVideoModeReq*)buf;
			SMsgAVIoctrlSetVideoModeResp res;
			GK_NET_IMAGE_CFG stImagingConfig;
/*
SDK_U8      flipEnabled; //垂直 翻转
SDK_U8      mirrorEnabled; // 水平 翻转

*/
            PRINT_INFO("old flipEnabled:%d,mirrorEnabled:%d\n",runImageCfg.flipEnabled,runImageCfg.mirrorEnabled);
    		memset(&stImagingConfig, 0, sizeof(GK_NET_IMAGE_CFG));
    		ret = netcam_image_get(&stImagingConfig);
    		if (ret != 0)
				goto SET_VIDEOMODE;

			if (p->mode & AVIOCTRL_VIDEOMODE_FLIP)
				stImagingConfig.flipEnabled = 1;
			else
				stImagingConfig.flipEnabled = 0;

			if (p->mode & AVIOCTRL_VIDEOMODE_MIRROR)
				stImagingConfig.mirrorEnabled = 1;
			else
				stImagingConfig.mirrorEnabled = 0;
            PRINT_INFO("new flipEnabled:%d,mirrorEnabled:%d\n",runImageCfg.flipEnabled,runImageCfg.mirrorEnabled);

			ret = netcam_image_set(stImagingConfig);
    		if (ret != 0)
				goto SET_VIDEOMODE;

			//CREATE_WORK(SetImage, EVENT_TIMER_WORK, (WORK_CALLBACK)ImageCfgSave);
			//INIT_WORK(SetImage, COMPUTE_TIME(0,0,0,2,0), NULL);
			//SCHEDULE_DEFAULT_WORK(SetImage);
			CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,2,0), (WORK_CALLBACK)ImageCfgSave);

SET_VIDEOMODE:
			res.result = 0;
            avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_VIDEOMODE_RESP, (char *)&res, sizeof(SMsgAVIoctrlSetVideoModeResp));
        }
		break;

        case IOTYPE_USER_IPCAM_GET_VIDEOMODE_REQ:
        {
			int ret;
            SMsgAVIoctrlGetVideoModeResp res;
			GK_NET_IMAGE_CFG stImagingConfig;

			memset(&stImagingConfig, 0, sizeof(GK_NET_IMAGE_CFG));
    		ret = netcam_image_get(&stImagingConfig);
    		if (ret != 0)
				goto GET_VIDEOMODE;

			memset(&res, 0, sizeof(SMsgAVIoctrlGetVideoModeResp));

			if (stImagingConfig.flipEnabled == 1)
				res.mode |= AVIOCTRL_VIDEOMODE_FLIP;
			if (stImagingConfig.mirrorEnabled == 1)
				res.mode |= AVIOCTRL_VIDEOMODE_MIRROR;

            PRINT_INFO("IOTYPE_USER_IPCAM_GET_VIDEOMODE_REQ res.mode:%d\n",res.mode);

GET_VIDEOMODE:
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_VIDEOMODE_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetVideoModeResp));
        }
		break;

        case IOTYPE_USER_IPCAM_FORMATEXTSTORAGE_REQ:
        {
            PRINT_INFO("[p2p] cmd:IOTYPE_USER_IPCAM_FORMATEXTSTORAGE_REQ\n");
			SMsgAVIoctrlFormatExtStorageResp res;

			grd_sd_format();
            format_runing = 1;
            #if USE_HTS_PROTOCOL
            int count = 0 ;
            while(1)
            {
                if(5 == mmc_get_sdcard_stauts() && grd_sd_get_format_process() >= 85)
                {
                    break;
                }
                sleep(1);
                count ++;
                if(count > 30)//30秒超时
                {                    
                    break;
                }
                PRINT_INFO("get sd status:%d,process:%d\n",mmc_get_sdcard_stauts(),grd_sd_get_format_process());
            }
            #endif
			res.result = 0;
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_FORMATEXTSTORAGE_RESP, (char *)&res, sizeof(SMsgAVIoctrlFormatExtStorageResp));
        }
		break;

        case IOTYPE_USER_IPCAM_PTZ_COMMAND:
        {
            int ret = 0;
			SMsgAVIoctrlPtzCmd *p = (SMsgAVIoctrlPtzCmd *)buf;

            PRINT_INFO("FUN[%s]  LINE[%d]  IOTYPE_USER_IPCAM_PTZ_COMMAND, ch:%d, avIndex:%d,control cmd:%d\n \n", __FUNCTION__, __LINE__, p->channel, avIndex,p->control);
            #if USE_HTS_PROTOCOL
            unsigned char cmd = 0;

            if(runImageCfg.flipEnabled && p->control == AVIOCTRL_PTZ_UP)
            {
                PRINT_INFO("flipEnabled and AVIOCTRL_PTZ_UP");
                cmd = AVIOCTRL_PTZ_DOWN;
            }
            else if(runImageCfg.flipEnabled && p->control == AVIOCTRL_PTZ_DOWN)
            {          
                PRINT_INFO("flipEnabled and AVIOCTRL_PTZ_DOWN");
                cmd = AVIOCTRL_PTZ_UP;
            }
            else if(runImageCfg.mirrorEnabled && p->control == AVIOCTRL_PTZ_LEFT)
            {
                PRINT_INFO("mirrorEnabled and AVIOCTRL_PTZ_LEFT");

                cmd = AVIOCTRL_PTZ_RIGHT;
            }    
            else if(runImageCfg.mirrorEnabled && p->control == AVIOCTRL_PTZ_RIGHT)
            {
            
                PRINT_INFO("mirrorEnabled and AVIOCTRL_PTZ_RIGHT");
                cmd = AVIOCTRL_PTZ_LEFT;
            }
            else
            {
            
                 PRINT_INFO("not mirrorEnabled ,not flipEnabled");
                 cmd = p->control;           
            }
       
            ret = ptz_process(p->channel, cmd, p->speed, p->point);
            #else
            ret = ptz_process(p->channel, p->control, p->speed, p->point);
            #endif

            if (ret)
                PRINT_INFO("FUN[%s]  LINE[%d]  Do ptz_process error!\n", __FUNCTION__, __LINE__);
			else
            	PRINT_INFO("FUN[%s]  LINE[%d]  Do IOTYPE_USER_IPCAM_PTZ_COMMAND OK!\n", __FUNCTION__, __LINE__);
        }
		break;

        case IOTYPE_USER_IPCAM_EVENT_REPORT:
        {
            /* This message should be sent from IPCamera to App.
                        * Send notification to App if IPCamera triggered an event.
                        * typedef enum
                            {
                                AVIOCTRL_EVENT_ALL              = 0x00, // all event type
                                AVIOCTRL_EVENT_MOTIONDECT   = 0x01, // motion detect start
                                AVIOCTRL_EVENT_VIDEOLOST        = 0x02, // video lost alarm
                                AVIOCTRL_EVENT_IOALARM          = 0x03, // IO alarm start

                                AVIOCTRL_EVENT_MOTIONPASS       = 0x04, // motion detect end
                                AVIOCTRL_EVENT_VIDEORESUME      = 0x05, // video resume
                                AVIOCTRL_EVENT_IOALARMPASS      = 0x06, // IO alarm end

                                AVIOCTRL_EVENT_EXPT_REBOOT      = 0x10, // system exception reboot
                                AVIOCTRL_EVENT_SDFAULT          = 0x11, // SDCard record exception
                            }ENUM_EVENTTYPE;
                       */
            break;
        }

        case IOTYPE_USER_IPCAM_GET_TIMEZONE_REQ:
        {
			SMsgAVIoctrlTimeZone timeZoneRes;
			memset(&timeZoneRes, 0, sizeof(SMsgAVIoctrlTimeZone));
            timeZoneRes.cbSize              = sizeof(SMsgAVIoctrlTimeZone);
            timeZoneRes.nIsSupportTimeZone  = 1;
            //timeZoneRes.nTimeZone           = runSystemCfg.timezoneCfg.timezone;

            /*时区直接用runSystemCfg.timezoneCfg.timezone替换，即分钟数，更方便*/
            /*
            int i;
            for (i = 0; i < sizeof(tutk_time_zone_table)/sizeof(int); i++)
            {
                if (runSystemCfg.timezoneCfg.timezone == tutk_time_zone_table[i])
                    break;
            }
            
            timeZoneRes.nTimeZone = (i == sizeof(tutk_time_zone_table)/sizeof(int) ? 0 : i);
            */
#if USE_HTS_PROTOCOL
            timeZoneRes.nGMTDiff            = runSystemCfg.timezoneCfg.timezone;
			strncpy(timeZoneRes.szTimeZoneString, runSystemCfg.timezoneCfg.time_desc, sizeof(timeZoneRes.szTimeZoneString));
#else
			timeZoneRes.nTimeZone = runSystemCfg.timezoneCfg.timezone;
#endif
            PRINT_INFO("get timezone: zone:%d desc:%s\n", runSystemCfg.timezoneCfg.timezone, runSystemCfg.timezoneCfg.time_desc);
            
			if (avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_TIMEZONE_RESP, (char *)&timeZoneRes, sizeof(SMsgAVIoctrlTimeZone)) < 0)
                PRINT_INFO("get timezone, send resp error!\n");
        }
		break;

        case IOTYPE_USER_IPCAM_SET_TIMEZONE_REQ:
        {
            SMsgAVIoctrlTimeZone *p = (SMsgAVIoctrlTimeZone *)buf;

			PRINT_INFO("set timezone, cbSize:%d,nTimeZone:%d,nIsSupportTimeZone:%d,szTimeZoneString:%s\n",
				p->cbSize, 
#if USE_HTS_PROTOCOL
				p->nGMTDiff, 
#else
				p->nTimeZone,
#endif
				p->nIsSupportTimeZone, 
				p->szTimeZoneString);

#if USE_HTS_PROTOCOL
            netcam_sys_set_time_zone_by_utc_second(0, p->nGMTDiff);
            runSystemCfg.timezoneCfg.timezone = p->nGMTDiff;
			strncpy(runSystemCfg.timezoneCfg.time_desc, p->szTimeZoneString, sizeof(runSystemCfg.timezoneCfg.time_desc));
#else
			netcam_sys_set_time_zone_by_utc_second(0, p->nTimeZone);            
			runSystemCfg.timezoneCfg.timezone = p->nTimeZone;
#endif
            SystemCfgSave();
            PRINT_INFO("cfg timezone:%d\n", runSystemCfg.timezoneCfg.timezone);

            if (avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_TIMEZONE_RESP, (char *)p, sizeof(SMsgAVIoctrlTimeZone)) < 0)
                PRINT_INFO("set timezone, send resp error!\n");
        }
		break;

		case IOTYPE_USER_IPCAM_SET_DEVICETIME_REQ:
		{
			SMsgAVIoctrlSetDeviceTimeReq* p = (SMsgAVIoctrlSetDeviceTimeReq*)buf;
			SMsgAVIoctrlSetDeviceTimeResp res;

			PRINT_INFO("set dev time, year:%d,month:%d,day:%d,hour:%d,minute:%d,second:%d,nIsSupportSync:%d,nGMTOffset:%d\n",
				p->year, p->month, p->day, p->hour, p->minute, p->second, p->nIsSupportSync, p->nGMTOffset);

			if (p->nIsSupportSync)
			{
				struct tm temp;
                time_t tt;

				temp.tm_year = p->year - 1900;
				temp.tm_mon = p->month - 1;
				temp.tm_mday = p->day;
				temp.tm_hour = p->hour;
				temp.tm_min = p->minute;
				temp.tm_sec = p->second;

				tt = mktime(&temp);
                //mktime和TZ环境变量相关联，系统默认设置了TZ环境变量，
                //则mktime计算会减去TZ代表的时区，使用netcam_sys_set_time_zone_by_utc_second
                //需要传入一个与当前UTC的差
                #if USE_HTS_PROTOCOL
				tt -= p->nGMTOffset*60;
                #endif
                printf("set tt :%d\n",tt);
				netcam_sys_set_time_zone_by_utc_second(tt, p->nGMTOffset);
                SystemCfgSave();
			}

            res.result = 0;
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_DEVICETIME_RESP, (char *)&res, sizeof(SMsgAVIoctrlSetDeviceTimeResp));

            
		}
		break;
		case IOTYPE_USER_IPCAM_GET_DEVICETIME_REQ:
		{
			//SMsgAVIoctrlSetDeviceTimeReq* p = (SMsgAVIoctrlSetDeviceTimeReq*)buf;
			SMsgAVIoctrlGetDeviceTimeResp res = {0};
            STimeDay td;
            get_localTime(&td);
          
            res.year = td.year;
            res.month = td.month;
            res.day = td.day;
            res.hour = td.hour;
            res.minute = td.minute;
            res.second = td.second;
            res.nGMTOffset = runSystemCfg.timezoneCfg.timezone ;
          
            PRINT_INFO("get dev time, year:%d,month:%d,day:%d,hour:%d,minute:%d,second:%d ,timezone:%d\n",
              res.year, res.month, res.day, res.hour, res.minute, res.second,res.nGMTOffset);
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_DEVICETIME_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetDeviceTimeResp));
		}
		break;

		case IOTYPE_USER_IPCAM_SETPRESET_REQ:
		{
			SMsgAVIoctrlSetPresetReq* p = (SMsgAVIoctrlSetPresetReq*)buf;
			SMsgAVIoctrlSetPresetResp res;
			GK_NET_PRESET_INFO gkPresetCfg;

            get_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            gkPresetCfg.nPresetNum++;
            if (gkPresetCfg.nPresetNum >= NETCAM_PTZ_MAX_PRESET_NUM)
            {
                gkPresetCfg.nPresetNum = NETCAM_PTZ_MAX_PRESET_NUM;
            }
            gkPresetCfg.no[gkPresetCfg.nPresetNum-1] = p->nPresetIdx;
            set_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            PresetCruiseCfgSave();
            netcam_ptz_set_preset(p->nPresetIdx, NULL);

			res.result = 0;
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETPRESET_RESP, (char *)&res, sizeof(SMsgAVIoctrlSetPresetResp));
		}
		break;

		case IOTYPE_USER_IPCAM_GETPRESET_REQ:
		{
			SMsgAVIoctrlGetPresetReq* p = (SMsgAVIoctrlGetPresetReq*)buf;
			GK_NET_CRUISE_GROUP  cruise_info;

            cruise_info.byPointNum    = 1;
            cruise_info.byCruiseIndex = 0;
            cruise_info.struCruisePoint[0].byPointIndex = 0;
            cruise_info.struCruisePoint[0].byPresetNo   = p->nPresetIdx;
            cruise_info.struCruisePoint[0].byRemainTime = 0;
            cruise_info.struCruisePoint[0].bySpeed      = 3;

            netcam_ptz_stop();
            netcam_ptz_preset_cruise(&cruise_info);

			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETPRESET_RESP, (char *)p, sizeof(SMsgAVIoctrlGetPresetResp));
		}
		break;

		case IOTYPE_HICHIP_CRUISE_START:
		{
			SMsgAVIoctrlCruiseStart* p = (SMsgAVIoctrlCruiseStart*)buf;

			if (p->mode == AVIOCTRL_CRUISEMODE_VERTICAL)
				netcam_ptz_vertical_cruise(5);
			else if (p->mode == AVIOCTRL_CRUISEMODE_HORIZONTAL)
				netcam_ptz_horizontal_cruise(5);
		}
		break;

		case IOTYPE_HICHIP_CRUISE_STOP:
		{
			netcam_ptz_stop();
		}
		break;

        case IOTYPE_USER_IPCAM_DOOROPEN_REQ:
		{
			SMsgAVIoctrlDoorOpenResp res;
//			CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), door_open, (void*)1);
//			CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,30,0), door_open, (void*)0); // for tw9912

			memset(&res, 0, sizeof(SMsgAVIoctrlDoorOpenResp));
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_DOOROPEN_RESP, (char *)&res, sizeof(SMsgAVIoctrlDoorOpenResp));
		}
		break;

		case IOTYPE_USER_IPCAM_SET_PUSH_REQ:
		{
            PRINT_INFO("cmd: IOTYPE_USER_IPCAM_SET_PUSH_REQ\n");
			SMsgAVIoctrlSetPushReq* p = (SMsgAVIoctrlSetPushReq*)buf;
			SMsgAVIoctrlSetPushResp res;
			int i = 0;
			memset(&res, 0, sizeof(SMsgAVIoctrlSetPushResp));

			GET_LOCK(&PushDevsLock);
			
			for(i=0;i<PUSH_MAX;i++){
				if(strcmp(PushDevs[i].Alias,p->Alias) == 0){
					PRINT_INFO("msg push alias:[%s] already exists.\n",PushDevs[i].Alias);
					res.result = -1;
					break;
				}
				
				if(strlen(PushDevs[i].Alias) == 0){
					memcpy(&PushDevs[i], p, sizeof(SMsgAVIoctrlSetPushReq));
					res.result =  0;
					break;
				}
			}

			PUT_LOCK(&PushDevsLock);

			if(res.result == 0){
                PRINT_INFO("SavePushDevs\n");
				SavePushDevs();	// 保存推送信息到配置文件
			}

			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_PUSH_RESP, (char *)&res, sizeof(SMsgAVIoctrlSetPushResp));
		}
		break;

		case IOTYPE_USER_IPCAM_DEL_PUSH_REQ:
		{
			SMsgAVIoctrlDelPushReq* p = (SMsgAVIoctrlDelPushReq*)buf;
			SMsgAVIoctrlDelPushResp res;

			int i = 0;

			GET_LOCK(&PushDevsLock);
			for(i=0;i<PUSH_MAX;i++){
				if(strcmp(PushDevs[i].Alias,p->Alias) == 0){
					memset(&PushDevs[i], 0, sizeof(SMsgAVIoctrlSetPushReq));
					break;
				}
			}
			PUT_LOCK(&PushDevsLock);

			SavePushDevs();	// 保存推送信息到配置文件

			memset(&res, 0, sizeof(SMsgAVIoctrlDelPushResp));

			PRINT_INFO("****************************************************************\n");
			PRINT_INFO("Remove Msg Push Device:\nappkey:%s\nmaster:%s\nalias:%s\ntype:%d\n",
				p->AppKey,p->Master,p->Alias,p->Type);
			PRINT_INFO("****************************************************************\n");

			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_DEL_PUSH_RESP, (char *)&res, sizeof(SMsgAVIoctrlDelPushResp));
		}
		break;

		case IOTYPE_USER_IPCAM_SET_MDP_REQ:{
            PRINT_INFO("cmd: IOTYPE_USER_IPCAM_SET_MDP_REQ\n");
            #if 0
			SMsgAVIoctrlSetMDPReq * p = (SMsgAVIoctrlSetMDPReq *)buf;
			SMsgAVIoctrlSetMDPResp  res;

			memset(&res,0,sizeof(res));

			sDAS.StartHour = p->MotionStartHour;
			sDAS.StartMins = p->MotionStartMins;
			sDAS.CloseHour = p->MotionCloseHour;
			sDAS.CloseMins = p->MotionCloseMins;
			sDAS.AlarmFrequency = p->MotionAlarmFrequency;
			runMdCfg.sensitive = p->MotionLevel * 20;
			runMdCfg.enable = p->MotionEnable;

            PRINT_INFO("SET MDCFG S:[%d:%d] E:[%d:%d].\n",
                p->MotionStartHour,
			    p->MotionStartMins,
			    p->MotionCloseHour,
			    p->MotionCloseMins);
			
			CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,1,0), (WORK_CALLBACK)MdCfgSave);

			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_MDP_RESP, (char *)&res, sizeof(res));
            #else
            SMsgAVIoctrlSetMDPReq* p = (SMsgAVIoctrlSetMDPReq*)buf;

            PRINT_INFO("SET MDCFG enable:%d S:[%d:%d] E:[%d:%d].\n",
                p->MotionEnable,
                p->MotionStartHour,
			    p->MotionStartMins,
			    p->MotionCloseHour,
			    p->MotionCloseMins);

			runMdCfg.enable = p->MotionEnable;
            int i = 0, j = 0;
            for (i = 0; i < 7; i++) {
                for (j = 0; j < 4; j++) {
                    runMdCfg.scheduleTime[i][j].startHour = p->MotionStartHour;
                    runMdCfg.scheduleTime[i][j].startMin  = p->MotionStartMins;
                    runMdCfg.scheduleTime[i][j].stopHour  = p->MotionCloseHour;
                    runMdCfg.scheduleTime[i][j].stopMin   = p->MotionCloseMins;
                }
            }
            
            MdCfgSave();

            SMsgAVIoctrlSetMDPResp res;
			memset(&res, 0, sizeof(SMsgAVIoctrlSetMDPResp));
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETMOTIONDETECT_RESP, (char *)&res, sizeof(SMsgAVIoctrlSetMDPResp));
            #endif
		}
		break;

		case IOTYPE_USER_IPCAM_GET_MDP_REQ:{
            PRINT_INFO("cmd: IOTYPE_USER_IPCAM_GET_MDP_REQ\n");
            #if 0
			//SMsgAVIoctrlGetMDPReq * p = (SMsgAVIoctrlGetMDPReq *)buf;
			SMsgAVIoctrlGetMDPResp  res;

			memset(&res,0,sizeof(res));

			res.MotionStartHour = sDAS.StartHour;
			res.MotionStartMins = sDAS.StartMins;
			res.MotionCloseHour = sDAS.CloseHour;
			res.MotionCloseMins = sDAS.CloseMins;
			res.MotionEnable = runMdCfg.enable;
			res.MotionLevel = runMdCfg.sensitive/20;
			res.MotionAlarmFrequency = sDAS.AlarmFrequency;

            PRINT_INFO("GET MDCFG S:[%d:%d] E:[%d:%d].\n",
                res.MotionStartHour,
			    res.MotionStartMins,
			    res.MotionCloseHour,
			    res.MotionCloseMins);

			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_MDP_RESP, (char *)&res, sizeof(res));
            #else
            SMsgAVIoctrlGetMDPResp res;

			res.MotionEnable = runMdCfg.enable;
			res.MotionStartHour = runMdCfg.scheduleTime[0][0].startHour;
            res.MotionStartMins = runMdCfg.scheduleTime[0][0].startMin;
            res.MotionCloseHour = runMdCfg.scheduleTime[0][0].stopHour;
            res.MotionCloseMins = runMdCfg.scheduleTime[0][0].stopMin;


            PRINT_INFO("GET MDCFG enable:%d [%d:%d] -- [%d:%d].\n", res.MotionEnable, res.MotionStartHour, 
                   res.MotionStartMins, res.MotionCloseHour, res.MotionCloseMins);
            
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETMOTIONDETECT_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetMDPResp));
            #endif
		}
		break;

        #if 0
        case IOTYPE_USER_IPCAM_SET_SDCARD_REQ:{
            PRINT_INFO("cmd: IOTYPE_USER_IPCAM_SET_SDCARD_REQ\n");
            PRINT_INFO("cmd: to format sd card.\n");
            
            SMsgAVIoctrlSetSDCardResp res;
			memset(&res, 0, sizeof(SMsgAVIoctrlSetSDCardResp));
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_SDCARD_RESP, (char *)&res, sizeof(SMsgAVIoctrlSetSDCardResp));
        }
        break;
        #endif

        case IOTYPE_USER_IPCAM_GET_SDCARD_REQ:{
            SMsgAVIoctrlGetSDCardResp res;
            res.status = mmc_get_sdcard_stauts();
            res.size = grd_sd_get_all_size();
            res.free = grd_sd_get_free_size();
			
			if(res.status == 5)
            	res.format_process = grd_sd_get_format_process();
#if 0
            //格式化过程不需要要出现未初始化状态
            if(format_runing)
            {   
                if(res.status == 0)//格式化后会由5变成0(未初始化)，其实这个状态不需要
                {
                    res.status = 5;
                }    
                if(res.status == 3)//格式化命令结束，跳转到3(格式化ok)需要将正在格式化的这个标志取消
                {
                    format_runing = 0;
                }
            }
#endif
            PRINT_INFO("get sdcard, status:%d all_size:%d free_size:%d process:%d\n", res.status, res.size, res.free, res.format_process);
            avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_SDCARD_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetSDCardResp));
        }
        break;

        case IOTYPE_USER_IPCAM_GET_OSD_REQ:{
            PRINT_INFO("cmd: IOTYPE_USER_IPCAM_GET_OSD_REQ\n");

            /* 获取 */
            SMsgAVIoctrlGetOSDResp* p = (SMsgAVIoctrlGetOSDResp*)buf;
            PRINT_INFO("get osd, channel:%d\n", p->channel);

            /* 处理 */
            SMsgAVIoctrlGetOSDResp res = {0};
            res.channel = p->channel;
            res.channel_name_enbale = runChannelCfg.channelInfo[0].osdChannelName.enable;
            strncpy(res.channel_name_text, runChannelCfg.channelInfo[0].osdChannelName.text, sizeof(runChannelCfg.channelInfo[0].osdChannelName.text));

            /* 回复*/
            PRINT_INFO("get osd, channel:%d, enable:%d, text:%s\n", res.channel, res.channel_name_enbale, res.channel_name_text);
            avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_OSD_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetOSDResp));
        }
        break;

        case IOTYPE_USER_IPCAM_SET_OSD_REQ:{
            PRINT_INFO("cmd: IOTYPE_USER_IPCAM_SET_OSD_REQ\n");

            /* 获取 */
            SMsgAVIoctrlGetOSDResp* p = (SMsgAVIoctrlGetOSDResp*)buf;
            PRINT_INFO("set osd, channel:%d, name:%s\n", p->channel, p->channel_name_text);

            /* 处理 */
            #if 1
            GK_NET_CHANNEL_INFO channelInfo = {0};
            int i = 0;
            int num = netcam_video_get_channel_number();
            for(i = 0; i < num; i++) {
                runChannelCfg.channelInfo[i].osdChannelName.enable = 1;
                strncpy(runChannelCfg.channelInfo[i].osdChannelName.text, p->channel_name_text, sizeof(runChannelCfg.channelInfo[i].osdChannelName.text));
                strncpy(runVideoCfg.vencStream[i].h264Conf.name, p->channel_name_text, sizeof(runVideoCfg.vencStream[i].h264Conf.name));
            }
            netcam_osd_update_title();
            //netcam_timer_add_task(netcam_osd_pm_save, NETCAM_TIMER_TWO_SEC, SDK_FALSE, SDK_TRUE);   // 8*250ms = 2s
            ChannelCfgSave();
            VideoCfgSave();
            #endif
            /* 回复*/
            SMsgAVIoctrlSetOSDResp res = {0};
            res.result = 0;
            avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_OSD_RESP, (char *)&res, sizeof(SMsgAVIoctrlSetOSDResp));
        }
        break;
		#ifdef MODULE_SUPPORT_RF433
		case IOTYPE_USER_IPCAM_SET_433_REQ:
		{
			SMsgAVIoctrlSet433Req *p = (SMsgAVIoctrlSet433Req *)buf;
			SMsgAVIoctrlSet433Resp resp = {-1,0};
			PRINT_INFO("modify 433 code,id=%d;name=%s\r\n",p->id,p->name);
			resp.result = rf433_modify_code_name(p->id,p->name);
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_433_RESP, (char *)&resp, sizeof(SMsgAVIoctrlSet433Resp));
		}
		break;
		case IOTYPE_USER_IPCAM_GET_433_REQ:
		{
			SMsgAVIoctrlGet433Req *p = (SMsgAVIoctrlGet433Req *)buf;
			SMsgAVIoctrlGet433Resp resp = {0};
			resp.num = rf433_get_dev_num();
			PRINT_INFO("433 code num=%d\r\n",resp.num);
			int ii;
			for(ii = 0;ii < resp.num;ii++)
			{
				rf433_get_dev_corresponding_info(ii,&(resp.dev[ii].id),&(resp.dev[ii].type),resp.dev[ii].name);
				PRINT_INFO("seq:%d,id:%d,type:%d,name:%s",ii,resp.dev[ii].id,resp.dev[ii].type,resp.dev[ii].name);
			}
			
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_433_RESP, (char *)&resp,sizeof(S433Dev)*resp.num + 4);
		}
		break;
		case IOTYPE_USER_IPCAM_CFG_433_REQ:
		{
			SMsgAVIoctrlCfg433Req *p = (SMsgAVIoctrlCfg433Req *)buf;
			SMsgAVIoctrlCfg433Resp resp = {-1,-1};
			PRINT_INFO("learn code;name=%s\r\n",p->name);
			if(p->name)
			{
				resp.result = AVIOCTRL_CFG_433_WAITING;
				rf433_learn_code(p->type,p->name);
			}
			else
			{
				resp.result = AVIOCTRL_CFG_433_ERROR;
			}
			rf433_learn_resp.none = avIndex;
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_CFG_433_RESP, (char *)&resp, sizeof(SMsgAVIoctrlCfg433Resp));
		}
		break;
		case IOTYPE_USER_IPCAM_DEL_433_REQ:
		{
			int ii;
			SMsgAVIoctrlDel433Req *p = (SMsgAVIoctrlDel433Req *)buf;
			SMsgAVIoctrlDel433Resp resp = {-1,-1};
			resp.result = 0;
			PRINT_INFO("delete code;id=%d\r\n",p->id);
			rf433_delete_code(p->id);
			for (ii = 0; ii < MAX_CLIENT_NUMBER; ii++)
			{
				if (gClientInfo[ii].avIndex < 0)	// 设备不在线
				continue;
				avSendIOCtrl(gClientInfo[ii].avIndex, IOTYPE_USER_IPCAM_DEL_433_RESP, (char *)&resp, sizeof(SMsgAVIoctrlDel433Resp));
			}
		}
		break;
		case IOTYPE_USER_IPCAM_CFG_433_EXIT_REQ:
		{
			SMsgAVIoctrlCfg433ExitReq *p =(SMsgAVIoctrlCfg433ExitReq *)buf;
			SMsgAVIoctrlCfg433ExitResp resp = {-1,-1};
			resp.result = 0;
			PRINT_INFO("exit learn code\r\n");
			rf433_exit_learn();
			avSendIOCtrl(avIndex,IOTYPE_USER_IPCAM_CFG_433_EXIT_RESP, (char *)&resp, sizeof(SMsgAVIoctrlCfg433ExitResp));
		}
		break;
		#endif


        //获取基本的参数,如厂商号，音频格式，视频格式等
#ifndef USE_APP_XUHUI
        case IOTYPE_USER_IPCAM_GET_CAPACITY_REQ:
        {
			SMsgAVIoctrlGetCapacityResp res;
            PRINT_INFO(" IOTYPE_USER_IPCAM_GET_CAPACITY_REQ\n");
            memset(&res,0,sizeof(SMsgAVIoctrlGetCapacityResp));

            char version_tmp[32] = {0};
            int i = last_index_at(runSystemCfg.deviceInfo.upgradeVersion, '.');
            strcpy(version_tmp, runSystemCfg.deviceInfo.upgradeVersion + i + 1);
            PRINT_INFO(" version:%s\n", version_tmp);

            struct device_info_mtd info;
            int ret = load_info_to_mtd_reserve(MTD_TUTK_P2P, &info, sizeof(info));
            if((ret < 0) || (info.device_id[0] == 0))
            {
                PRINT_ERR("load flash failed\n");
            }
            else 
            {
                #if 0
                res.devType = tutkGetDeviceType(info.device_type);
                res.netFamily = tutkGetP2PType(info.p2p_type); 
                res.odmID = tutkGetOdmType(info.odm_id);   
                res.language = tutkGetLanguageType(info.language);
                res.panoramaMode = tutkGetPanoramaType(info.panorama_mode);
                #endif
                //res.devType = info.devType;
                res.devType = runSystemCfg.deviceInfo.devType;
                res.netFamily = info.netFamily;
                //res.odmID = info.odmID;                
                res.odmID = runSystemCfg.deviceInfo.odmId;
                res.language = info.language;
                res.panoramaMode = info.panoramaMode;

                
                odm_id = res.odmID;
                strncpy(model,info.model,sizeof(info.model));
                
                strncpy(res.model,runSystemCfg.deviceInfo.model,sizeof(info.model));
                //strncpy(res.model,info.model,sizeof(info.model));
                memcpy(res.my_key,info.my_key,sizeof(res.my_key));

            }

            res.odmID = runSystemCfg.deviceInfo.odmId;
            strncpy(res.model,runSystemCfg.deviceInfo.model,sizeof(info.model));
            res.devType = runSystemCfg.deviceInfo.devType;//0-卡片机, 1-摇头机, 2-鱼眼摄像机.,3,门铃单向，4 门铃双向，5模组

            //res.devType = tutkGetDeviceType();// 设备类型: 0-卡片机, 1-摇头机, 2-鱼眼摄像机...

            //res.netFamily = tutkGetP2PType();          // 网络提供商: 1-迈特威视, 2-tutk...
            res.serverID =0;           // 服务器ID: 保留,默认为0
            strncpy(res.version,version_tmp,sizeof(res.version));        
            //res.odmID = tutkGetOdmType();              // odm商ID: 0-东舜自己, 01-TCL, 02-康佳
            //res.language = tutkGetLanguageType();
            // 硬件ID号
            strncpy(res.hid,runSystemCfg.deviceInfo.serialNumber,sizeof(runSystemCfg.deviceInfo.serialNumber));
            
            // 视频编码: 采用小写, 如: "h264", "h265"          
            //strncpy(res.videoCodec,"h264",sizeof(res.videoCodec));
 
            // 音频编码: 采用小写, 如: "aac", "mp3", "711a", "711u", "pcm"
            //strncpy(res.audioCodec,"G711a",sizeof(res.audioCodec));

            //res.audioSampleRate = 8000;    // 音频采样: 8000,16000 
            //res.audioChannelNum = 1;    // 音频通道: 1-单通, 2-双通道   
            //res.audioBitsPerSample = 16; // 音频bits: 8, 16

            res.manufacture = 0x676b647a;
            res.suportChannelNum = 1;
            res.suportPTZ = sdk_cfg.ptz.enable;          // 支持ptz控制: 支持为1,不支持为0

            char audioIn=0,audioOut=0;
            if(runAudioCfg.mode == 1 )
            {
                audioIn =1;
            }
            else
            if(runAudioCfg.mode == 2)
            {
                audioIn = 1;
                audioOut = 1;
            }
            res.suportAudioIn = audioIn;      // 支持音频采集: 支持为1,不支持为0
            res.suportAudioOut = audioOut;     // 支持声音播放: 支持为1,不支持为0  
            #ifdef MODULE_SUPPORT_LOCAL_REC
            res.suportStorage = 1;      // 支持存储: 支持为1,不支持为0          

            #endif
            //res.panoramaMode = 0;       // 全景安装模式: 0-吸顶式,1-壁挂式 

            PRINT_INFO(" capacity devtype:%d,odm:%d,p2p:%d,lang:%d,len:%d,audioin:%d,audioout:%d,manufacture:%x,model:%s\n",
            res.devType,res.odmID,res.netFamily,res.language,sizeof(res),res.suportAudioIn,res.suportAudioOut,res.manufacture,res.model);
            PRINT_INFO(" %d,%d,%d,%d,%d\n",res.my_key[0],res.my_key[1],res.my_key[2],res.my_key[3],res.my_key[4]);

			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_CAPACITY_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetCapacityResp));            

            break;
        }
     
#else

	case IOTYPE_USER_IPCAM_GET_CAPACITY_REQ:
		{
			 if(!cmd_len)
			 {
				SMsgAVIoctrlGetCapacityResp resp;
				PRINT_INFO(" IOTYPE_USER_IPCAM_GET_CAPACITY_REQ\n");
				memset(&resp,0,sizeof(SMsgAVIoctrlGetCapacityResp));
		
				char version_tmp[32] = {0};
				int i = last_index_at(runSystemCfg.deviceInfo.upgradeVersion, '.');
				strcpy(version_tmp, runSystemCfg.deviceInfo.upgradeVersion + i + 1);
				PRINT_INFO(" version:%s\n", version_tmp);

				resp.devType	 = runSystemCfg.deviceInfo.devType;
				resp.netFamily	 = 0;
				resp.odmID		 = runSystemCfg.deviceInfo.odmId = 0;
				resp.language	 = 0;//runSystemCfg.deviceInfo.languageType;//info.language;
				resp.suportChannelNum = 1;
	
				//memcpy(&resp.manufacture ,runSystemCfg.deviceInfo.manufacturer,sizeof(resp.manufacture));
				 resp.manufacture = 0x64736c7a;//0x64736c79;
				strncpy(resp.model,(char*)runSystemCfg.deviceInfo.model,sizeof(resp.model));

				resp.serverID =0;		   // 服务器ID: 保留,默认为0
				strncpy(resp.version,version_tmp,sizeof(resp.version));
				// 硬件ID号
				strncpy(resp.hid,runSystemCfg.deviceInfo.serialNumber,sizeof(runSystemCfg.deviceInfo.serialNumber));
		
				resp.suportPTZ = 1; 		 // 支持ptz控制: 支持为1,不支持为0
				resp.suportAudioIn = 1;   // 支持音频采集: 支持为1,不支持为0
				resp.suportAudioOut = 1;   // 支持声音播放: 支持为1,不支持为0
				resp.suportStorage = 1; 	// 支持存储: 支持为1,不支持为0
				resp.supportLight  = 1;
		
				PRINT_INFO(" capacity devtype:%d,odm:%d,p2p:%d,lang:%d\n",resp.devType,resp.odmID,resp.netFamily,resp.language);
		
				avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_CAPACITY_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetCapacityResp));
		
				break;
			 }
			 else
			 {
				 
				 SMsgAVIoctrlGetCapacityResp resp;
				 SMsgAVIoctrlGetCapcityReq* p = (SMsgAVIoctrlGetCapcityReq*)buf;
				 
				 PRINT_INFO("IOTYPE_USER_IPCAM_GET_CAPACITY_REQ, user_sign: %s,isWaveConfig:%d\n",p->user_sign,p->isWaveConfig);
				if(!strlen(runP2PCfg.user_sign))
				{
					//if((netcam_net_wifi_getWorkMode() == NETCAM_WIFI_AP) || p->isWaveConfig)
					{
						strncpy(runP2PCfg.user_sign,p->user_sign,sizeof(runP2PCfg.user_sign));
						P2PCfgSave();
					}
				}

				 memset(&resp,0,sizeof(SMsgAVIoctrlGetCapacityResp));
				 
				 char version_tmp[32] = {0};
				 int i = last_index_at(runSystemCfg.deviceInfo.upgradeVersion, '.');
				 strcpy(version_tmp, runSystemCfg.deviceInfo.upgradeVersion + i + 1);
		
				 resp.devType	  = runSystemCfg.deviceInfo.devType;
				 resp.netFamily   = 0;
				 resp.odmID 	  = runSystemCfg.deviceInfo.odmId = 0;
				 resp.language	  = 0;//runSystemCfg.deviceInfo.languageType;//info.language;
				 resp.suportChannelNum = 1;
				 
				 //memcpy(&resp.manufacture ,runSystemCfg.deviceInfo.manufacturer,sizeof(resp.manufacture));
				 resp.manufacture = 0x64736c7a;//0x64736c79;//double vi device
				 strncpy(resp.model,(char*)runSystemCfg.deviceInfo.model,sizeof(resp.model));
			 
				 resp.serverID =0;			// 服务器ID: 保留,默认为0
				 strncpy(resp.version,version_tmp,sizeof(resp.version));
				 // 硬件ID号
				 strncpy(resp.hid,runSystemCfg.deviceInfo.serialNumber,sizeof(runSystemCfg.deviceInfo.serialNumber));
				 
				 resp.suportPTZ = 1;		  // 支持ptz控制: 支持为1,不支持为0
				 resp.suportAudioIn = 1;   // 支持音频采集: 支持为1,不支持为0
				 resp.suportAudioOut = 1;		// 支持声音播放: 支持为1,不支持为0	 
				 resp.suportStorage = 1;	 // 支持存储: 支持为1,不支持为0
				 resp.supportLight	= 1;

				if(!strcmp(runP2PCfg.user_sign,p->user_sign))
				{
					resp.is_owner = 1;

					PRINT_INFO("%s is owner!\n",p->user_sign);
				}
				else
				{
					resp.is_owner = 0;
					PRINT_INFO("%s is not owner! owner is %s\n",p->user_sign,runP2PCfg.user_sign);
				}				 
				 avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_CAPACITY_RESP, (char *)&resp, sizeof(SMsgAVIoctrlGetCapacityResp));
				 
				 break;
			 }
		}
#endif
        
        //获取音频的输入输出开关状态
        case IOTYPE_USER_IPCAM_GET_AUDIO_REQ:
        {
			SMsgAVIoctrlAudioDevice resp = {0} ;
            			
			resp.enableIn = runP2PCfg.audioInEnable;
			resp.enableOut = runP2PCfg.audioOutEnable;

			PRINT_INFO(" IOTYPE_USER_IPCAM_GET_AUDIO_REQ inEn:%u,outEn:%u\n",resp.enableIn,resp.enableOut);
			avSendIOCtrl(avIndex,IOTYPE_USER_IPCAM_GET_AUDIO_RESP, (char *)&resp, sizeof(SMsgAVIoctrlAudioDevice));

            break;
        }

        //设置音频的输入输出开关
        case IOTYPE_USER_IPCAM_SET_AUDIO_REQ:
        {

            
			SMsgAVIoctrlAudioDevice *p = (SMsgAVIoctrlAudioDevice *)buf;
			SMsgAVIoctrlAudioDevice resp = {0} ;

			if(p->enableIn)
			{
				gadi_audio_ai_enable();
#ifdef AUDIO_ECHO_CANCELLATION_SUPPORT
				gadi_audio_ai_aec_enable();
#endif
			}
			else
			{
				gadi_audio_ai_disable();
#ifdef AUDIO_ECHO_CANCELLATION_SUPPORT
				gadi_audio_ai_aec_disable();
#endif	
			}

			if(p->enableOut)
				audio_spk_en_control(1);
            else
				audio_spk_en_control(0);

			runP2PCfg.audioInEnable = p->enableIn;
			runP2PCfg.audioOutEnable = p->enableOut;

			P2PCfgSave();
			
			memcpy(&resp,p,sizeof(SMsgAVIoctrlAudioDevice));
			PRINT_INFO(" SET_AUDIO_REQ inEn:%u,outEn:%u\n",p->enableIn,p->enableOut);
			avSendIOCtrl(avIndex,IOTYPE_USER_IPCAM_SET_AUDIO_RESP, (char *)&resp, sizeof(SMsgAVIoctrlAudioDevice));

            break;
        }
        
        //东舜，获取移动侦测联动报警
        case IOTYPE_USER_IPCAM_GET_MD_ALAM_REQ:
        {
#if 0
            int sensi;

            SMsgAVIoctrlMDAlarmResp resp = {0} ;
            netcam_md_get_sensitivity(0, &sensi);
            resp.enable = runMdCfg.enable;
            resp.level = sensi;
            resp.frequency = runMdCfg.handle.intervalTime;
            resp.trigAudioOut =  runMdCfg.handle.is_alarmout;
            resp.trigRecord = runMdCfg.handle.is_rec;
            resp.trigMsgPush = runMdCfg.handle.is_pushMsg ;

            resp.startHour = runMdCfg.scheduleTime[0][0].startHour;
            resp.startMins = runMdCfg.scheduleTime[0][0].startMin;
            resp.closeHour = runMdCfg.scheduleTime[0][0].stopHour;
            resp.closeMins = runMdCfg.scheduleTime[0][0].stopMin;
#else
		int sensi;
		int retSensi = 0;
		PRINT_INFO("IOTYPE_USER_IPCAM_GET_MD_ALAM_REQ");
		unsigned char sensorNo = gClientInfo[SID].bSensorNo;

		SMsgAVIoctrlMDAlarmResp resp = {0} ;

		sensi = runMdCfg.sensitive;
		if(sensi < 25)
		{
			retSensi = 0;
		}
		else if(sensi < 50)
		{
			retSensi = 1;
		}
		else if(sensi < 75)
		{
			retSensi = 2;
		}
		else if(sensi < 100)
		{
			retSensi = 3;
		}
		else
		{
			retSensi = 4;
		}

		resp.trigMsgPush = runP2PCfg.msgPush[sensorNo];

		resp.enable = runMdCfg.enable;

		resp.level = runMdCfg.sensitive/25;//retSensi;
		resp.frequency = runMdCfg.handle.intervalTime;
		resp.trigAudioOut =  runMdCfg.handle.is_alarmout;
		resp.trigRecord = runMdCfg.handle.is_rec;


		if(runMdCfg.scheduleTime[0][0].startHour == runMdCfg.scheduleTime[0][1].startHour &&
			runMdCfg.scheduleTime[0][0].startMin == runMdCfg.scheduleTime[0][1].startMin)
		{
			resp.startHour = runMdCfg.scheduleTime[0][0].startHour;
			resp.startMins = runMdCfg.scheduleTime[0][0].startMin;
			resp.closeHour = runMdCfg.scheduleTime[0][0].stopHour;
			resp.closeMins = runMdCfg.scheduleTime[0][0].stopMin;
		}
		else
		{
			resp.startHour = runMdCfg.scheduleTime[0][1].startHour;
			resp.startMins = runMdCfg.scheduleTime[0][1].startMin;
			resp.closeHour = runMdCfg.scheduleTime[0][0].stopHour;
			resp.closeMins = runMdCfg.scheduleTime[0][0].stopMin;
		}

#endif
            PRINT_INFO(" GET_MD_ALAM enable:%d,level:%d,trigout:%d,intertime:%d,pushMsg:%d\n",resp.enable,resp.level,resp.trigAudioOut,resp.frequency,resp.trigMsgPush);
            avSendIOCtrl(avIndex,IOTYPE_USER_IPCAM_GET_MD_ALAM_RESP, (char *)&resp, sizeof(SMsgAVIoctrlMDAlarmResp));

            break;
        }
        //设置移动侦测报警联动参数
        case IOTYPE_USER_IPCAM_SET_MD_ALAM_REQ:
        {

          
            SMsgAVIoctrlMDAlarmReq *p = (SMsgAVIoctrlMDAlarmReq *)buf;
            SMsgAVIoctrlMDAlarmResp resp = {0} ;
            memcpy(&resp,p,sizeof(SMsgAVIoctrlMDAlarmResp));

#if 0
            runMdCfg.enable = p->enable;
            netcam_md_set_sensitivity(0,p->level);
            runMdCfg.handle.intervalTime = p->frequency ;
            runMdCfg.handle.is_alarmout = p->trigAudioOut ;
            runMdCfg.handle.is_beep =p->trigAudioOut ;
            
            runMdCfg.handle.is_rec = p->trigRecord ;
            runMdCfg.handle.recStreamNo = 1;   
            runMdCfg.handle.recTime = 10;
            runMdCfg.handle.is_pushMsg = p->trigMsgPush ;
            int i = 0, j = 0;
            for (i = 0; i < 7; i++) 
            {
                for (j = 0; j < 4; j++) 
                {
                    runMdCfg.scheduleTime[i][j].startHour = p->startHour;
                    runMdCfg.scheduleTime[i][j].startMin  = p->startMins;
                    runMdCfg.scheduleTime[i][j].stopHour  = p->closeHour;
                    runMdCfg.scheduleTime[i][j].stopMin   = p->closeMins;
                }
            }
#else

			unsigned char sensorNo = gClientInfo[SID].bSensorNo;

			PRINT_INFO("IOTYPE_USER_IPCAM_SET_MD_ALAM_REQ");

			runMdCfg.enable = p->enable;
			runP2PCfg.msgPush[sensorNo] = p->trigMsgPush;

			netcam_md_set_sensitivity(0,p->level);

			runMdCfg.handle.intervalTime = p->frequency ;
			runMdCfg.handle.is_alarmout = p->trigAudioOut ;
			runMdCfg.handle.is_beep =p->trigAudioOut ;
			runMdCfg.handle.is_rec = p->trigRecord ;
			runMdCfg.handle.recStreamNo = 1;
			runMdCfg.handle.recTime = 1;
			runMdCfg.sensitive = p->level*25;

			int i = 0,j = 0;
			for (i = 0; i < 7; i ++)
			{
				if((p->startHour > p->closeHour) || ((p->startHour == p->closeHour) && p->startMins > p->closeMins))
				{
					runMdCfg.scheduleTime[i][0].startHour = 0;
					runMdCfg.scheduleTime[i][0].startMin = 0;
					runMdCfg.scheduleTime[i][0].stopHour = p->closeHour;
					runMdCfg.scheduleTime[i][0].stopMin = p->closeMins;

					runMdCfg.scheduleTime[i][1].startHour = p->startHour;
					runMdCfg.scheduleTime[i][1].startMin = p->startMins;
					runMdCfg.scheduleTime[i][1].stopHour = 23;
					runMdCfg.scheduleTime[i][1].stopMin = 59;

					runMdCfg.scheduleTime[i][2].startHour = 0;
					runMdCfg.scheduleTime[i][2].startMin = 0;
					runMdCfg.scheduleTime[i][2].stopHour = 0;
					runMdCfg.scheduleTime[i][2].stopMin = 0;

					runMdCfg.scheduleTime[i][3].startHour = 0;
					runMdCfg.scheduleTime[i][3].startMin = 0;
					runMdCfg.scheduleTime[i][3].stopHour = 0;
					runMdCfg.scheduleTime[i][3].stopMin = 0;

				}
				else
				{
					for (j = 0; j < 4; j ++)
					{
						runMdCfg.scheduleTime[i][j].startHour = p->startHour;
						runMdCfg.scheduleTime[i][j].startMin = p->startMins;
						runMdCfg.scheduleTime[i][j].stopHour = p->closeHour;
						runMdCfg.scheduleTime[i][j].stopMin = p->closeMins;
					}
				}

			}
#endif
            MdCfgSave();
            
            PRINT_INFO(" SET_MD_ALAM enable:%d,level:%d,trigout:%d,intertime:%d,pushMsg:%d\n",p->enable,p->level,p->trigAudioOut,p->frequency,p->trigMsgPush);
            avSendIOCtrl(avIndex,IOTYPE_USER_IPCAM_SET_MD_ALAM_RESP, (char *)&resp, sizeof(SMsgAVIoctrlMDAlarmResp));

            break;
        }

        #if 1
        //固件升级命令
        case IOTYPE_USER_IPCAM_UPDATE_REQ:
        {
#if 0          

            SMsgAVIoctrlUpdateReq *p = (SMsgAVIoctrlUpdateReq *)buf;
            SMsgAVIoctrlUpdateReq resp={0};
            char *purl = NULL;
            unsigned char *recv_md5=NULL;
            unsigned char mtws_md5_arr[16];
            int ret=0;
            char *ver_str = NULL;
            char *updateFile="/tmp/update.bin";
            char *fileData = NULL;


            //需要从flash中获取model和odm id
            struct device_info_mtd info;
            ret = load_info_to_mtd_reserve(MTD_TUTK_P2P, &info, sizeof(info));
            if((ret < 0) || (info.device_id[0] == 0))
            {
                PRINT_ERR("load flash failed\n");
            }
            else 
            {

            }
            //因为升级过程网络端口，升级后也无法发送消息给app，所以在收到命令后就
            //发送消息给app，告诉对方正在升级
            #if USE_RL_PROTOCOL
            memcpy(&resp,p,sizeof(SMsgAVIoctrlUpdateReq));            
            avSendIOCtrl(avIndex,IOTYPE_USER_IPCAM_UPDATE_RESP, (char *)&resp, sizeof(SMsgAVIoctrlUpdateReq));
            #endif


            
            //cJSON *urlValueJson = NULL;    
            //自动模式0，
            //非自动模式1，从指定地址获取
            if(p->updateType == 1)
            {
                purl = p->urlAddr;
                PRINT_INFO("update from url : %s\n",purl);
            }
            else
            {
                cJSON *updateJson, *verJson, *urlJson,*md5Json;
                char cmd[500];
        
                //char *tem_purl = "http://ts.vs98.com:8080/api/v1/update?devid=2903987&cloudid=2903987&curver=1012&model=1&odm=1";
                char tem_purl[500];
    
                unsigned long long now_n_ver = 0;
                tutk_get_ver(runSystemCfg.deviceInfo.upgradeVersion, &now_n_ver);
                printf("now -- s_ver:%s n_ver:%lld\n", runSystemCfg.deviceInfo.upgradeVersion, now_n_ver);
                local_now_ver = now_n_ver;

                

                #if 0
    
                snprintf(tem_purl,500,"http://ts.vs98.com:8080/api/v1/update?devid=%s&cloudid=%s&curver=%lld&model=%s&odm=%d"  
                ,runSystemCfg.deviceInfo.serialNumber,
                runSystemCfg.deviceInfo.serialNumber,
                now_n_ver,
                info.model,
                info.odmID);
                printf("tem_purl:%s\n",tem_purl);
                #else
                
                //http://lpwei.com/update/update.php?odm=10&model=DoorSM8801_v1
                snprintf(tem_purl,500,"http://lpwei.com/update/update.php?odm=%d&model=%s",  
                runSystemCfg.deviceInfo.odmId,
                runSystemCfg.deviceInfo.model 
                );
                printf("tem_purl:%s\n",tem_purl);
                #endif
                


                
                snprintf(cmd,500,"wget '%s' -O /tmp/update.json"  
                ,tem_purl);
    
                
                printf("system call: %s\n",cmd);         
                ret = system(cmd);
                printf("wget json cmd ret :%d\n",ret);
    
                
                updateJson=cJSON_ReadFile("/tmp/update.json");//从文件系统中读取 JSON 项目
                if(updateJson != NULL)
                {
                
                    char * tmpPri = cJSON_Print(updateJson);
                    printf("update  JSON pri:%s\n",tmpPri);
    
                    /* 获取升级文件的版本号 */
                    verJson = cJSON_GetObjectItem(updateJson, "ver");
                    if (verJson == NULL)
                    {
                        printf("verJson null \n\n");
                        return -1;
                        //ret = UPDATE_JSON_ERR;
                        //goto err0;
                    }
                    else
                    {
                        ver_str = verJson->valuestring;
                    }
    
                    /* 判断升级文件的版本号新旧来决定是否升级 */
                    char upgrade_s_ver[MAX_SYSTEM_STR_SIZE] = {0};
                    snprintf(upgrade_s_ver, "%s", ver_str);
                    unsigned long long upgrade_n_ver = 0;
                    tutk_get_ver(upgrade_s_ver, &upgrade_n_ver);
                    printf("upgrade -- s_ver:%s n_ver:%lld\n", upgrade_s_ver, upgrade_n_ver);
#if 0
                    unsigned long long now_n_ver = 0;
                    mtws_get_ver(runSystemCfg.deviceInfo.upgradeVersion, &now_n_ver);
                    printf("now -- s_ver:%s n_ver:%lld\n", runSystemCfg.deviceInfo.upgradeVersion, now_n_ver);
#endif
                    if (now_n_ver >= upgrade_n_ver)
                    {
                        PRINT_INFO("no need upgrade, now is the newest version.");
                        break;
                    }
                    else
                    {
                        PRINT_INFO("to upgrade: the newest version is %lld, local version is %lld.", upgrade_n_ver, now_n_ver);
                    }
                    
                    
                    urlJson = cJSON_GetObjectItem(updateJson, "url");
                    if(urlJson != NULL)
                    {
                        purl = urlJson->valuestring;
                    }
                    else
                    {      
                        printf("urlJson null \n\n");
                        break;
                        //ret = UPDATE_JSON_ERR;
                        //goto err0;
                        
                    }
    
                    md5Json = cJSON_GetObjectItem(updateJson, "md5");
                    if(urlJson != NULL)
                    {
                        recv_md5 = md5Json->valuestring;
                        printf("recv md5 %s\n",recv_md5);
                    }
                    else
                    {      
                        printf("md5 null \n\n"); 
                        break;
                        //ret = UPDATE_JSON_ERR;
                        //goto err0;
                    }
                }
                else
                {      
                    printf("updateJson /tmp/update.json not exit \n\n");
                    break;
                    //ret = UPDATE_JSON_ERR;
                    //goto err0;
                }    
                printf("update from default url : %s\n",purl);         
                /*to do ??????????*/              
            }
            char cmd[500];
            long fileSize = 0;
            long fileLen = 0;
            long offset = 0;
            int cnt;
            //wget http://ts.vs98.com:8080/firmware/gk7108v1_fw.bin
    
            /*下载文件到本地*/
            snprintf(cmd,500,"wget '%s' -O %s",  
            purl,
            updateFile);
            
            printf("system call: %s\n",cmd);         
            ret = system(cmd);       
            printf("update ret :%d\n");



            #if 1
            /**********
            1 ,释放系统资源
            ************/
            
            netcam_update_relase_system_resource();
            netcam_video_exit();
            #if 0 
            sdk_video_enc_stop();
            sdk_audio_enc_stop();
            #endif
            //tutklib_exit();
            /**********
            2,文件读取
            **************/
            FILE *fp = NULL;
            fp = fopen(updateFile, "rb");
            if (fp == NULL) 
            {
                PRINT_ERR("fopen error %s not exist!", updateFile);
                break;
                //ret = UPDATE_FILE_ERR;
                //goto err;
            }
            //获取文件大小
            if (0 != fseek(fp, 0, SEEK_END)) 
            {
                
                PRINT_ERR("fseek failed!");
                fclose(fp);
                //ret = UPDATE_FILE_ERR;
                //goto err;
            }
            fileSize = ftell(fp);
            printf("update fileSize:%d\n",fileSize);
            fileData = malloc(fileSize);
            if(!fileData) 
            {
                PRINT_ERR();
                fclose(fp);
                break;
                //ret = UPDATE_MEM_ERR;
                //goto err;
            }
            memset(fileData, 0, fileSize);
            
            if(0 != fseek(fp, 0, SEEK_SET)) 
            {
                PRINT_ERR();
                free(fileData);
                fileData = NULL;
                fclose(fp);
                break;
                //ret = UPDATE_FILE_ERR;
                //goto err;
            }
            
            //读取升级文件
            fileLen = fileSize;
            while (fileLen > 0) 
            {
                ret = fread(fileData + offset, 1, fileLen, fp);
                if (ret >= 0)
                {
                    offset += ret;
                    fileLen -= ret;
                }
                else
                {
                    PRINT_INFO("offset:%d  len:%d  ret:%d\n", offset, fileLen, ret);
                    //break;
                }
                
                //PRINT_INFO("offset:%d  len:%d  ret:%d\n", offset, fileLen, ret);
            }
            fclose(fp);

            //ret = system("rm -f /tmp/update.bin");       
            
            ret = new_system_call("rm -f /tmp/update.bin");//force REBOOT
            
            PRINT_INFO("rm /tmp/update.bin ret =%d\n",ret);
            sleep(2);
            /**********

            3,md5校验
            *************/
            if(recv_md5 != NULL)
            {   
                int ii;
                //根据升级文件得到md5校验码
                tutk_check(fileData,fileSize,mtws_md5_arr);
    #if 0
                printf("\n check md5 :\n"); 
                for(ii = 0;ii < 16;ii++)
                {
                    printf("%2x",mtws_md5_arr[ii]);
                }
                printf("\n recv md5 :%s\n",recv_md5);
    #endif
                
                char md5Str[32]={'\0'};
                snprintf(md5Str,500,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",  
                mtws_md5_arr[0],mtws_md5_arr[1],mtws_md5_arr[2],mtws_md5_arr[3],
                mtws_md5_arr[4],mtws_md5_arr[5],mtws_md5_arr[6],mtws_md5_arr[7],
                mtws_md5_arr[8],mtws_md5_arr[9],mtws_md5_arr[10],mtws_md5_arr[11],
                mtws_md5_arr[12],mtws_md5_arr[13],mtws_md5_arr[14],mtws_md5_arr[15]
                );
                printf("final md5str:%s\n",md5Str);
                /*比较计算得到的md5值(md5Str)，和接收到的md5值(recv_md5)，不相同则说明文件下载的不正确*/
                if(strcmp(md5Str,recv_md5) != 0)
                {
                    printf("md5 check failed,download file have error\n");
                    //ret = UPDATE_MD5_ERR;
                    free(fileData);
                    fileData = NULL;
                    break;
                    //goto err;
                    
                }
            }
            /**********
            4,开始升级
            ************/
            //ret = upgrade_flash_by_file(updateFile);
            ret = netcam_update(fileData, fileSize, NULL);
            if (ret < 0)
            {
                PRINT_ERR("Check updating package, failed: %d \n", ret);
                free(fileData);
                fileData = NULL;
                //ret = UPDATE_UP_ERR;
                //goto err;
                
                new_system_call("reboot -f");//force REBOOT
                break;
            }
            
            
            cnt = 0;
            while (cnt < 99)
            {
                cnt = netcam_update_get_process();
                fileLen = (fileSize * cnt)/100/2;
                usleep(218 * 1000);
                PRINT_INFO("-----> len:%d  cnt:%d  ret:%d \n", fileLen, cnt, ret);
            }
            
            //usleep(100 * 1000);
            if(fileData != NULL)
            {
                free(fileData);
                fileData = NULL;
            }
            sleep(2);
            PRINT_INFO("-----> update success ,reboot now! \n");
            new_system_call("reboot -f");//force REBOOT
            //system("reboot");
        
            #endif

            
            PRINT_INFO(" update \n");
            memcpy(&resp,p,sizeof(SMsgAVIoctrlUpdateReq));
            avSendIOCtrl(avIndex,IOTYPE_USER_IPCAM_UPDATE_RESP, (char *)&resp, sizeof(SMsgAVIoctrlUpdateReq));
#else
			SMsgAVIoctrlUpdateProgressResp resp;

			char status = tutk_update_get_status();
			PRINT_INFO("IOTYPE_USER_IPCAM_UPDATE_REQ: status:%d\n",status);

			memset(&resp,0,sizeof(SMsgAVIoctrlUpdateProgressResp));

			if(status == 0)
			{
				TUTK_UPDATE_ParT *arg;
				tutk_update_set_status(1);
				arg = malloc(sizeof(TUTK_UPDATE_ParT));
				if(arg )
				{
					memcpy(&arg->req,buf,sizeof(SMsgAVIoctrlUpdateReq));
					arg->avindex = avIndex;
					netcam_update_set_down_load_cb((cbFunc)tutk_update,arg); 

					resp.progress = 0;
					resp.status = 0;
					PRINT_INFO("IOTYPE_USER_IPCAM_UPDATE_RESP\n");

					avSendIOCtrl(avIndex,IOTYPE_USER_IPCAM_UPDATE_RESP, (char *)&resp, sizeof(SMsgAVIoctrlUpdateProgressResp));
				}
			}
#endif
            break;
        }
        #endif
        //固件升级命令进度查询
        case IOTYPE_USER_IPCAM_UPDATE_PROG_REQ:
        {

            SMsgAVIoctrlUpdateProgressReq *p = (SMsgAVIoctrlUpdateProgressReq *)buf;
            SMsgAVIoctrlUpdateProgressResp resp={0};
            int cnt = 0;

            char tem_ver[64];
            snprintf(resp.version,16,"%lld",local_now_ver);
            
            cnt = netcam_update_get_process();
            resp.progress = cnt;
            resp.status = 0;
            usleep(218 * 1000);
            PRINT_INFO("-----> update cnt:%d  <----- \n", cnt);

            
            PRINT_INFO(" update progress\n");
            avSendIOCtrl(avIndex,IOTYPE_USER_IPCAM_UPDATE_PROG_RESP, (char *)&resp, sizeof(SMsgAVIoctrlUpdateProgressResp));

            break;
        }

        //shell命令
        case IOTYPE_USER_IPCAM_SHELL_REQ:
        {

          
            SMsgAVIoctrlShellReq *p = (SMsgAVIoctrlShellReq *)buf;
            SMsgAVIoctrlShellResp resp={0};
            memset(&resp,0,sizeof(SMsgAVIoctrlShellResp));
            
            char *pshellString = p->value;
            char cret[2000];
            int ret=0;
            tutk_getStringBySystem(pshellString,cret,sizeof(cret),&ret);

            resp.result = ret;
            strncpy(resp.msg,cret,sizeof(resp.msg));
            
            PRINT_INFO(" shell cmd :%s\n",p->value);
            avSendIOCtrl(avIndex,IOTYPE_USER_IPCAM_SHELL_RESP, (char *)&resp, sizeof(SMsgAVIoctrlShellResp));

            break;
        }
        //心跳监测
        case IOTYPE_USER_IPCAM_HEARTBEAT_REQ:
        {

          
            SMsgAVIoctrlHeartbeatReq *p = (SMsgAVIoctrlHeartbeatReq *)buf;
            SMsgAVIoctrlHeartbeatResp resp={0};
            
            memcpy(&resp,p,sizeof(SMsgAVIoctrlHeartbeatResp));
            PRINT_INFO(" heartbeat test!\n");
            avSendIOCtrl(avIndex,IOTYPE_USER_IPCAM_HEARTBEAT_RESP, (char *)&resp, sizeof(SMsgAVIoctrlHeartbeatResp));

            break;
        }

		case IOTYPE_USER_IPCAM_LISTEVENT_DATE_REQ:
		{
			tutk_list_date_event_process(SID, avIndex, buf);
			break;
		}

		default:
		PRINT_INFO("avIndex %d: non-handle type[%X]\n", avIndex, type);
		break;
	}
}

/****
Thread - Start AV server and recv IOCtrl cmd for every new av idx
*/
static void *thread_ForAVServerStart(void *arg)
{
	int SID = (int)arg;
	int ret;
	unsigned int ioType;
	char ioCtrlBuf[MAX_SIZE_IOCTRL_BUF];
	struct st_SInfo Sinfo;

	SET_THREAD_NAME();
	PRINT_INFO("[p2p] SID[%d], thread_ForAVServerStart, OK\n", SID);

	int nResend=-1;
	int avIndex = avServStart3(SID, AuthCallBackFn, 0, SERVTYPE_STREAM_SERVER, 0, &nResend);
	//int avIndex = avServStart2(SID, AuthCallBackFn, 7, SERVTYPE_STREAM_SERVER, 0);
	//int avIndex = avServStart(SID, "admin", "888888", 0, SERVTYPE_STREAM_SERVER, 0);
	if(avIndex < 0)
	{
		PRINT_INFO("[p2p] avServStart2 failed!! SID[%d] code[%d]!!!\n", SID, avIndex);
		PRINT_INFO("[p2p] thread_ForAVServerStart: exit!! SID[%d]\n", SID);
		IOTC_Session_Close(SID);
		gOnlineNum--;
		return 0;
	}
    
	AV_Client *p = &gClientInfo[SID];
	p->avIndex = avIndex;
    
    
	if(IOTC_Session_Check(SID, &Sinfo) == IOTC_ER_NoERROR)
	{
		char *mode[3] = {"P2P", "RLY", "LAN"};
		// print session information(not a must)
		if(isdigit(Sinfo.RemoteIP[0]))
			PRINT_INFO("[p2p] Client is from[IP:%s, Port:%d] Mode[%s] VPG[%d:%d:%d] VER[%X] NAT[%d] AES[%d]\n",
			Sinfo.RemoteIP, Sinfo.RemotePort, mode[(int)Sinfo.Mode], Sinfo.VID, Sinfo.PID, Sinfo.GID,
			Sinfo.IOTCVersion, Sinfo.NatType, Sinfo.isSecure);
	}
	PRINT_INFO("[p2p] avServStart2 OK, avIndex[%d], Resend[%d]\n\n", avIndex, nResend);

	while(1)
	{
		ret = avRecvIOCtrl(avIndex, &ioType, (char *)&ioCtrlBuf, MAX_SIZE_IOCTRL_BUF, 1000);
		if(ret >= 0)
		{
			Handle_IOCTRL_Cmd(SID, avIndex, ioCtrlBuf, ioType,ret);
		}
		else if(ret != AV_ER_TIMEOUT)
		{
			PRINT_ERR("[p2p] AV_ER_TIMEOUT avIndex[%d], avRecvIOCtrl error, code[%d]\n",avIndex, ret);
			break;
		}
	}

	unregedit_client_from_video(SID);
	unregedit_client_from_audio(SID);

	PRINT_INFO("[p2p] SID[%d], avIndex[%d], thread_ForAVServerStart exit!!\n", SID, avIndex);
	avServStop(avIndex);
	IOTC_Session_Close(SID);
    gClientInfo[SID].avIndex = -1;
	gOnlineNum--;

	return 0;
}

static void *thread_Login(void *arg)
{
	int ret;
    unsigned int pnLoginInfo;

    SET_THREAD_NAME();
	while(1)
	{
        //获取登录信息。
        ret = IOTC_Get_Login_Info(&pnLoginInfo);
        //PRINT_INFO("get log info 0x%x ,ret=%d\n",pnLoginInfo,ret);
        //pnLoginInfo bit2 = 1,表示已经登录
        if((pnLoginInfo & 0x4))
        {
            sleep(10);
            continue;
        }
        else//没有登录，则登录之
        {
            ret = IOTC_Device_Login((char *)arg, NULL, NULL);
            //登录成功，
            if(ret == IOTC_ER_NoERROR)
            {
                PRINT_INFO("[p2p] IOTC_Device_Login success.");
                //NEW ADD ,20170520.消息推送需要先register
                SendRegister();
                sleep(10);
                continue;
                //break;
            }
            else
            {
                //登录失败
                PRINT_ERR("[p2p] IOTC_Device_Login() failed, ret = %d\n", ret);
                PrintErrHandling(ret);
                sleep(10);
            }
        }
	}
	return 0;
}

static unsigned int getTimeStamp()
{
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec*1000 + tv.tv_usec/1000);
}

static int isEnableAudio()
{
	int i;
	for(i = 0 ; i < MAX_CLIENT_NUMBER; i++)
	{
		if(gClientInfo[i].bEnableAudio == 1 && gClientInfo[i].avIndex >= 0)
			break;
	}

	return i == MAX_CLIENT_NUMBER ? 0 : 1;
}

static int ResendBufferUsageRate(int index){
	float usagerate=0;
	usagerate = avResendBufUsageRate(index);
	if(usagerate<0) return -1;
	else if(usagerate <= 0.1) return 10;
	else if(usagerate <= 0.2) return 20;
	else if(usagerate <= 0.3) return 30;
	else if(usagerate <= 0.4) return 40;
	else if(usagerate <= 0.5) return 50;
	else if(usagerate <= 0.6) return 60;
	else if(usagerate <= 0.7) return 70;
	else if(usagerate <= 0.8) return 80;
	else if(usagerate <= 0.9) return 90;
	else if(usagerate <= 1.0) return 100;
	else return -1;
}

#if 0
/********
Thread - Send Audio frames to all AV-idx
*/
static void *thread_AudioFrameData(void *arg)
{
	int i, ret;
	int avIndex, recv_frame_len;
	char* buffer;
	FRAMEINFO_t frameInfo;
	GK_NET_FRAME_HEADER frame_header;
	MEDIABUF_HANDLE mhandle = NULL;

	SET_THREAD_NAME();

	mhandle = mediabuf_add_reader(GK_NET_STREAM_TYPE_STREAM0);
	if (!mhandle)
    {
        PRINT_ERR("[p2p] create audio reader error\n");
		return NULL;
    }

	PRINT_INFO("[p2p] thread_AudioFrameData start OK\n");
	PRINT_INFO("[p2p] [Audio] is ENABLED!!\n");

	while(1)
	{
		pthread_mutex_lock(&gAudioMutex);
		while (!isEnableAudio())
			pthread_cond_wait(&gAudioCond, &gAudioMutex);
		pthread_mutex_unlock(&gAudioMutex);

        //摇头机音频输入开关。1开，0关
        if(audioInEnFlag==0)
        {
        
            usleep(200000);
            continue ;
        }
		buffer = NULL;
		recv_frame_len = 0;
		memset(&frame_header, 0, sizeof(GK_NET_FRAME_HEADER));

		ret = mediabuf_read_frame(mhandle, (void**)&buffer, &recv_frame_len, &frame_header);
		if (ret <= 0)
    	{
        	PRINT_INFO("ERROR! read no data.\n");
			continue;
    	}

		memset(&frameInfo, 0, sizeof(FRAMEINFO_t));
		frameInfo.timestamp = getTimeStamp();
		//frameInfo.codec_id = AUDIO_CODEC;
		//frameInfo.codec_id = MEDIA_CODEC_AUDIO_OPUS;
		#if USE_RL_PROTOCOL
        frameInfo.codec_id = MEDIA_CODEC_AUDIO_PCM;
        #else
		frameInfo.codec_id = MEDIA_CODEC_AUDIO_G711A;
        #endif
		if (frame_header.frame_type == GK_NET_FRAME_TYPE_A)
			frameInfo.flags = (AUDIO_SAMPLE_8K << 2) | (AUDIO_DATABITS_16 << 1) | AUDIO_CHANNEL_MONO;
		else
			continue;

        #if USE_RL_PROTOCOL
        #else
		char VoiceBuffer[4096] = {0};
		int new_len = audio_alaw_encode((unsigned char *)VoiceBuffer, (short *)buffer, recv_frame_len);
        #endif
        
		for(i = 0 ; i < MAX_CLIENT_NUMBER; i++)
		{
			if(gClientInfo[i].avIndex < 0 || gClientInfo[i].bEnableAudio == 0)
				continue;
			avIndex = gClientInfo[i].avIndex;

			#if 0
			if(((frameInfo.timestamp/1000)%3) == 0){
				printf("INDEX:[%d] Resend Buffer Usage rate is [%d/100].\n",
					avIndex,
					ResendBufferUsageRate(avIndex)
					);
			}
			#endif

            #if USE_RL_PROTOCOL
			// send audio data to av-idx
			ret = avSendAudioData(avIndex, buffer, recv_frame_len, &frameInfo, sizeof(FRAMEINFO_t));
			//PRINT_INFO("avIndex[%d] ret[%d]\n", gClientInfo[i].avIndex, ret);
            #else
            ret = avSendAudioData(avIndex, VoiceBuffer, new_len, &frameInfo, sizeof(FRAMEINFO_t));
            #endif

            if(ret == AV_ER_SESSION_CLOSE_BY_REMOTE)
			{
				PRINT_ERR("[p2p] thread_AudioFrameData: AV_ER_SESSION_CLOSE_BY_REMOTE\n");
				unregedit_client_from_audio(i);
			}
			else if(ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT)
			{
				PRINT_ERR("[p2p] thread_AudioFrameData: AV_ER_REMOTE_TIMEOUT_DISCONNECT\n");
				unregedit_client_from_audio(i);
			}
			else if(ret == IOTC_ER_INVALID_SID)
			{
				PRINT_ERR("[p2p] Session cant be used anymore\n");
				unregedit_client_from_audio(i);
			}            
            else if(ret == AV_ER_CLIENT_NO_AVLOGIN || ret == AV_ER_INVALID_SID)
            {
            
                PRINT_ERR("[p2p] Session cant be used anymore\n");
                unregedit_client_from_audio(i);
            }
			else if(ret == AV_ER_EXCEED_MAX_SIZE)
			{
			    PRINT_ERR("[p2p] AV_ER_EXCEED_MAX_SIZE\n");
                sleep(1);
                continue;
			}
			else if(ret < 0)
			{
				PRINT_ERR("[p2p] avSendAudioData error[%d]\n", ret);
				//unregedit_client_from_audio(i);
			}
		}
	}

	return 0;
}
#endif
static int isEnableVideo()
{
	int i;
	for(i = 0 ; i < MAX_CLIENT_NUMBER; i++)
	{
		if((gClientInfo[i].bEnableVideo == 1) && (gClientInfo[i].avIndex >= 0))
			break;
	}

	return i == MAX_CLIENT_NUMBER ? 0 : 1;
}
#if 0
/********
Thread - Send Video frames to all AV-idx
*/
static void *thread_VideoFrameData0(void *arg)
{
	int i, ret;
	int avIndex, recv_frame_len;

	char* buffer;
	FRAMEINFO_t frameInfo;
	GK_NET_FRAME_HEADER frame_header;
	MEDIABUF_HANDLE mhandle;

	SET_THREAD_NAME();

	mhandle = mediabuf_add_reader(GK_NET_STREAM_TYPE_STREAM0);
	if (!mhandle)
    {
        PRINT_ERR("[p2p] create video reader error\n");
		return NULL;
    }
    
	PRINT_INFO("[p2p] thread_VideoFrameData0 start OK\n");
	PRINT_INFO("[p2p] [Video] is ENABLED!!\n");

	while(1)
	{
	    #if 0
        pthread_mutex_lock(&gVideoMutex);
        while (!isEnableVideo())
            pthread_cond_wait(&gVideoCond, &gVideoMutex);
        pthread_mutex_unlock(&gVideoMutex);
        #endif
        
        //#if USE_RL_PROTOCOL
        if (tutk_v_send_sub_stream == 1)
        {
            sleep(1);
            continue;
        }
        //#endif

        //PRINT_INFO("client num:%d\n", count1);

		buffer = NULL;
		recv_frame_len = 0;
		memset(&frame_header, 0, sizeof(GK_NET_FRAME_HEADER));

		ret = mediabuf_read_frame(mhandle, (void**)&buffer, &recv_frame_len, &frame_header);
		if (ret <= 0)
    	{
        	PRINT_INFO("ERROR! read no data.\n");
			continue;
    	}

		memset(&frameInfo, 0, sizeof(FRAMEINFO_t));
		frameInfo.timestamp = getTimeStamp();
		frameInfo.codec_id = frame_header.media_codec_type?MEDIA_CODEC_VIDEO_HEVC : MEDIA_CODEC_VIDEO_H264;
		if (frame_header.frame_type == GK_NET_FRAME_TYPE_I)
			frameInfo.flags = IPC_FRAME_FLAG_IFRAME;
		else if (frame_header.frame_type == GK_NET_FRAME_TYPE_P)
			frameInfo.flags = IPC_FRAME_FLAG_PBFRAME;
		else
			continue;

		for(i = 0 ; i < MAX_CLIENT_NUMBER; i++)
		{
			if(gClientInfo[i].avIndex < 0 || gClientInfo[i].bEnableVideo == 0)
				continue;


            #if 0
            #if USE_RL_PROTOCOL
            #else
            if (gClientInfo[i].bVideoOpenStreamId != 0)
                continue;
            #endif
            #endif
            if (gClientInfo[i].bVideoSendKeyFrame == 1)
            {
                if (frame_header.frame_type == GK_NET_FRAME_TYPE_I)
                    gClientInfo[i].bVideoSendKeyFrame = 0;
                else
                    continue;
            }

            
            pthread_mutex_lock(&gSendVideoStreamMutex);
            
			avIndex = gClientInfo[i].avIndex;

			frameInfo.onlineNum = gOnlineNum;
            //PRINT_INFO("tutk send stream0, [i = %d] avIndex:%d onlineNum:%d size:%d\n", i, avIndex, frameInfo.onlineNum, recv_frame_len);
			ret = avSendFrameData(avIndex, buffer, recv_frame_len, &frameInfo, sizeof(FRAMEINFO_t));
            pthread_mutex_unlock(&gSendVideoStreamMutex);

            if(ret == AV_ER_EXCEED_MAX_SIZE)
			{
			    PRINT_ERR("[p2p] AV_ER_EXCEED_MAX_SIZE\n");
				//usleep(10000);
                PRINT_INFO("tutk send stream0, [i = %d] avIndex:%d onlineNum:%d size:%d\n", i, avIndex, frameInfo.onlineNum, recv_frame_len);

                gClientInfo[i].bVideoSendKeyFrame = 1;

                #if USE_V_ADJUST
                stream0_bps -= stream_bps_del;
                if (stream0_bps < V_QUALITY_LEVEL_3)
                    stream0_bps = V_QUALITY_LEVEL_3;
                netcam_adjust_bps(0, stream0_bps);
                PRINT_ERR("adjust stream0 bps:%d", stream0_bps);
                #endif
                sleep(1);
                continue;
			}
			else if(ret == AV_ER_SESSION_CLOSE_BY_REMOTE)
			{
				PRINT_ERR("[p2p] thread_VideoFrameData AV_ER_SESSION_CLOSE_BY_REMOTE SID[%d]\n", i);
				unregedit_client_from_video(i);
				continue;
			}
			else if(ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT)
			{
				PRINT_ERR("[p2p] thread_VideoFrameData AV_ER_REMOTE_TIMEOUT_DISCONNECT SID[%d]\n", i);
				unregedit_client_from_video(i);
				continue;
			}
			else if(ret == IOTC_ER_INVALID_SID)
			{
				PRINT_ERR("[p2p] Session cant be used anymore\n");
				unregedit_client_from_video(i);
				continue;
			}
            else if(ret == AV_ER_CLIENT_NO_AVLOGIN || ret == AV_ER_INVALID_SID)
            {
				PRINT_ERR("[p2p] thread_VideoFrameData ret=%d SID[%d]\n", ret,i);
				unregedit_client_from_video(i);
				continue;            
            }
			else if(ret < 0)
				PRINT_ERR("[p2p] XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX ret:%d\n", ret);
            
        }
	}

    mediabuf_del_reader(mhandle);

	return 0;
}

static void *thread_VideoFrameData1(void *arg)
{
	int i, ret;
	int avIndex, recv_frame_len;

	char* buffer;
	FRAMEINFO_t frameInfo;
	GK_NET_FRAME_HEADER frame_header;
	MEDIABUF_HANDLE mhandle;

	SET_THREAD_NAME();
    mhandle = mediabuf_add_reader(GK_NET_STREAM_TYPE_STREAM1);
	if (!mhandle)
    {
        PRINT_ERR("[p2p] create video reader error\n");
		return NULL;
    }
    
	PRINT_INFO("[p2p] thread_VideoFrameData1 start OK\n");
	PRINT_INFO("[p2p] [Video] is ENABLED!!\n");

	while(1)
	{
	    #if 0
        pthread_mutex_lock(&gVideoMutex);
        while (!isEnableVideo())
            pthread_cond_wait(&gVideoCond, &gVideoMutex);
        pthread_mutex_unlock(&gVideoMutex);
        #endif
        
        if (tutk_v_send_sub_stream == 0)
        {
            sleep(1);
            continue;
        }
   

        //PRINT_INFO("client num:%d\n", count1);

		buffer = NULL;
		recv_frame_len = 0;
		memset(&frame_header, 0, sizeof(GK_NET_FRAME_HEADER));

		ret = mediabuf_read_frame(mhandle, (void**)&buffer, &recv_frame_len, &frame_header);
		if (ret <= 0)
    	{
        	PRINT_INFO("ERROR! read no data.\n");
			continue;
    	}

		memset(&frameInfo, 0, sizeof(FRAMEINFO_t));
		frameInfo.timestamp = getTimeStamp();
        frameInfo.codec_id = frame_header.media_codec_type?MEDIA_CODEC_VIDEO_HEVC : MEDIA_CODEC_VIDEO_H264;
		if (frame_header.frame_type == GK_NET_FRAME_TYPE_I)
			frameInfo.flags = IPC_FRAME_FLAG_IFRAME;
		else if (frame_header.frame_type == GK_NET_FRAME_TYPE_P)
			frameInfo.flags = IPC_FRAME_FLAG_PBFRAME;
		else
			continue;

		for(i = 0 ; i < MAX_CLIENT_NUMBER; i++)
		{
			if(gClientInfo[i].avIndex < 0 || gClientInfo[i].bEnableVideo == 0)
				continue;

            #if 0
            #if USE_RL_PROTOCOL
            #else
            if (gClientInfo[i].bVideoOpenStreamId != 1)
                continue;
            #endif
            #endif

            if (gClientInfo[i].bVideoSendKeyFrame == 1)
            {
                if (frame_header.frame_type == GK_NET_FRAME_TYPE_I)
                    gClientInfo[i].bVideoSendKeyFrame = 0;
                else
                    continue;
            }

            pthread_mutex_lock(&gSendVideoStreamMutex);
            
			avIndex = gClientInfo[i].avIndex;

			frameInfo.onlineNum = gOnlineNum;
            //PRINT_INFO("tutk send stream1, [i = %d] avIndex:%d onlineNum:%d size:%d\n", i, avIndex, frameInfo.onlineNum, recv_frame_len);
			ret = avSendFrameData(avIndex, buffer, recv_frame_len, &frameInfo, sizeof(FRAMEINFO_t));
            pthread_mutex_unlock(&gSendVideoStreamMutex);

            if(ret == AV_ER_EXCEED_MAX_SIZE)
			{
                PRINT_ERR("[p2p] AV_ER_EXCEED_MAX_SIZE\n");
                //usleep(10000);

                gClientInfo[i].bVideoSendKeyFrame = 1;

                #if USE_V_ADJUST
                stream1_bps -= stream_bps_del;
                if (stream1_bps < V_QUALITY_LEVEL_3)
                    stream1_bps = V_QUALITY_LEVEL_3;
                netcam_adjust_bps(1, stream1_bps);
                PRINT_ERR("adjust stream1 bps:%d", stream1_bps);
                #endif
                sleep(1);

                continue;
			}
			else if(ret == AV_ER_SESSION_CLOSE_BY_REMOTE)
			{
				PRINT_ERR("[p2p] thread_VideoFrameData AV_ER_SESSION_CLOSE_BY_REMOTE SID[%d]\n", i);
				unregedit_client_from_video(i);
				continue;
			}
			else if(ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT)
			{
				PRINT_ERR("[p2p] thread_VideoFrameData AV_ER_REMOTE_TIMEOUT_DISCONNECT SID[%d]\n", i);
				unregedit_client_from_video(i);
				continue;
			}
			else if(ret == IOTC_ER_INVALID_SID)
			{
				PRINT_ERR("[p2p] Session cant be used anymore\n");
				unregedit_client_from_video(i);
				continue;
			}     
            else if(ret == AV_ER_CLIENT_NO_AVLOGIN || ret == AV_ER_INVALID_SID)
            {
				PRINT_ERR("[p2p] thread_VideoFrameData ret=%d SID[%d]\n", ret,i);
				unregedit_client_from_video(i);
				continue;            
            }
			else if(ret < 0)
				PRINT_ERR("[p2p] XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX ret:%d\n", ret);

        }
	}

    mediabuf_del_reader(mhandle);

	return 0;
}
#endif
static void tutk_venc_change_stream_attr(int vi,int streamId,int level)
{
    unsigned int bitrate = 0;

	GK_NET_IMAGE_CFG imgAttr;
	memset(&imgAttr,0,sizeof(GK_NET_IMAGE_CFG));
	netcam_image_get(&imgAttr);

    if(imgAttr.irCutMode == 1)
    {
        if(level == AV_DASA_LEVEL_QUALITY_HIGH){

            if(0 == streamId)
                bitrate = 800;
            else
                bitrate = 400;

        }
        else if(level == AV_DASA_LEVEL_QUALITY_BTWHIGHNORMAL){
            if(0 == streamId)
                bitrate = 512;
            else
                bitrate = 256;

        }
        else if(level == AV_DASA_LEVEL_QUALITY_NORMAL){
            if(0 == streamId)
                bitrate = 384;
            else
                bitrate = 192;

        }
        else if(level == AV_DASA_LEVEL_QUALITY_BTWNORMALLOW){
            if(0 == streamId)
                bitrate = 256;
            else
                bitrate = 128;

        }
        else{
            if(0 == streamId)
                bitrate = 128;
            else
                bitrate = 64;

        }
    }
    else  //isp night
    {
        if(level == AV_DASA_LEVEL_QUALITY_HIGH){
            if(0 == streamId)
                bitrate = 640;
            else
                bitrate = 320;

        }
        else if(level == AV_DASA_LEVEL_QUALITY_BTWHIGHNORMAL){
            if(0 == streamId)
                bitrate = 512;
            else
                bitrate = 256;

        }
        else if(level == AV_DASA_LEVEL_QUALITY_NORMAL){
            if(0 == streamId)
                bitrate = 256;//384;
            else
                bitrate = 192;

        }
        else if(level == AV_DASA_LEVEL_QUALITY_BTWNORMALLOW){
            if(0 == streamId)
                bitrate = 256;
            else
                bitrate = 128;

        }
        else{
            if(0 == streamId)
                bitrate = 128;
            else
                bitrate = 64;

       }
    }

#if 0
    ST_GK_ENC_STREAM_H264_ATTR h264Attr;
    memset(&h264Attr, 0, sizeof(ST_GK_ENC_STREAM_H264_ATTR));
    netcam_video_get(0, streamId ,&h264Attr);
    h264Attr.bps = bitrate;
    netcam_video_set(0, streamId ,&h264Attr);	
#endif

	netcam_adjust_bps(streamId,bitrate);
}

static void tutk_stream_set(int SID,MEDIABUF_HANDLE AVHandle,int isDAS,int isResetBuf)
{
    int DASALevel = 0;

	if(SID >= MAX_CLIENT_NUMBER)
	{
        PRINT_INFO("[tutk_stream_set] param error!\n");
		return;
	}

	PRINT_INFO("[tutk_stream_set] SID:%d,DAS:%d,restbuf:%d!\n",SID,isDAS,isResetBuf);

	if(isDAS)
	{
		DASALevel = avDASACheck(gClientInfo[SID].avIndex);
		tutk_venc_change_stream_attr(gClientInfo[SID].bSensorNo, gClientInfo[SID].bStreamNo, DASALevel);
		switch(DASALevel)
		{
			case AV_DASA_LEVEL_QUALITY_HIGH:
				gClientInfo[SID].bQuality = AVIOCTRL_QUALITY_HIGH;
				break;
			case AV_DASA_LEVEL_QUALITY_NORMAL:
				gClientInfo[SID].bQuality = AVIOCTRL_QUALITY_MIDDLE;
				break;
			case AV_DASA_LEVEL_QUALITY_LOW:
			default:
				gClientInfo[SID].bQuality = AVIOCTRL_QUALITY_LOW;
				break;
		
		}
	}
	
	if(isResetBuf)
		avServResetBuffer(gClientInfo[SID].avIndex, RESET_ALL, 2000);
	
	mediabuf_set_newest_frame(AVHandle);
	
	netcam_video_force_i_frame(gClientInfo[SID].bStreamNo);

    gClientInfo[SID].bNextIFrame = 1;
					
}

static void tutk_ReceiveAudio(int SID,int on)
{
	int ret = 0;
    unsigned int servType = 0, frmNo = 0;
    char buf[AUDIO_BUF_SIZE];
    FRAMEINFO_t frameInfo;
    AV_Client *p = NULL;

	if(SID >= MAX_CLIENT_NUMBER)
	{
        PRINT_INFO("[tutk_ReceiveAudio] param error!\n");
		return;
	}

    p = (AV_Client *)&gClientInfo[SID];

	if(on)
	{
		if(p->speakerAvIndex < 0)
		{
			PRINT_INFO("[tutk_ReceiveAudio]avClientStart+++\n");
			p->speakerAvIndex = avClientStart(SID, NULL, NULL, 10, &servType, p->speakerCh);
			PRINT_INFO("[tutk_ReceiveAudio]avClientStart---\n");

			if(p->speakerAvIndex > -1)
			{
				PRINT_INFO("[tutk_ReceiveAudio] start avIndex[%d],p->speakerCh:%d\n", p->speakerAvIndex,p->speakerCh);
				ret = avClientCleanAudioBuf(p->speakerAvIndex);
				if(ret != 0)
					P2P_ERR("Reset audio buf error:%d",ret);
			}
			else
				goto exit;
		}

		if(p->speakerAvIndex > -1)
		{
			ret = avCheckAudioBuf(p->speakerAvIndex);
			if(ret >= 20)
			{
				PRINT_INFO("Discard tutk audio frame conunt:%d",ret);
				ret = avClientCleanAudioBuf(p->speakerAvIndex);
				if(ret != 0)
					P2P_ERR("ret error:%d",ret);

				return;
			}

			memset(buf,AUDIO_BUF_SIZE,0);

			ret = avRecvAudioData(p->speakerAvIndex, buf, AUDIO_BUF_SIZE, (char *)&frameInfo, sizeof(FRAMEINFO_t), &frmNo);
			if(ret == AV_ER_SESSION_CLOSE_BY_REMOTE)
			{
				PRINT_INFO("avRecvAudioData AV_ER_SESSION_CLOSE_BY_REMOTE\n");
				goto exit;
			}
			else if(ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT)
			{
				PRINT_INFO("avRecvAudioData AV_ER_REMOTE_TIMEOUT_DISCONNECT\n");
				goto exit;
			}
			else if(ret == IOTC_ER_INVALID_SID)
			{
				PRINT_INFO("avRecvAudioData Session[%d] cant be used anymore\n", SID);
				goto exit;
			}
			else if(ret == AV_ER_LOSED_THIS_FRAME)
			{
				PRINT_INFO("Audio LOST[%d]\n", frmNo);
				return;
			}
			else if (ret == AV_ER_DATA_NOREADY)
	        {
				return;
			}
			else if(ret < 0)
			{
				PRINT_INFO("Other error[%d]!!!\n", ret);
				goto exit;
			}
			else
			{
				switch (frameInfo.codec_id)
				{
					case MEDIA_CODEC_AUDIO_PCM:
			        {
			            netcam_audio_output(buf, ret, NETCAM_AUDIO_ENC_RAW_PCM, SDK_FALSE);
						break;
			        }
					case MEDIA_CODEC_AUDIO_G711A:
			        {
			            netcam_audio_output(buf, ret, NETCAM_AUDIO_ENC_A_LAW, SDK_FALSE);
						break;
			        }
					default:
					break;
				}
			}

		}
	}
	else
	{

exit:
		PRINT_INFO("[tutk_ReceiveAudio] avClientStop++\n");
		if(p->speakerAvIndex >= 0)
		{
			avClientStop(p->speakerAvIndex);
			avClientExit(SID, p->speakerCh);
		}

    	PRINT_INFO("[tutk_ReceiveAudio] avClientStop--\n");

		pthread_mutex_lock(&p->sLock);
		p->speakerCh = -1;
		p->speakerAvIndex = -1;
		p->bEnableSpeaker = 0;
		pthread_mutex_unlock(&p->sLock);

	    speaker_stop();

		PRINT_INFO("[tutk_ReceiveAudio] exit\n");
	}

}

static void *thread_AVFrameData(void *arg)
{

	SET_THREAD_NAME();

	int ret = 0, i = 0, j;
    unsigned int writeSize = 0;
	AV_FRAME *vfrm = NULL;
    AV_FRAME *afrm = NULL;
    char* writeBuf = NULL;
	int bufferRate = 0;
	GK_NET_FRAME_HEADER frame_header;
    int recv_frame_len;
    int size = 1024;
	char *dataBuf = NULL;
    char alaw_data[4096];
	int sleepTime = 0;
	AV_FRAME AudioFrame;
	AV_FRAME VideoFrame;
	MEDIABUF_HANDLE AVHandle;
	unsigned char sensorNo,streamNo;
    FRAMEINFO_t vframeInfo,aframeInfo;
	unsigned int t1 = 0,t2 = 0;
    memset(&vframeInfo, 0, sizeof(FRAMEINFO_t));
    memset(&aframeInfo, 0, sizeof(FRAMEINFO_t));	
    memset(&VideoFrame, 0, sizeof(AV_FRAME));
    memset(&AudioFrame, 0, sizeof(AV_FRAME));

#if AUDIO_USED_G711A
    aframeInfo.codec_id = MEDIA_CODEC_AUDIO_G711A;
#else
    aframeInfo.codec_id = MEDIA_CODEC_AUDIO_PCM;;
#endif
    aframeInfo.flags = (AUDIO_SAMPLE_8K << 2) | (AUDIO_DATABITS_16 << 1) | AUDIO_CHANNEL_MONO;


	for(i=0; i<VIN_NUM; i++)
	{
	    for(j=0; j<MAX_CLIENT_NUMBER; j++)
	    {
		    AVHandleStream0[i][j] = mediabuf_add_reader(0);
	    	if (!AVHandleStream0[i][j])
	        {
	            P2P_ERR("[p2p] create stream0 av reader error\n");
	    		return NULL;
	        }

	        AVHandleStream1[i][j] = mediabuf_add_reader(1);
	    	if (!AVHandleStream1[i][j])
	        {
	            P2P_ERR("[p2p] create stream1 av reader error\n");
	    		return NULL;
	        }
	    }
	}

    PRINT_INFO("thread_AVFrameData start OK\n");
    PRINT_INFO("[Video/AUDIO] is ENABLED!!\n");

    while(1)
    {
    	if(t1 == 0)
    		t1 = getTimeStamp();

        for(i = 0 ; i < MAX_CLIENT_NUMBER; i++)
        {
        	sensorNo = 0;
        	streamNo = gClientInfo[i].bStreamNo;
			
            if(0 == streamNo)
                AVHandle = AVHandleStream0[sensorNo][i];
            else
                AVHandle = AVHandleStream1[sensorNo][i];

			//pthread_mutex_lock(&gClientInfo[i].sLock);
            if((!gClientInfo[i].bEnableVideo && !gClientInfo[i].bEnableAudio) || gClientInfo[i].avIndex<0)
            {
				//pthread_mutex_unlock(&gClientInfo[i].sLock);
				sleepTime = 0;
				goto SPEAKER_PROCESS;
            }

			if(gClientInfo[i].bNewConnection)
			{
				gClientInfo[i].bNewConnection = 0;
				
				tutk_stream_set(i, AVHandle, 0, 0);

				sleepTime = 0;
				//pthread_mutex_unlock(&gClientInfo[i].sLock);
				goto SPEAKER_PROCESS;
			}

			//pthread_mutex_unlock(&gClientInfo[i].sLock);

			afrm = NULL;
			vfrm = NULL;
			dataBuf = NULL;
			recv_frame_len = 0;

			memset(&frame_header, 0, sizeof(GK_NET_FRAME_HEADER));

			ret = mediabuf_read_frame(AVHandle, (void **)&dataBuf, &recv_frame_len, &frame_header);

			if (ret <= 0)
			{
				sleepTime = 10;
				goto SPEAKER_PROCESS;
			}
			
			vframeInfo.codec_id = frame_header.media_codec_type?MEDIA_CODEC_VIDEO_HEVC : MEDIA_CODEC_VIDEO_H264;

			if(gClientInfo[i].bEnableAudio && frame_header.frame_type == GK_NET_FRAME_TYPE_A)
			{

#if AUDIO_USED_G711A
				//pcm->alaw
				if(recv_frame_len / 2 > sizeof(alaw_data))
				{
					P2P_ERR("recv_frame_len[%d] error\n", recv_frame_len);
					sleepTime = 0;
					goto SPEAKER_PROCESS;
				}

				size = audio_alaw_encode((unsigned char *)alaw_data, (short *)dataBuf, recv_frame_len);
#else
				if(recv_frame_len > sizeof(alaw_data))
				{
					sleepTime = 0;
					goto SPEAKER_PROCESS;
				}

				memcpy(alaw_data,dataBuf,recv_frame_len);
				size = recv_frame_len;

#endif

				AudioFrame.size = size;
				AudioFrame.addr = alaw_data;
				AudioFrame.ts = (frame_header.sec*1000 + frame_header.usec/1000);

				afrm =	&AudioFrame;
			}

			if(gClientInfo[i].bEnableVideo && frame_header.frame_type != GK_NET_FRAME_TYPE_A)
			{
				if(frame_header.frame_type == GK_NET_FRAME_TYPE_I)
				{
					VideoFrame.type = IPC_FRAME_FLAG_IFRAME;
				}
				else if(frame_header.frame_type == GK_NET_FRAME_TYPE_P)
				{
					VideoFrame.type = IPC_FRAME_FLAG_PBFRAME;
				}
				
				vframeInfo.codec_id = frame_header.media_codec_type?MEDIA_CODEC_VIDEO_HEVC : MEDIA_CODEC_VIDEO_H264;
				
				VideoFrame.addr = dataBuf;
				VideoFrame.size = recv_frame_len;
				VideoFrame.ts = (frame_header.sec*1000 + frame_header.usec/1000);

				vfrm = &VideoFrame;
			}

            if (vfrm)
            {
                writeBuf = (char*)(vfrm->addr);
                writeSize = vfrm->size;
                vframeInfo.timestamp = vfrm->ts;
                vframeInfo.onlineNum = gOnlineNum;
				vframeInfo.cam_index = sensorNo;

                if(vfrm->type == IPC_FRAME_FLAG_IFRAME){
                    vframeInfo.flags = IPC_FRAME_FLAG_IFRAME;
                }
                else
                    vframeInfo.flags = IPC_FRAME_FLAG_PBFRAME;

				ret = mediabuf_get_leave_frame(AVHandle);
				if(ret >= 1)
				{
					bufferRate = ResendBufferUsageRate(gClientInfo[i].avIndex);

					if(ret >  40|| bufferRate > 40)
					{
						PRINT_INFO(" Media fifo(%d),or tutk buffer(%d) exceed,next use i frame",ret,bufferRate);
						
						tutk_stream_set(i, AVHandle, DASA_ENABLED, bufferRate > 40);

						sleepTime = 0;
						goto SPEAKER_PROCESS;
					}

				}
				
                if(gClientInfo[i].bNextIFrame)
                {
                    if(IPC_FRAME_FLAG_IFRAME == vframeInfo.flags)
                    {
                        ret = avSendFrameData(gClientInfo[i].avIndex, writeBuf, writeSize, &vframeInfo, sizeof(FRAMEINFO_t));
                        gClientInfo[i].bNextIFrame--;
                    }
                    else
                    {
                        //PRINT_INFO("\n%d skip P vframe\n", i);
                        ret = 0;
                    }
                }
                else
                {
                    // Send Video Frame to av-idx and know how many time it takes
                    ret = avSendFrameData(gClientInfo[i].avIndex, writeBuf, writeSize, &vframeInfo, sizeof(FRAMEINFO_t));
                }


                if(ret == AV_ER_EXCEED_MAX_SIZE) // means data not write to queue, send too slow, I want to skip it
                {
                    PRINT_INFO("AV_ER_EXCEED_MAX_SIZE SID[%d]\n", i);
                    //pthread_mutex_lock(&gClientInfo[i].sLock);

					tutk_stream_set(i, AVHandle, 0, 1);
					
                    //pthread_mutex_unlock(&gClientInfo[i].sLock);
                }
                else if(ret == AV_ER_SESSION_CLOSE_BY_REMOTE)
                {
                    PRINT_INFO("thread_AVFrameData AV_ER_SESSION_CLOSE_BY_REMOTE SID[%d]\n", i);
                    unregedit_client_from_video(i);
                }
                else if(ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT)
                {
                    PRINT_INFO("thread_AVFrameData AV_ER_REMOTE_TIMEOUT_DISCONNECT SID[%d]\n", i);
                    unregedit_client_from_video(i);
                }
                else if(ret == IOTC_ER_INVALID_SID)
                {
                    PRINT_INFO("thread_AVFrameData IOTC_ER_INVALID_SID\n");
                    unregedit_client_from_video(i);
                }
                else if(ret == AV_ER_DASA_CLEAN_BUFFER)
                {
					PRINT_INFO("avIndex[%d] need to do clean buffer\n", gClientInfo[i].avIndex);
					tutk_stream_set(i, AVHandle, DASA_ENABLED, 1);
                }
                else if(ret < 0)
                {
                    PRINT_INFO("thread_AVFrameData unknow error ret[%d]\n", ret);
                }

				sleepTime = 0;
            }

			if(afrm && (afrm->addr != NULL))
			{

				aframeInfo.timestamp = afrm->ts;
				aframeInfo.onlineNum = gOnlineNum;
				aframeInfo.cam_index = sensorNo;

				ret = avSendAudioData(gClientInfo[i].avIndex, (const char *)afrm->addr, afrm->size, (const void*)&aframeInfo, sizeof(FRAMEINFO_t));

				if(ret == AV_ER_SESSION_CLOSE_BY_REMOTE)
				{
					PRINT_INFO("thread_AVFrameData: AV_ER_SESSION_CLOSE_BY_REMOTE\n");
					unregedit_client_from_audio(i);
				}
				else if(ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT)
				{
					PRINT_INFO("thread_AVFrameData: AV_ER_REMOTE_TIMEOUT_DISCONNECT\n");
					unregedit_client_from_audio(i);
				}
				else if(ret == IOTC_ER_INVALID_SID)
				{
					PRINT_INFO("thread_AVFrameData: IOTC_ER_INVALID_SID\n");
					unregedit_client_from_audio(i);
				}
				else if(ret == AV_ER_EXCEED_MAX_SIZE)
				{
					PRINT_INFO("thread_AVFrameData: AV_ER_EXCEED_MAX_SIZE\n");
					//pthread_mutex_lock(&gClientInfo[i].sLock);

					tutk_stream_set(i, AVHandle, 1, 1);

					//pthread_mutex_unlock(&gClientInfo[i].sLock);
				}
				else if(ret == AV_ER_DASA_CLEAN_BUFFER)
				{
				
					PRINT_INFO("avIndex[%d] need to do clean buffer\n", gClientInfo[i].avIndex);
					tutk_stream_set(i, AVHandle, DASA_ENABLED, 1);
				}
				else if(ret < 0)
				{
					PRINT_INFO("thread_AVFrameData error[%d]\n", ret);

				}

				sleepTime = 0;
			}

		SPEAKER_PROCESS:
			#if 0
			if(gClientInfo[i].bEnableSpeaker == 1)
				tutk_ReceiveAudio(i,1);
			else if(gClientInfo[i].bEnableSpeaker == 0 && gClientInfo[i].speakerAvIndex >=0)
				tutk_ReceiveAudio(i,0);
			//else if(i == MAX_CLIENT_NUMBER -1)
				//sleepTime = 5;
			#endif
			if(sleepTime)
				usleep(sleepTime*1000);

		}

		t2 = getTimeStamp();

		if(gOnlineNum == 0 || (!isEnableAudio() && !isEnableVideo()))
			usleep(50000);
		else if((t2 - t1) > 2000)
		{
			t1 = t2;
			//usleep(5000);
		}
		
    }

	for(i=0; i<VIN_NUM; i++)
	{
	    for(j=0 ; j<MAX_CLIENT_NUMBER; j++)
	    {
			mediabuf_del_reader(AVHandleStream0[i][j]);
			mediabuf_del_reader(AVHandleStream1[i][j]);
	    }
	}

    PRINT_INFO("[thread_AVFrameData] exit\n");

    return NULL;
}

static int InitAVInfo()
{
#if 1
	int i;
	for(i=0;i<MAX_CLIENT_NUMBER;i++)
	{
		memset(&gClientInfo[i], 0, sizeof(AV_Client));
		gClientInfo[i].avIndex = -1;
		gClientInfo[i].playBackCh = -1;
		gClientInfo[i].bNextIFrame = 1;
		gClientInfo[i].speakerAvIndex = -1;
		gClientInfo[i].speakerCh = -1;
		pthread_mutex_init(&(gClientInfo[i].sLock), NULL);
	}

#else
	int i;
	for(i=0;i<MAX_CLIENT_NUMBER;i++)
	{
		memset(&gClientInfo[i], 0, sizeof(AV_Client));
		gClientInfo[i].avIndex = -1;
		gClientInfo[i].playBackCh = -1;

        gClientInfo[i].bVideoSendKeyFrame = 1;

	}

    audioInEnFlag = 1;
    audioOutEnFlag = 1;

	if (pthread_mutex_init(&gVideoMutex, NULL) < 0)
    {
        PRINT_ERR("init error.\n");
		return -1;
    }
	if (pthread_cond_init(&gVideoCond, NULL) < 0)
    {
        PRINT_ERR("init error.\n");
		return -1;
    }

	if (pthread_mutex_init(&gAudioMutex, NULL) < 0)
    {
        PRINT_ERR("init error.\n");
		return -1;
    }
	if (pthread_cond_init(&gAudioCond, NULL) < 0)
    {
        PRINT_ERR("init error.\n");
		return -1;
    }
#endif
	return 0;
}

static void DeInitAVInfo()
{
#if 1
	int i;
	for(i=0;i<MAX_CLIENT_NUMBER;i++)
	{
		memset(&gClientInfo[i], 0, sizeof(AV_Client));
		gClientInfo[i].avIndex = -1;
		gClientInfo[i].bEnableAudio = 0;
		gClientInfo[i].bEnableVideo = 0;
		gClientInfo[i].bEnableSpeaker = 0;
		gClientInfo[i].bStopPlayBack = 1;
	}
	
#else
	int i;
	for(i=0;i<MAX_CLIENT_NUMBER;i++)
	{
		memset(&gClientInfo[i], 0, sizeof(AV_Client));
		gClientInfo[i].avIndex = -1;
	}
    audioInEnFlag = 0;
    audioOutEnFlag = 0;

	pthread_mutex_destroy(&gVideoMutex);
	pthread_cond_destroy(&gVideoCond);
	pthread_mutex_destroy(&gAudioMutex);
	pthread_cond_destroy(&gAudioCond);
#endif
}

static void  *check_sd_thread()
{
    int i = 0;
    int count = 0;
    while(mmc_get_sdcard_stauts() == 2)
    {
        count ++;
        if(count >= 30)//30S超时还是发现sd没有格式化，则格式化之
        {
            grd_sd_format();
        }
        PRINT_INFO("SD status:%d,total size:%d\n",mmc_get_sdcard_stauts(),grd_sd_get_all_size());
        
        sleep(1);
    }
    PRINT_INFO("check_sd_thread end ,sd status:%d,total size:%d\n",mmc_get_sdcard_stauts(),grd_sd_get_all_size());

}
static int create_check_sd_thread()
{
    int ret = 0;
	pthread_t ThreadCheckSd;
	if((ret = pthread_create(&ThreadCheckSd, NULL, &check_sd_thread, NULL)))
	{
		PRINT_INFO("pthread_create ret=%d\n", ret);
		return -1;
	}
    return 0;
}

static int create_streamout_thread()
{
#if 0
	int ret;

	pthread_t ThreadVideoFrameData_ID0;
	if((ret = pthread_create(&ThreadVideoFrameData_ID0, NULL, &thread_VideoFrameData0, NULL)))
	{
		PRINT_INFO("pthread_create ret=%d\n", ret);
		return -1;
	}

    pthread_t ThreadVideoFrameData_ID1;
	if((ret = pthread_create(&ThreadVideoFrameData_ID1, NULL, &thread_VideoFrameData1, NULL)))
	{
		PRINT_INFO("pthread_create ret=%d\n", ret);
		return -1;
	}

    pthread_t ThreadAudioFrameData_ID;
	if((ret = pthread_create(&ThreadAudioFrameData_ID, NULL, &thread_AudioFrameData, NULL)))
	{
		PRINT_INFO("pthread_create ret=%d\n", ret);
		return -1;
	}
#else

	int ret;

	pthread_t ThreadAVFrameData_ID;
	if((ret = pthread_create(&ThreadAVFrameData_ID, NULL, &thread_AVFrameData, NULL)))
	{
		PRINT_INFO("pthread_create ret=%d\n", ret);
		return -1;
	}

	return 0;


#endif
	return 0;
}

static void LoginInfoCB(unsigned int nLoginInfo)
{
	if((nLoginInfo & 0x04))
		PRINT_ERR("I can be connected via Internet\n");
	else if((nLoginInfo & 0x08))
		PRINT_ERR("I am be banned by IOTC Server because UID multi-login\n");
}

static int tutk_load_p2p_id()
{
    #if 1
    struct device_info_mtd info;
    int ret = load_info_to_mtd_reserve(MTD_TUTK_P2P, &info, sizeof(info));
    if((ret < 0) || (info.device_id[0] == 0))
    {
        PRINT_ERR("[p2p] Fail to load p2p id from flash, p2p id:%s\n", info.device_id);

        strncpy(info.device_id, "123456789ABCDEFGHIJK", sizeof(info.device_id));
        strncpy(info.manufacturer_id, "goke_ds", sizeof(info.manufacturer_id));

        /*NEW ADD ,TUTK APP获取的参数,在进行生产时，先使用默认方式得到配置，然后生产人员
        读取flash相应位置的数据并根据实际情况改写数据，如读到ODM_DS处，并改写实际的ODM类型
        之后，再利用这个写好的flash去烧写其他机器，量产，app读取写着数据时，在Handle_IOCTRL_Cmd
        里面，从flash读取数据并返回app相应数据。
        */
        #if 0
        strncpy(info.device_type, "YTJ_NORMAL", sizeof(info.device_type));
        strncpy(info.odm_id, "ODM_DS", sizeof(info.odm_id));
        strncpy(info.p2p_type, "P2P_TUTK", sizeof(info.p2p_type));
        strncpy(info.panorama_mode, "PANORAMA_TOP", sizeof(info.panorama_mode));
        strncpy(info.language, "SIMPLE_CH", sizeof(info.panorama_mode));
        #endif
        //info.devType = 1;// 设备类型: 0-卡片机, 1-摇头机, 2-鱼眼摄像机...
        info.devType = runSystemCfg.deviceInfo.devType;//0-卡片机, 1-摇头机, 2-鱼眼摄像机.,3,门铃单向，4 门铃双向，5模组

        info.netFamily = 2;// 网络提供商: 1-迈特威视, 2-tutk...
        info.serverID = 0; // 服务器ID: 保留,默认为0
        info.language = 1; // 设备固件语言版本: 0为自动适应(即多语言),1为简体中文,2为繁体中文,3为英文
        info.odmID = runSystemCfg.deviceInfo.odmId;
        //info.odmID = 0;// odm商ID: 0-东舜自己, 01-TCL, 02-康佳...
        info.panoramaMode = 0;// 全景安装模式: 0-吸顶式,1-壁挂式  
        //strncpy(info.model, "ts-7000-w", sizeof(info.model));
        strncpy(info.model, runSystemCfg.deviceInfo.model, sizeof(info.model));


        ret = save_info_to_mtd_reserve(MTD_TUTK_P2P, &info, sizeof(info));
        if (ret < 0)
        {
            PRINT_ERR("[p2p] set p2p id to flash error.\n");
            return -1;
        }
        else
        {
            PRINT_INFO("[p2p] set p2p id %s to flash ok.\n", info.device_id);
        }
    }

    PRINT_INFO("P2P ID :%s\n",info.device_id);
    PRINT_INFO("devType :%u\n",info.devType);
    PRINT_INFO("netFamily :%u\n",info.netFamily);
    PRINT_INFO("language :%u\n",info.language);
    PRINT_INFO("odmID ID:%u\n",info.odmID);
    PRINT_INFO("panoramaMode :%u\n",info.panoramaMode);
    PRINT_INFO("model :%s\n",info.model);
    
    strncpy(runSystemCfg.deviceInfo.serialNumber, info.device_id,
    	sizeof(runSystemCfg.deviceInfo.serialNumber));
    #else
    sprintf(runSystemCfg.deviceInfo.serialNumber, "%s", "EFPA9D5WKV74AG6GY1XJ");
    #endif
    SystemCfgSave();
    PRINT_INFO("[p2p] get from mtd, serialNumber:%s\n", runSystemCfg.deviceInfo.serialNumber);

    return 0;
}
#if USE_HTS_PROTOCOL
static void tutk_alarm_md_hts(int avIndex)
{
	SMsgAVIoctrlEvent res;
	time_t timep;
	struct tm *p;
	struct tm t1 = {0};

	timep = time(NULL);
	timep -= runSystemCfg.timezoneCfg.timezone * 60;
    p = localtime_r(&timep, &t1);

	memset(&res, 0, sizeof(SMsgAVIoctrlEvent));
	res.time = timep;
	res.stTime.year = p->tm_year + 1900;
	res.stTime.month = p->tm_mon + 1;
	res.stTime.day = p->tm_mday;
	res.stTime.hour = p->tm_hour;
	res.stTime.minute = p->tm_min;
	res.stTime.second = p->tm_sec;
	res.event = AVIOCTRL_EVENT_MOTIONDECT;

	PRINT_INFO("time:%u, %d-%d-%d %d:%d:%d\n", (unsigned int)timep, res.stTime.year, res.stTime.month, res.stTime.day,
		res.stTime.hour, res.stTime.minute, res.stTime.second);
	int ret = avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_EVENT_REPORT, (char *)&res, sizeof(SMsgAVIoctrlEvent));
    if(ret != AV_ER_NoERROR){
        PRINT_INFO("send alarm msg to client failed with error:[%d].\n",ret);
    }

    PRINT_INFO("notifiy motion detect, ret = %d\n", ret);
}
#endif


#if 1

static void * TutkAlarmPushThread(void* arg)
{
    SetThreadName("TutkAlarmPushThread");

    int type = (int)arg;

	int i;
//	SMsgAVIoctrlSetPushReq * pi;

	SMsgAVIoctrlAlarmingReq  alarmReq;
	memset(&alarmReq,0,sizeof(alarmReq));
	alarmReq.AlarmType = type;
	alarmReq.AlarmTime = time(NULL);
	strncpy(alarmReq.AlarmDID, runSystemCfg.deviceInfo.serialNumber, sizeof(runSystemCfg.deviceInfo.serialNumber));

	//if(TRY_LOCK(&PushExecLock) != 0){
	//	PRINT_INFO("alarm pushing thread already running... ...\n");
	//	return NULL;
	//}

    //NEW add 20170520,tutk的消息推送
    if(runMdCfg.handle.is_pushMsg)
    {
        SendPushMessage(AVIOCTRL_EVENT_MOTIONDECT);
        printf("SendPushMessage \n");
    }
	for (i = 0; i < MAX_CLIENT_NUMBER; i++)
	{
	
		if (gClientInfo[i].avIndex < 0)	// 设备不在线
			continue;
		PRINT_INFO("alarm pushing thread check current channel:[%d] is valid.\n",gClientInfo[i].avIndex);

		int ret = avSendIOCtrl(gClientInfo[i].avIndex,
			IOTYPE_USER_IPCAM_ALARMING_REQ,
			(char *)&alarmReq,
			sizeof(SMsgAVIoctrlAlarmingReq));

		if(ret != AV_ER_NoERROR){
			PRINT_INFO("send alarm msg to client failed with error:[%d].\n",ret);
		}
	}
    #if USE_HTS_PROTOCOL
	for (i = 0; i < MAX_CLIENT_NUMBER; i++)
	{
	
		if (gClientInfo[i].avIndex < 0)	// 设备不在线
			continue;
		PRINT_INFO("alarm pushing thread check current channel:[%d] is valid.\n",gClientInfo[i].avIndex);

        tutk_alarm_md_hts(gClientInfo[i].avIndex);
        
	}

    #endif
	GET_LOCK(&PushDevsLock);

	for (i = 0; i < PUSH_MAX; i ++){
		if( (strlen(PushDevs[i].Alias) != 0)
		&&  (strlen(PushDevs[i].AppKey) != 0)
		&&  (strlen(PushDevs[i].Master) != 0)) {
#if DEF_GOOGLE_PUSH
			GooglePush(&PushDevs[i], type, time(NULL));
#else
			//AlarmPush(&PushDevs[i],type, time(NULL));
#endif
		}
	}

	PUT_LOCK(&PushDevsLock);

	

	//PUT_LOCK(&PushExecLock);
	return NULL;
}

static int tutk_is_valid_id(char *id)
{

	int ret ;
	if( id == NULL ||
		id[0] == 0 ||
		0 == strcmp(id,"00000000000000000000"))
	{
		ret = 0;
	}
	else
	{
		ret = 1;
	}

	return ret;
}

static int http_update_push_info(void)
{
	#define HTTP_UPDATE_PUSH_ADDR	"47.98.100.187"

    int socketfd = 0;
    struct sockaddr_in addr;
    char host[64],Header[500],body[128];
	char host_ext[64];
	int port = 80;
	int ret,isGzipMethod = 0;
	char *recv_buf = NULL;
	int recv_len = 2048;;
	int cnt = 0, i;
	//char instr[10] = "iCuteEye";
	char base64UserPsd[128];
	char *uncompressbuf = NULL;
	char url[100];
    struct timeval sendtimout = { 5, 0};
    struct timeval revtimout = { 5, 0};
    cJSON *json = NULL;
	cJSON *arrayItem = NULL;
	char *ptr = NULL;
	unsigned int dest_len;
	SMsgAVIoctrlSetPushReq pushMsgInfo[PUSH_MAX];
	int index = 0;
	int arraySize = 0;

	memset(url,0,sizeof(url));
	memset(host,0,sizeof(host));
	memset(body,0,sizeof(body));
	memset(&pushMsgInfo,0,sizeof(pushMsgInfo));

	if(!strlen(runP2PCfg.user_sign))
		return -2;

	utility_base64_encode(runP2PCfg.user_sign,base64UserPsd,strlen(runP2PCfg.user_sign));
	//printf("base64UserPsd :%s\n",base64UserPsd);
	sprintf(url,"http://dev.vs98.com:2300/v1/device/%s/%s",runSystemCfg.deviceInfo.serialNumber,base64UserPsd);


  	ret = http_parse_url_info(url,host,&port,body);
  	if(ret != 0)
  	{
  		P2P_ERR("parse url failed\n");
		return -1;
	}

	memset(&addr,0,sizeof(addr));
	#if 1
	ret = tutk_resolve_host(&addr.sin_addr,host);
	if(ret != 0)
	{
		addr.sin_addr.s_addr = inet_addr(HTTP_UPDATE_PUSH_ADDR);
		P2P_ERR("Dns %s praser failed!,use %s!\n",host,HTTP_UPDATE_PUSH_ADDR);
	}

	#else

	struct hostent *h;
	if((h=gethostbyname(host)) == NULL)
	{
		addr.sin_addr.s_addr = inet_addr(HTTP_UPDATE_PUSH_ADDR);
		P2P_ERR("Dns %s praser failed!,use %s!\n",host,HTTP_UPDATE_PUSH_ADDR);
	}
	else
	{
		addr.sin_addr.s_addr = inet_addr(inet_ntoa(*((struct in_addr *)h->h_addr)));
	}

	#endif

	cnt = 3;
	while( cnt--)
	{
	    socketfd = socket(AF_INET, SOCK_STREAM, 0);
        if (socketfd < 0) {
    		P2P_ERR("ERROR: create update socket fd error!\n");
    	    continue;
    	}

	    addr.sin_family = AF_INET;
	    addr.sin_port = htons(port);

	    ret = tcp_connect_timedwait(socketfd, (struct sockaddr *)&addr,sizeof(addr),4);
		if(ret != 0)
		{
			P2P_ERR("connect server timeout error\n");
			close(socketfd);
			continue;
		}
		else
			break;
	}

    if(0 > cnt)
        return -1;

	sprintf(host_ext,"%s:%d",host,port);

	setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO,(void *)&revtimout, sizeof(revtimout));
    setsockopt(socketfd, SOL_SOCKET, SO_SNDTIMEO,(void *)&sendtimout, sizeof(sendtimout));

	//utility_base64_encode(instr,base64UserPsd,strlen(instr));

	//printf("base64UserPsd :%s\n",base64UserPsd); //aUN1dGVFeWU=

    sprintf(Header, "GET %s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "authorization: basic aUN1dGU=\r\n"
            "Connection: keep-alive\r\n"
            "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.52 Safari/536.5\r\n"
            "Accept: */*\r\n"
            "Accept-Encoding: gzip,deflate,sdch\r\n"
            "Accept-Language: zh-TW,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n"
            "Accept-Charset: Big5,utf-8;q=0.7,*;q=0.3\r\n"
            "Pragma: no-cache\r\n"
            "Rang: bytes=0-\r\n"
            "Cache-Control: no-cache\r\n"
            "\r\n", body,host_ext);//,base64UserPsd);

    ret = send(socketfd,Header,strlen(Header),0);
	//ret = net_send_data(socketfd,Header,strlen(Header),0);
	if(ret != strlen(Header))
	{
		P2P_ERR("send len :%d, actul len:%d \n",ret,strlen(Header));
		ret = -1;
		goto end;
	}

	recv_len = http_recv_parse_response_header(socketfd,&isGzipMethod);
	if(recv_len == 0)
	{
		P2P_ERR("Response body len error \n");
		goto end;
	}

	recv_buf = malloc(recv_len+4);
	if(recv_buf == NULL)
	{
		P2P_ERR("malloc %d failed \n",recv_len);
		ret = -1;
		goto end;
	}

	memset(recv_buf,0,recv_len+4);

	cnt = 0;
	while(cnt < recv_len)
	{
		//ret = net_recv_data(socketfd,recv_buf+cnt,recv_len-cnt,0);
		ret = recv(socketfd,recv_buf+cnt,recv_len-cnt,0);
		if(ret < 0)
		{

			break;
		}
		else
		{
			cnt += ret;
		}
	}

    if(socketfd)
    {
        close(socketfd);
        socketfd = 0;
    }

	if(cnt != recv_len)
	{
		P2P_ERR("recv body error,ret:%d, recv_len:%d\n",cnt,recv_len);
		ret = -1;
		goto end;
	}
	else
	{

		if(isGzipMethod)
		{
			PRINT_INFO("gzip encoding used in http body!\n");

			dest_len = 10*recv_len;
			uncompressbuf = malloc(dest_len);
			if(!uncompressbuf)
			{
				P2P_ERR("uncompress buffer malloc failed!\n");
				ret = -1;
				goto end;
			}

			memset(uncompressbuf,0,dest_len);
			
			ret = uncompress(uncompressbuf,&dest_len,recv_buf,recv_len);
			if(ret != 0)
			{
				P2P_ERR("uncompress error!\n");
				ret = -1;
				goto end;
			}

			ptr = uncompressbuf;
			//printf("dest_len:%d, %s\n",dest_len,uncompressbuf);
		}
		else
			ptr = recv_buf;

		json = cJSON_Parse((char*)ptr);

		if(json == NULL)
		{
			P2P_ERR("data is not a json format,\n%s\n",uncompressbuf);
			ret = -1;
			goto end;
		}

		cJSON *cloud_item = cJSON_GetObjectItem(json,"cloud_storage");
		if(cloud_item == NULL)
		{
			P2P_ERR("cloud_storage not exist\n");
			//ret = -1;
			//goto end;
		}
		else
		{
			cJSON *tencent_item = cJSON_GetObjectItem(cloud_item,"tencent");
			if(tencent_item == NULL)
			{
				P2P_ERR("tencent not exist\n");
			}
			else
			{

#ifdef USE_APP_XUHUI
				for(i=0;i<PUSH_MAX;i++)
				{
					if(cJSON_GetObjectItem(tencent_item,"endPointUrl")->valuestring)
						strcpy(pushMsgInfo[i].endPointUrl,cJSON_GetObjectItem(tencent_item,"endPointUrl")->valuestring);
					if(cJSON_GetObjectItem(tencent_item,"accessKeyId")->valuestring)
						strcpy(pushMsgInfo[i].accessKeyId,cJSON_GetObjectItem(tencent_item,"accessKeyId")->valuestring);
					if(cJSON_GetObjectItem(tencent_item,"accessKeySecret")->valuestring)
						strcpy(pushMsgInfo[i].accessKeySecret,cJSON_GetObjectItem(tencent_item,"accessKeySecret")->valuestring);
					if(cJSON_GetObjectItem(tencent_item,"appId")->valuestring)
						strcpy(pushMsgInfo[i].appId,cJSON_GetObjectItem(tencent_item,"appId")->valuestring);
					if(cJSON_GetObjectItem(tencent_item,"bucketName")->valuestring)
						strcpy(pushMsgInfo[i].bucketName,cJSON_GetObjectItem(tencent_item,"bucketName")->valuestring);
					if(cJSON_GetObjectItem(tencent_item,"objectName")->valuestring)
						strcpy(pushMsgInfo[i].objectName,cJSON_GetObjectItem(tencent_item,"objectName")->valuestring);
				}
#endif
			}

		}

		cJSON *pushmsg_item = cJSON_GetObjectItem(json,"push_msg");
		if(pushmsg_item == NULL)
		{
			P2P_ERR("push_msg not exist\n");
		}
		else
		{
			cJSON *jg_item = cJSON_GetObjectItem(pushmsg_item,"jg");
			if(jg_item == NULL)
			{
				P2P_ERR("jgpush not exist\n");
			}
			else
			{

				arraySize = cJSON_GetArraySize(jg_item);

				//PRINT_INFO("jg_item arraySize=%d\n", arraySize);

				if(arraySize > PUSH_MAX)
				{
					arraySize = PUSH_MAX;
				}

				for(index = 0; index < arraySize; index++)
				{
					arrayItem = cJSON_GetArrayItem(jg_item, index);
					if(!arrayItem)
					{
						P2P_ERR("cJSON_GetArrayItem error\n");
						continue;
					}

					if(cJSON_GetObjectItem(arrayItem,"appKey")->valuestring)
						strcpy(pushMsgInfo[index].AppKey,cJSON_GetObjectItem(arrayItem,"appKey")->valuestring);
					if(cJSON_GetObjectItem(arrayItem,"secret")->valuestring)
						strcpy(pushMsgInfo[index].Master,cJSON_GetObjectItem(arrayItem,"secret")->valuestring);
				}



			}

			cJSON *fcm_item = cJSON_GetObjectItem(pushmsg_item,"fcm");
			if(fcm_item == NULL)
			{
				P2P_ERR("google push not exist\n");
			}
			else
			{
				arraySize = cJSON_GetArraySize(fcm_item);
				//PRINT_INFO("fcm_item arraySize=%d\n", arraySize);

				if(arraySize > PUSH_MAX)
					arraySize = PUSH_MAX;

				for(index = 0; index < arraySize; index++)
				{
					arrayItem = cJSON_GetArrayItem(fcm_item, index);
					if(!arrayItem)
					{
						P2P_ERR("cJSON_GetArrayItem error\n");
						continue;
					}

					if(cJSON_GetObjectItem(arrayItem,"secret")->valuestring)
						strcpy(pushMsgInfo[index].FcmKey,cJSON_GetObjectItem(arrayItem,"secret")->valuestring);
				}
			}
		}
	}


	GET_LOCK(&PushDevsLock);

	for(i=0;i<PUSH_MAX;i++)
	{
		strcpy(pushMsgInfo[i].Alias,runSystemCfg.deviceInfo.serialNumber);
		pushMsgInfo[i].Type = BELL_MOTION_DETECTIVE;

		/* Alias 没有的情况下直接更新 */
		if(strlen(PushDevs[i].Alias) == 0)
		{
			memcpy(&PushDevs[i], &pushMsgInfo[i], sizeof(SMsgAVIoctrlSetPushReq));
			PRINT_INFO("1. Update push info!\n");
			continue;
		}

		/* Alias 相同的情况下比较KEY是否相同，不相同则更新 */
		if(strcmp(PushDevs[i].Alias,pushMsgInfo[i].Alias) == 0)
		{
			if(strcmp(PushDevs[i].AppKey,pushMsgInfo[i].AppKey) ||
			   strcmp(PushDevs[i].Master,pushMsgInfo[i].Master) ||
			   (strlen(pushMsgInfo[i].FcmKey)!= 0 && strcmp(PushDevs[i].FcmKey,pushMsgInfo[i].FcmKey)))
			{
				memcpy(&PushDevs[i], &pushMsgInfo[i], sizeof(SMsgAVIoctrlSetPushReq));
				PRINT_INFO("2. Update push info!\n");
				continue;
			}
		}

		/* 单独更新cloud 信息 */
#ifdef USE_APP_XUHUI
		if(strlen(pushMsgInfo[i].endPointUrl)!= 0 && strcmp(PushDevs[i].endPointUrl,pushMsgInfo[i].endPointUrl))
		   strcpy(PushDevs[i].endPointUrl,pushMsgInfo[i].endPointUrl);

		if(strlen(pushMsgInfo[i].accessKeyId)!= 0 && strcmp(PushDevs[i].accessKeyId,pushMsgInfo[i].accessKeyId))
		   strcpy(PushDevs[i].accessKeyId,pushMsgInfo[i].accessKeyId);

		if(strlen(pushMsgInfo[i].accessKeySecret)!= 0 && strcmp(PushDevs[i].accessKeySecret,pushMsgInfo[i].accessKeySecret))
		   strcpy(PushDevs[i].accessKeySecret,pushMsgInfo[i].accessKeySecret);

		if(strlen(pushMsgInfo[i].appId)!= 0 && strcmp(PushDevs[i].appId,pushMsgInfo[i].appId))
		   strcpy(PushDevs[i].appId,pushMsgInfo[i].appId);

		if(strlen(pushMsgInfo[i].bucketName)!= 0 && strcmp(PushDevs[i].bucketName,pushMsgInfo[i].bucketName))
		   strcpy(PushDevs[i].bucketName,pushMsgInfo[i].bucketName);

		if(strlen(pushMsgInfo[i].objectName)!= 0 && strcmp(PushDevs[i].objectName,pushMsgInfo[i].objectName))
		   strcpy(PushDevs[i].objectName,pushMsgInfo[i].objectName);

		PRINT_INFO("3. Update push info!\n");
#endif
	}

	PUT_LOCK(&PushDevsLock);

	SavePushDevs();

#if 0
	for(i=0; i<PUSH_MAX; i++)
	{
		printf("########Push Key information begin########\n");
		printf("AppKey:           %s\n",pushMsgInfo[i].AppKey);
		printf("Master:           %s\n",pushMsgInfo[i].Master);
		printf("Alias:            %s\n",pushMsgInfo[i].Alias);
		printf("Type:             %d\n",pushMsgInfo[i].Type);
		printf("FcmKey:           %s\n",pushMsgInfo[i].FcmKey); // for google push
#ifdef USE_APP_XUHUI
		printf("endPointUrl:      %s\n",pushMsgInfo[i].endPointUrl);
		printf("accessKeyId:      %s\n",pushMsgInfo[i].accessKeyId);
		printf("accessKeySecret:  %s\n",pushMsgInfo[i].accessKeySecret);
		printf("appId:            %s\n",pushMsgInfo[i].appId);
		printf("bucketName:       %s\n",pushMsgInfo[i].bucketName);
		printf("objectName:       %s\n",pushMsgInfo[i].objectName);
		printf("########Push Key information end########\n");
#endif
	}
#endif

	ret = 0;

end:
    if(socketfd)
	{
		close(socketfd);
        socketfd = 0;
	}

	if(uncompressbuf)
	{
		free(uncompressbuf);
		uncompressbuf = NULL;
	}

	if(recv_buf)
	{
		free(recv_buf);
		recv_buf = NULL;
	}

	cJSON_Delete(json);

	return ret;

}

static void *load_push_cfg_thread(void *arg)
{	
	int ret = 0;
	char err_cnt = 0;
	
	SET_THREAD_NAME();
	
	while(1)
	{
		if(netcam_net_wifi_getWorkMode() == NETCAM_WIFI_AP && netcam_net_get_detect(runNetworkCfg.lan.netName))
		{
			usleep(500000);
			continue;
		}
		
		ret = http_update_push_info();

		if(ret != 0)
		{
#if 0
			if(ret == -1)
			{
				if(++err_cnt > 15)
					break;
			}
#endif	
			sleep(20);
			continue;
		}
		else
		{
			
			loadPushEndFlg = 1;
			runP2PCfg.PushKeyUpdateFlg = 1;
			P2PCfgSave();
			break;
		}
	}

	return NULL;
}

static void tutk_load_push_cfg_start(void)
{
	int ret ;
	pthread_t download_push_pid;

	if(tutk_is_valid_id(runSystemCfg.deviceInfo.serialNumber) == 0)
	{
		P2P_ERR("ID unvalid,no load push cfg!\n");
		return;
	}

	if(runP2PCfg.PushKeyUpdateFlg == 1)
	{
		ret = LoadPushDevs();
		if (ret != 0)
		{
			goto HTTP_DOWNLOAD;
		}
		else
		{
			loadPushEndFlg = 1;
			return;
		}
	}

HTTP_DOWNLOAD:
	
	if((ret = pthread_create(&download_push_pid, NULL, &load_push_cfg_thread, NULL)))
	{
		P2P_ERR("[p2p] tutk_load_push_cfg_start create fail, ret=[%d]\n", ret);
		return;
	}
}

static int tutk_alarm_md(int nChannel)
{
    int i;
	int push_count = 0;
    pthread_t Thread_ID;

	if(!runP2PCfg.msgPush[nChannel] || !loadPushEndFlg)
	{
		PRINT_INFO("##push msg not enable or push config not load!\n");
		return 0;
	}

	if(tutk_is_valid_id(gUID) == 0)
	{
		P2P_ERR("ID unvalid,not push msg\n");
		return -1;
	}

	time_t ts = time(NULL);

  	PRINT_INFO("Using Ji Guang push\n");
	GET_LOCK(&PushDevsLock);

	for (i = 0; i < PUSH_MAX; i ++)
	{
		if( (strlen(PushDevs[i].Alias) != 0)
		&&  (strlen(PushDevs[i].AppKey) != 0)
		&&  (strlen(PushDevs[i].Master) != 0)) 
		{
			RL_Push_MsgT* push_msg = (RL_Push_MsgT* )malloc(sizeof(RL_Push_MsgT));
			//RL_Push_MsgT* google_push_msg;
			if(push_msg)
			{
				memcpy(&push_msg->pi,&PushDevs[i],sizeof(SMsgAVIoctrlSetPushReq));
				push_msg->type = BELL_MOTION_DETECTIVE;
				push_msg->number = i+1;
				push_msg->channel = nChannel;
				push_msg->ts = ts;
				if (pthread_create(&Thread_ID, NULL, RLAlarmPushThread, (void*)push_msg))
		        {
		            P2P_ERR("RL jpush Thread create fail!,i:%d\n",i);
					free(push_msg);
		            break;
		        }
			}
			else
			{
				P2P_ERR("malloc failed,i:%d\n",i);
				break;
			}

			push_count++;
		}
	}
	PUT_LOCK(&PushDevsLock);
	
	PRINT_INFO("push count:%d",push_count);

	return 0;
}

#else

static void tutk_alarm_md()
{
	SMsgAVIoctrlEvent res;
	time_t timep;
	struct tm *p;
	struct tm t1 = {0};

	timep = time(NULL);
	timep -= runSystemCfg.timezoneCfg.timezone * 60;
    p = localtime_r(&timep, &t1);

	memset(&res, 0, sizeof(SMsgAVIoctrlEvent));
	res.time = timep;
	res.stTime.year = p->tm_year + 1900;
	res.stTime.month = p->tm_mon + 1;
	res.stTime.day = p->tm_mday;
	res.stTime.hour = p->tm_hour;
	res.stTime.minute = p->tm_min;
	res.stTime.second = p->tm_sec;
	res.event = AVIOCTRL_EVENT_MOTIONDECT;

	PRINT_INFO("time:%u, %d-%d-%d %d:%d:%d\n", (unsigned int)timep, res.stTime.year, res.stTime.month, res.stTime.day,
		res.stTime.hour, res.stTime.minute, res.stTime.second);
	int ret = avSendIOCtrl(0, IOTYPE_USER_IPCAM_EVENT_REPORT, (char *)&res, sizeof(SMsgAVIoctrlEvent));
	PRINT_INFO("notifiy motion detect, ret = %d\n", ret);
}
#endif

static int tutk_alarm_cb_func(int nChannel, int nAlarmType, int nAction, void* pParam)
{
	switch(nAlarmType)
    {
        case GK_ALARM_TYPE_ALARMIN:     //0:信号量报警开始
			break;

        case GK_ALARM_TYPE_DISK_FULL:       //1:硬盘满
			break;

        case GK_ALARM_TYPE_VLOST:            //2:信号丢失
			break;

        case GK_ALARM_TYPE_VMOTION:          //3:移动侦测
            tutk_alarm_md(0);
            break;
        case GK_ALARM_TYPE_DISK_UNFORMAT:    //4:硬盘未格式化
			break;

        case GK_ALARM_TYPE_DISK_RWERR:       //5:读写硬盘出错,
			break;

        case GK_ALARM_TYPE_VSHELTER:         //6:遮挡报警
			break;

        case GK_ALARM_TYPE_ALARMIN_RESUME:         //9:信号量报警恢复
			break;

        case GK_ALARM_TYPE_VLOST_RESUME:         //10:视频丢失报警恢复
			break;

        case GK_ALARM_TYPE_VMOTION_RESUME:         //11:视频移动侦测报警恢复
			break;

        case GK_ALARM_TYPE_NET_BROKEN:       //12:网络断开
			break;

        case GK_ALARM_TYPE_IP_CONFLICT:      //13:IP冲突
			break;

        default:
			PRINT_INFO("Alarm: Type = %d", nAlarmType);
        	break;
    }

	return 0;
}

void* tutk_main(void* arg)
{
	int ret , SID;
	void *tutk_alarm_handle = NULL;
	pthread_t ThreadLogin_ID;

	SET_THREAD_NAME();

    /* 初始化 */
    if (InitAVInfo())
    {
        PRINT_ERR("[p2p] InitAVInfo error");
        return NULL;
    }
    
    /* 设置最大连接数 */
	IOTC_Set_Max_Session_Number(MAX_CLIENT_NUMBER);

    /* 设置session超时时间 */
	IOTC_Setup_Session_Alive_Timeout(30);
    
	// use which Master base on location, port 0 means to get a random port
	ret = IOTC_Initialize2(0);
	if(ret != IOTC_ER_NoERROR)
	{
        PRINT_ERR("[p2p] IOTC_Initialize2() error, ret=[%d]\n", ret);
        /* 打印错误信息 */
        PrintErrHandling (ret);
        /* 反初始化 */
        DeInitAVInfo();
        return NULL;
	}
	PRINT_INFO("=============TUTK P2PName_add! begin========\n");
	P2PName_add(MTD_TUTK_P2P);
    PRINT_INFO("=============TUTK P2PName_add! end========\n");
    
	/* 打印 IOTCAPIs & AVAPIs 的版本信息 */
	unsigned int iotcVer;
	IOTC_Get_Version(&iotcVer);
	int avVer = avGetAVApiVer();
	unsigned char *p = (unsigned char *)&iotcVer;
	unsigned char *p2 = (unsigned char *)&avVer;
	char szIOTCVer[16], szAVVer[16];
	sprintf(szIOTCVer, "%d.%d.%d.%d", p[3], p[2], p[1], p[0]);
	sprintf(szAVVer, "%d.%d.%d.%d", p2[3], p2[2], p2[1], p2[0]);
	PRINT_INFO("[p2p] IOTCAPI version[%s] AVAPI version[%s]\n", szIOTCVer, szAVVer);

	IOTC_Get_Login_Info_ByCallBackFn(LoginInfoCB);
	// alloc MAX_CLIENT_NUMBER*3 for every session av data/speaker/play back
	avInitialize(MAX_CLIENT_NUMBER*3);

	if (tutk_load_p2p_id())
    {   
        PRINT_ERR("[p2p] get p2p id error.\n");
		return NULL;
    }

    //ADD 20170520，消息推送需要gUID
	strcpy(gUID, runSystemCfg.deviceInfo.serialNumber);

#if 0
    ret = LoadPushDevs();
    if (ret != 0)
    {
        PRINT_ERR("[p2p] LoadPushDevs error.\n");
    }
#endif 
	// create thread to login because without WAN still can work on LAN
	if((ret = pthread_create(&ThreadLogin_ID, NULL, &thread_Login, (void *)runSystemCfg.deviceInfo.serialNumber)))
	{
		PRINT_ERR("[p2p] Login Thread create fail, ret=[%d]\n", ret);
		return NULL;
	}

    /*  创建发送视频，音频线程 */
	if (create_streamout_thread())
    {
        PRINT_ERR("[p2p] create_streamout_thread error.\n");
        return NULL;
    }

	tutk_load_push_cfg_start();

	//注册报警回调函数
	tutk_alarm_handle = event_alarm_open(tutk_alarm_cb_func);

	//注册private smartlink
	if(netcam_net_private_smartlink())
    {
        PRINT_ERR("TUTK Register private smartlink fail.\n");
        return NULL;
    }


    //开机检查sd卡状态，如果没有格式化则格式化
    
    //create_check_sd_thread();
	//tutk_update_test_start();

	
	while(1)
	{
	    //PRINT_INFO("[p2p] server start listen.");
		// Accept connection only when IOTC_Listen() calling
		
        PRINT_INFO("[p2p] gOnlineNum %d\n",gOnlineNum);
        SID = IOTC_Listen(0);
        if(SID < 0){
            if(SID == IOTC_ER_TIMEOUT){
                //PRINT_INFO("[p2p] IOTC_Listen timeout.");
                sleep(1); 
                continue;
            }

            PrintErrHandling (SID);
            if(SID == IOTC_ER_EXCEED_MAX_SESSION){
                PRINT_INFO("[p2p] IOTC_Listen max clients.");
                sleep(5);
            }
            continue;
        }

		pthread_t Thread_ID;
		if (pthread_create(&Thread_ID, NULL, &thread_ForAVServerStart, (void *)SID))
		{
			PRINT_ERR("pthread_create failed ret[%d]\n", ret);
			continue;
		}

		gOnlineNum++;
        
	}
    event_alarm_close(tutk_alarm_handle);
    
	return 0;
}

int tutklib_start()
{
	pthread_t Thread_ID;
	if (pthread_create(&Thread_ID, NULL, tutk_main, NULL))
	{
		PRINT_ERR("tutklib_start Thread create fail!\n");
		return -1;
	}
    
    PRINT_INFO("tutklib_start end\n");
	return 0;
}


void tutklib_exit()
{
	DeInitAVInfo();
	avDeInitialize();
	IOTC_DeInitialize();
}


