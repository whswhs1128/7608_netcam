/*  
 * posixWrapper.c - A wrapper for POSIX thread, message queue, semaphore library.
 *  
 *
 *
 *
 * 
 *
 */

#include <limits.h>
#include <sched.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/time.h>
#include <signal.h>
#include <pthread.h>
#include <mqueue.h>
#include <time.h>
#include <semaphore.h>
#include <sys/types.h>
#include "pwrapper.h"
#include "hik_netfun.h"


//#define DEBUG2

/*
 * POSIX thread library wrapper
 */

/*
 * Function: funcWrapper2
 * Description: thread entry point.
 * Input:   arg - point to FUNC_WRAPPER2 structure
 * Output:  none 
 * Return:  NULL
 *
 */
static void *threadWrapper2(void *arg)
{
	FUNC_WRAPPER2 func = *((FUNC_WRAPPER2 *)arg);	
	free((char *)arg);

	(*(func.entry))(func.arg[0], func.arg[1], func.arg[2], func.arg[3], func.arg[4],
			func.arg[5], func.arg[6], func.arg[7], func.arg[8], func.arg[9]);

	return NULL;
}

/*
 * Function: funcWrapper
 * Description: thread entry point.
 * Input:   arg - point to FUNC_WRAPPER structure
 * Output:  none 
 * Return:  NULL
 *
 */
static void *threadWrapper(void *arg)
{
	FUNC_WRAPPER func = *((FUNC_WRAPPER *)arg);	
	free((char *)arg);

	(*(func.entry))(func.parms[0], func.parms[1], func.parms[2], func.parms[3], func.parms[4],
			func.parms[5], func.parms[6], func.parms[7], func.parms[8], func.parms[9]);

	return NULL;
}
#if 1
/*
 * Function: getTimespec
 * Description: Get the time spec struct.
 * Input:   wait_ms - NO_WAIT, or max wait time(ms)
 * Output:  tp - time spec struct
 * Return:  none
 *
 */
static inline void getTimespec(int wait_ms, struct timespec *tp)
{
	time_t sec, t;
	long long nsec;

	sec = 0;
	if (wait_ms == NO_WAIT)
	{
		nsec = 0;
	}
	else
	{
		nsec = wait_ms * 1000000LL;
	}

	if (clock_gettime(CLOCK_REALTIME, tp) == -1)
	{
		HPRINT_INFO("getTimespec: clock_gettime call fail, error %d(%s)\n", errno, strerror(errno));
		tp->tv_sec = time(NULL) + 1;
		tp->tv_nsec = 0;
	}
	else
	{
		t = time(NULL) + 1;
		if ((int)(tp->tv_sec - t) > 30) 
		{
			tp->tv_sec = t;
			tp->tv_nsec = 0;
		}
	}

	nsec += tp->tv_nsec;
	HPRINT_INFO("getTimespec: current time sec = %ld, time = %ld, nsec = %ld, total nsec = %lld\n", 
						tp->tv_sec, time(NULL)+1, tp->tv_nsec, nsec);
	if (nsec >= 1000000000)
	{
		sec = nsec / 1000000000;
		nsec = nsec % 1000000000;
	}
	tp->tv_sec += sec;
	tp->tv_nsec = nsec;
	HPRINT_INFO("getTimespec: after time sec = %ld, time = %ld, nsec = %ld\n", 
						tp->tv_sec, time(NULL)+1, tp->tv_nsec);

	return;
}

/*
 * Function: getTimespec2
 * Description: Get the time spec struct.
 * Input:   wait_ms - NO_WAIT, or max wait time(ms)
 * Output:  tp - time spec struct
 * Return:  none
 *
 */
static inline void getTimespec2(int wait_ms, struct timespec *tp)
{
	time_t sec, t;
	long long nsec;

	sec = 0;
	if (wait_ms == NO_WAIT)
	{
		nsec = 0;
	}
	else
	{
		nsec = wait_ms * 1000000LL;
	}

	if (clock_gettime(CLOCK_REALTIME, tp) == -1)
	{
		fprintf(stderr, "getTimespec: clock_gettime call fail, error %d(%s)\n", errno, strerror(errno));
		tp->tv_sec = time(NULL) + 1;
		tp->tv_nsec = 0;
	}
	else
	{
		t = time(NULL) + 1;
		if ((int)(tp->tv_sec - t) > 30) 
		{
			tp->tv_sec = t;
			tp->tv_nsec = 0;
		}
	}

	nsec += tp->tv_nsec;
	fprintf(stderr, "getTimespec: current time sec = %ld, time = %ld, nsec = %ld, total nsec = %lld\n", 
			tp->tv_sec, time(NULL)+1, tp->tv_nsec, nsec);
	if (nsec >= 1000000000)
	{
		sec = nsec / 1000000000;
		nsec = nsec % 1000000000;
	}
	tp->tv_sec += sec;
	tp->tv_nsec = nsec;
	fprintf(stderr, "getTimespec: after time sec = %ld, time = %ld, nsec = %ld\n", 
			tp->tv_sec, time(NULL)+1, tp->tv_nsec);

	return;
}

/*
 * Function: pthreadGetPriorityScope
 * Description: 
 * Input:   none
 * Output:  minPriority - if it is not NULL, return the minmum priority 
 *			maxPriority - if it is not NULL, return the maxmum priority 
 * Return:  0 if successful, otherwise return -1
 *
 */
int pthreadGetPriorityScope(int *minPriority, int *maxPriority)
{
	/* get the allowable priority range for the scheduling policy */
	if (minPriority != NULL)
	{
		(*minPriority) = sched_get_priority_min(SCHED_RR);
		if (*minPriority == -1)
		{
			return -1;
		}
	}
	if (maxPriority != NULL)
	{
		(*maxPriority) = sched_get_priority_max(SCHED_RR);
		if (*maxPriority == -1)
		{
			return -1;
		}
	}
	HPRINT_INFO("priority: min = %d, max = %d\n", *minPriority, *maxPriority);

	return 0;
}
#endif
/*
 * Function: setPthreadAttr
 * Description: set the pthread's attribute: priority and the stack size in details
 * Input:	priority - [minPriority, maxPriority] 
 *			stacksize - the pthread's stack size
 * Output:  attr - the pthread's attribute
 * Return:  0 if successful, otherwise return -1
 *
 */
static int setPthreadAttr(pthread_attr_t *attr, int priority, size_t stacksize)
{
	int rval;
	struct sched_param	params;
	int maxPriority, minPriority;

	rval = pthread_attr_init(attr);
	if (rval != 0)
	{
		return rval;
	}

	/* normally, need not to set */
#if 1 
	rval = pthread_attr_setinheritsched(attr, PTHREAD_EXPLICIT_SCHED);
	if (rval != 0)
	{
		pthread_attr_destroy(attr);
		return rval;
	}

	rval = pthread_attr_setscope(attr, PTHREAD_SCOPE_SYSTEM);
	if (rval != 0)
	{
		if (rval == ENOTSUP)
		{
			HPRINT_INFO("The system does not support the %s scope, using %s\n",
							"PTHREAD_SCOPE_SYSTEM", "PTHREAD_SCOPE_PROCESS");

			rval = pthread_attr_setscope(attr, PTHREAD_SCOPE_PROCESS);
		}

		if (rval)
		{
			pthread_attr_destroy(attr);
			return rval;
		}
	}
#endif

	/* use the round robin scheduling algorithm */
	rval = pthread_attr_setschedpolicy(attr, SCHED_RR);
	if (rval != 0)
	{
		pthread_attr_destroy(attr);
		return rval;
	}

	/* set the thread to be detached */
	rval = pthread_attr_setdetachstate(attr, PTHREAD_CREATE_DETACHED);
	if (rval != 0)
	{
		pthread_attr_destroy(attr);
		return rval;
	}

	/* first get the scheduling parameter, then set the new priority */
	rval = pthread_attr_getschedparam(attr, &params);
	if (rval != 0)
	{
		pthread_attr_destroy(attr);
		return rval;
	}

	rval = pthreadGetPriorityScope(&minPriority, &maxPriority);
	if (rval != 0)
	{
		pthread_attr_destroy(attr);
		return rval;
	}
	if (priority < minPriority)
	{
		priority = minPriority;
	}
	else if (priority > maxPriority)
	{
		priority = maxPriority;
	}
	params.sched_priority = priority;
	rval = pthread_attr_setschedparam(attr, &params);
	if (rval != 0)
	{
		pthread_attr_destroy(attr);
		return rval;
	}

	/* when set stack size, we define a minmum value to avoid fail */
	if (stacksize < PTHREAD_STACK_MIN)
	{
		stacksize = PTHREAD_STACK_MIN;
	}
	rval = pthread_attr_setstacksize(attr, stacksize);
	if (rval != 0)
	{
		pthread_attr_destroy(attr);
		return rval;
	}

	return 0;
}

/*
 * Function: pthreadSpawn
 * Description: create a pthread, this is a variadic function
 * Input:	priority - [minPriority, maxPriority]
 *          stacksize - new thread's stack size(byte)
 *          funcptr - function addr to start the new thread
 *          args - total optional arguments pass to the above function
 *          ... - optional arguments
 * Output:  ptid - pthread id
 * Return:  0 if successful, otherwise an error number returned
 *
 */
int pthreadSpawn(pthread_t *ptid, int priority, size_t stacksize,
		void *(*funcptr)(void*, void*, void*, void*, void*, void*, void*, void*, void*, void*), 
		unsigned args, ...)
{
	if (funcptr == NULL || args > 10)
	{
		return EINVAL;
	}

	int i, rval;
	void *arg[] = {[0 ... 9] = NULL};
	START_ROUTINE start;
	pthread_t tid, *raw;
	pthread_attr_t attr;
	va_list ap;

	va_start(ap, args);
	for (i = 0; i < args; i++)
	{
		arg[i] = va_arg(ap, void *);
		HPRINT_INFO("pthreadSpawn: arg[%d] = %d\n", i, (int)arg[i]);
	}
	va_end(ap);

	rval = setPthreadAttr(&attr, priority, stacksize);
	if (rval != 0)	
	{
		return rval;
	}

	if (ptid != NULL)
	{
		raw = ptid;
	}
	else
	{
		raw = &tid;
	}

	/* 
	 * If the total optional argumens is 0 or 1, we call the pthread_create directly. 
	 * Otherwise, we wrap a start routine.
	 */
	if (args <= 1)
	{
		rval = pthread_create(raw, &attr, (START_ROUTINE)funcptr, arg[0]);
	}
	else
	{
		FUNC_WRAPPER2 *func;
		func = (FUNC_WRAPPER2 *)malloc(sizeof(FUNC_WRAPPER2));
		if (func == NULL)
		{
			(void) pthread_attr_destroy(&attr);
			return ENOMEM;
		}

		start = threadWrapper2;
		memset((char *)func, 0, sizeof(FUNC_WRAPPER2));
		func->entry = funcptr;
		for (i = 0; i < args; i++)
		{
			func->arg[i] = arg[i];
		}
		rval = pthread_create(raw, &attr, start, (void *)func);
		if (rval != 0)
		{
			free((char *)func);
		}
	}

	pthread_attr_destroy(&attr);
	HPRINT_INFO("pthread create: tid = %d, priority = %d, stacksize = %d\n", (int)*raw, priority, stacksize);

	return rval;
}

/*
 * Function: pthreadCreate
 * Description: create a pthread,with often used control config together.
 * Input:	priority - [minPriority, maxPriority]
 *          stacksize - new thread's stack size(byte)
 *          funcptr - function addr to start the new thread
 *          arg - arguments to func
 * Output:  ptid - pthread id
 * Return:  0 if successful, otherwise an error number returned
 *
 */
int pthreadCreate(pthread_t	*ptid,
		int		priority,
		size_t 	stacksize,
		int (*funcptr)(int,int,int,int,int,int,int,int,int,int),
		int arg0, int arg1, int arg2, int arg3, int arg4,
		int arg5, int arg6, int arg7, int arg8, int arg9)
{
	if (funcptr == NULL)
	{
		return EINVAL;
	}

	int rval;
	pthread_t tid, *raw;
	pthread_attr_t attr;

	rval = setPthreadAttr(&attr, priority, stacksize);
	if (rval != 0)	
	{
		return rval;
	}

	/* We just to simulate the arguments pass to the vxworks's spawn task. */
	FUNC_WRAPPER *func = NULL;
	func = (FUNC_WRAPPER *)malloc(sizeof(FUNC_WRAPPER));
	if (func == NULL)
	{
		(void) pthread_attr_destroy(&attr);
		return ENOMEM;
	}

	func->entry = funcptr;
	func->parms[0] = arg0;
	func->parms[1] = arg1;
	func->parms[2] = arg2;
	func->parms[3] = arg3;
	func->parms[4] = arg4;
	func->parms[5] = arg5;
	func->parms[6] = arg6;
	func->parms[7] = arg7;
	func->parms[8] = arg8;
	func->parms[9] = arg9;

	if (ptid != NULL)
	{
		raw = ptid;
	}
	else
	{
		raw = &tid;
	}

	rval = pthread_create(raw, &attr, threadWrapper, (void *)func);
	if (rval != 0)
	{
		free((char *)func);
	}

	pthread_attr_destroy(&attr);
	HPRINT_INFO("pthread create: tid = %d, priority = %d, stacksize = %d\n", (int)*raw, priority, stacksize);

	return rval;
}

/*
 * Function: pthreadSelf
 * Description: shall return the thread ID of the calling thread.
 * Input:   none
 * Output:  none 
 * Return:  shall return the thread ID of the calling thread,
 *			no errors are defined.
 *
 */
pthread_t pthreadSelf()
{
	return pthread_self();
}

/*
 * Function: pthreadIdVerify
 * Description: verify the thread is exist.
 * Input:   tid - pthread id
 * Output:  none 
 * Return:  0 indicate the thread is found, -1 indicate isn't found
 *
 */
int pthreadIdVerify(pthread_t tid)
{
	int rval;

	rval = pthread_kill(tid, 0);
	if (rval == 0)
	{
		return 0;
	}
	else
	{
		return -1;
	}
}

/*
 * Function: pthreadSuspend
 * Description: suspend the thread.
 * Input:   tid - pthread id
 * Output:  none 
 * Return:  0 if successful, otherwise return -1
 *
 */
int pthreadSuspend(pthread_t tid)
{
	int rval;

	rval = pthread_kill(tid, SIGSTOP);
	if (rval != 0)
	{
		return -1;
	}
	
	return 0;
}

/*
 * Function: pthreadResume
 * Description: resume the thread.
 * Input:   tid - pthread id
 * Output:  none 
 * Return:  0 if successful, otherwise return -1
 *
 */
int pthreadResume(pthread_t tid)
{
	int rval;

	rval = pthread_kill(tid, SIGCONT);
	if (rval != 0)
	{
		return -1;
	}
	
	return 0;
}

/*
 * Function: pthreadCancel
 * Description: cancel a thread through the pid.
 * Input:   tid - pthread id
 * Output:  none 
 * Return:  0 if successful, otherwise an error number returned
 *
 */
int pthreadCancel(pthread_t tid)
{
	int	rval;

	rval = pthread_cancel(tid);

	return rval;
}

/*
 * Function: pthreadMutexInit
 * Description: initialize a mutex
 * Output:   mutex - point to the pthread mutex
 * Return:  0 if successful, otherwise an error number returned
 *
 */
int pthreadMutexInit(pthread_mutex_t *mutex)
{
	int	rval;

	rval = pthread_mutex_init(mutex, NULL);

	return rval;
}

/*
 * Function: pthreadMutexLock
 * Description: lock a mutex
 * Input:   mutex - point to the pthread mutex, must be initalized first
 *          wait_ms - NO_WAIT,WAIT_FOREVER,or max wait time(ms)
 * Output:  none 
 * Return:  0 if successful, otherwise an error number returned
 *
 */
int pthreadMutexLock(pthread_mutex_t *mutex, int wait_ms)
{
	int rval;
	struct timespec timeout;

	if (wait_ms == NO_WAIT)
	{
		rval = pthread_mutex_trylock(mutex);
	}
	else if (wait_ms == WAIT_FOREVER) 
	{
		rval = pthread_mutex_lock(mutex);
	}
	else
	{
		getTimespec(wait_ms, &timeout);
		rval = pthread_mutex_timedlock(mutex, &timeout);
	}

	return rval;
}

/*
 * Function: pthreadMutexUnlock
 * Description: unlock a mutex
 * Input:   mutex - point to the pthread mutex, must be initalized first
 * Output:  none 
 * Return:  0 if successful, otherwise an error number returned
 *
 */
int pthreadMutexUnlock(pthread_mutex_t *mutex)
{
	int	rval;

	rval = pthread_mutex_unlock(mutex);

	return rval;
}

/*
 * Function: pthreadMutexDestroy
 * Description: destroy a mutex
 * Input:   mutex - point to the pthread mutex
 * Output:  none 
 * Return:  0 if successful, otherwise an error number returned
 *
 */
int pthreadMutexDestroy(pthread_mutex_t *mutex)
{
	int	rval;

	rval = pthread_mutex_destroy(mutex);

	return rval;
}

/*
 * Function: pthreadCondWait
 * Description: block on a condition variable 
 * Input:   cond - condition variable
 *			mutex - point to the pthread mutex, must be initalized first
 *          wait_ms - NO_WAIT,WAIT_FOREVER,or max wait time(ms)
 * Output:  none 
 * Return:  0 if successful, otherwise an error number returned
 *
 */
int pthreadCondWait(pthread_cond_t *cond, pthread_mutex_t *mutex, int wait_ms)
{
	int rval;
	struct timespec timeout;

	if (wait_ms == WAIT_FOREVER) 
	{
		rval = pthread_cond_wait(cond, mutex);
	}
	else
	{
		getTimespec(wait_ms, &timeout);
		rval = pthread_cond_timedwait(cond, mutex, &timeout);
	}

	return rval;
}
#if 0
/*
* POSIX message queue library wrapper
*/

/*
 * Function: mqOpen
 * Description: open a POSIX message queue
 * Input:   name - points to a string naming a message queue
 *          oflag - O_RDONLY,O_WRONLY,RDWR | O_CREAT|O_EXCL|O_NONBLOCK
 * Output:  none 
 * Return:  a message queue descriptor if successful, otherwise return -1 and set errno
 *
 */
mqd_t mqOpen(const char *name, int oflag, va_list args) 
{
	mqd_t rval;

	rval = mq_open(name, oflag, args);

	return rval;
}

/*
 * Function: mqSend
 * Description: send a message to the POSIX message queue
 * Input:   mqdes - the message queue descriptor
 *          msg_ptr - points to the message
 *          msg_len - specifies the length of the message(byte)
 *          wait_ms - NO_WAIT,WAIT_FOREVER,or max wait time(ms)
 *          msg_prio - message priority
 * Output:  none 
 * Return:  0 if successful, otherwise return -1 and set errno
 *
 */
int mqSend(mqd_t mqdes, const char *msg_ptr, size_t msg_len, int wait_ms, unsigned msg_prio)
{
	int rval;
	struct timespec timeout;

	if (wait_ms == WAIT_FOREVER)
	{
		rval = mq_send(mqdes, msg_ptr, msg_len, msg_prio);
	}
	else
	{
		getTimespec(wait_ms, &timeout);
		rval = mq_timedsend(mqdes, msg_ptr, msg_len, msg_prio, &timeout);
	}

	return rval;
}

/*
 * Function: mqReceive
 * Description: receive the oldest of the highest priority message from the POSIX message queue
 * Input:   mqdes - the message queue descriptor
 *          msg_len - length of the buffer(byte)
 *          wait_ms - NO_WAIT,WAIT_FOREVER,or max wait time(ms)
 * Output:  msg_ptr - points to the buffer store the message
 *          msg_prio -  If  the  argument is not NULL, the priority of the selected message 
 *                      shall be stored in the location referenced by msg_prio.
 * Return:  length of the message if successful, otherwise return -1 and set errno
 *
 */
ssize_t mqReceive(mqd_t mqdes, char *msg_ptr, size_t msg_len, int wait_ms, unsigned *msg_prio)
{
	ssize_t rval;
	struct timespec timeout;

	if (wait_ms == WAIT_FOREVER)
	{
		rval = mq_receive(mqdes, msg_ptr, msg_len, msg_prio);
	}
	else
	{
#ifdef DEBUG2
		if (msg_prio == 0xaa55)
		{
			fprintf(stderr, "mqReceive: before getTimespec, wait ms = %d\n", wait_ms);
			getTimespec2(wait_ms, &timeout);
			fprintf(stderr, "mqReceive: wait ms = %d, timeout sec = %ld, nsec = %ld\n", wait_ms, timeout.tv_sec, timeout.tv_nsec);
		}
		else
		{
			getTimespec(wait_ms, &timeout);
		}
		rval = mq_timedreceive(mqdes, msg_ptr, msg_len, NULL, &timeout);
#else
		getTimespec(wait_ms, &timeout);
		rval = mq_timedreceive(mqdes, msg_ptr, msg_len, msg_prio, &timeout);
#endif

#ifdef DEBUG2
		if (msg_prio == 0xaa55)
		{
			fprintf(stderr, "mqReceive: mq recv return %d\n", rval);
			if (-1 == rval)
			{
				fprintf(stderr, "mqReceive: errno = %d(%s)\n", errno, strerror(errno));
			}
		}
#endif
	}

	return rval;
}

/*
 * Function: mqGetattr
 * Description: get attributes of the POSIX message queue 
 * Input:   mqdes - the message queue descriptor
 * Output:  mqstat - points to the mq_attr structure which stores the attributes
 * Return:  0 if successful, otherwise return -1 and set errno
 *
 */
int mqGetattr(mqd_t mqdes, struct mq_attr *mqstat)
{
	int rval;

	rval = mq_getattr(mqdes, mqstat);

	return rval;
}

/*
 * Function: mqSetattr
 * Description: set attributes of the POSIX message queue 
 * Input:   mqdes - the message queue descriptor
 *          mqstat - points to the mq_attr structure which stores the attributes
 * Output:  omqstat - points to the mq_attr structure which return the old attributes
 * Return:  0 if successful, otherwise return -1 and set errno
 *
 */
int mqSetattr(mqd_t mqdes, const struct mq_attr *mqstat, struct mq_attr *omqstat)
{
	int rval;

	rval = mq_setattr(mqdes, mqstat, omqstat);

	return rval;
}

/*
 * Function: mqUnlink
 * Description: remove a POSIX message queue 
 * Input:   name - points to the message queue's pathname
 * Output:  none
 * Return:  0 if successful, otherwise return -1 and set errno
 *
 */
int mqUnlink(const char *name)
{
	int rval;

	rval = mq_unlink(name);

	return rval;
}

/*
 * Function: mqClose
 * Description: remove the association between the message queue descriptor, mqdes, and its
 *              message queue
 * Input: mqdes - the message queue descriptor
 * Output: none
 * Return: 0 if successful, otherwise return -1 and set errno
 *
 */
int mqClose(mqd_t mqdes)
{
	int rval;

	rval = mq_close(mqdes);

	return rval;
}

/*
* POSIX semaphore library wrapper
*/

/*
 * Function: semInit
 * Description: initialize a unnamed semaphore
 * Input:   value -  the value of the initialized semaphore.
 * Output:  sem - semaphore
 * Return:  if fail, return -1, and set errno to indicate the error
 *			if success, return zero.
 *
 */
int semInit(sem_t *sem, unsigned value)
{
	int rval;
	
	/* the semaphore is shared between threads of the process */
	rval = sem_init(sem, 0, value);

	if (rval != -1)
	{
		return 0;
	}

	return rval;
}

/*
 * Function: semOpen
 * Description: initialize and open a named semaphore
 * Input:   name - points to a string naming a semaphore object
 *          oflag - O_CREAT,O_EXCL
 * Output:  none
 * Return:  the address of the semaphore if successful,otherwise 
 *          return a value of SEM_FAILED and set errno to indicate the error
 *
 */
sem_t *semOpen(const char *name, int oflag, ...)
{
	sem_t *rval;
	va_list args;

	va_start(args, oflag);
	rval = sem_open(name, oflag, args);
	va_end(args);

	return rval;
}

/*
 * Function: semPost
 * Description: unlock the semaphore
 * Input:   sem - points to the semaphore
 * Output:  none
 * Return:  0 if successful,otherwise return -1 and set errno
 *
 */
int semPost(sem_t *sem)
{
	int rval;

	rval = sem_post(sem);

	return rval;
}

/*
 * Function: semWait
 * Description: lock the semaphore
 * Input:   sem - points to the semaphore
 *          wait_ms - NO_WAIT,WAIT_FOREVER,or max wait time(ms)
 * Output:  none
 * Return:  0 if successful,otherwise return -1 and set errno
 *
 */
int semWait(sem_t *sem, int wait_ms)
{
	int rval;
	struct timespec timeout;

	if (wait_ms == NO_WAIT)
	{
		rval = sem_trywait(sem);
	}
	else if (wait_ms == WAIT_FOREVER) 
	{
		rval = sem_wait(sem);
	}
	else
	{
		getTimespec(wait_ms, &timeout);
		rval = sem_timedwait(sem, &timeout);
	}

	return rval;
}

/*
 * Function: semUnlink
 * Description: remove the named semaphore
 * Input:   name - points to the semaphore name string
 * Output:  none
 * Return:  0 if successful,otherwise return -1 and set errno
 *
 */
int semUnlink(const char *name)
{
	int rval;

	rval = sem_unlink(name);

	return rval;
}

/*
 * Function: semClose
 * Description: close the named semaphore, shall indicate that the calling process 
 *              is finished using the named semaphore indicated by sem
 * Input:   sem - points to the semaphore
 * Output:  none
 * Return:  0 if successful,otherwise return -1 and set errno
 *
 */
int semClose(sem_t *sem)
{
	int rval;

	rval = sem_close(sem);

	return rval;
}

/*
 * Function: semDestroy
 * Description: destroy an unnamed semaphore
 * Input:   sem - points to the semaphore
 * Output:  none
 * Return:  0 if successful,otherwise return -1 and set errno
 *
 */
int semDestroy(sem_t *sem)
{
	int rval;

	rval = sem_destroy(sem);

	return rval;
}
#endif


