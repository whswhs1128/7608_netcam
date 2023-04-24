#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/socket.h>
//#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/prctl.h>

#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
//#include<termios.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <netdb.h>

#include "sdk_def.h"
#include "sdk_gpio.h"
#include "tutk_push_func.h"
#include "cfg_system.h"

#include "tutk_access_export.h"
#include "tutk_debug.h"

// JPUSH
#define SIMPLE_MSG_SERVER 	"183.232.25.234"
#define SIMPLE_MSG_URL 		"http://api.jpush.cn:8800/v2/push"
#define SIMPLE_MSG_HOST 	"api.jpush.cn"
#define SIMPLE_MSG_PORT		8800


//TUTK HTTP PUSH
#define GOOGLE_MSG_SERVER 	"172.217.27.138"
#define GOOGLE_MSG_URL		"http://fcm.googleapis.com/fcm/send"
#define GOOGLE_MSG_HOST		"fcm.googleapis.com"
#define GOOGLE_MSG_PORT		80
#define GOOGLE_API_KEY 		"AIzaSyBUiYielsc1MBpGU972psurhBTO3zhkvCA"

#define SET_THREAD_NAME() \
    prctl(PR_SET_NAME, (unsigned long)__func__, 0,0,0)


static unsigned char PADDING[]={
	0x80,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0   ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0   ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
	0   ,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0
};


//extern unsigned char PADDING;

static sdk_gpio_handle_t GPIOHands[32] = {0};

//extern int gk_com1;

static char MsgPushStrings[][BELL_MAX_NUMBER + 1][128] = {
	{	// chinese msg push string URL-encode
		//"%e6%97%a0%e6%95%88%e7%9a%84%e6%b6%88%e6%81%af.",
		"%e8%ae%be%e5%a4%87%e4%b8%8a%e7%ba%bf.",
		"%e6%82%a8%e6%9c%89%e8%ae%bf%e5%ae%a2.",
		"%e4%ba%ba%e4%bd%93%e6%84%9f%e5%ba%94%e6%8a%a5%e8%ad%a6%e8%a7%a6%e5%8f%91.",
		"%e7%a7%bb%e5%8a%a8%e4%be%a6%e6%b5%8b%e6%8a%a5%e8%ad%a6%e8%a7%a6%e5%8f%91.",
		"%e8%ae%be%e5%a4%87%e4%b8%8a%e7%ba%bf4.",
		"%e8%ae%be%e5%a4%87%e4%b8%8a%e7%ba%bf5.",
		"%e8%ae%be%e5%a4%87%e4%b8%8a%e7%ba%bf6.",
		"%e6%82%a8%e7%9a%84%e8%ae%be%e5%a4%87%e8%a2%ab%e6%8b%86%e9%99%a4.",
		"%E6%82%A8%E7%9A%84%E8%AE%BE%E5%A4%87%E7%94%B5%E9%87%8F%E4%BD%8E%EF%BC%8C%E8%AF%B7%E5%85%85%E7%94%B5.",
		"%e6%9c%80%e5%a4%a7%e6%b6%88%e6%81%af.",
	},	// english msg push string
	{
		//"you got invalid message.",
		"Device online.",
		"New vistor.",
		"Alarm by pir.",
		"Alarm by motion detective.",
		"Device online 4.",
		"Device online 5.",
		"Device online 6.",
		"Device has been remove.",
		"Device battery is low,please recharge",
		"You got max message.",
	},
};

//极光V3推送用的是UTF-8编码
static char MsgPushStringsGb2312[][BELL_MAX_NUMBER + 1][64] = {
	{	// chinese msg push string
		//"%e6%97%a0%e6%95%88%e7%9a%84%e6%b6%88%e6%81%af.",
		"设备上线.",  /* 设备上线*/
		//{0x8B,0xBE,0x59,0x07,0x4E,0x0A,0x7E,0xBF,0x0},
		"你有访客.",  /*您有访客*/
		"人体感应报警触发.",/*人体感应报警触发.*/
		"移动侦测报警触发.", /*移动侦测报警触发.*/
		"设备上线4.",
		"设备上线5.",
		"设备上线6.",
		"您的设备被拆除.", /*您的设备被拆除.*/
		"你的设备电量低，请充电.", /*你的设备电量低，请充电.*/
		"最大消息.", /*最大消息.*/
	},	// english msg push string

};

typedef struct
{
	unsigned int count[2];
	unsigned int state[4];
	unsigned char buffer[64];   
}DB_MD5_CTX;

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

static void DB_MD5Init(DB_MD5_CTX *context)
{
	memset(context,0,sizeof(DB_MD5_CTX));
	context->count[0] = 0;
	context->count[1] = 0;
	context->state[0] = 0x67452301;
	context->state[1] = 0xEFCDAB89;
	context->state[2] = 0x98BADCFE;
	context->state[3] = 0x10325476;
}

static void DB_MD5Decode(
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


static void DB_MD5Transform(
	unsigned int state[4],
	unsigned char block[64]
){
	unsigned int a = state[0];
	unsigned int b = state[1];
	unsigned int c = state[2];
	unsigned int d = state[3];
	unsigned int x[64];
	DB_MD5Decode(x,block,64);
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

static void DB_MD5Encode(
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


static void DB_MD5Update(
	DB_MD5_CTX *context,
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
		DB_MD5Transform(context->state,context->buffer);
		for(i = partlen;i+64 <= inputlen;i+=64)
			DB_MD5Transform(context->state,&input[i]);
		index = 0;        
	}  
	else
	{
		i = 0;
	}
	memcpy(&context->buffer[index],&input[i],inputlen-i);
}

static void DB_MD5Final(
	DB_MD5_CTX *context,
	unsigned char digest[16]
){
	unsigned int index = 0,padlen = 0;
	unsigned char bits[8];
	index = (context->count[0] >> 3) & 0x3F;
	padlen = (index < 56)?(56-index):(120-index);
	DB_MD5Encode(bits,context->count,8);
	DB_MD5Update(context,PADDING,padlen);
	DB_MD5Update(context,bits,8);
	DB_MD5Encode(digest,context->state,16);
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


static int MD5HexStr(
	const char *	MD5EncodeStr,
	char *			MD5Str
){
	if(MD5EncodeStr == NULL) return -1;
	if(MD5Str == NULL) return -1;

	unsigned char MD5Sum[16] = {0};
	DB_MD5_CTX MD5CTX;
	DB_MD5Init(&MD5CTX);
	DB_MD5Update(&MD5CTX,(unsigned char*)MD5EncodeStr,strlen(MD5EncodeStr));
	DB_MD5Final(&MD5CTX, MD5Sum);
	StringHex(MD5Sum,16,MD5Str,33);

	return 0;
}

//
// 获取报警时间
//
#if 0
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
#endif

#ifndef MODULE_SUPPORT_RTSP
static int fd_wait_writeable(int sockfd, int milliseconds)
{
		struct pollfd fds;
		return 1;
	wait_again:
		memset(&fds, 0, sizeof(struct pollfd));
	
		fds.fd	   = sockfd;
		fds.events = POLLOUT;
	
		int ret = poll(&fds, 1, milliseconds);
		if (ret < 0 && (errno == EINTR))
			goto wait_again;	/* interrupted by signal, try again */
	
		return ret;
}
#endif


int tcp_connect_timedwait(int sockfd, struct sockaddr *peeraddr, socklen_t len, int seconds)
{
	int flags = fcntl(sockfd, F_GETFL);
	if (flags < 0)
		return -1;

	/* nonblocking */
	if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
		return -1;

	if (connect(sockfd, peeraddr, len) < 0) {
		if (errno != EINPROGRESS)
			goto fail;

		if (fd_wait_writeable(sockfd, seconds * 1000) < 0)
			goto fail;

		/* check if connect successfully */
		int optval;
		socklen_t optlen = sizeof(optval);

		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
			perror("getsockopt");
			goto fail;
		}

		if (optval != 0) {
			/* connect failed */
			errno = optval;
			goto fail;
		}
	}

	/* restore socket status */
	if (fcntl(sockfd, F_SETFL, flags) < 0)
		return -1;

	return 0;

fail:
	/* restore socket status */
	if (fcntl(sockfd, F_SETFL, flags) < 0)
		return -1;

	return -1;
}

int tcp_connect_timedwait_ms(int sockfd, struct sockaddr *peeraddr, socklen_t len, int ms)
{
	int flags = fcntl(sockfd, F_GETFL);
	if (flags < 0)
		return -1;

	/* nonblocking */
	if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
		return -1;

	if (connect(sockfd, peeraddr, len) < 0) {
		if (errno != EINPROGRESS)
			goto fail;

		if (fd_wait_writeable(sockfd, ms) < 0)
			goto fail;

		/* check if connect successfully */
		int optval;
		socklen_t optlen = sizeof(optval);

		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
			perror("getsockopt");
			goto fail;
		}

		if (optval != 0) {
			/* connect failed */
			errno = optval;
			goto fail;
		}
	}

	/* restore socket status */
	if (fcntl(sockfd, F_SETFL, flags) < 0)
		return -1;

	return 0;

fail:
	/* restore socket status */
	if (fcntl(sockfd, F_SETFL, flags) < 0)
		return -1;

	return -1;
}



//
//	创建 TCP 客户端
//
#if 0
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
	if(TCPSocketHandle < 0)
	{
		P2P_ERR("\n%s,%d creat socket error!\n",__func__,__LINE__);
		return -1;
	}

	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(DevicePort);
	saddr.sin_addr.s_addr = inet_addr(DeviceIP);

	if(setsockopt(TCPSocketHandle,
		SOL_SOCKET,SO_RCVTIMEO,
		(char*)&rcto,sizeof(struct timeval)) < 0){	
		P2P_ERR("SET SOCKET OPT ERROR!\n");
		close(TCPSocketHandle);
		return -1;
	}
	
	ErrCode = connect(TCPSocketHandle,(struct sockaddr *)&saddr,sizeof(saddr));

	if(ErrCode < 0){
		P2P_ERR("CONNECT TO %s ERROR.\n",DeviceIP);
		close(TCPSocketHandle);
		return -1;
	}

	return TCPSocketHandle;
}
#endif
//
//	创建 TCP 客户端
//
int TCPSocketClientTimeout(
	const char * 	DeviceIP,
	int				DevicePort,
	int				Timeout
){
	int  TCPSocketHandle = -1;
	int  ErrCode = -1;

	struct sockaddr_in saddr;
	struct timeval rcto = {Timeout,0};

	TCPSocketHandle = socket(AF_INET,SOCK_STREAM,0);
	if(TCPSocketHandle < 0)
	{
		P2P_ERR("\n%s,%d creat socket error!\n",__func__,__LINE__);
		return -1;
	}
	
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(DevicePort);
	saddr.sin_addr.s_addr = inet_addr(DeviceIP);

	if(setsockopt(TCPSocketHandle,
		SOL_SOCKET,SO_RCVTIMEO,
		(char*)&rcto,sizeof(struct timeval)) < 0){
		P2P_ERR("SET SOCKET OPT ERROR!\n");
		close(TCPSocketHandle);
		return -1;
	}

	ErrCode = tcp_connect_timedwait(TCPSocketHandle,(struct sockaddr *)&saddr,sizeof(saddr),Timeout);

	if(ErrCode < 0){
		P2P_ERR("CONNECT TO %s ERROR!.\n",DeviceIP);
		close(TCPSocketHandle);
		return -1;
	}

	return TCPSocketHandle;
}

//
//	string safe copy between string
//
char * StringSafeCopyEx(
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
	printf("COPY DATE FROM:[%c][%d].\n",
		lpBegin[0],CopyLens);
*/	
	memcpy(To,lpBegin,CopyLens);

	return To;
}
//
//GOOGLE FCM PUSH
//

static int GoogleMsgPush(
	char 	  *		FCMKey,
	const char *	ReceiveTarget,
	const char *	Msg,
	const char *	MsgExts,
	int				MsgLens,
	int				MsgNumb,
	int				Timeout)
{
	char HTTPPost[4096] = {0};
	char HTTPPostParams[2048] = {0};

	char * ServerIP = NULL;
	char dnsIp[32];
	struct sockaddr_in addr;
	int ret = tutk_resolve_host(&addr.sin_addr, GOOGLE_MSG_HOST);
	if(ret == 0)
	{
		memset(dnsIp,0,sizeof(dnsIp));		
        strcpy(dnsIp,inet_ntoa(addr.sin_addr));
		ServerIP = dnsIp;
	}
	else
	{
		ServerIP = GOOGLE_MSG_SERVER;
	}

	//printf("TCP Socket ------------------------->port:[%d]\n",GOOGLE_MSG_PORT);
	//printf("TCP Socket ------------------------->url:[%s] \n",GOOGLE_MSG_URL);
	//printf("TCP Socket ------------------------->host:[%s] \n",GOOGLE_MSG_HOST);

	int SocketHandle = TCPSocketClientTimeout(
		ServerIP,
		GOOGLE_MSG_PORT,
		Timeout);

	if(SocketHandle < 0){
		P2P_ERR("Invalid socket handle.\n");
		return -1;
	}

	//cJSON Data

	sprintf(HTTPPostParams,
		"{"
		"\"to\":\"/topics/%s\","
		"\"priority\":\"high\","
		"\"data\":{ \"context\":\"%s\",\"data\":%s}"
		"}",
		ReceiveTarget,
		Msg,
		MsgExts
		);


	sprintf(HTTPPost,
		"POST %s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"Authorization: key=%s\r\n"
		"Content-Type: application/json\r\n"
		"Content-Length: %d\r\n\r\n"
		"%s\r\n\r\n",
		GOOGLE_MSG_URL,GOOGLE_MSG_HOST,FCMKey,strlen(HTTPPostParams),HTTPPostParams);
	#if 0
	printf("********************** HTTP POST HEAD *********************\n");
	printf("%s",HTTPPost);
	printf("********************** HTTP POST FOOT *********************\n");
	#endif
	int nBytesDone = 0;
	nBytesDone = send(SocketHandle,HTTPPost,strlen(HTTPPost),0);
	//printf("Send ----------------ret:%d \n",nBytesDone);
	if(nBytesDone < 0){
		P2P_ERR("Google MSG PUSH FAILED:[%d].\n",errno);
		close(SocketHandle);
		return -1;
	}

	memset(HTTPPost,0,sizeof(HTTPPost));
	nBytesDone = recv(SocketHandle,HTTPPost,sizeof(HTTPPost),0);
	//printf("Recv ----------------ret:%d \n",nBytesDone);
	close(SocketHandle);


	if(nBytesDone > 0){
		char HTTPErrStr[64] = {0};
		char MSGPErrStr[64] = {0};
		#if 0
		printf("********************** HTTP RECV HEAD *********************\n");
		printf("%s",HTTPPost);
		printf("********************** HTTP RECV FOOT *********************\n");
		#endif
		StringSafeCopyEx(HTTPErrStr,HTTPPost,sizeof(HTTPErrStr),"HTTP/1.1 "," OK");
		StringSafeCopyEx(MSGPErrStr,HTTPPost,sizeof(MSGPErrStr),"\"errcode\":",",");

		//int HTTPErr = atoi(HTTPErrStr);
		int MSGPErr = atoi(MSGPErrStr);
		#if 0
		if(HTTPErr != 200)
		{
			printf("HTTPPost:%s\n",HTTPPost);
			return -HTTPErr;
		}
		#endif
		if(MSGPErr != 0)
		{
			P2P_ERR("HTTP ERROR HTTPPost:%s\n",HTTPPost);

			//return -MSGPErr; // if http server recv error,we don't send again
		}
		return 0;

	}


	return -1;
}

#if 0
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

	char * ServerIP = NULL;
	char dnsIp[32];
	struct sockaddr_in addr;
	int ret = tutk_resolve_host(&addr.sin_addr, SIMPLE_MSG_HOST);
	if(ret == 0)
	{
		memset(dnsIp,0,sizeof(dnsIp));		
        strcpy(dnsIp,inet_ntoa(addr.sin_addr));
		ServerIP = dnsIp;

		
		P2P_INFO("Serverip:%s\n",ServerIP);
	}
	else
	{
		ServerIP = SIMPLE_MSG_SERVER;
	}


	int SocketHandle = TCPSocketClientTimeout(
		ServerIP,
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
		"&msg_content={\"n_content\":\"%s\",\"n_extras\":%s}"
		"&time_to_live=16"
		"&apns_production=1",
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
	#if 0
	printf("********************** HTTP POST HEAD *********************\n");
	printf("%s",HTTPPost);
	printf("********************** HTTP POST FOOT *********************\n");
	#endif
	int nBytesDone = 0;
	nBytesDone = send(SocketHandle,HTTPPost,strlen(HTTPPost),0);
	if(nBytesDone < 0 || nBytesDone != strlen(HTTPPost)){
		P2P_ERR("MSG PUSH FAILED:[%d].,send len:%d, len:%d\n",errno,nBytesDone,strlen(HTTPPost));
		close(SocketHandle);
		return -1;
	}

	char HTTPErrStr[64] = {0};
	char MSGPErrStr[64] = {0};

	memset(HTTPPost,0,sizeof(HTTPPost));
	nBytesDone = recv(SocketHandle,HTTPPost,sizeof(HTTPPost),0);

	close(SocketHandle);
	
	if(nBytesDone > 0){
		#if 0
		printf("********************** HTTP RECV HEAD *********************\n");
		printf("%s",HTTPPost);
		printf("********************** HTTP RECV FOOT *********************\n");
		#endif

		StringSafeCopyEx(HTTPErrStr,HTTPPost,sizeof(HTTPErrStr),"HTTP/1.1 "," OK");
		StringSafeCopyEx(MSGPErrStr,HTTPPost,sizeof(MSGPErrStr),"\"errcode\":",",");

		int HTTPErr = atoi(HTTPErrStr);
		int MSGPErr = atoi(MSGPErrStr);

		if(HTTPErr != 200)
		{
			P2P_ERR("ERROR:HTTPPost:%s\n",HTTPPost);
			//return -HTTPErr; // if http server recv error,we don't send again
		}
		if(MSGPErr != 0)
		{
			P2P_ERR("ERROR:HTTPPost:%s\n",HTTPPost);

			//return -MSGPErr; // if http server recv error,we don't send again
		}
		return 0;
	}

	return -1;
}
#endif
void GooglePush(SMsgAVIoctrlSetPushReq * pi,int type,time_t ts){
	int retry = 3;
	int ret;
	char langtype = runSystemCfg.deviceInfo.languageType;
	char MsgExts[512]={0};
	char *sMsg;


	char lngStr = 0;

	if(type == BELL_DEVICE_ON_LINE)
		lngStr = 0;
	else if(type >= BELL_MAX_NUMBER)
		lngStr = 0;
	else
		lngStr = type;



	if(langtype < 2)
		sMsg = MsgPushStrings[langtype][lngStr];
	else
		sMsg = MsgPushStrings[1][lngStr];

	while(retry > 0){

		sprintf(MsgExts,"{"
			"\"ios\":{\"badge\":1,\"sound\":\"notify.wav\"},"
			"\"dev\":{\"uuid\":\"%s\",\"type\":%d,\"time\":\"%d\"}"
			"}",
			runSystemCfg.deviceInfo.serialNumber,type,ts);

	    //ret = GoogleMsgPush(pi->FcmKey,sMsg,2);
		ret = GoogleMsgPush(pi->FcmKey, pi->Alias, sMsg, MsgExts, strlen(sMsg), 1, 2);

		if(ret == 0)
		{
		  P2P_INFO("Send google msg ok Retry:%d",retry);
		  //doorbell_set_led_status(SYSTEM_PUSH_OK);

		  break;
		}
		else
		{
		  P2P_INFO("MSG PUSH TO KEY:[%s] FAILED.,google Retry:%d,ts:%d,ret:%d\n", pi->AppKey,retry,ret);
		}

	    retry--;

	}
	return;
}


void AlarmPush(SMsgAVIoctrlSetPushReq * pi, int type,int channel,time_t ts)
{
	int Retry = 3;

	char MsgExts[256] = {0};// = "{\"ios\":{\"badge\":1,\"sound\":\"notify.wav\"}}";
	int ret = 0;
	char langtype = 0;//runSystemCfg.deviceInfo.languageType;

	char *sMsg;
	char lngStr = 0;

#if !USE_JIGUANGPUSH_V2 
	char utf8Buff[128] = {0,};
#endif


	if(type == BELL_DEVICE_ON_LINE)
		lngStr = 0;
	else if(type >= BELL_MAX_NUMBER)
		lngStr = 0;
	else
		lngStr = type;

	if(langtype < 2)
	{
		if(langtype == 0)
		{
			utility_gbk_to_utf8(MsgPushStringsGb2312[langtype][lngStr],utf8Buff,sizeof(utf8Buff));
			sMsg = utf8Buff;
		}
		else
		{
			sMsg = MsgPushStrings[langtype][lngStr];
		}
	
	}
    else
		sMsg = MsgPushStrings[1][lngStr];

	if (pi == NULL ||
        strlen(pi->AppKey) == 0 ||
		strlen(pi->Master) == 0 ||
		strlen(pi->Alias) == 0)
		return;


	while(Retry > 0){

		P2P_INFO("Use jiguang HTTPS\n");
		ret = SimpleMsgPush_SSL_V3(pi->AppKey, pi->Master, runSystemCfg.deviceInfo.serialNumber,
			sMsg, ts, type + PUSH_MSG_TYPE_OFFSET, channel,4) ;

		if(ret == 0)
		{
			P2P_INFO("Send jpush msg ok Retry:%d\n",Retry);
            break;
        }
		else
		{
			P2P_INFO("MSG PUSH TO KEY:[%s] FAILED.,jpush Retry:%d,ret:%d\n", pi->AppKey,Retry,ret);
		}

		Retry--;
	}

	return;
}

int PPDevSetupGPIO(
	int	ActiveLow,
	int	Direction,
	int Pin,
	int Value
){
	SDK_ERR Err;
	//GADI_GPIO_OpenParam Set;
    sdk_gpio_handle_t Set;

	if(GPIOHands[Pin].value_fd != -1){
		sdk_gpio_close(&GPIOHands[Pin]);
	}

    Err = sdk_gpio_open(&GPIOHands[Pin], Pin);

	if(Err != 0){
		printf("GPIO:[%d] INIT FAILED WITH ERROR:[%d].\n",Pin,Err);
		//GPIOHands[Pin] = NULL;
		return -1;
	}

    sdk_gpio_set_direction(&GPIOHands[Pin], Direction);
    sdk_gpio_set_value(&GPIOHands[Pin], Value);
		
	return 0;
}

int PPDevGetGPIO(int Pin)
{
	SDK_ERR err;

	int val = 0;

	if(GPIOHands[Pin].value_fd == -1) return -1;
	
	err = sdk_gpio_get_value(&GPIOHands[Pin],&val);

	if(err != SDK_OK){
		printf("GADI GET GPIO:[%d] FAILED WITH ERROR:[%d].\n",Pin,err);
		return -1;
	}

	return val;
}

int PPDevSetGPIO(int Pin,int Value)
{
	SDK_ERR err;

	if(GPIOHands[Pin].value_fd == -1) return -1;
	
    err = sdk_gpio_set_value(&GPIOHands[Pin], Value);

	if(err != SDK_OK){
		printf("GADI GET GPIO:[%d] FAILED WITH ERROR:[%d].\n",Pin,err);
		return -1;
	}

	return 0;
}

int door_open(void* arg)
{
	//int pin = (int)arg;
//	PPDevSetGPIO(DBC_LOCK,pin); // for tw9912
//	HandlePut(gk_com1,"Lock2",strlen("Lock2"));
//	HandlePut(gk_com1,"Lock1",strlen("Lock1"));
	return 0;
}

//
// 串口操作
//

int SerialOpen(
	const char * 	ttyCOM
){
	int ttyFD = open(ttyCOM,O_RDWR);

	if(ttyFD < 0){
		printf("open %s error\n",ttyCOM);
		return -1;
	}

	return ttyFD;
}


int SerialClose(
	int 			ttyFD
){
	close(ttyFD);
	return 0;
}

int HandleGet(
	int 			FileHandle,
	char * 			Buffer, 
	int 			nBytesNeed
){
	char * lpBuffer = Buffer;
	int	nBytesRead = 0;

	while(nBytesNeed != 0){
		int nBytesDone = read(FileHandle,lpBuffer,nBytesNeed);

		if(nBytesDone < 0){
			return -1;
		}else if(nBytesDone == 0){
			return nBytesRead;
		}else{	
			nBytesNeed -= nBytesDone;
			nBytesRead += nBytesDone;
			lpBuffer += nBytesDone;
		}
	}

	return nBytesRead;    
}

int HandlePut(
	int 			FileHandle,
	char * 			Buffer, 
	int 			nBytesNeed
){
	while(nBytesNeed != 0){
		int Bytes = write(FileHandle,Buffer,nBytesNeed);

		if(Bytes < 0){
			return Bytes;
		}
		
		nBytesNeed -= Bytes;
	}

	return nBytesNeed;
}

void* RLAlarmPushThread(void *param)
{
	RL_Push_MsgT* push_msg = param;
	
	SET_THREAD_NAME();

	if(push_msg == NULL)
	{
		P2P_INFO("parame error\n");
		return 0;
	}

	P2P_INFO("RL Jpush Send msg number[%d],channel:[%d],appkey:%s\n",push_msg->number,push_msg->channel,push_msg->pi.AppKey);
	AlarmPush(&push_msg->pi,push_msg->type,push_msg->channel,push_msg->ts);

	free(push_msg);
	return 0;
}

void* RLGooglePushThread(void *param)
{
	RL_Push_MsgT* push_msg = param;

	SET_THREAD_NAME();

	if(push_msg == NULL)
	{
		P2P_INFO("parame error\n");
		return 0;
	}

	
	P2P_INFO("RL Google Send msg [%d],ts %ld",push_msg->number);
	GooglePush(&push_msg->pi,push_msg->type,push_msg->ts);

	free(push_msg);
	return 0;
}
/* resolve host with also IP address parsing */
int tutk_resolve_host(struct in_addr *sin_addr, const char *hostname)
{

	if (!inet_aton(hostname, sin_addr)) {
		struct addrinfo *ai, *cur;
		struct addrinfo hints = { 0 };
		hints.ai_family = AF_INET;
		if (getaddrinfo(hostname, NULL, &hints, &ai))
			return -1;
		/* getaddrinfo returns a linked list of addrinfo structs.
		 * Even if we set ai_family = AF_INET above, make sure
		 * that the returned one actually is of the correct type. */
		for (cur = ai; cur; cur = cur->ai_next) {
			if (cur->ai_family == AF_INET) {
				*sin_addr = ((struct sockaddr_in *)cur->ai_addr)->sin_addr;
				freeaddrinfo(ai);
				return 0;
			}
		}
		freeaddrinfo(ai);
		return -1;
	}
	return 0;
}
