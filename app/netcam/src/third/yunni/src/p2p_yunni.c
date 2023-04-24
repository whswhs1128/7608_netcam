#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "bellcall.h"
#include "media_fifo.h"
#include "netcam_api.h"
#include "avi_common.h"
#include "avi_search_api.h"

#define MAX_VIDEO_CHANNEL 3
#define MAX_AUDIO_CHANNEL 1
#define MAX_HANDLES       8


typedef struct video_handle
{
	MEDIABUF_HANDLE handle[MAX_VIDEO_CHANNEL];
}VHANDLE;

typedef struct audio_handle
{
	MEDIABUF_HANDLE handle[MAX_AUDIO_CHANNEL];
}AHANDLE;


static VHANDLE hVideos[MAX_HANDLES];
static AHANDLE hAudios[MAX_HANDLES];
static AviPBHandle* hMedia[MAX_HANDLES];
//
// 国科音频编解码库
//
unsigned int audio_alaw_encode(unsigned char *dst, short *src, unsigned int srcSize);
unsigned int audio_alaw_decode(short *dst, const unsigned char *src, unsigned int srcSize);
unsigned int audio_ulaw_encode(unsigned char *dst, short *src, unsigned int srcSize);
unsigned int audio_ulaw_decode(short *dst, const unsigned char *src, unsigned int srcSize);

//
// 设备初始化
//
int PPDevInit(
	void *	Data,
	int		DataLens
){
	int i, j;

	for(i = 0; i < MAX_HANDLES; i++)
	{
		for (j = 0; j < MAX_VIDEO_CHANNEL; j++)
		{
			hVideos[i].handle[j] = NULL;
		}
	}

	for(i = 0; i < MAX_HANDLES; i++)
	{
		for (j = 0; j < MAX_AUDIO_CHANNEL; j++)
		{
			hAudios[i].handle[j] = NULL;
		}
	}

	for (i = 0; i < MAX_HANDLES; i++)
	{
		hMedia[i] = NULL;
	}

	return 0;
}

//
// 关闭设备
//
int PPDevFree(
	void *	Data,
	int		DataLens
){
	int i, j;

	for(i = 0; i < MAX_HANDLES; i++)
	{
		for (j = 0; j < MAX_VIDEO_CHANNEL; j++)
		{
			if (hVideos[i].handle[j])
			{
				mediabuf_del_reader(hVideos[i].handle[j]);
				hVideos[i].handle[j] = NULL;
			}
		}
	}

	for(i = 0; i < MAX_HANDLES; i++)
	{
		for (j = 0; j < MAX_AUDIO_CHANNEL; j++)
		{
			if (hAudios[i].handle[j])
			{
				mediabuf_del_reader(hAudios[i].handle[j]);
				hAudios[i].handle[j] = NULL;
			}
		}
	}

	for (i = 0; i < MAX_HANDLES; i++)
	{
		if (hMedia[i])
		{
			avi_pb_close(hMedia[i]);
			hMedia[i] = NULL;
		}
	}

	return 0;
}

//
// 媒体文件流初始化
//
int PPDevMediaInit(
	void *	Data,
	int		DataLens
)
{
	int i;
	PPDEV_MEDIA_T* pDev = (PPDEV_MEDIA_T*)Data;
	if (!pDev)
	{
		printf("[%s]ERROR! data is null.\n", __FUNCTION__);
		return -1;
	}

	for (i = 0; i < MAX_HANDLES; i++)
	{
		if (!hMedia[i]) break;
	}

	if (i == MAX_HANDLES)
	{
		printf("[%s]ERROR! too much client connection.\n", __FUNCTION__);
		return -1;
	}

	hMedia[i] = create_pb_handle();
	if (!hMedia[i])
	{
        printf("[%s]ERROR! fail to create pb handle.\n", __FUNCTION__);
        return -1;
    }

    if (avi_pb_open(pDev->File, hMedia[i]) < 0)
	{
        printf("[%s]ERROR! fail to avi_pb_open file %s.\n", __FUNCTION__, pDev->File);
        avi_pb_close(hMedia[i]);
		hMedia[i] = NULL;
        return -1;
    }

	pDev->Handle = i;
	printf("[%s]OK! initialized media reader success. index[%d]\n", __FUNCTION__, i);
	return 0;
}


//
// 视频流初始化
//
int PPDevVideoInit(
	void *	Data,
	int		DataLens
)
{
	int i, j;
	PPDEV_VIDEO_T* pDev = (PPDEV_VIDEO_T*)Data;

	if (!pDev)
	{
		printf("[%s]ERROR! data is null.\n", __FUNCTION__);
		return -1;
	}

	for (i = 0; i < MAX_HANDLES; i++)
	{
		for (j = 0; j < MAX_VIDEO_CHANNEL; j++)
			if (hVideos[i].handle[j]) break;

		if (j == MAX_VIDEO_CHANNEL)
			break;
	}

	if (i == MAX_HANDLES)
	{
		printf("[%s]ERROR! too much client connection.\n", __FUNCTION__);
		return -1;
	}

	for(j = 0; j < MAX_VIDEO_CHANNEL; j++)
	{
		hVideos[i].handle[j] = mediabuf_add_reader(j);
		if (hVideos[i].handle[j] == NULL)
		{
			printf("[%s]ERROR! initialized video reader failed.\n", __FUNCTION__);
			return -1;
		}
	}

	pDev->Handle = i;
	pDev->MaxChannels = MAX_VIDEO_CHANNEL - 1;
	printf("[%s]OK! initialized video reader success. index[%d]\n", __FUNCTION__, i);
	return 0;
}

//
// 语音流初始化
//
int PPDevAudioInit(
	void *	Data,
	int		DataLens
)
{
	return 0;
}


//
// 音频流初始化
//
int PPDevVoiceInit(
	void *	Data,
	int		DataLens
)
{
	int i, j;
	PPDEV_AUDIO_T* pDev = (PPDEV_AUDIO_T*)Data;

	if (!pDev)
	{
		printf("[%s]ERROR! data is null.\n", __FUNCTION__);
		return -1;
	}

	for (i = 0; i < MAX_HANDLES; i++)
	{
		for (j = 0; j < MAX_AUDIO_CHANNEL; j++)
			if (hAudios[i].handle[j]) break;

		if (j == MAX_AUDIO_CHANNEL)
			break;
	}

	if (i == MAX_HANDLES)
	{
		printf("[%s]ERROR! too much client connection.\n", __FUNCTION__);
		return -1;
	}

	for(j = 0; j < MAX_AUDIO_CHANNEL; j++)
	{
		hAudios[i].handle[j] = mediabuf_add_reader(j);
		if (hAudios[i].handle[j] == NULL)
		{
			printf("[%s]ERROR! initialized audio reader failed.\n", __FUNCTION__);
			return -1;
		}
	}

	pDev->Handle = i;
	printf("[%s]OK! initialized audio reader success. index[%d]\n", __FUNCTION__, i);
	return 0;
}

//
// 视频流释放
//
int PPDevVideoFree(
	void *	Data,
	int		DataLens
)
{
	int i;
	PPDEV_VIDEO_T* pDev = (PPDEV_VIDEO_T*)Data;

	if (!pDev)
	{
		printf("[%s]ERROR! data is null.\n", __FUNCTION__);
		return -1;
	}

	if (pDev->Handle < 0 || pDev->Handle >= MAX_HANDLES)
	{
		printf("[%s]ERROR! pDev->Handle[%d] exceed.\n", __FUNCTION__, pDev->Handle);
		return -1;
	}

	for(i = 0; i < MAX_VIDEO_CHANNEL; i++)
	{
		if (hVideos[pDev->Handle].handle[i])
		{
			mediabuf_del_reader(hVideos[pDev->Handle].handle[i]);
			hVideos[pDev->Handle].handle[i] = NULL;
		}
	}

	printf("[%s]OK! free video reader success. index[%d]\n", __FUNCTION__, pDev->Handle);
	return 0;
}

//
// 音频流释放
//
int PPDevAudioFree(
	void *	Data,
	int		DataLens
)
{
	return 0;
}


//
// 语音流释放
//
int PPDevVoiceFree(
	void *	Data,
	int		DataLens
)
{
	int i;
	PPDEV_AUDIO_T* pDev = (PPDEV_AUDIO_T*)Data;

	if (!pDev)
	{
		printf("[%s]ERROR! data is null.\n", __FUNCTION__);
		return -1;
	}

	if (pDev->Handle < 0 || pDev->Handle >= MAX_HANDLES)
	{
		printf("[%s]ERROR! pDev->Handle[%d] exceed.\n", __FUNCTION__, pDev->Handle);
		return -1;
	}

	for(i = 0; i < MAX_AUDIO_CHANNEL; i++)
	{
		if (hAudios[pDev->Handle].handle[i])
		{
			mediabuf_del_reader(hAudios[pDev->Handle].handle[i]);
			hAudios[pDev->Handle].handle[i] = NULL;
		}
	}

	printf("[%s]OK! free audio reader success. index[%d]\n", __FUNCTION__, pDev->Handle);
	return 0;
}


//
// 媒体文件流释放
//
int PPDevMediaFree(
	void *	Data,
	int		DataLens
)
{
	PPDEV_MEDIA_T* pDev = (PPDEV_MEDIA_T*)Data;

	if (!pDev)
	{
		printf("[%s]ERROR! data is null.\n", __FUNCTION__);
		return -1;
	}

	if (pDev->Handle < 0 || pDev->Handle >= MAX_HANDLES)
	{
		printf("[%s]ERROR! pDev->Handle[%d] exceed.\n", __FUNCTION__, pDev->Handle);
		return -1;
	}

	if (!hMedia[pDev->Handle])
		return 0;

	avi_pb_close(hMedia[pDev->Handle]);
	hMedia[pDev->Handle] = NULL;

	printf("[%s]OK! free media reader success. index[%d]\n", __FUNCTION__, pDev->Handle);
	return 0;
}

//
// 通过文件捕获音视频帧
//
int PPDevGetMediaFrame(
	int		MediaHandle,		// 句柄
	char *	MediaBuffer,		// 缓冲区
	int		MediaBufferSize,	// 缓冲区大小
	int  *	MediaBufferType		// 类型
)
{
	int ret, size = 0;
	AviPBHandle* handle;

    if(MediaBuffer == NULL || MediaBufferType == NULL)
	{
		printf("[%s]ERROR! param is null.\n", __FUNCTION__);
        return -1;
    }

	if (MediaHandle < 0 || MediaHandle >= MAX_HANDLES)
	{
		printf("[%s]ERROR! VideoHandle[%d] exceed.\n", __FUNCTION__, MediaHandle);
		return -1;
	}

	if (hMedia[MediaHandle] == NULL)
	{
		printf("[%s]ERROR! hMedia[%d] is not init.\n", __FUNCTION__, MediaHandle);
		return -1;
	}

	handle = hMedia[MediaHandle];
	ret = avi_pb_get_frame(handle, NULL, &size);
	if (ret < 0)
	{
		printf("[%s]ERROR! avi pb get frame failed\n", __FUNCTION__);
		return -1;
    }

    if (ret == GK_NET_NOMOREFILE)
	{
		printf("[%s]OK! playback file end.\n", __FUNCTION__);
        return 0;
    }

    memcpy(MediaBuffer,handle->node.one_frame_buf,size);
	if (handle->node.frame_type == 0x11) {
		*MediaBufferType = 0;
	} else if (handle->node.frame_type == 0x10) {
		*MediaBufferType = 1;
	} else {
		*MediaBufferType = 6;
	}

	return size;
}

static int PPDevGetSnapshot(char** buf, int len)
{
	int ret, fd, rlen;
	time_t tt;
	struct tm tm_t;
	char snap_path[256] = {0};
	char* pbuf;

	if (len <= 0)
		return -1;

    time(&tt);
    localtime_r(&tt, &tm_t);
    tm_t.tm_year += 1900;
    tm_t.tm_mon += 1;

    sprintf(snap_path, "/tmp/yunni_%04d%02d%02d_%02d%02d%02d.jpg",
		tm_t.tm_year, tm_t.tm_mon, tm_t.tm_mday, tm_t.tm_hour,
		tm_t.tm_min, tm_t.tm_sec);

	ret = netcam_video_snapshot(640, 480, snap_path,
		GK_ENC_SNAPSHOT_QUALITY_MEDIUM);
    if (ret != 0)
	{
        PRINT_ERR("netcam_video_snapshot failed\n");
        goto error;
    }

	fd = open(snap_path, O_RDONLY);
	if(fd < 0) {
        PRINT_ERR("fopen %s failed\n", snap_path);
        goto cleanup;
    }

	pbuf = malloc(len);
	if (!pbuf)
		goto cleanup;

	*buf = pbuf;
	rlen = 0;
	while (len > 0)
	{
		ret = read(fd, pbuf, 4096);
		if (ret < 0)
		{
			PRINT_ERR("read fail. ret:%d,errno:%d,errinfo:%s\n", ret, errno, strerror(errno));
			goto cleanup;
		}
		if (ret == 0)
			break;

		rlen += ret;
		len -= ret;
		pbuf += ret;
	}

cleanup:
	close(fd);
error:
    //sprintf(cmd_line, "rm -rf %s", snap_path);
	//system(cmd_line);
	if (unlink(snap_path))
		PRINT_ERR("unlink fail. errno:%d,errinfo:%s\n", errno, strerror(errno));
	return rlen;
}


//
// 通过设备捕获视频帧
//
int PPDevGetVideoFrame(
	int		VideoHandle,		// 句柄
	char **	VideoBuffer,		// 缓冲区
	int		VideoBufferSize,	// 缓冲区大小
	int	 *	VideoBufferType,	// 图像类型
	int  	Channel,			// 通道
	int	 	Refresh				// 刷新缓冲
){
	GK_NET_FRAME_HEADER frame_header = {0};
	int recv_frame_len = 0;
    int ret = 0;
    char *buffer;

    if(VideoBufferType == NULL)
	{
		printf("[%s]ERROR! param is null.\n", __FUNCTION__);
        return -1;
    }

	if (VideoHandle < 0 || VideoHandle >= MAX_HANDLES)
	{
		printf("[%s]ERROR! VideoHandle[%d] exceed.\n", __FUNCTION__, VideoHandle);
		return -1;
	}

	if(Channel < 0 || Channel > MAX_VIDEO_CHANNEL)
	{
		printf("[%s]ERROR! Channel[%d] exceed.\n", __FUNCTION__, Channel);
		return -1;
	}

	if (Channel == MAX_VIDEO_CHANNEL)
		return PPDevGetSnapshot(VideoBuffer, VideoBufferSize);

	//channel id 是反的
	Channel = CHANNEL_ID_CONVERT(Channel);

	if (hVideos[VideoHandle].handle[Channel] == NULL)
	{
		printf("[%s]ERROR! hVideos[%d].handle[%d] is not init.\n",
			__FUNCTION__, VideoHandle, Channel);
		return -1;
	}

	buffer = NULL;
	if (Refresh)
	{
		printf("[%s]OK! Switch Channel[%d],hVideos[%d],handle_addr[0x%x]\n",
			__FUNCTION__, Channel, VideoHandle,
			(unsigned int)hVideos[VideoHandle].handle[Channel]);

		ret = mediabuf_read_I_frame(hVideos[VideoHandle].handle[Channel],
			(void*)&buffer, &recv_frame_len, &frame_header);

		printf("[%s]frame_header.frame_type = %c\n", __FUNCTION__,
			frame_header.frame_type == GK_NET_FRAME_TYPE_I ? 'I':'P');
	}
	else
	{
    	ret = mediabuf_read_frame(hVideos[VideoHandle].handle[Channel],
			(void*)&buffer, &recv_frame_len, &frame_header);
	}

    if (ret <= 0)
    {
        printf("[%s]ERROR! read no data.\n",__FUNCTION__);
		return 0;
    }

	if (frame_header.frame_type == GK_NET_FRAME_TYPE_I) {
		*VideoBufferType = 0;
	} else if (frame_header.frame_type == GK_NET_FRAME_TYPE_P) {
		*VideoBufferType = 1;
	} else {
		return 0;
	}

	*VideoBuffer = buffer;
	return recv_frame_len;
}

//
// 通过设备捕获音频帧
//
int PPDevGetVoiceFrame(
	int 	VoiceHandle,		// 句柄
	char *	VoiceBuffer,		// 缓冲区
	int		VoiceBufferSize,	// 缓冲区大小
	int	 *	VoiceBufferType,	// 音频类型
	int  	Channel,			// 通道
	int	 	Refresh				// 刷新缓冲
){
	GK_NET_FRAME_HEADER frame_header = {0};
	int recv_frame_len = 0;
    int recv_all_len = 0;
    char *audioBuffer = NULL;

    if(VoiceBuffer == NULL || VoiceBufferType == NULL)
	{
        printf("[%s]ERROR! param is null.\n", __FUNCTION__);
        return -1;
    }

	if (VoiceHandle < 0 || VoiceHandle >= MAX_HANDLES)
	{
		printf("[%s]ERROR! VoiceHandle[%d] exceed.\n", __FUNCTION__, VoiceHandle);
		return -1;
	}

	if(Channel < 0 || Channel >= MAX_AUDIO_CHANNEL)
	{
		printf("[%s]ERROR! Channel[%d] exceed.\n", __FUNCTION__, Channel);
		return -1;
	}

	if (hAudios[VoiceHandle].handle[Channel] == NULL)
	{
		printf("[%s]ERROR! hAudios[%d].handle[%d] is not init.\n",
			__FUNCTION__, VoiceHandle, Channel);
		return -1;
	}

    if (Refresh)
		mediabuf_set_newest_frame(hAudios[VoiceHandle].handle[Channel]);

    recv_all_len = mediabuf_read_frame(hAudios[VoiceHandle].handle[Channel],
		(void*)&audioBuffer, &recv_frame_len, &frame_header);
    if (recv_all_len <= 0)
    {
        printf("[%s]ERROR! read no data.\n",__FUNCTION__);
        return 0;
    }

    //unsigned char new_buf[4096] = {0};
    unsigned int new_len = 0;
	if (frame_header.frame_type == GK_NET_FRAME_TYPE_A) {
		*VoiceBufferType = 1;

	    #if 1
		/* g711a -> pcm */
	    new_len = audio_alaw_decode((short *)VoiceBuffer, (unsigned char*)audioBuffer, recv_frame_len);
		//memcpy(VoiceBuffer, new_buf, new_len);
		#else
		new_len = recv_frame_len;
		memcpy(VoiceBuffer, audioBuffer, recv_frame_len);
	    #endif

	} else {
		return 0;
	}

	return new_len;
}

//
// 通过设备播放音频帧
//
int PPDevPutAudioFrame(
	char *	AudioBuffer,		// 缓冲区
	int		AudioBufferLens,	// 缓冲区大小
	int	 *	AudioBufferType		// 音频类型
){
	printf("======================PPDevPutAudioFrame=========================\n");
	if (!AudioBuffer)
	{
		printf("yunni audio talk type param is null!\n");
		return -1;
	}

	PRINT_INFO("yuni audio talk len:%d\n", AudioBufferLens);
	// yunni input PCM data
	netcam_audio_output(AudioBuffer, AudioBufferLens, NETCAM_AUDIO_ENC_RAW_PCM, SDK_TRUE);
	return 0;
}

/*
// 设备 GPIO 初始化
//
int PPDevGPIOsInit(
	void *	Data,
	int 	DataLens
)
{
	return 0;
}

//
// 设备 GPIO 控制, 由上层循环调用
//
int PPDevGPIOsCall(
	void *	Data,
	int 	DataLens
)
{
	return 0;
}

//
// 设备 GPIO 释放
//
int PPDevGPIOsFree(
	void *	Data,
	int 	DataLens
)
{
	return 0;
}
*/

//
// 获取设备 GPIO 的状态
//
int PPDevGetGPIO(
	int		Pin
){
	int value;
	sdk_gpio_get_intput_value(Pin, &value);
	return value;
}

//
// 设置设备 GPIO 的状态
//
int PPDevSetGPIO(
	int 	Pin,
	int		Val
){
	gk_gpio_set_valu(Pin, Val);
	return 0;//++
}

/*
//	休眠动作
//
int PPDevSetStandby(
	void *	Data,
	int		DataLens
){
	return 0;//--
}

//
//	唤醒动作
//
int PPDevGetStandby(
	void *	Data,
	int		DataLens
){
	return 0;//--
}
*/
//
//	重启系统
//
int PPDevReboot(
	void *	Data,
	int		DataLens
)
{
	sync();
    usleep(20);
    netcam_sys_operation(0,SYSTEM_OPERATION_REBOOT);
	return 0;
}


