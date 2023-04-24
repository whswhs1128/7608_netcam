#include "http_const.h"
#include "http_llist.h"

void llist_add_tail(struct list_head* node, struct lock_list_head* lohead)
{
	pthread_mutex_lock(&lohead->_mutex);
	list_add_tail(node, &lohead->_mlst);
	pthread_mutex_unlock(&lohead->_mutex);

	if (!list_empty(&lohead->_mlst))
		pthread_cond_signal(&lohead->_cond);
}

struct list_head* llist_pop_entry(struct lock_list_head* lohead)
{
	struct list_head* node = NULL;

	pthread_mutex_lock(&lohead->_mutex);
	while (_http_server_init && list_empty(&lohead->_mlst))
		pthread_cond_wait(&lohead->_cond, &lohead->_mutex);

	node = lohead->_mlst.next;
	list_del(node);

	pthread_mutex_unlock(&lohead->_mutex);
	return node;
}

int init_llist(struct lock_list_head* lohead)
{
	INIT_LIST_HEAD(&lohead->_mlst);

	if (pthread_mutex_init(&lohead->_mutex, NULL) < 0)
	{
		PRINT_ERROR(("Fail to init mutex[init_list]! errno[%d] errinfo[%s]", 
			errno, strerror(errno)));
		return -1;
	}

	if (pthread_cond_init(&lohead->_cond, NULL) < 0)
	{
		PRINT_ERROR(("Fail to init cond[init_list]! errno[%d] errinfo[%s]", 
			errno, strerror(errno)));
		return -1;
	}
	
	return 0;
}

void destroy_llist(struct lock_list_head* lohead)
{
	pthread_mutex_destroy(&lohead->_mutex);
	pthread_cond_destroy(&lohead->_cond);
}


