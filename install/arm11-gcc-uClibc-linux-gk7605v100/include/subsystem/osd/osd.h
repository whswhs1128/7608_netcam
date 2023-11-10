/*!
*****************************************************************************
** file        osd.h
**
** version     2016-05-11 14:43 heyong -> v1.0
**
** brief       the osd user api defines.
**
** attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2016-2020 BY GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#ifndef _OSD_H_
#define _OSD_H_

#ifdef __cplusplus
extern "C" {
#endif

#include "canvas.h"
#include "sdk_osd.h"
#include "sdk_enc.h"


#define OSD_MULTI_TEXT_LINE_CNT	10
/*!
*******************************************************************************
** brief the struct define of osd module.
*******************************************************************************
*/
typedef struct osd_s {
    int stream_id;
    int area_id;
    int enable;
	
	int is_multi_line;
	int is_right_align;
	char line_text[OSD_MULTI_TEXT_LINE_CNT][128];
	
    float x_ratio; // [0~1.0]the ratio of stream_width and x_position.
    float y_ratio; // [0~1.0]the ratio of stream_width and y_position.
    int font_size;  // font size
    char text[128];  // text buffer
    int text_size;  // text buffer

    canvas_t *canvas;
	canvas_t *canvas_rect;
} osd_t;

/*!
*******************************************************************************
** brief init osd module.
**
** return
** - #>=0 , successed.
** - #<0  , failed.
**
*******************************************************************************
*/
int osd_init();

/*!
*******************************************************************************
** brief deinit osd module.
**
** return
** - #>=0 , successed.
** - #<0  , failed.
**
*******************************************************************************
*/
int osd_deinit();

/*!
*******************************************************************************
** brief set osd display params.
**
** param[in]  stream_id the stream id.
** param[in]  area_id   the area id.
** param[in]  x_ratio   [0~1.0]the ratio of stream_width and x_position.
** param[in]  y_ratio   [0~1.0]the ratio of stream_width and y_position.
** param[in]  text      the display string.
** param[in]  font_size the font size of display.
**
** return
** - #=0 , successed.
** - #<0 , failed.
**
*******************************************************************************
*/
int osd_set_display(int stream_id, int area_id, float x_ratio, float y_ratio, char *text, int font_size);

/*!
*******************************************************************************
** brief enable or disable osd display.
**
** param[in]  stream_id the stream id.
** param[in]  area_id   the area id.
** param[in]  enable    0->disable osd display, 1->enable osd display.
**
** return
** - #=0 , successed.
** - #<0 , failed.
**
*******************************************************************************
*/
int osd_set_enable(int stream_id, int area_id, int enable);

/*!
*******************************************************************************
** brief reload all osd.
*******************************************************************************
*/
void osd_reload_all();

/*!
*******************************************************************************
** brief the osd test function.
*******************************************************************************
*/
void osd_test();



int osd_set_display_rect(int stream_id, int area_id, float x_ratio, float y_ratio, int rec_width,int rec_height,char *text, int font_size);


int osd_set_display_multiple_line(int stream_id, int area_id, float x_ratio, float y_ratio ,int font_size, char line_text[OSD_MULTI_TEXT_LINE_CNT][128], int is_right_align);

#ifdef __cplusplus
};
#endif
#endif /* _OSD_H_ */

