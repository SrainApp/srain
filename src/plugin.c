/**
 * @file plugin.c
 * @brief Simple embedding python support
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-04-07
 */

// #define __DBG_ON
#define __LOG_ON

#include <glib.h>
#include <Python.h>

#include "i18n.h"
#include "meta.h"
#include "log.h"

#define PLUGIN_COUNT    2
#define PLUGIN_UPLOAD   0
#define PLUGIN_AVATAR   1

typedef struct {
    const char *mod_name;
    const char *func_name;
    PyObject *func;
} Plugin;

Plugin plugins[PLUGIN_COUNT] = {
    {"upload", "upload", NULL},
    {"avatar", "avatar", NULL},
};

/**
 * @brief Given a file path, upload it to internet
 *
 * @param path
 *
 * @return A URL of the uploaded file
 *
 * NOTE: This function is blocked.
 */
char* plugin_upload(const char *path){
    char *url;
    PyObject *py_args;
    PyObject *py_path;
    PyObject *py_url;

    if (!plugins[PLUGIN_UPLOAD].func)
        return _("Plugin no loaded");

    /* Build arguments */
    py_path = PyUnicode_DecodeFSDefault(path);
    if (!py_path){
        PyErr_Print();
        ERR_FR("Failed to convert %s", path);
        return NULL;
    }

    py_args = PyTuple_New(1);
    PyTuple_SetItem(py_args, 0, py_path);

    /* Call python function */
    py_url = PyObject_CallObject(plugins[PLUGIN_UPLOAD].func, py_args);
    Py_DECREF(py_args);

    if (!py_url){
        PyErr_Print();
        ERR_FR("Function `%s` failed", plugins[PLUGIN_UPLOAD].func_name);
        return NULL;
    }

    url = PyUnicode_AsUTF8(py_url);
    Py_DECREF(py_url);
    // TODO: leak?

    if (!url){
        PyErr_Print();
        ERR_FR("Failed to convert URL to UTF-8");
        return NULL;
    }

    LOG_FR("url: %s", url);

    return strdup(url);
}

/**
 * @brief Download `nick`'s avatar according to `token`
 *
 * @param nick
 * @param token
 */
int plugin_avatar(const char *nick, const char *token){
    char *path;
    PyObject *py_nick;
    PyObject *py_token;
    PyObject *py_args;

    if (!plugins[PLUGIN_AVATAR].func) return -1;
    if (!nick || !token) return -1;

    /* Build arguments */
    py_nick = PyUnicode_FromString(nick);
    if (!py_nick) return -1;
    py_token = PyUnicode_FromString(token);
    if (!py_token) return -1;

    py_args = PyTuple_New(2);
    PyTuple_SetItem(py_args, 0, py_nick);
    PyTuple_SetItem(py_args, 1, py_token);

    /* Call python function */
    PyObject_CallObject(plugins[PLUGIN_AVATAR].func, py_args);
    PyErr_Print();
    Py_DECREF(py_args);

    return 0;
}

void plugin_init(){
    PyObject *py_name;
    PyObject *py_module;
    PyObject *py_func;

    LOG_FR("...");

    Py_Initialize();
    PyRun_SimpleString("import sys");

    GString *plugin_path = g_string_new("");
    g_string_append_printf(plugin_path, "sys.path.append('%s');",
            PACKAGE_DATA_DIR "/share/" PACKAGE "/plugins");
    g_string_append_printf(plugin_path, "sys.path.append('%s/srain/plugins');",
            g_get_user_config_dir());

    PyRun_SimpleString(plugin_path->str);

    g_string_free(plugin_path, TRUE);

    int i;
    for (i = 0; i < PLUGIN_COUNT; i++){
        /* Load plugin `upload' */
        py_name = PyUnicode_FromString(plugins[i].mod_name);

        py_module = PyImport_Import(py_name);
        Py_DECREF(py_name);

        if (!py_module) {
            PyErr_Print();
            LOG_FR("Plugin `%s` no found", plugins[i].mod_name);
            continue;
        }

        py_func = PyObject_GetAttrString(py_module, plugins[i].func_name);
        Py_DECREF(py_module);

        if (!py_func || !PyCallable_Check(py_func)) {
            PyErr_Print();
            WARN_FR("Function `%s` no found in module `%s`",
                    plugins[i].func_name, plugins[i].mod_name);
            continue;
        }

        LOG_FR("Plugin `%s` loaded", plugins[i].mod_name);

        plugins[i].func = py_func;
    }
}

void plugin_finalize(){
    int i;

    LOG_FR("...");

    for (i = 0; i < PLUGIN_COUNT; i++){
        if (plugins[i].func){
            Py_DECREF(plugins[i].func);
        }
    }

    Py_Finalize();
}
