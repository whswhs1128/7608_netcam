/*!
*****************************************************************************
** FileName     : g711.h
**
** Description  : config for audio.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-8-23
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_G711_H__
#define _GK_G711_H__

#ifdef __cplusplus
extern "C"
{
#endif

unsigned int audio_alaw_encode(unsigned char *dst, short *src, unsigned int srcSize);
unsigned int audio_alaw_decode(short *dst, const unsigned char *src, unsigned int srcSize);
unsigned int audio_ulaw_encode(unsigned char *dst, short *src, unsigned int srcSize);
unsigned int audio_ulaw_decode(short *dst, const unsigned char *src, unsigned int srcSize);
unsigned int audio_ulaw_to_alaw(unsigned char *dst, const unsigned char *src, unsigned int srcSize);
unsigned int audio_alaw_to_ulaw(unsigned char *dst, const unsigned char *src, unsigned int srcSize);



#ifdef __cplusplus
}
#endif
#endif

