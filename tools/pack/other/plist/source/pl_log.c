/*
****************************************************************************
** \file      /home/amik/GokeFiles/smartlinkCapture/sl_log.c
**
** \version   $Id: sl_log.c 4 2016-02-01 09:30:58Z 	dengbiao $
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

#include <stdio.h>
#include <stdarg.h>
#include <errno.h>

#define DISABLE_LOG
#include "pl_log.h"
#undef  DISABLE_LOG
#include "pl_types.h"

//log private member.
typedef struct LogPrivate_s {
    FILE        *pOutFile;
    unsigned int   level;
} LogPrivateT;

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
    LogPrivateT     private;
} LogT;

extern LogT Log;

static int FAST_FUNC WriteLog(const char *head, const char *tail, const char * format, va_list arg_ptr)
{
    int nWrittenBytes = 0;
    if(unlikely(Log.private.pOutFile == NULL))
        Log.private.pOutFile = stderr;
    nWrittenBytes = fprintf(Log.private.pOutFile, "%s", head);
    nWrittenBytes += vfprintf(Log.private.pOutFile, format, arg_ptr);
    nWrittenBytes += fprintf(Log.private.pOutFile, "%s\n", tail);

    return nWrittenBytes;
}

static int LogInfo(const char * format, ...)
{
    if(Log.private.level >= LOG_LEVEL_INFO) {
        int nRet = 0;
        va_list arg_ptr;
        va_start(arg_ptr, format);
        nRet = WriteLog("\033[0;32m[Info] ", "\033[0;0m", format, arg_ptr);
        va_end(arg_ptr);
        return nRet;
    }
    else
        return 0;
}

static int LogWarn(const char * format, ...)
{
    if(Log.private.level >= LOG_LEVEL_WARN) {
        int nRet = 0;
        va_list arg_ptr;
        va_start(arg_ptr, format);
		nRet = WriteLog("\033[0;33m[Warn] ", "\033[0;0m", format, arg_ptr);
        va_end(arg_ptr);
        return nRet;
    }
    else
        return 0;

}

static int LogError(const char * format, ...)
{
    if(Log.private.level >= LOG_LEVEL_ERROR) {
        int nRet = 0;
        va_list arg_ptr;
        va_start(arg_ptr, format);
		nRet = WriteLog("\033[0;31m[Erro] ", "\033[0;0m", format, arg_ptr);
        va_end(arg_ptr);
        return nRet;
    }
    else
        return 0;

}

static int LogPerror(const char * format, ...)
{
    if(Log.private.level >= LOG_LEVEL_ERROR) {
        int nRet = 0;
        va_list arg_ptr;
        va_start(arg_ptr, format);
		if(unlikely(Log.private.pOutFile == NULL))
        	Log.private.pOutFile = stdout;
	    nRet = fprintf(Log.private.pOutFile, "\033[0;31m[Perr] ");
	    nRet += vfprintf(Log.private.pOutFile, format, arg_ptr);
	    nRet += fprintf(Log.private.pOutFile, "%s\033[0;0m\n", strerror(errno));
        va_end(arg_ptr);
        return nRet;
    }
    else
        return 0;

}

static void FAST_FUNC OpenOutFile(FILE * pfile)
{
    Log.private.pOutFile = pfile;
}

static FILE* FAST_FUNC CloseOutFile(void)
{
    FILE *file = Log.private.pOutFile;
    Log.private.pOutFile = stdout;
    return file;
}

static void FAST_FUNC SetOutLevel(unsigned int level)
{
    Log.private.level = level;
}

static unsigned int FAST_FUNC GetOutLevel(void)
{
    return Log.private.level;
}

static void FAST_FUNC On(void)
{
    Log.private.level = LOG_LEVEL_ERROR;
}

static void FAST_FUNC Off(void)
{
    Log.private.level = LOG_LEVEL_EMPTY;
}

LogT Log = {
    .i = LogInfo,
    .w = LogWarn,
    .e = LogError,
    .p = LogPerror,
    .on = On,
    .off = Off,
    .OpenOutFile = OpenOutFile,
    .CloseOutFile = CloseOutFile,
    .SetOutLevel = SetOutLevel,
    .GetOutLevel = GetOutLevel,
    .private.pOutFile = NULL,
    .private.level = LOG_LEVEL_ERROR,
};
