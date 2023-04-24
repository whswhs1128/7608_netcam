/*!
*****************************************************************************
** FileName     : gk_cms_media.h
**
** Description  : media api of cms.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Date         : 2015-9-9, create.
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/


#ifndef _GK_CMS_MEDIA_H_
#define _GK_CMS_MEDIA_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "media_fifo.h"

int GkSendFrame(void *data, int size, GK_NET_FRAME_HEADER *header, int stream_type);

#ifdef __cplusplus
}
#endif

#endif