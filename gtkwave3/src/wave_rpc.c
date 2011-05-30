/* 
 * Copyright (c) Tony Bybell 2010.
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
if(GLOBALS->rpc_ctx && !GLOBALS->tcl_running && !GLOBALS->busy_busy_c_1)
	{
	static int in_rpc = FALSE;

	if(GLOBALS->rpc_ctx->valid && !GLOBALS->rpc_ctx->resp_valid && !in_rpc)
		{
		char *s = GLOBALS->rpc_ctx->membuf;
		char *e = GLOBALS->rpc_ctx->membuf + sizeof(GLOBALS->rpc_ctx->membuf);
		char *s2;

		in_rpc = TRUE;
		GLOBALS->rpc_ctx->resp_membuf[0] = 0;
		
		while(s && *s)
			{
			WAVE_S2_S_MACRO

			if(!strcmp(s2, "--dump"))
				{
				WAVE_S2_S_MACRO

				if(!menu_new_viewer_tab_cleanup_2(s2)) 
					{
					strcat(GLOBALS->rpc_ctx->resp_membuf, "--dump fail\n");
					break;
					}
					else
					{
					strcat(GLOBALS->rpc_ctx->resp_membuf, "--dump ok\n");
					}
				}
			else
			if(!strcmp(s2, "--save"))
				{
				int rsh_rc;
				char rsh_buf[32];

				WAVE_S2_S_MACRO

				rsh_rc = read_save_helper(s2);
				sprintf(rsh_buf, "--save %d\n", rsh_rc);
				strcat(GLOBALS->rpc_ctx->resp_membuf, rsh_buf);
				}
			else
			if(!strcmp(s2, "--script"))
				{
				char *srtn;

				WAVE_S2_S_MACRO

				srtn = rpc_script_execute(s2);
				strcat(GLOBALS->rpc_ctx->resp_membuf, srtn);
				free_2(srtn);
				}
			else
				{
				break;
				}
			}

		GLOBALS->rpc_ctx->resp_valid = 1;
		GLOBALS->rpc_ctx->valid = 0;

		in_rpc = FALSE;
		return(TRUE);
		}
	}

return(FALSE);
}


