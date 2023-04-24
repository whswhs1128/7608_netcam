#ifndef CC1310MGR_H
#define CC1310MGR_H

#if defined( __cplusplus)
extern "C" {
#endif


#define MAX_OBJ 256
typedef struct JS_HEAP_OBJ_MGR{
    int num;
    cJSON * Json[MAX_OBJ];
}JS_HEAP_OBJ_MGR;

extern int nTimeOutMS; //太长了传输会很慢; 太短了吃cpu性能也不可取
#define TIMEOUT_DOWNLOAD 30 //高速读
#define TIMEOUT_JSON 200 //低速读
#define TIMEOUT_IDLE 800 //空闲poll速度
#define MAX_CONN_NUM 12 //一次性处理连接的个数; 太少了的话, 由于保活时间, 会频繁地拒绝掉.


    //协议头:
#define KEY_CC_LIST    "cc_list"
#define KEY_CC_ALARM   "cc_alarm"
#define KEY_CC_ADD     "cc_add"
#define KEY_CC_DEL     "cc_delete"
#define KEY_CC_WAKEUP  "cc_wakeup"
#define KEY_CC_SETWIFI "cc_setwifi"
#define KEY_PIR        "pir"
#define KEY_HUMANTRACE  "humantrace"
#define KEY_NETLED      "netled"
    //协议中的字段:
#define KEY_SENSORID    "id"
#define KEY_SENSORLOC   "location"
#define KEY_ACCEPT      "accept"
#define KEY_HANDLEBYAPP "handledbyapp"
#define KEY_ALARMING    "alarming"
#define KEY_TYPE        "type"
#define KEY_SSID        "ssid"
#define KEY_PASSWD      "passwd"
#define KEY_HT_SWITCH    "isOn"
    //20180820 增加pir字段中的key
#define KEY_OP_GET "get"
#define KEY_OP_SET "set"
#define KEY_PIR_SWITCH  "on-off"
#define KEY_PIR_SPANTIME  "time"
    //20180913 增加alarm-action字段中的key
#define KEY_ALARM_ACTION "alarm_action"
#define KEY_AA_PIRENABLE "PirEnable"
#define KEY_AA_MOTIONENABLE "MotionEnable"
#define KEY_AA_MOTIONSENSITIVE "MotionSensitive"
#define KEY_AA_FIGUREPILTERENABLE "FigureFilterEnable"
#define KEY_AA_STAYDEAYTIME       "StayDelayTime"
#define KEY_AA_ALARMAUDIOTYPE     "AlarmAudioType"
#define KEY_AA_ACTION             "Action"
#define KEY_AA_ACTION_START       "StartTime"
#define KEY_AA_ACTION_END         "EndTime"
#define KEY_AA_ACTIONRECORD       "ActionRecord"
#define KEY_AA_ACTIONSOUND        "ActionSound"
#define KEY_AA_ACTIONLIGHT        "ActionLight"
#define KEY_AA_ACTIONV12          "ActionV12"
#define KEY_AA_ACTIONPUSHUP       "ActionPushup"


//这里是所有传输协议
    typedef enum _USERDEFINE_PROTOCOLTYPE
    {
        USRDEF_CC_LIST = 1, //
        USRDEF_CC_ADD,
        USRDEF_CC_DEL,
        USRDEF_CC_ALARM,
        USRDEF_CC_WAKEUP, //唤醒某个cc
        USRDEF_CC_SETWIFI, //对某个cc设置wifi
        USRDEF_PIR, //20180820 补充对PIR的透传协议
        USRDEF_ALARM_ACTION, //20180913
        USRDEF_DOWNLOAD, //20180915
        USRDEF_HUMANTRACE, //20190124
        USRDEF_NETLED, //20190225
        USRDEF_AF_NOTIFY, //20190401
    }USERDEFINE_PROTOCOLTYPE;

    typedef enum _CONN_STATUS
    {
        CONN_FINISH = 10,
    }CONN_STATUS;

    typedef struct STRU_CONN_INFO{
        int hconnid; //连接id
        int iType; //这个连接是干什么的: USERDEFINE_PROTOCOLTYPE
        int nconnStatus; //填一个值主动加快关闭
    }STRU_CONN_INFO;

    typedef struct USERDEF_CMD{
        int iType;
        const char * szName;
    }USERDEF_CMD;
#define SES_BUF_SIZE 2048

    //每一个传感器需要保存的信息
    typedef struct SENSOR_NODE{
        char id[32]; //传感器的macid
        char szLocation[128]; //该传感器所在地址名称
        int bAccepted;
        int bHandledByApp; //是否已经被app处理过
        int bAlarming; //是否正在报警
        char sztype[2]; //一个字符表示类型, 一个表示结尾符0
    }SENSOR_NODE;
#define MAX_SENSOR_NODE 4 //传感器最大sensor个数
    extern SENSOR_NODE struSenseorNode[MAX_SENSOR_NODE];

    //=================================================
    //=================================================
    //=================================================
    //=================================================

#define KEY_NEWID   'n'
#define KEY_EXIT    'e'
#define KEY_ALERT   'a'
#define KEY_STATUS  's'
#define KEY_DELETE  'd'
#define KEY_WAKEUP  'u'
#define KEY_SETWIFI 'w'
    typedef struct CC_CMD{
        int iType;
        char cName;
    }CC_CMD;

    typedef enum _CC_PROTOCOLTYPE
    {
        //从CC接受的内容
        CC_NEWID, //有新ID上报
        CC_EXIT, //删除id
        CC_ALERT,  //有报警发出
        CC_STATUS,
        //发送给cc的内容
        CC_DELETE, //唤醒室外机
        CC_WAKEUP, //唤醒室外机
        CC_SETWIFI, //设置wifi信息
    }CC_PROTOCOLTYPE;

void glnk_cc_switch_init();
void glnk_cc_test();
int glnk_led_control_state(void);

void test_cc();

int cc_get_newid(char *pbuf, int len); //cc主动加入
int cc_get_exit(char *pbuf, int len); //cc主动离开
int cc_get_alarming(char *pbuf, int len); //cc主动报警

int cc_set_delete(char * szDelId); //dev主动删除cc某id
int cc_set_wakeup(char * szDelId); //dev主动唤醒cc某id
int cc_set_wifi(char * szDelId, char * szssid, char * szpasswd); //dev主动设置wifi, 针对全体cc
void cc_send_one_msg(char *pbuf, int len);

int MP4Sys_Set_PWR(int bEnable,int time);
int MP4Sys_Get_PWR(int *state);

int MPU_SetAutoHumanTrace(int bEnable); //下发自动人形追踪开关
int MPU_GetAutoHumanTrace();
int MPU_GetNetLed();
int MPU_SetNetLed(int bshow);

char * JsPrintFmt(cJSON * Json);
char * JsPrintStr(cJSON * Json);
char * JsToBuffer(cJSON * Json, char * pbuf);
void JsHeapObjFree(JS_HEAP_OBJ_MGR * pJsMgr);
cJSON *cJSON_CreateObjectEx(JS_HEAP_OBJ_MGR * pJsMgr);
cJSON *cJSON_CreateArrayEx(JS_HEAP_OBJ_MGR * pJsMgr);
cJSON *cJSON_ParseEx(JS_HEAP_OBJ_MGR * pJsMgr, const char *value);

extern STRU_CONN_INFO s_hOldConnID[MAX_CONN_NUM];

#if defined( __cplusplus)
};
#endif

#endif
