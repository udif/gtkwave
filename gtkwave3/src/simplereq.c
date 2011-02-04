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
#include "gtk12compat.h"
#include <gtk/gtk.h>
#include "menu.h"
#include "debug.h"
#include "pixmaps.h"


static void ok_callback(GtkWidget *widget, GtkWidget *nothing)
{
  DEBUG(printf("OK\n"));
  gtk_grab_remove(GLOBALS->window_simplereq_c_9);
  gtk_widget_destroy(GLOBALS->window_simplereq_c_9);
  GLOBALS->window_simplereq_c_9 = NULL;
  if(GLOBALS->cleanup)GLOBALS->cleanup(NULL,(gpointer)1);
}

static void destroy_callback(GtkWidget *widget, GtkWidget *nothing)
{
  DEBUG(printf("Cancel\n"));
  gtk_grab_remove(GLOBALS->window_simplereq_c_9);
  gtk_widget_destroy(GLOBALS->window_simplereq_c_9);
  GLOBALS->window_simplereq_c_9 = NULL;
  if(GLOBALS->cleanup)GLOBALS->cleanup(NULL,NULL);
}

void simplereqbox(char *title, int width, char *default_text,
	char *oktext, char *canceltext, GtkSignalFunc func, int is_alert)
{
    GtkWidget *vbox, *hbox;
    GtkWidget *button1, *button2;
    GtkWidget *label, *separator;
    GtkWidget *pixmapwid1;

    if(GLOBALS->window_simplereq_c_9) return; /* only should happen with GtkPlug */

    GLOBALS->cleanup=WAVE_GTK_SFUNCAST(func);

    if(GLOBALS->wave_script_args)
	{
	if(GLOBALS->cleanup)GLOBALS->cleanup(NULL,(gpointer)1);
	return;
	}

    /* create a new modal window */
    GLOBALS->window_simplereq_c_9 = gtk_window_new(GLOBALS->disable_window_manager ? GTK_WINDOW_POPUP : GTK_WINDOW_TOPLEVEL);
    install_focus_cb(GLOBALS->window_simplereq_c_9, ((char *)&GLOBALS->window_simplereq_c_9) - ((char *)GLOBALS));

    gtk_window_set_transient_for(GTK_WINDOW(GLOBALS->window_simplereq_c_9), GTK_WINDOW(GLOBALS->mainwindow));
    gtk_widget_set_usize( GTK_WIDGET (GLOBALS->window_simplereq_c_9), width, 55 + 52);
    gtk_window_set_title(GTK_WINDOW (GLOBALS->window_simplereq_c_9), title);
    gtkwave_signal_connect(GTK_OBJECT (GLOBALS->window_simplereq_c_9), "delete_event",(GtkSignalFunc) destroy_callback, NULL);
    gtk_window_set_policy(GTK_WINDOW(GLOBALS->window_simplereq_c_9), FALSE, FALSE, FALSE);

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (GLOBALS->window_simplereq_c_9), vbox);
    gtk_widget_show (vbox);

    label=gtk_label_new(default_text);
    gtk_box_pack_start (GTK_BOX (vbox), label, FALSE, FALSE, 0);
    gtk_widget_show (label);

    if(is_alert)
	{
	pixmapwid1=gtk_pixmap_new(GLOBALS->wave_alert_pixmap, GLOBALS->wave_alert_mask);
	}
	else
	{
	pixmapwid1=gtk_pixmap_new(GLOBALS->wave_info_pixmap, GLOBALS->wave_info_mask);
	}
    gtk_widget_show(pixmapwid1);
    gtk_container_add (GTK_CONTAINER (vbox), pixmapwid1);

    separator = gtk_hseparator_new ();
    gtk_box_pack_start (GTK_BOX (vbox), separator, FALSE, TRUE, 0);
    gtk_widget_show (separator);

    hbox = gtk_hbox_new (FALSE, 1);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show (hbox);

    button1 = gtk_button_new_with_label (oktext);
    gtk_widget_set_usize(button1, 100, -1);
    gtkwave_signal_connect(GTK_OBJECT (button1), "clicked", GTK_SIGNAL_FUNC(ok_callback), NULL);
    gtk_widget_show (button1);
    gtk_container_add (GTK_CONTAINER (hbox), button1);
    GTK_WIDGET_SET_FLAGS (button1, GTK_CAN_DEFAULT);
    gtkwave_signal_connect_object (GTK_OBJECT (button1), "realize", (GtkSignalFunc) gtk_widget_grab_default, GTK_OBJECT (button1));

    if(canceltext)
	{
    	button2 = gtk_button_new_with_label (canceltext);
    	gtk_widget_set_usize(button2, 100, -1);
    	gtkwave_signal_connect(GTK_OBJECT (button2), "clicked", GTK_SIGNAL_FUNC(destroy_callback), NULL);
    	GTK_WIDGET_SET_FLAGS (button2, GTK_CAN_DEFAULT);
    	gtk_widget_show (button2);
    	gtk_container_add (GTK_CONTAINER (hbox), button2);
	}

    gtk_widget_show(GLOBALS->window_simplereq_c_9);
    gtk_grab_add(GLOBALS->window_simplereq_c_9);
}

/*
 * $Id: simplereq.c,v 1.7 2010/05/27 06:07:25 gtkwave Exp $
 * $Log: simplereq.c,v $
 * Revision 1.7  2010/05/27 06:07:25  gtkwave
 * Moved gtk_grab_add() after gtk_widget_show() as newer gtk needs that order.
 *
 * Revision 1.6  2009/12/15 23:40:59  gtkwave
 * removed old style scripts; also removed tempfiles for Tcl args
 *
 * Revision 1.5  2008/11/14 19:01:28  gtkwave
 * updated simplereq so the window is transient
 *
 * Revision 1.4  2007/09/12 17:26:45  gtkwave
 * experimental ctx_swap_watchdog added...still tracking down mouse thrash crashes
 *
 * Revision 1.3  2007/09/10 18:08:49  gtkwave
 * tabs selection can swap dynamically based on external window focus
 *
 * Revision 1.2  2007/08/26 21:35:44  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.6  2007/08/07 03:18:55  kermin
 * Changed to pointer based GLOBAL structure and added initialization function
 *
 * Revision 1.1.1.1.2.5  2007/08/06 03:50:48  gtkwave
 * globals support for ae2, gtk1, cygwin, mingw.  also cleaned up some machine
 * generated structs, etc.
 *
 * Revision 1.1.1.1.2.4  2007/08/05 02:27:23  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1.2.3  2007/07/31 03:18:01  kermin
 * Merge Complete - I hope
 *
 * Revision 1.1.1.1.2.2  2007/07/28 19:50:40  kermin
 * Merged in the main line
 *
 * Revision 1.1.1.1  2007/05/30 04:27:22  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:17  gtkwave
 * initial release
 *
 */

