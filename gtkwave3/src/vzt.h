/* 
 * Copyright (c) Tony Bybell 2003-2004.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"

#ifndef WAVE_VZTRDR_H
#define WAVE_VZTRDR_H

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#include "vcd.h"


TimeType 	vzt_main(char *fname, char *skip_start, char *skip_end);
void 		import_vzt_trace(nptr np);
void 		vzt_set_fac_process_mask(nptr np);
void 		vzt_import_masked(void);

#endif

/*
 * $Id: vzt.h,v 1.3 2010/04/27 23:10:56 gtkwave Exp $
 * $Log: vzt.h,v $
 * Revision 1.3  2010/04/27 23:10:56  gtkwave
 * made inttype.h inclusion conditional
 *
 * Revision 1.2  2007/08/26 21:35:50  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.2  2007/08/25 19:43:46  gtkwave
 * header cleanups
 *
 * Revision 1.1.1.1.2.1  2007/08/05 02:27:28  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1  2007/05/30 04:27:50  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:18  gtkwave
 * initial release
 *
 */

