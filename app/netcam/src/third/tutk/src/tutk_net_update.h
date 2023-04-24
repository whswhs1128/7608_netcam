#ifndef TUTK_NET_UDPATE_H
#define TUTK_NET_UDPATE_H


typedef struct
{
	int avindex;
	SMsgAVIoctrlUpdateReq  req;
}TUTK_UPDATE_ParT;

unsigned long long tutk_update_version(void);

void tutk_update(void *arg);

int tutk_update_get_status(void);
int http_parse_url_info(char*url,char *host,int *port,char* body);
int tutk_update_get_process(void);
void tutk_update_set_status(char status);
int http_recv_parse_response_header(int sock,int *isGzipMethodUsed);
void tutk_update_test_start(void);

#endif
