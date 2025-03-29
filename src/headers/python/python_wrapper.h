#ifndef PY3DENGINE_PYTHON_WRAPPER_H
#define PY3DENGINE_PYTHON_WRAPPER_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

extern int initializePython(int argc, char **argv);
extern void appendImportPath(const char *relPath);
extern void finalizePython();
extern PyObject *getPy3dEngineModule();

#endif
