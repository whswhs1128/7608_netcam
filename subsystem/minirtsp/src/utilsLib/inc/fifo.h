/*!
*****************************************************************************
** \file      $gkprjfifo.h
**
** \version	$id: fifo.h 15-08-04  8月:08:1438655491 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#ifndef __FIFO_H__
#define __FIFO_H__

#include <stdint.h>

typedef struct JFifo {
    uint8_t *buffer;
    uint8_t *rptr, *wptr, *end;
    uint32_t rndx, wndx;
} JFifo;

/**
 * Initialize an JFifo.
 * @param size of FIFO
 * @return JFifo or NULL in case of memory allocation failure
 */
JFifo *j_fifo_alloc(unsigned int size);

/**
 * Free an JFifo.
 * @param f JFifo to free
 */
void j_fifo_free(JFifo *f);

/**
 * Reset the JFifo to the state right after j_fifo_alloc, in particular it is emptied.
 * @param f JFifo to reset
 */
void j_fifo_reset(JFifo *f);

/**
 * Return the amount of data in bytes in the JFifo, that is the
 * amount of data you can read from it.
 * @param f JFifo to read from
 * @return size
 */
int j_fifo_size(JFifo *f);

/**
 * Return the amount of space in bytes in the JFifo, that is the
 * amount of data you can write into it.
 * @param f JFifo to write into
 * @return size
 */
int j_fifo_space(JFifo *f);

/**
 * Feed data from an JFifo to a user-supplied callback.
 * @param f JFifo to read from
 * @param buf_size number of bytes to read
 * @param func generic read function
 * @param dest data destination
 */
int j_fifo_generic_read(JFifo *f, void *dest, int buf_size, void (*func)(void*, void*, int));

/**
 * Feed data from a user-supplied callback to an JFifo.
 * @param f JFifo to write to
 * @param src data source; non-const since it may be used as a
 * modifiable context by the function defined in func
 * @param size number of bytes to write
 * @param func generic write function; the first parameter is src,
 * the second is dest_buf, the third is dest_buf_size.
 * func must return the number of bytes written to dest_buf, or <= 0 to
 * indicate no more data available to write.
 * If func is NULL, src is interpreted as a simple byte array for source data.
 * @return the number of bytes written to the FIFO
 */
int j_fifo_generic_write(JFifo *f, void *src, int size, int (*func)(void*, void*, int));

/**
 * Resize an JFifo.
 * In case of reallocation failure, the old FIFO is kept unchanged.
 *
 * @param f JFifo to resize
 * @param size new JFifo size in bytes
 * @return <0 for failure, >=0 otherwise
 */
int j_fifo_realloc2(JFifo *f, unsigned int size);

/**
 * Enlarge an JFifo.
 * In case of reallocation failure, the old FIFO is kept unchanged.
 * The new fifo size may be larger than the requested size.
 *
 * @param f JFifo to resize
 * @param additional_space the amount of space in bytes to allocate in addition to j_fifo_size()
 * @return <0 for failure, >=0 otherwise
 */
int j_fifo_grow(JFifo *f, unsigned int additional_space);

/**
 * Read and discard the specified amount of data from an JFifo.
 * @param f JFifo to read from
 * @param size amount of data to read in bytes
 */
void j_fifo_drain(JFifo *f, int size);

/**
 * Return a pointer to the data stored in a FIFO buffer at a certain offset.
 * The FIFO buffer is not modified.
 *
 * @param f    JFifo to peek at, f must be non-NULL
 * @param offs an offset in bytes, its absolute value must be less
 *             than the used buffer size or the returned pointer will
 *             point outside to the buffer data.
 *             The used buffer size can be checked with j_fifo_size().
 */
static inline uint8_t *j_fifo_peek2(const JFifo *f, int offs)
{
    uint8_t *ptr = f->rptr + offs;
    if (ptr >= f->end)
        ptr = f->buffer + (ptr - f->end);
    else if (ptr < f->buffer)
        ptr = f->end - (f->buffer - ptr);
    return ptr;
}

#endif /* __FIFO_H__ */
