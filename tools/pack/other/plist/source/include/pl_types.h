/*
****************************************************************************
** \file      /home/amik/GokeFiles/smartlinkCapture/include/sl_types.h
**
** \version   $Id: sl_types.h 4 2016-02-01 09:33:17Z 	dengbiao $
**
** \brief     videc abstraction layer header file.
**
** \attention THIS SAMPLE CODE IS PROVIDED AS IS. GOFORTUNE SEMICONDUCTOR
**            ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**            OMMISSIONS.
**
** (C) Copyright 2015-2016 by GOKE MICROELECTRONICS CO.,LTD
**
****************************************************************************
*/

#ifndef     __PL_TYPES_H__
#define     __PL_TYPES_H__

#define     VERSIONS    1.0

/*################ define data types ####################*/
#ifndef s64
#define s64 	long long
#endif
#ifndef u64
#define u64 	unsigned long long
#endif
#ifndef s32
#define s32 	int
#endif
#ifndef u32
#define u32 	unsigned int
#endif
#ifndef s16
#define s16 	short
#endif
#ifndef u16
#define u16 	unsigned short
#endif
#ifndef s8
#define s8 		char
#endif
#ifndef u8
#define u8 		unsigned char
#endif
#ifndef bool
#define bool 	unsigned int
#endif

#ifndef true
#define true    1
#endif
#ifndef false
#define false   0
#endif

#ifndef likely
#define likely(x)	__builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x)	__builtin_expect(!!(x), 0)
#endif

#define isNull(n) 		((n) == NULL)
#define nMAX(n, m) 		((n)>(m)?(n):(m))
#define nMIN(n, m) 		((n)<(m)?(n):(m))
#define sMAX(n, m) 		(strcmp((n), (m))>0?(n):(m))
#define sMIN(n, m) 		(strcmp((n), (m))<0?(n):(m))
#define sl_clear(n)		(memset(&(n), 0, sizeof(typeof(n))))


#if defined __GNUC__ && defined __GNUC_MINOR__
# define __GNUC_PREREQ(maj, min) \
		((__GNUC__ << 16) + __GNUC_MINOR__ >= ((maj) << 16) + (min))
#else
# define __GNUC_PREREQ(maj, min) 0
#endif


#if __GNUC_PREREQ(3,0) && defined(i386) /* || defined(__x86_64__)? */
/* stdcall makes callee to pop arguments from stack, not caller */
# define FAST_FUNC __attribute__((regparm(3),stdcall))
/* #elif ... - add your favorite arch today! */
#else
# define FAST_FUNC
#endif


/**************error table******************/
#define NET_OK                      0
#define NET_ERR_BADPARAM            -1
#define NET_ERR_OUTOFMEMORY         -2
#define NET_ERR_ALREADINIT          -3
#define NET_ERR_NOTINIT             -4
#define NET_ERR_NOSUPPORT           -5
#define NET_ERR_TIMEOUT             -6
#define NET_ERR_DEVICEBUSY          -7
#define NET_ERR_INVALIDHANDLE       -8
#define NET_ERR_NOSUPPORT_VERSION   -9
#define NET_ERR_NOTOPEN             -10
#define NET_ERR_FROMDRIVER          -11

/******************error code base*******************/
#define NET_WIRELESS_BASE	-1000
#define NET_STA_BASE        -2000
#define NET_AP_BASE         -3000


/*################ define normal function ###############*/

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif
