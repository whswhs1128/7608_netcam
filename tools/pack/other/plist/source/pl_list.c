/*
****************************************************************************
** \file      /home/amik/GokeFiles/plist/pl_list.c
**
** \version   $Id: pl_list.c 0 2016-05-09 11:26:59Z dengbiao $
**
** \brief     videc abstraction layer header file.
**
** \attention THIS SAMPLE CODE IS PROVIDED AS IS. GOFORTUNE SEMICONDUCTOR
**            ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**            OMMISSIONS.
**
** (C) Copyright 2015-2016 by GOKE MICROELECTRONICS CO.,LTD
**
****************************************************************************
*/

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/epoll.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <sys/queue.h>
#include <sys/prctl.h>
#include <sys/syscall.h>
#include <linux/sched.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>
#include <time.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include <mqueue.h>
#include <time.h>
#include <limits.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <sched.h>
#include <getopt.h>
#include <pwd.h>

#include "pl_types.h"
#include "pl_log.h"

extern u32 HZ;
extern s32 gPID;
extern s32 gDELAY;

typedef struct {
    u32         pid;
    char        comm[32];
    char        task_state;
    u32         ppid;
    u64         utime;
    u64         stime;
    u64         cutime;
    u64         cstime;
    u64         start_time;
    u64         vsize;
    u64         rss;
    u32         priority;
    u32         policy;
    u32         avg_load;
    struct timeval tv;
} ThreadStatT;

typedef struct thread_node_t{
    char                        thread_name[20];
    s32                         thread_tid;
    ThreadStatT                 thread_stat;
    bool                        thread_online;
    SLIST_ENTRY(thread_node_t)  entries;
}thread_t;

SLIST_HEAD(thread_head_t, thread_node_t) threadHead =
    SLIST_HEAD_INITIALIZER(thread_head_t);

static int dump_by_file(const char *file, char buffer[], int bsize)
{
    int fd = -1;
    if (unlikely(!file || !buffer)) {
        LogE("Params error.");
        return -1;
    }

    if (unlikely(access(file, R_OK | F_OK))) {
        LogE("file(%s) is no exist or can not read.", file);
        return -2;
    }

    if (unlikely((fd = open(file, O_RDONLY)) < 0)) {
        return -2;
    }

    read(fd, buffer, bsize);

    close(fd);
    return 0;
}

static int getStringBySystem(const char *cmd, char *buff, int bufsize)
{
    FILE *fp = NULL;
	LogI("==> %s\n", cmd);
    if (NULL == (fp = popen(cmd, "r")))
    {
        goto err;
    }
    if (buff != NULL && NULL == fgets(buff, bufsize, fp))
    {
        goto err;
    }
    pclose(fp);
    return 0;
err:
	pclose(fp);
    return -1;
}

static inline pl_cut_tail(char *str)
{
    while(*str != '\0') {
        if(*str == '\n'){
            *str = '\0';
            break;
        }
        str++;
    }
}

static int pl_obtain_thread_status(s32 pid,
    thread_t* thread, ThreadStatT *thstat)
{
    char *ptxbuffer = NULL;
    char pt_buffer[128];
    char text_buffer[1024];
    u64 time_cnt_pre = 0, time_cnt_cur = 0;
    u64 time_use_pre = 0, time_use_cur = 0;


    /* read thread status */
    time_use_pre = thstat->tv.tv_sec * HZ +
                    (thstat->tv.tv_usec * HZ)/1000000;
    time_cnt_pre = thstat->utime + thstat->stime +
                    thstat->cutime + thstat->cstime;
    snprintf(pt_buffer, sizeof(pt_buffer), "/proc/%d/task/%d/stat",
                pid, thread->thread_tid);
    dump_by_file(pt_buffer, text_buffer, sizeof(text_buffer));
    gettimeofday(&thstat->tv, NULL);

    ptxbuffer = text_buffer;
    while (*ptxbuffer != '\0' && *ptxbuffer != '(') ptxbuffer++;
    while (*ptxbuffer != '\0' && *ptxbuffer != ')') {
        if(unlikely(*ptxbuffer == ' ')) *ptxbuffer = '_';
        ptxbuffer++;
    }

    sscanf(text_buffer, "%u %s %c %u %*u %*u %*u %*u "
        "%*u %*u %*u %*u %*u %Lu %Lu %Lu %Lu %*u %*u %*u %*u %Lu %Lu "
        "%Lu %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u %*u "
        "%*u %*u %u %u %*u %*u %*u %*u %*u %*u",
        &thstat->pid, thstat->comm, &thstat->task_state, &thstat->ppid,
        &thstat->utime, &thstat->stime, &thstat->cutime, &thstat->cstime,
        &thstat->start_time, &thstat->vsize, &thstat->rss, &thstat->priority,
        &thstat->policy);

    time_use_cur = thstat->tv.tv_sec * HZ +
                    (thstat->tv.tv_usec * HZ)/1000000;
    time_cnt_cur = thstat->utime + thstat->stime + thstat->cutime + thstat->cstime;
    thstat->avg_load = (time_cnt_cur - time_cnt_pre) * 10000 /
                        ((time_use_cur - time_use_pre)?:INT_MAX);

    return 0;
}

int pl_lists_init(s32 pl_pid)
{
    char pt_buffer[128];
    char text_buffer[2048];
    char *read_point = text_buffer;
    s32 tid = 0;
    thread_t *pl_thread = NULL;

    snprintf(pt_buffer, sizeof(pt_buffer), "ls /proc/%d/task/ | xargs", pl_pid);
    getStringBySystem(pt_buffer, text_buffer, sizeof(text_buffer));
    text_buffer[2047] = '\0';
    LogI("task:%s", text_buffer);
    while(*read_point != '\0') {
        LogI("point:%d", *read_point);
        sscanf(read_point, "%d ", &tid);
        pl_thread = calloc(1, sizeof(thread_t));
        if (pl_thread == NULL) {
            LogE("memory is no enough!");
            return -1;
        }
        pl_thread->thread_tid = tid;
        pl_thread->thread_online = true;
        /* read thread name */
        snprintf(pt_buffer, sizeof(pt_buffer),
                                "/proc/%d/task/%d/comm",
                                pl_pid, pl_thread->thread_tid);
        dump_by_file(pt_buffer, pl_thread->thread_name, sizeof(pl_thread->thread_name));
        pl_cut_tail(pl_thread->thread_name);
        LogI("Find thread: %s tid:%d", pl_thread->thread_name, pl_thread->thread_tid);

        SLIST_INSERT_HEAD(&threadHead, pl_thread, entries);

        pl_obtain_thread_status(pl_pid, pl_thread, &pl_thread->thread_stat);
        while(*read_point != ' ' && *read_point != '\0')
            read_point++;
        while(*read_point == ' ' || *read_point == '\n')
            read_point++;
    }
}

static int pl_lists_update(s32 pl_pid)
{
    char pt_buffer[128];
    char text_buffer[2048];
    char *read_point = text_buffer;
    s32 tid = 0;
    thread_t *pl_thread = NULL;

    snprintf(pt_buffer, sizeof(pt_buffer), "ls /proc/%d/task/ | xargs", pl_pid);
    getStringBySystem(pt_buffer, text_buffer, sizeof(text_buffer));
    text_buffer[2047] = '\0';
    pl_cut_tail(text_buffer);
    while(*read_point != '\0') {
        sscanf(read_point, "%d ", &tid);
        SLIST_FOREACH(pl_thread, &threadHead, entries) {
            if (pl_thread->thread_stat.pid == tid) {
                pl_thread->thread_online = true;
                break;
            }
        }
        /* no find this thread, need add to lists of thread statistic. */
        if (pl_thread == NULL) {
            pl_thread = calloc(1, sizeof(thread_t));
            if (pl_thread == NULL) {
                LogE("memory is no enough!");
                return -1;
            }
            pl_thread->thread_tid = tid;
            pl_thread->thread_online = true;
            /* read thread name */
            snprintf(pt_buffer, sizeof(pt_buffer),
                                    "/proc/%d/task/%d/comm",
                                    pl_pid, pl_thread->thread_tid);
            dump_by_file(pt_buffer, pl_thread->thread_name, sizeof(pl_thread->thread_name));
            pl_cut_tail(pl_thread->thread_name);
            LogI("Find thread: %s tid:%d", pl_thread->thread_name, pl_thread->thread_tid);

            SLIST_INSERT_HEAD(&threadHead, pl_thread, entries);

            pl_obtain_thread_status(pl_pid, pl_thread, &pl_thread->thread_stat);
        }
        while(*read_point != ' ' && *read_point != '\0')
            read_point++;
        while(*read_point == ' ' || *read_point == '\n')
            read_point++;
    }
}

#define PL_CVT_TO_STRING_1(n) #n
#define PL_CVT_TO_STRING(n) PL_CVT_TO_STRING_1(n)
static void pl_thread_statistics(s32 pid)
{
    thread_t *handlePtr = NULL;
    char text_buffer[128];
    u32 avg_cpu = 0;

    printf("\033[2J\033[0;0H");
    printf("\n");
    printf(" +------------------------------------------+\n");
    printf(" |       Thread statistic lists("PL_CVT_TO_STRING(VERSIONS)")        |\n");
    printf(" +-------+------+-------+-------------------+\n");
    printf(" |  Tid  | Stat |  CPU  |    Thread Name    |\n");
    printf(" +-------+------+-------+-------------------+\n");

    SLIST_FOREACH(handlePtr, &threadHead, entries) {
        if(handlePtr->thread_online) {
            handlePtr->thread_online = false;
            pl_obtain_thread_status(pid, handlePtr, &handlePtr->thread_stat);
            printf(" |\033[35m%6d\033[0m |"
                "   \033[36m%c\033[0m  |"
                "\033[33m%3d.%02d\033[0m%%|"
                "\033[34m%18.18s\033[0m |\n",
                handlePtr->thread_tid,
                handlePtr->thread_stat.task_state,
                handlePtr->thread_stat.avg_load / 100,
                handlePtr->thread_stat.avg_load % 100,
                handlePtr->thread_name);
            avg_cpu += handlePtr->thread_stat.avg_load;
        } else {
            SLIST_REMOVE(&threadHead, handlePtr, thread_node_t, entries);
            free(handlePtr);
        }
    }
    printf(" +-------+------+-------+-------------------+\n");
    printf(" |\033[35m%6d\033[0m |"
                "   \033[36m%c\033[0m  |"
                "\033[33m%3d.%02d\033[0m%%|"
                "\033[34m%18.18s\033[0m |\n",
                gPID,
                'S',
                avg_cpu / 100,
                avg_cpu % 100,
                "Total");
    printf(" +-------+------+-------+-------------------+\n");
    memset(text_buffer, 0, sizeof(text_buffer));
    dump_by_file("/proc/loadavg", text_buffer, sizeof(text_buffer));
    pl_cut_tail(text_buffer);
    printf(" | CPU Load average:%s |\n", text_buffer);
    printf(" +------------------------------------------+\n");
}

static void *pl_lists_display(void *arg)
{
    while(1){
        pl_lists_update(gPID);
        pl_thread_statistics(gPID);
        sleep(gDELAY);
    }
}

int pl_display_start(void)
{
    pthread_t pth;
    return pthread_create(&pth, NULL, pl_lists_display, NULL);
}
