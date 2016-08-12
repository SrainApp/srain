/**
 * @file srain_user_list.c
 * @brief Listbox used to display user list of channel
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-04-03
 */

// #define __DBG_ON
#define __LOG_ON

#include <gtk/gtk.h>
#include <string.h>
#include <strings.h>

#include "ui_common.h"
#include "srain_user_list.h"
#include "theme.h"

#include "log.h"
#include "i18n.h"

struct _SrainUserList {
    GtkBox parent;

    int num_total;
    int num_type[USER_TYPE_MAX];

    GtkListBox *list_box;
    GtkLabel *stat_label;   // users statistics
};

struct _SrainUserListClass {
    GtkBoxClass parent_class;
};

G_DEFINE_TYPE(SrainUserList, srain_user_list, GTK_TYPE_BOX);

static gint list_sort_func(GtkListBoxRow *row1, GtkListBoxRow *row2,
        gpointer user_data){
    const char *name1;
    const char *name2;
    UserType type1;
    UserType type2;

    type1 = (UserType)g_object_get_data(G_OBJECT(row1), "user-type");
    type2 = (UserType)g_object_get_data(G_OBJECT(row1), "user-type");

    if (type1 != type2) return type1 > type2;

    name1 = gtk_widget_get_name(GTK_WIDGET(row1));
    name2 = gtk_widget_get_name(GTK_WIDGET(row2));

    return strcasecmp(name1, name2);
}

static void srain_user_list_update_stat(SrainUserList *list){
    char stat[128];

    snprintf(stat, sizeof(stat), _("Users: %d, %d@, %d%% %d+"),
            list->num_total,
            // list->num_type[USER_OWNER],
            // list->num_type[USER_ADMIN],
            list->num_type[USER_FULL_OP],
            list->num_type[USER_HALF_OP],
            list->num_type[USER_VOICED]
            );

    // TODO: set markup
    gtk_label_set_text(list->stat_label, stat);
}

static void srain_user_list_init(SrainUserList *self){
    int i;

    gtk_widget_init_template(GTK_WIDGET(self));

    self->num_total = 0;
    for (i = 0; i < USER_TYPE_MAX; i++) self->num_type[i] = 0;

    gtk_list_box_set_sort_func(GTK_LIST_BOX(self->list_box),
            list_sort_func, NULL, NULL);

    srain_user_list_update_stat(self);
}

static void srain_user_list_class_init(SrainUserListClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/user_list.glade");

    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainUserList, list_box);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainUserList, stat_label);
}

SrainUserList* srain_user_list_new(void){
    return g_object_new(SRAIN_TYPE_USER_LIST, NULL);
}

/**
 * @brief Add a nick into SrainUserList
 *
 * @param list
 * @param nick
 * @param type
 *
 * @return 0 if successed, -1 if failed
 */
int srain_user_list_add(SrainUserList *list, const char *nick, UserType type){
    GtkLabel *label;
    // GtkImage *image;
    // GtkButton *button;
    GtkListBoxRow *row;

    if (type >= USER_TYPE_MAX){
        ERR_FR("nick: %s, type: %d, Unknown UserType", nick, type);
        return -1;
    }

    row = gtk_list_box_get_row_by_name(GTK_LIST_BOX(list->list_box), nick);
    if (row){
        DBG_FR("GtkListBoxRow %s already exist", nick);
        return -1;
    }

    label = GTK_LABEL(gtk_label_new(nick));
    gtk_label_set_xalign(label, 0.05);

    row = gtk_list_box_add_unfocusable_row(GTK_LIST_BOX(list->list_box), GTK_WIDGET(label));
    gtk_widget_set_name(GTK_WIDGET(row), nick);
    g_object_set_data(G_OBJECT(row), "user-type", (void *)type);

    list->num_total++;
    list->num_type[type]++;

    srain_user_list_update_stat(list);

    return 0;
}

/**
 * @brief Remove a nick from SrainUserList
 *
 * @param list
 * @param nick
 *
 * @return 0 if successed, -1 if failed
 */
int srain_user_list_rm(SrainUserList *list, const char *nick){
    UserType type;
    GtkListBoxRow *row;

    row = gtk_list_box_get_row_by_name(GTK_LIST_BOX(list->list_box), nick);
    if (!row){
        DBG_FR("GtkListBoxRow %s no found", nick);
        return -1;
    }

    type = (UserType)g_object_get_data(G_OBJECT(row), "user-type");
    gtk_container_remove(GTK_CONTAINER(list->list_box), GTK_WIDGET(row));

    list->num_total--;
    list->num_type[type]--;

    srain_user_list_update_stat(list);

    return 0;
}

/**
 * @brief Rename a nick in SrainUserList
 *
 * @param list
 * @param old_nick
 * @param new_nick
 *
 * @return 0 if successed, -1 if failed
 *
 * If `old_nick` == `new_nick`, change its user type.
 */
int srain_user_list_rename(SrainUserList *list, const char *old_nick,
                           const char *new_nick, UserType type){
    UserType old_type;
    GtkLabel *label;
    GtkListBoxRow *row;

    if (type >= USER_TYPE_MAX){
        ERR_FR("old_nick: %s, type: %d, Unknown UserType", old_nick, type);
        return -1;
    }

    row = gtk_list_box_get_row_by_name(GTK_LIST_BOX(list->list_box), new_nick);
    if (row && strcasecmp(old_nick, new_nick) != 0){
        DBG_FR("GtkListBoxRow %s already exist", new_nick);
        return -1;
    }
    row = gtk_list_box_get_row_by_name(GTK_LIST_BOX(list->list_box), old_nick);
    if (!row){
        DBG_FR("GtkListBoxRow %s no found", old_nick);
        return -1;
    }

    if (strcmp(new_nick, old_nick)){
        /* Rename */
        gtk_widget_set_name(GTK_WIDGET(row), new_nick);
        label = GTK_LABEL(gtk_bin_get_child(GTK_BIN(row)));
        gtk_label_set_text(label, new_nick);
    } else {
        /* Change someone's UserType */
        old_type = (UserType)g_object_get_data(G_OBJECT(row), "user-type");
        g_object_set_data(G_OBJECT(row), "user-type", (void *)type);

        list->num_type[old_type]--;
        list->num_type[type]++;
    }

    srain_user_list_update_stat(list);

    return 0;
}

void srain_user_list_clear(SrainUserList *list){
    int len;
    GtkListBoxRow *row;

    len = g_list_length(
            gtk_container_get_children(GTK_CONTAINER(list->list_box)));
    // TODO: len is always 0
    LOG_FR("TODO: len: %d", len);

    while (len > 0){
        while (gtk_events_pending()) gtk_main_iteration();

        row = gtk_list_box_get_row_at_index(GTK_LIST_BOX(list->list_box), 0);
        if (GTK_IS_LIST_BOX_ROW(row)){
            gtk_container_remove(GTK_CONTAINER(list->list_box), GTK_WIDGET(row));
            len--;
        }
    }
}
