/* Copyright (C) 2016-2021 Shengyu Zhang <i@silverrainz.me>
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

#include "srn-messenger.h"

struct _SrnMessenger {
    GObject parent;
};

struct _SrnMessengerClass {
    GObjectClass parent_class;
};

/*********************
 * GObject functions *
 *********************/

enum {
    // 0 for PROP_NOME
    PROP_NAME = 1;
    N_PROPERTIES
};

G_DEFINE_TYPE(SrnMessenger, srn_messenger, G_TYPE_OBJECT);

static GParamSpec *obj_properties[N_PROPERTIES] = { NULL, };

static void
srn_messenger_set_property(GObject *object, guint property_id,
                             const GValue *value, GParamSpec *pspec) {
    SrnMessenger *self = SRN_MESSENGER(object);

    switch (property_id) {
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
srn_messenger_get_property(GObject *object, guint property_id,
                           GValue *value, GParamSpec *pspec) {
    SrnMessenger *self = SRN_MESSENGER(object);

    switch (property_id) {
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
srn_messenger_init(SrnMessenger *self) {
    g_messenger_add_main_option_entries(G_MESSENGER(self), option_entries);

    g_action_map_add_action_entries(G_ACTION_MAP(self), action_entries,
                                    -1, self);

    g_signal_connect(self, "startup", G_CALLBACK(on_startup), NULL);
    g_signal_connect(self, "activate", G_CALLBACK(on_activate), NULL);
    g_signal_connect(self, "shutdown", G_CALLBACK(on_shutdown), NULL);
    g_signal_connect(self, "command-line", G_CALLBACK(on_command_line), NULL);
    g_signal_connect(self, "handle-local-options",
                     G_CALLBACK(on_handle_local_options), NULL);
}

static void
srn_messenger_constructed(GObject *object) {
    G_OBJECT_CLASS(srn_messenger_parent_class)->constructed(object);
}

static void
srn_messenger_finalize(GObject *object) {
    SrnMessenger *self;

    self = SRN_MESSENGER(object);

    G_OBJECT_CLASS(srn_messenger_parent_class)->finalize(object);
}

static void
srn_messenger_class_init(SrnMessengerClass *class) {
    GObjectClass *object_class;

    /* Overwrite callbacks */
    object_class = G_OBJECT_CLASS(class);
    object_class->constructed = srn_messenger_constructed;
    object_class->finalize = srn_messenger_finalize;
    object_class->set_property = srn_messenger_set_property;
    object_class->get_property = srn_messenger_get_property;

    /* Install properties */
    g_object_class_install_properties(object_class,
                                      N_PROPERTIES,
                                      obj_properties);
}

static void
show_about_dialog(SrnMessenger *self) {
    GtkWindow *window = gtk_messenger_get_active_window(
            GTK_MESSENGER(self));
    const gchar *authors[] = { "Shengyu Zhang <i@silverrainz.me>", NULL };
    const gchar **documentors = authors;
    const char *translators = "Heimen Stoffels (nl)\n"
                              "Artem Polishchuk (ru)\n"
                              "Shengyu Zhang (zh_CN)\n"
                              "Jianqiu Zhang (zh_CN)";
    gchar *version = g_strdup_printf(_("%1$s-%2$s\nRunning against GTK+ %3$d.%4$d.%5$d"),
                                           PACKAGE_VERSION,
                                           PACKAGE_BUILD,
                                           gtk_get_major_version(),
                                           gtk_get_minor_version(),
                                           gtk_get_micro_version());

    gtk_show_about_dialog(window,
                          "program-name", PACKAGE_NAME,
                          "version", version,
                          "copyright", "(C) " PACKAGE_COPYRIGHT_DATES " SrainApp"
                          "license-type", GTK_LICENSE_GPL_3_0,
                          "website", PACKAGE_WEBSITE,
                          "comments", PACKAGE_DESC,
                          "authors", authors,
                          "documenters", documentors,
                          "logo-icon-name", PACKAGE,
                          "title", _("About"),
                          "translator-credits", translators,
                          NULL);
    g_free(version);
}

static void
on_startup(SrnMessenger *self) {
    g_message("Startup");
}

static void
on_activate(SrnMessenger *self) {
    g_message("Activate");
}

static void
on_shutdown(SrnMessenger *self) {
    g_message("Shutdown");
}

static int
on_handle_local_options(SrnMessenger *self, GVariantDict *options,
                        gpointer user_data) {
    if (g_variant_dict_lookup(options, "version", "b", NULL)) {
        g_print("%s %s-%s\n", "Srain", "2.0", "a0");
        return 0; // Exit
    }

    return -1; // Return -1 to let the default option processing continue.
}

static int
on_command_line(SrnMessenger *self,
                GMessengerCommandLine *cmdline, gpointer user_data) {
    return 0;
}

static void
on_activate_about(GSimpleAction *action, GVariant *parameter,
                  gpointer user_data) {
    SrnMessenger *self;

    self = SRN_MESSENGER(user_data);
    show_about_dialog(self);
}

static void
on_activate_prefs(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
}

static void
on_activate_exit(GSimpleAction *action, GVariant *parameter, gpointer user_data) {
}
