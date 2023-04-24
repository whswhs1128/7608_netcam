/*!
*****************************************************************************
** \file      $gkprjbits.h
**
** \version	$id: bits.h 15-08-04  8月:08:1438655467 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#ifndef __RECORD_BITS_H__
#define __RECORD_BITS_H__

#include <stdint.h>

typedef struct JBits JBits;

JBits *j_bits_alloc(uint32_t length);
int j_bits_free(JBits *jb);

int j_bits_set(JBits *jb, uint32_t n);
int j_bits_clr(JBits *jb, uint32_t n);

int j_bits_get(JBits *jb, uint32_t n);

int j_bits_reset(JBits *jb);

#endif /* __RECORD_BITS_H__ */
