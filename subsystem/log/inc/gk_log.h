/*!
*****************************************************************************
** FileName     : gk_log.h
**
** Description  : config for log.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-8-28
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_LOG_H__
#define _GK_LOG_H__

#ifdef __cplusplus
extern "C"
{
#endif

#define LOG_DIR "/opt/custom/log/"
#define LOG_FILE "log.txt"

FILE*  log_open();
int log_write(char *msg);

#ifdef __cplusplus
}
#endif
#endif

