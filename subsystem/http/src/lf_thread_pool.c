
#include "http_const.h"
#include "lf_thread_pool.h"
#include "entry_queue.c"
#include <sys/prctl.h>


#define SET_THREAD_NAME() \
    prctl(PR_SET_NAME, (unsigned long)__func__, 0,0,0)


/**
	struct followers
*/
typedef struct __lf_follower_st
{
	pthread_t owner;
	pthread_mutex_t* pleader_lock;
	pthread_cond_t  thread_cond;

	struct list_head plst;
}LF_FOLLOWER_ST;

/**
	follower operator
*/
static void push_follower(LF_FOLLOWER_ST* fw, LF_THREAD_POOL* pool)
{
	pthread_mutex_lock(&pool->followers_lock);
	list_add_tail(&fw->plst, &pool->followers);
	pthread_mutex_unlock(&pool->followers_lock);
}


static LF_FOLLOWER_ST* make_follower(LF_THREAD_POOL* pool, 
	pthread_t owner, pthread_mutex_t* pleader_lock)
{
	LF_FOLLOWER_ST* fw = 0;

	fw = mem_allocate(pool->followers_pool, sizeof(LF_FOLLOWER_ST));
	//fw = (LF_FOLLOWER_ST*)malloc(sizeof(LF_FOLLOWER_ST));
	if (!fw)
		return 0;

	fw->owner = owner;
	fw->pleader_lock = pleader_lock;
	
	if (pthread_cond_init(&fw->thread_cond, 0))
		return 0;

	fw->plst.prev = 0;
	fw->plst.next = 0;

	push_follower(fw, pool);
    return fw;
}


static LF_FOLLOWER_ST* pop_follower(LF_THREAD_POOL* pool)
{
	LF_FOLLOWER_ST* fw = 0;

	pthread_mutex_lock(&pool->followers_lock);
	
	fw = list_first_entry(&pool->followers, LF_FOLLOWER_ST, plst);
	if (fw)
		list_del(&fw->plst);

	pthread_mutex_unlock(&pool->followers_lock);

	return fw;
}

static int follower_empty(LF_THREAD_POOL* pool)
{
	int ret = 0;
	
	pthread_mutex_lock(&pool->followers_lock);
	ret = list_empty(&pool->followers);
	pthread_mutex_unlock(&pool->followers_lock);
	
	return ret;
}

static inline void wait(LF_FOLLOWER_ST* fw)
{
	pthread_cond_wait(&fw->thread_cond, fw->pleader_lock);
}

static inline void signal(LF_FOLLOWER_ST* fw)
{
	if (fw)
		pthread_cond_signal(&fw->thread_cond);
}


/**
	leader operator
*/
static int lf_thread_done(LF_THREAD_POOL* pool)
{
	return *pool->running;
}

static inline void set_leader(LF_THREAD_POOL* pool, pthread_t leader)
{
	pool->current_leader = leader;
}

static inline int leader_active(LF_THREAD_POOL* pool)
{
    return pool->current_leader;
}

static inline int is_leader(LF_THREAD_POOL* pool)
{
    return pool->current_leader == pthread_self() ? 1 : 0;
}


static void become_leader(LF_THREAD_POOL* pool)
{
	LF_FOLLOWER_ST* fw;

	pthread_mutex_lock(&pool->leader_lock);
    if (leader_active(pool) == 0)
    {
        set_leader(pool, pthread_self());
    }
    else if (!is_leader(pool))
    {
    	//make a follower and in list
        fw = make_follower(pool, pthread_self(), &pool->leader_lock);
		//wait for active
        while (!is_leader(pool))
            wait(fw);

		//become leader
        //free(fw);
        mem_deallocate(pool->followers_pool, fw);
        fw = 0;
    }

    pthread_mutex_unlock(&pool->leader_lock);
}

static void elect_new_leader(LF_THREAD_POOL* pool)
{
	LF_FOLLOWER_ST* fw;
	
	pthread_mutex_lock(&pool->leader_lock);
    if (!follower_empty(pool))
    {
    	fw = pop_follower(pool);
		//set follower to leader
		set_leader(pool, fw->owner);
		//Wake up a follower.
		signal(fw);
        goto unlock;
    }

	set_leader(pool, 0);

unlock:
	pthread_mutex_unlock(&pool->leader_lock);
}


static void* lf_thread_svc(void *arg)
{
	struct list_head* entry = 0;
	LF_THREAD_POOL* pool = (LF_THREAD_POOL*)arg;

	SET_THREAD_NAME();
	PRINT_INFO(("[ThreadID 0x%x] leader follower thread start!\n", (unsigned int)pthread_self()));
	while (lf_thread_done(pool))
	{
		//Block until this thread is the leader.
        become_leader(pool);
		
        //Get a message, elect new leader, then process message.
        entry = pop_entry(&pool->entry_queue, pool->running);
		
		//signal other follower
		elect_new_leader(pool);
        if (!entry)
            continue;

		//process
		if (pool->process)
        	pool->process(entry, pool->arg);
	}

	
	PRINT_INFO(("[ThreadID 0x%x] leader follower thread exit!\n", (unsigned int)pthread_self()));
	return 0;
}


/**
	export operator
*/
LF_THREAD_POOL* lf_thread_pool_creat(int threadnum, PROC_FUNC proc, 
	void* arg, int* running)
{
	int i;
	LF_THREAD_POOL* pool;

	pool = (LF_THREAD_POOL*)malloc(sizeof(LF_THREAD_POOL));
	if (!pool)
	{
		PRINT_ERROR(("Fail to alloc space for thread pool! [lf_thread_pool_creat]\n"));
		goto error1;
	}

	memset(pool, 0, sizeof(LF_THREAD_POOL));
	pool->thread_ids = (pthread_t*)malloc(threadnum * sizeof(pthread_t));
	if (!pool->thread_ids)
	{
		PRINT_ERROR(("Fail to alloc space for thread ids! [lf_thread_pool_creat]\n"));
		goto error2;
	}

	memset(pool->thread_ids, 0, threadnum * sizeof(pthread_t));
	pool->followers_pool = create_static_mem_pool(threadnum, sizeof(LF_FOLLOWER_ST));
	if (!pool->followers_pool)
	{
		PRINT_ERROR(("Fail to alloc space for follwers! [lf_thread_pool_creat]\n"));
		goto error3;
	}

	if (pthread_mutex_init(&pool->leader_lock, 0))
	{
		PRINT_ERROR(("leader mutex init error[lf_thread_pool_creat]! errno[%d] errinfo[%s]", 
			errno, strerror(errno)));
		goto error3;
	}

	if (pthread_mutex_init(&pool->followers_lock, 0))
	{
		PRINT_ERROR(("followers mutex init error[lf_thread_pool_creat]! errno[%d] errinfo[%s]", 
			errno, strerror(errno)));
		goto error3;
	}

	pool->thread_num        = threadnum;
	pool->current_leader    = 0;
	pool->process           = proc;
	pool->arg               = arg;
	pool->running           = running;

	INIT_LIST_HEAD(&pool->followers);
	if (INIT_ENTRY_QUEUE(&pool->entry_queue))
		goto error3;
	
	for (i = 0; i < threadnum; i++)
	{
		if (pthread_create(&pool->thread_ids[i], 0, lf_thread_svc, pool))
		{
			PRINT_ERROR(("create lf thread[%d] error[lf_thread_pool_creat]! errno[%d] errinfo[%s]", 
				i, errno, strerror(errno)));
			goto error3;
		}
	}

	return pool;

error3:
	FREE_SPACE(pool->thread_ids);
error2:
	FREE_SPACE(pool);
error1:
	return 0;
}

void lf_thread_pool_free(LF_THREAD_POOL* pool)
{
	int i;

	if (!pool)
		return;

	ENTRY_QUEUE_HEAD* head = &pool->entry_queue;
	pthread_cond_signal(&head->cond);

/*	for (i = 0; i < pool->thread_num; i++)
	{
		pthread_join(pool->thread_ids[i], 0);
	}*/
	
	pthread_mutex_destroy(&pool->leader_lock);
	pthread_mutex_destroy(&pool->followers_lock);
	FREE_ENTRY_QUEUE(&pool->entry_queue);
	free_static_mem_pool(pool->followers_pool);
	FREE_SPACE(pool->thread_ids);
	FREE_SPACE(pool);
}

void push_entry_in_pool(struct list_head* entry, LF_THREAD_POOL* pool)
{
	ENTRY_QUEUE_HEAD* head = &pool->entry_queue;
	
	pthread_mutex_lock(&head->mutex);
	list_add_tail(entry, &head->queue);
	pthread_mutex_unlock(&head->mutex);
	
	if (!list_empty(&head->queue))
		pthread_cond_signal(&head->cond);
}


