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

#include "sui/sui.h"
#include "sui_event_hdr.h"
#include "srain_window.h"
#include "srain_join_dialog.h"

#include "srain.h"
#include "i18n.h"
#include "log.h"

#define MATCH_CHANNEL               0
#define MATCH_CHANNEL_WITH_REGEX    1
#define MATCH_TOPIC_WITH_REGEX      2

#define MATCH_LIST_STORE_COL_INDEX      0
#define MATCH_LIST_STORE_COL_COMMENT    1

#define CHAN_LIST_STORE_COL_CHANNEL     0
#define CHAN_LIST_STORE_COL_USERS       1
#define CHAN_LIST_STORE_COL_TOPIC       2

struct _SrainJoinDialog {
    GtkDialog parent;

    int match;

    /* Search area */
    GtkEntry *chan_entry;
    GtkCheckButton *advanced_check_button;
    GtkRevealer *revealer;

    /* Filter */
    GtkComboBox *match_combo_box;
    GtkSpinButton *min_users_spin_button;
    GtkSpinButton *max_users_spin_button;
    GtkButton *refresh_button;

    /* Channel list */
    GtkTreeView *chan_tree_view;
    GtkTreeViewColumn *chan_tree_view_column;
    GtkTreeViewColumn *users_tree_view_column;
    GtkTreeViewColumn *topic_tree_view_column;

    /* Dialog button */
    GtkButton *cancel_button;
    GtkButton *join_button;

    /* Data model */
    GtkListStore *match_list_store;
    GtkListStore *chan_list_store;
};

struct _SrainJoinDialogClass {
    GtkDialogClass parent_class;
};

G_DEFINE_TYPE(SrainJoinDialog, srain_join_dialog, GTK_TYPE_DIALOG);

static void srain_join_dialog_init(SrainJoinDialog *self);
static void srain_join_dialog_class_init(SrainJoinDialogClass *class);
static void match_combo_box_set_model(SrainJoinDialog *dialog);
static void chan_tree_view_set_model(SrainJoinDialog *dialog);

static void cancel_button_on_click(gpointer user_data);
static void join_button_on_click(gpointer user_data);
static void advanced_check_button_on_toggled(GtkToggleButton *togglebutton,
        gpointer user_data);
static void match_combo_box_on_changed(GtkComboBox *combobox,
        gpointer user_data);

SrainJoinDialog* srain_join_dialog_new(GtkWindow *parent){
    SrainJoinDialog *dialog;

    dialog = g_object_new(SRAIN_TYPE_JOIN_DIALOG, NULL);

    gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);

    return dialog;
}

void srain_join_dialog_add_chan_entry(SrainJoinDialog *dialog,
        const char *chan, int users, const char *topic){
    GtkTreeIter iter;

    gtk_list_store_append(dialog->chan_list_store, &iter);
    gtk_list_store_set(dialog->chan_list_store, &iter,
            CHAN_LIST_STORE_COL_CHANNEL, chan,
            CHAN_LIST_STORE_COL_USERS, users,
            CHAN_LIST_STORE_COL_TOPIC, topic,
            -1);
}


/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void srain_join_dialog_init(SrainJoinDialog *self){
    gtk_widget_init_template(GTK_WIDGET(self));

    self->match = MATCH_CHANNEL;
    match_combo_box_set_model(self);
    chan_tree_view_set_model(self);

    g_signal_connect_swapped(self->cancel_button, "clicked",
            G_CALLBACK(cancel_button_on_click), self);
    g_signal_connect_swapped(self->join_button, "clicked",
            G_CALLBACK(join_button_on_click), self);
    g_signal_connect(self->advanced_check_button, "toggled",
            G_CALLBACK(advanced_check_button_on_toggled), self);
    g_signal_connect(self->match_combo_box, "changed",
            G_CALLBACK(match_combo_box_on_changed), self);

    // TODO: remove me
    srain_join_dialog_add_chan_entry(self, "#srain", 1, "Here");

}

static void srain_join_dialog_class_init(SrainJoinDialogClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/join_dialog.glade");

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, chan_entry);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, advanced_check_button );
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, revealer);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, match_combo_box);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, min_users_spin_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, max_users_spin_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, refresh_button);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, chan_tree_view);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, chan_tree_view_column);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, users_tree_view_column);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, topic_tree_view_column);

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, cancel_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainJoinDialog, join_button);
}

static void match_combo_box_set_model(SrainJoinDialog *dialog){
    GtkListStore *store;
    GtkComboBox *combobox;
    GtkTreeIter iter;

    /* 2 columns: index, comment */
    dialog->match_list_store = gtk_list_store_new(2,
            G_TYPE_INT,
            G_TYPE_STRING);
    store = dialog->match_list_store;
    combobox = dialog->match_combo_box;

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
            MATCH_LIST_STORE_COL_INDEX, MATCH_CHANNEL,
            MATCH_LIST_STORE_COL_COMMENT, "Match channel name",
            -1);

    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
            MATCH_LIST_STORE_COL_INDEX, MATCH_CHANNEL_WITH_REGEX,
            MATCH_LIST_STORE_COL_COMMENT, "Match channel name with regular expression",
            -1);

    gtk_combo_box_set_model(combobox, GTK_TREE_MODEL(store));
}

static void chan_tree_view_set_model(SrainJoinDialog *dialog){
    GtkListStore *store;
    GtkTreeView *view;

    /* 3 columns: channel name, users, topic */
    dialog->chan_list_store = gtk_list_store_new(3,
            G_TYPE_STRING,
            G_TYPE_INT,
            G_TYPE_STRING);

    store = dialog->chan_list_store;
    view = dialog->chan_tree_view;

    gtk_tree_view_set_model(view, GTK_TREE_MODEL(store));
}

static void cancel_button_on_click(gpointer user_data){
    SrainJoinDialog *dialog;

    dialog = user_data;
    gtk_dialog_response(GTK_DIALOG(dialog), SRAIN_JOIN_DIALOG_RESP_CANCEL);
}

static void join_button_on_click(gpointer user_data){
    const char *chan;
    GVariantDict *params;
    SrnRet ret;
    SrainJoinDialog *dialog;
    SrainChat *chat;

    dialog = user_data;
    params = g_variant_dict_new(NULL);
    chat = srain_window_get_cur_chat(srain_win);

    if (!SRAIN_IS_CHAT(chat)){
       sui_message_box(_("Error"), _("Please connect to server before join any channel"));
    }

    chan = gtk_entry_get_text(dialog->chan_entry);
    chat = srain_window_get_cur_chat(srain_win);

    g_variant_dict_insert(params, "channel", SUI_EVENT_PARAM_STRING, chan);
    // TODO: password support
    g_variant_dict_insert(params, "password", SUI_EVENT_PARAM_STRING, "");

    ret = sui_event_hdr(srain_chat_get_session(chat), SUI_EVENT_JOIN, params);

    if (RET_IS_OK(ret)){
        gtk_dialog_response(GTK_DIALOG(dialog), SRAIN_JOIN_DIALOG_RESP_JOIN);
    } else {
        sui_message_box(_("Error"), RET_MSG(ret));
    }
}

static void advanced_check_button_on_toggled(GtkToggleButton *togglebutton,
        gpointer user_data){
    SrainJoinDialog *dialog;

    dialog = user_data;

    gtk_revealer_set_reveal_child(dialog->revealer,
            gtk_toggle_button_get_active(togglebutton));
}

static void match_combo_box_on_changed(GtkComboBox *combobox,
        gpointer user_data){
    int match;
    GtkTreeIter iter;
    SrainJoinDialog *dialog;

    dialog = user_data;

    if (!gtk_combo_box_get_active_iter(combobox, &iter)){
        ERR_FR("No acive item");
    }

    gtk_tree_model_get(GTK_TREE_MODEL(dialog->match_list_store), &iter,
            MATCH_LIST_STORE_COL_INDEX, &match,
            -1);
    dialog->match = match;

    DBG_FR("Selected index: %d", match);
}

gboolean chan_tree_view_visible_func(GtkTreeModel *model, GtkTreeIter *iter,
        gpointer user_data){
    int users;
    int min_users;
    int max_users;
    char *chan;
    char *topic;
    const char *input;
    gboolean visable;
    SrainJoinDialog *dialog;

    visable = FALSE;
    dialog = user_data;
    gtk_tree_model_get(model, iter,
            CHAN_LIST_STORE_COL_CHANNEL, &chan,
            CHAN_LIST_STORE_COL_USERS, &users,
            CHAN_LIST_STORE_COL_TOPIC, &topic,
            -1);

    min_users = gtk_spin_button_get_value(dialog->min_users_spin_button);
    max_users = gtk_spin_button_get_value(dialog->max_users_spin_button);
    input = gtk_entry_get_text(dialog->chan_entry);

    // Filter users
    if (min_users != - 1 && users < min_users){
        goto FIN;
    }
    if (max_users != - 1 && users > max_users){
        goto FIN;
    }

    switch (dialog->match){
        case MATCH_CHANNEL:
            if (g_ascii_strcasecmp(input, chan) == 0){
                visable = TRUE;
            }
            break;
        case MATCH_CHANNEL_WITH_REGEX:
        default:
            ERR_FR("Unsupported match type");
    }

FIN:
    g_free(chan);
    g_free(topic);

    return visable;
}
