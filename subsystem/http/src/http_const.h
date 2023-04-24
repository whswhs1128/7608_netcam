#ifndef HTTP_CONST_H
#define HTTP_CONST_H

#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "http_list.h"

/* Request Methods */
#define HTTP_METHOD_MAP(XX)         \
  XX(0,  DELETE,      DELETE)       \
  XX(1,  GET,         GET)          \
  XX(2,  HEAD,        HEAD)         \
  XX(3,  POST,        POST)         \
  XX(4,  PUT,         PUT)          \
  /* pathological */                   \
  XX(5,  CONNECT,     CONNECT)      \
  XX(6,  OPTIONS,     OPTIONS)      \
  XX(7,  TRACE,       TRACE)        \
  /* WebDAV */                      \
  XX(8,  COPY,        COPY)         \
  XX(9,  LOCK,        LOCK)         \
  XX(10, MKCOL,       MKCOL)        \
  XX(11, MOVE,        MOVE)         \
  XX(12, PROPFIND,    PROPFIND)     \
  XX(13, PROPPATCH,   PROPPATCH)    \
  XX(14, SEARCH,      SEARCH)       \
  XX(15, UNLOCK,      UNLOCK)       \
  XX(16, BIND,        BIND)         \
  XX(17, REBIND,      REBIND)       \
  XX(18, UNBIND,      UNBIND)       \
  XX(19, ACL,         ACL)          \
  /* subversion */                    \
  XX(20, REPORT,      REPORT)       \
  XX(21, MKACTIVITY,  MKACTIVITY)   \
  XX(22, CHECKOUT,    CHECKOUT)     \
  XX(23, MERGE,       MERGE)        \
  /* upnp */                         \
  XX(24, MSEARCH,     M-SEARCH)     \
  XX(25, NOTIFY,      NOTIFY)       \
  XX(26, SUBSCRIBE,   SUBSCRIBE)    \
  XX(27, UNSUBSCRIBE, UNSUBSCRIBE)  \
  /* RFC-5789 */                     \
  XX(28, PATCH,       PATCH)        \
  XX(29, PURGE,       PURGE)        \
  /* CalDAV */                       \
  XX(30, MKCALENDAR,  MKCALENDAR)   \

enum http_method
  {
#define XX(num, name, string) HTTP_##name = (num),
  HTTP_METHOD_MAP(XX)
#undef XX
  };


/*http packet error code*/
#define HTTP_ERROR_CODE_MAP(XX)                       \
  XX(0,  Continue, "100 Continue")                       \
  XX(1,  SwitchingProtocols, "101 Switching Protocols")            \
  XX(2,  OK, "200 OK")                             \
  XX(3,  Created, "201 Created")                        \
  XX(4,  Accepted, "202 Accepted")                       \
  XX(5,  Partial, "203 Partial Information")            \
  XX(6,  NoContent, "204 No Content")                     \
  XX(7,  ResetContent, "205 Reset Content")                  \
  XX(8,  PartialContent, "206 Partial Content")                \
  XX(9,  MultipleChoices, "300 Multiple Choices")               \
  XX(10, MovedPermanently, "301 Moved Permanently")             \
  XX(11, MovedTemporarily, "302 Moved Temporarily")             \
  XX(12, SeeOther, "303 See Other")                     \
  XX(13, NotModified, "304 Not Modified")                  \
  XX(14, UseProxy, "305 Use Proxy")                     \
  XX(15, BadRequest, "400 Bad Request")                   \
  XX(16, Unauthorized, "401 Unauthorized")                  \
  XX(17, PaymentRequired, "402 Payment Required")              \
  XX(18, Forbidden, "403 Forbidden")                     \
  XX(19, NotFound, "404 Not Found")                     \
  XX(20, MethodNotAllowed, "405 Method Not Allowed")            \
  XX(21, NotAcceptable, "406 Not Acceptable")                \
  XX(22, ProxyAuthRequired, "407 Proxy Authentication Required") \
  XX(23, RequestTimeOut, "408 Request Time-Out")              \
  XX(24, Conflict, "409 Conflict")                      \
  XX(25, Gone, "410 Gone")                          \
  XX(26, LengthRequired, "411 Length Required")               \
  XX(27, PreconditionFailed, "412 Precondition Failed")           \
  XX(28, RequestEntityTooLarge, "413 Request Entity Too Large")      \
  XX(29, RequestURLTooLarge, "414 Request-URL Too Large")         \
  XX(30, UnsupportedMediaType, "415 Unsupported Media Type")        \
  XX(31, ServerError, "500 Server Error")                  \
  XX(32, NotImplemented, "501 Not Implemented")               \
  XX(33, BadGateway, "502 Bad Gateway")                   \
  XX(34, OutOfResources, "503 Out of Resources")              \
  XX(35, GatewayTimeOut, "504 Gateway Time-Out")              \
  XX(36, VersionNotSupported, "505 HTTP Version not supported")



enum http_error_code
  {
#define XX(num, name, detail) HTTP_STATUS_##name = num,
  HTTP_ERROR_CODE_MAP(XX)
#undef XX
  };

extern const char* http_error[];

/*Content type*/
#define REQUEST_PATH_TYPE_CGI       0
#define REQUEST_PATH_TYPE_HTML      1

extern const char* file_suffix[];
extern const char* content_type[];

/*http server inner error code*/
#define HTTP_ERRNO_MAP(XX)                                           \
  /* Parsing-related errors */                                            \
  XX(INVALID_EOF_STATE, "stream ended at an unexpected time")        \
  XX(HEADER_OVERFLOW,                                                \
     "too many header bytes seen; overflow detected")                \
  XX(CLOSED_CONNECTION,                                              \
     "data received after completed connection: close message")      \
  XX(INVALID_VERSION, "invalid HTTP version")                        \
  XX(INVALID_STATUS, "invalid HTTP status code")                     \
  XX(INVALID_METHOD, "invalid HTTP method")                          \
  XX(INVALID_URL, "invalid URL")                                     \
  XX(INVALID_HOST, "invalid host")                                   \
  XX(INVALID_PORT, "invalid port")                                   \
  XX(INVALID_PATH, "invalid path")                                   \
  XX(INVALID_QUERY_STRING, "invalid query string")                   \
  XX(INVALID_FRAGMENT, "invalid fragment")                           \
  XX(LF_EXPECTED, "LF character expected")                           \
  XX(INVALID_HEADER_TOKEN, "invalid character in header")            \
  XX(INVALID_CONTENT_LENGTH,                                         \
     "invalid character in content-length header")                   \
  XX(INVALID_CHUNK_SIZE,                                             \
     "invalid character in chunk size header")                       \
  XX(INVALID_CONSTANT, "invalid constant string")                    \
  XX(INVALID_INTERNAL_STATE, "encountered unexpected internal state")\
  XX(STRICT, "strict mode assertion failed")                         \
  XX(PAUSED, "parser is paused")                                     \
  XX(UNKNOWN, "an unknown error occurred")                           \
  /* http server inner errors */                                           \
  XX(NO_HANDLE_FUNC, "no handle function for cgi call back")         \
  XX(MALLOC_FAIL, "system alloc space fail")                         \
  XX(FILE_NOT_EXIST, "file does not exist")                          \
  XX(READ_FILE_FAIL, "read common file fail")                        \
  XX(RECV_PACKET_FAIL, "recv http packet fail")                      \
  XX(RECOURCE_NOT_MODIFIED, "server resource not modified")			 \
  XX(ERRNO_END, "errno end")


#define HTTP_SUCCESS_MAP(XX)                                         \
	XX(DOWNLOAD_PRE_PROCESS, "download file pre-process success")    \

/* Define HPE_* values for each errno value above */
#define HTTP_ERRNO_GEN(n, s) HPE_##n,
enum http_errno
{
  HPE_OK = 0,

  //http server inner error code, must be minus
  HPE_ERRNO_BEGIN = 100000,
  HTTP_ERRNO_MAP(HTTP_ERRNO_GEN)

  HPE_SUCCESS_BEGIN = 200000,
  HTTP_SUCCESS_MAP(HTTP_ERRNO_GEN)
};
#undef HTTP_ERRNO_GEN

/*enum type for free struct hpb*/
enum
{
	HPB_CLOSE_SOCKET_ONLY = -2,
	HPB_FREE_BY_CGI_CONTROL = -1,
	HPB_FREE_BY_TIME_CONTROL = 0,
	HPB_REALLY_FREE = 1
};

/*enum type for timewait function*/
typedef enum
{
    WF_READ,
    WF_WRITE
}WAIT_EVENT;

/*fd ??*/
enum
{
	CONN_STATUS_ACCEPT = 0,
	CONN_STATUS_HANDLE
};


/* function for const*/
extern int error_code_mapping(int ret);

/* macro */

/**
	common macro
*/
#define MAX_THREAD_NUM            2
#define MAX_RETRY_TIMES           5
#define MIN_EXPIRE_TIME           60
#define MAX_EXPIRE_TIME           300
#define DEFAULT_BACKLOG           10
#define SELECT_TIMEOUT            200000
#define PACKET_LENGTH             4096
#define CGI_CACHE_TIME            60 * 60
#define READ_FILE_CACHE_TIME      24 * CGI_CACHE_TIME
#define DOWN_FILE_CACHE_TIME      365 * READ_FILE_CACHE_TIME
#define DEFAULT_HTML              "/index.html"

/* Maximium header size allowed. If the macro is not defined
 * before including this header then the default is used. To
 * change the maximum header size, define the macro in the build
 * environment (e.g. -DHTTP_MAX_HEADER_SIZE=<value>). To remove
 * the effective limit on the size of the header, define the macro
 * to a very large number (e.g. -DHTTP_MAX_HEADER_SIZE=0x7fffffff)
 */
#ifndef HTTP_MAX_HEADER_SIZE
# define HTTP_MAX_HEADER_SIZE (8*1024)
#endif


/**
	free space macro
*/
#define FREE_SPACE(X) \
do{                   \
	if (X)            \
	{                 \
		free(X);      \
		(X) = 0;      \
	}                 \
}while (0)

/**
	free struct list_head
*/
#define FREE_LIST_HEAD(pos, head, type, member, callfunc)   \
do{                                                         \
	type* x;                                                \
	pos = (head)->next;                                     \
	while (pos && pos != (head)) {					        \
		x = list_entry(pos, type, member);			        \
		pos = pos->next;							        \
		list_del(&(x->member)); 					        \
		callfunc(x);								        \
		FREE_SPACE(x);								        \
	}												        \
	INIT_LIST_HEAD(head);                                   \
}while (0)

/**
	debug macro
*/

#define PRINT_ERROR(X) do {printf X;}while (0)
#define PRINT_ERROR_RETURN(X, y) \
	do {printf X; return (y);}while (0)

#define PRINT_INFO(X) do {printf X;}while (0)

//#define DEBUG_MACRO
#ifdef DEBUG_MACRO
	#define PRINT_DEBUG(X) do {printf X;}while (0)
#else
	#define PRINT_DEBUG(X) do {}while (0)
#endif

#ifdef DEBUG_MACRO
#define INLINE
#else
#define INLINE inline
#endif


#endif //HTTP_CONST_H
