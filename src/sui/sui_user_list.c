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

/**
 * @file sui_user_list.c
 * @brief Widget for listing all participants of a SrnChatbuffer
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-04-03
 *
 * NOTE: It is allowed to add duplicate user, deduplication should be
 * do in the upper layer.
 */

#include <gtk/gtk.h>

#include "core/core.h"

#include "sui_user_list.h"
#include "nick_menu.h"

#include "log.h"
#include "i18n.h"

struct _SuiUserList {
    GtkBox parent;

    GtkLabel *stat_label;   // users statistics
    GtkTreeView *user_tree_view;

    /* Data model */
    SuiUserStat user_stat;
    GtkListStore *user_list_store;
    GtkTreeModel *user_tree_model_filter;   // FilterTreeModel of user_list_store
                                            // TODO: user search
};

struct _SuiUserListClass {
    GtkBoxClass parent_class;
};

static void user_tree_view_set_model(SuiUserList *self);
static void stat_label_update_stat(SuiUserList *self);
static int user_list_store_sort_func(GtkTreeModel *model,
        GtkTreeIter *iter1, GtkTreeIter *iter2, gpointer user_data);

static gboolean user_tree_view_on_popup(GtkWidget *widget,
        GdkEventButton *event, gpointer user_data);
void user_list_store_on_row_changed(GtkTreeModel *tree_model, GtkTreePath *path,
        GtkTreeIter *iter, gpointer user_data);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

G_DEFINE_TYPE(SuiUserList, sui_user_list, GTK_TYPE_BOX);

static void sui_user_list_init(SuiUserList *self){
    gtk_widget_init_template(GTK_WIDGET(self));

    user_tree_view_set_model(self);
    stat_label_update_stat(self);

    g_signal_connect(self->user_tree_view, "button-press-event",
            G_CALLBACK(user_tree_view_on_popup), NULL);
    g_signal_connect(self->user_list_store, "row-changed",
            G_CALLBACK(user_list_store_on_row_changed), self);
}

static void sui_user_list_class_init(SuiUserListClass *class){
    GtkWidgetClass *widget_class;

    widget_class = GTK_WIDGET_CLASS(class);

    gtk_widget_class_set_template_from_resource(widget_class,
            "/im/srain/Srain/user_list.glade");

    gtk_widget_class_bind_template_child(widget_class, SuiUserList, user_tree_view);
    gtk_widget_class_bind_template_child(widget_class, SuiUserList, stat_label);
}

/*****************************************************************************
 * Expored functions
 *****************************************************************************/

SuiUserList* sui_user_list_new(void){
    return g_object_new(SUI_TYPE_USER_LIST, NULL);
}

void sui_user_list_add_user(SuiUserList *self, SuiUser *user){
    gtk_list_store_append(self->user_list_store, (GtkTreeIter *)user);
    sui_user_set_list(user, self->user_list_store);
    sui_user_set_stat(user, &self->user_stat);
    self->user_stat.total++;
    sui_user_update(user);
}

void sui_user_list_rm_user(SuiUserList *self, SuiUser *user){
    gtk_list_store_remove(self->user_list_store, (GtkTreeIter *)user);
    sui_user_set_list(user, NULL);
    sui_user_set_stat(user, NULL);
    self->user_stat.total--;
    sui_user_update(user);
}

void sui_user_list_clear(SuiUserList *self){
    gtk_list_store_clear(self->user_list_store);
    memset(&self->user_stat, 0, sizeof(self->user_stat));
}

GSList* sui_user_list_get_users_by_prefix(SuiUserList *self, const char *prefix){
    GSList *users;
    GtkTreeModel *model;
    GtkTreeIter iter;

    model = GTK_TREE_MODEL(self->user_list_store);
    if (!gtk_tree_model_get_iter_first(model, &iter)){
        return NULL;
    }

    users = NULL;
    do {
        SuiUser *user;

        user = sui_user_new_from_iter(GTK_LIST_STORE(model), &iter);
        if (g_str_has_prefix(sui_user_get_nickname(user), prefix)){
            users = g_slist_append(users, user);
        } else {
            sui_user_free(user);
        }
    } while (gtk_tree_model_iter_next(model, &iter));

    return users;
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void user_tree_view_set_model(SuiUserList *self){
    GtkListStore *store;
    GtkTreeModel *filter;
    GtkTreeView *view;

    /* 3 columns: user, icon, type */
    self->user_list_store = gtk_list_store_new(3,
            G_TYPE_STRING,
            G_TYPE_STRING,
            G_TYPE_POINTER);
    store = self->user_list_store;
    view = self->user_tree_view;

    self->user_tree_model_filter = gtk_tree_model_filter_new(
            GTK_TREE_MODEL(store), NULL);
    filter = self->user_tree_model_filter;

    gtk_tree_sortable_set_default_sort_func(
            GTK_TREE_SORTABLE(store),
            user_list_store_sort_func, NULL, NULL);
    gtk_tree_sortable_set_sort_column_id(
            GTK_TREE_SORTABLE(store),
            GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
            GTK_SORT_ASCENDING);
    gtk_tree_view_set_model(view, filter);
}

static void stat_label_update_stat(SuiUserList *self){
    char *stat;

    stat = g_strdup_printf(
            _("Users: %d, <span color=\"#157915\">%d@</span>,"
                         "<span color=\"#856117\">%d%%</span>,"
                         "<span color=\"#451984\">%d+</span>"),
            self->user_stat.total,
            self->user_stat.full_op,
            self->user_stat.half_op,
            self->user_stat.voiced);
    gtk_label_set_markup(self->stat_label, stat);
    g_free(stat);
}

static int user_list_store_sort_func(GtkTreeModel *model,
        GtkTreeIter *iter1, GtkTreeIter *iter2, gpointer user_data){
    int ret;
    SuiUser *user1;
    SuiUser *user2;

    user1 = sui_user_new_from_iter(GTK_LIST_STORE(model), iter1);
    user2 = sui_user_new_from_iter(GTK_LIST_STORE(model), iter2);

    ret = sui_user_compare(user1, user2);

    sui_user_free(user1);
    sui_user_free(user2);

    return ret;
}

static gboolean user_tree_view_on_popup(GtkWidget *widget,
        GdkEventButton *event, gpointer user_data){
    GtkTreeView *view;
    GtkTreeModel *model;
    GtkTreeModel *child_model;
    GtkTreeIter iter;
    GtkTreeIter child_iter;
    GtkTreeSelection *selection;
    SuiUser *user;
    SrnChatUser *chat_user;

    if (event->button != 3){
        return FALSE;
    }

    view = GTK_TREE_VIEW(widget);
    model = gtk_tree_view_get_model(view);
    selection = gtk_tree_view_get_selection(view);
    if (!gtk_tree_selection_get_selected(selection, &model, &iter)){
        /* If not row is selected, just return */
        return FALSE;
    }
    child_model = gtk_tree_model_filter_get_model(GTK_TREE_MODEL_FILTER(model));
    gtk_tree_model_filter_convert_iter_to_child_iter(
            GTK_TREE_MODEL_FILTER(model), &child_iter, &iter);

    user = sui_user_new_from_iter(GTK_LIST_STORE(child_model), &child_iter);
    chat_user = sui_user_get_ctx(user);
    g_return_val_if_fail(chat_user, FALSE);

    // TODO: impl SuiUserPanel
    nick_menu_popup(widget, event, chat_user->srv_user->nick);

    sui_user_free(user);

    return TRUE;
}

void user_list_store_on_row_changed(GtkTreeModel *tree_model, GtkTreePath *path,
        GtkTreeIter *iter, gpointer user_data){
    SuiUserList *self;

    self = SUI_USER_LIST(user_data);
    stat_label_update_stat(self);
}
