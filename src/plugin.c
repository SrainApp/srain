#include <Python.h>

int plugin_init(){
    Py_Initialize();
    PyRun_SimpleString("import sys; sys.path.append('.')");
    PyRun_SimpleString("from upimg import upimg");
    PyRun_SimpleString("print(upimg())");
    Py_Finalize();

    return 0;
}
