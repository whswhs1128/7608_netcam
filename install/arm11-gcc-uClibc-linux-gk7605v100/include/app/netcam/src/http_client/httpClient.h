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
���form-data����, ע��ÿ��
����goke_http_form_data_add������ݵ��ڴ治Ҫ�ظ�ʹ�ã���Ҫ�ͷţ�
�����ڲ���������ڴ档
goke_http_form_data_request�󼴿��ͷ�
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

#endif
