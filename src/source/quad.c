#include <stdlib.h>

#include "util.h"
#include "quad.h"

static void refreshWorldMatrix(struct Quad *quad) {
    if (quad == NULL || quad->wMtxCacheDirty == false) return;

    float sMtx[16] = {0.0f};
    Mat4ScalingFA(sMtx, quad->_scale);

    float rMtx[16] = {0.0f};
    Mat4RotationQuaternionFA(rMtx, quad->_orientation);

    float tMtx[16] = {0.0f};
    Mat4TranslationFA(tMtx, quad->_posW);

    float wMtx[16] = {0.0f};
    Mat4Mult(wMtx, sMtx, rMtx);
    Mat4Mult(wMtx, wMtx, tMtx);

    Mat4Copy(quad->wMtxCache, wMtx);
    quad->wMtxCacheDirty = false;
}

void allocQuad(struct Quad **quadPtr, struct Model *model, struct Shader *shader) {
    if (quadPtr == NULL || (*quadPtr != NULL) || model == NULL || shader == NULL) return;

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
    newQuad->shader = shader;

    (*quadPtr) = newQuad;
}

void deleteQuad(struct Quad **quadPtr) {
    if (quadPtr == NULL || (*quadPtr) == NULL) {
        return;
    }

    struct Quad *quad = (*quadPtr);
    quad->model = NULL;
    quad->shader = NULL;

    free(quad);
    quad = NULL;
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
    if (quad == NULL || camera == NULL || quad->model == NULL || quad->shader == NULL) return;

    refreshWorldMatrix(quad);

    struct Shader *shader = quad->shader;

    enableShader(shader);
    setDiffuseColor(shader, quad->_diffuseColor);

    float cameraPos[3] = {0.0f};
    getCameraPositionW(camera, cameraPos);
    setCameraPosition(shader, cameraPos);

    setWMtx(shader, quad->wMtxCache);

    float witMtx[16] = {0.0f};
    Mat4Inverse(witMtx, quad->wMtxCache);
    Mat4Transpose(witMtx, witMtx);
    setWITMtx(shader, witMtx);

    float wvpMtx[16] = {0.0f};
    getVPMtx(camera, wvpMtx);
    Mat4Mult(wvpMtx, quad->wMtxCache, wvpMtx);
    setWVPMtx(shader, wvpMtx);

    bindModel(quad->model);
    renderModel(quad->model);
    unbindModel(quad->model);

    disableShader(shader);
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

void setDiffuseColorQuad(struct Quad *quad, float newDiffuseColor[3]) {
    if (quad == NULL) return;

    Vec3Copy(quad->_diffuseColor, newDiffuseColor);
}