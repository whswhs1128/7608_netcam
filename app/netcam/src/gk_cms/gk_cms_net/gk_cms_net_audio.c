#include "gk_cms_common.h"
#include "gk_cms_media.h"
#include "gk_cms_sock.h"
#include "gk_cms_protocol.h"
#include "gk_cms_utility.h"
#include "gk_cms_net_api.h"
#include "cfg_all.h"
#include "netcam_api.h"
#include "g711.h"

#if 1
int write_audio_from_file()
{
    printf("write_audio_from_file \n");
    int fd = -1;   
    unsigned int  count  = 0;
    unsigned char *buffer;
    int len;

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

    unsigned char *buffer2 = (unsigned char*)malloc(len / 2);
    if (buffer2 == NULL)
    {
        printf("malloc buffer2 failed.\n");
        return -1;
    }

    
    #if 1
    /* pcm -> g711a */

    int dst_len = audio_alaw_encode(buffer2, (short *)buffer, count);

    printf("src_len:%d dst_len:%d\n", count, dst_len);


    //gk_audio_init_aout(GK_AUDIO_BIT_WIDTH_16, GK_AUDIO_SOUND_MODE_MONO, GK_AUDIO_SAMPLE_RATE_8000, GK_AUDIO_SAMPLE_FORMAT_A_LAW, GK_VLEVEL_10);   
    //audioApi->init_aout(GK_AUDIO_BIT_WIDTH_16, GK_AUDIO_SOUND_MODE_MONO, GK_AUDIO_SAMPLE_RATE_8000, GK_AUDIO_SAMPLE_FORMAT_RAW_PCM, GK_VLEVEL_10);
    //audioApi->aout_play(buffer, count, true);

    //audioApi->init_aout(GK_AUDIO_BIT_WIDTH_16, GK_AUDIO_SOUND_MODE_MONO, GK_AUDIO_SAMPLE_RATE_8000, GK_AUDIO_SAMPLE_FORMAT_A_LAW, GK_VLEVEL_12);
    //audioApi->aout_play(buffer2, dst_len, true);
    //netcam_audio_output((char*)buffer2, dst_len, NETCAM_AUDIO_ENC_A_LAW, SDK_TRUE);	xqq
    printf("1end\n");
    
    free(buffer);
    free(buffer2);
    close(fd);

    printf("22end\n");

    #endif

#if 0
    #if 1
    /* pcm -> g711a */
    int offset = 0;
    int dst_offset = 0;
    
    unsigned char dst_buf[4096] = {0};
    int dst_len = 0;

    while (offset < count)
    {
        //dst_len = audio_alaw_encode((short *)dst_buf, buffer + offset, 2048);
        dst_len = audio_alaw_encode((short *)dst_buf, buffer + offset, 1600);
        memcpy(buffer2 + dst_offset, dst_buf, dst_len);
        offset += 1600;
        dst_offset += dst_len;
    }
    printf("src_offset:%d dst_offset:%d\n", offset, dst_offset);
    #endif

    //gk_audio_init_aout(GK_AUDIO_BIT_WIDTH_16, GK_AUDIO_SOUND_MODE_MONO, GK_AUDIO_SAMPLE_RATE_8000, GK_AUDIO_SAMPLE_FORMAT_A_LAW, GK_VLEVEL_10);   
    //audioApi->init_aout(GK_AUDIO_BIT_WIDTH_16, GK_AUDIO_SOUND_MODE_MONO, GK_AUDIO_SAMPLE_RATE_8000, GK_AUDIO_SAMPLE_FORMAT_RAW_PCM, GK_VLEVEL_10);
    //audioApi->aout_play(buffer, count, true);

    audioApi->init_aout(GK_AUDIO_BIT_WIDTH_16, GK_AUDIO_SOUND_MODE_MONO, GK_AUDIO_SAMPLE_RATE_8000, GK_AUDIO_SAMPLE_FORMAT_A_LAW, GK_VLEVEL_12);
    audioApi->aout_play(buffer2, count / 2, true);
    printf("1end\n");
    
    free(buffer);
    free(buffer2);
    close(fd);

    audioApi->destroy_aout();
    printf("2end\n");

#endif

#if 0
    #if 1
    /* pcm -> g711a */
    int offset = 0;
    int dst_offset = 0;
    
    unsigned char dst_buf[4096] = {0};
    int dst_len = 0;

    while (offset < count)
    {
        //dst_len = audio_alaw_encode((short *)dst_buf, buffer + offset, 2048);
        dst_len = audio_alaw_encode((short *)dst_buf, buffer + offset, 1600);
        memcpy(buffer2 + dst_offset, dst_buf, dst_len);
        offset += 1600;
        dst_offset += dst_len;

        audioApi->init_aout(GK_AUDIO_BIT_WIDTH_16, GK_AUDIO_SOUND_MODE_MONO, GK_AUDIO_SAMPLE_RATE_8000, GK_AUDIO_SAMPLE_FORMAT_A_LAW, GK_VLEVEL_12);
        audioApi->aout_play(dst_buf, dst_len, true);
        printf("1end\n");

        audioApi->destroy_aout();
        printf("2end\n");
    }
    printf("src_offset:%d dst_offset:%d\n", offset, dst_offset);
    #endif

    free(buffer);
    free(buffer2);
    close(fd);
#endif


    return 0;
}

#endif

int Gk_CmsWriteAudioData(char *buffer, int count, int type)
{
    //gk_audio_aout_play(buffer, count, true);
    //current only support G711_ALUW, 16 bit, MONO, 8000 
	//netcam_audio_output(buffer, count,NETCAM_AUDIO_ENC_A_LAW, SDK_TRUE);
	//netcam_audio_output(buffer, count, NETCAM_AUDIO_ENC_RAW_PCM, SDK_TRUE);	xqq

    return 0;
}



