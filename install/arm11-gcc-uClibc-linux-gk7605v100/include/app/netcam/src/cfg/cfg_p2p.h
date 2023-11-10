#ifndef CFG_P2P_H
#define CFG_P2P_H

#define CFG_P2P_MAGIC   0XA93B472E

#define P2P_RECORD_HISTORY_CNT   10

#define DOORBELL_DISABLE_WAKEUP  0X79

typedef struct {
    int startSec;
    int stopSec;
} SCHEDTIME_EXT;

typedef struct
{
	int		  		magic;
	int		  		language;
	char      		timeZoneString[64];//
    int       		msgPush[2];  // 2 sensor, push msg, 0:not push msg,1:push msg.
	int      		audioInEnable;//
	int      		audioOutEnable;//
	unsigned char   PushKeyUpdateFlg;//
	char      		user_sign[32];
    char      		scheduleTimeEnable[7];  // 星期一到星期天是否录像， 0-否 1-是 ,   support goolink p2p
    SCHEDTIME_EXT	scheduleTimeSec[7][4]; //扩展 GK_SCHEDTIME，增加秒级单位，support goolink p2p
    char            is_beep[4];   			// 是否蜂鸣器鸣叫, support goolink p2p
    char            alarmAudioType; 		// 报警声音类型, support goolink p2p
	int		  		endMagic;
}GK_P2P_CFG;


extern GK_P2P_CFG runP2PCfg;
#define P2P_CFG_FILE "gk_p2p.cjson"

void  P2PCfg_init(void);
int P2PCfgSave(void);
int P2PCfgLoad(void);
int P2PCfgLoadDefValue(void);

#endif

