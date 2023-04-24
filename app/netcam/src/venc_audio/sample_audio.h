#ifndef _SAMPLE_AUDIO_H_
#define _SAMPLE_AUDIO_H_
td_void sample_audio_ai_aenc_init_param(ot_aio_attr *aio_attr, ot_audio_dev *ai_dev, ot_audio_dev *ao_dev);

td_void sample_audio_set_ai_vqe_param(sample_comm_ai_vqe_param *ai_vqe_param,
    ot_audio_sample_rate out_sample_rate, td_bool resample_en, td_void *ai_vqe_attr, td_u32 ai_vqe_type);

#define sample_dbg(ret) \
    do { \
        printf("ret = %#x, fuc:%s, line:%d\n", ret, __FUNCTION__, __LINE__); \
    } while (0);

td_s32 sample_audio_aenc_bind_ai(ot_audio_dev ai_dev, td_u32 aenc_chn_cnt);


td_void sample_audio_aenc_unbind_ai(ot_audio_dev ai_dev, td_u32 aenc_chn_cnt);

// td_s32 sample_audio_ai_aenc(td_void);
#endif
