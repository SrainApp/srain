#include <srn-loader.h>

#include "spy-loader.h"

#define EXTENSION_NAMES "py;pyc"

struct _SpyLoader {
    GObject parent_instance;
};

static void spy_loader_interface_init(SrnLoaderInterface *iface);

G_DEFINE_TYPE_WITH_CODE(SpyLoader, spy_loader, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(SRN_TYPE_LOADER, spy_loader_interface_init))

enum {
    PROP_0,
    PROP_NAME,
    PROP_PRETTY_NAME,
    PROP_VERSION,
    PROP_EXTENSION_NAMES,
    N_PROPERTIES
};

/**
 * spy_loader_new:
 *
 * Allocates a new #SpyLoader.
 *
 * Returns: (transfer full): a #SpyLoader.
 */
SpyLoader *
spy_loader_new(void) {
    return g_object_new(SPY_TYPE_LOADER, NULL);
}

static void
spy_loader_interface_init(SrnLoaderInterface *iface) {
}

static void
spy_loader_finalize(GObject *object) {
    G_OBJECT_CLASS(spy_loader_parent_class)->finalize(object);
}

static void
spy_loader_get_property(GObject    *object,
                        guint       prop_id,
                        GValue     *value,
                        GParamSpec *pspec) {
    switch (prop_id) {
    case PROP_NAME:
        g_value_set_string(value, "spy");
        break;
    case PROP_PRETTY_NAME:
        g_value_set_string(value, "Python Loader");
        break;
    case PROP_VERSION:
        g_value_set_int(value, 1);
        break;
    case PROP_EXTENSION_NAMES:
        g_value_set_string(value, EXTENSION_NAMES);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
spy_loader_set_property(GObject      *object,
                        guint         prop_id,
                        const GValue *value,
                        GParamSpec   *pspec) {
    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
spy_loader_class_init(SpyLoaderClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->finalize = spy_loader_finalize;
    object_class->get_property = spy_loader_get_property;
    object_class->set_property = spy_loader_set_property;

    g_object_class_override_property(object_class, PROP_NAME, "name");
    g_object_class_override_property(object_class, PROP_PRETTY_NAME, "pretty-name");
    g_object_class_override_property(object_class, PROP_VERSION, "version");
    g_object_class_override_property(object_class, PROP_EXTENSION_NAMES,
                                     "extension-names");
}

static void
spy_loader_init(SpyLoader *self) {
}
