/*
  Copyright (c), 2001-2022, Shenshu Tech. Co., Ltd.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "ot_common_dis.h"
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <error.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include "sample_comm.h"
#include "securec.h"
#include "ss_mpi_snap.h"

#include "sample_comm.h"
#include "rtsp_demo.h"
#include "comm.h"

#include "ss_mpi_region.h"
// #include "pq_bin.h"
#include "ss_mpi_isp.h"
#include "ss_mpi_sys.h"

#include "ot_pq_bin.h"
#include "ot_defines.h"
#include <limits.h>
#include "ot_common_isp.h"
#include "ot_common_awb.h"
#include "ss_mpi_awb.h"
#include "sample_audio.h"
#include "audio_aac_adp.h"
#include "audio_dl_adp.h"
#include "ot_resample.h"
#include "sample_audio.h"
#if defined(OT_VQE_USE_STATIC_MODULE_REGISTER)
#include "ot_vqe_register.h"
#endif

#include "cfg_video.h"
GK_NET_VIDEO_CFG runVideoCfg;
static ot_payload_type g_payload_type = OT_PT_AAC;
static td_bool g_aio_resample = TD_FALSE;
static td_bool g_user_get_mode = TD_FALSE;
static td_bool g_ao_volume_ctrl = TD_FALSE;
static ot_audio_sample_rate g_in_sample_rate = OT_AUDIO_SAMPLE_RATE_BUTT;
static ot_audio_sample_rate g_out_sample_rate = OT_AUDIO_SAMPLE_RATE_BUTT;
/* 0: close, 1: record, 2: talk, 3: talkv2 */
static td_u32 g_ai_vqe_type = 1;
static void change_state(int signo);

#define BIG_STREAM_SIZE PIC_1080P
#define SMALL_STREAM_SIZE PIC_720P
static int aenc_open = 0;

typedef struct
{
    rtsp_demo_handle g_rtsplive;
    rtsp_session_handle session;
    int channel_num;
} rtsp_handle_struct;

static pthread_t venc_audio_pthread[4];

static int EXIT_MODE_X = 1;
static int End_Rtsp = 1;
rtsp_handle_struct rtsp_handle[2];

static ot_snap_attr g_norm_snap_attr = {
    .snap_type = OT_SNAP_TYPE_NORM,
    .load_ccm_en = TD_TRUE,
    .norm_attr = {
        .frame_cnt = 2, /* snap 2 frames */
        .repeat_send_times = 1,
        .zsl_en = TD_FALSE,
    },
};

#define VI_VB_YUV_CNT 6
#define VPSS_VB_YUV_CNT 8

#define ENTER_ASCII 10

#define VB_MAX_NUM 10

#define CHN_NUM_MAX 2

rtsp_demo_handle g_rtsplive = NULL;
rtsp_session_handle session = NULL;

typedef struct
{
    ot_size max_size;
    ot_pixel_format pixel_format;
    ot_size output_size[OT_VPSS_MAX_PHYS_CHN_NUM];
    ot_compress_mode compress_mode[OT_VPSS_MAX_PHYS_CHN_NUM];
    td_bool enable[OT_VPSS_MAX_PHYS_CHN_NUM];
} sample_venc_vpss_chn_attr;

typedef struct
{
    td_u32 valid_num;
    td_u64 blk_size[OT_VB_MAX_COMMON_POOLS];
    td_u32 blk_cnt[OT_VB_MAX_COMMON_POOLS];
    td_u32 supplement_config;
} sample_venc_vb_attr;

typedef struct
{
    ot_vpss_chn vpss_chn[CHN_NUM_MAX];
    ot_venc_chn venc_chn[CHN_NUM_MAX];
} sample_venc_vpss_chn;

static td_bool g_sample_venc_exit = TD_FALSE;

td_void *SAMPLE_COMM_AUDIO_AencProc_new(td_void *p)
{

    td_s32 ret = 0;
    int i = 0;
    static int s_aencFd = 0;
    static int s_maxAFd = 0;
    // rtsp_handle_struct rtsp_p = *(rtsp_handle_struct *)p;
    fd_set read_fds;
    ot_audio_stream stAStream;

    s_aencFd = ss_mpi_aenc_get_fd(0);
    s_maxAFd = s_maxAFd > s_aencFd ? s_maxAFd : s_aencFd;
    s_maxAFd = s_maxAFd + 1;

    struct timeval TimeoutVal;
    FD_ZERO(&read_fds);
    FD_SET(s_aencFd, &read_fds);

    TimeoutVal.tv_sec = 1;
    TimeoutVal.tv_usec = 0;
    while (End_Rtsp)
    {

        ret = ss_mpi_aenc_get_stream(0, &stAStream, TD_TRUE);
        if (TD_SUCCESS != ret)
        {
            // printf("ss_mpi_aenc_get_stream .. failed with %#x!\n", ret);
            continue;
            // return -1;
        }

        for (i = 0; i < CHN_NUM_MAX; i++)
        {
            if (runVideoCfg.vencStream[i].avStream == 0)
            {
                if (rtsp_handle[i].g_rtsplive)
                {
                    rtsp_tx_audio(rtsp_handle[i].session, (unsigned char *)stAStream.stream,
                                  stAStream.len, stAStream.time_stamp);
                }
            }
        }
        ret = ss_mpi_aenc_release_stream(0, &stAStream);

        if (TD_SUCCESS != ret)
        {
            sample_print("ss_mpi_aenc_release_stream chn[%d] .. failed with %#x!\n", 0, ret);
            // continue;
            return -1;
        }
    }
    printf("==========audio end==========\n");
    return NULL;
}

#if 1
static td_s32 sample_audio_ai_aenc(td_void)
{
    td_s32 ret;
    ot_audio_dev ai_dev;
    ot_audio_dev ao_dev;
    td_u32 ai_chn_cnt;
    td_u32 aenc_chn_cnt;
    td_bool send_adec = TD_TRUE;
    ot_aio_attr aio_attr = {0};
    sample_comm_ai_vqe_param ai_vqe_param = {0};

    sample_audio_ai_aenc_init_param(&aio_attr, &ai_dev, &ao_dev);

    /* step 1: start ai */
    ai_chn_cnt = aio_attr.chn_cnt;
    sample_audio_set_ai_vqe_param(&ai_vqe_param, g_out_sample_rate, g_aio_resample, TD_NULL, 0);
    ret = sample_comm_audio_start_ai(ai_dev, ai_chn_cnt, &aio_attr, &ai_vqe_param, -1);
    if (ret != TD_SUCCESS)
    {
        sample_dbg(ret);
        goto ai_aenc_err6;
    }
    /* step 2: config audio codec */
    ret = sample_comm_audio_cfg_acodec(&aio_attr);
    if (ret != TD_SUCCESS)
    {
        sample_dbg(ret);
        goto ai_aenc_err5;
    }

    /* step 3: start aenc */
    aenc_chn_cnt = aio_attr.chn_cnt >> ((td_u32)aio_attr.snd_mode);
    ret = sample_comm_audio_start_aenc(aenc_chn_cnt, &aio_attr, g_payload_type);
    if (ret != TD_SUCCESS)
    {
        sample_dbg(ret);
        goto ai_aenc_err5;
    }

    /* step 4: aenc bind ai chn */
    ret = sample_audio_aenc_bind_ai(ai_dev, aenc_chn_cnt);
    if (ret != TD_SUCCESS)
    {
        goto ai_aenc_err4;
    }


    pthread_create(&venc_audio_pthread[2], 0, SAMPLE_COMM_AUDIO_AencProc_new, NULL);
    pthread_detach(venc_audio_pthread[2]);
    while (EXIT_MODE_X)
    {
        signal(SIGINT, change_state);
    }

    printf("==============end audio=============\n");
   

    sample_audio_aenc_unbind_ai(ai_dev, aenc_chn_cnt);

ai_aenc_err4:
    ret = sample_comm_audio_stop_aenc(aenc_chn_cnt);
    if (ret != TD_SUCCESS)
    {
        sample_dbg(ret);
    }

ai_aenc_err5:
    ret = sample_comm_audio_stop_ai(ai_dev, ai_chn_cnt, g_aio_resample, TD_FALSE);
    if (ret != TD_SUCCESS)
    {
        sample_dbg(ret);
    }

ai_aenc_err6:
    return ret;
}
#endif
/******************************************************************************
 * function : to process abnormal case
 ******************************************************************************/
static td_void sample_venc_handle_sig(td_s32 signo)
{
    if (g_sample_venc_exit == TD_TRUE)
    {
        return;
    }

    if (signo == SIGINT || signo == SIGTERM)
    {
        g_sample_venc_exit = TD_TRUE;
    }

}

static td_s32 sample_venc_getchar()
{
    td_s32 c;
    if (g_sample_venc_exit == TD_TRUE)
    {
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
        return 'e';
    }

    c = getchar();

    if (g_sample_venc_exit == TD_TRUE)
    {
        printf("\033[0;31mprogram termination abnormally!\033[0;39m\n");
        return 'e';
    }

    return c;
}

static td_s32 get_gop_mode(ot_venc_gop_mode *gop_mode)
{
    td_s32 c[2] = {'\0'}; /* 2: len */

    *gop_mode = OT_VENC_GOP_MODE_NORMAL_P;
    return TD_SUCCESS;
}
#if 0
static td_void print_rc_mode(ot_payload_type type)
{
    printf("please input choose rc mode!\n");
    printf("\t c) cbr.\n");
    printf("\t v) vbr.\n");
    if (type != OT_PT_MJPEG) {
        printf("\t a) avbr.\n");
        printf("\t x) cvbr.\n");
        printf("\t q) qvbr.\n");
    }
    printf("\t f) fix_qp\n");
}
#endif
static td_s32 get_rc_mode(ot_payload_type type, sample_rc *rc_mode)
{
    td_s32 c[2] = {'\0'}; /* 2: len */

    *rc_mode = SAMPLE_RC_CBR;
    return TD_SUCCESS;
}

static td_void get_vb_attr(const ot_size *vi_size, const sample_venc_vpss_chn_attr *vpss_chn_attr,
                           sample_venc_vb_attr *vb_attr)
{
    td_s32 i;
    ot_pic_buf_attr pic_buf_attr = {0};

    vb_attr->valid_num = 0;

    // vb for vi-vpss
    pic_buf_attr.width = vi_size->width;
    pic_buf_attr.height = vi_size->height;
    pic_buf_attr.align = OT_DEFAULT_ALIGN;
    pic_buf_attr.bit_width = OT_DATA_BIT_WIDTH_8;
    pic_buf_attr.pixel_format = OT_PIXEL_FORMAT_YUV_SEMIPLANAR_422;
    pic_buf_attr.compress_mode = OT_COMPRESS_MODE_NONE;
    vb_attr->blk_size[vb_attr->valid_num] = ot_common_get_pic_buf_size(&pic_buf_attr);
    vb_attr->blk_cnt[vb_attr->valid_num] = VI_VB_YUV_CNT;
    vb_attr->valid_num++;

    // vb for vpss-venc(big stream)
    if (vb_attr->valid_num >= OT_VB_MAX_COMMON_POOLS)
    {
        return;
    }

    for (i = 0; i < OT_VPSS_MAX_PHYS_CHN_NUM && vb_attr->valid_num < OT_VB_MAX_COMMON_POOLS; i++)
    {
        if (vpss_chn_attr->enable[i] == TD_TRUE)
        {
            pic_buf_attr.width = vpss_chn_attr->output_size[i].width;
            pic_buf_attr.height = vpss_chn_attr->output_size[i].height;
            pic_buf_attr.align = OT_DEFAULT_ALIGN;
            pic_buf_attr.bit_width = OT_DATA_BIT_WIDTH_8;
            pic_buf_attr.pixel_format = vpss_chn_attr->pixel_format;
            pic_buf_attr.compress_mode = vpss_chn_attr->compress_mode[i];
            if (pic_buf_attr.compress_mode == OT_COMPRESS_MODE_SEG_COMPACT)
            {
                ot_vb_calc_cfg calc_cfg = {0};
                ot_common_get_vpss_compact_seg_buf_size(&pic_buf_attr, &calc_cfg);
                vb_attr->blk_size[vb_attr->valid_num] = calc_cfg.vb_size;
            }
            else
            {
                vb_attr->blk_size[vb_attr->valid_num] = ot_common_get_pic_buf_size(&pic_buf_attr);
            }
            vb_attr->blk_cnt[vb_attr->valid_num] = VPSS_VB_YUV_CNT + 1;

            vb_attr->valid_num++;
        }
    }

    //    vb_attr->supplement_config = OT_VB_SUPPLEMENT_JPEG_MASK | OT_VB_SUPPLEMENT_BNR_MOT_MASK;
    vb_attr->supplement_config = OT_VB_SUPPLEMENT_JPEG_MASK;
}

static td_void get_default_vpss_chn_attr(ot_size *vi_size, ot_size enc_size[], td_s32 len,
                                         sample_venc_vpss_chn_attr *vpss_chan_attr)
{
    td_s32 i;
    td_u32 max_width;
    td_u32 max_height;

    if (memset_s(vpss_chan_attr, sizeof(sample_venc_vpss_chn_attr), 0, sizeof(sample_venc_vpss_chn_attr)) != EOK)
    {
        printf("vpss chn attr call memset_s error\n");
        return;
    }

    max_width = vi_size->width;
    max_height = vi_size->height;

    for (i = 0; (i < len) && (i < OT_VPSS_MAX_PHYS_CHN_NUM); i++)
    {
        vpss_chan_attr->output_size[i].width = enc_size[i].width;
        vpss_chan_attr->output_size[i].height = enc_size[i].height;
        vpss_chan_attr->compress_mode[i] = (i == 0) ? OT_COMPRESS_MODE_SEG_COMPACT : OT_COMPRESS_MODE_NONE;
        vpss_chan_attr->enable[i] = TD_TRUE;

        max_width = MAX2(max_width, enc_size[i].width);
        max_height = MAX2(max_height, enc_size[i].height);
    }

    vpss_chan_attr->max_size.width = max_width;
    vpss_chan_attr->max_size.height = max_height;
    vpss_chan_attr->pixel_format = OT_PIXEL_FORMAT_YVU_SEMIPLANAR_420;

    return;
}

static td_s32 sample_venc_sys_init(sample_venc_vb_attr *vb_attr)
{
    td_u32 i;
    td_s32 ret;
    ot_vb_cfg vb_cfg = {0};

    if (vb_attr->valid_num > OT_VB_MAX_COMMON_POOLS)
    {
        sample_print("sample_venc_sys_init vb valid num(%d) too large than OT_VB_MAX_COMMON_POOLS(%d)!\n",
                     vb_attr->valid_num, OT_VB_MAX_COMMON_POOLS);
        return TD_FAILURE;
    }

    for (i = 0; i < vb_attr->valid_num; i++)
    {
        vb_cfg.common_pool[i].blk_size = vb_attr->blk_size[i];
        vb_cfg.common_pool[i].blk_cnt = vb_attr->blk_cnt[i];
    }

    vb_cfg.max_pool_cnt = vb_attr->valid_num;

    if (vb_attr->supplement_config == 0)
    {
        ret = sample_comm_sys_init(&vb_cfg);
    }
    else
    {
        ret = sample_comm_sys_init_with_vb_supplement(&vb_cfg, vb_attr->supplement_config);
    }

    if (ret != TD_SUCCESS)
    {
        sample_print("sample_venc_sys_init failed!\n");
    }

    return ret;
}

static td_s32 sample_venc_vi_init(sample_vi_cfg *vi_cfg)
{
    td_s32 ret;

    ret = sample_comm_vi_start_vi(vi_cfg);
    if (ret != TD_SUCCESS)
    {
        sample_print("sample_comm_vi_start_vi failed: 0x%x\n", ret);
        return ret;
    }

    return TD_SUCCESS;
}

static td_void sample_venc_vi_deinit(sample_vi_cfg *vi_cfg)
{
    sample_comm_vi_stop_vi(vi_cfg);
}

static td_s32 sample_venc_vpss_init(ot_vpss_grp vpss_grp, sample_venc_vpss_chn_attr *vpss_chan_cfg)
{
    td_s32 ret;
    ot_vpss_chn vpss_chn;
    ot_vpss_grp_attr grp_attr = {0};
    ot_vpss_chn_attr chn_attr[OT_VPSS_MAX_PHYS_CHN_NUM] = {0};

    grp_attr.max_width = vpss_chan_cfg->max_size.width;
    grp_attr.max_height = vpss_chan_cfg->max_size.height;
    grp_attr.nr_en = TD_FALSE;
    grp_attr.dei_mode = OT_VPSS_DEI_MODE_OFF;
    grp_attr.pixel_format = vpss_chan_cfg->pixel_format;
    grp_attr.frame_rate.src_frame_rate = -1;
    grp_attr.frame_rate.dst_frame_rate = -1;

    for (vpss_chn = 0; vpss_chn < OT_VPSS_MAX_PHYS_CHN_NUM; vpss_chn++)
    {
        if (vpss_chan_cfg->enable[vpss_chn] == 1)
        {
            // chn_attr[vpss_chn].width = vpss_chan_cfg->output_size[vpss_chn].width;
            // chn_attr[vpss_chn].height = vpss_chan_cfg->output_size[vpss_chn].height;
            chn_attr[0].width = 1920;
            chn_attr[0].height = 1080;
            chn_attr[1].width = 1280;
            chn_attr[1].height = 720;
            chn_attr[vpss_chn].chn_mode = OT_VPSS_CHN_MODE_USER;
            // chn_attr[vpss_chn].compress_mode = vpss_chan_cfg->compress_mode[vpss_chn];
            chn_attr[vpss_chn].compress_mode = OT_COMPRESS_MODE_NONE;
            chn_attr[vpss_chn].pixel_format = vpss_chan_cfg->pixel_format;
            chn_attr[vpss_chn].frame_rate.src_frame_rate = runVideoCfg.vencStream[vpss_chn].h264Conf.fps;
            chn_attr[vpss_chn].frame_rate.dst_frame_rate = runVideoCfg.vencStream[vpss_chn].h264Conf.fps;
            chn_attr[vpss_chn].depth = 0;
        }
    }

    ret = sample_common_vpss_start(vpss_grp, vpss_chan_cfg->enable, &grp_attr, chn_attr, OT_VPSS_MAX_PHYS_CHN_NUM);
    if (ret != TD_SUCCESS)
    {
        sample_print("failed with %#x!\n", ret);
    }

    return ret;
}

static td_void sample_venc_vpss_deinit(ot_vpss_grp vpss_grp, sample_venc_vpss_chn_attr *vpss_chan_cfg)
{
    td_s32 ret;

    ret = sample_common_vpss_stop(vpss_grp, vpss_chan_cfg->enable, OT_VPSS_MAX_PHYS_CHN_NUM);
    if (ret != TD_SUCCESS)
    {
        sample_print("failed with %#x!\n", ret);
    }
}

static td_s32 sample_venc_init_param(ot_size *enc_size, td_s32 chn_num_max, ot_size *vi_size,
                                     sample_venc_vpss_chn_attr *vpss_param)
{
    td_s32 i;
    td_s32 ret;
    ot_pic_size pic_size[CHN_NUM_MAX] = {BIG_STREAM_SIZE, SMALL_STREAM_SIZE};

    for (i = 0; i < chn_num_max && i < CHN_NUM_MAX; i++)
    {
        ret = sample_comm_sys_get_pic_size(pic_size[i], &enc_size[i]);
        // ret = sample_comm_sys_get_pic_size(pic_size[1], &enc_size[i]);

        if (ret != TD_SUCCESS)
        {
            sample_print("sample_comm_sys_get_pic_size failed!\n");
            return ret;
        }
    }

    // enc_size[0].width = 1920;
    // enc_size[0].height = 1080;
    // get vpss param
    get_default_vpss_chn_attr(vi_size, enc_size, CHN_NUM_MAX, vpss_param);

    return 0;
}

static td_void sample_venc_set_video_param(sample_comm_venc_chn_param *chn_param,
                                           ot_venc_gop_attr gop_attr, td_s32 chn_num_max, td_bool qp_map)
{
    td_u32 profile[CHN_NUM_MAX] = {0, 0};
    td_bool share_buf_en = TD_TRUE;
    ot_pic_size pic_size[CHN_NUM_MAX] = {BIG_STREAM_SIZE, SMALL_STREAM_SIZE};
    ot_payload_type payload[CHN_NUM_MAX] = {OT_PT_H265, OT_PT_H264};
    sample_rc rc_mode[CHN_NUM_MAX];
    int i,j;
    // if (get_rc_mode(payload[0], &rc_mode) != TD_SUCCESS)
    // {
    //     return;
    // }
    for (i = 0; i < CHN_NUM_MAX; i++)
    {
        if (runVideoCfg.vencStream[i].h264Conf.rc_mode == 0)
            rc_mode[i] = SAMPLE_RC_CBR;
        else
            rc_mode[i] = SAMPLE_RC_VBR;
    }

        for (i = 0; i < CHN_NUM_MAX; i++)
    {
        if (runVideoCfg.vencStream[i].enctype == 1)
            payload[i] = OT_PT_H264;
        else
            payload[i] = OT_PT_H265;
    }

    if (runVideoCfg.vencStream[0].h264Conf.bps < 1000 || runVideoCfg.vencStream[0].h264Conf.bps > 8000)
    {
        printf("bps setting error!\nuse default value\n");
        chn_param[0].bitrate_x = 4096;
    }
    else
        chn_param[0].bitrate_x = runVideoCfg.vencStream[0].h264Conf.bps;

    if (runVideoCfg.vencStream[0].h264Conf.bps < 300 || runVideoCfg.vencStream[0].h264Conf.bps > 3072)
    {
        printf("bps setting error!\nuse default value\n");
        chn_param[1].bitrate_x = 3072;
    }
    else
        chn_param[1].bitrate_x = runVideoCfg.vencStream[0].h264Conf.bps;

    if (runVideoCfg.vencStream[0].h264Conf.width == 1920)
    {
        pic_size[0] = PIC_1080P;
    }
    else if (runVideoCfg.vencStream[0].h264Conf.width == 1280 && runVideoCfg.vencStream[0].h264Conf.height == 1024)
    {
        pic_size[0] = PIC_1280X1024;
    }
    else if (runVideoCfg.vencStream[0].h264Conf.width == 1280 && runVideoCfg.vencStream[0].h264Conf.height == 720)
    {
        pic_size[0] = PIC_720P;
    }



    if (runVideoCfg.vencStream[1].h264Conf.width == 1280)
    {
        pic_size[1] = PIC_720P;
    }
    else if (runVideoCfg.vencStream[1].h264Conf.width == 720)
    {
        pic_size[1] = PIC_576P;
    }
    else if (runVideoCfg.vencStream[1].h264Conf.width == 640)
    {
        pic_size[1] = PIC_360P;
    }

    /* encode h.265 */
    chn_param[0].gop_attr = gop_attr;
    chn_param[0].type = payload[0];
    chn_param[0].size = pic_size[0];
    chn_param[0].rc_mode = rc_mode[0];
    chn_param[0].profile = profile[0];
    chn_param[0].is_rcn_ref_share_buf = share_buf_en;
    chn_param[0].frame_rate = runVideoCfg.vencStream[0].h264Conf.fps;
    /* encode h.264 */
    chn_param[1].gop_attr = gop_attr;
    chn_param[1].type = payload[1];
    chn_param[1].size = pic_size[1];
    chn_param[1].rc_mode = rc_mode[1];
    chn_param[1].profile = profile[1];
    chn_param[1].is_rcn_ref_share_buf = share_buf_en;
    chn_param[1].frame_rate = runVideoCfg.vencStream[1].h264Conf.fps;
}

static td_void sample_set_venc_vpss_chn(sample_venc_vpss_chn *venc_vpss_chn)
{
    td_s32 i;

    for (i = 0; i < CHN_NUM_MAX; i++)
    {
        venc_vpss_chn->vpss_chn[i] = i;
        venc_vpss_chn->venc_chn[0] = 3;
        venc_vpss_chn->venc_chn[1] = 4;
    }

    printf("======================venc chnn = %d\n", venc_vpss_chn->venc_chn[0]);
    printf("======================venc chnn = %d\n", venc_vpss_chn->venc_chn[1]);
}

static td_void sample_venc_unbind_vpss_stop(ot_vpss_grp vpss_grp, const sample_venc_vpss_chn *venc_vpss_chn)
{
    td_s32 i;

    for (i = 0; i < CHN_NUM_MAX; i++)
    {
        sample_comm_vpss_un_bind_venc(vpss_grp, venc_vpss_chn->vpss_chn[i], venc_vpss_chn->venc_chn[i]);
        sample_comm_venc_stop(venc_vpss_chn->venc_chn[i]);
    }
}

static td_void sample_venc_stop(const sample_venc_vpss_chn *venc_vpss_chn)
{
    td_s32 i;

    for (i = 0; i < CHN_NUM_MAX; i++)
    {
        sample_comm_venc_stop(venc_vpss_chn->venc_chn[i]);
    }
}

static void change_state(int signo)
{
    // printf("get this signo\n");

    return EXIT_MODE_X = 0;
}

static int get_stream_from_one_channl(int s_LivevencChn, rtsp_demo_handle g_rtsplive,
                                      rtsp_session_handle session)
{
    static int s_LivevencFd = 0;
    static int s_maxFd = 0;
    td_s32 ret = 0;
    fd_set read_fds;

    int nSize;
    int i;
    ot_venc_stream stVStream;

    ot_venc_chn_status stStat;
    struct timeval TimeoutVal;
    TimeoutVal.tv_sec = 2;
    TimeoutVal.tv_usec = 0;

    s_LivevencFd = ss_mpi_venc_get_fd(s_LivevencChn);
    s_maxFd = s_maxFd > s_LivevencFd ? s_maxFd : s_LivevencFd;
    s_maxFd = s_maxFd + 1;


    // printf("=======================channel = %d==============", s_LivevencChn);

    FD_ZERO(&read_fds);
    FD_SET(s_LivevencFd, &read_fds);

    ret = select(s_maxFd, &read_fds, NULL, NULL, &TimeoutVal);
    if (ret <= 0)
    {
        printf("%s select failed!\n", __FUNCTION__);
        // sleep(1);
        // continue;
        return -1;
    }

    // Live stream
    if (FD_ISSET(s_LivevencFd, &read_fds))
    {
        ret = ss_mpi_venc_query_status(s_LivevencChn, &stStat);
        if (TD_SUCCESS != ret)
        {
            printf("ss_mpi_venc_query_status chn[%d] failed with %#x!\n", s_LivevencChn, ret);
            // continue;
            return -1;
        }
        stVStream.pack = (ot_venc_pack *)malloc(sizeof(ot_venc_pack) * stStat.cur_packs);
        stVStream.pack_cnt = stStat.cur_packs;
        ret = ss_mpi_venc_get_stream(s_LivevencChn, &stVStream, TD_TRUE);
        if (TD_SUCCESS != ret)
        {
            printf("ss_mpi_venc_get_stream .. failed with %#x!\n", ret);
            // continue;
            return -1;
        }
        unsigned char *pStremData;
        for (i = 0; i < stVStream.pack_cnt; i++)
        {
            pStremData = (unsigned char *)stVStream.pack[i].addr + stVStream.pack[i].offset;
            nSize = stVStream.pack[i].len - stVStream.pack[i].offset;

            if (g_rtsplive)
            {
                rtsp_sever_tx_video(g_rtsplive, session, pStremData, nSize, stVStream.pack[i].pts);
            }
        }
        ret = ss_mpi_venc_release_stream(s_LivevencChn, &stVStream);
        if (TD_SUCCESS != ret)
        {
            sample_print("ss_mpi_venc_release_stream chn[%d] .. failed with %#x!\n", s_LivevencChn, ret);
            free(stVStream.pack);
            stVStream.pack = NULL;
            // continue;
            return -1;
        }

        free(stVStream.pack);
        stVStream.pack = NULL;
    }
}

/******************************************************************************
 * funciton : get stream from each channels and save them
 ******************************************************************************/
td_void *VENC_GetVencStreamProc(td_void *p)
{
    td_s32 ret = 0;
    int i;
    // static ot_venc_chn s_LivevencChn = 0;
    // rtsp_handle_struct rtsp_p[2];
    // rtsp_handle_struct rtsp_p[0] = *(rtsp_handle_struct *)p[0];
    // rtsp_handle_struct rtsp_p[1] = *(rtsp_handle_struct *)p[1];
    printf("=========chn = %d\n", rtsp_handle[0].channel_num);
    printf("=========chn = %d\n", rtsp_handle[1].channel_num);
    // s_LivevencChn = rtsp_p.channel_num;
    struct timeval TimeoutVal;
    TimeoutVal.tv_sec = 2;
    TimeoutVal.tv_usec = 0;
    ot_audio_stream stAStream;
    while (End_Rtsp)
    {
        
        for (i = 0; i < CHN_NUM_MAX; i++)
        // for (i = 0; i < 1; i++)
        {
            ret = get_stream_from_one_channl(rtsp_handle[i].channel_num, rtsp_handle[i].g_rtsplive,
                                             rtsp_handle[i].session);
            if (ret < 0)
                End_Rtsp = 0;
        }
    }
    return NULL;
}

static td_s32 sample_venc_normal_start_encode(ot_vpss_grp vpss_grp, sample_venc_vpss_chn *venc_vpss_chn)
{
    td_s32 ret;
    ot_venc_gop_mode gop_mode;
    ot_venc_gop_attr gop_attr;
    sample_comm_venc_chn_param chn_param[CHN_NUM_MAX] = {0};
    sample_comm_venc_chn_param *h265_chn_param = TD_NULL;
    sample_comm_venc_chn_param *h264_chn_param = TD_NULL;
    int i;

    if (get_gop_mode(&gop_mode) != TD_SUCCESS)
    {
        return TD_FAILURE;
    }
    if ((ret = sample_comm_venc_get_gop_attr(gop_mode, &gop_attr)) != TD_SUCCESS)
    {
        sample_print("Venc Get GopAttr for %#x!\n", ret);
        return ret;
    }

    sample_venc_set_video_param(chn_param, gop_attr, CHN_NUM_MAX, TD_FALSE);

      /* encode h.265 */

    h265_chn_param = &(chn_param[0]);
    if ((ret = sample_comm_venc_start(venc_vpss_chn->venc_chn[0], h265_chn_param)) != TD_SUCCESS)
    {
        sample_print("Venc Start failed for %#x!\n", ret);
        return ret;
    }

    ret = sample_comm_vpss_bind_venc(vpss_grp, venc_vpss_chn->vpss_chn[0], venc_vpss_chn->venc_chn[0]);
    if (ret != TD_SUCCESS)
    {
        sample_print("sample_comm_vpss_bind_venc failed for %#x!\n", ret);
        goto EXIT_VENC_H265_STOP;
    }
      /* encode h.264 */

    h264_chn_param = &(chn_param[1]);
    if ((ret = sample_comm_venc_start(venc_vpss_chn->venc_chn[1], h264_chn_param)) != TD_SUCCESS)
    {
        sample_print("Venc Start failed for %#x!\n", ret);
        goto EXIT_VENC_H264_UnBind;
    }

    ret = sample_comm_vpss_bind_venc(vpss_grp, venc_vpss_chn->vpss_chn[1], venc_vpss_chn->venc_chn[1]);
    if (ret != TD_SUCCESS)
    {
        sample_print("sample_comm_vpss_bind_venc failed for %#x!\n", ret);
        goto EXIT_VENC_H264_STOP;
    }
    // }
    rtsp_handle[0].g_rtsplive = create_rtsp_demo(554);

    rtsp_handle[0].channel_num = venc_vpss_chn->venc_chn[0];
    rtsp_handle[1].g_rtsplive = create_rtsp_demo(8554);
    rtsp_handle[1].channel_num = venc_vpss_chn->venc_chn[1];
    char rtsp_name[20];
    for (i = 0; i < CHN_NUM_MAX; i++)
    {
	sprintf(rtsp_name, "%s%d","/stream",i);
	printf("========rtsp_name=============%s\n",rtsp_name);
        if (chn_param[i].type == OT_PT_H265)
            rtsp_handle[i].session = create_rtsp_session(rtsp_handle[i].g_rtsplive, rtsp_name, 1);
        else
            rtsp_handle[i].session = create_rtsp_session(rtsp_handle[i].g_rtsplive, rtsp_name, 0);
    }

    pthread_create(&venc_audio_pthread[3], 0, VENC_GetVencStreamProc, NULL);
    pthread_detach(venc_audio_pthread[3]);


#if 0
        printf("press s to save video\n");
    char save = getchar();
    if (save == 's'){

    /******************************************
     stream save process
    ******************************************/
    if ((ret = sample_comm_venc_start_get_stream(venc_vpss_chn->venc_chn, CHN_NUM_MAX)) != TD_SUCCESS) {
        sample_print("Start Venc failed!\n");
        goto EXIT_VENC_H264_UnBind;
    }
    }
#endif
    while (EXIT_MODE_X)
    {
        signal(SIGINT, change_state);
    }

     printf("============end rtsp==============\n");
    // End_Rtsp = 0;

    //return TD_SUCCESS;

EXIT_VENC_H264_UnBind:
    sample_comm_vpss_un_bind_venc(vpss_grp, venc_vpss_chn->vpss_chn[1], venc_vpss_chn->venc_chn[1]);
    // sample_comm_vpss_un_bind_venc(vpss_grp, venc_vpss_chn->vpss_chn[0], venc_vpss_chn->venc_chn[0]);
EXIT_VENC_H264_STOP:
    sample_comm_venc_stop(venc_vpss_chn->venc_chn[1]);
    //sample_comm_venc_stop(venc_vpss_chn->venc_chn[0]);
EXIT_VENC_H265_UnBind:
    sample_comm_vpss_un_bind_venc(vpss_grp, venc_vpss_chn->vpss_chn[0], venc_vpss_chn->venc_chn[0]);
EXIT_VENC_H265_STOP:
    sample_comm_venc_stop(venc_vpss_chn->venc_chn[0]);

    return ret;
    // return NULL;
}

static td_void sample_venc_exit_process()
{
   // printf("please press twice ENTER to exit this sample\n");
   // (td_void) getchar();

   // if (g_sample_venc_exit != TD_TRUE)
   // {
   //     (td_void) getchar();
   // }
    //sample_comm_venc_stop_get_stream(CHN_NUM_MAX);
    int chn_id[2] = {3, 4};
    sample_comm_venc_stop_get_stream_x(chn_id,CHN_NUM_MAX);
}

/*
    hisi ap:SAMPLE_BIN_IMPORTBINDATA
    editor:xqq
 */
static td_s32 SAMPILE_BIN_ImportBinData(PQ_BIN_MODULE_S *pstBinParam, unsigned char *pBuffer, unsigned int dataLen)
{
    unsigned int tempLen;
    unsigned int size;
    int ret;

    FILE *pFile = fopen("./sample.bin", "r");
    if (pFile == NULL)
    {
        printf("fopen error.\n");
        ret = -1;
        goto EXIT;
    }
    fseek(pFile, 0, SEEK_END);
    size = ftell(pFile);
    fseek(pFile, 0, SEEK_SET);

    unsigned char *pDataBuffer = (unsigned char *)malloc(size);
    if (pDataBuffer == NULL)
    {
        printf("malloc erro\n");
        ret = -1;
        goto EXIT;
    }
    tempLen = fread(pDataBuffer, sizeof(unsigned char), size, pFile);
    if (tempLen <= 0)
    {
        printf("read erro\n");
        ret = -1;
        goto EXIT;
    }
    ret = OT_PQ_BIN_ImportBinData(pstBinParam, pDataBuffer, size);
    if (ret != 0)
    {
        printf("OT_PQ_BIN_ImportBinData error! errno(%#x)\n", ret);
    }
    else
    {
        printf("OT_PQ_BIN_ParseBinData success!\n");
    }

EXIT:
    free(pDataBuffer);
    pDataBuffer = NULL;
    if (pFile != NULL)
    {
        fclose(pFile);
    }

    return ret;
}

/*
    load_pq file function
    editor:xqq
 */
int load_pq_bin()
{
    int ret;
    unsigned int totalLen, ispDataLen, nrxDataLen;
    unsigned char *pBuffer;
    PQ_BIN_MODULE_S stBinParam;

    memset_s(&stBinParam, sizeof(stBinParam), 0, sizeof(stBinParam));

    stBinParam.stISP.enable = 1;
    stBinParam.st3DNR.enable = 1;
    stBinParam.st3DNR.viPipe = 0;
    stBinParam.st3DNR.vpssGrp = 0;
    ispDataLen = OT_PQ_GetISPDataTotalLen();
    nrxDataLen = OT_PQ_GetStructParamLen(&stBinParam);
    totalLen = nrxDataLen + ispDataLen;
    pBuffer = (unsigned char *)malloc(totalLen);
    if (pBuffer == NULL)
    {
        printf("malloc err!\n");
        return -1;
    }
    memset_s(pBuffer, totalLen, 0, totalLen);

    ret = SAMPILE_BIN_ImportBinData(&stBinParam, pBuffer, totalLen);

    free(pBuffer);
    pBuffer = NULL;

    return ret;
}

static td_s32 sample_snap_start_snap(td_void)
{
    td_s32 ret;
    const ot_venc_chn venc_chn = 0; /* 1: snap venc */

    ret = ss_mpi_snap_set_pipe_attr(0, &g_norm_snap_attr);
    if (ret != TD_SUCCESS)
    {
        printf("ss_mpi_snap_set_pipe_attr failed, ret: 0x%x\n", ret);
        return TD_FAILURE;
    }

    ret = ss_mpi_snap_enable_pipe(0);
    if (ret != TD_SUCCESS)
    {
        printf("ss_mpi_snap_enable_pipe failed, ret: 0x%x\n", ret);
        return TD_FAILURE;
    }

    printf("=======press Enter c to trigger=====\n");
    char trigger;
    trigger = getchar();
    if (trigger == 'c')
    {
        ret = ss_mpi_snap_trigger_pipe(0);
        if (ret != TD_SUCCESS)
        {
            printf("ss_mpi_snap_trigger_pipe failed, ret: 0x%x\n", ret);
            goto exit;
        }

        ret = sample_comm_venc_snap_process(venc_chn, g_norm_snap_attr.norm_attr.frame_cnt, TD_TRUE, TD_TRUE);
        if (ret != TD_SUCCESS)
        {
            printf("snap venc process failed!\n");
            goto exit;
        }
    }
    printf("snap success!\n");

exit:
    ss_mpi_snap_disable_pipe(0);
    return ret;
}

/******************************************************************************
 * function :  H.265e@1080P@30fps + h264e@D1@30fps
 ******************************************************************************/
static td_s32 sample_venc_normal(td_void)
{
    td_s32 ret;
    sample_sns_type sns_type = SC850SL_8M30;
    sample_vi_cfg vi_cfg;
    ot_size enc_size[CHN_NUM_MAX];
    const ot_vi_pipe vi_pipe = 0;
    const ot_vi_chn vi_chn = 0;
    const ot_vpss_grp vpss_grp = 0;
    sample_venc_vpss_chn_attr vpss_param;
    sample_venc_vb_attr vb_attr = {0};
    sample_venc_vpss_chn venc_vpss_chn = {0};

    sample_set_venc_vpss_chn(&venc_vpss_chn);
    sample_comm_vi_get_default_vi_cfg(sns_type, &vi_cfg);

    /******************************************
      step 0: related parameter ready
    ******************************************/
    ret = sample_venc_init_param(enc_size, CHN_NUM_MAX, &(vi_cfg.dev_info.dev_attr.in_size), &vpss_param);
    if (ret != TD_SUCCESS)
    {
        return ret;
    }

    /******************************************
      step 1: init sys alloc common vb
    ******************************************/
    get_vb_attr(&(vi_cfg.dev_info.dev_attr.in_size), &vpss_param, &vb_attr);

    if ((ret = sample_venc_sys_init(&vb_attr)) != TD_SUCCESS)
    {
        sample_print("Init SYS err for %#x!\n", ret);
        return ret;
    }

    if ((ret = sample_venc_vi_init(&vi_cfg)) != TD_SUCCESS)
    {
        sample_print("Init VI err for %#x!\n", ret);
        goto EXIT_SYS_STOP;
    }

    if ((ret = sample_venc_vpss_init(vpss_grp, &vpss_param)) != TD_SUCCESS)
    {
        sample_print("Init VPSS err for %#x!\n", ret);
        goto EXIT_VI_STOP;
    }

    if ((ret = sample_comm_vi_bind_vpss(vi_pipe, vi_chn, vpss_grp, 0)) != TD_SUCCESS)
    {
        sample_print("VI Bind VPSS err for %#x!\n", ret);
        goto EXIT_VPSS_STOP;
    }


    ot_dis_cfg dis_cfg;
    ss_mpi_vi_get_chn_dis_cfg(vi_pipe, vi_chn, &dis_cfg);
    dis_cfg.motion_level = 1;
    dis_cfg.mode = 1;

    ss_mpi_vi_set_chn_dis_cfg(vi_pipe, vi_chn, &dis_cfg);
    ot_dis_attr dis_attr;
    ss_mpi_vi_get_chn_dis_attr(vi_pipe, vi_chn, &dis_attr);
    dis_attr.enable = TD_TRUE;
    dis_attr.gdc_bypass = TD_FALSE;
    ss_mpi_vi_set_chn_dis_attr(vi_pipe, vi_chn, &dis_attr);

    if ((ret = sample_venc_normal_start_encode(vpss_grp, &venc_vpss_chn)) != TD_SUCCESS)
    {
        goto EXIT_VI_VPSS_UNBIND;
    }


    /******************************************
     exit process
    ******************************************/
    sample_venc_exit_process();
    sample_venc_unbind_vpss_stop(vpss_grp, &venc_vpss_chn);

EXIT_VI_VPSS_UNBIND:
    sample_comm_vi_un_bind_vpss(vi_pipe, vi_chn, vpss_grp, 0);
EXIT_VPSS_STOP:
    sample_venc_vpss_deinit(vpss_grp, &vpss_param);
EXIT_VI_STOP:
    sample_venc_vi_deinit(&vi_cfg);
EXIT_SYS_STOP:
    sample_comm_sys_exit();

    return ret;
}

void rtsp_reboot()
{
    int i;
    int retval;
    // printf("=================rtsp reboot===================\n");
    // printf("=================rtsp reboot===================\n");
    // printf("l=================rtsp reboot===================\n");
    End_Rtsp = 0;
    EXIT_MODE_X = 0;
    new_system_call("pkill udhcpc");
    // printf("===========pkill udhcpc==============\n");
    for(i = 0; i<2;i++)
    {
    rtsp_del_session(rtsp_handle[i].session);
    rtsp_del_demo(rtsp_handle[i].g_rtsplive);
    }
    ss_mpi_aenc_aac_deinit();
    // sample_comm_sys_exit();
    sleep(3);
    End_Rtsp = 1;
    EXIT_MODE_X = 1;

    venc_audio_start();
}

/******************************************************************************
 * function    : main()
 * description : video venc sample
 ******************************************************************************/
#if 1
#ifdef __LITEOS__
td_s32 app_main(td_s32 argc, td_char *argv[])
#else
td_s32 venc_audio_start()
#endif
{
    td_s32 ret;
#ifndef __LITEOS__
    sample_sys_signal(sample_venc_handle_sig);
#endif

#if defined(OT_VQE_USE_STATIC_MODULE_REGISTER)
    ret = sample_audio_register_vqe_module();
    if (ret != TD_SUCCESS)
    {
        return TD_FAILURE;
    }
#endif

    printf("====================start rtsp=================\n");
    printf("====================start rtsp=================\n");
    printf("====================start rtsp=================\n");

    pthread_create(&venc_audio_pthread[0], 0, sample_venc_normal, NULL);
    pthread_detach(venc_audio_pthread[0]);
    sleep(1);
    ss_mpi_aenc_aac_init();
    if (runVideoCfg.vencStream[0].avStream == 0 || runVideoCfg.vencStream[1].avStream == 0)
    {
        pthread_create(&venc_audio_pthread[1], 0, sample_audio_ai_aenc, NULL);
	    pthread_detach(venc_audio_pthread[1]);
    }

}
#endif
