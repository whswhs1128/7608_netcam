#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>


#include "http_class.h"
#include "http_main.h"

static int _listen_fd;
char* http_server_path = NULL;
int _http_server_init = 0;

LF_THREAD_POOL* g_read_thrpool = NULL;
//LF_THREAD_POOL* g_process_thrpool = NULL;
//LF_THREAD_POOL* g_write_thrpool = NULL;

struct lock_list_head g_hpb_mgr;
struct list_head g_callback_list;

pthread_t thread_run;
//pthread_t thread_read_select;

int time_wait(int sock, WAIT_EVENT e, int sec, int usec, int times)
{
	int ret = 0;
	fd_set fset;
	struct timeval tv;

	while(times > 0)
	{
		FD_ZERO(&fset);
		FD_SET(sock, &fset);

		tv.tv_sec = sec;
		tv.tv_usec = usec;

		if (e == WF_READ)
			ret = select(sock + 1, &fset, NULL, NULL, &tv);
		else
			ret = select(sock + 1, NULL, &fset, NULL, &tv);

		if (ret > 0)
			goto finish;

		if (ret <= 0)
		{
			if (errno != EINTR &&  errno != EAGAIN )
				goto error;       //other error

			times--;               //time out or EINTR
		}
	}

error:
	PRINT_ERROR(("select %s [time_wait]! fd[%d] times[%d] errno[%d] errinfo[%s]\n",
		errno == 0 ? "timeout" : "error", sock,
		times, errno, strerror(errno)));

finish:
	return ret;
}


static int set_nonblocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
	{
		PRINT_ERROR(("Fail to get old flags fd[%d] [set_nonblocking]! errno[%d] errinfo[%s]\n",
			fd, errno, strerror(errno)));
		return -1;
	}
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		PRINT_ERROR(("Fail to set flags to O_NONBLOCK fd[%d] [set_nonblocking]! errno[%d] errinfo[%s]\n",
			fd, errno, strerror(errno)));
		return -1;
	}

	return 0;
}

static int network_init(const char* s_ip, int port)
{
	int addr_len;
	int opt = 1;
	//unsigned int _i_ip;
	struct sockaddr_in addr;

	_listen_fd = socket(AF_INET, SOCK_STREAM, 0);
	if (_listen_fd == -1)
	{
		PRINT_ERROR(("Fail to create socket file descriptions [network_init]! errno[%d] errinfo[%s]\n",
			errno, strerror(errno)));
		return -1;
	}

	//_i_ip = ntohl(inet_addr(s_ip));

	addr_len = sizeof(struct sockaddr_in);
	bzero(&addr,sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_port = htons(port);
	addr.sin_addr.s_addr = inet_addr(s_ip);

	if (setsockopt(_listen_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		PRINT_ERROR(("Fail to reuse the port[%d] [network_init]! errno[%d] errinfo[%s]\n",
			port, errno, strerror(errno)));
		return -1;
	}
	if (bind(_listen_fd, (struct sockaddr *)&addr, addr_len) == -1)
	{
		PRINT_ERROR(("Fail to bind file descriptions[%d] [%s:%d] [network_init]! errno[%d] errinfo[%s]\n",
			_listen_fd, s_ip, port, errno, strerror(errno)));
		return -1;
	}

	if (listen(_listen_fd, DEFAULT_BACKLOG) == -1)
	{
		PRINT_ERROR(("Fail to listen file descriptions[%d] [%s:%d] [network_init]! errno[%d] errinfo[%s]\n",
			_listen_fd, s_ip, port, errno, strerror(errno)));
		return -1;
	}

	if (set_nonblocking(_listen_fd) == -1)
		return -1;

	return 0;
}

static int accept_connect(int fd)
{
	int new_fd;
	int retry;
	struct sockaddr_in sa;
	unsigned int sa_len;
	sa_len = sizeof(struct sockaddr_in);

	for (retry = 0; retry < MAX_RETRY_TIMES; retry++)
	{
		new_fd = accept(fd, (struct sockaddr *)&sa, (socklen_t *)&sa_len);
		if (new_fd > 0)
			break;
        /*
		PRINT_ERROR(("Fail to accept new_fd[%d] [accept_connect]! errno[%d] errinfo[%s]\n",
			new_fd, errno, strerror(errno)));
		*/	
	}

	if (retry == MAX_RETRY_TIMES)
	{
		PRINT_ERROR(("Accept retry max times [%d] [accept_connect]! errno[%d] errinfo[%s]\n",
			retry, errno, strerror(errno)));
		return -1;
	}

	if (set_nonblocking(new_fd) == -1)
		return -1;

	return new_fd;
}

static void* handle_connections(void *arg)
{
    sdk_sys_thread_set_name("handle_connections");
	int acceptor_fd;
	int max_connection;
	fd_set readFDs;
	struct timeval tv;
	HTTP_BASE* hpb;

	PRINT_INFO(("[ThreadID 0x%x] http connection thread start!\n",
		(unsigned int)pthread_self()));
	while (_http_server_init)
	{
		//ÖØÐÂ¼ÆËãmax fd
		FD_ZERO(&readFDs);
		FD_SET(_listen_fd, &readFDs);
		max_connection = fill_fdset(&g_hpb_mgr, &readFDs);
		max_connection = max_connection < _listen_fd ? _listen_fd : max_connection;
		max_connection += 1;
		
		tv.tv_sec = 0;
		tv.tv_usec = SELECT_TIMEOUT;

		if (select(max_connection, &readFDs, 0, 0, &tv) < 0)
		{
			if (errno == EINTR)
				continue;

			PRINT_ERROR(("select error [handle_connections]! errno[%d] errinfo[%s]\n",
				errno, strerror(errno)));
			goto out;
		}

		if (FD_ISSET(_listen_fd, &readFDs))
		{
			if ((acceptor_fd = accept_connect(_listen_fd)) < 0)
				continue;

			hpb = (HTTP_BASE*)malloc(sizeof(HTTP_BASE));
			if (!hpb)
			{
				PRINT_ERROR(("Fail to malloc space for acceptor[handle_connections]\n"));
				goto out;
			}

			INIT_HTTP_BASE(hpb, acceptor_fd);
			llist_add_tail(&hpb->plst, &g_hpb_mgr);
		}

		handle_read_i(&g_hpb_mgr, &readFDs);
	}

out:
	return 0;
}

int http_mini_server_run()
{
	signal(SIGPIPE, SIG_IGN);
	if (pthread_create(&thread_run, NULL, handle_connections, NULL) < 0)
	{
		PRINT_ERROR(("Fail to create thread[thread_http_server_run]! errno[%d] errinfo[%s]",
			errno, strerror(errno)));
		return -1;
	}

	PRINT_INFO(("[ThreadID 0x%x] http service start!\n", (unsigned int)pthread_self()));
	return 0;
}

int http_mini_server_init(const char* ipaddr, int port, const char* path)
{
	int len;

	_http_server_init = 1;
	if (!path)
	{
		PRINT_ERROR(("Param path is NULL! [http_mini_server_init]\n"));
		goto out;
	}

	len = strlen(path);
	http_server_path = (char*)malloc(len + 1);
	if (!http_server_path)
	{
		PRINT_ERROR(("Fail to alloc space! [http_mini_server_init]\n"));
		goto out;
	}
	memset(http_server_path, 0, len + 1);
	memcpy(http_server_path, path, len);

	if (network_init(ipaddr, port) < 0)
		goto error;

	if (init_llist(&g_hpb_mgr) < 0)
		goto error;

	//if (init_read_select_thread() < 0)
	//	goto error;

	g_read_thrpool = lf_thread_pool_creat(MAX_THREAD_NUM, read_data,
		NULL, &_http_server_init);
	//g_process_thrpool = lf_thread_pool_creat(5, process_data,
	//	NULL, &_http_server_init);
	//g_write_thrpool = lf_thread_pool_creat(MAX_THREAD_NUM, write_data,
	//	NULL, &_http_server_init);
	if (!g_read_thrpool/* || !g_process_thrpool || !g_write_thrpool*/)
	{
		lf_thread_pool_free(g_read_thrpool);
		//lf_thread_pool_free(g_process_thrpool);
		//lf_thread_pool_free(g_write_thrpool);

		goto error;
	}

	INIT_LIST_HEAD(&g_callback_list);
	PRINT_INFO(("[ThreadID 0x%x] http server init success!\n", (unsigned int)pthread_self()));
	return 0;

error:
	FREE_SPACE(http_server_path);
out:
	_http_server_init = 0;
	usleep(100000);
	return -1;
}

int http_mini_add_cgi_callback(const char* name, HANDLE_FUNC hfunc, int method, void* arg)
{
	int len;
	CGI_CALLBACK* cgi;

	if (!_http_server_init)
		PRINT_ERROR_RETURN(("server does not init!\n"), -1);

	if (!name)
		PRINT_ERROR_RETURN(("method name is NULL\n"), -1);

	cgi = (CGI_CALLBACK*)malloc(sizeof(CGI_CALLBACK));
	if (!cgi)
		PRINT_ERROR_RETURN(("Fail to alloc space for cgi\n"), -1);
	memset(cgi, 0, sizeof(CGI_CALLBACK));

	len = strlen(name);
	cgi->name = (char*)malloc(len + 1);
	if (!cgi->name)
		PRINT_ERROR_RETURN(("Fail to alloc space for cgi name\n"), -1);
	memset(cgi->name, 0, len + 1);
	memcpy(cgi->name, name, len);

	cgi->hfunc = hfunc;
	cgi->method = method;
	cgi->arg = arg;

	list_add_tail(&cgi->plst, &g_callback_list);
	return 0;
}

int http_mini_add_read_callback(const char* name, HANDLE_FUNC rfunc)
{
	CGI_CALLBACK* cgi;
	struct list_head* pos;

	if (!_http_server_init)
		PRINT_ERROR_RETURN(("server does not init!\n"), -1);

	list_for_each(pos, &g_callback_list)
	{
		cgi = list_entry(pos, CGI_CALLBACK, plst);
		if (strcasecmp(cgi->name, name) == 0)
		{
			cgi->rfunc = rfunc;
			return 0;
		}
	}

	PRINT_ERROR_RETURN(("can not find cgi function!\n"), -1);
}


void http_mini_server_exit()
{
	struct list_head* pos;

	_http_server_init = 0;
#define FREE_XX_1(X) ({FREE_SPACE((X)->name);})
	FREE_LIST_HEAD(pos, &g_callback_list, CGI_CALLBACK, plst, FREE_XX_1);
#undef FREE_XX_1
	FREE_LIST_HEAD(pos, &g_hpb_mgr._mlst, HTTP_BASE, plst, FREE_HTTP_BASE);
	destroy_llist(&g_hpb_mgr);

	lf_thread_pool_free(g_read_thrpool);
	//lf_thread_pool_free(g_read_thrpool);
	//lf_thread_pool_free(g_read_thrpool);

	FREE_SPACE(http_server_path);

	//pthread_join(thread_run, NULL);
	//pthread_join(thread_read_select, NULL);

	PRINT_INFO(("[ThreadID 0x%x] http server exit success!\n", (unsigned int)pthread_self()));
}

