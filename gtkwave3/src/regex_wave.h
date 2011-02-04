/*
 * Copyright (c) Tony Bybell 1999-2004.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"

#ifndef REGEX_WAVE_H
#define REGEX_WAVE_H

enum WaveRegexTypes { WAVE_REGEX_SEARCH, WAVE_REGEX_TREE, WAVE_REGEX_WILD, WAVE_REGEX_DND, WAVE_REGEX_TOTAL };

int wave_regex_compile(char *regex, int which);
int wave_regex_match(char *str, int which);

void *wave_regex_alloc_compile(char *regex);
int wave_regex_alloc_match(void *mreg, char *str);
void wave_regex_alloc_free(void *pnt);

#endif

/*
 * $Id: regex_wave.h,v 1.3 2008/09/22 03:27:03 gtkwave Exp $
 * $Log: regex_wave.h,v $
 * Revision 1.3  2008/09/22 03:27:03  gtkwave
 * more DND adds
 *
 * Revision 1.2  2007/08/26 21:35:44  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.2  2007/08/25 19:43:46  gtkwave
 * header cleanups
 *
 * Revision 1.1.1.1.2.1  2007/08/05 02:27:23  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1  2007/05/30 04:27:30  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:17  gtkwave
 * initial release
 *
 */

