/*!
*****************************************************************************
** \file      $gkprjlist.c
**
** \version	$id: list.c 15-08-04  8月:08:1438655371 
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

#include "mem.h"
#include "list.h"

struct JList {
	JListNode head;
	JListNode tail;
	int nodes;

	JMemPool *mempool;
};

#define j_list_node_remove(node)	do {		\
	node->next->prev = node->prev;				\
	node->prev->next = node->next;				\
} while (0)

JList *j_list_alloc(void)
{
	JList *list = j_mallocz(sizeof(JList));
	if (!list)
		return NULL;

	list->head.next = &list->tail;
	list->tail.prev = &list->head;

	list->mempool = j_mempool_create(sizeof(JListNode), 64, 0);
	if (!list->mempool) {
		j_free(list);
		return NULL;
	}

	return list;
}

void j_list_free(JList *list)
{
	/* FIXME: free list node ? */
	j_list_remove_full(list, NULL, NULL);

	j_mempool_destroy(list->mempool);
	j_free(list);
}

int j_list_length(JList *list)
{
	return list->nodes;
}

int j_list_empty(JList *list)
{
	return !list->nodes;
}

int j_list_append(JList *list, void *data)
{
	JListNode *node = j_mempool_alloc(list->mempool);
	if (!node)
		return -1;

	node->data = data;

	node->next = &list->tail;
	node->prev = list->tail.prev;
	list->tail.prev->next = node;
	list->tail.prev = node;

	list->nodes++;

	return 0;
}

int j_list_prepend(JList *list, void *data)
{
	JListNode *node = j_mempool_alloc(list->mempool);
	if (!node)
		return -1;

	node->data = data;

	node->next = list->head.next;
	node->prev = &list->head;
	list->head.next->prev = node;
	list->head.next = node;

	list->nodes++;

	return 0;
}

JListNode *j_list_find(JList *list, void *data)
{
	JListNode *node;

	for (node = list->head.next; node != &list->tail; node = node->next) {
		if (node->data == data)
			return node;
	}

	return NULL;
}

void *j_list_find_custom(JList *list, JLFindFunc func, void *priv_data)
{
	JListNode *node;

	for (node = list->head.next; node != &list->tail; node = node->next) {
		if (func && func(node->data, priv_data) == 1) {
			return node->data;
		}
	}

	return NULL;
}

int j_list_remove(JList *list, void *data)
{
	JListNode *node = j_list_find(list, data);
	if (!node)
		return -1;

	j_list_node_remove(node);

	j_mempool_release(list->mempool, node);

	list->nodes--;

	return 0;
}

int j_list_remove_full(JList *list, JLFreeFunc func, void *priv_data)
{
	JListNode *node, *node_next;

	for (node = list->head.next; node != &list->tail; node = node_next) {
		node_next = node->next;

		j_list_node_remove(node);

		if (func)
			func(node->data, priv_data);

		j_mempool_release(list->mempool, node);

		list->nodes--;
	}

	return 0;
}

int j_list_foreach(JList *list, JLIterFunc func, void *priv_data)
{
	JListNode *node, *node_next;

	for (node = list->head.next; node != &list->tail; node = node_next) {
		node_next = node->next;
		if (func)
			func(node->data, priv_data);
	}

	return 0;
}

JListNode *j_list_first(JList *list)
{
	if (j_list_empty(list))
		return NULL;

	return list->head.next;
}

JListNode *j_list_last(JList *list)
{
	if (j_list_empty(list))
		return NULL;

	return list->tail.prev;
}

JListNode *j_list_next(JList *list, JListNode *prev)
{
	if (j_list_empty(list))
		return NULL;

	if (prev->next != &list->tail)
		return prev->next;

	return NULL;
}

void *j_list_remove_last(JList *list)
{
	JListNode *node = j_list_last(list);
	if (!node)
		return NULL;

	void *data = node->data;

	j_list_node_remove(node);
	j_mempool_release(list->mempool, node);
	list->nodes--;

	return data;
}

//#define TEST

#ifdef TEST
void display(void *data, void *priv_data)
{
	int val = *(int *)data;
	printf("%d ", val);
}

int main(void)
{
	int arr[] = {0, 1, 2, 3, 4};

	JList *list = j_list_alloc();
	if (!list)
		return -1;

	j_list_append(list, &arr[2]);

	j_list_append(list, &arr[3]);
	j_list_append(list, &arr[4]);

	j_list_prepend(list, &arr[1]);
	j_list_prepend(list, &arr[0]);

	j_list_foreach(list, display, NULL);

	printf("\n");

	j_list_remove(list, &arr[0]);
	j_list_remove(list, &arr[4]);

	j_list_foreach(list, display, NULL);
	printf("\n");

	j_list_remove_full(list, NULL, NULL);

	j_list_foreach(list, display, NULL);
	printf("\n");

	j_list_free(list);

	return 0;
}
#endif
