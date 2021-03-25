#include <glib.h>
#include <gio/gio.h>
#include <gobject/gobject.h>

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <pygobject.h>

G_MODULE_EXPORT void
g_io_spy_load(GIOModule *module) {
    GBytes *data;

    g_message("GIOModule spy loaded");
    g_type_module_use(G_TYPE_MODULE(module));

    data = g_resources_lookup_data("/im/srain/Srain/PythonLoader/main.py", 0, NULL);
    g_message("Data: %p", data);
    g_message("PyUnicode_FromFromat: %p", PyUnicode_FromFormat);
    Py_Initialize();
    PyObject *pyobj = pygobject_init(-1, -1, -1);
    /* Initialize support for threads */
    // pyg_enable_threads();
    // PyEval_InitThreads();

    // GString *str = g_string_new_len(g_bytes_get_data(data, NULL),
    //                                 g_bytes_get_size(data));
    // PyRun_SimpleString(str->str);
    g_message("Py exit code: %d", Py_FinalizeEx());

    // g_message("PyRun Done");

    g_bytes_unref(data);
}

G_MODULE_EXPORT void
g_io_spy_unload(GIOModule *module) {
    g_message("GIOModule spy unloaded");
}
