/*
 * Copyright (c) Tony Bybell 2013.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <config.h>
#include "fsdb_wrapper_api.h"

#ifdef WAVE_FSDB_READER_IS_PRESENT

#ifdef NOVAS_FSDB
#undef NOVAS_FSDB
#endif

#include "ffrAPI.h"
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>

#ifndef FALSE
#define FALSE   0
#endif

#ifndef TRUE
#define TRUE    1
#endif

static bool_T __TreeCB(fsdbTreeCBType cb_type, void *client_data, void *tree_cb_data)
{
return(TRUE); /* currently unused along with var/scope traversal */
}

static bool_T __MyTreeCB(fsdbTreeCBType cb_type, void *client_data, void *tree_cb_data);


extern "C" void *fsdbReaderOpenFile(char *nam)
{
if(!ffrObject::ffrIsFSDB(nam))
	{
	return(NULL);
	}

ffrFSDBInfo fsdb_info;
ffrObject::ffrGetFSDBInfo(nam, fsdb_info);
if(fsdb_info.file_type != FSDB_FT_VERILOG)
	{
	return(NULL);
	}

ffrObject *fsdb_obj = ffrObject::ffrOpen3(nam);
if(!fsdb_obj)
	{
	return(NULL);
	}

fsdb_obj->ffrSetTreeCBFunc(__TreeCB, NULL);
    
if(fsdb_obj->ffrGetFileType() != FSDB_FT_VERILOG)
	{
        fsdb_obj->ffrClose();
	return(NULL);
	}

return((void *)fsdb_obj);
}


extern "C" void fsdbReaderReadScopeVarTree(void *ctx,void (*cb)(void *))
{
ffrObject *fsdb_obj = (ffrObject *)ctx;

fsdb_obj->ffrSetTreeCBFunc(__MyTreeCB, (void *) cb);
fsdb_obj->ffrReadScopeVarTree();
}


extern "C" int fsdbReaderGetMaxVarIdcode(void *ctx)
{
ffrObject *fsdb_obj = (ffrObject *)ctx;
fsdbVarIdcode max_var_idcode = fsdb_obj->ffrGetMaxVarIdcode();
return(max_var_idcode);
}


extern "C" void fsdbReaderAddToSignalList(void *ctx, int i)
{
ffrObject *fsdb_obj = (ffrObject *)ctx;
fsdb_obj->ffrAddToSignalList(i);
}


extern "C" void fsdbReaderLoadSignals(void *ctx)
{
ffrObject *fsdb_obj = (ffrObject *)ctx;
fsdb_obj->ffrLoadSignals();
}


extern "C" void *fsdbReaderCreateVCTraverseHandle(void *ctx, int i)
{
ffrObject *fsdb_obj = (ffrObject *)ctx;
ffrVCTrvsHdl hdl = fsdb_obj->ffrCreateVCTraverseHandle(i);
return((void *)hdl);
}


extern "C" int fsdbReaderHasIncoreVC(void *ctx, void *hdl)
{
ffrObject *fsdb_obj = (ffrObject *)ctx;
ffrVCTrvsHdl fsdb_hdl = (ffrVCTrvsHdl)hdl;
return(fsdb_hdl->ffrHasIncoreVC() == TRUE);
}


extern "C" void fsdbReaderFree(void *ctx, void *hdl)
{
ffrObject *fsdb_obj = (ffrObject *)ctx;
ffrVCTrvsHdl fsdb_hdl = (ffrVCTrvsHdl)hdl;

fsdb_hdl->ffrFree();
}


extern "C" uint64_t fsdbReaderGetMinXTag(void *ctx, void *hdl)
{
ffrObject *fsdb_obj = (ffrObject *)ctx;
ffrVCTrvsHdl fsdb_hdl = (ffrVCTrvsHdl)hdl;
fsdbTag64 timetag;

fsdb_hdl->ffrGetMinXTag((void*)&timetag);
uint64_t rv = (((uint64_t)timetag.H) << 32) | ((uint64_t)timetag.L);
return(rv);
}


extern "C" uint64_t fsdbReaderGetMaxXTag(void *ctx, void *hdl)
{
ffrObject *fsdb_obj = (ffrObject *)ctx;
ffrVCTrvsHdl fsdb_hdl = (ffrVCTrvsHdl)hdl;
fsdbTag64 timetag;

fsdb_hdl->ffrGetMaxXTag((void*)&timetag);
uint64_t rv = (((uint64_t)timetag.H) << 32) | ((uint64_t)timetag.L);
return(rv);
}


extern "C" void fsdbReaderGotoXTag(void *ctx, void *hdl, uint64_t tim)
{
ffrObject *fsdb_obj = (ffrObject *)ctx;
ffrVCTrvsHdl fsdb_hdl = (ffrVCTrvsHdl)hdl;
fsdbTag64 timetag;

timetag.H = (uint32_t)(tim >> 32);
timetag.L = (uint32_t)(tim & 0xFFFFFFFFUL);

fsdb_hdl->ffrGotoXTag((void*)&timetag);
}


extern "C" uint64_t fsdbReaderGetXTag(void *ctx, void *hdl)
{
ffrObject *fsdb_obj = (ffrObject *)ctx;
ffrVCTrvsHdl fsdb_hdl = (ffrVCTrvsHdl)hdl;
fsdbTag64 timetag;

fsdb_hdl->ffrGetXTag((void*)&timetag);
uint64_t rv = (((uint64_t)timetag.H) << 32) | ((uint64_t)timetag.L);
return(rv);
}


extern "C" int fsdbReaderGetVC(void *ctx, void *hdl, void **val_ptr)
{
ffrObject *fsdb_obj = (ffrObject *)ctx;
ffrVCTrvsHdl fsdb_hdl = (ffrVCTrvsHdl)hdl;

return(fsdb_hdl->ffrGetVC((byte_T**)val_ptr) == FSDB_RC_SUCCESS);
}


extern "C" int fsdbReaderGotoNextVC(void *ctx, void *hdl)
{
ffrObject *fsdb_obj = (ffrObject *)ctx;
ffrVCTrvsHdl fsdb_hdl = (ffrVCTrvsHdl)hdl;

return(fsdb_hdl->ffrGotoNextVC() == FSDB_RC_SUCCESS);
}


extern "C" void fsdbReaderUnloadSignals(void *ctx)
{
ffrObject *fsdb_obj = (ffrObject *)ctx;
fsdb_obj->ffrUnloadSignals();
}


extern "C" void fsdbReaderClose(void *ctx)
{
ffrObject *fsdb_obj = (ffrObject *)ctx;
fsdb_obj->ffrClose();
}


extern "C" int fsdbReaderGetBytesPerBit(void *hdl)
{
ffrVCTrvsHdl fsdb_hdl = (ffrVCTrvsHdl)hdl;

return(fsdb_hdl->ffrGetBytesPerBit());
}


extern "C" int fsdbReaderGetBitSize(void *hdl)
{
ffrVCTrvsHdl fsdb_hdl = (ffrVCTrvsHdl)hdl;

return(fsdb_hdl->ffrGetBitSize());
}


extern "C" int fsdbReaderGetVarType(void *hdl)
{
ffrVCTrvsHdl fsdb_hdl = (ffrVCTrvsHdl)hdl;

return(fsdb_hdl->ffrGetVarType());
}


extern "C" char *fsdbReaderTranslateVC(void *hdl, void *val_ptr)
{ 
ffrVCTrvsHdl vc_trvs_hdl = (ffrVCTrvsHdl)hdl;
byte_T *vc_ptr = (byte_T *)val_ptr;

static byte_T buffer[FSDB_MAX_BIT_SIZE+1];
uint_T i;
fsdbVarType   var_type; 
    
switch (vc_trvs_hdl->ffrGetBytesPerBit()) 
	{
	case FSDB_BYTES_PER_BIT_1B:
	        for (i = 0; i < vc_trvs_hdl->ffrGetBitSize(); i++) 
			{
		    	switch(vc_ptr[i]) 
				{
	 	    		case FSDB_BT_VCD_0:
		        		buffer[i] = '0';
		        		break;
	
		    		case FSDB_BT_VCD_1:
		        		buffer[i] = '1';
		        		break;

		    		case FSDB_BT_VCD_X:
		        		buffer[i] = 'x';
		        		break;
	
		    		case FSDB_BT_VCD_Z:
		        		buffer[i] = 'z';
		        		break;
	
		    		default:
		        		buffer[i] = 'u';
					break;
		    		}
	        	}
	        buffer[i] = 0;
		break;

	case FSDB_BYTES_PER_BIT_4B:
		var_type = vc_trvs_hdl->ffrGetVarType();
		switch(var_type)
			{
			case FSDB_VT_VCD_MEMORY_DEPTH:
			case FSDB_VT_VHDL_MEMORY_DEPTH:
				buffer[0] = 0;
				break;
	               
			default:    
				vc_trvs_hdl->ffrGetVC(&vc_ptr);
				sprintf((char *)buffer, "%f", *((float*)vc_ptr));
				break;
			}
		break;
	
	case FSDB_BYTES_PER_BIT_8B:
		sprintf((char *)buffer, "%lg", *((double*)vc_ptr));
		break;

	default:
		buffer[0] = 0;
		break;
	}

return((char *)buffer);
}


extern "C" int fsdbReaderExtractScaleUnit(void *ctx, int *mult, char *scale)
{
ffrObject *fsdb_obj = (ffrObject *)ctx;
uint_T digit;
char *unit;

str_T su = fsdb_obj->ffrGetScaleUnit();
fsdbRC rc = fsdb_obj->ffrExtractScaleUnit(su, digit, unit);

if(rc == FSDB_RC_SUCCESS)
	{
	*mult = ((int)digit);
	*scale = unit[0];
	}

return(rc == FSDB_RC_SUCCESS);
}


extern "C" int fsdbReaderGetMinFsdbTag64(void *ctx, uint64_t *tim)
{
ffrObject *fsdb_obj = (ffrObject *)ctx;
fsdbTag64 tag64;
fsdbRC rc = fsdb_obj->ffrGetMinFsdbTag64(&tag64);

if(rc == FSDB_RC_SUCCESS)
	{
	*tim = (((uint64_t)tag64.H) << 32) | ((uint64_t)tag64.L);
	}

return(rc == FSDB_RC_SUCCESS);
}


extern "C" int fsdbReaderGetMaxFsdbTag64(void *ctx, uint64_t *tim)
{
ffrObject *fsdb_obj = (ffrObject *)ctx;
fsdbTag64 tag64;
fsdbRC rc = fsdb_obj->ffrGetMaxFsdbTag64(&tag64);

if(rc == FSDB_RC_SUCCESS)
	{
	*tim = (((uint64_t)tag64.H) << 32) | ((uint64_t)tag64.L);
	}

return(rc == FSDB_RC_SUCCESS);
}


static bool_T __fsdbReaderGetStatisticsCB(fsdbTreeCBType cb_type, void *client_data, void *tree_cb_data)
{
struct fsdbReaderGetStatistics_t *gs = (struct fsdbReaderGetStatistics_t *)client_data;

switch (cb_type)
	{
	case FSDB_TREE_CBT_VAR:		gs->varCount++;
					break;
	case FSDB_TREE_CBT_SCOPE:	gs->scopeCount++;
					break;

	default:			break;
	}

return(TRUE);
}


extern "C" struct fsdbReaderGetStatistics_t *fsdbReaderGetStatistics(void *ctx)
{
ffrObject *fsdb_obj = (ffrObject *)ctx;
struct fsdbReaderGetStatistics_t *gs = (struct fsdbReaderGetStatistics_t *)calloc(1, sizeof(struct fsdbReaderGetStatistics_t));

fsdb_obj->ffrSetTreeCBFunc(__fsdbReaderGetStatisticsCB, gs);
fsdb_obj->ffrReadScopeVarTree();

return(gs);
}


static void 
__DumpScope(fsdbTreeCBDataScope* scope, void (*cb)(void *))
{
str_T type;
char bf[65537];

switch (scope->type) 
	{
    	case FSDB_ST_VCD_MODULE:
		type = (str_T) "module"; 
		break;

	case FSDB_ST_VCD_TASK:
		type = (str_T) "task"; 
		break;

	case FSDB_ST_VCD_FUNCTION:
		type = (str_T) "function"; 
		break;
	
	case FSDB_ST_VCD_BEGIN:
		type = (str_T) "begin"; 
		break;
	
	case FSDB_ST_VCD_FORK:
		type = (str_T) "fork"; 
		break;

	case FSDB_ST_VCD_GENERATE:
		type = (str_T) "generate"; 
		break;

	default:
		type = (str_T) "unknown_scope_type";
		break;
    	}

sprintf(bf, "Scope: vcd_%s %s %s\n", type, scope->name, scope->module ? scope->module : "NULL");
cb(bf);
}


static void 
__DumpVar(fsdbTreeCBDataVar *var, void (*cb)(void *))
{
str_T type;
str_T bpb;
str_T direction;
char bf[65537];

switch(var->bytes_per_bit) 
	{
   	case FSDB_BYTES_PER_BIT_1B:
		bpb = (str_T) "1B";
		break;

    	case FSDB_BYTES_PER_BIT_2B:
		bpb = (str_T) "2B";
		break;

    	case FSDB_BYTES_PER_BIT_4B:
		bpb = (str_T) "4B";
		break;

    	case FSDB_BYTES_PER_BIT_8B:
		bpb = (str_T) "8B";
		break;

    	default:
		bpb = (str_T) "XB";
		break;
	}

switch (var->type) 
	{
    	case FSDB_VT_VCD_EVENT:
		type = (str_T) "vcd_event"; 
  		break;

    	case FSDB_VT_VCD_INTEGER:
		type = (str_T) "vcd_integer"; 
		break;

    	case FSDB_VT_VCD_PARAMETER:
		type = (str_T) "vcd_parameter"; 
		break;

    	case FSDB_VT_VCD_REAL:
		type = (str_T) "vcd_real"; 
		break;

    	case FSDB_VT_VCD_REG:
		type = (str_T) "vcd_reg"; 
		break;

    	case FSDB_VT_VCD_SUPPLY0:
		type = (str_T) "vcd_supply0"; 
		break;

    	case FSDB_VT_VCD_SUPPLY1:
		type = (str_T) "vcd_supply1"; 
		break;

    	case FSDB_VT_VCD_TIME:
		type = (str_T) "vcd_time";
		break;

    	case FSDB_VT_VCD_TRI:
		type = (str_T) "vcd_tri";
		break;

    	case FSDB_VT_VCD_TRIAND:
		type = (str_T) "vcd_triand";
		break;

    	case FSDB_VT_VCD_TRIOR:
		type = (str_T) "vcd_trior";
		break;

    	case FSDB_VT_VCD_TRIREG:
		type = (str_T) "vcd_trireg";
		break;

    	case FSDB_VT_VCD_TRI0:
		type = (str_T) "vcd_tri0";
		break;

    	case FSDB_VT_VCD_TRI1:
		type = (str_T) "vcd_tri1";
		break;

    	case FSDB_VT_VCD_WAND:
		type = (str_T) "vcd_wand";
		break;

    	case FSDB_VT_VCD_WIRE:
		type = (str_T) "vcd_wire";
		break;

    	case FSDB_VT_VCD_WOR:
		type = (str_T) "vcd_wor";
		break;

    	case FSDB_VT_VHDL_SIGNAL:
    	case FSDB_VT_VHDL_VARIABLE:
    	case FSDB_VT_VHDL_CONSTANT:
    	case FSDB_VT_VHDL_FILE:
    	case FSDB_VT_VCD_MEMORY:
    	case FSDB_VT_VHDL_MEMORY:
    	case FSDB_VT_VCD_MEMORY_DEPTH:
    	case FSDB_VT_VHDL_MEMORY_DEPTH:         
    	default:
		type = (str_T) "vcd_wire";
		break;
    	}

    switch(var->direction)
	{
	case FSDB_VD_INPUT:    	direction = (str_T) "input"; break;
	case FSDB_VD_OUTPUT:   	direction = (str_T) "output"; break;
	case FSDB_VD_INOUT:    	direction = (str_T) "inout"; break;
	case FSDB_VD_BUFFER:   	direction = (str_T) "buffer"; break;
	case FSDB_VD_LINKAGE:  	direction = (str_T) "linkage"; break;
	case FSDB_VD_IMPLICIT: 
	default:	       	direction = (str_T) "implicit"; break;
	}

    sprintf(bf, "Var: %s %s l:%d r:%d %s %d %s %d\n", type, var->name, var->lbitnum, var->rbitnum, 
		direction,
		var->u.idcode, bpb, var->dtidcode);

    cb(bf);
}


static bool_T __MyTreeCB(fsdbTreeCBType cb_type, 
			 void *client_data, void *tree_cb_data)
{
    void (*cb)(void *) = (void (*)(void *))client_data;
    char bf[16];

    switch (cb_type) {
    case FSDB_TREE_CBT_BEGIN_TREE:
	/* fprintf(stderr, "Begin Tree:\n"); */
	break;

    case FSDB_TREE_CBT_SCOPE:
	__DumpScope((fsdbTreeCBDataScope*)tree_cb_data, cb);
	break;

    case FSDB_TREE_CBT_VAR:
	__DumpVar((fsdbTreeCBDataVar*)tree_cb_data, cb);
	break;

    case FSDB_TREE_CBT_UPSCOPE:
	strcpy(bf, "Upscope:\n");
	cb(bf);
	break;

    case FSDB_TREE_CBT_END_TREE:
	/* fprintf(stderr, "End Tree:\n"); */
	break;

    case FSDB_TREE_CBT_FILE_TYPE:
    case FSDB_TREE_CBT_SIMULATOR_VERSION:
    case FSDB_TREE_CBT_SIMULATION_DATE:
    case FSDB_TREE_CBT_X_AXIS_SCALE:
    case FSDB_TREE_CBT_END_ALL_TREE:
    case FSDB_TREE_CBT_ARRAY_BEGIN:
    case FSDB_TREE_CBT_ARRAY_END:
    case FSDB_TREE_CBT_RECORD_BEGIN:
    case FSDB_TREE_CBT_RECORD_END:
        break;
             
    default:
	return FALSE;
    }

    return TRUE;
}


#else

static void dummy_compilation_unit(void)
{

}

#endif
