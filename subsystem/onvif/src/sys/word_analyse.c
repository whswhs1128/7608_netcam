/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif-no-gsoap/src/sys/word_analyse.c
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

#include "word_analyse.h"

static GK_CHAR g_separator[]={' ','\t','\r','\n',',',':','{','}','(',')','\0','\'','"','?','<','>','=',';'};

static GK_BOOL is_num(GK_CHAR ch)
{
	if(ch >= '0' && ch <= '9')
		return GK_TRUE;
	return GK_FALSE;
}


static GK_BOOL is_separator(GK_CHAR ch)
{
	GK_U32 i;
	for(i=0; i<sizeof(g_separator); i++)
	{
		if(g_separator[i] == ch)
			return GK_TRUE;
	}

	return GK_FALSE;
}



GK_BOOL GetSipLine(GK_CHAR *p_buf, GK_S32 max_len, GK_S32 * len, GK_BOOL * bHaveNextLine)
{
	GK_CHAR * ptr_start = p_buf;
	GK_CHAR * ptr_end = ptr_start;
	GK_S32	   line_len;

	*bHaveNextLine = GK_TRUE;

	while((*ptr_end != '\0') &&  (!((*ptr_end == '\r') && (*(ptr_end+1) == '\n'))) && ((ptr_end - ptr_start) < max_len))	ptr_end++;
	while(*(ptr_end-1) == ',')
	{
		while((*ptr_end == '\r') || (*ptr_end == '\n'))
		{
			*ptr_end = ' ';
			ptr_end++;
		}

		while( (*ptr_end != '\r') && (*ptr_end != '\n') &&
			(*ptr_end != '\0') && ((ptr_end - ptr_start) < max_len))
			ptr_end++;
	}

	line_len = ptr_end - ptr_start;

	if((*ptr_end == '\r') && (*(ptr_end+1) == '\n'))
	{
		*ptr_end = '\0';
		*(ptr_end+1) = '\0';
		line_len += 2;
		if(line_len == max_len)
			*bHaveNextLine = GK_FALSE;

		*len = line_len;
		return GK_TRUE;
	}
	else
		return GK_FALSE;
}

GK_BOOL GetLineWord(GK_CHAR *line,GK_S32 cur_word_offset,GK_S32 line_max_len,GK_CHAR *word_buf,GK_S32 buf_len,GK_S32 *next_word_offset,WORD_TYPE w_t)
{
	GK_CHAR *	ptr_start = line+cur_word_offset;
	GK_CHAR *	ptr_end = ptr_start;
	GK_BOOL	bHaveNextWord = GK_TRUE;

	word_buf[0] = '\0';

	while(((*ptr_start == ' ') || (*ptr_start == '\t')) &&
		(cur_word_offset < line_max_len))
	{
		cur_word_offset++;
		ptr_start++;
	}

	if(*ptr_start == '\0')
		return GK_FALSE;

	ptr_end = ptr_start;

	while((!is_separator(*ptr_end)) && ((ptr_end - ptr_start) < line_max_len))
		ptr_end++;

	GK_S32 len = ptr_end - ptr_start;
	if(len >= buf_len)
	{
		word_buf[0] = '\0';
		return bHaveNextWord;
	}

	*next_word_offset = cur_word_offset + len;
	if((*next_word_offset >= line_max_len) || (line[*next_word_offset] == '\0'))
		bHaveNextWord = GK_FALSE;

	switch(w_t)
	{
	case WORD_TYPE_NULL:
		break;

	case WORD_TYPE_STRING:
		{
			if(len == 0 && is_separator(*ptr_end))
			{
				(*next_word_offset)++;
				word_buf[0] = *ptr_end;
				word_buf[1] = '\0';
				return bHaveNextWord;
			}
		}
		break;

	case WORD_TYPE_NUM:
		{
			char * ptr;
			for(ptr=ptr_start; ptr<ptr_end; ptr++)
			{
				if(!is_num(*ptr))
				{
					word_buf[0] = '\0';
					return bHaveNextWord;
				}
			}
		}
		break;

	case WORD_TYPE_SEPARATOR:
		{
			if(is_separator(*ptr_end))
			{
				(*next_word_offset)++;
				word_buf[0] = *ptr_end;
				word_buf[1] = '\0';
				return bHaveNextWord;
			}
		}
		break;
	}

	memcpy(word_buf,ptr_start,len);
	word_buf[len] = '\0';

	return bHaveNextWord;
}
