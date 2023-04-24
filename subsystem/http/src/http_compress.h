#ifndef HTTP_COMPRESS_H
#define HTTP_COMPRESS_H

/**
	compress by gzip
*/
extern int deflate_gzip(char* in, unsigned int in_len, 
	char* out, unsigned int* out_len, int front, int end);

/**
	compress by deflate
*/
extern int deflate_deflate(char* in, unsigned int in_len, 
	char* out, unsigned int* out_len, int end);

/**
	uncompress by gzip
*/
extern int inflate_gzip(char* in, unsigned int in_len, 
	char* out, unsigned int* out_len, int front, int end);

#endif //HTTP_COMPRESS_H
