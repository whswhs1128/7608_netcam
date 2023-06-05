#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "sdk_debug.h"
#include "sdk_vision.h"
#include "sdk_af.h"
#include "ttl.h"
#include "ot_common_isp.h"
#include "ss_mpi_isp.h"
#include "type.h"
#include "ot_type.h"

#define BLEND_SHIFT 6
#define ALPHA 64 // 1
#define BELTA 54 // 0.85
#define FEINFO

extern int uart;
static int af_weight[15][17] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 4, 4, 4, 4, 4, 4, 4, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 4, 8, 8, 8, 8, 8, 8, 8, 4, 1, 1, 1, 0},
    {0, 1, 1, 1, 4, 8, 8, 16, 16, 16, 8, 8, 4, 1, 1, 1, 0},
    {0, 1, 1, 4, 8, 8, 16, 16, 32, 16, 16, 8, 8, 4, 1, 1, 0},
    {0, 1, 1, 4, 8, 8, 16, 32, 32, 32, 16, 8, 8, 4, 1, 1, 0},
    {0, 1, 1, 4, 8, 8, 16, 16, 32, 16, 16, 8, 8, 4, 1, 1, 0},
    {0, 1, 1, 1, 4, 8, 8, 16, 16, 16, 8, 8, 4, 1, 1, 1, 0},
    {0, 1, 1, 1, 4, 8, 8, 8, 8, 8, 8, 8, 4, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 4, 4, 4, 4, 4, 4, 4, 1, 1, 1, 1, 0},
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0},
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0},
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0},
};

static GK_BOOL g_afStopSignal = GK_FALSE;
static pthread_t g_afThread = 0;

static void *af_calc(void *arg)
{
    td_s32 ret = TD_SUCCESS;
    td_u8 wdr_chn;
    td_u32 frm_cnt = 0;
    td_u32 i, j, k;
    td_u16 stat_data;
    ot_vi_pipe vi_pipe = 0;
    ot_isp_af_stats af_stats;
    ot_isp_stats_cfg stats_cfg;
    ot_isp_exp_info stIspExpInfo;
    ot_isp_exposure_attr pstExpAttr;
    ot_isp_focus_zone zone_metrics[OT_ISP_WDR_MAX_FRAME_NUM][OT_ISP_AF_ZONE_ROW]
                                  [OT_ISP_AF_ZONE_COLUMN] = {0};
    ot_isp_focus_stats_cfg focus_cfg = {
        {1, 17, 15, 1, 0, {0, 0, 0, 1920, 1080}, {0, 0, 0, 1920, 1080}, 0, {0x2, 0x4, 0}, {1, 0x9bff}, 0xf0},
        {1, {1, 1, 1}, 15, {188, 414, -330, 486, -461, 400, -328}, {7, 0, 3, 1}, {1, 0, 255, 0, 240, 8, 14}, {127, 12, 2047}},
        {0, {1, 1, 0}, 2, {200, 200, -110, 461, -415, 0, 0}, {6, 0, 1, 0}, {0, 0, 0, 0, 0, 0, 0}, {15, 12, 2047}},
        {{20, 16, 0, -16, -20}, {1, 0, 255, 0, 220, 8, 14}, {38, 12, 1800}},
        {{-12, -24, 0, 24, 12}, {1, 0, 255, 0, 220, 8, 14}, {15, 12, 2047}},
        {4, {0, 0}, {1, 1}, 0}};

    sdk_sys_thread_set_name("af_calc");
    pthread_detach(pthread_self());
    ret = ss_mpi_isp_get_stats_cfg(vi_pipe, &stats_cfg);
    if (TD_SUCCESS != ret)
    {
        printf("ss_mpi_isp_get_stats_cfg error!(ret = 0x%x)\n", ret);
        return TD_FAILURE;
    }
    memcpy(&stats_cfg.focus_cfg, &focus_cfg, sizeof(ot_isp_focus_stats_cfg));
    ret = ss_mpi_isp_set_stats_cfg(vi_pipe, &stats_cfg);
    if (TD_SUCCESS != ret)
    {
        printf("ss_mpi_isp_set_stats_cfg error!(ret = 0x%x)\n", ret);
        return TD_FAILURE;
    }

    send_absolutemove_value(uart, 0, 0, 0);
    while (g_afStopSignal)
    {
        ret = ss_mpi_isp_get_vd_time_out(vi_pipe, OT_ISP_VD_FE_START, 5000);
        ret |= ss_mpi_isp_get_focus_stats(vi_pipe, &af_stats);
        if (ret != TD_SUCCESS)
        {
            printf("ss_mpi_isp_get_focus_stats error!(ret = 0x%x)\n", ret);
            return TD_FAILURE;
        }
#ifdef FEINFO
        memcpy(zone_metrics, &af_stats.fe_af_stat, sizeof(ot_isp_focus_zone) * OT_ISP_WDR_MAX_FRAME_NUM * OT_ISP_AF_ZONE_ROW * OT_ISP_AF_ZONE_COLUMN);
#else
        memcpy(zone_metrics[0], &af_stats.be_af_stat, sizeof(ot_isp_focus_zone) * OT_ISP_AF_ZONE_ROW * OT_ISP_AF_ZONE_COLUMN);
#endif
      
        td_u32 sum_fv1 = 0;
        td_u32 sum_fv2 = 0;
        td_u32 wgt_sum = 0;
        td_u32 fv1_n, fv2_n, fv1, fv2;
        wdr_chn = 1;
        for (k = 0; k < wdr_chn; k++)
        {
            for (i = 0; i < focus_cfg.config.zone_row; i++)
            {
                for (j = 0; j < focus_cfg.config.zone_col; j++)
                {
                    td_u32 h1 = zone_metrics[k][i][j].h1;
                    td_u32 h2 = zone_metrics[k][i][j].h2;
                    td_u32 v1 = zone_metrics[k][i][j].v1;
                    td_u32 v2 = zone_metrics[k][i][j].v2;
                    fv1_n = (h1 * ALPHA + v1 * ((1 << BLEND_SHIFT) - ALPHA)) >> BLEND_SHIFT;
                    fv2_n = (h2 * BELTA + v2 * ((1 << BLEND_SHIFT) - BELTA)) >> BLEND_SHIFT;
                    sum_fv1 += af_weight[i][j] * fv1_n;
                    sum_fv2 += af_weight[i][j] * fv2_n;
                    wgt_sum += af_weight[i][j];
                }
            }
        }
        fv1 = sum_fv1 / wgt_sum;
        fv2 = sum_fv2 / wgt_sum;

        GK_U8 agc = 255 * (stIspExpInfo.a_gain - pstExpAttr.auto_attr.a_gain_range.min) / (pstExpAttr.auto_attr.a_gain_range.max - pstExpAttr.auto_attr.a_gain_range.min);
        send_af_value(uart, fv1, fv2, agc);
        usleep(50 * 1000);
    }
    return NULL;
}

int sdk_af_lens_init(af_lens_cb cb)
{
    int ret;
    //    sdk_lens_init(cb);
    g_afStopSignal = GK_TRUE;
    ret = pthread_create(&g_afThread, NULL, af_calc, NULL);
    if (ret != 0)
        LOG_ERR("Create af_calc thread failed,Please try again:%s", strerror(errno));
    return ret;
}

int sdk_af_lens_exit()
{
    g_afStopSignal = GK_FALSE;
    return 0;
}
