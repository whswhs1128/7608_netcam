/******************************************************************************
** \file        gkvision\demo\venc.c
**
** \brief       gkvision demo.
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#include "adi_types.h"
#include "adi_sys.h"
#include "adi_pda.h"
#include "gkcommon.h"
#include "gk_bd.h"
//#include "netcam_api.h"
//#include "cfg_common.h"

#include "sdk_cfg.h"
#include "sdk_enc.h"
#include "sdk_pda.h"
#include "sdk_vision.h"


//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************

#define RED          "\033[0;32;31m"
#define GREEN        "\033[0;32;32m"
#define BLUE         "\033[0;32;34m"
#define YELLOW       "\033[1;33m"
#define WHITE        "\033[1;37m"

#define SDK_VISION_DEBUG
#ifdef SDK_VISION_DEBUG
#define gkvision_printk(S,...)      printf("[%s:%s-%d]"S"\033[0;0m\n",GREEN,__func__,__LINE__, ##__VA_ARGS__)
#define gkvision_error(S,...)       printf("[%s:%s-%d]"S"\033[0;0m\n",RED,__func__,__LINE__, ##__VA_ARGS__)
#define gkvision_warning(S,...)     printf("[%s:%s-%d]"S"\033[0;0m\n",YELLOW,__func__,__LINE__, ##__VA_ARGS__)
#else
#define gkvision_printk(S,...)
#define gkvision_error(S,...)       printf("[%s:%s-%d]"S"\033[0;0m\n",RED,__func__,__LINE__, ##__VA_ARGS__)
#define gkvision_warning(S,...)
#endif


#define VENC_STREAM3_WIDTH   640//640//320
#define VENC_STREAM3_HEIGHT  480//360//180

#define MAX_DETECT 8
#define TRUE  1
#define FALSE 0


#define MIN_SIZE_MAX        256
#define MIN_SIZE_MIN        64
#define MIN_SIZE_SCALE      100
#define MIN_SIZE_STEP       ((MIN_SIZE_MAX-MIN_SIZE_MIN)/(MIN_SIZE_SCALE-1))


//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************


//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************

//static GADI_PDA_Buffer_InfoT pstAttr;
//static ST_GK_ENC_STREAM_H264_ATTR h264Attr;
static GADI_PDA_Buffer_InfoT pda_info;

static unsigned char running_enable = FALSE;
static unsigned char pause_enable = FALSE;

static int hdSensitivity = 0;
static GV_InitParam initParam;
static GADI_INT sensiTable[100] = {64,64,64,65,65,65,66,66,66,67,67,67,68,68,68,69,69,69,70,70,
                                   70,71,71,71,72,72,72,73,73,73,74,74,74,75,75,75,76,76,76,77,
                                   77,77,78,78,78,79,79,79,80,80,80,81,81,81,82,82,82,83,83,83,
                                   84,84,84,85,85,85,86,86,86,87,87,87,88,88,88,89,89,89,90,90,
                                   90,91,91,91,92,92,93,93,94,94,95,95,96,96,97,97,98,98,99,99};


//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************

static GK_VISION_NOTIFY visionNotify  = NULL;

extern GADI_PDA_DataT *sdk_pda_get_pda_p(void);

static int encoder_pda_cap_yuv(GADI_U8* yuvaddr, GADI_U32 yuvsize)
{
     int ret = 0;
     SDK_PDA_YUV_ATTR_T info;
     GADI_S32 row,col;
     GADI_PDA_CAP_Buffer_TypeT buffer;
     GADI_S32 uv_width, uv_height,uv_pitch;
    
    // buffer = GADI_PDA_CAP_SUB_A_BUFFER; //second channel
     buffer = GADI_PDA_CAP_SUB_A_BUFFER; //third channel

     ret = sdk_pda_get_yuv(0, buffer, &info);
     if(ret != GADI_OK)
     {
         GADI_ERROR("gadi_pda_capture_buffer: failed:%d\n", ret);
         return -1;
     }
     if (yuvsize < info.width*info.height)
     {
         GADI_ERROR("gadi_pda_capture_buffer buffer size %d*%d\n", info.width, info.height);
         return -1;
     }
    
     //printf("gadi_pda_capture_buffer %d*%d frameFormat:%d\n", info.width, info.height, info.frameFormat);
    
     if (info.stride == info.width)
     {
         memcpy(yuvaddr, info.yAddr, info.width * info.height);
         yuvaddr = yuvaddr + info.width*info.height;
     }
     else if (info.stride > info.width)
     {
         for (row = 0; row < info.height; row++) {
             memcpy(yuvaddr, info.yAddr, info.width);
             info.yAddr = info.yAddr + info.stride;
             yuvaddr = yuvaddr + info.width;
         }
     }
     else
     {
         GADI_ERROR("stride size smaller than width!\n");
         return -1;
     }
    
     if (SDK_FORMAT_YUV_SEMIPLANAR_420 == info.frameFormat) {
         uv_pitch  = info.stride / 2;
         uv_width  = info.width / 2;
         uv_height = info.height / 2;
     } else { // FRAME_FORMAT_YUV_SEMIPLANAR_422
         uv_pitch  = info.stride / 2;
         uv_width  = info.width / 2;
         uv_height = info.height;
     }
    
     GADI_U8 * output_u = yuvaddr;
     GADI_U8 * output_v = yuvaddr+info.width * info.height / 4;
    
     GADI_U8 * output_uv = info.uvAddr;
    
     for (row = 0; row < uv_height; row++) {
         for (col = 0; col < uv_width; col++) {
             if (SDK_FORMAT_YUV_SEMIPLANAR_420 == info.frameFormat)
             {
                 *output_u++ = *output_uv++;
                 *output_v++ = *output_uv++;
             }
             else
             {
                 if (row%2)
                 {
                     *output_u++ = *output_uv++;
                     *output_v++ = *output_uv++;
                 }
                 else
                 {
                     output_uv += 2;
                 }
             }
         }
         output_uv += (uv_pitch - uv_width) * 2;
     }
    
     return 0;

}


static void* sample_yuv_bd(void *arg)
{
    GADI_U8* ybuffer0 = NULL;
    GADI_U32 buffersize;
    int ret;
    //GV_Image lastFrame, curFrame;
    GV_Image gmdFrame, yFrame, uFrame, vFrame;
    GADI_U32 w,h;
    GADI_U32 idx;
    GADI_U8* lceData = NULL;
    //GADI_U32 firstFrame = 1;
    GADI_U32 mdFlag = 0;
    GADI_PDA_DataT* pstAttr = NULL;
    int idSetI = 0;
    int buffer;
    int yuvWidth, yuvHeight;
    //GV_InitParam initParam;
    SDK_PDA_ATTR_T pdaAttr;
    SDK_PDA_YUV_ATTR_T info;

    GV_MD_Result mdResult;
    GK_VISION_MD_RESULT result;
    GADI_PDA_DataT *pdaData;
	struct timeval start_tv;
    struct timeval end_tv;
	unsigned long time_val;
    int checkCnt = 0;
    int checkCntLast = 0;
    int timeSleep = 400000;

    
    sdk_sys_thread_set_name("sdkvision");

    memset(&result, 0, sizeof(GK_VISION_MD_RESULT));

    buffer = GADI_PDA_CAP_SUB_A_BUFFER; //third channel
    sdk_pda_get_yuv(0, buffer, &info);
    yuvWidth = info.width;
    yuvHeight = info.height;
    
    buffersize = yuvWidth * yuvHeight
                +yuvWidth * yuvHeight/2;
    ybuffer0 = malloc(buffersize);
    if (NULL == ybuffer0)
    {
        GADI_ERROR("encoder_capture_facerec: malloc failed\n");
        return NULL;
    }
    
    //k_pda_get_attr(0, &pdaAttr);
    pdaData = sdk_pda_get_pda_p();
    pdaAttr.width = pdaData->u32MbWidth;
    pdaAttr.height = pdaData->u32MbHeight;
    //retVal = sdk_pda_get_pda_p();

    if (initParam.useGmd == 1)
    {
//        gmdFrame = gv_image_create(pdaAttr.width, pdaAttr.height);	xqq
	printf(" \n");	//xqq
    }
    else
    {
        gmdFrame.data = NULL;
    }
    if (NULL == gmdFrame.data)
    {
        GADI_ERROR("gv_image_create: malloc failed\n");
    }

    yFrame.w = yuvWidth;
    yFrame.step = yuvWidth;
    yFrame.h = yuvHeight;
    yFrame.data = ybuffer0;

    uFrame.w = yuvWidth/2;
    uFrame.step = yuvWidth/2;
    uFrame.h = yuvHeight/2;
    uFrame.data = yFrame.data+yFrame.w*yFrame.h;

    vFrame.w = yuvWidth/2;
    vFrame.step = yuvWidth/2;
    vFrame.h = yuvHeight/2;
    vFrame.data = uFrame.data+uFrame.w*uFrame.h;

    while(running_enable)
    {
        if (initParam.useGmd)
        {
            pstAttr = sdk_pda_get_pda_p();
            checkCnt = 0;
            if (NULL != pstAttr)
            {
                for (h = 0; h < pstAttr->u32MbHeight; h++)
                {
                    for (w = 0; w < pstAttr->u32MbWidth; w++)
                    {
                        idx = pstAttr->u32MbWidth*h+w;
                        lceData = (GADI_U8*)pstAttr->pAddr;
                        if (lceData[idx/8] & (0x1 << (idx%8)))
                        {
                            gmdFrame.data[idx] = 0xFF;
                            checkCnt++;
                        }
                        else
                        {
                            gmdFrame.data[idx] = 0x0;
                        }
                    }
                }
                pstAttr = NULL;
            }
            else
            {
                GADI_ERROR("gadi_pda_get_data:  error.\n");
                usleep(100000);
                continue;
            }
        }

        ret = encoder_pda_cap_yuv(ybuffer0, buffersize);
        //ret = -1;
        if (ret != 0)
        {
            gkvision_error("encoder_pda_cap_yuv failed.\n");
            sleep(1);
            continue;
        }

        if (pause_enable)
        {
            //gv_md_set_first_frame(yFrame);	xqq
            //usleep(100*1000);
            printf("sdk vision pause....\n");
            sleep(1);
            continue;
        }

        if (checkCntLast < 500 && checkCnt > 1000)
        {
            usleep(300000);
            checkCntLast = checkCnt;
            printf("ignore...\n");
            continue;
        }
        checkCntLast = checkCnt;

        #if 0
        //gmd not set
        if (idSetI == 0)
        {
            gv_md_set_first_frame(yFrame);
            idSetI = 1;
        }
        #endif
        
        gettimeofday(&start_tv,NULL);
//        ret = gv_md_run(gmdFrame, yFrame, uFrame, vFrame, &mdResult);	xqq
        if (ret != 0)
        {
            printf("gv_md_run ret:%d\n", ret);
            continue;
        }
        //printf("--->gv_md_run ret:%d, cnt:%d\n", ret, mdResult.detectCnt);
        gettimeofday(&end_tv,NULL);
        time_val = (end_tv.tv_sec*1000 + end_tv.tv_usec/1000) - (start_tv.tv_sec*1000 + start_tv.tv_usec/1000);
        if (time_val > 100)
            printf("intval time: %d, checkCnt:%d\n",time_val, checkCnt);

        #if 0
        if (time_val > 300)
        {
            timeSleep = 1000000;
        }
        else
        {
            timeSleep = 400000;
        }
        #endif

		#if 0
        if (mdResult.maxMotionCnt >= 0)
        {
            printf("max motion rect:%d,%d, %d,%d, cnt:%d\n", mdResult.maxMotionRect.x, mdResult.maxMotionRect.y,
                mdResult.maxMotionRect.width, mdResult.maxMotionRect.height, mdResult.maxMotionCnt);
        }
		#endif	
        if (mdResult.maxMotionCnt > 0)
        {
            mdFlag = 1;
        }
        
        if (mdResult.detectCnt)
        {
	        for (idx = 0; idx < mdResult.detectCnt; idx++)
	        {
	            gkvision_printk(" idx:%d      mdResult.detectCnt:%d    start(%d,%d)size(%d,%d) score:%d\n",
					idx,mdResult.detectCnt,
	                mdResult.detectInfo[idx].rect.x, mdResult.detectInfo[idx].rect.y,
	                mdResult.detectInfo[idx].rect.width, mdResult.detectInfo[idx].rect.height,
	                mdResult.detectInfo[idx].score);
	        }

            mdFlag = 1;
		}

        //save yuv
        #if 0
        if (mdResult.maxMotionCnt > 0)
        {
            struct tm *ptm;
            long ts;
            struct tm tt = {0};
            char str[128] = {0};
            ts = time(NULL);
            ptm = localtime_r(&ts, &tt);
            sprintf(str, "/mnt/sd_card/%04d%02d%02d_%02d_%02d_%02d_c%d_%d_%d_%d_%d_%d.yuv", ptm->tm_year+1900,
                                                     ptm->tm_mon+1,
                                                     ptm->tm_mday,
                                                     ptm->tm_hour,
                                                     ptm->tm_min,
                                                     ptm->tm_sec,
                                                     mdResult.maxMotionRect.x,
                                                     mdResult.maxMotionRect.y,
                                                     mdResult.maxMotionRect.width,
                                                     mdResult.maxMotionRect.height,
                                                     mdResult.detectCnt);
            
            FILE *fw = fopen(str, "w+");
            printf("save file:%s\n", str);
            if (fw != NULL)
            {
                fwrite(ybuffer0, 1, buffersize, fw);
                fclose(fw);
            }
        }
        #endif

        for (idx = 0; idx < VISION_REGION_MAX; idx++)
        {
            if (mdResult.lineAlarm[idx])
            {
                gkvision_printk("mdResult.lineAlarm------------------[%d]\n", idx);
                mdFlag = 1;
            }


            if (mdResult.roiAlarm[idx])
            {
                gkvision_printk("mdResult.roiAlarm------------------[%d]\n", idx);
                mdFlag = 1;
            }
        }

        if (mdFlag)
        {
            memcpy(&(result.mdResult), &mdResult, sizeof(GV_MD_Result));
            result.buffInfo.addr = ybuffer0;
            result.buffInfo.size = buffersize;

            if (visionNotify)
            {
                visionNotify(&result);
            }

            mdFlag = 0;
            //sleep(10);
            //memset(&result, 0, sizeof(GK_VISION_MD_RESULT));
        }

        usleep(timeSleep);
    }

    free(ybuffer0);

//    ret = gv_exit();	xqq

    running_enable = FALSE;

    return NULL;
}

//*****************************************************************************
//*****************************************************************************
//** APP Functions
//*****************************************************************************
//*****************************************************************************
pthread_t videoThreadId;

int sdk_vision_init(int threshold, int useGmd, GK_VISION_NOTIFY notifyFunc)
{
    int retVal = GADI_OK;
    int buffer;
    SDK_PDA_ATTR_T pdaAttr;
    SDK_PDA_YUV_ATTR_T info;
    GADI_PDA_DataT *pdaData;

    if (running_enable)
    {
        gkvision_printk("sdk_vision_init has been initialized.\n");
        return 0;
    }

#if 0
    ST_GK_ENC_STREAM_H264_ATTR h264Attr;
    sdk_enc_get_stream_h264(0, 2, &h264Attr);
    h264Attr.width  = yuvWidth;
    h264Attr.height = yuvHeight;
    sdk_enc_set_stream_h264(0, 2, &h264Attr);
#endif

#if 0
    pdaHandle = (GADI_SYS_HandleT)sdk_pda_get_pdaHandle(0);
    if (NULL == pdaHandle)
    {
        gkvision_error("sdk_pda_get_pdaHandle error.\n");
        return NULL;
    }
#endif

    printf("gkvison sdk_pda_get_attr..\n");
    //retVal = sdk_pda_get_attr(0, &pdaAttr);
    if (useGmd)
    {
        pdaData = sdk_pda_get_pda_p();
        pdaAttr.width = pdaData->u32MbWidth;
        pdaAttr.height = pdaData->u32MbHeight;
        if (GADI_OK == retVal)
        {
            gkvision_printk("gadi_pda_get_attr %dx%d.\n", pdaAttr.width, pdaAttr.height);
        }
        else
        {
            gkvision_printk("gadi_pda_get_attr error.\n");
            return -1;
        }
        initParam.gmdWidth = pdaAttr.width;
        initParam.gmdHeight = pdaAttr.height;
    }

    initParam.useGmd = useGmd;

    buffer = GADI_PDA_CAP_SUB_A_BUFFER; //third channel
    
    sdk_pda_get_yuv(0, buffer, &info);
    gkvision_printk("yuv width:%d, %d\n", info.width, info.height);

    initParam.yuvWidth = info.width;
    initParam.yuvHeight = info.height;

    initParam.bdEnable = 1;
    initParam.trackEnable = 1;
    initParam.bdThreshold = 1;//sensiTable[100 - threshold];//64;
    initParam.diffThreshold = 4;//6;
    initParam.detectMax     = 1;
    gkvision_printk("gv_init use gmd:%d\n", initParam.useGmd);
//    retVal = gv_init(initParam);	xqq
    if(retVal != 0)
    {
        gkvision_error("gv_init() failed! %d\n", retVal);
        return -1;
    }

    visionNotify = notifyFunc;

    running_enable = TRUE;

    return 0;
}


int sdk_vision_open(void)
{
    pthread_attr_t  attr;
    struct sched_param param;
    int retVal = GADI_OK;

    /*creat get bit stream thread.*/
    if (pthread_attr_init(&attr) < 0)
    {
        return -1;
    }

    
    param.sched_priority = 70;
    pthread_attr_setschedpolicy(&attr,SCHED_RR);
    pthread_attr_setschedparam(&attr,&param);
    pthread_attr_setinheritsched(&attr,PTHREAD_EXPLICIT_SCHED);//要使优先级其作用必须要有这句话

    retVal = pthread_create(&videoThreadId, &attr, sample_yuv_bd, 0);
    if(retVal != 0){
        return -1;
    }

    return 0;
}

int sdk_vision_exit(void)
{
    int retVal = GADI_OK;

    if (0 == running_enable)
    {
        return 0;
    }

    running_enable = FALSE;

    if(videoThreadId != 0){
        pthread_join(videoThreadId, NULL);
    }

//    retVal = gv_exit();	xqq

    return retVal;
}

int sdk_vision_set_vision_sensitivity(int sensitivity)
{
    if (sensitivity != hdSensitivity)
    {
        hdSensitivity = sensitivity;
        /* invert the sensitivity in vision lib */
        //gv_set_bd_threshold(sensiTable[100 - sensitivity]);	xqq
    }

    return 0;
}

int sdk_vision_set_human_detection(GK_VISION_ALARM_HD_CFG *config)
{
    if (NULL == config)
    {
        GADI_ERROR("params error\n");
		return -1;
    }

    if(config->enable)
    {
        sdk_vision_set_pause_state(0);
    }
    else
    {
        sdk_vision_set_pause_state(1);
    }

    sdk_vision_set_vision_sensitivity(config->rect[0].sensitivity);

    return 0;
}

int sdk_vision_set_transboundary_detection(GK_VISION_ALARM_TD_CFG *config)
{
    int i = 0;
    GV_Point start, end;

    if (NULL == config)
    {
        GADI_ERROR("params error\n");
		return -1;
    }
#if 0		//xqq
    if (0 == config->enable)
    {
        for (i = 0; i < VISION_REGION_MAX; i ++)
        {
            gv_md_clear_line(i);
        }
    }
    else
    {
        for (i = 0; i < VISION_REGION_MAX; i ++)
        {
            if (1 == config->line[i].enable)
            {
                start.x = config->line[i].startX;
                start.y = config->line[i].startY;
                end.x   = config->line[i].endX;
                end.y   = config->line[i].endY;

                gv_md_set_line(i, start, end, config->line[i].direction);
            }
            else
            {
                gv_md_clear_line(i);
            }
        }
    }
#endif		//xqq
    return 0;
}

int sdk_vision_set_region_intrusion(GK_VISION_ALARM_RI_CFG *config)
{
    int i = 0;
    int minSize = 64;
    GV_Rect region;

    if (NULL == config)
    {
        GADI_ERROR("params error\n");
        return -1;
    }
#if 0		//xqq
    if (0 == config->enable)
    {
        for (i = 0; i < VISION_REGION_MAX; i ++)
        {
            gv_md_clear_roi(i);
        }
    }
    else
    {
        for (i = 0; i < VISION_REGION_MAX; i ++)
        {
            if (1 == config->rect[i].enable)
            {
                region.x = config->rect[i].x;
                region.y = config->rect[i].y;
                region.width  = config->rect[i].width;
                region.height = config->rect[i].height;
                minSize = MIN_SIZE_MAX-MIN_SIZE_STEP*(config->rect[i].sensitivity-1);
                gv_md_set_roi(i, region, minSize);
            }
            else
            {
                gv_md_clear_roi(i);
            }
        }
    }
#endif		//xqq
    return 0;
}

int sdk_vision_set_pause_state(int state)
{
    pause_enable = state;
    return 0;
}

int sdk_vision_get_pause_state(void)
{
    return pause_enable;
}

int sdk_vision_set_sensitive(int sensitive)
{
    return 0;
}

int sdk_vision_set_roi(int idx, GK_VISION_RECT rect, int minSize)
{
    GV_Rect gvRect;

    gvRect.x = rect.x;
    gvRect.y = rect.y;
    gvRect.width = rect.width;
    gvRect.height = rect.height;
    //return gv_md_set_roi(idx, gvRect, minSize);	xqq
    return 0;	//xqq
}
