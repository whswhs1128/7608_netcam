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
#include "adi_audio.h"
#include "sample_comm.h"
#include "rtsp_demo.h"
#include "comm.h"
#include "cfg_audio.h"
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
#include "SDL.h"
#include "SDL_ttf.h"
#define FONT_PATH "./hisi_osd.ttf"
#define OVERLAYEX_MIN_HANDLE 20
ot_bmp stBitmap;
int bmp_w, bmp_h;
#include "cfg_video.h"
#include "sdk_enc.h"
#include "media_fifo.h"
GK_NET_VIDEO_CFG runVideoCfg;
static ot_payload_type g_payload_type = OT_PT_G711A;
static td_bool g_aio_resample = TD_FALSE;
static td_bool g_user_get_mode = TD_FALSE;
static td_bool g_ao_volume_ctrl = TD_FALSE;
static ot_audio_sample_rate g_in_sample_rate = OT_AUDIO_SAMPLE_RATE_BUTT;
static ot_audio_sample_rate g_out_sample_rate = OT_AUDIO_SAMPLE_RATE_BUTT;
/* 0: close, 1: record, 2: talk, 3: talkv2 */
static td_u32 g_ai_vqe_type = 1;
static void change_state(int signo);
static void video_read_stream_callback(int stream, PS_GK_ENC_BUF_ATTR frameBuf);
static void audio_read_stream_callback(int stream, PS_GK_ENC_BUF_ATTR frameBuf);

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

static int video_frame_index[4] = {0, 0, 0, 0};
extern MEDIABUF_HANDLE writerid[4];
static MEDIABUF_HANDLE audio_writer[4] = {0, 0, 0, 0};
static GK_ENC_BUG_CALLBACK encStreamCb = NULL;

static td_bool g_sample_venc_exit = TD_FALSE;

static void *abuf = NULL;
static int audio_frame_index = 0;
static int a_num = 0;
static int read_audio_stream(GADI_AUDIO_AioFrameT ptrFrame)
{
    int ret;
    int i;

    ST_GK_ENC_BUF_ATTR header = {0};

#ifdef AUDIO_ECHO_CANCELLATION_SUPPORT
    GADI_AEC_AioFrameT aecFrame = {0};
    // ret = gadi_audio_ai_get_frame_aec(&ptrFrame, &aecFrame, GADI_TRUE);	xqq
    // LOG_INFO("a frame len:%d, aecframe len:%d \n", ptrFrame.len, aecFrame.len);
// #else		xqq
// ret = gadi_audio_ai_get_frame(&ptrFrame, GADI_TRUE);	xqq
// LOG_INFO("a frame len:%d\n", ptrFrame.len);
#endif
    if ((ptrFrame.len == 0))
    {
        printf("ret: %d ptrFrame.len: %d\n", ret, ptrFrame.len);
        return -1;
    }

    /* ��Ϊ160�ֽ�һ����Ƶ����һ����������2��160�ֽڵ���Ƶ֡���һ���µ�320�ֽڵ���Ƶ֡ */
    if (abuf == NULL)
    {
        abuf = malloc(ptrFrame.len * 2);
    }
    if (a_num == 0)
    {
        memcpy(abuf, ptrFrame.virAddr, ptrFrame.len);
        a_num++;
        return 0;
    }
    if (a_num == 1)
    {
        memcpy(abuf + ptrFrame.len, ptrFrame.virAddr, ptrFrame.len);
        a_num = 0;
    }
    header.data = abuf;
    header.data_sz = ptrFrame.len * 2;

    cal_audio_pts(&(header.time_us), header.data_sz);

    header.no = (++audio_frame_index);

    audio_read_stream_callback(0, &header);
    return 0;
}

static void audio_read_stream_callback(int stream, PS_GK_ENC_BUF_ATTR frameBuf)
{
    GK_NET_FRAME_HEADER header = {0};
    short *prawData = NULL;
    int outLen = 0;

    // ��Ƶ����٣�����Ƶ���ݶ���
    if ((runVideoCfg.vencStream[stream].avStream != 0) || (audio_writer[stream] == NULL))
    {
        // PRINT_ERR("no audio.\n");
        usleep(10000);
        return;
    }
    header.magic = MAGIC_TEST;
    header.device_type = 0;
    header.frame_size = frameBuf->data_sz;
    header.pts = frameBuf->time_us * (runAudioCfg.sampleRate / 8000);

    header.media_codec_type = MEDIA_CODEC_ID_PCM_ALAW;
    header.frame_type = GK_NET_FRAME_TYPE_A;
    header.frame_no = frameBuf->no;

    /* ����ʱ�䣬CMS��Ҫʱ�� */
    struct timeval tv = {0};
    struct timezone tz = {0};
    gettimeofday(&tv, &tz);
    header.sec = tv.tv_sec - tz.tz_minuteswest * 60;
    header.usec = tv.tv_usec;

    /* Remove G711A Encoder Header */
    if ((g_payload_type == OT_PT_G711A) ||
        (g_payload_type = OT_PT_G711U))
    {
        prawData = (short *)malloc(frameBuf->data_sz);
        if (NULL != prawData)
        {

           // outLen = gk_audio_get_rawstream((short *)frameBuf->data, prawData, (frameBuf->data_sz / sizeof(short)));
           // header.frame_size = outLen * sizeof(short);
	    header.frame_size = frameBuf->data_sz;
            if (g_payload_type == OT_PT_G711A)
            {
                header.media_codec_type = MEDIA_CODEC_ID_PCM_ALAW;
            }
            if (g_payload_type == OT_PT_G711U)
            {
                header.media_codec_type = MEDIA_CODEC_ID_PCM_MULAW;
            }
            mediabuf_write_frame(audio_writer[stream], prawData, outLen * sizeof(short), &header);
            free(prawData);
            prawData = NULL;
        }
    }
    else
    {
        mediabuf_write_frame(audio_writer[stream], frameBuf->data, frameBuf->data_sz, &header);
    }
}

td_void *SAMPLE_COMM_AUDIO_AencProc_new(td_void *p)
{

    td_s32 ret = 0;
    int i = 0;
    static int s_aencFd = 0;
    static int s_maxAFd = 0;
    GADI_AUDIO_AioFrameT ptrFrame = {0};
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
                if (i == 1)
                {
                    ptrFrame.virAddr = (unsigned char *)stAStream.stream;
                    ptrFrame.len = stAStream.len;
                    read_audio_stream(ptrFrame);
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

ot_aio_attr aio_attr = {0};
static td_s32 sample_audio_ai_aenc(td_void)
{
    td_s32 ret;
    ot_audio_dev ai_dev;
    ot_audio_dev ao_dev;
    td_u32 ai_chn_cnt;
    td_u32 aenc_chn_cnt;
    td_bool send_adec = TD_TRUE;

    int i = 0;
    for (i = 0; i < 2; i++)
    {
        audio_writer[i] = mediabuf_add_writer(i);
        if (audio_writer[i] == NULL)
        {
            PRINT_ERR("Create audio writer error\n");
        }
    }

    // ot_aio_attr aio_attr = {0};
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
        usleep(500 * 1000);
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

int string_to_bmp(char *pu8Str)
{
    SDL_PixelFormat *fmt;
    TTF_Font *font;
    SDL_Surface *text, *temp;
    if (TTF_Init() < 0)
    {
        fprintf(stderr, "Couldn't initialize TTF: %s\n", SDL_GetError());
        SDL_Quit();
    }

    font = TTF_OpenFont(FONT_PATH, 40); // change size
    if (font == NULL)
    {
        fprintf(stderr, "Couldn't load %d pt font from %s: %s\n", 18, "ptsize", SDL_GetError());
    }

    SDL_Color forecol = {0xff, 0xff, 0xff, 0xff};
    text = TTF_RenderUTF8_Solid(font, pu8Str, forecol);

    fmt = (SDL_PixelFormat *)malloc(sizeof(SDL_PixelFormat));
    memset(fmt, 0, sizeof(SDL_PixelFormat));
    fmt->BitsPerPixel = 16;
    fmt->BytesPerPixel = 2;
    //    fmt->colorkey = 0xffffffff;
    //    fmt->alpha = 0xff;

    temp = SDL_ConvertSurface(text, fmt, 0);
    // stBitmap.data = malloc(2 * (temp->w) * (temp->h));
    // if (stBitmap.data == NULL)
    // {
    //     printf("stBitmap.data faided\r\n");
    // }
    // 奇数会�?�致内�?�变成斜�?
    if (temp->w % 2 != 0)
        bmp_w = temp->w + 1;
    else
        bmp_w = temp->w;
    if (temp->h % 2 != 0)
        bmp_h = temp->h + 1;
    else
        bmp_h = temp->h;
    // bmp_w = temp->w;
    // bmp_h = temp->h;
    // memset(stBitmap.data, 0, (2 * (temp->w) * (temp->h)));
    // memcpy(stBitmap.data, temp->pixels, (2 * (temp->w) * (temp->h)));

    // stBitmap.width = temp->w;
    // stBitmap.height = temp->h;
    memset(stBitmap.data, 0, (2 * bmp_w * bmp_h));
    memcpy(stBitmap.data, temp->pixels, (2 * temp->w * temp->h));
    stBitmap.width = bmp_w;
    stBitmap.height = bmp_h;

    // char savename[20] = {0};

    // snprintf(savename, 20, "./osd/now_time.bmp");
    //  printf("savename = %s\n",savename);
    // SDL_SaveBMP(temp, savename);
    free(fmt);
    SDL_FreeSurface(text);
    SDL_FreeSurface(temp);
    TTF_CloseFont(font);
    TTF_Quit();

    return 0;
}

/*
 *描述  ：用于osd 字体bmp图像生成
 *参数  ：NULL
 *返回值：�?
 *注意  ：需要加载字体ttf才能使用，否则会报�?�错�?
 */
void *bitmap_update(void)
{
    ot_rgn_handle OverlayHandle = 0;
    td_s32 s32Ret;
    int z = 0;
    // time_t now;
    // struct tm *ptm;
    // char timestr[OSD_LENGTH] = {0};
    while (1)
    {
        sleep(1);

        //        if(p->labelN[0]==0 )
        //	{
        //		z++;
        //		if(z == 10)
        //		{
        //			z = 0;
        //			memset(stBitmap.data, 0, (2 * 3840 * 48));
        //			ss_mpi_rgn_update_canvas(OVERLAYEX_MIN_HANDLE);
        //			s32Ret = ss_mpi_rgn_set_bmp(OVERLAYEX_MIN_HANDLE,&stBitmap);
        //		}
        //		continue;
        //	}

        ss_mpi_rgn_update_canvas(OVERLAYEX_MIN_HANDLE);
        s32Ret = ss_mpi_rgn_set_bmp(OVERLAYEX_MIN_HANDLE, &stBitmap); // s32Ret 为RGN_HANDLE OverlayHandle
        if (s32Ret != TD_SUCCESS)
        {
            printf("HI_MPI_RGN_SetBitMap update failed with %#x!\n", s32Ret);
            // return -1;
            continue;
        }

        // memset(stBitmap.data, 0, (2 * (bmp_w) * (bmp_h)));
        memset(stBitmap.data, 0, (2 * 3840 * 48));
        z = 0;
    }
    return 0;
}

void *osd_ttf_task(void)
{
    ot_rgn_handle OverlayHandle = 0;
    int s32Ret;
    time_t now;
    struct tm *ptm;
    char timestr[128] = {0};
    int i, j;
    char b[3];
    stBitmap.data = malloc(2 * 3840 * 48);
    if (stBitmap.data == NULL)
    {
        printf("stBitmap.data faided\r\n");
    }
    while (1)
    {
        usleep(1000 * 1000);
        time(&now);
        ptm = localtime(&now);
        snprintf(timestr, 100, "时间:%d-%02d-%02d %02d:%02d:%02d", ptm->tm_year + 1900, ptm->tm_mon + 1, ptm->tm_mday, ptm->tm_hour, ptm->tm_min, ptm->tm_sec);

#if 0
        if(p->labelN[0]==0)
        {
	        continue;
            timestr[0] = ' ';
            // printf("=====no license=======\n");
        }
        else
        {
            for(i = 0;i < p->labelN[0];i++)
            {
                for(j=1;j<=15;j++)
                {
                    if(p->labelN[(i*15)+j] < 154 && p->labelN[(i*15)+j] != 0)
                    {
                        b[0] = platename[p->labelN[(i*15)+j]];
                        b[1] = platename[p->labelN[(i*15)+j]+1];
                        b[2] = platename[p->labelN[(i*15)+j]+2];
                    }
                    else if(p->labelN[(i*15)+j] != 0)
                    {
                        b[0] = platename[p->labelN[(i*15)+j]];
                    }
                    strcat(timestr,b);
                    memset(b,0,3);
                }    
	
    
    	strcat(timestr,"  ");
#endif
        //    printf("===========timestr================%s\n",timestr);
        //        strcat(timestr," ");
        string_to_bmp(timestr);
        memset(timestr, 0, 128);
    }
    return 0;
}

/*
 *描述  ：用于将视�?�文件添加时间水�?
 *参数  ：无
 *返回值：OverlayHandle
 *注意  ：参数在HI_MPI_RGN_Create并不做�?�查，�?有在HI_MPI_RGN_AttachToChn的时候才会报出相应的�?
 */
td_s32 RGN_AddOsdToVenc(void)
{
    td_s32 s32Ret;
    ot_rgn_attr stRgnAttr;
    ot_rgn_chn_attr stChnAttr;
    ot_mpp_chn stChn;
    ot_rgn_handle OverlayHandle;
    int handle_num = 1;
    int i = 0;
    int ret;
    // RGN_CANVAS_INFO_S stCanvasInfo;
    OverlayHandle = 0;
    stChn.mod_id = OT_ID_VPSS; /**模块�?**/ // HI_ID_VPSS  HI_ID_VENC
    stChn.dev_id = 0;                       /**设�?�号**/
    stChn.chn_id = 0;                       /**通道�?**/
    /**创建区域**/
    sleep(2); // 等待位图生成
    stRgnAttr.attr.overlay.canvas_num = 2;
    stRgnAttr.type = OT_RGN_OVERLAYEX;                                              /**区域类型:叠加**/
    stRgnAttr.attr.overlay.pixel_format = OT_PIXEL_FORMAT_ARGB_1555; /**像素格式**/ // PIXEL_FORMAT_BGR_565 PIXEL_FORMAT_ARGB_1555
    if (stBitmap.width % 2 != 0)
    {
        stBitmap.width += 1;
    }

    if (stBitmap.height % 2 != 0)
    {
        stBitmap.height += 1;
    }
    printf("stBitmap.width is %d ,stBitmap.height is %d\n", stBitmap.width, stBitmap.height);
    if (stBitmap.width == 0 || stBitmap.height == 0)
    {
        stBitmap.height = 4;
        stBitmap.width = 4;
    }
    stRgnAttr.attr.overlay.size.width = 560;  // 240;        /**区域�?**/
    stRgnAttr.attr.overlay.size.height = 48;  // 192;        /**区域�?**/
    stRgnAttr.attr.overlay.bg_color = 0x0000; // 0x00007c00; /**区域背景颜色**/

    for (i = OVERLAYEX_MIN_HANDLE; i < OVERLAYEX_MIN_HANDLE + handle_num; i++)
    {
        ret = ss_mpi_rgn_create(i, &stRgnAttr);
        if (ret != TD_SUCCESS)
        {
            sample_print("ss_mpi_rgn_create failed with %#x!\n", ret);
            return TD_FAILURE;
        }
    }

    // s32Ret = ss_mpi_rgn_create(OverlayHandle, &stRgnAttr);
    // if (s32Ret != TD_SUCCESS)
    // {
    //     printf("RGN create failed: %#x\n", s32Ret);
    // return -1;
    // }
    /**将区域叠加到通道**/
    /**设置叠加区域的通道显示属�?**/
    stChnAttr.is_show = TD_TRUE;
    stChnAttr.type = OT_RGN_OVERLAYEX;
    // stChnAttr.attr.overlay_chn.point.x = 640; // 240;
    // stChnAttr.attr.overlay_chn.point.y = 320; // 192;
    stChnAttr.attr.overlay_chn.point.x = 0; // 240;
    stChnAttr.attr.overlay_chn.point.y = 0; // 192;
    stChnAttr.attr.overlay_chn.bg_alpha = 128;
    stChnAttr.attr.overlay_chn.fg_alpha = 128;
    stChnAttr.attr.overlay_chn.layer = OverlayHandle;

    /**设置QP属�?**/
    stChnAttr.attr.overlay_chn.qp_info.is_abs_qp = TD_TRUE;
    stChnAttr.attr.overlay_chn.qp_info.qp_val = 0;
    stChnAttr.attr.overlay_chn.qp_info.enable = TD_TRUE;

    /**定义 OSD 反色相关属�?**/
    /**单元反色区域，反色�?�理的基�?单元,[16, 64]，需 16 对齐**/
#if 0
        stChnAttr.attr.overlay_chn.stInvertColor.stInvColArea.height = 16;
        stChnAttr.attr.overlay_chn.stInvertColor.stInvColArea.width  = 16;

        /**�?度阈�?,取值范围：[0, 255]**/
        stChnAttr.attr.overlay_chn.stInvertColor.u32LumThresh = 128;//128

        /**OSD 反色触发模式**/
        stChnAttr.attr.overlay_chn.stInvertColor.enChgMod     = LESSTHAN_LUM_THRESH;

        /**OSD 反色开关。overlay不支持反�?**/
        stChnAttr.attr.overlay_chn.stInvertColor.bInvColEn    = TD_FALSE;
#endif
    stChnAttr.attr.overlay_chn.dst = OT_RGN_ATTACH_JPEG_MAIN;
    // OverlayHandle = 0;
    for (i = OVERLAYEX_MIN_HANDLE; i < OVERLAYEX_MIN_HANDLE + handle_num; i++)
    {
        // sample_region_get_overlayex_chn_attr(i, &chn_attr->attr.overlayex_chn);
        ret = sample_region_attach_to_chn(i, &stChn, &stChnAttr);
        if (ret != TD_SUCCESS)
        {
            sample_print("sample_region_attach_to_chn failed!\n");
            sample_comm_region_detach_frm_chn(i - OVERLAYEX_MIN_HANDLE + 1, OT_RGN_OVERLAYEX, &stChn);
            return ret;
        }
    }

    stBitmap.pixel_format = OT_PIXEL_FORMAT_ARGB_1555;

    for (i = OVERLAYEX_MIN_HANDLE; i < OVERLAYEX_MIN_HANDLE + handle_num; i++)
    {
        s32Ret = ss_mpi_rgn_set_bmp(i, &stBitmap);
    }
    if (s32Ret != TD_SUCCESS)
    {
        printf("HI_MPI_RGN_SetBitMap failed with %#x!\n", s32Ret);
        return -1;
    }

    stChn.chn_id = 1; /**通道�?**/
    for (i = OVERLAYEX_MIN_HANDLE; i < OVERLAYEX_MIN_HANDLE + handle_num; i++)
    {
        ret = sample_region_attach_to_chn(i, &stChn, &stChnAttr);
        if (ret != TD_SUCCESS)
        {
            sample_print("sample_region_attach_to_chn failed!\n");
            sample_comm_region_detach_frm_chn(i - OVERLAYEX_MIN_HANDLE + 1, OT_RGN_OVERLAYEX, &stChn);
            return ret;
        }
    }
    return 0;
}

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
    int i, j;
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

    for (i = 1; i < CHN_NUM_MAX; i++)
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

static int get_stream_from_one_channl(int s_LivevencChn, int s_LivevencFd, fd_set read_fds, rtsp_demo_handle g_rtsplive,
                                      rtsp_session_handle session, ST_GK_ENC_BUF_ATTR header)
{

    // static int s_maxFd = 0;
    td_s32 ret = 0;
    int nSize;
    int i;
    ot_venc_stream stVStream;

    ot_venc_chn_status stStat;
    struct timeval TimeoutVal;
    TimeoutVal.tv_sec = 0;
    TimeoutVal.tv_usec = 500 * 1000;
    int keyframe = 0;
    // s_maxFd = s_maxFd > s_LivevencFd ? s_maxFd : s_LivevencFd;
    // s_maxFd = s_maxFd + 1;

    ret = select(s_LivevencFd + 1, &read_fds, NULL, NULL, &TimeoutVal);
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
            if (s_LivevencChn == 4)
            {
                header.data_sz = nSize;
                header.data = pStremData;

                if (stVStream.pack[i].data_type.h264_type == OT_VENC_H264_NALU_IDR_SLICE || stVStream.pack[i].data_type.h265_type == OT_VENC_H265_NALU_IDR_SLICE)
                {
                    // printf("=================keyframe = 1=============\n");
                    keyframe = 1; // 是I�?
                }
                if (nSize > 500 * 1024)
                {
                    printf("video_frame index:stream_id:%d size:%d, larger than 500KB\n", 4, nSize);
                }
                header.video.keyframe = keyframe;

                // encStreamCb(0, &header);    //0ֻ�Ƕ�Ӧ��buffer��ţ�����Ӧ�ҵı���ͨ�� 0->4
                video_read_stream_callback(0, &header);
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
    return 1;
}

/******************************************************************************
 * funciton : get stream from each channels and save them
 ******************************************************************************/
td_void *VENC_GetVencStreamProc(td_void *p)
{
    td_s32 ret = 0;
    int i;
    static int s_LivevencFd[2] = {0};
    fd_set read_fds;
    FD_ZERO(&read_fds);
    ST_GK_ENC_BUF_ATTR header = {0};
    // if (runVideoCfg.vencStream[1].enctype == 1)
    // {
    //     header.type = GK_ENC_DATA_H264;
    // }
    // else if (3 == runVideoCfg.vencStream[1].enctype)
    // {
    //     header.type = GK_ENC_DATA_H265;
    // }
    header.type = GK_ENC_DATA_H264;
    cal_video_pts(&header.time_us, 0);
    header.video.fps = 30;
    header.video.width = runVideoCfg.vencStream[1].h264Conf.width;
    header.video.height = runVideoCfg.vencStream[1].h264Conf.height;
    // printf("===============================header.video.width = %d\n",header.video.width);
    for (i = 0; i < CHN_NUM_MAX; i++)
    {
        s_LivevencFd[i] = ss_mpi_venc_get_fd(rtsp_handle[i].channel_num);
        FD_SET(s_LivevencFd[i], &read_fds);
    }
    printf("=========chn = %d\n", rtsp_handle[0].channel_num);
    printf("=========chn = %d\n", rtsp_handle[1].channel_num);

    while (End_Rtsp)
    {

        for (i = 0; i < CHN_NUM_MAX; i++)
        // for (i = 0; i < 1; i++)
        // for (i = 1; i >= 0; i--)
        {
            ret = get_stream_from_one_channl(rtsp_handle[i].channel_num, s_LivevencFd[i], read_fds, rtsp_handle[i].g_rtsplive,
                                             rtsp_handle[i].session, header);
            // if (ret < 0)
            //     continue;
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

    rtsp_handle[0].g_rtsplive = create_rtsp_demo(554);

    rtsp_handle[0].channel_num = venc_vpss_chn->venc_chn[0];
    rtsp_handle[1].g_rtsplive = create_rtsp_demo(8554);
    rtsp_handle[1].channel_num = venc_vpss_chn->venc_chn[1];
    char rtsp_name[20];
    for (i = 0; i < CHN_NUM_MAX; i++)
    {
        sprintf(rtsp_name, "%s%d", "/stream", i);
        printf("========rtsp_name=============%s\n", rtsp_name);
        if (chn_param[i].type == OT_PT_H265)
            rtsp_handle[i].session = create_rtsp_session(rtsp_handle[i].g_rtsplive, rtsp_name, 1);
        else
            rtsp_handle[i].session = create_rtsp_session(rtsp_handle[i].g_rtsplive, rtsp_name, 0);
    }

    pthread_create(&venc_audio_pthread[3], 0, VENC_GetVencStreamProc, NULL);
    pthread_detach(venc_audio_pthread[3]);

    while (EXIT_MODE_X)
    {
        usleep(500 * 1000);
    }

    printf("============end rtsp==============\n");
    // End_Rtsp = 0;

    // return TD_SUCCESS;

EXIT_VENC_H264_UnBind:
    sample_comm_vpss_un_bind_venc(vpss_grp, venc_vpss_chn->vpss_chn[1], venc_vpss_chn->venc_chn[1]);
    // sample_comm_vpss_un_bind_venc(vpss_grp, venc_vpss_chn->vpss_chn[0], venc_vpss_chn->venc_chn[0]);
EXIT_VENC_H264_STOP:
    sample_comm_venc_stop(venc_vpss_chn->venc_chn[1]);
    // sample_comm_venc_stop(venc_vpss_chn->venc_chn[0]);
// EXIT_VENC_H265_UnBind:
//     sample_comm_vpss_un_bind_venc(vpss_grp, venc_vpss_chn->vpss_chn[0], venc_vpss_chn->venc_chn[0]);
EXIT_VENC_H265_STOP:
    sample_comm_venc_stop(venc_vpss_chn->venc_chn[0]);

    return ret;
    // return NULL;
}

td_s32 sample_venc_start_svp_x(ot_vpss_grp vpss_grp, sample_venc_vpss_chn *venc_vpss_chn)
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

    // ret = sample_comm_vpss_bind_venc(vpss_grp, venc_vpss_chn->vpss_chn[0], venc_vpss_chn->venc_chn[0]);
    // if (ret != TD_SUCCESS)
    // {
    //     sample_print("sample_comm_vpss_bind_venc failed for %#x!\n", ret);
    //     goto EXIT_VENC_H265_STOP;
    // }
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
        sprintf(rtsp_name, "%s%d", "/stream", i);
        printf("========rtsp_name=============%s\n", rtsp_name);
        if (chn_param[i].type == OT_PT_H265)
            rtsp_handle[i].session = create_rtsp_session(rtsp_handle[i].g_rtsplive, rtsp_name, 1);
        else
            rtsp_handle[i].session = create_rtsp_session(rtsp_handle[i].g_rtsplive, rtsp_name, 0);
    }

    pthread_create(&venc_audio_pthread[3], 0, VENC_GetVencStreamProc, NULL);
    pthread_detach(venc_audio_pthread[3]);
    return TD_SUCCESS;
EXIT_VENC_H264_UnBind:
    sample_comm_vpss_un_bind_venc(vpss_grp, venc_vpss_chn->vpss_chn[1], venc_vpss_chn->venc_chn[1]);
    // sample_comm_vpss_un_bind_venc(vpss_grp, venc_vpss_chn->vpss_chn[0], venc_vpss_chn->venc_chn[0]);
EXIT_VENC_H264_STOP:
    sample_comm_venc_stop(venc_vpss_chn->venc_chn[1]);
    // sample_comm_venc_stop(venc_vpss_chn->venc_chn[0]);
// EXIT_VENC_H265_UnBind:
//     sample_comm_vpss_un_bind_venc(vpss_grp, venc_vpss_chn->vpss_chn[0], venc_vpss_chn->venc_chn[0]);
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
    // sample_comm_venc_stop_get_stream(CHN_NUM_MAX);
    int chn_id[2] = {3, 4};
    sample_comm_venc_stop_get_stream_x(chn_id, CHN_NUM_MAX);
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

static void video_read_stream_callback(int stream, PS_GK_ENC_BUF_ATTR frameBuf)
{
    GK_NET_FRAME_HEADER header = {0};
    int ret;
    header.magic = MAGIC_TEST;
    header.device_type = 0;
    header.frame_size = frameBuf->data_sz;
    header.pts = frameBuf->time_us;

    /* ����ʱ�䣬CMS��Ҫʱ�� */
    struct timeval tv = {0};
    struct timezone tz = {0};
    gettimeofday(&tv, &tz);
    header.sec = tv.tv_sec - tz.tz_minuteswest * 60;
    header.usec = tv.tv_usec;

    if (frameBuf->video.keyframe)
        header.frame_type = GK_NET_FRAME_TYPE_I;
    else
        header.frame_type = GK_NET_FRAME_TYPE_P;

    if (frameBuf->type == GK_ENC_DATA_H264) // h264
    {
        header.media_codec_type = 0; // CODEC_ID_H264;
    }
    else if (frameBuf->type == GK_ENC_DATA_H265) // h265
    {
        header.media_codec_type = 6; // CODEC_ID_H265;
    }

    header.frame_rate = frameBuf->video.fps;
    header.video_reso = ((frameBuf->video.width << 16) + frameBuf->video.height);
    header.frame_no = (++video_frame_index[stream]);
    ret = mediabuf_write_frame(writerid[stream], frameBuf->data, frameBuf->data_sz, &header);
    // printf("mediabuf_write_frame ret = %d\n",mediabuf_write_frame);
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

    pthread_t osd_task;
    pthread_create(&osd_task, NULL, osd_ttf_task, NULL);
    pthread_detach(osd_task);

    RGN_AddOsdToVenc();
    pthread_t bitmap_update_t;
    pthread_create(&bitmap_update_t, NULL, bitmap_update, NULL);
    pthread_detach(bitmap_update_t);

    // sdk_set_get_video_stream_cb(video_read_stream_callback);

    sample_venc_normal_start_encode(vpss_grp, &venc_vpss_chn);
    // if ((ret = sample_venc_normal_start_encode(vpss_grp, &venc_vpss_chn)) != TD_SUCCESS)
    // {
    //     goto EXIT_VI_VPSS_UNBIND;
    // }

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
    printf("=================rtsp reboot===================\n");
    printf("=================rtsp reboot===================\n");
    printf("l=================rtsp reboot===================\n");
    End_Rtsp = 0;
    EXIT_MODE_X = 0;
    new_system_call("pkill udhcpc");
    printf("===========pkill udhcpc==============\n");
    for (i = 0; i < 2; i++)
    {
        rtsp_del_session(rtsp_handle[i].session);
        rtsp_del_demo(rtsp_handle[i].g_rtsplive);
    }
    ss_mpi_aenc_aac_deinit();
    // sample_comm_sys_exit();
    printf("===========runVideoCfg.vencStream[1].h264Conf.width = %d===========\n", runVideoCfg.vencStream[1].h264Conf.width);
    sleep(3);
    End_Rtsp = 1;
    EXIT_MODE_X = 1;

    venc_audio_start();
    // RGN_AddOsdToVenc();
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
    if (runVideoCfg.vencStream[0].avStream == 0 && runVideoCfg.vencStream[1].avStream == 0)
    {
        pthread_create(&venc_audio_pthread[1], 0, sample_audio_ai_aenc, NULL);
        pthread_detach(venc_audio_pthread[1]);
    }
    // #ifdef __LITEOS__
    //     return TD_SUCCESS;
    // #else
    //     exit(TD_SUCCESS);
    // #endif
}
#endif
