#ifndef __LF_THREAD_POOL_INCLUDE_H
#define __LF_THREAD_POOL_INCLUDE_H

#include <pthread.h>
#include "http_list.h"
#include "static_memory_pool.h"
#include "entry_queue.h"


/**
	call back function for leader followers
*/
typedef void (*PROC_FUNC)(struct list_head* entry, void* arg);


/**
	struct leader followers pool
*/
typedef struct __lf_thread_pool
{
	int thread_num;
	int* running;
	pthread_t* thread_ids;
	pthread_t current_leader;
	
	pthread_mutex_t leader_lock;
	pthread_mutex_t followers_lock;

	void *arg;
	PROC_FUNC process;
	
	struct list_head followers;
	ENTRY_QUEUE_HEAD entry_queue;
	STATIC_MEM_POOL* followers_pool;
}LF_THREAD_POOL;


/**
	create a leader followers pool
*/
extern LF_THREAD_POOL* lf_thread_pool_creat(int threadnum, PROC_FUNC proc, 
	void* arg, int* running);

/**
	free a leader followers pool
*/
extern void lf_thread_pool_free(LF_THREAD_POOL* pool);

/**
	push a entry to leader followers pool
*/
extern void push_entry_in_pool(struct list_head* entry, LF_THREAD_POOL* pool);


#endif //__LF_THREAD_POOL_INCLUDE_H

