//
// Created by 姹磱 on 2019-07-25.
//

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "protool.h"
#include "product.h"
#include "tcpserver.h"
#include <errno.h>
#include "openssl/aes.h"
#include "devsdk.h"
#include "plugsdk.h"
#include "Md5.h"
#include "plugupgrade.h"

#define VERSION_0   0x01
#define VERSION_1   0x00
#define VERSION_2   0x01
#define VERSION_3   0x00

#define netcam_exit(valu) {\
	int exit_valu = valu;\
	printf("netcam_exit ---> %s, %d \n", __func__, __LINE__);\
	exit(exit_valu);\
}

extern PLUG_CONFIG local_cfg;
time_t opt_time;
struct PRO_PARA
{
    int   login_status;          // 登录状态标识 0:未登录（初始化状态）；1:登录成功
    int   protool_status;        // 协议状态标识 0:初始化；1:重新登录；2:退出
    int   heart_interval;        // 心跳间隔
    long  heart_time;            // 当前心跳时间

    int   link_status;           // 连接/接入状态标识 0:未连接（初始化状态）；1:连接成功
    int   link_port;             // 连接/接入服务器端口
    char  link_server[16];       // 连接/接入服务器IP
    char  link_tocken[32];       //
    char  link_plugin_id[32];    //

    struct CONFIG_PARA
    {
        char  pin_code[64];
        char  sn_code[128];
        char  protool_key[32];
        char  login_server[32];  // 登录服务器IP
        char  login_port[16];    // 登录服务器端口
    } conf; // 配置文件参数

    pthread_mutex_t protool_mutex;
} pro_para;



void init_para()
{
    time(&opt_time);
    pro_para.login_status = 0;
    pro_para.link_status = 0;
    pro_para.protool_status = 0;
    pro_para.heart_interval = 10;
    pro_para.heart_time = 0;
    pro_para.link_port = 0;
    bzero(&pro_para.link_server, sizeof (pro_para.link_server));
    bzero(&pro_para.link_tocken, sizeof (pro_para.link_tocken));
    bzero(&pro_para.link_plugin_id, sizeof (pro_para.link_plugin_id));
    bzero(&pro_para.conf, sizeof (pro_para.conf));
}

#define PRINTHEX 1
void printhex(unsigned char *src,int len, const char * func_name)
{
#ifdef PRINTHEX
    printf("######## func: %s len = %d\n", func_name, len);
    if(src==NULL)
    {
        return;
    }
    if(len>(1024*3-1))
    {
        return;
    }
    char x[1024*3]={0};
    int i=0;
    for(i=0;i<len;i++)
    {
        char tmp[10]={0};
        {
            snprintf(tmp,8,"%02X ",src[i]);
            strcat(x,tmp);
        }
    }
    printf("%s\n",x);
#endif
    return;
}

int get_link_status()
{
    return pro_para.link_status;
}

unsigned char get_check_sum(unsigned char *pack, int pack_len)
{
    unsigned char check_sum =0;
    while(--pack_len>=0)
    {
        check_sum += *pack++;
    }
    return check_sum;
}

void new_protool()
{
    pthread_mutex_init(&pro_para.protool_mutex, NULL);
    init_para();
    new_program_profile(CONFIG_FIE);
}

void delete_protool()
{
    pthread_mutex_destroy(&pro_para.protool_mutex);
    printf("pthread_mutex_destroy::Protool\n");

    init_para();
    delete_program_profile();
}



void read_conf()
{
    get_key_value_str(APP_DEV, KEY_PIN, DEF_EMPTY, pro_para.conf.pin_code, sizeof(pro_para.conf.pin_code));//PIN码固定12位
    get_key_value_str(APP_DEV, "SN", DEF_EMPTY, pro_para.conf.sn_code, sizeof(pro_para.conf.sn_code));
    get_key_value_str(APP_PORTOOL, KEY_KEY, DEF_EMPTY, pro_para.conf.protool_key, sizeof(pro_para.conf.protool_key));
    get_key_value_str(APP_PORTOOL, KEY_SERVER, DEF_SERVER_IP, pro_para.conf.login_server, sizeof(pro_para.conf.login_server));//服务器的域名或IP地址
    get_key_value_str(APP_PORTOOL, KEY_PORT, DEF_SERVER_PORT, pro_para.conf.login_port, sizeof(pro_para.conf.login_port));

    printf("config para:----\n");
    printf("pin_code-----%s----%d\n",pro_para.conf.pin_code,(int)strlen(pro_para.conf.pin_code));
    printf("sn_code-----%s----%d\n",pro_para.conf.sn_code,(int)strlen(pro_para.conf.sn_code));
    printf("protool_key-----%s----%d\n",pro_para.conf.protool_key,(int)strlen(pro_para.conf.protool_key));
    printf("login_server-----%s----%d\n",pro_para.conf.login_server,(int)strlen(pro_para.conf.login_server));
    printf("login_port-----%s----%d\n",pro_para.conf.login_port,(int)strlen(pro_para.conf.login_port));
}

void login_process()
{
    int ret;
    pthread_t login_thread_id;

    /// new login tcp
    int login_port = atoi(pro_para.conf.login_port);
    while (0 != new_tcp_server(pro_para.conf.login_server, login_port)){
        printf("######## ERROR: new_tcp_server failed.\n\n");
        sleep(10);
    };

    /// connect to login tcp server
    while(pro_para.protool_status == 0)
    {
        get_ip(pro_para.conf.login_server);
        if(0 != init_tcp_server())
        {
            printf("mTcpConnectServer->Init() error\n");
            sleep(10);
            continue;
        }
        break;
    }
    //this step means: connected login_status tcp server

    /// create recv and response thread to handle recv data
    if(0!=running())
    {
        printf("mTcpConnectServer->Running() error !!!!!\n");
        printf("########exit at [%s] %d \n\n", __func__, __LINE__);
        exit (-1);
    }

    /// register the callback function
    register_cb(0, rec_cb);

    /// create login thread
    ret = pthread_create(&login_thread_id, NULL, login_thread, 0);
    if( 0 != ret )
    {
        printf(  "Unable to create a thread for login_status thread %s error !!!!!",
                 strerror( errno ) ) ;
        printf("########exit at [%s] %d \n\n", __func__, __LINE__);
        netcam_exit(90);
    }
}

int init_protool()
{
    /// init para
    init_para();

    /// read conf para
    read_conf();

    /// obtain big end or litter end
    get_end_type();

    /// login process
    login_process();

    return 0;
}

int destroy_protool()
{
    pro_para.protool_status = 2;
    sleep(1);
    destroy_tcp_server();
    delete_tcp_server();

    return 0;
}


void * login_thread(void *arg)
{
    login_handle();
    return 0;
}

void login_handle()
{
    int ret = 0;
    int i = 0,j = 0;
    pthread_t checkThreadId;

    printf("Login %d,%d\n", pro_para.login_status, pro_para.protool_status);
    while((!pro_para.login_status)&&(!pro_para.protool_status))
    {
        login_service();
        for(i = 0; i < 60; i++)
        {
            if(pro_para.protool_status || pro_para.login_status)
                break;
            sleep(1);
        }
    }

    /// reinit tcp
    destroy_tcp_server();
    delete_tcp_server();

    printf("quit login_status,start link_status %s:%d\n", pro_para.link_server, pro_para.link_port);

    /// new link tcp
    if(0 != new_tcp_server(pro_para.link_server, pro_para.link_port)){
        printf("######## ERROR: new_tcp_server failed.\n\n");
    }

    /// connect to link tcp server, 5 times
    for(i = 0; i < 5; i++)
    {
        get_ip(pro_para.link_server);
        ret = init_tcp_server();
        if(0!=ret)
        {
            printf("mTcpConnectServer->Init() error2\n");
            sleep(5);
            continue;
        }
        else
            break;
    }

    if(i == 5||pro_para.protool_status)
    {
        printf("link_status failed !\n");
        destroy_tcp_server();
        delete_tcp_server();
        if(pro_para.protool_status != 2)
            init_protool();
        return;
    }

    ret = running();

    if(0!=ret)
    {
        printf("mTcpConnectServer->Running() error exit !!!!!\n");
        printf("########exit at [%s] %d \n\n", __func__, __LINE__);
        netcam_exit(90);
    }

    /// register the callback function
    register_cb(0, rec_cb);

    for(i = 0; i < 3; i++)
    {
        link_service();

        for( j = 0; j < 60; j++)
        {
            if((pro_para.protool_status)||(pro_para.link_status))
                break;
            sleep(1);
        }
        if((pro_para.protool_status)||(pro_para.link_status))
            break;
    }
    //printf("link_status break i = %d\n",i);
    if((i == 3)||(pro_para.protool_status))
    {
        printf("link_status failed !\n");
        destroy_tcp_server();
        delete_tcp_server();
        //delete mTcpConnectServer;
        if(pro_para.protool_status !=2)
            init_protool();
        return;
    }

    //鍥轰欢鐗堟湰淇℃伅涓婃姤
    for(i = 0; i < 3; i++){
        ret = dev_version_report();
        if (ret == 0){
            printf("######## dev_version_report done!\n\n");
            break;
        }
        sleep(1);
    }

    /*for(i = 0; i < 3; i++){
        sleep(1);
        ret = dev_fwversion_report();
        if (ret == 0){
            printf("######## dev_fwversion_report done!\n\n");
            break;
        }
    }*/

    ret = pthread_create(&checkThreadId, NULL, check_thread, 0);
    if( 0 != ret ) {

        printf(  "Unable to create a thread for checkThreadId thread %s exit !!!!!",
                 strerror( errno ) ) ;
        printf("########exit at [%s] %d \n\n", __func__, __LINE__);
        netcam_exit(90);
    }
    while(!pro_para.protool_status)
    {
        //sleep(mHeartInterval);
        for( i = 0;i < pro_para.heart_interval; i++)
        {
            if(pro_para.protool_status)
                break;
            sleep(1);
        }

        heart_beep();
    }

    destroy_tcp_server();
    delete_tcp_server();

    if(pro_para.protool_status == 1)
    {
        sleep(10);
        init_protool();
    }

}

int heart_beep()
{
    data_heartbeat_opt optdata = {0};
    time_t curtime;
    time(&curtime);
    optdata.timestamp = swapInt32(curtime);
    U8 checks = get_check_sum((unsigned char*)&optdata,sizeof(optdata));
    int ret = send_up_data((unsigned char *)&optdata,
                           (unsigned char *) 0,
                           heart,
                           ACK_YES,
                           sizeof(optdata),
                           0,
                           checks);
    return ret;
}

//
int login_service()
{

    char optdata[LOGIN_OPT_LENGTH + 1]; // 可选部数据
    char decdata[1024];       // 待加密数据
    int datalen = 0;          // 数据部长度
    char miwen_hex[1024];     // 加密后数据
    int milen = 0;            // 加密后数据长度
    U8 checkkdata[1024];      // 校验和数据内容

    U8 deviceSnLength = 0;    // sn长度

    memset(optdata, 0, LOGIN_OPT_LENGTH + 1);
    memset(decdata, 0, 1024);
    memset(miwen_hex, 0, 1024);
    memset(checkkdata, 0, 1024);

    /// 鍒ゆ柇pinc鍜宬ey
    if ((pro_para.conf.pin_code == NULL) || (strlen (pro_para.conf.pin_code) == 0)){
        printf("!\n!\n!\n!\n########ERROR: PIN is empty, plese check your plug.conf!\n!\n!\n!\n");
        netcam_exit(90);
    }
    if(ENCTYPE == 0){
        if ((pro_para.conf.protool_key == NULL) || (strlen (pro_para.conf.protool_key) == 0)){
            printf("!\n!\n!\n!\n########ERROR: KEY is empty, plese check your plug.conf!\n!\n!\n!\n");
            netcam_exit(90);
        }
    }

    /// 鑾峰彇sn
    if (0 != get_dev_sn(pro_para.conf.sn_code)){
        printf("!\n!\n!\n!\n########ERROR: SN is empty!\n!\n!\n!\n");
        netcam_exit(90);
    };
    // 璧嬪�約n闀垮害
    deviceSnLength = strlen(pro_para.conf.sn_code);

    /// 鑾峰彇mac
    U8 mac[6] = {0};//{0x66, 0x55, 0x44, 0x33, 0x22, 0x11};
    if (0 != memcmp(mac, local_cfg.mac, 6)){
        memcpy(mac, local_cfg.mac, 6);
    }
    else{
        get_dev_mac(mac);
    }
    printf("######## gwdev mac: %02x %02x %02x %02x %02x %02x \n\n",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    /// ------- 鍙�夐儴 --------
    optdata[0] = 2;
    memcpy(&optdata[3], pro_para.conf.pin_code, strlen(pro_para.conf.pin_code));


    /// ------- 寰呭姞瀵嗘暟鎹?--------
    // 鍓嶄袱涓瓧鑺傛槸鍖呭簭鍙穝eq
    // 璧嬪�紁incode
    memcpy(&decdata[PACKAGE_SEQ_LENGTH], pro_para.conf.pin_code, strlen(pro_para.conf.pin_code));
    // 璧嬪�約n闀垮害
    decdata[PACKAGE_SEQ_LENGTH + 12] = deviceSnLength;
    // 璧嬪�約n
    memcpy(&decdata[PACKAGE_SEQ_LENGTH + 12 + 1], pro_para.conf.sn_code, deviceSnLength);
    // 璧嬪�糾ac
    memcpy(&decdata[PACKAGE_SEQ_LENGTH + 13 + deviceSnLength], mac, 6);
    // 璁＄畻鏁版嵁閮ㄩ暱搴︼紝鍖呭ご + pincode + sn闀垮害 + sn + mac + deviceID
    datalen = PACKAGE_SEQ_LENGTH + 13 + deviceSnLength + 6 + 32;

    int ret = aes_encrypt((char*)pro_para.conf.protool_key, decdata, miwen_hex, datalen, &milen);
    if(ret != 0)
        return ret;
    //printf("aes_crypt_ecb encode: ----len %d---\n",dlen);

    /// ------- 鏍￠獙鍜?--------
    short midatalen = milen;
    midatalen = swapInt16(midatalen);
    memcpy(checkkdata, &midatalen, PACKAGE_DATA_LENGTH);
    memcpy(checkkdata + PACKAGE_DATA_LENGTH, optdata, LOGIN_OPT_LENGTH);
    memcpy(checkkdata + PACKAGE_DATA_LENGTH + LOGIN_OPT_LENGTH, decdata, datalen);

    U8 checksum = get_check_sum((unsigned char *) checkkdata, PACKAGE_DATA_LENGTH + LOGIN_OPT_LENGTH + datalen);

    return send_up_data((unsigned char *) optdata,
                        (unsigned char *) miwen_hex,
                        login_conform,
                        ACK_YES,
                        LOGIN_OPT_LENGTH,
                        midatalen,
                        checksum);

}

int link_service()
{
    data_link_opt optdata;    // 可选部
    data_link data;           // 数据部的包数据
    char decdata[1024];       // 待加密数据
    int datalen = 0;          // 数据部长度
    char miwen_hex[1024];     // 加密后数据
    int milen = 0;            // 加密后数据长度
    U8 checkkdata[1024];      // 校验和数据内容

    memset(&data, 0, sizeof(data));
    memset(decdata, 0, 1024);
    memset(miwen_hex, 0, 1024);
    memset(checkkdata, 0, 1024);

    if((!pro_para.login_status)&&(!pro_para.link_status))
        return -1;

    printf(">>>>>link_service command  \n");

    /// ------- 鍙�夐儴 --------

    memset(optdata.devicePin, 0, sizeof(optdata.devicePin));
    memcpy(optdata.devicePin, pro_para.conf.pin_code, 12);

    memset(optdata.token, 0, sizeof(optdata.token));
    memcpy(optdata.token, pro_para.link_tocken, 32);

    /// ------- 鏁版嵁閮?--------
    data.protocol = 0;
    data.deviceVersion[0] = VERSION_0;
    data.deviceVersion[1] = VERSION_1;
    data.deviceVersion[2] = VERSION_2;
    data.deviceVersion[3] = VERSION_3;

    // 计算数据部长度
    datalen = PACKAGE_SEQ_LENGTH + sizeof(data_link);

    /// ------- 寰呭姞瀵嗘暟鎹?--------
    memcpy(&decdata[2], &data, sizeof(data_link));

    int ret = aes_encrypt((char*)pro_para.conf.protool_key, decdata, miwen_hex, datalen, &milen);
    if(ret !=0)
        return ret;

    /// ------- 鏍￠獙鍜?--------
    short midatalen = milen;
    midatalen = swapInt16(midatalen);
    memcpy(checkkdata, &midatalen, PACKAGE_DATA_LENGTH);
    memcpy(checkkdata + PACKAGE_DATA_LENGTH, &optdata, sizeof(optdata));
    memcpy(checkkdata + PACKAGE_DATA_LENGTH + sizeof(optdata), &decdata, datalen);

    U8 checks = get_check_sum((unsigned char *) checkkdata, PACKAGE_DATA_LENGTH + sizeof(data_link_opt) + datalen);

    return send_up_data((unsigned char *) &optdata,
                        (unsigned char *) miwen_hex,
                        link_conform,
                        ACK_YES,
                        sizeof(optdata),
                        milen,
                        checks);

}

void * check_thread(void *arg)
{
    //Protool *pServer = (Protool *)arg;
    check_handle();
    return 0;
}

void check_handle()
{
    while(!pro_para.protool_status)
    {
        time_t curtime;
        time(&curtime);
        long updateTime = curtime;

        //姝ゅ瀵瑰績璺砤ck鐩戞祴

        if((pro_para.heart_time != 0)&&(updateTime - pro_para.heart_time) > pro_para.heart_interval*3)
        {
            pro_para.protool_status = 1;
            printf("heart beep error\n");
        }

        sleep(1);
    }
}

int rec_cb(void *powner, PACKAGE_HEAD phead, DATA_HEAD dhead, char *optbuff, char *buff, short seq)
{
    return cb_handle(phead, dhead, optbuff, buff, seq);
}

int cb_handle(PACKAGE_HEAD phead, DATA_HEAD dhead, char *optbuff, char *buff, short seq) {
    char out[1024];
    int i=0;
    int outlen;
    //printf(">>>>>>cb_handle seq %d\n",seq);

    if(seq ==-1)//force logout
    {
        printf("net error reconnect\n");
        pro_para.protool_status = 1;
        return 0;
    }

    switch ((phead.headp&0x1f))
    {
        case force_logout:
            printf("force disconnect exit !!!!!\n");
            printf("########exit at [%s] %d \n\n", __func__, __LINE__);
            netcam_exit(90);
            break;
        case heart_ack:
            printf("heart beep ack\n");
            time_t curtime;
            time(&curtime);
            pro_para.heart_time = curtime;
            break;
        case management_pack_ack:
        {
            printf("management_pack_ack %d\n",dhead.length);
            if((dhead.length>0))
            {
                int ret = aes_decrypt((char*)pro_para.conf.protool_key, buff, out, dhead.length, &outlen);
                if(ret !=0)
                {
                    printf("descrypt error\n");
                    break;
                }
                /*	for(int i =2;i<outlen;i++)
                    {
                        printf("%c ",out[i]);
                    }
                    printf("\n");*/
            }

            break;
        }
        case management_pack_down:
        {
            printf("management_pack_down\n");

            if(dhead.length>0)
            {
                int ret = aes_decrypt((char*)pro_para.conf.protool_key, buff, out, dhead.length, &outlen);
                if(ret !=0)
                {
                    printf("descrypt error\n");
                    break;
                }

                data_device_mt_req *data=(data_device_mt_req*)(out + 2);
                data->length = swapInt16(data->length);
                printf("management_pack_down datatype %d,len %d\n",
                       data->dataType, data->length);

                short len = data->length;

                if(ENCTYPE == 0)
                    outlen -=out[outlen-1];

                char* pData = (out+5);

                printf("len %d,outlen = %d,%d\n",dhead.length, outlen, out[outlen-1]);

                /*for(int i =2;i<outlen;i++)
                {
                    printf("%c ",out[i]);
                }
                printf("\n");*/
                char checkkdata[1024];
                memset(checkkdata, 0, 1024);
                memcpy(checkkdata, &dhead.length, 2);

                if(dhead.optLength)
                    memcpy(checkkdata+2,optbuff,dhead.optLength);
                if(outlen>0)
                    memcpy(checkkdata+2+dhead.optLength,out,outlen);

                U8 checks = get_check_sum((unsigned char *) checkkdata, 2 + dhead.optLength + outlen);
                printf("checks = %x, %x\n",checks,dhead.checksum);

                if (checks != dhead.checksum) {
                    send_management_ack_data(checksum_err, 0, 0);
                }
                else{
                    char response[1024*4];
                    int  responselen;
                    int r;
                    char str_data[1024] = {0};

                    switch (data->dataType){
                        case datatype_upgrade_cmd://涓嬪彂鍗囩骇鎸囦护
                            memcpy((void *)str_data, pData, data->length);
                            r = gw_parse_upgrade_cmd(str_data);

                            if (r==0) {
                                send_management_ack_data(0, (unsigned char *) response, responselen);
                            }
                            else {
                                char msg[]="6.5.1 error";
                                send_management_ack_data(r, (unsigned char *) msg, strlen(msg));
                            }
                            break;

                        case datatype_upgrade_param://涓嬪彂鍗囩骇鏁版嵁
                            memcpy((void *)str_data, pData, data->length);
                            r = gw_parse_upgrade_param(str_data);

                            if (r==0) {
                                send_management_ack_data(0, (unsigned char *) response, responselen);
                            }
                            else {
                                char msg[]="6.5.2 error";
                                send_management_ack_data(r, (unsigned char *) msg, strlen(msg));
                            }
                            break;
                        default:
                            send_management_ack_data(0, (unsigned char *) response, responselen);
                            break;
                    }
                }
            }

            break;
        }

        case bissness_pack_ack:
        {
            printf("bissness_pack_ack %d\n",dhead.length);
            if((dhead.length>0))
            {
                int ret = aes_decrypt((char*)pro_para.conf.protool_key, buff, out,dhead.length, &outlen);
                if(ret !=0)
                {
                    printf("descrypt error\n");
                    break;
                }
            }
            break;
        }

        case bissness_pack_down:
        {
            printf("bissness_pack_down\n");

            if(dhead.length>0)
            {
                int ret = aes_decrypt((char*)pro_para.conf.protool_key, buff, out, dhead.length, &outlen);
                if(ret != 0)
                {
                    printf("descrypt error\n");
                    break;
                }

                if(ENCTYPE == 0)
                    outlen -= out[outlen-1];

                char* pData = (out+2);
                short len = outlen - 2;
                pData[len]='\0';

                printf("len %d,outlen = %d,%02X\n",dhead.length, outlen, (unsigned char)out[outlen-1]);

                char  checkkdata[1024];
                memset(checkkdata,0,1024);
                memcpy(checkkdata, &dhead.length, 2);

                if(dhead.optLength)
                    memcpy(checkkdata + 2, optbuff, dhead.optLength);
                if(outlen>0)
                    memcpy(checkkdata + 2 + dhead.optLength, out, outlen);

                //printhex(checkkdata, 2+dhead.optLength+outlen, __func__);

                U8 checks = get_check_sum((unsigned char *) checkkdata, 2 + dhead.optLength + outlen);
                printf("checks = %x, %x\n",checks,dhead.checksum);

                if (checks != dhead.checksum) {
                    send_business_ack_data(checksum_err, "", 0);
                }
                else{
                    /*char msgadd[] = ",\"sequence\":123}\0";
                    memcpy(&pData[len-1], msgadd, strlen(msgadd));
                    len = len + strlen(msgadd) - 1;*/

                    int ret = 0;
                    char response[1024*4] = "";
                    int  responselen = 0;
                    ret = dev_data_down(pData, len);

                    /*sleep(2);
                    char updataT[100] = "{\"sequence\":0,\"SongControlS\":2}";
                    int data_len = swapInt16(strlen(updataT));

                    send_dev_data_up(updataT, data_len);*/
                }
            }

            break;
        }

        case ack:
        {
            if(!pro_para.login_status)
            {
                short errerno = *(short*)optbuff;
                if(dhead.optLength > 0)
                {
                    printf("######## optlen %d, err code or radom [%d]\n\n", dhead.optLength, swapInt16(errerno));
                }

                printf("login ack ===len %d\n",dhead.length);
                if(errerno == 0)
                {

                    int ret = aes_decrypt((char*)pro_para.conf.protool_key, buff, out, dhead.length, &outlen);
                    if(ret != 0)
                    {
                        printf("descrypt error\n");
                        break;
                    }
                    data_login_response* res = (data_login_response*)(out + PACKAGE_SEQ_LENGTH);
                    // session
                    memcpy(pro_para.link_tocken, res->token, 32);
                    // link IP
                    sprintf(pro_para.link_server,"%d.%d.%d.%d",res->ip[0],res->ip[1],res->ip[2],res->ip[3]);
                    // link port
                    pro_para.link_port = swapInt16(res->port);
                    // 鎻掍欢ID
                    memcpy(pro_para.link_plugin_id, res->deviceId, 32);
                    pro_para.login_status = 1;

                    printf("login ack======ip:%d,%d,%d,%d,port:%d,pluginId %s\n",
                           res->ip[0],res->ip[1],res->ip[2],res->ip[3],res->port, pro_para.link_plugin_id);


                    printf("login ok tocken ,port %d\n", pro_para.link_port);
                }
                else
                {
                    printf("login ack error\n");
                }
            }
            else
            {
                printf("link ack\n");
                short errerno = *(short*)optbuff;
                if (dhead.optLength>0)
                {
                    printf("######## optlen %d, err code or radom [%d]\n\n", dhead.optLength, errerno);
                }
                if (errerno == 0)
                {
                    int ret = aes_decrypt((char*)pro_para.conf.protool_key, buff, out, dhead.length, &outlen);
                    if(ret != 0)
                    {
                        printf("descrypt error\n");
                        break;
                    }

                    data_link_response* res = (data_link_response*)(out + PACKAGE_SEQ_LENGTH);

                    pro_para.heart_interval = swapInt16(res->heartbeat);
                    if(pro_para.heart_interval < 5)
                        pro_para.heart_interval = 5;
                    printf("link ack heartbeep interval %d\n", swapInt16(res->heartbeat));

                    pro_para.link_status = 1;

                }
                else
                {
                    printf("link ack error\n");

                    //鍒ゆ柇閿欒鐮侊紝閲嶆柊鍚姩鐧诲綍
                    pro_para.protool_status = 1;

                }
            }
            break;
        }

        default:
            break;
    }


    return 0;
}


int send_up_data(unsigned char *optdata, unsigned char *data, PACKAGE_TYPE type, int isack, int optlen, int len, U8 checks)
{
    time_t curtime;
    time(&curtime);
    long opt = opt_time;
    long cur = curtime;

    if(cur - opt == 0)
    {
        usleep(500000);
    }
    opt_time = curtime;

    PACKAGE_HEAD pk_head;
    DATA_HEAD dt_head;

    memset(&pk_head, 0, sizeof(pk_head));
    memset(&dt_head, 0, sizeof(dt_head));

    dt_head.optLength = optlen;

    if((len == 0)&&(optlen == 0))
        return -1;

    isack = (isack|(ENCTYPE<<1))&0x7;

    pk_head.magic = swapInt16(DX_MAGIC_HEAD);
    pk_head.headp = (type|(isack<<5));

    //printf("===%x\n",pk_head.headp);
    dt_head.checksum = checks;
    dt_head.length = swapInt16(len);


    char senddata[1024];
    memcpy(senddata, &pk_head, PACKAGE_HEAD_LENGTH);
    memcpy(senddata + PACKAGE_HEAD_LENGTH, &dt_head, DATA_HEAD_LENGTH);
    if((optlen)&&(optdata))
        memcpy(senddata + PACKAGE_HEAD_LENGTH + DATA_HEAD_LENGTH, optdata, optlen);
    if((len)&&(data))
        memcpy(senddata + PACKAGE_HEAD_LENGTH + DATA_HEAD_LENGTH + optlen, &data[0], len);

    int sendrt = send_data((unsigned char*)senddata, PACKAGE_HEAD_LENGTH + DATA_HEAD_LENGTH + optlen + len);
    if(sendrt == -1)
    {
        printf("mTcpConnectServer->sendData error\n");
        return -1;
    }
    return 0;
}

int send_business_up_data(void *buff, int len)
{
    int datalen;
    if(!pro_para.link_status)
        return -1;
    if((len>1022)||(len<=0))
        return -1;

    char  decdata[1024];
    char miwen_hex[1024];
    char checkkdata[1024];

    int milen = 0;

    memset(decdata, 0, 1024);
    memset(miwen_hex, 0, 1024);
    memset(checkkdata, 0, 1024);

    memcpy(&decdata[PACKAGE_SEQ_LENGTH],buff, len);
    datalen = PACKAGE_SEQ_LENGTH + len;

    int ret = aes_encrypt((char*)pro_para.conf.protool_key, decdata, miwen_hex, datalen, &milen);
    if(ret !=0)
        return ret;

    short midatalen = milen;
    midatalen = swapInt16(midatalen);
    memcpy(checkkdata, &midatalen, PACKAGE_DATA_LENGTH);
    memcpy(checkkdata + PACKAGE_DATA_LENGTH, decdata, datalen);

    U8 checks = get_check_sum((unsigned char*)checkkdata,PACKAGE_DATA_LENGTH + datalen);

    return send_up_data((unsigned char*)0,
                        (unsigned char*)miwen_hex,
                        bissness_pack_up,
                        ACK_YES,
                        0,
                        milen,
                        checks);

}

int send_management_up_data(void *buff, int len)
{
    int datalen;
    if(!pro_para.link_status)
        return -1;
    if((len>1022)||(len<=0))
        return -1;

    data_device_mo_req *p_dev_msg = (data_device_mo_req *)buff;

    char  decdata[1024];
    char miwen_hex[1024];
    char checkkdata[1024];

    int milen = 0;

    memset(decdata, 0, 1024);
    memset(miwen_hex, 0, 1024);
    memset(checkkdata, 0, 1024);


    memcpy(&decdata[PACKAGE_SEQ_LENGTH], &p_dev_msg->dataType, 1);
    U16 tmp = swapInt16(p_dev_msg->length);
    memcpy(&decdata[PACKAGE_SEQ_LENGTH + 1],&tmp,2);
    memcpy(&decdata[PACKAGE_SEQ_LENGTH + 3],p_dev_msg->dataByte, p_dev_msg->length);
    datalen = PACKAGE_SEQ_LENGTH + 3 + p_dev_msg->length;

    p_dev_msg->length = swapInt16(p_dev_msg->length);


    int ret = aes_encrypt((char*)pro_para.conf.protool_key,decdata,miwen_hex,datalen,&milen);
    if(ret != 0)
        return ret;

    short midatalen = milen;
    midatalen = swapInt16(midatalen);
    memcpy(checkkdata,&midatalen, PACKAGE_DATA_LENGTH);
    memcpy(checkkdata + PACKAGE_DATA_LENGTH, decdata, datalen);

    U8 checks = get_check_sum((unsigned char*)checkkdata, PACKAGE_DATA_LENGTH + datalen);

    return send_up_data((unsigned char*)0,
                        (unsigned char*)miwen_hex,
                        management_pack_up,
                        ACK_YES,
                        0,
                        milen,
                        checks);

}

int send_business_ack_data(short errcode, unsigned char *buff, int len)
{
    int datalen = 0;
    if(!pro_para.link_status)
        return -1;
    if(len>1022)
        return -1;

    char optdata[2];
    errcode = swapInt16(errcode);
    memcpy(optdata, &errcode, 2);

    if(len>0)
    {
        char decdata[1024];
        char miwen_hex[1024];
        char checkkdata[1024];
        int milen = 0;

        memset(decdata, 0, 1024);
        memset(miwen_hex, 0, 1024);
        memset(checkkdata, 0, 1024);

        memcpy(&decdata[PACKAGE_SEQ_LENGTH],buff,len);
        datalen = len;

        int ret = aes_encrypt((char*)pro_para.conf.protool_key, decdata, miwen_hex, datalen, &milen);
        if(ret != 0)
            return ret;

        short midatalen = milen;
        midatalen = swapInt16(midatalen);
        memcpy(checkkdata, &midatalen, PACKAGE_DATA_LENGTH);
        memcpy(checkkdata + PACKAGE_DATA_LENGTH, optdata, 2);
        memcpy(checkkdata + PACKAGE_DATA_LENGTH + 2, decdata, datalen);

        U8 checks = get_check_sum((unsigned char*)checkkdata, PACKAGE_DATA_LENGTH + 2 + datalen);

        return send_up_data((unsigned char*)optdata,
                            (unsigned char*)miwen_hex,
                            bissness_pack_ack,
                            ACK_NO,
                            2,
                            milen,
                            checks);
    }
    else
    {
        int dlen = 0;
        char checkkdata[1024];
        memset(checkkdata, 0, 1024);

        short datalen = dlen;
        datalen = swapInt16(datalen);
        memset(checkkdata, 0, 1024);
        memcpy(checkkdata, &datalen, PACKAGE_DATA_LENGTH);
        memcpy(checkkdata + PACKAGE_DATA_LENGTH, optdata, 2);


        U8 checks = get_check_sum((unsigned char*)checkkdata, PACKAGE_DATA_LENGTH + 2);
        return send_up_data((unsigned char*)optdata,
                            (unsigned char*)0,
                            bissness_pack_ack,
                            ACK_NO,
                            2,
                            0,
                            checks);
    }
}

int send_management_ack_data(short errcode, unsigned char *buff, int len)
{

    if(!pro_para.link_status)
        return -1;
    if(len > 1022)
        return -1;
    char optdata[2];
    errcode = swapInt16(errcode);
    memcpy(optdata, &errcode, 2);

    if(len > 0)
    {
        char decdata[1024];
        char miwen_hex[1024];
        char checkkdata[1024];
        int milen = 0;

        memset(decdata, 0, 1024);
        memset(miwen_hex, 0, 1024);
        memset(checkkdata, 0, 1024);

        memcpy(&decdata[PACKAGE_SEQ_LENGTH], buff, len);

        int datalen = PACKAGE_SEQ_LENGTH + len;

        int ret = aes_encrypt((char*)pro_para.conf.protool_key, decdata, miwen_hex, datalen, &milen);
        if(ret != 0)
            return ret;

        short midatalen = milen;
        midatalen = swapInt16(midatalen);
        memcpy(checkkdata, &midatalen, PACKAGE_DATA_LENGTH);
        memcpy(checkkdata + PACKAGE_DATA_LENGTH, optdata, 2);
        memcpy(checkkdata + PACKAGE_DATA_LENGTH + 2, decdata, datalen);

        U8 checks = get_check_sum((unsigned char*)checkkdata, PACKAGE_DATA_LENGTH + 2 + datalen);

        return send_up_data((unsigned char*)optdata,
                            (unsigned char*)miwen_hex,
                            management_pack_ack,
                            ACK_NO,
                            2,
                            milen,
                            checks);
    }
    else
    {
        int dlen = 0;
        char  checkkdata[1024];
        memset(checkkdata, 0, 1024);

        short datalen = dlen;
        datalen = swapInt16(datalen);
        memcpy(checkkdata, &datalen, PACKAGE_DATA_LENGTH);
        memcpy(checkkdata + PACKAGE_DATA_LENGTH, optdata, 2);

        U8 checks = get_check_sum((unsigned char*)checkkdata, PACKAGE_DATA_LENGTH + 2);

        return send_up_data((unsigned char*)optdata,
                            (unsigned char*)0,
                            management_pack_ack,
                            ACK_NO,
                            2,
                            dlen,
                            checks);
    }
}

/*
 * enckey锛氬瘑閽? * encbuf锛氬姞瀵嗘暟鎹? * decbuf锛氬姞瀵嗗悗鐨勬暟鎹? * inlen锛氬甫鍔犲瘑鏁版嵁闀垮害
 * outlen锛氬姞瀵嗗悗鏁版嵁闀垮害
 */
int aes_encrypt(char* enckey, char* encbuf, char* decbuf, int inlen, int* outlen)
{
    printhex(encbuf,inlen, __func__);

    AES_KEY aes;
#if 1
    if(ENCTYPE == 0)
    {
        char key[16]="";// = "12345678";
        memcpy(key, enckey, 16);
        char iv[16] = "";

        int nLen = inlen;//input_string.length();
        int nBei;

        if((!encbuf)||(!decbuf))
            return -1;
        nBei = nLen / AES_BLOCK_SIZE + 1;

        int nTotal = nBei * AES_BLOCK_SIZE;
        char *enc_s = (char*)malloc(nTotal);
        int nNumber;
        if (nLen % 16 > 0)
            nNumber = nTotal - nLen;
        else
            nNumber = 16;
        memset(enc_s, nNumber, nTotal);
        memcpy(enc_s, encbuf, nLen);

        if (AES_set_encrypt_key((unsigned char*)key, 128, &aes) < 0) {
            fprintf(stderr, "Unable to set encryption key in AES error !!!!!\n");
            free(enc_s);
            return -1;
        }

        AES_cbc_encrypt((unsigned char *)enc_s, (unsigned char*)decbuf, nBei * 16, &aes, (unsigned char*)iv, AES_ENCRYPT);
        //for(int i =0;i<(nBei * 16);i++)
        //	printf("%02X ", decbuf[i]&0xff);
        //printf("\n");

        * outlen = nBei * 16;
        free(enc_s);
    }
    else
#endif
    {
        memcpy(decbuf,encbuf,inlen);
        *outlen = inlen;
    }

    return 0;
}


int aes_decrypt(char* enckey, char* encbuf, char* decbuf, int inlen, int* outlen)
{


    AES_KEY aes;
#if 1
    if(ENCTYPE == 0)
    {
        char key[16]="";// = "12345678";
        memcpy(key, enckey, 16);
        char iv[16] = "";

        int nLen = inlen;//input_string.length();
        int nBei;

        if((!encbuf)||(!decbuf))
            return -1;
        nBei = nLen / AES_BLOCK_SIZE + 1;


        int nTotal = nBei * AES_BLOCK_SIZE;
        char *enc_s = (char*)malloc(nTotal);
        int nNumber;
        if (nLen % 16 > 0)
            nNumber = nTotal - nLen;
        else
            nNumber = 16;
        memset(enc_s, nNumber, nTotal);
        memcpy(enc_s, encbuf, nLen);

        if (AES_set_decrypt_key((unsigned char*)key, 128, &aes) < 0) {
            fprintf(stderr, "Unable to set decryption key in AES error !!!!!\n");
            free(enc_s);
            return -1;
        }

        AES_cbc_encrypt((unsigned char *)enc_s, (unsigned char*)decbuf, nBei * 16, &aes, (unsigned char*)iv, AES_DECRYPT);
        //for(int i =0;i<(nBei * 16);i++)
        //	printf("%02X ", decbuf[i]&0xff);
        //printf("\n");

        * outlen = nBei * 16-16;
        free(enc_s);
    }
    else
#endif
    {
        memcpy(decbuf,encbuf,inlen);
        *outlen = inlen;
    }

    printhex(decbuf,*outlen, __func__);

    return 0;

}

int compute_file_md5(const char *file_path, char *md5_str)
{
    int i;

    int ret;
    unsigned char data[READ_DATA_SIZE];
    unsigned char md5_value[MD5_SIZE];
    MD5_CTX md5;

    FILE* fd = fopen(file_path, "rb");
    if (fd <=0)
    {
        perror("fopen");
        return -1;
    }

    // init md5
    MD5Init(&md5);

    while (1)
    {
        // void * ptr, size_t size, size_t count, FILE * stream
        ret = fread(data, 1,READ_DATA_SIZE,fd);
        if (-1 == ret)
        {
            perror("fread");
            return -1;
        }

        MD5Update(&md5, data, ret);

        if (0 == ret || ret < READ_DATA_SIZE)
        {
            break;
        }
    }

    fclose(fd);

    MD5Final(&md5, md5_value);

    for(i = 0; i < MD5_SIZE; i++)
    {
        snprintf(md5_str + i*2, 2+1, "%02x", md5_value[i]);
    }
    md5_str[MD5_STR_LEN] = '\0'; // add end

    return 0;
}

int LogPlugStatus(ENUM_PLUG_LOG code)
{
    char log[8];

    sprintf(log,"%03d",code);
    FILE* fp = fopen(LOG_FILE,"wb");
    if(fp<=0)
        return -1;
    log[strlen(log)]=0x0a;
    fwrite(log,1,4,fp);
    fclose(fp);
    return 0;
}
