#include "logger.h"
#include "python/python_util.h"
#include "python/py3dgameobject.h"
#include "python/component_helper.h"

static PyTypeObject *componentType = NULL;

int Py3d_InitComponentHelper() {
    PyObject *componentMod = PyImport_ImportModule("Component");
    if (componentMod == NULL) return 0;

    // TODO: can I do this?
    componentType = (PyTypeObject *) PyObject_GetAttrString(componentMod, "Component");
    Py_CLEAR(componentMod);

    if (!componentType) {
        critical_log("[Python]: Cannot obtain reference to builtin type \"Component\"");
    }

    return componentType != NULL;
}

void Py3d_FinalizeComponentHelper() {
    if (componentType == NULL) return;

    Py_CLEAR(componentType);
}

PyTypeObject *Py3d_GetComponentType() {
    return componentType;
}

struct Py3dGameObject *Py3d_GetComponentOwner(PyObject *component) {
    if (component == NULL) return NULL;

    if (!Py3d_IsComponentSubclass(component)) return NULL;

    PyObject *ret = PyObject_CallMethod(component, "get_owner", NULL);
    if (ret == NULL) {
        handleException();
        return NULL;
    }

    if (!Py3dGameObject_Check(ret)) {
        Py_CLEAR(ret);
        critical_log("[Utility]: Py3dComponent has an owner that is not a GameObject");
        return NULL;
    }

    return (struct Py3dGameObject *) ret;
}

struct Py3dGameObject *Py3d_GetOwnerForComponent(PyObject *component) {
    if (component == NULL) {
        PyErr_SetString(PyExc_AssertionError, "Py3d_GetOwnerForComponent received NULL component");
        return NULL;
    }
    if (!Py3d_IsComponentSubclass(component)) {
        PyErr_SetString(PyExc_AssertionError, "Py3d_GetOwnerForComponent received component that is not a Component subclass");
        return NULL;
    }

    PyObject *ret = PyObject_CallMethod(component, "get_owner", NULL);
    if (ret == NULL) {
        return NULL;
    } else if (Py_IsNone(ret)) {
        Py_CLEAR(ret);
        PyErr_SetString(PyExc_ValueError, "Cannot get owner of detached component");
        return NULL;
    } else if (!Py3dGameObject_Check(ret)) {
        Py_CLEAR(ret);
        PyErr_SetString(PyExc_AssertionError, "Component is owned by something that isn't a game object");
        return NULL;
    }

    return (struct Py3dGameObject *) ret;
}

struct Py3dScene *Py3d_GetSceneForComponent(PyObject *component) {
    struct Py3dGameObject *owner = Py3d_GetComponentOwner(component);
    if (owner == NULL) return NULL;

    struct Py3dScene *ret = Py3d_GetSceneForGameObject(owner);
    Py_CLEAR(owner);

    return ret;
}

int Py3d_IsComponentSubclass(PyObject *obj) {
    if (obj == NULL) return 0;

    return PyObject_IsSubclass(obj, (PyObject *) componentType);
}

static PyObject *Py3d_Super(PyObject *obj) {
    if (obj == NULL) {
        PyErr_SetString(PyExc_AssertionError, "Py3d_Super called with NULL obj param");
        return NULL;
    }

    PyObject *builtinsMod = PyImport_ImportModule("builtins");
    if (builtinsMod == NULL) {
        critical_log("[Python]: Could not import builtins module");
        return NULL;
    }

    PyObject *ret = PyObject_CallMethod(builtinsMod, "super", "(O)", obj);
    Py_CLEAR(builtinsMod);
    return ret;
}

PyObject *Py3d_CallSuperMethod(PyObject *obj, const char* name, PyObject *args, PyObject *kwds) {
    PyObject *super = Py3d_Super(obj);
    if (super == NULL) return NULL;

    PyObject *superMethod = PyObject_GetAttrString(super, name);
    Py_CLEAR(super);
    if (superMethod == NULL) return NULL;

    PyObject *superMethodRet = PyObject_Call(superMethod, args, kwds);
    Py_CLEAR(superMethod);

    return superMethodRet;
}

int Py3d_CallSuperInit(PyObject *obj, PyObject *args, PyObject *kwds) {
    PyObject *super = Py3d_Super(obj);
    if (super == NULL) return -1;

    PyObject *superInit = PyObject_GetAttrString(super, "__init__");
    Py_CLEAR(super);
    if (superInit == NULL) return -1;

    PyObject *ret = PyObject_Call(superInit, args, kwds);
    Py_CLEAR(superInit);
    if (ret == NULL) return -1;

    Py_CLEAR(ret);
    return 0;
}

int Py3d_IsComponentEnabled(PyObject *obj) {
    if (!Py3d_IsComponentSubclass(obj)) {
        error_log("[Python]: Py3d_IsComponentEnabled was called with non component subclass");
        return 0;
    }

    PyObject *ret = PyObject_CallMethod(obj, "enabled", NULL);
    if (ret == NULL) {
        error_log("[Python]: Calling \"enabled\" on component subclass yield an exception");
        handleException();
        return 0;
    }

    const int result = Py_IsTrue(ret);
    Py_CLEAR(ret);

    return result;
}

int Py3d_IsComponentVisible(PyObject *obj) {
    if (!Py3d_IsComponentSubclass(obj)) {
        error_log("[Python]: Py3d_IsComponentVisible was called with non component subclass");
        return 0;
    }

    PyObject *ret = PyObject_CallMethod(obj, "visible", NULL);
    if (ret == NULL) {
        error_log("[Python]: Calling \"visible\" on component subclass yield an exception");
        handleException();
        return 0;
    }

    const int result = Py_IsTrue(ret);
    Py_CLEAR(ret);

    return result;
}