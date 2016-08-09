/**
 * @file srain_stack_sidebar_item.c
 * @brief item class of SrainStackSidebar
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-07
 */

#define __LOG_ON

#include <gtk/gtk.h>
#include <string.h>

#include "srain_stack_sidebar_item.h"

#include "meta.h"
#include "log.h"

struct _SrainStackSidebarItem {
    GtkBox parent;
    GtkImage *image;
    GtkLabel *chan_label;
    GtkLabel *server_label;
    GtkLabel *recentmsg_label;
    GtkLabel *count_label;
};

struct _SrainStackSidebarItemClass {
    GtkBoxClass parent_class;
};

G_DEFINE_TYPE(SrainStackSidebarItem, srain_stack_sidebar_item, GTK_TYPE_BOX);

static void srain_stack_sidebar_item_init(SrainStackSidebarItem *self){
    gtk_widget_init_template(GTK_WIDGET(self));
}

static void srain_stack_sidebar_item_class_init(SrainStackSidebarItemClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/stack_sidebar_item.glade");
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainStackSidebarItem, image);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainStackSidebarItem, chan_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainStackSidebarItem, recentmsg_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainStackSidebarItem, server_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainStackSidebarItem, count_label);
}

SrainStackSidebarItem *srain_stack_sidebar_item_new(const char *server_name, const char *chan_name){
    SrainStackSidebarItem *item;

    g_return_val_if_fail(chan_name, NULL);
    g_return_val_if_fail(server_name, NULL);

    item = g_object_new(SRAIN_TYPE_STACK_SIDEBAR_ITEM, NULL);

    gtk_label_set_text(item->chan_label, chan_name);
    gtk_label_set_text(item->server_label, server_name);

    // is a channel TODO: ui shouldn't konw anything about irc protocol
    if (chan_name[0] == '#'){
        gtk_image_set_from_icon_name(item->image, "srain-chan", GTK_ICON_SIZE_BUTTON);
    }
    else if (strcmp(chan_name, "Server ") == 0){
        // is a server
        gtk_image_set_from_icon_name(item->image, "srain-server", GTK_ICON_SIZE_BUTTON);
    } else {
        // is a normal user
        gtk_image_set_from_icon_name(item->image, "srain-person", GTK_ICON_SIZE_BUTTON);
    }

    return item;
}

void srain_stack_sidebar_item_recentmsg_update(
        SrainStackSidebarItem *item, const char *nick, const char *msg){
    char buf[512];

    if (nick){
        snprintf(buf, sizeof(buf), "%s: %s", nick, msg);
        gtk_label_set_text(item->recentmsg_label, buf);
    } else {
        gtk_label_set_text(item->recentmsg_label, msg);
    }
}

void srain_stack_sidebar_item_count_inc(SrainStackSidebarItem *item){
    int count;
    char buf[32];

    if ((count = atoi(gtk_label_get_text(item->count_label))) == 0){
        gtk_widget_set_name(GTK_WIDGET(item->count_label), "count_label");
    }
    snprintf(buf, 32, "%d", count + 1);

    gtk_label_set_text(item->count_label, buf);
}

void srain_stack_sidebar_item_count_clear(SrainStackSidebarItem *item){
    gtk_widget_set_name(GTK_WIDGET(item->count_label), "");
    gtk_label_set_text(item->count_label, "");
}
