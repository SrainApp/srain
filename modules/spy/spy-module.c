#include <glib.h>
#include <gio/gio.h>
#include <gobject/gobject.h>

#include <python3.9/Python.h>

#include <pygobject.h>

G_MODULE_EXPORT void
g_io_spy_load(GIOModule *module) {
    GBytes *data;

    g_message("GIOModule spy loaded");
    g_type_module_use(G_TYPE_MODULE(module));

    data = g_resources_lookup_data("/im/srain/Srain/PythonLoader/main.py", 0, NULL);
    g_message("Data: %p", data);
    Py_Initialize();
    pygobject_init(-1, -1, -1);

    g_message("PyRun %.*s",
              g_bytes_get_size(data),
              g_bytes_get_data(data, NULL));

    PyRun_SimpleString(g_bytes_get_data(data, NULL));
    PyRun_SimpleString("print(1)");

    g_message("PyRun Done");

    g_bytes_unref(data);
}

G_MODULE_EXPORT void
g_io_spy_unload(GIOModule *module) {
    g_message("GIOModule spy unloaded");
}
