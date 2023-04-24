/*!
*****************************************************************************
** \file      $gkprjfdevent.c
**
** \version	$id: fdevent.c 15-08-04  8月:08:1438655347 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#include <sys/epoll.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/resource.h>
//#include "sdk_sys.h"

#include "fdevent.h"

typedef struct JFdNode {
	int fd;
	int (*cb)(int fd, uint32_t event, void *ctx);
	void *ctx;
} JFdNode;

struct JFdEvent {
	int epoll_fd;
	int abort_request;

	pthread_t tid;
	pthread_mutex_t lock;

	int maxfds;
	JFdNode *fdsets;
};

#define valid_file_desc(fd)	(((fd) >= 0) && ((fd) < evbase->maxfds))

int j_fdevent_mod(JFdEvent *evbase, int fd, fdevent_cb cb, void *ctx, uint32_t events)
{
	if (!valid_file_desc(fd))
		return -1;

	pthread_mutex_lock(&evbase->lock);

	JFdNode *n = &evbase->fdsets[fd];
	if (n->fd < 0)
		goto fail;
	memset(n, 0, sizeof(JFdNode));

	n->fd  = fd;
	n->cb  = cb;
	n->ctx = ctx;

	struct epoll_event event = {.data.ptr = n, .events = events};

	if (epoll_ctl(evbase->epoll_fd, EPOLL_CTL_MOD, n->fd, &event) < 0)
		goto fail;

	pthread_mutex_unlock(&evbase->lock);
	return 0;

fail:
	pthread_mutex_unlock(&evbase->lock);
	return -1;
}

int j_fdevent_add(JFdEvent *evbase, int fd, fdevent_cb cb, void *ctx, uint32_t events)
{
	if (!valid_file_desc(fd))
		return -1;

	pthread_mutex_lock(&evbase->lock);

	JFdNode *n = &evbase->fdsets[fd];
	if (n->fd >= 0)
		goto fail;
	memset(n, 0, sizeof(JFdNode));

	n->fd  = fd;
	n->cb  = cb;
	n->ctx = ctx;

	struct epoll_event event = {.data.ptr = n, .events = events};

	if (epoll_ctl(evbase->epoll_fd, EPOLL_CTL_ADD, fd, &event) < 0)
		goto fail;

	pthread_mutex_unlock(&evbase->lock);
	return 0;

fail:
	pthread_mutex_unlock(&evbase->lock);
	return -1;
}

int j_fdevent_del(JFdEvent *evbase, int fd)
{
	if (!valid_file_desc(fd))
		return -1;

	pthread_mutex_lock(&evbase->lock);

	JFdNode *n = &evbase->fdsets[fd];
	if (n->fd < 0)
		goto fail;

	/* required a non-NULL pointer in kernel versions before 2.6.9 */
	if (epoll_ctl(evbase->epoll_fd, EPOLL_CTL_DEL, fd, (struct epoll_event *)-1) < 0)
		goto fail;

	n->fd = -1;

	pthread_mutex_unlock(&evbase->lock);
	return 0;

fail:
	pthread_mutex_unlock(&evbase->lock);
	return -1;
}

static void *fdevent_poll_thread(void *arg)
{
    sdk_sys_thread_set_name("fdevent_poll_thread");
	JFdEvent *evbase = arg;
	if (!evbase)
		return NULL;

	struct epoll_event *events = calloc(evbase->maxfds, sizeof(struct epoll_event));
	if (!events)
		return NULL;

	while (!evbase->abort_request) {
		int n = epoll_wait(evbase->epoll_fd, events, evbase->maxfds, 1000);
		if (n <= 0) {
			if (n == 0 || errno == EINTR)
				continue;
			break;
		}

		pthread_mutex_lock(&evbase->lock);
		int i;
		for (i = 0; i < n; i++) {
			JFdNode *n = events[i].data.ptr;
			if (!n)
				continue;

			if ((n->fd >= 0) && n->cb)
				n->cb(n->fd, events[i].events, n->ctx);
		}
		pthread_mutex_unlock(&evbase->lock);
	}

	free(events);

	return NULL;
}

/* get the maximum file descriptor number */
static int get_maxfds(void)
{
	struct rlimit rlim;
	if (getrlimit(RLIMIT_NOFILE, &rlim) < 0)
		return 4096;

	return rlim.rlim_max;
}

JFdEvent *j_fdevent_create(void)
{
	JFdEvent *evbase = malloc(sizeof(JFdEvent));
	if (!evbase)
		return NULL;
	memset(evbase, 0, sizeof(JFdEvent));

	evbase->epoll_fd = -1;
	evbase->maxfds = get_maxfds();

	evbase->fdsets = calloc(evbase->maxfds, sizeof(JFdNode));
	if (!evbase->fdsets)
		goto fail;

	int i;
	for (i = 0; i < evbase->maxfds; i++)
		evbase->fdsets[i].fd = -1;

	evbase->epoll_fd = epoll_create(evbase->maxfds);
	if (evbase->epoll_fd < 0)
		goto fail;

	/* recursive mutex lock */
	pthread_mutexattr_t attr;
	pthread_mutexattr_init(&attr);
	pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
	pthread_mutex_init(&evbase->lock, &attr);
	pthread_mutexattr_destroy(&attr);

	if (pthread_create(&evbase->tid, NULL, fdevent_poll_thread, evbase) != 0) {
		pthread_mutex_destroy(&evbase->lock);
		goto fail;
	}

	return evbase;

fail:
	if (evbase) {
		if (evbase->epoll_fd >= 0)
			close(evbase->epoll_fd);
		if (evbase->fdsets)
			free(evbase->fdsets);
		free(evbase);
	}

	return NULL;
}

int j_fdevent_destroy(JFdEvent *evbase)
{
	evbase->abort_request = 1;

	/* wait fdevent_poll_thread to exit */
	pthread_join(evbase->tid, NULL);

	int i;
	for (i = 0; i < evbase->maxfds; i++) {
		if (evbase->fdsets[i].fd >= 0)
			j_fdevent_del(evbase, evbase->fdsets[i].fd);
	}

	close(evbase->epoll_fd);
	evbase->epoll_fd = -1;

	pthread_mutex_destroy(&evbase->lock);

	if (evbase->fdsets)
		free(evbase->fdsets);

	free(evbase);

	return 0;
}
