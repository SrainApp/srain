#include <Python.h>

int plugin_init(){
    Py_Initialize();
    PyRun_SimpleString("import sys; sys.path.append('.')");
    PyRun_SimpleString("import upimg");
    PyRun_SimpleString("upimg.hello()");
    Py_Finalize();

    return 0;
}
