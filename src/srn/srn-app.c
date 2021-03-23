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

#include <glib/gi18n.h>
#include <gtk/gtk.h>
#include <girepository.h>

#include "sif/sif.h"

// For package meta infos
#include "srn-meta.h"
#include "srn-app.h"

struct _SrnApplication {
    GtkApplication parent;
};

static void
show_about_dialog(SrnApplication *self);

static void
on_startup(SrnApplication *self);
static void
on_activate(SrnApplication *self);
static void
on_shutdown(SrnApplication *self);
static int
on_handle_local_options(SrnApplication *self, GVariantDict *options,
                        gpointer user_data);
static int
on_command_line(SrnApplication *self,
                GApplicationCommandLine *cmdline, gpointer user_data);
static void
on_activate_about(GSimpleAction *action, GVariant *parameter,
                  gpointer user_data);
static void
on_activate_prefs(GSimpleAction *action, GVariant *parameter,
                  gpointer user_data);
static void
on_activate_exit(GSimpleAction *action, GVariant *parameter,
                 gpointer user_data);

/*****************************************************************************
 * GObject functions
 *****************************************************************************/

enum {
    PROP_0,
    N_PROPERTIES
};

G_DEFINE_TYPE(SrnApplication, srn_application, GTK_TYPE_APPLICATION);

static GParamSpec *obj_properties[N_PROPERTIES] = { };

static const GOptionEntry option_entries[] = {
    {
        .long_name = "version",
        .short_name = 'v',
        .flags = 0,
        .arg = G_OPTION_ARG_NONE,
        .arg_data = NULL,
        .description = N_("Show version information"),
        .arg_description = NULL,
    },
    {
        .long_name = "introspect-dump",
        .flags = 0,
        .arg = G_OPTION_ARG_STRING,
        .arg_data = NULL,
        .description = N_("Dump introspection data"),
        .arg_description = NULL,
    },
    { NULL }
};

static const GActionEntry action_entries[] = {
    {
        .name = "about",
        .activate = on_activate_about,
    },
    {
        .name = "preferences",
        .activate = on_activate_prefs,
    },
    {
        .name = "exit",
        .activate = on_activate_exit,
    },
    { NULL }
};

static void
srn_application_set_property(GObject *object, guint property_id,
                             const GValue *value, GParamSpec *pspec) {
    switch (property_id) {
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
srn_application_get_property(GObject *object, guint property_id,
                             GValue *value, GParamSpec *pspec) {
    switch (property_id) {
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
srn_application_init(SrnApplication *self) {
    g_application_add_main_option_entries(G_APPLICATION(self), option_entries);

    g_action_map_add_action_entries(G_ACTION_MAP(self), action_entries,
                                    -1, self);

    g_signal_connect(self, "startup", G_CALLBACK(on_startup), NULL);
    g_signal_connect(self, "activate", G_CALLBACK(on_activate), NULL);
    g_signal_connect(self, "shutdown", G_CALLBACK(on_shutdown), NULL);
    g_signal_connect(self, "command-line", G_CALLBACK(on_command_line), NULL);
    g_signal_connect(self, "handle-local-options",
                     G_CALLBACK(on_handle_local_options), NULL);

    /* Init GObject Introspection repository */
    g_irepository_prepend_search_path(PACKAGE_LIB_DIR "/girepository-1.0");
    g_irepository_prepend_library_path(PACKAGE_LIB_DIR);
}

static void
srn_application_constructed(GObject *object) {
    G_OBJECT_CLASS(srn_application_parent_class)->constructed(object);
}

static void
srn_application_finalize(GObject *object) {
    G_OBJECT_CLASS(srn_application_parent_class)->finalize(object);
}

static void
srn_application_class_init(SrnApplicationClass *class) {
    GObjectClass *object_class;

    /* Overwrite callbacks */
    object_class = G_OBJECT_CLASS(class);
    object_class->constructed = srn_application_constructed;
    object_class->finalize = srn_application_finalize;
    object_class->set_property = srn_application_set_property;
    object_class->get_property = srn_application_get_property;

    /* Install properties */
    g_object_class_install_properties(object_class,
                                      N_PROPERTIES,
                                      obj_properties);
}

static void
show_about_dialog(SrnApplication *self) {
    GtkWindow *window = gtk_application_get_active_window(
                            GTK_APPLICATION(self));
    const gchar *authors[] = { "Shengyu Zhang <i@silverrainz.me>", NULL };
    const gchar **documentors = authors;
    const char *translators = "Heimen Stoffels (nl)\n"
                              "Artem Polishchuk (ru)\n"
                              "Shengyu Zhang (zh_CN)\n"
                              "Jianqiu Zhang (zh_CN)";
    gchar *version = g_strdup_printf(
                         _("%1$s-%2$s\nRunning against GTK+ %3$d.%4$d.%5$d"),
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
on_startup(SrnApplication *self) {
    g_message("Startup");
    GIRepository *repository;
    GError *error = NULL;
    GIBaseInfo *base_info;
    GIArgument retval;

    repository = g_irepository_get_default();

    g_irepository_require(repository, "Sirc", "1.0", 0, &error);
    if (error) {
        g_error("ERROR: %s\n", error->message);
        return;
    }

    base_info = g_irepository_find_by_name(repository, "Sirc", "Messenger");
    if (!base_info) {
        g_error("ERROR: %s\n", "Could not find Sirc.Messenger");
        return;
    }
    g_warn_if_fail(GI_IS_OBJECT_INFO(base_info));
    GIObjectInfo *base_info1 = g_object_info_find_method(
                                   (GIObjectInfo *)base_info, "new");
    if (!base_info1) {
        g_error("ERROR: %s\n", "Could not find Sirc.Messenger.new() ");
        return;
    }

    if (!g_function_info_invoke((GIFunctionInfo *) base_info1,
                                NULL,
                                0,
                                NULL,
                                0,
                                &retval,
                                &error)) {
        g_error("ERROR: %s\n", error->message);
        return;
    }
    g_message("Sirc.Messenger.new() done");

    g_message("Is SifMessenger? %d", SIF_IS_MESSENGER((gpointer)retval.v_pointer));
    return;
}

static void
on_activate(SrnApplication *self) {
    return;
}

static void
on_shutdown(SrnApplication *self) {
    g_message("Shutdown");
}

static int
on_handle_local_options(SrnApplication *self, GVariantDict *options,
                        gpointer user_data) {
    if (g_variant_dict_lookup(options, "version", "b", NULL)) {
        g_print("%s %s-%s\n", PACKAGE_NAME, PACKAGE_VERSION, PACKAGE_BUILD);
        return 0; // Success
    }

    GVariant *dump = g_variant_dict_lookup_value(options, "introspect-dump",
                     G_VARIANT_TYPE_STRING);
    if (dump) {
        GError *err = NULL;
        if (!g_irepository_dump(g_variant_get_string(dump, NULL), &err)) {
            g_print("g_irepository_dump() failed: %s\n", err->message);
            g_error_free(err);
            g_variant_unref(dump);
            return 1; // Exit
        }
        g_variant_unref(dump);
        return 0; // Success
    }

    return -1; // Let the default option processing continue.
}

static int
on_command_line(SrnApplication *self,
                GApplicationCommandLine *cmdline, gpointer user_data) {
    return 0;
}

static void
on_activate_about(GSimpleAction *action, GVariant *parameter,
                  gpointer user_data) {
    show_about_dialog(SRN_APPLICATION(user_data));
}

static void
on_activate_prefs(GSimpleAction *action, GVariant *parameter,
                  gpointer user_data) {
}

static void
on_activate_exit(GSimpleAction *action, GVariant *parameter,
                 gpointer user_data) {
}

/**
 * srn_application_ping:
 * @self: a #SrnApplication.
 *
 * Print a Ping! message.
 */
void
srn_application_ping(SrnApplication *self) {
    g_return_if_fail(SRN_IS_APPLICATION(self));

    g_message("Ping!");
}
