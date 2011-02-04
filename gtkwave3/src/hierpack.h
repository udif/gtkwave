/*
 * Copyright (c) Tony Bybell 2008-2011.     
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef WAVE_HIERPACK_H
#define WAVE_HIERPACK_H

#include "globals.h"

void init_facility_pack(void);
char *compress_facility(unsigned char *key, int len);
void freeze_facility_pack(void);

char *hier_decompress_flagged(char *n, int *was_packed);

#endif

/*
 * $Id: hierpack.h,v 1.5 2011/01/13 17:20:39 gtkwave Exp $
 * $Log: hierpack.h,v $
 * Revision 1.5  2011/01/13 17:20:39  gtkwave
 * rewrote hierarchy / facility packing code
 *
 * Revision 1.4  2010/03/01 19:19:50  gtkwave
 * more hier_pfx code movement into hierpack.c
 *
 * Revision 1.3  2010/03/01 05:16:26  gtkwave
 * move compressed hier tree traversal to hierpack
 *
 * Revision 1.2  2008/07/18 17:29:50  gtkwave
 * adding cvs headers
 *
 * Revision 1.1  2008/07/18 17:27:01  gtkwave
 * adding hierpack code   
 *
 */

