/**
 * @file lv_ex_poring.c
 *
 */

/*
 * WELCOME TO THE LITTLEVGL TUTORIAL
 *
 *-------------------------------------------------------------------------------
 * Now you will learn how to port the Littlev Graphics Library to your system
 * ------------------------------------------------------------------------------
 *
 * 1. Before start download or clone the lvgl folder: https://github.com/littlevgl/lvgl.git
 * 2. Copy 'lvgl' into your project folder
 * 3. Copy 'lvgl/lv_conf_templ.h' and 'lv_conf.h' next to 'lvgl' and remove the first and last #if and  #endif
 * 4. To initialize the library your main.c should look like this file
 *
 *Here are some info about the drawing modes in the library:
 *
 * BUFFERED DRAWING
 * The graphics library works with an internal buffering mechanism to
 * create advances graphics features with only one frame buffer.
 * The internal buffer is called VDB (Virtual Display Buffer) and its size can be adjusted in lv_conf.h.
 * When LV_VDB_SIZE not zero then the internal buffering is used and you have to provide a function
 * which flushes the buffers content to your display.
 *
 * UNBUFFERED DRAWING
 * It is possible to draw directly to a frame buffer when the internal buffering is disabled (LV_VDB_SIZE = 0).
 * Keep in mind this way during refresh some artifacts can be visible because the layers are drawn after each other.
 * And some high level graphics features like anti aliasing, opacity or shadows aren't available in this configuration.
 * In this mode two functions are required:  fill and area with a color AND write a color array to an area
 *
 * HARDWARE ACCELERATION (GPU)
 * If your MCU supports graphical acceleration (GPU) then you can use it with two interface functions:
 * blend two memories using opacity AND fill a memory with a color.
 * Use only one function or none of them is right as well because every GPU function is optional
 */

/*********************
 *      INCLUDES
 *********************/
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "sdk_def.h"
#include "sdk_osd.h"
#include "sdk_api.h"
#include "lv_hal_porting.h"
#include "lv_conf.h"
#include "face6323.h"

//static SDK_HANDLE osd_handle;

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/
static  lv_indev_t * kp_indev;

/**********************
*      MACROS
**********************/

/**********************
*   STATIC FUNCTIONS
**********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/


void monitor_init(void)
{
#if 0
	GADI_OSD_OpenParamsT openParamsP;
	SDK_U32 layerIndex;

	memset(&openParamsP, 0 ,sizeof(GADI_OSD_OpenParamsT));
	openParamsP.layerIndex		= 0x01;
	openParamsP.width			= 480;
	openParamsP.height	       = 272;

	gadi_osd_open_layer(&openParamsP, &osd_handle);
	gadi_osd_show_layer(osd_handle);
#endif   

    fb_open();
    fb_blank();
	return;
}

/* Flush the content of the internal buffer the specific area on the display
 * You can use DMA or any hardware acceleration to do this operation in the background but
 * 'lv_flush_ready()' has to be called when finished
 * This function is required only when LV_VDB_SIZE != 0 in lv_conf.h*/
void monitor_flush(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p)
{
	fb_rect_t rect;
    
	//printf("monitor_flush:x1:%d y1:%d x2:%d y2:%d\n",x1,y1,x2,y2);

    rect.xPos = x1;
    rect.yPos = y1;
    rect.height = y2 - y1 + 1;
    rect.width = x2 - x1 + 1;

    fb_lvgl_memory_cpy((SDK_U16*)color_p,&rect,0xf81f);
    
	/* IMPORTANT!!!
	 * Inform the graphics library that you are ready with the flushing*/
	lv_flush_ready();
}


/* Write a pixel array (called 'map') to the a specific area on the display
 * This function is required only when LV_VDB_SIZE == 0 in lv_conf.h*/
void monitor_map(int32_t x1, int32_t y1, int32_t x2, int32_t y2, const lv_color_t * color_p)
{
    fb_rect_t rect;
	//printf("monitor_map:\n");

	int32_t x;
	int32_t y;
	for(y = y1; y <= y2; y++) {
	    for(x = x1; x <= x2; x++) {
			/* Put a pixel to the display. For example: */
			/* put_px(x, y, *color_p)*/
	   		rect.xPos = x;
	   		rect.yPos = y;
	   		rect.height = 1;
	   		rect.width = 1;

   			 fb_lvgl_fill_rectangle(rect, color_p->full);
		  	color_p++;
	    }
	}

}

/* Write a pixel array (called 'map') to the a specific area on the display
 * This function is required only when LV_VDB_SIZE == 0 in lv_conf.h*/
void monitor_fill(int32_t x1, int32_t y1, int32_t x2, int32_t y2,  lv_color_t color)
{
	fb_rect_t rect;
	//printf("monitor_fill:\n");

    rect.xPos = x1;
    rect.yPos = y1;
    rect.height = y2-y1;
    rect.width = x2-x1;
    fb_lvgl_fill_rectangle(rect, color.full);

}

#if USE_LV_GPU
/* If your MCU has hardware accelerator (GPU) then you can use it to blend to memories using opacity
 * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
void ex_mem_blend(lv_color_t * dest, const lv_color_t * src, uint32_t length, lv_opa_t opa)
{
    /*It's an example code which should be done by your GPU*/

    int32_t i;
    for(i = 0; i < length; i++) {
        dest[i] = lv_color_mix(dest[i], src[i], opa);
    }
}

/* If your MCU has hardware accelerator (GPU) then you can use it to fill a memory with a color
 * It can be used only in buffered mode (LV_VDB_SIZE != 0 in lv_conf.h)*/
void ex_mem_fill(lv_color_t * dest, uint32_t length, lv_color_t color)
{
    /*It's an example code which should be done by your GPU*/

    int32_t i;
    for(i = 0; i < length; i++) {
        dest[i] = color;
    }
}
#endif



/**
 * A task to measure the elapsed time for LittlevGL
 * @param data unused
 * @return never return
 */
static int tick_thread(void *data)
{
    printf("tick_thread start\n");
    
	sdk_sys_thread_set_name("Lvgl tick_thread");

    while(1) {
        lv_tick_inc(10);
        gadi_sys_thread_sleep(10);   /*Sleep for 1 millisecond*/
    }

    return 0;
}

/**
 * Initialize the Hardware Abstraction Layer (HAL) for the Littlev graphics library
 */
void hal_init(void)
{
	pthread_t thread_id;

    /* Add a display
     * Use the 'monitor' driver which creates window on PC's monitor to simulate a display*/
    monitor_init();
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);            /*Basic initialization*/
    disp_drv.disp_flush = monitor_flush;
    disp_drv.disp_fill = monitor_fill;
    disp_drv.disp_map = monitor_map;
    lv_disp_drv_register(&disp_drv);

    /* Add  button as input device*/
    /*Create an object group for objects to focus*/
    lv_indev_drv_t	kp_drv;
    lv_indev_drv_init(&kp_drv);
    #if 0
    kp_drv.type = LV_INDEV_TYPE_KEYPAD;
    kp_drv.read = button_key_read;
    kp_indev = lv_indev_drv_register(&kp_drv);
    #endif
    
    /* Tick init.
     * You have to call 'lv_tick_handler()' in every milliseconds
     * Create an SDL thread to do this*/
	pthread_create(&thread_id, NULL, tick_thread, NULL);
}


