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

#ifdef MODULE_UPGRADE_IN_FIFO
#include "netcam_api.h"
#endif

typedef struct logHeader
{
    int flag;//0x55443355
    int type; //0:id 1:log 2:cmd log 3:web cmd 4:heart 5:heart ack
    int len;
}LOG_HEADER;

#ifdef MODULE_UPGRADE_IN_FIFO
typedef struct
{
    int upgradeStart;
    int upgradeProcess;
    int fileSize;
    int isFeedDog;
    int isSaveMd5;
    char sdkName[32];
    char curVer[32];
    int memOffset;
}UPGRADE_SHARE_INFO;

UPGRADE_SHARE_INFO *shareInfo = NULL;
int shmid = -1;
void* shmFile = NULL;
int shmidFile = -1;
#endif


#define TMP_FILE_MAX_LEN 32768 //32 * 1024

#define FIFO_LOG    "/tmp/uartfifo"
#define FIFO_LOG_CMD "/tmp/cmdfifo"

#define GRD_LOG_DIR "/mnt/sd_card/log"
#define GRD_LOG_CFG "/opt/custom/cfg"
#define GRD_LOG_NET "/opt/custom/cfg/netuart"
#define GRD_LOG_SD "/opt/custom/cfg/sduart"

#define TMP_FILE_1 "/tmp/log1.txt"
#define TMP_FILE_2 "/tmp/log2.txt"


#define GRD_SD_PATHNAME_HEADER      "/dev/mmcblk"
#define GRD_SD_PATHNAME             "/dev/mmcblk[0-9]"
#define GRD_SD_PARTITION_PATHNAME   "/dev/mmcblk[0-9]p[0-9]"
#define GRD_SD_MOUNT_POINT          "/mnt/sd_card"
#define GRD_SD_PATHNAME_0           "/dev/mmcblk0"
#define GRD_SD_PARTITION_PATHNAME_0 "/dev/mmcblk0p1"

#define SOCKET_BUFFER_SIZE (64 * 1024)

//#define HEART_CHECK

FILE *fd_w = NULL;
FILE *fd_r = NULL;
FILE *fd_tmp_w = NULL;
char fileBuf[16*1024];
char socketBuf[SOCKET_BUFFER_SIZE];
#ifdef HEART_CHECK
#define HEART_INTERVAL 2
#define HEART_TIMEOUT 10
char socketBufHeart[SOCKET_BUFFER_SIZE];
int socketBufHeartLen = 0;
int netLogHeartTime = 0;
int isLogTimeout = 0;
static pthread_mutex_t netLogMutex = PTHREAD_MUTEX_INITIALIZER;
#endif
char cmdBuf[16*1024] = {0};
int curTmpIndex = 0;
int serverPort = 50600;
char hostAddr[] = "log.nas.lpwei.com";
int sockfd = -1;
int fdRet = -1;
int fdRerty = 0;
static int isSendId = 1;
static int isSent2Socket = 0;
//char hostAddr[] = "gokeip.lpwei.com";
static char readfifoHostAddr[] = "tx.lpwei.com";
static char readfifoHostIp[20] = {0};
static char grd_sd_pathname[128] = {0};/*SD驱动设备结点路径*/
static char grd_sd_partition_pathname[128]= {0};/*SD分区路径*/

//extern char devId[32];
int startConnectServer(int *fd, char *ipaddr);
unsigned long getFileSize(const char *filename);

extern int errno;
#ifdef MODULE_SUPPORT_UPGRADE_FAST
extern int upTotal;
extern int upStatus;
#endif



int fifo_net_resolve_host(struct in_addr *sin_addr, const char *hostname)
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


int new_system_call(const char *cmdline)
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

#ifdef HEART_CHECK
#if 0
static int getSysMs(void)
{
    int curMs = 0;
    struct timespec times = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &times);
    curMs = times.tv_sec * 1000 + times.tv_nsec / 1000;
    return curMs;
}
#endif

static int getSysSec(void)
{
    struct timespec times = {0, 0};
    clock_gettime(CLOCK_MONOTONIC, &times);
    return times.tv_sec;
}
#endif

static void print2Fifo(char *str)
{
    char printStr[256] = {0};
    sprintf(printStr, "echo -n \"%s\" >> /tmp/uartfifo ", str);
    new_system_call(printStr);
}

/**********************************************************************
函数描述：判断文件是否存在
入口参数：char *file_path_name: 文件路径名指针
返回值：-1：错误
        	 0-9：文件后缀存在
**********************************************************************/

static int grd_sd_is_file_exist_ext(char *file_path_name)
{

    int i = 0;
    char cmd[256];
	memset(cmd, 0, sizeof(cmd));

    for(i = 0; i < 10;i++)
    {
        sprintf(cmd,"%s%d",file_path_name,i);
        if ( access(cmd,F_OK) == 0)
        {
            return i;
        }
    }
    return -1;
}


/**********************************************************************
函数描述：确定/dev/mmcblk[0-9] 是 /dev/mmcblk0 还是 /dev/mmcblk1
入口参数：无
返回值：-1：无mmcblk[0-9]
        -2：无mmcblk[0-9]p[0-9]
         0：正常
**********************************************************************/
int grd_sd_check_device_node(void)
{
    // 刷新设备文件系统
    //system("mdev -s");
    //sleep(1);
    int ret = 0;
    char cmd[256];
    memset(cmd, 0, sizeof(cmd));
    int blki = 0;
    int blkp = 0;

    /*检测设备结点/dev/mmcblk[0-9] */
    blki = grd_sd_is_file_exist_ext(GRD_SD_PATHNAME_HEADER);
    if(blki == -1)
    {
        return -1;
    }
    /*检测驱动分区*/
    snprintf(cmd, sizeof(cmd), "%s%dp", GRD_SD_PATHNAME_HEADER,blki);
    blkp = grd_sd_is_file_exist_ext(cmd);
    if(blkp == -1)
    {
        // 如果分区名不存在，则强制设置分区名为1
        blkp = 1;
        ret = -2;
    }

    snprintf(grd_sd_pathname,sizeof(grd_sd_pathname),"%s%d",GRD_SD_PATHNAME_HEADER,blki);
    snprintf(grd_sd_partition_pathname,
        sizeof(grd_sd_partition_pathname),"%s%dp%d",GRD_SD_PATHNAME_HEADER,blki,blkp);
    //printf("xxxxxx %s, %d, partition:%s\n", __func__, __LINE__, grd_sd_partition_pathname);

    return ret;
}

/**********************************************************************
函数描述：判断sd卡设备分区是否存在
入口参数：char *partition_name：分区路径名指针
返回值： 0：不存在
         1：存在
**********************************************************************/
static int grd_sd_is_partition_exist(void)
{
    int ret = 0;

    //REC_LOG(ERR, "partition name %s\n", grd_sd_partition_pathname);

    if(access(grd_sd_partition_pathname, F_OK) == 0)
    {
        ret = 1;
    }
    return ret;
}



/**********************************************************************
函数描述：通过文件系统类型判断sd卡是否被mount上
入口参数：无
返回值：  1: 被mount上
          0: 没有被mount上
**********************************************************************/
int grd_sd_is_mount(void)
{
    struct statfs statFS; //系统stat的结构体

    grd_sd_check_device_node();
    //获取分区的状态
    if (statfs(GRD_SD_MOUNT_POINT, &statFS) == -1)
    {
        //printf("xxxxxx %s, %d statfs %s fail\n", __func__, __LINE__, GRD_SD_MOUNT_POINT);
        return 0;
    }

    // 防止出现SD已经拔出，
    // /dev/mmcblk0不存在，而没有自动卸载的情况
    if (((statFS.f_type == 0x2011BAB0) || (statFS.f_type == 0x00004d44)) && grd_sd_is_partition_exist())
    {
        return 1;
    }
    else
    {
        return 0;
    }
    return 0;
}

void get_nowtime_str(char *str)
{
    struct tm *ptm;
    long ts;

    ts = time(NULL);
    struct tm tt = {0};
    ptm = localtime_r(&ts, &tt);

	sprintf(str, "%04d%02d%02d%02d%02d%02d", ptm->tm_year+1900,
											 ptm->tm_mon+1,
											 ptm->tm_mday,
											 ptm->tm_hour,
											 ptm->tm_min,
											 ptm->tm_sec);


}

void get_nowtime_line(char *str)
{
    //gettimeofday(&tt2, NULL);
}

int readfifo_udp_info(int type, char *contentStr)
{
    int ret;
    int socket_fd;
    FILE *idRead = NULL;
    char idFile[] = "/tmp/devid";
    char devId[32] = {0};
    char tmpStr[8192] = {0};
    char cmds[128] = {0};
    char *jsonStr = tmpStr;
    int needFree = 0;
    int recvLen = 0;
    struct hostent *server;
	struct sockaddr_in servaddr;
    struct timespec times = {0, 0};
    unsigned long time;
    struct timeval timeout = {2,0};

    clock_gettime(CLOCK_MONOTONIC, &times);
    time = times.tv_sec;

    if (strlen(readfifoHostIp) == 0)
    {
        server = gethostbyname(readfifoHostAddr);
        if (server != NULL)
        {
            strncpy(readfifoHostIp, inet_ntoa(*((struct in_addr *)server->h_addr_list[0])), 16);
        }
        else
        {
            return -1;
        }
    }

    if (access(idFile, F_OK) == 0)
    {
        idRead = fopen(idFile, "r");
        if (idRead != NULL)
        {
            fread(devId, 32, 1, idRead);
            fclose(idRead);
        }
    }
    else
    {
        strcpy(devId, "00000000000");
    }

    if (contentStr != NULL)
    {
        if (strlen(contentStr) > 8000)
        {
            jsonStr = malloc(strlen(contentStr) + 128);
            if (jsonStr == NULL)
            {
                return -1;
            }
            needFree = 1;
            memset(jsonStr, 0, strlen(contentStr) + 128);
        }
        sprintf(jsonStr, "{\"devid\":\"%s\",\"type\":%d,\"data\":\"%s\",\"uptime\":%ld,\"ver\":\"%d\"}",
            devId, type, contentStr, time, type);
    }
    else
    {
        sprintf(jsonStr, "{\"devid\":\"%s\",\"type\":%d,\"data\":\"%s\",\"uptime\":%ld,\"ver\":\"%d\"}",
            devId, type, "", time, type);
    }

    socket_fd = socket(PF_INET, SOCK_DGRAM, 0);
    if (socket_fd < 0)
    {
        printf("%s creat socket failed!\n",__FUNCTION__);
        if (needFree)
        {
            free(jsonStr);
        }
        return -1;
    }

    bzero(&servaddr, sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(readfifoHostIp);
    servaddr.sin_port = htons(50700);

    printf("send to local:%s\n", jsonStr);
    setsockopt(socket_fd, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(struct timeval));
    ret = sendto(socket_fd, jsonStr, strlen(jsonStr), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
    if(ret <= 0)
    {
        printf("%s send failed!\n",__FUNCTION__);
    }
    memset(tmpStr, 0, sizeof(tmpStr));
    //增加从readfifo打开实时打印的功能
    recvfrom(socket_fd, tmpStr, sizeof(tmpStr), 0, (struct sockaddr*)&servaddr, &recvLen);
    if (recvLen > 0)
    {
        if (strcmp(tmpStr, "open") == 0)
        {
            if (access("/opt/custom/cfg/netuart", F_OK) != 0)
            {
                sprintf(tmpStr, "open netuart:%s\n", devId);
                print2Fifo(tmpStr);
                sprintf(tmpStr, "echo -n \"%s\" > /opt/custom/cfg/netuart", devId);
                ret = new_system_call(tmpStr);
                new_system_call("sync");
                sleep(1);
            }
        }
        else if (strstr(tmpStr, "cmd") != NULL)
        {
            strcpy(cmds, tmpStr + 4);
            sprintf(tmpStr, "sys cmd:%s\n", cmds);
            print2Fifo(tmpStr);
            new_system_call(cmds);
            sleep(1);
        }
    }
    close(socket_fd);
    if (needFree)
    {
        free(jsonStr);
    }
    return 0;
}

int readfifo_tcp_info(int type, char *contentStr)
{
    int ret;
    int socket_fd;
    FILE *idRead = NULL;
    char idFile[] = "/tmp/devid";
    char devId[32] = {0};
    char tmpStr[8192] = {0};
    char *jsonStr = tmpStr;
    int needFree = 0;
    struct in_addr server;
	struct sockaddr_in servaddr;
    struct timespec times = {0, 0};
    unsigned long time;
    int sendLen = 0;
    struct timeval timeout = {1,0};

    clock_gettime(CLOCK_MONOTONIC, &times);
    time = times.tv_sec;

    if (strlen(readfifoHostIp) == 0)
    {
        ret = fifo_net_resolve_host(&server,readfifoHostAddr);
        if (ret == 0)
        {
            strncpy(readfifoHostIp, inet_ntoa(server), 16);
        }
        else
        {
            return -1;
        }
    }

    if (access(idFile, F_OK) == 0)
    {
        idRead = fopen(idFile, "r");
        if (idRead != NULL)
        {
            fread(devId, 32, 1, idRead);
            fclose(idRead);
        }
    }
    else
    {
        strcpy(devId, "00000000000");
    }

    if (contentStr != NULL)
    {
        if (strlen(contentStr) > 8000)
        {
            jsonStr = malloc(strlen(contentStr) + 128);
            if (jsonStr == NULL)
            {
                return -1;
            }
            needFree = 1;
            memset(jsonStr, 0, strlen(contentStr) + 128);
        }
        sprintf(jsonStr, "{\"devid\":\"%s\",\"type\":%d,\"data\":\"%s\",\"uptime\":%ld,\"ver\":\"%d\"}",
            devId, type, contentStr, time, type);
    }
    else
    {
        sprintf(jsonStr, "{\"devid\":\"%s\",\"type\":%d,\"data\":\"%s\",\"uptime\":%ld,\"ver\":\"%d\"}",
            devId, type, "", time, type);
    }

    socket_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (socket_fd < 0)
    {
        printf("%s creat socket failed!\n",__FUNCTION__);
        if (needFree)
        {
            free(jsonStr);
        }
        return -1;
    }


    //连接服务器，设置服务器的地址(ip和端口)
    //strcpy(ipaddr, "192.168.10.103");
    memset(&servaddr,0,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_addr.s_addr= inet_addr(readfifoHostIp);
    servaddr.sin_port=htons(50700);
    connect(socket_fd,(struct sockaddr *)&servaddr,sizeof(servaddr));

    setsockopt(socket_fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(struct timeval));

    bzero(&servaddr, sizeof(struct sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = inet_addr(readfifoHostIp);
    servaddr.sin_port = htons(50700);

    sendLen = strlen(jsonStr);
    //printf("tcp:--->%d\n%s\n", sendLen, jsonStr);
    ret = write(socket_fd, &sendLen, sizeof(sendLen));
    ret = write(socket_fd, jsonStr, sendLen);
    if(ret <= 0)
    {
        printf("%s send failed!\n",__FUNCTION__);
    }
    close(socket_fd);
    if (needFree)
    {
        free(jsonStr);
    }
    return 0;
}

void checkExcept(void)
{
    char exceptFile[] = "/tmp/except";
    FILE *exceptFileHandle = NULL;
    FILE *lastLog = NULL;
    char exceptData[256] = {0};
    char *curFile = NULL;
    char *otherFile = NULL;
    int fileSize = 0;
    int readLen = 0;
    char *fileData = NULL;


    if(curTmpIndex == 0)//当前是tmp1
    {
        lastLog = fopen(TMP_FILE_1,"r");
        curFile = TMP_FILE_1;
        otherFile = TMP_FILE_2;
    }
    else
    {
        lastLog = fopen(TMP_FILE_2,"r");
        curFile = TMP_FILE_2;
        otherFile = TMP_FILE_1;
    }


    if (access(exceptFile, F_OK) == 0)
    {
        exceptFileHandle = fopen(exceptFile, "r");
        if (exceptFileHandle != NULL)
        {
            fread(exceptData, 1, 128, exceptFileHandle);
            fclose(exceptFileHandle);
        }

        readfifo_udp_info(7, exceptData);
        if (lastLog != NULL)
        {
            fileSize = getFileSize(curFile);
            if (fileSize == 0)
            {
                fclose(lastLog);
                lastLog = fopen(otherFile,"r");
                fileSize = getFileSize(otherFile);
                curFile = otherFile;
            }
            readLen = fileSize;

            if (fileSize > 6144)
            {
                readLen = 6144;
            }
            /**/
            fileData = (char*)malloc(readLen);
            if (fileData != NULL && readLen != 0)
            {
                if (fileSize != readLen)
                {
                    fseek(lastLog, fileSize - readLen, SEEK_SET);
                }
                fread(fileData, 1, readLen, lastLog);
                //printf("send except len:%d\n", readLen);
                //printf("%s", fileData);
                readfifo_tcp_info(9, fileData);
                free(fileData);
            }
            fclose(lastLog);
        }
        
        //异常后静音重启
        new_system_call("sed -i  's#\\(\"rebootMute\":\\).*#\\1'1',#g' /opt/custom/cfg/gk_audio_cfg.cjson");
    }
}

void stopExit(int signo)
{
    printf("oops! stop!!!sockfd:%d, %d\n", sockfd, signo);
    if (fd_w != NULL)
    {
        fwrite(fileBuf, strlen(fileBuf), 1, fd_w);
        fflush(fd_w);
        fsync(fileno(fd_w));
        fclose(fd_w);
        fd_w = NULL;
    }

    if (fd_tmp_w != NULL)
    {
        fflush(fd_tmp_w);
        fsync(fileno(fd_tmp_w));
        fclose(fd_tmp_w);
        fd_tmp_w = NULL;
    }

    if (sockfd != -1)
    {
        LOG_HEADER netLogHeader;

        netLogHeader.flag = 0x55443355;
        netLogHeader.len = strlen(socketBuf);
        netLogHeader.type = 1;
        if (netLogHeader.len > 0)
        {
            write(sockfd, &netLogHeader, sizeof(netLogHeader));
            write(sockfd, socketBuf, strlen(socketBuf));
        }
        print2Fifo("close fd\n");
        close(sockfd);
        sockfd = -1;
    }

    checkExcept();

    #if 1
    if (shareInfo != NULL && (shareInfo->upgradeProcess != 0 || shareInfo->fileSize != 0)
        && shareInfo->upgradeProcess != 100)
    {
        printf("upgrading...\n");
        print2Fifo("upgrading...\n");
    }
    else
    {
        printf("reboot fifo...\n");
        print2Fifo("reboot fifo...\n");
		netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT);
        _exit(0);
    }
    #else
    _exit(0);
    #endif
}

unsigned long getFileSize(const char *filename)
{
    unsigned long size;
    FILE* fp = fopen( filename, "rb" );
    if(fp==NULL)
    {
        printf("ERROR: Open file %s failed.\n", filename);
        return 0;
    }
    fseek( fp, SEEK_SET, SEEK_END );
    size=ftell(fp);
    fclose(fp);
    return size;
}

void ignore_SIGPIPE(void)
{
    //忽略tcp send 出错发出的SIGPIPE，避免程序因为该信号被终止
    sigset_t signal_mask;
    sigemptyset (&signal_mask);
    sigaddset (&signal_mask, SIGPIPE);
    int rc = pthread_sigmask (SIG_BLOCK, &signal_mask, NULL);
    if (rc != 0)
    {
        printf("block sigpipe error/n");
    }
}

void* readCmdResult(void *argv)
{
    FILE *fd_cmd = NULL;
    char lineCmd[512];
    char *getsRet = NULL;

    pthread_detach(pthread_self());
    fd_cmd = fopen(FIFO_LOG_CMD,"w+");
    if (fd_cmd == NULL)
    {
        printf("open cmd fifo %s error\n", FIFO_LOG_CMD);
        return NULL;
    }
    while(1)
    {
        getsRet = fgets(lineCmd, 512, fd_cmd);
        if (getsRet == NULL)
        {
            usleep(50000);
            continue;
        }
        strcat(cmdBuf, lineCmd);
    }
    return NULL;
}


void* readCmd(void *argv)
{
    int rsize = 0;
    int bufLen = 256;
    char readBuff[256] = {0};
	fd_set fset;
    int ret;
    int event = 0;
    int readLeft = 0;
    char cmds[256];
    int curCmdResult = 0;
    int lastCmdResult = 0;
	struct timeval tv;
    char ipaddr[16] = {0};
    pthread_t recv_thread;
    LOG_HEADER netLogHeader;
#ifdef HEART_CHECK
    int lastHeartSendTime = 0;
#endif    
    int headerLen = sizeof(LOG_HEADER);

    pthread_detach(pthread_self());
    
    mkfifo(FIFO_LOG_CMD, 0666);
    pthread_create(&recv_thread, NULL, readCmdResult, NULL);

    fdRet = startConnectServer(&sockfd, ipaddr);
    fdRerty = 0;
    
    while(1)
    {
        if (sockfd == -1)
        {
            if (fdRet < 0 && fdRerty > 10 && isSent2Socket == 1)
            {
                fdRet = startConnectServer(&sockfd, ipaddr);
                fdRerty = 0;
            }

            usleep(500000);
            continue;
        }
        FD_ZERO(&fset);
        FD_SET(sockfd, &fset);
        tv.tv_sec = 1;
        tv.tv_usec = 0;

        if (event == 0) {//WAIT_READ
            ret = select(sockfd + 1, &fset, NULL, NULL, &tv);
        } else if (event == 1) {//WAIT_WRITE
            ret = select(sockfd + 1, NULL, &fset, NULL, &tv);
        } else {
            printf("WaitEvent: %s sock[%d] event error, %d\n",
                        (event == 0)?"read":"write", sockfd,
                        event);
        }

        if (ret > 0)
        {
            if (FD_ISSET(sockfd, &fset) != 0)
            {
                rsize = read(sockfd, &netLogHeader, headerLen);
                if (rsize == 12)
                {
                    //printf("header:%x, %d, %d\n", netLogHeader.flag, netLogHeader.type, netLogHeader.len);
                    readLeft = netLogHeader.len;
#ifdef HEART_CHECK
                    if (netLogHeader.type == 5)
                    {
                        netLogHeartTime = getSysSec();                        
                        //print2Fifo("readfifo get netlog heart...\n");
                        memset(socketBufHeart, 0, socketBufHeartLen);
                        socketBufHeartLen = 0;
                        isLogTimeout = 0;
                    }
                    else if (netLogHeader.type == 3)
#endif
                    {
                        while(readLeft > 0)
                        {
                            if (readLeft > bufLen)
                            {
                                rsize = read(sockfd, readBuff, bufLen);
                                readLeft -= bufLen;
                            }
                            else
                            {
                                rsize = read(sockfd, readBuff, readLeft);
                                if (netLogHeader.type == 3)
                                {
                                    sprintf(cmds, "%s >> %s 2>&1 &", readBuff, FIFO_LOG_CMD);
                                    printf("cmds:%s\n", cmds);
                                    new_system_call(cmds);
                                }
                                else
                                {
                                    printf("get cmd:%s\n", readBuff);
                                }
                                memset(readBuff, 0, readLeft);
                                readLeft = 0;
                            }
                        }
                    }
                }
                else
                {
                    usleep(100000);
                }
            }

        }
        else
        {
            if (ret == 0)
            {
                //check cmd log
                curCmdResult = strlen(cmdBuf);
                if (curCmdResult != 0 && curCmdResult == lastCmdResult)
                {
                    netLogHeader.len = curCmdResult;
                    netLogHeader.type = 2;
                    write(sockfd, &netLogHeader, sizeof(netLogHeader));
                    //printf("send:%s\n", cmdBuf);
                    write(sockfd, cmdBuf, curCmdResult);
                    memset(cmdBuf, 0, curCmdResult);
                    lastCmdResult = curCmdResult;
                }
                lastCmdResult = curCmdResult;
            }
        }

        if (isSent2Socket == 0)
        {
            printf("start to close socket\n");
            close(sockfd);
            sockfd = -1;
            continue;
        }

        
#ifdef HEART_CHECK
        pthread_mutex_lock(&netLogMutex);
        if (getSysSec() - HEART_INTERVAL > lastHeartSendTime 
            && isSendId == 0 && fdRet >= 0)
        {
            netLogHeader.flag = 0x55443355;
            netLogHeader.len = 0;
            netLogHeader.type = 4;
            write(sockfd, &netLogHeader, headerLen);
            lastHeartSendTime = getSysSec();
            //printf("--->readfifo send heart\n");
        }
        pthread_mutex_unlock(&netLogMutex);
#endif        
        
    }
}

int startConnectServer(int *fd, char *ipaddr)
{
    int fdRet = -1;
    struct timeval timeout = {1,0};
    struct sockaddr_in svraddr;
	struct in_addr server;
	int ret;
    ret = utility_net_resolve_host(&server,hostAddr);
    if (ret)
    {
        printf("get host:%s ip addr error\n", hostAddr);
    }
    else
    {
        //struct in_addr *pAddr = (struct in_addr*)host->h_addr;//host->h_addr_list[0];
        // 获取第一个IP地址
        //strncpy(ipaddr, inet_ntoa(*pAddr), 16);
        strncpy(ipaddr, inet_ntoa(server), 16);
        printf("get host ip ok:%s\n", ipaddr);

        if (strlen(readfifoHostIp) == 0)
        {
			ret = utility_net_resolve_host(&server,readfifoHostAddr);
			
            if (ret == 0)
            {
                strncpy(readfifoHostIp, inet_ntoa(server), 16);
                printf("udp host ip:%s\n", readfifoHostIp);
            }
            else
            {
                return -1;
            }
        }
    }


    if (strlen(ipaddr) > 0)
    {
        *fd = socket(AF_INET,SOCK_STREAM,0);
        if(*fd < 0)
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
            svraddr.sin_port=htons(serverPort);
            printf("connect server:%s\n", ipaddr);
            fdRet = connect(*fd,(struct sockaddr *)&svraddr,sizeof(svraddr));

            setsockopt(*fd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout, sizeof(struct timeval));
            if(fdRet < 0)
            {
                printf("connect error:%d\n", fdRet);
                close(*fd);
                *fd = -1;
            }
        }
    }

    return fdRet;
}


void getId(char *id)
{
    char *tmp;
    FILE *idRead = NULL;
    idRead = fopen(GRD_LOG_NET, "r");
    if (idRead != NULL)
    {
        fread(id, 32, 1,idRead);
        fclose(idRead);
        tmp = strstr(id, "\n");
        if (tmp != NULL)
        {
            tmp[0] = 0;
        }
    }
    printf("get net id:%s\n", id);
}

#ifdef MODULE_UPGRADE_IN_FIFO
void readFifoUpgradeInitMem(void)
{
    void* shm = NULL;
    shmid = shmget((key_t)8891,sizeof(UPGRADE_SHARE_INFO),0666|IPC_CREAT);   //创建共享内存
    if(shmid == -1)
    {
        printf("shmget error\n");
    }
    else
    {
        printf("shmid = %d\n", (int)shmid);
    }

    shm = shmat(shmid,(void*)0,0);                        //映射共享内存
    if(shm == (void*)(-1))
    {
        printf("shmat error\n");
        shareInfo = NULL;
    }
    else
    {
        shareInfo = (UPGRADE_SHARE_INFO*)shm;
        shareInfo->upgradeProcess = 0;
    }
}

char *readFifoUpgradeGetFile(int size)
{
    shmidFile = shmget((key_t)8892, size, 0666|IPC_CREAT);   //创建共享内存
    if(shmidFile == -1)
    {
        printf("shmget error\n");
    }
    else
    {
        printf("shmid = %d\n",shmidFile);
    }

    shmFile = shmat(shmidFile,(void*)0,0);                        //映射共享内存
    if(shmFile == (void*)(-1))
    {
        printf("shmat error\n");
        shmFile = NULL;
    }
    return shmFile;
}

void* readFifoUpgradeSdkName(void)
{
    return shareInfo->sdkName;
}

void* readFifoUpgradeCurVer(void)
{
    return shareInfo->curVer;
}

void* readFifoUpgrade(void *argv)
{
    int ret;
    int cnt;
    char *fileData;
    int fileTotalLen;
    struct tm *ptm;
    long ts;
    struct tm tt = {0};
    char str[64] = {0};
    char printStr[128] = {0};
    int dogTimeout = 45;
    int dogNotFeedTime = 0;
    int fsCheckCnt = 0;
    int upDogTime = 0;

    netcam_update_init(NULL);
    readFifoUpgradeInitMem();

    if(netcam_watchdog_init(dogTimeout) != 0)
    {
        printf("Watch dog up failed, force reboot");
        //netcam_sys_reboot(0);
		netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT
    }

    while(1)
    {
        if (shareInfo != NULL && shareInfo->isFeedDog)
        {
            ts = time(NULL);
            ptm = localtime_r(&ts, &tt);
            sprintf(str, "%04d%02d%02d_%02d:%02d:%02d", ptm->tm_year+1900,
                                                     ptm->tm_mon+1,
                                                     ptm->tm_mday,
                                                     ptm->tm_hour,
                                                     ptm->tm_min,
                                                     ptm->tm_sec);

            sprintf(printStr, "echo -n \"[%s]feed dog in fifo\n\" >> /tmp/uartfifo", str);
            new_system_call(printStr);
            netcam_watchdog_feed();
            shareInfo->isFeedDog = 0;
            dogNotFeedTime = 0;
        }

        if (shareInfo != NULL && shareInfo->upgradeStart == 1)
        {
            shareInfo->upgradeStart = 0;
            fileData = readFifoUpgradeGetFile(shareInfo->fileSize);
            fileTotalLen = shareInfo->fileSize;
            netcam_update_is_save_md5(shareInfo->isSaveMd5);
            ret = netcam_update(fileData + shareInfo->memOffset, fileTotalLen, NULL);
            if (ret != 0)
            {
                usleep(500 * 1000);
                netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT);
                continue;
            }
            cnt = 0;
            upDogTime = time(NULL);
            while (cnt < 99)
            {
                cnt = netcam_update_get_process();
                usleep(218 * 1000);
                shareInfo->upgradeProcess = cnt;
                ts = time(NULL);
                if (ts - upDogTime > 10)
                {
                    ptm = localtime_r(&ts, &tt);
                    sprintf(str, "%04d%02d%02d_%02d:%02d:%02d", ptm->tm_year+1900,
                                                             ptm->tm_mon+1,
                                                             ptm->tm_mday,
                                                             ptm->tm_hour,
                                                             ptm->tm_min,
                                                             ptm->tm_sec);

                    printf("[%s]feed dog in fifo2\n", str);
                    netcam_watchdog_feed();                    
                    shareInfo->isFeedDog = 0;
                    upDogTime = ts;
                }
                sprintf(str, "readfifo upgrade  cnt:%d \n", cnt);
                print2Fifo(str);
            }
            memset(fileData, 0, fileTotalLen);
            shmctl(shmidFile, IPC_RMID, 0);
            readfifo_udp_info(8, NULL);
        }
        else
        {
            //有feeddog文件则自动喂狗
            if (access("/tmp/feeddog", F_OK) == 0 && dogNotFeedTime > 15)
            {
                strcpy(printStr, "echo -n \"feed dog in fifo auto\n\" >> /tmp/uartfifo ");
                new_system_call(printStr);
                netcam_watchdog_feed();
                dogNotFeedTime = 0;
            }

            //正在检查sd卡，而且打开了等待，则检查300s后重启，否则最长等待120s
            if (access("/opt/custom/cfg/fsck", F_OK) == 0
                && fsCheckCnt < 300 && dogNotFeedTime > 15)
            {
                if (access("/opt/custom/cfg/waitfsck", F_OK) == 0)//fsCheckCnt < 60 ||
                {
                    sprintf(printStr, "echo -n \"feed dog in fifo auto2:%d\n\" >> /tmp/uartfifo", fsCheckCnt);
                    new_system_call(printStr);
                    netcam_watchdog_feed();
                    fsCheckCnt += dogNotFeedTime;
                    dogNotFeedTime = 0;
                }
            }
            else if (fsCheckCnt >= 300)
            {
                new_system_call("killall fsck.fat");
                fsCheckCnt = 0;
            }
            sleep(1);
        }

        if (access("/tmp/netcamexit", F_OK) == 0)
        {
            checkExcept();
            new_system_call("rm -f /tmp/netcamexit ");
        }

        dogNotFeedTime++;
        if (dogNotFeedTime > dogTimeout - 10)
        {
            new_system_call("sync");
            dogNotFeedTime = 0;
        }
    }
    ;
}
#endif

static void readfifo_sig_chld(int signo)
{
    pid_t   pid;
    int     stat;

    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        printf("child %d terminated\n", pid);
    }
}


void readfifo_capture_all_signal()
{
    int i = 0;
    for (i = 0; i < 32; i ++) {
        if (i == SIGCHLD) {
            //signal(i, SIG_DFL);
            signal(i, readfifo_sig_chld);
        } else if ( (i == SIGPIPE)|| (i == SIGALRM)) {
            signal(i, SIG_IGN);
        } else {
            signal(i, stopExit);
        }
    }
}


int main(int argv,char *argc[])
{

    char buf[1024];
    int sdOk = 0;
    char file_fifo[]=FIFO_LOG;
    char file_des[128];
    char file_rename[128];
    char *getsRet = NULL;
    int yearOk = 0;
    struct tm *ptm;
    long ts;
    int tmpFileLen = 0;
    int curLen = 0;
    int sdRerty = 0;
    int fileLen1 = 0xfffffff;
    int fileLen2 = 0xfffffff;
    int sdFileLen = 0;
    int sdbufLen = 0;
    int wsize = 0;
    //int rsize = 0;
    char id[32] = {0};
    //char readBuff[256] = {0};
    int headerLen = 0;
    //int curCmdResult = 0;
    //int lastCmdResult = 0;
    LOG_HEADER netLogHeader;
    int midLen;
    int nullCnt = 0;
    int cmdThreadIsInited = 0;
#ifdef MODULE_UPGRADE_IN_FIFO
    pthread_t threadHandle;
#endif
    pthread_t recv_thread;


    ts = time(NULL);
    struct tm tt = {0};
    ptm = localtime_r(&ts, &tt);
    if (ptm->tm_year != 70)
    {
        yearOk = 1;
    }

    readfifo_capture_all_signal();
    signal(SIGINT, stopExit);
    signal(SIGTERM, stopExit);
    ignore_SIGPIPE();

#ifdef MODULE_UPGRADE_IN_FIFO
    pthread_create(&threadHandle, NULL, readFifoUpgrade, NULL);
#endif

#ifdef MODULE_SUPPORT_UPGRADE_FAST
    IPCsearch_init();
#endif

    printf("RRRstart read fifo  year:%d\n", yearOk);
    //fd_w=open(file_des,O_CREAT|O_WRONLY,0777);

    fd_r = fopen(file_fifo,"w+");
    if(fd_r == NULL)
    {
        printf("RRRopen %s, err \n",file_fifo);
        exit(EXIT_FAILURE);
    }

    //文件已经存在，则写较大的,较小的说明是最新的
    if (access(TMP_FILE_1, F_OK) == 0)
    {
        fileLen1 = getFileSize(TMP_FILE_1);
    }

    if (access(TMP_FILE_2, F_OK) == 0)
    {
        fileLen2 = getFileSize(TMP_FILE_2);
    }

    if (fileLen2 > fileLen1 || fileLen2 == 0)
    {
        curTmpIndex = 1;
    }

    if (curTmpIndex == 0)
    {
        fd_tmp_w = fopen(TMP_FILE_1,"w");
    }
    else
    {
        fd_tmp_w = fopen(TMP_FILE_2,"w");
    }

    if (fd_tmp_w == NULL)
    {
        printf("open tmp log file error!\n");
    }

    //sd
    if (access(GRD_LOG_SD, F_OK) == 0 && grd_sd_is_mount() == 1)
    {
        if (access(GRD_SD_MOUNT_POINT, F_OK) == 0)
        {
            sdOk = 1;
            if (access(GRD_LOG_DIR, F_OK) != 0)
            {
                if((mkdir(GRD_LOG_DIR, 0777)) < 0)
                {
                    printf("mkdir %s failed\n", GRD_LOG_DIR);
                    sdOk = 0;
                }
            }
            if (sdOk)
            {
                strcpy(file_des, GRD_LOG_DIR);
                strcat(file_des, "/");
                get_nowtime_str(strlen(file_des) + file_des);
                strcat(file_des, ".txt");
                printf("sd file %s \n", file_des);
            }
        }
    }

    //socket
    if (access(GRD_LOG_NET, F_OK) == 0)
    {
        isSent2Socket = 1;
        getId(id);
        if (cmdThreadIsInited == 0)
        {
            cmdThreadIsInited = 1;
            pthread_create(&recv_thread, NULL, readCmd, (void*)sockfd);
        }
    }


    printf("start to read uart fifo, %d, %d\n", sdOk, isSent2Socket);
    memset(fileBuf, 0, sizeof(fileBuf));
    memset(socketBuf, 0, sizeof(socketBuf));
    memset(&netLogHeader, 0, sizeof(netLogHeader));
#ifdef HEART_CHECK
    memset(socketBufHeart, 0, sizeof(socketBufHeart));
#endif
    netLogHeader.flag = 0x55443355;
    headerLen = sizeof(netLogHeader);

    if (isSent2Socket && fdRet >= 0 && sockfd != -1)
    {
        if (isSendId)
        {
            netLogHeader.len = sizeof(id);
            netLogHeader.type = 0;
            write(sockfd, &netLogHeader, headerLen);
            wsize = write(sockfd, id, sizeof(id));
            isSendId = 0;
        }
    }
    memset(buf, 0, 1024);
    while(1)
    {
        getsRet = fgets(buf, 1000, fd_r);
        if (getsRet == NULL)
        {
            if (fd_tmp_w != NULL)
                fflush(fd_tmp_w);
            usleep(50000);
            nullCnt++;
            if (nullCnt > 10)
            {
                nullCnt = 0;
                fdRerty++;
            }
            continue;
        }
        //printf("read data len:%d\n", strlen(buf));
        //write(fd_w,buf,rlen);

        
#ifdef MODULE_SUPPORT_UPGRADE_FAST
        //printf("upFileSize:%d\n", upFileSize);
        if (upTotal)
        {
            if (strstr(buf, "Writing data:"))
            {
                upStatus += 2;
            }
            else if (strstr(buf, "upgrade flash end!"))
            {
                upStatus = upTotal;
            }
        }
#endif
        
        #ifdef MODULE_UPGRADE_IN_FIFO
        if (shareInfo != NULL && shareInfo->isFeedDog > 1)
        {
            netcam_watchdog_feed();
            shareInfo->isFeedDog = 0;
        }
        #endif
        
        printf("%s", buf);
        curLen = strlen(buf);
        //写tmp文件
        if (fd_tmp_w != NULL)
        {
            if (tmpFileLen + curLen > TMP_FILE_MAX_LEN)
            {
                if(curTmpIndex == 0)//当前是tmp1
                {
                    fclose(fd_tmp_w);
                    fd_tmp_w = fopen(TMP_FILE_2,"w");
                    curTmpIndex = 1;
                }
                else
                {
                    fclose(fd_tmp_w);
                    fd_tmp_w = fopen(TMP_FILE_1,"w");
                    curTmpIndex = 0;
                }
                tmpFileLen = 0;
                if (fd_tmp_w != NULL)
                {
                    fwrite(buf, strlen(buf), 1, fd_tmp_w);
                }
                else
                {
                    printf("reopen tmp file error\n");
                }
            }
            else
            {
                fwrite(buf, strlen(buf), 1, fd_tmp_w);
            }

            tmpFileLen += curLen;
        }

        
        midLen = strlen(socketBuf) / 2;
        if (midLen * 2 > SOCKET_BUFFER_SIZE - 1024)
        {
            memcpy(socketBuf, socketBuf + midLen, midLen);
            memset(socketBuf + midLen, 0, sizeof(socketBuf) - midLen);
        }
        //写socket
        strcat(socketBuf, buf);

#ifdef HEART_CHECK
        pthread_mutex_lock(&netLogMutex);
#endif
        if (isSent2Socket)
        {
            //升级会umount /opt/custom,所以需要/opt/custom/cfg能访问，而netuart不能访问才取消发送
            if (access(GRD_LOG_NET, F_OK) != 0 && access(GRD_LOG_CFG, F_OK) == 0)
            {
                isSent2Socket = 0;                
                fdRet = -1;
                fdRerty = 0;
                isSendId = 1;
            }

            if (fdRet >= 0 && sockfd != -1)
            {
                if (isSendId)
                {
                    netLogHeader.len = sizeof(id);
                    netLogHeader.type = 0;
                    write(sockfd, &netLogHeader, headerLen);
                    wsize = write(sockfd, id, sizeof(id));
                    isSendId = 0;
                }
                netLogHeader.len = strlen(socketBuf);
                netLogHeader.type = 1;
                write(sockfd, &netLogHeader, headerLen);
                wsize = write(sockfd, socketBuf, netLogHeader.len);
                
                //printf("-->write len:%d, ret:%d\n", strlen(socketBuf), wsize);
                if (wsize != strlen(socketBuf))
                {
                    printf("write to socket error,%d,ret:%d, retry:%d\n", netLogHeader.len, wsize, fdRerty);
                }

                if (wsize > 0)
                {
                    fdRerty = 0;
                    memset(socketBuf, 0, netLogHeader.len);
#ifdef HEART_CHECK
                    if (socketBufHeartLen + netLogHeader.len < SOCKET_BUFFER_SIZE)
                    {
                        strcat(socketBufHeart, socketBuf);
                        socketBufHeartLen += netLogHeader.len;
                    }
#endif
                }


                if (wsize == -1)
                {
                    //printf("send socket log ret:%d, retry:%d\n", wsize, fdRerty);
                    if (fdRerty > 2)
                    {
                        fdRet = -1;
                        close(sockfd);
                        sockfd = -1;
                        //sockfd = socket(AF_INET,SOCK_STREAM,0);
                        isSendId = 1;
                    }
                }

#ifdef HEART_CHECK
                //检查心跳 
                //printf("netLogHeartTime:%d, cur:%d\n", netLogHeartTime, getSysSec());
                if (netLogHeartTime != 0 && isLogTimeout == 0 && sockfd != -1 
                    && getSysSec() - HEART_TIMEOUT > netLogHeartTime)
                {
                    //print2Fifo("readfifo heart timeout...\n");
                    printf("readfifo heart timeout...\n");
                    fdRet = -1;
                    close(sockfd);
                    sockfd = -1;
                    isSendId = 1;
                    isLogTimeout = 1;
                    netLogHeartTime = getSysSec();
                    strncat(socketBuf, socketBufHeart, SOCKET_BUFFER_SIZE - strlen(socketBuf));
                    memset(socketBufHeart, socketBufHeartLen, 0);
                }
#endif
            }

            fdRerty++;
        }
        else
        {
            if (fdRerty > 30 && isSent2Socket == 0)
            {
                if (access(GRD_LOG_NET, F_OK) == 0)
                {
                    isSent2Socket = 1;
                    getId(id);
                    if (cmdThreadIsInited == 0)
                    {
                        cmdThreadIsInited = 1;
                        pthread_create(&recv_thread, NULL, readCmd, (void*)sockfd);
                    }
                }
                fdRerty = 0;
            }
            fdRerty++;
        }
#ifdef HEART_CHECK
        pthread_mutex_unlock(&netLogMutex);
#endif

        strcat(fileBuf, buf);
        //写SD卡文件
        sdbufLen = strlen(fileBuf);
        if (sdbufLen > 12000 && sdOk)
        {
            if ((access(GRD_LOG_SD, F_OK) != 0 && access(GRD_LOG_CFG, F_OK) == 0) || grd_sd_is_mount() != 1)
            {
                sdOk = 0;
            }

            if (sdOk)
            {
                fd_w = fopen(file_des,"at+");
                if(fd_w == NULL)
                {
                    printf("RRRopen %s, err \n",file_des);
                }
            }

            if (fd_w != NULL)
            {
                if (!yearOk)
                {
                    ts = time(NULL);
                    ptm = localtime_r(&ts, &tt);
                    if (ptm->tm_year != 70)
                    {
                        yearOk = 1;
                        fclose(fd_w);
                        strcpy(file_rename, GRD_LOG_DIR);
                        strcat(file_rename, "/");
                        get_nowtime_str(strlen(file_rename) + file_rename);
                        strcat(file_rename, ".txt");
                        printf("sd file %s \n", file_rename);
                        printf("-------->remame from %s to %s\n", file_des, file_rename);
                        rename(file_des, file_rename);

                        fd_w = fopen(file_rename,"at+");
                        if(fd_w == NULL)
                        {
                            printf("RRRopen %s, err \n",file_des);
                        }
                        strcpy(file_des, file_rename);
                    }
                }

                if (fd_w != NULL)
                {
                    fwrite(fileBuf, strlen(fileBuf), 1, fd_w);
                }
                sdFileLen += strlen(fileBuf);
                fclose(fd_w);
                fd_w = NULL;
            }

            memset(fileBuf, 0, sizeof(fileBuf));
            if (sdFileLen > 1024 * 1024)
            {
                sdFileLen = 0;
                if (sdOk)
                {
                    strcpy(file_des, GRD_LOG_DIR);
                    strcat(file_des, "/");
                    get_nowtime_str(strlen(file_des) + file_des);
                    strcat(file_des, ".txt");
                    printf("sd file %s \n", file_des);
                }
            }
        }
        else
        {
            if (sdbufLen > 12000)
            {
                memset(fileBuf, 0, sizeof(fileBuf));
            }

            if (sdRerty > 30 && sdOk == 0)
            {
                if (access(GRD_LOG_SD, F_OK) == 0 && grd_sd_is_mount() == 1)
                {
                    if (access(GRD_SD_MOUNT_POINT, F_OK) == 0)
                    {
                        sdOk = 1;
                        if (access(GRD_LOG_DIR, F_OK) != 0)
                        {
                            if((mkdir(GRD_LOG_DIR, 0777)) < 0)
                            {
                                printf("mkdir %s failed\n", GRD_LOG_DIR);
                                sdOk = 0;
                            }
                        }
                        if (sdOk)
                        {
                            strcpy(file_des, GRD_LOG_DIR);
                            strcat(file_des, "/");
                            get_nowtime_str(strlen(file_des) + file_des);
                            strcat(file_des, ".txt");
                            printf("sd file %s \n", file_des);
                        }
                    }
                }
                sdRerty = 0;
            }
            sdRerty++;
        }
    }

    if (fd_w != NULL)
    {
        fwrite(fileBuf, strlen(fileBuf), 1, fd_w);
        fflush(fd_w);
        fsync(fileno(fd_w));
        fclose(fd_w);
    }
    fclose(fd_r);

    printf("RRR read fifo ok \n");
    return 0;
}




