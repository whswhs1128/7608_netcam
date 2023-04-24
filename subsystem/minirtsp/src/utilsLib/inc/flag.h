#ifndef __UTILS_FLAG_H__
#define __UTILS_FLAG_H__

#include <pthread.h>

typedef struct JFlag {
	int mask;

	pthread_cond_t cond;
	pthread_mutex_t lock;
} JFlag;

JFlag *j_flag_alloc(void);
int j_flag_free(JFlag *f);

int j_flag_set(JFlag *f, int bit);
int j_flag_clr(JFlag *f, int bit);
int j_flag_get(JFlag *f, int bit);
int j_flag_wait(JFlag *f, int mask);

#endif
