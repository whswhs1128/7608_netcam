/*!
*****************************************************************************
** \file        
**
** \brief       
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/


#ifndef _UVC_YST_H_
#define _UVC_YST_H_



#ifdef __cplusplus
extern "C" {
#endif

struct uvc_device
{
	int fd;

	struct uvc_streaming_control probe;
	struct uvc_streaming_control commit;

	int control;
	int streamon;

	unsigned int fcc;
	unsigned int width;
	unsigned int height;

	void **mem;
	unsigned int nbufs;
	unsigned int bufsize;

	unsigned int bulk;
	uint8_t color;
	unsigned int imgsize;
	void *imgdata;
};


struct uvc_frame_info
{
	unsigned int width;
	unsigned int height;
	unsigned int intervals[8];
};

struct uvc_format_info
{
	unsigned int fcc;
	struct uvc_frame_info *frames;
};


#ifdef __cplusplus
    }
#endif


#endif /* _UVC_H_ */


