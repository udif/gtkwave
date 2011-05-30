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

