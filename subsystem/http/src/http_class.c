#include <memory.h>
#include <unistd.h>

#include "http_class.h"

/**
	function for parser http head
*/
int insert_keyof_name(struct list_head* head, const char* str, int len)
{
	struct key_value* kv = (struct key_value*)malloc(sizeof(struct key_value));
	if (!kv)
		PRINT_ERROR_RETURN(("Fail to alloc space for kv\n"), -1);

	memset(kv, 0, sizeof(struct key_value));
	if (dump_chunk(&kv->name, str, len, 1))
	{
		FREE_SPACE(kv);
		return -1;
	}

	list_add_tail(&kv->plst, head);
	return 0;
}

int insert_keyof_value(struct list_head* head, const char* str, 
	int len, void (*shift)(char*, int))
{
	struct key_value* kv;

	if (list_empty(head))
		return 0;
	
	kv = list_entry(head->prev, struct key_value, plst);
	if (!kv) return -1;

	if (dump_chunk(&kv->value, str, len, 1))
	{
		list_del(&kv->plst);
		FREE_SPACE(kv);
		return -1;
	}

	if (shift)
		shift(kv->value, len);

	return 0;
}

int insert_stname(struct list_head* head, const char* str, int len)
{
	struct st_name* st = (struct st_name*)malloc(sizeof(struct st_name));
	if (!st)
		PRINT_ERROR_RETURN(("Fail to alloc space for st\n"), -1);

	memset(st, 0, sizeof(struct st_name));
	if (dump_chunk(&st->name, str, len, 1))
	{
		FREE_SPACE(st);
		return -1;
	}
	
	list_add_tail(&st->plst, head);
	return 0;
}

void set_request_path_type(HTTP_BASE* hpb, const char* suffix, 
	int len, int type)
{
	int i;

	//set default type
	if (type > 0)
	{
		hpb->req_path_type = type;
		return;
	}

	if (hpb->method != HTTP_GET)
	{
		hpb->req_path_type = REQUEST_PATH_TYPE_CGI;
		return;
	}

	//no dot
	if (!suffix)
	{
		hpb->req_path_type = REQUEST_PATH_TYPE_CGI;
		return;
	}

	for (i = 0; file_suffix[i]; i++)
	{
		if (strncasecmp(suffix, file_suffix[i], len) == 0)
		{
			hpb->req_path_type = i;
			break;
		}
	}
}


/**
	get function for struct http_base 
*/
static HTTP_TAG* get_tag(struct list_head* head, const char* name)
{
	HTTP_TAG* tag;
	struct list_head* pos;

	list_for_each(pos, head)
	{
		tag = list_entry(pos, HTTP_TAG, plst);
		if (strcasecmp(tag->name, name) == 0)
			return tag;
	}

	return NULL;
}

char* get_http_tag(HTTP_BASE* hpb, const char* name)
{
	HTTP_TAG* tag = get_tag(&hpb->req_tag_list, name);
	return tag ? tag->value : NULL;
}

int get_content_length(HTTP_BASE* hpb)
{
	char* cnt_len;

	cnt_len = get_http_tag(hpb, "Content-Length");
	if (!cnt_len)
		return 0;

	return atoi(cnt_len);
}

/**
	set function for struct http_base 
*/
char* set_http_body(HTTP_BASE* hpb, int len)
{
	if (len < 0)
		return NULL;

	if (len == 0)
	{
		hpb->resp_body_len = 0;
		FREE_SPACE(hpb->respond_body);
		return NULL;
	}
	
	hpb->respond_body = (char*)malloc(len);
	if (!hpb->respond_body)
		return NULL;

	hpb->resp_body_len = len;
	memset(hpb->respond_body, 0, len);
	return hpb->respond_body;
}

void add_http_tag(HTTP_BASE* hpb, const char* name, 
	const char* value, int headortail)
{
	HTTP_TAG* tag;

	//print_respond_tag(hpb);
	tag = get_tag(&hpb->resp_tag_list, name);
	if (tag)
	{
		FREE_SPACE(tag->value);
		if (dump_chunk(&tag->value, value, strlen(value), 1))
		{
			list_del(&tag->plst);
			goto out2;
		}
		
		goto out;
	}
	
	tag = (HTTP_TAG*)malloc(sizeof(HTTP_TAG));
	if (!tag)
		goto out;

	if (dump_chunk(&tag->name, name, strlen(name), 1))
		goto out1;
	
	if (dump_chunk(&tag->value, value, strlen(value), 1))
		goto out2;

	if (headortail)
		list_add_head(&tag->plst, &hpb->resp_tag_list);
	else
		list_add_tail(&tag->plst, &hpb->resp_tag_list);

out:
	return ;

out2:
	FREE_SPACE(tag->name);

out1:
	FREE_SPACE(tag);
}

void add_cache_control(HTTP_BASE* hpb, unsigned int time)
{
	char buf[64];
	//char* cache_ctrl = get_http_tag(hpb, "Cache-Control");
	
	/************** Add by Wang peng, please Teng xiaozhou check ************/	
	if (hpb->req_path_type == REQUEST_PATH_TYPE_CGI || time == 0)
		add_http_tag(hpb, "Cache-Control", "no-cache", 0);
	/************** Add by Wang peng, please Teng xiaozhou check ************/	

	//else if (strcasecmp(cache_ctrl, "no-cache") == 0)
	//	add_http_tag(hpb, "Cache-Control", "no-cache", 0);
	else
	{
		memset(buf, 0, sizeof(buf));
		snprintf(buf, sizeof(buf), "max-age=%u", time);
		add_http_tag(hpb, "Cache-Control", buf, 0);
	}

/*
request: 
| "no-cache"
| "no-store"
| "max-age" "=" delta-seconds
| "max-stale" [ "=" delta-seconds ]
| "min-fresh" "=" delta-seconds
| "no-transform"
| "only-if-cached"
| "cache-extension"

response:
| "public"
| "private" [ "=" <"> field-name <"> ]
| "no-cache" [ "=" <"> field-name <"> ]
| "no-store"
| "no-transform"
| "must-revalidate"
| "proxy-revalidate"
| "max-age" "=" delta-seconds
| "s-maxage" "=" delta-seconds
| "cache-extension"
*/
}

static void add_keepalive(HTTP_BASE* hpb)
{
	char* conn_type;
	char* keey_alive;
	char buf[64] = {0};

	conn_type = get_http_tag(hpb, "Connection");
	if (conn_type && strcasecmp(conn_type, "Keep-Alive") == 0)
	{
		add_http_tag(hpb, "Connection", "Keep-Alive", 0);
		hpb->start_time= time(NULL);

		keey_alive = get_http_tag(hpb, "Keep-Alive");
		if (keey_alive)
			hpb->expires_time = atoi(keey_alive);

		//no Keep-Alive, expires time is MIN_EXPIRE_TIME
		if (hpb->expires_time < MIN_EXPIRE_TIME)
			hpb->expires_time = MIN_EXPIRE_TIME;

		if (hpb->expires_time > MAX_EXPIRE_TIME)
			hpb->expires_time = MAX_EXPIRE_TIME;

		if (keey_alive)
		{
			memset(buf, 0, sizeof(buf));
			snprintf(buf, sizeof(buf), "timeout=%d", (int)hpb->expires_time);
			add_http_tag(hpb, "Keep-Alive", buf, 0);
		}
	}
	else
	{
		add_http_tag(hpb, "Connection", "close", 0);

		//for time(NULL)-start_time > expires  to close socket
		hpb->start_time = 1;
		hpb->expires_time = MIN_EXPIRE_TIME;
	}
}


void add_common_tag(HTTP_BASE* hpb)
{
	struct tm utc_tm;
	time_t lt;
	char buf[64];

	add_keepalive(hpb);
	add_http_tag(hpb, "Content-Type", content_type[hpb->req_path_type], 1);

	lt = time(NULL);
	gmtime_r(&lt, &utc_tm);
	memset(buf, 0, sizeof(buf));
	strftime(buf, sizeof(buf), "%a, %d %b %Y %T GMT", &utc_tm);
	add_http_tag(hpb, "Date", buf, 1);

	add_http_tag(hpb, "Server", "IPC GK7101 HttpServer 1.0", 1);
}

void add_content_len(HTTP_BASE* hpb, int len)
{
	char buf[64];
	
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "%d", len);
	add_http_tag(hpb, "Content-Length", buf, 0);
}

/**
	debug function for struct http_base
*/
void print_packet_head(const char* packet, const char* userstr)
{
#ifdef DEBUG_MACRO
	int len;
	char* p;

	if (!packet)
		return;
	
	p = strstr(packet, "\r\n\r\n");
	if (!p)
	{
		printf("\n[%s:\n%s\n]\n\n", userstr, packet);
		return;
	}

	len = p - packet;
	p = NULL;
	p = (char*)alloca(len + 1);
	if (!p)
	{
		printf("\n[%s:\n%s\n]\n\n", userstr, packet);
		return;
	}
	
	memset(p, 0, len + 1);
	memcpy(p, packet, len);
	printf("\n[%s:\n%s\n]\n\n", userstr, p);
#endif //DEBUG_MACRO
}

void print_http_parse_result(HTTP_BASE* hpb)
{
#ifdef DEBUG_MACRO
	struct list_head* pos;
	QUERY_STRING* qstring;
	HTTP_TAG* tag;
	STNAME* st;

	printf("\n{http parse result:\n");
	printf("fd[%d] errcode[%d] version[%d.%d] method[%d]\n",
		hpb->fd, hpb->errcode, hpb->http_major, hpb->http_minor, hpb->method);
	
	printf("request_path_type[%d] packet_len[%d] body_len[%d]\n", 
		hpb->req_path_type, hpb->packet_len, hpb->req_body_len);

	if (hpb->request_path)
		printf("request_path[%s]\n", hpb->request_path);
	printf("cgi_name[");
	list_for_each(pos, &hpb->rpath_list)
	{
		st = list_entry(pos, STNAME, plst);
		if (st->name)
			printf("(%s) ", st->name);
	}
	printf("]\n");

	if (hpb->fragment)
		printf("fragment[%s]\n", hpb->fragment);

	printf("query_string[");
	list_for_each(pos, &hpb->query_list)
	{
		qstring = list_entry(pos, QUERY_STRING, plst);
		if (qstring->name)
			printf("(%s)=", qstring->name);
		if (qstring->value)
			printf("(%s) ", qstring->value);
	}
	printf("]\n");

	printf("tag[");
	list_for_each(pos, &hpb->req_tag_list)
	{
		tag = list_entry(pos, HTTP_TAG, plst);
		if (tag->name)
			printf("(%s)=", tag->name);
		if (tag->value)
			printf("(%s) ", tag->value);
	}
	printf("]\n");
	printf("}\n\n");


#endif //DEBUG_MACRO
}

void print_respond_tag(HTTP_BASE* hpb)
{
#ifdef DEBUG_MACRO
	struct list_head* pos;
	HTTP_TAG* tag;

	printf("+++++++++++++++++++++++++\n");
	printf("tag[");
	list_for_each(pos, &hpb->resp_tag_list)
	{
		tag = list_entry(pos, HTTP_TAG, plst);
		if (tag->name)
			printf("(%s)=", tag->name);
		if (tag->value)
			printf("(%s) ", tag->value);
	}
	printf("]\n");
	printf("==========================\n");

#endif //DEBUG_MACRO
}


/**
	fuction for cgi call back
*/
static const char* get_tag_i(HTTP_OPS* ops, const char* name)
{
	HTTP_BASE* hpb;

	hpb = container_of(ops, HTTP_BASE, hp_ops);
	return get_http_tag(hpb, name);
}

static const char* get_param_string_i(HTTP_OPS* ops, const char* name)
{
	QUERY_STRING* qstring;
	struct list_head* pos;
	HTTP_BASE* hpb;

	hpb = container_of(ops, HTTP_BASE, hp_ops);
	list_for_each(pos, &hpb->query_list)
	{
		qstring = list_entry(pos, QUERY_STRING, plst);
		if (strcasecmp(qstring->name, name) == 0)
			return qstring->value;
	}

	return NULL;
}

static int get_param_int_i(HTTP_OPS* ops, const char* name)
{
	const char* value;
	value = get_param_string_i(ops, name);

	if (!value) return 0;
	else return atoi(value);
}

static const char* get_fragment_i(HTTP_OPS* ops)
{
	HTTP_BASE* hpb = container_of(ops, HTTP_BASE, hp_ops);
	
	if (!hpb->fragment)
		return NULL;
	return hpb->fragment;
}


static int add_tag_i(HTTP_OPS* ops, const char* name, const char* value)
{

	HTTP_BASE* hpb;
	hpb = container_of(ops, HTTP_BASE, hp_ops);
	add_http_tag(hpb, name, value, 0);
	
	return 0;
}

static const char* get_body_i(HTTP_OPS* ops, int* len)
{
	HTTP_BASE* hpb;

	hpb = container_of(ops, HTTP_BASE, hp_ops);
	if (hpb->req_body_len || hpb->request_body)
	{
		*len = hpb->req_body_len;
		return hpb->request_body;
	}

	*len = 0;
	return NULL;
}

static char* set_body_i(HTTP_OPS* ops, int len)
{
	HTTP_BASE* hpb;

	hpb = container_of(ops, HTTP_BASE, hp_ops);
	if (len <= 0)
		return NULL;
	
	hpb->respond_body = (char*)malloc(len);
	if (!hpb->respond_body)
		return NULL;

	hpb->resp_body_len = len;
	memset(hpb->respond_body, 0, len);
	return hpb->respond_body;
}

static void set_body_ex_i(HTTP_OPS* ops, const char* buf, int len)
{
	HTTP_BASE* hpb;

	hpb = container_of(ops, HTTP_BASE, hp_ops);
	if (!buf || len <= 0)
		return ;
	
	hpb->respond_body = (char*)malloc(len);
	if (!hpb->respond_body)
		return ;

	hpb->resp_body_len = len;
	memset(hpb->respond_body, 0, len);
	memcpy(hpb->respond_body, buf, len);
}

static void set_http_respond_i(HTTP_OPS* ops, const char* buf, int len)
{
	HTTP_BASE* hpb;
	hpb = container_of(ops, HTTP_BASE, hp_ops);

	if (!buf || len <= 0)
		return ;
	hpb->respond_body = (char*)malloc(len);
	if (!hpb->respond_body)
		return ;

	hpb->resp_body_len = len;
	memset(hpb->respond_body, 0, len);
	memcpy(hpb->respond_body, buf, len);
}

static const char* get_http_request_i(HTTP_OPS* ops, int* len)
{
	HTTP_BASE* hpb;
	hpb = container_of(ops, HTTP_BASE, hp_ops);

	*len = hpb->packet_len;
	return hpb->packet;
}

static int get_method_i(HTTP_OPS* ops)
{
	HTTP_BASE* hpb;
	hpb = container_of(ops, HTTP_BASE, hp_ops);

	return 1 << hpb->method;
}

static void disconnect_i(HTTP_OPS* ops)
{
	HTTP_BASE* hpb;
	hpb = container_of(ops, HTTP_BASE, hp_ops);
	hpb->isfree = HPB_REALLY_FREE;
}

static int get_connection_fd_i(HTTP_OPS* ops)
{
	HTTP_BASE* hpb;
	hpb = container_of(ops, HTTP_BASE, hp_ops);
    return hpb->fd;
}


/**
	function for free space of struct http_base
*/
#define FREE_ONE(X) \
	({FREE_SPACE((X)->name);})

#define FREE_TWO(X) \
	({FREE_SPACE((X)->name);FREE_SPACE((X)->value);})

void FREE_HTTP_BASE(HTTP_BASE* hpb)
{
	struct list_head* pos;

	hpb->request_body = NULL;
	FREE_SPACE(hpb->fragment);
	FREE_SPACE(hpb->respond_body);
	FREE_SPACE(hpb->request_path);
	FREE_SPACE(hpb->packet);

	FREE_LIST_HEAD(pos, &hpb->req_tag_list, struct key_value, plst, FREE_TWO);
	FREE_LIST_HEAD(pos, &hpb->resp_tag_list, struct key_value, plst, FREE_TWO);
	FREE_LIST_HEAD(pos, &hpb->query_list, struct key_value, plst, FREE_TWO);
	FREE_LIST_HEAD(pos, &hpb->rpath_list, struct st_name, plst, FREE_ONE);

	PRINT_DEBUG(("Http server will close socket fd[%d], regular file fd[%d]\n", 
		hpb->fd, hpb->filefd));
	
	if (hpb->filefd > 0)
		close(hpb->filefd);

	if (hpb->fd > 0)
		close(hpb->fd);
	FREE_SPACE(hpb);
}

void RESET_HTTP_BASE(HTTP_BASE* hpb)
{
	struct list_head* pos;

	hpb->errcode = HPE_OK;
	hpb->method = 0;
	hpb->req_path_type = REQUEST_PATH_TYPE_CGI;
	hpb->req_body_len = 0;
	hpb->resp_body_len = 0;
	hpb->packet_len = 0;
	hpb->file_size = 0;
	
	hpb->request_body = NULL;
	FREE_SPACE(hpb->fragment);
	FREE_SPACE(hpb->respond_body);
	FREE_SPACE(hpb->request_path);
	FREE_SPACE(hpb->packet);

	FREE_LIST_HEAD(pos, &hpb->req_tag_list, struct key_value, plst, FREE_TWO);
	FREE_LIST_HEAD(pos, &hpb->resp_tag_list, struct key_value, plst, FREE_TWO);
	FREE_LIST_HEAD(pos, &hpb->query_list, struct key_value, plst, FREE_TWO);
	FREE_LIST_HEAD(pos, &hpb->rpath_list, struct st_name, plst, FREE_ONE);

	if (hpb->filefd > 0)
	{
		PRINT_DEBUG(("Http server will close regular file fd[%d]\n", hpb->filefd));
		close(hpb->filefd);
	}
	hpb->filefd = -1;
}

#undef FREE_ONE
#undef FREE_TWO

void INIT_HTTP_BASE(HTTP_BASE* hpb, int fd)
{
	memset(hpb, 0, sizeof(HTTP_BASE));
	
	hpb->fd = fd;
	hpb->filefd = -1;
	hpb->errcode = HPE_OK;
	hpb->fdstatus = CONN_STATUS_ACCEPT;
	
	hpb->hp_ops.get_tag = get_tag_i;
	hpb->hp_ops.get_param_string = get_param_string_i;
	hpb->hp_ops.get_param_int = get_param_int_i;
	hpb->hp_ops.get_fragment = get_fragment_i;
	hpb->hp_ops.get_body = get_body_i;

	hpb->hp_ops.add_tag = add_tag_i;
	hpb->hp_ops.set_body = set_body_i;
	hpb->hp_ops.set_body_ex = set_body_ex_i;
	hpb->hp_ops.set_http_respond = set_http_respond_i;
	hpb->hp_ops.get_http_request = get_http_request_i;
	hpb->hp_ops.get_method = get_method_i;
	hpb->hp_ops.disconnect = disconnect_i;
	hpb->hp_ops.get_connection_fd = get_connection_fd_i;
}


