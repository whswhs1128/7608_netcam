#ifndef HTTP_MAIN_H
#define HTTP_MAIN_H

#include "http_class.h"
#include "http_llist.h"
#include "lf_thread_pool.h"


typedef struct __CGI_CALLBACK
{
	char* name;
	void* arg;           //user value
	HANDLE_FUNC hfunc;
	HANDLE_FUNC rfunc;
	int method;

	struct list_head plst;
}CGI_CALLBACK;


extern char* http_server_path;

extern LF_THREAD_POOL* g_read_thrpool;
extern LF_THREAD_POOL* g_process_thrpool;
extern LF_THREAD_POOL* g_write_thrpool;

extern struct lock_list_head g_hpb_mgr;
extern struct list_head g_callback_list;

extern pthread_t thread_run;
//extern pthread_t thread_read_select;


//extern int init_read_select_thread();
extern void handle_read_i(struct lock_list_head* head, fd_set* fdset);
extern int fill_fdset(struct lock_list_head* head, fd_set* fdset);
extern void read_data(struct list_head* entry, void* arg);
//extern void process_data(struct list_head* entry, void *arg);
//extern void write_data(struct list_head* entry, void *arg);
extern void process_data(HTTP_BASE *hpb);
extern void write_data(HTTP_BASE* hpb);
extern int time_wait(int sock, WAIT_EVENT e, int sec, int usec, int times);

#endif //HTTP_MAIN_H