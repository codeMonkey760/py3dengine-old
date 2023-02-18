#ifndef PY3DENGINE_PY3DRENDERINGCONTEXT_H
#define PY3DENGINE_PY3DRENDERINGCONTEXT_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

struct Py3dRenderingContext {
    PyObject_HEAD
    float vpMtx[16];
    float cameraPositionW[3];
};

extern PyTypeObject Py3dRenderingContext_Type;
struct Py3dGameObject;

extern int PyInit_Py3dRenderingContext(PyObject *module);
extern int Py3dRenderingContext_FindCtor(PyObject *module);
extern void Py3dRenderingContext_FinalizeCtor();

extern struct Py3dRenderingContext *Py3dRenderingContext_New(struct Py3dGameObject *activeCamera);
extern int Py3dRenderingContext_Check(PyObject *obj);

#endif


