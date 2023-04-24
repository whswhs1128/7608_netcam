#ifndef _ITE_GB28181_PLAYBACK_H_
#define _ITE_GB28181_PLAYBACK_H_

#include <pthread.h>

//#define USE_RTP_CHANNEL_IN_PLAYBACK_THREAD

typedef struct {

	unsigned long long start_time;  	// 20190722115022
	unsigned long long end_time;
	unsigned long long start_timetick;  // 1563767423, seconds
	unsigned long long end_timetick;
}TIME_RECORD_T;

void gk_gb28181_u64t_to_time(unsigned long long time64,char *timestr);
unsigned long long gk_gb28181_time_to_u64t(char *timestr);
unsigned long long gk_gb28181_timestamp_to_u64t(char *timestampstr);
int  gk_gb28181_playback_get_record_time(char *period_start, char *period_end, TIME_RECORD_T **timeInfo);
int  gk_gb28181_playback_init(void);
int  gk_gb28181_playback_session_open(int session_id);
void gk_gb28181_playback_set_playback_cmd(int session_id,TIME_RECORD_T *rangeInfo,char *local_ip, char *media_server_ip,
                    char *media_server_port, unsigned int enOverTcp, int ssrc, int eventId);
void gk_gb28181_playback_set_download_cmd(int session_id,TIME_RECORD_T *rangeInfo,int speed, char *local_ip, char *media_server_ip,
                    char *media_server_port, unsigned int enOverTcp, int ssrc, int eventId);

void gk_gb28181_playback_set_play_cmd(int session_id,float speed,int isNeedSeek,TIME_RECORD_T *rangeInfo);
void gk_gb28181_playback_set_pause_cmd(int session_id,int pauseTime);
void gk_gb28181_playback_set_stop_cmd(int session_id);

#endif
