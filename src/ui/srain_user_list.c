/**
 * @file srain_user_list.c
 * @brief listbox used to display user list of channel
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-04-03
 */

#include <gtk/gtk.h>
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

static void srain_user_list_init(SrainUserList *self){
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

    row = get_list_item_by_name(GTK_LIST_BOX(list), nick);
    if (row){
        ERR_FR("GtkListBoxRow %s already exist", nick);
        return -1;
    }

    if (type == IRC_USER_OP){
        button = GTK_BUTTON(gtk_button_new());
        image = GTK_IMAGE(gtk_image_new_from_file("img/op_icon.png"));
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

    row = GTK_LIST_BOX_ROW(gtk_list_box_row_new());
    // gtk_widget_set_halign(GTK_WIDGET(row), GTK_ALIGN_START);
    // TODO: align

    // gtk_button_set_focus_on_click is deprecated
    gtk_widget_set_can_focus(GTK_WIDGET(button), FALSE);
    gtk_widget_set_can_focus(GTK_WIDGET(row), FALSE);

    gtk_container_add(GTK_CONTAINER(row), GTK_WIDGET(button));
    gtk_container_add(GTK_CONTAINER(list), GTK_WIDGET(row));

    theme_apply(GTK_WIDGET(row));

    gtk_widget_show_all(GTK_WIDGET(row));

    return 0;
}

int srain_user_list_rm(SrainUserList *list, const char *nick){
    GtkListBoxRow *row;

    row = get_list_item_by_name(GTK_LIST_BOX(list), nick);
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
    row = get_list_item_by_name(GTK_LIST_BOX(list), old_nick);
    if (!row){
        ERR_FR("GtkListBoxRow %s no found", old_nick);
        return -1;
    }

    button = GTK_BUTTON(gtk_bin_get_child(GTK_BIN(row)));
    gtk_button_set_label(button, new_nick);
    gtk_widget_set_name(GTK_WIDGET(button), new_nick);

    return 0;
}
