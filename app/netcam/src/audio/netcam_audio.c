#include "netcam_api.h"
#include "cfg_com.h"
#include "media_fifo.h"
#include "common.h"
#include "netcam_api.h"
#include "g711.h"
#include "adpcm.h"
//#include "gk7101.h"
#include "zfifo.h"
#include "sdk_enc.h"
//#include "sdk_common.h"
#include "sdk_def.h"
#include "sdk_enc.h"
#include "sdk_audio.h"


static MEDIABUF_HANDLE audio_writer[4] = {0,0,0,0};
static pthread_rwlock_t audio_lock = PTHREAD_RWLOCK_INITIALIZER;
static int audio_mute = 0;
static char g_audio_init = 0;
static EM_GK_AUDIO_SAMPLEFORMAT audioEncType = GK_AUDIO_SAMPLE_FORMAT_RAW_PCM;
static EM_GK_AUDIO_SAMPLEFORMAT audioDecType = GK_AUDIO_SAMPLE_FORMAT_RAW_PCM;

extern int g_did_audioPlay;

int audio_spk_en_control(int enable)
{
    if(sdk_cfg.spk_en_gpio > 0)
    {
        int value = 0;
        if(enable == 1)
            value = sdk_cfg.spk_en_gpio_value==1?1:0;
        else
            value = sdk_cfg.spk_en_gpio_value==1?0:1;
        return sdk_gpio_set_output_value(sdk_cfg.spk_en_gpio, value);
    }
    else
    {
        return -1;
    }
}

static int audio_enter_lock(void)
{
	return pthread_rwlock_wrlock(&audio_lock);
}

static int audio_leave_lock(void)
{
	return pthread_rwlock_unlock(&audio_lock);
}

static int audio_digital_gain(char* input_valu,int data_len, int digitalGain)
{
    int i, len, tmp;
    short *tmpP = (short *)input_valu;

    if(digitalGain <= 1)
        return -1;
    len = data_len/2;
    for(i = 0; i < len; i++)
    {
        tmp = (int)(*tmpP) * digitalGain;
        if(tmp > 32767)
            tmp = 32767;
        else if(tmp < -32768)
            tmp = -32768;
        (*tmpP) = tmp;
        tmpP ++;
    }
    return 0;
}

static void audio_read_stream_callback(int stream, PS_GK_ENC_BUF_ATTR frameBuf)
{
    GK_NET_FRAME_HEADER header = {0};
    short *prawData = NULL;
    int outLen = 0;

	//音频输出假，则将音频数据丢掉
	if((runVideoCfg.vencStream[stream].avStream != 0)
	        || (audio_writer[stream] == NULL) )
	{
	    //PRINT_ERR("no audio.\n");
		usleep(10000);
		return ;
	}
    header.magic = MAGIC_TEST;
    header.device_type = 0;
    header.frame_size = frameBuf->data_sz;
    header.pts = frameBuf->time_us * (runAudioCfg.sampleRate/8000);

    header.media_codec_type = MEDIA_CODEC_ID_PCM;
    header.frame_type = GK_NET_FRAME_TYPE_A;
    header.frame_no = frameBuf->no;

    /* 加上时间，CMS需要时间 */
    struct timeval tv = {0};
    struct timezone tz = {0};
    gettimeofday(&tv, &tz);
    header.sec = tv.tv_sec - tz.tz_minuteswest*60;
    header.usec = tv.tv_usec;

    /* Remove G711A Encoder Header */
    if((GK_AUDIO_SAMPLE_FORMAT_A_LAW == netcam_audio_get_enctype())||
        (GK_AUDIO_SAMPLE_FORMAT_MU_LAW == netcam_audio_get_enctype())){
        prawData = (short*)malloc(frameBuf->data_sz);
        if(NULL != prawData){

            outLen = gk_audio_get_rawstream((short*)frameBuf->data,prawData,(frameBuf->data_sz/sizeof(short)));
            header.frame_size = outLen*sizeof(short);
            if(GK_AUDIO_SAMPLE_FORMAT_A_LAW == netcam_audio_get_enctype()){
    		    header.media_codec_type = MEDIA_CODEC_ID_PCM_ALAW;
            }
            if(GK_AUDIO_SAMPLE_FORMAT_MU_LAW == netcam_audio_get_enctype()){
    		    header.media_codec_type = MEDIA_CODEC_ID_PCM_MULAW;
            }
            mediabuf_write_frame(audio_writer[stream], prawData, outLen*sizeof(short), &header);
            free(prawData);
            prawData = NULL;
        }
    }
    else{
        mediabuf_write_frame(audio_writer[stream], frameBuf->data, frameBuf->data_sz, &header);
    }
}

/* get aduio enc type */
int netcam_audio_get_enctype(void)
{
    return audioEncType;
}

/* get audio enc type */
int netcam_audio_get_dectype(void)
{
    return audioDecType;
}

/* 音频初始化 */
int netcam_audio_init()
{
    PRINT_INFO("enter netcam_audio_init!");
    EM_GK_AUDIO_SAMPLEFORMAT sample_format;
    ST_GK_AUDIO_AioAttrT ao_attr;
    ST_GK_AUDIO_AioAttrT ai_attr;

    //AudioCfgPrint();
    if (runAudioCfg.mode == 0) {
        PRINT_INFO("not support audio!!!");
        return 0;
    }

    if(1 == runAudioCfg.rebootMute)
    {
        netcam_audio_mute(1);
        PRINT_INFO("set netcam_audio_mute \n");
    }

    int retVal = gk_audio_init();
    if(retVal < 0) {
        PRINT_ERR("gk_audio_init failed \n");
        return -1;
    }

    switch(runAudioCfg.type)
    {
        case 0:
            audioEncType = GK_AUDIO_SAMPLE_FORMAT_A_LAW;
            audioDecType = GK_AUDIO_SAMPLE_FORMAT_A_LAW;
            break;
        case 1:
            audioEncType = GK_AUDIO_SAMPLE_FORMAT_MU_LAW;
            audioDecType = GK_AUDIO_SAMPLE_FORMAT_MU_LAW;
            break;
        case 3:
            audioEncType = GK_AUDIO_SAMPLE_FORMAT_AAC;
            audioDecType = GK_AUDIO_SAMPLE_FORMAT_AAC;
            break;
        default:
        case 2:
            audioEncType = GK_AUDIO_SAMPLE_FORMAT_RAW_PCM;
            audioDecType = GK_AUDIO_SAMPLE_FORMAT_RAW_PCM;
            break;
    }
    switch(runAudioCfg.chans)
    {
        default:
        case 1:
            ai_attr.soundMode = GK_AUDIO_SOUND_MODE_SINGLE;
            ao_attr.soundMode = GK_AUDIO_SOUND_MODE_SINGLE;
            break;
        case 2:
            ai_attr.soundMode = GK_AUDIO_SOUND_MODE_STEREO;
            ao_attr.soundMode = GK_AUDIO_SOUND_MODE_STEREO;
            break;

    }
    switch(runAudioCfg.sampleRate)
    {
        default:
        case 8000:
            ai_attr.sampleRate = GK_AUDIO_SAMPLE_RATE_8000;
            ao_attr.sampleRate = GK_AUDIO_SAMPLE_RATE_8000;
            break;
        case 16000:            
            ai_attr.sampleRate = GK_AUDIO_SAMPLE_RATE_16000;
            ao_attr.sampleRate = GK_AUDIO_SAMPLE_RATE_16000;
            break;
        case 32000:            
            ai_attr.sampleRate = GK_AUDIO_SAMPLE_RATE_32000;
            ao_attr.sampleRate = GK_AUDIO_SAMPLE_RATE_32000;
            break;
    }

    switch(runAudioCfg.sampleBitWidth)
    {
        default:
        case 16:
            ai_attr.bitWidth = GK_AUDIO_BIT_WIDTH_16;
            ao_attr.bitWidth = GK_AUDIO_BIT_WIDTH_16;
            break;

    }


    PRINT_INFO("support audio vi. sample:%d\n", ai_attr.sampleRate);
    /* config aio dev attr */
    ai_attr.frameSamples = AUDIO_FRAMESAMPLES;
    ai_attr.frameNum = 30;
    EM_GK_AUDIO_GainLevelEnumT  vi_glevel = get_gain_level(GK_GLEVEL_MAX*runAudioCfg.inputVolume/100); // GLEVEL_10
    retVal = gk_audio_ai_init(&ai_attr, audioEncType, vi_glevel);
    if(retVal < 0) {
        PRINT_ERR("gk_audio_ai_init failed \n");
        return -1;
    }
    if (runAudioCfg.mode == 2) {
        PRINT_INFO("support audio vo.");
        ao_attr.frameSamples = AUDIO_FRAMESAMPLES;
        ao_attr.frameNum = 30;
        EM_GK_AUDIO_VOLUMELEVEL volumeLevel = get_volume_level(13*runAudioCfg.outputVolume/100); // VLEVEL_12
        retVal = gk_audio_ao_init(&ao_attr, audioDecType, volumeLevel);
        if(retVal < 0) {
            PRINT_ERR("gk_audio_ao_init failed \n");
            return -1;
        }
    }
    audio_spk_en_control(runAudioCfg.audioOutEnable);

	/* 添加音频写者 */
    int i = 0;
    int vencStream_cnt =  netcam_video_get_channel_number();
	for (i = 0; i < vencStream_cnt; i++) {
        audio_writer[i] = NULL;
    }

    /* 设置回调函数用于获取音频数据 */
    sdk_set_get_auido_stream_cb(audio_read_stream_callback);

    /* 开启线程，获取编码出来的音频流数据 */
	g_audio_init = 1;
	//netcam_audio_out(AUDIO_FILE_STARTING);
    if (runAudioCfg.audioInEnable == 0 || runAudioCfg.inputVolume == 0)
    {
        gk_audio_input_enable(0);
    }
    
    return 0;
}

/* 音频退出处理 */
int netcam_audio_exit()
{
    PRINT_INFO("enter netcam_audio_exit!");
	audio_enter_lock();
	if(g_audio_init == 0)
	{
		goto END;
	}

    if (runAudioCfg.mode == 0) {
        PRINT_INFO("not support audio!!!");
        goto END;
    }

    if (runAudioCfg.mode == 2) {
        PRINT_INFO("support audio vo.");
        gk_audio_ao_exit();
    }

    PRINT_INFO("support audio vi.");
    gk_audio_ai_exit();

    gk_audio_exit();

	/* 删除音频写者 */
    int i = 0;
    int vencStream_cnt =  netcam_video_get_channel_number();
	for(i = 0; i < vencStream_cnt; i++)
    {
        if (audio_writer[i] != NULL)
            mediabuf_del_writer(audio_writer[i]);
    }
	g_audio_init = 0;
END:
	audio_leave_lock();
    return 0;
}

void netcam_audio_add_writers()
{
	/* 添加音频写者 */
    int i = 0;
    int vencStream_cnt =  netcam_video_get_channel_number();
	for (i = 0; i < vencStream_cnt; i++) {
        audio_writer[i] = mediabuf_add_writer(i);
        if (audio_writer[i] == NULL) {
            PRINT_ERR("Create audio writer error\n");
        }
    }
}

/* clear aduio dec buffer */
int netcam_audio_clear_decbuf(void)
{
	if(audio_mute == 1 || sdk_ptz_get_startup_adjust_flag())
	{
		return 0;
	}

	audio_enter_lock();
	if(g_audio_init == 0)
	{
		audio_leave_lock();
		return 0;
	}
    gk_audio_ao_clear_buf();
	audio_leave_lock();
	return 0;

}

/* 让硬件输出 NETCAM_AUDIO_ENC_TYPE_t 格式的声音 */
int netcam_audio_output(char *buffer, int len, NETCAM_AUDIO_ENC_TYPE_t type, int isBlock)
{
//#if defined(CUS_SZ_MB_A1_IMX222) || defined(CUS_HZD_OJTV_V10_OV2710) || defined(CUS_HZD_OJTV_V10_JXH42)
#if 1
    int ret = -1;
	int bufferLen = 4096;
    unsigned char new_buf[4096] = {0};
    unsigned int new_len = 0;
	unsigned char *outBuf = NULL;
	int outBufLen = 0;

	if(buffer == NULL || len < 0 )
	{
		PRINT_ERR("parameter error");
		return -1;
	}

    #ifdef MODULE_SUPPORT_GB28181
    if (g_did_audioPlay != 0)
    {
        PRINT_INFO("len:%d,type:%d,block:%d\n",len,type,isBlock);
        return 0;
    }
    #endif

	if(audio_mute == 1 || sdk_ptz_get_startup_adjust_flag())
	{
		return 0;
	}

	audio_enter_lock();
	if(g_audio_init == 0)
	{
		audio_leave_lock();
		return 0;
	}

    //PRINT_INFO("len:%d,type:%d,block:%d\n",len,type,isBlock);
	switch(type)
	{
		case NETCAM_AUDIO_ENC_RAW_PCM:
		{
			ret = gk_audio_aout_play((unsigned char *)buffer, (unsigned int)len, (bool)isBlock);
			len = 0;
			break;
		}
		case NETCAM_AUDIO_ENC_MU_LAW:
		{
            if(GK_AUDIO_SAMPLE_FORMAT_MU_LAW == netcam_audio_get_dectype()){
                outBufLen = len + (len/GK_AUDIO_G711A_FRAME_LEN + 1)*4;
                outBuf = (short*)malloc(outBufLen);
                if(NULL != outBuf){
                    new_len = gk_audio_set_frameheader((short*)buffer,outBuf,GK_AUDIO_G711A_FRAME_LEN,len/sizeof(short));
                    ret = gk_audio_aout_play(outBuf, new_len*sizeof(short), (bool)isBlock);
                    free(outBuf);
                    outBuf = NULL;
                }
            }
            else{
			if(len <= (bufferLen/2))
			{
				new_len = audio_ulaw_decode((short *)new_buf, (unsigned char *)(buffer), len);
				outBuf = new_buf;
			}
			else
			{
				outBuf = malloc(len*2);
				if(outBuf == NULL)
				{
					PRINT_ERR("malloc failed, len:%d \n",len*2);
					break;
				}
				new_len = audio_ulaw_decode((short *)outBuf, (unsigned char *)(buffer ), len);
			}
			ret = gk_audio_aout_play(outBuf, new_len, (bool)isBlock);
			if(outBuf != new_buf)
			{
				free(outBuf);
			}
            }
			break;
		}

		case NETCAM_AUDIO_ENC_A_LAW:
		{
            if(GK_AUDIO_SAMPLE_FORMAT_A_LAW == netcam_audio_get_dectype()){
                outBufLen = len + (len/GK_AUDIO_G711A_FRAME_LEN + 1)*4;
                outBuf = (short*)malloc(outBufLen);
                if(NULL != outBuf){
                    new_len = gk_audio_set_frameheader((short*)buffer,outBuf,GK_AUDIO_G711A_FRAME_LEN,len/sizeof(short));
                    ret = gk_audio_aout_play(outBuf, new_len*sizeof(short), (bool)isBlock);
                    free(outBuf);
                    outBuf = NULL;
                }
            }
            else{
			if(len <= (bufferLen/2))
			{
				new_len = audio_alaw_decode((short*)new_buf, (unsigned char*)buffer, len);
				outBuf = new_buf;
			}
			else
			{
				outBuf = malloc(len*2);
				if(outBuf == NULL)
				{
					PRINT_ERR("malloc failed, len:%d \n",len*2);
					break;
				}
				new_len = audio_alaw_decode((short*)outBuf, (unsigned char*)buffer, len);
			}

			ret = gk_audio_aout_play(outBuf, new_len, (bool)isBlock);
			if(outBuf != new_buf)
			{
				free(outBuf);
			}
            }
			break;
		}
		case NETCAM_AUDIO_ENC_AD_PCM:
		{
			int pre_sample = 0;
			int index = 0;
			if(len <= (bufferLen/4))
			{
				adpcm_decode( (unsigned char*)buffer, len,new_buf,&pre_sample,&index);
				new_len = len * 4;
				outBuf = new_buf;
			}
			else
			{
				outBuf = malloc(len*4);
				if(outBuf == NULL)
				{
					PRINT_ERR("malloc failed, len:%d \n",len*4);
					break;
				}
				adpcm_decode( (unsigned char*)buffer, len,outBuf,&pre_sample,&index);
				new_len = len * 4;
			}
			ret = gk_audio_aout_play(outBuf, new_len, (bool)isBlock);
			if(outBuf != new_buf)
			{
				free(outBuf);
			}

			break;
		}

		default:
		{
			PRINT_ERR("Not support audio type");
			break;
		}

	}

	audio_leave_lock();
	return ret;
#else
    return 0;
#endif

}

#ifdef MODULE_SUPPORT_GB28181
/* 国标播放语音时不播放其他告警声 */
int netcam_audio_output_gb28181(char *buffer, int len, NETCAM_AUDIO_ENC_TYPE_t type, int isBlock)
{
//#if defined(CUS_SZ_MB_A1_IMX222) || defined(CUS_HZD_OJTV_V10_OV2710) || defined(CUS_HZD_OJTV_V10_JXH42)
#if 1
    int ret = -1;
	int bufferLen = 4096;
    unsigned char new_buf[4096] = {0};
    unsigned int new_len = 0;
	unsigned char *outBuf = NULL;

	if(buffer == NULL || len < 0 )
	{
		PRINT_ERR("parameter error");
		return -1;
	}

	if(audio_mute == 1 || sdk_ptz_get_startup_adjust_flag())
	{
		return 0;
	}

	audio_enter_lock();
	if(g_audio_init == 0)
	{
		audio_leave_lock();
		return 0;
	}

    //PRINT_INFO("len:%d,type:%d,block:%d\n",len,type,isBlock);
	switch(type)
	{
		case NETCAM_AUDIO_ENC_RAW_PCM:
		{
			ret = gk_audio_aout_play((unsigned char *)buffer, (unsigned int)len, (bool)isBlock);
			len = 0;
			break;
		}
		case NETCAM_AUDIO_ENC_MU_LAW:
		{
			if(len <= (bufferLen/2))
			{
				new_len = audio_ulaw_decode((short *)new_buf, (unsigned char *)(buffer), len);
				outBuf = new_buf;
			}
			else
			{
				outBuf = malloc(len*2);
				if(outBuf == NULL)
				{
					PRINT_ERR("malloc failed, len:%d \n",len*2);
					break;
				}
				new_len = audio_ulaw_decode((short *)outBuf, (unsigned char *)(buffer ), len);
			}
			ret = gk_audio_aout_play(outBuf, new_len, (bool)isBlock);
			if(outBuf != new_buf)
			{
				free(outBuf);
			}
			break;
		}

		case NETCAM_AUDIO_ENC_A_LAW:
		{

			if(len <= (bufferLen/2))
			{
				new_len = audio_alaw_decode((short*)new_buf, (unsigned char*)buffer, len);
				outBuf = new_buf;
			}
			else
			{
				outBuf = malloc(len*2);
				if(outBuf == NULL)
				{
					PRINT_ERR("malloc failed, len:%d \n",len*2);
					break;
				}
				new_len = audio_alaw_decode((short*)outBuf, (unsigned char*)buffer, len);
			}
            audio_digital_gain(outBuf, new_len, runAudioCfg.digitalGain);
           // printf("new len:%d\n", new_len);
			ret = gk_audio_aout_play(outBuf, new_len, (bool)isBlock);
			if(outBuf != new_buf)
			{
				free(outBuf);
			}

			break;
		}
		case NETCAM_AUDIO_ENC_AD_PCM:
		{
			int pre_sample = 0;
			int index = 0;
			if(len <= (bufferLen/4))
			{
				adpcm_decode( (unsigned char*)buffer, len,new_buf,&pre_sample,&index);
				new_len = len * 4;
				outBuf = new_buf;
			}
			else
			{
				outBuf = malloc(len*4);
				if(outBuf == NULL)
				{
					PRINT_ERR("malloc failed, len:%d \n",len*4);
					break;
				}
				adpcm_decode( (unsigned char*)buffer, len,outBuf,&pre_sample,&index);
				new_len = len * 4;
			}
			ret = gk_audio_aout_play(outBuf, new_len, (bool)isBlock);
			if(outBuf != new_buf)
			{
				free(outBuf);
			}

			break;
		}

		default:
		{
			PRINT_ERR("Not support audio type");
			break;
		}

	}

	audio_leave_lock();
	return ret;
#else
    return 0;
#endif

}
#endif

/* 播报信息 */
int netcam_audio_out(void* arg)
{
	char audio_filename[64] = {0};
	char* pospath = (char*)arg;
    char lan[3]={'\0'};
    if(arg == NULL) {
        PRINT_ERR("param error.");
        return -1;
    }

	if(audio_mute == 1)
	{
		return 0;
	}
    switch(runSystemCfg.deviceInfo.languageType)
    {
        case 0:
            strcpy(lan,"zh");
            break;
        case 1:
            strcpy(lan,"en");
            break;
        case 2:
            strcpy(lan,"ja");
            break;
        default:
            PRINT_ERR("language type :%d not support!\n",runSystemCfg.deviceInfo.languageType);
            strcpy(lan,"zh");
            break;

    }
	snprintf(audio_filename, sizeof(audio_filename), "%s/%s/%s", AUDIO_RES_PATH_PRE,
		lan, pospath);

    FILE *fp = fopen(audio_filename, "rb");
    if (fp  == NULL) {
        PRINT_ERR("fopen %s error.\n", audio_filename);
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    int len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
//    PRINT_ERR("aduio file %s len:%d.\n", audio_filename, len);
    char *buffer = (char*)malloc(len);
    if (buffer == NULL) {
        PRINT_ERR("malloc failed.\n");
		fclose(fp);
        return -1;
    }
    memset(buffer, 0, len);

    int ret = fread(buffer, 1, len, fp);
    netcam_audio_clear_decbuf();
    netcam_audio_output(buffer, ret, NETCAM_AUDIO_ENC_A_LAW, 1);

    free(buffer);
    fclose(fp);

	return 0;
}

#ifdef MODULE_SUPPORT_MOJING
int netcam_audio_out_mojing(void* arg)
{
	char audio_filename[128] = {0};
	char* pospath = (char*)arg;
    char lan[3]={'\0'};
    char fileDir[128] = {0};
    if(arg == NULL) {
        PRINT_ERR("param error.");
        return -1;
    }

	if(audio_mute == 1)
	{
		return 0;
	}

    switch(runSystemCfg.deviceInfo.languageType)
    {
        case 0:
            strcpy(lan,"zh");
            break;
        case 1:
            strcpy(lan,"en");
            break;
        case 2:
            strcpy(lan,"ja");
            break;
        default:
            PRINT_ERR("language type :%d not support!\n",runSystemCfg.deviceInfo.languageType);
            strcpy(lan,"zh");
            break;

    }

    if (strstr(pospath, "md") != NULL)
    {
        strcpy(fileDir, AUDIO_RES_PATH_PRE);
        snprintf(audio_filename, sizeof(audio_filename), "%s/%s/%s", fileDir, lan, pospath);
    }
    else
    {
        strcpy(fileDir, "/opt/custom/cfg");
        snprintf(audio_filename, sizeof(audio_filename), "%s/%s", fileDir, pospath);
    }
    

    FILE *fp = fopen(audio_filename, "rb");
    if (fp  == NULL) {
        PRINT_ERR("fopen %s error.\n", audio_filename);
        return -1;
    }
    fseek(fp, 0, SEEK_END);
    int len = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    //PRINT_ERR("aduio file %s len:%d.\n", audio_filename, len);
    char *buffer = (char*)malloc(len);
    if (buffer == NULL) {
        PRINT_ERR("malloc failed.\n");
		fclose(fp);
        return -1;
    }
    memset(buffer, 0, len);

    int ret = fread(buffer, 1, len, fp);
    netcam_audio_clear_decbuf();
    netcam_audio_output(buffer, ret, NETCAM_AUDIO_ENC_A_LAW, 1);

    free(buffer);
    fclose(fp);

	return 0;
}
#endif


/* 播报信息，网络状况等 */
int netcam_audio_hint(NETCAM_AUDIO_HintEnumT hint)
{
    int ret;
	int fd = -1;
	int fd2 = -1;
	char *pName = NULL;
	char *pName2 = NULL;
	char audio_filename[64];
	char buff[4096] = {0};
    char lan[3]={'\0'};
    if(1 == audio_mute)
    {
        PRINT_INFO("netcam_audio_hint audio_mute\n");
        return 0;
    }
	switch(hint)
	{
		case SYSTEM_AUDIO_HINT_LINK_SUCCESS:
			pName = HINT_WIRELESS;
			pName2 = HINT_SUCCESS;
			break;
		case SYSTEM_AUDIO_HINT_LINK_FAIL:
			pName = HINT_WIRELESS;
			pName2 = HINT_FAIL;
			break;
		case SYSTEM_AUDIO_HINT_USE_WIRE:
            if(runSystemCfg.deviceInfo.languageType == 2)//japanese,change sequence
            {
                pName = HINT_WIRE;
    			pName2 = HINT_USE;
            }
            else
            {
                pName = HINT_USE;
    			pName2 = HINT_WIRE;
            }

			break;
		case SYSTEM_AUDIO_HINT_USE_WIRELESS:
            if(runSystemCfg.deviceInfo.languageType == 2)//japanese,change sequence
            {
                pName = HINT_WIRELESS;
                pName2 = HINT_USE;
            }
            else
            {
                pName = HINT_USE;
                pName2 = HINT_WIRELESS;
            }
			break;
		case SYSTEM_AUDIO_HINT_DISCONNECT_FAIL:
			pName = HINT_DISCONNECT;
			pName2 = HINT_FAIL;
			break;
		case SYSTEM_AUDIO_HINT_DISCONNECT_SUCCESS:
			pName = HINT_DISCONNECT;
			pName2 = HINT_WIRELESS;
			break;
		case SYSTEM_AUDIO_HINT_GET_WIFIPW:
			pName = HINT_WIFIPW;
			pName2 = HINT_CONNECT;

		default:
			break;
	}

	if(pName == NULL || pName2 == NULL)
		return -1;

	memset(audio_filename, 0, sizeof(audio_filename));

    switch(runSystemCfg.deviceInfo.languageType)
    {
        case 0:
            strcpy(lan,"zh");
            break;
        case 1:
            strcpy(lan,"en");
            break;
        case 2:
            strcpy(lan,"ja");
            break;
        default:
            PRINT_ERR("language type :%d not support!\n",runSystemCfg.deviceInfo.languageType);
            strcpy(lan,"zh");
            break;

    }

	snprintf(audio_filename, sizeof(audio_filename), "%s/%s/%s", AUDIO_RES_PATH_PRE,
		lan, pName);
    PRINT_INFO("audio hint path1 :%s\n",audio_filename);

	fd = open(audio_filename, O_RDONLY);
	if(fd <= 0) {
		PRINT_ERR("can not open %s", audio_filename);
		return -1;
	}

	memset(audio_filename, 0, sizeof(audio_filename));
	snprintf(audio_filename, sizeof(audio_filename), "%s/%s/%s", AUDIO_RES_PATH_PRE,
		lan, pName2);
    PRINT_INFO("audio hint path2 :%s\n",audio_filename);
	fd2 = open(audio_filename, O_RDONLY);
	if(fd2 <= 0) {
		PRINT_ERR("can not open %s", audio_filename);
		close(fd);
		return -1;
	}

	ret = read(fd, buff, sizeof(buff));
    netcam_audio_clear_decbuf();
	while(ret > 0) {
		PRINT_INFO("audio buffsize:%d\n", ret);
		netcam_audio_output(buff, ret, NETCAM_AUDIO_ENC_A_LAW, 1);
		ret = read(fd, buff, sizeof(buff));
	}

	ret = read(fd2, buff, sizeof(buff));
    netcam_audio_clear_decbuf();
	while(ret > 0) {
		PRINT_INFO("audio buffsize:%d\n", ret);
		netcam_audio_output(buff, ret, NETCAM_AUDIO_ENC_A_LAW, 1);
		ret = read(fd2, buff, sizeof(buff));
	}

	close(fd);
	close(fd2);

	return 0;
}


/* 播报传入的IP地址 */
int netcam_audio_ip(const char* ipaddress)
{
	int fd, ret;
	char ch;
	char audio_filename[64];
	char buff[4096] = {0};
	const char* s = ipaddress;
    char lan[3]={'\0'};

	PRINT_INFO("play ip address:%s\n", ipaddress);
	if (!ipaddress)
		return -1;

	if(audio_mute == 1)
	{
		return 0;
	}
    switch(runSystemCfg.deviceInfo.languageType)
    {
        case 0:
            strcpy(lan,"zh");
            break;
        case 1:
            strcpy(lan,"en");
            break;
        case 2:
            strcpy(lan,"ja");
            break;
        default:
            PRINT_ERR("language type :%d not support!\n",runSystemCfg.deviceInfo.languageType);
            strcpy(lan,"zh");
            break;
    }

	while (*s)
	{
		ch = *s;
		s++;

		memset(audio_filename, 0, sizeof(audio_filename));
		if (ch == '.')
			snprintf(audio_filename, sizeof(audio_filename), "%s/%s/dot.alaw",
				AUDIO_RES_PATH_PRE,
				lan);
		else if(ch >= '0' && ch <= '9')
			snprintf(audio_filename, sizeof(audio_filename), "%s/%s/%c.alaw",
				AUDIO_RES_PATH_PRE,
				lan, ch);
		else
			continue;

		fd = open(audio_filename, O_RDONLY);
		if(fd < 0) {
			PRINT_ERR("can not open %s", audio_filename);
			return -1;
		}
        netcam_audio_clear_decbuf();
		while((ret = read(fd, buff, sizeof(buff))) > 0)
		{
			netcam_audio_output(buff, ret, NETCAM_AUDIO_ENC_A_LAW, 1);
			memset(buff, 0, sizeof(buff));
		}

		close(fd);
		//usleep(300000);
	}
	return 0;
}

ZFIFO_DESC *writer = NULL;

/* 在消回音过程中，从 audiobuf 读出数据 */
int audiobuf_read_frame(void *readerid, void **data, int *size)
{
    if (data == NULL || readerid == NULL || size == NULL) {
        PRINT_ERR("param error.");
        return -1;
    }

    ZFIFO_NODE node[1];

    ZFIFO_DESC *zfifo_desc = (ZFIFO_DESC *)readerid;
    if (*data == NULL) {
        node[0].base = zfifo_desc->frame_data;
        node[0].len = zfifo_desc->frame_len;
    } else {
        node[0].base = *data;
        node[0].len = *size;
    }

	int n = 0;
   // n = zfifo_readv((ZFIFO_DESC *)readerid, node, 1, 10);
    n = zfifo_readv((ZFIFO_DESC *)readerid, node, 1, 1);


    if (n < 0) {
        PRINT_ERR("zfifo_readv error.");
        netcam_exit(90);
    }
    if (n == 0) {
        //PRINT_INFO("zfifo_readv 0.");
        *size = 0;
        return 0;
    }

    if (*data == NULL) {
        if (node[0].len > zfifo_desc->frame_len) {
            PRINT_INFO("data buf size %d is too small, so to realloc %d len", zfifo_desc->frame_len, node[0].len);
            zfifo_desc->frame_len = node[0].len + FRAME_LEN_ADD;
            zfifo_desc->frame_data = realloc(zfifo_desc->frame_data, zfifo_desc->frame_len);
            if (zfifo_desc->frame_data == NULL) {
                PRINT_ERR("realloc %d error.", zfifo_desc->frame_len);
                netcam_exit(90);
            }
            //zfifo_desc->frame_len = node[1].len;

            memcpy(zfifo_desc->frame_data, node[0].base, node[0].len);
            free(node[0].base);
        }
        *data = zfifo_desc->frame_data;
        *size = node[0].len;
    } else {
        if(node[0].len > *size) {
            PRINT_ERR("data buf size is too small.");
            memcpy(*data, node[0].base, *size);
            free(node[0].base);
        } else {
            *size = node[0].len;
        }
    }

    return n;

}

/* 在消回音过程中，向 audiobuf 写入数据 */
int audiobuf_write_frame(void *writerid, void *data, int size)
{
    if (data == NULL || writerid == NULL) {
        PRINT_ERR("param error.");
        return -1;
    }
    ZFIFO_NODE node[1];
    node[0].base = data;
    node[0].len = size;

    int ret = zfifo_writev((ZFIFO_DESC *)writerid, node, 1);
    if (ret < 0) {
        PRINT_ERR("zfifo_writev error.");
        netcam_exit(90);
    }

    return ret;
}


static int audio_send_loop(void)
{
    //sleep(20);
    PRINT_INFO("in audio_send_loop");
    int ret = 0;

    char name[10] = {0};
    sprintf(name, "audio_send_buf");
    ZFIFO *audio_send_fifo = zfifo_init(name, 20 * 1024);
    if (audio_send_fifo == NULL) {
        PRINT_ERR("zfifo_init %s failed.", name);
        return -1;
    }

    ZFIFO_DESC *reader = zfifo_open(audio_send_fifo);
    if (reader == NULL) {
        PRINT_ERR("zfifo_open reader failed.");
        return NULL;
    }

    writer = zfifo_open(audio_send_fifo);
    if (writer == NULL) {
       PRINT_ERR("zfifo_open writer failed.");
       return NULL;
    } else {
        PRINT_INFO("audio add writer:%x\n", writer);
    }

    void *frame_data = NULL;
    int frame_len = 0;
    netcam_audio_clear_decbuf();
	while (1)
    {

        #if 1
        frame_data = NULL;
        frame_len = 0;

        //从zfifo读取数据
        ret = audiobuf_read_frame(reader, (void **)&frame_data, &frame_len);
        if (ret < 0)
        {
            PRINT_ERR("cms read stream%d : mediabuf_read_frame fail.\n", 0);
            return -1;
        }
        else if (ret == 0)
        {
            //printf("wirte 320 - 0.\n");
            /* 这里用320，避免太频繁 */
            char buf_tmp[320] = {0};
            usleep(30000);
            //netcam_audio_output(buf_tmp, 320, NETCAM_AUDIO_ENC_RAW_PCM, 1);
        }
        else
        {
            //printf("wirte %d.\n", frame_len);
            netcam_audio_output(frame_data, frame_len, NETCAM_AUDIO_ENC_RAW_PCM, 1);
        }

        #else
        printf("wirte 302.\n");
        char buf_tmp[320] = {0};
        netcam_audio_output(buf_tmp, 320, NETCAM_AUDIO_ENC_RAW_PCM, 1);

        #endif

    }

    zfifo_close(reader);
    zfifo_close(writer);
    zfifo_uninit(audio_send_fifo);
    PRINT_INFO("end audio_send_loop.");
    return 0;

}

static void *audio_send_thread(void *param)
{
    SetThreadName("audio_send_thread");
    audio_send_loop();
    return NULL;
}

/* 在消回音过程中，不断从 audiobuf 读取数据，如果没有数据，则发空包。开始这个流程 */
int audio_send_start()
{
    if (runAudioCfg.mode > 0)
        CreateDetachThread(audio_send_thread, NULL, NULL);

    return 0;
}

/* 在消回音过程中，不断从 audiobuf 读取数据，如果没有数据，则发空包。结束这个流程 */
int audio_send_stop()
{
    return 0;
}

void netcam_audio_mute(int enable)
{
	audio_mute = enable;

}

