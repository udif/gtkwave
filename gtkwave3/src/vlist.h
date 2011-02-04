/* 
 * Copyright (c) Tony Bybell 2006-8.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"

#ifndef WAVE_VLIST_H
#define WAVE_VLIST_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"

struct vlist_t
{
struct vlist_t *next;
unsigned int siz;
unsigned int offs;
unsigned int elem_siz;
};


/* experimentation shows that 255 is one of the least common 
   bytes found in recoded value change streams */
#define WAVE_ZIVFLAG (0xff) 

#define WAVE_ZIVWRAP (1<<7) 		  /* must be power of two because of AND mask */
#define WAVE_ZIVSRCH (WAVE_ZIVWRAP)	  /* search depth in bytes */
#define WAVE_ZIVSKIP (1)		  /* number of bytes to skip for alternate rollover searches */
#define WAVE_ZIVMASK ((WAVE_ZIVWRAP) - 1) /* then this becomes an AND mask for wrapping */

struct vlist_packer_t
{
struct vlist_t *v;

unsigned char buf[WAVE_ZIVWRAP];

#ifdef WAVE_VLIST_PACKER_STATS
unsigned int packed_bytes;
#endif
unsigned int unpacked_bytes;
unsigned int repcnt, repcnt2, repcnt3, repcnt4;

unsigned char bufpnt;
unsigned char repdist, repdist2, repdist3, repdist4;
};


void vlist_init_spillfile(void);
void vlist_kill_spillfile(void);

struct vlist_t *vlist_create(unsigned int elem_siz);
void vlist_destroy(struct vlist_t *v);
void *vlist_alloc(struct vlist_t **v, int compressable);
unsigned int vlist_size(struct vlist_t *v);
void *vlist_locate(struct vlist_t *v, unsigned int idx);
void vlist_freeze(struct vlist_t **v);
void vlist_uncompress(struct vlist_t **v);

struct vlist_packer_t *vlist_packer_create(void);
void vlist_packer_alloc(struct vlist_packer_t *v, unsigned char ch);
void vlist_packer_finalize(struct vlist_packer_t *v);
unsigned char *vlist_packer_decompress(struct vlist_t *vl, unsigned int *declen);
void vlist_packer_decompress_destroy(char *mem);

#endif

/*
 * $Id: vlist.h,v 1.7 2007/12/24 19:56:03 gtkwave Exp $
 * $Log: vlist.h,v $
 * Revision 1.7  2007/12/24 19:56:03  gtkwave
 * preparing for 3.1.2 version bump
 *
 * Revision 1.6  2007/12/18 03:49:51  gtkwave
 * fixed header typo (ifdef'd out so missed)
 *
 * Revision 1.5  2007/12/17 03:22:44  gtkwave
 * integration of (currently unused) vlist_packer routines
 *
 * Revision 1.4  2007/12/06 04:16:20  gtkwave
 * removed non-growable vlists
 *
 * Revision 1.3  2007/11/30 01:31:23  gtkwave
 * added vlist memory spill to disk code + fixed vcdload status bar on > 2GB
 *
 * Revision 1.2  2007/08/26 21:35:46  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.2  2007/08/25 19:43:46  gtkwave
 * header cleanups
 *
 * Revision 1.1.1.1.2.1  2007/08/05 02:27:28  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1  2007/05/30 04:27:22  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:18  gtkwave
 * initial release
 *
 */

