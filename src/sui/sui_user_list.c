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
#include <cairo-gobject.h>

#include "core/core.h"

#include "sui_user_list.h"
#include "nick_menu.h"

#include "log.h"
#include "i18n.h"

struct _SuiUserList {
    GtkBox parent;

    GtkLabel *stat_label;   // users statistics
    GtkTreeView *user_tree_view;
    GtkTreeViewColumn *user_tree_view_column;
    GtkCellRendererText *user_name_cell_renderer;
    GtkCellRendererPixbuf *user_icon_cell_renderer;

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
static void user_list_store_on_row_changed(GtkTreeModel *tree_model,
        GtkTreePath *path, GtkTreeIter *iter, gpointer user_data);
static void on_style_updated(SuiUserList *self, gpointer user_data);

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
    g_signal_connect(self, "style-updated",
            G_CALLBACK(on_style_updated), NULL);
}

static void sui_user_list_class_init(SuiUserListClass *class){
    GtkWidgetClass *widget_class;

    widget_class = GTK_WIDGET_CLASS(class);

    gtk_widget_class_set_template_from_resource(widget_class,
            "/im/srain/Srain/user_list.glade");

    gtk_widget_class_bind_template_child(widget_class, SuiUserList, stat_label);
    gtk_widget_class_bind_template_child(widget_class, SuiUserList, user_tree_view);
    gtk_widget_class_bind_template_child(widget_class, SuiUserList, user_tree_view_column);
    gtk_widget_class_bind_template_child(widget_class, SuiUserList, user_name_cell_renderer);
    gtk_widget_class_bind_template_child(widget_class, SuiUserList, user_icon_cell_renderer);
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
    sui_user_list_update_user(self, user);
}

void sui_user_list_rm_user(SuiUserList *self, SuiUser *user){
    // FIXME: A hack for correcting user statistic
    SrnChatUser *chat_user = sui_user_get_ctx(user);
    chat_user->type = SRN_CHAT_USER_TYPE_CHIGUA;

    self->user_stat.total--;
    sui_user_list_update_user(self, user);
    gtk_list_store_remove(self->user_list_store, (GtkTreeIter *)user);
    sui_user_set_list(user, NULL);
    sui_user_set_stat(user, NULL);
}

void sui_user_list_update_user(SuiUserList *self, SuiUser *user){
    sui_user_update(user,
            gtk_widget_get_style_context(GTK_WIDGET(self)),
            gtk_widget_get_window(GTK_WIDGET(self)));
}

void sui_user_list_clear(SuiUserList *self){
    gtk_list_store_clear(self->user_list_store);
    memset(&self->user_stat, 0, sizeof(self->user_stat));
}

GList* sui_user_list_get_users_by_prefix(SuiUserList *self, const char *prefix){
    GList *users;
    GtkTreeModel *model;
    GtkTreeIter iter;
    char *normalized_prefix;

    model = GTK_TREE_MODEL(self->user_list_store);
    if (!gtk_tree_model_get_iter_first(model, &iter)){
        return NULL;
    }

    normalized_prefix = g_utf8_strdown(prefix, -1);

    users = NULL;
    do {
        SuiUser *user;
        char *normalized_nickname;

        user = sui_user_new_from_iter(GTK_LIST_STORE(model), &iter);
        normalized_nickname = g_utf8_strdown(sui_user_get_nickname(user), -1);
        if (g_str_has_prefix(normalized_nickname, normalized_prefix)){
            users = g_list_append(users, user);
        } else {
            sui_user_free(user);
        }
        g_free(normalized_nickname);
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

    /* 4 columns: user, icon, model, type */
    self->user_list_store = gtk_list_store_new(4,
            G_TYPE_STRING,
            CAIRO_GOBJECT_TYPE_SURFACE,
            G_TYPE_POINTER,
            G_TYPE_INT);
    gtk_tree_view_column_add_attribute(self->user_tree_view_column,
            GTK_CELL_RENDERER(self->user_name_cell_renderer), "text", 0);
    gtk_tree_view_column_add_attribute(self->user_tree_view_column,
            GTK_CELL_RENDERER(self->user_icon_cell_renderer), "surface", 1);

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

static void user_list_store_on_row_changed(GtkTreeModel *tree_model,
        GtkTreePath *path, GtkTreeIter *iter, gpointer user_data){
    SuiUserList *self;

    self = SUI_USER_LIST(user_data);
    stat_label_update_stat(self);
}

static void on_style_updated(SuiUserList *self, gpointer user_data) {
    GtkTreeModel *model;
    GtkTreeIter iter;

    model = GTK_TREE_MODEL(self->user_list_store);
    if (!gtk_tree_model_get_iter_first(model, &iter)){
        return;
    }

    do {
        SuiUser *user;
        user = sui_user_new_from_iter(GTK_LIST_STORE(model), &iter);
        sui_user_set_stat(user, &self->user_stat);
        sui_user_list_update_user(self, user);
        sui_user_free(user);
    } while (gtk_tree_model_iter_next(model, &iter));
}
