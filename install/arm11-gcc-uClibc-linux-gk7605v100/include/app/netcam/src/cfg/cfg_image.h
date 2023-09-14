/*!
*****************************************************************************
** FileName     : cfg_image.h
**
** Description  : config for image.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-7-29
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_CFG_IMAGE_H__
#define _GK_CFG_IMAGE_H__

#ifdef __cplusplus
extern "C"
{
#endif

#include "cfg_common.h"

/***********************************/
/***          image              ***/
/***********************************/
typedef struct {
    SDK_U8	    flipEnabled; //��ֱ ��ת
    SDK_U8	    mirrorEnabled; // ˮƽ ��ת

    SDK_U8     sceneMode;  // ����ģʽ: 0 auto 1 indoor 2 outdoor
    SDK_U8     imageStyle; // ͼ����: 0 ��׼ 1 ���� 2 ����
    SDK_U8     wbMode;     // ��ƽ�� 0 auto 1 indoor 2 outdoor

    SDK_U8     irCutControlMode; // 0 software  ,  1 hardware
    SDK_U8     irCutMode;        // ircut��ҹģʽ 0 auto, 1 day, 2 night
    SDK_U8     enabledWDR;
    SDK_U8     strengthWDR; //��̬��Χ 1-5
    SDK_U8     enableDenoise3d;
    SDK_U8     strengthDenoise3d; // 3Dȥ��ǿ�� 1-5 ��ӦSDK 0-512

    SDK_U8     lowlightMode; // ���ն�ģʽ: 0 close, 1 only night, 2 day-night, 3 auto
    SDK_U8     exposureMode; /* �ع�ģʽ 0 - auto 1 - bright, 2 - dark */
    SDK_U8     dcIrisEnable; /* 1 - enable 0 - disable */
    SDK_U8     antiFlickerFreq; /* 50: 60  50HZ 60HZ */
    SDK_U8     backLightEnable;
    SDK_S32    backLightLevel;

    SDK_S32    brightness; /* 0 ~ 100 */
    SDK_S32    saturation; /* 0 ~ 100  */
    SDK_S32    contrast;   /* 0 ~ 100 */
    SDK_S32    sharpness;  /* 0 ~ 100 */
    SDK_S32    hue;        /* 0 ~ 100 */

    SDK_U8     lightMode;    /* ����ģʽ */
    SDK_U8     lightLevel;   /* �ֶ����ݿ��� */
    SDK_U8     led1;    /* ָʾ��1 */
    SDK_U8     led2;   /* ָʾ��2 */
} GK_NET_IMAGE_CFG;

extern int ImageCfgSave();
extern int ImageCfgLoad();
extern void ImageCfgPrint();
extern int ImageCfgLoadDefValue();
extern char *ImageCfgGetJosnString();

#define IMAGE_CFG_FILE "gk_image_cfg.cjson"

extern GK_NET_IMAGE_CFG runImageCfg;
extern GK_CFG_MAP imageMap[];

#ifdef __cplusplus
}
#endif
#endif

