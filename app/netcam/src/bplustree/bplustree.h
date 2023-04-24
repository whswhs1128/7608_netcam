
#ifndef _BPLUS_TREE_H
#define _BPLUS_TREE_H
#include <time.h>

#define _BPLUS_TREE_DEBUG

/* 5 node caches are needed at least for self, left and right sibling, sibling
 * of sibling, parent and node seeking */
#define MIN_CACHE_NUM 5

#define bpt_list_entry(ptr, type, member) \
        ((type *)((char *)(ptr) - (size_t)(&((type *)0)->member)))

#define bpt_list_first_entry(ptr, type, member) \
	bpt_list_entry((ptr)->next, type, member)

#define bpt_list_last_entry(ptr, type, member) \
	bpt_list_entry((ptr)->prev, type, member)

#define bpt_list_for_each(pos, head) \
        for (pos = (head)->next; pos != (head); pos = pos->next)

#define bpt_list_for_each_safe(pos, n, head) \
        for (pos = (head)->next, n = pos->next; pos != (head); \
                pos = n, n = pos->next)
/*每条B+树(日志)索引结构体*/
typedef struct __IPC_BPLUS_TREE_NODE_S_
{
    union
    {
        unsigned int u32Index;          //序号
        unsigned int u32DelIndex;       //删除序号,该结构体用于记录(所有索引/删除索引)相关信息时，该字段表示删除到第几个索引 
    };
    union
    {
        unsigned int u32TotalIndex;     //该结构体用于记录(所有索引/删除索引)相关信息时，该字段表示索引树总共有多少条索引
        unsigned int u32LogType;        //日志类型       
    };
    union
    {
        unsigned int u32CurIndex;       //该结构体用于记录(所有索引/删除索引)相关信息时，该字段表示当前的索引序号    
        unsigned int logTime_msec;      //日志时间毫秒
    };
    unsigned long logTime;              //日志UTC时间
    char   logContent[128];             //日志描述
    unsigned int u32CheckSum;
}IPC_BPLUS_TREE_NODE_S;
typedef int key_t;
typedef IPC_BPLUS_TREE_NODE_S value_t;

typedef struct __LOG_BPLUS_TREE_INFO_S_
{
    unsigned int u32LogDelIndex;       //删除序号,该结构体用于记录(所有索引/删除索引)相关信息时，该字段表示删除到第几个索引 
    unsigned int u32LogTotalIndex;     //该结构体用于记录(所有索引/删除索引)相关信息时，该字段表示索引树总共有多少条索引
    unsigned int u32LogCurIndex;       //该结构体用于记录(所有索引/删除索引)相关信息时，该字段表示当前的索引序号    
    struct bplus_tree *pLogIndexTree;
}LOG_BPLUS_TREE_INFO_S;

struct bpt_list_head {
        struct bpt_list_head *prev, *next;
};

static inline void bpt_list_init(struct bpt_list_head *link)
{
        link->prev = link;
        link->next = link;
}

static inline void
__bpt_list_add(struct bpt_list_head *link, struct bpt_list_head *prev, struct bpt_list_head *next)
{
        link->next = next;
        link->prev = prev;
        next->prev = link;
        prev->next = link;
}

static inline void __bpt_list_del(struct bpt_list_head *prev, struct bpt_list_head *next)
{
        prev->next = next;
        next->prev = prev;
}

static inline void bpt_list_add(struct bpt_list_head *link, struct bpt_list_head *prev)
{
        __bpt_list_add(link, prev, prev->next);
}

static inline void bpt_list_add_tail(struct bpt_list_head *link, struct bpt_list_head *head)
{
	__bpt_list_add(link, head->prev, head);
}

static inline void bpt_list_del(struct bpt_list_head *link)
{
        __bpt_list_del(link->prev, link->next);
        bpt_list_init(link);
}

static inline int bpt_list_empty(const struct bpt_list_head *head)
{
	return head->next == head;
}

typedef struct bplus_node {
        off_t self;
        off_t parent;
        off_t prev;
        off_t next;
        int type;
        /* If leaf node, it specifies  count of entries,
         * if non-leaf node, it specifies count of children(branches) */
        int children;
} bplus_node;

typedef struct free_block {
        struct bpt_list_head link;
        off_t offset;
} free_block;

struct bplus_tree {
        char *caches;
        int used[MIN_CACHE_NUM];
        char filename[1024];
        int bootfd;
        int fd;
        int level;
        off_t root;
        off_t file_size;
        int _block_size;
        int _max_entries;
        int _max_order;	
        struct bpt_list_head free_blocks;
};

struct bplus_tree_config {
        char filename[1024];
        int block_size;
}; 


void bplus_tree_dump(struct bplus_tree *tree);
value_t * bplus_tree_get(struct bplus_tree *tree, key_t key);
int bplus_tree_put(struct bplus_tree *tree, key_t key, value_t *data);
long bplus_tree_get_range(struct bplus_tree *tree, key_t key1, key_t key2);
struct bplus_tree *bplus_tree_init(char *filename, int block_size);
void bplus_tree_deinit(struct bplus_tree *tree);
int bplus_open(char *filename);
void bplus_close(int fd);
int _get_block_size(struct bplus_tree *tree);
void bplus_refresh_boot(struct bplus_tree *tree);



#endif  /* _BPLUS_TREE_H */
