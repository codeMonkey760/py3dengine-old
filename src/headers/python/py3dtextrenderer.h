#ifndef PY3DENGINE_PY3DTEXTRENDERER_H
#define PY3DENGINE_PY3DTEXTRENDERER_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include "python/py3dcomponent.h"

struct Py3dTextRenderer;
extern PyTypeObject Py3dTextRenderer_Type;

extern int PyInit_Py3dTextRenderer(PyObject *module);

extern int Py3dTextRenderer_Check(PyObject *obj);
extern struct Py3dTextRenderer *Py3dTextRenderer_New();
extern PyObject *Py3dTextRenderer_Render(struct Py3dTextRenderer *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dTextRenderer_Parse(struct Py3dTextRenderer *self, PyObject *args, PyObject *kwds);

extern PyObject *Py3dTextRenderer_SetText(struct Py3dTextRenderer *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dTextRenderer_SetColor(struct Py3dTextRenderer *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dTextRenderer_SetTextJustify(struct Py3dTextRenderer *self, PyObject *args, PyObject *kwds);

#endif
