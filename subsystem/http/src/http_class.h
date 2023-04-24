#ifndef HTTP_CLASS_H
#define HTTP_CLASS_H

#include <time.h>
#include "http_const.h"
#include "http_list.h"
#include "http_export.h"
#include "http_utils.h"


/**
	struct key-value for tag and query string
*/
typedef struct key_value
{
	char* name;
	char* value;
	struct list_head plst;
}QUERY_STRING, HTTP_TAG;

/**
	struct for request sub-path
*/
typedef struct st_name
{
	char* name;
	struct list_head plst;
}STNAME;


/**
	struct for http all information
*/
typedef struct http_base
{
	int fd;
	int filefd;                      //download file fd
	int errcode;
	int fdstatus;                    //fd¦Ì?¡Á¡ä¨¬?
	volatile int isfree;                      //flag for free http_base
	
	int method;
	unsigned short http_major;       //http version
  	unsigned short http_minor;
	
	int req_path_type;               //request type, such as cgi html or other
	int req_body_len;
	int resp_body_len;
	int packet_len;
	size_t file_size;                //download file size
	time_t start_time;               //for keep alive
	time_t expires_time;

	char* request_path;
	char* fragment;                  // GET /test?yyy=222&xxx=111#fragment
	
	char* request_body;              //point to packet
	char* respond_body;
	char* packet;

	HTTP_OPS hp_ops;
	
	struct list_head req_tag_list;    //request tag
	struct list_head resp_tag_list;   //respond tag
	struct list_head query_list;      //query string
	struct list_head rpath_list;      //request sub-path list
	
	struct list_head plst;            //http_base list
}HTTP_BASE;

/**
	function for parser http head
*/
extern int insert_keyof_name(struct list_head* head, const char* str, int len);
extern int insert_keyof_value(struct list_head* head, const char* str, 
	int len, void (*shift)(char*, int));
extern int insert_stname(struct list_head* head, const char* str, int len);
extern int dump_chunk(char** buffer, const char* chunk, int len, int append);
extern void set_request_path_type(HTTP_BASE* hpb, const char* suffix, 
	int len, int type);

/**
	function for set http
*/
extern char* set_http_body(HTTP_BASE* hpb, int len);
extern void add_common_tag(HTTP_BASE* hpb);
extern void add_content_len(HTTP_BASE* hpb, int len);
extern void add_cache_control(HTTP_BASE* hpb, unsigned int time);
extern void add_http_tag(HTTP_BASE* hpb, const char* name, 
	const char* value, int headortail);

/**
	get function for struct http_base 
*/
extern int get_content_length(HTTP_BASE* hpb);
extern inline char* get_http_tag(HTTP_BASE* hpb, const char* name);

/**
	debug function for struct http_base
*/
extern INLINE void print_packet_head(const char* packet, const char* userstr);
extern INLINE void print_http_parse_result(HTTP_BASE* hpb);
extern INLINE void print_respond_tag(HTTP_BASE* hpb);

/**
	function for free http_base
*/
extern void INIT_HTTP_BASE(HTTP_BASE* hpb, int fd);
extern void RESET_HTTP_BASE(HTTP_BASE* hpb);
extern void FREE_HTTP_BASE(HTTP_BASE* hpb);


#endif //HTTP_CLASS_H
