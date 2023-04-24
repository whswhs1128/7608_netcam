#ifndef HTTP_LIST_H
#define HTTP_LIST_H


#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)

#define container_of(ptr, type, member) ({			\
	const typeof(((type *)0)->member) * __mptr = (ptr);	\
	(type *)((char *)__mptr - offsetof(type, member)); })

/**
 * list_entry - get the struct for this entry
 * @ptr:	the &struct list_head pointer.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 */
#define list_entry(ptr, type, member) \
	container_of(ptr, type, member)

/**
 * list_first_entry - get the first element from a list
 * @ptr:	the list head to take the element from.
 * @type:	the type of the struct this is embedded in.
 * @member:	the name of the list_struct within the struct.
 *
 * Note, that list is expected to be not empty.
 */
#define list_first_entry(ptr, type, member) \
	list_entry((ptr)->next, type, member)


/**
 * list_for_each	-	iterate over a list
 * @pos:	the &struct list_head to use as a loop cursor.
 * @head:	the head for your list.
 */
#define list_for_each(pos, head) \
	for (pos = (head)->next; pos != (head); pos = pos->next)

/**
	struct for double list
*/
struct list_head
{
	struct list_head *next, *prev;
};


extern inline void INIT_LIST_HEAD(struct list_head *list);
extern inline void __list_add(struct list_head *new,
			      struct list_head *prev,
			      struct list_head *next);

extern inline void list_add_tail(struct list_head *new, struct list_head *head);
extern inline void list_add_head(struct list_head *new, struct list_head *head);
extern inline void __list_del(struct list_head * prev, struct list_head * next);
extern inline void list_del(struct list_head *entry);
extern inline int list_empty(const struct list_head *head);

#endif //HTTP_LIST_H