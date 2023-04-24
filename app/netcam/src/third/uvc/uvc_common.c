/*
 * UVC gadget test application
 *
 * Copyright (C) 2010 Ideas on board SPRL <laurent.pinchart@ideasonboard.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 */

#include <sys/time.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>

#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include <linux/usb/ch9.h>
#include <linux/usb/video.h>
#include <linux/videodev2.h>

//#include "tbox/tbox.h"

#include "adi_venc.h"

//#include "3.4.43/drivers/usb/gadget/uvc.h"
#include "uvc.h"


#define clamp(val, min, max) ({                 \
        typeof(val) __val = (val);              \
        typeof(min) __min = (min);              \
        typeof(max) __max = (max);              \
        (void) (&__val == &__min);              \
        (void) (&__val == &__max);              \
        __val = __val < __min ? __min: __val;   \
        __val > __max ? __max: __val; })

#define ARRAY_SIZE(a)	((sizeof(a) / sizeof(a[0])))

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

typedef struct __frame_head_t
{
    unsigned char ch;
    unsigned char resolution;    /*分辨率*/
    unsigned char frame_type;    /*帧类型 1: i frame 2:p frame 3:audio*/
    unsigned char frame_rate;    /*帧率*/
    unsigned long frame_size;    /*帧长度*/
    unsigned long sec;
    unsigned long usec;
	int  frame_no;
	char reserve[4];
}frame_head_t;

struct uvc_device *
uvc_open(const char *devname)
{
	struct uvc_device *dev;
	struct v4l2_capability cap;
	int ret;
	int fd;

	fd = open(devname, O_RDWR | O_NONBLOCK);
	if (fd == -1) {
		printf("v4l2 open failed: %s (%d)\n", strerror(errno), errno);
		return NULL;
	}

	printf("open succeeded, file descriptor = %d\n", fd);

	ret = ioctl(fd, VIDIOC_QUERYCAP, &cap);
	if (ret < 0) {
		printf("unable to query device: %s (%d)\n", strerror(errno),
			errno);
		close(fd);
		return NULL;
        }

	printf("device is %s on bus %s\n", cap.card, cap.bus_info);

	dev = malloc(sizeof(struct uvc_device));
	if (dev == NULL) {
		close(fd);
		return NULL;
	}

	memset(dev, 0, sizeof(struct uvc_device));
	dev->fd = fd;

	return dev;
}

void
uvc_close(struct uvc_device *dev)
{
	close(dev->fd);
	free(dev->imgdata);
	free(dev->mem);
	free(dev);
}

/* ---------------------------------------------------------------------------
 * Video streaming
 */
//extern void *get_mjpeg_info();
//extern void *get_stream_info();

static void uvc_video_fill_buffer(struct uvc_device *dev, struct v4l2_buffer *buf)
{
#if 0
	unsigned int bpl;
	//unsigned int i;
	static int i = 0;
	GADI_VENC_StreamT *info = NULL;

	static struct timeval ti = {0,0}, to;
#if 0
	printf("fcc                = 0x%x \n", dev->fcc);
	printf("V4L2_PIX_FMT_YUYV  = 0x%x\n", V4L2_PIX_FMT_YUYV);
	printf("V4L2_PIX_FMT_MJPEG = 0x%x\n", V4L2_PIX_FMT_MJPEG);
	printf("V4L2_PIX_FMT_H264  = 0x%x\n", V4L2_PIX_FMT_H264);
#endif
//printf("[%s] %d\n", __func__, __LINE__);
	switch (dev->fcc) {
	case V4L2_PIX_FMT_YUYV:
		/* Fill the buffer with video data. */
#if 0
		bpl = dev->width * 2;
		for (i = 0; i < dev->height; ++i)
			memset(dev->mem[buf->index] + i*bpl, dev->color++, bpl);

		buf->bytesused = bpl * dev->height;
#else
		memcpy(dev->mem[buf->index], dev->imgdata, dev->imgsize);
		buf->bytesused = dev->imgsize;
		printf("Send YUV img size=%d\n", buf->bytesused);
#endif
		break;


	case V4L2_PIX_FMT_MJPEG:
		gettimeofday(&to, NULL);
		//if((to.tv_sec-ti.tv_sec)*1000000+(to.tv_usec-ti.tv_usec) >= 40000)
		{
			ti.tv_sec = to.tv_sec;
			ti.tv_usec = to.tv_usec;

			do{
				info = (GADI_VENC_StreamT *)get_mjpeg_info();
				//printf("get info...\n");
				tb_coroutine_sleep(10);
			}while(info == 0);

			//printf("info = 0x%x\n", info);
			//printf("get the info: num = %u  addr = 0x%x  id = %d  size = %d  \n", info->frame_num, info->addr, info->stream_id, info->size);

			memcpy(dev->mem[buf->index], info->addr, info->size);

			buf->bytesused = info->size;

			if(info) free(info); //释放
			info = NULL;
		}
		//else
		{
			//buf->bytesused = 0;
		}
		break;
	case V4L2_PIX_FMT_H264:
		{
			do{
				info = (GADI_VENC_StreamT *)get_stream_info();
				//printf("get info...\n");
				tb_coroutine_sleep(10);
			}while(info == 0);

			//printf("fill: num = %u  addr = 0x%x  id = %d  size = %d index = %d \n", info->frame_num, info->addr, info->stream_id, info->size, buf->index);
			memcpy(dev->mem[buf->index], info->addr, info->size);

			buf->bytesused = info->size;

			if(info) free(info);
			info = NULL;

			break;
		}
	}
#endif
}


static int
uvc_video_process(struct uvc_device *dev)
{
	struct v4l2_buffer buf;
	int ret;

	memset(&buf, 0, sizeof buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	buf.memory = V4L2_MEMORY_MMAP;

//printf("[%s] %d\n", __func__, __LINE__);
	//printf("video process....\n");

	if ((ret = ioctl(dev->fd, VIDIOC_DQBUF, &buf)) < 0) {
		printf("Unable to dequeue buffer: %s (%d).\n", strerror(errno),
			errno);
		return ret;
	}

	uvc_video_fill_buffer(dev, &buf);

	if ((ret = ioctl(dev->fd, VIDIOC_QBUF, &buf)) < 0) {
		printf("Unable to requeue buffer: %s (%d).\n", strerror(errno),
			errno);
		return ret;
	}

	return 0;
}




static int
uvc_video_reqbufs(struct uvc_device *dev, int nbufs)
{
	struct v4l2_requestbuffers rb;
	struct v4l2_buffer buf;
	unsigned int i;
	int ret;
//printf("[%s] %d\n", __func__, __LINE__);
	for (i = 0; i < dev->nbufs; ++i)
		munmap(dev->mem[i], dev->bufsize);

	free(dev->mem);
	dev->mem = 0;
	dev->nbufs = 0;

	memset(&rb, 0, sizeof rb);
	rb.count = nbufs;
	rb.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	rb.memory = V4L2_MEMORY_MMAP;

	ret = ioctl(dev->fd, VIDIOC_REQBUFS, &rb);
	if (ret < 0) {
		printf("Unable to allocate buffers: %s (%d).\n",
			strerror(errno), errno);
		return ret;
	}

	printf("%u buffers allocated.\n", rb.count);

	/* Map the buffers. */
	dev->mem = malloc(rb.count * sizeof dev->mem[0]);

	for (i = 0; i < rb.count; ++i) {
		memset(&buf, 0, sizeof buf);
		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		buf.memory = V4L2_MEMORY_MMAP;
		ret = ioctl(dev->fd, VIDIOC_QUERYBUF, &buf);
		if (ret < 0) {
			printf("Unable to query buffer %u: %s (%d).\n", i,
				strerror(errno), errno);
			return -1;
		}
		printf("length: %u offset: %u\n", buf.length, buf.m.offset);

		dev->mem[i] = mmap(0, buf.length, PROT_READ | PROT_WRITE, MAP_SHARED, dev->fd, buf.m.offset);
		if (dev->mem[i] == MAP_FAILED) {
			printf("Unable to map buffer %u: %s (%d)\n", i,
				strerror(errno), errno);
			return -1;
		}
		printf("Buffer %u mapped at address %p. len = %d\n", i, dev->mem[i], buf.length);
	}

	dev->bufsize = buf.length;
	dev->nbufs = rb.count;

	return 0;
}

static int
uvc_video_stream(struct uvc_device *dev, int enable)
{
	struct v4l2_buffer buf;
	unsigned int i;
	int type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	int ret;
//printf("[%s] %d\n", __func__, __LINE__);
	if (!enable) {
		printf("Stopping video stream.\n");
		ioctl(dev->fd, VIDIOC_STREAMOFF, &type);
		return 0;
	}

	printf("Starting video stream.\n");

	for (i = 0; i < dev->nbufs; ++i) {
		memset(&buf, 0, sizeof buf);

		buf.index = i;
		buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
		buf.memory = V4L2_MEMORY_MMAP;

		printf("fill buffer...\n");
		uvc_video_fill_buffer(dev, &buf);

		printf("Queueing buffer %u.\n", i);
		if ((ret = ioctl(dev->fd, VIDIOC_QBUF, &buf)) < 0) {
			printf("Unable to queue buffer: %s (%d).\n",
				strerror(errno), errno);
			break;
		}
	}

	ioctl(dev->fd, VIDIOC_STREAMON, &type);
	return ret;
}

static int
uvc_video_set_format(struct uvc_device *dev)
{
	struct v4l2_format fmt;
	int ret;
//printf("[%s] %d\n", __func__, __LINE__);
	printf("Setting format to 0x%08x %ux%u\n",
		dev->fcc, dev->width, dev->height);

	memset(&fmt, 0, sizeof fmt);
	fmt.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	fmt.fmt.pix.width = dev->width;
	fmt.fmt.pix.height = dev->height;
	fmt.fmt.pix.pixelformat = dev->fcc;
	fmt.fmt.pix.field = V4L2_FIELD_NONE;
	//if (dev->fcc == V4L2_PIX_FMT_MJPEG)
		fmt.fmt.pix.sizeimage = 1024*1024;//dev->imgsize * 1.5;

	if ((ret = ioctl(dev->fd, VIDIOC_S_FMT, &fmt)) < 0)
		printf("Unable to set format: %s (%d).\n",
			strerror(errno), errno);

	return ret;
}

int
uvc_video_init(struct uvc_device *dev __attribute__((__unused__)))
{
	return 0;
}

/* ---------------------------------------------------------------------------
 * Request processing
 */

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

static struct uvc_frame_info uvc_frames_yuyv[] = {
	//{ 640, 360, { 50000000, 0 }, },//{  640, 360, { 666666, 10000000, 50000000, 0 }, },
	//{ 1280, 720, { 50000000, 0 }, },
	{ 1920, 1280, { 50000000, 0 }, },
	{ 0, 0, { 0, }, },
};

static struct uvc_frame_info uvc_frames_mjpeg[] = {
	//{ 640, 360, { 50000000, 0 }, },//{  640, 360, { 666666, 10000000, 50000000, 0 }, },
	//{ 1280, 720, { 50000000, 0 }, },
	{ 1920, 1280, { 50000000, 0 }, },
	{ 0, 0, { 0, }, },
};

static struct uvc_frame_info uvc_frames_h264[] =
{
//{ 1280, 720,
{ 1920, 1080,
{ 400000, 0 }, },
{ 640, 360,
{ 400000, 10000000, 5000000, 0 }, },
{ 0, 0,
{ 0, }, }, };

static struct uvc_format_info uvc_formats[] = {
	//{ V4L2_PIX_FMT_YUYV, uvc_frames_yuyv },
	//{ V4L2_PIX_FMT_MJPEG, uvc_frames_mjpeg },
	{ V4L2_PIX_FMT_H264, uvc_frames_h264 },
};

void uvc_format_set(u32 mode, u32 size)
{
    if(mode==0)
    {
        uvc_formats[0].fcc = V4L2_PIX_FMT_H264;
        if(size==0)
        {
            uvc_frames_h264[0].width    = 1920;
            uvc_frames_h264[0].height   = 1080;
        }
        else if(size==1)
        {
            uvc_frames_h264[0].width    = 1280;
            uvc_frames_h264[0].height   = 720;
        }
        else if(size==2)
        {
            uvc_frames_h264[0].width    = 640;
            uvc_frames_h264[0].height   = 360;
        }
        uvc_formats[0].frames = uvc_frames_h264;
    }
    else if(mode==1)
    {
        uvc_formats[0].fcc = V4L2_PIX_FMT_MJPEG;
        if(size==0)
        {
            uvc_frames_mjpeg[0].width    = 1920;
            uvc_frames_mjpeg[0].height   = 1080;
        }
        else if(size==1)
        {
            uvc_frames_mjpeg[0].width    = 1280;
            uvc_frames_mjpeg[0].height   = 720;
        }
        else if(size==2)
        {
            uvc_frames_mjpeg[0].width    = 640;
            uvc_frames_mjpeg[0].height   = 360;
        }
        uvc_formats[0].frames = uvc_frames_mjpeg;
    }
    else if(mode==2)
    {
        uvc_formats[0].fcc = V4L2_PIX_FMT_YUYV;
        if(size==0)
        {
            uvc_frames_yuyv[0].width    = 1920;
            uvc_frames_yuyv[0].height   = 1080;
        }
        else if(size==1)
        {
            uvc_frames_yuyv[0].width    = 1280;
            uvc_frames_yuyv[0].height   = 720;
        }
        else if(size==2)
        {
            uvc_frames_yuyv[0].width    = 640;
            uvc_frames_yuyv[0].height   = 360;
        }
        uvc_formats[0].frames = uvc_frames_yuyv;
    }
}

static void
uvc_fill_streaming_control(struct uvc_device *dev,
			   struct uvc_streaming_control *ctrl,
			   int iframe, int iformat)
{
	const struct uvc_format_info *format;
	const struct uvc_frame_info *frame;
	unsigned int nframes;
//printf("[%s] %d\n", __func__, __LINE__);
	if (iformat < 0)
		iformat = ARRAY_SIZE(uvc_formats) + iformat;
	if (iformat < 0 || iformat >= (int)ARRAY_SIZE(uvc_formats))
		return;
    printf("[%s %d]iformat=%d iframe=%d\n", __func__, __LINE__, iformat, iframe);
	format = &uvc_formats[iformat];

	nframes = 0;
	while (format->frames[nframes].width != 0)
		++nframes;

	if (iframe < 0)
		iframe = nframes + iframe;
	if (iframe < 0 || iframe >= (int)nframes)
		return;
	frame = &format->frames[iframe];

	memset(ctrl, 0, sizeof *ctrl);

	ctrl->bmHint = 1;
	ctrl->bFormatIndex = iformat + 1;
	ctrl->bFrameIndex = iframe + 1;
	ctrl->dwFrameInterval = frame->intervals[0];
	switch (format->fcc) {
	case V4L2_PIX_FMT_YUYV:
	   // printf("---[%s] %d\n", __func__, __LINE__);
		ctrl->dwMaxVideoFrameSize = frame->width * frame->height * 2;
		break;
	case V4L2_PIX_FMT_MJPEG:
	    //printf("+++[%s] %d\n", __func__, __LINE__);
		ctrl->dwMaxVideoFrameSize = 1024*1024;//dev->imgsize;
		break;
	case V4L2_PIX_FMT_H264:
		ctrl->dwMaxVideoFrameSize = 1024*1024;//frame->width * frame->height*2;
		break;
	}
	ctrl->dwMaxPayloadTransferSize = 512;	/* TODO this should be filled by the driver. */
	ctrl->bmFramingInfo = 3;
	ctrl->bPreferedVersion = 1;
	ctrl->bMaxVersion = 1;
}

static void
uvc_events_process_standard(struct uvc_device *dev, struct usb_ctrlrequest *ctrl,
			    struct uvc_request_data *resp)
{
	printf("standard request\n");
	(void)dev;
	(void)ctrl;
	(void)resp;
}

static void
uvc_events_process_control(struct uvc_device *dev, uint8_t req, uint8_t cs,
			   struct uvc_request_data *resp)
{
	printf("control request (req %02x cs %02x)\n", req, cs);
	(void)dev;
	(void)resp;
}

static void
uvc_events_process_streaming(struct uvc_device *dev, uint8_t req, uint8_t cs,
			     struct uvc_request_data *resp)
{
	struct uvc_streaming_control *ctrl;

	printf("streaming request (req %02x cs %02x)\n", req, cs);

	if (cs != UVC_VS_PROBE_CONTROL && cs != UVC_VS_COMMIT_CONTROL)
		return;

	ctrl = (struct uvc_streaming_control *)&resp->data;
	resp->length = sizeof *ctrl;

	switch (req) {
	case UVC_SET_CUR:
		dev->control = cs;
		resp->length = 34;
		break;

	case UVC_GET_CUR:
		if (cs == UVC_VS_PROBE_CONTROL)
			memcpy(ctrl, &dev->probe, sizeof *ctrl);
		else
			memcpy(ctrl, &dev->commit, sizeof *ctrl);
		break;

	case UVC_GET_MIN:
	case UVC_GET_MAX:
	case UVC_GET_DEF:
		uvc_fill_streaming_control(dev, ctrl, req == UVC_GET_MAX ? -1 : 0,
					   req == UVC_GET_MAX ? -1 : 0);
		break;

	case UVC_GET_RES:
		memset(ctrl, 0, sizeof *ctrl);
		break;

	case UVC_GET_LEN:
		//resp->data[0] = 0x00;
		//resp->data[1] = 0x22;
		resp->data[0] = 0x40;
		resp->data[1] = 0x00;
		resp->length = 2;
		break;

	case UVC_GET_INFO:
		resp->data[0] = 0x03;
		resp->length = 1;
		break;
	}
}

static void
uvc_events_process_class(struct uvc_device *dev, struct usb_ctrlrequest *ctrl,
			 struct uvc_request_data *resp)
{
	if ((ctrl->bRequestType & USB_RECIP_MASK) != USB_RECIP_INTERFACE)
		return;
//printf("[%s] %d\n", __func__, __LINE__);
	switch (ctrl->wIndex & 0xff) {
	case UVC_INTF_CONTROL:
		uvc_events_process_control(dev, ctrl->bRequest, ctrl->wValue >> 8, resp);
		break;

	case UVC_INTF_STREAMING:
		uvc_events_process_streaming(dev, ctrl->bRequest, ctrl->wValue >> 8, resp);
		break;

	default:
		break;
	}
}

static void
uvc_events_process_setup(struct uvc_device *dev, struct usb_ctrlrequest *ctrl,
			 struct uvc_request_data *resp)
{
	dev->control = 0;

	//printf("bRequestType %02x bRequest %02x wValue %04x wIndex %04x "
	//	"wLength %04x\n", ctrl->bRequestType, ctrl->bRequest,
	//	ctrl->wValue, ctrl->wIndex, ctrl->wLength);

	switch (ctrl->bRequestType & USB_TYPE_MASK) {
	case USB_TYPE_STANDARD:
		uvc_events_process_standard(dev, ctrl, resp);
		break;

	case USB_TYPE_CLASS:
		uvc_events_process_class(dev, ctrl, resp);
		break;

	default:
		break;
	}
}

static void
uvc_events_process_data(struct uvc_device *dev, struct uvc_request_data *data)
{
	struct uvc_streaming_control *target;
	struct uvc_streaming_control *ctrl;
	const struct uvc_format_info *format;
	const struct uvc_frame_info *frame;
	const unsigned int *interval;
	unsigned int iformat, iframe;
	unsigned int nframes;
//printf("[%s] %d\n", __func__, __LINE__);


	switch (dev->control) {
	case UVC_VS_PROBE_CONTROL:
		printf("setting probe control, length = %d\n", data->length);
		target = &dev->probe;
		break;

	case UVC_VS_COMMIT_CONTROL:
		printf("setting commit control, length = %d\n", data->length);
		target = &dev->commit;
		break;

	default:
		printf("setting unknown control, length = %d\n", data->length);
		return;
	}

	ctrl = (struct uvc_streaming_control *)&data->data;
	iformat = clamp((unsigned int)ctrl->bFormatIndex, 1U,
			(unsigned int)ARRAY_SIZE(uvc_formats));
	format = &uvc_formats[iformat-1];

	nframes = 0;
	while (format->frames[nframes].width != 0)
		++nframes;

	iframe = clamp((unsigned int)ctrl->bFrameIndex, 1U, nframes);
	frame = &format->frames[iframe-1];
	interval = frame->intervals;

	while (interval[0] < ctrl->dwFrameInterval && interval[1])
		++interval;

	target->bFormatIndex = iformat;
	target->bFrameIndex = iframe;
	switch (format->fcc) {
	case V4L2_PIX_FMT_YUYV:
	    printf("fmt yuv...\n");
		target->dwMaxVideoFrameSize = frame->width * frame->height * 2;
		break;
	case V4L2_PIX_FMT_H264:
	case V4L2_PIX_FMT_MJPEG:
	    printf("fmt mjpeg...\n");
		//if (dev->imgsize == 0)
		//	printf("WARNING: MJPEG requested and no image loaded.\n");
		target->dwMaxVideoFrameSize = 1024*1024;//dev->imgsize;
		break;
	}
	target->dwFrameInterval = *interval;
    printf("dwFrameInterval = %d  control = %d [%d]\n", target->dwFrameInterval, dev->control, UVC_VS_COMMIT_CONTROL);

	if (dev->control == UVC_VS_COMMIT_CONTROL) {
		dev->fcc = format->fcc;
		dev->width  = 1920;//frame->width;
		dev->height = 1080;//frame->height;

        //printf("[%s] %d\n", __func__, __LINE__);
		uvc_video_set_format(dev);
		if (dev->bulk)
			uvc_video_stream(dev, 1);
	}
}

static void
uvc_events_process(struct uvc_device *dev)
{
	struct v4l2_event v4l2_event;
	struct uvc_event *uvc_event = (void *)&v4l2_event.u.data;
	struct uvc_request_data resp;
	int ret;

//printf("[%s] %d\n", __func__, __LINE__);
	printf("event process.....\n");

	ret = ioctl(dev->fd, VIDIOC_DQEVENT, &v4l2_event);
	if (ret < 0) {
		printf("VIDIOC_DQEVENT failed: %s (%d)\n", strerror(errno),
			errno);
		return;
	}

	memset(&resp, 0, sizeof resp);
	resp.length = -EL2HLT;

	switch (v4l2_event.type) {
	case UVC_EVENT_CONNECT:
	case UVC_EVENT_DISCONNECT:
		return;

	case UVC_EVENT_SETUP:
		printf("---- SETUP ----\n");
		uvc_events_process_setup(dev, &uvc_event->req, &resp);
		break;

	case UVC_EVENT_DATA:
		printf("---- DATA ----\n");
		struct uvc_request_data *data = &uvc_event->data;
		uvc_events_process_data(dev, &uvc_event->data);
		return;

	case UVC_EVENT_STREAMON:
		printf("---- STREAMON ----\n");
		//if (!dev->streamon)
		{
		//	uvc_video_stream(dev, 0);
		//	uvc_video_reqbufs(dev, 0);
		}
		//dev->streamon = 1;
		uvc_video_reqbufs(dev, 1);
		uvc_video_stream(dev, 1);
		break;

	case UVC_EVENT_STREAMOFF:
		printf("---- STREAMOFF ----\n");
		//dev->streamon = 0;
		uvc_video_stream(dev, 0);
		uvc_video_reqbufs(dev, 0);
		break;
	}

	ioctl(dev->fd, UVCIOC_SEND_RESPONSE, &resp);
	if (ret < 0) {
		printf("UVCIOC_S_EVENT failed: %s (%d)\n", strerror(errno),
			errno);
		return;
	}
}

void
uvc_events_init(struct uvc_device *dev)
{
	struct v4l2_event_subscription sub;
//printf("[%s] %d\n", __func__, __LINE__);
	uvc_fill_streaming_control(dev, &dev->probe, 0, 0);
	uvc_fill_streaming_control(dev, &dev->commit, 0, 0);

	if (dev->bulk) {
		/* FIXME Crude hack, must be negotiated with the driver. */
		dev->probe.dwMaxPayloadTransferSize = 16 * 1024;
		dev->commit.dwMaxPayloadTransferSize = 16 * 1024;
	}


	memset(&sub, 0, sizeof sub);
	sub.type = UVC_EVENT_SETUP;
	ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);

	sub.type = UVC_EVENT_DATA;
	ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);

	sub.type = UVC_EVENT_STREAMON;
	ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);

	sub.type = UVC_EVENT_STREAMOFF;
	ioctl(dev->fd, VIDIOC_SUBSCRIBE_EVENT, &sub);
}

/* ---------------------------------------------------------------------------
 * main
 */

static void image_load(struct uvc_device *dev, const char *img)
{
	int fd = -1;

	if (img == NULL)
		return;

	fd = open(img, O_RDONLY);
	if (fd == -1) {
		printf("Unable to open MJPEG image '%s'\n", img);
		return;
	}

	dev->imgsize = lseek(fd, 0, SEEK_END);
	printf("imgsize is %d\n", dev->imgsize);
	lseek(fd, 0, SEEK_SET);
	dev->imgdata = malloc(dev->imgsize);
	if (dev->imgdata == NULL) {
		printf("Unable to allocate memory for MJPEG image\n");
		dev->imgsize = 0;
		return;
	}

	read(fd, dev->imgdata, dev->imgsize);
	close(fd);
}

static void usage(const char *argv0)
{
	fprintf(stderr, "Usage: %s [options]\n", argv0);
	fprintf(stderr, "Available options are\n");
	fprintf(stderr, " -b		Use bulk mode\n");
	fprintf(stderr, " -d device	Video device\n");
	fprintf(stderr, " -h		Print this help screen and exit\n");
	fprintf(stderr, " -i image	MJPEG image\n");
}

int test_main(int argc, char *argv[])
{
	char *device = "/dev/video0";
	struct uvc_device *dev;
	int bulk_mode = 0;
	char *mjpeg_image = NULL;
	fd_set fds;
	int ret, opt;

	while ((opt = getopt(argc, argv, "bd:hi:")) != -1) {
		switch (opt) {
		case 'b':
			bulk_mode = 1;
			break;

		case 'd':
			device = optarg;
			break;

		case 'h':
			usage(argv[0]);
			return 0;

		case 'i':
			mjpeg_image = optarg;
			break;

		default:
			fprintf(stderr, "Invalid option '-%c'\n", opt);
			usage(argv[0]);
			return 1;
		}
	}

	dev = uvc_open(device);
	if (dev == NULL)
		return 1;

	image_load(dev, mjpeg_image);

	dev->bulk = bulk_mode;

	uvc_events_init(dev);
	uvc_video_init(dev);

	FD_ZERO(&fds);
	FD_SET(dev->fd, &fds);

	while (1) {
		fd_set efds = fds;
		fd_set wfds = fds;

		ret = select(dev->fd + 1, NULL, &wfds, &efds, NULL);
		if (FD_ISSET(dev->fd, &efds))
			uvc_events_process(dev);

		if (FD_ISSET(dev->fd, &wfds))
			uvc_video_process(dev);
	}

	uvc_close(dev);
	return 0;
}

#if 0
static tb_poller_ref_t dev_poller = 0;

static tb_void_t tb_co_dev_io_events(tb_poller_ref_t poller, tb_socket_ref_t sock, tb_size_t events, tb_cpointer_t priv)
{
	struct uvc_device *dev = (struct uvc_device*)priv;

	if (events & TB_POLLER_EVENT_RECV)
		uvc_events_process(dev);

	if (events & TB_POLLER_EVENT_SEND)
		uvc_video_process(dev);
}

tb_void_t coroutine_uvc(tb_cpointer_t priv)
{
	tb_size_t count = (tb_size_t)priv;
	char *device = "/dev/video0";
	struct uvc_device *dev;
	int bulk_mode = 0;
	char *mjpeg_image = NULL;
	fd_set fds;
	int ret, opt;

	dev = uvc_open(device);
	if (dev == NULL)
		return 1;

	dev->bulk = bulk_mode;

	uvc_events_init(dev);
	uvc_video_init(dev);

	FD_ZERO(&fds);
	FD_SET(dev->fd, &fds);
	struct timeval timeout;

	fcntl(dev->fd, O_NONBLOCK);

	dev_poller = tb_poller_init(dev);
	tb_assert(dev_poller);

	while (1)
    {

	#if 1
		fd_set efds = fds;
		fd_set wfds = fds;

		timeout.tv_sec = 0;
		timeout.tv_usec = 100;

		//printf("uvc ...\n");
		tb_coroutine_sleep(1);

		ret = select(dev->fd + 1, NULL, &wfds, &efds, &timeout);
		if (FD_ISSET(dev->fd, &efds))
			uvc_events_process(dev);

		if (FD_ISSET(dev->fd, &wfds))
			uvc_video_process(dev);
	#else
		if (tb_poller_wait(dev_poller, tb_co_dev_io_events, 1) < 0) break;
		tb_coroutine_yield();
	#endif


		//tb_coroutine_yield();

    }
}
#endif

