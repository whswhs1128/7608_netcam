#include <stdio.h>   
#include <stdlib.h>   
#include <string.h>   
#include <sys/types.h>   
#include <sys/socket.h>   
#include <errno.h>   
#include <unistd.h>   
//#include <netinet/in.h>   
#include <limits.h>   
#include <netdb.h>   
#include <arpa/inet.h>   
#include <ctype.h>

#include "tutk_push_func.h"
//#include "common.h"
#include "mbedtls_ssl_api.h"
#include "tutk_debug.h"
#include "utility_api.h"

#define  JIGUANG_HOST        	"api.jpush.cn"
#define  SIMPLE_MSG_URL_V3  	"https://api.jpush.cn/v3/push"//"https://183.232.57.12/v3/push"
#define  SIMPLE_MSG_SERVER_V3 	"113.31.136.60"//"183.232.57.12"//"113.31.136.60"

char *Rstrchr(char *s, char x)   
{   
    int i = strlen(s);   
    if (!(*s))   
        return 0;   
    while (s[i - 1])   
        if (strchr(s + (i - 1), x))   
            return (s + (i - 1));   
        else   
            i--;   
    return 0;   
}   
   
void GetHost(char *src, char *web, char *file, int *port)   
{   
    char *pA;   
    char *pB;   
    //memset(web, 0, sizeof(web));   
    //memset(file, 0, sizeof(file));   
    *port = 0;   
    if (!(*src))   
        return;   
    pA = src;   
    if (!strncmp(pA, "http://", strlen("http://")))   
        pA = src + strlen("http://");   
    else if (!strncmp(pA, "https://", strlen("https://")))   
        pA = src + strlen("https://");   
    pB = strchr(pA, '/');   
    if (pB) {   
        memcpy(web, pA, strlen(pA) - strlen(pB));   
        if (pB + 1) {   
            memcpy(file, pB + 1, strlen(pB) - 1);   
            file[strlen(pB) - 1] = 0;   
        }   
    } else   
        memcpy(web, pA, strlen(pA));   
    if (pB)   
        web[strlen(pA) - strlen(pB)] = 0;   
    else   
        web[strlen(pA)] = 0;   
    pA = strchr(web, ':');   
    if (pA)   
        *port = atoi(pA + 1);   
    else   
        *port = 443;   
}   
   
//
//  创建 TCP 客户端
//
int TCPSocketClientTimeoutNew(
   const char *    DeviceIP,
   int			   DevicePort,
   int			   Timeout
){
   int	TCPSocketHandle = -1;
   int	ErrCode = -1;
	int ms = Timeout*1000;
   struct sockaddr_in saddr;
   struct timeval rcto = {Timeout,0};
   int time_diff = 2000;
	do
	{
	   TCPSocketHandle = socket(AF_INET,SOCK_STREAM,0);

	   saddr.sin_family = AF_INET;
	   saddr.sin_port = htons(DevicePort);
	   saddr.sin_addr.s_addr = inet_addr(DeviceIP);

#if 0
	   if(setsockopt(TCPSocketHandle,
		   SOL_SOCKET,SO_RCVTIMEO,
		   (int*)&ms,sizeof(int)) < 0){
		   P2P_INFO("SET SOCKET OPT ERROR!\n");
		   close(TCPSocketHandle);
		   return -1;
	   }
#endif
	   
	    setsockopt(TCPSocketHandle, SOL_SOCKET, SO_RCVTIMEO,(void *)&rcto, sizeof(rcto));

   		ErrCode = tcp_connect_timedwait_ms(TCPSocketHandle,(struct sockaddr *)&saddr,sizeof(saddr),time_diff);
		ms -=time_diff;

		if(ErrCode < 0)
   		{
		   	close(TCPSocketHandle);		
		}
   }while(ms >0  && ErrCode < 0);

   if(ErrCode < 0)
   {
	   P2P_ERR("CONNECT TO %s ERROR:%d.\n",DeviceIP);
	   return -1;
   }
   
   return TCPSocketHandle;
}

int SimpleMsgPush_SSL_V3(
	const char *	Akey,
	const char *	MasterSecret,
	const char *	id,
	const char *	tipMsg,
	int				timestap,
	int				eventType,
	int				channel,
	int				Timeout
)
{   
    int sockfd, ret = -1,retVal = -1;   
  
    int portnumber;   
    char host_addr[24];   
    char host_file[128];   

    int length;   
  
	char base64UserPsd[128];
	char base64in[64];
	char * ServerIP = NULL;
	char dnsIp[32];
	struct sockaddr_in addr;
	char HTTPPost[1024] = {0};
	char HTTPPostParams[1024] = {0};
    void *sslHandle = NULL;


	memset(host_addr,0,sizeof(host_addr));
	memset(host_file,0,sizeof(host_file));
    GetHost(SIMPLE_MSG_URL_V3, host_addr, host_file, &portnumber);        /*·???í??·?￠???ú?￠???t??μè */   


    printf("SimpleMsgPush_SSL_V3********************Mbedtls webhost:%s post:%s,port:%d,timeout:%d\n", host_addr,host_file, portnumber,Timeout);   


	retVal = tutk_resolve_host(&addr.sin_addr,host_addr);
	if(retVal == 0)
	{
		memset(dnsIp,0,sizeof(dnsIp));		
        strcpy(dnsIp,inet_ntoa(addr.sin_addr));
		ServerIP = dnsIp;
		P2P_INFO("Serverip:%s\n",ServerIP);
	}
	else
	{
		ServerIP = SIMPLE_MSG_SERVER_V3;
	}
 
    P2P_INFO("\n\nSimpleMsgPush_SSL_V3****************TCPSocketClientTimeout ip:%s, port:%d\n", ServerIP, portnumber);
	sockfd = TCPSocketClientTimeoutNew(
		ServerIP,
		portnumber,
		Timeout);
	
	if(sockfd <= 0)
	{
		P2P_ERR("Connect server error sockfd:%d\n", sockfd);
		return -1;
	}

	sprintf(base64in,"%s:%s",Akey,MasterSecret);
	
	utility_base64_encode(base64in,base64UserPsd,strlen(base64in));

	P2P_INFO("base64UserPsd :%s\n",base64UserPsd);

    sslHandle = gk_mbedtls_ssl_init(sockfd, JIGUANG_HOST, "443");
    if(!sslHandle){
        P2P_ERR("gk_mbedtls_ssl_init error\n");
        return -1;
    }

  
	//printf("%s %d\n",__FUNCTION__,__LINE__);   
   
	memset(HTTPPostParams,0,sizeof(HTTPPostParams));
#if 0
	sprintf(HTTPPostParams, "{"
				"\"platform\": \"all\","
				"\"audience\": {"
					"\"tag\": "
					"[\"%s\"	]"
				"},"
				"\"options\": {"
					"\"time_to_live\": 15,"
					"\"apns_production\": true"
				"},"
				"\"notification\": {"
					"\"alert\": \"%s\","
					"\"android\": {"
						"\"extras\": {"
							"\"dev\": {"
								"\"uuid\": \"%s\","
								"\"type\": %d,"
								"\"time\": \"%d\","
								"\"channel\": %d"
							"}"
						"}"
					"},"
					"\"ios\": {"
						"\"sound\": \"notify.wav\","
						"\"badge\": 1,"
						"\"extras\": {"
							"\"dev\": {"
								"\"uuid\": \"%s\","
								"\"type\": %d,"
								"\"time\": \"%d\","
								"\"channel\": %d"
							"}"
						"}"
					"}"
				"},"
				"\"message\": {"
					"\"msg_content\": \"%s\","
						"\"extras\": {"
						"\"dev\": {"
							"\"uuid\": \"%s\","
							"\"type\": %d,"
							"\"time\": \"%d\","
							"\"channel\": %d"
						"}"
					"}"
				"}"
			"}",
			id,
			tipMsg,
			id,eventType,timestap,channel,
			id,eventType,timestap,channel,
			tipMsg,id,eventType,timestap,channel
			);

#else

sprintf(HTTPPostParams, "{"
			"\"platform\": \"all\","
			"\"audience\": {"
				"\"tag\": "
				"[\"%s\"	]"
			"},"
			"\"options\": {"
				"\"time_to_live\": 15,"
				"\"apns_production\": true"
			"},"
			"\"notification\": {"
				"\"alert\": \"%s\","
				"\"android\": {"
					"\"extras\": {"
						"\"dev\": {"
							"\"uuid\": \"%s\","
							"\"type\": %d,"
							"\"time\": \"%d\""
						"}"
					"}"
				"},"
				"\"ios\": {"
					"\"sound\": \"notify.wav\","
					"\"badge\": 1,"
					"\"extras\": {"
						"\"dev\": {"
							"\"uuid\": \"%s\","
							"\"type\": %d,"
							"\"time\": \"%d\""
						"}"
					"}"
				"}"
			"},"
			"\"message\": {"
				"\"msg_content\": \"%s\","
					"\"extras\": {"
					"\"dev\": {"
						"\"uuid\": \"%s\","
						"\"type\": %d,"
						"\"time\": \"%d\""
					"}"
				"}"
			"}"
		"}",
		id,
		tipMsg,
		id,eventType,timestap,
		id,eventType,timestap,
		tipMsg,id,eventType,timestap
		);

#endif
		//printf("HTTPPostParams:%s\n",HTTPPostParams);
	memset(HTTPPost,0,sizeof(HTTPPost));
	sprintf(HTTPPost,
		"POST /%s HTTP/1.1\r\n"
		"Host: %s\r\n"
		"User-Agent: receiver/Receiver/1.1\r\n"
		"Authorization: Basic %s\r\n" 
		"Accept: */*\r\n"		
		"Content-Type: application/json\r\n"
		"Content-Length: %d\r\n\r\n"
		"%s",
		host_file,
		JIGUANG_HOST,
		base64UserPsd,
		strlen(HTTPPostParams),
		HTTPPostParams);


	//printf("HTTPS POST:%s\n",HTTPPost);
    ret = gk_mbedtls_ssl_send(sslHandle, HTTPPost, strlen(HTTPPost));
  	if(ret < 0){
		P2P_ERR("MSG PUSH FAILED:[%d]., len:%d\n",errno,strlen(HTTPPost));
		goto end;
	}


	char HTTPErrStr[128] = {0};
	char MSGPErrStr[128] = {0};

	memset(HTTPPost,0,sizeof(HTTPPost));
	length = gk_mbedtls_ssl_recv(sslHandle,HTTPPost,sizeof(HTTPPost));
	if(length > 0){
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
			//return -HTTPErr; // if http server recv error,we don't length again
		}
		else
		{
			if(MSGPErr != 0)
			{
				P2P_ERR("ERROR:HTTPPost:%s\n",HTTPPost);

				//return -MSGPErr; // if http server recv error,we don't length again
			}
			else
			{
				P2P_INFO("HTTPS ji guang push ok\n");				
			}
		}
		ret = 0;
	}
end:
    P2P_INFO("xxxxxxxxx ssl test\n");
	if(sslHandle)
    	gk_mbedtls_ssl_exit(sslHandle);

	return ret;
}

