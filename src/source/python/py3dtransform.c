#include "python/py3dtransform.h"
#include "math/vector3.h"
#include "python/python_util.h"
#include "util.h"
#include "logger.h"

static PyObject *py3dTransformCtor = NULL;

static PyObject *makeTupleFromFloatArray(const float *src, int length) {
    PyObject *ret = PyTuple_New(length);
    if (ret == NULL) return NULL;

    for (int i = 0; i < length; ++i) {
        if (PyTuple_SetItem(ret, i, PyFloat_FromDouble(src[i])) == -1) {
            Py_CLEAR(ret);
            return NULL;
        }
    }

    return ret;
}

static bool unpackNumberTupleIntoFloatArray(PyObject *tuple, unsigned int required_elements, float *dst) {
    if (tuple == NULL || required_elements == 0 || dst == NULL) return false;

    if (PyTuple_Check(tuple) != 1) return false;

    Py_ssize_t tuple_len = PyTuple_Size(tuple);
    if (tuple_len < required_elements) return false;

    for (unsigned int i = 0; i < required_elements; ++i) {
        PyObject *curElement = PyTuple_GetItem(tuple, i);
        curElement = PyNumber_Float(curElement);
        if (curElement == NULL) return false;

        dst[i] = (float) PyFloat_AsDouble(curElement);
        Py_CLEAR(curElement);
    }

    return true;
}

static void refreshMatrixCaches(struct Py3dTransform *component) {
    if (component == NULL || component->matrixCacheDirty == false) return;

    float sMtx[16] = {0.0f};
    Mat4ScalingFA(sMtx, component->scale);

    float rMtx[16] = {0.0f};
    Mat4RotationQuaternionFA(rMtx, component->orientation);

    float tMtx[16] = {0.0f};
    Mat4TranslationFA(tMtx, component->position);

    float wMtx[16] = {0.0f};
    Mat4Mult(wMtx, sMtx, rMtx);
    Mat4Mult(wMtx, wMtx, tMtx);

    Mat4Copy(component->wMatrixCache, wMtx);

    Mat4Inverse(component->witMatrixCache, wMtx);
    Mat4Transpose(component->witMatrixCache, component->witMatrixCache);

    component->matrixCacheDirty = false;
}

static void refreshViewMatrixCache(struct Py3dTransform *component) {
    if (component == NULL || component->viewMatrixCacheDirty == false) return;

    float camTarget[3] = {0.0f, 0.0f, 1.0f};
    float camUp[3] = {0.0f, 1.0f, 0.0f};

    QuaternionVec3Rotation(camTarget, component->orientation, camTarget);
    Vec3Add(camTarget, component->position, camTarget);

    QuaternionVec3Rotation(camUp, component->orientation, camUp);

    Mat4LookAtLH(component->viewMatrixCache, component->position, camTarget, camUp);
    component->viewMatrixCacheDirty = false;
}

static PyObject *Py3dTransform_GetPosition(struct Py3dTransform *self, PyObject *args, PyObject *kwds) {
    struct Py3dVector3 *result = Py3dVector3_New();
    result->elements[0] = self->position[0];
    result->elements[1] = self->position[1];
    result->elements[2] = self->position[2];

    return (PyObject *) result;
}

static PyObject *Py3dTransform_Move(struct Py3dTransform *self, PyObject *args, PyObject *kwds) {
    struct Py3dVector3 *displacement = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dVector3_Type, &displacement) != 1) return NULL;

    self->position[0] += displacement->elements[0];
    self->position[1] += displacement->elements[1];
    self->position[2] += displacement->elements[2];
    self->matrixCacheDirty = true;
    self->viewMatrixCacheDirty = true;

    Py_RETURN_NONE;
}

static PyObject *Py3dTransform_SetPosition(struct Py3dTransform *self, PyObject *args, PyObject *kwds) {
    struct Py3dVector3 *newPosition = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dVector3_Type, &newPosition) != 1) return NULL;

    self->position[0] = newPosition->elements[0];
    self->position[1] = newPosition->elements[1];
    self->position[2] = newPosition->elements[2];
    self->matrixCacheDirty = true;
    self->viewMatrixCacheDirty = true;

    Py_RETURN_NONE;
}

static PyObject *Py3dTransform_GetOrientation(struct Py3dTransform *self, PyObject *args, PyObject *kwds) {
    PyObject *ret = makeTupleFromFloatArray(self->orientation, 4);
    if (ret == NULL) {
        ret = Py_None;
    }

    Py_INCREF(ret);
    return ret;
}

static PyObject *Py3dTransform_Rotate(struct Py3dTransform *self, PyObject *args, PyObject *kwds) {
    float temp[4] = {0.0f};
    if (!unpackNumberTupleIntoFloatArray(args, 4, temp)) {
        PyErr_SetString(PyExc_ValueError, "Transform.rotate requires 4 floats as arguments");
        return NULL;
    }

    QuaternionMult(self->orientation, temp, self->orientation);
    self->matrixCacheDirty = true;
    self->viewMatrixCacheDirty = true;

    Py_RETURN_NONE;
}

static PyObject *Py3dTransform_SetOrientation(struct Py3dTransform *self, PyObject *args, PyObject *kwds) {
    float temp[4] = {0.0f};
    if (!unpackNumberTupleIntoFloatArray(args, 4, temp)) {
        PyErr_SetString(PyExc_ValueError, "Transform.set_orientation requires 4 floats as arguments");
        return NULL;
    }

    QuaternionCopy(self->orientation, temp);
    self->matrixCacheDirty = true;
    self->viewMatrixCacheDirty = true;

    Py_RETURN_NONE;
}

static PyObject *Py3dTransform_GetScale(struct Py3dTransform *self, PyObject *args, PyObject *kwds) {
    struct Py3dVector3 *result = Py3dVector3_New();
    result->elements[0] = self->scale[0];
    result->elements[1] = self->scale[1];
    result->elements[2] = self->scale[2];

    return (PyObject *) result;
}

static PyObject *Py3dTransform_Stretch(struct Py3dTransform *self, PyObject *args, PyObject *kwds) {
    struct Py3dVector3 *factor = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dVector3_Type, &factor) != 1) return NULL;

    self->scale[0] *= factor->elements[0];
    self->scale[1] *= factor->elements[1];
    self->scale[2] *= factor->elements[2];
    self->matrixCacheDirty = true;
    self->viewMatrixCacheDirty = true;

    Py_RETURN_NONE;
}

static PyObject *Py3dTransform_SetScale(struct Py3dTransform *self, PyObject *args, PyObject *kwds) {
    struct Py3dVector3 *newScale = NULL;
    if (PyArg_ParseTuple(args, "O!", &Py3dVector3_Type, &newScale) != 1) return NULL;

    self->scale[0] = newScale->elements[0];
    self->scale[1] = newScale->elements[1];
    self->scale[2] = newScale->elements[2];
    self->matrixCacheDirty = true;
    self->viewMatrixCacheDirty = true;

    Py_RETURN_NONE;
}

static PyMethodDef Py3dTransform_Methods[] = {
    {"get_position", (PyCFunction) Py3dTransform_GetPosition, METH_NOARGS, "Get position as tuple"},
    {"move", (PyCFunction) Py3dTransform_Move, METH_VARARGS, "Move the transform by displacement value"},
    {"set_position", (PyCFunction) Py3dTransform_SetPosition, METH_VARARGS, "Set the position by absolute value"},
    {"get_orientation", (PyCFunction) Py3dTransform_GetOrientation, METH_NOARGS, "Get orientation as tuple"},
    {"rotate", (PyCFunction) Py3dTransform_Rotate, METH_VARARGS, "Rotate the transform by displacement value"},
    {"set_orientation", (PyCFunction) Py3dTransform_SetOrientation, METH_VARARGS, "Set the orientation by absolute value"},
    {"get_scale", (PyCFunction) Py3dTransform_GetScale, METH_NOARGS, "Get scale as tuple"},
    {"stretch", (PyCFunction) Py3dTransform_Stretch, METH_VARARGS, "Stretch the transform by factor value"},
    {"set_scale", (PyCFunction) Py3dTransform_SetScale, METH_VARARGS, "Set the scale by absolute value"},
    {NULL},
};

static int Py3dTransform_Init(struct Py3dTransform *self, PyObject *args, PyObject *kwds) {
    if (Py3dComponent_Type.tp_init((PyObject *) self, args, kwds) == -1) return -1;

    Vec3Fill(self->position, 0.0f);
    QuaternionIdentity(self->orientation);
    Vec3Fill(self->scale, 1.0f);
    self->matrixCacheDirty = true;
    Mat4Identity(self->wMatrixCache);
    Mat4Identity(self->witMatrixCache);
    Mat4Identity(self->viewMatrixCache);

    return 0;
}

static PyTypeObject Py3dTransform_Type = {
    PyVarObject_HEAD_INIT(NULL, 0)
    .tp_name = "py3dengine.Transform",
    .tp_doc = "Stores a Games Object's spatial information",
    .tp_basicsize = sizeof(struct Py3dTransform),
    .tp_itemsize = 0,
    .tp_flags = Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE,
    .tp_init = (initproc) Py3dTransform_Init,
    .tp_methods = Py3dTransform_Methods
};

bool PyInit_Py3dTransform(PyObject *module) {
    Py3dTransform_Type.tp_base = &Py3dComponent_Type;
    if (PyType_Ready(&Py3dTransform_Type) < 0) return false;

    if (PyModule_AddObject(module, "Transform", (PyObject *) &Py3dTransform_Type) < 0) return false;

    Py_INCREF(&Py3dTransform_Type);

    return true;
}

bool Py3dTransform_FindCtor(PyObject *module) {
    if (PyObject_HasAttrString(module, "Transform") == 0) {
        critical_log("%s", "[Python]: Py3dTransform has not been initialized properly");

        return false;
    }

    py3dTransformCtor = PyObject_GetAttrString(module, "Transform");
    Py_INCREF(py3dTransformCtor);

    return true;
}

void Py3dTransform_FinalizeCtor() {
    Py_CLEAR(py3dTransformCtor);
}

struct Py3dTransform *Py3dTransform_New() {
    if (py3dTransformCtor == NULL) {
        critical_log("%s", "[Python]: Py3dTransform has not been initialized properly");

        return NULL;
    }

    PyObject *py3dtransform = PyObject_CallNoArgs(py3dTransformCtor);
    if (py3dtransform == NULL) {
        critical_log("%s", "[Python]: Failed to allocate Transform in python interpreter");
        handleException();

        return NULL;
    }

    return (struct Py3dTransform *) py3dtransform;
}

float *getTransformWorldMtx(struct Py3dTransform *component) {
    if (component == NULL) return NULL;

    refreshMatrixCaches(component);

    return component->wMatrixCache;
}

float *getTransformWITMtx(struct Py3dTransform *component) {
    if (component == NULL) return NULL;

    refreshMatrixCaches(component);

    return component->witMatrixCache;
}

float *getTransformViewMtx(struct Py3dTransform *component) {
    if (component == NULL) return NULL;

    refreshViewMatrixCache(component);

    return component->viewMatrixCache;
}
