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

#include "srain_join_dialog.h"
#include "sui/sui.h"

struct _SrainJoinDialog {
    GtkDialog parent;

    GVariantDict *params;   // Return value

    /* Search area */
    GtkEntry *chan_entry;
    GtkCheckButton *advanced_check_button;
    GtkRevealer *revealer;

    /* Filter */
    GtkCheckButton *regex_check_button;
    GtkSpinButton *min_user_spin_button;
    GtkSpinButton *max_user_spin_button;
    GtkButton *refresh_button;

    /* Channel list */
    GtkTreeView *chan_tree_view;
    GtkTreeViewColumn *chan_tree_view_column;
    GtkTreeViewColumn *users_tree_view_column;
    GtkTreeViewColumn *topic_tree_view_column;

    /* Dialog button */
    GtkButton *cancel_button;
    GtkButton *join_button;
};

struct _SrainJoinDialogClass {
    GtkDialogClass parent_class;
};

G_DEFINE_TYPE(SrainJoinDialog, srain_join_dialog, GTK_TYPE_DIALOG);

static void cancel_button_on_click(gpointer user_data);
static void join_button_on_click(gpointer user_data);

static void srain_join_dialog_init(SrainJoinDialog *self){
    gtk_widget_init_template(GTK_WIDGET(self));

    g_signal_connect_swapped(self->cancel_button, "clicked",
            G_CALLBACK(cancel_button_on_click), self);
    g_signal_connect_swapped(self->join_button, "clicked",
            G_CALLBACK(join_button_on_click), self);
}

static void srain_join_dialog_class_init(SrainJoinDialogClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/join_dialog.glade");

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, chan_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, advanced_check_button );
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, revealer);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, regex_check_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, min_user_spin_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, max_user_spin_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, refresh_button);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, chan_tree_view);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, chan_tree_view_column);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, users_tree_view_column);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, topic_tree_view_column);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, cancel_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, join_button);
}

SrainJoinDialog* srain_join_dialog_new(GtkWindow *parent, GVariantDict *params){
    SrainJoinDialog *dialog;

    dialog = g_object_new(SRAIN_TYPE_JOIN_DIALOG, NULL);

    gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
    dialog->params = params;

    return dialog;
}

static void cancel_button_on_click(gpointer user_data){
    SrainJoinDialog *dialog;

    dialog = user_data;
    gtk_dialog_response(GTK_DIALOG(dialog), SRAIN_JOIN_DIALOG_RESP_CANCEL);
}

static void join_button_on_click(gpointer user_data){
    const char *chan;
    const char *passwd;
    SrainJoinDialog *dialog;

    dialog = user_data;
    gtk_dialog_response(GTK_DIALOG(dialog), SRAIN_JOIN_DIALOG_RESP_JOIN);

    return;

    g_variant_dict_insert(dialog->params, "channel", SUI_EVENT_PARAM_STRING, chan);
    g_variant_dict_insert(dialog->params, "password", SUI_EVENT_PARAM_STRING, passwd);
}
