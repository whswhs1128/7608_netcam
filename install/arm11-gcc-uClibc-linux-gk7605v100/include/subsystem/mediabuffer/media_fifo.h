/*!
*****************************************************************************
** FileName     : media_fifo.h
**
** Description  : media fifo
**
** Author       : hyb <heyongbin@gokemicro.com>
** Date         : 2015-7-29 create it
** Author       : bruce <zhaoquanfeng@gokemicro.com>
** Date         : 2015-8-9 modify it
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _MEDIA_FIFO_H_
#define _MEDIA_FIFO_H_

#ifdef __cplusplus
extern "C" {
#endif

#define VENC_MAX_STREAM_NUM 4
#define MAX_ONE_FRAME_SIZE 700000

typedef void* MEDIABUF_HANDLE;

typedef struct _ZFIFO_NODE_ {
    void *base;
    int len;
} ZFIFO_NODE;

typedef enum {
    GK_NET_STREAM_TYPE_STREAM0 = 0,
    GK_NET_STREAM_TYPE_STREAM1,
    GK_NET_STREAM_TYPE_STREAM2,
    GK_NET_STREAM_TYPE_STREAM3,
} GK_NET_STREAM_TYPE;

typedef enum {
    GK_NET_FRAME_TYPE_I = 0,
    GK_NET_FRAME_TYPE_P = 1,
    GK_NET_FRAME_TYPE_A = 2,
    GK_NET_FRAME_TYPE_JPEG = 3,
} GK_NET_FRAME_TYPE;


typedef enum MEDIA_AVCodecID {
    /* video codecs */
    MEDIA_CODEC_ID_MJPEG      = 5,
    MEDIA_CODEC_ID_H264       = 0,
    MEDIA_CODEC_ID_HEVC       = 6,
    MEDIA_CODEC_ID_H265       = MEDIA_CODEC_ID_HEVC,

    /* various PCM "codecs" */
    MEDIA_CODEC_ID_PCM        = 3,
    MEDIA_CODEC_ID_PCM_MULAW  = 2,
    MEDIA_CODEC_ID_PCM_ALAW   = 1,

    /* various ADPCM codecs */
    MEDIA_CODEC_ID_ADPCM_G726 = 4,
    MEDIA_CODEC_ID_NONE,
}MEDIA_AV_CODEC_ID;
#define MAGIC_TEST  0XF3B0A4B8

/*! Media buffer frame header */
typedef struct {
    unsigned int magic;         /* MAGIC_TEST */
    unsigned int device_type;   /* user defined */
    unsigned int media_codec_type;
    unsigned int frame_size;    /* frame size */
    unsigned int frame_no;      /* frame number */
    unsigned int video_reso;    /* Video resolution.video_reso=(width << 16) + height */
    unsigned char reserved;     /* user defined */
    unsigned char frame_type;   /* frame type  GK_NET_FRAME_TYPE*/
    unsigned char frame_rate;   /* fps */
    unsigned char video_standard; /*video format*/
    unsigned int sec;             /*second of frame*/
    unsigned int usec;            /*usecond of frame*/
    unsigned int pts;             /*pts of frame*/

} GK_NET_FRAME_HEADER;


/*!
*******************************************************************************
** \brief Media buffer init
**
** \param[in]  fifo_id    fifo id of GK_NET_STREAM_TYPE
** \param[in]  size       size of medeia buffer
**
** \return
** - #0         ok
** - #other     failed
**
**
*******************************************************************************
*/

int mediabuf_init(int fifo_id, int size);

/*!
*******************************************************************************
** \brief Media buffer uninit
**
** \param[in]  fifo_id    fifo id of GK_NET_STREAM_TYPE
**
** \return
** - #0         ok
** - #other     failed
**
*******************************************************************************
*/
int mediabuf_uninit(int fifo_id);


/*!
*******************************************************************************
** \brief Media buffer add one writer for write stream frame by type
**
** \param[in]  size       stream type of reader
**
** \return
** - # MEDIABUF_HANDLE      writer's handle
** - # 0                    failed
**
*******************************************************************************
*/
MEDIABUF_HANDLE mediabuf_add_writer(GK_NET_STREAM_TYPE type);

/*!
*******************************************************************************
** \brief Media buffer delete one writer
**
** \param[in]  size       writer
**
** \return
**
*******************************************************************************
*/
void mediabuf_del_writer(MEDIABUF_HANDLE writerid);

/*!
*******************************************************************************
** \brief Media buffer write one frame data and header info
**
** \param[in]       writerid       writer's handle
** \param[in]       data           pointer of input frame,
** \param[in]       size           length of frame
** \param[out]      header         frame info
**
** \return
** - # < 0      write failed
** - # >=0      success
**
*******************************************************************************
*/
int mediabuf_write_frame(MEDIABUF_HANDLE writerid, void *data, int size, GK_NET_FRAME_HEADER *header);


/*!
*******************************************************************************
** \brief Media buffer add one reader for read stream by type
**
** \param[in]  size       stream type of reader
**
** \return
** - # MEDIABUF_HANDLE      reader's handle
** - # 0                    failed
**
*******************************************************************************
*/

MEDIABUF_HANDLE mediabuf_add_reader(GK_NET_STREAM_TYPE type);

/*!
*******************************************************************************
** \brief Media buffer delete one reader
**
** \param[in]  readerid       reader's handle
**
** \return
**
*******************************************************************************
*/
void mediabuf_del_reader(MEDIABUF_HANDLE readerid);


/*!
*******************************************************************************
** \brief read one frame from reader,timeout 1 second,  <= 0 failed , > 0 success
**
** \param[in]       readerid       reader's handle
** \param[in/out]   data           pointer of frame
**                      Attation:
**                          1. if input value of *data is not NULL, use buffer of caller.
**                          2.if input value of *data is NULL, use share buffer of mediabuf'size,
**                              the output value of *data is the start pointer of buffer.
**
** \param[in/out]   size            length of frame
**                      Attation:
**                        1.if input value of *size is not equal 0, it means buffer length of caller,
**                          if the frame'length > *size,it only copy *size data to caller's buffer.
**                        2.if input value of *size is equal 0, use buffer of mediabuf,
**                          the output value of *size is the size of frame.
**
** param[out]       header          frame info if success
**
** \return
** - # <= 0      read failed
** - # >0        success
*/
int mediabuf_read_frame(MEDIABUF_HANDLE readerid, void **data, int *size, GK_NET_FRAME_HEADER *header);

/*!
*******************************************************************************
** \brief read one newest frame from reader, <=0 failed , >0 success
**        default timeout 5 seconds
**
** \param[in]       readerid       reader's handle
** \param[in/out]   data           pointer of frame,
**                      Attation:
**                          1. if input value of *data is not NULL, use buffer of caller.
**                          2.if input value of *data is NULL, use share buffer of mediabuf'size,
**                              the output value of *data is the start pointer of buffer.
**
** \param[in/out]   size            length of frame
**                      Attation:
**                        1.if input value of *size is not equal 0, it means buffer length of caller,
**                          if the frame'length > *size,it only copy *size data to caller's buffer.
**                        2.if input value of *size is equal 0, use buffer of mediabuf,
**                          the output value of *size is the size of frame.
**
** param[out]       header          frame info if success
**
** \return
** - # <= 0      read failed, < 0 error, =0,  time out
** - # >0        success
*/
int mediabuf_read_I_frame(MEDIABUF_HANDLE readerid, void **data, int *size, GK_NET_FRAME_HEADER *header);


/*!
*******************************************************************************
** \brief read one I  oldest frame from reader,<=0 failed , >0 success
**           default timeout 5 seconds
**
** \param[in]       readerid       reader's handle
** \param[in/out]   data           pointer of frame,
**                      Attation:
**                          1. if input value of *data is not NULL, use buffer of caller.
**                          2.if input value of *data is NULL, use share buffer of mediabuf'size,
**                              the output value of *data is the start pointer of buffer.
**
** \param[in/out]   size            length of frame
**                      Attation:
**                        1.if input value of *size is not equal 0, it means buffer length of caller,
**                          if the frame'length > *size,it only copy *size data to caller's buffer.
**                        2.if input value of *size is equal 0, use buffer of mediabuf,
**                          the output value of *size is the size of frame.
**
** param[out]       header          frame info if success
**
** \return
** - # <= 0      read failed, < 0 error, =0,  time out
** - # >0        success
*/

int mediabuf_read_next_I_frame(MEDIABUF_HANDLE readerid,  void **data, int *size, GK_NET_FRAME_HEADER *header);


/*!
*******************************************************************************
** \brief set reader to read newset frame
**
** \param[in]       readerid       reader's handle
**
** \return
** - # other    failed
** - # 0        success
*/
int mediabuf_set_newest_frame(MEDIABUF_HANDLE readerid);


/*!
*******************************************************************************
** \brief set reader to read oldest frame
**
** \param[in]   readerid    reader's handle
**
** \return
** - # other    failed
** - # 0        success
*/
int mediabuf_set_oldest_frame(MEDIABUF_HANDLE readerid);

/*!
*******************************************************************************
** \brief get reader's leave frame
**
** \param[in]   readerid    reader's handle
**
** \return
** - # other    failed
** - # 0        success
*/

int mediabuf_get_leave_frame(MEDIABUF_HANDLE handle);

/*!
*******************************************************************************
** \brief set reader to read I frame timeout
**
** \param[in]   readerid    reader's handle
** \param[in]   timeout_sec    reader I frame timeout, second, default no timeout

**  
** \return
** - # other    failed
** - # 0        success
*/

int mediabuf_set_I_frame_timeout(MEDIABUF_HANDLE reader,unsigned int timeout_sec);

#ifdef __cplusplus
};
#endif
#endif


