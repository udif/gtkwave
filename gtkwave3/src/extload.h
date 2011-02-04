/* 
 * Copyright (c) Tony Bybell 2009.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"

#ifndef WAVE_EXTRDR_H
#define WAVE_EXTRDR_H

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#include "vcd.h"

#define EXTLOAD "EXTLOAD | "

TimeType 	extload_main(char *fname, char *skip_start, char *skip_end);
void 		import_extload_trace(nptr np);

#endif

/*
 * $Id: extload.h,v 1.2 2010/04/27 23:10:56 gtkwave Exp $
 * $Log: extload.h,v $
 * Revision 1.2  2010/04/27 23:10:56  gtkwave
 * made inttype.h inclusion conditional
 *
 * Revision 1.1  2009/01/27 07:04:28  gtkwave
 * added extload external process loader capability
 *
 *
 */
