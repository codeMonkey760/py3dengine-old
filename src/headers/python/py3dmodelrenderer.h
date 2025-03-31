#ifndef PY3DENGINE_PY3DMODELRENDERER_H
#define PY3DENGINE_PY3DMODELRENDERER_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

struct Model;
struct Shader;
struct Material;

struct Py3dModelRenderer {
    PyObject_HEAD
    struct Shader *shader;
    struct Model *model;
    struct Material *material;
};

extern int PyInit_Py3dModelRenderer(PyObject *module);
extern int Py3dModelRenderer_FindCtor(PyObject *module);
extern void Py3dModelRenderer_FinalizeCtor();
extern struct Py3dModelRenderer *Py3dModelRenderer_New();
extern int Py3dModelRenderer_Check(PyObject *obj);

#endif
