#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <arpa/inet.h>
//#define USE_LIBCURL_OPENSSL
#ifdef USE_LIBCURL_OPENSSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "SSLHandle.h"
#include "curl/curl.h"
#else
#include "mbedtls_ssl_api.h"
#include "md5.h"
#endif

//#include "mojingCMD.h"
//#include "adi_sys.h"
#include "mojing.h"
#include "cJSON.h"
#include "netcam_api.h"
#include "cfg_gb28181.h"
#include "flash_ctl.h"
#include "ipcsearch.h"
#include "mmc_api.h"
#include "avi_rec.h"
#include "mojing_cloud_storage.h"
#include "osd.h"

#ifdef MODULE_SUPPORT_WORKQUEUE
#include "work_queue.h"
#endif

#ifdef MODULE_SUPPORT_MOFANG
#include "mofang_plug.h"
#endif

#include "ntp_client.h"
#include "sdk_audio.h"
//#include "adi_types.h"
//#include "adi_audio.h"
#include "utility/util_sock.h"


#ifdef MOJING_USE_PROTOBUF

#include "mojingCMD.h"

#else

#include "pb_encode.h"
#include "pb_decode.h"
#include "mojingCMD.pb.h"
#include "sdk_pwm.h"
#include "utility_api.h"
#include "sdk_isp.h"
//#include "gkvision.h"

#ifdef USE_LIBCURL_OPENSSL
GADI_SYS_SemHandleT libcurLock = 0;
#endif
char mj_gateway[256]="https://open.mj.sctel.com.cn:29995/TerminalManager/DeviceServiceGateWay";
//char mj_gateway[256]="https://222.213.16.53:29995/TerminalManager/DeviceServiceGateWay";

#define SUPPORT_MOJING_V4	 1

#define MOJING_EVENT_UPLOAD_INFO_FILE "gk_mj_event_cfg.cjson"

#ifdef MODULE_SUPPORT_UDP_LOG
#if 0
#define LOG_SERVER_IP	"192.168.10.31"//"61.139.14.245"
#define LOG_SERVER_PORT	20000//2833
#else
#if 1
#define LOG_SERVER_IP	"61.139.14.245"
#define LOG_SERVER_PORT	2833
#else
#define LOG_SERVER_IP	"118.122.233.25"
#define LOG_SERVER_PORT	2834
#endif
#endif

#define SUPPORT_GOKE_LOG
#ifdef SUPPORT_GOKE_LOG
#define LOG_BUFF_SIZE 1300
static int timeCnt = 0;
static char gokeUdpIp[20] = {0};
static char logBuffer[LOG_BUFF_SIZE] = {0};
#define GOKE_UDP_LOP_PORT 50600
#define GOKE_UDP_LOG_ADDR "log.lpwei.com" //log.lpwei.com
static pthread_mutex_t logCache = PTHREAD_MUTEX_INITIALIZER;
#endif

static unsigned long long sn = 1;

static char MjUdpIp[20] = {0};
#define MJ_UDP_LOG_PORT		2833
#define MJ_UDP_LOG_DOMAIN	"gateway.mj.sctel.com.cn"

#endif

static unsigned long long motionAlarmCnt = 0;
static unsigned long long MjEvtSeqNo = 1;
static unsigned long long MjEvtTlsFailCnt = 0;
static unsigned long long MjEvtAlarmCnt = 0;


#ifdef MODULE_SUPPORT_UDP_LOG
static int mojing_log_sn_reset(void);
#endif

extern int gb28181_get_reg_serverip_port(char *ip, int *port);
int netcam_md_alarm_play(char *playFile, int playCnt);


static bool write_string(pb_ostream_t *stream, const pb_field_t *field, void * const *arg)
{
	return pb_encode_tag_for_field(stream, field) && pb_encode_string(stream, *((pb_byte_t **)arg), strlen(*((char **)arg)));
}

static bool read_string(pb_istream_t *stream, const pb_field_t *field, void **arg)
{
	uint8_t *buf = NULL;
	size_t len = stream->bytes_left;
	buf = (uint8_t *)malloc(len+1);
	if (NULL == buf)
		return false;

	memset(buf,0,len+1);

	if (!pb_read(stream, buf, len))
	{
		free(buf);
		return false;
	}

	*arg = (void *)buf;
	return true;
}

#endif


#if 0
int SetupTcpConnection(char* ip, int port)
{
    struct sockaddr_in sin;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(ip);
    sin.sin_port = htons(port);
    if(connect(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        printf("connect server failed.\n");
        close(sockfd);
        return -1;
    }
    return sockfd;
}


SSL* SetupSslConnection(int sockfd)
{
//创建SSL context
    const char* cipher_list = "ECDHE-ECDSA-AES128-GCM-SHA256:ECDHE-RSA-AES128-GCM-SHA256:ECDHE-ECDSA-AES128-SHA256:ECDHE-RSA-AES128-SHA256:ECDHE-ECDSA-AES256-GCM-SHA384:ECDHE-RSA-AES256-GCM-SHA384";
    SSL_METHOD *meth = NULL;
    SSL_CTX* pssl_ctx = NULL;
    int i = 0;
    int seed_int[100]={0};

    OpenSSL_add_ssl_algorithms();
    SSL_load_error_strings();
    meth = (SSL_METHOD *)TLSv1_2_client_method();
    pssl_ctx = SSL_CTX_new(meth);
    if (pssl_ctx == NULL)
    {
		printf();
		return NULL;
    }
    srand((unsigned)time(NULL));
    for (i = 0; i < 100; i++)
    {
        seed_int[i] = rand();
    }
    //RAND_seed(seed_int, sizeof(seed_int));
    SSL_CTX_set_cipher_list(pssl_ctx, cipher_list);
    SSL_CTX_set_mode(pssl_ctx, SSL_MODE_AUTO_RETRY);

//建立SSL连接
    SSL* pssl = SSL_new(pssl_ctx);
    if (SSL_set_fd(pssl, sockfd) <= 0)
    {
        return NULL;
    }
    if(SSL_connect(pssl) <= 0)
    {
        return NULL;
    }

    return pssl_ctx;
}
SSL* mjSslHandle = NULL;

int main(int argc, char *argv[])
{
	int fd ;
	int ret;
	char ip[32]="192.168.10.11";
	int port = 10000;

	if(argc != 3)
	{
		printf("add server and port\n");
		return -1;
	}
	strcpy(ip,argv[1]);
	port = atoi(argv[2]);
	printf("begin connect server %s:%d \n",ip,port);
	fd = SetupTcpConnection(ip,port);
	if(fd < 0)
	{
		printf("connect server %s:%d error\n",ip,port);
		return -1;
	}
	mjSslHandle = SetupSslConnection(fd);
	if(mjSslHandle )
		printf("ssl connect ok\n");
	else
		printf("ssl connect failed\n");

	return 0;
}
#endif
#pragma pack(1)
typedef struct
{
	short header;
	char version;
	short seq;
	short msg_type;
	short msg_len;
}MoJing_HeaderT;
#pragma pack()

#define MJ_PB_BUFFER_LEN   2048
#define MJ_PING_PONG_TIME	30

typedef struct
{
	MoJing_HeaderT mj_header;
#ifdef USE_LIBCURL_OPENSSL
	SSLHandle* ssl;
#else
    void* ssl;
#endif
	int fd;
	char device_id[128];
	char buffer[MJ_PB_BUFFER_LEN];
	int is_regok;// 1 register,0 unregister
	int last_ping_recv_time; // sec
	int last_ping_time;// sec
    int pingTimes;
	int ping_pong_time; // sec
	int exit;
	int server_port;
	char server_addr[128];
	char nonce[64];

	char user_name[32];
	char psd[32];

	int  update_type;
	char update_url[256];
	char update_ver[32];
	char update_md5[64];
	char update_tid[64];

	CloudStorageControlT cloud_storage;

}MoJing_ConnectT;

typedef enum
{
	Msg_RegisterRequest = 1000,
	Msg_RegisterResponseWithNonce = 1001,
	Msg_RegisterRequestWithAuth = 1002,
	Msg_RegisterResponse = 1003,
	Msg_UploadDeviceInfoRequest = 1004,
	Msg_UploadDeviceInfoResponse = 1005,
	Msg_Ping = 1006,
	Msg_Pong = 1007,
	Msg_DeviceUpdateRequest = 2000,
	Msg_DeviceUpdateResponse = 2001,
	Msg_DeviceUpdateNotify = 2002,
	Msg_DownLoadStatRequest = 2003,

	Msg_DeviceFlipRequest = 3004,
	Msg_DeviceFlipResponse = 3005,

    Msg_DeviceGetSDCardInfoRequest = 3006,
    Msg_DeviceGetSDCardInfoResponse = 3007,
	Msg_DeviceSdFormatRequest = 3008,
	Msg_DeviceSdFormatResponse = 3009,
    Msg_DeviceFromatSDCardStatNotify = 3010,

    Msg_GetVideoConfigRequest = 3100,
    Msg_GetVideoConfigResponse = 3101,
    Msg_SetVideoConfigReuqest = 3102,    
	Msg_SetVideoConfigResponse = 3103,

	Msg_GetImageConfigRequest = 3200,
	Msg_GetImageConfigResponse = 3201,
	Msg_SetImageConfigRequest = 3202,
	Msg_SetImageConfigResponse = 3203,

	Msg_OSDInfoRequest = 3204,
	Msg_OSDInfoResponse = 3205,
    Msg_SetOSDConfigRequest = 3206,
    Msg_SetOSDConfigResponse = 3207,

    Msg_GetMDStatusRequest = 3300,
    Msg_GetMDStatusResponse = 3301,
    Msg_SetMDStatusRequest = 3302,
    Msg_SetMDStatusResponse = 3303,

    Msg_GetAreaAlarmStatusRequest = 3304,
    Msg_GetAreaAlarmStatusResponse = 3305,
    Msg_SetAreaAlarmStatusRequest = 3306,
    Msg_SetAreaAlarmStatusResponse = 3307,

    Msg_GetEventAlarmOutRequest = 3308,
    Msg_GetEventAlarmOutResponse = 3309,
    Msg_SetEventAlarmOutRequest = 3310,
    Msg_SetEventAlarmOutResponse = 3311,

    Msg_SetManualAlarmRequest = 3312,
    Msg_SetManualAlarmResponse = 3313,
    Msg_SetAudioAlarmFileRequest = 3314,
    Msg_SetAudioAlarmFileResponse = 3315,
    Msg_AudioAlarmFileUpdateNotify = 3316,

    Msg_GetNightVisionConfigRequest = 3400,
    Msg_GetNightVisionConfigResponse = 3401,
    Msg_SetNightLightTypeRequest = 3402,
    Msg_SetNightLightTypeResponse = 3403,
    Msg_SetLightnessRequest = 3404,
    Msg_SetLightnessResponse = 3405,

	Msg_GetAudioConfigRequest = 3500,
	Msg_GetAudioConfigResponse = 3501,
	Msg_SetAudioConfigRequest = 3502,
	Msg_SetAudioConfigResponse = 3503,

	Msg_GetLocalStorageConfigReuqest = 3600,
	Msg_GetLocalStorageConfigResponse = 3601,
	Msg_SetLocalStorageConfigReuqest = 3602,
	Msg_SetLocalStorageConfigResponse = 3603,

	Msg_GetCloudStorageInfoRequest	= 4000,
	Msg_GetCloudStorageInfoResponse = 4001,
	Msg_CloudStorageInfoChangeNotify= 4002,
	Msg_CloudStorageInfoChangeNotifyOperateResponse				= 4003,
	Msg_GetCloudStorageAddrRequest	= 4004,
	Msg_GetCloudStorageAddrResponse	= 4005,
	Msg_CloudStorageEndNotify		= 4006,
	Msg_CloudStorageOperateResponse	= 4007,
	
	Msg_SetEventUploadRequest  = 5010,
	Msg_SetEventUploadResponse = 5011,

	Msg_GetGBConfigRequest = 5020,
	Msg_GetGBConfigResponse = 5021,
	Msg_UpdateGBConfigRequest = 5022,
	Msg_UpdateGBConfigResponse = 5023,
		
	Msg_GetTimingConfigRequest = 5060,
	Msg_GetTimingConfigResponse = 5061,
	Msg_SetTimingConfigRequest = 5062,
	Msg_SetTimingConfigResponse = 5063,

	Msg_RebootRequest = 5070,
	Msg_RebootResponse = 5071,
	Msg_FactoryDefaultRequest = 5072,
	Msg_FactoryDefaultResponse = 5073,

	Msg_GetNetInfoRequest = 5080,
	Msg_GetNetInfoResponse = 5081,
	Msg_UpdateNetInfoRequest = 5082,
	Msg_UpdateNetInfoResponse = 5083,

    Msg_GetDeviceOnvifRequest = 5090,
    Msg_GetDeviceOnvifResponse = 5091,
    Msg_SetDeviceOnvifRequest = 5092,
    Msg_SetDeviceOnvifResponse = 5093,

}MJ_MsgT;

typedef struct 
{
    char   host[16];
    int    port;
    int    enable;
}MJ_LogUploadT;

#define MOJING_OSD_INFO_FILE "gk_mj_osd_cfg.cjson"
#define OSD_COMPOSITIONS_MAX	6
#define OSD_COMPOSITIONS_ID 	2

typedef struct 
{
	int 	area_id;
	float   x[MAX_VENC_STREAM_NUM];    
	float   y[MAX_VENC_STREAM_NUM];
    int    index[OSD_COMPOSITIONS_MAX];
    char   name[OSD_COMPOSITIONS_MAX][128];
    int    display[OSD_COMPOSITIONS_MAX];
}MJ_OsdCompositionsT;

static MJ_OsdCompositionsT osd_composition;


MoJing_ConnectT* g_connect_handle = NULL;
int SetupTcpConnection(char* ip, int port);
int mojing_device_download_stat(MoJing_ConnectT *handle, int status);
char softVersion[32] = {0};
static int isFirst1004 = 1;
volatile char cloudStorageInProcess = 0;
extern char tsTestIp[];
extern int tsTestPort;
static pthread_mutex_t sslSend = PTHREAD_MUTEX_INITIALIZER;
static MJ_LogUploadT mojingEvtUploadInfo;
static int onvifEnable = 1;

extern int gk_audio_input_enable(bool flag);
extern EM_GK_AUDIO_GainLevelEnumT get_gain_level(int value);
extern int audio_spk_en_control(int enable);
extern int ite_check_test_server(void);
extern int gk_audio_ao_set_volume(int value);
extern void mojing_cloud_storage_get_send_status(unsigned int *totalCnt, unsigned int *successCnt);
extern int goke_upgrade_fresh_now(void);
extern int delete_path(char * path);

#ifdef MODULE_SUPPORT_UDP_LOG
int mojing_log_send(char *devid, int result, char *msg_type, char *msg_body);
int ite_eXosip_get_register_status(void);
#endif



#ifdef USE_LIBCURL_OPENSSL
int mojing_ssl_init(MoJing_ConnectT * handle)
{
	handle->fd = SetupTcpConnection(handle->server_addr,handle->server_port);
	if(handle->fd < 0)
	{
		return -1;
	}
	SSLHandle* pssl = new SSLHandle();//SetupSslConnection(fd);
	if(pssl == NULL)
	{
		close(handle->fd );
		handle->fd = -1;
		printf("ssl connetc error");
		return -1;
	}
	if (pssl->ConnectSocketFd(handle->fd) < 0) {
		printf("ssl connetc error");

		close(handle->fd );
		handle->fd = -1;

		delete ((SSLHandle*)(handle->ssl));
		handle->ssl = 0;

		return -1;
	}


	handle->ssl = pssl;

	return 0;
}

int mojing_ssl_exit(MoJing_ConnectT * handle)
{
 	// 关闭fd和ssl链接，用于设备重连
	if(handle->fd >= 0)
	{
        close(handle->fd);
		handle->fd = 0;
	}
	if (handle->ssl != 0) {
        delete ((SSLHandle*)(handle->ssl));
		handle->ssl = 0;
	}
 	return 0;


}

int mojing_ssl_send(MoJing_ConnectT * handle,char *data,int length,int tm_sec)
{
	int ret = handle->ssl->WriteN(data,length,tm_sec);
	if (ret > 0)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

int mojing_ssl_recv(MoJing_ConnectT * handle,char *data,int length,int tm_sec)
{

	return handle->ssl->ReadN(data,length,tm_sec);

}

static int fileTotalLen = 0;
static char *fileData = NULL;
static int upgrade_write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    //int written = fwrite(ptr, size, nmemb, (FILE *)stream);
    //printf("upgrade_write_data len:%d\n", size * nmemb);
    if(!fileData)
    {
		printf("upgrade_write_data fileData == NULL\n");
		return 0;
    }

    memcpy(fileData + fileTotalLen, ptr, size * nmemb);
    fileTotalLen += size * nmemb;
    return size * nmemb;
}

static int upgrade_header_info(void *ptr, size_t size, size_t nmemb, void *stream)
{
    //int written = fwrite(ptr, size, nmemb, (FILE *)stream);
    printf("upgrade_header_info len:%d\n", size * nmemb);
	printf("headers:%s\n", ptr);
    return size * nmemb;
}

static int upgrade_process(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    if ( dltotal > -0.1 && dltotal < 0.1 )
        return 0;
    int nPos = (int) ( (dlnow/dltotal)*100 );
	printf("get %%%d\n", nPos);
    return 0;
}

/* callback function */
static size_t callback_write_Func(void* buffer, size_t size, size_t nmemb, char *returnJson) {
    printf("callback_write_Func get:%d\n", size * nmemb);
    strcat(returnJson,(char*)buffer);
    return size * nmemb;
}

static void set_share_handle(CURL* curl_handle)
{
    static CURLSH* share_handle = NULL;
    if (!share_handle)
    {
        share_handle = curl_share_init();
        curl_share_setopt(share_handle, CURLSHOPT_SHARE, CURL_LOCK_DATA_DNS);
    }
    curl_easy_setopt(curl_handle, CURLOPT_SHARE, share_handle);
    curl_easy_setopt(curl_handle, CURLOPT_DNS_CACHE_TIMEOUT, 20);
}


int  mojing_send_http_message(const char *sServerAddr, const char *sPostParam, char *returnJson)
{
    CURL *pEasy = curl_easy_init();
    if ( pEasy == NULL ) {
        return -1;
    }

    printf("try to fresh gate from %s\nwith para:\n%s\n", sServerAddr, sPostParam);
	/* 优化dns解析，使curl 保存dns cache */
	set_share_handle(pEasy);

    curl_easy_setopt(pEasy, CURLOPT_URL, sServerAddr);
    curl_easy_setopt(pEasy, CURLOPT_WRITEDATA, returnJson);
    curl_easy_setopt(pEasy, CURLOPT_WRITEFUNCTION, callback_write_Func);
    curl_easy_setopt(pEasy, CURLOPT_TIMEOUT, 20);          //timeout: 30s
    curl_easy_setopt(pEasy, CURLOPT_NOSIGNAL, 1);
    //curl_easy_setopt(pEasy, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
//    curl_easy_setopt(pEasy, CURLOPT_ENCODING, "gzip");
    curl_easy_setopt(pEasy, CURLOPT_POST, 1);
    curl_easy_setopt(pEasy, CURLOPT_POSTFIELDS, sPostParam);
    curl_easy_setopt(pEasy, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(pEasy, CURLOPT_SSL_VERIFYHOST, 0);
    //curl_easy_setopt(pEasy, CURLOPT_VERBOSE, 1);

    CURLcode result = curl_easy_perform(pEasy);
    curl_easy_cleanup(pEasy);
	if(result != 0)
	{
    	printf("result:%d[%s], return json:\n %s\n\n",result, curl_easy_strerror(result), returnJson);
	}
    return result;
}


int mojing_upgrade_get_file(MoJing_ConnectT *handle)
{
	CURL *curl_handle;
	FILE *binFile;
	double fileLen = 0;
	int ret = 0;
	int cnt;
    //char pCaPath[] = "/key.crt";
    char *pCaPath = NULL;

	if(netcam_get_update_status() < 0)
	{
	    printf("#########updating..............\n");
	    return 0;
	}

	printf("upgrade file start:%s\n", handle->update_url);

	//get length
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, handle->update_url);
	curl_easy_setopt(curl_handle, CURLOPT_HEADER, 1);    //只需要header头
	curl_easy_setopt(curl_handle, CURLOPT_NOBODY, 1);    //不需要body

    if (pCaPath == NULL)
	{
		curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 0);//设定为不验证证书和HOST
		curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYHOST, 0);
        printf("not use crt\n");
	}
	else
	{
		curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, 1);
		curl_easy_setopt(curl_handle, CURLOPT_CAINFO, pCaPath);
        printf("not use crt\n");
	}

	ret = curl_easy_perform(curl_handle);
	if (ret == CURLE_OK)
	{
	    ret = curl_easy_getinfo(curl_handle, CURLINFO_CONTENT_LENGTH_DOWNLOAD, &fileLen);
	}
	else
	{
		printf("curl_easy_perform failed:%d\n", ret);
	}
	printf("file len:%d\n", (int)fileLen);

	if(!fileLen)
		return 0;

	netcam_update_relase_system_resource();

    fileTotalLen = 0;
	if (fileData != NULL)
	{
		free(fileData);
		fileData = NULL;
	}

    fileData = (char*)netcam_update_malloc((int)fileLen);
    if (fileData == NULL)
    {
        curl_easy_cleanup(curl_handle);
        printf("malloc:%d failed. upgrade exit.\n", (int)fileLen);
		goto UPGRADE_EXIT;
    }

	curl_easy_setopt(curl_handle, CURLOPT_HEADER, 0);    //只需要header头
	curl_easy_setopt(curl_handle, CURLOPT_NOBODY, 0);
	curl_easy_setopt(curl_handle, CURLOPT_URL, handle->update_url);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, upgrade_write_data);
    curl_easy_perform(curl_handle);
    curl_easy_cleanup(curl_handle);
	printf("down load file over:%d\n", fileTotalLen);
    if (fileTotalLen == (int)fileLen)
    {
        mojing_device_download_stat(g_connect_handle, 200);
        if(netcam_update(fileData, fileTotalLen, NULL) < 0)
        {
            mojing_device_download_stat(g_connect_handle, 400);
			goto UPGRADE_EXIT;
		}
		cnt = 0;
        while (cnt < 99)
        {
            cnt = netcam_update_get_process();
            usleep(218 * 1000);
            PRINT_INFO("upgrade  cnt:%d  ret:%d \n", cnt, ret);
        }

		goto UPGRADE_EXIT;
    }
    else
    {
        mojing_device_download_stat(g_connect_handle, 400);
        free(fileData);
        fileData = NULL;
        printf("get file failed. get %d, file len:%d\n", fileTotalLen, (int)fileLen);
    }

UPGRADE_EXIT:

	usleep(500 * 1000);
	//netcam_exit(90);
	//new_system_call("reboot -f");//force REBOOT
	netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT
    return 0;
}

int mojing_upgrade_save_file(MoJing_ConnectT *handle)
{
	CURL *curl_handle;
	FILE *binFile;
	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, handle->update_url);
	binFile = fopen("/test.bin", "wb");
	printf("down load file start:%s\n", handle->update_url);
	if (binFile)
	{
        /* write the page body to this file handle */
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, binFile);

        /* get it! */
        curl_easy_perform(curl_handle);

        /* close the header file */
        fclose(binFile);
    }

    /* cleanup curl stuff */
    curl_easy_cleanup(curl_handle);
	printf("down load file over\n");
}

int mojing_ssl_wait_event(MoJing_ConnectT * handle,int tm_sec)
{

	return handle->ssl->WaitReadEvent(5);//5S

}

#else
typedef struct
{
    int socket_fd;
    void* ssl;
}MOJING_SSL_HANDLE;

static char http_post_header[] =
{
"POST %s HTTP/1.1\r\n"
"Host: %s\r\n"
"Content-Type: application/x-www-form-urlencoded\r\n"
"Content-Length: %d\r\n"
"\r\n"
"%s"
};

static char http_get_header[] =
    "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Range: bytes=%d-\r\n"
    "Connection: Close\r\n"
    "Accept: */*\r\n"
    "\r\n";



int mojing_ssl_init(MoJing_ConnectT * handle)
{
	int flags;
    char portStr[8] = {0};
#if 0
	struct timeval recv_timeout = {5,0};
	struct timeval send_timeout = {5,0};
#endif
	handle->fd = SetupTcpConnection(handle->server_addr,handle->server_port);
	if(handle->fd < 0)
	{
		return -1;
	}

    sprintf(portStr, "%d", handle->server_port);

    flags = fcntl(handle->fd, F_GETFL, 0);
    fcntl(handle->fd, F_SETFL, flags | O_NONBLOCK);
    int on = 1;
    setsockopt(handle->fd, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));
#if 0
	setsockopt(handle->fd, SOL_SOCKET, SO_RCVTIMEO,(void *)&recv_timeout, sizeof(struct timeval));
    setsockopt(handle->fd, SOL_SOCKET, SO_SNDTIMEO, (void*)&send_timeout, sizeof(struct timeval));
#endif
    void* pssl = gk_mbedtls_ssl_init(handle->fd, handle->server_addr, portStr);
    if(pssl == NULL)
    {
        close(handle->fd);
        handle->fd = -1;
        printf("ssl connetc error");
        return -1;
    }

	handle->ssl = pssl;

	return 0;
}

int mojing_ssl_exit(MoJing_ConnectT * handle)
{
 	// 关闭fd和ssl链接，用于设备重连
	if(handle->fd >= 0)
	{
        close(handle->fd);
		handle->fd = 0;
	}
	if (handle->ssl != 0) {
        gk_mbedtls_ssl_exit(handle->ssl);
		handle->ssl = 0;
	}
 	return 0;


}

int mojing_ssl_send(MoJing_ConnectT * handle,char *data,int length,int tm_sec)
{
    pthread_mutex_lock(&sslSend);
	int ret = gk_mbedtls_ssl_send(handle->ssl, data, length);
    pthread_mutex_unlock(&sslSend);
	return ret;
}

int mojing_ssl_recv(MoJing_ConnectT * handle,char *data,int length,int tm_sec)
{
	return gk_mbedtls_ssl_recv(handle->ssl, data, length);;
}

static int fileTotalLen = 0;
static char *fileData = NULL;

static int mojing_create_request_socket(const char* host, int isSsl)
{
    int sockfd = -1;
    MOJING_SSL_HANDLE *sslHandle = NULL;
    int port = 80;
    char portStr[8] = {0};
    char tmpHost[128] = {0};
    char *portStart = NULL;
    //struct hostent *server;
    struct in_addr server;
	int ret;
    struct sockaddr_in serv_addr;

    printf("host:%s\n", host);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("[http_demo] create_request_socket create socket fail.\n");
        return -1;
    }

    /* lookup the ip address */
    if (strstr(host, ":") != NULL)
    {
        strncpy(tmpHost, host, strlen(host) - strlen(strstr(host, ":")));
        ret = utility_net_resolve_host(&server,tmpHost);
    }
    else
    {
        ret = utility_net_resolve_host(&server,host);
        strcpy(tmpHost, host);
    }
    if(ret == -1)
    {
        printf("[http_demo] create_request_socket gethostbyname fail.\n");
        close(sockfd);
        return -1;
    }

    memset(&serv_addr,0,sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    portStart = strstr(host, ":");
    if (portStart == NULL)
    {
        if (isSsl)
        {
            serv_addr.sin_port = htons(443);
        }
        else
        {
            serv_addr.sin_port = htons(80);
        }
    }
    else
    {
        if (strstr(portStart, "/") != NULL)
        {
            strncpy(portStr, portStart + 1, strlen(portStart) - strlen(strstr(portStart, "/")) - 1);
        }
        else
        {
            strcpy(portStr, portStart + 1);
        }
        port = strtol(portStr, NULL, 0);
        serv_addr.sin_port = htons(port);

    }
    memcpy(&serv_addr.sin_addr,&server,sizeof(server));

    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
    {
        printf("[http_demo] create_request_socket connect fail.\n");
        close(sockfd);
        return -1;
    }

    if (isSsl)
    {
        void* pssl = gk_mbedtls_ssl_init(sockfd, (char*)tmpHost, portStr);
        if(pssl == NULL)
        {
            close(sockfd);
            sockfd = -1;
            printf("ssl connetc error");
            return -1;
        }

        sslHandle = (MOJING_SSL_HANDLE*)malloc(sizeof(MOJING_SSL_HANDLE));
        sslHandle->ssl = pssl;
        sslHandle->socket_fd = sockfd;
        sockfd = (int)sslHandle;
    }
    return sockfd;
}


static int mojing_get_http_content_length(int sock_fd, int isSsl)
{
    int ret;
    int flag =0;
    int recv_len = 0;
    char res_header[1024] = {0};
    MOJING_SSL_HANDLE *sslHandle = (MOJING_SSL_HANDLE *)sock_fd;
    while(recv_len<1023)
    {
        if (isSsl)
        {
            ret = gk_mbedtls_ssl_recv(sslHandle->ssl, res_header+recv_len, 1);
        }
        else
        {
            ret = recv(sock_fd, res_header+recv_len, 1,0);
        }
        if(ret<1)  // recv fail
        {
            break;
        }
        //找到响应头的头部信息, 两个"\r\n"为分割点
        if((res_header[recv_len]=='\r'&&(flag==0||flag==2))||(res_header[recv_len]=='\n'&&(flag==1||flag==3)))
        {
            flag++;
        }
        else
        {
            flag = 0;
        }
        recv_len+=ret;
        if(flag==4)
        {
            break;
        }
    }
    //printf("[http_demo] recv_len=%d res_header = %s.\n",recv_len,res_header);
    /*获取响应头的信息*/
    int status_code=0;
    char content_type[128] = {0};
    int content_length =0;
    char *pos = strstr(res_header, "HTTP/");
    if(pos)
    {
        sscanf(pos, "%*s %d", &status_code);//返回状态码
    }
    if(status_code!=200 && status_code!=206)
    {
        printf("[http_demo] get_content_length status_code = %d\n",status_code);
        return -1;
    }
    pos = strstr(res_header, "Content-Type:");//返回内容类型
    if(pos)
    {
        sscanf(pos, "%*s %s", content_type);
    }
    pos = strstr(res_header, "Content-Length:");//内容的长度(字节)
    if(pos)
    {
        sscanf(pos, "%*s %d", &content_length);
    }
    else
    {
        if (strstr(res_header, "Transfer-Encoding: chunked") != NULL)
        {
            if (isSsl)
            {
                ret = gk_mbedtls_ssl_recv(sslHandle->ssl, res_header+recv_len, 4);
            }
            else
            {
                ret = recv(sock_fd, res_header+recv_len, 4, 0);
            }

            if(ret == 4)
            {
                sscanf(res_header+recv_len, "%x\r\n", &content_length);
            }
        }
    }
    return content_length;
}

/**
 * @brief http_parser_url 根据url解析出host 和 path ; host 和 path 为动态申请的内存，使用完后需要释放
 * @param url
 * @param host
 * @param path
 * @return
 */
static int mojing_http_parser_url(const char* url,char **host,char **path)
{
    if(url == NULL || host == NULL || path == NULL)
    {
         printf("[http_demo] url or host or path is null.\n");
         return -1;
    }

    if(url[0]!='h'||url[1]!='t'||url[2]!='t'||url[3]!='p')
    {
         printf("[http_demo] illegal url = %s.\n",url);
         return -1;
    }
    int host_index = 4;
    const char *temp = url+4;
    if(url[4]=='s')
    {
        temp++;
        host_index++;
    }
    if(*temp++ !=':'||*temp++ !='/'||*temp++ !='/')
    {
        printf("[http_demo] illegal url = %s.\n",url);
        return -1;
    }
    host_index +=3;
    while(*temp!='/')  //next /
    {
        if(*temp == '\0')  //
        {
            printf("[http_demo] illegal url = %s.\n",url);
            return -1;
        }
        temp++;
    }

    int host_len = temp-url-host_index;  //减掉http:// 或者https://
    int path_len = strlen(temp);
    char *host_temp = (char *)malloc(host_len + 1);  //多一个字符串结束标识 \0
    if(host_temp == NULL)
    {
        printf("[http_demo] malloc host fail.\n");
        return -1;
    }
    char *path_temp = (char *)malloc(path_len + 1);  //多一个字符串结束标识 \0
    if(path_temp == NULL)
    {
        printf("[http_demo] malloc path fail.\n");
        free(host_temp);
        return -1;
    }
    memcpy(host_temp,url+host_index,host_len);
    memcpy(path_temp,temp,path_len);
    host_temp[host_len] = '\0';
    path_temp[path_len] = '\0';
    *host = host_temp;
    *path = path_temp;
    return 0;
}

/**
 * @brief http_download_file 下载网络资源
 * @param url    网络资源的位置
 * @param file   保存到本地文件的路径
 * @return       成功返回0,否则失败
 */
static char *mojing_http_download_file(const char* url, int* fileLen, int *revLen, int type, char *sPostParam, int isUpgrade)
{
    int req_fd = -1;
    int isSsl = 0;
    char *host = NULL;
    char *path = NULL;
    int content_len;
    int recv_size = 0;
    int total_size = 0;
    char buff[256] = {0};
    int cpLen = 0;
    char *dataContent = NULL;
    char http_req_content[512] = {0};
    MOJING_SSL_HANDLE *sslHandle = NULL;
    printf("mojing_http_download_file:%s\n", url);
    if(mojing_http_parser_url(url,&host,&path))
    {
         printf("[http_demo] http_parser_url fail.\n");
         return NULL;
    }

    if (strstr(url, "https") != NULL)
    {
        isSsl = 1;
    }
    req_fd = mojing_create_request_socket(host, isSsl);
    if(req_fd < 0)
    {
        printf("[http_demo] read create_request_socket fail fd = %d.\n",req_fd);
        goto download_file_end;
    }

    if (type == 0)
    {
        content_len = snprintf(http_req_content,512, http_get_header, path,  host, 0);
    }
    else
    {
        if (sPostParam != NULL)
        {
            content_len = snprintf(http_req_content,512, http_post_header, path,  host, strlen(sPostParam), sPostParam);
        }
        else
        {
            content_len = snprintf(http_req_content,512, http_post_header, path,  host, 0, "");
        }
    }
    printf("[http_demo] http_req_content = \n%s\n",http_req_content);

    if (isSsl)
    {
        sslHandle = (MOJING_SSL_HANDLE *)req_fd;
        gk_mbedtls_ssl_send(sslHandle->ssl, http_req_content, content_len);
    }
    else
    {
        send(req_fd, http_req_content, content_len,0);
    }

    content_len = mojing_get_http_content_length(req_fd, isSsl);
    if(content_len <1)
    {
        printf("[http_demo] read get_http_content_length fail content_len = %d.\n",content_len);
        goto download_file_end;
    }

    if (fileLen != NULL)
    {
        *fileLen = content_len;
    }
    printf("file len:%d\n", content_len);
    if (isUpgrade)
    {
        dataContent = (char*)netcam_update_malloc(content_len + 1);
    }
    else
    {
        dataContent = (char*)malloc(content_len + 1);
    }
    if (dataContent == NULL)
    {
        printf("download file,malloc %d failed.\n", content_len);
        goto download_file_end;
    }
    dataContent[content_len] = 0;

    while(total_size < content_len)
    {
        if (isSsl)
        {
            recv_size = gk_mbedtls_ssl_recv(sslHandle->ssl, buff, 256);
        }
        else
        {
            recv_size = recv(req_fd, buff, 256, 0);
        }
        if(recv_size < 1)
        {
            printf("[http_demo] recv buff fail\n");
            goto download_file_end;
        }
        cpLen = recv_size;
        if (total_size + recv_size > content_len)
        {
            cpLen = content_len - total_size;
        }
        memcpy(dataContent + total_size, buff, cpLen);
        total_size += cpLen;

    }
    if (revLen != NULL)
    {
        *revLen = total_size;
    }

download_file_end:
    if (isSsl && sslHandle != NULL)
    {
        if (sslHandle->socket_fd > -1)
        {
            close(sslHandle->socket_fd);
        }
        if (sslHandle->ssl != NULL)
        {
            gk_mbedtls_ssl_exit(sslHandle->ssl);
        }
        free(sslHandle);
    }
    else
    {
        if(req_fd > -1)
        {
            close(req_fd);
        }
    }
    if(host != NULL)
    {
        free(host);
    }
    if(path != NULL)
    {
        free(path);
    }
    return dataContent;
}


int  mojing_send_http_message(const char *sServerAddr, const char *sPostParam, char *returnJson)
{
    int ret = -1;
    char *downContent = NULL;
    downContent = mojing_http_download_file(sServerAddr, NULL, NULL, 1, (char*)sPostParam, 0);
    if (downContent != NULL)
    {
        strcpy(returnJson, downContent);
        ret = 0;
        free(downContent);
    }
    return ret;
}


int mojing_upgrade_get_file(MoJing_ConnectT *handle)
{
	int fileLen = 0;
	int ret = 0;
	int cnt;
    int upgradeSuccess = 0;
    int retry = 0;
    //char pCaPath[] = "/key.crt";

	if(netcam_get_update_status() < 0)
	{
	    printf("#########updating..............\n");
	    return 0;
	}

	printf("upgrade file start:%s\n", handle->update_url);

	netcam_update_relase_system_resource();

    while(retry < 3)
    {
        retry++;
        fileTotalLen = 0;
    	if (fileData != NULL)
    	{
    		//free(fileData);
    		fileData = NULL;
    	}

        fileData = mojing_http_download_file(handle->update_url, &fileTotalLen, &fileLen, 0, NULL, 1);
        if (fileTotalLen == (int)fileLen && fileData != NULL && fileTotalLen > 0)
        {
            upgradeSuccess = 1;
            netcam_update_is_save_md5(1);
            if(netcam_update(fileData, fileTotalLen, NULL) < 0)
            {
                mojing_device_download_stat(g_connect_handle, 401);
    			break;
    		}
            
            mojing_device_download_stat(g_connect_handle, 200);
    		cnt = 0;
            while (cnt < 99)
            {
                cnt = netcam_update_get_process();
                usleep(218 * 1000);
                PRINT_INFO("upgrade  cnt:%d  ret:%d \n", cnt, ret);
            }
    		break;
        }
        else
        {
            //free(fileData);
            fileData = NULL;
            printf("get file failed. get %d, file len:%d\n", fileTotalLen, (int)fileLen);
        }
    }

//UPGRADE_EXIT:

    if (upgradeSuccess == 0)
    {
        mojing_device_download_stat(g_connect_handle, 400);
    }
	usleep(500 * 1000);
	//netcam_exit(90);
	//new_system_call("reboot -f");//force REBOOT
	netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT

    return 0;
}

int mojing_ssl_wait_event_type(int fd, int event, int timeout_sec)
{
	int ret = 0;
	fd_set fset;
	struct timeval tv;

	while (1) {
		FD_ZERO(&fset);
		FD_SET(fd, &fset);
		tv.tv_sec = 1;
		tv.tv_usec = 0;

		if (event == 0) {//WAIT_READ
			ret = select(fd + 1, &fset, NULL, NULL, &tv);
		} else if (event == 1) {//WAIT_WRITE
			ret = select(fd + 1, NULL, &fset, NULL, &tv);
		} else {
            printf("WaitEvent: %s sock[%d] event error, %d\n",
                        (event == 0)?"read":"write", fd,
                        event);
		}

 		if (ret > 0)
        {
            if (FD_ISSET(fd, &fset) != 0)
            {
                break;// select ok.
            }
            else
            {
                ret = 0;
            }
		}

 		if (((ret < 0) && (errno != EINTR) && (errno != EAGAIN))) {
            printf("WaitEvent: %s sock[%d] select error, [%d]%s\n",
                        (event == 0)?"read":"write", fd,
                        errno, strerror(errno));
            break;
		}
		if (timeout_sec > 0) {
    		timeout_sec--;
    		if (timeout_sec == 0) {
                //Y_PRINTF("WaitEvent: %s sock[%d] select timeout, %ds\n",
                //        (event == WAIT_READ)?"read":"write", m_sockfd,
                //        times);
                ret = 0;
    		    break;
    		}
        }
	}

	return ret;
}

int mojing_ssl_wait_event(MoJing_ConnectT * handle,int tm_sec)
{

	return mojing_ssl_wait_event_type(handle->fd, 0, 5);//5S
}

#endif

int mj_regrister(MoJing_ConnectT *mj_connect)
{
#ifdef MOJING_USE_PROTOBUF
	goprotobuf::RegisterRequest reg_req;
	MoJing_HeaderT mj_header;

	int ret = 0;
	int sendLen = 0;
	reg_req.set_deviceid(mj_connect->device_id);
	reg_req.set_registertime(time(NULL));
	memset(&mj_connect->mj_header,0,sizeof(MoJing_HeaderT));
	mj_header.header = 0x4D56;
	mj_header.version= 0x01;
	mj_header.seq = 0x01;

	if(!reg_req.SerializeToArray(mj_connect->buffer+ sizeof(MoJing_HeaderT), MJ_PB_BUFFER_LEN - sizeof(MoJing_HeaderT)))
	{
		printf("serialize info error\n");
		return -1;
	}

	mj_header.msg_type = Msg_RegisterRequest;
	mj_header.msg_len = reg_req.ByteSize();

	memcpy(mj_connect->buffer,&mj_header,sizeof(mj_header));
	mj_connect->mj_header = mj_header;
	printf("msg len:%d, ssl send:%d\n", mj_header.msg_len, mj_connect->mj_header.msg_len+sizeof(MoJing_HeaderT));
	ret = mojing_ssl_send(mj_connect,mj_connect->buffer,mj_connect->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
	if(ret == 0)
	{
		printf("Mojing Msg_RegisterRequest send ok\n");
	}
	else
	{
		printf("Mojing Msg_RegisterRequest send failed:%d\n", ret);
	}
#else
	MoJing_HeaderT mj_header;
	int ret = 0;
	uint8_t buffer[2048];
	protobuf_RegisterRequest regReq = protobuf_RegisterRequest_init_default;
	regReq.RegisterTime = time(NULL);
	pb_ostream_t stream;

	memset(&mj_connect->mj_header,0,sizeof(MoJing_HeaderT));
	mj_header.header = 0x4D56;
	mj_header.version= 0x01;
	mj_header.seq = 0x01;

	regReq.DeviceID.funcs.encode = &write_string;
	regReq.DeviceID.arg = mj_connect->device_id;

	stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&stream, protobuf_RegisterRequest_fields, &regReq))
	{
		mj_header.msg_type = Msg_RegisterRequest;
		mj_header.msg_len = stream.bytes_written;
		mj_connect->mj_header = mj_header;
		memcpy(mj_connect->buffer,&mj_header,sizeof(mj_header));
		memcpy(mj_connect->buffer + sizeof(MoJing_HeaderT),buffer,stream.bytes_written);
		ret = mojing_ssl_send(mj_connect,mj_connect->buffer,mj_connect->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("Mojing Msg_RegisterRequest send ok\n");
		}
		else
		{
			printf("Mojing Msg_RegisterRequest send failed\n");
		}

	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n",__FUNCTION__,__LINE__, PB_GET_ERROR(&stream));
		return -1;
	}

#endif

	return ret;
}

int mj_send_ping(MoJing_ConnectT *handle)
{
#ifdef MOJING_USE_PROTOBUF
	goprotobuf::Ping req;
	MoJing_HeaderT mj_header;

	int ret = 0;
	int sendLen = 0;

	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

	req.set_time(time(NULL));

	if(!req.SerializeToArray(handle->buffer+ sizeof(MoJing_HeaderT), MJ_PB_BUFFER_LEN - sizeof(MoJing_HeaderT)))
	{
		printf("serialize info error\n");
		return -1;
	}

	mj_header.msg_type = Msg_Ping;
	mj_header.msg_len = req.ByteSize();

	handle->mj_header = mj_header;
	memcpy(handle->buffer,&mj_header,sizeof(mj_header));
	ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
	if(ret == 0)
	{
		printf("Mojing Msg_Ping send ok\n");
	}
	else
	{
		printf("Mojing Msg_Ping send failed\n");
	}
#else
	MoJing_HeaderT mj_header;
	int ret = 0;
	uint8_t buffer[1024];
	protobuf_Ping ping = protobuf_Ping_init_zero;
	ping.Time = time(NULL);
	pb_ostream_t stream;
	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

	stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&stream, protobuf_Ping_fields, &ping))
	{
		mj_header.msg_type = Msg_Ping;
		mj_header.msg_len = stream.bytes_written;
		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,stream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("Mojing Msg_Ping send ok\n");
            handle->pingTimes++;
		}
		else
		{
			printf("Mojing Msg_Ping send failed\n");
		}

	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&stream));
		return -1;
	}

#endif
	return ret;
}


void mojing_md5_cal(char *input,int len, char *output)
{
    MD5_CTX c;
	unsigned char md5_res[16];
    #ifdef USE_LIBCURL_OPENSSL
    MD5_Init(&c);

	MD5_Update(&c, input, len);
    MD5_Final(md5_res, &c);
    #else
    MD5Init(&c);

	MD5Update(&c, (unsigned char*)input, len);
    MD5Final(&c, md5_res);
    #endif

	snprintf(output,64,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
	md5_res[0],md5_res[1],md5_res[2],md5_res[3],
	md5_res[4],md5_res[5],md5_res[6],md5_res[7],
	md5_res[8],md5_res[9],md5_res[10],md5_res[11],
	md5_res[12],md5_res[13],md5_res[14],md5_res[15]
    );/**/
	#if 0
	char md5_res[16];
	MD5_CTX md5;
	MD5Init(&md5);

	//unsigned char encrypt[] ="admin";//21232f297a57a5a743894a0e4a801fc3
	//unsigned char decrypt[16];
	MD5Update(&md5,input,len);
	MD5Final(&md5,md5_res);

	snprintf(output,64,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
	md5_res[0],md5_res[1],md5_res[2],md5_res[3],
	md5_res[4],md5_res[5],md5_res[6],md5_res[7],
	md5_res[8],md5_res[9],md5_res[10],md5_res[11],
	md5_res[12],md5_res[13],md5_res[14],md5_res[15]
    );
	#endif

}

int mojing_register_request_with_auth(MoJing_ConnectT *handle)
{
#ifdef MOJING_USE_PROTOBUF
	goprotobuf::RegisterRequestWithAuth req;
	MoJing_HeaderT mj_header;
	char md5_str[256];
	char md5_res[16];
	char md5_res_str[64]={'\0'};


	int ts = time(NULL);
	int ret = 0;
	int sendLen = 0;

	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

	memset(md5_str,0,sizeof(md5_str));
	req.set_registertime(ts);
	req.set_deviceid(handle->device_id);
	req.set_pingspan(handle->ping_pong_time);

	//auth string :  MD5(MD5(用户名+":"+密码)+ ":"+nonce)
	printf("username:%s,%s\n", handle->user_name,handle->psd);

	sprintf(md5_str,"%s:%s",handle->user_name,handle->psd);
	mojing_md5_cal(md5_str,strlen(md5_str),md5_res_str);
	printf("md51:\n%s\n", md5_res_str);
	sprintf(md5_str,"%s:%s",md5_res_str,handle->nonce);
	mojing_md5_cal(md5_str,strlen(md5_str),md5_res_str);
	printf("md52:\n%s\n", md5_res_str);

	req.set_auth(md5_res_str);

	if(!req.SerializeToArray(handle->buffer+ sizeof(MoJing_HeaderT), MJ_PB_BUFFER_LEN - sizeof(MoJing_HeaderT)))
	{
		printf("serialize info error\n");
		return -1;
	}

	mj_header.msg_type = Msg_RegisterRequestWithAuth;
	mj_header.msg_len = req.ByteSize();

	handle->mj_header = mj_header;
	memcpy(handle->buffer,&mj_header,sizeof(mj_header));
	ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
	if(ret == 0)
	{
		printf("Mojing Msg_RegisterRequestWithAuth send %d ok\n", handle->mj_header.msg_len);
	}
	else
	{
		printf("Mojing Msg_RegisterRequestWithAuth send failed\n");
	}
#else
	MoJing_HeaderT mj_header;
	char md5_str[256];
	//char md5_res[16];
	char md5_res_str[64]={'\0'};
	int ret = 0;
	uint8_t buffer[1024];
	protobuf_RegisterRequestWithAuth req = protobuf_RegisterRequestWithAuth_init_default;
	pb_ostream_t stream;

	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));
	memset(md5_str,0,sizeof(md5_str));
	//auth string :  MD5(MD5(用户名+":"+密码)+ ":"+nonce)
	printf("username:%s,%s\n", handle->user_name,handle->psd);

	sprintf(md5_str,"%s:%s",handle->user_name,handle->psd);
	mojing_md5_cal(md5_str,strlen(md5_str),md5_res_str);
	printf("md51:\n%s\n", md5_res_str);
	sprintf(md5_str,"%s:%s",md5_res_str,handle->nonce);
	mojing_md5_cal(md5_str,strlen(md5_str),md5_res_str);
	printf("md52:\n%s\n", md5_res_str);

	req.RegisterTime = time(NULL);
	req.PingSpan = handle->ping_pong_time;
	req.DeviceID.funcs.encode = &write_string;
	req.DeviceID.arg = handle->device_id;
	req.Auth.funcs.encode = &write_string;
	req.Auth.arg = md5_res_str;

	mj_header.msg_type = Msg_RegisterRequestWithAuth;

	stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&stream, protobuf_RegisterRequestWithAuth_fields, &req))
	{
		mj_header.msg_type = Msg_RegisterRequestWithAuth;
		mj_header.msg_len = stream.bytes_written;
		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,stream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("Mojing Msg_RegisterRequestWithAuth send %d ok\n", handle->mj_header.msg_len);
		}
		else
		{
			printf("Mojing Msg_RegisterRequestWithAuth send failed\n");
		}


	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&stream));
		return -1;
	}

#endif

	return ret;


}

int mojing_upload_device_info_request(MoJing_ConnectT *handle)
{
#ifdef MOJING_USE_PROTOBUF
	goprotobuf::UploadDeviceInfoRequest req;
	MoJing_HeaderT mj_header;
	char md5_str[256];
	char md5_res[16];
	char md5_res_str[64]={'\0'};


	long ts = time(NULL);
	int ret = 0;
	int sendLen = 0;

	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));


	req.set_deviceid(handle->device_id);
	req.set_firmwareversion("Goke_fw_v1.0");
	req.set_firmwaremd5("Goke_fw_md5");
	req.set_softwareversion("Goke_soft_v1.0");
	req.set_softwaremd5("Goke_soft_md5");

	if(!req.SerializeToArray(handle->buffer+ sizeof(MoJing_HeaderT), MJ_PB_BUFFER_LEN - sizeof(MoJing_HeaderT)))
	{
		printf("serialize info error\n");
		return -1;
	}

	mj_header.msg_type = Msg_UploadDeviceInfoRequest;
	mj_header.msg_len = req.ByteSize();

	handle->mj_header = mj_header;
	memcpy(handle->buffer,&mj_header,sizeof(mj_header));
	ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
	if(ret == 0)
	{
		printf("Mojing Msg_UploadDeviceInfoRequest send ok\n");
	}
	else
	{
		printf("Mojing Msg_UploadDeviceInfoRequest send failed\n");
	}
#else
	MoJing_HeaderT mj_header;
	//char md5_str[256];
	//char md5_res[16];
	char md5_res_str[64]={'\0'};
	int ret = 0;
	uint8_t buffer[1024];
	protobuf_UploadDeviceInfoRequest req = protobuf_UploadDeviceInfoRequest_init_default;
	pb_ostream_t stream;
	//char FirmwareVersion[] = "Goke_fw_v1.0";
	char FirmwareMD5[] = "92c138dc5336ef0c2d64f7aec66d0967";
	//char SoftwareVersion[] = "Goke_soft_v1.0";
	//char SoftwareMD5[] = "92c138dc5336ef0c2d64f7aec66d0967";
    char deviceName[] = "GOKE";
	char hardwarVersion[] = "V1.0";
	char MD5Val[16] = {0};

	if(load_info_to_mtd_reserve(MTD_MD5_INFO, MD5Val, sizeof(MD5Val)) != 0)
	{
		memcpy(md5_res_str,FirmwareMD5,strlen(FirmwareMD5));
		printf("Mojing load MTD_MD5_INFO failed! use default %s!\n",md5_res_str);
	}
	else
	{

		snprintf(md5_res_str,64,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
		MD5Val[0],MD5Val[1],MD5Val[2],MD5Val[3],
		MD5Val[4],MD5Val[5],MD5Val[6],MD5Val[7],
		MD5Val[8],MD5Val[9],MD5Val[10],MD5Val[11],
		MD5Val[12],MD5Val[13],MD5Val[14],MD5Val[15]);

		printf("Mojing load MTD_MD5_INFO success! %s!\n",md5_res_str);
	}

	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));


	req.DeviceID.funcs.encode = &write_string;
	req.DeviceID.arg = handle->device_id;
	req.DeviceName.funcs.encode = &write_string;
	req.DeviceName.arg = deviceName;
	req.FirmwareVersion.funcs.encode = &write_string;
	req.FirmwareVersion.arg = softVersion;
	req.FirmwareMD5.funcs.encode = &write_string;
	req.SoftwareVersion.funcs.encode = &write_string;
	req.SoftwareVersion.arg = softVersion;
	req.SoftwareMD5.funcs.encode = &write_string;
    req.HardwareVersion.arg = hardwarVersion;
	req.HardwareVersion.funcs.encode = &write_string;
    isFirst1004 = 0;
    if (isFirst1004)
    {
        isFirst1004 = 0;
        req.FirmwareMD5.arg = "";
        req.SoftwareMD5.arg = "";
    }
    else
    {
        req.FirmwareMD5.arg = md5_res_str;//FirmwareMD5;
        req.SoftwareMD5.arg = md5_res_str;//SoftwareMD5;
    }

	stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&stream, protobuf_UploadDeviceInfoRequest_fields, &req))
	{
		mj_header.msg_type = Msg_UploadDeviceInfoRequest;
		mj_header.msg_len = stream.bytes_written;
		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,stream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("Mojing Msg_UploadDeviceInfoRequest send ok\n");
		}
		else
		{
			printf("Mojing Msg_UploadDeviceInfoRequest send failed\n");
		}
	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&stream));
		return -1;
	}

#endif

	return ret;


}



int mojing_device_update_response(MoJing_ConnectT *handle)
{
#ifdef MOJING_USE_PROTOBUF
	goprotobuf::DeviceUpdateResponse req;
	MoJing_HeaderT mj_header;

	int ret = 0;
	int sendLen = 0;

	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

	req.set_code(200);
	req.set_message("Update ok");

	if(!req.SerializeToArray(handle->buffer+ sizeof(MoJing_HeaderT), MJ_PB_BUFFER_LEN - sizeof(MoJing_HeaderT)))
	{
		printf("serialize info error\n");
		return -1;
	}

	mj_header.msg_type = Msg_DeviceUpdateResponse;
	mj_header.msg_len = req.ByteSize();

	handle->mj_header = mj_header;
	memcpy(handle->buffer,&mj_header,sizeof(mj_header));
	ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
	if(ret == 0)
	{
		printf("Mojing Msg_DeviceUpdateResponse send ok\n");
	}
	else
	{
		printf("Mojing Msg_DeviceUpdateResponse send failed\n");
	}
#else
	MoJing_HeaderT mj_header;
	uint8_t buffer[1024];
	protobuf_UploadDeviceInfoResponse req = protobuf_UploadDeviceInfoResponse_init_default;
	pb_ostream_t stream;
	int ret = 0;
	char ackstr[] = "Update ok";

	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

	req.Code = 200;
	req.Message.funcs.encode = &write_string;
	req.Message.arg = ackstr;

	stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&stream, protobuf_DeviceUpdateResponse_fields, &req))
	{
		mj_header.msg_type = Msg_DeviceUpdateResponse;
		mj_header.msg_len = stream.bytes_written;

		printf("%s,LINE = %d,mj_header.msg_len = %d\n",__FUNCTION__,__LINE__,mj_header.msg_len);

		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,stream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("Mojing Msg_DeviceUpdateResponse send ok\n");
		}
		else
		{
			printf("Mojing Msg_DeviceUpdateResponse send failed\n");
		}
	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&stream));
		return -1;
	}

#endif

	return ret;
}


int mojing_device_operate_response(MoJing_ConnectT *handle, int status, int msg_type)
{
#ifdef MOJING_USE_PROTOBUF
    printf("mojing_device_operate_response\n");
#else
	MoJing_HeaderT mj_header;
	uint8_t buffer[1024];
	protobuf_OperateResponse req = protobuf_OperateResponse_init_default;
	pb_ostream_t stream;
	int ret = 0;
	char ackstr[] = "OK";
	char ackstrFailed[] = "Failed";

	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

	req.Code = status;
	req.Message.funcs.encode = &write_string;
    if (status == 200)
    {
    	req.Message.arg = ackstr;
    }
    else
    {
    	req.Message.arg = ackstrFailed;
    }

	stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&stream, protobuf_OperateResponse_fields, &req))
	{
		mj_header.msg_type = msg_type;
		mj_header.msg_len = stream.bytes_written;

		printf("%s,LINE = %d,mj_header.msg_len = %d\n",__FUNCTION__,__LINE__,mj_header.msg_len);

		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,stream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("Mojing mojing_device_operate_response send ok, type:%d\n", msg_type);
		}
		else
		{
			printf("Mojing mojing_device_operate_response send failed, type:%d\n", msg_type);
		}
	}
	else
	{
		printf("type:%d, %s,LINE = %d,Encoding failed: %s\n", msg_type, __FUNCTION__,__LINE__,PB_GET_ERROR(&stream));
		return -1;
	}

#endif

	return ret;
}

int mojing_device_sd_info_response(MoJing_ConnectT *handle)
{
    int sdStatus = mmc_get_sdcard_stauts();
#ifdef MOJING_USE_PROTOBUF
    printf("mojing_device_sd_info_response\n");
#else
	MoJing_HeaderT mj_header;
	uint8_t buffer[1024];
	protobuf_GetSDCardInfoResponse req = protobuf_GetSDCardInfoResponse_init_default;
	pb_ostream_t stream;
	int ret = 0;
	char ackstr[] = "OK";

	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

    if (sdStatus == SD_STATUS_NOTINIT)
    {
        req.SDStat = 0;
    }
    else if (sdStatus == SD_STATUS_OK)
    {
        req.SDStat = 3;
        req.SDCardSize = grd_sd_get_all_size_last();
        req.SDCardAvailableSize = grd_sd_get_free_size_last();
        req.SDCardUsedSize = req.SDCardSize - req.SDCardAvailableSize;
        req.SDCardSize /= 1024;
        if (req.SDCardAvailableSize >= req.SDCardSize * 1024)
        {
            req.SDCardAvailableSize = req.SDCardSize * 1024 - 2;
        }
    }
    else
    {
        req.SDStat = 1;
    }

	req.Code = 200;
	req.Message.funcs.encode = &write_string;
	req.Message.arg = ackstr;

    printf("sd status:%d, total:%d, %d, %d\n", req.SDStat, req.SDCardSize, req.SDCardUsedSize, req.SDCardAvailableSize);
	stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&stream, protobuf_GetSDCardInfoResponse_fields, &req))
	{
		mj_header.msg_type = Msg_DeviceGetSDCardInfoResponse;
		mj_header.msg_len = stream.bytes_written;

		printf("%s,LINE = %d,mj_header.msg_len = %d\n",__FUNCTION__,__LINE__,mj_header.msg_len);

		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,stream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("Mojing mojing_device_sd_info_response send ok\n");
		}
		else
		{
			printf("Mojing mojing_device_sd_info_response send failed\n");
		}
	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&stream));
		return -1;
	}

#endif

	return ret;
}

int mojing_device_sd_format_stat(MoJing_ConnectT *handle, int status)
{
#ifdef MOJING_USE_PROTOBUF
    printf("mojing_device_sd_format_stat\n");
#else
	MoJing_HeaderT mj_header;
	uint8_t buffer[1024];
	protobuf_FromatSDCardStatNotify req = protobuf_FromatSDCardStatNotify_init_default;
	pb_ostream_t stream;
	int ret = 0;
	char ackstr[] = "OK";

	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

    req.FromatSDCardStat = status;
	req.Message.funcs.encode = &write_string;
	req.Message.arg = ackstr;

    printf("sd status:%d\n", req.FromatSDCardStat);
	stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&stream, protobuf_FromatSDCardStatNotify_fields, &req))
	{
		mj_header.msg_type = Msg_DeviceFromatSDCardStatNotify;
		mj_header.msg_len = stream.bytes_written;

		printf("%s,LINE = %d,mj_header.msg_len = %d\n",__FUNCTION__,__LINE__,mj_header.msg_len);

		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,stream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("Mojing mojing_device_sd_format_stat send ok\n");
		}
		else
		{
			printf("Mojing mojing_device_sd_format_stat send failed\n");
		}
	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&stream));
		return -1;
	}

#endif

	return ret;
}

int mojing_device_download_stat(MoJing_ConnectT *handle, int status)
{
#ifdef MOJING_USE_PROTOBUF
    printf("mojing_device_sd_format_stat\n");
#else
	MoJing_HeaderT mj_header;
	uint8_t buffer[1024];
	protobuf_DownLoadStatRequest req = protobuf_DownLoadStatRequest_init_default;
	pb_ostream_t stream;
	int ret = 0;
	char ackstr[] = "OK";

	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

    req.DownLoadStat = status;
	req.Message.funcs.encode = &write_string;
	req.Message.arg = ackstr;

    printf("download status:%d\n", req.DownLoadStat);
	stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&stream, protobuf_DownLoadStatRequest_fields, &req))
	{
		mj_header.msg_type = Msg_DownLoadStatRequest;
		mj_header.msg_len = stream.bytes_written;

		printf("%s,LINE = %d,mj_header.msg_len = %d\n",__FUNCTION__,__LINE__,mj_header.msg_len);

		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,stream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("Mojing mojing_device_download_stat send ok\n");
		}
		else
		{
			printf("Mojing mojing_device_download_stat send failed\n");
		}
	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&stream));
		return -1;
	}

#endif

	return ret;
}


int mojing_device_update_notify(MoJing_ConnectT *handle)
{
#ifdef MOJING_USE_PROTOBUF
	goprotobuf::DeviceUpdateNotify req;
	MoJing_HeaderT mj_header;

	int ret = 0;
	int sendLen = 0;

	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

	req.set_updatetype(handle->update_type);
	req.set_tastid(handle->update_tid);
	req.set_wareversion(handle->update_ver);
	req.set_waremd5(handle->update_md5);
	req.set_updatetime(time(NULL));

	if(!req.SerializeToArray(handle->buffer+ sizeof(MoJing_HeaderT), MJ_PB_BUFFER_LEN - sizeof(MoJing_HeaderT)))
	{
		printf("serialize info error\n");
		return -1;
	}

	mj_header.msg_type = Msg_DeviceUpdateNotify;
	mj_header.msg_len = req.ByteSize();

	handle->mj_header = mj_header;
	memcpy(handle->buffer,&mj_header,sizeof(mj_header));
	ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
	if(ret == 0)
	{
		printf("Mojing Msg_DeviceUpdateNotify send ok\n");
	}
	else
	{
		printf("Mojing Msg_DeviceUpdateNotify send failed\n");
	}
#else
	MoJing_HeaderT mj_header;
	uint8_t buffer[1024];
	protobuf_DeviceUpdateNotify req = protobuf_DeviceUpdateNotify_init_zero;
	pb_ostream_t stream;
	int ret = 0;

	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

	req.UpdateType = handle->update_type;
	req.TaskID.funcs.encode = &write_string;
	req.TaskID.arg = handle->update_tid;
	req.wareVersion.funcs.encode = &write_string;
	req.wareVersion.arg = handle->update_ver;
	req.wareMD5.funcs.encode = &write_string;
	req.wareMD5.arg = handle->update_md5;
	req.UpdateTime = time(NULL);

	stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&stream, protobuf_DeviceUpdateNotify_fields, &req))
	{
		mj_header.msg_type = Msg_DeviceUpdateNotify;
		mj_header.msg_len = stream.bytes_written;

		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,stream.bytes_written);

		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("Mojing Msg_DeviceUpdateNotify send ok\n");
		}
		else
		{
			printf("Mojing Msg_DeviceUpdateNotify send failed\n");
		}
	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&stream));
		return -1;
	}

#endif

	return ret;
}

int mojing_send_ping(MoJing_ConnectT *mj_connect)
{
    time_t now ;
    now = time(NULL);
	if(mj_connect->is_regok == 0)
	{
		return 0; // no register,not send ping
	}


    if (mj_connect->last_ping_time == 0)
	{// fist, send ping.
        if (mj_send_ping(mj_connect) < 0)
		{
            // send ping error, maybe disconnect, restart hedgw. or record video to sd_card.
		    return -1; // error.
        }
    }
	else
   	{

		if((mj_connect->last_ping_recv_time != 0
			&& (now - mj_connect->last_ping_recv_time) > mj_connect->ping_pong_time)
			|| mj_connect->pingTimes > 3)
		{
			printf("Send ping bigger %d than pingpong time:%d\n",mj_connect->last_ping_recv_time,
					mj_connect->ping_pong_time);
			return -1;
		}
        if ((now - mj_connect->last_ping_time) > mj_connect->ping_pong_time/2)
		{

            if (mj_send_ping(mj_connect) < 0)
			{
                // send ping error, maybe disconnect, restart hedgw. or record video to sd_card.
    		    return -1; // error.
            }
        }
		else
            return 0; // need not send ping.
    }
    printf("SendPingMsg............%d,%d,%d,%d\n", 
        mj_connect->last_ping_time, now, mj_connect->ping_pong_time, mj_connect->pingTimes);
    mj_connect->last_ping_time = now;
    return 0;
}

void mojing_sd_format_check(void)
{
    int retStatus;
    int sd_status;
	sd_status = mmc_get_sdcard_stauts();

    if (sd_status != SD_STATUS_FORMATING && sd_status != SD_STATUS_NOTINIT)
    {
        netcam_timer_del_task(mojing_sd_format_check);
        if (SD_STATUS_OK == sd_status || SD_STATUS_FORMAT_OK == sd_status)
        {
            retStatus = 200;
        }
        else
        {
            retStatus = 400;
        }
        mojing_device_sd_format_stat(g_connect_handle, retStatus);
    }
}

 int mojing_get_osd_compositions_area_id(void)
{
#if SUPPORT_MOJING_V4
	return osd_composition.area_id;
#else
	return -1;
#endif
}



int mojing_event_log_send(char *devid, char *event_type, int flag, char *content, char *err_msg, char *extend)
{
	char dev_id[50] = {0};
	char jsonStr[2048] = {0};
	int socket_fd = -1;
	int ret = 0;
	char *out;
	cJSON *root;
	struct sockaddr_in servaddr;
	
	if(mojingEvtUploadInfo.enable == 0 || strlen(mojingEvtUploadInfo.host) == 0)
		return -1;

	if(!event_type || !content || !err_msg)
		return -1;

	if(!strcmp(runGB28181Cfg.DeviceUID,"0000000000000000000") || (strstr(runGB28181Cfg.DeviceUID, "00") == NULL))
	{
		return -1;
	}
	else
	{
		strcpy(dev_id,runGB28181Cfg.DeviceUID);
	}

	root = cJSON_CreateObject();//创建项目

	cJSON_AddStringToObject(root, "DeviceID", dev_id);
	cJSON_AddStringToObject(root, "ChannelID", dev_id);
	cJSON_AddNumberToObject(root, "SeqNo", MjEvtSeqNo);
	cJSON_AddStringToObject(root, "EventType", event_type);
	cJSON_AddNumberToObject(root, "Flag", flag);
	cJSON_AddStringToObject(root, "Content", content);
	if(strlen(err_msg))
		cJSON_AddStringToObject(root, "Message", err_msg);
	else
		cJSON_AddStringToObject(root, "Message", "");
	
	cJSON_AddStringToObject(root, "Extend", "");

	out = cJSON_PrintUnformatted(root);
	snprintf(jsonStr, sizeof(jsonStr), "%s", out);

	printf("%s\n",jsonStr);
	
	free(out);

	cJSON_Delete(root);

	socket_fd = socket(PF_INET, SOCK_DGRAM, 0);
	if (socket_fd < 0)
	{
		printf("%s creat socket failed!\n",__FUNCTION__);
		return -1;
	}

	bzero(&servaddr, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(mojingEvtUploadInfo.host);
	servaddr.sin_port = htons(mojingEvtUploadInfo.port);
	ret = sendto(socket_fd, jsonStr, strlen(jsonStr), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
	if(ret <= 0)
	{
		printf("%s send failed!\n",__FUNCTION__);
		close(socket_fd);
		return -1;
	}

	close(socket_fd);
	
	MjEvtSeqNo++;

	return 0;
}


#ifdef MODULE_SUPPORT_MOJING_V4

int mojing_osd_compositions_show(void)
{
	int i = 0;
	int cnt = 0;
	int font_size = 32;
	int stream_cnt = netcam_video_get_channel_number();
	char text[OSD_MULTI_TEXT_LINE_CNT][128];
	int lenth = 0, text_line_cnt = 0,max_size = 0;
    int enable = 0;

	memset(text,0,sizeof(text));
	
	for(i=0; i<OSD_COMPOSITIONS_MAX; ++i)
	{
		if(osd_composition.display[i] && strlen(osd_composition.name[i])>0)
		{
            memset(text[cnt], 0, 128);
			strcpy(text[cnt++],osd_composition.name[i]);
			
			text_line_cnt++;
			if(lenth > max_size)
				max_size = lenth;
            enable = 1;
		}
        else
        {
            memset(text[cnt++], 0, 128);
        }
	}

    //osd只显示主、次码流
    for(i = 0; i < stream_cnt && i < 2; ++i)
	{
#ifdef MODULE_SUPPORT_VECTOR_FONT
		if(runVideoCfg.vencStream[i].h264Conf.height <= 576) 
            font_size = 16;
		else if(runVideoCfg.vencStream[i].h264Conf.height <= 720) 
            font_size = 32;
        else if(runVideoCfg.vencStream[i].h264Conf.height <= 1080)
            font_size = 48;
        else if(runVideoCfg.vencStream[i].h264Conf.height <= 1296)
            font_size = 64;
        else
            font_size = 64;
#else
    
    #ifdef MODULE_SUPPORT_FONT_SIZE_48
        if(runVideoCfg.vencStream[i].h264Conf.height <= 720) 
            font_size = 32;
        else if(runVideoCfg.vencStream[i].h264Conf.height <= 1080)
            font_size = 48;
        else if(runVideoCfg.vencStream[i].h264Conf.height <= 1296)
            font_size = 48;
    #else
        if(runVideoCfg.vencStream[i].h264Conf.height <= 720) 
            font_size = 16;
        else if(runVideoCfg.vencStream[i].h264Conf.height <= 1080)
            font_size = 32;
        else if(runVideoCfg.vencStream[i].h264Conf.height <= 1296)
            font_size = 32;
    #endif  
        else
            font_size = 32;
#endif

		osd_set_display_multiple_line(i, osd_composition.area_id,
			osd_composition.x[i],
			osd_composition.y[i],
			font_size,text,1);

		osd_set_enable(i, osd_composition.area_id, enable);
    }
	
	return 0;
}

static int mojing_osd_compositions_info_save(void)
{
	cJSON *item[OSD_COMPOSITIONS_MAX] = {NULL};
	cJSON *root;
    cJSON *osd_comp;
    cJSON *x_pos;
    cJSON *y_pos;
	int i = 0;
	char *out = NULL;

	root = cJSON_CreateObject();//创建项目

    cJSON_AddItemToObject(root,"osd_ompositions", osd_comp = cJSON_CreateArray());

	x_pos = cJSON_CreateFloatArray(osd_composition.x,sizeof(osd_composition.x)/sizeof(osd_composition.x[0]));
	y_pos = cJSON_CreateFloatArray(osd_composition.y,sizeof(osd_composition.y)/sizeof(osd_composition.y[0]));
	
    cJSON_AddItemToObject(root,"x_pos", x_pos);
    cJSON_AddItemToObject(root,"y_pos", y_pos);

	for(i = 0; i < OSD_COMPOSITIONS_MAX; i++)
	{
		item[i] = cJSON_CreateObject();//创建项目
		cJSON_AddStringToObject(item[i], "Name", osd_composition.name[i]);
		cJSON_AddNumberToObject(item[i], "Index", osd_composition.index[i]);
		cJSON_AddNumberToObject(item[i], "Display", osd_composition.display[i]);
		cJSON_AddItemToArray(osd_comp, item[i]);
	}
	
	cJSON_AddNumberToObject(root, "area_id", osd_composition.area_id);

	out = cJSON_Print(root);

	int ret = CfgWriteToFile(MOJING_OSD_INFO_FILE, out);
	if (ret != 0)
	{
		printf("%s %s error\n",__FUNCTION__,MOJING_OSD_INFO_FILE);
	}
	
	free(out);
	cJSON_Delete(root);
		
	return ret;
}

static int mojing_osd_compositions_info_load(void)
{
	int ret = 0;
	char *data = NULL;
	cJSON *json = NULL;
	cJSON *array = NULL;
	cJSON *item = NULL;
	cJSON *tmp = NULL;
	int i = 0,ArraySize = 0;
	
	data = CfgReadFromFile(MOJING_OSD_INFO_FILE);
	if(data == NULL) 
	{
		printf("%s %s error\n", __FUNCTION__,MOJING_OSD_INFO_FILE);
		ret = -1;
		goto exit;
	}

	json = cJSON_Parse(data);
	if (!json)
	{
		printf("%s parse %s error\n", __FUNCTION__,MOJING_OSD_INFO_FILE);
		ret = -1;
		goto exit;
	}

	tmp = cJSON_GetObjectItem(json, "area_id");
	if(tmp)
		osd_composition.area_id = tmp->valueint;
	else
		osd_composition.area_id = OSD_COMPOSITIONS_ID;

	array = cJSON_GetObjectItem(json, "x_pos");
	if(!array)
	{
		printf("%s parse osd_ompositions error\n",__FUNCTION__);
		ret = -1;
		goto exit;
	}

	ArraySize = cJSON_GetArraySize(array);
	if(ArraySize > MAX_VENC_STREAM_NUM)
		ArraySize = MAX_VENC_STREAM_NUM;
	
	for(i=0; i<ArraySize; i++)
	{
		item = cJSON_GetArrayItem(array, i);
		if(!item)
		{
			printf("%s LINE:%d,cJSON_GetArrayItem error\n",__FUNCTION__,__LINE__);
			ret = -1;
			goto exit;
		}

    	osd_composition.x[i] =  (float)item->valuedouble;
	}

	ArraySize = cJSON_GetArraySize(array);
	if(ArraySize > MAX_VENC_STREAM_NUM)
		ArraySize = MAX_VENC_STREAM_NUM;

	for(i=0; i<ArraySize; i++)
	{
		item = cJSON_GetArrayItem(array, i);
		if(!item)
		{
			printf("%s LINE:%d,cJSON_GetArrayItem error\n",__FUNCTION__,__LINE__);
			ret = -1;
			goto exit;
		}

    	osd_composition.y[i] =  (float)item->valuedouble;
	}

	array = cJSON_GetObjectItem(json, "osd_ompositions");
	if(!array)
	{
		printf("%s parse osd_ompositions error\n",__FUNCTION__);
		ret = -1;
		goto exit;
	}

	ArraySize = cJSON_GetArraySize(array);
	if(ArraySize > OSD_COMPOSITIONS_MAX)
		ArraySize = OSD_COMPOSITIONS_MAX;

	for(i=0; i<ArraySize; i++)
	{

		item = cJSON_GetArrayItem(array, i);
		if(!item)
		{
			printf("%s LINE:%d,cJSON_GetArrayItem error\n",__FUNCTION__,__LINE__);
			ret = -1;
			goto exit;
		}

    	tmp = cJSON_GetObjectItem(item, "Name");
		if(tmp->valuestring)
		{
			memset(osd_composition.name[i],0,sizeof(osd_composition.name[i]));
			strcpy(osd_composition.name[i],tmp->valuestring);
		}

		tmp = cJSON_GetObjectItem(item, "Index");
		if(tmp)
		{
			osd_composition.index[i] = tmp->valueint;
		}

		tmp = cJSON_GetObjectItem(item, "Display");
		if(tmp)
		{
			osd_composition.display[i] = tmp->valueint;
		}

		printf("%s,NAME:%s, index:%d, display:%d\n",__FUNCTION__,osd_composition.name[i],osd_composition.index[i],osd_composition.display[i]);
	}

exit:

	if(data)
		free(data);

	if(json)
    	cJSON_Delete(json);

	if(ret == -1)
	{
		memset(&osd_composition,0,sizeof(osd_composition));
		for(i=0; i<OSD_COMPOSITIONS_MAX; i++)
		{
			osd_composition.index[i] = i + 1;			
		}
		
		for(i=0; i<MAX_VENC_STREAM_NUM; i++)
		{
            //设为1，底部对齐
			osd_composition.x[i] = 1;
			osd_composition.y[i] = 1;
		}
		osd_composition.area_id = OSD_COMPOSITIONS_ID;
#if 0
		osd_composition.display[0] = 1;
		osd_composition.display[1] = 1;
		osd_composition.display[2] = 1;
		osd_composition.display[3] = 1;
		osd_composition.display[4] = 1;
		osd_composition.display[5] = 1;
		strcpy(osd_composition.name[0],"四川省");
		strcpy(osd_composition.name[1],"成都市");
		strcpy(osd_composition.name[2],"武侯区");
		strcpy(osd_composition.name[3],"高朋大道");
		strcpy(osd_composition.name[4],"B座");
		strcpy(osd_composition.name[5],"6楼");
#endif		
		mojing_osd_compositions_info_save();
	}
	else
		mojing_osd_compositions_show();

	netcam_osd_update_title();
    netcam_osd_update_id();

	return ret;
}

static int mojing_get_audio_config_response(MoJing_ConnectT *handle,int msg_len)
{
	MoJing_HeaderT mj_header;
	uint8_t buffer[1024];
	protobuf_GetAudioConfigResponse resp = protobuf_GetAudioConfigResponse_init_default;
	pb_ostream_t ostream;
	int ret = 0;
	char ackstr[] = "OK";

	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}

	
	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

	resp.Code = 200;
	resp.Message.funcs.encode = &write_string;
	resp.Message.arg = ackstr;
    resp.AudioOut = runAudioCfg.outputVolume;
    resp.AudioOutStatus = runAudioCfg.audioOutEnable + 1;
    resp.AudioInStatus = runAudioCfg.audioInEnable + 1;
    resp.AudioIn = runAudioCfg.inputVolume;

	
	ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&ostream, protobuf_GetAudioConfigResponse_fields, &resp))
	{
		mj_header.msg_type = Msg_GetAudioConfigResponse;
		mj_header.msg_len = ostream.bytes_written;

		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,ostream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("%s send ok\n",__FUNCTION__);
		}
		else
		{
			printf("%s send failed\n",__FUNCTION__);
		}
	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&ostream));
		return -1;
	}

	return 0;
}

static int mojing_set_audio_config_response(MoJing_ConnectT *handle,int msg_len)
{
	protobuf_SetAudioConfigRequest message = protobuf_SetAudioConfigRequest_init_default;
	EM_GK_AUDIO_GainLevelEnumT  vi_glevel;
	int ret = 0;
    int retCode = 200;
	int save_flag = 0;
	
	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}

	/* Create a stream that reads from the buffer. */
	pb_istream_t istream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);

	/* Now we are ready to decode the message. */
	ret = pb_decode(&istream, protobuf_SetAudioConfigRequest_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&istream));
		return ret;
	}
	else
	{
        //1=扩音，2=拾音
		printf("%s,AudioType = %d,AudioStatus = %d,Sensitivity = %d\n",__FUNCTION__,message.Type,message.Status,message.Value);
		if(message.Type == 2)
		{
			if(runAudioCfg.inputVolume != message.Value)
			{
				runAudioCfg.inputVolume = message.Value;
				
				vi_glevel = get_gain_level(GK_GLEVEL_MAX*runAudioCfg.inputVolume/100);
				ret = gk_audio_ai_set_gain(&vi_glevel);
				if(ret!= SDK_OK)
					printf("%s,gadi_audio_ai_set_gain failed \n",__FUNCTION__);
				
				save_flag = 1;
			}

			if(message.Status != runAudioCfg.audioInEnable + 1)
			{
                if (message.Status == 1 || message.Status == 2)
                {
    				runAudioCfg.audioInEnable = message.Status - 1;
    				gk_audio_input_enable(runAudioCfg.audioInEnable);
                }
                else if (message.Status == 3)
                {
                    //set to default
                    runAudioCfg.inputVolume = 80;
                    vi_glevel = get_gain_level(GK_GLEVEL_MAX*runAudioCfg.inputVolume/100);
                    gk_audio_ai_set_gain(&vi_glevel);
                }
				save_flag = 1;
			}
		}
		else if(message.Type == 1)
		{
			if(runAudioCfg.outputVolume != message.Value && message.Status != 3)
			{
				runAudioCfg.outputVolume = message.Value;

                gk_audio_ao_set_volume(13*runAudioCfg.outputVolume/100);
				if(ret!= SDK_OK)
					printf("%s,gadi_audio_ao_set_volume failed \n",__FUNCTION__);

				save_flag = 1;
			}
			
			if(message.Status != runAudioCfg.audioOutEnable + 1)
			{
                if (message.Status == 1 || message.Status == 2)
                {
    				runAudioCfg.audioOutEnable = message.Status - 1;
    				audio_spk_en_control(runAudioCfg.audioOutEnable);
                }
                else if (message.Status == 3)
                {
                    //set to default
                    if (strcmp(runSystemCfg.deviceInfo.deviceType, "GK7205S_GC4653_IPC20") == 0)
                    {
                        runAudioCfg.outputVolume = 90;
                    }
                    else
                    {
                        runAudioCfg.outputVolume = 80;
                    }
                    gk_audio_ao_set_volume(13*runAudioCfg.outputVolume/100);
                }
				save_flag = 1;
                
            }
		}
        else
        {
            retCode = 400;
        }

	}

	if(save_flag)
		 AudioCfgSave();

    mojing_device_operate_response(handle, retCode, Msg_SetAudioConfigResponse);

	return 0;
}

static int mojing_osd_info_response(MoJing_ConnectT *handle,int msg_len)
{
	MoJing_HeaderT mj_header;
	uint8_t buffer[1024];
	protobuf_DeviceChannelInfoRequest message = protobuf_DeviceChannelInfoRequest_init_default;
	protobuf_OSDConfigResponse resp = protobuf_OSDConfigResponse_init_zero;
	pb_ostream_t ostream;
	int ret = 0;
	char ackstr[] = "OK";
	int i = 0;
	char tmp[OSD_COMPOSITIONS_MAX][128];
	char ch_name[128];
	
	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}

	/* Create a stream that reads from the buffer. */
	pb_istream_t istream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);
	
	message.ChannelCode.funcs.decode = &read_string;
	message.ChannelCode.arg = NULL;
	message.DeviceCode.funcs.decode = &read_string;
	message.DeviceCode.arg = NULL;

	/* Now we are ready to decode the message. */
	ret = pb_decode(&istream, protobuf_DeviceChannelInfoRequest_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&istream));
		return ret;
	}
	else
	{

		if(message.ChannelCode.arg)
		{
			printf("%s,ChannelCode = %s\n",__FUNCTION__,(char *)message.ChannelCode.arg);
			free(message.ChannelCode.arg);
		}

		if(message.DeviceCode.arg)
		{
			printf("%s,DeviceCode = %s\n",__FUNCTION__,(char *)message.DeviceCode.arg);
			free(message.DeviceCode.arg);
		}
	}

	
	memset(tmp,0,sizeof(tmp));
	memset(ch_name,0,sizeof(ch_name));
	
	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

	resp.Code = 200;
	resp.Message.funcs.encode = &write_string;
	resp.Message.arg = ackstr;
	resp.WeekDisplay = runChannelCfg.channelInfo[0].osdDatetime.displayWeek;
	resp.DateDisplay = runChannelCfg.channelInfo[0].osdDatetime.enable;
	resp.ChannelDisplay = runChannelCfg.channelInfo[0].osdChannelName.enable;

	resp.Channel.funcs.encode = &write_string;
	if(strlen(runChannelCfg.channelInfo[0].osdChannelName.text) > 0)
		utility_gbk_to_utf8(runChannelCfg.channelInfo[0].osdChannelName.text, ch_name, 128);
	
	resp.Channel.arg = (void *)ch_name;

	resp.Compositions_count = sizeof(resp.Compositions)/sizeof(resp.Compositions[0]);
	
	for(i=0; i<resp.Compositions_count; i++)
	{
		resp.Compositions[i].Index = osd_composition.index[i];
		resp.Compositions[i].Display = osd_composition.display[i];
		resp.Compositions[i].Name.funcs.encode = &write_string;

		
		if(strlen(osd_composition.name[i]) > 0)
			utility_gbk_to_utf8(osd_composition.name[i], tmp[i], 128);
		
		resp.Compositions[i].Name.arg = (void *)tmp[i];
	}

	ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&ostream, protobuf_OSDConfigResponse_fields, &resp))
	{
		mj_header.msg_type = Msg_OSDInfoResponse;
		mj_header.msg_len = ostream.bytes_written;

		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,ostream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("%s send ok\n",__FUNCTION__);
		}
		else
		{
			printf("%s send failed\n",__FUNCTION__);
		}
	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&ostream));
		return -1;
	}

	//netcam_osd_deinit();
	//delete_path("/opt/custom/cfg/gk_channel_cfg.cjson");
	//ChannelCfgLoad();
	//netcam_osd_init();

	return 0;
}

int mojing_get_split_str(char *outStr, int oneSize, int maxCnt, char *inStr, char *splitChar)
{
    int i = 0;
    char *p; 
    if (strlen(inStr) == 0)
    {
        return 0;
    }
    
    p = strtok(inStr, splitChar);
    while(p)
    {  
        strcpy(outStr + i * oneSize, p);
        //printf("%s\n", p);  
        
        i++;
        p = strtok(NULL, splitChar); 
        if (i >= maxCnt)
        {
            break;
        }
    }
    return i;
}

static int mojing_get_md_status_response(MoJing_ConnectT *handle)
{
	MoJing_HeaderT mj_header;
	uint8_t buffer[4096] = {0};
	protobuf_GetMDStatusResponse resp = protobuf_GetMDStatusResponse_init_zero;
	pb_ostream_t ostream;
	int ret = 0;
	char ackstr[] = "OK";
	int i = 0;
    int mdDayCnt = 0;
		
	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

	resp.Code = 200;
	resp.Message.funcs.encode = &write_string;
	resp.Message.arg = ackstr;
    if (runMdCfg.enable)
    {
        resp.EventStatus = 1;
    }
    else
    {
        resp.EventStatus = 2;
    }

    resp.Sensitivity = 100 - runMdCfg.sensitive;
    resp.ScreenShotStatus = runMdCfg.mojingMdCfg.screenShotStatus;
    resp.SmartStatus = runMdCfg.mojingMdCfg.smartStatus;
    if (resp.SmartStatus != -1)
    {
        if (runMdCfg.mojingMdCfg.smartType == 1)
        {    
            resp.SmartType_count = 1;
            resp.SmartType[0] = 1;
        }
        else if (runMdCfg.mojingMdCfg.smartType == 2)
        {
            resp.SmartType_count = 1;
            resp.SmartType[0] = 2;
        }
        else if (runMdCfg.mojingMdCfg.smartType == 3)
        {
            resp.SmartType_count = 2;
            resp.SmartType[0] = 1;
            resp.SmartType[1] = 2;
        }
    }

    resp.DefenceStatus = runMdCfg.mojingMdCfg.defenceStatus;
    for (i = 0; i < 7; i++)
    {
        if (strlen(runMdCfg.mojingMdCfg.defenceWeek[i]) > 0)
        {
            strcpy((char*)buffer, runMdCfg.mojingMdCfg.defenceWeek[i]);
            resp.DefenceTime[mdDayCnt].WeekDay = i;
            resp.DefenceTime[mdDayCnt].TimeDuration_count 
                = mojing_get_split_str((char*)&(resp.DefenceTime[mdDayCnt].TimeDuration[0]), 16, 24, (char*)buffer, ",");
            printf("mojing_get_md_status_response:%d,%s\n", resp.DefenceTime[mdDayCnt].TimeDuration_count, buffer);
            mdDayCnt++;
        }
    }
    resp.DefenceTime_count = mdDayCnt;

	ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&ostream, protobuf_GetMDStatusResponse_fields, &resp))
	{
		mj_header.msg_type = Msg_GetMDStatusResponse;
		mj_header.msg_len = ostream.bytes_written;

		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,ostream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("%s send ok\n",__FUNCTION__);
		}
		else
		{
			printf("%s send failed\n",__FUNCTION__);
		}
	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&ostream));
		return -1;
	}

	return 0;
}

int mojing_time_in_shedule(char *timeStrIn, int oneDayStrSize)
{
    int j = 0;
    int cnt = 0;
    int ret = 0;
    GK_SCHEDTIME timeSched[24];
    char timeStr[24][16];
    char buffer[512];    
	long ts = time(NULL);
    int curwDay;
    int index;
    int len = 0;
	struct tm tt = {0};
	struct tm *ptm = localtime_r(&ts, &tt);
    
    curwDay = ptm->tm_wday;
    len = strlen(timeStrIn + curwDay * oneDayStrSize);
    if (len > 0)
    {
        strcpy((char*)buffer, timeStrIn + curwDay * oneDayStrSize);
        cnt = mojing_get_split_str((char*)&(timeStr[0]), 16, 24, (char*)buffer, ",");
        for (j = 0; j < cnt; j++)
        {
            sscanf(timeStr[j], "%d:%d-%d:%d", &timeSched[j].startHour, &timeSched[j].startMin,
                &timeSched[j].stopHour, &timeSched[j].stopMin);
        }
        ret = is_in_schedule_timepoint_report_with_cnt(&timeSched[0], cnt, &index);
    }
    //printf("-->%d mojing_time_in_shedule, ret:%d len:%d:%s\n", curwDay, ret, len, timeStrIn + curwDay * oneDayStrSize);
    return ret;
}

static int mojing_set_md_status_response(MoJing_ConnectT *handle,int msg_len)
{
	protobuf_SetMDStatusRequest message = protobuf_SetMDStatusRequest_init_zero;
	int ret = 0;
    int i, j;

	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}

	/* Create a stream that reads from the buffer. */
	pb_istream_t istream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);
	/* Now we are ready to decode the message. */
	ret = pb_decode(&istream, protobuf_SetMDStatusRequest_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&istream));
        mojing_device_operate_response(handle, 400, Msg_SetMDStatusResponse);
        return ret;
	}
	else
	{
		if (message.EventStatus == 1)
		{
            runMdCfg.enable = 1;
		}
        else if (message.EventStatus == 2)
        {
            runMdCfg.enable = 0;
        } 

        if (message.Sensitivity != 0)
            runMdCfg.sensitive = 100 - message.Sensitivity;
        if (message.ScreenShotStatus != 0)
            runMdCfg.mojingMdCfg.screenShotStatus = message.ScreenShotStatus;
        if (message.SmartStatus != 0)
            runMdCfg.mojingMdCfg.smartStatus = message.SmartStatus;
        if (message.SmartType_count > 0)
        {
            if (message.SmartType[0] != 0)
            {
                if (message.SmartType_count > 1)
                {
                    runMdCfg.mojingMdCfg.smartType = message.SmartType[0] + message.SmartType[1];
                }
                else
                {
                    runMdCfg.mojingMdCfg.smartType = message.SmartType[0];
                }
            }
        }
        if (message.DefenceStatus != 0)
            runMdCfg.mojingMdCfg.defenceStatus = message.DefenceStatus;
        printf("%s, defenceStatus:%d, cnt:%d\n", __FUNCTION__, 
            runMdCfg.mojingMdCfg.defenceStatus, message.DefenceTime_count);
        if (runMdCfg.mojingMdCfg.defenceStatus == 2 && message.DefenceTime_count > 0)
        {
            for (i = 0; i < 7; i++)
            {
                memset(runMdCfg.mojingMdCfg.defenceWeek[i], 0, 256);
            }
            for (i = 0; i < message.DefenceTime_count; i++)
            {
                printf("cur week:%d\n", message.DefenceTime[i].WeekDay);
                for (j = 0; j < message.DefenceTime[i].TimeDuration_count; j++)
                {
                    if (j != 0)
                    {
                        strcat(runMdCfg.mojingMdCfg.defenceWeek[message.DefenceTime[i].WeekDay], ",");
                    }
                    printf("time:%s\n", message.DefenceTime[i].TimeDuration[j]);
                    strcat(runMdCfg.mojingMdCfg.defenceWeek[message.DefenceTime[i].WeekDay], message.DefenceTime[i].TimeDuration[j]);
                }
                printf("\n");
            }
        }
        MdCfgSave();
	}

    mojing_device_operate_response(handle, 200, Msg_SetMDStatusResponse);

	return 0;

}



static int mojing_get_area_alarm_status_response(MoJing_ConnectT *handle,int msg_len)
{
	MoJing_HeaderT mj_header;
	uint8_t buffer[4096] = {0};
    protobuf_GetAreaAlarmStatusRequest message = protobuf_GetAreaAlarmStatusRequest_init_default;
	protobuf_GetAreaAlarmStatusResponse resp = protobuf_GetAreaAlarmStatusResponse_init_zero;
	pb_ostream_t ostream;
	int ret = 0;
	char ackstr[] = "OK";
	int i = 0;
    int areaTimeCnt = 0;

	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}

	/* Create a stream that reads from the buffer. */
	pb_istream_t istream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);
	/* Now we are ready to decode the message. */
	ret = pb_decode(&istream, protobuf_GetAreaAlarmStatusRequest_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&istream));
		return ret;
	}
	else
		printf("%s,message.PresetPosition = %d\n",__FUNCTION__,message.PresetPosition);
    
	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

	resp.Code = 200;
	resp.Message.funcs.encode = &write_string;
	resp.Message.arg = ackstr;

    resp.EventStatus = runMdCfg.mojingMdCfg.areaEventStatus;
    resp.Sensitivity = runMdCfg.mojingMdCfg.areaSensitive;
    resp.ScreenShotStatus = runMdCfg.mojingMdCfg.areaScreenShotStatus;
    resp.SmartStatus = runMdCfg.mojingMdCfg.areaSmartStatus;
    /*
    printf("Sensitivity:%d, areaSmartType:%d, action:%d, areaDefenceStatus:%d\n", 
        runMdCfg.mojingMdCfg.areaSensitive, runMdCfg.mojingMdCfg.areaSmartType, 
        runMdCfg.mojingMdCfg.areaAction, runMdCfg.mojingMdCfg.areaDefenceStatus);
    */    
    if (runMdCfg.mojingMdCfg.areaSmartType == 1)
    {    
        resp.SmartType_count = 1;
        resp.SmartType[0] = 1;
    }
    else if (runMdCfg.mojingMdCfg.areaSmartType == 2)
    {
        resp.SmartType_count = 1;
        resp.SmartType[0] = 2;
    }
    else if (runMdCfg.mojingMdCfg.areaSmartType == 3)
    {
        resp.SmartType_count = 2;
        resp.SmartType[0] = 1;
        resp.SmartType[1] = 2;
    }
    else
    {
        resp.SmartType_count = 1;
        resp.SmartType[0] = 1;
    }

    if (runMdCfg.mojingMdCfg.areaAction == 1)
    {    
        resp.Action_count = 1;
        resp.Action[0] = 1;
    }
    else if (runMdCfg.mojingMdCfg.areaAction == 2)
    {
        resp.Action_count = 1;
        resp.Action[0] = 2;
    }
    else if (runMdCfg.mojingMdCfg.areaAction == 3)
    {
        resp.Action_count = 2;
        resp.Action[0] = 1;
        resp.Action[1] = 2;
    }

	resp.AreaGroup[0].Resolution.funcs.encode = &write_string;
	resp.AreaGroup[0].Resolution.arg = runMdCfg.mojingMdCfg.areaGroupResolution;
    //only handle one
    resp.AreaGroup_count = 1;
    strcpy((char*)buffer, runMdCfg.mojingMdCfg.areaGroupPoints);
    resp.AreaGroup[0].Points_count = mojing_get_split_str((char*)&resp.AreaGroup[0].Points[0], 10, 6, (char*)buffer, ":");;

    resp.ShowArea = runMdCfg.mojingMdCfg.areaShowArea;

    resp.DefenceStatus = runMdCfg.mojingMdCfg.areaDefenceStatus;
    for (i = 0; i < 7; i++)
    {
        if (strlen(runMdCfg.mojingMdCfg.areaDefenceWeek[i]) > 0)
        {
            strcpy((char*)buffer, runMdCfg.mojingMdCfg.areaDefenceWeek[i]);
            resp.DefenceTime[areaTimeCnt].WeekDay = i;
            resp.DefenceTime[areaTimeCnt].TimeDuration_count 
                = mojing_get_split_str((char*)&(resp.DefenceTime[areaTimeCnt].TimeDuration[0]), 16, 24, (char*)buffer, ",");
            areaTimeCnt++;
        }
    }
    resp.DefenceTime_count = areaTimeCnt;

	ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&ostream, protobuf_GetAreaAlarmStatusResponse_fields, &resp))
	{
		mj_header.msg_type = Msg_GetAreaAlarmStatusResponse;
		mj_header.msg_len = ostream.bytes_written;

		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,ostream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("%s send ok\n",__FUNCTION__);
		}
		else
		{
			printf("%s send failed\n",__FUNCTION__);
		}
	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&ostream));
		return -1;
	}

	return 0;
}

static int mojing_set_area_alarm_status_response(MoJing_ConnectT *handle,int msg_len)
{
	protobuf_SetAreaAlarmStatusRequest message = protobuf_SetAreaAlarmStatusRequest_init_default;
	int ret = 0;
    int i, j;
    char tmpPoints[128] = {0};

	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}

	/* Create a stream that reads from the buffer. */
	pb_istream_t istream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);

    for (i = 0; i < 3; i++)
    {        
    	message.AreaGroup[i].Resolution.funcs.decode = &read_string;
    	message.AreaGroup[i].Resolution.arg = NULL;
    }
    
    /* Now we are ready to decode the message. */
	ret = pb_decode(&istream, protobuf_SetAreaAlarmStatusRequest_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&istream));
        mojing_device_operate_response(handle, 400, Msg_SetAreaAlarmStatusResponse);
		return ret;
	}
	else
	{
        if (message.EventStatus != 0)
            runMdCfg.mojingMdCfg.areaEventStatus = message.EventStatus;
        if (message.Sensitivity != 0 && runMdCfg.mojingMdCfg.areaSensitive != message.Sensitivity)
        {
            runMdCfg.mojingMdCfg.areaSensitive = message.Sensitivity;
            #ifdef MODULE_SUPPORT_GKVISION
            gk_vision_set_sensitive(runMdCfg.mojingMdCfg.areaSensitive);
            #endif
        }
        runMdCfg.mojingMdCfg.areaScreenShotStatus = message.ScreenShotStatus;
        if (message.SmartStatus != 0)
            runMdCfg.mojingMdCfg.areaSmartStatus = message.SmartStatus;
        if (message.SmartType_count > 0)
        {
            if (message.SmartType[0] != 0)
            {
                if (message.SmartType_count > 1)
                {
                    runMdCfg.mojingMdCfg.areaSmartType = message.SmartType[0] + message.SmartType[1];
                }
                else
                {
                    runMdCfg.mojingMdCfg.areaSmartType = message.SmartType[0];
                }
            }
        }

        if (message.Action_count > 0)
        {
            if (message.Action_count > 1)
            {
                runMdCfg.mojingMdCfg.areaAction = message.Action[0] + message.Action[1];
            }
            else
            {
                runMdCfg.mojingMdCfg.areaAction = message.Action[0];
            }
        }
        for (i = 0; i < message.AreaGroup_count && message.AreaGroup_count > 0; i++)
        {
            memset(runMdCfg.mojingMdCfg.areaDefenceWeek[message.DefenceTime[i].WeekDay], 0, 256);
            if (message.AreaGroup[i].Resolution.arg != NULL)
            {
                printf("cur AreaGroup[%d].Resolution:%s\n", i, (char*)message.AreaGroup[i].Resolution.arg);
                strcpy(runMdCfg.mojingMdCfg.areaGroupResolution, message.AreaGroup[i].Resolution.arg);
            }
            if (message.AreaGroup[i].Points_count > 0)
            {
                strcpy(tmpPoints, runMdCfg.mojingMdCfg.areaGroupPoints);
                memset(runMdCfg.mojingMdCfg.areaGroupPoints, 0, sizeof(runMdCfg.mojingMdCfg.areaGroupPoints));
                for (j = 0; j < message.AreaGroup[i].Points_count; j++)
                {
                    if (j != 0)
                    {
                        strcat(runMdCfg.mojingMdCfg.areaGroupPoints, ":");
                    }
                    printf("AreaGroup points:%s\n", message.AreaGroup[i].Points[j]);
                    strcat(runMdCfg.mojingMdCfg.areaGroupPoints, message.AreaGroup[i].Points[j]);
                }
                printf("\n");
                if (strcmp(tmpPoints, runMdCfg.mojingMdCfg.areaGroupPoints) != 0)
                {
                    #ifdef MODULE_SUPPORT_GKVISION
                    gk_vision_fresh_rect();
                    #endif
                }
            }
            //only deal one
            break;
        }
        for (i = 0; i < message.AreaGroup_count; i++)
        {
            if (message.AreaGroup[i].Resolution.arg != NULL)
                free(message.AreaGroup[i].Resolution.arg);
        }

        runMdCfg.mojingMdCfg.areaShowArea = message.ShowArea;
        runMdCfg.mojingMdCfg.areaPresetPosition = message.PresetPosition;
        

        if (message.DefenceStatus == 1 || message.DefenceStatus == 2)
            runMdCfg.mojingMdCfg.areaDefenceStatus = message.DefenceStatus;
        if (runMdCfg.mojingMdCfg.areaDefenceStatus == 2 && message.DefenceTime_count > 0)
        {
            for (i = 0; i < 7; i++)
            {
                memset(runMdCfg.mojingMdCfg.areaDefenceWeek[i], 0, 256);
            }
            for (i = 0; i < message.DefenceTime_count; i++)
            {
                printf("cur week:%d\n", message.DefenceTime[i].WeekDay);
                for (j = 0; j < message.DefenceTime[i].TimeDuration_count; j++)
                {
                    if (j != 0)
                    {
                        strcat(runMdCfg.mojingMdCfg.areaDefenceWeek[message.DefenceTime[i].WeekDay], ",");
                    }
                    printf("time:%s\n", message.DefenceTime[i].TimeDuration[j]);
                    strcat(runMdCfg.mojingMdCfg.areaDefenceWeek[message.DefenceTime[i].WeekDay], message.DefenceTime[i].TimeDuration[j]);
                }
                printf("\n");
            }
        } 
        MdCfgSave();
	}

    mojing_device_operate_response(handle, 200, Msg_SetAreaAlarmStatusResponse);
	return 0;

}

static int mojing_get_event_alarm_out_response(MoJing_ConnectT *handle,int msg_len)
{
	MoJing_HeaderT mj_header;
	uint8_t buffer[4096] = {0};
    protobuf_GetEventAlarmOutRequest message = protobuf_GetEventAlarmOutRequest_init_default;
	protobuf_GetEventAlarmOutResponse resp = protobuf_GetEventAlarmOutResponse_init_zero;
	pb_ostream_t ostream;
	int ret = 0;
    int i = 0;
	char ackstr[] = "OK";
    int typeIndex = 0;
    char defaultName[4][64] = {"警报音", "请注意，您已进入监控区域", "危险区域，请勿靠近", "您好，欢迎光临"};
    char tmpBuf[384] = {0};
    char audioFiles[10][128];
    char audioFilesUTF8[10][128];

	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}

	/* Create a stream that reads from the buffer. */
	pb_istream_t istream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);
	/* Now we are ready to decode the message. */
	ret = pb_decode(&istream, protobuf_GetEventAlarmOutRequest_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&istream));
		return ret;
	}
	else
		printf("%s,message.EventType = %d\n",__FUNCTION__,message.EventType);
    
	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

	resp.Code = 200;
	resp.Message.funcs.encode = &write_string;
	resp.Message.arg = ackstr;

    //1：移动侦测事件，2：区域入侵事件
    typeIndex = message.EventType - 1;
    if (typeIndex > 1)
    {
	    resp.Code = 400;
        resp.Message.arg = "Failed";
    }
    else
    {        
        resp.GBAlarmStatus = runAlarmCfg.alarmInfo[typeIndex].gbStatus;
        resp.LightAlarmStatus = runAlarmCfg.alarmInfo[typeIndex].lightAlarmStatus;
        
        resp.LightAlarmType = runAlarmCfg.alarmInfo[typeIndex].lightAlarmType; //1=常亮，2=闪烁
        resp.LightAlarmFrequency = runAlarmCfg.alarmInfo[typeIndex].lightAlarmFrequency;
        resp.LightAlarmTime = runAlarmCfg.alarmInfo[typeIndex].lightAlarmTime;
        resp.AudioAlarmStatus = runAlarmCfg.alarmInfo[typeIndex].audioAlarmStatus;
        
        resp.AudioPlayCount = runAlarmCfg.alarmInfo[typeIndex].audioPlayCnt;
        strcpy(tmpBuf, runAlarmCfg.audioFiles);
        resp.AudioAlarmFiles_count = mojing_get_split_str((char*)&(audioFiles[0]), 128, 10, tmpBuf, ",");

        printf("cur audio files:%s-%s, cnt:%d\n", runAlarmCfg.audioFiles, runAlarmCfg.alarmInfo[typeIndex].audioFilesCur, resp.AudioAlarmFiles_count);
        for (i = 0; i < resp.AudioAlarmFiles_count; i++)
        {
            resp.AudioAlarmFiles[i].Name.funcs.encode = &write_string;
            resp.AudioAlarmFiles[i].Index = i + 1;
            memset(&(audioFilesUTF8[i]), 0, 128);
            //1=预置，2=自定义
            if (strstr(audioFiles[i], "md_") != NULL)
            {
                //md开头的为预置
                resp.AudioAlarmFiles[i].Type = 1;
                utility_gbk_to_utf8(defaultName[i], audioFilesUTF8[i], 128);
                resp.AudioAlarmFiles[i].Name.arg = audioFilesUTF8[i];
            }
            else
            {
                //自定义
                resp.AudioAlarmFiles[i].Type = 2;
                utility_gbk_to_utf8(audioFiles[i], audioFilesUTF8[i], 128);
                resp.AudioAlarmFiles[i].Name.arg = audioFilesUTF8[i];
            }
            if (strstr(audioFiles[i], runAlarmCfg.alarmInfo[typeIndex].audioFilesCur) != NULL)
            {
                resp.AudioAlarmFiles[i].Status = 1;//是否使用状态（0：否，1：是）
            }
            else
            {
                resp.AudioAlarmFiles[i].Status = 0;//是否使用状态（0：否，1：是）
            }

        }
    }

	ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&ostream, protobuf_GetEventAlarmOutResponse_fields, &resp))
	{
		mj_header.msg_type = Msg_GetEventAlarmOutResponse;
		mj_header.msg_len = ostream.bytes_written;

		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,ostream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("%s send ok\n",__FUNCTION__);
		}
		else
		{
			printf("%s send failed\n",__FUNCTION__);
		}
	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&ostream));
		return -1;
	}

	return 0;
}


static int mojing_set_event_alarm_out_response(MoJing_ConnectT *handle,int msg_len)
{
	protobuf_SetEventAlarmOutRequest message = protobuf_SetEventAlarmOutRequest_init_default;
	int ret = 0;
    int typeIndex = 0;
    int audioCnt = 0;
    char audioFiles[10][128];
    char tmpBuf[384] = {0};

	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}

	/* Create a stream that reads from the buffer. */
	pb_istream_t istream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);

    /* Now we are ready to decode the message. */
	ret = pb_decode(&istream, protobuf_SetEventAlarmOutRequest_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&istream));
        mojing_device_operate_response(handle, 400, Msg_SetEventAlarmOutResponse);
		return ret;
	}
	else
	{
        typeIndex = message.EventType - 1;
        if (message.GBAlarmStatus != 0)
        {
            runAlarmCfg.alarmInfo[typeIndex].gbStatus = message.GBAlarmStatus;
        }

        if (message.LightAlarmStatus != 0)
        {
            runAlarmCfg.alarmInfo[typeIndex].lightAlarmStatus = message.LightAlarmStatus;
        }

        if (message.LightAlarmType != 0)
        {
            runAlarmCfg.alarmInfo[typeIndex].lightAlarmType = message.LightAlarmType; //1=常亮，2=闪烁
        }
        if (message.LightAlarmFrequency != 0)
        {
            runAlarmCfg.alarmInfo[typeIndex].lightAlarmFrequency = message.LightAlarmFrequency;
        }
        if (message.LightAlarmTime > 0)
        {
            runAlarmCfg.alarmInfo[typeIndex].lightAlarmTime = message.LightAlarmTime;
        }
        if (message.AudioAlarmStatus != 0)
        {
            runAlarmCfg.alarmInfo[typeIndex].audioAlarmStatus = message.AudioAlarmStatus;
        }

        if (message.AudioPlayCount != 0)
        {
            runAlarmCfg.alarmInfo[typeIndex].audioPlayCnt = message.AudioPlayCount;
        }

        if (message.AudioAlarmFileIndex != 0)
        {
            strcpy(tmpBuf, runAlarmCfg.audioFiles);
            audioCnt = mojing_get_split_str((char*)&(audioFiles[0]), 128, 10, tmpBuf, ",");
            if (message.AudioAlarmFileIndex <= audioCnt)
            {
                memset(runAlarmCfg.alarmInfo[typeIndex].audioFilesCur, 0, 128);
                strcpy(runAlarmCfg.alarmInfo[typeIndex].audioFilesCur, audioFiles[message.AudioAlarmFileIndex - 1]);
            }
            else
            {
                printf("audio file idx:%d, error.\n", message.AudioAlarmFileIndex);
            }
        }
        printf("audio alrm set type:%d, index:%d,%s\n", message.EventType, message.AudioAlarmFileIndex, runAlarmCfg.alarmInfo[typeIndex].audioFilesCur);
        
        AlarmCfgSave();
	}

    mojing_device_operate_response(handle, 200, Msg_SetEventAlarmOutResponse);

	return 0;

}

static int mojing_set_manual_alarm_response(MoJing_ConnectT *handle,int msg_len)
{
	protobuf_SetManualAlarmRequest message = protobuf_SetManualAlarmRequest_init_default;
	int ret = 0;

	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}

	/* Create a stream that reads from the buffer. */
	pb_istream_t istream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);

    /* Now we are ready to decode the message. */
	ret = pb_decode(&istream, protobuf_SetManualAlarmRequest_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&istream));
        mojing_device_operate_response(handle, 400, Msg_SetManualAlarmResponse);
		return ret;
	}
	else
	{
        printf("mojing_set_manual_alarm_response type:%d\n", message.Type);
        if (message.Type == 2)
        {
            netcam_md_alarm_play(NULL, 0);
            if (netcam_md_get_md_light_status())
            {
                netcam_md_set_md_light_status(0);
            }
        }
        else if (message.Type == 1)
        {
            netcam_md_alarm_play_thread_start();
            netcam_md_alarm_play("md_alarm.alaw", 3);
        }
	}

    mojing_device_operate_response(handle, 200, Msg_SetManualAlarmResponse);

	return 0;

}

static int mojing_audio_file_save(char *fileUrl, char *name)
{
    int out = 0;
    int fileLen = 0;
    int revLen = 0;
    char savePath[128] = {0};
    int ret = 0;//0=成功，1=下载失败，2=校验失败，3=替换失败
    printf("mojing_audio_file_save..\n");
    
    char *downContent = NULL;
    downContent = mojing_http_download_file(fileUrl, &fileLen, &revLen, 0, NULL, 0);
    printf("mojing_audio_file_save file:%s, len:%d,%d\n", name, fileLen, revLen);
    if (downContent != NULL)
    {
        do
        {
            ret = 1;
            if (fileLen == revLen)
            {
                sprintf(savePath, "/opt/custom/cfg/%s", name);
                FILE *fp = NULL;
                fp = fopen(savePath, "wb+");
                if (fp == NULL) {
                    PRINT_ERR("fopen:%s fail\n", savePath);
                    break;
                }
                
                if (fwrite(downContent, 1, fileLen, fp) != fileLen) {
                    PRINT_ERR("fwrite:%s fail\n", savePath);
                    fclose(fp);
                    break;
                }
                
                fflush(fp);
                fclose(fp);
                ret = 0;
            }
        }while(out);
    
        free(downContent);
    }
    else
    {
        ret = 1;
    }
    return ret;
}
static int mojing_audio_files_handle(int action, char *fileName, int index, char *fileUrl)
{
    char tmp[128] = {0};
    char tmpNew[128] = {0};
    char filesName[10][128];
    int fileCount = 0;
    char tmpName[128] = {0};
    int i = 0;
    int fileIdx = -1;
    char defaultFiles[] = "md_alarm.alaw,md_leave.alaw,md_danger.alaw,md_welcome.alaw";
    int ret = 3;//0=成功，1=下载失败，2=校验失败，3=替换失败

    strcpy(tmp, runAlarmCfg.audioFiles);

    fileCount = mojing_get_split_str((char*)&(filesName[0]), 128, 10, tmp, ",");
    /*
    for (i = 0; i < fileCount; i++)
    {
        if (strstr(filesName[i], "md_") == NULL && action == 1)
        {
            //已经存在，改为
            action = 4;
            break;
        }
    }
    */

    if ((action == 2 || action == 3) && (index < 5 || index > fileCount))
    {
        printf("audio file idx:%d, action:%d, error\n", index, action);
        return 3;
    }

    if (action == 1 && fileCount >= 7)
    {
        return 1;
    }

    if (fileName != NULL)
        utility_utf8_to_gbk(fileName, tmpName, 128);

    printf("mojing_audio_files_handle action:%d, file:%s--%s:%d, cnt:%d %s\n", action, fileName, tmpName, strlen(tmpName), fileCount, runAlarmCfg.audioFiles);
    switch(action)
    {
        case 1:
        {
            //添加
            //if (fileCount < 3)
            {
                //save file
                ret = mojing_audio_file_save(fileUrl, tmpName);
                if (ret == 0)
                {
                    strcpy(filesName[fileCount], tmpName);
                    fileCount++;
                }
            }
            break;
        }
        case 2:
        {
            //删除
            ret = 3;
            sprintf(tmp, "/opt/custom/cfg/%s", filesName[index - 1]);
            printf("del file:%s\n", tmp);
            remove(tmp);
            if (strcmp(filesName[index - 1], runAlarmCfg.alarmInfo[0].audioFilesCur) == 0)
            {
                strcpy(runAlarmCfg.alarmInfo[0].audioFilesCur, filesName[0]);
            }
            
            if (strcmp(filesName[index - 1], runAlarmCfg.alarmInfo[1].audioFilesCur) == 0)
            {
                strcpy(runAlarmCfg.alarmInfo[1].audioFilesCur, filesName[0]);
            }
            memset(filesName[index - 1], 0, 64);
            ret = 0;
            break;
        }
        case 3:
        {
            //替换
            sprintf(tmp, "/opt/custom/cfg/%s", filesName[index - 1]);
            sprintf(tmpNew, "/opt/custom/cfg/%s_bak", filesName[index - 1]);
            printf("rename file:%s->%s\n", tmp, tmpNew);
            rename(tmp, tmpNew);
            ret = mojing_audio_file_save(fileUrl, tmpName);
            if (ret == 0)
            {
                if (strlen(tmpNew) > 0)
                    remove(tmpNew);
                strcpy(filesName[index - 1], tmpName);
            }
            else
            {
                ret = 3;
                if (strlen(tmpNew) > 0)
                    rename(tmpNew, tmp);
            }
            break;
        }
        default:
        {
            printf("mojing_audio_files_handle unknown action.\n");
            break;
        }
    }

    if (ret == 0)
    {
        memset(runAlarmCfg.audioFiles, 0, sizeof(runAlarmCfg.audioFiles));
        for (i = 0; i < fileCount; i++)
        {
            if (strlen(filesName[i]) > 0)
            {
                if (i > 0)
                {
                    strcat(runAlarmCfg.audioFiles, ",");
                }
                //sprintf(tmp, "%c:%s", index + '0', fileName);
                strcat(runAlarmCfg.audioFiles, filesName[i]);
            }
        }

        if (strstr(runAlarmCfg.audioFiles, defaultFiles) == NULL)
        {
            strcpy(runAlarmCfg.audioFiles, defaultFiles);
            if (strlen(tmpName) > 0)
            {
                sprintf(tmp, "/opt/custom/cfg/%s", tmpName);
                remove(tmp);
            }
            if (action == 1)
            {
                ret = 1;
            }
            else if (action == 3)
            {
                ret = 3;
            }
        }
        AlarmCfgSave();
    }
    return ret;
}

static int mojing_set_audio_alarm_file_response(MoJing_ConnectT *handle,int msg_len)
{
	MoJing_HeaderT mj_header;
	uint8_t buffer[1024];
	protobuf_SetAudioAlarmFileRequest message = protobuf_SetAudioAlarmFileRequest_init_default;
	protobuf_OperateResponse resp = protobuf_OperateResponse_init_default;
	protobuf_AudioAlarmFileUpdateNotify resp2 = protobuf_AudioAlarmFileUpdateNotify_init_default;
	pb_ostream_t ostream;
	int ret = 0;
	char ackstr[] = "OK";
    char ackstrFail[] = "Failed";
    char fileRet[4][32] = {"成功", "下载失败", "校验失败", "替换失败"};
    char tmp[16] = {0};
    char tmpStr[128] = {0};
    char filesName[10][128];
    int fileCount = 0;

    //MoJing_ConnectT tmpHandle;

	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}


	/* Create a stream that reads from the buffer. */
	pb_istream_t istream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);

    message.FileUrl.funcs.decode = &read_string;
    message.FileUrl.arg = NULL;
    message.FileMD5.funcs.decode = &read_string;
    message.FileMD5.arg = NULL;
    message.FileName.funcs.decode = &read_string;
    message.FileName.arg = NULL;
    message.TaskID.funcs.decode = &read_string;
    message.TaskID.arg = NULL;
    /* Now we are ready to decode the message. */
	ret = pb_decode(&istream, protobuf_SetAudioAlarmFileRequest_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&istream));
		return ret;
	}
	else
	{
        // return ok first. then parse audio file
        memcpy(&mj_header,handle->buffer,sizeof(mj_header));
        
        strcpy(tmpStr, runAlarmCfg.audioFiles);
        
        fileCount = mojing_get_split_str((char*)&(filesName[0]), 128, 10, tmpStr, ",");
        //memcpy(&tmpHandle, handle, sizeof(MoJing_ConnectT));
        if (fileCount >= 7 && message.Action == 1)
        {
            resp.Code = 400;
            resp.Message.arg = ackstrFail;
        }
        else
        {
            resp.Code = 200;
            resp.Message.arg = ackstr;
        }
        resp.Message.funcs.encode = &write_string;
        
        ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
        /* Now encode it and check if we succeeded. */
        if (pb_encode(&ostream, protobuf_OperateResponse_fields, &resp))
        {
            mj_header.msg_type = Msg_SetAudioAlarmFileResponse;
            mj_header.msg_len = ostream.bytes_written;
        
            handle->mj_header = mj_header;
            memcpy(handle->buffer,&mj_header,sizeof(mj_header));
            memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,ostream.bytes_written);
            ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
            if(ret == 0)
            {
                printf("%s send ok\n",__FUNCTION__);
            }
            else
            {
                printf("%s send failed\n",__FUNCTION__);
            }
        }
        else
        {
            printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&ostream));
            return -1;
        }
    
        if (message.TaskID.arg == NULL || (message.Action != 2 && (message.FileUrl.arg == NULL 
            || message.FileMD5.arg == NULL || message.FileName.arg == NULL)))
        {
            printf("mojing_set_audio_alarm_file_response arg is null\n");
            ret = 2;
        }
        else
        {
            ret = mojing_audio_files_handle(message.Action, message.FileName.arg, message.FileIndex, message.FileUrl.arg);
        }

	}

    //send audio file upate result
	resp2.UpdateType = ret;
	resp2.Message.funcs.encode = &write_string;
	utility_gbk_to_utf8(fileRet[ret], tmp, sizeof(tmp));
	resp2.Message.arg = tmp;
	resp2.TaskID.funcs.encode = &write_string;
	resp2.TaskID.arg = message.TaskID.arg;

	ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&ostream, protobuf_AudioAlarmFileUpdateNotify_fields, &resp2))
	{
		mj_header.msg_type = Msg_AudioAlarmFileUpdateNotify;
		mj_header.msg_len = ostream.bytes_written;

		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,ostream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("%s send ok\n",__FUNCTION__);
		}
		else
		{
			printf("%s send failed\n",__FUNCTION__);
		}
	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&ostream));
		ret = -1;
	}

    if (message.TaskID.arg)
    {
        free(message.TaskID.arg);
    }
    if (message.FileUrl.arg)
    {
        free(message.FileUrl.arg);
    }
    if (message.FileMD5.arg)
    {
        free(message.FileMD5.arg);
    }
    if (message.FileName.arg)
    {
        free(message.FileName.arg);
    }

    ret = 0;
	return ret;

}

static int mojing_get_nightVision_cfg_response(MoJing_ConnectT *handle,int msg_len)
{
	MoJing_HeaderT mj_header;
	uint8_t buffer[1024] = {0};
	protobuf_GetNightVisionConfigResponse resp = protobuf_GetNightVisionConfigResponse_init_zero;
	pb_ostream_t ostream;
	int ret = 0;
	char ackstr[] = "OK";

	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}
  
	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

	resp.Code = 200;
	resp.Message.funcs.encode = &write_string;
	resp.Message.arg = ackstr;

    //1=红外，2=全彩（补光），3=智能全彩，4=星光（不补光）
    if (strcmp(runSystemCfg.deviceInfo.deviceType, "GK7202_GC2053_TB_38_BIG_V3") == 0
        || strcmp(runSystemCfg.deviceInfo.deviceType, "GK7202_GC2053_TB_38_BIG_V2_V4") == 0
        || strcmp(runSystemCfg.deviceInfo.deviceType, "GK7202_GC2053_TB_38_BIG_V4") == 0
        || strcmp(runSystemCfg.deviceInfo.deviceType, "GK7205S_GC4653_IPC20") == 0
        || strcmp(runSystemCfg.deviceInfo.deviceType, "TB_38_C142V3") == 0)
    {
        resp.SupportedLightTypes_count = 3;
        resp.SupportedLightTypes[0] = 1;
        resp.SupportedLightTypes[1] = 2;
        resp.SupportedLightTypes[2] = 3;
    }
    else
    {
        resp.SupportedLightTypes_count = 1;
        resp.SupportedLightTypes[0] = 1;
    }

    resp.CurrentLightType = runImageCfg.lightMode + 1;
    //白光灯使用pwm channel 1
    if (sdk_cfg.gpio_light_led_on_value == 0)
    {
        resp.LightnessType = 100 - runpwmCfg.autolight[1].pwm_duty;
    }
    else
    {
        resp.LightnessType = runpwmCfg.autolight[1].pwm_duty;
    }

	ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&ostream, protobuf_GetNightVisionConfigResponse_fields, &resp))
	{
		mj_header.msg_type = Msg_GetNightVisionConfigResponse;
		mj_header.msg_len = ostream.bytes_written;

		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,ostream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("%s send ok\n",__FUNCTION__);
		}
		else
		{
			printf("%s send failed\n",__FUNCTION__);
		}
	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&ostream));
		return -1;
	}

	return 0;
}

static int mojing_set_nightVision_type_response(MoJing_ConnectT *handle,int msg_len)
{
	protobuf_SetNightLightTypeRequest message = protobuf_SetNightLightTypeRequest_init_default;
	int ret = 0;

	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}

	/* Create a stream that reads from the buffer. */
	pb_istream_t istream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);

    /* Now we are ready to decode the message. */
	ret = pb_decode(&istream, protobuf_SetNightLightTypeRequest_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&istream));
		return ret;
	}
	else
	{
        if (runImageCfg.lightMode != message.LightType - 1)
        {
            runImageCfg.lightMode = message.LightType - 1;
            sdk_isp_set_bulb_mode(runImageCfg.lightMode);
            ImageCfgSave();
        }
	}

    mojing_device_operate_response(handle, 200, Msg_SetNightLightTypeResponse);
	return 0;

}

static int mojing_set_nightVision_lightness_response(MoJing_ConnectT *handle,int msg_len)
{
	protobuf_SetLightnessRequest message = protobuf_SetLightnessRequest_init_default;
	int ret = 0;

	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}

	/* Create a stream that reads from the buffer. */
	pb_istream_t istream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);

    /* Now we are ready to decode the message. */
	ret = pb_decode(&istream, protobuf_SetLightnessRequest_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&istream));
		return ret;
	}
	else
	{
        if (runpwmCfg.autolight[1].pwm_duty != message.Lightness)
        {
            if (sdk_cfg.gpio_light_led_on_value == 0)
            {
                //越小越亮
                runpwmCfg.autolight[1].pwm_duty = 100 - message.Lightness;
            }
            else
            {
                runpwmCfg.autolight[1].pwm_duty = message.Lightness;
            }
            #ifdef MODULE_SUPPORT_PWM_IRCUT_LED
            sdk_isp_pwm_set_on_value(1, runpwmCfg.autolight[1].pwm_duty);
            sdk_set_pwm_duty(1, runpwmCfg.autolight[1].pwm_duty * 10);
            #else
            #endif  
            pwmCfgSave();
        }
	}

    mojing_device_operate_response(handle, 200, Msg_SetLightnessResponse);
	return 0;

}


static int mojing_set_osd_config_response(MoJing_ConnectT *handle,int msg_len)
{
	protobuf_SetOSDConfigRequest message = protobuf_SetOSDConfigRequest_init_default;
	int ret = 0;
	int i = 0,j = 0;
	int cnt = 0;
	char tmp[OSD_COMPOSITIONS_MAX][128];
	char ch_name[128];
	
	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}
	
	int stream_cnt = netcam_video_get_channel_number();

	/* Create a stream that reads from the buffer. */
	pb_istream_t istream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);

	message.ChannelCode.funcs.decode = &read_string;
	message.ChannelCode.arg = NULL;
	message.Channel.funcs.decode = &read_string;
	message.Channel.arg = NULL;

	for(i=0; i<sizeof(message.Compositions)/sizeof(message.Compositions[0]); i++)
	{
		message.Compositions[i].Name.funcs.decode = &read_string;
		message.Compositions[i].Name.arg = NULL;
	}

	memset(tmp,0,sizeof(tmp));
	memset(ch_name,0,sizeof(ch_name));
	
	/* Now we are ready to decode the message. */
	ret = pb_decode(&istream, protobuf_SetOSDConfigRequest_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&istream));
		return ret;
	}
	else
	{
		if(message.ChannelCode.arg)
		{
			printf("%s,ChannelCode = %s\n",__FUNCTION__,(char *)message.ChannelCode.arg);
			free(message.ChannelCode.arg);
		}

		if(message.Channel.arg)
		{
			printf("%s,Channel = %s\n",__FUNCTION__,(char *)message.Channel.arg);

			if(strlen((char *)message.Channel.arg) > 0)
				utility_utf8_to_gbk((char *)message.Channel.arg, ch_name, 128);
			
			free(message.Channel.arg);
		}

		printf("%s,WeekDisplay = %d,DateDisplay = %d, ChannelDisplay = %d, Compositions_count = %d\n",__FUNCTION__,\
			message.WeekDisplay,message.DateDisplay,message.ChannelDisplay,message.Compositions_count);

		if(message.ChannelDisplay != runChannelCfg.channelInfo[0].osdChannelName.enable || strlen(ch_name) > 0)
		{
			for (i = 0; i < stream_cnt; i++)
				runChannelCfg.channelInfo[i].osdChannelName.enable = message.ChannelDisplay;

			if(strlen(ch_name) > 0)
			{
				for (i = 0; i < stream_cnt; i++)
				{
					netcam_osd_set_title(i, ch_name);
				}
			}

			netcam_osd_update_title();
		}
		
		if(message.WeekDisplay != runChannelCfg.channelInfo[0].osdDatetime.displayWeek)
		{
			for (i = 0; i < stream_cnt; i++)
				runChannelCfg.channelInfo[i].osdDatetime.displayWeek = message.WeekDisplay;

		}
		
		if(message.DateDisplay!= runChannelCfg.channelInfo[0].osdDatetime.enable)
		{
			for (i = 0; i < stream_cnt; i++)
				runChannelCfg.channelInfo[i].osdDatetime.enable = message.DateDisplay;

		}

		cnt = message.Compositions_count;
		
		for(i=0; i<cnt; i++)
		{
			if(message.Compositions[i].Name.arg)
			{
				printf("%s,Compositions name %s\n",__FUNCTION__,(char *)message.Compositions[i].Name.arg);
			}
			
			printf("%s,Compositions index = %d,Compositions Display = %d\n",__FUNCTION__,message.Compositions[i].Index,message.Compositions[i].Display);

			for(j=0; j<OSD_COMPOSITIONS_MAX; j++)
			{

				if(osd_composition.index[j] != message.Compositions[i].Index)
					continue;

				osd_composition.display[j] = message.Compositions[i].Display;
				if(message.Compositions[i].Name.arg)
				{
					if(strlen((char *)message.Compositions[i].Name.arg) > 0)
						utility_utf8_to_gbk((char *)message.Compositions[i].Name.arg, tmp[i], 128);
					
					strcpy(osd_composition.name[j], tmp[i]);
				}
				else
					memset(osd_composition.name[j],0,sizeof(osd_composition.name[j]));

				break;
			}

			if(message.Compositions[i].Name.arg)
				free(message.Compositions[i].Name.arg);
		}
	}

    mojing_device_operate_response(handle, 200, Msg_SetOSDConfigResponse);

    ChannelCfgSave();
	mojing_osd_compositions_info_save();
	mojing_osd_compositions_show();
	netcam_timer_add_task(netcam_osd_pm_save, NETCAM_TIMER_ONE_SEC, SDK_FALSE, SDK_TRUE);

	return 0;
}
	

static int mojing_event_upload_info_save(char *ip, int port, int enable)
{
	char *out;
	cJSON *root;
	
	root = cJSON_CreateObject();//创建项目

    if (ip == NULL)
    {
    	cJSON_AddStringToObject(root, "ip", mojingEvtUploadInfo.host);
    	cJSON_AddNumberToObject(root, "port", mojingEvtUploadInfo.port);
    	cJSON_AddNumberToObject(root, "enable", mojingEvtUploadInfo.enable);
    }
    else
    {
    	cJSON_AddStringToObject(root, "ip", ip);
    	cJSON_AddNumberToObject(root, "port", port);
    	cJSON_AddNumberToObject(root, "enable", enable);
    }
	cJSON_AddNumberToObject(root, "onvifEnable", onvifEnable);

	out = cJSON_Print(root);

	int ret = CfgWriteToFile(MOJING_EVENT_UPLOAD_INFO_FILE, out);
	if (ret != 0)
	{
		printf("mojing_event_upload_info_save %s error\n", MOJING_EVENT_UPLOAD_INFO_FILE);
	}
	
	free(out);
	cJSON_Delete(root);
		
	return ret;
}

static int mojing_event_upload_info_load(void)
{
	int ret = 0;
	char *data = NULL;
	cJSON *json = NULL;
	cJSON *item = NULL;

	data = CfgReadFromFile(MOJING_EVENT_UPLOAD_INFO_FILE);
	if(data == NULL) 
	{
		printf("mojing_event_upload_info_load %s error\n", MOJING_EVENT_UPLOAD_INFO_FILE);
        mojing_event_upload_info_save("", 0, 0);
		ret = -1;
		goto exit;
	}

	json = cJSON_Parse(data);
	if (!json)
	{
		printf("mojing_event_upload_info_load parse %s error\n", MOJING_EVENT_UPLOAD_INFO_FILE);
		ret = -1;
		goto exit;
	}

	item = cJSON_GetObjectItem(json, "ip");
	if(!item)
	{
		printf("mojing_event_upload_info_load parse ip error\n");
		ret = -1;
		goto exit;
	}

	if(item->valuestring)
		strcpy(mojingEvtUploadInfo.host, item->valuestring);

	item = cJSON_GetObjectItem(json, "port");
	if(!item)
	{
		printf("mojing_event_upload_info_load parse port error\n");
		ret = -1;
		goto exit;
	}

	mojingEvtUploadInfo.port = item->valueint;

	item = cJSON_GetObjectItem(json, "enable");
	if(!item)
	{
		printf("mojing_event_upload_info_load parse enable error\n");
		ret = -1;
		goto exit;
	}

	mojingEvtUploadInfo.enable = item->valueint;
    
	item = cJSON_GetObjectItem(json, "onvifEnable");
	if(!item)
	{
		printf("mojing_event_upload_info_load parse enable error\n");
		ret = -1;
		goto exit;
	}
	onvifEnable = item->valueint;

exit:

	if(data)
		free(data);

	if(json)
    	cJSON_Delete(json);

	if(ret == -1)
	{
		memset(&mojingEvtUploadInfo,0,sizeof(MJ_LogUploadT));
	}

    #if 0
	if (0 != pthread_create(&pthid, NULL, mojing_event_log_send_thread, NULL))
	{
		printf("creat mojing_event_log_send_thread failed\n");
	}
	#endif
	return ret;
}

static int mojing_event_upload_info_response(MoJing_ConnectT *handle,int msg_len)
{
	protobuf_SetEventUploadRequest message = protobuf_SetEventUploadRequest_init_default;
	int ret = 0;

	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}

	/* Create a stream that reads from the buffer. */
	pb_istream_t istream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);
	
	message.Host.funcs.decode = &read_string;
	message.Host.arg = NULL;
	/* Now we are ready to decode the message. */
	ret = pb_decode(&istream, protobuf_SetEventUploadRequest_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&istream));
		return ret;
	}
	else
	{
		if(message.Host.arg)
		{
			printf("%s,Ip: %s,port:%d, uploadStats:%d\n", __FUNCTION__,(char *)message.Host.arg,message.Port,message.UploadStatus);

			if((strcmp(mojingEvtUploadInfo.host, message.Host.arg) != 0) || (mojingEvtUploadInfo.port != message.Port)
				|| (mojingEvtUploadInfo.enable != message.UploadStatus))
			{
				mojing_event_upload_info_save((char *)message.Host.arg,message.Port,message.UploadStatus);
				strcpy(mojingEvtUploadInfo.host, message.Host.arg);
				mojingEvtUploadInfo.port = message.Port;
				mojingEvtUploadInfo.enable = message.UploadStatus;
			}
			
			free(message.Host.arg);
		}
		else
		{
			printf("%s,Ip NULL!\n", __FUNCTION__);
		}

	}

    mojing_device_operate_response(handle, 200, Msg_SetEventUploadResponse);

	return 0;

}

static int mojing_get_gb_config_response(MoJing_ConnectT *handle)
{
	MoJing_HeaderT mj_header;
	uint8_t buffer[1024];
	protobuf_GetGBConfigResponse resp = protobuf_GetGBConfigResponse_init_zero;
	pb_ostream_t ostream;
	int ret = 0;
	char ackstr[] = "OK";

	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}

	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

	resp.Code = 200;
	resp.Message.funcs.encode = &write_string;
	resp.Message.arg = ackstr;
	
	resp.SipHost.funcs.encode = &write_string;
	resp.SipHost.arg = runGB28181Cfg.ServerIP;
	resp.SipPort = runGB28181Cfg.ServerPort;

	resp.SipServerID.funcs.encode = &write_string;
	resp.SipServerID.arg = runGB28181Cfg.ServerUID;
	
	resp.HearBeatTime = runGB28181Cfg.DevHBCycle;
	resp.HeartBeatNum = runGB28181Cfg.DevHBOutTimes;

	ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&ostream, protobuf_GetGBConfigResponse_fields, &resp))
	{
		mj_header.msg_type = Msg_GetGBConfigResponse;
		mj_header.msg_len = ostream.bytes_written;

		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,ostream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("%s send ok\n",__FUNCTION__);
		}
		else
		{
			printf("%s send failed\n",__FUNCTION__);
		}
	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&ostream));
		return -1;
	}

	return 0;

}

static int mojing_update_gb_config_response(MoJing_ConnectT *handle,int msg_len)
{
	protobuf_UpdateGBConfigRequest message = protobuf_UpdateGBConfigRequest_init_default;
	int ret = 0;

	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}
	/* Create a stream that reads from the buffer. */
	pb_istream_t istream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);

	message.SipHost.funcs.decode = &read_string;
	message.SipHost.arg = NULL;
	message.SipServerID.funcs.decode = &read_string;
	message.SipServerID.arg = NULL;	
	message.GBPassword.funcs.decode = &read_string;
	message.GBPassword.arg = NULL;
	/* Now we are ready to decode the message. */
	ret = pb_decode(&istream, protobuf_UpdateGBConfigRequest_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&istream));
		return ret;
	}
	else
	{
		if(message.SipHost.arg)
		{
			printf("%s,SipIp:%s\n", __FUNCTION__,(char *)message.SipHost.arg);
			memset(runGB28181Cfg.ServerIP,0, sizeof(runGB28181Cfg.ServerIP));
			strncpy(runGB28181Cfg.ServerIP,message.SipHost.arg,sizeof(runGB28181Cfg.ServerIP));
			free(message.SipHost.arg);
		}
		else
		{
			printf("%s,SipIp NULL!\n", __FUNCTION__);
		}
		
		if(message.SipServerID.arg)
		{
			printf("%s,SipServerID:%s\n", __FUNCTION__,(char *)message.SipServerID.arg);
			memset(runGB28181Cfg.ServerUID,0, sizeof(runGB28181Cfg.ServerUID));
			strncpy(runGB28181Cfg.ServerUID,message.SipServerID.arg,sizeof(runGB28181Cfg.ServerUID));
			free(message.SipServerID.arg);
		}
		else
		{
			printf("%s,SipServerID NULL!\n", __FUNCTION__);
		}

		if(message.GBPassword.arg)
		{
			printf("%s,GBPassword:%s\n", __FUNCTION__,(char *)message.GBPassword.arg);
			memset(runGB28181Cfg.ServerPwd,0, sizeof(runGB28181Cfg.ServerPwd));
			strncpy(runGB28181Cfg.ServerPwd,message.GBPassword.arg,sizeof(runGB28181Cfg.ServerPwd));
			free(message.GBPassword.arg);
		}
		else
		{
			printf("%s,GBPassword NULL!\n", __FUNCTION__);
		}

		
		printf("%s,SipPort:%d,HearBeatTime:%d,HeartBeatNum:%d\n", __FUNCTION__,message.SipPort,message.HearBeatTime,message.HeartBeatNum);
		runGB28181Cfg.DevHBCycle = message.HearBeatTime;
		runGB28181Cfg.DevHBOutTimes = message.HeartBeatNum;
		runGB28181Cfg.ServerPort = message.SipPort;
		GB28181CfgSave();
	}

    mojing_device_operate_response(handle, 200, Msg_UpdateGBConfigResponse);
	return 0;
}

static int parse_res_info(char *src, char (*dst)[15])
{

	char *p1 = NULL,*p2 = NULL;
	char tmp_str[100] = {0};
	int i = 0;
	char *s = NULL;
	
	if(!src)
		return -1;
	
	p1 = strchr(src,'[');
	p2 = strchr(src,']');
	
	if(p1 && p2)
	{
		strncpy(tmp_str,p1+1, p2-p1-1);

		s = (char *)&tmp_str[0];
		
		do
		{
			p1 = strchr(s,'"');
			if(p1)
			{
				
				p2 = strchr(p1+1,'"');
				if(p2)
				{
					strncpy(dst[i++],p1+1, p2-p1-1);
				}
				else
					break;
				
				s = p2 + 1;
			}
			else
				break;
			
		}while(*s != '\0' && i<3);
		
	}
	else
		return -1;
	
	return i;
}

static int mojing_get_video_config_response(MoJing_ConnectT *handle,int msg_len)
{
	MoJing_HeaderT mj_header;
	uint8_t buffer[1024];
	protobuf_GetVideoConfigRequest message = protobuf_GetVideoConfigRequest_init_default;
	protobuf_GetVideoConfigResponse resp = protobuf_GetVideoConfigResponse_init_default;
	pb_ostream_t ostream;
	int ret = 0;
	char ackstr[] = "OK";
	int stream_id = 0;
	char cur_res[10] = {0};
	char rate_limt[16] = {0};
	char gbk_buf[4][16] = {"极低","低","中","高"};
	char utf8_buf[4][16] = {{0},{0},{0},{0}};
	int frame_rate[4] = {10,12,14,15};
	char encode_type[2][8] = {"H264","H265"};
    char  resolution[5][15];
    char  def_res[2][3][15] = {{{"1920*1080"},{"1280*960"}},{{"1280*720"},{"720*576"},{"640*480"}}};
	int cnt = 0;
	int i = 0;
    int mjEncodeType = 0;
	
	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}
	/* Create a stream that reads from the buffer. */
	pb_istream_t istream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);

	message.ChannelCode.funcs.decode = &read_string;
	message.ChannelCode.arg = NULL;
	/* Now we are ready to decode the message. */
	ret = pb_decode(&istream, protobuf_GetVideoConfigRequest_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&istream));
		return ret;
	}
	else
	{
		if(message.ChannelCode.arg)
		{
			printf("%s,ChannelCode:%s\n", __FUNCTION__,(char *)message.ChannelCode.arg);
			free(message.ChannelCode.arg);
		}
		else
		{
			printf("%s,ChannelCode NULL!\n", __FUNCTION__);
		}
		
		printf("%s,StreamType:%d\n", __FUNCTION__,message.StreamType);

		stream_id = message.StreamType;
		if(stream_id >= MAX_VENC_STREAM_NUM)
		{
			printf("%s,stream_id exceed max stream, use stream 0 instead!\n",__FUNCTION__);
			stream_id = 0;
		}
	}

	ST_GK_ENC_STREAM_H264_ATTR stH264Config;
	memset(&stH264Config, 0, sizeof(ST_GK_ENC_STREAM_H264_ATTR));
	ret = netcam_video_get(0, stream_id, &stH264Config);
	if (ret != 0)
	{
		printf("%s,netcam_video_get failed!\n",__FUNCTION__);
		resp.Code = 404;
		resp.Message.funcs.encode = &write_string;
		resp.Message.arg = (void *)"Not Found";
	}
	else
	{
		resp.Code = 200;
		resp.Message.funcs.encode = &write_string;
		resp.Message.arg = ackstr;
	}
	
	resp.CurrFrameRate = netcam_video_get_actual_fps(stream_id);

	if(runVideoCfg.vi[0].fps >= 25)
	{
		frame_rate[0] = 15;
		frame_rate[1] = 18;
		frame_rate[2] = 20;
		frame_rate[3] = 25;
	}

	for(i=0; i<4; i++)
	{
		resp.FrameRateLimit[i] = frame_rate[i];
	}
	resp.FrameRateLimit_count = 4;

	sprintf(cur_res,"%dx%d",stH264Config.width,stH264Config.height);
	
	resp.CurrResolution.funcs.encode = &write_string;
	resp.CurrResolution.arg = (void *)cur_res;

	memset(resolution,0,sizeof(resolution));
	cnt = parse_res_info((char *)runVideoCfg.vencStream[stream_id].resolution,resolution);

	if(cnt > 0)
	{
		resp.Resolution_count = cnt;

		for(i = 0; i < resp.Resolution_count && i < 5; i++)
		{
			memset(resp.Resolution[i],0,sizeof(resp.Resolution[i]));
			strcpy(resp.Resolution[i],resolution[i]);
            if(strstr(resp.Resolution[i], "x") != NULL)
            {
                resp.Resolution[i][strlen(resp.Resolution[i]) - strlen(strstr(resp.Resolution[i], "x"))] = '*';
            }
			printf("%s\n",resp.Resolution[i]);
		}
	}
	else
	{
		printf("%s,parse resolution failed!,use default resolution!\n",__FUNCTION__);
		
		resp.Resolution_count = 3;
		
		for(i=0; i<resp.Resolution_count; i++)
		{
			memset(resp.Resolution[i],0,sizeof(resp.Resolution[i]));
			strcpy(resp.Resolution[i],def_res[stream_id][i]);
			printf("%s\n",resp.Resolution[i]);
		}
	}

	if(stH264Config.rc_mode == 0 || stH264Config.rc_mode == 2)
		resp.RateType = 1;
	else
		resp.RateType = 2;

	resp.Rate = stH264Config.bps;

	for(i=0; i<MAX_VENC_ITEM_NUM; i++)
	{
		if(strcmp(videoMap[stream_id][i].stringName,"bps") == 0)
		{
			sprintf(rate_limt,"%d~%dkb",(int)videoMap[stream_id][i].min,(int)videoMap[stream_id][i].max);
			break;
		}
	}
	
	resp.RateLimit.funcs.encode = &write_string;
	resp.RateLimit.arg = (void *)rate_limt;

	resp.CurrEncode.funcs.encode = &write_string;
    if (stH264Config.enctype == 1)
    {
        mjEncodeType = 0;
    }
    else if (stH264Config.enctype == 3)
    {
        mjEncodeType = 1;
    }
    printf("--->type:%d, stH264Config.enctype:%d\n", mjEncodeType, stH264Config.enctype);
	if(stH264Config.enctype < 4 && stH264Config.enctype > 0)
		resp.CurrEncode.arg = (void *)encode_type[mjEncodeType];
	else
		resp.CurrEncode.arg = (void *)"NONE";

	for(i=0; i<2; i++)
	{
		memset(resp.EncodeType[i],0,sizeof(resp.EncodeType[i]));
		strcpy(resp.EncodeType[i],encode_type[i]);
	}
	
	resp.EncodeType_count = 2;
	
	resp.CurrImageQuality.funcs.encode = &write_string;	
	utility_gbk_to_utf8(gbk_buf[stH264Config.quality], utf8_buf[stH264Config.quality], sizeof(utf8_buf[0]));
	resp.CurrImageQuality.arg = (void *)utf8_buf[stH264Config.quality];

	for(i=0; i<4; i++)
	{
		memset(resp.ImageQuality[i],0,sizeof(resp.ImageQuality[i]));
		utility_gbk_to_utf8(gbk_buf[i], utf8_buf[i], sizeof(utf8_buf[0]));
		strcpy(resp.ImageQuality[i],utf8_buf[i]);
	}
	
	resp.ImageQuality_count = 4;
	
	if(stH264Config.fps)
		resp.IframeInterval = (stH264Config.gop / stH264Config.fps)*1000;
	else
		resp.IframeInterval = 2000;

	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

	ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&ostream, protobuf_GetVideoConfigResponse_fields, &resp))
	{
		mj_header.msg_type = Msg_GetVideoConfigResponse;
		mj_header.msg_len = ostream.bytes_written;

		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,ostream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("%s send ok\n",__FUNCTION__);
		}
		else
		{
			printf("%s send failed\n",__FUNCTION__);
		}
	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&ostream));
		return -1;
	}

	return 0;
}

static int mojing_set_video_config_response(MoJing_ConnectT *handle,int msg_len)
{
	protobuf_SetVideoConfigRequest message = protobuf_SetVideoConfigRequest_init_default;
	int ret = 0;
	int stream_id = 0;
	int enc_type = 0;
	int rc_mode = 0;
	int image_quatiy = 0;
	int fps = 0, width = 0, height = 0;
	ST_GK_ENC_STREAM_H264_ATTR stH264Config;
	char gbk_buf[4][16] = {"极低","低","中","高"};
	char utf8_buf[4][16] = {{0},{0},{0},{0}};
	char encode_type[2][8] = {"H264","H265"};
	int i = 0;
	char *p = NULL;
	char w[10] = {0};
	char h[10] = {0};
    char checkRes[24] = {0};
    int retCode = 200;
    int checkEnc = 0;
	
	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}
	/* Create a stream that reads from the buffer. */
	pb_istream_t istream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);

	message.ChannelCode.funcs.decode = &read_string;
	message.ChannelCode.arg = NULL;
	
	message.CurrResolution.funcs.decode = &read_string;
	message.CurrResolution.arg = NULL;

	message.CurrImageQuality.funcs.decode = &read_string;
	message.CurrImageQuality.arg = NULL;

	message.CurrEncode.funcs.decode = &read_string;
	message.CurrEncode.arg = NULL;
	
	memset(&stH264Config, 0, sizeof(ST_GK_ENC_STREAM_H264_ATTR));

	/* Now we are ready to decode the message. */
	ret = pb_decode(&istream, protobuf_SetVideoConfigRequest_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&istream));
		return ret;
	}
	else
	{
		if(message.ChannelCode.arg)
		{
			printf("%s,ChannelCode:%s\n", __FUNCTION__,(char *)message.ChannelCode.arg);
			free(message.ChannelCode.arg);
		}
		else
		{
			printf("%s,ChannelCode NULL!\n", __FUNCTION__);
		}

		if(message.CurrResolution.arg)
		{
			printf("%s,CurrResolvePower:%s\n", __FUNCTION__,(char *)message.CurrResolution.arg);
			p = strchr(message.CurrResolution.arg,'*');
			if(p == NULL)
				p = strchr(message.CurrResolution.arg,'x');
			
			if(p)
			{
				strncpy(w,(char *)message.CurrResolution.arg, (char *)p-(char *)message.CurrResolution.arg);
				p++;
				strcpy(h, p);
				width = atoi(w);
				height = atoi(h);
				printf("%s,w:%s, h:%s, width = %d, height = %d!\n", __FUNCTION__,w,h,width,height);
			}
			
			free(message.CurrResolution.arg);
		}
		else
		{
			printf("%s,CurrResolvePower NULL!\n", __FUNCTION__);
		}
		
		if(message.CurrImageQuality.arg)
		{
			printf("%s,CurrImageQuality:%s\n", __FUNCTION__,(char *)message.CurrImageQuality.arg);

			for(i=0; i<4; i++)
			{
				utility_gbk_to_utf8(gbk_buf[i], utf8_buf[i], sizeof(utf8_buf[0]));
				if(strcmp(message.CurrImageQuality.arg, utf8_buf[i]) == 0)
					break;
			}
			
			if(i>=4)
				image_quatiy = 2;
			else
				image_quatiy = i;

			free(message.CurrImageQuality.arg);
		}
		else
		{
			printf("%s,CurrImageQuality NULL!\n", __FUNCTION__);
		}

		if(message.CurrEncode.arg)
		{
			printf("%s,CurrEncode:%s\n", __FUNCTION__,(char *)message.CurrEncode.arg);
			for(i=0; i<2; i++)
			{
				if(strcmp(encode_type[i],message.CurrEncode.arg) == 0)
					break;
			}

			if(i>=2)
				enc_type = 1;
			else if (i == 0)
			{
				enc_type = 1;
			}
            else if (i == 1)
            {
				enc_type = 3;
            }
			
			free(message.CurrEncode.arg);
		}
		else
		{
			printf("%s,CurrEncode NULL!\n", __FUNCTION__);
		}
		
		printf("%s,RateType:%d, Rate:%d, IframeInterval:%d, CurrFrameRate:%d, StreamType:%d\n", __FUNCTION__,message.RateType, message.Rate, message.IframeInterval, message.CurrFrameRate, message.StreamType);

		stream_id = message.StreamType;
		if(stream_id >= 2)
		{
			printf("%s,stream_id exceed max stream, use stream 0 instead!\n",__FUNCTION__);
			retCode = 400;
		}
        else
        {
    		ret = netcam_video_get(0, stream_id, &stH264Config);

    		if(message.RateType == 1)
    			rc_mode = 0;
    		else if(message.RateType == 2)
    			rc_mode = 1;

    		fps = message.CurrFrameRate;

    		if(fps != stH264Config.fps && fps != 0)
    		{
                if (fps > 20)
                {
                    retCode = 400;
                }
                else
                {
                    stH264Config.fps = fps;
                    runVideoCfg.vencStream[stream_id].h264Conf.fps = stH264Config.fps;
                }
    		}

    		if((width != 0 && height != 0) &&
                (width != stH264Config.width || height != stH264Config.height))
    		{
                sprintf(checkRes, "%dx%d", width, height);
                if (strstr(runVideoCfg.vencStream[stream_id].resolution, checkRes) == NULL)
                {
                    printf("mojing_set_video_config_response:%dx%d, %s\n", width, height,
                        runVideoCfg.vencStream[stream_id].resolution);
                    retCode = 400;
                }
                else
                {
                    stH264Config.width = width;
                    stH264Config.height = height;
                    runVideoCfg.vencStream[stream_id].h264Conf.width = stH264Config.width;
                    runVideoCfg.vencStream[stream_id].h264Conf.height = stH264Config.height;
                }
    		}
    		
    		if(enc_type)
    		{
    			stH264Config.enctype = enc_type;
                runVideoCfg.vencStream[stream_id].enctype = stH264Config.enctype;
    		}

    		if(rc_mode != stH264Config.rc_mode && message.RateType != 0)
    		{
    			stH264Config.rc_mode = rc_mode;
                runVideoCfg.vencStream[stream_id].h264Conf.rc_mode = stH264Config.rc_mode;
    		}

    		if(image_quatiy)
    		{
    			stH264Config.quality = image_quatiy;
                runVideoCfg.vencStream[stream_id].h264Conf.quality = stH264Config.quality;
    		}

    		if(message.Rate)
    		{
    			stH264Config.bps = message.Rate;
                runVideoCfg.vencStream[stream_id].h264Conf.bps = stH264Config.bps;
    		}

        }
		
	}

    mojing_device_operate_response(handle, retCode, Msg_SetVideoConfigResponse);
    if (retCode != 400)
    {
        #if 1
        if (runVideoCfg.vencStream[0].h264Conf.height > 1080)
        {
            if (stream_id == 0)
            {
                if (runVideoCfg.vencStream[1].enctype != runVideoCfg.vencStream[stream_id].enctype)
                {
                    runVideoCfg.vencStream[1].enctype = runVideoCfg.vencStream[stream_id].enctype;
                    checkEnc = 1;
                }
            }
            else if (stream_id == 1)
            {
                if (runVideoCfg.vencStream[0].enctype != runVideoCfg.vencStream[stream_id].enctype)
                {
                    runVideoCfg.vencStream[0].enctype = runVideoCfg.vencStream[stream_id].enctype;
                    checkEnc = 1;
                }
            }
        }
        #endif
        ret = netcam_video_set(0, stream_id, &stH264Config);
        #if 1
        //printf("mojing_set_video_config_response checkEnc:%d, %d\n", checkEnc, runVideoCfg.vencStream[0].h264Conf.width);
        if (checkEnc)
        {
            if (stream_id == 0)
            {
                stream_id = 1;
            }
            else if (stream_id == 1)
            {
                stream_id = 0;
            }
            ret = netcam_video_get(0, stream_id, &stH264Config);
            stH264Config.enctype = enc_type;
            ret = netcam_video_set(0, stream_id, &stH264Config);
        }
        #endif
        if (ret == 0)
        {
            VideoCfgSave();
#ifdef MODULE_SUPPORT_GB28181
            ProcessMediaStopAll();
#endif
            rtsp_stopAll();
        }
        else
            printf("netcam_video_set error, ret:%d\n", ret);
    }

	return 0;

}

static int mojing_get_local_rec_info_response(MoJing_ConnectT *handle)
{
	MoJing_HeaderT mj_header;
	uint8_t buffer[1024];
	protobuf_GetLocalStorageConfigResponse resp = protobuf_GetLocalStorageConfigResponse_init_zero;
	pb_ostream_t ostream;
	int ret = 0;
	char ackstr[] = "OK";
    int i;
    int recCnt = 0;
	
	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}

	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

	resp.Code = 200;
	resp.Message.funcs.encode = &write_string;
	resp.Message.arg = ackstr;
    //0：主码流，1：子码流
	resp.StreamType = runRecordCfg.stream_no;

    //1、全天录像，2、定时录像，3、移动侦测录像
    if (runRecordCfg.recordMode == 0)
        resp.StorageType = 2;
	else if(runRecordCfg.recordMode == 2)
		resp.StorageType = 3;
	else
		resp.StorageType = 1;

    if (runRecordCfg.enable == 0)
    {
		resp.StorageType = 0;
    }

    if (resp.StorageType == 2)
    {
        for (i = 0; i < 7; i++)
        {
            if (strlen(runRecordCfg.mojingRecTime[i]) > 0)
            {
                strcpy((char*)buffer, runRecordCfg.mojingRecTime[i]);
                resp.DefenceTime[recCnt].WeekDay = i;
                resp.DefenceTime[recCnt].TimeDuration_count 
                    = mojing_get_split_str((char*)&(resp.DefenceTime[recCnt].TimeDuration[0]), 16, 24, (char*)buffer, ",");
                recCnt++;
            }
        }
        resp.DefenceTime_count = recCnt;
    }
    else
    {
        resp.DefenceTime_count = 0;
    }
	
	ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&ostream, protobuf_GetLocalStorageConfigResponse_fields, &resp))
	{
		mj_header.msg_type = Msg_GetLocalStorageConfigResponse;
		mj_header.msg_len = ostream.bytes_written;

		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,ostream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("%s send ok\n",__FUNCTION__);
		}
		else
		{
			printf("%s send failed\n",__FUNCTION__);
		}
	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&ostream));
		return -1;
	}

	return 0;
}

static int mojing_set_local_rec_info_response(MoJing_ConnectT *handle,int msg_len)
{
	protobuf_SetLocalStorageConfigRequest message = protobuf_SetLocalStorageConfigRequest_init_zero;
	int ret = 0;
    int i, j;
	
	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}
	/* Create a stream that reads from the buffer. */
	pb_istream_t istream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);	

    /* Now we are ready to decode the message. */
	ret = pb_decode(&istream, protobuf_SetLocalStorageConfigRequest_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&istream));
		return ret;
	}
	else
	{
		printf("%s,StorageType:%d, StreamType:%d\n", __FUNCTION__,message.StorageType,message.StreamType);
		runRecordCfg.stream_no = message.StreamType;

        //0,关闭录像，1、全天录像，2、定时录像，3、移动侦测录像
        if(message.StorageType == 0)
        {
			runRecordCfg.enable = 0;            
            thread_record_close();
        }
		else if(message.StorageType == 1)
			runRecordCfg.recordMode = 1;
		else if (message.StorageType == 3)
		{
			runRecordCfg.recordMode = 2;
            if (runMdCfg.handle.is_rec != 1)
            {
			    runMdCfg.handle.is_rec = 1;
                MdCfgSave();
            }
		}
        else if (message.StorageType == 2 && message.DefenceTime_count > 0)
        {
            runRecordCfg.recordMode = 0;
            for (i = 0; i < 7; i++)
            {
                memset(runRecordCfg.mojingRecTime[i], 0, 256);
            }
            for (i = 0; i < message.DefenceTime_count; i++)
            {
                printf("cur week:%d\n", message.DefenceTime[i].WeekDay);
                for (j = 0; j < message.DefenceTime[i].TimeDuration_count; j++)
                {
                    if (j != 0)
                    {
                        strcat((char*)runRecordCfg.mojingRecTime[message.DefenceTime[i].WeekDay], ",");
                    }
                    printf("time:%s\n", message.DefenceTime[i].TimeDuration[j]);
                    strcat((char*)runRecordCfg.mojingRecTime[message.DefenceTime[i].WeekDay], message.DefenceTime[i].TimeDuration[j]);
                }
                printf("\n");
            }        
        }

        if (message.StorageType != 0 && runRecordCfg.enable == 0)
        {
            runRecordCfg.enable = 1;
        }

        if (message.StorageType == 1 || message.StorageType == 2)
        {
            if (runMdCfg.handle.is_rec == 1)
            {
                runMdCfg.handle.is_rec = 0;
                MdCfgSave();
            }
        }
		
		RecordCfgSave();
	}

    mojing_device_operate_response(handle, 200, Msg_SetLocalStorageConfigResponse);

	return 0;

}

static int mojing_get_image_config_response(MoJing_ConnectT *handle,int msg_len)
{
	MoJing_HeaderT mj_header;
	uint8_t buffer[1024];
	protobuf_GetImageConfigRequest message = protobuf_GetImageConfigRequest_init_zero;
	protobuf_GetImageConfigResponse resp = protobuf_GetImageConfigResponse_init_zero;
	pb_ostream_t ostream;
	int ret = 0;
	char ackstr[] = "OK";
	char type_gbk_buf[16] = {"白天"};
	char type_utf8_buf[16] = {0};
	char params_gbk_buf[5][16] = {"对比度","亮度","色度","饱和度","锐度"};
	char params_utf8_buf[5][16] = {{0}};
	int i = 0;
	
	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}
	
	/* Create a stream that reads from the buffer. */
	pb_istream_t istream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);	
	message.ChannelCode.funcs.decode = &read_string;
	message.ChannelCode.arg = NULL;
	/* Now we are ready to decode the message. */
	ret = pb_decode(&istream, protobuf_GetImageConfigRequest_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&istream));
		return ret;
	}
	else
	{

		if(message.ChannelCode.arg)
		{
			printf("%s,ChannelCode:%s\n", __FUNCTION__,(char *)message.ChannelCode.arg);
			free(message.ChannelCode.arg);
		}
	}
	
	resp.Code = 200;
	resp.Message.funcs.encode = &write_string;
	resp.Message.arg = ackstr;

    //1=默认值，2=手动值
    if (runImageCfg.contrast != 50 || runImageCfg.brightness != 50 || runImageCfg.hue != 50
        || runImageCfg.saturation != 50 || runImageCfg.sharpness != 50)
    {
        resp.Model = 2;
    }
    else
    {
        resp.Model = 1;
    }
	resp.Types_count = 1;
	resp.Types[0].Type.funcs.encode = &write_string;
	
	utility_gbk_to_utf8(type_gbk_buf, type_utf8_buf, sizeof(type_utf8_buf));
	resp.Types[0].Type.arg = type_utf8_buf;

	for(i=0; i<5; i++)
	{
		resp.Types[0].Params[i].Name.funcs.encode = &write_string;
		
		utility_gbk_to_utf8(params_gbk_buf[i], params_utf8_buf[i], sizeof(params_utf8_buf[0]));
		resp.Types[0].Params[i].Name.arg = params_utf8_buf[i];

		switch(i)
		{
			case 0:
				resp.Types[0].Params[i].Current = runImageCfg.contrast;
				break;
			case 1:
				resp.Types[0].Params[i].Current = runImageCfg.brightness;
				break;
			case 2:
				resp.Types[0].Params[i].Current = runImageCfg.hue;
				break;
			case 3:
				resp.Types[0].Params[i].Current = runImageCfg.saturation;
				break;
			case 4:
				resp.Types[0].Params[i].Current = runImageCfg.sharpness;
				break;
			default:
				break;
		}
	}
	
	resp.Types[0].Params_count = 5;

	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

	ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&ostream, protobuf_GetImageConfigResponse_fields, &resp))
	{
		mj_header.msg_type = Msg_GetImageConfigResponse;
		mj_header.msg_len = ostream.bytes_written;

		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,ostream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("%s send ok\n",__FUNCTION__);
		}
		else
		{
			printf("%s send failed\n",__FUNCTION__);
		}
	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&ostream));
		return -1;
	}

	return 0;

}

static int mojing_set_image_config_response(MoJing_ConnectT *handle,int msg_len)
{
	protobuf_SetImageConfigRequest message = protobuf_GetImageConfigRequest_init_zero;
	int ret = 0;
	int i = 0, j = 0;
	char params_gbk_buf[5][16] = {"对比度","亮度","色度","饱和度","锐度"};
	char params_utf8_buf[5][16] = {{0}};
    char dayType[16] = {0};
    GK_NET_IMAGE_CFG stImagingConfig;
    int responseCode = 200;
	
	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}
	
    memset(&stImagingConfig, 0, sizeof(GK_NET_IMAGE_CFG));
    ret = netcam_image_get(&stImagingConfig);
    if (ret != 0)
    {
        printf("%s get imaging parameters failed.",__FUNCTION__);
        return -1;
    }
	
	/* Create a stream that reads from the buffer. */
	pb_istream_t istream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);	
	message.ChannelCode.funcs.decode = &read_string;
	message.ChannelCode.arg = NULL;
	
	message.Types[0].Type.funcs.decode = &read_string;
	message.Types[0].Type.arg = NULL;

	for(i=0; i<5; i++)
	{
		message.Types[0].Params[i].Name.funcs.decode = &read_string;
		message.Types[0].Params[i].Name.arg = NULL;
	}
	
	/* Now we are ready to decode the message. */
	ret = pb_decode(&istream, protobuf_SetImageConfigRequest_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&istream));
		return ret;
	}
	else
	{

		if(message.ChannelCode.arg)
		{
			printf("%s,ChannelCode:%s\n", __FUNCTION__,(char *)message.ChannelCode.arg);
			free(message.ChannelCode.arg);
		}

		if(message.Types[0].Type.arg)
		{
			printf("%s,Type:%s\n", __FUNCTION__,(char *)message.Types[0].Type.arg);
			free(message.Types[0].Type.arg);
		}

        //irCutMode;
		if (message.Model == 1)
		{
            //默认值
            stImagingConfig.contrast    = 50;
            stImagingConfig.brightness  = 50;
            stImagingConfig.hue         = 50;
            stImagingConfig.saturation  = 50;
            stImagingConfig.sharpness   = 50;
		}
        else if (message.Model == 2)
        {
    		for(i=0; i<5; i++)
    		{
    			utility_gbk_to_utf8(params_gbk_buf[i], params_utf8_buf[i], sizeof(params_utf8_buf[0]));
    		}
    		
    		for(i=0; i<5; i++)
    		{
    			if(message.Types[0].Params[i].Name.arg == NULL)
    				continue;

                utility_utf8_to_gbk((char *)message.Types[0].Params[i].Name.arg, dayType, 16);
    			printf("%s,day type:%s\n", __FUNCTION__, dayType);

    			for(j=0; j<5; j++)
    			{	
    				if(strcmp(message.Types[0].Params[i].Name.arg,params_utf8_buf[j]) == 0)
    				{
    					break;
    				}
    			}

    			if(j >= 5)
    			{
    				printf("%s,not found param %s\n", __FUNCTION__,(char *)message.Types[0].Params[i].Name.arg);
    				continue;
    			}

    			switch(j)
    			{
    				case 0:
    					stImagingConfig.contrast = message.Types[0].Params[i].Current;
    					break;
    				case 1:
    					stImagingConfig.brightness = message.Types[0].Params[i].Current;
    					break;
    				case 2:
    					stImagingConfig.hue = message.Types[0].Params[i].Current;
    					break;
    				case 3:
    					stImagingConfig.saturation = message.Types[0].Params[i].Current;
    					break;
    				case 4:
    					stImagingConfig.sharpness = message.Types[0].Params[i].Current;
    					break;
    				default:
    					break;
    			}
    			
    			free(message.Types[0].Params[i].Name.arg);
    		}
        }
        else
        {
            responseCode = 400;
        }
	}

    //先返回结果后再设置，避免魔镜app长时间等待
    mojing_device_operate_response(handle, responseCode, Msg_SetImageConfigResponse);

    #if 0
    printf("stImagingConfig: %d,%d,%d,%d,%d\n", stImagingConfig.contrast, stImagingConfig.brightness, 
        stImagingConfig.hue, stImagingConfig.saturation, stImagingConfig.sharpness);
    #endif
	netcam_image_set2(stImagingConfig,1);
    //ImageCfgSave();
	return ret;
}

static int mojing_get_timing_config_response(MoJing_ConnectT *handle)
{
	MoJing_HeaderT mj_header;
	uint8_t buffer[1024];
	protobuf_GetTimingConfigResponse resp = protobuf_GetTimingConfigResponse_init_zero;
	pb_ostream_t ostream;
	int ret = 0;
	char ackstr[] = "OK";
	
	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}
	
	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

	resp.Code = 200;
	resp.Message.funcs.encode = &write_string;
	resp.Message.arg = (void *)ackstr;
	
	resp.NTPHost.funcs.encode = &write_string;
	resp.NTPHost.arg = (void *)runSystemCfg.ntpCfg.serverDomain;
		
	ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&ostream, protobuf_GetTimingConfigResponse_fields, &resp))
	{
		mj_header.msg_type = Msg_GetTimingConfigResponse;
		mj_header.msg_len = ostream.bytes_written;

		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,ostream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("%s send ok\n",__FUNCTION__);
		}
		else
		{
			printf("%s send failed\n",__FUNCTION__);
		}
	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&ostream));
		return -1;
	}

	return 0;
}

static int mojing_set_timing_config_response(MoJing_ConnectT *handle, int msg_len)
{
	protobuf_SetTimingConfigRequest message = protobuf_SetTimingConfigRequest_init_zero;
	int ret = 0;
	char ntp_server[100] = {0};
	
	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}

	/* Create a stream that reads from the buffer. */
	pb_istream_t istream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);	
	message.NTPHost.funcs.decode = &read_string;
	message.NTPHost.arg = NULL;
	/* Now we are ready to decode the message. */
	ret = pb_decode(&istream, protobuf_SetTimingConfigRequest_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&istream));
		return ret;
	}
	else
	{
		if(message.NTPHost.arg)
		{
			printf("%s,NTPService:%s, TimimgInterval:%d\n", __FUNCTION__,(char *)message.NTPHost.arg,message.TimingInterval);
			strcpy(ntp_server,(char *)message.NTPHost.arg);
			
			netcam_sys_ntp_set(ntp_server, strlen(ntp_server), 123);
			ntpc_set_synctime(message.TimingInterval);
			free(message.NTPHost.arg);
		}
		else
		{
			printf("%s,NTPService NULL\n",__FUNCTION__);
		}
	}

    mojing_device_operate_response(handle, 200, Msg_SetTimingConfigResponse);

	return 0;

}

static int mojing_reboot_response(MoJing_ConnectT *handle)
{	
	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}

    mojing_device_operate_response(handle, 200, Msg_RebootResponse);
	netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); 

	return 0;
}

static int mojing_factory_default_response(MoJing_ConnectT *handle)
{	
	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}

    mojing_device_operate_response(handle, 200, Msg_FactoryDefaultResponse);

	netcam_sys_operation(NULL, (void*)SYSTEM_OPERATION_HARD_DEFAULT);

	return 0;
}

static int mojing_get_net_info_response(MoJing_ConnectT *handle)
{
	MoJing_HeaderT mj_header;
	uint8_t buffer[1024];
	protobuf_NetInfoResponse resp = protobuf_NetInfoResponse_init_zero;
	pb_ostream_t ostream;
	int ret = 0;
	char ackstr[] = "OK";
	char wifi_name[SDK_ETHERSTR_LEN] = {0};
	ST_SDK_NETWORK_ATTR net_attr;
	ST_SDK_NETWORK_ATTR wifi_net_attr;
	int i = 0;
	
	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}
	
	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));
	
	resp.Code = 200;
	resp.Message.funcs.encode = &write_string;
	resp.Message.arg = ackstr;
	
	memset(&net_attr, 0, sizeof(ST_SDK_NETWORK_ATTR));
	strncpy(net_attr.name, "eth0", sizeof(net_attr.name));
	netcam_net_get(&net_attr);
	
	resp.NetCards[i].IP.funcs.encode = &write_string;
	resp.NetCards[i].IP.arg = (char *)net_attr.ip;
	resp.NetCards[i].Mac.funcs.encode = &write_string;
	resp.NetCards[i].Mac.arg = (char *)net_attr.mac;
	
	if(netcam_net_get_detect("eth0") == 0)
		resp.NetCards[i].Status = 1;
	else
		resp.NetCards[i].Status = 0;
	
	resp.NetCards[i].IsDynamic = net_attr.dhcp;
	resp.NetCards[i].NetCardType = 2;
	i++;
	
	if(netcam_net_wifi_get_devname() != NULL)
	{
		strcpy(wifi_name, netcam_net_wifi_get_devname());
		memset(&wifi_net_attr, 0, sizeof(ST_SDK_NETWORK_ATTR));
		strncpy(wifi_net_attr.name, wifi_name, sizeof(wifi_net_attr.name));
		netcam_net_get(&wifi_net_attr);
		
		resp.NetCards[i].IP.funcs.encode = &write_string;
		resp.NetCards[i].IP.arg = (char *)wifi_net_attr.ip;
		resp.NetCards[i].Mac.funcs.encode = &write_string;
		resp.NetCards[i].Mac.arg = (char *)wifi_net_attr.mac;
		
		if(netcam_net_get_detect(wifi_name) == 0)
			resp.NetCards[i].Status = 1;
		else
			resp.NetCards[i].Status = 0;
		
		resp.NetCards[i].IsDynamic = wifi_net_attr.dhcp;
		resp.NetCards[i].NetCardType = 1;
		i++;
	}

	resp.NetCards_count = i;

	ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&ostream, protobuf_NetInfoResponse_fields, &resp))
	{
		mj_header.msg_type = Msg_GetNetInfoResponse;
		mj_header.msg_len = ostream.bytes_written;

		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,ostream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("%s send ok\n",__FUNCTION__);
		}
		else
		{
			printf("%s send failed\n",__FUNCTION__);
		}
	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&ostream));
		return -1;
	}

	return 0;
}


static int mojing_update_net_info_response(MoJing_ConnectT *handle,int msg_len)
{
	protobuf_UpdateNetInfoRequest message = protobuf_UpdateNetInfoRequest_init_zero;
	int ret = 0;
    ST_SDK_NETWORK_ATTR net_attr;

	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}


	/* Create a stream that reads from the buffer. */
	pb_istream_t istream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);	
	
	message.Mac.funcs.decode = &read_string;
	message.Mac.arg = NULL;

	/* Now we are ready to decode the message. */
	ret = pb_decode(&istream, protobuf_UpdateNetInfoRequest_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&istream));
		return ret;
	}
	else
	{
		if(message.Mac.arg)
		{
			printf("%s,Mac: %s\n", __FUNCTION__,(char *)message.Mac.arg);
			free(message.Mac.arg);
		}

		printf("%s,Conversion: %d\n", __FUNCTION__,message.Conversion);
        //1：动态转静态，2：静态转动态
        if (message.Conversion == 1)
        {
            if (runNetworkCfg.lan.dhcpIp != 0)
            {
                runNetworkCfg.lan.dhcpIp = 0;
                ret = netcam_net_get_detect(runNetworkCfg.lan.netName);
                if (ret != 0) {
                    PRINT_INFO("eth0 is not running.");
                } else {
                    PRINT_INFO("eth0 is ok.");
                    strcpy(net_attr.name, (char*)runNetworkCfg.lan.netName);
                
                    if(netcam_net_get(&net_attr) != 0) {
                        PRINT_ERR("get network config error.\n");
                    }
                    else
                    {
                        strcpy((char*)runNetworkCfg.lan.ip, net_attr.ip);
                        strcpy((char*)runNetworkCfg.lan.gateway, net_attr.gateway);
                    }
                    printf("bc -- lan ip :%s & static ip :%s\n", net_attr.ip, (char*)runNetworkCfg.lan.ip);
                }
                net_attr.dhcp = 0;
                netcam_net_set(&net_attr) ;
                netcam_net_cfg_save();
            }
        }
        else
        {
            runNetworkCfg.lan.dhcpIp = 1;
        }


	}
    
    mojing_device_operate_response(handle, 200, Msg_UpdateNetInfoResponse);

	return 0;

}

static int mojing_get_device_onvif_response(MoJing_ConnectT *handle,int msg_len)
{
	MoJing_HeaderT mj_header;
	uint8_t buffer[1024] = {0};
	protobuf_GetDeviceOnvifResponse resp = protobuf_GetDeviceOnvifResponse_init_zero;
	pb_ostream_t ostream;
	int ret = 0;
	char ackstr[] = "OK";
	char onvifVer[] = "2.0";

	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}
  
	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

	resp.Code = 200;
	resp.Message.funcs.encode = &write_string;
	resp.Message.arg = ackstr;

    resp.Status = onvifEnable;
	resp.Version.funcs.encode = &write_string;
	resp.Version.arg = onvifVer;


	ostream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&ostream, protobuf_GetDeviceOnvifResponse_fields, &resp))
	{
		mj_header.msg_type = Msg_GetDeviceOnvifResponse;
		mj_header.msg_len = ostream.bytes_written;

		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,ostream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);
		if(ret == 0)
		{
			printf("%s send ok\n",__FUNCTION__);
		}
		else
		{
			printf("%s send failed\n",__FUNCTION__);
		}
	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&ostream));
		return -1;
	}

	return 0;
}

static int mojing_set_device_onvif_response(MoJing_ConnectT *handle,int msg_len)
{
	protobuf_SetDeviceOnvifRequest message = protobuf_SetDeviceOnvifRequest_init_default;
	int ret = 0;
    int needReboot = 0;

	if(!handle)
	{
		printf("%s param NULL!\n",__FUNCTION__);
		return -1;
	}

	/* Create a stream that reads from the buffer. */
	pb_istream_t istream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);

    /* Now we are ready to decode the message. */
	ret = pb_decode(&istream, protobuf_SetDeviceOnvifRequest_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&istream));
		return ret;
	}
	else
	{
        if (onvifEnable != message.Status)
        {
            onvifEnable = message.Status;
            printf("mojing_set_device_onvif_response status:%d\n", onvifEnable);
            mojing_event_upload_info_save(NULL, 0, 0);
            if (onvifEnable == 2)
            {
                needReboot = 1;
            }
            else if (onvifEnable == 1)
            {
#ifdef MODULE_SUPPORT_RTSP
                printf("support rtsp\n");
                rtsp_start();
#endif
            
            
#ifdef MODULE_SUPPORT_ONVIF
                //!!!onvif server must be called before starting HTTP server below.
                printf("support onvif\n");
                onvif_start(1, 80);
                
                netcam_http_onvif_init();
#endif
            }
        }
	}

    mojing_device_operate_response(handle, 200, Msg_SetDeviceOnvifResponse);

    if (needReboot == 1)
    {
        runAudioCfg.rebootMute = 1;
        AudioCfgSave();
        sleep(1);
        netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT
    }
	return 0;

}

int mojing_get_onvif_status(void)
{
    printf("mojing_get_onvif_status:%d\n", onvifEnable);
    return onvifEnable;
}
#else
int mojing_get_onvif_status(void)
{
    return 1;
}
#endif

static void *mojing_cloud_storage_tls_init(int fd,char *host, int port)
{
	int flags;
    char portStr[8] = {0};

    sprintf(portStr, "%d", port);

    flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    int on = 1;
    setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));

    void* pssl = gk_mbedtls_ssl_init(fd, host, portStr);
    if(pssl == NULL)
    {
        printf("mojing_cloud_storage_tls_init error!\n");
        return NULL;
    }

	return pssl;
}

static int mojing_cloud_storage_tls_exit(void *handle)
{
    return gk_mbedtls_ssl_exit(handle);
}

static int mojing_cloud_storage_tls_send(void *handle,char *data,int length)
{
	int ret = gk_mbedtls_ssl_send(handle, data, length);
	return ret;
}

static int mojing_cloud_storage_status_check(int cloudStorageType,int *endtime)
{
	if(g_connect_handle->cloud_storage.cloudStorageType != cloudStorageType)
	{
		printf("cloudStorageType may changed! current type:%d,send thread config type:%d\n",
			g_connect_handle->cloud_storage.cloudStorageType,cloudStorageType);

		return 1;
	}

	if(!endtime)
		return 0;

	if(g_connect_handle->cloud_storage.motionEndTime > *endtime)
		*endtime = g_connect_handle->cloud_storage.motionEndTime;

	return 0;
}

int mojing_cloud_storage_end_notify(int storage_type,char *task_id,long long start_time,long long end_time)
{
#ifdef MOJING_USE_PROTOBUF

#else
	MoJing_ConnectT *handle = g_connect_handle;
	MoJing_HeaderT mj_header;
	uint8_t buffer[1024];
	protobuf_CloudStorageEndNotify req = protobuf_CloudStorageEndNotify_init_zero;
	pb_ostream_t stream;
	int ret = 0;
	char taskID[64] = {0};

	if(!handle)
		return -1;

	if(handle->is_regok == 0)
	{
		//printf("mojing not register!\n");
		cloudStorageInProcess = 0;
		return -1;
	}
#if 0
	if(storage_type == CLOUD_STORAGE_ALL_DAY)
	{
		cloudStorageInProcess = 0;
		return 0;
	}
#endif
	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

	req.DeviceID.funcs.encode = &write_string;
	req.DeviceID.arg = handle->device_id;

	if(task_id && strlen(task_id))
	{
		strcpy(taskID,task_id);
		req.TaskID.funcs.encode = &write_string;
		req.TaskID.arg = taskID;
	}

	req.StartTime = start_time;
	req.EndTime = end_time;

	stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&stream, protobuf_CloudStorageEndNotify_fields, &req))
	{
		mj_header.msg_type = Msg_CloudStorageEndNotify;
		mj_header.msg_len = stream.bytes_written;

		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,stream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);

		printf("mojing_cloud_storage_end_notify send %s\n",ret==0?"OK":"FAILED");

	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&stream));
		ret = -1;
	}

	cloudStorageInProcess = 0;

#endif

	return ret;
}

int mojing_cloud_storage_end_notify_response(int msg_len)
{
    printf("mojing_cloud_storage_end_notify_response\n");

#ifdef MOJING_USE_PROTOBUF

#else

	int ret = 0;
	MoJing_ConnectT *handle = g_connect_handle;

	if(!handle)
		return -1;

	if(handle->is_regok == 0)
	{
		//printf("mojing not register!\n");
		return -1;
	}

	protobuf_OperateResponse message = protobuf_OperateResponse_init_zero;
	/* Create a stream that reads from the buffer. */
	pb_istream_t stream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);

	message.Message.funcs.decode = &read_string;
	message.Message.arg = NULL;

	/* Now we are ready to decode the message. */
	ret = pb_decode(&stream, protobuf_OperateResponse_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&stream));
		return ret;
	}

	/* Print the data contained in the message. */
	if(message.Message.arg)
	{
		printf("mojing_cloud_storage_end_notify_response message %s received!\n",(char *)message.Message.arg);
		free(message.Message.arg);
	}

	printf("mojing_cloud_storage_end_notify_response Code:%d!\n",message.Code);

#endif

	return ret;
}

int mojing_get_cloud_storage_addr_request(int eventType)
{
	motionAlarmCnt++;
	MjEvtAlarmCnt++;

#ifdef MODULE_SUPPORT_GB28181
    if (ite_check_test_server())
        return 0;
#endif

#ifdef MOJING_USE_PROTOBUF

#else
	MoJing_ConnectT *handle = g_connect_handle;
	MoJing_HeaderT mj_header;
	uint8_t buffer[1024];
	protobuf_GetCloudStorageAddrRequest req = protobuf_GetCloudStorageAddrRequest_init_zero;
	pb_ostream_t stream;
	int ret = 0;
	char desc[] = "RTP/AVP/TCP PS/90000";

	if(!handle)
		return -1;

	if(handle->is_regok == 0)
	{
		//printf("mojing not register!\n");
		return -1;
	}

	if(handle->cloud_storage.cloudStorageType == 0)
	{
		printf("mojing not support cloud storage!\n");
		return -1;
	}

	if(eventType == CLOUD_STORAGE_DYNAMIC && handle->cloud_storage.cloudStorageType == CLOUD_STORAGE_ALL_DAY)
	{
		printf("disable storage addr request in all day mode!\n");
		return -1;
	}

	if(cloudStorageInProcess)
	{
		printf("mojing_get_cloud_storage_addr_request cloudStorageInProcess!\n");

		if(handle->cloud_storage.cloudStorageType == CLOUD_STORAGE_DYNAMIC)
		{
#if 0
			diff_time = handle->cloud_storage.motionEndTime - time(NULL);

			if(diff_time > 0 && diff_time <= 5)
				handle->cloud_storage.motionEndTime += handle->cloud_storage.keepTimeSec;
#else
			handle->cloud_storage.motionEndTime = time(NULL) + handle->cloud_storage.keepTimeSec;
#endif
		}

		return -1;
	}

	if(handle->cloud_storage.cloudStorageType == CLOUD_STORAGE_DYNAMIC)
	{

		handle->cloud_storage.motionStartTime = time(NULL) - 1;
		handle->cloud_storage.motionEndTime = handle->cloud_storage.motionStartTime + handle->cloud_storage.keepTimeSec;
	}

	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

	req.DeviceID.funcs.encode = &write_string;
	req.DeviceID.arg = handle->device_id;

	req.CloudStorageType = handle->cloud_storage.cloudStorageType;
	req.StreamType = handle->cloud_storage.streamType;
	req.PreStorageTime = handle->cloud_storage.preStorageTimeSec;
	req.KeepTime = handle->cloud_storage.keepTimeSec;

	req.StreamDesc.funcs.encode = &write_string;
	req.StreamDesc.arg = desc;
	req.StartTime = time(NULL);

	stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&stream, protobuf_GetCloudStorageAddrRequest_fields, &req))
	{
		mj_header.msg_type = Msg_GetCloudStorageAddrRequest;
		mj_header.msg_len = stream.bytes_written;

		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,stream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);

		printf("mojing_get_cloud_storage_addr_request send %s\n",ret==0?"OK":"FAILED");

	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&stream));
		return -1;
	}

#endif

	return ret;
}

int mojing_get_cloud_storage_addr_response(int msg_len)
{
#ifdef MOJING_USE_PROTOBUF

#else
	int ret = 0;
	MoJing_ConnectT *handle = g_connect_handle;
	char serverip[64] = {0};
#ifdef MODULE_SUPPORT_UDP_LOG
	char msgStr[500] = {0};
#endif

	if(!handle)
		return -1;

	if(handle->is_regok == 0)
	{
		//printf("mojing not register!\n");
		return -1;
	}

	if(cloudStorageInProcess)
	{
		printf("mojing_get_cloud_storage_addr_response cloudStorageInProcess!\n");
		return -1;
	}

	protobuf_GetCloudStorageAddrResponse message = protobuf_GetCloudStorageAddrResponse_init_zero;
	/* Create a stream that reads from the buffer. */
	pb_istream_t stream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);

	message.Message.funcs.decode = &read_string;
	message.Message.arg = NULL;

	message.AddrIP.funcs.decode = &read_string;
	message.AddrIP.arg = NULL;

	message.TaskID.funcs.decode = &read_string;
	message.TaskID.arg = NULL;

	/* Now we are ready to decode the message. */
	ret = pb_decode(&stream, protobuf_GetCloudStorageAddrResponse_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&stream));
		return ret;
	}

	/* Print the data contained in the message. */
	if(message.Message.arg)
	{
		printf("mojing_get_cloud_storage_addr_response message %s received!\n",(char *)message.Message.arg);
		free(message.Message.arg);
	}

	if(message.AddrIP.arg)
	{
		printf("mojing_get_cloud_storage_addr_response addr %s received!\n",(char *)message.AddrIP.arg);
		strcpy(serverip,message.AddrIP.arg);
		free(message.AddrIP.arg);
	}
	if(message.TaskID.arg)
	{
		printf("mojing_get_cloud_storage_addr_response taskid %s received!\n",(char *)message.TaskID.arg);
		memset(handle->cloud_storage.taskID,0,sizeof(handle->cloud_storage.taskID));
		strncpy(handle->cloud_storage.taskID,message.TaskID.arg,sizeof(handle->cloud_storage.taskID));
		free(message.TaskID.arg);
	}

	handle->cloud_storage.transType = message.TransType;

	printf("mojing_get_cloud_storage_addr_response Code:%d, SSRC:%d, AddrPort:%d, TransType:%d!\n",message.Code,message.SSRC,message.AddrPort,message.TransType);

	if(!strlen(serverip))
	{
		printf("mojing_get_cloud_storage_addr_response addr is NULL!\n");
#ifdef MODULE_SUPPORT_UDP_LOG
		snprintf(msgStr, sizeof(msgStr),"cloud storage Addr is NULL");
		mojing_log_send(NULL, -1, "dsgw_get_cloud_storage_addr_response", msgStr);
#endif
		MjEvtTlsFailCnt++;
		return -1;
	}

#ifdef MODULE_SUPPORT_UDP_LOG
	snprintf(msgStr, sizeof(msgStr),"ssrc=%d, addrip=%s, addrport=%d,transtype=%d, taskid=%s",\
	message.SSRC,serverip,message.AddrPort,message.TransType,handle->cloud_storage.taskID);
	mojing_log_send(NULL, 0, "dsgw_get_cloud_storage_addr_response", msgStr);
#endif

#ifdef MODULE_SUPPORT_GB28181

	cloudStorageInProcess = 1;

	ret = mojing_cloud_storage_start(serverip,message.AddrPort, message.TransType,message.SSRC, &handle->cloud_storage);
	if(ret < 0)
	{
		cloudStorageInProcess = 0;
	}
#endif

#endif

	return ret;
}

int mojing_get_cloud_storage_request(void)
{
#ifdef MOJING_USE_PROTOBUF

#else
	MoJing_ConnectT *handle = g_connect_handle;
	MoJing_HeaderT mj_header;
	uint8_t buffer[1024];
	protobuf_GetCloudStorageInfoRequest req = protobuf_GetCloudStorageInfoRequest_init_zero;
	pb_ostream_t stream;
	int ret = 0;
	static int lastReqTime = 0;

	if(!handle)
		return -1;

	if(handle->is_regok == 0)
	{
		//printf("mojing not register!\n");
		return -1;
	}

	// copy seq and header info
	memcpy(&mj_header,handle->buffer,sizeof(mj_header));

	req.DeviceID.funcs.encode = &write_string;
	req.DeviceID.arg = handle->device_id;

	req.Time = time(NULL);

	if(!lastReqTime)
		lastReqTime = req.Time;
	else if(req.Time - lastReqTime < 15)
	{
		printf("mojing_get_cloud_storage_request send too frequently!\n");
		lastReqTime = req.Time;
		return -1;
	}

	stream = pb_ostream_from_buffer(buffer, sizeof(buffer));
	/* Now encode it and check if we succeeded. */
	if (pb_encode(&stream, protobuf_GetCloudStorageInfoRequest_fields, &req))
	{
		mj_header.msg_type = Msg_GetCloudStorageInfoRequest;
		mj_header.msg_len = stream.bytes_written;

		handle->mj_header = mj_header;
		memcpy(handle->buffer,&mj_header,sizeof(mj_header));
		memcpy(handle->buffer + sizeof(MoJing_HeaderT),buffer,stream.bytes_written);
		ret = mojing_ssl_send(handle,handle->buffer,handle->mj_header.msg_len+sizeof(MoJing_HeaderT),5);

		printf("mojing_get_cloud_storage_request send %s\n",ret==0?"OK":"FAILED");

	}
	else
	{
		printf("%s,LINE = %d,Encoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&stream));
		return -1;
	}

#endif

	return ret;
}

int mojing_get_cloud_storage_response(int msg_len)
{
#ifdef MODULE_SUPPORT_UDP_LOG
	char msg_str[500] = {0};
#endif
#ifdef MOJING_USE_PROTOBUF

#else
	int ret = 0;
	MoJing_ConnectT *handle = g_connect_handle;

	if(!handle)
		return -1;

	if(handle->is_regok == 0)
	{
		//printf("mojing not register!\n");
		return -1;
	}

	protobuf_GetCloudStorageInfoResponse message = protobuf_GetCloudStorageInfoResponse_init_zero;
	/* Create a stream that reads from the buffer. */
	pb_istream_t stream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);

	message.Message.funcs.decode = &read_string;
	message.Message.arg = NULL;

	/* Now we are ready to decode the message. */
	ret = pb_decode(&stream, protobuf_GetCloudStorageInfoResponse_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&stream));
		return ret;
	}

	/* Print the data contained in the message. */
	if(message.Message.arg)
	{
		printf("mojing_get_cloud_storage_response message %s received!\n",(char *)message.Message.arg);
		free(message.Message.arg);
	}

	printf("mojing_get_cloud_storage_response Code:%d, CloudStorageType:%d, StreamType:%d, PreStorageTime:%d,KeepTime:%d!\n",
		message.Code,message.CloudStorageType,message.StreamType,message.PreStorageTime,message.KeepTime);

	if(message.Code == 200)
	{
		if(message.CloudStorageType != 0)
		{
		    if (access("/opt/custom/cfg/cloud", F_OK) == 0)
    		{
				handle->cloud_storage.cloudStorageType = 1;
				printf("message cloudStorageType is %d, device cloudStorageType is 1!\n", message.CloudStorageType);
    		}
			else
				handle->cloud_storage.cloudStorageType = message.CloudStorageType;

		}
		else
			handle->cloud_storage.cloudStorageType = message.CloudStorageType;
		handle->cloud_storage.streamType = message.StreamType;
		handle->cloud_storage.preStorageTimeSec = message.PreStorageTime;
		handle->cloud_storage.keepTimeSec = message.KeepTime;

		if(handle->cloud_storage.cloudStorageType == 0)
			handle->cloud_storage.cloudStorageStatus = CLOUD_STATUS_NO_CLOUD_STORAGE;
		else if(handle->cloud_storage.cloudStorageType == CLOUD_STORAGE_ALL_DAY || handle->cloud_storage.cloudStorageType == CLOUD_STORAGE_DYNAMIC)
			handle->cloud_storage.cloudStorageStatus = CLOUD_STATUS_OK;

#ifdef MODULE_SUPPORT_UDP_LOG
		sprintf(msg_str,"streamType=%d,cloudStorageType=%d,prestorageTime=%d,keeptime=%d",message.StreamType,message.CloudStorageType,message.PreStorageTime,message.KeepTime);
		mojing_log_send(NULL, 0, "dsgw_get_cloud_storage_info", msg_str);
#endif

	}
	else
	{
		handle->cloud_storage.cloudStorageStatus = CLOUD_STATUS_GET_CONFIG;
#ifdef MODULE_SUPPORT_UDP_LOG
		sprintf(msg_str,"get_cloud_storage_info Code = %d",message.Code);
		mojing_log_send(NULL, -1, "dsgw_get_cloud_storage_info", msg_str);
#endif
		MjEvtTlsFailCnt++;

	}

#endif

	return ret;
}

int mojing_cloud_storage_change_response(void)
{
#ifdef MOJING_USE_PROTOBUF

#else

	MoJing_ConnectT *handle = g_connect_handle;
	int ret = 0;

	if(!handle)
		return -1;

	if(handle->is_regok == 0)
	{
		//printf("mojing not register!\n");
		return -1;
	}

    mojing_device_operate_response(handle, 200, Msg_CloudStorageInfoChangeNotifyOperateResponse);

#endif

	return ret;
}
int mojing_cloud_storage_change_notify(int msg_len)
{
#ifdef MODULE_SUPPORT_UDP_LOG
	char msgStr[500] = {0};
#endif

#ifdef MOJING_USE_PROTOBUF

#else

	int ret = 0;
	MoJing_ConnectT *handle = g_connect_handle;

	if(!handle)
		return -1;

	if(handle->is_regok == 0)
	{
		//printf("mojing not register!\n");
		return -1;
	}

	protobuf_CloudStorageInfoChangeNotify message = protobuf_CloudStorageInfoChangeNotify_init_zero;
	/* Create a stream that reads from the buffer. */
	pb_istream_t stream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), msg_len);

	message.DeviceID.funcs.decode = &read_string;
	message.DeviceID.arg = NULL;

	/* Now we are ready to decode the message. */
	ret = pb_decode(&stream, protobuf_CloudStorageInfoChangeNotify_fields, &message);
	/* Check for errors... */
	if (!ret)
	{
		printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&stream));
		return ret;
	}

	/* Print the data contained in the message. */
	if(message.DeviceID.arg)
	{
		printf("mojing_cloud_storage_change_notify message %s received!\n",(char *)message.DeviceID.arg);
		free(message.DeviceID.arg);
	}

	printf("mojing_cloud_storage_change_notify CloudStorageType:%d, StreamType:%d, PreStorageTime:%d,KeepTime:%d!\n",
		message.CloudStorageType,message.StreamType,message.PreStorageTime,message.KeepTime);

		if(message.CloudStorageType != 0)
		{
		    if (access("/opt/custom/cfg/cloud", F_OK) == 0)
    		{
				handle->cloud_storage.cloudStorageType = 1;
				printf("message cloudStorageType is %d, device cloudStorageType is 1!\n", message.CloudStorageType);
    		}
			else
				handle->cloud_storage.cloudStorageType = message.CloudStorageType;

		}
		else
			handle->cloud_storage.cloudStorageType = message.CloudStorageType;

	handle->cloud_storage.streamType = message.StreamType;
	handle->cloud_storage.preStorageTimeSec = message.PreStorageTime;
	handle->cloud_storage.keepTimeSec = message.KeepTime;

#ifdef MODULE_SUPPORT_UDP_LOG
	snprintf(msgStr, sizeof(msgStr),"streamType=%d,cloudStorageType=%d,prestorageTime=%d,keeptime=%d",\
	message.StreamType,message.CloudStorageType,message.PreStorageTime,message.KeepTime);

	mojing_log_send(NULL, 0, "dsgw_cloud_storage_info_change_notify", msgStr);
#endif

#endif

	return ret;
}

int mojing_get_register_status(void)
{
	return g_connect_handle->is_regok;
}

#ifdef MODULE_SUPPORT_UDP_LOG
char *mojing_get_version_info(void)
{
	return (char *)&softVersion[0];
}
#endif

void mojing_start_cloud_test()
{
    g_connect_handle->cloud_storage.cloudStorageType = CLOUD_STORAGE_DYNAMIC;
    g_connect_handle->cloud_storage.keepTimeSec = 20;
    g_connect_handle->cloud_storage.streamType = 0;
    g_connect_handle->cloud_storage.preStorageTimeSec = 5;
    if(cloudStorageInProcess)
    {
        printf("mojing_get_cloud_storage_addr_request cloudStorageInProcess!\n");

        if(g_connect_handle->cloud_storage.cloudStorageType == CLOUD_STORAGE_DYNAMIC)
        {
#if 0
            diff_time = handle->cloud_storage.motionEndTime - time(NULL);

            if(diff_time > 0 && diff_time <= 5)
                handle->cloud_storage.motionEndTime += handle->cloud_storage.keepTimeSec;
#else
            g_connect_handle->cloud_storage.motionEndTime = time(NULL) + g_connect_handle->cloud_storage.keepTimeSec;
#endif
        }

        return;
    }

    if(g_connect_handle->cloud_storage.cloudStorageType == CLOUD_STORAGE_DYNAMIC)
    {

        g_connect_handle->cloud_storage.motionStartTime = time(NULL);
        g_connect_handle->cloud_storage.motionEndTime = g_connect_handle->cloud_storage.motionStartTime + g_connect_handle->cloud_storage.keepTimeSec;
    }
#ifdef MODULE_SUPPORT_GB28181
    mojing_cloud_storage_start(tsTestIp, tsTestPort, 0, 123, &(g_connect_handle->cloud_storage));
#endif
    cloudStorageInProcess = 0;
}

#ifdef MODULE_SUPPORT_WORKQUEUE
static int mojing_cloud_storage_config_status_check(void *arg)
#else
static void mojing_cloud_storage_config_status_check(void)
#endif
{

	static int cloudStatusCheckCnt = 0;
	static int cloudAllDayCheckCnt = 0;

	if(netcam_is_prepare_update() || (netcam_get_update_status() != 0))
		goto exit;


#ifdef MODULE_SUPPORT_UDP_LOG
	mojing_log_sn_reset();
#endif

	if(g_connect_handle->cloud_storage.cloudStorageType == CLOUD_STORAGE_ALL_DAY && !cloudStorageInProcess)
	{
		if(++cloudAllDayCheckCnt % 5 == 0)
		{
			cloudAllDayCheckCnt = 0;
			mojing_get_cloud_storage_addr_request(CLOUD_STORAGE_ALL_DAY);
		}
	}
	else
	{
		cloudAllDayCheckCnt = 0;

		if(g_connect_handle->cloud_storage.cloudStorageStatus == CLOUD_STATUS_OK || cloudStorageInProcess)
		{
			cloudStatusCheckCnt = 0;
			goto exit;
		}

		cloudStatusCheckCnt++;

		if((g_connect_handle->cloud_storage.cloudStorageStatus == CLOUD_STATUS_GET_CONFIG) && (cloudStatusCheckCnt % 30 == 0))
		{

			cloudStatusCheckCnt = 0;
			printf("mojing_get_cloud_storage_request for CLOUD_STATUS_GET_CONFIG!\n");
			mojing_get_cloud_storage_request();
		}
		else if((g_connect_handle->cloud_storage.cloudStorageStatus == CLOUD_STATUS_NO_CLOUD_STORAGE) && (cloudStatusCheckCnt % 1800 == 0))
		{

			cloudStatusCheckCnt = 0;
			printf("mojing_get_cloud_storage_request for CLOUD_STATUS_NO_CLOUD_STORAGE!\n");
			mojing_get_cloud_storage_request();
		}
	}

exit:
#ifdef MODULE_SUPPORT_WORKQUEUE
    return 0;
#endif

}

#ifdef MODULE_SUPPORT_UDP_LOG

int socketKefen = -1;
int socketMj = -1;

int mojing_log_send(char *devid, int result, char *msg_type, char *msg_body)
{
	char dev_id[50] = {0};
	char resultStr[10] = {0};
	char msgTypeStr[100] = {0};
	char msgBodyStr[2048] = {0};
	char jsonStr[2500] = {0};
	int socket_fd = -1;
	int ret = 0;
	struct sockaddr_in servaddr;

	if(!msg_type || !msg_body)
		return -1;

#ifdef MODULE_SUPPORT_GB28181
	if(!strcmp(runGB28181Cfg.DeviceUID,"0000000000000000000") || (strstr(runGB28181Cfg.DeviceUID, "00") == NULL))
	{
		return -1;
	}
	else
	{
		strcpy(dev_id,runGB28181Cfg.DeviceUID);
	}
#else

	if(devid)
		strncpy(dev_id,devid,sizeof(dev_id)-1);
#endif

	if(result == 0)
		strncpy(resultStr,"0",sizeof(resultStr)-1);
	else
		strncpy(resultStr,"-1",sizeof(resultStr)-1);

	strncpy(msgTypeStr,msg_type,sizeof(msgTypeStr)-1);
	strncpy(msgBodyStr,msg_body,sizeof(msgBodyStr)-1);

	char *out;
	cJSON *root;
	root = cJSON_CreateObject();//创建项目

	cJSON_AddStringToObject(root, "devid", dev_id);
	cJSON_AddNumberToObject(root, "seq", sn);
	cJSON_AddStringToObject(root, "result", resultStr);
	cJSON_AddStringToObject(root, "msg_type", msgTypeStr);
	cJSON_AddStringToObject(root, "msg_body", msgBodyStr);


	out = cJSON_PrintUnformatted(root);
	//out = cJSON_Print(root);
	//printf("\n%s\n",out);
	snprintf(jsonStr, sizeof(jsonStr), "%s", out);

	free(out);

	cJSON_Delete(root);

    if (socketKefen == -1)
    {
    	socketKefen = socket(PF_INET, SOCK_DGRAM, 0);
    	if (socketKefen < 0)
    	{
    		printf("%s creat socket failed!\n",__FUNCTION__);
    		return -1;
    	}
    }

	bzero(&servaddr, sizeof(struct sockaddr_in));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = inet_addr(LOG_SERVER_IP);
	servaddr.sin_port = htons(LOG_SERVER_PORT);
	ret = sendto(socketKefen, jsonStr, strlen(jsonStr), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
	if(ret <= 0)
	{
		printf("%s send failed!\n",__FUNCTION__);
        close(socketKefen);
		socketKefen = -1;
	}

	if(strlen(MjUdpIp) > 0)
	{
        if (socketMj == -1)
        {
    		socketMj = socket(PF_INET, SOCK_DGRAM, 0);
    		if (socketMj < 0)
    		{
    			printf("%s creat socket failed!\n",__FUNCTION__);
    			return -1;
    		}
        }

		bzero(&servaddr, sizeof(struct sockaddr_in));
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = inet_addr(MjUdpIp);
		servaddr.sin_port = htons(MJ_UDP_LOG_PORT);
		ret = sendto(socketMj, jsonStr, strlen(jsonStr), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
		if(ret <= 0)
		{
			printf("%s send failed!\n",__FUNCTION__);
            close(socketMj);
		    socketMj = -1;
		}
		
	}


#ifdef SUPPORT_GOKE_LOG
    //发送到国科私有服务器
    if (strcmp(msgTypeStr, "dev_status") == 0)
    {
        timeCnt++;
    }
    if (strlen(gokeUdpIp) > 0 && (strcmp(msgTypeStr, "dev_status") != 0
        || timeCnt >= 6))
    {
        pthread_mutex_lock(&logCache);
        struct tm *ptm;
        long ts;
        int curBuffLen = strlen(logBuffer);
        struct tm tt = {0};
        char str[64] = {0};
        ts = time(NULL);
        ptm = localtime_r(&ts, &tt);//&tt
        sprintf(str, "%04d-%02d-%02d %02d:%02d:%02d", ptm->tm_year+1900,
                                                 ptm->tm_mon+1,
                                                 ptm->tm_mday,
                                                 ptm->tm_hour,
                                                 ptm->tm_min,
                                                 ptm->tm_sec);

        root = cJSON_CreateObject();//创建项目

        cJSON_AddStringToObject(root, "devid", dev_id);
        cJSON_AddNumberToObject(root, "sn", sn);
        cJSON_AddStringToObject(root, "result", resultStr);
        cJSON_AddStringToObject(root, "msg_type", msgTypeStr);
        cJSON_AddStringToObject(root, "msg_body", msgBodyStr);
        cJSON_AddStringToObject(root, "time", str);


        out = cJSON_PrintUnformatted(root);
        //out = cJSON_Print(root);
        //printf("\n%s\n",out);
        memset(jsonStr, 0, sizeof(jsonStr));
        snprintf(jsonStr, sizeof(jsonStr), "%s", out);

        free(out);

        cJSON_Delete(root);


        if (timeCnt >= 6)
        {
            timeCnt = 0;
        }

        if ((LOG_BUFF_SIZE - curBuffLen > strlen(jsonStr) + 16))
        {
            if (curBuffLen > 0)
                strcat(logBuffer, "\n");
            strcat(logBuffer, jsonStr);
        }
        else
        {
            timeCnt = 0;
            socket_fd = socket(PF_INET, SOCK_DGRAM, 0);
        	if (socket_fd < 0)
        	{
        		printf("%s creat socket failed!\n",__FUNCTION__);
                pthread_mutex_unlock(&logCache);
        		return -1;
        	}

        	bzero(&servaddr, sizeof(struct sockaddr_in));
        	servaddr.sin_family = AF_INET;
        	servaddr.sin_addr.s_addr = inet_addr(gokeUdpIp);
        	servaddr.sin_port = htons(GOKE_UDP_LOP_PORT);

            //printf("send to upd server len:%d\n", curBuffLen);
        	ret = sendto(socket_fd, logBuffer, curBuffLen, 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
        	if(ret <= 0)
        	{
        		printf("%s send failed!\n",__FUNCTION__);
        	}
            close(socket_fd);
            memset(logBuffer, 0, curBuffLen);
            strcpy(logBuffer, jsonStr);
        }
        pthread_mutex_unlock(&logCache);
    }
#endif

	sn++;

	return 0;
}

static int shell_cmd(char *cmd, char *buf, int buflen)
{
    FILE *fp = NULL;
	int len = 0;

	if(!cmd || !buf)
		return -1;

	if (NULL == (fp = popen(cmd, "r")))
	{
		goto err;
	}

	if (NULL == fgets(buf, buflen, fp))
	{
		goto err;
	}

	len = strlen(buf);
	if(len > 0)
	{
		if(buf[len-1] == '\n')
			buf[len-1] = '\0';
	}
	else
		goto err;

	pclose(fp);

	return 0;

err:
	if(fp)
		pclose(fp);

	return -1;
}
static const unsigned int tran_exp[] = {
	10000,		100000,		1000000,	10000000,
	0,		0,		0,		0
};

static const unsigned char tran_mant[] = {
	0,	10,	12,	13,	15,	20,	25,	30,
	35,	40,	45,	50,	55,	60,	70,	80,
};

int mojing_get_mmc_hw_info(unsigned int *speed, char *comid)
{
#define CMD_MAX_LEN 		128
#define	RESPOND_MAX_LEN		100
#define SYS_MMC_DIR			"/sys/bus/mmc/devices"

	char cmd[CMD_MAX_LEN];
	char result[RESPOND_MAX_LEN];
	char devnode[RESPOND_MAX_LEN];
	int i = 0, j = 0;
	unsigned int tmp[16] = {0};
	unsigned int sd_speed;

	if(!speed || !comid)
		return -1;

	memset(cmd,0,CMD_MAX_LEN);
	memset(devnode,0,RESPOND_MAX_LEN);
	sprintf(cmd,"ls %s",SYS_MMC_DIR);
	if(shell_cmd(cmd,devnode,RESPOND_MAX_LEN) < 0)
		return -1;

	printf("devnode:%s\n",devnode);

	memset(cmd,0,CMD_MAX_LEN);
	memset(result,0,RESPOND_MAX_LEN);
	sprintf(cmd,"cat %s/%s/name",SYS_MMC_DIR,devnode);
	if(shell_cmd(cmd,result,RESPOND_MAX_LEN) < 0)
		return -1;

	printf("sdname:%s\n",result);
	if(comid)
		strcpy(comid,result);

	memset(cmd,0,CMD_MAX_LEN);
	memset(result,0,RESPOND_MAX_LEN);
	sprintf(cmd,"cat %s/%s/csd",SYS_MMC_DIR,devnode);
	if(shell_cmd(cmd,result,RESPOND_MAX_LEN) < 0)
		return -1;

	printf("csd:%s\n",result);

	for(i=0; i<16; i++)
	{
		sscanf((char *)&result[j], "%02x", &tmp[i]);
		j+=2;
	}

	printf("TRAN_SPEED %x\n",tmp[3]);

	int m = (tmp[3]>>3) & 0xF;//  UNSTUFF_BITS(resp, 99, 4);
	int e = tmp[3]&0x7;//  UNSTUFF_BITS(resp, 96, 3);
	sd_speed = tran_exp[e] * tran_mant[m];
	*speed = sd_speed;

	printf("sd_speed = %u\n",sd_speed);

	return 0;
}
static void *mojing_log_send_thread(void *arg)
{
	int ret = 0,sdStatus = 0;
	char msgStr[2048] = {0};
	char netInfo[200] = {0};
	char sdInfo[200] = {0};
	char sdSotoreMD[5] = "DY";
	unsigned int total_send = 0,success_send = 0;
    ST_SDK_NETWORK_ATTR net_attr;
    struct timespec times = {0, 0};
    unsigned long time;
	int sensi;
	int retSensi = 100;
	unsigned int sdSpeed;
	char sdName[50];
	struct in_addr server;

    pthread_detach(pthread_self());
    sdk_sys_thread_set_name("mojing_log_send_thread");

#ifdef SUPPORT_GOKE_LOG
    int hostNameCheck = 0;
    ret = utility_net_resolve_host(&server,GOKE_UDP_LOG_ADDR);
    if (ret)
    {
        printf("udp get host:%s ip addr error\n", GOKE_UDP_LOG_ADDR);
    }
    else
    {
        strncpy(gokeUdpIp, inet_ntoa(server), 16);
        printf("get udp host ip ok:%s\n", gokeUdpIp);
    }
#endif

	while(1)
	{
		while(netcam_get_update_status() != 0)
			sleep(5);

		//网络信息
		memset(&net_attr, 0, sizeof(ST_SDK_NETWORK_ATTR));
		ret = netcam_net_get_detect((char*)runNetworkCfg.lan.netName);
		if (ret != 0)
		{
            strcpy(net_attr.name, (char*)runNetworkCfg.wifi.netName);

            if(netcam_net_get(&net_attr) != 0)
			{
				sleep(5);
                continue;
            }

			memset(netInfo,0,sizeof(netInfo));
			sprintf(netInfo,"netType=wifi,wifiSsid=%s,ipAddr=%s",	runNetworkCfg.wifilink[0].essid,net_attr.ip);
		}
		else
		{
            strcpy(net_attr.name, (char*)runNetworkCfg.lan.netName);

            if(netcam_net_get(&net_attr) != 0)
			{
				sleep(5);
                continue;
            }
			memset(netInfo,0,sizeof(netInfo));
			sprintf(netInfo,"netType=lan,wifissid=none,ipAddr=%s",net_attr.ip);
		}

		//SD 卡信息,
		//sdSpeed / sdComid / sdSotoreMD
		if(runRecordCfg.recordMode == 0)
			strcpy(sdSotoreMD,"MA");
		else if(runRecordCfg.recordMode == 1)
			strcpy(sdSotoreMD,"AL");
		else if(runRecordCfg.recordMode == 2 || runMdCfg.handle.is_rec)
			strcpy(sdSotoreMD,"DY");
		else if(runRecordCfg.recordMode == 3 && !runMdCfg.handle.is_rec)
			strcpy(sdSotoreMD,"OFF");

		memset(sdInfo,0,sizeof(sdInfo));
		sdStatus = mmc_get_sdcard_stauts();
		if(sdStatus == SD_STATUS_OK)
		{
			ret = mojing_get_mmc_hw_info(&sdSpeed, sdName);
			if(ret < 0)
			{
				sprintf(sdInfo,"sdTotal=%d,sdFree=%d,sdStatus=1,sdSotoreMD=%s",grd_sd_get_all_size(),grd_sd_get_free_size(),sdSotoreMD);
			}
			else
			{
				sprintf(sdInfo,"sdTotal=%d,sdFree=%d,sdStatus=1,sdSpeed=%u,sdComid=%s,sdSotoreMD=%s",grd_sd_get_all_size(),grd_sd_get_free_size(),sdSpeed,sdName,sdSotoreMD);
			}
		}
		else if(sdStatus == SD_STATUS_NOTINIT)
			strcpy(sdInfo,"sdTotal=0,sdFree=0,sdStatus=0");
		else
			strcpy(sdInfo,"sdTotal=0,sdFree=0,sdStatus=2");

		//云存推送信息
#ifdef MODULE_SUPPORT_GB28181
		mojing_cloud_storage_get_send_status(&total_send, &success_send);
#endif
		//移动侦测信息
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
                retSensi = 100;
                PRINT_INFO("get sensitivity :%d not support!\n",sensi);
                break;
        }

		if(runMdCfg.enable == 0)
			retSensi = 0;

		//开机运行时间信息
		clock_gettime(CLOCK_MONOTONIC, &times);
		time = times.tv_sec;

		//所有系统信息
		memset(msgStr,0,sizeof(msgStr));
#ifdef MODULE_SUPPORT_GB28181
		snprintf(msgStr, sizeof(msgStr),"ver=%s,%s,%s,uptime=%lu,gb2818Status=%d,dsgwStatus=%d,cloudAll=%d,cloudSuccess=%d,motionAlarm=%llu,motionLevel=%d",\
		    mojing_get_version_info(),netInfo,sdInfo,time,ite_eXosip_get_register_status(),mojing_get_register_status(),(int)total_send,(int)success_send,motionAlarmCnt,retSensi);
#else
		snprintf(msgStr, sizeof(msgStr),"ver=%s,%s,%s,uptime=%lu,gb2818Status=%d,dsgwStatus=%d,cloudAll=%d,cloudSuccess=%d,motionAlarm=%llu,motionLevel=%d",\
            mojing_get_version_info(),netInfo,sdInfo,time,0,mojing_get_register_status(),(int)total_send,(int)success_send,motionAlarmCnt,retSensi);
#endif
		if(strlen(MjUdpIp) == 0)
		{
            if (strlen(mojingEvtUploadInfo.host) > 0)
            {
			    ret = utility_net_resolve_host(&server, mojingEvtUploadInfo.host);
            }
            else
            {
			    ret = utility_net_resolve_host(&server, MJ_UDP_LOG_DOMAIN);
            }
			if (ret)
			{
				printf("utility_net_resolve_host:%s error\n", MJ_UDP_LOG_DOMAIN);
			}
			else
			{
				strncpy(MjUdpIp, inet_ntoa(server), 16);
				printf("-------->MjUdpIp :%s, %s\n", mojingEvtUploadInfo.host, MjUdpIp);
			}
		}

		ret = mojing_log_send(NULL, 0, "dev_status", msgStr);
#if 0
		if(ret < 0)
			sleep(60);
		else
#endif
		sleep(60*10);
#ifdef SUPPORT_GOKE_LOG
        hostNameCheck++;
        //20分钟更新域名
        if (hostNameCheck > 1)
        {
            ret =utility_net_resolve_host(&server,GOKE_UDP_LOG_ADDR);
            if (ret)
            {
                printf("udp get host:%s ip addr error\n", GOKE_UDP_LOG_ADDR);
            }
            else
            {
                strncpy(gokeUdpIp, inet_ntoa(server), 16);
                //printf("get udp host ip ok:%s\n", gokeUdpIp);
            }
            hostNameCheck = 0;
        }
#endif

	}

	return NULL;
}

static int mojing_log_sn_reset(void)
{

	long ts = time(NULL);
	struct tm tt = {0};
	struct tm *ptm = localtime_r(&ts, &tt);
	static int reset_flag = 1;
	
	if(ptm->tm_hour == 0 && ptm->tm_min == 0 && reset_flag)
	{
		sn = 1;
		reset_flag = 0;
		printf("###########mojing_log_sn_reset!\n");
	}
	else if(ptm->tm_hour == 23 && ptm->tm_min == 59)
	{
		if(!reset_flag)
			reset_flag = 1;
	}

	return 0;
}

int mojing_log_start(void)
{
    pthread_t pthid;

	if (0 != pthread_create(&pthid, NULL, mojing_log_send_thread, NULL))
	{
		printf("creat mojing_log_send_thread failed\n");
		return -1;
	}

	return 0;
}
#endif

int mojing_init(char *device_id)
{
	g_connect_handle = (MoJing_ConnectT*)malloc(sizeof(MoJing_ConnectT));
	memset(g_connect_handle, 0, sizeof(MoJing_ConnectT));
	g_connect_handle->ping_pong_time = MJ_PING_PONG_TIME;
	strcpy(g_connect_handle->device_id,device_id);
	strcpy(g_connect_handle->user_name,device_id);
	strcpy(g_connect_handle->psd,"123");
	g_connect_handle->exit = 1;
    g_connect_handle->pingTimes = 0;

	//software
	return 0;
}

int SetupTcpConnection(char* ip, int port)
{
    struct sockaddr_in sin;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    memset(&sin, 0, sizeof(struct sockaddr_in));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = inet_addr(ip);
    sin.sin_port = htons(port);

	printf("connect to %s, %d\n", ip, port);
    if(connect(sockfd, (struct sockaddr *)&sin, sizeof(sin)) < 0)
    {
        printf("connect server failed.errno:%d, %s\n", errno, strerror(errno));
        close(sockfd);
        return -1;
    }
	printf("connect to %s, %d success\n", ip, port);


    return sockfd;
}


#ifdef MODULE_SUPPORT_UDP_LOG
static int mojing_gw_info_status_send(int status, char *err_msg)
{
	char msgStr[200] = {0};
    char *host = NULL;
    char *path = NULL;
    int port = 80;
    char portStr[8] = {0};
    char tmpHost[128] = {0};
    char *portStart = NULL;
    struct in_addr server;
    struct sockaddr_in serv_addr;
	int ret;
    if(mojing_http_parser_url(mj_gateway,&host,&path))
    {
         printf("[mojing_get_gw_info_status_send] http_parser_url fail.\n");
         return -1;
    }

	/* lookup the ip address */
	if (strstr(host, ":") != NULL)
	{
		strncpy(tmpHost, host, strlen(host) - strlen(strstr(host, ":")));
		ret = utility_net_resolve_host(&server,tmpHost);
	}
	else
	{
		ret = utility_net_resolve_host(&server,host);
	}
	if(ret)
	{
		printf("[mojing_get_gw_info_status_send] create_request_socket gethostbyname fail.\n");
		return -1;
	}

	memset(&serv_addr,0,sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	portStart = strstr(host, ":");
	if (portStart == NULL)
	{
		if (strstr(mj_gateway, "https") != NULL)
		{
			serv_addr.sin_port = htons(443);
		}
		else
		{
			serv_addr.sin_port = htons(80);
		}
	}
	else
	{
		if (strstr(portStart, "/") != NULL)
		{
			strncpy(portStr, portStart + 1, strlen(portStart) - strlen(strstr(portStart, "/")) - 1);
		}
		else
		{
			strcpy(portStr, portStart + 1);
		}
		port = strtol(portStr, NULL, 0);
		serv_addr.sin_port = htons(port);
	}

	memcpy(&serv_addr.sin_addr,&server,sizeof(server));


	if(!status)
	{
		snprintf(msgStr, sizeof(msgStr),"ip=%s, port=%d",(char *)inet_ntoa(serv_addr.sin_addr),ntohs(serv_addr.sin_port));
	}
	else
	{

		if(!err_msg)
			snprintf(msgStr, sizeof(msgStr),"ip=%s, port=%d failed",(char *)inet_ntoa(serv_addr.sin_addr),ntohs(serv_addr.sin_port));
		else
			snprintf(msgStr, sizeof(msgStr),"%s",err_msg);
	}

    if (host != NULL)
    {
        free(host);
        host = NULL;
    }

	return mojing_log_send(NULL, status, "dsgw_get_addr", msgStr);

}

static int mojing_redirect_gw_info_status_send(int status)
{
	char msgStr[200] = {0};

	if(!status)
	{
		snprintf(msgStr, sizeof(msgStr),"redirect ip=%s, port=%d",g_connect_handle->server_addr,g_connect_handle->server_port);
	}
	else
	{
		snprintf(msgStr, sizeof(msgStr),"connect ip=%s, port=%d failed",g_connect_handle->server_addr,g_connect_handle->server_port);
	}

	return mojing_log_send(NULL, status, "dsgw_connect", msgStr);
}
#endif

int mojing_get_dw_info()
{
	int ret = 0;
    char postparm[1024]={0,};
    char response[1024]={0,};

	#if 1 // for test
	//send https msg;
	//parse sever_address and port;
	sprintf(postparm,"{\"DeviceID\":\"%s\","
		                    "\"FirmwareVersion\":\"%s\","
		                    "\"FirmwareMD5\":\"%s\","
		                    "\"SoftwareVersion\":\"%s\","
		                    "\"SoftwareMD5\":\"%s\"}",
			                g_connect_handle->device_id,softVersion,"",
			                softVersion, "");
    
#ifdef USE_LIBCURL_OPENSSL
    gadi_sys_sem_wait(libcurLock);
#endif
    ret = mojing_send_http_message(mj_gateway, postparm, response);
#ifdef USE_LIBCURL_OPENSSL
    gadi_sys_sem_post(libcurLock);
#endif
    if (ret != 0)
    {
        printf("mojing_get_dw_info error.\n");
        return -1;
    }
    printf("gate return:\n%s\n", response);

    cJSON *json = NULL;
    json = cJSON_Parse(response);
    if (!json){
        //从配置文件解析cjson失败，则使用默认参数
        printf("Error before: [%s]\n", cJSON_GetErrorPtr());
#ifdef MODULE_SUPPORT_UDP_LOG
		mojing_gw_info_status_send(-1,"parse json file failed!");
#endif
        return -1;
    }

    cJSON *item = NULL;
    item = cJSON_GetObjectItem(json, "code");
    if (!item) {
        //PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
#ifdef MODULE_SUPPORT_UDP_LOG
		mojing_gw_info_status_send(-1,"no code found in json file!");
#endif
        goto err;
    }
    printf("return code:%d\n", item->valueint);
    if (item->valueint == 200)
    {
        cJSON *gateInfo = NULL;
        gateInfo = cJSON_GetObjectItem(json, "DeviceServiceGateWayInfo");
        if (!gateInfo)
        {
#ifdef MODULE_SUPPORT_UDP_LOG
			mojing_gw_info_status_send(-1,"no DeviceServiceGateWayInfo found in json file!");
#endif
            goto err;
        }
        item = cJSON_GetObjectItem(gateInfo, "ServiceURI");
        if (!item) {
            //PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
#ifdef MODULE_SUPPORT_UDP_LOG
			mojing_gw_info_status_send(-1,"no ServiceURI found in json file!");
#endif
            goto err;
        }
        strcpy(g_connect_handle->server_addr, item->valuestring);
        item = cJSON_GetObjectItem(gateInfo, "ServicePort");
        if (!item) {
            //PRINT_ERR("Error before: [%s]\n", cJSON_GetErrorPtr());
#ifdef MODULE_SUPPORT_UDP_LOG
			mojing_gw_info_status_send(-1,"no ServicePort found in json file!");
#endif
            goto err;
        }
        g_connect_handle->server_port = item->valueint;
    }
    else
    {
        printf("get gate info error\n");

#ifdef MODULE_SUPPORT_UDP_LOG
		mojing_gw_info_status_send(-1,"code != 200 in json file!");
#endif
        ret = -1;
    }

err:
    printf("gate info, server:%s:%d\n", g_connect_handle->server_addr, g_connect_handle->server_port);
    cJSON_Delete(json);
	#else
	char res_json[256] ={0};
	char send_json[256] ={0};

	ret = mojing_open_api_send_http_message(mj_gateway,send_json,res_json);
	if(ret == 0)
	{
		ret = mojing_parse_gw_info(res_json,g_connect_handle);
	}
	#endif
	return ret;
}

int mojing_receive_msg(MoJing_ConnectT * handle)
{
	int ret;
#ifdef MODULE_SUPPORT_UDP_LOG
	char msgStr[100] = {0};
#endif

	MoJing_HeaderT *header = (MoJing_HeaderT *)handle->buffer;
	ret = mojing_ssl_recv(handle, handle->buffer, sizeof(MoJing_HeaderT),5);

	if(ret ==  sizeof(MoJing_HeaderT))
	{
		printf("mj_receive_msg:header:%x, type:%d,seq:%d, len:%d\n",header->header, header->msg_type,header->seq, header->msg_len);

        if (header->header != 0x4d56)
        {
            return -1;
        }

        if (header->msg_len >= MJ_PB_BUFFER_LEN)
        {
            return -1;
        }

		ret = mojing_ssl_recv(handle,handle->buffer+sizeof(MoJing_HeaderT),header->msg_len,5);
		if(ret == header->msg_len)
		{
			switch(header->msg_type)
			{
				case Msg_RegisterResponseWithNonce:
				{
#ifdef MOJING_USE_PROTOBUF
					goprotobuf::RegisterResponseWithNonce msg;
					msg.ParseFromArray(handle->buffer+sizeof(MoJing_HeaderT), header->msg_len);
					printf("Msg_RegisterResponseWithNonce message received,code:%d,msg:%s,noce:%s\n",msg.code(),
						msg.message().c_str(),
						msg.nonce().c_str());
					strcpy(handle->nonce, msg.nonce().c_str());
					ret = mojing_register_request_with_auth(handle);
					if(ret != 0)
					{
						printf("mojing_register_request_with_auth failed\n");
					}
					else
					{
						printf("mojing_register_request_with_auth send ok\n");
					}
#else

					/* Allocate space for the decoded message. */
					protobuf_RegisterResponseWithNonce message = protobuf_RegisterResponseWithNonce_init_default;
					/* Create a stream that reads from the buffer. */
					pb_istream_t stream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), header->msg_len);

					message.Message.funcs.decode = &read_string;
					message.Message.arg = NULL;

					message.nonce.funcs.decode = &read_string;
					message.nonce.arg = NULL;

					/* Now we are ready to decode the message. */
					ret = pb_decode(&stream, protobuf_RegisterResponseWithNonce_fields, &message);
					/* Check for errors... */
					if (!ret)
					{
						printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&stream));
						break;
					}

					/* Print the data contained in the message. */
					if(message.Message.arg)
					{
						printf("Msg_RegisterResponseWithNonce message received,code:%d,msg:%s\n",message.Code,
							(char *)message.Message.arg);
						free(message.Message.arg);
					}

					if(message.nonce.arg)
					{
						printf("Msg_RegisterResponseWithNonce message received,nonce:%s\n",(char *)message.nonce.arg);
						strcpy(handle->nonce, (char *)message.nonce.arg);
						free(message.nonce.arg);
					}

					ret = mojing_register_request_with_auth(handle);
					if(ret != 0)
					{
						printf("mojing_register_request_with_auth failed\n");
					}
					else
					{
						printf("mojing_register_request_with_auth send ok\n");
					}

#endif
					break;
				}
				case Msg_RegisterResponse:
				{
#ifdef MOJING_USE_PROTOBUF
					goprotobuf::RegisterResponse msg;
					msg.ParseFromArray(handle->buffer+sizeof(MoJing_HeaderT), header->msg_len);
					printf("RegisterResponse message received,code:%d,msg:%s\n",msg.code(),
						msg.message().c_str());
					handle->is_regok = 1;

					printf("mojing register ok\n");


					ret = mojing_upload_device_info_request(handle);
					if(ret != 0)
					{
						printf("mojing_upload_device_info_request failed\n");
					}
					else
					{
						printf("mojing_upload_device_info_request send ok\n");
					}
#else
                    long cur = time(NULL);
					/* Allocate space for the decoded message. */
					protobuf_RegisterResponse message = protobuf_RegisterResponse_init_default;
					/* Create a stream that reads from the buffer. */
					pb_istream_t stream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), header->msg_len);

					message.Message.funcs.decode = &read_string;
					message.Message.arg = NULL;

					/* Now we are ready to decode the message. */
					ret = pb_decode(&stream, protobuf_RegisterResponse_fields, &message);
					/* Check for errors... */
					if (!ret)
					{
						printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&stream));
						break;
					}

					/* Print the data contained in the message. */
					if(message.Message.arg)
					{
						printf("RegisterResponse message received,code:%d,msg:%s\n",message.Code,(char *)message.Message.arg);
						free(message.Message.arg);
					}

					handle->is_regok = 1;

					printf("mojing register ok, time:%lld\n", message.Time);
                    if (cur != message.Time && message.Time > 100)
                    {
                        struct timeval tval;
                        tval.tv_sec = message.Time;
                        tval.tv_usec = 0;
                        printf("Msg_RegisterResponse set time :%ld\n",tval.tv_sec);
                        settimeofday(&tval,NULL);
                    }

					ret = mojing_upload_device_info_request(handle);
					if(ret != 0)
					{
						printf("mojing_upload_device_info_request failed\n");
					}
					else
					{
						printf("mojing_upload_device_info_request send ok\n");
					}

                    #ifdef MODULE_SUPPORT_UDP_LOG
					memset(msgStr,0,sizeof(msgStr));
					snprintf(msgStr, sizeof(msgStr),"result=200, message=ok");
					mojing_log_send(NULL, 0, "dsgw_register_response", msgStr);
                    #endif

#endif
					break;
				}

				case Msg_UploadDeviceInfoResponse:
				{
					printf("Msg_UploadDeviceInfoResponse\n");

					mojing_get_cloud_storage_request();

                    #ifdef MODULE_SUPPORT_UDP_LOG
					memset(msgStr,0,sizeof(msgStr));
					snprintf(msgStr, sizeof(msgStr),"result=200, message=ok");
					mojing_log_send(NULL, 0, "dsgw_upload_device", msgStr);
                    #endif

					break;
				}

				case Msg_Pong:
				{
#ifdef MOJING_USE_PROTOBUF
					goprotobuf::Pong msg;
					msg.ParseFromArray(handle->buffer+sizeof(MoJing_HeaderT), header->msg_len);
					handle->last_ping_recv_time = time(NULL);
					printf("pong message received,code:%d,msg:%s, time:%d\n",msg.code(),
						msg.message().c_str(), handle->last_ping_recv_time);
#else
					/* Allocate space for the decoded message. */
					protobuf_Pong message = protobuf_Pong_init_default;
					/* Create a stream that reads from the buffer. */
					pb_istream_t stream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), header->msg_len);

					message.Message.funcs.decode = &read_string;
					message.Message.arg = NULL;

					/* Now we are ready to decode the message. */
					ret = pb_decode(&stream, protobuf_RegisterResponse_fields, &message);
					/* Check for errors... */
					if (!ret)
					{
						printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&stream));
						break;
					}

                    if (message.Code == 200)
                    {
					    handle->last_ping_recv_time = time(NULL);
                    }

                    handle->pingTimes = 0;

					/* Print the data contained in the message. */
					if(message.Message.arg)
					{
						printf("pong message received,code:%d,msg:%s,lasttime:%d,time:%lld\n",message.Code,(char *)message.Message.arg,handle->last_ping_recv_time, message.Time);
						free(message.Message.arg);
					}
                    
                    if (handle->last_ping_recv_time != message.Time && message.Time > 100)
                    {
                        struct timeval tval;
                        tval.tv_sec = message.Time;
                        tval.tv_usec = 0;
                        printf("Msg_Pong set time :%ld\n",tval.tv_sec);
                        settimeofday(&tval,NULL);
                    }

#endif
					break;
				}
				case Msg_DeviceUpdateRequest:
				{
#ifdef MOJING_USE_PROTOBUF

					goprotobuf::DeviceUpdateRequest msg;
					msg.ParseFromArray(handle->buffer+sizeof(MoJing_HeaderT), header->msg_len);


					printf("DeviceUpdateRequest message received,"
						"type:%d\n"
						"url:%s\n"
						"md5:%s\n"
						"ver:%s\n"
						"tid:%s\n",
						msg.updatetype(),
						msg.wareurl().c_str(),
						msg.waremd5().c_str(),
						msg.wareversion().c_str(),
						msg.taskid().c_str());

					ret = mojing_device_update_response(handle);
					if(ret != 0)
					{
						printf("mojing_device_update_response failed\n");
					}
					else
					{
						printf("mojing_device_update_response send ok\n");
					}

					handle->update_type = msg.updatetype();
					if (strcmp(handle->update_ver,msg.wareversion().c_str()) != 0)
					{
						strcpy(handle->update_url,msg.wareurl().c_str());
						strcpy(handle->update_md5,msg.waremd5().c_str());
						strcpy(handle->update_ver,msg.wareversion().c_str());
						strcpy(handle->update_tid,msg.taskid().c_str());

						//mojing_upgrade_save_file(handle);
						mojing_device_update_notify(handle);

                        gadi_sys_sem_wait(libcurLock);
						mojing_upgrade_get_file(handle);
                        gadi_sys_sem_post(libcurLock);
					}
					else
					{
						printf("update ver:%s is the same\n", handle->update_ver);
					}

#else
					/* Allocate space for the decoded message. */
					protobuf_DeviceUpdateRequest message = protobuf_DeviceUpdateRequest_init_zero;
					/* Create a stream that reads from the buffer. */
					pb_istream_t stream = pb_istream_from_buffer((const pb_byte_t *)(handle->buffer+sizeof(MoJing_HeaderT)), header->msg_len);

					message.wareURL.funcs.decode = &read_string;
					message.wareURL.arg = NULL;
					message.wareVersion.funcs.decode = &read_string;
					message.wareVersion.arg = NULL;
					message.wareMD5.funcs.decode = &read_string;
					message.wareMD5.arg = NULL;
					message.TaskID.funcs.decode = &read_string;
					message.TaskID.arg = NULL;

					/* Now we are ready to decode the message. */
					ret = pb_decode(&stream, protobuf_DeviceUpdateRequest_fields, &message);
					/* Check for errors... */
					if (!ret)
					{
						printf("%s,LINE:%d,Decoding failed: %s\n", __FUNCTION__,__LINE__,PB_GET_ERROR(&stream));
						break;
					}


					printf("DeviceUpdateRequest message received:\n");

					printf("type:%d, message.KeepSilence:%d\n",message.UpdateType, message.KeepSilence);

					if(message.wareURL.arg)
						printf("url:%s\n",(char *)message.wareURL.arg);

					if(message.wareMD5.arg)
						printf("md5:%s\n",(char *)message.wareMD5.arg);

					if(message.wareVersion.arg)
						printf("ver:%s\n",(char *)message.wareVersion.arg);

					if(message.TaskID.arg)
						printf("tid:%s\n",(char *)message.TaskID.arg);

					ret = mojing_device_update_response(handle);
					if(ret != 0)
					{
						printf("mojing_device_update_response failed\n");
					}
					else
					{
						printf("mojing_device_update_response send ok\n");
					}

					handle->update_type = message.UpdateType;

					if(message.wareVersion.arg == NULL)
					{
						if(message.wareURL.arg)
						{
							strcpy(handle->update_url,(char *)message.wareURL.arg);
							free(message.wareURL.arg);
						}

						if(message.wareMD5.arg)
						{
							strcpy(handle->update_md5,(char *)message.wareMD5.arg);
							free(message.wareMD5.arg);
						}

						if(message.TaskID.arg)
						{
							strcpy(handle->update_tid,(char *)message.TaskID.arg);
							free(message.TaskID.arg);
						}

						printf("update ver is NULL! drop upgrade msg!\n");

						break;
					}

					if (strcmp(handle->update_ver,(char *)message.wareVersion.arg) != 0)
					{
						if(message.wareURL.arg)
						{
							strcpy(handle->update_url,(char *)message.wareURL.arg);
							free(message.wareURL.arg);
						}

						if(message.wareMD5.arg)
						{
							strcpy(handle->update_md5,(char *)message.wareMD5.arg);
							free(message.wareMD5.arg);
						}

						if(message.wareVersion.arg)
						{
							memset(handle->update_ver,0,sizeof(handle->update_ver));
							strcpy(handle->update_ver,(char *)message.wareVersion.arg);
							free(message.wareVersion.arg);
						}

						if(message.TaskID.arg)
						{
							strcpy(handle->update_tid,(char *)message.TaskID.arg);
							free(message.TaskID.arg);
						}

						//mojing_upgrade_save_file(handle);
						mojing_device_update_notify(handle);

						if (strcmp(handle->update_ver,softVersion) != 0)
						{
                            if (message.KeepSilence == 1)
                            {
                                runAudioCfg.rebootMute = 1;
                                AudioCfgSave();
                            }
                            
							printf("update_ver %s,device version:%s \n", handle->update_ver,softVersion);
                        #ifdef USE_LIBCURL_OPENSSL
							gadi_sys_sem_wait(libcurLock);
							mojing_upgrade_get_file(handle);
							gadi_sys_sem_post(libcurLock);
                        #else
							mojing_upgrade_get_file(handle);
                        #endif
						}
						else
							printf("device version:%s is the same\n", softVersion);

					}
					else
					{
						printf("update ver:%s is the same\n", handle->update_ver);
					}
#endif
					break;
				}
                case Msg_DeviceFlipRequest:
                {
                    printf("Msg_DeviceFlipRequest\n");
#ifdef MOJING_USE_PROTOBUF

#else
                    int status = 200;
                    GK_NET_IMAGE_CFG stImagingConfig;

                    memset(&stImagingConfig, 0, sizeof(GK_NET_IMAGE_CFG));
                    ret = netcam_image_get(&stImagingConfig);
                    printf("old flipEnabled:%d,mirrorEnabled:%d\n",stImagingConfig.flipEnabled,stImagingConfig.mirrorEnabled);
                    if (ret != 0)
                        status = 400;
                    else
                    {
                        stImagingConfig.flipEnabled = !stImagingConfig.flipEnabled;

                        stImagingConfig.mirrorEnabled = !stImagingConfig.mirrorEnabled;
                        PRINT_INFO("new flipEnabled:%d,mirrorEnabled:%d\n",stImagingConfig.flipEnabled,stImagingConfig.mirrorEnabled);

                        ret = netcam_image_set(stImagingConfig);
                    }
                    mojing_device_operate_response(handle, status, Msg_DeviceFlipResponse);
#endif
#ifdef MODULE_SUPPORT_GOKE_UPGRADE
                    //if app request flip, refresh now
                    goke_upgrade_fresh_now();
#endif
                    break;
                }
                case Msg_DeviceGetSDCardInfoRequest:
                {
                    printf("Msg_DeviceGetSDCardInfoRequest\n");
#ifdef MOJING_USE_PROTOBUF


#else
                    mojing_device_sd_info_response(handle);
#endif
                    break;
                }
                case Msg_DeviceSdFormatRequest:
                {
                    printf("Msg_DeviceSdFormatRequest\n");
#ifdef MOJING_USE_PROTOBUF

#else
                    grd_sd_format();
                    netcam_timer_add_task(mojing_sd_format_check, NETCAM_TIMER_ONE_SEC, SDK_TRUE, SDK_FALSE);
                    mojing_device_operate_response(handle, 200, Msg_DeviceSdFormatResponse);
#endif
                    break;
                }

				case Msg_GetCloudStorageInfoResponse:
				{
					printf("Msg_GetCloudStorageInfoResponse\n");
#ifdef MOJING_USE_PROTOBUF

#else
					mojing_get_cloud_storage_response(header->msg_len);
#endif
					break;
				}
				case Msg_CloudStorageInfoChangeNotify:
				{
					printf("Msg_CloudStorageInfoChangeNotify\n");
#ifdef MOJING_USE_PROTOBUF

#else
					mojing_cloud_storage_change_notify(header->msg_len);
					mojing_cloud_storage_change_response();
#endif
					break;
				}
				case Msg_GetCloudStorageAddrResponse:
                {
                    printf("Msg_GetCloudStorageAddrResponse\n");
#ifdef MOJING_USE_PROTOBUF

#else
					mojing_get_cloud_storage_addr_response(header->msg_len);
#endif
                    break;
                }
				case Msg_CloudStorageOperateResponse:
                {
                    printf("Msg_CloudStorageOperateResponse\n");
#ifdef MOJING_USE_PROTOBUF

#else
					mojing_cloud_storage_end_notify_response(header->msg_len);
#endif
                    break;
                }
#ifdef MODULE_SUPPORT_MOJING_V4
//#if SUPPORT_MOJING_V4				
				case Msg_GetAudioConfigRequest:
				{
					printf("Msg_GetAudioConfigRequest\n");
					mojing_get_audio_config_response(handle, header->msg_len);
					break;
				}
				
				case Msg_SetAudioConfigRequest:
				{
					printf("Msg_SetAudioConfigRequest\n");
					mojing_set_audio_config_response(handle, header->msg_len);
					break;
				}
				case Msg_OSDInfoRequest:
				{
					printf("Msg_OSDInfoRequest\n");
					mojing_osd_info_response(handle, header->msg_len);
					break;
				}
				case Msg_SetOSDConfigRequest:
				{
					printf("Msg_SetOSDConfigRequest\n");
					mojing_set_osd_config_response(handle, header->msg_len);
					break;
				}

                case Msg_GetMDStatusRequest:
                {
					printf("Msg_GetMDStatusRequest\n");
					mojing_get_md_status_response(handle);
					break;
                }

                case Msg_SetMDStatusRequest:
                {
					printf("Msg_SetMDStatusRequest\n");
                    mojing_set_md_status_response(handle, header->msg_len);
                    break;
                }

                case Msg_GetAreaAlarmStatusRequest:
                {
					printf("Msg_GetAreaAlarmStatusRequest\n");
                    mojing_get_area_alarm_status_response(handle, header->msg_len);
                    break;
                }

                case Msg_SetAreaAlarmStatusRequest:
                {
					printf("Msg_SetAreaAlarmStatusRequest\n");
                    mojing_set_area_alarm_status_response(handle, header->msg_len);
                    break;
                }

                case Msg_GetEventAlarmOutRequest:
                {
					printf("Msg_GetEventAlarmOutRequest\n");
                    mojing_get_event_alarm_out_response(handle, header->msg_len);
                    break;
                }

                case Msg_SetEventAlarmOutRequest:
                {
					printf("Msg_SetEventAlarmOutRequest\n");
                    mojing_set_event_alarm_out_response(handle, header->msg_len);
                    break;
                }

                case Msg_SetManualAlarmRequest:
                {
                    printf("Msg_SetManualAlarmRequest\n");
                    mojing_set_manual_alarm_response(handle, header->msg_len);
                    break;
                }

                case Msg_SetAudioAlarmFileRequest:
                {
                    printf("Msg_SetAudioAlarmFileRequest\n");
                    mojing_set_audio_alarm_file_response(handle, header->msg_len);
                    break;
                }

                case Msg_GetNightVisionConfigRequest:
                {
                    printf("Msg_GetNightVisionConfigRequest\n");
                    mojing_get_nightVision_cfg_response(handle, header->msg_len);
                    break;
                }
                
                case Msg_SetNightLightTypeRequest:
                {
                    printf("Msg_SetNightLightTypeRequest\n");
                    mojing_set_nightVision_type_response(handle, header->msg_len);
                    break;
                }

                case Msg_SetLightnessRequest:
                {
                    printf("Msg_SetLightnessRequest\n");
                    mojing_set_nightVision_lightness_response(handle, header->msg_len);
                    break;
                }
                    				
				case Msg_SetEventUploadRequest:
				{
					printf("Msg_SetEventUploadRequest\n");
					mojing_event_upload_info_response(handle,header->msg_len);
					break;
				}
				case Msg_GetGBConfigRequest:
				{
					printf("Msg_GetGBConfigRequest\n");
					mojing_get_gb_config_response(handle);
					break;
				}
				case Msg_UpdateGBConfigRequest:
				{
					printf("Msg_UpdateGBConfigRequest\n");
					mojing_update_gb_config_response(handle,header->msg_len);
					break;
				}
				case Msg_GetVideoConfigRequest:
				{
					printf("Msg_GetVideoConfigRequest\n");
					mojing_get_video_config_response(handle,header->msg_len);
					break;
				}
				case Msg_SetVideoConfigReuqest:
				{
					printf("Msg_SetVideoConfigReuqest\n");
					mojing_set_video_config_response(handle,header->msg_len);
					break;
				}
				case Msg_GetLocalStorageConfigReuqest:
				{
					printf("Msg_GetLocalStorageConfigReuqest\n");
					mojing_get_local_rec_info_response(handle);
					break;
				}
				case Msg_SetLocalStorageConfigReuqest:
				{
					printf("Msg_SetLocalStorageConfigReuqest\n");
					mojing_set_local_rec_info_response(handle,header->msg_len);
					break;
				}
				case Msg_GetImageConfigRequest:
				{
					printf("Msg_GetImageConfigRequest\n");
					mojing_get_image_config_response(handle,header->msg_len);
					break;
				}
				case Msg_SetImageConfigRequest:
				{
					printf("Msg_SetImageConfigRequest\n");
					mojing_set_image_config_response(handle,header->msg_len);
					break;
				}
				case Msg_GetTimingConfigRequest:
				{
					printf("Msg_GetTimingConfigRequest\n");
					mojing_get_timing_config_response(handle);
					break;
				}
				case Msg_SetTimingConfigRequest:
				{
					printf("Msg_SetTimingConfigRequest\n");
					mojing_set_timing_config_response(handle,header->msg_len);
					break;
				}
				case Msg_RebootRequest:
				{
					printf("Msg_RebootRequest\n");
					mojing_reboot_response(handle);
					break;
				}
				case Msg_FactoryDefaultRequest:
				{
					printf("Msg_FactoryDefaultRequest\n");
					mojing_factory_default_response(handle);
					break;
				}
				case Msg_GetNetInfoRequest:
				{
					printf("Msg_GetNetInfoRequest\n");
					mojing_get_net_info_response(handle);
					break;
				}
				case Msg_UpdateNetInfoRequest:
				{
					printf("Msg_UpdateNetInfoRequest\n");
					mojing_update_net_info_response(handle,header->msg_len);
					break;
				}
				case Msg_GetDeviceOnvifRequest:
				{
					printf("Msg_GetDeviceOnvifRequest\n");
					mojing_get_device_onvif_response(handle,header->msg_len);
					break;
				}
				case Msg_SetDeviceOnvifRequest:
				{
					printf("Msg_SetDeviceOnvifRequest\n");
					mojing_set_device_onvif_response(handle,header->msg_len);
					break;
				}
                
#endif
				default:
				{
					printf("Unknown msg, no handle\n");
					ret = 0;
					break;
				}


			}
		}

	}
    else if (ret == 0)
    {
        ret = -1;
    }

	return ret;
}

void * mojing_event_hanle(void *arg)
{
	//MoJing_ConnectT * handle;
	int ret;
    int retryCheck = 0;
    int msgCheck = 0;
	MoJing_ConnectT * handle;
    sdk_sys_thread_set_name("mojing");
	pthread_detach(pthread_self());
	// add get server address and

	handle = g_connect_handle;

	handle->cloud_storage.cloudStorageStatus = CLOUD_STATUS_GET_CONFIG;
	handle->cloud_storage.notify_end_cb = mojing_cloud_storage_end_notify;
	handle->cloud_storage.status_check_cb = mojing_cloud_storage_status_check;
	handle->cloud_storage.tls_init_cb = mojing_cloud_storage_tls_init;
	handle->cloud_storage.tls_send_cb = mojing_cloud_storage_tls_send;
	handle->cloud_storage.tls_exit_cb = mojing_cloud_storage_tls_exit;

#ifdef MODULE_SUPPORT_WORKQUEUE
	CREATE_WORK(MjCloudStatusCheck, TIME_TIMER_WORK, (WORK_CALLBACK)mojing_cloud_storage_config_status_check);
	INIT_WORK(MjCloudStatusCheck, COMPUTE_TIME(0,0,0,1,0), NULL);
	SCHEDULE_DEFAULT_WORK(MjCloudStatusCheck);
#else

	netcam_timer_add_task(mojing_cloud_storage_config_status_check, NETCAM_TIMER_ONE_SEC, SDK_TRUE, SDK_FALSE);

#endif

    //wait until the dhcp ok
    sleep(5);
#ifdef MODULE_SUPPORT_MOJING_V4
    mojing_osd_compositions_info_load();
#endif

	while(handle->exit)
	{
		if(handle->server_port == 0)
		{
			ret = mojing_get_dw_info();
			if(ret != 0)
			{
				MjEvtTlsFailCnt++;
				sleep(60);
				printf("Get mojing server error... again\n");
				continue;
			}

#ifdef MODULE_SUPPORT_UDP_LOG
			mojing_gw_info_status_send(0,NULL);
#endif
		}
		if(handle->ssl == 0)
		{
			ret = mojing_ssl_init(handle);
			if(ret != 0)
			{
#ifdef MODULE_SUPPORT_UDP_LOG
				mojing_redirect_gw_info_status_send(-1);
#endif
				MjEvtTlsFailCnt++;

				sleep(20);
                retryCheck++;
				printf("Connect mojing server error... again\n");
                if (retryCheck > 20)
                {
                    handle->server_port = 0;
                    retryCheck = 0;
                }
				continue;
			}
			else
			{
                retryCheck = 0;
                //netcam_sys_operation(0,SYSTEM_OPERATION_HARD_DEFAULT);
				printf("Connect mojing server ok\n");

#ifdef MODULE_SUPPORT_UDP_LOG
				mojing_redirect_gw_info_status_send(0);
#endif
			}
		}
		ret = mj_regrister(handle);
		printf("mj_regrister ret:%d\n", ret);
		if(ret != 0)
		{
			MjEvtTlsFailCnt++;
			printf("Restart to resigter\n");
			goto REATART_HEDGW;
		}
		while(1)
		{
            if(handle->is_regok == 0)
            {
                retryCheck++;
                if (retryCheck > 60)
                {
                    retryCheck = 0;
					MjEvtTlsFailCnt++;
                    printf("wait 60s timeout to regist ok.\n");
                    goto REATART_HEDGW;
                }
                sleep(1);
            }

            if (mojing_send_ping(handle) < 0)
            {
				MjEvtTlsFailCnt++;
                printf(" send ping error, maybe disconnect, restart hedgw.\n");
                // send ping error, maybe disconnect, restart hedgw. or record video to sd_card.
                goto REATART_HEDGW; // error, exit 服务，重新开始
            }


            ret = mojing_ssl_wait_event(handle, 5);
            if (ret == 0) {//timeout. send ping.
                continue;
            } else if (ret < 0) {
				MjEvtTlsFailCnt++;
                printf("Wait event error,regsiter it");
                goto REATART_HEDGW; // error, exit 服务，重新开始
            } else {
                printf("ReadHedgwMsg............\n");
                ret = mojing_receive_msg(handle);
                if(ret < 0)
                {
					MjEvtTlsFailCnt++;
                    printf("ReadHedgwMsg error ,register it");
                    sleep(20);
                    goto REATART_HEDGW;
                }
                else if (ret == 0)
                {
                    //printf("ReadHedgwMsg 0 ,wait it");
                    msgCheck++;
                    if (msgCheck > 120)
                    {
                        printf("ReadHedgwMsg 0 msg too many\n");
						MjEvtTlsFailCnt++;
                        msgCheck = 0;
                        goto REATART_HEDGW;
                    }
                    usleep(500000);
                }
                else
                {
                    msgCheck = 0;
                    continue;
                }
            }
		}
		REATART_HEDGW:

		mojing_ssl_exit(handle);
		handle->last_ping_recv_time = handle->last_ping_time = 0;
		handle->is_regok = 0;
        retryCheck = 0;
		sleep(1);
	}

	return NULL;
}

	
int mojing_start(void)
{
    pthread_t thread_id;

    char device_id[32] = {0};
	//if (!strcmp(runGB28181Cfg.DeviceUID,"0000000000000000000"))
	{
		struct device_info_mtd info;
		memset(&info,0,sizeof(struct device_info_mtd));
		int ret = load_info_to_mtd_reserve(MTD_TUTK_P2P, &info, sizeof(struct device_info_mtd));
		if((ret < 0) || (info.device_id[0] == 0))
		{
			printf("Fail to load gb28181 id from flash!\n");
		}
		else
		{
			printf("load gb28181 id:%s\n", info.device_id);
			strcpy(device_id,info.device_id);
		}
	}
    #if 0
	else
		strcpy(device_id,runGB28181Cfg.DeviceUID);
    #endif


    //verLen = strlen(runSystemCfg.deviceInfo.upgradeVersion) - 1 - strlen(strrchr(runSystemCfg.deviceInfo.upgradeVersion, '.')) + 11;
	//strncpy(softVersion,(char *)&runSystemCfg.deviceInfo.upgradeVersion[1], verLen);//drop secords for web upgrade manager.
    //写死，每次更新测试后版本在此更新
	if(!strcmp(sdk_cfg.name, "CUS_HC_GK7202_GC2053_V10"))
	{
		memset(softVersion,0,sizeof(softVersion));
		strcpy(softVersion, "V30CGK01BPWL02P00");
	}
	else if(!strcmp(sdk_cfg.name, "CUS_ELIFE_38_GC2053"))
	{
		memset(softVersion,0,sizeof(softVersion));
		strcpy(softVersion, "2.1.9.2019112520");
	}
    #if 1
    else if (!strcmp(sdk_cfg.name, "CUS_TB_38_BIG_GC2053_V20"))
    {
		memset(softVersion,0,sizeof(softVersion));
		strcpy(softVersion, "V31CGK03GPWL02P01");
    }
    else if (!strcmp(sdk_cfg.name, "CUS_TB_GC2053_V20"))
    {
		memset(softVersion,0,sizeof(softVersion));
		strcpy(softVersion, "V31CGK02B0WL02P01");
    }
    else if (!strcmp(sdk_cfg.name, "CUS_TB_38_BIG_GC2053_V30"))
    {
		memset(softVersion,0,sizeof(softVersion));
		strcpy(softVersion, "V31CGK06GP0L02P01");
    }
    else if (!strcmp(sdk_cfg.name, "CUS_TB_38_BIG_GC2053_V40")
        || !strcmp(sdk_cfg.name, "CUS_TB_38_BIG_GC2053_V20_V4")
        || !strcmp(sdk_cfg.name, "CUS_HI3516EV300_IMX335"))
    {
		memset(softVersion,0,sizeof(softVersion));
		strcpy(softVersion, "V40CGK06GP0L02P01");
    }
    else if (!strcmp(sdk_cfg.name, "CUS_GK7205S_GC4653_IPC_20") 
        || !strcmp(sdk_cfg.name, "CUS_TB_C142V3"))
    {
		memset(softVersion,0,sizeof(softVersion));
		//strcpy(softVersion, "V40CGK09GP0L04P01");
		strcpy(softVersion, "V40CGK07GP0L04P01");
    }
    else if (!strcmp(sdk_cfg.name, "CUS_TB_GC2053_V30"))
    {
        memset(softVersion,0,sizeof(softVersion));
        strcpy(softVersion, "V40CGK08B0WL02P01");
    }
    #endif
	else
	{
	    strcpy(softVersion, "1.0.16403.2019111215");
	}


	printf("mojing softVersion :%s, id:%s\n",softVersion, device_id);
    if (strcmp(device_id, "000000000") == 0)
    {
        return 0;
    }
    
#ifdef MODULE_SUPPORT_MOFANG
    printf("support mofang\n");
    mofang_plug_start(device_id, sizeof(device_id));
#endif

    mojing_init(device_id);

    //for test server
    if (access("/opt/custom/cfg/mojing", F_OK) == 0)
    {
        FILE *urlFile = NULL;
        urlFile = fopen("/opt/custom/cfg/mojing", "r");
        if (urlFile != NULL)
        {
            memset(mj_gateway, 0, sizeof(mj_gateway));
            fgets(mj_gateway, 256, urlFile);
            fclose(urlFile);
            printf("get new url:%s\n", mj_gateway);
        }
    }

#ifdef USE_LIBCURL_OPENSSL
    libcurLock = gadi_sys_sem_create(1);
	curl_global_init(CURL_GLOBAL_ALL);
#endif

    printf("\n mojing start\n");
    //strcpy(g_connect_handle->update_url, "https://hd.wei.pm/gk7202_sc4236_aijia_gk7202_in_V1234_app.bin");
    //mojing_upgrade_get_file(g_connect_handle);
    if(0 != pthread_create(&thread_id, NULL, mojing_event_hanle, NULL))
    {
        printf("thread error\n");
        return -1;
    }

#ifdef MODULE_SUPPORT_UDP_LOG
	mojing_log_start();
#endif

#ifdef MODULE_SUPPORT_MOJING_V4
	mojing_event_upload_info_load();	
    
	//mojing_osd_compositions_info_load();
#endif

	return 0;
}
/*

int main(int argc, char *argv[])
{
	pthread_t thread_id;

	char device_id[]="08101021122A3A00000";
	mojing_init(device_id);

	printf("\n he mu module start\n");
	if(0 != pthread_create(&thread_id, NULL,mojing_event_hanle, NULL))
    {
        printf("thread error\n");
        return -1;
    }

	while(1) sleep(1);
	return 0;
}
*/
