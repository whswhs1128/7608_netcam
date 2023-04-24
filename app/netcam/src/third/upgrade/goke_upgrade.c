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

#include "cJSON.h"
#include "netcam_api.h"
#include "sdk_def.h"
#include "eventalarm.h"
#include "goke_upgrade.h"

#define SUPPORT_SSL
#ifdef SUPPORT_SSL
#include "mbedtls_ssl_api.h"
#endif
//extern GK_NET_GB28181_CFG runGB28181Cfg;
#ifdef SUPPORT_SSL
static char updateCheckUrl[] = "https://tx.lpwei.com/update/update.php?odm=10";
#else
static char updateCheckUrl[] = "http://tx.lpwei.com/update/update.php?odm=10";
#endif

static int fileTotalLen = 0;
static char *fileData = NULL;
static int enableSdLog = 0xff;
static int enableNetLog = 0xff;
static int freshNow = 0;
static int dnsErrorCnt = 0;
static char devId[32] = {0};
static char hostAddr[] = "tx.lpwei.com";

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "utility/util_sock.h"
#ifdef MODULE_SUPPORT_SERVER_KEY
#include "serverkey.h"
#include "flash_ctl.h"
#endif

#define HTTP_REQ_LENGTH         512
#define GOKE_UDP_INFO


// http 请求头信息
static char http_header[] =
    "GET %s HTTP/1.1\r\n"
    "Host: %s\r\n"
    "Range: bytes=%d-\r\n"
    "Connection: Close\r\n"
    "Accept: */*\r\n"
    "\r\n";

static char http_req_content[HTTP_REQ_LENGTH] = {0};

#ifdef GOKE_UDP_INFO
static char hostIp[20] = {0};
static int udpInfoPort = 50700;
#endif

#ifdef MODULE_SUPPORT_SERVER_KEY
static int keyStatus = 1;
static char keySaved[64] = {0};
static char keySigned[128] = {0};
int isKeyTest = 0;
int statusTmp = 0;
#endif

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
    char chunkedLen[128] = {0};
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
            recv_len = 0;
            while(recv_len<127)
            {
#ifdef SUPPORT_SSL
                if (isSsl)
                {
                    ret = goke_ssl_recv(sslHandle->ssl, chunkedLen+recv_len, 1, 2);
                }
                else
#endif
                {
                    ret = recv(sock_fd, chunkedLen+recv_len, 1,0);
                }
                if(ret<1)  // recv fail
                {
                    break;
                }
                if (chunkedLen[recv_len] == '\n')
                {
                    break;
                }
                recv_len += ret;
            }
            sscanf(chunkedLen, "%x\r\n", &content_length);

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
    //printf("file len:%d\n", content_len);
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


int goke_upgrade_get_file_update(char *url)
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

int goke_upgrade_get_file_update_gb28181(void *arg)
{
    int ret=0;
    int cnt;
    int tryCnt = 0;
    int reciveLen = 0;
    int dnldSuccess = 0;
    UPGRAE_PARAM_S *upgardeParam;
    
    if(arg == NULL)
    {
        PRINT_ERR("param err, arg is NULL\n");
        return 0;
    }
    
    if(netcam_get_update_status() != 0)
    {
        PRINT_ERR("already start upgrade.exit\n");
        return 0;
    }
    upgardeParam = (UPGRAE_PARAM_S *)arg;
    PRINT_INFO("upgardeParam->firmware:%s,upgardeParam->fileUrl:%s\n",upgardeParam->firmware,upgardeParam->fileUrl);
	netcam_update_relase_system_resource();
    while(tryCnt < 3)
    {
        tryCnt++;
        fileTotalLen = 0;
        fileData = http_download_file(upgardeParam->fileUrl, &fileTotalLen, &reciveLen, 1);
        if (fileData == NULL)
        {
            usleep(500 * 1000);
            continue;
        }
        if (fileTotalLen == reciveLen && fileTotalLen > 0)
        {
            dnldSuccess = 1;//下载成功
            break;
        }
        else    //下载失败，释放内存
        {
            #ifndef MODULE_SUPPORT_UPGRADE_OUT
            if (fileData != NULL)
            {
                free(fileData);
                fileData = NULL;
            }
            #endif
            PRINT_INFO("get file failed. get %d, file len:%d\n", fileTotalLen, reciveLen);
        }
    }
    if(dnldSuccess == 1)
    {
        ret = netcam_update(fileData, fileTotalLen, NULL);
        if (ret == 0)
        {
            cnt = 0;
            while (cnt < 99)
            {
                cnt = netcam_update_get_process();
                usleep(218 * 1000);
                PRINT_INFO("upgrade  cnt:%d  ret:%d \n", cnt, ret);
            }
            usleep(500 * 1000);
            netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT
            return 0;
        }
    }
GOKE_UPGRADE_EXIT:
    #ifndef MODULE_SUPPORT_UPGRADE_OUT
    if (fileData != NULL)
    {
        free(fileData);
        fileData = NULL;
    }
    #endif
    event_alarm_touch(0, GK_ALARM_TYPE_UPGRADE_ALARM, 1, (void *)upgardeParam->firmware);
    usleep(30*1000 * 1000);
    netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT
    return 0;
}

int goke_upgrade_get_svn_num(char *allVer, char *svn)
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


int goke_update_get_ver_info(void)
{
	int ret = -1;
    char *checkEnd = NULL;
    //char response[1024]={0,};
    char *response = NULL;
    char out[1024]={0,};
    char ver[64];
    char curSvn[32];
    char serverSvn[32];
    int curSvnNo = 0;
    int serverSvnNo = 0;
    char reqUrl[512] = {0};
    int ethType = 0;
    char sysCmd[64] = {0};
    int dataLen = 0;
    int isForce = 0;
    char sdStatus[32] = {0};
    struct timespec times = {0, 0};
    long time;
#ifdef MODULE_SUPPORT_SERVER_KEY
    char keyData[64] = {0};
    int checkRet = 0;
#endif

    clock_gettime(CLOCK_MONOTONIC, &times);
    time = times.tv_sec;

    ST_SDK_NETWORK_ATTR net_attr;
    memset(&net_attr, 0, sizeof(ST_SDK_NETWORK_ATTR));

    while(1)
    {
        sleep(3);
        ret = netcam_net_get_detect(runNetworkCfg.lan.netName);
        if (ret != 0) {
            if (netcam_net_get_detect(runNetworkCfg.wifi.netName) == 0)
            {
                PRINT_INFO("eth0 is not running.");
                strcpy(net_attr.name, runNetworkCfg.wifi.netName);

                if(netcam_net_get(&net_attr) != 0) {
                    PRINT_ERR("get network config error.\n");
                    continue;
                }
                ethType = 1;
                PRINT_INFO("bc -- wifi ip :%s & static ip :%s\n", net_attr.ip, runNetworkCfg.wifi.ip);
            }
        } else {
            PRINT_INFO("eth0 is ok.");
            strcpy(net_attr.name, runNetworkCfg.lan.netName);

            if(netcam_net_get(&net_attr) != 0) {
                PRINT_ERR("get network config error.\n");
                continue;
            }
            PRINT_INFO("bc -- lan ip :%s & static ip :%s\n", net_attr.ip, runNetworkCfg.lan.ip);
        }
        break;
    }

	//send https msg;
	//parse sever_address and port;

#ifdef MODULE_SUPPORT_GB28181
    memcpy(devId, runGB28181Cfg.DeviceUID, sizeof(devId));
#else
    memcpy(devId, runSystemCfg.deviceInfo.serialNumber, sizeof(devId));
#endif

    sprintf(sdStatus, "%d,%d,%d-%d,%d-%d,%d-%d-%d", grd_sd_check_device_node(), mmc_get_sdcard_stauts(), grd_sd_get_free_size_last(),
        grd_sd_get_all_size_last(), runRecordCfg.enable, runRecordCfg.recordMode,
        runMdCfg.enable, runMdCfg.sensitive, runMdCfg.handle.is_rec);

    if (access("/opt/resource/audio/zh/dev_starting.alaw", F_OK) != 0)
    {
        //声音文件访问失败，需要升级资源分区
        memset(sdStatus, 0, sizeof(sdStatus));
        strcpy(sdStatus, "upgrade");
    }

    sprintf(reqUrl, "%s&model=%s&devid=%s&curver=%s&curIp=%s&curWifi=%s&ethType=%d&uptime=%ld&sd=%s", updateCheckUrl,
            runSystemCfg.deviceInfo.deviceType,
            devId, runSystemCfg.deviceInfo.upgradeVersion, net_attr.ip,
            runNetworkCfg.wifilink[0].essid, ethType, time, sdStatus);
    #ifdef MODULE_SUPPORT_SERVER_KEY
    sprintf(keyData, "&key=%s", keySaved);
    strcat(reqUrl, keyData);
    #endif

    response = http_download_file(reqUrl, &dataLen, NULL, 0);
    if (response == NULL)
    {
        printf("goke_update_send_http_message error.\n");
#ifdef MODULE_SUPPORT_SERVER_KEY
        if (server_key_check(keySaved, keySigned, strlen(keySigned)) != 0)
        {
            keyStatus = 0;
        }
        else
        {
            keyStatus = 1;
        }
        printf("tmp:%d, %d\n", keyStatus, statusTmp);
#endif
        return -1;
    }
    //printf("http_download_file:%s\nret:%s\n", reqUrl, response);

    checkEnd = strstr(response, "}");
    if (checkEnd != NULL)
    {
        checkEnd[1] = 0;
    }
    //printf("response:%s, len:%d\n", response, strlen(response));
    if (response[0] == 0xef && response[1] == 0xbb && response[2] == 0xbf)
    {
        strcpy(out, response + 3);
    }
    else
    {
        strcpy(out, response);
    }
    cJSON *json = NULL;
    json = cJSON_Parse(out);
    sleep(1);
    if (!json){
        //从配置文件解析cjson失败，则使用默认参数
        printf("Error before: [%s]\n", cJSON_GetErrorPtr());
        goto err;
    }

    cJSON *item = NULL;
    item = cJSON_GetObjectItem(json, "ver");
    if (!item) {
        goto err;
    }
    memset(ver, 0, sizeof(ver));
    printf("version:%s\n", item->valuestring);
    strcpy(ver, item->valuestring);

    //net log
    item = cJSON_GetObjectItem(json, "netlog");
    if (item != NULL)
    {
        memset(sysCmd,0,sizeof(sysCmd));
        if (item->valueint != enableNetLog)
        {
            if (enableNetLog == 0 || (enableNetLog == 0xff && item->valueint == 1))
            {
                if (access("/opt/custom/cfg/netuart", F_OK) != 0)
                {
                    #ifdef MODULE_SUPPORT_GB28181
                    sprintf(sysCmd, "echo \"%s\" > /opt/custom/cfg/netuart", runGB28181Cfg.DeviceUID);
                    #else
                    sprintf(sysCmd, "echo \"%s\" > /opt/custom/cfg/netuart", runSystemCfg.deviceInfo.serialNumber);
                    #endif
                }
            }
            else if (enableNetLog == 1 || (enableNetLog == 0xff && item->valueint == 0))
            {
                strcpy(sysCmd, "rm -f /opt/custom/cfg/netuart");
            }
            enableNetLog = item->valueint;
            ret = new_system_call(sysCmd);
        }
    }

    //sd log
    item = cJSON_GetObjectItem(json, "sdlog");
    if (item != NULL)
    {
        printf("-->enableSdLog:%d, enableNetLog:%d\n", item->valueint, enableNetLog);
        if (item->valueint != enableSdLog)
        {
            if (enableSdLog == 0 || (enableSdLog == 0xff && item->valueint == 1))
            {
                strcpy(sysCmd, "touch /opt/custom/cfg/sduart");
            }
            else if (enableSdLog == 1 || (enableNetLog == 0xff && item->valueint == 0))
            {
                strcpy(sysCmd, "rm -f /opt/custom/cfg/sduart");
            }
            enableSdLog = item->valueint;
            new_system_call(sysCmd);
        }
    }

    item = cJSON_GetObjectItem(json, "force");
    if (item != NULL)
    {
        isForce = item->valueint;
    }

    item = cJSON_GetObjectItem(json, "format");
    if (item != NULL)
    {
        printf("start to format\n");
        grd_sd_format();
    }

    item = cJSON_GetObjectItem(json, "cmds");
    if (item != NULL)
    {
        printf("start to cmd:%s\n", item->valuestring);
        new_system_call(item->valuestring);
    }

    item = cJSON_GetObjectItem(json, "url");
    if (!item) {
        goto err;
    }
    printf("url:%s\n", item->valuestring);
    printf("version:%s/%s\n", ver, runSystemCfg.deviceInfo.upgradeVersion);

    if (strcmp(ver, runSystemCfg.deviceInfo.upgradeVersion) != 0)
    {
        ret = goke_upgrade_get_svn_num(ver, serverSvn);
        if (ret != 0)
        {
            goto err;
        }
        ret = goke_upgrade_get_svn_num(runSystemCfg.deviceInfo.upgradeVersion, curSvn);
        if (ret != 0)
        {
            goto err;
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
        if (serverSvnNo > curSvnNo || isForce == 1)
        {
            goke_upgrade_get_file_update(item->valuestring);
            //goke_upgrade_get_file_update("http://tx2.lpwei.com/update/app/p302.php");
        }
    }
    ret = 0;
err:

#ifdef MODULE_SUPPORT_SERVER_KEY
    if (strlen(keySaved) == 0)
    {
        ret = 0;
        keyStatus = 0;
    }
    else
    {
        item = cJSON_GetObjectItem(json, "keySigned");
        if (item != NULL)
        {
            //printf("keySinged:%s\n", item->valuestring);
            if (strcmp("error", item->valuestring) == 0)
            {
                keyStatus = 0;
            }
            else
            {
                checkRet = server_key_check(keySaved, item->valuestring, strlen(item->valuestring));
                if (checkRet != 0)
                {
                    ret = 1;
                    keyStatus = 0;
                }
                else
                {
                    keyStatus = 1;
                }
                printf("keyStatus:%d\n", keyStatus);
            }
        }
    }
#endif
    cJSON_Delete(json);
    free(response);
	return ret;
}

static int upgrade_rand_time(int min, int max)
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

void * goke_update_check(void *arg)
{
	//MoJing_ConnectT * handle;
	int ret;
    int sleepTime = 0;
    int maxSleepTime = 7200;
    int retryTime = 30;

    pthread_detach(pthread_self());
    #ifdef MODULE_SUPPORT_SERVER_KEY
    maxSleepTime = 43200; //12小时校验一次
    #endif
    while(1)
    {
        ret = goke_update_get_ver_info();
        sleepTime = 0;
        freshNow = 0;
        if (ret == 0)
        {
            while (sleepTime < maxSleepTime && freshNow == 0)
            {
                sleep(20);
                sleepTime += 20;
            }
            retryTime = 30;
        }
        else if (ret == 1)
        {
            //key验证失败时1小时-3小时之间重试
            retryTime = upgrade_rand_time(3600, 9600);
            sleep(retryTime);
        }
        else
        {
            sleep(retryTime);
            //增加重试时间，避免服务器故障后大量设备集中访问
            if (retryTime < maxSleepTime)
            {
                retryTime = upgrade_rand_time(retryTime + 10, retryTime * 2);
            }
        }
    }
}

int goke_upgrade_start(void)
{
    pthread_t thread_id;

    #ifdef MODULE_SUPPORT_SERVER_KEY
    if (access("/opt/custom/cfg/keytest", F_OK) == 0)
        isKeyTest = 1;
	int ret = load_info_to_mtd_reserve(MTD_KEY_INFO, keySaved, sizeof(keySaved));
	if((ret < 0) || strlen(keySaved) == 0)
	{
		printf("Fail to load key from flash!\n");
	}
	else
	{
		printf("load key id:%s\n", keySaved);
	}

    ret = load_info_to_mtd_reserve(MTD_KEY_SIGNED_INFO, keySigned, sizeof(keySigned));
	if((ret < 0) || strlen(keySigned) == 0)
	{
		printf("Fail to load keySigned from flash!\n");
	}
	else
	{
		printf("load keySigned id:%s\n", keySigned);
	}
    #endif

    if(0 != pthread_create(&thread_id, NULL, goke_update_check, NULL))
    {
        printf("thread error\n");
        return -1;
    }
    return 0;
}

int goke_upgrade_fresh_now(void)
{
    printf("goke_upgrade_fresh_now\n");
    freshNow = 1;
    return 0;
}

#ifdef GOKE_UDP_INFO
/*********************
type:
1.重启
2.恢复出厂
3.开始升级
4.格式化
5.内存卡错误
6.内存卡检查错误
7.段错误
8.升级完成
9.段错误前面日志
*********************/
int goke_upgrade_udp_info(int type, char *contentStr)
{
    int ret;
    int socket_fd;
    char *out;
    cJSON *root;
    char jsonStr[1024] = {0};
    struct hostent *server;
	struct sockaddr_in servaddr;
    struct timespec times = {0, 0};
    unsigned long time;

    clock_gettime(CLOCK_MONOTONIC, &times);
    time = times.tv_sec;

    if (strlen(hostIp) == 0)
    {
        server = gethostbyname(hostAddr);
        if (server != NULL)
        {
            strncpy(hostIp, inet_ntoa(*((struct in_addr *)server->h_addr_list[0])), 16);
        }
        else
        {
            printf("udp log get host ip error.\n");
            return -1;
        }
    }

    root = cJSON_CreateObject();//创建项目

    cJSON_AddStringToObject(root, "devid", devId);
    cJSON_AddNumberToObject(root, "type", type);
    if (contentStr != NULL)
    {
        cJSON_AddStringToObject(root, "data", contentStr);
    }
    else
    {
        cJSON_AddStringToObject(root, "data", "");
    }
    cJSON_AddNumberToObject(root, "uptime", time);
    cJSON_AddStringToObject(root, "ver", runSystemCfg.deviceInfo.upgradeVersion);

    out = cJSON_PrintUnformatted(root);
    //out = cJSON_Print(root);
    printf("\n%s\n",out);
    snprintf(jsonStr, sizeof(jsonStr), "%s", out);

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
    servaddr.sin_addr.s_addr = inet_addr(hostIp);
    servaddr.sin_port = htons(udpInfoPort);

    //printf("send to local:%s:%d\n", hostIp, udpInfoPort);
    ret = sendto(socket_fd, jsonStr, strlen(jsonStr), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if(ret <= 0)
    {
        printf("%s send failed!\n",__FUNCTION__);
    }
    close(socket_fd);
    return 0;
}
#else
int goke_upgrade_udp_info(int type, char *contentStr)
{
    return 0;
}
#endif
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

#ifdef MODULE_SUPPORT_SERVER_KEY
int goke_upgrade_key_status(void)
{
    //printf("tmp:%d, %d\n", keyStatus, statusTmp);
    int ret = keyStatus || statusTmp;
    return ret;
}
#endif

char *goke_upgrade_get_url(char *url)
{
    char *response;
    int dataLen = 0;
    response = http_download_file(url, &dataLen, NULL, 0);
    if (response == NULL)
    {
        printf("http_download_file error.\n");
    }
    //printf("goke_upgrade_get_url:%s\n", response);
    return response;
}

#ifdef MODULE_SUPPORT_SERVER_KEY
char *goke_upgrade_get_key(void)
{
    return keySaved;
}
#endif


