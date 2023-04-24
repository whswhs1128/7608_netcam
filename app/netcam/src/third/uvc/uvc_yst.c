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
#include "media_fifo.h"
#include "common.h"


//#include "3.4.43/drivers/usb/gadget/uvc.h"
#include "uvc.h"
#include "uvc_yst.h"

static int uvc_mode = 0;
static int  uvc_size = 0;
static int uvc_yst_thread_running = 0;


#if 1

static int uav_yst_send_frame(struct uvc_device *dev, char *frame_data, int frame_len)
{
	struct v4l2_buffer buf;
	int ret;

	memset(&buf, 0, sizeof buf);
	buf.type = V4L2_BUF_TYPE_VIDEO_OUTPUT;
	buf.memory = V4L2_MEMORY_MMAP;

	if ((ret = ioctl(dev->fd, VIDIOC_DQBUF, &buf)) < 0) {
		PRINT_ERR("Unable to dequeue buffer: %s (%d).\n", strerror(errno),
			errno);
		return -1;
	}

    //拷贝1帧数据
    memcpy(dev->mem[buf.index], frame_data, frame_len);
    buf.bytesused = frame_len;

	if ((ret = ioctl(dev->fd, VIDIOC_QBUF, &buf)) < 0) {
		PRINT_ERR("Unable to requeue buffer: %s (%d).\n", strerror(errno),
			errno);
		return -1;
	}

    return 0;
}

static int uvc_yst_read_and_send(struct uvc_device *dev, MEDIABUF_HANDLE reader)
{
    /* 读取数据 */
    if (reader == NULL) {
        PRINT_ERR("not add reader.\n");
        return -1;
    }

    GK_NET_FRAME_HEADER frame_header = {0};
    void *frame_data = NULL;
    int frame_len = 0;
    int ret;

    ret = mediabuf_read_frame(reader, (void **)&frame_data, &frame_len, &frame_header);
    if (ret < 0) {
        PRINT_ERR("mediabuf_read_frame fail.\n");
        return -1;
    } else if (ret == 0) {
        PRINT_INFO("mediabuf_read_frame = 0\n");
        return 0;
    } else {
        //PRINT_INFO("cmd read stream%d : mediabuf_read_frame OK.\n", tBST);
    }

    /* 不发送音频数据 */
    if (frame_header.frame_type == GK_NET_FRAME_TYPE_A) {
        //如果不发送音频数据，直接返回 0
        return 0;  
    }

    ////////////////////////////////
    return uav_yst_send_frame(dev, frame_data, frame_len);
}


static int uvc_yst_send_loop(MEDIABUF_HANDLE reader)
{
	
	char *mjpeg_image = NULL;
	
	int ret, opt;

    char *device = "/dev/video0";
	struct uvc_device *dev = uvc_open(device);
	if (dev == NULL) {
        PRINT_ERR("uav open error.");
        return -1;
    }

    int bulk_mode = 0;
	dev->bulk = bulk_mode;
    
	uvc_events_init(dev);
	uvc_video_init(dev);
    fcntl(dev->fd, O_NONBLOCK);

    fd_set wfds, efds;
	struct timeval timeout;
	while (1) {
        if (uvc_yst_thread_running == 0)
            break;
    
		timeout.tv_sec = 0;
		timeout.tv_usec = 100;

        FD_ZERO(&wfds);
        FD_ZERO(&efds);

        FD_SET(dev->fd, &wfds);
        FD_SET(dev->fd, &efds);

		ret = select(dev->fd + 1, NULL, &wfds, &efds, &timeout);
        if (ret < 0) {
            PRINT_ERR("select error:%s\n", strerror(errno));
        }
        
		if (FD_ISSET(dev->fd, &efds))
			uvc_events_process(dev);

		if (FD_ISSET(dev->fd, &wfds))
			uvc_yst_read_and_send(dev, reader);

    }

    uvc_close(dev);

    return 0; 
}


static int uav_yst_send_stream(int stream_id)
{
    MEDIABUF_HANDLE reader = mediabuf_add_reader(stream_id);
    if (reader == NULL) {
        PRINT_ERR("Add reader to stream%d error.\n", stream_id);
        return -1;
    }

    uvc_yst_send_loop(reader);

    mediabuf_del_reader(reader);

    return 0;
}


static void * uav_yst_stream0_thread(void *para)
{
    //sdk_sys_thread_set_name("uav_yst_stream0");

    uav_yst_send_stream(0);

    PRINT_INFO("uav_yst_stream0_thread end !\n");
    return NULL;
}
#endif

int uvc_start(char *mode, char *resolution)
{
    if(strcmp(mode, "-h264") == 0)
    {
        uvc_mode = 0;
    }
    else if(strcmp(mode, "-mjpg") == 0)
    {
        uvc_mode = 1;
    }
    else if(strcmp(mode, "-yuyv") == 0)
    {
        uvc_mode = 2;
    }
    
    if(strcmp(resolution, "-1080p") == 0)
    {
        uvc_size = 0;
    }
    else if(strcmp(resolution, "-720p") == 0)
    {
        uvc_size = 1;
    }
    else if(strcmp(resolution, "-360p") == 0)
    {
        uvc_size = 2;
    }

    uvc_format_set(uvc_mode, uvc_size);

    uvc_yst_thread_running = 1;
    CreateDetachThread(uav_yst_stream0_thread, NULL, NULL);

}

void uvc_stop()
{
    uvc_yst_thread_running = 0;
}

