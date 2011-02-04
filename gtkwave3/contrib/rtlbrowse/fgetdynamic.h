/* 
 * Copyright (c) Tony Bybell 1999.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef FGET_DYNAMIC_H
#define FGET_DYNAMIC_H

#include <stdio.h>
#include <stdlib.h>

struct alloc_bytechain
{
char val;
struct alloc_bytechain *next;
};

char *fgetmalloc(FILE *handle);
extern int fgetmalloc_len;

#endif

/*
 * $Id: fgetdynamic.h,v 1.1.1.1 2007/05/30 04:25:38 gtkwave Exp $
 * $Log: fgetdynamic.h,v $
 * Revision 1.1.1.1  2007/05/30 04:25:38  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:10  gtkwave
 * initial release
 *
 */

