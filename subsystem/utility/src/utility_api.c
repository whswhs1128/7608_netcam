/*!
*****************************************************************************
** FileName     : utility_api.c
**
** Description  : utility api.
**
** Author       :
** Create Date  : 2015-9-15
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/syscall.h>
#include <sys/prctl.h>
//#include "common.h"
#include "utility_api.h"
#include "pthread.h"

#define PRINT_ERR(fmt,args...) printf("[%s] Line: %d,"fmt" \n",  __FUNCTION__, __LINE__,##args)
#define PRINT_INFO(fmt,args...) printf("Info [%s] Line: %d, "fmt" \n",  __FUNCTION__, __LINE__,##args)

void sys_get_tm(struct tm* t)
{
    time_t tt;
    time(&tt);
    struct tm tm_t;
    struct tm *p_tm = localtime_r(&tt, &tm_t);
    memcpy(t, p_tm, sizeof(struct tm));
    t->tm_year += 1900;
    t->tm_mon += 1;

#if 0
    time_t tt;
    time(&tt);
    struct tm tm_t;
    struct tm *p_tm = gmtime_r(&tt, &tm_t);
    memcpy(t, p_tm, sizeof(struct tm));
    t->tm_year += 1900;
    t->tm_mon += 1;
#endif
}

#if 1
/* nDelay 微妙 */
int sys_sleep(int nDelay)
{
    #if 0
    struct timespec req;

    req.tv_sec = nDelay / 1000000;
    req.tv_nsec = (nDelay % 1000000) * 1000;
    int ret = nanosleep (&req, NULL);
    if (-1 == ret) {
        PRINT_ERR("nanosleep    %8d   not support\n", nDelay);
        return -1;
    }
    return 0;
    #endif

    #if 1
    fd_set rfds;
    struct timeval tv;
    int fd = 1;
    FD_ZERO (&rfds);
    FD_SET (fd, &rfds);
    tv.tv_sec = 0;
    tv.tv_usec = nDelay;
    int ret = select(0, NULL, NULL, NULL, &tv);
    if (-1 == ret) {
        PRINT_ERR("select    %d   not support\n", nDelay);
        return -1;
    }
    return 0;
    #endif
}
#else
/* use usleep directly */
#define sys_sleep       usleep
#endif


pid_t get_thread_id()
{
     return syscall(SYS_gettid);
}


void SetThreadName(const char *name)
{
    prctl(PR_SET_NAME, (unsigned long)name, 0,0,0);
    PRINT_INFO("%s pid:%d tid:%d\n", name, getpid(), get_thread_id());
}

int GenRandInt(int min, int max)
{
	int value = 0;

	srand( (unsigned int)time(NULL) );

	//产生随机数 [10000, 20000]
	value = rand() % (max + 1 - min) + min;

	return value;
}

unsigned long ip_to_dw(const char *ip_src)
{
    char ip_tmp[16] = {0};
    strcpy(ip_tmp, ip_src);

    char *buf = ip_tmp;
    unsigned char dst[4] = {0};
    int i = 0;
    char *p[4];
    char *saveptr = NULL;
    while ((p[i] = strtok_r(buf, ".", &saveptr)) != NULL) {
        dst[i] = (unsigned char)strtoul(p[i], NULL, 0);
        //printf("%u ", dst[i]);
        buf = NULL;
        i ++;
    }
    //printf("\n");
    unsigned long ret = (dst[3]<<24) | (dst[2]<<16) | (dst[1]<<8) | dst[0];

    return ret;
}

void mac_to_dw(unsigned char *dst, char *mac_src)
{
    char mac_tmp[20] = {0};
    strcpy(mac_tmp, mac_src);

    char *buf = mac_tmp;
    int i = 0;
    char *p[6];
    char *saveptr = NULL;
    while ((p[i] = strtok_r(buf, ":", &saveptr)) != NULL) {
        dst[i] = (unsigned char)strtoul(p[i], NULL, 16);
        //printf("%u ", dst[i]);
        buf = NULL;
        i ++;
    }
    //printf("\n");
}


/*
   -------------------------------------------------------------------------
       SIGHUP        1       Term    Hangup detected on controlling terminal
                                     or death of controlling process
       SIGINT        2       Term    Interrupt from keyboard
       SIGQUIT       3       Core    Quit from keyboard
       SIGILL        4       Core    Illegal Instruction
       SIGABRT       6       Core    Abort signal from abort(3)
       SIGFPE        8       Core    Floating point exception
       SIGKILL       9       Term    Kill signal
       SIGSEGV      11       Core    Invalid memory reference
       SIGPIPE      13       Term    Broken pipe: write to pipe with no readers
       SIGALRM      14       Term    Timer signal from alarm(2)
       SIGTERM      15       Term    Termination signal
       SIGUSR1   30,10,16    Term    User-defined signal 1
       SIGUSR2   31,12,17    Term    User-defined signal 2
       SIGCHLD   20,17,18    Ign     Child stopped or terminated
       SIGCONT   19,18,25            Continue if stopped
       SIGSTOP   17,19,23    Stop    Stop process
       SIGTSTP   18,20,24    Stop    Stop typed at tty
       SIGTTIN   21,21,26    Stop    tty input for background process
       SIGTTOU   22,22,27    Stop    tty output for background process
*/
static void signal_hander(int signal)
{
    static int bExit = 0;
    char name[32];
    char erroStr[256] = {0};
    FILE *fd_w;
    if(bExit == 0) {
        bExit = 1;
        printf("____________Application will exit by signal:%d, pid:%d\n", signal, getpid());
        sprintf(erroStr, "____________Application will exit by signal:%d, pid:%d", signal, getpid());
        pid_t tid;
        tid = syscall(SYS_gettid);
        prctl(PR_GET_NAME, (unsigned long)name);
        printf("____________Application exit thread tid: %d, name %s\n", tid, name);
        sprintf(erroStr + strlen(erroStr), "____________Application exit thread tid: %d, name %s", tid, name);

        fd_w = fopen("/tmp/except", "w");
        if (fd_w != NULL)
        {
            fwrite(erroStr, strlen(erroStr), 1, fd_w);
            fflush(fd_w);
            fsync(fileno(fd_w));
            fclose(fd_w);
        }
        #if 0
        if (signal == SIGSEGV) {
            SIG_DFL(signal);
        } else {
            gk710x_stop();
        }
        #else
        if (signal == SIGSEGV) {
            SIG_DFL(signal);
        }
        else if (signal == SIGUSR1) {
            exit(253);
        }
        #endif

    }

}

static void sig_chld(int signo)
{
    pid_t   pid;
    int     stat;

    while ((pid = waitpid(-1, &stat, WNOHANG)) > 0) {
        printf("child %d terminated\n", pid);
    }
}


/*********************************************************************
    Function:
    Description:
        捕捉所有的信号，主要用于类似死机的BUG调试
    Calls:
      Called By:
    parameter:
      Return:
      author:bruce
            capture SIGKILL SIGINT and SIGTERM to protect disk
********************************************************************/
void capture_all_signal()
{
    int i = 0;
    for (i = 0; i < 32; i ++) {
        if (i == SIGCHLD) {
            //signal(i, SIG_DFL);
            signal(i, sig_chld);
        } else if ( (i == SIGPIPE)|| (i == SIGALRM)) {
            signal(i, SIG_IGN);
        } else {
            signal(i, signal_hander);
        }
    }
}


/*************************************************************
* 函数介绍： 创建线程，并设置成分离属性
* 输入参数：
* 输出参数：
* 返回值  ：
*************************************************************/
int CreateDetachThread(ThreadEntryPtrType entry, void *para, pthread_t *pid)
{
    pthread_t ThreadId;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);//绑定
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);//分离
    if(pthread_create(&ThreadId, &attr, entry, para) == 0) { //创建线程
        pthread_attr_destroy(&attr);
		if(pid)
			*pid = ThreadId;
		return 0;
    }
    pthread_attr_destroy(&attr);
    return -1;
}

/*************************************************************
* 函数介绍： 创建线程
* 输入参数：
* 输出参数：
* 返回值  ：
*************************************************************/
int CreateThread(ThreadEntryPtrType entry, void *para, pthread_t *pid)
{
    pthread_t ThreadId;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);//绑定
    //pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);//分离
    if (pthread_create(&ThreadId, &attr, entry, para) == 0) { //创建线程
        pthread_attr_destroy(&attr);
		if(pid)
			*pid = ThreadId;
		return 0;
    }
    pthread_attr_destroy(&attr);
    return -1;
}



void * sub_thread_entry(void * param)
{
    ThreadArgs *pArgs = (ThreadArgs *)param;

    void *ret = pArgs->callback(pArgs->arg);

    free(pArgs->arg);
    free(pArgs);

    return ret;
}


int CreateDetachThread2(ThreadEntryPtrType fuc_call, void *arg1, int arg1_size, pthread_t *pid)
{
    /* push args into buffer */
    ThreadArgs *args = (ThreadArgs *)malloc(sizeof(ThreadArgs));
    if (NULL == args)
        return -1;
    args->callback = fuc_call;
    args->arg_size = arg1_size;
    args->arg = malloc(arg1_size);
    if (NULL == args->arg)
        return -1;
    memcpy(args->arg, arg1, arg1_size);

    pthread_t ThreadId;
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);//绑定
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);//分离
    if(pthread_create(&ThreadId, &attr, sub_thread_entry, args) == 0) { //创建线程
        pthread_attr_destroy(&attr);
		if(pid)
			*pid = ThreadId;
		return 0;
    }
    pthread_attr_destroy(&attr);

    return -1;
}


