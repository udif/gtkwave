/* 
 * Copyright (c) Tony Bybell 1999-2009.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"
#include <config.h>
#include <gtk/gtk.h>
#include "gtk12compat.h"
#include "menu.h"
#include "debug.h"
#include <string.h>


static void enter_callback(GtkWidget *widget, GtkWidget *nothing)
{
  G_CONST_RETURN gchar *entry_text;
  int len;
  entry_text = gtk_entry_get_text(GTK_ENTRY(GLOBALS->entry_entry_c_1));
  DEBUG(printf("Entry contents: %s\n", entry_text));
  if(!(len=strlen(entry_text))) GLOBALS->entrybox_text=NULL;
	else strcpy((GLOBALS->entrybox_text=(char *)malloc_2(len+1)),entry_text);

  gtk_grab_remove(GLOBALS->window_entry_c_1);
  gtk_widget_destroy(GLOBALS->window_entry_c_1);
  GLOBALS->window_entry_c_1 = NULL;

  GLOBALS->cleanup_entry_c_1();
}

static void destroy_callback(GtkWidget *widget, GtkWidget *nothing)
{
  DEBUG(printf("Entry Cancel\n"));
  GLOBALS->entrybox_text=NULL;
  gtk_grab_remove(GLOBALS->window_entry_c_1);
  gtk_widget_destroy(GLOBALS->window_entry_c_1);
  GLOBALS->window_entry_c_1 = NULL;
}

void entrybox(char *title, int width, char *dflt_text, char *comment, int maxch, GtkSignalFunc func)
{
    GtkWidget *vbox, *hbox;
    GtkWidget *button1, *button2;
    int height = (comment) ? 75 : 60;

    GLOBALS->cleanup_entry_c_1=func;

    if(GLOBALS->wave_script_args)
	{
        char *s = NULL;

        while((!s)&&(GLOBALS->wave_script_args->curr)) s = wave_script_args_fgetmalloc_stripspaces(GLOBALS->wave_script_args);
	if(s)
		{
		fprintf(stderr, "GTKWAVE | Entry '%s'\n", s);
		GLOBALS->entrybox_text = s;
		GLOBALS->cleanup_entry_c_1();
		}
		else
		{
		GLOBALS->entrybox_text = NULL;
		}

	return;
	}

    /* create a new modal window */
    GLOBALS->window_entry_c_1 = gtk_window_new(GLOBALS->disable_window_manager ? GTK_WINDOW_POPUP : GTK_WINDOW_TOPLEVEL);
    install_focus_cb(GLOBALS->window_entry_c_1, ((char *)&GLOBALS->window_entry_c_1) - ((char *)GLOBALS));

    gtk_widget_set_usize( GTK_WIDGET (GLOBALS->window_entry_c_1), width, height);
    gtk_window_set_title(GTK_WINDOW (GLOBALS->window_entry_c_1), title);
    gtkwave_signal_connect(GTK_OBJECT (GLOBALS->window_entry_c_1), "delete_event",(GtkSignalFunc) destroy_callback, NULL);
    gtk_window_set_policy(GTK_WINDOW(GLOBALS->window_entry_c_1), FALSE, FALSE, FALSE);

    vbox = gtk_vbox_new (FALSE, 0);
    gtk_container_add (GTK_CONTAINER (GLOBALS->window_entry_c_1), vbox);
    gtk_widget_show (vbox);

    if (comment)
      {
	GtkWidget *label, *cbox;

	cbox = gtk_hbox_new (FALSE, 1);
	gtk_box_pack_start (GTK_BOX (vbox), cbox, FALSE, FALSE, 0);
	gtk_widget_show (cbox);

	label = gtk_label_new(comment);
	gtk_widget_show (label);

	gtk_container_add (GTK_CONTAINER (cbox), label);
	GTK_WIDGET_SET_FLAGS (label, GTK_CAN_DEFAULT);
      }

    GLOBALS->entry_entry_c_1 = gtk_entry_new_with_max_length (maxch);
    gtkwave_signal_connect(GTK_OBJECT(GLOBALS->entry_entry_c_1), "activate",GTK_SIGNAL_FUNC(enter_callback),GLOBALS->entry_entry_c_1);
    gtk_entry_set_text (GTK_ENTRY (GLOBALS->entry_entry_c_1), dflt_text);
    gtk_entry_select_region (GTK_ENTRY (GLOBALS->entry_entry_c_1),0, GTK_ENTRY(GLOBALS->entry_entry_c_1)->text_length);
    gtk_box_pack_start (GTK_BOX (vbox), GLOBALS->entry_entry_c_1, FALSE, FALSE, 0);
    gtk_widget_show (GLOBALS->entry_entry_c_1);

    hbox = gtk_hbox_new (FALSE, 1);
    gtk_box_pack_start (GTK_BOX (vbox), hbox, FALSE, FALSE, 0);
    gtk_widget_show (hbox);

    button1 = gtk_button_new_with_label ("OK");
    gtk_widget_set_usize(button1, 100, -1);
    gtkwave_signal_connect(GTK_OBJECT (button1), "clicked", GTK_SIGNAL_FUNC(enter_callback), NULL);
    gtk_widget_show (button1);
    gtk_container_add (GTK_CONTAINER (hbox), button1);
    GTK_WIDGET_SET_FLAGS (button1, GTK_CAN_DEFAULT);
    gtkwave_signal_connect_object (GTK_OBJECT (button1), "realize", (GtkSignalFunc) gtk_widget_grab_default, GTK_OBJECT (button1));


    button2 = gtk_button_new_with_label ("Cancel");
    gtk_widget_set_usize(button2, 100, -1);
    gtkwave_signal_connect(GTK_OBJECT (button2), "clicked", GTK_SIGNAL_FUNC(destroy_callback), NULL);
    GTK_WIDGET_SET_FLAGS (button2, GTK_CAN_DEFAULT);
    gtk_widget_show (button2);
    gtk_container_add (GTK_CONTAINER (hbox), button2);

    gtk_widget_show(GLOBALS->window_entry_c_1);
    gtk_grab_add(GLOBALS->window_entry_c_1);
}

/*
 * $Id: entry.c,v 1.7 2010/05/27 06:07:24 gtkwave Exp $
 * $Log: entry.c,v $
 * Revision 1.7  2010/05/27 06:07:24  gtkwave
 * Moved gtk_grab_add() after gtk_widget_show() as newer gtk needs that order.
 *
 * Revision 1.6  2009/12/15 23:40:59  gtkwave
 * removed old style scripts; also removed tempfiles for Tcl args
 *
 * Revision 1.5  2009/09/14 03:00:08  gtkwave
 * bluespec code integration
 *
 * Revision 1.4  2007/09/12 17:26:44  gtkwave
 * experimental ctx_swap_watchdog added...still tracking down mouse thrash crashes
 *
 * Revision 1.3  2007/09/10 18:08:48  gtkwave
 * tabs selection can swap dynamically based on external window focus
 *
 * Revision 1.2  2007/08/26 21:35:40  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.7  2007/08/18 21:51:57  gtkwave
 * widget destroys and teardown of file formats which use external loaders
 * and are outside of malloc_2/free_2 control
 *
 * Revision 1.1.1.1.2.6  2007/08/07 03:18:54  kermin
 * Changed to pointer based GLOBAL structure and added initialization function
 *
 * Revision 1.1.1.1.2.5  2007/08/06 03:50:46  gtkwave
 * globals support for ae2, gtk1, cygwin, mingw.  also cleaned up some machine
 * generated structs, etc.
 *
 * Revision 1.1.1.1.2.4  2007/08/05 02:27:19  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1.2.3  2007/07/31 03:18:01  kermin
 * Merge Complete - I hope
 *
 * Revision 1.1.1.1.2.2  2007/07/28 19:50:39  kermin
 * Merged in the main line
 *
 * Revision 1.1.1.1  2007/05/30 04:27:29  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:11  gtkwave
 * initial release
 *
 */

