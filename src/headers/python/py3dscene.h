#ifndef PY3DENGINE_PY3DSCENE_H
#define PY3DENGINE_PY3DSCENE_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

struct Py3dScene;
extern PyTypeObject Py3dScene_Type;

extern int PyInit_Py3dScene(PyObject *module);
extern int Py3dScene_FindCtor(PyObject *module);
extern void Py3dScene_FinalizeCtor();
extern int Py3dScene_Check(PyObject *obj);
extern struct Py3dScene *Py3dScene_New();
extern int Py3dScene_IsEnabled(struct Py3dScene *scene);
extern void Py3dScene_Enable(struct Py3dScene *scene, int enable);
extern int Py3dScene_IsVisible(struct Py3dScene *scene);
extern void Py3dScene_MakeVisible(struct Py3dScene *scene, int visible);


#endif
