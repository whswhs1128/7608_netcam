/******************************************************************************
	Class:			netview_event
	Created:		2016/6/24 14:46:00
	Author:			Bao, John
	Copyright (C), 2010-2016, NetView Tech. Co., Ltd.
	This file may not be distributed, copied, or reproduced in any manner,
    electronic or otherwise, without the written consent of NetView.
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>


#include "zbar/zbar.h"
#include "qrcode_data_320_old.h"

#if 0
static unsigned char qrcode_bin[]=
{
    #include "qrcode_data"
};
#endif





zbar_image_scanner_t *scanner = NULL;

static void load_bin(unsigned char *source_bin, unsigned char* addr, int size)
{
    int i = 0;

    for (i=0; i<size; i++)
    {
        *((volatile unsigned char *)addr + i) = *source_bin++;
    }
}

int main(void)
{
   

    // printf(" Goke GK7101 RTOS Application               \n");
     printf(" (c) Goke Microelectronics China 2009 - 2016   \n");
    scanner = zbar_image_scanner_create();

    /* configure the reader */
    zbar_image_scanner_set_config(scanner, 0, ZBAR_CFG_ENABLE, 1);

    /* obtain image data */
    int width = 320, height = 320;
    void *raw = NULL;
#if 1
    raw = malloc(width*height);
    memset(raw, 0, width*height);
    memcpy(raw, qrcode_data, width*height);
    printf("size: %d\n", strlen(raw));
#else
    unsigned char *regDataBuf = NULL;
    int size;

    size = sizeof(qrcode_bin);
    printf("size: %d\n", size);
    regDataBuf = gadi_sys_malloc(size);
    if(regDataBuf == NULL)
    {
        printf("gadi_sys_malloc error,size:%ld\n", size);
    }

    load_bin(qrcode_bin, regDataBuf, size);
#endif

    //get_data(argv[1], width, height, &raw);
    //printf("aaaaaaaaaaaaaaaaaaaaaaaaaa \n");
    /* wrap image data */
    zbar_image_t *image = zbar_image_create();
    zbar_image_set_format(image, *(int*)"Y800");
    zbar_image_set_size(image, width, height);
    zbar_image_set_data(image, raw, width * height, zbar_image_free_data);

    /* scan the image for barcodes */
    int n = zbar_scan_image(scanner, image);
    printf("nsyms size: %d\n", n);

    /* extract results */
    const zbar_symbol_t *symbol = zbar_image_first_symbol(image);
    for(; symbol; symbol = zbar_symbol_next(symbol)) {
        /* do something useful with results */
        zbar_symbol_type_t typ = zbar_symbol_get_type(symbol);
        const char *data = zbar_symbol_get_data(symbol);
        printf("decoded %s symbol \"%s\"\n",
               zbar_get_symbol_name(typ), data);
    }

    /* clean up */
    zbar_image_destroy(image);
    zbar_image_scanner_destroy(scanner);

    return 0;
}
