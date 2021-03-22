#include <sif/sif.h>

#include "sirc-messenger.h"

#define SCHEMAS "irc;ircs"

struct _SircMessenger {
    GObject parent_instance;
};

static void sirc_messenger_interface_init(SifMessengerInterface *iface);

G_DEFINE_TYPE_WITH_CODE(SircMessenger, sirc_messenger, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(SIF_TYPE_MESSENGER,
                                sirc_messenger_interface_init))

enum {
    PROP_0,
    PROP_SCHEMAS,
    N_PROPERTIES
};

/**
 * sirc_messenger_new:
 *
 * Allocates a new #SircMessenger.
 *
 * Returns: (transfer full): a #SircMessenger.
 */
SircMessenger *
sirc_messenger_new(void) {
    return g_object_new(SIRC_TYPE_MESSENGER, NULL);
}

static void
sirc_messenger_interface_init(SifMessengerInterface *iface) {
}

static void
sirc_messenger_finalize(GObject *object) {
    G_OBJECT_CLASS(sirc_messenger_parent_class)->finalize(object);
}

static void
sirc_messenger_get_property(GObject    *object,
                            guint       prop_id,
                            GValue     *value,
                            GParamSpec *pspec) {
    switch (prop_id) {
    case PROP_SCHEMAS:
        g_value_set_string(value, SCHEMAS);
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
sirc_messenger_set_property(GObject      *object,
                            guint         prop_id,
                            const GValue *value,
                            GParamSpec   *pspec) {
    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
sirc_messenger_class_init(SircMessengerClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->finalize = sirc_messenger_finalize;
    object_class->get_property = sirc_messenger_get_property;
    object_class->set_property = sirc_messenger_set_property;

    g_object_class_override_property(object_class, PROP_SCHEMAS, "schemas");
}

static void
sirc_messenger_init(SircMessenger *self) {
}
