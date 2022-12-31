#include <stdlib.h>
#include <string.h>

#include "logger.h"
#include "util.h"
#include "custom_string.h"
#include "components/transform_component.h"

#define COMPONENT_TYPE_TRANSFORM 3

static bool isComponentValid(struct BaseComponent *component) {
    if (component == NULL) return false;

    return component->_type == COMPONENT_TYPE_TRANSFORM;
}

static void delete(struct BaseComponent **componentPtr) {
    if (componentPtr == NULL || (*componentPtr) == NULL) return;

    if (!isComponentValid( (*componentPtr) )) return;

    deleteTransformComponent((struct TransformComponent **) componentPtr);
}

static void refreshMatrixCaches(struct TransformComponent *component) {
    if (component == NULL || component->_matrixCacheDirty == false) return;

    float sMtx[16] = {0.0f};
    Mat4ScalingFA(sMtx, component->_scale);

    float rMtx[16] = {0.0f};
    Mat4RotationQuaternionFA(rMtx, component->_orientation);

    float tMtx[16] = {0.0f};
    Mat4TranslationFA(tMtx, component->_position);

    float wMtx[16] = {0.0f};
    Mat4Mult(wMtx, sMtx, rMtx);
    Mat4Mult(wMtx, wMtx, tMtx);

    Mat4Copy(component->_wMtxCache, wMtx);

    Mat4Inverse(component->_witMtxCache, wMtx);
    Mat4Transpose(component->_witMtxCache, component->_witMtxCache);

    component->_matrixCacheDirty = false;
}

static void refreshViewMatrixCache(struct TransformComponent *component) {
    if (component == NULL || component->_viewMtxCacheDirty == false) return;

    float yaw = 0.0f;
    float pitch = 0.0f;

    float camTargetW[3] = {0.0f,0.0f,0.0f};
    float camUpW[3] = {0.0f,1.0f,0.0f};

    float qY[4] = {0.0f};
    float qX[4] = {0.0f};
    float qTot[4] = {0.0f};

    QuaternionFromAxisAngle(0.0f,1.0f,0.0f,yaw,qY);
    QuaternionFromAxisAngle(1.0f,0.0f,0.0f,pitch,qX);
    QuaternionMult(qY,qX,qTot);

    QuaternionVec3Rotation(component->_position, qTot, component->_position);

    Mat4LookAtLH(component->_viewMtxCache, component->_position, camTargetW, camUpW);
    component->_viewMtxCacheDirty = false;
}

static bool parseVec(json_object *json, const char *name, float dst[4], size_t vecSize) {
    json_object *vec = json_object_object_get(json, name);
    if (vec == NULL || !json_object_is_type(vec, json_type_object)) {
        error_log("[TransformComponent]: Transform must have an object property called \"%s\"", name);
        return false;
    }

    char elements[8] = {'x', 0, 'y', 0, 'z', 0, 'w', 0};
    for (int i = 0; i < vecSize; ++i) {
        char *element_name = elements+(i*2);
        json_object *element = json_object_object_get(vec, element_name);
        if (element == NULL || !json_object_is_type(element, json_type_double)) {
            error_log("[TransformComponent]: Transform must have a double property called \"%s\"", element_name);
            return false;
        }
        dst[i] = (float) json_object_get_double(element);
    }

    return true;
}

void allocTransformComponent(struct TransformComponent **componentPtr) {
    if (componentPtr == NULL || (*componentPtr) != NULL) return;

    struct TransformComponent *newComponent = calloc(1, sizeof(struct TransformComponent));
    if (newComponent == NULL) return;

    struct BaseComponent *base = (struct BaseComponent *) newComponent;
    initializeBaseComponent(base);
    base->_type = COMPONENT_TYPE_TRANSFORM;
    allocString(&base->_typeName, COMPONENT_TYPE_NAME_TRANSFORM);
    base->delete = delete;

    Vec3Identity(newComponent->_position);
    QuaternionIdentity(newComponent->_orientation);
    Vec3Fill(newComponent->_scale, 1.0f);

    Mat4Identity(newComponent->_wMtxCache);
    Mat4Identity(newComponent->_witMtxCache);
    newComponent->_matrixCacheDirty = true;

    Mat4Identity(newComponent->_viewMtxCache);
    newComponent->_viewMtxCacheDirty = true;

    (*componentPtr) = newComponent;
    newComponent = NULL;
    base = NULL;
}

void deleteTransformComponent(struct TransformComponent **componentPtr) {
    if (componentPtr == NULL || (*componentPtr) == NULL) return;

    finalizeBaseComponent((struct BaseComponent *) (*componentPtr));

    free( (*componentPtr) );
    (*componentPtr) = NULL;
}

void parseTransformComponent(json_object *json, struct TransformComponent *component) {
    if (json == NULL || component == NULL) return;

    float dataBuffer[4];
    memset(dataBuffer, 0, sizeof(float) * 4);

    if (parseVec(json, "position", dataBuffer, 3) == false) return;
    Vec3Copy(component->_position, dataBuffer);

    if (parseVec(json, "orientation", dataBuffer, 4) == false) return;
    QuaternionCopy(component->_orientation, dataBuffer);

    if (parseVec(json, "scale", dataBuffer, 3) == false) return;
    Vec3Copy(component->_scale, dataBuffer);
}

void moveTransform(struct TransformComponent *component, float displacement[3]) {
    if (component == NULL || displacement == NULL) return;

    Vec3Add(component->_position, component->_position, displacement);
    component->_matrixCacheDirty = true;
    component->_viewMtxCacheDirty = true;
}

void setTransformPosition(struct TransformComponent *component, float newPosition[3]) {
    if (component == NULL || newPosition == NULL) return;

    Vec3Copy(component->_position, newPosition);
    component->_matrixCacheDirty = true;
    component->_viewMtxCacheDirty = true;
}

void rotateTransform(struct TransformComponent *component, float displacement[4]) {
    if (component == NULL || displacement == NULL) return;

    QuaternionMult(component->_orientation, displacement, component->_orientation);
    component->_matrixCacheDirty = true;
    component->_viewMtxCacheDirty = true;
}

void setTransformOrientation(struct TransformComponent *component, float newOrientation[4]) {
    if (component == NULL || newOrientation == NULL) return;

    QuaternionCopy(component->_orientation, newOrientation);
    component->_matrixCacheDirty = true;
    component->_viewMtxCacheDirty = true;
}

void stretchTransform(struct TransformComponent *component, float factors[3]) {
    if (component == NULL || factors == NULL) return;

    component->_scale[0] *= factors[0];
    component->_scale[1] *= factors[1];
    component->_scale[2] *= factors[2];
    component->_matrixCacheDirty = true;
    component->_viewMtxCacheDirty = true;
}

void setTransformScale(struct TransformComponent *component, float newScale[3]) {
    if (component == NULL || newScale == NULL) return;

    Vec3Copy(component->_scale, newScale);
    component->_matrixCacheDirty = true;
    component->_viewMtxCacheDirty = true;
}

float *getTransformWorldMtx(struct TransformComponent *component) {
    if (component == NULL) return NULL;

    refreshMatrixCaches(component);

    return component->_wMtxCache;
}

float *getTransformWITMtx(struct TransformComponent *component) {
    if (component == NULL) return NULL;

    refreshMatrixCaches(component);

    return component->_witMtxCache;
}

float *getTransformViewMtx(struct TransformComponent *component) {
    if (component == NULL) return NULL;

    refreshViewMatrixCache(component);

    return component->_viewMtxCache;
}