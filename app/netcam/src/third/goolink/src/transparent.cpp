#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <memory.h>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <csl_sem.h>
#include <cJSON.h>
#include <goolink_sesapi.h>
#include <goolink_apiv2.h>
#include "transparent.h"
#include "debug.h"
#include "goolink_cc_switch.h"
#include "netcam_api.h"

extern char s_SesBufSendOut[SES_BUF_SIZE];

unsigned char AF_get_maxzoom();
unsigned char AF_inquery(unsigned char u8Cmd, int timeoutms); //from hisdk
void AF_register_af_notfiy(FUNCNOTIFY pfunc, int item);

static int model_id = -1;
static char model_filename[32] = {0};
static int model_filesize = 0;
#define FILE_TMP_ALERT    "/tmp/alert.wav" //先下载到tmp目录, 最后move过去
#define FILE_FINAL_ALERT  "/opt/custom/audio/alert.wav"//"/etc/Wireless/alert.wav" //最终需要覆盖的文件
#define SIZE_WAVE_FILEHEADER 0x1000
#define MAX_FILESIZE (300 * 1024) //目前设定不能超过300K

int mysystem(const char *cmdstringl)
{
	printf(" %s:%d \n",__func__,__LINE__);
	new_system_call(cmdstringl);
}

int32_t DownLoadAlertFile(char *filename,
                        int type,
                        int size, //文件总长度
                        char *data,
                        int len)  //该次下发的长度
{
    GDEBUG(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> \n");
    int32_t ret = 0;
    static FILE *fd = NULL;
    static int count_size = 0;
    GDEBUG("%s, type = %d, size = %d, len = %d\n", filename, type, size, len);
    if(type == 1) //开始下载, 打开文件句柄
    {
        GDEBUG("begin 111111111111111 size = %d, filename=%s\n", size, filename);
        count_size = 0; //重新开始计数
        //check flash
        if( (fd = fopen(FILE_TMP_ALERT, "w+")) == NULL ){
            GDEBUG( "begin 111111222222, openfile failed !!! ret=0, [%s] error [%s]\n", filename,strerror(errno));
            ret = 0;
        }
        else{
            GDEBUG( "begin 111111333333333, success \n");
            ret = 1;
        }
    }
    else if(type == 2) //下载进度, 不断地写文件
    {
        GDEBUG("downloading 2222222222222 len=%d, count_size=%d, fd=%d, date=%p \n", len, count_size);
        if (count_size > MAX_FILESIZE){ //若发现文件超过大小则拒绝掉后面的内容
            //除非重新下载, count_size才能重新置为0, 否则此时就复位, 后面又开始累积
            GDEBUG("recived file = %d has been transfered over %d, skip later file segement!!", count_size, MAX_FILESIZE);
            if(fd != NULL){ //关掉临时文件
                fclose(fd);
                fd = 0;
                mysystem("mv "FILE_TMP_ALERT " "FILE_FINAL_ALERT); //前面传过来的文件还是接收
            }
            ret = 0; //失败拒绝
        }
        else{
            count_size += len; //累加下载的文件大小
            if(fd != NULL && data != NULL) //20180915 因为前面传的是带wav头部的PCM文件, 那边不好处理, 这里直接丢弃掉.
            {
                if(count_size > SIZE_WAVE_FILEHEADER){ //只有大于才写入
                    fwrite(data, len, 1, fd); //将文件写入到flash
                    fflush(fd);
                }
                ret = 1;
            }
            else{
                GDEBUG("downloading 22222222333333 NO write file, ret=0, len=%d, count_size=%d \n", len, count_size);
                ret = 0;
            }
        }
    }
    else if(type == 3) //下载完成
    {
        GDEBUG("completed successfully! 333333333333333333 len=%d, count_size=%d \n", len, count_size);
        if(fd != NULL){
            fclose(fd);
            fd = 0;
            mysystem("mv "FILE_TMP_ALERT " "FILE_FINAL_ALERT); //再重新命名
        }
        count_size = 0;
        ret = 1;
    }
    else if(type == 4)// 失败
    {
        GDEBUG("failed 4444444444444444444 len=%d, count_size=%d \n", len, count_size);
        if(fd != NULL){
            fclose(fd);
            fd = 0;
        }
        GDEBUG("下载失败\n");
        count_size = 0;
        ret = 1;

        //此时删掉文件
        GDEBUG("failed!! remove file: %s \n", FILE_TMP_ALERT);
        mysystem("rm "FILE_TMP_ALERT);
    }

    return ret;
}

int  download_file(char * bufrecv)
{
    TLV_V_UploadAlarmResponse *phead = (TLV_V_UploadAlarmResponse *)bufrecv;
    for (int i = 0; i<sizeof(TLV_V_UploadAlarmResponse); i++){
        if (i>0 && i%32 == 0){
            printf("\n");
        }
        if (i%4 == 0){
            printf("  ");
        }
        printf("%02x ", bufrecv[i]);
    }

    char *filename = (char *)FILE_TMP_ALERT;
    int ret = 0;
    int type   = phead->type;
    int size   = phead->size;
    char *data = bufrecv + sizeof(TLV_V_UploadAlarmResponse);
    int len    = phead->len;
    ret = DownLoadAlertFile(filename, type, size, data, len);


    TLV_V_DownloadAlarmResponse * pret =  (TLV_V_DownloadAlarmResponse * )s_SesBufSendOut; //返回内存
    strncpy(pret->szToken, DOWNLOADFILE, 16);
    pret->result = ret;
    pret->reserve = 0;

    GDEBUG("return : ret=%d\n", ret);
    return sizeof(TLV_V_DownloadAlarmResponse);
}
