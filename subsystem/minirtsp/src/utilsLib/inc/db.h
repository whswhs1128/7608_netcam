/*!
*****************************************************************************
** \file      $gkprjdb.h
**
** \version	$id: db.h 15-08-04  8月:08:1438655475 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#ifndef __RECORD_DB_H__
#define __RECORD_DB_H__

#include "sqlite3.h"

typedef struct JDb {
	char filename[1024];
	void *db;
	int transactions;	/* transactions refcnt */
} JDb;

typedef int (*JDbFunc)(void*,int,char**, char**);

JDb *j_db_open(const char *filename);
int j_db_close(JDb *jdb);

int j_db_exec(JDb *jdb, const char *sql, JDbFunc cb, void *ctx);

int j_db_table_exist(JDb *jdb, const char *table);

int j_db_get_table(JDb *jdb, const char *sql, char ***result, int *nrows, int *ncols);
int j_db_free_table(char **result);

int j_db_attach(JDb *dest, JDb *src, const char *attach_name);
int j_db_detach(JDb *dest, const char *detach_name);

int j_db_begin(JDb *jdb);
int j_db_commit(JDb *jdb);
int j_db_rollback(JDb *jdb);

int j_db_save(JDb *jdb, const char *filename);
int j_db_load(JDb *jdb, const char *filename);

#endif /* __RECORDLIB_DB_H__ */
