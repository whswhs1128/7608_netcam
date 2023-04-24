/*!
*****************************************************************************
** file        osd.h
**
** version     2016-05-08 18:43 heyong -> v1.0
**
** brief       the osd sdk api defines.
**
** attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2016-2020 BY GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#ifndef _SDK_OSD_H_
#define _SDK_OSD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "sdk_debug.h"

#define SDK_OSD_PLANE_NUM       6
#define SDK_OSD_AREA_NUM        3

typedef void (*SDK_OSD_RELOAD_ALL_CALLBACK)(void);

int sdk_osd_init();
int sdk_osd_deinit();
void sdk_osd_set_reload_callback(SDK_OSD_RELOAD_ALL_CALLBACK reload);


int sdk_osd_get_mirror();
int sdk_osd_get_flip();
int sdk_osd_get_stream_resolution(int stream_id,
                            int *srteam_width, int *stream_height);

int sdk_osd_enable(int stream_id, int area_id, int enable);
int sdk_osd_set_area_params(int stream_id, int area_id,
                            int area_width, int area_height,
                            int area_offsetx, int area_offsety);
int sdk_osd_set_data(int stream_id, int area_id,
                            void *pixel, int pixel_width, int pixel_height);
int sdk_osd_set_channel_attr(int stream_id, int area_id,
                            int font_size, int lumThreshold);

#ifdef __cplusplus
};
#endif
#endif /* _SDK_OSD_H_ */

