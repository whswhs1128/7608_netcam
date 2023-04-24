#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "adi_types.h"
#include "adi_gpio.h"
#include "doorbell_func.h"
#include "cfg_system.h"

#define SIMPLE_MSG_SERVER 	"183.232.25.234"
#define SIMPLE_MSG_URL 		"http://api.jpush.cn:8800/v2/push"
#define SIMPLE_MSG_HOST 	"api.jpush.cn"
#define SIMPLE_MSG_PORT		8800
static unsigned long TimePush = 0;	// 上一次推送的时间
#define IDC_PUSH_FERQ	30	// 报警默认推送频率



static unsigned char PADDING[]={
	0x80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0   ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0   ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0   ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};


static SDK_HandleT GPIOHands[32] = {0};


typedef struct
{
	unsigned int count[2];
	unsigned int state[4];
	unsigned char buffer[64];   
}MD5_CTX;

#define F(x,y,z) ((x & y) | (~x & z))
#define G(x,y,z) ((x & z) | (y & ~z))
#define H(x,y,z) (x^y^z)
#define I(x,y,z) (y ^ (x | ~z))
#define ROTATE_LEFT(x,n) ((x << n) | (x >> (32-n)))
#define FF(a,b,c,d,x,s,ac) \
{ \
	a += F(b,c,d) + x + ac; \
	a = ROTATE_LEFT(a,s); \
	a += b; \
}
#define GG(a,b,c,d,x,s,ac) \
{ \
	a += G(b,c,d) + x + ac; \
	a = ROTATE_LEFT(a,s); \
	a += b; \
}
#define HH(a,b,c,d,x,s,ac) \
{ \
	a += H(b,c,d) + x + ac; \
	a = ROTATE_LEFT(a,s); \
	a += b; \
}
#define II(a,b,c,d,x,s,ac) \
{ \
	a += I(b,c,d) + x + ac; \
	a = ROTATE_LEFT(a,s); \
	a += b; \
}


static void MD5Init(MD5_CTX *context)
{
	memset(context,0,sizeof(MD5_CTX));
	context->count[0] = 0;
	context->count[1] = 0;
	context->state[0] = 0x67452301;
	context->state[1] = 0xEFCDAB89;
	context->state[2] = 0x98BADCFE;
	context->state[3] = 0x10325476;
}

static void MD5Decode(
	unsigned int *output,
	unsigned char *input,
	unsigned int len
){
	unsigned int i = 0,j = 0;
	while(j < len)
	{
		output[i] = (input[j]) |
			(input[j+1] << 8) |
			(input[j+2] << 16) |
			(input[j+3] << 24);
		i++;
		j+=4; 
	}
}


static void MD5Transform(
	unsigned int state[4],
	unsigned char block[64]
){
	unsigned int a = state[0];
	unsigned int b = state[1];
	unsigned int c = state[2];
	unsigned int d = state[3];
	unsigned int x[64];
	MD5Decode(x,block,64);
	FF(a, b, c, d, x[ 0], 7, 0xd76aa478); /* 1 */
	FF(d, a, b, c, x[ 1], 12, 0xe8c7b756); /* 2 */
	FF(c, d, a, b, x[ 2], 17, 0x242070db); /* 3 */
	FF(b, c, d, a, x[ 3], 22, 0xc1bdceee); /* 4 */
	FF(a, b, c, d, x[ 4], 7, 0xf57c0faf); /* 5 */
	FF(d, a, b, c, x[ 5], 12, 0x4787c62a); /* 6 */
	FF(c, d, a, b, x[ 6], 17, 0xa8304613); /* 7 */
	FF(b, c, d, a, x[ 7], 22, 0xfd469501); /* 8 */
	FF(a, b, c, d, x[ 8], 7, 0x698098d8); /* 9 */
	FF(d, a, b, c, x[ 9], 12, 0x8b44f7af); /* 10 */
	FF(c, d, a, b, x[10], 17, 0xffff5bb1); /* 11 */
	FF(b, c, d, a, x[11], 22, 0x895cd7be); /* 12 */
	FF(a, b, c, d, x[12], 7, 0x6b901122); /* 13 */
	FF(d, a, b, c, x[13], 12, 0xfd987193); /* 14 */
	FF(c, d, a, b, x[14], 17, 0xa679438e); /* 15 */
	FF(b, c, d, a, x[15], 22, 0x49b40821); /* 16 */

	/* Round 2 */
	GG(a, b, c, d, x[ 1], 5, 0xf61e2562); /* 17 */
	GG(d, a, b, c, x[ 6], 9, 0xc040b340); /* 18 */
	GG(c, d, a, b, x[11], 14, 0x265e5a51); /* 19 */
	GG(b, c, d, a, x[ 0], 20, 0xe9b6c7aa); /* 20 */
	GG(a, b, c, d, x[ 5], 5, 0xd62f105d); /* 21 */
	GG(d, a, b, c, x[10], 9,  0x2441453); /* 22 */
	GG(c, d, a, b, x[15], 14, 0xd8a1e681); /* 23 */
	GG(b, c, d, a, x[ 4], 20, 0xe7d3fbc8); /* 24 */
	GG(a, b, c, d, x[ 9], 5, 0x21e1cde6); /* 25 */
	GG(d, a, b, c, x[14], 9, 0xc33707d6); /* 26 */
	GG(c, d, a, b, x[ 3], 14, 0xf4d50d87); /* 27 */
	GG(b, c, d, a, x[ 8], 20, 0x455a14ed); /* 28 */
	GG(a, b, c, d, x[13], 5, 0xa9e3e905); /* 29 */
	GG(d, a, b, c, x[ 2], 9, 0xfcefa3f8); /* 30 */
	GG(c, d, a, b, x[ 7], 14, 0x676f02d9); /* 31 */
	GG(b, c, d, a, x[12], 20, 0x8d2a4c8a); /* 32 */

	/* Round 3 */
	HH(a, b, c, d, x[ 5], 4, 0xfffa3942); /* 33 */
	HH(d, a, b, c, x[ 8], 11, 0x8771f681); /* 34 */
	HH(c, d, a, b, x[11], 16, 0x6d9d6122); /* 35 */
	HH(b, c, d, a, x[14], 23, 0xfde5380c); /* 36 */
	HH(a, b, c, d, x[ 1], 4, 0xa4beea44); /* 37 */
	HH(d, a, b, c, x[ 4], 11, 0x4bdecfa9); /* 38 */
	HH(c, d, a, b, x[ 7], 16, 0xf6bb4b60); /* 39 */
	HH(b, c, d, a, x[10], 23, 0xbebfbc70); /* 40 */
	HH(a, b, c, d, x[13], 4, 0x289b7ec6); /* 41 */
	HH(d, a, b, c, x[ 0], 11, 0xeaa127fa); /* 42 */
	HH(c, d, a, b, x[ 3], 16, 0xd4ef3085); /* 43 */
	HH(b, c, d, a, x[ 6], 23,  0x4881d05); /* 44 */
	HH(a, b, c, d, x[ 9], 4, 0xd9d4d039); /* 45 */
	HH(d, a, b, c, x[12], 11, 0xe6db99e5); /* 46 */
	HH(c, d, a, b, x[15], 16, 0x1fa27cf8); /* 47 */
	HH(b, c, d, a, x[ 2], 23, 0xc4ac5665); /* 48 */

	/* Round 4 */
	II(a, b, c, d, x[ 0], 6, 0xf4292244); /* 49 */
	II(d, a, b, c, x[ 7], 10, 0x432aff97); /* 50 */
	II(c, d, a, b, x[14], 15, 0xab9423a7); /* 51 */
	II(b, c, d, a, x[ 5], 21, 0xfc93a039); /* 52 */
	II(a, b, c, d, x[12], 6, 0x655b59c3); /* 53 */
	II(d, a, b, c, x[ 3], 10, 0x8f0ccc92); /* 54 */
	II(c, d, a, b, x[10], 15, 0xffeff47d); /* 55 */
	II(b, c, d, a, x[ 1], 21, 0x85845dd1); /* 56 */
	II(a, b, c, d, x[ 8], 6, 0x6fa87e4f); /* 57 */
	II(d, a, b, c, x[15], 10, 0xfe2ce6e0); /* 58 */
	II(c, d, a, b, x[ 6], 15, 0xa3014314); /* 59 */
	II(b, c, d, a, x[13], 21, 0x4e0811a1); /* 60 */
	II(a, b, c, d, x[ 4], 6, 0xf7537e82); /* 61 */
	II(d, a, b, c, x[11], 10, 0xbd3af235); /* 62 */
	II(c, d, a, b, x[ 2], 15, 0x2ad7d2bb); /* 63 */
	II(b, c, d, a, x[ 9], 21, 0xeb86d391); /* 64 */
	state[0] += a;
	state[1] += b;
	state[2] += c;
	state[3] += d;
}

static void MD5Encode(
	unsigned char *output,
	unsigned int *input,
	unsigned int len
){
	unsigned int i = 0,j = 0;
	while(j < len)
	{
		output[j] = input[i] & 0xFF;  
		output[j+1] = (input[i] >> 8) & 0xFF;
		output[j+2] = (input[i] >> 16) & 0xFF;
		output[j+3] = (input[i] >> 24) & 0xFF;
		i++;
		j+=4;
	}
}


static void MD5Update(
	MD5_CTX *context,
	unsigned char *input,
	unsigned int inputlen
){
	unsigned int i = 0,index = 0,partlen = 0;
	index = (context->count[0] >> 3) & 0x3F;
	partlen = 64 - index;
	context->count[0] += inputlen << 3;
	if(context->count[0] < (inputlen << 3))
		context->count[1]++;
	context->count[1] += inputlen >> 29;

	if(inputlen >= partlen)
	{
		memcpy(&context->buffer[index],input,partlen);
		MD5Transform(context->state,context->buffer);
		for(i = partlen;i+64 <= inputlen;i+=64)
			MD5Transform(context->state,&input[i]);
		index = 0;        
	}  
	else
	{
		i = 0;
	}
	memcpy(&context->buffer[index],&input[i],inputlen-i);
}

static void MD5Final(
	MD5_CTX *context,
	unsigned char digest[16]
){
	unsigned int index = 0,padlen = 0;
	unsigned char bits[8];
	index = (context->count[0] >> 3) & 0x3F;
	padlen = (index < 56)?(56-index):(120-index);
	MD5Encode(bits,context->count,8);
	MD5Update(context,PADDING,padlen);
	MD5Update(context,bits,8);
	MD5Encode(digest,context->state,16);
}

static char * StringHex(
	const char * 	HexBuffer,
	int				HexBufferLens,
	char *			To,
	int				ToSize
){
	if(ToSize < 2 * HexBufferLens) return NULL;

	int i = 0;
	for(i=0;i<HexBufferLens;i++){
		sprintf(&To[2*i],"%02x",HexBuffer[i]&0xFF);
	}

	return To;
}


static const char * MD5HexStr(
	const char *	MD5EncodeStr,
	char *			MD5Str
){
	if(MD5EncodeStr == NULL) return NULL;
	if(MD5Str == NULL) return NULL;
	
	char MD5Sum[16] = {0};
	MD5_CTX MD5CTX;
	MD5Init(&MD5CTX);
	MD5Update(&MD5CTX,(unsigned char*)MD5EncodeStr,strlen(MD5EncodeStr));
	MD5Final(&MD5CTX,(unsigned char*)MD5Sum);
	StringHex(MD5Sum,16,MD5Str,33);
	return MD5Str;
}

//
// 获取报警时间
//
static const char * AlarmPushTime(char * TimeStr)
{
	char *wday[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
	struct tm *p;
	
	time_t timep;
	time(&timep);
	p = localtime(&timep); //取得当地时间
	
	sprintf(TimeStr,
		"%04d-%02d-%02d-%s-%02d-%02d-%02d",
		1900+p->tm_year,
		p->tm_mon + 1,
		p->tm_mday,
		wday[p->tm_wday],
		p->tm_hour,
		p->tm_min,
		p->tm_sec);

	return TimeStr;
}

//
//	创建 TCP 客户端
//
static int TCPSocketClient(
	const char * 	DeviceIP,
	int				DevicePort,
	int				Timeout
){
	int  TCPSocketHandle = -1;
	int  ErrCode = -1;

	struct sockaddr_in saddr;
	struct timeval rcto = {Timeout,0};

	TCPSocketHandle = socket(AF_INET,SOCK_STREAM,0);

	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(DevicePort);
	saddr.sin_addr.s_addr = inet_addr(DeviceIP);

	if(setsockopt(TCPSocketHandle,
		SOL_SOCKET,SO_RCVTIMEO,
		(char*)&rcto,sizeof(struct timeval)) < 0){	
		printf("SET SOCKET OPT ERROR: %d\n",errno);
		close(TCPSocketHandle);
		return -1;
	}
	
	ErrCode = connect(TCPSocketHandle,(struct sockaddr *)&saddr,sizeof(saddr));

	if(ErrCode < 0){
		printf("CONNECT TO %s WITH ERROR:%d.\n",DeviceIP,errno);
		close(TCPSocketHandle);
		return -1;
	}

	return TCPSocketHandle;
}

//
//	string safe copy between string
//
static char * StringSafeCopyEx(
	char *			To,
	const char *	From,
	int				ToBufferSize,
	const char *	Begin,
	const char *	End
){
	if(To == NULL || From == NULL) return NULL;
	const char * lpBegin = NULL;
	const char * lpEnd = NULL;

	if(Begin != NULL) lpBegin = strstr(From,Begin);
	
	if(lpBegin == NULL){
		return NULL;
//		lpBegin = From;
	}else{
		lpBegin = lpBegin + strlen(Begin);
	}

	int CopyLens = 0;
	int	CopySize = 0;
	
	lpEnd = strstr(lpBegin,End);
	if(lpEnd == NULL){
		CopySize = strlen(lpBegin);
		CopyLens = ToBufferSize > CopySize ? CopySize : ToBufferSize;
	}else{
		CopySize = lpEnd - lpBegin;
		CopyLens = ToBufferSize > CopySize ? CopySize : ToBufferSize;
	}

/*
	TRACE("COPY DATE FROM:[%c][%d].\n",
		lpBegin[0],CopyLens);
*/	
	memcpy(To,lpBegin,CopyLens);

	return To;
}


static int SimpleMsgPush(
	const char *	Akey,
	const char *	MasterSecret,
	int				RT,
	const char *	RV,
	const char *	Msg,
	const char *	MsgExts,
	int				MsgLens,
	int				MsgNumb,
	int				Timeout
){
	int SocketHandle = TCPSocketClient(
		SIMPLE_MSG_SERVER,
		SIMPLE_MSG_PORT,
		Timeout);

	if(SocketHandle < 0){
		printf("Invalid socket handle.\n");
		return -1;
	}

	char MD5EncodeStr[1024] = {0};
	char MD5Str[33] = {0};
	sprintf(MD5EncodeStr,"%d%d%s%s",
		MsgNumb,
		RT,
		RV,
		MasterSecret
		);

	MD5HexStr(MD5EncodeStr,MD5Str);

	char HTTPPost[8192] = {0};
	char HTTPPostParams[2048] = {0};
	sprintf(HTTPPostParams,
		"sendno=%d"
		"&app_key=%s"
		"&receiver_type=%d"
		"&receiver_value=%s"
		"&verification_code=%s"
		"&platform=\"android,ios\""
		"&msg_type=1"
		"&msg_content={\"n_content\":\"%s\",\"n_extras\":%s}",
		MsgNumb,
		Akey,
		RT,
		RV,
		MD5Str,
		Msg,
		MsgExts);

	sprintf(HTTPPost,
		"POST %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"User-Agent: receiver/Receiver/1.1\r\n"
		"Content-Type: application/x-www-form-urlencoded\r\n"
		"Content-Length: %d\r\n\r\n"
		"%s\r\n\r\n",
		SIMPLE_MSG_URL,SIMPLE_MSG_HOST,
		strlen(HTTPPostParams),
		HTTPPostParams);

//	TRACE("********************** HTTP POST HEAD *********************\n");
//	TRACE("%s",HTTPPost);
//	TRACE("********************** HTTP POST FOOT *********************\n");

	int nBytesDone = 0;
	nBytesDone = send(SocketHandle,HTTPPost,strlen(HTTPPost),0);
	if(nBytesDone < 0){
		printf("MSG PUSH FAILED:[%d].\n",errno);
		close(SocketHandle);
		return -1;
	}

	char HTTPErrStr[64] = {0};
	char MSGPErrStr[64] = {0};

	memset(HTTPPost,0,sizeof(HTTPPost));
	nBytesDone = recv(SocketHandle,HTTPPost,sizeof(HTTPPost),0);
	if(nBytesDone > 0){
		printf("********************** HTTP RECV HEAD *********************\n");
		printf("%s",HTTPPost);
		printf("********************** HTTP RECV FOOT *********************\n");

		StringSafeCopyEx(HTTPErrStr,HTTPPost,sizeof(HTTPErrStr),"HTTP/1.1 "," OK");
		StringSafeCopyEx(MSGPErrStr,HTTPPost,sizeof(MSGPErrStr),"\"errcode\":",",");

		int HTTPErr = atoi(HTTPErrStr);
		int MSGPErr = atoi(MSGPErrStr);
		close(SocketHandle);
		if(HTTPErr != 200) return -HTTPErr;
		if(MSGPErr != 0)   return -MSGPErr;

		return 0;
	}

	close(SocketHandle);
	return -1;
}


int PPDevSetupGPIO(
	int	ActiveLow,
	int	Direction,
	int Pin,
	int Value
){
	SDK_ERR Err;
	GADI_GPIO_OpenParam Set;
	
	Set.active_low = ActiveLow;
	Set.direction = Direction;
	Set.num_gpio = Pin;
	Set.value = Value;

	if(GPIOHands[Pin] != NULL){
		gadi_gpio_close(GPIOHands[Pin]);
	}

	SDK_HandleT hGPIO = gadi_gpio_open(&Err,&Set);
	
	if(hGPIO == NULL){
		printf("GPIO:[%d] INIT FAILED WITH ERROR:[%d].\n",Pin,Err);
		GPIOHands[Pin] = NULL;
		return -1;
	}
	
	GPIOHands[Pin] = hGPIO;
	
	return 0;
}

int PPDevGetGPIO(int Pin)
{
	SDK_ERR err;

	int val = 0;

	if(GPIOHands[Pin] == NULL) return -1;
	
	err = gadi_gpio_read_value(GPIOHands[Pin],&val);

	if(err != SDK_OK){
		printf("GADI GET GPIO:[%d] FAILED WITH ERROR:[%d].\n",Pin,err);
		return -1;
	}

	return val;
}

int PPDevSetGPIO(int Pin,int Value)
{
	SDK_ERR err;

	if(GPIOHands[Pin] == NULL) return -1;
	
	if(Value == 0){
		err = gadi_gpio_clear(GPIOHands[Pin]);
	}else{
		err = gadi_gpio_set(GPIOHands[Pin]);
	}

	if(err != SDK_OK){
		printf("GADI GET GPIO:[%d] FAILED WITH ERROR:[%d].\n",Pin,err);
		return -1;
	}

	return 0;
}

int door_open(void* arg)
{
	int flag_open = (int)arg;
	PPDevSetGPIO(DBC_LOCK,flag_open);
	if( flag_open == 1)
		netcam_audio_out(AUDIO_FILE_OPEN);
	return 0;
}



#define PUSHDEV_CONF_FILE   "pushdev.conf"
#define MOTIONALARMPARA_CONF_FILE "MotionAlarmPara.conf"
#define IDC_PUSH_MAX	32  // 最大推送客户端
#define  MAXWAITTIME  30
#define  MAXTALKTIME 300
#define  MAXLIVETIME   600

static SMsgAVIoctrlSetPushReq PushDevs[IDC_PUSH_MAX];   //记录极光消息推送的参数
static  SMsgAVIoctrlSetMDPReq  MotionAlarmPara;  //记录移动侦测参数



int check_alarm_enable()
{
	struct tm *p;
	int scheme_now,scheme_start,scheme_close;
	
	if( !MotionAlarmPara.MotionEnable )
		return 0;
	
	time_t timep;
	time(&timep); 
	p = localtime(&timep); // 取得当地时间
	scheme_now = p->tm_hour * 60 + p->tm_min;	
	scheme_start = MotionAlarmPara.MotionStartHour*60+  MotionAlarmPara.MotionCloseMins;
	scheme_close = MotionAlarmPara.MotionCloseHour*60+MotionAlarmPara.MotionCloseMins;
	if(scheme_start < scheme_close){
		if(scheme_now < scheme_start || scheme_now > scheme_close){
			printf("INVLAID ALARM TIME:[S:%d][C:%d][N:%d]\n",
				scheme_start,scheme_close,scheme_now
				);
			return 0;
		}
	}else{
		if(scheme_now < scheme_start && scheme_now > scheme_close){
			printf("INVLAID ALARM TIME:[S:%d][C:%d][N:%d]\n",
				scheme_start,scheme_close,scheme_now
				);
			return 0;
		}
	}

	//
	// 检测推送延时
	//
	unsigned long TimeNow = time(NULL);
	if(TimeNow - TimePush < IDC_PUSH_FERQ){
		printf("PUSH FREQUENCY:[%d] NOT ENOUGH:[%d].\n",
			IDC_PUSH_FERQ,
			TimeNow - TimePush
			);
		return 0;
	}
	
	TimePush = TimeNow;
	printf("check_alarm_enable success \n");
	return 1;
	
}



void AlarmPush( int type)
{
	int Retry = 3;
	int i;
	char sMsg[128] = {0};
	char sNow[128] = {0};
	char MsgExts[] = "{\"ios\":{\"badge\":1,\"sound\":\"notify.wav\"}}";

	AlarmPushTime(sNow);
	sprintf(sMsg,"j_Calling...,%s,%s,%d", sNow, runSystemCfg.deviceInfo.serialNumber, type);
	
	for(i=0;i<IDC_PUSH_MAX;i++)
	{
		Retry = 3;
		if(strlen(PushDevs[i].AppKey) == 0 ||
			strlen(PushDevs[i].Master) == 0 ||
			strlen(PushDevs[i].Alias) == 0)
			continue;
		while(Retry > 0)
		{
			if (SimpleMsgPush(PushDevs[i].AppKey,PushDevs[i].Master, PushDevs[i].Type, PushDevs[i].Alias,
				sMsg, MsgExts, strlen(sMsg), 1, 1)==0)
				break;

			printf("MSG PUSH TO KEY:[%s] FAILED.\n",PushDevs[i].AppKey);
			Retry--;
		}
	}
	return;
}



int SavePushDevs(void)
{
	return 0;//return CfgWriteToFile2(PUSHDEV_CONF_FILE,PushDevs,sizeof(PushDevs));
}


//
// 增加一个推送设备地址
//
int SetPushDevParam(SMsgAVIoctrlSetPushReq* pPushReq)  
{

	int i = 0;

	for(i=0;i<IDC_PUSH_MAX;i++){
		if(strcmp(PushDevs[i].Alias,pPushReq->Alias) == 0){
			printf("SetPushDevParam  Alias:[%s] EXIST.\n",pPushReq->Alias);
			return 0;
		}
	}
	
	for(i=0;i<IDC_PUSH_MAX;i++){
		if(PushDevs[i].Alias[0] == 0|| PushDevs[i].AppKey[0] == 0)
		{			
			printf("SetPushDevParam  Alias:[%s] success!\n",pPushReq->Alias);
			memcpy(&PushDevs[i],pPushReq,sizeof(SMsgAVIoctrlSetPushReq));
			break;
		}
	}

	SavePushDevs();
	return 0;
}


//删除一个推送设备地址
int DelPushDevParam(SMsgAVIoctrlSetPushReq* pPushReq)  
{

	int i = 0;

	for(i=0;i<IDC_PUSH_MAX;i++){
		if(strcmp(PushDevs[i].Alias,pPushReq->Alias) == 0){
			memset(&PushDevs[i],0,sizeof(SMsgAVIoctrlSetPushReq));			
			printf("DelPushDevParam Alias :[%s] success.\n",pPushReq->Alias);
		}
	}
	SavePushDevs();
	return 0;
}


int SetMoveAlarmParam(SMsgAVIoctrlSetMDPReq* p)
{
	memcpy(&MotionAlarmPara,p,sizeof(SMsgAVIoctrlSetMDPReq));
	return CfgWriteToFile(MOTIONALARMPARA_CONF_FILE,&MotionAlarmPara);
}


int GetMoveAlarmParam(SMsgAVIoctrlGetMDPResp* p)
{
	memcpy(p,&MotionAlarmPara,sizeof(SMsgAVIoctrlSetMDPReq));
	return 0;
}

int loadrunparam(void)
{
	char *data = NULL;
	memset(&MotionAlarmPara,0,sizeof(MotionAlarmPara));
	memset(&PushDevs,0,sizeof(PushDevs));
	data = CfgReadFromFile(MOTIONALARMPARA_CONF_FILE);
	if(data !=NULL)
	{
		memcpy(&MotionAlarmPara,data,sizeof(SMsgAVIoctrlSetMDPReq));
		free(data);
	}

	data = CfgReadFromFile(PUSHDEV_CONF_FILE);
	if(data !=NULL)
	{
		memcpy(PushDevs,data,sizeof(PushDevs));
		free(data);
	}
	return 0;
	
}



int GetSystemParam(SMsgAVIoctrlGetSysparamResp* p)
{
	sprintf(p->hardware_ver,"%s","1.2.3");
	sprintf(p->software_ver,"%s","1.2.3.4");
	sprintf(p->user_name,"%s","admin");
	sprintf(p->password,"%s","admin");
	p->WaitMax = MAXWAITTIME;
	p->TalkMax = MAXTALKTIME;
	p->LiveMax = MAXLIVETIME;
	return 0;
}


