/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif-no-gsoap/src/sys/word_analyse.h
**
** \brief       word analyse
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/



#ifndef	_WORD_ANALYSE_H_
#define	_WORD_ANALYSE_H_
#include "onvif_std.h"
/***************************************************************************************/
typedef enum word_type
{
	WORD_TYPE_NULL = 0,
	WORD_TYPE_STRING,
	WORD_TYPE_NUM,
	WORD_TYPE_SEPARATOR
}WORD_TYPE;


#ifdef __cplusplus
extern "C" {
#endif

GK_BOOL GetSipLine(GK_CHAR *p_buf, GK_S32 max_len, GK_S32 * len, GK_BOOL * bHaveNextLine);
GK_BOOL GetLineWord(GK_CHAR *line, GK_S32 cur_word_offset, GK_S32 line_max_len, GK_CHAR *word_buf, GK_S32 buf_len, GK_S32 *next_word_offset, WORD_TYPE w_t);

#ifdef __cplusplus
}
#endif

#endif	//	__H_WORD_ANALYSE_H__



