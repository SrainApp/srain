
/**
 * @file srain_about_box.c
 * @brief A GtkBox subclass used to display about information
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-26
 */

#include <gtk/gtk.h>
#include "srain_about_box.h"
#include "meta.h"
#include "theme.h"

struct _SrainAboutBox {
    GtkBox parent;
    GtkLabel *title_label;
    GtkLabel *content_label;
};

struct _SrainAboutBoxClass {
    GtkBoxClass parent_class;
};

G_DEFINE_TYPE(SrainAboutBox, srain_about_box, GTK_TYPE_BOX);

static void close_button_on_click(GtkWidget *widget, gpointer user_data){
    SrainAboutBox *about_box;
    GtkContainer *container;

    about_box = user_data;

    container = GTK_CONTAINER(gtk_widget_get_parent(GTK_WIDGET(about_box)));
    gtk_container_remove(container, GTK_WIDGET(about_box));
}

static void srain_about_box_init(SrainAboutBox *self){
    GString *tmp;

    gtk_widget_init_template(GTK_WIDGET(self));

    tmp = g_string_new(NULL);

    g_string_printf(tmp, "<span size=\"xx-large\">%s</span> %s", META_NAME, META_VERSION);
    gtk_label_set_markup(self->title_label, tmp->str);

    g_string_free(tmp, TRUE);
    tmp = g_string_new(NULL);

    g_string_printf(tmp,
            "\n%s\n\n%s &lt;\
<span foreground=\"blue\"><a href=\"mailto://%s\">%s</a></span>&gt;\n\
<span foreground=\"blue\"><a href=\"%s\">%s</a></span>\n",
            META_DESC, META_AUTHOR_NAME,
            META_AUTHOR_MAIL, META_AUTHOR_MAIL,
            META_WEBSITE, META_WEBSITE);
    gtk_label_set_markup(self->content_label, tmp->str);

    g_string_free(tmp, TRUE);
}

static void srain_about_box_class_init(SrainAboutBoxClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/about_box.glade");
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainAboutBox, title_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainAboutBox, content_label);
}

SrainAboutBox* srain_about_box_new(void){
    return g_object_new(SRAIN_TYPE_ABOUT_BOX, NULL);
}
