/**
 * @file srain_stack_sidebar_item.c
 * @brief item class of SrainStackSidebar
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-07
 */

#define __LOG_ON

#include <gtk/gtk.h>
#include <string.h>
#include "srain_stack_sidebar_item.h"
#include "irc_magic.h"
#include "log.h"

struct _SrainStackSidebarItem {
    GtkBox parent;
    GtkImage *image;
    GtkLabel *name_label;
    GtkLabel *lastmsg_label;
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
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainStackSidebarItem, name_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainStackSidebarItem, lastmsg_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainStackSidebarItem, count_label);
}

SrainStackSidebarItem *srain_stack_sidebar_item_new(const char *name){
    SrainStackSidebarItem *item;

    g_return_val_if_fail(name, NULL);

    item = g_object_new(SRAIN_TYPE_STACK_SIDEBAR_ITEM, NULL);

    gtk_label_set_text(item->name_label, name);

    // is a channel
    // TODO server
    if (name[0] == CHAN_PREFIX1 || name[0] == CHAN_PREFIX2){
        gtk_image_set_from_file(item->image, "chan_icon.png");
    } else {
        gtk_image_set_from_file(item->image, "user_icon.png");
    }

    return item;
}

void srain_stack_sidebar_item_lastmsg_update(SrainStackSidebarItem *item, const char *nick, const char *msg){
    char buf[64];
    snprintf(buf, 64, "%s: %s", nick, msg);
    gtk_label_set_text(item->lastmsg_label, buf);
}

void srain_stack_sidebar_item_count_inc(SrainStackSidebarItem *item, int inc){
    int count;
    char buf[32];
    if (inc == 0) return;

    if ((count = atoi(gtk_label_get_text(item->count_label))) == 0){
        gtk_widget_set_name(GTK_WIDGET(item->count_label), "count_label");
    }
    snprintf(buf, 32, "%d", count + inc);

    gtk_label_set_text(item->count_label, buf);
}

void srain_stack_sidebar_item_count_clear(SrainStackSidebarItem *item){
    gtk_widget_set_name(GTK_WIDGET(item->count_label), "");
    gtk_label_set_text(item->count_label, "");
}
