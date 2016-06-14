/**
 * @file srain_user_list.c
 * @brief listbox used to display user list of channel
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-04-03
 */

#define __LOG_ON

#include <gtk/gtk.h>
#include <strings.h>
#include "ui_common.h"
#include "irc_magic.h"
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

static gint list_sort_func(GtkListBoxRow *row1, GtkListBoxRow *row2, gpointer user_data){
    gpointer is1_op;
    gpointer is2_op;
    const char *name1;
    const char *name2;
    GtkButton *button1;
    GtkButton *button2;

    button1 = GTK_BUTTON(gtk_bin_get_child(GTK_BIN(row1)));
    button2 = GTK_BUTTON(gtk_bin_get_child(GTK_BIN(row2)));

    is1_op = g_object_get_data(G_OBJECT(button1), "is-op");
    is2_op = g_object_get_data(G_OBJECT(button2), "is-op");

    if (is1_op && !is2_op) return -1;
    if (!is1_op && is2_op) return 1;

    name1 = gtk_button_get_label(button1);
    name2 = gtk_button_get_label(button2);

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
 * @brief srain_user_list_add add a nick into SrainUserList
 *
 * @param list
 * @param nick
 * @param type
 *
 * @return 0 if successed, -1 if failed
 */
int srain_user_list_add(SrainUserList *list, const char *nick, IRCUserType type){
    GtkImage *image;
    GtkButton *button;
    GtkListBoxRow *row;

    row = gtk_list_box_get_row_by_name(GTK_LIST_BOX(list), nick);
    if (row){
        LOG_FR("GtkListBoxRow %s already exist", nick);
        return -1;
    }

    /* NOTE: use a gobject associations table item to mark
     * whether it is a OP, uesd by func list_sort_func()
     */
    if (type == IRC_USER_OP){
        button = GTK_BUTTON(gtk_button_new());
        image = GTK_IMAGE(
                gtk_image_new_from_icon_name("srain-op", GTK_ICON_SIZE_BUTTON));
        g_object_set_data(G_OBJECT(button), "is-op", button);
    }
    else if (type == IRC_USER_PERSON){
        button = GTK_BUTTON(gtk_button_new());
        image = GTK_IMAGE(
                gtk_image_new_from_icon_name("srain-person", GTK_ICON_SIZE_BUTTON));
    } else {
        return -1;
    }

    gtk_button_set_label(button, nick);
    gtk_button_set_image(button, GTK_WIDGET(image));

    gtk_button_set_relief(button, GTK_RELIEF_NONE);
    gtk_widget_set_name(GTK_WIDGET(button), nick);
    gtk_widget_set_halign(GTK_WIDGET(button), GTK_ALIGN_START);

    gtk_list_box_add_unfocusable_row(GTK_LIST_BOX(list), GTK_WIDGET(button));

    return 0;
}

/**
 * @brief srain_user_list_rm remove a nick from SrainUserList
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
 * @brief srain_user_list_rename rename a nick in SrainUserList
 *
 * @param list
 * @param old_nick
 * @param new_nick
 *
 * @return 0 if successed, -1 if failed
 */
int srain_user_list_rename(SrainUserList *list,
        const char *old_nick, const char *new_nick){
    GtkButton *button;
    GtkListBoxRow *row;

    // TODO: person -> op
    row = gtk_list_box_get_row_by_name(GTK_LIST_BOX(list), old_nick);
    if (!row){
        LOG_FR("GtkListBoxRow %s no found", old_nick);
        return -1;
    }

    button = GTK_BUTTON(gtk_bin_get_child(GTK_BIN(row)));
    gtk_button_set_label(button, new_nick);
    gtk_widget_set_name(GTK_WIDGET(button), new_nick);

    return 0;
}

void srain_user_list_clear(SrainUserList *list){
    int len;
    GtkListBoxRow *row;

    len = g_list_length(
            gtk_container_get_children(GTK_CONTAINER(list)));
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
