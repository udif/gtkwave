/* 
 * Copyright (c) Tony Bybell 1999-2010.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"

#ifndef VCD_H
#define VCD_H

#include <stdio.h> 
#include <stdlib.h>

#ifndef _MSC_VER
#include <unistd.h>
#endif

#ifndef HAVE_FSEEKO
#define fseeko fseek
#define ftello ftell
#endif

#include <setjmp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include "symbol.h"
#include "wavealloca.h"
#include "debug.h"
#include "tree.h"

#define VCD_SIZE_WARN (256)	/* number of MB size where converter warning message appears */
#define VCD_BSIZ 32768	/* size of getch() emulation buffer--this val should be ok */
#define VCD_INDEXSIZ  (8 * 1024 * 1024)

#define vcd_exit(x) \
	if(GLOBALS->vcd_jmp_buf) \
		{ \
		longjmp(*(GLOBALS->vcd_jmp_buf), x); \
		} \
		else \
		{ \
		exit(x); \
		}

enum VCDName_ByteSubstitutions { VCDNAM_NULL=0, 
#ifdef WAVE_HIERFIX
VCDNAM_HIERSORT, 
#endif
VCDNAM_ESCAPE };

/* fix for contrib/rtlbrowse */
#ifndef VLEX_DEFINES_H
enum VarTypes { V_EVENT, V_PARAMETER,
                V_INTEGER, V_REAL, V_REAL_PARAMETER=V_REAL, V_REALTIME=V_REAL, V_REG, V_SUPPLY0,
                V_SUPPLY1, V_TIME, V_TRI, V_TRIAND, V_TRIOR,
                V_TRIREG, V_TRI0, V_TRI1, V_WAND, V_WIRE, V_WOR, V_PORT, V_IN=V_PORT, V_OUT=V_PORT, V_INOUT=V_PORT, V_STRINGTYPE,
                V_END, V_LB, V_COLON, V_RB, V_STRING };
#endif

/* for vcd_recoder.c */
enum FastloadState  { VCD_FSL_NONE, VCD_FSL_WRITE, VCD_FSL_READ };

TimeType vcd_main(char *fname);
TimeType vcd_recoder_main(char *fname);

TimeType vcd_partial_main(char *fname);
void kick_partial_vcd(void);


struct sym_chain
{
struct sym_chain *next;
struct symbol *val;
};

struct slist
{  
struct slist *next;
char *str;
struct tree *mod_tree_parent;
int len;
};


#ifdef WAVE_USE_STRUCT_PACKING
#pragma pack(push)
#pragma pack(1)
#endif

struct vcdsymbol
{
struct vcdsymbol *root, *chain;
struct symbol *sym_chain;

struct vcdsymbol *next;
char *name;
char *id;
char *value;
struct Node **narray;
hptr *tr_array;   /* points to synthesized trailers (which can move) */
hptr *app_array;   /* points to hptr to append to (which can move) */

unsigned int nid;
int msi, lsi;
int size;

unsigned char vartype;
};

#ifdef WAVE_USE_STRUCT_PACKING
#pragma pack(pop)
#endif


char *build_slisthier(void);
void append_vcd_slisthier(char *str);

struct HistEnt *histent_calloc(void);
void strcpy_vcdalt(char *too, char *from, char delim);
int strcpy_delimfix(char *too, char *from);
void vcd_sortfacs(void);
void set_vcd_vartype(struct vcdsymbol *v, nptr n);

void vcd_import_masked(void);
void vcd_set_fac_process_mask(nptr np);
void import_vcd_trace(nptr np);

int vcd_keyword_code(const char *s, unsigned int len);

#endif

/*
 * $Id: vcd.h,v 1.11 2010/12/17 06:29:20 gtkwave Exp $
 * $Log: vcd.h,v $
 * Revision 1.11  2010/12/17 06:29:20  gtkwave
 * Added --enable-struct-pack configure flag
 *
 * Revision 1.10  2010/12/12 18:32:45  gtkwave
 * add "string" variable type to parsing of vcd variable declarations
 *
 * Revision 1.9  2010/03/01 19:19:50  gtkwave
 * more hier_pfx code movement into hierpack.c
 *
 * Revision 1.8  2010/02/22 21:13:36  gtkwave
 * added "realtime" VCD variable
 *
 * Revision 1.7  2010/01/23 03:21:11  gtkwave
 * hierarchy fixes when characters < "." are in the signal names
 *
 * Revision 1.6  2009/07/02 18:50:47  gtkwave
 * decorate VCD module trees with type info, add move to front to buildname
 *
 * Revision 1.5  2009/06/25 18:31:19  gtkwave
 * added event types for VCD+FST and impulse arrows on event types
 *
 * Revision 1.4  2009/03/05 16:17:56  gtkwave
 * added fastload option
 *
 * Revision 1.3  2008/07/18 17:27:01  gtkwave
 * adding hierpack code
 *
 * Revision 1.2  2007/08/26 21:35:46  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.3  2007/08/25 19:43:46  gtkwave
 * header cleanups
 *
 * Revision 1.1.1.1.2.2  2007/08/24 21:23:55  gtkwave
 * use setjump + potential backout ctx to "harden" vcd reload error recovery
 *
 * Revision 1.1.1.1.2.1  2007/08/05 02:27:28  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1  2007/05/30 04:27:30  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:17  gtkwave
 * initial release
 *
 */

