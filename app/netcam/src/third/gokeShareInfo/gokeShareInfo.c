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
#include <sys/ipc.h>
#include <sys/shm.h>
#include <stdio.h>

#include "cJSON.h"
#include "netcam_api.h"
#include "gokeShareInfo.h"
#include "mmc_api.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "flash_ctl.h"
#include "utility/util_sock.h"


static char hostAddr[] = "tx.lpwei.com";
static int gokeShareInfoId = -1;
static GOKE_SHARE_INFO *gokeShareInfo = NULL;

static char hostIp[20] = {0};
static int udpInfoPort = 50700;
static int gokeFreshNow = 0;



int goke_share_fresh_info(void)
{
    if (gokeShareInfo)
    {
        sprintf(gokeShareInfo->extralData, "&sd=%d,%d,%d-%d,%d-%d,%d-%d-%d", grd_sd_check_device_node(), 
            mmc_get_sdcard_stauts(), grd_sd_get_free_size_last(),
            grd_sd_get_all_size_last(), runRecordCfg.enable, runRecordCfg.recordMode,
            runMdCfg.enable, runMdCfg.sensitive, runMdCfg.handle.is_rec);
        gokeShareInfo->freshNow = 1;
    }

    return 0;
}


void * goke_share_check(void *arg)
{
	int ret;
    int sleepTime = 0;
    int maxSleepTime = 7200;
    int sdLastStatus = 0;

    pthread_detach(pthread_self());
    sdLastStatus = mmc_get_sdcard_stauts();
    while(1)
    {
        ret = goke_share_fresh_info();
        sleepTime = 0;
        gokeFreshNow = 0;
        if (ret == 0)
        {
            while (sleepTime < maxSleepTime && gokeFreshNow == 0)
            {
                sleep(20);
                sleepTime += 20;
                if (gokeShareInfo != NULL && gokeShareInfo->cmdType != 0)
                {
                    printf("goke_share_check cmd:%d\n", gokeShareInfo->cmdType);
                    switch(gokeShareInfo->cmdType)
                    {
                        case GOKE_CMD_FORMAT:
                        {
                            grd_sd_format();
                            break;
                        }
                        default:
                            break;
                    }
                    gokeShareInfo->cmdType = 0;
                }
            }
        }
        else
        {
            sleep(20);
        }
    }
}

static void goke_share_info_init(void)
{
    void* shm = NULL;
    if (gokeShareInfoId != -1)
    {
        return 0;
    }
    gokeShareInfoId = shmget((key_t)1234,sizeof(GOKE_SHARE_INFO),0666|IPC_CREAT);   //创建共享内存
    if(gokeShareInfoId == -1)
    {
        printf("shmget error:%s\n", strerror(errno));
    }
    else
    {
        printf("gokeShareInfoId = %d\n",gokeShareInfoId);
    }

    shm = shmat(gokeShareInfoId,(void*)0,0);                        //映射共享内存
    if(shm == (void*)(-1))
    {
        printf("shmat error\n");
        gokeShareInfo = NULL;
    }
    else
    {
        gokeShareInfo = (GOKE_SHARE_INFO*)shm;
    }
    return 0;
}


int goke_share_start(void)
{
    pthread_t thread_id;

    goke_share_info_init();
    if(0 != pthread_create(&thread_id, NULL, goke_share_check, NULL))
    {
        printf("thread error\n");
        return -1;
    }

}

int goke_share_fresh_now(void)
{
    printf("goke_share_fresh_now\n");
    gokeFreshNow = 1;
}

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
int goke_share_udp_info(int type, char *contentStr)
{
    int ret;
    int socket_fd;
    char *out;
    cJSON *root;
    char jsonStr[1024] = {0};

	struct sockaddr_in servaddr;
    struct timespec times = {0, 0};
    unsigned long time;
	struct in_addr server;

    clock_gettime(CLOCK_MONOTONIC, &times);
    time = times.tv_sec;

    if (strlen(hostIp) == 0)
    {
        ret = utility_net_resolve_host(&server,hostAddr);
        if (ret == 0)
        {
            strncpy(hostIp, inet_ntoa(server), 16);
        }
        else
        {
            printf("udp log get host ip error.\n");
            return -1;
        }
    }

    root = cJSON_CreateObject();//创建项目

    cJSON_AddStringToObject(root, "devid", runSystemCfg.deviceInfo.serialNumber);
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
