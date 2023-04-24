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
#include "eventalarm.h"
#include "ad_pcm.h"
#include "netcam_api.h"
#include "media_fifo.h"
#include "cfg_user.h"
#include "cfg_video.h"
//#include "cfg_ptz.h"

#include "zfifo.h"
#include "ntp_client.h"
#include "parser_cfg.h"
#include "flash_ctl.h"
#include "work_queue.h"
//#include "mmc_api.h"
//#include "avi_search_api.h"
//#include "avi_utility.h"

#include "doorbell_func.h"

#include "media_fifo.h"
#include "ad_pcm.h"
#ifdef _AUDIO_OPUS_
#include "opus.h"
#endif
#include "ppcs_access_export.h"

//#define SERVTYPE_STREAM_SERVER       0

#define SERVTYPE_VDP_STERAM    0
#define SERVTYPE_IPC_STREAM     1

#define MAX_CLIENT_NUMBER	         8
#define MAX_SIZE_IOCTRL_BUF		     1024



//p2p work statue
#define IDC_WORK_IDLE	0	// 空闲模式
#define IDC_WORK_TALK  	1	// 实时通话模式	
#define IDC_WORK_CALL  	2	// 呼叫相应模式
//#define IDC_WORK_CONF	3	// 系统配置模式
//#define IDC_WORK_LIVE	4	// 实时预览模式
//#define IDC_SHUT_DOWN	5	// 关机


enum{
	//BELL_WITH_WAIT_TIMEOUT,
	BELL_CALL = 1,
	BELL_ALARM,	
	BELL_AGREE
	//BELL_CALL_WAIT_TIMEOUT,
	//BELL_CALL_TIMEOUT,
	//BELL_REST_TICK = 7,
	//BELL_AGREE_ME = 8
};


#define SET_THREAD_NAME() \
    prctl(PR_SET_NAME, (unsigned long)__func__, 0,0,0)

#define AUDIO_FORMAT_PCM

#ifdef AUDIO_FORMAT_PCM
#define AUDIO_FRAME_SIZE 640
#define AUDIO_FPS 25
#define AUDIO_CODEC 0x8C

#elif defined (AUDIO_FORMAT_ADPCM)
#define AUDIO_FRAME_SIZE 160
#define AUDIO_FPS 25
#define AUDIO_CODEC 0x8B

#elif defined (AUDIO_FORMAT_SPEEX)
#define AUDIO_FRAME_SIZE 38
#define AUDIO_FPS 56
#define AUDIO_CODEC 0x8D

#elif defined (AUDIO_FORMAT_MP3)
#define AUDIO_FRAME_SIZE 380
#define AUDIO_FPS 32
#define AUDIO_CODEC 0x8E

#elif defined (AUDIO_FORMAT_SPEEX_ENC)
#define AUDIO_FRAME_SIZE 160
#define AUDIO_ENCODED_SIZE 160
#define AUDIO_FPS 56
#define AUDIO_CODEC 0x8D

#elif defined (AUDIO_FORMAT_G726_ENC)
#define AUDIO_FRAME_SIZE 320
#define AUDIO_ENCODED_SIZE 40
#define AUDIO_FPS 50
#define AUDIO_CODEC 0x8F

#elif defined (AUDIO_FORMAT_G726)
#define AUDIO_FRAME_SIZE 40
#define AUDIO_FPS 50
#define AUDIO_CODEC 0x8F
#endif


#define VIDEO_BUF_SIZE	(1024 * 300)
#define AUDIO_BUF_SIZE	2048

typedef struct _AV_Client
{
	int avIndex;
	unsigned char bEnableAudio;
	unsigned char bEnableVideo;
	unsigned char bEnableSpeaker;
	unsigned char bStopPlayBack;
	unsigned char bPausePlayBack;
	pthread_t Thread_Audio_ID;
	pthread_t Thread_IOCtrl_ID;

	int speakerCh;
	int playBackCh;
	SMsgAVIoctrlPlayRecord playRecord;
	SMsgAVIoctrlPushReq pushInfo;
	pthread_mutex_t mutex;
	
}AV_Client;

pthread_t p2p_Thread_ID = -1;

int g_s32Quit = 0;
int gOnline_status = 0; //记录当前网络状态

pthread_mutex_t gconnect_mutex;
pthread_cond_t gconnect_cond;

static AV_Client gClientInfo[MAX_CLIENT_NUMBER]; //记录会话设备


static pthread_mutex_t gVideoMutex;
static pthread_cond_t  gVideoCond;
static pthread_mutex_t gAudioMutex;
static pthread_cond_t  gAudioCond;
static int gOnlineNum = 0;
static int p2p_work_status = IDC_WORK_IDLE;
static int network_status = 0; //记录当前网络状态
//#define _AUDIO_OPUS_ 1
//#define _AUDIO_OPUS_ENCOD_ 1
#define _AUDIO_ADPCM_ 1
char g_szppcsDefaultInitString[128];

unsigned int audio_alaw_decode(short *dst, const unsigned char *src, unsigned int srcSize);


void PrintErrHandling (int nErr)
{
	switch (nErr)
	{
	case IOTC_ER_SERVER_NOT_RESPONSE :
		//-1 IOTC_ER_SERVER_NOT_RESPONSE
		printf ("[Error code : %d]\n", IOTC_ER_SERVER_NOT_RESPONSE );
		printf ("Master doesn't respond.\n");
		printf ("Please check the network wheather it could connect to the Internet.\n");
		//LED_RED_ON();
		break;
	case IOTC_ER_FAIL_RESOLVE_HOSTNAME :
		//-2 IOTC_ER_FAIL_RESOLVE_HOSTNAME
		printf ("[Error code : %d]\n", IOTC_ER_FAIL_RESOLVE_HOSTNAME);
		printf ("Can't resolve hostname.\n");
		break;
	case IOTC_ER_ALREADY_INITIALIZED :
		//-3 IOTC_ER_ALREADY_INITIALIZED
		printf ("[Error code : %d]\n", IOTC_ER_ALREADY_INITIALIZED);
		printf ("Already initialized.\n");
		break;
	case IOTC_ER_FAIL_CREATE_MUTEX :
		//-4 IOTC_ER_FAIL_CREATE_MUTEX
		printf ("[Error code : %d]\n", IOTC_ER_FAIL_CREATE_MUTEX);
		printf ("Can't create mutex.\n");
		break;
	case IOTC_ER_FAIL_CREATE_THREAD :
		//-5 IOTC_ER_FAIL_CREATE_THREAD
		printf ("[Error code : %d]\n", IOTC_ER_FAIL_CREATE_THREAD);
		printf ("Can't create thread.\n");
		break;
	case IOTC_ER_UNLICENSE :
		//-10 IOTC_ER_UNLICENSE
		printf ("[Error code : %d]\n", IOTC_ER_UNLICENSE);
		printf ("This UID is unlicense.\n");
		printf ("Check your UID.\n");
		break;
	case IOTC_ER_NOT_INITIALIZED :
		//-12 IOTC_ER_NOT_INITIALIZED
		printf ("[Error code : %d]\n", IOTC_ER_NOT_INITIALIZED);
		printf ("Please initialize the IOTCAPI first.\n");
		break;
	case IOTC_ER_TIMEOUT :
		//-13 IOTC_ER_TIMEOUT
		break;
	case IOTC_ER_INVALID_SID :
		//-14 IOTC_ER_INVALID_SID
		printf ("[Error code : %d]\n", IOTC_ER_INVALID_SID);
		printf ("This SID is invalid.\n");
		printf ("Please check it again.\n");
		break;
	case IOTC_ER_EXCEED_MAX_SESSION :
		//-18 IOTC_ER_EXCEED_MAX_SESSION
		printf ("[Error code : %d]\n", IOTC_ER_EXCEED_MAX_SESSION);
		printf ("[Warning]\n");
		printf ("The amount of session reach to the maximum.\n");
		printf ("It cannot be connected unless the session is released.\n");
		break;
	case IOTC_ER_CAN_NOT_FIND_DEVICE :
		//-19 IOTC_ER_CAN_NOT_FIND_DEVICE
		printf ("[Error code : %d]\n", IOTC_ER_CAN_NOT_FIND_DEVICE);
		printf ("Device didn't register on server, so we can't find device.\n");
		printf ("Please check the device again.\n");
		printf ("Retry...\n");
		break;
	case IOTC_ER_SESSION_CLOSE_BY_REMOTE :
		//-22 IOTC_ER_SESSION_CLOSE_BY_REMOTE
		printf ("[Error code : %d]\n", IOTC_ER_SESSION_CLOSE_BY_REMOTE);
		printf ("Session is closed by remote so we can't access.\n");
		printf ("Please close it or establish session again.\n");
		break;
	case IOTC_ER_REMOTE_TIMEOUT_DISCONNECT :
		//-23 IOTC_ER_REMOTE_TIMEOUT_DISCONNECT
		printf ("[Error code : %d]\n", IOTC_ER_REMOTE_TIMEOUT_DISCONNECT);
		printf ("We can't receive an acknowledgement character within a TIMEOUT.\n");
		printf ("It might that the session is disconnected by remote.\n");
		printf ("Please check the network wheather it is busy or not.\n");
		printf ("And check the device and user equipment work well.\n");
		break;
	case IOTC_ER_DEVICE_NOT_LISTENING :
		//-24 IOTC_ER_DEVICE_NOT_LISTENING
		printf ("[Error code : %d]\n", IOTC_ER_DEVICE_NOT_LISTENING);
		printf ("Device doesn't listen or the sessions of device reach to maximum.\n");
		printf ("Please release the session and check the device wheather it listen or not.\n");
		break;
	case IOTC_ER_CH_NOT_ON :
		//-26 IOTC_ER_CH_NOT_ON
		printf ("[Error code : %d]\n", IOTC_ER_CH_NOT_ON);
		printf ("Channel isn't on.\n");
		printf ("Please open it by IOTC_Session_Channel_ON() or IOTC_Session_Get_Free_Channel()\n");
		printf ("Retry...\n");
		break;
	case IOTC_ER_SESSION_NO_FREE_CHANNEL :
		//-31 IOTC_ER_SESSION_NO_FREE_CHANNEL
		printf ("[Error code : %d]\n", IOTC_ER_SESSION_NO_FREE_CHANNEL);
		printf ("All channels are occupied.\n");
		printf ("Please release some channel.\n");
		break;
	case IOTC_ER_TCP_TRAVEL_FAILED :
		//-32 IOTC_ER_TCP_TRAVEL_FAILED
		printf ("[Error code : %d]\n", IOTC_ER_TCP_TRAVEL_FAILED);
		printf ("Device can't connect to Master.\n");
		printf ("Don't let device use proxy.\n");
		printf ("Close firewall of device.\n");
		printf ("Or open device's TCP port 80, 443, 8080, 8000, 21047.\n");
		break;
	case IOTC_ER_TCP_CONNECT_TO_SERVER_FAILED :
		//-33 IOTC_ER_TCP_CONNECT_TO_SERVER_FAILED
		printf ("[Error code : %d]\n", IOTC_ER_TCP_CONNECT_TO_SERVER_FAILED);
		printf ("Device can't connect to server by TCP.\n");
		printf ("Don't let server use proxy.\n");
		printf ("Close firewall of server.\n");
		printf ("Or open server's TCP port 80, 443, 8080, 8000, 21047.\n");
		printf ("Retry...\n");
		break;
	case IOTC_ER_NO_PERMISSION :
		//-40 IOTC_ER_NO_PERMISSION
		printf ("[Error code : %d]\n", IOTC_ER_NO_PERMISSION);
		printf ("This UID's license doesn't support TCP.\n");
		break;
	case IOTC_ER_NETWORK_UNREACHABLE :
		//-41 IOTC_ER_NETWORK_UNREACHABLE
		printf ("[Error code : %d]\n", IOTC_ER_NETWORK_UNREACHABLE);
		printf ("Network is unreachable.\n");
		printf ("Please check your network.\n");
		printf ("Retry...\n");
		break;
	case IOTC_ER_FAIL_SETUP_RELAY :
		//-42 IOTC_ER_FAIL_SETUP_RELAY
		printf ("[Error code : %d]\n", IOTC_ER_FAIL_SETUP_RELAY);
		printf ("Client can't connect to a device via Lan, P2P, and Relay mode\n");
		break;
	case IOTC_ER_NOT_SUPPORT_RELAY :
		//-43 IOTC_ER_NOT_SUPPORT_RELAY
		printf ("[Error code : %d]\n", IOTC_ER_NOT_SUPPORT_RELAY);
		printf ("Server doesn't support UDP relay mode.\n");
		printf ("So client can't use UDP relay to connect to a device.\n");
		break;

	default :
		break;
	}
}


static int AuthCallBackFn(char *viewAcc,char *viewPwd)
{
	printf("viewAcc[%s], viewPwd[%s]\n", viewAcc, viewPwd);
	//if (UserMatching_login(viewAcc, viewPwd) == 1)
		return 1;

	//return 0;
}

static void regedit_client_to_video(int SID, int avIndex)
{
	pthread_mutex_lock(&gVideoMutex);

	AV_Client *p = &gClientInfo[SID];
	//p->avIndex = avIndex;
	p->bEnableVideo = 1;

	pthread_cond_signal(&gVideoCond);
	pthread_mutex_unlock(&gVideoMutex);

	if (p2p_work_status == IDC_WORK_CALL)
	{
		LED_BLUE_ON();

		SMsgAVIoctrlAlarmingReq req;
		int i;
		memset(&req, 0, sizeof(SMsgAVIoctrlAlarmingReq));	
		req.AlarmType = BELL_AGREE;//通知其他未接听用户当前呼叫已接听
		req.AlarmTime = (long long)time(NULL); 
		sprintf(req.AlarmDID,"%s", runSystemCfg.deviceInfo.serialNumber);
		for (i = 0; i < MAX_CLIENT_NUMBER; i++)
		{
			if (gClientInfo[i].avIndex < 0)
				continue;
			avSendIOCtrl(gClientInfo[i].avIndex, IOTYPE_USER_IPCAM_ALARMING_REQ, (char *)&req, sizeof(SMsgAVIoctrlAlarmingReq)); 
			printf("regedit_client_to_video  send IOTYPE_USER_IPCAM_ALARMING_REQ  to %d \n ",gClientInfo[i].avIndex );
		}	

		p2p_work_status = IDC_WORK_TALK;
	}
}

static void unregedit_client_from_video(int SID)
{
	pthread_mutex_lock(&gVideoMutex);

	AV_Client *p = &gClientInfo[SID];
	memset(&p->pushInfo, 0, sizeof(SMsgAVIoctrlPushReq));
	p->bEnableVideo = 0;

	pthread_mutex_unlock(&gVideoMutex);

	if (p2p_work_status == IDC_WORK_TALK)
	{
		CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)netcam_audio_out);
		p2p_work_status = IDC_WORK_IDLE;
	}
}

static void regedit_client_to_audio(int SID, int avIndex)
{
	pthread_mutex_lock(&gAudioMutex);
	
	AV_Client *p = &gClientInfo[SID];
	p->bEnableAudio = 1;

	pthread_cond_signal(&gAudioCond);
	pthread_mutex_unlock(&gAudioMutex);
	
}

static void unregedit_client_from_audio(int SID)
{
	pthread_mutex_lock(&gAudioMutex);
	
	AV_Client *p = &gClientInfo[SID];
	p->bEnableAudio = 0;
        avServStop(p->avIndex);
	pthread_mutex_unlock(&gAudioMutex);
}


#if 1
extern ZFIFO_DESC *writer;
#endif


static void *thread_ReceiveAudio(void *arg)
{
	
	printf("[	****************************************.\n");
	printf("[	thread_ReceiveAudio.\n");
	printf("[	****************************************.\n");
	int SID = (int)arg;
	int avIndex, nResend = 0;
	unsigned int servType, frmNo = 0;
	AV_Client *p;
	char buf[AUDIO_BUF_SIZE];
	char dec[AUDIO_BUF_SIZE*4] = {0};
	char *writebuf = NULL;
    int echo_blocksize = 160;
	int decodesize = 0;
	int  declens = 0;
	
	FRAMEINFO_t frameInfo;
	int error = -1;
	int adpcm_pre_sample = 0;
	struct adpcm_state enc_state ;
	enc_state.valprev = 0;  
	enc_state.index = 0;
#ifdef _AUDIO_OPUS_
	OpusDecoder * hOpusDec = NULL;
	hOpusDec = opus_decoder_create(8000,1,&error);
	if(error != OPUS_OK){
		printf("[thread_ReceiveAudio]opus decoder init failed.\n");
		return -1;
	}
#endif
	SET_THREAD_NAME();
	p = &gClientInfo[SID];
//	avIndex = avClientStart2(SID, NULL, NULL, 30, &servType, p->speakerCh, &nResend);
  avIndex= p->avIndex;
	printf("[thread_ReceiveAudio] start ok! SID[%d], avIndex[%d], bEnableSpeaker[%d]\n", SID, avIndex, p->bEnableSpeaker);

	if (avIndex < 0)
		return 0;

	avStartRecvAudio(avIndex);
	while(p->bEnableSpeaker)
	{
		int ret = avRecvAudioData(avIndex, buf, AUDIO_BUF_SIZE, (char*)&frameInfo, sizeof(FRAMEINFO_t), &frmNo);

		//printf("avRecvAudioData %d \n", ret);
		if(ret == AV_ER_SESSION_CLOSE_BY_REMOTE)
		{
			printf("avRecvAudioData AV_ER_SESSION_CLOSE_BY_REMOTE\n");
			break;
		}
		else if(ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT)
		{
			printf("avRecvAudioData AV_ER_REMOTE_TIMEOUT_DISCONNECT\n");
			break;
		}
		else if(ret == IOTC_ER_INVALID_SID)
		{
			printf("avRecvAudioData Session[%d] cant be used anymore\n", SID);
			break;
		}
		else if(ret == AV_ER_LOSED_THIS_FRAME)
		{
			printf("Audio LOST[%d] /n", frmNo);
			continue;
		}
		else if (ret == AV_ER_DATA_NOREADY)
		{
		    printf("Audio LOST[%d] /n", frmNo);
			usleep(1000);
			continue;
		}
		else if (ret == AV_ER_IOTC_SESSION_CLOSED)
		{
			printf("thread_ReceiveAudio  AV_ER_IOTC_SESSION_CLOSED \n ");
			break;			
		}		
		else if(ret < 0)
		{
			printf("Other error[%d]!!!\n", ret);
			break;
		}

 		switch(frameInfo.codec_id){
			case MEDIA_CODEC_AUDIO_PCM:
#ifdef _AUDIO_DEBUG_
				printf("play audio with format:[%s] lens:[%d].\n","PCM",ret);
#endif
				audiobuf_write_frame(writer,buf,ret);
				break;
			case MEDIA_CODEC_AUDIO_ADPCM:
#ifdef _AUDIO_DEBUG_
				printf("play audio with format:[%s] lens:[%d].\n","ADPCM",ret);
#endif			
				adpcm_decoder(buf,(short *)dec,ret,&enc_state);
				audiobuf_write_frame(writer,dec,ret*4);
#ifdef _AUDIO_DEBUG_
				fwrite(dec,ret*4,1,AudioRecvFile);
#endif
				break;
			case MEDIA_CODEC_AUDIO_OPUS:
#ifdef _AUDIO_OPUS_
				declens = opus_decode(hOpusDec,(const unsigned char *)buf,ret,(opus_int16 *)dec,sizeof(dec),0);
				//printf("play audio with format:[%s] lens:[%d] decode:[%d].\n","OPUS",ret,declens);
#endif
#ifdef _AUDIO_DEBUG_
				printf("play audio with format:[%s] lens:[%d] decode:[%d].\n","OPUS",ret,declens);
#endif
				if(declens <= 0){
					printf("opus decode failed.\n");
					break;
				}
				decodesize = declens*2;
				writebuf = dec;
				if(writer!= NULL)
				{				
					//printf("audiobuf_write_frame %d\n",echo_blocksize);
					audiobuf_write_frame(writer,writebuf,decodesize);							
				}
				
				break;
			default:
#ifdef _AUDIO_DEBUG_
				printf("play audio with format:[%s] lens:[%d].\n","NULL",ret);
#endif
				if(ret>0 && buf!= NULL)
				{
				audiobuf_write_frame(writer,buf,ret);
				}
				break;
		}


	}

	printf("[thread_ReceiveAudio] exit\n");
	//avClientStop(avIndex);
	
#ifdef _AUDIO_OPUS_
	opus_decoder_destroy(hOpusDec);
#endif

	return 0;
}

static int ptz_process(unsigned char chn, unsigned char ptz_cmd, unsigned char speed, unsigned char point)
{
	return 0;
}

static void Handle_IOCTRL_Cmd(int SID, int avIndex, char *buf, int type)
{
	printf("Handle CMD: (0x%X) \n ",type);
	switch(type)
	{
		case IOTYPE_USER_IPCAM_START:
		{
			SMsgAVIoctrlAVStream *p = (SMsgAVIoctrlAVStream *)buf;
			printf("IOTYPE_USER_IPCAM_START, ch:%d, avIndex:%d\n\n", p->channel, avIndex);
			regedit_client_to_video(SID, avIndex);
			printf("regedit_client_to_video OK\n");
		}
		break;
		
		case IOTYPE_USER_IPCAM_STOP:
		{
			SMsgAVIoctrlAVStream *p = (SMsgAVIoctrlAVStream *)buf;
			printf("IOTYPE_USER_IPCAM_STOP, ch:%d, avIndex:%d\n\n", p->channel, avIndex);
			unregedit_client_from_video(SID);
			printf("unregedit_client_from_video OK\n");
		}
		break;
		
		case IOTYPE_USER_IPCAM_AUDIOSTART:
		{
			SMsgAVIoctrlAVStream *p = (SMsgAVIoctrlAVStream *)buf;
			printf("IOTYPE_USER_IPCAM_AUDIOSTART, ch:%d, avIndex:%d\n\n", p->channel, avIndex);
			regedit_client_to_audio(SID, avIndex);
			printf("regedit_client_to_audio OK\n");
		}
		break;
		
		case IOTYPE_USER_IPCAM_AUDIOSTOP:
		{
			SMsgAVIoctrlAVStream *p = (SMsgAVIoctrlAVStream *)buf;
			printf("IOTYPE_USER_IPCAM_AUDIOSTOP, ch:%d, avIndex:%d\n\n", p->channel, avIndex);
			unregedit_client_from_audio(SID);
			printf("unregedit_client_from_audio OK\n");
		}
		break;
		
		case IOTYPE_USER_IPCAM_SPEAKERSTART:
		{
			SMsgAVIoctrlAVStream *p = (SMsgAVIoctrlAVStream *)buf;
			printf("IOTYPE_USER_IPCAM_SPEAKERSTART, SID:%d, avIndex:%d, channel:%d\n\n", SID, avIndex, p->channel);
			gClientInfo[SID].bEnableSpeaker = 1;
			gClientInfo[SID].speakerCh = p->channel;
			
			if(pthread_create(&gClientInfo[SID].Thread_Audio_ID, NULL, &thread_ReceiveAudio, (void *)SID))
				printf("IOTYPE_USER_IPCAM_SPEAKERSTART pthread_create fail\n");
		}
		break;
		
		case IOTYPE_USER_IPCAM_SPEAKERSTOP:
		{
			printf("IOTYPE_USER_IPCAM_SPEAKERSTOP\n\n");
			
			gClientInfo[SID].bEnableSpeaker = 0;

			avServStop(avIndex);
			if(gClientInfo[SID].Thread_Audio_ID != -1) pthread_join(gClientInfo[SID].Thread_Audio_ID,NULL);
			gClientInfo[SID].Thread_Audio_ID = -1;
		}
		break;

        case IOTYPE_USER_IPCAM_SETSTREAMCTRL_REQ:
        {
			//SMsgAVIoctrlSetStreamCtrlReq* p = (SMsgAVIoctrlSetStreamCtrlReq *)buf;
        SMsgAVIoctrlSetStreamCtrlResp res;

			//res.channel = p->channel;
			//res.quality = p->quality;

			memset(&res, 0, sizeof(SMsgAVIoctrlSetStreamCtrlResp));
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETSTREAMCTRL_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetStreamCtrlResq));
        }
		break;

        case IOTYPE_USER_IPCAM_GETSTREAMCTRL_REQ:
        {
			SMsgAVIoctrlGetStreamCtrlReq* p = (SMsgAVIoctrlGetStreamCtrlReq*)buf;
			SMsgAVIoctrlGetStreamCtrlResq res;
			res.channel = p->channel;
			res.quality = AVIOCTRL_QUALITY_MAX;
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETSTREAMCTRL_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetStreamCtrlResq));
        }
		break;

        case IOTYPE_USER_IPCAM_SETMOTIONDETECT_REQ:
        {
            SMsgAVIoctrlSetMotionDetectReq* p = (SMsgAVIoctrlSetMotionDetectReq*)buf;
			SMsgAVIoctrlSetMotionDetectResp res;

			runMdCfg.sensitive = p->sensitivity;
			CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,1,0), (WORK_CALLBACK)MdCfgSave);
			memset(&res, 0, sizeof(SMsgAVIoctrlSetMotionDetectResp));
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETMOTIONDETECT_RESP, (char *)&res, sizeof(SMsgAVIoctrlSetMotionDetectResp));
        }
		break;

        case IOTYPE_USER_IPCAM_GETMOTIONDETECT_REQ:
        {
			SMsgAVIoctrlGetMotionDetectReq* p = (SMsgAVIoctrlGetMotionDetectReq*)buf;
            SMsgAVIoctrlGetMotionDetectResp res;

			res.channel = p->channel;
			res.sensitivity = runMdCfg.sensitive;

			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETMOTIONDETECT_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetMotionDetectResp));
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
			res.free = 0;
			strncpy((char*)res.model, runSystemCfg.deviceInfo.deviceName, sizeof(res.model));
			res.total = 0;
			strncpy((char*)res.vendor, runSystemCfg.deviceInfo.manufacturer, sizeof(res.vendor));
			res.version = 1;
            avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_DEVINFO_RESP, (char *)&res, sizeof(SMsgAVIoctrlDeviceInfoResp));
        }
		break;

        case IOTYPE_USER_IPCAM_SETPASSWORD_REQ:
		{
			SMsgAVIoctrlSetPasswdResp res;
			res.result = 0;
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETPASSWORD_RESP, (char *)&res, sizeof(SMsgAVIoctrlSetPasswdResp));
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

			WifiName = netcam_net_wifi_get_devname();
			if(WifiName == NULL)
				goto LISTWIFIAP;

			if(netcam_net_wifi_isOn() != 1)
				goto LISTWIFIAP;

			netcam_net_wifi_on();
			if(netcam_net_wifi_get_scan_list(list,&number) != 0)
				goto LISTWIFIAP;


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
				res.stWifiAp[i].status = 1;
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

			//设置ssid等参数
			WIFI_LINK_INFO_t linkInfo;
			ST_SDK_NETWORK_ATTR net_attr;

			WifiName = netcam_net_wifi_get_devname();
			if(WifiName == NULL)
				goto SETWIFI;

			netcam_net_wifi_on();

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

			if(netcam_net_wifi_set_connect_info(&linkInfo) == WIFI_CONNECT_OK && linkInfo.linkStatus == WIFI_CONNECT_OK)
			{
				net_attr.dhcp = 1;
				netcam_net_set(&net_attr);
			}

			CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,1,0), (WORK_CALLBACK)NetworkCfgSave);

SETWIFI:
			res.result = 0;
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETWIFI_RESP, (char *)&res, sizeof(SMsgAVIoctrlSetWifiResp));
        }
		break;
		
		case IOTYPE_USER_IPCAM_GETWIFI_REQ:
		{
			SMsgAVIoctrlGetWifiResp2 res;
			WIFI_LINK_INFO_t linkInfo;
			memset(&linkInfo,0,sizeof(linkInfo));
			netcam_net_wifi_get_connect_info(&linkInfo);
			strncpy(res.ssid,linkInfo.linkEssid,sizeof(res.ssid));
			strncpy(res.password,linkInfo.linkPsd,sizeof(res.password));
			switch(linkInfo.linkScurity){
				case 0:
					res.enctype = AVIOTC_WIFIAPENC_NONE;
					break;
				case 1:
				case 2:
				case 3:
				case 4:
					res.enctype = AVIOTC_WIFIAPENC_WEP;
					break;
				case 5:
					res.enctype = AVIOTC_WIFIAPENC_WPA_PSK_TKIP;
					break;
				case 6:
					res.enctype = AVIOTC_WIFIAPENC_WPA_PSK_AES;
					break;
				case 7:
					res.enctype = AVIOTC_WIFIAPENC_WPA2_PSK_TKIP;
					break;
				case 8:
					res.enctype = AVIOTC_WIFIAPENC_WPA2_PSK_AES;
					break;
				default:
					res.enctype = AVIOTC_WIFIAPENC_INVALID;
			}

			res.mode = linkInfo.mode;
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETWIFI_RESP_2, (char *)&res, sizeof(SMsgAVIoctrlGetWifiResp2));
		}
		break;

        case IOTYPE_USER_IPCAM_SETRECORD_REQ:
        /*{
			SMsgAVIoctrlSetRecordReq* p = (SMsgAVIoctrlSetRecordReq*)buf;
			SMsgAVIoctrlSetRecordResp res;

			if (p->recordType == AVIOTC_RECORDTYPE_FULLTIME)
				runRecordCfg.recordMode = 1;
			else if (p->recordType == AVIOTC_RECORDTYPE_MANUAL)
				runRecordCfg.recordMode = 2;

			CREATE_WORK(SetRecord, EVENT_TIMER_WORK, (WORK_CALLBACK)RecordCfgSave);
			INIT_WORK(SetRecord, COMPUTE_TIME(0,0,0,2,0), NULL);
			SCHEDULE_DEFAULT_WORK(SetRecord);

			res.result = 0;
            avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETRECORD_RESP, (char *)&res, sizeof(SMsgAVIoctrlSetRecordResp));
        }*/
		break;

        case IOTYPE_USER_IPCAM_GETRECORD_REQ:
        /*{
            SMsgAVIoctrlGetRecordResq res;

			if (runRecordCfg.recordMode == 1)
				res.recordType = AVIOTC_RECORDTYPE_FULLTIME;
			else if (runRecordCfg.recordMode == 2)
				res.recordType = AVIOTC_RECORDTYPE_MANUAL;
			else
				res.recordType = AVIOTC_RECORDTYPE_OFF;

			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETRECORD_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetRecordResq));
        }*/
		break;

		case IOTYPE_USER_IPCAM_LISTEVENT_REQ:
		/*{
			SMsgAVIoctrlListEventReq *p = (SMsgAVIoctrlListEventReq *)buf;
			SMsgAVIoctrlListEventResp* pres = NULL;
			SMsgAVIoctrlListEventResp res;
			AVI_DMS_TIME avi_start;
			AVI_DMS_TIME avi_stop;
			u64t start;
    		u64t stop;
			FILE_LIST *list;
			int i, j, lst_len, size, count;
			
			printf("IOTYPE_USER_IPCAM_LISTEVENT_REQ\n\n");
			time_convert(&p->stStartTime, runSystemCfg.timezoneCfg.timezone * 60);
			time_convert(&p->stEndTime, runSystemCfg.timezoneCfg.timezone * 60);
			
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

			list = search_file_by_time(0, 0xFF, start, stop);
			if (!list)
				goto LISTEVENT_REQ;
			
			lst_len = get_len_list(list);
			if (lst_len == 0)
				goto LISTEVENT_REQ;

#define EVENT_PACKET_MAX_COUNT 336
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
						printf("IOTYPE_USER_IPCAM_LISTEVENT_REQ Fail to get file node!\n");
						continue;
					}

					u64t_to_time(&avi_start, node.start);

					pres->stEvent[j].stTime.year = avi_start.dwYear;
					pres->stEvent[j].stTime.month = avi_start.dwMonth;
					pres->stEvent[j].stTime.day = avi_start.dwDay;
					pres->stEvent[j].stTime.wday = 0;
					pres->stEvent[j].stTime.hour = avi_start.dwHour;
					pres->stEvent[j].stTime.minute = avi_start.dwMinute;
					pres->stEvent[j].stTime.second = avi_stop.dwSecond;
					pres->stEvent[j].event = AVIOCTRL_EVENT_ALL;
					pres->stEvent[j].status = 0;

					time_convert(&pres->stEvent[j].stTime, -runSystemCfg.timezoneCfg.timezone * 60);
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
			printf("send IOTYPE_USER_IPCAM_LISTEVENT_RESP\n");
		}*/
		break;

		case IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL:
		/*{
			SMsgAVIoctrlPlayRecord *p = (SMsgAVIoctrlPlayRecord *)buf;
			SMsgAVIoctrlPlayRecordResp res;
			
			printf("IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL cmd[%d]\n\n", p->command);
			if(p->command == AVIOCTRL_RECORD_PLAY_START)
			{
				memcpy(&gClientInfo[SID].playRecord, p, sizeof(SMsgAVIoctrlPlayRecord));
				res.command = AVIOCTRL_RECORD_PLAY_START;

				if (gClientInfo[SID].playBackCh < 0)
				{
					gClientInfo[SID].bPausePlayBack = 0;
					gClientInfo[SID].bStopPlayBack = 0;
					gClientInfo[SID].playBackCh = IOTC_Session_Get_Free_Channel(SID);
					res.result = gClientInfo[SID].playBackCh;
				}
				else
					res.result = -1;

				if (res.result >= 0)
				{
					pthread_t ThreadID;
					printf("[%d] start playback\n", res.result);
					
					if(pthread_create(&ThreadID, NULL, &thread_PlayBack, (void *)SID))
					{
						printf("pthread_create thread_PlayBack fail\n");
						res.result = -1;
					}
				}
				else
					printf("Playback on SID %d is still functioning\n", SID);

				avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP, 
					(char *)&res, sizeof(SMsgAVIoctrlPlayRecordResp));
			}
			else if(p->command == AVIOCTRL_RECORD_PLAY_PAUSE)
			{
				gClientInfo[SID].bPausePlayBack = !gClientInfo[SID].bPausePlayBack;
				res.command = AVIOCTRL_RECORD_PLAY_PAUSE;
				res.result = 0;
				
				avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_RECORD_PLAYCONTROL_RESP, (char *)&res, 
					sizeof(SMsgAVIoctrlPlayRecordResp));
			}
			else if(p->command == AVIOCTRL_RECORD_PLAY_STOP)
			{
				gClientInfo[SID].bStopPlayBack = 1;
			}
		}*/
		break;

        case IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_REQ:
        {
			SMsgAVIoctrlGetAudioOutFormatReq* p = (SMsgAVIoctrlGetAudioOutFormatReq*)buf;
			SMsgAVIoctrlGetAudioOutFormatResp res;
			res.channel = p->channel;
			res.format = MEDIA_CODEC_AUDIO_PCM;
			printf("IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_REQ resp MEDIA_CODEC_AUDIO_PCM\n");
      avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GETAUDIOOUTFORMAT_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetAudioOutFormatResp));
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
			else
				stImagingConfig.irCutMode = 0;

			ret = netcam_image_set(stImagingConfig);
    		if (ret != 0)
				goto SET_ENVIRONMENT;

			CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,1,0), (WORK_CALLBACK)ImageCfgSave);
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

			ret = netcam_image_set(stImagingConfig);
    		if (ret != 0)
				goto SET_VIDEOMODE;

			CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,1,0), (WORK_CALLBACK)ImageCfgSave);

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
				res.mode = AVIOCTRL_VIDEOMODE_MIRROR;

GET_VIDEOMODE:
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_VIDEOMODE_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetVideoModeResp));
        }
		break;

        case IOTYPE_USER_IPCAM_FORMATEXTSTORAGE_REQ:
        /*{
			SMsgAVIoctrlFormatExtStorageResp res;

			grd_sd_format();
			res.result = 0;
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_FORMATEXTSTORAGE_RESP, (char *)&res, sizeof(SMsgAVIoctrlFormatExtStorageResp));
        }*/
		break;

        case IOTYPE_USER_IPCAM_PTZ_COMMAND:
        {
            int ret = 0;
			SMsgAVIoctrlPtzCmd *p = (SMsgAVIoctrlPtzCmd *)buf;

            printf("FUN[%s]  LINE[%d]  IOTYPE_USER_IPCAM_PTZ_COMMAND, ch:%d, avIndex:%d\n \n", __FUNCTION__, __LINE__, p->channel, avIndex);

            ret = ptz_process(p->channel, p->control, p->speed, p->point);
            if (ret)
                printf("FUN[%s]  LINE[%d]  Do ptz_process error!\n", __FUNCTION__, __LINE__);
			else
            	printf("FUN[%s]  LINE[%d]  Do IOTYPE_USER_IPCAM_PTZ_COMMAND OK!\n", __FUNCTION__, __LINE__);
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

            printf("FUN[%s]  LINE[%d]  IOTYPE_USER_IPCAM_GET_TIMEZONE_REQ \n", __FUNCTION__, __LINE__);
			memset(&timeZoneRes, 0, sizeof(SMsgAVIoctrlTimeZone));
            timeZoneRes.cbSize              = sizeof(SMsgAVIoctrlTimeZone);
            timeZoneRes.nIsSupportTimeZone  = 1;
            timeZoneRes.nGMTDiff            = runSystemCfg.timezoneCfg.timezone;

			if (runSystemCfg.timezoneCfg.timezone == 480)
				strncpy(timeZoneRes.szTimeZoneString, "People's Republic of China", sizeof(timeZoneRes.szTimeZoneString));

			if (avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_TIMEZONE_RESP, (char *)&timeZoneRes, sizeof(SMsgAVIoctrlTimeZone)) < 0)
                printf("FUN[%s]  LINE[%d]  IOTYPE_USER_IPCAM_GET_TIMEZONE_RESP error!\n", __FUNCTION__, __LINE__);
        }
		break;

        case IOTYPE_USER_IPCAM_SET_TIMEZONE_REQ:
        {
            SMsgAVIoctrlTimeZone *p = (SMsgAVIoctrlTimeZone *)buf;

            printf("FUN[%s]  LINE[%d]  IOTYPE_USER_IPCAM_SET_TIMEZONE_REQ \n", __FUNCTION__, __LINE__);
			printf("cbSize:%d,nGMTDiff:%d,nIsSupportTimeZone:%d,szTimeZoneString:%s\n", 
				p->cbSize, p->nGMTDiff, p->nIsSupportTimeZone, p->szTimeZoneString);

            netcam_sys_set_time_zone_by_utc_second(0, p->nGMTDiff);

            if (avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_TIMEZONE_RESP, (char *)p, sizeof(SMsgAVIoctrlTimeZone)) < 0)
                printf("FUN[%s]  LINE[%d]  IOTYPE_USER_IPCAM_SET_TIMEZONE_RESP error!\n", __FUNCTION__, __LINE__);
        }
		break;

		case IOTYPE_USER_IPCAM_SETPRESET_REQ:
		/*{
			SMsgAVIoctrlSetPresetReq* p = (SMsgAVIoctrlSetPresetReq*)buf;
			SMsgAVIoctrlSetPresetResp res;
			GK_NET_PRESET_INFO gkPresetCfg;

            get_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            gkPresetCfg.nPresetNum++;
            if (gkPresetCfg.nPresetNum >= MAX_PRESET_NUMBER)
            {
                gkPresetCfg.nPresetNum = MAX_PRESET_NUMBER;
            }
            gkPresetCfg.no[gkPresetCfg.nPresetNum-1] = p->nPresetIdx;
            set_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            PresetCruiseCfgSave();
            netcam_ptz_set_preset(p->nPresetIdx, NULL);
			
			res.result = 0;
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SETPRESET_RESP, (char *)&res, sizeof(SMsgAVIoctrlSetPresetResp));
		}*/
		break;

		case IOTYPE_USER_IPCAM_GETPRESET_REQ:
		/*{
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
		}*/
		break;

		case IOTYPE_HICHIP_CRUISE_START:
		/*{
			SMsgAVIoctrlCruiseStart* p = (SMsgAVIoctrlCruiseStart*)buf;

			if (p->mode == AVIOCTRL_CRUISEMODE_VERTICAL)
				netcam_ptz_vertical_cruise(5);
			else if (p->mode == AVIOCTRL_CRUISEMODE_HORIZONTAL)
				netcam_ptz_horizontal_cruise(5);
		}*/
		break;

		case IOTYPE_HICHIP_CRUISE_STOP:
		/*{
			netcam_ptz_stop();
		}*/
		break;

		case IOTYPE_USER_IPCAM_DOOROPEN_REQ:
		{
			SMsgAVIoctrlDoorOpenResp res;
			CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), (WORK_CALLBACK)door_open);
			CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,3,0), (WORK_CALLBACK)door_open);
			
			memset(&res, 0, sizeof(SMsgAVIoctrlDoorOpenResp));
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_DOOROPEN_RESP, (char *)&res, sizeof(SMsgAVIoctrlDoorOpenResp));
		}
		break;
		case IOTYPE_USER_IPCAM_SET_PUSH_REQ: 	// 消息推送注册
		{
			SMsgAVIoctrlSetPushReq* p = (SMsgAVIoctrlSetPushReq*)buf;
			SMsgAVIoctrlSetPushResp res;
			memset(&res, 0, sizeof(SMsgAVIoctrlSetPushResp));
			res.result = SetPushDevParam(p);			
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_PUSH_RESP, (char *)&res, sizeof(SMsgAVIoctrlSetPushResp));
			
 
		}
		break;
		case IOTYPE_USER_IPCAM_DEL_PUSH_REQ:// 消息推送注销
		{
			SMsgAVIoctrlDelPushReq* p = (SMsgAVIoctrlDelPushReq*)buf;
			SMsgAVIoctrlDelPushResp res;
			memset(&res, 0, sizeof(SMsgAVIoctrlDelPushResp));
			res.result = DelPushDevParam(p);	
			if(avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_DEL_PUSH_RESP, (char *)&res, sizeof(SMsgAVIoctrlSetPushResp))<0)
				printf("Error:send IOTYPE_USER_IPCAM_DEL_PUSH_RESP  error!!\n ");
			
		}
		break;
		case IOTYPE_USER_IPCAM_SET_MDP_REQ:// 设置移动侦测参数请求消息
		{
			SMsgAVIoctrlSetMDPReq* p = (SMsgAVIoctrlSetMDPReq*)buf;
			SMsgAVIoctrlSetMDPResp res;
			memset(&res, 0, sizeof(SMsgAVIoctrlDelPushResp));
			res.result = SetMoveAlarmParam(p);			
			if(avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_SET_MDP_RESP, (char *)&res, sizeof(SMsgAVIoctrlSetMDPResp))<0)
				printf("Error:send IOTYPE_USER_IPCAM_SET_MDP_REQ  error!!\n ");
		}
		break;
		case IOTYPE_USER_IPCAM_GET_MDP_REQ:
		{
			SMsgAVIoctrlGetMDPResp res;
			memset(&res, 0, sizeof(SMsgAVIoctrlGetMDPResp));			
			GetMoveAlarmParam(&res);			
			if(avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_MDP_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetMDPResp))<0)
				printf("Error:send IOTYPE_USER_IPCAM_GET_MDP_RESP  error!!\n ");
		}
		break;
		case IOTYPE_USER_IPCAM_GET_SYSPARAM_REQ:
		{	
			SMsgAVIoctrlGetSysparamResp res;
			memset(&res, 0, sizeof(SMsgAVIoctrlGetMDPResp));	
			GetSystemParam(&res);			
			if(avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_GET_SYSPARAM_RESP, (char *)&res, sizeof(SMsgAVIoctrlGetMDPResp))<0)
				printf("Error:send IOTYPE_USER_IPCAM_GET_SYSPARAM_RESP  error!!\n ");
		}
		break;
		

#if 0
		case IOTYPE_USER_IPCAM_PUSH_REQ:
		{
			SMsgAVIoctrlPushReq* p = (SMsgAVIoctrlPushReq*)buf;
			SMsgAVIoctrlPushResp res;

			memcpy(&gClientInfo[SID].pushInfo, p, sizeof(SMsgAVIoctrlPushReq));
			memset(&res, 0, sizeof(SMsgAVIoctrlPushResp));
			
			avSendIOCtrl(avIndex, IOTYPE_USER_IPCAM_PUSH_RESP, (char *)&res, sizeof(SMsgAVIoctrlPushResp));
		}
		break;
#endif

		default:
		printf("avIndex %d: non-handle type[%X]\n", avIndex, type);
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
	AV_Client *p = &gClientInfo[SID];

	SET_THREAD_NAME();
	printf("SID[%d], thread_ForAVServerStart, OK\n", SID);

	int nResend=-1;
	int avIndex = avServStart3(SID, AuthCallBackFn, 0, SERVTYPE_VDP_STERAM, 0, &nResend);
	//int avIndex = avServStart2(SID, AuthCallBackFn, 0, SERVTYPE_STREAM_SERVER, 0);
	//int avIndex = avServStart(SID, "admin", "888888", 0, SERVTYPE_STREAM_SERVER, 0);
	if(avIndex < 0)
	{
		printf("avServStart3 failed!! SID[%d] code[%d]!!!\n", SID, avIndex);
		printf("thread_ForAVServerStart: exit!! SID[%d]\n", SID);
		IOTC_Session_Close(SID);
		gOnlineNum--;
		return 0;
	}
	if(IOTC_Session_Check(SID, &Sinfo) == IOTC_ER_NoERROR)
	{
		char *mode[3] = {"P2P", "RLY", "LAN"};
		// print session information(not a must)
		if(isdigit(Sinfo.RemoteIP[0]))
			printf("Client is from[IP:%s, Port:%d] Mode[%s] VPG[%d:%d:%d] VER[%X] NAT[%d] AES[%d]\n",
			Sinfo.RemoteIP, Sinfo.RemotePort, mode[(int)Sinfo.Mode], Sinfo.VID, Sinfo.PID, Sinfo.GID,
			Sinfo.IOTCVersion, Sinfo.NatType, Sinfo.isSecure);
	}
	printf("avServStart3 OK, avIndex[%d], Resend[%d]\n\n", avIndex, nResend);
	
	p->avIndex = avIndex;

	while(!g_s32Quit)
	{
	   	ioType = -1;
		ret = avRecvIOCtrl(avIndex, &ioType, (char *)&ioCtrlBuf, MAX_SIZE_IOCTRL_BUF, 1000);
		if(ret > 0 && ioType>0)
		{
			Handle_IOCTRL_Cmd(SID, avIndex, ioCtrlBuf, ioType);
		}
		else if(ret != AV_ER_TIMEOUT)
		{
			printf("avIndex[%d], avRecvIOCtrl error, code[%d]\n",avIndex, ret);
			break;
		}

	}

    printf("unregedit_client_from_video\n");
	unregedit_client_from_video(SID);
	printf("unregedit_client_from_audio\n");
	unregedit_client_from_audio(SID);
	//pthread_mutex_lock(&gClientInfo[SID].mutex);
	
    
	if(p->Thread_Audio_ID != -1) 
		{
		pthread_join(p->Thread_Audio_ID,NULL);	
	    p->Thread_Audio_ID = -1;
		}
	usleep(3000);		
	avServStopEx(p->avIndex);
	avServExit(SID,0);
	p->avIndex = -1;
	p->Thread_IOCtrl_ID = -1;
	
	gOnlineNum--;
	//pthread_mutex_unlock(&gClientInfo[SID].mutex);
    printf("SID[%d], avIndex[%d], thread_ForAVServerStart exit!!\n", SID, avIndex);
	return 0;

}

static void *thread_Login(void *arg)
{
	int ret;
	int count = 0;
	  while(!g_s32Quit)
	  {
		  if(count == 0)
			  {   
		   if(gOnline_status == 0)
			  { 	  
			  ret = IOTC_Device_Login(runSystemCfg.deviceInfo.serialNumber, runSystemCfg.deviceInfo.manufacturer, NULL);
			  printf("IOTC_Device_Login() ret = %d\n", ret);
			  if(ret == IOTC_ER_NoERROR)
			  {
					gOnline_status = 1;
				  pthread_mutex_lock(&gconnect_mutex);
				  pthread_cond_signal(&gconnect_cond);
				  pthread_mutex_unlock(&gconnect_mutex);		  
			  }
			  else
			  {
					PrintErrHandling(ret);
			  }
		  }
		  else
		  {
				  printf("PPCS_Listen_Break \n");
	  
				  gOnline_status = 0;
				  IOTC_Listen_Exit();
						  
		  }
		  }
			count+=1;
			if(count)
			{
				if(count>1000*120)
				{
				  count = 0;
				}
			}
			sleep(1000);
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


/********
Thread - Send Audio frames to all AV-idx
*/
static void *thread_AudioFrameData(void *arg)
{
	int i, ret;
	int avIndex, recv_frame_len, new_len;
	char* buffer;
	FRAMEINFO_t frameInfo;
	GK_NET_FRAME_HEADER frame_header;
	MEDIABUF_HANDLE mhandle;

	SET_THREAD_NAME();
	mhandle = mediabuf_add_reader(GK_NET_STREAM_TYPE_STREAM1);
	if (!mhandle)
		return 0;

	printf("thread_AudioFrameData start OK\n");
	printf("[Audio] is ENABLED!!\n");
#if defined(_AUDIO_ADPCM_)
		struct adpcm_state enc_state;
		enc_state.valprev = 0;	 
		enc_state.index = 0;
		char Adpcm[ 4*320] = {0};
#endif

#ifdef _AUDIO_OPUS_ENCOD_
	char hOpuse[ 3*320] = {0};
	char hOpuss[12*320] = {0};

	int nBytesHave = 0;
	int nBytesNeed = 960;
	
	OpusEncoder * hOpusEnc = NULL;
	hOpusEnc = opus_encoder_create(8000, 1, OPUS_APPLICATION_VOIP, &ret);
	if(ret != OPUS_OK){
		printf("opus encoder init failed.\n");
		return 0;
	}

	struct timeval tvs = {0,0};
	struct timeval tve = {0,0};

//	int mscost = 0;
//	int pkgcnt = 0;
#if 0
	opus_encoder_ctl(hOpusEnc, OPUS_SET_BITRATE(24000));
//	opus_encoder_ctl(hOpusEnc, OPUS_SET_BANDWIDTH(OPUS_BANDWIDTH_WIDEBAND));
	opus_encoder_ctl(hOpusEnc, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE)); 
    opus_encoder_ctl(hOpusEnc, OPUS_SET_COMPLEXITY(0)); 
    opus_encoder_ctl(hOpusEnc, OPUS_SET_INBAND_FEC(0)); 
//  opus_encoder_ctl(hOpusEnc, OPUS_SET_DTX(0));  
//  opus_encoder_ctl(hOpusEnc, OPUS_SET_PACKET_LOSS_PERC(0));  
//  opus_encoder_ctl(hOpusEnc, OPUS_GET_LOOKAHEAD(&skip));  
//  opus_encoder_ctl(hOpusEnc, OPUS_SET_LSB_DEPTH(16));  
//	opus_encoder_ctl(hOpusEnc, OPUS_SET_FORCE_CHANNELS(1));
#endif
		opus_encoder_ctl(hOpusEnc, OPUS_SET_BITRATE(16000));
	//	opus_encoder_ctl(hOpusEnc, OPUS_SET_BANDWIDTH(OPUS_BANDWIDTH_WIDEBAND));
		opus_encoder_ctl(hOpusEnc, OPUS_SET_SIGNAL(OPUS_SIGNAL_VOICE)); 
		opus_encoder_ctl(hOpusEnc, OPUS_SET_COMPLEXITY(4));  
		opus_encoder_ctl(hOpusEnc, OPUS_SET_INBAND_FEC(0)); 
	//	opus_encoder_ctl(hOpusEnc, OPUS_SET_DTX(0));  
	//	opus_encoder_ctl(hOpusEnc, OPUS_SET_PACKET_LOSS_PERC(0));  
	//	opus_encoder_ctl(hOpusEnc, OPUS_GET_LOOKAHEAD(&skip));	
	//	opus_encoder_ctl(hOpusEnc, OPUS_SET_LSB_DEPTH(16));  
	//	opus_encoder_ctl(hOpusEnc, OPUS_SET_FORCE_CHANNELS(1));



#endif
    FILE * AudioRecvFile = NULL;
	while(!g_s32Quit)
	{
		pthread_mutex_lock(&gAudioMutex);
		while (!isEnableAudio())
			pthread_cond_wait(&gAudioCond, &gAudioMutex);
		pthread_mutex_unlock(&gAudioMutex);

		buffer = NULL;
		recv_frame_len = 0;
		memset(&frame_header, 0, sizeof(GK_NET_FRAME_HEADER));
		//if(AudioRecvFile == NULL)
		//{
		//		 AudioRecvFile = fopen("/tmp/nfs/audio_recv.pcm","wb");
		//}
    
		ret = mediabuf_read_frame(mhandle, (void**)&buffer, &recv_frame_len, &frame_header);
		if (ret <= 0)
    	{
        	printf("ERROR! read no data.\n");
			continue;
    	}
		if(recv_frame_len<=0)
		{
            continue;
		}

		if (frame_header.frame_type == GK_NET_FRAME_TYPE_A)
		{
					frameInfo.flags = (AUDIO_SAMPLE_8K << 2) | (AUDIO_DATABITS_16 << 1) | AUDIO_CHANNEL_MONO;
		}
		else
		{
			continue;
		}		

		memset(&frameInfo, 0, sizeof(FRAMEINFO_t));
		frameInfo.timestamp = getTimeStamp();
#if defined(_AUDIO_ADPCM_)
				frameInfo.codec_id = MEDIA_CODEC_AUDIO_ADPCM;
#elif defined(_AUDIO_OPUS_ENCOD_)
				frameInfo.codec_id = MEDIA_CODEC_AUDIO_OPUS;
#else
				frameInfo.codec_id = MEDIA_CODEC_AUDIO_PCM;
#endif

       // printf("frameInfo.codec_id = %d ,frame_header.frame_type  =%d ,recv_frame_len = %d \n",frameInfo.codec_id,frame_header.frame_type,recv_frame_len);
		//if(AudioRecvFile!=NULL)
    	//{
    	//	fwrite(buffer,recv_frame_len,1,AudioRecvFile);
    	//}
#if defined(_AUDIO_ADPCM_)
        adpcm_coder((short *)buffer, Adpcm, recv_frame_len/2, &enc_state);
        buffer = Adpcm;
		recv_frame_len/=4;
#endif
#ifdef _AUDIO_OPUS_ENCOD_
		if (recv_frame_len > (sizeof(hOpuss) - nBytesHave)){
			printf("ERROR! no enough space for audio data:[%d][%d].\n",recv_frame_len,nBytesHave);
			continue;
		}

		memcpy(&hOpuss[nBytesHave],buffer,recv_frame_len);
		nBytesHave += recv_frame_len;
		
		if(nBytesHave < nBytesNeed){
			continue;
		}else{
			nBytesHave -= nBytesNeed;
		}

		ret = opus_encode(hOpusEnc,(const opus_int16 *)hOpuss,nBytesNeed/2,(unsigned char *)hOpuse,sizeof(hOpuse));
	    //printf("Opus encode len :[%d].recv_frame_len:[%d] nBytesNeed:[%d]\n",ret,recv_frame_len,nBytesNeed);
		if(ret < 2){
			printf("Opus encode frame failed with error:[%d].\n",ret);
			continue;
		}else{
			recv_frame_len = ret;
		}
        
		buffer = hOpuse;
#endif
   

		for(i = 0 ; i < MAX_CLIENT_NUMBER; i++)
		{
			if(gClientInfo[i].avIndex < 0 || gClientInfo[i].bEnableAudio == 0)
				continue;
			avIndex = gClientInfo[i].avIndex;

			// send audio data to av-idx
     		 ret = avSendAudioData(avIndex, buffer, recv_frame_len, &frameInfo, sizeof(FRAMEINFO_t));
			//printf("avSendAudioData avIndex[%d] size[%d]\n", gClientInfo[i].avIndex, recv_frame_len);
			if(ret == AV_ER_SESSION_CLOSE_BY_REMOTE)
			{
				printf("thread_AudioFrameData: AV_ER_SESSION_CLOSE_BY_REMOTE\n");
				unregedit_client_from_audio(i);
			}
			else if(ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT)
			{
				printf("thread_AudioFrameData: AV_ER_REMOTE_TIMEOUT_DISCONNECT\n");
				unregedit_client_from_audio(i);
			}
			else if(ret == IOTC_ER_INVALID_SID)
			{
				printf("Session cant be used anymore\n");
				unregedit_client_from_audio(i);
			}
			else if(ret < 0)
			{
				printf("avSendAudioData error[%d]\n", ret);
				unregedit_client_from_audio(i);
			}
		}
	}
#ifdef _AUDIO_OPUS_ENCOD_
	if(hOpusEnc!= NULL)
		{
	  opus_encoder_destroy(hOpusEnc);				
		}

#endif
      mediabuf_del_reader(mhandle);
  // if(AudioRecvFile)
  // 	{
  // 		  fclose(AudioRecvFile);
  // 		  AudioRecvFile = NULL;
  // 	}
	return 0;
}

static int isEnableVideo()
{
	int i;
	for(i = 0 ; i < MAX_CLIENT_NUMBER; i++)
	{
		if(gClientInfo[i].bEnableVideo == 1 && gClientInfo[i].avIndex >= 0)
			break;
	}

	return i == MAX_CLIENT_NUMBER ? 0 : 1;
}

/********
Thread - Send Video frames to all AV-idx
*/
static void *thread_VideoFrameData(void *arg)
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
		return 0;

	printf("thread_VideoFrameData start OK\n");
	printf("[Video] is ENABLED!!\n");

	while(!g_s32Quit)
	{
		pthread_mutex_lock(&gVideoMutex);
		while (!isEnableVideo())
			pthread_cond_wait(&gVideoCond, &gVideoMutex);
		pthread_mutex_unlock(&gVideoMutex);

		buffer = NULL;
		recv_frame_len = 0;
		memset(&frame_header, 0, sizeof(GK_NET_FRAME_HEADER));

		ret = mediabuf_read_frame(mhandle, (void**)&buffer, &recv_frame_len, &frame_header);
		if (ret <= 0)
    	{
        	printf("ERROR! read no data.\n");
			continue;
    	}

		memset(&frameInfo, 0, sizeof(FRAMEINFO_t));
		frameInfo.timestamp = getTimeStamp();
		frameInfo.codec_id = MEDIA_CODEC_VIDEO_H264;
		if (frame_header.frame_type == GK_NET_FRAME_TYPE_I)
			frameInfo.flags = IPC_FRAME_FLAG_IFRAME;
		else if (frame_header.frame_type == GK_NET_FRAME_TYPE_P)
			frameInfo.flags = IPC_FRAME_FLAG_PBFRAME;
		else
			continue;

		for(i = 0 ; i < MAX_CLIENT_NUMBER; i++)
		{
			//printf("gClientInfo[%d].avIndex  =%d   bEnableVideo= %d \n",i,gClientInfo[i].avIndex, gClientInfo[i].bEnableVideo);
			if(gClientInfo[i].avIndex < 0 || gClientInfo[i].bEnableVideo == 0)
				continue;
			avIndex = gClientInfo[i].avIndex;

			frameInfo.onlineNum = gOnlineNum;
			ret = avSendFrameData(avIndex, buffer, recv_frame_len, &frameInfo, sizeof(FRAMEINFO_t));
			if(ret == AV_ER_EXCEED_MAX_SIZE)
			{
				printf("thread_VideoFrameData AV_ER_EXCEED_MAX_SIZE SID[%d]\n", i);
				usleep(10000);
				if(avServResetBuffer(avIndex,RESET_VIDEO,10)!=AV_ER_NoERROR)
					printf("avServResetBuffer error!\n");
				else
					printf("avServResetBuffer success!\n");
				continue;
			}
			else if(ret == AV_ER_SESSION_CLOSE_BY_REMOTE)
			{
				printf("thread_VideoFrameData AV_ER_SESSION_CLOSE_BY_REMOTE SID[%d]\n", i);
				unregedit_client_from_video(i);
				continue;
			}
			else if(ret == AV_ER_REMOTE_TIMEOUT_DISCONNECT)
			{
				printf("thread_VideoFrameData AV_ER_REMOTE_TIMEOUT_DISCONNECT SID[%d]\n", i);
				unregedit_client_from_video(i);
				continue;
			}
			else if(ret == IOTC_ER_INVALID_SID)
			{
				printf("Session cant be used anymore\n");
				unregedit_client_from_video(i);
				continue;
			}
			else if(ret < 0)
			{
			      printf("XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX\n");
			      unregedit_client_from_video(i);
				  continue;
				
			}
		}
	}
    mediabuf_del_reader(mhandle);
	return 0;
}

static int InitAVInfo()
{
	int i;
	for(i=0;i<MAX_CLIENT_NUMBER;i++)
	{
		memset(&gClientInfo[i], 0, sizeof(AV_Client));
		gClientInfo[i].avIndex = -1;
		gClientInfo[i].playBackCh = -1;
		gClientInfo[i].Thread_Audio_ID = -1; 
		gClientInfo[i].Thread_IOCtrl_ID = -1;
		gClientInfo[i].bEnableAudio = 0;
		gClientInfo[i].bEnableVideo = 0;
		gClientInfo[i].bEnableSpeaker = 0;
		gClientInfo[i].bStopPlayBack = 0;
		gClientInfo[i].bPausePlayBack = 0;
		pthread_mutex_init(&gClientInfo[i].mutex, NULL);

	}

	if (pthread_mutex_init(&gVideoMutex, NULL) < 0)
		return -1;
	if (pthread_cond_init(&gVideoCond, NULL) < 0)
		return -1;

	if (pthread_mutex_init(&gAudioMutex, NULL) < 0)
		return -1;
	if (pthread_cond_init(&gAudioCond, NULL) < 0)
		return -1;
/*
	PPDevSetupGPIO(0,1,DBC_AUDIO_ENABLE,1);
	PPDevSetupGPIO(0,0,DBC_RESET,0);
	PPDevSetupGPIO(0,1,DBC_R_LED,1);
	PPDevSetupGPIO(0,1,DBC_B_LED,1);
	PPDevSetupGPIO(0,0,DBC_CALL,0);
	PPDevSetupGPIO(0,1,DBC_LOCK,0);
	PPDevSetupGPIO(0,0,DBC_PIR,0);
	*/
	return 0;
}

static void DeInitAVInfo()
{
	int i;
	for(i=0;i<MAX_CLIENT_NUMBER;i++)
	{
	    pthread_mutex_destroy(&gClientInfo[i].mutex);
		memset(&gClientInfo[i], 0, sizeof(AV_Client));
		gClientInfo[i].avIndex = -1;

	}

	pthread_mutex_destroy(&gVideoMutex);
	pthread_cond_destroy(&gVideoCond);
	pthread_mutex_destroy(&gAudioMutex);
	pthread_cond_destroy(&gAudioCond);
}

static int create_streamout_thread()
{
	int ret;
	pthread_t ThreadVideoFrameData_ID;
	pthread_t ThreadAudioFrameData_ID;

	if((ret = pthread_create(&ThreadVideoFrameData_ID, NULL, &thread_VideoFrameData, NULL)))
	{
		printf("pthread_create ret=%d\n", ret);
		return -1;
	}

	if((ret = pthread_create(&ThreadAudioFrameData_ID, NULL, &thread_AudioFrameData, NULL)))
	{
		printf("pthread_create ret=%d\n", ret);
		return -1;
	}

	return 0;
}

static void LoginInfoCB(unsigned int nLoginInfo)
{
	printf("LoginInfoCB  nLoginInfo=0x%X \n",nLoginInfo);
	if(nLoginInfo & 0x04)
	{
		printf("I can be connected via Internet\n");
		LED_BLUE_ON();
	}
	else if(nLoginInfo & 0x08)
		printf("I am be banned by IOTC Server because UID multi-login\n");
}

static int p2p_device_id_load()
{
	struct device_info_mtd info;
	char device_id[32];
	int len = 0;
    if(get_dev_info_from_mtd("/dev/mtd2", 0x1000, &info))
    {
        printf("Fail to load p2p id from /dev/mtd2\n");
        return -1;
    };
	if(strlen(info.device_id) <= 0)
	{
		printf("no p2p device id!\n");
		return -1;
	}
    if(strlen(info.device_id)>0)
  	{
  	sprintf(device_id,"%s",info.device_id);
  	 char *p=strstr(device_id,",");
	 if(p!= NULL)
	 {
	     len = p-device_id;
		 if(len>0)
		 	{
		 memset(runSystemCfg.deviceInfo.serialNumber,0,sizeof(runSystemCfg.deviceInfo.serialNumber));
		 strncpy(runSystemCfg.deviceInfo.serialNumber,device_id,len);
		 p++;
		  sprintf(runSystemCfg.deviceInfo.manufacturer,"%s",p);
		 	}
		 else 
		 	{
			return -1;
		 }
		 
	 }
	 else
	 {
		return -1;
	 }
     	
  	}
	else
	{
		return -1;
	}
    printf(" p2p device id  %s   %s\n",runSystemCfg.deviceInfo.serialNumber,runSystemCfg.deviceInfo.manufacturer);
	if(strlen(info.manufacturer_id)>0)
	{
     sprintf(g_szppcsDefaultInitString,"%s",info.manufacturer_id);
	
	}
	else
	{
		return -1;
	}
	
	SystemCfgSave();

    return 0;

}



static void* AlarmPushThread(void* arg)
{
	int i;
	int type = (int)arg;
	time_t timep;
	time(&timep);

	SMsgAVIoctrlAlarmingReq req;
	memset(&req, 0, sizeof(SMsgAVIoctrlAlarmingReq));	
	req.AlarmType = type;
	req.AlarmTime = (long long)timep; 
	sprintf(req.AlarmDID,"%s", runSystemCfg.deviceInfo.serialNumber);
	for (i = 0; i < MAX_CLIENT_NUMBER; i++)
	{
		if (gClientInfo[i].avIndex < 0)
			continue;
		avSendIOCtrl(gClientInfo[i].avIndex, IOTYPE_USER_IPCAM_ALARMING_REQ, (char *)&req, sizeof(SMsgAVIoctrlAlarmingReq)); 
		printf("send IOTYPE_USER_IPCAM_ALARMING_REQ  to %d \n ",gClientInfo[i].avIndex );
	}
	
	AlarmPush(type);

	return 0;
}


static int tutk_alarm_md()
{
	pthread_t push_thrid;
#if 0
	if(PPDevGetGPIO(DBC_PIR) != 1)
	{
		printf("PIR IS INVALID SIGNAL.\n");
		return -1;
	}


	// 发送推送消息
	if (pthread_create(&push_thrid, NULL, AlarmPushThread, (void*)BELL_ALARM))
	{
		printf("fail to create alarm push thread!\n");
		return -1;
	}
#endif

	return 0;
}

#if 0
static int tutk_alarm_cb_func(int nChannel, int nAlarmType, int nAction, void* pParam)
{
	printf("---->>>tutk_alarm_cb_func  %d \n",nAlarmType);
	switch(nAlarmType)
    {
        case GK_ALARM_TYPE_ALARMIN:     //0:信号量报警开始
			break;

        case GK_ALARM_TYPE_DISK_FULL:       //1:硬盘满
			break;

        case GK_ALARM_TYPE_VLOST:            //2:信号丢失
			break;

        case GK_ALARM_TYPE_VMOTION:          //3:移动侦测
        	{
			int i;
			for(i=0;i<MAX_CLIENT_NUMBER;i++)
			{				
				if((gClientInfo[i].avIndex > 0) && (gClientInfo[i].bEnableVideo == 1)) //有人在视频观看，不报警
					return 0;
			}
			/*
			if(check_alarm_enable())
           		 	tutk_alarm_md();
            */
        	}
        	
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
#endif
static int check_call_timeout(void* arg)
{
	if (p2p_work_status == IDC_WORK_CALL)
	{
		p2p_work_status = IDC_WORK_IDLE;
		netcam_audio_out(AUDIO_FILE_CALLTIMEOUT);
		LED_BLUE_ON();
	}

	return 0;
}

static int play_calling(void* arg)
{
	if (p2p_work_status == IDC_WORK_CALL)
	{
		netcam_audio_out(AUDIO_FILE_CALLING);
		return 1;
	}

	return 0;
}

static void* EnterIOLogic(void* arg)
{
	pthread_t push_thrid;
	while (!g_s32Quit)
	{
	/*
		if(PPDevGetGPIO(DBC_CALL) && p2p_work_status == IDC_WORK_IDLE)
		{
			p2p_work_status = IDC_WORK_CALL;

			LED_RED_ON();
			CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,0,500000), netcam_audio_out, AUDIO_FILE_WELCOME);
			CRTSCH_DEFAULT_WORK(CONDITION_TIMER_WORK, COMPUTE_TIME(0,0,0,3,500000), play_calling, NULL);
			CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,30,0), check_call_timeout, NULL);
			
			// 发送推送消息
			if (pthread_create(&push_thrid, NULL, AlarmPushThread, (void*)BELL_CALL))
			{
				printf("fail to create alarm push thread!\n");
			}
		}

		if(PPDevGetGPIO(DBC_RESET) == 0)
		{
			system("rm -fr /opt/custom/cfg/*");		
			if(get_upgrade_rate() != 0)
			{
				printf("======SYSTEM UPGRADING STATUS GET FAILED======\n");
				continue;
			}
			netcam_audio_out(AUDIO_FILE_RESETOK);
			sleep(6);

			sync();
    			usleep(20);
    			netcam_sys_operation(0,SYSTEM_OPERATION_REBOOT);
		}
     */
		usleep(10000);
	}
	
	return 0;
}


static void* ppcs_main(void* arg)
{
	printf("\n");
	printf("\n");
	printf("	********************************************\n");
	printf("	*		ppcs_main	*\n");
	printf("	********************************************\n");
	printf("\n");
	printf("\n");
	int ret , SID;
//	int tutk_alarm_handle = 0;
	pthread_t ThreadLogin_ID, btns_thread;
	gOnline_status = 0;

	SET_THREAD_NAME();
	if (InitAVInfo()) return 0;
	IOTC_Set_Max_Session_Number(MAX_CLIENT_NUMBER);
	//IOTC_Setup_Session_Alive_Timeout(10);
	// use which Master base on location, port 0 means to get a random port
	ret = IOTC_Initialize2(0,g_szppcsDefaultInitString);
	if(ret != IOTC_ER_NoERROR)
	{
		printf("IOTC_Initialize2(), ret=[%d]\n", ret);
		PrintErrHandling (ret);
		DeInitAVInfo();
		return 0;
	}
	printf("	*******	IOTC_Initialize2*********\n");
  printf("	********************************************\n");
	// Versoin of IOTCAPIs & AVAPIs
	unsigned int iotcVer;
	IOTC_Get_Version(&iotcVer);
	int avVer = avGetAVApiVer();
	unsigned char *p = (unsigned char *)&iotcVer;
	unsigned char *p2 = (unsigned char *)&avVer;
	char szIOTCVer[16], szAVVer[16];
	sprintf(szIOTCVer, "%d.%d.%d.%d", p[3], p[2], p[1], p[0]);
	sprintf(szAVVer, "%d.%d.%d.%d", p2[3], p2[2], p2[1], p2[0]);
	printf("IOTCAPI version[%s] AVAPI version[%s]\n", szIOTCVer, szAVVer);

	IOTC_Get_Login_Info_ByCallBackFn(LoginInfoCB);
	// alloc MAX_CLIENT_NUMBER*3 for every session av data/speaker/play back


	//if (p2p_device_id_load())
	//	return 0;
	loadrunparam();

//	sprintf(runSystemCfg.deviceInfo.serialNumber,"%s","GOHN-000026-EYULR");
//	sprintf(runSystemCfg.deviceInfo.manufacturer,"%s","FKNQVU");
	IOTC_Set_Device_Name(runSystemCfg.deviceInfo.serialNumber);
	avInitialize(MAX_CLIENT_NUMBER*3);

	// create thread to login because without WAN still can work on LAN
	if((ret = pthread_create(&ThreadLogin_ID, NULL, &thread_Login, NULL)))
	{
		printf("Login Thread create fail, ret=[%d]\n", ret);
		return 0;
	}

	if (pthread_create(&btns_thread, NULL, EnterIOLogic, NULL))
	{
		printf("fail to create buttons thread!\n");
		return 0;
	}

	if (create_streamout_thread())
		return 0;

	//注册报警回调函数
//	tutk_alarm_handle = ptapi_open(GK_PT_TUTKP2P);
//    ret = ptapi_ioctrl(tutk_alarm_handle, GK_NET_REG_ALARMCALLBACK, 0, tutk_alarm_cb_func);
//    if(0 != ret)
//        printf("tutk init alarm fail!\n");

   CRTSCH_DEFAULT_WORK(EVENT_TIMER_WORK, COMPUTE_TIME(0,0,0,1,0), (WORK_CALLBACK)netcam_audio_out);
   if(gOnline_status == 0)
   {
   pthread_mutex_lock(&gconnect_mutex);
   pthread_cond_wait(&gconnect_cond, &gconnect_mutex);
   pthread_mutex_unlock(&gconnect_mutex);
   }

  printf("Start  IOTC_Listen!\n");
	while(!g_s32Quit)
	{
		// Accept connection only when IOTC_Listen() calling
		SID = IOTC_Listen(3600);
		if(SID < 0)
		{
			printf("IOTC_Listen %d  %d!\n",SID,g_s32Quit);
			PrintErrHandling (SID);
			if(SID == IOTC_ER_EXCEED_MAX_SESSION)
			{
				sleep(5);
			}
			continue;
		}

		pthread_t Thread_ID;
		if (pthread_create(&Thread_ID, NULL, &thread_ForAVServerStart, (void *)SID))
		{
			printf("pthread_create failed ret[%d]\n", ret);
			continue;
		}
		AV_Client *p = &gClientInfo[SID];
		p->Thread_IOCtrl_ID = Thread_ID;

		gOnlineNum++;
	}
    printf("ppcs_main exit!\n");
	return 0;
}

int ppcs_start()
{
	#if 0
	if (p2p_device_id_load()==-1)
	{
       return 0;
	}
	#endif
	strcpy(runSystemCfg.deviceInfo.serialNumber,"GOHN-000001-GNWZP");
	strcpy(runSystemCfg.deviceInfo.manufacturer,"FBPFFM");
	strcpy(g_szppcsDefaultInitString,"EFGHFDBJKDIHGEJJFDHGFIEHGPJJDJNBGNFLFHCHACMPLAOEGIAPHKODGFPFNJKLBOMEPHCMODJGADGPMFNONHBMMO");
	
	loadrunparam();
	if (pthread_mutex_init(&gconnect_mutex, NULL) < 0)
		return -1;
	if (pthread_cond_init(&gconnect_cond, NULL) < 0)
		return -1;	
	if (pthread_create(&p2p_Thread_ID, NULL, ppcs_main, NULL))
	{
		printf("tutklib_start Thread create fail!\n");
		return -1;
	}
	ppcs_search_daemon_init();
	return 0;
}


void ppcs_exit()
{

    printf("ppcs_exit !\n");
	int i;
	for(i = 0 ; i < MAX_CLIENT_NUMBER; i++)
	{
		if( gClientInfo[i].avIndex >= 0)
		{

			gClientInfo[i].bEnableSpeaker = 0;
			avServStopEx(gClientInfo[i].avIndex);

			if(gClientInfo[i].Thread_Audio_ID != -1) 
			{
			pthread_join(gClientInfo[i].Thread_Audio_ID,NULL);	
		    gClientInfo[i].Thread_Audio_ID = -1;
			}


			if(gClientInfo[i].Thread_IOCtrl_ID != -1) 
			{
			pthread_join(gClientInfo[i].Thread_IOCtrl_ID,NULL);	
		    gClientInfo[i].Thread_IOCtrl_ID = -1;
			}
		}
			
	}
	
    IOTC_Listen_Exit();
	printf("IOTC_Listen_Exit!\n");
	if(p2p_Thread_ID!= -1)
	{
		pthread_join(p2p_Thread_ID,NULL);
		p2p_Thread_ID = -1;
	}
	
    printf("DeInitAVInfo!\n");
	DeInitAVInfo();
	 printf("avDeInitialize!\n");
	avDeInitialize();
	 printf("IOTC_DeInitialize!\n");
	IOTC_DeInitialize();
	pthread_mutex_destroy(&gconnect_mutex);
	pthread_cond_destroy(&gconnect_cond);
	printf("ppcs_exit end !\n");
	ppcs_search_daemon_exit();
}


