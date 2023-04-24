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


//#include "gkcommon.h"
//#include "gk_bd.h"
#include "netcam_api.h"


#include "sdk_cfg.h"
#include "sdk_pda.h"
#include "sdk_vision.h"



#include "cfg_common.h"
//#include "httpClient.h"
//#include "sdk_vision.h"
#include "gkvision.h"
#include "eventalarm.h"


//#define USE_OML

#ifdef USE_OML
#include "SampleMbd.h"
#endif

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

#define GKVISION_DEBUG
#ifdef GKVISION_DEBUG
#define gkvision_printk(S,...)      printf("[%s:%s-%d]"S"\033[0;0m\n",GREEN,__func__,__LINE__, ##__VA_ARGS__)
#define gkvision_error(S,...)       printf("[%s:%s-%d]"S"\033[0;0m\n",RED,__func__,__LINE__, ##__VA_ARGS__)
#define gkvision_warning(S,...)     printf("[%s:%s-%d]"S"\033[0;0m\n",YELLOW,__func__,__LINE__, ##__VA_ARGS__)
#else
#define gkvision_printk(S,...)
#define gkvision_error(S,...)       printf("[%s:%s-%d]"S"\033[0;0m\n",RED,__func__,__LINE__, ##__VA_ARGS__)
#define gkvision_warning(S,...)
#endif


#define VENC_STREAM3_WIDTH   640//640//320
#define VENC_STREAM3_HEIGHT  360//360//180

#define TRUE  1
#define FALSE 0

#define VISION_PTZ_LR_STEP  230
#define VISION_PTZ_UD_STEP  80
#define VISION_PTZ_SPEED    16

#define VISION_REGION_MAX   4
#define RETURN_POINT_ID     222

#define ACT_TIME_SEGNUM     2
#define MIN_SIZE_MAX        256
#define MIN_SIZE_MIN        64
#define MIN_SIZE_SCALE      100
#define MIN_SIZE_STEP       ((MIN_SIZE_MAX-MIN_SIZE_MIN)/(MIN_SIZE_SCALE-1))

#define VISION_DETECT_JITTER_TIME_MAX  7//5

static int vision_sensitivity_map[] = {
	[SYS_MD_SENSITIVITY_LOWEST]  = 1,  //->64
	[SYS_MD_SENSITIVITY_LOW]     = 25, //->72
	[SYS_MD_SENSITIVITY_MEDIUM]  = 50, //->80
	[SYS_MD_SENSITIVITY_HIGH]    = 75, //->88
	[SYS_MD_SENSITIVITY_HIGHEST] = 93, //->95
};

//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************


typedef struct
{
    float offset_x;
    float offset_y;
}gk_servo_offset;

typedef enum ptz_msg_e {
    VC_PTZ_CTRL_STOP = 0,
    VC_PTZ_CTRL_START,
    VC_PTZ_CTRL_UP,
    VC_PTZ_CTRL_DOWN,
    VC_PTZ_CTRL_LEFT,
    VC_PTZ_CTRL_RIGHT,
}gk_servo_ptz_ctrl_t;

typedef struct msgbuf
{
	long int msgType;
	int  speed;
	int  ptzCtrl;
}gk_servo_msgbuf;


//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************

static float offset_dead_block = 0.1;//设置偏移量的死区


static int intrusionTime[VISION_REGION_MAX];
static int time_count[VISION_REGION_MAX] = {0,0,0,0};
static GK_VISION_HandleT gkHandle;
static GK_VISION_ALARM_RECT alarm_rect;
static int curScore = 0;
static int gkVisionInited = 0;
static int isGetRec = 1;
static int isUseGmd = 1;

#ifdef USE_OML
static pthread_mutex_t gkvisionMutex;
static int runing_enable;
static pthread_t gkvisionCnnThreadId;
//OKAO handle
static HMBD             OKAOHandel = NULL;
#endif

static int gmdWidth = 0;
static int gmdHeight = 0;
static int isCheat = 0;

static int bd_enable = 0;
static int people_detect_jitter_time = VISION_DETECT_JITTER_TIME_MAX;
static int bd_state = 0;

//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************

static int servo_calculate_offset(gk_servo_offset *offset ,GK_VISION_RECT *rectP);
static int servo_left_right_control(    float        offset_x);
static int servo_up_down_control(    float       offset_y);
static void servo_return(void);

static int  vision_ptz(GK_VISION_RECT *rectP);
static int vision_start(void);
static int human_is_in_region_report(int index, GK_VISION_RECT *rect, GK_VISION_REGION_TypeEnumT type);
static int human_is_in_schedule_timepoint_report(GK_SCHEDTIME *time_point, int* index);
static void intrusion_time_check(void);
static void vision_result_handle(GK_VISION_OBJ_ARRAY* obj_array);
static int vision_image_upload (char *data, int size);
extern int vision_mojing_area_check(GK_VISION_RECT bodyRec);

extern void md_light_alarm_smart(void);
//*****************************************************************************
//*****************************************************************************
//** APP Functions
//*****************************************************************************
//*****************************************************************************
pthread_t videoThreadId;

#ifdef USE_OML
static int CNNMutex_init(void)
{
	return pthread_mutex_init(&gkvisionMutex, NULL);
}

static int CNNMutex_lock(void)
{
	return pthread_mutex_lock(&gkvisionMutex);
}

static int CNNMutex_unlock(void)
{
	return pthread_mutex_unlock(&gkvisionMutex);
}

static int CNNMutex_destrory(void)
{
	return pthread_mutex_destroy(&gkvisionMutex);
}

static int gk_encoder_pda_cap_yuv(SDK_U8* yuvaddr, SDK_U32 yuvsize)
{
     SDK_PDA_YUV_ATTR_T info;
     SDK_S32 row,col;
     GADI_PDA_CAP_Buffer_TypeT buffer;
     SDK_S32 uv_width, uv_height,uv_pitch;
    
    // buffer = GADI_PDA_CAP_SUB_A_BUFFER; //second channel
     buffer = GADI_PDA_CAP_SUB_A_BUFFER; //third channel
    
     if(sdk_pda_get_yuv(0, buffer, &info) != SDK_OK)
     {
         LOG_ERR("gadi_pda_capture_buffer: failed\n");
         return -1;
     }
     if (yuvsize < info.width*info.height)
     {
         LOG_ERR("gadi_pda_capture_buffer buffer size %d*%d\n", info.width, info.height);
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
         LOG_ERR("stride size smaller than width!\n");
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
    
     SDK_U8 * output_u = yuvaddr;
     SDK_U8 * output_v = yuvaddr+info.width * info.height / 4;
    
     SDK_U8 * output_uv = info.uvAddr;
    
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


static void* gk_sample_yuv_bd(void *arg)
{
    SDK_U8* ybuffer0            = NULL;
    SDK_U32 buffersize          = 0;
	SDK_U32 osdDisableShowCount = 0;
	SDK_U32 osdDisableShow      = 0;
    //GKVISION_BD_Result bdResult;
    float   x      = 0;
    float   y      = 0;
    float   w      = 0;
    float   h      = 0;
	char text[128] = {0};
	int threshold  = 70;
	int ret        = 0;
	INT32 y_Image_w,y_Image_h;

	int retVal = 0;
	//OKAD param
	VOID * pBMemoryAddrV4,*pWMemoryAddr;
    UINT32 unBMemorySizeV4,unWMemorySize;
	RAWIMAGE *p_image_y;
	int i;
	struct timeval start_tv;
    struct timeval end_tv;
	unsigned long time_val;
	INT32 nRet = OKAO_ERR_VARIOUS;
    INT32 nI;
	int desX,desY,width,height;
    int maxRect = 0;
    int maxIndex = 0;
    GK_VISION_RECT bodyRec;
	static int detect_frame = 0;
    MBD_DETECTABILITY mbdSensitive = MBD_DETECTABILITY_HIGH;

	INT32 hum_cnt = 0;

	MBD_RESULT detect_result[20];
	
    sleep(1);
	pthread_detach(pthread_self());
	sdk_sys_thread_set_name("gkvision_cnn");
	gkvision_printk("gkvision_cnn start OK!\n");

	//y data is 1.5 times the stream resolution
	buffersize = VENC_STREAM3_WIDTH * VENC_STREAM3_HEIGHT+
					VENC_STREAM3_WIDTH * VENC_STREAM3_HEIGHT/2;

    ybuffer0 = malloc(buffersize);
    if (NULL == ybuffer0)
    {
        gkvision_error("malloc ybuffer0 failed!\n");
        return NULL;
    }

	y_Image_w = VENC_STREAM3_WIDTH;
    y_Image_h = VENC_STREAM3_HEIGHT;
	p_image_y = ybuffer0;

	//Calling the human shape detection function
	//30ms call one

	//init OKAO lib
	unBMemorySizeV4 = GetRequiredBMemSizeMbd();
    unWMemorySize   = GetRequiredWMemSizeBd();
    pBMemoryAddrV4 = malloc(unBMemorySizeV4);
    pWMemoryAddr = malloc(unWMemorySize);
    retVal=InitComFuncT(pBMemoryAddrV4,unBMemorySizeV4,pWMemoryAddr,unWMemorySize);
    if(retVal!=0)
    {
        printf("InitComFuncT error ret:%d\n", retVal);
		return -1;
    }
    retVal=InitializeMbd(&OKAOHandel);
    if(retVal!=0)
    {
        printf("InitializeMbd error ret:%d\n", retVal);
		return -2;
    }
    retVal=SetParametersMbd(OKAOHandel);
    if(retVal!=0)
    {
        printf("SetParametersMbd error ret:%d\n", retVal);
		return -3;
    }

    if (runMdCfg.mojingMdCfg.areaSensitive < 40)
    {
        mbdSensitive = MBD_DETECTABILITY_LOW;
    }
    else if (runMdCfg.mojingMdCfg.areaSensitive >= 40 && runMdCfg.mojingMdCfg.areaSensitive < 60)
    {
        mbdSensitive = MBD_DETECTABILITY_MIDDLE;
    }
    else
    {
        mbdSensitive = MBD_DETECTABILITY_HIGH;
    }
    
    OKAO_MBD_SetDetectability(OKAOHandel, mbdSensitive);
    printf("-->oml start ....%d\n", mbdSensitive);
    while(runing_enable)
    {
    	//Only when the humanoid switch is turned o

		CNNMutex_lock();
        /* capture the second channel for face rec */
        ret = gk_encoder_pda_cap_yuv(ybuffer0, buffersize);
		if (ret != 0)
		{
			CNNMutex_unlock();
			printf("gk_encoder_pda_cap_yuv failed!\n");
			usleep(300000);
			continue;
		}
        gettimeofday(&start_tv,NULL);
    	nRet = OKAO_MBD_Execute(OKAOHandel, p_image_y, y_Image_w, y_Image_h, &hum_cnt);
    	if((nRet != OKAO_NORMAL) && (nRet != MBD_WARNING_FRAMERATE)){
			CNNMutex_unlock();
            printf("OKAO_MBD_Execute err.\n");
			continue;
    	}
        gettimeofday(&end_tv,NULL);
        time_val = (end_tv.tv_sec*1000 + end_tv.tv_usec/1000) - (start_tv.tv_sec*1000 + start_tv.tv_usec/1000);
        printf("intval time: %d\n",time_val);
        for(nI = 0; nI < hum_cnt; nI++){
		    nRet = OKAO_MBD_GetResult(OKAOHandel, nI, &detect_result[nI]);
    		if((nRet != OKAO_NORMAL) && (nRet != MBD_WARNING_NODETECTION)){
				CNNMutex_unlock();
                printf("OKAO_MBD_GetResult err.\n");
				continue;
    		}
	    }
        //OKAO_MBD_GetDetectability(OKAOHandel, &mbdSensitive);
        //printf("sensitive:%d\n", mbdSensitive);
		CNNMutex_unlock();
		if(hum_cnt>0)
		{
			printf("detect hum cnt ==> [%d]\n",hum_cnt);
            maxRect = 0;
            maxIndex = 0;
			for(i = 0 ; i  < hum_cnt ; i ++ )
			{
				printf("statistic:==>%d 	 humID==>[%d]	 position:(%d,%d)<==>(%d %d)\n",detect_frame,detect_result[i].nID
					,detect_result[i].rcBody.left,detect_result[i].rcBody.top,
					detect_result[i].rcBody.right,detect_result[i].rcBody.bottom);
				desX = detect_result[i].rcBody.left;
				desY = detect_result[i].rcBody.top;
				width = detect_result[i].rcBody.right-detect_result[i].rcBody.left;
				height = detect_result[i].rcBody.bottom-detect_result[i].rcBody.top;
				printf("param:%d  %d  %d %d \n",desX,desY,width,height);
                if (width * height > maxRect)
                {
                    maxRect = width * height;
                    maxIndex = i;
                }

			}

#ifdef MODULE_SUPPORT_MOJING

            if (runImageCfg.lightMode == ISP_BULB_MODE_MIX)
            {
                md_light_alarm_smart();
            }
    
            if (runMdCfg.mojingMdCfg.areaEventStatus == 2 ||
                (runMdCfg.mojingMdCfg.areaDefenceStatus == 2 && mojing_time_in_shedule(&(runMdCfg.mojingMdCfg.areaDefenceWeek[0]), 256) == 0))
            {
                sleep(1);
                continue;
            }
            else    
            {
                
                event_alarm_touch(0, GK_ALARM_TYPE_AREA_ALARM,1,NULL);
                bodyRec.x = detect_result[maxIndex].rcBody.left;
                bodyRec.y = detect_result[maxIndex].rcBody.top;
                bodyRec.width = detect_result[maxIndex].rcBody.right-detect_result[maxIndex].rcBody.left;
                bodyRec.height = detect_result[maxIndex].rcBody.bottom-detect_result[maxIndex].rcBody.top;
                ret = vision_mojing_area_check(bodyRec);
                if (ret)
                {
                    md_light_alarm_mojing(1);
    
                    if (runAlarmCfg.alarmInfo[1].audioAlarmStatus == 1)
                    {
                        netcam_md_alarm_play_thread_start();
                        netcam_md_alarm_play(runAlarmCfg.alarmInfo[1].audioFilesCur, runAlarmCfg.alarmInfo[1].audioPlayCnt);
                    }
                }
            }
#endif     
            sleep(1);
		}
		else
		{
			//netcam_osd_update_id_rect(x,y,w,h,0,text);
			usleep(800000);
		}
		//gadi_sys_thread_sleep(1000);
		
	}

	if (ybuffer0){
   		free(ybuffer0);
		ybuffer0 = NULL;
	}

    TerminateMbd(OKAOHandel);
    TerminateCo();

    runing_enable = FALSE;

    return NULL;
}
#endif


int gk_vision_init(GK_VISION_HandleT *params)
{
#ifdef USE_OML
    pthread_attr_t  attr;
    int retVal = SDK_OK;

    CNNMutex_init();

    runing_enable = TRUE;

    /*creat get bit stream thread.*/
    if (pthread_attr_init(&attr) < 0)
    {
        gkvision_error("pthread_attr_init failed!\n");
        return -1;
    }

    retVal = pthread_create(&gkvisionCnnThreadId, NULL, gk_sample_yuv_bd, 0);
    if(retVal != 0){
        gkvision_error("create gkvisionCnnThreadId failed!\n");
        return -1;
    }

    return 0;
#else
    if (NULL == params)
    {
        LOG_ERR("gk_vision_init parameters error!\n");
        return -1;
    }

    if (gkVisionInited)
    {
        LOG_ERR("gk_vision_init already inited!\n");
        return -1;
    }

    #if 0
    int ret = netcam_net_get_detect(runNetworkCfg.lan.netName);
    if (ret != 0)
    {
        LOG_ERR("gk_vision_init error because use wifi!\n");
        return -1;
    }
    #endif
    memset(&alarm_rect, 0, sizeof(GK_VISION_ALARM_RECT));
    memcpy(&gkHandle, params, sizeof(GK_VISION_HandleT));

    int retVal = SDK_OK;
#if 0
    if (0 == runMdCfg.handle.humanDetection.enable)
    {
        return 0;
    }
#endif
    gmdWidth = runVideoCfg.vencStream[0].h264Conf.width / 16;
    gmdHeight = runVideoCfg.vencStream[0].h264Conf.height / 16;
    if (runVideoCfg.vencStream[0].h264Conf.height % 16 != 0)
    {
        gmdHeight += 1;
    }


    if (access("/opt/custom/cfg/cc", F_OK) == 0)
    {
        isCheat = 1;
    }

#ifdef MODULE_SUPPORT_MOJING
    if (runVideoCfg.vencStream[0].h264Conf.width > 1080)
    {
        //大于1080p不启用
        isUseGmd = 0;
    }
    else
    {
        isUseGmd = 1;
    }
    isUseGmd = 1;
#ifdef MODULE_SUPPORT_LARGE_SECOND_STREAM
    sdk_vision_set_yuv_channel(2);
#else
    sdk_vision_set_yuv_channel(1);
#endif

    retVal = sdk_vision_init(100 - runMdCfg.mojingMdCfg.areaSensitive, 1,vision_result_handle);		//xqq add 1
#else
    retVal = sdk_vision_init(runMdCfg.handle.humanDetection.rect[0].sensitivity, 1,vision_result_handle);	//xqq add 1 
#endif

    if (SDK_OK == retVal)
    {
        vision_start();
        sdk_vision_open();
    }
    else
    {
        LOG_ERR("gk_vision_init error\n");
		return -1;
    }

    gkVisionInited = 1;

    return 0;
#endif    
}

int gk_vision_exit(void)
{
    int retVal = SDK_OK;

    if (!gkVisionInited)
    {
        LOG_ERR("gk_vision_exit error, not inited.\n");
        return -1;
    }
    
    LOG_INFO("gk_vision_exit\n");

    netcam_timer_del_task(intrusion_time_check);
    netcam_timer_del_task(servo_return);

    #ifndef USE_OML
    retVal = sdk_vision_exit();
    #endif

    gkVisionInited = 0;
    return retVal;
}

/*!
*******************************************************************************
** \brief 计算人脸在画面中的偏移量
**
** 把人形检测的坐标转换成相对于流中心点的绝对位置；
** 并且转换成[-1,1]的范围内。
**
** \param[in]  rectP   (输入检测到人形的真实坐标和人形的宽和高)
** \param[out] offset   (输出转换后的相对坐标[-1,1])
**
**
** \return
** - # -1 (error)
** - # 0 (success)
**
** \sa
** - see_also_symbol
*******************************************************************************
*/
static int servo_calculate_offset(gk_servo_offset *offset ,
												GK_VISION_RECT *rectP)
{
	if ((NULL == offset) || (NULL == rectP))
	{
		PRINT_ERR("params error\n");
		return -1;
	}

	//人脸的中心位置[face_x,face_y]
	int face_x = (rectP->x + rectP->width/2);
	int face_y = (rectP->y + rectP->height/2);

	//人脸在画面中心X轴上的偏移量
	offset->offset_x = (float)(face_x*1.0/VENC_STREAM3_WIDTH - 0.5)*2.0;
	//人脸在画面中心y轴上的偏移量
	offset->offset_y = (float)(face_y*1.0/VENC_STREAM3_HEIGHT - 0.5)*2.0;

	PRINT_INFO("offset_x = %f,offset_y= %f\n",offset->offset_x,
														offset->offset_y);

	return 0;

}


/*!
*******************************************************************************
** \brief 控制左右电机转动多少步长，使人处于画面的中心位置
**
** 整个画面从最左边转动最右边，ptz的步长setup=240*4，
** 如果人在最左边，ptz只需要向左转动setup/2 = 240*4/2的步长，
** 人就处于整个画面的中心位置。因此把setup/2分解成120*4，
** 120作为leftRightKp系数和人形检测的x轴上的位置（x的位置已被去一化）相乘
** x轴的位置范围变成[0,120]
**
** \param[in]  offset_x   (输入被去一化的x坐标)
** \param[out] 无
**
**
** \return
** - [0,120] (返回的是0~120的范围值)
**
** \sa
** - see_also_symbol
*******************************************************************************
*/
static int servo_left_right_control(    float        offset_x)
{
	int ret                     = 0;
	float delta_degree          = 0;
	float next_leftRight_degree = 0;

	//设置最小阈值
	if (fabsf(offset_x) < offset_dead_block)
		offset_x = 0;

	//计算新的左右电机角度
	next_leftRight_degree = fabsf(offset_x)*(gkHandle.ptzCfg.leftRightKp);

	//添加边界检测，防止人轻微运动，导致云台不停转动
	if (next_leftRight_degree < 10)
		next_leftRight_degree = 0;
	else if(next_leftRight_degree > gkHandle.ptzCfg.leftRightKp)
		next_leftRight_degree = gkHandle.ptzCfg.leftRightKp;

	return (int)next_leftRight_degree;
}

/*!
*******************************************************************************
** \brief: 控制上下电机转动多少步长，使人处于画面的中心位置
**
** 整个画面从最上边转动最下边，ptz的步长setup=80*4，
** 如果人在最左边，ptz只需要向左转动setup/2 = 80*4/2的步长，
** 人就处于整个画面的中心位置。因此把setup/2分解成40*4，
** 40作为upDownKp系数和人形检测的y轴上的位置（y的位置已被去一化）相乘
** y轴的位置范围变成[0,40]
**
** \param[in]  offset_y   (输入被去一化的y坐标)
** \param[out] 无
**
**
** \return
** - [0,40] (返回的是0~40的范围值)
**
** \sa
** - see_also_symbol
*******************************************************************************
*/

static int servo_up_down_control(    float       offset_y)
{
	float delta_degree        = 0;
	float next_upDown_degree  = 0;

	//设置最小阈值
	if (fabsf(offset_y) < offset_dead_block)
		offset_y = 0;

	//计算新的左右电机角度
	next_upDown_degree = fabsf(offset_y)*(gkHandle.ptzCfg.upDownKp);

	//添加边界检测，防止人轻微运动，导致云台不停转动
	if (next_upDown_degree < 10)
		next_upDown_degree = 0;
	else if(next_upDown_degree > gkHandle.ptzCfg.upDownKp)
		next_upDown_degree = gkHandle.ptzCfg.upDownKp;

	return (int)next_upDown_degree;
}


/*!
*******************************************************************************
** \brief In the upper left corner
**
** Control the left and right and up and down of the gimbal,
** follow the movement of people and rotate
**
** \param[in]  rectP  (Enter the coordinates and height of the detected human figure)
**
**
** \return
** - # -1 (error)
** - # 0  (success)
**
** \sa
** - see_also_symbol
*******************************************************************************
*/
static int vision_ptz(GK_VISION_RECT *rectP)
{
	int leftRightDegree = 0;
	int upDownDegree    = 0;
	int i               = 0;
	int ret             = 0;

	if (NULL == rectP)
	{
		PRINT_INFO("params error\n");
		return -1;
	}
	gk_servo_offset offset;
	int stepTemp = 0;
	memset(&offset,0,sizeof(offset));

	//Denormalize human coordinates
	ret = servo_calculate_offset(&offset,rectP);
	if (ret != 0)
	{
		PRINT_ERR("servo_calculate_offset failed!,ret = %d\n",ret);
		return -1;
	}

	leftRightDegree = servo_left_right_control(offset.offset_x);
	upDownDegree    = servo_up_down_control(offset.offset_y);

	PRINT_INFO("leftRightDegree = %d,upDownDegree= %d\n",leftRightDegree,
																upDownDegree);


	stepTemp =leftRightDegree*2;
	if (offset.offset_x < 0)//right
    {
        if(runImageCfg.mirrorEnabled)
            sdk_ptz_left(stepTemp, gkHandle.ptzCfg.speed);
        else
            sdk_ptz_right(stepTemp, gkHandle.ptzCfg.speed);
    }   
	else //left
    {
        if(runImageCfg.mirrorEnabled)
            sdk_ptz_right(stepTemp, gkHandle.ptzCfg.speed);
        else
            sdk_ptz_left(stepTemp, gkHandle.ptzCfg.speed);
    }   

	stepTemp =upDownDegree*2;
	if (offset.offset_y < 0)
    {
        if(!runImageCfg.flipEnabled)
            sdk_ptz_up(stepTemp, gkHandle.ptzCfg.speed);
        else
            sdk_ptz_down(stepTemp, gkHandle.ptzCfg.speed);
    }   
	else
    {
        if(!runImageCfg.flipEnabled)
            sdk_ptz_down(stepTemp, gkHandle.ptzCfg.speed);
        else
            sdk_ptz_up(stepTemp, gkHandle.ptzCfg.speed);
    }   

	return 0;
}

int gk_vision_set_human_detection_state(int enable)
{
    int ret = 0;
    if (gkHandle.debugEnable)
    {
        gkvision_printk("set human_detection pause state %d\n", !enable);
    }
    #ifndef USE_OML
    sdk_vision_set_pause_state(!enable);
    #endif 
    return ret;
}

int gk_vision_set_motion_tracing(GK_ALARM_MT_CFG *config)
{
    int i = 0;
    int ret = 0;

    if (NULL == config)
    {
        gkvision_printk("params error\n");
		return -1;
    }

    if (0 == config->enable)
    {
        if (runMdCfg.handle.motionTracing.retPoint)
        {
            netcam_timer_del_task(servo_return);
            netcam_ptz_clr_preset(RETURN_POINT_ID);
        }
    }
    else
    {
        if (runMdCfg.handle.motionTracing.retPoint)
        {
            netcam_ptz_set_preset(RETURN_POINT_ID, NULL);
            netcam_timer_add_task(servo_return, NETCAM_TIMER_ONE_SEC*(config->retTime), SDK_FALSE, SDK_FALSE);
        }
    }

    return 0;
}

static int human_is_in_region_report(int index, GK_VISION_RECT *rect, GK_VISION_REGION_TypeEnumT type)
{
    int i = 0;
    int ret = 0;
    float area_set = 0;
    float area_act = 0;

    switch(type)
    {
        case GK_VISION_REGION_HUMAN_DETECTION:
            for (i = 0; i < VISION_REGION_MAX; i++)
            {
                if (0 == runMdCfg.handle.humanDetection.rect[i].enable)
                {
                    if(i==(VISION_REGION_MAX-1))
                    {
                        return 1;
                    }
                    continue;
                }
            }

            if (runMdCfg.handle.humanDetection.rect[index].enable)
            {
                if ((rect->x >= runMdCfg.handle.humanDetection.rect[index].x) &&
                    (rect->y >= runMdCfg.handle.humanDetection.rect[index].y))
                {
                    if (((rect->x+rect->width) <= (runMdCfg.handle.humanDetection.rect[index].x+runMdCfg.handle.humanDetection.rect[index].width)) &&
                        ((rect->x+rect->height) <= (runMdCfg.handle.humanDetection.rect[index].x+runMdCfg.handle.humanDetection.rect[index].height)))
                    {
                        ret = 1;
                    }
                }
            }
            break;

        case GK_VISION_REGION_MOTION_TRACING:
            if (0 == runMdCfg.handle.motionTracing.enable)
            {
                gkvision_warning("motion tracing is disable\n");
                return 0;
            }

            for (i = 0; i < VISION_REGION_MAX; i++)
            {
                if (0 == runMdCfg.handle.motionTracing.rect[i].enable)
                {
                    if(i==(VISION_REGION_MAX-1))
                    {
                        return 1;
                    }
                    continue;
                }
            }
            if (runMdCfg.handle.motionTracing.rect[index].enable)
            {
				if ((rect->x >= runMdCfg.handle.motionTracing.rect[i].x) &&
                    (rect->y >= runMdCfg.handle.motionTracing.rect[i].y))
                {
                    if (((rect->x+rect->width) <= (runMdCfg.handle.motionTracing.rect[index].x+runMdCfg.handle.motionTracing.rect[index].width)) &&
                        ((rect->x+rect->height) <= (runMdCfg.handle.motionTracing.rect[index].x+runMdCfg.handle.motionTracing.rect[index].height)))
                    {
                        ret = 1;
                    }
                }
            }
            break;

        case GK_VISION_REGION_REGION_INTRUSION:
            if (0 == runMdCfg.handle.regionIntrusion.enable)
            {
                gkvision_warning("region intrusion is disable\n");
                return 0;
            }

            if (runMdCfg.handle.regionIntrusion.rect[index].enable)
            {
                // TODO: open it after driver support.
                //area_set = (runMdCfg.handle.motionTracing.rect[index].x) * (runMdCfg.handle.motionTracing.rect[index].y);
                //area_act = (rect->x) * (rect->y);
                //printf("area size is %f\n", (area_act/area_set));
                //if ((area_act/area_set) >= runMdCfg.handle.motionTracing.rect[index].regionRatio)
                {
                    ret = 1;
                }
            }
            break;

        default:
            break;
    }

    return ret;
}

static int human_is_in_schedule_timepoint_report(GK_SCHEDTIME *time_point, int* index)
{
    long ts = time(NULL);
    struct tm tt = {0};
    struct tm *pTm = localtime_r(&ts, &tt);

    char str[30];
    memset(str, 0, sizeof(str));
    sprintf(str, "%02d%02d", pTm->tm_hour, pTm->tm_min);
    unsigned long long now_64 = atoll(str);

    int i;
    int ret = 0;
    unsigned long long start_64 = 0;
    unsigned long long stop_64 = 0;
    GK_SCHEDTIME *tPoint = NULL;
    //PRINT_INFO("pTm->tm_wday:%d\n", pTm->tm_wday);
    for (i = 0; i < ACTION_TIME_SEGNUM; i ++) {
        //tPoint = (time_point + pTm->tm_wday * 4 + i);
        tPoint = (time_point + i);
        timepoint_to_u64(tPoint, &start_64, &stop_64);
        //PRINT_INFO("day:%d, index:%d, time:%llu - %llu, now_time:%llu",
          //  pTm->tm_wday, i, start_64, stop_64, now_64);
        if (start_64 <= stop_64) {
            if ((now_64 >= start_64) && (now_64 <= stop_64)) {
                *index = i;
                ret = 1;
                break;
            }
        } else {
            if ((now_64 >= start_64) || (now_64 <= stop_64))  {
                *index = i;
                ret = 1;
                break;
            }
        }
    }

    return ret;
}

static void intrusion_time_check(void)
{
    float   x_osd;
    float   y_osd;
    float   w_osd;
    float   h_osd;
    int     idx = 0;
    char text[128] = "aa";

    if (0 == runMdCfg.handle.regionIntrusion.enable)
    {
        return;
    }

    for (idx=0; idx < VISION_REGION_MAX; idx++)
    {
        if (runMdCfg.handle.regionIntrusion.rect[idx].enable)
        {
            if (runMdCfg.handle.regionIntrusion.rect[idx].regionTime > 0)
            {
                if(intrusionTime[idx] == 0)
                {
                    time_count[idx] = 0;
                }
                else
                {
                    time_count[idx]++;
                    if (time_count[idx] > runMdCfg.handle.regionIntrusion.rect[idx].regionTime)
                    {
                        if (gkHandle.debugEnable)
                        {
                            gkvision_printk("reach intrusion[%d] time %d seconds\n", idx, time_count[idx]);
                        }
                        x_osd = (float)runMdCfg.handle.regionIntrusion.rect[idx].x/VENC_STREAM3_WIDTH;
                        y_osd = (float)runMdCfg.handle.regionIntrusion.rect[idx].y/VENC_STREAM3_HEIGHT;
                        w_osd = (float)runMdCfg.handle.regionIntrusion.rect[idx].width/VENC_STREAM3_WIDTH;
                        h_osd = (float)runMdCfg.handle.regionIntrusion.rect[idx].height/VENC_STREAM3_HEIGHT;

                        sprintf(text, "ROI_Alarm_%d", idx);
                    	//netcam_osd_update_id_rect(x_osd,y_osd,w_osd,h_osd,0,text);
                    	//usleep(100*1000);
                    	if (gkHandle.osdEnable)
                        {
                            netcam_osd_update_id_rect(x_osd,y_osd,w_osd,h_osd,1,text);
                        }
                    }
                }
            }
        }
    }
}

static void servo_return(void)
{
    GK_NET_CRUISE_GROUP cruise_info;

    if (gkHandle.debugEnable)
    {
        gkvision_printk("servo_return\n");
    }
    memset(&cruise_info, 0, sizeof(GK_NET_CRUISE_GROUP));
    cruise_info.byCruiseIndex = 0;
    cruise_info.byPointNum = 1;
    cruise_info.struCruisePoint[0].byPresetNo = RETURN_POINT_ID;
    cruise_info.struCruisePoint[0].byRemainTime = 0;
    cruise_info.struCruisePoint[0].bySpeed = 3;

    netcam_ptz_preset_cruise(&cruise_info);
}

static int vision_mojing_time_check(void)
{
    //灯光智能模式下检测到人形时亮灯
    #ifndef USE_OML
    if ((runImageCfg.lightMode != ISP_BULB_MODE_MIX && (runMdCfg.mojingMdCfg.areaEventStatus == 2))
        || netcam_get_update_status() != 0)
    {
        sdk_vision_set_pause_state(1);
    }
    else
    {
        sdk_vision_set_pause_state(0);
    }
    #endif
    return 0;
}

static int vision_start(void)
{
    #ifndef USE_OML
    sdk_vision_set_human_detection(&(runMdCfg.handle.humanDetection));
    //gk_vision_set_motion_tracing(&(runMdCfg.handle.motionTracing));
    sdk_vision_set_transboundary_detection(&(runMdCfg.handle.transboundaryDetection));
    sdk_vision_set_region_intrusion(&(runMdCfg.handle.regionIntrusion));

    netcam_timer_add_task(intrusion_time_check, NETCAM_TIMER_ONE_SEC, SDK_FALSE, SDK_FALSE);
    #ifdef MODULE_SUPPORT_MOJING
    netcam_timer_add_task(vision_mojing_time_check, NETCAM_TIMER_ONE_SEC, SDK_FALSE, SDK_FALSE);
    #endif
    #endif
    return 0;
}

static int isOverlap(GK_VISION_RECT rc1, GK_VISION_RECT rc2)
{
    if (rc1.x + rc1.width  > rc2.x &&
        rc2.x + rc2.width  > rc1.x &&
        rc1.y + rc1.height > rc2.y &&
        rc2.y + rc2.height > rc1.y
       )
        return 1;
    else
        return 0;
}
#ifdef MODULE_SUPPORT_MOJING
int vision_mojing_area_check(GK_VISION_RECT bodyRec)
{
    char buffer[128];  
    int pointCnt = 0;
    static char points[10][10];
    static GK_VISION_RECT checkRec;
    int x, y;
    int minX = 9999, minY = 9999, maxX = 0, maxY = 0;
    int i;
    int resWidth, resHeight;
    GK_VISION_RECT rect;
    SDK_PDA_YUV_ATTR_T yuvAttr;
    
    if (isGetRec == 1)
    {
        strcpy((char*)buffer, runMdCfg.mojingMdCfg.areaGroupPoints);
        pointCnt = mojing_get_split_str((char*)&(points[0]), 10, 10, (char*)buffer, ":");
        if (pointCnt > 4)
        {
            printf("vision_mojing_area_check point num:%d\n", pointCnt);
        }

        for (i = 0; i < pointCnt; i++)
        {
            sscanf(points[i], "%d,%d", &x, &y);
            if (x > maxX)
            {
                maxX = x;
            }

            if (x < minX)
            {
                minX = x;
            }
            
            if (y > maxY)
            {
                maxY = y;
            }

            if (y < minY)
            {
                minY = y;
            }
        }

        checkRec.x = minX;
        checkRec.width = maxX - minX;

#ifdef MODULE_SUPPORT_LARGE_SECOND_STREAM
        sdk_pda_get_yuv(0, 2, &yuvAttr);
#else
        sdk_pda_get_yuv(0, 1, &yuvAttr);
#endif
        if (yuvAttr.width != 0)
        {
            if (strstr(runMdCfg.mojingMdCfg.areaGroupResolution, "*") != NULL)
            {
                sscanf(runMdCfg.mojingMdCfg.areaGroupResolution, "%d*%d", &resWidth, &resHeight);
            }
            else if (strstr(runMdCfg.mojingMdCfg.areaGroupResolution, "x") != NULL)
            {
                sscanf(runMdCfg.mojingMdCfg.areaGroupResolution, "%dx%d", &resWidth, &resHeight);
            }
            else
            {
                printf("areaGroupResolution error:%s\n", runMdCfg.mojingMdCfg.areaGroupResolution);
                return 0;
            }

            //魔镜(0,0)在矩形左下角
            checkRec.y = resHeight - maxY;
            checkRec.height = maxY - minY;
            
            checkRec.x = (checkRec.x * 1.0) / resWidth * yuvAttr.width;
            checkRec.y = (checkRec.y * 1.0) / resHeight * yuvAttr.height;
            checkRec.width = (checkRec.width * 1.0) / resWidth * yuvAttr.width;
            checkRec.height = (checkRec.height * 1.0) / resHeight * yuvAttr.height;
            printf("vision_mojing_area_check %d,%d-%d,%d\n", checkRec.x, checkRec.y, checkRec.width, checkRec.height);
            isGetRec = 0;
            #if 0
            if (isUseGmd)
            {
                rect.x = abs(checkRec.x * 1.0 / (yuvAttr.width / gmdWidth) - 3);
                rect.y = abs(checkRec.y * 1.0 / (yuvAttr.height / gmdHeight) - 3);
                rect.width = checkRec.width * 1.0 / (yuvAttr.width / gmdWidth) + 6;
                rect.height = checkRec.height * 1.0 / (yuvAttr.height / gmdHeight) + 6;
                if (rect.width > gmdWidth)
                {
                    rect.width = gmdWidth;
                }

                if (rect.height > gmdHeight)
                {
                    rect.height = gmdHeight;
                }
            }
            else
            {
                rect.x = checkRec.x;
                rect.y = checkRec.y;
                rect.width = checkRec.width;
                rect.height = checkRec.height;
            }
            printf("vision_mojing_area_check roi %d,%d-%d,%d\n", rect.x, rect.y, rect.width, rect.height);
            
            sdk_vision_set_roi(0, rect, 0);
            #endif
        }
    }

    return isOverlap(bodyRec, checkRec);
    
}
#endif

#if 1
static void vision_result_handle(GK_VISION_OBJ_ARRAY* result)
{
    SDK_U32 idx;
    SDK_U32 ret = 0;
    SDK_U32 max_score = 0;
    SDK_U32 index     = 0;
    SDK_U32 time_index = 0;
    //GV_Rect rect;
    //GV_Point start, end;
    float   x_osd;
    float   y_osd;
    float   w_osd;
    float   h_osd;
    char text[128] = "aa";
    long ts = time(NULL);
    struct tm tt = {0};
    struct tm *pTm = NULL;

    if (result->class_num)
    {
        //Take out the one with the highest human figure score
        for (idx = 0; idx < result->class_num; idx++)
        {
            if (gkHandle.debugEnable)
            {
                gkvision_printk(" idx:%d      mdResult.detectCnt:%d    start(%d,%d)size(%d,%d) score:%d\n",
                    idx, result->class_num,
                    result->obj_class[idx].objs->rect.x, result->obj_class[idx].objs->rect.y,
                    result->obj_class[idx].objs->rect.width, result->obj_class[idx].objs->rect.height,
                    result->obj_class[idx].objs->quality);
            }
            if(max_score < result->obj_class[idx].objs->quality)
            {
                max_score = result->obj_class[idx].objs->quality;
                index = idx;
            }
        }


#ifdef MODULE_SUPPORT_MOJING
        if (runImageCfg.lightMode == ISP_BULB_MODE_MIX )
        {
            md_light_alarm_smart();
        }

        for (idx = 0; idx < result->class_num; idx++)
        {
            index = idx;
            if (runMdCfg.mojingMdCfg.areaSmartStatus == 1)
            {
                if (runMdCfg.mojingMdCfg.areaEventStatus == 2 ||
                    (runMdCfg.mojingMdCfg.areaDefenceStatus == 2 && mojing_time_in_shedule(&(runMdCfg.mojingMdCfg.areaDefenceWeek[0]), 256) == 0))
                {
                    sleep(1);
                    printf("time or status error.\n");
                    return;
                }
                else    
                {
                    ret = vision_mojing_area_check(result->obj_class[idx].objs->rect);
                    if (ret)
                    {
                        event_alarm_touch(0, GK_ALARM_TYPE_AREA_ALARM,1,NULL);
                        md_light_alarm_mojing(1);

                        if (runAlarmCfg.alarmInfo[1].audioAlarmStatus == 1)
                        {
                            netcam_md_alarm_play_thread_start();
                            netcam_md_alarm_play(runAlarmCfg.alarmInfo[1].audioFilesCur, runAlarmCfg.alarmInfo[1].audioPlayCnt);
                        }
                        sleep(1);
                        return;
                    }
                }
            }
        }
#endif        
    }

    
#ifdef MODULE_SUPPORT_MOJING
#if 1
    if (isCheat == 1 && access("/tmp/pp", F_OK) == 0)
    {
        if (runImageCfg.lightMode == ISP_BULB_MODE_MIX )
        {
            md_light_alarm_smart();
        }
        
        md_light_alarm_mojing(1);
        
        if (runAlarmCfg.alarmInfo[1].audioAlarmStatus == 1)
        {
            netcam_md_alarm_play_thread_start();
            netcam_md_alarm_play(runAlarmCfg.alarmInfo[1].audioFilesCur, runAlarmCfg.alarmInfo[1].audioPlayCnt);
        }
        
        new_system_call("rm -f /tmp/pp");
    }
#endif    
    //魔镜不做后面检测
    return;
#endif
}

#else
static void vision_result_handle(GK_VISION_MD_RESULT* result)
{
    SDK_U32 idx;
    SDK_U32 ret = 0;
	SDK_U32 max_score = 0;
	SDK_U32 index     = 0;
    SDK_U32 time_index = 0;
    //GV_Rect rect;
    //GV_Point start, end;
    float   x_osd;
    float   y_osd;
    float   w_osd;
    float   h_osd;
    char text[128] = "aa";
    long ts = time(NULL);
    struct tm tt = {0};
    struct tm *pTm = NULL;

#ifdef MODULE_SUPPORT_MOJING
    float ratioX = runVideoCfg.vencStream[2].h264Conf.width * 16.0 / runVideoCfg.vencStream[0].h264Conf.width;
    float ratioY = runVideoCfg.vencStream[2].h264Conf.height * 16.0 / runVideoCfg.vencStream[0].h264Conf.height;
    //printf("cnt:%d, status:%d,%d\n", result->mdResult.maxMotionCnt, runMdCfg.mojingMdCfg.areaSmartStatus, runMdCfg.mojingMdCfg.areaEventStatus);
    if (runMdCfg.mojingMdCfg.areaSmartStatus == 2 && runMdCfg.mojingMdCfg.areaEventStatus == 1)
    {
        if (result->mdResult.maxMotionCnt > 0)
        {
            if (runMdCfg.mojingMdCfg.areaEventStatus == 2 ||
                (runMdCfg.mojingMdCfg.areaDefenceStatus == 2 && mojing_time_in_shedule(&(runMdCfg.mojingMdCfg.areaDefenceWeek[0]), 256) == 0))
            {
                //sleep(1);
                return;
            }
            else    
            {
                event_alarm_touch(0, GK_ALARM_TYPE_AREA_ALARM,1,NULL);
                if (isUseGmd)
                {
                    //use gmd 
                    result->mdResult.maxMotionRect.x *= ratioX;
                    result->mdResult.maxMotionRect.y *= ratioY;
                    result->mdResult.maxMotionRect.width *= ratioX;
                    result->mdResult.maxMotionRect.height *= ratioY;
                }
                ret = vision_mojing_area_check(result->mdResult.maxMotionRect);
                printf("vision_result_handle maxMotionRect:%d,%d-%d,%d\n", result->mdResult.maxMotionRect.x, 
                        result->mdResult.maxMotionRect.y, result->mdResult.maxMotionRect.width, result->mdResult.maxMotionRect.height);
                if (ret)
                {
                    md_light_alarm_mojing(1);

                    if (runAlarmCfg.alarmInfo[1].audioAlarmStatus == 1)
                    {
                        netcam_md_alarm_play_thread_start();
                        netcam_md_alarm_play(runAlarmCfg.alarmInfo[1].audioFilesCur, runAlarmCfg.alarmInfo[1].audioPlayCnt);
                    }
                }
                sleep(1);
            }
        }
    }
#endif

    if (result->mdResult.detectCnt)
    {
    	//Take out the one with the highest human figure score
        for (idx = 0; idx < result->mdResult.detectCnt; idx++)
        {
            if (gkHandle.debugEnable)
            {
                gkvision_printk(" idx:%d      mdResult.detectCnt:%d    start(%d,%d)size(%d,%d) score:%d\n",
    				idx,result->mdResult.detectCnt,
                    result->mdResult.detectInfo[idx].rect.x, result->mdResult.detectInfo[idx].rect.y,
                    result->mdResult.detectInfo[idx].rect.width, result->mdResult.detectInfo[idx].rect.height,
                    result->mdResult.detectInfo[idx].score);
            }
			if(max_score < result->mdResult.detectInfo[idx].score)
			{
				max_score = result->mdResult.detectInfo[idx].score;
				index = idx;
			}
        }

        //test osd draw
        #if 0
        
        x_osd = (float)result->mdResult.detectInfo[index].rect.x/runVideoCfg.vencStream[1].h264Conf.width;
        y_osd = (float)result->mdResult.detectInfo[index].rect.y/runVideoCfg.vencStream[1].h264Conf.height;
        w_osd = (float)result->mdResult.detectInfo[index].rect.width/runVideoCfg.vencStream[1].h264Conf.width;
        h_osd = (float)result->mdResult.detectInfo[index].rect.height/runVideoCfg.vencStream[1].h264Conf.height;

        sprintf(text, "%d", result->mdResult.detectInfo[index].score);
		//netcam_osd_update_id_rect(x_osd,y_osd,w_osd,h_osd,0,text);
		//usleep(100*1000);
		//if (gkHandle.osdEnable)
		printf("draw osd:%d,%d-%d,%d\n", result->mdResult.detectInfo[index].rect.x, result->mdResult.detectInfo[index].rect.y,
		    result->mdResult.detectInfo[index].rect.width, result->mdResult.detectInfo[index].rect.height);
        {
            netcam_osd_update_id_rect(x_osd,y_osd,w_osd,h_osd,1,text);
        }
        return;
        #endif

#ifdef MODULE_SUPPORT_MOJING
        if (runImageCfg.lightMode == ISP_BULB_MODE_MIX )
        {
            md_light_alarm_smart();
        }

        for (idx = 0; idx < result->mdResult.detectCnt; idx++)
        {
            index = idx;
            if (runMdCfg.mojingMdCfg.areaSmartStatus == 1)
            {
                if (runMdCfg.mojingMdCfg.areaEventStatus == 2 ||
                    (runMdCfg.mojingMdCfg.areaDefenceStatus == 2 && mojing_time_in_shedule(&(runMdCfg.mojingMdCfg.areaDefenceWeek[0]), 256) == 0))
                {
                    sleep(2);
                    printf("time or status error.\n");
                    return;
                }
                else    
                {
                    ret = vision_mojing_area_check(result->mdResult.detectInfo[index].rect);
                    if (ret)
                    {
                        event_alarm_touch(0, GK_ALARM_TYPE_AREA_ALARM,1,NULL);
                        md_light_alarm_mojing(1);

                        if (runAlarmCfg.alarmInfo[1].audioAlarmStatus == 1)
                        {
                            netcam_md_alarm_play_thread_start();
                            netcam_md_alarm_play(runAlarmCfg.alarmInfo[1].audioFilesCur, runAlarmCfg.alarmInfo[1].audioPlayCnt);
                        }
                        sleep(2);
                        return;
                    }
                }
            }
        }

#endif        

		//Display a small humanoid icon in the first area of ??osd when a humanoid is detected
		//In the upper left corner
		if (human_is_in_region_report(index, &(result->mdResult.detectInfo[index].rect), GK_VISION_REGION_HUMAN_DETECTION))
		{
			x_osd = (float)result->mdResult.detectInfo[index].rect.x/VENC_STREAM3_WIDTH;
            y_osd = (float)result->mdResult.detectInfo[index].rect.y/VENC_STREAM3_HEIGHT;
            w_osd = (float)result->mdResult.detectInfo[index].rect.width/VENC_STREAM3_WIDTH;
            h_osd = (float)result->mdResult.detectInfo[index].rect.height/VENC_STREAM3_HEIGHT;

            sprintf(text, "%d", result->mdResult.detectInfo[index].score);
			//netcam_osd_update_id_rect(x_osd,y_osd,w_osd,h_osd,0,text);
			//usleep(100*1000);
			if (gkHandle.osdEnable)
            {
                netcam_osd_update_id_rect(x_osd,y_osd,w_osd,h_osd,1,text);
            }
            /*
            if (handle.imgUpload)
            {
                alarm_rect.x = result->mdResult.detectInfo[index].rect.x;
                alarm_rect.y = result->mdResult.detectInfo[index].rect.y;
                alarm_rect.width = result->mdResult.detectInfo[index].rect.width;
                alarm_rect.height = result->mdResult.detectInfo[index].rect.height;
                curScore = result->mdResult.detectInfo[index].score;
                vision_image_upload(result->buffInfo.addr, result->buffInfo.size);
            }
            */
		}

		if (runMdCfg.handle.motionTracing.enable == 1)//ptz
		{
		    if (human_is_in_region_report(index, &(result->mdResult.detectInfo[index].rect), GK_VISION_REGION_MOTION_TRACING))
            {
                ret = vision_ptz(&result->mdResult.detectInfo[index].rect);
				if (ret != 0)
				{
					gkvision_error("gk_vision_ptz failed!\n");
				}
            }
		}


	}
	else
	{
	    if (gkHandle.osdEnable)
        {
		    netcam_osd_update_id_rect(x_osd,y_osd,w_osd,h_osd,0,text);
        }
	}
    
#ifdef MODULE_SUPPORT_MOJING
#if 1
    if (isCheat == 1 && access("/tmp/pp", F_OK) == 0)
    {
        if (runImageCfg.lightMode == ISP_BULB_MODE_MIX )
        {
            md_light_alarm_smart();
        }
        
        md_light_alarm_mojing(1);
        
        if (runAlarmCfg.alarmInfo[1].audioAlarmStatus == 1)
        {
            netcam_md_alarm_play_thread_start();
            netcam_md_alarm_play(runAlarmCfg.alarmInfo[1].audioFilesCur, runAlarmCfg.alarmInfo[1].audioPlayCnt);
        }
        
        new_system_call("rm -f /tmp/pp");
    }
#endif    
    //魔镜不做后面检测
    return;
#endif

    for (idx = 0; idx < VISION_REGION_MAX; idx++)
    {
        if (result->mdResult.lineAlarm[idx])
        {
            pTm = localtime_r(&ts, &tt);
            if (gkHandle.debugEnable)
            {
                gkvision_printk("mdResult.lineAlarm------------------[%d]\n", idx);
            }
            if (human_is_in_schedule_timepoint_report(&(runMdCfg.handle.transboundaryDetection.scheduleTime[pTm->tm_wday][0]), &time_index))
            {
                if (gkHandle.debugEnable)
                {
                    gkvision_printk("mdresult.lineAlarm = %d, from (%d,%d) to (%d,%d)\n",
                        idx,
                        runMdCfg.handle.transboundaryDetection.line[idx].startX,
                        runMdCfg.handle.transboundaryDetection.line[idx].startY,
                        runMdCfg.handle.transboundaryDetection.line[idx].endX,
                        runMdCfg.handle.transboundaryDetection.line[idx].endY);
                }
				x_osd = (float)runMdCfg.handle.transboundaryDetection.line[idx].startX/VENC_STREAM3_WIDTH;
                y_osd = (float)runMdCfg.handle.transboundaryDetection.line[idx].startY/VENC_STREAM3_HEIGHT;
                h_osd = (float)20/VENC_STREAM3_WIDTH;
                w_osd = (float)40/VENC_STREAM3_HEIGHT;

                sprintf(text, "Line_Alarm_%d", idx);
    			//netcam_osd_update_id_rect(x_osd,y_osd,w_osd,h_osd,0,text);
    			//usleep(100*1000);
    			if (gkHandle.osdEnable)
                {
                    netcam_osd_update_id_rect(x_osd,y_osd,w_osd,h_osd,1,text);
                }
            }
            else
            {
                if (gkHandle.osdEnable)
                {
                    netcam_osd_update_id_rect(x_osd,y_osd,w_osd,h_osd,0,text);
                }
            }
        }

        if (result->mdResult.roiAlarm[idx])
        {
            if (gkHandle.debugEnable)
            {
                gkvision_printk("mdResult.roiAlarm------------------[%d]\n", idx);
            }

            pTm = localtime_r(&ts, &tt);
            if (human_is_in_schedule_timepoint_report(&(runMdCfg.handle.regionIntrusion.scheduleTime[pTm->tm_wday][0]), &time_index))
            {
                if (gkHandle.debugEnable)
                {
                    gkvision_printk("mdresult.roiAlarm = %d\n", idx);
                }
                if (human_is_in_region_report(index, &(runMdCfg.handle.regionIntrusion.rect[index]), GK_VISION_REGION_REGION_INTRUSION))
                {
                    if (runMdCfg.handle.regionIntrusion.rect[index].regionTime > 0)
                    {
                        intrusionTime[idx] = 1;
                    }
                    else
                    {
                        x_osd = (float)runMdCfg.handle.regionIntrusion.rect[idx].x/VENC_STREAM3_WIDTH;
                        y_osd = (float)runMdCfg.handle.regionIntrusion.rect[idx].y/VENC_STREAM3_HEIGHT;
                        w_osd = (float)runMdCfg.handle.regionIntrusion.rect[idx].width/VENC_STREAM3_WIDTH;
                        h_osd = (float)runMdCfg.handle.regionIntrusion.rect[idx].height/VENC_STREAM3_HEIGHT;

                        sprintf(text, "ROI_Alarm_%d", idx);
            			//netcam_osd_update_id_rect(x_osd,y_osd,w_osd,h_osd,0,text);
            			//usleep(100*1000);
            			if (gkHandle.osdEnable)
                        {
                            netcam_osd_update_id_rect(x_osd,y_osd,w_osd,h_osd,1,text);
                        }
                    }
                }
                else
                {
                    intrusionTime[idx] = 0;
                }
            }
            else
            {
                intrusionTime[idx] = 0;
                if (gkHandle.osdEnable)
                {
                    netcam_osd_update_id_rect(x_osd,y_osd,w_osd,h_osd,0,text);
                }
            }
        }
        else
        {
            intrusionTime[idx] = 0;
        }
    }
}
#endif

int gk_vision_fresh_rect(void)
{
    isGetRec = 1;
}

int gk_vision_set_sensitive(int sensitive)
{
#ifdef USE_OML
    MBD_DETECTABILITY mbdSensitive = MBD_DETECTABILITY_HIGH;

    if (runMdCfg.mojingMdCfg.areaSensitive < 40)
    {
        mbdSensitive = MBD_DETECTABILITY_LOW;
    }
    else if (runMdCfg.mojingMdCfg.areaSensitive >= 40 && runMdCfg.mojingMdCfg.areaSensitive < 60)
    {
        mbdSensitive = MBD_DETECTABILITY_MIDDLE;
    }
    else
    {
        mbdSensitive = MBD_DETECTABILITY_HIGH;
    }

    OKAO_MBD_SetDetectability(OKAOHandel, mbdSensitive);
#else
#endif
}

#if 0
static int vision_image_upload (char *data, int size)
{
    if (NULL == data)
    {
        gkvision_error("vision_image_upload bad parameter\n");
        return -1;
    }

    if (handle.debugEnable)
    {
        gkvision_printk("imagesize:%d\n", size);
    }

    /* 报警间隔时间内(秒)，不重复响应同一种报警 */
    static int last_md_sec = 0;
    struct timeval t1 = {0, 0};
    int interval = runMdCfg.handle.intervalTime;
    if (interval <= 0 || interval > 120)
    {
        gkvision_error("interval %d set to 10 seconds\n", interval);
        interval = 10;
    }

    gettimeofday(&t1, NULL);
    if(t1.tv_sec < last_md_sec)
    {
        last_md_sec = t1.tv_sec;
        //PRINT_INFO("time is change!\n");
    }
    if((t1.tv_sec - last_md_sec) <= interval)
    {
        //gkvision_error("t1.tv_sec-last_md_sec:%d,time is too closed, so not to notify.\n",(int)t1.tv_sec - last_md_sec);
        return;
    }
    last_md_sec = t1.tv_sec;

	char url[256] = {0};
	char *retData = NULL;

	char device_imei[32] = "gkvision_test_200w";

	strncpy(device_imei,
			(char*)&runSystemCfg.deviceInfo.serialNumber,
			sizeof(device_imei));

	HTTP_MULTI_META_DATA *fromData = goke_http_form_data_create();
	if (fromData == NULL)
	{
		gkvision_error("failture goke_http_form_data_create\n");
	}

    goke_http_form_data_add(fromData, "cid",
							device_imei, strlen(device_imei),
							MULTIPART_DATA_TXT, NULL);

    char short_time[128]       = {0};
    struct tm *ptm    = NULL;
    struct tm curTime = {0};
    struct timeval tv;
    gettimeofday( &tv, NULL );

    ptm = localtime_r(&tv.tv_sec, &curTime);

	sprintf(short_time, "%04d%02d%02d",
								ptm->tm_year+1900,
								ptm->tm_mon+1,
								ptm->tm_mday);
    goke_http_form_data_add(fromData, "short_time",
							short_time, strlen(short_time),
							MULTIPART_DATA_TXT, NULL);

    //office
	//sprintf(url,"http://logs.nas.lpwei.com:9090/upload/uploadPic.php");
    //home
	sprintf(url,"http://log.nas.lpwei.com:9080/upload/uploadPic.php");
    

	char *fileData       = data;
	int length	         = size;
	char imgFileName[64] = {0};
	sprintf(imgFileName, "%04d%02d%02d%02d%02d%02d%03d_%d_%d-%dx%d_%d.yuv",
								ptm->tm_year+1900,
								ptm->tm_mon+1,
								ptm->tm_mday,
								ptm->tm_hour,
								ptm->tm_min,
								ptm->tm_sec,
								tv.tv_usec/1000,
								alarm_rect.x,
								alarm_rect.y,
								alarm_rect.width,
								alarm_rect.height,
								curScore);

	goke_http_form_data_add(fromData, "name",
							fileData, length,
							MULTIPART_DATA_STREAM, imgFileName);
	int retLen  = 0;
	int recvLen = 0;
	retData = goke_http_form_data_request(url, fromData, &retLen, &recvLen);
	if (retData != NULL)
	{
		PRINT_INFO("http ret data:%s\n",retData);

		free(retData);
		retData = NULL;
	}

	return 0;
}//read_frame END
#endif
