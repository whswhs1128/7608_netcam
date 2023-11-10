#ifndef WORK_QUEUE_INCLUDE_H
#define WORK_QUEUE_INCLUDE_H

//任务类型
enum
{
	//定点时间任务
	EVERY_MON_TIMER_WORK = 0,    //EVERY_MON_TIMER_WORK~TIME_TIMER_WORK 是无限次
	EVERY_WEEK_TIMER_WORK,       //工作的任务
	EVERY_DAY_TIMER_WORK,
	EVERY_HOUR_TIMER_WORK,
	EVERY_MIN_TIMER_WORK,
	
	//时间间隔任务
	TIME_TIMER_WORK,

	EVENT_TIMER_WORK,           //只工作1次的任务
	CONDITION_TIMER_WORK        //工作有限次数，当WORK_CALLBACK返回0时，不在执行任务
};

//工作队列的类型
typedef void* workqueue_t;

/**
	arg: 用户的数据
	return value: 当使用 CONDITION_TIMER_WORK时，返回值才有用。
	                   return 0: CONDITION_TIMER_WORK执行结束，不再循环执行
	                   other:     CONDITION_TIMER_WORK继续执行直到返回值为0
*/
typedef int (*WORK_CALLBACK)(void* arg);

//任务结构体
typedef struct __WORKER
{
	int type;
	int interval;
	int expires;
	WORK_CALLBACK hfunc;
	void* arg;
}WORKER;

/**
	创建一个任务
	name: worker name
	TYPE:  任务类型
	CALLBACK: 回调函数
*/
#define CREATE_WORK(name, TYPE, CALLBACK) \
	WORKER __worker_##name = {.type = (TYPE), .interval = 0,\
	.expires = 0, .hfunc = (CALLBACK), .arg = 0}

/**
	初始化任务
	name: worker name
	time:  使用COMPUTE_TIME()来计算
	userdata: 用户数据传给回调函数使用
*/
#define INIT_WORK(name, time, userdata) \
do {\
	__worker_##name.interval = (time); \
	__worker_##name.arg = (userdata); \
}while (0)

/**
	default work queue
*/
extern workqueue_t g_workqueue;

/**
	create a work queue
*/
workqueue_t create_workqueue();

/**
	free a work queue
*/
void free_workqueue(workqueue_t wqt);

/**
	把任务加到工作队列
*/
void schedule(workqueue_t wqt, WORKER* worker);

/**
	schedule a worker in default work queue
*/
#define SCHEDULE_DEFAULT_WORK(name) \
	schedule(g_workqueue, &__worker_##name);

/**
	schedule a worker in work queue
*/
#define SCHEDULE_WORK(workqueue, name) \
	schedule(workqueue, &__worker_##name);

/**
	计算任务的定时时间
	day: 0~31, if 0, no concern
	hour: 0~23
	min:  0~59
	sec:   0~any
	usec: 0~1000000
*/
int COMPUTE_TIME(int day, int hour, int min, int sec, int usec);

/**
	创建任务并且把任务加入工作队列
	type: 任务类型
	time: COMPUTE_TIME() 计算
	call: 回调函数
	userdata: 用户数据
*/
void CRTSCH_DEFAULT_WORK(int type, int time, WORK_CALLBACK call);
void CRTSCH_DEFAULT_WORK1(int type, int time, WORK_CALLBACK call, void* userdata);



//默认工作队列的创建和释放
void start_default_workqueue();
void stop_default_workqueue();

#endif //WORK_QUEUE_INCLUDE_H

