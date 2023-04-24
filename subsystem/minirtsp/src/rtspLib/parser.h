/*!
*****************************************************************************
** \file      $gkprjparser.h
**
** \version	$id: parser.h 15-08-04  8月:08:1438655240 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/

#ifndef __PARSER_H__
#define __PARSER_H__

#include "rtspServ.h"

int rtsp_get_media_type(char *req);
int rtsp_parse_request(rtsp_session_t *sessp);
RtspRoute *rtsp_get_route(const char *route);

#endif /* __PARSER_H__ */
