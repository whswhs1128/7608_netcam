/*!
*****************************************************************************
** file        sdk_osd.c
**
** version     2016-05-08 16:21 heyong -> v1.0
**
** brief       the implementation of sdk_osd.h for goke platform.
**
** attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2016-2020 BY GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include "sdk_osd.h"
#include "adi_sys.h"
#include "adi_osd.h"

#define ROUND_DOWN(x, n)  ( (x) & ~((n)-1u) )
#define CLUT_MAX_COLOUR_NUM  (256)

static GADI_SYS_HandleT g_osd_handle = NULL;
static pthread_mutex_t g_osd_mutex;
static int g_osd_mirror = 0;
static int g_osd_flip = 0;
static SDK_OSD_RELOAD_ALL_CALLBACK g_osd_reload_all = NULL;

///////////////////////////////////////
// FIXME(heyong): for local osd area params.
#define IS_gadi_osd_get_area_params_INVALID    1

#if IS_gadi_osd_get_area_params_INVALID
static GADI_OSD_AreaParamsT g_osd_area_params[SDK_OSD_PLANE_NUM][SDK_OSD_AREA_NUM];
void _get_local_area_params(GADI_OSD_AreaParamsT *areaParams)
{
    GADI_OSD_AreaParamsT *area_param = &(g_osd_area_params[areaParams->planeId][areaParams->areaId]);

    area_param->planeId = areaParams->planeId;
    area_param->areaId = areaParams->areaId;
    memcpy(areaParams, area_param, sizeof(GADI_OSD_AreaParamsT));
}

void _set_local_area_params(GADI_OSD_AreaParamsT *areaParams)
{
    GADI_OSD_AreaParamsT *area_param = &(g_osd_area_params[areaParams->planeId][areaParams->areaId]);
    memcpy(area_param, areaParams, sizeof(GADI_OSD_AreaParamsT));
}
#endif
// local osd area params. end
//////////////////////////////////////

static int _get_colour_index(GADI_OSD_AreaMappingT *areaMapping, unsigned int argb8888)
{
    int cnt,  maxColNum;
    unsigned int *clutAddr;
    unsigned int vuya8888;

    /*bgra888 to uvya8888*/
    vuya8888 = utility_argb8888_to_uvya8888(argb8888);

    maxColNum = areaMapping->clutSize / sizeof(unsigned int);
    clutAddr  = (unsigned int*)areaMapping->clutStartAddr;

    /*find out the colour index. format: RGBA.*/
    for (cnt = 0; cnt < maxColNum; cnt++) {
        /*match colour, return the colour index.*/
        if (vuya8888 == *(clutAddr + cnt)) {
            return cnt;
        }
    }

    /*no this colour in CLUT, insert this colour in CLUT.*/
    /*colour 0 is 0x00000000.*/
    for (cnt = 1; cnt < maxColNum; cnt++) {
        if (*(clutAddr + cnt) == 0) {
            /*insert colour in CLUT, return the colour index.*/
            *(clutAddr + cnt) = vuya8888;
            return cnt;
        }
    }

    LOG_ERR("No colour space, only support 256 colour!!!\n");
    return -1;
}

int sdk_osd_init()
{
#if 0
	int retVal = gadi_osd_init();

	g_osd_handle = gadi_osd_open(&retVal);
    if (retVal != GADI_OK) {
        LOG_ERR("adi_osd_open error\n");
        return retVal;
    }
#endif	//xqq
	pthread_mutex_init(&g_osd_mutex, NULL);
	return 0;
}

int sdk_osd_deinit()
{
#if 0
    int retVal = gadi_osd_close(g_osd_handle);
    if (retVal != GADI_OK) {
        LOG_ERR("adi_osd_close error\n");
        return retVal;
    }
#endif		//xqq
	pthread_mutex_destroy(&g_osd_mutex);
	return 0;
}


void sdk_osd_set_reload_callback(SDK_OSD_RELOAD_ALL_CALLBACK reload)
{
    g_osd_reload_all = reload;
}

int sdk_osd_enable(int stream_id, int area_id, int enable)
{
    int retVal = GADI_OK;
    GADI_OSD_AreaParamsT  areaParams;

	pthread_mutex_lock(&g_osd_mutex);

	areaParams.planeId = stream_id;
	areaParams.areaId = area_id;

#if IS_gadi_osd_get_area_params_INVALID
    _get_local_area_params(&areaParams);
#else
    retVal = gadi_osd_get_area_params(g_osd_handle, &areaParams);
    if (retVal != GADI_OK) {
        LOG_ERR("get_area_params error\n");
        goto OUT;
    }
#endif

    if ((areaParams.enable == enable)
        ||(areaParams.width == 0)
        ||(areaParams.height == 0))
        goto OUT;

    areaParams.enable = enable;
#if IS_gadi_osd_get_area_params_INVALID
    _set_local_area_params(&areaParams);
#endif
//    retVal = gadi_osd_set_area_params(g_osd_handle, &areaParams);	xqq
    if (retVal != GADI_OK) {
        LOG_ERR("set_area_params error:%d\n", retVal);
        goto OUT;
    }

OUT:
	pthread_mutex_unlock(&g_osd_mutex);
    return retVal;
}

int sdk_osd_set_area_params(int stream_id, int area_id,
                            int area_width, int area_height,
                            int area_offsetx, int area_offsety)
{
    int retVal = GADI_OK;
    GADI_OSD_AreaParamsT  areaParams;
    GADI_OSD_AreaMappingT areaMapping;
    GADI_OSD_AreaIndexT areaIndex;

	pthread_mutex_lock(&g_osd_mutex);

	areaParams.planeId = stream_id;
	areaParams.areaId = area_id;
#if IS_gadi_osd_get_area_params_INVALID
    _get_local_area_params(&areaParams);
#else
    retVal = gadi_osd_get_area_params(g_osd_handle, &areaParams);
    if (retVal != GADI_OK) {
        LOG_ERR("get_area_params error\n");
        goto OUT;
    }
#endif
    /*get overlay area addr.*/
    areaIndex.planeId = stream_id;
    areaIndex.areaId  = area_id;
#if 0
    if (gadi_osd_get_area_mapping(g_osd_handle, areaIndex, &areaMapping) != GADI_OK)
    {
        LOG_ERR("get_area_mapping stream: %d, area: %d failed\n", stream_id, area_id);
        goto OUT;
    }
#endif		//xqq
    if((area_width * area_height)>areaMapping.areaSize)
        area_width = ROUND_DOWN((areaMapping.areaSize/area_height), 32);

	areaParams.width = (short)area_width;
	areaParams.height = (short)area_height;
	areaParams.offsetX = (short)area_offsetx;
	areaParams.offsetY = (short)area_offsety;

    /*set overlay disable when creating.*/
    areaParams.enable = 0;
#if IS_gadi_osd_get_area_params_INVALID
    _set_local_area_params(&areaParams);
#endif
//    retVal = gadi_osd_set_area_params(g_osd_handle, &areaParams);	xqq
    if (retVal != GADI_OK) {
        LOG_ERR("set_area_params error\n");
        goto OUT;
    }

OUT:
	pthread_mutex_unlock(&g_osd_mutex);
    return retVal;
}

int sdk_osd_set_channel_attr(int stream_id, int area_id,
                            int font_size, int lumThreshold)
{
    int retVal = GADI_OK;
    GADI_OSD_AreaIndexT areaIndex;
    GADI_OSD_ChannelAttrT chnAttr;

	pthread_mutex_lock(&g_osd_mutex);
    /*get overlay area addr.*/
    areaIndex.planeId = stream_id;
    areaIndex.areaId  = area_id;    
    //gadi_osd_get_channel_attr(g_osd_handle, &areaIndex, &chnAttr);	xqq
    chnAttr.invtColorEn = GADI_TRUE;
    chnAttr.lumThreshold = lumThreshold;
    chnAttr.invtAreaHeight = font_size;
    chnAttr.invtAreaWidth = font_size >> 1;
    chnAttr.invtColorMode = GADI_OSD_LT_LUM_THRESH;
//    gadi_osd_set_channel_attr(g_osd_handle, &areaIndex, &chnAttr);	xqq

OUT:
	pthread_mutex_unlock(&g_osd_mutex);
    return retVal;
}

int sdk_osd_set_data(int stream_id, int area_id,
                            void *pixel, int pixel_width, int pixel_height)
{
    int retVal = GADI_OK;
    unsigned char  colIndex;
    unsigned int argb8888;
    unsigned int pixelCnt, cnt, ignCnt, offset, i;
    unsigned int *pixelAddr   = NULL;
    unsigned char  *overlayAddr = NULL;
    unsigned int colArgb[CLUT_MAX_COLOUR_NUM] = {0};
    unsigned char  colTab[CLUT_MAX_COLOUR_NUM] = {0};

    GADI_OSD_AreaParamsT  areaParams;
    GADI_OSD_AreaMappingT areaMapping;
    GADI_OSD_AreaIndexT areaIndex;

	pthread_mutex_lock(&g_osd_mutex);

	areaParams.planeId = stream_id;
	areaParams.areaId = area_id;
#if IS_gadi_osd_get_area_params_INVALID
    _get_local_area_params(&areaParams);
#else
    retVal = gadi_osd_get_area_params(g_osd_handle, &areaParams);
    if (retVal != GADI_OK) {
        LOG_ERR("get_area_params error\n");
        goto OUT;
    }
#endif
    /*get overlay area addr.*/
    areaIndex.planeId = stream_id;
    areaIndex.areaId  = area_id;
#if 0
    if (gadi_osd_get_area_mapping(g_osd_handle, areaIndex, \
                              &areaMapping) != GADI_OK) {
        LOG_ERR("get_area_mapping stream: %d, area: %d failed\n",
                                        stream_id, area_id);
        goto OUT;
    }
#endif		//xqq
	pixelAddr   = (unsigned int*)pixel;
    overlayAddr = (unsigned char*)areaMapping.areaStartAddr;
    pixelCnt = pixel_width * pixel_height;
    if(pixelCnt>areaMapping.areaSize)
        pixelCnt = areaMapping.areaSize;
    /*write CLUT & overlay buffer.*/
    offset = 0;
    ignCnt = 0;

    for (cnt = 0; cnt < pixelCnt; ) {
        if (areaParams.width >= pixel_width) {
            argb8888 = *(pixelAddr + cnt);
            cnt++;
        } else {
            argb8888 = *(pixelAddr + cnt);
            /*ignore the data cross.*/
            if (ignCnt == areaParams.width) {
                cnt += pixel_width - areaParams.width;
                ignCnt = 0;
                continue;
            }
            ignCnt++;
            cnt++;
        }

        /*look out the col table.*/
        colIndex = 0x00;
        for (i = 0; i < CLUT_MAX_COLOUR_NUM; i++) {
            if (argb8888 == colArgb[i]) {
                colIndex = colTab[i];
                break;
            }

            if ((colArgb[i] == 0x00000000) && (colTab[i] == 0x00) && (i != 0)) {
                retVal = _get_colour_index(&(areaMapping), argb8888);
                if (retVal < 0 ) {
                    colIndex = 0x00;
                    break;
                } else {
                    colTab[i]  = retVal;
                    colArgb[i] = argb8888;
                    colIndex   = retVal;
                    break;
                }
            }
        }
        *(overlayAddr + offset) = colIndex;
        offset++;
    }

OUT:
    pthread_mutex_unlock(&g_osd_mutex);
    return retVal;
}


int gk_osd_set_mirror(int enable)
{
    g_osd_mirror = enable;
    if (g_osd_reload_all != NULL)
        g_osd_reload_all();
    return 0;
}

int sdk_osd_get_mirror()
{
    return g_osd_mirror;
}

int gk_osd_set_flip(int enable)
{
    g_osd_flip = enable;
    if (g_osd_reload_all != NULL)
        g_osd_reload_all();
    return 0;
}
int sdk_osd_get_flip()
{
    return g_osd_flip;
}

