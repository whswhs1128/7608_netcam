#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>

#include <sys/time.h>

#include "list.h"
#include "work_queue.h"
#include <sys/prctl.h>

#define ACCURACY_TIME                2
#define SEC_TO_USEC                  1000000
#define SELECT_EXPIRES_TIME          SEC_TO_USEC / ACCURACY_TIME
#define TIME_EXPIRES_UNIT            1

#define SET_THREAD_NAME() \
    prctl(PR_SET_NAME, (unsigned long)__func__, 0,0,0)

//任务结构
typedef struct __WORKER_STRUCT
{
	WORKER worker;

#define ws_type     worker.type
#define ws_interval worker.interval
#define ws_expires  worker.expires
#define ws_hfunc    worker.hfunc
#define ws_arg      worker.arg

	struct list_head plst;
}WORKER_STRUCT;

//工作队列结构体
typedef struct __st_work_queue
{
	volatile int is_done;
	pthread_t thread_id;

	pthread_mutex_t timewait_mutex;
	pthread_mutex_t available_mutex;
	pthread_cond_t  timewait_cond;
	struct list_head timewait_workers;           //定时队列
	struct list_head available_workers;          //可用任务队列，避免任务再次分配
}ST_WORKQUEUE;

//获取到点的任务列表
static void get_expires_workers(ST_WORKQUEUE* wq, struct list_head* head, int expires)
{
	WORKER_STRUCT* w;
	struct list_head* pos;

	pthread_mutex_lock(&wq->timewait_mutex);
	while(wq->is_done && list_empty(&wq->timewait_workers))
		pthread_cond_wait(&wq->timewait_cond, &wq->timewait_mutex);

	//检测整个任务列表，把到点的任务放到head列表中
	pos = wq->timewait_workers.next;
	while (pos && pos != &wq->timewait_workers)
	{
		w = list_entry(pos, WORKER_STRUCT, plst);
		pos = pos->next;
		
		w->ws_expires -= expires;

		if (w->ws_expires <= 0)
		{
			//任务到点
			list_del(&w->plst);
			list_add_tail(&w->plst, head); //加入head 列表
		}
	}
	pthread_mutex_unlock(&wq->timewait_mutex);
}

//把用完的任务放到可用队列中
//避免任务再次分配
static void put_in_available_list(ST_WORKQUEUE* wq, WORKER_STRUCT* w)
{
	pthread_mutex_lock(&wq->available_mutex);
	list_add_tail(&w->plst, &wq->available_workers);
	pthread_mutex_unlock(&wq->available_mutex);
}

//获取一个可用任务
static WORKER_STRUCT* pop_available_worker(ST_WORKQUEUE* wq)
{
	WORKER_STRUCT* w;
	struct list_head* pos;
	
	pthread_mutex_lock(&wq->available_mutex);
	if (list_empty(&wq->available_workers))
		w = 0;
	else
	{
		pos = wq->available_workers.next;
		list_del(pos);
		w = list_entry(pos, WORKER_STRUCT, plst);
	}
	pthread_mutex_unlock(&wq->available_mutex);

	return w;
}

//把任务放进定时队列
static void put_in_timewait_list(ST_WORKQUEUE* wq, WORKER_STRUCT* w)
{
	pthread_mutex_lock(&wq->timewait_mutex);
	list_add_tail(&w->plst, &wq->timewait_workers);
	pthread_mutex_unlock(&wq->timewait_mutex);
	
	if (!list_empty(&wq->timewait_workers))
		pthread_cond_signal(&wq->timewait_cond);
}

//根据任务类型计算时间间隔
static int get_expires_time(int type, int interval)
{
	int ret, day, hour, min, sec, usec;
	time_t expires;
	struct tm tm_exp;
	struct timeval tv;

	ret = interval;
	if (type >= TIME_TIMER_WORK)
		goto out;            //时间间隔任务，直接返回interval

	//定点时间任务
	usec  = (interval % ACCURACY_TIME) * SELECT_EXPIRES_TIME;
	ret   = interval / ACCURACY_TIME;
	day   = ret / 86400;
	hour  = ret % 86400 / 3600;
	min   = ret % 86400 % 3600 / 60;
	sec   = ret % 60;
	
	gettimeofday(&tv, NULL);
	localtime_r(&tv.tv_sec, &tm_exp);
	//tv.tv_usec /= SELECT_EXPIRES_TIME;

	//printf("mon:%d|day:%d|%d:%d:%d|week:%d\n", tm_exp.tm_mon+1, tm_exp.tm_mday, 
	//	tm_exp.tm_hour, tm_exp.tm_min, tm_exp.tm_sec, tm_exp.tm_wday);

	switch (type)
	{
	case EVERY_MON_TIMER_WORK:  //每个月的第几天几时几分几秒
		if (COMPUTE_TIME(tm_exp.tm_mday, tm_exp.tm_hour,
			tm_exp.tm_min, tm_exp.tm_sec, tv.tv_usec) >= interval)
			tm_exp.tm_mon += 1;
		
		tm_exp.tm_mday = day;
		tm_exp.tm_hour = hour;
		tm_exp.tm_min = min;
		tm_exp.tm_sec = sec;
		break;
		
	case EVERY_WEEK_TIMER_WORK:  //每周的第几天几时几分几秒
		if (tm_exp.tm_wday < day)
			tm_exp.tm_mday += day - tm_exp.tm_wday;
		else if (tm_exp.tm_wday > day)
			tm_exp.tm_mday += 7 - (tm_exp.tm_wday - day);
		else if (COMPUTE_TIME(0, tm_exp.tm_hour,
			tm_exp.tm_min, tm_exp.tm_sec, tv.tv_usec) >=
			COMPUTE_TIME(0, hour, min, sec, usec))
			tm_exp.tm_mday += 7;

		tm_exp.tm_hour = hour;
		tm_exp.tm_min = min;
		tm_exp.tm_sec = sec;
		break;
		
	case EVERY_DAY_TIMER_WORK:  //每天的几时几分几秒
		if (COMPUTE_TIME(0, tm_exp.tm_hour,
			tm_exp.tm_min, tm_exp.tm_sec, tv.tv_usec) >= interval)
			tm_exp.tm_mday += 1;

		tm_exp.tm_hour = hour;
		tm_exp.tm_min = min;
		tm_exp.tm_sec = sec;
		break;
		
	case EVERY_HOUR_TIMER_WORK: //每小时的第几分几秒
		if (COMPUTE_TIME(0, 0, tm_exp.tm_min, tm_exp.tm_sec, 
			tv.tv_usec) >= interval)
			tm_exp.tm_hour += 1;
		
		tm_exp.tm_min = min;
		tm_exp.tm_sec = sec;
		break;
		
	case EVERY_MIN_TIMER_WORK: //每分钟的第几秒
		if (COMPUTE_TIME(0, 0, 0, tm_exp.tm_sec, tv.tv_usec) 
			>= interval)
			tm_exp.tm_min += 1;
		
		tm_exp.tm_sec = sec;
		break;
		
	default:
		break;
	}
	
	expires = mktime(&tm_exp);
	
	//localtime_r(&expires, &tm_exp);
	//printf("mon:%d|day:%d|%d:%d:%d|week:%d\n", tm_exp.tm_mon+1, tm_exp.tm_mday, 
	//	tm_exp.tm_hour, tm_exp.tm_min, tm_exp.tm_sec, tm_exp.tm_wday);
	//计算与当前的时间的差值
	gettimeofday(&tv, NULL);
	ret = expires - tv.tv_sec;
	ret *= ACCURACY_TIME;
	ret += interval % ACCURACY_TIME - tv.tv_usec / SELECT_EXPIRES_TIME;

out:
	return ret;
}


//查看加入任务是否重复
static int check_duplicate_work(ST_WORKQUEUE* wq, WORKER* worker, int flag)
{
	int ret = 0;
	WORKER_STRUCT* w;
	struct list_head* pos;
	
	pthread_mutex_lock(&wq->timewait_mutex);
	if (list_empty(&wq->timewait_workers))
		goto out;

	list_for_each(pos, &wq->timewait_workers)
	{
		//当任务的类型，回调函数，用户参数为一样时，为同一任务
		w = list_entry(pos, WORKER_STRUCT, plst);
		if (w->ws_type == worker->type && 
			w->ws_hfunc == worker->hfunc &&
			w->ws_arg == worker->arg)
		{
			if (flag)
			{
				w->ws_interval = worker->interval;
				w->ws_expires = get_expires_time(w->ws_type, w->ws_interval);
			}
			ret = 1;
			goto out;
		}
	}

out:
	pthread_mutex_unlock(&wq->timewait_mutex);
	return ret;
}

static void* worker_thread(void* arg)
{
	WORKER_STRUCT* w;
	struct list_head worker_list;
	struct list_head* pos;
	struct timeval tv, tv1;
	ST_WORKQUEUE* wq = (ST_WORKQUEUE*)arg;
	int ret = 0;
	int timecount = TIME_EXPIRES_UNIT;

	SET_THREAD_NAME();
	printf("[ThreadID 0x%x] work queue thread start!\n", (unsigned int)pthread_self());
	while (wq->is_done)
	{
		//获取到点的任务列表
		INIT_LIST_HEAD(&worker_list);
		get_expires_workers(wq, &worker_list, timecount);
		if (list_empty(&worker_list))
		{
			//如果没有到点任务，睡眠SELECT_EXPIRES_TIME
			tv.tv_sec = 0;
			tv.tv_usec = SELECT_EXPIRES_TIME;
			if (select(0, NULL, NULL, NULL, &tv) < 0)
				usleep(SELECT_EXPIRES_TIME);

			timecount = TIME_EXPIRES_UNIT;
			continue;
		}

		//有到点任务
		gettimeofday(&tv, NULL);
		pos = worker_list.next;
		while (pos && pos != &worker_list)
		{
			w = list_entry(pos, WORKER_STRUCT, plst);
			pos = pos->next;

			list_del(&w->plst);
			if (w->ws_hfunc)
				ret = w->ws_hfunc(w->ws_arg);

			//如果是无限循环任务，继续放到工作队列的列表中
			if (w->ws_type < EVENT_TIMER_WORK)
			{
				if (!check_duplicate_work(wq, &w->worker, 0))
				{
					w->ws_expires = get_expires_time(w->ws_type, w->ws_interval);
					put_in_timewait_list(wq, w);
				}
				else
					//if work dis duplicate, use new work
					put_in_available_list(wq, w);

				continue;
			}
			
			//如果是有限次循环任务
			switch (w->ws_type)
			{
			case CONDITION_TIMER_WORK: //条件任务
				if (!ret)
				{
					put_in_available_list(wq, w);  //返回0,结束任务
				}
				else
				{
					w->ws_expires = get_expires_time(w->ws_type, w->ws_interval); //非0, 任务继续执行
					put_in_timewait_list(wq, w);
				}
				break;

			case EVENT_TIMER_WORK:  //单次任务，结束。把任务结构放到另一个列表
			default:                //以便下次使用，不用再分配空间
				put_in_available_list(wq, w);
				break;
			}
		}

		//计算任务执行花费多长的时间
		gettimeofday(&tv1, NULL);
		timecount = SEC_TO_USEC * (tv1.tv_sec - tv.tv_sec);
		timecount += tv1.tv_usec - tv.tv_usec;
		timecount /= SELECT_EXPIRES_TIME;
        if(timecount < 0) // prevent tv1 < tv when set ntp time, this case can be produce.
        {
            printf("[Work Queue] new time less than old time,reset time\n");
            timecount = 0;
        }
    }

	return 0;
}

//创建工作队列
workqueue_t create_workqueue()
{
	ST_WORKQUEUE* wq;

	wq = (ST_WORKQUEUE*)malloc(sizeof(ST_WORKQUEUE));
	if (!wq)
	{
		printf("Fail to alloc space for work queue! [create_workqueue]\n");
		goto error1;
	}
	memset(wq, 0, sizeof(ST_WORKQUEUE));

	wq->is_done = 1;
	INIT_LIST_HEAD(&wq->available_workers);
	INIT_LIST_HEAD(&wq->timewait_workers);

	if (pthread_mutex_init(&wq->timewait_mutex, 0) < 0)
	{
		printf("timewait mutex init error [create_workqueue]! errno[%d] errinfo[%s]", 
			errno, strerror(errno));
		goto error2;
	}

	if (pthread_mutex_init(&wq->available_mutex, 0) < 0)
	{
		printf("available mutex init error [create_workqueue]! errno[%d] errinfo[%s]", 
			errno, strerror(errno));
		goto error2;
	}

	if (pthread_cond_init(&wq->timewait_cond, 0) < 0)
	{
		printf("timewait cond init error [create_workqueue]! errno[%d] errinfo[%s]", 
			errno, strerror(errno));
		goto error2;
	}
	
	if (pthread_create(&wq->thread_id, 0, worker_thread, wq))
	{
		printf("create work queue thread error[create_workqueue]! errno[%d] errinfo[%s]", 
			errno, strerror(errno));
		goto error2;
	}

	return (workqueue_t)wq;

error2:
	free(wq);
error1:
	return 0;
}

static void free_workers(struct list_head* head)
{
	WORKER_STRUCT* w;
	struct list_head* pos;

	pos = head->next;
	while (pos && pos != head)
	{
		w = list_entry(pos, WORKER_STRUCT, plst);
		pos = pos->next;

		list_del(&w->plst);
		free(w);
	}
}

void free_workqueue(workqueue_t wqt)
{
	ST_WORKQUEUE* wq = (ST_WORKQUEUE*)wqt;
	
	if (!wq)
		return;

	wq->is_done = 0;
	pthread_join(wq->thread_id, 0);
	pthread_mutex_destroy(&wq->timewait_mutex);
	pthread_mutex_destroy(&wq->available_mutex);
	pthread_cond_destroy(&wq->timewait_cond);
	
	free_workers(&wq->timewait_workers);
	free_workers(&wq->available_workers);
	free(wq);
}

//任务加入工作队列
void schedule(workqueue_t wqt, WORKER* worker)
{
	WORKER_STRUCT* w;
	ST_WORKQUEUE* wq = (ST_WORKQUEUE*)wqt;

	if (!wq)
	{
		printf("Work queue is not create![schedule]\n");
		return;
	}

	if (check_duplicate_work(wq, worker, 1))
		return;

	//查看可能队列中是否有可用任务
	w = pop_available_worker(wq);
	if (!w)
	{
		//如果没有，则分配一任务
		w = (WORKER_STRUCT*)malloc(sizeof(WORKER_STRUCT));
		if (!w)
		{
			printf("Fail to alloc space for worker struct![schedule]\n");
			return;
		}
	}

	memset(w, 0, sizeof(WORKER_STRUCT));
	memcpy(&w->worker, worker, sizeof(WORKER));

	//把任务放到定时队列
	w->ws_expires = get_expires_time(w->ws_type, w->ws_interval);
	put_in_timewait_list(wq, w);
}


workqueue_t g_workqueue = 0;

void start_default_workqueue()
{
	g_workqueue = create_workqueue();
}

void stop_default_workqueue()
{
	free_workqueue(g_workqueue);
    g_workqueue = NULL;
}

int COMPUTE_TIME(int day, int hour, int min, int sec, int usec)
{
	if (day || hour || min)
		sec %= 60;
	
	return ACCURACY_TIME * (86400*day + 3600*hour + 60*min + sec) 
		+ usec / SELECT_EXPIRES_TIME;
}

void CRTSCH_DEFAULT_WORK(int type, int time, WORK_CALLBACK call)
{
	CREATE_WORK(TMP, type, call);
	INIT_WORK(TMP, time, NULL);
	SCHEDULE_DEFAULT_WORK(TMP);
}

void CRTSCH_DEFAULT_WORK1(int type, int time, WORK_CALLBACK call, void* userdata)
{
	CREATE_WORK(TMP, type, call);
	INIT_WORK(TMP, time, userdata);
	SCHEDULE_DEFAULT_WORK(TMP);
}


