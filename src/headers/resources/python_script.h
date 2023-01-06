#ifndef PY3DENGINE_PYTHON_SCRIPT_H
#define PY3DENGINE_PYTHON_SCRIPT_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "resources/base_resource.h"

#define RESOURCE_TYPE_NAME_PYTHON_SCRIPT "PythonScript"

struct PythonScript {
    struct BaseResource _base;

    PyObject *_componentType;
};

struct PythonComponent;

extern bool isResourceTypePythonScript(struct BaseResource *resource);
extern void allocPythonScript(struct PythonScript **pythonScriptPtr);
extern void deletePythonScript(struct PythonScript **pythonScript);

extern void initPythonScript(struct PythonScript *resource, PyObject *module, const char *newComponentTypeName);
extern void createPythonComponent(struct PythonScript *resource, struct PythonComponent **componentPtr);
extern PyObject *getPythonScriptType(struct PythonScript *resource);

#endif
