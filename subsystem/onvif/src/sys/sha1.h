/*!
*****************************************************************************
** \file        solution/software/subsystem/onvif-no-gsoap/src/sys/sha1.h
**
** \brief       sha1
**
** \attention   THIS SAMPLE CODE IS PROVIDED AS IS. GOKE MICROELECTRONICS
**              ACCEPTS NO RESPONSIBILITY OR LIABILITY FOR ANY ERRORS OR
**              OMMISSIONS
**
** (C) Copyright 2012-2013 by GOKE MICROELECTRONICS CO.,LTD
**
*****************************************************************************
*/



#ifndef _SHA1_H_
#define _SHA1_H_

#include "onvif_std.h"

typedef struct
{
  GK_U32 total[2];
  GK_U32 state[5];
  GK_U8 buffer[64];
} sha1_context;

#ifdef __cplusplus
extern "C" {
#endif

void sha1_starts(sha1_context *ctx);
void sha1_update(sha1_context *ctx, GK_U8 *input, GK_U32 length);
void sha1_finish(sha1_context *ctx, GK_U8 digest[20]);


#ifdef __cplusplus
}
#endif

#endif

