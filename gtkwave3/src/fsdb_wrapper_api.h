/*
 * Copyright (c) Tony Bybell 2013.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#ifndef FSDB_WRAPPER_API_H
#define FSDB_WRAPPER_API_H

#if defined(FSDB_IS_PRESENT) && defined(FSDB_NSYS_IS_PRESENT)
#define WAVE_FSDB_READER_IS_PRESENT
#endif

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <inttypes.h>

void *fsdbReaderOpenFile(char *nam);
void fsdbReaderReadScopeVarTree(void *ctx);
int fsdbReaderGetMaxVarIdcode(void *ctx);
void fsdbReaderAddToSignalList(void *ctx, int i);
void fsdbReaderLoadSignals(void *ctx);
void *fsdbReaderCreateVCTraverseHandle(void *ctx, int i);
int fsdbReaderHasIncoreVC(void *ctx, void *hdl);
void fsdbReaderFree(void *ctx, void *hdl);
uint64_t fsdbReaderGetMinXTag(void *ctx, void *hdl);
uint64_t fsdbReaderGetMaxXTag(void *ctx, void *hdl);
void fsdbReaderGotoXTag(void *ctx, void *hdl, uint64_t tim);
uint64_t fsdbReaderGetXTag(void *ctx, void *hdl);
int fsdbReaderGetVC(void *ctx, void *hdl, void **val_ptr);
int fsdbReaderGotoNextVC(void *ctx, void *hdl);
void fsdbReaderUnloadSignals(void *ctx);
void fsdbReaderClose(void *ctx);
int fsdbReaderGetBytesPerBit(void *ctx, void *hdl);
int fsdbReaderGetBitSize(void *ctx, void *hdl);
int fsdbReaderGetVarType(void *ctx, void *hdl);

char *fsdbReaderTranslateVC(void *hdl, void *val_ptr);

#ifdef __cplusplus
}
#endif

#endif

