#include <gio/gio.h>
#include <srn-messenger.h>

#include "sirc-messenger.h"

G_MODULE_EXPORT void
g_io_sirc_load(GIOModule *module) {
    g_message("GIOModule sirc loaded");

    g_type_module_use(G_TYPE_MODULE(module));

    g_io_extension_point_implement(SRN_MESSENGER_EXTENSION_POINT_NAME,
                                   SIRC_TYPE_MESSENGER,
                                   "Srain IRC Messenger",
                                   10);
}

G_MODULE_EXPORT void
g_io_sirc_unload(GIOModule *module) {
    g_message("GIOModule sirc unloaded");
}

G_MODULE_EXPORT gchar **
g_io_sirc_query(void) {
    gchar *eps[] = {SRN_MESSENGER_EXTENSION_POINT_NAME, NULL};
    return g_strdupv(eps);
}
