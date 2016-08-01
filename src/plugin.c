/**
 * @file plugin.c
 * @brief simple embedding python support
 * @author Shengyu Zhang <lastavengers@outlook.com>
 * @version 1.0
 * @date 2016-04-07
 *
 * currently, plugin.c will find python modules in
 * $DESTDIR/share/srain/plugin
 */
// #define __DBG_ON
#define __LOG_ON

#include <Python.h>
#include "log.h"

char* plugin_upload(const char *path){
    char *url;

    PyObject *py_module;
    PyObject *py_func;
    PyObject *py_args;
    PyObject *py_url;

    Py_Initialize();

    /* load current dirrectory *SHOULD BE REMOVED IN RELEASE* */
    PyRun_SimpleString("import sys; sys.path.append('" PACKAGE_DATA_DIR "/share/" PACKAGE "/plugins')");

    /* import */
    py_module = PyImport_Import(PyUnicode_FromString("upload"));
    if (!py_module) {
        LOG_FR("Plugin `upload` no found");
        return NULL;
    }

    py_func = PyObject_GetAttrString(py_module, "upload");
    if (!py_func) {
        LOG_FR("Function `upload()` no found");
        return NULL;
    }

    /* build args */
    py_args = PyTuple_New(1);
    PyTuple_SetItem(py_args, 0, PyUnicode_FromString(path));

    /* call */
    py_url = PyObject_CallObject(py_func, py_args);
    url = PyUnicode_AsUTF8(py_url);

    LOG_FR("%s", url);

    Py_Finalize();

    // TODO: should url be freed?
    if (url){
        return strdup(url);
    }
    else {
        return NULL;
    }
}

char* plugin_avatar(const char *nick, const char *user, const char *host){
    char *path;
    PyObject *py_module;
    PyObject *py_func;
    PyObject *py_args;
    PyObject *py_path;

    Py_Initialize();

    /* load current dirrectory *SHOULD BE REMOVED IN RELEASE* */
    PyRun_SimpleString("import sys; sys.path.append('" PACKAGE_DATA_DIR "/share/" PACKAGE "/plugins')");

    /* import */
    py_module = PyImport_Import(PyUnicode_FromString("avatar"));
    if (!py_module) {
        LOG_FR("plugin `avatar` no found");
        return NULL;
    }

    py_func = PyObject_GetAttrString(py_module, "avatar");
    if (!py_func) {
        LOG_FR("function `avatar()` no found");
        return NULL;
    }

    /* build args */
    py_args = PyTuple_New(3);
    PyTuple_SetItem(py_args, 0, PyUnicode_FromString(nick));
    PyTuple_SetItem(py_args, 1, PyUnicode_FromString(user));
    PyTuple_SetItem(py_args, 2, PyUnicode_FromString(host));

    /* call */
    py_path = PyObject_CallObject(py_func, py_args);
    path = PyUnicode_AsUTF8(py_path);

    DBG_FR("%s", path);

    Py_Finalize();

    // TODO: should path be freed?
    if (path){
        return strdup(path);
    }
    else {
        return NULL;
    }
}
int plugin_init(){

    return 0;
}
