#ifndef PY3DENGINE_RENDERING_CONTEXT_H
#define PY3DENGINE_RENDERING_CONTEXT_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>
#include <stdbool.h>

struct Py3dRenderingContext;

struct RenderingContext {
    float vpMtx[16];
    float cameraPositionW[3];
    struct Py3dRenderingContext *py3dRenderingContext;
};

struct Py3dRenderingContext {
    PyObject_HEAD
    struct RenderingContext *renderingContext;
};

extern PyTypeObject Py3dRenderingContext_Type;

extern bool PyInit_Py3dRenderingContext(PyObject *module);
extern bool Py3dRenderingContext_FindCtor(PyObject *module);
extern void Py3dRenderingContext_FinalizeCtor();
extern struct Py3dRenderingContext *Py3dRenderingContext_New();

extern void allocRenderingContext(struct RenderingContext **contextPtr);
extern void deleteRenderingContext(struct RenderingContext **contextPtr);
extern void initRenderingContext(struct RenderingContext *context, PyObject *activeCamera);

#endif
