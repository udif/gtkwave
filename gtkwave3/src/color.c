/* 
 * Copyright (c) Tony Bybell 1999-2005.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"
#include <config.h>
#include "color.h"
#include "debug.h"

/* 
 * return graphics context with tuple's color or
 * a fallback context.  Note that if tuple<0,
 * the fallback will be used!
 */
GdkGC *alloc_color(GtkWidget *widget, int tuple, GdkGC *fallback)
{
GdkColor color;
GdkGC    *gc;
int red, green, blue;

red=  (tuple>>16)&0x000000ff;
green=(tuple>>8) &0x000000ff;
blue= (tuple)    &0x000000ff;

if(tuple>=0)
if((gc=gdk_gc_new(widget->window)))
	{
	color.red=red*(65535/255);
	color.blue=blue*(65535/255);  
	color.green=green*(65535/255);
	color.pixel=(gulong)(tuple&0x00ffffff);
	gdk_color_alloc(gtk_widget_get_colormap(widget),&color);
	gdk_gc_set_foreground(gc,&color);
	return(gc);
	}

return(fallback);
}

/*
 * $Id: color.c,v 1.2 2007/08/26 21:35:39 gtkwave Exp $
 * $Log: color.c,v $
 * Revision 1.2  2007/08/26 21:35:39  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.3  2007/08/25 19:43:45  gtkwave
 * header cleanups
 *
 * Revision 1.1.1.1.2.2  2007/08/06 03:50:45  gtkwave
 * globals support for ae2, gtk1, cygwin, mingw.  also cleaned up some machine
 * generated structs, etc.
 *
 * Revision 1.1.1.1.2.1  2007/08/05 02:27:19  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1  2007/05/30 04:27:22  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:11  gtkwave
 * initial release
 *
 */

