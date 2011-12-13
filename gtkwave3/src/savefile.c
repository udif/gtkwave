/*
 * Copyright (c) Tony Bybell 2012.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"
#include <config.h>
#include "savefile.h"
#include "hierpack.h"
#if !defined __MINGW32__ && !defined _MSC_VER
#include <sys/stat.h>
#endif

char *append_array_row(nptr n)
{
int was_packed = HIER_DEPACK_ALLOC;
char *hname = hier_decompress_flagged(n->nname, &was_packed);

#ifdef WAVE_ARRAY_SUPPORT
if(!n->array_height)
#endif
      	{
	strcpy(GLOBALS->buf_menu_c_1, hname);
        }
#ifdef WAVE_ARRAY_SUPPORT
        else
	{
	sprintf(GLOBALS->buf_menu_c_1, "%s{%d}", hname, n->this_row);
        }
#endif

if(was_packed) free_2(hname);

return(GLOBALS->buf_menu_c_1);
}


void write_save_helper(char *savnam, FILE *wave) {
	Trptr t;
	int i;
	unsigned int def=0;
	int sz_x, sz_y;
	TimeType prevshift=LLDescriptor(0);
	int root_x, root_y;
        struct strace *st;
	int s_ctx_iter;
	time_t walltime;

	DEBUG(printf("Write Save Fini: %s\n", savnam));

	time(&walltime);
	fprintf(wave, "[*]\n");
	fprintf(wave, "[*] "WAVE_VERSION_INFO"\n");
	fprintf(wave, "[*] %s",asctime(localtime(&walltime)));
	fprintf(wave, "[*]\n");

	if(GLOBALS->loaded_file_name)
		{
		if((GLOBALS->loaded_file_type == MISSING_FILE)||(GLOBALS->is_optimized_stdin_vcd))
			{
			/* don't emit dumpfile tag */
			}
			else
			{
#if !defined __MINGW32__ && !defined _MSC_VER
			struct stat sbuf;
#endif
#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED || defined __CYGWIN__ || defined HAVE_REALPATH
		        char *can = realpath(GLOBALS->loaded_file_name, NULL);
			char *cansav = realpath(savnam, NULL);
			const int do_free = 1;
#else
			char *can = GLOBALS->loaded_file_name;
			char *cansav = savnam;
			const int do_free = 0;
#endif
			fprintf(wave, "[dumpfile] \"%s\"\n", can);
#if !defined __MINGW32__ && !defined _MSC_VER
			if(!stat(can, &sbuf))
				{
				char *asct = asctime(localtime(&sbuf.st_mtime));
				if(asct)
					{
					char *asct2 = strdup_2(asct);
					char *nl = strchr(asct2, '\n');
					if(nl) *nl = 0;
					fprintf(wave, "[dumpfile_mtime] \"%s\"\n", asct2);
					free_2(asct2);
					fprintf(wave, "[dumpfile_size] %"PRIu64"\n", sbuf.st_size);
					}
				}
#endif
			fprintf(wave, "[savefile] \"%s\"\n", cansav); /* emit also in order to do relative path matching in future... */
			if(do_free)
				{
				free(can);
				}
			}
		}

	fprintf(wave, "[timestart] "TTFormat"\n", GLOBALS->tims.start);

	get_window_size (&sz_x, &sz_y);
	if(!GLOBALS->ignore_savefile_size) fprintf(wave,"[size] %d %d\n", sz_x, sz_y);

	get_window_xypos(&root_x, &root_y);

	if(!GLOBALS->ignore_savefile_pos) fprintf(wave,"[pos] %d %d\n", root_x + GLOBALS->xpos_delta, root_y + GLOBALS->ypos_delta);

	fprintf(wave,"*%f "TTFormat, (float)(GLOBALS->tims.zoom),GLOBALS->tims.marker);

	for(i=0;i<26;i++)
		{
		TimeType nm = GLOBALS->named_markers[i]; /* gcc compiler problem...thinks this is a 'long int' in printf format warning reporting */
		fprintf(wave," "TTFormat,nm);
		}
	fprintf(wave,"\n");

	for(i=0;i<26;i++)
		{
		if(GLOBALS->marker_names[i])
			{
			fprintf(wave, "[markername] %c%s\n", 'A'+i, GLOBALS->marker_names[i]);
			}	
		}

	if(GLOBALS->ruler_step)
		{
		fprintf(wave, "[ruler] "TTFormat" "TTFormat"\n", GLOBALS->ruler_origin, GLOBALS->ruler_step);
		}

#if WAVE_USE_GTK2
	if(GLOBALS->open_tree_nodes)
		{
		dump_open_tree_nodes(wave, GLOBALS->open_tree_nodes);
		}
#endif

#if GTK_CHECK_VERSION(2,4,0)
	if(!GLOBALS->ignore_savefile_pane_pos)
		{
		if(GLOBALS->toppanedwindow)
			{
			fprintf(wave, "[sst_width] %d\n", gtk_paned_get_position(GTK_PANED(GLOBALS->toppanedwindow)));
			}
		if(GLOBALS->panedwindow)
			{
			fprintf(wave, "[signals_width] %d\n", gtk_paned_get_position(GTK_PANED(GLOBALS->panedwindow)));
			}
		if(GLOBALS->expanderwindow)
	        	{
			GLOBALS->sst_expanded = gtk_expander_get_expanded(GTK_EXPANDER(GLOBALS->expanderwindow));
			fprintf(wave, "[sst_expanded] %d\n", GLOBALS->sst_expanded);
			}
		if(GLOBALS->sst_vpaned)
			{
			fprintf(wave, "[sst_vpaned_height] %d\n", gtk_paned_get_position(GTK_PANED(GLOBALS->sst_vpaned)));
			}
		}
#endif

	t=GLOBALS->traces.first;
	while(t)
		{
		if((t->flags!=def)||(t==GLOBALS->traces.first))
			{
			if((t->flags & TR_PTRANSLATED) && (!t->p_filter)) t->flags &= (~TR_PTRANSLATED);
			if((t->flags & TR_FTRANSLATED) && (!t->f_filter)) t->flags &= (~TR_FTRANSLATED);
			if((t->flags & TR_TTRANSLATED) && (!t->t_filter)) t->flags &= (~TR_TTRANSLATED);
			fprintf(wave,"@%x\n",def=t->flags);
			}

		if((t->shift)||((prevshift)&&(!t->shift)))
			{
			fprintf(wave,">"TTFormat"\n", t->shift);
			}
		prevshift=t->shift;

		if(!(t->flags&(TR_BLANK|TR_ANALOG_BLANK_STRETCH)))	
			{
			if(t->t_color)
				{
				fprintf(wave, "[color] %d\n", t->t_color);
				}

			if(t->flags & TR_FTRANSLATED)
				{
				if(t->f_filter && GLOBALS->filesel_filter[t->f_filter])
					{
					fprintf(wave, "^%d %s\n", t->f_filter, GLOBALS->filesel_filter[t->f_filter]);
					}
					else
					{
					fprintf(wave, "^%d %s\n", 0, "disabled");
					}
				}
			else
			if(t->flags & TR_PTRANSLATED)
				{
				if(t->p_filter && GLOBALS->procsel_filter[t->p_filter])
					{
					fprintf(wave, "^>%d %s\n", t->p_filter, GLOBALS->procsel_filter[t->p_filter]);
					}
					else
					{
					fprintf(wave, "^>%d %s\n", 0, "disabled");
					}
				}

			/* NOT an else! */
			if(t->flags & TR_TTRANSLATED)
				{
				if(t->t_filter && GLOBALS->ttranssel_filter[t->t_filter])
					{
					fprintf(wave, "^<%d %s\n", t->t_filter, GLOBALS->ttranssel_filter[t->t_filter]);
					}
					else
					{
					fprintf(wave, "^<%d %s\n", 0, "disabled");
					}
				}

			if(t->vector)
				{
				int ix;
				nptr *nodes;
				bptr bits;
				baptr ba;

				if (HasAlias(t)) { fprintf(wave,"+{%s} ", t->name_full); }
				bits = t->n.vec->bits;
				ba = bits ? bits->attribs : NULL;

				fprintf(wave,"%c{%s}", ba ? ':' : '#', 
						t->n.vec->transaction_cache ? t->n.vec->transaction_cache->bvname : t->n.vec->bvname);

				nodes=t->n.vec->bits->nodes;
				for(ix=0;ix<t->n.vec->bits->nnbits;ix++)
					{
					if(nodes[ix]->expansion)
						{
						fprintf(wave," (%d)%s",nodes[ix]->expansion->parentbit, append_array_row(nodes[ix]->expansion->parent));
						}
						else
						{
						fprintf(wave," %s",append_array_row(nodes[ix]));
						}
					if(ba)
						{
						fprintf(wave, " "TTFormat" %x", ba[ix].shift, ba[ix].flags);
						}
					}
				fprintf(wave,"\n");
				}
				else
				{
				if(HasAlias(t))
					{
					if(t->n.nd->expansion)
						{
						fprintf(wave,"+{%s} (%d)%s\n",t->name_full,t->n.nd->expansion->parentbit, append_array_row(t->n.nd->expansion->parent));
						}
						else
						{
						fprintf(wave,"+{%s} %s\n",t->name_full,append_array_row(t->n.nd));
						}
					}
					else
					{
					if(t->n.nd->expansion)
						{
						fprintf(wave,"(%d)%s\n",t->n.nd->expansion->parentbit, append_array_row(t->n.nd->expansion->parent));
						}
						else
						{
						fprintf(wave,"%s\n",append_array_row(t->n.nd));
						}
					}
				}
			}
			else
			{
			if(!t->name) fprintf(wave,"-\n");
			else fprintf(wave,"-%s\n",t->name);
			}
		t=t->t_next;
		}

	WAVE_STRACE_ITERATOR(s_ctx_iter)
	{
	GLOBALS->strace_ctx = &GLOBALS->strace_windows[GLOBALS->strace_current_window = s_ctx_iter];
	fprintf(wave, "[pattern_trace] %d\n", s_ctx_iter);

	if(GLOBALS->strace_ctx->timearray)
		{
		if(GLOBALS->strace_ctx->shadow_straces)
			{
			swap_strace_contexts();

			st=GLOBALS->strace_ctx->straces;
			if(GLOBALS->strace_ctx->straces)
				{
				fprintf(wave, "!%d%d%d%d%d%d%c%c\n", GLOBALS->strace_ctx->logical_mutex[0], GLOBALS->strace_ctx->logical_mutex[1], GLOBALS->strace_ctx->logical_mutex[2], GLOBALS->strace_ctx->logical_mutex[3], GLOBALS->strace_ctx->logical_mutex[4], GLOBALS->strace_ctx->logical_mutex[5], '@'+GLOBALS->strace_ctx->mark_idx_start, '@'+GLOBALS->strace_ctx->mark_idx_end);
				}

			while(st)
				{
				if(st->value==ST_STRING)
					{
					fprintf(wave, "?\"%s\n", st->string ? st->string : ""); /* search type for this trace is string.. */
					}
					else
					{
					fprintf(wave, "?%02x\n", (unsigned char)st->value);	/* else search type for this trace.. */
					}
			
				t=st->trace;

				if(t->flags!=def)
					{
					if((t->flags & TR_FTRANSLATED) && (!t->f_filter)) t->flags &= (~TR_FTRANSLATED);
					if((t->flags & TR_PTRANSLATED) && (!t->p_filter)) t->flags &= (~TR_PTRANSLATED);
					if((t->flags & TR_TTRANSLATED) && (!t->t_filter)) t->flags &= (~TR_TTRANSLATED);
					fprintf(wave,"@%x\n",def=t->flags);
					}

				if((t->shift)||((prevshift)&&(!t->shift)))
					{
					fprintf(wave,">"TTFormat"\n", t->shift);
					}
				prevshift=t->shift;

				if(!(t->flags&(TR_BLANK|TR_ANALOG_BLANK_STRETCH)))	
					{
					if(t->flags & TR_FTRANSLATED)
						{
						if(t->f_filter && GLOBALS->filesel_filter[t->f_filter])
							{
							fprintf(wave, "^%d %s\n", t->f_filter, GLOBALS->filesel_filter[t->f_filter]);
							}
							else
							{
							fprintf(wave, "^%d %s\n", 0, "disabled");
							}
						}
					else
					if(t->flags & TR_PTRANSLATED)
						{
						if(t->p_filter && GLOBALS->procsel_filter[t->p_filter])
							{
							fprintf(wave, "^>%d %s\n", t->p_filter, GLOBALS->procsel_filter[t->p_filter]);
							}
							else
							{
							fprintf(wave, "^>%d %s\n", 0, "disabled");
							}
						}

					/* NOT an else! */
					if(t->flags & TR_TTRANSLATED)
						{
						if(t->t_filter && GLOBALS->ttranssel_filter[t->t_filter])
							{
							fprintf(wave, "^<%d %s\n", t->t_filter, GLOBALS->ttranssel_filter[t->t_filter]);
							}
							else
							{
							fprintf(wave, "^<%d %s\n", 0, "disabled");
							}
						}


					if(t->vector)
						{
						int ix;
                                                nptr *nodes;
						bptr bits;
						baptr ba;

						if (HasAlias(t)) { fprintf(wave,"+{%s} ", t->name_full); }

						bits = t->n.vec->bits;
						ba = bits ? bits->attribs : NULL;

						fprintf(wave,"%c{%s}", ba ? ':' : '#', 
							t->n.vec->transaction_cache ? t->n.vec->transaction_cache->bvname : t->n.vec->bvname);

						nodes=t->n.vec->bits->nodes;
						for(ix=0;ix<t->n.vec->bits->nnbits;ix++)
							{
							if(nodes[ix]->expansion)
								{
								fprintf(wave," (%d)%s",nodes[ix]->expansion->parentbit, append_array_row(nodes[ix]->expansion->parent));
								}
								else
								{
								fprintf(wave," %s",append_array_row(nodes[ix]));
								}
							if(ba)
								{
								fprintf(wave, " "TTFormat" %x", ba[ix].shift, ba[ix].flags);
								}
							}
						fprintf(wave,"\n");
						}
						else
						{
						if(HasAlias(t))
							{
							if(t->n.nd->expansion)
								{
								fprintf(wave,"+{%s} (%d)%s\n",t->name_full,t->n.nd->expansion->parentbit, append_array_row(t->n.nd->expansion->parent));
								}
								else
								{
								fprintf(wave,"+{%s} %s\n",t->name_full,append_array_row(t->n.nd));
								}
							}
							else
							{
							if(t->n.nd->expansion)
								{
								fprintf(wave,"(%d)%s\n",t->n.nd->expansion->parentbit, append_array_row(t->n.nd->expansion->parent));
								}
								else
								{
								fprintf(wave,"%s\n",append_array_row(t->n.nd));
								}
							}
						}
					}

				st=st->next;
				} /* while(st)... */

			if(GLOBALS->strace_ctx->straces)
				{
				fprintf(wave, "!!\n");	/* mark end of strace region */
				}
		
				swap_strace_contexts();
			}
			else
			{
			struct mprintf_buff_t *mt = GLOBALS->strace_ctx->mprintf_buff_head;

			while(mt)	
				{
				fprintf(wave, "%s", mt->str);
				mt=mt->next;
				}
			}

		} /* if(timearray)... */
	}
}


int read_save_helper(char *wname, char **dumpfile, char **savefile) { 
        FILE *wave;
        char *str = NULL;
        int wave_is_compressed;
	char traces_already_exist = (GLOBALS->traces.first != NULL);
	int rc = -1;
	int extract_dumpfile_savefile_only = (dumpfile != NULL) && (savefile != NULL);

	GLOBALS->is_gtkw_save_file = suffix_check(wname, ".gtkw") || suffix_check(wname, ".gtkw.gz") || suffix_check(wname, ".gtkw.zip");

	if(suffix_check(wname, ".gz") || suffix_check(wname, ".zip"))
                {
                str=wave_alloca(strlen(wname)+5+1);
                strcpy(str,"zcat ");
                strcpy(str+5,wname);
                wave=popen(str,"r");
                wave_is_compressed=~0;
                }
                else
                {   
                wave=fopen(wname,"rb");
                wave_is_compressed=0;
                }


        if(!wave)  
                {  
                fprintf(stderr, "Error opening save file '%s' for reading.\n", wname);
		perror("Why");
		errno=0;
                }
                else
                {
                char *iline;      
		int s_ctx_iter;

		if(extract_dumpfile_savefile_only)
			{
	                while((iline=fgetmalloc(wave)))
	                        {
				if(!strncmp(iline,  "[dumpfile]", 10))
					{
					char *lhq = strchr(iline, '"');
					char *rhq = strrchr(iline, '"');
					if((lhq) && (rhq) && (lhq != rhq)) /* no real need to check rhq != NULL*/
						{
						*rhq = 0;
						if(*dumpfile) free_2(*dumpfile);
						*dumpfile = strdup_2(lhq + 1);
						}
					}
				else
				if(!strncmp(iline,  "[dumpfile_mtime]", 16))
					{
					/* nothing for now */
					}
				else
				if(!strncmp(iline,  "[dumpfile_size]", 15))
					{
					/* nothing for now */
					}
				else
				if(!strncmp(iline,  "[savefile]", 10))
					{
					char *lhq = strchr(iline, '"');
					char *rhq = strrchr(iline, '"');
					if((lhq) && (rhq) && (lhq != rhq)) /* no real need to check rhq != NULL*/
						{
						*rhq = 0;
						if(*savefile) free_2(*savefile);
						*savefile = strdup_2(lhq + 1);
						}
					}

	                        free_2(iline);
				rc++;
	                        }

	                if(wave_is_compressed) pclose(wave); else fclose(wave);
			return(rc);
			}

                WAVE_STRACE_ITERATOR(s_ctx_iter)
                        {
                        GLOBALS->strace_ctx = &GLOBALS->strace_windows[GLOBALS->strace_current_window = s_ctx_iter];
                        GLOBALS->strace_ctx->shadow_encountered_parsewavline = 0;
                        }

		if(GLOBALS->traces.total)
			{
			  GLOBALS->group_depth=0;
			  /*		 AddBlankTrace(NULL); in order to terminate any possible collapsed groups */
			}

		if(GLOBALS->is_lx2)
			{
	                while((iline=fgetmalloc(wave)))
	                        {
	                        parsewavline_lx2(iline, NULL, 0);
	                        free_2(iline);
	                        }

			lx2_import_masked();

			if(wave_is_compressed)
		                {
				pclose(wave);
		                wave=popen(str,"r");
		                }
		                else
		                {   
				fclose(wave);
		                wave=fopen(wname,"rb");
		                }

		        if(!wave)  
		                {  
		                fprintf(stderr, "Error opening save file '%s' for reading.\n", wname);
				perror("Why");
				errno=0;
				return(rc);
		                }
			}

                GLOBALS->default_flags=TR_RJUSTIFY;
		GLOBALS->shift_timebase_default_for_add=LLDescriptor(0);
		GLOBALS->strace_current_window = 0; /* in case there are shadow traces */

		rc = 0;
		GLOBALS->which_t_color = 0;
                while((iline=fgetmalloc(wave)))
                        {
                        parsewavline(iline, NULL, 0);
			GLOBALS->strace_ctx->shadow_encountered_parsewavline |= GLOBALS->strace_ctx->shadow_active;
                        free_2(iline);
			rc++;
                        }
		GLOBALS->which_t_color = 0;

		WAVE_STRACE_ITERATOR(s_ctx_iter)
			{
			GLOBALS->strace_ctx = &GLOBALS->strace_windows[GLOBALS->strace_current_window = s_ctx_iter];

			if(GLOBALS->strace_ctx->shadow_encountered_parsewavline)
				{
				GLOBALS->strace_ctx->shadow_encountered_parsewavline = 0;

				if(GLOBALS->strace_ctx->shadow_straces)
					{
					GLOBALS->strace_ctx->shadow_active = 1;
	
					swap_strace_contexts();
					strace_maketimetrace(1);
					swap_strace_contexts();
	
					GLOBALS->strace_ctx->shadow_active = 0;
					}
				}
			}

                GLOBALS->default_flags=TR_RJUSTIFY;
		GLOBALS->shift_timebase_default_for_add=LLDescriptor(0);
		update_markertime(GLOBALS->tims.marker);
                if(wave_is_compressed) pclose(wave); else fclose(wave);

		if(traces_already_exist) GLOBALS->timestart_from_savefile_valid = 0;

		EnsureGroupsMatch();

		GLOBALS->signalwindow_width_dirty=1;
		MaxSignalLength();
		signalarea_configure_event(GLOBALS->signalarea, NULL);
		wavearea_configure_event(GLOBALS->wavearea, NULL);

#ifdef MAC_INTEGRATION
		if(GLOBALS->num_notebook_pages > 1)
#endif
			{		
			if(!GLOBALS->block_xy_update)
				{
				int x, y;
	
				get_window_size(&x, &y);
				set_window_size(x, y);
				}
			}
                }

	GLOBALS->current_translate_file = 0;

return(rc);
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
 memset(new, 0, len+1); /* scan-build */

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
				/* int made = 0; */ /* scan-build */
				char *w3;
				char *rbrack           = strrchr(w2,   ']');
				char *rightmost_lbrack = strrchr(sp+1, '[');

				if(rbrack && rightmost_lbrack)
					{
					*rbrack = 0;

					w3 = malloc_2(strlen(w2) + 1 + strlen(rightmost_lbrack+1) + 1);
					sprintf(w3, "%s:%s", w2, rightmost_lbrack+1);

					/* made = */ maketraces(w3, alias, 1); /* scan-build */
					free_2(w3);
					}

#if 0
				/* this is overkill for now with possible delay implications so commented out */
				if(!made)
					{
					*lbrack = 0;
					fprintf(stderr, "GTKWAVE | Attempting regex '%s' on missing stranded vector\n", w2);

					w3 = malloc_2(1 + strlen(w2) + 5);
					sprintf(w3, "^%s\\[.*", w2);
					maketraces(w3, alias, 1);
					free_2(w3);
					}
#endif
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
    else if (strcmp (w2, "sst_width") == 0)
      {
      if(!GLOBALS->ignore_savefile_pane_pos)
	{
	/* sst vs rhs of window position.  */
	int x;
	sscanf (w, "%d", &x);
	if(!GLOBALS->block_xy_update)
		{
		if(GLOBALS->toppanedwindow)
			{
#if GTK_CHECK_VERSION(2,4,0)
			gtk_paned_set_position(GTK_PANED(GLOBALS->toppanedwindow), x);
#endif
			}
			else
			{
			GLOBALS->toppanedwindow_size_cache = x;
			}
		}
	}
      }
    else if (strcmp (w2, "signals_width") == 0)
      {
      if(!GLOBALS->ignore_savefile_pane_pos)
	{
	/* signals vs waves panes position.  */
	int x;
	sscanf (w, "%d", &x);
	if(!GLOBALS->block_xy_update)
		{
		if(GLOBALS->panedwindow)
			{
#if GTK_CHECK_VERSION(2,4,0)
			gtk_paned_set_position(GTK_PANED(GLOBALS->panedwindow), x);
#endif
			}
			else
			{
			GLOBALS->panedwindow_size_cache = x;
			}
		}
	}
      }
    else if (strcmp (w2, "sst_expanded") == 0)
      {
      if(!GLOBALS->ignore_savefile_pane_pos)
	{
	/* sst is expanded?  */
	int x;
	sscanf (w, "%d", &x);
	GLOBALS->sst_expanded = (x != 0);
#if GTK_CHECK_VERSION(2,4,0)
	if(!GLOBALS->block_xy_update)
		{
		if(GLOBALS->expanderwindow)
			{
			gtk_expander_set_expanded(GTK_EXPANDER(GLOBALS->expanderwindow), GLOBALS->sst_expanded);
			}
		}
#endif
	}
      }
    else if (strcmp (w2, "sst_vpaned_height") == 0)
      {
      if(!GLOBALS->ignore_savefile_pane_pos)
	{
	/* signals vs waves panes position.  */
	int x;
	sscanf (w, "%d", &x);
	if(!GLOBALS->block_xy_update)
		{
		if(GLOBALS->sst_vpaned)
			{
#if GTK_CHECK_VERSION(2,4,0)
			gtk_paned_set_position(GTK_PANED(GLOBALS->sst_vpaned), x);
#endif
			}
			else
			{
			GLOBALS->vpanedwindow_size_cache = x;
			}
		}
	}
      }
    else if (strcmp (w2, "color") == 0)
      {
      int which_col = 0;
      sscanf (w, "%d", &which_col);
      if((which_col>=0)&&(which_col<=WAVE_NUM_RAINBOW))
		{
		GLOBALS->which_t_color = which_col;
		}
		else
		{
		GLOBALS->which_t_color = 0;
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
    else if (strcmp (w2, "dumpfile") == 0)
	{
        /* nothing here currently...only finder/DnD processes these externally */
        }
    else if (strcmp (w2, "savefile") == 0)
	{
        /* nothing here currently...intended future use of this tag is to determine relative file placements */
        }
    else if (strcmp (w2, "[*]") == 0)
	{
        /* reserved for comment lines */
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

/******************************************************************/

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

/******************************************************************/

/* GetRelativeFilename(), by Rob Fisher.
 * rfisher@iee.org
 * http://come.to/robfisher
 */

#define MAX_FILENAME_LEN PATH_MAX

/* The number of characters at the start of an absolute filename.  e.g. in DOS,
 * absolute filenames start with "X:\" so this value should be 3, in UNIX they start
 * with "\" so this value should be 1.
 */
#if defined _MSC_VER || defined __MINGW32__
#define ABSOLUTE_NAME_START 3
#else
#define ABSOLUTE_NAME_START 1
#endif

/* set this to '\\' for DOS or '/' for UNIX */
#if defined _MSC_VER || defined __MINGW32__
#define SLASH '\\'
#else
#define SLASH '/'
#endif

/* Given the absolute current directory and an absolute file name, returns a relative file name.
 * For example, if the current directory is C:\foo\bar and the filename C:\foo\whee\text.txt is given,
 * GetRelativeFilename will return ..\whee\text.txt.
 */
char* GetRelativeFilename(char *currentDirectory, char *absoluteFilename, int *dotdot_levels)
{
	int afMarker = 0, rfMarker = 0;
	int cdLen = 0, afLen = 0;
	int i = 0;
	int levels = 0;
	static char relativeFilename[MAX_FILENAME_LEN+1];

	*dotdot_levels = 0;

	cdLen = strlen(currentDirectory);
	afLen = strlen(absoluteFilename);
	
	/* make sure the names are not too long or too short */
	if(cdLen > MAX_FILENAME_LEN || cdLen < ABSOLUTE_NAME_START+1 || 
		afLen > MAX_FILENAME_LEN || afLen < ABSOLUTE_NAME_START+1)
	{
		return(NULL);
	}
	
	/* Handle DOS names that are on different drives: */
	if(currentDirectory[0] != absoluteFilename[0])
	{
		/* not on the same drive, so only absolute filename will do */
		strcpy(relativeFilename, absoluteFilename);
		return(relativeFilename);
	}

	/* they are on the same drive, find out how much of the current directory
	 * is in the absolute filename
         */
	i = ABSOLUTE_NAME_START;
	while(i < afLen && i < cdLen && currentDirectory[i] == absoluteFilename[i])
	{
		i++;
	}

	if(i == cdLen && (absoluteFilename[i] == SLASH || absoluteFilename[i-1] == SLASH))
	{
		/* the whole current directory name is in the file name,
		 * so we just trim off the current directory name to get the
		 * current file name.
		 */
		if(absoluteFilename[i] == SLASH)
		{
			/* a directory name might have a trailing slash but a relative
			 * file name should not have a leading one...
			 */
			i++;
		}

		strcpy(relativeFilename, &absoluteFilename[i]);
		return(relativeFilename);
	}


	/* The file is not in a child directory of the current directory, so we
	 * need to step back the appropriate number of parent directories by
	 * using "..\"s.  First find out how many levels deeper we are than the
	 * common directory
	 */
	afMarker = i;
	levels = 1;

	/* count the number of directory levels we have to go up to get to the
	 * common directory
	 */
	while(i < cdLen)
	{
		i++;
		if(currentDirectory[i] == SLASH)
		{
			/* make sure it's not a trailing slash */
			i++;
			if(currentDirectory[i] != '\0')
			{
				levels++;
			}
		}
	}

	/* move the absolute filename marker back to the start of the directory name
	 * that it has stopped in.
	 */
	while(afMarker > 0 && absoluteFilename[afMarker-1] != SLASH)
	{
		afMarker--;
	}

	/* check that the result will not be too long */
	if(levels * 3 + afLen - afMarker > MAX_FILENAME_LEN)
	{
		return(NULL);
	}
	
	/* add the appropriate number of "..\"s. */
	rfMarker = 0;
	*dotdot_levels = levels;
	for(i = 0; i < levels; i++)
	{
		relativeFilename[rfMarker++] = '.';
		relativeFilename[rfMarker++] = '.';
		relativeFilename[rfMarker++] = SLASH;
	}

	/* copy the rest of the filename into the result string */
	strcpy(&relativeFilename[rfMarker], &absoluteFilename[afMarker]);

	return(relativeFilename);
}

/******************************************************************/

char *find_dumpfile(char *orig_save, char *orig_dump, char *this_save)
{
char *synth_nam = NULL;

if(orig_save && orig_dump && this_save)
	{
	char *dup_orig_save = strdup_2(orig_save);
	char *rhs_orig_save_slash = strrchr(dup_orig_save, SLASH);
	char *grf = NULL;
	int dotdot_levels = 0;

	if(rhs_orig_save_slash)
		{
		*rhs_orig_save_slash = 0;
		grf =GetRelativeFilename(dup_orig_save, orig_dump, &dotdot_levels);
		if(grf)
			{
			char *dup_this_save = strdup_2(this_save);
			char *rhs_this_save_slash = strrchr(dup_this_save, SLASH);
			char *p = dup_this_save;
			int levels = 0;	

			if(rhs_this_save_slash)
				{
				*(rhs_this_save_slash+1) = 0;

				while(*p)
					{
					if(*p == SLASH) levels++;
					p++;
					}

				if(levels > dotdot_levels) /* > because we left the ending slash on dup_this_save */
					{
					synth_nam = malloc_2(strlen(dup_this_save) + strlen(grf) + 1);
					strcpy(synth_nam, dup_this_save);
					strcat(synth_nam, grf);
					}
				}
	
			free_2(dup_this_save);
			}
	
		}

	free_2(dup_orig_save);
	}

return(synth_nam);
}

/******************************************************************/

#ifdef MAC_INTEGRATION

/*
 * deliberately kept outside of GLOBALS control
 */
struct finder_file_chain
{
struct finder_file_chain *next;
unsigned queue_warning_presented : 1;
char *name;
};

static struct finder_file_chain *finder_name_integration = NULL;

/*
 * Integration with Finder...
 * cache name and load in later off a timer (similar to caching DnD for quartz...)
 */
gboolean deal_with_finder_open(GtkOSXApplication *app, gchar *path, gpointer user_data)
{
if(!finder_name_integration)
        {
        finder_name_integration = g_malloc(sizeof(struct finder_file_chain));
        finder_name_integration->name = g_strdup(path);
	finder_name_integration->queue_warning_presented = 0;
        finder_name_integration->next = NULL;
        }
        else
        {
        struct finder_file_chain *p = finder_name_integration;
        while(p->next) p = p->next;
        p->next = g_malloc(sizeof(struct finder_file_chain));
	p->next->queue_warning_presented = 0;
        p->next->name = g_strdup(path);
        p->next->next = NULL;
        }

return(TRUE);
}

/*
 * caled in timer routine
 */
gboolean process_finder_names_queued(void)
{
return(finder_name_integration != NULL);
}

char *process_finder_extract_queued_name(void)
{
struct finder_file_chain *lc = finder_name_integration;
while(lc)
	{
	if(!lc->queue_warning_presented)
		{
		lc->queue_warning_presented = 1;
		return(lc->name);
		}

	lc = lc->next;
	}

return(NULL);
}

gboolean process_finder_name_integration(void)
{
struct finder_file_chain *lc = finder_name_integration;
struct finder_file_chain *lc_next;

if(lc)
	{
	finder_name_integration = NULL; /* placed here to avoid race conditions with GLOBALS */

	while(lc)
		{
		char *lcname = lc->name;
		int try_to_load_file = 1;
		int reload_save_file = 0;
		char *dfn = NULL;
		char *sfn = NULL;
		char *fdf = NULL;
		FILE *f;

		if ((suffix_check(lcname, ".sav")) || (suffix_check(lcname, ".gtkw")))
			{
			reload_save_file = 1;
			try_to_load_file = 0;
			read_save_helper(lcname, &dfn, &sfn);

			if(sfn)
				{
				char *old_sfn = sfn;
				sfn = wave_alloca(strlen(sfn)+1); /* as context can change on file load */
				strcpy(sfn, old_sfn);
				free_2(old_sfn);
				}

#if defined __USE_BSD || defined __USE_XOPEN_EXTENDED || defined __CYGWIN__ || defined HAVE_REALPATH
	               	if(dfn && sfn)
              			{
	                        char *can = realpath(lcname, NULL);
	                        char *old_fdf = find_dumpfile(sfn, dfn, can);

	                        free(can);
				fdf = wave_alloca(strlen(old_fdf)+1);
				strcpy(fdf, old_fdf);
				free_2(old_fdf);

	                       	f = fopen(fdf, "rb");
	                        if(f)
	                                {
	                                fclose(f);
	                                lcname = fdf;
					try_to_load_file = 1;
	                               	}
				}
#endif

			if(dfn && !try_to_load_file)
				{
				char *old_dfn = dfn;

				dfn = wave_alloca(strlen(dfn)+1); /* as context can change on file load */
				strcpy(dfn, old_dfn);
				free_2(old_dfn);

				f = fopen(dfn, "rb");
				if(f)
					{
					fclose(f);
					lcname = dfn;
					try_to_load_file = 1;
					}
				}
			}

		if(try_to_load_file)
			{
			int plen = strlen(lcname);
			char *fni = wave_alloca(plen + 32); /* extra space for message */

			sprintf(fni, "Loading %s...", lcname);
			gtk_window_set_title(GTK_WINDOW(GLOBALS->mainwindow), fni);

			strcpy(fni, lcname);

			if(!menu_new_viewer_tab_cleanup_2(fni))
				{
				if(GLOBALS->winname)
					{
					gtk_window_set_title(GTK_WINDOW(GLOBALS->mainwindow), GLOBALS->winname);
					}
				}
			}

		/* now do save file... */
		if(reload_save_file)
			{
			if(GLOBALS->filesel_writesave) { free_2(GLOBALS->filesel_writesave); }
			GLOBALS->filesel_writesave = strdup_2(lc->name);
			read_save_helper(GLOBALS->filesel_writesave, NULL, NULL);
			}

		lc_next = lc->next;
		g_free(lc->name);
		g_free(lc);
		lc = lc_next;
		}
	return(TRUE);
	}

return(FALSE);
}
#endif
