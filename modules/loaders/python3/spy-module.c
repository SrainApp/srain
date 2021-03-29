#include <gio/gio.h>
#include <srn-loader.h>

#include "spy-loader.h"

#define PY_SSIZE_T_CLEAN
#include <Python.h>

// For test
// #include <girepository.h>
// #include <pygobject.h>

G_MODULE_EXPORT void
g_io_spy_load(GIOModule *module) {
    g_message("GIOModule spy loaded");

    g_type_module_use(G_TYPE_MODULE(module));

    g_io_extension_point_implement(SRN_LOADER_EXTENSION_POINT_NAME,
                                   SPY_TYPE_LOADER,
                                   "Srain Python3 Loader",
                                   10);

    // GBytes *data;
    // g_message("GIOModule spy loaded");

    // /* Init GObject Introspection repository */
    // g_irepository_prepend_search_path("/home/la/git/srain/prefix/lib/girepository-1.0");
    // g_irepository_prepend_library_path("/home/la/git/srain/prefix/lib");

    // data = g_resources_lookup_data("/im/srain/Srain/PythonLoader/main.py", 0, NULL);
    // g_message("Data: %p", data);
    // g_message("PyUnicode_FromFromat: %p", PyUnicode_FromFormat);
    // Py_Initialize();
    // PyObject *pyobj = pygobject_init(-1, -1, -1);

    // if (PyErr_Occurred()) {
    //     g_warning("Error initializing Python Plugin Loader: "
    //               "PyGObject initialization failed");
    // }
    // /* Initialize support for threads */
    // // pyg_enable_threads();
    // // PyEval_InitThreads();

    // GString *str = g_string_new_len(g_bytes_get_data(data, NULL),
    //                                 g_bytes_get_size(data));
    // PyRun_SimpleString(str->str);
    // // g_message("Py exit code: %d", Py_FinalizeEx());

    // // g_message("PyRun Done");

    // g_bytes_unref(data);
}

G_MODULE_EXPORT void
g_io_spy_unload(GIOModule *module) {
    g_message("GIOModule spy unloaded");
}

G_MODULE_EXPORT gchar **
g_io_spy_query(void) {
    gchar *eps[] = {SRN_LOADER_EXTENSION_POINT_NAME, NULL};
    return g_strdupv(eps);
}
