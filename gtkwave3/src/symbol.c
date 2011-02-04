/* 
 * Copyright (c) Tony Bybell 2001-2010
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include <config.h>
#include "globals.h"
#include <stdio.h>

#ifndef _MSC_VER
#include <unistd.h>
#ifndef __MINGW32__ 
#include <sys/mman.h>
#endif
#else
#include <windows.h>
#endif

#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "symbol.h"
#include "vcd.h"
#include "lxt.h"
#include "bsearch.h"
#include "hierpack.h"

#ifdef _WAVE_HAVE_JUDY
#include <Judy.h>
#endif

/*
 * s_selected accessors
 */
#ifdef _WAVE_HAVE_JUDY

char get_s_selected(struct symbol *s)
{
int rc = Judy1Test(GLOBALS->s_selected, (Word_t)s, PJE0);

return(rc);
}

char set_s_selected(struct symbol *s, char value)
{
if(value)
	{
	Judy1Set ((Pvoid_t)&GLOBALS->s_selected, (Word_t)s, PJE0);
	}
	else
	{
	Judy1Unset ((Pvoid_t)&GLOBALS->s_selected, (Word_t)s, PJE0);
	}

return(value);
}

void destroy_s_selected(void)
{
Judy1FreeArray(&GLOBALS->s_selected, PJE0);

GLOBALS->s_selected = NULL;
}

#else

char get_s_selected(struct symbol *s)
{
return(s->s_selected);
}

char set_s_selected(struct symbol *s, char value)
{
return((s->s_selected = value));
}

void destroy_s_selected(void)
{
/* nothing */
}

#endif


/*
 * hash create/destroy
 */
void sym_hash_initialize(void *g)
{
#ifdef _WAVE_HAVE_JUDY
((struct Global *)g)->sym_judy = NULL;
#else
((struct Global *)g)->sym_hash=(struct symbol **)calloc_2(SYMPRIME,sizeof(struct symbol *));
#endif
}


void sym_hash_destroy(void *g)
{
struct Global *gg = (struct Global *)g;

#ifdef _WAVE_HAVE_JUDY

JudySLFreeArray(&gg->sym_judy, PJE0);
gg->sym_judy = NULL;

#else

if(gg->sym_hash)
	{
	free_2(gg->sym_hash); gg->sym_hash = NULL;
	}

#endif
}


/*
 * Generic hash function for symbol names...
 */
int hash(char *s)
{
#ifndef _WAVE_HAVE_JUDY
char *p;
char ch;
unsigned int h=0, h2=0, pos=0, g;
for(p=s;*p;p++)
        {
	ch=*p;
	h2<<=3;
	h2-=((unsigned int)ch+(pos++));		/* this handles stranded vectors quite well.. */

        h=(h<<4)+ch;
        if((g=h&0xf0000000))
                {
                h=h^(g>>24);
                h=h^g;
                }   
        }

h^=h2;						/* combine the two hashes */
GLOBALS->hashcache=h%SYMPRIME;
#endif
return(GLOBALS->hashcache);
}


/*
 * add symbol to table.  no duplicate checking
 * is necessary as aet's are "correct."
 */
struct symbol *symadd(char *name, int hv)
{
struct symbol *s=(struct symbol *)calloc_2(1,sizeof(struct symbol));

#ifdef _WAVE_HAVE_JUDY

PPvoid_t PPValue = JudySLIns(&GLOBALS->sym_judy, (uint8_t *)name, PJE0);
*((struct symbol **)PPValue) = s;

#else

strcpy(s->name=(char *)malloc_2(strlen(name)+1),name);
s->sym_next=GLOBALS->sym_hash[hv];
GLOBALS->sym_hash[hv]=s;

#endif
return(s);
}

struct symbol *symadd_name_exists(char *name, int hv)
{
struct symbol *s=(struct symbol *)calloc_2(1,sizeof(struct symbol));

#ifdef _WAVE_HAVE_JUDY

PPvoid_t PPValue = JudySLIns(&GLOBALS->sym_judy, (uint8_t *)name, PJE0);
*((struct symbol **)PPValue) = s;

s->name = name; /* redundant for now */

#else

s->name = name;
s->sym_next=GLOBALS->sym_hash[hv];
GLOBALS->sym_hash[hv]=s;

#endif

return(s);
}

/*
 * find a slot already in the table...
 */
struct symbol *symfind(char *s, unsigned int *rows_return)
{
#ifndef _WAVE_HAVE_JUDY
int hv;
struct symbol *temp;
#endif

if(!GLOBALS->facs_are_sorted)
	{
#ifdef _WAVE_HAVE_JUDY
	PPvoid_t PPValue = JudySLGet(GLOBALS->sym_judy, (uint8_t *)s, PJE0);

	if(PPValue)
		{
		return(*(struct symbol **)PPValue);
		}
#else
	hv=hash(s);
	if(!(temp=GLOBALS->sym_hash[hv])) return(NULL); /* no hash entry, add here wanted to add */
	
	while(temp)
	        {
	        if(!strcmp(temp->name,s))
	                {
	                return(temp); /* in table already */    
	                }
	        if(!temp->sym_next) break;
	        temp=temp->sym_next;
	        }
#endif
	return(NULL); /* not found, add here if you want to add*/
	}
	else	/* no sense hashing if the facs table is built */
	{	
	struct symbol *sr;
	DEBUG(printf("BSEARCH: %s\n",s));

	sr = bsearch_facs(s, rows_return);
	if(sr)
		{
		}
		else
		{
		/* this is because . is > in ascii than chars like $ but . was converted to 0x1 on sort */
		char *s2;
		int i;
		int mat;

#ifndef WAVE_HIERFIX
		if(!GLOBALS->escaped_names_found_vcd_c_1)
			{
			return(sr);
			}
#endif

		if(GLOBALS->facs_have_symbols_state_machine == 0)
			{
			if(GLOBALS->escaped_names_found_vcd_c_1)
				{
				mat = 1;
				}
				else
				{
				mat = 0;

				for(i=0;i<GLOBALS->numfacs;i++)
				        {
				        int was_packed;
				        char *hfacname = NULL;
		
				        hfacname = hier_decompress_flagged(GLOBALS->facs[i]->name, &was_packed);
					s2 = hfacname;
					while(*s2)
						{
						if(*s2 < GLOBALS->hier_delimeter)
							{
							mat = 1;
							break;
							}
						s2++;
						}
	
				        if(was_packed) { free_2(hfacname); }
					if(mat) { break; }
				        }
				}

			if(mat) 
				{ GLOBALS->facs_have_symbols_state_machine = 1; }
				else
				{ GLOBALS->facs_have_symbols_state_machine = 2; } /* prevent code below from executing */
			}

		if(GLOBALS->facs_have_symbols_state_machine == 1)
			{
			mat = 0;
			for(i=0;i<GLOBALS->numfacs;i++)
			        {
			        int was_packed;
			        char *hfacname = NULL;
	
			        hfacname = hier_decompress_flagged(GLOBALS->facs[i]->name, &was_packed);
				if(!strcmp(hfacname, s))
					{
					mat = 1;
					}
	
			        if(was_packed) { free_2(hfacname); }
				if(mat)
					{
					sr = GLOBALS->facs[i];
					break;
					}
			        }
			}

		}

	return(sr);
	}
}

/*
 * $Id: symbol.c,v 1.13 2010/11/19 06:47:16 gtkwave Exp $
 * $Log: symbol.c,v $
 * Revision 1.13  2010/11/19 06:47:16  gtkwave
 * use PJE0 macro for unuser error return on judy function calls
 *
 * Revision 1.12  2010/03/18 17:12:37  gtkwave
 * pedantic warning cleanups
 *
 * Revision 1.11  2010/03/17 13:46:56  gtkwave
 * cast long to pointer to avoid warnings in Judy1Set
 *
 * Revision 1.10  2010/03/16 21:01:10  gtkwave
 * remove selected member of struct symbol
 *
 * Revision 1.9  2010/03/15 15:57:28  gtkwave
 * only allocate hash when necessary
 *
 * Revision 1.8  2010/03/15 03:14:53  gtkwave
 * deallocated symbol hash table after no longer needed
 *
 * Revision 1.7  2010/03/14 20:12:28  gtkwave
 * rename next hash pointer in struct symbol
 *
 * Revision 1.6  2010/01/23 21:26:38  gtkwave
 * additional fix for escaped names w/optimization
 *
 * Revision 1.5  2010/01/23 03:21:11  gtkwave
 * hierarchy fixes when characters < "." are in the signal names
 *
 * Revision 1.4  2010/01/22 02:10:49  gtkwave
 * added second pattern search capability
 *
 * Revision 1.3  2010/01/22 01:00:03  gtkwave
 * fix for bsearch_facs fail when $ is in name at same pos as . in another
 *
 * Revision 1.2  2007/08/26 21:35:45  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.4  2007/08/25 19:43:46  gtkwave
 * header cleanups
 *
 * Revision 1.1.1.1.2.3  2007/08/07 03:18:55  kermin
 * Changed to pointer based GLOBAL structure and added initialization function
 *
 * Revision 1.1.1.1.2.2  2007/08/06 03:50:49  gtkwave
 * globals support for ae2, gtk1, cygwin, mingw.  also cleaned up some machine
 * generated structs, etc.
 *
 * Revision 1.1.1.1.2.1  2007/08/05 02:27:23  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1  2007/05/30 04:27:26  gtkwave
 * Imported sources
 *
 * Revision 1.3  2007/05/28 00:55:06  gtkwave
 * added support for arrays as a first class dumpfile datatype
 *
 * Revision 1.2  2007/04/20 02:08:17  gtkwave
 * initial release
 *
 */

