/* 
 * Copyright (c) Tony Bybell 2005.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"

#ifndef GHW_H
#define GHW_H

#include <limits.h>
#include "tree.h"
#include "vcd.h"

#define WAVE_GHW_DUMMYFACNAME "!!__(dummy)__!!"

TimeType ghw_main(char *fname);
int strand_pnt(char *s);

#endif

/*
 * $Id: ghw.h,v 1.3 2009/12/29 07:07:49 gtkwave Exp $
 * $Log: ghw.h,v $
 * Revision 1.3  2009/12/29 07:07:49  gtkwave
 * fixes for ghw files
 *
 * Revision 1.2  2007/08/26 21:35:40  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.2  2007/08/25 19:43:45  gtkwave
 * header cleanups
 *
 * Revision 1.1.1.1.2.1  2007/08/05 02:27:20  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1  2007/05/30 04:27:22  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:12  gtkwave
 * initial release
 *
 */

