/*!
*****************************************************************************
** \file      $gkprjdynbuf.c
**
** \version	$id: dynbuf.c 15-08-04  8月:08:1438655338 
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
#include <stdarg.h>
#include <math.h>
#include <stdint.h>

#include "dynbuf.h"

#define min(x, y)	((x) < (y) ? (x) : (y))

JDynBuf *j_dynbuf_alloc(int packet_size)
{
	JDynBuf *p = malloc(sizeof(JDynBuf));
	if (!p)
		return NULL;
	memset(p, 0, sizeof(JDynBuf));

	p->buffer = malloc(packet_size);
	if (!p->buffer) {
		free(p);
		return NULL;
	}

	p->bufsiz = packet_size;

	j_dynbuf_drop(p);

	return p;
}

int j_dynbuf_free(JDynBuf *pb)
{
	if (pb) {
		if (pb->buffer)
			free(pb->buffer);
		free(pb);
	}

	return 0;
}

int j_dynbuf_size(JDynBuf *pb)
{
	if (!pb)
		return -1;

	return (pb->write_flag) ? pb->bufptr - pb->buffer : pb->bufend - pb->bufptr;
}

int j_dynbuf_space(JDynBuf *pb)
{
	if (!pb || !pb->write_flag)
		return -1;

	return pb->bufend - pb->bufptr;
}

int j_dynbuf_read(JDynBuf *pb, int (*cb)(char *buf, int size, void *ctx), void *ctx)
{
	if (!pb || !cb)
		return -1;

	int ret = cb(pb->bufptr, pb->bufend - pb->bufptr, ctx);
	if (ret < 0)
		return -1;

	pb->bufptr += ret;

	return ret;
}

void j_dynbuf_drop(JDynBuf *pb)
{
	pb->bufptr = pb->buffer;
	pb->bufend = pb->buffer + pb->bufsiz;
	pb->write_flag = 1;
}

void j_dynbuf_read_prepare(JDynBuf *pb)
{
	pb->bufend = pb->bufptr;
	pb->bufptr = pb->buffer;
	pb->write_flag = 0;
}

int j_dynbuf_copy(JDynBuf *dst, JDynBuf *src, int size, int auto_expand)
{
	if (!dst || !src || !size)
		return -1;

	int have = j_dynbuf_size(src);
	if (!have)
		return 0;

	int left = j_dynbuf_space(dst);
	if (left < size && auto_expand) {
		if (j_dynbuf_expand(dst, size - left) < 0)
			return -1;
		left = j_dynbuf_space(dst);
	} else if (left == 0)
		return -1;

	int copy;
	copy = min(have, left);
	copy = min(copy, size);

	memcpy(dst->bufptr, src->bufptr, copy);

	dst->bufptr += copy;

	return copy;
}

int j_dynbuf_move(JDynBuf *dst, JDynBuf *src, int size, int auto_expand)
{
	int len = j_dynbuf_copy(dst, src, size, auto_expand);
	if (len < 0)
		return -1;

	return j_dynbuf_drop_front(src, len);
}

int j_dynbuf_drop_front(JDynBuf *pb, int size)
{
	if (pb->write_flag)
		return -1;

	int left = j_dynbuf_size(pb);
	int drop = min(size, left);

	pb->bufptr += drop;

	return drop;
}

int j_dynbuf_drop_tail(JDynBuf *pb, int size)
{
	int left = j_dynbuf_size(pb);
	int drop = min(size, left);

	if (pb->write_flag)
		pb->bufptr -= drop;
	else
		pb->bufend -= drop;

	return drop;
}

void *j_dynbuf_get(JDynBuf *pb)
{
	if (!pb)
		return NULL;

	return pb->write_flag ? pb->buffer : pb->bufptr;
}

int j_dynbuf_expand(JDynBuf *pb, int more)
{
	char *new_buf;
	double radio;
	double radio_f = (more + pb->bufsiz) / (double)pb->bufsiz;
	double times = floor(radio_f);
	int data_len = pb->bufptr - pb->buffer;

	if ((radio_f - times) > 0.5) {
		radio = times + 1.0;
	} else {
		radio = times + 0.5;
	}

	int new_size = pb->bufsiz * radio;
	new_buf = realloc(pb->buffer, new_size);
	if (new_buf < 0)
		return -1;

	pb->buffer = new_buf;
	pb->bufsiz = new_size;
	pb->bufptr = pb->buffer + data_len;
	pb->bufend = pb->buffer + pb->bufsiz;

	return 0;
}

int j_dynbuf_append(JDynBuf *pb, char *buf, int len)
{
	int left = pb->bufend - pb->bufptr;

	if (left < len) {
		if (j_dynbuf_expand(pb, len) < 0)
			return -1;
	}

	memcpy(pb->bufptr, buf, len);
	pb->bufptr += len;

	return 0;
}

int j_dynbuf_printf(JDynBuf *pb, const char *fmt, ...)
{
	va_list ap;
	char buf[4096];
	int ret;

	va_start(ap, fmt);
	ret = vsnprintf(buf, sizeof(buf), fmt, ap);
	va_end(ap);

	if (ret < 0)
		return ret;

	int left = pb->bufend - pb->bufptr - 1;	/* one byte for '\0' */
	int new_len = strlen(buf);

	if (left < new_len) {
		if (j_dynbuf_expand(pb, new_len) < 0)
			return -1;
	}

	memcpy(pb->bufptr, buf, new_len);
	pb->bufptr += new_len;

	*(pb->bufptr) = '\0';

	return 0;
}
