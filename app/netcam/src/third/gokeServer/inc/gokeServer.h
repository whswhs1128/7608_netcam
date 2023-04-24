#ifndef GOKE_MOJING_H
#define GOKE_MOJING_H

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
    GOKE_DEV_INFO = 1,
    GOKE_GET_INFO,
    GOKE_HEART,
    GOKE_REBOOT,
    GOKE_GET_NETUART,
    GOKE_SET_NETUART,
    GOKE_GET_MD_INFO,
    GOKE_SET_MD_INFO,
    GOKE_GET_AUDIO,
    GOKE_SET_AUDIO,
    GOKE_GET_RECORD,
    GOKE_SET_RECORD,
    GOKE_GET_SD,
    GOKE_SD_FORMAT,
    GOKE_UPGRADE,
    GOKE_SNAP,
    GOKE_SET_ID,
    GOKE_RUN_CMD,
    GOKE_VIDEO_FLIP,
    GOKE_UNKNOWN
}GOKE_CMD_TYPE;

typedef struct
{
    unsigned char enable;
    unsigned char sensitive;
}GOKE_SERVER_MD;

typedef struct
{
    unsigned char inputVolume;    // 100
    unsigned char outputVolume;   // 100   
    unsigned char audioInEnable;   //1=关闭，2=开启
    unsigned char audioOutEnable;
    unsigned char rebootMute;      // 0 enable audio when boot; 1 disable audio
}GOKE_SERVER_AUDIO;

typedef struct
{
    unsigned char enable;             /*是否录像 0-否 1-是*/
    unsigned char stream_no;            /* 选择哪个通道开始录像 0-3 */
    unsigned char recordMode;         //  0:按预设时间录像 1:预设全天录像 2:手动录像模式 3:停止录像
    unsigned char preRecordTime;      /* 预录时间，单位是s，0表示不预录。 */
    unsigned char audioRecEnable;     /*录像时复合流编码时是否记录音频数据*/  
    unsigned char recAudioType;     /*录制的音频格式 0 a-law; 1 u-law; 2 pcm; 3-adpcm*/ 
    unsigned short recordLen;          //录像文件打包时长,以分钟为单位
}GOKE_SERVER_RECORD;

typedef struct
{
    int status;          
    int free;            //MB
    int all;         
}GOKE_SERVER_SD;

typedef struct
{
    int forceUpgrade; //不比较版本高低，否则只会从低版本到高版本
    char ver[32];          
    char model[32];
    char url[196];         
}GOKE_SERVER_UPGRADE;

typedef struct
{
    char path[32];          
}GOKE_SERVER_SNAP;

typedef struct
{
    int type;
    char id[32];          
}GOKE_SERVER_SET_ID;

typedef struct
{
    char flip;
    char mirror;          
}GOKE_SERVER_SET_FLIP;


typedef struct
{
    unsigned short headMagic; //0x3323
    unsigned short cmdType;
    int cmdLength;
}GOKE_SERVER_HEAD;


typedef struct
{
    char devId[32];
    char devVer[32];
    char devModel[32];
}GOKE_SERVER_DEV_INFO;

typedef struct
{
    char cmd[256];
}GOKE_SERVER_RUN_CMD;


int goke_server_start(void);


#ifdef __cplusplus
    }
#endif

#endif

