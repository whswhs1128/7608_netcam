//
// Created by 汪洋 on 2019-09-06.
//

#include "devsdk.h"
#include "protool.h"
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <stdio.h>
#include <stdlib.h>
#include "cJSON.h"
#include "common.h"
#include "sdk_def.h"
#include "cfg_system.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <linux/if_packet.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <pthread.h>
#include <unistd.h>

static char g_dev_sn[128];
char devVer[64] = {0};
extern char softVersion[32];
PLUG_CONFIG local_cfg = {0};

//extern PLUG_CONFIG local_cfg;

U16 get_dev_mac(U8 *mac){
    //获取MAC的功能厂商自己实现，本函数仅供参考
	char device[16]="eth0"; //teh0是网卡设备名
    unsigned char macaddr[ETH_ALEN]; //ETH_ALEN（6）是MAC地址长度
    //AF_INET = 1;
    int i,s;
    s = socket(AF_INET,SOCK_DGRAM,0); //建立套接口
    struct ifreq req;
    int err;
    char *ret;

    ret = strcpy(req.ifr_name,device); //将设备名作为输入参数传入
    err=ioctl(s,SIOCGIFHWADDR,&req); //执行取MAC地址操作
    close(s);
    if( err != -1 )
    {
        memcpy(macaddr,req.ifr_hwaddr.sa_data,ETH_ALEN); //取输出的MAC地址
        printf("########MAC= \n");
        for(i=0;i<ETH_ALEN;i++)
            printf("%02X:",macaddr[i]);
        memcpy(mac, macaddr, ETH_ALEN);
    }
    printf("\r\n");
    return 0;
}

U16 get_dev_sn(char * sn)
{
    //获取SN号的功能厂商自己实现，本函数仅供参考

    if (sn == NULL){
        return -1;
    }
    U8 mac[6];
    memset(mac, 0, sizeof(mac));
    get_dev_mac(mac);
    sprintf(sn, "%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX|%s", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5], g_dev_sn);
    return 0;
};

U16 set_dev_sn(char * sn, int len)
{
    if ((sn == NULL) || (len == 0)){
        return -1;
    }
    memset(g_dev_sn, 0, sizeof(g_dev_sn));
    strncpy(g_dev_sn, sn, sizeof(g_dev_sn)-1);
    return 0;
};

U16 get_dev_version(char * version){
    //获取固件版本信息的功能厂商自己实现，本函数仅供参考

    if (version == NULL){
        return -1;
    }
    //char test_version[256] = "V0.0.1";
    if (strlen(devVer) == 0)
    {
        sprintf(version, "%s", softVersion);
    }
    else
    {
        strcpy(version, devVer);
    }
    return 0;
}

//发送固件版本信息
int dev_version_report()
{
    int ret = 0;

    char dev_version[256] = {0};

    data_device_mt_req report_msg;

    report_msg.dataType = datatype_version_report;

    cJSON * pJsonRoot = NULL;

    pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot)
    {
        //error happend here
        return -1;
    }

    ret = get_dev_version(dev_version);

    cJSON_AddStringToObject(pJsonRoot, "version", dev_version);

    char * pJson = cJSON_Print(pJsonRoot);
    if(NULL == pJson)
    {
        cJSON_Delete(pJsonRoot);
        return -2;
    }
    report_msg.dataByte = pJson;
    report_msg.length = strlen(report_msg.dataByte);

    printf("######## %s %d: len = [%d] pJson = \n%s\n\n", __func__, __LINE__, report_msg.length, report_msg.dataByte);

    send_management_up_data((void *)&report_msg, sizeof(report_msg)+report_msg.length);

    cJSON_Delete(pJsonRoot);

    return 0;
}

/*int dev_fwversion_report()
{
    int ret = 0;

    char dev_version[256] = {0};

    data_device_mt_req report_msg;

    report_msg.dataType = 14;

    cJSON * pJsonRoot = NULL;

    pJsonRoot = cJSON_CreateObject();
    if(NULL == pJsonRoot)
    {
        //error happend here
        return -1;
    }

//	ret = get_dev_version(dev_version);
    memset(dev_version,0,sizeof(dev_version));
    memcpy(dev_version,"FW-V0.0.2",9);

    cJSON_AddStringToObject(pJsonRoot, "version", dev_version);

    char * pJson = cJSON_Print(pJsonRoot);
    if(NULL == pJson)
    {
        cJSON_Delete(pJsonRoot);
        return -2;
    }
    report_msg.dataByte = pJson;
    report_msg.length = strlen(report_msg.dataByte);

    printf("######## %s %d: len = [%d] pJson = \n%s\n\n", __func__, __LINE__, report_msg.length, report_msg.dataByte);

    send_management_up_data((void *)&report_msg, sizeof(report_msg)+report_msg.length);

    cJSON_Delete(pJsonRoot);

    return 0;
}*/

/// -------------设备---------------
// *设备数据下行*

// data：数据内容json字符串
// data_len：数据长度
///-- 备注：在不需要应答的情况下，response只需要回复下行数据中的sequence内容即可 --

// 举例：
//     下行数据data内容为：{"sequence":65535,"SongControlS":2}
//     应答数据response内容为：{"sequence":65535,"xxx1":"",......}，其中应答内容出了sequence外，根据具体情景可以包含多个参数
//     ---其中应答数据中的sequence为下行数据中的sequence的值
int dev_data_down(const char *data, int data_len){
    printf("######## data=%s len=%d \n", data, data_len);
    printf("######## %s OK. \n", __func__);
    int ret = -1;

    if(NULL == data)
    {

        return ret;
    }
    /// 可用一下方法处理数据，也可自行处理数据，供参考
    /*cJSON * pJsonData = NULL;
    cJSON * pJsonRes = NULL;
    pJsonData = cJSON_Parse(data);
    pJsonRes = cJSON_CreateObject(); //cJSON_Parse("{\"sequence\":0}");

    for(int i=0; i<cJSON_GetArraySize(pJsonData); i++)   //遍历最外层json键值对
    {
        cJSON * item = cJSON_GetArrayItem(pJsonData, i);

        *//* -- 打印json键值对 --
        if(cJSON_String == item->type)
        {
            printf("%s->", item->string);
            printf("%s\n", item->valuestring);
        }
        else if(cJSON_Number == item->type)
        {
            printf("%s->", item->string);
            printf("%s\n", item->valueint);
        }*//*

        if(!strcasecmp("sequence",item->string))
        {
            cJSON_AddNumberToObject(pJsonRes, item->string, item->valueint);
            ret = 0;
        }
        else if(!strcasecmp("xxx",item->string)) // example
        {
            //...

        }
        //...
    }

    /// 应答数据的添加举例
    // 值为字符串的处理方式
    //    cJSON_AddStringToObject(pJsonRes, string, string);
    // 值为数值的处理方式
    //    cJSON_AddNumberToObject(pJsonRes, string, int);


    if(-1 == ret)
    {
        cJSON_AddStringToObject(pJsonRes, "msg", "parser error");
    }

    char * pJson = cJSON_PrintUnformatted(pJsonRes);
    *response_len = strlen(pJson);
    memcpy(response, pJson, *response_len);

    cJSON_Delete(pJsonRes);
    cJSON_Delete(pJsonData);*/

    /*char response[1024*4] = "";
    int  responselen = 0;
    memcpy(response, data, data_len);
    responselen = data_len;
    send_business_ack_data(0, (unsigned char*) &response, responselen);
    send_business_up_data((unsigned char*) &response, responselen);*/

    return ret;
}
