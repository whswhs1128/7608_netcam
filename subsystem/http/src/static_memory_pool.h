#ifndef __STATIC_MEMORY_POOL_INCLUDE_H
#define __STATIC_MEMORY_POOL_INCLUDE_H

typedef union __memory_obj
{
	union __memory_obj* link;
	char data[1];                 /* The client sees this.        */
}MEMORY_OBJ;

typedef struct __static_mem_pool
{
	int size;
	MEMORY_OBJ* free_list;
	char* chunk;
}STATIC_MEM_POOL;

extern void* mem_allocate(STATIC_MEM_POOL* pool, int n);
extern void mem_deallocate(STATIC_MEM_POOL* pool, void* p);
extern STATIC_MEM_POOL* create_static_mem_pool(int nobj, int size);
extern void free_static_mem_pool(STATIC_MEM_POOL* pool);

#endif //__STATIC_MEMORY_POOL_INCLUDE_H

