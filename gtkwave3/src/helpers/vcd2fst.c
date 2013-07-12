/*
 * Copyright (c) 2009-2013 Tony Bybell.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#include <config.h>

#if HAVE_GETOPT_H
#include <getopt.h>
#endif

#include "fst/fstapi.h"
#include "../../contrib/rtlbrowse/jrb.h"
#include "wave_locale.h"

#ifdef EXTLOAD_SUFFIX
#ifdef EXTCONV_PATH
#define VCD2FST_EXTLOAD_CONV
#endif
#endif

#ifdef EXT2LOAD_SUFFIX
#ifdef EXT2CONV_PATH
#define VCD2FST_EXT2LOAD_CONV
#endif
#endif

#if defined(VCD2FST_EXTLOAD_CONV) || defined(VCD2FST_EXT2LOAD_CONV)
#define VCD2FST_EXTLOADERS_CONV
#endif

/*********************************************************/
/*** vvv extload component type name determination vvv ***/
/*********************************************************/

#if defined(VCD2FST_EXTLOAD_CONV)

#include <Judy.h>

#ifdef _WAVE_HAVE_JUDY
Pvoid_t  PJArray = NULL;
#else
JRB comp_name_jrb = NULL;
#endif

static const char *fst_scope_name = NULL;

static char *get_scopename(void *xc, FILE *extload)
{
static char sbuff[65537];
char * rc;
#ifdef _WAVE_HAVE_JUDY
PPvoid_t PPValue;
#else
JRB str;
Jval jv;
#endif

for(;;)
        {
        rc = fgets(sbuff, 65536, extload);
        if(!rc)
                {
                return(NULL);
                }

        if(rc[0] == 'S')
                {
                if(!strncmp(rc, "Scope:", 6))
                        {
                        char vht[2048];
                        char cname[2048];
                        char ctype[2048];

			cname[0] = ctype[0] = 0;

                        sscanf(rc+6, "%s %s %s", vht, cname, ctype);
                        fst_scope_name = fstReaderPushScope(xc, cname, NULL);

			/* process fst_scope_name + cname vs ctype here */
			if(strcmp(ctype, "NULL") && strcmp(cname, ctype))
				{
#ifdef _WAVE_HAVE_JUDY
				PPValue = JudySLIns(&PJArray, (uint8_t *)fst_scope_name, PJE0);
				if(!*((char **)PPValue))
					{
					*((char **)PPValue) = strdup(ctype);
					}
#else
				char cstring[65537];
				strcpy(cstring, fst_scope_name);
				str = jrb_find_str(comp_name_jrb, cstring);
				if(!str)
					{
					jv.s = strdup(ctype);
					jrb_insert_str(comp_name_jrb, strdup(cstring), jv);
					}
#endif
				}
                        }
                }
        else
        if(rc[0] == 'U')
                {
                fst_scope_name = fstReaderPopScope(xc);
                }
        }

return(rc);
}


static void iter_scope(char *fname)
{
char sbuff[65537];
FILE *extload;
void *xc = fstReaderOpenForUtilitiesOnly();

sprintf(sbuff, "%s -scope %s 2>&1", EXTLOAD_PATH, fname);
extload = popen(sbuff, "r");

while(get_scopename(xc, extload));

pclose(extload);
fstReaderClose(xc); /* corresponds to fstReaderOpenForUtilitiesOnly() */
}


static void dealloc_scope(void)
{
#ifdef _WAVE_HAVE_JUDY
PPvoid_t PPValue;

if(PJArray)
        {
        char Index[65537];
	Index[0] = 0;

        for (PPValue  = JudySLFirst (PJArray, (uint8_t *)Index, PJE0);
                 PPValue != (PPvoid_t) NULL;
                 PPValue  = JudySLNext  (PJArray, (uint8_t *)Index, PJE0))
            {
		free(*(char **)PPValue);
            }

        JudySLFreeArray(&PJArray, PJE0);
        PJArray = NULL;
        }
#else
if(comp_name_jrb)
	{
        JRB node;
	char *Index;

        jrb_traverse(node, comp_name_jrb)
            {
                Index = node->key.s;
                free(Index);
                Index = node->val.s;
                free(Index);
            }

        jrb_free_tree(comp_name_jrb);
        comp_name_jrb = NULL;
	}
#endif
}

#endif

/*********************************************************/
/*** ^^^ extload component type name determination ^^^ ***/
/*********************************************************/

size_t getline_replace(char **wbuf, char **buf, size_t *len, FILE *f)
{
char *fgets_rc;

if(!*wbuf)
	{
	*wbuf = malloc(32768);
	*len = 32767;
	}

(*wbuf)[0] = 0;
fgets_rc = fgets(*wbuf, 32767, f);
*buf = *wbuf;
while(*(buf)[0]==' ') { (*buf)++; } /* verilator leading spaces fix */

if((!(*buf)[0])||(!fgets_rc))
	{
	return(-1);
	}
	else
	{
	return(1);
	}
}

JRB vcd_ids = NULL;

static unsigned int vcdid_hash(char *s, int len)
{
unsigned int val=0;
int i;

s += len;

for(i=0;i<len;i++)
        {
	val *= 94;
        val += ((unsigned char)*(--s)) - 32;
        }

return(val);
}  

int pack_type = 0;  /* set to 1 for fastlz */
int repack_all = 0; /* 0 is normal, 1 does the repack (via fstapi) at end */
int parallel_mode = 0; /* 0 is is single threaded, 1 is multi-threaded */


static int suffix_check(const char *s, const char *sfx)
{
int sfxlen = strlen(sfx);
return((strlen(s)>=sfxlen)&&(!strcasecmp(s+strlen(s)-sfxlen,sfx)));
}


int fst_main(char *vname, char *fstname)
{
FILE *f;
char *buf = NULL, *wbuf = NULL;
size_t glen;
void *ctx;
int line = 0;
size_t ss;
fstHandle returnedhandle;
JRB node;
uint64_t prev_tim = 0;
char bin_fixbuff[32769];
int hash_kill = 0;
unsigned int hash_max = 0;
int *node_len_array = NULL;
int is_popen = 0;
#ifdef VCD2FST_EXTLOAD_CONV
int is_extload = 0;
void *xc = NULL;
#endif

if(!strcmp("-", vname))
	{
	f = stdin;
	}
	else
	{
#ifdef VCD2FST_EXTLOAD_CONV
	if(suffix_check(vname, "."EXTLOAD_SUFFIX))
		{
		sprintf(bin_fixbuff, EXTCONV_PATH" %s", vname);
		f = popen(bin_fixbuff, "r");
		is_popen = 1;
		is_extload = 1;
#ifndef _WAVE_HAVE_JUDY
		comp_name_jrb = make_jrb();
#endif
		iter_scope(vname);
		}
		else
#endif
		{
#ifdef VCD2FST_EXT2LOAD_CONV
		if(suffix_check(vname, "."EXT2LOAD_SUFFIX))
			{
			sprintf(bin_fixbuff, EXT2CONV_PATH" %s", vname);
			f = popen(bin_fixbuff, "r");
			is_popen = 1;
			}
			else
#endif
			{
			f = fopen(vname, "rb");
			}
		}
	}

if(!f)
	{
	printf("could not open '%s', exiting.\n", vname);
	exit(255);
	}

ctx = fstWriterCreate(fstname, 1);

if(!ctx)
	{
	printf("could not open '%s', exiting.\n", fstname);
	exit(255);
	}

#if defined(VCD2FST_EXTLOAD_CONV)
if(is_popen && is_extload)
	{
	xc = fstReaderOpenForUtilitiesOnly();
	}
#endif

vcd_ids = make_jrb();
fstWriterSetPackType(ctx, pack_type);
fstWriterSetRepackOnClose(ctx, repack_all);
fstWriterSetParallelMode(ctx, parallel_mode);

while(!feof(f))
	{
	ss = getline_replace(&wbuf, &buf, &glen, f);
	if(ss == -1)
		{
		break;
		}
	line++;

	if(!strncmp(buf, "$var", 4))
		{
		char *st = strtok(buf+5, " \t");
		enum fstVarType vartype;
		int len;
		char *nam;
		unsigned int hash;

		vartype = FST_VT_VCD_WIRE;
		switch(st[0])
			{
			case 'w':
				if(!strcmp(st, "wire"))
					{
					}
				else
				if(!strcmp(st, "wand"))
					{
					vartype = FST_VT_VCD_WAND;
					}
				else
				if(!strcmp(st, "wor"))
					{
					vartype = FST_VT_VCD_WOR;
					}
				break;

			case 'r':
				if(!strcmp(st, "reg"))
					{
					vartype = FST_VT_VCD_REG;
					}
				else
				if(!strcmp(st, "real"))
					{
					vartype = FST_VT_VCD_REAL;
					}
				else
				if(!strcmp(st, "real_parameter"))
					{
					vartype = FST_VT_VCD_REAL_PARAMETER;
					}
				else
				if(!strcmp(st, "realtime"))
					{
					vartype = FST_VT_VCD_REALTIME;
					}
				break;
				
			case 'p':
				if(!strcmp(st, "parameter"))
					{
					vartype = FST_VT_VCD_PARAMETER;
					}
				else
				if(!strcmp(st, "port"))
					{
					vartype = FST_VT_VCD_PORT;
					}
				break;

			case 'i':
				vartype = FST_VT_VCD_INTEGER;
				break;

			case 'e':
				vartype = FST_VT_VCD_EVENT;
				break;

			case 's':
				if(!strcmp(st, "supply1"))		
					{
					vartype = FST_VT_VCD_SUPPLY1;
					}
				else
				if(!strcmp(st, "supply0"))		
					{
					vartype = FST_VT_VCD_SUPPLY0;
					}
				else
				if(!strcmp(st, "string"))		
					{
					vartype = FST_VT_GEN_STRING;
					}
				break;

			case 't':
				if(!strcmp(st, "time"))
					{
					vartype = FST_VT_VCD_TIME;
					}
				else
				if(!strcmp(st, "tri"))
					{
					vartype = FST_VT_VCD_TRI;
					}
				else
				if(!strcmp(st, "triand"))
					{
					vartype = FST_VT_VCD_TRIAND;
					}
				else
				if(!strcmp(st, "trior"))
					{
					vartype = FST_VT_VCD_TRIOR;
					}
				else
				if(!strcmp(st, "trireg"))
					{
					vartype = FST_VT_VCD_TRIREG;
					}
				else
				if(!strcmp(st, "tri0"))
					{
					vartype = FST_VT_VCD_TRI0;
					}
				else
				if(!strcmp(st, "tri1"))
					{
					vartype = FST_VT_VCD_TRI1;
					}
				break;

			default:
				break;			
			}

		st = strtok(NULL, " \t");
		len = atoi(st);
		switch(vartype)
			{
			case FST_VT_VCD_PORT: len = (len * 3) + 2; break;
			case FST_VT_GEN_STRING: len = 0; break;
			case FST_VT_VCD_EVENT: len = (len != 0) ? len : 1;  break;
			default: 
				if(len == 0) { len = 1; }
				break;
			}

		st = strtok(NULL, " \t"); /* vcdid */
		hash = vcdid_hash(st, strlen(st));

		if(hash == (hash_max+1))
			{
			hash_max = hash;
			}
		else
		if((hash>0)&&(hash<=hash_max))
			{
			/* general case with aliases */
			}
		else
			{
			hash_kill = 1;
			}

		nam = strtok(NULL, " \t"); /* name */
		st = strtok(NULL, " \t"); /* $end */

		if(st)
			{
			if(strncmp(st, "$end", 4))
				{
				*(st-1) = ' ';
				}
	
			node = jrb_find_int(vcd_ids, hash);
			if(!node)
				{
				Jval val;
				returnedhandle = fstWriterCreateVar(ctx, vartype, FST_VD_IMPLICIT, len, nam, 0);
				val.i = returnedhandle;
				jrb_insert_int(vcd_ids, hash, val)->val2.i = len;
				}
				else
				{
				fstWriterCreateVar(ctx, vartype, FST_VD_IMPLICIT, node->val2.i, nam, node->val.i);
				}
			}
		}
	else
	if(!strncmp(buf, "$scope", 6))
		{
		char *st = strtok(buf+6, " \t");
		enum fstScopeType scopetype;

		if(!strcmp(st, "module"))
			{
			scopetype = FST_ST_VCD_MODULE;
			}
		else
		if(!strcmp(st, "task"))
			{
			scopetype = FST_ST_VCD_TASK;
			}
		else
		if(!strcmp(st, "function"))
			{
			scopetype = FST_ST_VCD_FUNCTION;
			}
		else
		if(!strcmp(st, "begin"))
			{
			scopetype = FST_ST_VCD_BEGIN;
			}
		else
		if(!strcmp(st, "fork"))
			{
			scopetype = FST_ST_VCD_FORK;
			}
		else
		if(!strcmp(st, "generate"))
			{
			scopetype = FST_ST_VCD_GENERATE;
			}
		else
			{
			scopetype = FST_ST_VCD_MODULE;
			}

		st = strtok(NULL, " \t");

#if defined(VCD2FST_EXTLOAD_CONV)
#ifdef _WAVE_HAVE_JUDY
		if(PJArray)
			{
			const char *fst_scope_name = fstReaderPushScope(xc, st, NULL);
			PPvoid_t PPValue = JudySLGet(PJArray, (uint8_t *)fst_scope_name, PJE0);

			fstWriterSetScope(ctx, scopetype, st, PPValue ? *PPValue : NULL);
			}
#else
		if(comp_name_jrb)
			{
			const char *fst_scope_name = fstReaderPushScope(xc, st, NULL);
			char cstring[65537];
			JRB str;

			strcpy(cstring, fst_scope_name);
			str = jrb_find_str(comp_name_jrb, cstring);

			fstWriterSetScope(ctx, scopetype, st, str ? str->val.s : NULL);
			}
#endif
			else
#endif
			{
			fstWriterSetScope(ctx, scopetype, st, NULL);
			}
		}
	else
	if(!strncmp(buf, "$upscope", 8))
		{
		fstWriterSetUpscope(ctx);
#if defined(VCD2FST_EXTLOAD_CONV)
		if(xc)
			{
			fstReaderPopScope(xc);
			}
#endif
		}
	else
	if(!strncmp(buf, "$endd", 5))
		{
#if defined(VCD2FST_EXTLOAD_CONV)
#ifdef _WAVE_HAVE_JUDY
		if(PJArray)
#else
		if(comp_name_jrb)
#endif
			{
			dealloc_scope();
			}
#endif
		break;
		}
	else
	if(!strncmp(buf, "$timezero", 9))
		{
		char *pnt;
		uint64_t tzero = 0;

		if((pnt = strstr(buf, "$end")))
			{
			*pnt = 0;
			sscanf(buf+10, "%"SCNu64, &tzero);
			}
		else
                        {
                        ss = getline_replace(&wbuf, &buf, &glen, f);
                        if(ss == -1)
                                {
                                break;
                                }
                        line++;             
			sscanf(buf, "%"SCNu64, &tzero);
                        }

		fstWriterSetTimezero(ctx, tzero);
		}
	else
	if(!strncmp(buf, "$timescale", 10))
		{
		char *pnt;
		char *num = NULL;
		int exp = -9;
		int tv = 1;

		if((pnt = strstr(buf, "$end")))
			{
			*pnt = 0;
			num = strchr(buf, '1');
			}

		if(!num)
			{
			ss = getline_replace(&wbuf, &buf, &glen, f);
			if(ss == -1)
				{
				break;
				}
			line++;
			num = buf;
			}

		pnt = num;
		while(*pnt)
			{
			int mat = 0;
			switch(*pnt)
				{
				case 'm': exp = -3; mat = 1; break;
				case 'u': exp = -6; mat = 1; break;
				case 'n': exp = -9; mat = 1; break;
				case 'p': exp = -12; mat = 1; break;
				case 'f': exp = -15; mat = 1; break;
				case 's': exp = -0; mat = 1; break;
				default: break;
				}
			if(mat) break;
			pnt++;
			}

		tv = atoi(num);
		if(tv == 10)
			{
			exp++;
			}
		else
		if(tv == 100)
			{
			exp+=2;
			}

		fstWriterSetTimescale(ctx, exp);
		}
	else
	if(!strncmp(buf, "$date", 5))
		{
		char *pnt, *rsp;
		int found = 0;

		if((pnt = strstr(buf, "$end")))
			{
                        *pnt = 0;
			pnt = buf + 5;
			while(*pnt && ((*pnt)==' ')) { pnt++; }
			while((rsp = strrchr(pnt, ' ')))
				{
				if(*(rsp+1) == 0)
					{
					*rsp = 0;
					}
					else
					{
					break;
					}					
				}
			if(strlen(pnt)) { found = 1; }
			}
		else
			{
			pnt = buf + 5;
			while(*pnt && ((*pnt)==' ')) { pnt++; }
			while((rsp = strrchr(pnt, ' ')))
				{
				if(*(rsp+1) == 0)
					{
					*rsp = 0;
					}
					else
					{
					break;
					}					
				}
			if(strlen(pnt) > 3) { found = 1; }
			}
		
		if(!found)		
			{
			ss = getline_replace(&wbuf, &buf, &glen, f);
			if(ss == -1)
				{
				break;
				}
			line++;
			pnt = buf;
			}

		while(*pnt == '\t') pnt++;
		fstWriterSetDate(ctx, pnt);
		}
	else
	if(!strncmp(buf, "$version", 8))
		{
		char *pnt, *crpnt, *rsp;

                if((pnt = strstr(buf, "$end")))
                        {
                        *pnt = 0;
			pnt = buf+8;
			while(*pnt && ((*pnt)==' ')) { pnt++; }
			while((rsp = strrchr(pnt, ' ')))
				{
				if(*(rsp+1) == 0)
					{
					*rsp = 0;
					}
					else
					{
					break;
					}					
				}
                        }           
			else
			{
			ss = getline_replace(&wbuf, &buf, &glen, f);
			if(ss == -1)
				{
				break;
				}
			line++;
			pnt = buf;
			}

		while(*pnt == '\t') pnt++;
		crpnt = strchr(pnt, '\n');
		if(crpnt) *crpnt = 0;
		crpnt = strchr(pnt, '\r');
		if(crpnt) *crpnt = 0;
		fstWriterSetVersion(ctx, pnt);
		}
	}

if((!hash_kill) && (vcd_ids))
	{
	unsigned int hash;

	node_len_array = calloc(hash_max + 1, sizeof(int));

	for(hash=1;hash<=hash_max;hash++)
		{
		node = jrb_find_int(vcd_ids, hash);
		if(node)
			{
			node_len_array[hash] = node->val2.i;
			}
			else
			{
			node_len_array[hash] = 1; /* should never happen */
			}
		}

	jrb_free_tree(vcd_ids);
	vcd_ids = NULL;
	}
	else
	{
	hash_kill = 1; /* scan-build */
	}

while(!feof(f))
	{
	unsigned int hash;
	uint64_t tim;
	size_t len;
	char *nl, *sp;
	double doub;

	ss = getline_replace(&wbuf, &buf, &len, f);
	if(ss == -1)
		{
		break;
		}

	nl = buf;
	while(*nl)
		{
		if((*nl == '\n') || (*nl == '\r'))
			{
			*nl = 0;
			break;
			}
		nl++;
		}

	switch(buf[0])
		{
		case '0':
		case '1':
		case 'x':
		case 'z':
			hash = vcdid_hash(buf+1, nl - (buf+1));
			if(!hash_kill)
				{
				fstWriterEmitValueChange(ctx, hash, buf);
				}
				else
				{
				node = jrb_find_int(vcd_ids, hash);
				if(node)
					{
					fstWriterEmitValueChange(ctx, node->val.i, buf);
					}
					else
					{
					}
				}
			break;

		case 'b':
			sp = strchr(buf, ' ');
			if(!sp) break;
			*sp = 0;
			hash = vcdid_hash(sp+1, nl - (sp+1));
			if(!hash_kill)
				{
				int bin_len = sp - (buf + 1); /* strlen(buf+1) */
				int node_len = node_len_array[hash];

				if(bin_len == node_len)
					{
					fstWriterEmitValueChange(ctx, hash, buf+1);
					}
					else
					{
					int delta = node_len - bin_len;
					memset(bin_fixbuff, buf[1] != '1' ? buf[1] : '0', delta);
					memcpy(bin_fixbuff + delta, buf+1, bin_len);
					fstWriterEmitValueChange(ctx, hash, bin_fixbuff);
					}
				}
				else
				{
				node = jrb_find_int(vcd_ids, hash);
				if(node)
					{
					int bin_len = sp - (buf + 1); /* strlen(buf+1) */
					int node_len = node->val2.i;
					if(bin_len == node_len)
						{
						fstWriterEmitValueChange(ctx, node->val.i, buf+1);
						}
						else
						{
						int delta = node_len - bin_len;
						memset(bin_fixbuff, buf[1] != '1' ? buf[1] : '0', delta);
						memcpy(bin_fixbuff + delta, buf+1, bin_len);
						fstWriterEmitValueChange(ctx, node->val.i, bin_fixbuff);
						}
					}
					else
					{
					}
				}
			break;

		case 's':
			sp = strchr(buf, ' ');
			if(!sp) break;
			*sp = 0;
			hash = vcdid_hash(sp+1, nl - (sp+1));
			if(!hash_kill)
				{
				int bin_len = sp - (buf + 1); /* strlen(buf+1) */

				bin_len = fstUtilityEscToBin(NULL, (unsigned char *)(buf+1), bin_len);
				fstWriterEmitVariableLengthValueChange(ctx, hash, buf+1, bin_len);
				}
				else
				{
				node = jrb_find_int(vcd_ids, hash);
				if(node)
					{
					int bin_len = sp - (buf + 1); /* strlen(buf+1) */

					bin_len = fstUtilityEscToBin(NULL, (unsigned char *)(buf+1), bin_len);
					fstWriterEmitVariableLengthValueChange(ctx, node->val.i, buf+1, bin_len);
					}
					else
					{
					}
				}
			break;

		case 'p':
			{
			char *src = buf+1;
			char *pnt = bin_fixbuff;
			int pchar = 0;

			for(;;)
				{
				if((*src == '\n') || (*src == '\r')) break;
				if(isspace((int)(unsigned char)*src))
					{
					if(pchar != ' ') { *(pnt++) = pchar = ' '; }
					src++;
					continue;
					}
				*(pnt++) = pchar = *(src++);
				}
			*pnt = 0;
			
			sp = strchr(bin_fixbuff, ' ');
			if(!sp) break;
			sp = strchr(sp+1, ' ');
			if(!sp) break;
			sp = strchr(sp+1, ' ');
			if(!sp) break;
			*sp = 0;
			hash = vcdid_hash(sp+1, nl - (sp+1));
			if(!hash_kill)
				{
				fstWriterEmitValueChange(ctx, hash, bin_fixbuff);
				}
				else
				{
				node = jrb_find_int(vcd_ids, hash);
				if(node)
					{
					fstWriterEmitValueChange(ctx, node->val.i, bin_fixbuff);
					}
					else
					{
					}
				}
			}
			break;

		case 'r':
			sp = strchr(buf, ' ');
			if(!sp) break;
			hash = vcdid_hash(sp+1, nl - (sp+1));
			if(!hash_kill)
				{
		                sscanf(buf+1,"%lg",&doub); 
				fstWriterEmitValueChange(ctx, hash, &doub);
				}
				else
				{
				node = jrb_find_int(vcd_ids, hash);
				if(node)
					{
			                sscanf(buf+1,"%lg",&doub); 
					fstWriterEmitValueChange(ctx, node->val.i, &doub);
					}
					else
					{
					}
				}
			break;

		case 'h': /* same as 01xz above but moved down here as it's less common */
		case 'u':
		case 'w':
		case 'l':
		case '-':
			hash = vcdid_hash(buf+1, nl - (buf+1));
			if(!hash_kill)
				{
				fstWriterEmitValueChange(ctx, hash, buf);
				}
				else
				{
				node = jrb_find_int(vcd_ids, hash);
				if(node)
					{
					fstWriterEmitValueChange(ctx, node->val.i, buf);
					}
					else
					{
					}
				}
			break;

		case '#':
			sscanf(buf+1, "%"SCNu64, &tim);
			if((tim >= prev_tim)||(!prev_tim))
				{
				prev_tim = tim;
				fstWriterEmitTimeChange(ctx, tim);
				}
			break;

		default:
			if(!strncmp(buf, "$dumpon", 7))
				{
				fstWriterEmitDumpActive(ctx, 1);
				}
			else
			if(!strncmp(buf, "$dumpoff", 8))
				{
				fstWriterEmitDumpActive(ctx, 0);
				}
			else
			if(!strncmp(buf, "$dumpvars", 9))
				{
				/* nothing */
				}
			else
				{
				/* printf("FST '%s'\n", buf); */
				}
			break;
		}
	}

fstWriterClose(ctx);

#if defined(VCD2FST_EXTLOAD_CONV)
if(xc)
	{
	fstReaderClose(xc);
	}
#endif


if(vcd_ids)
	{
	jrb_free_tree(vcd_ids);
	vcd_ids = NULL;
	}

free(wbuf); wbuf = NULL;
free(node_len_array); node_len_array = NULL;

if(f != stdin) 
	{ 
	if(is_popen)
		{
		pclose(f);
		}
		else
		{
		fclose(f);
		}
	}

return(0);
}


void print_help(char *nam)
{
#ifdef VCD2FST_EXTLOADERS_CONV
#if defined(VCD2FST_EXTLOAD_CONV) && defined(VCD2FST_EXT2LOAD_CONV)

int slen = strlen(EXTLOAD_SUFFIX) + 1 + strlen(EXT2LOAD_SUFFIX);
char *ucase_ext = calloc(1, slen+1);
int i;

sprintf(ucase_ext, "%s/%s", EXTLOAD_SUFFIX, EXT2LOAD_SUFFIX);

for(i=0;i<slen;i++)
	{
	ucase_ext[i] = toupper(ucase_ext[i]);
	}

#else
#ifdef VCD2FST_EXTLOAD_CONV
int slen = strlen(EXTLOAD_SUFFIX);
char *ucase_ext = calloc(1, slen+1);
int i;

for(i=0;i<slen;i++)
	{
	ucase_ext[i] = toupper(EXTLOAD_SUFFIX[i]);
	}
#endif
#ifdef VCD2FST_EXT2LOAD_CONV
int slen = strlen(EXT2LOAD_SUFFIX);
char *ucase_ext = calloc(1, slen+1);
int i;

for(i=0;i<slen;i++)
	{
	ucase_ext[i] = toupper(EXT2LOAD_SUFFIX[i]);
	}
#endif
#endif

#endif






#ifdef __linux__ 
printf(
"Usage: %s [OPTION]... [VCDFILE] [FSTFILE]\n\n"
#ifdef VCD2FST_EXTLOADERS_CONV
"  -v, --vcdname=FILE         specify VCD/%s input filename\n"
#else
"  -v, --vcdname=FILE         specify VCD input filename\n"
#endif
"  -f, --fstname=FILE         specify FST output filename\n"
"  -F, --fastpack             use fastlz algorithm for speed\n"
"  -c, --compress             compress entire file on close\n"
"  -p, --parallel             enable parallel mode\n"
"  -h, --help                 display this help then exit\n\n"

"Note that VCDFILE and FSTFILE are optional provided the\n"
"--vcdname and --fstname options are specified.\n\n"
"Report bugs to <"PACKAGE_BUGREPORT">.\n",nam
#ifdef VCD2FST_EXTLOADERS_CONV
,ucase_ext
#endif
);
#else
printf(
"Usage: %s [OPTION]... [VCDFILE] [FSTFILE]\n\n"
#ifdef VCD2FST_EXTLOADERS_CONV
"  -v FILE                    specify VCD/%s input filename\n"
#else
"  -v FILE                    specify VCD input filename\n"
#endif
"  -f FILE                    specify FST output filename\n"
"  -F                         use fastlz algorithm for speed\n"
"  -c                         compress entire file on close\n"
"  -p                         enable parallel mode\n"
"  -h                         display this help then exit\n\n"

"Note that VCDFILE and FSTFILE are optional provided the\n"
"--vcdname and --fstname options are specified.\n\n"
"Report bugs to <"PACKAGE_BUGREPORT">.\n",nam
#ifdef VCD2FST_EXTLOADERS_CONV
,ucase_ext
#endif
);
#endif

#ifdef VCD2FST_EXTLOADERS_CONV
free(ucase_ext);
#endif

exit(0);
}


int main(int argc, char **argv)
{
char opt_errors_encountered=0;
char *vname=NULL, *lxname=NULL;
int c;

WAVE_LOCALE_FIX

while (1)
        {
#ifdef __linux__
        int option_index = 0;
                        
        static struct option long_options[] =
                {
		{"vcdname", 1, 0, 'v'},
		{"fstname", 1, 0, 'f'},
		{"fastpack", 0, 0, 'F'},
		{"compress", 0, 0, 'c'},
		{"parallel", 0, 0, 'p'},
                {"help", 0, 0, 'h'},
                {0, 0, 0, 0}  
                };
                
        c = getopt_long (argc, argv, "v:f:Fcph", long_options, &option_index);
#else
        c = getopt      (argc, argv, "v:f:Fcph");
#endif
                        
        if (c == -1) break;     /* no more args */
                        
        switch (c)
                {
		case 'v':
			if(vname) free(vname);
                        vname = malloc(strlen(optarg)+1);
                        strcpy(vname, optarg);
			break;

		case 'f':
			if(lxname) free(lxname);
                        lxname = malloc(strlen(optarg)+1);
                        strcpy(lxname, optarg);
			break;

		case 'F':
			pack_type = 1;
			break;

		case 'c':
			repack_all = 1;
			break;

		case 'p':
			parallel_mode = 1;
			break;

                case 'h':
			print_help(argv[0]);
                        break;
                        
                case '?':
                        opt_errors_encountered=1;
                        break;
                        
                default:
                        /* unreachable */
                        break;
                }
        }
                        
if(opt_errors_encountered)
        {
        print_help(argv[0]);
        }

if (optind < argc)
        {               
        while (optind < argc)
                {
                if(!vname)
                        {
                        vname = malloc(strlen(argv[optind])+1);
                        strcpy(vname, argv[optind++]);
                        } 
                else if(!lxname)
                        {
                        lxname = malloc(strlen(argv[optind])+1);
                        strcpy(lxname, argv[optind++]);
                        }
		else
			{
			break;
			}
                }
        }
                        
if((!vname)||(!lxname))
        {
        print_help(argv[0]);
        }

fst_main(vname, lxname);

free(vname); free(lxname);

return(0);
}

