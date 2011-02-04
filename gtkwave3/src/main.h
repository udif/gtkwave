/* 
 * Copyright (c) Tony Bybell 1999.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"

#ifndef __MFMAIN_H__
#define __MFMAIN_H__

#include "busy.h"

#define HAVE_PANED_PACK	/* undefine this if you have an older GTK */

struct logfile_chain
{
struct logfile_chain *next;
char *name;
};

int main_2(int argc, char *argv[]);

GtkWidget *create_text(void);
GtkWidget *create_zoom_buttons(void);
GtkWidget *create_page_buttons(void);
GtkWidget *create_fetch_buttons(void);
GtkWidget *create_discard_buttons(void);
GtkWidget *create_edge_buttons(void);
GtkWidget *create_shift_buttons(void);
GtkWidget *create_entry_box(void);
GtkWidget *create_time_box(void);
GtkWidget *create_wavewindow(void);
GtkWidget *create_signalwindow(void);

/* Get/set the current size of the window.  */
extern void get_window_size (int *x, int *y);
extern void set_window_size (int x, int y);

/* Get/set the x/y pos of the window */
void get_window_xypos(int *root_x, int *root_y);
void set_window_xypos(int root_x, int root_y);

/* stems helper activation */
int stems_are_active(void);
void activate_stems_reader(char *stems_name);
#if !defined _MSC_VER
void kill_stems_browser(void);
#endif
void kill_stems_browser_single(void *G);

/* prototype only used in main.c */
void menu_reload_waveform(GtkWidget *widget, gpointer data);


/* function for spawning vcd conversions */
void optimize_vcd_file(void);

enum FileType {
  MISSING_FILE,
  LXT_FILE,
  LX2_FILE,
  VZT_FILE,
  AE2_FILE,
  GHW_FILE,
  VCD_FILE,
  VCD_RECODER_FILE,
#ifdef EXTLOAD_SUFFIX
  EXTLOAD_FILE,
#endif
  FST_FILE,
  DUMPLESS_FILE
};

#endif

/*
 * $Id: main.h,v 1.8 2009/06/07 08:40:44 gtkwave Exp $
 * $Log: main.h,v $
 * Revision 1.8  2009/06/07 08:40:44  gtkwave
 * adding FST support
 *
 * Revision 1.7  2009/04/24 20:00:03  gtkwave
 * rtlbrowse exit kill mods
 *
 * Revision 1.6  2009/03/26 20:57:41  gtkwave
 * added MISSING_FILE support for bringing up gtkwave without a dumpfile
 *
 * Revision 1.5  2009/01/27 07:04:31  gtkwave
 * added extload external process loader capability
 *
 * Revision 1.4  2008/07/01 18:51:07  gtkwave
 * compiler warning fixes for amd64
 *
 * Revision 1.3  2007/12/30 04:27:39  gtkwave
 * added edge buttons to main window
 *
 * Revision 1.2  2007/08/26 21:35:42  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.5  2007/08/26 19:05:55  gtkwave
 * added reload button in main window
 *
 * Revision 1.1.1.1.2.4  2007/08/25 19:43:45  gtkwave
 * header cleanups
 *
 * Revision 1.1.1.1.2.3  2007/08/19 23:13:53  kermin
 * -o flag will now target the original file (in theory reloaded), compress it to lxt2, and then reload the new compressed file.
 *
 * Revision 1.1.1.1.2.2  2007/08/15 03:26:01  kermin
 * Reload button does not cause a fault, however, state is still somehow incorrect.
 *
 * Revision 1.1.1.1.2.1  2007/08/05 02:27:21  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1  2007/05/30 04:27:28  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:13  gtkwave
 * initial release
 *
 */

