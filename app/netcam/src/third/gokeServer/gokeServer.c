#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <netdb.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/vfs.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <pthread.h> 



#include "inc/gokeServer.h"
#include "netcam_api.h"
#include "sdk_def.h"
#include "sdk_audio.h"
#include "avi_rec.h"
#include "httpClient.h"
#include "utility/util_sock.h"
#include "flash_ctl.h"

#define MAX_MSG_LEN 512

#define HEART_TIME 120

#define HTTP_REQ_LENGTH         512

char gokeServerUrl[] = "server.lpwei.com";
//static char gokeServerUrl[128] = "192.168.10.189";
static char picServer[128] = "http://picweb.lpwei.com:9080/upload/uploadPic.php";
static int serverPort = 9090;


static unsigned long lastHeart = 0;



// http 请求头信息
static char http_header[] =
    "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Range: bytes=%d-\r\n"
    "Connection: Close\r\n"
    "Accept: */*\r\n"
    "\r\n";

static char http_req_content[HTTP_REQ_LENGTH] = {0};
static int fileTotalLen = 0;
static char *fileData = NULL;
static int dnsErrorCnt = 0;
static char *http_download_file(const char* url, int* fileLen, int *revLen, int isUpgrade);


static int create_request_socket(const char* host, int isSsl)
{
    int sockfd;
#ifdef SUPPORT_SSL
    SSL_HANDLE *sslHandle = NULL;
#endif
    int port = 80;
    char portStr[8] = {0};
    char tmpHost[128] = {0};
    char *portStart = NULL;
    //struct hostent *server;
    struct in_addr server;
    struct timeval timeout = {10,0};
    struct sockaddr_in serv_addr;
    int ret = 0;
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
    }
    if(ret == -1)
    {
        if (dnsErrorCnt > 5)
        {
            //set once
            if (dnsErrorCnt != 99)
            {
                printf("[http_demo] set dns to 114.114.114.114.\n");
                strcpy(tmpHost, "echo \"nameserver 114.114.114.114\" > /etc/resolv.conf");
                new_system_call(tmpHost);
                dnsErrorCnt = 99;
            }
        }
        else
        {
            dnsErrorCnt++;
        }
        printf("[http_demo] create_request_socket utility_net_resolve_host fail.\n");
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

    setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(struct timeval));
    if (connect(sockfd,(struct sockaddr *)&serv_addr,sizeof(serv_addr)) < 0)
    {
        printf("[http_demo] create_request_socket connect fail.\n");
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

static int get_http_content_length(int sock_fd, int isSsl, char **is302)
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
    char url302[512] = {0};
    int url302Length = 0;
    int content_length =0;
    char *pos = strstr(res_header, "HTTP/");
    if(pos)
    {
        sscanf(pos, "%*s %d", &status_code);//返回状态码
    }
    if(status_code!=200 && status_code!=206 && status_code != 302)
    {
        printf("[http_demo] get_content_length status_code = %d\n",status_code);
        return -1;
    }

    if (status_code == 302)
    {
        //printf("res_header:%s\n", res_header);
        pos = strstr(res_header, "Location:");//返回内容类型
        if(pos)
        {
            sscanf(pos, "%*s %s", url302);
        }
        printf("302 url:%s\n", url302);
        *is302 = http_download_file(url302, &url302Length, &content_length, 1);
        if (url302Length != content_length)
        {
            *is302 = NULL;
            content_length = -1;
        }
        return content_length;
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

/**
 * @brief http_download_file 下载网络资源
 * @param url    网络资源的位置
 * @param file   保存到本地文件的路径
 * @return       成功返回0,否则失败
 */
static char *http_download_file(const char* url, int* fileLen, int *revLen, int isUpgrade)
{
    int ret = -1;
    int req_fd = -1;
    int isSsl = 0;
    char *is302 = NULL;
    //FILE *fp = NULL;
    char *host = NULL;
    char *path = NULL;
    char *dataContent = NULL;
#ifdef SUPPORT_SSL
    SSL_HANDLE *sslHandle = NULL;
#endif
    //printf("http_download_file:%s\n", url);
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
    req_fd = create_request_socket(host, isSsl);
    if(req_fd<0)
    {
        printf("[http_demo] read create_request_socket fail fd = %d.\n",req_fd);
        goto download_file_end;
    }
    int content_len = snprintf(http_req_content,HTTP_REQ_LENGTH, http_header, path,  host, 0);
    //printf("[http_demo] http_req_content = \n %s",http_req_content);

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

    content_len = get_http_content_length(req_fd, isSsl, &is302);
    if(content_len < 1 || is302 != NULL)
    {
        printf("[http_demo] read get_http_content_length fail content_len = %d.\n",content_len);
        if (is302 != NULL)
        {
            *fileLen = content_len;
            *revLen = content_len;
        }
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
    memset(dataContent,0,content_len + 1);

    dataContent[content_len] = 0;

    int recv_size = 0;
    int total_size = 0;
    char buff[256] = {0};
    int cpLen = 0;
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
    if (revLen != NULL)
    {
        *revLen = total_size;
    }
    ret = 0;
download_file_end:
#ifdef SUPPORT_SSL
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
    if (ret != 0 && isUpgrade == 0)
    {
        if (dataContent != NULL)
        {
            free(dataContent);
            dataContent = NULL;
        }
    }
    if (is302 != NULL)
    {
        return is302;
    }
    return dataContent;
}


static int goke_upgrade_get_file_update(char *url)
{
    int ret = 0;
    int cnt;
    int tryCnt = 0;
    int reciveLen = 0;

    if(netcam_get_update_status() != 0)
    {
        printf("already start upgrade.exit\n");
        return 0;
    }

    runAudioCfg.rebootMute = 1;
    AudioCfgSave();
    netcam_update_relase_system_resource();
    while(tryCnt < 3)
    {
        tryCnt++;
        fileTotalLen = 0;
        //升级使用共享内存，不释放内存
        /*
        if (fileData != NULL)
        {
            free(fileData);
            fileData = NULL;
        }
        */
        printf("upgrade download files.try:%d\n", tryCnt);
        fileData = http_download_file(url, &fileTotalLen, &reciveLen, 1);
        if (fileData == NULL)
        {
            usleep(500 * 1000);
            continue;
        }

        if (fileTotalLen == reciveLen && fileTotalLen > 0)
        {
            ret = netcam_update(fileData, fileTotalLen, NULL);
            if (ret != 0)
            {
                usleep(500 * 1000);
                continue;
            }

            cnt = 0;
            while (cnt < 99)
            {
                cnt = netcam_update_get_process();
                usleep(218 * 1000);
                PRINT_INFO("upgrade  cnt:%d  ret:%d \n", cnt, ret);
            }
            break;
        }
        //共享内存不用释放内存
        /*
        else
        {
            free(fileData);
            fileData = NULL;
            printf("get file failed. get %d, file len:%d\n", fileTotalLen, reciveLen);
        }
        */
    }

    //usleep(500 * 1000);
   // new_system_call("reboot -f");//force REBOOT
    netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT
    return 0;
}




static int goke_upgrade_get_svn_num(char *allVer, char *svn)
{
    //int ret = -1;
    //ver:v1.0.3235.20190806182605
    char *svnStart = NULL;
    char *svnEnd = NULL;
    svnStart = strstr(allVer, ".");
    if (svnStart == NULL)
    {
        return -1;
    }
    svnStart = strstr(svnStart + 1 , ".");
    if (svnStart == NULL)
    {
        return -1;
    }
    svnStart += 1;
    svnEnd = strstr(svnStart, ".");
    if (svnEnd == NULL)
    {
        return -1;
    }

    strncpy(svn, svnStart, strlen(svnStart) - strlen(svnEnd));
    return 0;

}


int server_system_call(const char *cmdline)
{
    pid_t pid;
    int ret;

    if(cmdline == NULL) {
        return 1;
    }

    if((pid = vfork()) < 0) {
        printf("create new proccess failed\n");
        ret = -1;
    } else if (pid == 0) {
        //printf("  #%s\n", cmdline);
        execl("/bin/sh", "sh", "-c", cmdline, (char *)0);
        exit(127);
    } else {
        //printf("enter waitpid\n");
        while(waitpid(pid, &ret, 0) < 0){
            if (errno != EINTR) {
                ret = -1;
                break;
            }
        }
        //printf("exit waitpid[%d]\n", ret);
    }
    return ret;
}


int server_net_resolve_host(struct in_addr *sin_addr, const char *hostname)
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


int goke_server_connect(char *host, int port)
{
    int fdRet = -1;
    //struct timeval timeout = {1,0};
    struct sockaddr_in svraddr;
    struct in_addr server;
    int ret;
    //int on = 1;
    int fd = -1;
    char ipaddr[16] = {0};
    ret = server_net_resolve_host(&server,host);
    if (ret)
    {
        printf("get host:%s ip addr error\n", host);
    }
    else
    {
        //struct in_addr *pAddr = (struct in_addr*)host->h_addr;//host->h_addr_list[0];
        // 获取第一个IP地址
        //strncpy(ipaddr, inet_ntoa(*pAddr), 16);
        strncpy(ipaddr, inet_ntoa(server), 16);
        printf("get server host ip ok:%s\n", ipaddr);
    }


    if (strlen(ipaddr) > 0)
    {
        fd = socket(AF_INET,SOCK_STREAM,0);
        if(fd < 0)
        {
            printf("create error");
        }
        else
        {
            //连接服务器，设置服务器的地址(ip和端口)
            //strcpy(ipaddr, "192.168.10.103");
            memset(&svraddr,0,sizeof(svraddr));
            svraddr.sin_family=AF_INET;
            svraddr.sin_addr.s_addr= inet_addr(ipaddr);
            svraddr.sin_port=htons(port);
            printf("connect server:%s\n", ipaddr);
            fdRet = connect(fd,(struct sockaddr *)&svraddr,sizeof(svraddr));

            //setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));
            //setsockopt(*fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(struct timeval));
            if(fdRet < 0)
            {
                printf("connect error:%d\n", fdRet);
                close(fd);
                fd = -1;
            }
        }
    }

    return fd;
}

int goke_server_wait_event_type(int fd, int event, int timeout_sec)
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

static int server_set_finish(int fd, unsigned short type)
{
    int ret = 0;
    GOKE_SERVER_HEAD headRet;
    headRet.headMagic = 0x3323;
    headRet.cmdType = type;
    headRet.cmdLength = 0;
    ret = write(fd, &headRet, sizeof(GOKE_SERVER_HEAD));
    return ret;
}

static int server_set_heart(int fd, int uptime)
{
    int ret = 0;
    char buffer[128];
    int headLen = sizeof(GOKE_SERVER_HEAD);
    GOKE_SERVER_HEAD headRet;
    headRet.headMagic = 0x3323;
    headRet.cmdType = GOKE_HEART;
    headRet.cmdLength = 4;
    
    memcpy(buffer, &headRet, headLen);
    memcpy(buffer + headLen, &uptime, headRet.cmdLength);
    ret = write(fd, buffer, headLen + headRet.cmdLength);
    return ret;
}


unsigned long server_get_uptime(void)
{
    struct timespec times = {0, 0};
    unsigned long time;
    
    clock_gettime(CLOCK_MONOTONIC, &times);
    time = times.tv_sec;
        
    return time;
}

static int server_image_upload (char *data, int size, char *fileName)
{
    char url[256] = {0};
    char *retData = NULL;

    char devId[32] = {0};
    
    strcpy(devId, (char*)runSystemCfg.deviceInfo.serialNumber);

    HTTP_MULTI_META_DATA *fromData = goke_http_form_data_create();
    if (fromData == NULL)
    {
        printf("failture goke_http_form_data_create\n");
    }

    goke_http_form_data_add(fromData, "devid",
                            devId, strlen(devId),
                            MULTIPART_DATA_TXT, NULL);

    char short_time[128]       = {0};
    struct tm *ptm    = NULL;
    struct tm curTime = {0};
    struct timeval tv;
    gettimeofday( &tv, NULL );

    ptm = localtime_r(&tv.tv_sec, &curTime);

    sprintf(short_time, "%04d%02d%02d",
                                ptm->tm_year+1900,
                                ptm->tm_mon+1,
                                ptm->tm_mday);
    goke_http_form_data_add(fromData, "short_time",
                            short_time, strlen(short_time),
                            MULTIPART_DATA_TXT, NULL);

    //office
    //sprintf(url,"http://logs.nas.lpwei.com:9090/upload/uploadPic.php");
    //home
    strcpy(url, picServer);
    

    char *fileData       = data;
    int length           = size;
    char imgFileName[64] = {0};
    sprintf(imgFileName, "%04d%02d%02d_%02d%02d%02d%03d.jpg",
                                ptm->tm_year+1900,
                                ptm->tm_mon+1,
                                ptm->tm_mday,
                                ptm->tm_hour,
                                ptm->tm_min,
                                ptm->tm_sec,
                                (int)tv.tv_usec/1000);

    goke_http_form_data_add(fromData, "name",
                            fileData, length,
                            MULTIPART_DATA_STREAM, imgFileName);
    strcpy(fileName, imgFileName);
    int retLen  = 0;
    int recvLen = 0;
    retData = goke_http_form_data_request(url, fromData, &retLen, &recvLen);
    if (retData != NULL)
    {
        PRINT_INFO("http ret data:%s\n",retData);

        free(retData);
        retData = NULL;
    }

    return 0;
}//read_frame END


int server_receive_msg(int fd)
{
    int ret;
    int headLen = sizeof(GOKE_SERVER_HEAD);
    char buffer[512] = {0};
    GOKE_SERVER_HEAD headRet;

    GOKE_SERVER_HEAD *header = (GOKE_SERVER_HEAD *)buffer;
    ret = read(fd, buffer, headLen);

    if(ret == headLen)
    {
        printf("server_receive_msg:header:%x, type:%d, len:%d\n", header->headMagic, header->cmdType, header->cmdLength);

        if (header->headMagic != 0x3323)
        {
            printf("server_receive_msg headmagic error. 0x%x\n", header->headMagic);
            return -1;
        }

        if (header->cmdLength + headLen >= MAX_MSG_LEN)
        {
            printf("server_receive_msg data error. len:%d\n", header->cmdLength);
            return -1;
        }

        headRet.headMagic = 0x3323;
        ret = read(fd, buffer + headLen, header->cmdLength);
        if(ret == header->cmdLength)
        {
            switch(header->cmdType)
            {
                case GOKE_REBOOT:
                {
                    int isMute = 0;
                    
                    headRet.cmdType = header->cmdType;
                    headRet.cmdLength = 0;
                    memcpy(buffer, &headRet, headLen);
                    ret = write(fd, buffer, headLen + headRet.cmdLength);
                    
                    memcpy(&isMute, buffer + headLen, sizeof(int));
                    if (isMute && runAudioCfg.rebootMute == 0)
                    {
                        runAudioCfg.rebootMute = isMute;
                        AudioCfgSave();
                    }
                    printf("retboot, mute:%d\n", isMute);
                    sleep(1);
                    netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); 
                    break;
                }
                case GOKE_HEART:
                {
                    printf("heart....\n");
                    
                    lastHeart = server_get_uptime();
                    break;
                }
                case GOKE_GET_NETUART:
                {
                    int status = 0;
                    if (access("/opt/custom/cfg/netuart", F_OK) == 0)
                    {
                        status = 1;
                    }
                    headRet.cmdType = GOKE_GET_NETUART;
                    headRet.cmdLength = 4;
                    memcpy(buffer, &headRet, headLen);
                    memcpy(buffer + headLen, &status, headRet.cmdLength);
                    ret = write(fd, buffer, headLen + headRet.cmdLength);
                    printf("GOKE_GET_NETUART status:%d\n", status);
                    break;
                }
                case GOKE_SET_NETUART:
                {
                    int status = 0;
                    char sysCmd[128] = {0};

                    //ret = read(fd, &status, header->cmdLength);
                    memcpy(&status, buffer + headLen, sizeof(int));
                    printf("GOKE_SET_NETUART status:%d, ret:%d\n", status, ret);
                    if (status)
                    {
                        if (access("/opt/custom/cfg/netuart", F_OK) != 0)
                        {
                            #if 1
                            #ifdef MODULE_SUPPORT_GB28181
                            sprintf(sysCmd, "echo -n \"%s\" > /opt/custom/cfg/netuart", runGB28181Cfg.DeviceUID);
                            #else
                            sprintf(sysCmd, "echo -n \"%s\" > /opt/custom/cfg/netuart", runSystemCfg.deviceInfo.serialNumber);
                            #endif
                            #else
                            sprintf(sysCmd, "echo -n \"%s\" > /opt/custom/cfg/netuart", "123456");
                            #endif
                        }
                    }
                    else if (access("/opt/custom/cfg/netuart", F_OK) == 0)
                    {
                        strcpy(sysCmd, "rm -f /opt/custom/cfg/netuart");
                    }
                    ret = server_system_call(sysCmd);
                    
                    headRet.cmdType = header->cmdType;
                    headRet.cmdLength = 0;
                    memcpy(buffer, &headRet, headLen);
                    ret = write(fd, buffer, headLen + headRet.cmdLength);
                    printf("GOKE_SET_NETUART finished:%d\n", status);
                    break;
                }
                case GOKE_GET_MD_INFO:
                {
                    GOKE_SERVER_MD md;
                    
                    printf("GOKE_GET_MD_INFO\n");
                    headRet.cmdType = header->cmdType;
                    headRet.cmdLength = sizeof(md);
                    md.enable = runMdCfg.enable;
                    md.sensitive = runMdCfg.sensitive;
                    memcpy(buffer, &headRet, headLen);
                    memcpy(buffer + headLen, &md, headRet.cmdLength);
                    ret = write(fd, buffer, headLen + headRet.cmdLength);
                    break;
                }
                case GOKE_SET_MD_INFO:
                {
                    GOKE_SERVER_MD md;

                    //ret = read(fd, &status, header->cmdLength);
                    memcpy(&md, buffer + headLen, sizeof(GOKE_SERVER_MD));
                    printf("GOKE_SET_MD_INFO status:%d, sensitive:%d\n", md.enable, md.sensitive);
                    if (runMdCfg.enable != md.enable || runMdCfg.sensitive != md.sensitive)
                    {
                        runMdCfg.enable = md.enable;
                        runMdCfg.sensitive = md.sensitive;
                        MdCfgSave();
                    }
                    ret = server_set_finish(fd, header->cmdType);
                    break;
                }
                case GOKE_GET_AUDIO:
                {
                    GOKE_SERVER_AUDIO res;
                    
                    printf("GOKE_GET_AUDIO\n");
                    headRet.cmdType = header->cmdType;
                    headRet.cmdLength = sizeof(res);
                    res.inputVolume = runAudioCfg.inputVolume;
                    res.outputVolume = runAudioCfg.outputVolume;
                    res.audioInEnable = runAudioCfg.audioInEnable;
                    res.audioOutEnable = runAudioCfg.audioOutEnable;
                    res.rebootMute = runAudioCfg.rebootMute;
                    memcpy(buffer, &headRet, headLen);
                    memcpy(buffer + headLen, &res, headRet.cmdLength);
                    ret = write(fd, buffer, headLen + headRet.cmdLength);
                    break;
                }
                case GOKE_SET_AUDIO:
                {
                    GOKE_SERVER_AUDIO res;
                    
                    memcpy(&res, buffer + headLen, sizeof(GOKE_SERVER_AUDIO));
                    printf("GOKE_SET_AUDIO in:%d-%d, out:%d-%d, remute:%d\n", res.audioInEnable, 
                        res.inputVolume, res.audioOutEnable, res.outputVolume, res.rebootMute);
                    if (runAudioCfg.inputVolume != res.inputVolume || runAudioCfg.outputVolume != res.outputVolume
                        || runAudioCfg.audioInEnable != res.audioInEnable || runAudioCfg.audioOutEnable != res.audioOutEnable
                        || runAudioCfg.rebootMute != res.rebootMute)
                    {
                        if (runAudioCfg.inputVolume != res.inputVolume)
                        {
                            EM_GK_AUDIO_GainLevelEnumT vi_glevel;
                            runAudioCfg.inputVolume = res.inputVolume;
                            vi_glevel = get_gain_level(GK_GLEVEL_MAX*runAudioCfg.inputVolume/100);
                            ret = gk_audio_ai_set_gain(&vi_glevel);
                            if(ret!= SDK_OK)
                                printf("%s,gadi_audio_ai_set_gain failed \n",__FUNCTION__);
                        }

                        if (runAudioCfg.outputVolume != res.outputVolume)
                        {
                            runAudioCfg.outputVolume = res.outputVolume;
                            
                            gk_audio_ao_set_volume(13*runAudioCfg.outputVolume/100);
                            if(ret!= SDK_OK)
                                printf("%s,gadi_audio_ao_set_volume failed \n",__FUNCTION__);
                        }

                        if (runAudioCfg.audioInEnable != res.audioInEnable)
                        {
                            runAudioCfg.audioInEnable = res.audioInEnable;
                            gk_audio_input_enable(runAudioCfg.audioInEnable);
                        }
                        
                        if (runAudioCfg.audioOutEnable != res.audioOutEnable)
                        {
                            runAudioCfg.audioOutEnable = res.audioOutEnable;
                            audio_spk_en_control(runAudioCfg.audioOutEnable);
                        }
                        
                        runAudioCfg.rebootMute = res.rebootMute;
                        AudioCfgSave();
                    }
                    ret = server_set_finish(fd, header->cmdType);
                    break;
                }
                case GOKE_GET_RECORD:
                {
                    GOKE_SERVER_RECORD res;
                    
                    printf("GOKE_GET_RECORD\n");
                    headRet.cmdType = header->cmdType;
                    headRet.cmdLength = sizeof(res);
                    res.enable = runRecordCfg.enable;
                    res.audioRecEnable = runRecordCfg.audioRecEnable;
                    res.preRecordTime = runRecordCfg.preRecordTime;
                    res.recAudioType = runRecordCfg.recAudioType;
                    res.recordLen = runRecordCfg.recordLen;
                    res.recordMode = runRecordCfg.recordMode;
                    res.stream_no = runRecordCfg.stream_no;
                    memcpy(buffer, &headRet, headLen);
                    memcpy(buffer + headLen, &res, headRet.cmdLength);
                    ret = write(fd, buffer, headLen + headRet.cmdLength);
                    break;
                }
                case GOKE_SET_RECORD:
                {
                    GOKE_SERVER_RECORD res;
                    
                    memcpy(&res, buffer + headLen, sizeof(GOKE_SERVER_RECORD));
                    printf("GOKE_SET_RECORD enable:%d,%d,%d,%d,%d,%d,%d\n", res.enable, 
                        res.stream_no, res.recordMode, res.preRecordTime, res.audioRecEnable,
                        res.recAudioType, res.recordLen);
                    if (runRecordCfg.enable != res.enable || runRecordCfg.stream_no != res.stream_no
                        || runRecordCfg.recordMode != res.recordMode || runRecordCfg.preRecordTime != res.preRecordTime
                        || runRecordCfg.audioRecEnable != res.audioRecEnable || runRecordCfg.recAudioType != res.recAudioType
                        || runRecordCfg.recordLen != res.recordLen )
                    {
                        runRecordCfg.enable = res.enable;
                        runRecordCfg.audioRecEnable = res.audioRecEnable;
                        runRecordCfg.preRecordTime = res.preRecordTime;
                        runRecordCfg.recAudioType = res.recAudioType;
                        runRecordCfg.recordLen = res.recordLen;
                        runRecordCfg.recordMode = res.recordMode;
                        runRecordCfg.stream_no = res.stream_no;
                        if (runRecordCfg.enable == 0)        
                            thread_record_close();
                        RecordCfgSave();
                    }
                    ret = server_set_finish(fd, header->cmdType);
                    break;
                }
                case GOKE_GET_SD:
                {
                    GOKE_SERVER_SD res;
                    
                    headRet.cmdType = header->cmdType;
                    headRet.cmdLength = sizeof(res);
                    res.status = mmc_get_sdcard_stauts();
                    res.free = grd_sd_get_free_size_last();
                    res.all = grd_sd_get_all_size_last();
                    printf("GOKE_GET_SD, %d, %d, %d\n", res.status, res.free, res.all);
                    memcpy(buffer, &headRet, headLen);
                    memcpy(buffer + headLen, &res, headRet.cmdLength);
                    ret = write(fd, buffer, headLen + headRet.cmdLength);
                    break;
                }
                case GOKE_SD_FORMAT:
                {
                    printf("GOKE_SD_FORMAT\n");
                    grd_sd_format();
                    ret = server_set_finish(fd, header->cmdType);
                    break;
                }
                case GOKE_UPGRADE:
                {
                    GOKE_SERVER_UPGRADE res;
                    int ret2;
                    char curSvn[32];
                    char serverSvn[32];
                    int curSvnNo = 0;
                    int serverSvnNo = 0;
                    
                    memcpy(&res, buffer + headLen, sizeof(GOKE_SERVER_UPGRADE));
                    printf("GOKE_UPGRADE force:%d, ver:%s, model:%s, url:%s\n",
                        res.forceUpgrade, res.ver, res.model, res.url);

                    ret = server_set_finish(fd, header->cmdType);
                    if (strcmp(res.ver, (char*)runSystemCfg.deviceInfo.upgradeVersion) != 0)
                    {
                        ret2 = goke_upgrade_get_svn_num(res.ver, serverSvn);
                        if (ret2 != 0)
                        {
                            break;
                        }
                        ret2 = goke_upgrade_get_svn_num((char*)runSystemCfg.deviceInfo.upgradeVersion, curSvn);
                        if (ret2 != 0)
                        {
                            break;
                        }
                        if (curSvn[0] == '0')
                        {
                            curSvnNo = strtol(curSvn + 1, NULL, 0);
                        }
                        else
                        {
                            curSvnNo = strtol(curSvn, NULL, 0);
                        }
                    
                        if (serverSvn[0] == '0')
                        {
                            serverSvnNo = strtol(serverSvn + 1, NULL, 0);
                        }
                        else
                        {
                            serverSvnNo = strtol(serverSvn, NULL, 0);
                        }
                        printf("cur ver:%s/server:%s, no:%d/%d\n", curSvn, serverSvn, curSvnNo, serverSvnNo);
                        if (serverSvnNo > curSvnNo || res.forceUpgrade == 1)
                        {
                            goke_upgrade_get_file_update(res.url);
                        }
                    }
                    else
                    {
                        printf("ver is the same:%s\n", res.ver);
                    }
                    break;
                }
                case GOKE_SNAP:
                {
                    FILE *f;
                    long len;
                    char *data;
                    int ret;
                    GOKE_SERVER_SNAP res;
                    
                    ret=netcam_video_snapshot(runVideoCfg.vencStream[2].h264Conf.width, runVideoCfg.vencStream[2].h264Conf.height, "/tmp/web_snapshot1.jpg", GK_ENC_SNAPSHOT_QUALITY_MEDIUM);
                    
                    if(ret != 0)
                    {
                        printf("snapshot timeout: %d\n", ret );
                        break;
                    }
                    
                    f=fopen("/tmp/web_snapshot1.jpg","rb");
                    if(f==NULL)
                    {
                        printf("open snap file error.\n");
                        delete_path("/tmp/web_snapshot1.jpg");
                        break;
                    }
                    fseek(f,0,SEEK_END);
                    len=ftell(f);
                    fseek(f,0,SEEK_SET);
                    data=(char*)malloc(len);
                    fread(data,1,len,f);
                    fclose(f);
                    delete_path("/tmp/web_snapshot1.jpg");
                    server_image_upload(data, len, res.path);
                    free(data);

                    headRet.cmdType = header->cmdType;
                    headRet.cmdLength = sizeof(res);
                    printf("snap file %s\n", res.path);
                    memcpy(buffer, &headRet, headLen);
                    memcpy(buffer + headLen, &res, headRet.cmdLength);
                    ret = write(fd, buffer, headLen + headRet.cmdLength);
                    break;
                }
                case GOKE_SET_ID:
                {
                    GOKE_SERVER_SET_ID res;
                    struct device_info_mtd idInfo;
                    
                    memcpy(&res, buffer + headLen, sizeof(GOKE_SERVER_SET_ID));

                    memset(&idInfo,0,sizeof(struct device_info_mtd));

                    ret = load_info_to_mtd_reserve(res.type, &idInfo, sizeof(idInfo));
                	if((ret < 0) || (idInfo.device_id[0] == 0))
                	{
                		printf("Fail to load %d id from flash!\n", res.type);
                	}
                	else
                	{
                		printf("load goolink id:%s\n", idInfo.device_id);
                        strcpy(idInfo.device_id, res.id);
                        save_info_to_mtd_reserve(res.type, &idInfo, sizeof(idInfo));
                	}
                    

                    ret = server_set_finish(fd, header->cmdType);
                    printf("set %d to %s\n", res.type, res.id);
                    break;
                }
                case GOKE_RUN_CMD:
                {
                    GOKE_SERVER_RUN_CMD res;
                    
                    memcpy(&res, buffer + headLen, sizeof(GOKE_SERVER_RUN_CMD));

                    printf("run cmd: %s..\n", res.cmd);
                    if (strlen(res.cmd) > 0)
                    {
                        server_system_call(res.cmd);
                    }

                    ret = server_set_finish(fd, header->cmdType);
                    break;
                }
                
                case GOKE_VIDEO_FLIP:
                {
                    GOKE_SERVER_SET_FLIP res;
                    GK_NET_IMAGE_CFG stImagingConfig;
                    
                    memcpy(&res, buffer + headLen, sizeof(GOKE_SERVER_SET_FLIP));

                    memset(&stImagingConfig, 0, sizeof(GK_NET_IMAGE_CFG));
                    ret = netcam_image_get(&stImagingConfig);
                    printf("old flipEnabled:%d,mirrorEnabled:%d\n",stImagingConfig.flipEnabled,stImagingConfig.mirrorEnabled);
                    if (ret == 0)
                    {
                        stImagingConfig.flipEnabled = !stImagingConfig.flipEnabled;
                    
                        stImagingConfig.mirrorEnabled = !stImagingConfig.mirrorEnabled;
                    
                        ret = netcam_image_set(stImagingConfig);
                        ImageCfgSave();
                    }
                    
                    ret = server_set_finish(fd, header->cmdType);
                    printf("set image flip:%d, mirror:%d\n", stImagingConfig.flipEnabled, stImagingConfig.mirrorEnabled);
                    break;
                }
                
                default:
                {
                    printf("unknow cmd:%d\n", header->cmdType);
                    ret = server_set_finish(fd, GOKE_UNKNOWN);
                    break;
                }
            }
        }
    }
    else
    {
        printf("server read data len:%d\n", ret);
        ret = -1;
    }
    return ret;
}

static void server_set_thread_name(char *name)
{
    prctl(PR_SET_NAME, (unsigned long)name, 0,0,0); \
    pid_t tid;\
    tid = syscall(SYS_gettid);\
    printf("set pthread name:%d, %s, %s pid:%d tid:%d\n", __LINE__, __func__, name, getpid(), tid);\
}

int server_rand_time(int min, int max)
{
    int value = 0;
    struct timespec times = {0, 0};
    unsigned long time;

    clock_gettime(CLOCK_MONOTONIC, &times);
    time = times.tv_sec;

    srand(time);

    //产生随机数 [10000, 20000]
    value = rand() % (max + 1 - min) + min;

    return value;
}


void * goke_server_event(void *arg)
{
    int ret;
    int fd = -1;
    char ipaddr[16]= {0};
    int retrySleep = 30;
    char buffer[512] = {0};
    int headLen = sizeof(GOKE_SERVER_HEAD);
    GOKE_SERVER_DEV_INFO devInfo;
    GOKE_SERVER_HEAD gokeHead;
    unsigned long lastTime = server_get_uptime();
    unsigned long curTime = 0;
    int errorSleep = 0;
    pthread_detach(pthread_self());
    server_set_thread_name("gokeserver");
    
    fd = goke_server_connect(gokeServerUrl, serverPort);
    gokeHead.headMagic = 0x3323;
    while(1)
    {
        if (fd == -1)
        {
            fd = goke_server_connect(gokeServerUrl, serverPort);
            if (fd == -1)
            {
                printf("goke_server_connect failed, wait:%d\n", retrySleep);
                sleep(retrySleep);
                if (retrySleep < 7200)
                {
                    retrySleep = server_rand_time(retrySleep + 10, retrySleep * 2);
                }
                continue;
            }
            else
            {
                retrySleep = 30;
            }
        }
        
        gokeHead.cmdType = GOKE_DEV_INFO;
        gokeHead.cmdLength = sizeof(GOKE_SERVER_DEV_INFO);
        strcpy(devInfo.devId, runSystemCfg.deviceInfo.serialNumber);
        strcpy(devInfo.devVer, runSystemCfg.deviceInfo.upgradeVersion);
        strcpy(devInfo.devModel, runSystemCfg.deviceInfo.deviceType);

        memcpy(buffer, &gokeHead, headLen);
        memcpy(buffer + headLen, &devInfo, gokeHead.cmdLength);
        printf("send dev info.\n");
        write(fd, buffer, headLen + gokeHead.cmdLength);
        lastHeart = server_get_uptime();
        //server_set_heart(fd, lastHeart);
        while(1)
        {
            curTime = server_get_uptime();
            if (curTime - lastTime >= HEART_TIME)
            {
                server_set_heart(fd, curTime);
                lastTime = curTime;
            }
            
            if (curTime - lastHeart > HEART_TIME * 2)
            {
                printf("server heart timeout...\n");
                close(fd);
                fd = -1;
                break;
            }
            ret = goke_server_wait_event_type(fd, 0, 5);
            //printf("end to wait...%d\n", ret);
            if (ret == 0) 
            {
                //timeout. send ping.
                //printf("wait event timeout..\n");
                continue;
            } 
            else if (ret < 0) 
            {
                errorSleep = server_rand_time(1, 120);
                printf("goke_server_wait_event_type faild:%d, wait:%d\n", ret, errorSleep);
                close(fd);
                fd = -1;
                sleep(errorSleep);
                break;
            } 
            else 
            {
                //handle event
                ret = server_receive_msg(fd);
                if (ret == -1)
                {
                    errorSleep = server_rand_time(1, 300);
                    printf("server_receive_msg ret:%d, close, %d\n", ret, errorSleep);
                    close(fd);
                    fd = -1;
                    sleep(errorSleep);
                    break;
                }
            }
        }
    }
}

int goke_server_start(void)
{
    pthread_t thread_id;

    if(0 != pthread_create(&thread_id, NULL, goke_server_event, NULL))
    {
        printf("thread error\n");
        return -1;
    }
    return 0;
}

/*
int main(int argc, char *argv[])
{
    printf("start..\n");
    if (argc > 1)
    {
        strcpy(gokeServerUrl, argv[1]);
    }
    goke_server_start();
    while(1)
        sleep(1);
}
*/

