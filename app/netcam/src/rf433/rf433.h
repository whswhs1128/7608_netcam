#ifndef __RF433_H__
#define __RF433_H__

#include<stdio.h>
#include<sys/ioctl.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>
#include<signal.h>
#include<pthread.h>
#include<sys/time.h>

#define _DBG_PRINT_433_
#ifdef _DBG_PRINT_433_
#define RF433_DEBUG(fmt,arg...)  do{printf("[RF433 DEBUG,%s %d :] "fmt" ",__FUNCTION__,__LINE__,##arg);}while(0)
#else
#define RF433_DEBUG(fmt,arg...)
#endif

#define RF433ERR_PRINT(fmt,arg...)  do{printf("[RF433 ERROR,%s %d :] "fmt" ",__FUNCTION__,__LINE__,##arg);}while(0)


#define CMD_SEND		0x01
#define CMD_RECV		0x03
#define RF433NUMBER_MAX		32
#define RF433NAMESIZE_MAX	64
#define RF433DATAFILE		"/opt/custom/cfg/Rf433Data.bin"//"/opt/custom/cfg/rf433.data"

#define RF433_TRUE			1
#define RF433_FALSE			0
#define RF433LEARNTIMEOUT	30

#define RF433_KEY_DEFENCE	0x08
#define RF433_KEY_UNDEFENCE	0x04
#define RF433_KEY_BELL		0x02
#define RF433_KEY_SOS		0X01

typedef struct{
	unsigned char *pbBuffer;
	unsigned char *pbBufferLen;
	unsigned int *pRet;
}strSbiData;


typedef struct{
	int status;
	int exit_proc;
	int codetype;//瀛︾爜鐨勬椂鍊欙紝绫诲瀷
	int fd;//  /dev/goke_sbi  file decription
	char CodeDataFile[RF433NAMESIZE_MAX];
	char name[RF433NAMESIZE_MAX];
	pthread_t phreadid;
	pthread_mutex_t mutex;
	void (*learn_handle)(int flag);
	void (*event_handle)(int areaid,int type,int code);
} strRf433PthreadCtl;

enum
{
	RF433STATUS_IDLE,
	RF433STATUS_LEARN,
	RF433STATUS_MAX,
};

typedef enum{
	RF433_DEV_TYPE_CONTROL = 10,	// 遥控
	RF433_DEV_TYPE_ALARM,					// 报警
	RF433_DEV_TYPE_OTHER,
}RF433_DEV_TYPE;

typedef enum{
	AVIOCTRL_CFG_433_OK = 0,	// 配对成功
	AVIOCTRL_CFG_433_TIMEOUT,	// 配对超时
	AVIOCTRL_CFG_433_MAX,		// 设备上限
	AVIOCTRL_CFG_433_WAITING,			// 正在学习
	AVIOCTRL_CFG_433_EXISTS,			// 设备已存在
	AVIOCTRL_CFG_433_ERROR,
}ENUM_AVIOTC_CFG_433_RET;


typedef struct{
	int areaid;
	int type;
	int code;
	char name[RF433NAMESIZE_MAX];
} strRf433Data;

typedef struct{
	unsigned char useflag[RF433NUMBER_MAX];
	strRf433Data Data[RF433NUMBER_MAX];
} strRf433Dev;

int rf433_Init(char *codeDataPath);
void rf433_Exit(void);
void rf433_learn_code(int codetype,char *name);
int rf433_modify_code_name(int codeid,char *name);
int rf433_get_dev_num(void);
void rf433_get_dev_corresponding_info(int seq,int *id,int *type,char *name);
void rf433_delete_code(int codeid);
int rf433_get_dev_type(int id);
void rf433_exit_learn(void);


#endif

