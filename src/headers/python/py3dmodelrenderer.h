#ifndef PY3DENGINE_PY3DMODELRENDERER_H
#define PY3DENGINE_PY3DMODELRENDERER_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdbool.h>

#include "py3dcomponent.h"

struct Model;
struct Shader;
struct Material;

struct Py3dModelRenderer {
    struct Py3dComponent base;
    struct Shader *shader;
    struct Model *model;
    struct Material *material;
};

extern bool PyInit_Py3dModelRenderer(PyObject *module);
extern bool Py3dModelRenderer_FindCtor(PyObject *module);
extern void Py3dModelRenderer_FinalizeCtor();
extern struct Py3dModelRenderer *Py3dModelRenderer_New();

#endif
