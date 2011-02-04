/*
 * Copyright (c) Tony Bybell 2005-2009
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"

#ifndef WAVE_PIPEIO_H
#define WAVE_PIPEIO_H

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include "debug.h"

#if defined _MSC_VER || defined __MINGW32__
#include <windows.h>
#endif


struct pipe_ctx
{
#if defined _MSC_VER || defined __MINGW32__

HANDLE g_hChildStd_IN_Rd;
HANDLE g_hChildStd_IN_Wr;  /* handle for gtkwave to write to */
HANDLE g_hChildStd_OUT_Rd; /* handle for gtkwave to read from */
HANDLE g_hChildStd_OUT_Wr;
PROCESS_INFORMATION piProcInfo;

#else

FILE *sin, *sout;
int fd0, fd1;
pid_t pid;

#endif
};


struct pipe_ctx *pipeio_create(char *execappname, char *arg);
void pipeio_destroy(struct pipe_ctx *p);
        
#endif

/*
 * $Id: pipeio.h,v 1.4 2010/08/25 22:58:23 gtkwave Exp $
 * $Log: pipeio.h,v $
 * Revision 1.4  2010/08/25 22:58:23  gtkwave
 * added process file support for mingw
 *
 * Revision 1.3  2009/09/14 03:00:08  gtkwave
 * bluespec code integration
 *
 * Revision 1.2  2007/08/26 21:35:43  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.2  2007/08/25 19:43:45  gtkwave
 * header cleanups
 *
 * Revision 1.1.1.1.2.1  2007/08/05 02:27:21  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1  2007/05/30 04:27:23  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:13  gtkwave
 * initial release
 *
 */

