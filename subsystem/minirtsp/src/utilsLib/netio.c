/*!
*****************************************************************************
** \file      $gkprjnetio.c
**
** \version	$id: netio.c 15-08-04  8月:08:1438655395 
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
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <sys/poll.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <netdb.h>
#include <fcntl.h>

int fd_wait_readable(int sockfd, int milliseconds)
{
	struct pollfd fds;
    return 1;
wait_again:
	memset(&fds, 0, sizeof(struct pollfd));

	fds.fd     = sockfd;
	fds.events = POLLIN;

	int ret = poll(&fds, 1, milliseconds);
	if (ret < 0 && (errno == EINTR))
		goto wait_again;	/* interrupted by signal, try again */

	return ret;
}

int fd_wait_writeable(int sockfd, int milliseconds)
{
	struct pollfd fds;
    return 1;
wait_again:
	memset(&fds, 0, sizeof(struct pollfd));

	fds.fd     = sockfd;
	fds.events = POLLOUT;

	int ret = poll(&fds, 1, milliseconds);
	if (ret < 0 && (errno == EINTR))
		goto wait_again;	/* interrupted by signal, try again */

	return ret;
}

static ssize_t read_timedwait(int fd, void *buf, size_t count, int sec)
{
	int n;

	if (fd_wait_readable(fd, sec * 1000) <= 0)
		return -1;

read_again:
	n = read(fd, buf, count);
	if (n < 0) {
		if (n < 0 && (errno == EINTR || errno == EAGAIN))
			goto read_again;   /* Call read() again. */
		return -1;
	}

	return n;
}

static ssize_t write_timedwait(int fd, const void *buf, size_t count, int sec)
{
	int n;

	if (fd_wait_writeable(fd, sec * 1000) <= 0)
		return -1;

write_again:
	n = write(fd, buf, count);
	if (n < 0) {
		if (n < 0 && (errno == EINTR || errno == EAGAIN))
			goto write_again;   /* Call write() again. */
		return -1;
	}

	return n;
}

ssize_t readn_timedwait(int fd, void *buf, size_t count)
{
	ssize_t n;
	size_t left = count;
	char *ptr = buf;

	while (left > 0) {
		n = read_timedwait(fd, ptr, left, 10);
		if (n <= 0)
			return -1;

		left -= n;
		ptr += n;
	}

	return count;
}

ssize_t writen_timedwait(int fd, void *buf, size_t count)
{
	ssize_t n;
	size_t left = count;
	const char *ptr = buf;

	while (left > 0) {
		n = write_timedwait(fd, ptr, left, 10);
		if (n <= 0)
			return -1;

		left -= n;
		ptr += n;
	}

	return count;
}

/* resolve host with also IP address parsing */
static int resolve_host(struct in_addr *sin_addr, const char *hostname)
{

	if (!inet_aton(hostname, sin_addr)) {
		struct addrinfo *ai, *cur;
		struct addrinfo hints = { 0 };
		hints.ai_family = AF_INET;
		if (getaddrinfo(hostname, NULL, &hints, &ai))
			return -1;
		/* getaddrinfo returns a linked list of addrinfo structs.
		 * Even if we set ai_family = AF_INET above, make sure
		 * that the returned one actually is of the correct type. */
		for (cur = ai; cur; cur = cur->ai_next) {
			if (cur->ai_family == AF_INET) {
				*sin_addr = ((struct sockaddr_in *)cur->ai_addr)->sin_addr;
				freeaddrinfo(ai);
				return 0;
			}
		}
		freeaddrinfo(ai);
		return -1;
	}
	return 0;
}

static int connect_timedwait(int sockfd, struct sockaddr *peeraddr, socklen_t len, int seconds)
{
	int flags = fcntl(sockfd, F_GETFL);
	if (flags < 0)
		return -1;

	/* nonblocking */
	if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0)
		return -1;

	if (connect(sockfd, peeraddr, len) < 0) {
		if (errno != EINPROGRESS)
			goto fail;

		if (fd_wait_writeable(sockfd, seconds * 1000) < 0)
			goto fail;

		/* check if connect successfully */
		int optval;
		socklen_t optlen = sizeof(optval);

		if (getsockopt(sockfd, SOL_SOCKET, SO_ERROR, &optval, &optlen) < 0) {
			perror("getsockopt");
			goto fail;
		}

		if (optval != 0) {
			/* connect failed */
			errno = optval;
			goto fail;
		}
	}

	/* restore socket status */
	if (fcntl(sockfd, F_SETFL, flags) < 0)
		return -1;

	return 0;

fail:
	/* restore socket status */
	if (fcntl(sockfd, F_SETFL, flags) < 0)
		return -1;

	return -1;
}

int socket_set_nonblock(int socket, int enable)
{
	if (enable)
		return fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) | O_NONBLOCK);
	else
		return fcntl(socket, F_SETFL, fcntl(socket, F_GETFL) & ~O_NONBLOCK);
}

int tcp_connect(char *ipaddr, int port)
{
	int fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd < 0) {
		perror("socket");
		return -1;
	}

	struct sockaddr_in peeraddr;
	memset(&peeraddr, 0, sizeof(peeraddr));

	peeraddr.sin_family = AF_INET;
	peeraddr.sin_port = htons(port);

	if (resolve_host(&peeraddr.sin_addr, ipaddr) < 0) {
		close(fd);
		return -1;
	}

	if (connect_timedwait(fd, (struct sockaddr *)&peeraddr, sizeof(peeraddr), 5) < 0) {
		close(fd);
		return -1;
	}

	if (socket_set_nonblock(fd, 1) < 0) {
		perror("socket_set_nonblock");
		close(fd);
		return -1;
	}

	return fd;
}

#define BACKLOG     (5)

int tcp_server(char *ipaddr, int port)
{
	int ret;
	int sockfd;
	in_addr_t s_addr;
	int reuseaddr = 1;
	struct sockaddr_in localaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0)
		return -1;

	if (ipaddr && ipaddr[0] && inet_pton(AF_INET, ipaddr, &s_addr) != 1) {
		goto fail;
	} else {
		s_addr = htonl(INADDR_ANY);
	}

	memset(&localaddr, 0, sizeof(localaddr));
	localaddr.sin_family = AF_INET;
	localaddr.sin_port = htons(port);
	localaddr.sin_addr.s_addr = s_addr;

	/* Create socket for listening connections. */
	ret = setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuseaddr, sizeof(reuseaddr));
	if (ret < 0)
		goto fail;

	ret = bind(sockfd, (struct sockaddr *)&localaddr, sizeof(localaddr));
	if (ret < 0)
		goto fail;

	ret = listen(sockfd, BACKLOG);
	if (ret < 0)
		goto fail;

	return sockfd;

fail:
	close(sockfd);
	return -1;
}

int udp_connect(char *ipaddr, int port)
{
	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
		return -1;

	struct sockaddr_in peeraddr;
	memset(&peeraddr, 0, sizeof(peeraddr));

	peeraddr.sin_family = AF_INET;
	peeraddr.sin_port = htons(port);

	if (resolve_host(&peeraddr.sin_addr, ipaddr) < 0) {
		close(sockfd);
		return -1;
	}

	if (connect_timedwait(sockfd, (struct sockaddr *)&peeraddr, sizeof(peeraddr), 5) < 0) {
		close(sockfd);
		return -1;
	}

	if (socket_set_nonblock(sockfd, 1) < 0) {
		close(sockfd);
		return -1;
	}

	return sockfd;
}

int udp_server(char *ipaddr, int port)
{
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof(struct sockaddr_in));

	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);

	if (ipaddr) {
		if (inet_pton(AF_INET, ipaddr, &addr.sin_addr.s_addr) != 1)
			return -1;
	} else {
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
	}

	int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
	if (sockfd < 0)
		return -1;

	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) < 0) {
		close(sockfd);
		return -1;
	}

	if (socket_set_nonblock(sockfd, 1) < 0) {
		close(sockfd);
		return -1;
	}

	return sockfd;
}
