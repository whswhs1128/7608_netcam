#ifndef HTTP_CLIENT_H
#define HTTP_CLIENT_H

typedef enum
{
    HTTP_GET,
    HTTP_POST
}HTTP_METHOD;

typedef enum
{
	MULTIPART_DATA_TXT,
	MULTIPART_DATA_STREAM,
}HTTP_UPLOAD_MULTI_META_TYPE;

typedef struct
{
	char *name;
	char *value;
	int   valueLen;
	HTTP_UPLOAD_MULTI_META_TYPE  dataType;
    char *fileName;
	void *next;
}HTTP_MULTI_META_DATA;

/*
添加form-data数据, 注意每次
调用goke_http_form_data_add添加数据的内存不要重复使用，不要释放，
函数内部不会分配内存。
goke_http_form_data_request后即可释放
*/
HTTP_MULTI_META_DATA * goke_http_form_data_create(void);
void goke_http_form_data_add(HTTP_MULTI_META_DATA *dataStart, char *name,char *value,int valueLen,
	HTTP_UPLOAD_MULTI_META_TYPE  dataType, char *fileName);
//need free the return data
char *goke_http_form_data_request(char* url, HTTP_MULTI_META_DATA *dataStart, int* dataLength, int *dataRecvLen);


//need free the return data
char *goke_http_request(const char* url, int* dataLength, int *dataRecvLen, HTTP_METHOD type, char *sPostParam);

char *goke_http_binary_request(const char* url, int* dataLength, 
int *dataRecvLen, HTTP_METHOD type, char *sPostParam,char *dataPtr,int dataSize);
int http_tcp_select_send(int sock, char *szbuf, int len, int timeout);
int http_create_request_socket(const char* host, int isSsl);
int http_parser_url(const char* url,char **host,char **path);
#endif
