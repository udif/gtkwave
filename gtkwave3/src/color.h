/* 
 * Copyright (c) Tony Bybell 1999-2005.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"

#ifndef WAVE_COLOR_H
#define WAVE_COLOR_H

#include <stdlib.h>
#include <gtk/gtk.h>

GdkGC *alloc_color(GtkWidget *widget, int tuple, GdkGC *fallback);	/* tuple is encoded as 32bit: --RRGGBB (>=0 is valid) */

#endif

/*
 * $Id: color.h,v 1.2 2007/08/26 21:35:40 gtkwave Exp $
 * $Log: color.h,v $
 * Revision 1.2  2007/08/26 21:35:40  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.2  2007/08/25 19:43:45  gtkwave
 * header cleanups
 *
 * Revision 1.1.1.1.2.1  2007/08/05 02:27:19  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1  2007/05/30 04:27:54  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:11  gtkwave
 * initial release
 *
 */

