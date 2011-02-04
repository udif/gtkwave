#ifndef BROWSE_WAVELINK_H
#define BROWSE_WAVELINK_H

/* kill the GLOBALS_H header inclusion as it's not needed here */
#define GLOBALS_H

#include "../../src/ae2.h"
#include "../../src/debug.h"
#include "../../src/helpers/vzt_read.h"
#include "../../src/helpers/lxt2_read.h"
#include <fstapi.h>

extern struct vzt_rd_trace  *vzt;
extern struct lxt2_rd_trace *lx2;
extern void *fst;

#ifdef AET2_IS_PRESENT
extern AE2_HANDLE ae2;
#endif

#endif

/*
 * $Id: wavelink.h,v 1.3 2009/06/13 22:02:18 gtkwave Exp $
 * $Log: wavelink.h,v $
 * Revision 1.3  2009/06/13 22:02:18  gtkwave
 * beginning to add FST support to rtlbrowse
 *
 * Revision 1.2  2007/08/26 21:35:39  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.1  2007/08/18 21:51:56  gtkwave
 * widget destroys and teardown of file formats which use external loaders
 * and are outside of malloc_2/free_2 control
 *
 * Revision 1.1.1.1  2007/05/30 04:25:38  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:10  gtkwave
 * initial release
 *
 */

