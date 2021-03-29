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
#include "srn-loader.h"

#include "srn-extension-manager.h"

struct _SrnExtensionManager {
    GtkApplication parent;

    GList *modules;
    GList *extensions;
    // TODO: modues unload
};

/*********************
 * GObject functions *
 *********************/

enum {
    PROP_0,
    PROP_MODULES,
    PROP_EXTENSIONS,
    N_PROPERTIES
};

G_DEFINE_TYPE(SrnExtensionManager, srn_extension_manager, G_TYPE_OBJECT);

static GParamSpec *obj_properties[N_PROPERTIES] = { };

static void
srn_extension_manager_set_property(GObject *object, guint property_id,
                                   const GValue *value, GParamSpec *pspec) {
    switch (property_id) {
    default:
        /* We don't have any other property... */
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, property_id, pspec);
        break;
    }
}

static void
srn_extension_manager_get_property(GObject *object, guint property_id,
                                   GValue *value, GParamSpec *pspec) {
    SrnExtensionManager *self = SRN_EXTENSION_MANAGER(object);

    switch (property_id) {
    case PROP_MODULES:
        g_value_set_pointer(value, srn_extension_manager_get_modules(self));
    case PROP_EXTENSIONS:
        g_value_set_pointer(value, srn_extension_manager_get_extensions(self));
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

/* pre_load_modules:
 *
 * The implementation of GIOModule loads GModule with
 * G_MODULE_BIND_LOCAL flags, which causes other share libraries
 * (such as pygobject's lib) can not use symbols (such as symbols
 * in libpython.so) introduced by loader module.
 *
 * So we open loader modules with correct flags before loading
 * them as GIOModule.
 */
static void
pre_load_modules(void) {
    const gchar *name;
    GDir *dir;

    g_return_if_fail(g_module_supported());

    dir = g_dir_open(PACKAGE_SYS_BIND_DIR, 0, NULL);
    if (!dir) return;

    while ((name = g_dir_read_name(dir))) {
        if (is_valid_module_name(name)) {
            gchar *path;

            path = g_build_filename(PACKAGE_SYS_BIND_DIR, name, NULL);
            /* NOTE:
             *
             * The implementation of GIOModule loads GModule with
             * G_MODULE_BIND_LOCAL flags, which causes other share libraries
             * (such as pygobject's lib) can not use symbols (such as symbols
             * in libpython.so) introduced by loader module.
             *
             * So we open loader modules with correct flags before loading
             * them as GIOModule. Don't care about leaking because we no need
             * to unload any modules.
             * */

            if (!g_module_open(path, G_MODULE_BIND_LAZY)) {
                g_error("Failed to open loader module: %s", path);
            } else {
                g_message("Pre-open language loader module: %s", path);
            }
            g_free(path);
        }
    }

    g_dir_close(dir);
}

static void
srn_extension_manager_init(SrnExtensionManager *self) {
    self->modules = NULL;
    self->extensions = NULL;
}

static void
srn_extension_manager_constructed(GObject *object) {
    G_OBJECT_CLASS(srn_extension_manager_parent_class)->constructed(object);
}

static void
srn_extension_manager_finalize(GObject *object) {
    SrnExtensionManager *self = SRN_EXTENSION_MANAGER(object);

    g_list_free_full(self->modules, (GDestroyNotify)g_type_module_unuse);

    G_OBJECT_CLASS(srn_extension_manager_parent_class)->finalize(object);
}

static void
srn_extension_manager_class_init(SrnExtensionManagerClass *class) {
    GObjectClass *object_class;

    /* Overwrite callbacks */
    object_class = G_OBJECT_CLASS(class);
    object_class->constructed = srn_extension_manager_constructed;
    object_class->finalize = srn_extension_manager_finalize;
    object_class->set_property = srn_extension_manager_set_property;
    object_class->get_property = srn_extension_manager_get_property;

    /* Install properties */
    obj_properties[PROP_MODULES] =
        g_param_spec_pointer("modules",
                             "Modules",
                             "List of GIOModule.",
                             G_PARAM_READABLE);

    obj_properties[PROP_EXTENSIONS] =
        g_param_spec_pointer("extensions",
                             "Extensions",
                             "List of GIOExtension.",
                             G_PARAM_READABLE);

    g_object_class_install_properties(object_class,
                                      N_PROPERTIES,
                                      obj_properties);
}

/**
 * srn_extension_manager_new:
 *
 * Allocates a new #SrnExtensionManager.
 *
 * Returns: (transfer full):
 */
SrnExtensionManager *
srn_extension_manager_new(void) {
    return SRN_EXTENSION_MANAGER(g_object_new(SRN_TYPE_EXTENSION_MANAGER, NULL));
}

/**
 * srn_extension_manager_load_modules:
 * @self: A #SrnExtensionManager.
 * @err: A #GError.
 */
void
srn_extension_manager_load_modules(SrnExtensionManager *self, GError **err) {
    GIOExtensionPoint *msger_ep;
    GIOExtensionPoint *loader_ep;
    GIOModuleScope *scope;

    /* Prevent dulplicate loading */
    g_return_if_fail(!self->modules);
    g_return_if_fail(!self->extensions);

    /* Pre load action */
    pre_load_modules();

    /* Extension point for SrnMessenger */
    msger_ep = g_io_extension_point_register(SRN_MESSENGER_EXTENSION_POINT_NAME);
    g_io_extension_point_set_required_type(msger_ep, SRN_TYPE_MESSENGER);
    /* TODO: More extension point */

    loader_ep = g_io_extension_point_register(SRN_LOADER_EXTENSION_POINT_NAME);
    g_io_extension_point_set_required_type(loader_ep, SRN_TYPE_LOADER);

    scope = g_io_module_scope_new(G_IO_MODULE_SCOPE_BLOCK_DUPLICATES);
    /* Scan language loader modules */
    self->modules = g_io_modules_load_all_in_directory_with_scope(
                        PACKAGE_SYS_BIND_DIR, scope);
    /* Scan other modules */
    self->modules = g_list_concat(self->modules,
                                  g_io_modules_load_all_in_directory_with_scope(PACKAGE_SYS_MOD_DIR, scope));
    g_io_module_scope_free(scope);

    for (GList *lst = g_io_extension_point_get_extensions(msger_ep); lst;
         lst = g_list_next(lst)) {
        GIOExtension *ext;

        ext = lst->data;
        self->extensions = g_list_append(self->extensions, ext);
        g_message("Found messenger extension, name: %s, type: %s",
                  g_io_extension_get_name(ext),
                  g_type_name(g_io_extension_get_type(ext)));
    }

    for (GList *lst = g_io_extension_point_get_extensions(loader_ep); lst;
         lst = g_list_next(lst)) {
        GIOExtension *ext;

        ext = lst->data;
        self->extensions = g_list_append(self->extensions, ext);
        g_message("Found loader extension, name: %s, type: %s",
                  g_io_extension_get_name(ext),
                  g_type_name(g_io_extension_get_type(ext)));
    }
}

/**
 * srn_extension_manager_get_modules:
 * @self: A #SrnExtensionManager.
 *
 * Returns: (element-type GIOModule) (transfer none):
 *          A #GList of #GIOModule. The list is owned by @self,
 *          and should not be modified.
 */
GList *
srn_extension_manager_get_modules(SrnExtensionManager *self) {
    return self->modules;
}

/**
 * srn_extension_manager_get_extensions:
 * @self: A #SrnExtensionManager.
 *
 * Returns: (element-type GIOExtension) (transfer none):
 *          A #GList of #GIOExtension. The list is owned by #self,
 *          and should not be modified.
 */
GList *
srn_extension_manager_get_extensions(SrnExtensionManager *self) {
    return self->extensions;
}

// TODO: srn_extension_manager_get_other_extensions ...
