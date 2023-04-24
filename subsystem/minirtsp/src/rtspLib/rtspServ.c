/*!
*****************************************************************************
** \file      $gkprjrtspServ.c
**
** \version	$id: rtspServ.c 15-08-04  8æœ?08:1438655168
**
** \brief
**
** \attention   this code is provided as is. goke microelectronics
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <time.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/poll.h>
#include "list.h"
#include "rtspServLib.h"
#include "rtspServ.h"
#include "rtp.h"
#include "handleMethod.h"
#include "utils.h"
#include "netio.h"
#include "parser.h"
#include "base64.h"
//#include "sdk_debug.h"
//#include "sdk_sys.h"

#define SPACE_CHARS " \t\r\n"
// set USE_RTSP_REAPTER_THREAD to 0, it can decrease one thread
#define USE_RTSP_REAPTER_THREAD  0

#if USE_RTSP_REAPTER_THREAD==0
static JList *waitclose = NULL;
#endif

static void get_word_until_chars(char *buf, int buf_size,
		char *sep, char **pp)
{
	char *p;
	char *q;

	p = *pp;
	p += strspn(p, SPACE_CHARS);
	q = buf;
	while (!strchr(sep, *p) && *p != '\0') {
		if ((q - buf) < buf_size - 1)
			*q++ = *p;
		p++;
	}
	if (buf_size > 0)
		*q = '\0';
	*pp = p;
}

uint16_t rtp_get_multicast_addr(rtsp_session_t *sessp, int media_type, char *addr, int addrlen, uint16_t *port, int *ttl)
{
	if (!sessp || !addr || !port)
		return -1;

	int i;
	for (i = 0; i < sessp->ndesc; i++) {
		MediaDesc *desc = &sessp->mdescs[i];

		if (desc->media == media_type && desc->multicast.enable) {
			strncpy(addr, desc->multicast.ipaddr, addrlen);
			*port = desc->multicast.port;
			if (ttl)
				*ttl = desc->multicast.ttl;
			return 0;
		}
	}

	return -1;
}

/**
 * Close RTSP session and free all the associated resource.
 */
int rtsp_closeRtspSession(rtsp_session_t *sessp)
{
	if (!sessp)
		return -1;

	if (sessp->cmdq)
		j_queue_abort(sessp->cmdq);	/* abort first */

	sessp->abort = 1;
	if (sessp->send_data_tid != 0) {
		pthread_join(sessp->send_data_tid, NULL);
		sessp->send_data_tid = 0;
	}
	if (sessp->recv_data_tid != 0) {
		pthread_join(sessp->recv_data_tid, NULL);
		sessp->recv_data_tid = 0;
	}

	int i;
	for (i = 0; i < 2; i++) {
		close(sessp->rtp[i].rtp_sock);
		close(sessp->rtp[i].rtcp_sock);
	}

	if (sessp->is_rtsp) {
		/* rtsp session */
		close(sessp->rtsp_rcv_sock);
		/* XXX: sessp->rtsp_rcv_sock == sessp->rtsp_snd_sock */
	} else {
		/* http session */
		close(sessp->rtsp_rcv_sock);
		close(sessp->rtsp_snd_sock);
	}

	if (sessp->rtsp_snd_lock_flag)
		pthread_mutex_destroy(&sessp->rtsp_snd_lock);

	if (sessp->ops && sessp->ops->close)
		sessp->ops->close(sessp->handle);

	if (sessp->url)
		UrlFree(sessp->url);

	if (sessp->cmdq) {
		void *node;
		while ((node = j_queue_get(sessp->cmdq, 0)))
			free(node);
		j_queue_destroy(sessp->cmdq);
	}

	if (sessp->saved_keyframe)
		free(sessp->saved_keyframe);
    if(sessp->y_buf)
    {
        if(sessp->y_buf->g_y_buffer)
        {
           free(sessp->y_buf->g_y_buffer) ;
        }
        free(sessp->y_buf);
    }
	free(sessp);
	return 0;
}

int rtsp_closeRtspMcastSession(rtsp_session_t *sessp)
{
	int i = 0;

	if (!sessp)
		return -1;

	sessp->abort = 1;

	if (sessp->send_data_tid != 0) {
		pthread_join(sessp->send_data_tid, NULL);
		sessp->send_data_tid = 0;
	}

	if (sessp->recv_data_tid != 0) {
		pthread_join(sessp->recv_data_tid, NULL);
		sessp->recv_data_tid = 0;
	}

	for (i = 0; i < 2; i++) {
		if (sessp->rtp[i].rtp_sock >= 0)
			close(sessp->rtp[i].rtp_sock);
		if (sessp->rtp[i].rtcp_sock >= 0)
			close(sessp->rtp[i].rtcp_sock);
	}

	if (sessp->ops->close)
		sessp->ops->close(sessp->handle);

	if (sessp->url)
		UrlFree(sessp->url);

	if (sessp->saved_keyframe)
		free(sessp->saved_keyframe);

	free(sessp);

	return 0;
}

static char *rtsp_methods[] = {
	"OPTIONS", "DESCRIBE", "SETUP", "TEARDOWN",
	"PLAY", "PAUSE", "GET_PARAMETER", "SET_PARAMETER"
};

int is_rtsp_request(char *buf, int len)
{
	int i;
	int size = sizeof(rtsp_methods) / sizeof(char *);

	for (i = 0; i < size; i++) {
		char *method = rtsp_methods[i];

		char buffer[20];
		strncpy(buffer, buf, len);
		buffer[19] = '\0';

		if (!strncasecmp(buf, method, strlen(method)))
			return 1;
	}

	return 0;
}

static char *http_methods[] = {
	"PUT", "GET", "POST",
};

int is_http_request(char *buf, int len)
{
	int i;
	int size = sizeof(http_methods) / sizeof(char *);

	for (i = 0; i < size; i++) {
		char *method = http_methods[i];

		if (!strncasecmp(buf, method, strlen(method)))
			return 1;
	}

	return 0;
}

static ssize_t rtsp_read_request(int fd, void *buf, size_t count)
{
	int n;
	int len;
	int rtsp_http_flag = 0;

	char *buffer = (char *)buf;
	char *buffer_ptr = buffer;
	char *buffer_end = buffer + count - 1;	/* one byte for null terminator */

	while (1) {
		len = rtsp_read(fd, buffer_ptr, 1);
		if (len <= 0)
			return -1;

		buffer_ptr += len;

		if (buffer_ptr < buffer + 15)
			continue;

		if (rtsp_http_flag || is_rtsp_request(buffer, 15) || is_http_request(buffer, 15)) {
			rtsp_http_flag = 1;	/* for optimize */
			if (!memcmp(buffer_ptr-2, "\n\n", 2) || !memcmp(buffer_ptr-4, "\r\n\r\n", 4)) {
				*(buffer_ptr + 1) = '\0';
				break;	/* get whole http request */
			} else if (buffer_ptr >= buffer_end) {
				/* request too long: cannot do anything */
				return -1;
			} else continue;
		} else {
			char *b64_decoded_buf = (char *)malloc(count);
			if (!b64_decoded_buf)
				return -1;

			while (1) {
				len = rtsp_read(fd, buffer_ptr, buffer_end - buffer_ptr);
				if (len <= 0) {
					free(b64_decoded_buf);
					return -1;
				}

				buffer_ptr += len;
				*(buffer_ptr + 1) = '\0';

				n = av_base64_decode((uint8_t *)b64_decoded_buf, buffer, buffer_ptr - buffer);
				char *ptr = b64_decoded_buf + n;
				if (!memcmp(ptr - 2, "\n\n", 2) || !memcmp(ptr - 4, "\r\n\r\n", 4)) {
					/* copy decoded buffer back into input buffer */
					memcpy(buffer, b64_decoded_buf, n);
					buffer_ptr = buffer + n;
					buffer[n] = '\0';
					break;
				} else if (buffer_ptr >= buffer_end) {
					/* request too long: cannot do anything */
					free(b64_decoded_buf);
					return -1;
				}
			}

			free(b64_decoded_buf);
			break;
		}
	}

	return buffer_ptr - buffer;
}
/*!
******************************************
** Function      @rtsp_read_rtcp
** Description   @first get rtcp_hdr,next read rtcp_hdr->size bite data
** Param[in]     @fd:rtcp socket(is TCP)
** Param[in]     @buf:buffer point address
** Param[in]     @count:is buf size
** Param[out]    @client send data
**
** Return        @
**- # success:recve data length
**- # fail:-1
** Others        @
******************************************
*/
static ssize_t rtsp_read_rtcp(int fd, void *buf, size_t count)
{
	int len;
	int need_length;

	char *buffer = (char *)buf;
	char *buffer_ptr = buffer;

	intlvd_t *rtcp_hdr = (intlvd_t *)buffer;

	len = rtsp_readn(fd, buffer_ptr, sizeof(intlvd_t));
	if (len != sizeof(intlvd_t))
		return -1;

	buffer_ptr += len;

	need_length = ntohs(rtcp_hdr->size);
	if (need_length > count - sizeof(intlvd_t))
		return -1;

	len = rtsp_readn(fd, buffer_ptr, need_length);
	if (len != need_length)
		return -1;

	buffer_ptr += len;

	return buffer_ptr - buffer;
}

static ssize_t rtsp_read_complete(int fd, void *buf, size_t count)
{
	if (fd_wait_readable(fd, 5000) <= 0)
		return -1;

	char magic;
	if (recv(fd, &magic, 1, MSG_PEEK) <= 0)
		return -1;

	if (magic == '$') {
		return rtsp_read_rtcp(fd, buf, count);
	} else {
		return rtsp_read_request(fd, buf, count);
	}
}

static void *rtsp_connect_thread(void *arg)
{
    sdk_sys_thread_set_name("rtsp_connect_thread");
	rtsp_session_t *sessp = (rtsp_session_t *)arg;

	if (!sessp)
		return NULL;

    RTSP_INFO("in rtsp_connect_thread, recv_sock=%d snd_sock:%d, rtp[0].sock=%d, rtp[1].sock=%d\n", 
        sessp->rtsp_rcv_sock, sessp->rtsp_snd_sock,
        sessp->rtp[0].rtcp_sock, sessp->rtp[1].rtcp_sock);


	struct pollfd fds[8];

	while (!sessp->abort) {
		int numfds = 0;

		fds[numfds].fd = sessp->rtsp_rcv_sock;
		fds[numfds].events = POLLIN;
		numfds++;

		int i;
		for (i = 0; i < 2; i++) {
			if (sessp->rtp[i].rtcp_sock != -1) {
				fds[numfds].fd = sessp->rtp[i].rtcp_sock;
				fds[numfds].events = POLLIN;
				numfds++;
			}
		}

		int ret = poll(fds, numfds, 1000);
		if (ret <= 0) {
			if (ret < 0 && errno != EINTR)
				goto fail;
			continue;
		}

		sessp->timeout = 0;	/* reset timeout */

		for (i = 0; i < numfds; i++) {
			if (fds[i].fd == sessp->rtsp_rcv_sock) {
				if (fds[i].revents & POLLIN) {
					ssize_t n = rtsp_read_complete(sessp->rtsp_rcv_sock,
							sessp->recv_buf, sizeof(sessp->recv_buf) - 1);
					if (n < 0) {
						RTSP_ERR("Read RTSP message error[rtsp socket = %d]\n",
								sessp->rtsp_rcv_sock);
						goto fail;
					}
					sessp->recv_buf[n] = '\0';
                    //RTSP_INFO("rtsp recv -- n=%d, s:\n%s\n", n, sessp->recv_buf);
					if (sessp->recv_buf[0] == '$') {
						/* RTCP over TCP: just drop it for now */
					} else {
						if (rtsp_parse_request(sessp) < 0)
							goto fail;
					}
				}
			} else {
				int j;
				for (j = 0; j < 2; j++) {
					if (fds[i].fd == sessp->rtp[j].rtcp_sock) {
						if (fds[i].revents & POLLIN) {
							/* RTCP over UDP: read and drop */
							char buffer[1024];
							read(sessp->rtp[j].rtcp_sock, buffer, sizeof(buffer));
						}
					}
				}
			}
		}
	}

fail:
	sessp->abort = 1;	/* abort this session */
    RTSP_ERR("rtsp connect thread end...\n");
	return NULL;
}

static rtsp_session_t *rtsp_session_new(int is_rtsp)
{
	rtsp_session_t *sessp = malloc(sizeof(rtsp_session_t));
	if (!sessp)
		return NULL;
	memset(sessp, 0, sizeof(rtsp_session_t));

	sessp->y_buf= malloc(sizeof(y_test_buf));
    if(!sessp->y_buf)
    {
        free(sessp );
        return NULL;
    }
    sessp->y_buf->g_y_buffer_total = 50*1024;

    sessp->y_buf->g_y_buffer = malloc(sessp->y_buf->g_y_buffer_total);
    if(!sessp->y_buf->g_y_buffer)
    {
        free(sessp->y_buf);
        free(sessp );

        return NULL;
    }
	sessp->is_rtsp = is_rtsp;
	sessp->rtsp_rcv_sock = -1;
	sessp->rtsp_snd_sock = -1;
	sessp->max_payload_size = RTP_MAX_PAYLOAD_SIZE;

	/* replay specified */
	sessp->scale = 1.0;
	sessp->max_interval = 5000;	/* 5 seconds */

	int i;
	for (i = 0; i < 2; i++) {
		sessp->rtp[i].rtp_sock = -1;
		sessp->rtp[i].rtcp_sock = -1;
	}

	sessp->state = RTSP_STATE_INITED;

	return sessp;
}

static int rtsp_new_connection(int server_fd)
{
	struct sockaddr_in peeraddr;
	socklen_t addrlen = sizeof(struct sockaddr_in);

	int connfd = accept(server_fd, (struct sockaddr *)&peeraddr, &addrlen);
	if (connfd < 0) {
		perror("Accept connection error");
		return -1;
	}

	if (socket_set_nonblock(connfd, 0) < 0) {
		close(connfd);
		return -1;
	}

	/* init as rtsp connection */
	rtsp_session_t *sessp = rtsp_session_new(1);
	if (!sessp) {
		close(connfd);
		return -1;
	}
	//milo: set no delay
    int on = 1;
    setsockopt(connfd, IPPROTO_TCP, TCP_NODELAY, (void *)&on, sizeof(on));
	//milo: enlarge send buffer size
    int  size = 204800;
    setsockopt(connfd, SOL_SOCKET, SO_SNDBUF, (char *)&size, sizeof(size));
    struct timeval timeo = { 5, 0};
    setsockopt(connfd, SOL_SOCKET, SO_SNDTIMEO,(void *) &timeo, sizeof(timeo));
    setsockopt(connfd, SOL_SOCKET, SO_SNDTIMEO,(void *) &timeo, sizeof(timeo));

	sessp->rtsp_rcv_sock = connfd;
	sessp->rtsp_snd_sock = connfd;
	pthread_mutex_init(&sessp->rtsp_snd_lock, NULL);

	/* Create new thread to receiving RTCP & RTSP messages. */
	if (pthread_create(&sessp->recv_data_tid, NULL, rtsp_connect_thread, sessp)!= 0) {
		pthread_mutex_destroy(&sessp->rtsp_snd_lock);
		close(connfd);
		free(sessp);
		return -1;
	}

	pthread_mutex_lock(&rtsp_serv.sess_list_lock);
	j_list_append(rtsp_serv.allsessions, sessp);
	pthread_mutex_unlock(&rtsp_serv.sess_list_lock);

	RTSP_INFO("Accept new RTSP client from [%s:%d], connfd=%d\n",
			inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port), connfd);

	return 0;
}

static char *response =
	"HTTP/1.0 200 OK\r\n"
	"Date: %s GMT\r\n"
	"Cache-Control: no-cache\r\n"
	"Pragma: no-cache\r\n"
	"Content-Type: application/x-rtsp-tunnelled\r\n"
	"\r\n";

static int get_gmt_time(char *buf, int size)
{
	/* output GMT time */
	time_t ti = time(NULL);
	struct tm tm;

	gmtime_r(&ti, &tm);

	if (strftime(buf, size, "%a, %d %b %Y %H:%M:%S", &tm) == 0)
		return -1;

	return 0;
}

static int http_handle_GET(int connfd)
{
	rtsp_session_t *sessp = rtsp_session_new(0);
	if (!sessp)
		return -1;

	sessp->rtsp_snd_sock = connfd;

	char datetime[128];
	get_gmt_time(datetime, sizeof(datetime));

	char buf[2048];
	snprintf(buf, sizeof(buf), response, datetime);

	if (rtsp_writen(connfd, buf, strlen(buf)) != strlen(buf)) {
		free(sessp);
		return -1;
	}

	pthread_mutex_init(&sessp->rtsp_snd_lock, NULL);

	pthread_mutex_lock(&rtsp_serv.sess_list_lock);
	j_list_append(rtsp_serv.allsessions, sessp);
	pthread_mutex_unlock(&rtsp_serv.sess_list_lock);

	return 0;
}

static int http_handle_POST(int connfd)
{
	struct sockaddr_in peeraddr;
	socklen_t addrlen = sizeof(peeraddr);
	if (getpeername(connfd, (struct sockaddr *)&peeraddr, &addrlen) < 0)
		return -1;

	JListNode *n, *n_next;

	pthread_mutex_lock(&rtsp_serv.sess_list_lock);
	for (n = j_list_first(rtsp_serv.allsessions); n; n = n_next) {
		n_next = j_list_next(rtsp_serv.allsessions, n);

		rtsp_session_t *sessp = n->data;

		if (sessp->is_rtsp)
			continue;

		struct sockaddr_in addr;
		socklen_t len = sizeof(addr);
		if (getpeername(sessp->rtsp_snd_sock, (struct sockaddr *)&addr, &len) < 0)
			continue;

		if ((addr.sin_addr.s_addr == peeraddr.sin_addr.s_addr) &&
			(ntohs(addr.sin_port) == ntohs(peeraddr.sin_port) - 1)) {
			/* session found */
			sessp->rtsp_rcv_sock = connfd;

			if (pthread_create(&sessp->recv_data_tid, NULL, rtsp_connect_thread, sessp) != 0) {
				pthread_mutex_unlock(&rtsp_serv.sess_list_lock);
				return -1;
			}

			pthread_mutex_unlock(&rtsp_serv.sess_list_lock);
			return 0;
		}
	}
	pthread_mutex_unlock(&rtsp_serv.sess_list_lock);

	return -1;
}

static int http_new_connection(int server_fd)
{
	int ret = -1;

	char buffer[BUFSIZ];

	struct sockaddr_in peeraddr;
	socklen_t addrlen = sizeof(struct sockaddr_in);

	int connfd = accept(server_fd, (struct sockaddr *)&peeraddr, &addrlen);
	if (connfd < 0) {
		perror("Accept connection error");
		return -1;
	}

	if (socket_set_nonblock(connfd, 1) < 0) {
		close(connfd);
		return -1;
	}

	if (rtsp_read_complete(connfd, buffer, sizeof(buffer)) < 0) {
		close(connfd);
		return -1;
	}

	if (!strncmp(buffer, "GET", 3))
		ret = http_handle_GET(connfd);
	else if (!strncmp(buffer, "POST", 4))
		ret = http_handle_POST(connfd);

	if (ret < 0) {
		close(connfd);
		return -1;
	}

	RTSP_INFO("Accept new HTTP client from [%s:%d]\n",
			inet_ntoa(peeraddr.sin_addr), ntohs(peeraddr.sin_port));

	return 0;
}
#if USE_RTSP_REAPTER_THREAD
static void *rtsp_reaper_thread(void *arg)
{
    sdk_sys_thread_set_name("rtsp_reaper_thread");
	JListNode *n, *n_next;

	JList *waitclose = j_list_alloc();
	if (!waitclose)
		return NULL;

	while (!rtsp_serv.abort_reaper) {
		pthread_mutex_lock(&rtsp_serv.sess_list_lock);

		for (n = j_list_first(rtsp_serv.allsessions); n; n = n_next) {
			n_next = j_list_next(rtsp_serv.allsessions, n);

			rtsp_session_t *sessp = n->data;

			if (sessp->abort || __sync_add_and_fetch(&sessp->timeout, 1) > 90) {
				/* delete from session list, add to aborted session list */
				j_list_remove(rtsp_serv.allsessions, sessp);
				if (sessp->multicast)
					rtsp_multicast_session_del_session(sessp);
				j_list_append(waitclose, sessp);
				continue;
			}
		}

		pthread_mutex_unlock(&rtsp_serv.sess_list_lock);

		for (n = j_list_first(waitclose); n; n = n_next) {
			n_next = j_list_next(waitclose, n);
			rtsp_session_t *sessp = n->data;
			j_list_remove(waitclose, sessp);
			rtsp_closeRtspSession(sessp);
		}

		sleep(1);
	}

	j_list_free(waitclose);

	return NULL;
}
#else
static void rtsp_repater_check()
{
	JListNode *n, *n_next;

	if (!waitclose)
	{
	    waitclose = j_list_alloc();
	}

		pthread_mutex_lock(&rtsp_serv.sess_list_lock);

		for (n = j_list_first(rtsp_serv.allsessions); n; n = n_next) {
			n_next = j_list_next(rtsp_serv.allsessions, n);

			rtsp_session_t *sessp = n->data;

			if (sessp->abort || __sync_add_and_fetch(&sessp->timeout, 1) > 90) {
				/* delete from session list, add to aborted session list */
				j_list_remove(rtsp_serv.allsessions, sessp);
				if (sessp->multicast)
					rtsp_multicast_session_del_session(sessp);
				j_list_append(waitclose, sessp);
				continue;
			}
		}

		pthread_mutex_unlock(&rtsp_serv.sess_list_lock);

		for (n = j_list_first(waitclose); n; n = n_next) {
			n_next = j_list_next(waitclose, n);
			rtsp_session_t *sessp = n->data;
			j_list_remove(waitclose, sessp);
            RTSP_INFO("rtsp session repater check close:0x%x\n",sessp);
			rtsp_closeRtspSession(sessp);
		}


}


#endif
static void *rtsp_listen_thread(void *arg)
{
    sdk_sys_thread_set_name("rtsp_listen_thread");
	struct pollfd fds[2];
	memset(&fds, 0, sizeof(fds));

	fds[0].fd = rtsp_serv.rtspsock;
	fds[0].events = POLLIN;

	fds[1].fd = rtsp_serv.httpsock;
	fds[1].events = POLLIN;

	/* Accept remote user connection and request of opening channes. */
	while (!rtsp_serv.abort_listen_thread) {
		int n = poll(fds, NELEMS(fds), 1000);
		if (n < 0) {
			if (errno == EINTR)
				continue;
			return NULL;
		} else if (n == 0) {
		    #if USE_RTSP_REAPTER_THREAD == 0
		    rtsp_repater_check();
            #endif
			continue;
		}

		if (fds[0].revents & POLLIN)
			rtsp_new_connection(rtsp_serv.rtspsock);

		if (fds[1].revents & POLLIN)
			http_new_connection(rtsp_serv.httpsock);
	}

	return NULL;
}

int rtsp_startRtspServ(void)
{
	int ret = 0;

	rtsp_serv.rtspsock = tcp_server(NULL, rtsp_serv.rtsp_port);
	if (rtsp_serv.rtspsock < 0) {
		perror("create rtsp listen socket fail");
		return -1;
	}

	rtsp_serv.httpsock = tcp_server(NULL, rtsp_serv.http_port);
	if (rtsp_serv.httpsock < 0) {
        RTSP_ERR("Create http listen socket fail, error: %s\n", strerror(errno));
		goto fail;
	}

    #if USE_RTSP_REAPTER_THREAD
	ret = pthread_create(&rtsp_serv.rtsp_reaper_tid, NULL, rtsp_reaper_thread, NULL);
	if (ret != 0) {
		RTSP_ERR("Create rtsp_reaper_thread error: %s\n", strerror(ret));
		goto fail;
	}
    #else

    #endif

	ret = pthread_create(&rtsp_serv.rtsp_listen_tid, NULL, rtsp_listen_thread, NULL);
	if (ret != 0) {
		RTSP_ERR("Create rtsp_listen_thread error: %s\n", strerror(ret));
		goto fail;
	}

	return 0;

fail:
	if (rtsp_serv.rtspsock >= 0)
		close(rtsp_serv.rtspsock);
	if (rtsp_serv.httpsock >= 0)
		close(rtsp_serv.httpsock);

    #if USE_RTSP_REAPTER_THREAD
	if (rtsp_serv.rtsp_reaper_tid != 0) {
		rtsp_serv.abort_reaper = 1;
		pthread_join(rtsp_serv.rtsp_reaper_tid, NULL);
	}
    #endif
	return -1;
}

int rtsp_stopAll(void)
{
	JListNode *n, *n_next;
    pthread_mutex_lock(&rtsp_serv.sess_list_lock);
    for (n = j_list_first(rtsp_serv.allsessions); n; n = n_next) {
        n_next = j_list_next(rtsp_serv.allsessions, n);

        rtsp_session_t *sessp = n->data;

        j_list_remove(rtsp_serv.allsessions, sessp);

        if (sessp->multicast)
            rtsp_multicast_session_del_session(sessp);

        rtsp_closeRtspSession(sessp);
    }
    pthread_mutex_unlock(&rtsp_serv.sess_list_lock);
    return 0;
}

int rtsp_stopRtspServ(void)
{
	JListNode *n, *n_next;

	rtsp_serv.abort_listen_thread = 1;
	rtsp_serv.abort_reaper = 1;

	if (rtsp_serv.rtsp_listen_tid != 0) {
		pthread_join(rtsp_serv.rtsp_listen_tid, NULL);
		rtsp_serv.rtsp_listen_tid = 0;
	}

	if (rtsp_serv.rtsp_reaper_tid != 0) {
		pthread_join(rtsp_serv.rtsp_reaper_tid, NULL);
		rtsp_serv.rtsp_reaper_tid = 0;
	}

	close(rtsp_serv.httpsock);
	rtsp_serv.httpsock = -1;

	close(rtsp_serv.rtspsock);
	rtsp_serv.rtspsock = -1;

	pthread_mutex_lock(&rtsp_serv.sess_list_lock);
	for (n = j_list_first(rtsp_serv.allsessions); n; n = n_next) {
		n_next = j_list_next(rtsp_serv.allsessions, n);

		rtsp_session_t *sessp = n->data;

		j_list_remove(rtsp_serv.allsessions, sessp);

		if (sessp->multicast)
			rtsp_multicast_session_del_session(sessp);

		rtsp_closeRtspSession(sessp);
	}
	pthread_mutex_unlock(&rtsp_serv.sess_list_lock);

	/* delete all multicast session */
	pthread_mutex_lock(&rtsp_serv.mcast_list_lock);
	for (n = j_list_first(rtsp_serv.allmulticasts); n; n = n_next) {
		n_next = j_list_next(rtsp_serv.allmulticasts, n);

		rtsp_session_t *sessp = n->data;

		j_list_remove(rtsp_serv.allmulticasts, sessp);
		rtsp_closeRtspMcastSession(sessp);
	}
	pthread_mutex_unlock(&rtsp_serv.mcast_list_lock);

	rtsp_serv.abort_listen_thread = 0;
	rtsp_serv.abort_reaper = 0;

    #if USE_RTSP_REAPTER_THREAD==0
    if(waitclose);
        j_list_free(waitclose);
    #endif

	return 0;
}

rtsp_session_t *rtsp_find_multicast_session(const char *path)
{
	JListNode *n, *n_next;

	pthread_mutex_lock(&rtsp_serv.mcast_list_lock);
	for (n = j_list_first(rtsp_serv.allmulticasts); n; n = n_next) {
		n_next = j_list_next(rtsp_serv.allmulticasts, n);

		rtsp_session_t *sessp = n->data;

		if (strcmp(sessp->url->path, path) == 0) {
			pthread_mutex_unlock(&rtsp_serv.mcast_list_lock);
			return sessp;
		}
	}
	pthread_mutex_unlock(&rtsp_serv.mcast_list_lock);

	return NULL;
}

static int rtp_multicast_init(rtsp_session_t *sessp, int media)
{
	int ret;
	int first_sock = -1;
	int second_sock = -1;
	uint16_t rtpport = 0;
	uint16_t rtcpport = 0;

	if (!sessp)
    {
        RTSP_ERR("param error");
        return -1;
    }

	sessp->multicast = 1;

	int ttl;
	char addr[64];
	if (rtp_get_multicast_addr(sessp, media, addr, sizeof(addr), &rtpport, &ttl) < 0)
    {
        RTSP_ERR("multicase error " );
		goto fail;
    }

	/* Create first socket. */
	if ((first_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Create first UDP socket for RTP error");
		goto fail;
	}

	if ((second_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
		perror("Create second UDP socket for RTP error");
		goto fail;
	}

	sessp->rtp[media].rtp_sock = first_sock;
	sessp->rtp[media].rtcp_sock = second_sock;

	rtcpport = rtpport + 1;

	in_addr_t s_addr = inet_addr(addr);

	/* setup multicast rtp */
	ret = socket_connect(sessp->rtp[media].rtp_sock, s_addr, rtpport);
	if (ret < 0)
    {
        RTSP_ERR("connect multicast rtp failed ");
		goto fail;
    }

	if (socket_set_nonblock(first_sock, 1) < 0)
		goto fail;

	/* setup multicast rtcp */
	ret = socket_connect(sessp->rtp[media].rtcp_sock, s_addr, rtcpport);
	if (ret < 0)
    {
         RTSP_ERR("connect multicast rtcp failed ");
		goto fail;
    }

	if (socket_set_nonblock(second_sock, 1) < 0)
		goto fail;

	if (setsockopt(first_sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
		perror("IP_MULTICAST_TTL");
		goto fail;
	}

	if (setsockopt(second_sock, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0) {
		perror("IP_MULTICAST_TTL");
		goto fail;
	}

	return 0;

fail:
	if (first_sock >= 0)
		close(first_sock);
	if (second_sock >= 0)
		close(second_sock);
	return -1;
}

static int valid_multicast_desc(MediaDesc *desc)
{
	if (desc->multicast.enable && strlen(desc->multicast.ipaddr) && desc->multicast.port)
		return 1;
	return 0;
}

rtsp_session_t *rtsp_create_multicast_session(const char *request_uri)
{
	int i;
	rtsp_session_t *sessp;

	sessp = malloc(sizeof(rtsp_session_t));
	if (!sessp)
		return NULL;
	memset(sessp, 0, sizeof(rtsp_session_t));

	strncpy(sessp->request_uri, request_uri, sizeof(sessp->request_uri) - 1);

	sessp->url = UrlParse(sessp->request_uri);
	if (!sessp->url) {
		free(sessp);
        RTSP_ERR("url is null");
		return NULL;
	}

	int media_type = 0;	/* 0 - all, 1 - video, 2 - audio */

	if (sessp->url->query) {
		char *q = sessp->url->query;
		while (q && (*q != '\0')) {
			char query[128] = {0};
			get_word_until_chars(query, sizeof(query), "&", &q);
			if (!strlen(query))
				continue;

			if (!strcmp(query, "media=all"))
				media_type = 0;
			else if (!strcmp(query, "media=video"))
				media_type = 1;
			else if (!strcmp(query, "media=audio"))
				media_type = 2;
		}
	}

	RtspRoute *r = rtsp_get_route(sessp->url->path);
	if (!r) {
		UrlFree(sessp->url);
		free(sessp);
        RTSP_ERR("route is null");
		return NULL;
	}

	sessp->ops = &r->ops;

	sessp->handle = sessp->ops->open(sessp->url->path, sessp->url->query);
	if (!sessp->handle) {
		UrlFree(sessp->url);
		free(sessp);
        RTSP_ERR("handle is null");
		return NULL;
	}

	sessp->ndesc = sessp->ops->describe(sessp->url->path, sessp->url->query, sessp->mdescs, NELEMS(sessp->mdescs));
	if (sessp->ndesc <= 0) {
		sessp->ops->close(sessp->handle);
		UrlFree(sessp->url);
		free(sessp);
        RTSP_ERR("ndesc < 0");
		return NULL;
	}

	int found = 0;
	for (i = 0; i < sessp->ndesc; i++) {
		MediaDesc *desc = &sessp->mdescs[i];
		if (!valid_multicast_desc(desc))
        {
            RTSP_ERR("%s", __func__);
			continue;
        }

		if (desc->media == 0 && (media_type == 0 || media_type == 1))
			found = 1;
		else if (desc->media == 1 && (media_type == 0 || media_type == 2))
			found = 1;
	}

	if (found == 0) {
		sessp->ops->close(sessp->handle);
		UrlFree(sessp->url);
        free(sessp);
        RTSP_ERR("not found file");
		return NULL;
	}

	sessp->max_payload_size = RTP_MAX_PAYLOAD_SIZE;

	for (i = 0; i < NELEMS(sessp->rtp); i++) {
		sessp->rtp[i].rtp_sock  = -1;
		sessp->rtp[i].rtcp_sock = -1;
	}

	/* FIXME: try to release all resource when fail */

	for (i = 0; i < sessp->ndesc; i++) {
		MediaDesc *desc = &sessp->mdescs[i];

		if (!valid_multicast_desc(desc))
        {
            RTSP_ERR("multicaset desc errr");
			continue;
        }

		if (desc->media == 0 && (media_type == 0 || media_type == 1)) { 	/* video */
			if (rtp_multicast_init(sessp, MEDIA_TYPE_VIDEO) < 0)
            {
                RTSP_ERR("multicaset init video errr");
				goto fail;
            }

			if (initRtpHdr(&sessp->rtp[MEDIA_TYPE_VIDEO].rtp_hdr, desc->codec) < 0)
            {
                RTSP_ERR("multicaset initRtpHdr video errr");
				goto fail;
            }

			sessp->rtp[MEDIA_TYPE_VIDEO].enable = 1;
		} else if (desc->media == 1 && (media_type == 0 || media_type == 2)) {	/* audio */
			if (rtp_multicast_init(sessp, MEDIA_TYPE_AUDIO) < 0)
            {
                RTSP_ERR("rtp_multicast_init audio errr");
				goto fail;
            }

			if (initRtpHdr(&sessp->rtp[MEDIA_TYPE_AUDIO].rtp_hdr, desc->codec) < 0)
            {
                RTSP_ERR("initRtpHdr audio errr");
				goto fail;
            }

			sessp->rtp[MEDIA_TYPE_AUDIO].enable = 1;
		}
	}
	sessp->y_buf= malloc(sizeof(y_test_buf));
    if(!sessp->y_buf)
    {
        free(sessp );
        return NULL;
    }
    sessp->y_buf->g_y_buffer_total = 50*1024;

    sessp->y_buf->g_y_buffer = malloc(sessp->y_buf->g_y_buffer_total);
    if(!sessp->y_buf->g_y_buffer)
    {
        free(sessp->y_buf);
        free(sessp );

        return NULL;
    }


	if (pthread_create(&sessp->send_data_tid, NULL, rtsp_stream_thread, sessp) != 0)
		goto fail;

	pthread_mutex_lock(&rtsp_serv.mcast_list_lock);
	j_list_append(rtsp_serv.allmulticasts, sessp);
	pthread_mutex_unlock(&rtsp_serv.mcast_list_lock);

	return sessp;

fail:
	rtsp_closeRtspMcastSession(sessp);
	return NULL;
}

int rtsp_multicast_session_add_session(rtsp_session_t *sessp)
{
	JListNode *n, *n_next;

	pthread_mutex_lock(&rtsp_serv.mcast_list_lock);
	for (n = j_list_first(rtsp_serv.allmulticasts); n; n = n_next) {
		n_next = j_list_next(rtsp_serv.allmulticasts, n);

		rtsp_session_t *msessp = n->data;
		if (strcmp(msessp->url->path, sessp->url->path) == 0) {
			msessp->mcast_refcount++;
			pthread_mutex_unlock(&rtsp_serv.mcast_list_lock);
			return 0;
		}
	}
	pthread_mutex_unlock(&rtsp_serv.mcast_list_lock);

	return -1;
}

int rtsp_multicast_session_del_session(rtsp_session_t *sessp)
{

	JListNode *n, *n_next;

	pthread_mutex_lock(&rtsp_serv.mcast_list_lock);

	for (n = j_list_first(rtsp_serv.allmulticasts); n; n = n_next) {
		n_next = j_list_next(rtsp_serv.allmulticasts, n);

		rtsp_session_t *msessp = n->data;

		if (strcmp(msessp->url->path, sessp->url->path) == 0) {
			msessp->mcast_refcount--;

			/* destroy rtsp multicast session when reference count become zero */
			if (msessp->mcast_refcount == 0) {
				j_list_remove(rtsp_serv.allmulticasts, msessp);
				rtsp_closeRtspMcastSession(msessp);
			}

			pthread_mutex_unlock(&rtsp_serv.mcast_list_lock);
			return 0;
		}
	}

	pthread_mutex_unlock(&rtsp_serv.mcast_list_lock);

	return -1;
}
