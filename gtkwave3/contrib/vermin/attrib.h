#ifndef ATTRIB_H
#define ATTRIB_H

#define ZZA_STACKSIZE 100000
#define ZZAST_STACKSIZE 100000

extern int zzline, zzrewind;
extern int zzerrors;
extern int zzcomment_start, zzcomment_depth, zzcomment_entry;
extern char *zzfilename;
extern char *lineroot, *linepos;
extern char *prevlineroot;
extern int linemaxlen;

typedef union
{
        int ival;
        double rval;
        char *text;
        struct i_oper *oper;
        struct i_primary *prim;
        struct i_number *num;
	struct i_symbol *symbol;
	struct i_explist *explist;
} Attrib;

#define USER_ZZSYN

extern int do_not_translate, do_not_translate_mask;
#define STMODE do{if(do_not_translate)zzskip();}while(0)
#define STMODE_XLATEOFF_SYNOPSYS  1
#define STMODE_XLATEOFF_SYNTHESIS 2
#define STMODE_XLATEOFF_VERILINT  4
#define STMODE_XLATEOFF_VERTEX    8
#define STMODE_XLATEOFF_IFDEF     16

#endif

/*
 * $Id: attrib.h,v 1.2 2008/11/12 19:49:42 gtkwave Exp $
 * $Log: attrib.h,v $
 * Revision 1.2  2008/11/12 19:49:42  gtkwave
 * changed usage of usize
 *
 * Revision 1.1.1.1  2007/05/30 04:25:51  gtkwave
 * Imported sources
 *
 * Revision 1.1  2007/04/21 21:08:51  gtkwave
 * changed from vertex to vermin
 *
 * Revision 1.2  2007/04/20 02:08:10  gtkwave
 * initial release
 *
 */

