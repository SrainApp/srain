/**
 * @file srain_user_list.c
 * @brief Listbox used to display user list of channel
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-04-03
 */

#define __LOG_ON

#include <gtk/gtk.h>
#include <strings.h>

#include "ui_common.h"
#include "srain_user_list.h"
#include "theme.h"

#include "log.h"

struct _SrainUserList {
    GtkListBox parent;
};

struct _SrainUserListClass {
    GtkListBoxClass parent_class;
};

G_DEFINE_TYPE(SrainUserList, srain_user_list, GTK_TYPE_LIST_BOX);

static gint list_sort_func(GtkListBoxRow *row1, GtkListBoxRow *row2,
        gpointer user_data){
    UserType type1;
    UserType type2;
    const char *name1;
    const char *name2;
    GtkLabel *label1;
    GtkLabel *label2;

    label1 = GTK_LABEL(gtk_bin_get_child(GTK_BIN(row1)));
    label2 = GTK_LABEL(gtk_bin_get_child(GTK_BIN(row2)));

    type1 = (UserType)g_object_get_data(G_OBJECT(label1), "user-type");
    type2 = (UserType)g_object_get_data(G_OBJECT(label2), "user-type");

    if (type1 != type2) return type1 > type2;

    name1 = gtk_label_get_text(label1);
    name2 = gtk_label_get_text(label2);

    return strcasecmp(name1, name2);
}

static void srain_user_list_init(SrainUserList *self){
    gtk_list_box_set_selection_mode(GTK_LIST_BOX(self), GTK_SELECTION_NONE);
    gtk_list_box_set_sort_func(GTK_LIST_BOX(self), list_sort_func, NULL, NULL);
}

static void srain_user_list_class_init(SrainUserListClass *class){
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

    row = gtk_list_box_get_row_by_name(GTK_LIST_BOX(list), nick);
    if (row){
        LOG_FR("GtkListBoxRow %s already exist", nick);
        return -1;
    }

    label = GTK_LABEL(gtk_label_new(nick));
    gtk_widget_set_name(GTK_WIDGET(label), nick);
    gtk_label_set_xalign(label, 0.05);

    g_object_set_data(G_OBJECT(label), "user-type", (void *)type);

    gtk_list_box_add_unfocusable_row(GTK_LIST_BOX(list), GTK_WIDGET(label));

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
    GtkListBoxRow *row;

    row = gtk_list_box_get_row_by_name(GTK_LIST_BOX(list), nick);
    if (!row){
        LOG_FR("GtkListBoxRow %s no found", nick);
        return -1;
    }

    gtk_container_remove(GTK_CONTAINER(list), GTK_WIDGET(row));

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
 */
int srain_user_list_rename(SrainUserList *list, const char *old_nick,
                           const char *new_nick, UserType type){
    GtkLabel *label;
    GtkListBoxRow *row;

    row = gtk_list_box_get_row_by_name(GTK_LIST_BOX(list), new_nick);
    if (row && strcasecmp(old_nick, new_nick) != 0){
        LOG_FR("GtkListBoxRow %s already exist", new_nick);
        return -1;
    }
    row = gtk_list_box_get_row_by_name(GTK_LIST_BOX(list), old_nick);
    if (!row){
        LOG_FR("GtkListBoxRow %s no found", old_nick);
        return -1;
    }

    label = GTK_LABEL(gtk_bin_get_child(GTK_BIN(row)));
    g_object_set_data(G_OBJECT(label), "user-type", (void *)type);
    gtk_label_set_text(label, new_nick);
    gtk_widget_set_name(GTK_WIDGET(label), new_nick);

    return 0;
}

void srain_user_list_clear(SrainUserList *list){
    int len;
    GtkListBoxRow *row;

    len = g_list_length(
            gtk_container_get_children(GTK_CONTAINER(list)));
    // TODO: len is always 0
    LOG_FR("len: %d", len);

    while (len > 0){
        while (gtk_events_pending()) gtk_main_iteration();

        row = gtk_list_box_get_row_at_index(GTK_LIST_BOX(list), 0);
        if (GTK_IS_LIST_BOX_ROW(row)){
            gtk_container_remove(GTK_CONTAINER(list), GTK_WIDGET(row));
            len--;
        }
    }
}
