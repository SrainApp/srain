/* Copyright (C) 2016-2017 Shengyu Zhang <srain@srain.im>
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

/**
 * @file srain_connect_dialog.c
 * @brief Connection dialog
 * @author Shengyu Zhang <srain@srain.im>
 * @version 0.06.2
 * @date 2016-09-16
 */

#include <gtk/gtk.h>

struct _SrainConnectDialog {
    GtkDialog parent;

    /* Server info */
    GtkEntry *name_entry;
    GtkEntry *host_entry;
    GtkEntry *port_entry;
    GtkEntry *passwd_entry;
    GtkEntry *nick_entry;
    GtkEntry *realname_entry;
    GtkCheckButton  *tls_check_button;
    GtkCheckButton  *tls_noverify_check_button;

    /* Server list */
    GtkListBox *server_list_box;
    GtkButton *add_server_button;
    GtkButton *rm_server_button;

    /* Dialog button */
    GtkButton *cancel_button;
    GtkButton *save_button;
    GtkButton *connect_button;
};

// gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
