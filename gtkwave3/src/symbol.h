/* 
 * Copyright (c) Tony Bybell 1999-2011.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"

#ifndef WAVE_SYMBOL_H
#define WAVE_SYMBOL_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "wavealloca.h"
#include "analyzer.h"
#include "currenttime.h"
#include "tree.h"
#include "debug.h"

#define SYMPRIME 500009
#define WAVE_DECOMPRESSOR "gzip -cd "	/* zcat alone doesn't cut it for AIX */

#ifndef _MSC_VER
#include <unistd.h>
#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif
#else
typedef long off_t;
#include <windows.h>
#include <io.h>
#endif

#ifdef WAVE_USE_STRUCT_PACKING
#pragma pack(push)
#pragma pack(1)
#endif

struct fac
{
struct Node *working_node;
int node_alias;
int len;
unsigned int flags;
};

#ifdef WAVE_USE_STRUCT_PACKING
#pragma pack(pop)
#endif


struct symbol
{
#ifndef _WAVE_HAVE_JUDY
struct symbol *sym_next;  /* for hash chain, judy uses sym_judy in globals */
#endif

struct symbol *vec_root, *vec_chain;
char *name;
struct Node *n;

#ifndef _WAVE_HAVE_JUDY
char s_selected;		/* for the clist object */
#endif
};


struct symchain		/* for restoring state of ->selected in signal regex search */
{
struct symchain *next;
struct symbol *symbol;
};


struct string_chain_t
{
struct string_chain_t *next;
char *str;
};


/* hash create/destroy */
void sym_hash_initialize(void *g);
void sym_hash_destroy(void *g);


struct symbol *symfind(char *, unsigned int *);
struct symbol *symadd(char *, int);
struct symbol *symadd_name_exists(char *name, int hv);
int hash(char *s);

/* typically use zero for hashval as it doesn't matter if facs are sorted as symfind will bsearch... */
#define symadd_name_exists_sym_exists(s, nam, hv) \
(s)->name = (nam); /* (s)->sym_next=GLOBALS->sym_hash[(hv)]; GLOBALS->sym_hash[(hv)]=(s); (obsolete) */

void facsplit(char *, int *, int *);
int sigcmp(char *, char *);
void quicksort(struct symbol **, int, int);

void wave_heapsort(struct symbol **a, int num);

struct Bits *makevec(char *, char *);
struct Bits *makevec_annotated(char *, char *);
int maketraces(char *, char *, int);

int parsewavline(char *, char *, int);
int parsewavline_lx2(char *, char *, int);


/* additions to bitvec.c because of search.c/menu.c ==> formerly in analyzer.h */
bvptr bits2vector(struct Bits *b);
struct Bits *makevec_selected(char *vec, int numrows, char direction);
int add_vector_selected(char *alias, int numrows, char direction);
struct Bits *makevec_range(char *vec, int lo, int hi, char direction);
int add_vector_range(char *alias, int lo, int hi, char direction);
struct Bits *makevec_chain(char *vec, struct symbol *sym, int len);
int add_vector_chain(struct symbol *s, int len);
char *makename_chain(struct symbol *sym);

/* splash screen activation (version >= GTK2 only) */
void splash_create(void);
void splash_sync(off_t current, off_t total);  

/* accessor functions for sym->selected moved (potentially) to sparse array */
char get_s_selected(struct symbol *s);
char set_s_selected(struct symbol *s, char value);
void destroy_s_selected(void);

#endif

/*
 * $Id: symbol.h,v 1.16 2011/01/07 20:17:10 gtkwave Exp $
 * $Log: symbol.h,v $
 * Revision 1.16  2011/01/07 20:17:10  gtkwave
 * remove redundant fields from struct fac
 *
 * Revision 1.15  2010/12/17 06:29:20  gtkwave
 * Added --enable-struct-pack configure flag
 *
 * Revision 1.14  2010/04/27 23:10:56  gtkwave
 * made inttype.h inclusion conditional
 *
 * Revision 1.13  2010/03/16 21:01:10  gtkwave
 * remove selected member of struct symbol
 *
 * Revision 1.12  2010/03/15 15:57:28  gtkwave
 * only allocate hash when necessary
 *
 * Revision 1.11  2010/03/15 03:14:53  gtkwave
 * deallocated symbol hash table after no longer needed
 *
 * Revision 1.10  2010/03/14 20:12:28  gtkwave
 * rename next hash pointer in struct symbol
 *
 * Revision 1.9  2010/03/13 19:48:53  gtkwave
 * remove nextinaet field and replace with temp symchain
 *
 * Revision 1.8  2010/03/13 07:56:41  gtkwave
 * removed unused h field in struct symbol
 *
 * Revision 1.7  2010/03/12 21:12:39  gtkwave
 * removed lastchange field
 *
 * Revision 1.6  2010/03/12 20:37:15  gtkwave
 * removed resolve_lxt_alias_to field from struct fac
 *
 * Revision 1.5  2010/03/11 23:31:52  gtkwave
 * remove name field from struct fac
 *
 * Revision 1.4  2009/09/14 03:00:08  gtkwave
 * bluespec code integration
 *
 * Revision 1.3  2008/09/26 17:05:10  gtkwave
 * force open tree nodes in ctree on initial .sav file read (didn't happen
 * before as ctree was not built yet)
 *
 * Revision 1.2  2007/08/26 21:35:45  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.3  2007/08/25 19:43:46  gtkwave
 * header cleanups
 *
 * Revision 1.1.1.1.2.2  2007/08/07 03:18:55  kermin
 * Changed to pointer based GLOBAL structure and added initialization function
 *
 * Revision 1.1.1.1.2.1  2007/08/05 02:27:24  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1  2007/05/30 04:27:55  gtkwave
 * Imported sources
 *
 * Revision 1.3  2007/05/28 00:55:06  gtkwave
 * added support for arrays as a first class dumpfile datatype
 *
 * Revision 1.2  2007/04/20 02:08:17  gtkwave
 * initial release
 *
 */

