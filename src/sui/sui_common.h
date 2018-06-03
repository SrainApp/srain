/* Copyright (C) 2016-2017 Shengyu Zhang <i@silverrainz.me>
 *
 * This file is part of Srain.
 *
 * Srain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef __UI_COMMON_H
#define __UI_COMMON_H

#include <gtk/gtk.h>

#include "sui_window.h"
#include "sui_buffer.h"
#include "sui_server_buffer.h"

GtkListBoxRow* gtk_list_box_get_row_by_name(GtkListBox *listbox, const char* name);
GtkListBoxRow* gtk_list_box_add_unfocusable_row(GtkListBox *listbox, GtkWidget *widget);
GtkPopover* create_popover(GtkWidget *parent, GtkWidget *child, GtkPositionType pos);
char* show_open_filechosser(GtkWindow *parent);
void scale_size_to( int src_width, int src_height, int max_width, int max_height, int *dst_width, int *dst_height);
gboolean activate_link(GtkLabel *label, const char *uri, gpointer user_data);
SuiWindow *sui_get_cur_window();
SuiBuffer *sui_get_cur_buffer();
SuiServerBuffer *sui_get_cur_server_buffer();
SrnRet sui_open_url(const char *url);

/* Panel helper function */
void sui_panel_popup(GtkWidget *relative_to, GtkWidget *child);
void sui_panel_popup_at_point(GtkWidget *relative_to, GtkWidget *child, int x, int y);
void sui_panel_popdown(GtkWidget *child);

#endif /** __UI_COMMON_H **/
