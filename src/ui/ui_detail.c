#include <assert.h>
#include "ui_common.h"

void detail_dialog_init(const char *name, const char *content){
        GtkBuilder *builder;
        GtkWidget *detail_dialog;
        GtkWidget *detail_name_label;
        GtkWidget *detail_content_label;
        GtkWidget *detail_chat_button;
        GtkWidget *detail_close_button;

        builder = gtk_builder_new_from_file( "../data/ui/detail_dialog.glade");
        UI_BUILDER_GET_WIDGET(builder, detail_dialog);
        UI_BUILDER_GET_WIDGET(builder, detail_chat_button);
        UI_BUILDER_GET_WIDGET(builder, detail_close_button);
        UI_BUILDER_GET_WIDGET(builder, detail_name_label);
        UI_BUILDER_GET_WIDGET(builder, detail_content_label);

        gtk_label_set_text(GTK_LABEL(detail_name_label), name);
        gtk_label_set_text(GTK_LABEL(detail_content_label), content);

        g_signal_connect_swapped(detail_dialog, "close", G_CALLBACK(gtk_widget_destroy), detail_dialog); // why i should close for 2 times?
        g_signal_connect_swapped(detail_close_button, "clicked", G_CALLBACK(gtk_widget_destroy), detail_dialog);

        gtk_dialog_run(GTK_DIALOG(detail_dialog));
}
