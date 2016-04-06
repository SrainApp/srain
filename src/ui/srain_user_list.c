/**
 * @file srain_user_list.c
 * @brief listbox used to display user list of channel
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-04-03
 */

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

int srain_user_list_add(SrainUserList *list, const char *nick, IRCUserType type){
    GtkImage *image;
    GtkButton *button;
    GtkListBoxRow *row;

    row = gtk_list_box_get_row_by_name(GTK_LIST_BOX(list), nick);
    if (row){
        ERR_FR("GtkListBoxRow %s already exist", nick);
        return -1;
    }

    /* NOTE: use a gobject associations table item to mark
     * whether it is a OP, uesd by func list_sort_func()
     */
    if (type == IRC_USER_OP){
        button = GTK_BUTTON(gtk_button_new());
        image = GTK_IMAGE(gtk_image_new_from_file("img/op_icon.png"));
        g_object_set_data(G_OBJECT(button), "is-op", button);
    }
    else if (type == IRC_USER_PERSON){
        button = GTK_BUTTON(gtk_button_new());
        image = GTK_IMAGE(gtk_image_new_from_file("img/person_icon.png"));
    } else {
        return -1;
    }

    gtk_button_set_label(button, nick);
    gtk_button_set_image(button, GTK_WIDGET(image));
    gtk_widget_set_name(GTK_WIDGET(button), nick);
    gtk_widget_set_halign(GTK_WIDGET(button), GTK_ALIGN_START);

    gtk_list_box_add_unfocusable_row(GTK_LIST_BOX(list), GTK_WIDGET(button));

    return 0;
}

int srain_user_list_rm(SrainUserList *list, const char *nick){
    GtkListBoxRow *row;

    row = gtk_list_box_get_row_by_name(GTK_LIST_BOX(list), nick);
    if (!row){
        ERR_FR("GtkListBoxRow %s no found", nick);
        return -1;
    }

    gtk_container_remove(GTK_CONTAINER(list), GTK_WIDGET(row));

    return 0;
}

int srain_user_list_rename(SrainUserList *list,
        const char *old_nick, const char *new_nick){
    GtkButton *button;
    GtkListBoxRow *row;

    // TODO: person -> op
    row = gtk_list_box_get_row_by_name(GTK_LIST_BOX(list), old_nick);
    if (!row){
        ERR_FR("GtkListBoxRow %s no found", old_nick);
        return -1;
    }

    button = GTK_BUTTON(gtk_bin_get_child(GTK_BIN(row)));
    gtk_button_set_label(button, new_nick);
    gtk_widget_set_name(GTK_WIDGET(button), new_nick);

    return 0;
}
