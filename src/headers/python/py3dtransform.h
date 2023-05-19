#ifndef PY3DENGINE_PY3DTRANSFORM_H
#define PY3DENGINE_PY3DTRANSFORM_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <ode/ode.h>

#include <stdbool.h>

#include "physics/collision.h"
#include "py3dcomponent.h"

struct Py3dTransform {
    struct Py3dComponent base;
    // TODO: these will be defined in world space until matrix chaining has been implemented
    // Then they will be defined relative to the parent game object's space
    float position[3];
    float orientation[4];
    float scale[3];
    bool matrixCacheDirty;
    bool viewMatrixCacheDirty;
    float wMatrixCache[16];
    float witMatrixCache[16];
    float viewMatrixCache[16];
    dBodyID dynamicsBody;
};

extern bool PyInit_Py3dTransform(PyObject *module);
extern bool Py3dTransform_FindCtor(PyObject *module);
extern void Py3dTransform_FinalizeCtor();
extern struct Py3dTransform *Py3dTransform_New();
extern int Py3dTransform_Check(PyObject *obj);
extern float *getTransformWorldMtx(struct Py3dTransform *component);
extern float *getTransformWITMtx(struct Py3dTransform *component);
extern float *getTransformViewMtx(struct Py3dTransform *component);

#endif
