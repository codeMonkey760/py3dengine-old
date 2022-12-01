#include <stdlib.h>

#include "camera.h"
#include "util.h"

static void refreshViewMatrix(struct Camera *camera) {
    if (camera == NULL || camera->_viewMtxCacheDirty == false) return;

    // calc and write view matrix
}

static void refreshProjMatrix(struct Camera *camera) {
    if (camera == NULL || camera->_projMtxCacheDirty == false) return;

    int screenWidth = 800;
    int screenHeight = 600;

    float fov_x = DEG_TO_RAD(100.0f);
    float fov_y = ((float) screenHeight) / ((float) screenWidth) * fov_x;
    float aspect = ((float) screenWidth) / ((float) screenHeight);
    float f = tanf(2.0f / fov_y);
    float far = 100.0f;
    float near = 0.05f;

    float pMtx[16] = {0.0f};
    pMtx[0] = f / aspect;
    pMtx[5] = f;
    pMtx[10] = (far + near) / (near - far);
    pMtx[11] = (2.0f * far * near) / (near - far);
    pMtx[14] = -1.0f;

    Mat4Copy(camera->_projMtxCache, pMtx);
    camera->_projMtxCacheDirty = false;
}

static void refreshViewProjMatrix(struct Camera *camera) {
    if (camera == NULL || camera->_viewProjMtxCacheDirty == false) return;

    refreshViewMatrix(camera);
    refreshProjMatrix(camera);

    Mat4Mult(camera->_viewProjMtxCache, camera->_viewMtxCache, camera->_projMtxCache);
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