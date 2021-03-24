#include <gio/gio.h>
#include <srn.h>

G_MODULE_EXPORT void
g_io_sirc_load(GIOModule *module) {
    g_message("GIOModule loaded");
    g_message("APP: %p", srn_application_get_instance());
}

G_MODULE_EXPORT void
g_io_sirc_unload(GIOModule *module) {
    g_message("GIOModule unloaded");
}
