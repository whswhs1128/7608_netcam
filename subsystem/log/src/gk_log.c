#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
//#include "common.h"
//#include "cfg_all.h"
#include "gk_log.h"
char deviceName[64] = "ipc";

FILE*  log_open()
{
    char path[100];

    //目录不存在，则创建
    if(access(LOG_DIR, F_OK) != 0 ) {
        PRINT_INFO("to create log dir: %s\n", LOG_DIR);
        if((mkdir(LOG_DIR, 0777)) < 0)
        {
            PRINT_ERR("mkdir %s failed\n", LOG_DIR);
            return NULL;
        }            
    } 

    memset(path, 0, sizeof(path));
    sprintf(path, "%s%s", LOG_DIR, LOG_FILE);
    PRINT_INFO("to load %s\n", path);

    FILE* fp = fopen(path, "rb+");
    if (fp == NULL) {
        fp = fopen(path, "wb+");
        if (fp == NULL) {
            PRINT_ERR("open %s failed\n", path);    
            return NULL;
        }
    }

    return fp;
}


int log_write(char *msg)
{
    FILE* fp = log_open();
    if ((fp == NULL) || (msg == NULL)) {
        PRINT_ERR("param error.\n");
        return -1;
    }
    
    if (0 != fseek(fp, 0, SEEK_END)) {
        PRINT_ERR();
        fclose(fp);
        return -1;
    }


    char startTime[128] = {0};

    long ts = time(NULL); 
    struct tm tt = {0}; 
    struct tm *pTm = localtime_r(&ts, &tt);

    sprintf(startTime,"%d-%02d-%02d %02d:%02d:%02d",pTm->tm_year+1900,pTm->tm_mon+1,pTm->tm_mday,pTm->tm_hour,pTm->tm_min,pTm->tm_sec);

    
    char log[1024] = {0};
    sprintf(log, "[%s] [%s] %s\\n", deviceName, startTime, msg);
    printf("log: %s\n", log);
    int len = strlen(log);
    if (fwrite(log, 1, len, fp) != len) {
        PRINT_ERR();
        fclose(fp);
        return -1;
    }

    fclose(fp);

    return 0;
}


