#ifndef DAHA_CONFIG_H_
#define DAHA_CONFIG_H_


#ifndef _HAVE_TYPE_BYTE
#define _HAVE_TYPE_BYTE
	typedef unsigned char       BYTE;
#endif

#ifndef _HAVE_TYPE_Bool
#define _HAVE_TYPE_Bool
typedef unsigned char       Bool;
#endif

#ifndef BOOL
#ifndef _HAVE_TYPE_BOOL
#define _HAVE_TYPE_BOOL
	typedef int					BOOL;
#endif
#endif

#ifndef _HAVE_TYPE_WORD
#define _HAVE_TYPE_WORD
	typedef unsigned short      WORD;
#endif

#ifndef _HAVE_TYPE_DWORD
#define _HAVE_TYPE_DWORD
	typedef unsigned long		DWORD;
	typedef unsigned long		ULONG;
#endif

#ifndef _HAVE_TYPE_ULONGLONG
#define _HAVE_TYPE_ULONGLONG
	typedef unsigned long long		ULONGLONG;
#endif

#ifndef _HAVE_TYPE_LONG
#define _HAVE_TYPE_LONG
	typedef long				LONG;
#endif

#ifndef _HAVE_TYPE_UINT
#define _HAVE_TYPE_UINT
	typedef unsigned int		UINT;
#endif

#ifndef _HAVE_TYPE_VOID
#define _HAVE_TYPE_VOID
	typedef void				VOID;
#endif

#ifndef _HAVE_TYPE_DWORD64
#define _HAVE_TYPE_DWORD64
	typedef unsigned long long  DWORD64;
	typedef unsigned long long	QWORD;
#endif

#ifndef _HAVE_TYPE_UINT8
#define _HAVE_TYPE_UINT8
	typedef unsigned char   	UINT8;
#endif


#ifndef _HAVE_TYPE_UINT16
#define _HAVE_TYPE_UINT16
	typedef unsigned short 		UINT16;
#endif

#ifndef _HAVE_TYPE_UINT32
#define _HAVE_TYPE_UINT32
	typedef unsigned int    	UINT32;
#endif

#ifndef _HAVE_TYPE_HWND
#define _HAVE_TYPE_HWND
	typedef unsigned int 		HWND;
#endif

#ifndef _HAVE_TYPE_WPARAM
#define _HAVE_TYPE_WPARAM
	typedef unsigned int    	WPARAM;
#endif

#ifndef _HAVE_TYPE_LPARAM
#define _HAVE_TYPE_LPARAM
	typedef unsigned long   	LPARAM;
#endif

#ifndef _HAVE_TYPE_SINT64
#define _HAVE_TYPE_SINT64
	typedef signed long long	SINT64;
#endif

#ifndef _HAVE_TYPE_UINT64
#define _HAVE_TYPE_UINT64
	typedef unsigned long long 	UINT64;
#endif

#ifndef _HAVE_TYPE_Int
#define _HAVE_TYPE_Int
	typedef int		Int;
#endif

#ifndef _HAVE_TYPE_Char
#define _HAVE_TYPE_Char
	typedef char	Char;
#endif
#define SOCKET int
#define FALSE	0
#define TRUE	1

#endif