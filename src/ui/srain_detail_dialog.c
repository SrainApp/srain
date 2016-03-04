/**
 * @file srain_detail_dialog.c
 * @brief dialog used to show user/channel's detail
 * @author LastAvengers <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-03-01
 */

#include <gtk/gtk.h>
#include <assert.h>
#include "ui_common.h"
#include "theme.h"
#include "srain_window.h"
#include "srain_detail_dialog.h"

struct _SrainDetailDialog {
    GtkDialog parent;
    GtkLabel *name_label;
    GtkLabel *content_label;
    GtkButton *chat_button;
    GtkButton *close_button;
};

struct _SrainDetailDialogClass {
    GtkDialogClass parent_class;
};

G_DEFINE_TYPE(SrainDetailDialog, srain_detail_dialog, GTK_TYPE_DIALOG);

static void srain_detail_dialog_init(SrainDetailDialog *self){
    gtk_widget_init_template(GTK_WIDGET(self));
}

static void srain_detail_dialog_class_init(SrainDetailDialogClass *class){
    gtk_widget_class_set_template_from_resource(GTK_WIDGET_CLASS(class),
            "/org/gtk/srain/detail_dialog.glade");
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainDetailDialog, name_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainDetailDialog, content_label);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainDetailDialog, chat_button);
    gtk_widget_class_bind_template_child(GTK_WIDGET_CLASS(class), SrainDetailDialog, close_button);
}

SrainDetailDialog* srain_detail_dialog_new(SrainWindow *win, const char *name, const char *content){
    SrainDetailDialog *dlg;

    dlg = g_object_new(SRAIN_TYPE_DETAIL_DIALOG, "transient-for", win, NULL);

    gtk_label_set_text(dlg->name_label, name);
    gtk_label_set_text(dlg->content_label, content);

    g_signal_connect_swapped(dlg, "close", G_CALLBACK(gtk_widget_destroy), dlg);
    g_signal_connect_swapped(dlg->close_button, "clicked", G_CALLBACK(gtk_widget_destroy), dlg);

    theme_apply(GTK_WIDGET(dlg));

    return dlg;
}
