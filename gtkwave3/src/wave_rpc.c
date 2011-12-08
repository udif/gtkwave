/* 
 * Copyright (c) Tony Bybell 2010-2012
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"
#include "debug.h"
#include "wave_rpc.h"

#define WAVE_S2_S_MACRO \
	s2 = s; \
	while(s != e) \
		{ \
		if(*s == '\n') \
			{ \
			*s = 0; \
			s++; \
			break; \
			} \
		s++; \
		} \
	if(s == e) break;


gboolean execute_rpc(void)
{
struct gtkwave_rpc_ipc_t *rpc_ctx = GLOBALS->rpc_ctx;
/* copy only in case GLOBALS->rpc_ctx somehow disappears -- which should *never* happen */

if(rpc_ctx && !GLOBALS->tcl_running && !GLOBALS->busy_busy_c_1)
	{
	static int in_rpc = FALSE;

	if(rpc_ctx->valid && !rpc_ctx->resp_valid && !in_rpc)
		{
		char *s = rpc_ctx->membuf;
		char *e = rpc_ctx->membuf + sizeof(rpc_ctx->membuf);
		char *s2;

		in_rpc = TRUE;
		rpc_ctx->resp_membuf[0] = 0;
		
		while(s && *s)
			{
			WAVE_S2_S_MACRO

			if(!strcmp(s2, "--dump"))
				{
				WAVE_S2_S_MACRO

				if(!menu_new_viewer_tab_cleanup_2(s2)) 
					{
					strcat(rpc_ctx->resp_membuf, "--dump fail\n");
					break;
					}
					else
					{
					strcat(rpc_ctx->resp_membuf, "--dump ok\n");
					}
				}
			else
			if(!strcmp(s2, "--save"))
				{
				int rsh_rc;
				char rsh_buf[32];

				WAVE_S2_S_MACRO

				rsh_rc = read_save_helper(s2, NULL);
				sprintf(rsh_buf, "--save %d\n", rsh_rc);
				strcat(rpc_ctx->resp_membuf, rsh_buf);
				}
			else
			if(!strcmp(s2, "--script"))
				{
				char *srtn;

				WAVE_S2_S_MACRO

				srtn = rpc_script_execute(s2);
				strcat(rpc_ctx->resp_membuf, srtn);
				free_2(srtn);
				}
			else
				{
				break;
				}
			}

		rpc_ctx->resp_valid = 1;
		rpc_ctx->valid = 0;

		in_rpc = FALSE;
		return(TRUE);
		}
	}

return(FALSE);
}


