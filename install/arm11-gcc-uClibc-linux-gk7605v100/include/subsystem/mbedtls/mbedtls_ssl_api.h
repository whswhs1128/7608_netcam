#ifndef _MBEDTLS_SSL_API_H_
#define _MBEDTLS_SSL_API_H_
void * gk_mbedtls_ssl_init(int fd, char *host, char* port);
int gk_mbedtls_ssl_send(void *Handle, char *buffer, int dataLen);
int gk_mbedtls_ssl_recv(void *Handle, char *buffer, int bufferLen);
int gk_mbedtls_ssl_exit(void *Handle);


#endif



