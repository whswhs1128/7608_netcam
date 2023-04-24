/*!
*****************************************************************************
** \file      GK-gk_pda.c
**
** \version	$id: gk_pda.c 19-05-16 17:49:10
**
** \brief
**
** \attention   this code is provided as is. goke microelectronics
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2019-2020 by goke microelectronics co.,ltd
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
#include "sdk_pda.h"

extern GADI_SYS_HandleT vencHandle;
//extern GK_PDA_MD_CALLBACK pdaMdCb;
GK_PDA_MD_CALLBACK pdaMdCb;	//xqq
static GADI_SYS_HandleT gPdaHandle[SDK_MAX_VIN_NUM];
static GADI_PDA_AttrT g_attr[SDK_MAX_VIN_NUM];
static SDK_PDA_YUV_ATTR_T lastYuvAttr;
static pthread_t pdaPid;
static char pddThreadRun = 0;

static SDK_PDA_DATA_T pdaData;

static void *pda_get_thread(void*arg)
{
    int i, j, k, ret, count, range;
    unsigned char *pbuf = NULL;
    unsigned char checkCnt = 0;

    int md_range_max = 0;	//add
	pthread_detach(pthread_self());

    while(pddThreadRun)
    {
        for(i = 0; i < SDK_MAX_VIN_NUM; i ++)
        {
            ret = sdk_pda_get_data(i, &pdaData);
            if(0 == ret)
            {
                GADI_VENC_StrAttr strAttr;                
                memset(&strAttr, 0, sizeof(GADI_VENC_StrAttr));
                pbuf = (unsigned char*)pdaData.pAddr;
                for(j = 0, count = 0; j < pdaData.totalNumByte; j++)
                {
                    for(k = 0; k < 8; k ++)
                    {
                        if((*(pbuf+j))>>k&0x1)
                            count++;
                    }
                }
                
//                ret = gadi_venc_get_stream_attr(vencHandle, &strAttr);	xqq
                if(ret != GADI_OK)
                {
                    LOG_ERR("gadi_venc_get_stream_attr error, ret:%d\n",ret);
                }

                if(strAttr.height == 1440)//pdaData.totalNumByte == 1800)
                {
                    md_range_max = 1000;   
                	//if(md_range_max != MD_FACTOR_MAX)
#if 0
					ISP_BULB_ModeEnumT ispMode = sdk_isp_get_bulb_mode();
					if ((ISP_BULB_MODE_IR == ispMode) || (ISP_BULB_MODE_MIX == ispMode))
					{
						if (1 == gk_isp_get_led_state())
						{
							printf("count:%d\n",count);
						}
					}
#endif
                }
                else
                {
                	if(md_range_max != 500)
                		md_range_max = 500;  		
				}
				
				range = md_range_max*count/8/pdaData.totalNumByte;                    
                if (count != 0 && range == 0)
                {
                    //range = 1;
                }
                if(pdaMdCb)
                    pdaMdCb(i, range);                
                if (checkCnt++ == 254 && (access("/tmp/pda", F_OK) == 0))
                    printf("range:%d, %d, %d, %d\n", range, md_range_max, count, pdaData.totalNumByte);
            }
        }
        usleep(40000);
    }
    return NULL;
}

static inline GADI_S32 delete_padding_from_strided_y
    (GADI_U8* output_y, const GADI_U8* input_y, GADI_S32 pitch, GADI_S32 width, GADI_S32 height)
{
    GADI_S32 row;
    for (row = 0; row < height; row++) {         //row
        memcpy(output_y, input_y, width);
        input_y = input_y + pitch;
        output_y = output_y + width ;
    }
    return 0;
}

static inline GADI_S32 delete_padding_and_deinterlace_from_strided_uv
    (GADI_U8* output_uv, const GADI_U8* input_uv, GADI_S32 pitch, GADI_S32 width, GADI_S32 height,GADI_U8 isPannel)
{
    GADI_S32 row, i;
    GADI_U8 * output_u = output_uv;
    GADI_U8 * output_v = output_uv + width * height;    //without padding

    for (row = 0; row < height; row++) {         //row
        for (i = 0; i < width; i++) {
            if (isPannel) {
                // U buffer and V buffer is plane buffer
                *output_u++ = *input_uv++;       //U Buffer
                *output_v++ =  *input_uv++;        //V buffer
            } else {
                // U buffer and V buffer is interlaced buffer
                *output_u++ = *input_uv++;
                *output_u++ = *input_uv++;
            }
        }
        input_uv += (pitch - width) * 2;        //skip padding
    }
    return 0;
}


static int save_yuv(GADI_PDA_Frame_InfoT *yuvInfo)
{
    GADI_PDA_Frame_InfoT info;
    FILE *yuv_stream = NULL;
    GADI_U8 * uvBuffer = NULL;
    GADI_U8 * yBuffer = NULL;
    GADI_U8 fileName[64];
    GADI_S32 uv_width, uv_height,uv_pitch;

    memcpy(&info, yuvInfo, sizeof(GADI_PDA_Frame_InfoT));


        yBuffer = malloc(info.width * info.height);
        if (yBuffer == NULL) {
            GADI_ERROR("Not enough memory for Y buffer:%dKB !\n",info.width * info.height);
            goto error_exit;
        }
        uvBuffer = malloc(info.width * info.height);
        if (uvBuffer == NULL) {
            GADI_ERROR("Not enough memory for UV buffer:%dKB !\n",info.width * info.height);
            goto error_exit;
        }

        sprintf((char*)fileName,"/root/pda_%d_%d_yv12.yuv",info.width,info.height);


        yuv_stream = fopen((char *)fileName,"wb");
        if(yuv_stream == NULL)
        {
            GADI_ERROR("pda_capture_bmp: open file error\n");
            goto error_exit;
        }
        printf("stride:%d, width:%d\n", info.stride,info.width);
        if (info.stride== info.width)
        {
            memcpy(yBuffer, info.yAddr, info.width * info.height);
        } else if (info.stride > info.width)
        {
            delete_padding_from_strided_y(yBuffer,
            info.yAddr, info.stride, info.width, info.height);
        } else
        {
            GADI_ERROR("stride size smaller than width!\n");
            goto error_exit;
        }

        //convert uv data from interleaved into planar format
        if (FRAME_FORMAT_YUV_SEMIPLANAR_420 == info.frameFormat) {
            uv_pitch = info.stride/ 2;
            uv_width = info.width / 2;
            uv_height = info.height / 2;
        } else { // FRAME_FORMAT_YUV_SEMIPLANAR_422
            uv_pitch = info.stride/ 2;
            uv_width = info.width / 2;
            uv_height = info.height;
        }

        delete_padding_and_deinterlace_from_strided_uv(uvBuffer,
            info.uvAddr, uv_pitch, uv_width, uv_height,1);

        fwrite(yBuffer, 1, info.width*info.height, yuv_stream);
        //fwrite(uvBuffer, 1, uv_width*uv_height*2, yuv_stream);

        GADI_INFO(" Cap ture yuv picture success: %s\n",fileName);
    error_exit:
        if(yBuffer != NULL)
        {
            free(yBuffer);
            yBuffer = NULL;
        }
        if(uvBuffer != NULL)
        {
            free(uvBuffer);
            uvBuffer = 0;
        }
        if(yuv_stream != NULL)
        {
            fclose(yuv_stream);
            yuv_stream = 0;
        }

    return 0;
}

int sdk_pda_init(void)
{
    int ret, i;

    memset(gPdaHandle, 0, sizeof(gPdaHandle));
    memset(g_attr, 0, sizeof(g_attr));
    memset(&lastYuvAttr, 0, sizeof(lastYuvAttr));
   // ret = gadi_pda_init();	xqq
    CHECK_CONTENT((ret != GADI_OK), -1, "gadi_pda_init fail\n");
    
    for(i = 0; i < SDK_MAX_VIN_NUM; i ++)
        sdk_pda_create(i);
    LOG_INFO( "pda create ok\n");
    pddThreadRun = 1;
    ret = pthread_create(&pdaPid, NULL, pda_get_thread, NULL);
    CHECK_CONTENT((0 != ret), -1, "create pda get data fail\n");
    return 0;
}

int sdk_pda_exit(void)
{
    int ret, i;

	pddThreadRun = 0;
	if(pdaPid)
	{
		pthread_join(pdaPid, NULL);
		pdaPid = 0;
	}
	for(i = 0; i < SDK_MAX_VIN_NUM; i ++)
		sdk_pda_destroy(i);

//    ret = gadi_pda_exit();	xqq
    CHECK_CONTENT((ret != GADI_OK), -1, "gadi_pda_exit fail\n");

    return 0;
}

int sdk_pda_create(int vin)
{
    LOG_INFO("pda: open %d\n", vin);
//    gPdaHandle[vin] = gadi_pda_open(&g_attr[vin], vin);	xqq
    LOG_INFO("pda: open success handle:%d:%p\n", vin, gPdaHandle[vin]);
    CHECK_CONTENT((NULL == gPdaHandle[vin]), -1, "gadi_pda_open fail\n");
    LOG_INFO("pda: open success handle:%d:%p\n", vin, gPdaHandle[vin]);

    return 0;
}

int sdk_pda_destroy(int vin)
{
    int ret;

//    ret = gadi_pda_close(gPdaHandle[vin]);	xqq
    CHECK_CONTENT((ret != GADI_OK), -1, "gadi_pda_close fail\n");
    return 0;
}

int sdk_pda_set_attr(int vin, SDK_PDA_ATTR_T *attr)
{
    GADI_PDA_AttrT gkPdaAttr;

    gkPdaAttr.width  = attr->width;
    gkPdaAttr.height = attr->height;
    //gadi_pda_set_attr(gPdaHandle[vin], &gkPdaAttr);	xqq
    return 0;
}

int sdk_pda_get_attr(int vin, SDK_PDA_ATTR_T *attr)
{
    int ret;
    GADI_PDA_AttrT pstAttr;
    if (gPdaHandle[vin] == NULL)
    {
        printf("gPdaHandle %d is null.\n", vin);
    }
//    ret = gadi_pda_get_attr(gPdaHandle[vin], &pstAttr);	xqq
    memcpy(attr, &pstAttr, sizeof(pstAttr));
    printf("--->sdk_pda_get_attr, %d, %d\n", pstAttr.width, pstAttr.height);
    return ret;
    return 0;
}

int sdk_pda_enable(int vin, bool falg)
{
    //sdk_pda_enable(vin, attr);
    return 0;
}

GADI_PDA_DataT gkPdaData;

int sdk_pda_get_data(int vin, SDK_PDA_DATA_T *pdaData)
{
    int ret;
//    ret = gadi_pda_get_data(gPdaHandle[vin],&gkPdaData, GADI_TRUE);	xqq
    if(ret != GADI_OK)
    {
        printf("gadi_pda_get_data fail:%d\n", ret);
        return -1;
    }
    pdaData->pAddr = gkPdaData.pAddr;
    pdaData->totalNumByte = gkPdaData.total_num_byte;
    pdaData->stride= gkPdaData.stride;
    pdaData->MbWidth = gkPdaData.u32MbWidth;
    pdaData->MbHeight = gkPdaData.u32MbHeight;
    return 0;
}

int sdk_pda_get_yuv(int vin, int bufferType, SDK_PDA_YUV_ATTR_T *yuvAttr)
{
	GADI_PDA_Frame_InfoT video_frame_info;
    int retVal;

//    retVal = gadi_pda_capture_buffer(gPdaHandle[vin], bufferType, &video_frame_info);	xqq
    CHECK_CONTENT((retVal != GADI_OK), -1, "gadi_pda_capture_buffer error,ret:%d,vi:%d\n", retVal, vin);

    yuvAttr->frameFormat = video_frame_info.frameFormat;
    yuvAttr->width = video_frame_info.width;
    yuvAttr->height = video_frame_info.height;
    yuvAttr->yAddr = video_frame_info.yAddr;
    yuvAttr->uvAddr = video_frame_info.uvAddr;
    yuvAttr->stride = video_frame_info.stride;

    memcpy(&lastYuvAttr, yuvAttr, sizeof(SDK_PDA_YUV_ATTR_T));
    //save_yuv(&video_frame_info);
    return 0;
}

GADI_PDA_DataT *sdk_pda_get_pda_p(void)
{
    return &gkPdaData;
}

int sdk_pda_get_yuv_last(SDK_PDA_YUV_ATTR_T *yuvAttr)
{
    memcpy(yuvAttr, &lastYuvAttr, sizeof(SDK_PDA_YUV_ATTR_T));
    return 0;
}

