#ifndef HTTPD_H
#define HTTPD_H

//not need free the data in the callback
typedef void(*HTTPD_CallBack)(char  *data, int dataLen, char *path);

int goke_httpd_start(int port, HTTPD_CallBack data_recv_func);

#endif
