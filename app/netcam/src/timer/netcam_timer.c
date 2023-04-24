#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <ctype.h>
#include <pthread.h>
#include <unistd.h>

#include "sdk_def.h"
#include "netcam_api.h"
#include "net_dhcp.h"
#include "net_utils.h"
#include "sdk_network.h"
#include "sdk_sys.h"



#define TIMEER_NUMBER  10
volatile char timerRun = 0;

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(array)   (sizeof(array) / sizeof(array[0]))
#endif

typedef struct tagAppTimer
{
    SDK_U32       stamp;              /* the start timestamp*/
    SDK_U32       interval;           /* the interval of the timer */
    SDK_BOOL      oneshot;            /* one shot flag */
    SDK_U32       id;                 /* Timer id */
    TIMER_HANDLER      fpcallback;    /* Timer id */
    TIMER_HANDLER2     fpcallback2;  /* Timer id */
    SDK_U8          argNum;
    void *         arg1;           /* argument */
    void *         arg2;
}TIMER_INFO_T;
static TIMER_INFO_T g_timers[TIMEER_NUMBER];

static pthread_t timerId ;
static pthread_rwlock_t timer_lock ;

SDK_U32 timer_get_ms()
{
    struct timeval tv;
    SDK_U32 time;
    gettimeofday(&tv, NULL);
    time = tv.tv_sec*1000  + tv.tv_usec / 1000;
    return time;
}
int netcam_timer_add_task(TIMER_HANDLER const task_handler, int interl, SDK_BOOL immediate,SDK_BOOL oneshot)
{
    int i;
    int ret = 0;

    if ((timerRun == 0) || (task_handler == NULL)) {
        return -1;
    }
    pthread_rwlock_wrlock(&timer_lock);

    int usable_id = 1110;
    for (i = 0; i < TIMEER_NUMBER; i++) {
        if (g_timers[i].id == 0 && (usable_id == 1110)) {
            usable_id = i;
        }

        if (g_timers[i].id != 0 &&  task_handler == g_timers[i].fpcallback) {
            usable_id = 1110;
            break;
        }
    }

    if (i == TIMEER_NUMBER) {
        if(usable_id == 1110) {// timer list full.
            LOG_ERR("netcam_timer_add_task: timer list full\n");
            ret = -1;
            goto OUT;
        } else {// create new timer.
            i = usable_id;
        }
    }// else find old timer, update it.

    memset(&g_timers[i],0,sizeof(TIMER_INFO_T));
    g_timers[i].fpcallback = task_handler;
    if(immediate == SDK_TRUE)
    {
        g_timers[i].stamp = 0;
    }
    else
    {
        g_timers[i].stamp =  timer_get_ms();
    }

    g_timers[i].interval = interl*250; // <= 400, in order to update real time to OSD.
    g_timers[i].id = i+1;
    g_timers[i].oneshot = oneshot;
    g_timers[i].argNum = 0;

OUT:
    pthread_rwlock_unlock(&timer_lock);
    return ret;

}
int netcam_timer_add_task2(TIMER_HANDLER2 const task_handler, int interl,SDK_BOOL immediate,SDK_BOOL oneshot, void *lPara, void *pPara)
{
    int i;
    int ret = 0;
    if ((timerRun == 0) || (task_handler == NULL)) {
        return -1;
    }
    pthread_rwlock_wrlock(&timer_lock);

    int usable_id = 1110;
    for (i = 0; i < TIMEER_NUMBER; i++) {
        if (g_timers[i].id == 0 && (usable_id == 1110)) {
            usable_id = i;
        }

        if (g_timers[i].id != 0 &&  task_handler == g_timers[i].fpcallback2) {
            usable_id = 1110;
            break;
        }
    }

    if (i == TIMEER_NUMBER) {
        if(usable_id == 1110) {// timer list full.
            LOG_ERR("netcam_timer_add_task2: timer list full\n");
            ret = -1;
            goto OUT;
        } else {// create new timer.
            i = usable_id;
        }
    }// else find old timer, update it.

    memset(&g_timers[i],0,sizeof(TIMER_INFO_T));
    g_timers[i].fpcallback2 = task_handler;
    if(immediate == SDK_TRUE)
    {
        g_timers[i].stamp = 0;
    }
    else
    {
        g_timers[i].stamp =  timer_get_ms();
    }
    g_timers[i].id = i+1;
    g_timers[i].interval = interl*1000; // ms
    g_timers[i].oneshot = oneshot;
    g_timers[i].argNum = 2;
    g_timers[i].arg1 = lPara;
    g_timers[i].arg2 = pPara;

OUT:
    pthread_rwlock_unlock(&timer_lock);
    return ret;
}
void netcam_timer_del_task(TIMER_HANDLER const task_handler)
{
    int i;
    if(timerRun == 0)
    {
        return;
    }
    for(i = 0; i < TIMEER_NUMBER; i++)
    {
        if(g_timers[i].id != 0 &&  task_handler == g_timers[i].fpcallback)
        {
            break;
        }
    }

    if(i != TIMEER_NUMBER)
    {
        pthread_rwlock_wrlock(&timer_lock);
        memset(&g_timers[i],0,sizeof(TIMER_INFO_T));
        pthread_rwlock_unlock(&timer_lock);
    }
}

void netcam_timer_del_task2(TIMER_HANDLER2 const task_handler)
{
    int i;
    if(timerRun == 0)
    {
        return;
    }

    for(i = 0; i < TIMEER_NUMBER; i++)
    {
        if(g_timers[i].id != 0 &&  task_handler == g_timers[i].fpcallback2)
        {
            break;
        }
    }

    if(i != TIMEER_NUMBER)
    {
        pthread_rwlock_wrlock(&timer_lock);
        memset(&g_timers[i],0,sizeof(TIMER_INFO_T));
        pthread_rwlock_unlock(&timer_lock);
    }



}


static void *timer_thread(void *arg)
{
    int i;
    TIMER_INFO_T *timer;
    TIMER_INFO_T extimer;
    SDK_U32 timeStap ;

    sdk_sys_thread_set_name("timer_thread");
    while(timerRun)
    {
        for(i = 0; i < TIMEER_NUMBER; i++)
        {
            if(g_timers[i].id != 0 )
            {
                pthread_rwlock_wrlock(&timer_lock);
                timer = &g_timers[i];
                timeStap = timer_get_ms();
                if( (timeStap < timer->stamp) || (timeStap >(timer->stamp+timer->interval)) )
                {

                    memcpy(&extimer,timer,sizeof(TIMER_INFO_T));
                    if(timer->oneshot == SDK_TRUE)
                    {
                        memset(timer,0,sizeof(TIMER_INFO_T));
                    }
                    else
                    {
                        timer->stamp = timer_get_ms();
                    }

                    pthread_rwlock_unlock(&timer_lock);
                    if(extimer.argNum == 0)
                    {
                        extimer.fpcallback();
                    }
                    else
                    {
                        extimer.fpcallback2(extimer.arg1,extimer.arg2);
                    }
                }
                else
                {
                    pthread_rwlock_unlock(&timer_lock);
                }
            }

        }

        usleep(100000);
    }
	
	printf("timer_thread exit!\n");

	pthread_exit(NULL);
	
    return NULL;
}

int  netcam_timer_init(void)
{
    if(timerRun == 0)
    {
        pthread_rwlock_init(&timer_lock, NULL);
        memset(g_timers,0,sizeof(g_timers));
        timerRun = 1;
        pthread_create(&timerId, NULL, timer_thread, NULL);
    }
    return 0;

}
void netcam_timer_destroy()
{
    if(timerRun == 1)
    {
        timerRun = 0;
		
		printf("####netcam_timer_destroy 1!\n");
		
        pthread_join(timerId,NULL);
        pthread_rwlock_wrlock(&timer_lock);
        memset(g_timers,0,sizeof(g_timers));
        pthread_rwlock_unlock(&timer_lock);
		
		printf("@@@@@netcam_timer_destroy 2!\n");
    }

}
