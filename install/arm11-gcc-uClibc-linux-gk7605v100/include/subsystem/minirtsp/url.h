/*!
*****************************************************************************
** \file      $gkprjurl.h
**
** \version	$id: url.h 15-08-04  8月:08:1438655295 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/


#ifndef _URL_PARSER_H
#define _URL_PARSER_H

/*
 * URL storage
 */
typedef struct UrlData {
	char *scheme;               /* mandatory */
	char *host;                 /* mandatory */
	char *port;                 /* optional */
	char *path;                 /* optional */
	char *query;                /* optional */
	char *fragment;             /* optional */
	char *username;             /* optional */
	char *password;             /* optional */
} UrlData;

#ifdef __cplusplus
extern "C" {
#endif

	/*
	 * Declaration of function prototypes
	 */
	UrlData *UrlParse(const char *url);
	void UrlFree(UrlData *ud);

#ifdef __cplusplus
}
#endif

#endif /* _URL_PARSER_H */
