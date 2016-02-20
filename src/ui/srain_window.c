#include <gtk/gtk.h>
#include <assert.h>
#include "ui_common.h"
#include "srain.h"
#include "srain_app.h"
#include "srain_window.h"

struct _SrainWindow {
    GtkApplicationWindow parent;
    GtkStack *stack;
};

struct _SrainWindowClass {
    GtkApplicationWindowClass parent_class;
};

G_DEFINE_TYPE(SrainWindow, srain_window, GTK_TYPE_APPLICATION_WINDOW);

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

static void srain_window_init(SrainWindow *win){
    GtkBuilder *builder;

    gtk_widget_init_template(GTK_WIDGET(win));
    g_action_map_add_action_entries(G_ACTION_MAP(win),
            win_entries, G_N_ELEMENTS(win_entries), win);
}

static void srain_window_class_init(SrainWindowClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/window.glade");
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainWindow, stack);

    // gtk_widget_class_bind_template_child_private(GTK_WIDGET_CLASS(class), SrainAppWindow, stack);
    // gtk_widget_class_bind_template_callback(GTK_WIDGET_CLASS(class), search_text_changed);
}

SrainWindow* srain_window_new(SrainApp *app){
    return g_object_new(SRAIN_TYPE_WINDOW, "application", app, NULL);
}
