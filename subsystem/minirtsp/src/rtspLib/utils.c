/*!
*****************************************************************************
** \file      $gkprjutils.c
**
** \version	$id: utils.c 15-08-04  8月:08:1438655194 
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
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <assert.h>
#include <errno.h>
#include <limits.h>
#include <ctype.h>

#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>

#include "utils.h"
#include "netio.h"

typedef struct status_code_s {
	unsigned int code;
	char *phrase;
} status_code_t;

status_code_t status_codes[] = {
	{100, "Continue"},
	{200, "OK"},
	{201, "Created"},
	{250, "Low on Storage Space"},
	{300, "Multiple Choices"},
	{301, "Moved Permanently"},
	{302, "Moved Temporarily"},
	{303, "See Other"},
	{304, "Not Modified"},
	{305, "Use Proxy"},
	{400, "Bad Request"},
	{401, "Unauthorized"},
	{402, "Payment Required"},
	{403, "Forbidden"},
	{404, "Not Found"},
	{405, "Method Not Allowed"},
	{406, "Not Acceptable"},
	{407, "Proxy Authentication Required"},
	{408, "Request Time-out"},
	{410, "Gone"},
	{411, "Length Required"},
	{412, "Precondition Failed"},
	{413, "Request Entity Too Large"},
	{414, "Request-URI Too Large"},
	{415, "Unsupported Media Type"},
	{451, "Parameter Not Understood"},
	{452, "Conference Not Found"},
	{453, "Not Enough Bandwidth"},
	{454, "Session Not Found"},
	{455, "Method Not Valid in This State"},
	{456, "Header Field Not Valid for Resource"},
	{457, "Invalid Range"},
	{458, "Parameter Is Read-Only"},
	{459, "Aggregate operation not allowed"},
	{460, "Only aggregate operation allowed"},
	{461, "Unsupported transport"},
	{462, "Destination unreachable"},
	{500, "Internal Server Error"},
	{501, "Not Implemented"},
	{502, "Bad Gateway"},
	{503, "Service Unavailable"},
	{504, "Gateway Time-out"},
	{505, "RTSP Version not supported"},
	{551, "Option not supported"},
};

char *rtsp_state_message(unsigned int code)
{
	int i = 0;
	int n = sizeof(status_codes) / sizeof(status_codes[0]);

	for (i = 0; i < n; i++) {
		if (status_codes[i].code == code) {
			return status_codes[i].phrase;
		}
	}
	return NULL;
}

ssize_t rtsp_read(int fd, void *buf, size_t count)
{
	int len;

	while (1) {
		if (fd_wait_readable(fd, 5000) <= 0)
			return -1;

		len = read(fd, buf, count);
		if (len <= 0) {
			if (len < 0 && errno == EINTR)
				continue;
			return -1;
		}

		break;
	}

	return len;
}


/**
 * Read @n bytes from a descriptor.
 */
#if 0
ssize_t rtsp_readn(int fd, void *buf, size_t n)
{
	size_t nleft;
	ssize_t nr;
	char *ptr;

	ptr = (char *)buf;
	nleft = n;
	while (nleft > 0) {
		if ((nr = read(fd, ptr, nleft)) <= 0) {
			if (nr == 0) {
				break;  /* End of descriptor. */
			} else {
				if (errno == EINTR || errno == EAGAIN) {
					perror("readn");
					continue;   /* Call read() again. */
				} else {
					return -1;
				}
			}
		}
		nleft -= nr;
		ptr += nr;
	}

	return (n - nleft);
}
#else
ssize_t rtsp_readn(int fd, void *bufptr, size_t count)
{
	int left = count;

	while (left > 0) {
		if (fd_wait_readable(fd, 5000) <= 0)
			return -1;

		int len = read(fd, bufptr, left);
		if (len <= 0)
			return -1;

		bufptr += len;
		left   -= len;
	}

	return count;
}
#endif

ssize_t rtsp_write(int fd, const void *buf, size_t count)
{
	if (fd_wait_writeable(fd, 5000) < 0)
		return -1;

	int len = write(fd, buf, count);
	if (len <= 0)
		return -1;

	return len;
}

/**
 * Write @n bytes to a descriptor. 
 */
#if 0
ssize_t rtsp_writen(int fd, const void *buf, size_t n)
{
	size_t nleft;
	ssize_t nw;
	char *ptr;

	ptr = (char *)buf;
	nleft = n;
	while (nleft > 0) {
		if ((nw = write(fd, ptr, nleft)) <= 0) {
			if (nw < 0 && (errno == EINTR || errno == EAGAIN)) {
				continue;   /* Call write() again. */
			} else {
				return -1;
			}
		}
		nleft -= nw;
		ptr += nw;
	}

	return (n - nleft);
}
#else
ssize_t rtsp_writen(int fd, const void *bufptr, size_t count)
{
	int left = count;

	while (left > 0) {
		if (fd_wait_writeable(fd, 5000) < 0)
			return -1;

		int len = write(fd, bufptr, left);
		if (len <= 0)
			return -1;

		bufptr += len;
		left -= len;
	}

	return count;
}
#endif

/**
 * Return a 64 bits random number.
 */
long long rtsp_random64(void)
{
	static int seed = 0x19860827;

	struct timeval tv;
	gettimeofday(&tv, NULL);

	seed += tv.tv_usec;
	srandom(seed);

	return ((long long)random() << 32) | random();
}

int rtsp_send_reply(rtsp_session_t *sessp, char *buf, int size)
{
	int ret;
    //RTSP_INFO("rtsp send, sock:%d, n=%d, s:\n%s\n", sessp->rtsp_snd_sock, size, buf);
	pthread_mutex_lock(&sessp->rtsp_snd_lock);
	ret = rtsp_writen(sessp->rtsp_snd_sock, buf, size);
	pthread_mutex_unlock(&sessp->rtsp_snd_lock);

	if (ret < 0)
		return -1;

	return 0;
}

int rtsp_reply_error(rtsp_session_t *sessp, int code)
{
	if (!sessp)
		return -1;

	char resp_buf[512];
	int len = 0;

	len = sprintf(resp_buf, RTSP_VERSION" %d %s\r\nCSeq: %d\r\n\r\n",
			code, rtsp_state_message(code), sessp->cseq);
	if (len >= sizeof(resp_buf)) {
		RTSP_ERR("Response message size is too long[%d]!", len);
		return -1;
	}

	if (rtsp_send_reply(sessp, resp_buf, strlen(resp_buf)) < 0)
		return -1;

	return 0;
}

void print_hex(void *buf, int len)
{
	char *pb = buf;
	int i;

	for (i = 0; i < len; i++) {
		printf("buf[%d] = 0x%02x = %d\n", i, pb[i], pb[i]);
	}

	printf("\n");
}

int socket_connect(int fd, in_addr_t s_addr, uint16_t port)
{
	int ret;
	struct sockaddr_in peeraddr;
	socklen_t addrlen;

	memset(&peeraddr, 0, sizeof(struct sockaddr_in));
	peeraddr.sin_family = AF_INET;
	peeraddr.sin_addr.s_addr = s_addr;
	peeraddr.sin_port = htons(port);

	addrlen = sizeof(struct sockaddr_in);
	ret = connect(fd, (struct sockaddr *)&peeraddr, addrlen);
	if (ret < 0) {
		perror("connect");
		return -1;
	}

	return 0;
}

/* get a positive number between n_min and n_max, for a maximum length
   of len_max. Return -1 if error. */
static int date_get_num(const char **pp,
                        int n_min, int n_max, int len_max)
{
    int i, val, c;
    const char *p;

    p = *pp;
    val = 0;
    for(i = 0; i < len_max; i++) {
        c = *p;
        if (!isdigit(c))
            break;
        val = (val * 10) + c - '0';
        p++;
    }
    /* no number read ? */
    if (p == *pp)
        return -1;
    if (val < n_min || val > n_max)
        return -1;
    *pp = p;
    return val;
}

static char *av_small_strptime(const char *p, const char *fmt, struct tm *dt)
{
    int c, val;

    for(;;) {
        /* consume time string until a non whitespace char is found */
        while (isspace(*fmt)) {
            while (isspace(*p))
                p++;
            fmt++;
        }
        c = *fmt++;
        if (c == '\0') {
            return (char *)p;
        } else if (c == '%') {
            c = *fmt++;
            switch(c) {
            case 'H':
            case 'J':
                val = date_get_num(&p, 0, c == 'H' ? 23 : INT_MAX, 2);
                if (val == -1)
                    return NULL;
                dt->tm_hour = val;
                break;
            case 'M':
                val = date_get_num(&p, 0, 59, 2);
                if (val == -1)
                    return NULL;
                dt->tm_min = val;
                break;
            case 'S':
                val = date_get_num(&p, 0, 59, 2);
                if (val == -1)
                    return NULL;
                dt->tm_sec = val;
                break;
            case 'Y':
                val = date_get_num(&p, 0, 9999, 4);
                if (val == -1)
                    return NULL;
                dt->tm_year = val - 1900;
                break;
            case 'm':
                val = date_get_num(&p, 1, 12, 2);
                if (val == -1)
                    return NULL;
                dt->tm_mon = val - 1;
                break;
            case 'd':
                val = date_get_num(&p, 1, 31, 2);
                if (val == -1)
                    return NULL;
                dt->tm_mday = val;
                break;
            case '%':
                goto match;
            default:
                return NULL;
            }
        } else {
        match:
            if (c != *p)
                return NULL;
            p++;
        }
    }
}

static time_t av_timegm(struct tm *tm)
{
    time_t t;

    int y = tm->tm_year + 1900, m = tm->tm_mon + 1, d = tm->tm_mday;

    if (m < 3) {
        m += 12;
        y--;
    }

    t = 86400LL *
        (d + (153 * m - 457) / 5 + 365 * y + y / 4 - y / 100 + y / 400 - 719469);

    t += 3600 * tm->tm_hour + 60 * tm->tm_min + tm->tm_sec;

    return t;
}

int parse_time(int64_t *timeval, const char *timestr, int duration)
{
    const char *p, *q;
    int64_t t;
    time_t now;
    struct tm dt = { 0 };
    int today = 0, negative = 0, microseconds = 0;
    int i;
    static const char * const date_fmt[] = {
        "%Y-%m-%d",
        "%Y%m%d",
    };
    static const char * const time_fmt[] = {
        "%H:%M:%S",
        "%H%M%S",
    };

    p = timestr;
    q = NULL;
    *timeval = INT64_MIN;
    if (!duration) {
        now = time(0);

        if (!strcasecmp(timestr, "now")) {
            *timeval = (int64_t) now * 1000000;
            return 0;
        }

        /* parse the year-month-day part */
        for (i = 0; i < NELEMS(date_fmt); i++) {
            q = av_small_strptime(p, date_fmt[i], &dt);
            if (q)
                break;
        }

        /* if the year-month-day part is missing, then take the
         * current year-month-day time */
        if (!q) {
            today = 1;
            q = p;
        }
        p = q;

        if (*p == 'T' || *p == 't' || *p == ' ')
            p++;

        /* parse the hour-minute-second part */
        for (i = 0; i < NELEMS(time_fmt); i++) {
            q = av_small_strptime(p, time_fmt[i], &dt);
            if (q)
                break;
        }
    } else {
        /* parse timestr as a duration */
        if (p[0] == '-') {
            negative = 1;
            ++p;
        }
        /* parse timestr as HH:MM:SS */
        q = av_small_strptime(p, "%J:%M:%S", &dt);
        if (!q) {
            /* parse timestr as MM:SS */
            q = av_small_strptime(p, "%M:%S", &dt);
            dt.tm_hour = 0;
        }
        if (!q) {
            /* parse timestr as S+ */
            dt.tm_sec = strtol(p, (void *)&q, 10);
			if (q == p) /* the parsing didn't succeed */
				return -EINVAL;
            dt.tm_min = 0;
            dt.tm_hour = 0;
        }
    }

    /* Now we have all the fields that we can get */
    if (!q)
        return -EINVAL;

    /* parse the .m... part */
    if (*q == '.') {
        int n;
        q++;
        for (n = 100000; n >= 1; n /= 10, q++) {
            if (!isdigit(*q))
                break;
            microseconds += n * (*q - '0');
        }
        while (isdigit(*q))
            q++;
    }

    if (duration) {
        t = dt.tm_hour * 3600 + dt.tm_min * 60 + dt.tm_sec;
    } else {
        int is_utc = *q == 'Z' || *q == 'z';
        q += is_utc;
        if (today) { /* fill in today's date */
            struct tm dt2 = is_utc ? *gmtime(&now) : *localtime(&now);
            dt2.tm_hour = dt.tm_hour;
            dt2.tm_min  = dt.tm_min;
            dt2.tm_sec  = dt.tm_sec;
            dt = dt2;
        }
        t = is_utc ? av_timegm(&dt) : mktime(&dt);
    }

    /* Check that we are at the end of the string */
	if (*q)
		return -EINVAL;

    t *= 1000000;
    t += microseconds;
    *timeval = negative ? -t : t;
    return 0;
}
