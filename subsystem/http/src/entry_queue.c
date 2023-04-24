#include "http_const.h"
#include "lf_thread_pool.h"

static struct list_head* pop_entry(ENTRY_QUEUE_HEAD* head, int* running)
{
	struct list_head* entry = 0;

	pthread_mutex_lock(&head->mutex);
	while(*running && list_empty(&head->queue))
		pthread_cond_wait(&head->cond, &head->mutex);

	if (!*running)
		goto out;

	entry = head->queue.next;
	list_del(entry);

out:
	pthread_mutex_unlock(&head->mutex);
	return entry;
}

static int INIT_ENTRY_QUEUE(ENTRY_QUEUE_HEAD* head)
{
	if (pthread_mutex_init(&head->mutex, 0) < 0)
	{
		PRINT_ERROR(("queue mutex init error [INIT_ENTRY_QUEUE]! errno[%d] errinfo[%s]", 
			errno, strerror(errno)));
		return -1;
	}

	if (pthread_cond_init(&head->cond, 0) < 0)
	{
		PRINT_ERROR(("queue cond init error [INIT_ENTRY_QUEUE]! errno[%d] errinfo[%s]", 
			errno, strerror(errno)));
		return -1;
	}

	INIT_LIST_HEAD(&head->queue);
	return 0;
}

static void FREE_ENTRY_QUEUE(ENTRY_QUEUE_HEAD* head)
{
	pthread_mutex_destroy(&head->mutex);
	pthread_cond_destroy(&head->cond);
}


