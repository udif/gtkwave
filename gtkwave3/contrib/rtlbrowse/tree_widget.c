/* 
 * Copyright (c) Tony Bybell 1999-2008.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <config.h>
#include <gtk/gtk.h>
#include "splay.h"

#if WAVE_USE_GTK2
#define set_winsize(w,x,y) gtk_window_set_default_size(GTK_WINDOW(w),(x),(y))
void create_toolbar(GtkWidget *table);
#else
#define set_winsize(w,x,y) gtk_widget_set_usize(GTK_WIDGET(w),(x),(y))
#endif

GtkWidget *notebook = NULL;

void bwmaketree(void);
void bwlogbox(char *title, int width, ds_Tree *t, int display_mode);
void setup_dnd(GtkWidget *wid);

static ds_Tree *selectedtree=NULL;

static int is_active=0;
GtkCTree *ctree_main=NULL;

static void select_row_callback(GtkWidget *widget, gint row, gint column,
        GdkEventButton *event, gpointer data)
{
ds_Tree *t=(ds_Tree *)gtk_clist_get_row_data(GTK_CLIST(ctree_main), row);
selectedtree=t;

if(t->filename)
        {
	/*
	printf("%s\n", t->fullname);
        printf("%s -> '%s' %d-%d\n\n", t->item, t->filename, t->s_line, t->e_line);
	*/

	bwlogbox(t->fullname, 640 + 8*8, t, 0);
        }
        else
        {
	/*
	printf("%s\n", t->fullname);
        printf("%s -> *MISSING*\n\n", t->item);
	*/
        }   
}

static void unselect_row_callback(GtkWidget *widget, gint row, gint column,
        GdkEventButton *event, gpointer data)
{
/* ds_Tree *t=(ds_Tree *)gtk_clist_get_row_data(GTK_CLIST(ctree_main), row); */
selectedtree=NULL;
}

int treebox_is_active(void)
{
return(is_active);
}


/***************************************************************************/

static GtkWidget *window;
static GtkWidget *tree;
static GtkSignalFunc cleanup;


static void destroy_callback(GtkWidget *widget, GtkWidget *nothing)
{
  is_active=0;
  gtk_widget_destroy(window);
  gtk_main_quit();
}



/*
 * mainline..
 */
void treebox(char *title, GtkSignalFunc func, GtkWidget *old_window)
{
    GtkWidget *scrolled_win;
    GtkWidget *frame2;
    GtkWidget *table;
    GtkCList  *clist;
#ifndef WAVE_USE_GTK2
    GtkWidget *frameh;
    GtkWidget *button5;
    GtkWidget *hbox;
#endif

    if(is_active) 
	{
	gdk_window_raise(window->window);
	return;
	}

    is_active=1;
    cleanup=func;

    /* create a new modal window */
    window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW (window), title);
    gtk_signal_connect(GTK_OBJECT (window), "delete_event",
                       (GtkSignalFunc) destroy_callback, NULL);
    set_winsize(window, 640, 600);


    table = gtk_table_new (256, 1, FALSE);
    gtk_widget_show (table);

#ifndef WAVE_USE_GTK2
    frame2 = gtk_frame_new (NULL);
    gtk_widget_show(frame2);
#else
    frame2 = gtk_hpaned_new();
    gtk_widget_show(frame2);

    notebook = gtk_notebook_new();
    gtk_notebook_set_tab_pos(GTK_NOTEBOOK(notebook), GTK_POS_TOP);
    gtk_notebook_set_show_tabs(GTK_NOTEBOOK(notebook), ~0); 
    gtk_notebook_set_show_border(GTK_NOTEBOOK(notebook), ~0);
    gtk_notebook_set_scrollable(GTK_NOTEBOOK(notebook), ~0);
    gtk_notebook_popup_enable(GTK_NOTEBOOK(notebook));
  
    gtk_widget_show(notebook);
    gtk_paned_pack2(GTK_PANED(frame2), notebook, TRUE, TRUE);
#endif

    gtk_table_attach (GTK_TABLE (table), frame2, 0, 1, 0, 255,
                        GTK_FILL | GTK_EXPAND,
                        GTK_FILL | GTK_EXPAND | GTK_SHRINK, 1, 1);

    tree=gtk_ctree_new(1,0);
    ctree_main=GTK_CTREE(tree);
    gtk_clist_set_column_auto_resize (GTK_CLIST (tree), 0, TRUE);
    gtk_widget_show(tree);

    clist=GTK_CLIST(tree);
    gtk_signal_connect_object (GTK_OBJECT (clist), "select_row",
                               GTK_SIGNAL_FUNC(select_row_callback),
                               NULL);
    gtk_signal_connect_object (GTK_OBJECT (clist), "unselect_row",
                               GTK_SIGNAL_FUNC(unselect_row_callback),
                               NULL);

    gtk_clist_freeze(clist);
    gtk_clist_clear(clist);

    bwmaketree();
    gtk_clist_thaw(clist);
    selectedtree=NULL;

    scrolled_win = gtk_scrolled_window_new (NULL, NULL);
#ifndef WAVE_USE_GTK2
    gtk_widget_set_usize( GTK_WIDGET (scrolled_win), -1, 300);
#else
    gtk_widget_set_usize( GTK_WIDGET (scrolled_win), 150, 300);
#endif
    gtk_scrolled_window_set_policy (GTK_SCROLLED_WINDOW (scrolled_win),
                                      GTK_POLICY_AUTOMATIC,
                                      GTK_POLICY_AUTOMATIC);
    gtk_widget_show(scrolled_win);
    gtk_container_add (GTK_CONTAINER (scrolled_win), GTK_WIDGET (tree));

#ifndef WAVE_USE_GTK2
    gtk_container_add (GTK_CONTAINER (frame2), scrolled_win);
#else
    gtk_paned_pack1(GTK_PANED(frame2), scrolled_win, TRUE, TRUE);
#endif

#ifndef WAVE_USE_GTK2
    frameh = gtk_frame_new (NULL);
    gtk_container_border_width (GTK_CONTAINER (frameh), 3);
    gtk_widget_show(frameh);
    gtk_table_attach (GTK_TABLE (table), frameh, 0, 1, 255, 256,
                        GTK_FILL | GTK_EXPAND,
                        GTK_FILL | GTK_EXPAND | GTK_SHRINK, 1, 1);


    hbox = gtk_hbox_new (FALSE, 1);
    gtk_widget_show (hbox);

    button5 = gtk_button_new_with_label (" Exit ");
    gtk_container_border_width (GTK_CONTAINER (button5), 3);
    gtk_signal_connect_object (GTK_OBJECT (button5), "clicked",
			       GTK_SIGNAL_FUNC(destroy_callback),
			       GTK_OBJECT (window));
    gtk_widget_show (button5);
    gtk_box_pack_start (GTK_BOX (hbox), button5, TRUE, TRUE, 0);

    gtk_container_add (GTK_CONTAINER (frameh), hbox);
#else
    create_toolbar(table);
#endif

    gtk_container_add (GTK_CONTAINER (window), table);

    gtk_widget_show(window);
    setup_dnd(window);
}

/*
 * $Id: tree_widget.c,v 1.10 2010/06/23 05:40:48 gtkwave Exp $
 * $Log: tree_widget.c,v $
 * Revision 1.10  2010/06/23 05:40:48  gtkwave
 * added dnd from gtkwave into rtlbrowse
 *
 * Revision 1.9  2008/12/20 05:45:03  gtkwave
 * gtk1 compatibility and -Wshadow warning fixes
 *
 * Revision 1.8  2008/11/24 03:26:52  gtkwave
 * warnings cleanups
 *
 * Revision 1.7  2008/11/18 05:37:34  gtkwave
 * code cleanups
 *
 * Revision 1.6  2008/11/18 04:15:26  gtkwave
 * prelim support for text searching in rtlbrowse
 *
 * Revision 1.5  2008/11/17 22:36:14  gtkwave
 * adding find widgets
 *
 * Revision 1.4  2008/11/16 02:17:47  gtkwave
 * rtlbrowse updates for single integrated window
 *
 * Revision 1.3  2008/11/12 19:49:42  gtkwave
 * changed usage of usize
 *
 * Revision 1.2  2007/08/26 21:35:39  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.1  2007/08/22 02:26:19  gtkwave
 * treebox fixes to match revised gtkwave headers
 *
 * Revision 1.1.1.1  2007/05/30 04:25:38  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:10  gtkwave
 * initial release
 *
 */

