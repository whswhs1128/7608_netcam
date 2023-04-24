#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include "eventalarm.h"
//#include "sdk/sdk_debug.h"

#define LOG_ERR(fmt,args...) printf("[%s] Line: %d,"fmt" \n",  __FUNCTION__, __LINE__,##args)

typedef struct EVNET_ALARM_API
{
    event_alarm_cb_func alarm_cb;

    struct EVNET_ALARM_API *next;

}ST_EVNET_ALARM_API, *PS_ST_EVNET_ALARM_API;


//不为0时屏蔽所有PT接口的操作
static int g_ptapi_stop = 1;
static ST_EVNET_ALARM_API *g_ptapi = NULL;
static pthread_mutex_t event_lock;

int event_alarm_init(void)
{
    if(g_ptapi_stop == 0)
    {
        LOG_ERR("has inited");
        return -1;
    }

    pthread_mutex_init(&event_lock, NULL);

    g_ptapi = NULL;
    g_ptapi_stop = 0;
    return 0;
}

int event_alarm_exit(void)
{
    ST_EVNET_ALARM_API *pPtApi = g_ptapi;
    ST_EVNET_ALARM_API *p1;

    if(g_ptapi_stop == 0)
    {
        g_ptapi_stop = 1;
        pthread_mutex_lock(&event_lock);
         while (pPtApi ) {
            p1 = pPtApi;
            pPtApi = pPtApi->next;
            free(p1);
        }
        pthread_mutex_unlock(&event_lock);
        pthread_mutex_destroy(&event_lock);

    }
    return 0;
}



void* event_alarm_open(event_alarm_cb_func alarm_cb)
{
    ST_EVNET_ALARM_API *pPtApi = NULL;

    if (g_ptapi_stop != 0) {
        LOG_ERR("not init");
        return NULL;
    }

    if(alarm_cb == NULL)
    {
        LOG_ERR("parameter error");
        return NULL;
    }


    pPtApi = (ST_EVNET_ALARM_API *)malloc(sizeof(ST_EVNET_ALARM_API));
    if (pPtApi == NULL) {
        LOG_ERR("malloc error");
        return NULL;
    }
    memset(pPtApi, 0, sizeof(ST_EVNET_ALARM_API));

    pthread_mutex_lock(&event_lock);
    pPtApi->alarm_cb = alarm_cb;

    if (g_ptapi == NULL) {
        g_ptapi = pPtApi;
    } else {
       pPtApi->next = g_ptapi;
       g_ptapi = pPtApi;
    }
    pthread_mutex_unlock(&event_lock);
    return (void*)(pPtApi);
}

int event_alarm_close(void *handle)
{
    ST_EVNET_ALARM_API  *p1 = g_ptapi;
    ST_EVNET_ALARM_API  *p2 = NULL;
    if(handle == NULL || g_ptapi_stop )
    {
        return 0;
    }

    pthread_mutex_lock(&event_lock);

    while (p1 != NULL ) {
        if (p1 == handle) {
            break;
        }
        else
        {
            p2 = p1;
        }
        p1 = p1->next;
    }
    // 找到handle
    if(p1 != NULL)
    {
        if(p1 == g_ptapi)
        {
            g_ptapi = g_ptapi->next;
        }
        else
        {
            p2->next = p1->next;
        }
        free(p1);
    }

    pthread_mutex_unlock(&event_lock);
    return 0;
}


int event_alarm_touch(int nChannel, GK_ALARM_TYPE_E nAlarmType, int nAction, void* pParam)
{
    ST_EVNET_ALARM_API *pPtApi = g_ptapi;
    while (pPtApi && g_ptapi_stop == 0) {

        pPtApi->alarm_cb(nChannel, nAlarmType, nAction, pParam);
	    pPtApi = pPtApi->next;
    }

    return 0;
}



