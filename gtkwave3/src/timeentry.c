/* 
 * Copyright (c) Tony Bybell 1999-2008
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <config.h>
#include "globals.h"
#include <gtk/gtk.h>
#include "gtk12compat.h"
#include "symbol.h"
#include "debug.h"


void update_endcap_times_for_partial_vcd(void)
{
char str[40];

if(GLOBALS->from_entry)
	{
	reformat_time(str, GLOBALS->tims.first, GLOBALS->time_dimension);
	gtk_entry_set_text(GTK_ENTRY(GLOBALS->from_entry),str);
	gtkwavetcl_setvar(WAVE_TCLCB_FROM_ENTRY_UPDATED, str, WAVE_TCLCB_FROM_ENTRY_UPDATED_FLAGS);
	}

if(GLOBALS->to_entry)
	{
	reformat_time(str, GLOBALS->tims.last, GLOBALS->time_dimension);
	gtk_entry_set_text(GTK_ENTRY(GLOBALS->to_entry),str);
	gtkwavetcl_setvar(WAVE_TCLCB_TO_ENTRY_UPDATED, str, WAVE_TCLCB_TO_ENTRY_UPDATED_FLAGS);
	}
}

void time_update(void)
{
DEBUG(printf("Timeentry Configure Event\n"));

calczoom(GLOBALS->tims.zoom);
fix_wavehadj();
gtk_signal_emit_by_name (GTK_OBJECT (GTK_ADJUSTMENT(GLOBALS->wave_hslider)), "changed");
gtk_signal_emit_by_name (GTK_OBJECT (GTK_ADJUSTMENT(GLOBALS->wave_hslider)), "value_changed");
}

   
void from_entry_callback(GtkWidget *widget, GtkWidget *entry)
{
G_CONST_RETURN gchar *entry_text;
TimeType newlo;
char fromstr[40];

entry_text=gtk_entry_get_text(GTK_ENTRY(entry));
DEBUG(printf("From Entry contents: %s\n",entry_text));

newlo=unformat_time(entry_text, GLOBALS->time_dimension);

if(newlo<GLOBALS->min_time) 
	{
	newlo=GLOBALS->min_time; 
	}

if(newlo<(GLOBALS->tims.last)) 
	{ 
	GLOBALS->tims.first=newlo;
	if(GLOBALS->tims.start<GLOBALS->tims.first) GLOBALS->tims.timecache=GLOBALS->tims.start=GLOBALS->tims.first;

	reformat_time(fromstr, GLOBALS->tims.first, GLOBALS->time_dimension);
	gtk_entry_set_text(GTK_ENTRY(entry),fromstr);
	time_update(); 
	gtkwavetcl_setvar(WAVE_TCLCB_FROM_ENTRY_UPDATED, fromstr, WAVE_TCLCB_FROM_ENTRY_UPDATED_FLAGS);
	return;
	}
	else
	{
	reformat_time(fromstr, GLOBALS->tims.first, GLOBALS->time_dimension);
	gtk_entry_set_text(GTK_ENTRY(entry),fromstr);
	gtkwavetcl_setvar(WAVE_TCLCB_FROM_ENTRY_UPDATED, fromstr, WAVE_TCLCB_FROM_ENTRY_UPDATED_FLAGS);
	return;
	}
}

void to_entry_callback(GtkWidget *widget, GtkWidget *entry)
{
G_CONST_RETURN gchar *entry_text;
TimeType newhi;
char tostr[40];

entry_text=gtk_entry_get_text(GTK_ENTRY(entry));
DEBUG(printf("To Entry contents: %s\n",entry_text));

newhi=unformat_time(entry_text, GLOBALS->time_dimension);

if(newhi>GLOBALS->max_time) 
	{
	newhi=GLOBALS->max_time; 
	}

if(newhi>(GLOBALS->tims.first)) 
	{ 
	GLOBALS->tims.last=newhi;
	reformat_time(tostr, GLOBALS->tims.last, GLOBALS->time_dimension);
	gtk_entry_set_text(GTK_ENTRY(entry),tostr);
	time_update(); 
	gtkwavetcl_setvar(WAVE_TCLCB_TO_ENTRY_UPDATED, tostr, WAVE_TCLCB_TO_ENTRY_UPDATED_FLAGS);
	return;
	}
	else
	{
	reformat_time(tostr, GLOBALS->tims.last, GLOBALS->time_dimension);
	gtk_entry_set_text(GTK_ENTRY(entry),tostr);
	gtkwavetcl_setvar(WAVE_TCLCB_TO_ENTRY_UPDATED, tostr, WAVE_TCLCB_TO_ENTRY_UPDATED_FLAGS);
	return;
	}
}
   
/* Create an entry box */
GtkWidget *
create_entry_box(void)
{
GtkWidget *label, *label2;
GtkWidget *box, *box2;
GtkWidget *mainbox;

char fromstr[32], tostr[32];

GtkTooltips *tooltips;

tooltips=gtk_tooltips_new_2();
gtk_tooltips_set_delay_2(tooltips,1500);

label=gtk_label_new("From:");
GLOBALS->from_entry=gtk_entry_new_with_max_length(40);

reformat_time(fromstr, GLOBALS->min_time, GLOBALS->time_dimension);

gtk_entry_set_text(GTK_ENTRY(GLOBALS->from_entry),fromstr);
gtk_signal_connect (GTK_OBJECT (GLOBALS->from_entry), "activate",GTK_SIGNAL_FUNC (from_entry_callback), GLOBALS->from_entry);
box=gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(box), label, TRUE, TRUE, 0); 
gtk_widget_show(label);
gtk_box_pack_start(GTK_BOX(box), GLOBALS->from_entry, TRUE, TRUE, 0); 
gtk_widget_set_usize(GTK_WIDGET(GLOBALS->from_entry), 90, 22); 
gtk_tooltips_set_tip_2(tooltips, GLOBALS->from_entry, "Scroll Lower Bound", NULL);
gtk_widget_show(GLOBALS->from_entry);


label2=gtk_label_new("To:");
GLOBALS->to_entry=gtk_entry_new_with_max_length(40);

reformat_time(tostr, GLOBALS->max_time, GLOBALS->time_dimension);

gtk_entry_set_text(GTK_ENTRY(GLOBALS->to_entry),tostr);
gtk_signal_connect (GTK_OBJECT (GLOBALS->to_entry), "activate",GTK_SIGNAL_FUNC (to_entry_callback), GLOBALS->to_entry);
box2=gtk_hbox_new(FALSE, 0);
gtk_box_pack_start(GTK_BOX(box2), label2, TRUE, TRUE, 0); 
gtk_widget_show(label2);
gtk_box_pack_start(GTK_BOX(box2), GLOBALS->to_entry, TRUE, TRUE, 0); 
gtk_widget_set_usize(GTK_WIDGET(GLOBALS->to_entry), 90, 22); 
gtk_tooltips_set_tip_2(tooltips, GLOBALS->to_entry, "Scroll Upper Bound", NULL);
gtk_widget_show(GLOBALS->to_entry);

if(!GLOBALS->use_toolbutton_interface)
	{
	mainbox=gtk_vbox_new(FALSE, 0);
	}
	else
	{
	mainbox=gtk_hbox_new(FALSE, 0);
	}

gtk_box_pack_start(GTK_BOX(mainbox), box, TRUE, FALSE, 1);
gtk_widget_show(box);
gtk_box_pack_start(GTK_BOX(mainbox), box2, TRUE, FALSE, 1);
gtk_widget_show(box2);

return(mainbox);
}
   
/*
 * $Id: timeentry.c,v 1.7 2010/08/03 01:37:26 gtkwave Exp $
 * $Log: timeentry.c,v $
 * Revision 1.7  2010/08/03 01:37:26  gtkwave
 * added gtkwave::cbFromEntryUpdated, gtkwave::cbToEntryUpdated
 * and gtkwave::cbStatusText.
 *
 * Revision 1.6  2009/01/16 19:27:00  gtkwave
 * added more tcl commands
 *
 * Revision 1.5  2008/01/09 08:06:17  gtkwave
 * remove context swap signal handler as it doesn't allow ctx > 0 to update
 *
 * Revision 1.4  2008/01/08 23:03:36  gtkwave
 * added toolbar using use_toolbutton_interface rc variable
 *
 * Revision 1.3  2007/09/12 17:26:45  gtkwave
 * experimental ctx_swap_watchdog added...still tracking down mouse thrash crashes
 *
 * Revision 1.2  2007/08/26 21:35:45  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.6  2007/08/07 03:18:55  kermin
 * Changed to pointer based GLOBAL structure and added initialization function
 *
 * Revision 1.1.1.1.2.5  2007/08/06 03:50:49  gtkwave
 * globals support for ae2, gtk1, cygwin, mingw.  also cleaned up some machine
 * generated structs, etc.
 *
 * Revision 1.1.1.1.2.4  2007/08/05 02:27:24  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1.2.3  2007/07/31 03:18:01  kermin
 * Merge Complete - I hope
 *
 * Revision 1.1.1.1.2.2  2007/07/28 19:50:40  kermin
 * Merged in the main line
 *
 * Revision 1.1.1.1  2007/05/30 04:27:29  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:17  gtkwave
 * initial release
 *
 */

