/* 
 * Copyright (c) Tony Bybell 1999-2010.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef WAVE_DEBUG_H
#define WAVE_DEBUG_H

#include <gtk/gtk.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifndef _MSC_VER
#ifndef __MINGW32__
#include <stdint.h>
#else
#include <windows.h>
#endif
#endif

#define WAVE_MAX_CLIST_LENGTH 15000


/*
 * If you have problems viewing traces (mangled timevalues),
 * make sure that you use longs rather than the glib 64-bit
 * types...
 */
#ifdef G_HAVE_GINT64
	typedef gint64          TimeType; 
	typedef guint64         UTimeType;

        #ifndef _MSC_VER
                #define LLDescriptor(x) x##LL
                #define ULLDescriptor(x) x##ULL
		#ifdef __MINGW32__
			#define TTFormat "%I64d"
			#define UTTFormat "%I64u"
		#else
			#if __WORDSIZE == 64
		                #define TTFormat "%ld"
		                #define UTTFormat "%lu"
			#else
		                #define TTFormat "%lld"
		                #define UTTFormat "%llu"
			#endif
		#endif
        #else
                #define LLDescriptor(x) x##i64
                #define ULLDescriptor(x) x##i64
                #define TTFormat "%I64d"
                #define UTTFormat "%I64u"
        #endif

	#define WAVE_MINZOOM (LLDescriptor(-4000000000))
#else
	typedef long            TimeType; 
	typedef unsigned long   UTimeType;
	#define TTFormat "%d"
	#define UTTFormat "%u"

	#define LLDescriptor(x) x
	#define ULLDescriptor(x) x

	#define WAVE_MINZOOM (LLDescriptor(-20000000))
#endif


#ifdef _MSC_VER
	#define strncasecmp(s1, s2, n) _strnicmp(s1, s2, n)
	#define popen _popen
	#define pclose _pclose
#endif


#ifdef DEBUG_PRINTF
#define DEBUG(x) x
#else
#define DEBUG(x)
#endif


#ifdef DEBUG_MALLOC_LINES
void free_2(void *ptr, char *filename, int lineno);
#define free_2(x) free_2((x),__FILE__,__LINE__)

void *malloc_2(size_t size, char *filename, int lineno);
#define malloc_2(x) malloc_2((x),__FILE__,__LINE__)

void *realloc_2(void *ptr, size_t size, char *filename, int lineno);
#define realloc_2(x, y) realloc_2((x),(y),__FILE__,__LINE__)

void *calloc_2(size_t nmemb, size_t size, char *filename, int lineno);
#define calloc_2(x, y) calloc_2((x),(y),__FILE__,__LINE__)

#else
void free_2(void *ptr);
void *malloc_2(size_t size);
void *realloc_2(void *ptr, size_t size);
void *calloc_2(size_t nmemb, size_t size);
#endif


void free_outstanding(void);

char *strdup_2(const char *s);
char *strdup_2s(const char *s);

char *tmpnam_2(char *s, int *fd);  /* mimic functionality of tmpnam() */

TimeType atoi_64(const char *str);

void gtk_tooltips_set_tip_2(GtkTooltips *tooltips, GtkWidget *widget,
        const gchar *tip_text, const gchar *tip_private);
void gtk_tooltips_set_delay_2(GtkTooltips *tooltips, guint delay);
GtkTooltips* gtk_tooltips_new_2(void);



#undef WAVE_USE_SIGCMP_INFINITE_PRECISION  /* define this for slow sigcmp with infinite digit accuracy */
#define WAVE_OPT_SKIP 1			   /* make larger for more accel on traces */

/* for source code annotation helper app */

#ifndef PATH_MAX
#define PATH_MAX (4096)
#endif

#define WAVE_MATCHWORD "WAVE"
enum AnnotateAetType { WAVE_ANNO_NONE, WAVE_ANNO_AE2, WAVE_ANNO_VZT, WAVE_ANNO_LXT2, WAVE_ANNO_FST, WAVE_ANNO_MAX };


#if !defined _MSC_VER && !defined __MINGW32__

#include <sys/ipc.h>
#include <sys/shm.h>

struct gtkwave_annotate_ipc_t
{
char matchword[4];			/* match against WAVE_MATCHWORD string */
char time_string[40];			/* formatted marker time */

pid_t gtkwave_process;
pid_t browser_process;

TimeType marker;
unsigned marker_set : 1;
unsigned cygwin_remote_kill : 1;

int aet_type;
char aet_name[PATH_MAX+1];
char stems_name[PATH_MAX+1];
};

#else

struct gtkwave_annotate_ipc_t
{
char matchword[4];			/* match against WAVE_MATCHWORD string */
char time_string[40];			/* formatted marker time */

#ifdef __MINGW32__
HANDLE browser_process;
#endif

TimeType marker;
unsigned marker_set : 1;
unsigned cygwin_remote_kill : 1;

int aet_type;
char aet_name[PATH_MAX+1];
char stems_name[PATH_MAX+1];
};

#endif

#define DUAL_MATCHWORD "DUAL"

struct gtkwave_dual_ipc_t
{
char matchword[4];			/* match against DUAL_MATCHWORD string */

TimeType left_margin_time;
TimeType marker, baseline;
gdouble zoom;
unsigned use_new_times : 1;
unsigned viewer_is_initialized : 1;
};

#endif

/*
 * $Id: debug.h,v 1.14 2010/10/06 20:15:51 gtkwave Exp $
 * $Log: debug.h,v $
 * Revision 1.14  2010/10/06 20:15:51  gtkwave
 * preliminary version of RPC mechanism
 *
 * Revision 1.13  2010/09/16 04:08:55  gtkwave
 * add back in memory debugging printfs and defines
 *
 * Revision 1.12  2010/03/09 22:21:29  gtkwave
 * added optional preliminary Judy array support
 *
 * Revision 1.11  2010/01/26 23:08:01  gtkwave
 * removed unused obsolete structure definition
 *
 * Revision 1.10  2009/10/08 15:32:53  gtkwave
 * remove all instances of "long long"
 *
 * Revision 1.9  2009/06/07 08:40:44  gtkwave
 * adding FST support
 *
 * Revision 1.8  2009/04/24 04:24:22  gtkwave
 * reload and cygwin fixes for rtlbrowse
 *
 * Revision 1.7  2009/04/23 21:57:53  gtkwave
 * added mingw support for rtlbrowse
 *
 * Revision 1.6  2009/01/20 06:11:48  gtkwave
 * added gtkwave::getDisplayedSignals command
 *
 * Revision 1.5  2009/01/13 22:39:24  gtkwave
 * compile fixes for mingw
 *
 * Revision 1.4  2008/07/01 18:51:06  gtkwave
 * compiler warning fixes for amd64
 *
 * Revision 1.3  2007/08/26 21:35:40  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.2.2.3  2007/08/07 04:54:59  gtkwave
 * slight modifications to global initialization scheme
 *
 * Revision 1.2.2.2  2007/08/06 03:50:45  gtkwave
 * globals support for ae2, gtk1, cygwin, mingw.  also cleaned up some machine
 * generated structs, etc.
 *
 * Revision 1.2.2.1  2007/08/05 02:27:19  kermin
 * Semi working global struct
 *
 * Revision 1.2  2007/06/01 21:13:41  gtkwave
 * regenerate configure for cygwin with proper flags, add missing files
 *
 * Revision 1.1.1.1  2007/05/30 04:28:00  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:11  gtkwave
 * initial release
 *
 */

