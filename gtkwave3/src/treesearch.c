/* 
 * Copyright (c) Tony Bybell 2006.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <config.h>
#include "globals.h"
#include "gtk12compat.h"

/*
 * gtk1 users are forced to use the old treesearch widget
 * for now, sorry.
 */
#if WAVE_USE_GTK2
#include "treesearch_gtk2.c"
#else
#include "treesearch_gtk1.c"
#endif

void mkmenu_treesearch_cleanup(GtkWidget *widget, gpointer data)
{
/* nothing */
}

/*
 * $Id: treesearch.c,v 1.2 2007/08/26 21:35:46 gtkwave Exp $
 * $Log: treesearch.c,v $
 * Revision 1.2  2007/08/26 21:35:46  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.2  2007/08/06 03:50:50  gtkwave
 * globals support for ae2, gtk1, cygwin, mingw.  also cleaned up some machine
 * generated structs, etc.
 *
 * Revision 1.1.1.1.2.1  2007/08/05 02:27:27  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1  2007/05/30 04:27:26  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:17  gtkwave
 * initial release
 *
 */

