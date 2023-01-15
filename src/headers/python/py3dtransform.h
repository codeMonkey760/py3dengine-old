#ifndef PY3DENGINE_PY3DTRANSFORM_H
#define PY3DENGINE_PY3DTRANSFORM_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <stdbool.h>

#include "pycomponent.h"

struct Py3dTransform {
    struct Py3dComponent base;
    // TODO: these will be defined in world space until matrix chaining has been implemented
    // Then they will be defined relative to the parent game object's space
    float __position[3];
    float __orientation[4];
    float __scale[3];
    bool __matrixCacheDirty;
    bool __viewMatrixCacheDirty;
    float __wMatrixCache[16];
    float __witMatrixCache[16];
    float __viewMatrixCache[16];
};

extern bool PyInit_Py3dTransform(PyObject *module);
extern bool Py3dTransform_FindCtor(PyObject *module);
extern void Py3dTransform_FinalizeCtor();
extern struct Py3dTransform *Py3dTransform_New();
extern float *getTransformWorldMtx(struct Py3dTransform *component);
extern float *getTransformWITMtx(struct Py3dTransform *component);
extern float *getTransformViewMtx(struct Py3dTransform *component);

#endif
