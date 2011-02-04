/* 
 * Copyright (c) Tony Bybell 1999.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"

#ifndef BSEARCH_NODES_VECTORS_H
#define BSEARCH_NODES_VECTORS_H

int bsearch_timechain(TimeType key);
int bsearch_aetinfo_timechain(TimeType key);
hptr bsearch_node(nptr n, TimeType key);
vptr bsearch_vector(bvptr b, TimeType key);
char *bsearch_trunc(char *ascii, int maxlen);
struct symbol *bsearch_facs(char *ascii, unsigned int *rows_return);

#endif

/*
 * $Id: bsearch.h,v 1.3 2008/02/19 22:56:11 gtkwave Exp $
 * $Log: bsearch.h,v $
 * Revision 1.3  2008/02/19 22:56:11  gtkwave
 * rtlbrowse update to handle aet time substitutions
 *
 * Revision 1.2  2007/08/26 21:35:39  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.2  2007/08/25 19:43:45  gtkwave
 * header cleanups
 *
 * Revision 1.1.1.1.2.1  2007/08/05 02:27:18  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1  2007/05/30 04:27:22  gtkwave
 * Imported sources
 *
 * Revision 1.3  2007/05/28 00:55:06  gtkwave
 * added support for arrays as a first class dumpfile datatype
 *
 * Revision 1.2  2007/04/20 02:08:11  gtkwave
 * initial release
 *
 */

