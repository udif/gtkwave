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


/* dealloc all gcs */
static void w_gdk_gc_destroy(GdkGC *g)
{
/* currently do nothing: need to mark these somehow */
/* gdk_gc_destroy(g); */
}


void dealloc_all_gcs(void)
{
int i;

if(GLOBALS->gccache.gc_ltgray) { w_gdk_gc_destroy(GLOBALS->gccache.gc_ltgray); }
if(GLOBALS->gccache.gc_normal) { w_gdk_gc_destroy(GLOBALS->gccache.gc_normal); }
if(GLOBALS->gccache.gc_mdgray) { w_gdk_gc_destroy(GLOBALS->gccache.gc_mdgray); }
if(GLOBALS->gccache.gc_dkgray) { w_gdk_gc_destroy(GLOBALS->gccache.gc_dkgray); }
if(GLOBALS->gccache.gc_dkblue) { w_gdk_gc_destroy(GLOBALS->gccache.gc_dkblue); }
if(GLOBALS->gccache.gc_brkred) { w_gdk_gc_destroy(GLOBALS->gccache.gc_brkred); }
if(GLOBALS->gccache.gc_ltblue) { w_gdk_gc_destroy(GLOBALS->gccache.gc_ltblue); }
if(GLOBALS->gccache.gc_gmstrd) { w_gdk_gc_destroy(GLOBALS->gccache.gc_gmstrd); }
if(GLOBALS->gccache.gc_back_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_back_wavewindow_c_1); }
if(GLOBALS->gccache.gc_baseline_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_baseline_wavewindow_c_1); }
if(GLOBALS->gccache.gc_grid_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_grid_wavewindow_c_1); }
if(GLOBALS->gccache.gc_grid2_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_grid2_wavewindow_c_1); }
if(GLOBALS->gccache.gc_time_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_time_wavewindow_c_1); }
if(GLOBALS->gccache.gc_timeb_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_timeb_wavewindow_c_1); }
if(GLOBALS->gccache.gc_value_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_value_wavewindow_c_1); }
if(GLOBALS->gccache.gc_low_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_low_wavewindow_c_1); }
if(GLOBALS->gccache.gc_high_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_high_wavewindow_c_1); }
if(GLOBALS->gccache.gc_trans_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_trans_wavewindow_c_1); }
if(GLOBALS->gccache.gc_mid_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_mid_wavewindow_c_1); }
if(GLOBALS->gccache.gc_xfill_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_xfill_wavewindow_c_1); }
if(GLOBALS->gccache.gc_x_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_x_wavewindow_c_1); }
if(GLOBALS->gccache.gc_vbox_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_vbox_wavewindow_c_1); }
if(GLOBALS->gccache.gc_vtrans_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_vtrans_wavewindow_c_1); }
if(GLOBALS->gccache.gc_mark_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_mark_wavewindow_c_1); }
if(GLOBALS->gccache.gc_umark_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_umark_wavewindow_c_1); }
if(GLOBALS->gccache.gc_0_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_0_wavewindow_c_1); }
if(GLOBALS->gccache.gc_1_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_1_wavewindow_c_1); }
if(GLOBALS->gccache.gc_ufill_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_ufill_wavewindow_c_1); }
if(GLOBALS->gccache.gc_u_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_u_wavewindow_c_1); }
if(GLOBALS->gccache.gc_wfill_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_wfill_wavewindow_c_1); }
if(GLOBALS->gccache.gc_w_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_w_wavewindow_c_1); }
if(GLOBALS->gccache.gc_dashfill_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_dashfill_wavewindow_c_1); }
if(GLOBALS->gccache.gc_dash_wavewindow_c_1) { w_gdk_gc_destroy(GLOBALS->gccache.gc_dash_wavewindow_c_1); }

for(i=0;i<WAVE_NUM_RAINBOW;i++)
	{
        if(GLOBALS->gc_rainbow[2*i+1]) { w_gdk_gc_destroy(GLOBALS->gc_rainbow[2*i+1]); }
        if(GLOBALS->gc_rainbow[2*i+1]) { w_gdk_gc_destroy(GLOBALS->gc_rainbow[2*i+1]); }
	}

if(GLOBALS->gc_black) { w_gdk_gc_destroy(GLOBALS->gc_black); }
if(GLOBALS->gc_white) { w_gdk_gc_destroy(GLOBALS->gc_white); }
}


void set_alternate_gcs(GdkGC *ctx, GdkGC *ctx_fill)
{
GLOBALS->gc.gc_low_wavewindow_c_1 = ctx;
GLOBALS->gc.gc_high_wavewindow_c_1 = ctx;
GLOBALS->gc.gc_trans_wavewindow_c_1 = ctx;
GLOBALS->gc.gc_mid_wavewindow_c_1 = ctx;
GLOBALS->gc.gc_xfill_wavewindow_c_1 = ctx_fill;
GLOBALS->gc.gc_x_wavewindow_c_1 = ctx;
GLOBALS->gc.gc_vbox_wavewindow_c_1 = ctx;
GLOBALS->gc.gc_vtrans_wavewindow_c_1 = ctx;
GLOBALS->gc.gc_mark_wavewindow_c_1 = ctx;
GLOBALS->gc.gc_umark_wavewindow_c_1 = ctx;
GLOBALS->gc.gc_0_wavewindow_c_1 = ctx;
GLOBALS->gc.gc_1_wavewindow_c_1 = ctx;
GLOBALS->gc.gc_ufill_wavewindow_c_1 = ctx_fill;
GLOBALS->gc.gc_u_wavewindow_c_1 = ctx;
GLOBALS->gc.gc_wfill_wavewindow_c_1 = ctx_fill;
GLOBALS->gc.gc_w_wavewindow_c_1 = ctx;
GLOBALS->gc.gc_dashfill_wavewindow_c_1 = ctx_fill;
GLOBALS->gc.gc_dash_wavewindow_c_1 = ctx;
}
