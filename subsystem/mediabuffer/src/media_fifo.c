/*!
*****************************************************************************
** \file      $gkprjmedia_fifo.c
**
** \version	$id: media_fifo.c 15-08-04  8月:08:1438651416
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
#include <unistd.h>
#include "media_fifo.h"
#include "zfifo.h"
//#include "common.h"

#define PRINT_ERR(fmt,args...) printf("[%s] Line: %d,"fmt" \n",  __FUNCTION__, __LINE__,##args)
#define MB_DEBUG 0
#if MB_DEBUG
#define PRINT_INFO(fmt,args...) printf("Info [%s] Line: %d, "fmt" \n",  __FUNCTION__, __LINE__,##args)
#else
#define PRINT_INFO(fmt,args...)
#endif

static ZFIFO *gk_fifo[VENC_MAX_STREAM_NUM];
static char mediabuf_diable = 0;

//#define ZFIFO_SIZE_0 (2 * 1024 * 1024)
//#define ZFIFO_SIZE_1 (1 * 1024 * 1024)
//#define ZFIFO_SIZE_2 (512 * 1024)
//#define ZFIFO_SIZE_3 (256 * 1024)

int mediabuf_init(int fifo_id, int size)
{
    if (fifo_id < 0 || fifo_id > 3 || size <= 0) {
        PRINT_ERR("param error.");
        return -1;
    }
    if (gk_fifo[fifo_id] != NULL) {
        PRINT_ERR("param error.");
        return -1;
    }

    char name[10] = {0};
    sprintf(name, "stream%d", fifo_id);
    gk_fifo[fifo_id] = zfifo_init(name, size);
    if (gk_fifo[fifo_id] == NULL) {
        PRINT_ERR("zfifo_init %s failed.", name);
        return -1;
    }
	mediabuf_diable = 0;

    return 0;
}


int mediabuf_uninit(int fifo_id)
{
	mediabuf_diable = 1;
    if (fifo_id < 0 || fifo_id > 3) {
        PRINT_ERR("param error.fifo_id:%d\n", fifo_id);
        return -1;
    }
    if (gk_fifo[fifo_id] == NULL) {
        PRINT_ERR("param error. fifo_id:%d\n", fifo_id);
        return -1;
    }

    zfifo_uninit(gk_fifo[fifo_id]);

    return 0;
}

#if 0
int mediabuf_add(int index)
{
    if (index < 0 || index > 3) {
        PRINT_ERR("param error.");
        return -1;
    }
    if (gk_fifo[index] != NULL) {
        PRINT_ERR("param error.");
        return -1;
    }
    int size = 0;

    switch (index) {
        case 0:
            size = ZFIFO_SIZE_0;
            break;
        case 1:
            size = ZFIFO_SIZE_1;
            break;
        case 2:
            size = ZFIFO_SIZE_2;
            break;
        case 3:
            size = ZFIFO_SIZE_3;
            break;
        default:
            break;
    }

    char name[10] = {0};
    sprintf(name, "stream%d", index);
    gk_fifo[index] = zfifo_init(name, size);
    if (gk_fifo[index] == NULL) {
        PRINT_ERR("zfifo_init %s failed.", name);
        return -1;
    }

    return 0;
}

int mediabuf_init(int num)
{
    if (num < 1 || num > 4) {
        PRINT_ERR("param %d error.", num);
        return -1;
    }

    int i, ret;
    for (i = 0; i < VENC_MAX_STREAM_NUM; i ++) {
        gk_fifo[i] = NULL;
    }

    if (num == 4) {
        ret = mediabuf_add(3);
        if (ret < 0)
            return -1;
    }
    if (num >= 3) {
        ret = mediabuf_add(2);
        if (ret < 0)
            return -1;
    }
    if (num >= 2) {
        ret = mediabuf_add(1);
        if (ret < 0)
            return -1;
    }
    if (num >= 1) {
        ret = mediabuf_add(0);
        if (ret < 0)
            return -1;
    }

    return 0;
}

int mediabuf_del(int index)
{
    if (index < 0 || index > 3) {
        PRINT_ERR("param error.");
        return -1;
    }
    if (gk_fifo[index] == NULL) {
        PRINT_ERR("param error.");
        return -1;
    }

    zfifo_uninit(gk_fifo[index]);

    return 0;
}

void mediabuf_uninit()
{
    int i;
    for (i = 0; i < VENC_MAX_STREAM_NUM; i ++) {
        if (gk_fifo[i] != NULL)
            zfifo_uninit(gk_fifo[i]);
    }
}
#endif

MEDIABUF_HANDLE mediabuf_add_reader(GK_NET_STREAM_TYPE type)
{
    ZFIFO * zfifo = gk_fifo[type];

	if(mediabuf_diable)
		return NULL;
	if(zfifo == NULL)
	{
		PRINT_ERR("zfifo is NULL\n");
		return NULL;
	}
    ZFIFO_DESC *id = zfifo_open(zfifo);
    if (id == NULL) {
        PRINT_ERR("zfifo_open stream%d failed.", type);
        return NULL;
    }
    return (MEDIABUF_HANDLE)id;
}

void mediabuf_del_reader(MEDIABUF_HANDLE readerid)
{
    ZFIFO_DESC *zdesc = (ZFIFO_DESC *)readerid;
    if (readerid == NULL) {
        PRINT_ERR("param error.");
        return;
    }
    zfifo_close(zdesc);
}

int mediabuf_read_frame(MEDIABUF_HANDLE readerid, void **data, int *size, GK_NET_FRAME_HEADER *header)
{
    ZFIFO_DESC *zfifo_desc = (ZFIFO_DESC *)readerid;
	int sleep_valu = 2;

    if (mediabuf_diable || header == NULL || data == NULL || readerid == NULL || size == NULL) {
        PRINT_ERR("param error.");
		if(zfifo_desc->timeout >= 1)
			sleep_valu = zfifo_desc->timeout;
		sleep(sleep_valu);
        return -1;
    }

    ZFIFO_NODE node[2];
    node[0].base = header;
    node[0].len = sizeof(GK_NET_FRAME_HEADER);

    if (*data == NULL) {
        node[1].base = zfifo_desc->frame_data;
        node[1].len = zfifo_desc->frame_len;
    } else {
        node[1].base = *data;
        node[1].len = *size;
    }

	int n = 0;
    n = zfifo_readv_plus((ZFIFO_DESC *)readerid, node, 2, 1000);
    if (n < 0) {
        PRINT_ERR("zfifo_readv error.");
		return -1;
    }
    if (n == 0) {
        *size = 0;
        return 0;
    }
    if (n > 0 && header->magic != MAGIC_TEST ) {
        PRINT_ERR("zfifo read magic error.");
		return -1;
    }

    if (*data == NULL) {
        if (node[1].len > zfifo_desc->frame_len) {
			
            PRINT_INFO("%s data buf size %d is too small, so to realloc %d len", zfifo_desc->zfifo->name, zfifo_desc->frame_len, node[1].len);
			if(zfifo_desc->frame_data)
				free(zfifo_desc->frame_data); // free old buffer
			zfifo_desc->frame_len = 0;
			
			zfifo_desc->frame_len = node[1].len;
			zfifo_desc->frame_data = node[1].base;
        }
        *data = zfifo_desc->frame_data;
        *size = node[1].len;
    } else {
        if(node[1].len > *size) {
            PRINT_ERR("Warning:data buf size is too small.it will case deocde picture error ");
            memcpy(*data, node[1].base, *size);
            free(node[1].base);
        } else {
            *size = node[1].len;
        }
    }

    return n;
}

int mediabuf_read_next_frame(MEDIABUF_HANDLE readerid, void **data, int *size, GK_NET_FRAME_HEADER *header)
{
    ZFIFO_DESC *zfifo_desc = (ZFIFO_DESC *)readerid;
	int sleep_valu = 2;
	
    if (mediabuf_diable || header == NULL || data == NULL || readerid == NULL || size == NULL) {
        PRINT_ERR("param error.");
		if(zfifo_desc->timeout >= 1)
			sleep_valu = zfifo_desc->timeout;
		sleep(sleep_valu);		
        return -1;
    }

    ZFIFO_NODE node[2];
    node[0].base = header;
    node[0].len = sizeof(GK_NET_FRAME_HEADER);

    if (*data == NULL) {
        node[1].base = zfifo_desc->frame_data;
        node[1].len = zfifo_desc->frame_len;
    } else {
        node[1].base = *data;
        node[1].len = *size;
    }

	int n = 0;
    n = zfifo_readv_next_plus((ZFIFO_DESC *)readerid, node, 2, 1000);
    if (n < 0) {
        PRINT_ERR("zfifo_readv error.");
        return -1;
    }
    if (n == 0) {
        *size = 0;
        return 0;
    }
    if (n > 0 && header->magic != MAGIC_TEST ) {
        PRINT_ERR("zfifo read magic error.");
        return -1;
    }

    if (*data == NULL) {
        if (node[1].len > zfifo_desc->frame_len) {
			
            PRINT_INFO("%s data buf size %d is too small, so to realloc %d len", zfifo_desc->zfifo->name, zfifo_desc->frame_len, node[1].len);
			if(zfifo_desc->frame_data)
				free(zfifo_desc->frame_data); // free old buffer
			zfifo_desc->frame_len = 0;
			
			zfifo_desc->frame_len = node[1].len;
			zfifo_desc->frame_data = node[1].base;
        }
        *data = zfifo_desc->frame_data;
        *size = node[1].len;
    } else {
        if(node[1].len > *size) {
            PRINT_ERR("Warning:data buf size is too small.it will case deocde picture error ");
            memcpy(*data, node[1].base, *size);
            free(node[1].base);
        } else {
            *size = node[1].len;
        }
    }

    return n;
}


int mediabuf_read_next_I_frame(MEDIABUF_HANDLE readerid,  void **data, int *size, GK_NET_FRAME_HEADER *header)
{
    int old_size = *size;
	time_t t1,t2;
    ZFIFO_DESC *zfifo_desc = (ZFIFO_DESC *)readerid;
	int sleep_valu = 2;
	
    if (mediabuf_diable || header == NULL || data == NULL || readerid == NULL) {
        PRINT_ERR("param error.");
		if(zfifo_desc->timeout >= 1)
			sleep_valu = zfifo_desc->timeout;
		sleep(sleep_valu);		
        return -1;
    }

    int flag = 0;
    if (*data == NULL)
        flag = 1;

    int ret;
	t1 = time(NULL);
    do {
        if (flag == 1) {
            *data = NULL;
            *size = 0;
        } else
            *size = old_size;
        ret = mediabuf_read_next_frame(readerid, data, size, header);
        if (ret < 0)
            return -1;
		t2= time(NULL);
    } while (header->frame_type != GK_NET_FRAME_TYPE_I && ((t2-t1) < zfifo_desc->timeout));

    //printf("size1:%d, size2:%d\n", *size, header->frame_size);
    return ret;
}

int mediabuf_read_I_frame(MEDIABUF_HANDLE readerid, void **data, int *size, GK_NET_FRAME_HEADER *header)
{
	int sleep_valu = 2;
    ZFIFO_DESC *zfifo_desc = (ZFIFO_DESC *)readerid;

    if (mediabuf_diable || header == NULL || data == NULL || readerid == NULL || size == NULL) {
        PRINT_ERR("param error.");
		if(zfifo_desc->timeout >= 1)
			sleep_valu = zfifo_desc->timeout;
		sleep(sleep_valu);		
        return -1;
    }

    ZFIFO_NODE node[2];
	
    node[0].base = header;
    node[0].len = sizeof(GK_NET_FRAME_HEADER);

    if (*data == NULL) {
        node[1].base = zfifo_desc->frame_data;
        node[1].len = zfifo_desc->frame_len;
    } else {
        node[1].base = *data;
        node[1].len = *size;
    }

    int n = 0;
	
	time_t t1,t2;
	t2 = t1 = time(NULL);
    while ((t2 - t1) < zfifo_desc->timeout) {
        n = zfifo_readv_flag_plus((ZFIFO_DESC *)readerid, node, 2, 1000);
        if (n < 0) {
            PRINT_ERR("zfifo_readv error.");
            return -1;
        }
        if (n == 0) {
            usleep(100 * 1000);
            continue;
        }
        if (n > 0 && header->magic != MAGIC_TEST ) {
            PRINT_ERR("zfifo read magic error.");
            return -1;
        } else {
            PRINT_INFO("read I frame\n");
            break;
        }

    }

	if (*data == NULL) {
		if (node[1].len > zfifo_desc->frame_len) {
		
		    PRINT_INFO("%s data buf size %d is too small, so to realloc %d len", zfifo_desc->zfifo->name, zfifo_desc->frame_len, node[1].len);
			if(zfifo_desc->frame_data)
				free(zfifo_desc->frame_data); // free old buffer
			zfifo_desc->frame_len = 0;
			
			zfifo_desc->frame_len = node[1].len;
			zfifo_desc->frame_data = node[1].base;
		}
		*data = zfifo_desc->frame_data;
		*size = node[1].len;
	} else {
		if(node[1].len > *size) {
		    PRINT_ERR("Warning:data buf size is too small.it will case deocde picture error ");
		    memcpy(*data, node[1].base, *size);
		    free(node[1].base);
		} else {
		    *size = node[1].len;
		}
	}

    return n;
}



MEDIABUF_HANDLE mediabuf_add_writer(GK_NET_STREAM_TYPE type)
{
    ZFIFO *zfifo = gk_fifo[type];
	
	if(mediabuf_diable || zfifo == NULL)
	{
		PRINT_ERR("zfifo is NULL\n");
		return NULL;
	}	
    ZFIFO_DESC *id = zfifo_open(zfifo);
    if (id == NULL) {
        PRINT_ERR("zfifo_open stream%d failed.", type);
        return NULL;
    }

    return (MEDIABUF_HANDLE)id;
}

void mediabuf_del_writer(MEDIABUF_HANDLE writerid)
{
    ZFIFO_DESC *zdesc = (ZFIFO_DESC *)writerid;
    if (writerid == NULL) {
        PRINT_ERR("param error.");
        return;
    }

    zfifo_close(zdesc);
}

int mediabuf_write_frame(MEDIABUF_HANDLE writerid, void *data, int size, GK_NET_FRAME_HEADER *header)
{
	if(mediabuf_diable)
		return -1;
    if (header == NULL || data == NULL || writerid == NULL) {
        PRINT_ERR("param error.head:%p,data:%p,writeid:%p",header,data,writerid);
		
        return -1;
    }
    ZFIFO_NODE node[2];
    node[0].base = header;
    node[0].len = sizeof(GK_NET_FRAME_HEADER);
    node[1].base = data;
    node[1].len = size;
	/* check zfifo */
    header->magic = MAGIC_TEST;

    int ret;
    #if 0
    ret = zfifo_writev((ZFIFO_DESC *)writerid, node, 2);
    if (ret < 0) {
        PRINT_ERR("zfifo_writev error.");
        exit(-1);
    }
    #else
    if (header->frame_type == GK_NET_FRAME_TYPE_I) {
        ret = zfifo_writev_plus((ZFIFO_DESC *)writerid, node, 2, 1);
        if (ret < 0) {
            PRINT_ERR("zfifo_writev error.");
            return -1;
        }
    } else {
        ret = zfifo_writev_plus((ZFIFO_DESC *)writerid, node, 2, 0);
        if (ret < 0) {
            PRINT_ERR("zfifo_writev error.");
            return -1;
        }
    }
    #endif
    return ret;
}

int mediabuf_set_newest_frame(MEDIABUF_HANDLE readerid)
{
    if (mediabuf_diable || readerid == NULL) {
        PRINT_ERR("param error.");
        return -1;
    }

    zfifo_set_newest_frame((ZFIFO_DESC *)readerid);

    return 0;
}

int mediabuf_set_oldest_frame(MEDIABUF_HANDLE readerid)
{
    if (mediabuf_diable || readerid == NULL) {
        PRINT_ERR("param error.");
        return -1;
    }

    zfifo_set_oldest_frame((ZFIFO_DESC *)readerid);

    return 0;
}

int mediabuf_toal_size(MEDIABUF_HANDLE w_r_id)
{
	ZFIFO *zfifo = ((ZFIFO_DESC *)w_r_id)->zfifo;

	if(mediabuf_diable || zfifo == NULL)
	{
		PRINT_ERR("zfifo is NULL\n");
		return -1;
	}

    return ((ZFIFO_DESC *)w_r_id)->zfifo->buf_size;
}

int mediabuf_get_leave_frame(MEDIABUF_HANDLE handle)
{
    if (mediabuf_diable || handle == NULL) {
        PRINT_ERR("Error: param error.\n");
        return -1;
    }

    return zfifo_get_leave_frame((ZFIFO_DESC *)handle);
}

int mediabuf_set_I_frame_timeout(MEDIABUF_HANDLE reader,unsigned int timeout_sec)
{
	ZFIFO_DESC *zfifo_desc = (ZFIFO_DESC *)reader;

	if (mediabuf_diable || reader == NULL) {
        PRINT_ERR("Error: param error.\n");
        return -1;
    }

   zfifo_desc->timeout = timeout_sec;
   return 0;
}


