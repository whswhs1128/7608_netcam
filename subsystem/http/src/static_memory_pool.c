#include <stdlib.h>

#include "http_const.h"
#include "static_memory_pool.h"

void* mem_allocate(STATIC_MEM_POOL* pool, int n)
{
	void* ret = 0;
	MEMORY_OBJ* __result;

	if (n > pool->size)
		goto out;

	__result = pool->free_list;
	if (!__result)
		goto out;

	pool->free_list = __result->link;
	ret = __result;

out:
	return ret;     
}


void mem_deallocate(STATIC_MEM_POOL* pool, void* p)
{
	MEMORY_OBJ* q;
	if (!p)
		return ;

	q = (MEMORY_OBJ*)p;

	q->link= pool->free_list;
	pool->free_list = q;
}


STATIC_MEM_POOL* create_static_mem_pool(int nobj, int size)
{
	int i;
	MEMORY_OBJ* current_obj, *next_obj;
	STATIC_MEM_POOL* pool = 0;

	pool = (STATIC_MEM_POOL*)malloc(sizeof(STATIC_MEM_POOL));
	if (!pool)
		goto error1;

	pool->chunk = (char*)malloc(nobj * size);
	if (!pool->chunk)
		goto error2;

	pool->size = size;
	pool->free_list = (MEMORY_OBJ*)pool->chunk;
	next_obj = pool->free_list;
	for (i = 0; i < nobj; i++)
	{
		current_obj = next_obj;
		next_obj = (MEMORY_OBJ*)((char*)next_obj + size);

		if (i == nobj - 1)
		{
			current_obj->link = 0;
			break;
		}
		
		current_obj->link = next_obj;
	}

	return pool;

error2: 
	FREE_SPACE(pool);
error1:
	return 0;
}

void free_static_mem_pool(STATIC_MEM_POOL* pool)
{
	FREE_SPACE(pool->chunk);
	FREE_SPACE(pool);
}



