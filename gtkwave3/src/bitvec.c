/* 
 * Copyright (c) Tony Bybell 1999-2011.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

/* AIX may need this for alloca to work */ 
#if defined _AIX
  #pragma alloca
#endif

#include "globals.h" 
#include <config.h>
#include "analyzer.h"
#include "symbol.h"
#include "lxt.h"
#include "lx2.h"
#include "lxt2_read.h"
#include "vcd.h"
#include "extload.h"
#include "debug.h"
#include "bsearch.h"
#include "strace.h"
#include "translate.h"
#include "ptranslate.h"
#include "ttranslate.h"
#include "main.h"
#include "menu.h"
#include "busy.h"
#include "hierpack.h"
#include <stdlib.h>

/*
 * attempt to match top vs bottom rather than emit <Vector>
 */
char *attempt_vecmatch_2(char *s1, char *s2)
{
char *s;
char *p1, *p2;
char *n1=NULL, *n2=NULL;
int n1len = 0, n2len = 0;
char *pfx = NULL;
int pfxlen = 0;
int pfxstate = 0;
char *sfx = NULL;
int sfxlen = 0;
int totlen;
int idx;

if(!strcmp(s1, s2)) { s = malloc_2(strlen(s1)+1); strcpy(s, s1); return(s); }

p1 = s1; p2 = s2;
while(*p1 && *p2)
	{
	if(pfxstate==0)
		{
		if(*p1 == *p2)
			{
			pfx = p1;
			pfxlen = p1 - s1 + 1;
			p1++;
			p2++;
			continue;
			}

		if(!pfx) return(NULL);

		if(isdigit((int)(unsigned char)*p1)&&isdigit((int)(unsigned char)*p2))
			{
			n1 = p1; n2 = p2;
			while(*p1)
				{
				if(isdigit((int)(unsigned char)*p1))
					{
					n1len++;
					}
					else
					{
					break;
					}
				p1++;
				}

			while(*p2)
				{
				if(isdigit((int)(unsigned char)*p2))
					{
					n2len++;
					}
					else
					{
					break;
					}
				p2++;
				}

			if(*p1 && *p2)
				{
				pfxstate = 1;
				sfx = p1;
				continue;
				}
				else
				{
				break;
				}
			}
		}

	if(pfxstate==1)
		{
		if(*p1 == *p2)
			{
			sfxlen++;
			p1++;
			p2++;
			continue;
			}
			else
			{
			return(NULL);
			}
		}

	return(NULL);
	}

if((*p1)||(*p2)) return(NULL);

while(pfxlen>1)	/* backup if matching a sequence like 20..24 where the 2 matches outside of the left bracket */
	{
	if(isdigit((int)(unsigned char)s1[pfxlen-1]))
		{
		pfxlen--;
		n1--; n1len++;
		n2--; n2len++;
		}
		else
		{
		break;
		}
	}

totlen = pfxlen + 1 + n1len + 1 + n2len + 1 + sfxlen + 1;
s = malloc_2(totlen);
memcpy(s, s1, pfxlen);
idx = pfxlen;
if(!(pfxlen && (s1[pfxlen-1]=='[')))
	{
	s[idx] = '[';
	idx++;
	}
memcpy(s+idx, n1, n1len);
idx += n1len;
s[idx] = ':';
idx++;
memcpy(s+idx, n2, n2len);
idx += n2len;
if((!sfx) || (*sfx != ']'))
	{
	s[idx] = ']';
	idx++;
	}

if(sfxlen) { memcpy(s+idx, sfx, sfxlen); idx+=sfxlen; }
s[idx] = 0;

return(s);
}


char *attempt_vecmatch(char *s1, char *s2)
{
char *pnt = NULL;

if(!s1 || !s2)
	{
	return(pnt);
	}
	else
	{
	int ns1_was_decompressed = HIER_DEPACK_ALLOC;
	char *ns1 = hier_decompress_flagged(s1, &ns1_was_decompressed);
	int ns2_was_decompressed = HIER_DEPACK_ALLOC;
	char *ns2 = hier_decompress_flagged(s2, &ns2_was_decompressed);

	if(*ns1 && *ns2)
		{
		pnt = attempt_vecmatch_2(ns1, ns2);
		}

	if(ns1_was_decompressed) free_2(ns1);
	if(ns2_was_decompressed) free_2(ns2);

	return(pnt);
	}
}


/*
 * mvlfac resolution
 */
void import_trace(nptr np)
{
set_window_busy(NULL);

if(GLOBALS->is_lxt)
	{
	import_lxt_trace(np);
	}
else
if(GLOBALS->is_lx2)
	{
	import_lx2_trace(np);
	}
else
if(GLOBALS->extload)
	{
	import_extload_trace(np);
	}
else
	{
	fprintf(stderr, "Internal error with mvlfac trace handling, exiting.\n");
	exit(255);
	}

set_window_idle(NULL);
}


/*
 * turn a Bits structure into a vector with deltas for faster displaying
 */
bvptr bits2vector(struct Bits *b)
{
int i;
int regions=0;	
struct Node *n;
hptr *h;
vptr vhead=NULL, vcurr=NULL, vadd;
int numextrabytes;
TimeType mintime, lasttime=-1;
bvptr bitvec=NULL;
TimeType tshift, tmod;

if(!b) return(NULL);

h=(hptr *)calloc_2(b->nnbits, sizeof(hptr));

numextrabytes=(b->nnbits)-1;

for(i=0;i<b->nnbits;i++)
	{
	n=b->nodes[i];
	h[i]=&(n->head);
	}

while(h[0])	/* should never exit through this point the way we set up histents with trailers now */
	{
	mintime=MAX_HISTENT_TIME;

	vadd=(vptr)calloc_2(1,sizeof(struct VectorEnt)+numextrabytes);
	
	for(i=0;i<b->nnbits;i++) /* was 1...big mistake */
		{
		tshift = (b->attribs) ? b->attribs[i].shift : 0;

		if(h[i]->next)
			{
			if((h[i]->next->time >= 0) && (h[i]->next->time < MAX_HISTENT_TIME-2))
				{
				tmod = h[i]->next->time+tshift;
				if(tmod < 0) tmod = 0;
				if(tmod > MAX_HISTENT_TIME-2) tmod = MAX_HISTENT_TIME-2;
				}
				else
				{
				tmod = h[i]->next->time;	/* don't timeshift endcaps */
				}

			if(tmod < mintime)
				{	
				mintime = tmod;
				}
			}
		}

	vadd->time=lasttime;
	lasttime=mintime;	

	regions++;

	for(i=0;i<b->nnbits;i++)
		{
		unsigned char enc;

		tshift = (b->attribs) ? b->attribs[i].shift : 0;

		if((b->attribs)&&(b->attribs[i].flags & TR_INVERT))
			{
			enc  = ((unsigned char)(h[i]->v.h_val));
			switch(enc)	/* don't remember if it's preconverted in all cases; being conservative is OK */
				{
				case AN_0: case '0':
					enc = AN_1; break;

				case AN_1: case '1':
					enc = AN_0; break;

				case AN_H: case 'h': case 'H':
					enc = AN_L; break;

				case AN_L: case 'l': case 'L':
					enc = AN_H; break;

				case 'x': case 'X':
					enc = AN_X; break;

				case 'z': case 'Z':
					enc = AN_Z; break;

				case 'u': case 'U':
					enc = AN_U; break;

				case 'w': case 'W':
					enc = AN_W; break;

				default: enc = enc & AN_MSK; break;
				}
			}
			else
			{
			enc  = ((unsigned char)(h[i]->v.h_val)) & AN_MSK;
			}

		vadd->v[i] = enc;

		if(h[i]->next)
			{
			if((h[i]->next->time >= 0) && (h[i]->next->time < MAX_HISTENT_TIME-2))
				{
				tmod = h[i]->next->time+tshift;
				if(tmod < 0) tmod = 0;
				if(tmod > MAX_HISTENT_TIME-2) tmod = MAX_HISTENT_TIME-2;
				}
				else
				{
				tmod = h[i]->next->time;	/* don't timeshift endcaps */
				}

			if(tmod < mintime)
				{	
				mintime = tmod;
				}

			if(tmod == mintime)
				{
				h[i]=h[i]->next;
				}
			}
		}

	if(vhead)
		{
		vcurr->next=vadd;
		vcurr=vadd;
		}
		else
		{
		vhead=vcurr=vadd;
		}

	if((mintime==MAX_HISTENT_TIME)) break;	/* normal bail part */
	}

vadd=(vptr)calloc_2(1,sizeof(struct VectorEnt)+numextrabytes);
vadd->time=MAX_HISTENT_TIME;
for(i=0;i<=numextrabytes;i++) vadd->v[i]=AN_U; /* formerly 0x55 */
if(vcurr) { vcurr->next=vadd; } /* scan-build */
regions++;

bitvec=(bvptr)calloc_2(1,sizeof(struct BitVector)+((regions-1)*sizeof(vptr))); /* ajb : found "regions" by manual inspection, changed to "regions-1" as array is already [1] */

strcpy(bitvec->bvname=(char *)malloc_2(strlen(b->name)+1),b->name);
bitvec->nbits=b->nnbits;
bitvec->numregions=regions;

vcurr=vhead;
for(i=0;i<regions;i++)
	{
	bitvec->vectors[i]=vcurr;
	if(vcurr) { vcurr=vcurr->next; } /* scan-build */
	}

return(bitvec);
}


/*
 * Make solitary traces from wildcarded signals...
 */
int maketraces(char *str, char *alias, int quick_return)
{
char *pnt, *wild;
char ch, wild_active=0;
int len;
int i;
int made=0;
unsigned int rows = 0;

pnt=str;
while((ch=*pnt))
	{
	if(ch=='*') 
		{
		wild_active=1;
		break;
		}
	pnt++;
	}

if(!wild_active)	/* short circuit wildcard evaluation with bsearch */
	{
	struct symbol *s;
	nptr nexp;

	if(str[0]=='(')
		{
		for(i=1;;i++)
			{
			if(str[i]==0) return(0);
			if((str[i]==')')&&(str[i+1])) {i++; break; }
			}

		s=symfind(str+i, &rows);
		if(s)
			{
			nexp = ExtractNodeSingleBit(&s->n[rows], atoi(str+1));
			if(nexp)
				{
				AddNode(nexp, alias);
				return(~0);
				}
			}

		return(0);
		}
		else
		{
		if((s=symfind(str, &rows)))
			{
			AddNode(&s->n[rows],alias);
			return(~0);
			}
			else
			{
			return(0);
			}
		}
	}

while(1)
{
pnt=str;
len=0;

while(1)
	{
	ch=*pnt++;
	if(isspace((int)(unsigned char)ch)||(!ch)) break;
	len++;
	}

if(len)
	{
	wild=(char *)calloc_2(1,len+1);
	memcpy(wild,str,len);
	wave_regex_compile(wild, WAVE_REGEX_WILD);

	for(i=0;i<GLOBALS->numfacs;i++)
		{
		if(wave_regex_match(GLOBALS->facs[i]->name, WAVE_REGEX_WILD))
			{
			AddNode(GLOBALS->facs[i]->n,NULL);
			made=~0;
			if(quick_return) break;
			}
		}

	free_2(wild);
	}

if(!ch) break;
str=pnt;
}
return(made);
}


/*
 * Create a vector from wildcarded signals...
 */
struct Bits *makevec(char *vec, char *str)
{
char *pnt, *pnt2, *wild=NULL;
char ch, ch2, wild_active;
int len, nodepnt=0;
int i;
struct Node *n[BITATTRIBUTES_MAX];
struct Bits *b=NULL;
unsigned int rows = 0;

while(1)
{
pnt=str;
len=0;

while(1)
	{
	ch=*pnt++;
	if(isspace((int)(unsigned char)ch)||(!ch)) break;
	len++;
	}

if(len)
	{
	wild=(char *)calloc_2(1,len+1);
	memcpy(wild,str,len);

	DEBUG(printf("WILD: %s\n",wild));

	wild_active=0;
	pnt2=wild;
	while((ch2=*pnt2))
		{
		if(ch2=='*') 
			{
			wild_active=1;
			break;
			}
		pnt2++;
		}

	if(!wild_active)	/* short circuit wildcard evaluation with bsearch */
		{
		struct symbol *s;
		if(wild[0]=='(')
			{
			nptr nexp;
			
			for(i=1;;i++)
				{
				if(wild[i]==0) break;
				if((wild[i]==')')&&(wild[i+1])) 
					{
					i++; 
					s=symfind(wild+i, &rows);
					if(s)
						{
						nexp = ExtractNodeSingleBit(&s->n[rows], atoi(wild+1));
						if(nexp)
							{
							n[nodepnt++]=nexp;
							if(nodepnt==BITATTRIBUTES_MAX) { free_2(wild); goto ifnode; }
							}		
						}
					else
                                              	{
                                                char *lp = strrchr(wild+i, '[');
                                                if(lp)
                                                        {
                                                        char *ns = malloc_2(strlen(wild+i) + 32);
                                                        char *colon = strchr(lp+1, ':');
                                                        int msi, lsi, bval, actual;
                                                        *lp = 0;

                                                        bval = atoi(wild+1);
                                                        if(colon)
                                                                {
                                                                msi = atoi(lp+1);
                                                                lsi = atoi(colon+1);

                                                                if(lsi > msi)
                                                                        {
                                                                        actual = msi + bval;
                                                                        }
                                                                        else
                                                                        {
                                                                        actual = msi - bval;
                                                                        }
                                                                }
                                                                else
                                                                {
                                                                actual = bval; /* punt */
                                                                }

                                                        sprintf(ns, "%s[%d]", wild+i, actual); 
                                                        *lp = '[';

                                                        s=symfind(ns, &rows);
                                                        if(s)
                                                                {
								nexp =&s->n[rows];
								if(nexp)
									{
									n[nodepnt++]=nexp;
									if(nodepnt==BITATTRIBUTES_MAX) { free_2(wild); goto ifnode; }
									}
                                                                }

							free_2(ns);
                                                        }
						}
					break;
					}
				}
			}
			else
			{
			if((s=symfind(wild, &rows)))	
				{
				n[nodepnt++]=&s->n[rows];
				if(nodepnt==BITATTRIBUTES_MAX) { free_2(wild); goto ifnode; }
				}
			}
		}
		else
		{
		wave_regex_compile(wild, WAVE_REGEX_WILD);
		for(i=GLOBALS->numfacs-1;i>=0;i--)	/* to keep vectors in little endian hi..lo order */
			{
			if(wave_regex_match(GLOBALS->facs[i]->name, WAVE_REGEX_WILD))
				{
				n[nodepnt++]=GLOBALS->facs[i]->n;
				if(nodepnt==BITATTRIBUTES_MAX) { free_2(wild); goto ifnode; }
				}
			}
		}
	free_2(wild);
	}

if(!ch) break;
str=pnt;
}

ifnode:
if(nodepnt)
	{
	b=(struct Bits *)calloc_2(1,sizeof(struct Bits)+(nodepnt-1)*
				  sizeof(struct Node *));

	for(i=0;i<nodepnt;i++)
		{
		b->nodes[i]=n[i];
		if(n[i]->mv.mvlfac) import_trace(n[i]);
		}

	b->nnbits=nodepnt;
	strcpy(b->name=(char *)malloc_2(strlen(vec)+1),vec);
	}

return(b);
}

/*
 * Create an annotated (b->attribs) vector from stranded signals...
 */
struct Bits *makevec_annotated(char *vec, char *str)
{
char *pnt, *wild=NULL;
char ch;
int len, nodepnt=0;
int i;
struct Node *n[BITATTRIBUTES_MAX];
struct BitAttributes ba[BITATTRIBUTES_MAX];
struct Bits *b=NULL;
int state = 0;
unsigned int rows = 0;

memset(ba, 0, sizeof(ba)); /* scan-build */

while(1)
{
pnt=str;
len=0;

while(1)
	{
	ch=*pnt++;
	if(isspace((int)(unsigned char)ch)||(!ch)) break;
	len++;
	}

if(len)
	{
	wild=(char *)calloc_2(1,len+1);
	memcpy(wild,str,len);

	DEBUG(printf("WILD: %s\n",wild));

	if(state==1)
		{
		ba[nodepnt-1].shift = atoi_64(wild);
		state++;
		goto fw;		
		}
	else
	if(state==2)
		{
		sscanf(wild, "%x", &ba[nodepnt-1].flags);
		state = 0;
		goto fw;
		}

	state++;

	/* no wildcards for annotated! */
		{
		struct symbol *s;

		if(nodepnt==BITATTRIBUTES_MAX) { free_2(wild); goto ifnode; }

		if(wild[0]=='(')
			{
			nptr nexp;
			
			for(i=1;;i++)
				{
				if(wild[i]==0) break;
				if((wild[i]==')')&&(wild[i+1])) 
					{
					i++; 
					s=symfind(wild+i, &rows);
					if(s)
						{
						nexp = ExtractNodeSingleBit(&s->n[rows], atoi(wild+1));
						if(nexp)
							{
							n[nodepnt++]=nexp;
							if(nodepnt==BITATTRIBUTES_MAX) { free_2(wild); goto ifnode; }
							}		
						}
					else
                                              	{
                                                char *lp = strrchr(wild+i, '[');
                                                if(lp)
                                                        {
                                                        char *ns = malloc_2(strlen(wild+i) + 32);
                                                        char *colon = strchr(lp+1, ':');
                                                        int msi, lsi, bval, actual;
                                                        *lp = 0;

                                                        bval = atoi(wild+1);
                                                        if(colon)
                                                                {
                                                                msi = atoi(lp+1);
                                                                lsi = atoi(colon+1);

                                                                if(lsi > msi)
                                                                        {
                                                                        actual = msi + bval;
                                                                        }
                                                                        else
                                                                        {
                                                                        actual = msi - bval;
                                                                        }
                                                                }
                                                                else
                                                                {
                                                                actual = bval; /* punt */
                                                                }

                                                        sprintf(ns, "%s[%d]", wild+i, actual); 
                                                        *lp = '[';

                                                        s=symfind(ns, &rows);
                                                        if(s)
                                                                {
								nexp =&s->n[rows];
								if(nexp)
									{
									n[nodepnt++]=nexp;
									if(nodepnt==BITATTRIBUTES_MAX) { free_2(wild); goto ifnode; }
									}
                                                                }

							free_2(ns);
                                                        }
						}

					break;
					}
				}
			}
			else
			{
			if((s=symfind(wild, &rows)))	
				{
				n[nodepnt++]=&s->n[rows];
				}
			}
		}

fw:	free_2(wild);
	}

if(!ch) break;
str=pnt;
}

ifnode:
if(nodepnt)
	{
	b=(struct Bits *)calloc_2(1,sizeof(struct Bits)+(nodepnt-1)*
				  sizeof(struct Node *));

	b->attribs = calloc_2(nodepnt, sizeof(struct BitAttributes));

	for(i=0;i<nodepnt;i++)
		{
		b->nodes[i]=n[i];
		if(n[i]->mv.mvlfac) import_trace(n[i]);

		b->attribs[i].shift = ba[i].shift;
		b->attribs[i].flags = ba[i].flags;
		}

	b->nnbits=nodepnt;
	strcpy(b->name=(char *)malloc_2(strlen(vec)+1),vec);
	}

return(b);
}


/*
 * Create a vector from selected_status signals...
 */
struct Bits *makevec_selected(char *vec, int numrows, char direction)
{
int nodepnt=0;
int i;
struct Node *n[BITATTRIBUTES_MAX];
struct Bits *b=NULL;

if(!direction)
for(i=GLOBALS->numfacs-1;i>=0;i--)	/* to keep vectors in hi..lo order */
	{
	if(get_s_selected(GLOBALS->facs[i]))
		{
		n[nodepnt++]=GLOBALS->facs[i]->n;
		if((nodepnt==BITATTRIBUTES_MAX)||(numrows==nodepnt)) break;
		}
	}
else
for(i=0;i<GLOBALS->numfacs;i++)		/* to keep vectors in lo..hi order */
	{
	if(get_s_selected(GLOBALS->facs[i]))
		{
		n[nodepnt++]=GLOBALS->facs[i]->n;
		if((nodepnt==BITATTRIBUTES_MAX)||(numrows==nodepnt)) break;
		}
	}

if(nodepnt)
	{
	b=(struct Bits *)calloc_2(1,sizeof(struct Bits)+(nodepnt-1)*
				  sizeof(struct Node *));

	for(i=0;i<nodepnt;i++)
		{
		b->nodes[i]=n[i];
		if(n[i]->mv.mvlfac) import_trace(n[i]);
		}

	b->nnbits=nodepnt;
	strcpy(b->name=(char *)malloc_2(strlen(vec)+1),vec);
	}

return(b);
}

/*
 * add vector made in previous function
 */
int add_vector_selected(char *alias, int numrows, char direction)
{
bvptr v=NULL;
bptr b=NULL;

if((b=makevec_selected(alias, numrows, direction)))
	{
        if((v=bits2vector(b)))
                {
                v->bits=b;      /* only needed for savefile function */
                AddVector(v, NULL);
                free_2(b->name);
                b->name=NULL;
                return(v!=NULL);
                }
                else
                {
                free_2(b->name);
		if(b->attribs) free_2(b->attribs);
                free_2(b);
                }
        }
return(v!=NULL);
}

/***********************************************************************************/

/*
 * Create a vector from a range of signals...currently the single
 * bit facility_name[x] case never gets hit, but may be used in the
 * future...
 */
struct Bits *makevec_chain(char *vec, struct symbol *sym, int len)
{
int nodepnt=0, nodepnt_rev;
int i;
struct Node **n;
struct Bits *b=NULL;
struct symbol *symhi = NULL, *symlo = NULL;
char hier_delimeter2;

if(!GLOBALS->vcd_explicit_zero_subscripts)	/* 0==yes, -1==no */
	{
	hier_delimeter2=GLOBALS->hier_delimeter;
	}
	else
	{
	hier_delimeter2='[';
	}

n=(struct Node **)wave_alloca(len*sizeof(struct Node *));
memset(n, 0, len*sizeof(struct Node *)); /* scan-build */

if(!GLOBALS->autocoalesce_reversal)		/* normal case for MTI */
	{
	symhi=sym;
	while(sym)
		{
		symlo=sym;
		n[nodepnt++]=sym->n;
		sym=sym->vec_chain;
		}
	}
	else				/* for verilog XL */
	{
	nodepnt_rev=len;
	symlo=sym;
	while(sym)
		{
		nodepnt++;
		symhi=sym;
		n[--nodepnt_rev]=sym->n;
		sym=sym->vec_chain;
		}
	}

if(nodepnt)
	{
	b=(struct Bits *)calloc_2(1,sizeof(struct Bits)+(nodepnt-1)*
				  sizeof(struct Node *));

	for(i=0;i<nodepnt;i++)
		{
		b->nodes[i]=n[i];
		if(n[i]->mv.mvlfac) import_trace(n[i]);
		}

	b->nnbits=nodepnt;

	if(vec)
		{
		strcpy(b->name=(char *)malloc_2(strlen(vec)+1),vec);
		}
		else
		{
		char *s1, *s2;
		int s1_was_packed = HIER_DEPACK_ALLOC, s2_was_packed = HIER_DEPACK_ALLOC;
		int root1len=0, root2len=0;
		int l1, l2;

		s1=symhi->n->nname;
		s2=symlo->n->nname;

		if(GLOBALS->do_hier_compress)
			{
			s1 = hier_decompress_flagged(s1, &s1_was_packed);
			s2 = hier_decompress_flagged(s2, &s2_was_packed);
			}
		
		l1=strlen(s1); 

		for(i=l1-1;i>=0;i--)
			{
			if(s1[i]==hier_delimeter2) { root1len=i+1; break; }
			}

		l2=strlen(s2);	
		for(i=l2-1;i>=0;i--)
			{
			if(s2[i]==hier_delimeter2) { root2len=i+1; break; }
			}

		if((root1len!=root2len)||(!root1len)||(!root2len)||
			(strncasecmp(s1,s2,root1len)))
			{
			if(symlo!=symhi)
				{
				if(!b->attribs)
					{
					char *aname = attempt_vecmatch(s1, s2);
					if(aname) b->name = aname; else { strcpy(b->name=(char *)malloc_2(8+1),"<Vector>"); }
					}
					else
					{
					char *aname = attempt_vecmatch(s1, s2);
					if(aname) b->name = aname; else { strcpy(b->name=(char *)malloc_2(15+1),"<ComplexVector>"); }
					}
				}
				else
				{
				strcpy(b->name=(char *)malloc_2(l1+1),s1);
				}
			}
			else
			{
			int add1, add2, totallen;

			add1=l1-root1len; add2=l2-root2len;
			if(GLOBALS->vcd_explicit_zero_subscripts==-1)
				{
				add1--;
				add2--;
				}
			
			if(symlo!=symhi)
				{
				unsigned char fixup1 = 0, fixup2 = 0;

				totallen=
					root1len
					-1		/* zap HIER_DELIMETER */
					+1		/* add [              */
					+add1		/* left value	      */
					+1		/* add :	      */
					+add2		/* right value	      */
					+1		/* add ]	      */
					+1		/* add 0x00	      */
					;

				if(GLOBALS->vcd_explicit_zero_subscripts==-1)
					{
					fixup1=*(s1+l1-1); *(s1+l1-1)=0;
					fixup2=*(s2+l2-1); *(s2+l2-1)=0;
					}

				b->name=(char *)malloc_2(totallen);
				strncpy(b->name,s1,root1len-1);
				sprintf(b->name+root1len-1,"[%s:%s]",s1+root1len, s2+root2len);

				if(GLOBALS->vcd_explicit_zero_subscripts==-1)
					{
					*(s1+l1-1)=fixup1;
					*(s2+l2-1)=fixup2;
					}
				}
				else
				{
				unsigned char fixup1 = 0;

				totallen=
					root1len
					-1		/* zap HIER_DELIMETER */
					+1		/* add [              */
					+add1		/* left value	      */
					+1		/* add ]	      */
					+1		/* add 0x00	      */
					;

				if(GLOBALS->vcd_explicit_zero_subscripts==-1)
					{
					fixup1=*(s1+l1-1); *(s1+l1-1)=0;
					}

				b->name=(char *)malloc_2(totallen);
				strncpy(b->name,s1,root1len-1);
				sprintf(b->name+root1len-1,"[%s]",s1+root1len);

				if(GLOBALS->vcd_explicit_zero_subscripts==-1)
					{
					*(s1+l1-1)=fixup1;
					}
				}
			}

		if(GLOBALS->do_hier_compress)
			{
			if(s2_was_packed) free_2(s2);
			if(s1_was_packed) free_2(s1);
			}
		}
	}

return(b);
}

/*
 * add vector made in previous function
 */
int add_vector_chain(struct symbol *s, int len)
{
bvptr v=NULL;
bptr b=NULL;

if(len>1)
	{
	if((b=makevec_chain(NULL, s, len)))
		{
	        if((v=bits2vector(b)))
	                {
	                v->bits=b;      /* only needed for savefile function */
	                AddVector(v, NULL);
	                free_2(b->name);
	                b->name=NULL;
	                return(v!=NULL);
	                }
	                else
	                {
	                free_2(b->name);
			if(b->attribs) free_2(b->attribs);
	                free_2(b);
	                }
	        }
	return(v!=NULL);
	}
	else
	{
	return(AddNode(s->n,NULL));
	}
}

/***********************************************************************************/

/*
 * Create a vector from a range of signals...currently the single
 * bit facility_name[x] case never gets hit, but may be used in the
 * future...
 */
struct Bits *makevec_range(char *vec, int lo, int hi, char direction)
{
int nodepnt=0;
int i;
struct Node *n[BITATTRIBUTES_MAX];
struct Bits *b=NULL;

if(!direction)
for(i=hi;i>=lo;i--)	/* to keep vectors in hi..lo order */
	{
	n[nodepnt++]=GLOBALS->facs[i]->n;
	if(nodepnt==BITATTRIBUTES_MAX) break;
	}
else
for(i=lo;i<=hi;i++)	/* to keep vectors in lo..hi order */
	{
	n[nodepnt++]=GLOBALS->facs[i]->n;
	if(nodepnt==BITATTRIBUTES_MAX) break;
	}

if(nodepnt)
	{
	b=(struct Bits *)calloc_2(1,sizeof(struct Bits)+(nodepnt-1)*
				  sizeof(struct Node *));

	for(i=0;i<nodepnt;i++)
		{
		b->nodes[i]=n[i];
		if(n[i]->mv.mvlfac) import_trace(n[i]);
		}

	b->nnbits=nodepnt;

	if(vec)
		{
		strcpy(b->name=(char *)malloc_2(strlen(vec)+1),vec);
		}
		else
		{
		char *s1, *s2;
		int s1_was_packed = HIER_DEPACK_ALLOC, s2_was_packed = HIER_DEPACK_ALLOC;
		int root1len=0, root2len=0;
		int l1, l2;

		if(!direction)
			{
			s1=GLOBALS->facs[hi]->n->nname;
			s2=GLOBALS->facs[lo]->n->nname;
			}
			else
			{
			s1=GLOBALS->facs[lo]->n->nname;
			s2=GLOBALS->facs[hi]->n->nname;
			}

		if(GLOBALS->do_hier_compress)
			{
			s1 = hier_decompress_flagged(s1, &s1_was_packed);
			s2 = hier_decompress_flagged(s2, &s2_was_packed);
			}
		
		l1=strlen(s1); 

		for(i=l1-1;i>=0;i--)
			{
			if(s1[i]==GLOBALS->hier_delimeter) { root1len=i+1; break; }
			}

		l2=strlen(s2);	
		for(i=l2-1;i>=0;i--)
			{
			if(s2[i]==GLOBALS->hier_delimeter) { root2len=i+1; break; }
			}

		if((root1len!=root2len)||(!root1len)||(!root2len)||
			(strncasecmp(s1,s2,root1len)))
			{
			if(lo!=hi)
				{
				if(!b->attribs)
					{
					char *aname = attempt_vecmatch(s1, s2);
					if(aname) b->name = aname; else { strcpy(b->name=(char *)malloc_2(8+1),"<Vector>"); }
					}
					else
					{
					char *aname = attempt_vecmatch(s1, s2);
					if(aname) b->name = aname; else { strcpy(b->name=(char *)malloc_2(15+1),"<ComplexVector>"); }
					}
				}
				else
				{
				strcpy(b->name=(char *)malloc_2(l1+1),s1);
				}
			}
			else
			{
			int add1, add2, totallen;

			add1=l1-root1len; add2=l2-root2len;
			
			if(lo!=hi)
				{
				totallen=
					root1len
					-1		/* zap HIER_DELIMETER */
					+1		/* add [              */
					+add1		/* left value	      */
					+1		/* add :	      */
					+add2		/* right value	      */
					+1		/* add ]	      */
					+1		/* add 0x00	      */
					;

				b->name=(char *)malloc_2(totallen);
				strncpy(b->name,s1,root1len-1);
				sprintf(b->name+root1len-1,"[%s:%s]",s1+root1len, s2+root2len);
				}
				else
				{
				totallen=
					root1len
					-1		/* zap HIER_DELIMETER */
					+1		/* add [              */
					+add1		/* left value	      */
					+1		/* add ]	      */
					+1		/* add 0x00	      */
					;

				b->name=(char *)malloc_2(totallen);
				strncpy(b->name,s1,root1len-1);
				sprintf(b->name+root1len-1,"[%s]",s1+root1len);
				}
			}
		if(GLOBALS->do_hier_compress)
			{
			if(s2_was_packed) free_2(s2);
			if(s1_was_packed) free_2(s1);
			}
		}
	}

return(b);
}

/*
 * add vector made in previous function
 */
int add_vector_range(char *alias, int lo, int hi, char direction)
{
bvptr v=NULL;
bptr b=NULL;

if(lo!=hi)
	{
	if((b=makevec_range(alias, lo, hi, direction)))
		{
	        if((v=bits2vector(b)))
	                {
	                v->bits=b;      /* only needed for savefile function */
	                AddVector(v, NULL);
	                free_2(b->name);
	                b->name=NULL;
	                return(v!=NULL);
	                }
	                else
	                {
	                free_2(b->name);
			if(b->attribs) free_2(b->attribs);
	                free_2(b);
	                }
	        }
	return(v!=NULL);
	}
	else
	{
	return(AddNode(GLOBALS->facs[lo]->n,NULL));
	}
}


/*
 * splits facility name into signal and bitnumber
 */
void facsplit(char *str, int *len, int *number)
{
char *ptr;
char *numptr=NULL;
char ch;

ptr=str;

while((ch=*ptr))
        {
        if((ch>='0')&&(ch<='9')) 
                {
                if(!numptr) numptr=ptr;
                }
                else numptr=NULL;
        ptr++;
        }

if(numptr)
        {
        *number=atoi(numptr);
        *len=numptr-str;
        }
        else
        {
        *number=0;
        *len=ptr-str;
        }
}


/*
 * compares two facilities a la strcmp but preserves
 * numbers for comparisons
 *
 * there are two flavors..the slow and accurate to any 
 * arbitrary number of digits version (first) and the
 * fast one good to 2**31-1.  we default to the faster
 * version since there's probably no real need to
 * process ints larger than two billion anyway...
 */

#ifdef WAVE_USE_SIGCMP_INFINITE_PRECISION
inline int sigcmp_2(char *s1, char *s2)
{
char *n1, *n2;
unsigned char c1, c2;
int len1, len2;

for(;;)
	{
	c1=(unsigned char)*s1;
	c2=(unsigned char)*s2;

	if((c1==0)&&(c2==0)) return(0);
	if((c1>='0')&&(c1<='9')&&(c2>='0')&&(c2<='9'))
		{
		n1=s1; n2=s2;
		len1=len2=0;

		do	{
			len1++;
			c1=(unsigned char)*(n1++);
			} while((c1>='0')&&(c1<='9'));
		if(!c1) n1--;

		do	{
			len2++;
			c2=(unsigned char)*(n2++);
			} while((c2>='0')&&(c2<='9'));
		if(!c2) n2--;

		do	{
			if(len1==len2)
				{
				c1=(unsigned char)*(s1++);
				len1--;
				c2=(unsigned char)*(s2++);
				len2--;
				}
			else
			if(len1<len2)
				{
				c1='0';
				c2=(unsigned char)*(s2++);
				len2--;
				}
			else
				{
				c1=(unsigned char)*(s1++);
				len1--;
				c2='0';
				}

			if(c1!=c2) return((int)c1-(int)c2);
			} while(len1);

		s1=n1; s2=n2;
		continue;
		}
		else
		{
		if(c1!=c2) return((int)c1-(int)c2);
		}

	s1++; s2++;
	}
}
#else
inline int sigcmp_2(char *s1, char *s2)
{
unsigned char c1, c2;
int u1, u2;

for(;;)
	{
	c1=(unsigned char)*(s1++);
	c2=(unsigned char)*(s2++);

	if(!(c1|c2)) return(0);	/* removes extra branch through logical or */
	if((c1<='9')&&(c2<='9')&&(c2>='0')&&(c1>='0'))
		{
		u1=(int)(c1&15);
		u2=(int)(c2&15);

		while(((c2=(unsigned char)*s2)>='0')&&(c2<='9'))
			{
			u2*=10;
			u2+=(unsigned int)(c2&15);
			s2++;
			}

		while(((c2=(unsigned char)*s1)>='0')&&(c2<='9'))
			{
			u1*=10;
			u1+=(unsigned int)(c2&15);
			s1++;
			}

		if(u1==u2) continue;
			else return((int)u1-(int)u2);
		}
		else
		{
		if(c1!=c2) return((int)c1-(int)c2);
		}
	}
}
#endif

int sigcmp(char *s1, char *s2)
{
int rc = sigcmp_2(s1, s2);
if(!rc)
	{
	rc = strcmp(s1, s2); /* to handle leading zero "0" vs "00" cases ... we provide a definite order so bsearch doesn't fail */
	}

return(rc);
}


#ifndef __linux__
/* 
 * heapsort algorithm.  this typically outperforms quicksort.  note
 * that glibc will use a modified mergesort if memory is available, so
 * under linux use the stock qsort instead.
 */
static struct symbol **hp;
static void heapify(int i, int heap_size)
{
int l, r;
unsigned int largest;
struct symbol *t;
int maxele=heap_size/2-1;	/* points to where heapswaps don't matter anymore */
                
for(;;)
        {
        l=2*i+1;
        r=l+1;
                         
        if((l<heap_size)&&(sigcmp(hp[l]->name,hp[i]->name)>0))
                {
                largest=l;
                }   
                else
                {
                largest=i;
                }
        if((r<heap_size)&&(sigcmp(hp[r]->name,hp[largest]->name)>0))
                {
                largest=r;
                }
        
        if(i!=largest)
                {
                t=hp[i];
                hp[i]=hp[largest];
                hp[largest]=t;
                
                if(largest<=maxele)
                        {
                        i=largest;
                        }
                        else
                        {
                        break;
                        } 
                }   
                else
                {
                break;
                }
        }
}

void wave_heapsort(struct symbol **a, int num)
{
int i;
int indx=num-1;
struct symbol *t;

hp=a;

for(i=(num/2-1);i>0;i--)	/* build facs into having heap property */
        {
        heapify(i,num);
        }

for(;;)
	{
        if(indx) heapify(0,indx+1);
	DEBUG(printf("%s\n", a[0]->name));

	if(indx!=0)
		{
		t=a[0];			/* sort in place by doing a REVERSE sort and */
		a[0]=a[indx];		/* swapping the front and back of the tree.. */
		a[indx--]=t;
		}
		else
		{
		break;
		}
	}
}

#else

static int qssigcomp(const void *v1, const void *v2)
{
struct symbol *a1 = *((struct symbol **)v1);
struct symbol *a2 = *((struct symbol **)v2);
return(sigcmp(a1->name, a2->name));
}

void wave_heapsort(struct symbol **a, int num)
{
qsort(a, num, sizeof(struct symbol *), qssigcomp);
}

#endif

/*
 * Malloc/Create a name from a range of signals starting from vec_root...currently the single
 * bit facility_name[x] case never gets hit, but may be used in the
 * future...
 */
char *makename_chain(struct symbol *sym)
{
int i;
struct symbol *symhi = NULL, *symlo = NULL;
char hier_delimeter2;
char *name=NULL;
char *s1, *s2;
int s1_was_packed = HIER_DEPACK_ALLOC, s2_was_packed = HIER_DEPACK_ALLOC;
int root1len=0, root2len=0;
int l1, l2;

if(!GLOBALS->vcd_explicit_zero_subscripts)	/* 0==yes, -1==no */
	{
	hier_delimeter2=GLOBALS->hier_delimeter;
	}
	else
	{
	hier_delimeter2='[';
	}

if(!GLOBALS->autocoalesce_reversal)		/* normal case for MTI */
	{
	symhi=sym;
	while(sym)
		{
		symlo=sym;
		sym=sym->vec_chain;
		}
	}
	else				/* for verilog XL */
	{
	symlo=sym;
	while(sym)
		{
		symhi=sym;
		sym=sym->vec_chain;
		}
	}


s1=hier_decompress_flagged(symhi->n->nname, &s1_was_packed);
s2=hier_decompress_flagged(symlo->n->nname, &s2_was_packed);
		
l1=strlen(s1); 

for(i=l1-1;i>=0;i--)
	{
	if(s1[i]==hier_delimeter2) { root1len=i+1; break; }
	}

l2=strlen(s2);	
for(i=l2-1;i>=0;i--)
	{
	if(s2[i]==hier_delimeter2) { root2len=i+1; break; }
	}

if((root1len!=root2len)||(!root1len)||(!root2len)||
	(strncasecmp(s1,s2,root1len)))
	{
	if(symlo!=symhi)
		{
		char *aname = attempt_vecmatch(s1, s2);
		if(aname) name = aname; else { strcpy(name=(char *)malloc_2(8+1),"<Vector>"); }
		}
		else
		{
		strcpy(name=(char *)malloc_2(l1+1),s1);
		}
	}
	else
	{
	int add1, add2, totallen;

	add1=l1-root1len; add2=l2-root2len;
	if(GLOBALS->vcd_explicit_zero_subscripts==-1)
		{
		add1--;
		add2--;
		}
			
	if(symlo!=symhi)
		{
		unsigned char fixup1 = 0, fixup2 = 0;

		totallen=
			root1len
			-1		/* zap HIER_DELIMETER */
			+1		/* add [              */
			+add1		/* left value	      */
			+1		/* add :	      */
			+add2		/* right value	      */
			+1		/* add ]	      */
			+1		/* add 0x00	      */
			;

		if(GLOBALS->vcd_explicit_zero_subscripts==-1)
			{
			fixup1=*(s1+l1-1); *(s1+l1-1)=0;
			fixup2=*(s2+l2-1); *(s2+l2-1)=0;
			}

		name=(char *)malloc_2(totallen);
		strncpy(name,s1,root1len-1);
		sprintf(name+root1len-1,"[%s:%s]",s1+root1len, s2+root2len);

		if(GLOBALS->vcd_explicit_zero_subscripts==-1)
			{
			*(s1+l1-1)=fixup1;
			*(s2+l2-1)=fixup2;
			}
		}
		else
		{
		unsigned char fixup1 = 0;

		totallen=
			root1len
			-1		/* zap HIER_DELIMETER */
			+1		/* add [              */
			+add1		/* left value	      */
			+1		/* add ]	      */
			+1		/* add 0x00	      */
			;

		if(GLOBALS->vcd_explicit_zero_subscripts==-1)
			{
			fixup1=*(s1+l1-1); *(s1+l1-1)=0;
			}

		name=(char *)malloc_2(totallen);
		strncpy(name,s1,root1len-1);
		sprintf(name+root1len-1,"[%s]",s1+root1len);

		if(GLOBALS->vcd_explicit_zero_subscripts==-1)
			{
			*(s1+l1-1)=fixup1;
			}
		}
	}

if(s1_was_packed) { free_2(s1); }
if(s2_was_packed) { free_2(s2); }

return(name);
}

/******************************************************************/

eptr ExpandNode(nptr n)
{
int width;
int msb, lsb, delta;
int actual;
hptr h, htemp;
int i, j;
nptr *narray;
char *nam;
int offset, len;
eptr rc=NULL;
exptr exp1;

if(n->mv.mvlfac) import_trace(n);

if(!n->extvals)
	{
	DEBUG(fprintf(stderr, "Nothing to expand\n"));
	}
	else
	{
	char *namex;
	int was_packed = HIER_DEPACK_ALLOC;

	msb = n->msi;
	lsb = n->lsi;
	if(msb>lsb)
		{
		width = msb - lsb + 1;
		delta = -1;
		}
		else
		{
		width = lsb - msb + 1;
		delta = 1;
		}
	actual = msb;

	narray=(nptr *)malloc_2(width*sizeof(nptr));
	rc = malloc_2(sizeof(ExpandInfo));
	rc->narray = narray;
	rc->msb = msb;
	rc->lsb = lsb;
	rc->width = width;

	if(GLOBALS->do_hier_compress)
		{
		namex = hier_decompress_flagged(n->nname, &was_packed);
		}
		else
		{
		namex = n->nname;
		}

	offset = strlen(namex);
	for(i=offset-1;i>=0;i--)
		{
		if(namex[i]=='[') break;
		}
	if(i>-1) offset=i;

	nam=(char *)wave_alloca(offset+20+30);
	memcpy(nam, namex, offset);

	if(was_packed)
		{
		free_2(namex);
		}

	if(!n->harray)         /* make quick array lookup for aet display--normally this is done in addnode */
	        {
		hptr histpnt;
		int histcount;
		hptr *harray;

	        histpnt=&(n->head);
	        histcount=0;
	
	        while(histpnt)
	                {
	                histcount++;
	                histpnt=histpnt->next;
	                }
	
	        n->numhist=histcount;
	 
	        if(!(n->harray=harray=(hptr *)malloc_2(histcount*sizeof(hptr))))
	                {
	                fprintf( stderr, "Out of memory, can't add to analyzer\n");
	                return(NULL);
	                }
	
	        histpnt=&(n->head);
	        for(i=0;i<histcount;i++)
	                {
	                *harray=histpnt;
	                harray++;
	                histpnt=histpnt->next;
	                }
	        }

	h=&(n->head);
	while(h)
		{
		if(h->flags & (HIST_REAL|HIST_STRING)) return(NULL);
		h=h->next;
		}

	DEBUG(fprintf(stderr, "Expanding: (%d to %d) for %d bits over %d entries.\n", msb, lsb, width, n->numhist));

	for(i=0;i<width;i++)
		{
		narray[i] = (nptr)calloc_2(1, sizeof(struct Node));
		sprintf(nam+offset, "[%d]", actual);	
#ifdef WAVE_ARRAY_SUPPORT
		if(n->array_height)
			{
			len = offset + strlen(nam+offset);
			sprintf(nam+len, "{%d}", n->this_row);
			}
#endif
		len = offset + strlen(nam+offset);
		narray[i]->nname = (char *)malloc_2(len+1);
		strcpy(narray[i]->nname, nam);

		exp1 = (exptr) calloc_2(1, sizeof(struct ExpandReferences));
		exp1->parent=n;							/* point to parent */
		exp1->parentbit=i;
		exp1->actual = actual;
		actual += delta;
		narray[i]->expansion = exp1;					/* can be safely deleted if expansion set like here */
		}

	for(i=0;i<n->numhist;i++)
		{
		h=n->harray[i];
		if((h->time<GLOBALS->min_time)||(h->time>GLOBALS->max_time))
			{
			for(j=0;j<width;j++)
				{
				if(narray[j]->curr)
					{
					htemp = (hptr) calloc_2(1, sizeof(struct HistEnt));				
					htemp->v.h_val = AN_X;			/* 'x' */
					htemp->time = h->time;
					narray[j]->curr->next = htemp;
					narray[j]->curr = htemp;
					}
					else
					{
					narray[j]->head.v.h_val = AN_X;		/* 'x' */
					narray[j]->head.time  = h->time;
					narray[j]->curr = &(narray[j]->head);
					}

				narray[j]->numhist++;
				}
			}
			else
			{
			for(j=0;j<width;j++)
				{
				unsigned char val = h->v.h_vector[j];
				switch(val)
					{
					case '0':		val = AN_0; break;
					case '1':		val = AN_1; break;
					case 'x': case 'X':	val = AN_X; break;
					case 'z': case 'Z':	val = AN_Z; break;
					case 'h': case 'H':	val = AN_H; break;
					case 'l': case 'L':	val = AN_L; break;
					case 'u': case 'U':	val = AN_U; break;
					case 'w': case 'W':	val = AN_W; break;
					case '-':		val = AN_DASH; break;
					default:	break;			/* leave val alone as it's been converted already.. */
					}

				if(narray[j]->curr->v.h_val != val) 		/* curr will have been established already by 'x' at time: -1 */
					{
					htemp = (hptr) calloc_2(1, sizeof(struct HistEnt));				
					htemp->v.h_val = val;
					htemp->time = h->time;
					narray[j]->curr->next = htemp;
					narray[j]->curr = htemp;
					narray[j]->numhist++;
					}
				}
			}
		}

	for(i=0;i<width;i++)
		{
		narray[i]->harray = (hptr *)calloc_2(narray[i]->numhist, sizeof(hptr));
		htemp = &(narray[i]->head);
		for(j=0;j<narray[i]->numhist;j++)
			{
			narray[i]->harray[j] = htemp;
			htemp = htemp->next;
			}
		}

	}

return(rc);
}

/******************************************************************/

nptr ExtractNodeSingleBit(nptr n, int bit)
{
int lft, rgh;
hptr h, htemp;
int i, j;
int actual;
nptr np;
char *nam;
int offset, len;
exptr exp1;

if(n->mv.mvlfac) import_trace(n);

if(!n->extvals)
	{
	DEBUG(fprintf(stderr, "Nothing to expand\n"));
	return(NULL);
	}
	else
	{
	char *namex;
	int was_packed = HIER_DEPACK_ALLOC;

	if(n->lsi > n->msi)
		{
		rgh = n->lsi; lft = n->msi;
		actual = n->msi + bit;
		}
		else
		{
		rgh = n->msi; lft = n->lsi;
		actual = n->msi - bit;
		}

	if((actual>rgh)||(actual<lft))
		{
		DEBUG(fprintf(stderr, "Out of range\n"));
		return(NULL);
		}

	if(GLOBALS->do_hier_compress)
		{
		namex = hier_decompress_flagged(n->nname, &was_packed);
		}
		else
		{
		namex = n->nname;
		}
	offset = strlen(namex);
	for(i=offset-1;i>=0;i--)
		{
		if(namex[i]=='[') break;
		}
	if(i>-1) offset=i;

	nam=(char *)wave_alloca(offset+20);
	memcpy(nam, namex, offset);

	if(was_packed)
		{
		free_2(namex);
		}

	if(!n->harray)         /* make quick array lookup for aet display--normally this is done in addnode */
	        {
		hptr histpnt;
		int histcount;
		hptr *harray;

	        histpnt=&(n->head);
	        histcount=0;
	
	        while(histpnt)
	                {
	                histcount++;
	                histpnt=histpnt->next;
	                }
	
	        n->numhist=histcount;
	 
	        if(!(n->harray=harray=(hptr *)malloc_2(histcount*sizeof(hptr))))
	                {
	                DEBUG(fprintf( stderr, "Out of memory, can't add to analyzer\n"));
	                return(NULL);
	                }
	
	        histpnt=&(n->head);
	        for(i=0;i<histcount;i++)
	                {
	                *harray=histpnt;
	                harray++;
	                histpnt=histpnt->next;
	                }
	        }

	h=&(n->head);
	while(h)
		{
		if(h->flags & (HIST_REAL|HIST_STRING)) return(NULL);
		h=h->next;
		}

	DEBUG(fprintf(stderr, "Extracting: (%d to %d) for offset #%d over %d entries.\n", n->msi, n->lsi, bit, n->numhist));

	np = (nptr)calloc_2(1, sizeof(struct Node));
	sprintf(nam+offset, "[%d]", actual);
#ifdef WAVE_ARRAY_SUPPORT
        if(n->array_height)
        	{
                len = offset + strlen(nam+offset);
                sprintf(nam+len, "{%d}", n->this_row);
                }
#endif
	len = offset + strlen(nam+offset);


	np->nname = (char *)malloc_2(len+1);
	strcpy(np->nname, nam);

	exp1 = (exptr) calloc_2(1, sizeof(struct ExpandReferences));
	exp1->parent=n;							/* point to parent */
	exp1->parentbit=bit;
	exp1->actual=actual;						/* actual bitnum in [] */
	np->expansion = exp1;						/* can be safely deleted if expansion set like here */

	for(i=0;i<n->numhist;i++)
		{
		h=n->harray[i];
		if((h->time<GLOBALS->min_time)||(h->time>GLOBALS->max_time))
			{
			if(np->curr)
				{
				htemp = (hptr) calloc_2(1, sizeof(struct HistEnt));				
				htemp->v.h_val = AN_X;			/* 'x' */
				htemp->time = h->time;
				np->curr->next = htemp;
				np->curr = htemp;
				}
				else
				{
				np->head.v.h_val = AN_X;		/* 'x' */
				np->head.time  = h->time;
				np->curr = &(np->head);
				}

			np->numhist++;
			}
			else
			{
			unsigned char val = h->v.h_vector[bit];
			switch(val)
				{
				case '0':		val = AN_0; break;
				case '1':		val = AN_1; break;
				case 'x': case 'X':	val = AN_X; break;
				case 'z': case 'Z':	val = AN_Z; break;
				case 'h': case 'H':	val = AN_H; break;
				case 'l': case 'L':	val = AN_L; break;
				case 'u': case 'U':	val = AN_U; break;
				case 'w': case 'W':	val = AN_W; break;
				case '-':		val = AN_DASH; break;
				default:	break;			/* leave val alone as it's been converted already.. */
				}

			if(np->curr->v.h_val != val) 		/* curr will have been established already by 'x' at time: -1 */
				{
				htemp = (hptr) calloc_2(1, sizeof(struct HistEnt));				
				htemp->v.h_val = val;
				htemp->time = h->time;
				np->curr->next = htemp;
				np->curr = htemp;
				np->numhist++;
				}
			}
		}

	np->harray = (hptr *)calloc_2(np->numhist, sizeof(hptr));
	htemp = &(np->head);
	for(j=0;j<np->numhist;j++)
		{
		np->harray[j] = htemp;
		htemp = htemp->next;
		}

	return(np);
	}
}

/******************************************************************/

/*
 * this only frees nodes created via expansion in ExpandNode() functions above!
 */
void DeleteNode(nptr n)
{
int i;

if(n->expansion)
	{
	if(n->expansion->refcnt==0)
		{
		for(i=1;i<n->numhist;i++)	/* 1st is actually part of the Node! */
			{
			free_2(n->harray[i]);	
			}
		free_2(n->harray);
		free_2(n->expansion);
		free_2(n->nname);
		free_2(n);
		}
		else
		{
		n->expansion->refcnt--;
		}
	}
}

/******************************************************************/

/*
 * attempt to synthesize bitwise on loader fail...caller must free return pnt
 */
static char *synth_blastvec(char *w)
{
char *mem = NULL;
char *t;
char *lbrack, *colon, *rbrack, *rname, *msbs, *lsbs;
int wlen, bitlen, msb, lsb;
int msbslen, lsbslen, maxnumlen;
int i, siz;

if(w)
	{
	if((lbrack = strrchr(w, '[')))
	if((colon = strchr(lbrack+1, ':')))
	if((rbrack = strchr(colon+1, ']')))
		{
		*lbrack = *colon = *rbrack = 0;
		msbs = lbrack + 1;
		lsbs = colon + 1;
		rname = hier_extract(w, GLOBALS->hier_max_level);

		msb = atoi(msbs);
		lsb = atoi(lsbs);
		bitlen = (msb > lsb) ? (msb - lsb + 1) : (lsb - msb + 1);
		if(bitlen > 1)
			{
			wlen = strlen(w);

			msbslen = strlen(msbs);
			lsbslen = strlen(lsbs);
			maxnumlen = (msbslen > lsbslen) ? msbslen : lsbslen;

			siz = 	1 + 				/* # */
				strlen(rname) +			/* vector alias name */
				1+				/*   */
				1+				/* [ */
				msbslen+			/* msb */
				1+				/* : */
				lsbslen+			/* lsb */
				1+				/* ] */
				1;				/*   */

			siz +=  bitlen * (
				wlen +				/* full bitname */
				1+				/* [ */
				maxnumlen+			/* individual bit */
				1+				/* ] */
				1				/*   */
				);

			mem = calloc_2(1, siz);
			t = mem + sprintf(mem, "#%s[%d:%d] ", rname, msb, lsb);
			
			if(msb > lsb)
				{
				for(i = msb; i >= lsb; i--)
					{
					t += sprintf(t, "%s[%d]", w, i);
					if(i!=lsb) t += sprintf(t, " ");
					}
				}
				else
				{
				for(i = msb; i <= lsb; i++)
					{
					t += sprintf(t, "%s[%d]", w, i);
					if(i!=lsb) t += sprintf(t, " ");
					}
				}

			/* fprintf(stderr, "%d,%d: %s\n", siz, strlen(mem), mem); */
			}

		}
	}

return(mem);
}

/******************************************************************/

/*
 * Parse a line of the wave file and act accordingly.. 
 * Returns nonzero if trace(s) added.
 */
int parsewavline(char *w, char *alias, int depth)
{
  int i;
  int len;
  char *w2;
  nptr nexp;
  unsigned int rows = 0;
  char *prefix, *suffix, *new;
  char *prefix_init, *w2_init;
  unsigned int mode;

  if(!(len=strlen(w))) return(0);
  if(*(w+len-1)=='\n')
    {
      *(w+len-1)=0x00; /* strip newline if present */
      len--;
      if(!len) return(0);
    }

  while(1)
    {
      if(isspace((int)(unsigned char)*w)) { w++; continue; }
      if(!(*w)) return(0);	/* no args */
      break;			/* start grabbing chars from here */
    }

  w2=w;

  /* sscanf(w2,"%s",prefix); */

 prefix=(char *)wave_alloca(len+1);
 suffix=(char *)wave_alloca(len+1);
 new=(char *)wave_alloca(len+1);

 prefix_init = prefix;
 w2_init = new;
 mode = 0; /* 0 = before "{", 1 = after "{", 2 = after "}" or " " */

 while(*w2)
   {
     if((mode == 0) && (*w2 == '{'))
       {
	 mode = 1;
	 w2++;
       }
     else if((mode == 1) && (*w2 == '}'))
       {
	 /* strcpy(prefix, ""); */
	 *(prefix) = '\0';
	 mode = 2;
	 w2++;
       }
     else if((mode == 0) && (*w2 == ' '))
       {
	 /* strcpy(prefix, ""); */
	 *(prefix) = '\0';
	 strcpy(new, w2);
	 mode = 2;
	 w2++;
	 new++;
       }
     else
       {
	 strcpy(new, w2);
	 if (mode != 2)
	   {
	     strcpy(prefix, w2);
	     prefix++;
	   }
	 w2++;
	 new++;
       }
   }

 prefix = prefix_init;
 w2 = w2_init;

 /* printf("HHHHH |%s| %s\n", prefix, w2); */


if(*w2=='*')
	{
	float f;
	TimeType ttlocal;
	int which=0;

	GLOBALS->zoom_was_explicitly_set=~0;
	w2++;

	for(;;)
		{
		while(*w2==' ') w2++;
		if(*w2==0) return(~0);

		if(!which) { sscanf(w2,"%f",&f); GLOBALS->tims.zoom=(gdouble)f; }
		else
		{
		sscanf(w2,TTFormat,&ttlocal);
		switch(which)
			{
			case 1:  GLOBALS->tims.marker=ttlocal; break;
			default: 
				if((which-2)<26) GLOBALS->named_markers[which-2]=ttlocal; 
				break;
			}
		}
		which++;
		w2++;
		for(;;)
			{
			if(*w2==0) return(~0);
			if(*w2=='\n') return(~0);
			if(*w2!=' ') w2++; else break;
			}
		}
	}
else
if(*w2=='-')
	{
	AddBlankTrace((*(w2+1)!=0)?(w2+1):NULL);
	}
else
if(*w2=='>')
	{
	char *wnptr=(*(w2+1)!=0)?(w2+1):NULL;
	GLOBALS->shift_timebase_default_for_add=wnptr?atoi_64(wnptr):LLDescriptor(0);
	}
else
if(*w2=='@')
	{
	/* handle trace flags */
	sscanf(w2+1, "%x", &GLOBALS->default_flags);
	if( (GLOBALS->default_flags & (TR_FTRANSLATED|TR_PTRANSLATED)) == (TR_FTRANSLATED|TR_PTRANSLATED) )
		{
		GLOBALS->default_flags &= ~TR_PTRANSLATED; /* safest bet though this is a cfg file error */
		}

	return(~0);
	}
else
if(*w2=='+')
	{
	/* handle aliasing */
	  struct symbol *s;
	  sscanf(w2+strlen(prefix),"%s",suffix);

	  if(suffix[0]=='(')
	    {
	      for(i=1;;i++)
		{
		  if(suffix[i]==0) return(0);
		  if((suffix[i]==')')&&(suffix[i+1])) {i++; break; }
		}
	      
	      s=symfind(suffix+i, &rows);
	      if (s) {
		nexp = ExtractNodeSingleBit(&s->n[rows], atoi(suffix+1));
		if(nexp)
		  {
		    AddNode(nexp, prefix+1);
		    return(~0);
		  }
		else
		  {
		    return(0);
		  }
	      }	
	      else 
		{
		char *lp = strrchr(suffix+i, '[');
		if(lp)
			{
			char *ns = malloc_2(strlen(suffix+i) + 32);
			char *colon = strchr(lp+1, ':');
			int msi, lsi, bval, actual;
			*lp = 0;

			bval = atoi(suffix+1);
			if(colon)
				{
				msi = atoi(lp+1);
				lsi = atoi(colon+1);

				if(lsi > msi)
					{
					actual = msi + bval;
					}
					else
					{
					actual = msi - bval;
					}
				}
				else
				{
				actual = bval; /* punt */
				}

			sprintf(ns, "%s[%d]", suffix+i, actual);
			*lp = '[';			

			s=symfind(ns, &rows);
			free_2(ns);
			if(s)
				{
				AddNode(&s->n[rows], prefix+1);
				return(~0);
				}

			}

		  return(0);
		}
	    }
	  else
	    {
	      int rc;
	      
	      char *newl   = strdup_2(w2+strlen(prefix));
	      char *nalias = strdup_2(prefix+1);

	      rc = parsewavline(newl, nalias, depth);
	      if (newl)   free_2(newl);
	      if (nalias) free_2(nalias);

	      return rc;
	    }
	/* 	{ */
/* 		if((s=symfind(suffix, &rows))) */
/* 			{ */
/* 			AddNode(&s->n[rows],prefix+1); */
/* 			return(~0); */
/* 			} */
/* 			else */
/* 			{ */
/* 			return(0); */
/* 			} */
/* 		} */
	}
else
if((*w2=='#')||(*w2==':'))
	{
	/* handle bitvec */
	bvptr v=NULL;
	bptr b=NULL;
	int maketyp = (*w2=='#');

	w2=w2+strlen(prefix);
	while(1)
		{
		if(isspace((int)(unsigned char)*w2)) { w2++; continue; }
		if(!(*w2)) return(0);	/* no more args */	
		break;			/* start grabbing chars from here */
		}

	b = maketyp ? makevec(prefix+1,w2) : makevec_annotated(prefix+1,w2);	/* '#' vs ':' cases... */

	if(b)
		{
		if((v=bits2vector(b)))
			{
			v->bits=b;	/* only needed for savefile function */
			AddVector(v, alias);
			free_2(b->name);
			b->name=NULL;
			return(v!=NULL);
			}
			else
			{
			free_2(b->name);
			if(b->attribs) free_2(b->attribs);
			free_2(b);
			}
		}
		else if(!depth) /* don't try vectorized if we're re-entrant */
		{
		char *sp = strchr(w2, ' ');
		char *lbrack;

		if(sp)
			{
			*sp = 0;

			lbrack = strrchr(w2, '[');

			if(lbrack)
				{
				int made = 0;
				char *w3;
				char *rbrack           = strrchr(w2,   ']');
				char *rightmost_lbrack = strrchr(sp+1, '[');

				if(rbrack && rightmost_lbrack)
					{
					*rbrack = 0;

					w3 = malloc_2(strlen(w2) + 1 + strlen(rightmost_lbrack+1) + 1);
					sprintf(w3, "%s:%s", w2, rightmost_lbrack+1);

					made = maketraces(w3, alias, 1);
					free_2(w3);
					}

				if(0)	/* this is overkill for now with possible delay implications so commented out */
				if(!made)
					{
					*lbrack = 0;
					fprintf(stderr, "GTKWAVE | Attempting regex '%s' on missing stranded vector\n", w2);

					w3 = malloc_2(1 + strlen(w2) + 5);
					sprintf(w3, "^%s\\[.*", w2);
					maketraces(w3, alias, 1);
					free_2(w3);
					}
				}
			}
		}
	
	return(v!=NULL);
	}
else
if(*w2=='!')
	{
	/* fill logical_mutex */
	char ch;

	for(i=0;i<6;i++)
		{
		ch = *(w2+i+1);
		if(ch != 0)
			{
			if(ch=='!')
				{
				GLOBALS->strace_ctx->shadow_active = 0;
				return(~0);
				}

			if((!i)&&(GLOBALS->strace_ctx->shadow_straces))
				{
				delete_strace_context();
				}

			GLOBALS->strace_ctx->shadow_logical_mutex[i] = (ch & 1);
			}
			else	/* in case of short read */
			{
			GLOBALS->strace_ctx->shadow_logical_mutex[i] = 0;
			}
		}

	GLOBALS->strace_ctx->shadow_mark_idx_start = 0;
	GLOBALS->strace_ctx->shadow_mark_idx_end = 0;

	if(i==6)
		{
		ch = *(w2+7);
		if(ch != 0)
			{
			if (isupper((int)(unsigned char)ch) || ch=='@')
				GLOBALS->strace_ctx->shadow_mark_idx_start = ch - '@';
		
			ch = *(w2+8);
			if(ch != 0)
				{
				if (isupper((int)(unsigned char)ch) || ch=='@')
					GLOBALS->strace_ctx->shadow_mark_idx_end = ch - '@';
				}
			}
		}

	GLOBALS->strace_ctx->shadow_active = 1;
	return(~0);
	}
	else
if(*w2=='?')
	{
	/* fill st->type */
	if(*(w2+1)=='\"')
		{
		int lens = strlen(w2+2);
		if(GLOBALS->strace_ctx->shadow_string) free_2(GLOBALS->strace_ctx->shadow_string);
		GLOBALS->strace_ctx->shadow_string=NULL;

		if(lens)
			{
			GLOBALS->strace_ctx->shadow_string = malloc_2(lens+1);		
			strcpy(GLOBALS->strace_ctx->shadow_string, w2+2);
			}

		GLOBALS->strace_ctx->shadow_type = ST_STRING;
		}
		else
		{
		unsigned int hex;
		sscanf(w2+1, "%x", &hex);	
		GLOBALS->strace_ctx->shadow_type = hex;
		}

	return(~0);
	}
else if(*w2=='^')
	{
	if(*(w2+1) == '>')
		{
		GLOBALS->current_translate_proc = 0;	/* will overwrite if loadable/translatable */

		if(*(w2+2) != '0')
			{
			  /*			char *fn = strstr(w2+3, " "); */
			char *fn = w2+3;
			if(fn)
				{
				while(*fn && isspace((int)(unsigned char)*fn)) fn++;
				if(*fn && !isspace((int)(unsigned char)*fn)) 
					{
					set_current_translate_proc(fn);
					}
				}
			}
		}
	else
	if(*(w2+1) == '<')
		{
		GLOBALS->current_translate_ttrans = 0;	/* will overwrite if loadable/translatable */

		if(*(w2+2) != '0')
			{
			  /*			char *fn = strstr(w2+3, " "); */
			char *fn = w2+3;
			if(fn)
				{
				while(*fn && isspace((int)(unsigned char)*fn)) fn++;
				if(*fn && !isspace((int)(unsigned char)*fn)) 
					{
					set_current_translate_ttrans(fn);
					}
				}
			}
		}
		else
		{
		GLOBALS->current_translate_file = 0;	/* will overwrite if loadable/translatable */

		if(*(w2+1) != '0')
			{
			char *fn = strstr(w2+2, " ");
			if(fn)
				{
				while(*fn && isspace((int)(unsigned char)*fn)) fn++;
				if(*fn && !isspace((int)(unsigned char)*fn)) 
					{
					set_current_translate_file(fn);
					}
				}
			}
		}
	}
else if (*w2 == '[')
  {
    /* Search for matching ']'.  */
    w2++;
    for (w = w2; *w; w++)
      if (*w == ']')
	break;
    if (!*w)
      return 0;
    
    *w++ = 0;
    if (strcmp (w2, "size") == 0)
      {
      if(!GLOBALS->ignore_savefile_size)
	{
	/* Main window size.  */
	int x, y;
	sscanf (w, "%d %d", &x, &y);
	if(!GLOBALS->block_xy_update) set_window_size (x, y);
	}
      }
    else if (strcmp (w2, "pos") == 0)
      {
      if(!GLOBALS->ignore_savefile_pos)
	{
	/* Main window position.  */
	int x, y;
	sscanf (w, "%d %d", &x, &y);
	if(!GLOBALS->block_xy_update) set_window_xypos (x, y);
	}
      }
    else if (strcmp (w2, "pattern_trace") == 0)
      {
      int which_ctx = 0;
      sscanf (w, "%d", &which_ctx);
      if((which_ctx>=0)&&(which_ctx<WAVE_NUM_STRACE_WINDOWS))
		{
		GLOBALS->strace_ctx = &GLOBALS->strace_windows[GLOBALS->strace_current_window = which_ctx];
		}
      }
    else if (strcmp (w2, "ruler") == 0)
      {
      GLOBALS->ruler_origin = GLOBALS->ruler_step = LLDescriptor(0);
      sscanf(w, TTFormat" "TTFormat, &GLOBALS->ruler_origin, &GLOBALS->ruler_step);
      }
    else if (strcmp (w2, "timestart") == 0)
      {
      sscanf(w, TTFormat, &GLOBALS->timestart_from_savefile);
      GLOBALS->timestart_from_savefile_valid = 1;
      }
#if WAVE_USE_GTK2
    else if (strcmp (w2, "treeopen") == 0)
	{
	while(*w)
		{
		if(!isspace((int)(unsigned char)*w))
			{
			break;
			}
		w++;
		}

	if(GLOBALS->ctree_main)
		{
		force_open_tree_node(w);
		}
		else
		{
		/* cache values until ctree_main is created */
		struct string_chain_t *t = calloc_2(1, sizeof(struct string_chain_t));
		t->str = strdup_2(w);

		if(!GLOBALS->treeopen_chain_curr)
			{
			GLOBALS->treeopen_chain_head = GLOBALS->treeopen_chain_curr = t;
			}
			else
			{
			GLOBALS->treeopen_chain_curr->next = t;
			GLOBALS->treeopen_chain_curr = t;
			}
		}
	}
#endif
    else if (strcmp (w2, "markername") == 0)
	{
	char *pnt = w;
	int which;

	if((*pnt) && (isspace((int)(unsigned char)*pnt))) pnt++;

	if(*pnt)
		{
		which = (*pnt) - 'A';
		if((which >=0) && (which <= 25))
			{
			pnt++;

			if(*pnt)
				{
				if(GLOBALS->marker_names[which]) free_2(GLOBALS->marker_names[which]);
				GLOBALS->marker_names[which] = strdup_2(pnt);
				}
			}
		}
	}
    else
      {
	/* Unknown attribute.  Forget it.  */
	return 0;
      }
  }
	else
	{
	int rc = maketraces(w, alias, 0);
	if(rc)
		{
		return(rc);
		}
		else
		{
		char *newl = synth_blastvec(w);

		if(newl)
			{
			rc = parsewavline(newl, alias, depth+1);
			free_2(newl);
			}

		return(rc);
		}
	}

return(0);
}


/****************/
/* LX2 variants */
/****************/

/*
 * Make solitary traces from wildcarded signals...
 */
int maketraces_lx2(char *str, char *alias, int quick_return)
{
char *pnt, *wild;
char ch, wild_active=0;
int len;
int i;
int made = 0;

pnt=str;
while((ch=*pnt))
	{
	if(ch=='*') 
		{
		wild_active=1;
		break;
		}
	pnt++;
	}

if(!wild_active)	/* short circuit wildcard evaluation with bsearch */
	{
	struct symbol *s;

	if(str[0]=='(')
		{
		for(i=1;;i++)
			{
			if(str[i]==0) return(0);
			if((str[i]==')')&&(str[i+1])) {i++; break; }
			}

		if((s=symfind(str+i, NULL)))
			{
			lx2_set_fac_process_mask(s->n);
			made = ~0;
			}
		return(made);
		}
		else
		{
		if((s=symfind(str, NULL)))
			{
			lx2_set_fac_process_mask(s->n);
			made = ~0;
			}
		return(made);
		}
	}

while(1)
{
pnt=str;
len=0;

while(1)
	{
	ch=*pnt++;
	if(isspace((int)(unsigned char)ch)||(!ch)) break;
	len++;
	}

if(len)
	{
	wild=(char *)calloc_2(1,len+1);
	memcpy(wild,str,len);
	wave_regex_compile(wild, WAVE_REGEX_WILD);

	for(i=0;i<GLOBALS->numfacs;i++)
		{
		if(wave_regex_match(GLOBALS->facs[i]->name, WAVE_REGEX_WILD))
			{
			lx2_set_fac_process_mask(GLOBALS->facs[i]->n);
			made = ~0;
			if(quick_return) break;
			}
		}

	free_2(wild);
	}

if(!ch) break;
str=pnt;
}
return(made);
}


/*
 * Create a vector from wildcarded signals...
 */
int makevec_lx2(char *str)
{
char *pnt, *pnt2, *wild=NULL;
char ch, ch2, wild_active;
int len;
int i;
int rc = 0;

while(1)
{
pnt=str;
len=0;

while(1)
	{
	ch=*pnt++;
	if(isspace((int)(unsigned char)ch)||(!ch)) break;
	len++;
	}

if(len)
	{
	wild=(char *)calloc_2(1,len+1);
	memcpy(wild,str,len);

	DEBUG(printf("WILD: %s\n",wild));

	wild_active=0;
	pnt2=wild;
	while((ch2=*pnt2))
		{
		if(ch2=='*') 
			{
			wild_active=1;
			break;
			}
		pnt2++;
		}

	if(!wild_active)	/* short circuit wildcard evaluation with bsearch */
		{
		struct symbol *s;
		if(wild[0]=='(')
			{
			for(i=1;;i++)
				{
				if(wild[i]==0) break;
				if((wild[i]==')')&&(wild[i+1])) 
					{
					i++; 
					s=symfind(wild+i, NULL);
					if(s)
						{
						lx2_set_fac_process_mask(s->n);
						rc = 1;
						}
					break;
					}
				}
			}
			else
			{
			if((s=symfind(wild, NULL)))	
				{
				lx2_set_fac_process_mask(s->n);
				rc = 1;
				}
			}
		}
		else
		{
		wave_regex_compile(wild, WAVE_REGEX_WILD);
		for(i=GLOBALS->numfacs-1;i>=0;i--)	/* to keep vectors in little endian hi..lo order */
			{
			if(wave_regex_match(GLOBALS->facs[i]->name, WAVE_REGEX_WILD))
				{
				lx2_set_fac_process_mask(GLOBALS->facs[i]->n);
				rc = 1;
				}
			}
		}
	free_2(wild);
	}

if(!ch) break;
str=pnt;
}

return(rc);
}


/*
 * Parse a line of the wave file and act accordingly.. 
 * Returns nonzero if trace(s) added.
 */
int parsewavline_lx2(char *w, char *alias, int depth)
{
  int made = 0;
  int i;
  int len;
  char *w2;
  char *prefix, *suffix, *new;
  char *prefix_init, *w2_init;
  unsigned int mode;


  if(!(len=strlen(w))) return(0);
  if(*(w+len-1)=='\n')
    {
      *(w+len-1)=0x00; /* strip newline if present */
      len--;
      if(!len) return(0);
    }

  while(1)
    {
      if(isspace((int)(unsigned char)*w)) { w++; continue; }
      if(!(*w)) return(0);	/* no args */
      break;			/* start grabbing chars from here */
    }

  w2=w;

/* sscanf(w2,"%s",prefix); */

 prefix=(char *)wave_alloca(len+1);
 suffix=(char *)wave_alloca(len+1);
 new=(char *)wave_alloca(len+1);
 new[0] = 0; /* scan-build : in case there are weird mode problems */

 prefix_init = prefix;
 w2_init = new;
 mode = 0; /* 0 = before "{", 1 = after "{", 2 = after "}" or " " */

 while(*w2)
   {
     if((mode == 0) && (*w2 == '{'))
       {
	 mode = 1;
	 w2++;
       }
     else if((mode == 1) && (*w2 == '}'))
       {

	 *(prefix) = '\0';
	 mode = 2;
	 w2++;
       }
     else if((mode == 0) && (*w2 == ' '))
       {
	 *(prefix) = '\0';
	 strcpy(new, w2);
	 mode = 2;
	 w2++;
	 new++;
       }
     else
       {
	 strcpy(new, w2);
	 if (mode != 2)
	   {
	     strcpy(prefix, w2);
	     prefix++;
	   }
	 w2++;
	 new++;
       }
   }

 prefix = prefix_init;
 w2 = w2_init;

 /* printf("IIIII |%s| %s\n", prefix, w2); */

if(*w2=='[')
	{
	}
else
if(*w2=='*')
	{
	}
else
if(*w2=='-')
	{
	}
else
if(*w2=='>')
	{
	}
else
if(*w2=='@')
	{
	}
else
if(*w2=='+')
	{
	/* handle aliasing */
	struct symbol *s;
	sscanf(w2+strlen(prefix),"%s",suffix);

	if(suffix[0]=='(')
		{
		for(i=1;;i++)
			{
			if(suffix[i]==0) return(0);
			if((suffix[i]==')')&&(suffix[i+1])) {i++; break; }
			}

		s=symfind(suffix+i, NULL);
		if(s)
			{
			lx2_set_fac_process_mask(s->n);
			made = ~0;
			}
                else
                        {
                        char *lp = strrchr(suffix+i, '[');
			if(lp)
				{
				char *ns = malloc_2(strlen(suffix+i) + 32);
				char *colon = strchr(lp+1, ':');
				int msi, lsi, bval, actual;
				*lp = 0;

				bval = atoi(suffix+1);
				if(colon)
					{
					msi = atoi(lp+1);
					lsi = atoi(colon+1);
	
					if(lsi > msi)
						{
						actual = msi + bval;
						}
						else
						{
						actual = msi - bval;
						}
					}
					else
					{
					actual = bval; /* punt */
					}

				sprintf(ns, "%s[%d]", suffix+i, actual);
				*lp = '[';			

				s=symfind(ns, NULL);
				free_2(ns);
				if(s)
					{
	                                lx2_set_fac_process_mask(s->n);
	                                made = ~0;
					}
				}
			}

		return(made);
		}
	else
	  {
	    int rc;
	    char *newl   = strdup_2(w2+strlen(prefix));
	    char *nalias = strdup_2(prefix+1);

	    rc = parsewavline_lx2(newl, nalias, depth);
	    if (newl)   free_2(newl);
	    if (nalias) free_2(nalias);

	    return rc;
	  }

	/* 	{ */
/* 		if((s=symfind(suffix, NULL))) */
/* 			{ */
/* 			lx2_set_fac_process_mask(s->n); */
/* 			made = ~0; */
/* 			} */
/* 		return(made); */
/* 		} */
	}
else
if((*w2=='#')||(*w2==':'))
	{
	int rc;

	/* handle bitvec, parsing extra time info and such is inefficient but ok for ":" case */
	w2=w2+strlen(prefix);
	while(1)
		{
		if(isspace((int)(unsigned char)*w2)) { w2++; continue; }
		if(!(*w2)) return(0);	/* no more args */	
		break;			/* start grabbing chars from here */
		}

	rc = makevec_lx2(w2);
	if((!rc)&&(!depth))		/* don't try vectorized if we're re-entrant */
		{
		char *sp = strchr(w2, ' ');
		char *lbrack;

		if(sp)
			{
			*sp = 0;

			lbrack = strrchr(w2, '[');

			if(lbrack)
				{
				char *w3;
				char *rbrack           = strrchr(w2,   ']');
				char *rightmost_lbrack = strrchr(sp+1, '[');

				if(rbrack && rightmost_lbrack)
					{
					*rbrack = 0;

					w3 = malloc_2(strlen(w2) + 1 + strlen(rightmost_lbrack+1) + 1);
					sprintf(w3, "%s:%s", w2, rightmost_lbrack+1);

					made = maketraces_lx2(w3, alias, 1);
					free_2(w3);
					}

				if(0)	/* this is overkill for now with possible delay implications so commented out */
				if(!made)
					{
					*lbrack = 0;

					w3 = malloc_2(1 + strlen(w2) + 5);
					sprintf(w3, "^%s\\[.*", w2);
					maketraces_lx2(w3, alias, 1);
					free_2(w3);
					}
				}
			}
		}

	return(made);
	}
else
if(*w2=='!')
	{
	}
	else
if(*w2=='?')
	{
	}
else if(*w2=='^')
	{
	}
	else
	{
	  made = maketraces_lx2(w, alias, 0);
        if(!made)
                {
                char *newl = synth_blastvec(w);

		if(newl)
			{
	                made = parsewavline_lx2(newl, alias, depth+1);
	                free_2(newl);
			}
                }
	}

return(made);
}


/****************/
/* LX2 variants */
/****************/

