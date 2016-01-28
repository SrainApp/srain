#include <stdio.h>
#include <gtk/gtk.h>
#include "i18n.h"

static void apply_css(GtkWidget *widget, GtkStyleProvider *provider){
    gtk_style_context_add_provider(gtk_widget_get_style_context(widget), provider, G_MAXUINT);

    if (GTK_IS_CONTAINER(widget))
        gtk_container_forall(GTK_CONTAINER(widget), (GtkCallback)apply_css, provider);
}

int main(int argc, char **argv){
    GtkBuilder *builder;
    GtkWidget *window;
    GtkStyleProvider *provider;

    setlocale(LC_ALL, "");
    textdomain("srain");
    bindtextdomain("srain", "locale");
    printf(_("Hello srain!\n"));

    gtk_init(&argc, &argv);

    /* load ui file */
    builder = gtk_builder_new();
    gtk_builder_add_from_file(builder, "../ui/main.glade", NULL);
    window = GTK_WIDGET(gtk_builder_get_object(builder, "SrainWindow"));
    // gtk_builder_connect_signals(builder, NULL);
    g_signal_connect (window, "destroy", G_CALLBACK (gtk_main_quit), NULL);
    g_object_unref(G_OBJECT( builder ) );

    /* load style */
    provider = GTK_STYLE_PROVIDER(gtk_css_provider_new());
    gtk_css_provider_load_from_path(GTK_CSS_PROVIDER(provider), "../ui/main.css", NULL);
    apply_css (window, provider);

    gtk_widget_show(window);

    gtk_main();

    return 0;
}
