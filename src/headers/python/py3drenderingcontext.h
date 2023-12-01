#ifndef PY3DENGINE_PY3DRENDERINGCONTEXT_H
#define PY3DENGINE_PY3DRENDERINGCONTEXT_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

struct Py3dScene;
struct Py3dGameObject;
struct Py3dRenderingContext;
extern PyTypeObject Py3dRenderingContext_Type;

extern int PyInit_Py3dRenderingContext(PyObject *module);
extern int Py3dRenderingContext_FindCtor(PyObject *module);
extern void Py3dRenderingContext_FinalizeCtor();

extern struct Py3dRenderingContext *Py3dRenderingContext_New(struct Py3dScene *scene);
extern int Py3dRenderingContext_Check(PyObject *obj);
extern int Py3dRenderingContext_SetCamera(struct Py3dRenderingContext *self, struct Py3dGameObject *newCamera);
extern void 

#endif


