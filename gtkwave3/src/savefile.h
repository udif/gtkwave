/*
 * Copyright (c) Tony Bybell 2012.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"
#ifdef MAC_INTEGRATION
#include <gtkosxapplication.h>
#endif

#ifndef __WAVE_SAVEFILE_H__
#define __WAVE_SAVEFILE_H__

/* These should eventually have error values */
void write_save_helper(char *savnam, FILE *file);
int read_save_helper(char *wname, char **dumpfile, char **savefile); /* -1 = error, 0+ = number of lines read */
char *append_array_row(nptr n);

int parsewavline(char *w, char *alias, int depth);
int parsewavline_lx2(char *w, char *alias, int depth);

char *find_dumpfile(char *orig_save, char *orig_dump, char *this_save);

#ifdef MAC_INTEGRATION
gboolean deal_with_finder_open(GtkOSXApplication *app, gchar *path, gpointer user_data);
gboolean deal_with_termination(GtkOSXApplication *app, gpointer user_data);

gboolean process_finder_names_queued(void);
char *process_finder_extract_queued_name(void);
gboolean process_finder_name_integration(void);
#endif

#endif
