/*
 * Copyright (c) Tony Bybell 2005.     
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"

#ifndef WAVE_PTRANSLATE_H
#define WAVE_PTRANSLATE_H

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "fgetdynamic.h"
#include "debug.h"

#define PROC_FILTER_MAX (128)


void ptrans_searchbox(char *title);
void init_proctrans_data(void);
int install_proc_filter(int which);
void set_current_translate_proc(char *name);
void remove_all_proc_filters(void);

#endif

/*
 * $Id: ptranslate.h,v 1.4 2010/07/19 21:12:19 gtkwave Exp $
 * $Log: ptranslate.h,v $
 * Revision 1.4  2010/07/19 21:12:19  gtkwave
 * added file/proc/trans access functions to Tcl script interpreter
 *
 * Revision 1.3  2010/03/30 18:33:27  gtkwave
 * fixed cut and paste errors from file to proc
 *
 * Revision 1.2  2007/08/26 21:35:43  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.2  2007/08/25 19:43:46  gtkwave
 * header cleanups
 *
 * Revision 1.1.1.1.2.1  2007/08/05 02:27:23  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1  2007/05/30 04:27:20  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:13  gtkwave
 * initial release
 *
 */

