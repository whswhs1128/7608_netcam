#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include "http_encode.h"
#include "http_main.h"

static int compare_resource_timestamp(time_t local, char *remote)
{
	//Thu, 19 Nov 2015 02:41:20 GMT
	if(remote == NULL)
	{
		PRINT_ERROR(("Invalid parameters."));
		return -1;
	}
	struct tm temp1;
	strptime(remote, "%a, %d %b %Y %T", &temp1);
	time_t temp2;
	temp2 = mktime(&temp1)+8*60*60;
	//printf("local resource timestamp: %d\nremote resource timestamp: %d\n", (int)local, (int)temp2);
	if(local > temp2)
	{
		//printf("resouce had been changed.");
		return 1;
	}
	else
	{
		//printf("resource had not been changed.");
		return 0;
	}
}

static int download_file(HTTP_BASE *hpb, int fd, const char* filename, struct stat* finfo, int flag)
{
	struct tm utc_tm;
	char buf[256];

	//set tag
	add_common_tag(hpb);
	
	memset(buf, 0, sizeof(buf));
	snprintf(buf, sizeof(buf), "attachment;filename=\"%s\"", filename);
	add_http_tag(hpb, "Content-Disposition", buf, 0);

	if (flag)
		add_cache_control(hpb, DOWN_FILE_CACHE_TIME);
	else
		add_cache_control(hpb, 0);

	gmtime_r(&finfo->st_mtime, &utc_tm);
	memset(buf, 0, sizeof(buf));
	strftime(buf, sizeof(buf), "%a, %d %b %Y %T GMT", &utc_tm);
	add_http_tag(hpb, "Last-Modified", buf, 0);

	add_content_len(hpb, (int)finfo->st_size);
	add_http_tag(hpb, "Accept-Ranges", "bytes", 0);
	
	//remember fd for write thread
	hpb->filefd = fd;
	hpb->file_size = finfo->st_size;
	set_http_body(hpb, 0);
	return HPE_DOWNLOAD_PRE_PROCESS;
}

static int read_file(HTTP_BASE *hpb, int fd, const char* filename, struct stat* finfo)
{
	char* pbuf;
	struct tm utc_tm;
	char buf[64];
	int ret = HPE_OK;
	/************** Add by Ludy, please Teng xiaozhou check ************/	
	#if 1
	int stampStatus = 1;	// default value is 1
	char *ifModified = NULL;
	ifModified = get_http_tag(hpb, "If-Modified-Since");
	if(ifModified)
	{
		stampStatus = compare_resource_timestamp(finfo->st_mtime, ifModified);
	}
	if(stampStatus == 0)
	{
		ret = -HPE_RECOURCE_NOT_MODIFIED;	
	}
	else
	{
		pbuf = set_http_body(hpb, finfo->st_size);
		if (!pbuf)
		{
			PRINT_ERROR(("Fail to alloc space [read_file]!\n"));
			ret = -HPE_MALLOC_FAIL;
			goto error;
		}
		
		//read
		if (read(fd, pbuf, finfo->st_size) < 0)
		{
			PRINT_ERROR(("Fail to read file[%s] [read_file]! errno[%d] errinfo[%s]\n", 
				filename, errno, strerror(errno)));
			ret = -HPE_READ_FILE_FAIL;
			goto error1;
		}
	}
	#else
	pbuf = set_http_body(hpb, finfo->st_size);
	if (!pbuf)
	{
		PRINT_ERROR(("Fail to alloc space [read_file]!\n"));
		ret = -HPE_MALLOC_FAIL;
		goto error;
	}

	//read
	if (read(fd, pbuf, finfo->st_size) < 0)
	{
		PRINT_ERROR(("Fail to read file[%s] [read_file]! errno[%d] errinfo[%s]\n", 
			filename, errno, strerror(errno)));
		ret = -HPE_READ_FILE_FAIL;
		goto error1;
	}
	#endif
	/************** Add by Ludy, please Teng xiaozhou check ************/	
	
	//set tag
	add_cache_control(hpb, READ_FILE_CACHE_TIME);
	
	gmtime_r(&finfo->st_mtime, &utc_tm);
	memset(buf, 0, sizeof(buf));
	strftime(buf, sizeof(buf), "%a, %d %b %Y %T GMT", &utc_tm);
	add_http_tag(hpb, "Last-Modified", buf, 0);

	finfo->st_mtime += READ_FILE_CACHE_TIME;
	gmtime_r(&finfo->st_mtime, &utc_tm);
	memset(buf, 0, sizeof(buf));
	strftime(buf, sizeof(buf), "%a, %d %b %Y %T GMT", &utc_tm);
	add_http_tag(hpb, "Expires", buf, 0);

	return ret;

error1:
	set_http_body(hpb, 0);
error:
	return ret;
}

static int handle_file(HTTP_BASE *hpb)
{
	int fd;
	STNAME* st;
	struct stat file_info;
	char filename[256];
	const char *first_subpath, *last_subpath;
	int ret = HPE_OK;

	if (!hpb->request_path)
	{
		PRINT_ERROR(("request path is null! [handle_file]\n"));
		ret = -HPE_FILE_NOT_EXIST;
		goto out;
	}

	memset(filename, 0, sizeof(filename));
	snprintf(filename, sizeof(filename), "%s%s", 
			http_server_path, hpb->request_path);
	
	//open
	fd = open(filename, O_RDONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
	if (fd < 0)
	{
		PRINT_ERROR(("Fail to open file[%s] [handle_file]! errno[%d] errinfo[%s]\n", 
			filename, errno, strerror(errno)));
		ret = -HPE_FILE_NOT_EXIST;
		goto out;
	}

	//stat
	if (fstat(fd, &file_info))
	{
		PRINT_ERROR(("Fail to stat file[%s] [handle_file]! errno[%d] errinfo[%s]\n", 
			filename, errno, strerror(errno)));
		ret = -HPE_READ_FILE_FAIL;
		goto error;
	}

	if (list_empty(&hpb->rpath_list))
	{
		PRINT_DEBUG(("Request sub-path is empty! [handle_file]\n"));
		goto _readfile_;
	}

	st = list_entry(hpb->rpath_list.next, STNAME, plst);
	if (!st || !st->name)
	{
		PRINT_ERROR(("Get first sub-path error! [handle_file]\n"));
		goto _readfile_;
	}
	first_subpath = st->name;
	
	st = list_entry(hpb->rpath_list.prev, STNAME, plst);
	if (!st || !st->name)
	{
		PRINT_ERROR(("Get last sub-path error! [handle_file]\n"));
		goto _readfile_;
	}
	last_subpath = st->name;

	//dowload file
	if (strcasecmp(first_subpath, "Download") == 0)
	{
		ret = download_file(hpb, fd, last_subpath, &file_info, 1);
		goto out;        //download file do not close fd in here
	}

	if (strcasecmp(first_subpath, "snapshot_dir") == 0)
	{
		ret = download_file(hpb, fd, last_subpath, &file_info, 0);
		goto out;
	}

_readfile_:
	ret = read_file(hpb, fd, filename, &file_info);

error:
	close(fd);
out:
	return ret;
}

static int handle_cgi(HTTP_BASE *hpb, int method)
{
	int ret = HPE_OK;
	CGI_CALLBACK* cgi;
	STNAME* st;
	struct list_head* pos;
	struct list_head* pst;

	//for match whole request path
	list_for_each(pos, &g_callback_list)
	{
		cgi = list_entry(pos, CGI_CALLBACK, plst);
		if (cgi->hfunc && hpb->request_path &&
			cgi->name && (cgi->method & method) &&
			strcasecmp(hpb->request_path, cgi->name)== 0)
		{
			ret = cgi->hfunc(&hpb->hp_ops, cgi->arg);
			goto out;
		}
	}

	//for match sub request path
	list_for_each(pst, &hpb->rpath_list)
	{
		st = list_entry(pst, STNAME, plst);

		list_for_each(pos, &g_callback_list)
		{
			cgi = list_entry(pos, CGI_CALLBACK, plst);

			if (cgi->hfunc && st->name && cgi->name && 
				(cgi->method & method) && 
				strcasecmp(st->name, cgi->name)== 0)
			{
				ret = cgi->hfunc(&hpb->hp_ops, cgi->arg);
				goto out;
			}
		}
	}

	ret = -HPE_NO_HANDLE_FUNC;
	PRINT_ERROR(("No cgi handle function to process request[%s]!\n",
		hpb->request_path ? hpb->request_path : "null"));

out:
	if (ret == HPE_OK)
		add_cache_control(hpb, CGI_CACHE_TIME);
	return ret;
}


static int process_get_method(HTTP_BASE *hpb)
{
	int ret = HPE_OK;

	//for common file
	if (hpb->req_path_type > REQUEST_PATH_TYPE_CGI)
	{
		ret = handle_file(hpb);
		goto out;
	}

	ret = handle_cgi(hpb, METHOD_GET);

out:
	return ret;
}

static int process(HTTP_BASE *hpb)
{
	int ret;
	int method = hpb->method;

	switch (method)
	{
	case HTTP_GET:
		ret = process_get_method(hpb);
		break;

	case HTTP_PUT:
		ret = handle_cgi(hpb, METHOD_PUT);
		break;
		
	case HTTP_POST:
		ret = handle_cgi(hpb, METHOD_POST);
		break;

	case HTTP_DELETE:
	case HTTP_HEAD:
	case HTTP_CONNECT:
	case HTTP_OPTIONS:
	case HTTP_TRACE:
	case HTTP_COPY:
	case HTTP_LOCK:
	case HTTP_MKCOL:
	case HTTP_MOVE:
	case HTTP_PROPFIND:
	case HTTP_PROPPATCH:
	case HTTP_SEARCH:
	case HTTP_UNLOCK:
	case HTTP_BIND:
	case HTTP_REBIND:
	case HTTP_UNBIND:
	case HTTP_ACL:
	case HTTP_REPORT:
	case HTTP_MKACTIVITY:
	case HTTP_CHECKOUT:
	case HTTP_MERGE:
	case HTTP_MSEARCH:
	case HTTP_NOTIFY:
	case HTTP_SUBSCRIBE:
	case HTTP_UNSUBSCRIBE:
	case HTTP_PATCH:
	case HTTP_PURGE:
	case HTTP_MKCALENDAR:
		ret = -HPE_NO_HANDLE_FUNC;
		break;

	default:
		ret = -HPE_INVALID_METHOD;
		break;
	}

	return ret;
}

//void process_data(struct list_head* entry, void *arg)
void process_data(HTTP_BASE *hpb)
{
	int ret;
	//HTTP_BASE *hpb;

	//hpb = container_of(entry, HTTP_BASE, plst);
	ret = hpb->errcode;
	if (ret < HPE_OK)
	{
		hpb->isfree = HPB_REALLY_FREE;
		goto error;             //read fail
	}
	
    ret = process(hpb);
	if (ret < HPE_OK)
		goto error;             //process fail

	switch (ret)
	{
	case HPE_DOWNLOAD_PRE_PROCESS:  //??¨®¨²???????t¦Ì?cgi¦Ì?¡¤¦Ì???¦Ì
	case HPE_RET_DISCONNECT:        //2?D¨¨¨°ahttpserver¨¬¨ª?¨®http¨ª¡¤¦Ì?cgi¦Ì?¡¤¦Ì???¦Ì¡ê?¨¨?onvif
	case HPE_RET_KEEP_ALIVE:        //http server??¨®|request, 2¡é?¨°¡À¡ê3?¡ä?¨¢??¨®?¡À¦Ì?cgi¨ª?3?
	case HPE_RET_KEEP_ALIVE1:       //http server2???¨®|request¡ê?¦Ì??¨¢¡À¡ê3?¡ä?¨¢??¨®?¡À¦Ì?cgi¨ª?3?
		goto finish;

	case HPE_RET_DISCONNECT1:       //¨º1¨®?httpserver¦Ì?http¨ª¡¤¦Ì?cgi¦Ì?¡¤¦Ì???¦Ì
		goto error;
	default:
		break;
	}

	//ret = HPE_OK, cgi or read file
error:
	add_common_tag(hpb);
	encoding_body(hpb);
	add_content_len(hpb, hpb->resp_body_len);

finish:
	hpb->errcode = ret;
	//push_entry_in_pool(&hpb->plst, g_write_thrpool);
	write_data(hpb);
}


