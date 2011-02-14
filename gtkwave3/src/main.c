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
#include <gtk/gtk.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef __MINGW32__
#include <windows.h>
#endif

#include "wave_locale.h"

#if !defined _MSC_VER && !defined __MINGW32__
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/types.h>
#endif

#if !defined _MSC_VER && !defined OS_X && defined WAVE_USE_GTK2
#define WAVE_USE_XID
#else
#undef WAVE_USE_XID
#endif


#ifdef HAVE_GETOPT_LONG
#include <getopt.h>
#else
#include "gnu-getopt.h"
#ifndef _MSC_VER
#include <unistd.h>
#else
#define strcasecmp _stricmp
#endif
#endif

#include "symbol.h"
#include "lx2.h"
#include "ae2.h"
#include "vzt.h"
#include "ghw.h"
#include "fst.h"
#include "main.h"
#include "menu.h"
#include "vcd.h"
#include "lxt.h"
#include "lxt2_read.h"
#include "vzt_read.h"
#include "pixmaps.h"
#include "currenttime.h"
#include "fgetdynamic.h"
#include "rc.h"
#include "translate.h"
#include "ptranslate.h"
#include "ttranslate.h"

#include "tcl_helper.h"
#if defined(HAVE_LIBTCL)
#include <tcl.h>
#include <tk.h>
#endif

#include "rpc.h"

static int suffix_check(const char *s, const char *sfx)
{
int sfxlen = strlen(sfx);
return((strlen(s)>=sfxlen)&&(!strcasecmp(s+strlen(s)-sfxlen,sfx)));
}


static void switch_page(GtkNotebook     *notebook,
			GtkNotebookPage *page,
			guint            page_num,
			gpointer         user_data) 
{
char timestr[32];
struct Global *g_old = GLOBALS;

set_GLOBALS((*GLOBALS->contexts)[page_num]);

GLOBALS->lxt_clock_compress_to_z = g_old->lxt_clock_compress_to_z;
GLOBALS->autoname_bundles = g_old->autoname_bundles;
GLOBALS->autocoalesce_reversal = g_old->autocoalesce_reversal;
GLOBALS->autocoalesce = g_old->autocoalesce;
GLOBALS->hier_grouping = g_old->hier_grouping;
GLOBALS->wave_scrolling = g_old->wave_scrolling;
GLOBALS->constant_marker_update = g_old->constant_marker_update;
GLOBALS->do_zoom_center = g_old->do_zoom_center;
GLOBALS->use_roundcaps = g_old->use_roundcaps;
GLOBALS->do_resize_signals = g_old->do_resize_signals;
GLOBALS->initial_signal_window_width = g_old->initial_signal_window_width;
GLOBALS->scale_to_time_dimension = g_old->scale_to_time_dimension;
GLOBALS->use_full_precision = g_old->use_full_precision;
GLOBALS->show_base = g_old->show_base;
GLOBALS->display_grid = g_old->display_grid;
GLOBALS->disable_mouseover = g_old->disable_mouseover;
GLOBALS->zoom_pow10_snap = g_old->zoom_pow10_snap;
GLOBALS->zoom_dyn = g_old->zoom_dyn;
GLOBALS->zoom_dyne = g_old->zoom_dyne;

reformat_time(timestr, GLOBALS->tims.first, GLOBALS->time_dimension);
gtk_entry_set_text(GTK_ENTRY(GLOBALS->from_entry),timestr);
reformat_time(timestr, GLOBALS->tims.last, GLOBALS->time_dimension);
gtk_entry_set_text(GTK_ENTRY(GLOBALS->to_entry),timestr);

update_maxmarker_labels();
update_basetime(GLOBALS->tims.baseline);

GLOBALS->keypress_handler_id = g_old->keypress_handler_id;

if(GLOBALS->second_page_created)
	{
	gtk_window_set_title(GTK_WINDOW(GLOBALS->mainwindow), GLOBALS->winname);

	MaxSignalLength();
	signalarea_configure_event(GLOBALS->signalarea, NULL);
	wavearea_configure_event(GLOBALS->wavearea, NULL);
	}

}


void kill_stems_browser_single(void *V)
{
struct Global *G = (struct Global *)V;
#if !defined _MSC_VER
	if(G && G->anno_ctx)
		{
#ifdef __MINGW32__
		if(G->anno_ctx->browser_process)
			{
			TerminateProcess(G->anno_ctx->browser_process, 0);
			CloseHandle(G->anno_ctx->browser_process);
			G->anno_ctx->browser_process = 0;
			}
#else
		if(G->anno_ctx->browser_process)
			{
#ifdef __CYGWIN__
			G->anno_ctx->cygwin_remote_kill = 1; /* let cygwin child exit() on its own */
#else
			kill(G->anno_ctx->browser_process, SIGKILL);
#endif
			G->anno_ctx->browser_process = (pid_t)0;
			}
#endif
		G->anno_ctx = NULL;
		}
#endif
}

#if !defined _MSC_VER
void kill_stems_browser(void)
{
int ix;

for(ix=0;ix<GLOBALS->num_notebook_pages;ix++)
	{
        struct Global *G = (*GLOBALS->contexts)[ix];
	kill_stems_browser_single(G);
	}
}
#endif


static int plug_destroy (GtkWidget *widget, gpointer data)
{
exit(0);

return(FALSE);
}       


#if defined __MINGW32__
static void close_all_fst_files(void) /* so mingw does delete of reader tempfiles */
{
unsigned int i;
for(i=0;i<GLOBALS->num_notebook_pages;i++)
	{
        if((*GLOBALS->contexts)[i]->fst_fst_c_1)
		{
		fstReaderClose((*GLOBALS->contexts)[i]->fst_fst_c_1);
		(*GLOBALS->contexts)[i]->fst_fst_c_1 = NULL;
		}
        }
}
#endif


static void print_help(char *nam)
{
#if !defined _MSC_VER && !defined __MINGW32__ && !defined __FreeBSD__ && !defined __CYGWIN__
#define WAVE_GETOPT_CPUS "  -c, --cpu=NUMCPUS          specify number of CPUs for parallelizable ops\n"
#else
#define WAVE_GETOPT_CPUS
#endif

#if !defined _MSC_VER && !defined __MINGW32__
#define VCD_GETOPT       "  -o, --optimize             optimize VCD to FST\n"
#else
#define VCD_GETOPT
#endif

#if !defined _MSC_VER
#define STEMS_GETOPT	 "  -t, --stems=FILE           specify stems file for source code annotation\n"
#define DUAL_GETOPT      "  -D, --dualid=WHICH         specify multisession identifier\n"
#define INTR_GETOPT      "  -I, --interactive          interactive VCD mode (filename is shared mem ID)\n"
#else
#define STEMS_GETOPT
#define DUAL_GETOPT
#define INTR_GETOPT
#endif

#ifdef WAVE_USE_XID
#define XID_GETOPT 	 "  -X, --xid=XID              specify XID of window for GtkPlug to connect to\n"
#else
#define XID_GETOPT
#endif

#if defined(WIN32) && defined(USE_TCL_STUBS)
#define WISH_GETOPT
#else
#define WISH_GETOPT	 "  -T, --tcl_init=FILE        specify Tcl command script file to be loaded on startup\n" \
			 "  -W, --wish                 enable Tcl command line on stdio\n"
#endif

#if defined(HAVE_LIBTCL)
#define REPSCRIPT_GETOPT WISH_GETOPT \
                         "  -R, --repscript=FILE       specify timer-driven Tcl command script file\n" \
                         "  -P, --repperiod=VALUE      specify repscript period in msec (default: 500)\n"
#else
#define REPSCRIPT_GETOPT
#endif

#if !defined _MSC_VER && !defined __MINGW32__
#define OUTPUT_GETOPT "  -O, --output=FILE          specify filename for stdout/stderr redirect\n"
#define RPC_GETOPT "  -1, --rpcid=RPCID          specify RPCID of gtkwave_server to connect to\n"
#else
#define OUTPUT_GETOPT
#define RPC_GETOPT
#endif

#if defined(WAVE_USE_GTK2)
#define SLIDEZOOM_OPT "  -z, --slider-zoom          enable horizontal slider stretch zoom\n"
#else
#define SLIDEZOOM_OPT 
#endif

printf(
"Usage: %s [OPTION]... [DUMPFILE] [SAVEFILE] [RCFILE]\n\n"
"  -n, --nocli=DIRPATH        use file requester for dumpfile name\n"
"  -f, --dump=FILE            specify dumpfile name\n"
"  -F, --fastload             generate/use VCD recoder fastload files\n"
VCD_GETOPT
"  -a, --save=FILE            specify savefile name\n"
"  -A, --autosavename         assume savefile is suffix modified dumpfile name\n"
"  -r, --rcfile=FILE          specify override .rcfile name\n"
"  -d, --defaultskip          if missing .rcfile, do not use useful defaults\n"
DUAL_GETOPT
"  -i, --indirect=FILE        specify indirect facs file name\n"
"  -l, --logfile=FILE         specify simulation logfile name for time values\n"
"  -s, --start=TIME           specify start time for LXT2/VZT block skip\n"
"  -e, --end=TIME             specify end time for LXT2/VZT block skip\n"  
STEMS_GETOPT
WAVE_GETOPT_CPUS
"  -N, --nowm                 disable window manager for most windows\n"
"  -M, --nomenus              do not render menubar (for making applets)\n"
"  -S, --script=FILE          specify Tcl command script file for execution\n"
REPSCRIPT_GETOPT
XID_GETOPT
RPC_GETOPT
INTR_GETOPT
"  -C, --comphier             use compressed hierarchy names (slower)\n"
"  -g, --giga                 use gigabyte mempacking when recoding (slower)\n"
"  -L, --legacy               use legacy VCD mode rather than the VCD recoder\n" 
"  -v, --vcd                  use stdin as a VCD dumpfile\n"
OUTPUT_GETOPT
SLIDEZOOM_OPT
"  -V, --version              display version banner then exit\n"
"  -h, --help                 display this help then exit\n"
"  -x, --exit                 exit after loading trace (for loader benchmarks)\n\n"

"VCD files and save files may be compressed with zip or gzip.\n"
"GHW files may be compressed with gzip or bzip2.\n"
"Other formats must remain uncompressed due to their non-linear access.\n"
"Note that DUMPFILE is optional if the --dump or --nocli options are specified.\n"
"SAVEFILE and RCFILE are always optional.\n\n"

"Report bugs to <bybell@nc.rr.com>.\n",nam);

exit(0);
}


/*
 * file selection for -n/--nocli flag 
 */

static void wave_get_filename_cleanup(GtkWidget *widget, gpointer data) { gtk_main_quit(); /* do nothing but exit gtk loop */ }

static char *wave_get_filename(char *dfile)
{
if(dfile) 
	{
	int len = strlen(dfile);
	GLOBALS->ftext_main_main_c_1 = malloc_2(strlen(dfile)+2);
	strcpy(GLOBALS->ftext_main_main_c_1, dfile);
#if !defined _MSC_VER && !defined __MINGW32__
	if((len)&&(dfile[len-1]!='/'))
		{
		strcpy(GLOBALS->ftext_main_main_c_1 + len, "/");
		}
#else
	if((len)&&(dfile[len-1]!='\\'))
		{
		strcpy(GLOBALS->ftext_main_main_c_1 + len, "\\");
		}
#endif
	}
fileselbox_old("GTKWave: Select a dumpfile...",&GLOBALS->ftext_main_main_c_1,GTK_SIGNAL_FUNC(wave_get_filename_cleanup), GTK_SIGNAL_FUNC(wave_get_filename_cleanup), NULL);
gtk_main();

return(GLOBALS->ftext_main_main_c_1);
}

/*
 * Modify the name of the executable (argv[0]) handed to Tk_MainEx;
 * The new executable name has _[pid] appended. This gives a unique
 * (and known) name to the interpreter (for use with send).
 */
void addPidToExecutableName(int argc, char* argv[], char* argv_mod[])
{
  char* pos;
  char* buffer;

  int i;
  for(i=0;i<argc;i++)
    {
      argv_mod[i] = argv[i];
    }

  buffer = malloc_2(strlen(argv[0])+1+10);
  pos = buffer;
  strcpy(pos, argv[0]);
  pos = buffer + strlen(buffer);
  strcpy(pos, "_");
  pos = buffer + strlen(buffer);
  sprintf(pos, "%d", getpid());
	  
  argv_mod[0] = buffer;
}


int main(int argc, char *argv[])
{
return(main_2(argc, argv));
}

int main_2(int argc, char *argv[])
{
static char *winprefix="GTKWave - ";
static char *winstd="GTKWave (stdio) ";
static char *vcd_autosave_name="vcd_autosave.sav";
char *output_name = NULL;

int i;
int c;
char is_vcd=0;
char is_wish=0;
char is_interactive=0;
char is_smartsave = 0;
char is_legacy = 0;
char is_fastload = VCD_FSL_NONE;
char is_giga = 0;
char fast_exit=0;
char opt_errors_encountered=0;
char is_missing_file = 0;

char *wname=NULL;
char *override_rc=NULL;
char *scriptfile=NULL;
FILE *wave = NULL;
FILE *vcd_save_handle_cached = NULL;

GtkWidget *main_vbox = NULL, *top_table = NULL, *whole_table = NULL;
GtkWidget *menubar;
GtkWidget *text1;
GtkWidget *zoombuttons;
GtkWidget *pagebuttons;
GtkWidget *fetchbuttons;
GtkWidget *discardbuttons;
GtkWidget *shiftbuttons;
GtkWidget *edgebuttons;
GtkWidget *entry;
GtkWidget *timebox;
GtkWidget *panedwindow;
GtkWidget *dummy1, *dummy2;
GtkWidget *toolhandle=NULL;
int tcl_interpreter_needs_making = 0;

int splash_disable_rc_override = 0;
int mainwindow_already_built;

WAVE_LOCALE_FIX

/* Initialize the GLOBALS structure for the first time... */ 
if(!GLOBALS)
	{
	set_GLOBALS(initialize_globals());
	mainwindow_already_built = 0;
	tcl_interpreter_needs_making = 1;

	GLOBALS->logfiles = calloc(1, sizeof(void **)); /* calloc is deliberate! */
	}
	else
	{
	struct Global *old_g = GLOBALS;

	set_GLOBALS(initialize_globals());

	GLOBALS->second_page_created = old_g->second_page_created = 1;

	GLOBALS->notebook = old_g->notebook;
	GLOBALS->num_notebook_pages = old_g->num_notebook_pages;
	GLOBALS->num_notebook_pages_cumulative = old_g->num_notebook_pages_cumulative;
	GLOBALS->contexts = old_g->contexts;

	GLOBALS->mainwindow = old_g->mainwindow;
	splash_disable_rc_override = 1;

	/* busy.c */
	GLOBALS->busycursor_busy_c_1 = old_g->busycursor_busy_c_1;

	/* logfiles.c */
	GLOBALS->logfiles = old_g->logfiles;

	/* menu.c */
#if defined(HAVE_LIBTCL)
	GLOBALS->interp = old_g->interp;
#endif
	GLOBALS->item_factory_menu_c_1 = old_g->item_factory_menu_c_1;
	GLOBALS->vcd_jmp_buf = old_g->vcd_jmp_buf;

	/* currenttime.c */
	GLOBALS->max_or_marker_label_currenttime_c_1 = old_g->max_or_marker_label_currenttime_c_1;
	GLOBALS->maxtext_currenttime_c_1=(char *)malloc_2(40);
	GLOBALS->maxtimewid_currenttime_c_1 = old_g->maxtimewid_currenttime_c_1;
	GLOBALS->curtext_currenttime_c_1 = old_g->curtext_currenttime_c_1;
	GLOBALS->base_or_curtime_label_currenttime_c_1 = old_g->base_or_curtime_label_currenttime_c_1;
	GLOBALS->curtimewid_currenttime_c_1 = old_g->curtimewid_currenttime_c_1;

	/* status.c */
	GLOBALS->text_status_c_2 = old_g->text_status_c_2;
	GLOBALS->vscrollbar_status_c_2 = old_g->vscrollbar_status_c_2;
#if defined(WAVE_USE_GTK2) && !defined(GTK_ENABLE_BROKEN)
	GLOBALS->iter_status_c_3 = old_g->iter_status_c_3;
	GLOBALS->bold_tag_status_c_3 = old_g->bold_tag_status_c_3;
#endif

	/* timeentry.c */
	GLOBALS->from_entry = old_g->from_entry;
	GLOBALS->to_entry = old_g->to_entry;
	
	/* rc.c */
	GLOBALS->possibly_use_rc_defaults = old_g->possibly_use_rc_defaults;
	GLOBALS->ignore_savefile_pos = old_g->ignore_savefile_pos;
	GLOBALS->ignore_savefile_size = old_g->ignore_savefile_size;
	
	GLOBALS->color_back = old_g->color_back;
	GLOBALS->color_baseline = old_g->color_baseline;
	GLOBALS->color_grid = old_g->color_grid;
	GLOBALS->color_grid2 = old_g->color_grid2;
	GLOBALS->color_high = old_g->color_high;
	GLOBALS->color_low = old_g->color_low;
	GLOBALS->color_1 = old_g->color_1;
	GLOBALS->color_0 = old_g->color_0;
	GLOBALS->color_mark = old_g->color_mark;
	GLOBALS->color_mid = old_g->color_mid;
	GLOBALS->color_time = old_g->color_time;
	GLOBALS->color_timeb = old_g->color_timeb;
	GLOBALS->color_trans = old_g->color_trans;
	GLOBALS->color_umark = old_g->color_umark;
	GLOBALS->color_value = old_g->color_value;
	GLOBALS->color_vbox = old_g->color_vbox;
	GLOBALS->color_vtrans = old_g->color_vtrans;
	GLOBALS->color_x = old_g->color_x;
	GLOBALS->color_xfill = old_g->color_xfill;
	GLOBALS->color_u = old_g->color_u;
	GLOBALS->color_ufill = old_g->color_ufill;
	GLOBALS->color_w = old_g->color_w;
	GLOBALS->color_wfill = old_g->color_wfill;
	GLOBALS->color_dash = old_g->color_dash;
	GLOBALS->color_dashfill = old_g->color_dashfill;
	GLOBALS->color_white = old_g->color_white;
	GLOBALS->color_black = old_g->color_black;
	GLOBALS->color_ltgray = old_g->color_ltgray;
	GLOBALS->color_normal = old_g->color_normal;
	GLOBALS->color_mdgray = old_g->color_mdgray;
	GLOBALS->color_dkgray = old_g->color_dkgray;
	GLOBALS->color_dkblue = old_g->color_dkblue;
	GLOBALS->color_brkred = old_g->color_brkred;
	GLOBALS->color_ltblue = old_g->color_ltblue;
	GLOBALS->color_gmstrd = old_g->color_gmstrd;
	
	GLOBALS->atomic_vectors = old_g->atomic_vectors;
	GLOBALS->autoname_bundles = old_g->autoname_bundles;
	GLOBALS->autocoalesce = old_g->autocoalesce;
	GLOBALS->autocoalesce_reversal = old_g->autocoalesce_reversal;
	GLOBALS->constant_marker_update = old_g->constant_marker_update;
	GLOBALS->convert_to_reals = old_g->convert_to_reals;
	GLOBALS->disable_mouseover = old_g->disable_mouseover;
	GLOBALS->disable_tooltips = old_g->disable_tooltips;
	GLOBALS->do_initial_zoom_fit = old_g->do_initial_zoom_fit;
	GLOBALS->do_resize_signals = old_g->do_resize_signals;
	GLOBALS->initial_signal_window_width = old_g->initial_signal_window_width;
	GLOBALS->scale_to_time_dimension = old_g->scale_to_time_dimension;
	GLOBALS->enable_fast_exit = old_g->enable_fast_exit;
	GLOBALS->enable_ghost_marker = old_g->enable_ghost_marker;
	GLOBALS->enable_horiz_grid = old_g->enable_horiz_grid;
	GLOBALS->make_vcd_save_file = old_g->make_vcd_save_file;
	GLOBALS->enable_vert_grid = old_g->enable_vert_grid;
	GLOBALS->force_toolbars = old_g->force_toolbars;
	GLOBALS->hide_sst = old_g->hide_sst;
	GLOBALS->sst_expanded = old_g->sst_expanded;
	GLOBALS->hier_grouping = old_g->hier_grouping;
	GLOBALS->hier_max_level = old_g->hier_max_level;
	GLOBALS->hier_max_level_shadow = old_g->hier_max_level_shadow;
	GLOBALS->paned_pack_semantics = old_g->paned_pack_semantics;
	GLOBALS->left_justify_sigs = old_g->left_justify_sigs;
	GLOBALS->lxt_clock_compress_to_z = old_g->lxt_clock_compress_to_z;
	GLOBALS->ps_maxveclen = old_g->ps_maxveclen;
	GLOBALS->show_base = old_g->show_base;
	GLOBALS->display_grid = old_g->display_grid;
	GLOBALS->use_big_fonts = old_g->use_big_fonts;
	GLOBALS->use_full_precision = old_g->use_full_precision;
	GLOBALS->use_frequency_delta = old_g->use_frequency_delta;
	GLOBALS->use_maxtime_display = old_g->use_maxtime_display;
	GLOBALS->use_nonprop_fonts = old_g->use_nonprop_fonts;
	GLOBALS->use_roundcaps = old_g->use_roundcaps;
	GLOBALS->use_scrollbar_only = old_g->use_scrollbar_only;
	GLOBALS->vcd_explicit_zero_subscripts = old_g->vcd_explicit_zero_subscripts;
	GLOBALS->vcd_preserve_glitches = old_g->vcd_preserve_glitches;
	GLOBALS->vcd_warning_filesize = old_g->vcd_warning_filesize;
	GLOBALS->vector_padding = old_g->vector_padding;
	GLOBALS->vlist_compression_depth = old_g->vlist_compression_depth;
	GLOBALS->wave_scrolling = old_g->wave_scrolling;
	GLOBALS->do_zoom_center = old_g->do_zoom_center;
	GLOBALS->zoom_pow10_snap = old_g->zoom_pow10_snap;
	GLOBALS->zoom_dyn = old_g->zoom_dyn;
	GLOBALS->zoom_dyne = old_g->zoom_dyne;
	GLOBALS->alt_hier_delimeter = old_g->alt_hier_delimeter;
	GLOBALS->cursor_snap = old_g->cursor_snap;
	GLOBALS->hier_delimeter = old_g->hier_delimeter;
	GLOBALS->hier_was_explicitly_set = old_g->hier_was_explicitly_set;
	GLOBALS->page_divisor = old_g->page_divisor;
	GLOBALS->ps_maxveclen = old_g->ps_maxveclen;
	GLOBALS->vector_padding = old_g->vector_padding;
	GLOBALS->vlist_compression_depth = old_g->vlist_compression_depth;
	GLOBALS->zoombase = old_g->zoombase;
	GLOBALS->splash_disable = old_g->splash_disable;
	GLOBALS->use_pango_fonts = old_g->use_pango_fonts;
	
	GLOBALS->vlist_spill_to_disk = old_g->vlist_spill_to_disk;
	GLOBALS->vlist_prepack = old_g->vlist_prepack;
	GLOBALS->do_dynamic_treefilter = old_g->do_dynamic_treefilter;
	GLOBALS->use_standard_clicking = old_g->use_standard_clicking;
	GLOBALS->use_toolbutton_interface = old_g->use_toolbutton_interface;

	GLOBALS->use_scrollwheel_as_y = old_g->use_scrollwheel_as_y;
	GLOBALS->enable_slider_zoom = old_g->enable_slider_zoom;

	strcpy2_into_new_context(GLOBALS, &GLOBALS->fontname_logfile, &old_g->fontname_logfile);
	strcpy2_into_new_context(GLOBALS, &GLOBALS->fontname_signals, &old_g->fontname_signals); 
	strcpy2_into_new_context(GLOBALS, &GLOBALS->fontname_waves, &old_g->fontname_waves);
        strcpy2_into_new_context(GLOBALS, &GLOBALS->argvlist, &old_g->argvlist);

	mainwindow_already_built = 1;
	}

GLOBALS->whoami=malloc_2(strlen(argv[0])+1);	/* cache name in case we fork later */
strcpy(GLOBALS->whoami, argv[0]);

if(!mainwindow_already_built)
	{
#ifdef __MINGW32__
	gtk_disable_setlocale();
#endif
	if(!gtk_init_check(&argc, &argv))
		{
#if defined(__APPLE__)
		if(!getenv("DISPLAY"))
			{
			fprintf(stderr, "DISPLAY environment variable is not set.  Have you ensured\n");
			fprintf(stderr, "that x11 has been initialized through open-x11, launching\n");
			fprintf(stderr, "gtkwave in an xterm or x11 window, etc?\n\n");
			fprintf(stderr, "Attempting to initialize using DISPLAY=:0.0 value...\n\n");
			setenv("DISPLAY", ":0.0", 0);
			if(gtk_init_check(&argc, &argv))
				{
				goto do_primary_inits;
				}
			}
#endif
		fprintf(stderr, "Could not initialize GTK!  Is DISPLAY env var/xhost set?\n\n");
		print_help(argv[0]);
		}
	}

#if defined(__APPLE__)
do_primary_inits:
#endif

init_filetrans_data(); /* for file translation splay trees */
init_proctrans_data(); /* for proc translation structs */
init_ttrans_data();    /* for transaction proc translation structs */
if(!mainwindow_already_built) 
	{
	atexit(remove_all_proc_filters);
	atexit(remove_all_ttrans_filters);
#if defined __MINGW32__
	atexit(close_all_fst_files);
#endif
	}

if(mainwindow_already_built)
	{
	optind = 1;
	}
else
while (1)
        {
        int option_index = 0;

        static struct option long_options[] =
                {
                {"dump", 1, 0, 'f'},
		{"fastload", 0, 0, 'F'},
                {"optimize", 0, 0, 'o'},
                {"nocli", 1, 0, 'n'},
                {"save", 1, 0, 'a'},
                {"autosavename", 0, 0, 'A'},
                {"rcfile", 1, 0, 'r'},
                {"defaultskip", 0, 0, 'd'},
                {"indirect", 1, 0, 'i'},
                {"logfile", 1, 0, 'l'},
                {"start", 1, 0, 's'},
                {"end", 1, 0, 'e'},
                {"cpus", 1, 0, 'c'},
                {"stems", 1, 0, 't'},
                {"nowm", 0, 0, 'N'},
                {"script", 1, 0, 'S'},
                {"vcd", 0, 0, 'v'},
                {"version", 0, 0, 'V'},
                {"help", 0, 0, 'h'},
                {"exit", 0, 0, 'x'},
                {"xid", 1, 0, 'X'},
                {"nomenus", 0, 0, 'M'},
                {"dualid", 1, 0, 'D'},
                {"interactive", 0, 0, 'I'},
		{"giga", 0, 0, 'g'},
		{"comphier", 0, 0, 'C'},
                {"legacy", 0, 0, 'L'},  
		{"tcl_init", 1, 0, 'T'},
		{"wish", 0, 0, 'W'},
                {"repscript", 1, 0, 'R'},   
                {"repperiod", 1, 0, 'P'},
		{"output", 1, 0, 'O' },
                {"slider-zoom", 0, 0, 'z'},
		{"rpc", 1, 0, '1' },
                {0, 0, 0, 0}
                };

        c = getopt_long (argc, argv, "zf:Fon:a:Ar:di:l:s:e:c:t:NS:vVhxX:MD:IgCLR:P:O:WT:1:", long_options, 
&option_index);

        if (c == -1) break;     /* no more args */

        switch (c)
                {
		case 'V':
			printf(
			WAVE_VERSION_INFO"\n\n"
			"This is free software; see the source for copying conditions.  There is NO\n"
			"warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n"
			);
			exit(0);

		case 'W':
#if defined(HAVE_LIBTCL)
#if defined(WIN32) && defined(USE_TCL_STUBS)
#else
			is_wish = 1;
#endif
#else
			fprintf(stderr, "GTKWAVE | Tcl support not compiled into this executable, exiting.\n");
			exit(255);
#endif
			break;

		case 'I':
#if !defined _MSC_VER
			is_interactive = 1;
#endif
			break;

		case 'L':
			is_legacy = 1;
			break;

		case 'D':
#if !defined _MSC_VER
			{
			char *s = optarg;
			char *plus = strchr(s, '+');
			if((plus)&&(*(plus+1)))
				{
				sscanf(plus+1, "%x", &GLOBALS->dual_attach_id_main_c_1);
				if(plus != s)
					{
					char p = *(plus-1);

					if(p=='0')
						{
						GLOBALS->dual_id = 0;
						break;
						}
					else
					if(p=='1')
						{
						GLOBALS->dual_id = 1;
						break;
						}
					}
				}

			fprintf(stderr, "Malformed dual session ID.  Must be of form m+nnnnnnnn where m is 0 or 1,\n"
					"and n is a hexadecimal shared memory ID for use with shmat()\n");
			exit(255);
			}
#else
			{
			fprintf(stderr, "Dual operation not implemented for Win32, exiting.\n");
			exit(255);
			}
#endif
			break;

		case 'A':
			is_smartsave = 1;
			break;

                case 'v':
			is_vcd = 1;
			if(GLOBALS->loaded_file_name) free_2(GLOBALS->loaded_file_name);
			GLOBALS->loaded_file_name = malloc_2(4+1);
			strcpy(GLOBALS->loaded_file_name, "-vcd");
                        break;

		case 'o':
			GLOBALS->optimize_vcd = 1;
			break;

		case 'n':
			wave_get_filename(optarg);
			if(GLOBALS->filesel_ok)
				{
				if(GLOBALS->loaded_file_name) free_2(GLOBALS->loaded_file_name);
				GLOBALS->loaded_file_name = GLOBALS->ftext_main_main_c_1;
				GLOBALS->ftext_main_main_c_1 = NULL;
				}
			break;

                case 'h':
			print_help(argv[0]);
                        break;

#ifdef WAVE_USE_XID
                case 'X': 
                        sscanf(optarg, "%x", &GLOBALS->socket_xid);
			splash_disable_rc_override = 1;
                        break;
#endif

		case '1':
			sscanf(optarg, "%x", &GLOBALS->rpc_id);
			break;

		case 'M':
			GLOBALS->disable_menus = 1;
			break;

                case 'x':
			fast_exit = 1;
			splash_disable_rc_override = 1;
                        break;

		case 'd':
			GLOBALS->possibly_use_rc_defaults = 0;
			break;

                case 'f':
			is_vcd = 0;
			if(GLOBALS->loaded_file_name) free_2(GLOBALS->loaded_file_name);
			GLOBALS->loaded_file_name = malloc_2(strlen(optarg)+1);
			strcpy(GLOBALS->loaded_file_name, optarg);
			break;

		case 'F':
			is_fastload = VCD_FSL_WRITE;
			is_giga = 1;
			break;
			
                case 'a':
			if(wname) free_2(wname);
			wname = malloc_2(strlen(optarg)+1);
			strcpy(wname, optarg);
			break;

                case 'r':
			if(override_rc) free_2(override_rc);
			override_rc = malloc_2(strlen(optarg)+1);
			strcpy(override_rc, optarg);
			break;

                case 'i':
			if(GLOBALS->indirect_fname) free_2(GLOBALS->indirect_fname);
			GLOBALS->indirect_fname = malloc_2(strlen(optarg)+1);
			strcpy(GLOBALS->indirect_fname, optarg);
			break;

                case 's':
			if(GLOBALS->skip_start) free_2(GLOBALS->skip_start);
			GLOBALS->skip_start = malloc_2(strlen(optarg)+1);
			strcpy(GLOBALS->skip_start, optarg);
			break;			

                case 'e':
			if(GLOBALS->skip_end) free_2(GLOBALS->skip_end);
			GLOBALS->skip_end = malloc_2(strlen(optarg)+1);
			strcpy(GLOBALS->skip_end, optarg);
                        break;

		case 't':
#if !defined _MSC_VER
			if(GLOBALS->stems_name) free_2(GLOBALS->stems_name);
			GLOBALS->stems_name = malloc_2(strlen(optarg)+1);
			strcpy(GLOBALS->stems_name, optarg);
#else
			fprintf(stderr, "GTKWAVE | Warning: '%c' option does not exist in this executable\n", c);
#endif
			break;

                case 'c':
#if !defined _MSC_VER && !defined __MINGW32__ && !defined __FreeBSD__ && !defined __CYGWIN__
			GLOBALS->num_cpus = atoi(optarg);
			if(GLOBALS->num_cpus<1) GLOBALS->num_cpus = 1;
			if(GLOBALS->num_cpus>8) GLOBALS->num_cpus = 8;
#else
			fprintf(stderr, "GTKWAVE | Warning: '%c' option does not exist in this executable\n", c);
#endif
                        break;

		case 'N':
			GLOBALS->disable_window_manager = 1;
			break;

		case 'S':
			if(scriptfile) free_2(scriptfile);
			scriptfile = malloc_2(strlen(optarg)+1);
			strcpy(scriptfile, optarg);
			splash_disable_rc_override = 1;
			break;

		case 'l':
			{
			struct logfile_chain *l = calloc_2(1, sizeof(struct logfile_chain));
			struct logfile_chain *ltraverse;
			l->name = malloc_2(strlen(optarg)+1);
			strcpy(l->name, optarg);

			if(GLOBALS->logfile)
				{
				ltraverse = GLOBALS->logfile;
				while(ltraverse->next) ltraverse = ltraverse->next;
				ltraverse->next = l;
				}
				else
				{
				GLOBALS->logfile = l;
				}
			}
			break;

		case 'g':
			is_giga = 1;
			break;

		case 'C':
			GLOBALS->do_hier_compress = 1;
			break;

                case 'R':
                        if(GLOBALS->repscript_name) free_2(GLOBALS->repscript_name);
                        GLOBALS->repscript_name = malloc_2(strlen(optarg)+1);
                        strcpy(GLOBALS->repscript_name, optarg);
                        break;
                
                case 'P':   
                        {
                        int pd = atoi(optarg);
                        if(pd > 0)
                                {
                                GLOBALS->repscript_period = pd;
                                }
                        }
                        break;

                case 'T':
#if defined(WIN32) && defined(USE_TCL_STUBS)
			fprintf(stderr, "GTKWAVE | Warning: '%c' option does not exist in this executable\n", c);
#else
		        {
			  char* pos;
			  is_wish = 1;
			  if(GLOBALS->tcl_init_cmd) 
			    {
			      int length = strlen(GLOBALS->tcl_init_cmd)+9+strlen(optarg);
			      char* buffer = malloc_2(strlen(GLOBALS->tcl_init_cmd)+1);
			      strcpy(buffer, GLOBALS->tcl_init_cmd);
			      free_2(GLOBALS->tcl_init_cmd);
			      GLOBALS->tcl_init_cmd = malloc_2(length+1);
			      strcpy(GLOBALS->tcl_init_cmd, buffer);
			      pos = GLOBALS->tcl_init_cmd + strlen(GLOBALS->tcl_init_cmd);
			      free_2(buffer);
			    }
			  else
			    {
			      int length = 9+strlen(optarg);
			      GLOBALS->tcl_init_cmd = malloc_2(length+1);
			      pos = GLOBALS->tcl_init_cmd;
			    }
			  strcpy(pos, "; source ");
			  pos = GLOBALS->tcl_init_cmd + strlen(GLOBALS->tcl_init_cmd);
			  strcpy(pos, optarg);
			}
#endif
			break;

                case 'O':
			if(output_name) free_2(output_name);
			output_name = malloc_2(strlen(optarg)+1);
			strcpy(output_name, optarg);
			break;

		case 'z':
			GLOBALS->enable_slider_zoom = 1;
			break;

                case '?':
			opt_errors_encountered=1;
                        break;

                default:
                        /* unreachable */
                        break;
                }
        } /* ...while(1) */

if(opt_errors_encountered)
	{
	print_help(argv[0]);
	}

if (optind < argc)
        {
        while (optind < argc)
		{
                if(argv[optind][0] == '-')
                        {
                        if(!strcmp(argv[optind], "--"))
                                {
                                break;
                                }
                        }   

		if(!GLOBALS->loaded_file_name)
			{
			is_vcd = 0;
			GLOBALS->loaded_file_name = malloc_2(strlen(argv[optind])+1);
			strcpy(GLOBALS->loaded_file_name, argv[optind++]);
			}
		else if(!wname)
			{
			wname = malloc_2(strlen(argv[optind])+1);
			strcpy(wname, argv[optind++]);
			}
		else if(!override_rc)
			{
			override_rc = malloc_2(strlen(argv[optind])+1);
			strcpy(override_rc, argv[optind++]);
			break; /* skip any extra args */
			}
		}
        }

if(is_wish && is_vcd)
	{
	fprintf(stderr, 
		"GTKWAVE | Cannot use --vcd and --wish options together as both use stdin,\n"
		"GTKWAVE | exiting!\n");
	exit(255);
	}

read_rc_file(override_rc);
GLOBALS->splash_disable |= splash_disable_rc_override;

if(!GLOBALS->loaded_file_name)
	{
	/* if rc can gates off gui, default is not to disable */
	if(GLOBALS->disable_empty_gui)
		{
		print_help(argv[0]);
		}
	}

if(is_giga)
	{
	GLOBALS->vlist_spill_to_disk = 1;
	GLOBALS->vlist_prepack = 1;
	}

if(output_name)
	{
#if !defined _MSC_VER && !defined __MINGW32__
	int iarg;
	time_t walltime;
	int fd_replace = open(output_name, O_CREAT|O_WRONLY|O_TRUNC, S_IRUSR|S_IWUSR);
	if(fd_replace<0)
		{
		fprintf(stderr, "Could not open redirect file, exiting.\n");
		perror("Why");
		exit(255);
		}

	dup2(fd_replace, 1);
	dup2(fd_replace, 2);

        time(&walltime);
        printf(WAVE_VERSION_INFO"\nDate: %s\n\n",asctime(localtime(&walltime)));

	for(iarg=0;iarg<argc;iarg++)
		{
		if(iarg) printf("\t");
		printf("%s\n", argv[iarg]);
		}

	printf("\n\n");
	fflush(stdout);

#endif
	free_2(output_name);
	output_name = NULL;
	}

fprintf(stderr, "\n%s\n\n",WAVE_VERSION_INFO);
if(!is_wish)
	{
	if(tcl_interpreter_needs_making)
	        {
	        GLOBALS->argvlist = zMergeTclList(argc, (const char**)argv);
	        make_tcl_interpreter(argv);
	        }
	}

if((!wname)&&(GLOBALS->make_vcd_save_file))
	{
	vcd_save_handle_cached = GLOBALS->vcd_save_handle=fopen(vcd_autosave_name,"wb");
	errno=0;	/* just in case */
	is_smartsave = (GLOBALS->vcd_save_handle != NULL); /* use smartsave if for some reason can't open auto savefile */
	}

if(!GLOBALS->loaded_file_name)
	{
	GLOBALS->loaded_file_name = strdup_2("[no file loaded]");	
	is_missing_file = 1;
	GLOBALS->min_time=LLDescriptor(0);
	GLOBALS->max_time=LLDescriptor(0);
	if(!is_wish)
		{
		fprintf(stderr, "GTKWAVE | Use the -h, --help command line flags to display help.\n");
		}
	}
	
/* load either the vcd or aet file depending on suffix then mode setting */
if(is_vcd)
	{
	GLOBALS->winname=malloc_2(strlen(winstd)+4+1);
	strcpy(GLOBALS->winname,winstd);
	}
	else
	{
	if(!is_interactive)
		{
		GLOBALS->winname=malloc_2(strlen(GLOBALS->loaded_file_name)+strlen(winprefix)+1);
		strcpy(GLOBALS->winname,winprefix);
		}
		else
		{
		char *iact = "GTKWave - Interactive Shared Memory ID ";
		GLOBALS->winname=malloc_2(strlen(GLOBALS->loaded_file_name)+strlen(iact)+1);
		strcpy(GLOBALS->winname,iact);
		}
	}

strcat(GLOBALS->winname,GLOBALS->loaded_file_name);

loader_check_head:

if(is_missing_file)
	{
	GLOBALS->loaded_file_type = MISSING_FILE;
	}
else
#ifdef EXTLOAD_SUFFIX
if(suffix_check(GLOBALS->loaded_file_name, EXTLOAD_SUFFIX))
	{
	TimeType extload_max;

	GLOBALS->loaded_file_type = EXTLOAD_FILE;
	extload_max = extload_main(GLOBALS->loaded_file_name, GLOBALS->skip_start, GLOBALS->skip_end);
	if((!GLOBALS->extload) || (GLOBALS->extload_already_errored) || (!extload_max))
		{
		fprintf(stderr, "GTKWAVE | Could not initialize '%s'%s.\n", GLOBALS->loaded_file_name, GLOBALS->vcd_jmp_buf ? "" : ", exiting");
		vcd_exit(255);
		}
	}
else
#endif
if(suffix_check(GLOBALS->loaded_file_name, ".lxt") || suffix_check(GLOBALS->loaded_file_name, ".lx2") || suffix_check(GLOBALS->loaded_file_name, ".lxt2"))
	{
	FILE *f = fopen(GLOBALS->loaded_file_name, "rb");
	int typ = 0;

	if(f)
		{
		char buf[2];
		unsigned int matchword;

		if(fread(buf, 2, 1, f))
			{
			matchword = (((unsigned int)buf[0])<<8) | ((unsigned int)buf[1]);
			if(matchword == LT_HDRID) typ = 1;
			}

		fclose(f);
		}

	if(typ)
		{
	          GLOBALS->loaded_file_type = LXT_FILE;
		  lxt_main(GLOBALS->loaded_file_name);
		}
		else
		{
#if !defined _MSC_VER
		GLOBALS->stems_type = WAVE_ANNO_LXT2;
		GLOBALS->aet_name = malloc_2(strlen(GLOBALS->loaded_file_name)+1);
		strcpy(GLOBALS->aet_name, GLOBALS->loaded_file_name);
#endif
                GLOBALS->loaded_file_type = LX2_FILE;
		lx2_main(GLOBALS->loaded_file_name, GLOBALS->skip_start, GLOBALS->skip_end);
		if(!GLOBALS->lx2_lx2_c_1)
			{
			fprintf(stderr, "GTKWAVE | Could not initialize '%s'%s.\n", GLOBALS->loaded_file_name, GLOBALS->vcd_jmp_buf ? "" : ", exiting");
			vcd_exit(255);
			}
		}	
	}
else
if(suffix_check(GLOBALS->loaded_file_name, ".fst"))
	{
#if !defined _MSC_VER
	GLOBALS->stems_type = WAVE_ANNO_FST;
	GLOBALS->aet_name = malloc_2(strlen(GLOBALS->loaded_file_name)+1);
	strcpy(GLOBALS->aet_name, GLOBALS->loaded_file_name);
#endif
        GLOBALS->loaded_file_type = FST_FILE;
	fst_main(GLOBALS->loaded_file_name, GLOBALS->skip_start, GLOBALS->skip_end);
	if(!GLOBALS->fst_fst_c_1)
		{
		fprintf(stderr, "GTKWAVE | Could not initialize '%s'%s.\n", GLOBALS->loaded_file_name, GLOBALS->vcd_jmp_buf ? "" : ", exiting");
		vcd_exit(255);
		}
	}
else
if(suffix_check(GLOBALS->loaded_file_name, ".vzt"))
	{
#if !defined _MSC_VER
	GLOBALS->stems_type = WAVE_ANNO_VZT;
	GLOBALS->aet_name = malloc_2(strlen(GLOBALS->loaded_file_name)+1);
	strcpy(GLOBALS->aet_name, GLOBALS->loaded_file_name);
#endif
        GLOBALS->loaded_file_type = VZT_FILE;
	vzt_main(GLOBALS->loaded_file_name, GLOBALS->skip_start, GLOBALS->skip_end);
	if(!GLOBALS->vzt_vzt_c_1)
		{
		fprintf(stderr, "GTKWAVE | Could not initialize '%s'%s.\n", GLOBALS->loaded_file_name, GLOBALS->vcd_jmp_buf ? "" : ", exiting");
		vcd_exit(255);
		}
	}
else if(suffix_check(GLOBALS->loaded_file_name, ".aet") || suffix_check(GLOBALS->loaded_file_name, ".ae2"))
	{
#if !defined _MSC_VER
	GLOBALS->stems_type = WAVE_ANNO_AE2;
	GLOBALS->aet_name = malloc_2(strlen(GLOBALS->loaded_file_name)+1);
	strcpy(GLOBALS->aet_name, GLOBALS->loaded_file_name);
#endif
        GLOBALS->loaded_file_type = AE2_FILE;
	ae2_main(GLOBALS->loaded_file_name, GLOBALS->skip_start, GLOBALS->skip_end, GLOBALS->indirect_fname);
	if(!GLOBALS->ae2)
		{
		fprintf(stderr, "GTKWAVE | Could not initialize '%s'%s.\n", GLOBALS->loaded_file_name, GLOBALS->vcd_jmp_buf ? "" : ", exiting");
		vcd_exit(255);
		}
	}
else if (suffix_check(GLOBALS->loaded_file_name, ".ghw") || suffix_check(GLOBALS->loaded_file_name, ".ghw.gz") ||
		suffix_check(GLOBALS->loaded_file_name, ".ghw.bz2"))
	{
          GLOBALS->loaded_file_type = GHW_FILE;
	  if(!ghw_main(GLOBALS->loaded_file_name))
		{
		/* error message printed in ghw_main() */
		vcd_exit(255);
		}
	}
else if (strlen(GLOBALS->loaded_file_name)>4)	/* case for .aet? type filenames */
	{
	char sufbuf[5];
	memcpy(sufbuf, GLOBALS->loaded_file_name+strlen(GLOBALS->loaded_file_name)-5, 4);
	sufbuf[4] = 0;
	if(!strcasecmp(sufbuf, ".aet"))	/* strncasecmp() in windows? */
		{
#if !defined _MSC_VER
		GLOBALS->stems_type = WAVE_ANNO_AE2;
		GLOBALS->aet_name = malloc_2(strlen(GLOBALS->loaded_file_name)+1);
		strcpy(GLOBALS->aet_name, GLOBALS->loaded_file_name);
#endif
                GLOBALS->loaded_file_type = AE2_FILE;
		ae2_main(GLOBALS->loaded_file_name, GLOBALS->skip_start, GLOBALS->skip_end, GLOBALS->indirect_fname);
		if(!GLOBALS->ae2)
			{
			fprintf(stderr, "GTKWAVE | Could not initialize '%s'%s.\n", GLOBALS->loaded_file_name, GLOBALS->vcd_jmp_buf ? "" : ", exiting");
			vcd_exit(255);
			}
		}
		else
		{
		goto load_vcd;
		}
	}
else	/* nothing else left so default to "something" */
	{
load_vcd:
#if !defined _MSC_VER && !defined __MINGW32__
	if(GLOBALS->optimize_vcd) {
                  GLOBALS->unoptimized_vcd_file_name = calloc_2(1,strlen(GLOBALS->loaded_file_name) + 1);
                  strcpy(GLOBALS->unoptimized_vcd_file_name, GLOBALS->loaded_file_name);  
                  optimize_vcd_file();
                  is_vcd = 0;
                  goto loader_check_head;
        }

#endif

#if !defined _MSC_VER 
	if(is_interactive)
		{
		GLOBALS->loaded_file_type = DUMPLESS_FILE;
		vcd_partial_main(GLOBALS->loaded_file_name);
		}
		else
#endif
		{
		if(is_legacy)
			{
			  GLOBALS->loaded_file_type = (strcmp(GLOBALS->loaded_file_name, "-vcd")) ? VCD_FILE : DUMPLESS_FILE;
			  vcd_main(GLOBALS->loaded_file_name);
			}
			else
			{
			  if(strcmp(GLOBALS->loaded_file_name, "-vcd"))
				{
				GLOBALS->loaded_file_type = VCD_RECODER_FILE;
				GLOBALS->use_fastload = is_fastload;
				}
				else
				{
				GLOBALS->loaded_file_type = DUMPLESS_FILE;
				GLOBALS->use_fastload = VCD_FSL_NONE;
				}
			  vcd_recoder_main(GLOBALS->loaded_file_name);
			}
		}
	}


if((GLOBALS->loaded_file_type != FST_FILE) || (!GLOBALS->fast_tree_sort))
	{
	GLOBALS->do_hier_compress = 0; /* for now, add more file formats in the future */
	}

/* deallocate the symbol hash table */
sym_hash_destroy(GLOBALS);

/* reset/initialize various markers and time values */
for(i=0;i<26;i++) GLOBALS->named_markers[i]=-1;	/* reset all named markers */

GLOBALS->tims.last=GLOBALS->max_time;
GLOBALS->tims.end=GLOBALS->tims.last;		/* until the configure_event of wavearea */
GLOBALS->tims.first=GLOBALS->tims.start=GLOBALS->tims.laststart=GLOBALS->min_time;
GLOBALS->tims.zoom=GLOBALS->tims.prevzoom=0;	/* 1 pixel/ns default */
GLOBALS->tims.marker=GLOBALS->tims.lmbcache=-1;	/* uninitialized at first */
GLOBALS->tims.baseline=-1;		/* middle button toggle marker */

if((wname)||(vcd_save_handle_cached)||(is_smartsave))
	{
	int wave_is_compressed;
        char *str = NULL;

	if(vcd_save_handle_cached)
		{
		wname=vcd_autosave_name;
		GLOBALS->do_initial_zoom_fit=1;
		}
	else
	if((!wname) /* && (is_smartsave) */)
		{
		char *pnt = wave_alloca(strlen(GLOBALS->loaded_file_name) + 1);
		char *pnt2;
		strcpy(pnt, GLOBALS->loaded_file_name);

	        if((strlen(pnt)>2)&&(!strcasecmp(pnt+strlen(pnt)-3,".gz")))
			{
			pnt[strlen(pnt)-3] = 0x00;
			}
		else if ((strlen(pnt)>3)&&(!strcasecmp(pnt+strlen(pnt)-4,".zip")))
			{
			pnt[strlen(pnt)-4] = 0x00;
			}

		pnt2 = pnt + strlen(pnt);
		if(pnt != pnt2)
			{
			do
				{
				if(*pnt2 == '.')
					{
					*pnt2 = 0x00;
					break;
					}
				} while(pnt2-- != pnt);
			}	

		wname = malloc_2(strlen(pnt) + 5);
		strcpy(wname, pnt);
		strcat(wname, ".sav");
		}

	if(((strlen(wname)>2)&&(!strcasecmp(wname+strlen(wname)-3,".gz")))||
	   ((strlen(wname)>3)&&(!strcasecmp(wname+strlen(wname)-4,".zip"))))
	        {
        	int dlen;
        	dlen=strlen(WAVE_DECOMPRESSOR);
	        str=wave_alloca(strlen(wname)+dlen+1);
	        strcpy(str,WAVE_DECOMPRESSOR);
	        strcpy(str+dlen,wname);
	        wave=popen(str,"r");
	        wave_is_compressed=~0;
	        }
	        else
	        {
	        wave=fopen(wname,"rb");
	        wave_is_compressed=0;

		GLOBALS->filesel_writesave = malloc_2(strlen(wname)+1); /* don't handle compressed files */
		strcpy(GLOBALS->filesel_writesave, wname);
	        }

	if(!wave)
	        {
	        fprintf(stderr, "** WARNING: Error opening .sav file '%s', skipping.\n",wname);
	        }	
	        else
	        {
	        char *iline;
		int s_ctx_iter;

                WAVE_STRACE_ITERATOR(s_ctx_iter)
                        {
                        GLOBALS->strace_ctx = &GLOBALS->strace_windows[GLOBALS->strace_current_window = s_ctx_iter];
                        GLOBALS->strace_ctx->shadow_encountered_parsewavline = 0;
			}

		if(GLOBALS->is_lx2)
			{
		        while((iline=fgetmalloc(wave)))
		                {
		                parsewavline_lx2(iline, NULL, 0);
				free_2(iline);
		                }

			switch(GLOBALS->is_lx2)
				{
				case LXT2_IS_LXT2: lx2_import_masked(); break;
				case LXT2_IS_AET2: ae2_import_masked(); break;
				case LXT2_IS_VZT:  vzt_import_masked(); break;
				case LXT2_IS_VLIST: vcd_import_masked(); break;
				case LXT2_IS_FST: fst_import_masked(); break;
				}

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
			        fprintf(stderr, "** WARNING: Error opening .sav file '%s', skipping.\n",wname);
				EnsureGroupsMatch();
				goto savefile_bail;
			        }	
			}

		GLOBALS->default_flags=TR_RJUSTIFY;
		GLOBALS->shift_timebase_default_for_add=LLDescriptor(0);
		GLOBALS->strace_current_window = 0; /* in case there are shadow traces */
	        while((iline=fgetmalloc(wave)))
	                {
	                parsewavline(iline, NULL, 0);
			GLOBALS->strace_ctx->shadow_encountered_parsewavline |= GLOBALS->strace_ctx->shadow_active;
			free_2(iline);
	                }
		GLOBALS->default_flags=TR_RJUSTIFY;
		GLOBALS->shift_timebase_default_for_add=LLDescriptor(0);

		if(wave_is_compressed) pclose(wave); else fclose(wave);

		EnsureGroupsMatch();

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
	        }
	}

savefile_bail:
GLOBALS->current_translate_file = 0;

if(fast_exit)
	{
	printf("Exiting early because of --exit request.\n");
	exit(0);
	}

if ((!GLOBALS->zoom_was_explicitly_set)&&
	((GLOBALS->tims.last-GLOBALS->tims.first)<=400)) GLOBALS->do_initial_zoom_fit=1;  /* force zoom on small traces */

calczoom(GLOBALS->tims.zoom);

if(!mainwindow_already_built)
{
#ifdef WAVE_USE_XID
if(!GLOBALS->socket_xid)
#endif
        {
	GLOBALS->mainwindow = gtk_window_new(GLOBALS->disable_window_manager ? GTK_WINDOW_POPUP : GTK_WINDOW_TOPLEVEL);
	gtk_window_set_title(GTK_WINDOW(GLOBALS->mainwindow), GLOBALS->winname);

	if((GLOBALS->initial_window_width>0)&&(GLOBALS->initial_window_height>0))
		{
		gtk_window_set_default_size(GTK_WINDOW (GLOBALS->mainwindow), GLOBALS->initial_window_width, GLOBALS->initial_window_height);
		}
		else
		{
		gtk_window_set_default_size(GTK_WINDOW(GLOBALS->mainwindow), GLOBALS->initial_window_x, GLOBALS->initial_window_y);
		}

	gtk_signal_connect(GTK_OBJECT(GLOBALS->mainwindow), "delete_event", 	/* formerly was "destroy" */GTK_SIGNAL_FUNC(file_quit_cmd_callback), "WM destroy");

	gtk_widget_show(GLOBALS->mainwindow);
	}
#ifdef WAVE_USE_XID
	else
	{
        GLOBALS->mainwindow = gtk_plug_new(GLOBALS->socket_xid);
        gtk_widget_show(GLOBALS->mainwindow);
                                
        gtk_signal_connect(GTK_OBJECT(GLOBALS->mainwindow), "destroy",   /* formerly was "destroy" */GTK_SIGNAL_FUNC(plug_destroy),"Plug destroy");
	}
#endif
}


make_pixmaps(GLOBALS->mainwindow);

#ifdef WAVE_USE_GTK2
if(GLOBALS->use_toolbutton_interface)
	{
	GtkWidget *tb;
	GtkWidget *stock;
	GtkStyle *style;
	int tb_pos;

	if(!mainwindow_already_built)
		{
		main_vbox = gtk_vbox_new(FALSE, 5);
		gtk_container_border_width(GTK_CONTAINER(main_vbox), 1);
		gtk_container_add(GTK_CONTAINER(GLOBALS->mainwindow), main_vbox);
		gtk_widget_show(main_vbox);
	
		if(!GLOBALS->disable_menus)
			{
			get_main_menu(GLOBALS->mainwindow, &menubar);
			gtk_widget_show(menubar);
	
			if(GLOBALS->force_toolbars)
				{
				toolhandle=gtk_handle_box_new();
				gtk_widget_show(toolhandle);
				gtk_container_add(GTK_CONTAINER(toolhandle), menubar);
				gtk_box_pack_start(GTK_BOX(main_vbox), toolhandle, FALSE, TRUE, 0);
				}
				else
				{
				gtk_box_pack_start(GTK_BOX(main_vbox), menubar, FALSE, TRUE, 0);
				}
			}
	
		whole_table = gtk_table_new (256, 16, FALSE);
	
		tb = gtk_toolbar_new();
		top_table = tb;		/* export this as our top widget rather than a table */

		gtk_toolbar_set_style(GTK_TOOLBAR(tb), GTK_TOOLBAR_ICONS);
		tb_pos = 0;

		if(GLOBALS->force_toolbars)
			{
			toolhandle=gtk_handle_box_new();
			gtk_widget_show(toolhandle);
			gtk_container_add(GTK_CONTAINER(toolhandle), top_table);
			}

		stock = gtk_toolbar_insert_stock(GTK_TOOLBAR(tb),
	                                         GTK_STOCK_CUT,
						 "Cut Traces",
						 NULL,
						 GTK_SIGNAL_FUNC(menu_cut_traces),
						 NULL,
						 tb_pos++);
		style = gtk_widget_get_style(stock);
		style->xthickness = style->ythickness = 0;
		gtk_widget_set_style (stock, style);
		gtk_widget_show(stock);

		stock = gtk_toolbar_insert_stock(GTK_TOOLBAR(tb),
	                                         GTK_STOCK_COPY,
						 "Copy Traces",
						 NULL,
						 GTK_SIGNAL_FUNC(menu_copy_traces),
						 NULL,
						 tb_pos++);
		style = gtk_widget_get_style(stock);
		style->xthickness = style->ythickness = 0;
		gtk_widget_set_style (stock, style);
		gtk_widget_show(stock);

		stock = gtk_toolbar_insert_stock(GTK_TOOLBAR(tb),
	                                         GTK_STOCK_PASTE,
						 "Paste Traces",
						 NULL,
						 GTK_SIGNAL_FUNC(menu_paste_traces),
						 NULL,
						 tb_pos++);
		style = gtk_widget_get_style(stock);
		style->xthickness = style->ythickness = 0;
		gtk_widget_set_style (stock, style);
		gtk_widget_show(stock);

		gtk_toolbar_insert_space(GTK_TOOLBAR(tb), tb_pos++);

		stock = gtk_toolbar_insert_stock(GTK_TOOLBAR(tb),
	                                         GTK_STOCK_ZOOM_FIT,
						 "Zoom Fit",
						 NULL,
						 GTK_SIGNAL_FUNC(service_zoom_fit),
						 NULL,
						 tb_pos++);
		style = gtk_widget_get_style(stock);
		style->xthickness = style->ythickness = 0;
		gtk_widget_set_style (stock, style);
		gtk_widget_show(stock);

		stock = gtk_toolbar_insert_stock(GTK_TOOLBAR(tb),
	                                         GTK_STOCK_ZOOM_IN,
						 "Zoom In",
						 NULL,
						 GTK_SIGNAL_FUNC(service_zoom_in),
						 NULL,
						 tb_pos++);
		style = gtk_widget_get_style(stock);
		style->xthickness = style->ythickness = 0;
		gtk_widget_set_style (stock, style);
		gtk_widget_show(stock);

		stock = gtk_toolbar_insert_stock(GTK_TOOLBAR(tb),
	                                         GTK_STOCK_ZOOM_OUT,
						 "Zoom Out",
						 NULL,
						 GTK_SIGNAL_FUNC(service_zoom_out),
						 NULL,
						 tb_pos++);
		style = gtk_widget_get_style(stock);
		style->xthickness = style->ythickness = 0;
		gtk_widget_set_style (stock, style);
		gtk_widget_show(stock);

		stock = gtk_toolbar_insert_stock(GTK_TOOLBAR(tb),
	                                         GTK_STOCK_UNDO,
						 "Zoom Undo",
						 NULL,
						 GTK_SIGNAL_FUNC(service_zoom_undo),
						 NULL,
						 tb_pos++);
		style = gtk_widget_get_style(stock);
		style->xthickness = style->ythickness = 0;
		gtk_widget_set_style (stock, style);
		gtk_widget_show(stock);

		stock = gtk_toolbar_insert_stock(GTK_TOOLBAR(tb),
	                                         GTK_STOCK_GOTO_FIRST,
						 "Zoom to Start",
						 NULL,
						 GTK_SIGNAL_FUNC(service_zoom_left),
						 NULL,
						 tb_pos++);
		style = gtk_widget_get_style(stock);
		style->xthickness = style->ythickness = 0;
		gtk_widget_set_style (stock, style);
		gtk_widget_show(stock);

		stock = gtk_toolbar_insert_stock(GTK_TOOLBAR(tb),
	                                         GTK_STOCK_GOTO_LAST,
						 "Zoom to End",
						 NULL,
						 GTK_SIGNAL_FUNC(service_zoom_right),
						 NULL,
						 tb_pos++);
		style = gtk_widget_get_style(stock);
		style->xthickness = style->ythickness = 0;
		gtk_widget_set_style (stock, style);
		gtk_widget_show(stock);

		gtk_toolbar_insert_space(GTK_TOOLBAR(tb), tb_pos++);

		stock = gtk_toolbar_insert_stock(GTK_TOOLBAR(tb),
	                                         GTK_STOCK_GO_BACK,
						 "Find Previous Edge",
						 NULL,
						 GTK_SIGNAL_FUNC(service_left_edge),
						 NULL,
						 tb_pos++);
		style = gtk_widget_get_style(stock);
		style->xthickness = style->ythickness = 0;
		gtk_widget_set_style (stock, style);
		gtk_widget_show(stock);

		stock = gtk_toolbar_insert_stock(GTK_TOOLBAR(tb),
	                                         GTK_STOCK_GO_FORWARD,
						 "Find Next Edge",
						 NULL,
						 GTK_SIGNAL_FUNC(service_right_edge),
						 NULL,
						 tb_pos++);
		style = gtk_widget_get_style(stock);
		style->xthickness = style->ythickness = 0;
		gtk_widget_set_style (stock, style);
		gtk_widget_show(stock);

		gtk_toolbar_insert_space(GTK_TOOLBAR(tb), tb_pos++);

		entry = create_entry_box();
		gtk_widget_show(entry);
		gtk_toolbar_insert_widget(GTK_TOOLBAR(tb),
                                          entry,
                                          NULL,
					  NULL,
					  tb_pos++);

		gtk_toolbar_insert_space(GTK_TOOLBAR(tb), tb_pos++);

		if((GLOBALS->loaded_file_type != DUMPLESS_FILE)&&(!GLOBALS->disable_menus))
			{
			stock = gtk_toolbar_insert_stock(GTK_TOOLBAR(tb),
	                                         GTK_STOCK_REFRESH,
						 "Reload",
						 NULL,
						 GTK_SIGNAL_FUNC(menu_reload_waveform),
						 NULL,
						 tb_pos++);
			style = gtk_widget_get_style(stock);
			style->xthickness = style->ythickness = 0;
			gtk_widget_set_style (stock, style);
			gtk_widget_show(stock);

			gtk_toolbar_insert_space(GTK_TOOLBAR(tb), tb_pos++);
			}

		timebox = create_time_box();
		gtk_widget_show (timebox);
		gtk_toolbar_insert_widget(GTK_TOOLBAR(tb),
                                          timebox,
                                          NULL,
					  NULL,
					  tb_pos++);

		GLOBALS->missing_file_toolbar = tb;
		if(GLOBALS->loaded_file_type == MISSING_FILE)
			{
			gtk_widget_set_sensitive(GLOBALS->missing_file_toolbar, FALSE);
			}
		} /* of ...if(mainwindow_already_built) */
	}
	else
#endif
	{
	if(!mainwindow_already_built)
		{
		main_vbox = gtk_vbox_new(FALSE, 5);
		gtk_container_border_width(GTK_CONTAINER(main_vbox), 1);
		gtk_container_add(GTK_CONTAINER(GLOBALS->mainwindow), main_vbox);
		gtk_widget_show(main_vbox);
	
		if(!GLOBALS->disable_menus)
			{
			get_main_menu(GLOBALS->mainwindow, &menubar);
			gtk_widget_show(menubar);
	
			if(GLOBALS->force_toolbars)
				{
				toolhandle=gtk_handle_box_new();
				gtk_widget_show(toolhandle);
				gtk_container_add(GTK_CONTAINER(toolhandle), menubar);
				gtk_box_pack_start(GTK_BOX(main_vbox), toolhandle, FALSE, TRUE, 0);
				}
				else
				{
				gtk_box_pack_start(GTK_BOX(main_vbox), menubar, FALSE, TRUE, 0);
				}
			}
	
		top_table = gtk_table_new (1, 284, FALSE);
	
		if(GLOBALS->force_toolbars)
			{
			toolhandle=gtk_handle_box_new();
			gtk_widget_show(toolhandle);
			gtk_container_add(GTK_CONTAINER(toolhandle), top_table);
			}

		whole_table = gtk_table_new (256, 16, FALSE);
	
		text1 = create_text ();
		gtk_table_attach (GTK_TABLE (top_table), text1, 0, 141, 0, 1,
		                      	GTK_FILL,
		                      	GTK_FILL | GTK_SHRINK, 0, 0);
		gtk_widget_set_usize(GTK_WIDGET(text1), 200, -1);
		gtk_widget_show (text1);

		dummy1=gtk_label_new("");
		gtk_table_attach (GTK_TABLE (top_table), dummy1, 141, 171, 0, 1,
		                      	GTK_FILL,
		                      	GTK_SHRINK, 0, 0);
		gtk_widget_show (dummy1);

		zoombuttons = create_zoom_buttons ();
		gtk_table_attach (GTK_TABLE (top_table), zoombuttons, 171, 173, 0, 1,
		                      	GTK_FILL,
		                      	GTK_SHRINK, 0, 0);
		gtk_widget_show (zoombuttons);

		if(!GLOBALS->use_scrollbar_only)
			{
			pagebuttons = create_page_buttons ();
			gtk_table_attach (GTK_TABLE (top_table), pagebuttons, 173, 174, 0, 1,
			                      	GTK_FILL,
			                      	GTK_SHRINK, 0, 0);
			gtk_widget_show (pagebuttons);
			fetchbuttons = create_fetch_buttons ();
			gtk_table_attach (GTK_TABLE (top_table), fetchbuttons, 174, 175, 0, 1,
			                      	GTK_FILL,
			                      	GTK_SHRINK, 0, 0);
			gtk_widget_show (fetchbuttons);
			discardbuttons = create_discard_buttons ();
			gtk_table_attach (GTK_TABLE (top_table), discardbuttons, 175, 176, 0, 1,
			                      	GTK_FILL,
			                      	GTK_SHRINK, 0, 0);
			gtk_widget_show (discardbuttons);
		
			shiftbuttons = create_shift_buttons ();
			gtk_table_attach (GTK_TABLE (top_table), shiftbuttons, 176, 177, 0, 1,
			                      	GTK_FILL,
			                      	GTK_SHRINK, 0, 0);
			gtk_widget_show (shiftbuttons);
			}

		edgebuttons = create_edge_buttons ();
		gtk_table_attach (GTK_TABLE (top_table), edgebuttons, 177, 178, 0, 1,
		                      	GTK_FILL,
		                      	GTK_SHRINK, 0, 0);
		gtk_widget_show (edgebuttons);
	

		dummy2=gtk_label_new("");
		gtk_table_attach (GTK_TABLE (top_table), dummy2, 178, 215, 0, 1,
		                      	GTK_FILL,
		                      	GTK_SHRINK, 0, 0);
		gtk_widget_show (dummy2);

		entry = create_entry_box();
		gtk_table_attach (GTK_TABLE (top_table), entry, 215, 216, 0, 1,
		                      	GTK_SHRINK,
		                      	GTK_SHRINK, 0, 0);
		gtk_widget_show(entry);

		timebox = create_time_box();
		gtk_table_attach (GTK_TABLE (top_table), timebox, 216, 284, 0, 1,
		                      	GTK_FILL | GTK_EXPAND,
		                      	GTK_FILL | GTK_EXPAND | GTK_SHRINK, 20, 0);
		gtk_widget_show (timebox);

		if((GLOBALS->loaded_file_type != DUMPLESS_FILE)&&(!GLOBALS->disable_menus))
			{
			GtkWidget *r_pixmap = gtk_pixmap_new(GLOBALS->redo_pixmap, GLOBALS->redo_mask);
			GtkWidget *main_vbox1;
			GtkWidget *table, *table2;
			GtkWidget *b1, *frame;
			GtkTooltips *tooltips;

			gtk_widget_show(r_pixmap);
		
			tooltips=gtk_tooltips_new_2();
			gtk_tooltips_set_delay_2(tooltips,1500);

			table = gtk_table_new (1, 1, FALSE);
		
			main_vbox1 = gtk_vbox_new (FALSE, 1);
			gtk_container_border_width (GTK_CONTAINER (main_vbox1), 1);
			gtk_container_add (GTK_CONTAINER (table), main_vbox1);
	
			frame = gtk_frame_new ("Reload ");
			gtk_box_pack_start (GTK_BOX (main_vbox1), frame, TRUE, TRUE, 0);
	
			gtk_widget_show (frame);
			gtk_widget_show (main_vbox1);

			table2 = gtk_table_new (2, 1, FALSE);
	
			b1 = gtk_button_new();
			gtk_container_add(GTK_CONTAINER(b1), r_pixmap);
			gtk_table_attach (GTK_TABLE (table2), b1, 0, 1, 0, 1,
			                        GTK_FILL | GTK_EXPAND,
			                        GTK_FILL | GTK_EXPAND | GTK_SHRINK, 1, 1);

			gtk_signal_connect_object (GTK_OBJECT (b1), "clicked", GTK_SIGNAL_FUNC(menu_reload_waveform), GTK_OBJECT (table2));
			gtk_tooltips_set_tip_2(tooltips, b1, "Reload waveform", NULL);
			gtk_widget_show(b1);
			gtk_container_add (GTK_CONTAINER (frame), table2);
			gtk_widget_show(table2);

			gtk_table_attach (GTK_TABLE (top_table), table, 284, 285, 0, 1,
		                      	0, 
		                      	0, 2, 0);
	
			gtk_widget_show (table);
			}
		} /* of ...if(mainwindow_already_built) */
	}


GLOBALS->wavewindow = create_wavewindow();
load_all_fonts(); /* must be done before create_signalwindow() */
gtk_widget_show(GLOBALS->wavewindow);
GLOBALS->signalwindow = create_signalwindow();

if(GLOBALS->do_resize_signals) 
                {
                int os;

		if(GLOBALS->initial_signal_window_width > GLOBALS->max_signal_name_pixel_width)
			{
			os=GLOBALS->initial_signal_window_width;
			}
			else
			{
	                os=GLOBALS->max_signal_name_pixel_width;
			}

                os=(os<48)?48:os;
                gtk_widget_set_usize(GTK_WIDGET(GLOBALS->signalwindow),
                                os+30, -1);
                }
else
	{
	if(GLOBALS->initial_signal_window_width)
		{
		int os;

		os=GLOBALS->initial_signal_window_width;
		os=(os<48)?48:os;
		gtk_widget_set_usize(GTK_WIDGET(GLOBALS->signalwindow), os+30, -1);
		}
	}

gtk_widget_show(GLOBALS->signalwindow);

#if GTK_CHECK_VERSION(2,4,0)
if((!GLOBALS->hide_sst)&&(GLOBALS->loaded_file_type != MISSING_FILE))
	{
	GLOBALS->toppanedwindow = gtk_hpaned_new();
	GLOBALS->sstpane = treeboxframe("SST", GTK_SIGNAL_FUNC(mkmenu_treesearch_cleanup));
	GLOBALS->expanderwindow = gtk_expander_new_with_mnemonic("_SST");
	gtk_expander_set_expanded(GTK_EXPANDER(GLOBALS->expanderwindow), (GLOBALS->sst_expanded==TRUE));
	gtk_container_add(GTK_CONTAINER(GLOBALS->expanderwindow), GLOBALS->sstpane);
	gtk_widget_show(GLOBALS->expanderwindow);
	}
#endif

panedwindow=gtk_hpaned_new();

#ifdef HAVE_PANED_PACK
if(GLOBALS->paned_pack_semantics)
	{
	gtk_paned_pack1(GTK_PANED(panedwindow), GLOBALS->signalwindow, 0, 0); 
	gtk_paned_pack2(GTK_PANED(panedwindow), GLOBALS->wavewindow, ~0, 0);
	}
	else
#endif
	{
	gtk_paned_add1(GTK_PANED(panedwindow), GLOBALS->signalwindow);
	gtk_paned_add2(GTK_PANED(panedwindow), GLOBALS->wavewindow);
	}

gtk_widget_show(panedwindow);

if(GLOBALS->dnd_sigview)
	{
	dnd_setup(GLOBALS->dnd_sigview, GLOBALS->signalarea, 1);
	}
	else
	{
	dnd_setup(NULL, GLOBALS->signalarea, 1);
	}
/* dnd_setup(GLOBALS->signalarea, GLOBALS->signalarea); */
dnd_setup(GLOBALS->signalarea, GLOBALS->wavearea, 1);

#if GTK_CHECK_VERSION(2,4,0)
if((!GLOBALS->hide_sst)&&(GLOBALS->loaded_file_type != MISSING_FILE))
	{
	gtk_paned_pack1(GTK_PANED(GLOBALS->toppanedwindow), GLOBALS->expanderwindow, 0, 0);
	gtk_paned_pack2(GTK_PANED(GLOBALS->toppanedwindow), panedwindow, ~0, 0);
	gtk_widget_show(GLOBALS->toppanedwindow);
	}
#endif

#if WAVE_USE_GTK2
if(GLOBALS->treeopen_chain_head)
	{
	struct string_chain_t *t = GLOBALS->treeopen_chain_head;
	struct string_chain_t *t2;
	while(t)
		{
		if(GLOBALS->ctree_main)
			{
			force_open_tree_node(t->str);
			}

		t2 = t->next;
		if(t->str) free_2(t->str);
		free_2(t);
		t = t2;
		}

	GLOBALS->treeopen_chain_head = GLOBALS->treeopen_chain_curr = NULL;
	}
#endif

if(!mainwindow_already_built)
	{
	gtk_widget_show(top_table);
	gtk_table_attach (GTK_TABLE (whole_table), GLOBALS->force_toolbars?toolhandle:top_table, 0, 16, 0, 1,
	                      	GTK_FILL | GTK_EXPAND,
	                      	GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0, 0);
	}


if(!GLOBALS->notebook)
	{
	GLOBALS->num_notebook_pages = 1;
	GLOBALS->this_context_page = 0;
	GLOBALS->contexts = calloc(1, sizeof(struct Globals ***)); /* calloc is deliberate! */
	*GLOBALS->contexts = calloc(1, sizeof(struct Globals **)); /* calloc is deliberate! */
	(*GLOBALS->contexts)[0] = GLOBALS;

	GLOBALS->dead_context = calloc(1, sizeof(struct Globals ***)); /* calloc is deliberate! */
	*GLOBALS->dead_context = calloc(1, sizeof(struct Globals **)); /* calloc is deliberate! */
	*(GLOBALS->dead_context)[0] = NULL;

	GLOBALS->notebook = gtk_notebook_new();
	gtk_notebook_set_tab_pos(GTK_NOTEBOOK(GLOBALS->notebook), GLOBALS->context_tabposition ? GTK_POS_LEFT : GTK_POS_TOP);

	gtk_widget_show(GLOBALS->notebook);
	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(GLOBALS->notebook), 0); /* hide for first time until next tabs */
	gtk_notebook_set_show_border(GTK_NOTEBOOK(GLOBALS->notebook), 0); /* hide for first time until next tabs */
	gtk_signal_connect(GTK_OBJECT(GLOBALS->notebook), "switch-page", GTK_SIGNAL_FUNC(switch_page), NULL);
	}
	else
	{
	unsigned int ix;

	GLOBALS->this_context_page = GLOBALS->num_notebook_pages;
	GLOBALS->num_notebook_pages++;
	GLOBALS->num_notebook_pages_cumulative++; /* this never decreases, acts as an incrementing flipper id for side tabs */
	*GLOBALS->contexts = realloc(*GLOBALS->contexts, GLOBALS->num_notebook_pages * sizeof(struct Globals *)); /* realloc is deliberate! */
	(*GLOBALS->contexts)[GLOBALS->this_context_page] = GLOBALS;

	for(ix=0;ix<GLOBALS->num_notebook_pages;ix++)
		{
		(*GLOBALS->contexts)[ix]->num_notebook_pages = GLOBALS->num_notebook_pages;
		(*GLOBALS->contexts)[ix]->num_notebook_pages_cumulative = GLOBALS->num_notebook_pages_cumulative;
		(*GLOBALS->contexts)[ix]->dead_context = (*GLOBALS->contexts)[0]->dead_context;	/* mirroring this is OK as page 0 always has value! */
		}

	gtk_notebook_set_show_tabs(GTK_NOTEBOOK(GLOBALS->notebook), ~0); /* then appear */
	gtk_notebook_set_show_border(GTK_NOTEBOOK(GLOBALS->notebook), ~0); /* then appear */
	gtk_notebook_set_scrollable(GTK_NOTEBOOK(GLOBALS->notebook), ~0);
	}

if(!GLOBALS->context_tabposition)
	{
	gtk_notebook_append_page(GTK_NOTEBOOK(GLOBALS->notebook), GLOBALS->toppanedwindow ? GLOBALS->toppanedwindow : panedwindow, 
		gtk_label_new(GLOBALS->loaded_file_name));
	}
	else
	{
	char buf[40];
	
	sprintf(buf, "%d", GLOBALS->num_notebook_pages_cumulative);

	gtk_notebook_append_page(GTK_NOTEBOOK(GLOBALS->notebook), GLOBALS->toppanedwindow ? GLOBALS->toppanedwindow : panedwindow, 
		gtk_label_new(buf));
	}

if(mainwindow_already_built)
	{
	gtk_notebook_set_current_page(GTK_NOTEBOOK(GLOBALS->notebook), GLOBALS->this_context_page);
	return(0);
	}

gtk_table_attach (GTK_TABLE (whole_table), GLOBALS->notebook, 0, 16, 1, 256,
                      	GTK_FILL | GTK_EXPAND,
                      	GTK_FILL | GTK_EXPAND | GTK_SHRINK, 0, 0);
gtk_widget_show(whole_table);
gtk_container_add (GTK_CONTAINER (main_vbox), whole_table);

if(GLOBALS->tims.marker != -1)
	{
	if(GLOBALS->tims.marker<GLOBALS->tims.first) GLOBALS->tims.marker=GLOBALS->tims.first;
	}
update_markertime(GLOBALS->tims.marker);

set_window_xypos(GLOBALS->initial_window_xpos, GLOBALS->initial_window_ypos);
GLOBALS->xy_ignore_main_c_1 = 1;

if(GLOBALS->logfile) 
	{
	struct logfile_chain *lprev;
	char buf[50];
	int which = 1;
	while(GLOBALS->logfile)
		{
		sprintf(buf, "Logfile viewer [%d]", which++);
		logbox(buf, 480, GLOBALS->logfile->name);
		lprev = GLOBALS->logfile;
		GLOBALS->logfile = GLOBALS->logfile->next;
		free_2(lprev->name);
		free_2(lprev);
		}
	}

activate_stems_reader(GLOBALS->stems_name);

while (gtk_events_pending()) gtk_main_iteration();

if(1)	/* here in order to calculate window manager delta if present... window is completely rendered by here */
	{
	int dummy_x, dummy_y;
	get_window_xypos(&dummy_x, &dummy_y);
	}

init_busy();

if(scriptfile)
	{
	execute_script(scriptfile, 1); /* deallocate the name in the script because context might swap out from under us! */
	scriptfile=NULL;
	}

#if !defined(_MSC_VER) && !defined(__MINGW32__)
if(GLOBALS->rpc_id)
	{
	GLOBALS->rpc_ctx = shmat(GLOBALS->rpc_id, NULL, 0);
	if(GLOBALS->rpc_ctx)
		{
                if(memcmp(&GLOBALS->rpc_ctx->matchword, RPC_MATCHWORD, 4))
                        {
                        fprintf(stderr, "RPC shared memory ID matchword mismatch, exiting.\n");
                        exit(255);
                        }
		}
		else
		{
                fprintf(stderr, "Not a valid RPC shared memory ID for remote operation, exiting.\n");
                exit(255);
		}
	}

#endif

#if !defined _MSC_VER
if(GLOBALS->dual_attach_id_main_c_1)
	{
	fprintf(stderr, "GTKWAVE | Attaching %08X as dual head session %d\n", GLOBALS->dual_attach_id_main_c_1, GLOBALS->dual_id);

#ifdef __MINGW32__
		{
		HANDLE hMapFile;
		char mapName[257];

		sprintf(mapName, "twinwave%d", GLOBALS->dual_attach_id_main_c_1);
		hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, mapName);
		if(hMapFile == NULL)
		        {
		        fprintf(stderr, "Could not attach shared memory map name '%s', exiting.\n", mapName);
		        exit(255);
		        }
		GLOBALS->dual_ctx = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 2 * sizeof(struct gtkwave_dual_ipc_t));
		if(GLOBALS->dual_ctx == NULL)
		        {
		        fprintf(stderr, "Could not map view of file '%s', exiting.\n", mapName);
		        exit(255);
		        }
		}
#else
	GLOBALS->dual_ctx = shmat(GLOBALS->dual_attach_id_main_c_1, NULL, 0);
#endif
	if(GLOBALS->dual_ctx)
		{
		if(memcmp(GLOBALS->dual_ctx[GLOBALS->dual_id].matchword, DUAL_MATCHWORD, 4))
			{
			fprintf(stderr, "Not a valid shared memory ID for dual head operation, exiting.\n");
			exit(255);
			}

		GLOBALS->dual_ctx[GLOBALS->dual_id].viewer_is_initialized = 1;
		for(;;)
			{
		        GtkAdjustment *hadj;
		        TimeType pageinc, gt;
#ifndef __MINGW32__
			struct timeval tv;
#endif
			if(GLOBALS->dual_ctx[1-GLOBALS->dual_id].use_new_times)
				{
				GLOBALS->dual_race_lock = 1;

			        gt = GLOBALS->dual_ctx[GLOBALS->dual_id].left_margin_time = GLOBALS->dual_ctx[1-GLOBALS->dual_id].left_margin_time;

				GLOBALS->dual_ctx[GLOBALS->dual_id].marker = GLOBALS->dual_ctx[1-GLOBALS->dual_id].marker;
				GLOBALS->dual_ctx[GLOBALS->dual_id].baseline = GLOBALS->dual_ctx[1-GLOBALS->dual_id].baseline;
				GLOBALS->dual_ctx[GLOBALS->dual_id].zoom = GLOBALS->dual_ctx[1-GLOBALS->dual_id].zoom;
				GLOBALS->dual_ctx[1-GLOBALS->dual_id].use_new_times = 0;
				GLOBALS->dual_ctx[GLOBALS->dual_id].use_new_times = 0;

				if(GLOBALS->dual_ctx[GLOBALS->dual_id].baseline != GLOBALS->tims.baseline)
					{
					if((GLOBALS->tims.marker != -1) && (GLOBALS->dual_ctx[GLOBALS->dual_id].marker == -1))
						{
				        	Trptr t;
  
        					for(t=GLOBALS->traces.first;t;t=t->t_next)
                					{
                					if(t->asciivalue) { free_2(t->asciivalue); t->asciivalue=NULL; }
                					}

	        				for(t=GLOBALS->traces.buffer;t;t=t->t_next)
	                				{
	                				if(t->asciivalue) { free_2(t->asciivalue); t->asciivalue=NULL; }
	                				}
						}

					GLOBALS->tims.marker = GLOBALS->dual_ctx[GLOBALS->dual_id].marker;
					GLOBALS->tims.baseline = GLOBALS->dual_ctx[GLOBALS->dual_id].baseline;
					update_basetime(GLOBALS->tims.baseline);
					update_markertime(GLOBALS->tims.marker);
					GLOBALS->signalwindow_width_dirty = 1;
					button_press_release_common();
					}
				else
				if(GLOBALS->dual_ctx[GLOBALS->dual_id].marker != GLOBALS->tims.marker)
					{
					if((GLOBALS->tims.marker != -1) && (GLOBALS->dual_ctx[GLOBALS->dual_id].marker == -1))
						{
				        	Trptr t;
  
        					for(t=GLOBALS->traces.first;t;t=t->t_next)
                					{
                					if(t->asciivalue) { free_2(t->asciivalue); t->asciivalue=NULL; }
                					}

	        				for(t=GLOBALS->traces.buffer;t;t=t->t_next)
	                				{
	                				if(t->asciivalue) { free_2(t->asciivalue); t->asciivalue=NULL; }
	                				}
						}

					GLOBALS->tims.marker = GLOBALS->dual_ctx[GLOBALS->dual_id].marker;
					update_markertime(GLOBALS->tims.marker);
					GLOBALS->signalwindow_width_dirty = 1;
					button_press_release_common();
					}

				GLOBALS->tims.prevzoom=GLOBALS->tims.zoom;
				GLOBALS->tims.zoom=GLOBALS->dual_ctx[GLOBALS->dual_id].zoom;

			        if(gt<GLOBALS->tims.first) gt=GLOBALS->tims.first;
			        else if(gt>GLOBALS->tims.last) gt=GLOBALS->tims.last;

			        hadj=GTK_ADJUSTMENT(GLOBALS->wave_hslider);
			        hadj->value=gt;
        
			        pageinc=(TimeType)(((gdouble)GLOBALS->wavewidth)*GLOBALS->nspx);
			        if(gt<(GLOBALS->tims.last-pageinc+1))
			                GLOBALS->tims.timecache=gt;
			                else
			                {
			                GLOBALS->tims.timecache=GLOBALS->tims.last-pageinc+1;
			                if(GLOBALS->tims.timecache<GLOBALS->tims.first) GLOBALS->tims.timecache=GLOBALS->tims.first;
			                }

			        time_update();
				}

			if(is_interactive)
				{
				kick_partial_vcd();
				}
				else
				{
				while (gtk_events_pending()) gtk_main_iteration();
				}

			GLOBALS->dual_race_lock = 0;

#ifdef __MINGW32__
			Sleep(1000 / 25);
#else
			tv.tv_sec = 0;
       			tv.tv_usec = 1000000 / 25;
			select(0, NULL, NULL, NULL, &tv);
#endif
			}
		}
		else
		{
		fprintf(stderr, "Could not attach to %08X, exiting.\n", GLOBALS->dual_attach_id_main_c_1);
		exit(255);
		}
	}
else
#endif
if(is_interactive)
	{
	for(;;) kick_partial_vcd();
	}
	else
	{
#if defined(HAVE_LIBTCL)
	if(is_wish)
	  {
	  char* argv_mod[1];

	  set_globals_interp(argv[0], 1);
	  addPidToExecutableName(1, argv, argv_mod);
	  Tk_MainEx(1, argv_mod, gtkwaveInterpreterInit, GLOBALS->interp);

	  /* note: for(kk=0;kk<argc;kk++) { free_2(argv_mod[kk]); } can't really be done here, doesn't matter anyway as context free will get it */
	  }
	  else
	  {
	  gtk_main();
          }
#else
	  gtk_main();
#endif
	}

return(0);
}

void
get_window_size (int *x, int *y)
{
#ifdef WAVE_USE_GTK2
  gtk_window_get_size (GTK_WINDOW (GLOBALS->mainwindow), x, y);
#else
  *x = GLOBALS->initial_window_x;
  *y = GLOBALS->initial_window_y;
#endif
}

void
set_window_size (int x, int y)
{
  if(GLOBALS->block_xy_update)
    {
      return;
    }

  if (GLOBALS->mainwindow == NULL)
    {
      GLOBALS->initial_window_width = x;
      GLOBALS->initial_window_height = y;
    }
  else
    {
#ifdef WAVE_USE_XID
      if(!GLOBALS->socket_xid)
#endif
	{
      	gtk_window_set_default_size(GTK_WINDOW (GLOBALS->mainwindow), x, y);
	}
    }
}


void 
get_window_xypos(int *root_x, int *root_y)
{
if(!GLOBALS->mainwindow) return;

#ifdef WAVE_USE_GTK2
gtk_window_get_position(GTK_WINDOW(GLOBALS->mainwindow), root_x, root_y);

if(!GLOBALS->initial_window_get_valid)
	{
	if((GLOBALS->mainwindow->window))
		{
		GLOBALS->initial_window_get_valid = 1;
		GLOBALS->initial_window_xpos_get = *root_x;
		GLOBALS->initial_window_ypos_get = *root_y;

		GLOBALS->xpos_delta = GLOBALS->initial_window_xpos_set - GLOBALS->initial_window_xpos_get;
		GLOBALS->ypos_delta = GLOBALS->initial_window_ypos_set - GLOBALS->initial_window_ypos_get;
		}
	}
#else
*root_x = *root_y = -1;
#endif
}

void 
set_window_xypos(int root_x, int root_y)
{
if(GLOBALS->xy_ignore_main_c_1) return;

#if !defined __MINGW32__ && !defined _MSC_VER
GLOBALS->initial_window_xpos = root_x;
GLOBALS->initial_window_ypos = root_y;

if(!GLOBALS->mainwindow) return;
if((GLOBALS->initial_window_xpos>=0)||(GLOBALS->initial_window_ypos>=0))
	{
	if (GLOBALS->initial_window_xpos<0) { GLOBALS->initial_window_xpos = 0; }
	if (GLOBALS->initial_window_ypos<0) { GLOBALS->initial_window_ypos = 0; }
#ifdef WAVE_USE_GTK2
	gtk_window_move(GTK_WINDOW(GLOBALS->mainwindow), GLOBALS->initial_window_xpos, GLOBALS->initial_window_ypos);
#else
	gtk_window_reposition(GTK_WINDOW(GLOBALS->mainwindow), GLOBALS->initial_window_xpos, GLOBALS->initial_window_ypos);
#endif

	if(!GLOBALS->initial_window_set_valid)
		{
		GLOBALS->initial_window_set_valid = 1;
		GLOBALS->initial_window_xpos_set = GLOBALS->initial_window_xpos;
		GLOBALS->initial_window_ypos_set = GLOBALS->initial_window_ypos;
		}
	}
#endif
}


/*
 * bring up stems browser
 */
#if !defined _MSC_VER
int stems_are_active(void)
{
#ifdef __MINGW32__
if(GLOBALS->anno_ctx && GLOBALS->anno_ctx->browser_process)
	{
	/* nothing */
	return(1);
	}
#else
if(GLOBALS->anno_ctx && GLOBALS->anno_ctx->browser_process)
	{
	int mystat =0;
	pid_t pid = waitpid(GLOBALS->anno_ctx->browser_process, &mystat, WNOHANG);
	if(!pid)
		{
		status_text("Stems reader already active.\n");
		return(1);
		}
		else
		{
		shmdt((void *)GLOBALS->anno_ctx);
		GLOBALS->anno_ctx = NULL;
		}
	}
#endif
return(0);
}
#endif


void activate_stems_reader(char *stems_name)
{
#if !defined _MSC_VER

#ifdef __CYGWIN__
/* ajb : ok static as this is a one-time warning message... */
static int cyg_called = 0;
#endif

if(!stems_name) return;

#ifdef __CYGWIN__
if(GLOBALS->stems_type != WAVE_ANNO_NONE)
	{
	if(!cyg_called)
		{
		char *cygserver_env = getenv("CYGWIN");
		gboolean found = cygserver_env && (strstr(cygserver_env, "server") != NULL);

		if(!found)
			{
			fprintf(stderr, "GTKWAVE | =================================================================\n");
			fprintf(stderr, "GTKWAVE | If the viewer crashes with a Bad system call error,\n");
			fprintf(stderr, "GTKWAVE | make sure that Cygserver is enabled.\n");
			fprintf(stderr, "GTKWAVE | The Cygserver services are used by Cygwin applications only\n");
			fprintf(stderr, "GTKWAVE | if you set the environment variable CYGWIN to contain the\n");
			fprintf(stderr, "GTKWAVE | string \"server\". You must do this before starting this program.\n");
			fprintf(stderr, "GTKWAVE |\n");
			fprintf(stderr, "GTKWAVE | If this still does not work, you may have to enable the cygserver\n");
			fprintf(stderr, "GTKWAVE | by entering \"cygserver-config\" and answering \"yes\" followed by\n");
			fprintf(stderr, "GTKWAVE | \"net start cygserver\".\n");
			fprintf(stderr, "GTKWAVE | =================================================================\n");
			}
		cyg_called = 1;
		}
	}
#endif

if(GLOBALS->stems_type != WAVE_ANNO_NONE)
	{
#ifdef __MINGW32__
	int shmid = getpid();
	char mapName[257];
	HANDLE hMapFile;
        STARTUPINFO si;
        PROCESS_INFORMATION pi;
        BOOL rc;

        memset(&si, 0, sizeof(STARTUPINFO));
        memset(&pi, 0, sizeof(PROCESS_INFORMATION));
	si.cb = sizeof(si);

	sprintf(mapName, "rtlbrowse%d", shmid);
	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(struct gtkwave_annotate_ipc_t), mapName);
	if(hMapFile != NULL)
	        {
	        GLOBALS->anno_ctx = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(struct gtkwave_annotate_ipc_t));
		if(GLOBALS->anno_ctx)
			{
			char mylist[257];

                        sprintf(mylist, "rtlbrowse.exe %08x", shmid);

			memset(GLOBALS->anno_ctx, 0, sizeof(struct gtkwave_annotate_ipc_t));

			memcpy(GLOBALS->anno_ctx->matchword, WAVE_MATCHWORD, 4);
			GLOBALS->anno_ctx->aet_type = GLOBALS->stems_type;			
			strcpy(GLOBALS->anno_ctx->aet_name, GLOBALS->aet_name);
			strcpy(GLOBALS->anno_ctx->stems_name, stems_name);

			update_markertime(GLOBALS->tims.marker);

                        rc = CreateProcess(
                                        "rtlbrowse.exe",
                                        mylist,
                                        NULL,
                                        NULL,
                                        FALSE,
                                        0,
                                        NULL,
                                        NULL,
                                        &si,
                                        &pi);
                                        
                        if(!rc)  
                        	{
				UnmapViewOfFile(GLOBALS->anno_ctx);
				CloseHandle(hMapFile);
				GLOBALS->anno_ctx = NULL;
				GLOBALS->stems_type = WAVE_ANNO_NONE;
                                }
				else
				{
				GLOBALS->anno_ctx->browser_process = pi.hProcess;
				}
			}
			else
			{
			CloseHandle(hMapFile);
			GLOBALS->stems_type = WAVE_ANNO_NONE;
			}
		}
#else
	int shmid = shmget(0, sizeof(struct gtkwave_annotate_ipc_t), IPC_CREAT | 0600 );
	if(shmid >=0)
		{
		struct shmid_ds ds;

		GLOBALS->anno_ctx = shmat(shmid, NULL, 0);
		if(GLOBALS->anno_ctx)
			{
			pid_t pid;

			memset(GLOBALS->anno_ctx, 0, sizeof(struct gtkwave_annotate_ipc_t));

			memcpy(GLOBALS->anno_ctx->matchword, WAVE_MATCHWORD, 4);
			GLOBALS->anno_ctx->aet_type = GLOBALS->stems_type;			
			strcpy(GLOBALS->anno_ctx->aet_name, GLOBALS->aet_name);
			strcpy(GLOBALS->anno_ctx->stems_name, stems_name);

			GLOBALS->anno_ctx->gtkwave_process = getpid();			
			update_markertime(GLOBALS->tims.marker);

#ifdef __linux__
			shmctl(shmid, IPC_RMID, &ds); /* mark for destroy */
#endif

		        pid=fork();

		        if(((int)pid) < 0) 
				{ 
				/* can't do anything about this */
				}
				else
				{
			        if(pid) /* parent==original server_pid */
			                {
#ifndef __CYGWIN__
					static int kill_installed = 0;

					if(!kill_installed)
						{
						kill_installed = 1;
						atexit(kill_stems_browser);
						}
#endif
					GLOBALS->anno_ctx->browser_process = pid;
#ifndef __linux__
					sleep(2);
					shmctl(shmid, IPC_RMID, &ds); /* mark for destroy */
#endif
			                }
					else
					{
					char buf[64];
					sprintf(buf, "%08x", shmid);
				        execlp("rtlbrowse", "rtlbrowse", buf, NULL);
				        exit(0);        /* control never gets here if successful */
					}
				}
			}
			else
			{
			shmctl(shmid, IPC_RMID, &ds); /* actually destroy */
			GLOBALS->stems_type = WAVE_ANNO_NONE;
			}
		}
#endif
	}
	else
	{
	fprintf(stderr, "GTKWAVE | Unsupported dumpfile type for rtlbrowse.\n");
	}
#endif
}


#if !defined _MSC_VER && !defined __MINGW32__
void optimize_vcd_file(void) {
  if(!strcmp("-vcd", GLOBALS->unoptimized_vcd_file_name)) {        
#ifdef __CYGWIN__
    char *buf = strdup_2("vcd2fst -- - vcd.fst");
    system(buf);
    free_2(buf);
    GLOBALS->loaded_file_name = strdup_2("vcd.fst");
    GLOBALS->is_optimized_stdin_vcd = 1;
#else
    pid_t pid;
    char *buf = malloc_2(strlen("vcd") + 4 + 1);
    sprintf(buf, "%s.fst", "vcd");
    pid = fork();
    if(((int)pid) < 0) {
      /* can't do anything about this */
    }
    else {
      if(pid) {
        int mystat;
        int rc = waitpid(pid, &mystat, 0);
	if(rc > 0) {
	  free_2(GLOBALS->loaded_file_name);
	  GLOBALS->loaded_file_name = buf;
	  GLOBALS->is_optimized_stdin_vcd = 1;
	}					
      }
      else {
        execlp("vcd2fst", "vcd2fst", "--", "-", buf, NULL);
	exit(255);
      }
    }
#endif
  }
  else {
#ifdef __CYGWIN__
    char *buf = malloc_2(9 + (strlen(GLOBALS->unoptimized_vcd_file_name) + 1) + (strlen(GLOBALS->unoptimized_vcd_file_name) + 4 + 1));
    sprintf(buf, "vcd2fst %s %s.fst", GLOBALS->unoptimized_vcd_file_name, GLOBALS->unoptimized_vcd_file_name);
    system(buf);
    free_2(buf);
    buf = malloc_2(strlen(GLOBALS->unoptimized_vcd_file_name) + 4 + 1);
    sprintf(buf, "%s.fst", GLOBALS->unoptimized_vcd_file_name);
    GLOBALS->loaded_file_name = buf;
#else
    pid_t pid;
    char *buf = malloc_2(strlen(GLOBALS->unoptimized_vcd_file_name) + 4 + 1);
    sprintf(buf, "%s.fst", GLOBALS->unoptimized_vcd_file_name);
    pid = fork(); 
    if(((int)pid) < 0) {
      /* can't do anything about this */
    }
    else {
      if(pid) {
	int mystat;
	int rc = waitpid(pid, &mystat, 0);
        if(rc > 0) {
          free_2(GLOBALS->loaded_file_name);
	  GLOBALS->loaded_file_name = buf;
        }					
      }
      else {
        execlp("vcd2fst", "vcd2fst", GLOBALS->unoptimized_vcd_file_name, buf, NULL);
	exit(255);
      }
    }
#endif
  }	
}
#endif

/*
 * $Id: main.c,v 1.104 2011/01/13 17:20:39 gtkwave Exp $
 * $Log: main.c,v $
 * Revision 1.104  2011/01/13 17:20:39  gtkwave
 * rewrote hierarchy / facility packing code
 *
 * Revision 1.103  2010/11/07 16:20:33  gtkwave
 * make fst the default file format for --optimize action in gtkwave
 *
 * Revision 1.102  2010/10/26 17:37:35  gtkwave
 * added initial_signal_window_width rc variable
 *
 * Revision 1.101  2010/10/06 20:15:51  gtkwave
 * preliminary version of RPC mechanism
 *
 * Revision 1.100  2010/09/14 17:11:56  gtkwave
 * added warning for unimplemented flags (for some systems)
 *
 * Revision 1.99  2010/07/12 20:51:42  gtkwave
 * free to non-malloc'd address fix
 *
 * Revision 1.98  2010/03/31 15:42:47  gtkwave
 * added preliminary transaction filter support
 *
 * Revision 1.97  2010/03/15 15:57:28  gtkwave
 * only allocate hash when necessary
 *
 * Revision 1.96  2010/03/15 03:14:53  gtkwave
 * deallocated symbol hash table after no longer needed
 *
 * Revision 1.95  2010/03/11 16:32:38  gtkwave
 * fix to add .lxt2 suffix recognition
 *
 * Revision 1.94  2010/03/10 18:14:13  gtkwave
 * Toggle Trace Hier fix (cache old value for more usability)
 *
 * Revision 1.93  2010/03/09 21:20:00  gtkwave
 * crash fix for using ->next on a struct after free
 *
 * Revision 1.92  2010/01/22 16:51:21  gtkwave
 * fixes to ensure WAVE_NUM_STRACE_WINDOWS is more maintainable
 *
 * Revision 1.91  2010/01/22 02:10:49  gtkwave
 * added second pattern search capability
 *
 * Revision 1.90  2009/12/24 20:55:27  gtkwave
 * warnings cleanups
 *
 * Revision 1.89  2009/12/16 17:41:21  gtkwave
 * code + documentation cleanups for --script
 *
 * Revision 1.88  2009/12/12 06:06:39  gtkwave
 * added error for --wish if no tcl present
 *
 * Revision 1.87  2009/12/12 01:12:57  gtkwave
 * tcl cross-platform fixes
 *
 * Revision 1.86  2009/12/11 23:41:07  gtkwave
 * cygwin fixes for optimized vcd
 *
 * Revision 1.85  2009/11/17 00:14:40  gtkwave
 * removed redundant is_wish check
 *
 * Revision 1.84  2009/11/11 16:30:57  gtkwave
 * changed tcl library ordering, no tk unless --wish
 *
 * Revision 1.83  2009/11/09 20:09:12  gtkwave
 * made --vcd and --wish mutually exclusive
 *
 * Revision 1.82  2009/11/06 04:15:17  gtkwave
 * gtk+-1.2 compile fixes
 *
 * Revision 1.81  2009/11/05 23:11:09  gtkwave
 * added EnsureGroupsMatch()
 *
 * Revision 1.80  2009/10/07 16:59:07  gtkwave
 * move Tcl_CreateInterp to tcl_helper.c to make stubify easier
 *
 * Revision 1.79  2009/09/22 13:51:13  gtkwave
 * warnings fixes
 *
 * Revision 1.78  2009/09/15 06:21:07  gtkwave
 * simpod-style tcl command reintegration
 *
 * Revision 1.77  2009/09/14 03:00:08  gtkwave
 * bluespec code integration
 *
 * Revision 1.76  2009/08/16 04:41:41  gtkwave
 * fix -d flag across reloads and new tab openings
 *
 * Revision 1.75  2009/08/06 20:03:31  gtkwave
 * warnings fixes
 *
 * Revision 1.74  2009/07/03 18:48:33  gtkwave
 * fst read compatibility fixes for mingw
 *
 * Revision 1.73  2009/06/07 08:40:44  gtkwave
 * adding FST support
 *
 * Revision 1.72  2009/04/24 20:00:03  gtkwave
 * rtlbrowse exit kill mods
 *
 * Revision 1.71  2009/04/24 19:47:15  gtkwave
 * multiple rtlbrowse session kill on gtkwave exit with mult tabs active
 *
 * Revision 1.70  2009/04/24 04:24:22  gtkwave
 * reload and cygwin fixes for rtlbrowse
 *
 * Revision 1.69  2009/04/23 21:57:53  gtkwave
 * added mingw support for rtlbrowse
 *
 * Revision 1.68  2009/04/23 19:38:50  gtkwave
 * add mingw support to twinwave
 *
 * Revision 1.67  2009/04/23 04:57:38  gtkwave
 * ported shmidcat and partial vcd loader function to mingw
 *
 * Revision 1.66  2009/04/11 21:57:53  gtkwave
 * changed exit() to vcd_exit() in some instances for tab loader hardening
 *
 * Revision 1.65  2009/04/09 22:17:21  gtkwave
 * install_keypress_handler() fixes
 *
 * Revision 1.64  2009/03/27 05:08:56  gtkwave
 * disable splash screen for now on empty gui due to undetermine conflicts
 *
 * Revision 1.63  2009/03/27 04:58:14  gtkwave
 * enable splash screen during empty gui handling / load dumpfile
 *
 * Revision 1.62  2009/03/27 04:38:05  gtkwave
 * working on ergonomics of drag and drop into an empty gui
 *
 * Revision 1.61  2009/03/27 00:11:01  gtkwave
 * minor change in -h, --help text when gui enabled
 *
 * Revision 1.60  2009/03/26 23:53:22  gtkwave
 * added disable_empty_gui rc variable which reverts to old behavior
 *
 * Revision 1.59  2009/03/26 20:57:41  gtkwave
 * added MISSING_FILE support for bringing up gtkwave without a dumpfile
 *
 * Revision 1.58  2009/03/09 06:23:37  gtkwave
 * cygwin tcl compatibility fix
 *
 * Revision 1.57  2009/03/05 16:17:56  gtkwave
 * added fastload option
 *
 * Revision 1.56  2009/02/23 18:29:50  gtkwave
 * add more information on init failure for mac users
 *
 * Revision 1.55  2009/02/16 17:16:05  gtkwave
 * extload error hardening and recovery
 *
 * Revision 1.54  2009/01/28 20:06:20  gtkwave
 * changed -ignore to a second --
 *
 * Revision 1.53  2009/01/28 16:28:58  gtkwave
 * added -- -ignore passthrough processing for argv list in main.c
 *
 * Revision 1.52  2009/01/27 07:04:31  gtkwave
 * added extload external process loader capability
 *
 * Revision 1.51  2009/01/19 07:13:11  gtkwave
 * examine CYGWIN env var for "server" before printing warning for rtlbrowse
 *
 * Revision 1.50  2009/01/13 23:09:12  gtkwave
 * added copy traces button to toolbar in gtkwave
 *
 * Revision 1.49  2009/01/12 16:01:45  gtkwave
 * fixed OUTPUT_GETOPT for mingw
 *
 * Revision 1.48  2009/01/12 04:17:39  gtkwave
 * added dynamic zoom for end for partial vcd
 *
 * Revision 1.47  2009/01/08 16:58:22  gtkwave
 * added --output command line flag
 *
 * Revision 1.46  2009/01/02 06:24:28  gtkwave
 * bumped copyright to 2009
 *
 * Revision 1.45  2009/01/02 06:12:48  gtkwave
 * needed check for TCL presence for GLOBALS->interp cloning
 *
 * Revision 1.44  2009/01/02 06:11:00  gtkwave
 * needed to clone GLOBALS->interp from one instance to the next in maketabs
 *
 * Revision 1.43  2009/01/02 06:01:51  gtkwave
 * added getArgv for tcl commands
 *
 * Revision 1.42  2008/12/28 03:03:33  gtkwave
 * Added scale_to_time_dimension rc variable and menu options.
 *
 * Revision 1.41  2008/12/25 04:07:29  gtkwave
 * -Wshadow warning fixes
 *
 * Revision 1.40  2008/12/20 07:44:22  gtkwave
 * experimental support for Tcl repscripts
 *
 * Revision 1.39  2008/11/17 23:08:54  gtkwave
 * make images in toolbars smaller by removing border space
 *
 * Revision 1.38  2008/10/21 04:03:01  gtkwave
 * setlocale disable for mingw on gtk init
 *
 * Revision 1.37  2008/10/13 22:16:52  gtkwave
 * tcl interpreter integration
 *
 * Revision 1.36  2008/10/04 15:15:20  gtkwave
 * gtk1 compatibility fixes
 *
 * Revision 1.35  2008/10/02 00:52:25  gtkwave
 * added dnd of external filetypes into viewer
 *
 * Revision 1.34  2008/10/01 01:31:38  gtkwave
 * more dnd fixes
 *
 * Revision 1.33  2008/09/26 17:05:10  gtkwave
 * force open tree nodes in ctree on initial .sav file read (didn't happen
 * before as ctree was not built yet)
 *
 * Revision 1.32  2008/09/16 00:01:25  gtkwave
 * prelim drag and drop from external apps (now disabled)
 *
 * Revision 1.31  2008/07/20 01:29:36  gtkwave
 * added command line option --comphier
 *
 * Revision 1.30  2008/07/01 18:51:06  gtkwave
 * compiler warning fixes for amd64
 *
 * Revision 1.29  2008/06/11 08:01:50  gtkwave
 * gcc 4.3.x compiler warning fixes
 *
 * Revision 1.28  2008/04/03 16:45:06  gtkwave
 * removed call to time_trunc with marker setting on initialization
 *
 * Revision 1.27  2008/03/20 01:37:44  gtkwave
 * fix for primary marker time on init (should be -- instead of 0 sec if not
 * specified in a save file)
 *
 * Revision 1.26  2008/02/10 18:08:28  gtkwave
 * removal of logfiles static and moved into global structure
 *
 * Revision 1.25  2008/02/10 01:24:00  gtkwave
 * minor cleanups
 *
 * Revision 1.24  2008/01/09 22:25:55  gtkwave
 * added toolbar spacers, increased starting mainwindow x size
 *
 * Revision 1.23  2008/01/09 19:20:52  gtkwave
 * more updating to globals management (expose events cause wrong swap)
 *
 * Revision 1.22  2008/01/09 08:25:41  gtkwave
 * added cut + paste buttons
 *
 * Revision 1.21  2008/01/09 08:07:03  gtkwave
 * removal of explicit menu string names when unnecessary
 *
 * Revision 1.20  2008/01/09 04:09:11  gtkwave
 * fix keyboard focus sighandler when multi-tabs are being used
 *
 * Revision 1.19  2008/01/08 23:03:35  gtkwave
 * added toolbar using use_toolbutton_interface rc variable
 *
 * Revision 1.18  2008/01/05 22:25:46  gtkwave
 * degate busy during treeview dnd as it disrupts focus; dnd cleanups
 *
 * Revision 1.17  2008/01/02 18:54:11  gtkwave
 * cleaned up inconsistent punctuation on --exit option
 *
 * Revision 1.16  2007/12/30 04:27:39  gtkwave
 * added edge buttons to main window
 *
 * Revision 1.15  2007/12/18 17:28:54  gtkwave
 * added --giga command line option to gtkwave
 *
 * Revision 1.14  2007/09/23 18:33:54  gtkwave
 * warnings cleanups from sun compile
 *
 * Revision 1.13  2007/09/17 19:26:46  gtkwave
 * added dead context sweep code (deferred cleanup of multi-tab destroy)
 *
 * Revision 1.12  2007/09/14 14:08:56  gtkwave
 * updating busy handling
 *
 * Revision 1.11  2007/09/12 19:24:45  gtkwave
 * more ctx_swap_watchdog updates
 *
 * Revision 1.10  2007/09/12 17:26:45  gtkwave
 * experimental ctx_swap_watchdog added...still tracking down mouse thrash crashes
 *
 * Revision 1.9  2007/09/11 19:56:39  gtkwave
 * missing dynamic cast added
 *
 * Revision 1.8  2007/09/11 17:17:32  gtkwave
 * allow tab movement with context_tabposition rc variable
 *
 * Revision 1.7  2007/09/11 04:27:44  gtkwave
 * duplicate disable_mouseover copy on ctx switch
 *
 * Revision 1.6  2007/09/11 04:13:25  gtkwave
 * loader hardening for tabbed loads
 *
 * Revision 1.5  2007/09/11 02:20:21  gtkwave
 * migrate busy cursor to new tabbed context
 *
 * Revision 1.4  2007/09/09 20:10:30  gtkwave
 * preliminary support for tabbed viewing of multiple waveforms
 *
 * Revision 1.3  2007/08/29 23:38:47  gtkwave
 * 3.1.0 RC2 minor compatibility/bugfixes
 *
 * Revision 1.2  2007/08/26 21:35:42  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.14  2007/08/26 19:13:30  gtkwave
 * datatype fix
 *
 * Revision 1.1.1.1.2.13  2007/08/26 19:05:55  gtkwave
 * added reload button in main window
 *
 * Revision 1.1.1.1.2.12  2007/08/23 23:28:48  gtkwave
 * reload fail handling and retries
 *
 * Revision 1.1.1.1.2.11  2007/08/23 03:16:03  gtkwave
 * NO_FILE now set on stdin sourced VCDs
 *
 * Revision 1.1.1.1.2.10  2007/08/23 02:42:51  gtkwave
 * convert c++ style comments to c to aid with compiler compatibility
 *
 * Revision 1.1.1.1.2.9  2007/08/19 23:13:53  kermin
 * -o flag will now target the original file (in theory reloaded), compress it to lxt2, and then reload the new compressed file.
 *
 * Revision 1.1.1.1.2.8  2007/08/15 03:26:01  kermin
 * Reload button does not cause a fault, however, state is still somehow incorrect.
 *
 * Revision 1.1.1.1.2.7  2007/08/07 04:54:59  gtkwave
 * slight modifications to global initialization scheme
 *
 * Revision 1.1.1.1.2.6  2007/08/07 03:18:54  kermin
 * Changed to pointer based GLOBAL structure and added initialization function
 *
 * Revision 1.1.1.1.2.5  2007/08/06 03:50:47  gtkwave
 * globals support for ae2, gtk1, cygwin, mingw.  also cleaned up some machine
 * generated structs, etc.
 *
 * Revision 1.1.1.1.2.4  2007/08/05 02:27:21  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1.2.3  2007/07/31 03:18:01  kermin
 * Merge Complete - I hope
 *
 * Revision 1.1.1.1.2.2  2007/07/28 19:50:39  kermin
 * Merged in the main line
 *
 * Revision 1.1.1.1  2007/05/30 04:27:56  gtkwave
 * Imported sources
 *
 * Revision 1.3  2007/05/28 00:55:06  gtkwave
 * added support for arrays as a first class dumpfile datatype
 *
 * Revision 1.2  2007/04/20 02:08:13  gtkwave
 * initial release
 *
 */
