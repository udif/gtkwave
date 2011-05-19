/* 
 * Copyright (c) Tony Bybell 1999-2009.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */

#include "globals.h"

/* example-start menu menufactory.h */

#ifndef __MENUFACTORY_H__
#define __MENUFACTORY_H__

#include <gtk/gtk.h>
#include <stdio.h>

#ifndef _MSC_VER
	#include <strings.h>
#endif

#include <errno.h>
#include "currenttime.h"
#include "fgetdynamic.h"
#include "strace.h"
#include "debug.h"
#include "symbol.h"
#include "main.h"

void do_popup_menu (GtkWidget *my_widget, GdkEventButton *event);
void get_main_menu (GtkWidget *, GtkWidget **menubar);
void menu_set_sensitive(void);
int file_quit_cmd_callback (GtkWidget *widget, gpointer data);
int set_wave_menu_accelerator(char *str);
int execute_script(char *name, int dealloc_name);
char *append_array_row(nptr n);

struct stringchain_t
{
struct stringchain_t *next;
char *name;
};


enum WV_MenuItems {
WV_MENU_FONV,
WV_MENU_FONVT,
WV_MENU_FRW,
WV_MENU_WRVCD,
WV_MENU_WRLXT,
WV_MENU_WRTIM,
WV_MENU_WCLOSE,
WV_MENU_SEP2VCD,
WV_MENU_FPTF,
WV_MENU_SEP1,
WV_MENU_FRSF,
WV_MENU_FWSF,
WV_MENU_FWSFAS,
WV_MENU_SEP2,
WV_MENU_FRLF,
WV_MENU_SEP2LF,
#if !defined _MSC_VER
WV_MENU_FRSTMF,
WV_MENU_SEP2STMF,
#endif
#if defined(HAVE_LIBTCL)
WV_MENU_TCLSCR,
WV_MENU_TCLSEP,
#endif
WV_MENU_FQY,
WV_MENU_ESTMH,
WV_MENU_ETH,
WV_MENU_SEP3,
WV_MENU_EIB,
WV_MENU_EIC,
WV_MENU_EIA,
WV_MENU_EC,
WV_MENU_ECY,
WV_MENU_EP,
WV_MENU_SEP3A,
WV_MENU_EAHT,
WV_MENU_ERHA,
WV_MENU_SEP4,
WV_MENU_EE,
WV_MENU_ECD,
WV_MENU_ECU,
WV_MENU_SEP5,
WV_MENU_EDFH,
WV_MENU_EDFD,
WV_MENU_EDFSD,
WV_MENU_EDFB,
WV_MENU_EDFO,
WV_MENU_EDFA,
WV_MENU_EDRL,
WV_MENU_EDR2BON,
WV_MENU_EDR2BOFF,
WV_MENU_EDFRJON,
WV_MENU_EDFRJOFF,
WV_MENU_EDFION,
WV_MENU_EDFIOFF,
WV_MENU_EDFRON,
WV_MENU_EDFROFF,
WV_MENU_XLF_0,
WV_MENU_XLF_1,
WV_MENU_XLP_0,
WV_MENU_XLP_1,
WV_MENU_TTXLP_0,
WV_MENU_TTXLP_1,
WV_MENU_EDFAOFF,
WV_MENU_EDFASTEP,
WV_MENU_EDFAINTERPOL,
WV_MENU_EDFAINTERPOL2,
WV_MENU_EDFARSD,
WV_MENU_EDFARAD,
WV_MENU_RFILL0,
WV_MENU_RFILL1,
WV_MENU_RFILLOFF,
WV_MENU_B2G,
WV_MENU_G2B,
WV_MENU_GBNONE,
WV_MENU_ESCAH,
WV_MENU_ESCFH,
WV_MENU_SEP6,
WV_MENU_WARP,
WV_MENU_UNWARP,
WV_MENU_UNWARPA,
WV_MENU_SEP7A,
WV_MENU_EEX,
WV_MENU_ESH,
WV_MENU_SEP6A,
/* WV_MENU_EXA, */
/* WV_MENU_CPA, */
WV_MENU_TG,
WV_MENU_AG,
WV_MENU_SEP6A1,
WV_MENU_EHR,
WV_MENU_EUHR,
WV_MENU_EHA,
WV_MENU_EUHA,
WV_MENU_SEP6B,
WV_MENU_ALPHA,
WV_MENU_ALPHA2,
WV_MENU_LEX,
WV_MENU_RVS,
WV_MENU_SPS,
#ifdef WAVE_USE_GTK2
WV_MENU_SPS2,
#endif
WV_MENU_SEP7B,
WV_MENU_SSR,
WV_MENU_SSH,
WV_MENU_SST,
WV_MENU_SEP7,
WV_MENU_ACOL,
WV_MENU_ACOLR,
WV_MENU_ABON,
WV_MENU_HTGP,
WV_MENU_SEP7C,
WV_MENU_STRSE,
WV_MENU_TMTT,
WV_MENU_TZZA,
WV_MENU_TZZB,
WV_MENU_TZZI,
WV_MENU_TZZO,
WV_MENU_TZZBFL,
WV_MENU_TZZBF,
WV_MENU_TZZTS,
WV_MENU_TZZTE,
WV_MENU_TZUZ,
WV_MENU_TFFS,
WV_MENU_TFFR,
WV_MENU_TFFL,
WV_MENU_TDDR,
WV_MENU_TDDL,
WV_MENU_TSSR,
WV_MENU_TSSL,
WV_MENU_TPPR,
WV_MENU_TPPL,
WV_MENU_MSCMD,
WV_MENU_MDNM,
WV_MENU_MCNM,
WV_MENU_MCANM,
WV_MENU_MDPM,
WV_MENU_SEP8,
WV_MENU_MWSON,
WV_MENU_MLKLT, 
WV_MENU_MLKRT, 
WV_MENU_MLKOFF,
WV_MENU_VSG,
WV_MENU_SEP9,
#if !defined _MSC_VER && !defined __MINGW32__
WV_MENU_VSMO,
WV_MENU_SEP9A,
#endif
WV_MENU_VSBS,
WV_MENU_SEP10,
WV_MENU_VDR,
WV_MENU_SEP11,
WV_MENU_VCZ,
WV_MENU_SEP12,
WV_MENU_VTDF,
WV_MENU_VTMM,
WV_MENU_SEP13,
WV_MENU_VCMU,
WV_MENU_SEP14,
WV_MENU_VDRV,
WV_MENU_SEP15,
WV_MENU_VLJS,
WV_MENU_VRJS,
WV_MENU_SEP16,
WV_MENU_VZPS,
WV_MENU_VZDYN,
WV_MENU_VZDYNE,
WV_MENU_VFTP,
WV_MENU_SEP17,
WV_MENU_RULER,
WV_MENU_RMRKS,
WV_MENU_SEP17A,
WV_MENU_USECOLOR,
WV_MENU_USEBW,
WV_MENU_SEP18,
WV_MENU_LXTCC2Z,
WV_MENU_SEP19,
WV_MENU_TDSCALEX,
WV_MENU_TDSCALES,
WV_MENU_TDSCALEM,
WV_MENU_TDSCALEU,
WV_MENU_TDSCALEN,
WV_MENU_TDSCALEP,
WV_MENU_TDSCALEF,
WV_MENU_HWH,
WV_MENU_HWV,
WV_MENU_NUMITEMS
};

void menu_new_viewer(GtkWidget *widget, gpointer data);
void menu_write_vcd_file(GtkWidget *widget, gpointer data);
void menu_write_lxt_file(GtkWidget *widget, gpointer data);
void menu_print(GtkWidget *widget, gpointer data);
void menu_read_save_file(GtkWidget *widget, gpointer data);
void menu_write_save_file(GtkWidget *widget, gpointer data);
void menu_write_save_file_as(GtkWidget *widget, gpointer data);
void menu_read_log_file(GtkWidget *widget, gpointer data);
void menu_read_stems_file(GtkWidget *widget, gpointer data);
void menu_quit(GtkWidget *widget, gpointer data);
void menu_set_max_hier(GtkWidget *widget, gpointer data);
void menu_insert_blank_traces(GtkWidget *widget, gpointer data);
void menu_insert_comment_traces(GtkWidget *widget, gpointer data);
void menu_insert_analog_height_extension(GtkWidget *widget, gpointer data);
void menu_alias(GtkWidget *widget, gpointer data);
void menu_remove_aliases(GtkWidget *widget, gpointer data);
void menu_cut_traces(GtkWidget *widget, gpointer data);
void menu_copy_traces(GtkWidget *widget, gpointer data);
void menu_paste_traces(GtkWidget *widget, gpointer data);
void menu_combine_down(GtkWidget *widget, gpointer data);
void menu_combine_up(GtkWidget *widget, gpointer data);
void menu_reduce_singlebit_vex(GtkWidget *widget, gpointer data);
void menu_dataformat_hex(GtkWidget *widget, gpointer data);
void menu_dataformat_dec(GtkWidget *widget, gpointer data);
void menu_dataformat_signed(GtkWidget *widget, gpointer data);
void menu_dataformat_bin(GtkWidget *widget, gpointer data);
void menu_dataformat_oct(GtkWidget *widget, gpointer data);
void menu_dataformat_ascii(GtkWidget *widget, gpointer data);
void menu_dataformat_real(GtkWidget *widget, gpointer data);
void menu_dataformat_rjustify_on(GtkWidget *widget, gpointer data);
void menu_dataformat_rjustify_off(GtkWidget *widget, gpointer data);
void menu_dataformat_invert_on(GtkWidget *widget, gpointer data);
void menu_dataformat_invert_off(GtkWidget *widget, gpointer data);
void menu_dataformat_reverse_on(GtkWidget *widget, gpointer data);
void menu_dataformat_reverse_off(GtkWidget *widget, gpointer data);
void menu_dataformat_xlate_file_0(GtkWidget *widget, gpointer data);
void menu_dataformat_xlate_file_1(GtkWidget *widget, gpointer data);
void menu_dataformat_xlate_proc_0(GtkWidget *widget, gpointer data);
void menu_dataformat_xlate_proc_1(GtkWidget *widget, gpointer data);
void menu_dataformat_analog_off(GtkWidget *widget, gpointer data);
void menu_dataformat_analog_step(GtkWidget *widget, gpointer data);
void menu_dataformat_analog_interpol(GtkWidget *widget, gpointer data);
void menu_showchangeall(GtkWidget *widget, gpointer data);
void menu_showchange(GtkWidget *widget, gpointer data);
void menu_warp_traces(GtkWidget *widget, gpointer data);
void menu_unwarp_traces(GtkWidget *widget, gpointer data);
void menu_unwarp_traces_all(GtkWidget *widget, gpointer data);
void menu_dataformat_exclude_on(GtkWidget *widget, gpointer data);
void menu_dataformat_exclude_off(GtkWidget *widget, gpointer data);
void menu_regexp_highlight(GtkWidget *widget, gpointer data);
void menu_regexp_unhighlight(GtkWidget *widget, gpointer data);
void menu_dataformat_highlight_all(GtkWidget *widget, gpointer data);
void menu_dataformat_unhighlight_all(GtkWidget *widget, gpointer data);
void menu_alphabetize(GtkWidget *widget, gpointer data);
void menu_alphabetize2(GtkWidget *widget, gpointer data);
void menu_lexize(GtkWidget *widget, gpointer data);
void menu_reverse(GtkWidget *widget, gpointer data);
void menu_tracesearchbox(GtkWidget *widget, gpointer data);
void menu_signalsearch(GtkWidget *widget, gpointer data);
void menu_hiersearch(GtkWidget *widget, gpointer data);
void menu_treesearch(GtkWidget *widget, gpointer data);
void menu_autocoalesce(GtkWidget *widget, gpointer data);
void menu_autocoalesce_reversal(GtkWidget *widget, gpointer data);
void menu_autoname_bundles_on(GtkWidget *widget, gpointer data);
void menu_hgrouping(GtkWidget *widget, gpointer data);
void menu_movetotime(GtkWidget *widget, gpointer data);
void menu_zoomsize(GtkWidget *widget, gpointer data);
void menu_zoombase(GtkWidget *widget, gpointer data);
void menu_fetchsize(GtkWidget *widget, gpointer data);
void menu_markerbox(GtkWidget *widget, gpointer data);
void drop_named_marker(GtkWidget *widget, gpointer data);
void collect_named_marker(GtkWidget *widget, gpointer data);
void collect_all_named_markers(GtkWidget *widget, gpointer data);
void delete_unnamed_marker(GtkWidget *widget, gpointer data);
void wave_scrolling_on(GtkWidget *widget, gpointer data);
void menu_show_grid(GtkWidget *widget, gpointer data);
void menu_show_mouseover(GtkWidget *widget, gpointer data);
void menu_show_base(GtkWidget *widget, gpointer data);
void menu_enable_dynamic_resize(GtkWidget *widget, gpointer data);
void menu_center_zooms(GtkWidget *widget, gpointer data);
void menu_toggle_delta_or_frequency(GtkWidget *widget, gpointer data);
void menu_toggle_max_or_marker(GtkWidget *widget, gpointer data);
void menu_enable_constant_marker_update(GtkWidget *widget, gpointer data);
void menu_use_roundcaps(GtkWidget *widget, gpointer data);
void menu_left_justify(GtkWidget *widget, gpointer data);
void menu_right_justify(GtkWidget *widget, gpointer data);
void menu_zoom10_snap(GtkWidget *widget, gpointer data);
void menu_use_full_precision(GtkWidget *widget, gpointer data);
void menu_remove_marked(GtkWidget *widget, gpointer data);
void menu_lxt_clk_compress(GtkWidget *widget, gpointer data);
void menu_help(GtkWidget *widget, gpointer data);
void menu_version(GtkWidget *widget, gpointer data);
void menu_toggle_group(GtkWidget *widget, gpointer data);


GtkItemFactoryEntry *retrieve_menu_items_array(int *num_items);

void menu_read_stems_cleanup(GtkWidget *widget, gpointer data);
void menu_new_viewer_tab_cleanup(GtkWidget *widget, gpointer data);
int menu_new_viewer_tab_cleanup_2(char *fname);


void set_scale_to_time_dimension_toggles(void);

/* These should eventually have error values */
void write_save_helper(FILE *file);
int read_save_helper(char *wname); /* -1 = error, 0+ = number of lines read */

void SetTraceScrollbarRowValue(int row, unsigned center);

bvptr combine_traces(int direction, Trptr single_trace_only);

#endif

/*
 * $Id: menu.h,v 1.44 2010/10/06 20:15:51 gtkwave Exp $
 * $Log: menu.h,v $
 * Revision 1.44  2010/10/06 20:15:51  gtkwave
 * preliminary version of RPC mechanism
 *
 * Revision 1.43  2010/08/26 18:42:23  gtkwave
 * added support for transaction filters in mingw
 *
 * Revision 1.42  2010/08/26 01:32:01  gtkwave
 * added mingw support for Open New Window menu option
 *
 * Revision 1.41  2010/08/25 22:58:23  gtkwave
 * added process file support for mingw
 *
 * Revision 1.40  2010/07/12 20:51:42  gtkwave
 * free to non-malloc'd address fix
 *
 * Revision 1.39  2010/07/01 23:02:41  gtkwave
 * header file cleanup
 *
 * Revision 1.38  2010/06/29 02:05:14  gtkwave
 * changed quit accelerator
 *
 * Revision 1.37  2010/06/13 20:38:00  gtkwave
 * added strace repeat count
 *
 * Revision 1.36  2010/04/12 23:07:12  gtkwave
 * add ability to make single signals transactions
 *
 * Revision 1.35  2010/03/31 15:42:47  gtkwave
 * added preliminary transaction filter support
 *
 * Revision 1.34  2010/03/24 23:05:10  gtkwave
 * added RealToBits menu option
 *
 * Revision 1.33  2010/02/24 18:06:14  gtkwave
 * removed reduce single bit vectors
 *
 * Revision 1.32  2010/02/24 17:35:35  gtkwave
 * gtk1 compile fixes
 *
 * Revision 1.31  2010/01/22 02:10:49  gtkwave
 * added second pattern search capability
 *
 * Revision 1.30  2010/01/18 20:47:02  gtkwave
 * added named locker marking
 *
 * Revision 1.29  2009/11/25 09:49:28  gtkwave
 * added gray code support
 *
 * Revision 1.28  2009/09/22 13:51:14  gtkwave
 * warnings fixes
 *
 * Revision 1.27  2009/09/14 03:00:08  gtkwave
 * bluespec code integration
 *
 * Revision 1.26  2009/04/23 21:57:53  gtkwave
 * added mingw support for rtlbrowse
 *
 * Revision 1.25  2009/04/10 04:33:58  gtkwave
 * added menu blackouts on empty gui
 *
 * Revision 1.24  2009/01/13 23:09:12  gtkwave
 * added copy traces button to toolbar in gtkwave
 *
 * Revision 1.23  2009/01/12 04:17:39  gtkwave
 * added dynamic zoom for end for partial vcd
 *
 * Revision 1.22  2008/12/28 03:03:34  gtkwave
 * Added scale_to_time_dimension rc variable and menu options.
 *
 * Revision 1.21  2008/12/16 19:28:20  gtkwave
 * more warnings cleanups
 *
 * Revision 1.20  2008/12/05 19:44:08  gtkwave
 * prelim support for timinganalyzer file format export
 *
 * Revision 1.19  2008/12/04 19:21:00  gtkwave
 * added zoom_dynamic option for partial VCD loading
 *
 * Revision 1.18  2008/11/27 19:37:38  gtkwave
 * disable filter process in windows compiles
 *
 * Revision 1.17  2008/11/25 18:07:32  gtkwave
 * added cut copy paste functionality that survives reload and can do
 * multiple pastes on the same cut buffer
 *
 * Revision 1.16  2008/11/24 02:55:10  gtkwave
 * use TCL_INCLUDE_SPEC to fix ubuntu compiles
 *
 * Revision 1.15  2008/11/19 19:08:34  gtkwave
 * add tcl menu option, update file requester name reentrancy code
 *
 * Revision 1.14  2008/10/14 00:53:46  gtkwave
 * enabled tcl scripts to call existing gtkwave style scripted menu functions
 *
 * Revision 1.13  2008/09/24 23:41:23  gtkwave
 * drag from signal window into external process
 *
 * Revision 1.12  2008/03/25 03:22:11  gtkwave
 * expanded zero fill to include also a one fill (for pre-inverted nets)
 *
 * Revision 1.11  2008/03/24 19:34:00  gtkwave
 * added zero range fill feature
 *
 * Revision 1.10  2008/02/12 23:35:42  gtkwave
 * preparing for 3.1.5 revision bump
 *
 * Revision 1.9  2008/01/25 04:10:14  gtkwave
 * added new resizing options to menu
 *
 * Revision 1.8  2008/01/23 02:05:43  gtkwave
 * added interpolated + step mode
 *
 * Revision 1.7  2008/01/12 21:36:44  gtkwave
 * added black and white vs color rendering menu options
 *
 * Revision 1.6  2007/09/23 18:33:55  gtkwave
 * warnings cleanups from sun compile
 *
 * Revision 1.5  2007/09/09 20:10:30  gtkwave
 * preliminary support for tabbed viewing of multiple waveforms
 *
 * Revision 1.4  2007/08/26 21:35:43  gtkwave
 * integrated global context management from SystemOfCode2007 branch
 *
 * Revision 1.1.1.1.2.6  2007/08/25 19:43:45  gtkwave
 * header cleanups
 *
 * Revision 1.1.1.1.2.5  2007/08/23 02:19:49  gtkwave
 * merge GLOBALS state from old hier_search widget into new one
 *
 * Revision 1.1.1.1.2.4  2007/08/15 03:26:01  kermin
 * Reload button does not cause a fault, however, state is still somehow incorrect.
 *
 * Revision 1.1.1.1.2.3  2007/08/05 02:27:21  kermin
 * Semi working global struct
 *
 * Revision 1.1.1.1.2.2  2007/07/28 19:50:40  kermin
 * Merged in the main line
 *
 * Revision 1.3  2007/07/23 23:13:08  gtkwave
 * adds for color tags in filtered trace data
 *
 * Revision 1.2  2007/06/18 05:05:08  gtkwave
 * noticed non-extern declaraction (detected only by tcc it seems...)
 *
 * Revision 1.1.1.1  2007/05/30 04:27:58  gtkwave
 * Imported sources
 *
 * Revision 1.2  2007/04/20 02:08:13  gtkwave
 * initial release
 *
 */

