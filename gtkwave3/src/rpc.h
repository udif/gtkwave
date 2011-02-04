/*
 * Copyright (c) Tony Bybell 2010.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef WAVE_RPC_H
#define WAVE_RPC_H

#define RPC_MATCHWORD "RPCM"

struct gtkwave_rpc_ipc_t  
{
char matchword[4];                      /* match against RPC_MATCHWORD string */
unsigned valid : 1;
unsigned resp_valid : 1;
char membuf[65536];
char resp_membuf[65536];
}; 


gboolean execute_rpc(void);

#endif

