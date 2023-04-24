/*
****************************************************************************
** \file      /home/amik/GokeFiles/smartlinkCapture/include/sl_log.h
**
** \version   $Id: sl_log.h 4 2016-02-01 09:32:37Z 	dengbiao $
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

#ifndef     __PL_LOG_H__
#define     __PL_LOG_H__

#include <stdio.h>

typedef int (*LogS)(const char *, ...);
typedef void (*LogOpenOutFile)(FILE *);
typedef void (*LogSetOutLevel)(unsigned int);
typedef FILE *(*LogCloseOutFile)(void);
typedef unsigned int (*LogGetOutLevel)(void);
typedef void (*LogOpera)(void);

#define LOG_LEVEL_EMPTY     0
#define LOG_LEVEL_INFO     1
#define LOG_LEVEL_WARN      2
#define LOG_LEVEL_ERROR     3

#ifndef DISABLE_LOG
typedef struct Log_s {
    //log public member.
    LogS i;
    LogS w;
    LogS e;
    LogS p;
    LogOpera on;
    LogOpera off;
    LogOpenOutFile  OpenOutFile;
    LogCloseOutFile CloseOutFile;
    LogSetOutLevel  SetOutLevel;
    LogGetOutLevel  GetOutLevel;
} LogT;

extern LogT Log;

#define LogI(format, ...) Log.i("%s:%d " format, __func__, __LINE__, ##__VA_ARGS__)
#define LogW(format, ...) Log.w("%s:%d " format, __func__, __LINE__, ##__VA_ARGS__)
#define LogE(format, ...) Log.e("%s:%d " format, __func__, __LINE__, ##__VA_ARGS__)
#define LogP(format, ...) Log.p("%s:%d " format, __func__, __LINE__, ##__VA_ARGS__)
#endif

#ifdef __cplusplus
extern "C" {
#endif

#ifdef __cplusplus
}
#endif
#endif
