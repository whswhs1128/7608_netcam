#ifndef HTTP_ENCODE_H
#define HTTP_ENCODE_H

#include "http_class.h"

extern int get_http_packet_len(HTTP_BASE* hpb);
extern int http_encode(HTTP_BASE* hpb, char* buffer, int len);
extern int encoding_body(HTTP_BASE* hpb);

#endif //HTTP_ENCODE_H
