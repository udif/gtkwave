/*
 * Copyright (c) Tony Bybell 2006-2009.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"
#include <config.h>
#include "busy.h"

static void GuiDoEvent(GdkEvent *event)
{
if(!GLOBALS->busy_busy_c_1)
	{
	gtk_main_do_event(event);
	}
	else
	{
	/* filter out user input when we're "busy" */

	/* originally we allowed these two sets only... */

        /* usual expose events */
        /* case GDK_CONFIGURE: */
        /* case GDK_EXPOSE: */

        /* needed to keep dnd from hanging */
        /* case GDK_ENTER_NOTIFY: */
        /* case GDK_LEAVE_NOTIFY: */
        /* case GDK_FOCUS_CHANGE: */
        /* case GDK_MAP: */

	/* now it has been updated to remove keyboard/mouse input */

	switch (event->type) 
		{
		/* more may be needed to be added in the future */
		case GDK_MOTION_NOTIFY:
		case GDK_BUTTON_PRESS:
		case GDK_2BUTTON_PRESS:
		case GDK_3BUTTON_PRESS:
		case GDK_BUTTON_RELEASE:
		case GDK_KEY_PRESS:
		case GDK_KEY_RELEASE:
#if GTK_CHECK_VERSION(2,6,0)
		case GDK_SCROLL:
#endif
			/* printf("event->type: %d\n", event->type); */
			break;

		default:
	            	gtk_main_do_event(event);
			/* printf("event->type: %d\n", event->type); */
			break;
		}
	}
}


void gtkwave_main_iteration(void)
{
if(GLOBALS->partial_vcd)
	{
	while (gtk_events_pending()) gtk_main_iteration();
	}
	else
	{
	struct Global *g_old = GLOBALS;
	struct Global *gcache = NULL;

	set_window_busy(NULL);

	while (gtk_events_pending()) 
		{
		gtk_main_iteration();
		if(GLOBALS != g_old)
			{	
			/* this should never happen! */
			/* if it does, the program state is probably screwed */
			fprintf(stderr, "GTKWAVE | WARNING: globals changed during gtkwave_main_iteration()!\n");
			gcache = GLOBALS;
			}
		}
	
	set_GLOBALS(g_old);
	set_window_idle(NULL);
	
	if(gcache)
		{
		set_GLOBALS(gcache);
		}
	}
}


void init_busy(void)
{
GLOBALS->busycursor_busy_c_1 = gdk_cursor_new(GDK_WATCH);
gdk_event_handler_set((GdkEventFunc)GuiDoEvent, 0, 0);
}


void set_window_busy(GtkWidget *w)
{
int i;

/* if(GLOBALS->tree_dnd_begin) return; */

if(!GLOBALS->busy_busy_c_1)
	{
	if(w) gdk_window_set_cursor (w->window, GLOBALS->busycursor_busy_c_1);
	else
	if(GLOBALS->mainwindow) gdk_window_set_cursor (GLOBALS->mainwindow->window, GLOBALS->busycursor_busy_c_1);
	}

GLOBALS->busy_busy_c_1++;

for(i=0;i<GLOBALS->num_notebook_pages;i++)
        {
        (*GLOBALS->contexts)[i]->busy_busy_c_1 = GLOBALS->busy_busy_c_1;
        }

busy_window_refresh();
}


void set_window_idle(GtkWidget *w)
{
int i;

/* if(GLOBALS->tree_dnd_begin) return; */

if(GLOBALS->busy_busy_c_1)
	{
	if(GLOBALS->busy_busy_c_1 <= 1) /* defensively, in case something causes the value to go below zero */
		{
		if(w) gdk_window_set_cursor (w->window, NULL);
		else
		if(GLOBALS->mainwindow) gdk_window_set_cursor (GLOBALS->mainwindow->window, NULL);
		}

	GLOBALS->busy_busy_c_1--;

	for(i=0;i<GLOBALS->num_notebook_pages;i++)
	        {
	        (*GLOBALS->contexts)[i]->busy_busy_c_1 = GLOBALS->busy_busy_c_1;
	        }
	}
}


void busy_window_refresh(void)
{
if(GLOBALS->busy_busy_c_1)
	{
	while (gtk_events_pending()) gtk_main_iteration();
	}
}

/*
 * $Id: busy.c,v 1.14 2010/08/03 01:33:48 gtkwave Exp $
 * $Log: busy.c,v $
 * Revision 1.14  2010/08/03 01:33:48  gtkwave
 * error message to stderr
 *
 * Revision 1.13  2010/02/18 23:06:04  gtkwave
 * change name of main iteration loop calls
 *
 * Revision 1.12  2009/11/07 22:37:49  gtkwave
 * allow more events in GuiDoEvent
 *
 * Revision 1.11  2009/11/07 22:11:10  gtkwave
 * underflow handling on idle
 *
 * Revision 1.10  2008/01/13 02:16:27  gtkwave
 * re-enable busy pointer during dnd: allow more gdkevents to pass through
 *
 * Revision 1.9  2008/01/09 19:20:52  gtkwave
 * more updating to globals management (expose events cause wrong swap)
 *
 * Revision 1.8  2008/01/09 04:09:11  gtkwave
 * fix keyboard focus sighandler when multi-tabs are being used
 *
 * Revision 1.7  2008/01/05 22:25:46  gtkwave
 * degate busy during treeview dnd as it disrupts focus; dnd cleanups
 *
 * Revision 1.6  2007/09/14 14:08:56  gtkwave
 * updating busy handling
 *
 * Revision 1.5  2007/09/13 21:24:45  gtkwave
 * configure_events must be beyond watchdog monitoring due to how gtk generates one per tab
 *
 * Revision 1.4  2007/09/11 11:43:01  gtkwave
 * freeze-out tabs on partial vcd due to context swapping conflicts
 *
 * Revision 1.3  2007/09/11 02:12:47  gtkwave
 * context locking in busy spinloops (gtk_main_iteration() calls)
 *
 * Revision 1.2  2007/08/26 21:35:39  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.3  2007/08/07 03:18:54  kermin
 * Changed to pointer based GLOBAL structure and added initialization function
 *
 * Revision 1.1.1.1.2.2  2007/08/06 03:50:45  gtkwave
 * globals support for ae2, gtk1, cygwin, mingw.  also cleaned up some machine
 * generated structs, etc.
 *
 * Revision 1.1.1.1.2.1  2007/08/05 02:27:18  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1  2007/05/30 04:27:22  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:11  gtkwave
 * initial release
 *
 */

