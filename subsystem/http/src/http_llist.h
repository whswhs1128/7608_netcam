#ifndef HTTP_LOCK_LIST_H
#define HTTP_LOCK_LIST_H

#include <pthread.h>
#include "http_list.h"

/**
	struct for mutex list head
*/
struct lock_list_head
{
	pthread_mutex_t _mutex;
	pthread_cond_t  _cond;
	struct list_head _mlst;
};

extern int _http_server_init;

extern int init_llist(struct lock_list_head* lohead);
extern void destroy_llist(struct lock_list_head* lohead);
extern inline void llist_add_tail(struct list_head* node, struct lock_list_head* lohead);
extern struct list_head* llist_pop_entry(struct lock_list_head* lohead);

#endif //HTTP_LOCK_LIST_H