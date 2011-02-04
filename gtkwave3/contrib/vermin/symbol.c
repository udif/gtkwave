#include "inter.h"
#include "symbol.h"
#include "jrb.h"

struct i_symbol_scope *sym_base=NULL;	/* points to current scope, you look backward (up) from here */


/*
 * Simply look for a symbol's presence in the table...
 */
struct i_symbol *sym_find(char *s)
{
JRB node;
struct i_symbol_scope *ss = sym_base;

if(!ss)	/* this is only to avoid crashes */
	{
        ss = (struct i_symbol_scope *)calloc(1, sizeof(struct i_symbol_scope));
        ss->symtable = make_jrb();
        sym_base = ss;
	}

while(ss)
	{
	node = jrb_find_str(ss->symtable, s);
	if(node)
		{
		return((struct i_symbol *)(node->val.v));
		}
	ss=ss->parent;
	}

return(NULL); /* not found */
}


/*
 * Malloc a symbol and copy the name data into it.
 */
struct i_symbol *sym_make(char *s)
{
struct i_symbol *sym;

sym=(struct i_symbol *)calloc(1,sizeof(struct i_symbol));
sym->firstref=zzline;
strcpy(sym->name=(char *)malloc(strlen(s)+1),s);

return(sym);
}

/*
 * Add a symbol presence in the table...
 */
struct i_symbol *sym_add(char *s)
{
struct i_symbol *sym;
Jval j;

sym = sym_find(s);
if(!sym)
	{
	sym = sym_make(s);
	j.v = (void *)sym;
	jrb_insert_str(sym_base->symtable, sym->name, j);
	}

return(sym); 
}

/*
 * $Id: symbol.c,v 1.1.1.1 2007/05/30 04:25:46 gtkwave Exp $
 * $Log: symbol.c,v $
 * Revision 1.1.1.1  2007/05/30 04:25:46  gtkwave
 * Imported sources
 *
 * Revision 1.1  2007/04/21 21:08:51  gtkwave
 * changed from vertex to vermin
 *
 * Revision 1.2  2007/04/20 02:08:11  gtkwave
 * initial release
 *
 */

