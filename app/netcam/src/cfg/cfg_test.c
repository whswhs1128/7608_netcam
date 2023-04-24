/*!
*****************************************************************************
** FileName     : cfg_test.c
**
** Description  : test demo of config api.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-8-1
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include "cfg_audio.h"
#include "cfg_image.h"
#include "cfg_video.h"
#include "cfg_system.h"
#include "cfg_network.h"
#include "cfg_channel.h"
#include "cfg_md.h"
#include "cfg_snap.h"
#include "cfg_record.h"
#include "cfg_alarm.h"
#include "cfg_ptz.h"
#include "cfg_user.h"

void AudioCfgTest()
{
    AudioCfgLoad();
    AudioCfgPrint();

    runAudioCfg.chans = 2;
    runAudioCfg.sampleRate = 9000;
    runAudioCfg.inputVolume = 30;
    AudioCfgPrint();

    AudioCfgSave();

    runAudioCfg.chans = 1;
    runAudioCfg.sampleRate = 3200;
    runAudioCfg.inputVolume = 70;
    AudioCfgPrint();

    AudioCfgLoad();
    AudioCfgPrint();

    AudioCfgLoadDefValue();
    AudioCfgPrint();

}

void ImageCfgTest()
{
    ImageCfgLoad();
    ImageCfgPrint();

    runImageCfg.backLightEnable = 0;
    runImageCfg.backLightLevel = 30;
    runImageCfg.hue = -5;
    ImageCfgPrint();

    ImageCfgSave();

    runImageCfg.backLightEnable = 1;
    runImageCfg.backLightLevel = 70;
    runImageCfg.hue = -10;
    ImageCfgPrint();

    ImageCfgLoad();
    ImageCfgPrint();

    ImageCfgLoadDefValue();
    ImageCfgPrint();

}

//创建数组，数组值是另一个JSON的item，这里使用数字作为演示
char * makeArray(int iSize)
{
    cJSON * root =  cJSON_CreateArray();
    if(NULL == root)
    {
        printf("create json array faild\n");
        return NULL;
    }
    int i = 0;

    char tmp[10];
    for(i = 0; i < iSize; i++)
    {
        sprintf(tmp, "%d", i);
        //cJSON_AddNumberToObject(root, "hehe", i);
        cJSON_AddItemToArray(root, cJSON_CreateString(tmp));
        cJSON_AddItemReferenceToArray(root, cJSON_CreateString("ref"));
        cJSON_AddItemReferenceToObject(root, "test",cJSON_CreateString("ref2"));
    }
    char * out = cJSON_Print(root);
    cJSON_Delete(root);

    return out;
}

//解析刚刚的CJSON数组
void parseArray(char * pJson)
{
    if(NULL == pJson)
    {
        return ;
    }
    cJSON * root = NULL;
    if((root = cJSON_Parse(pJson)) == NULL)
    {
        return ;
    }
    int iSize = cJSON_GetArraySize(root);
    int iCnt = 0;
    for(iCnt = 0; iCnt < iSize; iCnt++)
    {
        cJSON * pSub = cJSON_GetArrayItem(root, iCnt);
        if(NULL == pSub)
        {
            continue;
        }
        int iValue = pSub->valueint;
        printf("value[%2d] : [%d]\n", iCnt, iValue);
    }
    cJSON_Delete(root);
    return;
}


void CjsonArrayTest()
{
    char *out = makeArray(5);
    printf("%s\n", out);

    parseArray(out);
}


void VideoCfgTest()
{
    VideoCfgLoad();
    VideoCfgPrint();

    runVideoCfg.vencStream[0].h264Conf.height = 100;
    VideoCfgPrint();

    VideoCfgSave();

    runVideoCfg.vencStream[0].h264Conf.height = 300;
    VideoCfgPrint();

    VideoCfgLoad();
    VideoCfgPrint();

    VideoCfgLoadDefValue();
    VideoCfgPrint();
}

void SystemCfgTest()
{
    SystemCfgLoad();
    SystemCfgPrint();

    sprintf(runSystemCfg.deviceInfo.deviceName, "ipc %d", 1);
    runSystemCfg.timezoneCfg.timezone = 480;
    runSystemCfg.netDstCfg.enableDST = 1;
    strcpy(runSystemCfg.ntpCfg.serverDomain, "ntpserver1.cn");
    SystemCfgPrint();

    SystemCfgSave();

    sprintf(runSystemCfg.deviceInfo.deviceName, "ipc %d", 2);
    runSystemCfg.timezoneCfg.timezone = 540;
    runSystemCfg.netDstCfg.enableDST = 0;
    strcpy(runSystemCfg.ntpCfg.serverDomain, "ntpserver2.cn");
    SystemCfgPrint();

    SystemCfgLoad();
    SystemCfgPrint();

    SystemCfgLoadDefValue();
    SystemCfgPrint();
}

void NetworkCfgTest()
{
    NetworkCfgLoad();
    NetworkCfgPrint();

    strcpy(runNetworkCfg.lan.netName, "eth1");
    strcpy(runNetworkCfg.ftp.address, "ftp2.cn");
    runNetworkCfg.port.httpPort = 8080;
    NetworkCfgPrint();

    NetworkCfgSave();

    strcpy(runNetworkCfg.lan.netName, "eth3");
    strcpy(runNetworkCfg.ftp.address, "ftp3.cn");
    runNetworkCfg.port.httpPort = 80801;
    NetworkCfgPrint();

    NetworkCfgLoad();
    NetworkCfgPrint();

    NetworkCfgLoadDefValue();
    NetworkCfgPrint();
}

void ChannelCfgTest()
{
    ChannelCfgLoad();
    ChannelCfgPrint();

    strcpy(runChannelCfg.channelInfo[0].osdChannelName.text, "channel01");
    ChannelCfgPrint();

    ChannelCfgSave();

    strcpy(runChannelCfg.channelInfo[0].osdChannelName.text, "ch01");
    ChannelCfgPrint();

    ChannelCfgLoad();
    ChannelCfgPrint();

    ChannelCfgLoadDefValue();
    ChannelCfgPrint();
}


void MdCfgTest()
{
    MdCfgLoad();
    MdCfgPrint();

#if 1
    runMdCfg.mdRegion[0].width = 100;
    runMdCfg.scheduleTime[0][0].startHour = 1;
    runMdCfg.scheduleTime[0][0].startMin = 2;
    runMdCfg.scheduleTime[0][0].stopHour = 23;
    runMdCfg.scheduleTime[0][0].stopMin = 3;

    runMdCfg.scheduleTime[6][3].startHour = 1;
    runMdCfg.scheduleTime[6][3].startMin = 2;
    runMdCfg.scheduleTime[6][3].stopHour = 23;
    runMdCfg.scheduleTime[6][3].stopMin = 3;
    MdCfgPrint();

    MdCfgSave();

    runMdCfg.mdRegion[0].width = 200;
    runMdCfg.scheduleTime[0][0].startHour = 4;
    runMdCfg.scheduleTime[0][0].startMin = 5;
    runMdCfg.scheduleTime[0][0].stopHour = 22;
    runMdCfg.scheduleTime[0][0].stopMin = 6;
    runMdCfg.scheduleTime[6][3].startHour = 4;
    runMdCfg.scheduleTime[6][3].startMin = 5;
    runMdCfg.scheduleTime[6][3].stopHour = 21;
    runMdCfg.scheduleTime[6][3].stopMin = 6;
    MdCfgPrint();

    MdCfgLoad();
    MdCfgPrint();

    MdCfgLoadDefValue();
    MdCfgPrint();
#endif
}

void SnapCfgTest()
{
    SnapCfgLoad();
    SnapCfgPrint();

#if 0
    runSnapCfg.scheduleTime[0][0].startHour = 1;
    runSnapCfg.scheduleTime[0][0].startMin = 2;
    runSnapCfg.scheduleTime[0][0].stopHour = 23;
    runSnapCfg.scheduleTime[0][0].stopMin = 3;

    runSnapCfg.scheduleTime[6][3].startHour = 1;
    runSnapCfg.scheduleTime[6][3].startMin = 2;
    runSnapCfg.scheduleTime[6][3].stopHour = 23;
    runSnapCfg.scheduleTime[6][3].stopMin = 3;
    SnapCfgPrint();

    SnapCfgSave();

    runSnapCfg.scheduleTime[0][0].startHour = 4;
    runSnapCfg.scheduleTime[0][0].startMin = 5;
    runSnapCfg.scheduleTime[0][0].stopHour = 22;
    runSnapCfg.scheduleTime[0][0].stopMin = 6;
    runSnapCfg.scheduleTime[6][3].startHour = 4;
    runSnapCfg.scheduleTime[6][3].startMin = 5;
    runSnapCfg.scheduleTime[6][3].stopHour = 21;
    runSnapCfg.scheduleTime[6][3].stopMin = 6;
    SnapCfgPrint();

    SnapCfgLoad();
    SnapCfgPrint();

    SnapCfgLoadDefValue();
    SnapCfgPrint();
#endif
}


void RecordCfgTest()
{
    RecordCfgLoad();
    RecordCfgPrint();

#if 1
    runRecordCfg.scheduleTime[0][0].startHour = 1;
    runRecordCfg.scheduleTime[0][0].startMin = 2;
    runRecordCfg.scheduleTime[0][0].stopHour = 23;
    runRecordCfg.scheduleTime[0][0].stopMin = 3;

    runRecordCfg.scheduleTime[6][3].startHour = 1;
    runRecordCfg.scheduleTime[6][3].startMin = 2;
    runRecordCfg.scheduleTime[6][3].stopHour = 23;
    runRecordCfg.scheduleTime[6][3].stopMin = 3;
    RecordCfgPrint();

    RecordCfgSave();

    runRecordCfg.scheduleTime[0][0].startHour = 4;
    runRecordCfg.scheduleTime[0][0].startMin = 5;
    runRecordCfg.scheduleTime[0][0].stopHour = 22;
    runRecordCfg.scheduleTime[0][0].stopMin = 6;
    runRecordCfg.scheduleTime[6][3].startHour = 4;
    runRecordCfg.scheduleTime[6][3].startMin = 5;
    runRecordCfg.scheduleTime[6][3].stopHour = 21;
    runRecordCfg.scheduleTime[6][3].stopMin = 6;
    RecordCfgPrint();

    RecordCfgLoad();
    RecordCfgPrint();

    RecordCfgLoadDefValue();
    RecordCfgPrint();
#endif
}

#if 1
void PtzCfgTest()
{
    PtzCfgLoad();
    PtzCfgPrint();


    runPtzCfg.protocol = 1;

    PtzCfgPrint();

    PtzCfgSave();

    runPtzCfg.protocol = 8;
    PtzCfgPrint();

    PtzCfgLoad();
    PtzCfgPrint();

    PtzCfgLoadDefValue();
    PtzCfgPrint();
}
#endif

void AlarmCfgTest()
{
    AlarmCfgLoad();
    AlarmCfgPrint();

#if 1
    runAlarmCfg.alarmIn.scheduleTime[0][0].startHour = 1;
    runAlarmCfg.alarmIn.scheduleTime[0][0].startMin = 2;
    runAlarmCfg.alarmIn.scheduleTime[0][0].stopHour = 23;
    runAlarmCfg.alarmIn.scheduleTime[0][0].stopMin = 3;

    runAlarmCfg.alarmIn.scheduleTime[6][3].startHour = 1;
    runAlarmCfg.alarmIn.scheduleTime[6][3].startMin = 2;
    runAlarmCfg.alarmIn.scheduleTime[6][3].stopHour = 23;
    runAlarmCfg.alarmIn.scheduleTime[6][3].stopMin = 3;
    AlarmCfgPrint();

    AlarmCfgSave();

    runAlarmCfg.alarmIn.scheduleTime[0][0].startHour = 4;
    runAlarmCfg.alarmIn.scheduleTime[0][0].startMin = 5;
    runAlarmCfg.alarmIn.scheduleTime[0][0].stopHour = 22;
    runAlarmCfg.alarmIn.scheduleTime[0][0].stopMin = 6;
    runAlarmCfg.alarmIn.scheduleTime[6][3].startHour = 4;
    runAlarmCfg.alarmIn.scheduleTime[6][3].startMin = 5;
    runAlarmCfg.alarmIn.scheduleTime[6][3].stopHour = 21;
    runAlarmCfg.alarmIn.scheduleTime[6][3].stopMin = 6;
    AlarmCfgPrint();

    AlarmCfgLoad();
    AlarmCfgPrint();

    AlarmCfgLoadDefValue();
    AlarmCfgPrint();
#endif
}


void UserCfgTest()
{
    UserCfgLoad();
    UserCfgPrint();

    strcpy(runUserCfg.user[0].password, "11111");
    UserCfgPrint();

    UserCfgSave();

    strcpy(runUserCfg.user[0].password, "22222");
    UserCfgPrint();

    UserCfgLoad();
    UserCfgPrint();

    UserCfgLoadDefValue();
    UserCfgPrint();
}

#define CFG_TEST_DEG 0
#if CFG_TEST_DEG
int main()
{
    printf("hello, main.\n");
    #if 0
    //CjsonArrayTest();

    SystemCfgTest();
    printf("\n\n\n");

    AudioCfgTest();
    printf("\n\n\n");

    VideoCfgTest();
    printf("\n\n\n");

    ImageCfgTest();
    printf("\n\n\n");

    NetworkCfgTest();
    printf("\n\n\n");

    MdCfgTest();
    SnapCfgTest();
    RecordCfgTest();
    AlarmCfgTest();
    PtzCfgTest();
    #else

    UserCfgTest();
    #endif

    return 0;
}

#endif

