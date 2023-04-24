#include <stdio.h>
#include <string.h>
#include <math.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include "osd_font.h"
#include "utf8_to_unicode.h"

#define FONT_FILE OSD_FONT_DIR"/DroidSansFallback.ttf"
FT_Library library; //用来初始化freetype这个开源库
FT_Face    face;    //对应于类似于"simsun.ttc"这样的字库
pthread_mutex_t ft_mutex = PTHREAD_MUTEX_INITIALIZER;

static void text_style2color(text_style_t style, canvas_pixel_t *color)
{
    canvas_pixel_t style_color;
    style_color.argb8888 = 0;

	// check style
	if(style & (TEXT_STYLE_BACKGROUND_TRANSPARENT|TEXT_STYLE_FOREGROUND_TRANSPARENT|TEXT_STYLE_ENCLOSED_TRANSPARENT))
    {
		RGB_PIXEL_FILL(style_color, 0, 0, 0, 0);
	}
    else if(style & (TEXT_STYLE_BACKGROUND_WHRITE|TEXT_STYLE_FOREGROUND_WHRITE|TEXT_STYLE_ENCLOSED_WHRITE))
	{
		RGB_PIXEL_FILL(style_color, 0xFF, 255, 255, 255);
	}
    else if(style & (TEXT_STYLE_BACKGROUND_BLACK|TEXT_STYLE_FOREGROUND_BLACK|TEXT_STYLE_ENCLOSED_BLACK))
    {
		RGB_PIXEL_FILL(style_color, 0xFF, 0, 0, 0);
	}
    if(color)
        memcpy(color, &style_color, sizeof(canvas_pixel_t));
}

static int draw_enclosed(canvas_t *canvas, int x, int y, int font_size, canvas_pixel_t bg_color, canvas_pixel_t fg_color, canvas_pixel_t enclosed_color)
{
    canvas_pixel_t center_color, right_color, bottom_color;
    int i, j, text_width, text_height;
    if(!canvas)
    {
		return -1;
	}
    text_height = canvas->height;//font_size < canvas->height ? font_size : canvas->height;
    text_width  = canvas->width_min;
    for(i = 0; i < text_height - 1 ; ++i)
    {
        for(j = 0; j < text_width - 1 ; ++j)
        {
            canvas_get_pixel(canvas, x + j, y + i, &center_color);
            if(0 == i || text_height - 1 == i ||
               0 == j || text_width - 1 == j)
            {
                // on the edage
                if(canvas_match_pixel(canvas, fg_color, center_color))
                {
                    canvas_put_pixel(canvas, x + j, y + i, enclosed_color);
                }
            }
            else
            {
                canvas_get_pixel(canvas, x + j + 1, y + i, &right_color); // to the right
                canvas_get_pixel(canvas, x + j, y + i + 1, &bottom_color); // to the bottom

                if(canvas_match_pixel(canvas, bg_color, center_color))
                {
                    // background
                    if(canvas_match_pixel(canvas, fg_color, right_color) ||
                       canvas_match_pixel(canvas, fg_color, bottom_color))
                    {
                        canvas_put_pixel(canvas, x + j, y + i, enclosed_color);
                    }
                }
                else if(canvas_match_pixel(canvas, fg_color, center_color))
                {
                    // forground
                    if(canvas_match_pixel(canvas, bg_color, right_color))
                    {
                        canvas_put_pixel(canvas, x + j + 1, y + i, enclosed_color);
                    }
                    if(canvas_match_pixel(canvas, bg_color, bottom_color))
                    {
                        canvas_put_pixel(canvas, x + j, y + i + 1, enclosed_color);
                    }
                }
            }
        }
    }
    return 0;
}

/* Replace this function with something useful. */
static void draw_bitmap( FT_Bitmap* bitmap, FT_Int x, FT_Int y, canvas_t *canvas)
{
    FT_Int  xd, yd, xs, ys, dst_index, src_index;
    FT_Int  x_max = x + bitmap->width;
    FT_Int  y_max = y + bitmap->rows; 
    unsigned char *src_image = bitmap->buffer;
    unsigned char *dst_image = canvas->image;
    for ( yd = y, ys = 0; yd < y_max; yd++, ys++)    
    {
        dst_index = canvas->width*yd;
        src_index = bitmap->width*ys;
        for ( xd = x, xs = 0; xd < x_max; xd++, xs++ )
        {
            if ( xd < 0 || xd >= canvas->width || yd < 0 || yd >= canvas->height_max)
                continue;
            *(dst_image+dst_index+xd) = *(src_image+src_index+xs);
        }
    }
}

static void show_image(canvas_t *canvas, int xs, int ys, int font_size, text_style_t style)
{
    int  y, x, yp, index;
    char imgbit;
    char imgbitNext;
    char imgbitAbove;
    int lastY;
    int lastX;
    int isSetBorader = 0;
    int aboveTwo = 0;
    int isEnclosed = 1;
    unsigned char *image = canvas->image;    
    unsigned int *pixel = canvas->pixels;
    canvas_pixel_t color, fg_color, bg_color, enclosed_color;
    if(!style){
        if (font_size > 16)
            style = TEXT_STYLE_BACKGROUND_TRANSPARENT|TEXT_STYLE_FOREGROUND_WHRITE|TEXT_STYLE_ENCLOSED_BLACK;
        else
        {
            isEnclosed = 0;
            style = TEXT_STYLE_BACKGROUND_TRANSPARENT|TEXT_STYLE_FOREGROUND_WHRITE|TEXT_STYLE_ENCLOSED_WHRITE;
        }
	}
    text_style2color(style&TEXT_STYLE_BACKGROUND_BIT_AREA, &bg_color);
    text_style2color(style&TEXT_STYLE_FOREGROUND_BIT_AREA, &fg_color);
    text_style2color(style&TEXT_STYLE_ENCLOSED_BIT_AREA,   &enclosed_color);
    bg_color.alpha = 0x00;
    
    //memset(canvas->pixels, 0, canvas->width*canvas->height*sizeof(int));    
    for ( y = ys; y < font_size + ys; y++)
    {
        index = canvas->width*y;
        for ( x = xs; x < canvas->width; x++ ) // for ( x = 0; x < canvas->width_min; x++ )
        {
            *(pixel+index+x) = x < canvas->width_min ? bg_color.argb8888:0x0; //*(pixel+index+x) = bg_color.argb8888;
        }        
    }
    
    //yp = (canvas->height-(canvas->height_max-canvas->height_min))/2;
    yp = ys;
    lastY = canvas->height_max - 1;
    lastX = canvas->width_min - 1;
    for ( y = canvas->height_min; y < canvas->height_max; y++, yp++ )
    {
        index = canvas->width*y;
        for ( x = xs; x < canvas->width_min; x++ )
        {
            isSetBorader = 0;
            imgbit = *(image+index+x - xs);
            if (imgbit == 0)
	        {
                continue;
                color.argb8888 = bg_color.argb8888;
	        }
            else if(imgbit == 0xFF) 
            {
                color.argb8888 = fg_color.argb8888;
			} 
            else 
            {
                isSetBorader = 1;
                //printf("boader color value:%d\n", imgbit);
                color.argb8888 = enclosed_color.argb8888;
                //color.alpha    = imgbit;
                if (isEnclosed)
                    color.alpha    = 0xff;
                else
                    color.alpha    = imgbit;
			}

            if (style&TEXT_STYLE_ENCLOSED_BIT_AREA && isEnclosed)
            {
                aboveTwo = 0;
                if (!isSetBorader && y > 2)
                {
                    //上边的上边如果已经是黑的，那么就不再画了
                    imgbitNext = *(image+index+x - xs - canvas->width * 2);
                    imgbitAbove = *(image+index+x - xs - canvas->width);
                    if (imgbitNext != 0xff && imgbitAbove != 0x0)
                    {
                        aboveTwo = 1;
                    }
                }
                //aboveTwo = 0;

                if (aboveTwo == 0)
                {
                    if (!isSetBorader && x < lastX)
                    {
                        //右边
                        imgbitNext = *(image+index+x - xs + 1);
                        if (imgbitNext == 0)
                        {
                            isSetBorader = 1;
                        }
                    }
                    
                    if (!isSetBorader && x > 1)
                    {
                        //左边
                        imgbitNext = *(image+index+x - xs - 1);
                        if (imgbitNext == 0)
                        {
                            isSetBorader = 1;
                        }
                    }

                    if (!isSetBorader && y > 1)
                    {
                        //上边
                        imgbitNext = *(image+index+x - xs - canvas->width);
                        if (imgbitNext == 0)
                        {
                            isSetBorader = 1;
                        }
                    }

                    #if 1
                    if (!isSetBorader && y < lastY)
                    {
                        //下边
                        imgbitNext = *(image+index+x - xs + canvas->width);
                        if (imgbitNext == 0)
                        {
                            isSetBorader = 1;
                        }
                    }
                    #endif

                    if (isSetBorader && imgbit == 0xFF)
                    {
                        color.argb8888 = enclosed_color.argb8888;
                        //color.alpha    = imgbit;
                        color.alpha    = 0xFF;
                    }
                }
            }
            canvas_put_pixel(canvas, x, yp, color);
        }        
    }  
    if(style & TEXT_STYLE_ENCLOSED_ENABLE)
        draw_enclosed(canvas, xs, ys, font_size, bg_color, fg_color, enclosed_color);
}

int osd_font_init()
{
	FT_Error error;
	error = FT_Init_FreeType(&library);              /* initialize library */
	if (error)
    {
		printf("FT_Init_FreeType error:%d\n", error);
		return -1;
	}

	error = FT_New_Face(library, FONT_FILE, 0, &face); /* create face object */
	if (error)
    {
        printf("The font file %s could not be opened or read. Error:%d!", FONT_FILE, error);
        return -1;
    }
	return 0;
}

void osd_font_deinit()
{
	FT_Done_Face(face);
	FT_Done_FreeType(library);
    return;
}

int canvas_fill_text(canvas_t *canvas, int x, int y, const char *text, font_size_e font_size, text_style_t style)
{
    unsigned long ui_char_code;
    int len, n, num_chars, calc_width=1;
    const char *character = text;
    char utfChar[128] = {0};
    
    FT_GlyphSlot slot; //对应于类似于"simsun.ttc里面具体的比如'A'或者'好'这样的glyph"
    FT_Matrix    matrix;   /* transformation matrix */
    FT_Vector    pen;      /* untransformed origin  */
    FT_Error     error = 0;

    FT_BBox  bbox;   //里面包含具体glyph的宽高等具体参数
    FT_Glyph glyph;  //用来备份取出来的具体glyph
    
    if(!canvas||!text)
    {
		return -1;
	}

    
	utility_gbk_to_utf8(text, utfChar, sizeof(utfChar));
    character = utfChar;
    pthread_mutex_lock(&ft_mutex);
    num_chars  = strlen( utfChar );
    if(canvas->width_min)
    {
        calc_width = 0;
        canvas->image = calloc(canvas->width * canvas->height_max, sizeof(char));
        if(!canvas->image)
        {
            printf("canvas->image malloc failed!\n");
            pthread_mutex_unlock(&ft_mutex);
            return -1;
        }
    }

    FT_Set_Pixel_Sizes(face, font_size, 0);    /* Set pixel size*/
    /* error handling omitted */

    slot = face->glyph;        /*slot指向里面即将具体取出的glyph*/
    /* set up matrix */
    double angle = ( 0.0 / 360 ) * 3.14159 * 2;      /* use 0 degrees     */
    matrix.xx = (FT_Fixed)( cos( angle ) * 0x10000L );
    matrix.xy = (FT_Fixed)(-sin( angle ) * 0x10000L );
    matrix.yx = (FT_Fixed)( sin( angle ) * 0x10000L );
    matrix.yy = (FT_Fixed)( cos( angle ) * 0x10000L );

    /* the pen position in 26.6 cartesian space coordinates; */
    pen.x = 0 * 64;        /*pen的单位是1/64pixel*/
    pen.y = 0 * 64;

    for ( n = 0; n < num_chars; n+=len, character+=len)
    {
        len = enc_utf8_to_unicode(character, &ui_char_code);
		if(len <= 0)
		{
			printf("enc_utf8_to_unicode error, len:%d\n", len);
			break; 
		}
        /* set transformation 包括大小，旋转，坐标等*/
        FT_Set_Transform( face, &matrix, &pen );

        /* load glyph image into the slot (erase previous one)*//*  读出glyph并渲染，渲染就是画图，将图形画出来*/
        error = FT_Load_Char(face, ui_char_code, FT_LOAD_RENDER);
        if ( error )
    	{
    		printf("FT_Load_Char error!\n");
            continue;                 /* ignore errors */
    	}

        error = FT_Get_Glyph(face->glyph, &glyph); /*拷贝实例，下面要用*/
        if (error)
        {
            printf("FT_Get_Glyph error!\n");
            break;
        }

        FT_Glyph_Get_CBox(glyph, FT_GLYPH_BBOX_TRUNCATE, &bbox); /*获取各种信息*/
        
        /* now, draw to our target surface (convert position) */
        if(!calc_width)
        {
            draw_bitmap(&slot->bitmap, slot->bitmap_left, font_size - slot->bitmap_top, canvas);
        }
        else
        {
            int y = font_size - slot->bitmap_top;
            int y_max = y + slot->bitmap.rows;
            if(y < canvas->height_min)
                canvas->height_min = y;
            if(y_max > canvas->height_max)
                canvas->height_max = y_max;
            canvas->width_min += slot->advance.x/64;
        }
        FT_Done_Glyph(glyph);
        /* increment pen position */ /*根据上一个字体，更新下一个字体的起始坐标*/
        pen.x += slot->advance.x;
        pen.y += slot->advance.y;
    }
    if(!calc_width)
    {
        show_image(canvas, x, y, font_size, style);
        if(canvas->image)
            free(canvas->image);
    }
	pthread_mutex_unlock(&ft_mutex);

    return error;
}

int canvas_get_fill_text_width(canvas_t *canvas, const char *text, int font_size)
{
    return canvas_fill_text(canvas, 0, 0, text, font_size, 0);
}

