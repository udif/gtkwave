/* 
 * Copyright (c) Tony Bybell 1999-2009.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"

#ifndef FGET_DYNAMIC_H
#define FGET_DYNAMIC_H

#include "vlist.h"

/* using alloca avoids having to preserve across contexts */
struct wave_script_args {
  struct wave_script_args *curr;
  struct wave_script_args *next;
  char payload[1];
};

char *fgetmalloc(FILE *handle);
char *fgetmalloc_stripspaces(FILE *handle);

char *wave_script_args_fgetmalloc(struct wave_script_args *wave_script_args);
char *wave_script_args_fgetmalloc_stripspaces(struct wave_script_args *wave_script_args);

#endif

/*
 * $Id: fgetdynamic.h,v 1.3 2009/12/15 23:40:59 gtkwave Exp $
 * $Log: fgetdynamic.h,v $
 * Revision 1.3  2009/12/15 23:40:59  gtkwave
 * removed old style scripts; also removed tempfiles for Tcl args
 *
 * Revision 1.2  2007/08/26 21:35:40  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.2  2007/08/25 19:43:45  gtkwave
 * header cleanups
 *
 * Revision 1.1.1.1.2.1  2007/08/05 02:27:19  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1  2007/05/30 04:27:22  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:11  gtkwave
 * initial release
 *
 */

