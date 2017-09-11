#include <gtk/gtk.h>

void connect_dialog_run(GtkWindow *parent) {
    GtkDialog *dialog;
    GtkBuilder *builder;

    builder = gtk_builder_new_from_resource("/org/gtk/srain/connect_dialog.glade");
    dialog = GTK_DIALOG(gtk_builder_get_object(builder, "connect_dialog"));

    gtk_window_set_transient_for(GTK_WINDOW(dialog), parent);
    gtk_dialog_run(dialog);
}
