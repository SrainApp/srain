/* Copyright (C) 2016-2018 Shengyu Zhang <i@silverrainz.me>
 *
 * This file is part of Srain.
 *
 * Srain is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file sui_prefs_window.c
 * @brief Sui preferences window class
 * @author Shengyu Zhang <i@silverrainz.me>
 * @version
 * @date 2018-12-12
 *
 * TODO(SilverRainZ): The current implementation is an ad-hoc method
 * to let user change config without restart,
 * we should implement the real configuration panel in future.
 */

#include <gtk/gtk.h>

#include "core/core.h"
#include "sui_common.h"
#include "sui_prefs_dialog.h"
#include "i18n.h"
#include "path.h"

struct _SuiPrefsDialog {
    GtkDialog parent;

    /* Dialog buttons */
    GtkButton *ok_button;
    GtkButton *cancel_button;

    /* Reload config */
    GtkLabel *reload_config_label;
    GtkButton *reload_config_button;
};

struct _SuiPrefsDialogClass {
    GtkDialogClass parent_class;
};

static void ok_button_on_clicked(GtkWidget *widget, gpointer user_data);
static void cancel_button_on_clicked(GtkWidget *widget, gpointer user_data);
static void reload_config_button_on_clicked(GtkWidget *widget,
        gpointer user_data);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

G_DEFINE_TYPE(SuiPrefsDialog, sui_prefs_dialog, GTK_TYPE_DIALOG);

static void sui_prefs_dialog_init(SuiPrefsDialog *self){
    gtk_widget_init_template(GTK_WIDGET(self));

    g_signal_connect(self->ok_button, "clicked",
            G_CALLBACK(ok_button_on_clicked), self);
    g_signal_connect(self->cancel_button, "clicked",
            G_CALLBACK(cancel_button_on_clicked), self);
    g_signal_connect(self->reload_config_button, "clicked",
            G_CALLBACK(reload_config_button_on_clicked), self);

    // Set hint messsage for configuration reload
    char *user_cfg_path;
    char *markup;

    user_cfg_path = srn_get_user_config_file();
    markup = g_markup_printf_escaped(
            "  Srain has no available configuration panel for now, \
The only way of changing the configuration is editing the \
<a href=\"file://%s\">configuration file</a>.\
\n\
  If you want your change to take effect without restarting Srain, \
click the <b>Reload</b> button please.\
\n\n\
<i>NOTE: There are some configuration items can not take effect \
without restarting Srain (such as csd)</i>.",
            user_cfg_path);

    gtk_label_set_markup(self->reload_config_label, markup);

    g_free(user_cfg_path);
    g_free(markup);
}

static void sui_prefs_dialog_class_init(SuiPrefsDialogClass *class){
    GtkWidgetClass *widget_class;

    widget_class = GTK_WIDGET_CLASS(class);
    gtk_widget_class_set_template_from_resource(widget_class,
            "/im/srain/Srain/prefs_dialog.glade");

    gtk_widget_class_bind_template_child(widget_class, SuiPrefsDialog, ok_button);
    gtk_widget_class_bind_template_child(widget_class, SuiPrefsDialog, cancel_button);

    gtk_widget_class_bind_template_child(widget_class, SuiPrefsDialog, reload_config_label);
    gtk_widget_class_bind_template_child(widget_class, SuiPrefsDialog, reload_config_button);
}

/*****************************************************************************
 * Exported functions
 *****************************************************************************/

SuiPrefsDialog* sui_prefs_dialog_new(SuiApplication *app, SuiWindow *win){
    SuiPrefsDialog *self;

    self = g_object_new(SUI_TYPE_PREFS_DIALOG,
            "application", app,
            "transient-for", win,
            NULL);

    return self;
}


/*****************************************************************************
 * Static functions
 *****************************************************************************/

static void ok_button_on_clicked(GtkWidget *widget, gpointer user_data){
    SuiPrefsDialog *self;

    self = SUI_PREFS_DIALOG(user_data);

    gtk_dialog_response(GTK_DIALOG(self), GTK_RESPONSE_OK);
}

static void cancel_button_on_clicked(GtkWidget *widget, gpointer user_data){
    SuiPrefsDialog *self;

    self = SUI_PREFS_DIALOG(user_data);

    gtk_dialog_response(GTK_DIALOG(self), GTK_RESPONSE_CANCEL);
}

static void reload_config_button_on_clicked(GtkWidget *widget,
        gpointer user_data){
    SrnRet ret;
    SrnApplication *srn_app;

    srn_app = sui_application_get_ctx(sui_application_get_instance());

    ret = srn_application_reload_config(srn_app);
    if (!RET_IS_OK(ret)) {
        char *errmsg;

        errmsg = g_strdup_printf(_("Failed to reload configuration: %s"),
                RET_MSG(ret));
        sui_message_box(_("Error"), errmsg);
        g_free(errmsg);
    } else {
        sui_message_box(_("OK"), _("Configuration is reloaded"));
    }
}
