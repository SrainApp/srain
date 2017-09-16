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
 * @file srain_join_dialog.c
 * @brief Join dialog
 * @author Shengyu Zhang <srain@srain.im>
 * @version 0.06.2
 * @date 2016-09-16
 */

#include <gtk/gtk.h>

struct _SrainJoinDialog {
    GtkDialog parent;

    /* Search area */
    GtkEntry *chan_entry;
    GtkCheckButton *advanced_check_button;
    GtkRevealer *revealer;

    /* Filter */
    GtkCheckButton *regex_check_button;
    GtkSpinButton *min_user_spin_button;
    GtkSpinButton *max_user_spin_button;
    GtkButton *refresh_button;

    GtkTreeView *chan_tree_view;
    GtkTreeViewColumn *chan_tree_view_colunm;
    GtkTreeViewColumn *users_tree_view_colunm;
    GtkTreeViewColumn *topic_tree_view_colunm;

    /* Dialog button */
    GtkButton *cancal_button;
    GtkButton *join_button;
};
