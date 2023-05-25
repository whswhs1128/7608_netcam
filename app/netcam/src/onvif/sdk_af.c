#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include "af_comm.h" 
#include "gk_api_isp.h"
#include "sdk_debug.h"
#include "sdk_vision.h"
#include "comm_vpss.h"
#include "sdk_af.h"
#include "ttl.h"

#define BLEND_SHIFT 6
#define ALPHA 64 // 1
#define BELTA 54 // 0.85
//#define FEINFO
#define GAST

extern int uart;
static int AFWeight[15][17] = {
    {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}, 
    {0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}, 
    {0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0}, 
    {0, 1, 1, 1, 1, 4, 4, 4, 4, 4, 4, 4, 1, 1, 1, 1, 0}, 
    {0, 1, 1, 1, 4, 8, 8, 8, 8, 8, 8, 8, 4, 1, 1, 1, 0}, 
    {0, 1, 1, 1, 4, 8, 8,16,16,16, 8, 8, 4, 1, 1, 1, 0}, 
    {0, 1, 1, 4, 8, 8,16,16,32,16,16, 8, 8, 4, 1, 1, 0}, 
    {0, 1, 1, 4, 8, 8,16,32,32,32,16, 8, 8, 4, 1, 1, 0}, 
    {0, 1, 1, 4, 8, 8,16,16,32,16,16, 8, 8, 4, 1, 1, 0}, 
    {0, 1, 1, 1, 4, 8, 8,16,16,16, 8, 8, 4, 1, 1, 1, 0}, 
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
    int s32Ret = 0;
    GK_U32 i, j, k;
    GK_U32 u32SumFv1, u32SumFv2, u32WgtSum, u32WgtSum1;
    GK_U32 u32Fv1_n, u32Fv2_n, u32Fv1, u32Fv2;    
    GK_U32 u32H1, u32H2, u32V1, u32V2;    
    GK_U8 u8WdrChn;
    VI_PIPE ViPipe = 0;
    ISP_EXPOSURE_ATTR_S pstExpAttr;
    ISP_EXP_INFO_S stIspExpInfo;
    ISP_AF_STATISTICS_S stIspStatics;
    ISP_STATISTICS_CFG_S stIspStaticsCfg;
    ISP_FOCUS_ZONE_S stZoneMetrics[WDR_CHN_MAX][AF_ZONE_ROW][AF_ZONE_COLUMN] = {0};
//    ISP_FOCUS_STATISTICS_CFG_S stFocusCfg = 
//    {
//        {1, 17, 15, 4096, 2160, 1, 0, {0, 0, 0, 4096, 2160}, 2, {0x2, 0x1, 0}, {1, 0x9bff}, 0xf0},
//        {1, {1, 1, 1}, 15,{188, 414, -330, 486, -461, 400, -328}, {7, 0, 3, 1}, {1, 0, 255, 0, 220, 8, 14}, {127, 12, 2047} },
//        {0, {1, 1, 0}, 2, {200, 200, -110, 461, -415, 0, 0}, {6, 0, 1, 0}, {0, 0, 0, 0, 0, 0, 0 }, {15, 12, 2047} },
//        {{20, 16, 0, -16, -20}, {1, 0, 255, 0, 220, 8, 14}, {38, 12, 1800} },
//        {{-12, -24, 0, 24, 12}, {1, 0, 255, 0, 220, 8, 14}, {15, 12, 2047} },
//        {4, {0, 0}, {0, 0}, 0}
//    };

    ISP_FOCUS_STATISTICS_CFG_S stFocusCfg = 
    {
        {1, 17, 15, 1920, 1080, 1, 0, {0, 0, 0, 1920, 1080}, 2, {0x2, 0x1, 0}, {1, 0x9bff}, 0xf0},
        {1, {1, 1, 1}, 15,{188, 414, -330, 486, -461, 400, -328}, {7, 0, 3, 1}, {1, 0, 255, 0, 220, 8, 14}, {127, 12, 2047} },
        {0, {1, 1, 0}, 2, {200, 200, -110, 461, -415, 0, 0}, {6, 0, 1, 0}, {0, 0, 0, 0, 0, 0, 0 }, {15, 12, 2047} },
        {{20, 16, 0, -16, -20}, {1, 0, 255, 0, 220, 8, 14}, {38, 12, 1800} },
        {{-12, -24, 0, 24, 12}, {1, 0, 255, 0, 220, 8, 14}, {15, 12, 2047} },
        {4, {0, 0}, {0, 0}, 0}
    };

    sdk_sys_thread_set_name("af_calc");
    pthread_detach(pthread_self());
    //s32Ret = GK_API_ISP_GetStatisticsConfig(ViPipe, &stIspStaticsCfg);
    if (GK_SUCCESS != s32Ret)
    {
        LOG_ERR("GK_API_ISP_GetStatisticsConfig error!(s32Ret = 0x%x)\n", s32Ret);
        return GK_FAILURE;
    }
    stIspStaticsCfg.unKey.bit1BEAfStat = 1;
    memcpy(&stIspStaticsCfg.stFocusCfg, &stFocusCfg, sizeof(ISP_FOCUS_STATISTICS_CFG_S));
    //s32Ret = GK_API_ISP_SetStatisticsConfig(ViPipe, &stIspStaticsCfg);
    if (GK_SUCCESS != s32Ret)
    {
        LOG_ERR("GK_API_ISP_SetStatisticsConfig error!(s32Ret = 0x%x)\n", s32Ret);
        return GK_FAILURE;
    }

    send_absolutemove_value(uart, 0, 0, 0);
    while (g_afStopSignal)
    {
       // s32Ret = GK_API_ISP_GetVDTimeOut(ViPipe, ISP_VD_FE_START, 5000);
       // s32Ret |= GK_API_ISP_GetFocusStatistics(ViPipe, &stIspStatics);
        if (GK_SUCCESS != s32Ret)
        {
            LOG_ERR("GK_API_ISP_GetFocusStatistics error!(s32Ret = 0x%x)\n", s32Ret);
            return GK_FAILURE;
        }
#ifdef FEINFO
        memcpy(stZoneMetrics, &stIspStatics.stFEAFStat, sizeof(ISP_FOCUS_ZONE_S) * WDR_CHN_MAX * AF_ZONE_ROW * AF_ZONE_COLUMN);
#else
        memcpy(stZoneMetrics[0], &stIspStatics.stBEAFStat, sizeof(ISP_FOCUS_ZONE_S) * AF_ZONE_ROW * AF_ZONE_COLUMN);
#endif
        u32SumFv1 = 0;
        u32SumFv2 = 0;
        u32WgtSum = 0;
        u32WgtSum1 = 0;
        u8WdrChn = 1; 
        for(k = 0; k < u8WdrChn; k++) 
        { 
            for ( i = 0 ; i < stFocusCfg.stConfig.u16Vwnd; i++ ) 
            { 
                for ( j = 0 ; j < stFocusCfg.stConfig.u16Hwnd; j++ ) 
                { 
                    u32H1 = stZoneMetrics[k][i][j].u16h1;
                    u32H2 = stZoneMetrics[k][i][j].u16h2;
                    u32V1 = stZoneMetrics[k][i][j].u16v1;
                    u32V2 = stZoneMetrics[k][i][j].u16v2;
                    u32Fv2_n = (u32H2 * BELTA + u32V2 * ((1<<BLEND_SHIFT) - BELTA)) >> BLEND_SHIFT;
                    #ifndef GAST
                    u32Fv1_n = (u32H1 * ALPHA + u32V1 * ((1<<BLEND_SHIFT) - ALPHA)) >> BLEND_SHIFT;
                    u32SumFv1 += AFWeight[i][j] * u32Fv1_n;
                    #else
                    if(i>4 && i<(stFocusCfg.stConfig.u16Vwnd-5) && j>4 && j<(stFocusCfg.stConfig.u16Hwnd-5))
                    {
                        u32SumFv1 += AFWeight[i][j] * u32Fv2_n;
                        u32WgtSum1 += AFWeight[i][j];
                    }
                    #endif                    
                    u32SumFv2 += AFWeight[i][j] * u32Fv2_n;
                    u32WgtSum += AFWeight[i][j];
                }
            } 
        } 
        #ifndef GAST
        u32Fv1 = u32SumFv1 / u32WgtSum;
        u32Fv2 = u32SumFv2 / u32WgtSum;
        #else
        u32Fv1 = u32SumFv1 / (u32WgtSum1>>3);
        u32Fv2 = u32SumFv2 / (u32WgtSum>>3);
        #endif
       // s32Ret = GK_API_ISP_QueryExposureInfo(ViPipe, &stIspExpInfo);
       // s32Ret = GK_API_ISP_GetExposureAttr(ViPipe, &pstExpAttr);
        GK_U8 agc = 255*(stIspExpInfo.u32AGain-pstExpAttr.stAuto.stAGainRange.u32Min)/(pstExpAttr.stAuto.stAGainRange.u32Max - pstExpAttr.stAuto.stAGainRange.u32Min);
	send_af_value(uart, u32Fv1, u32Fv2, agc);
	usleep(50 * 1000);
	//        pelco_set_agc_and_focus_value(agc, (short)u32Fv1, (short)u32Fv2);
    } 
    return NULL; 
}

int sdk_af_lens_init(af_lens_cb cb)
{
    int ret;
//    sdk_lens_init(cb);
    g_afStopSignal = GK_TRUE;
    ret = pthread_create(&g_afThread, NULL, af_calc, NULL);
	if(ret != 0)
		LOG_ERR("Create af_calc thread failed,Please try again:%s",strerror(errno));
    return ret;
}

int sdk_af_lens_exit()
{
    g_afStopSignal = GK_FALSE;
    return 0;
}

