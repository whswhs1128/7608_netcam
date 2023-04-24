/*!
*****************************************************************************
** FileName     : gk_cms_pb.h
**
** Description  : playback api of cms.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Date         : 2015-10-12, create.
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/


#ifndef _GK_CMS_PB_H_
#define _GK_CMS_PB_H_

#ifdef __cplusplus
extern "C"
{
#endif

int Gk_Playback(int sock);
int Gk_CmsPbControl(int sock);

#ifdef __cplusplus
}
#endif

#endif