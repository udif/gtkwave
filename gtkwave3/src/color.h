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

struct wave_gcmaster_t
{
GdkGC *gc_ltgray;
GdkGC *gc_normal;
GdkGC *gc_mdgray;
GdkGC *gc_dkgray;
GdkGC *gc_dkblue;
GdkGC *gc_brkred;
GdkGC *gc_ltblue;
GdkGC *gc_gmstrd;
GdkGC *gc_back_wavewindow_c_1;
GdkGC *gc_baseline_wavewindow_c_1;
GdkGC *gc_grid_wavewindow_c_1;
GdkGC *gc_grid2_wavewindow_c_1;
GdkGC *gc_time_wavewindow_c_1;
GdkGC *gc_timeb_wavewindow_c_1;
GdkGC *gc_value_wavewindow_c_1;
GdkGC *gc_low_wavewindow_c_1;
GdkGC *gc_high_wavewindow_c_1;
GdkGC *gc_trans_wavewindow_c_1;
GdkGC *gc_mid_wavewindow_c_1;
GdkGC *gc_xfill_wavewindow_c_1;
GdkGC *gc_x_wavewindow_c_1;
GdkGC *gc_vbox_wavewindow_c_1;
GdkGC *gc_vtrans_wavewindow_c_1;
GdkGC *gc_mark_wavewindow_c_1;
GdkGC *gc_umark_wavewindow_c_1;
GdkGC *gc_0_wavewindow_c_1;
GdkGC *gc_1_wavewindow_c_1;
GdkGC *gc_ufill_wavewindow_c_1;
GdkGC *gc_u_wavewindow_c_1;
GdkGC *gc_wfill_wavewindow_c_1;
GdkGC *gc_w_wavewindow_c_1;
GdkGC *gc_dashfill_wavewindow_c_1;
GdkGC *gc_dash_wavewindow_c_1;
};

GdkGC *alloc_color(GtkWidget *widget, int tuple, GdkGC *fallback);	/* tuple is encoded as 32bit: --RRGGBB (>=0 is valid) */

#endif

