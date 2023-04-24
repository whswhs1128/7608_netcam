/*!
*****************************************************************************
** file        image.c
**
** version     2016-05-12 15:43 heyong -> v1.0
**
** brief       the implementation of image.h.
**
** attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2016-2020 BY GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/
#include "utility_api.h"

unsigned int utility_argb8888_to_uvya8888(unsigned int argb8888)
{
    rgb_format_t rgb;
    yuv_format_t yuv;

    rgb.argb8888 = argb8888;

	yuv.y = (unsigned char)((257 * rgb.r + 504 * rgb.g + 98 * rgb.b + 16000)/1000);
	yuv.u = (unsigned char)((439 * rgb.b - 291 * rgb.g - 148 * rgb.r + 128000)/1000);
	yuv.v = (unsigned char)((439 * rgb.r - 368 * rgb.g - 71 * rgb.b + 128000)/1000);
	yuv.a = 0;
	if (rgb.a) {
		yuv.a = rgb.a | 0x80;
	}
    return ((yuv.a << 24) | (yuv.y << 16) | (yuv.u<< 8) | yuv.v);
}
