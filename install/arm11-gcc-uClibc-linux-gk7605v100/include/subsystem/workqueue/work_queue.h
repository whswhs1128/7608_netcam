#ifndef WORK_QUEUE_INCLUDE_H
#define WORK_QUEUE_INCLUDE_H

//��������
enum
{
	//����ʱ������
	EVERY_MON_TIMER_WORK = 0,    //EVERY_MON_TIMER_WORK~TIME_TIMER_WORK �����޴�
	EVERY_WEEK_TIMER_WORK,       //����������
	EVERY_DAY_TIMER_WORK,
	EVERY_HOUR_TIMER_WORK,
	EVERY_MIN_TIMER_WORK,
	
	//ʱ��������
	TIME_TIMER_WORK,

	EVENT_TIMER_WORK,           //ֻ����1�ε�����
	CONDITION_TIMER_WORK        //�������޴�������WORK_CALLBACK����0ʱ������ִ������
};

//�������е�����
typedef void* workqueue_t;

/**
	arg: �û�������
	return value: ��ʹ�� CONDITION_TIMER_WORKʱ������ֵ�����á�
	                   return 0: CONDITION_TIMER_WORKִ�н���������ѭ��ִ��
	                   other:     CONDITION_TIMER_WORK����ִ��ֱ������ֵΪ0
*/
typedef int (*WORK_CALLBACK)(void* arg);

//����ṹ��
typedef struct __WORKER
{
	int type;
	int interval;
	int expires;
	WORK_CALLBACK hfunc;
	void* arg;
}WORKER;

/**
	����һ������
	name: worker name
	TYPE:  ��������
	CALLBACK: �ص�����
*/
#define CREATE_WORK(name, TYPE, CALLBACK) \
	WORKER __worker_##name = {.type = (TYPE), .interval = 0,\
	.expires = 0, .hfunc = (CALLBACK), .arg = 0}

/**
	��ʼ������
	name: worker name
	time:  ʹ��COMPUTE_TIME()������
	userdata: �û����ݴ����ص�����ʹ��
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
	������ӵ���������
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
	��������Ķ�ʱʱ��
	day: 0~31, if 0, no concern
	hour: 0~23
	min:  0~59
	sec:   0~any
	usec: 0~1000000
*/
int COMPUTE_TIME(int day, int hour, int min, int sec, int usec);

/**
	���������Ұ�������빤������
	type: ��������
	time: COMPUTE_TIME() ����
	call: �ص�����
	userdata: �û�����
*/
void CRTSCH_DEFAULT_WORK(int type, int time, WORK_CALLBACK call);
void CRTSCH_DEFAULT_WORK1(int type, int time, WORK_CALLBACK call, void* userdata);



//Ĭ�Ϲ������еĴ������ͷ�
void start_default_workqueue();
void stop_default_workqueue();

#endif //WORK_QUEUE_INCLUDE_H

