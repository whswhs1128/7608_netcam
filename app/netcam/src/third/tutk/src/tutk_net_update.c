#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include "pthread.h"
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include <resolv.h>

#include "cfg_all.h"
#include "flash_ctl.h"


#include "cJSON.h"
#include "AVIOCTRLDEFs.h"
#include "AVAPIs.h"

#include "md5.h"

#include "tutk_net_update.h"
#include "tutk_debug.h"
#include "tutk_push_func.h"
#include "netcam_api.h"

#define UPDATE_START    1
#define UPDATE_SUCCESS  2
#define UPDATE_FAIL     3
#define UPDATE_CHECK_FAILL 4
#define UPDATE_DOWNING_FILE 5


typedef struct
{
	void *updatebin;
	int len;
}NET_UpdateInfoT;

typedef enum
{
    WF_READ,
    WF_WRITE
}WAIT_EVENT;

static unsigned long long udpate_ver = 0;
static TUTK_UPDATE_ParT *update_arg = NULL;
static int update_status = 0; /*1:update start 0:update end*/
static int update_process = 0;
static int download_process = 0;


static void net_update_produce_md5(unsigned char *inEncrypt,int len,unsigned char *md5_str)
{
	MD5_CTX md5;
	MD5Init(&md5);

	//unsigned char encrypt[] ="admin";//21232f297a57a5a743894a0e4a801fc3
	//unsigned char decrypt[16];
	MD5Update(&md5,inEncrypt,len);
	MD5Final(&md5,md5_str);

	//P2P_INFO("\n befor md5:%s\n after md5 32 bit:\n",inEncrypt);
//		P2P_INFO("\n after md5 32bit out Decypty :\n");
#if 0
	for(i=0;i<16;i++)
	{
		P2P_ERR("%02x",outDecrypt[i]);  //02x前需要加上 %
	}

	printf("\n");
#endif


}

#if 0
static void net_update_status_audio_output(NETCAM_AUDIO_HintEnumT hint)
{
	NETCAM_AUDIO_MSG msg;
	memset(&msg, 0, sizeof(msg));
	msg.type = HINT;
	msg.hint = hint;
	netcam_audio_speaker_play(msg);
}
#endif

/* resolve host with also IP address parsing */
unsigned int net_resolve_host(struct sockaddr_in *addr, char * port, const char *hostname)
{
#if 0
    struct addrinfo *res = NULL;
    struct addrinfo hints;
	int ret = 0;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;


    ret = getaddrinfo(hostname, port, &hints, &res);
    if(res == NULL || ret != 0)
    {
        P2P_ERR("DNS parse error\n");
        return -1;
    }

    memcpy(addr, res->ai_addr, sizeof(struct sockaddr_in));
	P2P_INFO("dns parse: %s -> %s \n",hostname,inet_ntoa(addr->sin_addr));

    freeaddrinfo(res);


#else
	struct hostent *host = gethostbyname(hostname);
	if(host != NULL)
	{
		memcpy(addr, host->h_addr_list[0], host->h_length);
        P2P_INFO("ip[0]:%s\n",inet_ntoa(addr->sin_addr));
	}
	else
	{
        P2P_ERR("DNS parse error\n");
        return -1;
	}

#endif

    return 0;
}


static int net_update_last_index_at(char *str, char c)
{
    int n;
    n = strlen(str);

    while(n--)
    {
        if(c == str[n-1])
            return n-1 ;
    }
    return -1;
}


static int net_update_get_ver(char *s_ver, unsigned long long *n_ver)
{
    char s_tmp[32] = {0};
    int i = net_update_last_index_at(s_ver, '.');
    strcpy(s_tmp, s_ver + i + 1);

    *n_ver = (unsigned long long)atoll(s_tmp);
	return 0;
}


int http_parse_url_info(char*url,char *host,int *port,char* body)
{
	int len = strlen(url);
	int i,j;
	char pport[32]={0};
	char *p= url;
	char *h= host;
	if(len == 0)
	{
		P2P_ERR("url len is 0\n");
		return -1;
	}
	p = strstr(url,"http://");
	if(p == NULL)
	{
		P2P_ERR("url not vaild:%s\n",url);
		return -1;
	}
	p = url+7;
	i = 0;
	j = 0;
	while(p[i] && p[i]!=':' &&p[i]!= '/')
	{
		h[j] = p[i];
		j++;
		i++;
	}
	h[j] = 0;

	j = 0;
	if(p[i] && p[i]==':')
	{
		i++;
		while(p[i]!='/')
			pport[j++]= p[i++];
		pport[j] = 0;
		*port = atoi(pport);
	}
	else
	{
		*port = 80;
	}

	strcpy(body,p+i);

	//P2P_INFO("url:%s\n",url);
	//P2P_INFO("head:%s,port:%d,body:%s\n",host,*port,body);
	return 0;

}
static int net_time_wait(int sock, WAIT_EVENT e, int sec, int usec, int times)
{
	int ret = 0;
	fd_set fset;
	struct timeval tv;
	while(times > 0)
	{
		FD_ZERO(&fset);
		FD_SET(sock,&fset);

		tv.tv_sec = sec;
		tv.tv_usec = usec;

		if(e ==WF_READ)
			ret = select(sock+1, &fset, NULL, NULL, &tv);
		else
			ret = select(sock+1, NULL, &fset, NULL, &tv);
		if(ret > 0)
			goto finish;
		else if(ret < 0){
			if(errno != EINTR && errno != EAGAIN)
				goto error;
			}
		times--;
	}
error:
	P2P_ERR("select %s [time_wait] in net_time_wait!fd[%d] times[%d] errno[%d]\n",errno == 0?"timeout":"error",sock,times,errno);
finish:
	return ret;
}

int net_recv_data(int sock,char* buffer,int len,int flags)
{
	int n , ret;
	char* pbuf =buffer;
	int offset = 0;
	while(len > 0)
	{
		n = recv(sock,pbuf,len,flags);
		if(n < 0)
		{
			if(errno == EINTR || errno == EAGAIN ||errno == EWOULDBLOCK)
			{
				ret = net_time_wait(sock, WF_READ, 0, 200000, 5);
				if(ret > 0)
					continue;
				else if (ret == 0)
					break;
				else
					goto error;
			}
			else
			{
				ret = 1;
				goto error;
			}
		}
		if (n == 0)
			break;
		len -= n;
		offset += n;
		pbuf += n;
	}
	return offset;
error:
	P2P_ERR("Failed to recv data[net_recv_data]! n[%d] fd[%d] ret[%d] errno[%d]\n",n, sock, ret, errno);
	return -1;
}

int net_send_data(int sock,char* buffer,int len,int flags)
{
	int n , ret;
	char* pbuf =buffer;
	int offset = 0;
	while(len > 0)
	{
		n = send(sock,pbuf,len,flags);
		if(n < 0)
		{
			if(errno == EINTR || errno == EAGAIN)
			{
				ret = net_time_wait(sock, WF_WRITE, 1, 0, 5);
				if(ret > 0)
					continue;
				else if (ret == 0)
					break;
				else
					goto error;
			}
			else
			{
				ret = 1;
				goto error;
			}
		}
		if (n == 0)
			break;
		len -= n;
		offset += n;
		pbuf += n;
	}
	return offset;
error:
	P2P_ERR("Failed to send data[net_send_data]! n[%d] fd[%d] ret[%d] errno[%d]\n",n, sock, ret, errno);
	return -1;
}

int http_recv_parse_response_header(int sock,int *isGzipMethodUsed)
{
	char buffer[2048];
	int len = 2048;
	int ret;
	char *head_end = NULL;
	char version[25],errocode[32];
	int resvalue = 0;
	int content_len = 0;
	int res_len ;
	while (1)
	{
		memset(buffer, 0, len);
		// peek the receive buffer and check the completed header firstly
		//ret = recv(sock, buffer, len, MSG_PEEK);
		ret = net_recv_data(sock, buffer, len,MSG_PEEK);
		if (ret < 0)
		{
			P2P_ERR("Fail to recv data[parse_header]! "
				"fd[%d] ret[%d] \n",
				sock, ret);
			return 0;   //return 0 for close socket
		}

		if (ret == 0)
		{
			P2P_ERR("recv data 0, client close[parse_header]! "
				"fd[%d] ret[%d]\n",
				sock, ret);
			return 0;
		}

		head_end = strstr(buffer, "\r\n\r\n");
		if (head_end)
			break;

		if (ret >= 2048)
		{
			P2P_ERR("http header is too large[parse_header]!\n");
			return 0;
		}
	}

	if(head_end == NULL)
		return 0;
	res_len = ((int)head_end - (int)buffer)+4;
	ret = recv(sock, buffer, res_len, 0);
	if(ret != res_len)
	{
		P2P_ERR("recv error:ret[%d]\n",ret);
		return 0;
	}

	buffer[res_len] = 0;

	P2P_INFO("http response head:\n%s\n",buffer);
	head_end = strstr(buffer,"HTTP/");
	if(head_end)
	{
		sscanf(head_end,"HTTP/%s %d %s\r\n",version,&resvalue,errocode);
		P2P_INFO("version:%s value:%d err:%s\n",version,resvalue,errocode);
		if(resvalue != 200 && resvalue != 206)
		{
			return 0;
		}
	}
	else
	{
		return 0;
	}


	if(isGzipMethodUsed)
	{
		head_end = strstr(buffer,"Content-Encoding:");
		if(head_end)
		{
			if(strstr(head_end,"gzip"))
				*isGzipMethodUsed = 1;
			else
				*isGzipMethodUsed = 0;
		}
	}

	head_end = strstr(buffer,"Content-Length:");
	if(head_end)
	{
		sscanf(head_end,"Content-Length:%d\r\n",&content_len);
		P2P_INFO("content_len:%d \n",content_len);
		return content_len;
	}
	return 0;

}


char* http_down_load_file(char *url,int *file_len,int* process,char is_binfile)
{

#define HTTP_DOWNLOAD_ADDR	"47.98.100.187"

    int socketfd = 0;
    struct sockaddr_in addr;
    char host[64],Header[500],body[128];
	char host_ext[64];
	int port = 80;
	int ret;
	char *recv_buf = NULL;
	int recv_len = 2048, recv_len1;;
	int cnt = 0, icnt, i;
	int recv_try = 0;

	struct timeval rcto = {5,0};
	memset(host,0,sizeof(host));
	memset(body,0,sizeof(body));

	*file_len = 0;

  	ret = http_parse_url_info(url,host,&port,body);
  	if(ret != 0)
  	{
  		P2P_ERR("parse url failed\n");
		return NULL;
	}

	memset(&addr,0,sizeof(addr));
	//ret = net_resolve_host(&addr,NULL,host);
	ret = tutk_resolve_host(&addr.sin_addr,host);
	if(ret != 0)
	{

		addr.sin_addr.s_addr = inet_addr(HTTP_DOWNLOAD_ADDR);
		P2P_ERR("Dns %s praser failed!,use %s!\n",host,HTTP_DOWNLOAD_ADDR);
	}

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

	  	if(setsockopt(socketfd, SOL_SOCKET,SO_RCVTIMEO,  (char*)&rcto,sizeof(struct timeval)) < 0)
		{
			P2P_ERR("SET SOCKET OPT ERROR: %d\n",errno);
			close(socketfd);
			continue;
		}

	    ret = tcp_connect_timedwait(socketfd, (struct sockaddr *)&addr,sizeof(addr),2);
		if(ret != 0 && cnt <= 0)
		{
			close(socketfd);
			P2P_ERR("connect server timeout error\n");
			continue;
		}
		else
			break;
	}
    if(0 == cnt)
        return NULL;

	sprintf(host_ext,"%s:%d",host,port);
    sprintf(Header, "GET %s HTTP/1.1\r\n"
            "Host: %s\r\n"
            "Connection: keep-alive\r\n"
            "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.52 Safari/536.5\r\n"
            "Accept: */*\r\n"
            "Accept-Encoding: gzip,deflate,sdch\r\n"
            "Accept-Language: zh-TW,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n"
            "Accept-Charset: Big5,utf-8;q=0.7,*;q=0.3\r\n"
            "Pragma: no-cache\r\n"
            "Rang: bytes=0-\r\n"
            "Cache-Control: no-cache\r\n"
            "\r\n", body, host_ext);

    ret = send(socketfd,Header,strlen(Header),0);
	if(ret != strlen(Header))
	{
		P2P_ERR("send len :%d, actul len:%d \n",ret,strlen(Header));
		ret = -1;
		goto end;
	}

	recv_len = http_recv_parse_response_header(socketfd,NULL);
	if(recv_len == 0)
	{
		P2P_ERR("Response body len error \n");
		goto end;
	}

	if(!is_binfile)
	{
		recv_buf = malloc(recv_len+4);
	}
    else
    {
		recv_buf = netcam_update_malloc(recv_len+4);
    }
	if(recv_buf == NULL)
	{
		P2P_ERR("malloc %d failed \n",recv_len);
		ret = -1;
		goto end;
	}
	//else
		//recv_buf = (char *)DDR_MEMORY_DSP_BASE;

	memset(recv_buf,0,recv_len+4);

	cnt = 0;
	recv_try = 0;
	while(cnt < recv_len)
	{
    	//ret = recv(socketfd,recv_buf+cnt,recv_len-cnt,0);
		ret = net_recv_data(socketfd,recv_buf+cnt,recv_len-cnt,0);
		if(ret <= 0)
		{
			if (errno != EINTR && errno != EAGAIN)
			{
				P2P_ERR("Recv error, %d\n",errno);
				break;
			}
			else
				recv_try++;

			if(recv_try >= 3)
				break;

		}
		else
		{
			cnt += ret;
			if(process)
				*process = (cnt*100) /recv_len;

			recv_try = 0;
		}
	}

    if(socketfd)
    {
        close(socketfd);
        socketfd = 0;
    }
	if(cnt != recv_len)
	{
		P2P_ERR("First recv body error,cnt:%d, recv_len:%d\n",cnt,recv_len);
        icnt = 2;
        while(icnt--)
	    {
	        P2P_ERR("continue update cnt:%d, recv_len:%d\n",icnt,recv_len);
            #if 1
            memset(host,0,sizeof(host));
            memset(body,0,sizeof(body));

        	ret = http_parse_url_info(url,host,&port,body);
        	if(ret != 0)
        	{
        		P2P_ERR("parse url failed\n");
        	    continue;
            }

            memset(&addr,0,sizeof(addr));
            ret = net_resolve_host(&addr,NULL,host);
            if(ret != 0)
            {
				addr.sin_addr.s_addr = inet_addr(HTTP_DOWNLOAD_ADDR);
				P2P_ERR("Dns %s praser failed!,use %s!\n",host,HTTP_DOWNLOAD_ADDR);
            }
            #endif

            i = 3;
            while(i--)
            {
                socketfd = socket(AF_INET, SOCK_STREAM, 0);
                if (socketfd < 0) {
            		P2P_ERR("ERROR(icnt:%d): create update socket fd error!\n",icnt);
            	    continue;
            	}
                addr.sin_family = AF_INET;
                addr.sin_port = htons(port);

                if(setsockopt(socketfd, SOL_SOCKET,SO_RCVTIMEO,  (char*)&rcto,sizeof(struct timeval)) < 0)
                {
                    P2P_ERR("SET SOCKET OPT ERROR: %d\n",errno);
                    close(socketfd);
                    continue;
                }

                ret = tcp_connect_timedwait(socketfd, (struct sockaddr *)&addr,sizeof(addr),2);
                if(ret != 0 && i <= 0)
                {
                    close(socketfd);
                    P2P_ERR("connect server timeout error\n");

                    continue;
                }
                else
                    break;
            }
            if(0 == i)
                return NULL;

			sprintf(host_ext,"%s:%d",host,port);
            sprintf(Header, "GET %s HTTP/1.1\r\n"
                       "Host: %s\r\n"
                       "Connection: keep-alive\r\n"
                       "User-Agent: Mozilla/5.0 (Windows NT 6.1; WOW64) AppleWebKit/536.5 (KHTML, like Gecko) Chrome/19.0.1084.52 Safari/536.5\r\n"
                       "Accept: */*\r\n"
                       "Accept-Encoding: gzip,deflate,sdch\r\n"
                       "Accept-Language: zh-TW,zh;q=0.8,en-US;q=0.6,en;q=0.4\r\n"
                       "Accept-Charset: Big5,utf-8;q=0.7,*;q=0.3\r\n"
                       "Pragma: no-cache\r\n"
                       "Range: bytes=%d-\r\n"
                       "Cache-Control: no-cache\r\n"
                       "\r\n", body, host_ext, cnt);

            ret = send(socketfd,Header,strlen(Header),0);
            if(ret != strlen(Header))
            {
                P2P_ERR("send(icnt:%d) len :%d, actul len:%d \n",icnt,ret,strlen(Header));
                ret = -1;
                continue;
            }

            recv_len1 = http_recv_parse_response_header(socketfd,NULL);
            if(recv_len1 == 0)
            {
                P2P_ERR("Response(icnt:%d) body len error \n", icnt);
                continue;
            }

            i = 0;
			recv_try = 0;
            while(i < recv_len1)
            {
                //ret = recv(socketfd,recv_buf+cnt,recv_len-cnt,0);
				ret = net_recv_data(socketfd,recv_buf+cnt,recv_len-cnt,0);
                if(ret <= 0)
                {
                    if (errno != EINTR && errno != EAGAIN)
                    {
                        P2P_ERR("Recv error, %d\n",errno);
                        break;
                    }
					else
						recv_try++;

					if(recv_try >= 3)
						break;
                }
                else
                {
                    i += ret;
                    cnt += ret;
                    if(process)
                        *process = (cnt*100) /recv_len;

					recv_try = 0;
                }
            }

            if(cnt == recv_len)
            {
                P2P_INFO("recv body Success,ret:%d, recv_len:%d\n",cnt,recv_len);
                *file_len = recv_len;
                break;
            }
            else
            {
        		P2P_ERR("recv body error,ret:%d, recv_len:%d\n",cnt,recv_len);
        		if(socketfd)
	            {
            		close(socketfd);
                    socketfd = 0;
            	}
            }
        }

        if(cnt != recv_len)
        {
    		P2P_ERR("recv body error,ret:%d, recv_len:%d\n",cnt,recv_len);

			if(!is_binfile)
    			free(recv_buf);

    		recv_buf = NULL;
        }
	}
	else
	{
		*file_len = recv_len;
	}

end:
    if(socketfd)
	{
		close(socketfd);
        socketfd = 0;
	}
	return recv_buf;

}


int net_update_parse_json(unsigned char *data,unsigned long long *update_ver,char *update_url,char *update_md5)
{
    cJSON *json = NULL;
	cJSON *item = NULL;
	char version[64] ={0};
	unsigned char *ptr = data;

//	P2P_INFO("%x %x %x \n",data[0],data[1],data[2]);
	if(data[0] == 0xef && data[1] == 0xbb && data[2] == 0xbf)
	{
		P2P_ERR("delete utf8 format\n");
		ptr = data+3;
	}
	json = cJSON_Parse((char*)ptr);

	if(json == NULL)
	{
		P2P_ERR("data is not a json format,\n%s\n",data);
		return -1;
	}

	item = cJSON_GetObjectItem(json,"ver");
	if(item == NULL)
	{
		P2P_ERR("ver not exist\n");
		return -1;
	}


	snprintf(version,64,"%s",item->valuestring);
	net_update_get_ver(version,update_ver);

	item = cJSON_GetObjectItem(json,"url");
	if(item == NULL)
	{
		P2P_ERR("url not exist\n");
		return -1;
	}
	strcpy(update_url,item->valuestring);

	item = cJSON_GetObjectItem(json,"md5");
	if(item )
	{
		strcpy(update_md5,item->valuestring);
	}
	else
	{
		update_md5[0] = 0;
	}

	cJSON_Delete(json);

	return 0;

}

int net_update_md5_check(char *buffer,int len, char *recv_md5)
{

	//check md5, if equal return 0 else return other

	char md5Str[64]={'\0'};
	unsigned char mtws_md5_arr[64]={0};
	//根据升级文件得到md5校验码
	net_update_produce_md5((unsigned char *)buffer,len,mtws_md5_arr);
#if 0
	P2P_INFO("\n check md5 :\n");
	for(ii = 0;ii < 16;ii++)
	{
		printf("%2x",mtws_md5_arr[ii]);
	}
	printf("\n recv md5 :%s\n",recv_md5);
#endif

	snprintf(md5Str,33,"%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
	mtws_md5_arr[0],mtws_md5_arr[1],mtws_md5_arr[2],mtws_md5_arr[3],
	mtws_md5_arr[4],mtws_md5_arr[5],mtws_md5_arr[6],mtws_md5_arr[7],
	mtws_md5_arr[8],mtws_md5_arr[9],mtws_md5_arr[10],mtws_md5_arr[11],
	mtws_md5_arr[12],mtws_md5_arr[13],mtws_md5_arr[14],mtws_md5_arr[15]
	);
	//P2P_INFO("final md5str:%s\n",md5Str);
	/*比较计算得到的md5值(md5Str)，和接收到的md5值(recv_md5)，不相同则说明文件下载的不正确*/
	if(strcmp(md5Str,recv_md5) != 0)
	{
		P2P_ERR("md5 check failed,download file have error\n");
		return -1;
	}

	return 0;
}

int update_cfg_bin_process(char *cfg_url,unsigned long long cur_ver, NET_UpdateInfoT* info)
{
	//char url_upgrade_cfg[]="http://goke.lpwei.com/update/update.php?odm=10&model=DoorSM8801_v122";
	//char url_upgrade_cfg[]="http://119.29.92.205/update/update.php?odm=10&model=DoorSM8801_v1";
	int file_len  = 0;
	unsigned long long update_ver ;

	char update_url[256] = {0,};
	char update_md5[64] = {0,};
	int ret;

	char *data = http_down_load_file(cfg_url,&file_len,NULL,0);

	if(data == NULL)
	{
		P2P_ERR("http get modeule cfg failed\n");
		ret = -6;
	}
	else
	{
		//printf("upgrade json file len:%d ,strlen:%d,content:\n%s\n",file_len,strlen(data),data);
		//FILE* fp = fopen("udpate.cjson","wb");
		//fwrite(data,1,file_len,fp);
		//fclose(fp);

		//data = strdup(test);

		ret = net_update_parse_json((unsigned char*)data,&update_ver,update_url,update_md5);

		free(data);

		if(ret != 0)
		{
			return -6;
		}



		if(cur_ver >= update_ver)
		{
			P2P_ERR("Version is latest,NO need update\n");
			return -6;
		}
		udpate_ver = update_ver;

		P2P_INFO("Version update,cur_ver:%lld, update_ver:%lld\n",cur_ver,update_ver);
		file_len = 0;

		netcam_update_relase_system_resource();

		//data = http_down_load_file(update_url,&file_len,&download_process,1);
		data = http_down_load_file(update_url,&file_len,&download_process,1);
		if(data)
		{
			P2P_INFO("Down file udpate.bin ok,md5:%s\n",update_md5);
			//check md5;
			if(strlen(update_md5))
			{
				if(net_update_md5_check((char*)data,file_len,update_md5) == 0)
				{
					P2P_INFO("Md5 chekc ok\n");
					info->len = file_len;
					info->updatebin = data;
					ret = 0;
				}
				else
				{
					P2P_ERR("Md5 check error\n");
					//free(data);
					ret = -2;
					//netcam_audio_hint(SYSTEM_AUDIO_HINT_UPDATE_CHECK_FAILED);
					usleep(1500000);
				}
			}
			else
			{
				P2P_ERR("No Check md5\n");
				info->len = file_len;
				info->updatebin = data;
				ret = 0;
			}
		}
		else
		{
			P2P_ERR("Download updte bin failed\n");
			ret = -3;
		}
	}

	return ret;
}

int update_bin_process(char *bin_url,char *bin_md5, NET_UpdateInfoT* info)
{
	//char url_upgrade_cfg[]="http://goke.lpwei.com/update/update.php?odm=10&model=DoorSM8801_v122";
	//char url_upgrade_cfg[]="http://119.29.92.205/update/update.php?odm=10&model=DoorSM8801_v1";
	int file_len  = 0;
	//unsigned long long update_ver ;

	int ret = 0;

	//char *data = http_down_load_file(bin_url,&file_len,&download_process,1);
	char *data = http_down_load_file(bin_url,&file_len,&download_process,1);

	if(data == NULL)
	{
		P2P_ERR("http get modeule cfg failed\n");
		ret = -1;
	}
	else
	{

		P2P_INFO("Down file udpate.bin ok,md5:%s\n",bin_md5);
		//check md5;
		if(strlen(bin_md5))
		{
			if(net_update_md5_check((char*)data,file_len,bin_md5) == 0)
			{
				P2P_INFO("Md5 chekc ok\n");
				info->len = file_len;
				info->updatebin = data;
				ret = 0;
			}
			else
			{
				P2P_ERR("Md5 check error\n");
				//free(data);
				ret = -2;

				//netcam_audio_hint(SYSTEM_AUDIO_HINT_UPDATE_CHECK_FAILED);
				usleep(1500000);
			}
		}
		else
		{
			P2P_ERR("No Check md5\n");
			info->len = file_len;
			info->updatebin = data;
			ret = 0;
		}

	}

	return ret;
}

static void update_process_status_cb(void *arg)
{
	int process = (int)arg;
	SMsgAVIoctrlUpdateProgressResp resp;
	memset(&resp,0,sizeof(SMsgAVIoctrlUpdateProgressResp));

	P2P_INFO("update status:%d",process);
	if(update_arg)
	{
		snprintf(resp.version,16,"%lld",tutk_update_version());

		if(process == UPDATE_FAIL)
		{
			resp.progress = 0;
			resp.status = -1; //升级失败

			//netcam_audio_hint(SYSTEM_AUDIO_HINT_UPDATE_FAILED);
		}
		else if(process == UPDATE_SUCCESS)
		{
			resp.progress = 100;
			resp.status = 0; // 升级成功且进度为100

			//netcam_audio_hint(SYSTEM_AUDIO_HINT_UPDATE_SUCCESS);
		}
		else
		{
			return;
		}

		P2P_INFO("Update end ,result:%d\n",arg);
		avSendIOCtrl(update_arg->avindex,IOTYPE_USER_IPCAM_UPDATE_PROG_RESP, (char *)&resp, sizeof(SMsgAVIoctrlUpdateProgressResp));

		if(resp.progress == 100)
		{
			// 提示设备升级成功
		}

	}

}

unsigned long long tutk_update_version(void)
{
	return udpate_ver;
}

int tutk_update_get_status(void)
{
	return update_status;
}

void tutk_update_set_status(char status)
{
	update_status = status;
}

void tutk_update( void *arg)
{
	update_arg = (TUTK_UPDATE_ParT*)arg;
	int avIndex = update_arg->avindex;
	SMsgAVIoctrlUpdateReq *p = &update_arg->req;
	SMsgAVIoctrlUpdateProgressResp resp;
	NET_UpdateInfoT info;

	char *purl = NULL;
	int ret=0;

	memset(&resp,0,sizeof(SMsgAVIoctrlUpdateProgressResp));
	memset(&info,0,sizeof(info));

#if 0
	//需要从flash中获取model和odm id
	struct device_info_mtd devinfo;
	memset(&devinfo,0,sizeof(devinfo));
	ret = load_info_to_mtd_reserve(MTD_TUTK_P2P, &devinfo, sizeof(devinfo));
	if((ret < 0) || (devinfo.device_id[0] == 0) || (devinfo.device_id[0] == '0'))
	{
		P2P_ERR("load flash failed\n");
		free(update_arg);
		update_arg = NULL;
		tutk_update_set_status(0);

		//netcam_audio_hint(SYSTEM_AUDIO_HINT_UPDATE_FAILED);

		return ;
	}
#endif

	//因为升级过程网络端口，升级后也无法发送消息给app，所以在收到命令后就
	//发送消息给app，告诉对方正在升级
	//cJSON *urlValueJson = NULL;
	//自动模式0，
	//非自动模式1，从指定地址获取
	P2P_INFO("Update mode:%d",p->updateType);
	if(avIndex >= 0)
	{
		resp.progress = 0;
		resp.status = 0;
		avSendIOCtrl(avIndex,IOTYPE_USER_IPCAM_UPDATE_PROG_RESP, (char *)&resp, sizeof(SMsgAVIoctrlUpdateProgressResp));
	}

	//netcam_audio_hint(SYSTEM_AUDIO_HINT_UPDATE_DONWING);

	if(p->updateType == 1)
	{
		purl = p->urlAddr;
		netcam_update_relase_system_resource();
		P2P_INFO("update bin url : %s \n",purl);
		ret = update_bin_process(purl,p->md5,&info);
	}
	else
	{
		//char *tem_purl = "http://ts.vs98.com:8080/api/v1/update?devid=2903987&cloudid=2903987&curver=1012&model=1&odm=1";
		//char *tem_purl = "http://goke.lpwei.com/update/update.php?devid=2903987&cloudid=2903987&curver=1012&model=1&odm=1";

		char cfg_purl[500];

		unsigned long long now_n_ver = 0;
		net_update_get_ver(runSystemCfg.deviceInfo.upgradeVersion, &now_n_ver);
		P2P_INFO("now -- s_ver:%s n_ver:%lld\n", runSystemCfg.deviceInfo.upgradeVersion, now_n_ver);

		//char url_upgrade_cfg[]="http://goke.lpwei.com/update/update.php?odm=10&model=DoorSM8801_v122";
		#if 1//def USE_APP_XUHUI
		snprintf(cfg_purl,500,"http://ts.vs98.com:8080/api/v1/update?devid=%s&cloudid=%s&curver=%lld&model=%s&odm=%d"
		,runSystemCfg.deviceInfo.serialNumber,
		runSystemCfg.deviceInfo.serialNumber,
		now_n_ver,
		runSystemCfg.deviceInfo.model,
		runSystemCfg.deviceInfo.odmId);
		#else
		snprintf(cfg_purl,500,"http://goke.lpwei.com/update/update.php?devid=%s&cloudid=%s&curver=%lld&model=%s&odm=%d"
		,runSystemCfg.deviceInfo.serialNumber,
		runSystemCfg.deviceInfo.serialNumber,
		now_n_ver,
		runSystemCfg.deviceInfo.deviceName,
		runSystemCfg.deviceInfo.odmId);
		#endif
		P2P_INFO("cfg_purl:%s\n",cfg_purl);

		ret = update_cfg_bin_process(cfg_purl,now_n_ver,&info);
	}

	if(ret < 0)
	{
		P2P_INFO("NO update\n");

		//netcam_audio_hint(SYSTEM_AUDIO_HINT_UPDATE_FAILED);

		//下载文件失败，设备重启
		//update_process_status_cb((void*)UPDATE_CHECK_FAILL);


		free(update_arg);
		update_arg = NULL;
		tutk_update_set_status(0);
		if(ret != -6) // -6 不升级,不重启
		{
			usleep(1500000);
			netcam_sys_operation(0,SYSTEM_OPERATION_REBOOT);
		}

		return ;
	}

	//netcam_audio_hint(SYSTEM_AUDIO_HINT_UPDATE_BEGIN);

	ret = netcam_update((char *)info.updatebin, info.len, (cbFunc)update_process_status_cb);
	if(ret != 0)
	{
		P2P_ERR("Update file check error, reboot\n");
		netcam_sys_operation(0,SYSTEM_OPERATION_REBOOT);

		//升级文件校验失败，设备重启
	}


}

int tutk_update_get_process(void)
{
	int base_process = 0;
	if(tutk_update_get_status())
	{
		base_process = netcam_update_get_process();
		if(base_process == 0)
		{
			update_process = 2 + 18*download_process/100;
		}
		else
		{
			if(base_process < update_process)
			{
				update_process = 20+base_process*30/100;
			}
			else
			{
				update_process = base_process;
			}
		}
		return update_process;
	}
	else
		return 0;

}


static void* tutk_update_test_thread(void *arg)
{
	int ret = 0;
	TUTK_UPDATE_ParT *par;
    ST_SDK_NETWORK_ATTR net_attr;
    memset(&net_attr, 0, sizeof(ST_SDK_NETWORK_ATTR));

	pthread_detach(pthread_self());

    while(1)
    {
        sleep(10);

        ret = netcam_net_get_detect(runNetworkCfg.lan.netName);
		if(ret == 0)
			break;

		ret = netcam_net_get_detect(runNetworkCfg.wifi.netName);
		if(ret == 0)
			break;
    }

	while(1)
	{
		if(tutk_update_get_status() || netcam_get_update_status() < 0)
		{
			sleep(10);
			continue;
		}

		tutk_update_set_status(1);
		par = malloc(sizeof(TUTK_UPDATE_ParT));
		if(par )
		{
			memset(par,0,sizeof(TUTK_UPDATE_ParT)); //使用自动升级模式
			par->avindex = -1;
			netcam_update_set_down_load_cb((cbFunc)tutk_update,par);
		}
		else
		{
			tutk_update_set_status(0);

			P2P_ERR("tutk_update_test_thread malloc failed!\n");
			break;
		}
	}
}

void tutk_update_test_start(void)
{
	pthread_t Thread_ID;

    strcpy(runSystemCfg.deviceInfo.upgradeVersion,"v1.0.0.20191010000000");
	runSystemCfg.deviceInfo.odmId = 0;

	if (pthread_create(&Thread_ID, NULL, tutk_update_test_thread,NULL) != 0)
	{
		P2P_ERR("Crteate tutk_update_test_thread failed\n");
	}
}

