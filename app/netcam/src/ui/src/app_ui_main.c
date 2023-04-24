/*!
*****************************************************************************
** \file        gui/trunk/test/src/test_xgui_ui_main.c
**
** \version     $Id: app_ui_main.c 138 2017-12-05 03:28:25Z chenke $
**
** \brief       Application ui
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include <sys/prctl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include "lv_group.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <linux/fb.h>
#include <stdlib.h>
#include "face6323.h"
#include "app_ui_main.h"
#include "jpeglib.h"
#include "jerror.h"


typedef struct
{
	long int msgType;     
	char date[4];
}ui_Send_S;

/*------------------------------- --------------------------------------------*/
/*  Resrouce Config                                                  */
/*---------------------------------------------------------------------------*/
struct jpeg_decompress_struct cinfo;
struct jpeg_error_mgr jerr;
FILE * infile;
static int  uiMsgID;

/*---------------------------------------------------------------------------*/
/* local data                                                                */
/*---------------------------------------------------------------------------*/
int gapp_ui_lvgl_refrsh(void)
{
    int ret = 0;
    ui_Send_S uiEventMsg;
    
    uiEventMsg.msgType = 1;
    
    ret = msgsnd(uiMsgID, (void *)&uiEventMsg, 4, 0);
    if(ret == -1)
    {
        printf("send ui_lvgl message faild.");
        return -1;
    }

    return 0;
}

static int lvgl_thread(void *data)
{
    ui_Send_S uiEventMsg;
	int cnt = 0;
	int ret;
    
    printf("lvgl_thread start\n");

	sdk_sys_thread_set_name("Lvgl");
    
	while(1) {
		/* Periodically call the lv_task handler.
		 * It could be done in a timer interrupt or an OS task too.*/
        if(msgrcv(uiMsgID, &uiEventMsg, 4, 0, IPC_NOWAIT) == -1)
        {
            //TODO: no event occured.
        }
        else
		{
            //handle ui
            lv_task_handler();
		}
	}
	return 0;

}

int gapp_ui_init(void)
{
    int retVal = 0;
	pthread_t thread_id;

    /*Initialize LittlevGL*/
    lv_init();

    printf("lv_init end\n");
    
    /*Initialize the HAL for LittlevGL*/
    hal_init();
    
    printf("hal_init end\n");
    #if 0
    if((uiMsgID = msgget(1235, 0666 | IPC_CREAT)) == -1)
    {
        printf("creat uiMsgID error \n");
        return -1;
    }
    
	pthread_create(&thread_id, NULL, lvgl_thread, NULL);
    #endif
    
    app_ui_face_menu_init();
    app_ui_face_menu_entery();

    return retVal;
}


int jpeg2rgb565(char *jpgfile,unsigned char * fbp)
{
    int i,j;
    //unsigned char * fbp = 0;
    char *rdata;
    //char *Tbmp[]={"/usr/local/jpg/testorig.jpg"};
    char *Tbmp = jpgfile;
    
    //产生相应变量
    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_decompress(&cinfo);  

    //打开图片文件
    if ((infile = fopen(Tbmp, "rb")) == NULL) 
    {
        fprintf(stderr, "can't open %s\n", "tu.jpg");
        return -1;
    }

    //获取头信息
    jpeg_stdio_src(&cinfo, infile);

    jpeg_read_header(&cinfo, 1);

    printf("jpeg info: image_width:%d image_height:%d  %d\n",cinfo.image_width,cinfo.image_height,cinfo.num_components);

    //分配内存存储字节
    rdata=(char*)gadi_sys_malloc(cinfo.image_width*cinfo.image_height*cinfo.num_components);
    //fbp = (unsigned char *)malloc(cinfo.image_width * cinfo.image_height * 2);
    
    //开始解压
    jpeg_start_decompress(&cinfo);

    JSAMPROW row_pointer[1];
    while (cinfo.output_scanline < cinfo.output_height)
    {
       row_pointer[0] = & rdata[(cinfo.output_scanline)*cinfo.image_width*cinfo.num_components];
       jpeg_read_scanlines(&cinfo,row_pointer ,1);
    }

    //结束解压
    jpeg_finish_decompress(&cinfo);

    printf("rgb888->rgb565:\n");
    //framebuffer填充刚才分配的字节

    unsigned short R,G,B,result; 
	fb_rect_t rect;
    
    for(i=0 ;i<cinfo.image_height*cinfo.image_width;i++)   
    {   
        B = ( rdata[3*i+2]>> 3) & 0x001F;   
        G = ((rdata[3*i+1] >> 2) << 5) & 0x07E0;   
        R = ((rdata[3*i] >> 3) << 11) & 0xF800;   
        result =  (R | G | B);   
        fbp[2*i] = result;   
        fbp[2*i+1] = result>>8;   
    }
    
#if 0
    rect.xPos = 0;
    rect.yPos = 0;
    rect.width = cinfo.image_width;
    rect.height = cinfo.image_height;
    fb_lvgl_memory_cpy((unsigned short *)fbp,&rect,0);
#endif 

    fclose(infile);
    jpeg_destroy_decompress(&cinfo);
    gadi_sys_free(rdata);
    
    return 0;
}



int jpeg_test(int argc ,char **argv)
{
    int fd,screen_size,i,j;
    unsigned short value;
    unsigned char * fbp = 0;
    int pos;
    char *rdata;
    char *Tbmp[]={"/usr/local/jpg/testorig.jpg"};
    char  Tpos = 0;
    
    //产生相应变量
    cinfo.err = jpeg_std_error(&jerr);

    jpeg_create_decompress(&cinfo);  

    //打开图片文件
    if ((infile = fopen(Tbmp[Tpos], "rb")) == NULL) 
    {
        fprintf(stderr, "can't open %s\n", "tu.jpg");
        return -1;
    }

    //获取头信息
    jpeg_stdio_src(&cinfo, infile);

    jpeg_read_header(&cinfo, 1);

    printf("jpeg info: image_width:%d image_height:%d  %d\n",cinfo.image_width,cinfo.image_height,cinfo.num_components);

    //分配内存存储字节
    rdata=(char*)malloc(cinfo.image_width*cinfo.image_height*cinfo.num_components);
    fbp = (unsigned char *)malloc(cinfo.image_width * cinfo.image_height * 2);
    
    //开始解压
    jpeg_start_decompress(&cinfo);

    JSAMPROW row_pointer[1];
    while (cinfo.output_scanline < cinfo.output_height)
    {
       row_pointer[0] = & rdata[(cinfo.output_scanline)*cinfo.image_width*cinfo.num_components];
       jpeg_read_scanlines(&cinfo,row_pointer ,1);
    }

    //结束解压
    jpeg_finish_decompress(&cinfo);

    printf("rgb888->rgb565:\n");
    //framebuffer填充刚才分配的字节

    unsigned short R,G,B,result; 
	fb_rect_t rect;
    
    for(i=0 ;i<cinfo.image_height*cinfo.image_width;i++)   
    {   
        B = ( rdata[3*i+2]>> 3) & 0x001F;   
        G = ((rdata[3*i+1] >> 2) << 5) & 0x07E0;   
        R = ((rdata[3*i] >> 3) << 11) & 0xF800;   
        result =  (R | G | B);   
        fbp[2*i] = result;   
        fbp[2*i+1] = result>>8;   
    }   

    rect.xPos = 0;
    rect.yPos = 0;
    rect.width = cinfo.image_width;
    rect.height = cinfo.image_height;
    fb_lvgl_memory_cpy((unsigned short *)fbp,&rect,0);
    
    printf("\n");
    sleep(3);
    fclose(infile);
    jpeg_destroy_decompress(&cinfo);
    close(fd);
    
    return 0;
}

/**********************
 *   lvgl function end
 **********************/
