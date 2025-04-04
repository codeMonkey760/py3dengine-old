#ifndef PY3DENGINE_PYTHON_UTIL_H
#define PY3DENGINE_PYTHON_UTIL_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

struct Py3dGameObject;

extern void handleException();
extern void forceGarbageCollection();
extern void dumpPythonObjects();
extern PyObject *Py3d_GetTypeFromTuple(PyObject *tuple, Py_ssize_t index, PyTypeObject *type);

extern int Py3d_GetBooleanParseData(PyObject *parseDataDict, const char *keyName, int *dst, const char *componentName);
extern int Py3d_GetIntParseData(PyObject *parseDataDict, const char *keyName, int *dst, const char *componentName);
extern int Py3d_GetFloatParseData(PyObject *parseDataDict, const char *keyName, float *dst, const char *componentName);
extern int Py3d_GetVector3ParseData(PyObject *parseDataDict, const char *keyName, float dst[3], const char *componentName);

#endif
