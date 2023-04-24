/*!
*****************************************************************************
** \file      $gkprjlist.h
**
** \version	$id: list.h 15-08-04  8月:08:1438655508 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#ifndef __LIST_H__
#define __LIST_H__

#include "mempool.h"

typedef struct JList JList;

typedef struct JListNode {
	void             *data;
	struct JListNode *prev;
	struct JListNode *next;
} JListNode;

typedef int  (*JLFindFunc)(void *data, void *priv_data);
typedef void (*JLFreeFunc)(void *data, void *priv_data);
typedef void (*JLIterFunc)(void *data, void *priv_data);

JList *j_list_alloc(void);
void j_list_free(JList *list);

int j_list_length(JList *list);
int j_list_empty(JList *list);

int j_list_append(JList *list, void *data);
int j_list_prepend(JList *list, void *data);

JListNode *j_list_find(JList *list, void *data);
void *j_list_find_custom(JList *list, JLFindFunc func, void *priv_data);

int j_list_remove(JList *list, void *data);
int j_list_remove_full(JList *list, JLFreeFunc func, void *priv_data);
void *j_list_remove_last(JList *list);

int j_list_foreach(JList *list, JLIterFunc func, void *priv_data);

JListNode *j_list_first(JList *list);
JListNode *j_list_last(JList *list);
JListNode *j_list_next(JList *list, JListNode *prev);

#endif /* __LIST_H__ */
