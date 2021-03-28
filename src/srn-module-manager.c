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

#include <glib.h>
#include <gtk/gtk.h>

#include "srn-meta.h"
#include "srn-messenger.h"

#include "srn-module-manager.h"

struct _SrnModuleManager {
    GtkApplication parent;

    GList *modules;
    GList *bindings;
    GList *messengers;
};

/*********************
 * GObject functions *
 *********************/

enum {
    PROP_0,
    PROP_MODULES,
    PROP_MESSENGERS,
    N_PROPERTIES
};

G_DEFINE_TYPE(SrnModuleManager, srn_module_manager, G_TYPE_OBJECT);

static GParamSpec *obj_properties[N_PROPERTIES] = { };

static void
srn_module_manager_set_property(GObject *object, guint property_id,
                                const GValue *value, GParamSpec *pspec) {
    switch (property_id) {
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
srn_module_manager_get_property(GObject *object, guint property_id,
                                GValue *value, GParamSpec *pspec) {
    SrnModuleManager *self = SRN_MODULE_MANAGER(object);

    switch (property_id) {
    case PROP_MODULES:
        g_value_set_pointer(value, srn_module_manager_get_modules(self));
    case PROP_MESSENGERS:
        g_value_set_pointer(value, srn_module_manager_get_messengers(self));
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

/*
  Copyied from glib/gio/giomodule.c::is_valid_module_name
 */
static gboolean
is_valid_module_name(const gchar *basename) {
#if !defined(G_OS_WIN32) && !defined(G_WITH_CYGWIN)
    if (!g_str_has_prefix(basename, "lib") ||
        !g_str_has_suffix(basename, ".so"))
        return FALSE;
#else
    if (!g_str_has_suffix(basename, ".dll"))
        return FALSE;
#endif

    return TRUE;
}

/** pre_load_modules:
 *
 * The implementation of GIOModule loads GModule with
 * G_MODULE_BIND_LOCAL flags, which causes other share libraries
 * (such as pygobject's lib) can not use symbols (such as symbols
 * in libpython.so) introduced by binding module.
 *
 * So we open binding modules with correct flags before loading
 * them as GIOModule. Don't care about leaking because we unref
 * modules at post_load_modules.
 *
 * Returns: (element-type GModule) (transfer full):
 *          Free it with g_list_free_full(list, g_object_unref).
 */
static GList *
pre_load_modules(void) {
    const gchar *name;
    GDir *dir;
    GList *bindings;

    g_return_val_if_fail(!g_module_supported(), NULL);

    dir = g_dir_open(PACKAGE_SYS_BIND_DIR, 0, NULL);
    if (!dir) return NULL;

    bindings = NULL;
    while ((name = g_dir_read_name(dir))) {
        if (is_valid_module_name(name)) {
            gchar *path;
            GModule *binding;

            path = g_build_filename(PACKAGE_SYS_BIND_DIR, name, NULL);
            /* NOTE:
             *
             * The implementation of GIOModule loads GModule with
             * G_MODULE_BIND_LOCAL flags, which causes other share libraries
             * (such as pygobject's lib) can not use symbols (such as symbols
             * in libpython.so) introduced by binding module.
             *
             * So we open binding modules with correct flags before loading
             * them as GIOModule. Don't care about leaking because we no need
             * to unload any modules.
             * */
            binding = g_module_open(path, G_MODULE_BIND_LAZY);
            if (!binding) {
                g_error("Failed to load binding module: %s", path);
                g_free(path);
                continue;
            }
            g_message("Loaded language binding module: %s", path);
            g_free(path);

            bindings = g_list_append(bindings, binding);
        }
    }

    g_dir_close(dir);

    return bindings;
}

/** post_load_modules:
 * @bindings: (element-type GModule) (transfer full):
 *
 * Unref modules opened by pre_load_modules.
 */
static void
post_load_modules(GList *bindings) {
    g_list_free_full(bindings, g_object_unref);
}

static void
srn_module_manager_init(SrnModuleManager *self) {
    GIOExtensionPoint *ep;

    self->modules = NULL;
    self->messengers = NULL;
}

static void
srn_module_manager_constructed(GObject *object) {
    G_OBJECT_CLASS(srn_module_manager_parent_class)->constructed(object);
}

static void
srn_module_manager_finalize(GObject *object) {
    SrnModuleManager *self = SRN_MODULE_MANAGER(object);

    g_list_free_full(self->messengers, g_object_unref);

    G_OBJECT_CLASS(srn_module_manager_parent_class)->finalize(object);
}

static void
srn_module_manager_class_init(SrnModuleManagerClass *class) {
    GObjectClass *object_class;

    /* Overwrite callbacks */
    object_class = G_OBJECT_CLASS(class);
    object_class->constructed = srn_module_manager_constructed;
    object_class->finalize = srn_module_manager_finalize;
    object_class->set_property = srn_module_manager_set_property;
    object_class->get_property = srn_module_manager_get_property;

    /* Install properties */
    obj_properties[PROP_MESSENGERS] =
        g_param_spec_pointer("messengers",
                             "Messengers",
                             "Instances of registered messenger.",
                             G_PARAM_READABLE);

    g_object_class_install_properties(object_class,
                                      N_PROPERTIES,
                                      obj_properties);
}

/**
 * srn_module_manager_new:
 *
 * Allocates a new #SrnModuleManager.
 *
 * Returns: (transfer full):
 */
SrnModuleManager *
srn_module_manager_new(void) {
    return SRN_MODULE_MANAGER(g_object_new(SRN_TYPE_MODULE_MANAGER, NULL));
}

/**
 * srn_module_manager_load_modules:
 * @self: A #SrnModuleManager.
 * @err: A #GError.
 */
void
srn_module_manager_load_modules(SrnModuleManager *self, GError **err) {
    GIOExtensionPoint *ep;
    GIOModuleScope *scope;
    GList *bindings;

    /* Pre load action */
    bindings = pre_load_modules();

    /* Extension point for SrnMessenger */
    ep = g_io_extension_point_register(SRN_MESSENGER_EXTENSION_POINT_NAME);
    g_io_extension_point_set_required_type(ep, SRN_TYPE_MESSENGER);
    /* TODO: More extension point */

    scope = g_io_module_scope_new(G_IO_MODULE_SCOPE_BLOCK_DUPLICATES);
    /* Scan language binding modules */
    g_io_modules_load_all_in_directory_with_scope(PACKAGE_SYS_BIND_DIR, scope);
    /* Scan other modules */
    g_io_modules_load_all_in_directory_with_scope(PACKAGE_SYS_MOD_DIR, scope);
    g_io_module_scope_free(scope);

    post_load_modules(bindings);

    for (gint i = 0; backends[i]; i++) {
        GIOExtension *ext;
        GType type;
        GObject *obj;

        ext = g_io_extension_point_get_extension_by_name(ep, backends[i]);
        if (!ext)
            continue;

        g_message("Found %s messenger", backends[i]);

        type = g_io_extension_get_type(ext);
        obj = g_object_new(type, NULL);
        self->messengers = g_list_append(self->messengers, SRN_MESSENGER(obj));

        gchar *name;
        gint version;
        g_object_get(obj,
                     "pretty-name", &name,
                     "version", &version,
                     NULL);
        g_message("Name: %s, Version: %d", name, version);
    }
}

/**
 * srn_module_manager_get_modules:
 * @self: A #SrnModuleManager.
 *
 * Returns: (element-type GIOModule) (transfer none):
 *          A #GList of #GIOModule. The list is owned by @self,
 *          and should not be modified.
 */
GList *
srn_module_manager_get_modules(SrnModuleManager *self) {
    return NULL;
}

/**
 * srn_module_manager_get_messengers:
 * @self: A #SrnModuleManager.
 *
 * Returns: (element-type SrnMessenger) (transfer none):
 *          A #GList of #SrnMessenger. The list is owned by #self,
 *          and should not be modified.
 */
GList *
srn_module_manager_get_messengers(SrnModuleManager *self) {
    return self->messengers;
}

// TODO: srn_module_manager_get_other_extensions ...
