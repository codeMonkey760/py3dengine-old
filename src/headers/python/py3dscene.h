#ifndef PY3DENGINE_PY3DSCENE_H
#define PY3DENGINE_PY3DSCENE_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <GLFW/glfw3.h>

struct PhysicsSpace;

struct Py3dScene {
    PyObject_HEAD
    int enabled;
    int visible;
    PyObject *sceneGraph;
    PyObject *activeCamera;
    struct PhysicsSpace *space;
    PyObject *callbackTable[GLFW_KEY_MENU+1][GLFW_REPEAT+1][64];
    int cursorMode;
};
extern PyTypeObject Py3dScene_Type;

extern int PyInit_Py3dScene(PyObject *module);
extern int Py3dScene_FindCtor(PyObject *module);
extern void Py3dScene_FinalizeCtor();
extern int Py3dScene_Check(PyObject *obj);
extern struct Py3dScene *Py3dScene_New();
extern PyObject *Py3dScene_IsEnabled(struct Py3dScene *self, PyObject *args, PyObject *kwds);
extern int Py3dScene_IsEnabledBool(struct Py3dScene *scene);
extern PyObject *Py3dScene_Enable(struct Py3dScene *self, PyObject *args, PyObject *kwds);
extern void Py3dScene_EnableBool(struct Py3dScene *scene, int enable);
extern PyObject *Py3dScene_IsVisible(struct Py3dScene *self, PyObject *args, PyObject *kwds);
extern int Py3dScene_IsVisibleBool(struct Py3dScene *scene);
extern PyObject *Py3dScene_MakeVisible(struct Py3dScene *self, PyObject *args, PyObject *kwds);
extern void Py3dScene_MakeVisibleBool(struct Py3dScene *scene, int makeVisible);

extern void Py3dScene_KeyEvent(struct Py3dScene *self, int key, int scancode, int action, int mods);
extern PyObject *Py3dScene_SetKeyCallback(struct Py3dScene *self, PyObject *args, PyObject *kwds);
PyObject *Py3dInput_SetCursorMode(struct Py3dScene *self, PyObject *args, PyObject *kwds);

#endif
