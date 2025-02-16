#ifndef PY3DLIGHT_H
#define PY3DLIGHT_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

struct LightData;
struct Py3dLight;
extern PyTypeObject Py3dLight_Type;

extern int PyInit_Py3dLight(PyObject *module);
extern int Py3dLight_FindCtor(PyObject *module);
extern void Py3dLight_FinalizeCtor();
extern struct Py3dLight *Py3dLight_New();
extern int Py3dLight_Check(PyObject *obj);

extern void Py3dLight_GetType(struct Py3dLight *self, int *dst);
extern void Py3dLight_GetDiffuse(struct Py3dLight *self, float dst[3]);
extern void Py3dLight_GetSpecular(struct Py3dLight *self, float dst[3]);
extern void Py3dLight_GetAmbient(struct Py3dLight *self, float dst[3]);
extern void Py3dLight_GetIntensity(struct Py3dLight *self, float *dst);
extern void Py3dLight_GetAttenuation(struct Py3dLight *self, float dst[3]);

extern PyObject *Py3dLight_Parse(struct Py3dLight *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dLight_Attach(struct Py3dLight *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dLight_Detach(struct Py3dLight *self, PyObject *args, PyObject *kwds);

extern PyObject *Py3dLight_SetType(struct Py3dLight *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dLight_SetDiffuseColor(struct Py3dLight *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dLight_SetSpecularColor(struct Py3dLight *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dLight_SetAmbientColor(struct Py3dLight *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dLight_SetIntensity(struct Py3dLight *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dLight_SetAttenuation(struct Py3dLight *self, PyObject *args, PyObject *kwds);

#endif
