/*!
*****************************************************************************
** \file        gk7101_vin.c
**
** \version
**
** \brief       GK7101 vin porting.
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2014-2015 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#include "adi_types.h"
#include "adi_sys.h"
#include "adi_venc.h"
#include "adi_pda.h"
#include "adi_vi.h"
#include "adi_pm.h"

#include "sdk.h"
#include "sdk_api.h"
#include "sdk_debug.h"
#include "sdk_osd.h"
//#include "zbar.h"
//#include "sdk_network.h"

//*****************************************************************************
//*****************************************************************************
//** Local Defines
//*****************************************************************************
//*****************************************************************************

#ifdef LOG_CHECK
#undef LOG_CHECK
#undef LOG_SYNTAX
#endif

#define LOG_SYNTAX "1;32"

#define LOG_CHECK(exp, fmt...) \
	do{\
		uint32_t ret = exp;\
		if(GADI_OK != ret){\
			const char* const bname = __FILE__;\
			printf("\033["LOG_SYNTAX"m");\
			printf("%s @ [%s: %d] err: 0x%08x ", #exp, bname, __LINE__, ret);\
			printf("\033[0m\r\n");\
		}\
	}while(0)

#define GK_VIN_MAGIC               (0xf0f0f0f0)
#define GK_VIN_CH_BACKLOG_REF      (1)
#define GK_VIN_MD_RECT_BACKLOG_REF (8)
#define GK_VIN_COVER_BACKLOG_REF   (8)  // goke support 8,hisi only support 4

//*****************************************************************************
//*****************************************************************************
//** Local structures
//*****************************************************************************
//*****************************************************************************
typedef struct SDK_VIN_MD_BITMAP_MODE {
	size_t mask_bytes;
	uint8_t *mask_bitflag;
	float threshold;
}ST_GK_VIN_MD_BITMAP_MODE, *PS_GK_VIN_MD_BITMAP_MODE;

typedef struct SDK_VIN_MD_RECT_MODE {
	bool occupation;
	float x_ratio, y_ratio, width_ratio, height_ratio;
	float threshold;
}ST_GK_VIN_MD_RECT_MODE, *PS_GK_VIN_MD_RECT_MODE;

typedef struct SDK_VIN_MD_ATTR {
	uint32_t magic; // = GK_MD_CH_ATTR_MAGIC
	size_t ref_hblock, ref_vblock;
	// bitmap mode flag, describe using bitmap mode or rect mode
	bool bitmap_mode;

	ST_GK_VIN_MD_BITMAP_MODE bitmap;
	ST_GK_VIN_MD_RECT_MODE rect[GK_VIN_MD_RECT_BACKLOG_REF];
	GK_MD_DO_TRAP do_trap;
}ST_GK_VIN_MD_ATTR, *PS_GK_VIN_MD_ATTR;

typedef struct SDK_VIN_ATTR {
	// motion detection
	//ST_GK_VIN_MD_ATTR md_attr[GK_VIN_CH_BACKLOG_REF];
	//uint32_t md_intl;
	//bool md_trigger;
	//pthread_t md_tid;
    GADI_U8  ref_FrameNum;

	// cover
	ST_GK_VIN_COVER_ATTR cover_attr[GK_VIN_CH_BACKLOG_REF][GK_VIN_COVER_BACKLOG_REF];
}ST_GK_VIN_ATTR, *PS_GK_VIN_ATTR;

//*****************************************************************************
//*****************************************************************************
//** Global Data
//*****************************************************************************
//*****************************************************************************
extern GADI_SYS_HandleT viHandle;
extern GADI_SYS_HandleT voutHandle;
extern GADI_SYS_HandleT vencHandle;

//*****************************************************************************
//*****************************************************************************
//** Local Data
//*****************************************************************************
//*****************************************************************************
static GADI_SYS_HandleT pmHandle =  NULL;
static GADI_SYS_HandleT pdaHandle =  NULL;

static ST_GK_VIN_ATTR   _sdk_vin;
static GADI_U8 sdk_vin_inited = 0;
//static zbar_image_scanner_t *scanner = NULL;

//*****************************************************************************
//*****************************************************************************
//** Local Functions Declaration
//*****************************************************************************
//*****************************************************************************


//*****************************************************************************
//*****************************************************************************
//** Local Functions
//*****************************************************************************
//*****************************************************************************



static GADI_S32 vin_get_input_frame_rate(void)
{
    GADI_VI_SettingParamsT viParam;
    // get vi framerate for reference
    LOG_CHECK(gadi_vi_get_params(viHandle, &viParam));
    switch(viParam.fps)
    {
        case GADI_VI_FPS_3_125:
        case GADI_VI_FPS_3_75:
            viParam.fps = 3;
            break;
        case GADI_VI_FPS_6_25:
            viParam.fps = 6;
            break;
        case GADI_VI_FPS_7_5:
            viParam.fps = 7;
            break;
        case GADI_VI_FPS_12_5:
            viParam.fps = 12;
            break;
        case GADI_VI_FPS_23_976:
            viParam.fps = 24;
            break;
        case GADI_VI_FPS_29_97:
            viParam.fps = 30;
            break;
        case GADI_VI_FPS_59_94:
            viParam.fps = 30;
            break;
        default:
            break;
    }
    return (GADI_S32)viParam.fps;
}



//*****************************************************************************
//*****************************************************************************
//** API Functions
//*****************************************************************************
//*****************************************************************************





/*!
*******************************************************************************
** \brief set vi privacy mask
**
** \param[in]  vin          video input id
** \param[in]  cover        privacy mask attribute
**
** \return
** - #SDK_OK   success
**   other     failed
**
*******************************************************************************
*/
int sdk_vin_set_cover(int vin, int id, PS_GK_VIN_COVER_ATTR cover)
{
    GADI_U8 pmIndex=0;

    int x = 0, y = 0, width = 0, height = 0;
    GADI_PM_MallocParamsT pmParams;
    PS_GK_VIN_COVER_ATTR  coverAttr = NULL;
    GADI_U32 viWidth, viHeight;

    /*check input parameters.*/
    if((vin >= GK_VIN_CH_BACKLOG_REF) || (id >= GK_VIN_COVER_BACKLOG_REF) || (cover == NULL))
    {
        LOG_ERR("sdk_vin_set_cover: input invalid parameters.\n");
        return -1;
    }

    LOG_INFO("sdk_vin_set_cover, vin:%d,id:%d, enable:%d,x:%f,y:%f,w:%f,h:%f,color:0x%06X\n",
             vin, id, cover->enable, cover->x, cover->y, cover->width, cover->height, cover->color);

    coverAttr= &_sdk_vin.cover_attr[vin][id];

    if((cover->x >= 1.0) || (cover->y >= 1.0) ||
       (cover->width < 0.0) || (cover->width > 1.0) ||
       (cover->height < 0.0) || (cover->height > 1.0))
    {
        LOG_ERR("sdk_vin_set_cover: invalid RECT parameters.\n");
        return -1;
    }
    if(coverAttr != cover) // gk_vin_reset_cover use
        memcpy(coverAttr, cover, sizeof(ST_GK_VIN_COVER_ATTR));
#if 0
    if(gadi_vi_get_resolution(viHandle, &viWidth,&viHeight) < 0)
    {
        LOG_ERR("get venc channel parameters.\n");
        return -1;
    }
#endif
    viWidth = 0;
    viHeight = 0;	//xqq no adi
    x      = (int)(coverAttr->x * (float)viWidth);
    y      = (int)(coverAttr->y * (float)viHeight);
    width  = (int)(coverAttr->width * (float)viWidth);
    height = (int)(coverAttr->height * (float)viHeight);
    //if encode is mirror, so refixed position.
    if(sdk_osd_get_mirror()){
        x = viWidth- x - width;
    }
    //if encode is flip, so refixed position.
    if(sdk_osd_get_flip()){
        y = viHeight- y - height;
    }
    x      = SDK_ALIGNED_LITTLE_ENDIAN(x, 2);
    y      = SDK_ALIGNED_LITTLE_ENDIAN(y, 4);
    width  = SDK_ALIGNED_LITTLE_ENDIAN(width, 2);
    height = SDK_ALIGNED_LITTLE_ENDIAN(height, 4);

    pmParams.srcId   = 0;
    pmParams.unit    = 1;
    pmParams.offsetX = x;
    pmParams.offsetY = y;
    pmParams.width   = width;
    pmParams.height  = height;
    pmParams.colour  = coverAttr->color;

    if(coverAttr->enable)
    {
        pmParams.action  = id == 0 ? GADI_PM_REPLACE : GADI_PM_ADD_INC; /*id == 0: replace action means replace all old covers by a new covers.*/
#if 0
        if(gadi_pm_malloc(pmHandle, &pmParams, &pmIndex) < 0)
        {
            LOG_ERR("VIN: pm_malloc action:%d failed.\n", pmParams.action);
            return  -1;
        }
        if(gadi_pm_enable(pmHandle, pmIndex, 1))
        {
            LOG_ERR("VIN: pm_enable failed.\n");
            return -1;
        }
#endif		//xqq
    }
    else if(id == 0)
    {
        pmParams.action = GADI_PM_REMOVE_ALL;
#if 0
        if(gadi_pm_malloc(pmHandle, &pmParams, &pmIndex) < 0)
        {
            LOG_ERR("VIN: pm_malloc action:%d failed.\n", pmParams.action);
            return  -1;
        }
        if(gadi_pm_enable(pmHandle, pmIndex, 1))
        {
            LOG_ERR("VIN: pm_disable failed.\n");
            return -1;
        }
#endif		//xqq
    }    
    return 0;
}

/*!
*******************************************************************************
** \brief get vi privacy mask
**
** \param[in]  vin          video input id
** \param[out]  cover        privacy mask attribute
**
** \return
** - #SDK_OK   success
**   other     failed
**
*******************************************************************************
*/
int sdk_vin_get_cover(int vin, int id, PS_GK_VIN_COVER_ATTR cover)
{
    int i = 0 ;
    LOG_INFO("sdk_vin_get_cover: call-> vin:%d,id:%d\n",  vin,id);

    if(vin < GK_VIN_CH_BACKLOG_REF && id < GK_VIN_COVER_BACKLOG_REF)
    {
        PS_GK_VIN_COVER_ATTR const cover_attr = &_sdk_vin.cover_attr[vin][i];
        if(cover)
        {
            memcpy(cover, cover_attr, sizeof(ST_GK_VIN_COVER_ATTR));
            return 0;
        }
    }
    return -1;
}

/*!
*******************************************************************************
** \brief reset vi privacy mask
** \after stop & start encode stream, the privacy mask must be reseted.
**
** \return
** - #SDK_OK   success
**   other     failed
**
*******************************************************************************
*/
int gk_vin_reset_cover(void)
{
    uint8_t cnt;
    PS_GK_VIN_COVER_ATTR  coverAttr = NULL;
    
    for(cnt=0; cnt<4; cnt++)
    {
        coverAttr= &_sdk_vin.cover_attr[0][cnt];
        sdk_vin_set_cover(0, cnt, coverAttr);
    }

    return 0;
}


/*!
*******************************************************************************
** \brief vin module init
**
** \return
** - #SDK_OK   success
**   other     failed
**
*******************************************************************************
*/
int sdk_vin_init()
{
    GADI_ERR retVal = GADI_OK;
	
    LOG_INFO("sdk_vin_init: call-> vin\n");
    if(0 == sdk_vin_inited)
    {
#if 0
        if(gadi_pm_init() != GADI_OK)
        {
            LOG_ERR("VIN: pm init error\n");
            return -1;
        }
		pmHandle = gadi_pm_open(&retVal, 0);
#endif	//xqq
        if( pmHandle == NULL)
        {
            LOG_ERR("VIN: pm open error\n");
            return -1;
        }
        memset(&_sdk_vin, 0, sizeof(_sdk_vin));
        sdk_vin_inited = 1;
        return 0;
    }
    return -1;
}

/*!
*******************************************************************************
** \brief vin module exit
**
** \return
** - #SDK_OK   success
**   other     failed
**
*******************************************************************************
*/
int sdk_vin_destroy()
{
    int i = 0;

    if(sdk_vin_inited)
    {
        //gadi_pm_close(pmHandle);	xqq
        sdk_vin_inited = 0;
        return 0;
    }
    return -1;
}


ssize_t sdk_vin_yuv420sem_to_bitmap888(const void *yuv420sem_y,const void *yuv420sen_uv, size_t yuv_width, size_t yuv_height, size_t yuv_stride,
	void *bmp888)
{
	int i = 0, ii = 0;

	// allocate the bitmap data
	size_t bitmap24_size = yuv_width * yuv_height * 3;
	uint8_t *bitmap_offset = bmp888;

	//LOG_DEBUG("BMP [%dx%d] stride %d", yuv_width, yuv_height, yuv_stride);

	if(yuv420sem_y){
		int y, u, v, yy, vr, ug, vg, ub;
		int r, g, b;
		const uint8_t *py = (uint8_t*)(yuv420sem_y);
		const uint8_t *puv = (uint8_t*)(yuv420sen_uv);

		// yuv420 to rgb888
		for(i = 0; i < yuv_height; ++i){
            py  = (uint8_t*)(yuv420sem_y + yuv_stride * i);
            puv = (uint8_t*)(yuv420sen_uv+ yuv_stride * (i>>1));
			for(ii = 0; ii < yuv_width; ++ii){
				y = py[0];
				yy = y * 256;

				u = puv[0] - 128;
				ug = 88 * u;
				ub = 454 * u;

				v = puv[1] - 128;
				vg = 183 * v;
				vr = 359 * v;
				// convert
				r = (yy + vr) >> 8;
				g = (yy - ug - vg) >> 8;
				b = (yy + ub) >> 8;

				if(r < 0){
					r = 0;
				}
				if(r > 255){
					r = 255;
				}
				if(g < 0){
					g = 0;
				}
				if(g > 255){
					g = 255;
				}
				if(b < 0){
					b = 0;
				}
				if(b > 255){
					b = 255;
				}

				*bitmap_offset++ = (uint8_t)b;
				*bitmap_offset++ = (uint8_t)g;
				*bitmap_offset++ = (uint8_t)r;
				++py;
				if(0 != (ii % 2)){
					// even
					puv += 2;
				}
			}

		}
		return bitmap24_size;
	}
	return -1;
}

int sdk_vin_capture_data(int vin, char** data, int *width, int *height)
{
    printf("not support, %s, %d\n", __func__, __LINE__);
	return 0;
}

