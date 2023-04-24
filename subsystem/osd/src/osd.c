/*!
*****************************************************************************
** file        osd.c
**
** version     2016-05-10 16:21 heyong -> v1.0
**
** brief       the implementation of osd.h.
**
** attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2016-2020 BY GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include "osd.h"
#include "canvas.h"
#include "osd_font.h"

static osd_t g_osds[SDK_OSD_PLANE_NUM][SDK_OSD_AREA_NUM];

int osd_init()
{
    int i, j;
    osd_t *osd_temp = NULL;

	sdk_osd_init();
    sdk_osd_set_reload_callback(osd_reload_all);
    osd_font_init();

    memset(g_osds, 0, sizeof(g_osds));//reset all osd.
    for (i = 0; i < SDK_OSD_PLANE_NUM; i++) {
        for (j = 0; j < SDK_OSD_AREA_NUM; j++) {
            osd_temp = &(g_osds[i][j]);
            osd_temp->stream_id = i;
            osd_temp->area_id = j;
            osd_temp->enable = 0;
        }
    }
	return 0;
}

int osd_deinit()
{
    int i, j;
    osd_t *osd_temp = NULL;

    osd_font_deinit();
    sdk_osd_set_reload_callback(NULL);
	sdk_osd_deinit();

    for (i = 0; i < SDK_OSD_PLANE_NUM; i++) {
        for (j = 0; j < SDK_OSD_AREA_NUM; j++) {
            osd_temp = &(g_osds[i][j]);
            canvas_deinit(osd_temp->canvas);
        }
    }
    memset(g_osds, 0, sizeof(g_osds));
	return 0;
}

static int _osd_calc_area_size(osd_t *osd)
{
    int canvasX = 0, canvasY = 0;
    int canvasWidth = 0, canvasHeight = 0;
    int stream_width = 0;
    int stream_height = 0;

    canvas_t *canvas = NULL;
	
	if(osd->canvas_rect)
		canvas = osd->canvas_rect;
	else
		canvas = osd->canvas;

    sdk_venc_get_stream_resolution(osd->stream_id, &stream_width, &stream_height);

    canvasX = (int)(osd->x_ratio * (float)stream_width);
    canvasY = (int)(osd->y_ratio * (float)stream_height);
    canvasWidth = (canvas->width > stream_width)? stream_width:canvas->width;
    canvasHeight = (canvas->height > stream_height)? stream_height:canvas->height;
    
    /*alignment*/
    canvasWidth  = ROUND_UP(canvasWidth, 32);//osd->font_size);
    canvasHeight = ROUND_UP(canvasHeight, 4);
    
    /*adjustment*/
    if (canvasY + canvasHeight > stream_height) {
        canvasY = stream_height - canvasHeight;
    }

    if (canvasX + canvasWidth > stream_width) {
        canvasX = stream_width - canvasWidth;
    }
    
    //LOG_INFO("stream_width=%d, stream_height=%d \n", stream_width, stream_height);
    //LOG_INFO("before: W=%d, H=%d, X=%d, Y=%d \n", canvasWidth, canvasHeight, canvasX, canvasY);
	if(sdk_osd_get_mirror()){
		canvasX = stream_width - canvasX - canvasWidth;
        //LOG_INFO("mirror \n");
	}
	if(sdk_osd_get_flip()){
		canvasY = stream_height - canvasY - canvasHeight;
        //LOG_INFO("flip \n");
	}

    canvasX = ROUND_UP(canvasX, 4);
    canvasY = ROUND_UP(canvasY, 4);

    //LOG_INFO("end   : W=%d, H=%d, X=%d, Y=%d \n", canvasWidth, canvasHeight, canvasX, canvasY);
    sdk_osd_set_area_params(osd->stream_id, osd->area_id,
                                canvasWidth, canvasHeight, canvasX, canvasY);
    return 0;
}

static int _osd_show(osd_t *osd)
{
    if (osd == NULL) {
        LOG_ERR("_osd_show: osd is null\n");
        return -1;
    }
    canvas_t *canvas = NULL;
	
	if(osd->canvas_rect)
		canvas = osd->canvas_rect;
	else
		canvas = osd->canvas;

    // load font to canvas
    //printf("--->_osd_show %d,%d, %d\n", osd->canvas->width, osd->canvas->height, osd->enable);
    memset(osd->canvas->pixels, 0, osd->canvas->width * osd->canvas->height * sizeof(int));
	canvas_fill_text(osd->canvas, 0, 0, osd->text, osd->font_size, 0);
    //printf("osd draw:%s, width:%d, font:%d\n", osd->text, canvas->width, osd->font_size);

	if(osd->canvas_rect)
	{
		canvas_pixel_t pixel2;
		pixel2.argb8888= 0xffff0000;
		canvas_null_rect(osd->canvas_rect,pixel2);
		canvas_copy(osd->canvas,osd->canvas_rect);
	}
    sdk_osd_set_data(osd->stream_id, osd->area_id,
                        canvas->pixels, canvas->width, canvas->height);

    if (osd->enable)
        sdk_osd_enable(osd->stream_id, osd->area_id, osd->enable);
    return 0;
}

#ifdef MODULE_SUPPORT_VECTOR_FONT
int osd_set_display(int stream_id, int area_id, float x_ratio, float y_ratio, char *text, int font_size)
{
	int widch = 0, height=0;
	int need_calc_area_size = 0;
    // check input parameters.
    if ((stream_id >= SDK_OSD_PLANE_NUM) ||
            (area_id >= SDK_OSD_AREA_NUM)) {
        LOG_ERR("osd_set_enable: invalid params %d-%d\n", stream_id, area_id);
        return -1;
    }
    canvas_t canvas;

    if ((text == NULL)) {
        LOG_ERR("osd_set_display: invalid params text null \n");
        return -1;
    }
    
    canvas.width_min  = 0;
    canvas.height_min = font_size;
    canvas.height_max = font_size;
    canvas_get_fill_text_width(&canvas, text, font_size);
    widch  = canvas.width_min;
    height = canvas.height_max-canvas.height_min;
    if(height < font_size)
        height = font_size;
    //printf("osd_set_display---->%s, %d, %d, %d\n", text, widch, height, canvas.height_max);

    if ((text == NULL) || (widch <= 0)) {
        LOG_ERR("osd_set_display: invalid params text null %d.\n", widch);
        return -1;
    }

    osd_t *osd = &(g_osds[stream_id][area_id]);
	osd->is_multi_line = 0;

    if ((osd->x_ratio != x_ratio) || (osd->y_ratio != y_ratio))
        need_calc_area_size = 1;

    osd->x_ratio = x_ratio;
    osd->y_ratio = y_ratio;
    //LOG_INFO("osd_set_display: %d-%d, (%f,%f)%s\n", osd->stream_id, osd->area_id
    //                                        , osd->x_ratio, osd->y_ratio, osd->text);

    if ((osd->text_size != widch) || (osd->font_size != font_size) || osd->canvas == NULL) {
        //LOG_INFO("osd_set_display: update canvas %d-%d-%d-%d.\n",
        //            osd->text_size, lenth, osd->font_size, font_size);
        //if(!(1 == area_id && osd->text_size > widch&&(osd->text_size - widch)<font_size/2)) 
        {
            // save new value.
            osd->text_size = widch;
            osd->font_size = font_size;

            // update canvas size.
            canvas_deinit(osd->canvas);
            widch = BIG_ENDIAN_ALIGNED(widch, BIG_ENDIAN_ALIGNED(font_size, 32));
            osd->canvas = canvas_init(widch, height);
            //printf("-->osd malloc canvas:%x\n", (unsigned int)osd->canvas);
            if(!osd->canvas) {
                LOG_ERR("osd_set_display: canvas_init failed\n");
                return -1;
            }
            osd->canvas->width_min  = canvas.width_min;
            osd->canvas->height_min = canvas.height_min;
            osd->canvas->height_max = canvas.height_max;
            need_calc_area_size = 1;
        }
    }

	if(osd->canvas_rect)
	{
		canvas_deinit(osd->canvas_rect);
		osd->canvas_rect = NULL; 
		need_calc_area_size = 1;
	}

    if (need_calc_area_size) {
        // calc the osd area params.
        // ATTENTION(heyong): do not calc everytime, or osd will flicker.
        //printf("--->_osd_calc_area_size %d\n", _osd_calc_area_size);
        _osd_calc_area_size(osd);
    }

    strcpy(osd->text, text);
    //printf("-->osd show canvas:%d, %x, width:%d,%d\n", stream_id, (unsigned int)osd->canvas, osd->canvas->width, osd->canvas->height);
    _osd_show(osd);

    if (area_id != 1) //NETCAM_OSD_TITLE_AREA=0 NETCAM_OSD_CLOCK_AREA=1  NETCAM_OSD_ID_AREA=2
    {
        //title ƒ⁄¥Ê Õ∑≈µÙ£¨÷ª”– ±º‰≤ª Õ∑≈,Ω⁄ °ƒ⁄¥Ê
        //printf("-->osd free canvas:%x\n", (unsigned int)osd->canvas);
        canvas_deinit(osd->canvas);
        osd->canvas = NULL;
    }
	return 0;
}

static int calc_width(int stream_id, int text_len, int font_size)
{

	int lenth = 0;
	int width = 0;

	lenth = (text_len < 2 ? 2 : text_len%2 ? text_len+1 : text_len)/2;
	width = font_size * lenth;
	
	return width;
}


static int osd_reload_multiple_line(osd_t *osd) // TODO
{
    osd_set_display_multiple_line(osd->stream_id, osd->area_id, osd->x_ratio, osd->y_ratio, osd->font_size, osd->line_text, osd->is_right_align);
    return 0;
}


int osd_set_display_multiple_line(int stream_id, int area_id, float x_ratio, float y_ratio ,int font_size, char line_text[OSD_MULTI_TEXT_LINE_CNT][128], int is_right_align) // TODO
{
    #if 1
	int widch = 0, height=0;
	int need_calc_area_size = 0;
    int lenth, maxHeight, canvasWidth;
    int x = 0, y = 0, i;
    int width;
    int line_cnt;
    int lineMax = 0;
    int lineWidth[OSD_MULTI_TEXT_LINE_CNT] = {0};
    canvas_t canvas;
    
    // check input parameters.
    if ((stream_id >= SDK_OSD_PLANE_NUM) ||
            (area_id >= SDK_OSD_AREA_NUM)) {
        LOG_ERR("osd_set_enable: invalid params %d-%d\n", stream_id, area_id);
        return -1;
    }
            
    maxHeight = 0;   
    canvasWidth = 0;
	for(i=0; i<OSD_MULTI_TEXT_LINE_CNT; i++)
	{
		lenth = strlen(line_text[i]);
		if(lenth > 0)
		{
            canvas.width_min  = 0;
            canvas.height_min = font_size;
            canvas.height_max = font_size;
            canvas_get_fill_text_width(&canvas, line_text[i], font_size);
            maxHeight += canvas.height_max;
            if (canvas.width_min > canvasWidth)
                canvasWidth = canvas.width_min;
            if (canvas.height_max > lineMax)
                lineMax = canvas.height_max;
            lineWidth[i] = canvas.width_min;
		}
	}
    widch  = canvasWidth;    
	//widch = widch%font_size ? widch+(font_size-widch%font_size) : widch;
    height = maxHeight;
    if(height < font_size)
        height = font_size;
    //printf("osd_set_display---->%s, %d, %d, %d\n", text, widch, height, canvas.height_max);

    osd_t *osd = &(g_osds[stream_id][area_id]);
	osd->is_multi_line = 0;

    if ((osd->x_ratio != x_ratio) || (osd->y_ratio != y_ratio))
        need_calc_area_size = 1;

    osd->x_ratio = x_ratio;
    osd->y_ratio = y_ratio;
    //LOG_INFO("osd_set_display: %d-%d, (%f,%f)%s\n", osd->stream_id, osd->area_id
    //                                        , osd->x_ratio, osd->y_ratio, osd->text);

    osd->text_size = widch;
    osd->font_size = font_size;
    
    // update canvas size.
    canvas_deinit(osd->canvas);
    widch = BIG_ENDIAN_ALIGNED(widch, BIG_ENDIAN_ALIGNED(font_size, 32));
    osd->canvas = canvas_init(widch, height);
    if(!osd->canvas) {
        LOG_ERR("osd_set_display: canvas_init failed\n");
        return -1;
    }
    osd->canvas->width_min  = widch;
    osd->canvas->height_min = canvas.height_min;
    osd->canvas->height_max = lineMax;
    need_calc_area_size = 1;
    osd->is_multi_line = 1;
    osd->is_right_align = is_right_align;

    if (need_calc_area_size) {
        // calc the osd area params.
        // ATTENTION(heyong): do not calc everytime, or osd will flicker.
        _osd_calc_area_size(osd);
    }

	//canvas_fill_text(osd->canvas, 0, 0, line_text[0], osd->font_size, 0);
	line_cnt = 0;
	for(i=0; i<OSD_MULTI_TEXT_LINE_CNT; i++)
	{
		if(strlen(line_text[i]) == 0)
			continue;

		strcpy(osd->line_text[line_cnt++],line_text[i]);
		
		if(is_right_align)
		{
			width = lineWidth[i];//calc_width(stream_id, strlen(line_text[i]),font_size);
			printf("%s, width:%d, len:%d,canvasWidth:%d\n",line_text[i],width,strlen(line_text[i]),canvasWidth);
			if(width <= widch)
			{
				x = widch - width;
					
				if(x < 0)
					x = 0;
			}
			else
				x = 0;
		}

		canvas_fill_text(osd->canvas, x, y, line_text[i], osd->font_size, 0);
		
		y += osd->font_size;
	}
    sdk_osd_set_data(stream_id, osd->area_id, osd->canvas->pixels, osd->canvas->width, osd->canvas->height);
    
    if (osd->enable)
        sdk_osd_enable(stream_id, osd->area_id, osd->enable);
    
    canvas_deinit(osd->canvas);
    osd->canvas = NULL;
    #else
    int lenth = 0;
    int widch = 0;
    int canvasX = 0, canvasY = 0;
    int canvasWidth = 0, canvasHeight = 0;
    int stream_width = 0;
    int stream_height = 0;
    canvas_t *canvas = NULL;
    int i = 0, text_line_cnt = 0,max_size = 0;
    int x = 0, y = 0;
    int width = 0;
    int line_cnt = 0;
    
    // check input parameters.
    if ((stream_id >= SDK_OSD_PLANE_NUM) || (area_id >= SDK_OSD_AREA_NUM)) 
    {
        LOG_ERR("osd_set_enable: invalid params %d-%d\n", stream_id, area_id);
        return -1;
    }
    
    osd_t *osd = &(g_osds[stream_id][area_id]);

    for(i=0; i<OSD_MULTI_TEXT_LINE_CNT; i++)
    {
        lenth = strlen(line_text[i]);
        if(lenth > 0)
        {
            text_line_cnt++;
            if(lenth > max_size)
                max_size = lenth;

            printf("osd_set_display_multiple_line:%s, length:%d\n",line_text[i], lenth);
        }
    }

    if(max_size == 0)
    {
        LOG_ERR("osd_set_display_multiple_line: no text to show.\n");
        return -1;
    }

    osd->is_multi_line = 1;
    
    if(is_right_align)
        osd->is_right_align = 1;
    else
        osd->is_right_align = 0;
        
    memset(osd->line_text,0,sizeof(osd->line_text));
    
    osd->x_ratio = x_ratio;
    osd->y_ratio = y_ratio;

    lenth = (max_size+1)/2 + 1;
    widch = font_size * lenth;
    widch = widch%osd->font_size ? widch+(osd->font_size-widch%osd->font_size) : widch;
    
    sdk_venc_get_stream_resolution(osd->stream_id, &stream_width, &stream_height);
    
    canvasWidth = (widch > stream_width)? stream_width:widch;
    canvasHeight = (text_line_cnt*font_size > stream_height)? stream_height:text_line_cnt*font_size;
    
    /*alignment*/
    canvasWidth  = ROUND_UP(canvasWidth, 32);//osd->font_size);
    canvasHeight = ROUND_UP(canvasHeight, 4);

    if(is_right_align)  
    {
        canvasX = stream_width - canvasWidth;
        canvasY = stream_height - canvasHeight - font_size;
    }
    else
    {
        canvasX = (int)(osd->x_ratio * (float)stream_width);
        canvasY = (int)(osd->y_ratio * (float)stream_height);
    }
    
    /*adjustment*/
    if (canvasY + canvasHeight > stream_height) {
        canvasY = stream_height - canvasHeight;
    }
    
    if (canvasX + canvasWidth > stream_width) {
        canvasX = stream_width - canvasWidth;
    }
    if(sdk_osd_get_mirror()){
        canvasX = stream_width - canvasX - canvasWidth;
    }
    if(sdk_osd_get_flip()){
        canvasY = stream_height - canvasY - canvasHeight;
    }
    
#if 0	
    canvasX = ROUND_UP(canvasX, 4);
    canvasY = ROUND_UP(canvasY, 4);
#else

    canvasX = canvasX%4 ? (canvasX - canvasX%4) : canvasX;
    canvasY = canvasY%4 ? (canvasY - canvasY%4) : canvasY;

#endif
    
    sdk_osd_set_area_params(osd->stream_id, osd->area_id, canvasWidth, canvasHeight, canvasX, canvasY);

    canvas_deinit(osd->canvas);

    osd->text_size = lenth; 
    osd->canvas = canvas_init(canvasWidth, canvasHeight);
    osd->font_size = font_size;

    canvas = osd->canvas;

    
    osd->canvas->width_min  = canvasWidth;
    osd->canvas->height_min = font_size;
    osd->canvas->height_max = font_size + 10;
    
    for(i=0; i<OSD_MULTI_TEXT_LINE_CNT; i++)
    {
        if(strlen(line_text[i]) == 0)
            continue;

        strcpy(osd->line_text[line_cnt++],line_text[i]);
        
        if(is_right_align)
        {
            width = calc_width(osd->stream_id, strlen(line_text[i]),font_size);
            printf("width:%d, len:%d,canvasWidth:%d\n",width,strlen(line_text[i]),canvasWidth);
            if(width <= canvasWidth)
            {
                if(strlen(line_text[i])%2)
                    x = canvasWidth - width - osd->font_size/2;
                else
                    x = canvasWidth - width - osd->font_size;
                    
                if(x < 0)
                    x = 0;
            }
            else
                x = 0;
        }

        canvas_fill_text(osd->canvas, x, y, line_text[i], osd->font_size, 0);
        
        y += osd->font_size;
    }
    
    sdk_osd_set_data(osd->stream_id, osd->area_id,canvas->pixels, canvas->width, canvas->height);
    
    if (osd->enable)
        sdk_osd_enable(osd->stream_id, osd->area_id, osd->enable);
    #endif
    return 0;
}

#else
int osd_set_display(int stream_id, int area_id, float x_ratio, float y_ratio, char *text, int font_size)
{
    int lenth = 0;
	int widch = 0;
	int need_calc_area_size = 0;
    // check input parameters.
    if ((stream_id >= SDK_OSD_PLANE_NUM) ||
            (area_id >= SDK_OSD_AREA_NUM)) {
        LOG_ERR("osd_set_enable: invalid params %d-%d\n", stream_id, area_id);
        return -1;
    }

    // now only support FONT_SIZE_32 and FONT_SIZE_16.
    if ((font_size != FONT_SIZE_16) && (font_size != FONT_SIZE_32)) {
        LOG_ERR("osd_set_display: invalid font_size %d(%d-%d).\n", font_size, FONT_SIZE_16, FONT_SIZE_32);
        return -1;
    }

    if ((text == NULL) || ((lenth = strlen(text)) <= 0)) {
        LOG_ERR("osd_set_display: invalid params text null %d.\n", lenth);
        return -1;
    }

    osd_t *osd = &(g_osds[stream_id][area_id]);
	osd->is_multi_line = 0;

    if ((osd->x_ratio != x_ratio) || (osd->y_ratio != y_ratio))
        need_calc_area_size = 1;

    osd->x_ratio = x_ratio;
    osd->y_ratio = y_ratio;
    //LOG_INFO("osd_set_display: %d-%d, (%f,%f)%s\n", osd->stream_id, osd->area_id
    //                                        , osd->x_ratio, osd->y_ratio, osd->text);

    if ((osd->text_size != lenth) || (osd->font_size != font_size)) {
        //LOG_INFO("osd_set_display: update canvas %d-%d-%d-%d.\n",
        //            osd->text_size, lenth, osd->font_size, font_size);
        // save new value.
        osd->text_size = lenth;
        osd->font_size = font_size;

        // update canvas size.
        canvas_deinit(osd->canvas);
        lenth = (lenth < 2 ? 2 : lenth%2 ? lenth+1 : lenth)/2;
        widch = font_size * lenth;
        widch = widch%32 ? widch+(32-widch%32) : widch;
        osd->canvas = canvas_init(widch, font_size);

        need_calc_area_size = 1;
    }

    if(osd->canvas_rect)
	{
		canvas_deinit(osd->canvas_rect);
		osd->canvas_rect = NULL; 
		need_calc_area_size = 1;
	}

    if (need_calc_area_size) {
        // calc the osd area params.
        // ATTENTION(heyong): do not calc everytime, or osd will flicker.
        _osd_calc_area_size(osd);
    }

    strcpy(osd->text, text);
    _osd_show(osd);
	return 0;
}
#endif
int osd_set_enable(int stream_id, int area_id, int enable)
{
    // check input parameters.
    if ((stream_id >= SDK_OSD_PLANE_NUM) ||
            (area_id >= SDK_OSD_AREA_NUM)) {
        LOG_ERR("osd_set_enable: invalid params %d-%d\n", stream_id, area_id);
        return -1;
    }

    osd_t *osd = &(g_osds[stream_id][area_id]);
    if(osd->enable == enable && enable == 0)
    {
        return 0;
    }
    else
    {
        osd->enable = enable;
        return sdk_osd_enable(osd->stream_id, osd->area_id, osd->enable);
    }
}

#ifdef MODULE_SUPPORT_VECTOR_FONT
int osd_set_display_rect(int stream_id, int area_id, float x_ratio, float y_ratio, int rec_width,int rec_height,char *text, int font_size)
{
	int widch = 0, height=0;
	int need_calc_area_size = 0;
    // check input parameters.
    if ((stream_id >= SDK_OSD_PLANE_NUM) ||
            (area_id >= SDK_OSD_AREA_NUM)) {
        LOG_ERR("osd_set_enable: invalid params %d-%d\n", stream_id, area_id);
        return -1;
    }
	
    canvas_t canvas;
    canvas.width_min  = 0;
    canvas.height_min = font_size;
    canvas.height_max = font_size;
    canvas_get_fill_text_width(&canvas, text, font_size);
    widch  = canvas.width_min;
    height = canvas.height_max-canvas.height_min;
    if(height < font_size)
        height = font_size;

    if ((text == NULL) || (widch <= 0)) {
        LOG_ERR("osd_set_display: invalid params text null %d.\n", widch);
        return -1;
    }

    osd_t *osd = &(g_osds[stream_id][area_id]);
	osd->is_multi_line = 0;

    if ((osd->x_ratio != x_ratio) || (osd->y_ratio != y_ratio))
        need_calc_area_size = 1;

    osd->x_ratio = x_ratio;
    osd->y_ratio = y_ratio;
    //LOG_INFO("osd_set_display: %d-%d, (%f,%f)%s\n", osd->stream_id, osd->area_id
    //                                        , osd->x_ratio, osd->y_ratio, osd->text);
	if(osd->canvas)
	{
		canvas_deinit(osd->canvas);
		osd->canvas = NULL;
	}

		widch = ROUND_UP(canvas.width_min, 32);
		height = ROUND_UP(height, 4);
	    osd->canvas = canvas_init(widch, height);
				
	    if(!osd->canvas) {
	        LOG_ERR("osd_set_display: canvas_init failed\n");
	        return -1;
	    }
	    osd->canvas->width_min  = canvas.width_min;
	    osd->canvas->height_min = canvas.height_min;
	    osd->canvas->height_max = canvas.height_max;
			        // save new value.
	    osd->text_size = widch;
	    osd->font_size = font_size;

	#if 1

	if(osd->canvas_rect)
	{
		canvas_deinit(osd->canvas_rect);
		osd->canvas_rect = NULL;  
	}
	
	widch = ROUND_UP(rec_width, 32);
	height = ROUND_UP(rec_height, 4);;
    osd->canvas_rect = canvas_init(widch, height);
    if(!osd->canvas_rect) {
        LOG_ERR("osd_set_display: canvas_init failed\n");
        return -1;
    }
	#endif
	
  
    need_calc_area_size = 1;

    if (need_calc_area_size) {
        // calc the osd area params.
        // ATTENTION(heyong): do not calc everytime, or osd will flicker.
        _osd_calc_area_size(osd);
    }

    strcpy(osd->text, text);
    _osd_show(osd);
	return 0;
}
#else
int osd_set_display_rect(int stream_id, int area_id, float x_ratio, float y_ratio, int rec_width,int rec_height,char *text, int font_size)
{
    int lenth = 0;
	int widch = 0;
	int need_calc_area_size = 0;
    // check input parameters.
    if ((stream_id >= SDK_OSD_PLANE_NUM) ||
            (area_id >= SDK_OSD_AREA_NUM)) {
        LOG_ERR("osd_set_enable: invalid params %d-%d\n", stream_id, area_id);
        return -1;
    }
	
    // now only support FONT_SIZE_32 and FONT_SIZE_16.
    if ((font_size != FONT_SIZE_16) && (font_size != FONT_SIZE_32)) {
        LOG_ERR("osd_set_display: invalid font_size %d(%d-%d).\n", font_size, FONT_SIZE_16, FONT_SIZE_32);
        return -1;
    }

    if ((text == NULL) || ((lenth = strlen(text)) <= 0)) {
        LOG_ERR("osd_set_display: invalid params text null %d.\n", lenth);
        return -1;
    }

    osd_t *osd = &(g_osds[stream_id][area_id]);
	osd->is_multi_line = 0;

    if ((osd->x_ratio != x_ratio) || (osd->y_ratio != y_ratio))
        need_calc_area_size = 1;

    osd->x_ratio = x_ratio;
    osd->y_ratio = y_ratio;
    //LOG_INFO("osd_set_display: %d-%d, (%f,%f)%s\n", osd->stream_id, osd->area_id
    //                                        , osd->x_ratio, osd->y_ratio, osd->text);
    if ((osd->text_size != lenth) || (osd->font_size != font_size)) {
        //LOG_INFO("osd_set_display: update canvas %d-%d-%d-%d.\n",
        //            osd->text_size, lenth, osd->font_size, font_size);
        // save new value.
        osd->text_size = lenth;
        osd->font_size = font_size;

        // update canvas size.
        canvas_deinit(osd->canvas);
        lenth = (lenth < 2 ? 2 : lenth%2 ? lenth+1 : lenth)/2;
        widch = font_size * lenth;
        widch = widch%32 ? widch+(32-widch%32) : widch;
        osd->canvas = canvas_init(widch, font_size);

        need_calc_area_size = 1;
    }

	#if 1
	if(osd->canvas_rect)
	{
		canvas_deinit(osd->canvas_rect);
		osd->canvas_rect = NULL;  
	}
	
	widch = ROUND_UP(rec_width, 32);
    osd->canvas_rect = canvas_init(widch, font_size);
    if(!osd->canvas_rect) {
        LOG_ERR("osd_set_display: canvas_init failed\n");
        return -1;
    }
	#endif
	
  
    need_calc_area_size = 1;

    if (need_calc_area_size) {
        // calc the osd area params.
        // ATTENTION(heyong): do not calc everytime, or osd will flicker.
        _osd_calc_area_size(osd);
    }

    strcpy(osd->text, text);
    _osd_show(osd);
	return 0;
}

static int calc_width(int stream_id, int text_len, int font_size)
{

	int lenth = 0;
	int width = 0;

	lenth = (text_len < 2 ? 2 : text_len%2 ? text_len+1 : text_len)/2;
	width = font_size * lenth;
	
	return width;
}

int osd_set_display_multiple_line(int stream_id, int area_id, float x_ratio, float y_ratio ,int font_size, char line_text[OSD_MULTI_TEXT_LINE_CNT][128], int is_right_align)
{
    int lenth = 0;
	int widch = 0;
	int canvasX = 0, canvasY = 0;
	int canvasWidth = 0, canvasHeight = 0;
	int stream_width = 0;
	int stream_height = 0;
	canvas_t *canvas = NULL;
	int i = 0, text_line_cnt = 0,max_size = 0;
	int x = 0, y = 0;
	int width = 0;
	int line_cnt = 0;
	
    // check input parameters.
    if ((stream_id >= SDK_OSD_PLANE_NUM) || (area_id >= SDK_OSD_AREA_NUM)) 
    {
        LOG_ERR("osd_set_enable: invalid params %d-%d\n", stream_id, area_id);
        return -1;
    }
	
    // now only support FONT_SIZE_32 and FONT_SIZE_16.
    if ((font_size != FONT_SIZE_16) && (font_size != FONT_SIZE_32)) 
	{
        LOG_ERR("osd_set_display_multiple_line: invalid font_size %d(%d-%d).\n", font_size, FONT_SIZE_16, FONT_SIZE_32);
        return -1;
    }
	
    osd_t *osd = &(g_osds[stream_id][area_id]);

	for(i=0; i<OSD_MULTI_TEXT_LINE_CNT; i++)
	{
		lenth = strlen(line_text[i]);
		if(lenth > 0)
		{
			text_line_cnt++;
			if(lenth > max_size)
				max_size = lenth;

			printf("osd_set_display_multiple_line:%s, length:%d\n",line_text[i], lenth);
		}
	}

	if(max_size == 0)
	{
		LOG_ERR("osd_set_display_multiple_line: no text to show.\n");
		return -1;
	}

	osd->is_multi_line = 1;
	
	if(is_right_align)
		osd->is_right_align = 1;
	else
		osd->is_right_align = 0;
		
	memset(osd->line_text,0,sizeof(osd->line_text));
	
    osd->x_ratio = x_ratio;
    osd->y_ratio = y_ratio;

	lenth = (max_size+1)/2 + 1;
	widch = font_size * lenth;
	widch = widch%32 ? widch+(32-widch%32) : widch;
	
	sdk_venc_get_stream_resolution(osd->stream_id, &stream_width, &stream_height);
	
	canvasWidth = (widch > stream_width)? stream_width:widch;
	canvasHeight = (text_line_cnt*font_size > stream_height)? stream_height:text_line_cnt*font_size;
	
	/*alignment*/
	canvasWidth  = ROUND_UP(canvasWidth, 32);//osd->font_size);
	canvasHeight = ROUND_UP(canvasHeight, 4);

	if(is_right_align)	
	{
		canvasX = stream_width - canvasWidth;
		canvasY = stream_height - canvasHeight - font_size;
	}
	else
	{
		canvasX = (int)(osd->x_ratio * (float)stream_width);
		canvasY = (int)(osd->y_ratio * (float)stream_height);
	}
	
	/*adjustment*/
	if (canvasY + canvasHeight > stream_height) {
		canvasY = stream_height - canvasHeight;
	}
	
	if (canvasX + canvasWidth > stream_width) {
		canvasX = stream_width - canvasWidth;
	}
	if(sdk_osd_get_mirror()){
		canvasX = stream_width - canvasX - canvasWidth;
	}
	if(sdk_osd_get_flip()){
		canvasY = stream_height - canvasY - canvasHeight;
	}
	
#if 0	
	canvasX = ROUND_UP(canvasX, 4);
	canvasY = ROUND_UP(canvasY, 4);
#else

	canvasX = canvasX%4 ? (canvasX - canvasX%4) : canvasX;
	canvasY = canvasY%4 ? (canvasY - canvasY%4) : canvasY;

#endif
	
	sdk_osd_set_area_params(osd->stream_id, osd->area_id, canvasWidth, canvasHeight, canvasX, canvasY);

    canvas_deinit(osd->canvas);

	osd->text_size = lenth;	
    osd->canvas = canvas_init(canvasWidth, canvasHeight);
	osd->font_size = font_size;

	canvas = osd->canvas;
	
	for(i=0; i<OSD_MULTI_TEXT_LINE_CNT; i++)
	{
		if(strlen(line_text[i]) == 0)
			continue;

		strcpy(osd->line_text[line_cnt++],line_text[i]);
		
		if(is_right_align)
		{
			width = calc_width(osd->stream_id, strlen(line_text[i]),font_size);
			printf("width:%d, len:%d,canvasWidth:%d\n",width,strlen(line_text[i]),canvasWidth);
			if(width <= canvasWidth)
			{
				if(strlen(line_text[i])%2)
					x = canvasWidth - width - osd->font_size/2;
				else
					x = canvasWidth - width - osd->font_size;
					
				if(x < 0)
					x = 0;
			}
			else
				x = 0;
		}

		canvas_fill_text(osd->canvas, x, y, line_text[i], osd->font_size, 0);
		
		y += osd->font_size;
	}
	
	sdk_osd_set_data(osd->stream_id, osd->area_id,canvas->pixels, canvas->width, canvas->height);
	
    if (osd->enable)
        sdk_osd_enable(osd->stream_id, osd->area_id, osd->enable);

	return 0;
}

static int osd_reload_multiple_line(osd_t *osd)
{
    int lenth = 0;
	int widch = 0;
	int canvasX = 0, canvasY = 0;
	int canvasWidth = 0, canvasHeight = 0;
	int stream_width = 0;
	int stream_height = 0;
	canvas_t *canvas = NULL;
	int i = 0, text_line_cnt = 0,max_size = 0;
	int x = 0, y = 0;
	int width = 0;
	
    if(osd == NULL)
	{
		LOG_ERR("osd_reload_multiple_line: params NULL!\n");
		return -1;
	}

	if(osd->is_multi_line == 0)
	{
		LOG_ERR("osd_reload_multiple_line: not multi line osd!\n");
		return -1;
	}

    // check input parameters.
    if ((osd->stream_id >= SDK_OSD_PLANE_NUM) || (osd->area_id >= SDK_OSD_AREA_NUM)) 
    {
        LOG_ERR("osd_reload_multiple_line: invalid params %d-%d\n", osd->stream_id, osd->area_id);
        return -1;
    }
	
    // now only support FONT_SIZE_32 and FONT_SIZE_16.
    if ((osd->font_size != FONT_SIZE_16) && (osd->font_size != FONT_SIZE_32)) 
	{
        LOG_ERR("osd_reload_multiple_line: invalid font_size %d(%d-%d).\n", osd->font_size, FONT_SIZE_16, FONT_SIZE_32);
        return -1;
    }
	
	for(i=0; i<OSD_MULTI_TEXT_LINE_CNT; i++)
	{
		lenth = strlen(osd->line_text[i]);
		if(lenth > 0)
		{
			text_line_cnt++;
			if(lenth > max_size)
				max_size = lenth;

			printf("osd_reload_multiple_line:%s\n",osd->line_text[i]);
		}
	}

	if(max_size == 0)
	{
		LOG_ERR("osd_reload_multiple_line: no text to show.\n");
		return -1;
	}

	lenth = (max_size+1)/2 + 1;
	widch = osd->font_size * lenth;
	widch = widch%32 ? widch+(32-widch%32) : widch;
	
	sdk_venc_get_stream_resolution(osd->stream_id, &stream_width, &stream_height);
		
	canvasWidth = (widch > stream_width)? stream_width:widch;
	canvasHeight = (text_line_cnt*osd->font_size > stream_height)? stream_height:text_line_cnt*osd->font_size;
	
	/*alignment*/
	canvasWidth  = ROUND_UP(canvasWidth, 32);//osd->font_size);
	canvasHeight = ROUND_UP(canvasHeight, 4);
	
	if(osd->is_right_align)	
	{
		canvasX = stream_width - canvasWidth;
		canvasY = stream_height - canvasHeight - osd->font_size;
	}
	else
	{
		canvasX = (int)(osd->x_ratio * (float)stream_width);
		canvasY = (int)(osd->y_ratio * (float)stream_height);
	}
	
	/*adjustment*/
	if (canvasY + canvasHeight > stream_height) {
		canvasY = stream_height - canvasHeight;
	}
	
	if (canvasX + canvasWidth > stream_width) {
		canvasX = stream_width - canvasWidth;
	}
	if(sdk_osd_get_mirror()){
		canvasX = stream_width - canvasX - canvasWidth;
	}
	if(sdk_osd_get_flip()){
		canvasY = stream_height - canvasY - canvasHeight;
	}

#if 0	
	canvasX = ROUND_UP(canvasX, 4);
	canvasY = ROUND_UP(canvasY, 4);
#else

	canvasX = canvasX%4 ? (canvasX - canvasX%4) : canvasX;
	canvasY = canvasY%4 ? (canvasY - canvasY%4) : canvasY;

#endif
	sdk_osd_set_area_params(osd->stream_id, osd->area_id, canvasWidth, canvasHeight, canvasX, canvasY);

    canvas_deinit(osd->canvas);

    osd->canvas = canvas_init(canvasWidth, canvasHeight);

	canvas = osd->canvas;
	
	for(i=0; i<OSD_MULTI_TEXT_LINE_CNT; i++)
	{
		if(strlen(osd->line_text[i]) == 0)
			continue;
		
		if(osd->is_right_align)
		{
			width = calc_width(osd->stream_id, strlen(osd->line_text[i]),osd->font_size);
			printf("width:%d, len:%d,canvasWidth:%d\n",width,strlen(osd->line_text[i]),canvasWidth);
			if(width <= canvasWidth)
			{
				if(strlen(osd->line_text[i])%2)
					x = canvasWidth - width - osd->font_size/2;
				else
					x = canvasWidth - width - osd->font_size;
				
				if(x < 0)
					x = 0;
			}
			else
				x = 0;
		}
		
		canvas_fill_text(osd->canvas, x, y, osd->line_text[i], osd->font_size, 0);
		
		y += osd->font_size;
	}
	
	sdk_osd_set_data(osd->stream_id, osd->area_id,canvas->pixels, canvas->width, canvas->height);
	
    if (osd->enable)
        sdk_osd_enable(osd->stream_id, osd->area_id, osd->enable);

	return 0;
}

#endif

void osd_reload_all()
{
    osd_t *osd_temp = NULL;
    int i, j;
    //LOG_INFO("osd_reload_all: in.\n");
    for (i = 0; i < SDK_OSD_PLANE_NUM; i++) {
        for (j = 0; j < SDK_OSD_AREA_NUM; j++) {
            osd_temp = &(g_osds[i][j]);
            // reload(show) the opened osd handle.
            if ((osd_temp->enable != 0)) {
                //printf("--->osd_reload_all %d,%d\n", i, j);
                canvas_clear(osd_temp->canvas);
                // calc the osd area params.
                // ATTENTION(heyong): do not calc everytime, or osd will flicker.
                if(osd_temp->is_multi_line == 0)
                {
                    //printf("-->osd_reload_all, %d-", osd_temp->area_id);
                    if (osd_temp->canvas == NULL)
                    {
                        //printf("canvas is null-%s\n", osd_temp->text);
                        osd_set_display(osd_temp->stream_id, osd_temp->area_id, 
                            osd_temp->x_ratio, osd_temp->y_ratio, osd_temp->text, osd_temp->font_size);
                    }
                    else
                    {
                        //printf("%d,%d\n", osd_temp->canvas->width, osd_temp->canvas->height);
    	                _osd_calc_area_size(osd_temp);
    	                _osd_show(osd_temp);
                        //printf("-->osd show over\n");
                    }
                }
				else if(osd_temp->is_multi_line == 1)
				{
                    //printf("-->osd_reload_all, mutlie\n", osd_temp->area_id);
					osd_reload_multiple_line(osd_temp);
				}
            }
        }
    }
    //LOG_INFO("osd_reload_all: out.\n");
}

void osd_test()
{
    // you should call osd_init() at first.

    osd_set_display(0, 0, 0, 0, "≤‚ ‘ osd_s0_z0", 32);
    osd_set_enable(0, 0, 1);

    osd_set_display(0, 1, 0.2, 0.1, "≤‚ ‘ osd_s0_z1", 16);
    osd_set_enable(0, 1, 1);
    osd_set_display(0, 2, 0.4, 0.2, "≤‚ ‘ osd_s0_z2", 16);
    osd_set_enable(0, 2, 1);

    osd_set_display(1, 0, 0.2, 0, "≤‚ ‘ osd_s1_z0", 32);
    osd_set_enable(1, 0, 1);
    osd_set_display(1, 1, 0.4, 0.1, "≤‚ ‘ osd_s1_z1", 16);
    osd_set_enable(1, 1, 1);
    osd_set_display(1, 2, 0.6, 0.2, "≤‚ ‘ osd_s1_z2", 16);
    osd_set_enable(1, 2, 1);

    osd_set_display(2, 0 , 0.1, 0, "≤‚ ‘ osd_s2_z0", 32);
    osd_set_enable(2, 0, 1);
    osd_set_display(2, 1, 0.3, 0.1, "≤‚ ‘ osd_s2_z1", 16);
    osd_set_enable(2, 1, 1);
    osd_set_display(2, 2, 0.5, 0.2, "≤‚ ‘ osd_s2_z2", 16);
    osd_set_enable(2, 2, 1);
}
