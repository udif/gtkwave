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

#include <config.h>
#include "globals.h"
#include "tree.h"
#include "vcd.h"

enum TreeBuildTypes { MAKETREE_FLATTEN, MAKETREE_LEAF, MAKETREE_NODE };


#ifdef WAVE_USE_STRUCT_PACKING
struct tree *talloc_2(size_t siz)
{
if(GLOBALS->talloc_pool_base)
	{
	if((siz + GLOBALS->talloc_idx) <= WAVE_TALLOC_POOL_SIZE)
		{
		unsigned char *m = GLOBALS->talloc_pool_base + GLOBALS->talloc_idx;
		GLOBALS->talloc_idx += siz;
		return((struct tree *)m);
		}
	else
	if(siz >= WAVE_TALLOC_ALTREQ_SIZE)
		{
		return(calloc_2(1, siz));
		}
	}

GLOBALS->talloc_pool_base = calloc_2(1, WAVE_TALLOC_POOL_SIZE);
GLOBALS->talloc_idx = 0;
return(talloc_2(siz));
}
#endif


/*
 * init pointers needed for n-way tree
 */
void init_tree(void)
{
GLOBALS->module_tree_c_1=(char *)malloc_2(GLOBALS->longestname+1);
}


/*
 * extract the next part of the name in the flattened
 * hierarchy name.  return ptr to next name if it exists
 * else NULL
 */
static const char *get_module_name(const char *s)
{
char ch;
char *pnt;

pnt=GLOBALS->module_tree_c_1;

for(;;)
	{
	ch=*(s++);

	if((ch==GLOBALS->hier_delimeter) || (ch == '|'))
		{
		*(pnt)=0;	
		GLOBALS->module_len_tree_c_1 = pnt - GLOBALS->module_tree_c_1;
		return(s);		
		}

	if(!(*(pnt++)=ch))
		{
		GLOBALS->module_len_tree_c_1 = pnt - GLOBALS->module_tree_c_1;
		return(NULL);	/* nothing left to extract */		
		}
	}
}


/*
 * decorated module add
 */
void allocate_and_decorate_module_tree_node(unsigned char ttype, const char *scopename, const char *compname, uint32_t scopename_len, uint32_t compname_len)
{
struct tree *t;
int mtyp = WAVE_T_WHICH_UNDEFINED_COMPNAME;

if(compname && compname[0] && strcmp(scopename, compname))
	{
	int ix = add_to_comp_name_table(compname, compname_len);
	if(ix)
		{
		ix--;
		mtyp = WAVE_T_WHICH_COMPNAME_START - ix;
		}
	}

if(GLOBALS->treeroot)
	{
	if(GLOBALS->mod_tree_parent)
		{
		t = GLOBALS->mod_tree_parent->child;
		while(t)
			{
			if(!strcmp(t->name, scopename))
				{
				GLOBALS->mod_tree_parent = t;
				return;
				}
			t = t->next;
			}

		t = talloc_2(sizeof(struct tree) + scopename_len);
		strcpy(t->name, scopename);
		t->kind = ttype;
		t->t_which = mtyp;

		if(GLOBALS->mod_tree_parent->child)
			{
			t->next = GLOBALS->mod_tree_parent->child;
			}					
		GLOBALS->mod_tree_parent->child = t;
		GLOBALS->mod_tree_parent = t;
		}
		else
		{
		t = GLOBALS->treeroot;
			while(t)
			{
			if(!strcmp(t->name, scopename))
				{
				GLOBALS->mod_tree_parent = t;
				return;
				}
			t = t->next;
			}

		t = talloc_2(sizeof(struct tree) + scopename_len);
		strcpy(t->name, scopename);
		t->kind = ttype;
		t->t_which = mtyp;

		t->next = GLOBALS->treeroot;
		GLOBALS->mod_tree_parent = GLOBALS->treeroot = t;
		}
	}
	else
	{
	t = talloc_2(sizeof(struct tree) + scopename_len);
	strcpy(t->name, scopename);
	t->kind = ttype;
	t->t_which = mtyp;

	GLOBALS->mod_tree_parent = GLOBALS->treeroot = t;
	}
}


/*
 * adds back netnames
 */
int treegraft(struct tree **t)
{
struct tree *tx = GLOBALS->terminals_tchain_tree_c_1;
struct tree *t2;
struct tree *par;

while(tx)
	{
	t2 = tx->next;

	par = tx->child;
	tx->child = NULL;

	if(par)
		{
		if(par->child)
			{
			tx->next = par->child;
			par->child = tx;
			}
			else
			{
			par->child = tx;
			tx->next = NULL;
			}
		}
		else
		{
		if(*t)
			{
			tx->next = (*t)->next;
			(*t)->next = tx;
			}
			else
			{
			*t = tx;
			tx->next = NULL;
			}
		}

	tx = t2;
	}

return(1);
}


/*
 * unswizzle extended names in tree
 */ 
void treenamefix_str(char *s)
{
while(*s)
	{
	if(*s==VCDNAM_ESCAPE) *s=GLOBALS->hier_delimeter;
	s++;
	}
}

void treenamefix(struct tree *t)
{
struct tree *tnext;
if(t->child) treenamefix(t->child);

tnext = t->next;

while(tnext)
	{
	if(tnext->name) treenamefix_str(tnext->name);
	tnext=tnext->next;
	}

if(t->name) treenamefix_str(t->name);
}


/*
 * for debugging purposes only
 */
void treedebug(struct tree *t, char *s)
{
while(t)
	{
	char *s2;

	s2=(char *)malloc_2(strlen(s)+strlen(t->name)+2);
	strcpy(s2,s);
	strcat(s2,".");
	strcat(s2,t->name);
	
	if(t->child)
		{
		treedebug(t->child, s2);
		}

	if(t->t_which>=0) /* for when valid netnames like A.B.C, A.B.C.D exist (not legal excluding texsim) */
			/* otherwise this would be an 'else' */
		{
		printf("%3d) %s\n", t->t_which, s2);
		}

	free_2(s2);
	t=t->next;
	}
}


static GtkCTreeNode *maketree_nodes(GtkCTreeNode *subtree, struct tree *t2, GtkCTreeNode *sibling, int mode)
{
char *tmp, *tmp2, *tmp3;
gchar *text [1];
GdkDrawable *pxm, *msk;

if(t2->t_which >= 0)
	{
        if(GLOBALS->facs[t2->t_which]->vec_root)
        	{
                if(GLOBALS->autocoalesce)
                	{
                        if(GLOBALS->facs[t2->t_which]->vec_root!=GLOBALS->facs[t2->t_which])
                        	{
				return(NULL);
                                }

                        tmp2=makename_chain(GLOBALS->facs[t2->t_which]);
                        tmp3=leastsig_hiername(tmp2);
                        tmp=wave_alloca(strlen(tmp3)+4);
                        strcpy(tmp,   "[] ");
                        strcpy(tmp+3, tmp3);
                        free_2(tmp2);
                        }
                        else
                        {
                        tmp=wave_alloca(strlen(t2->name)+4);
                        strcpy(tmp,   "[] ");
                        strcpy(tmp+3, t2->name);
                        }
		}
                else
                {
                tmp=t2->name;
                }
	}
        else
        {
	if(t2->t_which == WAVE_T_WHICH_UNDEFINED_COMPNAME)
		{
        	tmp=t2->name;
		}
		else
		{
		int thidx = -t2->t_which + WAVE_T_WHICH_COMPNAME_START;
		if((thidx >= 0) && (thidx < GLOBALS->comp_name_serial))
			{
			char *sc = GLOBALS->comp_name_idx[thidx];
			int tlen = strlen(t2->name) + 2 + 1 + strlen(sc) + 1 + 1;
			tmp = wave_alloca(tlen);
			sprintf(tmp, "%s  (%s)", t2->name, sc);
			}
			else
			{
	        	tmp=t2->name;	/* should never get a value out of range here! */
			}
		}
        }

text[0]=tmp;
switch(mode)
	{
	case MAKETREE_FLATTEN:
		if(t2->child)
			{
		        sibling = gtk_ctree_insert_node (GLOBALS->ctree_main, subtree, sibling, text, 3,
                	                       NULL, NULL, NULL, NULL,
                	                       FALSE, FALSE);
			gtk_ctree_node_set_row_data(GLOBALS->ctree_main, sibling, t2);
			maketree(sibling, t2->child);
			}
			else
			{
		        sibling = gtk_ctree_insert_node (GLOBALS->ctree_main, subtree, sibling, text, 3,
                	                       NULL, NULL, NULL, NULL,
                	                       TRUE, FALSE);
			gtk_ctree_node_set_row_data(GLOBALS->ctree_main, sibling, t2);
			}
		break;

	default:
		switch(t2->kind)
			{
   			case TREE_VCD_ST_MODULE:	pxm = GLOBALS->hiericon_module_pixmap; msk = GLOBALS->hiericon_module_mask; break;
   			case TREE_VCD_ST_TASK:		pxm = GLOBALS->hiericon_task_pixmap; msk = GLOBALS->hiericon_task_mask; break;
   			case TREE_VCD_ST_FUNCTION:	pxm = GLOBALS->hiericon_function_pixmap; msk = GLOBALS->hiericon_function_mask; break;
   			case TREE_VCD_ST_BEGIN:		pxm = GLOBALS->hiericon_begin_pixmap; msk = GLOBALS->hiericon_begin_mask; break;
   			case TREE_VCD_ST_FORK:		pxm = GLOBALS->hiericon_fork_pixmap; msk = GLOBALS->hiericon_fork_mask; break;

			case TREE_VHDL_ST_DESIGN:	pxm = GLOBALS->hiericon_design_pixmap; msk = GLOBALS->hiericon_design_mask; break;
			case TREE_VHDL_ST_BLOCK:	pxm = GLOBALS->hiericon_block_pixmap; msk = GLOBALS->hiericon_block_mask; break;
			case TREE_VHDL_ST_GENIF:	pxm = GLOBALS->hiericon_generateif_pixmap; msk = GLOBALS->hiericon_generateif_mask; break;
			case TREE_VHDL_ST_GENFOR:	pxm = GLOBALS->hiericon_generatefor_pixmap; msk = GLOBALS->hiericon_generatefor_mask; break;
			case TREE_VHDL_ST_INSTANCE:	pxm = GLOBALS->hiericon_instance_pixmap; msk = GLOBALS->hiericon_instance_mask; break;
			case TREE_VHDL_ST_PACKAGE:	pxm = GLOBALS->hiericon_package_pixmap; msk = GLOBALS->hiericon_package_mask; break;

			case TREE_VHDL_ST_SIGNAL:	pxm = GLOBALS->hiericon_signal_pixmap; msk = GLOBALS->hiericon_signal_mask; break;
			case TREE_VHDL_ST_PORTIN:	pxm = GLOBALS->hiericon_portin_pixmap; msk = GLOBALS->hiericon_portin_mask; break;
			case TREE_VHDL_ST_PORTOUT:	pxm = GLOBALS->hiericon_portout_pixmap; msk = GLOBALS->hiericon_portout_mask; break;
			case TREE_VHDL_ST_PORTINOUT:	pxm = GLOBALS->hiericon_portinout_pixmap; msk = GLOBALS->hiericon_portinout_mask; break;
			case TREE_VHDL_ST_BUFFER:	pxm = GLOBALS->hiericon_buffer_pixmap; msk = GLOBALS->hiericon_buffer_mask; break;
			case TREE_VHDL_ST_LINKAGE:	pxm = GLOBALS->hiericon_linkage_pixmap; msk = GLOBALS->hiericon_linkage_mask; break;

			default:			pxm = msk = NULL; break;
			}

	        sibling = gtk_ctree_insert_node (GLOBALS->ctree_main, subtree, sibling, text, 3,
               	                       pxm, msk, pxm, msk,
               	                       (mode==MAKETREE_LEAF), FALSE);
		gtk_ctree_node_set_row_data(GLOBALS->ctree_main, sibling, t2);
		break;
	}

return(sibling);
}


/*
 * return least significant member name of a hierarchy
 * (used for tree and hier vec_root search hits)
 */
char *leastsig_hiername(char *nam)
{
char *t, *pnt=NULL;
char ch;

if(nam)
	{
	t=nam;
	while((ch=*(t++)))
		{
		if(ch==GLOBALS->hier_delimeter) pnt=t;
		}
	}

return(pnt?pnt:nam);
}

/**********************************/
/* Experimental treesorting code  */
/* (won't directly work with lxt2 */
/* because alias hier is after    */
/* fac hier so fix with partial   */
/* mergesort...)                  */
/**********************************/

/*
 * sort the hier tree..should be faster than
 * moving numfacs longer strings around
 */

static int tree_qsort_cmp(const void *v1, const void *v2)
{
struct tree *t1 = *(struct tree **)v1;
struct tree *t2 = *(struct tree **)v2;

return(sigcmp(t2->name, t1->name));	/* because list must be in rvs */
}

static void treesort_2(struct tree *t, struct tree *p, struct tree ***tm, int *tm_siz)
{
struct tree *it;
struct tree **srt;
int cnt;
int i;

if(t->next)
	{
	it = t;
	cnt = 0;
	do	{
		cnt++;
		it=it->next;
		} while(it);

	if(cnt > *tm_siz)
		{
		*tm_siz = cnt;
		if(*tm) { free_2(*tm); }
		*tm = malloc_2((cnt+1) * sizeof(struct tree *));
		}
	srt = *tm;

	for(i=0;i<cnt;i++)
		{
		srt[i] = t;
		t=t->next;
		}
	srt[i] = NULL;

	qsort((void *)srt, cnt, sizeof(struct tree *), tree_qsort_cmp);

	if(p)
		{
		p->child = srt[0];
		}
		else
		{
		GLOBALS->treeroot = srt[0];
		}

	for(i=0;i<cnt;i++)
		{
		srt[i]->next = srt[i+1];
		}

	it = srt[0];
	for(i=0;i<cnt;i++)
		{
		if(it->child)
			{
			treesort_2(it->child, it, tm, tm_siz);
			}
		it = it->next;
		}
	}
else if (t->child)
	{
	treesort_2(t->child, t, tm, tm_siz);
	}
}


void treesort(struct tree *t, struct tree *p)
{
struct tree **tm = NULL;
int tm_siz = 0;

treesort_2(t, p, &tm, &tm_siz);
if(tm)
	{
	free_2(tm);
	}

}


void order_facs_from_treesort_2(struct tree *t)
{
while(t)
	{
	if(t->child)
		{
		order_facs_from_treesort_2(t->child);
		}

	if(t->t_which>=0) /* for when valid netnames like A.B.C, A.B.C.D exist (not legal excluding texsim) */
			/* otherwise this would be an 'else' */
		{
		GLOBALS->facs2_tree_c_1[GLOBALS->facs2_pos_tree_c_1] = GLOBALS->facs[t->t_which];
		t->t_which = GLOBALS->facs2_pos_tree_c_1--;
		}

	t=t->next;
	}
}


void order_facs_from_treesort(struct tree *t, void *v)
{
struct symbol ***f = (struct symbol ***)v; /* eliminate compiler warning in tree.h as symbol.h refs tree.h */

GLOBALS->facs2_tree_c_1=(struct symbol **)malloc_2(GLOBALS->numfacs*sizeof(struct symbol *));
GLOBALS->facs2_pos_tree_c_1 = GLOBALS->numfacs-1;
order_facs_from_treesort_2(t);

if(GLOBALS->facs2_pos_tree_c_1>=0)
	{
	fprintf(stderr, "Internal Error: GLOBALS->facs2_pos_tree_c_1 = %d\n",GLOBALS->facs2_pos_tree_c_1);
	fprintf(stderr, "[This is usually the result of multiply defined facilities.]\n");
	exit(255);
	}

free_2(*f);
*f = GLOBALS->facs2_tree_c_1;
GLOBALS->facs2_tree_c_1 = NULL;
}


void build_tree_from_name(const char *s, int which)
{
struct tree *t, *nt;
struct tree *tchain, *tchain_iter;
struct tree *prevt;

if(s==NULL || !s[0]) return;

t = GLOBALS->treeroot;

if(t)
	{
	prevt = NULL;
	while(s)
		{
rs:		s=get_module_name(s);

		if(t && !strcmp(t->name, GLOBALS->module_tree_c_1))
			{
			prevt = t;
			t = t->child;
			continue;
			}

		tchain = tchain_iter = t;
		if(s && t)
			{
		      	nt = t->next;
		      	while(nt)
				{
				if(nt && !strcmp(nt->name, GLOBALS->module_tree_c_1))
					{
					/* move to front to speed up next compare if in same hier during build */
					if(prevt)
						{
						tchain_iter->next = nt->next;
						nt->next = tchain;
						prevt->child = nt;
						}

					prevt = nt;
					t = nt->child;
					goto rs;
					}

				tchain_iter = nt;
				nt = nt->next;
				}
			}

		nt=(struct tree *)talloc_2(sizeof(struct tree)+GLOBALS->module_len_tree_c_1);
		memcpy(nt->name, GLOBALS->module_tree_c_1, GLOBALS->module_len_tree_c_1);

		if(s)
			{
			nt->t_which = WAVE_T_WHICH_UNDEFINED_COMPNAME;

			if(prevt)				/* make first in chain */
				{
				nt->next = prevt->child;
				prevt->child = nt;
				}
				else				/* make second in chain as it's toplevel */
				{
				nt->next = tchain->next;	
				tchain->next = nt;
				}
			}
			else
			{
			nt->child = prevt;			/* parent */
			nt->t_which = which;
			nt->next = GLOBALS->terminals_tchain_tree_c_1;
			GLOBALS->terminals_tchain_tree_c_1 = nt;
			return;
			}
	
		/* blindly clone fac from next part of hier on down */
		t = nt;
		while(s)
			{
			s=get_module_name(s);
		
			nt=(struct tree *)talloc_2(sizeof(struct tree)+GLOBALS->module_len_tree_c_1);
			memcpy(nt->name, GLOBALS->module_tree_c_1, GLOBALS->module_len_tree_c_1);

			if(s)
				{
				nt->t_which = WAVE_T_WHICH_UNDEFINED_COMPNAME;
				t->child = nt;
				t = nt;
				}
				else
				{
				nt->child = t;			/* parent */
				nt->t_which = which;
				nt->next = GLOBALS->terminals_tchain_tree_c_1;
				GLOBALS->terminals_tchain_tree_c_1 = nt;
				}
			}
		}
	}
else	
	{
	/* blindly create first fac in the tree (only ever called once) */
	while(s)
		{
		s=get_module_name(s);

		nt=(struct tree *)talloc_2(sizeof(struct tree)+GLOBALS->module_len_tree_c_1);
		memcpy(nt->name, GLOBALS->module_tree_c_1, GLOBALS->module_len_tree_c_1);

		if(!s) nt->t_which=which; else nt->t_which = WAVE_T_WHICH_UNDEFINED_COMPNAME;

		if(GLOBALS->treeroot)
			{
			t->child = nt;
			t = nt;
			}
			else
			{
			GLOBALS->treeroot = t = nt;
			}
		}
	}
}


/* ######################## */
/* ## compatibility code ## */
/* ######################## */

/*
 * tree widgets differ between GTK2 and GTK1 so we need two different
 * maketree() routines
 */
#if WAVE_USE_GTK2

/*
 * GTK2: build the tree.
 */
void maketree2(GtkCTreeNode *subtree, struct tree *t, int depth, GtkCTreeNode *graft)
{
GtkCTreeNode *sibling=NULL, *sibling_test;
struct tree *t2;

#ifndef WAVE_DISABLE_FAST_TREE
if(depth > 1) return;
#endif

/* 
 * TG reworked treesearch widget so there is no need to 
 * process anything other than nodes.  Leaves are handled
 * in the filtered list below the node expand/contract
 * tree
 */
t2=t;
while(t2)
	{
#ifndef WAVE_DISABLE_FAST_TREE
	if(depth < 1) 
#endif
		{ 
		t2->children_in_gui = 1; 
		}

	if(t2->child)
		{
		if(!graft)
			{
			sibling_test=maketree_nodes(subtree, t2, sibling, MAKETREE_NODE);
			}
			else
			{
			sibling_test = graft;
			}
		if(sibling_test)
			{
			GLOBALS->any_tree_node = sibling_test;
			maketree2(sibling=sibling_test, t2->child, depth + 1, NULL);
			}
		}

	if(graft) break;
	t2=t2->next;
	}
}


void maketree(GtkCTreeNode *subtree, struct tree *t)
{
maketree2(subtree, t, 0, NULL);
}

#else

/*
 * GTK1: build the tree.
 */
void maketree(GtkCTreeNode *subtree, struct tree *t)
{
GtkCTreeNode *sibling=NULL, *sibling_test;
struct tree *t2;

if(!GLOBALS->hier_grouping)
	{
	t2=t;
	while(t2)
		{
		sibling_test=maketree_nodes(subtree, t2, sibling, MAKETREE_FLATTEN);	
		sibling=sibling_test?sibling_test:sibling;
		t2=t2->next;
		}
	}
	else
	{
	t2=t;
	while(t2)
		{
		if(!t2->child)
			{
			sibling_test=maketree_nodes(subtree, t2, sibling, MAKETREE_LEAF);
			if(sibling_test)
				{
				maketree(sibling=sibling_test, t2->child);
				}
			}
	
		t2=t2->next;
		}

	t2=t;
	while(t2)
		{
		if(t2->child)
			{
			sibling_test=maketree_nodes(subtree, t2, sibling, MAKETREE_NODE);
			if(sibling_test)
				{
				maketree(sibling=sibling_test, t2->child);
				}
			}
	
		t2=t2->next;
		}
	}
}

#endif

/*
 * $Id: tree.c,v 1.16 2011/01/23 21:55:54 gtkwave Exp $
 * $Log: tree.c,v $
 * Revision 1.16  2011/01/23 21:55:54  gtkwave
 * more optimizations
 *
 * Revision 1.15  2011/01/21 22:40:28  gtkwave
 * pass string lengths from api directly to code to avoid length calculations
 *
 * Revision 1.14  2011/01/19 16:18:19  gtkwave
 * fix for large allocations of tree alloc
 *
 * Revision 1.13  2011/01/19 06:36:31  gtkwave
 * added tree allocation pool when misaligned structs are enabled
 *
 * Revision 1.12  2011/01/18 02:38:35  gtkwave
 * added extra spacing between component type name for readability
 *
 * Revision 1.11  2011/01/18 00:00:12  gtkwave
 * preliminary tree component support
 *
 * Revision 1.10  2011/01/17 19:24:21  gtkwave
 * tree modifications to support decorated internal hierarchy nodes
 *
 * Revision 1.9  2010/09/23 22:04:55  gtkwave
 * added incremental SST build code
 *
 * Revision 1.8  2009/07/02 18:50:47  gtkwave
 * decorate VCD module trees with type info, add move to front to buildname
 *
 * Revision 1.7  2009/07/01 21:58:32  gtkwave
 * more GHW module type adds for icons in hierarchy window
 *
 * Revision 1.6  2009/07/01 18:22:35  gtkwave
 * added VHDL (GHW) instance types as icons
 *
 * Revision 1.5  2009/07/01 16:47:47  gtkwave
 * move decorated module alloc routine to tree.c
 *
 * Revision 1.4  2009/07/01 07:39:12  gtkwave
 * decorating hierarchy tree with module type info
 *
 * Revision 1.3  2008/12/26 20:47:59  gtkwave
 * fix for stack overflow crash on dumpfiles with very many signals
 *
 * Revision 1.2  2007/08/26 21:35:46  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.5  2007/08/25 19:43:46  gtkwave
 * header cleanups
 *
 * Revision 1.1.1.1.2.4  2007/08/21 22:35:40  gtkwave
 * prelim tree state merge
 *
 * Revision 1.1.1.1.2.3  2007/08/07 03:18:55  kermin
 * Changed to pointer based GLOBAL structure and added initialization function
 *
 * Revision 1.1.1.1.2.2  2007/08/06 03:50:49  gtkwave
 * globals support for ae2, gtk1, cygwin, mingw.  also cleaned up some machine
 * generated structs, etc.
 *
 * Revision 1.1.1.1.2.1  2007/08/05 02:27:24  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1  2007/05/30 04:27:35  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:17  gtkwave
 * initial release
 *
 */

