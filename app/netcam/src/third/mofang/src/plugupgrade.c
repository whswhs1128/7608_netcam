
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "common.h"
#include "plugupgrade.h"
#include "cJSON.h"
#include "product.h"
#include "protool.h"
#include "Md5.h"
#include "download.h"

char mPluginID[128];
char mFromVersion[128];
char mToVersion[128];
U8   mMac[6];
char mToken[64];
char mFileMD5[64];
int  mFileLength;
char mDownloadUrl[128];
char mDownloadServer[32];
int  mProcess;
int  mDownloadPort;
char mRecogResult;


int gw_parse_upgrade_cmd(char * pMsg){
    cJSON * pSub;
    if(NULL == pMsg)
    {
        return -1;
    }
    cJSON * pJson = cJSON_Parse(pMsg);
    if(NULL == pJson)                                                                                         
    {
       cJSON_Delete(pJson);
      return -2;
    }
    
    pSub = cJSON_GetObjectItem(pJson, "pluginID");
    if(NULL == pSub)
    {
       cJSON_Delete(pJson);
       return -3;
    }
    printf("########pluginID : %s\n", pSub->valuestring);
    
    strcpy(mPluginID, pSub->valuestring);
    
    pSub = cJSON_GetObjectItem(pJson, "fromVersion");
    if(NULL == pSub)
    {
        cJSON_Delete(pJson);
        return -4;
    }
    printf("########fromVersion : %s\n", pSub->valuestring);
    strcpy(mFromVersion, pSub->valuestring);
    
    pSub = cJSON_GetObjectItem(pJson, "toVersion");
    if(NULL == pSub)
    {
        cJSON_Delete(pJson);
        return -5;
    }
    printf("########toVersion : %s\n", pSub->valuestring);
    strcpy(mToVersion, pSub->valuestring);

    cJSON_Delete(pJson);
    return 0;
};

int gw_parse_upgrade_param(char * pMsg){
    cJSON * pSub;
    if(NULL == pMsg)
    {
        return -1;
    }
    cJSON * pJson = cJSON_Parse(pMsg);
    if(NULL == pJson)                                                                                         
    {
       cJSON_Delete(pJson);
      return -2;
    }
    pSub = cJSON_GetObjectItem(pJson, "pluginID");
    if(NULL == pSub)
    {
       cJSON_Delete(pJson);
       return -3;
    }
    printf("########pluginID : %s\n", pSub->valuestring);
    
    strcpy(mPluginID, pSub->valuestring);
    
    pSub = cJSON_GetObjectItem(pJson, "fromVersion");
    if(NULL == pSub)
    {
        cJSON_Delete(pJson);
        return -4;
    }
    printf("########fromVersion : %s\n", pSub->valuestring);
    strcpy(mFromVersion, pSub->valuestring);
    
    pSub = cJSON_GetObjectItem(pJson, "toVersion");
    if(NULL == pSub)
    {
        cJSON_Delete(pJson);
        return -5;
    }
    printf("########toVersion : %s\n", pSub->valuestring);
    strcpy(mToVersion, pSub->valuestring);
    
    //此处最好分解一下，serverip，path
    
    pSub = cJSON_GetObjectItem(pJson, "url");
    if(NULL == pSub)
    {
        cJSON_Delete(pJson);
        return -6;
    }
    char mDownloadUrl2[128];
    memset(mDownloadUrl2,0,128);
    printf("########url : %s\n", pSub->valuestring);
    strcpy(mDownloadUrl2, pSub->valuestring);
    //解析出downloadsever&path
    //http://171.208.222.109:6030/File/pluginFile/602aee54-35b4-4f32-91b3-33b448a538ce
    char* p2;char* p3;int serverlen=0;
    char* p1 = strstr(mDownloadUrl2, "://");
    
    if(p1)
    {
                            
        p2 = strstr(p1+3, ":");
        if(p2)
        {
            serverlen =p2-p1-3;
            p3=strstr(p2+1, "/");
            if(p3)
            {
                char port[8];
                int portlen = p3-p2-1;
                printf("########portlen %d\n",portlen);
                if(portlen<8)
                {
                    memset(port,0,8);
                    memcpy(port,p2+1,portlen);
                    mDownloadPort = atoi(port);
                }
                else
                {
                    cJSON_Delete(pJson);
                    return -7;
                }
                
                memcpy(mDownloadUrl,p3,strlen(p3));
            }
            else
            {
                cJSON_Delete(pJson);
                return -8;
            }
        }
        else
        {
            mDownloadPort =80;
            p3=strstr(p1+3, "/");
            if(p3)
            {
                serverlen =p3-p1-3;
                memcpy(mDownloadUrl,p2,strlen(p3));
            }
            else
            {
                cJSON_Delete(pJson);
                return -9;
            }
        }
            
    }
    else
    {
        cJSON_Delete(pJson);
        return -10;
    }
    memset(mDownloadServer,0,sizeof(mDownloadServer));
    
    
    
    
    printf("########server len  %d\n",serverlen);
    //serverlen =20;
    if(serverlen<32)
        memcpy(mDownloadServer,p1+3,serverlen);
    else
    {
        cJSON_Delete(pJson);
        return -11;
    }
                
    
    printf("########port %d;mDownloadUrl : %s; mDownloadServer: %s \n", mDownloadPort,mDownloadUrl,mDownloadServer);
    //strcpy(mUrl,pSub->valuestring);
    pSub = cJSON_GetObjectItem(pJson, "fileLength");
    if(NULL == pSub)
    {
        cJSON_Delete(pJson);
        return -12;
    }
    printf("########fileLength : %d\n", pSub->valueint);
    mFileLength =pSub->valueint;
    pSub = cJSON_GetObjectItem(pJson, "fileMD5");
    if(NULL == pSub)
    {
        cJSON_Delete(pJson);
        return -13;
    }
    
    strcpy(mFileMD5, pSub->valuestring);
    printf("########fileMD5 : %s\n", mFileMD5);
    
    pSub = cJSON_GetObjectItem(pJson, "token");
    if(NULL == pSub)
    {
        cJSON_Delete(pJson);
        return -14;
    }
    printf("########token : %s\n", pSub->valuestring);
    strcpy(mToken, pSub->valuestring);
    
    cJSON_Delete(pJson);
    return 0;
};

int gw_parse_do_upgrade(void){

	int ret =0;
    char senddata[256];
    printf("-----------md5: down :%s\n", mFileMD5);
    
    if(ret ==0)
    {
        printf("plug update url :%s\n",mDownloadUrl);
        ret = DownLoadFIle(mDownloadServer, mDownloadPort,mDownloadUrl,(char*)PLUG_DOWNLOAD_FILENAME);
    }	
    
    data_device_mo_req gwdev_msg;

    gwdev_msg.dataType = datatype_upgrade_process;

    if(ret ==0)
    {
        //检测MD5
        printf("1-----------md5: down :%s\n", mFileMD5);
        const char *file_path = PLUG_DOWNLOAD_FILENAME;
        char md5_str[MD5_STR_LEN + 1] = "";
        ret = compute_file_md5(file_path, md5_str);
        printf("md5: self:%s===down :%s\n", md5_str,mFileMD5);
        if (strcmp(md5_str,mFileMD5)==0)
        {
            //printf("[file - %s] md5 value:\n", file_path);

            LogPlugStatus(log_update);

            mProcess = 1;
            char* pmsg=(char*)"";
   			sprintf(senddata,"{\"pluginID\":\"%s\",\"fromVersion\":\"%s\",\"toVersion\":\"%s\",\"process\":%d,\"result\":%d,\"resultDesc\":\"%s\"}",
				mPluginID,mFromVersion,mToVersion,mProcess,1,pmsg);
            
            gwdev_msg.length = strlen(senddata);
            gwdev_msg.dataByte = senddata;
            send_management_up_data((void *)&gwdev_msg, 3 + gwdev_msg.length);
            sleep(3);
            printf("########exit at [%s] %d \n\n", __func__, __LINE__);
            exit(-1);
        }
        else
        {
            char* pmsg=(char*)"";
   			sprintf(senddata,"{\"pluginID\":\"%s\",\"fromVersion\":\"%s\",\"toVersion\":\"%s\",\"process\":%d,\"result\":%d,\"resultDesc\":\"%s\"}",
				mPluginID,mFromVersion,mToVersion,mProcess,0,pmsg);
            send_management_up_data((void *)senddata, sizeof(gwdev_msg)+gwdev_msg.length);

            LogPlugStatus(log_update_err);

        }

    }
    else
    {
        char* pmsg=(char*)"";
        sprintf(senddata,"{\"pluginID\":\"%s\",\"fromVersion\":\"%s\",\"toVersion\":\"%s\",\"process\":%d,\"result\":%d,\"resultDesc\":\"%s\"}",
            mPluginID,mFromVersion,mToVersion,mProcess,0,pmsg);
        send_management_up_data((void *)senddata, sizeof(gwdev_msg)+gwdev_msg.length);

        LogPlugStatus(log_update_err);
    }
}

