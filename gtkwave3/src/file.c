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
#include "wavealloca.h"
#include <string.h>
#include <stdlib.h>

#ifdef __linux__
extern char *canonicalize_file_name (__const char *__name);
#endif

static void enter_callback(GtkWidget *widget, GtkFileSelection *fw)
{
G_CONST_RETURN char *allocbuf;
int alloclen;

allocbuf=gtk_file_selection_get_filename(GTK_FILE_SELECTION(GLOBALS->fs_file_c_1));
if((alloclen=strlen(allocbuf)))
	{
	GLOBALS->filesel_ok=1;
	if(*GLOBALS->fileselbox_text) free_2(*GLOBALS->fileselbox_text);
	*GLOBALS->fileselbox_text=(char *)malloc_2(alloclen+1);
	strcpy(*GLOBALS->fileselbox_text, allocbuf);
	}

DEBUG(printf("Filesel OK %s\n",allocbuf));
gtk_grab_remove(GLOBALS->fs_file_c_1);
gtk_widget_destroy(GLOBALS->fs_file_c_1);
gtkwave_main_iteration();
GLOBALS->cleanup_file_c_2();
}

static void cancel_callback(GtkWidget *widget, GtkWidget *nothing)
{
DEBUG(printf("Filesel Entry Cancel\n"));
gtk_grab_remove(GLOBALS->fs_file_c_1);
gtk_widget_destroy(GLOBALS->fs_file_c_1);
gtkwave_main_iteration();
if(GLOBALS->bad_cleanup_file_c_1) GLOBALS->bad_cleanup_file_c_1();
}

static void destroy_callback(GtkWidget *widget, GtkWidget *nothing)
{
DEBUG(printf("Filesel Destroy\n"));
gtkwave_main_iteration();
if(GLOBALS->bad_cleanup_file_c_1) GLOBALS->bad_cleanup_file_c_1();
}

void fileselbox_old(char *title, char **filesel_path, GtkSignalFunc ok_func, GtkSignalFunc notok_func, char *pattn)
{
GLOBALS->fileselbox_text=filesel_path;
GLOBALS->filesel_ok=0;
GLOBALS->cleanup_file_c_2=ok_func;
GLOBALS->bad_cleanup_file_c_1=notok_func;

GLOBALS->fs_file_c_1=gtk_file_selection_new(title);
gtkwave_signal_connect(GTK_OBJECT(GLOBALS->fs_file_c_1), "destroy", (GtkSignalFunc) destroy_callback, NULL);
gtkwave_signal_connect(GTK_OBJECT(GTK_FILE_SELECTION(GLOBALS->fs_file_c_1)->ok_button), "clicked", (GtkSignalFunc) enter_callback, GTK_OBJECT(GLOBALS->fs_file_c_1));
gtkwave_signal_connect_object(GTK_OBJECT(GTK_FILE_SELECTION(GLOBALS->fs_file_c_1)->cancel_button),"clicked", (GtkSignalFunc) cancel_callback, GTK_OBJECT(GLOBALS->fs_file_c_1));
gtk_file_selection_hide_fileop_buttons(GTK_FILE_SELECTION(GLOBALS->fs_file_c_1));
if(*GLOBALS->fileselbox_text) gtk_file_selection_set_filename(GTK_FILE_SELECTION(GLOBALS->fs_file_c_1), *GLOBALS->fileselbox_text);

/*
 * XXX: filter on patterns when this feature eventually works (or for a later version of GTK)!
 * if((pattn)&&(pattn[0])) gtk_file_selection_complete(GTK_FILE_SELECTION(fs), pattn);
 */

gtk_widget_show(GLOBALS->fs_file_c_1);
gtk_grab_add(GLOBALS->fs_file_c_1);
}


void fileselbox(char *title, char **filesel_path, GtkSignalFunc ok_func, GtkSignalFunc notok_func, char *pattn, int is_writemode)
{
int can_set_filename = 0;
#if GTK_CHECK_VERSION(2,4,0)
GtkWidget *pFileChoose;
GtkWidget *pWindowMain;
GtkFileFilter *filter;
#endif

if(!*filesel_path) /* if no name specified, hijack loaded file name path */
	{
	if(GLOBALS->loaded_file_name)
		{
		char *tname = strdup_2(GLOBALS->loaded_file_name);
		char *delim = strrchr(tname, '/');
		if(!delim) delim =  strrchr(tname, '\\');
		if(delim) 
			{ 
			*(delim+1) = 0; /* keep slash for gtk_file_chooser_set_filename vs gtk_file_chooser_set_current_folder test below */
			*filesel_path = tname;
			}
			else
			{
			free_2(tname);
			}
		}
	}

 
if(GLOBALS->wave_script_args)
	{
	char *s = NULL;

	GLOBALS->fileselbox_text=filesel_path;
	GLOBALS->filesel_ok=1;

	while((!s)&&(GLOBALS->wave_script_args->curr)) s = wave_script_args_fgetmalloc_stripspaces(GLOBALS->wave_script_args);

	if(*GLOBALS->fileselbox_text) free_2(*GLOBALS->fileselbox_text); 
	if(!s)
		{
		fprintf(stderr, "Null filename passed to fileselbox in script, exiting.\n");
		exit(255);
		}
	*GLOBALS->fileselbox_text = s;
	fprintf(stderr, "GTKWAVE | Filename '%s'\n", s);

	ok_func();	
	return;
	}


#if defined __MINGW32__ || !GTK_CHECK_VERSION(2,4,0)

fileselbox_old(title, filesel_path, ok_func, notok_func, pattn);
return;

#else


pWindowMain = GLOBALS->mainwindow;
GLOBALS->fileselbox_text=filesel_path;
GLOBALS->filesel_ok=0;

if(*GLOBALS->fileselbox_text && (!g_path_is_absolute(*GLOBALS->fileselbox_text)))
	{
#ifdef __linux__
	char *can = canonicalize_file_name(*GLOBALS->fileselbox_text);

	if(can)
		{
		if(*GLOBALS->fileselbox_text) free_2(*GLOBALS->fileselbox_text);
	        *GLOBALS->fileselbox_text=(char *)malloc_2(strlen(can)+1);
	        strcpy(*GLOBALS->fileselbox_text, can);
		free(can);
		can_set_filename = 1;
		}

#else
#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED || defined __CYGWIN__ || defined HAVE_REALPATH
	char *can = realpath(*GLOBALS->fileselbox_text, NULL);

	if(can)
		{
		if(*GLOBALS->fileselbox_text) free_2(*GLOBALS->fileselbox_text);
	        *GLOBALS->fileselbox_text=(char *)malloc_2(strlen(can)+1);
	        strcpy(*GLOBALS->fileselbox_text, can);
		free(can);
		can_set_filename = 1;
		}
#else
#if __GNUC__
#warning Absolute file path warnings might be issued by the file chooser dialogue on this system!
#endif
#endif
#endif
	}
	else
	{
	if(*GLOBALS->fileselbox_text)
		{
		can_set_filename = 1;
		}
	}

if(is_writemode)
	{
	pFileChoose = gtk_file_chooser_dialog_new(
		title,
	        NULL,
	        GTK_FILE_CHOOSER_ACTION_SAVE,
	        GTK_STOCK_CANCEL,
	        GTK_RESPONSE_CANCEL,
	        GTK_STOCK_SAVE,
	        GTK_RESPONSE_ACCEPT,
	        NULL);

#if GTK_CHECK_VERSION(2,8,0)
	gtk_file_chooser_set_do_overwrite_confirmation(GTK_FILE_CHOOSER (pFileChoose), TRUE);
#endif
	}
	else
	{
	pFileChoose = gtk_file_chooser_dialog_new(
		title,
	        NULL,
	        GTK_FILE_CHOOSER_ACTION_OPEN,
	        GTK_STOCK_CANCEL,
	        GTK_RESPONSE_CANCEL,
	        GTK_STOCK_OPEN,
	        GTK_RESPONSE_ACCEPT,
	        NULL);
	}

if((can_set_filename) && (*filesel_path)) 
	{
	int flen = strlen(*filesel_path);
	if(((*filesel_path)[flen-1]=='/') || ((*filesel_path)[flen-1]=='\\'))
		{
		gtk_file_chooser_set_current_folder(GTK_FILE_CHOOSER(pFileChoose), *filesel_path);
		}
		else
		{
		gtk_file_chooser_set_filename(GTK_FILE_CHOOSER(pFileChoose), *filesel_path);
		}
	}

if(pattn)
	{
	filter = gtk_file_filter_new();
	gtk_file_filter_add_pattern(filter, pattn);
	gtk_file_chooser_add_filter(GTK_FILE_CHOOSER(pFileChoose), filter);
	}

gtk_dialog_set_default_response(GTK_DIALOG(pFileChoose), GTK_RESPONSE_ACCEPT);

gtk_object_set_data(GTK_OBJECT(pFileChoose), "FileChooseWindow", pFileChoose);
gtk_container_set_border_width(GTK_CONTAINER(pFileChoose), 10);
gtk_window_set_position(GTK_WINDOW(pFileChoose), GTK_WIN_POS_CENTER);
gtk_window_set_modal(GTK_WINDOW(pFileChoose), TRUE);
gtk_window_set_policy(GTK_WINDOW(pFileChoose), FALSE, FALSE, FALSE);
gtk_window_set_resizable(GTK_WINDOW(pFileChoose), TRUE); /* some distros need this */
if(pWindowMain)
	{
	gtk_window_set_transient_for(GTK_WINDOW(pFileChoose), GTK_WINDOW(pWindowMain));
	}
gtk_widget_show(pFileChoose);
gtk_grab_add(pFileChoose);

if (gtk_dialog_run(GTK_DIALOG (pFileChoose)) == GTK_RESPONSE_ACCEPT)
	{
	G_CONST_RETURN char *allocbuf;
	int alloclen;

	allocbuf = gtk_file_chooser_get_filename (GTK_FILE_CHOOSER (pFileChoose));
	if((alloclen=strlen(allocbuf)))
	        {
	        GLOBALS->filesel_ok=1;
	        if(*GLOBALS->fileselbox_text) free_2(*GLOBALS->fileselbox_text);
	        *GLOBALS->fileselbox_text=(char *)malloc_2(alloclen+1);
	        strcpy(*GLOBALS->fileselbox_text, allocbuf);

		/* add missing suffix to write files */
		if(pattn && is_writemode)
			{
			char *s = *GLOBALS->fileselbox_text;
			char *s2;
			char *suffix = wave_alloca(strlen(pattn) + 1);					
			char *term;

			strcpy(suffix, pattn);
			while((*suffix) && (*suffix != '.')) suffix++;
			term = *suffix ? suffix+1 : suffix;
			while((*term) && (isalnum((int)(unsigned char)*term))) term++;
			*term = 0;

                        if(strlen(s) > strlen(suffix))
                                {
                                if(strcmp(s + strlen(s) - strlen(suffix), suffix))
                                        {
                                        goto fix_suffix;
                                        }
                                }
                                else
                                {
fix_suffix:                     s2 = malloc_2(strlen(s) + strlen(suffix) + 1);
                                strcpy(s2, s);
                                strcat(s2, suffix);
                                free_2(s);
				*GLOBALS->fileselbox_text = s2;
                                }
			}
	        }

	DEBUG(printf("Filesel OK %s\n",allocbuf));
	gtk_grab_remove(pFileChoose);
	gtk_widget_destroy(pFileChoose);

	gtkwave_main_iteration();
	ok_func();
	}
	else
	{
	DEBUG(printf("Filesel Entry Cancel\n"));
	gtk_grab_remove(pFileChoose);
	gtk_widget_destroy(pFileChoose);

	gtkwave_main_iteration();
	if(GLOBALS->bad_cleanup_file_c_1) notok_func();
	}
#endif
}

