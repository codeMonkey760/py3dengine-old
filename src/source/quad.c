#include <stdlib.h>

#include "util.h"
#include "shader.h"
#include "quad.h"
#include "model.h"

static void refreshWorldMatrix(struct Quad *quad) {
    if (quad == NULL || quad->wMtxCacheDirty == false) return;

    float sMtx[16] = {0.0f};
    Mat4ScalingFA(sMtx, quad->_scale);

    float rMtx[16] = {0.0f};
    Mat4RotationQuaternionFA(rMtx, quad->_orientation);

    float tMtx[16] = {0.0f};
    Mat4TranslationFA(tMtx, quad->_posW);

    float wMtx[16] = {0.0f};
    Mat4Mult(wMtx, rMtx, sMtx);
    Mat4Mult(wMtx, wMtx, tMtx);

    Mat4Copy(quad->wMtxCache, wMtx);
    quad->wMtxCacheDirty = false;
}

void allocQuad(struct Quad **quadPtr, struct Model *model) {
    if (quadPtr == NULL || (*quadPtr != NULL) || model == NULL) return;

    struct Quad *newQuad = calloc(1, sizeof(struct Quad));
    if (newQuad == NULL) return;

    Vec3Identity(newQuad->_posW);
    newQuad->_posW[3] = 1.0f;
    QuaternionIdentity(newQuad->_orientation);
    newQuad->_scale[0] = 1.0f;
    newQuad->_scale[1] = 1.0f;
    newQuad->_scale[2] = 1.0f;

    Mat4Identity(newQuad->wMtxCache);
    newQuad->wMtxCacheDirty = true;

    refreshWorldMatrix(newQuad);

    newQuad->model = model;

    (*quadPtr) = newQuad;
}

void deleteQuad(struct Quad **quadPtr) {
    if (quadPtr == NULL || (*quadPtr) == NULL) {
        return;
    }

    free((*quadPtr));
    (*quadPtr) = NULL;
}

void updateQuad(struct Quad *quad, float dt) {
    if (quad == NULL) return;

    float deltaRot[4];
    QuaternionIdentity(deltaRot);
    QuaternionFromAxisAngle(0.0f, 1.0f, 0.0f, dt * 45.0f, deltaRot);

    QuaternionMult(quad->_orientation, deltaRot, quad->_orientation);
    quad->wMtxCacheDirty = true;
}

void renderQuad(struct Quad *quad, struct Camera *camera) {
    if (quad == NULL) return;

    refreshWorldMatrix(quad);

    enableShader();
    setDiffuseColor(quad->_diffuseColor);

    float wvpMtx[16] = {0.0f};
    getVPMtx(camera, wvpMtx);
    Mat4Mult(wvpMtx, quad->wMtxCache, wvpMtx);
    setWVPMtx(wvpMtx);

    bindModel(quad->model);
    renderModel(quad->model);
    unbindModel(quad->model);

    disableShader();
}

void setPosWQuad(struct Quad *quad, float newPosW[3]) {
    if (quad == NULL) return;

    Vec3Copy(quad->_posW, newPosW);
    quad->wMtxCacheDirty = true;
}

void setOrientationQuad(struct Quad *quad, float newOrientation[4]) {
    if (quad == NULL) return;

    QuaternionCopy(quad->_orientation, newOrientation);
    quad->wMtxCacheDirty = true;
}

void setScaleQuad(struct Quad *quad, float newScale[3]) {
    if (quad == NULL) return;

    Vec3Copy(quad->_scale, newScale);
    quad->wMtxCacheDirty = true;
}

void setDiffuseColorQuad(struct Quad *quad, float newDiffuseColor[4]) {
    if (quad == NULL) return;

    Vec4Copy(quad->_diffuseColor, newDiffuseColor);
}