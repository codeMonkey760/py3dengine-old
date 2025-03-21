#include "python/py3drigidbody.h"
#include "python/python_util.h"
#include "logger.h"
#include "python/py3dgameobject.h"
#include "python/py3dscene.h"
#include "physics/collision.h"

struct Py3dRigidBody {
    struct Py3dComponent base;
    dBodyID dynamicsBody;
    dGeomID geomId;
    int isTrigger; //generate collision messages only when true, don't physically collide
};

static PyMethodDef Py3dRigidBody_Methods[] = {
    {"set_shape", (PyCFunction) Py3dRigidBody_SetShape, METH_VARARGS, "Set collision shape"},
    {"parse", (PyCFunction) Py3dRigidBody_Parse, METH_VARARGS, "Handle parse messages"},
    {"update", (PyCFunction) Py3dRigidBody_Update, METH_VARARGS, "Update event handler"},
    {"is_trigger", (PyCFunction) Py3dRigidBody_IsTrigger, METH_NOARGS, "Determine if RigidBodyComponent is trigger"},
    {"make_trigger", (PyCFunction) Py3dRigidBody_MakeTrigger, METH_VARARGS, "Make RigidBodyComponent a trigger or not"},
    {NULL}
};

static void allocBody(struct Py3dRigidBody *self, struct PhysicsSpace *space) {
    if (self->dynamicsBody != NULL || space == NULL) return;

    self->dynamicsBody = createDynamicsBody(space);
}

static void deleteGeom(struct Py3dRigidBody *self) {
    if (self->geomId == NULL) return;


    dGeomSetBody(self->geomId, 0);
    dGeomDestroy(self->geomId);
    self->geomId = NULL;
}

static int Py3dRigidBody_Init(struct Py3dRigidBody *self, PyObject *args, PyObject *kwds) {
    if (Py3dComponent_Type.tp_init((PyObject *) self, args, kwds) == -1) return -1;

    self->dynamicsBody = 0;
    self->geomId = 0;
    self->isTrigger = 0;

    return 0;
}

static void Py3dRigidBody_Dealloc(struct Py3dRigidBody *self) {
    deleteGeom(self);
    destroyDynamicsBody(self->dynamicsBody);
    self->dynamicsBody = NULL;

    Py3dComponent_Dealloc((struct Py3dComponent *) self);
}

PyTypeObject Py3dRigidBody_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py3dengineEXT.RigidBodyComponent",
    .tp_doc = "Gives GameObjects a presence in the physics engine",
    .tp_basicsize = sizeof(struct Py3dRigidBody),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_init = (initproc) Py3dRigidBody_Init,
    .tp_methods = Py3dRigidBody_Methods,
    .tp_dealloc = (destructor) Py3dRigidBody_Dealloc
};

int PyInit_Py3dRigidBody(PyObject *module) {
    Py3dRigidBody_Type.tp_base = &Py3dComponent_Type;
    if (PyType_Ready(&Py3dRigidBody_Type) < 0) return 0;

    if (PyModule_AddObject(module, "RigidBodyComponent", (PyObject *) &Py3dRigidBody_Type) < 0) return 0;

    Py_INCREF(&Py3dRigidBody_Type);

    return 1;
}

struct Py3dRigidBody *Py3dRigidBody_New() {
    PyObject *py3dengine = PyImport_ImportModule("py3dengine");
    if (py3dengine == NULL) {
        critical_log("[RigidBodyComponent]: Failed to import py3dengine module");
        return NULL;
    }

    PyObject *ctor = PyObject_GetAttrString(py3dengine, "RigidBodyComponent");
    Py_CLEAR(py3dengine);

    if (ctor == NULL) {
        critical_log("[RigidBodyComponent]: Failed to find ctor in py3dengine module");
        return NULL;
    }

    PyObject *args = Py_BuildValue("()");
    PyObject *ret = PyObject_Call(ctor, args, NULL);

    Py_CLEAR(args);
    Py_CLEAR(ctor);

    if (ret == NULL) {
        critical_log("[RigidBodyComponent]: Failed to instantiate new object");
        return NULL;
    } else if (!Py3dRigidBody_Check(ret)) {
        critical_log("[RigidBodyComponent]: Newly instantiated object failed type check");
        Py_CLEAR(ret);
        PyErr_SetString(PyExc_TypeError, "Newly instantiated object failed type check");
        return NULL;
    }

    return (struct Py3dRigidBody *) ret;
}

int Py3dRigidBody_Check(PyObject *obj) {
    int ret = PyObject_IsInstance(obj, (PyObject *) &Py3dRigidBody_Type);
    if (ret == -1) {
        handleException();
        return 0;
    }

    return ret;
}

static int convertPyObjectToReal(PyObject *obj, dReal *result) {
    if (obj == NULL || result == NULL) return 0;

    obj = PyNumber_Float(obj);
    if (obj == NULL) {
        PyErr_Clear();
        return 0;
    }

    (*result) = (dReal) PyFloat_AsDouble(obj);
    Py_CLEAR(obj);

    return 1;
}

// if this function returns false, the global exception state is set
static int fetchNumericArguments(PyObject *args, Py_ssize_t numArgs, dReal *dst) {
    if (args == NULL || numArgs < 1 || dst == NULL) {
        critical_log("[Py3dRigidBody]: fetchNumericArguments was not supplied with valid params");
        PyErr_SetString(PyExc_AssertionError, "Cannot capture arguments with empty params");
        return 0;
    }

    // index starts at one, arg 0 is assumed to be the shape name
    for (Py_ssize_t i = 1; i <= numArgs; ++i) {
        if (!convertPyObjectToReal(Py3d_GetTypeFromTuple(args, i, NULL), &dst[i-1])) {
            PyErr_Format(PyExc_ValueError, "Argument %d must be a number", i);
            return 0;
        }
    }

    return 1;
}

static struct PhysicsSpace *getOwnersPhysicsSpace(struct Py3dGameObject *owner) {
    struct Py3dScene *scene = Py3dGameObject_GetScene(owner);
    if (Py3dScene_Check((PyObject *) scene) != 1) {
        PyErr_SetString(PyExc_ValueError, "Py3dRigidBody's owner is not attached to a scene");
        return NULL;
    }

    struct PhysicsSpace *space = scene->space;
    Py_CLEAR(scene); // TODO: THIS IS A DEFECT!!!! Py_CLEAR can dealloc python objects! If this happens here
                     // then space is now a dangling pointer and will result in a use after free!!!!
    if (space == NULL) {
        PyErr_SetString(PyExc_ValueError, "Scene has no physics space");
    }

    return space;
}

PyObject *Py3dRigidBody_SetShape(struct Py3dRigidBody *self, PyObject *args, PyObject *kwds) {
    PyObject *shapeNameObj = Py3d_GetTypeFromTuple(args, 0, &PyUnicode_Type);
    if (shapeNameObj == NULL) {
        handleException();
        PyErr_SetString(PyExc_ValueError, "Argument 0 must be a string with a valid collision shape name");
        return NULL;
    }
    Py_INCREF(shapeNameObj);

    dGeomID newGeom = NULL;
    dReal dimensions[3] = {0.0f, 0.0f, 0.0f};
    const char *shapeName = PyUnicode_AsUTF8(shapeNameObj);

    if (strcmp(shapeName, "SPHERE") == 0) {
        if (fetchNumericArguments(args, 1, dimensions) == 0) return NULL;
        newGeom = dCreateSphere(NULL, dimensions[0]);
    } else if (strcmp(shapeName, "CYLINDER") == 0) {
        if (fetchNumericArguments(args, 2, dimensions) == 0) return NULL;
        newGeom = dCreateCylinder(NULL, dimensions[0], dimensions[1]);
    } else if (strcmp(shapeName, "CAPSULE") == 0) {
        if (fetchNumericArguments(args, 2, dimensions) == 0) return NULL;
        newGeom = dCreateCapsule(NULL, dimensions[0], dimensions[1]);
    } else if (strcmp(shapeName, "BOX") == 0) {
        if (fetchNumericArguments(args, 3, dimensions) == 0) return NULL;
        newGeom = dCreateBox(NULL, dimensions[0], dimensions[1], dimensions[2]);
    } else {
        PyErr_SetString(PyExc_ValueError, "Argument 0 must be a string with a valid collision shape name");
        return NULL;
    }

    shapeName = NULL;
    Py_CLEAR(shapeNameObj);

    struct Py3dGameObject *owner = Py3d_GetComponentOwner((struct Py3dComponent *) self);
    if (owner == NULL) return NULL;

    struct PhysicsSpace *space = getOwnersPhysicsSpace(owner);
    if (space == NULL) {
        PyErr_SetString(PyExc_ValueError, "Could not obtain owners physics space");
        critical_log("[RigidBodyComponent]: Could not attach new ode geom to scene physics space");
        dGeomDestroy(newGeom);
        newGeom = NULL;
        Py_CLEAR(owner);
        return NULL;
    }

    dGeomSetData(newGeom, self);
    allocBody(self, space);
    dGeomSetBody(newGeom, self->dynamicsBody);
    addGeomToWorldSpace(space, newGeom);
    //deleteGeom(self);
    self->geomId = newGeom;
    Py_CLEAR(owner);

    Py_RETURN_NONE;
}

PyObject *Py3dRigidBody_Parse(struct Py3dRigidBody *self, PyObject *args, PyObject *kwds) {
    PyObject *superParseRet = Py3dComponent_Parse((struct Py3dComponent *) self, args, kwds);
    if (superParseRet == NULL) return NULL;
    Py_CLEAR(superParseRet);

    PyObject *parseDataDict = NULL, *py3dResourceManager = NULL;
    if (PyArg_ParseTuple(args, "O!O", &PyDict_Type, &parseDataDict, &py3dResourceManager) != 1) return NULL;

    PyObject *isTriggerObj = PyDict_GetItemString(parseDataDict, "is_trigger");
    if (isTriggerObj == NULL) {
        PyErr_SetString(PyExc_KeyError, "Parse data for \"RigidBodyComponent\" must include a boolean called \"is_trigger\"");
        return NULL;
    }
    if (Py_IsTrue(isTriggerObj)) {
        self->isTrigger = 1;
    } else if (Py_IsFalse(isTriggerObj)) {
        self->isTrigger = 0;
    } else {
        PyErr_SetString(PyExc_KeyError, "Parse data for \"RigidBodyComponent\" must include a boolean called \"is_trigger\"");
        return NULL;
    }

    PyObject *shapeName = PyDict_GetItemString(parseDataDict, "shape");
    if (shapeName == NULL) {
        PyErr_SetString(PyExc_KeyError, "Parse data for \"RigidBodyComponent\" must include a string called \"shape\"");
        return NULL;
    }
    shapeName = PyObject_Str(shapeName);
    if (shapeName == NULL) return NULL;

    PyObject *argsList = PyDict_GetItemString(parseDataDict, "args");
    if (!PyList_Check(argsList)) {
        PyErr_SetString(PyExc_KeyError, "Parse data for \"RigidBodyComponent\" must include a list called \"args\"");
        Py_CLEAR(shapeName);
        return NULL;
    }
    Py_INCREF(argsList);

    Py_ssize_t numArgs = PyList_Size(argsList);
    PyObject *setShapeArgsTuple = PyTuple_New(numArgs + 1);
    PyTuple_SetItem(setShapeArgsTuple, 0, shapeName); //PyTuple_SetItem steals this reference, so don't clear shapeName
    for (Py_ssize_t i = 0; i < numArgs; ++i) {
        //Without Py_NewRef, PyTuple_SetItem would steal a reference from argsList
        PyObject *curArg = Py_NewRef(PyList_GetItem(argsList, i));
        PyTuple_SetItem(setShapeArgsTuple, i+1, curArg);
    }
    Py_CLEAR(argsList);

    PyObject *setShapeCallable = PyObject_GetAttrString((PyObject *) self, "set_shape");
    PyObject *setShapeRet = PyObject_Call(setShapeCallable, setShapeArgsTuple, NULL);

    Py_CLEAR(setShapeCallable);
    Py_CLEAR(setShapeArgsTuple);
    if (setShapeRet == NULL) return NULL;

    Py_CLEAR(setShapeRet);
    Py_RETURN_NONE;
}

PyObject *Py3dRigidBody_Update(struct Py3dRigidBody *self, PyObject *args, PyObject *kwds) {
    PyObject *superUpdateRet = Py3dComponent_Update((struct Py3dComponent *) self, args, kwds);
    if (superUpdateRet == NULL) return NULL;
    Py_CLEAR(superUpdateRet);

    struct Py3dGameObject *owner = Py3d_GetComponentOwner((struct Py3dComponent *) self);

    const float *pos = Py3dGameObject_GetPositionFA(owner);
    dBodySetPosition(self->dynamicsBody, pos[0], pos[1], pos[2]);

    const float *orientation = Py3dGameObject_GetOrientationFA(owner);
    dQuaternion odeOrientation;
    odeOrientation[0] = orientation[3];
    odeOrientation[1] = orientation[0];
    odeOrientation[2] = orientation[1];
    odeOrientation[3] = orientation[2];

    dBodySetQuaternion(self->dynamicsBody, odeOrientation);

    Py_CLEAR(owner);

    Py_RETURN_NONE;
}

int Py3dRigidBody_IsTriggerInt(struct Py3dRigidBody *self) {
    return self->isTrigger;
}

PyObject *Py3dRigidBody_IsTrigger(struct Py3dRigidBody *self, PyObject *args, PyObject *kwds) {
    return PyBool_FromLong(Py3dRigidBody_IsTriggerInt(self));
}

void Py3dRigidBody_MakeTriggerInt(struct Py3dRigidBody *self, int is_trigger) {
    self->isTrigger = is_trigger;
}

PyObject *Py3dRigidBody_MakeTrigger(struct Py3dRigidBody *self, PyObject *args, PyObject *kwds) {
    int make_trigger = 0;
    if (PyArg_ParseTuple(args, "p", &make_trigger) != 1) return NULL;

    Py3dRigidBody_MakeTriggerInt(self, make_trigger);

    Py_RETURN_NONE;
}
