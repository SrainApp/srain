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
#include "gtk_compat.h"
#include "nick_menu.h"

#include "log.h"
#include "i18n.h"

struct _SuiUserList {
    GtkBox parent;

    GtkLabel *stat_label;   // users statistics
#if GTK_MAJOR_VERSION >= 4
    GtkListBox *user_list_box;
    GHashTable *user_rows;
#else
    GtkTreeView *user_tree_view;
    GtkTreeViewColumn *user_tree_view_column;
    GtkCellRendererText *user_name_cell_renderer;
    GtkCellRendererPixbuf *user_icon_cell_renderer;
#endif

    /* Data model */
    SuiUserStat user_stat;
#if GTK_MAJOR_VERSION < 4
    GtkListStore *user_list_store;
    GtkTreeModel *user_tree_model_filter;   // FilterTreeModel of user_list_store
                                            // TODO: user search
#endif
};

struct _SuiUserListClass {
    GtkBoxClass parent_class;
};

static void user_view_set_model(SuiUserList *self);
static void stat_label_update_stat(SuiUserList *self);
#if GTK_MAJOR_VERSION < 4
static int user_list_store_sort_func(GtkTreeModel *model,
        GtkTreeIter *iter1, GtkTreeIter *iter2, gpointer user_data);
#endif
#if GTK_MAJOR_VERSION >= 4
static const char *user_type_marker(SrnChatUserType type);
static int user_list_box_sort_func(GtkListBoxRow *row1, GtkListBoxRow *row2,
        gpointer user_data);
static GtkListBoxRow *user_list_box_find_row(SuiUserList *self, SuiUser *user);
static GtkWidget *new_user_row(SuiUser *user);
static void update_user_row(SuiUserList *self, SuiUser *user);
#endif

#if GTK_MAJOR_VERSION >= 4
static void user_list_box_on_popup(GtkGestureClick *gesture, int n_press,
        double x, double y, gpointer user_data);
#else
static gboolean user_tree_view_on_popup(GtkWidget *widget,
        GdkEventButton *event, gpointer user_data);
#endif
#if GTK_MAJOR_VERSION < 4
static void user_list_store_on_row_changed(GtkTreeModel *tree_model,
        GtkTreePath *path, GtkTreeIter *iter, gpointer user_data);
#endif
static void on_style_updated(SuiUserList *self, gpointer user_data);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

G_DEFINE_TYPE(SuiUserList, sui_user_list, GTK_TYPE_BOX);

static void sui_user_list_init(SuiUserList *self){
#if GTK_MAJOR_VERSION >= 4
    GtkGesture *click;
#endif

    gtk_widget_init_template(GTK_WIDGET(self));

    user_view_set_model(self);
    stat_label_update_stat(self);

#if GTK_MAJOR_VERSION >= 4
    self->user_rows = g_hash_table_new(NULL, NULL);
    click = gtk_gesture_click_new();
    gtk_widget_add_controller(GTK_WIDGET(self->user_list_box),
            GTK_EVENT_CONTROLLER(click));
    g_signal_connect(click, "pressed",
            G_CALLBACK(user_list_box_on_popup), self);
#else
    g_signal_connect(self->user_tree_view, "button-press-event",
            G_CALLBACK(user_tree_view_on_popup), NULL);
    g_signal_connect(self->user_list_store, "row-changed",
            G_CALLBACK(user_list_store_on_row_changed), self);
    g_signal_connect(self, "style-updated",
            G_CALLBACK(on_style_updated), NULL);
#endif
    on_style_updated(self, NULL);
}

static void sui_user_list_class_init(SuiUserListClass *class){
    GtkWidgetClass *widget_class;

    widget_class = GTK_WIDGET_CLASS(class);

#if GTK_MAJOR_VERSION >= 4
    gtk_widget_class_set_template_from_resource(widget_class,
            "/im/srain/Srain/user_list.ui");
#else
    gtk_widget_class_set_template_from_resource(widget_class,
            "/im/srain/Srain/user_list.glade");
#endif

    gtk_widget_class_bind_template_child(widget_class, SuiUserList, stat_label);
#if GTK_MAJOR_VERSION >= 4
    gtk_widget_class_bind_template_child(widget_class, SuiUserList, user_list_box);
#else
    gtk_widget_class_bind_template_child(widget_class, SuiUserList, user_tree_view);
    gtk_widget_class_bind_template_child(widget_class, SuiUserList, user_tree_view_column);
    gtk_widget_class_bind_template_child(widget_class, SuiUserList, user_name_cell_renderer);
    gtk_widget_class_bind_template_child(widget_class, SuiUserList, user_icon_cell_renderer);
#endif
}

/*****************************************************************************
 * Expored functions
 *****************************************************************************/

SuiUserList* sui_user_list_new(void){
    return g_object_new(SUI_TYPE_USER_LIST, NULL);
}

void sui_user_list_add_user(SuiUserList *self, SuiUser *user){
    sui_user_set_stat(user, &self->user_stat);
    self->user_stat.total++;
#if GTK_MAJOR_VERSION >= 4
    {
        GtkWidget *row;

        row = new_user_row(user);
        g_hash_table_insert(self->user_rows, sui_user_get_ctx(user), row);
        gtk_list_box_insert(self->user_list_box, row, -1);
    }
#else
    gtk_list_store_append(self->user_list_store, (GtkTreeIter *)user);
    sui_user_set_list(user, self->user_list_store);
#endif
    sui_user_list_update_user(self, user);
}

void sui_user_list_rm_user(SuiUserList *self, SuiUser *user){
    // FIXME: A hack for correcting user statistic
    SrnChatUser *chat_user = sui_user_get_ctx(user);
    chat_user->type = SRN_CHAT_USER_TYPE_CHIGUA;

    self->user_stat.total--;
#if GTK_MAJOR_VERSION >= 4
    sui_user_update(user, NULL, NULL);
    {
        GtkListBoxRow *row;

        row = user_list_box_find_row(self, user);
        if (row){
            gtk_list_box_remove(self->user_list_box, GTK_WIDGET(row));
            g_hash_table_remove(self->user_rows, sui_user_get_ctx(user));
        }
    }
#else
    sui_user_list_update_user(self, user);
    gtk_list_store_remove(self->user_list_store, (GtkTreeIter *)user);
#endif
    sui_user_set_stat(user, NULL);
#if GTK_MAJOR_VERSION < 4
    sui_user_set_list(user, NULL);
#endif
    stat_label_update_stat(self);
}

void sui_user_list_update_user(SuiUserList *self, SuiUser *user){
#if GTK_MAJOR_VERSION >= 4
    sui_user_update(user, NULL,
            srn_gtk_widget_get_surface(GTK_WIDGET(self)));
#else
    sui_user_update(user,
            gtk_widget_get_style_context(GTK_WIDGET(self)),
            srn_gtk_widget_get_surface(GTK_WIDGET(self)));
#endif
#if GTK_MAJOR_VERSION >= 4
    update_user_row(self, user);
#endif
}

void sui_user_list_clear(SuiUserList *self){
#if GTK_MAJOR_VERSION < 4
    gtk_list_store_clear(self->user_list_store);
#else
    while (gtk_widget_get_first_child(GTK_WIDGET(self->user_list_box))){
        gtk_list_box_remove(self->user_list_box,
                gtk_widget_get_first_child(GTK_WIDGET(self->user_list_box)));
    }
    g_hash_table_remove_all(self->user_rows);
#endif
    memset(&self->user_stat, 0, sizeof(self->user_stat));
}

GList* sui_user_list_get_users_by_prefix(SuiUserList *self, const char *prefix){
    GList *users;
    char *normalized_prefix;

    normalized_prefix = g_utf8_strdown(prefix, -1);

    users = NULL;
#if GTK_MAJOR_VERSION >= 4
    for (GtkWidget *child = gtk_widget_get_first_child(GTK_WIDGET(self->user_list_box));
            child != NULL;
            child = gtk_widget_get_next_sibling(child)){
        SrnChatUser *chat_user;
        SuiUser *user;
        char *normalized_nickname;

        chat_user = g_object_get_data(G_OBJECT(child), "chat-user");
        if (!chat_user){
            continue;
        }

        user = sui_user_new(chat_user);
        normalized_nickname = g_utf8_strdown(sui_user_get_nickname(user), -1);
        if (g_str_has_prefix(normalized_nickname, normalized_prefix)){
            users = g_list_append(users, user);
        } else {
            sui_user_free(user);
        }
        g_free(normalized_nickname);
    }
#else
    GtkTreeModel *model;
    GtkTreeIter iter;

    model = GTK_TREE_MODEL(self->user_list_store);
    if (!gtk_tree_model_get_iter_first(model, &iter)){
        g_free(normalized_prefix);
        return NULL;
    }

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
#endif

    g_free(normalized_prefix);
    return users;
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void user_view_set_model(SuiUserList *self){
#if GTK_MAJOR_VERSION >= 4
    gtk_list_box_set_sort_func(self->user_list_box,
            user_list_box_sort_func, self, NULL);
#else
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
#endif
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

#if GTK_MAJOR_VERSION < 4
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
#endif

#if GTK_MAJOR_VERSION >= 4
static void user_list_box_on_popup(GtkGestureClick *gesture, int n_press,
        double x, double y, gpointer user_data){
    GtkWidget *widget;
    GtkListBoxRow *row;
    guint button;
    SrnChatUser *chat_user;
    SuiUserList *self;

    self = SUI_USER_LIST(user_data);
    button = gtk_gesture_single_get_current_button(GTK_GESTURE_SINGLE(gesture));
    if (button != GDK_BUTTON_SECONDARY){
        return;
    }

    widget = gtk_event_controller_get_widget(GTK_EVENT_CONTROLLER(gesture));
    row = gtk_list_box_get_row_at_y(self->user_list_box, (int)y);
    if (!row){
        return;
    }
    gtk_list_box_select_row(self->user_list_box, row);
    chat_user = g_object_get_data(G_OBJECT(row), "chat-user");
    g_return_if_fail(chat_user);

    nick_menu_popup(widget, chat_user->srv_user->nick);
}
#else
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
    nick_menu_popup(widget, chat_user->srv_user->nick);

    sui_user_free(user);

    return TRUE;
}
#endif

#if GTK_MAJOR_VERSION < 4
static void user_list_store_on_row_changed(GtkTreeModel *tree_model,
        GtkTreePath *path, GtkTreeIter *iter, gpointer user_data){
    SuiUserList *self;

    self = SUI_USER_LIST(user_data);
    stat_label_update_stat(self);
}
#endif

static void on_style_updated(SuiUserList *self, gpointer user_data) {
#if GTK_MAJOR_VERSION >= 4
    for (GtkWidget *child = gtk_widget_get_first_child(GTK_WIDGET(self->user_list_box));
            child != NULL;
            child = gtk_widget_get_next_sibling(child)){
        SrnChatUser *chat_user;
        SuiUser *user;

        chat_user = g_object_get_data(G_OBJECT(child), "chat-user");
        if (!chat_user){
            continue;
        }

        user = sui_user_new(chat_user);
        sui_user_set_stat(user, &self->user_stat);
        sui_user_list_update_user(self, user);
        sui_user_free(user);
    }
#else
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
#endif
}

#if GTK_MAJOR_VERSION >= 4
static const char *user_type_marker(SrnChatUserType type){
    switch (type) {
        case SRN_CHAT_USER_TYPE_ADMIN:
        case SRN_CHAT_USER_TYPE_OWNER:
        case SRN_CHAT_USER_TYPE_FULL_OP:
            return "@";
        case SRN_CHAT_USER_TYPE_HALF_OP:
            return "%";
        case SRN_CHAT_USER_TYPE_VOICED:
            return "+";
        default:
            return "";
    }
}

static int user_list_box_sort_func(GtkListBoxRow *row1, GtkListBoxRow *row2,
        gpointer user_data){
    SrnChatUser *chat_user1;
    SrnChatUser *chat_user2;
    SuiUser *user1;
    SuiUser *user2;
    int ret;

    chat_user1 = g_object_get_data(G_OBJECT(row1), "chat-user");
    chat_user2 = g_object_get_data(G_OBJECT(row2), "chat-user");
    g_return_val_if_fail(chat_user1 && chat_user2, 0);

    user1 = sui_user_new(chat_user1);
    user2 = sui_user_new(chat_user2);
    ret = sui_user_compare(user1, user2);
    sui_user_free(user1);
    sui_user_free(user2);

    return ret;
}

static GtkListBoxRow *user_list_box_find_row(SuiUserList *self, SuiUser *user){
    return g_hash_table_lookup(self->user_rows, sui_user_get_ctx(user));
}

static GtkWidget *new_user_row(SuiUser *user){
    GtkWidget *row;
    GtkWidget *box;
    GtkWidget *type_label;
    GtkWidget *nick_label;
    SrnChatUser *chat_user;

    chat_user = sui_user_get_ctx(user);
    row = gtk_list_box_row_new();
    box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 6);
    type_label = gtk_label_new(user_type_marker(chat_user->type));
    nick_label = gtk_label_new(chat_user->srv_user->nick);

    gtk_widget_set_halign(type_label, GTK_ALIGN_START);
    gtk_widget_set_halign(nick_label, GTK_ALIGN_START);
    gtk_widget_set_hexpand(nick_label, TRUE);

    gtk_box_append(GTK_BOX(box), type_label);
    gtk_box_append(GTK_BOX(box), nick_label);
    gtk_list_box_row_set_child(GTK_LIST_BOX_ROW(row), box);

    g_object_set_data(G_OBJECT(row), "chat-user", chat_user);
    g_object_set_data(G_OBJECT(row), "type-label", type_label);
    g_object_set_data(G_OBJECT(row), "nick-label", nick_label);

    return row;
}

static void update_user_row(SuiUserList *self, SuiUser *user){
    GtkListBoxRow *row;
    GtkWidget *type_label;
    GtkWidget *nick_label;
    SrnChatUser *chat_user;

    row = user_list_box_find_row(self, user);
    if (!row){
        return;
    }

    chat_user = sui_user_get_ctx(user);
    type_label = g_object_get_data(G_OBJECT(row), "type-label");
    nick_label = g_object_get_data(G_OBJECT(row), "nick-label");

    gtk_label_set_text(GTK_LABEL(type_label), user_type_marker(chat_user->type));
    gtk_label_set_text(GTK_LABEL(nick_label), chat_user->srv_user->nick);
    g_object_set_data(G_OBJECT(row), "chat-user", chat_user);
    gtk_list_box_invalidate_sort(self->user_list_box);
}
#endif
