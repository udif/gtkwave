/* 
 * Copyright (c) Tony Bybell 1999-2005.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"

#ifndef WAVE_PIXMAPS_H
#define WAVE_PIXMAPS_H

#include <gtk/gtk.h>

void make_pixmaps(GtkWidget *window);

#ifdef WAVE_USE_GTK2

#define WAVE_SPLASH_X (512)
#define WAVE_SPLASH_Y (384)

void make_splash_pixmaps(GtkWidget *window);

#endif

#endif

/*
 * $Id: pixmaps.h,v 1.2 2007/08/26 21:35:43 gtkwave Exp $
 * $Log: pixmaps.h,v $
 * Revision 1.2  2007/08/26 21:35:43  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.2  2007/08/25 19:43:46  gtkwave
 * header cleanups
 *
 * Revision 1.1.1.1.2.1  2007/08/05 02:27:22  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1  2007/05/30 04:27:26  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:13  gtkwave
 * initial release
 *
 */

