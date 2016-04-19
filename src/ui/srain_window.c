/**
 * @file srain_window.c
 * @brief Srain's main windows
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */

#define __LOG_ON

#include <gtk/gtk.h>
#include <assert.h>
#include "ui_common.h"
#include "theme.h"
#include "srain_app.h"
#include "srain_window.h"
#include "srain_chan.h"
#include "srain_stack_sidebar.h"
#include "srain_about_box.h"
#include "tray_icon.h"
#include "meta.h"
#include "log.h"

struct _SrainWindow {
    GtkApplicationWindow parent;
    GtkButton *about_button;
    GtkButton *join_button;
    GtkSpinner *spinner;
    GtkBox *sidebar_box;
    SrainStackSidebar *sidebar;
    GtkStack *stack;
    GtkStatusIcon *tray_icon;
    GtkMenu *tray_menu;

    GtkPopover *about_popover;
    GtkPopover *join_popover;

    GtkBox *join_box;
    GtkEntry *join_entry;
};

struct _SrainWindowClass {
    GtkApplicationWindowClass parent_class;
};

G_DEFINE_TYPE(SrainWindow, srain_window, GTK_TYPE_APPLICATION_WINDOW);

static void join_entry_on_activate(GtkWidget *widget, gpointer user_data){
    GtkPopover *popover;
    GtkEntry *entry;

    entry = GTK_ENTRY(widget);
    popover = user_data;

    srain_app_join(gtk_entry_get_text(entry));
    gtk_widget_set_visible(GTK_WIDGET(popover), FALSE);
    gtk_entry_set_text(entry, "");
}

static void header_button_on_click(gpointer user_data){
    GtkPopover *popover;

    popover = user_data;
    gtk_widget_set_visible(GTK_WIDGET(popover), TRUE);
}

static gboolean CTRL_J_K_on_press(GtkAccelGroup *group, GObject *obj, guint keyval,
        GdkModifierType mod, gpointer user_data){
    SrainStackSidebar *sidebar;

    if (mod != GDK_CONTROL_MASK) return FALSE;

    sidebar = user_data;
    switch (keyval){
        case GDK_KEY_k:
            LOG_FR("<C-k>");
            srain_stack_sidebar_prev(sidebar);
            break;
        case GDK_KEY_j:
            LOG_FR("<C-j>");
            srain_stack_sidebar_next(sidebar);
            break;
        default:
            ERR_FR("unknown keyval %d", keyval);
            return FALSE;
    }

    return TRUE;
}

static void srain_window_init(SrainWindow *self){
    GClosure *closure_j;
    GClosure *closure_k;
    GtkAccelGroup *accel;
    SrainAboutBox *about_box;

    gtk_widget_init_template(GTK_WIDGET(self));

    /* about popover init */
    about_box = srain_about_box_new();
    self->about_popover = create_popover(GTK_WIDGET(self->about_button),
            GTK_WIDGET(about_box), GTK_POS_BOTTOM);
    gtk_container_set_border_width(GTK_CONTAINER(self->about_popover), 10);

    /* join popover init */
    self->join_popover = create_popover(GTK_WIDGET(self->join_button),
            GTK_WIDGET(self->join_box), GTK_POS_BOTTOM);
    gtk_container_set_border_width(GTK_CONTAINER(self->join_popover), 2);

    /* stack sidebar init */
    self->sidebar = srain_stack_sidebar_new();
    gtk_widget_show(GTK_WIDGET(self->sidebar));
    gtk_box_pack_start(self->sidebar_box, GTK_WIDGET(self->sidebar), TRUE, TRUE, 0);
    srain_stack_sidebar_set_stack(self->sidebar, self->stack);

    theme_apply(GTK_WIDGET(self));
    theme_apply(GTK_WIDGET(self->tray_menu));
    theme_apply(GTK_WIDGET(self->about_popover));

    tray_icon_set_callback(self->tray_icon, self, self->tray_menu);
    g_signal_connect_swapped(self->about_button, "clicked",
            G_CALLBACK(header_button_on_click), self->about_popover);
    g_signal_connect_swapped(self->join_button, "clicked",
            G_CALLBACK(header_button_on_click), self->join_popover);
    g_signal_connect(self->join_entry, "activate",
            G_CALLBACK(join_entry_on_activate), self->join_popover);

    /* shortcut <C-j> and <C-k> */
    accel = gtk_accel_group_new();

    closure_j = g_cclosure_new(G_CALLBACK(CTRL_J_K_on_press), self->sidebar, NULL);
    closure_k = g_cclosure_new(G_CALLBACK(CTRL_J_K_on_press), self->sidebar, NULL);

    gtk_accel_group_connect(accel, GDK_KEY_j, GDK_CONTROL_MASK,
            GTK_ACCEL_VISIBLE, closure_j);
    gtk_accel_group_connect(accel, GDK_KEY_k, GDK_CONTROL_MASK,
            GTK_ACCEL_VISIBLE, closure_k);

    gtk_window_add_accel_group(GTK_WINDOW(self), accel);

    g_closure_unref(closure_j);
    g_closure_unref(closure_k);
}

static void srain_window_class_init(SrainWindowClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/window.glade");
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, about_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, join_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, spinner);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, stack);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, tray_icon);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, tray_menu);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, sidebar_box);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, join_box);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, join_entry);
}

SrainWindow* srain_window_new(SrainApp *app){
    return g_object_new(SRAIN_TYPE_WINDOW, "application", app, NULL);
}

SrainChan* srain_window_add_chan(SrainWindow *win, const char *name){
    SrainChan *chan;

    if (srain_window_get_chan_by_name(win, name)){
        ERR_FR("chan'%s' alread exist", name);
        return NULL;
    }

    chan = srain_chan_new("irc.freenode.net", name);

    gtk_stack_add_named(win->stack, GTK_WIDGET(chan), name);
    // gtk_container_child_set(GTK_CONTAINER(win->stack), GTK_WIDGET(chan), "title", name, NULL);
    theme_apply(GTK_WIDGET(win));

    gtk_stack_set_visible_child (win->stack, GTK_WIDGET(chan));
    return chan;
}

void srain_window_rm_chan(SrainWindow *win, SrainChan *chan){
    gtk_container_remove(GTK_CONTAINER(win->stack), GTK_WIDGET(chan));
}

SrainChan *srain_window_get_cur_chan(SrainWindow *win){
    SrainChan *chan = NULL;

    chan = SRAIN_CHAN(gtk_stack_get_visible_child(win->stack));

    // TODO:
    //  if (chan == NULL) ERR_FR("no visible chan");

    return chan;
}

SrainChan *srain_window_get_chan_by_name(SrainWindow *win, const char *name){
    SrainChan *chan = NULL;

    if (name)
        chan = SRAIN_CHAN(gtk_stack_get_child_by_name(win->stack, name));
    else
        chan = srain_window_get_cur_chan(win);

    return chan;
}

void srain_window_spinner_toggle(SrainWindow *win, gboolean is_busy){
   is_busy  
        ? gtk_spinner_start(win->spinner)
        : gtk_spinner_stop(win->spinner);
}

void srain_window_stack_sidebar_update(SrainWindow *win, SrainChan *chan,
        const char *nick, const char *msg){
    if (SRAIN_CHAN(gtk_stack_get_visible_child(win->stack)) != chan){
        srain_stack_sidebar_update(win->sidebar, chan, nick, msg, 0);
    } else {
        srain_stack_sidebar_update(win->sidebar, chan, nick, msg, 1);
    }

}
