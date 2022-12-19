#include <stdlib.h>

#include "camera.h"
#include "util.h"

static void refreshViewMatrix(struct Camera *camera) {
    if (camera == NULL || camera->_viewMtxCacheDirty == false) return;

    float yaw = 0.0f;
    float pitch = 0.0f;

    float camPosW[3] = {0.0f,0.0f,-2.0f};
    float camTargetW[3] = {0.0f,0.0f,0.0f};
    float camUpW[3] = {0.0f,1.0f,0.0f};

    float qY[4] = {0.0f};
    float qX[4] = {0.0f};
    float qTot[4] = {0.0f};

    QuaternionFromAxisAngle(0.0f,1.0f,0.0f,yaw,qY);
    QuaternionFromAxisAngle(1.0f,0.0f,0.0f,pitch,qX);
    QuaternionMult(qY,qX,qTot);

    QuaternionVec3Rotation(camPosW,qTot,camPosW);

    Mat4LookAtLH(camera->_viewMtxCache,camPosW,camTargetW,camUpW);
    camera->_viewMtxCacheDirty = false;
}

static void refreshProjMatrix(struct Camera *camera) {
    if (camera == NULL || camera->_projMtxCacheDirty == false) return;

    int screenWidth = 800;
    int screenHeight = 600;
    float aspectRatio = ((float) screenWidth) / ((float) screenHeight);
    float aspectRatioInv = ((float) screenHeight) / ((float) screenWidth);

    float fovxInDegrees = 100.0f;
    float nearZ = 0.05f;
    float farZ = 10.0f;
    float fov_y = DEG_TO_RAD(aspectRatioInv * fovxInDegrees);

    float w = 1.0f / (aspectRatio * tanf(fov_y / 2.0f));
    float h = 1.0f / (tanf(fov_y / 2.0f));

    float pMtx[16] = {0.0f};
    Mat4Identity(pMtx);
    pMtx[0] = w;
    pMtx[5] = h;
    pMtx[10] = (farZ) / (farZ - nearZ);
    pMtx[11] = 1.0f;
    pMtx[14] = (-1.0f * nearZ * farZ) / (farZ - nearZ);

    Mat4Copy(camera->_projMtxCache, pMtx);
    camera->_projMtxCacheDirty = false;
}

static void refreshViewProjMatrix(struct Camera *camera) {
    if (camera == NULL || camera->_viewProjMtxCacheDirty == false) return;

    refreshViewMatrix(camera);
    refreshProjMatrix(camera);

    Mat4Mult(camera->_viewProjMtxCache, camera->_viewMtxCache, camera->_projMtxCache);
    camera->_viewProjMtxCacheDirty = false;
}

void allocCamera(struct Camera **cameraPtr) {
    if (cameraPtr == NULL || (*cameraPtr) != NULL) return;

    struct Camera *camera = calloc(1, sizeof(struct Camera));
    if (camera == NULL) return;

    Mat4Identity(camera->_viewMtxCache);
    camera->_viewMtxCacheDirty = true;
    Mat4Identity(camera->_projMtxCache);
    camera->_projMtxCacheDirty = true;
    Mat4Identity(camera->_viewProjMtxCache);
    camera->_viewProjMtxCacheDirty = true;

    refreshViewProjMatrix(camera);

    (*cameraPtr) = camera;
}

void deleteCamera(struct Camera **cameraPtr) {
    if (cameraPtr == NULL || (*cameraPtr) == NULL) return;

    free((*cameraPtr));
    (*cameraPtr) = NULL;
}

void updateCamera(struct Camera *camera, float dt) {
    if (camera == NULL) return;

    refreshViewProjMatrix(camera);
}

void getVPMtx(struct Camera *camera, float dst[16]) {
    if (camera == NULL || dst == NULL) return;

    refreshViewProjMatrix(camera);

    Mat4Copy(dst, camera->_viewProjMtxCache);
}

void getCameraPositionW(struct Camera *camera, float dst[3]) {
    if (camera == NULL || dst == NULL) return;

    Vec3Copy(dst, camera->_posW);
}