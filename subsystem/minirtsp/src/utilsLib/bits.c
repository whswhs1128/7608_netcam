/*!
*****************************************************************************
** \file      $gkprjbits.c
**
** \version	$id: bits.c 15-08-04  8月:08:1438655319 
**
** \brief       
**
** \attention   this code is provided as is. goke microelectronics 
		accepts no responsibility or liability for any errors
		orommissions
** (c) copyright 2015 by He yongbing co.,ltd
*****************************************************************************
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "mem.h"
#include "bits.h"

#define BITS_PER_BYTE	(8)
#define BYTE_PER_WORD	(sizeof(uint32_t))
#define BITS_PER_WORD	(BITS_PER_BYTE * BYTE_PER_WORD)
#define WORD_PER_SLOT	(32)
#define BYTE_PER_SLOT	(BYTE_PER_WORD * WORD_PER_SLOT)
#define BITS_PER_SLOT	(BITS_PER_BYTE * BYTE_PER_SLOT)

struct JBits {
	uint32_t length;
	uint32_t nslot;
	uint32_t **slots;
};

JBits *j_bits_alloc(uint32_t length)
{
	if (length == 0)
		return NULL;

	JBits *jb = j_mallocz(sizeof(JBits));
	if (!jb)
		return NULL;

	jb->length = length;
	jb->nslot = (length + BITS_PER_SLOT - 1) / BITS_PER_SLOT;

	return jb;
}

int j_bits_free(JBits *jb)
{
	if (!jb)
		return -1;

	if (jb->slots) {
		int i;
		for (i = 0; i < jb->nslot; i++) {
			if (jb->slots[i]) j_free(jb->slots[i]);
		}
		j_free(jb->slots);
	}

	j_free(jb);

	return 0;
}

int j_bits_set(JBits *jb, uint32_t n)
{
	if (!jb || (n >= jb->length))
		return -1;

	int slot = n / BITS_PER_SLOT;
	int bits = n % BITS_PER_SLOT;
	int word = bits / BITS_PER_WORD;
	int offs = bits % BITS_PER_WORD;

	if (!jb->slots) {
		jb->slots = j_mallocz(jb->nslot * sizeof(jb->slots[0]));
		if (!jb->slots)
			return -1;
	}

	if (!jb->slots[slot])
		jb->slots[slot] = j_mallocz(BYTE_PER_SLOT);

	if (!jb->slots[slot])
		return -1;

	uint32_t *bitset = jb->slots[slot];
	bitset += word;
	*bitset |= (1 << offs);

	return 0;
}

int j_bits_reset(JBits *jb)
{
	if (!jb)
		return -1;

	if (jb->slots) {
		int i;
		for (i = 0; i < jb->nslot; i++) {
			if (jb->slots[i]) j_free(jb->slots[i]);
		}
		j_free(jb->slots);
		jb->slots = NULL;
	}

	return 0;
}

int j_bits_clr(JBits *jb, uint32_t n)
{
	if (!jb || (n >= jb->length))
		return -1;

	int slot = n / BITS_PER_SLOT;
	int bits = n % BITS_PER_SLOT;
	int word = bits / BITS_PER_WORD;
	int offs = bits % BITS_PER_WORD;

	if (!jb->slots || !jb->slots[slot])
		return 0;

	uint32_t *bitset = jb->slots[slot];
	bitset += word;
	*bitset &= ~(1 << offs);

	return 0;
}

int j_bits_get(JBits *jb, uint32_t n)
{
	if (!jb || (n >= jb->length))
		return -1;

	int slot = n / BITS_PER_SLOT;
	int bits = n % BITS_PER_SLOT;
	int word = bits / BITS_PER_WORD;
	int offs = bits % BITS_PER_WORD;

	if (!jb->slots || !jb->slots[slot])
		return 0;

	uint32_t *bitset = jb->slots[slot];
	bitset += word;
	uint32_t val = *bitset & (1 << offs);

	return !!val;
}

//#define TEST

#ifdef TEST

#define LENGTH	(2048)

int main(void)
{
	JBits *jb = j_bits_alloc(LENGTH);
	if (!jb)
		return -1;

#if 0
	int i;
	for (i = 0; i < LENGTH; i++) {
		j_bits_set(jb, i);
	}

	for (i = 0; i < LENGTH; i++) {
		printf("%d: %d\n", i, j_bits_get(jb, i));
	}

	for (i = 0; i < LENGTH; i += 2) {
		j_bits_clr(jb, i);
	}

	for (i = 0; i < LENGTH; i++) {
		printf("%d: %d\n", i, j_bits_get(jb, i));
	}
#endif

	j_bits_free(jb);
	return 0;
}
#endif
