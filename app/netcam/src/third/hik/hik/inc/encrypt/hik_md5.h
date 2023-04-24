/* HIK_MD5.H - header file for HIK_MD5C.C
 */

/* Copyright (C) 1991-2, RSA Data Security, Inc. Created 1991. All
rights reserved.

License to copy and use this software is granted provided that it
is identified as the "RSA Data Security, Inc. HIK_MD5 Message-Digest
Algorithm" in all material mentioning or referencing this software
or this function.

License is also granted to make and use derivative works provided
that such works are identified as "derived from the RSA Data
Security, Inc. HIK_MD5 Message-Digest Algorithm" in all material
mentioning or referencing the derived work.

RSA Data Security, Inc. makes no representations concerning either
the merchantability of this software or the suitability of this
software for any particular purpose. It is provided "as is"
without express or implied warranty of any kind.

These notices must be retained in any copies of any part of this
documentation and/or software.
 */

#ifndef _HIK_MD5_H_
#define _HIK_MD5_H_

#ifdef __cplusplus
extern "C"
{
#endif

/* modified for oSIP: GCC supports this feature */
#define PROTOTYPES 1

#ifndef PROTOTYPES
#define PROTOTYPES 0
#endif

/* POINTER defines a generic pointer type */
typedef unsigned char *POINTER;

/* UINT2 defines a two byte word */
typedef unsigned short int UINT2;

/* UINT4 defines a four byte word */
typedef unsigned int UINT4;

/**
 * Structure for holding HIK_MD5 context.
 * @var HIK_MD5_CTX
 */
typedef struct
{
    UINT4 state[4];             /* state (ABCD) */
    UINT4 count[2];             /* number of bits, modulo 2^64 (lsb first) */
    unsigned char buffer[64];   /* input buffer */
} HIK_MD5_CTX;

void HIK_MD5Init(HIK_MD5_CTX *);
void HIK_MD5Update(HIK_MD5_CTX *, unsigned char *, unsigned int);
void HIK_MD5Final(unsigned char[16], HIK_MD5_CTX *);
  
/* add this by xiemq */
extern void HIK_MD5String(char *in, int in_len, unsigned char *out);
extern void HIK_MD5toStr(unsigned char *src, int len, char *out);

#ifdef __cplusplus
}
#endif

#endif

