/*!
*****************************************************************************
** \file      $gkprjzfifo.h
**
** \version	$id: zfifo.h 15-08-04  8月:08:1438654894
**
** \brief
**
** \attention   this code is provided as is. goke microelectronics
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#ifndef _ZFIFO_H_
#define _ZFIFO_H_


typedef struct _ZFIFO_ {
	char *name;
	void *buf;
	int buf_size;

	int first_index;
	int first_offset;

	int last_index;
	int last_offset;

	int flag_index;
	int flag_offset;

	int end_offset;
    int deadline_offset;

    pthread_mutex_t mutex;
    pthread_cond_t  cond;
} ZFIFO;

typedef struct _ZFIFO_DESC_ {
	ZFIFO *zfifo;

	int index;
	int offset;

    void *frame_data;
    int frame_len;
	unsigned int timeout; //seconds
} ZFIFO_DESC;



typedef struct _NODE_HEADER_ {
	int index;
	int type;
    int size;
} NODE_HEADER;

#define FRAME_INIT_LEN (102400)
#define FRAME_LEN_ADD  (30000)

ZFIFO *zfifo_init(const char* zfifo_name, int size);
void zfifo_uninit(ZFIFO *zfifo);
ZFIFO_DESC *zfifo_open(ZFIFO *fifo);
void zfifo_close(ZFIFO_DESC *zfifo_desc);
int zfifo_writev(ZFIFO_DESC *zfifo_desc, const ZFIFO_NODE *iov, int iovcnt);
int zfifo_readv(ZFIFO_DESC *zfifo_desc, ZFIFO_NODE *iov, int iovcnt, int timeout);
int zfifo_set_newest_frame(ZFIFO_DESC *zfifo_desc);
int zfifo_set_oldest_frame(ZFIFO_DESC *zfifo_desc);


int zfifo_writev_plus(ZFIFO_DESC *zfifo_desc, const ZFIFO_NODE *iov, int iovcnt, int flag);
int zfifo_readv_plus(ZFIFO_DESC *zfifo_desc, ZFIFO_NODE *iov, int iovcnt, int timeout);
int zfifo_readv_next_plus(ZFIFO_DESC *zfifo_desc, ZFIFO_NODE *iov, int iovcnt, int timeout);
int zfifo_readv_flag_plus(ZFIFO_DESC *zfifo_desc, ZFIFO_NODE *iov, int iovcnt, int timeout);
int zfifo_get_leave_frame(ZFIFO_DESC *zfifo_desc);
#endif
