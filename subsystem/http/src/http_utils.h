#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

extern void str_tolower(char* s);
extern void http_space_to_sys_space(char* src, int n);
extern int dump_chunk(char** buffer, const char* chunk, int len, int append);

#endif //HTTP_UTILS_H
