#ifndef SYMBOL_H
#define SYMBOL_H

#include "vpp_common.h"
#include "jrb.h"

struct i_symbol_scope
{
struct i_symbol_scope *parent;
JRB symtable;
};


struct i_symbol *sym_find(char *s);
struct i_symbol *sym_make(char *s);
struct i_symbol *sym_add(char *s);

extern struct i_symbol_scope *sym_base;

/*
 * for defines, etc
 */
extern JRB define_tree;
extern struct ifdef_stack_t *ifdef_stack_top;

#endif

/*
 * $Id: symbol.h,v 1.1.1.1 2007/05/30 04:25:52 gtkwave Exp $
 * $Log: symbol.h,v $
 * Revision 1.1.1.1  2007/05/30 04:25:52  gtkwave
 * Imported sources
 *
 * Revision 1.1  2007/04/21 21:08:51  gtkwave
 * changed from vertex to vermin
 *
 * Revision 1.2  2007/04/20 02:08:11  gtkwave
 * initial release
 *
 */

