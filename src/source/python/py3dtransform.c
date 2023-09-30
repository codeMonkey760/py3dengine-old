typedef struct {} PyObject;
struct Py3dTransform;

static PyObject *Py3dTransform_Update(struct Py3dTransform *self, PyObject *args, PyObject *kwds) {
/*
    PyObject *superUpdateRet = Py3dComponent_Update((struct Py3dComponent *) self, args, NULL);
    if (superUpdateRet == NULL) return NULL;
    Py_CLEAR(superUpdateRet);

    dBodySetPosition(self->dynamicsBody, self->position[0], self->position[1], self->position[2]);

    dQuaternion localOrientation;
    localOrientation[0] = self->orientation[3];
    localOrientation[1] = self->orientation[0];
    localOrientation[2] = self->orientation[1];
    localOrientation[3] = self->orientation[2];

    dBodySetQuaternion(self->dynamicsBody, localOrientation);
*/
    return (void *) 0;
}

static struct PhysicsSpace *Py3dTransform_GetPhysicsSpace(struct Py3dTransform *self) {
/*
    struct Py3dGameObject *owner = (struct Py3dGameObject *) Py3dComponent_GetOwner((struct Py3dComponent *) self, NULL);
    if (owner == NULL) return NULL;

    struct Py3dScene *scene = Py3dGameObject_GetScene(owner);
    Py_CLEAR(owner);

    if (scene == NULL) return NULL;

    return scene->space;
*/
    return (void *) 0;
}