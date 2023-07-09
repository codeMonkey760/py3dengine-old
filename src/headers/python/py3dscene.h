#ifndef PY3DENGINE_PY3DSCENE_H
#define PY3DENGINE_PY3DSCENE_H

#define PY_SSIZE_T_CLEAN
#include <Python.h>

#include <GLFW/glfw3.h>

struct PhysicsSpace;
struct Py3dResourceManager;

struct Py3dScene {
    PyObject_HEAD
    int enabled;
    int visible;
    PyObject *name;
    PyObject *sceneGraph;
    PyObject *activeCamera;
    PyObject *resourceManager;
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
extern void Py3dScene_Activate(struct Py3dScene *self);
extern void Py3dScene_Deactivate(struct Py3dScene *self);
extern PyObject *Py3dScene_IsEnabled(struct Py3dScene *self, PyObject *args, PyObject *kwds);
extern int Py3dScene_IsEnabledBool(struct Py3dScene *scene);
extern PyObject *Py3dScene_Enable(struct Py3dScene *self, PyObject *args, PyObject *kwds);
extern void Py3dScene_EnableBool(struct Py3dScene *scene, int enable);
extern PyObject *Py3dScene_IsVisible(struct Py3dScene *self, PyObject *args, PyObject *kwds);
extern int Py3dScene_IsVisibleBool(struct Py3dScene *scene);
extern PyObject *Py3dScene_MakeVisible(struct Py3dScene *self, PyObject *args, PyObject *kwds);
extern void Py3dScene_MakeVisibleBool(struct Py3dScene *scene, int makeVisible);
extern PyObject *Py3dScene_GetName(struct Py3dScene *self, PyObject *Py_UNUSED(ignored));
extern void Py3dScene_SetNameCStr(struct Py3dScene *self, const char *newName);
extern void Py3dScene_Start(struct Py3dScene *self);
extern void Py3dScene_Update(struct Py3dScene *self, float dt);
extern void Py3dScene_Render(struct Py3dScene *self);
extern void Py3dScene_End(struct Py3dScene *self);
extern void Py3dScene_SetResourceManager(struct Py3dScene *self, PyObject *newManager);
extern void Py3dScene_SetSceneGraph(struct Py3dScene *self, PyObject *newSceneGraph);
extern PyObject *Py3dScene_ActivateCamera(struct Py3dScene *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dScene_ActivateCameraByName(struct Py3dScene *self, PyObject *args, PyObject *kwds);
extern PyObject *Py3dScene_ActivateCameraByNameCStr(struct Py3dScene *self, const char *name);

extern void Py3dScene_KeyEvent(struct Py3dScene *self, int key, int scancode, int action, int mods);
extern PyObject *Py3dScene_SetKeyCallback(struct Py3dScene *self, PyObject *args, PyObject *kwds);
PyObject *Py3dScene_SetCursorMode(struct Py3dScene *self, PyObject *args, PyObject *kwds);

#endif
