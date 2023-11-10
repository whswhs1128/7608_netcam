/*!
*****************************************************************************
** FileName     : utility_api.h
**
** Description  : utility api.
**
** Author       : Bruce <zhaoquanfeng@gokemicro.com>
** Create Date  : 2015-9-15
**
** (C) Copyright 2013-2014 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/

#ifndef _GK_UTILITY_API_H__
#define _GK_UTILITY_API_H__

#include "time.h"
#include <pthread.h>

#ifdef __cplusplus
extern "C"
{
#endif

void sys_get_tm(struct tm* t);
int  sys_sleep(int nDelay);
void SetThreadName(const char *name);
int GenRandInt(int min, int max);
unsigned long ip_to_dw(const char *ip_src);
void mac_to_dw(unsigned char *dst, char *mac_src);

void capture_all_signal();

typedef	void *(*ThreadEntryPtrType)(void *);
int CreateDetachThread(ThreadEntryPtrType entry, void *para, pthread_t *pid);
int CreateThread(ThreadEntryPtrType entry, void *para, pthread_t *pid);

typedef struct {
   ThreadEntryPtrType callback;
   void * arg;
   int arg_size;
} ThreadArgs;

int CreateDetachThread2(ThreadEntryPtrType fuc_call, void *arg1, int arg1_size, pthread_t *pid);


/* the table struct of bitmap file header.*/
typedef struct bitmap_file_header_s {
	char type[2]; // "BM" (0x4d42)
    unsigned int file_size;
    unsigned int reserved_zero;
    unsigned int off_bits; // data area offset to the file set (unit. byte)
	unsigned int info_size;
	unsigned int width;
	unsigned int height;
	unsigned short planes; // 0 - 1
	unsigned short bit_count; // 0 - 1
	unsigned int compression; // 0 - 1
	unsigned int size_image; // 0 - 1
	unsigned int xpels_per_meter;
	unsigned int ypels_per_meter;
	unsigned int clr_used;
	unsigned int clr_important;
} __attribute__((packed)) bitmap_file_header_t;

/*colour look-up table struct of RGB domain.*/
typedef union rgb_pixel_s {
    unsigned int  argb8888;
    struct {
    	unsigned char	b;
    	unsigned char	g;
    	unsigned char	r;
    	unsigned char	a;
    };
} rgb_format_t;

/*colour look-up table struct of YUV domain.*/
typedef struct yuv_format_s {
    unsigned char   v;
    unsigned char   u;
    unsigned char   y;
    unsigned char   a;
} yuv_format_t;

/* argb8888 to uvya8888*/
unsigned int utility_argb8888_to_uvya8888(unsigned int argb8888);

/* gdk ת��utf8��lenΪutf8���Ĵ�С������0���ɹ�����0ʧ��*/
int utility_gbk_to_utf8(char *gbk, char *utf8, int len);


/* utf8 ת��gbk��lenΪutf8���Ĵ�С������0���ɹ�����0ʧ��*/
int utility_utf8_to_gbk(char *utf8, char *gbk, int len);


/* ��in_str���봮��base64��ʽ���뵽out_str��lengthΪ���봮���ȣ�����ֵΪ����󳤶�*/
int utility_base64_encode(const void *in_str, void *out_str, int length);

/* ��in_str���봮��base64��ʽ���뵽out_str��lengthΪ���봮���ȣ�����Ϊ4�ֽڶ��룬����ֵ-1Ϊʧ�ܣ�����ֵΪ��������ݳ���*/
int utility_base64_decode(const void *in_str, void *out_str, int length);


/* crc32 ���㣬���������������bufferһ���crc32ֵ��valΪ�ϴμ����crc32ֵ������Ϊ������crc32ֵ*/
unsigned int utility_crc32(unsigned int val, const void *ss, int len);
#ifdef __cplusplus
}
#endif
#endif

