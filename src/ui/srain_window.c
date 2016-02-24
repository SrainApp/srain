#include <gtk/gtk.h>
#include <assert.h>
#include "ui_common.h"
#include "srain.h"
#include "srain_app.h"
#include "srain_window.h"
#include "srain_chan.h"
#include "tray_icon.h"
#include "log.h"

struct _SrainWindow {
    GtkApplicationWindow parent;
    GtkStack *stack;
    GtkStackSidebar *sidebar;
    GtkMenu *sidebar_menu;
    GtkStatusIcon *tray_icon;
    GtkMenu *tray_menu;
};

struct _SrainWindowClass {
    GtkApplicationWindowClass parent_class;
};

G_DEFINE_TYPE(SrainWindow, srain_window, GTK_TYPE_APPLICATION_WINDOW);

static gint sidebar_menu_popup(GtkWidget *widget, GdkEventButton *event, gpointer *user_data){
  GtkMenu *menu;
  GdkEventButton *event_button;

  menu = GTK_MENU(user_data);

  if (event->button == 3){
      gtk_menu_popup(GTK_MENU(menu), NULL, NULL, NULL, NULL, event->button, event->time);
      return TRUE;
  }
  return FALSE;
}

static void activate_about(GSimpleAction *action, GVariant *parameter, gpointer user_data){
    GtkWidget *window = user_data;
    const gchar *authors[] = { "LastAvengers <lastavengers@outlook.com>", NULL };
    const gchar *documentors[] = { "LastAvengers <lastavengers@outlook.com>", NULL };
    const gchar *version = g_strdup_printf("%s,\nRunning against GTK+ %d.%d.%d", 
            "1.0 alpha",
            gtk_get_major_version(),
            gtk_get_minor_version(),
            gtk_get_micro_version());

    gtk_show_about_dialog(GTK_WINDOW(window),
            "program-name", "Srain",
            "version", version,
            "copyright", "(C) 2016 LastAvengers",
            "license-type", GTK_LICENSE_GPL_3_0,
            "website", "https://github.com/lastavenger/srain",
            "comments", "A modren IRC client.",
            "authors", authors,
            "documenters", documentors,
            "logo-icon-name", "gtk3-demo",
            "title", "About Srain",
            NULL);
}

static GActionEntry win_entries[] = {
    { "about", activate_about, NULL, NULL, NULL },
};

static void srain_window_init(SrainWindow *self){
    gtk_widget_init_template(GTK_WIDGET(self));

    g_action_map_add_action_entries(G_ACTION_MAP(self),
            win_entries, G_N_ELEMENTS(win_entries), self);

    gtk_window_set_title(GTK_WINDOW(self), "Srain");

    tray_icon_set_callback(self->tray_icon, self, self->tray_menu);
    g_signal_connect(self->sidebar, "button_press_event", G_CALLBACK(sidebar_menu_popup), self->sidebar_menu);
}

static void srain_window_class_init(SrainWindowClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/window.glade");
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, stack);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, tray_icon);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, tray_menu);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, sidebar);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, sidebar_menu);
}

SrainWindow* srain_window_new(SrainApp *app){
    return g_object_new(SRAIN_TYPE_WINDOW, "application", app, NULL);
}

SrainChan* srain_window_add_chan(SrainWindow *win, const char *name){
    SrainChan *chan;

    chan = srain_chan_new(name);

    gtk_stack_add_named(win->stack, GTK_WIDGET(chan), name);
    gtk_container_child_set(GTK_CONTAINER(win->stack), GTK_WIDGET(chan), "title", name, NULL);

    return chan;
}

void srain_window_rm_chan(SrainWindow *win, const char *name){
    SrainChan *chan;

    chan = SRAIN_CHAN(gtk_stack_get_child_by_name(win->stack, name));
    gtk_container_remove(GTK_CONTAINER(win->stack), GTK_WIDGET(chan));
}

SrainChan *srain_window_cur_chan(SrainWindow *win){
    return SRAIN_CHAN(gtk_stack_get_visible_child(win->stack));
}
