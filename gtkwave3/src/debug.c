/* 
 * Copyright (c) Tony Bybell 1999-2012
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */


/*
 * debug.c 01feb99ajb
 * malloc debugs added on 13jul99ajb
 * malloc tracking added on 05aug07ajb for 3.1 series
 */
#include <config.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "globals.h"
#include "debug.h"
#ifdef _WAVE_HAVE_JUDY
#include <Judy.h>
#endif

#undef free_2
#undef malloc_2
#undef realloc_2
#undef calloc_2

#ifdef _WAVE_HAVE_JUDY
void free_outstanding(void)
{
Pvoid_t  PJArray = (Pvoid_t)GLOBALS->alloc2_chain;
int rcValue;
Word_t Index;
#ifdef DEBUG_PRINTF
int ctr = 0;

printf("\n*** cleanup ***\n");
printf("Freeing %d chunks\n", GLOBALS->outstanding);
system("date");
#endif

if(GLOBALS->s_selected)
	{
	destroy_s_selected();
	}

Index = 0;
for (rcValue = Judy1First(PJArray, &Index, PJE0); rcValue != 0; rcValue = Judy1Next(PJArray, &Index, PJE0))
	{
	free((void *)Index);
#ifdef DEBUG_PRINTF
	ctr++;
#endif
	}
Judy1FreeArray(&PJArray, PJE0);

GLOBALS->alloc2_chain = NULL;
GLOBALS->outstanding = 0;

#ifdef DEBUG_PRINTF
printf("Freed %d chunks\n", ctr);
system("date");
#endif
}
#else
void free_outstanding(void)
{
void **t = (void **)GLOBALS->alloc2_chain;
void **t2;
int ctr = 0;

#ifdef DEBUG_PRINTF
printf("\n*** cleanup ***\n");
printf("Freeing %d chunks\n", GLOBALS->outstanding);
system("date");
#endif

while(t)
	{
	t2 = (void **) *(t+1);
	free(t);
	t = t2;
	ctr++;
	}

GLOBALS->alloc2_chain = NULL;
GLOBALS->outstanding = 0;

#ifdef DEBUG_PRINTF
printf("Freed %d chunks\n", ctr);
system("date");
#endif
}
#endif

/*
 * wrapped malloc family...
 */
#ifdef _WAVE_HAVE_JUDY
void *malloc_2(size_t size
#ifdef DEBUG_MALLOC_LINES
, char *filename, int lineno
#endif
)
{
void *ret;

ret=malloc(size);
if(ret)  
        {
	Judy1Set ((Pvoid_t)&GLOBALS->alloc2_chain, (Word_t)ret, PJE0);

        GLOBALS->outstanding++;
        
        return(ret);
        }   
        else
        {
#ifdef DEBUG_MALLOC_LINES
        fprintf(stderr, "FATAL ERROR: malloc_2() Out of memory, sorry. ['%s', %d]\n", filename, lineno);
#else
        fprintf(stderr, "FATAL ERROR : malloc_2() Out of memory, sorry.\n");
#endif
        exit(1);
        }
}
#else
void *malloc_2(size_t size
#ifdef DEBUG_MALLOC_LINES
, char *filename, int lineno
#endif
)
{
void *ret;

ret=malloc(size + 2*sizeof(void *));
if(ret)
	{
	void **ret2 = (void **)ret;
	*(ret2+0) = NULL;
	*(ret2+1) = GLOBALS->alloc2_chain;
	if(GLOBALS->alloc2_chain)
		{
		*(GLOBALS->alloc2_chain+0) = ret2;
		}
	GLOBALS->alloc2_chain = ret2;

	GLOBALS->outstanding++;

	return((char *)ret + 2*sizeof(void *));
	}
	else
	{
#ifdef DEBUG_MALLOC_LINES
        fprintf(stderr, "FATAL ERROR: malloc_2() Out of memory, sorry. ['%s', %d]\n", filename, lineno);
#else
	fprintf(stderr, "FATAL ERROR : malloc_2() Out of memory, sorry.\n");
#endif
	exit(1);
	}
}
#endif

#ifdef _WAVE_HAVE_JUDY
void *realloc_2(void *ptr, size_t size
#ifdef DEBUG_MALLOC_LINES
, char *filename, int lineno
#endif
)
{
void *ret=realloc(ptr, size);

if(ret)
        {
	if(ptr != ret)
		{
		Judy1Unset ((Pvoid_t)&GLOBALS->alloc2_chain, (Word_t)ptr, PJE0);
		Judy1Set ((Pvoid_t)&GLOBALS->alloc2_chain, (Word_t)ret, PJE0);
		}

        return(ret);
        }
        else
        {
#ifdef DEBUG_MALLOC_LINES
        fprintf(stderr, "FATAL ERROR: realloc_2() Out of memory, sorry. ['%s', %d]\n", filename, lineno);
#else
        fprintf(stderr, "FATAL ERROR : realloc_2() Out of memory, sorry.\n");
#endif
        exit(1);
        }
}
#else
void *realloc_2(void *ptr, size_t size
#ifdef DEBUG_MALLOC_LINES
, char *filename, int lineno
#endif
)
{
void *ret;

void **ret2 = ((void **)ptr) - 2;
void **prv = (void **)*(ret2+0);
void **nxt = (void **)*(ret2+1);
                 
if(prv)
	{
        *(prv+1) = nxt;
        }
        else
        {
        GLOBALS->alloc2_chain = nxt;
        }
        
if(nxt)
	{
        *(nxt+0) = prv;
        }

ret=realloc((char *)ptr - 2*sizeof(void *), size + 2*sizeof(void *));

ret2 = (void **)ret;
*(ret2+0) = NULL;
*(ret2+1) = GLOBALS->alloc2_chain;
if(GLOBALS->alloc2_chain)
	{
	*(GLOBALS->alloc2_chain+0) = ret2;
	}
GLOBALS->alloc2_chain = ret2;

if(ret)
	{
	return((char *)ret + 2*sizeof(void *));
	}
	else
	{
#ifdef DEBUG_MALLOC_LINES
        fprintf(stderr, "FATAL ERROR: realloc_2() Out of memory, sorry. ['%s', %d]\n", filename, lineno);
#else
	fprintf(stderr, "FATAL ERROR : realloc_2() Out of memory, sorry.\n");
#endif
	exit(1);
	}
}
#endif

#ifdef _WAVE_HAVE_JUDY
void *calloc_2_into_context(struct Global *g, size_t nmemb, size_t size
#ifdef DEBUG_MALLOC_LINES
, char *filename, int lineno
#endif
)
{
void *ret;

ret=calloc(nmemb, size);
if(ret)
	{
	Judy1Set ((Pvoid_t)&g->alloc2_chain, (Word_t)ret, PJE0);

	g->outstanding++;

	return(ret);
	}
	else
	{
#ifdef DEBUG_MALLOC_LINES
	fprintf(stderr, "FATAL ERROR: calloc_2() Out of memory, sorry. ['%s', %d]\n", filename, lineno);
#else
	fprintf(stderr, "FATAL ERROR: calloc_2() Out of memory, sorry.\n");
#endif
	exit(1);
	}
}
#else
void *calloc_2_into_context(struct Global *g, size_t nmemb, size_t size
#ifdef DEBUG_MALLOC_LINES
, char *filename, int lineno
#endif
)
{
void *ret;

ret=calloc(1, (nmemb * size) + 2*sizeof(void *));
if(ret)
	{
	void **ret2 = (void **)ret;
	*(ret2+0) = NULL;
	*(ret2+1) = g->alloc2_chain;
	if(g->alloc2_chain)
		{
		*(g->alloc2_chain+0) = ret2;
		}
	g->alloc2_chain = ret2;

	g->outstanding++;

	return((char *)ret + 2*sizeof(void *));
	}
	else
	{
#ifdef DEBUG_MALLOC_LINES
        fprintf(stderr, "FATAL ERROR: calloc_2() Out of memory, sorry. ['%s', %d]\n", filename, lineno);
#else
	fprintf(stderr, "FATAL ERROR: calloc_2() Out of memory, sorry.\n");
#endif
	exit(1);
	}
}
#endif

void *calloc_2(size_t nmemb, size_t size
#ifdef DEBUG_MALLOC_LINES
, char *filename, int lineno
#endif
)
{
return(calloc_2_into_context(GLOBALS, nmemb, size
#ifdef DEBUG_MALLOC_LINES
, filename, lineno
#endif
));
}


#ifdef _WAVE_HAVE_JUDY
void free_2(void *ptr
#ifdef DEBUG_MALLOC_LINES
, char *filename, int lineno
#endif
)
{
if(ptr)
	{
	int delstat = Judy1Unset ((Pvoid_t)&GLOBALS->alloc2_chain, (Word_t)ptr, PJE0);

	if(delstat)
		{
		GLOBALS->outstanding--;
		free(ptr);
		}
		else
		{
#ifdef DEBUG_MALLOC_LINES
		printf("JUDYMEM | free to non-malloc'd address %p blocked ['%s', %d]\n", ptr, filename, lineno);
#else
		printf("JUDYMEM | free to non-malloc'd address %p blocked\n", ptr);
#endif
		}
	}
	else
	{
#ifdef DEBUG_MALLOC_LINES
        fprintf(stderr, "WARNING: Attempt to free NULL pointer caught. ['%s', %d]\n", filename, lineno);
#else
	fprintf(stderr, "WARNING: Attempt to free NULL pointer caught.\n");
#endif
	}
}
#else
void free_2(void *ptr
#ifdef DEBUG_MALLOC_LINES
, char *filename, int lineno
#endif
)
{
if(ptr)
	{
	void **ret2 = ((void **)ptr) - 2;
	void **prv = (void **)*(ret2+0);
	void **nxt = (void **)*(ret2+1);

	if(prv)
		{
		*(prv+1) = nxt;
		}	
		else
		{
		GLOBALS->alloc2_chain = nxt;
		}

	if(nxt)
		{
		*(nxt+0) = prv;
		}

	GLOBALS->outstanding--;

	free((char *)ptr - 2*sizeof(void *));
	}
	else
	{
#ifdef DEBUG_MALLOC_LINES
        fprintf(stderr, "WARNING: Attempt to free NULL pointer caught. ['%s', %d]\n", filename, lineno);
#else
	fprintf(stderr, "WARNING: Attempt to free NULL pointer caught.\n");
#endif
	}
}
#endif


#ifdef DEBUG_MALLOC_LINES
#define malloc_2(x) malloc_2((x),__FILE__,__LINE__)
#endif


char *strdup_2(const char *s)
{
char *s2 = NULL;

if(s)
	{
	int len = strlen(s);
	s2 = malloc_2(len+1);
	strcpy(s2, s); 
	}

return(s2);
}

char *strdup_2s(const char *s)
{
char *s2 = NULL;

if(s)
	{
	int len = strlen(s);
	s2 = malloc_2(len+2);
	strcpy(s2, s); 
	strcat(s2+len, " ");
	}

return(s2);
}


/*
 * atoi 64-bit version..
 * y/on     default to '1'
 * n/nonnum default to '0'
 */
TimeType atoi_64(const char *str)
{
TimeType val=0;
unsigned char ch, nflag=0;
int consumed = 0;

GLOBALS->atoi_cont_ptr=NULL;

switch(*str)
	{
	case 'y':
	case 'Y':
		return(LLDescriptor(1));

	case 'o':
	case 'O':
		str++;
		ch=*str;
		if((ch=='n')||(ch=='N'))
			return(LLDescriptor(1));
		else	return(LLDescriptor(0));

	case 'n':
	case 'N':
		return(LLDescriptor(0));
		break;

	default:
		break;
	}

while((ch=*(str++)))
	{
	if((ch>='0')&&(ch<='9'))
		{
		val=(val*10+(ch&15));
		consumed = 1;
		}
	else
	if((ch=='-')&&(val==0)&&(!nflag))
		{
		nflag=1;
		consumed = 1;
		}
	else
	if(consumed)
		{
		GLOBALS->atoi_cont_ptr=str-1;
		break;
		}
	}
return(nflag?(-val):val);
}


/*
 * wrapped tooltips
 */
void gtk_tooltips_set_tip_2(GtkTooltips *tooltips, GtkWidget *widget, 
	const gchar *tip_text, const gchar *tip_private)
{
if(!GLOBALS->disable_tooltips)
	{
	gtk_tooltips_set_tip(tooltips, widget, tip_text, tip_private);
	}
}


void gtk_tooltips_set_delay_2(GtkTooltips *tooltips, guint delay)
{
if(!GLOBALS->disable_tooltips)
	{
	gtk_tooltips_set_delay(tooltips, delay);
	}
}


GtkTooltips* gtk_tooltips_new_2(void)
{
if(!GLOBALS->disable_tooltips)
	{
	return(gtk_tooltips_new());
	}
	else
	{
	return(NULL);
	}
}


char *tmpnam_2(char *s, int *fd)
{
#if defined _MSC_VER || defined __MINGW32__

*fd = -1;
return(tmpnam(s));

#else

char *backpath = "gtkwaveXXXXXX";
char *tmpspace;
int len = strlen(P_tmpdir);
int i;

unsigned char slash = '/';
for(i=0;i<len;i++)
	{
	if((P_tmpdir[i] == '\\') || (P_tmpdir[i] == '/'))
		{
		slash = P_tmpdir[i];
		break;
		}
	}

tmpspace = malloc_2(len + 1 + strlen(backpath) + 1);
sprintf(tmpspace, "%s%c%s", P_tmpdir, slash, backpath);
*fd = mkstemp(tmpspace);
if(*fd<0)
	{
	fprintf(stderr, "tmpnam_2() could not create tempfile, exiting.\n");
	perror("Why");
	exit(255);
	}

return(tmpspace);

#endif
}

/************************************************************************/

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

/************************************************************************/

char *find_dumpfile(char *orig_save, char *orig_dump, char *this_save)
{
char *synth_nam = NULL;

if(orig_save && orig_dump && this_save)
	{
	char *dup_orig_save = strdup_2(orig_save);
	char *rhs_orig_save_slash = strrchr(dup_orig_save, SLASH);
	char *grf = NULL;
	char *nam = NULL;
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

