
#include <unistd.h>
#include <pthread.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include "ite_sip_api.h"
#include <ite_msg_define.h>
//#include <ite_msg_key.h>
//#include <ite_message.h>
#include <ite_uac.h>
#include "ite_gb28181_playback.h"
#include "cfg_record.h"
#include "cfg_md.h"
#include "avi_rec.h"
#include "sdk_sys.h"
#include "netcam_api.h"

struct _ite_roi_param
{
    int roi_seq; /*感兴趣区域编号*/
    int top_left; /*感兴趣区域左上角坐标, 参考 GB/T 25724—2010 的5.2.4.4.2定义, 取值范围0~19683*/
    int bottom_right;/*感兴趣区域右下角坐标, 参考 GB/T 25724—2010 的5.2.4.4.2定义, 取值范围0~19683*/
    int roi_qp;/*ROI 区域编码质量等级, 取值0: 一般,1: 较好,2: 好,3: 很好*/
};

struct _ite_roi_info
{
    int roi_flag; /*感兴趣区域开关, 取值0: 关闭,1: 打开*/
    int roi_number; /*感兴趣区域数量, 取值范围0~16*/
    struct _ite_roi_param roi_param[16]; /*感兴趣区域*/
    int back_groundQP; /*背景区域编码质量等级, 取值0: 一般,1: 较好,2: 好,3: 很好*/
    int back_ground_skip_flag; /*背景跳过开关, 取值0: 关闭,1: 打开*/
} g_roi_info;

struct _ite_svac_audio_param/*音频参数*/
{
    int audio_recognition_flag;/*声音识别特征参数开关, 取值0: 关闭,1: 打开*/
} g_svac_audio_param;

struct _ite_svc_param/*SVC 参数*/
{
    int svc_space_domainmode;/*空域编码方式, 取值0: 基本层;1:1 级增强(1 个增强层) ;2:2 级增强(2 个增强层) ;3:3 级增强(3 个增强层)*/
    int svc_time_domainmode;/*时域编码方式, 取值0: 基本层;1:1 级增强;2:2 级增强;3:3 级增强*/
    int svc_space_supportmode;/*空域编码能力, 取值0: 不支持;1:1 级增强(1 个增强层) ;2:2 级增强(2 个增强层) ;3:3 级增强(3 个增强层)*/
    int svc_time_supportmode;/*时域编码能力, 取值0: 不支持;1:1 级增强;2:2 级增强;3:3 级增强*/
} g_dec_svc_param, g_enc_svc_param;/*解码配置 SVC 参数*//*编码配置 SVC 参数*/

struct _ite_surveillance_param/*监控专用信息参数*/
{
    int time_flag;/*绝对时间信息开关, 取值0: 关闭,1: 打开*/
    int event_flag;/*监控事件信息开关, 取值0: 关闭,1: 打开*/
    int alert_flag;/*报警信息开关, 取值0: 关闭,1: 打开*/
} g_dec_surveillance_param, g_enc_surveillance_param;/*解码配置监控专用信息参数*//*编码配置监控专用信息参数*/

/*预置位*/
struct _ite_preset_param
{
    int status;
    int preset_id;/*预置位id*/
    int preset_name;
};
struct _ite_presetlist_param
{
    int preset_num;
    struct _ite_preset_param preset_list[255];
} g_presetlist_param;

//static struct eXosip_t excontext_buf;
//static struct eXosip_t*excontext = &excontext_buf;

//OS_S32 g_bterminate = ITE_OSAL_FALSE;
//OS_S32 g_recvalarm = 0;/*为发报警定义的变量0不发报警，1为AlarmMethod视频报警, 2为AlarmMethod设备报警*/

extern OS_S32 g_bterminate;
extern OS_S32 g_recvalarm;
extern OS_S32 g_recvalarmType;


int g_register_id  = 0;//= 0;/*注册ID/用来更新注册或取消注册*/
int g_call_id      ;//= 0;/*INVITE连接ID/用来分辨不同的INVITE连接，每个时刻只允许有一个INVITE连接*/
int g_did_realPlay ;//= 0;/*会话ID/用来分辨不同的会话：实时视音频点播  */
int g_did_backPlay ;//= 0;/*会话ID/用来分辨不同的会话：历史视音频回放*/
int g_did_fileDown ;//= 0;/*会话ID/用来分辨不同的会话：视音频文件下载*/
int g_did_audioPlay ;//= 0;/*会话ID/用来分辨不同的会话：实时对讲*/
int g_register_status=0;

int g_keepalive_flag = 0;//发送心跳标志位置1，收到响应置0
int g_keepalive_outtimes = 0;//记录心跳没有收到响应的次数
char g_keepalive_callid[128];//记录心跳信令中的Call-ID number
char g_broadcast_callid[128];//记录广播请求中的Call-ID number
char g_broadcast_invite_callid[128];//记录广播响应中的Call-ID number
char g_broadcast_srcid[64];//记录广播请求中的SourceID
char g_broadcast_dstid[64];//记录广播请求中的TargetID

int positionExpires = 0;
int positionCurTime = 0;
int positionInterval = 5;

char g_alarm_method[10];

char g_strAuth[1024];
char g_call_id_str[2][100];

int MjEvtSipFailCnt = 0;
int MjEvtSipFailTime = 0;

extern char serverIp[50];
extern int serverPort;


extern void SetIFameFlag(int flag);
extern int ProcessInviteMediaPlay(char *control_type, char *media_fromat, char *local_ip, char *media_server_ip,
                        char *media_server_port, unsigned int enOverTcp, int channleIndex, int ssrc, int eventId, char *callidStr);
extern int ProcessMediaStop(int eventdid);
extern int ProcessMediaStopAll(void);


int  ite_eXosip_init(struct eXosip_t *excontext,IPNC_SipIpcConfig * SipConfig);/*初始化eXosip*/
int  ite_eXosip_register(struct eXosip_t *excontext,IPNC_SipIpcConfig * SipConfig,int expires);/*注册eXosip，手动处理服务器返回的401状态*/
int  ite_eXosip_unregister(struct eXosip_t *excontext,IPNC_SipIpcConfig * SipConfig);/*注销eXosip*/
void ite_eXosip_sendEventAlarm(struct eXosip_t *excontext,char *alarm_time,IPNC_SipIpcConfig * SipConfig);/*报警事件通知和分发：发送报警通知*/
void ite_eXosip_sendFileEnd(struct eXosip_t *excontext,IPNC_SipIpcConfig * SipConfig,int session_id);/*历史视音频回放：发送文件结束*/
void ite_eXosip_ProcessMsgBody(struct eXosip_t *excontext,eXosip_event_t *p_event,IPNC_SipIpcConfig * SipConfig);/*解析MESSAGE的XML消息体*/
void ite_eXosip_parseInviteBody(struct eXosip_t *excontext,eXosip_event_t *p_event,IPNC_SipIpcConfig *SipConfig);/*解析INVITE的SDP消息体，同时保存全局INVITE连接ID和全局会话ID*/
void ite_eXosip_parseInfoBody(struct eXosip_t *excontext,eXosip_event_t *p_event);/*解析INFO的RTSP消息体*/
void ite_eXosip_printEvent(struct eXosip_t *excontext,eXosip_event_t *p_event);/*检测并打印事件*/
void ite_eXosip_launch(struct eXosip_t *excontext,IPNC_SipIpcConfig * SipConfig);/*启动并注册eXosip*/

//int ite_eXosip_sendinvite(char * to,char * sdpMessage, char ** responseSdp,sessionId *id);//by jzw
int ite_eXosip_invit(struct eXosip_t *excontext,sessionId * id, char * to, char * sdpMessage, char * responseSdp,IPNC_SipIpcConfig * SipConfig);
int ite_eXosip_bye(struct eXosip_t *excontext,sessionId id);

/*初始化eXosip*/
int ite_eXosip_init(struct eXosip_t *excontext,IPNC_SipIpcConfig * SipConfig)
{
    g_register_id  = 0;/*注册ID/用来更新注册或取消注册*/
    g_call_id      = 0;/*INVITE连接ID/用来分辨不同的INVITE连接，每个时刻只允许有一个INVITE连接*/
    g_did_realPlay = 0;/*会话ID/用来分辨不同的会话：实时视音频点播*/
    g_did_backPlay = 0;/*会话ID/用来分辨不同的会话：历史视音频回放*/
    g_did_fileDown = 0;/*会话ID/用来分辨不同的会话：视音频文件下载*/
    printf("ite_eXosip_init start!\r\n");

    int ret = 0;
    printf("ite_eXosip_init--->eXosip_init [%s %d]---[%s %d]\r\n",SipConfig->sipDeviceUID,SipConfig->sipDevicePort,SipConfig->sipRegServerIP,SipConfig->sipRegServerPort);

    ret = eXosip_init(excontext);/*初始化osip和eXosip协议栈*/
    if (0 != ret)
    {
        printf("Couldn't initialize eXosip!\r\n");
        return -1;
    }

    return 0;
}

int SetSystemTime(char *dt)
{
    struct tm tm;
    struct timeval tv;
    struct timeval LocalTime;
    struct timezone tz;
    time_t timep;
    ITE_Time_Struct ptPackedBits;

    gettimeofday(&LocalTime, &tz);

    sscanf(dt, "%d-%d-%dT%d:%d:%d.%d", (int*)&ptPackedBits.wYear,
        (int*)&ptPackedBits.wMonth, (int*)&ptPackedBits.wDay,(int*)&ptPackedBits.wHour,
        (int*)&ptPackedBits.wMinute, (int*)&ptPackedBits.wSecond, (int*)&ptPackedBits.wMilliseconds);
    memset(&tm, '\0', sizeof(tm));
    tm.tm_sec   = ptPackedBits.wSecond;
    tm.tm_min   = ptPackedBits.wMinute;
    tm.tm_hour  = ptPackedBits.wHour;
    tm.tm_mday  = ptPackedBits.wDay;
    tm.tm_mon   = ptPackedBits.wMonth - 1;
    tm.tm_year  = ptPackedBits.wYear - 1900;

    printf("wYear:%d, wMonth:%d, wDay:%d, wHour:%d, wMinute:%d, wSecond:%d, wMilliseconds:%d\r\n",
        ptPackedBits.wYear, ptPackedBits.wMonth, ptPackedBits.wDay, ptPackedBits.wHour,
        ptPackedBits.wMinute, ptPackedBits.wSecond, ptPackedBits.wMilliseconds);

    timep = mktime(&tm);
    tv.tv_sec = timep;
    tv.tv_usec  = ptPackedBits.wMilliseconds * 1000;
    //tv.tv_usec = 0;
    if(settimeofday (&tv, (struct timezone *) 0) < 0)
    {
        printf("Set system datatime error!\r\n");
        return ITE_OSAL_FAIL;
    }

    //system("hwclock -w");/*强制把系统时间写入CMOS*/
    return 0;
}

int ite_eXosip_rand_num(int min, int max)
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

/*注册eXosip，手动处理服务器返回的401状态*/
int ite_eXosip_register(struct eXosip_t *excontext,IPNC_SipIpcConfig * SipConfig,int expires)/*expires/注册消息过期时间，单位为秒*/
{
    int ret = 0;
    int waitTime = 50;
    int waitTimeTotal = 0;
    int errorCnt = 0;
    eXosip_event_t *je  = NULL;
    osip_message_t *reg = NULL;
    osip_contact_t *clone_contact = NULL;
    //char *message = NULL;
    char from[128];/*sip:主叫用户名@被叫IP地址*/
    char proxy[128];/*sip:被叫IP地址:被叫IP端口*/
    char contract[128];
    char localip[128]={0};
    int lastId = -1;
    int isSetVia = 0;
    
#ifdef MODULE_SUPPORT_UDP_LOG
	char msgStr[200] = {0};
#endif

    memset(localip,0,128);
    memset(from, 0, 128);
    memset(proxy, 0, 128);
    memset(contract, 0, 128);
    eXosip_guess_localip (excontext,AF_INET, localip, 128);
    printf("eXosip_guess_localip[%s]\r\n",localip);

    sprintf(from, "sip:%s@%s:%d", SipConfig->sipDeviceUID,localip,SipConfig->sipDevicePort);
    sprintf(proxy, "sip:%s@%s:%d",SipConfig->sipRegServerUID,SipConfig->sipRegServerIP, SipConfig->sipRegServerPort);
    sprintf(contract, "sip:%s@%s:%d", SipConfig->sipDeviceUID,localip,SipConfig->sipDevicePort);

    /*发送不带认证信息的注册请求*/


    if(eXosip_add_authentication_info(excontext, SipConfig->sipDeviceUID, SipConfig->sipDeviceUID,  SipConfig->sipRegServerPwd, "MD5", SipConfig->sipRegServerDomain) !=   OSIP_SUCCESS)
    {
         printf("eXosip_add_authentication_info [error\r\n");
         return -1;
    }
retry:
    eXosip_lock(excontext);
    if (g_register_id > 0)
    {
        eXosip_register_remove(excontext, g_register_id);
    }
    g_register_id = eXosip_register_build_initial_register(excontext,from, proxy, contract, expires, &reg);
    printf("eXosip_register_build_initial_register g_register_id[%d]\r\n",g_register_id);
    //osip_message_set_authorization(reg, "Capability algorithm=\"H:MD5\"");
    if (0 > g_register_id)
    {
        eXosip_unlock(excontext);
        printf("eXosip_register_build_initial_register error!\r\n");
        return -1;
    }
    #if 0
    if (isSetVia)
    {
        osip_via_t *sipVia = (osip_via_t *)osip_list_get (&reg->vias, 0);
        printf("via_get_port22:%s\n", via_get_port(sipVia));
        if (sipVia->port != NULL)
            sprintf(sipVia->port, "%d", SipConfig->sipDevicePort);
        //via_set_port(sipVia, sipVia->port);
        printf("via_get_port22:%s\n", via_get_port(sipVia));
        isSetVia = 0;
    }
    #endif

    ret = eXosip_register_send_register(excontext,g_register_id, reg);
    eXosip_unlock(excontext);
    if (0 != ret)
    {
        printf("eXosip_register_send_register no authorization error!\r\n");
        return -1;
    }
    printf("eXosip_register_send_register  over!\r\n");

    for (;;)
    {
        if(je)
        {
            eXosip_event_free(je);
            je = NULL;
        }

        if (errorCnt > 3)
        {
            ret = -1;
            break;
        }
        je = eXosip_event_wait(excontext,0, waitTime);/*侦听消息的到来*/
        if (NULL == je)/*没有接收到消息*/
        {
            waitTimeTotal += waitTime;
            if (waitTimeTotal > 60000)
            {
                //超时没有等到返回结果，需要退出，否则一直收不到注册消息卡死在这里
                break;
            }
            continue;
        }
        waitTimeTotal = 0;
        printf("-->get event\n");
        //eXosip_listen_addr(excontext,IPPROTO_UDP, NULL, SipConfig->sipDevicePort, AF_INET, 0);
        ite_eXosip_printEvent(excontext,je);
        if (EXOSIP_REGISTRATION_FAILURE == je->type)/*注册失败*/
        {
            printf("<EXOSIP_REGISTRATION_FAILURE>\r\n");
            printf("je->rid=%d\r\n", je->rid);
            if (NULL != je->response)
                printf("je->status=%d\r\n", je->response->status_code);
            else
            {
				#if 1
                #ifdef MODULE_SUPPORT_MOJING
                if (je->rid - lastId > 6 && mojing_get_register_status() == 1)
                {
                    #if 0
                    //if (runGB28181Cfg.DevicePort == 5060)
                    {
                        runGB28181Cfg.DevicePort = ite_eXosip_rand_num(5060, 9999);
                        GB28181CfgSave();
                        runAudioCfg.rebootMute = 1;
                        AudioCfgSave();
                        sleep(1);
                        if(!(netcam_is_prepare_update() || (netcam_get_update_status() != 0)))
                        {
                            netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); 
                        }
                    }
                    #endif
                    lastId = je->rid;
                }
                if (lastId == -1)
                {
                    lastId = je->rid;
                }
                #endif
				#endif
                printf("je->response is null\n");
            }
            /*收到服务器返回的注册失败/401未认证状态*/
            if ((NULL != je->response) && (401 == je->response->status_code))
            {

                if(0/*when receive 401Unauthorized package，send ACK and Regester*/)
                {
                    osip_message_t *ack = NULL;
                    //int call_id = atoi(reg->call_id->number);
                    printf("je->did:%d\n", je->did);
                    ret = eXosip_call_build_ack(excontext,je->rid, &ack);
                    ret = eXosip_call_send_ack(excontext,je->rid, ack);
                }

                reg = NULL;
                /*发送携带认证信息的注册请求*/
                eXosip_lock(excontext);
                eXosip_clear_authentication_info(excontext);/*清除认证信息*/
                eXosip_add_authentication_info(excontext,SipConfig->sipDeviceUID, SipConfig->sipDeviceUID, SipConfig->sipRegServerPwd, "MD5", NULL);/*添加主叫用户的认证信息*/
                eXosip_register_build_register(excontext,je->rid, expires, &reg);

                ret = eXosip_register_send_register(excontext,je->rid, reg);
                #if 0
                {
                    osip_authorization_t * auth = NULL;
                    char *strAuth=NULL;
                    osip_message_get_authorization(reg,0,&auth);
                    osip_authorization_to_str(auth,&strAuth);
                    if(strAuth)
                    {
                        strcpy(g_strAuth,strAuth);//保存认证字符串
                        free(strAuth);
                    }
                }
                #endif

                eXosip_unlock(excontext);
                //if(je)eXosip_event_free(je);
                if (0 != ret)
                {
                    printf("eXosip_register_send_register authorization error!\r\n");
                    ret = -1;
                    break;
                }
                printf("eXosip_register_send_register authorization success!\r\n");
            }
            else if ((NULL != je->response) && (302 == je->response->status_code))
            {

                eXosip_lock(excontext);
                osip_message_get_contact(je->response, 0, &clone_contact);
                sprintf(proxy, "sip:%s@%s:%s",clone_contact->url->username, clone_contact->url->host, clone_contact->url->port);
                printf("contact:server ip:%s, port:%s, name:%s, pwd:%s\r\n", clone_contact->url->host,
                    clone_contact->url->port,clone_contact->url->username, clone_contact->url->password);
                strcpy(SipConfig->sipRegServerUID, clone_contact->url->username);
                strcpy(SipConfig->sipRegServerIP, clone_contact->url->host);
                SipConfig->sipRegServerPort = strtol(clone_contact->url->port, NULL, 0);
                ret = eXosip_clear_authentication_info(excontext);
                if(eXosip_add_authentication_info(excontext, SipConfig->sipDeviceUID, SipConfig->sipDeviceUID,  SipConfig->sipRegServerPwd, "MD5", SipConfig->sipRegServerDomain) !=   OSIP_SUCCESS)
                {
                     printf("eXosip_add_authentication_info [error\r\n");
                     eXosip_unlock(excontext);
                     //if(je)eXosip_event_free(je);
                     ret = -1;
                     break;
                }
                eXosip_unlock(excontext);
                //if(je)eXosip_event_free(je);
                //sleep(30);

#ifdef MODULE_SUPPORT_UDP_LOG
				snprintf(msgStr, sizeof(msgStr),"redirect()ip=%s, port=%s",clone_contact->url->host,clone_contact->url->port);
#ifdef MODULE_SUPPORT_MOJING
				mojing_log_send(NULL, 0, "gb28181_connect", msgStr);
#endif
#endif
                goto retry;/*重新注册*/
            }
            else/*真正的注册失败*/
            {
                printf("EXOSIP_REGISTRATION_FAILURE error!\r\n");
#ifdef DMODULE_SUPPORT_MOJING_LOG_AES
                snprintf(msgStr, sizeof(msgStr),"register()ip=%s, port=%s",clone_contact->url->host,clone_contact->url->port);
                mojing_log_send(NULL, -1, "gb28181_gw_register", msgStr);
#endif
                errorCnt++;
                
                MjEvtSipFailCnt++;
                if (MjEvtSipFailCnt == 1)
                {
                    MjEvtSipFailTime = time(NULL);
                }
                sleep(30);
                //if(je)eXosip_event_free(je);
                goto retry;/*重新注册*/
            }
        }
        else if (EXOSIP_REGISTRATION_SUCCESS == je->type)
        {
            /*收到服务器返回的注册成功*/
            printf("<EXOSIP_REGISTRATION_SUCCESS>\r\n");
            g_register_id = je->rid;/*保存注册成功的注册ID*/
            printf("g_register_id=%d\r\n", g_register_id);
            g_register_status =1;
            MjEvtSipFailCnt = 0;

            osip_header_t *header = NULL;
            osip_message_header_get_byname(je->response, "Date", 0, &header);/*获取头Data字段的值，保存的是时间例如:2006-08-14T02:34:56*/
            if (header != NULL)
            {
                printf("header->hname:%s, hvalue:%s\r\n", header->hname, header->hvalue);
                #ifndef MODULE_SUPPORT_MOJING
                SetSystemTime(header->hvalue);/*将SIP服务器时间设置到系统中*/
                #endif
            }
            break;
        }
    }

    eXosip_lock (excontext);
    int opt_keepalive = 0;
    eXosip_set_option (excontext, EXOSIP_OPT_UDP_KEEP_ALIVE, &opt_keepalive);
    eXosip_unlock (excontext);

    if(je)eXosip_event_free(je);
    if (ret != 0)
    {
        eXosip_register_remove(excontext, g_register_id);
    }
    return ret;
}
/*注销eXosip*/
int ite_eXosip_unregister(struct eXosip_t *excontext,IPNC_SipIpcConfig * SipConfig)
{
    return ite_eXosip_register(excontext,SipConfig,0);
    //eXosip_register_remove(excontext,g_register_id);
}

/*报警事件通知和分发：发送报警通知*/
void ite_eXosip_sendEventAlarm(struct eXosip_t *excontext,char *alarm_time,IPNC_SipIpcConfig * SipConfig)
{
	if(!runGB28181Cfg.GBWarnEnable)
		return;

    if (0 == strcmp(device_status.status_guard, "OFFDUTY"))/*当前布防状态为：OFFDUTY，不发送报警消息*/
    {
        printf("device_status.status_guard=OFFDUTY\r\n");
    }
    else
    {
        osip_message_t *rqt_msg = NULL;
        char to[100];/*sip:主叫用户名@被叫IP地址*/
        char from[100];/*sip:被叫IP地址:被叫IP端口*/
        char xml_body[4096];
        char localip[128]={0};
        static int SN=1;
        int alarm_method = 2;
        char alarm_device_id[30] = {0};
        char areaStr[128] = {0};

        memset(to, 0, 100);
        memset(from, 0, 100);
        memset(xml_body, 0, 4096);
        //if(g_recvalarm == 1)
        {/*g_recvalarm定义，1为AlarmMethod视频报警, 2为AlarmMethod设备报警*/
            alarm_method = 5; //35信号
            strcpy(alarm_device_id, device_info.ipc_alarmid);
        }
        #if 0
        else
        {
            alarm_method = 2; //36信号
            strcpy(alarm_device_id, device_info.ipc_id);
        }
        #endif
        strcpy(alarm_device_id, SipConfig->sipDeviceUID);
        sprintf(g_alarm_method, "%d", alarm_method);
        eXosip_guess_localip(excontext,AF_INET, localip, 128);
        printf("localip:%s\n", localip);
        sprintf(to, "sip:%s@%s:%d",SipConfig->sipRegServerUID,SipConfig->sipRegServerIP, SipConfig->sipRegServerPort);
        sprintf(from, "sip:%s@%s:%d", SipConfig->sipDeviceUID,localip,SipConfig->sipDevicePort);

        //sprintf(from, "sip:%s@%s:%d", SipConfig->sipDeviceUID,SipConfig->sipRegServerIP, SipConfig->sipRegServerPort);
        //sprintf(to, "sip:%s@%s:%d", SipConfig->sipRegServerUID,SipConfig->sipRegServerIP,SipConfig->sipRegServerPort);

        //sprintf(to, "sip:%s@%s", SipConfig->sipDeviceUID, SipConfig->sipRegServerIP);
        //sprintf(from, "sip:%s:%d",SipConfig->sipRegServerIP, SipConfig->sipRegServerPort);
        eXosip_message_build_request(excontext,&rqt_msg, "MESSAGE", to, from, NULL);/*构建"MESSAGE"请求*/
#if 1
        if (g_recvalarmType == 6)
        {
            sprintf(areaStr, "<AlarmTypeParam>\r\n<EventType>%d</EventType>\r\n</AlarmTypeParam>\r\n", 1);
        }
        snprintf(xml_body, 4096, "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n"
                 "<Notify>\r\n"
                 "<CmdType>Alarm</CmdType>\r\n"/*命令类型*/
                 "<SN>%d</SN>\r\n"/*命令序列号*/
                 "<DeviceID>%s</DeviceID>\r\n"/*报警设备编码/报警中心编码*/
                 "<AlarmPriority>1</AlarmPriority>\r\n"/*报警级别/1为一级警情/2为二级警情/3为三级警情/4为四级警情*/
                 "<AlarmTime>%s</AlarmTime>\r\n"/*报警时间/格式：2012-12-18T16:23:32*/
                 "<AlarmMethod>%s</AlarmMethod>\r\n"/*报警方式/1为电话报警/2为设备报警/3为短信报警/4为GPS报警/5为视频报警/6为设备故障报警/7其它报警*/
                 "<Info>\r\n"
                 "<AlarmType>%d</AlarmType>\r\n"/*报警方式为5时，取值如下：1-人工视频报警；2-运动目标检测报警；3-遗留物检测报警；4-物体移除检测报警；5-绊线检测报警；6-入侵检测报警；7-逆行检测报警；8-徘徊检测报警；9-流量统计报警；10-密度检测报警；11-视频异常检测报警；12-快速移动报警*/
                 "%s"
                 "</Info>\r\n"
                 "</Notify>\r\n",
                 SN++,
                 alarm_device_id,/*需要是报警通道*/
                 alarm_time,
                 g_alarm_method,
                 g_recvalarmType, areaStr);
#else
        snprintf(xml_body, 4096, "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n"
        "<Notify>\r\n"
        "<CmdType>Alarm</CmdType>\r\n"
        "<SN>1</SN>\r\n"
        "<DeviceID>34020000001340000001</DeviceID>\r\n"
        "<AlarmPriority>1</AlarmPriority>\r\n"
        "<AlarmTime>2014-09-24T10:39:15</AlarmTime>\r\n"
        "<AlarmMethod>2</AlarmMethod>\r\n"
        "</Notify>\r\n");
#endif
        if (rqt_msg)
        {
            osip_message_set_body(rqt_msg, xml_body, strlen(xml_body));
            osip_message_set_content_type(rqt_msg, "Application/MANSCDP+xml");

            eXosip_lock (excontext);
            eXosip_message_send_request(excontext,rqt_msg);/*回复"MESSAGE"请求*/
            eXosip_unlock (excontext);
        }
        printf("ite_eXosip_sendEventAlarm success!\r\n");

        strcpy(device_status.status_guard, "ALARM");/*设置布防状态为"ALARM"*/
    }

    return;
}

void ite_eXosip_sendEventPostion(struct eXosip_t *excontext,char *alarm_time,IPNC_SipIpcConfig * SipConfig)
{
    osip_message_t *rqt_msg = NULL;
    char to[100];/*sip:主叫用户名@被叫IP地址*/
    char from[100];/*sip:被叫IP地址:被叫IP端口*/
    char xml_body[4096];
    char localip[128]={0};
    static int SN=1;
    float Longitude = 123.2;
    float Latitude = 23.2;
    float Speed = 0;
    float Direction = 1;
    float Altitude = 0;

    memset(to, 0, 100);
    memset(from, 0, 100);
    memset(xml_body, 0, 4096);

    eXosip_guess_localip(excontext,AF_INET, localip, 128);
    printf("localip:%s\n", localip);
    sprintf(to, "sip:%s@%s:%d",SipConfig->sipRegServerUID,SipConfig->sipRegServerIP, SipConfig->sipRegServerPort);
    sprintf(from, "sip:%s@%s:%d", SipConfig->sipDeviceUID,localip,SipConfig->sipDevicePort);

    //sprintf(from, "sip:%s@%s:%d", SipConfig->sipDeviceUID,SipConfig->sipRegServerIP, SipConfig->sipRegServerPort);
    //sprintf(to, "sip:%s@%s:%d", SipConfig->sipRegServerUID,SipConfig->sipRegServerIP,SipConfig->sipRegServerPort);

    //sprintf(to, "sip:%s@%s", SipConfig->sipDeviceUID, SipConfig->sipRegServerIP);
    //sprintf(from, "sip:%s:%d",SipConfig->sipRegServerIP, SipConfig->sipRegServerPort);
    eXosip_message_build_request(excontext,&rqt_msg, "MESSAGE", to, from, NULL);/*构建"MESSAGE"请求*/

#if 1
    snprintf(xml_body, 4096, "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n"
             "<Notify>\r\n"
             "<CmdType>MobilePosition</CmdType>\r\n"/*命令类型*/
             "<SN>%d</SN>\r\n"/*命令序列号*/
             "<Time>%s</Time>\r\n"
             "<Longitude>%.2f</Longitude>\r\n"
             "<Latitude>%.2f</Latitude>\r\n"
             "<Speed>%.2f</Speed>\r\n"
             "<Direction>%.2f</Direction>\r\n"
             "<Altitude>%.2f</Altitude>\r\n"
             "</Notify>\r\n",
             SN++,
             alarm_time,
             Longitude,
             Latitude,
             Speed,
             Direction,
             Altitude);
#else
    snprintf(xml_body, 4096, "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n"
    "<Notify>\r\n"
    "<CmdType>Alarm</CmdType>\r\n"
    "<SN>1</SN>\r\n"
    "<DeviceID>34020000001340000001</DeviceID>\r\n"
    "<AlarmPriority>1</AlarmPriority>\r\n"
    "<AlarmTime>2014-09-24T10:39:15</AlarmTime>\r\n"
    "<AlarmMethod>2</AlarmMethod>\r\n"
    "</Notify>\r\n");
#endif
    osip_message_set_body(rqt_msg, xml_body, strlen(xml_body));
    osip_message_set_content_type(rqt_msg, "Application/MANSCDP+xml");

    eXosip_lock (excontext);
    eXosip_message_send_request(excontext,rqt_msg);/*回复"MESSAGE"请求*/
    eXosip_unlock (excontext);
    printf("ite_eXosip_sendEventPostion success!\r\n");

    return;
}

/*历史视音频回放：发送文件结束*/
void ite_eXosip_sendFileEnd(struct eXosip_t *excontext,IPNC_SipIpcConfig * SipConfig,int session_id)
{
    //if (0 != g_did_backPlay)/*当前会话为：历史视音频回放*/
    //printf("ite_eXosip_sendFileEnd return\n");
    //return;

	if(session_id != 0 && session_id != 1)
	{
		printf("ite_eXosip_sendFileEnd session_id error!\n");
		return;
	}

    {
        osip_message_t *rqt_msg = NULL;
        char to[128];/*sip:主叫用户名@被叫IP地址*/
        char from[128];/*sip:被叫IP地址:被叫IP端口*/
        char contact[128];
        char xml_body[4096];
		char localip[128]={0};

        memset(to, 0, 128);
        memset(from, 0, 128);
        memset(contact, 0, 128);
        memset(xml_body, 0, 4096);

		eXosip_guess_localip (excontext,AF_INET, localip, 128);
		sprintf(from, "sip:%s@%s:%d", SipConfig->sipDeviceUID,localip,SipConfig->sipDevicePort);
		sprintf(to, "sip:%s@%s:%d",SipConfig->sipRegServerUID,SipConfig->sipRegServerIP, SipConfig->sipRegServerPort);
		sprintf(contact, "sip:%s@%s:%d",SipConfig->sipDeviceUID,localip,SipConfig->sipDevicePort);

        eXosip_message_build_request(excontext,&rqt_msg, "MESSAGE", to, from, NULL);/*构建"MESSAGE"请求*/
        snprintf(xml_body, 4096, "<?xml version=\"1.0\"?>\r\n"
                 "<Notify>\r\n"
                 "<CmdType>MediaStatus</CmdType>\r\n"
                 "<SN>8</SN>\r\n"
                 "<DeviceID>%s</DeviceID>\r\n"
                 "<NotifyType>121</NotifyType>\r\n"
                 "</Notify>\r\n",
                 SipConfig->sipDeviceUID);

		osip_message_set_contact(rqt_msg, contact);
		osip_call_id_t *callid = osip_message_get_call_id(rqt_msg);
        char *tmpId = osip_call_id_get_number(callid);
        if (tmpId != NULL)
        {
            free(tmpId);
        }
        tmpId = (char*)malloc(strlen(g_call_id_str[session_id]) + 1);
        strcpy(tmpId, g_call_id_str[session_id]);
		osip_call_id_set_number (callid, tmpId);
        osip_message_set_body(rqt_msg, xml_body, strlen(xml_body));
        osip_message_set_content_type(rqt_msg, "Application/MANSCDP+xml");
        eXosip_lock (excontext);
        eXosip_message_send_request(excontext,rqt_msg);/*回复"MESSAGE"请求*/
        eXosip_unlock (excontext);
        printf("ite_eXosip_sendFileEnd success!\r\n");
    }
}

static int ite_eXosip_ProcessPtzCmd(char *xml_cmd)
{
#define PTZ_STEP				30

#define PTZ_STOP_FLAG			0x0
#define PTZ_RIGHT_FLAG			0x1
#define PTZ_LEFT_FLAG			0x2
#define PTZ_DOWN_FLAG			0x4
#define PTZ_UP_FLAG				0x8
#define PTZ_RIGHT_DOWN_FLG		(PTZ_RIGHT_FLAG | PTZ_DOWN_FLAG)
#define PTZ_RIGHT_UP_FLG		(PTZ_RIGHT_FLAG | PTZ_UP_FLAG)
#define PTZ_LEFT_DOWN_FLG		(PTZ_LEFT_FLAG  | PTZ_DOWN_FLAG)
#define PTZ_LEFT_UP_FLG			(PTZ_LEFT_FLAG  | PTZ_UP_FLAG)
#define ZOOM_OUT_FLAG			0x10
#define ZOOM_IN_FLAG			0x20

	int h_speed = 0,v_speed = 0,zoom_speed = 0;
	int cmd = 0;
	char tmp[2];
	int ret = 0;

	if(!xml_cmd)
		return -1;

	if(strlen(xml_cmd) != 16)
	{
		printf("ptz cmd len %d error!\n",strlen(xml_cmd));
		return -1;
	}

	memset(tmp,0,sizeof(tmp));
	memcpy(tmp,&xml_cmd[6],2);
	cmd = strtoul(tmp,NULL,16);

	memcpy(tmp,&xml_cmd[8],2);
	v_speed = strtoul(tmp,NULL,16);

	memcpy(tmp,&xml_cmd[10],2);
	h_speed = strtoul(tmp,NULL,16);

	memcpy(tmp,&xml_cmd[12],2);
	zoom_speed = strtoul(tmp,NULL,16);

	printf("ite_eXosip_ProcessPtzCmd:%s\n",xml_cmd);

	printf("cmd:0x%x,v_speed:%d,h_speed:%d,zoom_speed:%d\n",cmd,v_speed,h_speed,zoom_speed);

	switch(cmd)
	{
		case PTZ_STOP_FLAG:
		{
			printf("netcam_ptz_stop\n");
			netcam_ptz_stop();
			break;
		}
		case PTZ_RIGHT_FLAG:
		{
			printf("netcam_ptz_right,speed:%d\n",h_speed);
			netcam_ptz_right(PTZ_STEP,h_speed);
			break;
		}
		case PTZ_LEFT_FLAG:
		{
			printf("netcam_ptz_left,speed:%d\n",h_speed);
			netcam_ptz_left(PTZ_STEP,h_speed);
			break;
		}
		case PTZ_DOWN_FLAG:
		{
			printf("netcam_ptz_down,speed:%d\n",v_speed);
			netcam_ptz_down(PTZ_STEP,v_speed);
			break;
		}
		case PTZ_UP_FLAG:
		{
			printf("netcam_ptz_up,speed:%d\n",v_speed);
			netcam_ptz_up(PTZ_STEP,v_speed);
			break;
		}
		case PTZ_RIGHT_DOWN_FLG:
		{
			printf("netcam_ptz_right_down,speed:%d,%d\n",h_speed,v_speed);

			if(v_speed)
				netcam_ptz_right(PTZ_STEP,v_speed);
            
			if(h_speed)
				netcam_ptz_down(PTZ_STEP,h_speed);
			break;
		}
		case PTZ_RIGHT_UP_FLG:
		{
			printf("netcam_ptz_right_up,speed:%d,%d\n",h_speed,v_speed);
			if(v_speed)
				netcam_ptz_right(PTZ_STEP,v_speed);
			if(h_speed)
				netcam_ptz_up(PTZ_STEP,h_speed);
			break;
		}
		case PTZ_LEFT_UP_FLG:
		{
			printf("netcam_ptz_left_up,speed:%d,%d\n",h_speed,v_speed);
			if(v_speed)
				netcam_ptz_left(PTZ_STEP,v_speed);
			if(h_speed)
				netcam_ptz_up(PTZ_STEP,h_speed);
			break;
		}
		case PTZ_LEFT_DOWN_FLG:
		{
			printf("netcam_ptz_left_down,speed:%d,%d\n",h_speed,v_speed);
			if(v_speed)
				netcam_ptz_left(PTZ_STEP,v_speed);
			if(h_speed)
				netcam_ptz_down(PTZ_STEP,h_speed);
			break;
		}
		case ZOOM_OUT_FLAG:
		{
			ret = -1;
			break;
		}
		case ZOOM_IN_FLAG:
		{
			ret = -1;
			break;
		}
		default:
		{
			ret = -1;
			break;
		}
	}

	return ret;
}

static int ite_eXosip_BroadcastInvite(struct eXosip_t *excontext,eXosip_event_t *p_event,IPNC_SipIpcConfig * SipConfig)
{
	osip_message_t *invite;
	int i;
    char to[100];/*sip:主叫用户名@被叫IP地址*/
    char from[100];/*sip:被叫IP地址:被叫IP端口*/
    char subject[100];
	int tcp_port = 10620;
	char tmp[4096];
	char localip[128];
	int  seq = 1;

    memset(to, 0, 100);
    memset(from, 0, 100);
    memset(subject, 0, 100);
    memset(localip, 0, 128);

    eXosip_guess_localip (excontext,AF_INET, localip, 128);

    sprintf(from, "sip:%s@%s:%d", SipConfig->sipDeviceUID,localip, SipConfig->sipRegServerPort);
#if 0
    sprintf(to, "sip:%s@%s:%d", SipConfig->sipRegServerUID,SipConfig->sipRegServerIP,SipConfig->sipRegServerPort);
    sprintf(subject, "%s:1,%s:1", SipConfig->sipRegServerUID,SipConfig->sipDeviceUID);
#else
    sprintf(to, "sip:%s@%s:%d", g_broadcast_srcid,SipConfig->sipRegServerIP,SipConfig->sipRegServerPort);
    sprintf(subject, "%s:%d,%s:%d", g_broadcast_srcid,seq,SipConfig->sipDeviceUID,seq);
#endif


	i = eXosip_call_build_initial_invite (excontext,&invite,
										to,
										from,
										NULL, // optional route header
										subject);
	if (i != 0)
	{
		printf("ite_eXosip_BroadcastInvite failed!\n");
		return -1;
	}

	//osip_message_set_supported (invite, "100rel");
	// 格式化SDP信息体
	snprintf (tmp, 4096,
			 "v=0\r\n"							 // SDP版本
			 "o=%s 0 0 IN IP4 %s\r\n"		 // 用户名、ID、版本、网络类型、地址类型、IP地址
			 "s=Play\r\n"				 // 会话名称
			 "i=Voice Call Session\r\n"				// 会话名称
			 "c=IN IP4 %s\r\n"
			 "t=0 0\r\n"						 // 开始时间、结束时间。此处不需要设置
			 "m=audio %d TCP/RTP/AVP 8\r\n"	 // 音频、传输端口、传输类型、格式列表
			 "a=recvonly\r\n" 		 // 以下为具体描述格式列表中的
			 "a=rtpmap:8 PCMA/8000\r\n"
			 "a=setup:passive\r\n"
			 "a=connection:new\r\n"
			 "y=0100000001\r\n"
			 "f=v/////a/1/8/1\r\n", SipConfig->sipDeviceUID, localip, localip,tcp_port);

	osip_message_set_body (invite, tmp, strlen (tmp));
	osip_message_set_content_type (invite, "application/sdp");

	eXosip_lock(excontext);
	i = eXosip_call_send_initial_invite (excontext,invite); // 发送INVITE请求
	eXosip_unlock(excontext);

	osip_call_id_t *callid = osip_message_get_call_id(invite);

	if(callid && callid->number)
	{
		memset(g_broadcast_invite_callid,0,sizeof(g_broadcast_invite_callid));
		strncpy(g_broadcast_invite_callid,callid->number,sizeof(g_broadcast_invite_callid));

		printf("ite_eXosip_BroadcastInvite callid number:%s\n", callid->number);
	}

	return i;
}

/*解析MESSAGE的XML消息体*/
void ite_eXosip_ProcessMsgBody(struct eXosip_t *excontext,eXosip_event_t *p_event,IPNC_SipIpcConfig * SipConfig)
{
    /*与请求相关的变量*/
    osip_body_t *p_rqt_body = NULL;
    char *p_xml_body  = NULL;
    char *p_str_begin = NULL;
    char *p_str_end   = NULL;
    char xml_cmd_type[20];
    char xml_cmd_sn[10];
    char xml_device_id[30];
    char xml_command[30];
    char localip[128]={0};
    /*与回复相关的变量*/
    osip_message_t *rsp_msg = NULL;
    char to[100];/*sip:主叫用户名@被叫IP地址*/
    char from[100];/*sip:被叫IP地址:被叫IP端口*/
    char rsp_xml_body[4096];
    int step = 10, speed = 3;

    memset(xml_cmd_type, 0, 20);
    memset(xml_cmd_sn, 0, 10);
    memset(xml_device_id, 0, 30);
    memset(xml_command, 0, 30);
    memset(to, 0, 100);
    memset(from, 0, 100);
    memset(rsp_xml_body, 0, 4096);

    eXosip_guess_localip (excontext,AF_INET, localip, 128);

    sprintf(from, "sip:%s@%s:%d", SipConfig->sipDeviceUID,SipConfig->sipRegServerIP, SipConfig->sipRegServerPort);
    sprintf(to, "sip:%s@%s:%d", SipConfig->sipRegServerUID,SipConfig->sipRegServerIP,SipConfig->sipRegServerPort);

    eXosip_message_build_request(excontext,&rsp_msg, "MESSAGE", to, from, NULL);/*构建"MESSAGE"请求*/

    osip_message_get_body(p_event->request, 0, &p_rqt_body);/*获取接收到请求的XML消息体*/
    if (NULL == p_rqt_body)
    {
        printf("osip_message_get_body null!\r\n");
        return;
    }
    p_xml_body = p_rqt_body->body;

    printf("**********CMD START**********\r\n");
    p_str_begin = strstr(p_xml_body, "<CmdType>");/*查找字符串"<CmdType>"*/
    p_str_end   = strstr(p_xml_body, "</CmdType>");

	if(p_str_begin && p_str_end)
	{
	    memcpy(xml_cmd_type, p_str_begin + 9, p_str_end - p_str_begin - 9); /*保存<CmdType>到xml_cmd_type*/
	    printf("<CmdType>:%s\r\n", xml_cmd_type);
	}

    p_str_begin = strstr(p_xml_body, "<SN>");/*查找字符串"<SN>"*/
    p_str_end   = strstr(p_xml_body, "</SN>");
	if(p_str_begin && p_str_end)
	{
	    memcpy(xml_cmd_sn, p_str_begin + 4, p_str_end - p_str_begin - 4); /*保存<SN>到xml_cmd_sn*/
	    printf("<SN>:%s\r\n", xml_cmd_sn);
	}

    p_str_begin = strstr(p_xml_body, "<DeviceID>");/*查找字符串"<DeviceID>"*/
    p_str_end   = strstr(p_xml_body, "</DeviceID>");
	if(p_str_begin && p_str_end)
	{
	    memcpy(xml_device_id, p_str_begin + 10, p_str_end - p_str_begin - 10); /*保存<DeviceID>到xml_device_id*/
	    printf("<DeviceID>:%s\r\n", xml_device_id);
	}

    printf("***********CMD END***********\r\n");

    if (0 == strcmp(xml_cmd_type, "DeviceControl"))/*设备控制*/
    {
        printf("**********CONTROL START**********\r\n");
        /*向左、向右、向上、向下、放大、缩小、停止遥控*/
        p_str_begin = strstr(p_xml_body, "<PTZCmd>");/*查找字符串"<PTZCmd>"*/
        if (NULL != p_str_begin)
        {
            p_str_end = strstr(p_xml_body, "</PTZCmd>");
            memcpy(xml_command, p_str_begin + 8, p_str_end - p_str_begin - 8); /*保存<PTZCmd>到xml_command*/
            printf("<PTZCmd>:%s\r\n", xml_command);
            goto DeviceControl_Next;
        }
        /*拉框放大控制命令*/
        p_str_begin = strstr(p_xml_body, "<DragZoomIn>");/*查找字符串"<DragZoomIn>"*/
        if (NULL != p_str_begin)
        {
            p_str_end = strstr(p_xml_body, "</DragZoomIn>");
            memcpy(xml_command, p_str_begin + 12, p_str_end - p_str_begin - 12); /*保存<DragZoomIn>到xml_command*/
            printf("<PTZCmd>:%s\r\n", xml_command);
            return ;/*源设备向目标设备发送球机/云台控制命令、 远程启动命令、 强制关键帧、 拉框放大、 拉框缩小命令后, 目标设备不发送应答命令*/
            //goto DeviceControl_Next;
        }
        /*拉框缩小控制命令*/
        p_str_begin = strstr(p_xml_body, "<DragZoomOut>");/*查找字符串"<DragZoomOut>"*/
        if (NULL != p_str_begin)
        {
            p_str_end = strstr(p_xml_body, "</DragZoomOut>");
            memcpy(xml_command, p_str_begin + 13, p_str_end - p_str_begin - 13); /*保存<DragZoomOut>到xml_command*/
            printf("<PTZCmd>:%s\r\n", xml_command);
            return ;/*源设备向目标设备发送球机/云台控制命令、 远程启动命令、 强制关键帧、 拉框放大、 拉框缩小命令后, 目标设备不发送应答命令*/
            //goto DeviceControl_Next;
        }

        /*开始手动录像、停止手动录像*/
        p_str_begin = strstr(p_xml_body, "<RecordCmd>");/*查找字符串"<RecordCmd>"*/
        if (NULL != p_str_begin)
        {
            p_str_end = strstr(p_xml_body, "</RecordCmd>");
            memcpy(xml_command, p_str_begin + 11, p_str_end - p_str_begin - 11); /*保存<RecordCmd>到xml_command*/
            printf("<RecordCmd>:%s\r\n", xml_command);
            goto DeviceControl_Next;
        }
        /*布防、撤防*/
        p_str_begin = strstr(p_xml_body, "<GuardCmd>");/*查找字符串"<GuardCmd>"*/
        if (NULL != p_str_begin)
        {
            p_str_end = strstr(p_xml_body, "</GuardCmd>");
            memcpy(xml_command, p_str_begin + 10, p_str_end - p_str_begin - 10); /*保存<GuardCmd>到xml_command*/
            printf("<GuardCmd>:%s\r\n", xml_command);
            goto DeviceControl_Next;
        }
        /*报警复位：30秒内不再触发报警*/
        p_str_begin = strstr(p_xml_body, "<AlarmCmd>");/*查找字符串"<AlarmCmd>"*/
        if (NULL != p_str_begin)
        {
            p_str_end = strstr(p_xml_body, "</AlarmCmd>");
            memcpy(xml_command, p_str_begin + 10, p_str_end - p_str_begin - 10); /*保存<AlarmCmd>到xml_command*/
            printf("<AlarmCmd>:%s\r\n", xml_command);
            goto DeviceControl_Next;
        }
        /*设备远程启动*/
        p_str_begin = strstr(p_xml_body, "<TeleBoot>");/*查找字符串"<TeleBoot>"*/
        if (NULL != p_str_begin)
        {
            p_str_end = strstr(p_xml_body, "</TeleBoot>");
            memcpy(xml_command, p_str_begin + 10, p_str_end - p_str_begin - 10); /*保存<TeleBoot>到xml_command*/
            printf("<TeleBoot>:%s\r\n", xml_command);
            goto DeviceControl_Next;
        }
        p_str_begin = strstr(p_xml_body, "<IFameCmd>");/*查找字符串"<IFameCmd>"*/
        if (NULL != p_str_begin)
        {
            p_str_end = strstr(p_xml_body, "</IFameCmd>");
            memcpy(xml_command, p_str_begin + 10, p_str_end - p_str_begin - 10); /*保存<IFameCmd>到xml_command*/
            printf("<IFameCmd>:%s\r\n", xml_command);
            goto DeviceControl_Next;
        }
        p_str_begin = strstr(p_xml_body, "<HomePosition>");/*查找字符串"<HomePosition>"*/
        if (NULL != p_str_begin)
        {
            p_str_end = strstr(p_xml_body, "</HomePosition>");
            memcpy(xml_command, p_str_begin + 14, p_str_end - p_str_begin - 14); /*保存<HomePosition>到xml_command*/
            printf("<HomePosition>:%s\r\n", xml_command);
            goto DeviceControl_Next;
        }
DeviceControl_Next:
		printf("***********CONTROL END***********\r\n");

		if(ite_eXosip_ProcessPtzCmd(xml_command) == 0)
			return;
#if 0
		/*源设备向目标设备发送球机/云台控制命令、 远程启动命令、 强制关键帧、 拉框放大、 拉框缩小命令后, 目标设备不发送应答命令*/
		if (0 == strncmp(xml_command, "A50F0002",8))/*向左 A50F0002230000D9*/
		{
			//ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_RMT_LEFT);
			printf("netcam_ptz_left!\n");
			netcam_ptz_left(step, speed);
			return;
		}
		else if (0 == strncmp(xml_command, "A50F0001",8))/*向右 A50F0001230000D8*/
		{
			//ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_RMT_RIGHT);
			printf("netcam_ptz_right!\n");
			netcam_ptz_right(step, speed);
			return;
		}
		else if (0 == strncmp(xml_command, "A50F0008",8))/*向上 A50F0008002300DF*/
		{
			//ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_RMT_UP);
			printf("netcam_ptz_up!\n");
			netcam_ptz_up(step, speed);
			return;
		}
		else if (0 == strncmp(xml_command, "A50F0004",8))/*向下 A50F0004002300DB*/
		{
			//ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_RMT_DOWN);
			printf("netcam_ptz_down!\n");
			netcam_ptz_down(step, speed);
			return;
		}
		else if (0 == strncmp(xml_command, "A50F0005",8))/*右下 A50F0005232300FF*/
		{
			//ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_RMT_DOWN);
			printf("netcam_ptz_right_down!\n");
			netcam_ptz_right_down(step, speed);
			return;
		}
		else if (0 == strncmp(xml_command, "A50F0006",8))/*左下 A50F000623230000*/
		{
			//ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_RMT_DOWN);
			printf("netcam_ptz_left_down!\n");
			netcam_ptz_left_down(step, speed);
			return;
		}
		else if (0 == strncmp(xml_command, "A50F0009",8))/*右上 A50F000923230003*/
		{
			//ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_RMT_DOWN);
			printf("netcam_ptz_right_up!\n");
			netcam_ptz_right_up(step, speed);
			return;
		}
		else if (0 == strncmp(xml_command, "A50F000A",8))/*左上 A50F000A23230004*/
		{
			//ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_RMT_DOWN);
			printf("netcam_ptz_left_up!\n");
			netcam_ptz_left_up(step, speed);
			return;
		}
		else if (0 == strcmp(xml_command, "A50F0110000010D5"))/*放大*/
		{
			//ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_RMT_LARGE);
			return;
		}
		else if (0 == strcmp(xml_command, "A50F0120000010E5"))/*缩小*/
		{
			//ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_RMT_SMALL);
			return;
		}
		else if (0 == strncmp(xml_command, "A50F0000",8))/*停止遥控 A50F0000000000B4*/
		{
			//ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_RMT_STOP);
			printf("netcam_ptz_stop!\n");
			netcam_ptz_stop();
			return;
		}
#endif
        if (0 == strncmp(xml_command, "A50F0181", 8))/*设置预置位*/
        {
            int i = 0, ret;
            char preset[3] = {0};
            //ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_RMT_STOP);

            strncpy(preset, xml_command+10, 2);
            printf("set Preset :%s, %d\n", preset, strtoul(preset,NULL,16));
			#if 0
            for (i = 0; i < 255; i++)
            {
                if (1 == g_presetlist_param.preset_list[i].status)
                {
                    continue;
                }
                g_presetlist_param.preset_list[i].preset_id = atoi(preset);
                g_presetlist_param.preset_list[i].preset_name = atoi(preset);
                g_presetlist_param.preset_list[i].status = 1;
                g_presetlist_param.preset_num++;
                break;
            }
			#else
			GK_NET_PRESET_INFO   gkPresetCfg;
			i = strtoul(preset,NULL,16);
            if (i > PTZ_MAX_PRESET)
                i = PTZ_MAX_PRESET - 1;
            else if (i > 0)
                i -= 1;
            else
                i = 0;
            get_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            gkPresetCfg.nPresetNum++;
            if (gkPresetCfg.nPresetNum >= NETCAM_PTZ_MAX_PRESET_NUM)
            {
                gkPresetCfg.nPresetNum = NETCAM_PTZ_MAX_PRESET_NUM;
            }
            gkPresetCfg.no[gkPresetCfg.nPresetNum-1] = i;
            set_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            PresetCruiseCfgSave();
            if ((ret = netcam_ptz_set_preset(i, NULL)))
            {
                PRINT_ERR("call  DMS_PTZ_CMD_PRESET error!\n");
                return -1;
            }
			#endif

            return;
        }
        else if (0 == strncmp(xml_command, "A50F0182", 8))/*调用预置位*/
        {
            //ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_RMT_STOP);
            printf("use Preset\n");
			char preset[3] = {0};
            strncpy(preset, xml_command+10, 2);
			GK_NET_CRUISE_GROUP  cruise_info;
			int ret, num = strtoul(preset,NULL,16);
            if (num > PTZ_MAX_PRESET)
                num = PTZ_MAX_PRESET - 1;
            else if (num > 0)
                num -= 1;
            else
                num = 0;
            cruise_info.byPointNum    = 1;
            cruise_info.byCruiseIndex = 0;
            cruise_info.struCruisePoint[0].byPointIndex = 0;
            cruise_info.struCruisePoint[0].byPresetNo   = num;
            cruise_info.struCruisePoint[0].byRemainTime = 0;
            cruise_info.struCruisePoint[0].bySpeed      = -1;
            if ((ret = netcam_ptz_stop()))
            {
                PRINT_ERR("call  netcam_ptz_stop error!\n");
                return ;
            }
            if ((ret = netcam_ptz_preset_cruise(&cruise_info)))
            {
                PRINT_ERR("call  DMS_PTZ_CMD_AUTO_STRAT error!\n");
                return ;
            }
            return;
        }
        else if (0 == strncmp(xml_command, "A50F0183", 8))/*删除预置位*/
        {
            char preset[3] = {0};
            int i = 0;
            //ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_RMT_STOP);
            strncpy(preset, xml_command+10, 2);
			
            printf("del Preset %s\n", preset);
			#if 0
            for (i = 0; i < 255; i++)
            {
                if (1 == g_presetlist_param.preset_list[i].status)
                {
                    if (atoi(preset) == g_presetlist_param.preset_list[i].preset_id)
                    {
                        g_presetlist_param.preset_list[i].status = 0;
                        g_presetlist_param.preset_num--;
                    }
                }
            }
			#else
			GK_NET_PRESET_INFO   gkPresetCfg;
			int ret, num = strtoul(preset,NULL,16);
            if (num > PTZ_MAX_PRESET)
                num = PTZ_MAX_PRESET - 1;
            else if (num > 0)
                num -= 1;
            else
                num = 0;
            get_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            gkPresetCfg.nPresetNum--;
            if (gkPresetCfg.nPresetNum <= 0)
            {
                gkPresetCfg.nPresetNum = 0;
            }
            set_param(PTZ_PRESET_PARAM_ID, &gkPresetCfg);
            PresetCruiseCfgSave();
            if ((ret = netcam_ptz_clr_preset(num)))
            {
                PRINT_ERR("call  DMS_PTZ_CMD_PRESET_CLS error!\n");
                return -1;
            }
			#endif
            return;
        }
		else if (0 == strncmp(xml_command, "A50F0184", 8)) /* 将预置点加入巡航序列 */
        {
            char grp_idxs[3] = {0}, pot_idxs[3] = {0};
            int cnt = 0;
            //ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_RMT_STOP);
            strncpy(grp_idxs, xml_command+8, 2);
            strncpy(pot_idxs, xml_command+10, 2);
			
            printf("grp_idx:%s, pot_idx:%s\n", grp_idxs, pot_idxs);
			GK_NET_CRUISE_CFG	 gkCruiseCfg; 
            int grp_idx = strtoul(grp_idxs,NULL,16);
			int pot_idx = strtoul(pot_idxs,NULL,16);

			if (grp_idx > PTZ_MAX_CRUISE_GROUP_NUM)
                grp_idx = PTZ_MAX_CRUISE_GROUP_NUM - 1;
            else if (grp_idx > 0)
                grp_idx -= 1;
            else
                grp_idx = 0;

			if (pot_idx > 0)
                pot_idx -= 1;
            else
                pot_idx = 0;

            get_param(PTZ_CRUISE_PARAM_ID, &gkCruiseCfg);
			printf("gkCruiseCfg.struCruise[%d].byPointNum:%d\n", grp_idx,gkCruiseCfg.struCruise[grp_idx].byPointNum);
            if(gkCruiseCfg.struCruise[grp_idx].byPointNum < PTZ_MAX_CRUISE_POINT_NUM)
            {
                gkCruiseCfg.struCruise[grp_idx].byPointNum++;
            }
            else
            {
                gkCruiseCfg.struCruise[grp_idx].byPointNum = PTZ_MAX_CRUISE_POINT_NUM;
                PRINT_INFO("total preset point num:PTZ_MAX_CRUISE_POINT_NUM ,don't add preset point!\n");
            }

            /* 巡航组中的下标,如果值大于PTZ_MAX_CRUISE_POINT_NUM 表示添加到末尾 */
            if (pot_idx >= gkCruiseCfg.struCruise[grp_idx].byPointNum)
                pot_idx = gkCruiseCfg.struCruise[grp_idx].byPointNum - 1;
            for (cnt = gkCruiseCfg.struCruise[grp_idx].byPointNum -1; cnt > pot_idx; cnt--)
            {
                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt+1].byPointIndex =
                                            gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].byPointIndex;

                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt+1].byPresetNo   =
                                            gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].byPresetNo;

                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt+1].byRemainTime =
                                            gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].byRemainTime;

                gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt+1].bySpeed      =
                                            gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].bySpeed;
            }
            gkCruiseCfg.nChannel =0;
            gkCruiseCfg.struCruise[grp_idx].byCruiseIndex = grp_idx;
            gkCruiseCfg.struCruise[grp_idx].struCruisePoint[pot_idx].byPointIndex = pot_idx;
            gkCruiseCfg.struCruise[grp_idx].struCruisePoint[pot_idx].byPresetNo   = pot_idx;
            gkCruiseCfg.struCruise[grp_idx].struCruisePoint[pot_idx].byRemainTime = 0;
            gkCruiseCfg.struCruise[grp_idx].struCruisePoint[pot_idx].bySpeed      = -1;
			printf("gkCruiseCfg.struCruise[%d].byPointNum:%d\n", grp_idx,gkCruiseCfg.struCruise[grp_idx].byPointNum);

            set_param(PTZ_CRUISE_PARAM_ID, &gkCruiseCfg);
            PresetCruiseCfgSave();
            return;
        }
		else if (0 == strncmp(xml_command, "A50F0185", 8)) /* 删除一个巡航点 */
        {/*
        注1:字节5表示巡航组号,字节6表示预置位号。
		注2:序号2中,字节6为00H 时,删除对应的整条巡航;序号3、4中字节6表示数据的低8位,字节7的高4位
		表示数据的高4位。
		注3:巡航停留时间的单位是秒(s)。
		注4:停止巡航用PTZ指令中的字节4的各Bit位均为0的停止指令。
        */
            char grp_idxs[3] = {0}, pot_idxs[3] = {0};
            int cnt = 0;
            //ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_RMT_STOP);
            strncpy(grp_idxs, xml_command+8, 2);
            strncpy(pot_idxs, xml_command+10, 2);
			
            printf("grp_idx:%s, pot_idx:%s\n", grp_idxs, pot_idxs);
			GK_NET_CRUISE_CFG	 gkCruiseCfg; 
            int grp_idx = strtoul(grp_idxs,NULL,16);
			int pot_idx = strtoul(pot_idxs,NULL,16);
			if (grp_idx > PTZ_MAX_CRUISE_GROUP_NUM)
                grp_idx = PTZ_MAX_CRUISE_GROUP_NUM - 1;
            else if (grp_idx > 0)
                grp_idx -= 1;
            else
                grp_idx = 0;

			get_param(PTZ_CRUISE_PARAM_ID, &gkCruiseCfg);
			
			PRINT_INFO("DELETE	grp_idx:%d, byPointNum:%d, pot_idx:%d\n",
															grp_idx,
															gkCruiseCfg.struCruise[grp_idx].byPointNum,
															pot_idx);
			if(pot_idx == 0)
				gkCruiseCfg.struCruise[grp_idx].byPointNum = 0;
			else
				pot_idx -= 1;
			
			if (pot_idx >= PTZ_MAX_CRUISE_POINT_NUM)
			{
				PRINT_ERR("pot_idx:%d > PTZ_MAX_CRUISE_GROUP_NUM!\n", pot_idx);
				pot_idx = PTZ_MAX_CRUISE_POINT_NUM - 1;
			}

			gkCruiseCfg.nChannel = 0;
			if (pot_idx > gkCruiseCfg.struCruise[grp_idx].byPointNum)
			{
				gkCruiseCfg.struCruise[grp_idx].struCruisePoint[pot_idx].byPointIndex = 0;
				gkCruiseCfg.struCruise[grp_idx].struCruisePoint[pot_idx].byPresetNo   = 0;
				gkCruiseCfg.struCruise[grp_idx].struCruisePoint[pot_idx].byRemainTime = 0;
				gkCruiseCfg.struCruise[grp_idx].struCruisePoint[pot_idx].bySpeed	  = 0;
				return;
			}
			
			gkCruiseCfg.struCruise[grp_idx].byPointNum--;
			gkCruiseCfg.struCruise[grp_idx].byCruiseIndex = grp_idx;
			for (cnt = pot_idx; cnt < gkCruiseCfg.struCruise[grp_idx].byPointNum; cnt++)
			{
				gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].byPointIndex =
											gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt+1].byPointIndex;
			
				gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].byPresetNo   =
											gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt+1].byPresetNo;
			
				gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].byRemainTime =
											gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt+1].byRemainTime;
			
				gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].bySpeed	  =
											gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt+1].bySpeed;
			}
			
			pot_idx = gkCruiseCfg.struCruise[grp_idx].byPointNum;
			//printf("----------> pot_idx:%d \n", pot_idx);
			gkCruiseCfg.struCruise[grp_idx].struCruisePoint[pot_idx].byPointIndex = 0;
			gkCruiseCfg.struCruise[grp_idx].struCruisePoint[pot_idx].byPresetNo   = 0;
			gkCruiseCfg.struCruise[grp_idx].struCruisePoint[pot_idx].byRemainTime = 0;
			gkCruiseCfg.struCruise[grp_idx].struCruisePoint[pot_idx].bySpeed	  = 0;
			
			set_param(PTZ_CRUISE_PARAM_ID, &gkCruiseCfg);
			PresetCruiseCfgSave();
            return;
        }
		else if (0 == strncmp(xml_command, "A50F0186", 8)) /* 设置巡航速度 */
        {
            char grp_idxs[3] = {0}, h_speeds[3] = {0};
            int cnt = 0;
            //ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_RMT_STOP);
            strncpy(grp_idxs, xml_command+8, 2);
			strncpy(h_speeds, xml_command+12, 1);
            strncpy(h_speeds+1, xml_command+10, 2);
			
            printf("grp_idx:%s, speeds:%s\n", grp_idxs, h_speeds);
			GK_NET_CRUISE_CFG	 gkCruiseCfg; 
            int grp_idx = strtoul(grp_idxs,NULL,16);
			int h_speed = strtoul(h_speeds,NULL,16);
			if (grp_idx > PTZ_MAX_CRUISE_GROUP_NUM)
                grp_idx = PTZ_MAX_CRUISE_GROUP_NUM - 1;
            else if (grp_idx > 0)
                grp_idx -= 1;
            else
                grp_idx = 0;
			get_param(PTZ_CRUISE_PARAM_ID, &gkCruiseCfg);
			
			PRINT_INFO("set	grp_idx:%d, byPointNum:%d, speed:%d\n",
															grp_idx,
															gkCruiseCfg.struCruise[grp_idx].byPointNum,
															h_speed);
			for (cnt = 0; cnt < gkCruiseCfg.struCruise[grp_idx].byPointNum; cnt++)
			{
				gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].bySpeed = h_speed;
			}
			set_param(PTZ_CRUISE_PARAM_ID, &gkCruiseCfg);
			PresetCruiseCfgSave();
            //return;
        }
		else if (0 == strncmp(xml_command, "A50F0187", 8)) /* 设置巡航速度 */
        {
            char grp_idxs[3] = {0}, h_times[3] = {0};
            int cnt = 0;
            //ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_RMT_STOP);
            strncpy(grp_idxs, xml_command+8, 2);
			strncpy(h_times, xml_command+12, 1);
            strncpy(h_times+1, xml_command+10, 2);
			
            printf("grp_idx:%s, speeds:%s\n", grp_idxs, h_times);
			GK_NET_CRUISE_CFG	 gkCruiseCfg; 
            int grp_idx = strtoul(grp_idxs,NULL,16);
			int time = strtoul(h_times,NULL,16);
			if (grp_idx > PTZ_MAX_CRUISE_GROUP_NUM)
                grp_idx = PTZ_MAX_CRUISE_GROUP_NUM - 1;
            else if (grp_idx > 0)
                grp_idx -= 1;
            else
                grp_idx = 0;
			get_param(PTZ_CRUISE_PARAM_ID, &gkCruiseCfg);
			
			PRINT_INFO("set	grp_idx:%d, byPointNum:%d, byRemainTime:%d\n",
															grp_idx,
															gkCruiseCfg.struCruise[grp_idx].byPointNum,
															time);
			for (cnt = 0; cnt < gkCruiseCfg.struCruise[grp_idx].byPointNum; cnt++)
			{
				gkCruiseCfg.struCruise[grp_idx].struCruisePoint[cnt].byRemainTime = time;
			}
			set_param(PTZ_CRUISE_PARAM_ID, &gkCruiseCfg);
			PresetCruiseCfgSave();
            //return;
        }
		else if (0 == strncmp(xml_command, "A50F0188", 8)) /* 开始巡航 */
        {
            char grp_idxs[3] = {0};
            int ret, cnt = 0;
            //ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_RMT_STOP);
            strncpy(grp_idxs, xml_command+8, 2);
			
            printf("grp_idx:%s\n", grp_idxs);
			GK_NET_CRUISE_CFG	 gkCruiseCfg; 
            int grp_idx = strtoul(grp_idxs,NULL,16);
			if (grp_idx > PTZ_MAX_CRUISE_GROUP_NUM)
                grp_idx = PTZ_MAX_CRUISE_GROUP_NUM - 1;
            else if (grp_idx > 0)
                grp_idx -= 1;
            else
                grp_idx = 0;
			get_param(PTZ_CRUISE_PARAM_ID, &gkCruiseCfg);
			ret = netcam_ptz_preset_cruise(&gkCruiseCfg.struCruise[grp_idx]);
			if (ret < 0)
			{
				PRINT_ERR("Call netcam_ptz_preset_cruise error!\n");
				return -1;
			}
			return;
        }
        else if (0 == strcmp(xml_command, "Record"))/*开始手动录像*/
        {
            //ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_REC_START);
            printf("****Record****\r\n");
			manu_rec_start_alltime(runRecordCfg.stream_no, runRecordCfg.recordLen);
        }
        else if (0 == strcmp(xml_command, "StopRecord"))/*停止手动录像*/
        {
            //ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_REC_STOP);
            printf("****StopRecord****\r\n");
            manu_rec_stop(runRecordCfg.stream_no);
        }
        else if (0 == strcmp(xml_command, "SetGuard"))/*布防*/
        {
			int i = 0, j = 0;
			runMdCfg.enable = 1;
			for (i = 0; i < 7; i++) {
				for (j = 0; j < 4; j++) {
					runMdCfg.scheduleTime[i][j].startHour = 0;
					runMdCfg.scheduleTime[i][j].startMin  = 0;
					runMdCfg.scheduleTime[i][j].stopHour  = 23;
					runMdCfg.scheduleTime[i][j].stopMin   = 59;
				}
			}

			MdCfgSave();

            //ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_GUD_START);
            strcpy(device_status.status_guard, "ONDUTY");/*设置布防状态为"ONDUTY"*/
        }
        else if (0 == strcmp(xml_command, "ResetGuard"))/*撤防*/
        {
            //ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_GUD_STOP);

			runMdCfg.enable = 0;
			MdCfgSave();

            strcpy(device_status.status_guard, "OFFDUTY");/*设置布防状态为"OFFDUTY"*/
        }
        else if (0 == strcmp(xml_command, "ResetAlarm"))/*报警复位*/
        {
            //ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_ALM_RESET);
            strcpy(device_status.status_guard, "ONDUTY");/*设置布防状态为"ONDUTY"*/

            p_str_begin = strstr(p_xml_body, "<AlarmMethod>");/*查找字符串"<AlarmMethod>"*/
            if (NULL != p_str_begin)
            {
                p_str_end = strstr(p_xml_body, "</AlarmMethod>");
                memset(g_alarm_method, 0, 10);
                memcpy(g_alarm_method, p_str_begin + 13, p_str_end - p_str_begin - 13); /*保存<AlarmMethod>*/
                printf("<AlarmMethod>:%s\r\n", g_alarm_method);
            }
        }
        else if (0 == strcmp(xml_command, "Boot"))/*设备远程启动*/
        {
            //ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_TEL_BOOT);
			if(netcam_is_prepare_update() || (netcam_get_update_status() != 0))
			{
				printf("<<<<<<<Receive reboot CMD when upgrade!, Discard it!>>>>>>>>>\n");
				return;
			}

            uac_UNregister(excontext, SipConfig);
            //system("reboot");
            //netcam_exit(90);
			//new_system_call("reboot -f");//force REBOOT
			netcam_sys_operation(NULL,(void *)SYSTEM_OPERATION_REBOOT); //force REBOOT
            return;
        }
        else if (0 == strcmp(xml_command, "Send"))/*强制关键帧命令, 设备收到此命令应立刻发送一个IDR 帧*/
        {
            //ite_eXosip_callback.ite_eXosip_deviceControl(EXOSIP_CTRL_TEL_BOOT);
            printf("IFameCmd Send IDR\r\n");
            SetIFameFlag(1);

            return;
        }
        else
        {
            printf("unknown device control command!\r\n");
        }
        snprintf(rsp_xml_body, 4096, "<?xml version=\"1.0\"?>\r\n"
                 "<Response>\r\n"
                 "<CmdType>DeviceControl</CmdType>\r\n"/*命令类型*/
                 "<SN>%s</SN>\r\n"/*命令序列号*/
                 "<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统编码*/
                 "<Result>OK</Result>\r\n"/*执行结果标志*/
                 "</Response>\r\n",
                 xml_cmd_sn,
                 xml_device_id);
    }
    else if (0 == strcmp(xml_cmd_type, "PresetQuery"))
    {
        printf("**********PresetQuery START**********\r\n");
        /*设备信息查询*/
        printf("***********PresetQuery END***********\r\n");
        int i = 0;
        char temp_item_body[1024];
        memset(temp_item_body, 0, 1024);

        for (i = 0; i < 255; i++)
        {
            if (0 == g_presetlist_param.preset_list[i].status)
            {
                continue;
            }
            char temp_body[1024];
            memset(temp_body, 0, 1024);
            snprintf(temp_body, 1024, "<Item>\r\n"
                    "<PresetID>%d</PresetID>\r\n" /*预置位编码*/
                    "<PresetName>%d</PresetName>\r\n"/* 预置位名称*/
                    "</Item>\r\n",
                    g_presetlist_param.preset_list[i].preset_id,
                    g_presetlist_param.preset_list[i].preset_name);
            strcat(temp_item_body, temp_body);
        }

        snprintf(rsp_xml_body, 4096, "<?xml version=\"1.0\"  encoding=\"GB2312\" ?>\r\n"
                 "<Response>\r\n"
                 "<CmdType>PresetQuery</CmdType>\r\n"/*命令类型*/
                 "<SN>%s</SN>\r\n"/*命令序列号*/
                 "<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统的编码*/
                 "<PresetList Num=\"%d\">"
                 "%s"
                 "</PresetList>"
                 "</Response>\r\n",
                 xml_cmd_sn,
                 xml_device_id,
                 g_presetlist_param.preset_num,
                 temp_item_body);
    }
    else if (0 == strcmp(xml_cmd_type, "Alarm"))/*报警事件通知和分发：报警通知响应*/
    {
        printf("**********ALARM START**********\r\n");
        /*报警通知响应*/
        printf("local eventAlarm response success!\n");
        printf("***********ALARM END***********\r\n");
#if 0
        snprintf(rsp_xml_body, 4096, "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n"
                "<Response>\r\n"
                "<CmdType>Alarm</CmdType>\r\n"/*命令类型*/
                "<SN>%s</SN>\r\n"/*命令序列号*/
                "<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统的编码*/
                "<Result>OK</Result>\r\n"/*执行结果*/
                "</Response>\r\n",
                xml_cmd_sn,
                xml_device_id);
#endif
        return;
    }
    else if (0 == strcmp(xml_cmd_type, "Catalog"))/*网络设备信息查询：设备目录查询Catalog*/
    {
        printf("**********CATALOG START**********\r\n");
        /*设备目录查询*/
        printf("***********CATALOG END***********\r\n");
        char temp_item_body[2048];
        char temp_body[1024];
        char civil_code[7];

        memset(civil_code, 0, 7);
        strncpy(civil_code, xml_device_id, 6);
        memset(temp_item_body, 0, 2048);
        memset(temp_body, 0, 1024);
        snprintf(temp_body, 1024, "<Item>\r\n"
             "<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统的编码*/
             "<Name>%s</Name>\r\n"/*设备/区域/系统名称*/
             "<Manufacturer>%s</Manufacturer>\r\n"/*设备厂商*/
             "<Model>%s</Model>\r\n"/*设备型号*/
             "<Owner>0</Owner>\r\n"/*设备归属*/
             "<CivilCode>%s</CivilCode>\r\n"/*行政区域*/
             /*"<Block>Block1</Block>\r\n"*//*警区*/
             /*"<Address>Address1</Address>\r\n"*//*安装地址*/
             "<Parental>0</Parental>\r\n"/*是否有子设备*/
             "<ParentID>%s</ParentID>\r\n"/*父设备/区域/系统ID*/
             "<SafetyWay>0</SafetyWay>\r\n"/*信令安全模式/0为不采用/2为S/MIME签名方式/3为S/MIME加密签名同时采用方式/4为数字摘要方式*/
             "<RegisterWay>1</RegisterWay>\r\n"/*注册方式/1为符合sip3261标准的认证注册模式/2为基于口令的双向认证注册模式/3为基于数字证书的双向认证注册模式*/
#if 0
             /*"<CertNum>CertNum1</CertNum>\r\n"*//*证书序列号*/
             "<Certifiable>0</Certifiable>\r\n"/*证书有效标识/0为无效/1为有效*/
             "<ErrCode>400</ErrCode>\r\n"/*无效原因码*/
             "<EndTime>2099-12-31T23:59:59</EndTime>\r\n"/*证书终止有效期*/
#endif
             "<Secrecy>0</Secrecy>\r\n"/*保密属性/0为不涉密/1为涉密*/
             "<IPAddress>%s</IPAddress>\r\n"/*设备/区域/系统IP地址*/
             "<Port>%d</Port>\r\n"/*设备/区域/系统端口*/
             "<Password>null</Password>\r\n"/*设备口令*/
             "<Status>ON</Status>\r\n"/*设备状态*/
#if 0
             "<Longitude>171.3</Longitude>\r\n"/*经度*/
             "<Latitude>34.2</Latitude>\r\n"/*纬度*/
#endif
             "</Item>\r\n",
             xml_device_id,
             device_info.device_name,
             device_info.device_manufacturer,
             device_info.device_model,
             civil_code,
             xml_device_id,
             localip,
             SipConfig->sipDevicePort);
        strcat(temp_item_body, temp_body);

        #if 0
        memset(temp_body, 0, 1024);
        snprintf(temp_body, 1024, "<Item>\r\n"
             "<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统的编码*/
             "<Name>%s1</Name>\r\n"/*设备/区域/系统名称*/
             "<Manufacturer>%s</Manufacturer>\r\n"/*设备厂商*/
             "<Model>%s</Model>\r\n"/*设备型号*/
             "<Owner>0</Owner>\r\n"/*设备归属*/
             "<CivilCode>%s</CivilCode>\r\n"/*行政区域*/
             /*"<Block>Block1</Block>\r\n"*//*警区*/
             /*"<Address>Address1</Address>\r\n"*//*安装地址*/
             "<Parental>0</Parental>\r\n"/*是否有子设备*/
             "<ParentID>%s</ParentID>\r\n"/*父设备/区域/系统ID*/
             "<SafetyWay>0</SafetyWay>\r\n"/*信令安全模式/0为不采用/2为S/MIME签名方式/3为S/MIME加密签名同时采用方式/4为数字摘要方式*/
             "<RegisterWay>1</RegisterWay>\r\n"/*注册方式/1为符合sip3261标准的认证注册模式/2为基于口令的双向认证注册模式/3为基于数字证书的双向认证注册模式*/
#if 0
             /*"<CertNum>CertNum1</CertNum>\r\n"*//*证书序列号*/
             "<Certifiable>0</Certifiable>\r\n"/*证书有效标识/0为无效/1为有效*/
             "<ErrCode>400</ErrCode>\r\n"/*无效原因码*/
             "<EndTime>2099-12-31T23:59:59</EndTime>\r\n"/*证书终止有效期*/
#endif
             "<Secrecy>0</Secrecy>\r\n"/*保密属性/0为不涉密/1为涉密*/
             "<IPAddress>%s</IPAddress>\r\n"/*设备/区域/系统IP地址*/
             "<Port>%d</Port>\r\n"/*设备/区域/系统端口*/
             "<Password>null</Password>\r\n"/*设备口令*/
             "<Status>ON</Status>\r\n"/*设备状态*/
#if 0
             "<Longitude>171.3</Longitude>\r\n"/*经度*/
             "<Latitude>34.2</Latitude>\r\n"/*纬度*/
#endif
             "</Item>\r\n",
             device_info.ipc_alarmid,
             device_info.device_name,
             device_info.device_manufacturer,
             device_info.device_model,
             civil_code,
             xml_device_id,
             localip,
             SipConfig->sipDevicePort);
        strcat(temp_item_body, temp_body);

        memset(temp_body, 0, 1024);
        snprintf(temp_body, 1024, "<Item>\r\n"
             "<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统的编码*/
             "<Name>%s2</Name>\r\n"/*设备/区域/系统名称*/
             "<Manufacturer>%s</Manufacturer>\r\n"/*设备厂商*/
             "<Model>%s</Model>\r\n"/*设备型号*/
             "<Owner>0</Owner>\r\n"/*设备归属*/
             "<CivilCode>%s</CivilCode>\r\n"/*行政区域*/
             /*"<Block>Block1</Block>\r\n"*//*警区*/
             /*"<Address>Address1</Address>\r\n"*//*安装地址*/
             "<Parental>0</Parental>\r\n"/*是否有子设备*/
             "<ParentID>%s</ParentID>\r\n"/*父设备/区域/系统ID*/
             "<SafetyWay>0</SafetyWay>\r\n"/*信令安全模式/0为不采用/2为S/MIME签名方式/3为S/MIME加密签名同时采用方式/4为数字摘要方式*/
             "<RegisterWay>1</RegisterWay>\r\n"/*注册方式/1为符合sip3261标准的认证注册模式/2为基于口令的双向认证注册模式/3为基于数字证书的双向认证注册模式*/
#if 0
             /*"<CertNum>CertNum1</CertNum>\r\n"*//*证书序列号*/
             "<Certifiable>0</Certifiable>\r\n"/*证书有效标识/0为无效/1为有效*/
             "<ErrCode>400</ErrCode>\r\n"/*无效原因码*/
             "<EndTime>2099-12-31T23:59:59</EndTime>\r\n"/*证书终止有效期*/
#endif
             "<Secrecy>0</Secrecy>\r\n"/*保密属性/0为不涉密/1为涉密*/
             "<IPAddress>%s</IPAddress>\r\n"/*设备/区域/系统IP地址*/
             "<Port>%d</Port>\r\n"/*设备/区域/系统端口*/
             "<Password>null</Password>\r\n"/*设备口令*/
             "<Status>ON</Status>\r\n"/*设备状态*/
#if 0
             "<Longitude>171.3</Longitude>\r\n"/*经度*/
             "<Latitude>34.2</Latitude>\r\n"/*纬度*/
#endif
             "</Item>\r\n",
             device_info.ipc_voiceid,
             device_info.device_name,
             device_info.device_manufacturer,
             device_info.device_model,
             civil_code,
             xml_device_id,
             localip,
             SipConfig->sipDevicePort);
        strcat(temp_item_body, temp_body);
        #endif

        snprintf(rsp_xml_body, 4096, "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n"
                 "<Response>\r\n"
                 "<CmdType>Catalog</CmdType>\r\n"/*命令类型*/
                 "<SN>%s</SN>\r\n"/*命令序列号*/
                 "<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统的编码*/
                 "<SumNum>1</SumNum>\r\n"/*查询结果总数*/
                 "<DeviceList Num=\"1\">\r\n"/*设备目录项列表*/
                 "%s"
                 "</DeviceList>\r\n"
                 "</Response>\r\n",
                 xml_cmd_sn,
                 xml_device_id,
                 temp_item_body
                 );
    }
    else if (0 == strcmp(xml_cmd_type, "DeviceInfo"))/*网络设备信息查询：设备信息查询*/
    {
        printf("**********DEVICE INFO START**********\r\n");
        /*设备信息查询*/
        printf("***********DEVICE INFO END***********\r\n");
        snprintf(rsp_xml_body, 4096, "<?xml version=\"1.0\"  encoding=\"GB2312\" ?>\r\n"
                 "<Response>\r\n"
                 "<CmdType>DeviceInfo</CmdType>\r\n"/*命令类型*/
                 "<SN>%s</SN>\r\n"/*命令序列号*/
                 "<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统的编码*/
                 "<Result>OK</Result>\r\n"/*查询结果*/
                 "<Manufacturer>%s</Manufacturer>\r\n"/*设备生产商*/
                 "<Model>%s</Model>\r\n"/*设备型号*/
                 "<Firmware>%s</Firmware>\r\n"/*设备固件版本*/
                 "<Channel>3</Channel>\r\n"/*< 视频输入通道数(可选)*/
                 "</Response>\r\n",
                 xml_cmd_sn,
                 xml_device_id,
                 device_info.device_manufacturer,
                 device_info.device_model,
                 device_info.device_firmware);
    }
    else if (0 == strcmp(xml_cmd_type, "DeviceStatus"))/*网络设备信息查询：设备状态查询*/
    {
        printf("**********DEVICE STATUS START**********\r\n");
        /*设备状态查询*/
        printf("***********DEVICE STATUS END***********\r\n");
        char xml_status_guard[10];
        strcpy(xml_status_guard, device_status.status_guard);/*保存当前布防状态*/
        //    ite_eXosip_callback.ite_eXosip_getDeviceStatus(&device_status);/*获取设备当前状态*/
        strcpy(device_status.status_guard, xml_status_guard);/*恢复当前布防状态*/
        snprintf(rsp_xml_body, 4096, "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n"
                 "<Response>\r\n"
                 "<CmdType>DeviceStatus</CmdType>\r\n"/*命令类型*/
                 "<SN>%s</SN>\r\n"/*命令序列号*/
                 "<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统的编码*/
                 "<Result>OK</Result>\r\n"/*查询结果标志*/
                 "<Online>%s</Online>\r\n"/*是否在线/ONLINE/OFFLINE*/
                 "<Status>%s</Status>\r\n"/*是否正常工作*/
                 "<Encode>%s</Encode>\r\n"/*是否编码*/
                 "<Record>%s</Record>\r\n"/*是否录像*/
                 "<DeviceTime>%s</DeviceTime>\r\n"/*设备时间和日期*/
                 "<Alarmstatus Num=\"1\">\r\n"/*报警设备状态列表*/
                 "<Item>\r\n"
                 "<DeviceID>%s</DeviceID>\r\n"/*报警设备编码*/
                 "<DutyStatus>%s</DutyStatus>\r\n"/*报警设备状态*/
                 "</Item>\r\n"
                 "</Alarmstatus>\r\n"
                 "</Response>\r\n",
                 xml_cmd_sn,
                 xml_device_id,
                 device_status.status_online,
                 device_status.status_ok,
                 device_info.device_encode,
                 device_info.device_record,
                 device_status.status_time,
                 device_info.ipc_alarmid,
                 device_status.status_guard);
    }
    else if (0 == strcmp(xml_cmd_type, "RecordInfo"))/*设备视音频文件检索*/
    {
        /*录像文件检索*/
        char xml_file_path[30];
        char xml_start_time[30];
        char xml_end_time[30];
        char xml_recorder_id[30];
        char item_start_time[30];
        char item_end_time[30];
        char rsp_item_body[4096];
        int  record_list_num = 0;
        //int  record_list_ret = 0;

        memset(xml_file_path, 0, 30);
        memset(xml_start_time, 0, 30);
        memset(xml_end_time, 0, 30);
        memset(xml_recorder_id, 0, 30);
        memset(item_start_time, 0, 30);
        memset(item_end_time, 0, 30);
        memset(rsp_item_body, 0, 4096);
        printf("**********RECORD INFO START**********\r\n");
        p_str_begin = strstr(p_xml_body, "<FilePath>");/*查找字符串"<FilePath>"*/
        p_str_end   = strstr(p_xml_body, "</FilePath>");
		if(p_str_begin && p_str_end)
		{
	        memcpy(xml_file_path, p_str_begin + 10, p_str_end - p_str_begin - 10); /*保存<FilePath>到xml_file_path*/
	        printf("<FilePath>:%s\r\n", xml_file_path);
		}

        p_str_begin = strstr(p_xml_body, "<StartTime>");/*查找字符串"<StartTime>"*/
        p_str_end   = strstr(p_xml_body, "</StartTime>");
		if(p_str_begin && p_str_end)
		{
	        memcpy(xml_start_time, p_str_begin + 11, p_str_end - p_str_begin - 11); /*保存<StartTime>到xml_start_time*/
	        printf("<StartTime>:%s\r\n", xml_start_time);
		}

        p_str_begin = strstr(p_xml_body, "<EndTime>");/*查找字符串"<EndTime>"*/
        p_str_end   = strstr(p_xml_body, "</EndTime>");
		if(p_str_begin && p_str_end)
		{
	        memcpy(xml_end_time, p_str_begin + 9, p_str_end - p_str_begin - 9); /*保存<EndTime>到xml_end_time*/
	        printf("<EndTime>:%s\r\n", xml_end_time);
		}

        p_str_begin = strstr(p_xml_body, "<RecorderID>");/*查找字符串"<RecorderID>"*/
        p_str_end   = strstr(p_xml_body, "</RecorderID>");

		if(p_str_begin && p_str_end)
		{
	        memcpy(xml_recorder_id, p_str_begin + 12, p_str_end - p_str_begin - 12); /*保存<RecorderID>到xml_recorder_id*/
	        printf("<RecorderID>:%s\r\n", xml_recorder_id);
		}

        printf("***********RECORD INFO END***********\r\n");
#if 0
        for (;;)
        {
            //record_list_ret = ite_eXosip_callback.ite_eXosip_getRecordTime(xml_start_time, xml_end_time, item_start_time, item_end_time);
            if (0 > record_list_ret)
            {
                break;
            }
            else
            {
                char temp_body[1024];
                memset(temp_body, 0, 1024);
                snprintf(temp_body, 1024, "<Item>\r\n"
                         "<DeviceID>%s</DeviceID>\r\n"/*设备/区域编码*/
                         "<Name>%s</Name>\r\n"/*设备/区域名称*/
                         "<FilePath>%s</FilePath>\r\n"/*文件路径名*/
                         "<Address>Address1</Address>\r\n"/*录像地址*/
                         "<StartTime>%s</StartTime>\r\n"/*录像开始时间*/
                         "<EndTime>%s</EndTime>\r\n"/*录像结束时间*/
                         "<Secrecy>0</Secrecy>\r\n"/*保密属性/0为不涉密/1为涉密*/
                         "<Type>time</Type>\r\n"/*录像产生类型*/
                         "<RecorderID>%s</RecorderID>\r\n"/*录像触发者ID*/
                         "</Item>\r\n",
                         xml_device_id,
                         device_info.device_name,
                         xml_file_path,
                         item_start_time,
                         item_end_time,
                         xml_recorder_id);
                strcat(rsp_item_body, temp_body);
                record_list_num ++;
                if (0 == record_list_ret)
                {
                    break;
                }
            }
        }
        if (0 >= record_list_num)/*未检索到任何设备视音频文件*/
        {
            return;
        }
        snprintf(rsp_xml_body, 4096, "<?xml version=\"1.0\"?>\r\n"
                 "<Response>\r\n"
                 "<CmdType>RecordInfo</CmdType>\r\n"/*命令类型*/
                 "<SN>%s</SN>\r\n"/*命令序列号*/
                 "<DeviceID>%s</DeviceID>\r\n"/*设备/区域编码*/
                 "<Name>%s</Name>\r\n"/*设备/区域名称*/
                 "<SumNum>%d</SumNum>\r\n"/*查询结果总数*/
                 "<RecordList Num=\"%d\">\r\n"/*文件目录项列表*/
                 "%s\r\n"
                 "</RecordList>\r\n"
                 "</Response>\r\n",
                 xml_cmd_sn,
                 xml_device_id,
                 device_info.device_name,
                 record_list_num,
                 record_list_num,
                 rsp_item_body);
#else
		#define MAX_FILE_CNT 		60*24
		#define MAX_FILE_PER_MSG 	8

		TIME_RECORD_T *timeInfo = NULL;
		int i=0,file_cnt=0,total_cnt=0,send_cnt=0,offset = 0;
		char temp_body[1024];

		timeInfo = malloc(sizeof(TIME_RECORD_T)*MAX_FILE_CNT);
		if(timeInfo == NULL)
		{
			printf("%s,LINE:%d,malloc failed\n",__FUNCTION__,__LINE__);
			return;
		}

		memset(timeInfo,0,sizeof(TIME_RECORD_T)*MAX_FILE_CNT);

		file_cnt = gk_gb28181_playback_get_record_time(xml_start_time, xml_end_time, timeInfo);
		if(file_cnt <=0)
		{

			memset(rsp_xml_body, 0, 4096);
			snprintf(rsp_xml_body, 4096, "<?xml version=\"1.0\"?>\r\n"
										 "<Response>\r\n"
										 "<CmdType>RecordInfo</CmdType>\r\n"/*命令类型*/
										 "<SN>%s</SN>\r\n"/*命令序列号*/
										 "<DeviceID>%s</DeviceID>\r\n"/*设备/区域编码*/
										 "<Name>%s</Name>\r\n"/*设备/区域名称*/
										 "<SumNum>%d</SumNum>\r\n"/*查询结果总数*/
										 "<RecordList Num=\"%d\">\r\n"/*文件目录项列表*/
										 "</RecordList>\r\n"
										 "</Response>\r\n",
										 xml_cmd_sn,
										 xml_device_id,
										 device_info.device_name,
										 0,
										 0);
            if (rsp_msg == NULL)
            {
                eXosip_message_build_request(excontext,&rsp_msg, "MESSAGE", to, from, NULL);
            }
			osip_message_set_body(rsp_msg, rsp_xml_body, strlen(rsp_xml_body));
			osip_message_set_content_type(rsp_msg, "Application/MANSCDP+xml");
            eXosip_lock (excontext);
            eXosip_message_send_request(excontext,rsp_msg);/*回复"MESSAGE"请求*/
            eXosip_unlock (excontext);
            rsp_msg = NULL;
			memset(rsp_item_body, 0, 4096);
			free(timeInfo);
			return;
		}

		total_cnt = file_cnt;

		while(file_cnt > 0)
		{

			send_cnt = file_cnt>MAX_FILE_PER_MSG?MAX_FILE_PER_MSG:file_cnt;
			record_list_num = 0;

			for(i=0; i<send_cnt; i++)
			{
				memset(item_start_time,0,sizeof(item_start_time));
				memset(item_end_time,0,sizeof(item_end_time));
				memset(temp_body, 0, 1024);

				gk_gb28181_u64t_to_time(timeInfo[offset+i].start_time,item_start_time);
				gk_gb28181_u64t_to_time(timeInfo[offset+i].end_time,item_end_time);

#if 0
				snprintf(temp_body, 1024, "<Item>\r\n"
										  "<DeviceID>%s</DeviceID>\r\n"/*设备/区域编码*/
										  "<Name>%s</Name>\r\n"/*设备/区域名称*/
										  "<FilePath>%s</FilePath>\r\n"/*文件路径名*/
										  "<Address>Address1</Address>\r\n"/*录像地址*/
										  "<StartTime>%s</StartTime>\r\n"/*录像开始时间*/
										  "<EndTime>%s</EndTime>\r\n"/*录像结束时间*/
										  "<Secrecy>0</Secrecy>\r\n"/*保密属性/0为不涉密/1为涉密*/
										  "<Type>time</Type>\r\n"/*录像产生类型*/
										  "<RecorderID>%s</RecorderID>\r\n"/*录像触发者ID*/
										  "</Item>\r\n",
										  xml_device_id,
										  device_info.device_name,
										  xml_file_path,
										  item_start_time,
										  item_end_time,
										  xml_recorder_id);
#else
				snprintf(temp_body, 1024, "<Item>\r\n"
										  "<DeviceID>%s</DeviceID>\r\n"/*设备/区域编码*/
										  "<Name>%s</Name>\r\n"/*设备/区域名称*/
										  "<StartTime>%s</StartTime>\r\n"/*录像开始时间*/
										  "<EndTime>%s</EndTime>\r\n"/*录像结束时间*/
										  "<Secrecy>0</Secrecy>\r\n"/*保密属性/0为不涉密/1为涉密*/
										  "<Type>all</Type>\r\n"/*录像产生类型*/
										  "</Item>\r\n",
										  xml_device_id,
										  device_info.device_name,
										  item_start_time,
										  item_end_time
										  );



#endif
				strcat(rsp_item_body, temp_body);
				record_list_num ++;
			}

			memset(rsp_xml_body, 0, 4096);
			snprintf(rsp_xml_body, 4096, "<?xml version=\"1.0\"?>\r\n"
										 "<Response>\r\n"
										 "<CmdType>RecordInfo</CmdType>\r\n"/*命令类型*/
										 "<SN>%s</SN>\r\n"/*命令序列号*/
										 "<DeviceID>%s</DeviceID>\r\n"/*设备/区域编码*/
										 "<Name>%s</Name>\r\n"/*设备/区域名称*/
										 "<SumNum>%d</SumNum>\r\n"/*查询结果总数*/
										 "<RecordList Num=\"%d\">\r\n"/*文件目录项列表*/
										 "%s\r\n"
										 "</RecordList>\r\n"
										 "</Response>\r\n",
										 xml_cmd_sn,
										 xml_device_id,
										 device_info.device_name,
										 total_cnt,
										 record_list_num,
										 rsp_item_body);
            if (rsp_msg == NULL)
            {
                eXosip_message_build_request(excontext,&rsp_msg, "MESSAGE", to, from, NULL);
            }
			osip_message_set_body(rsp_msg, rsp_xml_body, strlen(rsp_xml_body));
			osip_message_set_content_type(rsp_msg, "Application/MANSCDP+xml");

			file_cnt -= send_cnt;
			offset += send_cnt;
            eXosip_lock (excontext);
            eXosip_message_send_request(excontext,rsp_msg);/*回复"MESSAGE"请求*/
            eXosip_unlock (excontext);
            //printf("osip send 0x%x\n", rsp_msg);
            rsp_msg = NULL;

			memset(rsp_item_body, 0, 4096);
		}

		free(timeInfo);
		return;
#endif

    }
    else if (0 == strcmp(xml_cmd_type, "ConfigDownload"))/*配置参数下载及设备参数能力*/
    {
        /*查询配置参数类型*/
        char xml_config_type[100]; /*可能出现BasicParam/VideoParamOpt/SVACEncodeConfig/SVACDecodeConfig组合形式, 可返回与查询SN 值相同的多个响应, 每个响应对应一个配置类型*/
        char rsp_item_body[4096];

        memset(xml_config_type, 0, 100);

        printf("**********Config  Download START **********\r\n");
        p_str_begin = strstr(p_xml_body, "<ConfigType>");/*查找字符串"<ConfigType>"*/
        p_str_end   = strstr(p_xml_body, "</ConfigType>");
        memcpy(xml_config_type, p_str_begin + 12, p_str_end - p_str_begin - 12); /*保存<ConfigType>到xml_config_type*/
        printf("<ConfigType>:%s\r\n", xml_config_type);
        printf("**********Config  Download END**********\r\n");

        if (NULL != strstr(xml_config_type, "BasicParam"))/*基本参数类型*/
        {
            memset(rsp_item_body, 0, 4096);
            memset(rsp_xml_body, 0, 4096);

            snprintf(rsp_item_body, 4096, "<BasicParam>\r\n"
                    "<Name>%s</Name>\r\n"/*设备名称*/
                    "<Expiration>%d</Expiration>\r\n"/*注册过期时间*/
                    "<HeartBeatInterval>%d</HeartBeatInterval>\r\n"/*心跳间隔时间*/
                    "<HeartBeatCount>%d</HeartBeatCount>\r\n"/*心跳超时次数*/
                    "<PositionCapability>%d</PositionCapability>\r\n"/*定位功能支持情况。 取值:0-不支持;1-支持 GPS 定位;2-支持北斗定位默认取值为0*/
                    "<Longitude>0.0000000</Longitude>\r\n"/*经度*/
                    "<Latitude>0.0000000</Latitude>\r\n"/*纬度*/
                    "</BasicParam>\r\n",
                    device_info.device_name,
                    SipConfig->regExpire_s,
                    SipConfig->sipDevHBCycle_s,
                    SipConfig->sipDevHBOutTimes,
                    0);
            snprintf(rsp_xml_body, 4096, "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n"
                 "<Response>\r\n"
                 "<CmdType>ConfigDownload</CmdType>\r\n"/*命令类型*/
                 "<SN>%s</SN>\r\n"/*命令序列号*/
                 "<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统的编码*/
                 "<Result>OK</Result>\r\n"/*查询结果标志*/
                 "%s"
                 "</Response>\r\n",
                 xml_cmd_sn,
                 xml_device_id,
                 rsp_item_body);
            eXosip_message_build_request(excontext,&rsp_msg, "MESSAGE", to, from, NULL);/*构建"MESSAGE"请求*/
            osip_message_set_body(rsp_msg, rsp_xml_body, strlen(rsp_xml_body));
            osip_message_set_content_type(rsp_msg, "Application/MANSCDP+xml");
            eXosip_lock (excontext);
            eXosip_message_send_request(excontext,rsp_msg);/*回复"MESSAGE"请求*/
            eXosip_unlock (excontext);
            printf("eXosip_message_send_request success!\r\n");
        }
        if (NULL != strstr(xml_config_type, "VideoParamOpt"))/*视频参数范围*/
        {
            memset(rsp_item_body, 0, 4096);
            memset(rsp_xml_body, 0, 4096);

            snprintf(rsp_item_body, 4096, "<VideoParamOpt>\r\n"
                    "<DownloadSpeed>1/2/4</DownloadSpeed>\r\n"/*下载倍速范围, 如设备支持1,2,4 倍速下载则应写为“1/2/4”*/
                    "<Resolution>v/3/6/24/1/38000a/1/38000/5/1</Resolution>\r\n"/*摄像机支持的分辨率,分辨率取值参见附录 F 中SDPf 字段规定*/
                    "</VideoParamOpt>\r\n");
            snprintf(rsp_xml_body, 4096, "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n"
                 "<Response>\r\n"
                 "<CmdType>ConfigDownload</CmdType>\r\n"/*命令类型*/
                 "<SN>%s</SN>\r\n"/*命令序列号*/
                 "<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统的编码*/
                 "<Result>OK</Result>\r\n"/*查询结果标志*/
                 "%s"
                 "</Response>\r\n",
                 xml_cmd_sn,
                 xml_device_id,
                 rsp_item_body);
            eXosip_message_build_request(excontext,&rsp_msg, "MESSAGE", to, from, NULL);/*构建"MESSAGE"请求*/
            osip_message_set_body(rsp_msg, rsp_xml_body, strlen(rsp_xml_body));
            osip_message_set_content_type(rsp_msg, "Application/MANSCDP+xml");
            eXosip_message_send_request(excontext,rsp_msg);/*回复"MESSAGE"请求*/
            printf("eXosip_message_send_request success!\r\n");
        }
        if (NULL != strstr(xml_config_type, "SVACEncodeConfig"))/*SVAC 编码配置*/
        {
            int i = 0;
            char temp_item_body[1024];
            memset(temp_item_body, 0, 1024);
            memset(rsp_item_body, 0, 4096);
            memset(rsp_xml_body, 0, 4096);

            for(i = 0; i < g_roi_info.roi_number; i++)
            {
                char temp_body[1024];
                memset(temp_body, 0, 1024);
                snprintf(temp_body, 1024, "<Item>\r\n"
                        "<ROISeq>%d</ROISeq>\r\n" /*感兴趣区域编号, 取值范围1~16*/
                        "<TopLeft>%d</TopLeft>\r\n"/*感兴趣区域左上角坐标, 参考 GB/T 25724—2010 的5.2.4.4.2定义, 取值范围0~19683*/
                        "<BottomRight>%d</BottomRight>\r\n" /*感兴趣区域右下角坐标, 参考 GB/T 25724—2010 的5.2.4.4.2定义, 取值范围0~19683*/
                        "<ROIQP>%d</ROIQP>\r\n"/*ROI 区域编码质量等级, 取值0: 一般,1: 较好,2: 好,3: 很好*/
                        "</Item>\r\n",
                        g_roi_info.roi_param[i].roi_seq,
                        g_roi_info.roi_param[i].top_left,
                        g_roi_info.roi_param[i].bottom_right,
                        g_roi_info.roi_param[i].roi_qp);
                strcat(temp_item_body, temp_body);
            }
            snprintf(rsp_item_body, 4096, "<SVACEncodeConfig>\r\n"
                    "<ROIParam>\r\n" /*感兴趣区域参数*/
                        "<ROIFlag>%d</ROIFlag>\r\n" /*感兴趣区域开关*/
                        "<ROINumber>%d</ROINumber>\r\n" /*感兴趣区域数量, 取值范围0~16*/
                        "%s"
                        "<BackGroundQP>%d</BackGroundQP>\r\n"/*背景区域编码质量等级, 取值0: 一般,1: 较好,2: 好,3: 很好*/
                        "<BackGroundSkipFlag>%d</BackGroundSkipFlag>\r\n"/*背景跳过开关, 取值0: 关闭,1: 打开*/
                        "<SVCParam>\r\n"/*SVC 参数*/
                            "<SVCSpaceDomainMode>%d</SVCSpaceDomainMode>\r\n"/*空域编码方式, 取值0: 基本层;1:1 级增强(1 个增强层) ;2:2 级增强(2 个增强层) ;3:3 级增强(3 个增强层)*/
                            "<SVCTimeDomainMode>%d</SVCTimeDomainMode>\r\n"/*时域编码方式, 取值0: 基本层;1:1 级增强;2:2 级增强;3:3 级增强*/
                            "<SVCSpaceSupportMode>%d</SVCSpaceSupportMode>\r\n"/*空域编码能力, 取值0: 不支持;1:1 级增强(1 个增强层) ;2:2 级增强(2 个增强层) ;3:3 级增强(3 个增强层)*/
                            "<SVCTimeSupportMode>%d</SVCTimeSupportMode>\r\n"/*时域编码能力, 取值0: 不支持;1:1 级增强;2:2 级增强;3:3 级增强*/
                        "</SVCParam>\r\n"
                        "<SurveillanceParam>\r\n"/*监控专用信息参数*/
                            "<TimeFlag>%d</TimeFlag>\r\n"/*绝对时间信息开关, 取值0: 关闭,1: 打开*/
                            "<EventFlag>%d</EventFlag>\r\n"/*监控事件信息开关, 取值0: 关闭,1: 打开*/
                            "<AlertFlag>%d</AlertFlag>\r\n"/*报警信息开关, 取值0: 关闭,1: 打开*/
                        "</SurveillanceParam>\r\n"
                        "<AudioParam>\r\n"/*音频参数*/
                            "<AudioRecognitionFlag>%d</AudioRecognitionFlag>\r\n"/*声音识别特征参数开关, 取值0: 关闭,1: 打开(*/
                        "</AudioParam>\r\n"
                    "</ROIParam>\r\n"
                    "</SVACEncodeConfig>\r\n",
                    g_roi_info.roi_flag,
                    g_roi_info.roi_number,
                    temp_item_body,
                    g_roi_info.back_groundQP,
                    g_roi_info.back_ground_skip_flag,
                    g_enc_svc_param.svc_space_domainmode,
                    g_enc_svc_param.svc_time_domainmode,
                    g_enc_svc_param.svc_space_supportmode,
                    g_enc_svc_param.svc_time_supportmode,
                    g_enc_surveillance_param.time_flag,
                    g_enc_surveillance_param.event_flag,
                    g_enc_surveillance_param.alert_flag,
                    g_svac_audio_param.audio_recognition_flag);

            snprintf(rsp_xml_body, 4096, "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n"
                 "<Response>\r\n"
                 "<CmdType>ConfigDownload</CmdType>\r\n"/*命令类型*/
                 "<SN>%s</SN>\r\n"/*命令序列号*/
                 "<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统的编码*/
                 "<Result>OK</Result>\r\n"/*查询结果标志*/
                 "%s"
                 "</Response>\r\n",
                 xml_cmd_sn,
                 xml_device_id,
                 rsp_item_body);
            eXosip_message_build_request(excontext,&rsp_msg, "MESSAGE", to, from, NULL);/*构建"MESSAGE"请求*/
            osip_message_set_body(rsp_msg, rsp_xml_body, strlen(rsp_xml_body));
            osip_message_set_content_type(rsp_msg, "Application/MANSCDP+xml");
            eXosip_message_send_request(excontext,rsp_msg);/*回复"MESSAGE"请求*/
            printf("eXosip_message_send_request success!\r\n");
        }
        if (NULL != strstr(xml_config_type, "SVACDecodeConfig"))/*SVAC 解码配置*/
        {
            memset(rsp_item_body, 0, 4096);
            memset(rsp_xml_body, 0, 4096);

            snprintf(rsp_item_body, 4096, "<SVACDecodeConfig>\r\n"
                    "<SVCParam>\r\n"
                    "<SVCSpaceSupportMode>%d</SVCSpaceSupportMode>\r\n"/*空域编码能力, 取值0: 不支持;1:1 级增强(1 个增强层) ;2:2 级增强(2 个增强层) ;3:3 级增强(3 个增强层) */
                    "<SVCTimeSupportMode>%d</SVCTimeSupportMode>\r\n"/*时域编码能力, 取值0: 不支持;1:1 级增强;2:2 级增强;3:3 级增强*/
                    "</SVCParam>\r\n"
                    "<SurveillanceParam>\r\n"/*监控专用信息参数*/
                    "<TimeShowFlag>%d</TimeShowFlag>\r\n"/*绝对时间信息显示开关, 取值0: 关闭,1: 打开*/
                    "<EventShowFlag>%d</EventShowFlag>\r\n"/*监控事件信息显示开关, 取值0: 关闭,1: 打开*/
                    "<AlertShowtFlag>%d</AlertShowtFlag>\r\n"/*报警信息显示开关, 取值0: 关闭,1: 打开*/
                    "</SurveillanceParam>\r\n"
                    "</SVACDecodeConfig>\r\n",
                    g_dec_svc_param.svc_space_supportmode,
                    g_dec_svc_param.svc_time_supportmode,
                    g_dec_surveillance_param.time_flag,
                    g_dec_surveillance_param.event_flag,
                    g_dec_surveillance_param.alert_flag);

            snprintf(rsp_xml_body, 4096, "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n"
                 "<Response>\r\n"
                 "<CmdType>ConfigDownload</CmdType>\r\n"/*命令类型*/
                 "<SN>%s</SN>\r\n"/*命令序列号*/
                 "<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统的编码*/
                 "<Result>OK</Result>\r\n"/*查询结果标志*/
                 "%s"
                 "</Response>\r\n",
                 xml_cmd_sn,
                 xml_device_id,
                 rsp_item_body);
            eXosip_message_build_request(excontext,&rsp_msg, "MESSAGE", to, from, NULL);/*构建"MESSAGE"请求*/
            osip_message_set_body(rsp_msg, rsp_xml_body, strlen(rsp_xml_body));
            osip_message_set_content_type(rsp_msg, "Application/MANSCDP+xml");
            eXosip_message_send_request(excontext,rsp_msg);/*回复"MESSAGE"请求*/
            printf("eXosip_message_send_request success!\r\n");
        }

        return;
#if 0
        snprintf(rsp_xml_body, 4096, "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n"
                 "<Response>\r\n"
                 "<CmdType>ConfigDownload</CmdType>\r\n"/*命令类型*/
                 "<SN>%s</SN>\r\n"/*命令序列号*/
                 "<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统的编码*/
                 "<Result>OK</Result>\r\n"/*查询结果标志*/
                 "<Online>%s</Online>\r\n"/*是否在线/ONLINE/OFFLINE*/
                 "<Status>%s</Status>\r\n"/*是否正常工作*/
                 "<Encode>%s</Encode>\r\n"/*是否编码*/
                 "<Record>%s</Record>\r\n"/*是否录像*/
                 "<DeviceTime>%s</DeviceTime>\r\n"/*设备时间和日期*/
                 "<Alarmstatus Num=\"1\">\r\n"/*报警设备状态列表*/
                 "<Item>\r\n"
                 "<DeviceID>%s</DeviceID>\r\n"/*报警设备编码*/
                 "<DutyStatus>%s</DutyStatus>\r\n"/*报警设备状态*/
                 "</Item>\r\n"
                 "</Alarmstatus>\r\n"
                 "</Response>\r\n",
                 xml_cmd_sn,
                 xml_device_id,
                 device_status.status_online,
                 device_status.status_ok,
                 device_info.device_encode,
                 device_info.device_record,
                 device_status.status_time,
                 xml_device_id,
                 device_status.status_guard);
#endif
    }
    else if (0 == strcmp(xml_cmd_type, "DeviceConfig"))/* 设备配置*/
    {
        /*配置参数类型*/
        int i = 0;
        char xml_config_body[4096];
        char xml_param[4096];
        char xml_param_value[30];
        char *p_xml_item = NULL;

        memset(xml_config_body, 0, 4096);
        memset(xml_param, 0, 4096);
        memset(xml_param_value, 0, 30);
        printf("**********DEVICE CONFIG START **********\r\n");
        printf("**********DEVICE CONFIG END**********\r\n");

        p_str_begin = strstr(p_xml_body, "<BasicParam>");/*查找字符串"<BasicParam>"*//*基本参数配置类型(可选) */
        if (NULL != p_str_begin)
        {
            p_str_end = strstr(p_xml_body, "</BasicParam>");
            memcpy(xml_config_body, p_str_begin + 12, p_str_end - p_str_begin - 12); /*保存<BasicParam>到xml_config_body*/

            p_str_begin = strstr(xml_config_body, "<Name>");/*查找字符串"<Name>"*/
            if (NULL != p_str_begin)
            {
                p_str_end = strstr(xml_config_body, "</Name>");
                memcpy(xml_param, p_str_begin + 6, p_str_end - p_str_begin - 6);/*保存设备名称*/
                strcpy(device_info.device_name, xml_param);/*改变设备名称*/
            }
            memset(xml_param, 0, 4096);
            p_str_begin = strstr(xml_config_body, "<Expiration>");/*查找字符串"<Expiration>"*/
            if (NULL != p_str_begin)
            {
                p_str_end = strstr(xml_config_body, "</Expiration>");
                memcpy(xml_param, p_str_begin + 12, p_str_end - p_str_begin - 12);/*保存注册过期时间*/
                SipConfig->regExpire_s = atoi(xml_param); /*改变注册过期时间*/
            }
            memset(xml_param, 0, 4096);
            p_str_begin = strstr(xml_config_body, "<HeartBeatInterval>");/*查找字符串"<HeartBeatInterval>"*/
            if (NULL != p_str_begin)
            {
                p_str_end = strstr(xml_config_body, "</HeartBeatInterval>");
                memcpy(xml_param, p_str_begin + 19, p_str_end - p_str_begin - 19);/*保存心跳间隔时间*/
                SipConfig->sipDevHBCycle_s = atoi(xml_param);
            }
            memset(xml_param, 0, 4096);
            p_str_begin = strstr(xml_config_body, "<HeartBeatCount>");/*查找字符串"<HeartBeatCount>"*/
            if (NULL != p_str_begin)
            {
                p_str_end = strstr(xml_config_body, "</HeartBeatCount>");
                memcpy(xml_param, p_str_begin + 16, p_str_end - p_str_begin - 16);/*保存心跳超时次数*/
                SipConfig->sipDevHBOutTimes = atoi(xml_param);
            }
            memset(xml_param, 0, 4096);
        }
        memset(xml_config_body, 0, 4096);
        p_str_begin = strstr(p_xml_body, "<SVACEncodeConfig>");/*查找字符串"<SVACEncodeConfig>"*//*SVAC 编码配置(可选)*/
        if (NULL != p_str_begin)
        {
            p_str_end = strstr(p_xml_body, "</SVACEncodeConfig>");
            memcpy(xml_config_body, p_str_begin + 18, p_str_end - p_str_begin - 18); /*保存<SVACEncodeConfig>到xml_config_body*/
            p_str_begin = strstr(xml_config_body, "<ROIParam>");/*查找字符串"<ROIParam>"*/
            if (NULL != p_str_begin)
            {
                p_str_end = strstr(xml_config_body, "</ROIParam>");
                memcpy(xml_param, p_str_begin + 10, p_str_end - p_str_begin - 10);/*保存ROIParam*/
                p_str_begin = strstr(xml_param, "<ROIFlag>");/*查找字符串"<ROIFlag>"*/
                if (NULL != p_str_begin)
                {
                    p_str_end = strstr(xml_param, "</ROIFlag>");
                    memcpy(xml_param_value, p_str_begin + 9, p_str_end - p_str_begin - 9);/*保存ROIFlag*/
                    g_roi_info.roi_flag = atoi(xml_param_value);
                    memset(xml_param_value, 0, 30);
                }
                p_str_begin = strstr(xml_param, "<ROINumber>");/*查找字符串"<ROINumber>"*/
                if (NULL != p_str_begin)
                {
                    p_str_end = strstr(xml_param, "</ROINumber>");
                    memcpy(xml_param_value, p_str_begin + 11, p_str_end - p_str_begin - 11);/*保存ROINumber*/
                    g_roi_info.roi_number = atoi(xml_param_value);
                    memset(xml_param_value, 0, 30);
                }

                p_xml_item = xml_param;
                for(i = 0; i < g_roi_info.roi_number; i++)
                {
                    char tmp_item_body[1024];
                    memset(tmp_item_body, 0, 1024);

                    p_str_begin = strstr(p_xml_item, "<Item>");/*查找字符串"<Item>"*/
                    if (NULL != p_str_begin)
                    {
                        p_str_end = strstr(p_xml_item, "</Item>");
                        p_xml_item = p_str_end + 7;

                        memcpy(tmp_item_body, p_str_begin + 6, p_str_end - p_str_begin - 6);/*保存Item*/
                        p_str_begin = strstr(tmp_item_body, "<ROISeq>");/*查找字符串"<ROISeq>"*/
                        if (NULL != p_str_begin)
                        {
                            p_str_end = strstr(tmp_item_body, "</ROISeq>");
                            memcpy(xml_param_value, p_str_begin + 8, p_str_end - p_str_begin - 8);/*保存ROISeq*/
                            g_roi_info.roi_param[i].roi_seq = atoi(xml_param_value);
                            memset(xml_param_value, 0, 30);
                        }
                        p_str_begin = strstr(tmp_item_body, "<TopLeft>");/*查找字符串"<TopLeft>"*/
                        if (NULL != p_str_begin)
                        {
                            p_str_end = strstr(tmp_item_body, "</TopLeft>");
                            memcpy(xml_param_value, p_str_begin + 9, p_str_end - p_str_begin - 9);/*保存TopLeft*/
                            g_roi_info.roi_param[i].top_left = atoi(xml_param_value);
                            memset(xml_param_value, 0, 30);
                        }
                        p_str_begin = strstr(tmp_item_body, "<BottomRight>");/*查找字符串"<BottomRight>"*/
                        if (NULL != p_str_begin)
                        {
                            p_str_end = strstr(tmp_item_body, "</BottomRight>");
                            memcpy(xml_param_value, p_str_begin + 13, p_str_end - p_str_begin - 13);/*保存BottomRight*/
                            g_roi_info.roi_param[i].bottom_right = atoi(xml_param_value);
                            memset(xml_param_value, 0, 30);
                        }
                        p_str_begin = strstr(tmp_item_body, "<ROIQP>");/*查找字符串"<ROIQP>"*/
                        if (NULL != p_str_begin)
                        {
                            p_str_end = strstr(tmp_item_body, "</ROIQP>");
                            memcpy(xml_param_value, p_str_begin + 7, p_str_end - p_str_begin - 7);/*保存ROIQP*/
                            g_roi_info.roi_param[i].roi_qp = atoi(xml_param_value);
                            memset(xml_param_value, 0, 30);
                        }
                    }
                }
            }
            memset(xml_param, 0, 4096);
            p_str_begin = strstr(xml_config_body, "<BackGroundQP>");/*查找字符串"<BackGroundQP>"*/
            if (NULL != p_str_begin)
            {
                p_str_end = strstr(xml_config_body, "</BackGroundQP>");
                memcpy(xml_param, p_str_begin + 14, p_str_end - p_str_begin - 14);/*保存BackGroundQP*/
                g_roi_info.back_groundQP = atoi(xml_param);
                memset(xml_param, 0, 4096);
            }
            p_str_begin = strstr(xml_config_body, "<BackGroundSkipFlag>");/*查找字符串"<BackGroundSkipFlag>"*/
            if (NULL != p_str_begin)
            {
                p_str_end = strstr(xml_config_body, "</BackGroundSkipFlag>");
                memcpy(xml_param, p_str_begin + 20, p_str_end - p_str_begin - 20);/*保存BackGroundSkipFlag*/
                g_roi_info.back_ground_skip_flag = atoi(xml_param);
                memset(xml_param, 0, 4096);
            }
            p_str_begin = strstr(xml_config_body, "<AudioParam>");/*查找字符串"<AudioParam>"*/
            if (NULL != p_str_begin)
            {
                p_str_end = strstr(xml_config_body, "</AudioParam>");
                memcpy(xml_param, p_str_begin + 12, p_str_end - p_str_begin - 12);/*保存AudioParam*/
                p_str_begin = strstr(xml_param, "<AudioRecognitionFlag>");/*查找字符串"<AudioRecognitionFlag>"*/
                if (NULL != p_str_begin)
                {
                    p_str_end = strstr(xml_param, "</AudioRecognitionFlag>");
                    memset(xml_param_value, 0, 30);
                    memcpy(xml_param_value, p_str_begin + 22, p_str_end - p_str_begin - 22);/*保存AudioRecognitionFlag*/
                    g_svac_audio_param.audio_recognition_flag = atoi(xml_param_value);
                }
            }
        }
        memset(xml_param, 0, 4096);
        memset(xml_config_body, 0, 4096);
        p_str_begin = strstr(p_xml_body, "<SVACDecodeConfig>");/*查找字符串"<SVACDecodeConfig>"*//*SVAC 解码配置(可选)*/
        if (NULL != p_str_begin)
        {
            p_str_end = strstr(p_xml_body, "</SVACDecodeConfig>");
            memcpy(xml_config_body, p_str_begin + 18, p_str_end - p_str_begin - 18); /*保存<SVACDecodeConfig>到xml_config_body*/
            p_str_begin = strstr(xml_config_body, "<SVCParam>");/*查找字符串"<SVCParam>"*/
            if (NULL != p_str_begin)
            {
                p_str_end = strstr(xml_config_body, "</SVCParam>");
                memcpy(xml_param, p_str_begin + 10, p_str_end - p_str_begin - 10);/*保存SVCParam*/
                p_str_begin = strstr(xml_param, "<SVCSTMMode>");/*查找字符串"<SVCSTMMode>"*/
                if (NULL != p_str_begin)
                {
                    p_str_end = strstr(xml_param, "</SVCSTMMode>");
                    memset(xml_param_value, 0, 30);
                    memcpy(xml_param_value, p_str_begin + 12, p_str_end - p_str_begin - 12);/*保存SVCSTMMode*/
                    g_dec_svc_param.svc_space_domainmode = atoi(xml_param_value);
                    g_dec_svc_param.svc_time_domainmode = atoi(xml_param_value);

                }
            }
            memset(xml_param, 0, 4096);
            p_str_begin = strstr(xml_config_body, "<SurveillanceParam>");/*查找字符串"<SurveillanceParam>"*/
            if (NULL != p_str_begin)
            {
                p_str_end = strstr(xml_config_body, "</SurveillanceParam>");
                memcpy(xml_param, p_str_begin + 19, p_str_end - p_str_begin - 19);/*保存SurveillanceParam*/
                p_str_begin = strstr(xml_param, "<TimeShowFlag>");/*查找字符串"<TimeShowFlag>"*/
                if (NULL != p_str_begin)
                {
                    p_str_end = strstr(xml_param, "</TimeShowFlag>");
                    memset(xml_param_value, 0, 30);
                    memcpy(xml_param_value, p_str_begin + 14, p_str_end - p_str_begin - 14);/*保存TimeShowFlag*/
                    g_dec_surveillance_param.time_flag = atoi(xml_param_value);
                }
                p_str_begin = strstr(xml_param, "<EventShowFlag>");/*查找字符串"<EventShowFlag>"*/
                if (NULL != p_str_begin)
                {
                    p_str_end = strstr(xml_param, "</EventShowFlag>");
                    memset(xml_param_value, 0, 30);
                    memcpy(xml_param_value, p_str_begin + 15, p_str_end - p_str_begin - 15);/*保存EventShowFlag*/
                    g_dec_surveillance_param.event_flag = atoi(xml_param_value);
                }
                p_str_begin = strstr(xml_param, "<AlertShowtFlag>");/*查找字符串"<AlerShowtFlag>"*/
                if (NULL != p_str_begin)
                {
                    p_str_end = strstr(xml_param, "</AlertShowtFlag>");
                    memset(xml_param_value, 0, 30);
                    memcpy(xml_param_value, p_str_begin + 16, p_str_end - p_str_begin - 16);/*保存AlerShowtFlag*/
                    g_dec_surveillance_param.alert_flag = atoi(xml_param_value);
                }
            }
        }

        snprintf(rsp_xml_body, 4096, "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n"
                  "<Response>\r\n"
                  "<CmdType>DeviceConfig</CmdType>\r\n"/*命令类型*/
                  "<SN>%s</SN>\r\n"/*命令序列号*/
                  "<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统的编码*/
                  "<Result>OK</Result>\r\n"/*查询结果标志*/
                  "</Response>\r\n",
                  xml_cmd_sn,
                  xml_device_id);
    }
    else if (0 == strcmp(xml_cmd_type, "Broadcast"))/* 语音广播*/
    {
		memset(rsp_xml_body, 0, 4096);

        p_str_begin = strstr(p_xml_body, "<SourceID>");
		p_str_end = strstr(p_xml_body, "</SourceID>");
        if ((NULL != p_str_begin) && (NULL != p_str_end))
        {
            memset(g_broadcast_srcid, 0, sizeof(g_broadcast_srcid));
            memcpy(g_broadcast_srcid, p_str_begin + 10, p_str_end - p_str_begin - 10);
        }

        p_str_begin = strstr(p_xml_body, "<TargetID>");
		p_str_end = strstr(p_xml_body, "</TargetID>");
        if ((NULL != p_str_begin) && (NULL != p_str_begin))
        {
            memset(g_broadcast_dstid, 0, sizeof(g_broadcast_dstid));
            memcpy(g_broadcast_dstid, p_str_begin + 10, p_str_end - p_str_begin - 10);
        }

		snprintf(rsp_xml_body, 4096, "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n"
			 "<Response>\r\n"
			 "<CmdType>Broadcast</CmdType>\r\n"/*命令类型*/
			 "<SN>%s</SN>\r\n"/*命令序列号*/
			 "<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统的编码*/
			 "<Result>OK</Result>\r\n"/*查询结果标志*/
			 "</Response>\r\n",
			 xml_cmd_sn,
			 g_broadcast_dstid);

		printf("receive Broadcast success!\r\n");

		//osip_call_id_t *callid = osip_message_get_call_id(p_event->request);
		osip_call_id_t *callid = osip_message_get_call_id(rsp_msg);

		printf("Broadcast callid number:%s,sourceID:%s,targetID:%s\n", callid->number,g_broadcast_srcid,g_broadcast_dstid);

		memset(g_broadcast_callid, 0, 128);
		strcpy(g_broadcast_callid, callid->number);

	}
    else if (0 == strcmp(xml_cmd_type, "MobilePosition"))/* 移动位置信息*/
    {

        osip_header_t *header = NULL;
        char xml_param_value[30];
		memset(rsp_xml_body, 0, 4096);

		snprintf(rsp_xml_body, 4096, "<?xml version=\"1.0\" encoding=\"GB2312\" ?>\r\n"
			 "<Response>\r\n"
			 "<CmdType>MobilePosition</CmdType>\r\n"/*命令类型*/
			 "<SN>%s</SN>\r\n"/*命令序列号*/
			 "<DeviceID>%s</DeviceID>\r\n"/*目标设备/区域/系统的编码*/
			 "<Result>OK</Result>\r\n"/*查询结果标志*/
			 "</Response>\r\n",
			 xml_cmd_sn,
			 xml_device_id);

        p_str_begin = strstr(p_xml_body, "<Interval>");
        if (NULL != p_str_begin)
        {
            p_str_end = strstr(p_xml_body, "</Interval>");
            memset(xml_param_value, 0, 30);
            memcpy(xml_param_value, p_str_begin + 10, p_str_end - p_str_begin - 10);
            positionInterval = atoi(xml_param_value);
        }

		osip_call_id_t *callid = osip_message_get_call_id(p_event->request);
        positionExpires = osip_message_get_expires(p_event->request, 0, &header);
        positionCurTime = 0;
		printf("receive MobilePosition expires:%d, interval:%d\n", positionExpires, positionInterval);

	}
    else/*CmdType为不支持的类型*/
    {
        printf("**********OTHER TYPE START**********\r\n");
        printf("***********OTHER TYPE END***********\r\n");
        return;
    }
    osip_message_set_body(rsp_msg, rsp_xml_body, strlen(rsp_xml_body));
    osip_message_set_content_type(rsp_msg, "Application/MANSCDP+xml");
    eXosip_lock (excontext);
    eXosip_message_send_request(excontext,rsp_msg);/*回复"MESSAGE"请求*/
    eXosip_unlock (excontext);
    printf("eXosip_message_send_request success!\r\n");
}
/*解析INVITE的SDP消息体，同时保存全局INVITE连接ID和全局会话ID*/
void ite_eXosip_parseInviteBody(struct eXosip_t *excontext,eXosip_event_t *p_event,IPNC_SipIpcConfig *SipConfig)
{
    sdp_message_t *sdp_msg = NULL;
    char *sdp_buf = NULL;
    char *media_sever_ip   = NULL;
    char *media_sever_port = NULL;
    char *media_fromat =NULL;
    char *OverTcp=NULL;
    char *tcp_attr=NULL;
    int playChannel = 0;
    unsigned int enOverTcp = 0;
    char localip[128]={0};
	char *playback_start_time = NULL;
	char *playback_stop_time = NULL;
	TIME_RECORD_T rangeInfo;
    int ssrc = 0;
    char *message = NULL;
    size_t length = 0;
    char tmp[256]={0};
	char *call_id_str = NULL;
	int download_speed = 4;
    char callidStr[64] = {0};

	memset(&rangeInfo,0,sizeof(TIME_RECORD_T));

	eXosip_guess_localip (excontext,AF_INET, localip, 128);
    sdp_msg = eXosip_get_sdp_info(p_event->request);//eXosip_get_remote_sdp(excontext,p_event->did);
    if (sdp_msg == NULL)
    {
        printf("eXosip_get_SDP NULL!\r\n");
        return;
    }
    printf("eXosip_get_SDP success!\r\n");

    g_call_id = p_event->cid;/*保存全局INVITE连接ID*/
    if(p_event->request->call_id)
	{
		osip_call_id_to_str(p_event->request->call_id, &call_id_str);
		if(call_id_str)
		{
			printf("%s callid:%s\n", sdp_msg->s_name, call_id_str);
			strcpy(callidStr, call_id_str);
            free(call_id_str);
		}
	}
    /*实时点播*/
    if (0 == strcmp(sdp_msg->s_name, "Play"))
    {
        g_did_realPlay = p_event->did;/*保存全局会话ID：实时视音频点播*/
    }
    /*回放*/
    else if (0 == strcmp(sdp_msg->s_name, "Playback"))
    {
        g_did_backPlay = p_event->did;/*保存全局会话ID：历史视音频回放*/

		playback_start_time = sdp_message_t_start_time_get(sdp_msg, 0);/*start time*/
		playback_stop_time 	= sdp_message_t_stop_time_get(sdp_msg, 0);/*end time*/

		rangeInfo.start_time = gk_gb28181_timestamp_to_u64t(playback_start_time);
		rangeInfo.end_time   = gk_gb28181_timestamp_to_u64t(playback_stop_time);

		if(playback_start_time)
			rangeInfo.start_timetick = (unsigned long long)atoll(playback_start_time);

		if(playback_stop_time)
			rangeInfo.end_timetick 	 = (unsigned long long)atoll(playback_stop_time);

		printf("playback from %s to %s\r\n", playback_start_time, playback_stop_time);

        strcpy(&g_call_id_str[0][0], callidStr);
        
        playChannel = 3;

    }
    /*下载*/
    else if (0 == strcmp(sdp_msg->s_name, "Download"))
    {
        g_did_fileDown = p_event->did;/*保存全局会话ID：视音频文件下载*/

		playback_start_time = sdp_message_t_start_time_get(sdp_msg, 0);/*start time*/
		playback_stop_time 	= sdp_message_t_stop_time_get(sdp_msg, 0);/*end time*/

		rangeInfo.start_time = gk_gb28181_timestamp_to_u64t(playback_start_time);
		rangeInfo.end_time   = gk_gb28181_timestamp_to_u64t(playback_stop_time);

		if(playback_start_time)
			rangeInfo.start_timetick = (unsigned long long)atoll(playback_start_time);

		if(playback_stop_time)
			rangeInfo.end_timetick 	 = (unsigned long long)atoll(playback_stop_time);

		printf("download from %s to %s\r\n", playback_start_time, playback_stop_time);

        strcpy(&g_call_id_str[0][0], callidStr);

		osip_message_to_str(p_event->request, &message, &length);
        if (message != NULL)
        {
    		if (strstr(message, "a=downloadspeed:") != NULL)
    		{
    			memset(tmp,0,sizeof(tmp));
    			strcpy(tmp, strstr(message, "a=downloadspeed:"));
    			sscanf(tmp, "a=downloadspeed:%d", &download_speed);
    			printf("download_speed:%d\n", download_speed);
    		}
    		else
    		{
    			printf("not find downloadspeed!\n");
    		}

    		free(message);
    		message = NULL;
        }

        playChannel = 3;

    }
    /*从SIP服务器发过来的INVITE请求的o字段或c字段中获取媒体服务器的IP地址与端口*/
    media_sever_ip   = sdp_message_o_addr_get(sdp_msg);/*媒体服务器IP地址*/
    media_sever_port = sdp_message_m_port_get(sdp_msg, 0);/*媒体服务器IP端口*/
    media_fromat       = sdp_message_o_addrtype_get (sdp_msg);
    OverTcp = sdp_message_m_proto_get (sdp_msg, 0);
    sdp_message_to_str(sdp_msg, &sdp_buf);
    if (strstr(OverTcp, "TCP"))
    {
        enOverTcp = 1 & 0x000F;/*低8字节中的低4位为1代表是TCP方式*/
        if (strstr(sdp_buf, "active"))/*SIP服务器是active,则IPC是TCP server*/
        {
            enOverTcp |= 0x0010;/*低8字节中的高4位为1代表IPC是TCP server*/
            printf("enOverTcp 0x%04x\r\n", enOverTcp);
        }
        else
        {
            enOverTcp |= 0x0020;/*低8字节中的高4位为2代表IPC是TCP client*/
        }
    }


    if (strstr(sdp_buf, "a=stream:sub") != NULL)
    {
        playChannel = 1;
    }


    osip_message_to_str(p_event->request, &message, &length);
    if (strstr(message, "y=") != NULL)
    {
		memset(tmp,0,sizeof(tmp));
        strcpy(tmp, strstr(message, "y="));
        sscanf(tmp, "y=%d", &ssrc);
        printf("ssrc2:%d\n", ssrc);
    }
    else
    {
        printf("not find ssrc\n");
    }

    printf("sdp_buf %s\r\n", sdp_buf);
    printf("enOverTcp 0x%x, chnnel:%d\r\n", enOverTcp, playChannel);
    printf("%s->%s:%s  [%s] \r\n", sdp_msg->s_name, media_sever_ip, media_sever_port,media_fromat);
    printf("OverTcp :%s, tcp_attr:%s, %s\r\n", OverTcp, tcp_attr, sdp_message_a_att_value_get(sdp_msg, 0, 0));

#ifdef USE_RTP_CHANNEL_IN_PLAYBACK_THREAD
    if (0 == strcmp(sdp_msg->s_name, "Play"))
    {
	    ProcessInviteMediaPlay(sdp_msg->s_name, media_fromat, localip, media_sever_ip, media_sever_port,
	        enOverTcp, playChannel, ssrc, p_event->did, callidStr);

    }
	else if(0 == strcmp(sdp_msg->s_name, "Playback"))
	{
		if(!gk_gb28181_playback_session_open(1)) //(p_event->did))
			gk_gb28181_playback_set_playback_cmd(1,&rangeInfo, localip, media_sever_ip, media_sever_port,
			enOverTcp, ssrc, p_event->did);

	}
	else if(0 == strcmp(sdp_msg->s_name, "Download"))
	{
		if(!gk_gb28181_playback_session_open(0)) //(p_event->did))
			gk_gb28181_playback_set_download_cmd(0,&rangeInfo,download_speed,localip, media_sever_ip, media_sever_port,
			enOverTcp, ssrc, p_event->did);
	}
#else

	if(0 == strcmp(sdp_msg->s_name, "Playback"))
	{
		if(!gk_gb28181_playback_session_open(1)) //(p_event->did))
			gk_gb28181_playback_set_playback_cmd(1,&rangeInfo, localip, media_sever_ip, media_sever_port,
			enOverTcp, ssrc, p_event->did);

	}
	else if(0 == strcmp(sdp_msg->s_name, "Download"))
	{
		if(!gk_gb28181_playback_session_open(0)) //(p_event->did))
			gk_gb28181_playback_set_download_cmd(0,&rangeInfo,download_speed,localip, media_sever_ip, media_sever_port,
			enOverTcp, ssrc, p_event->did);
	}

	ProcessInviteMediaPlay(sdp_msg->s_name, media_fromat, localip, media_sever_ip, media_sever_port,
	        enOverTcp, playChannel, ssrc, p_event->did, callidStr);
#endif

    free(message);
    message = NULL;
    free(sdp_buf);
    sdp_buf = NULL;
    sdp_message_free(sdp_msg);
    sdp_msg = NULL;
}

static void ite_eXosip_parseInviteBodyForAudio(struct eXosip_t *excontext,eXosip_event_t *p_event,IPNC_SipIpcConfig *SipConfig)
{
    sdp_message_t *sdp_msg = NULL;
    char *sdp_buf = NULL;
    char *media_sever_ip   = NULL;
    char *media_sever_port = NULL;
    char *media_fromat =NULL;
    char *OverTcp=NULL;
    int playChannel = 0;
    unsigned int enOverTcp = 0;
    char localip[128]={0};
    int ssrc = 0;
    char *message = NULL;
    size_t length = 0;
    char tmp[256]={0};
    char *call_id_str = NULL;
    char callidStr[64] = {0};

	eXosip_guess_localip (excontext,AF_INET, localip, 128);
    sdp_msg = eXosip_get_sdp_info(p_event->response);
    if (sdp_msg == NULL)
    {
        printf("ite_eXosip_parseInviteBodyForAudio eXosip_get_sdp_info NULL!\r\n");
        return;
    }

    //g_call_id = p_event->cid;/*保存全局INVITE连接ID*/
    /*实时点播*/
    if (0 != strcmp(sdp_msg->s_name, "Play"))
    {
        return;
    }

    /*从SIP服务器发过来的INVITE请求的o字段或c字段中获取媒体服务器的IP地址与端口*/
    media_sever_ip   = sdp_message_o_addr_get(sdp_msg);/*媒体服务器IP地址*/
    media_sever_port = sdp_message_m_port_get(sdp_msg, 0);/*媒体服务器IP端口*/
    media_fromat       = sdp_message_o_addrtype_get (sdp_msg);
    OverTcp = sdp_message_m_proto_get (sdp_msg, 0);
    sdp_message_to_str(sdp_msg, &sdp_buf);
	if(!sdp_buf)
	{
        printf("ite_eXosip_parseInviteBodyForAudio sdp_buf NULL!\r\n");
        goto exit;
	}

    if (strstr(OverTcp, "TCP"))
    {
        enOverTcp = 1 & 0x000F;/*低8字节中的低4位为1代表是TCP方式*/
        if (strstr(sdp_buf, "active"))/*SIP服务器是active,则IPC是TCP server*/
        {
            enOverTcp |= 0x0010;/*低8字节中的高4位为1代表IPC是TCP server*/
            printf("enOverTcp 0x%04x\r\n", enOverTcp);
        }
        else
        {
            enOverTcp |= 0x0020;/*低8字节中的高4位为2代表IPC是TCP client*/
        }
    }

	osip_message_to_str(p_event->response, &message, &length);

    if (strstr(sdp_buf, "m=audio") != NULL)
    {
		if(message)
			printf("\n%s\n",message);

	    if (strstr(message, "y=") != NULL)
	    {
			memset(tmp,0,sizeof(tmp));
			char *p = strstr(message, "y=");
			if(p)
			{
		        strcpy(tmp, p);
		        sscanf(tmp, "y=%d", &ssrc);
			}

	        printf("ssrc2:%d\n", ssrc);
	    }
	    else
	    {
	        printf("not find ssrc\n");
	    }

	    printf("enOverTcp 0x%x, chnnel:%d\r\n", enOverTcp, playChannel);
	    printf("%s->%s:%s  [%s] \r\n", sdp_msg->s_name, media_sever_ip, media_sever_port,media_fromat);

		g_did_audioPlay = p_event->did;/*保存全局会话ID：实时对讲*/

        if(p_event->request->call_id)
    	{
    		osip_call_id_to_str(p_event->request->call_id, &call_id_str);
    		if(call_id_str)
    		{
    			printf("%s callid:%s\n", sdp_msg->s_name, call_id_str);
    			strcpy(callidStr, call_id_str);
                free(call_id_str);
    		}
    	}

		ProcessInviteAudioPlay("audio", media_fromat, localip, media_sever_ip, media_sever_port,
	        enOverTcp, playChannel, ssrc, p_event->did, callidStr);
	}

exit:
	if(message)
	{
	    free(message);
	    message = NULL;
	}
	if(sdp_buf)
	{
	    free(sdp_buf);
	    sdp_buf = NULL;
	}

    sdp_message_free(sdp_msg);
    sdp_msg = NULL;
}

/*解析INFO的RTSP消息体*/
void ite_eXosip_parseInfoBody(struct eXosip_t *excontext,eXosip_event_t *p_event)
{
    osip_body_t *p_msg_body = NULL;
    char *p_rtsp_body = NULL;
    char *p_str_begin = NULL;
    char *p_str_end   = NULL;
    char *p_strstr    = NULL;
    char rtsp_scale[10];
    char rtsp_range_begin[10];
    char rtsp_range_end[10];
    char rtsp_pause_time[10];
	TIME_RECORD_T rangeInfo;
	float speed = 1.0;
	int is_need_seek = 0;

    memset(rtsp_scale, 0, 10);
    memset(rtsp_range_begin, 0, 10);
    memset(rtsp_range_end, 0, 10);
    memset(rtsp_pause_time, 0, 10);

    osip_message_get_body(p_event->request, 0, &p_msg_body);
    if (NULL == p_msg_body)
    {
        printf("osip_message_get_body null!\r\n");
        return;
    }
    p_rtsp_body = p_msg_body->body;
    printf("osip_message_get_body success!\r\n");

    p_strstr = strstr(p_rtsp_body, "PLAY");
    if (NULL != p_strstr)/*查找到字符串"PLAY"*/
    {
        /*播放速度*/
        p_str_begin = strstr(p_rtsp_body, "Scale:");/*查找字符串"Scale:"*/
        p_str_end   = strstr(p_rtsp_body, "Range:");
		if(p_str_begin && p_str_end)
		{
	        memcpy(rtsp_scale, p_str_begin + 6, p_str_end - p_str_begin - 6); /*保存Scale到rtsp_scale*/
	        printf("PlayScale:%s\r\n", rtsp_scale);
		}
		else if(p_str_begin)
		{
	        strcpy(rtsp_scale, p_str_begin+6); /*保存Scale到rtsp_scale*/
	        printf("rtsp_scale:%s\r\n", rtsp_scale);
		}
        /*播放范围*/
        p_str_begin = strstr(p_rtsp_body, "npt=");/*查找字符串"npt="*/
        p_str_end   = strstr(p_rtsp_body, "-");

		if(p_str_begin && p_str_end)
		{
	        memcpy(rtsp_range_begin, p_str_begin + 4, p_str_end - p_str_begin - 4); /*保存RangeBegin到rtsp_range_begin*/
	        printf("PlayRangeBegin:%s\r\n", rtsp_range_begin);
		}

        p_str_begin = strstr(p_rtsp_body, "-");/*查找字符串"-"*/
		if(p_str_begin)
		{
	        strcpy(rtsp_range_end, p_str_begin + 1); /*保存RangeEnd到rtsp_range_end*/
	        printf("PlayRangeEnd:%s\r\n", rtsp_range_end);
		}

		if(!strcmp(rtsp_range_begin,"now"))
		{
			rangeInfo.start_time = 0;
			rangeInfo.end_time   = 0xFFFFFFFFFFFFFFFF;
			is_need_seek = 0;
		}
		else
		{
			is_need_seek = 1;

			if(strlen(rtsp_range_begin))
				rangeInfo.start_time = atoi(rtsp_range_begin);
			else
			{
				rangeInfo.start_time = 0;
				is_need_seek = 0;
			}

			if(strlen(rtsp_range_end))
				rangeInfo.end_time= atoi(rtsp_range_end);
			else
				rangeInfo.end_time = 0xFFFFFFFFFFFFFFFF;
		}

		if(strlen(rtsp_scale))
			speed = atof(rtsp_scale);

        printf("##speed:%f\r\n", speed);

		gk_gb28181_playback_set_play_cmd(1,speed,is_need_seek,&rangeInfo);//(p_event->did,speed,is_need_seek,&rangeInfo);

		if(ite_eXosip_callback.ite_eXosip_playControl)
        	ite_eXosip_callback.ite_eXosip_playControl("PLAY", rtsp_scale, NULL, rtsp_range_begin, rtsp_range_end);
        return;
    }

    p_strstr = strstr(p_rtsp_body, "PAUSE");
    if (NULL != p_strstr)/*查找到字符串"PAUSE"*/
    {
        /*暂停时间*/
        p_str_begin = strstr(p_rtsp_body, "PauseTime:");/*查找字符串"PauseTime:"*/
		if(p_str_begin)
        	strcpy(rtsp_pause_time, p_str_begin + 10); /*保存PauseTime到rtsp_pause_time*/
        printf("PauseTime:%3s\r\n", rtsp_pause_time);

		if(strlen(rtsp_pause_time) && strcmp(rtsp_pause_time,"now"))
			gk_gb28181_playback_set_pause_cmd(1,atoi(rtsp_pause_time));//(p_event->did,atoi(rtsp_pause_time));
		else
			gk_gb28181_playback_set_pause_cmd(1,0);//(p_event->did,0);

		if(ite_eXosip_callback.ite_eXosip_playControl)
        	ite_eXosip_callback.ite_eXosip_playControl("PAUSE", NULL, rtsp_pause_time, NULL, NULL);
        return;
    }

	p_strstr = strstr(p_rtsp_body, "TEARDOWN");
	if (NULL != p_strstr)/*查找到字符串"TEARDOWN"*/
	{
		gk_gb28181_playback_set_stop_cmd(1);//(p_event->did);

		if(ite_eXosip_callback.ite_eXosip_playControl)
			ite_eXosip_callback.ite_eXosip_playControl("TEARDOWN", NULL, NULL, NULL, NULL);
		return;
	}

    printf("can`t find string PLAY/PAUSE/TEARDOWN!");
}

/*检测并打印事件*/
void ite_eXosip_printEvent(struct eXosip_t *excontext,eXosip_event_t *p_event)
{
    //osip_message_t *clone_event = NULL;
    size_t length = 0;
    char *message = NULL;

    printf("\r\n##############################################################\r\n");
    switch (p_event->type)
    {
    #if 0
        case EXOSIP_REGISTRATION_NEW:
            printf("EXOSIP_REGISTRATION_NEW\r\n");
            break;
        case EXOSIP_REGISTRATION_REFRESHED:
            printf("EXOSIP_REGISTRATION_REFRESHED\r\n");
            break;
        case EXOSIP_REGISTRATION_TERMINATED:
            printf("EXOSIP_REGISTRATION_TERMINATED\r\n");
            break;
        case EXOSIP_CALL_TIMEOUT:
            printf("EXOSIP_CALL_TIMEOUT\r\n");
            break;
        case EXOSIP_SUBSCRIPTION_UPDATE:
            printf("EXOSIP_SUBSCRIPTION_UPDATE\r\n");
            break;
        case EXOSIP_SUBSCRIPTION_CLOSED:
            printf("EXOSIP_SUBSCRIPTION_CLOSED\r\n");
            break;
        case EXOSIP_SUBSCRIPTION_RELEASED:
            printf("EXOSIP_SUBSCRIPTION_RELEASED\r\n");
            break;
        case EXOSIP_IN_SUBSCRIPTION_RELEASED:
            printf("EXOSIP_IN_SUBSCRIPTION_RELEASED\r\n");
            break;
#endif
       case EXOSIP_REGISTRATION_SUCCESS:
            printf("EXOSIP_REGISTRATION_SUCCESS\r\n");
            break;
        case EXOSIP_REGISTRATION_FAILURE:
            printf("EXOSIP_REGISTRATION_FAILURE\r\n");
            break;

        case EXOSIP_CALL_INVITE:
            printf("EXOSIP_CALL_INVITE\r\n");
            break;
        case EXOSIP_CALL_REINVITE:
            printf("EXOSIP_CALL_REINVITE\r\n");
            break;
        case EXOSIP_CALL_NOANSWER:
            printf("EXOSIP_CALL_NOANSWER\r\n");
            break;
        case EXOSIP_CALL_PROCEEDING:
            printf("EXOSIP_CALL_PROCEEDING\r\n");
            break;
        case EXOSIP_CALL_RINGING:
            printf("EXOSIP_CALL_RINGING\r\n");
            break;
        case EXOSIP_CALL_ANSWERED:
            printf("EXOSIP_CALL_ANSWERED\r\n");
            break;
        case EXOSIP_CALL_REDIRECTED:
               printf("EXOSIP_CALL_REDIRECTED\r\n");
            break;
        case EXOSIP_CALL_REQUESTFAILURE:
            printf("EXOSIP_CALL_REQUESTFAILURE\r\n");
            break;
        case EXOSIP_CALL_SERVERFAILURE:
            printf("EXOSIP_CALL_SERVERFAILURE\r\n");
            break;
        case EXOSIP_CALL_GLOBALFAILURE:
            printf("EXOSIP_CALL_GLOBALFAILURE\r\n");
            break;
        case EXOSIP_CALL_ACK:
            printf("EXOSIP_CALL_ACK\r\n");
            break;
        case EXOSIP_CALL_CANCELLED:
            printf("EXOSIP_CALL_CANCELLED\r\n");
            break;

        case EXOSIP_CALL_MESSAGE_NEW:
            printf("EXOSIP_CALL_MESSAGE_NEW\r\n");
            break;
        case EXOSIP_CALL_MESSAGE_PROCEEDING:
            printf("EXOSIP_CALL_MESSAGE_PROCEEDING\r\n");
            break;
        case EXOSIP_CALL_MESSAGE_ANSWERED:
            printf("EXOSIP_CALL_MESSAGE_ANSWERED\r\n");
            break;
        case EXOSIP_CALL_MESSAGE_REDIRECTED:
            printf("EXOSIP_CALL_MESSAGE_REDIRECTED\r\n");
            break;
        case EXOSIP_CALL_MESSAGE_REQUESTFAILURE:
            printf("EXOSIP_CALL_MESSAGE_REQUESTFAILURE\r\n");
            break;
        case EXOSIP_CALL_MESSAGE_SERVERFAILURE:
            printf("EXOSIP_CALL_MESSAGE_SERVERFAILURE\r\n");
            break;
        case EXOSIP_CALL_MESSAGE_GLOBALFAILURE:
            printf("EXOSIP_CALL_MESSAGE_GLOBALFAILURE\r\n");
            break;
        case EXOSIP_CALL_CLOSED:
            printf("EXOSIP_CALL_CLOSED\r\n");
            break;
        case EXOSIP_CALL_RELEASED:
            printf("EXOSIP_CALL_RELEASED\r\n");
            break;
        case EXOSIP_MESSAGE_NEW:
            printf("EXOSIP_MESSAGE_NEW\r\n");
            break;
        case EXOSIP_MESSAGE_PROCEEDING:
            printf("EXOSIP_MESSAGE_PROCEEDING\r\n");
            break;
        case EXOSIP_MESSAGE_ANSWERED:
            printf("EXOSIP_MESSAGE_ANSWERED\r\n");
            break;
        case EXOSIP_MESSAGE_REDIRECTED:
            printf("EXOSIP_MESSAGE_REDIRECTED\r\n");
            break;
        case EXOSIP_MESSAGE_REQUESTFAILURE:
            printf("EXOSIP_MESSAGE_REQUESTFAILURE\r\n");
            break;
        case EXOSIP_MESSAGE_SERVERFAILURE:
            printf("EXOSIP_MESSAGE_SERVERFAILURE\r\n");
            break;
        case EXOSIP_MESSAGE_GLOBALFAILURE:
            printf("EXOSIP_MESSAGE_GLOBALFAILURE\r\n");
            break;

        case EXOSIP_SUBSCRIPTION_NOANSWER:
            printf("EXOSIP_SUBSCRIPTION_NOANSWER\r\n");
            break;
        case EXOSIP_SUBSCRIPTION_PROCEEDING:
            printf("EXOSIP_SUBSCRIPTION_PROCEEDING\r\n");
            break;
        case EXOSIP_SUBSCRIPTION_ANSWERED:
            printf("EXOSIP_SUBSCRIPTION_ANSWERED\r\n");
            break;
        case EXOSIP_SUBSCRIPTION_REDIRECTED:
            printf("EXOSIP_SUBSCRIPTION_REDIRECTED\r\n");
            break;
        case EXOSIP_SUBSCRIPTION_REQUESTFAILURE:
            printf("EXOSIP_SUBSCRIPTION_REQUESTFAILURE\r\n");
            break;
        case EXOSIP_SUBSCRIPTION_SERVERFAILURE:
            printf("EXOSIP_SUBSCRIPTION_SERVERFAILURE\r\n");
            break;
        case EXOSIP_SUBSCRIPTION_GLOBALFAILURE:
            printf("EXOSIP_SUBSCRIPTION_GLOBALFAILURE\r\n");
            break;
        case EXOSIP_SUBSCRIPTION_NOTIFY:
            printf("EXOSIP_SUBSCRIPTION_NOTIFY\r\n");
            break;

        case EXOSIP_IN_SUBSCRIPTION_NEW:
            printf("EXOSIP_IN_SUBSCRIPTION_NEW\r\n");
            break;
        case EXOSIP_NOTIFICATION_NOANSWER:
            printf("EXOSIP_NOTIFICATION_NOANSWER\r\n");
            break;
        case EXOSIP_NOTIFICATION_PROCEEDING:
            printf("EXOSIP_NOTIFICATION_PROCEEDING\r\n");
            break;
        case EXOSIP_NOTIFICATION_ANSWERED:
            printf("EXOSIP_NOTIFICATION_ANSWERED\r\n");
            break;
        case EXOSIP_NOTIFICATION_REDIRECTED:
            printf("EXOSIP_NOTIFICATION_REDIRECTED\r\n");
            break;
        case EXOSIP_NOTIFICATION_REQUESTFAILURE:
            printf("EXOSIP_NOTIFICATION_REQUESTFAILURE\r\n");
            break;
        case EXOSIP_NOTIFICATION_SERVERFAILURE:
            printf("EXOSIP_NOTIFICATION_SERVERFAILURE\r\n");
            break;
        case EXOSIP_NOTIFICATION_GLOBALFAILURE:
            printf("EXOSIP_NOTIFICATION_GLOBALFAILURE\r\n");
            break;
        case EXOSIP_EVENT_COUNT:
            printf("EXOSIP_EVENT_COUNT\r\n");
            break;
        default:
            printf("..................\r\n");
            break;
    }
    //osip_message_clone(p_event->request, &clone_event);
    if (p_event->request != NULL)
    {
        osip_message_to_str(p_event->request, &message, &length);
        if (message != NULL)
        {
            if (strstr(message, "RecordInfo") != NULL)
            {
                printf("RecordInfo\r\n");
            }
            else
            {
                printf("%s\r\n", message);
            }
            free(message);
            message = NULL;
        }
    }
    printf("##############################################################\r\n\r\n");
    //osip_message_free(clone_event);
}

/*
    服务处理线程
*/
void *ite_eXosip_NotifyAlarm_Task(void *pData)
{
    struct eXosip_t *excontext = ((ite_gb28181Obj *)pData)->excontext;
    IPNC_SipIpcConfig *Sipconfig = ((ite_gb28181Obj *)pData)->sipconfig;

    struct timezone Ztest;
    struct tm timeNow;
    struct timeval systime;
    char eXosip_status_time[30]         = {0};
    pthread_detach(pthread_self());
    sdk_sys_thread_set_name("gb_alarm");

    while(g_bterminate == ITE_OSAL_FALSE)
    {
        gettimeofday(&systime,&Ztest);
        localtime_r(&systime.tv_sec, &timeNow);
        sprintf(eXosip_status_time, "%04d-%02d-%02dT%02d:%02d:%02d",
                                                timeNow.tm_year + 1900, timeNow.tm_mon + 1, timeNow.tm_mday,
                                                timeNow.tm_hour, timeNow.tm_min, timeNow.tm_sec);
        strcpy(device_status.status_time, eXosip_status_time);
        //printf("status_time:%s\r\n", device_status.status_time);

        if(g_recvalarm != 0 && g_register_status)
        {
#ifdef MODULE_SUPPORT_MOJING_V4
            //优先检测人形
            if (runMdCfg.mojingMdCfg.areaEventStatus == 1)
                sleep(1);
#endif
            if (g_recvalarm & 0x1)
            {
                g_recvalarmType = 2;
                ite_eXosip_sendEventAlarm(excontext, device_status.status_time, Sipconfig);
                g_recvalarm = g_recvalarm & 0xe;
            }
            else if (g_recvalarmType & 0x2)
            {
                g_recvalarmType = 6;
                ite_eXosip_sendEventAlarm(excontext, device_status.status_time, Sipconfig);
                g_recvalarm = g_recvalarm & 0xd;
            }
        }
        usleep(1000000);


    }
	printf("\nite_eXosip_NotifyAlarm_Task Exit!\n");
    return NULL;
}

void *ite_eXosip_PositionAlert_Task(void *pData)
{
    struct eXosip_t *excontext = ((ite_gb28181Obj *)pData)->excontext;
    IPNC_SipIpcConfig *Sipconfig = ((ite_gb28181Obj *)pData)->sipconfig;

    struct timezone Ztest;
    struct tm timeNow;
    struct timeval systime;
    char eXosip_status_time[30]         = {0};

    pthread_detach(pthread_self());
    sdk_sys_thread_set_name("gb_pos");

    while(g_bterminate == ITE_OSAL_FALSE)
    {

        if (positionCurTime < positionExpires && g_register_status)
        {
            gettimeofday(&systime,&Ztest);
            localtime_r(&systime.tv_sec, &timeNow);
            sprintf(eXosip_status_time, "%04d-%02d-%02dT%02d:%02d:%02d",
                                                    timeNow.tm_year + 1900, timeNow.tm_mon + 1, timeNow.tm_mday,
                                                    timeNow.tm_hour, timeNow.tm_min, timeNow.tm_sec);

            ite_eXosip_sendEventPostion(excontext, eXosip_status_time, Sipconfig);
            positionCurTime += positionInterval;
            sleep(positionInterval);
        }
        else
        {
            sleep(2);
        }
    }

	printf("\nite_eXosip_PositionAlert_Task Exit!\n");
    return NULL;
}


/*消息循环处理*/
void *ite_eXosip_processEvent_Task(void *pData)
{
    eXosip_event_t *g_event  = NULL;/*消息事件*/
    osip_message_t *g_answer = NULL;/*请求的确认型应答*/
    sdp_message_t *sdp_body_rcv=NULL;
    char *sdp_buf = NULL;
    struct eXosip_t *excontext = ((ite_gb28181Obj *)pData)->excontext;
    IPNC_SipIpcConfig *SipConfig = ((ite_gb28181Obj *)pData)->sipconfig;
    int enOverTCP=0;
    pthread_detach(pthread_self());
    sdk_sys_thread_set_name("gb_event");

    while (g_bterminate == ITE_OSAL_FALSE)
    {
        /*没有注册成功时不在此线程接收消息, 避免注册成功的消息在此线程中获取*/
        if (0 == g_register_status || netcam_is_prepare_update() || (netcam_get_update_status() != 0))
        {
            sleep(1);
            continue;
        }

        /*等待新消息的到来*/
        char localip[128]={0};
        enOverTCP = 0;
        g_event = eXosip_event_wait(excontext,0, 200);/*侦听消息的到来*/
        eXosip_lock(excontext);
        eXosip_default_action(excontext,g_event);
        eXosip_guess_localip (excontext,AF_INET, localip, 128);


      //  eXosip_automatic_refresh();/*Refresh REGISTER and SUBSCRIBE before the expiration delay*/
        eXosip_unlock(excontext);
        if (NULL == g_event)
        {
            continue;
        }
        //ite_eXosip_printEvent(excontext,g_event);
        /*处理感兴趣的消息*/
        switch (g_event->type)
        {
            /*即时消息：通信双方无需事先建立连接*/
            case EXOSIP_MESSAGE_NEW:/*MESSAGE:MESSAGE*/
            {
                printf("\r\n<EXOSIP_MESSAGE_NEW>\r\n");
                if(MSG_IS_MESSAGE(g_event->request))/*使用MESSAGE方法的请求*/
                {
                    /*设备控制*/
                    /*报警事件通知和分发：报警通知响应*/
                    /*网络设备信息查询*/
                    /*设备视音频文件检索*/
                    eXosip_lock(excontext);
                    eXosip_message_build_answer(excontext,g_event->tid, 200, &g_answer);/*Build default Answer for request*/
                    eXosip_message_send_answer(excontext,g_event->tid, 200, g_answer);/*按照规则回复200OK*/
                    printf("eXosip_message_send_answer ack 200 success!\r\n");
                    eXosip_unlock(excontext);
                    ite_eXosip_ProcessMsgBody(excontext,g_event,SipConfig);/*解析MESSAGE的XML消息体，同时保存全局会话ID*/
                }
                else if(MSG_IS_BYE(g_event->request))/*使用MESSAGE方法的请求*/
                {
                    eXosip_lock(excontext);
                    eXosip_message_build_answer(excontext,g_event->tid, 200, &g_answer);/*Build default Answer for request*/
                    eXosip_message_send_answer(excontext,g_event->tid, 200, g_answer);/*按照规则回复200OK*/
                    eXosip_unlock(excontext);

                    printf("eXosip_message_send_answer ack 200 success!\r\n");
					osip_call_id_t *callid = osip_message_get_call_id(g_event->request);
					if(callid && callid->number)
					{
						if (0 == strcmp(callid->number, g_broadcast_invite_callid))
						{
                            char localTime[64] = {0};
                            int zone = 0;
                            char msgStr[1024] = {0};
                            netcam_sys_get_local_time_string(localTime,&zone);
                            
                            snprintf(msgStr, sizeof(msgStr),"callId=%s,endTime=%s",
                                    callid->number, localTime);
                        
                            #ifdef MODULE_SUPPORT_UDP_LOG
                            mojing_log_send(NULL, 0, "gb28181_bye", msgStr);
                            #endif
                            /*关闭：语音播报*/
                            printf("VoiceCall closed success!\r\n");
                            ProcessVoiceStop(g_event->did);
                            g_did_audioPlay = 0;
						}
					}
                }
            }
            break;
            /*即时消息回复的200OK*/
            case EXOSIP_MESSAGE_ANSWERED:/*200OK*/
            {
                /*设备控制*/
                /*报警事件通知和分发：报警通知*/
                /*网络设备信息查询*/
                /*设备视音频文件检索*/
                printf("\r\n<EXOSIP_MESSAGE_ANSWERED>\r\n");

                osip_call_id_t *callid = osip_message_get_call_id(g_event->request);
                if (0 == strcmp(callid->number, g_keepalive_callid))
                {
                    printf("EXOSIP_CALL_MESSAGE_ANSWERED callid number:%s\n", callid->number);
                    g_keepalive_flag = 0;
                }

				if (0 == strcmp(callid->number, g_broadcast_callid))
                {
					printf("EXOSIP_CALL_MESSAGE_ANSWERED g_broadcast_callid number:%s\n", callid->number);

					// TODO:  to be check
					//if(0 == strcmp(SipConfig->sipDeviceUID,g_broadcast_dstid))
						ite_eXosip_BroadcastInvite(excontext,g_event,SipConfig);
                }
            }
            break;
            /*以下类型的消息都必须事先建立连接*/
            case EXOSIP_CALL_INVITE:/*INVITE*/
            {
                printf("\r\n<EXOSIP_CALL_INVITE>\r\n");
                if(MSG_IS_INVITE(g_event->request))/*使用INVITE方法的请求*/
                {
                    /*实时视音频点播*/
                    /*历史视音频回放*/
                    /*视音频文件下载*/
                    osip_message_t *asw_msg = NULL;/*请求的确认型应答*/
                    char sdp_body[4096];
                    char m[256],tcp_a[256];
                    char streamType[32] = "a=stream:main";
                    unsigned short udp_videoport = 9898;
                    unsigned short tcp_videoport = 9998;
                    int ssrc = 0;
                    char *message = NULL;
                    size_t length = 0;
                    int sid = 0;
                    int sipVideoType = 2;
                    ST_GK_ENC_STREAM_H264_ATTR stH264Config;

                    memset(sdp_body, 0, 4096);
                    memset(tcp_a, 0, 256);
                    memset(m,0,256);
                    sdp_body_rcv = eXosip_get_sdp_info(g_event->request);
                    sdp_message_to_str(sdp_body_rcv, &sdp_buf);
                    if (sdp_buf == NULL)
                    {
                        printf("EXOSIP_CALL_INVITE sdp_buf is null\n");
                        break;
                    }

                    osip_message_to_str(g_event->request, &message, &length);

                    if (sdp_body_rcv)
                    {
                         printf("eXosip_get_sdp_info %s  %s  %s  %s  \r\n",
                                    sdp_message_v_version_get(sdp_body_rcv),
                                    sdp_message_o_username_get(sdp_body_rcv),
                                    sdp_message_o_sess_id_get(sdp_body_rcv),
                                    sdp_message_m_proto_get(sdp_body_rcv, 0)
                                 );
                        if (strstr(sdp_message_m_proto_get(sdp_body_rcv, 0),"TCP"))
                        {
                            enOverTCP =1;
                        }
                    }
                    else
                    {
                        printf("eXosip_get_sdp_info error  [%s] \r\n", (char*)g_event->request->application_data);

                    }

                    memset(streamType, 0, sizeof(streamType));
                    if (strstr(sdp_buf, "a=stream:main") != NULL)
                    {
                        strcpy(streamType, "a=stream:main");
                        sid = 0;
                    }
                    else if (strstr(sdp_buf, "a=stream:sub") != NULL)
                    {
                        strcpy(streamType, "a=stream:sub");
                        sid = 1;
                    }


                    memset(&stH264Config, 0, sizeof(stH264Config));
                    netcam_video_get(0, sid, &stH264Config);
                    if (stH264Config.enctype == 1)
                    {
                        sipVideoType = 2;
                    }
                    else
                    {
                        sipVideoType = 9;
                    }

                    if (strstr(message, "y=") != NULL)
                    {
                        strcpy(m, strstr(message, "y="));
                        sscanf(m, "y=%d", &ssrc);
                        printf("ssrc:%d\n", ssrc);
                    }
                    else
                    {
                        printf("not find ssrc\n");
                    }
                    free(message);
                    message = NULL;
                    memset(m,0,256);
                    eXosip_lock(excontext);
                    if (0 != eXosip_call_build_answer(excontext,g_event->tid, 200, &asw_msg))/*Build default Answer for request*/
                    {
                        eXosip_call_send_answer(excontext,g_event->tid, 603, NULL);
                        eXosip_unlock(excontext);
                        printf("eXosip_call_build_answer error!\r\n");
                        break;
                    }

                    if (!enOverTCP)
                    {
                        sprintf(m, "m=video %d RTP/AVP 96",udp_videoport);// udp
                    }
                    else
                    {
                        sprintf(m, "m=video %d TCP/RTP/AVP 96",tcp_videoport);
                        if (strstr(sdp_buf, "active"))
                        {
                            sprintf(tcp_a,  "a=setup:active\r\n""a=connection:new\r\n");//active    ipc:port   <-------server    ipc作为tcp服务端口 server 来连接ipc端口后传输视频
                        }
                        else
                        {
                            sprintf(tcp_a,  "a=setup:passive\r\n""a=connection:new\r\n");//passive   // ipc--->server:port  ipc是tcp的客户端
                        }
                    }
                    eXosip_unlock(excontext);
                    snprintf(sdp_body, 4096, "v=%s\r\n"/*协议版本*/
                             "o=%s %s %s IN IP4 %s\r\n"/*会话源*//*用户名/会话ID/版本/网络类型/地址类型/地址*/
                             "s=Play\r\n"/*会话名*/
                             "c=IN IP4 %s\r\n"/*连接信息*//*网络类型/地址信息/多点会议的地址*/
                             "t=0 0\r\n"/*时间*//*开始时间/结束时间*/
                             "%s\r\n"/*媒体/端口/传送层协议/格式列表*/
                             "%s\r\n"
                             "a=sendonly\r\n"/*收发模式*/
                             "%s"
                             "a=rtpmap:96 PS/90000\r\n"/*净荷类型/编码名/时钟速率*/
                             /*"a=rtpmap:98 H264/90000\r\n"
                             "a=rtpmap:97 MPEG4/90000\r\n"
                             "a=streamprofile:0\r\n"*/
                             "y=%d\r\n"/*标识发送流时的 RTP SSRC 值*/
                             "f=v/%d/6/15/2/%d/a///"/*"f=v/2/6/0/0/fa0/0/0/0\r\n"*/,
                             sdp_message_v_version_get(sdp_body_rcv),
                             sdp_message_o_username_get(sdp_body_rcv),
                             sdp_message_o_sess_id_get(sdp_body_rcv),
                             sdp_message_o_sess_version_get(sdp_body_rcv),
                            // sdp_message_a_att_value_get (sdp_body_rcv,pos_media, pos)
                             localip,
                             localip,
                             m,
                             streamType,
                             tcp_a,
                             ssrc,
                             sipVideoType, stH264Config.bps
                            );
                    eXosip_lock(excontext);
                    //printf("%s\n", sdp_body);
                    //osip_message_set_contact(asw_msg, "<sip:34020000001110000001@192.168.3.178:5060>");
                    osip_message_set_body(asw_msg, sdp_body, strlen(sdp_body));/*设置SDP消息体*/
                    osip_message_set_content_type(asw_msg, "application/sdp");
                    eXosip_call_send_answer(excontext,g_event->tid, 200, asw_msg);/*按照规则回复200OK with SDP*/
                    eXosip_unlock(excontext);
                    sdp_message_free(sdp_body_rcv);
                    free(sdp_buf);
                }
            }
            break;
            case EXOSIP_CALL_ACK:/*ACK*/
            {
                printf("\r\n<EXOSIP_CALL_ACK> line : %d, id:%d\r\n", __LINE__, g_event->did);
                /*实时视音频点播*/
                /*历史视音频回放*/
                /*视音频文件下载*/
                printf("\r\n<EXOSIP_CALL_ACK>\r\n");/*收到ACK才表示成功建立连接*/
                ite_eXosip_parseInviteBody(excontext,g_event,SipConfig);/*解析INVITE的SDP消息体，同时保存全局INVITE连接ID和全局会话ID*/
            }
            break;

			case EXOSIP_CALL_ANSWERED:
			{
                printf ("EXOSIP_CALL_ANSWERED call_id is %d, dialog_id is %d \n", g_event->cid, g_event->did);
				if(!g_event->response)
				{
                	printf ("#########EXOSIP_CALL_ANSWERED response is NULL!\n");
					break;
				}

                if(MSG_IS_RESPONSE(g_event->response))
                {
	                osip_message_t *ack = NULL;
					eXosip_lock(excontext);
	                eXosip_call_build_ack (excontext,g_event->did, &ack);
	                eXosip_call_send_ack (excontext,g_event->did, ack);
					eXosip_unlock(excontext);

					ite_eXosip_parseInviteBodyForAudio(excontext,g_event,SipConfig);
                }
			}
			break;

            case EXOSIP_CALL_CLOSED:/*BEY*/
            {
                /*实时视音频点播*/
                /*历史视音频回放*/
                /*视音频文件下载*/
                int ret;
                printf("\r\n<EXOSIP_CALL_CLOSED>\r\n");
                osip_message_t *bye_answer=NULL;
                char dtmf_body[1024]={0};
                printf("\r\n<EXOSIP_CALL_CLOSED> line : %d\r\n", __LINE__);
                if (NULL == g_event->request)
                {
                    printf("\r\n<EXOSIP_CALL_CLOSED> line : %d\r\n", __LINE__);
                    break;
                }
                else
                {
                    printf("\r\n<EXOSIP_CALL_CLOSED> line : %d\r\n", __LINE__);
                    if (g_event->request != NULL && MSG_IS_BYE(g_event->request))
                    {
                        printf("\r\n<EXOSIP_CALL_CLOSED> line : %d\r\n", __LINE__);
                        printf("<MSG_IS_BYE> event_id:%d,play:%d,playback:%d,download:%d\r\n",
                            g_event->did, g_did_realPlay, g_did_backPlay, g_did_fileDown);
                        char localTime[64] = {0};
                        int zone = 0;
                        char msgStr[1024] = {0};
                        osip_call_id_t *callid = osip_message_get_call_id(g_event->request);
                        netcam_sys_get_local_time_string(localTime,&zone);
                        
                        snprintf(msgStr, sizeof(msgStr),"callId=%s,endTime=%s",
                                callid->number, localTime);
                    
                        #ifdef MODULE_SUPPORT_UDP_LOG
                        mojing_log_send(NULL, 0, "gb28181_bye", msgStr);
                        #endif
                        
                        ProcessMediaStop(g_event->did);
                        

                        if((0 != g_did_backPlay) && (g_did_backPlay == g_event->did)) /*历史视音频回放*/
                        {
                            /*关闭：历史视音频回放*/
                            printf("backPlay closed success!\r\n");
							gk_gb28181_playback_set_stop_cmd(1);
                            g_did_backPlay = 0;
                        }
                        else if((0 != g_did_fileDown) && (g_did_fileDown == g_event->did)) /*视音频文件下载*/
                        {
                            /*关闭：视音频文件下载*/
                            printf("fileDown closed success!\r\n");

							gk_gb28181_playback_set_stop_cmd(0);
                            g_did_fileDown = 0;
                        }
#if 0
                        else if((0 != g_did_audioPlay) && (g_did_audioPlay == g_event->did)) /*视音频文件下载*/
                        {
                            /*关闭：语音播报*/
                            printf("VoiceCall closed success!\r\n");
                            ProcessMediaStop(g_event->did);
                            g_did_audioPlay = 0;
                        }
#endif
                        if((0 != g_call_id) && (0 == g_did_realPlay) && (0 == g_did_backPlay) && (0 == g_did_fileDown)) /*设置全局INVITE连接ID*/
                        {
                            printf("call closed success!\r\n");
                          //  g_call_id = 0;
                        }
                    }
                    else
                    {
                        printf("g_event->request is null\n");
                        break;
                    }
                }
                eXosip_lock (excontext);
                ret = eXosip_call_build_answer (excontext,g_event->tid, 200, &bye_answer);
                if (ret == OSIP_SUCCESS)
                {
                    osip_message_set_body (bye_answer, dtmf_body, strlen(dtmf_body));
                    eXosip_call_send_answer (excontext,g_event->tid, 200, bye_answer);
                    eXosip_call_terminate(excontext,g_event->cid,g_event->did);
                }
                eXosip_unlock(excontext);
                printf("\r\n<EXOSIP_CALL_CLOSED> line : %d, id:%d\r\n", __LINE__, g_event->did);
            }
            break;
            case EXOSIP_CALL_MESSAGE_NEW:/*MESSAGE:INFO*/
            {
                /*历史视音频回放*/
                printf("\r\n<EXOSIP_CALL_MESSAGE_NEW>\r\n");
                if(MSG_IS_INFO(g_event->request))
                {
                    osip_body_t *msg_body = NULL;

                    printf("<MSG_IS_INFO>\r\n");
                    osip_message_get_body(g_event->request, 0, &msg_body);
                    if(NULL != msg_body)
                    {
                        eXosip_call_build_answer(excontext,g_event->tid, 200, &g_answer);/*Build default Answer for request*/
                        eXosip_call_send_answer(excontext,g_event->tid, 200, g_answer);/*按照规则回复200OK*/
                        printf("eXosip_call_send_answer success!\r\n");
                        ite_eXosip_parseInfoBody(excontext,g_event);/*解析INFO的RTSP消息体*/
                    }
                }
            }
            break;
            case EXOSIP_CALL_MESSAGE_ANSWERED:/*200OK*/
            {
                /*历史视音频回放*/
                /*文件结束时发送MESSAGE(File to end)的应答*/
                printf("\r\n<EXOSIP_CALL_MESSAGE_ANSWERED>\r\n");
            }
            break;
            case EXOSIP_IN_SUBSCRIPTION_NEW:/*移动设备位置数据订阅*/
            {
                printf("\r\n<EXOSIP_IN_SUBSCRIPTION_NEW>\r\n");
                if(MSG_IS_SUBSCRIBE(g_event->request))
                {
                    eXosip_lock(excontext);
                    eXosip_insubscription_build_answer(excontext,g_event->tid, 200, &g_answer);/*Build default Answer for request*/
                    eXosip_insubscription_send_answer(excontext,g_event->tid, 200, g_answer);/*按照规则回复200OK*/
                    printf("eXosip_call_send_answer success!\r\n");
                    eXosip_unlock(excontext);
                    ite_eXosip_ProcessMsgBody(excontext,g_event,SipConfig);/*解析MESSAGE的XML消息体，同时保存全局会话ID*/
                }
            }
            break;
            case EXOSIP_MESSAGE_REQUESTFAILURE:
            {
                int length;
                char *message = NULL;
                printf("\r\n<EXOSIP_MESSAGE_REQUESTFAILURE>\r\n");
                if (g_event->response != NULL)
                {
                    osip_message_to_str(g_event->response, &message, &length);
                    if (message)
                    {
                        printf("%s\r\n", message);
                        free(message);
                        message = NULL;
                    }
                }
            }
            break;
            /*其它不感兴趣的消息*/
            default:
            {
                printf("\r\n<OTHER>\r\n");
                printf("eXosip event type:%d\n", g_event->type);
            }
            break;
        }
        if(g_event)
			eXosip_event_free(g_event);
    }

	printf("\nite_eXosip_processEvent_Task Exit!\n");

    return NULL;
}

int ite_eXosip_invit(struct eXosip_t *excontext,sessionId * id, char * to, char * sdpMessage, char * responseSdp,IPNC_SipIpcConfig * SipConfig)
{
    osip_message_t *invite;
    char localip[128] ={0};

    //int port = 5070;
    int i;// optionnal route header
    char to_[100];
    snprintf (to_, 100, "sip:%s", to);
    char from_[100];
    eXosip_guess_localip(excontext,AF_INET, localip, 128);

    snprintf (from_, 100,"sip:%s:%d", localip ,SipConfig->sipDevicePort);
    //snprintf (tmp, 4096, "");
    /*i = eXosip_call_build_initial_invite (excontext,&invite,
            "sip:34020000001180000002@192.168.17.1:5060",
            "sip:34020000001320000001@192.168.17.1:5060",
            NULL,
            "34020000001320000001:1,34020000001180000002:1" );*/
    i = eXosip_call_build_initial_invite (excontext,&invite,
                                          to_,
                                          from_,
                                          NULL,
                                          "This is a call for a conversation" );
    //i = eXosip_call_build_initial_invite (excontext,&invite,"<sip:user2@192.168.17.128>",   "<sip:user1@192.168.17.128>",NULL,  "This is a call for a conversation" );
    if (i != 0)
    {
        return -1;
    }
    //osip_message_set_supported (invite, "100rel");
    {
        //char tmp[4096];
        char localip[128];
        eXosip_guess_localip (excontext,AF_INET, localip, 128);
        //strcpy(localip,device_info.ipc_ip);

        i = osip_message_set_body (invite, sdpMessage, strlen (sdpMessage));
        i = osip_message_set_content_type (invite, "APPLICATION/SDP");
    }
    eXosip_lock (excontext);
    i = eXosip_call_send_initial_invite (excontext,invite);

    if (i > 0)
    {
        //eXosip_call_set_reference (i, "ssss");
    }
    eXosip_unlock (excontext);
    int flag1 = 1;
    while (flag1)
    {
        eXosip_event_t *je;
        je = eXosip_event_wait (excontext,0, 200);

        if (je == NULL)
        {
            printf ("No response or the time is over!\n");
            break;
        }

        switch (je->type)
        {
            case EXOSIP_CALL_INVITE:
                printf ("a new invite reveived!\n");
                break;
            case EXOSIP_CALL_PROCEEDING:
                printf ("proceeding!\n");
                break;
            case EXOSIP_CALL_RINGING:
                printf ("ringing!\n");
                //printf ("call_id is %d, dialog_id is %d \n", je->cid, je->did);
                break;
            case EXOSIP_CALL_ANSWERED:
                printf ("ok! connected!\n");
                printf ("call_id is %d, dialog_id is %d \n", je->cid, je->did);
                id->cid = je->cid;
                id->did = je->did;
                osip_body_t *body;
                osip_message_get_body (je->response, 0, &body);
                //printf ("I get the msg is: %s\n", body->body);
                //(*responseSdp)=(char *)malloc (body->length*sizeof(char));
                snprintf (responseSdp, body->length, "%s", body->body);

                //response a ack
                osip_message_t *ack = NULL;
                eXosip_call_build_ack (excontext,je->did, &ack);
                eXosip_call_send_ack (excontext,je->did, ack);
                flag1 = 0;
                break;
            case EXOSIP_CALL_CLOSED:
                printf ("the other sid closed!\n");
                break;
            case EXOSIP_CALL_ACK:
                printf ("ACK received!\n");
                break;
            default:
                printf ("other response!\n");
                break;
        }
        eXosip_event_free (je);

    }
    return 0;

}

int ite_eXosip_bye(struct eXosip_t *excontext,sessionId id)
{
    eXosip_call_terminate(excontext, id.cid, id.did);
    return 0;
}


int  ite_eXosip_keep_alive(struct eXosip_t *excontext,IPNC_SipIpcConfig *Sipconfig)
{
    int ret = 0;
    osip_message_t *kalive_msg=NULL;
    char rsp_xml_body[4096];
    static int SN=0;
    char from[128];
    char to[128];
    char localip[128]={0};

    memset(from, 0, 128);
    memset(to, 0, 128);
    eXosip_guess_localip (excontext,AF_INET, localip, 128);

    sprintf(from, "sip:%s@%s:%d", Sipconfig->sipDeviceUID,localip,Sipconfig->sipDevicePort);
    sprintf(to, "sip:%s@%s:%d",Sipconfig->sipRegServerUID,Sipconfig->sipRegServerIP, Sipconfig->sipRegServerPort);

    eXosip_message_build_request(excontext,&kalive_msg, "MESSAGE", to, from, NULL);/*构建"MESSAGE"请求*/
    snprintf(rsp_xml_body, 4096, "<?xml version=\"1.0\" encoding=\"GB2312\"?>\r\n"
                 "<Notify>\r\n"
                 "<CmdType>Keepalive</CmdType>\r\n"
                 "<SN>%d</SN>\r\n"
                 "<DeviceID>%s</DeviceID>\r\n"
                 "<Status>OK</Status>\r\n"
                 "</Notify>\r\n",
                SN++,
                Sipconfig->sipDeviceUID);

    printf("ite_eXosip_keep_alive ---times %d\n", SN);

    osip_call_id_t *callid = osip_message_get_call_id(kalive_msg);
    printf("callid number:%s\n", callid->number);
    memset(g_keepalive_callid, 0, 128);
    strcpy(g_keepalive_callid, callid->number);

    eXosip_lock (excontext);
    osip_message_set_body(kalive_msg, rsp_xml_body, strlen(rsp_xml_body));
    osip_message_set_content_type(kalive_msg, "Application/MANSCDP+xml");
    ret = eXosip_message_send_request(excontext,kalive_msg);

    eXosip_unlock (excontext);
    printf("ite_eXosip_keep_alive ret:%d --- send \n", ret);

    return 0;
}



/*
    心跳线程
*/
void * ite_eXosip_Kalive_Task(void *pData)
{
    int error_alive = 0;

    sdk_sys_thread_set_name("gb_kalive");
    struct eXosip_t *excontext = ((ite_gb28181Obj *)pData)->excontext;
    IPNC_SipIpcConfig *Sipconfig = ((ite_gb28181Obj *)pData)->sipconfig;
    //unsigned int startTime=(unsigned int)time(NULL);
    pthread_detach(pthread_self());

    while((g_bterminate == ITE_OSAL_FALSE))
    {
        if (Sipconfig->sipDevHBCycle_s <= 0)
        {
            Sipconfig->sipDevHBCycle_s = 55;
        }
        if (g_register_status)
        {
            g_keepalive_flag = 1;//发送心跳记录标志1
            error_alive = ite_eXosip_keep_alive(excontext,Sipconfig);
        }
        else
        {
            sleep(1);
            continue;
        }
        if (error_alive == -1)
        {
            printf("GBT28181  error %d --- %d\n", error_alive, Sipconfig->sipDevHBCycle_s);
            break;
        }
        sleep(Sipconfig->sipDevHBCycle_s);
        if (g_register_status)
        {
            if (1 == g_keepalive_flag)//等待一个心跳周期后，检测flag状态
            {
                g_keepalive_outtimes++;//还是1代表没有收到心跳响应，则记录心跳超时次数++
                printf("keep alive out times : %d\n", g_keepalive_outtimes);
            }
            else
            {
                g_keepalive_outtimes = 0;//是0代表收到心跳响应，则清空心跳超时次数
            }
            if (Sipconfig->sipDevHBOutTimes <= g_keepalive_outtimes)
            {
                g_keepalive_flag = 0;
                g_keepalive_outtimes = 0;
                g_register_status = 0;//重置注册标志，重新注册
                ProcessMediaStopAll();//重置实时播放标志位，不在传输视频流
                printf("keep alive g_register_status : %d\n", g_register_status);
                gb28181_set_sip_info();
            }
        }
    }

	printf("\nite_eXosip_Kalive_Task Exit!\n");

    return (void *)NULL;
}

int ite_eXosip_get_register_status(void)
{
	return g_register_status;
}

/*
    服务处理线程
*/

void *ite_eXosip_register_Task(void *pData)
{
    struct eXosip_t *excontext = ((ite_gb28181Obj *)pData)->excontext;
    IPNC_SipIpcConfig *Sipconfig = ((ite_gb28181Obj *)pData)->sipconfig;
    int  reg=0;//RandomS;
    int reg_time = 0;
    struct timeval ts;

    pthread_detach(pthread_self());
    sdk_sys_thread_set_name("gb_register");

    //RandomS = (ts.tv_usec%30);
    //if(RandomS<10)RandomS+=10;
    while(g_bterminate == ITE_OSAL_FALSE)
    {
        if(0 == g_register_status)
        {
            printf("g_register_status:%d\n", g_register_status);
            if (strcmp(Sipconfig->sipDeviceUID, "0000000000000000000") != 0)
            {
                reg = uac_register(excontext,Sipconfig);
                if (0 != reg)
                {
                    /*注册失败,则重新注册*/
                    sleep(30);
                    continue;
                }
                else
                {
                    gettimeofday(&ts,NULL);
                    reg_time = Sipconfig->regExpire_s + ts.tv_sec;//记录有效期后的时间
                    printf("Sipconfig->regExpire_s:%d, ts.tv_sec:%d, reg_time : %d\n", Sipconfig->regExpire_s, (int)ts.tv_sec, reg_time);
                }
            }
            else
            {
                printf("g_register_status id:%s, not regist\n", Sipconfig->sipDeviceUID);
                sleep(60);
            }
        }
        else
        {
            gettimeofday(&ts,NULL);
            //printf("ts.tv_sec:%d, reg_time - 120:%d\n", ts.tv_sec, reg_time - 120);

            if (0 == Sipconfig->regExpire_s)/*注销一次后不再检测时间*/
            {
                continue;
            }
            /*检测注册有效期前120秒,再次进行注册,设置标志位0*/
            if (reg_time - 120 < ts.tv_sec)
            {
                g_register_status = 0;
                gb28181_set_sip_info();
                printf("111Sipconfig->regExpire_s:%d, ts.tv_sec:%d, reg_time : %d\n", Sipconfig->regExpire_s, (int)ts.tv_sec, reg_time);
            }
            sleep(1);//等待1s，避免线程空转消耗CPU
        }
    }

	printf("\nite_eXosip_register_Task Exit!\n");

    return NULL;
}

/*GB28181主函数*/
void ite_eXosip_launch(struct eXosip_t *excontext,IPNC_SipIpcConfig * SipConfig)
{

    struct timezone Ztest;
    struct tm timeNow;
    struct timeval systime;
    gettimeofday(&systime,&Ztest);
    localtime_r(&systime.tv_sec, &timeNow);
    char localIP[128];
    getlocalip(localIP);

    static  char eXosip_device_name[100]         = "IPC";
    static  char eXosip_device_manufacturer[30] = "GK";
    static  char eXosip_device_model[30]        = "GB28181";
    static  char eXosip_device_firmware[30]     = "V1.0";
    static  char eXosip_device_encode[10]       = "ON";
    static  char eXosip_device_record[10]       = "OFF";

    static  char eXosip_status_on[10]           = "ON";
    static  char eXosip_status_ok[10]           = "OK";
    static  char eXosip_status_online[10]       = "ONLINE";
    static  char eXosip_status_guard[10]        = "ONDUTY";
    static char eXosip_status_time[30]         = {0};

    device_info.device_name         = eXosip_device_name;
    device_info.device_manufacturer = eXosip_device_manufacturer;
    device_info.device_model        = eXosip_device_model;
    device_info.device_firmware     = eXosip_device_firmware;
    device_info.device_encode       = eXosip_device_encode;
    device_info.device_record       = eXosip_device_record;

    device_status.status_on         = eXosip_status_on;
    device_status.status_ok         = eXosip_status_ok;
    device_status.status_online     = eXosip_status_online;
    device_status.status_guard      = eXosip_status_guard;

    sprintf(eXosip_status_time, "%04d-%02d-%02dT%02d:%02d:%02d",
                                                timeNow.tm_year + 1900, timeNow.tm_mon + 1, timeNow.tm_mday,
                                                timeNow.tm_hour, timeNow.tm_min, timeNow.tm_sec);
    device_status.status_time       = eXosip_status_time;

    device_info.server_ip           = SipConfig->sipRegServerIP;
    //device_info.server_port         = SipConfig->sipRegServerPort;
    device_info.ipc_id              = SipConfig->sipDeviceUID;
    device_info.ipc_pwd             = SipConfig->sipRegServerPwd;
    device_info.ipc_ip              = localIP;
    //device_info.ipc_port            = SipConfig->sipDevicePort;

    device_info.ipc_alarmid = SipConfig->sipAlarmID;
    static char temp_voiceid[] = "34020000001370000003";
    device_info.ipc_voiceid = temp_voiceid;//SipConfig->sipVoiceID;
    printf("device_info.ipc_alarmid : %s\n", device_info.ipc_alarmid);

    /*init struct data for (config download && device config)*/
    memset(&g_roi_info, 0, sizeof(struct _ite_roi_info));
    memset(&g_svac_audio_param, 0, sizeof(struct _ite_svac_audio_param));
    memset(&g_dec_surveillance_param, 0, sizeof(struct _ite_surveillance_param));
    memset(&g_enc_surveillance_param, 0, sizeof(struct _ite_surveillance_param));
    memset(&g_dec_svc_param, 0, sizeof(struct _ite_svc_param));
    memset(&g_enc_svc_param, 0, sizeof(struct _ite_svc_param));
    g_enc_svc_param.svc_space_supportmode = 3;
    g_enc_svc_param.svc_time_supportmode = 3;
    g_dec_svc_param.svc_space_supportmode = 3;
    g_dec_svc_param.svc_time_supportmode = 3;

    memset(&g_presetlist_param, 0, sizeof(struct _ite_presetlist_param));
    strcpy(g_alarm_method, "5");/*报警方式/1为电话报警/2为设备报警/3为短信报警/4为GPS报警/5为视频报警/6为设备故障报警/7其它报警*/

    printf("ite_eXosip_launch --end \n");

}

#if 0
OS_S32 ite_gb28181_init_comm(OS_HANDLE *pt_data)
{
    OS_S32 i = 0, ret = 0;

    ITE_OSAL_MSG_INFO_T st_sys_msg_info = {0};

    if (ITE_MESSAGE_Create(ITE_CONFIG_INFO_KEY, &st_sys_msg_info, 0) < 0)
    {
        printf("Create the Aval Osal Message failed\n");
        return ITE_OSAL_FAIL;
    }
    ret = ITE_SHM_Get_Addr(st_sys_msg_info.shmid, pt_data);
    if (pt_data == NULL)
    {
        printf("%s %d ITE_SHM_Get_Addr failed\n", __func__, __LINE__);
        return ITE_OSAL_FAIL;
    }

    return 0;
}
#endif

