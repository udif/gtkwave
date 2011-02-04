/*
 * Copyright (c) Tony Bybell 2011.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "debug.h"

#ifdef _WAVE_HAVE_JUDY
#include <Judy.h>
#else
#include "jrb.h"
#endif

#ifndef WAVE_TREE_COMP_H
#define WAVE_TREE_COMP_H

void iter_through_comp_name_table(void);
int add_to_comp_name_table(const char *s, int slen);

#endif

/*
 * $Id: tree_component.h,v 1.3 2011/01/21 22:40:29 gtkwave Exp $
 * $Log: tree_component.h,v $
 * Revision 1.3  2011/01/21 22:40:29  gtkwave
 * pass string lengths from api directly to code to avoid length calculations
 *
 * Revision 1.2  2011/01/18 03:06:29  gtkwave
 * added JRB support for component trees when Judy is not available
 *
 * Revision 1.1  2011/01/18 00:00:12  gtkwave
 * preliminary tree component support
 *
 */

