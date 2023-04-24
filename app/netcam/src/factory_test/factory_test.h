#ifndef _FACTORY_TEST_H_
#define _FACTORY_TEST_H_

#define FACTORY_MAX_WIFI_SSID_LENGTH			128
#define FACTORY_MAX_WIFI_PASSWD_LENGTH			128

#define FACTORY_TEST_TF_PATH					"/mnt/sd_card/factorytest"
#define FACTORY_TEST_PATH						"/tmp/factorytest"//"/mnt/sd_card/factorytest"//"/home/share/gk_cms_v2/applications/netcam/src/factory_test"//"/mnt/sd_card/factorytest"
#define FACTORY_TEST_CFG						"factory_test_cfg.cjson"

#define FACTORY_AUDIO_NAME_LENGTH				128
#define FACTORY_TEST_AUDIO_TEST					"test_mode.alaw"
#define FACTORY_TEST_AUDIO_BURNIN				"burn_in_mode.alaw"
#define FACTORY_TEST_AUDIO_RECORD				"record.alaw"
#define FACTORY_TEST_AUDIO_STOPRECORD			"stop_record.alaw"
#define FACTORY_TEST_AUDIO_START_WIFI_CONNECT	"start_wifi_connect.alaw"
#define FACTORY_TEST_AUDIO_STOP_WIFI_CONNECT	"stop_wifi_connect.alaw"


#define FACTORY_WORKMODE_TEST_RECORD_FILE		"test.pcm"

typedef enum
{
	FACTORY_WORKMODE_NORMAL = 0,
	FACTORY_WORKMODE_TEST,
	FACTORY_WORKMODE_BURNIN,
	FACTORY_WORKMODE_MAX = 0XFF,
}FACTORY_WORK_MODE_E;

typedef struct FACTORY_WIFI_STR
{
	int flag;//0 unuse wifi ;1 use wifi
	char ssid[FACTORY_MAX_WIFI_SSID_LENGTH];//test wifi ssid
	char passwd[FACTORY_MAX_WIFI_PASSWD_LENGTH];//test wifi password
}FACTORY_WIFI_S;

typedef struct FACTORYTESTMODE_STR
{
	int flag;//0 stop test; 1 start test
	int record_time;//test mode record time
	int record_fd;//record file description
	void *mhandle;
	FACTORY_WIFI_S wifi;
}FACTORYTESTMODE_S;

typedef struct FACTORYBURNINMODE_STR
{
	int flag;//0 stop burn-in mode ; 1 start burn-in mode
	int link_wifi_time;//link wifi time
	int unlink_wifi_time;//stop wifi time
	int ptz_loop;//0 stop ptz loop; 1 start loop
	FACTORY_WIFI_S wifi;
}FACTORYBURNINMODE_S;

typedef union FACTORY_WORKMODE_STR
{
	FACTORYTESTMODE_S test_mode;
	FACTORYBURNINMODE_S buinin_mode;
}FACTORY_WORKMODE_S;

typedef struct FACTORY_AUDIO_STR
{
	char audio_test[FACTORY_AUDIO_NAME_LENGTH];
	char audio_burnin[FACTORY_AUDIO_NAME_LENGTH];
	char audio_record[FACTORY_AUDIO_NAME_LENGTH];
}FACTORY_AUDIO_S;

typedef struct FACTORY_TEST_PTHREAD_CTL_STR
{
	int flag;//0 pthread stop;1 start
	pthread_t phreadid;
	pthread_mutex_t mutex;
}FACTORY_TEST_PTHREAD_CTL_S;


typedef struct FACTORY_TEST_STR
{
	int mode;//mode
	FACTORY_TEST_PTHREAD_CTL_S pthread_ctl;
	FACTORY_AUDIO_S audio_file;
	FACTORY_WORKMODE_S work_mode;
}FACTORY_TEST_S;



void vFactoryWorkMode_Init(void);
void vFactoryWorkMode_Exit(void);
#endif

