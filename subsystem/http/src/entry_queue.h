#ifndef __ENTRY_QUEUE_INCLUDE_H
#define __ENTRY_QUEUE_INCLUDE_H


#include "http_list.h"


typedef struct __entry_queue
{
	pthread_mutex_t mutex;
	pthread_cond_t  cond;
	struct list_head queue;
}ENTRY_QUEUE_HEAD;


#endif //__ENTRY_QUEUE_INCLUDE_H

