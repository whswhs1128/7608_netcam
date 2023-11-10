
#ifndef __LOG_DEBUG_H__
#define __LOG_DEBUG_H__
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <stdint.h>
#include <stdbool.h>
#include <libgen.h>

extern const char* LOG_strerror(uint32_t errno);

#if LOG_DEBUG
#define LOG_SYNTAX "1;32"
#define LOG_TRACE(fmt, arg...) \
	do{\
		const char* const bname = __FILE__;\
		printf("\033["LOG_SYNTAX"m[%12s:%4d]\033[0m ", bname, __LINE__);\
		printf(fmt, ##arg);\
		printf("\r\n");\
	}while(0)

#define LOG_ASSERT(exp, fmt, arg...) \
	do{\
		if(!(exp)){\
			const char* const bname = __FILE__;\
			printf("\033["LOG_SYNTAX"m[%12s:%4d]\033[0m assert(\"%s\") ", bname, __LINE__, #exp);\
			printf(fmt, ##arg);\
			printf("\r\n");\
		}\
	}while(0)

#define LOG_CHECK(exp, fmt...) \
	do{\
		uint32_t ret = exp;\
		if(GK_SUCCESS != ret){\
			const char* const bname = __FILE__;\
			printf("\033["LOG_SYNTAX"m");\
			printf("%s @ [%s: %d] err: 0x%08x <%s>", #exp, bname, __LINE__, ret, LOG_strerror(ret));\
			printf("\033[0m\r\n");\
		}\
	}while(0)

#else
#define LOG_TRACE(fmt...)
#define LOG_ASSERT(exp, fmt, arg...)
#define LOG_CHECK(exp, fmt...)
#endif
/*=====================================*/


#define LOG_TRACE_LEVEL (7)
#define __LOG_TRACE(syntax,fmt,arg...) \
	do{\
		char bname[128];\
		strncpy(bname, __FILE__, sizeof(bname));\
		printf("\033[%sm[%16s:%4d]\033[0m "fmt"\r\n", syntax, basename(bname), __LINE__, ##arg);\
	}while(0)



#if (LOG_TRACE_LEVEL==0)
	#define LOG_EMERG(fmt,arg...) __LOG_TRACE("1;30",fmt,##arg)
#else
	#define LOG_EMERG(fmt,arg...)
#endif

#if (LOG_TRACE_LEVEL>=1)
	#define LOG_ALERT(fmt,arg...) __LOG_TRACE("1;31",fmt,##arg)
#else
	#define LOG_ALERT(fmt,arg...)
#endif

#if (LOG_TRACE_LEVEL>=2)
	#define LOG_CRIT(fmt,arg...) __LOG_TRACE("1;32",fmt,##arg)
#else
	#define LOG_CRIT(fmt,arg...)
#endif

#if (LOG_TRACE_LEVEL>=3)
	#define LOG_ERR(fmt,arg...) __LOG_TRACE("1;31",fmt,##arg)
#else
	#define LOG_ERR(fmt,arg...)
#endif

#if (LOG_TRACE_LEVEL>=4)
	#define LOG_WARNING(fmt,arg...) __LOG_TRACE("1;34",fmt,##arg)
#else
	#define LOG_WARNING(fmt,arg...)
#endif

#if (LOG_TRACE_LEVEL>=5)
	#define LOG_NOTICE(fmt,arg...) __LOG_TRACE("1;35",fmt,##arg)
#else
	#define LOG_NOTICE(fmt,arg...)
#endif

#if (LOG_TRACE_LEVEL>=6)
	#define LOG_INFO(fmt,arg...) __LOG_TRACE("1;36",fmt,##arg)
#else
	#define LOG_INFO(fmt,arg...)
#endif

#if (LOG_TRACE_LEVEL>=7)
	#define LOG_DEBUG(fmt,arg...) __LOG_TRACE("37",fmt,##arg)
#else
	#define LOG_DEBUG(fmt,arg...)
#endif




#ifdef __cplusplus
};
#endif
#endif //__LOG_DEBUG_H__

