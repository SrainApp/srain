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
 * @file srain_user_list.c
 * @brief Widget for listing all participants of a chat
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version 0.06.2
 * @date 2016-04-03
 *
 * NOTE: It is allowed to add duplicate user, deduplication should be
 * do in the upper layer.
 */

#include <gtk/gtk.h>
#include <string.h>
#include <strings.h>

#include "theme.h"
#include "sui_common.h"
#include "nick_menu.h"
#include "srain_user_list.h"

#include "log.h"
#include "i18n.h"

#define USER_LIST_STORE_COL_USER    0
#define USER_LIST_STORE_COL_ICON    1
#define USER_LIST_STORE_COL_TYPE    2

struct _SrainUserList {
    GtkBox parent;

    int num_total;
    int num_type[USER_TYPE_MAX];

    GtkTreeView *user_tree_view;
    GtkLabel *stat_label;   // users statistics

    /* Data model */
    GtkListStore *user_list_store;
    GtkTreeModel *user_tree_model_filter;   // FilterTreeModel of user_list_store
                                            // TODO: user search
};

struct _SrainUserListClass {
    GtkBoxClass parent_class;
};

G_DEFINE_TYPE(SrainUserList, srain_user_list, GTK_TYPE_BOX);

static void srain_user_list_init(SrainUserList *self);
static void srain_user_list_class_init(SrainUserListClass *class);
static void user_tree_view_set_model(SrainUserList *list);
static void stat_label_update_stat(SrainUserList *list);
static const char *type_to_icon(UserType type);

static int user_list_store_sort_func(GtkTreeModel *model,
        GtkTreeIter *iter1, GtkTreeIter *iter2, gpointer user_data);
static gboolean user_tree_view_on_popup(GtkWidget *widget,
        GdkEventButton *event, gpointer user_data);

SrainUserList* srain_user_list_new(void){
    return g_object_new(SRAIN_TYPE_USER_LIST, NULL);
}

/**
 * @brief Add a user into SrainUserList
 *
 * @param list
 * @param user
 * @param type
 *
 * @return SRN_OK if successed, SRN_ERR if failed
 */
SrnRet srain_user_list_add(SrainUserList *list, const char *user, UserType type){
    const char *icon;
    GtkTreeIter iter;
    GtkListStore *store;

    icon = type_to_icon(type);
    if (!icon) {
        return SRN_ERR;
    }

    /* Insert user */
    store = list->user_list_store;
    gtk_list_store_append(store, &iter);
    gtk_list_store_set(store, &iter,
            USER_LIST_STORE_COL_USER, user,
            USER_LIST_STORE_COL_ICON, icon,
            USER_LIST_STORE_COL_TYPE, type,
            -1);

    /* Update stat */
    list->num_total++;
    list->num_type[type]++;
    stat_label_update_stat(list);

    return SRN_OK;
}

/**
 * @brief Remove a nick from SrainUserList
 *
 * @param list
 * @param nick
 *
 * @return SRN_OK if successed, SRN_ERR if failed
 */
SrnRet srain_user_list_rm(SrainUserList *list, const char *user){
    bool found;
    GtkTreeIter iter;
    GtkTreeModel *model;
    UserType type;

    model = GTK_TREE_MODEL(list->user_list_store);

    if (!gtk_tree_model_get_iter_first(model, &iter)){
        ERR_FR("Failed to get first iter");
        return SRN_ERR;
    }

    found = FALSE;
    do {
        char *useri = NULL;
        gtk_tree_model_get(model, &iter,
                USER_LIST_STORE_COL_USER, &useri,
                USER_LIST_STORE_COL_TYPE, &type,
                -1);
        if (useri){
            found = (g_ascii_strcasecmp(user, useri) == 0);
            g_free(useri);
        }
        if (found) {
            break;
        }
    } while (gtk_tree_model_iter_next(model, &iter));

    if (!found){
        DBG_FR("'%s' not found");
        return SRN_ERR;
    }

    if (!gtk_list_store_remove(GTK_LIST_STORE(model), &iter)){
        ERR_FR("Failed to remove iter");
    }

    /* Update stat */
    list->num_total--;
    list->num_type[type]--;
    stat_label_update_stat(list);

    return SRN_OK;
}

/**
 * @brief Rename a user in SrainUserList
 *
 * @param list
 * @param old_user
 * @param newuser
 *
 * @return SRN_OK if successed, SRN_ERR if failed
 *
 * If `old_user` == `new_user`, change its user type.
 */
SrnRet srain_user_list_rename(SrainUserList *list, const char *old_user,
                           const char *new_user, UserType new_type){
    bool found;
    GtkTreeIter iter;
    GtkTreeModel *model;
    UserType old_type;

    model = GTK_TREE_MODEL(list->user_list_store);

    if (!gtk_tree_model_get_iter_first(model, &iter)){
        ERR_FR("Failed to get first iter");
        return SRN_ERR;
    }

    found = FALSE;
    do {
        char *useri = NULL;
        gtk_tree_model_get(model, &iter,
                USER_LIST_STORE_COL_USER, &useri,
                USER_LIST_STORE_COL_TYPE, &old_type,
                -1);
        if (useri){
            found = (g_ascii_strcasecmp(old_user, useri) == 0);
            g_free(useri);
        }
        if (found) {
            break;
        }
    } while (gtk_tree_model_iter_next(model, &iter));

    if (!found){
        DBG_FR("'%s' not found", old_user);
        return SRN_ERR;
    }

    if (g_ascii_strcasecmp(old_user, new_user) == 0) {
        /* Change type */
        const char *icon = type_to_icon(new_type);
        if (!icon) {
            return SRN_ERR;
        }
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                USER_LIST_STORE_COL_TYPE, new_type,
                USER_LIST_STORE_COL_ICON, icon,
                -1);

        /* Update stat */
        list->num_type[old_type]--;
        list->num_type[new_type]++;
        stat_label_update_stat(list);
    } else {
        /* Change user */
        gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                USER_LIST_STORE_COL_USER, new_user,
                -1);
    }

    return SRN_OK;
}

void srain_user_list_clear(SrainUserList *list){
    /* Clear data */
    gtk_list_store_clear(list->user_list_store);

    /* Clear stat */
    list->num_total = 0;
    for (int i = 0; i < USER_TYPE_MAX; i++){
        list->num_type[i] = 0;
    }
    stat_label_update_stat(list);
}

/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void srain_user_list_init(SrainUserList *self){
    gtk_widget_init_template(GTK_WIDGET(self));

    self->num_total = 0;
    for (int i = 0; i < USER_TYPE_MAX; i++){
        self->num_type[i] = 0;
    }

    user_tree_view_set_model(self);
    stat_label_update_stat(self);

    g_signal_connect(self->user_tree_view, "button-press-event",
            G_CALLBACK(user_tree_view_on_popup), self);
}

static void srain_user_list_class_init(SrainUserListClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/im/srain/Srain/user_list.glade");

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainUserList, user_tree_view);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainUserList, stat_label);
}

static void user_tree_view_set_model(SrainUserList *list){
    GtkListStore *store;
    GtkTreeModel *filter;
    GtkTreeView *view;

    /* 3 columns: user, icon, type */
    list->user_list_store = gtk_list_store_new(3,
            G_TYPE_STRING,
            G_TYPE_STRING,
            G_TYPE_INT);
    store = list->user_list_store;
    view = list->user_tree_view;

    list->user_tree_model_filter = gtk_tree_model_filter_new(
            GTK_TREE_MODEL(store), NULL);
    filter = list->user_tree_model_filter;

    gtk_tree_sortable_set_default_sort_func(
            GTK_TREE_SORTABLE(store),
            user_list_store_sort_func, list, NULL);
    gtk_tree_sortable_set_sort_column_id(
            GTK_TREE_SORTABLE(store),
            GTK_TREE_SORTABLE_DEFAULT_SORT_COLUMN_ID,
            GTK_SORT_ASCENDING);
    gtk_tree_view_set_model(view, filter);
}

static void stat_label_update_stat(SrainUserList *list){
    char *stat;

    stat = g_strdup_printf(
            _("Users: %d, <span color=\"#157915\">%d@</span>,"
                         "<span color=\"#856117\">%d%%</span>,"
                         "<span color=\"#451984\">%d+</span>"),
            list->num_total,
            // list->num_type[USER_OWNER],
            // list->num_type[USER_ADMIN],
            list->num_type[USER_FULL_OP],
            list->num_type[USER_HALF_OP],
            list->num_type[USER_VOICED]);

    gtk_label_set_markup(list->stat_label, stat);
}

static const char *type_to_icon(UserType type){
    const char *icon;

    switch (type){
        case USER_ADMIN:
        case USER_OWNER:
        case USER_FULL_OP:
            icon = "srain-user-full-op";
            break;
        case USER_HALF_OP:
            icon = "srain-user-half-op";
            break;
        case USER_VOICED:
            icon = "srain-user-voiced";
            break;
        case USER_CHIGUA:
            icon = "srain-person";
            break;
        default:
            ERR_FR("Unknown UserType: %d", type);
            icon = NULL;
            break;
    }

    return icon;
}
static int user_list_store_sort_func(GtkTreeModel *model,
        GtkTreeIter *iter1, GtkTreeIter *iter2, gpointer user_data){
    int res;
    char *name1;
    char *name2;
    UserType type1;
    UserType type2;

    gtk_tree_model_get(model, iter1,
            USER_LIST_STORE_COL_USER, &name1,
            USER_LIST_STORE_COL_TYPE, &type1,
            -1);
    gtk_tree_model_get(model, iter2,
            USER_LIST_STORE_COL_USER, &name2,
            USER_LIST_STORE_COL_TYPE, &type2,
            -1);

    if (type1 != type2) {
        res = type1 > type2;
    } else {
        res = g_ascii_strcasecmp(name1, name2);
    }
    DBG_FR("Sorting '%s' and '%s', res: %d", name1, name2, res);

    g_free(name1);
    g_free(name2);

    return res;
}

static gboolean user_tree_view_on_popup(GtkWidget *widget,
        GdkEventButton *event, gpointer user_data){
    bool pop;
    char *user = NULL;
    GtkTreeIter       iter;
    GtkTreeModel     *model;
    GtkTreeSelection *selection;
    SrainUserList *list;

    list = user_data;
    model = gtk_tree_view_get_model(list->user_tree_view);
    selection = gtk_tree_view_get_selection(list->user_tree_view);
    if (!gtk_tree_selection_get_selected(selection, &model, &iter)){
        /* If not row is selected, just return */
        return FALSE;
    }

    gtk_tree_model_get(model, &iter,
            USER_LIST_STORE_COL_USER, &user,
            -1);
    if (!user) {
        return FALSE;
    }

    if (event->button == 3){
        nick_menu_popup(GTK_WIDGET(list), event, user);
        pop = TRUE;
    } else {
        pop = FALSE;
    }

    // FIXME:
    // g_free(user);

    return pop;
}
