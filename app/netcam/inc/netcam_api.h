#ifndef NETCAM_API_H
#define NETCAM_API_H
#ifdef __cplusplus
extern "C" {
#endif

#include "sdk_api.h"
#include "sdk_network.h"
#include "sdk_ptz.h"
#include "sdk_debug.h"
#include "cfg_all.h"


#define MAX_ITEM_NUM	30

#define NETCAM_TIMER_HALF_ONE_SEC  2 // the timer is 250ms for one cycle, 250ms * 2  = 500ms
#define NETCAM_TIMER_ONE_SEC       4 // the timer is 250ms for one cycle, 250ms * 4  = 1s
#define NETCAM_TIMER_TWO_SEC       8 // the timer is 250ms for one cycle, 250ms * 8  = 2s
#define NETCAM_TIMER_FOUR_SEC       16 // the timer is 250ms for one cycle, 250ms * 8  = 2s

#define LOCAL_ETH_NAME 	"eth0"
//#define WIFI_NAME 		"wlan0"


#define   RESOURCE_DIR "/opt/resource"
#define   RESOURCE_FONT_DIR  RESOURCE_DIR"/font"
#define   RESOURCE_WEB_DIR  RESOURCE_DIR"/web"
#define   RESOURCE_DANA_DEFAULT_DIR  RESOURCE_DIR"/dana"

#define	  RESOURCE_SNAPSHOT_DIR	RESOURCE_WEB_DIR"/snapshot"

#define AUDIO_RES_PATH_PRE "/opt/resource/audio"
#define HINT_SUCCESS	"success.alaw"
#define HINT_FAIL		"fail.alaw"
#define HINT_USE		"use.alaw"
#define HINT_WIRE		"wire.alaw"
#define HINT_WIRELESS	"wireless.alaw"
#define HINT_DISCONNECT	"disconnect.alaw"
#define HINT_TF_MOUNT	"TFmount.alaw"
#define HINT_TF_NOT_MOUNT "TFunmount.alaw"
#define HINT_SYS_RECOVERY "RecoverySuccess.alaw"
#define HINT_WIFIPW   "wifipw.alaw"
#define HINT_CONNECT  "connect.alaw"

#define AUDIO_FILE_MD_ALARM	"md_alarm.alaw"
#define AUDIO_FILE_STARTING "dev_starting.alaw"
#define AUDIO_FILE_START_FINISH "dev_start_finish.alaw"
#define AUDIO_FILE_WIFI_NORMAL  "wifi_normal.alaw"
#define AUDIO_FILE_WIFI_SMART_LINK "wifi_smart_link.alaw"

#define AUDIO_FILE_ALERT_DEFAULT_1 "alert_default1.alaw"
#define AUDIO_FILE_ALERT_DEFAULT_2 "alert_default2.alaw"
#define HINT_CONFIG_WIFI  "please_config_wifi.alaw"

extern void add_p2p_info(char *name,char *ID);
typedef enum
{
	SYSTEM_OPERATION_REBOOT = 0,
	SYSTEM_OPERATION_RESTART_APP,
	SYSTEM_OPERATION_HARD_DEFAULT,
	SYSTEM_OPERATION_SOFT_DEFAULT,
}NETCAM_SYS_OPERATION;

typedef enum SYS_MD_SENSITIVITY
{
	SYS_MD_SENSITIVITY_INVALID = -1,
	SYS_MD_SENSITIVITY_LOWEST = 0,
	SYS_MD_SENSITIVITY_LOW,
	SYS_MD_SENSITIVITY_MEDIUM,
	SYS_MD_SENSITIVITY_HIGH,
	SYS_MD_SENSITIVITY_HIGHEST,
}SYS_MD_SENSITIVITY_t;


typedef struct
{
    int bufId;
    int streamId;
    int maxStreamNum;
}NETCAM_VIDEO_STREAM_Porpety;



typedef enum
{
	NETCAM_AUDIO_ENC_RAW_PCM,
	NETCAM_AUDIO_ENC_A_LAW,
	NETCAM_AUDIO_ENC_MU_LAW,
	NETCAM_AUDIO_ENC_AD_PCM,
}NETCAM_AUDIO_ENC_TYPE_t;

typedef enum
{
	SYSTEM_AUDIO_HINT_LINK_SUCCESS = 0,
	SYSTEM_AUDIO_HINT_LINK_FAIL,
	SYSTEM_AUDIO_HINT_USE_WIRE,
	SYSTEM_AUDIO_HINT_USE_WIRELESS,
	SYSTEM_AUDIO_HINT_DISCONNECT_FAIL,
	SYSTEM_AUDIO_HINT_DISCONNECT_SUCCESS,
	SYSTEM_AUDIO_HINT_SD_NOT_MOUNT,
	SYSTEM_AUDIO_HINT_SD_MOUNT,
	SYSTEM_AUDIO_HINT_RECOVERY_SUCCESS,
	SYSTEM_AUDIO_HINT_GET_WIFIPW,
} NETCAM_AUDIO_HintEnumT;

typedef enum gkPIC_SIZE_E
{
    PIC_CIF,
    PIC_360P,      /* 640 * 360 */
    PIC_VGA,
    PIC_640x360,
    PIC_D1_PAL,    /* 720 * 576 */
    PIC_D1_NTSC,   /* 720 * 480 */
    PIC_720P,      /* 1280 * 720  */
    PIC_1080P,     /* 1920 * 1080 */
    PIC_2304x1296,
    PIC_2560x1440,
    PIC_2592x1520,
    PIC_2592x1536,
    PIC_2592x1944,
    PIC_2688x1520,
    PIC_2688x1536,
    PIC_2688x1944,
    PIC_2716x1524,
    PIC_3840x2160,
    PIC_4096x2160,
    PIC_3000x3000,
    PIC_4000x3000,
    PIC_7680x4320,
    PIC_3840x8640,
    PIC_BUTT
} PIC_SIZE_E;


#define UPDATE_START    1
#define UPDATE_SUCCESS  2
#define UPDATE_FAIL     3
typedef void (*cbFunc)(int stat);


#define netcam_exit(valu) {\
	int exit_valu = valu;\
	printf("netcam_exit ---> %s, %d \n", __func__, __LINE__);\
	exit(exit_valu);\
}



int netcam_video_web_stream_init(NETCAM_VIDEO_STREAM_Porpety *streamPro, int streamNum);

void netcam_image_init(void);
int netcam_image_get_map(GK_CFG_MAP *imageArr, int mapSize);
int netcam_image_get(GK_NET_IMAGE_CFG *imagAttr);
int netcam_image_set(GK_NET_IMAGE_CFG imagAttr);
int netcam_image_set2(GK_NET_IMAGE_CFG imagAttr, int flag);

void netcam_image_cfg_save(void);
void netcam_image_reset_default(void);
void netcam_iamge_ircut_auto_check(void);

void netcam_autolight_init(void);
int netcam_autolight_get(GK_AUTO_LIGHT_CFG *autolightAttr);
int netcam_autolight_set(GK_AUTO_LIGHT_CFG autolightAttr);
void netcam_autolight_cfg_save(void);

void netcam_video_init(void);
int netcam_video_get_channel_number();
int netcam_video_get_all_channel_par(GK_NET_VIDEO_CFG *pstConfigs);
int netcam_video_snapshot(int width, int height, char *filePath, EM_GK_ENC_SNAPSHOT_QUALITY quality);
int netcam_video_get_map(GK_CFG_MAP video2Array[][MAX_ITEM_NUM], int maxStreamSize, int maxItemSize);
int netcam_video_get(int vin_id, int stream_id, PS_GK_ENC_STREAM_H264_ATTR h264Attr);
int netcam_video_set(int vin_id, int stream_id, PS_GK_ENC_STREAM_H264_ATTR h264Attr);
void netcam_video_cfg_save(void);
void netcam_video_exit(void);
char *netcam_video_get_cfg_json_string(int stream_id);
int netcam_video_get_actual_fps(int stream_id);

//间隔一定时间后，调用该函数检测编码是否正常工作
int netcam_video_check_alive();


/* 音频初始化 */
int netcam_audio_init();
/* 音频退出处理 */
int netcam_audio_exit();

/* 添加音频写者 */
void netcam_audio_add_writers();

/* 让硬件输出 NETCAM_AUDIO_ENC_TYPE_t 格式的声音 */
int netcam_audio_output(char *buffer, int len, NETCAM_AUDIO_ENC_TYPE_t type, int isBlock);
/* 播报信息，网络状况等 */
int netcam_audio_hint(NETCAM_AUDIO_HintEnumT hint);
/* 播报信息 */
int netcam_audio_out(void* arg);
/* 魔镜播放音频，文件存储位置特殊*/
int netcam_audio_out_mojing(void* arg);
/* 播报传入的IP地址 */
int netcam_audio_ip(const char* ipaddress);
int netcam_audio_get_pcm_db(const unsigned char *pcmdata, int size); 
int netcam_audio_get_db(void); 

/* 在消回音过程中，不断从 audiobuf 读取数据，如果没有数据，则发空包。开始这个流程 */
int audio_send_start();
/* 在消回音过程中，不断从 audiobuf 读取数据，如果没有数据，则发空包。结束这个流程 */
int audio_send_stop();
/* 在消回音过程中，向 audiobuf 写入数据 */
int audiobuf_write_frame(void *writerid, void *data, int size);
/* 在消回音过程中，从 audiobuf 读出数据 */
int audiobuf_read_frame(void *readerid, void **data, int *size);
void netcam_audio_mute(int enable);



int netcam_adjust_bps(int stream_id, int bps);
int netcam_adjust_fps(int stream_id, int fps);
int netcam_venc_set_h264_qp_config(int stream_id);



//timer
typedef void (*TIMER_HANDLER)(void);
typedef void (*TIMER_HANDLER2)(void *lPara, void *pPara);
//初始化定时器任务，精度250ms，主要用来刷新时间显示，以及一些短时间内完成的任务
int  netcam_timer_init();
void netcam_timer_destroy();
//添加一个任务，interl间隔时间，为1表示间隔250ms，2间隔500ms ...， immediate，TRUE立即执行，FALSE，下次执行，
//oneshot: TRUE,只执行一次，FALSE循环执行，直到调用netcam_timer_del_task
int netcam_timer_add_task(TIMER_HANDLER const task_handler, int interl, SDK_BOOL immediate,SDK_BOOL oneshot);
int netcam_timer_add_task2(TIMER_HANDLER2 const task_handler, int interl,SDK_BOOL immediate,SDK_BOOL oneshot, void *lPara, void *pPara);
void netcam_timer_del_task(TIMER_HANDLER const task_handler);
void netcam_timer_del_task2(TIMER_HANDLER2 const task_handler);



//mintinus -720,+720 china is: 8*60
void netcam_sys_init();
void netcam_sys_set_time_zone_by_utc_string(char *buf, int zone);
void netcam_sys_set_time_zone_by_utc_second(int utcOffset, int zone);
void netcam_sys_get_local_time_string(char *localTimeStr, int *timeZone);
void netcam_sys_get_DevInfor(GK_NET_DEVICE_INFO *pstDevInfo);
void netcam_sys_use_timer_init();
void netcam_sys_save();
void netcam_sys_operation(void *fps,void *operation);
int netcam_sys_ntp_start_stop(int enNTP);
int netcam_sys_ntp_get(int *enNTP, char *ntpserver, int serLen, int *port);
int netcam_sys_ntp_set(char *ntpserver, int serLen, int port);
void netcam_sys_reboot(int second);

// http web and onvif
//注册web需要用到的cgi响应函数
void netcam_http_web_init();
//注册onvif需要用到的cgi响应函数
int netcam_http_onvif_init(void);

// osd
//加载OSD字库，根据runChannelCfg中各参数项的配置，初始化OSD并显示OSD中各项， clock每1秒更新一次
int netcam_osd_init(void);
// 应用修改runChannelCfg中各参数项的值，再调用netcam_osd_update_xxx就可以刷新OSD内容
void netcam_osd_update_clock();
void netcam_osd_update_title();
void netcam_osd_update_id();
//单独更新某路编码的title
void netcam_osd_set_title(int ch, char *title);
//单独更新某路编码的id
void netcam_osd_set_id(int ch, char *id);
//结束osd显示，释放资源
int netcam_osd_deinit(void);
int netcam_osd_update_id_rect(float x,float y, float w, float h,int enable, char *text);

// 获取channel 信息
int  netcam_osd_get_info(int id,GK_NET_CHANNEL_INFO *channelInfo);

//隐私遮挡初始化
void netcam_pm_init( void );
// 设置隐私遮挡，id为编码流号，0-3， cover为设置的参数
int  netcam_pm_set_cover( int vin, int id, GK_NET_SHELTER_RECT cover );
int  netcam_pm_get_cover( int vin, int id, GK_NET_SHELTER_RECT *cover );
void netcam_pm_exit_cover(void);

//pm的osd的参数
void netcam_osd_pm_save(void);
char *netcam_osd_cfg_json_string(int channelId);
char *netcam_pm_get_cfg_json_string(int channelId);
int netcam_osd_text_copy(char *dsttext, char *text, int dstlen);


//network
int netcam_net_init(void);
int netcam_net_set(ST_SDK_NETWORK_ATTR *net_attr);
int netcam_net_get(ST_SDK_NETWORK_ATTR *net_attr);
int netcam_net_get_all_Infor(ST_SDK_NETWORK_ATTR *net_attr, int *count);
void netcam_net_cfg_save(void);
int netcam_net_autotracking(char *ip);
int netcam_net_exit(void);
int netcam_net_get_detect(char *name);
int netcam_net_reset_net_ip(char *name, ST_SDK_NETWORK_ATTR *newInfo);
int netcam_p2pid_init();
int netcam_net_wifi_init(void);



//motion detect
int netcam_md_init(int n_md);
int netcam_md_destroy();
int netcam_md_set_sensitivity(int vin, SYS_MD_SENSITIVITY_t sensi);
int netcam_md_get_sensitivity(int vin, SYS_MD_SENSITIVITY_t *sensi);
int netcam_md_set_sensitivity_percentage(int vin, int sensitive);
void netcam_md_play_start_one(void);
void netcam_md_play_stop_one(void);
int netcam_md_play_get_cnt(void);
int netcam_md_get_md_light_status(void);
void netcam_md_set_md_light_status(int mdLightStatus);
void netcam_md_enable(int enable);

//ptz
#define NETCAM_PTZ_MAX_PRESET_CRUISE_NUM    32
#define NETCAM_PTZ_MAX_PRESET_NUM           255
#define NETCAM_PTZ_PRESET_NAME_SIZE         64
#define NETCAM_PTZ_PRESET_CFG_FILENAME      "ptz_preset"
#define NETCAM_PTZ_MAX_SPEED                64

typedef struct ptz_preset_info_s {
    int   sizePreset;//预置点个数
    int   presetNum[NETCAM_PTZ_MAX_PRESET_NUM];//预置点序号
    char  presetName[NETCAM_PTZ_MAX_PRESET_NUM][NETCAM_PTZ_PRESET_NAME_SIZE];//预置点名称
} ptz_preset_info_t;


int netcam_ptz_init(void);
int netcam_ptz_exit(void);
int netcam_ptz_start();
int netcam_ptz_up(int step, int speed);
int netcam_ptz_down(int step, int speed);
int netcam_ptz_left(int step, int speed);
int netcam_ptz_right(int step, int speed);
int netcam_ptz_left_up(int step, int speed);
int netcam_ptz_left_down(int step, int speed);
int netcam_ptz_right_up(int step, int speed);
int netcam_ptz_right_down(int step, int speed);
int netcam_ptz_get_presets(ptz_preset_info_t *pstPresetsInfo);
int netcam_ptz_set_preset(int num, char *name);
int netcam_ptz_clr_preset(int num);
int netcam_ptz_preset_cruise(GK_NET_CRUISE_GROUP *cruise_info);
int netcam_ptz_horizontal_cruise(int speed);
int netcam_ptz_vertical_cruise(int speed);
int netcam_ptz_hor_ver_cruise(int speed);
int netcam_ptz_stop(void);
int netcam_ptz_ciruise_stop(void);
int netcam_ptz_is_moving(void);
void netcam_ptz_set_default(int step, int speed);

uint32_t pelco_set_stop();
uint32_t pelco_set_up(unsigned char speed);
uint32_t pelco_set_down(unsigned char speed);
uint32_t pelco_set_left(unsigned char speed);
uint32_t pelco_set_right(unsigned char speed);
uint32_t pelco_set_left_top(unsigned char pan_speed);
uint32_t pelco_set_right_top(unsigned char pan_speed);
uint32_t pelco_set_left_bottom(unsigned char pan_speed);
uint32_t pelco_set_right_bottom(unsigned char pan_speed);
uint32_t pelco_set_zoom_tele();
uint32_t pelco_set_zoom_wide();

#ifdef MODULE_SUPPORT_AF
int netcam_ptz_zoom_stop(void);
int netcam_ptz_zoom_add(void);
int netcam_ptz_zoom_sub(void);
int netcam_ptz_focus_stop(void);
int netcam_ptz_focus_add(void);
int netcam_ptz_focus_sub(void);
#endif

// yudp
int netcam_yudp_start();
int netcam_yudp_stop();

//watchdog
//开门够初始化，时间秒，超过该时间没有喂够，则系统会自动重启
int netcam_watchdog_init(int time_s);
int netcam_watchdog_exit(void);

//设置开门够超时时间，用来做系统复位。
int netcam_watchdog_set_timeout(int time_s);


//SD升级时的默认配置文件
#define NETCAM_SDCARD_UPDATE_INI "/mnt/sd_card/update.ini"

#ifdef __cplusplus
extern "C"
{
#endif

/*
	升级函数//线程操作添加到workqueue中执行
*/
int netcam_update(char *binData, int length,cbFunc updateCb);
void *netcam_update_malloc(int size);
int netcam_update_feed_dog(void);
int netcam_update_init(cbFunc updateCb);
void netcam_update_is_save_md5(int isSet);



#ifdef __cplusplus
}
#endif

/*
	升级进度获取函数
*/
int netcam_update_get_process();
/*
	解析Web升级文件
*/
int netcam_update_mail_style(char *binData, int length,cbFunc updateCb);
/*
	释放资源准备升级
*/
int netcam_update_relase_system_resource(void);
/*
	获取升级状态
*/
int netcam_get_update_status(); // return 0 not updating, other updating.
int netcam_update_set_down_load_cb(cbFunc down_load,void *arg);
int netcam_is_prepare_update(void);

//maintain
int netcam_sys_init_maintain();
int maintain_handle(void* arg);

//get dev name
char *netcam_sys_get_name(void);
//get shell cmd result
int netcam_sys_shell_result(char *cmd, char *buf, int buflen);



//ftp
#define FTP_JPG_FILE "/tmp/ftp.jpg"
int netcam_ftp_upload_get_status(void);
void netcam_ftp_upload_set_status(int flag);
int netcam_ftp_start();
int netcam_ftp_stop();
int netcam_ftp_check(char *ipaddr, int port, char *username, char * passdword);
int netcam_ftp_setting_is_ok();




//mail
#define MAIL_JPG_FILE "/tmp/mail.jpg"
int netcam_mail_send_get_status(void);
void netcam_mail_send_set_status(int flag);
int netcam_mail_test_send(LPGK_NET_EMAIL_PARAM email);
int netcam_mail_start();
int netcam_mail_stop();
int netcam_mail_setting_is_ok();


//int netcam_mail_send(char *jpegfilename);
int get_hw_array(unsigned char *hwaddr, unsigned char *addr);

extern int delete_path(char * path);
extern int copy_file(char *src_name, char *des_name);
extern int delete_cjson_file_frome_dir(char * path);
extern int gk_adjust_bps(int stream_id, int bps);
extern int gk_adjust_fps(int stream_id, int fps);
extern int gk_venc_set_h264_qp_config(ST_VENC_H264QP_CONF *pQpConfig);

int netcam_video_set_stream_name(int streamID, char *name);
int netcam_osd_set_info(int channel, GK_NET_CHANNEL_INFO *channelInfo);

int netcam_video_force_i_frame(int stream_id);
int netcam_net_setwifi(int wifiMode,WIFI_LINK_INFO_t linkInfo);
PIC_SIZE_E netcam_video_get_pic_format(int width, int height);


#ifdef __cplusplus
};
#endif

#endif


