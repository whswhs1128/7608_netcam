/*!
*****************************************************************************
** \file      $gkprjfifo.c
**
** \version	$id: fifo.c 15-08-04  8月:08:1438655353 
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
#include <errno.h>
#include <assert.h>

#include "fifo.h"

#define AVERROR(x)		(-(x))

#define min(x, y)	((x) < (y) ? (x) : (y))
#define max(x, y)	((x) > (y) ? (x) : (y))

JFifo *j_fifo_alloc(unsigned int size)
{
	JFifo *f = malloc(sizeof(JFifo));
	if (!f)
		return NULL;
	memset(f, 0, sizeof(JFifo));

	f->buffer = malloc(size);
	f->end    = f->buffer + size;
	j_fifo_reset(f);
	if (!f->buffer) {
		free(f);
		return NULL;
	}
	return f;
}

void j_fifo_free(JFifo *f)
{
    if (f) {
        free(f->buffer);
        free(f);
    }
}

void j_fifo_reset(JFifo *f)
{
    f->wptr = f->rptr = f->buffer;
    f->wndx = f->rndx = 0;
}

int j_fifo_size(JFifo *f)
{
    return (uint32_t)(f->wndx - f->rndx);
}

int j_fifo_space(JFifo *f)
{
    return f->end - f->buffer - j_fifo_size(f);
}

int j_fifo_realloc2(JFifo *f, unsigned int new_size)
{
    unsigned int old_size = f->end - f->buffer;

    if (old_size < new_size) {
        int len          = j_fifo_size(f);
        JFifo *f2 = j_fifo_alloc(new_size);

        if (!f2)
            return AVERROR(ENOMEM);
        j_fifo_generic_read(f, f2->buffer, len, NULL);
        f2->wptr += len;
        f2->wndx += len;
        free(f->buffer);
        *f = *f2;
        free(f2);
    }
    return 0;
}

int j_fifo_grow(JFifo *f, unsigned int size)
{
    unsigned int old_size = f->end - f->buffer;
    if(size + (unsigned)j_fifo_size(f) < size)
        return AVERROR(EINVAL);

    size += j_fifo_size(f);

    if (old_size < size)
        return j_fifo_realloc2(f, max(size, 2*size));
    return 0;
}

/* src must NOT be const as it can be a context for func that may need
 * updating (like a pointer or byte counter) */
int j_fifo_generic_write(JFifo *f, void *src, int size,
                          int (*func)(void *, void *, int))
{
    int total = size;
    uint32_t wndx= f->wndx;
    uint8_t *wptr= f->wptr;

    do {
        int len = min(f->end - wptr, size);
        if (func) {
            if (func(src, wptr, len) <= 0)
                break;
        } else {
            memcpy(wptr, src, len);
            src = (uint8_t *)src + len;
        }
// Write memory barrier needed for SMP here in theory
        wptr += len;
        if (wptr >= f->end)
            wptr = f->buffer;
        wndx    += len;
        size    -= len;
    } while (size > 0);
    f->wndx= wndx;
    f->wptr= wptr;
    return total - size;
}

int j_fifo_generic_read(JFifo *f, void *dest, int buf_size,
                         void (*func)(void *, void *, int))
{
// Read memory barrier needed for SMP here in theory
    do {
        int len = min(f->end - f->rptr, buf_size);
        if (func)
            func(dest, f->rptr, len);
        else {
            memcpy(dest, f->rptr, len);
            dest = (uint8_t *)dest + len;
        }
// memory barrier needed for SMP here in theory
        j_fifo_drain(f, len);
        buf_size -= len;
    } while (buf_size > 0);
    return buf_size;
}

/** Discard data from the FIFO. */
void j_fifo_drain(JFifo *f, int size)
{
    f->rptr += size;
    if (f->rptr >= f->end)
        f->rptr -= f->end - f->buffer;
    f->rndx += size;
}

#ifdef TEST

int main(void)
{
    /* create a FIFO buffer */
    JFifo *fifo = j_fifo_alloc(13 * sizeof(int));
    int i, j, n;

    /* fill data */
    for (i = 0; j_fifo_space(fifo) >= sizeof(int); i++)
        j_fifo_generic_write(fifo, &i, sizeof(int), NULL);

    /* peek at FIFO */
    n = j_fifo_size(fifo) / sizeof(int);
    for (i = -n + 1; i < n; i++) {
        int *v = (int *)j_fifo_peek2(fifo, i * sizeof(int));
        printf("%d: %d\n", i, *v);
    }
    printf("\n");

    /* read data */
    for (i = 0; j_fifo_size(fifo) >= sizeof(int); i++) {
        j_fifo_generic_read(fifo, &j, sizeof(int), NULL);
        printf("%d ", j);
    }
    printf("\n");

    j_fifo_free(fifo);

    return 0;
}

#endif
