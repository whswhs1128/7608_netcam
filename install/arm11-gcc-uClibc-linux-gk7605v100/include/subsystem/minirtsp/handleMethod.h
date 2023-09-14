/*!
*****************************************************************************
** \file      $gkprjhandleMethod.h
**
** \version	$id: handleMethod.h 15-08-04  8月:08:1438655232 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/

#ifndef __HANDLEMETHOD_H__
#define __HANDLEMETHOD_H__

#include "rtspServ.h"

int initRtpHdr(rtp_hdr_t *hdrp, int media_type);

int rtsp_cmd_options(rtsp_session_t *sessp);
int rtsp_cmd_describe(rtsp_session_t *sessp);
int rtsp_cmd_setup(rtsp_session_t *sessp);
int rtsp_cmd_teardown(rtsp_session_t *sessp);
int rtsp_cmd_play(rtsp_session_t *sessp);
int rtsp_cmd_pause(rtsp_session_t *sessp);
int rtsp_cmd_get_parameter(rtsp_session_t *sessp);
int rtsp_cmd_set_parameter(rtsp_session_t *sessp);


#endif /* __HANDLEMETHOD_H__ */
