#define __LOG_ON

#include <Python.h>
#include "log.h"

int plugin_init(){
    return 0;
}

void plugin_upload(const char *path){
    char *url;

    PyObject *py_module;
    PyObject *py_func;
    PyObject *py_path;
    PyObject *py_args;
    PyObject *py_url;

    Py_Initialize();

    /* load current dirrectory *SHOULD BE REMOVED IN RELEASE* */
    PyRun_SimpleString("import sys; sys.path.append('.')");

    /* import */
    py_module = PyImport_Import(PyUnicode_FromString("upload"));
    if (!py_module) {
        LOG_FR("plugin upload no found");
        return;
    }

    py_func = PyObject_GetAttrString(py_module, "upload");
    if (!py_func) {
        LOG_FR("function upload() no found");
        return;
    }

    /* build args */
    py_args = PyTuple_New(1);
    PyTuple_SetItem(py_args, 0, PyUnicode_FromString(path));

    /* call */
    py_url = PyObject_CallObject(py_func, py_args);
    url = PyUnicode_AsUTF8(py_url);

    LOG_FR("%s", url);
    Py_Finalize();
}

void plugin_avatar(const char *nick, const char *host, const char *fullname){

}
