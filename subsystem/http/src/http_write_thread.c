#include <unistd.h>
#include <sys/sendfile.h>

#include "http_encode.h"
#include "http_main.h"

static int write_i(int fd, char* buffer, int length)
{
	int bytes_left = length;
	int written = 0;
	int n, ret = 0;
	char* ptr = buffer;

	while (bytes_left > 0)
	{
		n = write(fd, ptr, bytes_left);
		if (n < 0)
		{
			if(errno == EINTR || errno == EAGAIN)
			{
				//wait 200ms every time
				ret = time_wait(fd, WF_WRITE, 0, SELECT_TIMEOUT, MAX_RETRY_TIMES);
				if (ret > 0)
					continue;
				else if (ret == 0)
				{
					PRINT_DEBUG(("no data to write[write_i]! fd[%d] n[%d] ret[%d] "
						"length[%d] written[%d] bytes_left[%d] errno[%d] errinfo[%s]\n", 
						fd, n, ret, length, written, bytes_left, errno, strerror(errno)));
					break;
				}
				else
				{goto error;}
			}
			else
				{ret = 1; goto error;}
		}

		bytes_left -= n;
		written += n;
		ptr += n;
	}

	return written;

error:
	PRINT_ERROR(("write error[write_i]! fd[%d] n[%d] ret[%d] "
		"length[%d] written[%d] bytes_left[%d] errno[%d] errinfo[%s]\n", 
		fd, n, ret, length, written, bytes_left, errno, strerror(errno)));

	return -1;
}

static int write_raw_packet(HTTP_BASE* hpb)
{
	if (!hpb->respond_body || hpb->resp_body_len <= 0)
		PRINT_ERROR_RETURN(("Fail to get raw http packet! len[%d] [write_raw_packet]\n", 
		hpb->resp_body_len), -1);

	if (write_i(hpb->fd, hpb->respond_body, hpb->resp_body_len) < 0)
		return -1;
	
	print_packet_head(hpb->respond_body, "respond OK");
	return 0;
}

static int write_packet(HTTP_BASE* hpb)
{
	int len;
	char* buffer = NULL;

	len = get_http_packet_len(hpb);
	buffer = (char*)malloc(len);
	if (!buffer)
		PRINT_ERROR_RETURN(("Fail to alloc space for buffer! [write_packet]\n"), -1);
	
	memset(buffer, 0, len);
	if (http_encode(hpb, buffer, len))
	{
		FREE_SPACE(buffer);
		return -1;
	}

	if (write_i(hpb->fd, buffer, len) < 0)
	{
		FREE_SPACE(buffer);
		return -1;
	}

	print_packet_head(buffer, "respond OK");
	FREE_SPACE(buffer);
	return 0;
}

static int send_file(HTTP_BASE* hpb)
{
	ssize_t ret;
	ssize_t send_len = 0;
	off_t offset = 0;
	
	//send head: hpb->resp_body_len = 0
	if (write_packet(hpb) < 0)
		return -1;

	while (send_len < hpb->file_size)
	{
		if ((ret = sendfile(hpb->fd, hpb->filefd, &offset, hpb->file_size)) < 0)
		{
			if (errno == EAGAIN)
			{
				while((ret = time_wait(hpb->fd, WF_WRITE, 
					0, SELECT_TIMEOUT, MAX_RETRY_TIMES)) == 0);
				if (ret > 0)
					continue;
			}
			
			PRINT_ERROR_RETURN(("Fail to send file! [send_file] "
				"filesize[%d], send[%d], ret[%d], errno[%d] errinfo[%s]\n",
				hpb->file_size, send_len, ret, errno, strerror(errno)),-1);
		}

		send_len += ret;
	}

	PRINT_DEBUG(("Download file ![send_file] filesize[%d], send[%d]\n", 
		hpb->file_size, send_len));
	return 0;
}

static int handle_write(HTTP_BASE* hpb)
{
	int ret = 0;
	switch (hpb->errcode)
	{
	case HPE_DOWNLOAD_PRE_PROCESS:
		hpb->errcode = error_code_mapping(HPE_OK);
		ret = send_file(hpb);
		break;
		
	case HPE_RET_DISCONNECT:
		ret = write_raw_packet(hpb);
		hpb->isfree = HPB_REALLY_FREE;
		break;

	case HPE_RET_KEEP_ALIVE:
		ret = write_raw_packet(hpb);
		hpb->isfree = HPB_FREE_BY_CGI_CONTROL;
		break;

	case HPE_RET_KEEP_ALIVE1:
		hpb->isfree = HPB_FREE_BY_CGI_CONTROL;
		break;

	case HPE_RET_DISCONNECT1: //for cgi use http server send packet
		hpb->isfree = HPB_REALLY_FREE;
		hpb->errcode = HPE_OK;
	default:
		hpb->errcode = error_code_mapping(hpb->errcode);
		ret = write_packet(hpb);
        
		break;
	}

	return ret;
}

//void write_data(struct list_head* entry, void *arg)
void write_data(HTTP_BASE* hpb)
{
	int retry;
	//HTTP_BASE* hpb;
	fd_set writeFDs;
	struct timeval tv;

	//hpb = container_of(entry, HTTP_BASE, plst);

	for (retry = 0; retry < MAX_RETRY_TIMES; retry++)
	{
		FD_ZERO(&writeFDs);
		FD_SET(hpb->fd, &writeFDs);
		tv.tv_sec = 0;
		tv.tv_usec = SELECT_TIMEOUT;

		if (select(hpb->fd + 1, 0, &writeFDs, 0, &tv) < 0)
		{
			//whatever error, still while
			PRINT_ERROR(("select error [write_data]! errno[%d] errinfo[%s]\n", 
				errno, strerror(errno)));
			goto error;
		}

		if (FD_ISSET(hpb->fd, &writeFDs))
		{
            int ret = handle_write(hpb);
           
			if (ret < 0)
				goto error;
            else
				goto finish;
		}
	}

	//select MAX_RETRY_TIMES but fd can not write, so free it
	PRINT_ERROR(("Retry %d times to write data but error! fd[%d]\n", 
		MAX_RETRY_TIMES, hpb->fd));
error:
	hpb->isfree = HPB_REALLY_FREE;
finish:
	//push in g_hpb_mgr for next step
	llist_add_tail(&hpb->plst, &g_hpb_mgr);
}


