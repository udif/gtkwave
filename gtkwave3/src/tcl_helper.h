/*
 * Copyright (c) Tony Bybell and Concept Engineering GmbH 2008-2009.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef WAVE_TCLHELPER_H
#define WAVE_TCLHELPER_H

#include <config.h>
#include "tcl_callbacks.h"

#ifdef HAVE_LIBTCL

#include <tcl.h>
#include <tk.h>
#include "debug.h"

#define WAVE_TCL_CHECK_VERSION(major,minor,micro)    \
    (TCL_MAJOR_VERSION > (major) || \
     (TCL_MAJOR_VERSION == (major) && TCL_MINOR_VERSION > (minor)) || \
     (TCL_MAJOR_VERSION == (major) && TCL_MINOR_VERSION == (minor) && \
      TCL_RELEASE_SERIAL >= (micro)))


typedef struct
        {
        const char *cmdstr;
        int (*func)();
        } tcl_cmdstruct;

extern tcl_cmdstruct gtkwave_commands[];

#endif

#define WAVE_OE_ME \
	if(one_entry) \
		{ \
		if(!mult_entry) \
			{ \
			mult_entry = one_entry; \
			mult_len = strlen(mult_entry); \
			} \
			else \
			{ \
			int sing_len = strlen(one_entry); \
			mult_entry = realloc_2(mult_entry, mult_len + sing_len + 1); \
			strcpy(mult_entry + mult_len, one_entry); \
			mult_len += sing_len; \
			free_2(one_entry); \
			} \
		}

struct iter_dnd_strings 
	{
	char *one_entry;
	char *mult_entry;
	int mult_len;
	};

typedef enum {LL_NONE, LL_INT, LL_UINT, LL_CHAR, LL_SHORT, LL_STR, 
	      LL_VOID_P, LL_TIMETYPE} ll_elem_type;

typedef union llist_payload {
  int i ;
  unsigned int u ;
  char c ;
  short s ;
  char *str ;
  void *p ;
  TimeType tt ;
} llist_u;

typedef struct llist_s {
  llist_u u;
  struct llist_s *prev ;
  struct llist_s *next ;
} llist_p ;


int process_url_file(char *s);
int process_url_list(char *s);
int process_tcl_list(char *s, gboolean track_mouse_y);
char *add_dnd_from_searchbox(void);
char *add_dnd_from_signal_window(void);
char *add_traces_from_signal_window(gboolean is_from_tcl_command);
char *add_dnd_from_tree_window(void);
char *emit_gtkwave_savefile_formatted_entries_in_tcl_list(Trptr trhead, gboolean use_tcl_mode);

char* zMergeTclList(int argc, const char** argv);
char** zSplitTclList(const char* list, int* argcPtr);
char *make_single_tcl_list_name(char *s, char *opt_value, int promote_to_bus);

void make_tcl_interpreter(char *argv[]);
const char *gtkwavetcl_setvar(const char *name1, const char *val, int flags);
const char *gtkwavetcl_setvar_nonblocking(const char *name1, const char *val, int flags);

char *rpc_script_execute(const char *nam);

#ifdef HAVE_LIBTCL
int gtkwaveInterpreterInit (Tcl_Interp *interp);
void set_globals_interp(char *me, int install_tk);
#endif

#endif

/* 
 * $Id: tcl_helper.h,v 1.28 2010/10/06 20:15:51 gtkwave Exp $
 * $Log: tcl_helper.h,v $
 * Revision 1.28  2010/10/06 20:15:51  gtkwave
 * preliminary version of RPC mechanism
 *
 * Revision 1.27  2010/07/27 22:52:16  gtkwave
 * added nonblocking setvar (to be used only for well-known reasons)
 *
 * Revision 1.26  2010/07/27 19:25:41  gtkwave
 * initial tcl callback function adds
 *
 * Revision 1.25  2010/07/27 05:01:45  gtkwave
 * added gtkwavetcl_setvar for Tcl callback framework
 *
 * Revision 1.24  2010/07/15 14:27:05  gtkwave
 * repscript timer fix to print stack trace
 *
 * Revision 1.23  2009/12/15 23:40:59  gtkwave
 * removed old style scripts; also removed tempfiles for Tcl args
 *
 * Revision 1.22  2009/11/11 16:30:58  gtkwave
 * changed tcl library ordering, no tk unless --wish
 *
 * Revision 1.21  2009/10/23 20:10:33  gtkwave
 * compatibility cleanups with syntax
 *
 * Revision 1.20  2009/10/08 17:40:49  gtkwave
 * removed casting on llist_new, use union instead as arg
 *
 * Revision 1.19  2009/10/07 16:59:08  gtkwave
 * move Tcl_CreateInterp to tcl_helper.c to make stubify easier
 *
 * Revision 1.18  2009/09/28 05:58:05  gtkwave
 * changes to support signal_change_list
 *
 * Revision 1.17  2009/09/22 13:51:14  gtkwave
 * warnings fixes
 *
 * Revision 1.16  2009/09/14 03:00:08  gtkwave
 * bluespec code integration
 *
 * Revision 1.15  2009/01/23 19:23:10  gtkwave
 * compatibility fix for gcc 3.x
 *
 * Revision 1.14  2009/01/20 06:11:48  gtkwave
 * added gtkwave::getDisplayedSignals command
 *
 * Revision 1.13  2009/01/02 06:24:28  gtkwave
 * bumped copyright to 2009
 *
 * Revision 1.12  2009/01/02 06:01:51  gtkwave
 * added getArgv for tcl commands
 *
 * Revision 1.11  2008/11/25 18:07:32  gtkwave
 * added cut copy paste functionality that survives reload and can do
 * multiple pastes on the same cut buffer
 *
 * Revision 1.10  2008/11/24 02:55:10  gtkwave
 * use TCL_INCLUDE_SPEC to fix ubuntu compiles
 *
 * Revision 1.9  2008/11/17 16:49:38  gtkwave
 * convert net object to netBus when encountering stranded bits in
 * signal search and tree search window
 *
 * Revision 1.8  2008/10/26 02:36:06  gtkwave
 * added netValue and netBusValue tcl list values from sigwin drag
 *
 * Revision 1.7  2008/10/17 18:05:27  gtkwave
 * split tcl command extensions out into their own separate file
 *
 * Revision 1.6  2008/10/13 22:16:52  gtkwave
 * tcl interpreter integration
 *
 * Revision 1.5  2008/10/02 00:52:25  gtkwave
 * added dnd of external filetypes into viewer
 *
 * Revision 1.4  2008/09/29 22:46:39  gtkwave
 * complex dnd handling with gtkwave trace attributes
 *
 * Revision 1.3  2008/09/27 05:05:05  gtkwave
 * removed unnecessary sing_len struct item
 *
 * Revision 1.2  2008/09/25 01:41:36  gtkwave
 * drag from tree clist window into external process
 *
 * Revision 1.1  2008/09/25 01:31:29  gtkwave
 * file creation
 *
 */
