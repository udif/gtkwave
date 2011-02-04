/* 
 * Copyright (c) Tony Bybell 2003-2010.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"

#ifndef WAVE_LX2RDR_H
#define WAVE_LX2RDR_H

#ifdef HAVE_INTTYPES_H
#include <inttypes.h>
#endif

#include "vcd.h"
#include "ae2.h"

#define F_NAME_MODULUS (3)

enum LXT2_Loader_Type_Encodings { LXT2_IS_INACTIVE, LXT2_IS_LXT2, LXT2_IS_VZT, LXT2_IS_AET2, LXT2_IS_VLIST, LXT2_IS_FST };

#ifdef WAVE_USE_STRUCT_PACKING
#pragma pack(push)
#pragma pack(1)
#endif

struct lx2_entry
{
struct HistEnt *histent_head, *histent_curr;
int numtrans;
nptr np;
};

#ifdef WAVE_USE_STRUCT_PACKING
#pragma pack(pop)
#endif


TimeType lx2_main(char *fname, char *skip_start, char *skip_end);
void import_lx2_trace(nptr np);

void lx2_set_fac_process_mask(nptr np);
void lx2_import_masked(void);

#endif

/*
 * $Id: lx2.h,v 1.6 2010/12/17 06:29:20 gtkwave Exp $
 * $Log: lx2.h,v $
 * Revision 1.6  2010/12/17 06:29:20  gtkwave
 * Added --enable-struct-pack configure flag
 *
 * Revision 1.5  2010/09/15 18:35:42  gtkwave
 * added F_NAME_MODULUS to reduce temp memory usage
 *
 * Revision 1.4  2010/04/27 23:10:56  gtkwave
 * made inttype.h inclusion conditional
 *
 * Revision 1.3  2009/06/07 08:40:44  gtkwave
 * adding FST support
 *
 * Revision 1.2  2007/08/26 21:35:41  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.2  2007/08/25 19:43:45  gtkwave
 * header cleanups
 *
 * Revision 1.1.1.1.2.1  2007/08/05 02:27:21  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1  2007/05/30 04:27:21  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:13  gtkwave
 * initial release
 *
 */

