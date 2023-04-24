#include "http_utils.h"
#include "http_compress.h"
#include "http_encode.h"

int get_http_packet_len(HTTP_BASE* hpb)
{
	int len;
	HTTP_TAG* tag;
	struct list_head* pos;
	char buf[64] = {0};

	snprintf(buf, sizeof(buf), "HTTP/%d.%d ", hpb->http_major, hpb->http_minor);
	
	len = strlen(buf);
	len += strlen(http_error[hpb->errcode]);
	len += 2; // for /r/n
	
	list_for_each(pos, &hpb->resp_tag_list)
	{
		tag = list_entry(pos, HTTP_TAG, plst);
		if (!tag->name || !tag->value)
			continue;

		len += strlen(tag->name) + strlen(tag->value) + 4;  //for : space /r/n
	}

	len += 2;  //  for  /r/n
	len += hpb->resp_body_len;

	return len;
}

static int gzip_encoding(HTTP_BASE* hpb)
{
	unsigned int max_len;
	char* encoding_body;
	char* tmp;
	
	max_len = (unsigned int)(1.5 * (hpb->resp_body_len + 12));
	encoding_body = (char*)malloc(max_len);
	if (!encoding_body)
		PRINT_ERROR_RETURN(("Fail to alloc space for gzip encoding!\n"), HPE_OK);
	memset(encoding_body, 0, max_len);

	if (deflate_gzip(hpb->respond_body, hpb->resp_body_len, 
		encoding_body, &max_len, 1, 1) < 0)
	{
		FREE_SPACE(encoding_body);
		//fail to gzip, send original
		PRINT_ERROR_RETURN(("Fail to gzip http body!\n"), HPE_OK);
	}

	//switch data
	hpb->resp_body_len = (int)max_len;
	tmp = (char*)malloc(max_len);
	if (!tmp)
	{
		FREE_SPACE(hpb->respond_body);
		hpb->respond_body = encoding_body;
		PRINT_ERROR_RETURN(("Fail to alloc space to switch gzip http body!\n"), HPE_OK);
	}
	
	memset(tmp, 0, max_len);
	memcpy(tmp, encoding_body, max_len);

	FREE_SPACE(hpb->respond_body);
	FREE_SPACE(encoding_body);
	hpb->respond_body = tmp;
	
	add_http_tag(hpb, "Content-Encoding", "gzip", 0);
	return HPE_OK;
}

static int deflate_encoding(HTTP_BASE* hpb)
{	
	unsigned int max_len;
	char* encoding_body;
	char* tmp;
	
	max_len = (unsigned int)(1.5 * (hpb->resp_body_len + 12));
	encoding_body = (char*)malloc(max_len);
	if (!encoding_body)
		PRINT_ERROR_RETURN(("Fail to alloc space for deflate encoding!\n"), HPE_OK);
	memset(encoding_body, 0, max_len);

	if (deflate_deflate(hpb->respond_body, hpb->resp_body_len, 
		encoding_body, &max_len, 1) < 0)
	{
		FREE_SPACE(encoding_body);
		//fail to deflate, send original
		PRINT_ERROR_RETURN(("Fail to deflate http body!\n"), HPE_OK);
	}

	//switch data
	hpb->resp_body_len = (int)max_len;
	tmp = (char*)malloc(max_len);
	if (!tmp)
	{
		FREE_SPACE(hpb->respond_body);
		hpb->respond_body = encoding_body;
		PRINT_ERROR_RETURN(("Fail to alloc space to switch deflate http body!\n"), HPE_OK);
	}
	
	memset(tmp, 0, max_len);
	memcpy(tmp, encoding_body, max_len);

	FREE_SPACE(hpb->respond_body);
	FREE_SPACE(encoding_body);
	hpb->respond_body = tmp;
	
	add_http_tag(hpb, "Content-Encoding", "deflate", 0);
	return HPE_OK;
}

int encoding_body(HTTP_BASE* hpb)
{
	int ret = HPE_OK;
	char* accept_encoding;
	
	if (hpb->method != HTTP_GET)
		goto out;
	
	if (!hpb->respond_body || hpb->resp_body_len <= 0)
		goto out;

	accept_encoding = get_http_tag(hpb, "Accept-Encoding");
	if (!accept_encoding)
		goto out;         //no encoding

	str_tolower(accept_encoding);
	if (strstr(accept_encoding, "gzip"))
	{
		ret = gzip_encoding(hpb);
		goto out;
	}
	
	if (strstr(accept_encoding, "deflate"))
	{
		ret = deflate_encoding(hpb);
		goto out;
	}

out:
	return ret;
}


int http_encode(HTTP_BASE* hpb, char* buffer, int len)
{
	int index;
    int curIndex = 0;
	char* pbuf;
	HTTP_TAG* tag;
	struct list_head* pos;

	index = 0;
	pbuf = buffer;

	//protocol version
	index = snprintf(pbuf, len, "HTTP/%d.%d ", hpb->http_major, hpb->http_minor);
	pbuf += index;
    curIndex += index;

	//status
	index = snprintf(pbuf, len - curIndex, "%s\r\n", http_error[hpb->errcode]);
	pbuf += index;
    curIndex += index;

	//tag
	list_for_each(pos, &hpb->resp_tag_list)
	{
		tag = list_entry(pos, HTTP_TAG, plst);
		if (!tag->name || !tag->value)
			continue;

		//name
		index = snprintf(pbuf, len - curIndex, "%s: ", tag->name);
		pbuf += index;
        curIndex += index;

		//value
		index = snprintf(pbuf, len - curIndex, "%s\r\n", tag->value);
		pbuf += index;
        curIndex += index;
	}

	//http head end
	//can not use snprintf or sprintf at the end, snprintf or sprintf while copy '\0' to buffer
	//it will make "double free or corruption (!prev)"
	if (curIndex + 2 <= len)
	{
    	memcpy(pbuf, "\r\n", 2);
    	pbuf += 2;
        curIndex += 2;
	}

	index = len - (pbuf - buffer);
	if (hpb->resp_body_len != index)
		PRINT_ERROR_RETURN(("Calculate buffer len error! len[%d], body_len[%d], left[%d]",
		len, hpb->resp_body_len, index), -1);
	
	//no body
	if (hpb->resp_body_len == 0)
		return 0;

	//data
	if (curIndex + hpb->resp_body_len <= len)
	{
	    memcpy(pbuf, hpb->respond_body, hpb->resp_body_len);
	}
	return 0;
}


