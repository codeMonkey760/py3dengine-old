#include <stdlib.h>
#define PY_SSIZE_T_CLEAN
#include <Python.h>
#include <structmember.h>

#include "custom_string.h"
#include "logger.h"
#include "game_object.h"
#include "components/base_component.h"
#include "python/python_util.h"
#include "python/py3dcomponent.h"
#include "python/py3dtransform.h"

struct ComponentListNode {
    struct BaseComponent *component;
    struct Py3dComponent *pyComponent;
    struct ComponentListNode *next;
};

struct ChildListNode {
    struct GameObject *child;
    struct ChildListNode *next;
};

struct Py3dGameObject {
    PyObject_HEAD
    struct GameObject *gameObject;
};

static PyObject *py3dGameObjectCtor = NULL;

static void Py3dGameObject_Dealloc(struct Py3dGameObject *self) {
    Py_TYPE(self)->tp_free((PyObject *) self);
}

static int Py3dGameObject_Init(struct Py3dGameObject *self, PyObject *args, PyObject *kwds) {
    self->gameObject = NULL;

    return 0;
}

static PyObject *Py3dGameObject_GetName(struct Py3dGameObject *self, PyObject *Py_UNUSED(ignored)) {
    if (self == NULL || self->gameObject == NULL) {
        PyErr_SetString(PyExc_AssertionError, "Python GameObject is detached from C GameObject");
        return NULL;
    }

    return PyUnicode_FromString(getChars(getGameObjectName(self->gameObject)));
}

PyObject *Py3dGameObject_GetTransform(struct Py3dGameObject *self, PyObject *Py_UNUSED(ignored)) {
    if (self->gameObject == NULL) {
        PyErr_SetString(PyExc_AssertionError, "Python GameObject is detached from C GameObject");
        return NULL;
    }

    if (self->gameObject->transform == NULL) {
        // Should I set an exception here instead?
        Py_RETURN_NONE;
    }

    if (Py3dTransform_Check((PyObject *) self->gameObject->transform) == 0) {
        PyErr_SetString(PyExc_AssertionError, "Python GameObject's transform is not of type Py3dTransform");
        return NULL;
    }

    Py_INCREF(self->gameObject->transform);
    return (PyObject *) self->gameObject->transform;
}

PyMethodDef Py3dGameObject_Methods[] = {
    {"get_name", (PyCFunction) Py3dGameObject_GetName, METH_NOARGS, "Get Game Object's name"},
    {"get_transform", (PyCFunction) Py3dGameObject_GetTransform, METH_NOARGS, "Get Game Object's transform"},
    {NULL}
};

PyTypeObject Py3dGameObjectType = {
    PyObject_HEAD_INIT(NULL)
    .tp_name = "py3dengine.GameObject",
    .tp_basicsize = sizeof(struct Py3dGameObject),
    .tp_dealloc = (destructor) Py3dGameObject_Dealloc,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_doc = "Class for manipulating Game Objects",
    .tp_methods = Py3dGameObject_Methods,
    .tp_init = (initproc) Py3dGameObject_Init,
    .tp_new = PyType_GenericNew
};

static PyObject *Py3dGameObject_New() {
    if (py3dGameObjectCtor == NULL) {
        critical_log("%s", "[Python]: Py3dGameObject has not been initialized properly");

        return NULL;
    }

    PyObject *py3dGameObject = PyObject_Call(py3dGameObjectCtor, PyTuple_New(0), NULL);
    if (py3dGameObject == NULL) {
        critical_log("%s", "[Python]: Failed to allocate GameObject in python interpreter");
        handleException();

        return NULL;
    }

    return py3dGameObject;
}

static void allocComponentListNode(struct ComponentListNode **listNodePtr) {
    if (listNodePtr == NULL || (*listNodePtr) != NULL) return;

    struct ComponentListNode *newNode = calloc(1, sizeof(struct ComponentListNode));
    if (newNode == NULL) return;

    newNode->component = NULL;
    newNode->pyComponent = NULL;
    newNode->next = NULL;

    (*listNodePtr) = newNode;
    newNode = NULL;
}

static void deleteComponentListNode(struct ComponentListNode **listNodePtr) {
    if (listNodePtr == NULL || (*listNodePtr) == NULL) return;

    struct ComponentListNode *node = (*listNodePtr);
    deleteComponent(&node->component);
    Py_CLEAR(node->pyComponent);

    deleteComponentListNode(&node->next);

    free(node);
    node = NULL;
    (*listNodePtr) = NULL;
}

static PyObject *getNameFromPythonComponent(PyObject *pyComponent) {
    if (pyComponent == NULL) return NULL;

    if (PyObject_HasAttrString(pyComponent, "get_name") == 0) {
        error_log("%s", "[GameObject]: Component must have a \"get_name\" property");
        return NULL;
    }

    PyObject *get_name = PyObject_GetAttrString(pyComponent, "get_name");
    if (PyCallable_Check(get_name) == 0) {
        error_log("%s", "[GameObject]: Component's \"get_name\" property must be callable");
        return NULL;
    }

    PyObject *name = PyObject_CallNoArgs(get_name);
    if (name == NULL) {
        error_log("%s", "[GameObject]: Python raised exception while querying for component name");
        handleException();
        return NULL;
    }
    if (Py_IsNone(name)) {
        error_log("%s", "[GameObject]: Component must have a name");
        return NULL;
    }

    return name;
}

static bool componentNamesAreEqual(struct ComponentListNode *listNode, const char *componentName) {
    if (listNode == NULL || componentName == NULL) return false;

    unsigned int componentNameLen = strlen(componentName);
    if (componentNameLen == 0) return false;

    if (
        (listNode->component == NULL && listNode->pyComponent == NULL) ||
        (listNode->component != NULL && listNode->pyComponent != NULL)
    ) {
        critical_log("%s", "[GameObject]: Component list node failed sanity check and is not well formed.");
        return false;
    }

    if (listNode->component != NULL) {
        return stringEqualsCStr(getComponentName(listNode->component), componentName);
    }

    PyObject *nodeName = getNameFromPythonComponent((PyObject *) listNode->pyComponent);
    if (nodeName == NULL) {
        critical_log("%s", "Could not query name for already attached component");
        return false;
    }
    const char *nodeNameCStr = PyUnicode_AsUTF8(nodeName);
    unsigned int nodeNameLen = strlen(nodeNameCStr);
    if (nodeNameLen == 0) return false;

    unsigned int longestLen = (nodeNameLen > componentNameLen) ? nodeNameLen : componentNameLen;

    return strncmp(nodeNameCStr, componentName, longestLen) == 0;
}

static void allocChildListNode(struct ChildListNode **listNodePtr) {
    if (listNodePtr == NULL || (*listNodePtr) != NULL) return;

    struct ChildListNode *newNode = calloc(1, sizeof(struct ChildListNode));
    if (newNode == NULL) return;

    newNode->child = NULL;
    newNode->next = NULL;

    (*listNodePtr) = newNode;
    newNode = NULL;
}

static void deleteChildListNode(struct ChildListNode **listNodePtr) {
    if (listNodePtr == NULL || (*listNodePtr) == NULL) return;

    struct ChildListNode *node = (*listNodePtr);
    deleteGameObject(&node->child);

    deleteChildListNode(&node->next);

    free(node);
    node = NULL;
    (*listNodePtr) = NULL;
}

static void initializePyGameObject(struct GameObject *gameObject) {
    if (gameObject == NULL || gameObject->pyGameObject != NULL) return;

    struct Py3dGameObject *newPyGameObject = (struct Py3dGameObject *) Py3dGameObject_New();
    if (newPyGameObject == NULL) {
        critical_log("%s", "[GameObject]: Could not instantiate new PyGameObject");

        return;
    }

    gameObject->pyGameObject = newPyGameObject;
    newPyGameObject->gameObject = gameObject;
}

static void updatePythonComponent(struct Py3dComponent *component, float dt) {
    if (component == NULL) return;

    if (PyObject_HasAttrString((PyObject *) component, "update") != 1) return;

    PyObject *pyUpdate = PyObject_GetAttrString((PyObject *) component, "update");
    if (PyCallable_Check(pyUpdate) != 1) {
        Py_CLEAR(pyUpdate);
        return;
    }

    PyObject *componentName = getNameFromPythonComponent((PyObject *) component);
    const char *componentNameCStr = (componentName != NULL) ? PyUnicode_AsUTF8(componentName) : "NAME_ERROR";

    PyObject *dtArg = PyFloat_FromDouble(dt);
    PyObject *pyUpdateRet = PyObject_CallOneArg(pyUpdate, dtArg);
    if (pyUpdateRet == NULL) {
        error_log("[GameObject]: Python component named \"%s\" threw exception while updating", componentNameCStr);
        handleException();
    } else if (!Py_IsNone(pyUpdateRet)) {
        warning_log("%s", "[GameObject]: Python component named \"%s\" returned something while updating, which is weird", componentNameCStr);
    }

    Py_CLEAR(componentName);
    Py_CLEAR(dtArg);
    Py_CLEAR(pyUpdateRet);
    Py_CLEAR(pyUpdate);
}

bool PyInit_Py3dGameObject(PyObject *module) {
    if (PyType_Ready(&Py3dGameObjectType) == -1) return false;

    if (PyModule_AddObject(module, "GameObject", (PyObject *) &Py3dGameObjectType) == -1) return false;

    Py_INCREF(&Py3dGameObjectType);

    return true;
}

bool findPyGameObjectCtor(PyObject *module) {
    if (PyObject_HasAttrString(module, "GameObject") == 0) {
        critical_log("%s", "[Python]: Py3dGameObject has not been initialized properly");

        return false;
    }

    py3dGameObjectCtor = PyObject_GetAttrString(module, "GameObject");

    return true;
}

void finalizePyGameObjectCtor() {
    Py_CLEAR(py3dGameObjectCtor);
}

int Py3dGameObject_Check(PyObject *obj) {
    return PyObject_IsInstance(obj, (PyObject *) &Py3dGameObjectType);
}

void allocGameObject(struct GameObject **gameObjectPtr) {
    if (gameObjectPtr == NULL || (*gameObjectPtr) != NULL) return;

    struct GameObject *gameObject = NULL;
    gameObject = calloc(1, sizeof(struct GameObject));
    if (gameObject == NULL) return;

    gameObject->components = NULL;
    gameObject->children = NULL;
    gameObject->parent = NULL;
    gameObject->name = NULL;
    gameObject->transform = Py3dTransform_New();
    gameObject->pyGameObject = NULL;
    initializePyGameObject(gameObject);

    (*gameObjectPtr) = gameObject;
    gameObject = NULL;
}

void deleteGameObject(struct GameObject **gameObjectPtr) {
    if (gameObjectPtr == NULL || (*gameObjectPtr) == NULL) return;

    struct GameObject *gameObject = (*gameObjectPtr);
    deleteComponentListNode(&gameObject->components);
    deleteChildListNode(&gameObject->children);
    gameObject->parent = NULL;

    deleteString(&gameObject->name);
    Py_CLEAR(gameObject->transform);
    Py_CLEAR(gameObject->pyGameObject);

    free(gameObject);
    gameObject = NULL;
    (*gameObjectPtr) = NULL;
}

void updateGameObject(struct GameObject *gameObject, float dt) {
    if (gameObject == NULL) return;

    struct ComponentListNode *curNode = gameObject->components;
    while (curNode != NULL) {
        struct BaseComponent *curComponent = curNode->component;
        struct Py3dComponent *curPyComponent = curNode->pyComponent;
        if (curComponent != NULL && curComponent->update != NULL) {
            curComponent->update(curComponent, dt);
        } else if (curPyComponent != NULL) {
            updatePythonComponent(curPyComponent, dt);
        }

        curNode = curNode->next;
    }

    struct ChildListNode *curChild = gameObject->children;
    while (curChild != NULL) {
        updateGameObject(curChild->child, dt);

        curChild = curChild->next;
    }
}

void renderGameObject(struct GameObject *gameObject, struct RenderingContext *renderingContext) {
    if (gameObject == NULL || renderingContext == NULL) return;

    struct ComponentListNode *curNode = gameObject->components;
    while (curNode != NULL) {
        struct BaseComponent *curComponent = curNode->component;
        if (curComponent != NULL && curComponent->render != NULL) {
            curComponent->render(curComponent, renderingContext);
        }

        curNode = curNode->next;
    }

    struct ChildListNode *curChild = gameObject->children;
    while (curChild != NULL) {
        renderGameObject(curChild->child, renderingContext);

        curChild = curChild->next;
    }
}

void attachChild(struct GameObject *parent, struct GameObject *newChild) {
    if (parent == NULL || newChild == NULL) return;

    struct String *newChildName = getGameObjectName(newChild);
    if (newChildName == NULL) {
        error_log("%s", "Cannot attach child game object unless it has a name");
        return;
    }

    struct ChildListNode *prevNode = NULL, *curNode = parent->children;
    while (curNode != NULL) {
        if (stringEquals(getGameObjectName(curNode->child), newChildName)) {
            error_log("%s", "Cannot attach child game object unless its name is unique");
            return;
        }

        prevNode = curNode;
        curNode = curNode->next;
    }

    struct ChildListNode *newNode = NULL;
    allocChildListNode(&newNode);
    if (newNode == NULL) return;
    newNode->child = newChild;

    if (prevNode == NULL) {
        parent->children = newNode;
    } else {
        prevNode->next = newNode;
    }
    newChild->parent = parent;
}

void removeChild(struct GameObject *gameObject, struct GameObject *target) {
    critical_log("%s", "GameObject::removeChild is not yet implemented");

    // TODO: implement GameObject::removeChild
}

void removeChildByName(struct GameObject *gameObject, const char* name) {
    critical_log("%s", "GameObject::removeChildByName is not yet implemented");

    // TODO: implement GameObject::removeChildByName
}

struct GameObject *findGameObjectByName(struct GameObject *gameObject, const char *name) {
    if (gameObject == NULL || name == NULL) return NULL;

    if (stringEqualsCStr(gameObject->name, name)) {
        return gameObject;
    }

    struct ChildListNode *curNode = gameObject->children;
    struct GameObject *ret = NULL;
    while (curNode != NULL) {
        ret = findGameObjectByName(curNode->child, name);
        if (ret != NULL) {
            return ret;
        }

        curNode = curNode->next;
    }

    return ret;
}

void attachComponent(struct GameObject *gameObject, struct BaseComponent *newComponent) {
    if (gameObject == NULL || newComponent == NULL) return;

    struct String *newComponentName = getComponentName(newComponent);
    if (newComponentName == NULL) {
        error_log("%s", "Cannot attach component to game object unless it has a name");
        return;
    }

    struct ComponentListNode *prevNode = NULL, *curNode = gameObject->components;
    while (curNode != NULL) {
        if (stringEquals(getComponentName(curNode->component), newComponentName)) {
            error_log("%s", "Cannot attach component to game object unless its name is unique");
            return;
        }

        prevNode = curNode;
        curNode = curNode->next;
    }

    struct ComponentListNode *newNode = NULL;
    allocComponentListNode(&newNode);
    if (newNode == NULL) return;

    newNode->component = newComponent;

    if (prevNode == NULL) {
        gameObject->components = newNode;
    } else {
        prevNode->next = newNode;
    }
    newComponent->_owner = gameObject;
}

void attachPyComponent(struct GameObject *gameObject, struct Py3dComponent *newPyComponent) {
    if (gameObject == NULL || newPyComponent == NULL) return;

    PyObject *pyComponentName = getNameFromPythonComponent((PyObject *) newPyComponent);
    if (pyComponentName == NULL) {
        error_log("%s", "[GameObject]: Cannot attach component unless its name can be queried");
        return;
    }
    const char *pyComponentNameAsCStr = PyUnicode_AsUTF8(pyComponentName);

    struct ComponentListNode *prevNode = NULL, *curNode = gameObject->components;
    while (curNode != NULL) {
        if (componentNamesAreEqual(curNode, pyComponentNameAsCStr)) {
            error_log("[GameObject]: Cannot attach python component named \"%s\". Component names must be unique");
            return;
        }

        prevNode = curNode;
        curNode = curNode->next;
    }

    struct ComponentListNode *newNode = NULL;
    allocComponentListNode(&newNode);
    if (newNode == NULL) return;

    newNode->pyComponent = newPyComponent;
    if (prevNode == NULL) {
        gameObject->components = newNode;
    } else {
        prevNode->next = newNode;
    }
    newPyComponent->owner = gameObject;
}

struct BaseComponent *getGameObjectComponentByType(struct GameObject *gameObject, const char *typeName) {
    if (gameObject == NULL || typeName == NULL) return NULL;

    struct ComponentListNode *curNode = gameObject->components;
    while (curNode != NULL) {
        if (stringEqualsCStr(getComponentTypeName(curNode->component), typeName)) {
            return curNode->component;
        }

        curNode = curNode->next;
    }

    return NULL;
}

size_t getGameObjectComponentsLength(struct GameObject *gameObject) {
    if (gameObject == NULL) return -1;

    size_t count = 0;
    struct ComponentListNode *curNode = gameObject->components;
    while (curNode != NULL) {
        count++;
        curNode = curNode->next;
    }

    return count;
}

struct Py3dComponent *getGameObjectComponentByIndex(struct GameObject *gameObject, size_t index) {
    if (gameObject == NULL) return NULL;

    size_t count = 0;
    struct ComponentListNode *curNode = gameObject->components;
    while (curNode != NULL) {
        if (count == index) break;

        curNode = curNode->next;
    }

    if (curNode == NULL) return NULL;

    if (curNode->pyComponent == NULL) return NULL;

    return curNode->pyComponent;
}

struct String *getGameObjectName(struct GameObject *gameObject) {
    if (gameObject == NULL) return NULL;

    return gameObject->name;
}

void setGameObjectName(struct GameObject *gameObject, const char *newName) {
    if (gameObject == NULL) return;

    if (gameObject->name == NULL) {
        allocString(&(gameObject->name), newName);
    } else {
        setChars(gameObject->name, newName);
    }
}

struct Py3dTransform *getGameObjectTransform(struct GameObject *gameObject) {
    if (gameObject == NULL) return NULL;

    return gameObject->transform;
}
