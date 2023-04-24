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

#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include "httpClient.h"

#define HTTP_REQ_LENGTH         1024

#define HTTP_FORM_DATA_Boundary     "--------------------------741925991969099215290785"

// http 请求头信息
static char http_post_header[] =
{
"POST %s HTTP/1.1\r\n"
"Accept: */*\r\n"
"Host: %s\r\n"
"Content-Length: %d\r\n"
"Content-Type: application/json\r\n"
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

static char http_req_content[HTTP_REQ_LENGTH] = {0};



#ifdef SUPPORT_SSL

typedef struct
{
    int socket_fd;
    void* ssl;
}SSL_HANDLE;

int goke_ssl_send(void* ssl,char *data,int length,int tm_sec)
{
	int ret = gk_mbedtls_ssl_send(ssl, data, length);
    return ret;
}

int goke_ssl_recv(void* ssl,char *data,int length,int tm_sec)
{
	return gk_mbedtls_ssl_recv(ssl, data, length);
}
#endif

char* strstri(char* str, char* subStr)
{
    int len = strlen(subStr);
    if(len == 0)
    {
        return NULL;
    }

    while(*str)
    {
        if(strncasecmp(str, subStr, len) == 0)
        {
            return str;
        }
        ++str;
    }
    return NULL;
}


static int http_create_request_socket(const char* host, int isSsl)
{
    int sockfd;
#ifdef SUPPORT_SSL
    SSL_HANDLE *sslHandle = NULL;
#endif
    int port = 80;
    char portStr[8] = {0};
    char tmpHost[128] = {0};
    char *portStart = NULL;
    struct hostent *server;
    struct timeval timeout = {10,0};
    struct sockaddr_in serv_addr;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("[http_demo] http_create_request_socket create socket fail.\n");
        return -1;
    }

    /* lookup the ip address */
    if (strstr(host, ":") != NULL)
    {
        strncpy(tmpHost, host, strlen(host) - strlen(strstr(host, ":")));
        server = gethostbyname(tmpHost);
    }
    else
    {
        server = gethostbyname(host);
    }
    if(server == NULL)
    {
        printf("[http_demo] http_create_request_socket gethostbyname fail.\n");
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
    memcpy(&serv_addr.sin_addr.s_addr,server->h_addr,server->h_length);

    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
    {
        printf("[http_demo] http_create_request_socket connect fail.\n");
        close(sockfd);
        return -1;
    }

#ifdef SUPPORT_SSL
    if (isSsl)
    {
        void* pssl = gk_mbedtls_ssl_init(sockfd, host, portStr);

        if(pssl == NULL)
        {
            close(sockfd);
            sockfd = -1;
            printf("ssl connetc error");
            return -1;
        }

        sslHandle = (SSL_HANDLE*)malloc(sizeof(SSL_HANDLE));
        sslHandle->ssl = pssl;
        sslHandle->socket_fd = sockfd;
        sockfd = (int)sslHandle;
    }
#endif
    return sockfd;
}

static int http_get_content_length(int sock_fd, int isSsl)
{
    int ret;
    int flag =0;
    int recv_len = 0;
    char res_header[1024] = {0};
#ifdef SUPPORT_SSL
    SSL_HANDLE *sslHandle = (SSL_HANDLE *)sock_fd;
#endif
    while(recv_len<1023)
    {
#ifdef SUPPORT_SSL
        if (isSsl)
        {
            ret = goke_ssl_recv(sslHandle->ssl, res_header+recv_len, 1, 2);
        }
        else
#endif
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
    pos = strstri(res_header, "Content-Length:");//内容的长度(字节)
    if(pos)
    {
        sscanf(pos + strlen("Content-Length:"), "%d", &content_length);
    }
    else
    {
        if (strstr(res_header, "Transfer-Encoding: chunked") != NULL)
        {
#ifdef SUPPORT_SSL
            if (isSsl)
            {
                ret = goke_ssl_recv(sslHandle->ssl, res_header+recv_len, 4, 2);
            }
            else
#endif
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
static int http_parser_url(const char* url,char **host,char **path)
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

HTTP_MULTI_META_DATA * goke_http_form_data_create(void)
{
    HTTP_MULTI_META_DATA *dataStart = (HTTP_MULTI_META_DATA*)malloc(sizeof(HTTP_MULTI_META_DATA));
    memset(dataStart, 0, sizeof(HTTP_MULTI_META_DATA));
    return dataStart;
}

void goke_http_form_data_add(HTTP_MULTI_META_DATA *dataStart, char *name,char *value,int valueLen,
	HTTP_UPLOAD_MULTI_META_TYPE  dataType, char *fileName)
{
    HTTP_MULTI_META_DATA *dataNew = NULL;
    HTTP_MULTI_META_DATA *tmpData = NULL;
    if (name == NULL || value == NULL)
    {
        return;
    }

    if (dataStart->name == NULL)
    {
        dataNew = dataStart;
    }
    else
    {
        dataNew = (HTTP_MULTI_META_DATA*)malloc(sizeof(HTTP_MULTI_META_DATA));
    }
    dataNew->name = name;
    dataNew->value = value;
    dataNew->valueLen = valueLen;
    dataNew->dataType = dataType;
    dataNew->fileName = fileName;
    dataNew->next = NULL;

    if (dataNew != dataStart)
    {
        tmpData = dataStart;
        while(tmpData->next != NULL)
        {
            tmpData = tmpData->next;
        }
        tmpData->next = dataNew;
    }
}


int http_tcp_select_send(int sock, char *szbuf, int len, int timeout)
{
	int ret = 0;
	fd_set send_set;
	struct timeval tv;
	int send_total = 0;
	int n_timeout_num = 0;
	if (timeout == 0)
		timeout = 3;
	while(send_total < len) {
		if(3 == n_timeout_num) {//如果超时3次则关闭此次会话
			return -2;
		}

		FD_ZERO(&send_set);
		FD_SET(sock, &send_set);
		tv.tv_sec = timeout;
		tv.tv_usec = 0;
		ret = select(sock + 1, NULL, &send_set, NULL, &tv);
		if (ret < 0) {
			printf("sock:%d select send error:%s\n", sock, strerror(errno));
			return -1;
		}
		if(0 == ret) {
			printf("sock:%d select send timeout error:%s\n", sock, strerror(errno));
			n_timeout_num++;
			continue;
		}

		if(FD_ISSET(sock, &send_set)) {
			ret = send(sock, szbuf + send_total, len - send_total, 0);
			if(ret <= 0) {
				printf("SockSend Failed ret:%d, sock:%d, len:%d error:%s\n", ret, sock, len, strerror(errno));
				return -3;
			}

			n_timeout_num = 0;
			//break;
		}

		send_total += ret;
	}

	printf("Leave ==========> SockSend sock:%d, send_total:%d\n",sock,  send_total);
	return send_total;
}


char *goke_http_form_data_request(char* url, HTTP_MULTI_META_DATA *dataStart, int* dataLength, int *dataRecvLen)
{
    int ret = -1;
    int reqFd = -1;
    char boundaryData[64];
    char dataTmp[1024];
    int boundaryLen;
    int totalLen = 0;
    char HTTPPost[4096];
    char *host = NULL;
    char *path = NULL;
    int offset = 0;
    int isSsl = 0;
    int contentLen;
    int recv_size = 0;
    int totalSize = 0;
    char buff[256] = {0};
    int cpLen = 0;
    char *dataContent = NULL;
    HTTP_MULTI_META_DATA *tmpData = NULL;
#ifdef SUPPORT_SSL
    SSL_HANDLE *sslHandle = NULL;
#endif

    tmpData = dataStart;

    printf("goke_http_form_data_request:%s\n", url);
    if(http_parser_url(url,&host,&path))
    {
         printf("[http_demo] http_parser_url fail.\n");
         return NULL;
    }

    #ifdef SUPPORT_SSL
    if (strstr(url, "https") != NULL)
    {
        isSsl = 1;
    }
#endif
    reqFd = http_create_request_socket(host, isSsl);
    if(reqFd < 0)
    {
        printf("[http_demo] read http_create_request_socket fail fd = %d.\n", reqFd);
        goto download_file_end;
    }

    sprintf(boundaryData, "--%s\r\n", HTTP_FORM_DATA_Boundary);
    boundaryLen = strlen(boundaryData);
    //get content length
    do
    {
        totalLen += boundaryLen;

        if(tmpData->dataType == MULTIPART_DATA_TXT)
        {
            sprintf(dataTmp,"Content-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",tmpData->name,tmpData->value);
            totalLen += strlen(dataTmp);
        }
        else if(tmpData->dataType == MULTIPART_DATA_STREAM)
        {
            if(strlen(tmpData->fileName))
            {
                sprintf(dataTmp,"Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n",tmpData->name,tmpData->fileName);
            }
            else
            {
                sprintf(dataTmp,"Content-Disposition: form-data; name=\"%s\"; filename=\"\"\r\n",tmpData->name);
            }
            totalLen += strlen(dataTmp);

            if( strstr(tmpData->fileName,"jpeg")|| strstr(tmpData->fileName,"jpg") )
            {
                sprintf(dataTmp,"Content-Type: image/jpeg\r\n\r\n");
            }
            else if(strstr(tmpData->fileName,"json")  )
            {
                strcpy(dataTmp,"Content-Type: application/json\r\n\r\n");
            }
            else
            {
                strcpy(dataTmp,"Content-Type: application/octet-stream\r\n\r\n");
            }
            totalLen += strlen(dataTmp);

            totalLen += tmpData->valueLen;

            totalLen += strlen("\r\n");
        }
        tmpData = (HTTP_MULTI_META_DATA *)tmpData->next;
    }while(tmpData != NULL);
    totalLen += boundaryLen + strlen("--");

    sprintf(HTTPPost,
        "POST %s HTTP/1.1\r\n"
        "Host: %s\r\n"
        "User-Agent: receiver/Receiver/1.1\r\n"
        "Content-Type: multipart/form-data; boundary=%s\r\n"
        "Content-Length: %d\r\n\r\n",
        path,
        host,
        HTTP_FORM_DATA_Boundary,
        totalLen);

    offset = strlen(HTTPPost);
    tmpData = dataStart;
    do
    {
        strcpy(HTTPPost + offset, boundaryData);
        offset += boundaryLen;
        if(tmpData->dataType == MULTIPART_DATA_TXT)
        {
            sprintf(HTTPPost + offset,"Content-Disposition: form-data; name=\"%s\"\r\n\r\n%s\r\n",tmpData->name,tmpData->value);
            offset += strlen(HTTPPost + offset);
        }
        else if(tmpData->dataType == MULTIPART_DATA_STREAM)
        {
            if(strlen(tmpData->fileName))
            {
                sprintf(HTTPPost + offset,"Content-Disposition: form-data; name=\"%s\"; filename=\"%s\"\r\n",tmpData->name,tmpData->fileName);
            }
            else
            {
                sprintf(HTTPPost + offset,"Content-Disposition: form-data; name=\"%s\"; filename=\"\"\r\n",tmpData->name);
            }
            offset += strlen(HTTPPost + offset);

            if( strstr(tmpData->fileName,"jpeg")|| strstr(tmpData->fileName,"jpg") )
            {
                sprintf(HTTPPost + offset,"Content-Type: image/jpeg\r\n\r\n");
            }
            else if(strstr(tmpData->fileName,"json")  )
            {
                strcpy(HTTPPost + offset,"Content-Type: application/json\r\n\r\n");
            }
            else
            {
                strcpy(HTTPPost + offset,"Content-Type: application/octet-stream\r\n\r\n");
            }
            offset += strlen(HTTPPost + offset);

#ifdef SUPPORT_SSL
            if (isSsl)
            {
                sslHandle = (SSL_HANDLE *)reqFd;
                goke_ssl_send(sslHandle->ssl, HTTPPost, offset, 2);
            }
            else
#endif
            {
                send(reqFd, HTTPPost, offset, 0);
            }

            //send file
            #ifdef SUPPORT_SSL
            if (isSsl)
            {
                sslHandle = (SSL_HANDLE *)reqFd;
                goke_ssl_send(sslHandle->ssl, tmpData->value, tmpData->valueLen, 2);
            }
            else
#endif
            {
                send(reqFd, tmpData->value, tmpData->valueLen, 0);
            }

            offset = 0;
            strcpy(HTTPPost + offset,"\r\n");
            offset += strlen("\r\n");
        }
        tmpData = (HTTP_MULTI_META_DATA *)tmpData->next;
    }while(tmpData != NULL);
    
    sprintf(HTTPPost + offset, "--%s--\r\n", HTTP_FORM_DATA_Boundary);
    offset += boundaryLen + strlen("--");

    //send left
#ifdef SUPPORT_SSL
    if (isSsl)
    {
        sslHandle = (SSL_HANDLE *)reqFd;
        goke_ssl_send(sslHandle->ssl, HTTPPost, offset, 2);
    }
    else
#endif
    {
        send(reqFd, HTTPPost, offset, 0);
    }

    //get response
    contentLen = http_get_content_length(reqFd, isSsl);
    if(contentLen <1)
    {
        printf("[http_demo] read http_get_content_length fail contentLen = %d.\n",contentLen);
        goto download_file_end;
    }

    if (dataLength != NULL)
    {
        *dataLength = contentLen;
    }
    //printf("file len:%d\n", content_len);
    dataContent = (char*)malloc(contentLen + 1);
    if (dataContent == NULL)
    {
        printf("download file,malloc %d failed.\n", contentLen);
        goto download_file_end;
    }
	memset(dataContent,0,contentLen + 1);

    dataContent[contentLen] = 0;

    while(totalSize < contentLen)
    {

#ifdef SUPPORT_SSL
        if (isSsl)
        {
            recv_size = goke_ssl_recv(sslHandle->ssl, buff, 256, 2);
        }
        else
#endif
        {
            recv_size = recv(reqFd, buff, 256,0);
        }
        if(recv_size < 1)
        {
            printf("[http_demo] recv buff fail\n");
            goto download_file_end;
        }
        cpLen = recv_size;
        if (totalSize + recv_size > contentLen)
        {
            cpLen = contentLen - totalSize;
        }
        memcpy(dataContent + totalSize, buff, cpLen);
        totalSize += cpLen;

    }
    if (dataRecvLen != NULL)
    {
        *dataRecvLen = totalSize;
    }
    ret = 0;
download_file_end:
#ifdef SUPPORT_SSL
    if (isSsl)
    {
        if (sslHandle->socket_fd > -1)
        {
            close(sslHandle->socket_fd);
        }
        if (sslHandle->ssl != NULL)
        {
            gk_mbedtls_ssl_exit(sslHandle);
        }
        free(sslHandle);
    }
    else
#endif
    {
        if(reqFd>-1)
        {
            close(reqFd);
        }
    }

    tmpData = dataStart;
    while(tmpData != NULL)
    {
        dataStart = tmpData->next;
        free(tmpData);
        tmpData = dataStart;
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


/**
 * @brief goke_http_request 下载网络资源
 * @param url    网络资源的位置
 * @param file   保存到本地文件的路径
 * @return       成功返回0,否则失败
 */
char *goke_http_request(const char* url, int* dataLength, 
int *dataRecvLen, HTTP_METHOD type, char *sPostParam)
{
    int ret = -1;
    int req_fd = -1;
    int isSsl = 0;
    FILE *fp = NULL;
    char *host = NULL;
    char *path = NULL;
    int content_len;
    int recv_size = 0;
    int total_size = 0;
    char buff[256] = {0};
    int cpLen = 0;
    char *dataContent = NULL;
    char http_req_content[512] = {0};
#ifdef SUPPORT_SSL
    SSL_HANDLE *sslHandle = NULL;
#endif
    //printf("goke_http_request:%s\n", url);
    if(http_parser_url(url,&host,&path))
    {
         printf("[http_demo] http_parser_url fail.\n");
         return NULL;
    }

#ifdef SUPPORT_SSL
    if (strstr(url, "https") != NULL)
    {
        isSsl = 1;
    }
#endif
    req_fd = http_create_request_socket(host, isSsl);
    if(req_fd<0)
    {
        printf("[http_demo] read http_create_request_socket fail fd = %d.\n",req_fd);
        goto download_file_end;
    }
    if (type == HTTP_GET)
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
    //printf("[http_demo] http_req_content = \n%s\n",http_req_content);

#ifdef SUPPORT_SSL
    if (isSsl)
    {
        sslHandle = (SSL_HANDLE *)req_fd;
        goke_ssl_send(sslHandle->ssl, http_req_content, content_len, 2);
    }
    else
#endif
    {
        send(req_fd, http_req_content, content_len,0);
    }

    content_len = http_get_content_length(req_fd, isSsl);
    if(content_len <1)
    {
        printf("[http_demo] read http_get_content_length fail content_len = %d.\n",content_len);
        goto download_file_end;
    }

    if (dataLength != NULL)
    {
        *dataLength = content_len;
    }
    //printf("file len:%d\n", content_len);
    dataContent = (char*)malloc(content_len + 1);
    if (dataContent == NULL)
    {
        printf("download file,malloc %d failed.\n", content_len);
        goto download_file_end;
    }
	memset(dataContent,0,content_len + 1);

    dataContent[content_len] = 0;

    while(total_size < content_len)
    {

#ifdef SUPPORT_SSL
        if (isSsl)
        {
            recv_size = goke_ssl_recv(sslHandle->ssl, buff, 256, 2);
        }
        else
#endif
        {
            recv_size = recv(req_fd, buff, 256,0);
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
    if (dataRecvLen != NULL)
    {
        *dataRecvLen = total_size;
    }
    ret = 0;
download_file_end:
#ifdef SUPPORT_SSL
    if (isSsl)
    {
        if (sslHandle->socket_fd > -1)
        {
            close(sslHandle->socket_fd);
        }
        if (sslHandle->ssl != NULL)
        {
            gk_mbedtls_ssl_exit(sslHandle);
        }
        free(sslHandle);
    }
    else
#endif
    {
        if(req_fd>-1)
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



/**
 * @brief goke_http_request 下载网络资源
 * @param url    网络资源的位置
 * @param file   保存到本地文件的路径
 * @return       成功返回0,否则失败
 */
char *goke_http_binary_request(const char* url, int* dataLength, 
int *dataRecvLen, HTTP_METHOD type, char *sPostParam,char *dataPtr,int dataSize)
{
    int ret = -1;
    int req_fd = -1;
    int isSsl = 0;
    FILE *fp = NULL;
    char *host = NULL;
    char *path = NULL;
    int content_len;
    int recv_size = 0;
    int total_size = 0;
    char buff[256] = {0};
    int cpLen = 0;
    char *dataContent          = NULL;
    char http_req_content[512] = {0};
    
#ifdef SUPPORT_SSL
    SSL_HANDLE *sslHandle = NULL;
#endif
    printf("goke_http_request:%s\n", url);
    if(http_parser_url(url,&host,&path))
    {
         printf("[http_demo] http_parser_url fail.\n");
         return NULL;
    }

#ifdef SUPPORT_SSL
    if (strstr(url, "https") != NULL)
    {
        isSsl = 1;
    }
#endif

    req_fd = http_create_request_socket(host, isSsl);
    if(req_fd<0)
    {
        printf("[http_demo] read http_create_request_socket fail fd = %d.\n",req_fd);
        goto download_file_end;
    }
    
    if (type == HTTP_GET)
    {
        content_len = snprintf(http_req_content,512, http_get_header, path,  host, 0);
    }
    else
    {
        if (sPostParam != NULL)
        {
        	printf("[http_demo] -----111--- = %d\n",dataSize);
        	
            //content_len = snprintf(http_req_content,512, http_post_header, path,  host, strlen(sPostParam), sPostParam);
            content_len = snprintf(http_req_content,512, http_post_header, path,  host, dataSize, "");
        }
        else
        {
        	printf("[http_demo] -----2222222--- = %d\n",dataSize);
            content_len = snprintf(http_req_content,512, http_post_header, path,  host, dataSize, "");
        }
    }
    printf("[http_demo] http_req_content = \n%s\n",http_req_content);

#ifdef SUPPORT_SSL
    if (isSsl)
    {
        sslHandle = (SSL_HANDLE *)req_fd;
        goke_ssl_send(sslHandle->ssl, http_req_content, content_len, 2);
    }
    else
#endif
    {
        send(req_fd, http_req_content, content_len,0);

        //send(req_fd, dataPtr, dataSize,0);

		http_tcp_select_send(req_fd, dataPtr, dataSize,0);
    
    }

    content_len = http_get_content_length(req_fd, isSsl);
    if(content_len <1)
    {
        printf("[http_demo] read http_get_content_length fail content_len = %d.\n",content_len);
        goto download_file_end;
    }

    if (dataLength != NULL)
    {
        *dataLength = content_len;
    }
    //printf("file len:%d\n", content_len);
    dataContent = (char*)malloc(content_len + 1);
    if (dataContent == NULL)
    {
        printf("download file,malloc %d failed.\n", content_len);
        goto download_file_end;
    }
	memset(dataContent,0,content_len + 1);

    dataContent[content_len] = 0;

    while(total_size < content_len)
    {

#ifdef SUPPORT_SSL
        if (isSsl)
        {
            recv_size = goke_ssl_recv(sslHandle->ssl, buff, 256, 2);
        }
        else
#endif
        {
            recv_size = recv(req_fd, buff, 256,0);
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
    if (dataRecvLen != NULL)
    {
        *dataRecvLen = total_size;
    }
    ret = 0;
download_file_end:
#ifdef SUPPORT_SSL
    if (isSsl)
    {
        if (sslHandle->socket_fd > -1)
        {
            close(sslHandle->socket_fd);
        }
        if (sslHandle->ssl != NULL)
        {
            gk_mbedtls_ssl_exit(sslHandle);
        }
        free(sslHandle);
    }
    else
#endif
    {
        if(req_fd>-1)
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

