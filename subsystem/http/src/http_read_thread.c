#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

#include "http_decode.h"
#include "http_main.h"
//#include "sdk_sys.h"


static int read_i(int fd, char* buffer, int length)
{
	int n, ret;
	int readlen = 0;
	char* pbuf = buffer;

	while (length > 0)
	{
		n = read(fd, pbuf, length);
		if (n < 0)
		{
			if (errno == EINTR || errno == EAGAIN)
			{
				//wait 200ms every time
				ret = time_wait(fd, WF_READ, 0, SELECT_TIMEOUT, MAX_RETRY_TIMES*2);
				if (ret > 0)
					continue;               //read again
				else if (ret == 0)
					break;                  //read finish
				else
				{goto error;}              //error
			}
			else
			{ret = 1; goto error;}
		}

		if (n == 0)
			break;                          //client close

		length     -= n;
		readlen    += n;
		pbuf       += n;
	}

	return readlen;

error:
	PRINT_ERROR(("Fail to read data[read_i]! n[%d] fd[%d] ret[%d] errno[%d] errinfo[%s]\n",
		n, fd, ret, errno, strerror(errno)));
	return -1;
}

static int parse_header(HTTP_BASE* hpb)
{
	int ret, len = PACKET_LENGTH;
	char buffer[HTTP_MAX_HEADER_SIZE] = {0};
	char* head_end;
#if 0
    char tmp[64] = {0};
    int tmpLen = 0;
    char defaultHead[] = "GET / HTTP/1.1";
#endif
	http_parser parser;
	while (1)
	{
		// peek the receive buffer and check the completed header firstly
		ret = recv(hpb->fd, buffer, len, MSG_PEEK);
		if (ret < 0)
		{
			PRINT_ERROR(("Fail to recv data[parse_header]! "
				"fd[%d] ret[%d] errno[%d] errinfo[%s]\n",
				hpb->fd, ret, errno, strerror(errno)));
			return 0;   //return 0 for close socket
		}

		if (ret == 0)
		{
            #if 0
			PRINT_ERROR(("recv data 0, client close[parse_header]! "
				"fd[%d] ret[%d] errno[%d] errinfo[%s]\n",
				hpb->fd, ret, errno, strerror(errno)));
            #endif
			return 0;
		}

		head_end = strstr(buffer, "\r\n\r\n");
		if (head_end)
			break;

		if (ret >= HTTP_MAX_HEADER_SIZE)
		{
			PRINT_ERROR(("http header is too large[parse_header]!\n"));
			hpb->errcode = -HPE_HEADER_OVERFLOW;
			return -1;
		}
	}

	//compute head len
	len = (head_end - buffer) + 4;    //  /r/n/r/n


	http_parser_init(&parser, HTTP_REQUEST);
    #if 0
    //分析http问题打开
    tmpLen = strlen(buffer) - strlen(strstr(buffer, "\r\n"));
    if (tmpLen >= 64)
    {
        tmpLen = 63;
    }
    memcpy(tmp, buffer, tmpLen);
    if (strcmp(tmp, defaultHead) == 0)
    {
        if (len > 64)
            memcpy(tmp, buffer, 64);
        else
            memcpy(tmp, buffer, len);
        printf("-->http1 len:%d,:%s\n", len, tmp);
    }
    else
    {
        if (strcmp(tmp, "P") == 0)
        {
            if (len > 64)
                memcpy(tmp, buffer, 64);
            else
                memcpy(tmp, buffer, len);
            printf("-->http2 len:%d,:%s\n", len, tmp);
        }
        else
            printf("-->http3 len:%d,:%s\n", len, tmp);
    }
    #endif
    //useful data is 0. it will cause Segmentation fault. so return
    if (len == 4)
    {
        hpb->isfree == HPB_FREE_BY_CGI_CONTROL;
        return 0;
    }
	http_parser_execute(&parser, hpb, buffer, len);
	hpb->errcode = 0 - parser.http_errno;

	if(hpb->errcode){
		return -1;
	}
	print_packet_head(buffer, "request OK");
	print_http_parse_result(hpb);

    //hpb->request_path [/nice%20ports%2C/Tri%6Eity.txt%2ebak] may error.
    //printf("request path:%s\n", hpb->request_path);
    if (strstr(hpb->request_path, "/nice") != NULL)
    //if(0)    
    {
        hpb->isfree == HPB_FREE_BY_CGI_CONTROL;
        return 0;
    }

	return len;
}

static void read_discard_head(int fd, int headlen)
{
	char* buffer = (char*)alloca(headlen);
	if (buffer == NULL) {
        PRINT_ERROR(("read_discard_head: malloc error %d\n", headlen));
	} else {
	    read_i(fd, buffer, headlen);
	}
}

static int check_cgi_read(HTTP_BASE* hpb)
{
	CGI_CALLBACK* cgi;
	struct list_head* pos;

	list_for_each(pos, &g_callback_list)
	{
		cgi = list_entry(pos, CGI_CALLBACK, plst);
		if (strcasecmp(cgi->name, hpb->request_path) == 0 &&
			cgi->rfunc)
		{
			read_discard_head(hpb->fd, hpb->packet_len);
			return cgi->rfunc(&hpb->hp_ops, cgi->arg);
		}
	}

	return 0;
}

void read_data(struct list_head* entry, void* arg)
{
	int ret;
	HTTP_BASE* hpb;

	hpb = container_of(entry, HTTP_BASE, plst);

	//parse head
	hpb->packet_len = parse_header(hpb);
	if (hpb->packet_len < 0)
		goto error;
	if (hpb->packet_len == 0)
		goto clientclose;

	//cgi read packet
	if ((ret = check_cgi_read(hpb)) < 0)
		goto error;

	if (ret == HPE_RET_KEEP_ALIVE1)
	{
		hpb->isfree = HPB_FREE_BY_CGI_CONTROL;
		goto cgiread;
	}

	if (ret == HPE_RET_KEEP_ALIVE)
		goto error;

	//http server read packet
	//remember http head len
	hpb->req_body_len = hpb->packet_len;

	//recv packet
	hpb->packet_len += get_content_length(hpb);
	hpb->packet = (char*)malloc(hpb->packet_len + 1);
	if (!hpb->packet)
	{
		PRINT_ERROR(("Fail to alloc space for request packet[read_data]!\n"));
		hpb->errcode = -HPE_MALLOC_FAIL;
		goto error;
	}
	memset(hpb->packet, 0, hpb->packet_len + 1);

	ret = read_i(hpb->fd, hpb->packet, hpb->packet_len);
	if (ret < hpb->packet_len)
	{
		PRINT_ERROR(("recv data length error[read_data]! "
				"fd[%d],recv_len[%d],content_len[%d],total_len[%d],"
				"errno[%d],errinfo[%s]\n[\n%s\n]\n\n",
				hpb->fd, ret, get_content_length(hpb), hpb->packet_len,
				errno, strerror(errno), hpb->packet));
		hpb->errcode = -HPE_RECV_PACKET_FAIL;
		FREE_SPACE(hpb->packet);
		hpb->packet_len = hpb->req_body_len = 0;
		goto error;
	}

	//compute request body
	hpb->request_body = hpb->packet + hpb->req_body_len;
	hpb->req_body_len = hpb->packet_len - hpb->req_body_len;
	if (!hpb->req_body_len) hpb->request_body = NULL;

error:
	//push_entry_in_pool(&hpb->plst, g_process_thrpool);
	process_data(hpb);
	return;

clientclose:
	//error: push in g_hpb_mgr to free
	if (hpb->isfree == HPB_FREE_BY_CGI_CONTROL)
		hpb->isfree = HPB_CLOSE_SOCKET_ONLY;
	else
		hpb->isfree = HPB_REALLY_FREE;

cgiread:
	llist_add_tail(&hpb->plst, &g_hpb_mgr);
	return ;
}


int fill_fdset(struct lock_list_head* head, fd_set* fdset)
{
	int max_fd = 0;
	HTTP_BASE* hpb;
	struct list_head* pos;

	pthread_mutex_lock(&head->_mutex);
	//while (_http_server_init && list_empty(&head->_mlst))
	//	pthread_cond_wait(&head->_cond, &head->_mutex);

	//if (!_http_server_init) return 0;

	pos = head->_mlst.next;
	while (pos && pos != &head->_mlst)
	{
		hpb = list_entry(pos, HTTP_BASE, plst);
		pos = pos->next;          //point next

		if (hpb->isfree == HPB_REALLY_FREE)
		{
			PRINT_DEBUG(("Http server will free struct hpb[0x%x],isfree[%d]\n",
				(unsigned int)hpb, hpb->isfree));
			list_del(&hpb->plst);     //delete node and free hpb
			FREE_HTTP_BASE(hpb);
			continue;
		}

		if (hpb->isfree == HPB_FREE_BY_TIME_CONTROL && hpb->start_time &&
			(time(NULL) - hpb->start_time > hpb->expires_time))
		{
			PRINT_DEBUG(("struct hpb[0x%x] is time out, Http server will free it. isfree[%d]\n",
				(unsigned int)hpb, hpb->isfree));
			hpb->isfree = HPB_REALLY_FREE;
			continue;
		}

		if (hpb->isfree == HPB_CLOSE_SOCKET_ONLY)
		{
			if (hpb->fd > 0)
				PRINT_DEBUG(("Http server will close socket fd[%d],struct hpb[0x%x],isfree[%d]\n",
					hpb->fd, (unsigned int)hpb, hpb->isfree));
			close(hpb->fd);
			hpb->fd = -1;
			continue;
		}

		if (hpb->fd < 0)
			continue;

		FD_SET(hpb->fd, fdset);
		hpb->fdstatus = CONN_STATUS_HANDLE;
		if (max_fd < hpb->fd)
			max_fd = hpb->fd;

		if (hpb->isfree != HPB_FREE_BY_CGI_CONTROL)
			RESET_HTTP_BASE(hpb);
	}
	pthread_mutex_unlock(&head->_mutex);

	return max_fd;
}

void handle_read_i(struct lock_list_head* head, fd_set* fdset)
{
	HTTP_BASE* hpb;
	struct list_head* pos;

	pthread_mutex_lock(&head->_mutex);
	pos = head->_mlst.next;
	while (pos && pos != &head->_mlst)
	{
		hpb = list_entry(pos, HTTP_BASE, plst);
		pos = pos->next;

		if (hpb->fd < 0)
			continue;

		if (hpb->fdstatus == CONN_STATUS_ACCEPT)
			continue;

		if (FD_ISSET(hpb->fd, fdset))
		{
			list_del(&hpb->plst);
			push_entry_in_pool(&hpb->plst, g_read_thrpool);
		}
	}
	pthread_mutex_unlock(&head->_mutex);
}

//read select 和listen select合并，减少线程

/*
static void* handle_read(void *arg)
{
    sdk_SetThreadName("httpHandle_read");
	int max_fd = 0;
	fd_set readFDs;
	struct timeval tv;

	PRINT_INFO(("[ThreadID 0x%x] read select thread start!\n",
		(unsigned int)pthread_self()));
	while (_http_server_init)
	{
		FD_ZERO(&readFDs);
		max_fd = fill_fdset(&g_hpb_mgr, &readFDs);

		if (max_fd == 0)
		{
			usleep(100000);
			continue;
		}

		tv.tv_sec = 0;
		tv.tv_usec = SELECT_TIMEOUT;

		if (select(max_fd + 1, &readFDs, 0, 0, &tv) < 0)
		{
			//whatever error, still while
			PRINT_ERROR(("select error [handle_read]! errno[%d] errinfo[%s]\n",
				errno, strerror(errno)));
			continue;
		}

		handle_read_i(&g_hpb_mgr, &readFDs);
	}

	return 0;
}

int init_read_select_thread()
{
	if (pthread_create(&thread_read_select, NULL, handle_read, NULL) < 0)
	{
		PRINT_ERROR(("Fail to create thread [init_read_select_thread]! errno[%d] errinfo[%s]",
			errno, strerror(errno)));
		return -1;
	}

	return 0;
}
*/

