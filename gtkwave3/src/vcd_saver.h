/*
 * Copyright (c) Tony Bybell 2005.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"

#ifndef VCD_SAVER_H
#define VCD_SAVER_H

#include "vcd.h"
#include "strace.h"

enum vcd_export_typ 		{ WAVE_EXPORT_VCD, WAVE_EXPORT_LXT, WAVE_EXPORT_TIM, WAVE_EXPORT_TRANS };
enum vcd_saver_rc 		{ VCDSAV_OK, VCDSAV_EMPTY, VCDSAV_FILE_ERROR };
enum vcd_saver_tr_datatype	{ VCDSAV_IS_BIN, VCDSAV_IS_HEX,  VCDSAV_IS_TEXT };

int save_nodes_to_export(const char *fname, int export_typ);
int do_timfile_save(const char *fname);
int save_nodes_to_trans(FILE *trans, Trptr t);

/* from helpers/scopenav.c */
extern void free_hier(void);
extern char *output_hier(char *name);

#endif

/*
 * $Id: vcd_saver.h,v 1.5 2010/03/31 15:42:47 gtkwave Exp $
 * $Log: vcd_saver.h,v $
 * Revision 1.5  2010/03/31 15:42:47  gtkwave
 * added preliminary transaction filter support
 *
 * Revision 1.4  2008/12/06 19:55:53  gtkwave
 * more adds to the timinganalyzer output writer
 *
 * Revision 1.3  2008/12/05 19:44:08  gtkwave
 * prelim support for timinganalyzer file format export
 *
 * Revision 1.2  2007/08/26 21:35:46  gtkwave
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

