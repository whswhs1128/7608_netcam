#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "media_fifo.h"
#include "avi_search_api.h"
#include "avi_utility.h"
#include "ite_gb28181_playback.h"
#include "ite_sip_api.h"
#include "sdk_sys.h"
#include "ite_sip_api.h"
#include "netcam_api.h"

#define MAX_SESSION_CNT		3
#define USE_RECORD_TIMETICK	0

typedef struct
{
	char isValid;
	int  downloadSpeed;
	TIME_RECORD_T rangeInfo;
}PLAYBACK_DOWNLOAD_CMD_T;

typedef struct
{
	char   isValid;
	char   isNeedSeek;
	float  speed;
	TIME_RECORD_T rangeInfo;
}PLAY_CMD_T;

typedef struct
{
	char isValid;
	int  pauseTime;
}PAUSE_CMD_T;

typedef struct
{
	AviPBHandle* 			FileHandle;
	PLAYBACK_DOWNLOAD_CMD_T playbackCmd;
	PLAYBACK_DOWNLOAD_CMD_T downloadCmd;
	PLAY_CMD_T				playCmd;
	PAUSE_CMD_T				pauseCmd;
	char					stopCmd;
	int						sessionID;
	pthread_mutex_t 		playbackLock;
    char 					localip[64];
    char 					remoteip[64];
    unsigned short 			port;
    unsigned int 			enOverTcp;
    int 					ssrc;
    int 					eventId;

}PLAYBACK_CTRL_T;

static PLAYBACK_CTRL_T playbackCtrlInfo[MAX_SESSION_CNT];
static unsigned int ptsStart = 0;

extern int gk_rtp_playback_send(void *data, int size, GK_NET_FRAME_HEADER *header,int session_id,unsigned short *u16CSeq);
extern int gk_rtp_playback_open_rtp_channel(int session_id,char *local_ip, char *media_server_ip,
                    int port, unsigned int enOverTcp, int ssrc, int eventId);
extern int gk_rtp_playback_close_rtp_channel(int session_id);

extern ite_gb28181Obj sip_gb28281Obj;

static void gb28181_substring(char *dest, char *src, int start, int end)
{
    int i=start;
    if(start>strlen(src))
        return;
    if(end>strlen(src))
        end=strlen(src);
    while(i<end)
    {
        dest[i-start]=src[i];
        i++;
    }
    dest[i-start]='\0';
    return;
}

unsigned long long gk_gb28181_timestamp_to_u64t(char *timestampstr)
{
    struct tm *ptm;
	struct tm t = {0};
	unsigned long long tmp64 = 0;
    char time[30];

	if(timestampstr == NULL)
		return 0;

    memset(time, 0, sizeof(time));

	tmp64 = (unsigned long long)atoll(timestampstr);
	ptm = localtime_r(&tmp64, &t);

    sprintf(time, "%04d%02d%02d%02d%02d%02d",
            ptm->tm_year+1900,
            ptm->tm_mon+1,
            ptm->tm_mday,
            ptm->tm_hour,
            ptm->tm_min,
            ptm->tm_sec);

	tmp64 = atoll(time);
	//printf("time:%llu\n",tmp64);

    return tmp64;
}
unsigned long long gk_gb28181_time_to_u64t(char *timestr)
{
	char tmp[32];
	int year=0,month=0,day=0,hour=0,min=0,sec=0;
	unsigned long long tmp64 = 0;

	sscanf(timestr,"%d-%d-%dT%d:%d:%d",&year,&month,&day,&hour,&min,&sec);

	memset(tmp, 0, sizeof(tmp));
	sprintf(tmp, "%04lu%02lu%02lu%02lu%02lu%02lu", (unsigned long)year, (unsigned long)month,\
		(unsigned long)day,(unsigned long)hour,(unsigned long)min, (unsigned long)sec);
	tmp64 = (unsigned long long)atoll(tmp);

	//printf("gbt28181_time_to_u64t:%llu\n",tmp64);

	return tmp64;

}

void gk_gb28181_u64t_to_time(unsigned long long time64,char *timestr)
{

	char tmp_str[20];
    char tmp[20];
	int year=0,month=0,day=0,hour=0,min=0,sec=0;

    memset(tmp_str, 0, sizeof(tmp_str));
    sprintf(tmp_str, "%lld", time64);

    memset(tmp, 0, sizeof(tmp));
    gb28181_substring(tmp, tmp_str, 0, 0+4);
    year = atoi(tmp);

    memset(tmp, 0, sizeof(tmp));
    gb28181_substring(tmp, tmp_str, 4, 4+2);
    month = atoi(tmp);

    memset(tmp, 0, sizeof(tmp));
    gb28181_substring(tmp, tmp_str, 6, 6+2);
    day = atoi(tmp);

    memset(tmp, 0, sizeof(tmp));
    gb28181_substring(tmp, tmp_str, 8, 8+2);
    hour = atoi(tmp);

    memset(tmp, 0, sizeof(tmp));
    gb28181_substring(tmp, tmp_str, 10, 10+2);
    min = atoi(tmp);

    memset(tmp, 0, sizeof(tmp));
    gb28181_substring(tmp, tmp_str, 12, 12+2);
    sec = atoi(tmp);


	sprintf(timestr, "%04lu-%02lu-%02luT%02lu:%02lu:%02lu", (unsigned long)year, (unsigned long)month, (unsigned long)day,\
		(unsigned long)hour, (unsigned long)min, (unsigned long)sec);
	//printf("gbt28181_u64t_to_time:%llu-->%s\n",time64,timestr);

}

int  gk_gb28181_playback_get_record_time(char *period_start, char *period_end, TIME_RECORD_T **timeInfo)
{    
    #define MAX_FILE_CNT        60*24
	u64t start;
	u64t stop;
	int i=0,sched_file_cnt=0,lst_len=0, min_len;
    FILE_NODE node;
	FILE_LIST *list = NULL;
    TIME_RECORD_T *timerecInfo;

	start = gk_gb28181_time_to_u64t(period_start);
	stop = gk_gb28181_time_to_u64t(period_end);

	printf("period_start:%llu,period_end:%llu\n", start, stop);
	list = search_file_by_time(0, 0xFF, start, stop);
	if (!list)
	{
		printf("search file none!\n");
		return -1;
	}

	lst_len = get_len_list(list);
	if (lst_len == 0)
	{
		printf("no file found!\n");
		search_close(list );
		return -1;
	}
    min_len = lst_len>MAX_FILE_CNT?MAX_FILE_CNT:lst_len;
    
    timerecInfo = malloc(sizeof(TIME_RECORD_T)*min_len);
    if(timerecInfo == NULL)
    {
        printf("%s,LINE:%d,malloc failed\n",__FUNCTION__,__LINE__);
        search_close(list );
		return -1;
    }
    
    memset(timerecInfo,0,sizeof(TIME_RECORD_T)*min_len);
    printf("get_len_list:%d, min_len:%d\n", lst_len, min_len);

	for(i=0; i<lst_len; i++)
	{
		memset(&node, 0, sizeof(FILE_NODE));
		if (get_file_node(list, &node) != DMS_NET_FILE_SUCCESS)
		{
			continue;
		}

		if(node.start >= node.stop)
		{
			printf("---->file name may wrong,start-->stop [%llu:%llu]\n",node.start,node.stop);
			continue;
		}

		timerecInfo[sched_file_cnt].start_time  = node.start;
		timerecInfo[sched_file_cnt].end_time 	 = node.stop;
		sched_file_cnt++;
        if(sched_file_cnt >= min_len)
        {
            printf("sched_file_cnt:%d, min_len:%d\n", sched_file_cnt, min_len);
            break;
        }
	}

	search_close(list );
    *timeInfo = timerecInfo;

	return sched_file_cnt;

}

#if 1
static unsigned int vpts=0,apts=0;
// type =1 返回系统运行秒，type=2 返回系统运行ms
static unsigned long get_sys_timestamp(int type)
{
    struct timespec times = {0, 0};
    unsigned long time;

    clock_gettime(CLOCK_MONOTONIC, &times);
    //LOG_INFO("CLOCK_MONOTONIC: %lu, %lu\n", times.tv_sec, times.tv_nsec);

    if (1 == type)
    {
        time = times.tv_sec;
    }
    else
    {
        time = times.tv_sec * 1000 + times.tv_nsec / 1000000;
    }


    return time;
}
static void cal_video_frame_pts(unsigned int *pts, unsigned int time_tick)
{
    #if 0
    static unsigned long tv1 = 0, tv2 = 0;
    unsigned int time_ms = 0;

    if(0 == tv1)
        tv1 = get_sys_timestamp(2);
    tv2 = get_sys_timestamp(2);

    time_ms = tv2 - tv1;

    vpts += 90 * time_ms;
    *pts =vpts;
    tv1 = tv2;
    #else
    if (ptsStart == 0)
    {
        ptsStart = time_tick;
    }
    *pts = (time_tick - ptsStart) * 90;
    #endif

    return;
}


static void cal_audio_frame_pts(unsigned int *pts, unsigned int time_tick)
{
    #if 0
    static unsigned long tv1 = 0, tv2 = 0;
    unsigned int time_ms = 0;

    if(0 == tv1)
        tv1 = get_sys_timestamp(2);
    tv2 = get_sys_timestamp(2);
    time_ms = tv2 - tv1;
    apts += 8 * time_ms;
    *pts = apts;
    tv1 = tv2;
    #else
    if (ptsStart == 0)
    {
        ptsStart = time_tick;
    }
    *pts = (time_tick - ptsStart) * 90;
    #endif

    return;
}

#endif

static u64t gb28181_timetick_to_time(AVI_DMS_TIME *ptime, unsigned int timetick)
{
    struct tm *p;
    long timep = timetick;

    struct tm t1 = {0};
    p = localtime_r(&timep, &t1);

    ptime->dwYear = p->tm_year + 1900;
    ptime->dwMonth = p->tm_mon + 1;
    ptime->dwDay = p->tm_mday;
    ptime->dwHour = p->tm_hour;
    ptime->dwMinute = p->tm_min;
    ptime->dwSecond = p->tm_sec;

	return time_to_u64t(ptime);
}

static u64t gb28181_cal_timetick_sub(AVI_DMS_TIME *pStartTime, AVI_DMS_TIME *pEndTime)
{

	time_t time1,time2;
	struct tm *p1,*p2;

	time(&time1);

	struct tm t1 = {0},t2 = {0};
	p1 = localtime_r(&time1, &t1);

	p1->tm_year = pStartTime->dwYear - 1900;
	p1->tm_mon = pStartTime->dwMonth - 1;
	p1->tm_mday = pStartTime->dwDay;
	p1->tm_hour = pStartTime->dwHour;
	p1->tm_min = pStartTime->dwMinute;
	p1->tm_sec = pStartTime->dwSecond;

	time1 = mktime(p1);

	p2 = localtime_r(&time2, &t2);

	p2->tm_year = pEndTime->dwYear - 1900;
	p2->tm_mon = pEndTime->dwMonth - 1;
	p2->tm_mday = pEndTime->dwDay;
	p2->tm_hour = pEndTime->dwHour;
	p2->tm_min = pEndTime->dwMinute;
	p2->tm_sec = pEndTime->dwSecond;

	time2 = mktime(p2);

	printf("gb28181_cal_timetick_sub:%d,%d,%d\n",time1,time2,(time2-time1));

	return (time2-time1);
}

static AviPBHandle *gb28181_playback_by_time(u64t start, u64t stop, char is_download, char try_cnt,int *offset_seconds)
{
    u32t ch_num = 0;
    int ret = 0,seekPos = 0;
	AVI_DMS_TIME StartTime,StopTime;
    AviPBHandle *pPBHandle = NULL;
    FILE_NODE node;
	int i=0,lst_len=0;

    pPBHandle = (AviPBHandle *)create_pb_handle();
    if (NULL == pPBHandle)
	{
        return NULL;
    }

	u64t_to_time(&StartTime, start);

    pPBHandle->list = search_file_by_time(0, ch_num, start, stop);  //0， 所有的情况
    if (NULL == pPBHandle->list)
	{
        printf("%s,LINE:%d,list is null.\n",__FUNCTION__,__LINE__);
        goto end;
    }

	lst_len = get_len_list(pPBHandle->list);

	printf("1. playback lst_len = %d!\n",lst_len);
	if(lst_len == 0)
        goto end;
		

    //按起始时间定位 pPBHandle 的no
    seekPos = cal_pb_seek_time(pPBHandle, &StartTime);
    if(seekPos < 0)
    {
        printf("%s,LINE:%d,cal_pb_seek_time error!\n",__FUNCTION__,__LINE__);

		if(!is_download && try_cnt == 2)
		{
			if(pPBHandle->list)
			{
				search_close(pPBHandle->list);
				pPBHandle->list = NULL;
			}

			pPBHandle->list = search_file_by_time(0, ch_num, start, stop);  //0， 所有的情况
			if (NULL == pPBHandle->list)
			{
				printf("%s,LINE:%d,list is null.\n",__FUNCTION__,__LINE__);
				goto end;
			}

			lst_len = get_len_list(pPBHandle->list);
			if(lst_len == 0)
			{
				printf("no file found!\n");
				goto end;
			}
			else
				printf("2. playback lst_len = %d!\n",lst_len);

			for(i=0; i<lst_len; i++)
			{
				memset(&node, 0, sizeof(FILE_NODE));
				if (get_file_node(pPBHandle->list, &node) != DMS_NET_FILE_SUCCESS)
					continue;

				strcpy(pPBHandle->file_path, node.path);
				seekPos = 0;
				break;
			}

			if(i == lst_len)
			{
				printf("playback no found file!\n");
				goto end;
			}
		}
		else
        	goto end;
    }

    ret = avi_pb_open(pPBHandle->file_path, pPBHandle);
    if (ret < 0)
	{
        printf("%s,LINE:%d,avi_pb_open %s error!\n", __FUNCTION__,__LINE__,pPBHandle->file_path);
        goto end;
    }
	
    ret = avi_pb_set_pos(pPBHandle, &seekPos);
    if (ret < 0)
    {
        printf("%s,LINE:%d,avi_pb_set_pos %s error!\n", __FUNCTION__,__LINE__,pPBHandle->file_path);
        goto end;
    }

    print_pb_handle(pPBHandle);

	printf("pPBHandle->gop = %d!\n", pPBHandle->gop);


	if(offset_seconds)
		*offset_seconds = seekPos;

    return pPBHandle;

end:

    if(pPBHandle->list)
    {
        search_close(pPBHandle->list);
        pPBHandle->list = NULL;
    }
    if(pPBHandle->file)
    {
		fclose(pPBHandle->file);
        pPBHandle->file = NULL;
    }
    if(pPBHandle->idx_array)
    {
        free(pPBHandle->idx_array);
        pPBHandle->idx_array = NULL;
    }
    if(pPBHandle->pb_buf)
    {
        free(pPBHandle->pb_buf);
        pPBHandle->pb_buf = NULL;
		pPBHandle->pb_buf_size = 0;
    }
    if(pPBHandle)
    {
        free(pPBHandle);
        pPBHandle = NULL;
    }

	if(offset_seconds)
		*offset_seconds = seekPos;

	return NULL;
}


static u64t gb28181_playback_add_offset(unsigned long long time_start_64, int offset_seconds)
{
	time_t time1;
	struct tm *p1;
	AVI_DMS_TIME srcTime,dstTime;

	u64t_to_time(&srcTime, time_start_64);

	struct tm t1 = {0};
	p1 = localtime_r(&time1, &t1);

	p1->tm_year = srcTime.dwYear - 1900;
	p1->tm_mon = srcTime.dwMonth - 1;
	p1->tm_mday = srcTime.dwDay;
	p1->tm_hour = srcTime.dwHour;
	p1->tm_min = srcTime.dwMinute;
	p1->tm_sec = srcTime.dwSecond;

	time1 = mktime(p1);
	time1 += offset_seconds;

	struct tm tt = {0};
	struct tm *t = localtime_r(&time1, &tt);
	dstTime.dwYear = t->tm_year + 1900;
	dstTime.dwMonth = t->tm_mon + 1;
	dstTime.dwDay = t->tm_mday;
	dstTime.dwHour = t->tm_hour;
	dstTime.dwMinute = t->tm_min;
	dstTime.dwSecond = t->tm_sec;

	return time_to_u64t(&dstTime);
}

static int gb28181_playback_seek_by_offset(AviPBHandle *pPBHandle,char *pb_file_name,unsigned long long offset)
{
    char file_name[128];
	int i = 0, seekPos = 0,ret = 0;
	u64t time_start_64 = 0;
	AVI_DMS_TIME seekTime;
	u64t time_real_64 = 0;

	if(!pPBHandle || !pb_file_name)
		return -1;

	#if 0
    memset(file_name, 0 ,sizeof(file_name));
    i = last_index_at(pPBHandle->file_path, '/');
    strcpy(file_name, pPBHandle->file_path + i + 1);

	time_start_64 = avi_get_start_time(file_name);
	#endif

	time_start_64 = avi_get_start_time(pb_file_name);

	time_real_64 = gb28181_playback_add_offset(time_start_64,offset);

	printf("pb_file_name:%s,time_start_64 = %lld,offset = %d,time_real_64 = %lld!\n",pb_file_name,time_start_64,offset,time_real_64);

	u64t_to_time(&seekTime, time_real_64);

    seekPos = cal_pb_seek_time(pPBHandle, &seekTime);
    if(seekPos < 0)
    {
        printf("%s,LINE:%d,seek position error!\n",__FUNCTION__,__LINE__);
        return -1;
    }

    ret = avi_pb_open(pPBHandle->file_path, pPBHandle);
    if (ret < 0)
	{
        printf("%s,LINE:%d,avi_pb_open %s error!\n", __FUNCTION__,__LINE__,pPBHandle->file_path);
        return -1;
    }

    ret = avi_pb_set_pos(pPBHandle, &seekPos);
	if (ret < 0)
	{
		printf("%s,LINE:%d,avi_pb_set_pos %s error!\n", __FUNCTION__,__LINE__,pPBHandle->file_path);
		return -1;
	}	
	printf("pPBHandle->gop = %d!\n", pPBHandle->gop);

    return 0;
}

static int gb28181_playback_get_frame(AviPBHandle *pPBHandle, char *p_buff, int *p_size, char is_download,float speed)
{
    int ret;
    int sleep_um = 0;

    //正在升级，退出录像
    if(netcam_get_update_status() != 0)
    {
        printf("call gb28181_playback_get_frame, exit for updating.");
        return  -1;
    }
    
	if(!pPBHandle)
	{
        printf("pPBHandle is NULL\n");
		return -1;
	}

	if(pPBHandle->no >= pPBHandle->index_count)
	{
		return -1;
	}

	if(0)//(is_download)
	{
		ret = avi_get_frame_loop(pPBHandle, p_buff, p_size);
		if(ret < 0)
		{
			printf("call avi_get_frame_loop error 1.");
			return ret;
		}
		
		if (pPBHandle->node.frame_type == 0x11 || pPBHandle->node.frame_type == 0x10)
        {
        	if(pPBHandle->fps != 10)
				usleep(8000);
			else
				usleep(10000);
		}	
	}
	else
	{
		if(speed != 0)
		{
			ret = avi_get_frame_loop(pPBHandle, p_buff, p_size);
			if(ret < 0)
			{
				printf("call avi_get_frame_loop error.");
				return ret;
			}
			if (speed > 0.29 && speed < 0.31)
				speed = 0.25;
			else if (speed > 0.09 && speed < 0.11)
				speed = 0.125;
			
            if (pPBHandle->node.frame_type == 0x11 || pPBHandle->node.frame_type == 0x10)
            {
                int useTime = 0;
                struct timeval tt1, tt2;
                int t1 = 0, t2 = 0;
                int time32;
                int diffTime = 0;
                int diffTimeus = 0;
                static int extraSleep = 0;
                int divAll = 1000 % ((int)(pPBHandle->fps*speed));
                int maxSleepTime = 1000/(pPBHandle->fps*speed);
                int oneExtralSleep = (divAll*1000)/(pPBHandle->fps*speed);

                gettimeofday(&tt2, NULL);
                if (divAll != 0)
                {
                    if (extraSleep >= 10000)
                    {
                        usleep(extraSleep);
                        extraSleep = 0;
                    }
                    extraSleep += oneExtralSleep;
                }
                time32 = (tt2.tv_sec % 1000000) * 1000 + (tt2.tv_usec / 1000);
                if (pPBHandle->last_tick != 0)
                {
                    diffTime = time32 - pPBHandle->last_tick;
                    diffTimeus = diffTime * 1000;
                    //printf("diffTime:%d,%d,%d, %d\n", diffTime, maxSleepTime, time32, pPBHandle->last_tick);
                    if (diffTime > 0)
                    {
                        if (diffTime <= maxSleepTime)
                        {
                            //printf("--->sleep1:%d\n", 1000 * (maxSleepTime - diffTime));
                            if (maxSleepTime != diffTime)
                            {
                                usleep(1000 * (maxSleepTime - diffTime));
                            }
                            pPBHandle->last_tick = time32 + maxSleepTime - diffTime;
                        }
                        else if (diffTime > maxSleepTime)
                        {
                            pPBHandle->last_tick = time32 - (diffTime - maxSleepTime);
                        }
                    }
                    else
                    {
                        pPBHandle->last_tick = time32;
                    }
                }
                else
                {
                    extraSleep = 0;
                    pPBHandle->last_tick = time32;
                }
            }
		}
	}

	return 0;
}

//int sendCnt = 0;
static void *gb28181_playback_thread(void *arg)
{
	PLAYBACK_DOWNLOAD_CMD_T pbDownLoadCmd;
	PLAY_CMD_T				playCmd;
	int 					ret,frame_size;
	unsigned int 			pts = 0;
	char 					pauseCmd = 1;
    struct timeval 			tval;
	PLAYBACK_CTRL_T			*pbCtrlInfo = (PLAYBACK_CTRL_T *)arg;
	char					threadName[40] = {0};
	GK_NET_FRAME_HEADER 	header = {0};
	int 					video_frame_no=0,audio_frame_no=0;
	struct timeval 			tv = {0};
	char 					try_cnt = 0,download_flag = 0;
	int						wait_cnt = 0;
	unsigned long long 		download_end = 0,frame_time = 0,total_frame = 0;
	AVI_DMS_TIME 			src,dst,time;
	unsigned long long 		end_timetick = 0;
	int 					filename_time_tick = 0;
	float					playback_speed = 1.0;
	int 					playback_start_offset = 0;
    char 					pb_file_name[128];
	int    		            last_video_no = 0;
	char 					isSendIframe = 0;
    int                     streamType = 0x1b;
    #if 0
    struct timeval tt1, tt2;
    static int sendCnt = 0;
    int t1;
    #endif
	unsigned short 			u16CSeq = 1;

    pthread_detach(pthread_self());

	memset(&pbDownLoadCmd,0,sizeof(PLAYBACK_DOWNLOAD_CMD_T));

	sprintf(threadName,"gb28181_playback_thread_%d",pbCtrlInfo->sessionID);

    sdk_sys_thread_set_name(threadName);

	while(pbCtrlInfo->stopCmd == 0)
	{	
		if(netcam_is_prepare_update() || (netcam_get_update_status() != 0))
		{
            printf("upgrade exit gb28181_playback_thread_%d\n", pbCtrlInfo->sessionID);
			break;
		}
		memset(&pbDownLoadCmd,0,sizeof(PLAYBACK_DOWNLOAD_CMD_T));

		pthread_mutex_lock(&pbCtrlInfo->playbackLock);

		if(pbCtrlInfo->playbackCmd.isValid)
		{
			memcpy(&pbDownLoadCmd,&pbCtrlInfo->playbackCmd,sizeof(PLAYBACK_DOWNLOAD_CMD_T));
			pbCtrlInfo->playbackCmd.isValid = 0;
			pauseCmd = 0;
			download_flag = 0;
            ptsStart = 0;
			isSendIframe = 1;
			
			printf("session:%d,playbackCmd! from %lld to %lld!\n",pbCtrlInfo->sessionID,
			pbCtrlInfo->playbackCmd.rangeInfo.start_time,pbCtrlInfo->playbackCmd.rangeInfo.end_time);
			
			if(gk_rtp_playback_open_rtp_channel(pbCtrlInfo->sessionID,pbCtrlInfo->localip,pbCtrlInfo->remoteip,
				pbCtrlInfo->port,pbCtrlInfo->enOverTcp,pbCtrlInfo->ssrc,pbCtrlInfo->eventId) < 0)
			{
				pbDownLoadCmd.isValid = 0;
				pauseCmd = 1;
				printf("session:%d,gk_rtp_playback_open_rtp_channel failed!\n",pbCtrlInfo->sessionID);
			}
		}
		else if(pbCtrlInfo->downloadCmd.isValid)
		{
			memcpy(&pbDownLoadCmd,&pbCtrlInfo->downloadCmd,sizeof(PLAYBACK_DOWNLOAD_CMD_T));
			pbCtrlInfo->downloadCmd.isValid = 0;
			download_flag = 1;
			pauseCmd = 0;
            ptsStart = 0;
			memset(&playCmd,0,sizeof(PLAY_CMD_T));
			isSendIframe = 1;
			
			printf("session:%d,downloadCmd! from %lld to %lld!\n\n",pbCtrlInfo->sessionID,
			pbCtrlInfo->downloadCmd.rangeInfo.start_time,pbCtrlInfo->downloadCmd.rangeInfo.end_time);

			if(gk_rtp_playback_open_rtp_channel(pbCtrlInfo->sessionID,pbCtrlInfo->localip,pbCtrlInfo->remoteip,
				pbCtrlInfo->port,pbCtrlInfo->enOverTcp,pbCtrlInfo->ssrc,pbCtrlInfo->eventId) < 0)
			{
				pbDownLoadCmd.isValid = 0;
				pauseCmd = 1;
				printf("session:%d,gk_rtp_playback_open_rtp_channel failed!\n",pbCtrlInfo->sessionID);
			}
		}
		else if(pbCtrlInfo->pauseCmd.isValid)
		{
			pauseCmd = 1;
			pbCtrlInfo->pauseCmd.isValid = 0;
			printf("session:%d,pauseCmd!\n",pbCtrlInfo->sessionID);
		}
		else if(pbCtrlInfo->playCmd.isValid)
		{
			memcpy(&playCmd,&pbCtrlInfo->playCmd,sizeof(PLAY_CMD_T));
			pbCtrlInfo->playCmd.isValid = 0;
			pauseCmd = 0;
			printf("session:%d,playCmd!\n",pbCtrlInfo->sessionID);
		}
		else if(pbCtrlInfo->FileHandle == NULL)
		{
			pauseCmd = 1;
		}

		pthread_mutex_unlock(&pbCtrlInfo->playbackLock);

		if(pbDownLoadCmd.isValid)
		{
			if(pbCtrlInfo->FileHandle)
			{
				avi_pb_close(pbCtrlInfo->FileHandle);				
				pbCtrlInfo->FileHandle = NULL;
			}

			try_cnt = 0;
			
			if(download_flag)
			{ 
				if(pbCtrlInfo->downloadCmd.downloadSpeed > 0)
					playback_speed = pbCtrlInfo->downloadCmd.downloadSpeed;
				else
					playback_speed = 4;
			}
			else
				playback_speed = 1.0;

			while(try_cnt < 3)
			{

				playback_start_offset = 0;

				pbCtrlInfo->FileHandle = gb28181_playback_by_time(pbDownLoadCmd.rangeInfo.start_time+try_cnt*5,pbDownLoadCmd.rangeInfo.end_time,download_flag,try_cnt,&playback_start_offset);

				if(pbCtrlInfo->FileHandle)
				{
					u64t_to_time(&src, (pbDownLoadCmd.rangeInfo.start_time+try_cnt*5));
					u64t_to_time(&dst, pbDownLoadCmd.rangeInfo.end_time);
					video_frame_no = 1;
					audio_frame_no = 0;

					total_frame = gb28181_cal_timetick_sub(&src,&dst)*pbCtrlInfo->FileHandle->fps;//pbDownLoadCmd.rangeInfo.end;

					end_timetick = pbDownLoadCmd.rangeInfo.end_timetick;

					int i = 0;
					memset(pb_file_name, 0 ,sizeof(pb_file_name));
					i = last_index_at(pbCtrlInfo->FileHandle->file_path, '/');
					strcpy(pb_file_name, pbCtrlInfo->FileHandle->file_path + i + 1);

					last_video_no = video_frame_no;
					printf("session:%d,playback_start_offset:%d,pb_file_name:%s\n",pbCtrlInfo->sessionID,playback_start_offset,pb_file_name);
					break;
				}
				try_cnt++;
			}

			if(try_cnt == 3)
				continue;
		}

        //printf("pauseCmd:%d\n", pauseCmd);
		if(pauseCmd)
		{
            sleep(1); //usleep(50000);
			continue;
		}

		if(playCmd.isValid)
		{
			if(!pbCtrlInfo->FileHandle)
			{
				pauseCmd = 1;
				memset(&playCmd,0,sizeof(PLAY_CMD_T));
				continue;
			}

			if(playCmd.isNeedSeek)
			{
				if(gb28181_playback_seek_by_offset(pbCtrlInfo->FileHandle,pb_file_name,playback_start_offset + playCmd.rangeInfo.start_time) < 0)
				{
					printf("session:%d,gb28181_playback_seek_by_offset failed!,offset:%lld\n",pbCtrlInfo->sessionID,playCmd.rangeInfo.start_time);

					if(pbCtrlInfo->FileHandle)
					{
						avi_pb_close(pbCtrlInfo->FileHandle);
						pbCtrlInfo->FileHandle = NULL;
					}
					gk_rtp_playback_close_rtp_channel(pbCtrlInfo->sessionID);

					pauseCmd = 1;
					playback_speed = 1.0;
					memset(&playCmd,0,sizeof(PLAY_CMD_T));
					continue;
				}
			}
			else
			{
				//PAUSE ---> PLAY
				gettimeofday(&tv, NULL);
                if (pbCtrlInfo->FileHandle->last_tick != 0)
                	pbCtrlInfo->FileHandle->last_tick = (tv.tv_sec % 1000000) * 1000 + (tv.tv_usec / 1000);
				playback_speed = playCmd.speed;
			}

			memset(&playCmd,0,sizeof(PLAY_CMD_T));

			last_video_no = video_frame_no;
		}

		ret = gb28181_playback_get_frame(pbCtrlInfo->FileHandle, NULL, &frame_size,download_flag,playback_speed);

        if (ret < 0)
		{
            printf("session:%d,call gb28181_playback_get_frame failed. slide to next file 1\n",pbCtrlInfo->sessionID);

			if(pbCtrlInfo->FileHandle->pb_buf)
			{
				free(pbCtrlInfo->FileHandle->pb_buf);
				pbCtrlInfo->FileHandle->pb_buf = NULL;
				pbCtrlInfo->FileHandle->pb_buf_size = 0;
			}

            ret = avi_pb_slide_to_next_file(pbCtrlInfo->FileHandle);
            if (ret == 0)
            {
				ptsStart = 0;

				#if 0 //marked for seamless playback
				video_frame_no = 1;
				audio_frame_no = 0;
				#endif

                continue; //继续下一个文件
            }
        }

		if(ret != 0)
		{
			printf("session:%d,playback_read_frame end!\n",pbCtrlInfo->sessionID);

			ite_eXosip_sendFileEnd(sip_gb28281Obj.excontext, sip_gb28281Obj.sipconfig,pbCtrlInfo->sessionID);

			if(pbCtrlInfo->FileHandle)
			{
				avi_pb_close(pbCtrlInfo->FileHandle);
				pbCtrlInfo->FileHandle = NULL;
			}
			gk_rtp_playback_close_rtp_channel(pbCtrlInfo->sessionID);

            ret = 0;
            if(pbCtrlInfo->sessionID == 0)
                rtp_stream_status_send(1, pbCtrlInfo->remoteip, pbCtrlInfo->port, pbCtrlInfo->ssrc, 0, ret,0,0,0,video_frame_no,&playback_speed);
            else
                rtp_stream_status_send(2, pbCtrlInfo->remoteip, pbCtrlInfo->port, pbCtrlInfo->ssrc, 0, ret,0,0,0,video_frame_no,&playback_speed);

			pauseCmd = 1;

			continue;
		}
		else
		{
			memset(&header,0,sizeof(GK_NET_FRAME_HEADER));
			header.magic = MAGIC_TEST;
			header.device_type = 0;
			header.frame_size = frame_size;

			gettimeofday(&tv, NULL);
			header.sec = tv.tv_sec;
			header.usec = tv.tv_usec;

			if (pbCtrlInfo->FileHandle->node.frame_type == 0x11)
			{
				cal_video_frame_pts(&pts, pbCtrlInfo->FileHandle->node.timetick);
			    header.frame_type = GK_NET_FRAME_TYPE_I;
#if !USE_RECORD_TIMETICK
				header.pts = video_frame_no*(90000/pbCtrlInfo->FileHandle->fps);
#else
				header.pts = pbCtrlInfo->FileHandle->node.timetick;
#endif
			    header.media_codec_type = 0;// CODEC_ID_H264;
			    header.frame_rate = pbCtrlInfo->FileHandle->fps;
			    header.video_reso = ((pbCtrlInfo->FileHandle->video_width<< 16) + pbCtrlInfo->FileHandle->video_height);
			    header.frame_no = ++video_frame_no;
				printf("session:%d,write I frame:%d\n", pbCtrlInfo->sessionID,header.frame_no);

			}
			else if (pbCtrlInfo->FileHandle->node.frame_type == 0x10)
			{

				cal_video_frame_pts(&pts, pbCtrlInfo->FileHandle->node.timetick);
			    header.frame_type = GK_NET_FRAME_TYPE_P;
#if !USE_RECORD_TIMETICK
				header.pts = video_frame_no*(90000/pbCtrlInfo->FileHandle->fps);
#else
				header.pts = pbCtrlInfo->FileHandle->node.timetick;
#endif
			    header.media_codec_type = 0;// CODEC_ID_H264;
			    header.frame_rate = pbCtrlInfo->FileHandle->fps;
			    header.video_reso = ((pbCtrlInfo->FileHandle->video_width<< 16) + pbCtrlInfo->FileHandle->video_height);
			    header.frame_no = ++video_frame_no;
			}
			else
			{
				cal_audio_frame_pts(&pts, pbCtrlInfo->FileHandle->node.timetick);
			    header.frame_type = GK_NET_FRAME_TYPE_A;
#if !USE_RECORD_TIMETICK
				header.pts = pts;
#else
				header.pts = pbCtrlInfo->FileHandle->node.timetick;
#endif
			    header.frame_no = ++audio_frame_no;
                continue;
			}

			if((isSendIframe && header.frame_type == GK_NET_FRAME_TYPE_I) || !isSendIframe)
			{
                if (isSendIframe)
                {
                    if (runVideoCfg.vencStream[runRecordCfg.stream_no].enctype == 1)
                    {
                        streamType = 0x1b; //h264
                    }
                    else if (runVideoCfg.vencStream[runRecordCfg.stream_no].enctype == 3)
                    {
                        streamType = 0x24; //h265
                    }
                    else
                    {
                        streamType = 0x1b; //h264
                    }
                }
				isSendIframe = 0;
                header.video_standard = streamType;
            	ret = gk_rtp_playback_send(pbCtrlInfo->FileHandle->node.one_frame_buf, frame_size, &header,pbCtrlInfo->sessionID,&u16CSeq);
			}
            #if 0
            if (sendCnt % 80 == 0)
            {
                if (sendCnt != 0)
                {
                    gettimeofday(&tt2, NULL);
                    t1 = (tt2.tv_sec-tt1.tv_sec)*1000000+(tt2.tv_usec-tt1.tv_usec);
                }
                printf("--->send cnt:%d, %d\n", sendCnt, t1);
                gettimeofday(&tt1, NULL);
            }
            sendCnt++;
            #endif
#if !USE_RECORD_TIMETICK
			if(video_frame_no >= total_frame || ret<0)
#else
			if((pbCtrlInfo->FileHandle->node.timetick >= end_timetick) || ret<0)
#endif
			{

				ite_eXosip_sendFileEnd(sip_gb28281Obj.excontext, sip_gb28281Obj.sipconfig,pbCtrlInfo->sessionID);

				if(pbCtrlInfo->FileHandle)
				{
					avi_pb_close(pbCtrlInfo->FileHandle);
					pbCtrlInfo->FileHandle = NULL;
				}

				gk_rtp_playback_close_rtp_channel(pbCtrlInfo->sessionID);

#if !USE_RECORD_TIMETICK
				printf("\nsession:%d,video_frame_no = %d,total_frame = %llu!,ret = %d,playback_speed:%f\n",pbCtrlInfo->sessionID,video_frame_no,total_frame,ret,playback_speed);
#endif

				if(pbCtrlInfo->sessionID == 0)
					rtp_stream_status_send(1, pbCtrlInfo->remoteip, pbCtrlInfo->port, pbCtrlInfo->ssrc, 0, ret,0,0,0,video_frame_no,&playback_speed);
				else
					rtp_stream_status_send(2, pbCtrlInfo->remoteip, pbCtrlInfo->port, pbCtrlInfo->ssrc, 0, ret,0,0,0,video_frame_no,&playback_speed);

				video_frame_no = 1;
				audio_frame_no = 0;
				pauseCmd = 1;
				continue;
			}
		}
	}

out:

	if(pbCtrlInfo->FileHandle)
	{
		avi_pb_close(pbCtrlInfo->FileHandle);
		pbCtrlInfo->FileHandle = NULL;
	}
	gk_rtp_playback_close_rtp_channel(pbCtrlInfo->sessionID);

	pthread_mutex_lock(&pbCtrlInfo->playbackLock);

	pbCtrlInfo->sessionID = -1;
	memset(&pbCtrlInfo->playbackCmd,0,sizeof(PLAYBACK_DOWNLOAD_CMD_T));
	memset(&pbCtrlInfo->downloadCmd,0,sizeof(PLAYBACK_DOWNLOAD_CMD_T));
	memset(&pbCtrlInfo->playCmd,0,sizeof(PLAY_CMD_T));
	memset(&pbCtrlInfo->pauseCmd,0,sizeof(PAUSE_CMD_T));

	pthread_mutex_unlock(&pbCtrlInfo->playbackLock);

	printf("%s exit!\n",threadName);

	return NULL;
}

void gk_gb28181_playback_set_download_cmd(int session_id,TIME_RECORD_T *rangeInfo,int speed, char *local_ip, char *media_server_ip,
                    char *media_server_port, unsigned int enOverTcp, int ssrc, int eventId)
{
	int i=0;

	if(!rangeInfo)
		return;

	for(i=0; i<MAX_SESSION_CNT; i++)
	{
		pthread_mutex_lock(&playbackCtrlInfo[i].playbackLock);
		if(playbackCtrlInfo[i].sessionID == session_id)
		{
			playbackCtrlInfo[i].downloadCmd.isValid = 1;
			playbackCtrlInfo[i].downloadCmd.rangeInfo.start_time 		= rangeInfo->start_time;
			playbackCtrlInfo[i].downloadCmd.rangeInfo.end_time	 		= rangeInfo->end_time;
			playbackCtrlInfo[i].downloadCmd.rangeInfo.start_timetick 	= rangeInfo->start_timetick;
			playbackCtrlInfo[i].downloadCmd.rangeInfo.end_timetick	 	= rangeInfo->end_timetick;
			playbackCtrlInfo[i].downloadCmd.downloadSpeed			 	= speed;
			playbackCtrlInfo[i].stopCmd = 0;

			memset(playbackCtrlInfo[i].localip,0,sizeof(playbackCtrlInfo[i].localip));
			memset(playbackCtrlInfo[i].remoteip,0,sizeof(playbackCtrlInfo[i].remoteip));
			strcpy(playbackCtrlInfo[i].localip, local_ip);
			strcpy(playbackCtrlInfo[i].remoteip, media_server_ip);
			playbackCtrlInfo[i].port  = atoi(media_server_port);
			playbackCtrlInfo[i].enOverTcp = enOverTcp;
			playbackCtrlInfo[i].ssrc = ssrc;
			playbackCtrlInfo[i].eventId = eventId;
			pthread_mutex_unlock(&playbackCtrlInfo[i].playbackLock);
			break;
		}
		pthread_mutex_unlock(&playbackCtrlInfo[i].playbackLock);
	}


}

void gk_gb28181_playback_set_playback_cmd(int session_id,TIME_RECORD_T *rangeInfo,char *local_ip, char *media_server_ip,
                    char *media_server_port, unsigned int enOverTcp, int ssrc, int eventId)
{
	int i=0;

	if(!rangeInfo)
		return;


	for(i=0; i<MAX_SESSION_CNT; i++)
	{
		pthread_mutex_lock(&playbackCtrlInfo[i].playbackLock);

		if(playbackCtrlInfo[i].sessionID == session_id)
		{
			playbackCtrlInfo[i].playbackCmd.isValid = 1;
			playbackCtrlInfo[i].playbackCmd.rangeInfo.start_time 		= rangeInfo->start_time;
			playbackCtrlInfo[i].playbackCmd.rangeInfo.end_time			= rangeInfo->end_time;
			playbackCtrlInfo[i].playbackCmd.rangeInfo.start_timetick 	= rangeInfo->start_timetick;
			playbackCtrlInfo[i].playbackCmd.rangeInfo.end_timetick		= rangeInfo->end_timetick;
			playbackCtrlInfo[i].stopCmd = 0;

			memset(playbackCtrlInfo[i].localip,0,sizeof(playbackCtrlInfo[i].localip));
			memset(playbackCtrlInfo[i].remoteip,0,sizeof(playbackCtrlInfo[i].remoteip));
			strcpy(playbackCtrlInfo[i].localip, local_ip);
			strcpy(playbackCtrlInfo[i].remoteip, media_server_ip);
			playbackCtrlInfo[i].port  = atoi(media_server_port);
			playbackCtrlInfo[i].enOverTcp = enOverTcp;
			playbackCtrlInfo[i].ssrc = ssrc;
			playbackCtrlInfo[i].eventId = eventId;
			pthread_mutex_unlock(&playbackCtrlInfo[i].playbackLock);
			break;
		}
		
		pthread_mutex_unlock(&playbackCtrlInfo[i].playbackLock);
	}


}

void gk_gb28181_playback_set_play_cmd(int session_id,float speed,int isNeedSeek,TIME_RECORD_T *rangeInfo)
{
	int i=0;

	if(!rangeInfo)
		return;


	for(i=0; i<MAX_SESSION_CNT; i++)
	{
		pthread_mutex_lock(&playbackCtrlInfo[i].playbackLock);

		if(playbackCtrlInfo[i].sessionID == session_id)
		{
			playbackCtrlInfo[i].playCmd.isValid 				 = 1;
			playbackCtrlInfo[i].playCmd.isNeedSeek 				 = isNeedSeek;
			playbackCtrlInfo[i].playCmd.speed 					 = speed;
			playbackCtrlInfo[i].playCmd.rangeInfo.start_time 	 = rangeInfo->start_time;
			playbackCtrlInfo[i].playCmd.rangeInfo.end_time	 	 = rangeInfo->end_time;
			playbackCtrlInfo[i].stopCmd = 0;
			pthread_mutex_unlock(&playbackCtrlInfo[i].playbackLock);
			break;
		}
		pthread_mutex_unlock(&playbackCtrlInfo[i].playbackLock);
	}


}

void gk_gb28181_playback_set_pause_cmd(int session_id,int pauseTime)
{
	int i=0;


	for(i=0; i<MAX_SESSION_CNT; i++)
	{
		pthread_mutex_lock(&playbackCtrlInfo[i].playbackLock);

		if(playbackCtrlInfo[i].sessionID == session_id)
		{
			playbackCtrlInfo[i].pauseCmd.isValid	= 1;
			playbackCtrlInfo[i].pauseCmd.pauseTime = pauseTime;
			playbackCtrlInfo[i].stopCmd = 0;
			
			pthread_mutex_unlock(&playbackCtrlInfo[i].playbackLock);
			break;
		}
		pthread_mutex_unlock(&playbackCtrlInfo[i].playbackLock);
	}

}

void gk_gb28181_playback_set_stop_cmd(int session_id)
{
	int i=0;


	for(i=0; i<MAX_SESSION_CNT; i++)
	{
		pthread_mutex_lock(&playbackCtrlInfo[i].playbackLock);

		if(playbackCtrlInfo[i].sessionID == session_id)
		{
			playbackCtrlInfo[i].stopCmd = 1;
			pthread_mutex_unlock(&playbackCtrlInfo[i].playbackLock);
			break;
		}
		
		pthread_mutex_unlock(&playbackCtrlInfo[i].playbackLock);
	}

}

int gk_gb28181_playback_session_open(int session_id)
{
	int ret,i=0;
    pthread_t threadID;


	for(i=0; i<MAX_SESSION_CNT; i++)
	{
		if(playbackCtrlInfo[i].sessionID == session_id)
		{
			printf("%s,session already open!\n", __FUNCTION__);

			return 0;
		}
	}

	for(i=0; i<MAX_SESSION_CNT; i++)
	{
		if(playbackCtrlInfo[i].sessionID == -1)
		{
			playbackCtrlInfo[i].sessionID = session_id;
			break;
		}
	}


	if(i == MAX_SESSION_CNT)
	{
		printf("%s,session full!\n", __FUNCTION__);
		return -1;
	}

	if((ret = pthread_create(&threadID, NULL, &gb28181_playback_thread, (void *)&playbackCtrlInfo[i])))
	{
		printf("pthread_create sip_playback_thread failed ret=%d\n", ret);
		return -1;
	}

	return 0;
}

int gk_gb28181_playback_init(void)
{
	int i=0;

	memset(&playbackCtrlInfo,0,sizeof(playbackCtrlInfo));

	for(i=0; i<MAX_SESSION_CNT; i++)
	{
		playbackCtrlInfo[i].sessionID = -1;
		pthread_mutex_init(&playbackCtrlInfo[i].playbackLock, NULL);
	}

	return 0;
}

