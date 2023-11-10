/*!
*****************************************************************************
** FileName     : ds_search.h
**
** Description  : search for ds.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2016-9-29
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_DS_SEARCH_H__
#define _GK_DS_SEARCH_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define MTWS_SEARCH_SRC_PORT 	(3001)
#define MTWS_SEARCH_DEST_PORT 	(3002)
#define MTWS_SEARCH_DEST_ADDR "255.255.255.255"

int ds_search_start();
void ds_search_stop();


#ifdef __cplusplus
}
#endif
#endif

