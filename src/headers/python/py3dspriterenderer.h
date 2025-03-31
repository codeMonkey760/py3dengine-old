#ifndef PY3DENGINE_PY3DSPRITERENDERER_H
#define PY3DENGINE_PY3DSPRITERENDERER_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

struct Sprite;
struct Model;
struct Shader;

struct Py3dSpriteRenderer {
    PyObject_HEAD
    struct Sprite *sprite;
    struct Model *quad;
    struct Shader *shader;
};
extern PyTypeObject Py3dSpriteRenderer_Type;

extern int PyInit_Py3dSpriteRenderer(PyObject *module);
extern int Py3dSpriteRenderer_FindCtor(PyObject *module);
extern void Py3dSpriteRenderer_FinalizeCtor();

extern int Py3dSpriteRenderer_Check(PyObject *obj);
extern struct Py3dSpriteRenderer *Py3dSpriteRenderer_New();
extern PyObject *Py3dSpriteRenderer_Render(struct Py3dSpriteRenderer *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dSpriteRenderer_Parse(struct Py3dSpriteRenderer *self, PyObject *args, PyObject *kwds);

#endif
