#include <srn-flow.h>

#include "sirc-login-flow.h"

struct _SircLoginFlow {
    GObject parent_instance;
};

static void sirc_login_flow_interface_init(SrnFlowInterface *iface);

G_DEFINE_TYPE_WITH_CODE(SircLoginFlow, sirc_login_flow, G_TYPE_OBJECT,
                        G_IMPLEMENT_INTERFACE(SRN_TYPE_FLOW,
                                sirc_login_flow_interface_init))

enum {
    PROP_0,
    PROP_NAME,
    N_PROPERTIES
};

/**
 * sirc_login_flow_new:
 *
 * Allocates a new IRC login flow.
 *
 * Returns: (transfer full):
 */
SircLoginFlow *
sirc_login_flow_new(void) {
    return g_object_new(SIRC_TYPE_LOGIN_FLOW, NULL);
}

static void
sirc_login_flow_interface_init(SrnFlowInterface *iface) {
}

static void
sirc_login_flow_finalize(GObject *object) {
    G_OBJECT_CLASS(sirc_login_flow_parent_class)->finalize(object);
}

static void
sirc_login_flow_get_property(GObject    *object,
                             guint       prop_id,
                             GValue     *value,
                             GParamSpec *pspec) {
    switch (prop_id) {
    case PROP_NAME:
        g_value_set_string(value, "sirc");
        break;
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
sirc_login_flow_set_property(GObject      *object,
                             guint         prop_id,
                             const GValue *value,
                             GParamSpec   *pspec) {
    switch (prop_id) {
    default:
        G_OBJECT_WARN_INVALID_PROPERTY_ID(object, prop_id, pspec);
    }
}

static void
sirc_login_flow_class_init(SircLoginFlowClass *klass) {
    GObjectClass *object_class = G_OBJECT_CLASS(klass);

    object_class->finalize = sirc_login_flow_finalize;
    object_class->get_property = sirc_login_flow_get_property;
    object_class->set_property = sirc_login_flow_set_property;

    g_object_class_override_property(object_class, PROP_NAME, "name");
}

static void
sirc_login_flow_init(SircLoginFlow *self) {
}
