/* 
 * Copyright (c) Yiftach Tzori 2009.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 */
#ifndef WAVE_TCL_SUPPORT_CMDS_H
#define WAVE_TCL_SUPPORT_CMDS_H

#include <gtk/gtk.h>
#include "debug.h"

GtkCTreeNode *SST_find_node_by_path(GtkCTreeRow *root, char *path);
int SST_open_path(GtkCTree *ctree, GtkCTreeNode *node);
void fill_sig_store (void);
int SST_open_node(char *name);

llist_p *llist_new(llist_u v, ll_elem_type type, int arg); 
llist_p *llist_append(llist_p *head, llist_p *elem, llist_p **tail); 
llist_p *llist_remove_last(llist_p *head, llist_p **tail, ll_elem_type type, void *f() ); 
void llist_free(llist_p *head, ll_elem_type type, void *f()); 

llist_p *signal_change_list(char *sig_name, int dir, TimeType start_time,
                       TimeType end_time, int max_elements); 

#define SST_NODE_FOUND 0
#define SST_NODE_CURRENT 2
#define SST_NODE_NOT_EXIST 1
#define SST_TREE_NOT_EXIST -1

#endif

/*
 * $Id: tcl_support_commands.h,v 1.5 2010/08/15 18:54:49 gtkwave Exp $
 * $Log: tcl_support_commands.h,v $
 * Revision 1.5  2010/08/15 18:54:49  gtkwave
 * fixes to SST open node to support tcl bug workaround (?)
 *
 * Revision 1.4  2009/10/08 17:40:50  gtkwave
 * removed casting on llist_new, use union instead as arg
 *
 * Revision 1.3  2009/10/07 21:13:10  gtkwave
 * 64-bit fixes on signal list generation
 *
 * Revision 1.2  2009/09/28 05:58:05  gtkwave
 * changes to support signal_change_list
 *
 * Revision 1.1  2009/09/20 21:43:35  gtkwave
 * created
 *
 */

