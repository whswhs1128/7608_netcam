/*!
*****************************************************************************
** \file      $gkprjdb.c
**
** \version	$id: db.c 15-08-04  8月:08:1438655329 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "mem.h"
#include "db.h"

//#define DEBUG

JDb *j_db_open(const char *filename)
{
	JDb *jdb = j_mallocz(sizeof(JDb));
	if (!jdb)
		return NULL;

	if (sqlite3_open(filename, (sqlite3 **)&jdb->db) != SQLITE_OK) {
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(jdb->db));
		sqlite3_close(jdb->db);
		return NULL;
	}

	strncpy(jdb->filename, filename, sizeof(jdb->filename) - 1);

	return jdb;
}

int j_db_close(JDb *jdb)
{
	sqlite3_close(jdb->db);
	j_free(jdb);
	return 0;
}

int j_db_exec(JDb *jdb, const char *sql, JDbFunc cb, void *ctx)
{
#ifdef DEBUG
	printf("%s exec sql: %s\n", sqlite3_db_filename(jdb->db, NULL), sql);
#endif

#if 1
	char *err;
	if (sqlite3_exec(jdb->db, sql, cb, ctx, &err) != SQLITE_OK) {
		if (err != NULL) {
			printf("%s exec sql: %s\n", sqlite3_db_filename(jdb->db, NULL), sql);
			fprintf(stderr, "\033[1;32m%s.%d: %s, SQL error: %s\033[0m\n", __func__, __LINE__,
					sqlite3_db_filename(jdb->db, NULL), err);
			sqlite3_free(err);
		}
		return -1;
	}
#else
	if (sqlite3_exec(jdb->db, sql, cb, ctx, NULL) != SQLITE_OK) {
		fprintf(stderr, "\033[1;32m%s.%d: exec sql '%s' failed\033[0m\n", __func__, __LINE__,
				sqlite3_db_filename(jdb->db, NULL));
		printf("%s exec sql: %s\n", sqlite3_db_filename(jdb->db, NULL), sql);
		return -1;
	}
#endif

	return 0;
}

int j_db_get_table(JDb *jdb, const char *sql, char ***result, int *nrows, int *ncols)
{
#ifdef DEBUG
	printf("%s get_table sql: %s\n", sqlite3_db_filename(jdb->db, NULL), sql);
#endif

	char *err;
	if (sqlite3_get_table(jdb->db, sql, result, nrows, ncols, &err) != SQLITE_OK) {
		if (err != NULL) {
			fprintf(stderr, "\033[1;32m%s.%d: %s, SQL error: %s\033[0m\n", __func__, __LINE__,
					sqlite3_db_filename(jdb->db, NULL), err);
			sqlite3_free(err);
		}
		return -1;
	}
	return 0;
}

int j_db_begin(JDb *jdb)
{
	if (__sync_fetch_and_add(&jdb->transactions, 1) != 0)
		return 0;
	return j_db_exec(jdb, "BEGIN", NULL, NULL);
}

int j_db_commit(JDb *jdb)
{
	if (__sync_sub_and_fetch(&jdb->transactions, 1) != 0)
		return 0;
	return j_db_exec(jdb, "COMMIT", NULL, NULL);
}

int j_db_rollback(JDb *jdb)
{
	return j_db_exec(jdb, "ROLLBACK", NULL, NULL);
}

int j_db_free_table(char **result)
{
	sqlite3_free_table(result);
	return 0;
}

int j_db_attach(JDb *dest, JDb *src, const char *attach_name)
{
	const char *filename = sqlite3_db_filename(src->db, NULL);
	if (!filename)
		return -1;

	char sql[1024];
	snprintf(sql, sizeof(sql), "ATTACH DATABASE '%s' AS %s", filename, attach_name);

	return j_db_exec(dest, sql, NULL, NULL);
}

int j_db_detach(JDb *dest, const char *detach_name)
{
	char sql[1024];
	snprintf(sql, sizeof(sql), "DETACH DATABASE %s", detach_name);
	return j_db_exec(dest, sql, NULL, NULL);
}

int j_db_table_exist(JDb *jdb, const char *table)
{
	int nrows, ncols;
	int exist;
	char **result;
	char sql[256];

	snprintf(sql, 256, "SELECT count(*) FROM sqlite_master WHERE type='table' AND name='%s'", table);

	if (j_db_get_table(jdb, sql, &result, &nrows, &ncols) < 0)
		return 0;

	exist = atoi(result[1]);
	j_db_free_table(result);

	return exist;
}

/*
 * This function is used to load the contents of a database file on disk
 * into the "main" database of open database connection pInMemory, or
 * to save the current contents of the database opened by pInMemory into
 * a database file on disk. pInMemory is probably an in-memory database,
 * but this function will also work fine if it is not.
 *
 * Parameter zFilename points to a nul-terminated string containing the
 * name of the database file on disk to load from or save to. If parameter
 * isSave is non-zero, then the contents of the file zFilename are
 * overwritten with the contents of the database opened by pInMemory. If
 * parameter isSave is zero, then the contents of the database opened by
 * pInMemory are replaced by data loaded from the file zFilename.
 *
 * If the operation is successful, SQLITE_OK is returned. Otherwise, if
 * an error occurs, an SQLite error code is returned.
 */
static int loadOrSaveDb(JDb *pInMemory, const char *zFilename, int isSave){
	JDb *pFile;           /* Database connection opened on zFilename */
	sqlite3_backup *pBackup;  /* Backup object used to copy data */
	JDb *pTo;             /* Database to copy to (pFile or pInMemory) */
	JDb *pFrom;           /* Database to copy from (pFile or pInMemory) */

	/* Open the database file identified by zFilename. Exit early if this fails
	 * for any reason. */
	pFile = j_db_open(zFilename);
	if (pFile) {

		/* If this is a 'load' operation (isSave==0), then data is copied
		 * from the database file just opened to database pInMemory.
		 * Otherwise, if this is a 'save' operation (isSave==1), then data
		 * is copied from pInMemory to pFile.  Set the variables pFrom and
		 * pTo accordingly. */
		pFrom = (isSave ? pInMemory : pFile);
		pTo   = (isSave ? pFile     : pInMemory);

		/* Set up the backup procedure to copy from the "main" database of
		 * connection pFile to the main database of connection pInMemory.
		 * If something goes wrong, pBackup will be set to NULL and an error
		 * code and  message left in connection pTo.
		 *
		 * If the backup object is successfully created, call backup_step()
		 * to copy data from pFile to pInMemory. Then call backup_finish()
		 * to release resources associated with the pBackup object.  If an
		 * error occurred, then  an error code and message will be left in
		 * connection pTo. If no error occurred, then the error code belonging
		 * to pTo is set to SQLITE_OK.
		 */
		pBackup = sqlite3_backup_init(pTo->db, "main", pFrom->db, "main");
		if (pBackup) {
			(void)sqlite3_backup_step(pBackup, -1);
			(void)sqlite3_backup_finish(pBackup);
		}
	}

	/* Close the database connection opened on database file zFilename
	 * and return the result of this function. */
	(void)j_db_close(pFile);
	return 0;
}

int j_db_save(JDb *jdb, const char *filename)
{
	if (loadOrSaveDb(jdb, filename, 1) != SQLITE_OK)
		return -1;
	return 0;
}

int j_db_load(JDb *jdb, const char *filename)
{
	if (loadOrSaveDb(jdb, filename, 0) != SQLITE_OK)
		return -1;
	return 0;
}
