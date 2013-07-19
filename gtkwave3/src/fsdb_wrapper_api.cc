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
return(TRUE); // currently unused along with var/scope traversal
}


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


extern "C" void fsdbReaderReadScopeVarTree(void *ctx)
{
ffrObject *fsdb_obj = (ffrObject *)ctx;
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


extern "C" int fsdbReaderGetBytesPerBit(void *ctx, void *hdl)
{
ffrObject *fsdb_obj = (ffrObject *)ctx;
ffrVCTrvsHdl fsdb_hdl = (ffrVCTrvsHdl)hdl;

return(fsdb_hdl->ffrGetBytesPerBit());
}


extern "C" int fsdbReaderGetBitSize(void *ctx, void *hdl)
{
ffrObject *fsdb_obj = (ffrObject *)ctx;
ffrVCTrvsHdl fsdb_hdl = (ffrVCTrvsHdl)hdl;

return(fsdb_hdl->ffrGetBitSize());
}


extern "C" int fsdbReaderGetVarType(void *ctx, void *hdl)
{
ffrObject *fsdb_obj = (ffrObject *)ctx;
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

#else

static void dummy_compilation_unit(void)
{

}

#endif
