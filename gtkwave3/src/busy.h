/*
 * Copyright (c) Tony Bybell 2006.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"

#ifndef WAVE_BUSYWIN_H
#define WAVE_BUSYWIN_H

#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#include "main.h"

/* number of histents to create before kicking off gtk_main_iteration() checking */
#define WAVE_BUSY_ITER (1000)

void init_busy(void); 
void set_window_busy(GtkWidget *w);
void set_window_idle(GtkWidget *w);
void busy_window_refresh(void);

void gtkwave_main_iteration(void);

#endif

/*
 * $Id: busy.h,v 1.4 2010/02/18 23:06:04 gtkwave Exp $
 * $Log: busy.h,v $
 * Revision 1.4  2010/02/18 23:06:04  gtkwave
 * change name of main iteration loop calls
 *
 * Revision 1.3  2007/09/11 02:12:50  gtkwave
 * context locking in busy spinloops (gtk_main_iteration() calls)
 *
 * Revision 1.2  2007/08/26 21:35:39  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.2  2007/08/25 19:43:45  gtkwave
 * header cleanups
 *
 * Revision 1.1.1.1.2.1  2007/08/05 02:27:19  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1  2007/05/30 04:27:26  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:11  gtkwave
 * initial release
 *
 */
