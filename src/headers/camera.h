#ifndef PY3DENGINE_CAMERA_H
#define PY3DENGINE_CAMERA_H

#include <stdbool.h>

struct Camera {
    float _posW[3];

    float _viewMtxCache[16];
    bool _viewMtxCacheDirty;
    float _projMtxCache[16];
    bool _projMtxCacheDirty;
    float _viewProjMtxCache[16];
    bool _viewProjMtxCacheDirty;
};

extern void allocCamera(struct Camera **cameraPtr);
extern void deleteCamera(struct Camera **cameraPtr);

extern void updateCamera(struct Camera *camera, float dt);

extern void getVPMtx(struct Camera *camera, float dst[16]);
extern void getCameraPositionW(struct Camera *camera, float dst[3]);

#endif
