/*!
*****************************************************************************
** FileName     : rec_pcm.c
**
** Description  : record pcm.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create       : 2015-8-11, create
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include "common.h"
#include "media_fifo.h"
#include "g711.h"
#include "rec_pcm.h"
#include "sdk_audio.h"

static int rec_pcm_test(void)
{
    int ret = 0;

    //添加读者
    MEDIABUF_HANDLE reader = mediabuf_add_reader(0);
    if (reader == NULL) 
    {
        PRINT_ERR("mediabuf_add_reader error.\n");
        return -1;
    }

    FILE *fp = fopen("/tmp/audio_test.pcm", "wb+");
    if (fp == NULL)
    {
        PRINT_ERR("fopen error.\n");
        mediabuf_del_reader(reader);
        return -1;
    }
    PRINT_INFO("1, open ok.");
    
    int have_audio = 1;
    int have_video = 1;

    void *frame_data = NULL;
    int frame_len = 0;
    GK_NET_FRAME_HEADER frame_header;
    struct timeval t1, t2;
    gettimeofday(&t1, NULL);
	while (1) 
    {
        memset(&frame_header, 0, sizeof(GK_NET_FRAME_HEADER));
        frame_data = NULL;
        frame_len = 0;

        //从zfifo读取数据
        ret = mediabuf_read_frame(reader, (void **)&frame_data, &frame_len, &frame_header);
        if (ret < 0) 
        {
            PRINT_ERR("cms read stream%d : mediabuf_read_frame fail.\n", 0);
            return -1;
        } 
        else if (ret == 0) 
        {
            continue;
        }

        if (frame_header.frame_type == GK_NET_FRAME_TYPE_A) 
        {
            if (have_audio == 1) 
            {
                printf("******** mediabuf have audio, rec pcm ************ \n");
                printf("a alaw len:%d\n", frame_len);
                have_audio = 0;
            }

            //audioApi->aout_play(frame_data, frame_len, true);
            
            /* g711a -> pcm */
            unsigned char dst_buf[4096] = {0};
            int dst_len = 0;
            if (frame_len * 2 > sizeof(dst_buf)) 
            {
                PRINT_ERR("audio bufsize error.\n");
                return -1;
            }
            dst_len = audio_alaw_decode((short *)dst_buf, frame_data, frame_len);
            frame_header.frame_size = dst_len;

            /* write pcm file */
            //audioApi->aout_play(dst_buf, dst_len, true);
            ret = fwrite(dst_buf, 1, dst_len, fp);
            if (ret <= 0) 
            {
                PRINT_ERR("write error.\n");
                continue;
            }
        }
        else if ( (frame_header.frame_type == GK_NET_FRAME_TYPE_I) || (frame_header.frame_type == GK_NET_FRAME_TYPE_P) )
        {
            if (have_video == 1)
            {
                printf("******** mediabuf have video ************ \n");
                printf("v len:%d\n", frame_len);
                have_video = 0;
            }
        }

        gettimeofday(&t2, NULL);
        if ((t2.tv_sec - t1.tv_sec) >= 15) // 15 seconds
        {
            PRINT_INFO("rec pcm, time to stop.\n");
            break;
        }
    }

    PRINT_INFO("2, write audio data ok.");
    if (fp)
    {
        fclose(fp); 
        fp = NULL;
    }

    mediabuf_del_reader(reader);
    PRINT_INFO("3, end audio file ok.");  
    return 0;
  
}    


static int rec_pcm_talk_test()
{
    int fd = -1;   
    unsigned int  count  = 0;
    unsigned char *buffer;
    off_t len;

    if ((fd = open("/tmp/audio_test.pcm", O_RDONLY, 0)) == -1)
    {
        printf("open audio_test.pcm failed.\n");
        return -1;
    }
    len = lseek(fd,0,SEEK_END);
    printf("len:%d\n", len);
    //len=ftell(f);
    lseek(fd, 0, SEEK_SET);
    buffer = (unsigned char*)malloc(len);
    if (buffer == NULL)
    {
        printf("malloc failed.\n");
        return -1;
    }
    count = read(fd, buffer, len);           
    printf("count=%d\n",count);

    #if 1
    unsigned char *buffer2 = (unsigned char*)malloc(len / 2);
    if (buffer2 == NULL)
    {
        printf("malloc buffer2 failed.\n");
        return -1;
    }

    /* pcm -> g711a */
    int offset = 0;
    int dst_offset = 0;
    
    unsigned char dst_buf[4096] = {0};
    int dst_len = 0;

	while(len > 0)
	{
		dst_len = len > 10000?10000:len;
		netcam_audio_output(tempbuffer,dst_len,0,1); // pcm and bolock
		tempbuffer +=dst_len;

		len -= dst_len;

		
	}

    printf("src_offset:%d dst_offset:%d\n", offset, dst_offset);
  

    //gk_audio_init_aout(GK_AUDIO_BIT_WIDTH_16, GK_AUDIO_SOUND_MODE_MONO, GK_AUDIO_SAMPLE_RATE_8000, GK_AUDIO_SAMPLE_FORMAT_A_LAW, GK_VLEVEL_10);   
    //audioApi->init_aout(GK_AUDIO_BIT_WIDTH_16, GK_AUDIO_SOUND_MODE_MONO, GK_AUDIO_SAMPLE_RATE_8000, GK_AUDIO_SAMPLE_FORMAT_RAW_PCM, GK_VLEVEL_10);
    //audioApi->aout_play(buffer, count, true);

    //audioApi->init_aout(GK_AUDIO_BIT_WIDTH_16, GK_AUDIO_SOUND_MODE_MONO, GK_AUDIO_SAMPLE_RATE_8000, GK_AUDIO_SAMPLE_FORMAT_A_LAW, GK_VLEVEL_12);
    //audioApi->aout_play(buffer2, count / 2, true);
    printf("1end\n");
    
    free(buffer);
    free(buffer2);
    close(fd);

    //audioApi->destroy_aout();
    printf("2end\n");
    return 0;
}

static void *rec_pcm_test_thread(void *param)
{
	sdk_sys_thread_set_name("rec_pcm_test_thread");
    rec_pcm_test();
    rec_pcm_talk_test();
    rec_pcm_test();
    rec_pcm_talk_test();
    return NULL;
}

int rec_pcm_test_start()
{
    pthread_t thread_id;
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    //pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);//detached
    if(0 != pthread_create(&thread_id, &attr, rec_pcm_test_thread, NULL))
    {
        pthread_attr_destroy(&attr);
        return -1;
    }
    pthread_attr_destroy(&attr);

    return 0;
}


int rec_pcm_test_stop()
{

    return 0;
}

