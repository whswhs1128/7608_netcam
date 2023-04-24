
#ifndef ITE_SIP_API_H
#define ITE_SIP_API_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <ite_typedef.h>

#include <osipparser2/osip_message.h>
#include <osipparser2/osip_parser.h>
#include <osipparser2/osip_port.h>
//#include <eXosip2/eXosip2.h>
#include <eXosip2/eXosip.h>
#include <eXosip2/eX_setup.h>
#include <eXosip2/eX_register.h>
#include <eXosip2/eX_options.h>
#include <eXosip2/eX_message.h>
#include <osip2/osip_mt.h>

//#include "ITENetSDK.h"
#include "itg_config_info.h"

#ifdef MODULE_SUPPORT_MOJING
#include "mojing_cloud_storage.h"
#endif

#define GB_MSG_ALARM_TYPE 0x10
typedef struct _ite_alarm_t{
    int  method; /*为发报警定义的变量0不发报警，1为AlarmMethod视频报警, 2为AlarmMethod设备报警*/
    int  type;   /*报警类型，2为移动侦测 6为区域入侵*/
    char time[32]; //时间格式2020-09-30T14:00:00
    char para[128]; 
} ite_alarm_t;

typedef struct _gb28181_msg_queue_t{
    long mtype;
    ite_alarm_t mmsg;
}gb28181_msg_queue_t;

typedef struct _sessionId
{
    int cid;
    int did;

} sessionId;


typedef struct _ite_gb28181Obj
{
    struct eXosip_t *excontext ;
    IPNC_SipIpcConfig *sipconfig;
} ite_gb28181Obj;


typedef struct _ite_rtp_Info
{
    char localip[64];
    char remoteip[64];
    unsigned short videoPort;
    unsigned short audioPort;
    unsigned int enOverTcp;
    unsigned int mediaChannel;
    int veRtpSock;
    int veRtcpSock;
    int auRtpSock;
    int auRtcpSock;

    unsigned int playLoadType;
    int ssrc;
    char *frameBuffer;
    int frameBufferSize;
    int eventId;
    char rtpType;  //play:0,  download:1, playback:2, audio:3, cloudstorage 4,
    char isUsed;
    char isRun;

#ifdef MODULE_SUPPORT_MOJING
	CloudStorageControlT cloudCtrl;
#endif
    int selfCloudTestFd;
	unsigned int rtpResv[56];
}ite_rtp_Info;

struct _device_info/*设备信息结构体*/
{
    char *server_id;/*SIP服务器ID*//*默认值：34020000002000000001*/
    char *server_ip;/*SIP服务器IP地址*//*默认值：192.168.1.178*/
    char *server_port;/*SIP服务器IP端口*//*默认值：5060*/

    char *ipc_id;/*媒体流发送者ID*//*默认值：34020000001180000002*/
    char *ipc_pwd;/*媒体流发送者密码*//*默认值：12345678*/
    char *ipc_ip;/*媒体流发送者IP地址*//*默认值：192.168.1.144*/
    char *ipc_port;/*媒体流发送者IP端口*//*默认值：6000*/

    char *ipc_alarmid;/*报警通道*/
    char *ipc_voiceid;/*语音输出通道*/

    char *device_name;/*设备/区域/系统名称*//*默认值：IPC*/
    char *device_manufacturer;/*设备厂商*//*默认值：iTarge*/
    char *device_model;/*设备型号*//*默认值：GB28181*/
    char *device_firmware;/*设备固件版本*//*默认值：V1.0*/
    char *device_encode;/*是否编码*//*取值范围：ON/OFF*//*默认值：ON*/
    char *device_record;/*是否录像*//*取值范围：ON/OFF*//*默认值：OFF*/
    char *device_gbsn;/*设备串码*/
    char *device_mac;/*设备Mac地址*/
    char *device_ctei;/*设备Ctei*/
} device_info;
struct _device_status/*设备状态结构体*/
{
    char *status_on;/*设备打开状态*//*取值范围：ON/OFF*//*默认值：ON*/
    char *status_ok;/*是否正常工作*//*取值范围：OK/ERROR*//*默认值：OFF*/
    char *status_online;/*是否在线*//*取值范围：ONLINE/OFFLINE*//*默认值：ONLINE*/
    char *status_guard;/*布防状态*//*取值范围：ONDUTY/OFFDUTY/ALARM*//*默认值：OFFDUTY*/
    char *status_time;/*设备日期和时间*//*格式：xxxx-xx-xxTxx:xx:xx*//*默认值：2012-12-20T12:12:20*/
} device_status;
enum _device_control
{
    EXOSIP_CTRL_RMT_LEFT = 1,/*向左*/
    EXOSIP_CTRL_RMT_RIGHT,   /*向右*/
    EXOSIP_CTRL_RMT_UP,      /*向上*/
    EXOSIP_CTRL_RMT_DOWN,    /*向下*/
    EXOSIP_CTRL_RMT_LARGE,   /*放大*/
    EXOSIP_CTRL_RMT_SMALL,   /*缩小*/
    EXOSIP_CTRL_RMT_STOP,    /*停止遥控*/
    EXOSIP_CTRL_REC_START,   /*开始手动录像*/
    EXOSIP_CTRL_REC_STOP,    /*停止手动录像*/
    EXOSIP_CTRL_GUD_START,   /*布防*/
    EXOSIP_CTRL_GUD_STOP,    /*撤防*/
    EXOSIP_CTRL_ALM_RESET,   /*报警复位*/
    EXOSIP_CTRL_TEL_BOOT,    /*设备远程启动*/
};

/*回调函数*/
struct _ite_eXosip_callback
{
    /*获取设备信息*/
    /*device_info：设备信息结构体指针*/
    /*返回值：成功时返回0，失败时返回负值*/
    int (*ite_eXosip_getDeviceInfo)(struct _device_info *device_info);

    /*获取设备状态*/
    /*device_info：设备状态结构体指针*/
    /*返回值：成功时返回0，失败时返回负值*/
    int (*ite_eXosip_getDeviceStatus)(struct _device_status *device_status);

    /*获取录像文件的起始时间与结束时间*/
    /*时间格式：xxxx-xx-xxTxx:xx:xx*/
    /*period_start：录像时间段起始值*/
    /*period_end：录像时间段结束值*/
    /*start_time：当前返回录像文件的起始时间*/
    /*end_time：当前返回录像文件的结束时间*/
    /*返回值：成功时返回符合时间段条件的剩余录像文件数量，失败时返回负值*/
    int (*ite_eXosip_getRecordTime)(char *period_start, char *period_end, char *start_time, char *end_time);

    /*设备控制：向左、向右、向上、向下、放大、缩小、停止遥控/开始手动录像、停止手动录像/布防、撤防/报警复位/设备远程启动*/
    /*ctrl_cmd：设备控制命令，_device_control类型的枚举变量*/
    /*返回值：成功时返回0，失败时返回负值*/
    int (*ite_eXosip_deviceControl)(enum _device_control ctrl_cmd);

    /*媒体控制：实时点播/回放/下载*/
    /*control_type：媒体控制类型，实时点播/Play，回放/Playback，下载/Download*/
    /*media_ip：媒体服务器IP地址*/
    /*media_port：媒体服务器IP端口*/
    /*返回值：成功时返回0，失败时返回负值*/
    int (*ite_eXosip_mediaControl)(char *control_type, char *media_ip, char *media_port);

    /*播放控制：播放/快放/慢放/暂停*/
    /*control_type：播放控制，播放/快放/慢放/PLAY，暂停/PAUSE*/
    /*play_speed：播放速度，1为播放，大于1为快放，小于1为慢放*/
    /*pause_time：暂停时间，单位为秒*/
    /*range_start：播放范围的起始值*/
    /*range_end：播放范围的结束值*/
    /*返回值：成功时返回0，失败时返回负值*/
    int (*ite_eXosip_playControl)(char *control_type, char *play_speed, char *pause_time, char *range_start, char *range_end);
} ite_eXosip_callback;

/*被调函数*/
/*启动并注册eXosip*/
void ite_eXosip_launch(struct eXosip_t *excontext,IPNC_SipIpcConfig * SipConfig);

int  ite_eXosip_init(struct eXosip_t *excontext,IPNC_SipIpcConfig * SipConfig);/*初始化eXosip*/
/*注册eXosip*/
/*expires：重新注册的超时时间，单位为秒*/
/*返回值：成功时返回0，失败时返回负值*/
int ite_eXosip_register(struct eXosip_t *excontext,IPNC_SipIpcConfig * SipConfig,int expires);

/*注销eXosip*/
/*返回值：成功时返回0，失败时返回负值*/
int ite_eXosip_unregister(struct eXosip_t *excontext,IPNC_SipIpcConfig * SipConfig);

/*发送MESSAGE：报警*/
/*时间格式：xxxx-xx-xxTxx:xx:xx*/
/*alarm_time：报警时间*/
void ite_eXosip_sendEventAlarm(struct eXosip_t *excontext,ite_alarm_t *alarm_time,IPNC_SipIpcConfig * SipConfig);

/*发送MESSAGE：文件结束*/
void ite_eXosip_sendFileEnd(struct eXosip_t *excontext,IPNC_SipIpcConfig * SipConfig,int session_id);

/* send invite message */
//int ite_eXosip_sendinvite(char * to,char * sdpMessage, char ** responseSdp,sessionId * id);//by jzw
int ite_eXosip_invit(struct eXosip_t *excontext,sessionId * id, char * to, char * sdpMessage, char * responseSdp,IPNC_SipIpcConfig * SipConfig);
/* send bye message */
int ite_eXosip_bye(struct eXosip_t *excontext,sessionId id);

void *ite_eXosip_register_Task(void *pData);

void * ite_eXosip_Kalive_Task(void *pData);
void *ite_eXosip_processEvent_Task(void *pData);/*消息循环处理*/
void *ite_eXosip_NotifyAlarm_Task(void *pData);
void *ite_eXosip_PositionAlert_Task(void *pData);
void *ite_eXosip_ftp_upload_log_Task(void *pData);
//OS_S32 ite_gb28181_init_comm(OS_HANDLE *pt_data);
int ite_logUploadParam_init(void *pData);
int ite_eXosip_log_bplus_tree_init(void);
int ite_eXosip_log_bplus_tree_deinit(void);
#endif //ITE_SIP_API_H


